/*
 * Versão da linguagem — FONTE ÚNICA.
 *
 * Já foi um arquivo de texto que o Makefile raspava com `sed`.
 * Com o interpretador fora, isso virou o que sempre deveria ter sido num
 * projeto C: uma constante de header, que o compilador enxerga e o `pool`
 * carrega. Nada de arquivo de texto raspado por shell.
 *
 * Regra de versionamento (base 100, sem semver): os dígitos 2 e 3 vão de 0 a
 * 99; ao chegar em 100 zeram e somam 1 no dígito à esquerda.
 *   8.2.83 -> 8.2.84 ... 8.2.99 -> 8.3.0 ... 8.99.99 -> 9.0.0
 */
#ifndef PS_VERSAO_H
#define PS_VERSAO_H

#define PS_VERSAO "15.90.16"

#endif /* PS_VERSAO_H */
