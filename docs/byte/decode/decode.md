# `b.decode(encoding="utf-8", errors="strict")`

Volta pra texto, decodificando com o codec pedido.

## Parâmetros

| nome | default |
|---|---|
| `encoding` | `"utf-8"` |
| `errors` | `"strict"` |

## Retorno

str

## Exemplos

```ps
import bytes
b = "ção".encode()
post(b.decode())
post(b.len(), "bytes ->", len(b.decode()), "caracteres")
```

```saida
ção
5 bytes -> 3 caracteres
```

## `errors`

- `"strict"` (padrão): o que não decodifica levanta `UnicodeDecodeError`, com a
  posição. No UTF-8, quando o trecho inválido tem mais de um byte, a frase cita
  a faixa (`bytes in position 0-1`), como no Python.
- `"replace"`: cada trecho inválido vira **um** `\ufffd` (U+FFFD), como no
  Python.
- `"ignore"`: o trecho inválido some.

```ps
import bytes
b = bytes.fromhex("e28228")
post(b.decode("utf-8", "replace").encode("utf-8").hex())   # efbfbd28: um U+FFFD e o (
try {
    b.decode()
} catch (UnicodeDecodeError e) {
    post(e)
}
```

```saida
efbfbd28
'utf-8' codec can't decode bytes in position 0-1: invalid continuation byte (linha 5)
```

[← índice](../byte.md)
