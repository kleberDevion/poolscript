# `PoolFile` — arquivo binário carregado

`PoolFile` é o tipo que [`os.loadFile()`](../loadFile/loadFile.md) devolve
quando o arquivo é **binário** (imagem, PDF, docx, etc.). Ele carrega os bytes
e oferece operações de mover, copiar, apagar e pegar os bytes crus.

Você normalmente **não cria** um `PoolFile` na mão — recebe um de `loadFile`.
Mas o nome é exportado como builtin global pra você poder checar o tipo
(`x is PoolFile`).

Os exemplos desta página pressupõem:

```ps contexto
import os
import bytes
# os arquivos dos exemplos: o começo de um PNG e de um PDF de verdade, e as
# pastas de destino (o copy e o move não criam pasta)
os.mkdir("temp")
os.mkdir("backup")
os.mkdir("final")
os.writeFile("logo.png", bytes.fromhex("89504e470d0a1a0a0000000d49484452"))
os.writeFile("temp/foto.png", bytes.fromhex("89504e470d0a1a0a0000000d49484452"))
os.writeFile("nota.pdf", bytes.fromhex("255044462d312e340a25e2e3cfd30a0000"))
os.writeFile("arquivo_qualquer.dat", bytes.fromhex("00ff10ee"))
```

---

## Como obter um

```ps
import os

img = os.loadFile("logo.png")     # PoolFile (extensão binária)
pdf = os.loadFile("nota.pdf")     # PoolFile
```

---

## Métodos e propriedades

| Acesso | O que faz |
|---|---|
| `.name` | nome do arquivo (`"foto.png"`) |
| `.ext` | extensão em minúsculas (`".png"`) |
| `.size` | tamanho em bytes |
| `.path()` | caminho absoluto atual do arquivo |
| `.bytes()` | os bytes crus do conteúdo |
| `.save(path=Null)` | grava o conteúdo em disco — devolve um novo `PoolFile`. Sem `path`, salva **na pasta do script em execução** com o próprio `.name`; com `path`, salva lá (criando as pastas que faltarem) |
| `.move(destino)` | move o arquivo pro destino — devolve um novo `PoolFile` |
| `.copy(destino)` | copia pro destino — devolve um novo `PoolFile` |
| `.delete()` | apaga o arquivo do disco. Arquivo que já não existe não é erro; o que existe e não sai (pasta sem permissão de escrita; no Windows, o arquivo aberto em outro programa) levanta `PermissionError` |

```ps
img = os.loadFile("temp/foto.png")

post(img.name, img.ext, img.size)   # foto.png .png 4821
post(img.path())              # caminho absoluto
dados = img.bytes()          # bytes (ex: pra enviar por rede)

img.copy("backup/foto.png")  # duplica
img.move("final/foto.png")   # move (devolve o novo PoolFile)
```

## Arquivo aberto com `open()` também é `PoolFile`

`type(open("a.txt", "r"))` responde `PoolFile`: o arquivo aberto é o mesmo
tipo, com os métodos de leitura e escrita por cima dos de acima. A página do
[`open()`](../../builtins/open/open.md) detalha cada um.

E vale nos DOIS sentidos: o `PoolFile` que o `os.loadFile()` devolve também
lê e escreve, sobre o conteúdo que já está em memória. O `read` avança um
cursor, o `write` acrescenta no fim, e o `close` só marca fechado — o
conteúdo continua acessível pelo `.bytes()` e pelo `.save()`. É o mesmo tipo,
com os mesmos métodos, venha ele do `open()` ou do `loadFile()`.

| Acesso | O que faz |
|---|---|
| `.read(n=-1)` | lê tudo (ou `n` caracteres) |
| `.readline()` | a próxima linha; `Null` no fim |
| `.readlines()` | todas as linhas, numa `list` |
| `.write(conteudo)` | escreve `str` ou `bytes` |
| `.writelines(linhas)` | escreve cada item da `list` |
| `.close()` | fecha; o `using` faz sozinho |

`save` grava o **conteúdo que está em memória** — útil quando o `PoolFile`
veio de uma lib (download, geração de áudio/imagem...) e ainda não existe no
disco, ou pra materializar uma cópia onde você quiser:

```ps
arq = os.loadFile("nota.pdf")
arq.save()                    # grava na pasta do .pr, como "nota.pdf"
arq.save("saida/nota.pdf")    # ou onde você mandar (cria "saida/" se faltar)
```

---

## Checando o tipo

`PoolFile` é builtin global (não precisa importar) — dá pra usar com `is`:

```ps
conteudo = os.loadFile("arquivo_qualquer.dat")
if (conteudo is PoolFile) {
    post("é binário, tamanho:", len(conteudo.bytes()))
} else {
    post("é texto/JSON:", conteudo)
}
```

---

## Relacionados

- [`os.loadFile()`](../loadFile/loadFile.md) — o que devolve um `PoolFile`
- [`os.copy()`](../copy/copy.md) / [`os.move()`](../move/move.md) — versões por caminho (sem carregar)
