#include <string.h>
#include <stdio.h>

#include "ps_hash.h"

/* ── SHA-256 (RFC 6234) ─────────────────────────────────────────────────── */

static const uint32_t K[64] = {
    0x428a2f98u,0x71374491u,0xb5c0fbcfu,0xe9b5dba5u,0x3956c25bu,0x59f111f1u,
    0x923f82a4u,0xab1c5ed5u,0xd807aa98u,0x12835b01u,0x243185beu,0x550c7dc3u,
    0x72be5d74u,0x80deb1feu,0x9bdc06a7u,0xc19bf174u,0xe49b69c1u,0xefbe4786u,
    0x0fc19dc6u,0x240ca1ccu,0x2de92c6fu,0x4a7484aau,0x5cb0a9dcu,0x76f988dau,
    0x983e5152u,0xa831c66du,0xb00327c8u,0xbf597fc7u,0xc6e00bf3u,0xd5a79147u,
    0x06ca6351u,0x14292967u,0x27b70a85u,0x2e1b2138u,0x4d2c6dfcu,0x53380d13u,
    0x650a7354u,0x766a0abbu,0x81c2c92eu,0x92722c85u,0xa2bfe8a1u,0xa81a664bu,
    0xc24b8b70u,0xc76c51a3u,0xd192e819u,0xd6990624u,0xf40e3585u,0x106aa070u,
    0x19a4c116u,0x1e376c08u,0x2748774cu,0x34b0bcb5u,0x391c0cb3u,0x4ed8aa4au,
    0x5b9cca4fu,0x682e6ff3u,0x748f82eeu,0x78a5636fu,0x84c87814u,0x8cc70208u,
    0x90befffau,0xa4506cebu,0xbef9a3f7u,0xc67178f2u
};

#define ROTR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))

typedef struct {
    uint32_t h[8];
    unsigned char buf[64];
    size_t nbuf;
    uint64_t total;      /* bits, é o que entra no padding */
} Sha256;

static void sha256_bloco(Sha256 *s, const unsigned char *b)
{
    uint32_t w[64];
    for (int i = 0; i < 16; i++)
        w[i] = ((uint32_t)b[i*4] << 24) | ((uint32_t)b[i*4+1] << 16)
             | ((uint32_t)b[i*4+2] << 8) | (uint32_t)b[i*4+3];
    for (int i = 16; i < 64; i++) {
        uint32_t s0 = ROTR(w[i-15], 7) ^ ROTR(w[i-15], 18) ^ (w[i-15] >> 3);
        uint32_t s1 = ROTR(w[i-2], 17) ^ ROTR(w[i-2], 19) ^ (w[i-2] >> 10);
        w[i] = w[i-16] + s0 + w[i-7] + s1;
    }
    uint32_t a = s->h[0], bb = s->h[1], c = s->h[2], d = s->h[3];
    uint32_t e = s->h[4], f = s->h[5], g = s->h[6], hh = s->h[7];
    for (int i = 0; i < 64; i++) {
        uint32_t S1 = ROTR(e, 6) ^ ROTR(e, 11) ^ ROTR(e, 25);
        uint32_t ch = (e & f) ^ ((~e) & g);
        uint32_t t1 = hh + S1 + ch + K[i] + w[i];
        uint32_t S0 = ROTR(a, 2) ^ ROTR(a, 13) ^ ROTR(a, 22);
        uint32_t maj = (a & bb) ^ (a & c) ^ (bb & c);
        uint32_t t2 = S0 + maj;
        hh = g; g = f; f = e; e = d + t1;
        d = c; c = bb; bb = a; a = t1 + t2;
    }
    s->h[0] += a; s->h[1] += bb; s->h[2] += c; s->h[3] += d;
    s->h[4] += e; s->h[5] += f;  s->h[6] += g; s->h[7] += hh;
}

static void sha256_init(Sha256 *s)
{
    s->h[0] = 0x6a09e667u; s->h[1] = 0xbb67ae85u;
    s->h[2] = 0x3c6ef372u; s->h[3] = 0xa54ff53au;
    s->h[4] = 0x510e527fu; s->h[5] = 0x9b05688cu;
    s->h[6] = 0x1f83d9abu; s->h[7] = 0x5be0cd19u;
    s->nbuf = 0;
    s->total = 0;
}

