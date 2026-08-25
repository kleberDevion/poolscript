# `ChannelStatus`

> **Objeto interno da linguagem** — você não cria `ChannelStatus` na mão:
> é o TIPO de um objeto que a lib `jinker` te entrega pronto.
> Confira com `type(obj)`, que mostra exatamente este nome.

Status de envio do channel.

## Métodos e propriedades

**Nenhum.** `ChannelStatus` é um valor que se COMPARA e se TESTA, não um objeto que se
navega — não existe `.status` (tentar acessar dá `membro inexistente`).

| Uso | Resultado |
|---|---|
| `str(x)` / `post(x)` | "Success"/"Error" |
| `x == "Success"` | compara direto com o texto |
| `if x:` | verdadeiro quando deu certo |

[← índice](objetos-internos.md)
