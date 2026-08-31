/*
 * Semântica da linguagem: escopo, laço, string/UTF-8, coleção, arquivo,
 * import. Casos que já foram bug pelo menos uma vez — e o básico ao redor
 * deles, pra a correção não passar por cima do que funcionava.
 */
#include "ps_teste.h"

const Caso CASOS_LINGUAGEM[] = {
/* ── escopo ── */
{ "for each sombreia variável de fora",
  "i = \"importante\"\n"
  "for each i in [1, 2] {\n"
  "    post(i)\n"
  "}\n"
  "post(i)\n",
  "1\n2\nimportante", NULL, 0 },
{ "for each não apaga action de mesmo nome",
  "action f() {\n"
  "    return \"sou action\"\n"
  "}\n"
  "for each f in [1, 2] {\n"
  "    post(f)\n"
  "}\n"
  "post(f())\n",
  "1\n2\nsou action", NULL, 0 },
{ "variável do laço não vaza",
  "for each z in [1] {\n"
  "    post(z)\n"
  "}\n"
  "post(z)\n", "1", "name 'z' is not defined", 1 },
{ "variável de bloco não vaza",
  "if true {\n"
  "    dentro = 5\n"
  "}\n"
  "post(dentro)\n", "", "name 'dentro' is not defined", 1 },
{ "atribuir nome de fora dentro do bloco altera o de fora",
  "x = 1\n"
  "if true {\n"
  "    x = 2\n"
  "}\n"
  "post(x)\n", "2", NULL, 0 },
{ "acumulador em laço",
  "total = 0\n"
  "for each i in [1, 2, 3] {\n"
  "    total = total + i\n"
  "}\n"
  "post(total)\n", "6", NULL, 0 },

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
  "post(\"banana\".index(\"na\", 0, 3))\n", "", "ValueError: substring not found", 1 },
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
  "t = (1, 2, 3)\nt.append(9)\n", "", "'tup' object has no attribute 'append'", 1 },
{ "tupla é imutável: sort",
  "t = (1, 2, 3)\nt.sort()\n", "", "'tup' object has no attribute 'sort'", 1 },
{ "tupla é imutável: copy",
  "t = (1, 2, 3)\nt.copy()\n", "", "'tup' object has no attribute 'copy'", 1 },
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
/* ── I10: `is` com literal a direita ────────────────────────────────────────
 * `is` e o operador de TIPO. Com literal a direita ele caia em igualdade de
 * valor CALADO: `x is 0` no lugar de `x == 0` dava o resultado "certo" e nunca
 * reclamava. */
{ "is com literal a direita e erro de compilacao",
  "x = 5\npost(x is 5)\n", "", "para comparar valor use '=='", 2 },
{ "is com str a direita tambem",
  "post(\"a\" is \"a\")\n", "", "literal 'str' a direita", 2 },
{ "is com TIPO a direita continua valendo",
  "post(5 is int, \"x\" is str, 3.0 is flo)\n"
  "x = [1, 2]\n"
  "post(x is list, x is not dict)\n", "True True True\nTrue True", NULL, 0 },
{ "is Null e o idioma de ausencia, e fica",
  /* equivale ao `x is None` do Python; `type(null)` e literalmente "Null" */
  "x = Null\npost(x is Null)\n", "True", NULL, 0 },
{ "is com NOME a direita continua valendo",
  /* o nome pode guardar um tipo — so o literal e recusado */
  "x = 5\nt = int\npost(x is t)\n", "True", NULL, 0 },

/* ── I11: `//`, `**` e `pow()` ───────────────────────────────────────────────
 * A linguagem tinha `/` sempre real e nada pra quociente inteiro nem pra
 * potencia. Num idioma com int de precisao arbitraria isso perdia precisao
 * calado na unica divisao que existia. */
{ "// e divisao inteira com piso, nao truncamento",
  "post(7 // 2, -7 // 2, 7 // -2, -7 // -2)\n", "3 -4 -4 3", NULL, 0 },
{ "// entre inteiros da int; com flo da flo",
  "post(7 // 2, 7.0 // 2, 7 // 2.0)\n", "3 3.0 3.0", NULL, 0 },
{ "// nao perde precisao onde int(a / b) perdia",
  /* o caso que motivou o item: `/` passa por double */
  "post(int(10000000000000001 / 1))\n"
  "post(10000000000000001 // 1)\n",
  "10000000000000000\n10000000000000001", NULL, 0 },
{ "// por zero levanta",
  "post(7 // 0)\n", "", "integer division or modulo by zero", 1 },
{ "// deixou de ser comentario",
  /* era comentario de linha ate esta mudanca; hoje o comentario e `#` */
  "x = 10 // 3\npost(x)\n", "3", NULL, 0 },
{ "** com as tres regras de precedencia do Python",
  "post(-2 ** 2)\n"      /* mais forte que o unario a ESQUERDA  */
  "post(2 ** -1)\n"      /* mais fraco a DIREITA                */
  "post(2 ** 3 ** 2)\n", /* associa a DIREITA                   */
  "-4\n0.5\n512", NULL, 0 },
{ "** promove a bignum",
  "post(2 ** 100)\n", "1267650600228229401496703205376", NULL, 0 },
{ "0 ** negativo levanta",
  "post(0 ** -1)\n", "", "cannot be raised to a negative power", 1 },
{ "pow() de 2 e de 3 argumentos",
  /* o 3o argumento e o que o operador nao tem: potencia modular, sem
   * materializar a potencia inteira */
  "post(pow(2, 10), pow(2, -1), pow(2.0, 3))\n"
  "post(pow(3, 200, 1000), pow(7, 128, 13))\n",
  "1024 0.5 8.0\n1 3", NULL, 0 },
{ "pow() com 3 argumentos exige inteiros",
  "post(pow(2.0, 3, 5))\n", "", "3rd argument not allowed", 1 },

{ "regex.compile de padrão inválido erra cedo",
  /* ValueError, nao TypeError: o argumento E uma str (o tipo esta certo) — o
   * que nao serve e o VALOR dela. O CPython usa `re.error`, que a linguagem
   * nao tem; ValueError e o parente mais proximo, e e a mesma divisao que o
   * resto do motor segue. */
  "import regex\np = regex.compile(\"[a-\")\n", "",
  "ValueError: unterminated character set at position 0", 1 },

