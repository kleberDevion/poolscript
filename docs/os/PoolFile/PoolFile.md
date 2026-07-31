# `PoolFile` — arquivo binário carregado

`PoolFile` é o tipo que [`os.loadFile()`](../loadFile/loadFile.md) devolve
quando o arquivo é **binário** (imagem, PDF, docx, etc.). Ele carrega os bytes
e oferece operações de mover, copiar, apagar e pegar os bytes crus.

Você normalmente **não cria** um `PoolFile` na mão — recebe um de `loadFile`.
Mas o nome é exportado como builtin global pra você poder checar o tipo
(`x is PoolFile`).

---

## Como obter um

```
import os

img = os.loadFile("logo.png")     // PoolFile (extensão binária)
pdf = os.loadFile("nota.pdf")     // PoolFile
```

---

## Métodos e propriedades

| Acesso | O que faz |
|---|---|
| `.path()` | caminho absoluto atual do arquivo |
| `.bytes()` | os bytes crus do conteúdo |
| `.move(destino)` | move o arquivo pro destino — devolve um novo `PoolFile` |
| `.copy(destino)` | copia pro destino — devolve um novo `PoolFile` |
| `.delete()` | apaga o arquivo do disco |

```
img = os.loadFile("temp/foto.png")

post(img.path())              // caminho absoluto
dados = img.bytes()          // bytes (ex: pra enviar por rede)

img.copy("backup/foto.png")  // duplica
img.move("final/foto.png")   // move (devolve o novo PoolFile)
```

---

## Checando o tipo

`PoolFile` é builtin global (não precisa importar) — dá pra usar com `is`:

```
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
