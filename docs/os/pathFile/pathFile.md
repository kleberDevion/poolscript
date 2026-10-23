# `os.pathFile(name)`

Devolve o **caminho absoluto** de um arquivo, buscando-o pelo nome a partir da
pasta do `.pr` em execução e depois do diretório atual. Erro se não encontrar.

```assinatura
os.pathFile(name: str) -> str
```

Os exemplos desta página pressupõem o arquivo `dados.json`:

```ps contexto
import os
os.writeFile("dados.json", "{\"versao\": 1}")
```

---

## Uso

```ps
import os

caminho = os.pathFile("dados.json")
post(caminho)     # "C:\projeto\dados.json"
```

Diferente de `loadFile`, o `pathFile` **não lê** o conteúdo — só te dá o
caminho absoluto, pra passar a outra função que precise dele.

---

## Não encontrou → erro

Se o arquivo não existir em nenhuma das raízes de busca, levanta erro. A busca
desce nas subpastas, mas **não entra em link simbólico pra pasta** (como o
`find -P`): com `a -> .` na pasta, seguir o link era uma busca sem fim. Vale
também pra `os.pathFolder`, `os.loadFile` e `sys.RelativePath`. Se quiser
checar antes:

```ps
if (os.exists("dados.json")) {
    caminho = os.pathFile("dados.json")
}
```

---

## Relacionados

- [`os.pathFolder()`](../pathFolder/pathFolder.md) — o equivalente pra pastas
- [`os.loadFile()`](../loadFile/loadFile.md) — ler o conteúdo direto
- [`os.exists()`](../exists/exists.md) — checar antes
