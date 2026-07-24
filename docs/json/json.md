# json — Converter entre texto JSON e dados

Lib pra converter entre uma **string JSON** e os **dados** da PoolScript (dicts,
listas). Você usa quando recebe/manda JSON como texto e precisa ir e voltar.

```
import json
// (JSON — maiúsculo — é o mesmo módulo, apelido)
```

| Membro | O que faz | Página |
|---|---|---|
| `json.parse(texto)` | texto JSON → dict/lista | [parse/parse.md](parse/parse.md) |
| `json.stringify(valor)` | dict/lista → texto JSON | [stringify/stringify.md](stringify/stringify.md) |

---

## As duas direções

```
import json

// texto → dados
dados = json.parse('{"nome": "ana", "idade": 30}')
post(dados["nome"])          // "ana"

// dados → texto
texto = json.stringify({"nome": "ana", "idade": 30})
post(texto)                  // '{"nome": "ana", "idade": 30}'
```

`parse` e `stringify` são **opostos**: um desfaz o outro.

---

## Quando você precisa disto

Muitas libs já entregam/aceitam dicts direto (o jinker converte sozinho, o
`request.get_json()` já parseia). Você usa a lib `json` quando tem o JSON **como
texto puro** — lendo de um arquivo `.txt`, de uma variável de ambiente, ou
montando manualmente pra salvar.

```
import json
import os

// um JSON guardado numa variável de ambiente
config = json.parse(os.getenv("CONFIG_JSON"))
```

---

## Relacionados

- [`request.get_json()`](../request/Response/Response.md) — já parseia respostas HTTP
- [jinker jsonify](../jinker/jsonify/jsonify.md) — resposta JSON num servidor