/* ── arquivo ── */
{ "read(n) não consome o arquivo",
  "import os\n"
  "os.writeFile(\"/tmp/ps_teste_leitura.txt\", \"abcdef\")\n"
  "using open(\"/tmp/ps_teste_leitura.txt\") as f {\n"
  "    post(f.read(3))\n"
  "    post(f.read(2))\n"
  "    post(f.read())\n"
  "}\n",
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
  "action fora() {\n"
  "    a = 1\n"
  "    action dentro() {\n"
  "        post(a)\n"
  "    }\n"
  "    dentro()\n"
  "}\n"
  "fora()\n",
  "1", NULL, 0 },
{ "aninhada chama a si mesma (recursão)",
  "action fora() {\n"
  "    action rec(n) {\n"
  "        if n <= 0 {\n"
  "            return 0\n"
  "        }\n"
  "        return rec(n - 1)\n"
  "    }\n"
  "    return rec(3)\n"
  "}\n"
  "post(fora())\n",
  "0", NULL, 0 },
{ "aninhada dentro de método enxerga self",
  "Entity C() {\n"
  "    action __init__(self) {\n"
  "        self.v = 3\n"
  "    }\n"
  "    action m(self) {\n"
  "        action inner() {\n"
  "            return self.v\n"
  "        }\n"
  "        return inner()\n"
  "    }\n"
  "}\n"
  "c = C()\n"
  "post(c.m())\n",
  "3", NULL, 0 },
{ "contador: a aninhada MUTA a variável de fora",
  "action faz() {\n"
  "    n = 0\n"
  "    action inc() {\n"
  "        n = n + 1\n"
  "        return n\n"
  "    }\n"
  "    return inc\n"
  "}\n"
  "c = faz()\n"
  "post(c(), c(), c())\n",
  "1 2 3", NULL, 0 },
{ "cada closure tem o próprio estado",
  "action faz() {\n"
  "    n = 0\n"
  "    action inc() {\n"
  "        n = n + 1\n"
  "        return n\n"
  "    }\n"
  "    return inc\n"
  "}\n"
  "a = faz()\n"
  "b = faz()\n"
  "post(a(), a(), b())\n",
  "1 2 1", NULL, 0 },
{ "captura em cadeia (três níveis)",
  "action n1() {\n"
  "    a = 10\n"
  "    action n2() {\n"
  "        action n3() {\n"
  "            return a * 2\n"
  "        }\n"
  "        return n3()\n"
  "    }\n"
  "    return n2()\n"
  "}\n"
  "post(n1())\n",
  "20", NULL, 0 },
{ "closure como callback do map",
  "action mult(k) {\n"
  "    action f(x) {\n"
  "        return x * k\n"
  "    }\n"
  "    return f\n"
  "}\n"
  "post(map([1, 2, 3], mult(10)))\n",
  "[10, 20, 30]", NULL, 0 },
{ "parâmetro capturado",
  "action soma(a) {\n"
  "    action mais(b) {\n"
  "        return a + b\n"
  "    }\n"
  "    return mais\n"
  "}\n"
  "post(soma(3)(4))\n",
  "7", NULL, 0 },
{ "closures do laço compartilham a variável",
  "action junta() {\n"
  "    fs = []\n"
  "    for each i in range(3) {\n"
  "        action g() {\n"
  "            return i\n"
  "        }\n"
  "        fs.append(g)\n"
  "    }\n"
  "    return fs\n"
  "}\n"
  "fs = junta()\n"
  "post(fs[0](), fs[1](), fs[2]())\n",
  "2 2 2", NULL, 0 },
{ "recursão mútua entre aninhadas",
  "action mutuo(n) {\n"
  "    action par(k) {\n"
  "        if k == 0 {\n"
  "            return true\n"
  "        }\n"
  "        return impar(k - 1)\n"
  "    }\n"
  "    action impar(k) {\n"
  "        if k == 0 {\n"
  "            return false\n"
  "        }\n"
  "        return par(k - 1)\n"
  "    }\n"
  "    return par(n)\n"
  "}\n"
  "post(mutuo(4), mutuo(7))\n",
  "True False", NULL, 0 },
{ "global continua visível de dentro da aninhada",
  "G = 99\n"
  "action f() {\n"
  "    action dentro() {\n"
  "        return G\n"
  "    }\n"
  "    return dentro()\n"
  "}\n"
  "post(f())\n",
  "99", NULL, 0 },
{ "gerador aninhado mantém a captura",
  "action faz(k) {\n"
  "    action g() {\n"
  "        for each i in range(3) {\n"
  "            yield i * k\n"
  "        }\n"
  "    }\n"
  "    return g\n"
  "}\n"
  "s = 0\n"
  "for each v in faz(10)() {\n"
  "    s = s + v\n"
  "}\n"
  "post(s)\n",
  "30", NULL, 0 },
{ "closure em async action (fibra)",
  "async action t(k) {\n"
  "    action calc() {\n"
  "        return k * 3\n"
  "    }\n"
  "    return calc()\n"
  "}\n"
  "post(gather(t(1), t(2)))\n",
  "[3, 6]", NULL, 0 },
{ "closure sobrevive ao GC e continua mutando",
  "action cont() {\n"
  "    n = 0\n"
  "    action inc() {\n"
  "        n = n + 1\n"
  "        return n\n"
  "    }\n"
  "    return inc\n"
  "}\n"
  "c = cont()\n"
  "for each i in range(50000) {\n"
  "    lixo = [i, i, i]\n"
  "}\n"
  "post(c(), c())\n",
  "1 2", NULL, 0 },
{ "muitas closures vivas ao mesmo tempo",
  "action cria(n) {\n"
  "    action f() {\n"
  "        return n\n"
  "    }\n"
  "    return f\n"
  "}\n"
  "l = []\n"
  "for each i in range(20000) {\n"
  "    l.append(cria(i))\n"
  "}\n"
  "post(len(l), l[0](), l[19999]())\n",
  "20000 0 19999", NULL, 0 },
{ "type() de closure é action",
  "action f() {\n"
  "    a = 1\n"
  "    action g() {\n"
  "        return a\n"
  "    }\n"
  "    return g\n"
  "}\n"
  "post(type(f()))\n",
  "action", NULL, 0 },
{ "variável capturada usada antes de receber valor é erro",
  "action f() {\n"
  "    action g() {\n"
  "        return z\n"
  "    }\n"
  "    r = g()\n"
  "    z = 1\n"
  "    return r\n"
  "}\n"
  "post(f())\n",
  "", "'z'", 1 },

