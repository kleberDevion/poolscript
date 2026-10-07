# `sys.exit(code=0)`

Encerra o programa com um código de saída. Antes de sair, os `finally` (e os
`using`) em volta rodam, de dentro pra fora; nenhum `catch` o pega, nem o sem
tipo (seção 10.2). Na rota do `jinker`, encerra o servidor na hora.

```
sys.exit(code: int = 0) -> Null
```

| Parâmetro | Padrão | O que é |
|---|---|---|
| `code` | `0` | código de saída: `0` = sucesso, diferente de `0` = erro |

---

## Uso

```
import sys
import os

if (not os.exists("config.json")) {
    post("config não encontrado")
    sys.exit(1)                # encerra com erro
}

# ... segue o programa ...
sys.exit()                     # encerra com sucesso (código 0)
```

---

## Com `finally`

```ps
import sys

try {
    post("trabalhando")
    sys.exit(3)
} catch (e) {
    post("nao roda: o catch nao pega o exit")
} finally {
    post("finally roda antes de sair")
}
post("nao roda")
```

Imprime `trabalhando` e `finally roda antes de sair`, e o programa sai com 3.
No `jinga build`, o `sys.exit` encerra só o arquivo dele: o build segue com
os outros, e `sys.exit(0)` conta como sucesso.

---

## Convenção dos códigos

- **`0`** — tudo certo (sucesso). É o padrão.
- **diferente de `0`** — algo deu errado. Scripts e o sistema usam isso pra
  saber se seu programa falhou (ex: `1` = erro genérico).

---

## Relacionados

- [`sys.stderr`](../stderr/stderr.md) — escrever a mensagem de erro antes de sair
