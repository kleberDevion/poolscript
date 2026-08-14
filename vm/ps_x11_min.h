/*
 * ps_x11_min — interface MÍNIMA do Xlib, declarada À MÃO (sem libx11-dev).
 *
 * Só o que o guzer usa. A ABI do X11 é estável e pública; o ps_guzer.c tem
 * `_Static_assert` conferindo o layout dos eventos, então um erro de offset
 * quebra a compilação (não precisa abrir janela pra pegar). Linka direto
 * contra libX11.so.6 (o runtime já vem no sistema; zero download).
 *
 * Usa só as formas X-prefixadas (funções reais) — nada de macro que cutuque o
 * struct interno do Display (que é opaco aqui).
 */
#ifndef PS_X11_MIN_H
#define PS_X11_MIN_H

#include <stddef.h>

typedef unsigned long XID;
typedef XID  Window;
typedef XID  Drawable;
typedef XID  Font;
typedef XID  Colormap;
typedef XID  Atom;
typedef unsigned long Time;
typedef int  Bool;
typedef int  Status;
typedef struct _XDisplay Display;   /* opaco */
typedef struct _XGC     *GC;        /* opaco */

#define False 0
#define True  1

/* tipos de evento */
#define ButtonPress    4
#define Expose         12
#define ClientMessage  33
/* máscaras de XSelectInput */
#define ExposureMask     (1L << 15)
#define ButtonPressMask  (1L << 2)
/* flags de XColor */
#define DoRed   1
#define DoGreen 2
#define DoBlue  4

typedef struct {
    unsigned long  pixel;
    unsigned short red, green, blue;
    char flags;
    char pad;
} XColor;

typedef struct {
    int type;
    unsigned long serial;
    Bool send_event;
    Display *display;
    Window window;
} XAnyEvent;

typedef struct {
    int type;
    unsigned long serial;
    Bool send_event;
    Display *display;
    Window window;
    Window root;
    Window subwindow;
    Time time;
    int x, y;
    int x_root, y_root;
    unsigned int state;
    unsigned int button;
    Bool same_screen;
} XButtonEvent;

typedef struct {
    int type;
    unsigned long serial;
    Bool send_event;
    Display *display;
    Window window;
    int x, y;
    int width, height;
    int count;
} XExposeEvent;

typedef struct {
    int type;
    unsigned long serial;
    Bool send_event;
    Display *display;
    Window window;
    Atom message_type;
    int format;
    union { char b[20]; short s[10]; long l[5]; } data;
} XClientMessageEvent;

typedef union _XEvent {
    int type;
    XAnyEvent           xany;
    XButtonEvent        xbutton;
    XExposeEvent        xexpose;
    XClientMessageEvent xclient;
    long pad[24];       /* garante o tamanho real do XEvent (24 longs) */
} XEvent;

extern Display *XOpenDisplay(const char *);
extern int      XCloseDisplay(Display *);
extern int      XDefaultScreen(Display *);
extern Window   XRootWindow(Display *, int);
extern Colormap XDefaultColormap(Display *, int);
extern unsigned long XBlackPixel(Display *, int);
extern Window   XCreateSimpleWindow(Display *, Window, int, int, unsigned, unsigned,
                                    unsigned, unsigned long, unsigned long);
extern int      XStoreName(Display *, Window, const char *);
extern Atom     XInternAtom(Display *, const char *, Bool);
extern Status   XSetWMProtocols(Display *, Window, Atom *, int);
extern int      XSelectInput(Display *, Window, long);
extern int      XMapWindow(Display *, Window);
extern GC       XCreateGC(Display *, Drawable, unsigned long, void *);
extern Font     XLoadFont(Display *, const char *);
extern int      XSetFont(Display *, GC, Font);
extern int      XSetForeground(Display *, GC, unsigned long);
extern int      XFillRectangle(Display *, Drawable, GC, int, int, unsigned, unsigned);
extern int      XDrawString(Display *, Drawable, GC, int, int, const char *, int);
extern int      XFlush(Display *);
extern int      XNextEvent(Display *, XEvent *);
extern Status   XAllocColor(Display *, Colormap, XColor *);
extern int      XFreeGC(Display *, GC);
extern int      XDestroyWindow(Display *, Window);
extern int      XUnloadFont(Display *, Font);

#endif /* PS_X11_MIN_H */
