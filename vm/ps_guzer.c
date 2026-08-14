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

static void desenha(Display *dpy, Window win, GC gc, Colormap cmap,
                    const PSGuzWidget *ws, int n)
{
    for (int i = 0; i < n; i++) {
        const PSGuzWidget *w = &ws[i];
        XSetForeground(dpy, gc, aloca_cor(dpy, cmap, w->bg));
        XFillRectangle(dpy, win, gc, w->x, w->y, (unsigned)w->w, (unsigned)w->h);
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

    for (;;) {
        XEvent ev;
        XNextEvent(dpy, &ev);
        if (ev.type == Expose) {
            if (ev.xexpose.count == 0)
                desenha(dpy, win, gc, cmap, widgets, n);
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
            if ((Atom)ev.xclient.data.l[0] == wm_delete) break;
        }
    }

    if (font) XUnloadFont(dpy, font);
    XFreeGC(dpy, gc);
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
    return 0;
}
