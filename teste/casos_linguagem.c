/*
 * Semântica da linguagem: escopo, laço, string/UTF-8, coleção, arquivo,
 * import. Casos que já foram bug pelo menos uma vez — e o básico ao redor
 * deles, pra a correção não passar por cima do que funcionava.
 */
#include "ps_teste.h"

const Caso CASOS_LINGUAGEM[] = {
/* ── escopo ── */
{ "for each sombreia variável de fora",
  "i = \"importante\"\nfor each i in [1, 2]:\n    post(i)\npost(i)\n",
  "1\n2\nimportante", NULL, 0 },
{ "for each não apaga action de mesmo nome",
  "action f():\n    return \"sou action\"\nfor each f in [1, 2]:\n    post(f)\npost(f())\n",
  "1\n2\nsou action", NULL, 0 },
{ "variável do laço não vaza",
  "for each z in [1]:\n    post(z)\npost(z)\n", NULL, "não definida", -1 },
{ "variável de bloco não vaza",
  "if true:\n    dentro = 5\npost(dentro)\n", NULL, "não definida", -1 },
{ "atribuir nome de fora dentro do bloco altera o de fora",
  "x = 1\nif true:\n    x = 2\npost(x)\n", "2", NULL, 0 },
{ "acumulador em laço",
  "total = 0\nfor each i in [1, 2, 3]:\n    total = total + i\npost(total)\n", "6", NULL, 0 },

/* ── UTF-8: fatia e busca em CARACTERES, não bytes ── */
{ "fatia com acento",
  "s = \"padrão: str\"\npost(s[0:6], s[5], s[-3:len(s)])\n", "padrão o str", NULL, 0 },
{ "fatia com passo negativo",
  "post(\"padrão\"[::-1])\n", "oãrdap", NULL, 0 },
{ "índice de caractere multibyte",
  "post(\"ação\"[1], \"ação\"[-1], \"ação\"[1:3])\n", "ç o çã", NULL, 0 },
{ "find com início e fim",
  "s = \"banana\"\npost(s.find(\"na\"), s.find(\"na\", 3), s.find(\"na\", 0, 3))\n", "2 4 -1", NULL, 0 },
{ "count com faixa",
  "s = \"banana\"\npost(s.count(\"na\"), s.count(\"na\", 3), s.count(\"a\", 1, 4))\n", "2 1 2", NULL, 0 },
{ "index erra quando não acha na faixa",
  "post(\"banana\".index(\"na\", 0, 3))\n", NULL, "SomeValueUnexpected", -1 },
{ "len conta caracteres",
  "post(len(\"ação\"), len(\"abc\"))\n", "4 3", NULL, 0 },

/* ── coleções ── */
{ "dict: in olha a chave",
  "d = { \"nome\": \"ana\", \"idade\": 30 }\npost(\"nome\" in d, \"ana\" in d)\n", "True False", NULL, 0 },
{ "dict.value() olha o valor",
  "d = { \"nome\": \"ana\", \"idade\": 30 }\npost(\"ana\" in d.value(), 30 in d.value())\n",
  "True True", NULL, 0 },
{ "dict.value() com list/tup/flo",
  "d = { \"a\": [1, 2], \"b\": (3, 4), \"c\": 2.5 }\n"
  "post([1, 2] in d.value(), (3, 4) in d.value(), 2.5 in d.value())\n",
  "True True True", NULL, 0 },
{ "value() e values() são o mesmo",
  "d = { \"a\": 1, \"b\": 2 }\npost(d.value() == d.values())\n", "True", NULL, 0 },
{ "tupla é imutável: append",
  "t = (1, 2, 3)\nt.append(9)\n", NULL, "membro inexistente", -1 },
{ "tupla é imutável: sort",
  "t = (1, 2, 3)\nt.sort()\n", NULL, "membro inexistente", -1 },
{ "tupla é imutável: copy",
  "t = (1, 2, 3)\nt.copy()\n", NULL, "membro inexistente", -1 },
{ "tupla lê normal",
  "t = (1, 2, 3)\npost(t.len(), t.count(1), t.index(2), t.contains(2))\n", "3 1 1 True", NULL, 0 },
{ "lista continua com tudo",
  "l = [3, 1, 2]\nl.append(4)\nl.sort()\npost(l)\npost(l.pop(), l.index(2), l.copy())\npost(l)\n",
  "[1, 2, 3, 4]\n4 1 [1, 2, 3]\n[1, 2, 3]", NULL, 0 },
{ "fatia de bytes",
  "c = \"ola\".encode()\npost(c[0:2], c[-1:], len(c[:]))\n", "b'ol' b'a' 3", NULL, 0 },

/* ── regex ── */
{ "regex.compile devolve Pattern",
  "import regex\np = regex.compile(\"\\\\d+\")\npost(type(p), p.pattern)\n", "Pattern \\d+", NULL, 0 },
{ "Pattern reusado várias vezes",
  "import regex\np = regex.compile(\"\\\\d+\")\n"
  "post(p.sub(\"#\", \"a1b22\"), p.split(\"a1b22c\"))\npost(p.findall(\"x9y8\"), p.match(\"77\"))\n",
  "a#b# ['a', 'b', 'c']\n['9', '8'] True", NULL, 0 },
{ "regex.fullmatch é o match",
  "import regex\npost(regex.fullmatch(\"\\\\d+\", \"123\"), regex.fullmatch(\"\\\\d+\", \"a123\"))\n",
  "True False", NULL, 0 },
{ "regex.compile de padrão inválido erra cedo",
  "import regex\np = regex.compile(\"[a-\")\n", NULL, "SomeValueUnexpected", -1 },

/* ── arquivo ── */
{ "read(n) não consome o arquivo",
  "import os\nos.writeFile(\"/tmp/ps_teste_leitura.txt\", \"abcdef\")\n"
  "using open(\"/tmp/ps_teste_leitura.txt\") as f:\n"
  "    post(f.read(3))\n    post(f.read(2))\n    post(f.read())\n",
  "abc\nde\nf", NULL, 0 },

/* ── request: head e multipart ── */
{ "request.head existe",
  "import request\npost(type(request.head))\n", "action", NULL, 0 },
{ "request tem os métodos HTTP",
  "import request\npost(type(request.get), type(request.post), type(request.delete))\n",
  "action action action", NULL, 0 },

/* ── closure: action aninhada captura o escopo de fora ──────────────────
 * A captura é por CÉLULA (o modelo do CPython): quem declara e quem captura
 * mexem no mesmo valor, então a aninhada VÊ e MUTA a variável de fora. */
{ "aninhada lê local de fora",
  "action fora():\n    a = 1\n    action dentro():\n        post(a)\n    dentro()\nfora()\n",
  "1", NULL, 0 },
{ "aninhada chama a si mesma (recursão)",
  "action fora():\n    action rec(n):\n        if n <= 0:\n            return 0\n        return rec(n - 1)\n"
  "    return rec(3)\npost(fora())\n",
  "0", NULL, 0 },
{ "aninhada dentro de método enxerga self",
  "Entity C():\n    action __init__(self):\n        self.v = 3\n    action m(self):\n"
  "        action inner():\n            return self.v\n        return inner()\nc = C()\npost(c.m())\n",
  "3", NULL, 0 },
{ "contador: a aninhada MUTA a variável de fora",
  "action faz():\n    n = 0\n    action inc():\n        n = n + 1\n        return n\n    return inc\n"
  "c = faz()\npost(c(), c(), c())\n",
  "1 2 3", NULL, 0 },
{ "cada closure tem o próprio estado",
  "action faz():\n    n = 0\n    action inc():\n        n = n + 1\n        return n\n    return inc\n"
  "a = faz()\nb = faz()\npost(a(), a(), b())\n",
  "1 2 1", NULL, 0 },
{ "captura em cadeia (três níveis)",
  "action n1():\n    a = 10\n    action n2():\n        action n3():\n            return a * 2\n"
  "        return n3()\n    return n2()\npost(n1())\n",
  "20", NULL, 0 },
{ "closure como callback do map",
  "action mult(k):\n    action f(x):\n        return x * k\n    return f\n"
  "post(map([1, 2, 3], mult(10)))\n",
  "[10, 20, 30]", NULL, 0 },
{ "parâmetro capturado",
  "action soma(a):\n    action mais(b):\n        return a + b\n    return mais\npost(soma(3)(4))\n",
  "7", NULL, 0 },
{ "closures do laço compartilham a variável",
  "action junta():\n    fs = []\n    for each i in range(3):\n        action g():\n            return i\n"
  "        fs.append(g)\n    return fs\nfs = junta()\npost(fs[0](), fs[1](), fs[2]())\n",
  "2 2 2", NULL, 0 },
{ "recursão mútua entre aninhadas",
  "action mutuo(n):\n    action par(k):\n        if k == 0:\n            return true\n"
  "        return impar(k - 1)\n    action impar(k):\n        if k == 0:\n            return false\n"
  "        return par(k - 1)\n    return par(n)\npost(mutuo(4), mutuo(7))\n",
  "True False", NULL, 0 },
{ "global continua visível de dentro da aninhada",
  "G = 99\naction f():\n    action dentro():\n        return G\n    return dentro()\npost(f())\n",
  "99", NULL, 0 },
{ "gerador aninhado mantém a captura",
  "action faz(k):\n    action g():\n        for each i in range(3):\n            yield i * k\n    return g\n"
  "s = 0\nfor each v in faz(10)():\n    s = s + v\npost(s)\n",
  "30", NULL, 0 },
{ "closure em async action (fibra)",
  "async action t(k):\n    action calc():\n        return k * 3\n    return calc()\n"
  "post(gather(t(1), t(2)))\n",
  "[3, 6]", NULL, 0 },
{ "closure sobrevive ao GC e continua mutando",
  "action cont():\n    n = 0\n    action inc():\n        n = n + 1\n        return n\n    return inc\n"
  "c = cont()\nfor each i in range(50000):\n    lixo = [i, i, i]\npost(c(), c())\n",
  "1 2", NULL, 0 },
{ "muitas closures vivas ao mesmo tempo",
  "action cria(n):\n    action f():\n        return n\n    return f\n"
  "l = []\nfor each i in range(20000):\n    l.append(cria(i))\npost(len(l), l[0](), l[19999]())\n",
  "20000 0 19999", NULL, 0 },
{ "type() de closure é action",
  "action f():\n    a = 1\n    action g():\n        return a\n    return g\npost(type(f()))\n",
  "action", NULL, 0 },
{ "variável capturada usada antes de receber valor é erro",
  "action f():\n    action g():\n        return z\n    r = g()\n    z = 1\n    return r\npost(f())\n",
  NULL, "'z'", -1 },

/* ── `str(x)` tem que dizer o MESMO que `post(x)` ────────────────────────
 * Eram dois caminhos independentes (`escreve_valor` imprime, `valor_para_texto`
 * monta string) e o segundo não conhecia 33 tipos de objeto: `post(conn)`
 * mostrava `<sqlite3.Connection>` e `str(conn)` devolvia STRING VAZIA. Agora
 * os dois leem da mesma `descreve_obj`. */
{ "str() de conexao sqlite",
  "import sqlite3\nc = sqlite3.connect(\"/tmp/ps_str1.db\")\npost(\"[\" + str(c) + \"]\")\n",
  "[<sqlite3.Connection>]", NULL, 0 },
{ "str() de arquivo aberto",
  "using open(\"/tmp/ps_str2.txt\", \"w\") as f:\n    post(\"[\" + str(f) + \"]\")\n",
  "[<arquivo /tmp/ps_str2.txt>]", NULL, 0 },
{ "str() de gerador",
  "action g():\n    yield 1\npost(\"[\" + str(g()) + \"]\")\n",
  "[<generator g>]", NULL, 0 },
{ "str() de socket fechado",
  "import sockets\ns = sockets.socket()\ns.close()\npost(\"[\" + str(s) + \"]\")\n",
  "[<socket fechado>]", NULL, 0 },
{ "str() e post() concordam no builtin",
  "post(\"[\" + str(len) + \"]\")\n", "[<builtin>]", NULL, 0 },

/* ── dicionário multilinha x bloco de chaves ─────────────────────────────
 * O `{` é as DUAS coisas na linguagem: abre bloco e abre dicionário. Quando
 * o lexer passou a emitir indentação dentro de `{ }` (pra o bloco `:` aninhado
 * funcionar), o dicionário com continuação INDENTADA passou a empurrar um
 * nível que ninguém tirava, e a linha seguinte vinha com um DEDENT órfão:
 * `{"a": 1,\n     "b": 2}` virava "expressao invalida".
 *
 * Agora o lexer classifica cada `{` pelo token ANTERIOR: depois de operador,
 * `(`, `[`, `,`, `:`, `return`, `yield` ou `case` é DICIONÁRIO (indentação
 * ignorada); depois de `)`, nome ou literal é BLOCO (indentação conta). */
{ "dicionario multilinha com continuacao indentada",
  "d = {\"a\": 1,\n     \"b\": 2}\npost(d)\n", "{'a': 1, 'b': 2}", NULL, 0 },
{ "dicionario multilinha dentro de action",
  "action f():\n    return {\"a\": 1,\n            \"b\": 2}\npost(f())\n",
  "{'a': 1, 'b': 2}", NULL, 0 },
{ "dicionario com a chave em linha propria",
  "d = {\n    \"a\": 1,\n    \"b\": 2\n}\npost(d)\n", "{'a': 1, 'b': 2}", NULL, 0 },
{ "lista multilinha indentada",
  "action g():\n    return [1,\n            2]\npost(g())\n", "[1, 2]", NULL, 0 },
{ "dicionario aninhado multilinha",
  "d = {\"a\": {\"b\": 1,\n            \"c\": 2},\n     \"d\": 3}\npost(d)\n",
  "{'a': {'b': 1, 'c': 2}, 'd': 3}", NULL, 0 },
{ "dicionario como argumento multilinha",
  "action f(x):\n    return x[\"a\"]\npost(f({\"a\": 1,\n        \"b\": 2}))\n", "1", NULL, 0 },
{ "bloco de chaves continua sendo bloco depois de )",
  "if (true) {\n    action f():\n        return 1\n    post(f())\n}\n", "1", NULL, 0 },
{ "match usa bloco, e o padrao dict usa dicionario",
  "d = {\"a\": 1}\nmatch d {\n case {a: 1} { post(\"casou\") }\n case _ { post(\"nao\") }\n}\n",
  "casou", NULL, 0 },
{ "for each com bloco de chaves",
  "for each i in [1,2] {\n    post(i)\n}\n", "1\n2", NULL, 0 },

/* ── `full` e `mei` não são palavras da linguagem ────────────────────────
 * Estavam na lista de reservadas desde a criação do lexer em C, copiadas da
 * tabela do interpretador — mas são valor de argumento de UMA lib (manpu), e
 * o C sempre os comparou como STRING. Reservados serviam pra nada: não viravam
 * valor (`post(full)` dava "variável não definida") e bloqueavam os dois nomes
 * pro usuário. */
{ "full e mei valem como nome de variavel",
  "full = 1\nmei = \"x\"\npost(full, mei)\n", "1 x", NULL, 0 },

/* ── aridade de método nativo ────────────────────────────────────────────
 * A linguagem sempre recusou `f(1,2,3)` numa action de zero parâmetros, mas
 * 95 métodos nativos engoliam argumento a mais em silêncio. O teto agora sai
 * do próprio `params` da tabela, conferido no despacho.
 *
 * Chegar aqui custou dois erros meus: primeiro tratei "params vazio" como
 * zero-argumento com base numa varredura de TEXTO, e quebrei
 * `Pattern.match/search/findall`, que leem o argumento por um helper. A regra
 * só voltou depois de sondar as 91 entradas por COMPORTAMENTO. */
{ "metodo nativo recusa argumento demais",
  "post([1,2].append(1,2,3))\n", NULL, "aceita ate 1 argumento, recebeu 3", -1 },
{ "metodo de zero argumento recusa argumento",
  "import sockets\ns = sockets.socket()\npost(s.fileno(1,2))\n",
  NULL, "nao aceita argumento, recebeu 2", -1 },
{ "format continua variadico",
  "post(\"{} {}\".format(1,2,3,4,5))\n", "1 2", NULL, 0 },
{ "Pattern le o argumento pelo helper e continua funcionando",
  "import regex\np = regex.compile(\"\\\\d+\")\n"
  "post(p.match(\"77\"), p.search(\"a9\"), p.findall(\"x9y8\"), p.fullmatch(\"12\"))\n",
  "True True ['9', '8'] True", NULL, 0 },
{ "elemento do guzer sem argumento continua valendo",
  "import guzer\na = guzer.UI()\npost(type(a.p()), type(a.div()))\n", "p div", NULL, 0 },

/* ── jinker: status e corpo vazio ────────────────────────────────────────
 * Achados rodando o servidor da linguagem em loopback e batendo nele com o
 * cliente dela (teste/e2e/). Nenhum caso escrito à mão pegaria: os dois só
 * aparecem quando a requisição atravessa de verdade. */
{ "status() nao e sobrescrito pelo json()",
  "import jinker\nr = jinker.JinkerResponse()\npost(r.status(418).json({\"a\": 1}).status_code)\n",
  "418", NULL, 0 },
{ "status() nao e sobrescrito pelo send()",
  "import jinker\nr = jinker.JinkerResponse()\npost(r.status(503).send(\"x\").status_code)\n",
  "503", NULL, 0 },
{ "json(dados, status) continua mandando no status",
  "import jinker\nr = jinker.JinkerResponse()\npost(r.json({\"a\": 1}, 201).status_code)\n",
  "201", NULL, 0 },
{ "resposta nasce com 200",
  "import jinker\npost(jinker.JinkerResponse().json({\"a\": 1}).status_code)\n",
  "200", NULL, 0 },

/* ── conferido contra o Python (oráculo) ─────────────────────────────────
 * 4484 expressões rodadas no `pool` E no `python3`, comparando resultado. A
 * doc diz "igual ao Python" em toda parte, então divergência é defeito até
 * prova em contrário. Estes três eram defeito. */
{ "repeticao de string",
  "post(\"ab\" * 3, 3 * \"ab\", \"e\" * 3)\n", "ababab ababab eee", NULL, 0 },
{ "repeticao de string com contagem <= 0",
  "post(\"[\" + \"ab\" * 0 + \"]\", \"[\" + \"ab\" * -1 + \"]\")\n", "[] []", NULL, 0 },
{ "repeticao de string conta CARACTERE, nao byte",
  "post(\"é\" * 3, len(\"é\" * 3))\n", "ééé 3", NULL, 0 },
{ "ss é letra minúscula (nao tem maiuscula de 1 caractere)",
  "post(\"ß\".isalpha(), \"ß\".islower(), \"ß\".isalnum(), \"ß\".isupper())\n",
  "True True True False", NULL, 0 },
{ "isidentifier recusa sobrescrito e digito nao-decimal",
  "post(\"²³\".isidentifier(), \"٣٤\".isidentifier(), \"a²\".isidentifier())\n",
  "False False False", NULL, 0 },
{ "isidentifier aceita acento, digito decimal depois do inicio e _",
  "post(\"ção\".isidentifier(), \"a٣\".isidentifier(), \"_x1\".isidentifier(), \"1abc\".isidentifier())\n",
  "True True True False", NULL, 0 },
{ "preenchimento de string vazia",
  "post(\"[\" + \"\".rjust(3) + \"]\", \"[\" + \"\".ljust(3) + \"]\", \"[\" + \"\".center(3) + \"]\")\n",
  "[   ] [   ] [   ]", NULL, 0 },

/* ── misturar os dois estilos de bloco ───────────────────────────────────
 * A doc promete indentação **ou** chaves "à vontade e no mesmo arquivo", mas
 * um bloco `:` DENTRO de `{ }` morria com "faltou quebra de linha apos ':'":
 * o lexer não emitia NEWLINE/INDENT dentro de chaves. Emite agora, e a
 * indentação dentro de `{ }` passou a ser LIVRE (o `}` é que fecha) — fora
 * dela a regra dos 4 espaços continua valendo. */
{ "action ':' dentro de bloco de chaves",
  "if (true) {\n    action f():\n        return 1\n    post(f())\n}\n", "1", NULL, 0 },
{ "if ':' dentro de bloco de chaves",
  "if (true) {\n    if true:\n        post(\"x\")\n}\n", "x", NULL, 0 },
{ "try/catch ':' dentro de bloco de chaves",
  "if (true) {\n    try:\n        raise Boom(\"x\")\n    catch(e):\n        post(\"peguei\")\n}\n",
  "peguei", NULL, 0 },
{ "for each ':' dentro de bloco de chaves",
  "if (true) {\n    for each i in range(2):\n        post(i)\n}\n", "0\n1", NULL, 0 },
{ "bloco de chaves dentro de bloco ':'",
  "if true:\n    action f() {\n        return 1\n    }\n    post(f())\n", "1", NULL, 0 },
{ "metodo ':' dentro de Entity de chaves",
  "Entity P() {\n    action m(self):\n        return 3\n}\npost(P().m())\n", "3", NULL, 0 },
{ "metodo de chaves dentro de Entity ':'",
  "Entity Q():\n    action m(self) {\n        return 4\n    }\npost(Q().m())\n", "4", NULL, 0 },
{ "indentacao livre dentro de chaves",
  "action r(n) {\n if (n < 1) {\n  return 0\n }\n return 1 + r(n - 1)\n}\npost(r(5))\n",
  "5", NULL, 0 },
{ "model de chaves continua valendo",
  "model M() {\n    a: str(length=3)\n}\npost(\"ok\")\n", "ok", NULL, 0 },
{ "fora de chaves a regra dos 4 espacos vale",
  "if true:\n  post(\"2 espacos\")\n", NULL, "multiplo de 4", -1 },
{ "dicionario multilinha nao vira bloco",
  "d = {\n    \"a\": 1,\n    \"b\": 2\n}\npost(d)\n", "{'a': 1, 'b': 2}", NULL, 0 },

/* ── `for each` sobre range não materializa a lista ──────────────────────
 * `range(20000000)` construía 20 milhões de itens (325 MB) só pra o laço
 * jogar fora um a um; o `while` equivalente usava 13 MB. O laço agora conta
 * por aritmética. Estes casos travam a SEMÂNTICA — a medida de memória está
 * no notas/LIMITACOES.md. */
{ "range de um argumento",
  "for each i in range(3):\n    post(i)\n", "0\n1\n2", NULL, 0 },
{ "range com inicio e fim",
  "for each i in range(2, 5):\n    post(i)\n", "2\n3\n4", NULL, 0 },
{ "range com passo negativo",
  "for each i in range(10, 0, -3):\n    post(i)\n", "10\n7\n4\n1", NULL, 0 },
{ "range vazio nao entra no laco",
  "for each i in range(0):\n    post(\"NAO\")\npost(\"fim\")\n", "fim", NULL, 0 },
{ "range aceita texto numerico",
  "for each i in range(\"3\"):\n    post(i)\n", "0\n1\n2", NULL, 0 },
{ "range com passo 0 e erro",
  "for each i in range(1, 5, 0):\n    post(i)\n", NULL, "passo de range", -1 },
{ "range com expressao nos limites",
  "n = 4\nfor each i in range(n - 2):\n    post(i)\n", "0\n1", NULL, 0 },
{ "break e continue dentro de range",
  "for each i in range(10):\n    if i == 3:\n        break\n    if i == 1:\n        continue\n    post(i)\n",
  "0\n2", NULL, 0 },
{ "range aninhado",
  "for each i in range(2):\n    for each j in range(2):\n        post(i, j)\n",
  "0 0\n0 1\n1 0\n1 1", NULL, 0 },
{ "range fora do laco continua sendo lista",
  "r = range(4)\npost(type(r), len(r), r[2], r)\n", "list 4 2 [0, 1, 2, 3]", NULL, 0 },
{ "range redefinido pelo usuario ganha do embutido",
  "action range(n):\n    return [\"MEU\", n]\nfor each x in range(2):\n    post(x)\n",
  "MEU\n2", NULL, 0 },

/* ── `char` como tipo declarável ─────────────────────────────────────────
 * `char c = 64` dava "variável não definida: char": o parser só abria
 * declaração tipada pros quatro escalares. `char` é a restrição da DECLARAÇÃO
 * (um caractere); o valor em runtime é `str`. */
{ "char aceita um caractere",
  "char a = \"x\"\npost(a, type(a))\n", "x str", NULL, 0 },
{ "char converte inteiro pelo codepoint",
  "char opa = 64\npost(opa)\n", "@", NULL, 0 },
{ "char aceita caractere fora do ASCII",
  "char c = \"ç\"\nchar e = 128512\npost(c, len(c), e)\n", "ç 1 😀", NULL, 0 },
{ "char recusa mais de um caractere",
  "char c = \"abc\"\n", NULL, "esperava char", -1 },
{ "char recusa flutuante",
  "char c = 1.5\n", NULL, "esperava char", -1 },
{ "char recusa codepoint invalido",
  "char c = -1\n", NULL, "nao e um caractere valido", -1 },
{ "char action nao existe",
  "char action f():\n    return 1\n", NULL, "int action", -1 },
{ "as outras declaracoes tipadas continuam iguais",
  "str a = \"oi\"\nint b = \"7\"\nflo c = 1\nbool d = true\npost(a, b, c, d)\n",
  "oi 7 1.0 True", NULL, 0 },
{ "int tipado ainda recusa texto invalido",
  "int x = \"abc\"\n", NULL, "ConversionError", -1 },

/* ── módulos sem `import` ────────────────────────────────────────────────
 * `Parsing` é namespace pré-ligado e `sys.stdout`/`sys.stderr` são atributos:
 * nenhum dos três se importa. O `--metadata` chegou a anunciá-los pelo nome de
 * REGISTRO (`_Parsing`, `_stdout`), e aí o editor sugeria `import _stdout`, que
 * só podia dar ImportError. */
{ "Parsing existe sem import",
  "post(type(Parsing), type(Parsing.integer))\n", "module action", NULL, 0 },
{ "sys.stdout e sys.stderr são atributos de sys",
  "import sys\npost(type(sys.stdout), type(sys.stderr), type(sys.stdout.write))\n",
  "module module action", NULL, 0 },
{ "import de nome interno é erro",
  "import _stdout\n", NULL, "modulo nao encontrado", -1 },

/* ── `pass` — no-op igual ao Python ─────────────────────────────────────────
 * Nasceu porque a doc do middleware do jinker mandava usar `continue` fora de
 * laço, que não compila. `pass` vale em QUALQUER posição de statement. */
{ "pass como corpo de action devolve null",
  "action f():\n    pass\npost(f())\n", "null", NULL, 0 },
{ "pass no if e no else",
  "x = 0\nif x == 0:\n    pass\nelse:\n    post(\"nao\")\npost(\"ok\")\n", "ok", NULL, 0 },
{ "pass em laco nao interrompe",
  "for each n in range(3):\n    pass\npost(\"fim\")\n", "fim", NULL, 0 },
{ "pass nao encerra o resto do bloco",
  "action f():\n    pass\n    return 7\npost(f())\n", "7", NULL, 0 },
{ "pass no while",
  "n = 0\nwhile n < 3:\n    n += 1\n    pass\npost(n)\n", "3", NULL, 0 },
{ "pass no catch engole o erro",
  "try:\n    raise Boom(\"x\")\ncatch (e):\n    pass\npost(\"seguiu\")\n", "seguiu", NULL, 0 },
{ "pass com chaves",
  "action f() { pass }\npost(f())\n", "null", NULL, 0 },
{ "pass e palavra reservada",
  "pass = 1\n", NULL, "palavra reservada", -1 },
{ "pass no corpo de classe",
  "class Vazia():\n    pass\npost(type(Vazia))\n", "Entity", NULL, 0 },
{ "pass fora de laco NAO e erro (ao contrario de continue)",
  "pass\npost(\"ok\")\n", "ok", NULL, 0 },
{ "continue fora de laco continua sendo erro",
  "continue\n", NULL, "'continue' fora de laco", -1 },

/* ── input(): fim da entrada é `null`, linha vazia é `""` ───────────────────
 * O runner roda todo caso com stdin em /dev/null, então aqui a entrada já
 * começa acabada. Sem o `null`, `while true: input()` giraria pra sempre
 * quando o outro lado fechasse o cano — foi o que travou o servidor LSP. */
{ "input() no fim da entrada devolve null",
  "post(input())\n", "null", NULL, 0 },
{ "input() no fim é null, não string vazia",
  "post(input() == Null, input() == \"\")\n", "True False", NULL, 0 },
{ "input() em laço termina no fim da entrada",
  "n = 0\nwhile true:\n    l = input()\n    if l == Null:\n        break\n    n = n + 1\npost(\"linhas:\", n)\n",
  "linhas: 0", NULL, 0 },

/* ── módulo: o erro nomeia o membro e sugere o parecido ─────────────────── */
{ "membro inexistente nomeia modulo e membro",
  "import json\npost(json.naoexiste)\n", NULL,
  "módulo 'json' não tem membro 'naoexiste'", -1 },
{ "membro parecido vira sugestao",
  "import json\npost(json.parsee)\n", NULL, "você quis dizer 'parse'?", -1 },

/* ── CLI ── */
{ "--check não executa o script",
  "post(\"NAO DEVIA RODAR\")\n", "NAO DEVIA RODAR", NULL, 0 },
};
const int NC_LINGUAGEM = N_CASOS(CASOS_LINGUAGEM);
