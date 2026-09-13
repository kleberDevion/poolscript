/*
 * Invariantes do motor — o build de asserção.
 *
 * POR QUE ISTO EXISTE: em 41 mil linhas havia 8 `_Static_assert` (7 no guzer,
 * 1 no X11) e 6 `assert()`, todos num arquivo só. Pilha, frames, GC e fibras
 * não tinham UMA invariante checada.
 *
 * A consequência é o que limita todo o resto do ferramental: ASan, fuzzer e
 * injeção de falha de alocação só acham MORTE, nunca ESTADO ERRADO. Um `sp`
 * desequilibrado, um operando de tipo impossível chegando num opcode, uma raiz
 * de GC esquecida — nada disso vira falha enquanto não virar segfault. Com
 * 41% de ramo coberto, há muito caminho onde não vira.
 *
 * É a diferença entre "não quebrou" e "as invariantes valeram". O SQLite roda
 * a suíte com `SQLITE_DEBUG` e sem.
 *
 *     make check          binário normal: PS_ASSERT some, custo zero
 *     make check-debug    a MESMA suíte com -DPS_DEBUG: invariantes valendo
 *
 * COMO USAR
 *
 *   PS_ASSERT(cond)              invariante interna. Falhar aborta com
 *                                arquivo:linha e a condição, e o runner da
 *                                suíte transforma isso em falha do caso (ele
 *                                roda cada caso em subprocesso justamente pra
 *                                morte virar resultado).
 *   PS_ASSERT_MSG(cond, fmt, …)  idem, dizendo o que se esperava.
 *
 * O QUE NÃO PÔR AQUI: validação de entrada do usuário. `PS_ASSERT` some no
 * build de produção — usar pra conferir dado que veio de fora é fazer a
 * checagem desaparecer justamente onde ela importa. Entrada errada é erro
 * levantado (`MERRO`/`BERRO`); invariante quebrada é defeito NOSSO, e é o que
 * vem pra cá.
 */
#ifndef PS_ASSERT_H
#define PS_ASSERT_H

#ifdef PS_DEBUG

#include <stdio.h>
#include <stdlib.h>

/* `abort()` e não `exit()`: deixa core, e o runner distingue morte por sinal
 * de saída com código. */
#define PS_ASSERT(cond)                                                       \
    do {                                                                      \
        if (!(cond)) {                                                        \
            fprintf(stderr, "\nINVARIANTE QUEBRADA: %s\n  em %s:%d (%s)\n",   \
                    #cond, __FILE__, __LINE__, __func__);                     \
            abort();                                                          \
        }                                                                     \
    } while (0)

#define PS_ASSERT_MSG(cond, ...)                                              \
    do {                                                                      \
        if (!(cond)) {                                                        \
            fprintf(stderr, "\nINVARIANTE QUEBRADA: %s\n  em %s:%d (%s)\n  ", \
                    #cond, __FILE__, __LINE__, __func__);                     \
            fprintf(stderr, __VA_ARGS__);                                     \
            fprintf(stderr, "\n");                                            \
            abort();                                                          \
        }                                                                     \
    } while (0)

#else

/* Some por completo, mas TUDO continua sendo compilado e conferido — condição
 * E mensagem. Sem isso, uma asserção que cita variável removida (ou um `%d`
 * recebendo ponteiro) só quebraria no build de debug, que é justamente o que
 * ninguém roda todo dia: a asserção apodrece calada e só reaparece no dia em
 * que alguém liga o `-DPS_DEBUG`.
 *
 * O `if (0)` mantém o type-check e o gerador de código descarta o bloco. O
 * `format(printf)` faz o compilador conferir o formato contra os argumentos
 * TAMBÉM no build normal. */
static inline void ps_assert_conf_fmt(const char *fmt, ...)
    __attribute__((format(printf, 1, 2)));
static inline void ps_assert_conf_fmt(const char *fmt, ...) { (void)fmt; }

#define PS_ASSERT(cond)                                                       \
    do { if (0) { (void)((cond) ? 1 : 0); } } while (0)

#define PS_ASSERT_MSG(cond, ...)                                              \
    do { if (0) { (void)((cond) ? 1 : 0); ps_assert_conf_fmt(__VA_ARGS__); } } while (0)

#endif /* PS_DEBUG */

#endif /* PS_ASSERT_H */
