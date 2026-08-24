# `json.stringify(value)`

Converte dados da PoolScript (dict, lista, etc.) em uma **string JSON**.

```
json.stringify(value) -> str
```

---

## Uso

```
import json

texto = json.stringify({"nome": "ana", "idade": 30})
post(texto)          // '{"nome": "ana", "idade": 30}'
```

Serve pra **guardar** ou **enviar** dados como texto — num arquivo, numa
variável de ambiente, num corpo de requisição montado na mão.

```
import json
import os

// salvar um dict como texto num arquivo
config = {"tema": "escuro", "porta": 8080}
using open("config.json", "w") as f {
    f.write(json.stringify(config))
}
```

---

## É o oposto de `parse`

```
dados = {"a": 1}
texto = json.stringify(dados)       // '{"a": 1}'
json.parse(texto)                   // {"a": 1}  (de volta)
```

---

## Relacionados

- [`json.parse()`](../parse/parse.md) — o caminho inverso (texto → dados)
- [jinker jsonify](../../jinker/jsonify/jsonify.md) — num servidor, use `jsonify` (já faz isso)
