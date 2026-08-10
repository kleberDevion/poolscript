# `PoolFileUpload`

Arquivo recebido via upload — vive em memória, sem path temporário.

## Métodos e propriedades

| Acesso | O que faz |
|---|---|
| `.bytes()` | Retorna os bytes brutos. |
| `.move(destino)` | Alias de save() — semântica de mover da memória pro disco. |
| `.save(destino)` | Salva os bytes em disco no caminho especificado. |
| `.content_type` | atributo |
| `.ext` | atributo |
| `.name` | atributo |
| `.size` | atributo |

[← índice](../jinker.md)