static void sha256_update(Sha256 *s, const unsigned char *d, size_t n)
{
    s->total += (uint64_t)n * 8;
    while (n > 0) {
        size_t cabe = 64 - s->nbuf;
        size_t leva = n < cabe ? n : cabe;
        memcpy(s->buf + s->nbuf, d, leva);
        s->nbuf += leva; d += leva; n -= leva;
        if (s->nbuf == 64) { sha256_bloco(s, s->buf); s->nbuf = 0; }
    }
}

static void sha256_final(Sha256 *s, unsigned char saida[PS_SHA256_TAM])
{
    uint64_t bits = s->total;
    unsigned char um = 0x80;
    sha256_update(s, &um, 1);
    s->total = bits;                      /* o padding não conta no tamanho */
    unsigned char zero = 0;
    while (s->nbuf != 56) { sha256_update(s, &zero, 1); s->total = bits; }
    unsigned char tam[8];
    for (int i = 0; i < 8; i++) tam[i] = (unsigned char)(bits >> (56 - i * 8));
    sha256_update(s, tam, 8);
    for (int i = 0; i < 8; i++) {
        saida[i*4]     = (unsigned char)(s->h[i] >> 24);
        saida[i*4 + 1] = (unsigned char)(s->h[i] >> 16);
        saida[i*4 + 2] = (unsigned char)(s->h[i] >> 8);
        saida[i*4 + 3] = (unsigned char)(s->h[i]);
    }
}

void ps_sha256(const unsigned char *dados, size_t n, unsigned char saida[PS_SHA256_TAM])
{
    Sha256 s;
    sha256_init(&s);
    sha256_update(&s, dados, n);
    sha256_final(&s, saida);
}

/* ── HMAC (RFC 2104) ────────────────────────────────────────────────────── */
void ps_hmac_sha256(const unsigned char *chave, size_t nchave,
                    const unsigned char *msg, size_t nmsg,
                    unsigned char saida[PS_SHA256_TAM])
{
    unsigned char k[64];
    memset(k, 0, sizeof(k));
    /* chave maior que o bloco vira o hash dela — é a regra da RFC, e sem ela
     * chaves longas dariam um HMAC que nenhuma outra implementação valida */
    if (nchave > 64) ps_sha256(chave, nchave, k);
    else memcpy(k, chave, nchave);

    unsigned char ipad[64], opad[64];
    for (int i = 0; i < 64; i++) { ipad[i] = k[i] ^ 0x36; opad[i] = k[i] ^ 0x5c; }

    Sha256 s;
    unsigned char interno[PS_SHA256_TAM];
    sha256_init(&s);
    sha256_update(&s, ipad, 64);
    sha256_update(&s, msg, nmsg);
    sha256_final(&s, interno);

    sha256_init(&s);
    sha256_update(&s, opad, 64);
    sha256_update(&s, interno, PS_SHA256_TAM);
    sha256_final(&s, saida);
}


/* ── SHA-512 e SHA-384 (RFC 6234) ───────────────────────────────────────── */
/* Mesma estrutura do SHA-256 com palavra de 64 bits, 80 rodadas e outras
 * constantes. SHA-384 é o MESMO algoritmo com IV diferente, truncado em 48
 * bytes — não é um hash separado. */

