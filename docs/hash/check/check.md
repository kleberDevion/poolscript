# `hash.check(senha_hash, senha_digitada)`

Confere se a senha que o usuário **digitou** bate com o **hash** guardado no
banco. Devolve `True`/`False`.

```assinatura
hash.check(senha_hash: str, senha_digitada: str) -> bool
```

| Parâmetro | O que é |
|---|---|
| `senha_hash` | o hash que você guardou (vindo de [`hash.crypt`](../crypt/crypt.md)) |
| `senha_digitada` | a senha em texto que o usuário informou no login |

Os exemplos desta página pressupõem (o `senha_hash` faz o papel do hash que
veio do banco):

```ps contexto
import hash
str senha_hash = hash.crypt("minhaSenha123")
```

---

## Uso no login

```ps
import hash

# senha_hash veio do banco; "minhaSenha123" veio do formulário de login
if (hash.check(senha_hash, "minhaSenha123")) {
    post("login ok")
} else {
    post("senha incorreta")
}
```

---

## Por que não usar `==`

Como cada hash tem um salt aleatório, você **não pode** comparar a senha com o
hash usando `==` (nunca vai bater). O `hash.check()` extrai o salt do hash
guardado e refaz a conta certa — é o único jeito correto de conferir.

---

## Exemplo: rota de login (jinker)

```ps
@app.route("/api/login", methods=cors.options(["POST"]))
funct login() {
    senha = request.get("senha")
    # senha_hash você busca no banco pelo usuário...
    if (hash.check(senha_hash, senha)) {
        return jsonify({"ok": True})
    }
    return jsonify({"erro": "credenciais inválidas"}), 401
}
```

---

## Relacionados

- [`hash.crypt()`](../crypt/crypt.md) — gerar o hash no cadastro
- lib `jwt` — emitir um token após o `check` passar
