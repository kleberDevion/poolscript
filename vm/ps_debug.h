/*
 * Transporte do Debug Adapter Protocol (DAP).
 *
 * Só o enquadramento: ler e escrever mensagens `Content-Length: N\r\n\r\nJSON`
 * num descritor. Nada aqui sabe o que é `Value`, `VM` ou breakpoint — igual ao
 * `ps_jinker.c`, que fala HTTP sem conhecer a linguagem. A cola com a VM (os
 * frames, as variáveis, o passo) mora no `poolscript_vm.c`, que é quem enxerga
 * as estruturas internas.
 *
 * O DAP é o mesmo protocolo que o VS Code usa pra qualquer depurador; falar ele
 * direto do motor é o que evita reimplementar o depurador em JavaScript, do
 * mesmo jeito que o LSP já é servido pelo motor e não pela extensão.
 */
#ifndef PS_DEBUG_H
#define PS_DEBUG_H

#include <stddef.h>

/* Lê UMA mensagem. Devolve o corpo JSON em `*corpo` (malloc do chamador, com
 * '\0' no fim) e o tamanho em `*tam`.
 *   0  = leu
 *  -1  = fim do fluxo (o editor fechou) ou erro irrecuperável
 * Cabeçalho que não seja `Content-Length` é ignorado, como manda o protocolo. */
int psdbg_le(int fd, char **corpo, size_t *tam);

/* Escreve UMA mensagem já serializada, pondo o cabeçalho. Devolve 0 ou -1.
 * Escrita parcial é retomada: em pipe, `write` pode levar menos do que se pede. */
int psdbg_escreve(int fd, const char *json, size_t tam);

#endif
