# bytes — Criar e converter sequências de bytes

Lib pra trabalhar com **dados binários** em PoolScript puro: converter
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
  `br"\n"` tem 2 bytes); `b'''...'''` é multilinha (a quebra vira o byte `\n`).
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

Estes se chamam NO valor, não na lib — `b.metodo()`, não `bytes.metodo(b)`.
Cada página tem a assinatura tirada do motor e a saída de um exemplo que foi
RODADO.

As páginas ficam sob [`metodos/`](metodos/) porque os dois espaços de nome se
cruzam: `bytes.hex(b)` é função da lib e `b.hex()` é método do valor.

**Buscar**

| Método | O que faz |
|---|---|
| [`b.find(sub, inicio=0, fim=Null)`](metodos/find/find.md) | posição da primeira ocorrência, ou -1 |
| [`b.rfind(sub, inicio=0, fim=Null)`](metodos/rfind/rfind.md) | posição da última ocorrência, ou -1 |
| [`b.index(sub, inicio=0, fim=Null)`](metodos/index/index.md) | como `find`, mas levanta se não achar |
| [`b.rindex(sub, inicio=0, fim=Null)`](metodos/rindex/rindex.md) | como `rfind`, mas levanta se não achar |
| [`b.count(sub, inicio=0, fim=Null)`](metodos/count/count.md) | quantas vezes aparece |
| [`b.contains(sub)`](metodos/contains/contains.md) | True se contém |
| [`b.has(sub)`](metodos/has/has.md) | o mesmo que `contains` |
| [`b.startswith(prefixo, inicio=0, fim=Null)`](metodos/startswith/startswith.md) | começa com? aceita tupla de opções |
| [`b.endswith(sufixo, inicio=0, fim=Null)`](metodos/endswith/endswith.md) | termina com? aceita tupla de opções |

**Caixa** — todos mexem SÓ no ASCII

| Método | O que faz |
|---|---|
| [`b.upper()`](metodos/upper/upper.md) | maiúsculas |
| [`b.lower()`](metodos/lower/lower.md) | minúsculas |
| [`b.title()`](metodos/title/title.md) | inicial de cada palavra em maiúscula |
| [`b.capitalize()`](metodos/capitalize/capitalize.md) | só a primeira em maiúscula |
| [`b.swapcase()`](metodos/swapcase/swapcase.md) | troca maiúscula por minúscula |

**Perguntar**

| Método | O que faz |
|---|---|
| [`b.isalpha()`](metodos/isalpha/isalpha.md) | só letras ASCII? |
| [`b.isdigit()`](metodos/isdigit/isdigit.md) | só dígitos? |
| [`b.isalnum()`](metodos/isalnum/isalnum.md) | só letra ou dígito? |
| [`b.isspace()`](metodos/isspace/isspace.md) | só branco? |
| [`b.isupper()`](metodos/isupper/isupper.md) | tem letra e nenhuma minúscula? |
| [`b.islower()`](metodos/islower/islower.md) | tem letra e nenhuma maiúscula? |
| [`b.istitle()`](metodos/istitle/istitle.md) | está em formato de título? |
| [`b.isascii()`](metodos/isascii/isascii.md) | todo byte < 0x80? (vazio é True) |

**Aparar e trocar**

| Método | O que faz |
|---|---|
| [`b.strip(chars=Null)`](metodos/strip/strip.md) | tira das duas pontas; `chars` é CONJUNTO |
| [`b.lstrip(chars=Null)`](metodos/lstrip/lstrip.md) | só da esquerda |
| [`b.rstrip(chars=Null)`](metodos/rstrip/rstrip.md) | só da direita |
| [`b.removeprefix(p)`](metodos/removeprefix/removeprefix.md) | tira o prefixo, se estiver lá |
| [`b.removesuffix(p)`](metodos/removesuffix/removesuffix.md) | tira o sufixo, se estiver lá |
| [`b.replace(old, new, count=-1)`](metodos/replace/replace.md) | troca ocorrências |
| [`b.translate(tabela, delete=Null)`](metodos/translate/translate.md) | traduz byte a byte |
| [`b.maketrans(de, para)`](metodos/maketrans/maketrans.md) | monta a tabela do `translate` |

**Partir e juntar**

| Método | O que faz |
|---|---|
| [`b.split(sep=Null, maxsplit=-1)`](metodos/split/split.md) | parte no separador, ou em branco |
| [`b.rsplit(sep=Null, maxsplit=-1)`](metodos/rsplit/rsplit.md) | o mesmo, contando do fim |
| [`b.splitlines(keepends=false)`](metodos/splitlines/splitlines.md) | parte em linhas (`\n`, `\r`, `\r\n`) |
| [`b.partition(sep)`](metodos/partition/partition.md) | `(antes, sep, depois)` na primeira |
| [`b.rpartition(sep)`](metodos/rpartition/rpartition.md) | o mesmo, na última |
| [`b.join(lista)`](metodos/join/join.md) | junta usando este valor como separador |

**Preencher**

| Método | O que faz |
|---|---|
| [`b.ljust(width, fillbyte)`](metodos/ljust/ljust.md) | enche à direita |
| [`b.rjust(width, fillbyte)`](metodos/rjust/rjust.md) | enche à esquerda |
| [`b.center(width, fillbyte)`](metodos/center/center.md) | centraliza |
| [`b.zfill(largura)`](metodos/zfill/zfill.md) | zeros à esquerda, respeitando o sinal |
| [`b.expandtabs(tabsize=8)`](metodos/expandtabs/expandtabs.md) | tabulação vira espaços |

**Converter**

| Método | O que faz |
|---|---|
| [`b.decode(encoding="utf-8", errors="strict")`](metodos/decode/decode.md) | volta pra texto |
| [`b.hex(sep=Null, bytes_per_sep=1)`](metodos/hex/hex.md) | texto hexadecimal, com separador opcional |
| [`b.len()`](metodos/len/len.md) | **quantos BYTES** — não caracteres |

`b.len()` conta byte, e a diferença importa:

```
b = "ção".encode()
post(b.len())        # 5  — em UTF-8, "ç" e "ã" ocupam 2 bytes cada
post("ção".len())    # 3  — caracteres
```

É o mesmo número que o builtin `len(b)` devolve; existe como método porque
`str`, `list`, `dict` e `tup` também têm `.len()`, e `bytes` era o único de
fora — quem escrevia `b.len()` por analogia tomava erro em tempo de execução.

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
- [string methods](../string/string.md) — `"texto".encode()` cria bytes; `b.decode()` volta pra texto
- lib `hash` — hashes e base64 de senhas
