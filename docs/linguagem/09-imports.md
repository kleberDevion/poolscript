# Referência da Linguagem — 9. Imports

Um `.pr` pode usar código de outro arquivo `.pr` ou de uma biblioteca — da
stdlib (embutida) ou instalada. Esta seção cobre as formas de import
(`import`, `from … import`, `PUSH … GET`), o `as`, o `*`, os imports relativos
e a ordem em que um nome é resolvido.

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

**O que um módulo exporta** é o que o arquivo dele liga **no nível do
arquivo**:

- `funct`, `Entity`, `enum` e `model` declarados no topo;
- variáveis atribuídas ou declaradas no topo (`x = 1`, `str nome = "a"`);
- os nomes que os imports do próprio módulo ligam — inclusive os que vieram
  por `*` (seção 9.2.1);
- o nome que uma funct grava com `global x`.

Fica **de fora**:

- o que é `private` — **qualquer** declaração do topo: `private funct`,
  `private class`, `private model`, `private enum`, e variável (`private x =
  1`, `private int n = 1`, `private a, b = 1, 2`). O `private` é do **nome**,
  no módulo inteiro: reatribuir `x` depois não o expõe;
- o que só existe **dentro de um bloco** (`if`, `for each`, `try`, o guard
  `if __name__ == "main"`) — no fim do bloco o nome some, como em qualquer
  lugar;
- o nome que o módulo só **usa** sem definir. Um módulo que chama `len` não
  exporta `len`: `from m import len` é `ImportError` e `m.len` é
  `AttributeError`, a não ser que o arquivo defina a própria `funct len`.

Nome começado por `_` é exportado como qualquer outro — o que esconde é o
`private`. É a mesma regra pra `from m import x`, pra `m.x` e pro `*`.

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
não sugere, e a sugestão só aponta nome que o módulo exporta. Nome que existe
mas é `private` sai como
`AttributeError: module '…' has no attribute '…' (existe, mas é private)` —
e sai **antes de rodar**: o `--check` (e o `jinga` que barra na tipagem)
acusa `m.x` e `from m import x` com a mesma frase, mais `declarada em m.pr,
linha N` e o quadro da declaração. Rodando, o traceback termina com a nota
`'x' e private: declarada em m.pr, linha N` e a linha do fonte, com o cursor
no nome:

```ps
# lib.pr
private x = 5
private funct f() {
    return x
}
pub = 9

# main.pr
import lib
post(lib.pub)          # 9
post(lib.x)            # AttributeError: module 'lib' has no attribute 'x' (existe, mas é private: declarada em lib.pr, linha 1)
from lib import f      # a mesma frase
from lib import *      # traz `pub`; `x` e `f` ficam de fora (NameError no uso)
```

`private` só tem efeito no **topo do arquivo** (e no corpo da Entity, seção
7). Dentro de uma funct, de um `if`/`for`/`try` ou do guard `if __name__ ==
"main"` ele não esconderia nada — e por isso é erro, não silêncio:
`'private' aqui nao tem efeito: so vale no topo do arquivo (o que nao sai
pelo import) ou no corpo da Entity`. Antes de uma coisa que não é declaração
(`private if`, `private x += 1`) também é erro de sintaxe.

### 9.2.1. `*` — todos os nomes que o módulo exporta

Três grafias, **a mesma regra**:

```ps
from json import *
post(stringify([1]), parse("[2]"))    # [1] [2]
```

```
import json *          # igual a `from json import *`
PUSH json GET *        # igual a `from json import *`
```

O `*` liga, sem prefixo, **cada nome que o módulo exporta** (a lista acima), do
mesmo jeito que `from m import a, b, c` escrito à mão ligaria. **Não** liga o
nome do módulo: depois de `import json *`, `json` sozinho é `NameError`. De
módulo nativo (`json`, `os`, `regex`…), entram todos os membros.

Vale com caminho entre aspas e com pontos: `from './util.pr' import *`,
`import '../pacote/modulo' *`, `from ..pacote.modulo import *`.

**Como os nomes se comportam** — exatamente como os de um import explícito:

- o `*` **sobrescreve** o que já existia com o mesmo nome, e o que vier
  **depois** sobrescreve o que o `*` trouxe;
- um membro que tem o nome de um builtin passa a valer a partir da linha do
  import (`bytes` exporta `hex`: antes do `from bytes import *`, `hex(255)` é o
  embutido; depois, é o do módulo);
- variável tipada do arquivo (`str x = "a"`) confere o valor que chega;
- escrita dentro de bloco atravessa até o nome, e `for each` sobre ele devolve
  o valor anterior no fim do laço;
- **reexportação**: os nomes que um módulo trouxe por `*` saem dele de novo,
  por `*` ou por `m.nome`.

