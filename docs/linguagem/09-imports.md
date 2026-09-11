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

post(mymod.saudar("ana"))    # acesso por ponto
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

from mymod import saudar as oi      # com apelido
post(oi("ze"))

from mymod import Ponto             # funções, Entities, constantes — tudo que o módulo exporta
p = Ponto(1, 2)
```

Pedir um nome que o módulo não exporta é erro, e o **tipo depende de como você
pediu**:

```ps
from json import naotem      # ImportError: cannot import name 'naotem' from 'json' (unknown location)

import json
post(json.naotem)            # AttributeError: module 'json' has no attribute 'naotem'
```

O `ImportError` cita o **arquivo** do módulo entre parênteses; módulo nativo,
que não tem arquivo, sai como `(unknown location)`. O `AttributeError` é o que
traz a sugestão de nome parecido (`Did you mean: 'parse'?`) — o `ImportError`
não sugere. Nome que existe mas é `private` sai como
`AttributeError: module '…' has no attribute '…' (existe, mas é private)`.

---

## 9.3. `PUSH … GET`

`PUSH` é uma forma alternativa de import:

- **`PUSH <modulo> [as <nome>]`** — igual a `import` (liga o módulo inteiro).
- **`PUSH <modulo> GET <x>, <y>`** — igual a `from <modulo> import <x>, <y>`
  (liga só os nomes listados).

```ps
PUSH mymod                 # == import mymod
post(mymod.valor)

PUSH mymod GET valor       # == from mymod import valor
post(valor)
```

---

## 9.4. Import por caminho — `import '…'`

O módulo pode vir **entre aspas**. É a forma de importar um arquivo `.ps` pelo
caminho dele:

```ps
import '../pacote/modulo.ps'             # liga `modulo`
from './irmao.ps' import w               # nomes soltos
from '../pacote/modulo' import z as zz   # a extensão pode ficar de fora
import '/opt/app/util.ps' as u           # caminho absoluto, com `as`
```

- O caminho é **relativo à pasta do arquivo que contém o `import`** — não ao
  diretório atual nem ao arquivo principal. `./`, `../` e subpastas valem;
  caminho que começa com `/` é absoluto.
- Sem extensão, o motor tenta o nome como escrito e depois `.ps`, `.psl`, `.p`.
- O nome ligado é o **nome do arquivo**, sem pasta e sem extensão
  (`'../pacote/modulo.ps'` liga `modulo`). Com `as`, é o nome do `as`. Se o
  nome do arquivo não serve de nome de variável, o `as` é obrigatório:

  ```
  import 'sub/meu-mod.ps'
  SyntaxError: 'meu-mod' nao serve de nome de variavel: ligue com `as` (import 'sub/meu-mod.ps' as nome)
  ```

- Caminho que não existe é `ImportError: No module named './x.ps'`, com o
  caminho como foi escrito.
- Dentro do módulo importado, `__name__` é o nome do arquivo (`modulo`), e as
  mensagens de atributo citam esse nome: `module 'modulo' has no attribute 'x'`.

A string também aceita o **nome de um módulo ou de uma lib** — sem `/` e sem
extensão, ela vale o mesmo que o `import` sem aspas:

```ps
import 'json'                       # == import json
from 'jinker' import Jinker         # == from jinker import Jinker
import 'minhalib'                   # lib instalada com `psl install … -asLib`
```

O que decide é a forma da string: com `/`, ou terminando em `.ps`/`.psl`/`.p`,
é caminho de arquivo; sem isso é nome de módulo, e segue a ordem da seção 9.5.
Por isso `import 'pacote'` (uma pasta, sem barra) não é caminho: é o nome
`pacote`, e dá `ImportError: No module named 'pacote'` se não houver módulo com
esse nome. String vazia (`import ''`) é `SyntaxError`.

`PUSH` aceita a mesma string: `PUSH '../pacote/modulo.ps' as pm` e
`PUSH './irmao.ps' GET w`.

### 9.4.1. Relativo com pontos

A forma com pontos antes do nome continua valendo:

```ps
from .modulo import x           # mesma pasta
from ..pacote.modulo import y   # um nível acima
```

Cada `.` extra sobe um diretório, a partir da pasta do arquivo atual. Um
relativo não encontrado é `ImportError` — a mesma mensagem do absoluto
(`No module named '..pacote.modulo'`); não há texto próprio pro relativo.

> O tipo é `ImportError`, e só. Não existe um `ModuleNotFoundError` mais
> específico; `ImportError` é o nome único para módulo ausente. Ele fica direto
> sob `Exception` na árvore de exceções, então `catch (Exception e)` também o
> pega.

---

## 9.5. Ordem de resolução

Para um `import nome` **sem pontos**, a busca segue esta ordem — a primeira que
casar vence:

1. **stdlib** — as bibliotecas embutidas (`import json`, `import os`,
   `import sys`, `import request`, `import jinker`, …).
2. **lib instalada globalmente** — o que foi instalado com
   `psl install … -asLib` (em `~/.poolscript/libs/`). Vem **antes** dos
   arquivos locais: o nome de um arquivo seu nunca ofusca uma lib instalada.
3. **arquivo ao lado de quem importa** — a pasta do arquivo que contém o
   `import`. É o que faz `import smtp` dentro de `acesso/controller.ps` achar
   `acesso/smtp.ps`, mesmo com `controller` tendo sido importado por um
   arquivo de outra pasta.
4. **arquivo `.ps` do projeto** — resolvido a partir da raiz do projeto, a
   pasta do arquivo executado (ex.: `from services.smtp import x` →
   `<raiz>/services/smtp.ps`).

Um nome que não casa com nenhum dos quatro é `ImportError`.

A string sem caminho (`import 'json'`, `import 'minhalib'`) segue esta mesma
ordem.

> Imports por caminho (`import '../x.ps'`) e relativos com pontos
> (`from .x import …`) **não** entram nessa ordem — são resolvidos direto contra
> o sistema de arquivos, a partir da pasta do arquivo atual, e nunca caem nas
> libs.

### 9.5.1. Erro DENTRO do módulo importado

Quando o módulo é achado mas **não compila**, o traceback tem dois quadros: o
`import` de quem pediu e, por último, o **arquivo e a linha do defeito**:

```
SyntaxError: random: '//' e divisao inteira, nao comentario — comentario e '#' (ou bloco entre tres aspas)

