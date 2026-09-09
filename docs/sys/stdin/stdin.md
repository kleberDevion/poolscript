# sys.stdin — ler da entrada padrão

```
sys.stdin.read(tamanho=Null) -> str | Null
sys.stdin.readline()         -> str | Null
sys.stdin.raw(ligar)         -> bool
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

## Modo cru — `raw()`: tecla na hora, sem Enter

Por padrão o terminal só entrega a linha quando você aperta Enter, e ecoa o que
você digita. `sys.stdin.raw(true)` põe o terminal em **modo cru** — a tecla
chega na hora, sem eco — e `raw(false)` volta ao normal. É do próprio motor
(termios), sem `stty` externo.

```ps
import sys

sys.stdin.raw(true)
tecla = sys.stdin.read(1)     # volta na hora, sem Enter, sem aparecer na tela
sys.stdin.raw(false)
post("apertou:", tecla)
```

- Devolve `true` se ligou, `false` quando **não há terminal** (entrada por `|`
  ou arquivo) — aí não há modo cru pra ligar.
- O motor **restaura o terminal sozinho** ao fim do programa e num `Ctrl+C`,
  mesmo que o script esqueça o `raw(false)` ou quebre no meio. Ainda assim,
  ponha o `raw(false)` num `finally` — é o certo.
- `Ctrl+C` continua encerrando (o modo é *cbreak*, não raw total).

As setas chegam como três caracteres: `\x1b`, `[` e `A`/`B`/`C`/`D` (cima,
baixo, direita, esquerda). Leia `read(1)` mais duas vezes quando o primeiro for
`\x1b`.

---

## Ler tecla sem travar — o laço de um jogo

Em modo cru, `read(1)` **não bloqueia**: se não há tecla, devolve `""` (string
vazia), não `Null`. `Null` fica reservado pro fim real da entrada. Assim um
laço só — ler, mover, desenhar, dormir — roda livre e só reage quando há tecla:

```ps
import sys

sys.stdin.raw(true)
n = 0
while n < 40 {
    t = sys.stdin.read(1)
    if t == "q" {
        break
    }
    if t != "" {
        post("apertou:", t)
    }
    sleep(0.1)               # o resto do jogo: mover, desenhar
    n = n + 1
}
sys.stdin.raw(false)
```

Uma cobrinha completa, com esse laço e desenho em ANSI (`"\x1b[2J"` limpa,
`"\x1b[H"` volta ao canto, `"\x1b[31m"` cor), está em
[`examples/cobrinha.ps`](../../../examples/cobrinha.ps).

---

## Relacionados

- [`input()`](../../builtins/input/input.md) — `readline()` com prompt
- [`sys.stdout`](../stdout/stdout.md) — escrever; `write(texto, end)`,
  `writeln(texto)`, `flush()`
- [`sys.stderr`](../stderr/stderr.md) — a saída de erro

[← sys](../sys.md)
