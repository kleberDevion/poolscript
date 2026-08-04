# bytes — Criar e converter sequências de bytes

Lib pra trabalhar com **dados binários** em PoolScript puro: converter
hex/base64, empacotar/desempacotar inteiros, fatiar, concatenar e fazer XOR —
sem depender de nenhuma lib externa. É o que dá autonomia pra construir suas
próprias ferramentas binárias.

```
import bytes
```

O tipo `bytes` já existe na linguagem (`"oi".encode()`, leitura de arquivo em
modo binário). Esta lib é o que permite **criar** bytes do zero e **converter**
entre formatos.

| Membro | O que faz | Página |
|---|---|---|
| `bytes.new(x)` | cria bytes de lista, texto, tamanho ou cópia | [new/new.md](new/new.md) |
| `bytes.fromhex(s)` | bytes a partir de uma string hex | [fromhex/fromhex.md](fromhex/fromhex.md) |
| `bytes.hex(b)` | os bytes como string hex | [hex/hex.md](hex/hex.md) |
| `bytes.base64(b)` | os bytes como string base64 | [base64/base64.md](base64/base64.md) |
| `bytes.frombase64(s)` | bytes a partir de uma string base64 | [frombase64/frombase64.md](frombase64/frombase64.md) |
| `bytes.fromint(n, tam, ordem)` | empacota um inteiro em bytes | [fromint/fromint.md](fromint/fromint.md) |
| `bytes.toint(b, ordem)` | desempacota bytes num inteiro | [toint/toint.md](toint/toint.md) |
| `bytes.tolist(b)` | lista com o valor de cada byte (0-255) | [tolist/tolist.md](tolist/tolist.md) |
| `bytes.concat(lista)` | junta uma lista de bytes num só | [concat/concat.md](concat/concat.md) |
| `bytes.slice(b, ini, fim)` | uma fatia dos bytes | [slice/slice.md](slice/slice.md) |
| `bytes.get(b, i)` | o valor inteiro do byte na posição `i` | [get/get.md](get/get.md) |
| `bytes.xor(dados, chave)` | XOR byte a byte (chave repetida) | [xor/xor.md](xor/xor.md) |

---

## Exemplos rápidos

```
import bytes

// criar
b = bytes.new([72, 105])         // b'Hi'  (de lista de inteiros)
b = bytes.new("Oi")              // b'Oi'  (de texto, utf-8)
b = bytes.fromhex("deadbeef")    // de hex

// converter
post(bytes.hex(b))               // "deadbeef"
post(bytes.base64(b))            // "3q2+7w=="
post(bytes.toint(b))             // 3735928559
post(bytes.tolist(b))            // [222, 173, 190, 239]

// manipular
post(bytes.slice(b, 0, 2))       // b'\xde\xad'
post(bytes.get(b, 0))            // 222
post(bytes.concat([bytes.new("a"), bytes.new("b")]))  // b'ab'
```

---

## Empacotar e desempacotar inteiros

Útil pra ler/escrever formatos binários (protocolos, cabeçalhos de arquivo):

```
import bytes

// inteiro → 4 bytes, big-endian (o padrão)
cab = bytes.fromint(258, 4)      // b'\x00\x00\x01\x02'
post(bytes.toint(cab))           // 258

// little-endian quando o formato pede
le = bytes.fromint(258, 4, "little")   // b'\x02\x01\x00\x00'
post(bytes.toint(le, "little"))        // 258
```

---

## XOR de chave repetida

`bytes.xor` aplica XOR byte a byte; se a chave for menor, ela repete. Como o XOR
é reversível, aplicar a mesma chave duas vezes volta ao original — dá pra fazer
uma cifra simples:

```
import bytes

segredo = bytes.new("mensagem secreta")
chave   = bytes.new("K3y")

cifrado  = bytes.xor(segredo, chave)
decifrado = bytes.xor(cifrado, chave)    // XOR de novo com a mesma chave
post(decifrado)                          // b'mensagem secreta'
```

---

## Relacionados

- [builtins](../builtins/builtins.md) — `open(..., "rb")` lê um arquivo como bytes
- [string methods](../string/string.md) — `"texto".encode()` cria bytes; `b.decode()` volta pra texto
- lib `hash` — hashes e base64 de senhas
