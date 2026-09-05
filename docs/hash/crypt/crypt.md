# `hash.crypt(senha)`

Gera um **hash seguro** de uma senha, com salt aleatório. É o que você **guarda**
no banco no momento do cadastro — nunca a senha original.

```
hash.crypt(senha: str) -> str
```

---

## Uso

```
import hash

senha_hash = hash.crypt("minhaSenha123")
post(senha_hash)     # algo tipo "$2b$12$Xk...longo..." — irreversível

# salve senha_hash no banco (NÃO a senha em texto)
```

---

## Salt aleatório: mesmo texto, hashes diferentes

Cada chamada usa um salt novo, então a **mesma senha gera hashes diferentes**:

```
hash.crypt("abc")    # "$2b$12$aaa..."
hash.crypt("abc")    # "$2b$12$bbb..."  (diferente!)
```

Isso é proposital e seguro — o [`hash.check()`](../check/check.md) sabe
comparar mesmo assim. Não tente comparar dois hashes com `==`; use sempre
`hash.check()`.

---

## Relacionados

- [`hash.check()`](../check/check.md) — conferir a senha no login
- [visão geral do hash](../hash.md) — o fluxo cadastro → login
