/*
 * SHA-256, HMAC, PBKDF2 e base64 em C puro — sem OpenSSL, sem `Python.h`.
 *
 * O formato segue os padrões (hashlib/HMAC): um hash gerado
 * pelo interpretador precisa validar no binário e vice-versa. Por isso nada
 * aqui é "uma variação minha" — é RFC 6234 (SHA-2), RFC 2104 (HMAC),
 * RFC 8018 (PBKDF2) e RFC 4648 (base64), exatamente.
 */
#ifndef PS_HASH_H
#define PS_HASH_H

#include <stddef.h>
#include <stdint.h>

#define PS_SHA256_TAM 32
#define PS_SHA384_TAM 48
#define PS_SHA512_TAM 64
#define PS_HASH_MAX   64

void ps_sha256(const unsigned char *dados, size_t n, unsigned char saida[PS_SHA256_TAM]);
void ps_sha384(const unsigned char *dados, size_t n, unsigned char saida[PS_SHA384_TAM]);
void ps_sha512(const unsigned char *dados, size_t n, unsigned char saida[PS_SHA512_TAM]);

void ps_hmac_sha256(const unsigned char *chave, size_t nchave,
                    const unsigned char *msg, size_t nmsg,
                    unsigned char saida[PS_SHA256_TAM]);

/* HMAC com a variante escolhida pelo tamanho do digest: 32, 48 ou 64.
 * Devolve o tamanho, ou 0 se `tam` não for um dos três. */
size_t ps_hmac(int tam_digest,
               const unsigned char *chave, size_t nchave,
               const unsigned char *msg, size_t nmsg,
               unsigned char saida[PS_HASH_MAX]);

/* PBKDF2-HMAC-SHA256 com saída de exatamente 32 bytes (dkLen = hLen). */
void ps_pbkdf2_sha256(const unsigned char *senha, size_t nsenha,
                      const unsigned char *sal, size_t nsal,
                      uint32_t iteracoes, unsigned char saida[PS_SHA256_TAM]);

/* `urlsafe` troca `+/` por `-_`; `padding` liga o `=` do fim. O JWT usa
 * urlsafe SEM padding, e a assinatura é calculada sobre esses bytes — errar
 * qualquer um dos dois gera token que não valida em lugar nenhum. */
size_t ps_base64_encode_ex(const unsigned char *dados, size_t n, char *saida,
                           int urlsafe, int padding);
size_t ps_base64_encode(const unsigned char *dados, size_t n, char *saida);

/* Aceita as duas variantes e padding ausente. -1 se inválido. */
long ps_base64_decode(const char *texto, size_t n, unsigned char *saida, size_t cap);

/* Compara em tempo constante — comparar hash com `memcmp` vaza, pelo tempo,
 * quantos bytes iniciais bateram. */
int ps_iguais_constante(const unsigned char *a, const unsigned char *b, size_t n);

/* Bytes aleatórios do sistema. 0 em sucesso. */
int ps_random_bytes(unsigned char *saida, size_t n);

#endif /* PS_HASH_H */
