/*
 * Casos de COBERTURA — gerados a partir de duas fontes, não escritos à mão:
 *
 *  1. o corpus de testes do repositório (commit f90845d), de onde saíram os
 *     programas `.ps` E a resposta esperada de cada um; só entraram aqui os
 *     que a VM de hoje confirma — divergência virou investigação, não caso.
 *  2. a matriz de INTERAÇÃO: cada feature nova exercitada dentro de cada
 *     contexto da linguagem (action, método, aninhada, for, while, try,
 *     finally, gerador, async, using, match, bloco `:` e bloco `{}`).
 *     Passar isolado não prova nada — o que quebra é a combinação.
 */
#include "ps_teste.h"

const Caso CASOS_COBERTURA[] = {
{ "git: test_async_deep #0",
  "async action lenta() {\n"
  "    sleep(0.3)\n"
  "    return 1\n"
  "}\n"
  "f = lenta()\n"
  "post(\"chamou\")\n"
  "\n",
  "chamou", NULL, 0 },
{ "git: test_async_deep #1",
  "async action lenta(n) {\n"
  "    sleep(0.3)\n"
  "    return n\n"
  "}\n"
  "a = lenta(1)\n"
  "b = lenta(2)\n"
  "post(await a)\n"
  "post(await b)\n"
  "\n",
  "1\n2", NULL, 0 },
{ "git: test_async_deep #2",
  "async action falha() {\n"
  "    x = 1 / 0\n"
  "    return x\n"
  "}\n"
  "f = falha()\n"
  "sleep(0.05)\n"
  "post(\"nao_lancou_ainda\")\n"
  "\n",
  "nao_lancou_ainda", NULL, 0 },
{ "git: test_async_deep #3",
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
{ "git: test_async_deep #4",
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
{ "git: test_async_deep #5",
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
{ "git: test_async_deep #6",
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
{ "git: test_async_deep #7",
  "async action ident(n) {\n"
  "    sleep(0.05)\n"
  "    return n\n"
  "}\n"
  "a = ident(1)\n"
  "b = ident(2)\n"
  "c = ident(3)\n"
  "post(await c)\n"
  "post(await b)\n"
  "post(await a)\n"
  "\n",
  "3\n2\n1", NULL, 0 },
{ "git: test_async_deep #8",
  "async action semRetorno() {\n"
  "    x = 1 + 1\n"
  "}\n"
  "post(await semRetorno())\n"
  "\n",
  "Null", NULL, 0 },
{ "git: test_async_deep #9",
  "async action vazio() {\n"
  "    return\n"
  "}\n"
  "post(await vazio())\n"
  "\n",
  "Null", NULL, 0 },
{ "git: test_async_deep #10",
  "post(await 42)\n"
  "\n",
  "42", NULL, 0 },
{ "git: test_async_deep #11",
  "post(await \"texto\")\n"
  "\n",
  "texto", NULL, 0 },
{ "git: test_async_deep #12",
  "post(await Null)\n"
  "\n",
  "Null", NULL, 0 },
{ "git: test_async_deep #13",
  "async int reaction f() {\n"
  "    return 1 / 0\n"
  "}\n"
  "post(await f())\n"
  "\n",
  "500", NULL, 0 },
{ "git: test_async_deep #14",
  "async bool reaction f() {\n"
  "    return 1 / 0\n"
  "}\n"
  "post(await f())\n"
  "\n",
  "False", NULL, 0 },
{ "git: test_bitwise_ops #15",
  "post(5 ^ 3)\n"
  "\n",
  "6", NULL, 0 },
{ "git: test_bitwise_ops #16",
  "post(5 | 2)\n"
  "\n",
  "7", NULL, 0 },
{ "git: test_bitwise_ops #17",
  "post(6 & 3)\n"
  "\n",
  "2", NULL, 0 },
{ "git: test_bitwise_ops #18",
  "post(1 << 4)\n"
  "\n",
  "16", NULL, 0 },
{ "git: test_bitwise_ops #19",
  "post(256 >> 4)\n"
  "\n",
  "16", NULL, 0 },
{ "git: test_bitwise_ops #20",
  "post(~5)\n"
  "\n",
  "-6", NULL, 0 },
{ "git: test_bitwise_ops #21",
  "post(~0)\n"
  "\n",
  "-1", NULL, 0 },
{ "git: test_bitwise_ops #22",
  "post(2 + 3 << 1)\n"
  "\n",
  "10", NULL, 0 },
{ "git: test_bitwise_ops #23",
  "post(1 == 1 | 0)\n"
  "\n",
  "True", NULL, 0 },
{ "git: test_bitwise_ops #24",
  "post(8 & 4 ^ 2 | 1)\n"
  "\n",
  "3", NULL, 0 },
{ "git: test_bitwise_ops #25",
  "post(1 << 2 & 6)\n"
  "\n",
  "4", NULL, 0 },
{ "git: test_bitwise_ops #26",
  "post((2 + 3) << 1)\n"
  "\n",
  "10", NULL, 0 },
{ "git: test_bitwise_ops #27",
  "post(2 + (3 << 1))\n"
  "\n",
  "8", NULL, 0 },
{ "git: test_bitwise_ops #28",
  "int a = 12\n"
  "int b = 10\n"
  "post(a ^ b)\n"
  "\n",
  "6", NULL, 0 },
{ "git: test_bitwise_ops #29",
  "int n = 1\n"
  "int k = 3\n"
  "post(n << k)\n"
  "\n",
  "8", NULL, 0 },
{ "git: test_bitwise_ops #30",
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
{ "git: test_bitwise_ops #31",
  "list nums = [1, 7, 7, 17, 7]\n"
  "post(count int(7) in nums)\n"
  "\n",
  "3", NULL, 0 },
{ "git: test_bugfixes_v0_2_0 #32",
  "x = 42\n"
  "post(\"valor:\" x)\n"
  "\n",
  "valor: 42", NULL, 0 },
{ "git: test_bugfixes_v0_2_0 #33",
  "a = 1\n"
  "b = 2\n"
  "post(\"a\" a \"b\" b)\n"
  "\n",
  "a 1 b 2", NULL, 0 },
{ "git: test_bugfixes_v0_2_0 #34",
  "import os as sistema\n"
  "post(\"ok\")\n"
  "\n",
  "ok", NULL, 0 },
{ "git: test_bugfixes_v0_2_0 #35",
  "from request import get as buscar\n"
  "post(\"ok\")\n"
  "\n",
  "ok", NULL, 0 },
{ "git: test_bugfixes_v0_2_0 #36",
  "from request import get as buscar, post as enviar, put\n"
  "post(\"ok\")\n"
  "\n",
  "ok", NULL, 0 },
{ "git: test_bugfixes_v0_2_0 #37",
  "PUSH os GET getenv as ler_env\n"
  "post(\"ok\")\n"
  "\n",
  "ok", NULL, 0 },
{ "git: test_builtins_c #38",
  "post(1/3)\n"
  "\n",
  "0.3333333333333333", NULL, 0 },
{ "git: test_builtins_c #39",
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
{ "git: test_builtins_c #40",
  "post(Parsing.integer(123.7))\n"
  "\n",
  "123", NULL, 0 },
{ "git: test_builtins_c #41",
  "post(Parsing.floating(\"1.299,90\"))\n"
  "\n",
  "1299.9", NULL, 0 },
{ "git: test_builtins_c #42",
  "post(Parsing.floating(\"1,5\"))\n"
  "\n",
  "1.5", NULL, 0 },
{ "git: test_builtins_c #43",
  "post(Parsing.floating(\"12.5\"))\n"
  "\n",
  "12.5", NULL, 0 },
{ "git: test_builtins_c #44",
  "post(Parsing.boolean(\"false\"))\n"
  "\n",
  "False", NULL, 0 },
{ "git: test_builtins_c #45",
  "post(Parsing.boolean(\"x\"))\n"
  "\n",
  "True", NULL, 0 },
{ "git: test_builtins_c #46",
  "post(bool(\"false\"))\n"
  "\n",
  "True", NULL, 0 },
{ "git: test_builtins_c #47",
  "post(\"{}\".format([Null]))\n"
  "\n",
  "[Null]", NULL, 0 },
{ "git: test_builtins_c #48",
  "post(\"\".maketrans(\"ab\",\"xy\"))\n"
  "\n",
  "{97: 120, 98: 121}", NULL, 0 },
{ "git: test_builtins_c #49",
  "post(len(\"ab\".encode()))\n"
  "\n",
  "2", NULL, 0 },
{ "git: test_builtins_c #50",
  "post(\"ab\".encode() == \"ab\")\n"
  "\n",
  "False", NULL, 0 },
{ "git: test_builtins_c #51",
  "post(\"ab\".encode() == \"ab\".encode())\n"
  "\n",
  "True", NULL, 0 },
{ "git: test_builtins_c #52",
  "post(\"\")\n"
  "\n",
  "", NULL, 0 },
{ "git: test_consistency #53",
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
{ "git: test_consistency #54",
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
{ "git: test_consistency #55",
  "\n"
  "l = [3, 1, 2]\n"
  "post(l.count(1))\n"
  "post(l.index(2))\n"
  "\n",
  "1\n2", NULL, 0 },
{ "git: test_consistency #56",
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
{ "git: test_interpreter #57",
  "post(\"oi\")\n"
  "\n",
  "oi", NULL, 0 },
{ "git: test_interpreter #58",
  "x = 10\n"
  "post(x)\n"
  "\n",
  "10", NULL, 0 },
{ "git: test_interpreter #59",
  "str nome = \"Pool\"\n"
  "post(nome)\n"
  "\n",
  "Pool", NULL, 0 },
{ "git: test_interpreter #60",
  "if Null == 0 {\n"
  "    post(\"yes\")\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "git: test_interpreter #61",
  "x = Null > 0\n"
  "post(x)\n"
  "\n",
  NULL, "'>' not supported between instances of 'Null' and 'int'", -1 },
{ "git: test_interpreter #62",
  "lista = [1, 2, 3]\n"
  "x = lista[99]\n"
  "post(x)\n"
  "\n",
  NULL, "list index out of range", -1 },
{ "git: test_interpreter #63",
  "for each i in [\"a\",\"b\",\"c\"] {\n"
  "    post(i)\n"
  "}\n"
  "\n",
  "a\nb\nc", NULL, 0 },
{ "git: test_interpreter #64",
  "action sum(a, b) { return a + b }\n"
  "post(sum(2, 3))\n"
  "\n",
  "5", NULL, 0 },
{ "git: test_interpreter #65",
  "action f() { local = 99 }\n"
  "f()\n"
  "post(\"ok\")\n"
  "\n",
  "ok", NULL, 0 },
{ "git: test_interpreter #66",
  "x = 10\n"
  "post(f\"v={x}\")\n"
  "\n",
  "v=10", NULL, 0 },
{ "git: test_interpreter #67",
  "g = 25\n"
  "post(\"Clima: \" {g} \"°\")\n"
  "\n",
  "Clima: 25°", NULL, 0 },
{ "git: test_interpreter #68",
  "g = 25\n"
  "j = (\"Resultado: \" {g})\n"
  "post(j)\n"
  "\n",
  "Resultado: 25", NULL, 0 },
{ "git: test_interpreter #69",
  "i = 0\n"
  "while i < 3 {\n"
  "    post(i)\n"
  "    i++\n"
  "}\n"
  "\n",
  "0\n1\n2", NULL, 0 },
{ "git: test_language_deep #70",
  "int x = 5\n"
  "post(x)\n"
  "\n",
  "5", NULL, 0 },
{ "git: test_language_deep #71",
  "flo x = 5\n"
  "post(x)\n"
  "\n",
  "5.0", NULL, 0 },
{ "git: test_language_deep #72",
  "a = Null\n"
  "b = null\n"
  "c = None\n"
  "d = none\n"
  "post(a == b)\n"
  "post(b == c)\n"
  "post(c == d)\n"
  "\n",
  "True\nTrue\nTrue", NULL, 0 },
{ "git: test_language_deep #73",
  "post(Null == 0)\n"
  "\n",
  "False", NULL, 0 },
{ "git: test_language_deep #74",
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
{ "git: test_language_deep #75",
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
{ "git: test_language_deep #76",
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
{ "git: test_language_deep #77",
  "resultado = []\n"
  "for each c in \"abc\" {\n"
  "    addEnd(resultado, c)\n"
  "}\n"
  "post(resultado)\n"
  "\n",
  "['a', 'b', 'c']", NULL, 0 },
{ "git: test_language_deep #78",
  "try {\n"
  "    x = 1 / 0\n"
  "} catch (ZeroDivisionError e) {\n"
  "    post(\"tipo_certo\")\n"
  "} catch (e) {\n"
  "    post(\"generico\")\n"
  "}\n"
  "\n",
  "tipo_certo", NULL, 0 },
{ "git: test_language_deep #79",
  "try {\n"
  "    raise \"algo\"\n"
  "} catch (TypeError e) {\n"
  "    post(\"tipo_certo\")\n"
  "} catch (e) {\n"
  "    post(\"generico:\" e)\n"
  "}\n"
  "\n",
  "generico: algo (linha 2)", NULL, 0 },
{ "git: test_language_deep #80",
  "try {\n"
  "    post(\"a\")\n"
  "} catch (e) {\n"
  "    post(\"nunca\")\n"
  "} finally {\n"
  "    post(\"fim\")\n"
  "}\n"
  "\n",
  "a\nfim", NULL, 0 },
{ "git: test_language_deep #81",
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
{ "git: test_language_deep #82",
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
{ "git: test_language_deep #83",
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
{ "git: test_language_deep #84",
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
{ "git: test_language_deep #85",
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
{ "git: test_language_deep #86",
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
{ "git: test_language_deep #87",
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
{ "git: test_language_deep #88",
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
{ "git: test_language_deep #89",
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
{ "git: test_language_deep #90",
  "Entity Util() {\n"
  "    @static\n"
  "    action triplo(n) {\n"
  "        return n * 3\n"
  "    }\n"
  "}\n"
  "post(Util.triplo(4))\n"
  "\n",
  "12", NULL, 0 },
{ "git: test_language_deep #91",
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
{ "git: test_language_deep #92",
  "@NonNull\n"
  "action precisa(v) {\n"
  "    return v\n"
  "}\n"
  "post(precisa(5))\n"
  "\n",
  "5", NULL, 0 },
{ "git: test_language_deep #93",
  "nums = [1, 7, 2, 7, 3, 7]\n"
  "post(count int(7) in nums)\n"
  "\n",
  "3", NULL, 0 },
{ "git: test_language_deep #94",
  "nums = [1, 7, 2, 7]\n"
  "post(int(7) count in nums)\n"
  "\n",
  "2", NULL, 0 },
{ "git: test_language_deep #95",
  "post(count int in [1, \"a\", 2, \"b\", 3])\n"
  "\n",
  "3", NULL, 0 },
{ "git: test_language_deep #96",
  "nums = [7, 1, 7, 2, 7]\n"
  "count each int(7) in nums {\n"
  "    post(_index)\n"
  "}\n"
  "\n",
  "0\n2\n4", NULL, 0 },
{ "git: test_language_deep #97",
  "action total() {\n"
  "    nums = [7, 1, 7, 2, 7]\n"
  "    count each int(7) in nums {\n"
  "        return;\n"
  "    }\n"
  "}\n"
  "post(total())\n"
  "\n",
  "3", NULL, 0 },
{ "git: test_language_deep #98",
  "action primeiro() {\n"
  "    nums = [1, 2, 7, 9, 7]\n"
  "    count each int(7) in nums {\n"
  "        return _index\n"
  "    }\n"
  "}\n"
  "post(primeiro())\n"
  "\n",
  "2", NULL, 0 },
{ "git: test_language_deep #99",
  "nums = [1,2,3]\n"
  "if (count int(2) in nums) { post(\"achou\") }\n"
  "\n",
  "achou", NULL, 0 },
{ "git: test_language_deep #100",
  "nums = [1,2,3]\n"
  "if (count int(9) in nums) { post(\"achou\") } else { post(\"nao\") }\n"
  "\n",
  "nao", NULL, 0 },
{ "git: test_language_deep #101",
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
{ "git: test_language_deep #102",
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
{ "git: test_language_deep #103",
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
{ "git: test_language_deep #104",
  "a = \"x\"\n"
  "b = 1\n"
  "c = 1.5\n"
  "d = true\n"
  "if (a is str and b is int and c is flo and d is bool) {\n"
  "    post(\"todos_ok\")\n"
  "}\n"
  "\n",
  "todos_ok", NULL, 0 },
{ "git: test_language_deep #105",
  "x = \"texto\"\n"
  "if (x not is int) { post(\"a\") }\n"
  "if (x is not None) { post(\"b\") }\n"
  "\n",
  "a\nb", NULL, 0 },
{ "git: test_language_deep #106",
  "items = [1, 2, 3]\n"
  "if (2 in items) { post(\"tem2\") }\n"
  "if (9 not in items) { post(\"sem9\") }\n"
  "\n",
  "tem2\nsem9", NULL, 0 },
{ "git: test_language_deep #107",
  "a = true\n"
  "b = false\n"
  "if (a and not b) { post(\"1\") }\n"
  "if (a or b) { post(\"2\") }\n"
  "if (not (a and b)) { post(\"3\") }\n"
  "if (not a and b) { post(\"nao\") } else { post(\"4\") }\n"
  "\n",
  "1\n2\n3\n4", NULL, 0 },
{ "git: test_language_deep #108",
  "grau = 25\n"
  "post(f\"Clima: {grau} graus\")\n"
  "\n",
  "Clima: 25 graus", NULL, 0 },
{ "git: test_language_deep #109",
  "grau = 25\n"
  "post(\"Clima: \" {grau} \" graus\")\n"
  "\n",
  "Clima: 25 graus", NULL, 0 },
{ "git: test_language_deep #110",
  "post(r\"C:\\Users\\test\")\n"
  "\n",
  "C:\\Users\\test", NULL, 0 },
{ "git: test_language_deep #111",
  "if (1 == 1) {\n"
  "    post(\"ok\")\n"
  "}\n"
  "\n",
  "ok", NULL, 0 },
{ "git: test_language_deep #112",
  "if (1 == 1) { post(\"a\") }\n"
  "if (2 == 2) {\n"
  "    post(\"b\")\n"
  "}\n"
  "\n",
  "a\nb", NULL, 0 },
{ "git: test_language_deep #113",
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
{ "git: test_language_deep #114",
  "action soma(a, b) {\n"
  "    return a + b\n"
  "}\n"
  "post(soma(1, 2))\n"
  "post(soma(3, 4))\n"
  "post(soma(5, 6))\n"
  "\n",
  "3\n7\n11", NULL, 0 },
{ "git: test_language_deep #115",
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
{ "git: test_language_deep #116",
  "post(enumerate([10, 20]))\n"
  "\n",
  "[(0, 10), (1, 20)]", NULL, 0 },
{ "git: test_language_deep #117",
  "resultado = []\n"
  "for each par in enumerate([10, 20]) { addEnd(resultado, par) }\n"
  "post(resultado)\n"
  "\n",
  "[(0, 10), (1, 20)]", NULL, 0 },
{ "git: test_language_deep #118",
  "post(map([1, 2, 3], action(x) { return x * 2 }))\n"
  "\n",
  "[2, 4, 6]", NULL, 0 },
{ "git: test_language_deep #119",
  "post(filter([1, 2, 3], action(x) { return x > 1 }))\n"
  "\n",
  "[2, 3]", NULL, 0 },
{ "git: test_language_deep #120",
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
{ "git: test_language_deep #121",
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
{ "git: test_language_deep #122",
  "action registrar() {\n"
  "    global visitas\n"
  "    visitas = 1\n"
  "}\n"
  "registrar()\n"
  "post(visitas)\n"
  "\n",
  "1", NULL, 0 },
{ "git: test_language_deep #123",
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
{ "git: test_language_deep #124",
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
{ "git: test_libs #125",
  "from json import stringify\n"
  "post(stringify([1, 2, 3]))\n"
  "\n",
  "[1, 2, 3]", NULL, 0 },
{ "git: test_libs #126",
  "from json import parse\n"
  "data = parse(\"{\\\"name\\\": \\\"Pool\\\"}\")\n"
  "post(data[\"name\"])\n"
  "\n",
  "Pool", NULL, 0 },
{ "git: test_libs #127",
  "s = \"{\\\"x\\\": 42}\"\n"
  "post(s.get_json(\"x\"))\n"
  "\n",
  "42", NULL, 0 },
{ "git: test_libs #128",
  "import flask\n"
  "post(\"ok\")\n"
  "\n",
  "ok", NULL, 0 },
{ "git: test_pipeline_c #129",
  "async action f() {\n"
  " return 1\n"
  "}\n"
  "post(await f())\n"
  "post(1)\n"
  "\n",
  "1\n1", NULL, 0 },
{ "git: test_unpacking #130",
  "a, b = 1, 2\n"
  "post(a)\n"
  "post(b)\n"
  "\n",
  "1\n2", NULL, 0 },
{ "git: test_unpacking #131",
  "l = [1, 2, 3]\n"
  "a, b, c = l\n"
  "post(a)\n"
  "post(b)\n"
  "post(c)\n"
  "\n",
  "1\n2\n3", NULL, 0 },
{ "git: test_unpacking #132",
  "t = (1, 2, 3)\n"
  "a, b, c = t\n"
  "post(a)\n"
  "post(b)\n"
  "post(c)\n"
  "\n",
  "1\n2\n3", NULL, 0 },
{ "git: test_unpacking #133",
  "a = 1\n"
  "b = 2\n"
  "a, b = b, a\n"
  "post(a)\n"
  "post(b)\n"
  "\n",
  "2\n1", NULL, 0 },
{ "git: test_unpacking #134",
  "a = 1\n"
  "b = 2\n"
  "c = 3\n"
  "a, b, c = c, a, b\n"
  "post(a)\n"
  "post(b)\n"
  "post(c)\n"
  "\n",
  "3\n1\n2", NULL, 0 },
{ "git: test_unpacking #135",
  "a, (b, c) = 1, (2, 3)\n"
  "post(a)\n"
  "post(b)\n"
  "post(c)\n"
  "\n",
  "1\n2\n3", NULL, 0 },
{ "git: test_unpacking #136",
  "a, (b, c) = 1, (2, 3)\n"
  "post(a)\n"
  "\n",
  "1", NULL, 0 },
{ "git: test_unpacking #137",
  "a, (b, rest) = 1, (2, [3, 4])\n"
  "post(b)\n"
  "post(rest)\n"
  "\n",
  "2\n[3, 4]", NULL, 0 },
{ "git: test_unpacking #138",
  "a, *resto = [1, 2, 3, 4]\n"
  "post(a)\n"
  "post(resto)\n"
  "\n",
  "1\n[2, 3, 4]", NULL, 0 },
{ "git: test_unpacking #139",
  "a, *resto = (1, 2, 3, 4)\n"
  "post(type(resto))\n"
  "\n",
  "list", NULL, 0 },
{ "git: test_unpacking #140",
  "*inicio, z = [1, 2, 3, 4]\n"
  "post(inicio)\n"
  "post(z)\n"
  "\n",
  "[1, 2, 3]\n4", NULL, 0 },
{ "git: test_unpacking #141",
  "a, *meio, z = [1, 2, 3, 4, 5]\n"
  "post(a)\n"
  "post(meio)\n"
  "post(z)\n"
  "\n",
  "1\n[2, 3, 4]\n5", NULL, 0 },
{ "git: test_unpacking #142",
  "a, b, *resto = [1, 2]\n"
  "post(a)\n"
  "post(b)\n"
  "post(resto)\n"
  "\n",
  "1\n2\n[]", NULL, 0 },
{ "git: test_unpacking #143",
  "a, = [5]\n"
  "post(a)\n"
  "\n",
  "5", NULL, 0 },
{ "git: test_unpacking #144",
  "a, b = \"hi\"\n"
  "post(a)\n"
  "post(b)\n"
  "\n",
  "h\ni", NULL, 0 },
{ "git: test_unpacking #145",
  "a, b = 1, 2\n"
  "post(a)\n"
  "post(b)\n"
  "a, b = 10, 20\n"
  "post(a)\n"
  "post(b)\n"
  "\n",
  "1\n2\n10\n20", NULL, 0 },
{ "git: test_unpacking #146",
  "action f() {\n"
  "    a, b = 1, 2\n"
  "    return a + b\n"
  "}\n"
  "post(f())\n"
  "\n",
  "3", NULL, 0 },
{ "git: test_unpacking #147",
  "a = 0\n"
  "b = 0\n"
  "if (True) {\n"
  "    a, b = 7, 8\n"
  "}\n"
  "post(a)\n"
  "post(b)\n"
  "\n",
  "7\n8", NULL, 0 },
{ "git: test_unpacking #148",
  "a, b, c = 1, 2, 3\n"
  "c, a, b = a, b, c\n"
  "post(a)\n"
  "post(b)\n"
  "post(c)\n"
  "\n",
  "2\n3\n1", NULL, 0 },
{ "git: test_unpacking #149",
  "action gen() {\n"
  "    yield 1\n"
  "    yield 2\n"
  "}\n"
  "a, b = gen()\n"
  "post(a)\n"
  "post(b)\n"
  "\n",
  "1\n2", NULL, 0 },
{ "git: test_unpacking #150",
  "for each i in [1, 2, 3] {\n"
  "    post(i)\n"
  "}\n"
  "\n",
  "1\n2\n3", NULL, 0 },
{ "git: test_unpacking #151",
  "Entity Ponto() {\n"
  "    action __init__(self, x) {\n"
  "        self.x = x\n"
  "    }\n"
  "}\n"
  "p = Ponto(5)\n"
  "post(p.x)\n"
  "\n",
  "5", NULL, 0 },
{ "git: test_unpacking #152",
  "action f() {\n"
  "    return 1, 2\n"
  "}\n"
  "r = f()\n"
  "post(r)\n"
  "\n",
  "(1, 2)", NULL, 0 },
{ "git: test_v0_4_0_indent #153",
  "int x = 10\n"
  "if x > 5 {\n"
  "    post(\"big\")\n"
  "}\n"
  "\n",
  "big", NULL, 0 },
{ "git: test_v0_4_0_indent #154",
  "int i = 0\n"
  "while i < 3 {\n"
  "    post(i)\n"
  "    i = i + 1\n"
  "}\n"
  "\n",
  "0\n1\n2", NULL, 0 },
{ "git: test_v0_4_0_indent #155",
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
{ "git: test_v0_4_0_indent #156",
  "if 1 == 1 {\n"
  "    if 2 == 2 { post(\"misto\") }\n"
  "}\n"
  "\n",
  "misto", NULL, 0 },
{ "git: test_v0_4_0_indent #157",
  "if 1 == 1 {\n"
  "    if 2 == 2 { post(\"misto2\") }\n"
  "}\n"
  "\n",
  "misto2", NULL, 0 },
{ "git: test_v0_4_0_indent #158",
  "for each x in [1, 2, 3] {\n"
  "    post(x)\n"
  "}\n"
  "\n",
  "1\n2\n3", NULL, 0 },
{ "git: test_v0_4_0_indent #159",
  "try {\n"
  "    int x = 1 / 0\n"
  "} catch (e) {\n"
  "    post(\"pego\")\n"
  "}\n"
  "\n",
  "pego", NULL, 0 },
{ "git: test_v0_5_1_type_checks #160",
  "\n"
  "x = \"123\"\n"
  "int y = 123\n"
  "if (x not is int and y is int and None is None) {\n"
  "    post(\"negative-path-ok\")\n"
  "}\n"
  "\n",
  "negative-path-ok", NULL, 0 },
{ "git: test_v0_5_2_count #161",
  "\n"
  "list nums = [1, 7, 7, 2, 7]\n"
  "post(count int(7) in nums)\n"
  "\n",
  "3", NULL, 0 },
{ "git: test_v0_5_2_count #162",
  "\n"
  "list nums = [1, 7, 7, 2]\n"
  "post(int(7) count in nums)\n"
  "\n",
  "2", NULL, 0 },
{ "git: test_v0_5_2_count #163",
  "\n"
  "list mix = [1, \"a\", 2, \"b\", 3]\n"
  "post(count int in mix)\n"
  "post(count str in mix)\n"
  "\n",
  "3\n2", NULL, 0 },
{ "git: test_v0_5_2_count #164",
  "\n"
  "str s = \"ana banana\"\n"
  "post(count str(\"ana\") in s)\n"
  "\n",
  "3", NULL, 0 },
{ "git: test_v0_5_2_count #165",
  "\n"
  "int n = 17717\n"
  "post(count int(7) in n)\n"
  "\n",
  "3", NULL, 0 },
{ "git: test_v0_5_2_count #166",
  "\n"
  "json d = {\"a\": 1, \"b\": \"x\", \"c\": 1, \"d\": 2}\n"
  "post(count int(1) in d)\n"
  "post(count str in d)\n"
  "\n",
  "2\n1", NULL, 0 },
{ "git: test_v0_5_2_count #167",
  "\n"
  "list xs = [1, 2, 3]\n"
  "if (count int(99) in xs) {\n"
  "    post(\"found\")\n"
  "} else {\n"
  "    post(\"none\")\n"
  "}\n"
  "\n",
  "none", NULL, 0 },
{ "git: test_v0_5_2_count #168",
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
{ "git: test_v0_5_2_count #169",
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
{ "git: test_v0_5_2_count #170",
  "\n"
  "list xs = [7, 8, 7]\n"
  "count each int(7) in xs {\n"
  "    post(_match)\n"
  "}\n"
  "\n",
  "7\n7", NULL, 0 },
{ "git: test_v0_5_2_count #171",
  "\n"
  "list a = [1, 1]\n"
  "list b = [3]\n"
  "if (count int(1) in a and count int(3) in b) {\n"
  "    post(\"both\")\n"
  "}\n"
  "\n",
  "both", NULL, 0 },
{ "git: test_v0_5_2_count #172",
  "\n"
  "list xs = [9, 9, 9, 1]\n"
  "if (count int(9) in xs == 3) { post(\"ok\") }\n"
  "\n",
  "ok", NULL, 0 },
{ "git: test_v5_features #173",
  "reaction f() { return 1 }\n"
  "post(f())\n"
  "\n",
  "1", NULL, 0 },
{ "git: test_v5_features #174",
  "reaction soma(a,b) { return a+b }\n"
  "post(soma(3,4))\n"
  "\n",
  "7", NULL, 0 },
{ "git: test_v5_features #175",
  "reaction f() {\n"
  "    return 42\n"
  "}\n"
  "post(f())\n"
  "\n",
  "42", NULL, 0 },
{ "git: test_v5_features #176",
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
{ "git: test_v5_features #177",
  "action f()\n"
  "{\n"
  "    return 99\n"
  "}\n"
  "post(f())\n"
  "\n",
  "99", NULL, 0 },
{ "git: test_v5_features #178",
  "reaction f()\n"
  "{\n"
  "    return 77\n"
  "}\n"
  "post(f())\n"
  "\n",
  "77", NULL, 0 },
{ "git: test_v5_features #179",
  "x=5\n"
  "if (x>0) {\n"
  "    post(\"ok\")\n"
  "}\n"
  "\n",
  "ok", NULL, 0 },
{ "git: test_v5_features #180",
  "int reaction f() { return 42 }\n"
  "post(f())\n"
  "\n",
  "42", NULL, 0 },
{ "git: test_v5_features #181",
  "int reaction f() { x=1 }\n"
  "post(f())\n"
  "\n",
  "0", NULL, 0 },
{ "git: test_v5_features #182",
  "int reaction f() { return 1/0 }\n"
  "post(f())\n"
  "\n",
  "500", NULL, 0 },
{ "git: test_v5_features #183",
  "bool reaction f() { return true }\n"
  "post(f())\n"
  "\n",
  "True", NULL, 0 },
{ "git: test_v5_features #184",
  "bool reaction f() { return false }\n"
  "post(f())\n"
  "\n",
  "False", NULL, 0 },
{ "git: test_v5_features #185",
  "bool reaction f() { return 1/0 }\n"
  "post(f())\n"
  "\n",
  "False", NULL, 0 },
{ "git: test_v5_features #186",
  "async int reaction f() { return 21 }\n"
  "post(await f())\n"
  "\n",
  "21", NULL, 0 },
{ "git: test_v5_features #187",
  "async action f() { return 55 }\n"
  "post(await f())\n"
  "\n",
  "55", NULL, 0 },
{ "git: test_v5_features #188",
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
{ "git: test_v5_features #189",
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
{ "git: test_v5_features #190",
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
{ "git: test_v5_features #191",
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
{ "git: test_v5_features #192",
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
{ "git: test_v5_features #193",
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
{ "git: test_v5_features #194",
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
{ "git: test_v5_features #195",
  "l=[1,2]\n"
  "addEnd(l,3)\n"
  "post(l)\n"
  "\n",
  "[1, 2, 3]", NULL, 0 },
{ "git: test_v5_features #196",
  "l=[1,2,3]\n"
  "v=removeEnd(l)\n"
  "post(v)\n"
  "post(l)\n"
  "\n",
  "3\n[1, 2]", NULL, 0 },
{ "git: test_v5_features #197",
  "l=[2,3]\n"
  "addStart(l,1)\n"
  "post(l)\n"
  "\n",
  "[1, 2, 3]", NULL, 0 },
{ "git: test_v5_features #198",
  "l=[1,2,3]\n"
  "v=removeStart(l)\n"
  "post(v)\n"
  "post(l)\n"
  "\n",
  "1\n[2, 3]", NULL, 0 },
{ "git: test_v5_features #199",
  "l=[]\n"
  "post(removeEnd(l))\n"
  "\n",
  "Null", NULL, 0 },
{ "git: test_v5_features #200",
  "l=[]\n"
  "post(removeStart(l))\n"
  "\n",
  "Null", NULL, 0 },
{ "git: test_v5_features #201",
  "post(hex(255))\n"
  "\n",
  "0xff", NULL, 0 },
{ "git: test_v5_features #202",
  "post(bin(10))\n"
  "\n",
  "0b1010", NULL, 0 },
{ "git: test_v5_features #203",
  "post(oct(8))\n"
  "\n",
  "0o10", NULL, 0 },
{ "git: test_v5_features #204",
  "post(ord(\"A\"))\n"
  "\n",
  "65", NULL, 0 },
{ "git: test_v5_features #205",
  "post(chr(65))\n"
  "\n",
  "A", NULL, 0 },
{ "git: test_v5_features #206",
  "post(abs(-42))\n"
  "\n",
  "42", NULL, 0 },
{ "git: test_v5_features #207",
  "post(round(3.7))\n"
  "\n",
  "4", NULL, 0 },
{ "git: test_v5_features #208",
  "post(sum([1,2,3,4,5]))\n"
  "\n",
  "15", NULL, 0 },
{ "git: test_v5_features #209",
  "post(min([5,1,3]))\n"
  "\n",
  "1", NULL, 0 },
{ "git: test_v5_features #210",
  "post(max([5,1,3]))\n"
  "\n",
  "5", NULL, 0 },
{ "git: test_v5_features #211",
  "post(sorted([3,1,2]))\n"
  "\n",
  "[1, 2, 3]", NULL, 0 },
{ "git: test_v5_features #212",
  "post(reversed([1,2,3]))\n"
  "\n",
  "[3, 2, 1]", NULL, 0 },
{ "git: test_v5_features #213",
  "x=150\n"
  "post(x.isdigit())\n"
  "\n",
  "True", NULL, 0 },
{ "git: test_v5_features #214",
  "x=150.5\n"
  "post(x.isdigit())\n"
  "\n",
  "False", NULL, 0 },
{ "git: test_v5_features #215",
  "post(\"150\".isdigit())\n"
  "\n",
  "True", NULL, 0 },
{ "git: test_v5_features #216",
  "x=123\n"
  "post(x.isalpha())\n"
  "\n",
  "False", NULL, 0 },
{ "git: test_v5_features #217",
  "post(count int(2) in [1,2,2,3])\n"
  "\n",
  "2", NULL, 0 },
{ "git: test_v5_features #218",
  "post(count str(\"a\") in \"banana\")\n"
  "\n",
  "3", NULL, 0 },
{ "git: test_v5_features #219",
  "post(count int(1) in 112211)\n"
  "\n",
  "4", NULL, 0 },
{ "git: test_v5_features #220",
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
{ "git: test_v5_features #221",
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
{ "git: test_v5_features #222",
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
{ "git: test_v5_features #223",
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
{ "git: test_v5_features #224",
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
{ "git: test_v5_features #225",
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
{ "git: test_v5_features #226",
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
{ "git: test_v5_features #227",
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
{ "git: test_v5_features #228",
  "try {\n"
  "    x=1/0\n"
  "} catch (e) {\n"
  "    post(\"err\")\n"
  "}\n"
  "\n",
  "err", NULL, 0 },
{ "git: test_v5_features #229",
  "try {\n"
  "    raise \"ops\"\n"
  "} catch (e) {\n"
  "    post(\"catch\")\n"
  "} finally {\n"
  "    post(\"finally\")\n"
  "}\n"
  "\n",
  "catch\nfinally", NULL, 0 },
{ "git: test_vm_c #230",
  "action r(n) {\n"
  " if (n < 1) {\n"
  "  return 0\n"
  " }\n"
  " return 1 + r(n - 1)\n"
  "}\n"
  "post(r(10000))\n"
  "\n",
  "10000", NULL, 0 },
{ "git: test_vm_c #231",
  "post([1, 2][9])\n"
  "\n",
  NULL, "list index out of range", -1 },
{ "git: test_vm_c #232",
  "post(\"ab\"[9])\n"
  "\n",
  NULL, "string index out of range", -1 },
{ "git: test_vm_c #233",
  "post([Null])\n"
  "\n",
  "[Null]", NULL, 0 },
{ "git: test_vm_c #234",
  "post(str(Null))\n"
  "\n",
  "Null", NULL, 0 },
{ "matriz: char em topo",
  "char c = 64\n"
  "post(c)\n"
  "\n",
  "@", NULL, 0 },
{ "matriz: char_texto em topo",
  "char c = \"ç\"\n"
  "post(c, len(c))\n"
  "\n",
  "ç 1", NULL, 0 },
{ "matriz: closure em topo",
  "a = 1\n"
  "action inc() {\n"
  "    a = a + 1\n"
  "    return a\n"
  "}\n"
  "post(inc(), inc())\n"
  "\n",
  "2 3", NULL, 0 },
{ "matriz: closure_self em topo",
  "x = 5\n"
  "action le() {\n"
  "    return x\n"
  "}\n"
  "post(le())\n"
  "\n",
  "5", NULL, 0 },
{ "matriz: startswith em topo",
  "post(\"abc\".startswith((\"z\", \"a\")))\n"
  "\n",
  "True", NULL, 0 },
{ "matriz: index_faixa em topo",
  "l = [1, 2, 3, 2]\n"
  "post(l.index(2, 2))\n"
  "\n",
  "3", NULL, 0 },
{ "matriz: encode em topo",
  "post(\"café\".encode(\"latin-1\"))\n"
  "\n",
  "b'caf\\xe9'", NULL, 0 },
{ "matriz: decode em topo",
  "post(\"café\".encode(\"latin-1\").decode(\"latin-1\"))\n"
  "\n",
  "café", NULL, 0 },
{ "matriz: num_base em topo",
  "post(0x1F, 0b101, 1_000, 1e3)\n"
  "\n",
  "31 5 1000 1000.0", NULL, 0 },
{ "matriz: unicode em topo",
  "post(\"Ω\".lower(), \"ß\".upper())\n"
  "\n",
  "ω SS", NULL, 0 },
{ "matriz: isdigit em topo",
  "post(\"²\".isdigit(), \"²\".isdecimal())\n"
  "\n",
  "True False", NULL, 0 },
{ "matriz: regex_split em topo",
  "import regex\n"
  "post(regex.split(r\"(,)\", \"a,b\"))\n"
  "\n",
  "['a', ',', 'b']", NULL, 0 },
{ "matriz: regex_sub em topo",
  "import regex\n"
  "post(regex.sub(r\"(\\d)\", r\"[\\g<1>]\", \"a1\"))\n"
  "\n",
  "a[1]", NULL, 0 },
{ "matriz: json_emoji em topo",
  "import json\n"
  "post(json.parse(\"{\\\"a\\\": \\\"\\\\ud83d\\\\ude00\\\"}\"))\n"
  "\n",
  "{'a': '😀'}", NULL, 0 },
{ "matriz: await_lista em topo",
  "async action d(n) {\n"
  "    return n * 2\n"
  "}\n"
  "post(await [d(1), d(2)])\n"
  "\n",
  "[2, 4]", NULL, 0 },
{ "matriz: int_inf em topo",
  "try {\n"
  "    post(int(flo(\"inf\")))\n"
  "} catch(e) {\n"
  "    post(\"ok-erro\")\n"
  "}\n"
  "\n",
  "ok-erro", NULL, 0 },
{ "matriz: nul em topo",
  "post(len(\"a\\x00b\"))\n"
  "\n",
  "3", NULL, 0 },
{ "matriz: fatia em topo",
  "post(\"abcdef\"[999999999999999999999:])\n"
  "\n",
  "", NULL, 0 },
{ "matriz: char em action",
  "action f() {\n"
  "    char c = 64\n"
  "    post(c)\n"
  "}\n"
  "f()\n"
  "\n",
  "@", NULL, 0 },
{ "matriz: char_texto em action",
  "action f() {\n"
  "    char c = \"ç\"\n"
  "    post(c, len(c))\n"
  "}\n"
  "f()\n"
  "\n",
  "ç 1", NULL, 0 },
{ "matriz: closure em action",
  "action f() {\n"
  "    a = 1\n"
  "    action inc() {\n"
  "        a = a + 1\n"
  "        return a\n"
  "    }\n"
  "    post(inc(), inc())\n"
  "}\n"
  "f()\n"
  "\n",
  "2 3", NULL, 0 },
{ "matriz: closure_self em action",
  "action f() {\n"
  "    x = 5\n"
  "    action le() {\n"
  "        return x\n"
  "    }\n"
  "    post(le())\n"
  "}\n"
  "f()\n"
  "\n",
  "5", NULL, 0 },
{ "matriz: startswith em action",
  "action f() {\n"
  "    post(\"abc\".startswith((\"z\", \"a\")))\n"
  "}\n"
  "f()\n"
  "\n",
  "True", NULL, 0 },
{ "matriz: index_faixa em action",
  "action f() {\n"
  "    l = [1, 2, 3, 2]\n"
  "    post(l.index(2, 2))\n"
  "}\n"
  "f()\n"
  "\n",
  "3", NULL, 0 },
{ "matriz: encode em action",
  "action f() {\n"
  "    post(\"café\".encode(\"latin-1\"))\n"
  "}\n"
  "f()\n"
  "\n",
  "b'caf\\xe9'", NULL, 0 },
{ "matriz: decode em action",
  "action f() {\n"
  "    post(\"café\".encode(\"latin-1\").decode(\"latin-1\"))\n"
  "}\n"
  "f()\n"
  "\n",
  "café", NULL, 0 },
{ "matriz: num_base em action",
  "action f() {\n"
  "    post(0x1F, 0b101, 1_000, 1e3)\n"
  "}\n"
  "f()\n"
  "\n",
  "31 5 1000 1000.0", NULL, 0 },
{ "matriz: unicode em action",
  "action f() {\n"
  "    post(\"Ω\".lower(), \"ß\".upper())\n"
  "}\n"
  "f()\n"
  "\n",
  "ω SS", NULL, 0 },
{ "matriz: isdigit em action",
  "action f() {\n"
  "    post(\"²\".isdigit(), \"²\".isdecimal())\n"
  "}\n"
  "f()\n"
  "\n",
  "True False", NULL, 0 },
{ "matriz: regex_split em action",
  "action f() {\n"
  "    import regex\n"
  "    post(regex.split(r\"(,)\", \"a,b\"))\n"
  "}\n"
  "f()\n"
  "\n",
  "['a', ',', 'b']", NULL, 0 },
{ "matriz: regex_sub em action",
  "action f() {\n"
  "    import regex\n"
  "    post(regex.sub(r\"(\\d)\", r\"[\\g<1>]\", \"a1\"))\n"
  "}\n"
  "f()\n"
  "\n",
  "a[1]", NULL, 0 },
{ "matriz: json_emoji em action",
  "action f() {\n"
  "    import json\n"
  "    post(json.parse(\"{\\\"a\\\": \\\"\\\\ud83d\\\\ude00\\\"}\"))\n"
  "}\n"
  "f()\n"
  "\n",
  "{'a': '😀'}", NULL, 0 },
{ "matriz: await_lista em action",
  "action f() {\n"
  "    async action d(n) {\n"
  "        return n * 2\n"
  "    }\n"
  "    post(await [d(1), d(2)])\n"
  "}\n"
  "f()\n"
  "\n",
  "[2, 4]", NULL, 0 },
{ "matriz: int_inf em action",
  "action f() {\n"
  "    try {\n"
  "        post(int(flo(\"inf\")))\n"
  "    } catch(e) {\n"
  "        post(\"ok-erro\")\n"
  "    }\n"
  "}\n"
  "f()\n"
  "\n",
  "ok-erro", NULL, 0 },
{ "matriz: nul em action",
  "action f() {\n"
  "    post(len(\"a\\x00b\"))\n"
  "}\n"
  "f()\n"
  "\n",
  "3", NULL, 0 },
{ "matriz: fatia em action",
  "action f() {\n"
  "    post(\"abcdef\"[999999999999999999999:])\n"
  "}\n"
  "f()\n"
  "\n",
  "", NULL, 0 },
{ "matriz: char em bloco_if",
  "if true {\n"
  "    char c = 64\n"
  "    post(c)\n"
  "}\n"
  "\n",
  "@", NULL, 0 },
{ "matriz: char_texto em bloco_if",
  "if true {\n"
  "    char c = \"ç\"\n"
  "    post(c, len(c))\n"
  "}\n"
  "\n",
  "ç 1", NULL, 0 },
{ "matriz: closure em bloco_if",
  "if true {\n"
  "    a = 1\n"
  "    action inc() {\n"
  "        a = a + 1\n"
  "        return a\n"
  "    }\n"
  "    post(inc(), inc())\n"
  "}\n"
  "\n",
  "2 3", NULL, 0 },
{ "matriz: closure_self em bloco_if",
  "if true {\n"
  "    x = 5\n"
  "    action le() {\n"
  "        return x\n"
  "    }\n"
  "    post(le())\n"
  "}\n"
  "\n",
  "5", NULL, 0 },
{ "matriz: startswith em bloco_if",
  "if true {\n"
  "    post(\"abc\".startswith((\"z\", \"a\")))\n"
  "}\n"
  "\n",
  "True", NULL, 0 },
{ "matriz: index_faixa em bloco_if",
  "if true {\n"
  "    l = [1, 2, 3, 2]\n"
  "    post(l.index(2, 2))\n"
  "}\n"
  "\n",
  "3", NULL, 0 },
{ "matriz: encode em bloco_if",
  "if true {\n"
  "    post(\"café\".encode(\"latin-1\"))\n"
  "}\n"
  "\n",
  "b'caf\\xe9'", NULL, 0 },
{ "matriz: decode em bloco_if",
  "if true {\n"
  "    post(\"café\".encode(\"latin-1\").decode(\"latin-1\"))\n"
  "}\n"
  "\n",
  "café", NULL, 0 },
{ "matriz: num_base em bloco_if",
  "if true {\n"
  "    post(0x1F, 0b101, 1_000, 1e3)\n"
  "}\n"
  "\n",
  "31 5 1000 1000.0", NULL, 0 },
{ "matriz: unicode em bloco_if",
  "if true {\n"
  "    post(\"Ω\".lower(), \"ß\".upper())\n"
  "}\n"
  "\n",
  "ω SS", NULL, 0 },
{ "matriz: isdigit em bloco_if",
  "if true {\n"
  "    post(\"²\".isdigit(), \"²\".isdecimal())\n"
  "}\n"
  "\n",
  "True False", NULL, 0 },
{ "matriz: regex_split em bloco_if",
  "if true {\n"
  "    import regex\n"
  "    post(regex.split(r\"(,)\", \"a,b\"))\n"
  "}\n"
  "\n",
  "['a', ',', 'b']", NULL, 0 },
{ "matriz: regex_sub em bloco_if",
  "if true {\n"
  "    import regex\n"
  "    post(regex.sub(r\"(\\d)\", r\"[\\g<1>]\", \"a1\"))\n"
  "}\n"
  "\n",
  "a[1]", NULL, 0 },
{ "matriz: json_emoji em bloco_if",
  "if true {\n"
  "    import json\n"
  "    post(json.parse(\"{\\\"a\\\": \\\"\\\\ud83d\\\\ude00\\\"}\"))\n"
  "}\n"
  "\n",
  "{'a': '😀'}", NULL, 0 },
{ "matriz: await_lista em bloco_if",
  "if true {\n"
  "    async action d(n) {\n"
  "        return n * 2\n"
  "    }\n"
  "    post(await [d(1), d(2)])\n"
  "}\n"
  "\n",
  "[2, 4]", NULL, 0 },
{ "matriz: int_inf em bloco_if",
  "if true {\n"
  "    try {\n"
  "        post(int(flo(\"inf\")))\n"
  "    } catch(e) {\n"
  "        post(\"ok-erro\")\n"
  "    }\n"
  "}\n"
  "\n",
  "ok-erro", NULL, 0 },
{ "matriz: nul em bloco_if",
  "if true {\n"
  "    post(len(\"a\\x00b\"))\n"
  "}\n"
  "\n",
  "3", NULL, 0 },
{ "matriz: fatia em bloco_if",
  "if true {\n"
  "    post(\"abcdef\"[999999999999999999999:])\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "matriz: char em bloco_chaves",
  "if (true) {\n"
  "    char c = 64\n"
  "    post(c)\n"
  "}\n"
  "\n",
  "@", NULL, 0 },
{ "matriz: char_texto em bloco_chaves",
  "if (true) {\n"
  "    char c = \"ç\"\n"
  "    post(c, len(c))\n"
  "}\n"
  "\n",
  "ç 1", NULL, 0 },
{ "matriz: startswith em bloco_chaves",
  "if (true) {\n"
  "    post(\"abc\".startswith((\"z\", \"a\")))\n"
  "}\n"
  "\n",
  "True", NULL, 0 },
{ "matriz: index_faixa em bloco_chaves",
  "if (true) {\n"
  "    l = [1, 2, 3, 2]\n"
  "    post(l.index(2, 2))\n"
  "}\n"
  "\n",
  "3", NULL, 0 },
{ "matriz: encode em bloco_chaves",
  "if (true) {\n"
  "    post(\"café\".encode(\"latin-1\"))\n"
  "}\n"
  "\n",
  "b'caf\\xe9'", NULL, 0 },
{ "matriz: decode em bloco_chaves",
  "if (true) {\n"
  "    post(\"café\".encode(\"latin-1\").decode(\"latin-1\"))\n"
  "}\n"
  "\n",
  "café", NULL, 0 },
{ "matriz: num_base em bloco_chaves",
  "if (true) {\n"
  "    post(0x1F, 0b101, 1_000, 1e3)\n"
  "}\n"
  "\n",
  "31 5 1000 1000.0", NULL, 0 },
{ "matriz: unicode em bloco_chaves",
  "if (true) {\n"
  "    post(\"Ω\".lower(), \"ß\".upper())\n"
  "}\n"
  "\n",
  "ω SS", NULL, 0 },
{ "matriz: isdigit em bloco_chaves",
  "if (true) {\n"
  "    post(\"²\".isdigit(), \"²\".isdecimal())\n"
  "}\n"
  "\n",
  "True False", NULL, 0 },
{ "matriz: regex_split em bloco_chaves",
  "if (true) {\n"
  "    import regex\n"
  "    post(regex.split(r\"(,)\", \"a,b\"))\n"
  "}\n"
  "\n",
  "['a', ',', 'b']", NULL, 0 },
{ "matriz: regex_sub em bloco_chaves",
  "if (true) {\n"
  "    import regex\n"
  "    post(regex.sub(r\"(\\d)\", r\"[\\g<1>]\", \"a1\"))\n"
  "}\n"
  "\n",
  "a[1]", NULL, 0 },
{ "matriz: json_emoji em bloco_chaves",
  "if (true) {\n"
  "    import json\n"
  "    post(json.parse(\"{\\\"a\\\": \\\"\\\\ud83d\\\\ude00\\\"}\"))\n"
  "}\n"
  "\n",
  "{'a': '😀'}", NULL, 0 },
{ "matriz: nul em bloco_chaves",
  "if (true) {\n"
  "    post(len(\"a\\x00b\"))\n"
  "}\n"
  "\n",
  "3", NULL, 0 },
{ "matriz: fatia em bloco_chaves",
  "if (true) {\n"
  "    post(\"abcdef\"[999999999999999999999:])\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "matriz: char em for_each",
  "for each _i in range(2) {\n"
  "    char c = 64\n"
  "    post(c)\n"
  "}\n"
  "\n",
  "@\n@", NULL, 0 },
{ "matriz: char_texto em for_each",
  "for each _i in range(2) {\n"
  "    char c = \"ç\"\n"
  "    post(c, len(c))\n"
  "}\n"
  "\n",
  "ç 1\nç 1", NULL, 0 },
{ "matriz: closure em for_each",
  "for each _i in range(2) {\n"
  "    a = 1\n"
  "    action inc() {\n"
  "        a = a + 1\n"
  "        return a\n"
  "    }\n"
  "    post(inc(), inc())\n"
  "}\n"
  "\n",
  "2 3\n2 3", NULL, 0 },
{ "matriz: closure_self em for_each",
  "for each _i in range(2) {\n"
  "    x = 5\n"
  "    action le() {\n"
  "        return x\n"
  "    }\n"
  "    post(le())\n"
  "}\n"
  "\n",
  "5\n5", NULL, 0 },
{ "matriz: startswith em for_each",
  "for each _i in range(2) {\n"
  "    post(\"abc\".startswith((\"z\", \"a\")))\n"
  "}\n"
  "\n",
  "True\nTrue", NULL, 0 },
{ "matriz: index_faixa em for_each",
  "for each _i in range(2) {\n"
  "    l = [1, 2, 3, 2]\n"
  "    post(l.index(2, 2))\n"
  "}\n"
  "\n",
  "3\n3", NULL, 0 },
{ "matriz: encode em for_each",
  "for each _i in range(2) {\n"
  "    post(\"café\".encode(\"latin-1\"))\n"
  "}\n"
  "\n",
  "b'caf\\xe9'\nb'caf\\xe9'", NULL, 0 },
{ "matriz: decode em for_each",
  "for each _i in range(2) {\n"
  "    post(\"café\".encode(\"latin-1\").decode(\"latin-1\"))\n"
  "}\n"
  "\n",
  "café\ncafé", NULL, 0 },
{ "matriz: num_base em for_each",
  "for each _i in range(2) {\n"
  "    post(0x1F, 0b101, 1_000, 1e3)\n"
  "}\n"
  "\n",
  "31 5 1000 1000.0\n31 5 1000 1000.0", NULL, 0 },
{ "matriz: unicode em for_each",
  "for each _i in range(2) {\n"
  "    post(\"Ω\".lower(), \"ß\".upper())\n"
  "}\n"
  "\n",
  "ω SS\nω SS", NULL, 0 },
{ "matriz: isdigit em for_each",
  "for each _i in range(2) {\n"
  "    post(\"²\".isdigit(), \"²\".isdecimal())\n"
  "}\n"
  "\n",
  "True False\nTrue False", NULL, 0 },
{ "matriz: regex_split em for_each",
  "for each _i in range(2) {\n"
  "    import regex\n"
  "    post(regex.split(r\"(,)\", \"a,b\"))\n"
  "}\n"
  "\n",
  "['a', ',', 'b']\n['a', ',', 'b']", NULL, 0 },
{ "matriz: regex_sub em for_each",
  "for each _i in range(2) {\n"
  "    import regex\n"
  "    post(regex.sub(r\"(\\d)\", r\"[\\g<1>]\", \"a1\"))\n"
  "}\n"
  "\n",
  "a[1]\na[1]", NULL, 0 },
{ "matriz: json_emoji em for_each",
  "for each _i in range(2) {\n"
  "    import json\n"
  "    post(json.parse(\"{\\\"a\\\": \\\"\\\\ud83d\\\\ude00\\\"}\"))\n"
  "}\n"
  "\n",
  "{'a': '😀'}\n{'a': '😀'}", NULL, 0 },
{ "matriz: await_lista em for_each",
  "for each _i in range(2) {\n"
  "    async action d(n) {\n"
  "        return n * 2\n"
  "    }\n"
  "    post(await [d(1), d(2)])\n"
  "}\n"
  "\n",
  "[2, 4]\n[2, 4]", NULL, 0 },
{ "matriz: int_inf em for_each",
  "for each _i in range(2) {\n"
  "    try {\n"
  "        post(int(flo(\"inf\")))\n"
  "    } catch(e) {\n"
  "        post(\"ok-erro\")\n"
  "    }\n"
  "}\n"
  "\n",
  "ok-erro\nok-erro", NULL, 0 },
{ "matriz: nul em for_each",
  "for each _i in range(2) {\n"
  "    post(len(\"a\\x00b\"))\n"
  "}\n"
  "\n",
  "3\n3", NULL, 0 },
{ "matriz: fatia em for_each",
  "for each _i in range(2) {\n"
  "    post(\"abcdef\"[999999999999999999999:])\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "matriz: char em while",
  "n = 0\n"
  "while n < 1 {\n"
  "    n = n + 1\n"
  "    char c = 64\n"
  "    post(c)\n"
  "}\n"
  "\n",
  "@", NULL, 0 },
{ "matriz: char_texto em while",
  "n = 0\n"
  "while n < 1 {\n"
  "    n = n + 1\n"
  "    char c = \"ç\"\n"
  "    post(c, len(c))\n"
  "}\n"
  "\n",
  "ç 1", NULL, 0 },
{ "matriz: closure em while",
  "n = 0\n"
  "while n < 1 {\n"
  "    n = n + 1\n"
  "    a = 1\n"
  "    action inc() {\n"
  "        a = a + 1\n"
  "        return a\n"
  "    }\n"
  "    post(inc(), inc())\n"
  "}\n"
  "\n",
  "2 3", NULL, 0 },
{ "matriz: closure_self em while",
  "n = 0\n"
  "while n < 1 {\n"
  "    n = n + 1\n"
  "    x = 5\n"
  "    action le() {\n"
  "        return x\n"
  "    }\n"
  "    post(le())\n"
  "}\n"
  "\n",
  "5", NULL, 0 },
{ "matriz: startswith em while",
  "n = 0\n"
  "while n < 1 {\n"
  "    n = n + 1\n"
  "    post(\"abc\".startswith((\"z\", \"a\")))\n"
  "}\n"
  "\n",
  "True", NULL, 0 },
{ "matriz: index_faixa em while",
  "n = 0\n"
  "while n < 1 {\n"
  "    n = n + 1\n"
  "    l = [1, 2, 3, 2]\n"
  "    post(l.index(2, 2))\n"
  "}\n"
  "\n",
  "3", NULL, 0 },
{ "matriz: encode em while",
  "n = 0\n"
  "while n < 1 {\n"
  "    n = n + 1\n"
  "    post(\"café\".encode(\"latin-1\"))\n"
  "}\n"
  "\n",
  "b'caf\\xe9'", NULL, 0 },
{ "matriz: decode em while",
  "n = 0\n"
  "while n < 1 {\n"
  "    n = n + 1\n"
  "    post(\"café\".encode(\"latin-1\").decode(\"latin-1\"))\n"
  "}\n"
  "\n",
  "café", NULL, 0 },
{ "matriz: num_base em while",
  "n = 0\n"
  "while n < 1 {\n"
  "    n = n + 1\n"
  "    post(0x1F, 0b101, 1_000, 1e3)\n"
  "}\n"
  "\n",
  "31 5 1000 1000.0", NULL, 0 },
{ "matriz: unicode em while",
  "n = 0\n"
  "while n < 1 {\n"
  "    n = n + 1\n"
  "    post(\"Ω\".lower(), \"ß\".upper())\n"
  "}\n"
  "\n",
  "ω SS", NULL, 0 },
{ "matriz: isdigit em while",
  "n = 0\n"
  "while n < 1 {\n"
  "    n = n + 1\n"
  "    post(\"²\".isdigit(), \"²\".isdecimal())\n"
  "}\n"
  "\n",
  "True False", NULL, 0 },
{ "matriz: regex_split em while",
  "n = 0\n"
  "while n < 1 {\n"
  "    n = n + 1\n"
  "    import regex\n"
  "    post(regex.split(r\"(,)\", \"a,b\"))\n"
  "}\n"
  "\n",
  "['a', ',', 'b']", NULL, 0 },
{ "matriz: regex_sub em while",
  "n = 0\n"
  "while n < 1 {\n"
  "    n = n + 1\n"
  "    import regex\n"
  "    post(regex.sub(r\"(\\d)\", r\"[\\g<1>]\", \"a1\"))\n"
  "}\n"
  "\n",
  "a[1]", NULL, 0 },
{ "matriz: json_emoji em while",
  "n = 0\n"
  "while n < 1 {\n"
  "    n = n + 1\n"
  "    import json\n"
  "    post(json.parse(\"{\\\"a\\\": \\\"\\\\ud83d\\\\ude00\\\"}\"))\n"
  "}\n"
  "\n",
  "{'a': '😀'}", NULL, 0 },
{ "matriz: await_lista em while",
  "n = 0\n"
  "while n < 1 {\n"
  "    n = n + 1\n"
  "    async action d(n) {\n"
  "        return n * 2\n"
  "    }\n"
  "    post(await [d(1), d(2)])\n"
  "}\n"
  "\n",
  "[2, 4]", NULL, 0 },
{ "matriz: int_inf em while",
  "n = 0\n"
  "while n < 1 {\n"
  "    n = n + 1\n"
  "    try {\n"
  "        post(int(flo(\"inf\")))\n"
  "    } catch(e) {\n"
  "        post(\"ok-erro\")\n"
  "    }\n"
  "}\n"
  "\n",
  "ok-erro", NULL, 0 },
{ "matriz: nul em while",
  "n = 0\n"
  "while n < 1 {\n"
  "    n = n + 1\n"
  "    post(len(\"a\\x00b\"))\n"
  "}\n"
  "\n",
  "3", NULL, 0 },
{ "matriz: fatia em while",
  "n = 0\n"
  "while n < 1 {\n"
  "    n = n + 1\n"
  "    post(\"abcdef\"[999999999999999999999:])\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "matriz: char em try",
  "try {\n"
  "    char c = 64\n"
  "    post(c)\n"
  "} catch(e) {\n"
  "    post(\"CATCH:\", e)\n"
  "}\n"
  "\n",
  "@", NULL, 0 },
{ "matriz: char_texto em try",
  "try {\n"
  "    char c = \"ç\"\n"
  "    post(c, len(c))\n"
  "} catch(e) {\n"
  "    post(\"CATCH:\", e)\n"
  "}\n"
  "\n",
  "ç 1", NULL, 0 },
{ "matriz: closure em try",
  "try {\n"
  "    a = 1\n"
  "    action inc() {\n"
  "        a = a + 1\n"
  "        return a\n"
  "    }\n"
  "    post(inc(), inc())\n"
  "} catch(e) {\n"
  "    post(\"CATCH:\", e)\n"
  "}\n"
  "\n",
  "2 3", NULL, 0 },
{ "matriz: closure_self em try",
  "try {\n"
  "    x = 5\n"
  "    action le() {\n"
  "        return x\n"
  "    }\n"
  "    post(le())\n"
  "} catch(e) {\n"
  "    post(\"CATCH:\", e)\n"
  "}\n"
  "\n",
  "5", NULL, 0 },
{ "matriz: startswith em try",
  "try {\n"
  "    post(\"abc\".startswith((\"z\", \"a\")))\n"
  "} catch(e) {\n"
  "    post(\"CATCH:\", e)\n"
  "}\n"
  "\n",
  "True", NULL, 0 },
{ "matriz: index_faixa em try",
  "try {\n"
  "    l = [1, 2, 3, 2]\n"
  "    post(l.index(2, 2))\n"
  "} catch(e) {\n"
  "    post(\"CATCH:\", e)\n"
  "}\n"
  "\n",
  "3", NULL, 0 },
{ "matriz: encode em try",
  "try {\n"
  "    post(\"café\".encode(\"latin-1\"))\n"
  "} catch(e) {\n"
  "    post(\"CATCH:\", e)\n"
  "}\n"
  "\n",
  "b'caf\\xe9'", NULL, 0 },
{ "matriz: decode em try",
  "try {\n"
  "    post(\"café\".encode(\"latin-1\").decode(\"latin-1\"))\n"
  "} catch(e) {\n"
  "    post(\"CATCH:\", e)\n"
  "}\n"
  "\n",
  "café", NULL, 0 },
{ "matriz: num_base em try",
  "try {\n"
  "    post(0x1F, 0b101, 1_000, 1e3)\n"
  "} catch(e) {\n"
  "    post(\"CATCH:\", e)\n"
  "}\n"
  "\n",
  "31 5 1000 1000.0", NULL, 0 },
{ "matriz: unicode em try",
  "try {\n"
  "    post(\"Ω\".lower(), \"ß\".upper())\n"
  "} catch(e) {\n"
  "    post(\"CATCH:\", e)\n"
  "}\n"
  "\n",
  "ω SS", NULL, 0 },
{ "matriz: isdigit em try",
  "try {\n"
  "    post(\"²\".isdigit(), \"²\".isdecimal())\n"
  "} catch(e) {\n"
  "    post(\"CATCH:\", e)\n"
  "}\n"
  "\n",
  "True False", NULL, 0 },
{ "matriz: regex_split em try",
  "try {\n"
  "    import regex\n"
  "    post(regex.split(r\"(,)\", \"a,b\"))\n"
  "} catch(e) {\n"
  "    post(\"CATCH:\", e)\n"
  "}\n"
  "\n",
  "['a', ',', 'b']", NULL, 0 },
{ "matriz: regex_sub em try",
  "try {\n"
  "    import regex\n"
  "    post(regex.sub(r\"(\\d)\", r\"[\\g<1>]\", \"a1\"))\n"
  "} catch(e) {\n"
  "    post(\"CATCH:\", e)\n"
  "}\n"
  "\n",
  "a[1]", NULL, 0 },
{ "matriz: json_emoji em try",
  "try {\n"
  "    import json\n"
  "    post(json.parse(\"{\\\"a\\\": \\\"\\\\ud83d\\\\ude00\\\"}\"))\n"
  "} catch(e) {\n"
  "    post(\"CATCH:\", e)\n"
  "}\n"
  "\n",
  "{'a': '😀'}", NULL, 0 },
{ "matriz: await_lista em try",
  "try {\n"
  "    async action d(n) {\n"
  "        return n * 2\n"
  "    }\n"
  "    post(await [d(1), d(2)])\n"
  "} catch(e) {\n"
  "    post(\"CATCH:\", e)\n"
  "}\n"
  "\n",
  "[2, 4]", NULL, 0 },
{ "matriz: int_inf em try",
  "try {\n"
  "    try {\n"
  "        post(int(flo(\"inf\")))\n"
  "    } catch(e) {\n"
  "        post(\"ok-erro\")\n"
  "    }\n"
  "} catch(e) {\n"
  "    post(\"CATCH:\", e)\n"
  "}\n"
  "\n",
  "ok-erro", NULL, 0 },
{ "matriz: nul em try",
  "try {\n"
  "    post(len(\"a\\x00b\"))\n"
  "} catch(e) {\n"
  "    post(\"CATCH:\", e)\n"
  "}\n"
  "\n",
  "3", NULL, 0 },
{ "matriz: fatia em try",
  "try {\n"
  "    post(\"abcdef\"[999999999999999999999:])\n"
  "} catch(e) {\n"
  "    post(\"CATCH:\", e)\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "matriz: char em finally",
  "try {\n"
  "    char c = 64\n"
  "    post(c)\n"
  "} catch(e) {\n"
  "    post(\"C\")\n"
  "} finally {\n"
  "    post(\"fim\")\n"
  "}\n"
  "\n",
  "@\nfim", NULL, 0 },
{ "matriz: char_texto em finally",
  "try {\n"
  "    char c = \"ç\"\n"
  "    post(c, len(c))\n"
  "} catch(e) {\n"
  "    post(\"C\")\n"
  "} finally {\n"
  "    post(\"fim\")\n"
  "}\n"
  "\n",
  "ç 1\nfim", NULL, 0 },
{ "matriz: closure em finally",
  "try {\n"
  "    a = 1\n"
  "    action inc() {\n"
  "        a = a + 1\n"
  "        return a\n"
  "    }\n"
  "    post(inc(), inc())\n"
  "} catch(e) {\n"
  "    post(\"C\")\n"
  "} finally {\n"
  "    post(\"fim\")\n"
  "}\n"
  "\n",
  "2 3\nfim", NULL, 0 },
{ "matriz: closure_self em finally",
  "try {\n"
  "    x = 5\n"
  "    action le() {\n"
  "        return x\n"
  "    }\n"
  "    post(le())\n"
  "} catch(e) {\n"
  "    post(\"C\")\n"
  "} finally {\n"
  "    post(\"fim\")\n"
  "}\n"
  "\n",
  "5\nfim", NULL, 0 },
{ "matriz: startswith em finally",
  "try {\n"
  "    post(\"abc\".startswith((\"z\", \"a\")))\n"
  "} catch(e) {\n"
  "    post(\"C\")\n"
  "} finally {\n"
  "    post(\"fim\")\n"
  "}\n"
  "\n",
  "True\nfim", NULL, 0 },
{ "matriz: index_faixa em finally",
  "try {\n"
  "    l = [1, 2, 3, 2]\n"
  "    post(l.index(2, 2))\n"
  "} catch(e) {\n"
  "    post(\"C\")\n"
  "} finally {\n"
  "    post(\"fim\")\n"
  "}\n"
  "\n",
  "3\nfim", NULL, 0 },
{ "matriz: encode em finally",
  "try {\n"
  "    post(\"café\".encode(\"latin-1\"))\n"
  "} catch(e) {\n"
  "    post(\"C\")\n"
  "} finally {\n"
  "    post(\"fim\")\n"
  "}\n"
  "\n",
  "b'caf\\xe9'\nfim", NULL, 0 },
{ "matriz: decode em finally",
  "try {\n"
  "    post(\"café\".encode(\"latin-1\").decode(\"latin-1\"))\n"
  "} catch(e) {\n"
  "    post(\"C\")\n"
  "} finally {\n"
  "    post(\"fim\")\n"
  "}\n"
  "\n",
  "café\nfim", NULL, 0 },
{ "matriz: num_base em finally",
  "try {\n"
  "    post(0x1F, 0b101, 1_000, 1e3)\n"
  "} catch(e) {\n"
  "    post(\"C\")\n"
  "} finally {\n"
  "    post(\"fim\")\n"
  "}\n"
  "\n",
  "31 5 1000 1000.0\nfim", NULL, 0 },
{ "matriz: unicode em finally",
  "try {\n"
  "    post(\"Ω\".lower(), \"ß\".upper())\n"
  "} catch(e) {\n"
  "    post(\"C\")\n"
  "} finally {\n"
  "    post(\"fim\")\n"
  "}\n"
  "\n",
  "ω SS\nfim", NULL, 0 },
{ "matriz: isdigit em finally",
  "try {\n"
  "    post(\"²\".isdigit(), \"²\".isdecimal())\n"
  "} catch(e) {\n"
  "    post(\"C\")\n"
  "} finally {\n"
  "    post(\"fim\")\n"
  "}\n"
  "\n",
  "True False\nfim", NULL, 0 },
{ "matriz: regex_split em finally",
  "try {\n"
  "    import regex\n"
  "    post(regex.split(r\"(,)\", \"a,b\"))\n"
  "} catch(e) {\n"
  "    post(\"C\")\n"
  "} finally {\n"
  "    post(\"fim\")\n"
  "}\n"
  "\n",
  "['a', ',', 'b']\nfim", NULL, 0 },
{ "matriz: regex_sub em finally",
  "try {\n"
  "    import regex\n"
  "    post(regex.sub(r\"(\\d)\", r\"[\\g<1>]\", \"a1\"))\n"
  "} catch(e) {\n"
  "    post(\"C\")\n"
  "} finally {\n"
  "    post(\"fim\")\n"
  "}\n"
  "\n",
  "a[1]\nfim", NULL, 0 },
{ "matriz: json_emoji em finally",
  "try {\n"
  "    import json\n"
  "    post(json.parse(\"{\\\"a\\\": \\\"\\\\ud83d\\\\ude00\\\"}\"))\n"
  "} catch(e) {\n"
  "    post(\"C\")\n"
  "} finally {\n"
  "    post(\"fim\")\n"
  "}\n"
  "\n",
  "{'a': '😀'}\nfim", NULL, 0 },
{ "matriz: await_lista em finally",
  "try {\n"
  "    async action d(n) {\n"
  "        return n * 2\n"
  "    }\n"
  "    post(await [d(1), d(2)])\n"
  "} catch(e) {\n"
  "    post(\"C\")\n"
  "} finally {\n"
  "    post(\"fim\")\n"
  "}\n"
  "\n",
  "[2, 4]\nfim", NULL, 0 },
{ "matriz: int_inf em finally",
  "try {\n"
  "    try {\n"
  "        post(int(flo(\"inf\")))\n"
  "    } catch(e) {\n"
  "        post(\"ok-erro\")\n"
  "    }\n"
  "} catch(e) {\n"
  "    post(\"C\")\n"
  "} finally {\n"
  "    post(\"fim\")\n"
  "}\n"
  "\n",
  "ok-erro\nfim", NULL, 0 },
{ "matriz: nul em finally",
  "try {\n"
  "    post(len(\"a\\x00b\"))\n"
  "} catch(e) {\n"
  "    post(\"C\")\n"
  "} finally {\n"
  "    post(\"fim\")\n"
  "}\n"
  "\n",
  "3\nfim", NULL, 0 },
{ "matriz: fatia em finally",
  "try {\n"
  "    post(\"abcdef\"[999999999999999999999:])\n"
  "} catch(e) {\n"
  "    post(\"C\")\n"
  "} finally {\n"
  "    post(\"fim\")\n"
  "}\n"
  "\n",
  "\nfim", NULL, 0 },
{ "matriz: char em metodo",
  "Entity K() {\n"
  "    action m(self) {\n"
  "        char c = 64\n"
  "        post(c)\n"
  "    }\n"
  "}\n"
  "K().m()\n"
  "\n",
  "@", NULL, 0 },
{ "matriz: char_texto em metodo",
  "Entity K() {\n"
  "    action m(self) {\n"
  "        char c = \"ç\"\n"
  "        post(c, len(c))\n"
  "    }\n"
  "}\n"
  "K().m()\n"
  "\n",
  "ç 1", NULL, 0 },
{ "matriz: closure em metodo",
  "Entity K() {\n"
  "    action m(self) {\n"
  "        a = 1\n"
  "        action inc() {\n"
  "            a = a + 1\n"
  "            return a\n"
  "        }\n"
  "        post(inc(), inc())\n"
  "    }\n"
  "}\n"
  "K().m()\n"
  "\n",
  "2 3", NULL, 0 },
{ "matriz: closure_self em metodo",
  "Entity K() {\n"
  "    action m(self) {\n"
  "        x = 5\n"
  "        action le() {\n"
  "            return x\n"
  "        }\n"
  "        post(le())\n"
  "    }\n"
  "}\n"
  "K().m()\n"
  "\n",
  "5", NULL, 0 },
{ "matriz: startswith em metodo",
  "Entity K() {\n"
  "    action m(self) {\n"
  "        post(\"abc\".startswith((\"z\", \"a\")))\n"
  "    }\n"
  "}\n"
  "K().m()\n"
  "\n",
  "True", NULL, 0 },
{ "matriz: index_faixa em metodo",
  "Entity K() {\n"
  "    action m(self) {\n"
  "        l = [1, 2, 3, 2]\n"
  "        post(l.index(2, 2))\n"
  "    }\n"
  "}\n"
  "K().m()\n"
  "\n",
  "3", NULL, 0 },
{ "matriz: encode em metodo",
  "Entity K() {\n"
  "    action m(self) {\n"
  "        post(\"café\".encode(\"latin-1\"))\n"
  "    }\n"
  "}\n"
  "K().m()\n"
  "\n",
  "b'caf\\xe9'", NULL, 0 },
{ "matriz: decode em metodo",
  "Entity K() {\n"
  "    action m(self) {\n"
  "        post(\"café\".encode(\"latin-1\").decode(\"latin-1\"))\n"
  "    }\n"
  "}\n"
  "K().m()\n"
  "\n",
  "café", NULL, 0 },
{ "matriz: num_base em metodo",
  "Entity K() {\n"
  "    action m(self) {\n"
  "        post(0x1F, 0b101, 1_000, 1e3)\n"
  "    }\n"
  "}\n"
  "K().m()\n"
  "\n",
  "31 5 1000 1000.0", NULL, 0 },
{ "matriz: unicode em metodo",
  "Entity K() {\n"
  "    action m(self) {\n"
  "        post(\"Ω\".lower(), \"ß\".upper())\n"
  "    }\n"
  "}\n"
  "K().m()\n"
  "\n",
  "ω SS", NULL, 0 },
{ "matriz: isdigit em metodo",
  "Entity K() {\n"
  "    action m(self) {\n"
  "        post(\"²\".isdigit(), \"²\".isdecimal())\n"
  "    }\n"
  "}\n"
  "K().m()\n"
  "\n",
  "True False", NULL, 0 },
{ "matriz: regex_split em metodo",
  "Entity K() {\n"
  "    action m(self) {\n"
  "        import regex\n"
  "        post(regex.split(r\"(,)\", \"a,b\"))\n"
  "    }\n"
  "}\n"
  "K().m()\n"
  "\n",
  "['a', ',', 'b']", NULL, 0 },
{ "matriz: regex_sub em metodo",
  "Entity K() {\n"
  "    action m(self) {\n"
  "        import regex\n"
  "        post(regex.sub(r\"(\\d)\", r\"[\\g<1>]\", \"a1\"))\n"
  "    }\n"
  "}\n"
  "K().m()\n"
  "\n",
  "a[1]", NULL, 0 },
{ "matriz: json_emoji em metodo",
  "Entity K() {\n"
  "    action m(self) {\n"
  "        import json\n"
  "        post(json.parse(\"{\\\"a\\\": \\\"\\\\ud83d\\\\ude00\\\"}\"))\n"
  "    }\n"
  "}\n"
  "K().m()\n"
  "\n",
  "{'a': '😀'}", NULL, 0 },
{ "matriz: await_lista em metodo",
  "Entity K() {\n"
  "    action m(self) {\n"
  "        async action d(n) {\n"
  "            return n * 2\n"
  "        }\n"
  "        post(await [d(1), d(2)])\n"
  "    }\n"
  "}\n"
  "K().m()\n"
  "\n",
  "[2, 4]", NULL, 0 },
{ "matriz: int_inf em metodo",
  "Entity K() {\n"
  "    action m(self) {\n"
  "        try {\n"
  "            post(int(flo(\"inf\")))\n"
  "        } catch(e) {\n"
  "            post(\"ok-erro\")\n"
  "        }\n"
  "    }\n"
  "}\n"
  "K().m()\n"
  "\n",
  "ok-erro", NULL, 0 },
{ "matriz: nul em metodo",
  "Entity K() {\n"
  "    action m(self) {\n"
  "        post(len(\"a\\x00b\"))\n"
  "    }\n"
  "}\n"
  "K().m()\n"
  "\n",
  "3", NULL, 0 },
{ "matriz: fatia em metodo",
  "Entity K() {\n"
  "    action m(self) {\n"
  "        post(\"abcdef\"[999999999999999999999:])\n"
  "    }\n"
  "}\n"
  "K().m()\n"
  "\n",
  "", NULL, 0 },
{ "matriz: char em aninhada",
  "action fora() {\n"
  "    action dentro() {\n"
  "        char c = 64\n"
  "        post(c)\n"
  "    }\n"
  "    dentro()\n"
  "}\n"
  "fora()\n"
  "\n",
  "@", NULL, 0 },
{ "matriz: char_texto em aninhada",
  "action fora() {\n"
  "    action dentro() {\n"
  "        char c = \"ç\"\n"
  "        post(c, len(c))\n"
  "    }\n"
  "    dentro()\n"
  "}\n"
  "fora()\n"
  "\n",
  "ç 1", NULL, 0 },
{ "matriz: closure em aninhada",
  "action fora() {\n"
  "    action dentro() {\n"
  "        a = 1\n"
  "        action inc() {\n"
  "            a = a + 1\n"
  "            return a\n"
  "        }\n"
  "        post(inc(), inc())\n"
  "    }\n"
  "    dentro()\n"
  "}\n"
  "fora()\n"
  "\n",
  "2 3", NULL, 0 },
{ "matriz: closure_self em aninhada",
  "action fora() {\n"
  "    action dentro() {\n"
  "        x = 5\n"
  "        action le() {\n"
  "            return x\n"
  "        }\n"
  "        post(le())\n"
  "    }\n"
  "    dentro()\n"
  "}\n"
  "fora()\n"
  "\n",
  "5", NULL, 0 },
{ "matriz: startswith em aninhada",
  "action fora() {\n"
  "    action dentro() {\n"
  "        post(\"abc\".startswith((\"z\", \"a\")))\n"
  "    }\n"
  "    dentro()\n"
  "}\n"
  "fora()\n"
  "\n",
  "True", NULL, 0 },
{ "matriz: index_faixa em aninhada",
  "action fora() {\n"
  "    action dentro() {\n"
  "        l = [1, 2, 3, 2]\n"
  "        post(l.index(2, 2))\n"
  "    }\n"
  "    dentro()\n"
  "}\n"
  "fora()\n"
  "\n",
  "3", NULL, 0 },
{ "matriz: encode em aninhada",
  "action fora() {\n"
  "    action dentro() {\n"
  "        post(\"café\".encode(\"latin-1\"))\n"
  "    }\n"
  "    dentro()\n"
  "}\n"
  "fora()\n"
  "\n",
  "b'caf\\xe9'", NULL, 0 },
{ "matriz: decode em aninhada",
  "action fora() {\n"
  "    action dentro() {\n"
  "        post(\"café\".encode(\"latin-1\").decode(\"latin-1\"))\n"
  "    }\n"
  "    dentro()\n"
  "}\n"
  "fora()\n"
  "\n",
  "café", NULL, 0 },
{ "matriz: num_base em aninhada",
  "action fora() {\n"
  "    action dentro() {\n"
  "        post(0x1F, 0b101, 1_000, 1e3)\n"
  "    }\n"
  "    dentro()\n"
  "}\n"
  "fora()\n"
  "\n",
  "31 5 1000 1000.0", NULL, 0 },
{ "matriz: unicode em aninhada",
  "action fora() {\n"
  "    action dentro() {\n"
  "        post(\"Ω\".lower(), \"ß\".upper())\n"
  "    }\n"
  "    dentro()\n"
  "}\n"
  "fora()\n"
  "\n",
  "ω SS", NULL, 0 },
{ "matriz: isdigit em aninhada",
  "action fora() {\n"
  "    action dentro() {\n"
  "        post(\"²\".isdigit(), \"²\".isdecimal())\n"
  "    }\n"
  "    dentro()\n"
  "}\n"
  "fora()\n"
  "\n",
  "True False", NULL, 0 },
{ "matriz: regex_split em aninhada",
  "action fora() {\n"
  "    action dentro() {\n"
  "        import regex\n"
  "        post(regex.split(r\"(,)\", \"a,b\"))\n"
  "    }\n"
  "    dentro()\n"
  "}\n"
  "fora()\n"
  "\n",
  "['a', ',', 'b']", NULL, 0 },
{ "matriz: regex_sub em aninhada",
  "action fora() {\n"
  "    action dentro() {\n"
  "        import regex\n"
  "        post(regex.sub(r\"(\\d)\", r\"[\\g<1>]\", \"a1\"))\n"
  "    }\n"
  "    dentro()\n"
  "}\n"
  "fora()\n"
  "\n",
  "a[1]", NULL, 0 },
{ "matriz: json_emoji em aninhada",
  "action fora() {\n"
  "    action dentro() {\n"
  "        import json\n"
  "        post(json.parse(\"{\\\"a\\\": \\\"\\\\ud83d\\\\ude00\\\"}\"))\n"
  "    }\n"
  "    dentro()\n"
  "}\n"
  "fora()\n"
  "\n",
  "{'a': '😀'}", NULL, 0 },
{ "matriz: await_lista em aninhada",
  "action fora() {\n"
  "    action dentro() {\n"
  "        async action d(n) {\n"
  "            return n * 2\n"
  "        }\n"
  "        post(await [d(1), d(2)])\n"
  "    }\n"
  "    dentro()\n"
  "}\n"
  "fora()\n"
  "\n",
  "[2, 4]", NULL, 0 },
{ "matriz: int_inf em aninhada",
  "action fora() {\n"
  "    action dentro() {\n"
  "        try {\n"
  "            post(int(flo(\"inf\")))\n"
  "        } catch(e) {\n"
  "            post(\"ok-erro\")\n"
  "        }\n"
  "    }\n"
  "    dentro()\n"
  "}\n"
  "fora()\n"
  "\n",
  "ok-erro", NULL, 0 },
{ "matriz: nul em aninhada",
  "action fora() {\n"
  "    action dentro() {\n"
  "        post(len(\"a\\x00b\"))\n"
  "    }\n"
  "    dentro()\n"
  "}\n"
  "fora()\n"
  "\n",
  "3", NULL, 0 },
{ "matriz: fatia em aninhada",
  "action fora() {\n"
  "    action dentro() {\n"
  "        post(\"abcdef\"[999999999999999999999:])\n"
  "    }\n"
  "    dentro()\n"
  "}\n"
  "fora()\n"
  "\n",
  "", NULL, 0 },
{ "matriz: char em gerador",
  "action g() {\n"
  "    char c = 64\n"
  "    post(c)\n"
  "    yield 1\n"
  "}\n"
  "for each _v in g() {\n"
  "    _u = _v\n"
  "}\n"
  "\n",
  "@", NULL, 0 },
{ "matriz: char_texto em gerador",
  "action g() {\n"
  "    char c = \"ç\"\n"
  "    post(c, len(c))\n"
  "    yield 1\n"
  "}\n"
  "for each _v in g() {\n"
  "    _u = _v\n"
  "}\n"
  "\n",
  "ç 1", NULL, 0 },
{ "matriz: closure em gerador",
  "action g() {\n"
  "    a = 1\n"
  "    action inc() {\n"
  "        a = a + 1\n"
  "        return a\n"
  "    }\n"
  "    post(inc(), inc())\n"
  "    yield 1\n"
  "}\n"
  "for each _v in g() {\n"
  "    _u = _v\n"
  "}\n"
  "\n",
  "2 3", NULL, 0 },
{ "matriz: closure_self em gerador",
  "action g() {\n"
  "    x = 5\n"
  "    action le() {\n"
  "        return x\n"
  "    }\n"
  "    post(le())\n"
  "    yield 1\n"
  "}\n"
  "for each _v in g() {\n"
  "    _u = _v\n"
  "}\n"
  "\n",
  "5", NULL, 0 },
{ "matriz: startswith em gerador",
  "action g() {\n"
  "    post(\"abc\".startswith((\"z\", \"a\")))\n"
  "    yield 1\n"
  "}\n"
  "for each _v in g() {\n"
  "    _u = _v\n"
  "}\n"
  "\n",
  "True", NULL, 0 },
{ "matriz: index_faixa em gerador",
  "action g() {\n"
  "    l = [1, 2, 3, 2]\n"
  "    post(l.index(2, 2))\n"
  "    yield 1\n"
  "}\n"
  "for each _v in g() {\n"
  "    _u = _v\n"
  "}\n"
  "\n",
  "3", NULL, 0 },
{ "matriz: encode em gerador",
  "action g() {\n"
  "    post(\"café\".encode(\"latin-1\"))\n"
  "    yield 1\n"
  "}\n"
  "for each _v in g() {\n"
  "    _u = _v\n"
  "}\n"
  "\n",
  "b'caf\\xe9'", NULL, 0 },
{ "matriz: decode em gerador",
  "action g() {\n"
  "    post(\"café\".encode(\"latin-1\").decode(\"latin-1\"))\n"
  "    yield 1\n"
  "}\n"
  "for each _v in g() {\n"
  "    _u = _v\n"
  "}\n"
  "\n",
  "café", NULL, 0 },
{ "matriz: num_base em gerador",
  "action g() {\n"
  "    post(0x1F, 0b101, 1_000, 1e3)\n"
  "    yield 1\n"
  "}\n"
  "for each _v in g() {\n"
  "    _u = _v\n"
  "}\n"
  "\n",
  "31 5 1000 1000.0", NULL, 0 },
{ "matriz: unicode em gerador",
  "action g() {\n"
  "    post(\"Ω\".lower(), \"ß\".upper())\n"
  "    yield 1\n"
  "}\n"
  "for each _v in g() {\n"
  "    _u = _v\n"
  "}\n"
  "\n",
  "ω SS", NULL, 0 },
{ "matriz: isdigit em gerador",
  "action g() {\n"
  "    post(\"²\".isdigit(), \"²\".isdecimal())\n"
  "    yield 1\n"
  "}\n"
  "for each _v in g() {\n"
  "    _u = _v\n"
  "}\n"
  "\n",
  "True False", NULL, 0 },
{ "matriz: regex_split em gerador",
  "action g() {\n"
  "    import regex\n"
  "    post(regex.split(r\"(,)\", \"a,b\"))\n"
  "    yield 1\n"
  "}\n"
  "for each _v in g() {\n"
  "    _u = _v\n"
  "}\n"
  "\n",
  "['a', ',', 'b']", NULL, 0 },
{ "matriz: regex_sub em gerador",
  "action g() {\n"
  "    import regex\n"
  "    post(regex.sub(r\"(\\d)\", r\"[\\g<1>]\", \"a1\"))\n"
  "    yield 1\n"
  "}\n"
  "for each _v in g() {\n"
  "    _u = _v\n"
  "}\n"
  "\n",
  "a[1]", NULL, 0 },
{ "matriz: json_emoji em gerador",
  "action g() {\n"
  "    import json\n"
  "    post(json.parse(\"{\\\"a\\\": \\\"\\\\ud83d\\\\ude00\\\"}\"))\n"
  "    yield 1\n"
  "}\n"
  "for each _v in g() {\n"
  "    _u = _v\n"
  "}\n"
  "\n",
  "{'a': '😀'}", NULL, 0 },
{ "matriz: await_lista em gerador",
  "action g() {\n"
  "    async action d(n) {\n"
  "        return n * 2\n"
  "    }\n"
  "    post(await [d(1), d(2)])\n"
  "    yield 1\n"
  "}\n"
  "for each _v in g() {\n"
  "    _u = _v\n"
  "}\n"
  "\n",
  "[2, 4]", NULL, 0 },
{ "matriz: int_inf em gerador",
  "action g() {\n"
  "    try {\n"
  "        post(int(flo(\"inf\")))\n"
  "    } catch(e) {\n"
  "        post(\"ok-erro\")\n"
  "    }\n"
  "    yield 1\n"
  "}\n"
  "for each _v in g() {\n"
  "    _u = _v\n"
  "}\n"
  "\n",
  "ok-erro", NULL, 0 },
{ "matriz: nul em gerador",
  "action g() {\n"
  "    post(len(\"a\\x00b\"))\n"
  "    yield 1\n"
  "}\n"
  "for each _v in g() {\n"
  "    _u = _v\n"
  "}\n"
  "\n",
  "3", NULL, 0 },
{ "matriz: fatia em gerador",
  "action g() {\n"
  "    post(\"abcdef\"[999999999999999999999:])\n"
  "    yield 1\n"
  "}\n"
  "for each _v in g() {\n"
  "    _u = _v\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "matriz: char em async",
  "async action a() {\n"
  "    char c = 64\n"
  "    post(c)\n"
  "    return 1\n"
  "}\n"
  "post(await a())\n"
  "\n",
  "@\n1", NULL, 0 },
{ "matriz: char_texto em async",
  "async action a() {\n"
  "    char c = \"ç\"\n"
  "    post(c, len(c))\n"
  "    return 1\n"
  "}\n"
  "post(await a())\n"
  "\n",
  "ç 1\n1", NULL, 0 },
{ "matriz: closure em async",
  "async action a() {\n"
  "    a = 1\n"
  "    action inc() {\n"
  "        a = a + 1\n"
  "        return a\n"
  "    }\n"
  "    post(inc(), inc())\n"
  "    return 1\n"
  "}\n"
  "post(await a())\n"
  "\n",
  "2 3\n1", NULL, 0 },
{ "matriz: closure_self em async",
  "async action a() {\n"
  "    x = 5\n"
  "    action le() {\n"
  "        return x\n"
  "    }\n"
  "    post(le())\n"
  "    return 1\n"
  "}\n"
  "post(await a())\n"
  "\n",
  "5\n1", NULL, 0 },
{ "matriz: startswith em async",
  "async action a() {\n"
  "    post(\"abc\".startswith((\"z\", \"a\")))\n"
  "    return 1\n"
  "}\n"
  "post(await a())\n"
  "\n",
  "True\n1", NULL, 0 },
{ "matriz: index_faixa em async",
  "async action a() {\n"
  "    l = [1, 2, 3, 2]\n"
  "    post(l.index(2, 2))\n"
  "    return 1\n"
  "}\n"
  "post(await a())\n"
  "\n",
  "3\n1", NULL, 0 },
{ "matriz: encode em async",
  "async action a() {\n"
  "    post(\"café\".encode(\"latin-1\"))\n"
  "    return 1\n"
  "}\n"
  "post(await a())\n"
  "\n",
  "b'caf\\xe9'\n1", NULL, 0 },
{ "matriz: decode em async",
  "async action a() {\n"
  "    post(\"café\".encode(\"latin-1\").decode(\"latin-1\"))\n"
  "    return 1\n"
  "}\n"
  "post(await a())\n"
  "\n",
  "café\n1", NULL, 0 },
{ "matriz: num_base em async",
  "async action a() {\n"
  "    post(0x1F, 0b101, 1_000, 1e3)\n"
  "    return 1\n"
  "}\n"
  "post(await a())\n"
  "\n",
  "31 5 1000 1000.0\n1", NULL, 0 },
{ "matriz: unicode em async",
  "async action a() {\n"
  "    post(\"Ω\".lower(), \"ß\".upper())\n"
  "    return 1\n"
  "}\n"
  "post(await a())\n"
  "\n",
  "ω SS\n1", NULL, 0 },
{ "matriz: isdigit em async",
  "async action a() {\n"
  "    post(\"²\".isdigit(), \"²\".isdecimal())\n"
  "    return 1\n"
  "}\n"
  "post(await a())\n"
  "\n",
  "True False\n1", NULL, 0 },
{ "matriz: regex_split em async",
  "async action a() {\n"
  "    import regex\n"
  "    post(regex.split(r\"(,)\", \"a,b\"))\n"
  "    return 1\n"
  "}\n"
  "post(await a())\n"
  "\n",
  "['a', ',', 'b']\n1", NULL, 0 },
{ "matriz: regex_sub em async",
  "async action a() {\n"
  "    import regex\n"
  "    post(regex.sub(r\"(\\d)\", r\"[\\g<1>]\", \"a1\"))\n"
  "    return 1\n"
  "}\n"
  "post(await a())\n"
  "\n",
  "a[1]\n1", NULL, 0 },
{ "matriz: json_emoji em async",
  "async action a() {\n"
  "    import json\n"
  "    post(json.parse(\"{\\\"a\\\": \\\"\\\\ud83d\\\\ude00\\\"}\"))\n"
  "    return 1\n"
  "}\n"
  "post(await a())\n"
  "\n",
  "{'a': '😀'}\n1", NULL, 0 },
{ "matriz: await_lista em async",
  "async action a() {\n"
  "    async action d(n) {\n"
  "        return n * 2\n"
  "    }\n"
  "    post(await [d(1), d(2)])\n"
  "    return 1\n"
  "}\n"
  "post(await a())\n"
  "\n",
  "[2, 4]\n1", NULL, 0 },
{ "matriz: int_inf em async",
  "async action a() {\n"
  "    try {\n"
  "        post(int(flo(\"inf\")))\n"
  "    } catch(e) {\n"
  "        post(\"ok-erro\")\n"
  "    }\n"
  "    return 1\n"
  "}\n"
  "post(await a())\n"
  "\n",
  "ok-erro\n1", NULL, 0 },
{ "matriz: nul em async",
  "async action a() {\n"
  "    post(len(\"a\\x00b\"))\n"
  "    return 1\n"
  "}\n"
  "post(await a())\n"
  "\n",
  "3\n1", NULL, 0 },
{ "matriz: fatia em async",
  "async action a() {\n"
  "    post(\"abcdef\"[999999999999999999999:])\n"
  "    return 1\n"
  "}\n"
  "post(await a())\n"
  "\n",
  "\n1", NULL, 0 },
{ "matriz: char em using",
  "using open(\"/tmp/ps_mtz.txt\", \"w\") as _f {\n"
  "    char c = 64\n"
  "    post(c)\n"
  "}\n"
  "\n",
  "@", NULL, 0 },
{ "matriz: char_texto em using",
  "using open(\"/tmp/ps_mtz.txt\", \"w\") as _f {\n"
  "    char c = \"ç\"\n"
  "    post(c, len(c))\n"
  "}\n"
  "\n",
  "ç 1", NULL, 0 },
{ "matriz: closure em using",
  "using open(\"/tmp/ps_mtz.txt\", \"w\") as _f {\n"
  "    a = 1\n"
  "    action inc() {\n"
  "        a = a + 1\n"
  "        return a\n"
  "    }\n"
  "    post(inc(), inc())\n"
  "}\n"
  "\n",
  "2 3", NULL, 0 },
{ "matriz: closure_self em using",
  "using open(\"/tmp/ps_mtz.txt\", \"w\") as _f {\n"
  "    x = 5\n"
  "    action le() {\n"
  "        return x\n"
  "    }\n"
  "    post(le())\n"
  "}\n"
  "\n",
  "5", NULL, 0 },
{ "matriz: startswith em using",
  "using open(\"/tmp/ps_mtz.txt\", \"w\") as _f {\n"
  "    post(\"abc\".startswith((\"z\", \"a\")))\n"
  "}\n"
  "\n",
  "True", NULL, 0 },
{ "matriz: index_faixa em using",
  "using open(\"/tmp/ps_mtz.txt\", \"w\") as _f {\n"
  "    l = [1, 2, 3, 2]\n"
  "    post(l.index(2, 2))\n"
  "}\n"
  "\n",
  "3", NULL, 0 },
{ "matriz: encode em using",
  "using open(\"/tmp/ps_mtz.txt\", \"w\") as _f {\n"
  "    post(\"café\".encode(\"latin-1\"))\n"
  "}\n"
  "\n",
  "b'caf\\xe9'", NULL, 0 },
{ "matriz: decode em using",
  "using open(\"/tmp/ps_mtz.txt\", \"w\") as _f {\n"
  "    post(\"café\".encode(\"latin-1\").decode(\"latin-1\"))\n"
  "}\n"
  "\n",
  "café", NULL, 0 },
{ "matriz: num_base em using",
  "using open(\"/tmp/ps_mtz.txt\", \"w\") as _f {\n"
  "    post(0x1F, 0b101, 1_000, 1e3)\n"
  "}\n"
  "\n",
  "31 5 1000 1000.0", NULL, 0 },
{ "matriz: unicode em using",
  "using open(\"/tmp/ps_mtz.txt\", \"w\") as _f {\n"
  "    post(\"Ω\".lower(), \"ß\".upper())\n"
  "}\n"
  "\n",
  "ω SS", NULL, 0 },
{ "matriz: isdigit em using",
  "using open(\"/tmp/ps_mtz.txt\", \"w\") as _f {\n"
  "    post(\"²\".isdigit(), \"²\".isdecimal())\n"
  "}\n"
  "\n",
  "True False", NULL, 0 },
{ "matriz: regex_split em using",
  "using open(\"/tmp/ps_mtz.txt\", \"w\") as _f {\n"
  "    import regex\n"
  "    post(regex.split(r\"(,)\", \"a,b\"))\n"
  "}\n"
  "\n",
  "['a', ',', 'b']", NULL, 0 },
{ "matriz: regex_sub em using",
  "using open(\"/tmp/ps_mtz.txt\", \"w\") as _f {\n"
  "    import regex\n"
  "    post(regex.sub(r\"(\\d)\", r\"[\\g<1>]\", \"a1\"))\n"
  "}\n"
  "\n",
  "a[1]", NULL, 0 },
{ "matriz: json_emoji em using",
  "using open(\"/tmp/ps_mtz.txt\", \"w\") as _f {\n"
  "    import json\n"
  "    post(json.parse(\"{\\\"a\\\": \\\"\\\\ud83d\\\\ude00\\\"}\"))\n"
  "}\n"
  "\n",
  "{'a': '😀'}", NULL, 0 },
{ "matriz: await_lista em using",
  "using open(\"/tmp/ps_mtz.txt\", \"w\") as _f {\n"
  "    async action d(n) {\n"
  "        return n * 2\n"
  "    }\n"
  "    post(await [d(1), d(2)])\n"
  "}\n"
  "\n",
  "[2, 4]", NULL, 0 },
{ "matriz: int_inf em using",
  "using open(\"/tmp/ps_mtz.txt\", \"w\") as _f {\n"
  "    try {\n"
  "        post(int(flo(\"inf\")))\n"
  "    } catch(e) {\n"
  "        post(\"ok-erro\")\n"
  "    }\n"
  "}\n"
  "\n",
  "ok-erro", NULL, 0 },
{ "matriz: nul em using",
  "using open(\"/tmp/ps_mtz.txt\", \"w\") as _f {\n"
  "    post(len(\"a\\x00b\"))\n"
  "}\n"
  "\n",
  "3", NULL, 0 },
{ "matriz: fatia em using",
  "using open(\"/tmp/ps_mtz.txt\", \"w\") as _f {\n"
  "    post(\"abcdef\"[999999999999999999999:])\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "matriz: char em match",
  "match 1 {\n"
  "    case 1 {\n"
  "        char c = 64\n"
  "        post(c)\n"
  "    }\n"
  "}\n"
  "\n",
  "@", NULL, 0 },
{ "matriz: char_texto em match",
  "match 1 {\n"
  "    case 1 {\n"
  "        char c = \"ç\"\n"
  "        post(c, len(c))\n"
  "    }\n"
  "}\n"
  "\n",
  "ç 1", NULL, 0 },
{ "matriz: closure em match",
  "match 1 {\n"
  "    case 1 {\n"
  "        a = 1\n"
  "        action inc() {\n"
  "            a = a + 1\n"
  "            return a\n"
  "        }\n"
  "        post(inc(), inc())\n"
  "    }\n"
  "}\n"
  "\n",
  "2 3", NULL, 0 },
{ "matriz: closure_self em match",
  "match 1 {\n"
  "    case 1 {\n"
  "        x = 5\n"
  "        action le() {\n"
  "            return x\n"
  "        }\n"
  "        post(le())\n"
  "    }\n"
  "}\n"
  "\n",
  "5", NULL, 0 },
{ "matriz: startswith em match",
  "match 1 {\n"
  "    case 1 {\n"
  "        post(\"abc\".startswith((\"z\", \"a\")))\n"
  "    }\n"
  "}\n"
  "\n",
  "True", NULL, 0 },
{ "matriz: index_faixa em match",
  "match 1 {\n"
  "    case 1 {\n"
  "        l = [1, 2, 3, 2]\n"
  "        post(l.index(2, 2))\n"
  "    }\n"
  "}\n"
  "\n",
  "3", NULL, 0 },
{ "matriz: encode em match",
  "match 1 {\n"
  "    case 1 {\n"
  "        post(\"café\".encode(\"latin-1\"))\n"
  "    }\n"
  "}\n"
  "\n",
  "b'caf\\xe9'", NULL, 0 },
{ "matriz: decode em match",
  "match 1 {\n"
  "    case 1 {\n"
  "        post(\"café\".encode(\"latin-1\").decode(\"latin-1\"))\n"
  "    }\n"
  "}\n"
  "\n",
  "café", NULL, 0 },
{ "matriz: num_base em match",
  "match 1 {\n"
  "    case 1 {\n"
  "        post(0x1F, 0b101, 1_000, 1e3)\n"
  "    }\n"
  "}\n"
  "\n",
  "31 5 1000 1000.0", NULL, 0 },
{ "matriz: unicode em match",
  "match 1 {\n"
  "    case 1 {\n"
  "        post(\"Ω\".lower(), \"ß\".upper())\n"
  "    }\n"
  "}\n"
  "\n",
  "ω SS", NULL, 0 },
{ "matriz: isdigit em match",
  "match 1 {\n"
  "    case 1 {\n"
  "        post(\"²\".isdigit(), \"²\".isdecimal())\n"
  "    }\n"
  "}\n"
  "\n",
  "True False", NULL, 0 },
{ "matriz: regex_split em match",
  "match 1 {\n"
  "    case 1 {\n"
  "        import regex\n"
  "        post(regex.split(r\"(,)\", \"a,b\"))\n"
  "    }\n"
  "}\n"
  "\n",
  "['a', ',', 'b']", NULL, 0 },
{ "matriz: regex_sub em match",
  "match 1 {\n"
  "    case 1 {\n"
  "        import regex\n"
  "        post(regex.sub(r\"(\\d)\", r\"[\\g<1>]\", \"a1\"))\n"
  "    }\n"
  "}\n"
  "\n",
  "a[1]", NULL, 0 },
{ "matriz: json_emoji em match",
  "match 1 {\n"
  "    case 1 {\n"
  "        import json\n"
  "        post(json.parse(\"{\\\"a\\\": \\\"\\\\ud83d\\\\ude00\\\"}\"))\n"
  "    }\n"
  "}\n"
  "\n",
  "{'a': '😀'}", NULL, 0 },
{ "matriz: await_lista em match",
  "match 1 {\n"
  "    case 1 {\n"
  "        async action d(n) {\n"
  "            return n * 2\n"
  "        }\n"
  "        post(await [d(1), d(2)])\n"
  "    }\n"
  "}\n"
  "\n",
  "[2, 4]", NULL, 0 },
{ "matriz: int_inf em match",
  "match 1 {\n"
  "    case 1 {\n"
  "        try {\n"
  "            post(int(flo(\"inf\")))\n"
  "        } catch(e) {\n"
  "            post(\"ok-erro\")\n"
  "        }\n"
  "    }\n"
  "}\n"
  "\n",
  "ok-erro", NULL, 0 },
{ "matriz: nul em match",
  "match 1 {\n"
  "    case 1 {\n"
  "        post(len(\"a\\x00b\"))\n"
  "    }\n"
  "}\n"
  "\n",
  "3", NULL, 0 },
{ "matriz: fatia em match",
  "match 1 {\n"
  "    case 1 {\n"
  "        post(\"abcdef\"[999999999999999999999:])\n"
  "    }\n"
  "}\n"
  "\n",
  "", NULL, 0 },
};
const int NC_COBERTURA = N_CASOS(CASOS_COBERTURA);