static const uint64_t K5[80] = {
    0x428a2f98d728ae22ULL,0x7137449123ef65cdULL,0xb5c0fbcfec4d3b2fULL,0xe9b5dba58189dbbcULL,
    0x3956c25bf348b538ULL,0x59f111f1b605d019ULL,0x923f82a4af194f9bULL,0xab1c5ed5da6d8118ULL,
    0xd807aa98a3030242ULL,0x12835b0145706fbeULL,0x243185be4ee4b28cULL,0x550c7dc3d5ffb4e2ULL,
    0x72be5d74f27b896fULL,0x80deb1fe3b1696b1ULL,0x9bdc06a725c71235ULL,0xc19bf174cf692694ULL,
    0xe49b69c19ef14ad2ULL,0xefbe4786384f25e3ULL,0x0fc19dc68b8cd5b5ULL,0x240ca1cc77ac9c65ULL,
    0x2de92c6f592b0275ULL,0x4a7484aa6ea6e483ULL,0x5cb0a9dcbd41fbd4ULL,0x76f988da831153b5ULL,
    0x983e5152ee66dfabULL,0xa831c66d2db43210ULL,0xb00327c898fb213fULL,0xbf597fc7beef0ee4ULL,
    0xc6e00bf33da88fc2ULL,0xd5a79147930aa725ULL,0x06ca6351e003826fULL,0x142929670a0e6e70ULL,
    0x27b70a8546d22ffcULL,0x2e1b21385c26c926ULL,0x4d2c6dfc5ac42aedULL,0x53380d139d95b3dfULL,
    0x650a73548baf63deULL,0x766a0abb3c77b2a8ULL,0x81c2c92e47edaee6ULL,0x92722c851482353bULL,
    0xa2bfe8a14cf10364ULL,0xa81a664bbc423001ULL,0xc24b8b70d0f89791ULL,0xc76c51a30654be30ULL,
    0xd192e819d6ef5218ULL,0xd69906245565a910ULL,0xf40e35855771202aULL,0x106aa07032bbd1b8ULL,
    0x19a4c116b8d2d0c8ULL,0x1e376c085141ab53ULL,0x2748774cdf8eeb99ULL,0x34b0bcb5e19b48a8ULL,
    0x391c0cb3c5c95a63ULL,0x4ed8aa4ae3418acbULL,0x5b9cca4f7763e373ULL,0x682e6ff3d6b2b8a3ULL,
    0x748f82ee5defb2fcULL,0x78a5636f43172f60ULL,0x84c87814a1f0ab72ULL,0x8cc702081a6439ecULL,
    0x90befffa23631e28ULL,0xa4506cebde82bde9ULL,0xbef9a3f7b2c67915ULL,0xc67178f2e372532bULL,
    0xca273eceea26619cULL,0xd186b8c721c0c207ULL,0xeada7dd6cde0eb1eULL,0xf57d4f7fee6ed178ULL,
    0x06f067aa72176fbaULL,0x0a637dc5a2c898a6ULL,0x113f9804bef90daeULL,0x1b710b35131c471bULL,
    0x28db77f523047d84ULL,0x32caab7b40c72493ULL,0x3c9ebe0a15c9bebcULL,0x431d67c49c100d4cULL,
    0x4cc5d4becb3e42b6ULL,0x597f299cfc657e2aULL,0x5fcb6fab3ad6faecULL,0x6c44198c4a475817ULL
};

#define ROTR64(x, n) (((x) >> (n)) | ((x) << (64 - (n))))

typedef struct {
    uint64_t h[8];
    unsigned char buf[128];
    size_t nbuf;
    uint64_t total;      /* bits; 128 bits na RFC, mas 64 cobre 2 exabytes */
} Sha512;

static void sha512_bloco(Sha512 *s, const unsigned char *b)
{
    uint64_t w[80];
    for (int i = 0; i < 16; i++) {
        w[i] = 0;
        for (int k = 0; k < 8; k++) w[i] = (w[i] << 8) | b[i*8 + k];
    }
    for (int i = 16; i < 80; i++) {
        uint64_t s0 = ROTR64(w[i-15],1) ^ ROTR64(w[i-15],8) ^ (w[i-15] >> 7);
        uint64_t s1 = ROTR64(w[i-2],19) ^ ROTR64(w[i-2],61) ^ (w[i-2] >> 6);
        w[i] = w[i-16] + s0 + w[i-7] + s1;
    }
    uint64_t a=s->h[0],bb=s->h[1],c=s->h[2],d=s->h[3];
    uint64_t e=s->h[4],f=s->h[5],g=s->h[6],hh=s->h[7];
    for (int i = 0; i < 80; i++) {
        uint64_t S1 = ROTR64(e,14) ^ ROTR64(e,18) ^ ROTR64(e,41);
        uint64_t ch = (e & f) ^ ((~e) & g);
        uint64_t t1 = hh + S1 + ch + K5[i] + w[i];
        uint64_t S0 = ROTR64(a,28) ^ ROTR64(a,34) ^ ROTR64(a,39);
        uint64_t maj = (a & bb) ^ (a & c) ^ (bb & c);
        uint64_t t2 = S0 + maj;
        hh=g; g=f; f=e; e=d+t1; d=c; c=bb; bb=a; a=t1+t2;
    }
    s->h[0]+=a; s->h[1]+=bb; s->h[2]+=c; s->h[3]+=d;
    s->h[4]+=e; s->h[5]+=f;  s->h[6]+=g; s->h[7]+=hh;
}

