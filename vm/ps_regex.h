/*
 * Motor de expressão regular da PoolScript, em C puro.
 *
 * Sem `Python.h` e sem dependência da VM — recebe e devolve buffers de bytes.
 * É o que permite `regex_lib`, `parsing_lib` e os métodos `match`/`findall`/
 * `sub` de string saírem do Python.
 *
 * Backtracking recursivo, não autômato. A escolha é deliberada: o `re` do
 * Python também é backtracking, e reproduzir a semântica dele (ganância,
 * ordem de alternativas, quais grupos ficam preenchidos) é o requisito —
 * um autômato daria outra resposta em casos como `(a|ab)c`.
 *
 * Suporta: classes, quantificadores (gulosos/preguiçosos), alternância,
 * grupos, âncoras `^ $ \A \Z \b \B`, `\d \w \s` e negações, retrovisor
 * (`\1`..`\9`), grupo nomeado `(?P<n>...)` (tratado como numerado), flags
 * inline global e com escopo `(?i)`/`(?m)`/`(?s)`/`(?i:...)`, e
 * lookahead/lookbehind `(?=)`/`(?!)`/`(?<=)`/`(?<!)` (lookbehind de largura
 * fixa, como o `re`). IGNORECASE dobra ASCII + Latin-1 (café/CAFÉ); outros
 * scripts (grego, cirílico) não dobram.
 *
 * O que NÃO suporta, e para com erro em vez de errar em silêncio: grupo
 * atômico `(?>...)`, condicional `(?(id)...)`, comentário `(?#...)`, e
 * `\D`/`\W`/`\S` negado DENTRO de `[...]`.
 */
#ifndef PS_REGEX_H
#define PS_REGEX_H

#define RX_MAX_GRUPOS 32

typedef struct PSRegex PSRegex;

/* Posições de início/fim de cada grupo, em BYTES. -1 = não participou. */
typedef struct {
    int inicio[RX_MAX_GRUPOS];
    int fim[RX_MAX_GRUPOS];
    int ngrupos;              /* quantos grupos capturantes o padrão tem */
} RxCaptura;

/* Compila. Devolve NULL e escreve em `erro` se o padrão for inválido ou usar
 * construção não suportada. */
PSRegex *ps_regex_compila(const char *padrao, int len, char *erro, int erro_cap);
void     ps_regex_free(PSRegex *r);

/* Procura a partir de `de`. Devolve 1 se achou (preenchendo `cap`, com o
 * grupo 0 = casamento inteiro), 0 se não achou, -1 em estouro de
 * backtracking. */
int ps_regex_busca(PSRegex *r, const char *s, int len, int de, RxCaptura *cap);

/* Casa a string INTEIRA (equivalente ao `fullmatch`). */
int ps_regex_casa_tudo(PSRegex *r, const char *s, int len, RxCaptura *cap);

int ps_regex_ngrupos(const PSRegex *r);

#endif /* PS_REGEX_H */
