# `os.pathFile(nome)`

Devolve o **caminho absoluto** de um arquivo, buscando-o pelo nome a partir da
pasta do `.ps` em execução e depois do diretório atual. Erro se não encontrar.

```
os.pathFile(nome: str) -> str
```

---

## Uso

```
import os

caminho = os.pathFile("dados.json")
post(caminho)     // "C:\projeto\dados.json"
```

Diferente de `loadFile`, o `pathFile` **não lê** o conteúdo — só te dá o
caminho absoluto, pra passar a outra função que precise dele.

---

## Não encontrou → erro

Se o arquivo não existir em nenhuma das raízes de busca, levanta erro. Se
quiser checar antes:

```
if (os.exists("dados.json")) {
    caminho = os.pathFile("dados.json")
}
```

---

## Relacionados

- [`os.pathFolder()`](../pathFolder/pathFolder.md) — o equivalente pra pastas
- [`os.loadFile()`](../loadFile/loadFile.md) — ler o conteúdo direto
- [`os.exists()`](../exists/exists.md) — checar antes