/* ── `str(x)` tem que dizer o MESMO que `post(x)` ────────────────────────
 * Eram dois caminhos independentes (`escreve_valor` imprime, `valor_para_texto`
 * monta string) e o segundo não conhecia 33 tipos de objeto: `post(conn)`
 * mostrava `<sqlite3.Connection>` e `str(conn)` devolvia STRING VAZIA. Agora
 * os dois leem da mesma `descreve_obj`. */
{ "str() de conexao sqlite",
  "import sqlite3\nc = sqlite3.connect(\"/tmp/ps_str1.db\")\npost(\"[\" + str(c) + \"]\")\n",
  "[<sqlite3.Connection>]", NULL, 0 },
{ "str() de arquivo aberto",
  "using open(\"/tmp/ps_str2.txt\", \"w\") as f {\n"
  "    post(\"[\" + str(f) + \"]\")\n"
  "}\n",
  "[<arquivo /tmp/ps_str2.txt>]", NULL, 0 },
{ "str() de gerador",
  "action g() {\n"
  "    yield 1\n"
  "}\n"
  "post(\"[\" + str(g()) + \"]\")\n",
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
  "action f() {\n"
  "    return {\"a\": 1,\n"
  "            \"b\": 2}\n"
  "}\n"
  "post(f())\n",
  "{'a': 1, 'b': 2}", NULL, 0 },
{ "dicionario com a chave em linha propria",
  "d = {\n    \"a\": 1,\n    \"b\": 2\n}\npost(d)\n", "{'a': 1, 'b': 2}", NULL, 0 },
{ "lista multilinha indentada",
  "action g() {\n"
  "    return [1,\n"
  "            2]\n"
  "}\n"
  "post(g())\n", "[1, 2]", NULL, 0 },
{ "dicionario aninhado multilinha",
  "d = {\"a\": {\"b\": 1,\n            \"c\": 2},\n     \"d\": 3}\npost(d)\n",
  "{'a': {'b': 1, 'c': 2}, 'd': 3}", NULL, 0 },
{ "dicionario como argumento multilinha",
  "action f(x) {\n"
  "    return x[\"a\"]\n"
  "}\n"
  "post(f({\"a\": 1,\n"
  "        \"b\": 2}))\n", "1", NULL, 0 },
{ "bloco de chaves continua sendo bloco depois de )",
  "if (true) {\n"
  "    action f() {\n"
  "        return 1\n"
  "    }\n"
  "    post(f())\n"
  "}\n", "1", NULL, 0 },
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
  "post([1,2].append(1,2,3))\n", "", "TypeError: append() takes at most 1 argument (3 given)", 1 },
{ "metodo de zero argumento recusa argumento",
  "import sockets\ns = sockets.socket()\npost(s.fileno(1,2))\n",
  "", "TypeError: fileno() takes no arguments (2 given)", 1 },
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
{ "if ':' dentro de bloco de chaves",
  "if (true) {\n"
  "    if true {\n"
  "        post(\"x\")\n"
  "    }\n"
  "}\n", "x", NULL, 0 },
{ "try/catch ':' dentro de bloco de chaves",
  "if (true) {\n"
  "    try {\n"
  "        raise Boom(\"x\")\n"
  "    } catch(e) {\n"
  "        post(\"peguei\")\n"
  "    }\n"
  "}\n",
  "peguei", NULL, 0 },
{ "for each ':' dentro de bloco de chaves",
  "if (true) {\n"
  "    for each i in range(2) {\n"
  "        post(i)\n"
  "    }\n"
  "}\n", "0\n1", NULL, 0 },
{ "bloco de chaves dentro de bloco ':'",
  "if true {\n"
  "    action f() {\n"
  "        return 1\n"
  "    }\n"
  "    post(f())\n"
  "}\n", "1", NULL, 0 },
{ "metodo ':' dentro de Entity de chaves",
  "Entity P() {\n"
  "    action m(self) {\n"
  "        return 3\n"
  "    }\n"
  "}\n"
  "post(P().m())\n", "3", NULL, 0 },
{ "metodo de chaves dentro de Entity ':'",
  "Entity Q() {\n"
  "    action m(self) {\n"
  "        return 4\n"
  "    }\n"
  "}\n"
  "post(Q().m())\n", "4", NULL, 0 },
{ "indentacao livre dentro de chaves",
  "action r(n) {\n if (n < 1) {\n  return 0\n }\n return 1 + r(n - 1)\n}\npost(r(5))\n",
  "5", NULL, 0 },
{ "model de chaves continua valendo",
  "model M() {\n    a: str(length=3)\n}\npost(\"ok\")\n", "ok", NULL, 0 },
{ "fora de chaves a regra dos 4 espacos vale",
  "if true:\n  post(\"2 espacos\")\n", "", "multiplo de 4", 2 },

/* ── `for each` sobre range não materializa a lista ──────────────────────
 * `range(20000000)` construía 20 milhões de itens (325 MB) só pra o laço
 * jogar fora um a um; o `while` equivalente usava 13 MB. O laço agora conta
 * por aritmética. Estes casos travam a SEMÂNTICA — a medida de memória está
 * no notas/LIMITACOES.md. */
{ "range de um argumento",
  "for each i in range(3) {\n"
  "    post(i)\n"
  "}\n", "0\n1\n2", NULL, 0 },
{ "range com inicio e fim",
  "for each i in range(2, 5) {\n"
  "    post(i)\n"
  "}\n", "2\n3\n4", NULL, 0 },
{ "range com passo negativo",
  "for each i in range(10, 0, -3) {\n"
  "    post(i)\n"
  "}\n", "10\n7\n4\n1", NULL, 0 },
{ "range vazio nao entra no laco",
  "for each i in range(0) {\n"
  "    post(\"NAO\")\n"
  "}\n"
  "post(\"fim\")\n", "fim", NULL, 0 },
{ "range aceita texto numerico",
  "for each i in range(\"3\") {\n"
  "    post(i)\n"
  "}\n", "0\n1\n2", NULL, 0 },
{ "range com passo 0 e erro",
  "for each i in range(1, 5, 0) {\n"
  "    post(i)\n"
  "}\n", "", "range() arg 3 must not be zero", 1 },
{ "range com expressao nos limites",
  "n = 4\n"
  "for each i in range(n - 2) {\n"
  "    post(i)\n"
  "}\n", "0\n1", NULL, 0 },
{ "break e continue dentro de range",
  "for each i in range(10) {\n"
  "    if i == 3 {\n"
  "        break\n"
  "    }\n"
  "    if i == 1 {\n"
  "        continue\n"
  "    }\n"
  "    post(i)\n"
  "}\n",
  "0\n2", NULL, 0 },
{ "range aninhado",
  "for each i in range(2) {\n"
  "    for each j in range(2) {\n"
  "        post(i, j)\n"
  "    }\n"
  "}\n",
  "0 0\n0 1\n1 0\n1 1", NULL, 0 },
{ "range fora do laco continua sendo lista",
  "r = range(4)\npost(type(r), len(r), r[2], r)\n", "list 4 2 [0, 1, 2, 3]", NULL, 0 },
{ "range redefinido pelo usuario ganha do embutido",
  "action range(n) {\n"
  "    return [\"MEU\", n]\n"
  "}\n"
  "for each x in range(2) {\n"
  "    post(x)\n"
  "}\n",
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
  "char c = \"abc\"\n", "", "esperava char", 1 },
{ "char recusa flutuante",
  "char c = 1.5\n", "", "esperava char", 1 },
{ "char recusa codepoint invalido",
  "char c = -1\n", "", "nao e um caractere valido", 1 },
{ "char action nao existe",
  "char action f() {\n"
  "    return 1\n"
  "}\n", "", "int action", 2 },
{ "as outras declaracoes tipadas continuam iguais",
  "str a = \"oi\"\nint b = \"7\"\nflo c = 1\nbool d = true\npost(a, b, c, d)\n",
  "oi 7 1.0 True", NULL, 0 },
{ "int tipado ainda recusa texto invalido",
  "int x = \"abc\"\n", "",
  "ConversionError: não foi possível converter 'abc' para int (declarado como 'int x')", 1 },

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
  "import _stdout\n", "", "ImportError: No module named '_stdout'", 1 },