**Resolvido na compilação.** O `*` vira a lista de nomes antes de o programa
rodar — é isso que faz todos os pontos acima valerem igual ao import
explícito. O que fica pra execução é o **valor** de
cada nome. Em import circular (dois arquivos que se importam com `*`), o nome
que ainda não tem valor quando o import roda é pulado, sem erro; o que já tem
valor é ligado.

Duas consequências dessa escolha:

- **Só no topo do arquivo.** Dentro de funct, método, lambda ou de qualquer
  bloco (inclusive `try` e o guard), o `*` é recusado, e a mensagem diz o
  conserto — dentro de funct os slots são decididos na compilação, e o fim de
  um bloco apaga os nomes que nasceram nele pelo nome:

  ```
  funct g() {
      from json import *
  }
  SyntaxError: `*` do import so vale no topo do arquivo; dentro de funct ou bloco nomeie o que usa: from json import a, b
  ```

- **O módulo tem que existir quando o arquivo é compilado**, e a lista sai
  inteira ou não sai: um `*` que aponta pra módulo ausente deixa o arquivo sem
  a lista completa, e a recusa sobe pro import que você escreveu, em vez de
  sumir nome em silêncio. Uma cadeia de `*` muito funda (centenas de arquivos
  reexportando um ao outro) também é recusada. Um arquivo que o próprio
  programa gera antes do `import *` não tem nomes a trazer:

  ```
  ImportError: `*` de 'gerado': os nomes do `*` sao resolvidos antes de o programa rodar, e nessa hora nao deu pra ler o modulo (ausente, sem compilar, ou cadeia de `*` funda demais) — importe pelo nome (from gerado import a, b)
  ```

Módulo que não existe é o `ImportError: No module named '…'` de sempre, na
linha do import. As formas misturadas são recusadas:

```
import json as j *
SyntaxError: `import m as x *` mistura as duas formas: `import m as x` liga o modulo, `import m *` liga os nomes dele — escolha uma

from json import parse, *
SyntaxError: `*` traz todos os nomes e nao se mistura com uma lista: escreva so `*` ou so os nomes (a, b)

from json import * as t
SyntaxError: `*` nao aceita `as`: ele liga cada nome com o proprio nome
```

---

## 9.3. `PUSH … GET`

`PUSH` é uma forma alternativa de import:

- **`PUSH <modulo> [as <nome>]`** — igual a `import` (liga o módulo inteiro).
- **`PUSH <modulo> GET <x>, <y>`** — igual a `from <modulo> import <x>, <y>`
  (liga só os nomes listados).
- **`PUSH <modulo> GET *`** — igual a `from <modulo> import *` (seção 9.2.1).

```ps
PUSH mymod                 # == import mymod
post(mymod.valor)

PUSH mymod GET valor       # == from mymod import valor
post(valor)
```

Cada forma tem a **sua** palavra antes dos nomes: `from … import` e
`PUSH … GET`. Trocar uma pela outra é `SyntaxError`, e a frase diz de qual forma
é a palavra que foi escrita — com o `*` quando foi `*`:

```
from mymod GET *
SyntaxError: `GET` e do PUSH; no from os nomes vem depois de `import`: from m import * — ou PUSH m GET *

PUSH mymod import valor
SyntaxError: `import` e do from; no PUSH os nomes vem depois de `GET`: PUSH m GET nome — ou from m import nome

import mymod GET valor
SyntaxError: `GET` e do PUSH; `import m` liga o modulo inteiro. Pra trazer nomes: PUSH m GET nome — ou from m import nome
```

---

## 9.4. Import por caminho — `import '…'`

O módulo pode vir **entre aspas**. É a forma de importar um arquivo `.pr` pelo
caminho dele:

```ps
import '../pacote/modulo.pr'             # liga `modulo`
from './irmao.pr' import w               # nomes soltos
from '../pacote/modulo' import z as zz   # a extensão pode ficar de fora
import '/opt/app/util.pr' as u           # caminho absoluto, com `as`
```

- O caminho é **relativo à pasta do arquivo que contém o `import`** — não ao
  diretório atual nem ao arquivo principal. `./`, `../` e subpastas valem;
  caminho que começa com `/` é absoluto.
- Sem extensão, o motor tenta o nome como escrito e depois `.pr`.
- O nome ligado é o **nome do arquivo**, sem pasta e sem extensão
  (`'../pacote/modulo.pr'` liga `modulo`). Com `as`, é o nome do `as`. Se o
  nome do arquivo não serve de nome de variável, o `as` é obrigatório:

  ```
  import 'sub/meu-mod.pr'
  SyntaxError: 'meu-mod' nao serve de nome de variavel: ligue com `as` (import 'sub/meu-mod.pr' as nome)
  ```

