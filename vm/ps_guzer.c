/*
 * ps_guzer — backend de janela nativa do guzer (Xlib puro). Ver ps_guzer.h.
 *
 * Desenha os widgets como retângulos com texto e roda o loop de eventos do
 * X11. Não conhece a VM: clique -> callback(id). X11 já vem no sistema
 * (libX11.so.6), zero dependência nova. A interface do Xlib é declarada à mão
 * em ps_x11_min.h; os _Static_assert abaixo garantem o layout dos eventos.
 */
#include "ps_guzer.h"
#include "ps_x11_min.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <poll.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <png.h>

/* Layout da ABI do X11 (x86-64 SysV). Se algo aqui falhar, é o header à mão
 * que está errado — pega na compilação, sem precisar abrir janela. */
_Static_assert(sizeof(XEvent) >= 192,               "XEvent pequeno demais");
_Static_assert(offsetof(XButtonEvent, window) == 32, "xbutton.window offset");
_Static_assert(offsetof(XButtonEvent, x) == 64,      "xbutton.x offset");
_Static_assert(offsetof(XButtonEvent, y) == 68,      "xbutton.y offset");
_Static_assert(offsetof(XExposeEvent, count) == 56,  "xexpose.count offset");
_Static_assert(offsetof(XClientMessageEvent, data) == 56, "xclient.data offset");

/* 0xRRGGBB -> pixel do visual atual (via colormap, correto em qualquer visual) */
static unsigned long aloca_cor(Display *dpy, Colormap cmap, unsigned long rgb)
{
    XColor c;
    c.red   = (unsigned short)(((rgb >> 16) & 0xFF) * 257);
    c.green = (unsigned short)(((rgb >> 8)  & 0xFF) * 257);
    c.blue  = (unsigned short)(( rgb        & 0xFF) * 257);
    c.flags = DoRed | DoGreen | DoBlue;
    if (!XAllocColor(dpy, cmap, &c)) return 0;
    return c.pixel;
}

static void desenha_png(Display *dpy, Window win, GC gc, const PSGuzWidget *w);

static void desenha(Display *dpy, Window win, GC gc, Colormap cmap,
                    const PSGuzWidget *ws, int n)
{
    for (int i = 0; i < n; i++) {
        const PSGuzWidget *w = &ws[i];
        XSetForeground(dpy, gc, aloca_cor(dpy, cmap, w->bg));
        XFillRectangle(dpy, win, gc, w->x, w->y, (unsigned)w->w, (unsigned)w->h);
        if (w->kind == PSGUZ_VIDEO || w->kind == PSGUZ_AUDIO) {
            /* a caixa base; os frames do vídeo entram por cima no loop */
            const char *aviso = w->text;
            if (aviso && aviso[0]) {
                XSetForeground(dpy, gc, aloca_cor(dpy, cmap, w->fg));
                XDrawString(dpy, win, gc, w->x + 8, w->y + w->h / 2 + 4,
                            aviso, (int)strlen(aviso));
            }
            continue;
        }
        if (w->src) { desenha_png(dpy, win, gc, w); continue; }
        const char *t = w->text ? w->text : "";
        int len = (int)strlen(t);
        if (len > 0) {
            /* sem XFontStruct: texto ancorado à esquerda, centralizado na vertical */
            int tx = w->x + 8;
            int ty = w->y + w->h / 2 + 4;
            XSetForeground(dpy, gc, aloca_cor(dpy, cmap, w->fg));
            XDrawString(dpy, win, gc, tx, ty, t, len);
        }
    }
    XFlush(dpy);
}

int ps_guz_png_tamanho(const char *path, int *w, int *h)
{
    png_image img;
    memset(&img, 0, sizeof img);
    img.version = PNG_IMAGE_VERSION;
    if (!png_image_begin_read_from_file(&img, path)) return -1;
    *w = (int)img.width; *h = (int)img.height;
    png_image_free(&img);
    return 0;
}

/* img: decodifica o PNG, mistura o alpha com o fundo do widget, escala
 * (vizinho mais próximo) pra caixa e joga na janela via XPutImage. */
