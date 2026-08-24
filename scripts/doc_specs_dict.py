# -*- coding: utf-8 -*-
"""Especificação dos 12 métodos de `dict` — dados do gerador de doc.

A contagem e os nomes saem da tabela `METODOS_DICT` do `vm/poolscript_vm.c`
(o `tests/test_paridade_membros.py` cruza VM x interpretador). Cada exemplo
daqui roda nos DOIS motores via `test_docs_exemplos.py`.

Detalhe que pega todo mundo: `x in d` testa a **chave**. Pro **valor**, é
`x in d.value()`.
"""

DICTMET = {
    # ── ver o conteúdo ──────────────────────────────────────────────────────
    "keys": dict(sig='d.keys()', resumo="Lista das CHAVES, na ordem de inserção.",
        params=[], ret="list", erros=[],
        ex=[('d = { "a": 1, "b": 2 }\npost(d.keys())', "['a', 'b']")],
        bordas=["a ordem é a de inserção, como no Python 3.7+"]),
    "values": dict(sig='d.values()', resumo="Lista dos VALORES, na ordem de inserção.",
        params=[], ret="list", erros=[],
        ex=[('d = { "a": 1, "b": 2 }\npost(d.values())', "[1, 2]")],
        bordas=["`value()` é o mesmo — o nome curto existe pro `x in d.value()`"]),
    "value": dict(sig='d.value()', resumo="Os VALORES — idêntico a `values()`, com o nome curto.",
        params=[], ret="list", erros=[],
        ex=[('d = { "nome": "ana", "idade": 30 }\npost("ana" in d, "ana" in d.value())',
             "False True"),
            ('d = { "a": [1, 2], "b": (3, 4), "c": 2.5 }\npost([1, 2] in d.value(), (3, 4) in d.value(), 2.5 in d.value())',
             "True True True")],
        bordas=["existe por causa do `in`: `x in d` olha a CHAVE, `x in d.value()` olha o VALOR",
                "vale pra qualquer tipo de valor — str, int, flo, list, tup"]),
    "items": dict(sig='d.items()', resumo="Lista de tuplas `(chave, valor)`.",
        params=[], ret="list de tup", erros=[],
        ex=[('d = { "a": 1, "b": 2 }\npost(d.items())', "[('a', 1), ('b', 2)]")],
        bordas=["pra percorrer os dois de uma vez; a tupla chega inteira na variável do laço"]),
    "len": dict(sig='d.len()', resumo="Quantos pares — forma de método do builtin `len`.",
        params=[], ret="int", erros=[],
        ex=[('d = { "a": 1, "b": 2 }\npost(d.len(), len(d))', "2 2")], bordas=[]),

    # ── ler um valor ────────────────────────────────────────────────────────
    "get": dict(sig='d.get(chave, default=Null)', resumo="Valor da chave — NÃO erra se a chave não existir.",
        params=[("chave", "qualquer", "—", "a chave a buscar"),
                ("default", "qualquer", "Null", "o que devolver quando a chave falta")],
        ret="o valor, ou o `default`", erros=[],
        ex=[('d = { "a": 1 }\npost(d.get("a"), d.get("z"), d.get("z", 0))', "1 null 0")],
        bordas=["`d[\"z\"]` inexistente dá KeyError; `d.get(\"z\")` devolve `null`"]),
    "has": dict(sig='d.has(chave)', resumo="A chave existe? (o mesmo que `chave in d`)",
        params=[("chave", "qualquer", "—", "a chave a testar")], ret="bool", erros=[],
        ex=[('d = { "a": 1 }\npost(d.has("a"), d.has("z"), "a" in d)', "True False True")],
        bordas=["testa CHAVE — pro valor, `x in d.value()`"]),
    "contains": dict(sig='d.contains(chave)', resumo="Apelido de `has` — as duas grafias existem.",
        params=[("chave", "qualquer", "—", "a chave a testar")], ret="bool", erros=[],
        ex=[('d = { "a": 1 }\npost(d.contains("a"), d.contains("z"))', "True False")],
        bordas=[]),

    # ── mudar ───────────────────────────────────────────────────────────────
    "pop": dict(sig='d.pop(chave)', resumo="Remove a chave e DEVOLVE o valor dela (muta).",
        params=[("chave", "qualquer", "—", "a chave a remover")],
        ret="o valor removido", erros=[("KeyError", "a chave não existe")],
        ex=[('d = { "a": 1, "b": 2 }\npost(d.pop("a"))\npost(d)', "1\n{'b': 2}")],
        bordas=[]),
    "update": dict(sig='d.update(outro)', resumo="Mescla os pares de outro dict; chave repetida é sobrescrita (muta).",
        params=[("outro", "dict", "—", "os pares a mesclar")],
        ret="null — muta o dict", erros=[],
        ex=[('d = { "a": 1 }\nd.update({ "b": 2 })\npost(d)', "{'a': 1, 'b': 2}"),
            ('d = { "a": 1 }\nd.update({ "a": 9 })\npost(d)', "{'a': 9}")],
        bordas=[]),
    "clear": dict(sig='d.clear()', resumo="Esvazia o dict (muta).",
        params=[], ret="null — muta o dict", erros=[],
        ex=[('d = { "a": 1 }\nd.clear()\npost(d, len(d))', "{} 0")], bordas=[]),
    "copy": dict(sig='d.copy()', resumo="Cópia RASA: dict novo, valores compartilhados.",
        params=[], ret="dict — a cópia", erros=[],
        ex=[('d = { "a": 1 }\nc = d.copy()\nc["b"] = 2\npost(d, c)', "{'a': 1} {'a': 1, 'b': 2}"),
            ('dentro = [1]\nd = { "l": dentro }\nc = d.copy()\ndentro.append(2)\npost(c)', "{'l': [1, 2]}")],
        bordas=["RASA: mexer num valor aninhado aparece nos dois dicts (2º exemplo)"]),
}
