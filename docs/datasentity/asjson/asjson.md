# `asjson(instancia)`

Converte uma instância de `@dataentity` direto numa **string JSON**.

```
asjson(instancia) -> str
```

---

## Uso

```
from datasentity import dataentity, asjson

@dataentity
Entity Pessoa():
    nome: str
    idade: int

p = Pessoa(nome="Ana", idade=30)
post(asjson(p))     // '{"nome": "Ana", "idade": 30}'
```

Atalho pra `asdict` + `json.stringify` — útil pra gravar em arquivo ou mandar
numa resposta HTTP.

---

## Relacionados

- [`asdict()`](../asdict/asdict.md) — dict (se quiser mexer antes de virar JSON)
- [jinker jsonify](../../jinker/jsonify/jsonify.md) — numa API, jsonify(asdict(p)) responde JSON