/* ── `pass` — no-op igual ao Python ─────────────────────────────────────────
 * Nasceu porque a doc do middleware do jinker mandava usar `continue` fora de
 * laço, que não compila. `pass` vale em QUALQUER posição de statement. */
{ "pass como corpo de action devolve null",
  "action f() {\n"
  "    pass\n"
  "}\n"
  "post(f())\n", "Null", NULL, 0 },
{ "pass no if e no else",
  "x = 0\n"
  "if x == 0 {\n"
  "    pass\n"
  "} else {\n"
  "    post(\"nao\")\n"
  "}\n"
  "post(\"ok\")\n", "ok", NULL, 0 },
{ "pass em laco nao interrompe",
  "for each n in range(3) {\n"
  "    pass\n"
  "}\n"
  "post(\"fim\")\n", "fim", NULL, 0 },
{ "pass nao encerra o resto do bloco",
  "action f() {\n"
  "    pass\n"
  "    return 7\n"
  "}\n"
  "post(f())\n", "7", NULL, 0 },
{ "pass no while",
  "n = 0\n"
  "while n < 3 {\n"
  "    n += 1\n"
  "    pass\n"
  "}\n"
  "post(n)\n", "3", NULL, 0 },
{ "pass no catch engole o erro",
  "try {\n"
  "    raise Boom(\"x\")\n"
  "} catch (e) {\n"
  "    pass\n"
  "}\n"
  "post(\"seguiu\")\n", "seguiu", NULL, 0 },
{ "pass com chaves",
  "action f() { pass }\npost(f())\n", "Null", NULL, 0 },
{ "pass e palavra reservada",
  "pass = 1\n", "", "palavra reservada", 2 },
{ "pass no corpo de classe",
  "class Vazia() {\n"
  "    pass\n"
  "}\n"
  "post(type(Vazia))\n", "Entity", NULL, 0 },
{ "pass fora de laco NAO e erro (ao contrario de continue)",
  "pass\npost(\"ok\")\n", "ok", NULL, 0 },
{ "continue fora de laco continua sendo erro",
  "continue\n", "", "'continue' fora de laco", 3 },

/* ── estilo Allman: a chave na LINHA SEGUINTE ───────────────────────────────
 * FALTAVA NO PORTE (nunca esteve no parser em C, conferido até a 8.2.30):
 * `Entity`/`class`/`action` aceitavam, porque o cabeçalho deles pula
 * separadores antes de procurar o `{`; `if`/`while`/`for`/`try` não. O MESMO
 * arquivo passava numa construção e falhava na outra — foi o que impedia uma
 * lib real de carregar. */
{ "if com chave na linha seguinte",
  "if (true)\n{\n    post(\"A\")\n}\n", "A", NULL, 0 },
{ "if/else com chave na linha seguinte",
  "x = 2\n"
  "if (x == 1)\n"
  "{\n"
  "    post(\"A\")\n"
  "} else\n"
  "{\n"
  "    post(\"B\")\n"
  "}\n", "B", NULL, 0 },
{ "while com chave na linha seguinte",
  "n = 0\nwhile (n < 2)\n{\n    n = n + 1\n}\npost(n)\n", "2", NULL, 0 },
{ "for each com chave na linha seguinte",
  "for each i in [1,2]\n{\n    post(i)\n}\n", "1\n2", NULL, 0 },
{ "try/catch com chave na linha seguinte",
  "try\n"
  "{\n"
  "    raise B(\"x\")\n"
  "} catch (e)\n"
  "{\n"
  "    post(\"peguei\")\n"
  "}\n", "peguei", NULL, 0 },
{ "action com chave na linha seguinte continua valendo",
  "action f()\n{\n    return 7\n}\npost(f())\n", "7", NULL, 0 },
{ "chave na linha seguinte nao estraga bloco de dois-pontos",
  "if true {\n"
  "    post(\"A\")\n"
  "}\n", "A", NULL, 0 },
/* ── compreensão de lista ───────────────────────────────────────────────────
 * `[<expr> for each <v> in <it> (if <cond>)?]` — a forma do Python, escrita
 * com o `for each` que a linguagem já tem. O acumulador vive num nome
 * escondido e numerado, pra compreensão aninhada não pisar na de fora. */
{ "compreensao de lista",
  "nums = [1,2,3,4]\npost([n * 2 for each n in nums])\n", "[2, 4, 6, 8]", NULL, 0 },
{ "compreensao com filtro",
  "post([x for each x in [1,2,3,4] if x % 2 == 0])\n", "[2, 4]", NULL, 0 },
{ "compreensao sobre string",
  "post([c.upper() for each c in \"abc\"])\n", "['A', 'B', 'C']", NULL, 0 },
{ "compreensao sobre range",
  "post([i * i for each i in range(5)])\n", "[0, 1, 4, 9, 16]", NULL, 0 },
{ "compreensao aninhada",
  "post([[y for each y in range(2)] for each z in range(3)])\n",
  "[[0, 1], [0, 1], [0, 1]]", NULL, 0 },
{ "compreensao sobre vazio",
  "post([1 for each q in []])\n", "[]", NULL, 0 },
{ "variavel da compreensao SOMBREIA, nao destroi",
  "n = \"de fora\"\npost([n for each n in [1,2]], n)\n", "[1, 2] de fora", NULL, 0 },
{ "compreensao dentro de action",
  "action f(l) {\n    return [v + 1 for each v in l]\n}\npost(f([1,2]))\n", "[2, 3]", NULL, 0 },
{ "lista comum continua igual",
  "post([1, 2, 3], [], [1])\n", "[1, 2, 3] [] [1]", NULL, 0 },
/* Sem os colchetes, como argumento ÚNICO de uma chamada — a forma do Python.
 * Vale em qualquer chamada, não só no `post`. */
{ "compreensao como argumento de chamada",
  "nums = [1,2,3,4,5]\npost(n * 1 for each n in nums)\n", "[1, 2, 3, 4, 5]", NULL, 0 },
{ "compreensao como argumento, com filtro",
  "post(n * 2 for each n in [1,2,3,4,5] if n > 2)\n", "[6, 8, 10]", NULL, 0 },
{ "compreensao dentro de sum/len/max",
  "n = [1,2,3]\npost(sum(v for each v in n), len(c for each c in \"abc\"), max(v for each v in n))\n",
  "6 3 3", NULL, 0 },
{ "compreensao com mais de um argumento e recusada com o conserto",
  "post(\"a\", n for each n in [1])\n", "", "ponha entre colchetes", 2 },
{ "com os colchetes, mais de um argumento vale",
  "post(\"a\", [n for each n in [1]])\n", "a [1]", NULL, 0 },
{ "compreensao sem 'each' e erro claro",
  "post([n for n in [1]])\n", "", "esperado 'each'", 2 },
{ "compreensao sem 'in' e erro claro",
  "post([n for each n [1]])\n", "", "esperado 'in'", 2 },

