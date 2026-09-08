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
| `capture` | `false` | `true` = **espera** e devolve a saída; `false` = dispara em segundo plano e devolve o **PID** |

---

## Rodar em segundo plano — o padrão

Sem `capture`, o `run` **dispara e volta na hora**. É o que separa `run` de
[`cmd`](../cmd/cmd.md): o `cmd` espera, o `run` não.

```
pid = os.run(["sleep", "3"])
post("já estou aqui:", pid)     # imprime na hora, não depois de 3s
```

O que volta é o **PID** (`int`) do processo, pra acompanhar ou matar:

```
pid = os.run(["ffmpeg", "-i", entrada, saida])
os.run(["kill", str(pid)])      # se precisar interromper
```

O processo é **solto do terminal** (`setsid`) e sobrevive ao fim do programa
que o disparou — quem quiser esperar, espera pelo PID. E não deixa zumbi: o
disparo é por fork duplo, então o processo é adotado pelo init e ninguém
precisa recolher o código de saída.

A saída dele vai pro mesmo terminal do programa. Pra mandar pra outro lugar,
redirecione no próprio comando (`["sh", "-c", "prog > log.txt 2>&1"]`) — mas
aí é shell, e vale a advertência de injeção lá embaixo.

## Capturar a saída — aí espera

```
versao = os.run(["pool", "--version"], capture=true)
post(versao)                        # "PoolScript 8.4.4 [PSVM]"
```

Com `capture=true` o `run` **espera** o processo terminar: colher a saída exige
o fim dele. Devolve o **stdout** (sem espaços nas pontas); se o comando não
produziu stdout mas gerou erro, devolve o **stderr** — igual ao `os.cmd`.

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
os.run("pool -e \"post(1+1)\"", capture=true)   # "2"
```

A forma com lista é a recomendada — não depende das regras de divisão por
aspas.

---

## Programa inexistente → erro

Se o programa não existe, `os.run` **levanta `IOError`** (não devolve `Null`) —
o processo nem chega a rodar. Trate com `try`/`catch`:

```
import os

funct versao_de(programa) {
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
