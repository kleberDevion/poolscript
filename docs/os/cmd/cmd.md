# `os.cmd(command, capture=false)`

Executa um **comando no terminal** do sistema. Por padrão só roda; com
`capture=true`, devolve a saída como texto.

```
os.cmd(command: str, capture: bool = false) -> str | Null
```

| Parâmetro | Padrão | O que é |
|---|---|---|
| `command` | — | a linha de comando a executar (ex: `"python --version"`) |
| `capture` | `false` | `true` = devolve a saída; `false` = só roda e devolve `Null` |

---

## Rodar sem capturar

```
os.cmd("mkdir uploads")           // executa; devolve Null
os.cmd("git status")              // a saída vai direto pro terminal
```

## Capturar a saída

```
versao = os.cmd("python --version", capture=true)
post(versao)                      // "Python 3.14.6"
```

Com `capture=true`, devolve o **stdout** (sem espaços nas pontas). Se o comando
não produziu stdout mas gerou erro, devolve o **stderr** — então você vê a
mensagem de erro em vez de string vazia.

---

## Exemplo prático

```
import os

// checa se uma ferramenta existe
git = os.cmd("git --version", capture=true)
if (git) {
    post("git instalado:", git)
} else {
    post("git não encontrado")
}
```

---

## Cuidado (segurança)

`os.cmd(...)` roda com `shell=true` — o comando é interpretado pelo shell. **Não
monte o comando concatenando entrada não confiável do usuário** (isso permite
injeção de comando): `os.cmd("mkdir " + nome)` com `nome = "x; rm -rf ~"`
executa o `rm`. Para valores dinâmicos vindos de fora, use
[`os.run([...])`](../run/run.md), que roda **sem shell** e trata cada argumento
como texto literal.

---

## Relacionados

- [`os.run()`](../run/run.md) — roda SEM shell (à prova de injeção); prefira para dado de usuário
- [`os.code()`](../code/code.md) — abrir o editor de código
- [`os.mkdir()`](../mkdir/mkdir.md) — criar pasta sem shell (mais seguro que `cmd("mkdir")`)
