# `hash.b64encode(dado)` — texto em base64 padrão

Codifica o dado em **base64** (RFC 4648, alfabeto padrão, com padding). É o
jeito de enfiar conteúdo num lugar que só aceita texto: cabeçalho HTTP, JSON,
QR Code, corpo de e-mail.

```
hash.b64encode(dado) -> str
```

---

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `dado` | qualquer valor | — | posicional; o motor **não aceita por nome** |

Exatamente **um** argumento posicional. O valor é convertido com `str()` antes
de codificar, e o texto entra em **UTF-8** — `hash.b64encode(255)` codifica o
texto `"255"`.

---

## Retorno

**`str`** — o base64 com o **alfabeto padrão** `A`–`Z`, `a`–`z`, `0`–`9`, `+`
e `/`, e **sempre com o padding `=`** até fechar múltiplo de 4.

Não é a variante URL-safe: `+` e `/` saem de verdade quando os bits pedem —
`hash.b64encode(chr(65535))` é `"77+/"`. Se o destino é uma URL, troque você
mesmo ou use outro caminho.

O comprimento é `4 × arredonda_pra_cima(n / 3)`, onde `n` é o número de
**bytes UTF-8** da entrada. Entrada vazia devolve `""` (string vazia, não
`Null`).

```
import hash

hash.b64encode("abc")           # "YWJj"              (3 bytes -> 4 chars)
hash.b64encode("ab")            # "YWI="              (1 char de padding)
hash.b64encode("a")             # "YQ=="              (2 chars de padding)
hash.b64encode("")              # ""
hash.b64encode("ç")             # "w6c="              (2 bytes em UTF-8)
hash.b64encode("PoolScript")    # "UG9vbFNjcmlwdA=="  (10 bytes -> 16 chars)
```

---

## Erros

- **TypeError** — número de argumentos errado:
  `b64encode() takes exactly one argument (0 given)`, idem com 2 ou mais.
- **TypeError** — argumento por nome: `b64encode() takes no keyword arguments`.

Nenhum valor de entrada é recusado — tudo tem um `str()`.

---

## Bordas

- **`bytes` não vira conteúdo — vira representação.**
  `hash.b64encode(bytes.fromhex("616263"))` devolve `"YidhYmMn"`, que é o
  base64 do **texto** `b'abc'`, não dos 3 bytes `61 62 63` (esse seria
  `"YWJj"`). O mesmo vale pra lista: `hash.b64encode([1, 2])` é `"WzEsIDJd"`,
  o base64 de `"[1, 2]"`. Pra binário de verdade use
  [`bytes.base64()`](../../bytes/base64/base64.md), que recebe `bytes` e
  codifica os bytes.
- **Acento ocupa mais de um byte** — `"ç"` são 2 bytes em UTF-8, por isso
  `"w6c="` e não 4 caracteres limpos.
- O caminho de volta é [`hash.b64decode()`](../b64decode/b64decode.md), e a
  ida-e-volta preserva o texto: `hash.b64decode(hash.b64encode("Ola, mundo!"))`
  é `"Ola, mundo!"`.

---

## Uso: mandar um valor num cabeçalho

```
import hash
import request

credencial = hash.b64encode("usuario:senha")   # "dXN1YXJpbzpzZW5oYQ=="
r = request.get("https://api.exemplo.com/dados",
                headers={"Authorization": "Basic " + credencial})
```

---

## Relacionados

- [`hash.b64decode()`](../b64decode/b64decode.md) — o caminho de volta
- [`bytes.base64()`](../../bytes/base64/base64.md) — a mesma codificação, mas de `bytes` pra `str`
- [`hash.sha256()`](../sha256/sha256.md) — impressão digital, não codificação

[← índice](../hash.md)
