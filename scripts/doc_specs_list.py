# -*- coding: utf-8 -*-
"""Especificação dos 14 métodos de `list` — dados do gerador de doc.

A contagem e os nomes saem da tabela `METODOS_LIST` do `vm/poolscript_vm.c`
(o `tests/test_paridade_membros.py` cruza VM x interpretador). Cada exemplo
daqui roda nos DOIS motores via `test_docs_exemplos.py` — expectativa errada
quebra a suíte.

A maioria dos métodos **muta a lista no lugar** e devolve `null`: não encadeia
(`l.sort().reverse()` não existe).
"""

LISTMET = {
    # ── crescer ─────────────────────────────────────────────────────────────
    "append": dict(sig='l.append(item)', resumo="Anexa UM item no fim da lista (muta).",
        params=[("item", "qualquer", "—", "o valor a anexar; lista/dict entram como um único item")],
        ret="null — muta a lista", erros=[],
        ex=[('l = [1, 2]\nl.append(3)\npost(l)', "[1, 2, 3]"),
            ('l = [1]\nl.append([2, 3])\npost(l, len(l))', "[1, [2, 3]] 2")],
        bordas=["anexa UM item — pra juntar outra lista item a item, use `extend`"]),
    "extend": dict(sig='l.extend(outra)', resumo="Anexa TODOS os itens de outra sequência no fim (muta).",
        params=[("outra", "list | tup", "—", "a sequência cujos itens entram")],
        ret="null — muta a lista", erros=[],
        ex=[('l = [1]\nl.extend([2, 3])\npost(l)', "[1, 2, 3]"),
            ('l = [1]\nl.extend((2, 3))\npost(l)', "[1, 2, 3]")],
        bordas=[]),
    "insert": dict(sig='l.insert(i, item)', resumo="Insere `item` NA posição `i`, empurrando o resto (muta).",
        params=[("i", "int", "—", "posição onde o item passa a ficar"),
                ("item", "qualquer", "—", "o valor a inserir")],
        ret="null — muta a lista", erros=[],
        ex=[('l = ["a", "c"]\nl.insert(1, "b")\npost(l)', "['a', 'b', 'c']"),
            ('l = [1, 2]\nl.insert(0, 0)\npost(l)', "[0, 1, 2]")],
        bordas=["índice além do fim não erra: o item vai pro fim"]),

    # ── encolher ────────────────────────────────────────────────────────────
    "pop": dict(sig='l.pop(i=-1)', resumo="Remove e DEVOLVE o item da posição `i` (o último, por padrão).",
        params=[("i", "int", "-1", "posição a remover; negativo conta do fim")],
        ret="o item removido", erros=[],
        ex=[('l = [1, 2, 3]\npost(l.pop(), l)', "3 [1, 2]"),
            ('l = [1, 2, 3]\npost(l.pop(0), l)', "1 [2, 3]")],
        bordas=["é o único mutador que devolve algo útil"]),
    "remove": dict(sig='l.remove(item)', resumo="Remove a PRIMEIRA ocorrência do item (muta).",
        params=[("item", "qualquer", "—", "o valor a remover")],
        ret="null — muta a lista", erros=[],
        ex=[('l = [1, 2, 1]\nl.remove(1)\npost(l)', "[2, 1]")],
        bordas=["remove por VALOR, não por posição — pra posição use `pop(i)`"]),
    "clear": dict(sig='l.clear()', resumo="Esvazia a lista (muta).",
        params=[], ret="null — muta a lista", erros=[],
        ex=[('l = [1, 2]\nl.clear()\npost(l, len(l))', "[] 0")],
        bordas=[]),

    # ── reordenar ───────────────────────────────────────────────────────────
    "reverse": dict(sig='l.reverse()', resumo="Inverte a ordem dos itens NO LUGAR (muta).",
        params=[], ret="null — muta a lista", erros=[],
        ex=[('l = [1, 2, 3]\nl.reverse()\npost(l)', "[3, 2, 1]")],
        bordas=["não devolve a lista: `post(l.reverse())` imprime `null`",
                "pra uma CÓPIA invertida, use o builtin `reversed(l)`"]),
    "sort": dict(sig='l.sort()', resumo="Ordena crescente NO LUGAR (muta).",
        params=[], ret="null — muta a lista", erros=[],
        ex=[('l = [3, 1, 2]\nl.sort()\npost(l)', "[1, 2, 3]"),
            ('l = ["banana", "abacaxi"]\nl.sort()\npost(l)', "['abacaxi', 'banana']")],
        bordas=["pra uma CÓPIA ordenada, use o builtin `sorted(l)`"]),

    # ── consultar (não mutam) ───────────────────────────────────────────────
    "index": dict(sig='l.index(item)', resumo="Posição da primeira ocorrência do item.",
        params=[("item", "qualquer", "—", "o valor a procurar")],
        ret="int — a posição", erros=[("SomeValueUnexpected", "o item não está na lista")],
        ex=[('post([10, 20, 30].index(20))', "1")],
        bordas=["ERRA se não achar — pra só testar presença, use `contains`"]),
    "count": dict(sig='l.count(item)', resumo="Quantas vezes o item aparece.",
        params=[("item", "qualquer", "—", "o valor a contar")], ret="int", erros=[],
        ex=[('post([1, 2, 1, 1].count(1), [1, 2].count(9))', "3 0")], bordas=[]),
    "contains": dict(sig='l.contains(item)', resumo="O item está na lista? (o mesmo que `item in l`)",
        params=[("item", "qualquer", "—", "o valor a procurar")], ret="bool", erros=[],
        ex=[('l = [1, 2]\npost(l.contains(2), l.contains(9), 2 in l)', "True False True")],
        bordas=[]),
    "has": dict(sig='l.has(item)', resumo="Apelido de `contains` — as duas grafias existem.",
        params=[("item", "qualquer", "—", "o valor a procurar")], ret="bool", erros=[],
        ex=[('post([1, 2].has(1), [1, 2].has(9))', "True False")], bordas=[]),
    "copy": dict(sig='l.copy()', resumo="Cópia RASA: lista nova, itens compartilhados.",
        params=[], ret="list — a cópia", erros=[],
        ex=[('l = [1, 2]\nc = l.copy()\nc.append(3)\npost(l, c)', "[1, 2] [1, 2, 3]"),
            ('dentro = [1]\nl = [dentro]\nc = l.copy()\ndentro.append(2)\npost(c)', "[[1, 2]]")],
        bordas=["RASA: mexer num item aninhado aparece nas duas listas (2º exemplo)"]),
    "len": dict(sig='l.len()', resumo="Quantidade de itens — forma de método do builtin `len`.",
        params=[], ret="int", erros=[],
        ex=[('l = [1, 2, 3]\npost(l.len(), len(l))', "3 3")], bordas=[]),
}
