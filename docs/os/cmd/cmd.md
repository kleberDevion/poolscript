# `os.cmd(command, capture=False)`

Executa um **comando no terminal** do sistema. Por padrão só roda; com
`capture=True`, devolve a saída como texto. Nos dois modos ele **espera** o
comando terminar, e a espera cede: numa rota do jinker ou numa `async funct`,
o resto continua sendo atendido enquanto o comando roda.

```assinatura
os.cmd(command: str, capture: bool = False) -> str | Null
```

| Parâmetro | Padrão | O que é |
|---|---|---|
| `command` | — | a linha de comando a executar (ex: `"jinga --version"`) |
| `capture` | `False` | `True` = devolve a saída; `False` = só roda e devolve `Null` |

O shell é o do sistema: `/bin/sh -c` no Linux e no macOS, `cmd.exe /d /s /c`
no Windows. A linha tem que ser na sintaxe daquele shell (no Windows, `dir` e
`%VAR%`, não `ls` e `$VAR`). Um comando inexistente sai com 127 no `sh` e 9009
no `cmd.exe`.

Os exemplos desta página pressupõem:

```ps contexto
import os
```

---

## Rodar sem capturar

```ps
os.cmd("mkdir uploads")           # executa; devolve Null
os.cmd("git status")              # a saída vai direto pro terminal
```

## Capturar a saída

```ps
versao = os.cmd("jinga --version", capture=True)
post(versao)                      # "Jinga 19.0.3 [PSVM] (2026-09-24) Runtime standalone"
```

Com `capture=True`, devolve o **stdout** (sem espaços nas pontas). Se o comando
não produziu stdout mas gerou erro, devolve o **stderr** — então você vê a
mensagem de erro em vez de string vazia.

---

## Exemplo prático

```ps
import os

# checa se uma ferramenta existe — pelo código de saída, não pelo texto
achou = os.cmd("command -v git > /dev/null; echo $?", capture=True).strip()
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

`os.cmd(...)` roda com `shell=True` — o comando é interpretado pelo shell. **Não
monte o comando concatenando entrada não confiável do usuário** (isso permite
injeção de comando): `os.cmd("mkdir " + nome)` com `nome = "x; rm -rf ~"`
executa o `rm`. Para valores dinâmicos vindos de fora, use
[`os.run([...])`](../run/run.md), que roda **sem shell** e trata cada argumento
como texto literal.

Comando com o byte nulo (`\x00`) é `ValueError` antes de rodar: o shell
receberia só o pedaço de antes dele (`o comando tem o byte nulo \x00 na
posicao N: nenhum argumento de programa tem esse caractere — '...'`).

---

## Relacionados

- [`os.run()`](../run/run.md) — roda SEM shell (à prova de injeção); prefira para dado de usuário
- [`os.code()`](../code/code.md) — abrir o editor de código
- [`os.mkdir()`](../mkdir/mkdir.md) — criar pasta sem shell (mais seguro que `cmd("mkdir")`)
