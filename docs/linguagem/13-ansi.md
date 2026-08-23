# Referência da Linguagem — 13. Cores e ANSI

A PoolScript escreve cor no terminal de duas formas: um **literal de cor**
embutido na linguagem (`<cor>"texto"`), que cobre a cor de **primeiro plano**
(texto) de um jeito seguro e auto-fechado; e as **sequências ANSI manuais**
(via o escape `\e`), para tudo o mais — negrito, sublinhado, fundo, 256 cores.

Verificado nos dois motores (mesmos bytes de saída).

---

## 13.1. Literal de cor — `<cor>"texto"`

Colar uma cor entre `< >` imediatamente antes de uma string tinge **aquela
string**:

![exemplo 1](../assets/linguagem__13-ansi_ex1.png)

<details><summary>código</summary>

```ps
post(<red>"erro!")
post(<2196F3>"azul em hex")
```

</details>

A cor pode ser:

- **um nome** (ver a tabela em 13.2): `<red>`, `<blue>`, …
- **hex de 6 dígitos** (como no CSS): `<2196F3>`
- **hex de 3 dígitos** (expandido, como no CSS `#f00` → `#ff0000`): `<f00>`

4 ou 5 dígitos hex **não** são cor válida (aí o `<…>` volta a ser tratado como
operador de comparação).

### Como funciona

O literal vira uma sequência ANSI **de cor real (24-bit / truecolor)**: emite o
código da cor, o texto, e um **reset** logo depois. Ou seja, `<red>"X"` produz
exatamente:

```
\e[38;2;255;59;48m  X  \e[0m
```

Consequências práticas:

- **Auto-fecha**: a cor vale só para aquela string e **não vaza** para o resto.
  `post(<red>"erro", "normal")` sai com `erro` vermelho e `normal` na cor
  padrão.
- É **cor de texto (primeiro plano)** apenas. Fundo e estilos (negrito etc.)
  são via ANSI manual (13.3).
- A sequência é **sempre emitida** (mesmo com a saída redirecionada a arquivo /
  outro programa) — o literal não detecta terminal sozinho.

---

## 13.2. Cores nomeadas

| Nome | Hex | | Nome | Hex |
|---|---|---|---|---|
| `red` | `FF3B30` | | `orange` | `FF9500` |
| `green` | `34C759` | | `purple` | `AF52DE` |
| `blue` | `2196F3` | | `pink` | `FF2D55` |
| `yellow` | `FFD60A` | | `gray` / `grey` | `8E8E93` |
| `cyan` | `5AC8FA` | | `lime` | `30D158` |
| `magenta` | `FF2D55` | | `teal` | `5AC8FA` |
| `white` | `FFFFFF` | | `black` | `000000` |

(`gray` e `grey` são o mesmo; `pink` coincide com `magenta`, e `teal` com
`cyan`.) Para qualquer outra cor, use o hex direto: `<hexde6>"…"`.

---

## 13.3. ANSI manual — o escape `\e`

Para o que o literal de cor não cobre (negrito, fundo, sublinhado, 256 cores…),
escreva a sequência ANSI na mão. O byte ESC que abre toda sequência é o escape
**`\e`** (equivalente a `\033` e a `\x1b` — o mesmo byte nos dois motores):

![exemplo 2](../assets/linguagem__13-ansi_ex2.png)

<details><summary>código</summary>

```ps
post("\e[1mnegrito\e[0m")
post("\e[4msublinhado\e[0m")
post("\e[41m fundo vermelho \e[0m")
```

</details>

Uma sequência de estilo tem a forma `\e[` + códigos separados por `;` + `m`.
Sempre feche com `\e[0m` (reset), senão o estilo **vaza** para o resto da saída.

### Códigos comuns

| Código | Efeito | | Código | Efeito |
|---|---|---|---|---|
| `0` | reset (limpa tudo) | | `7` | inverte fundo/texto |
| `1` | negrito | | `9` | riscado |
| `2` | fraco (dim) | | `4` | sublinhado |
| `3` | itálico | | `5` | piscando |

**Cor de texto (16):** `30`–`37` (normais) e `90`–`97` (brilhantes) —
`30` preto, `31` vermelho, `32` verde, `33` amarelo, `34` azul, `35` magenta,
`36` ciano, `37` branco.

**Cor de fundo (16):** `40`–`47` e `100`–`107` (mesma ordem de cores).

**256 cores:** texto `38;5;N`, fundo `48;5;N` (N de 0 a 255).

**Cor real (24-bit):** texto `38;2;R;G;B`, fundo `48;2;R;G;B` — é o que o
literal `<cor>` usa por baixo. Dá pra combinar códigos: `\e[1;38;2;255;0;0m` é
negrito + vermelho vivo.

![exemplo 3](../assets/linguagem__13-ansi_ex3.png)

<details><summary>código</summary>

```ps
// negrito + fundo azul + texto branco
post("\e[1;44;97m  título  \e[0m")
```

</details>

> Para colorir com um NOME sem escrever a sequência à mão, o mais direto é o
> literal `<cor>` (13.1). O `\e` é para estilos e fundo, que o literal não faz.

---

## 13.4. Resumo

- **`<cor>"texto"`** — cor de **texto**, por nome ou hex (3/6 dígitos); vira ANSI
  24-bit, **auto-reseta**, não vaza. É a forma segura e curta de colorir.
- **`\e[…m`** (com `\e` = `\033` = `\x1b`) — ANSI manual para **negrito, fundo,
  sublinhado, 256/24-bit**; feche sempre com `\e[0m`.
- 14 cores nomeadas (13.2); qualquer outra, por hex.
