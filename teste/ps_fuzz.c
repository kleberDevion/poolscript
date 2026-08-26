/*
 * Fuzzer do front-end (lexer + parser + compilador), sobre libFuzzer.
 *
 * POR QUE: a suíte é 100% de entradas FIXAS. Nada gera entrada nova, e um
 * compilador em C sem fuzzer no parser é o caso de manual — é o primeiro item
 * da "ordem de retorno" da auditoria de testes.
 *
 * O harness certo já existia e é o `pool --check`: `ps_verifica_fonte()`
 * analisa o fonte inteiro (lexer -> parser -> compilador) e NÃO executa nada.
 * A libFuzzer chama isto milhões de vezes com entradas que ela muta, guiada
 * por COBERTURA — ela enxerga quais ramos cada entrada alcançou e prioriza as
 * que abrem caminho novo. É o que separa isto de jogar bytes aleatórios.
 *
 * O contrato é o mesmo do OOM: fonte nenhum, por mais torto, pode causar
 * segfault, estouro de buffer, laço infinito ou vazamento. Erro de sintaxe é
 * RESPOSTA CERTA, não falha — o que se caça é a morte violenta.
 *
 *     make fuzz               # constrói e roda com o corpus semeado
 *     make fuzz FUZZ_T=600    # 10 minutos
 *
 * Achado vira arquivo `crash-*`, que É o caso de regressão — mesmo modelo do
 * `testdata/fuzz` do Go. Reproduza com:  ./pool-fuzz <arquivo>
 */
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ps_vm.h"

/* Fonte grande demais só gasta tempo do fuzzer sem abrir ramo novo. */
#define MAX_FONTE (64 * 1024)

int LLVMFuzzerTestOneInput(const uint8_t *dados, size_t n)
{
    if (n == 0 || n > MAX_FONTE) return 0;

    /* Terminado em NUL, como o `--check` entrega. Um NUL NO MEIO é entrada
     * legítima de fuzz — o lexer tem que lidar com ela sozinho. */
    char *fonte = malloc(n + 1);
    if (!fonte) return 0;
    memcpy(fonte, dados, n);
    fonte[n] = '\0';

    PSErroExec e;
    memset(&e, 0, sizeof e);
    ps_verifica_fonte(fonte, n, "fuzz.ps", &e);   /* erro é resposta, não falha */

    free(fonte);
    return 0;
}
