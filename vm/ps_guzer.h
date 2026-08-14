/*
 * ps_guzer — backend de janela nativa do guzer (Xlib puro, X11).
 *
 * Mesmo princípio do ps_jinker.c: este arquivo NÃO conhece a VM. Recebe uma
 * lista de widgets já resolvidos (retângulo + cor + texto) e um callback de
 * clique; abre UMA janela X11, desenha, roda o loop de eventos e, quando um
 * widget clicável é clicado, chama o callback com o id dele. Quem traduz os
 * objetos guzer da linguagem pra esta lista, e quem chama a `reaction` no
 * clique (via chama_valor), é a camada guzer dentro da VM.
 *
 * O espelho semântico é o guzer_lib.py (tkinter): os mesmos objetos, o mesmo
 * design default, o mesmo disparo de handler no clique.
 *
 * X11 já vem no sistema (libX11). Zero dependência de runtime nova.
 */
#ifndef PS_GUZER_H
#define PS_GUZER_H

#include <stddef.h>

/* Um widget já resolvido em pixels e cores (0xRRGGBB). */
typedef enum { PSGUZ_BUTTON, PSGUZ_POPUP } PSGuzKind;

typedef struct {
    PSGuzKind kind;
    int   x, y, w, h;
    unsigned long bg;     /* 0xRRGGBB */
    unsigned long fg;
    const char   *text;   /* pode ser "" */
    int   clicavel;       /* tem handler? */
    int   id;             /* devolvido no callback de clique */
} PSGuzWidget;

/* Chamado quando um widget clicável é clicado. `ud` é o userdata passado. */
typedef void (*PSGuzClickCb)(int id, void *ud);

/*
 * Abre a janela `titulo` (win_w x win_h, fundo win_bg), desenha os widgets e
 * roda o loop até a janela fechar. Cada clique dentro de um widget clicável
 * chama `cb(id, ud)`.
 *
 * Retorna 0 ao fechar normalmente; -1 se não deu pra abrir (sem DISPLAY, sem
 * libX11), com a razão em `erro`.
 */
int ps_guz_run(const char *titulo, const char *icone,
               int win_w, int win_h, unsigned long win_bg,
               const PSGuzWidget *widgets, int n,
               PSGuzClickCb cb, void *ud,
               char *erro, size_t ecap);

#endif /* PS_GUZER_H */