- Caminho que não existe é `ImportError: No module named './x.pr'`, com o
  caminho como foi escrito.
- Dentro do módulo importado, `__name__` é o nome do arquivo (`modulo`), e as
  mensagens de atributo citam esse nome: `module 'modulo' has no attribute 'x'`.
  No arquivo executado, `__name__` vale `"main"` (seção 5.7).

A string também aceita o **nome de um módulo ou de uma lib** — sem `/` e sem
extensão, ela vale o mesmo que o `import` sem aspas:

```ps
import 'json'                       # == import json
from 'jinker' import Jinker         # == from jinker import Jinker
import 'minhalib'                   # lib instalada com `psl install … -asLib`
```

O que decide é a forma da string: com `/`, ou terminando na extensão da
linguagem, é caminho de arquivo; sem isso é nome de módulo, e segue a ordem da
seção 9.5. As três extensões de antes (`.ps`, `.psl`, `.p`) também contam como
caminho — não pra serem carregadas, e sim pra o erro dizer que a extensão
mudou, em vez de mandar procurar um módulo que nunca existiu.
Por isso `import 'pacote'` (uma pasta, sem barra) não é caminho: é o nome
`pacote`, e dá `ImportError: No module named 'pacote'` se não houver módulo com
esse nome. String vazia (`import ''`) é `SyntaxError`.

`PUSH` aceita a mesma string: `PUSH '../pacote/modulo.pr' as pm` e
`PUSH './irmao.pr' GET w`.

### 9.4.1. Relativo com pontos

A forma com pontos antes do nome continua valendo:

```ps
from .modulo import x           # mesma pasta
from ..pacote.modulo import y   # um nível acima
```

Cada `.` extra sobe um diretório, a partir da pasta do arquivo atual. Um
relativo não encontrado é `ImportError` — a mesma mensagem do absoluto
(`No module named '..pacote.modulo'`); não há texto próprio pro relativo.

O relativo é **só com `from`**. `import .modulo` e `PUSH .modulo` não existem,
e o erro diz as duas formas que funcionam, com os nomes escritos — o caminho
entre aspas liga o módulo (um ponto é `./`, cada ponto a mais é um `../`), e o
`from` traz os nomes:

```
import ..pacote.modulo
SyntaxError: import com ponto na frente nao existe (o relativo e so com from); pra ligar o modulo: import '../pacote/modulo.pr' — pra trazer nomes: from ..pacote.modulo import nome
```

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
   `jpkg install … -asLib` (em `~/.jinga/libs/`). Vem **antes** dos
   arquivos locais: o nome de um arquivo seu nunca ofusca uma lib instalada.
3. **arquivo ao lado de quem importa** — a pasta do arquivo que contém o
   `import`. É o que faz `import smtp` dentro de `acesso/controller.pr` achar
   `acesso/smtp.pr`, mesmo com `controller` tendo sido importado por um
   arquivo de outra pasta.
4. **arquivo `.pr` do projeto** — resolvido a partir da raiz do projeto, a
   pasta do arquivo executado (ex.: `from services.smtp import x` →
   `<raiz>/services/smtp.pr`).

Um nome que não casa com nenhum dos quatro é `ImportError`.

A string sem caminho (`import 'json'`, `import 'minhalib'`) segue esta mesma
ordem.

> Imports por caminho (`import '../x.pr'`) e relativos com pontos
> (`from .x import …`) **não** entram nessa ordem — são resolvidos direto contra
> o sistema de arquivos, a partir da pasta do arquivo atual, e nunca caem nas
> libs.

### Nome qualificado da lib instalada

A lib instalada também atende pelo nome completo, com o prefixo
`jinga.libs.`:

```
import jinga.libs.minhalib          # liga `minhalib`
import jinga.libs.minhalib as m
from jinga.libs.minhalib import nome
PUSH jinga.libs.minhalib GET nome
```

- `jinga.libs` é o **nome** da pasta de libs, não o caminho escrito: vale
  onde as libs estiverem, inclusive com `JINGA_HOME` (aí a pasta é
  `$JINGA_HOME/libs`).
- Liga o **último** nome, como qualquer `import a.b.c`: depois de
  `import jinga.libs.minhalib`, usa-se `minhalib.x`.
- É o **mesmo arquivo** do `import minhalib`, então é o mesmo módulo: com as
  duas formas no programa, o corpo da lib roda uma vez só.
- Se não houver lib instalada com esse nome, segue a ordem acima (itens 3 e 4),
  e uma pasta `jinga/libs/` do próprio projeto continua sendo achada. Sem
  nada, é `ImportError: No module named 'jinga.libs.x'`.
- O prefixo de antes do rename, `poolscript.libs.`, continua aceito e cai na
  mesma lib.

