# `request.get(key)`

Pega **um valor** da requisição, procurando primeiro na **query string** e
depois no **corpo JSON**. Atalho pra quando você quer só um valor sem parsear o
corpo inteiro.

```assinatura
request.get(key: str) -> valor | list | Null
```

Os exemplos desta página pressupõem:

```ps contexto
from jinker import Jinker, cors, jsonify
Jinker app = Jinker(__name__)
```

---

## Exemplos

**Da query string** (`/buscar?termo=jinga`):

```ps
@app.route("/buscar", methods=cors.options(["GET"]))
funct buscar() {
    termo = request.get("termo")     # "jinga"
    return jsonify({"buscando": termo})
}
```

**Do corpo JSON** (`POST` com `{"nome": "ana"}`):

```ps
@app.route("/salvar", methods=cors.options(["POST"]))
funct salvar() {
    nome = request.get("nome")       # "ana"
    return jsonify({"salvo": nome})
}
```

**Chave repetida na query string** (`/tags?tag=go&tag=jinga`):

```ps
@app.route("/tags", methods=cors.options(["GET"]))
funct tags() {
    tag = request.get("tag")         # ["go", "jinga"]
    return jsonify({"tag": tag})
}
```

---

## Ordem de busca e ausência

1. procura na query string;
2. se não achar, procura no corpo JSON.

Se a chave não existir em nenhum dos dois, devolve `Null`. Trate isso quando o
campo é obrigatório:

```ps
funct salvar() {
    nome = request.get("nome")
    if (nome is Null) {
        return jsonify({"erro": "campo 'nome' obrigatório"}), 400
    }
}
```

---

## O que o `get` enxerga

| Origem | O que sai |
|---|---|
| query string, chave única | `str` — sempre texto; `%20` e `+` são decodificados; `?a=` vira `""` |
| query string, chave repetida | `list` de `str`, na ordem (`?a=1&a=2` → `["1", "2"]`) |
| corpo JSON: `int`, `flo`, `bool` | o valor com o tipo do JSON |
| corpo JSON: lista ou objeto | `list` / `dict` |
| corpo JSON: `null` | `Null` |
| ausente nos dois | `Null` |
| corpo que não é JSON, ou JSON inválido | `Null` — não é erro |
| formulário (`urlencoded`/`multipart`) | **não é lido**; use [`text()`](../text/text.md) ou [`file()`](../file/file.md)/[`files()`](../files/files.md) |

A busca ignora o `Content-Type`: se o corpo for JSON, ele é lido mesmo sem o
cabeçalho. A chave diferencia maiúscula de minúscula (`?Q=1` não é `q`).

---

## Chave repetida e rota tipada

`str nome = request.get("nome")` só é seguro quando a chave **não** se repete.
Com `?nome=ana&nome=bia` o valor é uma lista (`list`) e a atribuição falha em
execução (500). Use uma variável sem tipo e confira `type(valor)`, ou trate a
lista:

```ps
@app.route("/tags", methods=cors.options(["GET"]))
funct tags_tipado() {
    valor = request.get("tag")       # str, list ou Null
    if (type(valor) == "list") {
        return jsonify({"tags": valor})
    }
    return jsonify({"tags": [valor]})
}
```

---

## `.get()` vs `.get_json()`

- `.get("nome")` — **um** valor (query ou JSON); pode ser lista se a chave se
  repete na query.
- [`.get_json()`](../get_json/get_json.md) — o corpo JSON **inteiro** como dict.

Use `.get()` pra um ou dois campos; `.get_json()` quando precisa do objeto todo.

---

## Relacionados

- [`.get_json()`](../get_json/get_json.md) — corpo inteiro
- [`.text()`](../text/text.md) — corpo cru (formulário não entra no `get`)
- [`.path_param()`](../path_param/path_param.md) — valor vindo da URL, não do corpo