static void desenha_png(Display *dpy, Window win, GC gc, const PSGuzWidget *w)
{
    png_image img;
    memset(&img, 0, sizeof img);
    img.version = PNG_IMAGE_VERSION;
    if (!png_image_begin_read_from_file(&img, w->src)) return;
    img.format = PNG_FORMAT_RGBA;
    png_bytep rgba = malloc(PNG_IMAGE_SIZE(img));
    if (!rgba) { png_image_free(&img); return; }
    if (!png_image_finish_read(&img, NULL, rgba, 0, NULL)) { free(rgba); png_image_free(&img); return; }
    int sw = (int)img.width, sh = (int)img.height;
    int dw = w->w > 0 ? w->w : sw, dh = w->h > 0 ? w->h : sh;
    unsigned *px = malloc(sizeof(unsigned) * (size_t)dw * (size_t)dh);
    if (!px) { free(rgba); png_image_free(&img); return; }
    unsigned br = (w->bg >> 16) & 0xFF, bgc = (w->bg >> 8) & 0xFF, bb = w->bg & 0xFF;
    for (int y = 0; y < dh; y++) {
        int sy = sh > 1 ? (int)((long)y * sh / dh) : 0;
        for (int x = 0; x < dw; x++) {
            int sx = sw > 1 ? (int)((long)x * sw / dw) : 0;
            const unsigned char *q = rgba + ((size_t)sy * (size_t)sw + (size_t)sx) * 4;
            unsigned al = q[3];
            unsigned r = (q[0] * al + br  * (255 - al)) / 255;
            unsigned g = (q[1] * al + bgc * (255 - al)) / 255;
            unsigned b2 = (q[2] * al + bb  * (255 - al)) / 255;
            px[(size_t)y * (size_t)dw + (size_t)x] = (r << 16) | (g << 8) | b2;
        }
    }
    int scr = XDefaultScreen(dpy);
    XImage *xi = XCreateImage(dpy, XDefaultVisual(dpy, scr),
                              (unsigned)XDefaultDepth(dpy, scr), ZPixmap, 0,
                              (char *)px, (unsigned)dw, (unsigned)dh, 32, 0);
    if (xi) {
        XPutImage(dpy, win, gc, xi, 0, 0, w->x, w->y, (unsigned)dw, (unsigned)dh);
        XFree(xi);           /* a struct; o buffer px é nosso */
    }
    free(px);
    free(rgba);
    png_image_free(&img);
}

/* Ícone da janela: decodifica o PNG (libpng) e seta _NET_WM_ICON (w,h,ARGB...). */
static void guz_set_icon(Display *dpy, Window win, const char *path)
{
    png_image img;
    memset(&img, 0, sizeof img);
    img.version = PNG_IMAGE_VERSION;
    if (!png_image_begin_read_from_file(&img, path)) return;   /* não é PNG / não abriu */
    img.format = PNG_FORMAT_RGBA;
    png_bytep buf = malloc(PNG_IMAGE_SIZE(img));
    if (!buf) { png_image_free(&img); return; }
    if (png_image_finish_read(&img, NULL, buf, 0, NULL)) {
        int w = (int)img.width, h = (int)img.height;
        long *prop = malloc(sizeof(long) * (size_t)(2 + w * h));
        if (prop) {
            prop[0] = w; prop[1] = h;
            for (int i = 0; i < w * h; i++) {
                unsigned char r = buf[i*4], g = buf[i*4+1], b = buf[i*4+2], a = buf[i*4+3];
                prop[2 + i] = ((long)a << 24) | ((long)r << 16) | ((long)g << 8) | (long)b;
            }
            Atom net_icon = XInternAtom(dpy, "_NET_WM_ICON", False);
            XChangeProperty(dpy, win, net_icon, XA_CARDINAL, 32, PropModeReplace,
                            (const unsigned char *)prop, 2 + w * h);
            free(prop);
        }
    }
    free(buf);
    png_image_free(&img);
}

/* ── mídia (audio/video) via ffmpeg — subprocesso, zero dependência de build ──
 * video: `ffmpeg -re -i src -vf scale=WxH -f rawvideo -pix_fmt rgb24 -` manda
 * frames CRUS pelo pipe; o loop de eventos (poll no fd do X + fd do pipe)
 * desenha cada frame na caixa do elemento com XPutImage. O áudio da trilha (e
 * o elemento audio) tocam com `ffplay -nodisp`. Sem ffmpeg no sistema, a caixa
 * mostra o aviso e o resto do app segue normal. */
typedef struct {
    const PSGuzWidget *w;
    int   fd;            /* stdout do ffmpeg (frames rgb24) */
    pid_t pid;
    unsigned char *acc;  /* frame em montagem */
    size_t tam, cheio;   /* tam = w*h*3 */
    unsigned *px;        /* frame convertido pro X (0x00RRGGBB) */
} GuzVideo;

