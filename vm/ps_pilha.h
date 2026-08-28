/*
 * Folga da pilha do C — a medição que substitui os tetos por contagem.
 *
 * POR QUE ISTO EXISTE: o motor tinha limites de profundidade escritos como
 * NÚMERO DE NÍVEIS (`PS_CICLO_MAX`, `PS_PARSE_PROF_MAX`). Contar nível não mede
 * pilha: o mesmo 300 que sobra folgado num binário `-O2` estoura num `-O0
 * --coverage`, porque o quadro é várias vezes maior. Um número fixo ou está
 * apertado demais pra um build ou frouxo demais pro outro, e o jeito de
 * escolher acaba sendo "o valor que faz o teste calar".
 *
 * O que decide é a FOLGA QUE SOBRA. Aqui se marca a base ao entrar e se mede a
 * distância no meio da recursão. É o que o CPython faz em `PyOS_CheckStack` e
 * o SQLite no parser.
 *
 * Vive em módulo próprio porque três unidades precisam: o parser (descida
 * recursiva de expressão e bloco), a VM (impressão de estrutura aninhada) e o
 * teste de unidade, que o exercita direto.
 */
#ifndef PS_PILHA_H
#define PS_PILHA_H

#include <stddef.h>

/* Marca a base da pilha CORRENTE e quanto ela tem.
 *
 * Chame na entrada de cada pilha distinta: o `main` do processo e o trampolim
 * de cada fibra (que roda numa `mmap` de 128 KB, ~64x menor que os 8 MB do
 * processo). Marcar com a pilha ainda intocada é o que faz a conta valer. */
void ps_pilha_marca(size_t tam);

/* Como `ps_pilha_marca`, mas descobre o tamanho sozinho via
 * `getrlimit(RLIMIT_STACK)` — respeita `ulimit -s`. Para o `main`. */
void ps_pilha_marca_processo(void);

/* Lê e repõe a marcação. Existe pro contexto que TROCA de pilha (ucontext):
 * quem entra numa fibra salva o par de fora, põe o da fibra, e repõe na volta.
 * Sem isso a medição compara endereços de regiões sem relação e responde
 * qualquer coisa. */
void ps_pilha_le(const char **base, size_t *tam);
void ps_pilha_repoe(const char *base, size_t tam);

/* 1 = está perto do fim da pilha; PARE de descer. 0 = pode continuar (e também
 * quando ninguém marcou nada, porque aí não há o que medir).
 *
 * A margem é PROPORCIONAL ao tamanho (tam/8, entre 16 KB e 512 KB). Margem
 * fixa não serve: o utilizável abaixo da base é menor que o `ulimit`, porque o
 * bloco de ambiente, o argv e a inicialização da libc ficam acima dela — com
 * 24 KB fixos a recursão ainda estourava numa pilha de 8 MB. */
int ps_pilha_apertada(void);

#endif /* PS_PILHA_H */