static void sha512_init(Sha512 *s, int tam)
{
    if (tam == PS_SHA384_TAM) {
        s->h[0]=0xcbbb9d5dc1059ed8ULL; s->h[1]=0x629a292a367cd507ULL;
        s->h[2]=0x9159015a3070dd17ULL; s->h[3]=0x152fecd8f70e5939ULL;
        s->h[4]=0x67332667ffc00b31ULL; s->h[5]=0x8eb44a8768581511ULL;
        s->h[6]=0xdb0c2e0d64f98fa7ULL; s->h[7]=0x47b5481dbefa4fa4ULL;
    } else {
        s->h[0]=0x6a09e667f3bcc908ULL; s->h[1]=0xbb67ae8584caa73bULL;
        s->h[2]=0x3c6ef372fe94f82bULL; s->h[3]=0xa54ff53a5f1d36f1ULL;
        s->h[4]=0x510e527fade682d1ULL; s->h[5]=0x9b05688c2b3e6c1fULL;
        s->h[6]=0x1f83d9abfb41bd6bULL; s->h[7]=0x5be0cd19137e2179ULL;
    }
    s->nbuf = 0;
    s->total = 0;
}

static void sha512_update(Sha512 *s, const unsigned char *d, size_t n)
{
    s->total += (uint64_t)n * 8;
    while (n > 0) {
        size_t cabe = 128 - s->nbuf;
        size_t leva = n < cabe ? n : cabe;
        memcpy(s->buf + s->nbuf, d, leva);
        s->nbuf += leva; d += leva; n -= leva;
        if (s->nbuf == 128) { sha512_bloco(s, s->buf); s->nbuf = 0; }
    }
}

static void sha512_final(Sha512 *s, unsigned char *saida, int tam)
{
    uint64_t bits = s->total;
    unsigned char um = 0x80, zero = 0;
    sha512_update(s, &um, 1); s->total = bits;
    while (s->nbuf != 112) { sha512_update(s, &zero, 1); s->total = bits; }
    unsigned char comp[16];
    memset(comp, 0, 8);                       /* os 64 bits altos: sempre 0 */
    for (int i = 0; i < 8; i++) comp[8 + i] = (unsigned char)(bits >> (56 - i * 8));
    sha512_update(s, comp, 16);
    for (int i = 0; i < 8 && i * 8 < tam; i++)
        for (int k = 0; k < 8 && i * 8 + k < tam; k++)
            saida[i * 8 + k] = (unsigned char)(s->h[i] >> (56 - k * 8));
}

void ps_sha512(const unsigned char *d, size_t n, unsigned char saida[PS_SHA512_TAM])
{
    Sha512 s; sha512_init(&s, PS_SHA512_TAM);
    sha512_update(&s, d, n); sha512_final(&s, saida, PS_SHA512_TAM);
}

void ps_sha384(const unsigned char *d, size_t n, unsigned char saida[PS_SHA384_TAM])
{
    Sha512 s; sha512_init(&s, PS_SHA384_TAM);
    sha512_update(&s, d, n); sha512_final(&s, saida, PS_SHA384_TAM);
}

/* HMAC genérico. O bloco é 64 no SHA-256 e 128 nos de 512 bits — usar o
 * errado dá um resultado que parece plausível e não bate com ninguém. */
size_t ps_hmac(int tam, const unsigned char *chave, size_t nchave,
               const unsigned char *msg, size_t nmsg, unsigned char saida[PS_HASH_MAX])
{
    if (tam == PS_SHA256_TAM) { ps_hmac_sha256(chave, nchave, msg, nmsg, saida); return PS_SHA256_TAM; }
    if (tam != PS_SHA384_TAM && tam != PS_SHA512_TAM) return 0;

    unsigned char k[128];
    memset(k, 0, sizeof(k));
    if (nchave > 128) {
        if (tam == PS_SHA384_TAM) ps_sha384(chave, nchave, k);
        else                      ps_sha512(chave, nchave, k);
    } else {
        memcpy(k, chave, nchave);
    }
    unsigned char ipad[128], opad[128];
    for (int i = 0; i < 128; i++) { ipad[i] = k[i] ^ 0x36; opad[i] = k[i] ^ 0x5c; }

    Sha512 s;
    unsigned char interno[PS_SHA512_TAM];
    sha512_init(&s, tam); sha512_update(&s, ipad, 128);
    sha512_update(&s, msg, nmsg); sha512_final(&s, interno, tam);
    sha512_init(&s, tam); sha512_update(&s, opad, 128);
    sha512_update(&s, interno, (size_t)tam); sha512_final(&s, saida, tam);
    return (size_t)tam;
}

