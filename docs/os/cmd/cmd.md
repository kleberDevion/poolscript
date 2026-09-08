# `os.cmd(command, capture=false)`

Executa um **comando no terminal** do sistema. Por padrão só roda; com
`capture=true`, devolve a saída como texto.

```
os.cmd(command: str, capture: bool = false) -> str | Null
```

| Parâmetro | Padrão | O que é |
|---|---|---|
| `command` | — | a linha de comando a executar (ex: `"pool --version"`) |
| `capture` | `false` | `true` = devolve a saída; `false` = só roda e devolve `Null` |

---

## Rodar sem capturar

```
os.cmd("mkdir uploads")           # executa; devolve Null
os.cmd("git status")              # a saída vai direto pro terminal
```

## Capturar a saída

```
versao = os.cmd("pool --version", capture=true)
post(versao)                      # "PoolScript 8.4.3 [PSVM]"
```

Com `capture=true`, devolve o **stdout** (sem espaços nas pontas). Se o comando
não produziu stdout mas gerou erro, devolve o **stderr** — então você vê a
mensagem de erro em vez de string vazia.

---

## Exemplo prático

```
import os

# checa se uma ferramenta existe — pelo código de saída, não pelo texto
achou = os.cmd("command -v git > /dev/null; echo $?", capture=true).strip()
if achou == "0" {
    post("git instalado")
} else {
    post("git não encontrado")
}
```

> **Não teste com `if (os.cmd(...))`.** Sem stdout o `cmd` devolve o **stderr**
> (regra das linhas acima), e `sh: 1: gitzz: not found` é string não-vazia:
> o `if` é verdadeiro mesmo quando o programa não existe, e o `else` nunca
> roda. Por isso o exemplo pergunta o código de saída.

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
