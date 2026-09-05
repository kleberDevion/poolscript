# `os.run(args, capture=false)`

Executa um comando **sem shell** — cada argumento é uma entrada separada da
lista, então caracteres de shell (`;`, `|`, `$`, `&`, `>`) viram **texto
literal** e não conseguem injetar comando. É a forma **segura** de rodar
programa com valor vindo do usuário. Por padrão só roda; com `capture=true`,
devolve a saída como texto.

```
os.run(args: list | str, capture: bool = false) -> str | Null
```

| Parâmetro | Padrão | O que é |
|---|---|---|
| `args` | — | lista `["programa", "arg1", "arg2"]` (recomendado) ou string |
| `capture` | `false` | `true` = devolve a saída; `false` = só roda e devolve `Null` |

---

## Rodar sem capturar

```
os.run(["mkdir", "uploads"])        # executa; devolve Null
os.run(["git", "status"])           # a saída vai direto pro terminal
```

## Capturar a saída

```
versao = os.run(["python", "--version"], capture=true)
post(versao)                        # "Python 3.14.6"
```

Com `capture=true`, devolve o **stdout** (sem espaços nas pontas). Se o comando
não produziu stdout mas gerou erro, devolve o **stderr** — igual ao `os.cmd`.

---

## Por que `run` em vez de `cmd`: segurança

`os.run` NÃO passa pelo shell, então metacaracteres não são interpretados —
são só texto. Compare:

```
nome = "x; rm -rf ~"                # valor malicioso vindo de fora

os.cmd("mkdir " + nome)             # ⚠️ o shell executa o `rm -rf ~`
os.run(["mkdir", nome])             # seguro: cria uma pasta chamada
                                    #   literalmente "x; rm -rf ~"
```

Regra prática: **dado do usuário no comando → sempre `os.run([...])`**. Só use
[`os.cmd`](../cmd/cmd.md) quando precisar de recurso de shell (pipe,
redirecionamento, variável) numa string que **você** controla.

---

## Forma com string

Também aceita string, dividida respeitando aspas, **sem** interpretar shell:

```
os.run("python -c \"print(1+1)\"", capture=true)   # "2"
```

A forma com lista é a recomendada — não depende das regras de divisão por
aspas.

---

## Programa inexistente → erro

Se o programa não existe, `os.run` **levanta `IOError`** (não devolve `Null`) —
o processo nem chega a rodar. Trate com `try`/`catch`:

```
import os

action versao_de(programa) {
    try {
        return os.run([programa, "--version"], capture=true)
    } catch (IOError e) {
        return programa + " não encontrado"
    }
}

post(versao_de("git"))          # ex: "git version 2.43.0"
post(versao_de("nao_existe"))   # "nao_existe não encontrado"
```

Diferença importante para o [`os.cmd`](../cmd/cmd.md): como o `cmd` passa pelo
shell, um comando inexistente **não** vira `IOError` — o shell roda, não acha, e
`cmd` devolve `Null` (a mensagem "command not found" vai pro stderr).

---

## Relacionados

- [`os.cmd()`](../cmd/cmd.md) — roda COM shell (interpreta `;` `|` `$`); use só com comando confiável
- [`os.code()`](../code/code.md) — abrir o editor de código
- [`os.mkdir()`](../mkdir/mkdir.md) — criar pasta direto, sem processo externo
