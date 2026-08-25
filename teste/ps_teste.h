/*
 * Suíte de testes da PoolScript — em C, sem Python.
 *
 * Cada caso é um programa `.ps` + o que se espera dele. O runner NÃO executa
 * o programa dentro deste processo: ele faz fork/exec do `./pool`. Isso é
 * deliberado — boa parte dos casos existe justamente porque a VM já MORREU
 * (segfault, SIGFPE, OOM) rodando aquele fonte, e um teste que morre junto
 * não relata nada. Com subprocesso, morte vira resultado: `WIFSIGNALED`.
 *
 * Um caso passa quando bate TUDO o que ele declara: saída, código de saída e
 * trecho do erro. Campo NULL/-1 = "não confiro isso".
 */
#ifndef PS_TESTE_H
#define PS_TESTE_H

#include <stddef.h>   /* NULL nas tabelas de caso */

typedef struct {
    const char *nome;    /* identificação curta, usada no filtro e no relato */
    const char *fonte;   /* o programa .ps */
    const char *saida;   /* stdout esperado, EXATO (NULL = não confere) */
    const char *erro;    /* trecho que precisa aparecer no stderr
                          * (NULL = o programa tem que terminar sem erro) */
    int         rc;      /* código de saída esperado (-1 = não confere) */
    /* Nome FIXO do arquivo em /tmp (com extensão), quando o próprio fonte
     * precisa saber como se chama — é o caso do `import` de si mesmo. NULL =
     * nome aleatório do mkstemp, que é o normal. */
    const char *arquivo;
} Caso;

typedef struct {
    const char *grupo;
    const Caso *casos;
    int         n;
} Grupo;

#define N_CASOS(v) ((int)(sizeof(v) / sizeof((v)[0])))

extern const Caso CASOS_CRASH[];        extern const int NC_CRASH;
extern const Caso CASOS_INTEIROS[];     extern const int NC_INTEIROS;
extern const Caso CASOS_ERROS[];        extern const int NC_ERROS;
extern const Caso CASOS_LINGUAGEM[];    extern const int NC_LINGUAGEM;
extern const Caso CASOS_PENDENTES[];    extern const int NC_PENDENTES;
extern const Caso CASOS_COBERTURA[];    extern const int NC_COBERTURA;
extern const Caso CASOS_DIFERENCIAL[];  extern const int NC_DIFERENCIAL;
extern const Caso CASOS_EQUIVALENCIA[]; extern const int NC_EQUIVALENCIA;
extern const Caso CASOS_ORACULO[];      extern const int NC_ORACULO;
extern const Caso CASOS_ROBUSTEZ[];     extern const int NC_ROBUSTEZ;

#endif /* PS_TESTE_H */
