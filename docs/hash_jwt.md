# hash e jwt — Autenticação

---

## hash — Hash de senhas

```
import hash
```

### hash.crypt()

Gera hash seguro de uma senha com salt aleatório:

```
str senha = "minha_senha_123"
str senha_hash = hash.crypt(senha)
post(senha_hash)
# bWn9mPzs3QMwzoQBKUOq0tIKB5LUj7oRGN+KNLqhpqVfbUhWNhXLBs...
```

Nunca guarde a senha original no banco — sempre o hash.

### hash.check()

Verifica se a senha digitada bate com o hash salvo:

```
ok = hash.check(senha_hash, "minha_senha_123")
post(ok)   # True

errado = hash.check(senha_hash, "senha_errada")
post(errado)  # False
```

### Exemplo — cadastro e login

```
import psodbc
import hash

action cadastrar(nome, email, senha) {
    str senha_hash = hash.crypt(senha)

    psodbc.query(
        base="banco.db",
        cmd=("INSERT INTO @t (nome, email, senha) VALUES (?, ?, ?)",
             (nome, email, senha_hash)),
        table="users"
    )
}

action logar(email, senha) {
    result = psodbc.query(
        base="banco.db",
        cmd=("SELECT * FROM @t WHERE email = ?", (email,)),
        table="users"
    )

    if (result and hash.check(result[0]["senha"], senha)) {
        return result[0]
    } else {
        return None
    }
}
```

---

## jwt — Tokens de autenticação

```
import jwt
import date
```

### jwt.gen()

Gera um token JWT:

```
payload = {
    "user_id": 1,
    "email": "ana@email.com",
    "exp": date.timestamp() + date.hora(hours=24)
}

str token = jwt.gen(payload, "minha_chave_secreta", algorithm="HS256")
post(token)
# eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...
```

**Parâmetros:**

| Parâmetro | Descrição |
|---|---|
| `payload` | Dict com os dados do token |
| `secret` | Chave secreta — guarde no `.env` |
| `algorithm` | Algoritmo de assinatura (default `"HS256"`) |

O campo `exp` define quando o token expira. Use `date.timestamp() + date.hora(hours=N)`.

### jwt.check()

Verifica o token e retorna o payload:

```
payload = jwt.check(token, "minha_chave_secreta")

if (payload) {
    post(payload["user_id"])
    post(payload["email"])
} else {
    post("token inválido ou expirado")
}
```

Retorna `None` se o token for inválido ou expirado.

---

## date.hora() — para o JWT

```
import date

# 24 horas em segundos
exp = date.timestamp() + date.hora(hours=24)

# 30 minutos
exp = date.timestamp() + date.hora(minutes=30)

# 7 dias
exp = date.timestamp() + date.hora(days=7)
```

---

## Exemplo completo com middleware JWT

```
import jwt
import hash
import psodbc
import date
import os
from dotenv import load
from jinker import Jinker, cors, jsonify

load()

str SECRET = os.getenv("SECRET_KEY")
str DB = os.getenv("DB_PATH")

app = Jinker(__name__)
cors(options=["POST", "GET"], permiser=["*/api", "allowed.all/Users-Agent"])

@app.middleware()
action auth() {
    token = request.get("token")
    if (not token) {
        return jsonify({"msg": "não autorizado"}), 401
    }
    payload = jwt.check(token, SECRET)
    if (not payload) {
        return jsonify({"msg": "token inválido ou expirado"}), 401
    }
    continue
}

@app.route("/api/login", auth=cors.permiser(), methods=cors.options(["POST"]))
action login() {
    data = request.get_json()
    email = data.get("email")
    senha = data.get("senha")

    result = psodbc.query(
        base=DB,
        cmd=("SELECT * FROM @t WHERE email = ?", (email,)),
        table="users"
    )

    if (result and hash.check(result[0]["senha"], senha)) {
        payload = {
            "user_id": result[0]["id"],
            "email": result[0]["email"],
            "exp": date.timestamp() + date.hora(hours=24)
        }
        token = jwt.gen(payload, SECRET, algorithm="HS256")
        return jsonify({
            "msg": "login feito!",
            "token": token,
            "user_id": result[0]["id"]
        }), 200
    } else {
        return jsonify({"msg": "email ou senha incorretos"}), 401
    }
}

@app.route("/api/perfil", auth=cors.permiser(), methods=cors.options(["GET"]), middleware=app.middleware)
action perfil() {
    return jsonify({"msg": "área protegida — token válido!"}), 200
}

if __name__ == "main" {
    app(debug=False, host="0.0.0.0", port=7700)
}
```
