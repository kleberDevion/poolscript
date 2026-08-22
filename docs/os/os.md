# os — Sistema de arquivos, ambiente e terminal

A lib `os` reúne o que você precisa pra mexer com **arquivos e pastas**, ler
**variáveis de ambiente**, e rodar **comandos do terminal**. É o canivete do
sistema operacional dentro da PoolScript.

```
import os
// ou: import os as sistema
```

Todo caminho relativo (`"dados.json"`, `"pasta/x.txt"`) é resolvido a partir da
pasta do `.ps` em execução e depois do diretório atual — você quase nunca
precisa de caminho absoluto.

---

## Referência — todos os membros

### Localizar e carregar arquivos

| Membro | O que faz | Página |
|---|---|---|
| `pathFile(nome)` | caminho absoluto de um arquivo (busca pelo nome) | [pathFile/pathFile.md](pathFile/pathFile.md) |
| `pathFolder(nome)` | caminho absoluto de uma pasta | [pathFolder/pathFolder.md](pathFolder/pathFolder.md) |
| `loadFile(nome, encoding)` | lê um arquivo — texto vira str/dict, binário vira `PoolFile` | [loadFile/loadFile.md](loadFile/loadFile.md) |
| `readFile(caminho, encoding)` | lê um arquivo como **texto** (str), UTF-8 por padrão | [readFile/readFile.md](readFile/readFile.md) |
| `writeFile(caminho, conteudo, encoding)` | **escreve** str/bytes num arquivo, criando a pasta pai; devolve o caminho | [writeFile/writeFile.md](writeFile/writeFile.md) |
| `PoolFile` | tipo de arquivo binário carregado (move/copy/delete) | [PoolFile/PoolFile.md](PoolFile/PoolFile.md) |

### Verificar / navegar

| Membro | O que faz | Página |
|---|---|---|
| `exists(caminho)` | arquivo **ou** pasta existe? | [exists/exists.md](exists/exists.md) |
| `isfile(caminho)` | é um arquivo? | [isfile/isfile.md](isfile/isfile.md) |
| `isdir(caminho)` | é uma pasta? | [isdir/isdir.md](isdir/isdir.md) |
| `size(caminho)` | tamanho do arquivo em bytes | [size/size.md](size/size.md) |
| `ls(caminho)` | lista arquivos/pastas com detalhes | [ls/ls.md](ls/ls.md) |
| `cwd()` | diretório atual | [cwd/cwd.md](cwd/cwd.md) |
| `chdir(caminho)` | muda o diretório atual | [chdir/chdir.md](chdir/chdir.md) |

### Criar / mexer

| Membro | O que faz | Página |
|---|---|---|
| `mkdir(caminho, exist_ok)` | cria uma pasta | [mkdir/mkdir.md](mkdir/mkdir.md) |
| `rmdir(caminho, force)` | remove uma pasta | [rmdir/rmdir.md](rmdir/rmdir.md) |
| `rename(orig, dest)` | renomeia arquivo/pasta | [rename/rename.md](rename/rename.md) |
| `copy(orig, dest)` | copia arquivo | [copy/copy.md](copy/copy.md) |
| `move(orig, dest)` | move arquivo/pasta | [move/move.md](move/move.md) |

### Ambiente e terminal

| Membro | O que faz | Página |
|---|---|---|
| `getenv(chave, default)` | lê variável de ambiente | [getenv/getenv.md](getenv/getenv.md) |
| `environ(chave)` | uma variável, ou todas se sem argumento | [environ/environ.md](environ/environ.md) |
| `cmd(comando, capture)` | roda um comando COM shell (interpreta `;` `\|` `$`) | [cmd/cmd.md](cmd/cmd.md) |
| `run(args, capture)` | roda SEM shell (lista de args — à prova de injeção) | [run/run.md](run/run.md) |
| `code(caminho)` | abre o editor de código no caminho | [code/code.md](code/code.md) |
| `ipmach()` | IP da máquina | [ipmach/ipmach.md](ipmach/ipmach.md) |
| `warn(texto, cor)` | mensagem colorida no terminal | [warn/warn.md](warn/warn.md) |

---

## Exemplo rápido

```
import os
from dotenv import load

load()                                   // carrega o .env
str banco = os.getenv("DB_PATH")         // variável de ambiente

if (os.exists("uploads")) {              // pasta existe?
    for each item in os.ls("uploads") {
        post(item)
    }
} else {
    os.mkdir("uploads")                  // cria se não existe
}

conteudo = os.loadFile("config.json")    // lê e parseia JSON automaticamente
post(conteudo["versao"])
```
