# `Process` — o processo filho vivo

O que `os.run(args, capture="live")` devolve: o processo em si, ainda rodando.
Serve pra escrever na entrada dele, ler a saída **enquanto** ele trabalha,
esperar o código de saída e matar.

```
import os

p = os.run(["pool", "prog.pr"], capture="live")
```

Os outros dois modos do [`os.run`](../run/run.md) não fazem isso: `capture=false`
dispara e esquece, `capture=true` só devolve o texto no fim.

---

## Membros

| Membro | Devolve | O que faz |
|---|---|---|
| `p.pid` | `int` | o PID do filho (campo, sem parênteses) |
| `p.returncode` | `Null` \| `int` | `Null` enquanto roda; depois, o código de saída |
| `p.write(texto)` | `int` | escreve no stdin do filho; devolve quantos bytes foram |
| `p.close()` | `bool` | fecha a entrada: o filho vê fim de arquivo |
| `p.read(n=4096)` | `str` | até `n` bytes do que o filho já imprimiu |
| `p.readline()` | `str` | uma linha do stdout, com a quebra |
| `p.read_err(n=4096)` | `str` | o mesmo, do stderr |
| `p.wait()` | `int` | espera o filho terminar e devolve o código |
| `p.kill(sinal=15)` | `bool` | manda um sinal (15 = SIGTERM, 9 = SIGKILL) |
| `p.resize(colunas, linhas)` | `bool` | muda o tamanho do terminal (só com `pty=true`) |

---

## Ler enquanto roda

`read` e `readline` esperam só até **chegar** alguma coisa — não até o processo
acabar. É o que permite mostrar a saída de um build linha a linha:

```
p = os.run(["make"], capture="live")
linha = p.readline()
while linha != "" {
    post(linha.strip())
    linha = p.readline()
}
post("terminou com", p.wait())
```

No fim da saída, `read` e `readline` devolvem `""` — é assim que se sabe que
acabou.

Um caractere de vários bytes (acento, ideograma) que chegar partido entre duas
leituras **não sai pela metade**: o `read` guarda o pedaço e espera o resto.

## Escrever na entrada

```
using os.run(["sort"], capture="live") as p {
    p.write("banana\nabacaxi\n")
    p.close()                    # sem isto o `sort` espera mais entrada
    post(p.read().strip())
}
```

`close()` é o que diz "acabou a entrada". Com `pty=true` ele manda o caractere
de fim (o mesmo `Ctrl+D` do teclado) e o canal continua aberto pra leitura.

## Esperar, matar e o código

```
p = os.run(["sleep", "30"], capture="live")
post(p.returncode)     # Null — ainda roda
p.kill()               # SIGTERM
post(p.wait())         # -15: morreu pelo sinal 15
```

O código de saída é o número que o programa devolveu (`0` é sucesso). Quando
ele morre por um **sinal**, o código é `-sinal` — `p.kill()` dá `-15`, e
`p.kill(9)` dá `-9`.

`wait()` cede enquanto espera: numa rota do jinker, as outras requisições
continuam sendo atendidas.

## Terminal (`pty=true`)

```
p = os.run(["htop"], capture="live", pty=true)
p.resize(120, 40)
```

`resize` só existe com terminal; sem `pty` ele levanta `RuntimeError`. Depois
de mudar o tamanho, o filho recebe o aviso e se redesenha.

---

## Fim de vida

Os canais fecham sozinhos quando o objeto é coletado, e o `using` os fecha no
fim do bloco. O filho **não** é morto junto: se ele ainda estiver rodando, seu
código de saída é recolhido pelo motor quando ele terminar, sem virar zumbi.

---

## Relacionados

- [`os.run()`](../run/run.md) — quem cria o processo
- [`os.cmd()`](../cmd/cmd.md) — a forma com shell, que sempre espera
