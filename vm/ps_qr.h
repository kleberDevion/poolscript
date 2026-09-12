/*
 * Gerador de QR Code em C — ISO/IEC 18004, sem libqrencode.
 *
 * libqrencode não tem `.a` nesta máquina (só `.so`), e o encoder de QR não é
 * sensível como TLS — então é escrito à mão, como o HTTP foi. Modo byte,
 * versões 1..40, níveis L/M/Q/H, seleção automática de versão e de máscara
 * (pelas 4 regras de penalidade do padrão). A saída é a matriz do padrão
 * ISO/IEC 18004.
 *
 * O PNG sai pela libpng (essa tem `.a`, entra estática).
 */
#ifndef PS_QR_H
#define PS_QR_H

#include <stddef.h>
#include <stdint.h>

/* Preenche `*grid` (malloc, dim*dim bytes 0/1) com a matriz final — já com
 * máscara e format/version info. `nivel` é 'L'/'M'/'Q'/'H'. Devolve 0, ou -1
 * com a mensagem em `erro` (dados grandes demais pro maior QR, etc.). */
int ps_qr_matriz(const char *dados, int ndados, char nivel,
                 uint8_t **grid, int *dim, char *erro, size_t ecap);

/* Renderiza a matriz como PNG (P&G escalado por `box`, com `border` módulos de
 * quiet zone). `*png` é malloc. Devolve 0 ou -1. */
/* `tw`/`th` > 0 redimensionam a saída pra exatamente tw×th (nearest), como o
 * qr32 do gen(); 0 usa o tamanho natural (mod*box). */
int ps_qr_png(const uint8_t *grid, int dim, int box, int border,
              const char *cor, const char *fundo, int tw, int th,
              unsigned char **png, size_t *npng, char *erro, size_t ecap);

#endif /* PS_QR_H */
