# `ChannelStatus`

> **Objeto interno da linguagem** — você não cria `ChannelStatus` na mão:
> é o TIPO de um objeto que a lib `jinker` te entrega pronto.
> Confira com `type(obj)`, que mostra exatamente este nome.

Status de envio do channel.

## Métodos e propriedades

**Nenhum.** `ChannelStatus` é um valor que se COMPARA e se TESTA, não um objeto que se
navega — não existe `.status` (tentar acessar dá `AttributeError: 'ChannelStatus' object has no attribute 'status'`).

| Uso | Resultado |
|---|---|
| `str(x)` / `post(x)` | "Success"/"Error" |
| `x == "Success"` | compara direto com o texto |
| `if x:` | verdadeiro quando deu certo |

`Success` = a mensagem chegou a **pelo menos uma** conexão. Sem alvo nenhum
(nenhuma conexão aberta, ou nenhuma no canal) o resultado é `Error`, e antes
do primeiro envio também: nada chegou a ninguém.

## Não vira JSON

`ChannelStatus` não é texto: ele não vira JSON, nem no `json.stringify`, nem
na rota que o devolve dentro de um dict (que dá `500`), nem no `emit`. O
`str()` vira:

```ps
from jinker import Jinker
import json
Jinker app = Jinker(__name__)
post(json.stringify({"status": str(app.channel.status)}))
```
```saida
{"status": "Error"}
```

```ps
from jinker import Jinker
import json
Jinker app = Jinker(__name__)
post(json.stringify({"status": app.channel.status}))
# TypeError: Object of type ChannelStatus is not JSON serializable
```

[← índice](objetos-internos.md)
