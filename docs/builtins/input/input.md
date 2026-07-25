# `input(prompt="")`

Lê uma linha digitada pelo usuário no terminal. **Sempre devolve string.**

```
input(prompt: str = "") -> str
```

---

## Uso

```
nome = input("Seu nome: ")
post(f"olá, {nome}")

linha = input()              // sem prompt
```

---

## Sempre string — converta se precisar de número

`input` devolve texto, mesmo que o usuário digite um número. Pra usar como
número, converta — ou declare o tipo, que converte automático:

```
texto = input("idade: ")         // "25" (string)
idade = int(input("idade: "))    // 25 (int)

int idade = input("idade: ")     // declarar o tipo já converte
```

---

## Relacionados

- [`post()`](../post/post.md) — imprimir (o lado da saída)
- builtin `int`/`flo` (em [conversores](../conversores/conversores.md)) — converter o texto lido
