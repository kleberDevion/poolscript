/* O endereço de cada função da biblioteca padrão do C como o compilador liga
 * em C11 estrito (`-std=c11`, sem as extensões do sistema). O teste de
 * unidade (teste_c_tabela, em unidade.c) compara cada um com o que o motor
 * acha pelo nome: na glibc, o `scanf` pelo nome é o da GNU antiga, e o que o
 * C11 liga é o `__isoc99_scanf`.
 *
 * Compilado à parte, com `-std=c11` (ver o alvo `unidade` do Makefile). A
 * lista é gerada de vm/c_padrao.decl por scripts/gera_c_padrao.pr. */
#include <ctype.h>
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <wchar.h>

/* O macOS marca o `sprintf` como obsoleto. Ele é do padrão, e o teste
 * precisa do endereço dele. */
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

typedef struct { const char *nome; void (*f)(void); } PSCEnderecoC11;

const PSCEnderecoC11 ps_c11_enderecos[] = {
#include "c_padrao_enderecos.inc"
};
const int ps_c11_n = (int)(sizeof ps_c11_enderecos / sizeof ps_c11_enderecos[0]);