/* ── PBKDF2 (RFC 8018) ──────────────────────────────────────────────────── */
/*
 * A senha é a mesma nas 310 mil iterações, então o estado do SHA depois de
 * absorver o ipad e o opad é sempre idêntico. Calcular uma vez e clonar corta
 * as compressões por iteração de 4 para 2 — o dobro de velocidade, sem mudar
 * um bit do resultado.
 */
typedef struct { Sha256 dentro, fora; } HmacPronto;

static void hmac_prepara(HmacPronto *hp, const unsigned char *chave, size_t nchave)
{
    unsigned char k[64];
    memset(k, 0, sizeof(k));
    if (nchave > 64) ps_sha256(chave, nchave, k);
    else memcpy(k, chave, nchave);

    unsigned char ipad[64], opad[64];
    for (int i = 0; i < 64; i++) { ipad[i] = k[i] ^ 0x36; opad[i] = k[i] ^ 0x5c; }
    sha256_init(&hp->dentro); sha256_update(&hp->dentro, ipad, 64);
    sha256_init(&hp->fora);   sha256_update(&hp->fora,   opad, 64);
}

static void hmac_uma(const HmacPronto *hp, const unsigned char *msg, size_t nmsg,
                     unsigned char saida[PS_SHA256_TAM])
{
    Sha256 s = hp->dentro;                    /* clona o estado pronto */
    unsigned char interno[PS_SHA256_TAM];
    sha256_update(&s, msg, nmsg);
    sha256_final(&s, interno);
    s = hp->fora;
    sha256_update(&s, interno, PS_SHA256_TAM);
    sha256_final(&s, saida);
}

void ps_pbkdf2_sha256(const unsigned char *senha, size_t nsenha,
                      const unsigned char *sal, size_t nsal,
                      uint32_t iteracoes, unsigned char saida[PS_SHA256_TAM])
{
    /* Um bloco só: dkLen = hLen = 32, então o contador é sempre 1. */
    unsigned char bloco[512];
    size_t n = nsal;
    if (n > sizeof(bloco) - 4) n = sizeof(bloco) - 4;
    memcpy(bloco, sal, n);
    bloco[n]     = 0; bloco[n + 1] = 0;
    bloco[n + 2] = 0; bloco[n + 3] = 1;      /* INT(1), big-endian */

    HmacPronto hp;
    hmac_prepara(&hp, senha, nsenha);

    unsigned char u[PS_SHA256_TAM], acc[PS_SHA256_TAM];
    hmac_uma(&hp, bloco, n + 4, u);
    memcpy(acc, u, PS_SHA256_TAM);
    for (uint32_t i = 1; i < iteracoes; i++) {
        hmac_uma(&hp, u, PS_SHA256_TAM, u);
        for (int k = 0; k < PS_SHA256_TAM; k++) acc[k] ^= u[k];
    }
    memcpy(saida, acc, PS_SHA256_TAM);
}

/* ── base64 (RFC 4648) ──────────────────────────────────────────────────── */
static const char B64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