static int guz_tem_cmd(const char *cmd)
{
    char linha[256];
    snprintf(linha, sizeof linha, "command -v %s >/dev/null 2>&1", cmd);
    return system(linha) == 0;
}

static pid_t guz_spawn_audio(const char *src)
{
    pid_t pid = fork();
    if (pid != 0) return pid;
    /* filho: toca e sai sozinho no fim */
    int nulo = open("/dev/null", O_RDWR);
    if (nulo >= 0) { dup2(nulo, 1); dup2(nulo, 2); }
    execlp("ffplay", "ffplay", "-nodisp", "-autoexit", "-loglevel", "quiet", src, (char *)NULL);
    _exit(127);
}

static int guz_video_abre(GuzVideo *v, const PSGuzWidget *w)
{
    memset(v, 0, sizeof *v);
    v->w = w; v->fd = -1; v->pid = -1;
    int canos[2];
    if (pipe(canos) != 0) return -1;
    char escala[64];
    snprintf(escala, sizeof escala, "scale=%dx%d", w->w > 0 ? w->w : 320, w->h > 0 ? w->h : 240);
    pid_t pid = fork();
    if (pid < 0) { close(canos[0]); close(canos[1]); return -1; }
    if (pid == 0) {
        close(canos[0]);
        dup2(canos[1], 1);
        int nulo = open("/dev/null", O_RDWR);
        if (nulo >= 0) dup2(nulo, 2);
        execlp("ffmpeg", "ffmpeg", "-loglevel", "quiet", "-re", "-i", w->src,
               "-vf", escala, "-f", "rawvideo", "-pix_fmt", "rgb24", "-", (char *)NULL);
        _exit(127);
    }
    close(canos[1]);
    fcntl(canos[0], F_SETFL, O_NONBLOCK);
    v->fd = canos[0]; v->pid = pid;
    v->tam = (size_t)(w->w > 0 ? w->w : 320) * (size_t)(w->h > 0 ? w->h : 240) * 3;
    v->acc = malloc(v->tam);
    v->px  = malloc(sizeof(unsigned) * (v->tam / 3));
    if (!v->acc || !v->px) { free(v->acc); free(v->px); close(v->fd); v->fd = -1; }
    return v->fd >= 0 ? 0 : -1;
}

/* lê o que der do pipe; devolve 1 quando um frame completo ficou pronto */
static int guz_video_le(GuzVideo *v)
{
    if (v->fd < 0) return 0;
    int pronto = 0;
    for (;;) {
        ssize_t r = read(v->fd, v->acc + v->cheio, v->tam - v->cheio);
        if (r <= 0) break;
        v->cheio += (size_t)r;
        if (v->cheio == v->tam) {
            size_t n = v->tam / 3;
            for (size_t i = 0; i < n; i++)
                v->px[i] = ((unsigned)v->acc[i*3] << 16) |
                           ((unsigned)v->acc[i*3+1] << 8) | v->acc[i*3+2];
            v->cheio = 0;
            pronto = 1;
        }
    }
    return pronto;
}

static void guz_video_desenha(Display *dpy, Window win, GC gc, const GuzVideo *v)
{
    if (!v->px) return;
    int scr = XDefaultScreen(dpy);
    int vw = v->w->w > 0 ? v->w->w : 320, vh = v->w->h > 0 ? v->w->h : 240;
    XImage *xi = XCreateImage(dpy, XDefaultVisual(dpy, scr),
                              (unsigned)XDefaultDepth(dpy, scr), ZPixmap, 0,
                              (char *)v->px, (unsigned)vw, (unsigned)vh, 32, 0);
    if (xi) {
        XPutImage(dpy, win, gc, xi, 0, 0, v->w->x, v->w->y, (unsigned)vw, (unsigned)vh);
        XFree(xi);
    }
    XFlush(dpy);
}

static void guz_video_fecha(GuzVideo *v)
{
    if (v->fd >= 0) close(v->fd);
    if (v->pid > 0) { kill(v->pid, SIGTERM); waitpid(v->pid, NULL, 0); }
    free(v->acc); free(v->px);
    v->fd = -1; v->pid = -1; v->acc = NULL; v->px = NULL;
}

