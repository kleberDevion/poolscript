/*
 * Propriedades de caractere pela tabela do Unicode (UCD), pros métodos de
 * texto: caixa (upper, lower, title, capitalize, swapcase, casefold) e as
 * classes (isalpha, isdigit, isdecimal, isnumeric, isspace, isprintable,
 * isupper, islower, istitle).
 *
 * As tabelas vêm de ps_unicode_tab.inc, gerado por scripts/gera_unicode.pr a
 * partir dos arquivos oficiais do Unicode. Nome no código (variável, funct,
 * classe) NÃO passa por aqui: a regra dele é a do lexer (doc 1.4).
 */
#ifndef PS_UNICODE_H
#define PS_UNICODE_H

#include <stdint.h>

/* categoria geral: os 5 bits de baixo das propriedades (a ordem é a do
 * gerador) */
enum {
    UNI_CN = 0, UNI_LU, UNI_LL, UNI_LT, UNI_LM, UNI_LO, UNI_MN, UNI_MC, UNI_ME,
    UNI_ND, UNI_NL, UNI_NO, UNI_PC, UNI_PD, UNI_PS, UNI_PE, UNI_PI, UNI_PF,
    UNI_PO, UNI_SM, UNI_SC, UNI_SK, UNI_SO, UNI_ZS, UNI_ZL, UNI_ZP, UNI_CC,
    UNI_CF, UNI_CS, UNI_CO
};
#define UNI_CATEGORIA     31
/* as marcas, das propriedades do Unicode de mesmo nome */
#define UNI_MINUSCULA     32     /* Lowercase */
#define UNI_MAIUSCULA     64     /* Uppercase */
#define UNI_TEM_CAIXA     128    /* Cased */
#define UNI_IGNORA_CAIXA  256    /* Case_Ignorable */
#define UNI_BRANCO        512    /* White_Space */
#define UNI_DECIMAL       1024   /* Numeric_Type=Decimal */
#define UNI_DIGITO        2048   /* Numeric_Type=Digit */
#define UNI_NUMERICO      4096   /* Numeric_Type=Numeric */

/* versão do UCD das tabelas ("15.1.0") */
const char *ps_uni_versao(void);

/* categoria geral e marcas de um codepoint (fora da faixa: 0 = Cn) */
int ps_uni_props(uint32_t c);

int ps_uni_eh_letra(uint32_t c);       /* categoria L*: isalpha */
int ps_uni_eh_decimal(uint32_t c);     /* isdecimal */
int ps_uni_eh_digito(uint32_t c);      /* decimal ou dígito (²): isdigit */
int ps_uni_eh_numerico(uint32_t c);    /* + fração, romano, numeral CJK: isnumeric */
int ps_uni_eh_branco(uint32_t c);      /* White_Space: isspace, split(), strip() */
int ps_uni_eh_imprimivel(uint32_t c);  /* nem C*, nem Z* (fora o espaço) */
int ps_uni_eh_maiuscula(uint32_t c);
int ps_uni_eh_minuscula(uint32_t c);
int ps_uni_eh_titulo(uint32_t c);      /* categoria Lt (ǅ) */
int ps_uni_tem_caixa(uint32_t c);
int ps_uni_ignora_caixa(uint32_t c);

/* caixa simples: um codepoint vira um codepoint */
uint32_t ps_uni_maiuscula(uint32_t c);
uint32_t ps_uni_minuscula(uint32_t c);
uint32_t ps_uni_titulo(uint32_t c);

/* caixa completa (com o SpecialCasing sem condição: ß -> SS, ﬁ -> FI,
 * İ -> i + ponto): escreve até 3 codepoints em `out` e devolve quantos.
 * `qual`: 0 maiúscula, 1 minúscula, 2 título. */
int ps_uni_caixa(uint32_t c, int qual, uint32_t out[3]);

/* casefold completo (CaseFolding C + F): até 3 codepoints */
int ps_uni_dobra(uint32_t c, uint32_t out[3]);

#endif
