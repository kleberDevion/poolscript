<!-- gerado: gera_doc_gaps.py — pode regenerar -->
# `ChannelManager`

> **Objeto interno da linguagem** — você não cria `ChannelManager` na mão:
> é o TIPO de um objeto que a lib `jinker` te entrega pronto.
> Confira com `type(obj)`, que mostra exatamente este nome.

app.channel — gerencia conexões websocket abertas, com suporte a salas.

Uma conexão entra automaticamente numa "sala" quando o path do socket tem
parâmetro dinâmico (ex: /sala:id → sala = valor de :id).

app.channel(forAll=msg)               → broadcast pra todos
app.channel.emit(msg, room_id="123")  → manda só pra sala "123"
app.channel.status                    → status do último envio

## Métodos e propriedades

| Acesso | O que faz |
|---|---|
| `.emit(payload=None, room_id=None, exclude=None)` | Envia payload pros conectados. room_id filtra pra uma sala específica; |
| `.status` | atributo |

### `.emit(...)`

Envia payload pros conectados. room_id filtra pra uma sala específica;
sem room_id, faz broadcast geral (igual __call__(forAll=...)).
exclude: set de conexões a pular (usado pra não ecoar pro remetente).

[← índice](../jinker.md)
