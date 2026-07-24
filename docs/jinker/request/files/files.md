# `request.files(campo, allowed=None)`

Pega **vários arquivos** enviados no mesmo campo (upload múltiplo). Devolve uma
**lista** de `PoolFileUpload` — lista vazia se não houver arquivo.

```
request.files(campo: str, allowed: list = None) -> list[PoolFileUpload]
```

| Parâmetro | O que é |
|---|---|
| `campo` | nome do campo do formulário |
| `allowed` | extensões permitidas (ex: `[".jpg", ".png"]`); `Null` = qualquer extensão *segura* |

---

## Uso

```
@app.route("/galeria", methods=cors.options(["POST"]))
action galeria() {
    fotos = request.files("fotos", allowed=[".jpg", ".png"])
    if (len(fotos) == 0) {
        return jsonify({"erro": "envie ao menos uma foto"}), 400
    }
    for each foto in fotos {
        foto.save(f"uploads/{foto.name}")
    }
    return jsonify({"recebidas": len(fotos)})
}
```

---

## É o `.file()` no plural

Cada item da lista é um `PoolFileUpload` idêntico ao que
[`.file()`](../file/file.md) devolve — mesmas propriedades (`.name`, `.size`,
`.ext`, …) e métodos (`.save()`, `.bytes()`, `.move()`).

A mesma **segurança de extensão** vale: bloqueadas (`.exe`, `.py`, `.js`, …)
sempre levantam erro; sua lista `allowed` restringe o resto.

---

## `.files()` vs `.file()`

- [`.file(campo)`](../file/file.md) — devolve **um** `PoolFileUpload` (ou `Null`).
- `.files(campo)` — devolve uma **lista** (vazia se nada). Use quando o
  formulário permite selecionar vários arquivos no mesmo campo.

---

## Relacionados

- [`.file()`](../file/file.md) — um arquivo só (detalha o `PoolFileUpload` e a segurança)
- [`request`](../request.md) — visão geral
