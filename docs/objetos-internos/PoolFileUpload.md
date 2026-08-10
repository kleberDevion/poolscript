<!-- gerado: gera_doc_gaps.py — pode regenerar -->
# `PoolFileUpload`

> **Objeto interno da linguagem** — você não cria `PoolFileUpload` na mão:
> é o TIPO de um objeto que a lib `jinker` te entrega pronto.
> Confira com `type(obj)`, que mostra exatamente este nome.

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

[← índice](objetos-internos.md)