/* ── o bloco `:` saiu da linguagem ──────────────────────────────────────────
 * DECISÃO: o bloco é `{ }`, e só. Conviver com os dois custou caro — as
 * regressões de parser desta linha do tempo saíram todas da interação entre
 * indentação e chave. A mensagem tem que ENSINAR, não só recusar. */
{ "bloco com ':' e recusado",
  "if true:\n    post(\"A\")\n", "", "bloco com ':' nao existe mais", 2 },
{ "bloco com ':' numa linha so tambem e recusado",
  "if true: post(\"A\")\n", "", "bloco com ':' nao existe mais", 2 },
{ "Entity com ':' e recusada",
  "Entity A():\n    action m(self) { return 1 }\n", "", "bloco com ':' nao existe mais", 2 },
{ "a mensagem diz o que usar no lugar",
  "while true:\n    break\n", "", "use '{ }'", 2 },
{ "dicionario com ':' continua valendo",
  "d = { \"a\": 1, \"b\": 2 }\npost(d[\"a\"], d[\"b\"])\n", "1 2", NULL, 0 },
{ "fatia com ':' continua valendo",
  "post(\"abcdef\"[1:3], [1,2,3][0:2])\n", "bc [1, 2]", NULL, 0 },
{ "campo tipado de Entity com ':' continua valendo",
  "Entity P() {\n    nome: str\n}\np = P(\"ana\")\npost(p.nome)\n", "ana", NULL, 0 },

/* ── closure DENTRO de módulo importado ─────────────────────────────────────
 * O índice de proto do `OP_MAKE_CLOSURE` e o índice de global do
 * `OP_CELL_GET_NAME`/`OP_CELL_SET_NAME` não eram relocados ao anexar um
 * módulo: a action aninhada apontava pro protótipo de OUTRA função do
 * programa principal. `poe()` dentro de `um()` virava `um()` — recursão
 * infinita. Aqui o arquivo se importa, que é o caminho mais curto pra passar
 * pela relocação. */
{ "closure dentro de modulo importado",
  "import ps_mod_clo\n"
  "action com_closure() {\n"
  "    l = []\n"
  "    action poe(r) {\n"
  "        l.append(r)\n"
  "    }\n"
  "    poe(\"a\")\n"
  "    poe(\"b\")\n"
  "    return l\n"
  "}\n"
  "post(ps_mod_clo.com_closure())\n",
  "['a', 'b']\n['a', 'b']", NULL, 0, "ps_mod_clo.ps" },
{ "closure de modulo mantem estado proprio",
  "import ps_mod_cnt\n"
  "action contador() {\n"
  "    n = 0\n"
  "    action inc() {\n"
  "        n = n + 1\n"
  "        return n\n"
  "    }\n"
  "    return inc\n"
  "}\n"
  "c = ps_mod_cnt.contador()\n"
  "post(c(), c(), c())\n",
  "1 2 3\n1 2 3", NULL, 0, "ps_mod_cnt.ps" },

/* ── posição do fonte: nada aninhado vaza pra quem o contém ─────────────────
 * O interior de uma f-string é re-lexado a partir de uma string isolada, então
 * lá tudo é linha 1. Como o compilador guardava a "linha atual" numa variável
 * global que ninguém restaurava, esse 1 escapava e era gravado nas instruções
 * emitidas DEPOIS — `raise` na linha 3 aparecia como linha 1 no traceback.
 * Um erro de posição de uma construção alcançando outra é o defeito; estes
 * casos travam as duas pontas. */
{ "f-string nao muda a linha do statement que a contem",
  "action f() {\n"
  "    e = \"x\"\n"
  "    raise Boom(f\"erro: {e}\")\n"
  "}\n"
  "f()\n", "", "linha 3", 1 },
{ "erro DENTRO da f-string aponta a linha da f-string",
  "action f() {\n"
  "    x = 0\n"
  "\n"
  "    post(f\"v: {1 / x}\")\n"
  "}\n"
  "f()\n", "", "linha 4", 1 },
{ "f-string no meio nao desloca o que vem depois",
  "action f() {\n"
  "    e = 1\n"
  "    post(f\"a {e}\")\n"
  "    raise Boom(\"y\")\n"
  "}\n"
  "f()\n", "a 1", "linha 4", 1 },
{ "f-string continua interpolando",
  "n = 7\ns = \"ana\"\npost(f\"{s} tem {n}\", f\"{n * 2}\")\n", "ana tem 7 14", NULL, 0 },
{ "f-string aninhada em chamada aninhada",
  "action g(x) {\n    return x\n}\n"
  "v = 3\npost(g(f\"v={v}\"))\n", "v=3", NULL, 0 },

/* ── input(): fim da entrada é `null`, linha vazia é `""` ───────────────────
 * O runner roda todo caso com stdin em /dev/null, então aqui a entrada já
 * começa acabada. Sem o `null`, `while true: input()` giraria pra sempre
 * quando o outro lado fechasse o cano — foi o que travou o servidor LSP. */
{ "input() no fim da entrada devolve null",
  "post(input())\n", "Null", NULL, 0 },
{ "input() no fim é null, não string vazia",
  "post(input() == Null, input() == \"\")\n", "True False", NULL, 0 },
{ "input() em laço termina no fim da entrada",
  "n = 0\n"
  "while true {\n"
  "    l = input()\n"
  "    if l == Null {\n"
  "        break\n"
  "    }\n"
  "    n = n + 1\n"
  "}\n"
  "post(\"linhas:\", n)\n",
  "linhas: 0", NULL, 0 },

/* ── ponto de entrada: `if __name__ == "main"` ──────────────────────────────
 * Substituiu o `run_selfwith_`. É a forma do Python e é reconhecida pela
 * FORMA, não avaliando a condição: `__name__` vale o caminho do arquivo (é o
 * que se passa pro `Jinker`), então a comparação nunca daria verdadeiro.
 * É o único lugar onde `:` ainda abre bloco; `{ }` vale igual. */
{ "guard com dois-pontos",
  "if __name__ == \"main\":\n    post(\"direto\")\n", "direto", NULL, 0 },
{ "guard com chaves",
  "if __name__ == \"main\" {\n    post(\"direto\")\n}\n", "direto", NULL, 0 },
{ "guard com parenteses",
  "if (__name__ == \"main\") {\n    post(\"direto\")\n}\n", "direto", NULL, 0 },
/* I12: fechar o bloco na linha do ULTIMO comando era SyntaxError, enquanto
 * `{ x }` numa linha e o `}` sozinho na linha de baixo funcionavam. A regra da
 * chave mudava conforme a FORMA do bloco, e o erro ainda apontava a linha
 * SEGUINTE — porque quem estourava era um DEDENT solto, que carrega a posicao
 * do proximo token. */
{ "fecha o bloco na linha do ultimo comando: while",
  "i = 0\n"
  "while (i < 3) {\n"
  "    i = i + 1 }\n"
  "post(i)\n", "3", NULL, 0 },
{ "fecha o bloco na linha do ultimo comando: if",
  "x = 1\n"
  "if (x == 1) {\n"
  "    post(\"a\") }\n"
  "post(\"b\")\n", "a\nb", NULL, 0 },
{ "fecha o bloco na linha do ultimo comando: action",
  "action f() {\n"
  "    return 1 }\n"
  "post(f())\n", "1", NULL, 0 },
{ "fecha DOIS blocos na mesma linha",
  "x = 0\n"
  "if (x == 0) {\n"
  "    while (x < 2) {\n"
  "        x = x + 1 } }\n"
  "post(x)\n", "2", NULL, 0 },
