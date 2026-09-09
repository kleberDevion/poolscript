# sys.stdin — ler da entrada padrão

```
sys.stdin.read(tamanho=Null) -> str | Null
sys.stdin.readline()         -> str | Null
```

`sys.stdin` é o que chega pelo terminal ou por um `|`. Três formas de ler, e
todas devolvem **`Null` no fim da entrada** — não string vazia, que se
confundiria com uma linha em branco.

| chamada | devolve |
|---|---|
| `read()` | **tudo** até o fim da entrada, numa `str` só |
| `read(n)` | até `n` caracteres — o que houver |
| `readline()` | a próxima linha **sem** o `\n` do fim |
| fim da entrada, em qualquer uma | `Null` |

O builtin [`input(prompt)`](../../builtins/input/input.md) é o `readline()` com
um prompt na frente — e também devolve `Null` no fim.

---

## Exemplos

Lendo linha a linha até acabar:

```ps
import sys

while True {
    linha = sys.stdin.readline()
    if linha == Null {
        break
    }
    post("li:", linha)
}
```

```
$ printf 'a\nb\n' | pool le.ps
li: a
li: b
```

Tudo de uma vez — o que se faz com entrada por `|`:

```ps
import sys

texto = sys.stdin.read()
post(len(texto.split("\n")), "linhas")
```

---

## Uma tecla, sem Enter

Por padrão o terminal só entrega a linha quando você aperta Enter. Pra ler
tecla por tecla, o terminal entra em modo cru — pelo `stty` do sistema, que
age no mesmo terminal do programa:

```ps
import os
import sys

os.cmd("stty -icanon -echo min 1 time 0")   # tecla por tecla, sem eco
tecla = sys.stdin.read(1)                   # volta na hora, sem Enter
os.cmd("stty sane")                         # SEMPRE devolver o terminal
post("apertou:", tecla)
```

Quem não devolve com `stty sane` deixa o terminal do usuário sem eco depois
que o programa sai. Ponha o `sane` num `finally`.

As setas chegam como três caracteres: `\x1b`, `[` e `A`/`B`/`C`/`D` (cima,
baixo, direita, esquerda). Leia `read(1)` três vezes quando o primeiro for
`\x1b`.

---

## Ler tecla sem travar o programa

`read(1)` espera a tecla. Dentro de um `async funct` ele **cede**: o resto do
programa continua — é o que um jogo no terminal precisa.

```ps
import sys

async funct tecla() {
    return sys.stdin.read(1)
}

async funct laco() {
    for each i in range(4) {
        post("tique", i)
        sleep(0.3)
    }
    return "fim"
}

post(gather(tecla(), laco()))
```

```
$ (sleep 1; printf 'k') | pool jogo.ps
tique 0
tique 1
tique 2
tique 3
['k', 'fim']
```

Os quatro `tique` saíram **enquanto** a tecla ainda não tinha chegado. Uma
cobrinha é isto: uma fibra guardando a última direção lida, outra com
`sleep → move → redesenha` (cor e cursor em ANSI: `"\x1b[2J"` limpa,
`"\x1b[H"` volta ao canto, `"\x1b[31m"` vermelho), e `gather` das duas.

---

## Relacionados

- [`input()`](../../builtins/input/input.md) — `readline()` com prompt
- [`sys.stdout`](../stdout/stdout.md) — escrever; `write(texto, end)`,
  `writeln(texto)`, `flush()`
- [`sys.stderr`](../stderr/stderr.md) — a saída de erro
- [`os.cmd`](../../os/cmd/cmd.md) — o `stty` acima

[← sys](../sys.md)
