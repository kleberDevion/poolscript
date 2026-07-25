# `post(...)`

Imprime no terminal. É o "print" da PoolScript — a função que você mais usa.

```
post(valor1, valor2, ...) -> None
```

---

## Uso

```
post("olá mundo")
post("resultado:", 42)          // vários argumentos, separados por espaço
post(nome, idade, ativo)

x = [1, 2, 3]
post(x)                         // [1, 2, 3]
```

Vários argumentos são impressos juntos, separados por espaço, com quebra de
linha no fim.

---

## `post.flush()` — efeito de digitação

`post.flush(texto, delay)` imprime caractere por caractere, com uma pausa entre
eles — efeito de "digitando":

```
post.flush("Carregando...", delay=0.05)
post.flush("Pronto!", delay=0.08)
```

| Parâmetro | Padrão | O que é |
|---|---|---|
| `texto` | — | o que imprimir |
| `delay` | `0.05` | segundos entre cada caractere |

---

## `post` colorido

Aceita cor inline (ANSI):

```
post(<red>"erro!")
post(<green>"sucesso")
post(<2196f3>"azul por hex")
```

Ver strings coloridas em `LANGUAGE.md`.

---

## `post` vs `sys.stdout.write`

- **`post`** — simples, sempre quebra linha no fim.
- **[`sys.stdout.write`](../../sys/stdout/stdout.md)** — controle fino, sem
  quebra automática (pra barra de progresso, montar linha em pedaços).

---

## Relacionados

- [`input()`](../input/input.md) — o lado da entrada
- [`sys.stdout`](../../sys/stdout/stdout.md) — saída com controle fino
