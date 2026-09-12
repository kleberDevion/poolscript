/*
 * Leitura e escrita de .xlsx em C — sem libzip.
 *
 * xlsx é um ZIP de XMLs (OOXML). libzip não tem `.a` aqui, então o container
 * ZIP é feito à mão sobre a zlib (que já entra estática pelo PNG); o XML das
 * planilhas é lido com expat e escrito na mão. Não é sensível como TLS — é só
 * formato de arquivo —, então cabe reescrever, como o QR e o HTTP.
 *
 * O modelo é o que a manpu usa: uma matriz de células (linhas × colunas) de
 * strings. Número vira texto na leitura (data_only), célula vazia = "".
 */
#ifndef PS_XLSX_H
#define PS_XLSX_H

#include <stddef.h>

typedef struct {
    char ***celulas;   /* [linha][coluna] -> string (malloc) ou NULL */
    char  **tipos;     /* [linha][coluna] -> 'n' numero, 's' string, 0 vazio */
    int    *ncols;     /* colunas por linha */
    int     nlin;
    int     cap;
} PSGrade;

/* Lê a primeira planilha de `caminho`. Devolve 0 e preenche `g`, ou -1 com a
 * mensagem em `erro`. */
int ps_xlsx_le(const char *caminho, PSGrade *g, char *erro, size_t ecap);

/* Escreve `g` como .xlsx em `caminho`. 0/-1. */
int ps_xlsx_escreve(const char *caminho, const PSGrade *g, char *erro, size_t ecap);

void ps_grade_init(PSGrade *g);
void ps_grade_libera(PSGrade *g);
/* Garante que a célula (lin,col) existe (expande com ""), e grava `valor`
 * (copiado) com o tipo dado ('n'/'s'/0). 0/-1. */
int  ps_grade_set(PSGrade *g, int lin, int col, const char *valor, char tipo);
/* Tipo da célula: 'n' numero, 's' string, 0 vazio/ausente. */
char ps_grade_tipo(const PSGrade *g, int lin, int col);
/* Célula (lin,col) ou "" se fora da faixa. */
const char *ps_grade_get(const PSGrade *g, int lin, int col);

#endif /* PS_XLSX_H */
