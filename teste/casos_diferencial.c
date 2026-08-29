/*
 * DIFERENCIAL — antes x depois, o corpus inteiro.
 *
 * Sem um segundo motor pra comparar, o diferencial da linguagem é o
 * comportamento dela CONTRA ELA MESMA no tempo: cada programa aqui tem a
 * saída (ou a primeira linha do erro) que a VM produzia quando o caso foi
 * gravado. Qualquer mudança de comportamento — em qualquer canto — acende
 * aqui, e aí se decide se foi correção ou estrago.
 *
 * Os programas vêm do corpus de testes do repositório (commit f90845d).
 * Ficam de fora os não reprodutíveis: `id()` (endereço), relógio, rede,
 * banco, stdin e aleatório.
 *
 * NÃO edite à mão. Um caso que muda é uma DECISÃO: ou o motor regrediu, ou
 * o comportamento novo é o certo e a expectativa é que deve ser regravada.
 */
#include "ps_teste.h"

const Caso CASOS_DIFERENCIAL[] = {
{ "dif #0",
  "\n"
  "        i += 1\n"
  "    }\n"
  "    return a + \"_\" + \"SEGUNDA\"\n"
  "}\n"
  "post(g())\n"
  "\n",
  NULL, "SyntaxError: indentacao avancou 8 espacos; esperado exatamente 4", -1 },
{ "dif #1",
  "\n"
  "        i += 1\n"
  "    }\n"
  "    return marca + \"_intacto\"\n"
  "}\n"
  "post(trabalha(\"LOCAL\"))\n"
  "\n",
  NULL, "SyntaxError: indentacao avancou 8 espacos; esperado exatamente 4", -1 },
{ "dif #2",
  "\n"
  " i += 1\n"
  "}\n"
  "post(\"CONSTANTE\")\n"
  "\n",
  NULL, "SyntaxError: indentacao deve ser multiplo de 4 espacos (achou 1)", -1 },
{ "dif #3",
  "\n"
  " i += 1\n"
  "}\n"
  "post(1)\n"
  "\n",
  NULL, "SyntaxError: indentacao deve ser multiplo de 4 espacos (achou 1)", -1 },
{ "dif #4",
  "\n"
  " i += 1\n"
  "}\n"
  "post(d[\"n\"][\"n\"][\"n\"][1][1][1][0])\n"
  "\n",
  NULL, "SyntaxError: indentacao deve ser multiplo de 4 espacos (achou 1)", -1 },
{ "dif #5",
  "\n"
  " i += 1\n"
  "}\n"
  "post(guardada)\n"
  "\n",
  NULL, "SyntaxError: indentacao deve ser multiplo de 4 espacos (achou 1)", -1 },
{ "dif #6",
  "\n"
  " i += 1\n"
  "}\n"
  "post(viva[2])\n"
  "\n",
  NULL, "SyntaxError: indentacao deve ser multiplo de 4 espacos (achou 1)", -1 },
{ "dif #7",
  "\n"
  " i += 1\n"
  "}\n"
  "post(vivo[\"chave\"])\n"
  "\n",
  NULL, "SyntaxError: indentacao deve ser multiplo de 4 espacos (achou 1)", -1 },
{ "dif #8",
  "\n"
  "Entity Animal() {\n"
  "    action __init__(self, nome) {\n"
  "        self.nome = nome\n"
  "    }\n"
  "    action falar(self) {\n"
  "        return \"...\"\n"
  "    }\n"
  "}\n"
  "Entity Cachorro(Animal) {\n"
  "    action __init__(self, nome) {\n"
  "        base(nome)\n"
  "    }\n"
  "    action falar(self) {\n"
  "        return \"Au!\"\n"
  "    }\n"
  "}\n"
  "c = Cachorro(\"Rex\")\n"
  "post(c.nome)\n"
  "post(c.falar())\n"
  "\n",
  "Rex\nAu!", NULL, 0 },
{ "dif #9",
  "\n"
  "Entity C() {\n"
  "    action __init__(self) {\n"
  "        self.v = 10\n"
  "    }\n"
  "    reaction dobro(self) {\n"
  "        return self.v * 2\n"
  "    }\n"
  "}\n"
  "c = C()\n"
  "post(c.dobro())\n"
  "\n",
  "20", NULL, 0 },
{ "dif #10",
  "\n"
  "Entity C() {\n"
  "    action __init__(self, n) {\n"
  "        self.n = n\n"
  "    }\n"
  "    async action fetch(self) {\n"
  "        return self.n * 10\n"
  "    }\n"
  "}\n"
  "c = C(5)\n"
  "post(await c.fetch())\n"
  "\n",
  "50", NULL, 0 },
{ "dif #11",
  "\n"
  "Entity U() {\n"
  "    @static\n"
  "    action dobrar(n) {\n"
  "        return n * 2\n"
  "    }\n"
  "}\n"
  "post(U.dobrar(5))\n"
  "\n",
  "10", NULL, 0 },
{ "dif #12",
  "\n"
  "a = 10\n"
  "b = 20\n"
  "items = [10, 30]\n"
  "if (a < b and b > a and a <= 10 and b >= 20 and a in items and b not in items and a is int and b is not None) {\n"
  "    post(\"all-comparisons-ok\")\n"
  "}\n"
  "\n",
  "all-comparisons-ok", NULL, 0 },
{ "dif #13",
  "\n"
  "action contar(n) {\n"
  "    i = 0\n"
  "    while i < n {\n"
  "        yield i\n"
  "        i += 1\n"
  "    }\n"
  "}\n"
  "resultado = []\n"
  "for each v in contar(3) {\n"
  "    addEnd(resultado, v)\n"
  "}\n"
  "post(resultado)\n"
  "\n",
  "[0, 1, 2]", NULL, 0 },
{ "dif #14",
  "\n"
  "action f(x) {\n"
  "    // comentário na primeira linha do bloco\n"
  "    return x + 1\n"
  "}\n"
  "\n"
  "action g(x) {\n"
  "\n"
  "    return x * 2\n"
  "}\n"
  "\n"
  "Entity E() {\n"
  "    // comentário\n"
  "    action __init__(self, v) {\n"
  "        // outro\n"
  "        self.v = v\n"
  "    }\n"
  "    action dobro(self) {\n"
  "        return self.v * 2\n"
  "    }\n"
  "}\n"
  "\n"
  "n = 3\n"
  "if n > 2 {\n"
  "    // só comentário aqui\n"
  "    post(\"maior\")\n"
  "}\n"
  "for each i in range(2) {\n"
  "    # comentário com cerquilha\n"
  "    post(i)\n"
  "}\n"
  "post(f(1), g(2), E(5).dobro())\n"
  "\n",
  "maior\n0\n1\n2 4 10", NULL, 0 },
{ "dif #15",
  "\n"
  "action f1() { return Null }\n"
  "action f2() { return null }\n"
  "action f3() { return None }\n"
  "action f4() { return none }\n"
  "post(\"v1:\" f1())\n"
  "post(\"v2:\" f2())\n"
  "post(\"v3:\" f3())\n"
  "post(\"v4:\" f4())\n"
  "\n",
  "v1: null\nv2: null\nv3: null\nv4: null", NULL, 0 },
{ "dif #16",
  "\n"
  "async action t(n) { return n * 2 }\n"
  "fs = [t(1), t(2), t(3)]\n"
  "rs = await fs\n"
  "post(sorted(rs))\n"
  "\n",
  "[2, 4, 6]", NULL, 0 },
{ "dif #17",
  "\n"
  "class Animal() {\n"
  "    action __init__(self, nome) {\n"
  "        self.nome = nome\n"
  "    }\n"
  "    action falar(self) {\n"
  "        return \"...\"\n"
  "    }\n"
  "}\n"
  "Entity Gato(Animal) {\n"
  "    action falar(self) {\n"
  "        return f\"{self.nome}: miau\"\n"
  "    }\n"
  "}\n"
  "Class Cao(Animal) {\n"
  "    action falar(self) {\n"
  "        return f\"{self.nome}: au\"\n"
  "    }\n"
  "}\n"
  "g = Gato(\"Felix\")\n"
  "c = Cao(\"Rex\")\n"
  "post(g.falar())\n"
  "post(c.falar())\n"
  "\n",
  "Felix: miau\nRex: au", NULL, 0 },
{ "dif #18",
  "\n"
  "d = {\"a\": 1, \"b\": 2}\n"
  "s = \"abc\"\n"
  "l = [1, 2, 3, 4]\n"
  "t = (1, 2, 3)\n"
  "post(d.len())\n"
  "post(s.len())\n"
  "post(l.len())\n"
  "post(t.len())\n"
  "\n",
  "2\n3\n4\n3", NULL, 0 },
{ "dif #19",
  "\n"
  "d = {\"a\": 1, \"b\": 2}\n"
  "s = \"poolscript\"\n"
  "l = [1, 2, 3]\n"
  "t = (1, 2, 3)\n"
  "post(d.contains(\"a\"))\n"
  "post(s.contains(\"pool\"))\n"
  "post(l.contains(2))\n"
  "post(t.contains(2))\n"
  "post(d.has(\"x\"))\n"
  "post(s.has(\"java\"))\n"
  "post(l.has(9))\n"
  "post(t.has(9))\n"
  "\n",
  "True\nTrue\nTrue\nTrue\nFalse\nFalse\nFalse\nFalse", NULL, 0 },
{ "dif #20",
  "\n"
  "from datasentity import dataentity\n"
  "@dataentity\n"
  "Entity P() {\n"
  "    nome: str\n"
  "    action upper(self) {\n"
  "        return self.nome.upper()\n"
  "    }\n"
  "}\n"
  "p = P(nome=\"kleber\")\n"
  "post(p.upper())\n"
  "\n",
  "KLEBER", NULL, 0 },
{ "dif #21",
  "\n"
  "from datasentity import dataentity, asdict\n"
  "@dataentity\n"
  "Entity P() {\n"
  "    nome: str\n"
  "    idade: int\n"
  "}\n"
  "p = P(nome=\"Kleber\", idade=17)\n"
  "post(p.nome)\n"
  "post(p.idade)\n"
  "\n",
  "Kleber\n17", NULL, 0 },
{ "dif #22",
  "\n"
  "from datasentity import dataentity, asdict\n"
  "@dataentity\n"
  "Entity P() {\n"
  "    x: int\n"
  "    y: int\n"
  "}\n"
  "p = P(x=1, y=2)\n"
  "d = asdict(p)\n"
  "post(d[\"x\"])\n"
  "\n",
  "1", NULL, 0 },
{ "dif #23",
  "\n"
  "from datasentity import dataentity, asjson\n"
  "import json as _j\n"
  "@dataentity\n"
  "Entity P() {\n"
  "    nome: str\n"
  "}\n"
  "p = P(nome=\"Ana\")\n"
  "d = _j.parse(asjson(p))\n"
  "post(d[\"nome\"])\n"
  "\n",
  "Ana", NULL, 0 },
{ "dif #24",
  "\n"
  "from datasentity import dataentity, aslist\n"
  "@dataentity\n"
  "Entity P() {\n"
  "    x: int\n"
  "}\n"
  "p = P(x=5)\n"
  "l = aslist(p)\n"
  "post(l[0])\n"
  "\n",
  "5", NULL, 0 },
{ "dif #25",
  "\n"
  "from datasentity import dataentity, astuple\n"
  "@dataentity\n"
  "Entity P() {\n"
  "    a: str\n"
  "    b: int\n"
  "}\n"
  "p = P(a=\"ok\", b=9)\n"
  "t = astuple(p)\n"
  "post(t[0])\n"
  "\n",
  "ok", NULL, 0 },
{ "dif #26",
  "\n"
  "from os import loadFile\n"
  "\n"
  "texto = \"so um texto\"\n"
  "arq = loadFile(r\"\n"
  "\n",
  NULL, "SyntaxError: string nao fechada antes da quebra de linha", -1 },
{ "dif #27",
  "\n"
  "if (n > 10) {\n"
  " post(1)\n"
  "} elif (n > 5) {\n"
  " post(2)\n"
  "} else {\n"
  " post(3)\n"
  "}\n"
  "\n",
  NULL, "NameError: name 'n' is not defined", -1 },
{ "dif #28",
  "\n"
  "if (x == 1) {\n"
  "    post(\"a\")\n"
  "} elif x == 2 {\n"
  "    post(\"b\")\n"
  "} else {\n"
  "    post(\"c\")\n"
  "}\n"
  "\n",
  NULL, "NameError: name 'x' is not defined", -1 },
{ "dif #29",
  "\n"
  "import json\n"
  "s = json.stringify({\"ok\": true})\n"
  "post(s)\n"
  "d = json.parse(s)\n"
  "post(d[\"ok\"])\n"
  "x = 1\n"
  "post(type(x) == \"int\")\n"
  "\n",
  "{\"ok\": true}\nTrue\nTrue", NULL, 0 },
{ "dif #30",
  "\n"
  "int n = 17717\n"
  "post(count int(7) in n)\n"
  "\n",
  "3", NULL, 0 },
{ "dif #31",
  "\n"
  "json d = {\"a\": 1, \"b\": \"x\", \"c\": 1, \"d\": 2}\n"
  "post(count int(1) in d)\n"
  "post(count str in d)\n"
  "\n",
  "2\n1", NULL, 0 },
{ "dif #32",
  "\n"
  "l = [1, 2, 3]\n"
  "d = {\"a\": 1}\n"
  "// forma Python\n"
  "post(2 in l)\n"
  "post(\"a\" in d)\n"
  "post(len(l))\n"
  "// forma método (atalho)\n"
  "post(l.contains(2))\n"
  "post(d.has(\"a\"))\n"
  "post(l.len())\n"
  "\n",
  "True\nTrue\n3\nTrue\nTrue\n3", NULL, 0 },
{ "dif #33",
  "\n"
  "l = [3, 1, 2]\n"
  "post(l.count(1))\n"
  "post(l.index(2))\n"
  "\n",
  "1\n2", NULL, 0 },
{ "dif #34",
  "\n"
  "list a = [1, 1]\n"
  "list b = [3]\n"
  "if (count int(1) in a and count int(3) in b) {\n"
  "    post(\"both\")\n"
  "}\n"
  "\n",
  "both", NULL, 0 },
{ "dif #35",
  "\n"
  "list mix = [1, \"a\", 2, \"b\", 3]\n"
  "post(count int in mix)\n"
  "post(count str in mix)\n"
  "\n",
  "3\n2", NULL, 0 },
{ "dif #36",
  "\n"
  "list nums = [1, 7, 7, 2, 7]\n"
  "post(count int(7) in nums)\n"
  "\n",
  "3", NULL, 0 },
{ "dif #37",
  "\n"
  "list nums = [1, 7, 7, 2]\n"
  "post(int(7) count in nums)\n"
  "\n",
  "2", NULL, 0 },
{ "dif #38",
  "\n"
  "list xs = [1, 2, 3]\n"
  "if (count int(99) in xs) {\n"
  "    post(\"found\")\n"
  "} else {\n"
  "    post(\"none\")\n"
  "}\n"
  "\n",
  "none", NULL, 0 },
{ "dif #39",
  "\n"
  "list xs = [7, 7, 7, 8, 7]\n"
  "action total() {\n"
  "    count each int(7) in xs {\n"
  "        return;\n"
  "    }\n"
  "}\n"
  "post(total())\n"
  "\n",
  "4", NULL, 0 },
{ "dif #40",
  "\n"
  "list xs = [7, 7, 7]\n"
  "action firstHit() {\n"
  "    count each int(7) in xs {\n"
  "        return _index\n"
  "    }\n"
  "}\n"
  "post(firstHit())\n"
  "\n",
  "0", NULL, 0 },
{ "dif #41",
  "\n"
  "list xs = [7, 8, 7]\n"
  "count each int(7) in xs {\n"
  "    post(_match)\n"
  "}\n"
  "\n",
  "7\n7", NULL, 0 },
{ "dif #42",
  "\n"
  "list xs = [9, 9, 9, 1]\n"
  "if (count int(9) in xs == 3) { post(\"ok\") }\n"
  "\n",
  "ok", NULL, 0 },
{ "dif #43",
  "\n"
  "match 2 {\n"
  "    case 1 | 2 | 3 {\n"
  "        post(\"bateu\")\n"
  "    }\n"
  "    case _ {\n"
  "        post(\"nao bateu\")\n"
  "    }\n"
  "}\n"
  "\n",
  "bateu", NULL, 0 },
{ "dif #44",
  "\n"
  "match x {\n"
  " case 1 | 2 | 3 { post(\"a\") }\n"
  " case _ { post(\"b\") }\n"
  "}\n"
  "\n",
  NULL, "NameError: name 'x' is not defined", -1 },
{ "dif #45",
  "\n"
  "match x {\n"
  " case v if v > 5 { post(\"maior\") }\n"
  " case _ { post(\"menor\") }\n"
  "}\n"
  "\n",
  NULL, "NameError: name 'x' is not defined", -1 },
{ "dif #46",
  "\n"
  "post(\n"
  "\n",
  NULL, "SyntaxError: expressao invalida", -1 },
{ "dif #47",
  "\n"
  "post(x())\n"
  "\n",
  NULL, "NameError: name 'x' is not defined", -1 },
{ "dif #48",
  "\n"
  "post(x.\n"
  "\n",
  NULL, "SyntaxError: esperado nome do membro apos '.'", -1 },
{ "dif #49",
  "\n"
  "post(x[\n"
  "\n",
  NULL, "SyntaxError: expressao invalida", -1 },
{ "dif #50",
  "\n"
  "s = \"banana\"\n"
  "post(s.find(\"na\"), s.find(\"na\", 3), s.find(\"na\", 0, 3), s.find(\"na\", 99))\n"
  "post(s.rfind(\"na\"), s.rfind(\"na\", 0, 4), s.rfind(\"na\", 5))\n"
  "post(s.index(\"na\", 3), s.rindex(\"na\", 0, 4))\n"
  "post(s.count(\"na\"), s.count(\"na\", 3), s.count(\"a\", 1, 4), s.count(\"\", 1, 3))\n"
  "post(s.find(\"a\", -2), s.find(\"a\", -100), s.count(\"a\", -3))\n"
  "p = \"pão de mel\"\n"
  "post(p.find(\"o\"), p.find(\"e\", -3), p.rfind(\"e\", 0, 6), p.count(\"e\", 4))\n"
  "\n",
  "2 4 -1 -1\n4 2 -1\n4 2\n2 1 2 3\n5 1 2\n2 8 5 2", NULL, 0 },
{ "dif #51",
  "\n"
  "s = \"padrão: str\"\n"
  "post(s[5], s[6], s[-1])\n"
  "post(s[-3:len(s)])\n"
  "post(s[2:-2])\n"
  "post(s[0:6] == \"padrão\")\n"
  "post(s[::-1])\n"
  "post(s[1:10:2])\n"
  "post(len(s[3:len(s)]))\n"
  "t = \"regex.findall(padrão: str, texto: str, flags=0) -> list\"\n"
  "post(t[t.find(\"texto\"):len(t)])\n"
  "post(t[0:5] + \"|\" + t[14:20])\n"
  "post(\"ação\"[1], \"ação\"[-1], \"ação\"[1:3])\n"
  "\n",
  "o : r\nstr\ndrão: s\nTrue\nrts :oãrdap\naro t\n8\ntexto: str, flags=0) -> list\nregex|padrão\nç o çã", NULL, 0 },
{ "dif #52",
  "\n"
  "str s = \"ana banana\"\n"
  "post(count str(\"ana\") in s)\n"
  "\n",
  "3", NULL, 0 },
{ "dif #53",
  "\n"
  "x = \"123\"\n"
  "int y = 123\n"
  "if (x not is int and y is int and None is None) {\n"
  "    post(\"negative-path-ok\")\n"
  "}\n"
  "\n",
  "negative-path-ok", NULL, 0 },
{ "dif #54",
  "        post(\"ok:\", fn())\n"
  "\n",
  NULL, "NameError: name 'fn' is not defined", -1 },
{ "dif #55",
  "        post(\"preparando\", path)\n"
  "\n",
  NULL, "NameError: name 'path' is not defined", -1 },
{ "dif #56",
  "        post(\"registrou:\", fn())\n"
  "\n",
  NULL, "NameError: name 'fn' is not defined", -1 },
{ "dif #57",
  "        post(prev)\n"
  "\n",
  NULL, "NameError: name 'prev' is not defined", -1 },
{ "dif #58",
  "        post(self.\n"
  "\n",
  NULL, "SyntaxError: esperado nome do membro apos '.'", -1 },
{ "dif #59",
  "        post(self.)\n"
  "\n",
  NULL, "SyntaxError: esperado nome do membro apos '.'", -1 },
{ "dif #60",
  "    @static\n"
  "    action m(a) {\n"
  "        return a\n"
  "    }\n"
  "\n",
  NULL, "SyntaxError: expressao invalida", -1 },
{ "dif #61",
  "    @static\n"
  "    action m(self, a) {\n"
  "        return a\n"
  "    }\n"
  "\n",
  NULL, "SyntaxError: expressao invalida", -1 },
{ "dif #62",
  "    action __init__(self) {\n"
  "\n",
  NULL, "SyntaxError: bloco com '{' nao foi fechado com '}'", -1 },
{ "dif #63",
  "    action __init__(self) { self.x = 99 }\n"
  "\n",
  "", NULL, 0 },
{ "dif #64",
  "    action __init__(self):\n"
  "\n",
  NULL, "SyntaxError: bloco com ':' nao existe mais", -1 },
{ "dif #65",
  "    action __init__(self, v) { self.v = v }\n"
  "\n",
  "", NULL, 0 },
{ "dif #66",
  "    action close(self):\n"
  "\n",
  NULL, "SyntaxError: bloco com ':' nao existe mais", -1 },
{ "dif #67",
  "    action f():\n"
  "\n",
  NULL, "SyntaxError: bloco com ':' nao existe mais", -1 },
{ "dif #68",
  "    action faz(self, n) {\n"
  "\n",
  NULL, "SyntaxError: bloco com '{' nao foi fechado com '}'", -1 },
{ "dif #69",
  "    action m(self):\n"
  "\n",
  NULL, "SyntaxError: bloco com ':' nao existe mais", -1 },
{ "dif #70",
  "    action m(self, a):\n"
  "        return a\n"
  "\n",
  NULL, "SyntaxError: indentacao avancou 8 espacos; esperado exatamente 4", -1 },
{ "dif #71",
  "    action qualquerNome(self):\n"
  "\n",
  NULL, "SyntaxError: bloco com ':' nao existe mais", -1 },
{ "dif #72",
  "    action register(self, fn) { fn() }\n"
  "\n",
  "", NULL, 0 },
{ "dif #73",
  "    action register(self, fn):\n"
  "\n",
  NULL, "SyntaxError: bloco com ':' nao existe mais", -1 },
{ "dif #74",
  "    action rota(self, p) { self.p = p\n"
  "\n",
  NULL, "SyntaxError: bloco com '{' nao foi fechado com '}'", -1 },
{ "dif #75",
  "    action rota(self, path):\n"
  "\n",
  NULL, "SyntaxError: bloco com ':' nao existe mais", -1 },
{ "dif #76",
  "    action semself():\n"
  "\n",
  NULL, "SyntaxError: bloco com ':' nao existe mais", -1 },
{ "dif #77",
  "    action ver(self) {\n"
  "\n",
  NULL, "SyntaxError: bloco com '{' nao foi fechado com '}'", -1 },
{ "dif #78",
  "    action ver(self) { return self.v }\n"
  "\n",
  "", NULL, 0 },
{ "dif #79",
  "    count each int(7) in xs {\n"
  "\n",
  NULL, "SyntaxError: bloco com '{' nao foi fechado com '}'", -1 },
{ "dif #80",
  "    d = { \"a\": i, \"b\": [i, i + 1], \"s\": str(i), \"t\": (i, i) }\n"
  "\n",
  NULL, "NameError: name 'i' is not defined", -1 },
{ "dif #81",
  "    if i % 1000 == 0:\n"
  "\n",
  NULL, "SyntaxError: bloco com ':' nao existe mais", -1 },
{ "dif #82",
  "    if i == 2:\n"
  "\n",
  NULL, "SyntaxError: bloco com ':' nao existe mais", -1 },
{ "dif #83",
  "    if s == Cor.RED { return \"vermelho\" }\n"
  "\n",
  NULL, "NameError: name 's' is not defined", -1 },
{ "dif #84",
  "    if v > 0 { r = \"pos\" }\n"
  "\n",
  NULL, "NameError: name 'v' is not defined", -1 },
{ "dif #85",
  "    int reaction m(self, name=none) { return 1 }\n"
  "\n",
  "", NULL, 0 },
{ "dif #86",
  "    l = [d, d[\"b\"], \"x\" + str(i)]\n"
  "\n",
  NULL, "NameError: name 'd' is not defined", -1 },
{ "dif #87",
  "    post(\"ok\")\n"
  "\n",
  "ok", NULL, 0 },
{ "dif #88",
  "    raise Erro(\"x\")\n"
  "\n",
  NULL, "Erro: x", -1 },
{ "dif #89",
  "   post(\"interno\")\n"
  "\n",
  "interno", NULL, 0 },
{ "dif #90",
  "   post(1/0)\n"
  "\n",
  NULL, "ZeroDivisionError: division by zero", -1 },
{ "dif #91",
  "  post(\"IOError\")\n"
  "\n",
  "IOError", NULL, 0 },
{ "dif #92",
  "  post(\"externo\")\n"
  "\n",
  "externo", NULL, 0 },
{ "dif #93",
  "  post(\"pegou\")\n"
  "\n",
  "pegou", NULL, 0 },
{ "dif #94",
  "  post(1/0)\n"
  "\n",
  NULL, "ZeroDivisionError: division by zero", -1 },
{ "dif #95",
  "  post(os.run([\"prog_inexistente_zzz_123\"], true))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #96",
  "  post(z[\"boom\"])\n"
  "\n",
  NULL, "NameError: name 'z' is not defined", -1 },
{ "dif #97",
  "  try {\n"
  "\n",
  NULL, "SyntaxError: bloco com '{' nao foi fechado com '}'", -1 },
{ "dif #98",
  "  | str ruim = \n"
  "\n",
  NULL, "SyntaxError: expressao invalida", -1 },
{ "dif #99",
  " 3\n"
  "post(x)\n"
  "\n",
  NULL, "NameError: name 'x' is not defined", -1 },
{ "dif #100",
  " e) { post(\"pego\") }\n"
  "\n",
  NULL, "SyntaxError: expressao invalida", -1 },
{ "dif #101",
  " int x = 5.9\n"
  "\n",
  NULL, "AttributedValueError: variável x esperava int", -1 },
{ "dif #102",
  " post(\"DatabaseError: \" + e)\n"
  "\n",
  NULL, "NameError: name 'e' is not defined", -1 },
{ "dif #103",
  " post(\"caro\")\n"
  "\n",
  "caro", NULL, 0 },
{ "dif #104",
  " post(\"efeito\")\n"
  "\n",
  "efeito", NULL, 0 },
{ "dif #105",
  " post(\"params errados\")\n"
  "\n",
  "params errados", NULL, 0 },
{ "dif #106",
  " post(\"pego\")\n"
  "\n",
  "pego", NULL, 0 },
{ "dif #107",
  " post(\"pego: \" + e)\n"
  "\n",
  NULL, "NameError: name 'e' is not defined", -1 },
{ "dif #108",
  " post(1)\n"
  "\n",
  "1", NULL, 0 },
{ "dif #109",
  " post(_match)\n"
  "\n",
  NULL, "NameError: name '_match' is not defined", -1 },
{ "dif #110",
  " post(c)\n"
  "\n",
  NULL, "NameError: name 'c' is not defined", -1 },
{ "dif #111",
  " post(x)\n"
  "\n",
  NULL, "NameError: name 'x' is not defined", -1 },
{ "dif #112",
  " return \"n=\" + str(n)\n"
  "\n",
  NULL, "NameError: name 'n' is not defined", -1 },
{ "dif #113",
  " try {\n"
  "\n",
  NULL, "SyntaxError: bloco com '{' nao foi fechado com '}'", -1 },
{ "dif #114",
  " try { x = 1/0 } catch (KeyError e) { post(\"k\") }\n"
  "\n",
  NULL, "ZeroDivisionError: division by zero", -1 },
{ "dif #115",
  " while i < 3 {\n"
  "\n",
  NULL, "SyntaxError: bloco com '{' nao foi fechado com '}'", -1 },
{ "dif #116",
  " while i < n {\n"
  "\n",
  NULL, "SyntaxError: bloco com '{' nao foi fechado com '}'", -1 },
{ "dif #117",
  " } catch (e) { post(\"c\") }\n"
  "\n",
  NULL, "SyntaxError: expressao invalida", -1 },
{ "dif #118",
  " } catch (e) { post(\"catch de f\") }\n"
  "\n",
  NULL, "SyntaxError: expressao invalida", -1 },
{ "dif #119",
  "\")\n"
  "\n"
  "if (arq is PoolFile) { post(\"arquivo-e-poolfile\") }\n"
  "if (texto not is PoolFile) { post(\"texto-nao-e-poolfile\") }\n"
  "\n",
  NULL, "SyntaxError: string nao fechada antes da quebra de linha", -1 },
{ "dif #120",
  "\", \"r\") as f {\n"
  "    str x = f.read()\n"
  "    post(x)\n"
  "    int y = 1 + \"boom\"\n"
  "  }\n"
  "} catch (e) {\n"
  "  post(\"pegou:\" e)\n"
  "}\n"
  "\n",
  NULL, "SyntaxError: string nao fechada antes da quebra de linha", -1 },
{ "dif #121",
  "\", \"r\") as f {\n"
  "  str c = f.read()\n"
  "  post(c)\n"
  "}\n"
  "\n",
  NULL, "SyntaxError: string nao fechada antes da quebra de linha", -1 },
{ "dif #122",
  "\", \"w\") as f {\n"
  "  f.write(\"linha um\\n\")\n"
  "  f.write(\"linha dois\\n\")\n"
  "}\n"
  "using open(r\"\n"
  "\n",
  NULL, "SyntaxError: string nao fechada antes da quebra de linha", -1 },
{ "dif #123",
  "\"}})\n"
  "d = r.get_json()\n"
  "post(d[\"campos\"][\"model\"], d[\"campos\"][\"language\"], d[\"campos\"][\"n\"])\n"
  "post(d[\"arquivos\"][\"file\"])\n"
  "\n",
  NULL, "SyntaxError: string nao fechada antes da quebra de linha", -1 },
{ "dif #124",
  "#!cmd\n"
  "action main() {\n"
  "    post(\"oi\")\n"
  "}\n"
  "main()\n"
  "\n",
  "oi", NULL, 0 },
{ "dif #125",
  "#!cmd\n"
  "post(\"comando rodou\")\n"
  "\n",
  "comando rodou", NULL, 0 },
{ "dif #126",
  "#!lib\n"
  "action greet() {\n"
  "    return \"hi\"\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "dif #127",
  "#!lib\n"
  "action ola(n) { return \"oi \" + n }\n"
  "\n",
  "", NULL, 0 },
{ "dif #128",
  "(Literal bool \n"
  "\n",
  NULL, "SyntaxError: faltou ')'", -1 },
{ "dif #129",
  "(Literal flo \n"
  "\n",
  NULL, "SyntaxError: faltou ')'", -1 },
{ "dif #130",
  "(Literal int \n"
  "\n",
  NULL, "SyntaxError: faltou ')'", -1 },
{ "dif #131",
  "(Literal str \"\n"
  "\n",
  NULL, "SyntaxError: string nao fechada antes da quebra de linha", -1 },
{ "dif #132",
  "*inicio, z = [1, 2, 3, 4]\n"
  "post(inicio)\n"
  "post(z)\n"
  "\n",
  "[1, 2, 3]\n4", NULL, 0 },
{ "dif #133",
  "*r, b = [1,2,3]\n"
  "post(r)\n"
  "post(b)\n"
  "\n",
  "[1, 2]\n3", NULL, 0 },
{ "dif #134",
  "/up\",\n"
  "    fields={\"model\": \"whisper-large-v3\", \"language\": \"pt\", \"n\": 3},\n"
  "    file={\"file\": {\"name\": \"\n"
  "\n",
  NULL, "SyntaxError: string nao fechada antes da quebra de linha", -1 },
{ "dif #135",
  "/x\")\n"
  "post(g.status, len(g.content))\n"
  "\n",
  NULL, "SyntaxError: string nao fechada antes da quebra de linha", -1 },
{ "dif #136",
  "@NonNull\n"
  "action f(x) { return x }\n"
  "\n",
  "", NULL, 0 },
{ "dif #137",
  "@NonNull\n"
  "action g(a) {\n"
  " return a\n"
  "}\n"
  "post(g(1))\n"
  "\n",
  "1", NULL, 0 },
{ "dif #138",
  "@NonNull\n"
  "action g(a) {\n"
  " return a\n"
  "}\n"
  "post(g(Null))\n"
  "\n",
  NULL, "RuntimeError: @NonNull: parametro 'a' em 'g' nao pode ser Null", -1 },
{ "dif #139",
  "@NonNull\n"
  "action g(a, b) {\n"
  " return a\n"
  "}\n"
  "post(g(1, Null))\n"
  "\n",
  NULL, "RuntimeError: @NonNull: parametro 'b' em 'g' nao pode ser Null", -1 },
{ "dif #140",
  "@NonNull\n"
  "action g(a, b=2) {\n"
  " return a+b\n"
  "}\n"
  "post(g(1))\n"
  "\n",
  "3", NULL, 0 },
{ "dif #141",
  "@NonNull\n"
  "action g(a, b=Null) {\n"
  " return a\n"
  "}\n"
  "post(g(1))\n"
  "\n",
  NULL, "RuntimeError: @NonNull: parametro 'b' em 'g' nao pode ser Null", -1 },
{ "dif #142",
  "@NonNull\n"
  "action precisa(v) {\n"
  "    return v\n"
  "}\n"
  "post(precisa(5))\n"
  "\n",
  "5", NULL, 0 },
{ "dif #143",
  "@NonNull\n"
  "action precisa(v) {\n"
  "    return v\n"
  "}\n"
  "precisa(Null)\n"
  "\n",
  NULL, "RuntimeError: @NonNull: parametro 'v' em 'precisa' nao pode ser Null", -1 },
{ "dif #144",
  "@NonNull\n"
  "async action f() { return 1 }\n"
  "\n",
  "", NULL, 0 },
{ "dif #145",
  "@NonNull\n"
  "int action f() { return 1 }\n"
  "\n",
  "", NULL, 0 },
{ "dif #146",
  "@app\n"
  "action h() { return 1 }\n"
  "\n",
  "", NULL, 0 },
{ "dif #147",
  "@app.route(\"/x\")\n"
  "action h() { return 1 }\n"
  "\n",
  NULL, "NameError: name 'app' is not defined", -1 },
{ "dif #148",
  "@app.route(\"/x\", methods=[\"GET\"])\n"
  "action h() { return 1 }\n"
  "\n",
  NULL, "NameError: name 'app' is not defined", -1 },
{ "dif #149",
  "@dataentity\n"
  "Entity P() {\n"
  "    nome: str\n"
  "}\n"
  "p=P(nome=\"k\")\n"
  "post(p.nome)\n"
  "\n",
  "k", NULL, 0 },
{ "dif #150",
  "@server.route(\"/api\") {\n"
  " action handler() { return 1 }\n"
  "}\n"
  "\n",
  NULL, "NameError: name 'server' is not defined", -1 },
{ "dif #151",
  "@static\n"
  "action f() {\n"
  " return 1\n"
  "}\n"
  "post(f())\n"
  "\n",
  "1", NULL, 0 },
{ "dif #152",
  "@static\n"
  "action f() { return 1 }\n"
  "\n",
  "", NULL, 0 },
{ "dif #153",
  "Entity A():\n"
  "\n",
  NULL, "SyntaxError: bloco com ':' nao existe mais", -1 },
{ "dif #154",
  "Entity A() {\n"
  "    action __init__(self) {\n"
  "        base()\n"
  "    }\n"
  "}\n"
  "post(1)\n"
  "\n",
  NULL, "SyntaxError: base() numa Entity sem heranca: nao ha pai pra inicializar", -1 },
{ "dif #155",
  "Entity A() {\n"
  "    action __init__(self, n) {\n"
  "        self.n = n\n"
  "    }\n"
  "    action falar(self) {\n"
  "        return \"...\"\n"
  "    }\n"
  "}\n"
  "Entity M(A) {\n"
  "    action __init__(self, n) {\n"
  "        base(n)\n"
  "    }\n"
  "}\n"
  "Entity C(M) {\n"
  "    action __init__(self, n) {\n"
  "        base(n)\n"
  "    }\n"
  "    action falar(self) {\n"
  "        return \"Au!\"\n"
  "    }\n"
  "}\n"
  "c = C(\"Rex\")\n"
  "post(c.n)\n"
  "post(c.falar())\n"
  "\n",
  "Rex\nAu!", NULL, 0 },
{ "dif #156",
  "Entity A() {\n"
  "    action __init__(self, v) {\n"
  "        self.v = v\n"
  "    }\n"
  "}\n"
  "Entity B(A) {\n"
  "    action __init__(self, a, b) {\n"
  "        base(a + b)\n"
  "    }\n"
  "}\n"
  "post(B(3, 4).v)\n"
  "\n",
  "7", NULL, 0 },
{ "dif #157",
  "Entity A() {\n"
  "    action __init__(self, x) {\n"
  "        self.x = x\n"
  "    }\n"
  "}\n"
  "Entity B(A) {\n"
  "    action __init__(self, x, y) {\n"
  "        base(x)\n"
  "        self.y = y\n"
  "    }\n"
  "}\n"
  "b = B(1, 2)\n"
  "post(b.x)\n"
  "post(b.y)\n"
  "\n",
  "1\n2", NULL, 0 },
{ "dif #158",
  "Entity A() {\n"
  "    action f(self) {\n"
  "        return \"A\"\n"
  "    }\n"
  "}\n"
  "Entity B(A) {\n"
  "    action f(self) {\n"
  "        return \"B\"\n"
  "    }\n"
  "}\n"
  "post(A().f())\n"
  "post(B().f())\n"
  "\n",
  "A\nB", NULL, 0 },
{ "dif #159",
  "Entity A() {\n"
  "    action oi(self) {\n"
  "        return \"oi\"\n"
  "    }\n"
  "}\n"
  "Entity B(A) {\n"
  "    action __init__(self) {\n"
  "        base()\n"
  "        self.v = 5\n"
  "    }\n"
  "}\n"
  "b = B()\n"
  "post(b.oi())\n"
  "post(b.v)\n"
  "\n",
  "oi\n5", NULL, 0 },
{ "dif #160",
  "Entity A() {\n"
  "    action oi(self) {\n"
  "        return \"oi\"\n"
  "    }\n"
  "}\n"
  "Entity B(A) {\n"
  "    action __init__(self, v) {\n"
  "        self.v = v\n"
  "    }\n"
  "}\n"
  "b = B(1)\n"
  "post(b.oi())\n"
  "post(b.v)\n"
  "\n",
  "oi\n1", NULL, 0 },
{ "dif #161",
  "Entity A() {\n"
  "    action ver(self) {\n"
  "        return self.z\n"
  "    }\n"
  "}\n"
  "Entity B(A) {\n"
  "    action __init__(self) {\n"
  "        self.z = 11\n"
  "    }\n"
  "}\n"
  "post(B().ver())\n"
  "\n",
  "11", NULL, 0 },
{ "dif #162",
  "Entity A() {\n"
  "    x: int\n"
  "}\n"
  "Entity B(A) {\n"
  "    action __init__(self, x) {\n"
  "        base(x)\n"
  "    }\n"
  "}\n"
  "post(B(7).x)\n"
  "\n",
  "7", NULL, 0 },
{ "dif #163",
  "Entity Animal() {\n"
  "    action __init__(self, nome) {\n"
  "        self.nome = nome\n"
  "    }\n"
  "    action falar(self) {\n"
  "        return \"...\"\n"
  "    }\n"
  "}\n"
  "Entity Mamifero(Animal) {\n"
  "    action __init__(self, nome) {\n"
  "        base(nome)\n"
  "    }\n"
  "}\n"
  "Entity Cachorro(Mamifero) {\n"
  "    action __init__(self, nome) {\n"
  "        base(nome)\n"
  "    }\n"
  "    action falar(self) {\n"
  "        return \"Au!\"\n"
  "    }\n"
  "}\n"
  "c = Cachorro(\"Rex\")\n"
  "post(c.nome)\n"
  "post(c.falar())\n"
  "\n",
  "Rex\nAu!", NULL, 0 },
{ "dif #164",
  "Entity App():\n"
  "\n",
  NULL, "SyntaxError: bloco com ':' nao existe mais", -1 },
{ "dif #165",
  "Entity C() {\n"
  "  n: int = 10\n"
  "  public reaction add(self, v) { self.n += v  return self.n }\n"
  "  public reaction sub(self, v) { self.n -= v  return self.n }\n"
  "}\n"
  "c = C()\n"
  "post(c.add(5))\n"
  "post(c.sub(3))\n"
  "\n",
  "15\n12", NULL, 0 },
{ "dif #166",
  "Entity C() {\n"
  " @NonNull\n"
  " action s(self, v) { return v }\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "dif #167",
  "Entity Cliente():\n"
  "\n",
  NULL, "SyntaxError: bloco com ':' nao existe mais", -1 },
{ "dif #168",
  "Entity Conta() {\n"
  "\n",
  NULL, "SyntaxError: corpo da Entity nao foi fechado", -1 },
{ "dif #169",
  "Entity Conta() {\n"
  "    private saldo: int = 0\n"
  "    public dono: str = \"kleber\"\n"
  "    public reaction deposita(self, v) { self.saldo = self.saldo + v  return self.saldo }\n"
  "    private reaction _log(self) { return \"secreto\" }\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "dif #170",
  "Entity Conta() {\n"
  "    action __init__(self, saldo) {\n"
  "        self.saldo = saldo\n"
  "    }\n"
  "    async action depositar(self, valor) {\n"
  "        self.saldo = self.saldo + valor\n"
  "        return self.saldo\n"
  "    }\n"
  "}\n"
  "c = Conta(100)\n"
  "post(await c.depositar(50))\n"
  "post(c.saldo)\n"
  "\n",
  "150\n150", NULL, 0 },
{ "dif #171",
  "Entity Contador() {\n"
  "    action __init__(self, inicio) {\n"
  "        self.n = inicio\n"
  "    }\n"
  "    async action inc(self) {\n"
  "        self.n = self.n + 1\n"
  "        return self.n\n"
  "    }\n"
  "}\n"
  "a = Contador(0)\n"
  "b = Contador(100)\n"
  "post(await a.inc())\n"
  "post(await b.inc())\n"
  "\n",
  "1\n101", NULL, 0 },
{ "dif #172",
  "Entity Ctrl() {\n"
  "\n",
  NULL, "SyntaxError: corpo da Entity nao foi fechado", -1 },
{ "dif #173",
  "Entity F(A, B) {\n"
  " action m(self) { return 1 }\n"
  "}\n"
  "\n",
  NULL, "NameError: name 'A' is not defined", -1 },
{ "dif #174",
  "Entity F(P) {\n"
  "    action __init__(self, v) {\n"
  "        base(v)\n"
  "    }\n"
  "}\n"
  "\n",
  NULL, "NameError: name 'P' is not defined", -1 },
{ "dif #175",
  "Entity F(P) {\n"
  " action __init__(self) {\n"
  "  base()\n"
  " }\n"
  "}\n"
  "\n",
  NULL, "NameError: name 'P' is not defined", -1 },
{ "dif #176",
  "Entity F(P) {\n"
  " action __init__(self, v) {\n"
  "  base(v)\n"
  " }\n"
  "}\n"
  "\n",
  NULL, "NameError: name 'P' is not defined", -1 },
{ "dif #177",
  "Entity Filho(Pai) {\n"
  " action m(self) { return 1 }\n"
  "}\n"
  "\n",
  NULL, "NameError: name 'Pai' is not defined", -1 },
{ "dif #178",
  "Entity Foo() {\n"
  " action m(self) { return 1 }\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "dif #179",
  "Entity M():\n"
  "\n",
  NULL, "SyntaxError: bloco com ':' nao existe mais", -1 },
{ "dif #180",
  "Entity P() {\n"
  " @static\n"
  " action m() { return 1 }\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "dif #181",
  "Entity P() {\n"
  " nome: str\n"
  " idade: int = 0\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "dif #182",
  "Entity P() { v: int = 7 }\n"
  "p = P()\n"
  "post(p.v)\n"
  "\n",
  "7", NULL, 0 },
{ "dif #183",
  "Entity P() { valor: int = 7 }\n"
  "p = P()\n"
  "post(p.valor)\n"
  "\n",
  "7", NULL, 0 },
{ "dif #184",
  "Entity P():\n"
  "\n",
  NULL, "SyntaxError: bloco com ':' nao existe mais", -1 },
{ "dif #185",
  "Entity P() {\n"
  "    a: int\n"
  "    b: int\n"
  "}\n"
  "p=P(1, b=2)\n"
  "post(p.b)\n"
  "\n",
  "2", NULL, 0 },
{ "dif #186",
  "Entity P() {\n"
  "    a: int\n"
  "    b: int = 2\n"
  "}\n"
  "p=P(1)\n"
  "post(p.a)\n"
  "post(p.b)\n"
  "\n",
  "1\n2", NULL, 0 },
{ "dif #187",
  "Entity P() {\n"
  "    a: int\n"
  "    b: int = 9\n"
  "}\n"
  "p=P(a=1)\n"
  "post(p.b)\n"
  "\n",
  "9", NULL, 0 },
{ "dif #188",
  "Entity P() {\n"
  "    a: int\n"
  "}\n"
  "action g() {\n"
  " yield P(1)\n"
  " yield P(2)\n"
  "}\n"
  "for each p in g() {\n"
  " post(p.a)\n"
  "}\n"
  "\n",
  "1\n2", NULL, 0 },
{ "dif #189",
  "Entity P() {\n"
  "    a: int\n"
  "}\n"
  "p=P(z=1)\n"
  "post(p.a)\n"
  "\n",
  NULL, "TypeError: __init__() missing 1 required positional argument: 'a'", -1 },
{ "dif #190",
  "Entity P() {\n"
  "    a: int\n"
  "}\n"
  "p=P(z=1)\n"
  "post(p.z)\n"
  "\n",
  NULL, "TypeError: __init__() missing 1 required positional argument: 'a'", -1 },
{ "dif #191",
  "Entity P() {\n"
  "    action __init__(self) {\n"
  "        self.a = 1\n"
  "    }\n"
  "}\n"
  "post(P().b)\n"
  "\n",
  NULL, "AttributeError: 'P' object has no attribute 'b'", -1 },
{ "dif #192",
  "Entity P() {\n"
  "    action __init__(self) {\n"
  "        self.a=1\n"
  "    }\n"
  "}\n"
  "p=P()\n"
  "post(p.type())\n"
  "\n",
  "P", NULL, 0 },
{ "dif #193",
  "Entity P() {\n"
  "    action __init__(self) {\n"
  "        self.a=1\n"
  "    }\n"
  "}\n"
  "post(type(P()))\n"
  "\n",
  "P", NULL, 0 },
{ "dif #194",
  "Entity P() {\n"
  "    action __init__(self) {\n"
  "        self.l = [1,2]\n"
  "    }\n"
  "}\n"
  "p = P()\n"
  "p.l[0] = 9\n"
  "post(p.l[0])\n"
  "\n",
  "9", NULL, 0 },
{ "dif #195",
  "Entity P() {\n"
  "    action __init__(self) {\n"
  "        self.v = 2\n"
  "    }\n"
  "    action dobro(self) {\n"
  "        return self.v * 2\n"
  "    }\n"
  "    action quadruplo(self) {\n"
  "        return self.dobro() * 2\n"
  "    }\n"
  "}\n"
  "post(P().quadruplo())\n"
  "\n",
  "8", NULL, 0 },
{ "dif #196",
  "Entity P() {\n"
  "    action __init__(self, n) {\n"
  "        self.n = n\n"
  "    }\n"
  "    action soma(self, k) {\n"
  "        return self.n + k\n"
  "    }\n"
  "}\n"
  "post(P(7).soma(3))\n"
  "\n",
  "10", NULL, 0 },
{ "dif #197",
  "Entity P() {\n"
  "    action __init__(self, n) {\n"
  "        self.n = n\n"
  "    }\n"
  "}\n"
  "a = P(1)\n"
  "b = P(2)\n"
  "a.n = 50\n"
  "post(a.n)\n"
  "post(b.n)\n"
  "\n",
  "50\n2", NULL, 0 },
{ "dif #198",
  "Entity P() {\n"
  "    action __init__(self, n) {\n"
  "        self.n = n\n"
  "    }\n"
  "}\n"
  "l = [P(1), P(2)]\n"
  "for each p in l {\n"
  " post(p.n)\n"
  "}\n"
  "\n",
  "1\n2", NULL, 0 },
{ "dif #199",
  "Entity P() {\n"
  "    action __init__(self, n) {\n"
  "        self.n = n\n"
  "    }\n"
  "}\n"
  "p = P(1)\n"
  "p.n = 9\n"
  "post(p.n)\n"
  "\n",
  "9", NULL, 0 },
{ "dif #200",
  "Entity P() {\n"
  "    action __init__(self, n) {\n"
  "        self.n = n\n"
  "    }\n"
  "}\n"
  "post(P(7).n)\n"
  "\n",
  "7", NULL, 0 },
{ "dif #201",
  "Entity P() {\n"
  "    action __init__(self, x, y) {\n"
  "        self.x = x\n"
  "        self.y = y\n"
  "    }\n"
  "}\n"
  "p=P(1, y=2)\n"
  "post(p.y)\n"
  "\n",
  "2", NULL, 0 },
{ "dif #202",
  "Entity P() {\n"
  "    action m(self) {\n"
  "        return 1\n"
  "    }\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "dif #203",
  "Entity P() {\n"
  "    n: str\n"
  "    action __init__(self) {\n"
  "        self.n = \"proprio\"\n"
  "    }\n"
  "}\n"
  "post(P().n)\n"
  "\n",
  "proprio", NULL, 0 },
{ "dif #204",
  "Entity P() {\n"
  "    n: str\n"
  "    action oi(self) {\n"
  "        return self.n\n"
  "    }\n"
  "}\n"
  "post(P(\"z\").oi())\n"
  "\n",
  "z", NULL, 0 },
{ "dif #205",
  "Entity P() {\n"
  "    nome: str\n"
  "    idade: int\n"
  "}\n"
  "p=P(\"k\",1)\n"
  "post(p.nome)\n"
  "post(p.idade)\n"
  "\n",
  "k\n1", NULL, 0 },
{ "dif #206",
  "Entity P() {\n"
  "    nome: str\n"
  "    idade: int\n"
  "}\n"
  "p=P(nome=\"k\",idade=1)\n"
  "post(p.nome)\n"
  "\n",
  "k", NULL, 0 },
{ "dif #207",
  "Entity P() {\n"
  "    nome: str = \"x\"\n"
  "}\n"
  "p=P(\"y\")\n"
  "post(p.nome)\n"
  "\n",
  "y", NULL, 0 },
{ "dif #208",
  "Entity P() {\n"
  "    nome: str = \"x\"\n"
  "}\n"
  "p=P()\n"
  "post(p.nome)\n"
  "\n",
  "x", NULL, 0 },
{ "dif #209",
  "Entity Pessoa() {\n"
  " action __init__(self, nome) {\n"
  "  self.nome = nome\n"
  " }\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "dif #210",
  "Entity Ponto() {\n"
  "    action __init__(self, x) {\n"
  "        self.x = x\n"
  "    }\n"
  "}\n"
  "p = Ponto(5)\n"
  "post(p.x)\n"
  "\n",
  "5", NULL, 0 },
{ "dif #211",
  "Entity Ponto():\n"
  "\n",
  NULL, "SyntaxError: bloco com ':' nao existe mais", -1 },
{ "dif #212",
  "Entity R():\n"
  "\n",
  NULL, "SyntaxError: bloco com ':' nao existe mais", -1 },
{ "dif #213",
  "Entity Reg():\n"
  "\n",
  NULL, "SyntaxError: bloco com ':' nao existe mais", -1 },
{ "dif #214",
  "Entity T() {\n"
  "    action __init__(self) { self.x = 1 }\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "dif #215",
  "Entity U():\n"
  "\n",
  NULL, "SyntaxError: bloco com ':' nao existe mais", -1 },
{ "dif #216",
  "Entity U() {\n"
  "    @static\n"
  "    action tri(n) {\n"
  "        return n*3\n"
  "    }\n"
  "}\n"
  "post(U.tri(4))\n"
  "\n",
  "12", NULL, 0 },
{ "dif #217",
  "Entity Util() {\n"
  "    @static\n"
  "    action triplo(n) {\n"
  "        return n * 3\n"
  "    }\n"
  "}\n"
  "post(Util.triplo(4))\n"
  "\n",
  "12", NULL, 0 },
{ "dif #218",
  "Entity base() {\n"
  " action m(self) { return 1 }\n"
  "}\n"
  "\n",
  NULL, "SyntaxError: 'base' e palavra reservada da linguagem e nao pode ser usada como n", -1 },
{ "dif #219",
  "Entity gerador() {\n"
  "\n",
  NULL, "SyntaxError: corpo da Entity nao foi fechado", -1 },
{ "dif #220",
  "Entity tup() {\n"
  "\n",
  NULL, "SyntaxError: 'tup' e palavra reservada da linguagem e nao pode ser usada como no", -1 },
{ "dif #221",
  "PUSH os GET getenv\n"
  "post(getenv(\"POOL_TEST_VAR\"))\n"
  "\n",
  "null", NULL, 0 },
{ "dif #222",
  "PUSH os GET getenv as ler_env\n"
  "post(\"ok\")\n"
  "\n",
  "ok", NULL, 0 },
{ "dif #223",
  "a = \"x\"\n"
  "b = 1\n"
  "c = 1.5\n"
  "d = true\n"
  "if (a is str and b is int and c is flo and d is bool) {\n"
  "    post(\"todos_ok\")\n"
  "}\n"
  "\n",
  "todos_ok", NULL, 0 },
{ "dif #224",
  "a = 0\n"
  "b = 0\n"
  "if (True) {\n"
  "    a, b = 7, 8\n"
  "}\n"
  "post(a)\n"
  "post(b)\n"
  "\n",
  "7\n8", NULL, 0 },
{ "dif #225",
  "a = 1\n"
  "b = 2\n"
  "a, b = b, a\n"
  "post(a)\n"
  "post(b)\n"
  "\n",
  "2\n1", NULL, 0 },
{ "dif #226",
  "a = 1\n"
  "b = 2\n"
  "action f() {\n"
  " global a, b\n"
  " a = 10\n"
  " b = 20\n"
  "}\n"
  "f()\n"
  "post(a)\n"
  "post(b)\n"
  "\n",
  "10\n20", NULL, 0 },
{ "dif #227",
  "a = 1\n"
  "b = 2\n"
  "c = 3\n"
  "a, b, c = c, a, b\n"
  "post(a)\n"
  "post(b)\n"
  "post(c)\n"
  "\n",
  "3\n1\n2", NULL, 0 },
{ "dif #228",
  "a = 1\n"
  "b = 2\n"
  "post(\"a\" a \"b\" b)\n"
  "\n",
  "a 1 b 2", NULL, 0 },
{ "dif #229",
  "a = 1\n"
  "b = 2\n"
  "post(\"a\" {a} \"b\" {b} \"c\")\n"
  "\n",
  "a1b2c", NULL, 0 },
{ "dif #230",
  "a = 1\n"
  "b = 2\n"
  "post(f\"a={a} b={b}\")\n"
  "\n",
  "a=1 b=2", NULL, 0 },
{ "dif #231",
  "a = 2\n"
  "post(\"r=\" {a + 3})\n"
  "\n",
  "r=5", NULL, 0 },
{ "dif #232",
  "a = Null\n"
  "b = null\n"
  "c = None\n"
  "d = none\n"
  "post(a == b)\n"
  "post(b == c)\n"
  "post(c == d)\n"
  "\n",
  "True\nTrue\nTrue", NULL, 0 },
{ "dif #233",
  "a = [1, 3, 1]\n"
  "b = [3, 3]\n"
  "if (count int(1) in a and count int(3) in b == 1) {\n"
  "    post(\"complexo_ok\")\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "dif #234",
  "a = [1]\n"
  "b = [1]\n"
  "a.append(b)\n"
  "b.append(a)\n"
  "post(a == b)\n"
  "post(a.contains(b))\n"
  "\n",
  "False\nTrue", NULL, 0 },
{ "dif #235",
  "a = true\n"
  "b = false\n"
  "if (a and not b) { post(\"1\") }\n"
  "if (a or b) { post(\"2\") }\n"
  "if (not (a and b)) { post(\"3\") }\n"
  "if (not a and b) { post(\"nao\") } else { post(\"4\") }\n"
  "\n",
  "1\n2\n3\n4", NULL, 0 },
{ "dif #236",
  "a, (b, c) = 1, (2, 3)\n"
  "post(a)\n"
  "\n",
  "1", NULL, 0 },
{ "dif #237",
  "a, (b, c) = 1, (2, 3)\n"
  "post(a)\n"
  "post(b)\n"
  "post(c)\n"
  "\n",
  "1\n2\n3", NULL, 0 },
{ "dif #238",
  "a, (b, rest) = 1, (2, [3, 4])\n"
  "post(b)\n"
  "post(rest)\n"
  "\n",
  "2\n[3, 4]", NULL, 0 },
{ "dif #239",
  "a, *m, z = [1,2,3,4]\n"
  "post(a)\n"
  "post(m)\n"
  "post(z)\n"
  "\n",
  "1\n[2, 3]\n4", NULL, 0 },
{ "dif #240",
  "a, *meio, z = [1, 2, 3, 4, 5]\n"
  "post(a)\n"
  "post(meio)\n"
  "post(z)\n"
  "\n",
  "1\n[2, 3, 4]\n5", NULL, 0 },
{ "dif #241",
  "a, *r = \"abc\"\n"
  "post(r)\n"
  "\n",
  "['b', 'c']", NULL, 0 },
{ "dif #242",
  "a, *r = [1,2,3]\n"
  "post(a)\n"
  "post(r)\n"
  "\n",
  "1\n[2, 3]", NULL, 0 },
{ "dif #243",
  "a, *r = [1]\n"
  "post(r)\n"
  "\n",
  "[]", NULL, 0 },
{ "dif #244",
  "a, *resto = (1, 2, 3, 4)\n"
  "post(type(resto))\n"
  "\n",
  "list", NULL, 0 },
{ "dif #245",
  "a, *resto = [1, 2, 3, 4]\n"
  "post(a)\n"
  "post(resto)\n"
  "\n",
  "1\n[2, 3, 4]", NULL, 0 },
{ "dif #246",
  "a, = [5]\n"
  "post(a)\n"
  "\n",
  "5", NULL, 0 },
{ "dif #247",
  "a, b = \"hi\"\n"
  "post(a)\n"
  "post(b)\n"
  "\n",
  "h\ni", NULL, 0 },
{ "dif #248",
  "a, b = \"xy\"\n"
  "post(a)\n"
  "post(b)\n"
  "\n",
  "x\ny", NULL, 0 },
{ "dif #249",
  "a, b = \"çã\"\n"
  "post(a)\n"
  "post(b)\n"
  "\n",
  "ç\nã", NULL, 0 },
{ "dif #250",
  "a, b = 1, 2\n"
  "post(a)\n"
  "post(b)\n"
  "\n",
  "1\n2", NULL, 0 },
{ "dif #251",
  "a, b = 1, 2\n"
  "post(a)\n"
  "post(b)\n"
  "a, b = 10, 20\n"
  "post(a)\n"
  "post(b)\n"
  "\n",
  "1\n2\n10\n20", NULL, 0 },
{ "dif #252",
  "a, b = [1,2]\n"
  "post(a)\n"
  "post(b)\n"
  "\n",
  "1\n2", NULL, 0 },
{ "dif #253",
  "a, b, *resto = [1, 2]\n"
  "post(a)\n"
  "post(b)\n"
  "post(resto)\n"
  "\n",
  "1\n2\n[]", NULL, 0 },
{ "dif #254",
  "a, b, c = 1, 2, 3\n"
  "c, a, b = a, b, c\n"
  "post(a)\n"
  "post(b)\n"
  "post(c)\n"
  "\n",
  "2\n3\n1", NULL, 0 },
{ "dif #255",
  "a, b, c = 1, 2, 3\n"
  "post(a, b, c)\n"
  "x, y = [10, 20]\n"
  "post(x, y)\n"
  "\n",
  "1 2 3\n10 20", NULL, 0 },
{ "dif #256",
  "action a() {\n"
  " return 1/0\n"
  "}\n"
  "action b() {\n"
  " return a()\n"
  "}\n"
  "try {\n"
  " post(b())\n"
  "} catch (e) {\n"
  " post(\"pego\")\n"
  "}\n"
  "\n",
  "pego", NULL, 0 },
{ "dif #257",
  "action a() {\n"
  "    return 1 / 0\n"
  "}\n"
  "action b() {\n"
  "    return a()\n"
  "}\n"
  "post(b())\n"
  "\n",
  NULL, "ZeroDivisionError: division by zero", -1 },
{ "dif #258",
  "action a(n) {\n"
  "\n",
  NULL, "SyntaxError: bloco com '{' nao foi fechado com '}'", -1 },
{ "dif #259",
  "action b(n) {\n"
  "\n",
  NULL, "SyntaxError: bloco com '{' nao foi fechado com '}'", -1 },
{ "dif #260",
  "action b(x) {\n"
  "\n",
  NULL, "SyntaxError: bloco com '{' nao foi fechado com '}'", -1 },
{ "dif #261",
  "action bad(x) {\n"
  " return x / 0\n"
  "}\n"
  "try {\n"
  " post(map([1], bad))\n"
  "} catch (e) {\n"
  " post(\"pego\")\n"
  "}\n"
  "\n",
  "pego", NULL, 0 },
{ "dif #262",
  "action base() {\n"
  " return 1\n"
  "}\n"
  "\n",
  NULL, "SyntaxError: 'base' e palavra reservada da linguagem e nao pode ser usada como n", -1 },
{ "dif #263",
  "action base() { return 1 }\n"
  "\n",
  NULL, "SyntaxError: 'base' e palavra reservada da linguagem e nao pode ser usada como n", -1 },
{ "dif #264",
  "action c() {\n"
  " x = 5\n"
  " return x[\"a\"]\n"
  "}\n"
  "try {\n"
  " post(c())\n"
  "} catch (e) {\n"
  " post(e)\n"
  "}\n"
  "\n",
  "'int' object is not subscriptable (linha 3)", NULL, 0 },
{ "dif #265",
  "action c(x) {\n"
  "\n",
  NULL, "SyntaxError: bloco com '{' nao foi fechado com '}'", -1 },
{ "dif #266",
  "action classifica(v) {\n"
  "\n",
  NULL, "SyntaxError: bloco com '{' nao foi fechado com '}'", -1 },
{ "dif #267",
  "action conectar() { return 1 }\n"
  "\n",
  "", NULL, 0 },
{ "dif #268",
  "action conta(n) {\n"
  "\n",
  NULL, "SyntaxError: bloco com '{' nao foi fechado com '}'", -1 },
{ "dif #269",
  "action contar(n) {\n"
  "    i = 0\n"
  "    while (i < n) {\n"
  "        yield i\n"
  "        i = i + 1\n"
  "    }\n"
  "}\n"
  "resultado = []\n"
  "for each v in contar(0) {\n"
  "    addEnd(resultado, v)\n"
  "}\n"
  "post(resultado)\n"
  "\n",
  "[]", NULL, 0 },
{ "dif #270",
  "action contar(n) {\n"
  "    i = 0\n"
  "    while (i < n) {\n"
  "        yield i\n"
  "        i = i + 1\n"
  "    }\n"
  "}\n"
  "resultado = []\n"
  "for each v in contar(10) {\n"
  "    if (v > 2) { break }\n"
  "    addEnd(resultado, v)\n"
  "}\n"
  "post(resultado)\n"
  "\n",
  "[0, 1, 2]", NULL, 0 },
{ "dif #271",
  "action contar(n) {\n"
  "    i = 0\n"
  "    while (i < n) {\n"
  "        yield i\n"
  "        i = i + 1\n"
  "    }\n"
  "}\n"
  "resultado = []\n"
  "for each v in contar(4) {\n"
  "    addEnd(resultado, v)\n"
  "}\n"
  "post(resultado)\n"
  "\n",
  "[0, 1, 2, 3]", NULL, 0 },
{ "dif #272",
  "action d(n) {\n"
  " return n * 2\n"
  "}\n"
  "post(d(d(d(3))))\n"
  "\n",
  "24", NULL, 0 },
{ "dif #273",
  "action d(n) { return n*2 }\n"
  "x=5\n"
  "post(f\"r={d(x)}\")\n"
  "\n",
  "r=10", NULL, 0 },
{ "dif #274",
  "action d(x) {\n"
  " return x * 2\n"
  "}\n"
  "post(map([1,2,3], d))\n"
  "\n",
  "[2, 4, 6]", NULL, 0 },
{ "dif #275",
  "action d(x) {\n"
  " return x * 2\n"
  "}\n"
  "post(map([], d))\n"
  "\n",
  "[]", NULL, 0 },
{ "dif #276",
  "action d(x) {\n"
  " return x * 2\n"
  "}\n"
  "post(map(map([1,2], d), d))\n"
  "\n",
  "[4, 8]", NULL, 0 },
{ "dif #277",
  "action database() { return 1 }\n"
  "\n",
  "", NULL, 0 },
{ "dif #278",
  "action dict() {\n"
  "\n",
  NULL, "SyntaxError: 'dict' e palavra reservada da linguagem e nao pode ser usada como n", -1 },
{ "dif #279",
  "action dobro(n) {\n"
  "\n",
  NULL, "SyntaxError: bloco com '{' nao foi fechado com '}'", -1 },
{ "dif #280",
  "action dobro(n) {\n"
  " return n * 2\n"
  "}\n"
  "post(dobro(21))\n"
  "\n",
  "42", NULL, 0 },
{ "dif #281",
  "action enviar() { return \"enviado\" }\n"
  "\n",
  "", NULL, 0 },
{ "dif #282",
  "action ext() {\n"
  " action int_() {\n"
  "  return 1\n"
  " }\n"
  " return int_()\n"
  "}\n"
  "post(ext())\n"
  "\n",
  "1", NULL, 0 },
{ "dif #283",
  "action externa() {\n"
  " action interna() {\n"
  "  return 1\n"
  " }\n"
  " return 2\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "dif #284",
  "action f(\n"
  "\n",
  NULL, "SyntaxError: esperado nome de parametro", -1 },
{ "dif #285",
  "action f( {\n"
  "\n",
  NULL, "SyntaxError: esperado nome de parametro", -1 },
{ "dif #286",
  "action f()\n"
  "{\n"
  "    return 99\n"
  "}\n"
  "post(f())\n"
  "\n",
  "99", NULL, 0 },
{ "dif #287",
  "action f() {\n"
  "\n",
  NULL, "SyntaxError: bloco com '{' nao foi fechado com '}'", -1 },
{ "dif #288",
  "action f() {\n"
  "    a, b = 1, 2\n"
  "    return a + b\n"
  "}\n"
  "post(f())\n"
  "\n",
  "3", NULL, 0 },
{ "dif #289",
  "action f() {\n"
  "    return 1, 2\n"
  "}\n"
  "r = f()\n"
  "post(r)\n"
  "\n",
  "(1, 2)", NULL, 0 },
{ "dif #290",
  "action f() {\n"
  "    try {\n"
  "        return \"do try\"\n"
  "    } catch (e) {\n"
  "        return \"do catch\"\n"
  "    } finally {\n"
  "        post(\"finally\")\n"
  "    }\n"
  "}\n"
  "post(f())\n"
  "\n",
  "finally\ndo try", NULL, 0 },
{ "dif #291",
  "action f() {\n"
  "    x = 99\n"
  "}\n"
  "f()\n"
  "try {\n"
  "    post(x)\n"
  "} catch (e) {\n"
  "    post(\"nao-vazou\")\n"
  "}\n"
  "\n",
  "nao-vazou", NULL, 0 },
{ "dif #292",
  "action f() {\n"
  " global base\n"
  "}\n"
  "\n",
  NULL, "SyntaxError: 'base' e palavra reservada da linguagem e nao pode ser usada como n", -1 },
{ "dif #293",
  "action f() {\n"
  " global x\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "dif #294",
  "action f() {\n"
  " global x, y\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "dif #295",
  "action f() {\n"
  " if (a) {\n"
  "  while (b) {\n"
  "   post(1)\n"
  "  }\n"
  " }\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "dif #296",
  "action f() {\n"
  " r = 0\n"
  " if r == 0 { r = 1 }\n"
  " return r\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "dif #297",
  "action f() {\n"
  " return\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "dif #298",
  "action f() {\n"
  " return \"vem daqui\"\n"
  "}\n"
  "post(f())\n"
  "\n",
  "vem daqui", NULL, 0 },
{ "dif #299",
  "action f() {\n"
  " return (1,2,3)\n"
  "}\n"
  "a,b,c = f()\n"
  "post(c)\n"
  "\n",
  "3", NULL, 0 },
{ "dif #300",
  "action f() {\n"
  " return 1\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "dif #301",
  "action f() {\n"
  " return 1, 2\n"
  "}\n"
  "a, b = f()\n"
  "post(a)\n"
  "post(b)\n"
  "\n",
  "1\n2", NULL, 0 },
{ "dif #302",
  "action f() {\n"
  " return 1/0\n"
  "}\n"
  "try {\n"
  " post(f())\n"
  "} catch (e) {\n"
  " post(\"pego\")\n"
  "}\n"
  "\n",
  "pego", NULL, 0 },
{ "dif #303",
  "action f() {\n"
  " return [1, 2, 3]\n"
  "}\n"
  "post(f()[1])\n"
  "\n",
  "2", NULL, 0 },
{ "dif #304",
  "action f() {\n"
  " return [1,2]\n"
  "}\n"
  "post(count int in f())\n"
  "\n",
  "2", NULL, 0 },
{ "dif #305",
  "action f() {\n"
  " try {\n"
  "  return 1/0\n"
  " } catch (e) {\n"
  "  return -1\n"
  " }\n"
  "}\n"
  "post(f())\n"
  "\n",
  "-1", NULL, 0 },
{ "dif #306",
  "action f() {\n"
  " x = await g()\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "dif #307",
  "action f() {\n"
  " y = 99\n"
  "}\n"
  "f()\n"
  "post(y)\n"
  "\n",
  NULL, "NameError: name 'y' is not defined", -1 },
{ "dif #308",
  "action f() {\n"
  " z = 5\n"
  " return z + 1\n"
  "}\n"
  "post(f())\n"
  "\n",
  "6", NULL, 0 },
{ "dif #309",
  "action f() { local = 99 }\n"
  "f()\n"
  "post(\"ok\")\n"
  "\n",
  "ok", NULL, 0 },
{ "dif #310",
  "action f() { return 1 }\n"
  "\n",
  "", NULL, 0 },
{ "dif #311",
  "action f() { return a, 409 }\n"
  "\n",
  "", NULL, 0 },
{ "dif #312",
  "action f() {\n"
  "    return \"oi\"\n"
  "}\n"
  "if __name__ == \"main\" {\n"
  "    post(\"NAO\")\n"
  "}\n"
  "\n",
  "NAO", NULL, 0 },
{ "dif #313",
  "action f() {\n"
  "    return \"sou action\"\n"
  "}\n"
  "for each f in [1, 2] {\n"
  "    post(f)\n"
  "}\n"
  "post(f())\n"
  "\n",
  "1\n2\nsou action", NULL, 0 },
{ "dif #314",
  "action f() {\n"
  "    return 1 / 0\n"
  "}\n"
  "post(f())\n"
  "\n",
  NULL, "ZeroDivisionError: division by zero", -1 },
{ "dif #315",
  "action f() {\n"
  "    return self\n"
  "}\n"
  "post(f())\n"
  "\n",
  NULL, "NameError: name 'self' is not defined", -1 },
{ "dif #316",
  "action f() {\n"
  "    return zzz\n"
  "}\n"
  "post(f())\n"
  "\n",
  NULL, "NameError: name 'zzz' is not defined", -1 },
{ "dif #317",
  "action f(){ return 1 }\n"
  "\n",
  "", NULL, 0 },
{ "dif #318",
  "action f(a) {\n"
  " int b = a * 2\n"
  " return b\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "dif #319",
  "action f(a) {\n"
  " int b = a * 2\n"
  " return b\n"
  "}\n"
  "post(f(5))\n"
  "post(f(7))\n"
  "\n",
  "10\n14", NULL, 0 },
{ "dif #320",
  "action f(a) {\n"
  " return a\n"
  "}\n"
  "post(f(1, a=2))\n"
  "\n",
  "2", NULL, 0 },
{ "dif #321",
  "action f(a) {\n"
  " return a\n"
  "}\n"
  "post(f(z=1))\n"
  "\n",
  NULL, "TypeError: argumento nomeado 'z' nao corresponde a nenhum parametro de", -1 },
{ "dif #322",
  "action f(a) {\n"
  " return a\n"
  "}\n"
  "post(f(zzz=1))\n"
  "\n",
  NULL, "TypeError: argumento nomeado 'zzz' nao corresponde a nenhum parametro ", -1 },
{ "dif #323",
  "action f(a):\n"
  "\n",
  NULL, "SyntaxError: bloco com ':' nao existe mais", -1 },
{ "dif #324",
  "oi = \"ola\"\n"
  "post(f\"v: \n"
  "\n",
  NULL, "SyntaxError: string nao fechada antes da quebra de linha", -1 },
{ "dif #325",
  "out = []\n"
  "for each i in [1, 2, 3] {\n"
  "    for each j in [1, 2, 3] {\n"
  "        if (j == 2) { break }\n"
  "        addEnd(out, i * 10 + j)\n"
  "    }\n"
  "}\n"
  "post(out)\n"
  "\n",
  "[11, 21, 31]", NULL, 0 },
{ "dif #326",
  "p = [3, 3]\n"
  "match p {\n"
  "    case [0, 0] { post(\"orig\") }\n"
  "    case [a, b] if a == b { post(\"diag\") }\n"
  "    case _ { post(\"o\") }\n"
  "}\n"
  "\n",
  "diag", NULL, 0 },
{ "dif #327",
  "post(\n"
  "\n",
  NULL, "SyntaxError: expressao invalida", -1 },
{ "dif #328",
  "post(!0)\n"
  "\n",
  "True", NULL, 0 },
{ "dif #329",
  "post(!1)\n"
  "\n",
  "False", NULL, 0 },
{ "dif #330",
  "post(\"   \".strip())\n"
  "\n",
  "", NULL, 0 },
{ "dif #331",
  "post(\"  \".isspace())\n"
  "\n",
  "True", NULL, 0 },
{ "dif #332",
  "post(\"  \".split())\n"
  "\n",
  "[]", NULL, 0 },
{ "dif #333",
  "post(\"  Ab  \".strip().lower())\n"
  "\n",
  "ab", NULL, 0 },
{ "dif #334",
  "post(\"  x  \".strip())\n"
  "\n",
  "x", NULL, 0 },
{ "dif #335",
  "post(\"  x\".lstrip())\n"
  "\n",
  "x", NULL, 0 },
{ "dif #336",
  "post(\"\" or \"x\")\n"
  "\n",
  "True", NULL, 0 },
{ "dif #337",
  "post(\"\")\n"
  "\n",
  "", NULL, 0 },
{ "dif #338",
  "post(\"\".encode())\n"
  "\n",
  "b''", NULL, 0 },
{ "dif #339",
  "post(\"\".isalpha())\n"
  "\n",
  "False", NULL, 0 },
{ "dif #340",
  "post(\"\".isidentifier())\n"
  "\n",
  "False", NULL, 0 },
{ "dif #341",
  "post(\"\".join([\"a\",\"b\"]))\n"
  "\n",
  "ab", NULL, 0 },
{ "dif #342",
  "post(\"\".len())\n"
  "\n",
  "0", NULL, 0 },
{ "dif #343",
  "post(\"\".maketrans(\"ab\",\"x\"))\n"
  "\n",
  NULL, "TypeError: maketrans() exige os dois com o mesmo tamanho", -1 },
{ "dif #344",
  "post(\"\".maketrans(\"ab\",\"xy\"))\n"
  "\n",
  "{97: 120, 98: 121}", NULL, 0 },
{ "dif #345",
  "post(\"\".split(\",\"))\n"
  "\n",
  "['']", NULL, 0 },
{ "dif #346",
  "post(\"\".splitlines())\n"
  "\n",
  "[]", NULL, 0 },
{ "dif #347",
  "post(\"\".strip())\n"
  "\n",
  "", NULL, 0 },
{ "dif #348",
  "post(\"\".upper())\n"
  "\n",
  "", NULL, 0 },
{ "dif #349",
  "post(\",\".join([]))\n"
  "\n",
  "", NULL, 0 },
{ "dif #350",
  "post(\",a,\".split(\",\"))\n"
  "\n",
  "['', 'a', '']", NULL, 0 },
{ "dif #351",
  "post(\"-\".join(\"a,b\".split(\",\")))\n"
  "\n",
  "a-b", NULL, 0 },
{ "dif #352",
  "post(\"-\".join([\"a\",\"b\"]))\n"
  "\n",
  "a-b", NULL, 0 },
{ "dif #353",
  "post(\"-5\".zfill(4))\n"
  "\n",
  "-005", NULL, 0 },
{ "dif #354",
  "post(\"12\".isnumeric())\n"
  "\n",
  "True", NULL, 0 },
{ "dif #355",
  "post(\"123\".isdigit())\n"
  "\n",
  "True", NULL, 0 },
{ "dif #356",
  "post(\"123\".match(\"\\\\d+\"))\n"
  "\n",
  "True", NULL, 0 },
{ "dif #357",
  "post(\"150\".isdigit())\n"
  "\n",
  "True", NULL, 0 },
{ "dif #358",
  "post(\"1abc\".isidentifier())\n"
  "\n",
  "False", NULL, 0 },
{ "dif #359",
  "post(\"5\".zfill(3))\n"
  "\n",
  "005", NULL, 0 },
{ "dif #360",
  "post(\"AB\".isupper())\n"
  "\n",
  "True", NULL, 0 },
{ "dif #361",
  "post(\"AB\".lower())\n"
  "\n",
  "ab", NULL, 0 },
{ "dif #362",
  "post(\"Ab Cd\".istitle())\n"
  "\n",
  "True", NULL, 0 },
{ "dif #363",
  "post(\"AbC\".casefold())\n"
  "\n",
  "abc", NULL, 0 },
{ "dif #364",
  "post(\"AÇÃO\".isupper())\n"
  "\n",
  "True", NULL, 0 },
{ "dif #365",
  "post(\"Clima: \" {grau} \"°\")\n"
  "\n",
  NULL, "NameError: name 'grau' is not defined", -1 },
{ "dif #366",
  "post(\"Clima: \" {g} \"C\")\n"
  "\n",
  NULL, "NameError: name 'g' is not defined", -1 },
{ "dif #367",
  "post(\"NAO DEVE APARECER\")\n"
  "\n",
  "NAO DEVE APARECER", NULL, 0 },
{ "dif #368",
  "post(\"NAO DEVIA RODAR\")\n"
  "\n",
  "NAO DEVIA RODAR", NULL, 0 },
{ "dif #369",
  "post(\"\\033[1mA\\033[0m\")\n"
  "\n",
  "[1mA[0m", NULL, 0 },
{ "dif #370",
  "post(\"\\101-\\x42-\\xe9\")\n"
  "\n",
  "A-B-é", NULL, 0 },
{ "dif #371",
  "post(\"\\e[3mC\\e[0m\")\n"
  "\n",
  "[3mC[0m", NULL, 0 },
{ "dif #372",
  "post(\"\\tx\\n\".strip())\n"
  "\n",
  "x", NULL, 0 },
{ "dif #373",
  "post(\"\\x1b[31mB\\x1b[0m\")\n"
  "\n",
  "[31mB[0m", NULL, 0 },
{ "dif #374",
  "post(\"_\".isidentifier())\n"
  "\n",
  "True", NULL, 0 },
{ "dif #375",
  "post(\"a b  c\".split())\n"
  "\n",
  "['a', 'b', 'c']", NULL, 0 },
{ "dif #376",
  "post(\"a b\".isidentifier())\n"
  "\n",
  "False", NULL, 0 },
{ "dif #377",
  "post(\"a\" != \"b\")\n"
  "\n",
  "True", NULL, 0 },
{ "dif #378",
  "post(\"a\" * \"b\")\n"
  "\n",
  NULL, "TypeError: can't multiply sequence by non-int of type 'str'", -1 },
{ "dif #379",
  "post(\"a\" + \"b\")\n"
  "\n",
  "ab", NULL, 0 },
{ "dif #380",
  "post(\"a\" + 1)\n"
  "\n",
  NULL, "TypeError: can only concatenate str (not \"int\") to str", -1 },
{ "dif #381",
  "post(\"a\" - 1)\n"
  "\n",
  NULL, "TypeError: unsupported operand type(s) for -: 'str' and 'int'", -1 },
{ "dif #382",
  "post(\"a\" <= \"a\")\n"
  "\n",
  "True", NULL, 0 },
{ "dif #383",
  "post(\"a\" > 1)\n"
  "\n",
  NULL, "TypeError: '>' not supported between instances of 'str' and 'int'", -1 },
{ "dif #384",
  "post(\"a\" ^ 1)\n"
  "\n",
  NULL, "TypeError: unsupported operand type(s) for ^: 'str' and 'int'", -1 },
{ "dif #385",
  "post(\"a\" in \"cab\")\n"
  "\n",
  "True", NULL, 0 },
{ "dif #386",
  "post(\"a\" in {\"a\":1})\n"
  "\n",
  "True", NULL, 0 },
{ "dif #387",
  "post(\"a\")\n"
  "\n",
  "a", NULL, 0 },
{ "dif #388",
  "post(\"a\".append(1))\n"
  "\n",
  NULL, "AttributeError: 'str' object has no attribute 'append'", -1 },
{ "dif #389",
  "post(\"a\".center(3,\"xy\"))\n"
  "\n",
  NULL, "TypeError: center() espera um unico caractere de preenchimento", -1 },
{ "dif #390",
  "post(\"a\".center(4))\n"
  "\n",
  " a  ", NULL, 0 },
{ "dif #391",
  "post(\"a\".center(9223372036854775807))\n"
  "\n",
  NULL, "MemoryError: memória insuficiente: center() pediu largura 9223372036854775807", -1 },
{ "dif #392",
  "post(\"a\".count())\n"
  "\n",
  NULL, "TypeError: count expected at least 1 argument, got 0", -1 },
{ "dif #393",
  "post(\"a\".encode(\"utf-8\"))\n"
  "\n",
  "b'a'", NULL, 0 },
{ "dif #394",
  "post(\"a\".encode() + \"b\")\n"
  "\n",
  NULL, "TypeError: can't concat str to bytes", -1 },
{ "dif #395",
  "post(\"a\".index(\"z\"))\n"
  "\n",
  NULL, "ValueError: valor invalido: subcadeia nao encontrada", -1 },
{ "dif #396",
  "post(\"a\".isprintable())\n"
  "\n",
  "True", NULL, 0 },
{ "dif #397",
  "post(\"a\".join([1]))\n"
  "\n",
  NULL, "TypeError: join() so junta str", -1 },
{ "dif #398",
  "post(\"a\".ljust(\"x\"))\n"
  "\n",
  NULL, "TypeError: 'str' object cannot be interpreted as an integer", -1 },
{ "dif #399",
  "post(\"a\".ljust(9223372036854775807))\n"
  "\n",
  NULL, "MemoryError: memória insuficiente: ljust() pediu largura 9223372036854775807", -1 },
{ "dif #400",
  "post(\"a\".match())\n"
  "\n",
  NULL, "TypeError: match expected 1 argument, got 0", -1 },
{ "dif #401",
  "post(\"a\".naoexiste())\n"
  "\n",
  NULL, "AttributeError: 'str' object has no attribute 'naoexiste'", -1 },
{ "dif #402",
  "post(\"a\".partition(\"\"))\n"
  "\n",
  NULL, "ValueError: valor invalido: separador vazio em partition()", -1 },
{ "dif #403",
  "post(\"a\".replace(\"\",\"x\"))\n"
  "\n",
  "xax", NULL, 0 },
{ "dif #404",
  "post(\"a\".rindex(\"z\"))\n"
  "\n",
  NULL, "ValueError: valor invalido: subcadeia nao encontrada", -1 },
{ "dif #405",
  "post(\"a\".rjust(9223372036854775807))\n"
  "\n",
  NULL, "MemoryError: memória insuficiente: rjust() pediu largura 9223372036854775807", -1 },
{ "dif #406",
  "post(\"a\".split(\"\"))\n"
  "\n",
  NULL, "ValueError: valor invalido: separador vazio em split()", -1 },
{ "dif #407",
  "post(\"a\".startswith(1))\n"
  "\n",
  NULL, "TypeError: startswith() argument 1 must be str, not int", -1 },
{ "dif #408",
  "post(\"a\".sub(\"a\"))\n"
  "\n",
  NULL, "TypeError: sub expected 2 arguments, got 1", -1 },
{ "dif #409",
  "post(\"a\".type())\n"
  "\n",
  "str", NULL, 0 },
{ "dif #410",
  "post(\"a\".upper(1))\n"
  "\n",
  NULL, "TypeError: upper expected 0 arguments, got 1", -1 },
{ "dif #411",
  "post(\"a\".zfill(5))\n"
  "post(\"ab\".ljust(5, \"-\"))\n"
  "post(\"ab\".center(6, \".\"))\n"
  "post(\"7\".rjust(3, \"0\"))\n"
  "\n",
  "0000a\nab---\n..ab..\n007", NULL, 0 },
{ "dif #412",
  "post(\"a\".zfill(9223372036854775807))\n"
  "\n",
  NULL, "MemoryError: memória insuficiente: zfill() pediu largura 9223372036854775807", -1 },
{ "dif #413",
  "post(\"a,b,c\".split(\",\"))\n"
  "\n",
  "['a', 'b', 'c']", NULL, 0 },
{ "dif #414",
  "post(\"a,b,c\".split(\",\", maxsplit=1))\n"
  "\n",
  "['a', 'b,c']", NULL, 0 },
{ "dif #415",
  "post(\"a,b,c\".split(\",\",1))\n"
  "\n",
  "['a', 'b,c']", NULL, 0 },
{ "dif #416",
  "post(\"a.b\".findall(\"[.]\"))\n"
  "\n",
  "['.']", NULL, 0 },
{ "dif #417",
  "post(\"a1\".isalnum())\n"
  "\n",
  "True", NULL, 0 },
{ "dif #418",
  "post(\"a1b\".title())\n"
  "\n",
  "A1B", NULL, 0 },
{ "dif #419",
  "post(\"a1b2\".findall(\"\\\\d\"))\n"
  "\n",
  "['1', '2']", NULL, 0 },
{ "dif #420",
  "post(\"a1b2\".sub(\"\\\\d\", \"#\"))\n"
  "\n",
  "a#b#", NULL, 0 },
{ "dif #421",
  "post(\"a=b\".partition(\"=\"))\n"
  "\n",
  "('a', '=', 'b')", NULL, 0 },
{ "dif #422",
  "post(\"a=b=c\".partition(\"=\"))\n"
  "\n",
  "('a', '=', 'b=c')", NULL, 0 },
{ "dif #423",
  "post(\"a=b=c\".rpartition(\"=\"))\n"
  "\n",
  "('a=b', '=', 'c')", NULL, 0 },
{ "dif #424",
  "post(\"aBc\".swapcase())\n"
  "\n",
  "AbC", NULL, 0 },
{ "dif #425",
  "post(\"aXb\".replace(\"X\",\"\"))\n"
  "\n",
  "ab", NULL, 0 },
{ "dif #426",
  "post(\"aXbXc\".replace(\"X\",\"-\"))\n"
  "\n",
  "a-b-c", NULL, 0 },
{ "dif #427",
  "post(\"aXbXc\".replace(\"X\",\"-\",1))\n"
  "\n",
  "a-bXc", NULL, 0 },
{ "dif #428",
  "post(\"a\\\\nb\".encode())\n"
  "\n",
  "b'a\\\\nb'", NULL, 0 },
{ "dif #429",
  "post(\"a\\nb\\nc\".splitlines())\n"
  "\n",
  "['a', 'b', 'c']", NULL, 0 },
{ "dif #430",
  "post(\"a\\r\\nb\".splitlines())\n"
  "\n",
  "['a', 'b']", NULL, 0 },
{ "dif #431",
  "post(\"a\\tb\".expandtabs())\n"
  "\n",
  "a       b", NULL, 0 },
{ "dif #432",
  "post(\"a\\tb\".expandtabs(4))\n"
  "\n",
  "a   b", NULL, 0 },
{ "dif #433",
  "post(\"a_b\".isidentifier())\n"
  "\n",
  "True", NULL, 0 },
{ "dif #434",
  "post(\"aaa\".replace(\"a\", \"b\", conta=2))\n"
  "\n",
  NULL, "TypeError: argumento nomeado desconhecido: conta", -1 },
{ "dif #435",
  "post(\"aaa\".replace(old=\"a\", new=\"b\", count=1))\n"
  "\n",
  "baa", NULL, 0 },
/* REGRAVADO: `"ab" * 2` era erro e passou a repetir, como a lista já fazia
 * e como o Python faz. Foi o oráculo (comparação com o Python) que apontou. */
{ "dif #436",
  "post(\"ab\" * 2)\n"
  "\n",
  "abab", NULL, 0 },
{ "dif #437",
  "post(\"ab\".center(5))\n"
  "\n",
  "  ab ", NULL, 0 },
{ "dif #438",
  "post(\"ab\".center(6))\n"
  "\n",
  "  ab  ", NULL, 0 },
{ "dif #439",
  "post(\"ab\".encode() == \"ab\")\n"
  "\n",
  "False", NULL, 0 },
{ "dif #440",
  "post(\"ab\".encode() == \"ab\".encode())\n"
  "\n",
  "True", NULL, 0 },
{ "dif #441",
  /* Este caso guardava o comportamento de quando `bytes` era o ÚNICO tipo
   * embutido SEM `.len()`: a chamada levantava "membro inexistente". O método
   * entrou em 28/08 e o esperado passou a ser o tamanho. A mudança é
   * deliberada, e o diferencial fez o trabalho dele — avisou. */
  "post(\"ab\".encode().len())\n"
  "\n",
  "2", NULL, 0 },
{ "dif #442",
  "post(\"ab\".isascii())\n"
  "\n",
  "True", NULL, 0 },
{ "dif #443",
  "post(\"ab\".islower())\n"
  "\n",
  "True", NULL, 0 },
{ "dif #444",
  "post(\"ab\".ljust(5))\n"
  "\n",
  "ab   ", NULL, 0 },
{ "dif #445",
  "post(\"ab\".ljust(5,\"*\"))\n"
  "\n",
  "ab***", NULL, 0 },
{ "dif #446",
  "post(\"ab\".replace(\"\",\"-\"))\n"
  "\n",
  "-a-b-", NULL, 0 },
{ "dif #447",
  "post(\"ab\".rjust(5))\n"
  "\n",
  "   ab", NULL, 0 },
{ "dif #448",
  "post(\"ab\".upper())\n"
  "\n",
  "AB", NULL, 0 },
{ "dif #449",
  "post(\"ab\"[9])\n"
  "\n",
  NULL, "IndexError: string index out of range", -1 },
{ "dif #450",
  "post(\"abc\" < \"abd\")\n"
  "\n",
  "True", NULL, 0 },
{ "dif #451",
  "post(\"abc\" == \"abc\")\n"
  "\n",
  "True", NULL, 0 },
{ "dif #452",
  "post(\"abc\" == \"abd\")\n"
  "\n",
  "False", NULL, 0 },
{ "dif #453",
  "post(\"abc\".\n"
  "\n",
  NULL, "SyntaxError: esperado nome do membro apos '.'", -1 },
{ "dif #454",
  "post(\"abc\".center(7))\n"
  "\n",
  "  abc  ", NULL, 0 },
{ "dif #455",
  "post(\"abc\".contains(\"b\"))\n"
  "\n",
  "True", NULL, 0 },
{ "dif #456",
  "post(\"abc\".count(\"\"))\n"
  "\n",
  "4", NULL, 0 },
{ "dif #457",
  "post(\"abc\".encode() + \"d\".encode())\n"
  "\n",
  "b'abcd'", NULL, 0 },
{ "dif #458",
  "post(\"abc\".encode())\n"
  "\n",
  "b'abc'", NULL, 0 },
{ "dif #459",
  "post(\"abc\".endswith(\"bc\"))\n"
  "\n",
  "True", NULL, 0 },
{ "dif #460",
  "post(\"abc\".find(\"z\"))\n"
  "\n",
  "-1", NULL, 0 },
{ "dif #461",
  "post(\"abc\".format_map({\"x\":1}))\n"
  "\n",
  "abc", NULL, 0 },
{ "dif #462",
  "post(\"abc\".has(\"z\"))\n"
  "\n",
  "False", NULL, 0 },
{ "dif #463",
  "post(\"abc\".index(\"b\"))\n"
  "\n",
  "1", NULL, 0 },
{ "dif #464",
  "post(\"abc\".isalpha())\n"
  "\n",
  "True", NULL, 0 },
{ "dif #465",
  "post(\"abc\".isidentifier())\n"
  "\n",
  "True", NULL, 0 },
{ "dif #466",
  "post(\"abc\".maketrans(\"ab\",\"xy\"))\n"
  "\n",
  "{97: 120, 98: 121}", NULL, 0 },
{ "dif #467",
  "post(\"abc\".match(\"\\\\d+\"))\n"
  "\n",
  "False", NULL, 0 },
{ "dif #468",
  "post(\"abc\".partition(\"z\"))\n"
  "\n",
  "('abc', '', '')", NULL, 0 },
{ "dif #469",
  "post(\"abc\".replace(\"z\",\"y\"))\n"
  "\n",
  "abc", NULL, 0 },
{ "dif #470",
  "post(\"abc\".rpartition(\"z\"))\n"
  "\n",
  "('', '', 'abc')", NULL, 0 },
{ "dif #471",
  "post(\"abc\".startswith(\"ab\"))\n"
  "\n",
  "True", NULL, 0 },
{ "dif #472",
  "post(\"abc\".startswith(\"z\"))\n"
  "\n",
  "False", NULL, 0 },
{ "dif #473",
  "post(\"abc\".zfill(2))\n"
  "\n",
  "abc", NULL, 0 },
{ "dif #474",
  "post(\"abcabc\".count(\"a\"))\n"
  "\n",
  "2", NULL, 0 },
{ "dif #475",
  "post(\"abcabc\".find(\"c\"))\n"
  "\n",
  "2", NULL, 0 },
{ "dif #476",
  "post(\"abcabc\".rfind(\"c\"))\n"
  "\n",
  "5", NULL, 0 },
{ "dif #477",
  "post(\"abcba\".strip(\"ab\"))\n"
  "\n",
  "c", NULL, 0 },
{ "dif #478",
  "post(\"antes\")\n"
  "\n",
  "antes", NULL, 0 },
{ "dif #479",
  "post(\"ação e coração\")\n"
  "x = 1\n"
  "\n",
  "ação e coração", NULL, 0 },
{ "dif #480",
  "post(\"ação\" + \" çã\")\n"
  "\n",
  "ação çã", NULL, 0 },
{ "dif #481",
  "post(\"ação\")\n"
  "\n",
  "ação", NULL, 0 },
{ "dif #482",
  "post(\"b\" > \"a\")\n"
  "\n",
  "True", NULL, 0 },
{ "dif #483",
  "post(\"b\")\n"
  "\n",
  "b", NULL, 0 },
{ "dif #484",
  "post(\"banana\".index(\"na\", 0, 3))\n"
  "\n",
  NULL, "ValueError: valor invalido: subcadeia nao encontrada", -1 },
{ "dif #485",
  "post(\"corpo\")\n"
  "\n",
  "corpo", NULL, 0 },
{ "dif #486",
  "post(\"fim\")\n"
  "\n",
  "fim", NULL, 0 },
{ "dif #487",
  "post(\"hello cli\")\n"
  "\n",
  "hello cli", NULL, 0 },
{ "dif #488",
  "post(\"hello world\".sub(\"\\\\s+\", \"_\"))\n"
  "\n",
  "hello_world", NULL, 0 },
{ "dif #489",
  "post(\"linha1\\nlinha2\")\n"
  "\n",
  "linha1\nlinha2", NULL, 0 },
{ "dif #490",
  "post(\"nao json\".get(\"k\"))\n"
  "\n",
  "null", NULL, 0 },
{ "dif #491",
  "post(\"nao json\".get_json())\n"
  "\n",
  "null", NULL, 0 },
{ "dif #492",
  "post(\"oi\")\n"
  "\n",
  "oi", NULL, 0 },
{ "dif #493",
  "post(\"ok\")\n"
  "\n",
  "ok", NULL, 0 },
{ "dif #494",
  "post(\"ola {}\".format(\"mundo\"))\n"
  "\n",
  "ola mundo", NULL, 0 },
{ "dif #495",
  "post(\"olá mundo\".capitalize())\n"
  "\n",
  "Olá mundo", NULL, 0 },
{ "dif #496",
  "post(\"olá mundo\".title())\n"
  "\n",
  "Olá Mundo", NULL, 0 },
{ "dif #497",
  "post(\"olá ção\".upper())\n"
  "\n",
  "OLÁ ÇÃO", NULL, 0 },
{ "dif #498",
  "post(\"po\" + \"ol\" + \"script\")\n"
  "\n",
  "poolscript", NULL, 0 },
{ "dif #499",
  "post(\"pool\"[0])\n"
  "post(\"pool\"[-1])\n"
  "\n",
  "p\nl", NULL, 0 },
{ "dif #500",
  "post(\"pre-x\".removeprefix(\"pre-\"))\n"
  "\n",
  "x", NULL, 0 },
{ "dif #501",
  "post(\"segue\")\n"
  "\n",
  "segue", NULL, 0 },
{ "dif #502",
  "post(\"sim\" if 1 > 3 else \"nao\")\n"
  "\n",
  "nao", NULL, 0 },
{ "dif #503",
  "post(\"sim\" if 5 > 3 else \"nao\")\n"
  "\n",
  "sim", NULL, 0 },
{ "dif #504",
  "post(\"um\" if n == 1 else \"dois\" if n == 2 else \"outro\")\n"
  "\n",
  NULL, "NameError: name 'n' is not defined", -1 },
{ "dif #505",
  "post(\"x  \".rstrip())\n"
  "\n",
  "x", NULL, 0 },
{ "dif #506",
  "post(\"x\" is PoolFile)\n"
  "\n",
  "False", NULL, 0 },
{ "dif #507",
  "post(\"x\".removeprefix(\"z\"))\n"
  "\n",
  "x", NULL, 0 },
{ "dif #508",
  "post(\"x\".upper().len())\n"
  "\n",
  "1", NULL, 0 },
{ "dif #509",
  "post(\"x.txt\".removesuffix(\".txt\"))\n"
  "\n",
  "x", NULL, 0 },
{ "dif #510",
  "post(\"xxaxx\".strip(\"x\"))\n"
  "\n",
  "a", NULL, 0 },
{ "dif #511",
  "post(\"z\" in {\"a\":1})\n"
  "\n",
  "False", NULL, 0 },
{ "dif #512",
  "post(\"z\" not in \"cab\")\n"
  "\n",
  "True", NULL, 0 },
{ "dif #513",
  "post(\"{0} {0}\".format(\"a\"))\n"
  "\n",
  "a a", NULL, 0 },
{ "dif #514",
  "post(\"{1} {0}\".format(\"a\",\"b\"))\n"
  "\n",
  "b a", NULL, 0 },
{ "dif #515",
  "post(\"{:!!}\".format(1))\n"
  "\n",
  NULL, "TypeError: format: spec invalido", -1 },
{ "dif #516",
  "post(\"{:*^7}|\".format(\"ab\"))\n"
  "\n",
  "**ab***|", NULL, 0 },
{ "dif #517",
  "post(\"{:.2f}\".format(3.14159))\n"
  "\n",
  "3.14", NULL, 0 },
{ "dif #518",
  "post(\"{:.3f}\".format(1))\n"
  "\n",
  "1.000", NULL, 0 },
{ "dif #519",
  "post(\"{:05d}\".format(42))\n"
  "\n",
  "00042", NULL, 0 },
{ "dif #520",
  "post(\"{:5}|\".format(\"a\"))\n"
  "\n",
  "a    |", NULL, 0 },
{ "dif #521",
  "post(\"{:<5}|\".format(\"a\"))\n"
  "\n",
  "a    |", NULL, 0 },
{ "dif #522",
  "post(\"{:>5}|\".format(\"a\"))\n"
  "\n",
  "    a|", NULL, 0 },
{ "dif #523",
  "post(\"{:>5}|\".format(7))\n"
  "\n",
  "    7|", NULL, 0 },
{ "dif #524",
  "post(\"{:X}\".format(255))\n"
  "\n",
  "FF", NULL, 0 },
{ "dif #525",
  "post(\"{:^5}|\".format(\"a\"))\n"
  "\n",
  "  a  |", NULL, 0 },
{ "dif #526",
  "post(\"{:b}\".format(5))\n"
  "\n",
  "101", NULL, 0 },
{ "dif #527",
  "post(\"{:o}\".format(8))\n"
  "\n",
  "10", NULL, 0 },
{ "dif #528",
  "post(\"{:x}\".format(255))\n"
  "\n",
  "ff", NULL, 0 },
{ "dif #529",
  "post(\"{a}-{b}\".format_map({\"a\":1,\"b\":2}))\n"
  "\n",
  "1-2", NULL, 0 },
{ "dif #530",
  "post(\"{x}\".format_map({\"x\":1}))\n"
  "\n",
  "1", NULL, 0 },
{ "dif #531",
  "post(\"{x}\".format_map({\"x\":Null}))\n"
  "\n",
  "null", NULL, 0 },
{ "dif #532",
  "post(\"{{}}\".format())\n"
  "\n",
  "{}", NULL, 0 },
{ "dif #533",
  "post(\"{} e {}\".format(1,2))\n"
  "\n",
  "1 e 2", NULL, 0 },
{ "dif #534",
  "post(\"{} {}\".format(1))\n"
  "\n",
  NULL, "TypeError: format: campo '' sem valor", -1 },
{ "dif #535",
  "post(\"{}\".format(Null))\n"
  "\n",
  "null", NULL, 0 },
{ "dif #536",
  "post(\"{}\".format([1,2]))\n"
  "\n",
  "[1, 2]", NULL, 0 },
{ "dif #537",
  "post(\"{}\".format([Null]))\n"
  "\n",
  "[null]", NULL, 0 },
{ "dif #538",
  "post(\"ÇÃO\".lower())\n"
  "\n",
  "ção", NULL, 0 },
{ "dif #539",
  "post(\"ção\".center(7,\"-\"))\n"
  "\n",
  "--ção--", NULL, 0 },
{ "dif #540",
  "post(\"ção\".encode())\n"
  "\n",
  "b'\\xc3\\xa7\\xc3\\xa3o'", NULL, 0 },
{ "dif #541",
  "post(\"ção\".findall(\"\\\\w+\"))\n"
  "\n",
  "['ção']", NULL, 0 },
{ "dif #542",
  "post(\"ção\".isalpha())\n"
  "\n",
  "True", NULL, 0 },
{ "dif #543",
  "post(\"ção\".isascii())\n"
  "\n",
  "False", NULL, 0 },
{ "dif #544",
  "post(\"ção\".isidentifier())\n"
  "\n",
  "True", NULL, 0 },
{ "dif #545",
  "post(\"ção\".len())\n"
  "\n",
  "3", NULL, 0 },
{ "dif #546",
  "post(\"ção\".ljust(5,\".\"))\n"
  "\n",
  "ção..", NULL, 0 },
{ "dif #547",
  "post(\"ção\".upper())\n"
  "\n",
  "ÇÃO", NULL, 0 },
{ "dif #548",
  "post(\"çãoção\".find(\"o\"))\n"
  "\n",
  "2", NULL, 0 },
{ "dif #549",
  "post(((1 + 2) * (3 + 4)) - (5 * (6 - 4)))\n"
  "\n",
  "11", NULL, 0 },
{ "dif #550",
  "post((1 < 2) < 3)\n"
  "\n",
  "True", NULL, 0 },
{ "dif #551",
  "post((1).keys())\n"
  "\n",
  NULL, "AttributeError: 'int' object has no attribute 'keys'", -1 },
{ "dif #552",
  "post((1).type())\n"
  "\n",
  "int", NULL, 0 },
{ "dif #553",
  "post((1).upper())\n"
  "\n",
  "1", NULL, 0 },
{ "dif #554",
  "post((1, zzz))\n"
  "\n",
  NULL, "NameError: name 'zzz' is not defined", -1 },
{ "dif #555",
  "post((1,) is list)\n"
  "\n",
  "False", NULL, 0 },
{ "dif #556",
  "post((1,2) == (1,2))\n"
  "\n",
  "True", NULL, 0 },
{ "dif #557",
  "post((1,2).contains(2))\n"
  "\n",
  "True", NULL, 0 },
{ "dif #558",
  "post((1,2).type())\n"
  "\n",
  "tup", NULL, 0 },
{ "dif #559",
  "post((1.5).contains(\".\"))\n"
  "\n",
  "True", NULL, 0 },
{ "dif #560",
  "post((1.5).type())\n"
  "\n",
  "flo", NULL, 0 },
{ "dif #561",
  "post((12).len())\n"
  "\n",
  NULL, "TypeError: object of type 'int' has no len()", -1 },
{ "dif #562",
  "post((123).zfill(5))\n"
  "\n",
  "00123", NULL, 0 },
{ "dif #563",
  "post((150).isdigit())\n"
  "\n",
  "True", NULL, 0 },
{ "dif #564",
  "post((2 + 3) * 4)\n"
  "\n",
  "20", NULL, 0 },
{ "dif #565",
  "post((2 + 3) << 1)\n"
  "\n",
  "10", NULL, 0 },
{ "dif #566",
  "post()\n"
  "\n",
  "", NULL, 0 },
{ "dif #567",
  "post(-1 < Null)\n"
  "\n",
  NULL, "TypeError: '<' not supported between instances of 'int' and 'Null'", -1 },
{ "dif #568",
  "post(-1.0 % 3)\n"
  "\n",
  "2.0", NULL, 0 },
{ "dif #569",
  "post(-7 % 2.5)\n"
  "\n",
  "0.5", NULL, 0 },
{ "dif #570",
  "post(-7)\n"
  "\n",
  "-7", NULL, 0 },
{ "dif #571",
  "post(-9223372036854775808 % -1)\n"
  "\n",
  "0", NULL, 0 },
{ "dif #572",
  "post(-a)\n"
  "\n",
  NULL, "NameError: name 'a' is not defined", -1 },
{ "dif #573",
  "post(0 - 5)\n"
  "\n",
  "-5", NULL, 0 },
{ "dif #574",
  "post(0 - 7 % 3)\n"
  "\n",
  "-1", NULL, 0 },
{ "dif #575",
  "post(0 == false)\n"
  "\n",
  "True", NULL, 0 },
{ "dif #576",
  "post(0 and 5)\n"
  "\n",
  "False", NULL, 0 },
{ "dif #577",
  "post(0 or 5)\n"
  "\n",
  "True", NULL, 0 },
{ "dif #578",
  "post(0 || 7)\n"
  "\n",
  "True", NULL, 0 },
{ "dif #579",
  "post(0.1 + 0.2)\n"
  "\n",
  "0.30000000000000004", NULL, 0 },
{ "dif #580",
  "post(1 % 0)\n"
  "\n",
  NULL, "ZeroDivisionError: integer modulo by zero", -1 },
{ "dif #581",
  "post(1 && 2)\n"
  "\n",
  "True", NULL, 0 },
{ "dif #582",
  "post(1 + \"a\")\n"
  "\n",
  NULL, "TypeError: unsupported operand type(s) for +: 'int' and 'str'", -1 },
{ "dif #583",
  "post(1 + 1)\n"
  "\n",
  "2", NULL, 0 },
{ "dif #584",
  "post(1 + zzz)\n"
  "\n",
  NULL, "NameError: name 'zzz' is not defined", -1 },
{ "dif #585",
  "post(1 / 0)\n"
  "\n",
  NULL, "ZeroDivisionError: division by zero", -1 },
{ "dif #586",
  "post(1 < 2, 2 <= 2, 3 > 4, 5 == 5, 5 != 6)\n"
  "\n",
  "True True False True True", NULL, 0 },
{ "dif #587",
  "post(1 << -1)\n"
  "\n",
  NULL, "RuntimeError: deslocamento negativo", -1 },
{ "dif #588",
  "post(1 << 2 & 6)\n"
  "\n",
  "4", NULL, 0 },
{ "dif #589",
  "post(1 << 4)\n"
  "\n",
  "16", NULL, 0 },
{ "dif #590",
  "post(1 == 1 | 0)\n"
  "\n",
  "True", NULL, 0 },
{ "dif #591",
  "post(1 == true)\n"
  "\n",
  "True", NULL, 0 },
{ "dif #592",
  "post(1 > Null)\n"
  "\n",
  NULL, "TypeError: '>' not supported between instances of 'int' and 'Null'", -1 },
{ "dif #593",
  "post(1 >> -1)\n"
  "\n",
  NULL, "RuntimeError: deslocamento negativo", -1 },
{ "dif #594",
  "post(1 if x > 0 else x / 0)\n"
  "\n",
  NULL, "NameError: name 'x' is not defined", -1 },
{ "dif #595",
  "post(1 in (1,2))\n"
  "\n",
  "True", NULL, 0 },
{ "dif #596",
  "post(1 in [])\n"
  "\n",
  "False", NULL, 0 },
{ "dif #597",
  "post(1 in zzz)\n"
  "\n",
  NULL, "NameError: name 'zzz' is not defined", -1 },
{ "dif #598",
  "post(1%0)\n"
  "\n",
  NULL, "ZeroDivisionError: integer modulo by zero", -1 },
{ "dif #599",
  "post(1)\n"
  "\n",
  "1", NULL, 0 },
{ "dif #600",
  "post(1, 2)\n"
  "\n",
  "1 2", NULL, 0 },
{ "dif #601",
  "post(1, 2.5, \"x\", true, false, null)\n"
  "\n",
  "1 2.5 x True False null", NULL, 0 },
{ "dif #602",
  "post(1.0 / 0)\n"
  "\n",
  NULL, "ZeroDivisionError: flo division by zero", -1 },
{ "dif #603",
  "post(1.0)\n"
  "\n",
  "1.0", NULL, 0 },
{ "dif #604",
  "post(1.5 % 0)\n"
  "\n",
  NULL, "ZeroDivisionError: flo modulo", -1 },
{ "dif #605",
  "post(1.5 * 2)\n"
  "\n",
  "3.0", NULL, 0 },
{ "dif #606",
  "post(1.5 + 2.5)\n"
  "\n",
  "4.0", NULL, 0 },
{ "dif #607",
  "post(1.5 is int)\n"
  "\n",
  "False", NULL, 0 },
{ "dif #608",
  "post(1.5 | 1)\n"
  "\n",
  NULL, "TypeError: unsupported operand type(s) for |: 'flo' and 'int'", -1 },
{ "dif #609",
  "post(1/0)\n"
  "\n",
  NULL, "ZeroDivisionError: division by zero", -1 },
{ "dif #610",
  "post(1/3)\n"
  "\n",
  "0.3333333333333333", NULL, 0 },
{ "dif #611",
  "post(1/7)\n"
  "\n",
  "0.14285714285714285", NULL, 0 },
{ "dif #612",
  "post(10 - 3 - 2)\n"
  "\n",
  "5", NULL, 0 },
{ "dif #613",
  "post(1000000000000 * 1000000000000)\n"
  "\n",
  "1000000000000000000000000", NULL, 0 },
{ "dif #614",
  "post(123456789012345678901234567890 + 1)\n"
  "\n",
  "123456789012345678901234567891", NULL, 0 },
{ "dif #615",
  "post(150.0)\n"
  "\n",
  "150.0", NULL, 0 },
{ "dif #616",
  "post(2 != 3)\n"
  "\n",
  "True", NULL, 0 },
{ "dif #617",
  "post(2 + (3 << 1))\n"
  "\n",
  "8", NULL, 0 },
{ "dif #618",
  "post(2 + 3 * 4)\n"
  "\n",
  "14", NULL, 0 },
{ "dif #619",
  "post(2 + 3 << 1)\n"
  "\n",
  "10", NULL, 0 },
{ "dif #620",
  "post(2 == 2)\n"
  "\n",
  "True", NULL, 0 },
{ "dif #621",
  "post(2 == true)\n"
  "\n",
  "False", NULL, 0 },
{ "dif #622",
  "post(2 in [1,2])\n"
  "\n",
  "True", NULL, 0 },
{ "dif #623",
  "post(256 >> 4)\n"
  "\n",
  "16", NULL, 0 },
{ "dif #624",
  "post(3 < 5)\n"
  "\n",
  "True", NULL, 0 },
{ "dif #625",
  "post(3 > 5)\n"
  "\n",
  "False", NULL, 0 },
{ "dif #626",
  "post(3 and 5 or 7)\n"
  "\n",
  "True", NULL, 0 },
{ "dif #627",
  "post(3 and 5)\n"
  "\n",
  "True", NULL, 0 },
{ "dif #628",
  "post(3 in [1, 2, 3], \"a\" in {\"a\": 1}, 5 is int)\n"
  "\n",
  "True True True", NULL, 0 },
{ "dif #629",
  "post(3 or 5)\n"
  "\n",
  "True", NULL, 0 },
{ "dif #630",
  "post(42)\n"
  "\n",
  "42", NULL, 0 },
{ "dif #631",
  "post(5 % 3.0)\n"
  "\n",
  "2.0", NULL, 0 },
{ "dif #632",
  "post(5 <= 5)\n"
  "\n",
  "True", NULL, 0 },
{ "dif #633",
  "post(5 >= 5)\n"
  "\n",
  "True", NULL, 0 },
{ "dif #634",
  "post(5 ^ 3)\n"
  "\n",
  "6", NULL, 0 },
{ "dif #635",
  "post(5 in [1,2])\n"
  "\n",
  "False", NULL, 0 },
{ "dif #636",
  "post(5 is type)\n"
  "\n",
  "False", NULL, 0 },
{ "dif #637",
  "post(5 | 2)\n"
  "\n",
  "7", NULL, 0 },
{ "dif #638",
  "post(5.0 % 3)\n"
  "\n",
  "2.0", NULL, 0 },
{ "dif #639",
  "post(6 & 3)\n"
  "\n",
  "2", NULL, 0 },
{ "dif #640",
  "post(7 % 3)\n"
  "\n",
  "1", NULL, 0 },
{ "dif #641",
  "post(7 + 3, 7 - 3, 7 * 3, 7 / 2, 7 % 3)\n"
  "\n",
  "10 4 21 3.5 1", NULL, 0 },
{ "dif #642",
  "post(7 / 2)\n"
  "\n",
  "3.5", NULL, 0 },
{ "dif #643",
  "post(7.5 % 2)\n"
  "\n",
  "1.5", NULL, 0 },
{ "dif #644",
  "post(8 & 4 ^ 2 | 1)\n"
  "\n",
  "3", NULL, 0 },
{ "dif #645",
  "post(9223372036854775807 * 2 - 1)\n"
  "\n",
  "18446744073709551613", NULL, 0 },
{ "dif #646",
  "post(9223372036854775807 + 1)\n"
  "\n",
  "9223372036854775808", NULL, 0 },
{ "dif #647",
  "post(99999999999999999999999999999999999999)\n"
  "\n",
  "99999999999999999999999999999999999999", NULL, 0 },
{ "dif #648",
  "post(<\n"
  "\n",
  NULL, "SyntaxError: expressao invalida", -1 },
{ "dif #649",
  "post(<2196f3>\"azul\")\n"
  "\n",
  "[38;2;33;150;243mazul[0m", NULL, 0 },
{ "dif #650",
  "post(<F00>\"vermelho\")\n"
  "\n",
  "[38;2;255;0;0mvermelho[0m", NULL, 0 },
{ "dif #651",
  "post(<FF0000>\"vermelho\")\n"
  "\n",
  "[38;2;255;0;0mvermelho[0m", NULL, 0 },
{ "dif #652",
  "post(<blue>[1, 2])\n"
  "\n",
  "[38;2;33;150;243m[1, 2][0m", NULL, 0 },
{ "dif #653",
  "post(<fff>\"branco\")\n"
  "\n",
  "[38;2;255;255;255mbranco[0m", NULL, 0 },
{ "dif #654",
  "post(<green>x)\n"
  "\n",
  NULL, "NameError: name 'x' is not defined", -1 },
{ "dif #655",
  "post(<red>\"alerta\")\n"
  "\n",
  "[38;2;255;59;48malerta[0m", NULL, 0 },
{ "dif #656",
  "post(<red>\"ola\")\n"
  "\n",
  "[38;2;255;59;48mola[0m", NULL, 0 },
{ "dif #657",
  "post(<red>\"txt\")\n"
  "\n",
  "[38;2;255;59;48mtxt[0m", NULL, 0 },
{ "dif #658",
  "post(<red>\"vermelho\")\n"
  "\n",
  "[38;2;255;59;48mvermelho[0m", NULL, 0 },
{ "dif #659",
  "post(<red>\"x\")\n"
  "\n",
  "[38;2;255;59;48mx[0m", NULL, 0 },
{ "dif #660",
  "post(<red>f)\n"
  "\n",
  NULL, "NameError: name 'f' is not defined", -1 },
{ "dif #661",
  "post(A().n)\n"
  "\n",
  NULL, "NameError: name 'A' is not defined", -1 },
{ "dif #662",
  "post(Aberta(7).ver())\n"
  "\n",
  NULL, "NameError: name 'Aberta' is not defined", -1 },
{ "dif #663",
  "post(B().n)\n"
  "\n",
  NULL, "NameError: name 'B' is not defined", -1 },
{ "dif #664",
  "post(C().campo)\n"
  "\n",
  NULL, "NameError: name 'C' is not defined", -1 },
{ "dif #665",
  "post(C().f(5))\n"
  "\n",
  NULL, "NameError: name 'C' is not defined", -1 },
{ "dif #666",
  "post(C().f(a=7))\n"
  "\n",
  NULL, "NameError: name 'C' is not defined", -1 },
{ "dif #667",
  "post(C().m())\n"
  "\n",
  NULL, "NameError: name 'C' is not defined", -1 },
{ "dif #668",
  "post(C().m(5))\n"
  "\n",
  NULL, "NameError: name 'C' is not defined", -1 },
{ "dif #669",
  "post(C.f())\n"
  "\n",
  NULL, "NameError: name 'C' is not defined", -1 },
{ "dif #670",
  "post(C.f(1, 2, 3))\n"
  "\n",
  NULL, "NameError: name 'C' is not defined", -1 },
{ "dif #671",
  "post(C.f(5))\n"
  "\n",
  NULL, "NameError: name 'C' is not defined", -1 },
{ "dif #672",
  "post(C.f(5, 20))\n"
  "\n",
  NULL, "NameError: name 'C' is not defined", -1 },
{ "dif #673",
  "post(C.f(a=7))\n"
  "\n",
  NULL, "NameError: name 'C' is not defined", -1 },
{ "dif #674",
  "post(C.f(a=7, b=1))\n"
  "\n",
  NULL, "NameError: name 'C' is not defined", -1 },
{ "dif #675",
  "post(C.m())\n"
  "\n",
  NULL, "NameError: name 'C' is not defined", -1 },
{ "dif #676",
  "post(C.m(5))\n"
  "\n",
  NULL, "NameError: name 'C' is not defined", -1 },
{ "dif #677",
  "post(C.naoexiste())\n"
  "\n",
  NULL, "NameError: name 'C' is not defined", -1 },
{ "dif #678",
  "post(Cor)\n"
  "\n",
  NULL, "NameError: name 'Cor' is not defined", -1 },
{ "dif #679",
  "post(Cor.BLUE)\n"
  "\n",
  NULL, "NameError: name 'Cor' is not defined", -1 },
{ "dif #680",
  "post(Cor.RED)\n"
  "\n",
  NULL, "NameError: name 'Cor' is not defined", -1 },
{ "dif #681",
  "post(Cor.ROXO)\n"
  "\n",
  NULL, "NameError: name 'Cor' is not defined", -1 },
{ "dif #682",
  "post(Cor.type())\n"
  "\n",
  NULL, "NameError: name 'Cor' is not defined", -1 },
{ "dif #683",
  "post(Hex.RED)\n"
  "\n",
  NULL, "NameError: name 'Hex' is not defined", -1 },
{ "dif #684",
  "post(M().f())\n"
  "\n",
  NULL, "NameError: name 'M' is not defined", -1 },
{ "dif #685",
  "post(MAX)\n"
  "\n",
  NULL, "NameError: name 'MAX' is not defined", -1 },
{ "dif #686",
  "post(Mix.A)\n"
  "\n",
  NULL, "NameError: name 'Mix' is not defined", -1 },
{ "dif #687",
  "post(Mix.C)\n"
  "\n",
  NULL, "NameError: name 'Mix' is not defined", -1 },
{ "dif #688",
  "post(Mix.E)\n"
  "\n",
  NULL, "NameError: name 'Mix' is not defined", -1 },
{ "dif #689",
  "post(NOME)\n"
  "\n",
  NULL, "NameError: name 'NOME' is not defined", -1 },
{ "dif #690",
  "post(Null < 0)\n"
  "\n",
  NULL, "TypeError: '<' not supported between instances of 'Null' and 'int'", -1 },
{ "dif #691",
  "post(Null <= 0)\n"
  "\n",
  NULL, "TypeError: '<=' not supported between instances of 'Null' and 'int'", -1 },
{ "dif #692",
  "post(Null == 0)\n"
  "\n",
  "False", NULL, 0 },
{ "dif #693",
  "post(Null == Null)\n"
  "\n",
  "True", NULL, 0 },
{ "dif #694",
  "post(Null > \"a\")\n"
  "\n",
  NULL, "TypeError: '>' not supported between instances of 'Null' and 'str'", -1 },
{ "dif #695",
  "post(Null > 0)\n"
  "\n",
  NULL, "TypeError: '>' not supported between instances of 'Null' and 'int'", -1 },
{ "dif #696",
  "post(Null >= 0)\n"
  "\n",
  NULL, "TypeError: '>=' not supported between instances of 'Null' and 'int'", -1 },
{ "dif #697",
  "post(Null >= Null)\n"
  "\n",
  NULL, "TypeError: '>=' not supported between instances of 'Null' and 'Null'", -1 },
{ "dif #698",
  "post(Null)\n"
  "\n",
  "null", NULL, 0 },
{ "dif #699",
  "post(Null.keys())\n"
  "\n",
  NULL, "AttributeError: 'Null' object has no attribute 'keys'", -1 },
{ "dif #700",
  "post(Null.type())\n"
  "\n",
  "Null", NULL, 0 },
{ "dif #701",
  "post(P().n)\n"
  "\n",
  NULL, "NameError: name 'P' is not defined", -1 },
{ "dif #702",
  "post(Parsing)\n"
  "\n",
  "<Parsing>", NULL, 0 },
{ "dif #703",
  "post(Parsing.Arrayformatt(\"ab\"))\n"
  "\n",
  "['a', 'b']", NULL, 0 },
{ "dif #704",
  "post(Parsing.Arrayformatt(\"ção\"))\n"
  "\n",
  "['ç', 'ã', 'o']", NULL, 0 },
{ "dif #705",
  "post(Parsing.Arrayformatt((1,2)))\n"
  "\n",
  "[1, 2]", NULL, 0 },
{ "dif #706",
  "post(Parsing.Arrayformatt(5))\n"
  "\n",
  "[5]", NULL, 0 },
{ "dif #707",
  "post(Parsing.Arrayformatt([1]))\n"
  "\n",
  "[1]", NULL, 0 },
{ "dif #708",
  "post(Parsing.Arrayformatt({\"a\":1}))\n"
  "\n",
  "['a']", NULL, 0 },
{ "dif #709",
  "post(Parsing.JSONformatt(\"[1,2]\"))\n"
  "\n",
  "[1, 2]", NULL, 0 },
{ "dif #710",
  "post(Parsing.JSONformatt(\"lixo\"))\n"
  "\n",
  "{}", NULL, 0 },
{ "dif #711",
  "post(Parsing.JSONformatt(5))\n"
  "\n",
  "{}", NULL, 0 },
{ "dif #712",
  "post(Parsing.JSONformatt({\"a\":1}))\n"
  "\n",
  "{'a': 1}", NULL, 0 },
{ "dif #713",
  "post(Parsing.TransientValue(\"123.7\", \"int\"))\n"
  "\n",
  "1237", NULL, 0 },
{ "dif #714",
  "post(Parsing.TransientValue(\"7\",\"flo\"))\n"
  "\n",
  "7.0", NULL, 0 },
{ "dif #715",
  "post(Parsing.TransientValue(\"7\",\"int\"))\n"
  "\n",
  "7", NULL, 0 },
{ "dif #716",
  "post(Parsing.TransientValue(7,\"str\"))\n"
  "\n",
  "7", NULL, 0 },
{ "dif #717",
  "post(Parsing.Tuplasformatt(\"ab\"))\n"
  "\n",
  "('a', 'b')", NULL, 0 },
{ "dif #718",
  "post(Parsing.Tuplasformatt(5))\n"
  "\n",
  "(5,)", NULL, 0 },
{ "dif #719",
  "post(Parsing.Tuplasformatt([1,2]))\n"
  "\n",
  "(1, 2)", NULL, 0 },
{ "dif #720",
  "post(Parsing.boolean(\"\"))\n"
  "\n",
  "False", NULL, 0 },
{ "dif #721",
  "post(Parsing.boolean(\"0\"))\n"
  "\n",
  "False", NULL, 0 },
{ "dif #722",
  "post(Parsing.boolean(\"FALSE\"))\n"
  "\n",
  "False", NULL, 0 },
{ "dif #723",
  "post(Parsing.boolean(\"false\"))\n"
  "\n",
  "False", NULL, 0 },
{ "dif #724",
  "post(Parsing.boolean(\"null\"))\n"
  "\n",
  "False", NULL, 0 },
{ "dif #725",
  "post(Parsing.boolean(\"x\"))\n"
  "\n",
  "True", NULL, 0 },
{ "dif #726",
  "post(Parsing.boolean(1))\n"
  "\n",
  "True", NULL, 0 },
{ "dif #727",
  "post(Parsing.boolean([1]))\n"
  "\n",
  "True", NULL, 0 },
{ "dif #728",
  "post(Parsing.boolean([]))\n"
  "\n",
  "False", NULL, 0 },
{ "dif #729",
  "post(Parsing.floating(\"\"))\n"
  "\n",
  "0.0", NULL, 0 },
{ "dif #730",
  "post(Parsing.floating(\"1,5\"))\n"
  "\n",
  "1.5", NULL, 0 },
{ "dif #731",
  "post(Parsing.floating(\"1.299,90\"))\n"
  "\n",
  "1299.9", NULL, 0 },
{ "dif #732",
  "post(Parsing.floating(\"12.5\"))\n"
  "\n",
  "12.5", NULL, 0 },
{ "dif #733",
  "post(Parsing.floating(3))\n"
  "\n",
  "3.0", NULL, 0 },
{ "dif #734",
  "post(Parsing.integer(\"\"))\n"
  "\n",
  "0", NULL, 0 },
{ "dif #735",
  "post(Parsing.integer(\"10\") + Parsing.integer(\"5\"))\n"
  "\n",
  "15", NULL, 0 },
{ "dif #736",
  "post(Parsing.integer(\"R$ 1.299\"))\n"
  "\n",
  "1299", NULL, 0 },
{ "dif #737",
  "post(Parsing.integer(\"R$ 1.299,90\"))\n"
  "\n",
  "129990", NULL, 0 },
{ "dif #738",
  "post(Parsing.integer(\"abc\"))\n"
  "\n",
  "0", NULL, 0 },
{ "dif #739",
  "post(Parsing.integer(123.7))\n"
  "\n",
  "123", NULL, 0 },
{ "dif #740",
  "post(Parsing.integer(5))\n"
  "\n",
  "5", NULL, 0 },
{ "dif #741",
  "post(Parsing.string(\"  a   b  \"))\n"
  "\n",
  "a b", NULL, 0 },
{ "dif #742",
  "post(Parsing.string(\"  a   b  c \"))\n"
  "\n",
  "a b c", NULL, 0 },
{ "dif #743",
  "post(Parsing.string(\"  ana  \", to str))\n"
  "\n",
  "ana", NULL, 0 },
{ "dif #744",
  "post(Parsing.string(\"R$ 1.299,90\", \"flo\"))\n"
  "\n",
  "129990.0", NULL, 0 },
{ "dif #745",
  "post(Parsing.string(\"R$ 1.299,90\", \"int\"))\n"
  "\n",
  "129990", NULL, 0 },
{ "dif #746",
  "post(Parsing.string(\"a9\", \"flo\"))\n"
  "\n",
  "9.0", NULL, 0 },
{ "dif #747",
  "post(Parsing.string(\"abc123\", \"int\"))\n"
  "\n",
  "123", NULL, 0 },
{ "dif #748",
  "post(PoolFile)\n"
  "\n",
  "PoolFile", NULL, 0 },
{ "dif #749",
  "post(R())\n"
  "\n",
  NULL, "NameError: name 'R' is not defined", -1 },
{ "dif #750",
  "post(Secreta())\n"
  "\n",
  NULL, "NameError: name 'Secreta' is not defined", -1 },
{ "dif #751",
  "post(True & 1)\n"
  "\n",
  "1", NULL, 0 },
{ "dif #752",
  "post(True ^ False)\n"
  "\n",
  "1", NULL, 0 },
{ "dif #753",
  "post(True.isdigit())\n"
  "\n",
  NULL, "AttributeError: 'bool' object has no attribute 'isdigit'", -1 },
{ "dif #754",
  "post(True.type())\n"
  "\n",
  "bool", NULL, 0 },
{ "dif #755",
  "post(U(\"ana\", 30))\n"
  "\n",
  NULL, "NameError: name 'U' is not defined", -1 },
{ "dif #756",
  "post([\"a\", \"b\"])\n"
  "\n",
  "['a', 'b']", NULL, 0 },
{ "dif #757",
  "post([\"a\".encode()])\n"
  "\n",
  "[b'a']", NULL, 0 },
{ "dif #758",
  "post([\"a\"].contains(\"a\"))\n"
  "\n",
  "True", NULL, 0 },
{ "dif #759",
  "post([1, \"dois\", 3.5, True])\n"
  "\n",
  "[1, 'dois', 3.5, True]", NULL, 0 },
{ "dif #760",
  "post([1, 2, 3, 4, 5][1:4])\n"
  "post(\"abcdef\"[2:])\n"
  "\n",
  "[2, 3, 4]\ncdef", NULL, 0 },
{ "dif #761",
  "post([1, 2, 3])\n"
  "\n",
  "[1, 2, 3]", NULL, 0 },
{ "dif #762",
  "post([1, 2, 3])\n"
  "post({\"a\": 1, \"b\": 2})\n"
  "post((1, 2, 3))\n"
  "\n",
  "[1, 2, 3]\n{'a': 1, 'b': 2}\n(1, 2, 3)", NULL, 0 },
{ "dif #763",
  "post([1, 2].index(9))\n"
  "\n",
  NULL, "ValueError: valor invalido: index() nao achou o item", -1 },
{ "dif #764",
  "post([1, 2][\"x\"])\n"
  "\n",
  NULL, "TypeError: list indices must be integers or slices, not str", -1 },
{ "dif #765",
  "post([1, 2][9])\n"
  "\n",
  NULL, "IndexError: list index out of range", -1 },
{ "dif #766",
  "post([1, 2][zzz])\n"
  "\n",
  NULL, "NameError: name 'zzz' is not defined", -1 },
{ "dif #767",
  "post([1, zzz])\n"
  "\n",
  NULL, "NameError: name 'zzz' is not defined", -1 },
{ "dif #768",
  "post([1,2,3][-1])\n"
  "\n",
  "3", NULL, 0 },
{ "dif #769",
  "post([1,2,3][1])\n"
  "\n",
  "2", NULL, 0 },
{ "dif #770",
  "post([1,2,3][::0])\n"
  "\n",
  NULL, "ValueError: slice step cannot be zero", -1 },
{ "dif #771",
  "post([1,2].contains(2))\n"
  "\n",
  "True", NULL, 0 },
{ "dif #772",
  "post([1,2].has(9))\n"
  "\n",
  "False", NULL, 0 },
{ "dif #773",
  "post([1,2].len())\n"
  "\n",
  "2", NULL, 0 },
{ "dif #774",
  "post([1,2].type())\n"
  "\n",
  "list", NULL, 0 },
{ "dif #775",
  "post([1,2][-1])\n"
  "\n",
  "2", NULL, 0 },
{ "dif #776",
  "post([1,2][-9])\n"
  "\n",
  NULL, "IndexError: list index out of range", -1 },
{ "dif #777",
  "post([1,2][9])\n"
  "\n",
  NULL, "IndexError: list index out of range", -1 },
{ "dif #778",
  "post([1] < [2], [1,2] >= [1,2], (1,2) < (1,3))\n"
  "\n",
  "True True True", NULL, 0 },
{ "dif #779",
  "post([1] is tup)\n"
  "\n",
  "False", NULL, 0 },
{ "dif #780",
  "post([1].append())\n"
  "\n",
  NULL, "TypeError: append expected 1 argument, got 0", -1 },
{ "dif #781",
  "post([1].extend(1))\n"
  "\n",
  NULL, "TypeError: 'int' object is not iterable", -1 },
{ "dif #782",
  "post([1].index(9))\n"
  "\n",
  NULL, "ValueError: valor invalido: index() nao achou o item", -1 },
{ "dif #783",
  "post([1].insert(\"a\",1))\n"
  "\n",
  NULL, "TypeError: 'str' object cannot be interpreted as an integer", -1 },
{ "dif #784",
  "post([1].keys())\n"
  "\n",
  NULL, "AttributeError: 'list' object has no attribute 'keys'", -1 },
{ "dif #785",
  "post([1].pop(9))\n"
  "\n",
  NULL, "IndexError: indice fora do intervalo em pop()", -1 },
{ "dif #786",
  "post([1].remove(9))\n"
  "\n",
  NULL, "ValueError: valor invalido: remove() nao achou o item", -1 },
{ "dif #787",
  "post([1].sort(1))\n"
  "\n",
  NULL, "TypeError: sort expected 0 arguments, got 1", -1 },
{ "dif #788",
  "post([1].type(1))\n"
  "\n",
  NULL, "TypeError: type expected 0 arguments, got 1", -1 },
{ "dif #789",
  "post([1].upper())\n"
  "\n",
  NULL, "AttributeError: 'list' object has no attribute 'upper'", -1 },
{ "dif #790",
  "post([3,1,2])\n"
  "\n",
  "[3, 1, 2]", NULL, 0 },
{ "dif #791",
  "post([Null])\n"
  "\n",
  "[null]", NULL, 0 },
{ "dif #792",
  "post([[1, 2], [3, 4]])\n"
  "\n",
  "[[1, 2], [3, 4]]", NULL, 0 },
{ "dif #793",
  "post([[1,2],[3,4]][1][0])\n"
  "\n",
  "3", NULL, 0 },
{ "dif #794",
  "post([[Null]])\n"
  "\n",
  "[[null]]", NULL, 0 },
{ "dif #795",
  "post([] and 1)\n"
  "\n",
  "False", NULL, 0 },
{ "dif #796",
  "post([])\n"
  "\n",
  "[]", NULL, 0 },
{ "dif #797",
  "post([].pop())\n"
  "\n",
  NULL, "IndexError: pop() de lista vazia", -1 },
{ "dif #798",
  "post([str, int])\n"
  "\n",
  "[str, int]", NULL, 0 },
{ "dif #799",
  "post(a % 7)\n"
  "\n",
  NULL, "NameError: name 'a' is not defined", -1 },
{ "dif #800",
  "post(a / 0)\n"
  "\n",
  NULL, "NameError: name 'a' is not defined", -1 },
{ "dif #801",
  "post(a == a)\n"
  "\n",
  NULL, "NameError: name 'a' is not defined", -1 },
{ "dif #802",
  "post(a == b)\n"
  "\n",
  NULL, "NameError: name 'a' is not defined", -1 },
{ "dif #803",
  "post(a > 999999999999999999)\n"
  "\n",
  NULL, "NameError: name 'a' is not defined", -1 },
{ "dif #804",
  "post(a is str and not (a is int))\n"
  "\n",
  NULL, "NameError: name 'a' is not defined", -1 },
{ "dif #805",
  "post(a(10), b(10))\n"
  "\n",
  NULL, "NameError: name 'a' is not defined", -1 },
{ "dif #806",
  "post(a)\n"
  "\n",
  NULL, "NameError: name 'a' is not defined", -1 },
{ "dif #807",
  "post(a, b)\n"
  "\n",
  NULL, "NameError: name 'a' is not defined", -1 },
{ "dif #808",
  "post(a, b, c)\n"
  "\n",
  NULL, "NameError: name 'a' is not defined", -1 },
{ "dif #809",
  "post(a, r)\n"
  "\n",
  NULL, "NameError: name 'a' is not defined", -1 },
{ "dif #810",
  "post(a.b.c)\n"
  "\n",
  NULL, "NameError: name 'a' is not defined", -1 },
{ "dif #811",
  "post(a.b[0].c)\n"
  "\n",
  NULL, "NameError: name 'a' is not defined", -1 },
{ "dif #812",
  "post(abs(\"a\"))\n"
  "\n",
  NULL, "TypeError: bad operand type for abs(): 'str'", -1 },
{ "dif #813",
  "post(abs(-3))\n"
  "\n",
  "3", NULL, 0 },
{ "dif #814",
  "post(abs(-3.5))\n"
  "\n",
  "3.5", NULL, 0 },
{ "dif #815",
  "post(abs(-42))\n"
  "\n",
  "42", NULL, 0 },
{ "dif #816",
  "post(abs(-9223372036854775808))\n"
  "\n",
  "9223372036854775808", NULL, 0 },
{ "dif #817",
  "post(abs(0))\n"
  "\n",
  "0", NULL, 0 },
{ "dif #818",
  "post(abs(3))\n"
  "\n",
  "3", NULL, 0 },
{ "dif #819",
  "post(abs(True))\n"
  "\n",
  "1", NULL, 0 },
{ "dif #820",
  "post(achou)\n"
  "\n",
  NULL, "NameError: name 'achou' is not defined", -1 },
{ "dif #821",
  "post(addEnd(1,2))\n"
  "\n",
  NULL, "TypeError: addEnd() argument 1 must be list, not int", -1 },
{ "dif #822",
  "post(app.POOLHTMLElements.getitemByIdentify(\"email\").value == \"\")\n"
  "\n",
  NULL, "NameError: name 'app' is not defined", -1 },
{ "dif #823",
  "post(app.POOLHTMLElements.getitemByIdentify(\"email\").value)\n"
  "\n",
  NULL, "NameError: name 'app' is not defined", -1 },
{ "dif #824",
  "post(app.POOLHTMLElements.getitemByIdentify(\"nome\").value)\n"
  "\n",
  NULL, "NameError: name 'app' is not defined", -1 },
{ "dif #825",
  "post(app.POOLHTMLElements.getitemByIdentify(\"x\") == null)\n"
  "\n",
  NULL, "NameError: name 'app' is not defined", -1 },
{ "dif #826",
  "post(app.div().stylesheet({\"width\": \"300\"}).text(\"x\") is null == false)\n"
  "\n",
  NULL, "NameError: name 'app' is not defined", -1 },
{ "dif #827",
  "post(await \"texto\")\n"
  "\n",
  "texto", NULL, 0 },
{ "dif #828",
  "post(await 42)\n"
  "\n",
  "42", NULL, 0 },
{ "dif #829",
  "post(await Null)\n"
  "\n",
  "null", NULL, 0 },
{ "dif #830",
  "post(b % 26)\n"
  "\n",
  NULL, "NameError: name 'b' is not defined", -1 },
{ "dif #831",
  "post(b(1))\n"
  "\n",
  NULL, "NameError: name 'b' is not defined", -1 },
{ "dif #832",
  "post(b)\n"
  "\n",
  NULL, "NameError: name 'b' is not defined", -1 },
{ "dif #833",
  "post(b[0], b[1], b[2], b[3])\n"
  "\n",
  NULL, "NameError: name 'b' is not defined", -1 },
{ "dif #834",
  "post(big * big)\n"
  "\n",
  NULL, "NameError: name 'big' is not defined", -1 },
{ "dif #835",
  "post(big + 1)\n"
  "\n",
  NULL, "NameError: name 'big' is not defined", -1 },
{ "dif #836",
  "post(bin(\"a\"))\n"
  "\n",
  NULL, "TypeError: operacao invalida: bin() so aceita int", -1 },
{ "dif #837",
  "post(bin(-5))\n"
  "\n",
  "-0b101", NULL, 0 },
{ "dif #838",
  "post(bin(0))\n"
  "\n",
  "0b0", NULL, 0 },
{ "dif #839",
  "post(bin(10))\n"
  "\n",
  "0b1010", NULL, 0 },
{ "dif #840",
  "post(bool(\"\"))\n"
  "\n",
  "False", NULL, 0 },
{ "dif #841",
  "post(bool(\"false\"))\n"
  "\n",
  "True", NULL, 0 },
{ "dif #842",
  "post(bool(0))\n"
  "\n",
  "False", NULL, 0 },
{ "dif #843",
  "post(bool(1))\n"
  "\n",
  "True", NULL, 0 },
{ "dif #844",
  "post(bool(Null))\n"
  "\n",
  "False", NULL, 0 },
{ "dif #845",
  "post(bool([1]))\n"
  "\n",
  "True", NULL, 0 },
{ "dif #846",
  "post(bool([]))\n"
  "\n",
  "False", NULL, 0 },
{ "dif #847",
  "post(bool({}))\n"
  "\n",
  "False", NULL, 0 },
{ "dif #848",
  "post(bytes.base64(bytes.new(\"Hello\")))\n"
  "\n",
  NULL, "NameError: name 'bytes' is not defined", -1 },
{ "dif #849",
  "post(bytes.concat([bytes.new(\"Hi\"), bytes.new(\"!!\")]))\n"
  "\n",
  NULL, "NameError: name 'bytes' is not defined", -1 },
{ "dif #850",
  "post(bytes.frombase64(\"SGVsbG8=\"))\n"
  "\n",
  NULL, "NameError: name 'bytes' is not defined", -1 },
{ "dif #851",
  "post(bytes.get(bytes.new(\"ABC\"), -1))\n"
  "\n",
  NULL, "NameError: name 'bytes' is not defined", -1 },
{ "dif #852",
  "post(bytes.hex(bytes.fromhex(\"48 65 6c 6c 6f\")))\n"
  "\n",
  NULL, "NameError: name 'bytes' is not defined", -1 },
{ "dif #853",
  "post(bytes.hex(bytes.fromint(258, 4)))\n"
  "\n",
  NULL, "NameError: name 'bytes' is not defined", -1 },
{ "dif #854",
  "post(bytes.hex(bytes.fromint(258, 4, \"little\")))\n"
  "\n",
  NULL, "NameError: name 'bytes' is not defined", -1 },
{ "dif #855",
  "post(bytes.hex(bytes.xor(bytes.xor(bytes.new(\"secret\"), bytes.new(\"KEY\")), bytes.new(\"KEY\"))))\n"
  "\n",
  NULL, "NameError: name 'bytes' is not defined", -1 },
{ "dif #856",
  "post(bytes.new(\"Oi\"))\n"
  "\n",
  NULL, "NameError: name 'bytes' is not defined", -1 },
{ "dif #857",
  "post(bytes.new())\n"
  "\n",
  NULL, "NameError: name 'bytes' is not defined", -1 },
{ "dif #858",
  "post(bytes.new(3))\n"
  "\n",
  NULL, "NameError: name 'bytes' is not defined", -1 },
{ "dif #859",
  "post(bytes.new(3.5))\n"
  "\n",
  NULL, "NameError: name 'bytes' is not defined", -1 },
{ "dif #860",
  "post(bytes.new([72, 105]))\n"
  "\n",
  NULL, "NameError: name 'bytes' is not defined", -1 },
{ "dif #861",
  "post(bytes.slice(bytes.new(\"Hello\"), 1, 3))\n"
  "\n",
  NULL, "NameError: name 'bytes' is not defined", -1 },
{ "dif #862",
  "post(bytes.toint(bytes.fromint(70000, 4)))\n"
  "\n",
  NULL, "NameError: name 'bytes' is not defined", -1 },
{ "dif #863",
  "post(bytes.tolist(bytes.new(\"ABC\")))\n"
  "\n",
  NULL, "NameError: name 'bytes' is not defined", -1 },
{ "dif #864",
  "post(c)\n"
  "\n",
  NULL, "NameError: name 'c' is not defined", -1 },
{ "dif #865",
  "post(c.cursor().execute(\"CREATE TABLE a (x)\"))\n"
  "\n",
  NULL, "NameError: name 'c' is not defined", -1 },
{ "dif #866",
  "post(c.cursor().execute(\"SELECT nota FROM u ORDER BY id\").fetchall())\n"
  "\n",
  NULL, "NameError: name 'c' is not defined", -1 },
{ "dif #867",
  "post(c.execute(\"SELECT * FROM x\").fetchall())\n"
  "\n",
  NULL, "NameError: name 'c' is not defined", -1 },
{ "dif #868",
  "post(c.execute(\"SELECT 1 + 1 AS soma, ?\", (\"oi\",)).fetchall())\n"
  "\n",
  NULL, "NameError: name 'c' is not defined", -1 },
{ "dif #869",
  "post(c2.cursor().execute(\"SELECT * FROM u\").fetchall())\n"
  "\n",
  NULL, "NameError: name 'c2' is not defined", -1 },
{ "dif #870",
  "post(c2.cursor().execute(\"SELECT count(*) AS n FROM u WHERE nome = '__rb__'\").fetchall())\n"
  "\n",
  NULL, "NameError: name 'c2' is not defined", -1 },
{ "dif #871",
  "post(c2.execute(\"SELECT * FROM t\").fetchall())\n"
  "\n",
  NULL, "NameError: name 'c2' is not defined", -1 },
{ "dif #872",
  "post(c2.execute(\"SELECT * FROM z\").fetchall())\n"
  "\n",
  NULL, "NameError: name 'c2' is not defined", -1 },
{ "dif #873",
  "post(chamadas)\n"
  "\n",
  NULL, "NameError: name 'chamadas' is not defined", -1 },
{ "dif #874",
  "post(check(crypt(\"z\"), \"z\"))\n"
  "\n",
  NULL, "NameError: name 'check' is not defined", -1 },
{ "dif #875",
  "post(chr(-1))\n"
  "\n",
  NULL, "ValueError: chr() arg not in range(0x110000)", -1 },
{ "dif #876",
  "post(chr(1114112))\n"
  "\n",
  NULL, "ValueError: chr() arg not in range(0x110000)", -1 },
{ "dif #877",
  "post(chr(128512))\n"
  "\n",
  "😀", NULL, 0 },
{ "dif #878",
  "post(chr(65))\n"
  "\n",
  "A", NULL, 0 },
{ "dif #879",
  "post(chr(955))\n"
  "\n",
  "λ", NULL, 0 },
{ "dif #880",
  "post(chr(false) == chr(0))\n"
  "\n",
  "True", NULL, 0 },
{ "dif #881",
  "post(classifica(7))\n"
  "\n",
  NULL, "NameError: name 'classifica' is not defined", -1 },
{ "dif #882",
  "post(col.count())\n"
  "\n",
  NULL, "NameError: name 'col' is not defined", -1 },
{ "dif #883",
  "post(col.find({\"nome\":\"ana\"}))\n"
  "\n",
  NULL, "NameError: name 'col' is not defined", -1 },
{ "dif #884",
  "post(col.find_one({\"nome\":\"ana\"}))\n"
  "\n",
  NULL, "NameError: name 'col' is not defined", -1 },
{ "dif #885",
  "post(col.find_one({\"nome\":\"leo\"}))\n"
  "\n",
  NULL, "NameError: name 'col' is not defined", -1 },
{ "dif #886",
  "post(col.find_one({\"nome\":\"zzz\"}))\n"
  "\n",
  NULL, "NameError: name 'col' is not defined", -1 },
{ "dif #887",
  "post(count char in s)\n"
  "\n",
  NULL, "NameError: name 's' is not defined", -1 },
{ "dif #888",
  "post(count dict in [{}, {}, 1])\n"
  "\n",
  "2", NULL, 0 },
{ "dif #889",
  "post(count int in 5)\n"
  "\n",
  "1", NULL, 0 },
{ "dif #890",
  "post(count int in 5.5)\n"
  "\n",
  "0", NULL, 0 },
{ "dif #891",
  "post(count int in 555)\n"
  "\n",
  "3", NULL, 0 },
{ "dif #892",
  "post(count int in Null)\n"
  "\n",
  "0", NULL, 0 },
{ "dif #893",
  "post(count int in True)\n"
  "\n",
  "0", NULL, 0 },
{ "dif #894",
  "post(count int in [1, \"a\", 2, \"b\", 3])\n"
  "\n",
  "3", NULL, 0 },
{ "dif #895",
  "post(count int in [1,\"a\",2])\n"
  "\n",
  "2", NULL, 0 },
{ "dif #896",
  "post(count int in abs)\n"
  "\n",
  "0", NULL, 0 },
{ "dif #897",
  "post(count int in nums)\n"
  "\n",
  NULL, "NameError: name 'nums' is not defined", -1 },
{ "dif #898",
  "post(count int in post)\n"
  "\n",
  "0", NULL, 0 },
{ "dif #899",
  "post(count int() in nums)\n"
  "\n",
  NULL, "NameError: name 'nums' is not defined", -1 },
{ "dif #900",
  "post(count int(1) in 112211)\n"
  "\n",
  "4", NULL, 0 },
{ "dif #901",
  "post(count int(2) in [1,2,2,3])\n"
  "\n",
  "2", NULL, 0 },
{ "dif #902",
  "post(count int(5) in 5155)\n"
  "\n",
  "3", NULL, 0 },
{ "dif #903",
  "post(count int(7) in nums)\n"
  "\n",
  NULL, "NameError: name 'nums' is not defined", -1 },
{ "dif #904",
  "post(count json in [{}, 1])\n"
  "\n",
  "1", NULL, 0 },
{ "dif #905",
  "post(count str in \"banana\")\n"
  "\n",
  "6", NULL, 0 },
{ "dif #906",
  "post(count str in Null)\n"
  "\n",
  "0", NULL, 0 },
{ "dif #907",
  "post(count str(\"\") in \"ab\")\n"
  "\n",
  "0", NULL, 0 },
{ "dif #908",
  "post(count str(\"a\") in \"banana\")\n"
  "\n",
  "3", NULL, 0 },
{ "dif #909",
  "post(count str(\"aa\") in \"aaaa\")\n"
  "\n",
  "3", NULL, 0 },
{ "dif #910",
  "post(count str(\"ana\") in \"ana banana\")\n"
  "\n",
  "3", NULL, 0 },
{ "dif #911",
  "post(count str(\"oi\") in frase)\n"
  "\n",
  NULL, "NameError: name 'frase' is not defined", -1 },
{ "dif #912",
  "post(count str(\"z\") in \"banana\")\n"
  "\n",
  "0", NULL, 0 },
{ "dif #913",
  "post(count str(\"ção\") in \"ação canção\")\n"
  "\n",
  "2", NULL, 0 },
{ "dif #914",
  "post(count tup in [(1,), 1])\n"
  "\n",
  "1", NULL, 0 },
{ "dif #915",
  "post(d is dict)\n"
  "\n",
  NULL, "NameError: name 'd' is not defined", -1 },
{ "dif #916",
  "post(d is json)\n"
  "\n",
  NULL, "NameError: name 'd' is not defined", -1 },
{ "dif #917",
  "post(d is tup)\n"
  "\n",
  NULL, "NameError: name 'd' is not defined", -1 },
{ "dif #918",
  "post(d not is tup)\n"
  "\n",
  NULL, "NameError: name 'd' is not defined", -1 },
{ "dif #919",
  "post(d)\n"
  "\n",
  NULL, "NameError: name 'd' is not defined", -1 },
{ "dif #920",
  "post(d.get(\"nome\"))\n"
  "\n",
  NULL, "NameError: name 'd' is not defined", -1 },
{ "dif #921",
  "post(d.n)\n"
  "\n",
  NULL, "NameError: name 'd' is not defined", -1 },
{ "dif #922",
  "post(d.nome)\n"
  "\n",
  NULL, "NameError: name 'd' is not defined", -1 },
{ "dif #923",
  "post(d[\"nome\"])\n"
  "\n",
  NULL, "NameError: name 'd' is not defined", -1 },
{ "dif #924",
  "post(d[\"tag\"], d[\"attrs\"][\"a\"], d[\"children\"].len())\n"
  "\n",
  NULL, "NameError: name 'd' is not defined", -1 },
{ "dif #925",
  "post(d[0][\"id\"], d[0][\"nota\"], d[0][\"nome\"])\n"
  "\n",
  NULL, "NameError: name 'd' is not defined", -1 },
{ "dif #926",
  "post(d[0][\"idade\"], d[0][\"nota\"], d[1][\"nota\"])\n"
  "\n",
  NULL, "NameError: name 'd' is not defined", -1 },
{ "dif #927",
  "post(dentro)\n"
  "\n",
  NULL, "NameError: name 'dentro' is not defined", -1 },
{ "dif #928",
  "post(dotenv.load())\n"
  "\n",
  NULL, "NameError: name 'dotenv' is not defined", -1 },
{ "dif #929",
  "post(e)\n"
  "\n",
  NULL, "NameError: name 'e' is not defined", -1 },
{ "dif #930",
  "post(ef.f())\n"
  "\n",
  NULL, "NameError: name 'ef' is not defined", -1 },
{ "dif #931",
  "post(enumerate(\"çá\"))\n"
  "\n",
  "[(0, 'ç'), (1, 'á')]", NULL, 0 },
{ "dif #932",
  "post(enumerate(1))\n"
  "\n",
  NULL, "TypeError: 'int' object is not iterable", -1 },
{ "dif #933",
  "post(enumerate([\"a\",\"b\"]))\n"
  "\n",
  "[(0, 'a'), (1, 'b')]", NULL, 0 },
{ "dif #934",
  "post(enumerate([10, 20]))\n"
  "\n",
  "[(0, 10), (1, 20)]", NULL, 0 },
{ "dif #935",
  "post(enumerate([]))\n"
  "\n",
  "[]", NULL, 0 },
{ "dif #936",
  "post(f is PoolFile)\n"
  "\n",
  NULL, "NameError: name 'f' is not defined", -1 },
{ "dif #937",
  "post(f\"\")\n"
  "\n",
  "", NULL, 0 },
{ "dif #938",
  "post(f\"Ola, {nome}!\")\n"
  "\n",
  NULL, "NameError: name 'nome' is not defined", -1 },
{ "dif #939",
  "post(f\"oi {x}\")\n"
  "\n",
  NULL, "NameError: name 'x' is not defined", -1 },
{ "dif #940",
  "post(f\"texto puro\")\n"
  "\n",
  "texto puro", NULL, 0 },
{ "dif #941",
  "post(f\"v: {zzz + 1}\")\n"
  "\n",
  NULL, "NameError: name 'zzz' is not defined", -1 },
{ "dif #942",
  "post(f\"v: {zzz}\")\n"
  "\n",
  NULL, "NameError: name 'zzz' is not defined", -1 },
{ "dif #943",
  "post(f\"x {1/0} y\")\n"
  "\n",
  NULL, "ZeroDivisionError: division by zero", -1 },
{ "dif #944",
  "post(f\"{a} {a + 1}\")\n"
  "\n",
  NULL, "NameError: name 'a' is not defined", -1 },
{ "dif #945",
  "post(f\"{a} {b} {a+1}\")\n"
  "\n",
  NULL, "NameError: name 'a' is not defined", -1 },
{ "dif #946",
  "post(f\"{{literal}} e {1 + 1}\")\n"
  "\n",
  "{literal} e 2", NULL, 0 },
{ "dif #947",
  "post(f\"{{literal}}\")\n"
  "\n",
  "{literal}", NULL, 0 },
{ "dif #948",
  "post(f(\"5\"))\n"
  "\n",
  NULL, "NameError: name 'f' is not defined", -1 },
{ "dif #949",
  "post(f())\n"
  "\n",
  NULL, "NameError: name 'f' is not defined", -1 },
{ "dif #950",
  "post(f(1))\n"
  "\n",
  NULL, "NameError: name 'f' is not defined", -1 },
{ "dif #951",
  "post(f(1, 2))\n"
  "\n",
  NULL, "NameError: name 'f' is not defined", -1 },
{ "dif #952",
  "post(f(3))\n"
  "\n",
  NULL, "NameError: name 'f' is not defined", -1 },
{ "dif #953",
  "post(f(Cor.RED))\n"
  "\n",
  NULL, "NameError: name 'f' is not defined", -1 },
{ "dif #954",
  "post(f(a=1, zzz=2))\n"
  "\n",
  NULL, "NameError: name 'f' is not defined", -1 },
{ "dif #955",
  "post(f.delete(), os.exists(\"img.png\"))\n"
  "\n",
  NULL, "NameError: name 'f' is not defined", -1 },
{ "dif #956",
  "post(f.name, f.ext)\n"
  "\n",
  NULL, "NameError: name 'f' is not defined", -1 },
{ "dif #957",
  "post(f.name, f.ext, f.size)\n"
  "\n",
  NULL, "NameError: name 'f' is not defined", -1 },
{ "dif #958",
  "post(f.name, f.size, f is PoolFile)\n"
  "\n",
  NULL, "NameError: name 'f' is not defined", -1 },
{ "dif #959",
  "post(f.name.upper())\n"
  "\n",
  NULL, "NameError: name 'f' is not defined", -1 },
{ "dif #960",
  "post(f.naoexiste)\n"
  "\n",
  NULL, "NameError: name 'f' is not defined", -1 },
{ "dif #961",
  "post(f.path().endswith(\"img.png\"))\n"
  "\n",
  NULL, "NameError: name 'f' is not defined", -1 },
{ "dif #962",
  "post(f.size + 1)\n"
  "\n",
  NULL, "NameError: name 'f' is not defined", -1 },
{ "dif #963",
  "post(f.size > 0)\n"
  "\n",
  NULL, "NameError: name 'f' is not defined", -1 },
{ "dif #964",
  "post(false < true)\n"
  "\n",
  "True", NULL, 0 },
{ "dif #965",
  "post(false and f())\n"
  "\n",
  "False", NULL, 0 },
{ "dif #966",
  "post(filter([0,1,2,0], bool))\n"
  "\n",
  "[1, 2]", NULL, 0 },
{ "dif #967",
  "post(filter([1, 2, 3], action(x) { return x > 1 }))\n"
  "\n",
  "[2, 3]", NULL, 0 },
{ "dif #968",
  "post(filter([1], 5))\n"
  "\n",
  NULL, "TypeError: tentativa de chamar algo que nao e funcao", -1 },
{ "dif #969",
  "post(flo(\"1.5\"))\n"
  "\n",
  "1.5", NULL, 0 },
{ "dif #970",
  "post(flo(\"1.5abc\"))\n"
  "\n",
  NULL, "ValueError: could not convert string to flo: '1.5abc'", -1 },
{ "dif #971",
  "post(flo(\"abc\"))\n"
  "\n",
  NULL, "ValueError: could not convert string to flo: 'abc'", -1 },
{ "dif #972",
  "post(flo(1.5))\n"
  "\n",
  "1.5", NULL, 0 },
{ "dif #973",
  "post(flo(2))\n"
  "\n",
  "2.0", NULL, 0 },
{ "dif #974",
  "post(flo(Null))\n"
  "\n",
  NULL, "TypeError: flo() argument must be a string or a real number, not 'Null'", -1 },
{ "dif #975",
  "post(flo(True))\n"
  "\n",
  "1.0", NULL, 0 },
{ "dif #976",
  "post(fv)\n"
  "\n",
  NULL, "NameError: name 'fv' is not defined", -1 },
{ "dif #977",
  "post(g(\"42\") + 1)\n"
  "\n",
  NULL, "NameError: name 'g' is not defined", -1 },
{ "dif #978",
  "post(g())\n"
  "\n",
  NULL, "NameError: name 'g' is not defined", -1 },
{ "dif #979",
  "post(g.name, os.exists(\"dup.png\"), os.exists(\"img.png\"))\n"
  "\n",
  NULL, "NameError: name 'g' is not defined", -1 },
{ "dif #980",
  "post(g.name, os.exists(\"img.png\"), g.size)\n"
  "\n",
  NULL, "NameError: name 'g' is not defined", -1 },
{ "dif #981",
  "post(g.name, os.exists(\"outro.png\"), os.exists(\"img.png\"))\n"
  "\n",
  NULL, "NameError: name 'g' is not defined", -1 },
{ "dif #982",
  "post(g.name, os.exists(\"sub/nova/c.png\"), os.exists(\"img.png\"))\n"
  "\n",
  NULL, "NameError: name 'g' is not defined", -1 },
{ "dif #983",
  "post(g.size, f.size, len(g.bytes()))\n"
  "\n",
  NULL, "NameError: name 'g' is not defined", -1 },
{ "dif #984",
  "post(getenv(\"POOL_TEST_VAR\"))\n"
  "\n",
  NULL, "NameError: name 'getenv' is not defined", -1 },
{ "dif #985",
  "post(getenv(\"POOL_TEST_VAR\"), cwd() is str)\n"
  "\n",
  NULL, "NameError: name 'getenv' is not defined", -1 },
{ "dif #986",
  "post(glob.oi())\n"
  "\n",
  NULL, "NameError: name 'glob' is not defined", -1 },
{ "dif #987",
  "post(hash.b64decode(\"!!!\"))\n"
  "\n",
  NULL, "NameError: name 'hash' is not defined", -1 },
{ "dif #988",
  "post(hash.b64decode(\"UG9vbFNjcmlwdA==\"))\n"
  "\n",
  NULL, "NameError: name 'hash' is not defined", -1 },
{ "dif #989",
  "post(hash.b64encode(\"PoolScript\"))\n"
  "\n",
  NULL, "NameError: name 'hash' is not defined", -1 },
{ "dif #990",
  "post(hash.b64encode(\"ção\"))\n"
  "\n",
  NULL, "NameError: name 'hash' is not defined", -1 },
{ "dif #991",
  "post(hash.check(\"\n"
  "\n",
  NULL, "SyntaxError: string nao fechada antes da quebra de linha", -1 },
{ "dif #992",
  "post(hash.check(\"\", \"abc\"))\n"
  "\n",
  NULL, "NameError: name 'hash' is not defined", -1 },
{ "dif #993",
  "post(hash.check(\"lixo\", \"abc\"))\n"
  "\n",
  NULL, "NameError: name 'hash' is not defined", -1 },
{ "dif #994",
  "post(hash.check(123, \"abc\"))\n"
  "\n",
  NULL, "NameError: name 'hash' is not defined", -1 },
{ "dif #995",
  "post(hash.check(a, \"x\"))\n"
  "\n",
  NULL, "NameError: name 'hash' is not defined", -1 },
{ "dif #996",
  "post(hash.check(b, \"x\"))\n"
  "\n",
  NULL, "NameError: name 'hash' is not defined", -1 },
{ "dif #997",
  "post(hash.check(h, \"abc\"))\n"
  "\n",
  NULL, "NameError: name 'hash' is not defined", -1 },
{ "dif #998",
  "post(hash.check(h, \"abd\"))\n"
  "\n",
  NULL, "NameError: name 'hash' is not defined", -1 },
{ "dif #999",
  "post(hash.crypt(\"minhasenha\"))\n"
  "\n",
  NULL, "NameError: name 'hash' is not defined", -1 },
{ "dif #1000",
  "post(hash.sha256(\"\"))\n"
  "\n",
  NULL, "NameError: name 'hash' is not defined", -1 },
{ "dif #1001",
  "post(hash.sha256(\"abc\"))\n"
  "\n",
  NULL, "NameError: name 'hash' is not defined", -1 },
{ "dif #1002",
  "post(hex(-255))\n"
  "\n",
  "-0xff", NULL, 0 },
{ "dif #1003",
  "post(hex(0))\n"
  "\n",
  "0x0", NULL, 0 },
{ "dif #1004",
  "post(hex(1.5))\n"
  "\n",
  NULL, "TypeError: operacao invalida: hex() so aceita int", -1 },
{ "dif #1005",
  "post(hex(255))\n"
  "\n",
  "0xff", NULL, 0 },
{ "dif #1006",
  "post(i)\n"
  "\n",
  NULL, "NameError: name 'i' is not defined", -1 },
{ "dif #1007",
  "post(int is int)\n"
  "\n",
  "True", NULL, 0 },
{ "dif #1008",
  "post(int is str)\n"
  "\n",
  "False", NULL, 0 },
{ "dif #1009",
  "post(int(\"  7 \"))\n"
  "\n",
  "7", NULL, 0 },
{ "dif #1010",
  "post(int(\"\"))\n"
  "\n",
  NULL, "ValueError: invalid literal for int() with base 10: ''", -1 },
{ "dif #1011",
  "post(int(\"+5\"))\n"
  "\n",
  "5", NULL, 0 },
{ "dif #1012",
  "post(int(\"-5\"))\n"
  "\n",
  "-5", NULL, 0 },
{ "dif #1013",
  "post(int(\"0x1f\"))\n"
  "\n",
  NULL, "ValueError: invalid literal for int() with base 10: '0x1f'", -1 },
{ "dif #1014",
  "post(int(\"12345678901234567890123456789012\"))\n"
  "\n",
  "12345678901234567890123456789012", NULL, 0 },
{ "dif #1015",
  "post(int(\"42\"))\n"
  "\n",
  "42", NULL, 0 },
{ "dif #1016",
  "post(int(\"42\"), int(\"-7\"), int(\" 8 \"))\n"
  "\n",
  "42 -7 8", NULL, 0 },
{ "dif #1017",
  "post(int(\"a\"))\n"
  "\n",
  NULL, "ValueError: invalid literal for int() with base 10: 'a'", -1 },
{ "dif #1018",
  "post(int(\"abc\"))\n"
  "\n",
  NULL, "ValueError: invalid literal for int() with base 10: 'abc'", -1 },
{ "dif #1019",
  "post(int(\"x\"))\n"
  "\n",
  NULL, "ValueError: invalid literal for int() with base 10: 'x'", -1 },
{ "dif #1020",
  "post(int())\n"
  "\n",
  "0", NULL, 0 },
{ "dif #1021",
  "post(int(-3.9))\n"
  "\n",
  "-3", NULL, 0 },
{ "dif #1022",
  "post(int(1))\n"
  "\n",
  "1", NULL, 0 },
{ "dif #1023",
  "post(int(3.9))\n"
  "\n",
  "3", NULL, 0 },
{ "dif #1024",
  "post(int(Null))\n"
  "\n",
  NULL, "TypeError: int() argument must be a string, a bytes-like object or a real number, not 'Null'", -1 },
{ "dif #1025",
  "post(int(True))\n"
  "\n",
  "1", NULL, 0 },
{ "dif #1026",
  "post(int(null))\n"
  "\n",
  NULL, "TypeError: int() argument must be a string, a bytes-like object or a real number, not 'Null'", -1 },
{ "dif #1027",
  "post(ip.count(\".\") == 3)\n"
  "\n",
  NULL, "NameError: name 'ip' is not defined", -1 },
{ "dif #1028",
  "post(j)\n"
  "\n",
  NULL, "NameError: name 'j' is not defined", -1 },
{ "dif #1029",
  "post(json is dict)\n"
  "\n",
  "True", NULL, 0 },
{ "dif #1030",
  "post(json.parse(\"\n"
  "\n",
  NULL, "SyntaxError: string nao fechada antes da quebra de linha", -1 },
{ "dif #1031",
  "post(json.parse(text=\"[1,2]\"))\n"
  "\n",
  NULL, "NameError: name 'json' is not defined", -1 },
{ "dif #1032",
  "post(json.stringify([1]))\n"
  "\n",
  NULL, "NameError: name 'json' is not defined", -1 },
{ "dif #1033",
  "post(json.stringify({\"a\":1,\"b\":[2,3]}))\n"
  "\n",
  NULL, "NameError: name 'json' is not defined", -1 },
{ "dif #1034",
  "post(json.stringify({\"a\":1,\"b\":[2,3]}, True))\n"
  "\n",
  NULL, "NameError: name 'json' is not defined", -1 },
{ "dif #1035",
  "post(jwt.check(\"\n"
  "\n",
  NULL, "SyntaxError: string nao fechada antes da quebra de linha", -1 },
{ "dif #1036",
  "post(jwt.check(\"a.b\", \"k\"))\n"
  "\n",
  NULL, "NameError: name 'jwt' is not defined", -1 },
{ "dif #1037",
  "post(jwt.check(\"a.b.c\", \"k\"))\n"
  "\n",
  NULL, "NameError: name 'jwt' is not defined", -1 },
{ "dif #1038",
  "post(jwt.check(\"a.b.c.d\", \"k\"))\n"
  "\n",
  NULL, "NameError: name 'jwt' is not defined", -1 },
{ "dif #1039",
  "post(jwt.check(\"abc\", \"k\"))\n"
  "\n",
  NULL, "NameError: name 'jwt' is not defined", -1 },
{ "dif #1040",
  "post(jwt.check(1, \"k\"))\n"
  "\n",
  NULL, "NameError: name 'jwt' is not defined", -1 },
{ "dif #1041",
  "post(jwt.check(v, \"k\") is Null)\n"
  "\n",
  NULL, "NameError: name 'jwt' is not defined", -1 },
{ "dif #1042",
  "post(jwt.check(x, \"k\") is Null)\n"
  "\n",
  NULL, "NameError: name 'jwt' is not defined", -1 },
{ "dif #1043",
  "post(jwt.gen({\"a\":1}, \"k\"))\n"
  "\n",
  NULL, "NameError: name 'jwt' is not defined", -1 },
{ "dif #1044",
  "post(jwt.gen({\"a\":1}, \"k\", \"\n"
  "\n",
  NULL, "SyntaxError: string nao fechada antes da quebra de linha", -1 },
{ "dif #1045",
  "post(jwt.gen({\"id\": 9}, \"s\"))\n"
  "\n",
  NULL, "NameError: name 'jwt' is not defined", -1 },
{ "dif #1046",
  "post(jwt.gen({\"u\": 9}, \"s\", \"\n"
  "\n",
  NULL, "SyntaxError: string nao fechada antes da quebra de linha", -1 },
{ "dif #1047",
  "post(k)\n"
  "\n",
  NULL, "NameError: name 'k' is not defined", -1 },
{ "dif #1048",
  "post(k.execute(\"PRAGMA user_version\"))\n"
  "\n",
  NULL, "NameError: name 'k' is not defined", -1 },
{ "dif #1049",
  "post(k.execute(\"SELECT * FROM m\").fetchall())\n"
  "\n",
  NULL, "NameError: name 'k' is not defined", -1 },
{ "dif #1050",
  "post(k.execute(\"SELECT * FROM n\").fetchall())\n"
  "\n",
  NULL, "NameError: name 'k' is not defined", -1 },
{ "dif #1051",
  "post(k.execute(\"SELECT * FROM p\").fetchall())\n"
  "\n",
  NULL, "NameError: name 'k' is not defined", -1 },
{ "dif #1052",
  "post(k.execute(\"SELECT * FROM r\").fetchall())\n"
  "\n",
  NULL, "NameError: name 'k' is not defined", -1 },
{ "dif #1053",
  "post(k.execute(\"SELECT * FROM u ORDER BY id\").fetchall())\n"
  "\n",
  NULL, "NameError: name 'k' is not defined", -1 },
{ "dif #1054",
  "post(k.execute(\"SELECT * FROM u WHERE id = 99\").fetchone())\n"
  "\n",
  NULL, "NameError: name 'k' is not defined", -1 },
{ "dif #1055",
  "post(k.execute(\"SELECT * FROM u WHERE id=99\").fetchone())\n"
  "\n",
  NULL, "NameError: name 'k' is not defined", -1 },
{ "dif #1056",
  "post(k.execute(\"SELECT * FROM u\").fetchall())\n"
  "\n",
  NULL, "NameError: name 'k' is not defined", -1 },
{ "dif #1057",
  "post(k.execute(\"SELECT * FROM u\").fetchmany(1))\n"
  "\n",
  NULL, "NameError: name 'k' is not defined", -1 },
{ "dif #1058",
  "post(k.execute(\"SELECT d FROM b\").fetchone())\n"
  "\n",
  NULL, "NameError: name 'k' is not defined", -1 },
{ "dif #1059",
  "post(k.execute(\"SELECT nome FROM u WHERE id = ?\", (2,)).fetchone())\n"
  "\n",
  NULL, "NameError: name 'k' is not defined", -1 },
{ "dif #1060",
  "post(k.execute(\"SELECT nome FROM u WHERE nota > %s\", (8,)).fetchall())\n"
  "\n",
  NULL, "NameError: name 'k' is not defined", -1 },
{ "dif #1061",
  "post(k.execute(\"SELECT nome FROM u\").fetchone())\n"
  "\n",
  NULL, "NameError: name 'k' is not defined", -1 },
{ "dif #1062",
  "post(k.fetchall())\n"
  "\n",
  NULL, "NameError: name 'k' is not defined", -1 },
{ "dif #1063",
  "post(k.fetchmany(9))\n"
  "\n",
  NULL, "NameError: name 'k' is not defined", -1 },
{ "dif #1064",
  "post(k.fetchone())\n"
  "\n",
  NULL, "NameError: name 'k' is not defined", -1 },
{ "dif #1065",
  "post(k.lastrowid)\n"
  "\n",
  NULL, "NameError: name 'k' is not defined", -1 },
{ "dif #1066",
  "post(k.rowcount)\n"
  "\n",
  NULL, "NameError: name 'k' is not defined", -1 },
{ "dif #1067",
  "post(k.rowcount, k.lastrowid)\n"
  "\n",
  NULL, "NameError: name 'k' is not defined", -1 },
{ "dif #1068",
  "post(l is dict)\n"
  "\n",
  NULL, "NameError: name 'l' is not defined", -1 },
{ "dif #1069",
  "post(l)\n"
  "\n",
  NULL, "NameError: name 'l' is not defined", -1 },
{ "dif #1070",
  "post(l[0])\n"
  "\n",
  NULL, "NameError: name 'l' is not defined", -1 },
{ "dif #1071",
  "post(l[0].keys())\n"
  "\n",
  NULL, "NameError: name 'l' is not defined", -1 },
{ "dif #1072",
  "post(l[0][1])\n"
  "\n",
  NULL, "NameError: name 'l' is not defined", -1 },
{ "dif #1073",
  "post(len(\"ab\".encode()))\n"
  "\n",
  "2", NULL, 0 },
{ "dif #1074",
  "post(len(\"olá\"), len([1, 2]))\n"
  "\n",
  "3 2", NULL, 0 },
{ "dif #1075",
  "post(len(\"pool\"))\n"
  "\n",
  "4", NULL, 0 },
{ "dif #1076",
  "post(len(\"poolscript\"))\n"
  "\n",
  "10", NULL, 0 },
{ "dif #1077",
  "post(len((1,2,3)))\n"
  "\n",
  "3", NULL, 0 },
{ "dif #1078",
  "post(len(1))\n"
  "\n",
  NULL, "TypeError: object of type 'int' has no len()", -1 },
{ "dif #1079",
  "post(len(5))\n"
  "\n",
  NULL, "TypeError: object of type 'int' has no len()", -1 },
{ "dif #1080",
  "post(len([1, 2, 3, 4]))\n"
  "\n",
  "4", NULL, 0 },
{ "dif #1081",
  "post(len([1, 2, 3]), len(\"abc\"), str(5), int(\"7\"), flo(\"2.5\"), bool(1))\n"
  "\n",
  "3 3 5 7 2.5 True", NULL, 0 },
{ "dif #1082",
  "post(len([1,2,3]))\n"
  "\n",
  "3", NULL, 0 },
{ "dif #1083",
  "post(len([]))\n"
  "\n",
  "0", NULL, 0 },
{ "dif #1084",
  "post(len(d), d[1])\n"
  "\n",
  NULL, "NameError: name 'd' is not defined", -1 },
{ "dif #1085",
  "post(len(d2))\n"
  "\n",
  NULL, "NameError: name 'd2' is not defined", -1 },
{ "dif #1086",
  "post(len(f.bytes()), f.bytes()[0])\n"
  "\n",
  NULL, "NameError: name 'f' is not defined", -1 },
{ "dif #1087",
  "post(len(null))\n"
  "\n",
  "0", NULL, 0 },
{ "dif #1088",
  "post(len(os.environ()) > 3)\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1089",
  "post(len(os.ls(\".\")) > 0)\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1090",
  "post(len(range(100)))\n"
  "\n",
  "100", NULL, 0 },
{ "dif #1091",
  "post(len(sys.argv))\n"
  "\n",
  NULL, "NameError: name 'sys' is not defined", -1 },
{ "dif #1092",
  "post(len(x=[1,2]))\n"
  "\n",
  NULL, "TypeError: esta funcao nao aceita argumento nomeado", -1 },
{ "dif #1093",
  "post(len(zzz))\n"
  "\n",
  NULL, "NameError: name 'zzz' is not defined", -1 },
{ "dif #1094",
  "post(len({\"a\": 1, \"b\": 2, \"c\": 3}))\n"
  "\n",
  "3", NULL, 0 },
{ "dif #1095",
  "post(len({\"a\":1}))\n"
  "\n",
  "1", NULL, 0 },
{ "dif #1096",
  "post(ler(\"POOL_TEST_VAR\"))\n"
  "\n",
  NULL, "NameError: name 'ler' is not defined", -1 },
{ "dif #1097",
  "post(lib.rotulo(7))\n"
  "\n",
  NULL, "NameError: name 'lib' is not defined", -1 },
{ "dif #1098",
  "post(list is list)\n"
  "\n",
  "True", NULL, 0 },
{ "dif #1099",
  "post(list(\"\"))\n"
  "\n",
  "[]", NULL, 0 },
{ "dif #1100",
  "post(list(\"ab\"))\n"
  "\n",
  "['a', 'b']", NULL, 0 },
{ "dif #1101",
  "post(list(\"ção\"))\n"
  "\n",
  "['ç', 'ã', 'o']", NULL, 0 },
{ "dif #1102",
  "post(list((1,2)))\n"
  "\n",
  "[1, 2]", NULL, 0 },
{ "dif #1103",
  "post(list())\n"
  "\n",
  "[]", NULL, 0 },
{ "dif #1104",
  "post(list(1))\n"
  "\n",
  NULL, "TypeError: 'int' object is not iterable", -1 },
{ "dif #1105",
  "post(list([1,2]))\n"
  "\n",
  "[1, 2]", NULL, 0 },
{ "dif #1106",
  "post(list(b))\n"
  "\n",
  NULL, "NameError: name 'b' is not defined", -1 },
{ "dif #1107",
  "post(list(g()))\n"
  "\n",
  NULL, "NameError: name 'g' is not defined", -1 },
{ "dif #1108",
  "post(list(lib.conta(3)))\n"
  "\n",
  NULL, "NameError: name 'lib' is not defined", -1 },
{ "dif #1109",
  "post(list(zip(\"çã\", [1,2])))\n"
  "\n",
  "[('ç', 1), ('ã', 2)]", NULL, 0 },
{ "dif #1110",
  "post(list({\"a\":1}))\n"
  "\n",
  "['a']", NULL, 0 },
{ "dif #1111",
  "post(list({\"b\":1,\"a\":2}))\n"
  "\n",
  "['b', 'a']", NULL, 0 },
{ "dif #1112",
  "post(m.from_address(\"a@b.c\").to(\"d@e.f\").subject(\"s\") == m)\n"
  "\n",
  NULL, "NameError: name 'm' is not defined", -1 },
{ "dif #1113",
  "post(m.get_as_string())\n"
  "\n",
  NULL, "NameError: name 'm' is not defined", -1 },
{ "dif #1114",
  "post(m.q())\n"
  "\n",
  NULL, "NameError: name 'm' is not defined", -1 },
{ "dif #1115",
  "post(map(1, str))\n"
  "\n",
  NULL, "TypeError: 'int' object is not iterable", -1 },
{ "dif #1116",
  "post(map([\"1\",\"2\"], int))\n"
  "\n",
  "[1, 2]", NULL, 0 },
{ "dif #1117",
  "post(map([1, 2, 3], action(x) { return x * 2 }))\n"
  "\n",
  "[2, 4, 6]", NULL, 0 },
{ "dif #1118",
  "post(map([1, 2, 3], action(x) { return x * 2 }))\n"
  "post(filter([1, 2, 3, 4], action(x) { return x % 2 == 0 }))\n"
  "\n",
  "[2, 4, 6]\n[2, 4]", NULL, 0 },
{ "dif #1119",
  "post(map([1,2,3], str))\n"
  "\n",
  "['1', '2', '3']", NULL, 0 },
{ "dif #1120",
  "post(map([1,2], flo))\n"
  "\n",
  "[1.0, 2.0]", NULL, 0 },
{ "dif #1121",
  "post(map([1]))\n"
  "\n",
  NULL, "TypeError: map expected 2 arguments, got 1", -1 },
{ "dif #1122",
  "post(map([1], 5))\n"
  "\n",
  NULL, "TypeError: tentativa de chamar algo que nao e funcao", -1 },
{ "dif #1123",
  "post(mat.dobro(21))\n"
  "\n",
  NULL, "NameError: name 'mat' is not defined", -1 },
{ "dif #1124",
  "post(max(\"a\",\"b\"))\n"
  "\n",
  "b", NULL, 0 },
{ "dif #1125",
  "post(max(\"ab\"))\n"
  "\n",
  "b", NULL, 0 },
{ "dif #1126",
  "post(max(1,2,3))\n"
  "\n",
  "3", NULL, 0 },
{ "dif #1127",
  "post(max([3,1,2]))\n"
  "\n",
  "3", NULL, 0 },
{ "dif #1128",
  "post(max([5,1,3]))\n"
  "\n",
  "5", NULL, 0 },
{ "dif #1129",
  "post(max([]))\n"
  "\n",
  NULL, "ValueError: max() iterable argument is empty", -1 },
{ "dif #1130",
  "post(min(\"çab\"), max(\"çab\"))\n"
  "\n",
  "a ç", NULL, 0 },
{ "dif #1131",
  "post(min(3,1))\n"
  "\n",
  "1", NULL, 0 },
{ "dif #1132",
  "post(min([\"b\",\"a\"]))\n"
  "\n",
  "a", NULL, 0 },
{ "dif #1133",
  "post(min([1,2],[3]))\n"
  "\n",
  "[1, 2]", NULL, 0 },
{ "dif #1134",
  "post(min([1.5,2]))\n"
  "\n",
  "1.5", NULL, 0 },
{ "dif #1135",
  "post(min([3, 1, 2]), max(5, 2, 8))\n"
  "\n",
  "1 8", NULL, 0 },
{ "dif #1136",
  "post(min([3,1,2]))\n"
  "\n",
  "1", NULL, 0 },
{ "dif #1137",
  "post(min([5,1,3]))\n"
  "\n",
  "1", NULL, 0 },
{ "dif #1138",
  "post(min([]))\n"
  "\n",
  NULL, "ValueError: min() iterable argument is empty", -1 },
{ "dif #1139",
  "post(min([], default=0))\n"
  "\n",
  NULL, "TypeError: esta funcao nao aceita argumento nomeado", -1 },
{ "dif #1140",
  "post(minhalib.quem())\n"
  "\n",
  NULL, "NameError: name 'minhalib' is not defined", -1 },
{ "dif #1141",
  "post(not \"\")\n"
  "\n",
  "True", NULL, 0 },
{ "dif #1142",
  "post(not (1 and 0))\n"
  "\n",
  "True", NULL, 0 },
{ "dif #1143",
  "post(not 0)\n"
  "\n",
  "True", NULL, 0 },
{ "dif #1144",
  "post(not 3)\n"
  "\n",
  "False", NULL, 0 },
{ "dif #1145",
  "post(not zzz)\n"
  "\n",
  NULL, "NameError: name 'zzz' is not defined", -1 },
{ "dif #1146",
  "post(o.base)\n"
  "\n",
  NULL, "NameError: name 'o' is not defined", -1 },
{ "dif #1147",
  "post(oct(0))\n"
  "\n",
  "0o0", NULL, 0 },
{ "dif #1148",
  "post(oct(15))\n"
  "\n",
  "0o17", NULL, 0 },
{ "dif #1149",
  "post(oct(8))\n"
  "\n",
  "0o10", NULL, 0 },
{ "dif #1150",
  "post(ok())\n"
  "\n",
  NULL, "NameError: name 'ok' is not defined", -1 },
{ "dif #1151",
  "post(ord(\"\"))\n"
  "\n",
  NULL, "TypeError: ord() expected a character, but string of length 0 found", -1 },
{ "dif #1152",
  "post(ord(\"A\"))\n"
  "\n",
  "65", NULL, 0 },
{ "dif #1153",
  "post(ord(\"ab\"))\n"
  "\n",
  NULL, "TypeError: ord() expected a character, but string of length 2 found", -1 },
{ "dif #1154",
  "post(ord(\"ç\"))\n"
  "\n",
  "231", NULL, 0 },
{ "dif #1155",
  "post(ord(\"λ\"))\n"
  "\n",
  "955", NULL, 0 },
{ "dif #1156",
  "post(ord(1))\n"
  "\n",
  NULL, "TypeError: ord() expected string of length 1, but int found", -1 },
{ "dif #1157",
  "post(os.cmd(\"cmd_inexistente_zzz 2>/dev/null\"))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1158",
  "post(os.cmd(\"echo    varios   espacos\", true))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1159",
  "post(os.cmd(\"echo a; echo b\", true))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1160",
  "post(os.cmd(\"echo ola\", true))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1161",
  "post(os.cmd(\"echo silencio\"))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1162",
  "post(os.cmd(\"exit 3\", true))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1163",
  "post(os.cmd(\"ls naoexistezzz 2>&1\", true).contains(\"No such\"))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1164",
  "post(os.cwd() is str)\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1165",
  "post(os.cwd())\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1166",
  "post(os.cwd().endswith(\"caixa\"))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1167",
  "post(os.cwd().endswith(\"sub\"))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1168",
  "post(os.environ(\"PS_NAO_EXISTE_ZZZ\"))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1169",
  "post(os.environ(\"PS_TESTE_X\"))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1170",
  "post(os.environ()[\"PS_TESTE_X\"])\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1171",
  "post(os.exists(\"dados.txt\"))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1172",
  "post(os.exists(\"dados.txt\"), os.exists(\"r.txt\"))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1173",
  "post(os.exists(\"dados.txt\"), os.loadFile(\"m.txt\"))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1174",
  "post(os.exists(\"dentro.txt\"))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1175",
  "post(os.exists(\"nada.txt\"))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1176",
  "post(os.exists(\"sub\"))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1177",
  "post(os.exists(\"sub/fundo\"))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1178",
  "post(os.exists(\"sub/nova/x.png\"))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1179",
  "post(os.exists(\"vazia\"))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1180",
  "post(os.exists(5))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1181",
  "post(os.isdir(\"a/b/c\"))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1182",
  "post(os.isdir(\"nova\"))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1183",
  "post(os.isdir(\"sub\"), os.isfile(\"sub\"))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1184",
  "post(os.isfile(\"dados.txt\"), os.isdir(\"dados.txt\"))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1185",
  "post(os.isfile(\"sub/dentro.txt\"))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1186",
  "post(os.loadFile(\"asp.csv\"))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1187",
  "post(os.loadFile(\"c.txt\"))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1188",
  "post(os.loadFile(\"cfg.json\"))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1189",
  "post(os.loadFile(\"cfg.json\")[\"a\"])\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1190",
  "post(os.loadFile(\"cru.txt\"))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1191",
  "post(os.loadFile(\"dados.txt\") is PoolFile)\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1192",
  "post(os.loadFile(\"dados.txt\") is str)\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1193",
  "post(os.loadFile(\"dados.txt\"))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1194",
  "post(os.loadFile(\"dados.txt\", \"rb\"))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1195",
  "post(os.loadFile(\"dados.txt\", \"utf-8\"))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1196",
  "post(os.loadFile(\"fundo.txt\"))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1197",
  "post(os.loadFile(\"img.png\") is PoolFile)\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1198",
  "post(os.loadFile(\"img.png\") is os.PoolFile)\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1199",
  "post(os.loadFile(\"img.png\", \"rb\").name)\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1200",
  "post(os.loadFile(\"img.png\", \"utf-8\"))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1201",
  "post(os.loadFile(\"nada.txt\"))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1202",
  "post(os.loadFile(\"quebrado.json\"))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1203",
  "post(os.loadFile(\"rag.csv\"))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1204",
  "post(os.loadFile(\"sub/fundo/fundo.txt\"))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1205",
  "post(os.loadFile(\"tab.csv\"))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1206",
  "post(os.loadFile(\"tab.csv\")[0][\"nome\"])\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1207",
  "post(os.ls(\"naoexiste\"))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1208",
  "post(os.ls(\"sub\")[0][\"type\"])\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1209",
  "post(os.mkdir(5))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1210",
  "post(os.pathFile(\"dados.txt\").endswith(\"dados.txt\"))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1211",
  "post(os.pathFile(\"fundo.txt\").endswith(\"fundo.txt\"))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1212",
  "post(os.pathFile(\"naoexiste.zzz\"))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1213",
  "post(os.pathFile(\"sub\"))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1214",
  "post(os.pathFolder(\"fundo\").endswith(\"fundo\"))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1215",
  "post(os.pathFolder(\"naoexiste\"))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1216",
  "post(os.pathFolder(\"sub\").endswith(\"sub\"))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1217",
  "post(os.run(\"echo oi\", true))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1218",
  "post(os.run([\"echo\", \"a b\"], true))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1219",
  "post(os.run([\"echo\", \"a; b | c\"], true))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1220",
  "post(os.run([\"echo\", \"oi\"], true))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1221",
  "post(os.run([\"echo\", \"ok\"], true))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1222",
  "post(os.run([\"sh\", \"-c\", \"echo x\"], true))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1223",
  "post(os.size(\"dados.txt\"))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1224",
  "post(os.size(\"nada.txt\"))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1225",
  "post(p.x + p.y)\n"
  "\n",
  NULL, "NameError: name 'p' is not defined", -1 },
{ "dif #1226",
  "post(p.x)\n"
  "\n",
  NULL, "NameError: name 'p' is not defined", -1 },
{ "dif #1227",
  "post(platform())\n"
  "\n",
  NULL, "NameError: name 'platform' is not defined", -1 },
{ "dif #1228",
  "post(qrcode.ERROR_CORRECT_L, qrcode.ERROR_CORRECT_M, qrcode.ERROR_CORRECT_Q, qrcode.ERROR_CORRECT_H)\n"
  "\n",
  NULL, "NameError: name 'qrcode' is not defined", -1 },
{ "dif #1229",
  "post(quebra())\n"
  "\n",
  NULL, "NameError: name 'quebra' is not defined", -1 },
{ "dif #1230",
  "post(r == true)\n"
  "\n",
  NULL, "NameError: name 'r' is not defined", -1 },
{ "dif #1231",
  "post(r\"C:\\Users\\test\")\n"
  "\n",
  "C:\\Users\\test", NULL, 0 },
{ "dif #1232",
  "post(r)\n"
  "\n",
  NULL, "NameError: name 'r' is not defined", -1 },
{ "dif #1233",
  "post(r, r == true)\n"
  "\n",
  NULL, "NameError: name 'r' is not defined", -1 },
{ "dif #1234",
  "post(r.close())\n"
  "\n",
  NULL, "NameError: name 'r' is not defined", -1 },
{ "dif #1235",
  "post(r.content_type(\"application/json\").status)\n"
  "\n",
  NULL, "NameError: name 'r' is not defined", -1 },
{ "dif #1236",
  "post(r.decode())\n"
  "\n",
  NULL, "NameError: name 'r' is not defined", -1 },
{ "dif #1237",
  "post(r.filename, r.size, type(r.content))\n"
  "\n",
  NULL, "NameError: name 'r' is not defined", -1 },
{ "dif #1238",
  "post(r.get(\"Content-Type\"))\n"
  "\n",
  NULL, "NameError: name 'r' is not defined", -1 },
{ "dif #1239",
  "post(r.get(\"a\"))\n"
  "\n",
  NULL, "NameError: name 'r' is not defined", -1 },
{ "dif #1240",
  "post(r.get_json(\"a\"))\n"
  "\n",
  NULL, "NameError: name 'r' is not defined", -1 },
{ "dif #1241",
  "post(r.get_json(\"b\"))\n"
  "\n",
  NULL, "NameError: name 'r' is not defined", -1 },
{ "dif #1242",
  "post(r.get_json(\"body\"))\n"
  "\n",
  NULL, "NameError: name 'r' is not defined", -1 },
{ "dif #1243",
  "post(r.get_json(\"ctype\"), r.get_json(\"body\"))\n"
  "\n",
  NULL, "NameError: name 'r' is not defined", -1 },
{ "dif #1244",
  "post(r.get_json(\"method\"))\n"
  "\n",
  NULL, "NameError: name 'r' is not defined", -1 },
{ "dif #1245",
  "post(r.get_json(\"ua\"))\n"
  "\n",
  NULL, "NameError: name 'r' is not defined", -1 },
{ "dif #1246",
  "post(r.get_json(\"ua\").contains(\"PoolScript\"))\n"
  "\n",
  NULL, "NameError: name 'r' is not defined", -1 },
{ "dif #1247",
  "post(r.json())\n"
  "\n",
  NULL, "NameError: name 'r' is not defined", -1 },
{ "dif #1248",
  "post(r.size)\n"
  "\n",
  NULL, "NameError: name 'r' is not defined", -1 },
{ "dif #1249",
  "post(r.status == r.status_code)\n"
  "\n",
  NULL, "NameError: name 'r' is not defined", -1 },
{ "dif #1250",
  "post(r.status, r.get_json(\"a\"))\n"
  "\n",
  NULL, "NameError: name 'r' is not defined", -1 },
{ "dif #1251",
  "post(r.status, r.ok)\n"
  "\n",
  NULL, "NameError: name 'r' is not defined", -1 },
{ "dif #1252",
  "post(r.status, r.ok, r.text)\n"
  "\n",
  NULL, "NameError: name 'r' is not defined", -1 },
{ "dif #1253",
  "post(r.status_code)\n"
  "\n",
  NULL, "NameError: name 'r' is not defined", -1 },
{ "dif #1254",
  "post(r.text)\n"
  "\n",
  NULL, "NameError: name 'r' is not defined", -1 },
{ "dif #1255",
  "post(range(\"2\", \"5\"))\n"
  "\n",
  "[2, 3, 4]", NULL, 0 },
{ "dif #1256",
  "post(range(\"3\"))\n"
  "\n",
  "[0, 1, 2]", NULL, 0 },
{ "dif #1257",
  "post(range())\n"
  "\n",
  NULL, "TypeError: range expected at least 1 argument, got 0", -1 },
{ "dif #1258",
  "post(range(-3))\n"
  "\n",
  "[]", NULL, 0 },
{ "dif #1259",
  "post(range(0))\n"
  "\n",
  "[]", NULL, 0 },
{ "dif #1260",
  "post(range(0,10,2))\n"
  "\n",
  "[0, 2, 4, 6, 8]", NULL, 0 },
{ "dif #1261",
  "post(range(1,2,0))\n"
  "\n",
  NULL, "ValueError: range() arg 3 must not be zero", -1 },
{ "dif #1262",
  "post(range(1,4))\n"
  "\n",
  "[1, 2, 3]", NULL, 0 },
{ "dif #1263",
  "post(range(10,0,-3))\n"
  "\n",
  "[10, 7, 4, 1]", NULL, 0 },
{ "dif #1264",
  "post(range(3))\n"
  "\n",
  "[0, 1, 2]", NULL, 0 },
{ "dif #1265",
  "post(range(3), range(1, 7, 2))\n"
  "\n",
  "[0, 1, 2] [1, 3, 5]", NULL, 0 },
{ "dif #1266",
  "post(range(3,0,-1))\n"
  "\n",
  "[3, 2, 1]", NULL, 0 },
{ "dif #1267",
  "post(range(5,1))\n"
  "\n",
  "[]", NULL, 0 },
{ "dif #1268",
  "post(regex.findall(\"\n"
  "\n",
  NULL, "SyntaxError: string nao fechada antes da quebra de linha", -1 },
{ "dif #1269",
  "post(regex.findall(\"(?<=\n"
  "\n",
  NULL, "SyntaxError: string nao fechada antes da quebra de linha", -1 },
{ "dif #1270",
  "post(regex.findall(\"(?<=@)\n"
  "\n",
  NULL, "SyntaxError: string nao fechada antes da quebra de linha", -1 },
{ "dif #1271",
  "post(regex.findall(\"(?P<n>\n"
  "\n",
  NULL, "SyntaxError: string nao fechada antes da quebra de linha", -1 },
{ "dif #1272",
  "post(regex.findall(\"(?i)ab\", \"AB ab Ab aB\"))\n"
  "\n",
  NULL, "NameError: name 'regex' is not defined", -1 },
{ "dif #1273",
  "post(regex.findall(\"(?m)^\n"
  "\n",
  NULL, "SyntaxError: string nao fechada antes da quebra de linha", -1 },
{ "dif #1274",
  "post(regex.findall(\"(a)(b)\n"
  "\n",
  NULL, "SyntaxError: string nao fechada antes da quebra de linha", -1 },
{ "dif #1275",
  "post(regex.match(\"(?>a)b\", \"ab\"))\n"
  "\n",
  NULL, "NameError: name 'regex' is not defined", -1 },
{ "dif #1276",
  "post(regex.match(\"(a+)+b\", \"\n"
  "\n",
  NULL, "SyntaxError: string nao fechada antes da quebra de linha", -1 },
{ "dif #1277",
  "post(regex.match(\"[a\n"
  "\n",
  NULL, "SyntaxError: string nao fechada antes da quebra de linha", -1 },
{ "dif #1278",
  "post(regex.search(\"\n"
  "\n",
  NULL, "SyntaxError: string nao fechada antes da quebra de linha", -1 },
{ "dif #1279",
  "post(regex.search(\"(\n"
  "\n",
  NULL, "SyntaxError: string nao fechada antes da quebra de linha", -1 },
{ "dif #1280",
  "post(regex.search(\"(?#coment)abc\", \"xabc\"))\n"
  "\n",
  NULL, "NameError: name 'regex' is not defined", -1 },
{ "dif #1281",
  "post(regex.search(\"(?<![a-z])cat\", \"bobcat\"))\n"
  "\n",
  NULL, "NameError: name 'regex' is not defined", -1 },
{ "dif #1282",
  "post(regex.search(\"(?i)caf\n"
  "\n",
  NULL, "SyntaxError: string nao fechada antes da quebra de linha", -1 },
{ "dif #1283",
  "post(regex.search(\"(?i:hello) world\", \"HELLO WORLD\"))\n"
  "\n",
  NULL, "NameError: name 'regex' is not defined", -1 },
{ "dif #1284",
  "post(regex.search(\"(?i:hello) world\", \"HELLO world\"))\n"
  "\n",
  NULL, "NameError: name 'regex' is not defined", -1 },
{ "dif #1285",
  "post(regex.search(\"(?s)a.b\", \"a\n"
  "\n",
  NULL, "SyntaxError: string nao fechada antes da quebra de linha", -1 },
{ "dif #1286",
  "post(regex.search(\"end\n"
  "\n",
  NULL, "SyntaxError: string nao fechada antes da quebra de linha", -1 },
{ "dif #1287",
  "post(regex.sub(\"(\n"
  "\n",
  NULL, "SyntaxError: string nao fechada antes da quebra de linha", -1 },
{ "dif #1288",
  "post(regex.sub(\"a\", \"b\", \"aaa\", count=2))\n"
  "\n",
  NULL, "NameError: name 'regex' is not defined", -1 },
{ "dif #1289",
  "post(removeEnd(1))\n"
  "\n",
  NULL, "TypeError: removeEnd() argument 1 must be list, not int", -1 },
{ "dif #1290",
  "post(reversed(\"ab\"))\n"
  "\n",
  "['b', 'a']", NULL, 0 },
{ "dif #1291",
  "post(reversed(\"ção\"))\n"
  "\n",
  "['o', 'ã', 'ç']", NULL, 0 },
{ "dif #1292",
  "post(reversed(1))\n"
  "\n",
  NULL, "TypeError: 'int' object is not reversible", -1 },
{ "dif #1293",
  "post(reversed([1,2,3]))\n"
  "\n",
  "[3, 2, 1]", NULL, 0 },
{ "dif #1294",
  "post(reversed([]))\n"
  "\n",
  "[]", NULL, 0 },
{ "dif #1295",
  "post(round(\"a\"))\n"
  "\n",
  NULL, "TypeError: type str doesn't define __round__ method", -1 },
{ "dif #1296",
  "post(round(-2.5))\n"
  "\n",
  "-2", NULL, 0 },
{ "dif #1297",
  "post(round(0.5))\n"
  "\n",
  "0", NULL, 0 },
{ "dif #1298",
  "post(round(1.5))\n"
  "\n",
  "2", NULL, 0 },
{ "dif #1299",
  "post(round(1234.5, -2))\n"
  "\n",
  "1234.0", NULL, 0 },
{ "dif #1300",
  "post(round(2.5))\n"
  "\n",
  "2", NULL, 0 },
{ "dif #1301",
  "post(round(2.675, 2))\n"
  "\n",
  "2.67", NULL, 0 },
{ "dif #1302",
  "post(round(3))\n"
  "\n",
  "3", NULL, 0 },
{ "dif #1303",
  "post(round(3.14159, 2))\n"
  "\n",
  "3.14", NULL, 0 },
{ "dif #1304",
  "post(round(3.5))\n"
  "\n",
  "4", NULL, 0 },
{ "dif #1305",
  "post(round(3.567, 2), round(2.5))\n"
  "\n",
  "3.57 2", NULL, 0 },
{ "dif #1306",
  "post(round(3.7))\n"
  "\n",
  "4", NULL, 0 },
{ "dif #1307",
  "post(s.get_json(\"x\"))\n"
  "\n",
  NULL, "NameError: name 's' is not defined", -1 },
{ "dif #1308",
  "post(s.get_json())\n"
  "\n",
  NULL, "NameError: name 's' is not defined", -1 },
{ "dif #1309",
  "post(s.quit())\n"
  "\n",
  NULL, "NameError: name 's' is not defined", -1 },
{ "dif #1310",
  "post(s.replace(\"a\", \"b\", count=2))\n"
  "\n",
  NULL, "NameError: name 's' is not defined", -1 },
{ "dif #1311",
  "post(self)\n"
  "\n",
  NULL, "NameError: name 'self' is not defined", -1 },
{ "dif #1312",
  "post(sis.cwd() is str)\n"
  "\n",
  NULL, "NameError: name 'sis' is not defined", -1 },
{ "dif #1313",
  "post(soma(1, 2))\n"
  "\n",
  NULL, "NameError: name 'soma' is not defined", -1 },
{ "dif #1314",
  "post(somax.soma(2, 3))\n"
  "\n",
  NULL, "NameError: name 'somax' is not defined", -1 },
{ "dif #1315",
  "post(sorted(\"bça\"))\n"
  "\n",
  "['a', 'b', 'ç']", NULL, 0 },
{ "dif #1316",
  "post(sorted((3,1)))\n"
  "\n",
  "[1, 3]", NULL, 0 },
{ "dif #1317",
  "post(sorted(1))\n"
  "\n",
  NULL, "TypeError: 'int' object is not iterable", -1 },
{ "dif #1318",
  "post(sorted([\"b\",\"a\"]))\n"
  "\n",
  "['a', 'b']", NULL, 0 },
{ "dif #1319",
  "post(sorted([1,\"a\"]))\n"
  "\n",
  NULL, "TypeError: '<' not supported between instances of 'str' and 'int'", -1 },
{ "dif #1320",
  "post(sorted([2.5,1]))\n"
  "\n",
  "[1, 2.5]", NULL, 0 },
{ "dif #1321",
  "post(sorted([3,1,2]))\n"
  "\n",
  "[1, 2, 3]", NULL, 0 },
{ "dif #1322",
  "post(sorted([]))\n"
  "\n",
  "[]", NULL, 0 },
{ "dif #1323",
  "post(sorted(reversed([1,2,3])))\n"
  "\n",
  "[1, 2, 3]", NULL, 0 },
{ "dif #1324",
  "post(str == int)\n"
  "\n",
  "False", NULL, 0 },
{ "dif #1325",
  "post(str == str)\n"
  "\n",
  "True", NULL, 0 },
{ "dif #1326",
  "post(str is str)\n"
  "\n",
  "True", NULL, 0 },
{ "dif #1327",
  "post(str is type)\n"
  "\n",
  "True", NULL, 0 },
{ "dif #1328",
  "post(str(\"ab\"))\n"
  "\n",
  "ab", NULL, 0 },
{ "dif #1329",
  "post(str((1,2)))\n"
  "\n",
  "(1, 2)", NULL, 0 },
{ "dif #1330",
  "post(str())\n"
  "\n",
  NULL, "TypeError: str expected 1 argument, got 0", -1 },
{ "dif #1331",
  "post(str(1) + \"x\")\n"
  "\n",
  "1x", NULL, 0 },
{ "dif #1332",
  "post(str(1))\n"
  "\n",
  "1", NULL, 0 },
{ "dif #1333",
  "post(str(1,2))\n"
  "\n",
  NULL, "TypeError: str expected 1 argument, got 2", -1 },
{ "dif #1334",
  "post(str(1.5))\n"
  "\n",
  "1.5", NULL, 0 },
{ "dif #1335",
  "post(str(Null))\n"
  "\n",
  "null", NULL, 0 },
{ "dif #1336",
  "post(str(True))\n"
  "\n",
  "True", NULL, 0 },
{ "dif #1337",
  "post(str([1,\"a\"]))\n"
  "\n",
  "[1, 'a']", NULL, 0 },
{ "dif #1338",
  "post(str([1,2]))\n"
  "\n",
  "[1, 2]", NULL, 0 },
{ "dif #1339",
  "post(str({\"a\":1}))\n"
  "\n",
  "{'a': 1}", NULL, 0 },
{ "dif #1340",
  "post(str)\n"
  "\n",
  "str", NULL, 0 },
{ "dif #1341",
  "post(sum(1))\n"
  "\n",
  NULL, "TypeError: 'int' object is not iterable", -1 },
{ "dif #1342",
  "post(sum([1, 2, 3]))\n"
  "\n",
  "6", NULL, 0 },
{ "dif #1343",
  "post(sum([1, 2, 3], 10))\n"
  "\n",
  "16", NULL, 0 },
{ "dif #1344",
  "post(sum([1,\"a\"]))\n"
  "\n",
  NULL, "TypeError: unsupported operand type(s) for +: 'int' and 'str'", -1 },
{ "dif #1345",
  "post(sum([1,2,3,4,5]))\n"
  "\n",
  "15", NULL, 0 },
{ "dif #1346",
  "post(sum([1,2,3]))\n"
  "\n",
  "6", NULL, 0 },
{ "dif #1347",
  "post(sum([1,2.5,3]))\n"
  "\n",
  "6.5", NULL, 0 },
{ "dif #1348",
  "post(sum([1.5,2]))\n"
  "\n",
  "3.5", NULL, 0 },
{ "dif #1349",
  "post(sum([1.5], 1))\n"
  "\n",
  "2.5", NULL, 0 },
{ "dif #1350",
  "post(sum([True,1]))\n"
  "\n",
  "2", NULL, 0 },
{ "dif #1351",
  "post(sum([]))\n"
  "\n",
  "0", NULL, 0 },
{ "dif #1352",
  "post(sum(range(100)))\n"
  "\n",
  "4950", NULL, 0 },
{ "dif #1353",
  "post(sys.RelativePath(\"nao_existe_xyz_123\"))\n"
  "\n",
  NULL, "NameError: name 'sys' is not defined", -1 },
{ "dif #1354",
  "post(sys.argv[0])\n"
  "\n",
  NULL, "NameError: name 'sys' is not defined", -1 },
{ "dif #1355",
  "post(sys.argv[1])\n"
  "\n",
  NULL, "NameError: name 'sys' is not defined", -1 },
{ "dif #1356",
  "post(sys.argv[99])\n"
  "\n",
  NULL, "NameError: name 'sys' is not defined", -1 },
{ "dif #1357",
  "post(sys.platform())\n"
  "\n",
  NULL, "NameError: name 'sys' is not defined", -1 },
{ "dif #1358",
  "post(t is dict)\n"
  "\n",
  NULL, "NameError: name 't' is not defined", -1 },
{ "dif #1359",
  "post(t is tup)\n"
  "\n",
  NULL, "NameError: name 't' is not defined", -1 },
{ "dif #1360",
  "post(t)\n"
  "\n",
  NULL, "NameError: name 't' is not defined", -1 },
{ "dif #1361",
  "post(to int)\n"
  "\n",
  "int", NULL, 0 },
{ "dif #1362",
  "post(to str)\n"
  "\n",
  "str", NULL, 0 },
{ "dif #1363",
  "post(total())\n"
  "\n",
  NULL, "NameError: name 'total' is not defined", -1 },
{ "dif #1364",
  "post(total, len(guardados))\n"
  "\n",
  NULL, "NameError: name 'total' is not defined", -1 },
{ "dif #1365",
  "post(true * 5, false * 5)\n"
  "\n",
  "5 0", NULL, 0 },
{ "dif #1366",
  "post(true + 1)\n"
  "\n",
  "2", NULL, 0 },
{ "dif #1367",
  "post(true + true + false)\n"
  "\n",
  "2", NULL, 0 },
{ "dif #1368",
  "post(true - 1, true / 2, true % 2)\n"
  "\n",
  "0 0.5 1", NULL, 0 },
{ "dif #1369",
  "post(true < 3)\n"
  "\n",
  "True", NULL, 0 },
{ "dif #1370",
  "post(true == 1.0)\n"
  "\n",
  "True", NULL, 0 },
{ "dif #1371",
  "post(true and false, true or false, not true)\n"
  "\n",
  "False True False", NULL, 0 },
{ "dif #1372",
  "post(true or f())\n"
  "\n",
  "True", NULL, 0 },
{ "dif #1373",
  "post(true.isdigit())\n"
  "\n",
  NULL, "AttributeError: 'bool' object has no attribute 'isdigit'", -1 },
{ "dif #1374",
  "post(type(\n"
  "\n",
  NULL, "SyntaxError: expressao invalida", -1 },
{ "dif #1375",
  "post(type(\"a\"))\n"
  "\n",
  "str", NULL, 0 },
{ "dif #1376",
  "post(type((1,2)))\n"
  "\n",
  "tup", NULL, 0 },
{ "dif #1377",
  "post(type(1))\n"
  "\n",
  "int", NULL, 0 },
{ "dif #1378",
  "post(type(1.5))\n"
  "\n",
  "flo", NULL, 0 },
{ "dif #1379",
  "post(type(Cliente()))\n"
  "\n",
  NULL, "NameError: name 'Cliente' is not defined", -1 },
{ "dif #1380",
  "post(type(Null))\n"
  "\n",
  "Null", NULL, 0 },
{ "dif #1381",
  "post(type(Parsing.integer(\"1\")))\n"
  "\n",
  "int", NULL, 0 },
{ "dif #1382",
  "post(type(PoolFile))\n"
  "\n",
  "type", NULL, 0 },
{ "dif #1383",
  "post(type(True))\n"
  "\n",
  "bool", NULL, 0 },
{ "dif #1384",
  "post(type([1]))\n"
  "\n",
  "list", NULL, 0 },
{ "dif #1385",
  "post(type(a))\n"
  "\n",
  NULL, "NameError: name 'a' is not defined", -1 },
{ "dif #1386",
  "post(type(abs))\n"
  "\n",
  "action", NULL, 0 },
{ "dif #1387",
  "post(type(app))\n"
  "\n",
  NULL, "NameError: name 'app' is not defined", -1 },
{ "dif #1388",
  "post(type(app.\n"
  "\n",
  NULL, "SyntaxError: esperado nome do membro apos '.'", -1 },
{ "dif #1389",
  "post(type(b))\n"
  "\n",
  NULL, "NameError: name 'b' is not defined", -1 },
{ "dif #1390",
  "post(type(c))\n"
  "\n",
  NULL, "NameError: name 'c' is not defined", -1 },
{ "dif #1391",
  "post(type(e))\n"
  "\n",
  NULL, "NameError: name 'e' is not defined", -1 },
{ "dif #1392",
  "post(type(f), f.name, f.ext, f.size > 0)\n"
  "\n",
  NULL, "NameError: name 'f' is not defined", -1 },
{ "dif #1393",
  "post(type(f), f.name, f.size > 0)\n"
  "\n",
  NULL, "NameError: name 'f' is not defined", -1 },
{ "dif #1394",
  "post(type(form), type(e1), type(bt))\n"
  "\n",
  NULL, "NameError: name 'form' is not defined", -1 },
{ "dif #1395",
  "post(type(i))\n"
  "\n",
  NULL, "NameError: name 'i' is not defined", -1 },
{ "dif #1396",
  "post(type(k))\n"
  "\n",
  NULL, "NameError: name 'k' is not defined", -1 },
{ "dif #1397",
  "post(type(os.environ()))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1398",
  "post(type(os.loadFile(\"cfg.json\")))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1399",
  "post(type(os.loadFile(\"img.png\")))\n"
  "\n",
  NULL, "NameError: name 'os' is not defined", -1 },
{ "dif #1400",
  "post(type(r))\n"
  "\n",
  NULL, "NameError: name 'r' is not defined", -1 },
{ "dif #1401",
  "post(type(str))\n"
  "\n",
  "type", NULL, 0 },
{ "dif #1402",
  "post(type({\"a\":1}))\n"
  "\n",
  "dict", NULL, 0 },
{ "dif #1403",
  "post(util())\n"
  "\n",
  NULL, "NameError: name 'util' is not defined", -1 },
{ "dif #1404",
  "post(viveu)\n"
  "\n",
  NULL, "NameError: name 'viveu' is not defined", -1 },
{ "dif #1405",
  "post(x is int)\n"
  "\n",
  NULL, "NameError: name 'x' is not defined", -1 },
{ "dif #1406",
  "post(x)\n"
  "\n",
  NULL, "NameError: name 'x' is not defined", -1 },
{ "dif #1407",
  "post(x, type(x))\n"
  "\n",
  NULL, "NameError: name 'x' is not defined", -1 },
{ "dif #1408",
  "post(y)\n"
  "\n",
  NULL, "NameError: name 'y' is not defined", -1 },
{ "dif #1409",
  "post(zip())\n"
  "\n",
  "[]", NULL, 0 },
{ "dif #1410",
  "post(zip(1))\n"
  "\n",
  NULL, "TypeError: 'int' object is not iterable", -1 },
{ "dif #1411",
  "post(zip([1,2,3],[\"a\"]))\n"
  "\n",
  "[(1, 'a')]", NULL, 0 },
{ "dif #1412",
  "post(zip([1,2],[\"a\",\"b\"]))\n"
  "\n",
  "[(1, 'a'), (2, 'b')]", NULL, 0 },
{ "dif #1413",
  "post(zip([1,2],[3,4]))\n"
  "\n",
  "[(1, 3), (2, 4)]", NULL, 0 },
{ "dif #1414",
  "post(zip([1],[2],[3]))\n"
  "\n",
  "[(1, 2, 3)]", NULL, 0 },
{ "dif #1415",
  "post(zzz)\n"
  "\n",
  NULL, "NameError: name 'zzz' is not defined", -1 },
{ "dif #1416",
  "post(zzz.campo)\n"
  "\n",
  NULL, "NameError: name 'zzz' is not defined", -1 },
{ "dif #1417",
  "post(zzz.metodo())\n"
  "\n",
  NULL, "NameError: name 'zzz' is not defined", -1 },
{ "dif #1418",
  "post(zzz[0])\n"
  "\n",
  NULL, "NameError: name 'zzz' is not defined", -1 },
{ "dif #1419",
  "post({ \"a\": zzz })\n"
  "\n",
  NULL, "NameError: name 'zzz' is not defined", -1 },
{ "dif #1420",
  "post({\"a\": 1}[\"z\"])\n"
  "\n",
  NULL, "KeyError: 'z'", -1 },
{ "dif #1421",
  "post({\"a\": Null})\n"
  "\n",
  "{'a': null}", NULL, 0 },
{ "dif #1422",
  "post({\"a\":1,\"b\":2,\"c\":3,\"d\":4,\"e\":5})\n"
  "\n",
  "{'a': 1, 'b': 2, 'c': 3, 'd': 4, 'e': 5}", NULL, 0 },
{ "dif #1423",
  "post({\"a\":1,\"b\":2} == {\"b\":2,\"a\":1})\n"
  "\n",
  "True", NULL, 0 },
{ "dif #1424",
  "post({\"a\":1} == {\"a\":1})\n"
  "\n",
  "True", NULL, 0 },
{ "dif #1425",
  "post({\"a\":1} == {\"a\":2})\n"
  "\n",
  "False", NULL, 0 },
{ "dif #1426",
  "post({\"a\":1}.append(1))\n"
  "\n",
  NULL, "KeyError: 'append'", -1 },
{ "dif #1427",
  "post({\"a\":1}.pop(\"z\"))\n"
  "\n",
  NULL, "KeyError: 'z'", -1 },
{ "dif #1428",
  "post({\"a\":1}.type())\n"
  "\n",
  "dict", NULL, 0 },
{ "dif #1429",
  "post({\"a\":1}.update([1]))\n"
  "\n",
  NULL, "TypeError: 'list' object is not iterable", -1 },
{ "dif #1430",
  "post({\"a\":1}[\"z\"])\n"
  "\n",
  NULL, "KeyError: 'z'", -1 },
{ "dif #1431",
  "post({\"b\":1,\"a\":2})\n"
  "\n",
  "{'b': 1, 'a': 2}", NULL, 0 },
{ "dif #1432",
  "post({\"k\": \"a\".encode()})\n"
  "\n",
  "{'k': b'a'}", NULL, 0 },
{ "dif #1433",
  "post({\"z\":1,\"y\":2,\"x\":3})\n"
  "\n",
  "{'z': 1, 'y': 2, 'x': 3}", NULL, 0 },
{ "dif #1434",
  "post({1.5:\"a\", True:\"b\"})\n"
  "\n",
  "{1.5: 'a', True: 'b'}", NULL, 0 },
{ "dif #1435",
  "post({1:\"a\",2:\"b\"})\n"
  "\n",
  "{1: 'a', 2: 'b'}", NULL, 0 },
{ "dif #1436",
  "post({1:\"a\"} == {true:\"a\"})\n"
  "\n",
  "True", NULL, 0 },
{ "dif #1437",
  "post(~\"x\")\n"
  "\n",
  NULL, "TypeError: bad operand type for unary ~: 'str'", -1 },
{ "dif #1438",
  "post(~0)\n"
  "\n",
  "-1", NULL, 0 },
{ "dif #1439",
  "post(~5)\n"
  "\n",
  "-6", NULL, 0 },
{ "dif #1440",
  "post(~True)\n"
  "\n",
  NULL, "TypeError: bad operand type for unary ~: 'bool'", -1 },
{ "dif #1441",
  "private async reaction foo() {\n"
  "    return 9\n"
  "}\n"
  "post(foo())\n"
  "\n",
  "9", NULL, 0 },
{ "dif #1442",
  "private class B() { action __init__(self) { self.n = 2 } }\n"
  "\n",
  "", NULL, 0 },
{ "dif #1443",
  "public action foo() {\n"
  "    return 12\n"
  "}\n"
  "post(foo())\n"
  "\n",
  "12", NULL, 0 },
{ "dif #1444",
  "public async reaction foo() {\n"
  "    return 8\n"
  "}\n"
  "post(foo())\n"
  "\n",
  "8", NULL, 0 },
{ "dif #1445",
  "public class A() {\n"
  "    public reaction nome(self) { return \"A\" }\n"
  "}\n"
  "public class B(A) {\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "dif #1446",
  "public class A() { action __init__(self) { self.n = 1 } }\n"
  "\n",
  "", NULL, 0 },
{ "dif #1447",
  "public class C()\n"
  "{\n"
  "    @static\n"
  "    public reaction dobro(self, x)\n"
  "    {\n"
  "        return x * 2\n"
  "    }\n"
  "}\n"
  "post(map([1, 2, 3], C.dobro))\n"
  "\n",
  "[2, 4, 6]", NULL, 0 },
{ "dif #1448",
  "public class C()\n"
  "{\n"
  "    @static\n"
  "    public reaction f(self, a, b=10)\n"
  "    {\n"
  "        return a + b\n"
  "    }\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "dif #1449",
  "public class Contador() {\n"
  "    public reaction __init__(self, n) { self.n = n }\n"
  "    public reaction get(self) { return self.n }\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "dif #1450",
  "public class Mat() {\n"
  "    @static\n"
  "    public reaction soma(self, a, b) { return a + b }\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "dif #1451",
  "raise NetworkError(\"propaga limpo\")\n"
  "\n",
  NULL, "NetworkError: propaga limpo", -1 },
{ "dif #1452",
  "raise ValueError(\"x\")\n"
  "\n",
  NULL, "ValueError: x", -1 },
{ "dif #1453",
  "raise ValueError(zzz)\n"
  "\n",
  NULL, "NameError: name 'zzz' is not defined", -1 },
{ "dif #1454",
  "reaction conta(n) {\n"
  "    int i = 0\n"
  "    while (i < n) { yield i\n"
  "        i++ }\n"
  "}\n"
  "int soma = 0\n"
  "for each v in conta(10000) { soma = soma + v }\n"
  "post(soma)\n"
  "\n",
  "49995000", NULL, 0 },
{ "dif #1455",
  "reaction dobro(n) { return n * 2 }\n"
  "\n",
  "", NULL, 0 },
{ "dif #1456",
  "reaction f()\n"
  "{\n"
  "    return 77\n"
  "}\n"
  "post(f())\n"
  "\n",
  "77", NULL, 0 },
{ "dif #1457",
  "reaction f() { return 1 }\n"
  "\n",
  "", NULL, 0 },
{ "dif #1458",
  "reaction f() { return 1 }\n"
  "post(f())\n"
  "\n",
  "1", NULL, 0 },
{ "dif #1459",
  "reaction f() {\n"
  "    return 42\n"
  "}\n"
  "post(f())\n"
  "\n",
  "42", NULL, 0 },
{ "dif #1460",
  "reaction f(a, b) { return b }\n"
  "post(f(1, {\"k\": 2}))\n"
  "\n",
  "{'k': 2}", NULL, 0 },
{ "dif #1461",
  "reaction f(a, b=0) { return a }\n"
  "post(f({\"a\": 1} {\"b\": 2}, b=9))\n"
  "\n",
  NULL, "SyntaxError: faltou ',' antes do dicionario '{' — cada argumento precisa de vi", -1 },
{ "dif #1462",
  "reaction f(a, b=10, c=20) { return (a, b, c) }\n"
  "post(f(1))\n"
  "post(f(1, c=99))\n"
  "post(f(1, 2, 3))\n"
  "\n",
  "(1, 10, 20)\n(1, 10, 99)\n(1, 2, 3)", NULL, 0 },
{ "dif #1463",
  "reaction f(a=0, b=none) { return b }\n"
  "post(f(a=1, b={\"k\": 2}))\n"
  "\n",
  "{'k': 2}", NULL, 0 },
{ "dif #1464",
  "reaction f(x) { return x }\n"
  "post(f({\"a\": 1} {\"b\": 2}))\n"
  "\n",
  NULL, "SyntaxError: faltou ',' antes do dicionario '{' — cada argumento precisa de vi", -1 },
{ "dif #1465",
  "reaction f(x) { return x }\n"
  "post(f({\"nome\": \"valor\"}))\n"
  "\n",
  "{'nome': 'valor'}", NULL, 0 },
{ "dif #1466",
  "reaction fib(n) {\n"
  "    if (n < 2) { return n }\n"
  "    return fib(n - 1) + fib(n - 2)\n"
  "}\n"
  "post(fib(10))\n"
  "\n",
  "55", NULL, 0 },
{ "dif #1467",
  "reaction g() {\n"
  "    yield 1\n"
  "    yield 2\n"
  "    yield 3\n"
  "}\n"
  "for each v in g() { post(v) }\n"
  "\n",
  "1\n2\n3", NULL, 0 },
{ "dif #1468",
  "reaction precisa(v) { return v }\n"
  "\n",
  "", NULL, 0 },
{ "dif #1469",
  "reaction soma(a,b) { return a+b }\n"
  "post(soma(3,4))\n"
  "\n",
  "7", NULL, 0 },
{ "dif #1470",
  "reaction soma_ate(n) {\n"
  "    if (n == 0) { return 0 }\n"
  "    return n + soma_ate(n - 1)\n"
  "}\n"
  "post(soma_ate(1000))\n"
  "\n",
  "500500", NULL, 0 },
{ "dif #1471",
  "resultado = []\n"
  "for each c in \"abc\" {\n"
  "    addEnd(resultado, c)\n"
  "}\n"
  "post(resultado)\n"
  "\n",
  "['a', 'b', 'c']", NULL, 0 },
{ "dif #1472",
  "resultado = []\n"
  "for each par in enumerate([10, 20]) { addEnd(resultado, par) }\n"
  "post(resultado)\n"
  "\n",
  "[(0, 10), (1, 20)]", NULL, 0 },
{ "dif #1473",
  "if __name__ == \"main\" {\n"
  " post(1)\n"
  "}\n"
  "\n",
  "1", NULL, 0 },
{ "dif #1474",
  "if __name__ == \"main\" { post(\"NAO DEVIA RODAR NO IMPORT\") }\n"
  "\n",
  "NAO DEVIA RODAR NO IMPORT", NULL, 0 },
{ "dif #1475",
  "if __name__ == \"main\" {\n"
  "    post(\"sim\")\n"
  "}\n"
  "\n",
  "sim", NULL, 0 },
{ "dif #1476",
  "s = \"Hello World\"\n"
  "post(s.upper(), s.lower(), s.replace(\"o\", \"0\"))\n"
  "post(s.split(\" \"))\n"
  "\n",
  "HELLO WORLD hello world Hell0 W0rld\n['Hello', 'World']", NULL, 0 },
{ "dif #1477",
  "s = \"abc\"\n"
  "for each c in s {\n"
  " post(c)\n"
  "}\n"
  "\n",
  "a\nb\nc", NULL, 0 },
{ "dif #1478",
  "s = \"abc\"\n"
  "match s {\n"
  " case \"abc\" { post(1) }\n"
  " case _ { post(2) }\n"
  "}\n"
  "\n",
  "1", NULL, 0 },
{ "dif #1479",
  "s = \"poolscript\"\n"
  "post(s[0:4])\n"
  "post(s[4:])\n"
  "post(s[::-1])\n"
  "\n",
  "pool\nscript\ntpircsloop", NULL, 0 },
{ "dif #1480",
  "s = \"{\\\"x\\\": 42}\"\n"
  "post(s.get_json(\"x\"))\n"
  "\n",
  "42", NULL, 0 },
{ "dif #1481",
  "s=\"ab\"\n"
  "count each str in s {\n"
  " post(_match)\n"
  "}\n"
  "\n",
  "a\nb", NULL, 0 },
{ "dif #1482",
  "str NOME = \"oi\"\n"
  "\n",
  "", NULL, 0 },
{ "dif #1483",
  "str action f() { return 1 }\n"
  "\n",
  "", NULL, 0 },
{ "dif #1484",
  "str action f() { return 1/0 }\n"
  "\n",
  "", NULL, 0 },
{ "dif #1485",
  "str email = \"x\"\n"
  "\n",
  "", NULL, 0 },
{ "dif #1486",
  "str guardada = \"SOBREVIVE\"\n"
  "str lixo = \"\"\n"
  "int i = 0\n"
  "while (i < 120000) {\n"
  " \n"
  "\n",
  NULL, "SyntaxError: bloco com '{' nao foi fechado com '}'", -1 },
{ "dif #1487",
  "str lixo = \"\"\n"
  "int i = 0\n"
  "while (i < 120000) {\n"
  " \n"
  "\n",
  NULL, "SyntaxError: bloco com '{' nao foi fechado com '}'", -1 },
{ "dif #1488",
  "str lixo = \"\"\n"
  "int i = 0\n"
  "while (i < 150000) {\n"
  " \n"
  "\n",
  NULL, "SyntaxError: bloco com '{' nao foi fechado com '}'", -1 },
{ "dif #1489",
  "str nome = \"Pool\"\n"
  "\n",
  "", NULL, 0 },
{ "dif #1490",
  "str nome = \"Pool\"\n"
  "post(nome)\n"
  "\n",
  "Pool", NULL, 0 },
{ "dif #1491",
  "str nome = 10\n"
  "\n",
  NULL, "AttributedValueError: variável nome esperava str", -1 },
{ "dif #1492",
  "str reaction precisa(v) { return v }\n"
  "\n",
  "", NULL, 0 },
{ "dif #1493",
  "str ruim = 123\n"
  "\n",
  NULL, "AttributedValueError: variável ruim esperava str", -1 },
{ "dif #1494",
  "str s = \"\"\n"
  "for each i in range(2000) { s = s + \"ab\" }\n"
  "post(len(s))\n"
  "\n",
  "4000", NULL, 0 },
{ "dif #1495",
  "str s = \"\"\n"
  "if (s) {\n"
  " post(1)\n"
  "} else {\n"
  " post(2)\n"
  "}\n"
  "\n",
  "2", NULL, 0 },
{ "dif #1496",
  "str s = \"\"\n"
  "int i = 0\n"
  "while (i < 5) {\n"
  " s = s + \"x\"\n"
  " i += 1\n"
  "}\n"
  "post(s)\n"
  "\n",
  "xxxxx", NULL, 0 },
{ "dif #1497",
  "str s = \"a\"\n"
  "\n",
  "", NULL, 0 },
{ "dif #1498",
  "str s = \"aaa\"\n"
  "\n",
  "", NULL, 0 },
{ "dif #1499",
  "str s = \"aaa\"\n"
  "post(s.replace(\"a\", \"b\", count=2))\n"
  "\n",
  "bba", NULL, 0 },
{ "dif #1500",
  "str s = \"pool\"\n"
  "post(s + \"script\")\n"
  "\n",
  "poolscript", NULL, 0 },
{ "dif #1501",
  "str x = \"a\"\n"
  "\n",
  "", NULL, 0 },
{ "dif #1502",
  "str x = 5\n"
  "\n",
  NULL, "AttributedValueError: variável x esperava str", -1 },
{ "dif #1503",
  "t = \"\".maketrans(\"ab\",\"xy\")\n"
  "post(\"abc\".translate(t))\n"
  "\n",
  "xyc", NULL, 0 },
{ "dif #1504",
  "t = ()\n"
  "post(t)\n"
  "\n",
  "()", NULL, 0 },
{ "dif #1505",
  "t = (1, 2, 3)\n"
  "a, b, c = t\n"
  "post(a)\n"
  "post(b)\n"
  "post(c)\n"
  "\n",
  "1\n2\n3", NULL, 0 },
{ "dif #1506",
  "t = (1, 2, 3)\n"
  "post(t)\n"
  "\n",
  "(1, 2, 3)", NULL, 0 },
{ "dif #1507",
  "t = (1, 2, 3)\n"
  "post(t.\n"
  "\n",
  NULL, "SyntaxError: esperado nome do membro apos '.'", -1 },
{ "dif #1508",
  "t = (1, 2, 3)\n"
  "try { t.append(9) } catch (e) { post(\"recusou\") }\n"
  "post(t, len(t))\n"
  "\n",
  "recusou\n(1, 2, 3) 3", NULL, 0 },
{ "dif #1509",
  "t = (1,)\n"
  "post(t)\n"
  "\n",
  "(1,)", NULL, 0 },
{ "dif #1510",
  "t = (1,2) + (3,)\n"
  "post(len(t))\n"
  "\n",
  "3", NULL, 0 },
{ "dif #1511",
  "t = (1,2) + (3,)\n"
  "post(t[2])\n"
  "\n",
  "3", NULL, 0 },
{ "dif #1512",
  "t = (1,2,3,4)\n"
  "post(t[1:3])\n"
  "\n",
  "(2, 3)", NULL, 0 },
{ "dif #1513",
  "t = (10, 20)\n"
  "post(t[0])\n"
  "post(t[-1])\n"
  "\n",
  "10\n20", NULL, 0 },
{ "dif #1514",
  "t = [1]\n"
  "l = [t]\n"
  "t.append(l)\n"
  "post(t)\n"
  "\n",
  "[1, [[...]]]", NULL, 0 },
{ "dif #1515",
  "t = {97: 120}\n"
  "post(\"abc\".translate(t))\n"
  "\n",
  "xbc", NULL, 0 },
{ "dif #1516",
  "total = 0\n"
  "i = 0\n"
  "while (i < 5) {\n"
  "    if (true) {\n"
  "        if (true) {\n"
  "            total = total + i\n"
  "        }\n"
  "    }\n"
  "    i = i + 1\n"
  "}\n"
  "post(total)\n"
  "\n",
  "10", NULL, 0 },
{ "dif #1517",
  "try {\n"
  "\n",
  NULL, "SyntaxError: bloco com '{' nao foi fechado com '}'", -1 },
{ "dif #1518",
  "try {\n"
  "    post(\"a\")\n"
  "} catch (e) {\n"
  "    post(\"nunca\")\n"
  "} finally {\n"
  "    post(\"fim\")\n"
  "}\n"
  "\n",
  "a\nfim", NULL, 0 },
{ "dif #1519",
  "try {\n"
  "    post(\"ok\")\n"
  "} catch (e) {\n"
  "    post(\"nao\")\n"
  "} finally {\n"
  "    post(\"f1\")\n"
  "}\n"
  "try {\n"
  "    raise ValueError(\"x\")\n"
  "} catch (e) {\n"
  "    post(\"peguei\")\n"
  "} finally {\n"
  "    post(\"f2\")\n"
  "}\n"
  "\n",
  "ok\nf1\npeguei\nf2", NULL, 0 },
{ "dif #1520",
  "try {\n"
  "    raise \"algo\"\n"
  "} catch (TypeError e) {\n"
  "    post(\"tipo_certo\")\n"
  "} catch (e) {\n"
  "    post(\"generico:\" e)\n"
  "}\n"
  "\n",
  "generico: algo (linha 2)", NULL, 0 },
{ "dif #1521",
  "try {\n"
  "    raise \"boom\"\n"
  "} catch (e) {\n"
  "    post(\"peguei\")\n"
  "} finally {\n"
  "    post(\"fim\")\n"
  "}\n"
  "\n",
  "peguei\nfim", NULL, 0 },
{ "dif #1522",
  "try {\n"
  "    try {\n"
  "        raise ValueError(\"x\")\n"
  "    } catch (e) {\n"
  "        raise KeyError(\"re-raise\")\n"
  "    } finally {\n"
  "        post(\"finally\")\n"
  "    }\n"
  "} catch (e) {\n"
  "    post(\"fora\")\n"
  "}\n"
  "\n",
  "finally\nfora", NULL, 0 },
{ "dif #1523",
  "try {\n"
  "    try {\n"
  "        x = 1/0\n"
  "    } catch (KeyError e) {\n"
  "        post(\"errado\")\n"
  "    }\n"
  "} catch (ZeroDivisionError e) {\n"
  "    post(\"certo\")\n"
  "}\n"
  "\n",
  "certo", NULL, 0 },
{ "dif #1524",
  "try {\n"
  "    x = 1\n"
  "} catch (e) {\n"
  "    post(e)\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "dif #1525",
  "try {\n"
  "    x = 1 / 0\n"
  "} catch (ZeroDivisionError e) {\n"
  "    post(\"tipo_certo\")\n"
  "} catch (e) {\n"
  "    post(\"generico\")\n"
  "}\n"
  "\n",
  "tipo_certo", NULL, 0 },
{ "dif #1526",
  "try {\n"
  "    x = 1/0\n"
  "} catch (KeyError e) {\n"
  "    post(\"nao_deveria\")\n"
  "} finally {\n"
  "    post(\"fim\")\n"
  "}\n"
  "\n",
  NULL, "ZeroDivisionError: division by zero", -1 },
{ "dif #1527",
  "try {\n"
  "    x = 1/0\n"
  "} catch (KeyError e) {\n"
  "    post(\"nunca\")\n"
  "} finally {\n"
  "    post(\"fim\")\n"
  "}\n"
  "\n",
  NULL, "ZeroDivisionError: division by zero", -1 },
{ "dif #1528",
  "try {\n"
  "    x = 10 / 0\n"
  "} catch (e) {\n"
  "    post(\"err: \" {e})\n"
  "}\n"
  "\n",
  "err: division by zero (linha 2)", NULL, 0 },
{ "dif #1529",
  "try {\n"
  "  using open(r\"\n"
  "\n",
  NULL, "SyntaxError: string nao fechada antes da quebra de linha", -1 },
{ "dif #1530",
  "try {\n"
  " post(1)\n"
  "} catch (KeyError e) {\n"
  " post(2)\n"
  "}\n"
  "\n",
  "1", NULL, 0 },
{ "dif #1531",
  "try {\n"
  " post(1)\n"
  "} catch (KeyError e) {\n"
  " post(2)\n"
  "} catch (e) {\n"
  " post(3)\n"
  "}\n"
  "\n",
  "1", NULL, 0 },
{ "dif #1532",
  "try {\n"
  " post(1)\n"
  "} catch (base) {\n"
  " post(2)\n"
  "}\n"
  "\n",
  NULL, "SyntaxError: 'base' e palavra reservada da linguagem e nao pode ser usada como n", -1 },
{ "dif #1533",
  "try {\n"
  " post(1)\n"
  "} catch (e) {\n"
  " post(2)\n"
  "}\n"
  "\n",
  "1", NULL, 0 },
{ "dif #1534",
  "try {\n"
  " post(1)\n"
  "} catch (e) {\n"
  " post(2)\n"
  "} finally {\n"
  " post(3)\n"
  "}\n"
  "\n",
  "1\n3", NULL, 0 },
{ "dif #1535",
  "try {\n"
  " post(1)\n"
  "} catch (return) {\n"
  " post(2)\n"
  "}\n"
  "\n",
  NULL, "SyntaxError: 'return' e palavra reservada da linguagem e nao pode ser usada como", -1 },
{ "dif #1536",
  "try {\n"
  " post(1/0)\n"
  "} catch (e) {\n"
  " post(\"pegou\")\n"
  "}\n"
  "\n",
  "pegou", NULL, 0 },
{ "dif #1537",
  "try {\n"
  " post(1/0)\n"
  "} catch (e) {\n"
  " post(2)\n"
  "} finally {\n"
  " post(3)\n"
  "}\n"
  "\n",
  "2\n3", NULL, 0 },
{ "dif #1538",
  "try {\n"
  " post(1/0)\n"
  "} catch (e) {\n"
  " post(e)\n"
  "}\n"
  "\n",
  "division by zero (linha 2)", NULL, 0 },
{ "dif #1539",
  "try {\n"
  " post(5 % 0)\n"
  "} catch (e) {\n"
  " post(e)\n"
  "}\n"
  "\n",
  "integer modulo by zero (linha 2)", NULL, 0 },
{ "dif #1540",
  "try {\n"
  " post(int(\"x\"))\n"
  "} catch (e) {\n"
  " post(\"pego\")\n"
  "}\n"
  "\n",
  "pego", NULL, 0 },
{ "dif #1541",
  "try {\n"
  " post(len(1))\n"
  "} catch (e) {\n"
  " post(\"pego\")\n"
  "}\n"
  "\n",
  "pego", NULL, 0 },
{ "dif #1542",
  "try {\n"
  " post(min([]))\n"
  "} catch (e) {\n"
  " post(\"pego\")\n"
  "}\n"
  "\n",
  "pego", NULL, 0 },
{ "dif #1543",
  "try {\n"
  " post(zzz)\n"
  "} catch (e) {\n"
  " post(\"nome\")\n"
  "}\n"
  "\n",
  "nome", NULL, 0 },
{ "dif #1544",
  "try {\n"
  " post(zzz)\n"
  "} catch (e) {\n"
  " post(e)\n"
  "}\n"
  "\n",
  "name 'zzz' is not defined (linha 2)", NULL, 0 },
{ "dif #1545",
  "try {\n"
  " post({\"a\":1}[\"z\"])\n"
  "} catch (e) {\n"
  " post(\"chave\")\n"
  "}\n"
  "\n",
  "chave", NULL, 0 },
{ "dif #1546",
  "try {\n"
  " post({\"a\":1}[\"z\"])\n"
  "} catch (e) {\n"
  " post(e)\n"
  "}\n"
  "\n",
  "'z' (linha 2)", NULL, 0 },
{ "dif #1547",
  "try {\n"
  " raise \"meu erro\"\n"
  "} catch (e) {\n"
  " post(e)\n"
  "}\n"
  "\n",
  "meu erro (linha 2)", NULL, 0 },
{ "dif #1548",
  "try {\n"
  " raise 42\n"
  "} catch (e) {\n"
  " post(e)\n"
  "}\n"
  "\n",
  "42 (linha 2)", NULL, 0 },
{ "dif #1549",
  "try {\n"
  " try {\n"
  "  post(1/0)\n"
  " } catch (e) {\n"
  "  post(\"interno\")\n"
  " }\n"
  "} catch (e) {\n"
  " post(\"externo\")\n"
  "}\n"
  "\n",
  "interno", NULL, 0 },
{ "dif #1550",
  "try { c.cursor().execute(\"\n"
  "\n",
  NULL, "SyntaxError: string nao fechada antes da quebra de linha", -1 },
{ "dif #1551",
  "try { f.save(\"naoexiste/x.png\") } catch (e) { post(\"erro\") }\n"
  "\n",
  "erro", NULL, 0 },
{ "dif #1552",
  "try { r.content_type(\"text/html\") } catch (e) { post(\"errado\") }\n"
  "\n",
  "errado", NULL, 0 },
{ "dif #1553",
  "try { r.select() } catch (e) { post(\"pego: \" + e) }\n"
  "\n",
  "pego: name 'r' is not defined (linha 1)", NULL, 0 },
{ "dif #1554",
  "try { raise \"algo\" } catch (TypeError e) { post(\"s\") } catch (e) { post(\"g\") }\n"
  "\n",
  "g", NULL, 0 },
{ "dif #1555",
  "try { raise \"b1\" } catch (RuntimeError) { post(\"A\") }\n"
  "\n",
  "A", NULL, 0 },
{ "dif #1556",
  "try { raise \"b2\" } catch (RuntimeError e) { post(\"B\", e) }\n"
  "\n",
  "B b2 (linha 1)", NULL, 0 },
{ "dif #1557",
  "try { raise \"b3\" } catch (e) { post(\"C\", e) }\n"
  "\n",
  "C b3 (linha 1)", NULL, 0 },
{ "dif #1558",
  "try { raise \"b4\" } catch () { post(\"D\") }\n"
  "\n",
  "D", NULL, 0 },
{ "dif #1559",
  "try { raise \"x\" } catch (e) { post(e) }\n"
  "\n",
  "x (linha 1)", NULL, 0 },
{ "dif #1560",
  "try { raise DatabaseError(\"off\") } catch (DatabaseError) { post(\"B\") }\n"
  "\n",
  "B", NULL, 0 },
{ "dif #1561",
  "try { raise MinhaFalhaQualquer(\"x\") } catch (e) { post(\"C\", e) }\n"
  "\n",
  "C x (linha 1)", NULL, 0 },
{ "dif #1562",
  "try { raise NetworkError(\"caiu\") } catch (NetworkError e) { post(\"A\", e) }\n"
  "\n",
  "A caiu (linha 1)", NULL, 0 },
{ "dif #1563",
  "try { s.login(\"a\", \"b\") } catch (e) { post(\"pego: \" + e) }\n"
  "\n",
  "pego: name 's' is not defined (linha 1)", NULL, 0 },
{ "dif #1564",
  "try { x = 1/0 } catch (KeyError e) { post(\"k\") }\n"
  "\n",
  NULL, "ZeroDivisionError: division by zero", -1 },
{ "dif #1565",
  "try { x = 1/0 } catch (ZeroDivisionError e) { post(\"s\") } catch (e) { post(\"g\") }\n"
  "\n",
  "s", NULL, 0 },
{ "dif #1566",
  "try {\n"
  "    int x = 1 / 0\n"
  "} catch (e) {\n"
  "    post(\"pego\")\n"
  "}\n"
  "\n",
  "pego", NULL, 0 },
{ "dif #1567",
  "try {\n"
  "    post(1)\n"
  "} catch (e) {\n"
  "    post(2)\n"
  "}\n"
  "\n",
  "1", NULL, 0 },
{ "dif #1568",
  "try {\n"
  "    raise \"ops\"\n"
  "} catch (e) {\n"
  "    post(\"catch\")\n"
  "} finally {\n"
  "    post(\"finally\")\n"
  "}\n"
  "\n",
  "catch\nfinally", NULL, 0 },
{ "dif #1569",
  "try {\n"
  "    raise ValueError(\"x\")\n"
  "} catch (KeyError e) {\n"
  "    post(\"pegou\")\n"
  "}\n"
  "\n",
  NULL, "ValueError: x (linha 2)", -1 },
{ "dif #1570",
  "try {\n"
  "    raise ValueError(\"x\")\n"
  "} catch (ValueError e) {\n"
  "    post(\"pegou\")\n"
  "}\n"
  "\n",
  "pegou", NULL, 0 },
{ "dif #1571",
  "try {\n"
  "    raise ValueError(\"x\")\n"
  "} catch (e) {\n"
  "    post(1)\n"
  "}\n"
  "post(e)\n"
  "\n",
  NULL, "NameError: name 'e' is not defined", -1 },
{ "dif #1572",
  "try {\n"
  "    raise ValueError(\"x\")\n"
  "} catch (e) {\n"
  "    post(e)\n"
  "}\n"
  "\n",
  "x (linha 2)", NULL, 0 },
{ "dif #1573",
  "try {\n"
  "    raise ValueError(\"x\")\n"
  "} catch (e) {\n"
  "    raise ValueError(f\"Erro: {e}\")\n"
  "}\n"
  "\n",
  NULL, "ValueError: Erro: x (linha 2)", -1 },
{ "dif #1574",
  "try {\n"
  "    t = 1\n"
  "} catch (e) {\n"
  "    t = 2\n"
  "}\n"
  "post(t)\n"
  "\n",
  NULL, "NameError: name 't' is not defined", -1 },
{ "dif #1575",
  "try {\n"
  "    x=1/0\n"
  "} catch (e) {\n"
  "    post(\"err\")\n"
  "}\n"
  "\n",
  "err", NULL, 0 },
{ "dif #1576",
  "tup = 5\n"
  "\n",
  NULL, "SyntaxError: 'tup' e palavra reservada da linguagem e nao pode ser usada como no", -1 },
{ "dif #1577",
  "tup t = (1, 2)\n"
  "\n",
  "", NULL, 0 },
{ "dif #1578",
  "using R() as r:\n"
  "\n",
  NULL, "SyntaxError: bloco com ':' nao existe mais", -1 },
{ "dif #1579",
  "using open(\"saida.txt\", mode=\"w\") as arq:\n"
  "\n",
  NULL, "SyntaxError: bloco com ':' nao existe mais", -1 },
{ "dif #1580",
  "using open(\"x\") as f {\n"
  " post(1)\n"
  "}\n"
  "\n",
  NULL, "IOError: arquivo nao encontrado: 'x'", -1 },
{ "dif #1581",
  "using open(\"x\") as f {\n"
  "    post(1)\n"
  "}\n"
  "\n",
  NULL, "IOError: arquivo nao encontrado: 'x'", -1 },
{ "dif #1582",
  "using open(\"x.txt\") as f {\n"
  "    post(f.read(3))\n"
  "    post(f.read(2))\n"
  "    post(f.read())\n"
  "}\n"
  "\n",
  NULL, "IOError: arquivo nao encontrado: 'x.txt'", -1 },
{ "dif #1583",
  "using open(\"x.txt\", argento=\"w\") as f:\n"
  "\n",
  NULL, "SyntaxError: bloco com ':' nao existe mais", -1 },
{ "dif #1584",
  "using open(r\"\n"
  "\n",
  NULL, "SyntaxError: string nao fechada antes da quebra de linha", -1 },
{ "dif #1585",
  "using sqlite3.connect(\"u.db\") as c {\n"
  "\n",
  NULL, "SyntaxError: bloco com '{' nao foi fechado com '}'", -1 },
{ "dif #1586",
  "viva = [111, 222, 333]\n"
  "str lixo = \"\"\n"
  "int i = 0\n"
  "while (i < 120000) {\n"
  " \n"
  "\n",
  NULL, "SyntaxError: bloco com '{' nao foi fechado com '}'", -1 },
{ "dif #1587",
  "vivo = {\"chave\": \"VALOR_INTACTO\"}\n"
  "str lixo = \"\"\n"
  "int i = 0\n"
  "while (i < 120000) {\n"
  " \n"
  "\n",
  NULL, "SyntaxError: bloco com '{' nao foi fechado com '}'", -1 },
{ "dif #1588",
  "while (x) {\n"
  " break\n"
  "}\n"
  "\n",
  NULL, "NameError: name 'x' is not defined", -1 },
{ "dif #1589",
  "while (x) {\n"
  " continue\n"
  "}\n"
  "\n",
  NULL, "NameError: name 'x' is not defined", -1 },
{ "dif #1590",
  "while (x) {\n"
  " post(1)\n"
  "}\n"
  "\n",
  NULL, "NameError: name 'x' is not defined", -1 },
{ "dif #1591",
  "while += 1\n"
  "\n",
  NULL, "SyntaxError: 'while' e palavra reservada da linguagem e nao pode ser usada como ", -1 },
{ "dif #1592",
  "while = 1\n"
  "\n",
  NULL, "SyntaxError: 'while' e palavra reservada da linguagem e nao pode ser usada como ", -1 },
{ "dif #1593",
  "while = 5\n"
  "\n",
  NULL, "SyntaxError: 'while' e palavra reservada da linguagem e nao pode ser usada como ", -1 },
{ "dif #1594",
  "x = \"sim\" if 5 > 3 else \"nao\"\n"
  "\n",
  "", NULL, 0 },
{ "dif #1595",
  "x = \"texto\"\n"
  "if (x not is int) { post(\"a\") }\n"
  "if (x is not None) { post(\"b\") }\n"
  "\n",
  "a\nb", NULL, 0 },
{ "dif #1596",
  "x = 0 - 10\n"
  "match x {\n"
  " case -10 { post(\"neg\") }\n"
  " case _ { post(\"nao\") }\n"
  "}\n"
  "\n",
  "neg", NULL, 0 },
{ "dif #1597",
  "x = 1\n"
  "action f() {\n"
  " action g() {\n"
  "  x = 99\n"
  " }\n"
  " g()\n"
  "}\n"
  "f()\n"
  "post(x)\n"
  "\n",
  "99", NULL, 0 },
{ "dif #1598",
  "x = 1\n"
  "action f() {\n"
  " global x\n"
  " x = 99\n"
  "}\n"
  "f()\n"
  "post(x)\n"
  "\n",
  "99", NULL, 0 },
{ "dif #1599",
  "x = 1\n"
  "action f() {\n"
  " int x = 99\n"
  "}\n"
  "f()\n"
  "post(x)\n"
  "\n",
  "1", NULL, 0 },
{ "dif #1600",
  "x = 1\n"
  "action f() {\n"
  " x = 99\n"
  "}\n"
  "f()\n"
  "post(x)\n"
  "\n",
  "99", NULL, 0 },
{ "dif #1601",
  "x = 1\n"
  "action f(x) {\n"
  " x = 99\n"
  "}\n"
  "f(5)\n"
  "post(x)\n"
  "\n",
  "1", NULL, 0 },
{ "dif #1602",
  "x = 1\n"
  "if (true) {\n"
  "    if (true) {\n"
  "        if (true) {\n"
  "            if (true) {\n"
  "                y = x + 1\n"
  "                post(y)\n"
  "            }\n"
  "        }\n"
  "    }\n"
  "}\n"
  "\n",
  "2", NULL, 0 },
{ "dif #1603",
  "x = 1\n"
  "if (x == 1) { post(\"ok\") }\n"
  "\n",
  "ok", NULL, 0 },
{ "dif #1604",
  "x = 1\n"
  "match x {\n"
  " case 1 { post(\"um\") }\n"
  " case _ { post(\"outro\") }\n"
  "}\n"
  "\n",
  "um", NULL, 0 },
{ "dif #1605",
  "x = 1\n"
  "match x {\n"
  "    case 1 {\n"
  "        post(\"um\")\n"
  "    }\n"
  "    case _ {\n"
  "        post(\"outro\")\n"
  "    }\n"
  "}\n"
  "\n",
  "um", NULL, 0 },
{ "dif #1606",
  "x = 1\n"
  "reaction f() {\n"
  "    global x\n"
  "    x = 99\n"
  "}\n"
  "f()\n"
  "post(x)\n"
  "\n",
  "99", NULL, 0 },
{ "dif #1607",
  "x = 10\n"
  "post(f\"v={x}\")\n"
  "\n",
  "v=10", NULL, 0 },
{ "dif #1608",
  "x = 10\n"
  "post(x)\n"
  "\n",
  "10", NULL, 0 },
{ "dif #1609",
  "x = 15\n"
  "match x {\n"
  "    case v if v < 10 {\n"
  "        post(\"pequeno\")\n"
  "    }\n"
  "    case v if v < 20 {\n"
  "        post(\"medio\")\n"
  "    }\n"
  "    case _ {\n"
  "        post(\"grande\")\n"
  "    }\n"
  "}\n"
  "\n",
  "medio", NULL, 0 },
{ "dif #1610",
  "x = 18446744073709551616\n"
  "post(sum([x, 1]))\n"
  "post(sorted([x, 1]))\n"
  "post(max(x, 1), min(x, 1))\n"
  "post(round(x))\n"
  "post(int(x))\n"
  "post(str(x))\n"
  "\n",
  "18446744073709551617\n[1, 18446744073709551616]\n18446744073709551616 1\n18446744073709551616\n18446744073709551616\n18446744073709551616", NULL, 0 },
{ "dif #1611",
  "x = 18446744073709551616\n"
  "post(type(x), x.type())\n"
  "\n",
  "int int", NULL, 0 },
{ "dif #1612",
  "x = 2\n"
  "match x {\n"
  "    case 1 { post(\"um\") }\n"
  "    case 2 { post(\"dois\") }\n"
  "    case _ { post(\"?\") }\n"
  "}\n"
  "\n",
  "dois", NULL, 0 },
{ "dif #1613",
  "x = 3\n"
  "post(f\"dobro={x * 2}\")\n"
  "\n",
  "dobro=6", NULL, 0 },
{ "dif #1614",
  "x = 4\n"
  "post(\"par\" if x % 2 == 0 else \"impar\")\n"
  "\n",
  "par", NULL, 0 },
{ "dif #1615",
  "x = 42\n"
  "match x {\n"
  " case v { post(v) }\n"
  "}\n"
  "\n",
  "42", NULL, 0 },
{ "dif #1616",
  "x = 42\n"
  "post(\"valor:\" x)\n"
  "\n",
  "valor: 42", NULL, 0 },
{ "dif #1617",
  "x = 5\n"
  "match x {\n"
  " case 1 { post(\"um\") }\n"
  " case _ { post(\"outro\") }\n"
  "}\n"
  "\n",
  "outro", NULL, 0 },
{ "dif #1618",
  "x = 5\n"
  "post(-x, +x, ~x, not false)\n"
  "\n",
  "-5 5 -6 True", NULL, 0 },
{ "dif #1619",
  "x = 5\n"
  "post(f\"{x}\")\n"
  "\n",
  "5", NULL, 0 },
{ "dif #1620",
  "x = 5\n"
  "y = \"oi\"\n"
  "post(f\"{y} {x} {x + 1}\")\n"
  "\n",
  "oi 5 6", NULL, 0 },
{ "dif #1621",
  "x = 7\n"
  "action f() {\n"
  " return x * 2\n"
  "}\n"
  "post(f())\n"
  "\n",
  "14", NULL, 0 },
{ "dif #1622",
  "x = 7\n"
  "post(<blue>\"val \" {x} \" fim\")\n"
  "\n",
  "[38;2;33;150;243mval 7 fim[0m", NULL, 0 },
{ "dif #1623",
  "x = 99\n"
  "match x {\n"
  " case 1 { post(\"um\") }\n"
  "}\n"
  "post(\"fim\")\n"
  "\n",
  "fim", NULL, 0 },
{ "dif #1624",
  "x = Null\n"
  "action f() {\n"
  " x = 9\n"
  "}\n"
  "f()\n"
  "post(x)\n"
  "\n",
  "9", NULL, 0 },
{ "dif #1625",
  "x = Null\n"
  "post(\"v=\" {x})\n"
  "\n",
  "v=null", NULL, 0 },
{ "dif #1626",
  "x = Null\n"
  "post(f\"v={x}\")\n"
  "\n",
  "v=null", NULL, 0 },
{ "dif #1627",
  "x = Null > 0\n"
  "post(x)\n"
  "\n",
  NULL, "TypeError: '>' not supported between instances of 'Null' and 'int'", -1 },
{ "dif #1628",
  "x = Parsing.floating(\"1,5\")\n"
  "post(x * 2)\n"
  "post(x.type())\n"
  "\n",
  "3.0\nflo", NULL, 0 },
{ "dif #1629",
  "x = Parsing.integer(\"12\")\n"
  "post(x + 1)\n"
  "post(x.type())\n"
  "\n",
  "13\nint", NULL, 0 },
{ "dif #1630",
  "x = True\n"
  "match x {\n"
  " case True { post(1) }\n"
  " case Null { post(2) }\n"
  " case _ { post(3) }\n"
  "}\n"
  "\n",
  "1", NULL, 0 },
{ "dif #1631",
  "x = a is bool\n"
  "\n",
  NULL, "NameError: name 'a' is not defined", -1 },
{ "dif #1632",
  "x = a is flo\n"
  "\n",
  NULL, "NameError: name 'a' is not defined", -1 },
{ "dif #1633",
  "x = a is int\n"
  "\n",
  NULL, "NameError: name 'a' is not defined", -1 },
{ "dif #1634",
  "x = a is str\n"
  "\n",
  NULL, "NameError: name 'a' is not defined", -1 },
{ "dif #1635",
  "x = count each int(7) in nums\n"
  "\n",
  NULL, "NameError: name 'nums' is not defined", -1 },
{ "dif #1636",
  "x = int(y)\n"
  "\n",
  NULL, "NameError: name 'y' is not defined", -1 },
{ "dif #1637",
  "x=\"a\"\n"
  "post(x is str)\n"
  "\n",
  "True", NULL, 0 },
{ "dif #1638",
  "x=(1,)\n"
  "post(x is tup)\n"
  "\n",
  "True", NULL, 0 },
{ "dif #1639",
  "x=1\n"
  "y=2\n"
  "x, y = y, x\n"
  "post(x)\n"
  "post(y)\n"
  "\n",
  "2\n1", NULL, 0 },
{ "dif #1640",
  "x=1.5\n"
  "post(x is flo)\n"
  "\n",
  "True", NULL, 0 },
{ "dif #1641",
  "x=123\n"
  "post(x.isalpha())\n"
  "\n",
  "False", NULL, 0 },
{ "dif #1642",
  "x=150\n"
  "post(x.isdigit())\n"
  "\n",
  "True", NULL, 0 },
{ "dif #1643",
  "x=150.5\n"
  "post(x.isdigit())\n"
  "\n",
  "False", NULL, 0 },
{ "dif #1644",
  "x=30\n"
  "match x {\n"
  "    case p if p<50 {\n"
  "        post(\"barato\")\n"
  "    }\n"
  "    case _ {\n"
  "        post(\"caro\")\n"
  "    }\n"
  "}\n"
  "\n",
  "barato", NULL, 0 },
{ "dif #1645",
  "x=5\n"
  "if (x>0) {\n"
  "    post(\"ok\")\n"
  "}\n"
  "\n",
  "ok", NULL, 0 },
{ "dif #1646",
  "x=5\n"
  "if x is int {\n"
  " post(\"eh int\")\n"
  "}\n"
  "\n",
  "eh int", NULL, 0 },
{ "dif #1647",
  "x=5\n"
  "post(x is int)\n"
  "\n",
  "True", NULL, 0 },
{ "dif #1648",
  "x=5\n"
  "post(x is not str)\n"
  "\n",
  "True", NULL, 0 },
{ "dif #1649",
  "x=5\n"
  "post(x is str)\n"
  "\n",
  "False", NULL, 0 },
{ "dif #1650",
  "x=5\n"
  "post(x not is str)\n"
  "\n",
  "True", NULL, 0 },
{ "dif #1651",
  "x=Null\n"
  "post(x is Null)\n"
  "\n",
  "True", NULL, 0 },
{ "dif #1652",
  "x=True\n"
  "post(x is bool)\n"
  "\n",
  "True", NULL, 0 },
{ "dif #1653",
  "x=True\n"
  "post(x is int)\n"
  "\n",
  "False", NULL, 0 },
{ "dif #1654",
  "x=[1]\n"
  "post(x is list)\n"
  "\n",
  "True", NULL, 0 },
{ "dif #1655",
  "x={\"a\":1}\n"
  "post(x is dict)\n"
  "\n",
  "True", NULL, 0 },
{ "dif #1656",
  "x={\"a\":1}\n"
  "post(x is json)\n"
  "\n",
  "True", NULL, 0 },
{ "dif #1657",
  "xlsx volta tipado (int/float/null), csv tudo string, xml aninhado,\n"
  "    html limpo, json decodificado.\n"
  "\n",
  NULL, "SyntaxError: expressao invalida", -1 },
{ "dif #1658",
  "y = 1 if a else 2 if b else 3\n"
  "\n",
  NULL, "NameError: name 'a' is not defined", -1 },
{ "dif #1659",
  "} catch (NetworkError e) { post(\"rede\") }\n"
  "\n",
  NULL, "SyntaxError: expressao invalida", -1 },
{ "dif #1660",
  "} catch (TypeError e) { post(\"tipo certo\") }\n"
  "\n",
  NULL, "SyntaxError: expressao invalida", -1 },
{ "dif #1661",
  "} catch (TimeoutError e) { post(\"expirou:\", e) }\n"
  "\n",
  NULL, "SyntaxError: expressao invalida", -1 },
{ "dif #1662",
  "} catch (e) { post(\"erro sem commit\") }\n"
  "\n",
  NULL, "SyntaxError: expressao invalida", -1 },
{ "dif #1663",
  "} catch (e) { post(\"estourou\") }\n"
  "\n",
  NULL, "SyntaxError: expressao invalida", -1 },
{ "dif #1664",
  "} catch (e) { post(\"fora\") }\n"
  "\n",
  NULL, "SyntaxError: expressao invalida", -1 },
{ "dif #1665",
  "} catch (e) { post(\"sem servidor\") }\n"
  "\n",
  NULL, "SyntaxError: expressao invalida", -1 },
{ "dif #1666",
  "action f(a) {\n"
  "    return a\n"
  "}\n"
  "post(f(a=zzz))\n"
  "\n",
  NULL, "NameError: name 'zzz' is not defined", -1 },
{ "dif #1667",
  "action f(a, b) {\n"
  " return a\n"
  "}\n"
  "post(f(1))\n"
  "\n",
  NULL, "TypeError: f() missing 1 required positional argument: 'b'", -1 },
{ "dif #1668",
  "action f(a, b) {\n"
  " return a + b\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "dif #1669",
  "action f(a, b) {\n"
  " return a - b\n"
  "}\n"
  "post(f(10, b=3))\n"
  "\n",
  "7", NULL, 0 },
{ "dif #1670",
  "action f(a, b) {\n"
  " return a - b\n"
  "}\n"
  "post(f(a=10, b=3))\n"
  "\n",
  "7", NULL, 0 },
{ "dif #1671",
  "action f(a, b) {\n"
  " return a - b\n"
  "}\n"
  "post(f(b=3, a=10))\n"
  "\n",
  "7", NULL, 0 },
{ "dif #1672",
  "action f(a, b) {\n"
  " return a+b\n"
  "}\n"
  "post(f(1, b=2))\n"
  "\n",
  "3", NULL, 0 },
{ "dif #1673",
  "action f(a, b) { return a }\n"
  "\n",
  "", NULL, 0 },
{ "dif #1674",
  "action f(a, b):\n"
  "\n",
  NULL, "SyntaxError: bloco com ':' nao existe mais", -1 },
{ "dif #1675",
  "action f(a, b,) { return a }\n"
  "\n",
  "", NULL, 0 },
{ "dif #1676",
  "action f(a, b=1, c=2) {\n"
  " return a\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "dif #1677",
  "action f(a, b=1, c=2) {\n"
  " return a+b+c\n"
  "}\n"
  "post(f(0))\n"
  "post(f(0,10))\n"
  "post(f(0,10,100))\n"
  "\n",
  "3\n12\n110", NULL, 0 },
{ "dif #1678",
  "action f(a, b=10) {\n"
  " return a + b\n"
  "}\n"
  "post(f(1))\n"
  "post(f(1,2))\n"
  "\n",
  "11\n3", NULL, 0 },
{ "dif #1679",
  "action f(a, b=100) {\n"
  " return a + b\n"
  "}\n"
  "post(f(a=1))\n"
  "post(f(a=1, b=2))\n"
  "\n",
  "101\n3", NULL, 0 },
{ "dif #1680",
  "action f(a, b=2) {\n"
  "    return a\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "dif #1681",
  "action f(a, b=2*3) {\n"
  " return a + b\n"
  "}\n"
  "post(f(1))\n"
  "\n",
  "7", NULL, 0 },
{ "dif #1682",
  "action f(a, b=5, c=10) {\n"
  " return a+b+c\n"
  "}\n"
  "post(f(1, c=100))\n"
  "\n",
  "106", NULL, 0 },
{ "dif #1683",
  "action f(a=1, b=2) {\n"
  " return a+b\n"
  "}\n"
  "post(f())\n"
  "post(f(10))\n"
  "post(f(10,20))\n"
  "\n",
  "3\n12\n30", NULL, 0 },
{ "dif #1684",
  "action f(a=1, b=2, c=3) {\n"
  " return a+b+c\n"
  "}\n"
  "post(f(c=30, a=10))\n"
  "\n",
  "42", NULL, 0 },
{ "dif #1685",
  "action f(a=9) {\n"
  " return a\n"
  "}\n"
  "post(f(a=Null))\n"
  "\n",
  "null", NULL, 0 },
{ "dif #1686",
  "action f(a=Null) {\n"
  " return a\n"
  "}\n"
  "post(f())\n"
  "post(f(5))\n"
  "\n",
  "null\n5", NULL, 0 },
{ "dif #1687",
  "action f(base) {\n"
  " return 1\n"
  "}\n"
  "\n",
  NULL, "SyntaxError: 'base' e palavra reservada da linguagem e nao pode ser usada como n", -1 },
{ "dif #1688",
  "action f(base) { return 1 }\n"
  "\n",
  NULL, "SyntaxError: 'base' e palavra reservada da linguagem e nao pode ser usada como n", -1 },
{ "dif #1689",
  "action f(if) {\n"
  " return 1\n"
  "}\n"
  "\n",
  NULL, "SyntaxError: 'if' e palavra reservada da linguagem e nao pode ser usada como nom", -1 },
{ "dif #1690",
  "action f(n) {\n"
  "\n",
  NULL, "SyntaxError: bloco com '{' nao foi fechado com '}'", -1 },
{ "dif #1691",
  "action f(n) {\n"
  " if (n < 2) {\n"
  "  return n\n"
  " }\n"
  " return f(n-1) + f(n-2)\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "dif #1692",
  "action f(n) {\n"
  " match n {\n"
  "  case 0 { return \"zero\" }\n"
  "  case _ { return \"outro\" }\n"
  " }\n"
  "}\n"
  "post(f(0))\n"
  "post(f(7))\n"
  "\n",
  "zero\noutro", NULL, 0 },
{ "dif #1693",
  "action f(o) {\n"
  " o.a.b = 1\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "dif #1694",
  "action f(o) { return o.base }\n"
  "\n",
  "", NULL, 0 },
{ "dif #1695",
  "action f(s) {\n"
  "\n",
  NULL, "SyntaxError: bloco com '{' nao foi fechado com '}'", -1 },
{ "dif #1696",
  "action f(s=\"oi\") {\n"
  " return s\n"
  "}\n"
  "post(f())\n"
  "post(f(\"tchau\"))\n"
  "\n",
  "oi\ntchau", NULL, 0 },
{ "dif #1697",
  "action f(self) {\n"
  " self.x = 1\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "dif #1698",
  "action f(x) {\n"
  "\n",
  NULL, "SyntaxError: bloco com '{' nao foi fechado com '}'", -1 },
{ "dif #1699",
  "action f(x) {\n"
  " return x\n"
  "}\n"
  "post(map([1,\"a\",True], f))\n"
  "\n",
  "[1, 'a', True]", NULL, 0 },
{ "dif #1700",
  "action f(x) {\n"
  " return x*2\n"
  "}\n"
  "action g() {\n"
  " yield f(3)\n"
  "}\n"
  "post(list(g()))\n"
  "\n",
  "[6]", NULL, 0 },
{ "dif #1701",
  "action f(x) {\n"
  "    return x + 1\n"
  "}\n"
  "post(f(1))\n"
  "\n",
  "2", NULL, 0 },
{ "dif #1702",
  "action f(x:\n"
  "    return x\n"
  "\n",
  NULL, "SyntaxError: faltou ')' na declaracao da action", -1 },
{ "dif #1703",
  "action fib(n) {\n"
  " if (n < 2) {\n"
  "  return n\n"
  " }\n"
  " return fib(n-1) + fib(n-2)\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "dif #1704",
  "action fib(n) {\n"
  " if (n < 2) {\n"
  "  return n\n"
  " }\n"
  " return fib(n-1) + fib(n-2)\n"
  "}\n"
  "post(fib(15))\n"
  "\n",
  "610", NULL, 0 },
{ "dif #1705",
  "action fib(n) {\n"
  " if (n < 2) {\n"
  "  return n\n"
  " }\n"
  " return fib(n-1) + fib(n-2)\n"
  "}\n"
  "post(fib(18))\n"
  "\n",
  "2584", NULL, 0 },
{ "dif #1706",
  "action fora() {\n"
  "    a = 1\n"
  "    action dentro() {\n"
  "        return a\n"
  "    }\n"
  "    return dentro()\n"
  "}\n"
  "post(fora())\n"
  "\n",
  "1", NULL, 0 },
{ "dif #1707",
  "action g() {\n"
  "\n",
  NULL, "SyntaxError: bloco com '{' nao foi fechado com '}'", -1 },
{ "dif #1708",
  "action g() {\n"
  "    str a = \"PRIMEIRA\"\n"
  "    str lixo = \"\"\n"
  "    int i = 0\n"
  "    while (i < 120000) {\n"
  "        \n"
  "\n",
  NULL, "SyntaxError: bloco com '{' nao foi fechado com '}'", -1 },
{ "dif #1709",
  "action g() {\n"
  " a = 10\n"
  " yield a\n"
  " a = a + 5\n"
  " yield a\n"
  "}\n"
  "post(list(g()))\n"
  "\n",
  "[10, 15]", NULL, 0 },
{ "dif #1710",
  "action g() {\n"
  " for each i in [1,2,3] {\n"
  "  yield i*2\n"
  " }\n"
  "}\n"
  "post(list(g()))\n"
  "\n",
  "[2, 4, 6]", NULL, 0 },
{ "dif #1711",
  "action g() {\n"
  " for each i in [1,2] {\n"
  "  for each j in [10,20] {\n"
  "   yield i*j\n"
  "  }\n"
  " }\n"
  "}\n"
  "post(list(g()))\n"
  "\n",
  "[10, 20, 20, 40]", NULL, 0 },
{ "dif #1712",
  "action g() {\n"
  " i=0\n"
  " while i<3 {\n"
  "  yield i\n"
  "  i++\n"
  " }\n"
  "}\n"
  "t=0\n"
  "for each v in g() {\n"
  " t = t + v\n"
  "}\n"
  "post(t)\n"
  "\n",
  "3", NULL, 0 },
{ "dif #1713",
  "action g() {\n"
  " l=[1,2]\n"
  " count each int in l {\n"
  "  post(self)\n"
  " }\n"
  "}\n"
  "g()\n"
  "\n",
  "2\n2", NULL, 0 },
{ "dif #1714",
  "action g() {\n"
  " x = 0\n"
  " while x < 3 {\n"
  "  yield x\n"
  "  x++\n"
  " }\n"
  "}\n"
  "post(list(g()))\n"
  "\n",
  "[0, 1, 2]", NULL, 0 },
{ "dif #1715",
  "action g() {\n"
  " yield\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "dif #1716",
  "action g() {\n"
  " yield 1\n"
  " yield 2\n"
  " yield 3\n"
  "}\n"
  "for each v in g() {\n"
  " if v == 2 {\n"
  "  break\n"
  " }\n"
  " post(v)\n"
  "}\n"
  "\n",
  "1", NULL, 0 },
{ "dif #1717",
  "action g() {\n"
  " yield 1\n"
  " yield 2\n"
  "}\n"
  "a = g()\n"
  "b = g()\n"
  "post(list(a))\n"
  "post(list(b))\n"
  "\n",
  "[1, 2]\n[1, 2]", NULL, 0 },
{ "dif #1718",
  "action g() {\n"
  " yield 1\n"
  " yield 2\n"
  "}\n"
  "for each x in g() {\n"
  " post(x)\n"
  "}\n"
  "\n",
  "1\n2", NULL, 0 },
{ "dif #1719",
  "action g() {\n"
  " yield 1\n"
  " yield 2\n"
  "}\n"
  "post(list(g()))\n"
  "\n",
  "[1, 2]", NULL, 0 },
{ "dif #1720",
  "action g() {\n"
  " yield 1\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "dif #1721",
  "action g() {\n"
  " yield 1\n"
  "}\n"
  "post(g())\n"
  "\n",
  "<generator g>", NULL, 0 },
{ "dif #1722",
  "action g() {\n"
  " yield 1\n"
  "}\n"
  "post(type(g()))\n"
  "\n",
  "generator", NULL, 0 },
{ "dif #1723",
  "action g() {\n"
  " yield 1\n"
  "}\n"
  "try {\n"
  " post(list(g()))\n"
  "} catch (e) {\n"
  " post(\"x\")\n"
  "}\n"
  "\n",
  "[1]", NULL, 0 },
{ "dif #1724",
  "action g() {\n"
  " yield 1\n"
  "}\n"
  "x=g()\n"
  "post(x + 1)\n"
  "\n",
  NULL, "TypeError: unsupported operand type(s) for +: 'generator' and 'int'", -1 },
{ "dif #1725",
  "action g() {\n"
  " yield 1/0\n"
  "}\n"
  "post(list(g()))\n"
  "\n",
  NULL, "ZeroDivisionError: division by zero", -1 },
{ "dif #1726",
  "action g() {\n"
  " yield [1,2]\n"
  " yield {\"a\":1}\n"
  "}\n"
  "post(list(g()))\n"
  "\n",
  "[[1, 2], {'a': 1}]", NULL, 0 },
{ "dif #1727",
  "action g(a = Null) {\n"
  "\n",
  NULL, "SyntaxError: bloco com '{' nao foi fechado com '}'", -1 },
{ "dif #1728",
  "action g(n) {\n"
  " i=0\n"
  " while i<n {\n"
  "  yield i\n"
  "  i++\n"
  " }\n"
  "}\n"
  "post(list(g(4)))\n"
  "\n",
  "[0, 1, 2, 3]", NULL, 0 },
{ "dif #1729",
  "action g(n) {\n"
  " post(\"v=\" {n})\n"
  "}\n"
  "g(7)\n"
  "\n",
  "v=7", NULL, 0 },
{ "dif #1730",
  "action g(n) {\n"
  " return f\"n={n}\"\n"
  "}\n"
  "post(g(9))\n"
  "\n",
  "n=9", NULL, 0 },
{ "dif #1731",
  "action g(nome=\"x\") {\n"
  " return nome\n"
  "}\n"
  "post(g(nome=\"pool\"))\n"
  "\n",
  "pool", NULL, 0 },
{ "dif #1732",
  "action g(x) {\n"
  "\n",
  NULL, "SyntaxError: bloco com '{' nao foi fechado com '}'", -1 },
{ "dif #1733",
  "action gen() {\n"
  "    yield 1\n"
  "    yield 2\n"
  "}\n"
  "a, b = gen()\n"
  "post(a)\n"
  "post(b)\n"
  "\n",
  "1\n2", NULL, 0 },
{ "dif #1734",
  "action gen(n) {\n"
  "    i = 0\n"
  "    while (i < n) {\n"
  "        if (i == 1) {\n"
  "            yield 999\n"
  "        }\n"
  "        yield i\n"
  "        i = i + 1\n"
  "    }\n"
  "}\n"
  "r1 = []\n"
  "for each v in gen(2) { addEnd(r1, v) }\n"
  "r2 = []\n"
  "for each v in gen(3) { addEnd(r2, v) }\n"
  "post(r1)\n"
  "post(r2)\n"
  "\n",
  "[0, 999, 1]\n[0, 999, 1, 2]", NULL, 0 },
{ "dif #1735",
  "action greet() {\n"
  "    return \"hi from lib\"\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "dif #1736",
  "action greet() {\n"
  "    return \"hi\"\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "dif #1737",
  "action greet(name) {\n"
  "    if name == \"oi\" {\n"
  "        post(\"hello\")\n"
  "    } else {\n"
  "        post(\"bye\")\n"
  "    }\n"
  "}\n"
  "greet(\"oi\")\n"
  "greet(\"x\")\n"
  "\n",
  "hello\nbye", NULL, 0 },
{ "dif #1738",
  "action h() { post(\"handler\") }\n"
  "\n",
  "", NULL, 0 },
{ "dif #1739",
  "action handler():\n"
  "\n",
  NULL, "SyntaxError: bloco com ':' nao existe mais", -1 },
{ "dif #1740",
  "action logger() { return \"logIn do nivel acima\" }\n"
  "\n",
  "", NULL, 0 },
{ "dif #1741",
  "action m(self) {\n"
  " return 1\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "dif #1742",
  "action main() {\n"
  "    post(\"oi\")\n"
  "}\n"
  "main()\n"
  "\n",
  "oi", NULL, 0 },
{ "dif #1743",
  "action n(x) {\n"
  "}\n"
  "post(map([1,2], n))\n"
  "\n",
  "[null, null]", NULL, 0 },
{ "dif #1744",
  "action nada() {\n"
  " int x = 1\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "dif #1745",
  "action nada() {\n"
  " int x = 1\n"
  "}\n"
  "post(nada())\n"
  "\n",
  "null", NULL, 0 },
{ "dif #1746",
  "action oi() {\n"
  "\n",
  NULL, "SyntaxError: bloco com '{' nao foi fechado com '}'", -1 },
{ "dif #1747",
  "action oi() {\n"
  "    return \"do sub\"\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "dif #1748",
  "action ok() {\n"
  "\n",
  NULL, "SyntaxError: bloco com '{' nao foi fechado com '}'", -1 },
{ "dif #1749",
  "action p(x) {\n"
  " return False\n"
  "}\n"
  "post(filter([1,2,3], p))\n"
  "\n",
  "[]", NULL, 0 },
{ "dif #1750",
  "action p(x) {\n"
  " return True\n"
  "}\n"
  "post(filter([1,2,3], p))\n"
  "\n",
  "[1, 2, 3]", NULL, 0 },
{ "dif #1751",
  "action p(x) {\n"
  " return True\n"
  "}\n"
  "post(filter([], p))\n"
  "\n",
  "[]", NULL, 0 },
{ "dif #1752",
  "action p(x) {\n"
  " return x\n"
  "}\n"
  "post(filter([0,1,2], p))\n"
  "\n",
  "[1, 2]", NULL, 0 },
{ "dif #1753",
  "action p(x) {\n"
  " return x > 2\n"
  "}\n"
  "post(filter([1,2,3,4], p))\n"
  "\n",
  "[3, 4]", NULL, 0 },
{ "dif #1754",
  "action p(x) {\n"
  " return x.isdigit()\n"
  "}\n"
  "post(filter([\"1\",\"a\",\"2\"], p))\n"
  "\n",
  "['1', '2']", NULL, 0 },
{ "dif #1755",
  "action precisa(v) { return v }\n"
  "\n",
  "", NULL, 0 },
{ "dif #1756",
  "action primeiro() {\n"
  "    nums = [1, 2, 7, 9, 7]\n"
  "    count each int(7) in nums {\n"
  "        return _index\n"
  "    }\n"
  "}\n"
  "post(primeiro())\n"
  "\n",
  "2", NULL, 0 },
{ "dif #1757",
  "action q() {\n"
  "\n",
  NULL, "SyntaxError: bloco com '{' nao foi fechado com '}'", -1 },
{ "dif #1758",
  "action quebra() {\n"
  "\n",
  NULL, "SyntaxError: bloco com '{' nao foi fechado com '}'", -1 },
{ "dif #1759",
  "action quem() { return \"ARQUIVO_LOCAL\" }\n"
  "\n",
  "", NULL, 0 },
{ "dif #1760",
  "action quem() { return \"LIB\" }\n"
  "\n",
  "", NULL, 0 },
{ "dif #1761",
  "action r() {\n"
  "    return 4\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "dif #1762",
  "action r(n) {\n"
  " if (n < 1) {\n"
  "  return 0\n"
  " }\n"
  " return 1 + r(n - 1)\n"
  "}\n"
  "post(r(10000))\n"
  "\n",
  "10000", NULL, 0 },
{ "dif #1763",
  "action r(n) {\n"
  " return r(n + 1)\n"
  "}\n"
  "post(r(1))\n"
  "\n",
  NULL, "RuntimeError: estouro de frames (recursao profunda demais)", -1 },
{ "dif #1764",
  "action r(x) {\n"
  " if x <= 1 {\n"
  "  return 1\n"
  " }\n"
  " return x * r(x-1)\n"
  "}\n"
  "post(map([3,4,5], r))\n"
  "\n",
  "[6, 24, 120]", NULL, 0 },
{ "dif #1765",
  "action registrar() {\n"
  "    global visitas\n"
  "    visitas = 1\n"
  "}\n"
  "registrar()\n"
  "post(visitas)\n"
  "\n",
  "1", NULL, 0 },
{ "dif #1766",
  "action rotulo(n) {\n"
  "\n",
  NULL, "SyntaxError: bloco com '{' nao foi fechado com '}'", -1 },
{ "dif #1767",
  "action s(x) {\n"
  " return x + 1\n"
  "}\n"
  "action t(x) {\n"
  " return s(x) * 10\n"
  "}\n"
  "post(map([1,2], t))\n"
  "\n",
  "[20, 30]", NULL, 0 },
{ "dif #1768",
  "action saudar(nome) { return \"ola \" + nome }\n"
  "str VALOR = \"1.0\"\n"
  "\n",
  "", NULL, 0 },
{ "dif #1769",
  "action soma(a, b) {\n"
  "\n",
  NULL, "SyntaxError: bloco com '{' nao foi fechado com '}'", -1 },
{ "dif #1770",
  "action soma(a, b) {\n"
  "    return a + b\n"
  "}\n"
  "post(soma(1, 2))\n"
  "post(soma(3, 4))\n"
  "post(soma(5, 6))\n"
  "\n",
  "3\n7\n11", NULL, 0 },
{ "dif #1771",
  "action soma(a, b) { return a + b }\n"
  "\n",
  "", NULL, 0 },
{ "dif #1772",
  "action soma(a, b, c) {\n"
  " return a + b + c\n"
  "}\n"
  "post(soma(1, 2, 3))\n"
  "\n",
  "6", NULL, 0 },
{ "dif #1773",
  "action stringify(x) {\n"
  "\n",
  NULL, "SyntaxError: bloco com '{' nao foi fechado com '}'", -1 },
{ "dif #1774",
  "action sum(a, b) { return a + b }\n"
  "\n",
  "", NULL, 0 },
{ "dif #1775",
  "action sum(a, b) { return a + b }\n"
  "post(sum(2, 3))\n"
  "\n",
  "5", NULL, 0 },
{ "dif #1776",
  "action total() {\n"
  "\n",
  NULL, "SyntaxError: bloco com '{' nao foi fechado com '}'", -1 },
{ "dif #1777",
  "action total() {\n"
  "    nums = [7, 1, 7, 2, 7]\n"
  "    count each int(7) in nums {\n"
  "        return;\n"
  "    }\n"
  "}\n"
  "post(total())\n"
  "\n",
  "3", NULL, 0 },
{ "dif #1778",
  "action trabalha(marca) {\n"
  "    str lixo = \"\"\n"
  "    int i = 0\n"
  "    while (i < 120000) {\n"
  "        \n"
  "\n",
  NULL, "SyntaxError: bloco com '{' nao foi fechado com '}'", -1 },
{ "dif #1779",
  "action u(x) {\n"
  " return x.upper()\n"
  "}\n"
  "post(map([\"a\",\"b\",\"ç\"], u))\n"
  "\n",
  "['A', 'B', 'Ç']", NULL, 0 },
{ "dif #1780",
  "action util() { return 42 }\n"
  "\n",
  "", NULL, 0 },
{ "dif #1781",
  "action while() { return 1 }\n"
  "\n",
  NULL, "SyntaxError: 'while' e palavra reservada da linguagem e nao pode ser usada como ", -1 },
{ "dif #1782",
  "async action dobra(n) {\n"
  "    return n * 2\n"
  "}\n"
  "fs = [dobra(1), 99, dobra(3)]\n"
  "rs = await fs\n"
  "post(rs)\n"
  "\n",
  "[2, 99, 6]", NULL, 0 },
{ "dif #1783",
  "async action f() {\n"
  "    import mymod\n"
  "    return mymod.saudar(\"ana\")\n"
  "}\n"
  "post(await f())\n"
  "\n",
  NULL, "ImportError: modulo nao encontrado: mymod", -1 },
{ "dif #1784",
  "async action f() {\n"
  " return 1\n"
  "}\n"
  "post(await f())\n"
  "\n",
  "1", NULL, 0 },
{ "dif #1785",
  "async action f() {\n"
  " return 1\n"
  "}\n"
  "post(await f())\n"
  "post(1)\n"
  "\n",
  "1\n1", NULL, 0 },
{ "dif #1786",
  "async action f() { return 1 }\n"
  "\n",
  "", NULL, 0 },
{ "dif #1787",
  "async action f() { return 55 }\n"
  "post(await f())\n"
  "\n",
  "55", NULL, 0 },
{ "dif #1788",
  "async action falha() {\n"
  "    return 1 / 0\n"
  "}\n"
  "await falha()\n"
  "\n",
  NULL, "ZeroDivisionError: division by zero", -1 },
{ "dif #1789",
  "async action falha() {\n"
  "    return 1 / 0\n"
  "}\n"
  "f = falha()\n"
  "await f\n"
  "\n",
  NULL, "ZeroDivisionError: division by zero", -1 },
{ "dif #1790",
  "async action fat(n) {\n"
  "    if (n <= 1) {\n"
  "        return 1\n"
  "    }\n"
  "    prev = await fat(n - 1)\n"
  "    return n * prev\n"
  "}\n"
  "post(await fat(5))\n"
  "\n",
  "120", NULL, 0 },
{ "dif #1791",
  "async action interno(n) {\n"
  "    return n + 1\n"
  "}\n"
  "async action externo(n) {\n"
  "    v = await interno(n)\n"
  "    return v * 10\n"
  "}\n"
  "post(await externo(4))\n"
  "\n",
  "50", NULL, 0 },
{ "dif #1792",
  "async action ok(n) { return n }\n"
  "async action falha() { return 1 / 0 }\n"
  "fs = [ok(1), falha(), ok(3)]\n"
  "await fs\n"
  "\n",
  NULL, "ZeroDivisionError: division by zero", -1 },
{ "dif #1793",
  "async action precisa(v) { return v }\n"
  "\n",
  "", NULL, 0 },
{ "dif #1794",
  "async action semRetorno() {\n"
  "    x = 1 + 1\n"
  "}\n"
  "post(await semRetorno())\n"
  "\n",
  "null", NULL, 0 },
{ "dif #1795",
  "async action valor() {\n"
  "    return 42\n"
  "}\n"
  "f = valor()\n"
  "post(f.result())\n"
  "\n",
  NULL, "AttributeError: 'future' object has no attribute 'result'", -1 },
{ "dif #1796",
  "async action vazio() {\n"
  "    return\n"
  "}\n"
  "post(await vazio())\n"
  "\n",
  "null", NULL, 0 },
{ "dif #1797",
  "async bool reaction f() {\n"
  "    return 1 / 0\n"
  "}\n"
  "post(await f())\n"
  "\n",
  "False", NULL, 0 },
{ "dif #1798",
  "async bool reaction f() { return 1 }\n"
  "\n",
  "", NULL, 0 },
{ "dif #1799",
  "async bool reaction precisa(v) { return v }\n"
  "\n",
  "", NULL, 0 },
{ "dif #1800",
  "async flo reaction f() { return 1 }\n"
  "\n",
  "", NULL, 0 },
{ "dif #1801",
  "async int action f() { return 1 }\n"
  "\n",
  "", NULL, 0 },
{ "dif #1802",
  "async int reaction f() {\n"
  "    return 1 / 0\n"
  "}\n"
  "post(await f())\n"
  "\n",
  "500", NULL, 0 },
{ "dif #1803",
  "async int reaction f() { return 21 }\n"
  "post(await f())\n"
  "\n",
  "21", NULL, 0 },
{ "dif #1804",
  "async int reaction foo() {\n"
  "    return 7\n"
  "}\n"
  "post(foo())\n"
  "\n",
  "7", NULL, 0 },
{ "dif #1805",
  "async int reaction precisa(v) { return v }\n"
  "\n",
  "", NULL, 0 },
{ "dif #1806",
  "async reaction d(n) { return n * 2 }\n"
  "reaction main() {\n"
  "    post(await d(21))\n"
  "    post(gather(d(1), d(2), d(3)))\n"
  "}\n"
  "main()\n"
  "\n",
  "42\n[2, 4, 6]", NULL, 0 },
{ "dif #1807",
  "async reaction f() { return 1 }\n"
  "\n",
  "", NULL, 0 },
{ "dif #1808",
  "async reaction foo() {\n"
  "    return 3\n"
  "}\n"
  "post(foo())\n"
  "\n",
  "3", NULL, 0 },
{ "dif #1809",
  "async reaction precisa(v) { return v }\n"
  "\n",
  "", NULL, 0 },
{ "dif #1810",
  "async str action f() { return 1 }\n"
  "\n",
  "", NULL, 0 },
{ "dif #1811",
  "b = \"ab\".encode()\n"
  "for each x in b {\n"
  " post(x)\n"
  "}\n"
  "\n",
  NULL, "RuntimeError: for each exige lista, tupla ou string", -1 },
{ "dif #1812",
  "b = \"ab\".encode()\n"
  "post(b == \"ab\")\n"
  "\n",
  "False", NULL, 0 },
{ "dif #1813",
  "b = \"ab\".encode()\n"
  "post(b == \"ab\".encode())\n"
  "\n",
  "True", NULL, 0 },
{ "dif #1814",
  "b = \"ab\".encode()\n"
  "post(b.decode())\n"
  "\n",
  "ab", NULL, 0 },
{ "dif #1815",
  "b = \"ab\".encode()\n"
  "post(b.hex())\n"
  "\n",
  "6162", NULL, 0 },
{ "dif #1816",
  "b = \"ab\".encode()\n"
  "post(b.type())\n"
  "\n",
  "bytes", NULL, 0 },
{ "dif #1817",
  "b = \"ab\".encode()\n"
  "post(b[-1])\n"
  "\n",
  "98", NULL, 0 },
{ "dif #1818",
  "b = \"ab\".encode()\n"
  "post(b[-9])\n"
  "\n",
  NULL, "IndexError: index out of range", -1 },
{ "dif #1819",
  "b = \"ab\".encode()\n"
  "post(b[0])\n"
  "\n",
  "97", NULL, 0 },
{ "dif #1820",
  "b = \"ab\".encode()\n"
  "post(b[9])\n"
  "\n",
  NULL, "IndexError: index out of range", -1 },
{ "dif #1821",
  "b = \"ab\".encode()\n"
  "post(len(b))\n"
  "\n",
  "2", NULL, 0 },
{ "dif #1822",
  "b = \"ab\".encode()\n"
  "post(str(b))\n"
  "\n",
  "b'ab'", NULL, 0 },
{ "dif #1823",
  "b = \"ab\".encode()\n"
  "post(type(b))\n"
  "\n",
  "bytes", NULL, 0 },
{ "dif #1824",
  "b = 1\n"
  "for each i in range(200) { b = b * 2 }\n"
  "post(b)\n"
  "\n",
  "1606938044258990275541962092341162602522202993782792835301376", NULL, 0 },
{ "dif #1825",
  "b = 1\n"
  "for each x in [\"a\" {b}] {\n"
  " post(x)\n"
  "}\n"
  "\n",
  "a1", NULL, 0 },
{ "dif #1826",
  "base_dados = 5\n"
  "post(base_dados)\n"
  "\n",
  "5", NULL, 0 },
{ "dif #1827",
  "bool action f() { return 1 }\n"
  "\n",
  "", NULL, 0 },
{ "dif #1828",
  "bool action f() { return 1/0 }\n"
  "\n",
  "", NULL, 0 },
{ "dif #1829",
  "bool action f() { return 5 }\n"
  "\n",
  "", NULL, 0 },
{ "dif #1830",
  "bool action f() { }\n"
  "\n",
  "", NULL, 0 },
{ "dif #1831",
  "bool b = True\n"
  "\n",
  "", NULL, 0 },
{ "dif #1832",
  "bool reaction f() { return 1 }\n"
  "\n",
  "", NULL, 0 },
{ "dif #1833",
  "bool reaction f() { return 1/0 }\n"
  "\n",
  "", NULL, 0 },
{ "dif #1834",
  "bool reaction f() { return 1/0 }\n"
  "post(f())\n"
  "\n",
  "False", NULL, 0 },
{ "dif #1835",
  "bool reaction f() { return false }\n"
  "post(f())\n"
  "\n",
  "False", NULL, 0 },
{ "dif #1836",
  "bool reaction f() { return true }\n"
  "post(f())\n"
  "\n",
  "True", NULL, 0 },
{ "dif #1837",
  "bool reaction precisa(v) { return v }\n"
  "\n",
  "", NULL, 0 },
{ "dif #1838",
  "bool x = 1\n"
  "\n",
  NULL, "AttributedValueError: variável x esperava bool", -1 },
{ "dif #1839",
  "bool x = 5\n"
  "\n",
  NULL, "AttributedValueError: variável x esperava bool", -1 },
{ "dif #1840",
  "bool x = true\n"
  "\n",
  "", NULL, 0 },
{ "dif #1841",
  "builtin post vazou: \n"
  "\n",
  NULL, "SyntaxError: expressao invalida", -1 },
{ "dif #1842",
  "c = \"ola\".encode()\n"
  "post(c[0:2], c[-1:], len(c[:]))\n"
  "\n",
  "b'ol' b'a' 3", NULL, 0 },
{ "dif #1843",
  "c = 0\n"
  "action inc() {\n"
  " global c\n"
  " c += 1\n"
  "}\n"
  "inc()\n"
  "inc()\n"
  "post(c)\n"
  "\n",
  "2", NULL, 0 },
{ "dif #1844",
  "c = Conta()\n"
  "post(c._log())\n"
  "\n",
  NULL, "NameError: name 'Conta' is not defined", -1 },
{ "dif #1845",
  "c = Conta()\n"
  "post(c.deposita(100))\n"
  "\n",
  NULL, "NameError: name 'Conta' is not defined", -1 },
{ "dif #1846",
  "c = Conta()\n"
  "post(c.deposita(100))\n"
  "post(c.dono)\n"
  "\n",
  NULL, "NameError: name 'Conta' is not defined", -1 },
{ "dif #1847",
  "c = Conta()\n"
  "post(c.dono)\n"
  "\n",
  NULL, "NameError: name 'Conta' is not defined", -1 },
{ "dif #1848",
  "c = Conta()\n"
  "post(c.saldo)\n"
  "\n",
  NULL, "NameError: name 'Conta' is not defined", -1 },
{ "dif #1849",
  "chamadas = 0\n"
  "action f() {\n"
  " chamadas = chamadas + 1\n"
  " return 0\n"
  "}\n"
  "l = [10]\n"
  "l[f()] += 5\n"
  "post(l[0])\n"
  "post(chamadas)\n"
  "\n",
  "15\n1", NULL, 0 },
{ "dif #1850",
  "class A() {\n"
  "    public reaction __init__(self, x) { self.x = x }\n"
  "    public reaction nome(self) { return \"A\" }\n"
  "}\n"
  "class B(A) {\n"
  "    public reaction __init__(self, x) { base(x) }\n"
  "    public reaction get(self) { return self.x }\n"
  "}\n"
  "b = B(7)\n"
  "post(b.get(), b.nome())\n"
  "\n",
  "7 A", NULL, 0 },
{ "dif #1851",
  "class B() { reaction m(self, x) { return x } }\n"
  "post(B().m({\"nome\": \"valor\"}))\n"
  "\n",
  "{'nome': 'valor'}", NULL, 0 },
{ "dif #1852",
  "class B() { reaction m(x) { return x } }\n"
  "b = B()\n"
  "post(map([1, 2, 3], b.m))\n"
  "\n",
  NULL, "RuntimeError: action 'm' dentro de Entity deve ter 'self' como primeiro parâmet", -1 },
{ "dif #1853",
  "class B() { reaction m(x) { return x } }\n"
  "post(B().m(x={\"nome\": \"valor\"}))\n"
  "\n",
  NULL, "RuntimeError: action 'm' dentro de Entity deve ter 'self' como primeiro parâmet", -1 },
{ "dif #1854",
  "class B() { reaction m(x) { return x } }\n"
  "post(B().m({\"nome\": \"valor\"}))\n"
  "\n",
  NULL, "RuntimeError: action 'm' dentro de Entity deve ter 'self' como primeiro parâmet", -1 },
{ "dif #1855",
  "class C() {\n"
  "    @static\n"
  "    public reaction soma(self, a, b) { return a + b }\n"
  "}\n"
  "post(C.soma(2, 3))\n"
  "\n",
  "5", NULL, 0 },
{ "dif #1856",
  "class C() {\n"
  "    private reaction seg(self) { return 42 }\n"
  "    public reaction pub(self) { return self.seg() }\n"
  "}\n"
  "post(C().pub())\n"
  "\n",
  "42", NULL, 0 },
{ "dif #1857",
  "class C() {\n"
  "    public reaction __init__(self, x) { self.x = x }\n"
  "    public reaction get(self) { return self.x }\n"
  "}\n"
  "post(C(7).get())\n"
  "\n",
  "7", NULL, 0 },
{ "dif #1858",
  "class C() {\n"
  "    @NonNull\n"
  "    action f(self, a) {\n"
  "        return a\n"
  "    }\n"
  "}\n"
  "post(C().f(7))\n"
  "\n",
  "7", NULL, 0 },
{ "dif #1859",
  "class C() {\n"
  "    @NonNull\n"
  "    action f(self, a) {\n"
  "        return a\n"
  "    }\n"
  "}\n"
  "post(C().f(null))\n"
  "\n",
  NULL, "RuntimeError: @NonNull: parametro 'a' em 'f' nao pode ser Null", -1 },
{ "dif #1860",
  "class C() {\n"
  "    @static\n"
  "    action m() {\n"
  "        return self.x\n"
  "    }\n"
  "}\n"
  "post(C.m())\n"
  "\n",
  NULL, "NameError: name 'self' is not defined", -1 },
{ "dif #1861",
  "class P() {\n"
  " action __init__(self, n) {\n"
  "  self.n = n\n"
  " }\n"
  "}\n"
  "post(P(4).n)\n"
  "\n",
  "4", NULL, 0 },
{ "dif #1862",
  "class P() {\n"
  " action m(self) { return 1 }\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "dif #1863",
  "class Pt() { public reaction __init__(self, v) { self.v = v } }\n"
  "int soma = 0\n"
  "for each i in range(5000) {\n"
  "    p = Pt(i)\n"
  "    soma = soma + p.v\n"
  "}\n"
  "post(soma)\n"
  "\n",
  "12497500", NULL, 0 },
{ "dif #1864",
  "class R() {\n"
  "    public reaction close(self) { return 0 }\n"
  "}\n"
  "using R() as r { post(\"usando\") }\n"
  "\n",
  "usando", NULL, 0 },
{ "dif #1865",
  "contador = 0\n"
  "action bump() {\n"
  "    global contador\n"
  "    if (true) {\n"
  "        contador = contador + 100\n"
  "    }\n"
  "}\n"
  "bump()\n"
  "post(contador)\n"
  "\n",
  "100", NULL, 0 },
{ "dif #1866",
  "contador = 0\n"
  "action incrementar() {\n"
  "    global contador\n"
  "    contador = contador + 1\n"
  "}\n"
  "incrementar()\n"
  "incrementar()\n"
  "incrementar()\n"
  "post(contador)\n"
  "\n",
  "3", NULL, 0 },
{ "dif #1867",
  "count each int in [10, \"a\", 20, 30] { post(_match, _index, _count) }\n"
  "\n",
  "10 0 3\n20 2 3\n30 3 3", NULL, 0 },
{ "dif #1868",
  "count each int in f() {\n"
  "\n",
  NULL, "SyntaxError: bloco com '{' nao foi fechado com '}'", -1 },
{ "dif #1869",
  "count each int in post {\n"
  "\n",
  NULL, "SyntaxError: bloco com '{' nao foi fechado com '}'", -1 },
{ "dif #1870",
  "count each int(7) in nums {\n"
  " post(1)\n"
  "}\n"
  "\n",
  NULL, "NameError: name 'nums' is not defined", -1 },
{ "dif #1871",
  "count each int(7) in nums {\n"
  "    post(1)\n"
  "}\n"
  "\n",
  NULL, "NameError: name 'nums' is not defined", -1 },
{ "dif #1872",
  "d = { \"a\": 1 }\n"
  "d[\"eu\"] = d\n"
  "post(d)\n"
  "\n",
  "{'a': 1, 'eu': {...}}", NULL, 0 },
{ "dif #1873",
  "d = { \"a\": 1 }\n"
  "post(d.value(\"a\"))\n"
  "\n",
  NULL, "TypeError: value expected 0 arguments, got 1", -1 },
{ "dif #1874",
  "d = { \"a\": 1 }\n"
  "post(d.z)\n"
  "\n",
  NULL, "KeyError: 'z'", -1 },
{ "dif #1875",
  "d = { \"a\": 1 }\n"
  "post(d[\"z\"])\n"
  "\n",
  NULL, "KeyError: 'z'", -1 },
{ "dif #1876",
  "d = { \"a\": 1, \"b\": 2, \"c\": 3 }\n"
  "soma = 0\n"
  "for each v in d.value() {\n"
  "    soma = soma + v\n"
  "}\n"
  "post(soma)\n"
  "\n",
  "6", NULL, 0 },
{ "dif #1877",
  "d = { \"a\": 1, \"b\": [2], \"c\": \"x\" }\n"
  "post(d.value() == d.values())\n"
  "post(d.value())\n"
  "post(len(d.value()), len(d))\n"
  "\n",
  "True\n[1, [2], 'x']\n3 3", NULL, 0 },
{ "dif #1878",
  "d = { \"a\": [1, 2], \"b\": (3, 4), \"c\": \"txt\", \"d\": 9, \"e\": 2.5 }\n"
  "post([1, 2] in d.value(), (3, 4) in d.value())\n"
  "post(\"txt\" in d.value(), 9 in d.value(), 2.5 in d.value())\n"
  "post([9] in d.value(), \"naotem\" in d.value())\n"
  "\n",
  "True True\nTrue True True\nFalse False", NULL, 0 },
{ "dif #1879",
  "d = { \"nome\": \"ana\", \"idade\": 30, \"peso\": 1.5 }\n"
  "post(\"nome\" in d, \"ana\" in d, 30 in d)\n"
  "post(\"ana\" in d.value(), 30 in d.value(), 1.5 in d.value())\n"
  "\n",
  "True False False\nTrue True True", NULL, 0 },
{ "dif #1880",
  "d = { \"value\": 42, \"outro\": 1 }\n"
  "post(d[\"value\"])\n"
  "post(d.value())\n"
  "post(42 in d.value())\n"
  "\n",
  "42\n[42, 1]\nTrue", NULL, 0 },
{ "dif #1881",
  "d = {\"a\": 1, \"b\": 2}\n"
  "post(d.keys(), d.values(), d.has(\"a\"), d.get(\"z\"))\n"
  "\n",
  "['a', 'b'] [1, 2] True null", NULL, 0 },
{ "dif #1882",
  "d = {\"a\": 1, \"b\": 2}\n"
  "post(d[\"a\"])\n"
  "\n",
  "1", NULL, 0 },
{ "dif #1883",
  "d = {\"a\": 1, \"b\": 2}\n"
  "post(d[\"a\"])\n"
  "post(d[\"b\"])\n"
  "\n",
  "1\n2", NULL, 0 },
{ "dif #1884",
  "d = {\"a\": 1}\n"
  "d[\"a\"] += 2\n"
  "post(d[\"a\"])\n"
  "\n",
  "3", NULL, 0 },
{ "dif #1885",
  "d = {\"a\": 1}\n"
  "d[\"a\"] = 2\n"
  "post(d[\"a\"])\n"
  "\n",
  "2", NULL, 0 },
{ "dif #1886",
  "d = {\"a\": 1}\n"
  "for each k in d {\n"
  " post(k)\n"
  "}\n"
  "\n",
  NULL, "RuntimeError: for each exige lista, tupla ou string", -1 },
{ "dif #1887",
  "d = {\"a\": 1}\n"
  "match d {\n"
  " case {a: 1} { post(\"casou\") }\n"
  " case _ { post(\"nao\") }\n"
  "}\n"
  "\n",
  "casou", NULL, 0 },
{ "dif #1888",
  "d = {\"a\": 9}\n"
  "match d {\n"
  " case {a: 1} { post(\"casou\") }\n"
  " case _ { post(\"nao\") }\n"
  "}\n"
  "\n",
  "nao", NULL, 0 },
{ "dif #1889",
  "d = {\"a\":1,\"b\":2}\n"
  "d[\"a\"] = 9\n"
  "post(d)\n"
  "\n",
  "{'a': 9, 'b': 2}", NULL, 0 },
{ "dif #1890",
  "d = {\"k\": 7}\n"
  "post(f\"v={d['k']}\")\n"
  "\n",
  "v=7", NULL, 0 },
{ "dif #1891",
  "d = {\"n\": {\"n\": {\"n\": [1, [2, [3, [4]]]]}}}\n"
  "str lixo = \"\"\n"
  "int i = 0\n"
  "while (i < 120000) {\n"
  " \n"
  "\n",
  NULL, "SyntaxError: bloco com '{' nao foi fechado com '}'", -1 },
{ "dif #1892",
  "d = {\"x\": [1, 2], \"y\": {\"z\": 9}}\n"
  "post(d[\"x\"][1])\n"
  "post(d[\"y\"][\"z\"])\n"
  "\n",
  "2\n9", NULL, 0 },
{ "dif #1893",
  "d = {\"x\": [1,2]}\n"
  "d[\"x\"][0] = 7\n"
  "post(d[\"x\"][0])\n"
  "\n",
  "7", NULL, 0 },
{ "dif #1894",
  "d = {\"z\": 1}\n"
  "match d {\n"
  " case {a: 1} { post(\"casou\") }\n"
  " case _ { post(\"nao\") }\n"
  "}\n"
  "\n",
  "nao", NULL, 0 },
{ "dif #1895",
  "d = {nome: \"pool\"}\n"
  "post(d[\"nome\"])\n"
  "\n",
  "pool", NULL, 0 },
{ "dif #1896",
  "d = {}\n"
  "d[\"a\"] = 1\n"
  "post(d[\"a\"])\n"
  "\n",
  "1", NULL, 0 },
{ "dif #1897",
  "d = {}\n"
  "d[\"b\"] = 1\n"
  "d[\"a\"] = 2\n"
  "post(d)\n"
  "\n",
  "{'b': 1, 'a': 2}", NULL, 0 },
{ "dif #1898",
  "d = {}\n"
  "d[\"s\"] = 1\n"
  "d[2] = \"b\"\n"
  "d[2.5] = \"c\"\n"
  "d[true] = \"d\"\n"
  "d[(1, 2)] = \"tup\"\n"
  "post(len(d), d[\"s\"], d[2], d[(1, 2)])\n"
  "\n",
  "5 1 b tup", NULL, 0 },
{ "dif #1899",
  "d = {}\n"
  "i = 0\n"
  "d[i] = 7\n"
  "post(d[0])\n"
  "\n",
  "7", NULL, 0 },
{ "dif #1900",
  "d = {}\n"
  "i = 0\n"
  "while i < 200 {\n"
  " d[i] = i\n"
  " i++\n"
  "}\n"
  "post(len(d))\n"
  "post(list(d)[0])\n"
  "post(list(d)[199])\n"
  "\n",
  "200\n0\n199", NULL, 0 },
{ "dif #1901",
  "d = {}\n"
  "if (d) {\n"
  " post(1)\n"
  "} else {\n"
  " post(2)\n"
  "}\n"
  "\n",
  "2", NULL, 0 },
{ "dif #1902",
  "d = {}\n"
  "k = \"a\"\n"
  "d[k] = 1\n"
  "post(d[\"a\"])\n"
  "\n",
  "1", NULL, 0 },
{ "dif #1903",
  "d = {}\n"
  "post(d.value(), len(d.value()))\n"
  "post(1 in d.value())\n"
  "\n",
  "[] 0\nFalse", NULL, 0 },
{ "dif #1904",
  "d={\"a\":1,\"b\":\"x\"}\n"
  "post(count int in d)\n"
  "\n",
  "1", NULL, 0 },
{ "dif #1905",
  "d={\"a\":1,\"b\":2}\n"
  "d.pop(\"a\")\n"
  "d[\"c\"]=3\n"
  "post(d.keys())\n"
  "\n",
  "['b', 'c']", NULL, 0 },
{ "dif #1906",
  "d={\"a\":1,\"b\":2}\n"
  "post(d.items())\n"
  "\n",
  "[('a', 1), ('b', 2)]", NULL, 0 },
{ "dif #1907",
  "d={\"a\":1,\"b\":2}\n"
  "post(d.keys())\n"
  "\n",
  "['a', 'b']", NULL, 0 },
{ "dif #1908",
  "d={\"a\":1,\"b\":2}\n"
  "post(d.pop(\"a\"))\n"
  "post(d)\n"
  "\n",
  "1\n{'b': 2}", NULL, 0 },
{ "dif #1909",
  "d={\"a\":1}\n"
  "count each int in d {\n"
  " post(_index)\n"
  "}\n"
  "\n",
  "a", NULL, 0 },
{ "dif #1910",
  "d={\"a\":1}\n"
  "d.clear()\n"
  "post(d)\n"
  "\n",
  "{}", NULL, 0 },
{ "dif #1911",
  "d={\"a\":1}\n"
  "d.update(d)\n"
  "post(d)\n"
  "\n",
  "{'a': 1}", NULL, 0 },
{ "dif #1912",
  "d={\"a\":1}\n"
  "d.update({\"b\":2})\n"
  "post(d)\n"
  "\n",
  "{'a': 1, 'b': 2}", NULL, 0 },
{ "dif #1913",
  "d={\"a\":1}\n"
  "e=d.copy()\n"
  "e[\"b\"]=2\n"
  "post(d)\n"
  "post(e)\n"
  "\n",
  "{'a': 1}\n{'a': 1, 'b': 2}", NULL, 0 },
{ "dif #1914",
  "d={\"a\":1}\n"
  "post(d.contains(\"z\"))\n"
  "\n",
  "False", NULL, 0 },
{ "dif #1915",
  "d={\"a\":1}\n"
  "post(d.get(\"a\"))\n"
  "\n",
  "1", NULL, 0 },
{ "dif #1916",
  "d={\"a\":1}\n"
  "post(d.get(\"z\"))\n"
  "\n",
  "null", NULL, 0 },
{ "dif #1917",
  "d={\"a\":1}\n"
  "post(d.get(\"z\",9))\n"
  "\n",
  "9", NULL, 0 },
{ "dif #1918",
  "d={\"a\":1}\n"
  "post(d.has(\"a\"))\n"
  "\n",
  "True", NULL, 0 },
{ "dif #1919",
  "d={\"a\":1}\n"
  "post(d.items())\n"
  "\n",
  "[('a', 1)]", NULL, 0 },
{ "dif #1920",
  "d={\"a\":1}\n"
  "post(d.len())\n"
  "\n",
  "1", NULL, 0 },
{ "dif #1921",
  "d={\"a\":1}\n"
  "post(d.pop(\"z\",7))\n"
  "\n",
  "7", NULL, 0 },
{ "dif #1922",
  "d={\"a\":1}\n"
  "post(d.values())\n"
  "\n",
  "[1]", NULL, 0 },
{ "dif #1923",
  "d={\"b\":1,\"a\":2}\n"
  "post(d.keys())\n"
  "\n",
  "['b', 'a']", NULL, 0 },
{ "dif #1924",
  "d={}\n"
  "post(d.keys())\n"
  "\n",
  "[]", NULL, 0 },
{ "dif #1925",
  "d={}\n"
  "post(d.len())\n"
  "\n",
  "0", NULL, 0 },
{ "dif #1926",
  "dia = \"Terca\"\n"
  "match dia {\n"
  "    case \"Sabado\" | \"Domingo\" {\n"
  "        post(\"fim\")\n"
  "    }\n"
  "    case \"Segunda\" | \"Terca\" | \"Quarta\" {\n"
  "        post(\"inicio\")\n"
  "    }\n"
  "    case _ {\n"
  "        post(\"outro\")\n"
  "    }\n"
  "}\n"
  "\n",
  "inicio", NULL, 0 },
{ "dif #1927",
  "dia=\"Sabado\"\n"
  "match dia {\n"
  "    case \"Sabado\" | \"Domingo\" {\n"
  "        post(\"fim\")\n"
  "    }\n"
  "    case _ {\n"
  "        post(\"semana\")\n"
  "    }\n"
  "}\n"
  "\n",
  "fim", NULL, 0 },
{ "dif #1928",
  "dict = 5\n"
  "\n",
  NULL, "SyntaxError: 'dict' e palavra reservada da linguagem e nao pode ser usada como n", -1 },
{ "dif #1929",
  "dict d = {\"a\": 1}\n"
  "\n",
  "", NULL, 0 },
{ "dif #1930",
  "dict d = {}\n"
  "for each i in range(5000) { d[str(i)] = i }\n"
  "post(len(d), d[\"4999\"])\n"
  "\n",
  "5000 4999", NULL, 0 },
{ "dif #1931",
  "enum Cor {\n"
  "    RED\n"
  "    GREEN\n"
  "    BLUE\n"
  "}\n"
  "post(Cor.RED, Cor.GREEN, Cor.BLUE)\n"
  "\n",
  "0 1 2", NULL, 0 },
{ "dif #1932",
  "enum Cor { RED, GREEN }\n"
  "\n",
  "", NULL, 0 },
{ "dif #1933",
  "enum Cor { RED, GREEN, BLUE }\n"
  "\n",
  "", NULL, 0 },
{ "dif #1934",
  "enum Hex { RED=\"#f00\", GREEN=\"#0f0\" }\n"
  "\n",
  "", NULL, 0 },
{ "dif #1935",
  "enum Mix { A, B=10, C, D=\"x\", E }\n"
  "\n",
  "", NULL, 0 },
{ "dif #1936",
  "enum S {\n"
  "    OK = 200,\n"
  "    NF = 404\n"
  "}\n"
  "post(S.OK, S.NF)\n"
  "\n",
  "200 404", NULL, 0 },
{ "dif #1937",
  "enum Status {\n"
  "    ATIVO\n"
  "    INATIVO\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "dif #1938",
  "enum Vazio { SO }\n"
  "\n",
  "", NULL, 0 },
{ "dif #1939",
  "f = \"abc\".upper\n"
  "post(f())\n"
  "\n",
  "ABC", NULL, 0 },
{ "dif #1940",
  "f = abs\n"
  "post(map([1,-2,3], f))\n"
  "\n",
  "[1, 2, 3]", NULL, 0 },
{ "dif #1941",
  "f = action(a, b) { return a + b }\n"
  "\n",
  "", NULL, 0 },
{ "dif #1942",
  "f = action(x) { return x * 2 }\n"
  "post(f(21))\n"
  "post(map([1, 2, 3], action(n) { return n + 1 }))\n"
  "\n",
  "42\n[2, 3, 4]", NULL, 0 },
{ "dif #1943",
  "f = str\n"
  "\n",
  "", NULL, 0 },
{ "dif #1944",
  "flo action f() { return 1 }\n"
  "\n",
  "", NULL, 0 },
{ "dif #1945",
  "flo f = 1.5\n"
  "\n",
  "", NULL, 0 },
{ "dif #1946",
  "flo reaction precisa(v) { return v }\n"
  "\n",
  "", NULL, 0 },
{ "dif #1947",
  "flo x = \"1.5\"\n"
  "\n",
  "", NULL, 0 },
{ "dif #1948",
  "flo x = \"abc\"\n"
  "\n",
  NULL, "ConversionError: não foi possível converter 'abc' para flo (declarado como 'fl", -1 },
{ "dif #1949",
  "flo x = 5\n"
  "\n",
  "", NULL, 0 },
{ "dif #1950",
  "flo x = 5\n"
  "post(x)\n"
  "\n",
  "5.0", NULL, 0 },
{ "dif #1951",
  "for each base in [1] {\n"
  " post(1)\n"
  "}\n"
  "\n",
  NULL, "SyntaxError: 'base' e palavra reservada da linguagem e nao pode ser usada como n", -1 },
{ "dif #1952",
  "for each base in l {\n"
  " post(1)\n"
  "}\n"
  "\n",
  NULL, "SyntaxError: 'base' e palavra reservada da linguagem e nao pode ser usada como n", -1 },
{ "dif #1953",
  "for each c in \"abc\" {\n"
  " post(c)\n"
  "}\n"
  "\n",
  "a\nb\nc", NULL, 0 },
{ "dif #1954",
  "for each c in \"abc\" {\n"
  "    post(c)\n"
  "}\n"
  "\n",
  "a\nb\nc", NULL, 0 },
{ "dif #1955",
  "for each c in \"ção\" {\n"
  "\n",
  NULL, "SyntaxError: bloco com '{' nao foi fechado com '}'", -1 },
{ "dif #1956",
  "for each i in [\"a\",\"b\",\"c\"] {\n"
  "    post(i)\n"
  "}\n"
  "\n",
  "a\nb\nc", NULL, 0 },
{ "dif #1957",
  "for each i in [1, 2, 3] {\n"
  "    try {\n"
  "        if (i == 2) { break }\n"
  "        post(\"corpo\", i)\n"
  "    } catch (e) {\n"
  "        post(\"c\")\n"
  "    } finally {\n"
  "        post(\"finally\", i)\n"
  "    }\n"
  "}\n"
  "post(\"fim\")\n"
  "\n",
  "corpo 1\nfinally 1\nfinally 2\nfim", NULL, 0 },
{ "dif #1958",
  "for each i in [1, 2, 3] {\n"
  "    post(i)\n"
  "}\n"
  "\n",
  "1\n2\n3", NULL, 0 },
{ "dif #1959",
  "for each i in [1, 2] {\n"
  "    try {\n"
  "        continue\n"
  "    } catch (e) {\n"
  "        post(\"c\")\n"
  "    } finally {\n"
  "        post(\"finally\", i)\n"
  "    }\n"
  "}\n"
  "\n",
  "finally 1\nfinally 2", NULL, 0 },
{ "dif #1960",
  "for each i in [1, 2] {\n"
  "    post(i)\n"
  "}\n"
  "post(i)\n"
  "\n",
  NULL, "NameError: name 'i' is not defined", -1 },
{ "dif #1961",
  "for each i in [1, 2] {\n"
  "    y = i\n"
  "}\n"
  "post(y)\n"
  "\n",
  NULL, "NameError: name 'y' is not defined", -1 },
{ "dif #1962",
  "for each i in [1,2,3,4] {\n"
  " if (i == 2) {\n"
  "  continue\n"
  " }\n"
  " post(i)\n"
  "}\n"
  "\n",
  "1\n3\n4", NULL, 0 },
{ "dif #1963",
  "for each i in [1,2,3,4] {\n"
  " if (i == 3) {\n"
  "  break\n"
  " }\n"
  " post(i)\n"
  "}\n"
  "\n",
  "1\n2", NULL, 0 },
{ "dif #1964",
  "for each i in [1,2,3] {\n"
  " match i {\n"
  "  case 2 { post(\"dois\") }\n"
  "  case _ { post(i) }\n"
  " }\n"
  "}\n"
  "\n",
  "1\ndois\n3", NULL, 0 },
{ "dif #1965",
  "for each i in [1,2,3] {\n"
  " post(f\"i={i}\")\n"
  "}\n"
  "\n",
  "i=1\ni=2\ni=3", NULL, 0 },
{ "dif #1966",
  "for each i in [1,2,3] {\n"
  " post(i)\n"
  "}\n"
  "\n",
  "1\n2\n3", NULL, 0 },
{ "dif #1967",
  "for each i in [1,2,3]:\n"
  "\n",
  NULL, "SyntaxError: bloco com ':' nao existe mais", -1 },
{ "dif #1968",
  "for each i in [1,2] {\n"
  " for each j in [1,2,3] {\n"
  "  if (j == 2) {\n"
  "   break\n"
  "  }\n"
  "  post(j)\n"
  " }\n"
  " post(i)\n"
  "}\n"
  "\n",
  "1\n1\n1\n2", NULL, 0 },
{ "dif #1969",
  "for each i in [1,2] {\n"
  " for each j in [1,2,3] {\n"
  "  if (j == 2) {\n"
  "   continue\n"
  "  }\n"
  "  post(j)\n"
  " }\n"
  "}\n"
  "\n",
  "1\n3\n1\n3", NULL, 0 },
{ "dif #1970",
  "for each i in [1,2] {\n"
  " for each j in [3,4] {\n"
  "  post(j)\n"
  " }\n"
  "}\n"
  "\n",
  "3\n4\n3\n4", NULL, 0 },
{ "dif #1971",
  "for each i in [1,2] {\n"
  " try {\n"
  "  post(1/0)\n"
  " } catch (e) {\n"
  "  post(i)\n"
  " }\n"
  "}\n"
  "\n",
  "1\n2", NULL, 0 },
{ "dif #1972",
  "for each i in [1,2] {\n"
  "    post(i)\n"
  "}\n"
  "\n",
  "1\n2", NULL, 0 },
{ "dif #1973",
  "for each i in [] {\n"
  " post(i)\n"
  "}\n"
  "post(\"fim\")\n"
  "\n",
  "fim", NULL, 0 },
{ "dif #1974",
  "for each i in l {\n"
  " post(i)\n"
  "}\n"
  "\n",
  NULL, "NameError: name 'l' is not defined", -1 },
{ "dif #1975",
  "for each i in range(120000):\n"
  "\n",
  NULL, "SyntaxError: bloco com ':' nao existe mais", -1 },
{ "dif #1976",
  "for each i in range(25) { b = 1103515245 * b + 12345 }\n"
  "\n",
  NULL, "NameError: name 'b' is not defined", -1 },
{ "dif #1977",
  "for each i in range(40) { a = a * 10 }\n"
  "\n",
  NULL, "NameError: name 'a' is not defined", -1 },
{ "dif #1978",
  "for each i in zzz {\n"
  "    post(i)\n"
  "}\n"
  "\n",
  NULL, "NameError: name 'zzz' is not defined", -1 },
{ "dif #1979",
  "for each par in [[1,2],[3,4]] {\n"
  " post(par[0])\n"
  "}\n"
  "\n",
  "1\n3", NULL, 0 },
{ "dif #1980",
  "for each while in [1] {\n"
  " post(1)\n"
  "}\n"
  "\n",
  NULL, "SyntaxError: 'while' e palavra reservada da linguagem e nao pode ser usada como ", -1 },
{ "dif #1981",
  "for each x in (1,2,3) {\n"
  " post(x)\n"
  "}\n"
  "\n",
  "1\n2\n3", NULL, 0 },
{ "dif #1982",
  "for each x in [1, 2, 3] { post(x) }\n"
  "for each i in range(3) { post(i) }\n"
  "\n",
  "1\n2\n3\n0\n1\n2", NULL, 0 },
{ "dif #1983",
  "for each x in [1, 2, 3] {\n"
  "    post(x)\n"
  "}\n"
  "\n",
  "1\n2\n3", NULL, 0 },
{ "dif #1984",
  "for each x in a {\n"
  "\n",
  NULL, "SyntaxError: bloco com '{' nao foi fechado com '}'", -1 },
{ "dif #1985",
  "for each x in f(\"a\" {b}) {\n"
  " post(x)\n"
  "}\n"
  "\n",
  NULL, "NameError: name 'f' is not defined", -1 },
{ "dif #1986",
  "for each z in [1] {\n"
  "    post(z)\n"
  "}\n"
  "post(z)\n"
  "\n",
  NULL, "NameError: name 'z' is not defined", -1 },
{ "dif #1987",
  "from ..logIn import chave\n"
  "action run() { return chave }\n"
  "\n",
  NULL, "ImportError: modulo nao encontrado: ..logIn", -1 },
{ "dif #1988",
  "from ..logIn import logger\n"
  "action run() { return logger() }\n"
  "\n",
  NULL, "ImportError: modulo nao encontrado: ..logIn", -1 },
{ "dif #1989",
  "from .classes.user_struct import person_data\n"
  "action logger() { return person_data }\n"
  "\n",
  NULL, "ImportError: modulo nao encontrado: .classes.user_struct", -1 },
{ "dif #1990",
  "from .nao_existe import x\n"
  "post(x)\n"
  "\n",
  NULL, "ImportError: modulo nao encontrado: .nao_existe", -1 },
{ "dif #1991",
  "from .sibling import valor\n"
  "action pega() { return valor }\n"
  "\n",
  NULL, "ImportError: modulo nao encontrado: .sibling", -1 },
{ "dif #1992",
  "from .sibling import valor\n"
  "post(valor)\n"
  "\n",
  NULL, "ImportError: modulo nao encontrado: .sibling", -1 },
{ "dif #1993",
  "from controll_api.logIn import logger\n"
  "post(logger())\n"
  "\n",
  NULL, "ImportError: modulo nao encontrado: controll_api.logIn", -1 },
{ "dif #1994",
  "from datasentity import dataentity, asdict, astuple, aslist\n"
  "@dataentity\n"
  "Entity P() {\n"
  "    nome: str\n"
  "    idade: int\n"
  "}\n"
  "p = P(nome=\"Ana\", idade=30)\n"
  "post(asdict(p)[\"nome\"])\n"
  "post(astuple(p)[1])\n"
  "post(aslist(p)[0])\n"
  "\n",
  "Ana\n30\nAna", NULL, 0 },
{ "dif #1995",
  "from datasentity import dataentity, asdict, astuple, aslist, asjson\n"
  "@dataentity\n"
  "Entity P() {\n"
  "    nome: str\n"
  "    idade: int\n"
  "}\n"
  "p=P(nome=\"k\",idade=1)\n"
  "post(asdict(p))\n"
  "\n",
  "{'nome': 'k', 'idade': 1}", NULL, 0 },
{ "dif #1996",
  "from datasentity import dataentity, asdict, astuple, aslist, asjson\n"
  "@dataentity\n"
  "Entity P() {\n"
  "    nome: str\n"
  "    idade: int\n"
  "}\n"
  "p=P(nome=\"k\",idade=1)\n"
  "post(asjson(p))\n"
  "\n",
  "{\"nome\": \"k\", \"idade\": 1}", NULL, 0 },
{ "dif #1997",
  "from datasentity import dataentity, asdict, astuple, aslist, asjson\n"
  "@dataentity\n"
  "Entity P() {\n"
  "    nome: str\n"
  "    idade: int\n"
  "}\n"
  "p=P(nome=\"k\",idade=1)\n"
  "post(aslist(p))\n"
  "\n",
  "['k', 1]", NULL, 0 },
{ "dif #1998",
  "from datasentity import dataentity, asdict, astuple, aslist, asjson\n"
  "@dataentity\n"
  "Entity P() {\n"
  "    nome: str\n"
  "    idade: int\n"
  "}\n"
  "p=P(nome=\"k\",idade=1)\n"
  "post(astuple(p))\n"
  "\n",
  "('k', 1)", NULL, 0 },
{ "dif #1999",
  "from datasentity import dataentity, asdict, astuple, aslist, asjson\n"
  "@dataentity\n"
  "Entity P() {\n"
  "    nome: str\n"
  "    idade: int\n"
  "}\n"
  "p=P(nome=\"k\",idade=1)\n"
  "post(p.nome)\n"
  "\n",
  "k", NULL, 0 },
{ "dif #2000",
  "from datasentity import dataentity, asdict, astuple, aslist, asjson\n"
  "post(asdict())\n"
  "\n",
  NULL, "TypeError: asdict expected 1 argument, got 0", -1 },
{ "dif #2001",
  "from datasentity import dataentity, asdict, astuple, aslist, asjson\n"
  "post(asdict(1))\n"
  "\n",
  NULL, "TypeError: asdict() argument 1 must be Entity, not int", -1 },
{ "dif #2002",
  "from datasentity import dataentity, asdict, astuple, aslist, asjson\n"
  "post(astuple(\"x\"))\n"
  "\n",
  NULL, "TypeError: astuple() argument 1 must be Entity, not str", -1 },
{ "dif #2003",
  "from date import hora\n"
  "post(hora(2))\n"
  "\n",
  "7200", NULL, 0 },
/* CORRIGIDO: o caso gravava a data de HOJE ("25/08/2026") e passava a falhar
 * no dia seguinte — bomba-relogio, nao teste. Agora afere a FORMA, que e o que
 * o caso queria dizer: `today()` devolve dd/mm/aaaa. */
{ "dif #2004",
  "from date import today\n"
  "str d = today()\n"
  "post(len(d), d[2], d[5])\n"
  "\n",
  "10 / /", NULL, 0 },
{ "dif #2005",
  "from flask import Flask\n"
  "Flask(\"app\")\n"
  "\n",
  NULL, "NotImplemented: esta funcao ainda nao esta implementada nesta versao da PoolScri", -1 },
{ "dif #2006",
  "from json import parse\n"
  "data = parse(\"{\\\"name\\\": \\\"Pool\\\"}\")\n"
  "post(data[\"name\"])\n"
  "\n",
  "Pool", NULL, 0 },
{ "dif #2007",
  "from json import stringify\n"
  "post(stringify([1, 2, 3]))\n"
  "\n",
  "[1, 2, 3]", NULL, 0 },
{ "dif #2008",
  "from json import stringify\n"
  "post(stringify([1,2]))\n"
  "\n",
  "[1, 2]", NULL, 0 },
{ "dif #2009",
  "from json import stringify as sj, parse\n"
  "post(sj([1]))\n"
  "post(parse(\"[2]\")[0])\n"
  "\n",
  "[1]\n2", NULL, 0 },
{ "dif #2010",
  "from lib import f\n"
  "post(f())\n"
  "\n",
  NULL, "ImportError: modulo nao encontrado: lib", -1 },
{ "dif #2011",
  "from pkg.deep.worker import run\n"
  "post(run())\n"
  "\n",
  NULL, "ImportError: modulo nao encontrado: pkg.deep.worker", -1 },
{ "dif #2012",
  "from pkg.mod import pega\n"
  "from pkg.deep.worker import run\n"
  "post(pega(), run())\n"
  "\n",
  NULL, "ImportError: modulo nao encontrado: pkg.mod", -1 },
{ "dif #2013",
  "from pkg.mod import valor\n"
  "post(valor)\n"
  "\n",
  NULL, "ImportError: modulo nao encontrado: pkg.mod", -1 },
{ "dif #2014",
  "from regex import match, findall, sub, split, escape\n"
  "post(findall(\"\\\\d\", \"a1\"))\n"
  "\n",
  "['1']", NULL, 0 },
{ "dif #2015",
  "from request import get\n"
  "h = {\"X-Token\": \"abc\"}\n"
  "r = get(\"https://example.com\", headers=h)\n"
  "\n",
  "", NULL, 0 },
{ "dif #2016",
  "from request import get as buscar\n"
  "post(\"buscar:\" buscar)\n"
  "\n",
  "buscar: <builtin>", NULL, 0 },
{ "dif #2017",
  "from request import get as buscar\n"
  "post(\"ok\")\n"
  "\n",
  "ok", NULL, 0 },
{ "dif #2018",
  "from request import get as buscar, post as enviar, put\n"
  "post(\"ok\")\n"
  "\n",
  "ok", NULL, 0 },
{ "dif #2019",
  "from request import post as http_post\n"
  "h = {\"X-Token\": \"abc\"}\n"
  "b = {\"name\": \"ana\"}\n"
  "r = http_post(\"https://example.com\", headers=h, body=b)\n"
  "\n",
  "", NULL, 0 },
{ "dif #2020",
  "from services.controllSmtp import enviar\n"
  "post(enviar())\n"
  "\n",
  NULL, "ImportError: modulo nao encontrado: services.controllSmtp", -1 },
{ "dif #2021",
  "from sub.mod import oi\n"
  "post(oi())\n"
  "\n",
  NULL, "ImportError: modulo nao encontrado: sub.mod", -1 },
{ "dif #2022",
  "from zza import checa\n"
  "post(checa({\"v\": 7}))\n"
  "post(checa({\"v\": \"x\"}))\n"
  "\n",
  NULL, "ImportError: modulo nao encontrado: zza", -1 },
{ "dif #2023",
  "from zzb import Interno\n"
  "reaction checa(d) { return d == Interno }\n"
  "\n",
  NULL, "ImportError: modulo nao encontrado: zzb", -1 },
{ "dif #2024",
  "from zzlib import B\n"
  "post(B().nome())\n"
  "\n",
  NULL, "ImportError: modulo nao encontrado: zzlib", -1 },
{ "dif #2025",
  "from zzlib import Contador\n"
  "post(Contador(9).get())\n"
  "\n",
  NULL, "ImportError: modulo nao encontrado: zzlib", -1 },
{ "dif #2026",
  "from zzlib import M1, M2, M3, E1, E2\n"
  "post({\"a\": 1} == M1, {\"b\": \"x\"} == M2, {\"c\": true} == M3)\n"
  "post(E1.X, E1.Y, E2.P, E2.Q)\n"
  "\n",
  NULL, "ImportError: modulo nao encontrado: zzlib", -1 },
{ "dif #2027",
  "from zzlib import Mat\n"
  "post(Mat.soma(2, 3))\n"
  "\n",
  NULL, "ImportError: modulo nao encontrado: zzlib", -1 },
{ "dif #2028",
  "from zzlib import Produto, Cor\n"
  "post({\"nome\": \"x\", \"preco\": 3} == Produto)\n"
  "post(Cor.R, Cor.G, Cor.B)\n"
  "\n",
  NULL, "ImportError: modulo nao encontrado: zzlib", -1 },
{ "dif #2029",
  "from zzlib import Status\n"
  "post(Status.ATIVO, Status.INATIVO)\n"
  "\n",
  NULL, "ImportError: modulo nao encontrado: zzlib", -1 },
{ "dif #2030",
  "from zzlib import Usuario\n"
  "post({\"nome\": \"a\", \"idade\": 5} == Usuario)\n"
  "post({\"nome\": \"a\"} == Usuario)\n"
  "\n",
  NULL, "ImportError: modulo nao encontrado: zzlib", -1 },
{ "dif #2031",
  "from zzlib import dobro\n"
  "post(dobro(21))\n"
  "\n",
  NULL, "ImportError: modulo nao encontrado: zzlib", -1 },
{ "dif #2032",
  "g = 1\n"
  "action f() {\n"
  "    g = 2\n"
  "}\n"
  "f()\n"
  "post(g)\n"
  "\n",
  "2", NULL, 0 },
{ "dif #2033",
  "g = 1\n"
  "action f() {\n"
  "    return g\n"
  "}\n"
  "post(f())\n"
  "\n",
  "1", NULL, 0 },
{ "dif #2034",
  "g = 25\n"
  "j = (\"Resultado: \" {g})\n"
  "post(j)\n"
  "\n",
  "Resultado: 25", NULL, 0 },
{ "dif #2035",
  "g = 25\n"
  "post(\"Clima: \" {g} \"C\")\n"
  "\n",
  "Clima: 25C", NULL, 0 },
{ "dif #2036",
  "g = 25\n"
  "post(\"Clima: \" {g} \"°\")\n"
  "\n",
  "Clima: 25°", NULL, 0 },
{ "dif #2037",
  "g = int\n"
  "\n",
  "", NULL, 0 },
{ "dif #2038",
  "grau = 25\n"
  "post(\"Clima: \" {grau} \" graus\")\n"
  "\n",
  "Clima: 25 graus", NULL, 0 },
{ "dif #2039",
  "grau = 25\n"
  "post(f\"Clima: {grau} graus\")\n"
  "\n",
  "Clima: 25 graus", NULL, 0 },
{ "dif #2040",
  "i = \"importante\"\n"
  "for each i in [1, 2] {\n"
  "    post(i)\n"
  "}\n"
  "post(i)\n"
  "\n",
  "1\n2\nimportante", NULL, 0 },
{ "dif #2041",
  "i = 0\n"
  "resultado = []\n"
  "while (i < 10) {\n"
  "    i = i + 1\n"
  "    if (i % 2 == 0) { continue }\n"
  "    if (i > 7) { break }\n"
  "    addEnd(resultado, i)\n"
  "}\n"
  "post(resultado)\n"
  "\n",
  "[1, 3, 5, 7]", NULL, 0 },
{ "dif #2042",
  "i = 0\n"
  "while i < 3 {\n"
  "    post(i)\n"
  "    i++\n"
  "}\n"
  "\n",
  "0\n1\n2", NULL, 0 },
{ "dif #2043",
  "if (1 == 1) { post(\"a\") }\n"
  "if (2 == 2) {\n"
  "    post(\"b\")\n"
  "}\n"
  "\n",
  "a\nb", NULL, 0 },
{ "dif #2044",
  "if (1 == 1):\n"
  "\tpost(\"x\")\n"
  "\n",
  NULL, "SyntaxError: indentacao com TAB nao e permitida; use 4 espacos", -1 },
{ "dif #2045",
  "if (1 == 1):\n"
  "    if (2 == 2):\n"
  "            post(\"x\")\n"
  "\n",
  NULL, "SyntaxError: indentacao avancou 8 espacos; esperado exatamente 4", -1 },
{ "dif #2046",
  "if (1 == 1):\n"
  "    if (2 == 2):\n"
  "        post(\"x\")\n"
  "      post(\"y\")\n"
  "\n",
  NULL, "SyntaxError: indentacao deve ser multiplo de 4 espacos (achou 6)", -1 },
{ "dif #2047",
  "if (1 == 1) {\n"
  "    post(\"ok\")\n"
  "}\n"
  "\n",
  "ok", NULL, 0 },
{ "dif #2048",
  "if (1 == 1):\n"
  "  post(\"x\")\n"
  "\n",
  NULL, "SyntaxError: indentacao deve ser multiplo de 4 espacos (achou 2)", -1 },
{ "dif #2049",
  "if (count int(1) in a) {\n"
  " post(1)\n"
  "}\n"
  "\n",
  NULL, "NameError: name 'a' is not defined", -1 },
{ "dif #2050",
  "if (x == 1) {\n"
  " post(1)\n"
  "}\n"
  "\n",
  NULL, "NameError: name 'x' is not defined", -1 },
{ "dif #2051",
  "if (x) {\n"
  " post(1)\n"
  "}\n"
  "\n",
  NULL, "NameError: name 'x' is not defined", -1 },
{ "dif #2052",
  "if (x) {\n"
  " post(1)\n"
  "} elif (y) {\n"
  " post(2)\n"
  "} else {\n"
  " post(3)\n"
  "}\n"
  "\n",
  NULL, "NameError: name 'x' is not defined", -1 },
{ "dif #2053",
  "if (x) {\n"
  " post(1)\n"
  "} else {\n"
  " post(2)\n"
  "}\n"
  "\n",
  NULL, "NameError: name 'x' is not defined", -1 },
{ "dif #2054",
  "if (x):\n"
  "\tpost(1)\n"
  "\n",
  NULL, "SyntaxError: indentacao com TAB nao e permitida; use 4 espacos", -1 },
{ "dif #2055",
  "if (x) {\n"
  "    post(1)\n"
  "}\n"
  "\n",
  NULL, "NameError: name 'x' is not defined", -1 },
{ "dif #2056",
  "if (x) {\n"
  "    post(1)\n"
  "    if (y) {\n"
  "        post(2)\n"
  "    }\n"
  "}\n"
  "post(3)\n"
  "\n",
  NULL, "NameError: name 'x' is not defined", -1 },
{ "dif #2057",
  "if (x) {\n"
  "    post(1)\n"
  "}\n"
  "post(2)\n"
  "\n",
  NULL, "NameError: name 'x' is not defined", -1 },
{ "dif #2058",
  "if (x):\n"
  "  post(1)\n"
  "\n",
  NULL, "SyntaxError: indentacao deve ser multiplo de 4 espacos (achou 2)", -1 },
{ "dif #2059",
  "if 1 == 1 {\n"
  "    if 2 == 2 { post(\"misto\") }\n"
  "}\n"
  "\n",
  "misto", NULL, 0 },
{ "dif #2060",
  "if 1 == 1 { post(\"a\") }\n"
  "\n",
  "a", NULL, 0 },
{ "dif #2061",
  "if 1 == 1 { post(\"a\") }\n"
  "if 1 == 1 {\n"
  "    post(\"b\")\n"
  "}\n"
  "\n",
  "a\nb", NULL, 0 },
{ "dif #2062",
  "if 1 == 1:\n"
  "\tpost(\"ruim\")\n"
  "\n",
  NULL, "SyntaxError: indentacao com TAB nao e permitida; use 4 espacos", -1 },
{ "dif #2063",
  "if 1 == 1:\n"
  "        post(\"pulou nivel\")\n"
  "\n",
  NULL, "SyntaxError: indentacao avancou 8 espacos; esperado exatamente 4", -1 },
{ "dif #2064",
  "if 1 == 1 {\n"
  "    if 2 == 2 { post(\"misto2\") }\n"
  "}\n"
  "\n",
  "misto2", NULL, 0 },
{ "dif #2065",
  "if 1 == 1 {\n"
  "    post(\"a\")\n"
  "}\n"
  "\n",
  "a", NULL, 0 },
{ "dif #2066",
  "if 1 == 1:\n"
  "   post(\"ruim\")\n"
  "\n",
  NULL, "SyntaxError: indentacao deve ser multiplo de 4 espacos (achou 3)", -1 },
{ "dif #2067",
  "if 1 == 1:\n"
  "  post(\"ruim\")\n"
  "\n",
  NULL, "SyntaxError: indentacao deve ser multiplo de 4 espacos (achou 2)", -1 },
{ "dif #2068",
  "if = 5\n"
  "\n",
  NULL, "SyntaxError: 'if' e palavra reservada da linguagem e nao pode ser usada como nom", -1 },
{ "dif #2069",
  "if Null == 0 {\n"
  "    post(\"yes\")\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "dif #2070",
  /* Antes: `Null > 0` era False, entao caia no else e imprimia "ok". Agora
   * comparar Null LEVANTA — o `if` nem chega a decidir. O corpo do caso e o
   * mesmo; so a expectativa mudou. */
  "if Null > 0 {\n"
  "    post(\"not\")\n"
  "} else {\n"
  "    post(\"ok\")\n"
  "}\n"
  "\n",
  NULL, "TypeError: '>' not supported between instances of 'Null' and 'int'", -1 },
{ "dif #2071",
  "if a { post(1)\n"
  "      post(2)\n"
  "  post(3) }\n"
  "\n",
  NULL, "NameError: name 'a' is not defined", -1 },
{ "dif #2072",
  "if a { post(1) }\n"
  "\n",
  NULL, "NameError: name 'a' is not defined", -1 },
{ "dif #2073",
  "if a { post(1) }\n"
  "if b {\n"
  "    post(2)\n"
  "}\n"
  "\n",
  NULL, "NameError: name 'a' is not defined", -1 },
{ "dif #2074",
  "if a {\n"
  "    if b {\n"
  "        post(1)\n"
  "    }\n"
  "}\n"
  "\n",
  NULL, "NameError: name 'a' is not defined", -1 },
{ "dif #2075",
  "if a:\n"
  "    if b:\n"
  "        post(1)\n"
  "      post(2)\n"
  "\n",
  NULL, "SyntaxError: indentacao deve ser multiplo de 4 espacos (achou 6)", -1 },
{ "dif #2076",
  "if a {\n"
  "    post(1)\n"
  "\n"
  "    post(2)\n"
  "}\n"
  "\n",
  NULL, "NameError: name 'a' is not defined", -1 },
{ "dif #2077",
  "if a {\n"
  "    post(1)\n"
  "    // comentario\n"
  "    post(2)\n"
  "}\n"
  "\n",
  NULL, "NameError: name 'a' is not defined", -1 },
{ "dif #2078",
  "if false {\n"
  "    x = 1\n"
  "} else {\n"
  "    x = 2\n"
  "}\n"
  "post(x)\n"
  "\n",
  NULL, "NameError: name 'x' is not defined", -1 },
{ "dif #2079",
  "if true {\n"
  "    dentro = 5\n"
  "}\n"
  "post(dentro)\n"
  "\n",
  NULL, "NameError: name 'dentro' is not defined", -1 },
{ "dif #2080",
  "if v > 40 {\n"
  "\n",
  NULL, "SyntaxError: bloco com '{' nao foi fechado com '}'", -1 },
{ "dif #2081",
  "if x:\n"
  "\tpost(1)\n"
  "\n",
  NULL, "SyntaxError: indentacao com TAB nao e permitida; use 4 espacos", -1 },
{ "dif #2082",
  "if x:\n"
  "        post(1)\n"
  "\n",
  NULL, "SyntaxError: indentacao avancou 8 espacos; esperado exatamente 4", -1 },
{ "dif #2083",
  "if x {\n"
  "    post(1)\n"
  "}\n"
  "\n",
  NULL, "NameError: name 'x' is not defined", -1 },
{ "dif #2084",
  "if x {\n"
  "    post(1)\n"
  "}\n"
  "post(2)\n"
  "\n",
  NULL, "NameError: name 'x' is not defined", -1 },
{ "dif #2085",
  "if x {\n"
  "    post(1)\n"
  "}\n"
  "post(2)\n"
  "\n",
  NULL, "NameError: name 'x' is not defined", -1 },
{ "dif #2086",
  "if x:\n"
  "   post(1)\n"
  "\n",
  NULL, "SyntaxError: indentacao deve ser multiplo de 4 espacos (achou 3)", -1 },
{ "dif #2087",
  "if x:\n"
  "  post(1)\n"
  "\n",
  NULL, "SyntaxError: indentacao deve ser multiplo de 4 espacos (achou 2)", -1 },
{ "dif #2088",
  "if zzz {\n"
  "    post(1)\n"
  "}\n"
  "\n",
  NULL, "NameError: name 'zzz' is not defined", -1 },
{ "dif #2089",
  "if {\n"
  "}\n"
  "\n",
  NULL, "SyntaxError: esperado inicio de bloco com '{'", -1 },
{ "dif #2090",
  "import bytes\n"
  "post(\n"
  "\n",
  NULL, "SyntaxError: expressao invalida", -1 },
{ "dif #2091",
  "import bytes\n"
  "x = bytes.concat([bytes.new(\"a\"), 5])\n"
  "\n",
  NULL, "TypeError: operação inválida entre os tipos: bytes.concat: item 1 n", -1 },
{ "dif #2092",
  "import bytes\n"
  "x = bytes.fromhex(\"zz\")\n"
  "\n",
  NULL, "ValueError: valor inválido: bytes.fromhex: hex inválido: 'zz'", -1 },
{ "dif #2093",
  "import bytes\n"
  "x = bytes.fromint(70000, 1)\n"
  "\n",
  NULL, "ValueError: valor inválido: bytes.fromint: 70000 não cabe em 1 byte(s", -1 },
{ "dif #2094",
  "import bytes\n"
  "x = bytes.get(bytes.new(\"ab\"), 9)\n"
  "\n",
  NULL, "ValueError: valor inválido: bytes.get: índice 9 fora do range (0..1)", -1 },
{ "dif #2095",
  "import bytes\n"
  "x = bytes.new(3.5)\n"
  "\n",
  NULL, "TypeError: operação inválida entre os tipos: bytes.new: não sei cr", -1 },
{ "dif #2096",
  "import bytes\n"
  "x = bytes.new([300])\n"
  "\n",
  NULL, "ValueError: valor inválido: bytes.new: a lista precisa conter inteiros", -1 },
{ "dif #2097",
  "import bytes\n"
  "x = bytes.xor(bytes.new(\"a\"), bytes.new())\n"
  "\n",
  NULL, "ValueError: valor inválido: bytes.xor: chave vazia", -1 },
/* CORRIGIDO: mesma bomba-relogio do #2004. O que este caso testa e a
 * INTERPOLACAO `"texto" expr`, nao o valor da data. */
{ "dif #2098",
  "import date\n"
  "h = date.today()\n"
  "post(\"hoje:\" len(h))\n"
  "\n",
  "hoje: 10", NULL, 0 },
{ "dif #2099",
  "import date\n"
  "post(date.datahora().len())\n"
  "\n",
  "19", NULL, 0 },
{ "dif #2100",
  "import date\n"
  "post(date.hora(\"a\"))\n"
  "\n",
  NULL, "TypeError: 'str' object cannot be interpreted as an integer", -1 },
{ "dif #2101",
  "import date\n"
  "post(date.hora())\n"
  "\n",
  "0", NULL, 0 },
{ "dif #2102",
  "import date\n"
  "post(date.hora(0,0,1))\n"
  "\n",
  "86400", NULL, 0 },
{ "dif #2103",
  "import date\n"
  "post(date.hora(0,30))\n"
  "\n",
  "1800", NULL, 0 },
{ "dif #2104",
  "import date\n"
  "post(date.hora(1))\n"
  "\n",
  "3600", NULL, 0 },
{ "dif #2105",
  "import date\n"
  "post(date.hora(1,30,2))\n"
  "\n",
  "178200", NULL, 0 },
{ "dif #2106",
  "import date\n"
  "post(date.timestamp() > 1700000000)\n"
  "\n",
  "True", NULL, 0 },
{ "dif #2107",
  "import date\n"
  "post(date.today().len())\n"
  "\n",
  "10", NULL, 0 },
{ "dif #2108",
  "import date\n"
  "post(date.today(1))\n"
  "\n",
  NULL, "TypeError: today expected 0 arguments, got 1", -1 },
{ "dif #2109",
  "import flask\n"
  "post(\"ok\")\n"
  "\n",
  "ok", NULL, 0 },
{ "dif #2110",
  "import greetlib\n"
  "post(greetlib.greet())\n"
  "\n",
  NULL, "ImportError: modulo nao encontrado: greetlib", -1 },
{ "dif #2111",
  "import json\n"
  "d = json.parse(\"{\\\"a\\\": 42, \\\"b\\\": -7, \\\"c\\\": 2.5}\")\n"
  "post(d[\"a\"], d[\"b\"], d[\"c\"])\n"
  "\n",
  "42 -7 2.5", NULL, 0 },
{ "dif #2112",
  "import json\n"
  "d = json.parse(\"{\\\"n\\\": 123456789012345678901234567890}\")\n"
  "post(d[\"n\"])\n"
  "\n",
  "123456789012345678901234567890", NULL, 0 },
{ "dif #2113",
  "import json\n"
  "f=json.stringify\n"
  "post(f([1]))\n"
  "\n",
  "[1]", NULL, 0 },
{ "dif #2114",
  "import json\n"
  "post(json.naoexiste())\n"
  "\n",
  /* DECISAO 2026-08-25: a msg passou a nomear o modulo e o membro (e a sugerir
   * o nome parecido, quando ha um perto). "nao tem esse membro" nao dizia qual. */
  NULL, "AttributeError: module 'json' has no attribute 'naoexiste'", -1 },
{ "dif #2115",
  "import json\n"
  "post(json.parse(\"  {\\\"a\\\" : [1, {\\\"b\\\": null}] }  \")[\"a\"][1][\"b\"])\n"
  "\n",
  "null", NULL, 0 },
{ "dif #2116",
  "import json\n"
  "post(json.parse(\"\"))\n"
  "\n",
  NULL, "ValueError: Expecting value: line 1 column 1 (char 0)", -1 },
{ "dif #2117",
  "import json\n"
  "post(json.parse(\"1 2\"))\n"
  "\n",
  NULL, "ValueError: Extra data: line 1 column 3 (char 2)", -1 },
{ "dif #2118",
  "import json\n"
  "post(json.parse(\"1.5\"))\n"
  "\n",
  "1.5", NULL, 0 },
{ "dif #2119",
  "import json\n"
  "post(json.parse(\"[1, 2, 3]\")[2])\n"
  "\n",
  "3", NULL, 0 },
{ "dif #2120",
  "import json\n"
  "post(json.parse(\"[1,\"))\n"
  "\n",
  NULL, "ValueError: Expecting value: line 1 column 4 (char 3)", -1 },
{ "dif #2121",
  "import json\n"
  "post(json.parse(\"[1] x\"))\n"
  "\n",
  NULL, "ValueError: Extra data: line 1 column 5 (char 4)", -1 },
{ "dif #2122",
  "import json\n"
  "post(json.parse(\"[]\"))\n"
  "\n",
  "[]", NULL, 0 },
{ "dif #2123",
  "import json\n"
  "post(json.parse(\"\\\"\\\\u00e7\\\"\"))\n"
  "\n",
  "ç", NULL, 0 },
{ "dif #2124",
  "import json\n"
  "post(json.parse(\"lixo\"))\n"
  "\n",
  NULL, "ValueError: Expecting value: line 1 column 1 (char 0)", -1 },
{ "dif #2125",
  "import json\n"
  "post(json.parse(\"null\"))\n"
  "\n",
  "null", NULL, 0 },
{ "dif #2126",
  "import json\n"
  "post(json.parse(\"true\"))\n"
  "\n",
  "True", NULL, 0 },
{ "dif #2127",
  "import json\n"
  "post(json.parse(\"{\"))\n"
  "\n",
  NULL, "ValueError: Expecting property name enclosed in double quotes: line 1 column 2 (char 1)", -1 },
{ "dif #2128",
  "import json\n"
  "post(json.parse(\"{1: 2}\"))\n"
  "\n",
  NULL, "ValueError: Expecting property name enclosed in double quotes: line 1 column 2 (char 1)", -1 },
{ "dif #2129",
  "import json\n"
  "post(json.parse(\"{\\\"n\\\": -1.5e2}\")[\"n\"])\n"
  "\n",
  "-150.0", NULL, 0 },
{ "dif #2130",
  "import json\n"
  "post(json.parse())\n"
  "\n",
  NULL, "TypeError: parse expected 1 argument, got 0", -1 },
{ "dif #2131",
  "import json\n"
  "post(json.parse(1))\n"
  "\n",
  NULL, "TypeError: parse() argument 1 must be str, not int", -1 },
{ "dif #2132",
  "import json\n"
  "post(json.parse([1,2]))\n"
  "\n",
  "[1, 2]", NULL, 0 },
{ "dif #2133",
  "import json\n"
  "post(json.stringify(\"ção\"))\n"
  "\n",
  "\"ção\"", NULL, 0 },
{ "dif #2134",
  "import json\n"
  "post(json.stringify(150.0))\n"
  "\n",
  "150.0", NULL, 0 },
{ "dif #2135",
  "import json\n"
  "post(json.stringify([1,\"a\",True,Null]))\n"
  "\n",
  "[1, \"a\", true, null]", NULL, 0 },
{ "dif #2136",
  "import json\n"
  "post(json.stringify([]))\n"
  "\n",
  "[]", NULL, 0 },
{ "dif #2137",
  "import json\n"
  "post(json.stringify(json.parse(\"{\\\"b\\\":1,\\\"a\\\":2}\")))\n"
  "\n",
  "{\"b\": 1, \"a\": 2}", NULL, 0 },
{ "dif #2138",
  "import json\n"
  "post(json.stringify({\"a\":1}))\n"
  "\n",
  "{\"a\": 1}", NULL, 0 },
{ "dif #2139",
  "import json\n"
  "post(json.stringify({\"n\":1.5,\"l\":[1,2],\"d\":{\"x\":Null}}))\n"
  "\n",
  "{\"n\": 1.5, \"l\": [1, 2], \"d\": {\"x\": null}}", NULL, 0 },
{ "dif #2140",
  "import json\n"
  "post(json.stringify({\"t\":\"as\\\"pas\"}))\n"
  "\n",
  "{\"t\": \"as\\\"pas\"}", NULL, 0 },
{ "dif #2141",
  "import json\n"
  "post(json.stringify({}))\n"
  "\n",
  "{}", NULL, 0 },
{ "dif #2142",
  "import json\n"
  "post(map([[1],[2]], json.stringify))\n"
  "\n",
  "['[1]', '[2]']", NULL, 0 },
{ "dif #2143",
  "import json as j\n"
  "post(j.stringify(\"oi\"))\n"
  "\n",
  "\"oi\"", NULL, 0 },
{ "dif #2144",
  "import os\n"
  "p = os.writeFile(\"__DIR__/sub/nota.txt\", \"linha1\\nábc\")\n"
  "post(p)\n"
  "post(os.readFile(\"__DIR__/sub/nota.txt\"))\n"
  "\n",
  "__DIR__/sub/nota.txt\nlinha1\nábc", NULL, 0 },
{ "dif #2145",
  "import os as sistema\n"
  "post(\"ok\")\n"
  "\n",
  "ok", NULL, 0 },
{ "dif #2146",
  "import regex\n"
  "n = 0\n"
  "for each i in range(2000) {\n"
  "    p = regex.compile(\"a\" + str(i) + \"\\\\d+\")\n"
  "    if p.search(\"a\" + str(i) + \"77\") {\n"
  "        n = n + 1\n"
  "    }\n"
  "}\n"
  "post(n)\n"
  "\n",
  "2000", NULL, 0 },
{ "dif #2147",
  "import regex\n"
  "p = regex.compile(\"\\\\d+\")\n"
  "post(p.match(\"123\"), p.match(\"a123\"), p.fullmatch(\"123\"))\n"
  "post(p.search(\"abc123\"), p.search(\"abc\"))\n"
  "post(p.findall(\"a1b22c333\"))\n"
  "post(p.sub(\"#\", \"a1b22\"))\n"
  "post(p.split(\"a1b22c\"))\n"
  "\n",
  "True False True\nTrue False\n['1', '22', '333']\na#b#\n['a', 'b', 'c']", NULL, 0 },
{ "dif #2148",
  "import regex\n"
  "p = regex.compile(\"\\\\d+\")\n"
  "post(p.sub(\"#\", \"a1b22\"), p.split(\"a1b22c\"))\n"
  "\n",
  "a#b# ['a', 'b', 'c']", NULL, 0 },
{ "dif #2149",
  "import regex\n"
  "p = regex.compile(\"\\\\d+\")\n"
  "post(type(p))\n"
  "post(p.pattern)\n"
  "\n",
  "Pattern\n\\d+", NULL, 0 },
{ "dif #2150",
  "import regex\n"
  "p = regex.compile(\"\\\\d+\")\n"
  "saida = []\n"
  "for each i in range(5) {\n"
  "    saida.append(p.sub(\"#\", \"a1b22\"))\n"
  "    saida.append(str(p.findall(\"x9y8\")))\n"
  "    saida.append(str(p.split(\"a1b2\")))\n"
  "    saida.append(str(p.match(\"77\")))\n"
  "}\n"
  "post(len(saida))\n"
  "post(saida[0], saida[1], saida[2], saida[3])\n"
  "post(saida[16], saida[17], saida[18], saida[19])\n"
  "\n",
  "20\na#b# ['9', '8'] ['a', 'b', ''] True\na#b# ['9', '8'] ['a', 'b', ''] True", NULL, 0 },
{ "dif #2151",
  "import regex\n"
  "post(regex.escape(\"a.b*c\"))\n"
  "\n",
  "a\\.b\\*c", NULL, 0 },
{ "dif #2152",
  "import regex\n"
  "post(regex.findall(\"(?:ab)+\", \"ababab\"))\n"
  "\n",
  "['ababab']", NULL, 0 },
{ "dif #2153",
  "import regex\n"
  "post(regex.findall(\"(\\\\d+)\", \"a1b22\"))\n"
  "\n",
  "['1', '22']", NULL, 0 },
{ "dif #2154",
  "import regex\n"
  "post(regex.findall(\"(\\\\w)(\\\\d)\", \"a1 b2\"))\n"
  "\n",
  "[('a', '1'), ('b', '2')]", NULL, 0 },
{ "dif #2155",
  "import regex\n"
  "post(regex.findall(\"(a|ab)c\", \"abc\"))\n"
  "\n",
  "['ab']", NULL, 0 },
{ "dif #2156",
  "import regex\n"
  "post(regex.findall(\"\\\\d+\", \"a1b22c333\"))\n"
  "\n",
  "['1', '22', '333']", NULL, 0 },
{ "dif #2157",
  "import regex\n"
  "post(regex.findall(\"\\\\w+\", \"olá mundo ção\"))\n"
  "\n",
  "['olá', 'mundo', 'ção']", NULL, 0 },
{ "dif #2158",
  "import regex\n"
  "post(regex.findall(\"a*\", \"bab\"))\n"
  "\n",
  "['', 'a', '', '']", NULL, 0 },
{ "dif #2159",
  "import regex\n"
  "post(regex.findall(\"a+?\", \"aaa\"))\n"
  "\n",
  "['a', 'a', 'a']", NULL, 0 },
{ "dif #2160",
  "import regex\n"
  "post(regex.findall(\"a{2,3}\", \"aaaa\"))\n"
  "\n",
  "['aaa']", NULL, 0 },
{ "dif #2161",
  "import regex\n"
  "post(regex.findall(\"x\", \"aaa\"))\n"
  "\n",
  "[]", NULL, 0 },
{ "dif #2162",
  "import regex\n"
  "post(regex.findall(1, \"a\"))\n"
  "\n",
  NULL, "TypeError: findall() espera str como padrao", -1 },
{ "dif #2163",
  "import regex\n"
  "post(regex.fullmatch(\"\\\\d+\", \"123\"), regex.fullmatch(\"\\\\d+\", \"a123\"))\n"
  "post(regex.fullmatch(\"\\\\d+\", \"123\") == regex.match(\"\\\\d+\", \"123\"))\n"
  "\n",
  "True False\nTrue", NULL, 0 },
{ "dif #2164",
  "import regex\n"
  "post(regex.match(\"(\", \"a\"))\n"
  "\n",
  NULL, "TypeError: regex: faltou ')' (posicao 1)", -1 },
{ "dif #2165",
  "import regex\n"
  "post(regex.match(\"*a\", \"a\"))\n"
  "\n",
  NULL, "TypeError: regex: quantificador sem alvo (posicao 0)", -1 },
{ "dif #2166",
  "import regex\n"
  "post(regex.match(\"[a\", \"a\"))\n"
  "\n",
  NULL, "TypeError: regex: classe nao fechada (posicao 2)", -1 },
{ "dif #2167",
  "import regex\n"
  "post(regex.match(\"[a-z]+\", \"abC\"))\n"
  "\n",
  "False", NULL, 0 },
{ "dif #2168",
  "import regex\n"
  "post(regex.match(\"[a-z]+\", \"abc\"))\n"
  "\n",
  "True", NULL, 0 },
{ "dif #2169",
  "import regex\n"
  "post(regex.match(\"^abc$\", \"abc\"))\n"
  "\n",
  "True", NULL, 0 },
{ "dif #2170",
  "import regex\n"
  "post(regex.match(\"a\", 1))\n"
  "\n",
  NULL, "TypeError: match() argument 2 must be str, not int", -1 },
{ "dif #2171",
  "import regex\n"
  "post(regex.match(\"a\\\\1\", \"a\"))\n"
  "\n",
  NULL, "TypeError: regex: referencia a grupo inexistente (posicao 3)", -1 },
{ "dif #2172",
  "import regex\n"
  "post(regex.match(\"a{2,1}\", \"a\"))\n"
  "\n",
  NULL, "TypeError: regex: {n,m} com m < n (posicao 6)", -1 },
{ "dif #2173",
  "import regex\n"
  "post(regex.search(\"ção\", \"a ção b\"))\n"
  "\n",
  "True", NULL, 0 },
{ "dif #2174",
  "import regex\n"
  "post(regex.split(\",\", \"a,b,,c\"))\n"
  "\n",
  "['a', 'b', '', 'c']", NULL, 0 },
{ "dif #2175",
  "import regex\n"
  "post(regex.split(\"\\\\s*,\\\\s*\", \"a , b,c\"))\n"
  "\n",
  "['a', 'b', 'c']", NULL, 0 },
{ "dif #2176",
  "import regex\n"
  "post(regex.split(\"x\", \"abc\"))\n"
  "\n",
  "['abc']", NULL, 0 },
{ "dif #2177",
  "import regex\n"
  "post(regex.sub(\"(\\\\w+)@(\\\\w+)\", \"\\\\2:\\\\1\", \"eu@casa e tu@la\"))\n"
  "\n",
  "casa:eu e la:tu", NULL, 0 },
{ "dif #2178",
  "import regex\n"
  "post(regex.sub(\"(a)(b)?\", \"[\\\\1|\\\\2]\", \"ab a\"))\n"
  "\n",
  "[a|b] [a|]", NULL, 0 },
{ "dif #2179",
  "import regex\n"
  "post(regex.sub(\"[^\\\\d]\", \"\", \"a1b2c3\"))\n"
  "\n",
  "123", NULL, 0 },
{ "dif #2180",
  "import regex\n"
  "post(regex.sub(\"\\\\d\", \"#\", \"a1b2\", 1))\n"
  "\n",
  "a#b2", NULL, 0 },
{ "dif #2181",
  "import regex\n"
  "post(regex.sub(\"\\\\d+\", \"#\", \"a1b22\"))\n"
  "post(regex.findall(\"\\\\d+\", \"a1b22c333\"))\n"
  "post(regex.split(\"\\\\d+\", \"a1b22c\"))\n"
  "post(regex.match(\"\\\\d+\", \"123\"), regex.search(\"\\\\d+\", \"abc1\"))\n"
  "post(regex.escape(\"a.b*c\"))\n"
  "post(\"a1b22\".sub(\"\\\\d+\", \"#\"), \"a1b22\".findall(\"\\\\d+\"))\n"
  "\n",
  "a#b#\n['1', '22', '333']\n['a', 'b', 'c']\nTrue True\na\\.b\\*c\na#b# ['1', '22']", NULL, 0 },
{ "dif #2182",
  "import regex\n"
  "post(regex.sub(\"^\", \">\", \"abc\"))\n"
  "\n",
  ">abc", NULL, 0 },
{ "dif #2183",
  "import regex\n"
  "post(regex.sub(\"a\"))\n"
  "\n",
  NULL, "TypeError: sub expected at least 3 arguments, got 1", -1 },
{ "dif #2184",
  "import regex\n"
  "post(regex.sub(\"a\", \"b\", \"aaa\", count=2))\n"
  "\n",
  "bba", NULL, 0 },
{ "dif #2185",
  "import regex\n"
  "regex.compile(\"[a-\")\n"
  "\n",
  NULL, "TypeError: regex: classe nao fechada (posicao 3)", -1 },
{ "dif #2186",
  "import regex as rx\n"
  "post(rx.match(\"a+\", \"aaa\"))\n"
  "\n",
  "True", NULL, 0 },
{ "dif #2187",
  "import requests\n"
  "post(type(requests.head))\n"
  "\n",
  "action", NULL, 0 },
{ "dif #2188",
  "import saud\n"
  "post(saud.ola(\"ana\"))\n"
  "\n",
  NULL, "ImportError: modulo nao encontrado: saud", -1 },
{ "dif #2189",
  "import sub.mod\n"
  "post(mod.oi())\n"
  "\n",
  NULL, "ImportError: modulo nao encontrado: sub.mod", -1 },
{ "dif #2190",
  "import sub.mod as sm\n"
  "post(sm.oi())\n"
  "\n",
  NULL, "ImportError: modulo nao encontrado: sub.mod", -1 },
{ "dif #2191",
  "import swagger\n"
  "d = swagger.infos(target=\"__APP__\").title(\"API X\").description(\"d\").version(\"2.0.0\").authorContact().email(\"e@x.com\").name(\"K\").number(\"(27)0\")\n"
  "d.newDoc(folder_name=\"__DOCS__\", contracts=[\"__CONTR__\", \"json\"])\n"
  "d.bundle(lang=\"typescript\", router=\"react_router\")\n"
  "d.metaData(file_type=\".json\", name=\"openapi\")\n"
  "post(d.SwaggerGEN())\n"
  "\n",
  "__DOCS__/openapi.json", NULL, 0 },
{ "dif #2192",
  "import sys\n"
  "sys.stdout.write(\"x\", true)\n"
  "\n",
  "x", NULL, 0 },
{ "dif #2193",
  "import sys\n"
  "sys.stdout.writeln(\"ola\")\n"
  "\n",
  "ola", NULL, 0 },
{ "dif #2194",
  "import sys; print(sys.argv[1])\n"
  "\n",
  NULL, "NameError: name 'print' is not defined", -1 },
{ "dif #2195",
  "int a = 12\n"
  "int b = 10\n"
  "post(a ^ b)\n"
  "\n",
  "6", NULL, 0 },
{ "dif #2196",
  "int a = 5\n"
  "str b = \"oi\"\n"
  "c = 3.14\n"
  "post(a, b, c)\n"
  "\n",
  "5 oi 3.14", NULL, 0 },
{ "dif #2197",
  "int action f() { return 1 }\n"
  "\n",
  "", NULL, 0 },
{ "dif #2198",
  "int action f() { return 1/0 }\n"
  "\n",
  "", NULL, 0 },
{ "dif #2199",
  "int action f() { return 7 }\n"
  "\n",
  "", NULL, 0 },
{ "dif #2200",
  "int action f() { }\n"
  "\n",
  "", NULL, 0 },
{ "dif #2201",
  "int async reaction foo() {\n"
  "    return 7\n"
  "}\n"
  "post(foo())\n"
  "\n",
  "7", NULL, 0 },
{ "dif #2202",
  "int base = 5\n"
  "\n",
  NULL, "SyntaxError: 'base' e palavra reservada da linguagem e nao pode ser usada como n", -1 },
{ "dif #2203",
  "int i = 0\n"
  "int s = 0\n"
  "while (i < 5) {\n"
  " s += i\n"
  " i += 1\n"
  "}\n"
  "post(s)\n"
  "\n",
  "10", NULL, 0 },
{ "dif #2204",
  "int i = 0\n"
  "while (i < 10) {\n"
  " i += 1\n"
  " if (i == 3) {\n"
  "  break\n"
  " }\n"
  " post(i)\n"
  "}\n"
  "\n",
  "1\n2", NULL, 0 },
{ "dif #2205",
  "int i = 0\n"
  "while (i < 5) {\n"
  " i += 1\n"
  " if (i == 2) {\n"
  "  continue\n"
  " }\n"
  " post(i)\n"
  "}\n"
  "\n",
  "1\n3\n4\n5", NULL, 0 },
{ "dif #2206",
  "int i = 0\n"
  "while (i < 5) {\n"
  " i += 1\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "dif #2207",
  "int i = 0\n"
  "while (i < 6) {\n"
  "    i++\n"
  "    if (i == 2) { continue }\n"
  "    if (i == 5) { break }\n"
  "    post(i)\n"
  "}\n"
  "\n",
  "1\n3\n4", NULL, 0 },
{ "dif #2208",
  "int i = 0\n"
  "while (i < 60000) {\n"
  " lixo = [1, 2, 3, 4, 5, 6, 7, 8]\n"
  " i += 1\n"
  "}\n"
  "post(1)\n"
  "\n",
  "1", NULL, 0 },
{ "dif #2209",
  "int i = 0\n"
  "while i < 3 {\n"
  "    post(i)\n"
  "    i = i + 1\n"
  "}\n"
  "\n",
  "0\n1\n2", NULL, 0 },
{ "dif #2210",
  "int n = \n"
  "\n",
  NULL, "SyntaxError: expressao invalida", -1 },
{ "dif #2211",
  "int n = 1\n"
  "int k = 3\n"
  "post(n << k)\n"
  "\n",
  "8", NULL, 0 },
{ "dif #2212",
  "int nota = 6\n"
  "if (nota >= 9) {\n"
  "    post(\"A\")\n"
  "} elif (nota >= 7) {\n"
  "    post(\"B\")\n"
  "} elif (nota >= 5) {\n"
  "    post(\"C\")\n"
  "} else {\n"
  "    post(\"D\")\n"
  "}\n"
  "\n",
  "C", NULL, 0 },
{ "dif #2213",
  "int reaction Clicker() { post(\"clicou\") }\n"
  "\n",
  "", NULL, 0 },
{ "dif #2214",
  "int reaction Clicker() { post(\"ok\") }\n"
  "\n",
  "", NULL, 0 },
{ "dif #2215",
  "int reaction f() { return 1 }\n"
  "\n",
  "", NULL, 0 },
{ "dif #2216",
  "int reaction f() { return 1/0 }\n"
  "post(f())\n"
  "\n",
  "500", NULL, 0 },
{ "dif #2217",
  "int reaction f() { return 42 }\n"
  "post(f())\n"
  "\n",
  "42", NULL, 0 },
{ "dif #2218",
  "int reaction f() { x=1 }\n"
  "post(f())\n"
  "\n",
  "0", NULL, 0 },
{ "dif #2219",
  "int reaction foo() {\n"
  "    return 11\n"
  "}\n"
  "post(foo())\n"
  "\n",
  "11", NULL, 0 },
{ "dif #2220",
  "int reaction precisa(v) { return v }\n"
  "\n",
  "", NULL, 0 },
{ "dif #2221",
  "int s = 0\n"
  "for each i in [1,2,3,4] {\n"
  " s += i\n"
  "}\n"
  "post(s)\n"
  "\n",
  "10", NULL, 0 },
{ "dif #2222",
  "int soma = 0\n"
  "for each i in range(20000) {\n"
  "    lixo = [i, i + 1, i + 2]\n"
  "    tmp = {\"a\": i}\n"
  "    soma = soma + i\n"
  "}\n"
  "post(soma)\n"
  "\n",
  "199990000", NULL, 0 },
{ "dif #2223",
  "int soma = 0\n"
  "list L = []\n"
  "for each i in range(10000) { addEnd(L, i) }\n"
  "for each x in L { soma = soma + x }\n"
  "post(soma)\n"
  "\n",
  "49995000", NULL, 0 },
{ "dif #2224",
  "int x = \"7\"\n"
  "\n",
  "", NULL, 0 },
{ "dif #2225",
  "int x = \"a\"\n"
  "\n",
  NULL, "ConversionError: não foi possível converter 'a' para int (declarado como 'int ", -1 },
{ "dif #2226",
  "int x = \"abc\"\n"
  "\n",
  NULL, "ConversionError: não foi possível converter 'abc' para int (declarado como 'in", -1 },
{ "dif #2227",
  "int x = 0\n"
  "while (x < 3) {\n"
  " x++\n"
  "}\n"
  "post(x)\n"
  "\n",
  "3", NULL, 0 },
{ "dif #2228",
  "int x = 1\n"
  "\n",
  "", NULL, 0 },
{ "dif #2229",
  "int x = 1\n"
  "int x = 2\n"
  "\n",
  "", NULL, 0 },
{ "dif #2230",
  "int x = 1\n"
  "x++\n"
  "x++\n"
  "post(x)\n"
  "x--\n"
  "post(x)\n"
  "\n",
  "3\n2", NULL, 0 },
{ "dif #2231",
  "int x = 10\n"
  "\n",
  "", NULL, 0 },
{ "dif #2232",
  "int x = 10\n"
  "if x > 5 {\n"
  "    post(\"big\")\n"
  "}\n"
  "\n",
  "big", NULL, 0 },
{ "dif #2233",
  "int x = 10\n"
  "post(x)\n"
  "\n",
  "10", NULL, 0 },
{ "dif #2234",
  "int x = 10\n"
  "x \n"
  "\n",
  "", NULL, 0 },
{ "dif #2235",
  "int x = 10\n"
  "x += 5\n"
  "post(x)\n"
  "\n",
  "15", NULL, 0 },
{ "dif #2236",
  "int x = 3\n"
  "\n",
  "", NULL, 0 },
{ "dif #2237",
  "int x = 5\n"
  "post(x)\n"
  "\n",
  "5", NULL, 0 },
{ "dif #2238",
  "int x = 5\n"
  "x++\n"
  "post(x)\n"
  "\n",
  "6", NULL, 0 },
{ "dif #2239",
  "int x = 5\n"
  "x--\n"
  "post(x)\n"
  "\n",
  "4", NULL, 0 },
{ "dif #2240",
  "int x = 5.9\n"
  "\n",
  NULL, "AttributedValueError: variável x esperava int", -1 },
{ "dif #2241",
  "io = \"ola\"\n"
  "post(<red>\"cuuuuu {io}\")\n"
  "\n",
  "[38;2;255;59;48mcuuuuu {io}[0m", NULL, 0 },
{ "dif #2242",
  "io = \"ola\"\n"
  "post(<red>f\"cuuuuu {io}\")\n"
  "\n",
  "[38;2;255;59;48mcuuuuu ola[0m", NULL, 0 },
{ "dif #2243",
  "items = [1, 2, 3]\n"
  "if (2 in items) { post(\"tem2\") }\n"
  "if (9 not in items) { post(\"sem9\") }\n"
  "\n",
  "tem2\nsem9", NULL, 0 },
{ "dif #2244",
  "l = 3 * [1,2]\n"
  "post(len(l))\n"
  "\n",
  "6", NULL, 0 },
{ "dif #2245",
  "l = [\"a\"]\n"
  "l[0] += \"b\"\n"
  "post(l[0])\n"
  "\n",
  "ab", NULL, 0 },
{ "dif #2246",
  "l = [0]\n"
  "if (l) {\n"
  " post(1)\n"
  "} else {\n"
  " post(2)\n"
  "}\n"
  "\n",
  "1", NULL, 0 },
{ "dif #2247",
  "l = [1, 2, 3]\n"
  "a, b, c = l\n"
  "post(a)\n"
  "post(b)\n"
  "post(c)\n"
  "\n",
  "1\n2\n3", NULL, 0 },
{ "dif #2248",
  "l = [1, 2, 3]\n"
  "l[1] = 99\n"
  "post(l)\n"
  "\n",
  "[1, 99, 3]", NULL, 0 },
{ "dif #2249",
  "l = [1, 2]\n"
  "l.append(l)\n"
  "post(l)\n"
  "\n",
  "[1, 2, [...]]", NULL, 0 },
{ "dif #2250",
  "l = [1, 2]\n"
  "s = f\"{l}\"\n"
  "post(type(s))\n"
  "post(l)\n"
  "\n",
  "str\n[1, 2]", NULL, 0 },
{ "dif #2251",
  "l = [1,2,3,4,5]\n"
  "post(l[-2:])\n"
  "\n",
  "[4, 5]", NULL, 0 },
{ "dif #2252",
  "l = [1,2,3,4,5]\n"
  "post(l[1:3])\n"
  "\n",
  "[2, 3]", NULL, 0 },
{ "dif #2253",
  "l = [1,2,3,4,5]\n"
  "post(l[1:4:2])\n"
  "\n",
  "[2, 4]", NULL, 0 },
{ "dif #2254",
  "l = [1,2,3,4,5]\n"
  "post(l[2:])\n"
  "\n",
  "[3, 4, 5]", NULL, 0 },
{ "dif #2255",
  "l = [1,2,3,4,5]\n"
  "post(l[:2])\n"
  "\n",
  "[1, 2]", NULL, 0 },
{ "dif #2256",
  "l = [1,2,3,4,5]\n"
  "post(l[::-1])\n"
  "\n",
  "[5, 4, 3, 2, 1]", NULL, 0 },
{ "dif #2257",
  "l = [1,2,3,4,5]\n"
  "post(l[::2])\n"
  "\n",
  "[1, 3, 5]", NULL, 0 },
{ "dif #2258",
  "l = [1,2,3]\n"
  "i = 0\n"
  "while i < 3 {\n"
  " l[i] = l[i] * 2\n"
  " i++\n"
  "}\n"
  "post(l[0])\n"
  "post(l[1])\n"
  "post(l[2])\n"
  "\n",
  "2\n4\n6", NULL, 0 },
{ "dif #2259",
  "l = [1,2,3]\n"
  "l[-1] = 8\n"
  "post(l[2])\n"
  "\n",
  "8", NULL, 0 },
{ "dif #2260",
  "l = [1,2,3]\n"
  "l[0] = 9\n"
  "post(l[0])\n"
  "\n",
  "9", NULL, 0 },
{ "dif #2261",
  "l = [1,2,3]\n"
  "l[0] = 9\n"
  "post(l[1])\n"
  "\n",
  "2", NULL, 0 },
{ "dif #2262",
  "l = [1,2,3]\n"
  "match l {\n"
  " case [1, 2] { post(\"casou\") }\n"
  " case _ { post(\"nao\") }\n"
  "}\n"
  "\n",
  "nao", NULL, 0 },
{ "dif #2263",
  "l = [1,2]\n"
  "match l {\n"
  " case [1, 2] { post(\"casou\") }\n"
  " case _ { post(\"nao\") }\n"
  "}\n"
  "\n",
  "casou", NULL, 0 },
{ "dif #2264",
  "l = [1,2]\n"
  "post(\"l=\" {l})\n"
  "\n",
  "l=[1, 2]", NULL, 0 },
{ "dif #2265",
  "l = [1,2]\n"
  "post(f\"l={l}\")\n"
  "\n",
  "l=[1, 2]", NULL, 0 },
{ "dif #2266",
  "l = [1,2]\n"
  "post(l[0:99])\n"
  "\n",
  "[1, 2]", NULL, 0 },
{ "dif #2267",
  "l = [1,2]\n"
  "post(removeEnd(l))\n"
  "post(l)\n"
  "\n",
  "2\n[1]", NULL, 0 },
{ "dif #2268",
  "l = [1,2]\n"
  "post(removeStart(l))\n"
  "post(l)\n"
  "\n",
  "1\n[2]", NULL, 0 },
{ "dif #2269",
  "l = [1,2] * 3\n"
  "post(l[3])\n"
  "\n",
  "2", NULL, 0 },
{ "dif #2270",
  "l = [1,2] * 3\n"
  "post(len(l))\n"
  "\n",
  "6", NULL, 0 },
{ "dif #2271",
  "l = [1,2] + [3]\n"
  "post(l[2])\n"
  "\n",
  "3", NULL, 0 },
{ "dif #2272",
  "l = [1,2] + [3]\n"
  "post(len(l))\n"
  "\n",
  "3", NULL, 0 },
{ "dif #2273",
  "l = [1,[2,3]]\n"
  "match l {\n"
  " case [1, [2, 3]] { post(\"casou\") }\n"
  " case _ { post(\"nao\") }\n"
  "}\n"
  "\n",
  "casou", NULL, 0 },
{ "dif #2274",
  "l = [10, 20, 30]\n"
  "d = {\"k\": 9}\n"
  "post(l[0], l[-1], d[\"k\"])\n"
  "\n",
  "10 30 9", NULL, 0 },
{ "dif #2275",
  "l = [10,20]\n"
  "for each x in l {\n"
  " post(x)\n"
  "}\n"
  "\n",
  "10\n20", NULL, 0 },
{ "dif #2276",
  "l = [10]\n"
  "l[0] %= 3\n"
  "post(l[0])\n"
  "\n",
  "1", NULL, 0 },
{ "dif #2277",
  "l = [10]\n"
  "l[0] -= 3\n"
  "post(l[0])\n"
  "\n",
  "7", NULL, 0 },
{ "dif #2278",
  "l = [1]\n"
  "addEnd(l, 2)\n"
  "post(l)\n"
  "\n",
  "[1, 2]", NULL, 0 },
{ "dif #2279",
  "l = [1]\n"
  "addStart(l, 0)\n"
  "post(l)\n"
  "\n",
  "[0, 1]", NULL, 0 },
{ "dif #2280",
  "l = [1]\n"
  "l[0] += 5\n"
  "post(l[0])\n"
  "\n",
  "6", NULL, 0 },
{ "dif #2281",
  "l = [1]\n"
  "m = l + [2]\n"
  "l = l + [9]\n"
  "post(len(l))\n"
  "post(len(m))\n"
  "\n",
  "2\n2", NULL, 0 },
{ "dif #2282",
  "l = [1] * -2\n"
  "post(len(l))\n"
  "\n",
  "0", NULL, 0 },
{ "dif #2283",
  "l = [1] * 0\n"
  "post(len(l))\n"
  "\n",
  "0", NULL, 0 },
{ "dif #2284",
  "l = [3, 1, 2]\n"
  "addEnd(l, 4)\n"
  "addStart(l, 0)\n"
  "post(l)\n"
  "post(sorted([3, 1, 2]))\n"
  "\n",
  "[0, 3, 1, 2, 4]\n[1, 2, 3]", NULL, 0 },
{ "dif #2285",
  "l = [3, 1, 2]\n"
  "l.append(4)\n"
  "l.sort()\n"
  "post(l, l.pop(), l.index(2), l.count(1), l.copy(), l.len())\n"
  "l.reverse()\n"
  "post(l)\n"
  "l.clear()\n"
  "post(l)\n"
  "\n",
  "[1, 2, 3] 4 1 1 [1, 2, 3] 3\n[3, 2, 1]\n[]", NULL, 0 },
{ "dif #2286",
  "l = [3]\n"
  "l[0] *= 4\n"
  "post(l[0])\n"
  "\n",
  "12", NULL, 0 },
{ "dif #2287",
  "l = [[1, 2], [3, 4]]\n"
  "post(l[1][0])\n"
  "\n",
  "3", NULL, 0 },
{ "dif #2288",
  "l = [[1,2]]\n"
  "l[0][1] = 9\n"
  "post(l[0][1])\n"
  "\n",
  "9", NULL, 0 },
{ "dif #2289",
  "l = []\n"
  "addEnd(l,1)\n"
  "addEnd(l,2)\n"
  "addStart(l,0)\n"
  "post(l)\n"
  "\n",
  "[0, 1, 2]", NULL, 0 },
{ "dif #2290",
  "l = []\n"
  "if (l) {\n"
  " post(1)\n"
  "} else {\n"
  " post(2)\n"
  "}\n"
  "\n",
  "2", NULL, 0 },
{ "dif #2291",
  "l = []\n"
  "post(removeEnd(l))\n"
  "\n",
  "null", NULL, 0 },
{ "dif #2292",
  "l = []\n"
  "post(removeStart(l))\n"
  "\n",
  "null", NULL, 0 },
{ "dif #2293",
  "l = [] + []\n"
  "post(len(l))\n"
  "\n",
  "0", NULL, 0 },
{ "dif #2294",
  "l=\"a b\"\n"
  "post(count char in l)\n"
  "\n",
  "2", NULL, 0 },
{ "dif #2295",
  "l=\"abc\"\n"
  "post(count char in l)\n"
  "\n",
  "3", NULL, 0 },
{ "dif #2296",
  "l=[\"b\",\"a\"]\n"
  "l.sort()\n"
  "post(l)\n"
  "\n",
  "['a', 'b']", NULL, 0 },
{ "dif #2297",
  "l=[(1,),(2,)]\n"
  "post(count tup in l)\n"
  "\n",
  "2", NULL, 0 },
{ "dif #2298",
  "l=[1,\"a\",2,True]\n"
  "post(count int in l)\n"
  "\n",
  "2", NULL, 0 },
{ "dif #2299",
  "l=[1,\"a\",2]\n"
  "count each int in l {\n"
  " post(_index)\n"
  "}\n"
  "\n",
  "0\n2", NULL, 0 },
{ "dif #2300",
  "l=[1,\"a\",2]\n"
  "count each int in l {\n"
  " post(_match)\n"
  "}\n"
  "\n",
  "1\n2", NULL, 0 },
{ "dif #2301",
  "l=[1,\"a\"]\n"
  "for each v in l {\n"
  " if v is str {\n"
  "  post(v)\n"
  " }\n"
  "}\n"
  "\n",
  "a", NULL, 0 },
{ "dif #2302",
  "l=[1,\"a\"]\n"
  "post(count str in l)\n"
  "\n",
  "1", NULL, 0 },
{ "dif #2303",
  "l=[1,2,1]\n"
  "l.remove(1)\n"
  "post(l)\n"
  "\n",
  "[2, 1]", NULL, 0 },
{ "dif #2304",
  "l=[1,2,1]\n"
  "post(l.count(1))\n"
  "\n",
  "2", NULL, 0 },
{ "dif #2305",
  "l=[1,2,1]\n"
  "post(l.index(2))\n"
  "\n",
  "1", NULL, 0 },
{ "dif #2306",
  "l=[1,2,3]\n"
  "count each int in l {\n"
  " if _index == 1 {\n"
  "  continue\n"
  " }\n"
  " post(_match)\n"
  "}\n"
  "\n",
  "1\n3", NULL, 0 },
{ "dif #2307",
  "l=[1,2,3]\n"
  "count each int in l {\n"
  " post(\"achei\")\n"
  "}\n"
  "\n",
  "achei\nachei\nachei", NULL, 0 },
{ "dif #2308",
  "l=[1,2,3]\n"
  "post(l.pop(-1))\n"
  "\n",
  "3", NULL, 0 },
{ "dif #2309",
  "l=[1,2,3]\n"
  "post(l.pop(0))\n"
  "\n",
  "1", NULL, 0 },
{ "dif #2310",
  "l=[1,2,3]\n"
  "v=removeEnd(l)\n"
  "post(v)\n"
  "post(l)\n"
  "\n",
  "3\n[1, 2]", NULL, 0 },
{ "dif #2311",
  "l=[1,2,3]\n"
  "v=removeStart(l)\n"
  "post(v)\n"
  "post(l)\n"
  "\n",
  "1\n[2, 3]", NULL, 0 },
{ "dif #2312",
  "l=[1,2,3]\n"
  "x = count each int in l\n"
  "post(x)\n"
  "\n",
  "3", NULL, 0 },
{ "dif #2313",
  "l=[1,2]\n"
  "addEnd(l,3)\n"
  "post(l)\n"
  "\n",
  "[1, 2, 3]", NULL, 0 },
{ "dif #2314",
  "l=[1,2]\n"
  "count each int in l {\n"
  " break\n"
  "}\n"
  "post(\"ok\")\n"
  "\n",
  "ok", NULL, 0 },
{ "dif #2315",
  "l=[1,2]\n"
  "count each int in l {\n"
  " count each int in l {\n"
  "  post(_match)\n"
  " }\n"
  "}\n"
  "\n",
  "1\n2\n1\n2", NULL, 0 },
{ "dif #2316",
  "l=[1,2]\n"
  "count each int in l {\n"
  " post(_count)\n"
  "}\n"
  "\n",
  "2\n2", NULL, 0 },
{ "dif #2317",
  "l=[1,2]\n"
  "count each int in l {\n"
  " post(self)\n"
  "}\n"
  "\n",
  "2\n2", NULL, 0 },
{ "dif #2318",
  "l=[1,2]\n"
  "l.append(3)\n"
  "post(l)\n"
  "\n",
  "[1, 2, 3]", NULL, 0 },
{ "dif #2319",
  "l=[1,2]\n"
  "l.clear()\n"
  "post(l)\n"
  "\n",
  "[]", NULL, 0 },
{ "dif #2320",
  "l=[1,2]\n"
  "l.insert(-1,9)\n"
  "post(l)\n"
  "\n",
  "[1, 9, 2]", NULL, 0 },
{ "dif #2321",
  "l=[1,2]\n"
  "l.insert(0,9)\n"
  "post(l)\n"
  "\n",
  "[9, 1, 2]", NULL, 0 },
{ "dif #2322",
  "l=[1,2]\n"
  "l.insert(99,9)\n"
  "post(l)\n"
  "\n",
  "[1, 2, 9]", NULL, 0 },
{ "dif #2323",
  "l=[1,2]\n"
  "l.reverse()\n"
  "post(l)\n"
  "\n",
  "[2, 1]", NULL, 0 },
{ "dif #2324",
  "l=[1,2]\n"
  "post(l.pop())\n"
  "post(l)\n"
  "\n",
  "2\n[1]", NULL, 0 },
{ "dif #2325",
  "l=[1,7,7,2]\n"
  "post(count int(7) in l)\n"
  "\n",
  "2", NULL, 0 },
{ "dif #2326",
  "l=[1,7,7,2]\n"
  "post(int(7) count in l)\n"
  "\n",
  "2", NULL, 0 },
{ "dif #2327",
  "l=[1.5,1]\n"
  "post(count flo in l)\n"
  "\n",
  "1", NULL, 0 },
{ "dif #2328",
  "l=[1]\n"
  "l.extend([2,3])\n"
  "post(l)\n"
  "\n",
  "[1, 2, 3]", NULL, 0 },
{ "dif #2329",
  "l=[1]\n"
  "l.extend(l)\n"
  "post(l)\n"
  "\n",
  "[1, 1]", NULL, 0 },
{ "dif #2330",
  "l=[1]\n"
  "m=l.copy()\n"
  "m.append(2)\n"
  "post(l)\n"
  "post(m)\n"
  "\n",
  "[1]\n[1, 2]", NULL, 0 },
{ "dif #2331",
  "l=[2,3]\n"
  "addStart(l,1)\n"
  "post(l)\n"
  "\n",
  "[1, 2, 3]", NULL, 0 },
{ "dif #2332",
  "l=[3,1,2]\n"
  "l.sort()\n"
  "post(l)\n"
  "\n",
  "[1, 2, 3]", NULL, 0 },
{ "dif #2333",
  "l=[True,1]\n"
  "post(count bool in l)\n"
  "\n",
  "1", NULL, 0 },
{ "dif #2334",
  "l=[[1,2]]\n"
  "post(f\"v={l[0][1]}\")\n"
  "\n",
  "v=2", NULL, 0 },
{ "dif #2335",
  "l=[[1],[2]]\n"
  "post(count list in l)\n"
  "\n",
  "2", NULL, 0 },
{ "dif #2336",
  "l=[]\n"
  "count each int in l {\n"
  " post(\"nunca\")\n"
  "}\n"
  "post(\"fim\")\n"
  "\n",
  "fim", NULL, 0 },
{ "dif #2337",
  "l=[]\n"
  "l.reverse()\n"
  "post(l)\n"
  "\n",
  "[]", NULL, 0 },
{ "dif #2338",
  "l=[]\n"
  "l.sort()\n"
  "post(l)\n"
  "\n",
  "[]", NULL, 0 },
{ "dif #2339",
  "l=[]\n"
  "post(count int in l)\n"
  "\n",
  "0", NULL, 0 },
{ "dif #2340",
  "l=[]\n"
  "post(removeEnd(l))\n"
  "\n",
  "null", NULL, 0 },
{ "dif #2341",
  "l=[]\n"
  "post(removeStart(l))\n"
  "\n",
  "null", NULL, 0 },
{ "dif #2342",
  "l=[{},{}]\n"
  "post(count dict in l)\n"
  "\n",
  "2", NULL, 0 },
{ "dif #2343",
  "l=[{},{}]\n"
  "post(count json in l)\n"
  "\n",
  "2", NULL, 0 },
{ "dif #2344",
  "list n = [1, 7, 7]\n"
  "post(count int(7) in n)\n"
  "\n",
  "2", NULL, 0 },
{ "dif #2345",
  "list n = [1, 7, 7]\n"
  "post(int(7) count in n)\n"
  "\n",
  "2", NULL, 0 },
{ "dif #2346",
  "list n = [7, 7]\n"
  "action f() {\n"
  "    count each int(7) in n {\n"
  "        return;\n"
  "    }\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "dif #2347",
  "list nums = [1, 7, 7, 17, 7]\n"
  "post(count int(7) in nums)\n"
  "\n",
  "3", NULL, 0 },
{ "dif #2348",
  "list x = (1, 2)\n"
  "\n",
  "", NULL, 0 },
{ "dif #2349",
  "list xs = [7, 7, 7, 8, 7]\n"
  "\n",
  "", NULL, 0 },
{ "dif #2350",
  "lista = [1, 2, 3]\n"
  "x = lista[99]\n"
  "post(x)\n"
  "\n",
  NULL, "IndexError: list index out of range", -1 },
{ "dif #2351",
  "lista = [10, 20, 30]\n"
  "post(lista[-1])\n"
  "post(lista[-3])\n"
  "\n",
  "30\n10", NULL, 0 },
{ "dif #2352",
  "lista = [10, 20, 30]\n"
  "post(lista[0])\n"
  "post(lista[2])\n"
  "\n",
  "10\n30", NULL, 0 },
{ "dif #2353",
  "match 200 {\n"
  "    case 200 {\n"
  "        post(\"ok\")\n"
  "    }\n"
  "    case _ {\n"
  "        post(\"nao\")\n"
  "    }\n"
  "}\n"
  "\n",
  "ok", NULL, 0 },
{ "dif #2354",
  "match 5 {\n"
  "    case 1 {\n"
  "        post(\"a\")\n"
  "    }\n"
  "    case 2 {\n"
  "        post(\"b\")\n"
  "    }\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "dif #2355",
  "match 999 {\n"
  "    case 1 {\n"
  "        post(\"um\")\n"
  "    }\n"
  "    case _ {\n"
  "        post(\"outro\")\n"
  "    }\n"
  "}\n"
  "\n",
  "outro", NULL, 0 },
{ "dif #2356",
  "match s {\n"
  " case \"abc\" { post(1) }\n"
  "}\n"
  "\n",
  NULL, "NameError: name 's' is not defined", -1 },
{ "dif #2357",
  "match x {\n"
  " case -10 { post(1) }\n"
  "}\n"
  "\n",
  NULL, "NameError: name 'x' is not defined", -1 },
{ "dif #2358",
  "match x {\n"
  " case 1 { post(\"um\") }\n"
  " case _ { post(\"outro\") }\n"
  "}\n"
  "\n",
  NULL, "NameError: name 'x' is not defined", -1 },
{ "dif #2359",
  "match x {\n"
  " case 1 | 2 | 3 { post(\"a\") }\n"
  "}\n"
  "\n",
  NULL, "NameError: name 'x' is not defined", -1 },
{ "dif #2360",
  "match x {\n"
  " case True { post(1) }\n"
  " case Null { post(2) }\n"
  "}\n"
  "\n",
  NULL, "NameError: name 'x' is not defined", -1 },
{ "dif #2361",
  "match x {\n"
  " case [1, 2] { post(1) }\n"
  "}\n"
  "\n",
  NULL, "NameError: name 'x' is not defined", -1 },
{ "dif #2362",
  "match x {\n"
  " case [1, [2, 3]] { post(1) }\n"
  "}\n"
  "\n",
  NULL, "NameError: name 'x' is not defined", -1 },
{ "dif #2363",
  "match x {\n"
  " case v if v > 5 { post(1) }\n"
  "}\n"
  "\n",
  NULL, "NameError: name 'x' is not defined", -1 },
{ "dif #2364",
  "match x {\n"
  " case v { post(v) }\n"
  "}\n"
  "\n",
  NULL, "NameError: name 'x' is not defined", -1 },
{ "dif #2365",
  "match x {\n"
  " case {a: 1} { post(1) }\n"
  "}\n"
  "\n",
  NULL, "NameError: name 'x' is not defined", -1 },
{ "dif #2366",
  "match x {\n"
  "    case 1 {\n"
  "        post(\"um\")\n"
  "    }\n"
  "    case _ {\n"
  "        post(\"outro\")\n"
  "    }\n"
  "}\n"
  "\n",
  NULL, "NameError: name 'x' is not defined", -1 },
{ "dif #2367",
  "model Cliente() {\n"
  "    id: int\n"
  "}\n"
  "enum Local {\n"
  "    A\n"
  "    B\n"
  "}\n"
  "public class Meu() {\n"
  "    public reaction oi(self) { return \"main\" }\n"
  "}\n"
  "from zzlib import Produto, Status, Helper\n"
  "post({\"id\": 1} == Cliente)\n"
  "post({\"nome\": \"x\"} == Produto)\n"
  "post(Local.A, Status.ON, Status.OFF)\n"
  "post(Meu().oi(), Helper().oi())\n"
  "\n",
  NULL, "ImportError: modulo nao encontrado: zzlib", -1 },
{ "dif #2368",
  "model D() {\n"
  "    cpf: str(length=3)\n"
  "}\n"
  "post({\"cpf\": \"123\"} == D)\n"
  "post({\"cpf\": \"1234\"} == D)\n"
  "\n",
  "True\nFalse", NULL, 0 },
{ "dif #2369",
  "model Interno() {\n"
  "    v: int\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "dif #2370",
  "model M() {\n"
  " ativo: bool\n"
  "}\n"
  "post({\"ativo\":1} == M)\n"
  "\n",
  "False", NULL, 0 },
{ "dif #2371",
  "model M() {\n"
  " ativo: bool\n"
  "}\n"
  "post({\"ativo\":True} == M)\n"
  "\n",
  "True", NULL, 0 },
{ "dif #2372",
  "model M() {\n"
  " n: int(length=3)\n"
  "}\n"
  "post({\"n\":999} == M)\n"
  "post({\"n\":1000} == M)\n"
  "post({\"n\":-999} == M)\n"
  "\n",
  "True\nFalse\nTrue", NULL, 0 },
{ "dif #2373",
  "model M() {\n"
  " n: str\n"
  "}\n"
  "post({\"n\":Null} == M)\n"
  "\n",
  "False", NULL, 0 },
{ "dif #2374",
  "model M() {\n"
  " n: str(length=3)\n"
  "}\n"
  "post({\"n\":\"ção\"} == M)\n"
  "\n",
  "True", NULL, 0 },
{ "dif #2375",
  "model M() {\n"
  " x: flo\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "dif #2376",
  "model M() {\n"
  " x: flo\n"
  "}\n"
  "post({\"x\":1.5} == M)\n"
  "\n",
  "True", NULL, 0 },
{ "dif #2377",
  "model M() {\n"
  " x: flo\n"
  "}\n"
  "post({\"x\":1} == M)\n"
  "\n",
  "True", NULL, 0 },
{ "dif #2378",
  "model M() { a: int }\n"
  "int ok = 0\n"
  "for each i in range(5000) {\n"
  "    d = {\"a\": i}\n"
  "    if (d == M) { ok = ok + 1 }\n"
  "}\n"
  "post(ok)\n"
  "\n",
  "5000", NULL, 0 },
{ "dif #2379",
  "model M1() {\n"
  "    a: int\n"
  "}\n"
  "model M2() {\n"
  "    b: str\n"
  "}\n"
  "model M3() {\n"
  "    c: bool\n"
  "}\n"
  "enum E1 {\n"
  "    X\n"
  "    Y\n"
  "}\n"
  "enum E2 {\n"
  "    P = 5,\n"
  "    Q\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "dif #2380",
  "model Produto() {\n"
  "    nome: str\n"
  "    preco: int\n"
  "}\n"
  "enum Cor {\n"
  "    R = 10,\n"
  "    G,\n"
  "    B\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "dif #2381",
  "model Produto() {\n"
  "    nome: str\n"
  "}\n"
  "enum Status {\n"
  "    ON\n"
  "    OFF\n"
  "}\n"
  "public class Helper() {\n"
  "    public reaction oi(self) { return \"lib\" }\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "dif #2382",
  "model U() {\n"
  "    nome: str\n"
  "    idade: int\n"
  "}\n"
  "post({\"nome\": \"a\", \"idade\": 5} == U)\n"
  "post({\"nome\": \"a\"} == U)\n"
  "\n",
  "True\nFalse", NULL, 0 },
{ "dif #2383",
  "model U() {\n"
  " nome: str(length=3)\n"
  " idade: int\n"
  "}\n"
  "post(1 == U)\n"
  "\n",
  "False", NULL, 0 },
{ "dif #2384",
  "model U() {\n"
  " nome: str(length=3)\n"
  " idade: int\n"
  "}\n"
  "post(U == {\"nome\":\"ab\",\"idade\":1})\n"
  "\n",
  "True", NULL, 0 },
{ "dif #2385",
  "model U() {\n"
  " nome: str(length=3)\n"
  " idade: int\n"
  "}\n"
  "post(U)\n"
  "\n",
  "<model U>", NULL, 0 },
{ "dif #2386",
  "model U() {\n"
  " nome: str(length=3)\n"
  " idade: int\n"
  "}\n"
  "post(type(U))\n"
  "\n",
  "PoolModel", NULL, 0 },
{ "dif #2387",
  "model U() {\n"
  " nome: str(length=3)\n"
  " idade: int\n"
  "}\n"
  "post({\"nome\":\"ab\",\"idade\":1,\"extra\":9} == U)\n"
  "\n",
  "True", NULL, 0 },
{ "dif #2388",
  "model U() {\n"
  " nome: str(length=3)\n"
  " idade: int\n"
  "}\n"
  "post({\"nome\":\"ab\",\"idade\":1} != U)\n"
  "\n",
  "False", NULL, 0 },
{ "dif #2389",
  "model U() {\n"
  " nome: str(length=3)\n"
  " idade: int\n"
  "}\n"
  "post({\"nome\":\"ab\",\"idade\":1} == U)\n"
  "\n",
  "True", NULL, 0 },
{ "dif #2390",
  "model U() {\n"
  " nome: str(length=3)\n"
  " idade: int\n"
  "}\n"
  "post({\"nome\":\"ab\"} == U)\n"
  "\n",
  "False", NULL, 0 },
{ "dif #2391",
  "model U() {\n"
  " nome: str(length=3)\n"
  " idade: int\n"
  "}\n"
  "post({\"nome\":\"abcd\",\"idade\":1} == U)\n"
  "\n",
  "False", NULL, 0 },
{ "dif #2392",
  "model Usuario() {\n"
  "    idade: int(length=3)\n"
  "}\n"
  "data = {\"idade\": \"vinte\"}\n"
  "if (data == Usuario) {\n"
  "    post(\"valido\")\n"
  "} else {\n"
  "    post(\"invalido\")\n"
  "}\n"
  "\n",
  "invalido", NULL, 0 },
{ "dif #2393",
  "model Usuario() {\n"
  "    nome: str\n"
  "    idade: int\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "dif #2394",
  "model Usuario() {\n"
  "    nome: str(length=3)\n"
  "}\n"
  "data = {\"nome\": \"abcdef\"}\n"
  "if (data == Usuario) {\n"
  "    post(\"valido\")\n"
  "} else {\n"
  "    post(\"invalido\")\n"
  "}\n"
  "\n",
  "invalido", NULL, 0 },
{ "dif #2395",
  "model Usuario() {\n"
  "    nome: str(length=60)\n"
  "    idade: int(length=3)\n"
  "    ativo: bool\n"
  "}\n"
  "data = {\"nome\": \"ana\", \"idade\": 20, \"ativo\": true}\n"
  "if (data == Usuario) {\n"
  "    post(\"valido\")\n"
  "} else {\n"
  "    post(\"invalido\")\n"
  "}\n"
  "\n",
  "valido", NULL, 0 },
{ "dif #2396",
  "model Usuario() {\n"
  "    nome: str(length=60)\n"
  "    idade: int(length=3)\n"
  "}\n"
  "data = {\"nome\": \"ana\"}\n"
  "if (data == Usuario) {\n"
  "    post(\"valido\")\n"
  "} else {\n"
  "    post(\"invalido\")\n"
  "}\n"
  "\n",
  "invalido", NULL, 0 },
{ "dif #2397",
  "model Usuario() {\n"
  " nome: str(length=60)\n"
  " idade: int(length=3)\n"
  " ativo: bool\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "dif #2398",
  "n = 0\n"
  "while n < 1 {\n"
  "    n = n + 1\n"
  "    z = 9\n"
  "}\n"
  "post(z)\n"
  "\n",
  NULL, "NameError: name 'z' is not defined", -1 },
{ "dif #2399",
  "n = 25\n"
  "post(<00ff00>f\"grau {n}\")\n"
  "\n",
  "[38;2;0;255;0mgrau 25[0m", NULL, 0 },
{ "dif #2400",
  "n = 5\n"
  "if (n > 10) { post(\"g\") } elif (n > 3) { post(\"m\") } else { post(\"p\") }\n"
  "\n",
  "m", NULL, 0 },
{ "dif #2401",
  "n = 7\n"
  "post(\"v=\" {n})\n"
  "\n",
  "v=7", NULL, 0 },
{ "dif #2402",
  "nome = \"pool\"\n"
  "post(f\"Ola, {nome}!\")\n"
  "\n",
  "Ola, pool!", NULL, 0 },
{ "dif #2403",
  "nums = [1, 7, 2, 7, 3, 7]\n"
  "post(count int(7) in nums)\n"
  "\n",
  "3", NULL, 0 },
{ "dif #2404",
  "nums = [1, 7, 2, 7]\n"
  "post(int(7) count in nums)\n"
  "\n",
  "2", NULL, 0 },
{ "dif #2405",
  "nums = [1,2,3]\n"
  "if (count int(2) in nums) { post(\"achou\") }\n"
  "\n",
  "achou", NULL, 0 },
{ "dif #2406",
  "nums = [1,2,3]\n"
  "if (count int(9) in nums) { post(\"achou\") } else { post(\"nao\") }\n"
  "\n",
  "nao", NULL, 0 },
{ "dif #2407",
  "nums = [7, 1, 7, 2, 7]\n"
  "count each int(7) in nums {\n"
  "    post(_index)\n"
  "}\n"
  "\n",
  "0\n2\n4", NULL, 0 },
};
const int NC_DIFERENCIAL = N_CASOS(CASOS_DIFERENCIAL);
