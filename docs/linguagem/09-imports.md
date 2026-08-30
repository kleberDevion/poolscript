# Referência da Linguagem — 9. Imports

Um `.ps` pode usar código de outro arquivo `.ps` ou de uma biblioteca — da
stdlib (embutida) ou instalada. Esta seção cobre as formas de import
(`import`, `from … import`, `PUSH … GET`), o `as`, os imports relativos e a
ordem em que um nome é resolvido.

Verificado na VM.

---

## 9.1. `import` — o módulo inteiro

```ps
import mymod

post(mymod.saudar("ana"))    // acesso por ponto
post(mymod.valor)
```

`import <modulo>` executa o módulo (uma vez) e liga o **nome do módulo** no
escopo atual; tudo que ele define é acessado por `modulo.membro`.

Com **`as`**, o módulo ganha outro nome local:

```ps
import mymod as m
post(m.valor)
```

Um caminho com pontos importa submódulos: `import pacote.modulo` (o nome ligado é
o último segmento, ou o `as`).

---

## 9.2. `from … import` — nomes específicos

Traz **nomes soltos** do módulo direto pro escopo (sem o prefixo):

```ps
from mymod import saudar, valor
post(saudar("bia"), valor)

from mymod import saudar as oi      // com apelido
post(oi("ze"))

from mymod import Ponto             // funções, Entities, constantes — tudo que o módulo exporta
p = Ponto(1, 2)
```

Pedir um nome que o módulo não exporta é erro, e o **tipo depende de como você
pediu** — a mesma separação do Python:

```ps
from json import naotem      // ImportError: cannot import name 'naotem' from 'json' (unknown location)

import json
post(json.naotem)            // AttributeError: module 'json' has no attribute 'naotem'
```

O `ImportError` cita o **arquivo** do módulo entre parênteses; módulo nativo,
que não tem arquivo, sai como `(unknown location)`. O `AttributeError` é o que
traz a sugestão de nome parecido (`Did you mean: 'parse'?`) — o `ImportError`
não sugere, como no Python. Nome que existe mas é `private` sai como
`AttributeError: module '…' has no attribute '…' (existe, mas é private)`.

---

## 9.3. `PUSH … GET`

`PUSH` é uma forma alternativa de import:

- **`PUSH <modulo> [as <nome>]`** — igual a `import` (liga o módulo inteiro).
- **`PUSH <modulo> GET <x>, <y>`** — igual a `from <modulo> import <x>, <y>`
  (liga só os nomes listados).

```ps
PUSH mymod                 // == import mymod
post(mymod.valor)

PUSH mymod GET valor       // == from mymod import valor
post(valor)
```

---

## 9.4. Imports relativos

Prefixar o módulo com pontos importa **relativo à pasta do arquivo atual** (como
no Python), sem passar pela stdlib:

```ps
from .modulo import x        // mesma pasta
from ..pacote.modulo import y   // um nível acima
```

Cada `.` extra sobe um diretório. Um relativo não encontrado é `ImportError` —
a mesma mensagem do absoluto (`No module named '.x'`); não há texto próprio pro
relativo.

> O tipo é `ImportError`, não `ModuleNotFoundError`. No Python o segundo é
> **subclasse** do primeiro, então `except ImportError` pega os dois. Aqui o
> `catch` compara o nome e não há hierarquia — usar o nome do Python quebraria,
> em silêncio, todo `catch (ImportError e)` que hoje pega módulo ausente.

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

## 9.6. Import não roda o guard de entrada

Quando um arquivo é **importado**, o bloco `if __name__ == "main":` dele **não
executa** (só roda quando o arquivo é o principal — seção 5.7). Assim, importar
um módulo traz as definições (actions, Entities, constantes) sem disparar o
ponto de entrada:

```ps
// mymod.ps
action saudar(nome) {
    return "ola " + nome
}
if __name__ == "main" {
    post("só quando rodo o mymod direto")
}

// outro.ps
import mymod                 // NÃO imprime a linha do guard
post(mymod.saudar("ana"))
```

---

## 9.7. Resumo

- **`import mod [as m]`** — liga o módulo; acesso por `mod.x`.
- **`from mod import x [as y], z`** — liga nomes soltos.
- **`PUSH mod [as m] [GET x, y]`** — alternativa: `PUSH` = `import`, `GET` =
  `from … import`.
- **`from .mod` / `from ..pkg.mod`** — relativo ao arquivo atual.
- Resolução (sem pontos): **stdlib → lib global → arquivo do projeto**;
  senão `ImportError`.
- **Importar não dispara** o guard `if __name__ == "main"` do módulo.