/* O arquivo importa a SI MESMO: o corpo do módulo roda (imprime "corpo") mas
 * o guard dele NÃO — por isso "guard" aparece UMA vez só, no fim, quando o
 * arquivo roda como principal. */
{ "guard NAO roda quando o arquivo e importado",
  "import ps_guard\n"
  "post(\"corpo\")\n"
  "if __name__ == \"main\" {\n"
  "    post(\"guard\")\n"
  "}\n",
  "corpo\ncorpo\nguard", NULL, 0, "ps_guard.ps" },
{ "run_selfwith_ saiu, e a recusa ensina",
  "run_selfwith_(\"main\") {\n    post(1)\n}\n",
  "", "use: if __name__ == \"main\"", 2 },
{ "if normal continua sem aceitar dois-pontos",
  "if 1 == 1:\n    post(1)\n", "", "bloco com ':' nao existe mais", 2 },
{ "if com __name__ mas comparando outra coisa e if normal",
  "x = 1\nif __name__ == x {\n    post(\"nao\")\n} else {\n    post(\"if normal\")\n}\n",
  "if normal", NULL, 0 },

/* ── `{` depois de string NAO interpola quando abre bloco ───────────────────
 * `case c if m == 'GET' {` lia o `{` como interpolação da string e o case
 * ficava sem corpo. Já valia pro `for each`; agora vale pro `if`, `while`,
 * `match` e a guarda do `case`. */
{ "case com guarda terminada em string",
  "m = \"GET\"\nmatch m {\n    case c if c == \"GET\" {\n        post(\"pegou\")\n    }\n}\n",
  "pegou", NULL, 0 },
{ "match com sujeito terminado em string",
  "match \"a\" {\n    case \"a\" {\n        post(\"casou\")\n    }\n}\n", "casou", NULL, 0 },
{ "interpolacao de string continua valendo",
  "n = 7\npost(\"vale: \" {n})\n", "vale: 7", NULL, 0 },

/* ── import malformado: a mensagem tem que dizer O QUE falta ────────────────
 * `import jinker.` acontece o tempo todo: digita-se o ponto pra chamar o
 * completion do editor e o arquivo fica salvo assim. A mensagem antiga era
 * "esperado caminho de modulo" com o cursor no `import`, no começo da linha —
 * não dizia nada. */
{ "import com ponto solto no fim",
  "import jinker.\n", "", "faltou o nome do submodulo depois do '.'", 2 },
/* `route` é palavra reservada: exigir IDENT depois do ponto quebrava TODO
 * decorador cujo membro é keyword. A recusa só vale pro que não pode ser
 * nome de jeito nenhum (fim de linha, fim de arquivo). */
{ "decorador com membro que e palavra reservada",
  "@app.route(\"/x\")\naction h() { return 1 }\n", "", "name 'app' is not defined", 1 },
{ "import sem nome nenhum",
  "import\n", "", "esperado nome de modulo depois de 'import'", 2 },
{ "import valido continua valendo",
  "import sys\npost(type(sys))\n", "module", NULL, 0 },
{ "from ... import continua valendo",
  "from jinker import cors\npost(type(cors))\n", "CorsConfig", NULL, 0 },

/* ── módulo: o erro nomeia o membro e sugere o parecido ─────────────────── */
{ "membro inexistente nomeia modulo e membro",
  "import json\npost(json.naoexiste)\n",
  "", "module 'json' has no attribute 'naoexiste'", 1 },
{ "membro parecido vira sugestao",
  "import json\npost(json.parsee)\n", "", "Did you mean: 'parse'?", 1 },

/* ── tipo: `type(x)` é o NOME do tipo, `int` é a referência ─────────────────
 * Os dois escrevem "int" na tela. Antes disso, compará-los dava falso calado:
 * um `if type(x) == int` nunca entrava e ninguém era avisado. */
{ "type(x) compara igual a referencia de tipo",
  "post(type(200) == int)\n", "True", NULL, 0 },
{ "type(x) compara igual ao nome em texto",
  "post(type(200) == \"int\")\n", "True", NULL, 0 },
{ "tipo errado continua falso",
  "post(type(\"a\") == int)\n", "False", NULL, 0 },
{ "referencia de tipo dos dois lados",
  "post(int == type(200), type(1.5) == flo, type([1]) == list)\n",
  "True True True", NULL, 0 },
{ "referencia de tipo nao vira igual a texto qualquer",
  "post(int == \"inteiro\", int == 1, int == none)\n", "False False False", NULL, 0 },
{ "tipo com tipo continua por identidade",
  "post(int == int, int == str)\n", "True False", NULL, 0 },
{ "o idioma canonico continua o `is`",
  "post(200 is int, \"a\" is int, 200 not is str)\n", "True False True", NULL, 0 },
{ "o `if` que antes nunca entrava agora entra",
  "action f(x) {\n"
  "    if type(x) == int {\n"
  "        return \"inteiro\"\n"
  "    }\n"
  "    return \"outro\"\n"
  "}\n"
  "post(f(200), f(\"a\"))\n", "inteiro outro", NULL, 0 },

/* ── I/O que falha tem que AVISAR ───────────────────────────────────────────
 * `/dev/full` é o disco cheio do Linux: aceita o open e recusa toda gravação
 * com ENOSPC. Antes disso, os três casos abaixo terminavam com sucesso e o
 * dado sumia — o `fwrite` só enche o buffer da libc, e o erro só aparece no
 * flush, que é o `fclose`. */
{ "close() acusa o que nao conseguiu gravar",
  "f = open(\"/dev/full\", \"w\")\n"
  "f.write(\"abc\")\n"
  "f.close()\n"
  "post(\"NAO DEVIA CHEGAR\")\n", "", "OSError: [Errno 28] No space left on device", 1 },
{ "write() grande acusa gravacao incompleta",
  "f = open(\"/dev/full\", \"w\")\n"
  "f.write(\"x\" * 200000)\n", "", "[Errno 28]", 1 },
{ "writelines() acusa e diz qual linha",
  "f = open(\"/dev/full\", \"w\")\n"
  "f.writelines([\"x\" * 200000])\n", "", "OSError: [Errno 28] No space left on device", 1 },
{ "write() continua devolvendo quantos bytes gravou",
  "using open(\"/tmp/ps_t_w.txt\", \"w\") as f {\n"
  "    post(f.write(\"abcde\"), f.write(\"xy\"))\n"
  "}\n", "5 2", NULL, 0 },
{ "close() em arquivo bom nao levanta nada",
  "f = open(\"/tmp/ps_t_c.txt\", \"w\")\n"
  "f.write(\"ok\")\n"
  "f.close()\n"
  "f.close()\n"
  "post(open(\"/tmp/ps_t_c.txt\").read())\n", "ok", NULL, 0 },