Não é necessário — `import minhalib` já acha a lib antes de qualquer arquivo
local. É a forma explícita, para quem quer deixar escrito de onde o nome vem.

O ponto na frente **não** vale aqui: `import .jinga.libs.minhalib` é o
`import` relativo, que não existe (ver 9.4.1), e o erro diz a forma certa:

```
import .jinga.libs.minhalib
SyntaxError: import com ponto na frente nao existe (o relativo e so com from); pra lib instalada, sem o ponto: import jinga.libs.minhalib
```

### 9.5.1. Erro DENTRO do módulo importado

Quando o módulo é achado mas **não compila**, o traceback tem dois quadros: o
`import` de quem pediu e, por último, o **arquivo e a linha do defeito**:

```
SyntaxError: random: '//' e divisao inteira, nao comentario — comentario e '#' (ou bloco entre tres aspas)

Traceback (arquivo mais recente por último):
  em d.pr, linha 1
  | import random
  | ^^^
  em random.pr, linha 16
  |             # 127.970.195
  |             ^^^
```

O prefixo da mensagem (`random:`) é o nome do módulo como foi escrito no
`import`; o último quadro é onde consertar.

---

### 9.5.2. O que se confere antes de rodar

O `--check` (e o `jinga`, antes de executar) lê o módulo `.pr` importado e
confere contra a assinatura **dele**, como faz com as functs do próprio
arquivo:

- chamada a funct do módulo — aridade, nomes e tipos dos parâmetros
  (`util.soma(1, 2, 3)`, `soma("a")`), inclusive por apelido (`import util as u`);
- classe importada — argumentos do construtor, métodos e campos de uma
  instância (`Conta k = Conta(1); k.extrato(9)`);
- membro que o módulo não exporta (`util.naoexiste`) e nome que ele não tem
  (`from util import naoexiste`) — as mesmas frases do erro de runtime.

```ps
import util                    # util.pr: funct soma(int a, int b=1)
post(util.soma(1, 2, 3))       # TypeError: soma() takes from 1 to 2 positional arguments but 3 were given
```

Fica de fora, de propósito, o que não dá pra saber sem rodar: funct do
módulo decorada por um decorador geral (ele pode tê-la trocado), variável do
módulo (o tipo é o do valor, só rodando) e módulo em ciclo de import. Módulo
que não compila (sintaxe ou tipo) é acusado na linha do `import`, com a
frase que o import daria rodando.

## 9.6. Import não roda o guard de entrada

Quando um arquivo é **importado**, o bloco `if __name__ == "main":` dele **não
executa** (só roda quando o arquivo é o principal — seção 5.7). Vale até pra um
módulo chamado `main.pr`, cujo `__name__` também é `"main"`: o guard é pulado em
todo import. Assim, importar um módulo traz as definições (functs, Entities,
constantes) sem disparar o ponto de entrada:

```ps
# mymod.pr
funct saudar(nome) {
    return "ola " + nome
}
if __name__ == "main" {
    post("só quando rodo o mymod direto")
}

# outro.pr
import mymod                 # NÃO imprime a linha do guard
post(mymod.saudar("ana"))
```

---

## 9.7. Resumo

- **`import mod [as m]`** — liga o módulo; acesso por `mod.x`.
- **`from mod import x [as y], z`** — liga nomes soltos.
- **O que sai de um módulo**: o que o arquivo liga no nível do arquivo (funct,
  Entity, enum, model, variável, os imports dele, `global x`), sem `private`
  (que vale em qualquer uma dessas declarações, e é acusado antes de rodar),
  sem o que só existe em bloco e sem o builtin que ele só usa — a mesma regra
  pra `from mod import x`, `mod.x` e `*`.
- **`from mod import *`** = **`import mod *`** = **`PUSH mod GET *`** — liga
  todos os nomes que o módulo exporta, e não o nome do módulo; resolvido na
  compilação, só no topo do arquivo; em ciclo, nome ainda sem valor é pulado.
- **`PUSH mod [as m] [GET x, y]`** — alternativa: `PUSH` = `import`, `GET` =
  `from … import`.
- **`import '../pasta/arquivo.pr' [as m]`** / **`from './arquivo.pr' import x`**
  — por caminho, relativo à pasta do arquivo atual; liga o nome do arquivo.
  Sem `/` nem extensão (`import 'json'`) é nome de módulo ou lib.
- **`from .mod` / `from ..pkg.mod`** — relativo com pontos.
- Resolução (nome, sem caminho): **stdlib → lib global → arquivo ao lado de
  quem importa → arquivo do projeto**; senão `ImportError`.
- **Importar não dispara** o guard `if __name__ == "main"` do módulo; no
  arquivo executado, `__name__` vale `"main"`.
