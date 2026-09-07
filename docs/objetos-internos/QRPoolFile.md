# `QRPoolFile`

> **Objeto interno da linguagem** — você não cria `QRPoolFile` na mão:
> é o TIPO de um objeto que a lib `qrcode` te entrega pronto.
> Confira com `type(obj)`, que mostra exatamente este nome.

Arquivo de QR Code em memória — usado quando save= não é fornecido.

name pode ser customizado via gen(data, name="meu.png").

## Métodos e propriedades

| Acesso | O que faz |
|---|---|
| `.bytes()` |  |
| `.save(caminho)` | o parâmetro se chama `caminho`, não `path` |
| `.ext` | atributo — `.png` |
| `.name` | atributo — `qrcode.png` |
| `.size` | atributo — tamanho em bytes |

`.content_type` **não existe**: `AttributeError: 'QRPoolFile' object has no
attribute 'content_type'`.

[← índice](objetos-internos.md)
