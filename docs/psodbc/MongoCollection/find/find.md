# `MongoCollection.find(query=Null, skip=0, limit=0, sort=Null)`

Busca os documentos que batem com o filtro. Devolve uma lista — ou **`Null`**
quando nenhum bate.

```
col.find(query: dict = Null, skip: int = 0, limit: int = 0, sort: dict = Null) -> list | Null
```

| Parâmetro | Padrão | O que é |
|---|---|---|
| `query` | `Null` | o filtro do Mongo (`{"ativo": true}`); `Null` = todos |
| `skip` | `0` | quantos documentos pular antes de começar a devolver |
| `limit` | `0` | no máximo quantos devolver; `0` = sem limite |
| `sort` | `Null` | a ordem: `{"campo": 1}` crescente, `{"campo": -1}` decrescente; várias chaves valem na ordem escrita |

**Quem pula, corta e ordena é o SERVIDOR.** `skip`, `limit` e `sort` vão na
própria consulta, então só a página pedida atravessa a rede. É isso que torna
paginar possível sem trazer a coleção inteira pra memória.

O `_id` **não** vem nos documentos devolvidos.

---

## Uso

```
col.find({"ativo": true})      # todos com ativo = true
col.find()                     # todos (sem filtro)

for each doc in col.find({"preco": 15}) {
    post(doc["nome"])
}
```

## Paginar

```
# página 3, de 20 em 20, os mais novos primeiro
pagina = col.find({"ativo": true}, skip=40, limit=20, sort={"criado": -1})
```

Sem `sort`, a ordem é a que o servidor quiser, e duas páginas seguidas podem
repetir ou pular documento. Pra paginar, ordene por um campo que não se repete.

A ordem posicional é `query, skip, limit, sort`, e dá pra passar só os nomeados:

```
col.find(limit=5, sort={"criado": -1})   # os 5 mais novos, sem filtro
```

---

## Erros

| Chamada | Erro |
|---|---|
| `col.find({}, skip=-1)` | `ValueError: find(): 'skip' nao pode ser negativo (-1)` |
| `col.find({}, limit="2")` | `TypeError: find(): 'limit' tem que ser int, nao str` |
| `col.find({}, sort="criado")` | `TypeError: find(): 'sort' tem que ser dict ({"campo": 1 ou -1}), nao str` |

---

## `find` vs `find_one`

- **`find`** — **lista** dos que batem (ou `Null`).
- **[`find_one`](../find_one/find_one.md)** — só o **primeiro** (um dict, ou `Null`).

---

## Relacionados

- [`.find_one()`](../find_one/find_one.md) — um documento só
- [`.count()`](../count/count.md) — só contar, sem trazer os dados
