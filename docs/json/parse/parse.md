# `json.parse(text)`

Converte uma **string JSON** em dados da PoolScript (dict ou lista).

```
json.parse(text: str) -> dict | list
```

---

## Uso

```
import json

dados = json.parse('{"nome": "ana", "tags": [1, 2, 3]}')
post(dados["nome"])          # "ana"
post(dados["tags"][0])       # 1
```

Objeto JSON vira **dict**; array JSON vira **lista**:

```
json.parse('{"a": 1}')       # dict → {"a": 1}
json.parse('[1, 2, 3]')      # lista → [1, 2, 3]
```

---

## É o oposto de `stringify`

```
texto = json.stringify({"a": 1})    # '{"a": 1}'
dados = json.parse(text)           # {"a": 1}  (de volta)
```

---

## Relacionados

- [`json.stringify()`](../stringify/stringify.md) — o caminho inverso (dados → texto)
