# `request.get(key)`

Pega **um campo** da requisição, procurando primeiro na **query string** e
depois no **corpo JSON**. Atalho pra quando você quer só um valor sem parsear o
corpo inteiro.

```
request.get(key: str) -> valor | Null
```

---

## Exemplos

**Da query string** (`/buscar?termo=poolscript`):

```
@app.route("/buscar", methods=cors.options(["GET"]))
action buscar() {
    termo = request.get("termo")     # "poolscript"
    return jsonify({"buscando": termo})
}
```

**Do corpo JSON** (`POST` com `{"nome": "ana"}`):

```
@app.route("/salvar", methods=cors.options(["POST"]))
action salvar() {
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

```
nome = request.get("nome")
if (nome is Null) {
    return jsonify({"erro": "campo 'nome' obrigatório"}), 400
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
