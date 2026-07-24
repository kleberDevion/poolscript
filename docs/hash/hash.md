# hash — Senhas seguras

Lib pra guardar senhas do jeito certo: você **nunca** salva a senha em texto.
Em vez disso, salva um **hash** (uma versão embaralhada e irreversível) e, no
login, compara.

```
import hash
```

Só duas funções — e é tudo que você precisa pra autenticação segura:

| Membro | O que faz | Página |
|---|---|---|
| `hash.crypt(senha)` | gera o hash pra **guardar** no cadastro | [crypt/crypt.md](crypt/crypt.md) |
| `hash.check(hash, senha)` | confere se uma senha **bate** com o hash guardado | [check/check.md](check/check.md) |

---

## O fluxo completo

```
import hash

// CADASTRO — guarde o hash, nunca a senha
senha_hash = hash.crypt("minhaSenha123")
// ... salve senha_hash no banco ...

// LOGIN — compare a senha digitada com o hash guardado
if (hash.check(senha_hash, "minhaSenha123")) {
    post("senha correta")
} else {
    post("senha errada")
}
```

---

## Por que não salvar a senha direto

Se o banco vazar, senhas em texto puro expõem todos os usuários (e as pessoas
reusam senhas em outros sites). Um hash é **irreversível** — quem rouba o hash
não consegue voltar pra senha. E cada hash usa um **salt aleatório**, então
duas pessoas com a mesma senha têm hashes diferentes.

---

## Relacionados

- lib `jwt` — gerar tokens de sessão após o login validar
- [jinker/middleware](../jinker/middleware/middleware.md) — proteger rotas com o token
