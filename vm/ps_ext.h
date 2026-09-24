/*
 * A extensão de arquivo da linguagem, num lugar só.
 *
 * Era uma tripla (`.ps`, `.psl`, `.p`) copiada em SEIS pontos do motor — duas
 * vezes no `jinga_vm.c` (resolução de módulo e `spec_eh_caminho`), no
 * `ps_compiler.c` (nome ligado pelo import entre aspas), no `main.c` (`pool
 * build`) e duas no `ps_pkg.c` (nome do pacote e "é arquivo local?") —, cada
 * cópia com sua própria forma: vetor de strings num, cadeia de `strcmp` por
 * sufixo noutro, aritmética de tamanho no terceiro. Seis cópias da mesma
 * regra são cinco chances de uma envelhecer sozinha; e o `psl` já gravava
 * `.ps` cravado, ignorando as outras duas.
 *
 * Hoje a extensão é UMA: `.pr`. Quem precisa dela pergunta aqui.
 */
#ifndef PS_EXT_H
#define PS_EXT_H

#include <string.h>

/* A extensão dos arquivos da linguagem, com o ponto. */
#define PS_EXT      ".pr"
#define PS_EXT_TAM  3

/* As extensões que a linguagem usava antes. Não são aceitas em lugar nenhum —
 * existem só pra o motor RECONHECER o arquivo velho e dizer o conserto, em vez
 * de um "não achei" seco. */
#define PS_EXTS_VELHAS_N 3
static const char *const PS_EXTS_VELHAS[PS_EXTS_VELHAS_N] = { ".ps", ".psl", ".p" };

/* 1 = o nome termina na extensão da linguagem. */
static inline int ps_eh_fonte(const char *nome)
{
    size_t l = nome ? strlen(nome) : 0;
    return l > PS_EXT_TAM && strcmp(nome + l - PS_EXT_TAM, PS_EXT) == 0;
}

/* A extensão VELHA com que o nome termina, ou NULL. */
static inline const char *ps_ext_velha(const char *nome)
{
    size_t l = nome ? strlen(nome) : 0;
    for (int i = 0; i < PS_EXTS_VELHAS_N; i++) {
        size_t le = strlen(PS_EXTS_VELHAS[i]);
        if (l > le && strcmp(nome + l - le, PS_EXTS_VELHAS[i]) == 0) return PS_EXTS_VELHAS[i];
    }
    return NULL;
}

/* Tira a extensão da linguagem (ou uma das velhas) do fim de `out`, no lugar.
 * É o que dá o nome do módulo a partir do nome do arquivo. */
static inline void ps_tira_ext(char *out)
{
    char *ext = strrchr(out, '.');
    if (!ext || ext == out) return;
    if (strcmp(ext, PS_EXT) == 0 || ps_ext_velha(out)) *ext = '\0';
}

#endif /* PS_EXT_H */
