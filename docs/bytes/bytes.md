# bytes — Criar e converter sequências de bytes

Lib pra trabalhar com **dados binários** em Jinga puro: converter
hex/base64, empacotar/desempacotar inteiros, fatiar, concatenar e fazer XOR —
sem depender de nenhuma lib externa. É o que dá autonomia pra construir suas
próprias ferramentas binárias.

```
import bytes
```

O **tipo** do valor chama-se `byte` (`type("oi".encode())` → `byte`); `bytes`
é o **módulo**, com as funções de criar e converter. Um valor `byte` nasce de
`"oi".encode()`, de um arquivo aberto em `"rb"`, das funções desta lib, ou do
**literal `b"..."`**:

## Literal `b"..."`

```ps
sig = b"\x89PNG\r\n\x1a\n"        # 8 bytes crus, exatamente esses
post(len(sig), sig)                # 8 b'\x89PNG\r\n\x1a\n'
post(b"a\x00b", len(b"a\x00b"))    # b'a\x00b' 3 — NUL dentro vale
post(b"\xff"[0])                   # 255
post(b"é")                         # b'\xc3\xa9' — não-ASCII entra com os bytes UTF-8 do fonte
```

- **Qualquer byte, 0 a 255.** `\xHH` é sempre UM byte; octal `\ooo` (0–255);
  `\n \r \t \a \b \f \v \e \0 \\ \" \'` valem como na string.
- `\u`/`\U` **não existem** em bytes (byte não tem codepoint):
  `SyntaxError: \u nao vale em bytes: use \xHH`. Octal acima de 255 também é
  erro. Escape desconhecido mantém a barra e avisa, como na string.
- `B"..."` é o mesmo que `b"..."`; `br"..."`/`rb"..."` é **cru** (a barra fica:
  `br"\n"` tem 2 bytes); `b'''...'''` (ou `b"""..."""`) é multilinha (a quebra vira o byte `\n`).
- `b = 1` continua sendo uma variável: o prefixo só vale com a aspa colada.

## O tipo `byte`

```ps
byte x = b"a"          # declaração tipada confere: `byte y = "texto"` é AttributedValueError
post(x is byte)        # True
post(byte("a"))        # b'a' — o mesmo que bytes.new("a")
post(type(x))          # byte
```

`byte` é um nome global (não precisa de `import`), como `PoolFile`.

| Membro | O que faz | Página |
|---|---|---|
| `bytes.new(x)` | cria bytes de lista, texto, tamanho ou cópia | [new/new.md](new/new.md) |
| `bytes.fromhex(s)` | bytes a partir de uma string hex | [fromhex/fromhex.md](fromhex/fromhex.md) |
| `bytes.hex(b)` | os bytes como string hex | [hex/hex.md](hex/hex.md) |
| `bytes.base64(b)` | os bytes como string base64 | [base64/base64.md](base64/base64.md) |
| `bytes.frombase64(s)` | bytes a partir de uma string base64 | [frombase64/frombase64.md](frombase64/frombase64.md) |
| `bytes.fromint(n, length, byteorder)` | empacota um inteiro em bytes | [fromint/fromint.md](fromint/fromint.md) |
| `bytes.toint(b, byteorder)` | desempacota bytes num inteiro | [toint/toint.md](toint/toint.md) |
| `bytes.tolist(b)` | lista com o valor de cada byte (0-255) | [tolist/tolist.md](tolist/tolist.md) |
| `bytes.concat(lista)` | junta uma lista de bytes num só | [concat/concat.md](concat/concat.md) |
| `bytes.slice(b, ini, fim)` | uma fatia dos bytes | [slice/slice.md](slice/slice.md) |
| `bytes.get(b, i)` | o valor inteiro do byte na posição `i` | [get/get.md](get/get.md) |
| `bytes.xor(dados, chave)` | XOR byte a byte (chave repetida) | [xor/xor.md](xor/xor.md) |

## Métodos do próprio valor

Estes se chamam NO valor, não na lib — `b.metodo()`, não `bytes.metodo(b)` — e
são do **tipo** `byte`, não do módulo. Os 44 estão em
[`docs/byte/`](../byte/byte.md), cada um com a assinatura tirada do motor e a
saída de um exemplo que foi RODADO. Os dois espaços de nome se cruzam de
propósito: `bytes.hex(b)` é função da lib e `b.hex()` é método do valor.

---

## Operadores

`byte` responde aos mesmos operadores de sequência que `str` e `list`:

```
import bytes

b = "Hello".encode()

post(len(b))                 # 5
post(b[0])                   # 72     — indexar dá o INTEIRO do byte
post(b[0:2])                 # b'He'  — fatiar dá bytes
post(b + " ali".encode())    # b'Hello ali'
post(b * 2)                  # b'HelloHello'
post("ell".encode() in b)    # True   — subsequência
post(101 in b)               # True   — esse BYTE aparece?
post("abc".encode() < "abd".encode())   # True — ordem lexicográfica por byte

for each x in "abc".encode() {
    post(x)                  # 97, 98, 99 — itera em INTEIROS
}
```

Duas coisas que valem lembrar:

- **Indexar dá inteiro, fatiar dá bytes.** `b[0]` é `72`; `b[0:1]` é `b'H'`.
- **Iterar dá inteiro.** `for each x in b` entrega `int`, não pedaços de um
  byte. É o que faz `if x == 0` funcionar direto pra procurar byte NUL.

---

## Exemplos rápidos

```
import bytes

# criar
b = bytes.new([72, 105])         # b'Hi'  (de lista de inteiros)
b = bytes.new("Oi")              # b'Oi'  (de texto, utf-8)
b = bytes.fromhex("deadbeef")    # de hex

# converter
post(bytes.hex(b))               # "deadbeef"
post(bytes.base64(b))            # "3q2+7w=="
post(bytes.toint(b))             # 3735928559
post(bytes.tolist(b))            # [222, 173, 190, 239]

# manipular
post(bytes.slice(b, 0, 2))       # b'\xde\xad'
post(bytes.get(b, 0))            # 222
post(bytes.concat([bytes.new("a"), bytes.new("b")]))  # b'ab'
```

---

## Empacotar e desempacotar inteiros

Útil pra ler/escrever formatos binários (protocolos, cabeçalhos de arquivo):

```
import bytes

# inteiro → 4 bytes, big-endian (o padrão)
cab = bytes.fromint(258, 4)      # b'\x00\x00\x01\x02'
post(bytes.toint(cab))           # 258

# little-endian quando o formato pede
le = bytes.fromint(258, 4, "little")   # b'\x02\x01\x00\x00'
post(bytes.toint(le, "little"))        # 258
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
decifrado = bytes.xor(cifrado, chave)    # XOR de novo com a mesma chave
post(decifrado)                          # b'mensagem secreta'
```

---

## Relacionados

- [builtins](../builtins/builtins.md) — `open(..., "rb")` lê um arquivo como bytes
- [string methods](../str/str.md) — `"texto".encode()` cria bytes; `b.decode()` volta pra texto
- lib `hash` — hashes e base64 de senhas