Traceback (arquivo mais recente por último):
  em d.ps, linha 1
  | import random
  | ^^^
  em random.ps, linha 16
  |             # 127.970.195
  |             ^^^
```

O prefixo da mensagem (`random:`) é o nome do módulo como foi escrito no
`import`; o último quadro é onde consertar.

---

## 9.6. Import não roda o guard de entrada

Quando um arquivo é **importado**, o bloco `if __name__ == "main":` dele **não
executa** (só roda quando o arquivo é o principal — seção 5.7). Assim, importar
um módulo traz as definições (functs, Entities, constantes) sem disparar o
ponto de entrada:

```ps
# mymod.ps
funct saudar(nome) {
    return "ola " + nome
}
if __name__ == "main" {
    post("só quando rodo o mymod direto")
}

# outro.ps
import mymod                 # NÃO imprime a linha do guard
post(mymod.saudar("ana"))
```

---

## 9.7. Resumo

- **`import mod [as m]`** — liga o módulo; acesso por `mod.x`.
- **`from mod import x [as y], z`** — liga nomes soltos.
- **`PUSH mod [as m] [GET x, y]`** — alternativa: `PUSH` = `import`, `GET` =
  `from … import`.
- **`import '../pasta/arquivo.ps' [as m]`** / **`from './arquivo.ps' import x`**
  — por caminho, relativo à pasta do arquivo atual; liga o nome do arquivo.
  Sem `/` nem extensão (`import 'json'`) é nome de módulo ou lib.
- **`from .mod` / `from ..pkg.mod`** — relativo com pontos.
- Resolução (nome, sem caminho): **stdlib → lib global → arquivo ao lado de
  quem importa → arquivo do projeto**; senão `ImportError`.
- **Importar não dispara** o guard `if __name__ == "main"` do módulo.
