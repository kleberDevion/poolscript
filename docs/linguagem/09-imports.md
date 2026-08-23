# Referência da Linguagem — 9. Imports

Um `.ps` pode usar código de outro arquivo `.ps` ou de uma biblioteca — da
stdlib (embutida) ou instalada. Esta seção cobre as formas de import
(`import`, `from … import`, `PUSH … GET`), o `as`, os imports relativos e a
ordem em que um nome é resolvido.

Verificado nos dois motores.

---

## 9.1. `import` — o módulo inteiro

![exemplo 1](../assets/linguagem__09-imports_ex1.png)

<details><summary>código</summary>

```ps
import mymod

post(mymod.saudar("ana"))    // acesso por ponto
post(mymod.valor)
```

</details>

`import <modulo>` executa o módulo (uma vez) e liga o **nome do módulo** no
escopo atual; tudo que ele define é acessado por `modulo.membro`.

Com **`as`**, o módulo ganha outro nome local:

![exemplo 2](../assets/linguagem__09-imports_ex2.png)

<details><summary>código</summary>

```ps
import mymod as m
post(m.valor)
```

</details>

Um caminho com pontos importa submódulos: `import pacote.modulo` (o nome ligado é
o último segmento, ou o `as`).

---

## 9.2. `from … import` — nomes específicos

Traz **nomes soltos** do módulo direto pro escopo (sem o prefixo):

![exemplo 3](../assets/linguagem__09-imports_ex3.png)

<details><summary>código</summary>

```ps
from mymod import saudar, valor
post(saudar("bia"), valor)

from mymod import saudar as oi      // com apelido
post(oi("ze"))

from mymod import Ponto             // funções, Entities, constantes — tudo que o módulo exporta
p = Ponto(1, 2)
```

</details>

Pedir um nome que o módulo não exporta é erro
(`módulo '…' não exporta '…'`).

---

## 9.3. `PUSH … GET`

`PUSH` é uma forma alternativa de import:

- **`PUSH <modulo> [as <nome>]`** — igual a `import` (liga o módulo inteiro).
- **`PUSH <modulo> GET <x>, <y>`** — igual a `from <modulo> import <x>, <y>`
  (liga só os nomes listados).

![exemplo 4](../assets/linguagem__09-imports_ex4.png)

<details><summary>código</summary>

```ps
PUSH mymod                 // == import mymod
post(mymod.valor)

PUSH mymod GET valor       // == from mymod import valor
post(valor)
```

</details>

---

## 9.4. Imports relativos

Prefixar o módulo com pontos importa **relativo à pasta do arquivo atual** (como
no Python), sem passar pela stdlib:

![exemplo 5](../assets/linguagem__09-imports_ex5.png)

<details><summary>código</summary>

```ps
from .modulo import x        // mesma pasta
from ..pacote.modulo import y   // um nível acima
```

</details>

Cada `.` extra sobe um diretório. Um relativo não encontrado é
`ImportError` (`módulo relativo não encontrado: …`).

---

## 9.5. Ordem de resolução

Para um `import nome` **sem pontos**, a busca segue esta ordem — a primeira que
casar vence:

1. **stdlib** — as bibliotecas embutidas (`import json`, `import os`,
   `import sys`, `import request`, `import jinker`, …).
2. **lib instalada globalmente** — o que foi instalado com
   `psl install … -asLib` (em `~/.poolscript/libs/`). Vem **antes** dos
   arquivos locais: o nome de um arquivo seu nunca ofusca uma lib instalada.
3. **arquivo `.ps` do projeto** — resolvido a partir da raiz do projeto
   (ex.: `from services.smtp import x` → `<raiz>/services/smtp.ps`).

Um nome que não casa com nenhum dos três é `ImportError`.

> Imports relativos (`from .x import …`) **não** entram nessa ordem — são sempre
> resolvidos direto contra o sistema de arquivos, relativos ao arquivo atual.

---

## 9.6. Import não roda o `run_selfwith_`

Quando um arquivo é **importado**, o bloco `run_selfwith_("main"):` dele **não
executa** (só roda quando o arquivo é o principal — seção 5.7). Assim, importar
um módulo traz as definições (actions, Entities, constantes) sem disparar o
ponto de entrada:

![exemplo 6](../assets/linguagem__09-imports_ex6.png)

<details><summary>código</summary>

```ps
// mymod.ps
action saudar(nome):
    return "ola " + nome
run_selfwith_("main"):
    post("só quando rodo o mymod direto")

// outro.ps
import mymod                 // NÃO imprime a linha do run_selfwith_
post(mymod.saudar("ana"))
```

</details>

---

## 9.7. Resumo

- **`import mod [as m]`** — liga o módulo; acesso por `mod.x`.
- **`from mod import x [as y], z`** — liga nomes soltos.
- **`PUSH mod [as m] [GET x, y]`** — alternativa: `PUSH` = `import`, `GET` =
  `from … import`.
- **`from .mod` / `from ..pkg.mod`** — relativo ao arquivo atual.
- Resolução (sem pontos): **stdlib → lib global → arquivo do projeto**;
  senão `ImportError`.
- **Importar não dispara** o `run_selfwith_` do módulo.
