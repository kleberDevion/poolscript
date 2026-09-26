# `os.run(args, capture=false, pty=false)`

Executa um comando **sem shell** — cada argumento é uma entrada separada da
lista, então caracteres de shell (`;`, `|`, `$`, `&`, `>`) viram **texto
literal** e não conseguem injetar comando. É a forma **segura** de rodar
programa com valor vindo do usuário.

```
os.run(args: list | str, capture: bool | str = false, pty: bool = false) -> int | str | Process
```

| Parâmetro | Padrão | O que é |
|---|---|---|
| `args` | — | lista `["programa", "arg1", "arg2"]` (recomendado) ou string |
| `capture` | `false` | `false` = dispara em segundo plano e devolve o **PID**; `true` = **espera** e devolve a saída; `"live"` = devolve o [`Process`](../Process/Process.md), pra conversar com ele enquanto roda |
| `pty` | `false` | `true` = o filho roda num **terminal de verdade**; precisa de `capture="live"` ou `capture=true` |

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
que o disparou. E não deixa zumbi: o disparo é por fork duplo, então o processo
é adotado pelo init e ninguém precisa recolher o código de saída — o outro lado
disso é que **não dá pra esperar por esse PID**, nem saber como ele terminou.
Quem precisa disso usa `capture="live"`, abaixo.

A saída dele vai pro mesmo terminal do programa. Pra mandar pra outro lugar,
redirecione no próprio comando (`["sh", "-c", "prog > log.txt 2>&1"]`) — mas
aí é shell, e vale a advertência de injeção lá embaixo.

## Capturar a saída — aí espera

```
versao = os.run(["jinga", "--version"], capture=true)
post(versao)                        # "Jinga 16.0.2 [PSVM] (2026-09-24) Runtime standalone"
```

Com `capture=true` o `run` **espera** o processo terminar: colher a saída exige
o fim dele. Devolve o **stdout** (sem espaços nas pontas); se o comando não
produziu stdout mas gerou erro, devolve o **stderr** — igual ao `os.cmd`. A
espera **cede**: dentro de uma rota do jinker ou de uma `async funct`, as
outras requisições continuam sendo atendidas enquanto o comando roda.

---

## `capture="live"` — o processo vivo

Os dois modos acima resolvem "dispare e esqueça" e "rode e me dê o texto no
fim". O que eles não fazem é **conversar** com o processo: escrever na entrada
dele, ler o que ele já imprimiu, saber o código de saída, matar. Isso é
[`Process`](../Process/Process.md):

```
p = os.run(["jinga", "prog.pr"], capture="live")
p.write("sim\n")                 # vai pro stdin do filho
p.close()                        # fecha a entrada (fim de arquivo pra ele)
post(p.readline())               # a primeira linha que ele imprimir
post(p.wait())                   # o código de saída
```

Com `using`, os canais fecham no fim do bloco:

```
using os.run(["wc", "-l"], capture="live") as p {
    p.write("a\nb\nc\n")
    p.close()
    post(p.read().strip())       # "3"
}
```

## `pty=true` — o filho num terminal

Programa interativo se comporta diferente quando não está num terminal: some a
cor, o `input()` não aparece, a barra de progresso vira lixo. Com `pty=true` o
filho ganha um terminal de verdade:

```
p = os.run(["sh", "-c", "test -t 0 && echo tem terminal"], capture="live", pty=true)
post(p.read().strip())           # "tem terminal"
```

Um terminal tem **um canal só**: com `pty=true`, o stderr do filho chega junto
no `read`, e `read_err` devolve `""`. O tamanho começa em 80x24 e muda com
[`resize`](../Process/Process.md).

`pty=true` com `capture=false` é `ValueError`: ninguém leria o terminal, e o
filho travaria com o buffer cheio.

---

## Por que `run` em vez de `cmd`: segurança

`os.run` NÃO passa pelo shell, então metacaracteres não são interpretados —
são só texto. Compare:

```
nome = "x; rm -rf ~"                # valor malicioso vindo de fora

os.cmd("mkdir " + nome)             # PERIGO: o shell executa o `rm -rf ~`
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
os.run("jinga -e \"post(1+1)\"", capture=true)   # "2"
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
o `cmd` devolve `Null` **sem `capture`**; com `capture=true` ele devolve o
texto do stderr do shell ("command not found"), porque o stdout veio vazio.

---

## Relacionados

- [`os.cmd()`](../cmd/cmd.md) — roda COM shell (interpreta `;` `|` `$`); use só com comando confiável
- [`os.code()`](../code/code.md) — abrir o editor de código
- [`os.mkdir()`](../mkdir/mkdir.md) — criar pasta direto, sem processo externo
