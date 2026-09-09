# hash — Senhas seguras

Lib pra guardar senhas do jeito certo: você **nunca** salva a senha em texto.
Em vez disso, salva um **hash** (uma versão embaralhada e irreversível) e, no
login, compara.

```
import hash
```

Duas funções cobrem a autenticação inteira:

| Membro | O que faz | Página |
|---|---|---|
| `hash.crypt(senha)` | gera o hash pra **guardar** no cadastro | [crypt/crypt.md](crypt/crypt.md) |
| `hash.check(hash, senha)` | confere se uma senha **bate** com o hash guardado | [check/check.md](check/check.md) |

E mais três, que são ferramenta de **transporte e impressão digital** — não de
senha:

| Membro | Devolve | O que faz | Página |
|---|---|---|---|
| `hash.sha256(dado)` | `str` de 64 hex | resumo SHA-256 do dado | [sha256/sha256.md](sha256/sha256.md) |
| `hash.b64encode(dado)` | `str` base64 | codifica em base64 padrão, com padding | [b64encode/b64encode.md](b64encode/b64encode.md) |
| `hash.b64decode(texto)` | `str` | decodifica base64 de volta pro conteúdo | [b64decode/b64decode.md](b64decode/b64decode.md) |

---

## O fluxo completo

```
import hash

# CADASTRO — guarde o hash, nunca a senha
senha_hash = hash.crypt("minhaSenha123")
# ... salve senha_hash no banco ...

# LOGIN — compare a senha digitada com o hash guardado
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

## `sha256` não é hash de senha

`hash.sha256()` é rápido e **não tem salt**: a mesma entrada dá sempre a mesma
saída, e quem rouba o banco testa bilhões de candidatas por segundo. Ele serve
pra **identificar conteúdo** (comparar arquivos, chave de cache, assinar
token). Pra senha, o par é `crypt` + `check`, sempre.

---

## Relacionados

- lib `jwt` — gerar tokens de sessão após o login validar
- [jinker/middleware](../jinker/middleware/middleware.md) — proteger rotas com o token