/* ── o runner nao pode travar com muita saida ───────────────────────────────
 * Cada cano guarda 64 KB. O runner lia o stdout ATÉ O FIM e só então o stderr:
 * com mais de 64 KB no stderr o filho bloqueava escrevendo, o pai esperava um
 * stdout que não vinha, e o `alarm` relatava "TRAVOU" — uma falha inventada.
 * Este caso despeja ~200 KB no stderr; se o runner regredir, ele trava. */
{ "stderr maior que o cano nao trava o runner",
  "import sys\n"
  "for each i in range(4000) {\n"
  "    sys.stderr.write(\"linha de erro bem comprida pra encher o cano \" + str(i) + \"\\n\")\n"
  "}\n"
  "post(\"stdout chegou inteiro\")\n", "stdout chegou inteiro", NULL, 0 },

/* ── xlsx: escrever e ler de volta ──────────────────────────────────────────
 * A primeira linha de uma planilha NOVA era recusada com "colunas
 * insuficientes": o limite era calculado varrendo as linhas existentes, e numa
 * planilha vazia isso dava 1. Não havia ordem de chamadas que contornasse — o
 * arquivo saía com uma célula só e `mp.read` devolvia []. */
{ "xlsx ida e volta",
  "import manpu as mp\n"
  "using mp.open(target=\"p.xlsx\") as a {\n"
  "    a.write(column=\"full\", cell=\"full\", content=\"nome,idade\", sep=\",\")\n"
  "    a.write(column=\"full\", cell=\"full\", content=\"ana,30\", sep=\",\")\n"
  "}\n"
  "post(mp.read(\"p.xlsx\"))\n",
  "[{'nome': 'ana', 'idade': '30'}]", NULL, 0 },
{ "xlsx: primeira linha define a largura",
  "import manpu as mp\n"
  "using mp.open(target=\"q.xlsx\") as a {\n"
  "    post(a.write(column=\"full\", cell=\"full\", content=\"a,b,c\", sep=\",\"))\n"
  "}\n", "Success", NULL, 0 },
{ "xlsx: linha mais larga que a planilha continua recusada",
  "import manpu as mp\n"
  "using mp.open(target=\"r.xlsx\") as a {\n"
  "    a.write(column=\"full\", cell=\"full\", content=\"a,b\", sep=\",\")\n"
  "    post(a.write(column=\"full\", cell=\"full\", content=\"1,2,3,4\", sep=\",\"))\n"
  "}\n", "Error: Arquivo xlsx tem colunas insuficientes", NULL, 0 },
/* `copy()` é método de PoolFile, que vem de `os.loadFile` num binário — não
 * do FileHandle do `open()`. O caminho inteiro do copy() nunca tinha sido
 * exercitado por caso nenhum da suíte (gcov: linha ##### na função). */
{ "PoolFile.copy() copia o conteudo",
  "import os\n"
  "using open(\"o.png\", \"wb\") as f { f.write(\"\\x89PNG\\r\\n\\x1a\\nDADOS\") }\n"
  "a = os.loadFile(\"o.png\")\n"
  "a.copy(\"d.png\")\n"
  "using open(\"d.png\", \"rb\") as f { post(len(f.read())) }\n", "14", NULL, 0 },

/* ── regex: recursao profunda vira ERRO, nao segfault ───────────────────────
 * O casador é recursivo e gasta um quadro de pilha C por caractere. O teto de
 * PASSOS (2 milhões) não protegia disso: a pilha de 8 MB acaba muito antes, e
 * o processo morria de SIGSEGV, sem mensagem. Achado escrevendo o semeador do
 * fuzzer em PoolScript — o `pool` inteiro caiu casando
 * `"((?:[^"\\]|\\.)*)"` contra um trecho de 29 mil caracteres. */
{ "regex profundo demais e erro, nao morte",
  "import regex\n"
  "alvo = \"x\" * 60000\n"
  "post(regex.findall(\"(?:a|(x))*\", alvo))\n", "", "backtracking demais", 1 },
{ "regex normal continua valendo",
  "import regex\n"
  "post(regex.findall(\"\\\\d+\", \"a1b22c333\"))\n"
  "post(regex.sub(\"(\\\\w)(\\\\d)\", \"\\\\2\\\\1\", \"a1 b2\"))\n"
  "post(regex.split(\"[,;]\", \"a,b;c\"))\n",
  "['1', '22', '333']\n1a 2b\n['a', 'b', 'c']", NULL, 0 },
{ "grupo repetido em texto medio ainda casa",
  "import regex\n"
  "post(len(regex.findall(\"(?:ab)+\", \"ab\" * 400)))\n", "1", NULL, 0 },

/* ── classe negada DENTRO de `[]`: `[\s\S]`, `[a\D]`, `[^\S]` ────────────────
 * O motor RECUSAVA isso com "classe negada (\D \W \S) dentro de [] nao
 * suportada". `[\s\S]` é o "qualquer coisa, inclusive \n" que todo mundo
 * escreve — o buraco apareceu escrevendo ferramenta EM PoolScript, que é onde
 * a linguagem deixa de ser hipótese.
 *
 * A negação não podia ser o flag da classe inteira: `[a\D]` não é
 * "não (a ou dígito)", é "a ou não-dígito". Agora ela é materializada na hora
 * da união. Os três esperados abaixo foram conferidos contra o `re` do Python. */
{ "[\\s\\S] casa tudo, inclusive quebra de linha",
  "import regex\n"
  "post(regex.sub(\"/\\\\*[\\\\s\\\\S]*?\\\\*/\", \"-\", \"a/* x\\ny */b\"))\n",
  "a-b", NULL, 0 },
{ "[a\\D] é uniao com o complemento, nao negacao do conjunto",
  "import regex\n"
  "post(regex.findall(\"[a\\\\D]+\", \"ab12cd\"))\n",
  "['ab', 'cd']", NULL, 0 },
{ "[^\\S] é a dupla negacao — volta a ser \\s",
  "import regex\n"
  "post(regex.findall(\"[^\\\\S]\", \"a b\"))\n",
  "[' ']", NULL, 0 },
{ "[\\w\\S] e [\\d\\W] continuam unindo certo",
  "import regex\n"
  "post(regex.findall(\"[\\\\d\\\\W]+\", \"ab 12 cd\"))\n"
  "post(len(regex.findall(\"[\\\\w\\\\S]\", \"ab!\")))\n",
  "[' 12 ']\n3", NULL, 0 },
{ "classe negada nao vaza pro proximo item da classe",
  /* `[\Dx]` tem que ser o mesmo conjunto que `[x\D]`: a ordem não pode importar */
  "import regex\n"
  "post(regex.findall(\"[\\\\D5]+\", \"a1b5c\"))\n"
  "post(regex.findall(\"[5\\\\D]+\", \"a1b5c\"))\n",
  "['a', 'b5c']\n['a', 'b5c']", NULL, 0 },

