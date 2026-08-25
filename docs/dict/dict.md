# Métodos de dict

Chamados direto no valor: `d.metodo()`. Lembre: `x in d` testa a **chave**; para procurar um **valor**, `x in d.value()`.

**12 no total** — a contagem sai da fonte (`vm/poolscript_vm.c`), não da memória de ninguém. Cada exemplo das páginas roda de verdade na suíte em C (`make -f rebuild/Makefile check`): doc errada quebra o teste.

| nome | assinatura | o que faz |
|---|---|---|
| [`clear`](clear/clear.md) | `d.clear()` | Esvazia o dict (muta). |
| [`contains`](contains/contains.md) | `d.contains(chave)` | Apelido de `has` — as duas grafias existem. |
| [`copy`](copy/copy.md) | `d.copy()` | Cópia RASA: dict novo, valores compartilhados. |
| [`get`](get/get.md) | `d.get(chave, default=Null)` | Valor da chave — NÃO erra se a chave não existir. |
| [`has`](has/has.md) | `d.has(chave)` | A chave existe? (o mesmo que `chave in d`) |
| [`items`](items/items.md) | `d.items()` | Lista de tuplas `(chave, valor)`. |
| [`keys`](keys/keys.md) | `d.keys()` | Lista das CHAVES, na ordem de inserção. |
| [`len`](len/len.md) | `d.len()` | Quantos pares — forma de método do builtin `len`. |
| [`pop`](pop/pop.md) | `d.pop(chave)` | Remove a chave e DEVOLVE o valor dela (muta). |
| [`update`](update/update.md) | `d.update(outro)` | Mescla os pares de outro dict; chave repetida é sobrescrita (muta). |
| [`value`](value/value.md) | `d.value()` | Os VALORES — idêntico a `values()`, com o nome curto. |
| [`values`](values/values.md) | `d.values()` | Lista dos VALORES, na ordem de inserção. |
