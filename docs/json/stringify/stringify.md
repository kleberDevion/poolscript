# `json.stringify(value)`

Converte dados da Jinga (dict, lista, etc.) em uma **string JSON**.

```assinatura
json.stringify(value) -> str
```

Os exemplos desta página pressupõem:

```ps contexto
import json
```

---

## Uso

```ps
import json

texto = json.stringify({"nome": "ana", "idade": 30})
post(texto)          # '{"nome": "ana", "idade": 30}'
```

Serve pra **guardar** ou **enviar** dados como texto — num arquivo, numa
variável de ambiente, num corpo de requisição montado na mão.

```ps
import json
import os

# salvar um dict como texto num arquivo
config = {"tema": "escuro", "porta": 8080}
using open("config.json", "w") as f {
    f.write(json.stringify(config))
}
```

---

## É o oposto de `parse`

```ps
dados = {"a": 1}
texto = json.stringify(dados)       # '{"a": 1}'
json.parse(texto)                   # {"a": 1}  (de volta)
```

---

## Bordas

- O JSON não tem `nan` nem infinito: `json.stringify(flo("nan"))` dá
  `ValueError: Out of range float values are not JSON compliant: nan` (e
  `inf`, `-inf`), em qualquer profundidade — dentro de lista e como valor
  de dict também. Antes saía um texto (`[nan, inf]`) que o próprio
  `json.parse` recusava.
- A chave de dict sempre vira texto, porque o JSON só tem chave de texto:
  `Null` vira `"null"` e `True`/`False` viram `"true"`/`"false"` (a mesma
  grafia de quando são valor), e número vira o texto dele:

```ps
json.stringify({Null: 1, True: 2, 3: "x"})    # '{"null": 1, "true": 2, "3": "x"}'
```

---

## Relacionados

- [`json.parse()`](../parse/parse.md) — o caminho inverso (texto → dados)
- [jinker jsonify](../../jinker/jsonify/jsonify.md) — num servidor, use `jsonify` (já faz isso)
