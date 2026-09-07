# Métodos de list

Chamados direto no valor: `l.metodo()`. A MAIORIA muta a lista no lugar e devolve `null` — não encadeia. Para uma cópia modificada, use os builtins `sorted(l)`/`reversed(l)`.

A lista sai da tabela `METODOS_LIST` de `vm/poolscript_vm.c`, publicada por `pool --metadata`; cada página traz um exemplo executável.

| nome | assinatura | o que faz |
|---|---|---|
| [`append`](append/append.md) | `l.append(item)` | Anexa UM item no fim da lista (muta). |
| [`clear`](clear/clear.md) | `l.clear()` | Esvazia a lista (muta). |
| [`contains`](contains/contains.md) | `l.contains(item)` | O item está na lista? (o mesmo que `item in l`) |
| [`copy`](copy/copy.md) | `l.copy()` | Cópia RASA: lista nova, itens compartilhados. |
| [`count`](count/count.md) | `l.count(item)` | Quantas vezes o item aparece. |
| [`extend`](extend/extend.md) | `l.extend(outra)` | Anexa TODOS os itens de outra sequência no fim (muta). |
| [`has`](has/has.md) | `l.has(item)` | Apelido de `contains` — as duas grafias existem. |
| [`index`](index/index.md) | `l.index(item, inicio=0, fim=len)` | Posição da primeira ocorrência do item. |
| [`insert`](insert/insert.md) | `l.insert(i, item)` | Insere `item` NA posição `i`, empurrando o resto (muta). |
| [`len`](len/len.md) | `l.len()` | Quantidade de itens — forma de método do builtin `len`. |
| [`pop`](pop/pop.md) | `l.pop(i=-1)` | Remove e DEVOLVE o item da posição `i` (o último, por padrão). |
| [`remove`](remove/remove.md) | `l.remove(item)` | Remove a PRIMEIRA ocorrência do item (muta). |
| [`reverse`](reverse/reverse.md) | `l.reverse()` | Inverte a ordem dos itens NO LUGAR (muta). |
| [`sort`](sort/sort.md) | `l.sort()` | Ordena crescente NO LUGAR (muta). |
