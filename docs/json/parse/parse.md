# `json.parse(text)`

Converte uma **string JSON** em dados da Jinga (dict ou lista).

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
dados = json.parse(texto)           # {"a": 1}  (de volta)
```

---

## Números

Pela gramática do número do JSON: `-` opcional, `0` ou dígitos sem zero à
esquerda, fração (`.` seguido de dígito) e expoente opcionais. Sem `.` e sem
expoente é `int` — de qualquer tamanho (`json.parse("9" * 600)` tem os 600
dígitos); com eles é `flo`.

- `json.parse("-")` → `ValueError: Expecting value: line 1 column 1 (char 0)`
- `json.parse("01")` e `json.parse("1.")` → `ValueError: Extra data: line 1
  column 2 (char 1)` (o número acaba no `0` e no `1`; o resto sobra)

---

## Relacionados

- [`json.stringify()`](../stringify/stringify.md) — o caminho inverso (dados → texto)
