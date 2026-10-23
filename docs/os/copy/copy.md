# `os.copy(src, dst)`

Copia um arquivo de um caminho pra outro. O original permanece.

```assinatura
os.copy(src: str, dst: str) -> Null
```

| Parâmetro | O que é |
|---|---|
| `src` | caminho do arquivo de origem |
| `dst` | caminho de destino |

Os exemplos desta página pressupõem os arquivos `dados.db` e `config.json` e
a pasta `backup` (o `copy` não cria a pasta de destino):

```ps contexto
import os
os.writeFile("dados.db", "conteúdo do banco")
os.writeFile("config.json", "{\"porta\": 8000}")
os.mkdir("backup")
```

---

## Uso

```ps
import os

os.copy("dados.db", "backup/dados.db")     # duplica o arquivo
os.copy("config.json", "config.bak.json")  # cópia de segurança
```

Depois de copiar, os **dois** arquivos existem.

Se a cópia falha no meio (disco cheio, erro de leitura), levanta o erro com o
motivo, e o destino pela metade é apagado — só se ele é o arquivo comum que
a cópia escreveu. Destino que é link ou dispositivo fica como estava.

Origem e destino sendo o **mesmo arquivo** (`os.copy("a.txt", "./a.txt")`) é
`OSError: origem e destino sao o mesmo arquivo: './a.txt'`, e o arquivo fica
como estava — antes o destino era truncado antes de ler a origem, e o
conteúdo sumia sem erro. Vale igual em `PoolFile.copy` e no `move`.

---

## `os.copy` vs `PoolFile.copy`

- **`os.copy(src, dst)`** — trabalha por **caminho**, sem carregar o conteúdo
  na memória. Bom pra arquivos grandes.
- **[`PoolFile.copy()`](../PoolFile/PoolFile.md)** — quando você já carregou o
  arquivo com `loadFile`.

---

## Relacionados

- [`os.move()`](../move/move.md) — mover (não deixa o original)
- [`os.rename()`](../rename/rename.md) — renomear