/* ── JWT: header grande nao pode invalidar token bom ────────────────────────
 * O header era decodificado num `unsigned char hdr[256]` fixo, e header de 256
 * bytes e' rotina: `kid`, `jku` e `x5c` sao campos normais de emissor de
 * verdade. O decode nao cabia, devolvia <= 0, e o token VALIDO era recusado.
 * Medido no binario de 21/08: `kid` de 219 passava, de 220 devolvia null.
 * Os tokens abaixo sao HS256 de verdade, assinados fora do motor. */
{ "JWT com kid de 220 (header 342B) e aceito",
  "import jwt\n"
  "post(jwt.check(\"eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCIsImtpZCI6Imtra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2sifQ.eyJzdWIiOiJhbmEifQ.I39z_GkxpTt460dKPku7BQOjRAKbxYGVbKZtKtbzGwM\", \"segredo\"))\n",
  "{'sub': 'ana'}", NULL, 0 },
{ "JWT com kid de 3000 (header 4KB) e aceito",
  "import jwt\n"
  "post(jwt.check(\"eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCIsImtpZCI6Imtra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2trayJ9.eyJzdWIiOiJhbmEifQ.bsGwhH1TUhrmZjtLmgh-09qguz8iP3F3esgVQszdQgU\", \"segredo\"))\n",
  "{'sub': 'ana'}", NULL, 0 },
/* O header maior NAO pode afrouxar a verificacao: */
{ "JWT com header grande e assinatura errada continua recusado",
  "import jwt\n"
  "post(jwt.check(\"eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCIsImtpZCI6Imtra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2sifQ.eyJzdWIiOiJhbmEifQ.GUXDhT7Cr4UIbhDLNYAtP9iqswpvr9pbmu7_uQRrUUg\", \"segredo\"))\n", "Null", NULL, 0 },
{ "JWT com header grande e alg none continua recusado",
  "import jwt\n"
  "post(jwt.check(\"eyJhbGciOiJub25lIiwidHlwIjoiSldUIiwia2lkIjoia2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2tra2trayJ9.eyJzdWIiOiJhbmEifQ.ol1xhouZIoiUzxMELnPs4OEtVdsQY3-FUbV_EjzCuz8\", \"segredo\"))\n", "Null", NULL, 0 },

/* ── base64: padding invalido e ERRO, nao dado parcial ──────────────────────
 * O laco parava no primeiro `=` e devolvia o que tinha decodificado ate ali.
 * `Zg=`, `Zg===`, `=Zm9v` e `Zm==9v` entregavam dado parcial como se fossem
 * validos. O padding continua OPCIONAL — base64url de JWT nao tem — mas
 * quando existe tem que estar no fim e na quantidade exata. */
{ "base64 valido continua decodificando",
  "import hash\n"
  "post(hash.b64decode(\"Zg==\"), hash.b64decode(\"Zm9v\"), hash.b64decode(\"Zm9vYg==\"))\n",
  "f foo foob", NULL, 0 },
{ "base64url SEM padding continua valendo (e o do JWT)",
  "import hash\n"
  "post(hash.b64decode(\"Zg\"), hash.b64decode(\"Zm9\"), hash.b64decode(\"Zm9vYmFy\"))\n",
  "f fo foobar", NULL, 0 },
{ "padding curto demais e erro",
  "import hash\npost(hash.b64decode(\"Zg=\"))\n", "", "ValueError: Incorrect padding", 1 },
{ "padding demais e erro",
  "import hash\npost(hash.b64decode(\"Zg===\"))\n", "", "ValueError: Incorrect padding", 1 },
{ "padding no comeco e erro",
  "import hash\npost(hash.b64decode(\"=Zm9v\"))\n", "", "ValueError: Excess data after padding", 1 },
{ "dado depois do padding e erro",
  "import hash\npost(hash.b64decode(\"Zm==9v\"))\n", "", "ValueError: Excess data after padding", 1 },
{ "um caractere solto e erro (6 bits nao formam byte)",
  "import hash\npost(hash.b64decode(\"Z\"))\n", "", "base64", 1 },

/* ── mensagem de erro nao pode conter UTF-8 QUEBRADO ────────────────────────
 * O lexer imprimia o caractere com `%c`, ou seja o PRIMEIRO BYTE dele: `ç` é
 * 0xC3 0xA7 e saia so o 0xC3. Nao e cosmetico — o LSP serializa a mensagem em
 * JSON, e byte invalido quebra o JSON: o editor recusava a resposta
 * ("Expected ',' or '}' ... in JSON") e DERRUBAVA o servidor. Um acento fora
 * do lugar matava o suporte a editor inteiro. */
/* Os literais sao SEPARADOS de proposito: em C, `"\xc3\xa7ao"` faz o escape
 * hexadecimal engolir o `a` e o `o` como digitos, e o fonte do caso sai
 * corrompido. Literal adjacente encerra o escape. */
{ "caractere inesperado sai INTEIRO, nao meio byte",
  "x = fun" "\xc3\xa7" "ao(\n", "", "caractere inesperado: 'ç'", 2 },
{ "o --check devolve JSON valido com acento no erro",
  "x = \xc3\xa7\n", "", "'ç'", 2 },

/* ── `===` e `!==` foram REMOVIDOS (29/08) ──────────────────────────────────
 *
 * Eles nunca foram igualdade estrita: compilavam pro MESMO opcode do `==`, e
 * `false === Null` respondia igual a `false == Null`. Quem escrevia
 * `x === Null` acreditando estar protegido da comparação frouxa estava rodando
 * exatamente ela — nome de uma coisa, comportamento de outra. O dono usou isso
 * por meses achando que protegia.
 *
 * O lexer ainda RECONHECE os dois, de propósito: sem isso `a === b` viraria
 * `a == (= b)` e o erro falaria de outra coisa. */
{ "`===` é erro de sintaxe, dizendo o que usar",
  "post(1 === 1)\n", "", "`===` nao existe nesta linguagem; use `==`", 2 },
{ "`!==` é erro de sintaxe, dizendo o que usar",
  "post(1 !== 2)\n", "", "`!==` nao existe nesta linguagem; use `!=`", 2 },
{ "`==` e `!=` seguem valendo",
  "post(1 == 1, 1 != 2, \"a\" == \"a\", [1] == [1])\n",
  "True True True True", NULL, 0 },
{ "o erro do `===` aponta a coluna certa",
  /* o lexer casa o token de 3 chars inteiro, entao a coluna e a do operador */
  "x = 1\npost(x === 1)\n", "", "nao existe nesta linguagem", 2 },

/* ── CLI ── */
{ "--check não executa o script",
  "post(\"NAO DEVIA RODAR\")\n", "NAO DEVIA RODAR", NULL, 0 },
};
const int NC_LINGUAGEM = N_CASOS(CASOS_LINGUAGEM);
