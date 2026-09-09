# `type(x)`

Nome do tipo do valor, como string.

## Parâmetros

| nome | tipo | default | nota |
|---|---|---|---|
| `x` | qualquer | — | qualquer valor da linguagem, inclusive `Null` |

## Retorno

**`str`** — o nome do tipo. Nunca `Null`, nunca um objeto de tipo: é sempre
texto, e sempre um dos nomes da tabela abaixo. Compare com `==`:

```ps
post(type(1) == "int")
```

```saida
True
```

## A tabela completa do que ele responde

Esta é a tabela do motor (`nome_do_tipo_valor` em `vm/poolscript_vm.c`),
transcrita inteira. Repare no padrão: **valor da linguagem responde em
minúsculas**, **objeto entregue por uma lib responde com o próprio nome, em
CamelCase**.

### Valores da linguagem

| valor | `type(x)` |
|---|---|
| inteiro (`1`, `-9`) | `"int"` |
| inteiro grande, além de 64 bits (`10 ** 40`) | `"int"` |
| decimal (`1.5`) | `"flo"` |
| booleano (`True`, `False`) | `"bool"` |
| `Null` (e variável declarada sem valor) | `"Null"` |
| texto (`"a"`) | `"str"` |
| lista (`[1, 2]`) | `"list"` |
| tupla (`(1, 2)`) | `"tup"` |
| dicionário (`{"a": 1}`) | `"dict"` |
| bytes (`"oi".encode()`) | `"bytes"` |
| função, lambda, closure, método ligado, nativa | `"funct"` |
| nome de tipo usado como valor (`int`, `long`, `str`) | `"type"` |
| gerador (retorno de uma `funct` com `yield`) | `"generator"` |
| `future` (retorno pendente de chamada assíncrona) | `"future"` |
| a `Entity` em si (`type(Ponto)`) | `"Entity"` |
| **instância** de uma `Entity` | o **nome da classe** (`"Ponto"`) |
| `model` | `"PoolModel"` |
| `enum` | `"enum"` |
| módulo importado (nativo ou `.ps`) | `"module"` |
| `Pattern` compilado (`regex.compile`) | `"Pattern"` |
| arquivo (`open`) | `"PoolFile"` |
| socket (`sockets.socket()`) | `"socket"` |

### Objetos entregues pelas libs

| objeto | lib | `type(x)` |
|---|---|---|
| conexão SQLite | `sqlite3` | `"PoolConnection"` |
| cursor SQLite | `sqlite3` | `"PoolCursor"` |
| conexão ODBC | `psodbc` | `"DbConnection"` |
| cursor ODBC | `psodbc` | `"DbCursor"` |
| conexão Mongo | `psodbc` | `"MongoConnection"` |
| coleção Mongo | `psodbc` | `"MongoCollection"` |
| servidor de e-mail | `mail` | `"MailServer"` |
| mensagem de e-mail | `mail` | `"MailMessage"` |
| leitor de caixa | `mail` | `"MailReader"` |
| resposta HTTP | `request` | `"Response"` |
| conexão WebSocket cliente | `request` | `"WsConnection"` |
| aplicação | `jinker` | `"Jinker"` |
| configuração de CORS | `jinker` | `"CorsConfig"` |
| registrador de rota | `jinker` | `"_RouteRegistrar"` |
| registrador de socket | `jinker` | `"_SocketRegistrar"` |
| registrador de middleware | `jinker` | `"MiddlewareRegistrar"` |
| resposta da rota | `jinker` | `"JinkerResponse"` |
| requisição | `jinker` | `"JinkerRequest"` |
| proxy da requisição | `jinker` | `"RequestProxy"` |
| arquivo recebido no upload | `jinker` | `"PoolFileUpload"` |
| namespace de socket | `jinker` | `"SocketNamespace"` |
| emissor de socket | `jinker` | `"SocketEmitter"` |
| gerenciador de canais | `jinker` | `"ChannelManager"` |
| estado de um canal | `jinker` | `"ChannelStatus"` |
| conexão WebSocket do servidor | `jinker` | `"WsConnection"` |
| construtor de QR | `qrcode` | `"PoolQRCode"` |
| imagem de QR | `qrcode` | `"QRImage"` |
| arquivo de QR | `qrcode` | `"QRPoolFile"` |
| arquivo de mangá | `manpu` | `"ManpuFile"` |
| resultado da leitura | `manpu` | `"ManpuResult"` |

Os três registradores do `jinker` são os únicos nomes com **underscore na
frente** (`_RouteRegistrar`, `_SocketRegistrar`): o sublinhado marca que o
objeto é interno ao decorador e não se guarda numa variável.

Um valor que não caia em nenhum caso responderia `"object"`. Não há tipo do
motor sem entrada própria na tabela, então `"object"` é sentinela — se aparecer,
é defeito do motor, não uso errado.

## Erros

- **TypeError** — com zero ou mais de um argumento:
  `type() takes exactly one argument (2 given)`.

## Exemplos

```ps
post(type(1), type("a"), type([1]), type(Null))
post(type((1, 2)), type({"a": 1}), type(True))
post(type(10 ** 40), type(1.5))
```

```saida
int str list Null
tup dict bool
int flo
```

## O escopo `__universal__`

`type` é o único membro do escopo `__universal__` do motor: além do builtin
`type(x)`, todo valor — sem exceção — expõe o **método** `x.type()`, sem
argumento, que devolve exatamente o mesmo `str`.

```ps
post((1, 2).type(), "a".type(), 1.5.type())
```

```saida
tup str flo
```

As duas formas saem de duas funções diferentes em C (`nativa_type` e
`met_type`), com tabelas **idênticas** — já foram divergentes no passado, e
hoje um caso novo tem que entrar nas duas.

- `x.type()` não aceita argumento: `type() takes no arguments (1 given)`
- em `Null` as duas funcionam: `Null.type()` e `type(Null)` respondem `"Null"`

## Bordas

- `bool` é subtipo de `int` na aritmética, mas `type(True)` é `"bool"`, não
  `"int"`
- inteiro grande não tem nome próprio: `type(10 ** 40)` é `"int"`, igual ao
  inteiro comum — a promoção é invisível
- o decimal chama-se **`flo`**, não `float`
- instância responde o **nome da classe**, e a classe responde `"Entity"`:
  `type(Ponto)` é `"Entity"` e `type(Ponto(1))` é `"Ponto"`
- para perguntar "é deste tipo?" sem comparar texto, use o operador `is`:
  `x is long`

[← índice](../builtins.md)