int ps_guz_run(const char *titulo, const char *icone,
               int win_w, int win_h, unsigned long win_bg,
               const PSGuzWidget *widgets, int n,
               PSGuzClickCb cb, void *ud,
               char *erro, size_t ecap)
{
    Display *dpy = XOpenDisplay(NULL);
    if (!dpy) {
        snprintf(erro, ecap, "guzer: sem servidor X (defina DISPLAY / rode num desktop)");
        return -1;
    }
    int      scr  = XDefaultScreen(dpy);
    Window   root = XRootWindow(dpy, scr);
    Colormap cmap = XDefaultColormap(dpy, scr);

    Window win = XCreateSimpleWindow(
        dpy, root, 0, 0, (unsigned)win_w, (unsigned)win_h, 0,
        XBlackPixel(dpy, scr), aloca_cor(dpy, cmap, win_bg));
    XStoreName(dpy, win, titulo ? titulo : "PoolScript");
    if (icone) guz_set_icon(dpy, win, icone);

    Atom wm_delete = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(dpy, win, &wm_delete, 1);
    XSelectInput(dpy, win, ExposureMask | ButtonPressMask);
    XMapWindow(dpy, win);

    GC gc = XCreateGC(dpy, win, 0, NULL);
    Font font = XLoadFont(dpy, "fixed");
    if (font) XSetFont(dpy, gc, font);

    /* mídia: vídeo(s) por pipe de frames; áudio (e trilha do vídeo) no ffplay */
    int tem_ffmpeg = guz_tem_cmd("ffmpeg");
    int tem_ffplay = guz_tem_cmd("ffplay");
    GuzVideo vids[8]; int nvids = 0;
    pid_t auds[16]; int nauds = 0;
    for (int i = 0; i < n; i++) {
        const PSGuzWidget *w = &widgets[i];
        if (!w->src || (w->kind != PSGUZ_VIDEO && w->kind != PSGUZ_AUDIO)) continue;
        if (w->kind == PSGUZ_VIDEO && tem_ffmpeg && nvids < 8) {
            if (guz_video_abre(&vids[nvids], w) == 0) nvids++;
        }
        if (tem_ffplay && nauds < 16)
            auds[nauds++] = guz_spawn_audio(w->src);   /* audio E trilha do vídeo */
    }
    int xfd = XConnectionNumber(dpy);
    int rodando = 1;
    while (rodando) {
        /* espera evento do X OU frame de vídeo (33ms ~ 30fps) */
        while (!XPending(dpy)) {
            struct pollfd pf[9];
            pf[0].fd = xfd; pf[0].events = POLLIN;
            for (int v = 0; v < nvids; v++) { pf[1 + v].fd = vids[v].fd; pf[1 + v].events = POLLIN; }
            int pr = poll(pf, (nfds_t)(1 + nvids), nvids ? 33 : -1);
            if (pr < 0) break;
            for (int v = 0; v < nvids; v++)
                if (guz_video_le(&vids[v]))
                    guz_video_desenha(dpy, win, gc, &vids[v]);
            if (pf[0].revents & POLLIN) break;
        }
        XEvent ev;
        XNextEvent(dpy, &ev);
        if (ev.type == Expose) {
            if (ev.xexpose.count == 0) {
                desenha(dpy, win, gc, cmap, widgets, n);
                for (int v = 0; v < nvids; v++) guz_video_desenha(dpy, win, gc, &vids[v]);
            }
        } else if (ev.type == ButtonPress) {
            int mx = ev.xbutton.x, my = ev.xbutton.y;
            /* de trás pra frente: o desenhado por último fica "por cima" */
            for (int i = n - 1; i >= 0; i--) {
                const PSGuzWidget *w = &widgets[i];
                if (!w->clicavel) continue;
                if (mx >= w->x && mx < w->x + w->w &&
                    my >= w->y && my < w->y + w->h) {
                    if (cb) cb(w->id, ud);
                    break;
                }
            }
        } else if (ev.type == ClientMessage) {
            if ((Atom)ev.xclient.data.l[0] == wm_delete) rodando = 0;
        }
    }

    for (int v = 0; v < nvids; v++) guz_video_fecha(&vids[v]);
    for (int a2 = 0; a2 < nauds; a2++) {
        if (auds[a2] > 0) { kill(auds[a2], SIGTERM); waitpid(auds[a2], NULL, 0); }
    }

    if (font) XUnloadFont(dpy, font);
    XFreeGC(dpy, gc);
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
    return 0;
}
