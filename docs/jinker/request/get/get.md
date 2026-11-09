# `request.get(key)`

Pega **um campo** da requisição, procurando primeiro na **query string** e
depois no **corpo JSON**. Atalho pra quando você quer só um valor sem parsear o
corpo inteiro.

```assinatura
request.get(key: str) -> valor | Null
```

Os exemplos desta página pressupõem:

```ps contexto
from jinker import Jinker, cors, jsonify
Jinker app = Jinker(__name__)
```

## Query string

Uma chave pode aparecer mais de uma vez. Uma ocorrência retorna `str`; várias
ocorrências retornam `list[str]`, na ordem em que aparecem. O tipo, portanto,
depende da requisição. O valor da query é sempre texto: `?idade=42` retorna
`"42"`, `?idade=` retorna `""`, e `%20` e `+` são decodificados como espaço.
As chaves diferenciam maiúsculas de minúsculas.

```ps
@app.route("/unico")
funct um_valor() {
    str um = request.get("q")       # /unico?q=jinga
    return jsonify({"q": um})
}
```

Uma variável declarada como `str` só pode receber a forma de uma ocorrência;
se o cliente puder repetir a chave, trate o retorno como `str | list` antes
de atribuir:

```ps
@app.route("/varios")
funct varios_valores() {
    list varios = request.get("q")  # /varios?q=jinga&q=pool
    return jsonify({"q": varios})
}
```

---

## Corpo JSON

Se a chave não estiver na query, `request.get()` procura um objeto JSON no
corpo e preserva o tipo do campo: número inteiro, número fracionário, `bool`,
`list` ou `dict`. Isso vale mesmo sem o cabeçalho
`Content-Type: application/json`.

```ps
@app.route("/tipos", methods=cors.options(["POST"]))
funct tipos() {
    inteiro = request.get("idade")       # 42
    ativo = request.get("ativo")         # True
    return jsonify({"idade": inteiro, "ativo": ativo})
}
```

`null`, uma chave ausente, JSON inválido e um corpo JSON cuja raiz é uma lista
não fornecem um campo: todos resultam em `Null`. Query tem prioridade sobre o
corpo JSON.

## Exemplos

**Da query string** (`/buscar?termo=jinga`):

```ps
@app.route("/buscar", methods=cors.options(["GET"]))
funct buscar() {
    termo = request.get("termo")     # "jinga"
    return jsonify({"buscando": termo})
}
```

Chaves repetidas preservam todos os valores:

```ps
@app.route("/buscar")
funct buscar_varios() {
    termos = request.get("termo")    # ["jinga", "pool"]
    return jsonify({"buscando": termos})
}
```

Campos de formulário `application/x-www-form-urlencoded` e campos de
`multipart/form-data` não são lidos por `request.get()`: o resultado é `Null`.
Para multipart, use [`request.file()`](../file/file.md) ou
[`request.files()`](../files/files.md) para arquivos e `request.text()` se
precisar processar o corpo cru.

**Do corpo JSON** (`POST` com `{"nome": "ana"}`):

```ps
@app.route("/salvar", methods=cors.options(["POST"]))
funct salvar() {
    nome = request.get("nome")       # "ana"
    return jsonify({"salvo": nome})
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

## `.get()` vs `.get_json()`

- `.get("nome")` — **um** campo (query ou JSON).
- [`.get_json()`](../get_json/get_json.md) — o corpo JSON **inteiro** como dict.

Use `.get()` pra um ou dois campos; `.get_json()` quando precisa do objeto todo.

---

## Relacionados

- [`.get_json()`](../get_json/get_json.md) — corpo inteiro
- [`.path_param()`](../path_param/path_param.md) — valor vindo da URL, não do corpo