size_t ps_base64_encode_ex(const unsigned char *d, size_t n, char *saida,
                           int urlsafe, int padding)
{
    const char c62 = urlsafe ? '-' : '+';
    const char c63 = urlsafe ? '_' : '/';
    size_t o = 0;
    size_t i = 0;
    for (; i + 2 < n; i += 3) {
        uint32_t v = ((uint32_t)d[i] << 16) | ((uint32_t)d[i+1] << 8) | d[i+2];
        saida[o++] = B64[(v >> 18) & 63];
        saida[o++] = B64[(v >> 12) & 63];
        saida[o++] = B64[(v >> 6) & 63];
        saida[o++] = B64[v & 63];
    }
    for (size_t k = 0; k < o; k++) {
        if (saida[k] == '+') saida[k] = c62;
        else if (saida[k] == '/') saida[k] = c63;
    }
    if (i < n) {
        uint32_t v = (uint32_t)d[i] << 16;
        int sobra = 1;
        if (i + 1 < n) { v |= (uint32_t)d[i+1] << 8; sobra = 2; }
        char a = B64[(v >> 18) & 63], b = B64[(v >> 12) & 63];
        saida[o++] = a == '+' ? c62 : (a == '/' ? c63 : a);
        saida[o++] = b == '+' ? c62 : (b == '/' ? c63 : b);
        if (sobra == 2) {
            char cc = B64[(v >> 6) & 63];
            saida[o++] = cc == '+' ? c62 : (cc == '/' ? c63 : cc);
        } else if (padding) {
            saida[o++] = '=';
        }
        if (padding) saida[o++] = '=';
    }
    saida[o] = '\0';
    return o;
}

size_t ps_base64_encode(const unsigned char *d, size_t n, char *saida)
{
    return ps_base64_encode_ex(d, n, saida, 0, 1);
}

static int b64_valor(char c)
{
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    /* aceita as duas variantes na entrada: quem decodifica não deveria
     * precisar saber qual alfabeto o produtor usou */
    if (c == '+' || c == '-') return 62;
    if (c == '/' || c == '_') return 63;
    return -1;
}

/* Decodifica base64 / base64url. Devolve quantos bytes saíram, ou -1.
 *
 * O PADDING é opcional — JWT usa base64url SEM padding, e recusar isso
 * quebraria todo token. Mas quando ele existe tem que estar no FIM e na
 * quantidade exata. Antes, um `=` simplesmente encerrava o laço e a função
 * devolvia o que tinha decodificado até ali: `Zg=`, `Zg===`, `=Zm9v` e
 * `Zm==9v` todos entregavam DADO PARCIAL como se fossem válidos. Não fura o
 * JWT (a verificação exige tamanho exato de assinatura), mas os outros
 * consumidores ficavam com um contrato que não recusa nada.
 *
 * A regra: `n % 4 == 1` não existe em base64 nenhum — 6 bits não formam byte.
 * Com padding, o total tem que fechar múltiplo de 4. */
long ps_base64_decode(const char *texto, size_t n, unsigned char *saida, size_t cap)
{
    uint32_t acc = 0;
    int bits = 0;
    size_t o = 0;
    size_t nsig = 0;      /* caracteres de dado (sem padding nem espaço) */
    size_t npad = 0;
    for (size_t i = 0; i < n; i++) {
        char c = texto[i];
        if (c == '\n' || c == '\r' || c == ' ' || c == '\t') continue;
        if (c == '=') { npad++; continue; }
        /* dado DEPOIS do padding é o `Zm==9v`: o `=` não é separador, é fim */
        if (npad) return -1;
        int v = b64_valor(c);
        if (v < 0) return -1;
        nsig++;
        acc = (acc << 6) | (uint32_t)v;
        bits += 6;
        if (bits >= 8) {
            bits -= 8;
            if (o >= cap) return -1;
            saida[o++] = (unsigned char)((acc >> bits) & 0xFF);
        }
    }
    if (nsig % 4 == 1) return -1;            /* 6 bits soltos: impossível */
    if (npad) {
        size_t esperado = (4 - nsig % 4) % 4;
        if (npad != esperado) return -1;     /* `Zg=`, `Zg===`, `=Zm9v` */
    }
    return (long)o;
}

int ps_iguais_constante(const unsigned char *a, const unsigned char *b, size_t n)
{
    unsigned char dif = 0;
    for (size_t i = 0; i < n; i++) dif |= (unsigned char)(a[i] ^ b[i]);
    return dif == 0;
}

int ps_random_bytes(unsigned char *saida, size_t n)
{
    /* `/dev/urandom` em vez de `getrandom()` por portabilidade: o mesmo
     * código serve em BSD e macOS sem #ifdef de sistema. */
    FILE *f = fopen("/dev/urandom", "rb");
    if (!f) return -1;
    size_t lidos = fread(saida, 1, n, f);
    fclose(f);
    return lidos == n ? 0 : -1;
}
