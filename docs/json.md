# json — Parse e Serialização

```
from json import parse, stringify
```

---

## parse(text)

Converte texto JSON em `dict`/`list`. Se `text` já for `dict` ou `list`,
devolve como está (idempotente — seguro chamar em cima de algo que já foi
parseado).

```
data = parse('{"nome": "ana", "idade": 30}')
post(data["nome"])   // "ana"

data2 = parse({"ja": "e dict"})
post(data2)   // {"ja": "e dict"} — passou direto
```

---

## stringify(value)

Converte `dict`/`list`/valor pra string JSON. Não escapa caracteres
unicode (`ensure_ascii=False`) — acentos saem como estão.

```
texto = stringify({"nome": "joão", "idade": 30})
post(texto)   // {"nome": "joão", "idade": 30}
```

---

## Exemplo — request e response de API

```
import json

body = json.parse(request.text())
resposta = json.stringify({"status": "ok", "recebido": body})
post(resposta)
```
