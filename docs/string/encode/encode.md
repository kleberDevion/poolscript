# `s.encode(encoding="utf-8", errors="strict")`

Converte a string para bytes.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `encoding` | str | "utf-8" | **respeitado**, não ignorado |
| `errors` | str | "strict" | `"ignore"` descarta o que não couber; `"replace"` troca por `?` |

## Retorno

bytes

## Erros

- **UnicodeEncodeError** — o caractere não existe no charset pedido e
  `errors` é `"strict"`:

  ```ps
  post("ção".encode("latin-1"))            # b'\xe7\xe3o'   — bytes de latin-1
  post("ção".encode("ascii"))              # erro: 'ascii' codec can't encode characters
  post("ção".encode("ascii", "ignore"))    # b'o'
  post("ção".encode("ascii", errors="replace"))   # b'??o'
  ```

## Exemplos

```ps
post("ab".encode())
```

```saida
b'ab'
```

## Bordas

- bytes só é igual a bytes: `"ab" == "ab".encode()` é False

[← índice](../string.md)
