# `os.move(src, dst)`

Move um arquivo ou pasta pra outro lugar. Diferente de `copy`, o original
**deixa de existir** no lugar antigo.

```assinatura
os.move(src: str, dst: str) -> Null
```

| Parâmetro | O que é |
|---|---|
| `src` | caminho de origem (arquivo ou pasta) |
| `dst` | caminho de destino |

Os exemplos desta página pressupõem o arquivo `temp/foto.png`, a pasta
`final` (o `move` não cria a pasta de destino) e a pasta `rascunhos`:

```ps contexto
import os
os.mkdir("temp")
os.writeFile("temp/foto.png", "imagem")
os.mkdir("final")
os.mkdir("rascunhos")
```

---

## Uso

```ps
import os

os.move("temp/foto.png", "final/foto.png")   # move o arquivo
os.move("rascunhos", "arquivados")           # move a pasta inteira
```

Também serve como "renomear + mudar de lugar" ao mesmo tempo (é só o destino
ter nome/pasta diferente). Destino que já existe é substituído, nos três
sistemas.

## Quando falha

Entre discos diferentes o `move` copia e depois apaga a origem. Se a cópia
falha, a origem fica e o erro diz o motivo. Se a origem não pode ser apagada
(pasta sem permissão de escrita; no Windows, arquivo aberto em outro
programa), é erro também: o arquivo ficou nos dois lugares, e o `move` diz
isso em vez de fingir que moveu.

---

## `move` vs `rename` vs `copy`

| Função | Efeito |
|---|---|
| [`os.move()`](../move/move.md) | tira do lugar antigo e põe no novo (pode mudar de pasta) |
| [`os.rename()`](../rename/rename.md) | troca o nome no mesmo lugar |
| [`os.copy()`](../copy/copy.md) | duplica — o original continua |

---

## Relacionados

- [`os.copy()`](../copy/copy.md) — copiar sem remover o original
- [`os.rename()`](../rename/rename.md) — só trocar o nome
