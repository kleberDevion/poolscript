/*
 * Semântica da linguagem: escopo, laço, string/UTF-8, coleção, arquivo,
 * import. Casos que já foram bug pelo menos uma vez — e o básico ao redor
 * deles, pra a correção não passar por cima do que funcionava.
 */
#include "ps_teste.h"

const Caso CASOS_LINGUAGEM[] = {

/* ── as tres limitacoes que a doc listava, e que viraram conserto ─────────
 *
 * `docs/LANGUAGE.md` tinha um bloco "Limitacoes e comportamentos conhecidos"
 * com cinco itens. DUAS eram falsas — `for each c in "abc" { }` funciona, e
 * `elif`/`else` em linha nova tambem — e o mesmo arquivo dizia o contrario
 * algumas linhas antes. As tres verdadeiras estao consertadas aqui.
 *
 * O oraculo e o CPython: cada esperado abaixo foi colhido rodando o mesmo
 * caso no python3. */

/* 1. try/finally SEM catch. Era `SyntaxError: esperado 'catch'`. */
{ "try/finally sem catch roda o finally",
  "try {\n"
  "    post(\"dentro\")\n"
  "}\n"
  "finally {\n"
  "    post(\"fim\")\n"
  "}\n",
  "dentro\nfim", NULL, 0 },
{ "try/finally sem catch: a excecao PROPAGA depois do finally",
  /* O conserto do parser sozinho engolia o erro: o compilador so emitia
   * RERAISE quando algum catch existia e nao casava. Saia rc=0 e o erro
   * sumia — trocar um SyntaxError por um erro calado seria piorar. */
  "try {\n"
  "    post(\"dentro\")\n"
  "    raise ValueError(\"x\")\n"
  "}\n"
  "finally {\n"
  "    post(\"fim\")\n"
  "}\n",
  "dentro\nfim", "ValueError: x", 1 },
{ "try/finally sem catch: o finally roda antes do return",
  "funct f() {\n"
  "    try {\n"
  "        return \"A\"\n"
  "    }\n"
  "    finally {\n"
  "        post(\"fim\")\n"
  "    }\n"
  "}\n"
  "post(f())\n",
  "fim\nA", NULL, 0 },
{ "try/finally aninhado: o de fora pega o que o de dentro deixou passar",
  "try {\n"
  "    try {\n"
  "        raise KeyError(\"k\")\n"
  "    }\n"
  "    finally {\n"
  "        post(\"interno\")\n"
  "    }\n"
  "}\n"
  "catch (KeyError e) {\n"
  "    post(\"pegou\")\n"
  "}\n",
  "interno\npegou", NULL, 0 },
{ "try sozinho, sem catch nem finally, continua sendo erro",
  /* o `try` nao pediria nada; e a unica forma que segue recusada */
  "try {\n"
  "    post(\"a\")\n"
  "}\n",
  "", "esperado 'catch' ou 'finally' apos bloco do try", 2 },

/* 2. escape invalido: mantem a barra E avisa, como o CPython. */
{ "escape invalido MANTEM a barra",
  /* Era `C:pasta` (7 chars): a barra sumia, calada. O python3 da
   * 'C:\\pasta' com 8 — perder um byte do dado em silencio e o pior dos
   * dois mundos. O aviso vai pro stderr e nao entra no stdout. */
  "x = \"C:\\pasta\"\n"
  "post(x, len(x))\n",
  "C:\\pasta 8", "SyntaxWarning: sequencia de escape invalida '\\p' \xe2\x80\x94 a barra fica no texto; use '\\\\p' se ela e mesmo pra estar ali", 0 },
{ "escape CONHECIDO segue igual",
  "post(len(\"a\\nb\"), len(\"a\\tb\"), len(\"a\\\\b\"))\n",
  "3 3 3", NULL, 0 },
{ "aviso nao e erro: o programa roda ate o fim",
  "x = \"\\q\"\n"
  "post(\"terminei\")\n",
  "terminei", "SyntaxWarning: sequencia de escape invalida '\\q' \xe2\x80\x94 a barra fica no texto; use '\\\\q' se ela e mesmo pra estar ali", 0 },

/* 3. unpacking no for each. Era `SyntaxError: esperado 'in'`. */
{ "for each com dois nomes desempacota",
  "for each a, b in [[1, 2], [3, 4]] {\n"
  "    post(a, b)\n"
  "}\n",
  "1 2\n3 4", NULL, 0 },
{ "for each com tres nomes, e aninhado",
  "for each a, b, c in [[1, 2, 3]] {\n"
  "    post(a, b, c)\n"
  "}\n"
  "for each x, (y, z) in [[1, [2, 3]]] {\n"
  "    post(x, y, z)\n"
  "}\n",
  "1 2 3\n1 2 3", NULL, 0 },
{ "for each com UM nome continua igual",
  "for each x in [1, 2, 3] {\n"
  "    post(x)\n"
  "}\n",
  "1\n2\n3", NULL, 0 },
{ "for each desempacota com a quantidade errada: a mensagem e a do CPython",
  "for each a, b in [[1, 2, 3]] {\n"
  "    post(a)\n"
  "}\n",
  "", "too many values to unpack (expected 2)", 1 },
{ "for each desempacota o que nao e sequencia",
  "for each a, b in [1, 2] {\n"
  "    post(a)\n"
  "}\n",
  "", "cannot unpack non-iterable int object", 1 },

/* ── alvo de desempacotamento: nao era so nome ────────────────────────────
 *
 * `lista[c], lista[c + 1] = lista[c + 1], lista[c]` — a troca do bubble sort —
 * dava `SyntaxError: expressao invalida`, e junto com ela `d["a"], d["b"] = `
 * e `o.x, o.y = `. A linguagem tinha desempacotamento (`a, b = b, a`) e tinha
 * atribuicao indexada (`l[0] = 9`); o que nao existia era a interseccao.
 *
 * O oraculo e o CPython: cada esperado abaixo foi colhido rodando o mesmo
 * caso no python3, saida e mensagem de erro. */
{ "troca com alvo indexado (o bubble sort dele)",
  "int funct main(lista){\n"
  "    n = len(lista)\n"
  "    for each i in range(n - 1){\n"
  "        for each c in range(0, n - 1 - i){\n"
  "            if (lista[c] > lista[c + 1]) {\n"
  "                lista[c], lista[c + 1] = lista[c + 1], lista[c]\n"
  "            }\n"
  "        }\n"
  "    }\n"
  "    return lista\n"
  "}\n"
  "post(main([5,1,4,3,6,7]))\n",
  "[1, 3, 4, 5, 6, 7]", NULL, 0 },
{ "alvo indexado: chave de dict dos dois lados",
  "d = {}\n"
  "d[\"a\"], d[\"b\"] = 1, 2\n"
  "post(d)\n",
  "{'a': 1, 'b': 2}", NULL, 0 },
{ "alvo indexado: indice negativo",
  "l = [1, 2, 3]\n"
  "l[-1], l[0] = l[0], l[-1]\n"
  "post(l)\n",
  "[3, 2, 1]", NULL, 0 },
{ "alvo membro de Entity",
  "Entity O() {\n"
  "    x: int\n"
  "    y: int\n"
  "}\n"
  "o = O(0, 0)\n"
  "o.x, o.y = 5, 6\n"
  "post(o.x, o.y)\n",
  "5 6", NULL, 0 },
{ "alvo em cadeia: membro e depois indice",
  "Entity O() {\n"
  "    d: dict\n"
  "}\n"
  "o = O({})\n"
  "o.d[\"k\"], z = 9, 8\n"
  "post(o.d, z)\n",
  "{'k': 9} 8", NULL, 0 },
{ "alvo indexado dentro de grupo aninhado",
  "a = [1, 2, 3]\n"
  "a[0], (a[1], a[2]) = 9, (8, 7)\n"
  "post(a)\n",
  "[9, 8, 7]", NULL, 0 },
{ "alvo indexado com estrela",
  "x = [0, 0, 0]\n"
  "*x[0], y = [1, 2, 3]\n"
  "post(x, y)\n",
  "[[1, 2], 0, 0] 3", NULL, 0 },
{ "o mesmo alvo duas vezes: vence o da direita",
  /* CPython guarda da ESQUERDA pra direita, entao o ultimo store manda. */
  "a = [0]\n"
  "a[0], a[0] = 1, 2\n"
  "post(a)\n",
  "[2]", NULL, 0 },
{ "o indice e calculado na hora de escrever, nao antes",
  /* `l[i], i = 5, 1`: o alvo da esquerda usa o i ANTIGO (0), porque os alvos
   * recebem em ordem e o `i` so muda no segundo. */
  "l = [0, 0]\n"
  "i = 0\n"
  "l[i], i = 5, 1\n"
  "post(l, i)\n",
  "[5, 0] 1", NULL, 0 },
{ "alvo indexado fora do alcance: a mensagem e a de ESCRITA",
  "l = [1, 2]\n"
  "x = 0\n"
  "l[5], x = 1, 2\n",
  "", "IndexError: list assignment index out of range", 1 },
{ "alvo indexado em tupla: nao aceita item assignment",
  "t = (1, 2)\n"
  "x = 0\n"
  "t[0], x = 9, 8\n",
  "", "TypeError: 'tup' object does not support item assignment", 1 },
{ "quantidade errada reprova ANTES de escrever em alvo nenhum",
  "l = [0, 0]\n"
  "l[0], l[1] = [1]\n"
  "post(l)\n",
  "", "ValueError: not enough values to unpack (expected 2, got 1)", 1 },
{ "for each com alvo indexado unico",
  "l = [0, 0]\n"
  "for each l[0] in [7, 8] { }\n"
  "post(l)\n",
  "[8, 0]", NULL, 0 },
{ "for each com alvo indexado na segunda posicao",
  "l = [0, 0]\n"
  "for each a, l[0] in [[1, 2], [3, 4]] {\n"
  "    post(a)\n"
  "}\n"
  "post(l)\n",
  "1\n3\n[4, 0]", NULL, 0 },
{ "chamada continua NAO sendo alvo",
  /* O lookahead que passou a aceitar `[` e `.` nao pode aceitar `f(`: sem
   * isso `post(a, b)` viraria desempacotamento. */
  "funct f(x) { return x }\n"
  "post(f(1), f(2))\n",
  "1 2", NULL, 0 },
{ "atribuicao indexada sozinha continua igual",
  "l = [1, 2]\n"
  "l[0] = 9\n"
  "l[1] += 5\n"
  "post(l)\n",
  "[9, 7]", NULL, 0 },

/* ── `private <tipo> <nome> = <valor>` ────────────────────────────────────
 *
 * Encapsulamento so existia como `nome: tipo` no corpo da classe. Quem
 * declarava o campo no construtor — `private str name = nome`, a forma que
 * ele pediu — nao levava erro: `private` chegava no parser de EXPRESSAO e
 * virava um nome comum. O programa compilava limpo e estourava
 * `NameError: name 'private' is not defined` so em runtime; dentro de
 * `int funct` isso vira o 500 da secao 6.4, ou seja, some.
 *
 * O irmao do mesmo bug: `str x = "a"` NO CORPO DA CLASSE virava um VarDecl
 * empurrado pra lista de METODOS (que so olha N_ACTION_DECL). Compilava,
 * sumia, e `self.x` dava AttributeError sem uma linha de aviso. */
{ "campo declarado no construtor, com tipo e visibilidade",
  "private Class Pagamento() {\n"
  "    public funct __init__(self, nome, doc) {\n"
  "        private str name = nome\n"
  "        private int cpf = doc\n"
  "    }\n"
  "    public funct mostra(self) {\n"
  "        return self.name + \"/\" + str(self.cpf)\n"
  "    }\n"
  "}\n"
  "post(Pagamento(\"ana\", 123).mostra())\n",
  "ana/123", NULL, 0 },
{ "campo private declarado no construtor BARRA de fora",
  /* Registrar a visibilidade e o ponto: `private` que compila e nao barra e
   * pior que nao ter encapsulamento, porque parece que tem. */
  "Class A() {\n"
  "    public funct __init__(self, nome) {\n"
  "        private str name = nome\n"
  "    }\n"
  "}\n"
  "post(A(\"ana\").name)\n",
  "", "acesso negado: 'name' e private de A", 1 },
{ "public declarado no construtor NAO barra",
  "Class A() {\n"
  "    public funct __init__(self, nome) {\n"
  "        public str name = nome\n"
  "    }\n"
  "}\n"
  "post(A(\"ana\").name)\n",
  "ana", NULL, 0 },
{ "o tipo do campo e conferido como o da variavel",
  "Class A() {\n"
  "    public funct __init__(self) {\n"
  "        private int n = 5.9\n"
  "    }\n"
  "}\n"
  "a = A()\n",
  "", "AttributedValueError: variável n esperava int", 1 },
{ "tipo nao escalar guarda sem conferir, como na variavel",
  "Class A() {\n"
  "    public funct __init__(self) {\n"
  "        public list itens = [1, 2]\n"
  "    }\n"
  "    public funct ver(self) { return self.itens }\n"
  "}\n"
  "post(A().ver())\n",
  "[1, 2]", NULL, 0 },
{ "a mesma declaracao vale NO CORPO da classe",
  "Class A() {\n"
  "    str x = \"a\"\n"
  "    public funct ver(self) { return self.x }\n"
  "}\n"
  "post(A().ver())\n",
  "a", NULL, 0 },
{ "no corpo da classe, sem default, vira parametro do construtor",
  "Class A() {\n"
  "    str x\n"
  "    public funct ver(self) { return self.x }\n"
  "}\n"
  "post(A(\"oi\").ver())\n",
  "oi", NULL, 0 },
{ "private no corpo da classe pela forma nova tambem barra",
  "Class A() {\n"
  "    private str x = \"a\"\n"
  "}\n"
  "post(A().x)\n",
  "", "acesso negado: 'x' e private de A", 1 },
{ "int funct dentro da Entity continua sendo funct, nao campo",
  /* A condicao que separa `int funct f()` de `str x = 1` no corpo da
   * classe: depois do tipo de RETORNO vem sempre outra palavra da
   * linguagem. */
  "Class A() {\n"
  "    int funct f(self) { return 7 }\n"
  "    public async int funct g(self) { return 8 }\n"
  "}\n"
  "a = A()\n"
  "post(a.f(), await a.g())\n",
  "7 8", NULL, 0 },
{ "a ordem 'nome: tipo' dentro da funct diz o conserto",
  /* Era `SyntaxError: expressao invalida` apontando pro ':'. */
  "Class A() {\n"
  "    public funct __init__(self, nome) {\n"
  "        private name: str = nome\n"
  "    }\n"
  "}\n",
  "", "dentro de uma funct escreva 'private <tipo> name = <valor>'", 2 },
{ "private sem tipo nenhum nao passa mais calado",
  /* Antes: compilava, e `private` virava um nome inexistente em runtime. */
  "Class A() {\n"
  "    public funct __init__(self, nome) {\n"
  "        private name = nome\n"
  "    }\n"
  "}\n",
  "", "'private' so vale antes de class/Entity", 2 },
{ "campo do objeto exige uma funct com self",
  "funct f(n) {\n"
  "    private str x = n\n"
  "}\n",
  "", "so vale dentro de uma funct de Entity que recebe 'self'", 2 },
/* `//` deixou de ser comentário no I11. A mensagem tinha que dizer isso. */
{ "'//' no lugar de expressao diz que virou divisao inteira",
  "x = 1\n"
  "// comentario velho\n"
  "post(x)\n",
  "", "'//' e divisao inteira, nao comentario", 2 },
{ "'//' indentado dentro de bloco diz o mesmo",
  "for each i in [1] {\n"
  "    // nota\n"
  "    post(i)\n"
  "}\n",
  "", "'//' e divisao inteira, nao comentario", 2 },
{ "a divisao inteira de verdade continua",
  "post(7 // 2, -7 // 2, 10 // 3)\n",
  "3 -4 3", NULL, 0 },

{ "private class e private funct continuam valendo",
  "private Class A() {\n"
  "    private saldo: int\n"
  "    public funct ver(self) { return self.saldo }\n"
  "    private funct log(self) { return \"x\" }\n"
  "}\n"
  "post(A(10).ver())\n",
  "10", NULL, 0 },

/* ── escopo ── */
{ "for each sombreia variável de fora",
  "i = \"importante\"\n"
  "for each i in [1, 2] {\n"
  "    post(i)\n"
  "}\n"
  "post(i)\n",
  "1\n2\nimportante", NULL, 0 },
{ "for each não apaga funct de mesmo nome",
  "funct f() {\n"
  "    return \"sou funct\"\n"
  "}\n"
  "for each f in [1, 2] {\n"
  "    post(f)\n"
  "}\n"
  "post(f())\n",
  "1\n2\nsou funct", NULL, 0 },
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
  "import request\npost(type(request.head))\n", "funct", NULL, 0 },
{ "request tem os métodos HTTP",
  "import request\npost(type(request.get), type(request.post), type(request.delete))\n",
  "funct funct funct", NULL, 0 },

/* ── closure: funct aninhada captura o escopo de fora ──────────────────
 * A captura é por CÉLULA (o modelo do CPython): quem declara e quem captura
 * mexem no mesmo valor, então a aninhada VÊ e MUTA a variável de fora. */
{ "aninhada lê local de fora",
  "funct fora() {\n"
  "    a = 1\n"
  "    funct dentro() {\n"
  "        post(a)\n"
  "    }\n"
  "    dentro()\n"
  "}\n"
  "fora()\n",
  "1", NULL, 0 },
{ "aninhada chama a si mesma (recursão)",
  "funct fora() {\n"
  "    funct rec(n) {\n"
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
  "    funct __init__(self) {\n"
  "        self.v = 3\n"
  "    }\n"
  "    funct m(self) {\n"
  "        funct inner() {\n"
  "            return self.v\n"
  "        }\n"
  "        return inner()\n"
  "    }\n"
  "}\n"
  "c = C()\n"
  "post(c.m())\n",
  "3", NULL, 0 },
{ "contador: a aninhada MUTA a variável de fora",
  "funct faz() {\n"
  "    n = 0\n"
  "    funct inc() {\n"
  "        n = n + 1\n"
  "        return n\n"
  "    }\n"
  "    return inc\n"
  "}\n"
  "c = faz()\n"
  "post(c(), c(), c())\n",
  "1 2 3", NULL, 0 },
{ "cada closure tem o próprio estado",
  "funct faz() {\n"
  "    n = 0\n"
  "    funct inc() {\n"
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
  "funct n1() {\n"
  "    a = 10\n"
  "    funct n2() {\n"
  "        funct n3() {\n"
  "            return a * 2\n"
  "        }\n"
  "        return n3()\n"
  "    }\n"
  "    return n2()\n"
  "}\n"
  "post(n1())\n",
  "20", NULL, 0 },
{ "closure como callback do map",
  "funct mult(k) {\n"
  "    funct f(x) {\n"
  "        return x * k\n"
  "    }\n"
  "    return f\n"
  "}\n"
  "post(map([1, 2, 3], mult(10)))\n",
  "[10, 20, 30]", NULL, 0 },
{ "parâmetro capturado",
  "funct soma(a) {\n"
  "    funct mais(b) {\n"
  "        return a + b\n"
  "    }\n"
  "    return mais\n"
  "}\n"
  "post(soma(3)(4))\n",
  "7", NULL, 0 },
{ "closures do laço compartilham a variável",
  "funct junta() {\n"
  "    fs = []\n"
  "    for each i in range(3) {\n"
  "        funct g() {\n"
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
  "funct mutuo(n) {\n"
  "    funct par(k) {\n"
  "        if k == 0 {\n"
  "            return true\n"
  "        }\n"
  "        return impar(k - 1)\n"
  "    }\n"
  "    funct impar(k) {\n"
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
  "funct f() {\n"
  "    funct dentro() {\n"
  "        return G\n"
  "    }\n"
  "    return dentro()\n"
  "}\n"
  "post(f())\n",
  "99", NULL, 0 },
{ "gerador aninhado mantém a captura",
  "funct faz(k) {\n"
  "    funct g() {\n"
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
{ "closure em async funct (fibra)",
  "async funct t(k) {\n"
  "    funct calc() {\n"
  "        return k * 3\n"
  "    }\n"
  "    return calc()\n"
  "}\n"
  "post(gather(t(1), t(2)))\n",
  "[3, 6]", NULL, 0 },
{ "closure sobrevive ao GC e continua mutando",
  "funct cont() {\n"
  "    n = 0\n"
  "    funct inc() {\n"
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
  "funct cria(n) {\n"
  "    funct f() {\n"
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
{ "type() de closure é funct",
  "funct f() {\n"
  "    a = 1\n"
  "    funct g() {\n"
  "        return a\n"
  "    }\n"
  "    return g\n"
  "}\n"
  "post(type(f()))\n",
  "funct", NULL, 0 },
{ "variável capturada usada antes de receber valor é erro",
  "funct f() {\n"
  "    funct g() {\n"
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
  "funct g() {\n"
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
{ "dicionario multilinha dentro de funct",
  "funct f() {\n"
  "    return {\"a\": 1,\n"
  "            \"b\": 2}\n"
  "}\n"
  "post(f())\n",
  "{'a': 1, 'b': 2}", NULL, 0 },
{ "dicionario com a chave em linha propria",
  "d = {\n    \"a\": 1,\n    \"b\": 2\n}\npost(d)\n", "{'a': 1, 'b': 2}", NULL, 0 },
{ "lista multilinha indentada",
  "funct g() {\n"
  "    return [1,\n"
  "            2]\n"
  "}\n"
  "post(g())\n", "[1, 2]", NULL, 0 },
{ "dicionario aninhado multilinha",
  "d = {\"a\": {\"b\": 1,\n            \"c\": 2},\n     \"d\": 3}\npost(d)\n",
  "{'a': {'b': 1, 'c': 2}, 'd': 3}", NULL, 0 },
{ "dicionario como argumento multilinha",
  "funct f(x) {\n"
  "    return x[\"a\"]\n"
  "}\n"
  "post(f({\"a\": 1,\n"
  "        \"b\": 2}))\n", "1", NULL, 0 },
{ "bloco de chaves continua sendo bloco depois de )",
  "if (true) {\n"
  "    funct f() {\n"
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
 * A linguagem sempre recusou `f(1,2,3)` numa funct de zero parâmetros, mas
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
  "    funct f() {\n"
  "        return 1\n"
  "    }\n"
  "    post(f())\n"
  "}\n", "1", NULL, 0 },
{ "metodo ':' dentro de Entity de chaves",
  "Entity P() {\n"
  "    funct m(self) {\n"
  "        return 3\n"
  "    }\n"
  "}\n"
  "post(P().m())\n", "3", NULL, 0 },
{ "metodo de chaves dentro de Entity ':'",
  "Entity Q() {\n"
  "    funct m(self) {\n"
  "        return 4\n"
  "    }\n"
  "}\n"
  "post(Q().m())\n", "4", NULL, 0 },
{ "indentacao livre dentro de chaves",
  "funct r(n) {\n if (n < 1) {\n  return 0\n }\n return 1 + r(n - 1)\n}\npost(r(5))\n",
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
  "funct range(n) {\n"
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
/* `char funct` passou a EXISTIR (2026-09-08): todo tipo vale como retorno. O
 * `char` aqui declara o retorno e nao coage — devolve o 1 como esta. Quem
 * coage e a declaracao de VARIAVEL (`char c = 1`), logo acima. */
{ "char funct existe e nao coage o retorno",
  "char funct f() {\n"
  "    return 1\n"
  "}\npost(f())\n", "1", NULL, 0 },
/* ── NENHUMA conversao implicita, 2026-09-09 ───────────────────────────────
 * `int b = "7"` virava 7 e `flo c = 1` virava 1.0 ("conversoes que nao perdem
 * informacao"). Ele nunca pediu isso: "se eu tenho terra eu transformo em
 * cacau em po?". A declaracao CONFERE o tipo e nao converte nada; converter e
 * escrever `int("7")`, `flo(5)`. O `ConversionError` da declaracao de int/flo
 * deixou de existir — sobrou so no `char c = -1`. */
{ "declaracao tipada aceita o tipo EXATO",
  "str a = \"oi\"\nint b = 7\nflo c = 1.0\nbool d = true\npost(a, b, c, d)\n",
  "oi 7 1.0 True", NULL, 0 },
{ "int NAO aceita string numerica: nao converte",
  "int b = \"7\"\n", "", "AttributedValueError: variável b esperava int", 1 },
{ "flo NAO aceita int: nao alarga",
  "flo c = 1\n", "", "AttributedValueError: variável c esperava flo", 1 },
{ "flo NAO aceita string numerica",
  "flo x = \"1.5\"\n", "", "AttributedValueError: variável x esperava flo", 1 },
{ "int recusa texto invalido pelo MESMO erro (nao ha 'quase converteu')",
  "int x = \"abc\"\n", "", "AttributedValueError: variável x esperava int", 1 },
{ "conversao e explicita: tipo(valor)",
  "int n = int(\"7\")\nflo f = flo(5)\nstr s = str(42)\npost(n, f, s)\n", "7 5.0 42", NULL, 0 },
{ "long aceita int porque int E inteiro — nao e conversao",
  "long y = 5\npost(y)\n", "5", NULL, 0 },

/* ── módulos sem `import` ────────────────────────────────────────────────
 * `Parsing` é namespace pré-ligado e `sys.stdout`/`sys.stderr` são atributos:
 * nenhum dos três se importa. O `--metadata` chegou a anunciá-los pelo nome de
 * REGISTRO (`_Parsing`, `_stdout`), e aí o editor sugeria `import _stdout`, que
 * só podia dar ImportError. */
{ "Parsing existe sem import",
  "post(type(Parsing), type(Parsing.integer))\n", "module funct", NULL, 0 },
{ "sys.stdout e sys.stderr são atributos de sys",
  "import sys\npost(type(sys.stdout), type(sys.stderr), type(sys.stdout.write))\n",
  "module module funct", NULL, 0 },
{ "import de nome interno é erro",
  "import _stdout\n", "", "ImportError: No module named '_stdout'", 1 },

/* ── `pass` — no-op igual ao Python ─────────────────────────────────────────
 * Nasceu porque a doc do middleware do jinker mandava usar `continue` fora de
 * laço, que não compila. `pass` vale em QUALQUER posição de statement. */
{ "pass como corpo de funct devolve null",
  "funct f() {\n"
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
  "funct f() {\n"
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
  "funct f() { pass }\npost(f())\n", "Null", NULL, 0 },
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
 * `Entity`/`class`/`funct` aceitavam, porque o cabeçalho deles pula
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
{ "funct com chave na linha seguinte continua valendo",
  "funct f()\n{\n    return 7\n}\npost(f())\n", "7", NULL, 0 },
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
{ "compreensao dentro de funct",
  "funct f(l) {\n    return [v + 1 for each v in l]\n}\npost(f([1,2]))\n", "[2, 3]", NULL, 0 },
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
  "Entity A():\n    funct m(self) { return 1 }\n", "", "bloco com ':' nao existe mais", 2 },
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
 * módulo: a funct aninhada apontava pro protótipo de OUTRA função do
 * programa principal. `poe()` dentro de `um()` virava `um()` — recursão
 * infinita. Aqui o arquivo se importa, que é o caminho mais curto pra passar
 * pela relocação. */
{ "closure dentro de modulo importado",
  "import ps_mod_clo\n"
  "funct com_closure() {\n"
  "    l = []\n"
  "    funct poe(r) {\n"
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
  "funct contador() {\n"
  "    n = 0\n"
  "    funct inc() {\n"
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
  "funct f() {\n"
  "    e = \"x\"\n"
  "    raise Boom(f\"erro: {e}\")\n"
  "}\n"
  "f()\n", "", "linha 3", 1 },
{ "erro DENTRO da f-string aponta a linha da f-string",
  "funct f() {\n"
  "    x = 0\n"
  "\n"
  "    post(f\"v: {1 / x}\")\n"
  "}\n"
  "f()\n", "", "linha 4", 1 },
{ "f-string no meio nao desloca o que vem depois",
  "funct f() {\n"
  "    e = 1\n"
  "    post(f\"a {e}\")\n"
  "    raise Boom(\"y\")\n"
  "}\n"
  "f()\n", "a 1", "linha 4", 1 },
{ "f-string continua interpolando",
  "n = 7\ns = \"ana\"\npost(f\"{s} tem {n}\", f\"{n * 2}\")\n", "ana tem 7 14", NULL, 0 },
{ "f-string aninhada em chamada aninhada",
  "funct g(x) {\n    return x\n}\n"
  "v = 3\npost(g(f\"v={v}\"))\n", "v=3", NULL, 0 },

/* ── input(): fim da entrada é `null`, linha vazia é `""` ───────────────────
 * O runner roda todo caso com stdin em /dev/null, então aqui a entrada já
 * começa acabada. Sem o `null`, `while true: input()` giraria pra sempre
 * quando o outro lado fechasse o cano — foi o que travou o servidor LSP. */
/* ── sys.stdin.raw() — modo cru do terminal, 2026-09-09 ────────────────────
 * A suíte roda com stdin em /dev/null (não é terminal), então aqui `raw` só
 * pode ser CONFERIDO no caminho não-tty: devolve false e não estoura, e a
 * leitura segue devolvendo Null no fim (o polling em tty tem prova própria em
 * examples/cobrinha.ps, que precisa de pty). */
/* ── sys.argv na convencao C/Python, 2026-09-09 ────────────────────────────
 * `argv[0]` e o NOME DO SCRIPT; os argumentos do usuario vem de `argv[1]`. O
 * runner roda `pool <arquivo>` sem argumentos extras, entao argv tem SO o [0]
 * (o script). Antes o nome do script ficava de fora e argv[0] ja era o 1o
 * argumento — quem lia argv[1] levava IndexError numa lista "sem limite". */
/* ── TODO ARQUIVO É PoolFile, 2026-09-09 ───────────────────────────────────
 * O handle de `open()` dizia `FileHandle` no `type()`, `Arquivo` no
 * `--metadata` (o que a doc e o LSP liam) e nao tinha `move`/`copy`/`path`/
 * `name`/`ext`/`size`: tres nomes e dois conjuntos de metodo pra mesma ideia.
 * Quem procurava a doc de `FileHandle` nao achava pagina nenhuma. Agora o
 * tipo e UM: `PoolFile`, e o que vale num vale no outro. */
{ "open() devolve PoolFile",
  "f = open(\"u1.txt\", \"w\")\npost(type(f))\nf.close()\n", "PoolFile", NULL, 0 },
{ "PoolFile aberto tem name, ext e size",
  "f = open(\"u2.txt\", \"w\")\nf.write(\"12345\")\npost(f.name, f.ext, f.size)\nf.close()\n",
  "u2.txt .txt 5", NULL, 0 },
/* O `ext` com ponto e o MESMO do PoolFile carregado — um tipo so nao pode dar
 * duas respostas pro mesmo campo. */
/* O fixture escrevia so `\x89PNG` — quatro bytes sem NUL nenhum, um PNG
 * FALSO. Passava porque a extensao `.png` estava numa lista fixa; agora quem
 * decide o modo automatico e o CONTEUDO, e conteudo sem NUL e texto. O
 * cabecalho de verdade continua depois da assinatura: `\x00\x00\x00\rIHDR`. */
{ "ext do aberto e do carregado sao iguais",
  "import os\nusing open(\"u3.png\", \"wb\") as f { f.write(\"\\x89PNG\\r\\n\\x1a\\n\\x00\\x00\\x00\\rIHDR\") }\n"
  "a = os.loadFile(\"u3.png\")\nf2 = open(\"u3.png\", \"rb\")\npost(a.ext, f2.ext)\nf2.close()\n",
  ".png .png", NULL, 0 },
{ "PoolFile aberto: path, bytes, copy e delete",
  "import os\nf = open(\"u4.txt\", \"w\")\nf.write(\"dados\")\nf.close()\n"
  "post(f.path().endswith(\"u4.txt\"), f.bytes())\n"
  "f.copy(\"u5.txt\")\npost(os.isfile(\"u5.txt\"))\nf.delete()\npost(os.isfile(\"u4.txt\"))\n",
  "True b'dados'\nTrue\nFalse", NULL, 0 },
{ "PoolFile aberto: move leva o arquivo e atualiza o caminho",
  "import os\nf = open(\"u6.txt\", \"w\")\nf.write(\"x\")\nf.close()\nf.move(\"u7.txt\")\n"
  "post(os.isfile(\"u7.txt\"), os.isfile(\"u6.txt\"), f.name)\n",
  "True False u7.txt", NULL, 0 },
/* Mexer no arquivo com o descritor ABERTO faria a escrita seguinte ir pro
 * lugar antigo. Recusa dizendo o que fazer, em vez de perder o dado calado. */
{ "move num PoolFile ainda aberto e recusado com a saida escrita",
  "f = open(\"u8.txt\", \"w\")\nf.write(\"x\")\nf.move(\"u9.txt\")\n",
  "", "move() num arquivo ainda ABERTO — chame .close() antes", 1 },
{ "sys.argv[0] e o nome do script; sem args tem so ele",
  "import sys\npost(type(sys.argv), len(sys.argv), type(sys.argv[0]))\n",
  "list 1 str", NULL, 0 },
{ "raw() sem terminal devolve false, nao estoura",
  "import sys\npost(sys.stdin.raw(true), sys.stdin.raw(false))\n", "False False", NULL, 0 },
{ "raw() nao muda a leitura de pipe: Null no fim",
  "import sys\nsys.stdin.raw(true)\npost(type(sys.stdin.read()))\n", "Null", NULL, 0 },
{ "raw() exige bool",
  "import sys\nsys.stdin.raw(1)\n", "", "raw() espera bool", 1 },
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
{ "fecha o bloco na linha do ultimo comando: funct",
  "funct f() {\n"
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
  "@app.route(\"/x\")\nfunct h() { return 1 }\n", "", "name 'app' is not defined", 1 },
/* Condicao que COMECA com parentese: `if (a) or (b) {` dava "esperado inicio
 * de bloco com '{'" — o parser lia o grupo como se fosse a forma `if (cond) {`
 * e exigia o bloco logo depois do `)`. O parentese e so precedencia; a
 * condicao vai ate o `{`. Achado escrevendo o gerador do corpus diferencial. */
{ "condicao que comeca com parentese: `if (a) or (b) {` e `while (a) and (b) {`",
  "x = 1\n"
  "if (x == 1) or (x == 2) {\n"
  "    post(\"sim\")\n"
  "}\n"
  "while (x < 2) and (x > 0) {\n"
  "    post(\"laco\")\n"
  "    x = x + 1\n"
  "}\n"
  "if (x == 2) {\n"
  "    post(\"so um grupo continua valendo\")\n"
  "}\n",
  "sim\nlaco\nso um grupo continua valendo", NULL, 0 },
/* Campo SEM tipo no corpo da classe: `conexao = ""`, `LIMITE = 10`, com ou sem
 * `private`. Era recusado ("so sao permitidas declaracoes 'funct'..."): quem
 * declara um campo como declara uma variavel era repelido. E a terceira
 * grafia do mesmo no (`nome: tipo`, `tipo nome`, `nome = valor`), dinamica
 * como `x = 1`, e entra no construtor sintetizado como argumento opcional. */
{ "campo sem tipo no corpo da classe: `nome = valor`, `private nome = valor`",
  "class P() {\n"
  "    nome = \"a\"\n"
  "    private conexao = \"\"\n"
  "    LIMITE = 10\n"
  "    n: int = 2\n"
  "    funct m(self) {\n"
  "        return self.nome + str(self.n) + str(self.LIMITE) + self.conexao\n"
  "    }\n"
  "}\n"
  "post(P().m())\n"
  "post(P(\"z\", \"-\", 5, 7).m())\n",
  "a210\nz75-", NULL, 0 },

/* ── TIPAGEM ESTATICA: o tipo declarado e da VARIAVEL ──────────────────────
 * Antes a checagem valia so na criacao e `str s = "a"` seguido de `s = 42`
 * passava (a doc chamava de "dinamica"). Agora toda escrita num nome
 * declarado com tipo confere: reatribuicao, for each, desempacotamento,
 * write-through de dentro de uma funct, closure. Sem tipo declarado nada
 * muda. `Object` e o tipo de qualquer objeto (instancia, servidor, arquivo). */
{ "tipagem estatica: `str s` recusa `s = 42` depois",
  "str s = \"a\"\n"
  "s = 42\n"
  "post(s)\n",
  "", "AttributedValueError: variável s esperava str", 1 },
/* Sem conversao implicita (2026-09-09): a reatribuicao confere o tipo EXATO,
 * igual a criacao. `n = "7"` num `int n` era convertido pra 7; agora e erro. */
{ "tipagem estatica: `int n` recusa `n = \"7\"` — nao converte na escrita",
  "int n = 1\n"
  "n = \"7\"\n",
  "", "AttributedValueError: variável n esperava int", 1 },
{ "tipagem estatica: `flo f` recusa int na escrita; flo passa",
  "flo f = 1.0\n"
  "f = 2.0\n"
  "post(f)\n"
  "f = 2\n",
  "2.0", "AttributedValueError: variável f esperava flo", 1 },
{ "tipagem estatica: `list l` recusa string; `dict d` recusa lista",
  "list l = [1]\n"
  "l = \"x\"\n",
  "", "AttributedValueError: variável l esperava list", 1 },
{ "tipagem estatica: Object recebe instancia de classe e o app do jinker",
  "from jinker import Jinker\n"
  "class C() {\n"
  "    funct m(self) {\n"
  "        return 1\n"
  "    }\n"
  "}\n"
  "Object app = Jinker(__name__)\n"
  "object c = C()\n"
  "post(c.m())\n",
  "1", NULL, 0 },
{ "tipagem estatica: Object recusa str e lista",
  "Object o = \"x\"\n",
  "", "AttributedValueError: variável o esperava Object", 1 },

/* ── FUNCT E OBJETO, 2026-09-10 ────────────────────────────────────────────
 * `Object f = funct(){ … }` era recusado: `V_FUNC`/`V_NATIVE` nao sao
 * `V_OBJ` (o valor carrega o indice do proto, nao um ponteiro), entao o
 * teste do `Object` os deixava de fora — embora o comentario dele no C ja
 * prometesse "função". A incoerencia aparecia dentro da MESMA expressao:
 * lambda que captura vira closure (que e objeto) e passava; lambda que nao
 * captura nao passava. */
{ "Object aceita lambda",
  "Object f = funct(){ return 1 }\npost(type(f), f())\n", "funct 1", NULL, 0 },
{ "Object aceita funct nomeada, builtin e metodo",
  "funct nom() { return 1 }\n"
  "class C() { funct m(self) { return 2 } }\n"
  "Object a = nom\nObject b = post\nObject c = C().m\n"
  "post(a(), c())\n", "1 2", NULL, 0 },
{ "Object no parametro aceita funct",
  "funct roda(Object cb) { return cb() }\npost(roda(funct(){ return 7 }))\n",
  "7", NULL, 0 },
{ "Object continua recusando escalar e colecao",
  "Object o = 1\n", "", "AttributedValueError: variável o esperava Object", 1 },

/* Funct e igual a si mesma. `V_FUNC`/`V_NATIVE` caiam no `return 0` final do
 * comparador, entao `f == f` respondia False — inclusive pra builtin. */
{ "funct e igual a si mesma, e duas referencias a mesma funct sao iguais",
  "funct nom() { return 1 }\nf = nom\ng = nom\n"
  "lam = funct(){ return 1 }\nh = lam\n"
  "post(f == f, f == g, post == post, lam == lam, lam == h)\n",
  "True True True True True", NULL, 0 },
{ "duas lambdas escritas separadas NAO sao iguais",
  "a = funct(){ return 1 }\nb = funct(){ return 1 }\npost(a == b, a != b)\n",
  "False True", NULL, 0 },
{ "funct serve de chave de dict: hash e igualdade batem",
  "funct nom() { return 1 }\nd = {}\nd[nom] = \"x\"\n"
  "post(d[nom], nom in d)\n", "x True", NULL, 0 },
{ "tipagem estatica: global tipado escrito de dentro de uma funct confere",
  "str s = \"a\"\n"
  "funct f() {\n"
  "    s = 5\n"
  "}\n"
  "f()\n",
  "", "AttributedValueError: variável s esperava str", 1 },
{ "tipagem estatica: local tipado capturado por closure confere na escrita de dentro",
  "funct f() {\n"
  "    int n = 1\n"
  "    funct g() {\n"
  "        n = \"z\"\n"
  "    }\n"
  "    g()\n"
  "    return n\n"
  "}\n"
  "post(f())\n",
  "", "AttributedValueError: variável n esperava int", 1 },
{ "tipagem estatica: `for each` num nome declarado confere cada volta",
  "str s = \"a\"\n"
  "for each s in [1, 2] {\n"
  "    post(s)\n"
  "}\n",
  "", "AttributedValueError: variável s esperava str", 1 },
{ "sem tipo declarado continua livre: `x = 1` depois `x = \"a\"`",
  "x = 1\n"
  "x = \"a\"\n"
  "post(x)\n",
  "a", NULL, 0 },
/* Apelidos de tipo, decisao dele: string/String = str, integer/Integer = int,
 * tuple/Tuple = tup, dictionary/Dictionary = dict — a mesma regra do tipo que
 * apelidam, inclusive a estatica. */
{ "apelidos de tipo: string/Integer/tuple/Dictionary declaram e conferem como str/int/tup/dict",
  "string s = \"a\"\n"
  "Integer n = 7\n"
  "post(n + 1)\n"
  "tuple t = (1, 2)\n"
  "Dictionary d = {\"k\": 1}\n"
  "post(len(t), d[\"k\"])\n"
  "s = 1\n",
  "8\n2 1", "AttributedValueError: variável s esperava str", 1 },
{ "apelido de tipo NAO e palavra reservada: `string` como variavel e como argumento nomeado",
  "import regex\n"
  "string = 5\n"
  "post(string)\n"
  "post(regex.sub(\"a\", \"b\", string=\"aXa\"))\n",
  "5\nbXb", NULL, 0 },

/* ── achados da auditoria doc x motor (2026-09-06), cada um medido antes ── */
{ "guard `if __name__ == \"main\"` com a chave na linha seguinte RODA",
  "if __name__ == \"main\"\n"
  "{\n"
  "    post(\"rodou\")\n"
  "}\n"
  "post(\"fim\")\n",
  "rodou\nfim", NULL, 0 },
{ "guard `if (__name__ == \"main\")` com parenteses e chave na linha seguinte RODA",
  "if (__name__ == \"main\")\n"
  "{\n"
  "    post(\"rodou\")\n"
  "}\n"
  "post(\"fim\")\n",
  "rodou\nfim", NULL, 0 },
{ "`match (expr) {` — sujeito entre parenteses e statement, nao dict",
  "match (1 + 1) {\n"
  "    case 2 {\n"
  "        post(\"dois\")\n"
  "    }\n"
  "}\n",
  "dois", NULL, 0 },
{ "Entity sem __init__ e sem campo nao recebe argumento",
  "Entity Zero() {\n"
  "    funct m(self) {\n"
  "        return 1\n"
  "    }\n"
  "}\n"
  "post(Zero().m())\n"
  "Zero(1, 2, 3)\n",
  "1", "TypeError: Zero() takes no arguments (3 given)", 1 },
{ "@static chamado pela instancia: a mensagem diz pra chamar pela Entity",
  "Entity Mat() {\n"
  "    @static\n"
  "    funct soma(a, b) {\n"
  "        return a + b\n"
  "    }\n"
  "}\n"
  "post(Mat.soma(1, 2))\n"
  "m = Mat()\n"
  "m.soma(1, 2)\n",
  "3", "RuntimeError: funct 'soma' e static: chame pela Entity (Tipo.soma(...)), nao pela instancia", 1 },
{ "zip: a mensagem nomeia o argumento que NAO itera, nao o primeiro",
  "zip([1], 5)\n",
  "", "TypeError: 'int' object is not iterable", 1 },
{ "match com guarda falsa dentro de funct nao corrompe o slot do case seguinte",
  "funct m(valor) {\n"
  "    match valor {\n"
  "        case 200 {\n"
  "            post(\"ok\")\n"
  "        }\n"
  "        case v if v < 50 {\n"
  "            post(\"barato\")\n"
  "        }\n"
  "        case _ {\n"
  "            post(\"outro\")\n"
  "        }\n"
  "    }\n"
  "}\n"
  "m(999)\n"
  "m(7)\n"
  "m(200)\n",
  "outro\nbarato\nok", NULL, 0 },
/* A cabeça da funct sob um decorador é a MESMA unidade do statement:
 * `[public|private] {async|tipo}* funct|funct`, em qualquer ordem. O
 * lookahead do decorador era uma cópia à mão que conhecia quatro formas e
 * não conhecia `tipo async funct`: `@app.post(...)` sobre `int async funct
 * h()` compilava limpo, a funct virava statement solto SEM decorador, a rota
 * nunca era registrada e o cliente via 404 — sem aviso. O `register` abaixo
 * imprime quando é chamado; o caso reprova se o decorador não pegar. */
{ "decorador pega `int async funct` (tipo antes de async)",
  "class Reg() {\n"
  "    funct reg(self) {\n"
  "        return self\n"
  "    }\n"
  "    funct register(self, fn) {\n"
  "        post(\"registrou\")\n"
  "        return fn\n"
  "    }\n"
  "}\n"
  "r = Reg()\n"
  "@r.reg()\n"
  "int async funct h() {\n"
  "    return 1\n"
  "}\n",
  "registrou", NULL, 0 },
{ "decorador pega `public int async funct` e `bool async funct`",
  "class Reg() {\n"
  "    funct reg(self) {\n"
  "        return self\n"
  "    }\n"
  "    funct register(self, fn) {\n"
  "        post(\"registrou\")\n"
  "        return fn\n"
  "    }\n"
  "}\n"
  "r = Reg()\n"
  "@r.reg()\n"
  "public int async funct h() {\n"
  "    return 1\n"
  "}\n"
  "@r.reg()\n"
  "bool async funct g() {\n"
  "    return true\n"
  "}\n",
  "registrou\nregistrou", NULL, 0 },
{ "import sem nome nenhum",
  "import\n", "", "esperado nome de modulo depois de 'import'", 2 },
{ "import valido continua valendo",
  "import sys\npost(type(sys))\n", "module", NULL, 0 },
/* ── `funct` e os modificadores COLADOS (`static funct`), 2026-09-06 ────────
 * A funcao passou a se chamar `funct`; `funct`/`funct` continuam valendo,
 * sao a mesma declaracao. `@static` e `@NonNull` viraram modificadores colados
 * na cabeca (`static funct m()`), e os decoradores antigos seguem funcionando.
 * A ordem dos modificadores e do usuario: a matriz inteira entra aqui, porque
 * cada ponto do parser que conhecia a cabeca tinha a sua copia a mao — e a que
 * esquecia um modificador deixava a declaracao virar statement solto, calada. */
{ "funct declara funcao",
  "funct soma(a, b) {\n    return a + b\n}\npost(soma(2, 3))\n", "5", NULL, 0 },
{ "funct com tipo de retorno, async e visibilidade",
  "int funct d(n) {\n    return n * 2\n}\n"
  "private funct p() {\n    return \"p\"\n}\n"
  "async funct t(n) {\n    return n\n}\n"
  "post(d(4), p(), gather(t(1))[0])\n", "8 p 1", NULL, 0 },
{ "funct como lambda",
  "f = funct(x) {\n    return x * 10\n}\npost(f(5))\n", "50", NULL, 0 },
/* Em POSICAO DE EXPRESSAO o que vem nao e declaracao de valor, e funcao sem
 * nome — entao os modificadores valem ali tambem. So o `funct(` pelado era
 * lambda: qualquer modificador na frente jogava a linha no caminho da
 * declaracao, que exige nome, e saia `esperado nome de funct`. Quem decide e a
 * CABECA terminando em `funct` seguida de `(`; se nao terminar assim o bloco
 * nao dispara, e por isso `int x = 1` continua declaracao. */
{ "lambda com async: devolve future",
  "g = async funct(x) {\n    return x * 2\n}\npost(await g(2))\n", "4", NULL, 0 },
{ "lambda com tipo de retorno: o sentinela 500 vale",
  "h = int funct(x) {\n    return x / 0\n}\npost(h(9))\n", "500", NULL, 0 },
{ "lambda com nonnull recusa Null",
  "i = nonnull funct(x) {\n    return x\n}\ni(Null)\n",
  "", "nonnull: parametro 'x' em '<funct>' nao pode ser Null", 1 },
{ "lambda com static compila e roda (a marca so vale em Entity)",
  "j = static funct(x) {\n    return x + 1\n}\npost(j(4))\n", "5", NULL, 0 },
{ "lambda com a cabeca inteira, em qualquer ordem",
  "k = private int async funct(x) {\n    return x\n}\npost(await k(6))\n", "6", NULL, 0 },
{ "a cabeca de lambda NAO mexe em declaracao tipada",
  "int x = 1\nstr s = \"a\"\nstring s2 = \"b\"\nlist l = [1]\npost(x, s, s2, l)\n",
  "1 a b [1]", NULL, 0 },
{ "post de uma funct imprime <funct #N>",
  "funct f() {\n    return 1\n}\npost(f)\n", "<funct #1>", NULL, 0 },
{ "static colado: chamavel pela Entity",
  "Entity Mat() {\n    static funct soma(a, b) {\n        return a + b\n    }\n}\n"
  "post(Mat.soma(1, 2))\n", "3", NULL, 0 },
{ "static colado: pela instancia e erro, e a mensagem fala em static",
  "Entity Mat() {\n    static funct soma(a, b) {\n        return a + b\n    }\n}\n"
  "m = Mat()\nm.soma(1, 2)\n",
  "", "RuntimeError: funct 'soma' e static: chame pela Entity (Tipo.soma(...)), nao pela instancia", 1 },
{ "nonnull colado: valor passa, Null nao",
  "nonnull funct e(v) {\n    return v\n}\npost(e(5))\ne(Null)\n",
  "5", "nonnull: parametro 'v' em 'e' nao pode ser Null", 1 },
{ "modificadores colados em qualquer ordem: private static int funct",
  "Entity K() {\n    private static int funct tres() {\n        return 3\n    }\n}\n"
  "post(K.tres())\n", "3", NULL, 0 },
{ "modificadores colados em qualquer ordem: static nonnull funct",
  "Entity K() {\n    static nonnull funct eco(x) {\n        return x\n    }\n}\n"
  "post(K.eco(\"ok\"))\n", "ok", NULL, 0 },
{ "modificadores colados em qualquer ordem: nonnull async funct",
  "nonnull async funct f(v) {\n    return v\n}\npost(gather(f(7))[0])\n", "7", NULL, 0 },
{ "static colado tambem vale fora de Entity (marca inofensiva)",
  "static funct f() {\n    return 1\n}\npost(f())\n", "1", NULL, 0 },
{ "NonNull colado, na grafia do decorador antigo",
  "NonNull funct e(v) {\n    return v\n}\ne(Null)\n",
  "", "nonnull: parametro 'v' em 'e' nao pode ser Null", 1 },
/* ── a ordem dos modificadores e LIVRE, inclusive dentro da Entity ─────────
 * `public int static funct r(n)` dentro de Entity compilava e a funct saia SEM
 * static, SEM tipo e SEM public: o despacho do corpo da classe tinha uma copia
 * a mao da cabeca, que nao conhecia os modificadores colados, e `int static`
 * casava com a regra de CAMPO — nascia um campo chamado `static`. Na tela dele:
 * `static` escrito e "Entity 'asterisco' nao tem metodo estatico 'randint'".
 * As 24 permutacoes de {public, int, static, async} foram medidas; aqui ficam
 * as que cobrem cada posicao do que quebrava. */
{ "Entity: `public int static funct` (a ordem dele) e chamavel pelo tipo",
  "Entity A() {\n    public int static funct r(n) {\n        return n\n    }\n}\npost(A.r(2))\n",
  "2", NULL, 0 },
{ "Entity: visibilidade DEPOIS do modificador (`static public funct`)",
  "Entity A() {\n    static public funct b(n) {\n        return n\n    }\n}\npost(A.b(3))\n",
  "3", NULL, 0 },
{ "Entity: `int static funct` sem visibilidade nenhuma",
  "Entity A() {\n    int static funct d(n) {\n        return n\n    }\n}\npost(A.d(4))\n",
  "4", NULL, 0 },
{ "Entity: `static private funct` mantem o private lido na cabeca",
  "Entity A() {\n    static private funct p(n) {\n        return n\n    }\n}\nx = A()\nx.p(1)\n",
  "", "acesso negado: 'p' e private de A (so acessivel de dentro da classe)", 1 },
{ "Entity: campo `int y = 2` continua campo ao lado de funct com modificador",
  "Entity A() {\n    static funct m(n) {\n        return n\n    }\n    int y = 2\n}\n"
  "post(A.m(1), A(2).y)\n", "1 2", NULL, 0 },
{ "topo: a ordem tambem e livre fora de Entity",
  "static private int funct f(n) {\n    return n\n}\npost(f(9))\n", "9", NULL, 0 },
{ "`static` e `nonnull` NAO viraram palavra reservada",
  "static = 7\nnonnull = 8\npost(static + nonnull)\n", "15", NULL, 0 },
/* ── `long`: inteiro de QUALQUER tamanho ────────────────────────────────────
 * A VM promove pra bignum sozinha quando estoura 64 bits, e `type()` do
 * resultado responde `"int"` — mas `int x = <bignum>` RECUSAVA o mesmo valor.
 * O motor se contradizia: nomeava de um jeito e testava de outro, porque quem
 * nomeia e quem testa eram dois codigos diferentes.
 *
 * `long` e a declaracao que nao promete 64 bits, como em C. `int` continua
 * prometendo — e por isso continua recusando, agora com a mensagem dizendo
 * qual e a palavra. Mesma ideia do `char`, que tambem e restricao de
 * DECLARACAO: `type()` dos dois responde o tipo do VALOR, nao o da promessa. */
{ "long aceita bignum, int nao",
  "long a = 99999999999999999999999999\npost(a)\n",
  "99999999999999999999999999", NULL, 0 },
{ "long aceita int pequeno tambem (alarga, como em C)",
  "long b = 42\npost(b)\n", "42", NULL, 0 },
{ "long recebe o resultado que estourou 64 bits",
  "x = 1103515245 * 99999999999999999\nlong c = x\npost(c)\n",
  "110351524499999998896484755", NULL, 0 },
{ "int com bignum diz que nao cabe e aponta o long",
  "x = 1103515245 * 99999999999999999\nint y = x\n",
  "", "esperava int, e o valor nao cabe em 64 bits (declare como 'long y'", 1 },
{ "long e CHAMAVEL, e converte igual ao int",
  "post(long(\"123\"), long(3.9), long(), type(long(\"1\")))\n", "123 3 0 int", NULL, 0 },
{ "long(bignum) devolve o bignum intacto",
  "b = 99999999999999999999999999\npost(long(b))\n",
  "99999999999999999999999999", NULL, 0 },
{ "`is long` vale pros dois tamanhos, e so pra inteiro",
  "b = 99999999999999999999999999\npost(b is long, 5 is long, \"a\" is long, 1.5 is long)\n",
  "True True False False", NULL, 0 },
{ "long recusa o que nao e inteiro",
  "long d = \"texto\"\n", "", "variável d esperava long", 1 },
{ "type() de bignum continua int — o valor e inteiro",
  "b = 99999999999999999999999999\nlong a = b\npost(type(a), type(b))\n", "int int", NULL, 0 },

/* ── `action` e `reaction` SAIRAM ──────────────────────────────────────────
 * Nao sao mais palavra reservada. Sem uma recusa com nome, `action f() {`
 * viraria nome solto + chamada + literal de dicionario ("faltou ':' no
 * dicionario"), ou pior, um NameError em tempo de execucao. Sao QUATRO as
 * posicoes onde a grafia morta pode aparecer, e cada uma tem a sua mensagem. */
{ "grafia morta: funcao solta",
  "action g() {\n    return 1\n}\n",
  "", "'action' saiu da linguagem; a funcao se declara com 'funct': funct g(args) { ... }", 2 },
{ "grafia morta: metodo de Entity",
  "Entity A() {\n    action m(self) {\n        return 1\n    }\n}\n",
  "", "'action' saiu da linguagem; o metodo se declara com 'funct': funct m(self) { ... }", 2 },
{ "grafia morta: lambda",
  "x = action(y) {\n    return y\n}\n",
  "", "'action' saiu da linguagem; a funcao sem nome se escreve 'funct(args) { ... }'", 2 },
{ "grafia morta: cabeca com modificadores",
  "int async reaction h(a) {\n    return a\n}\n",
  "", "'reaction' saiu da linguagem; troque por 'funct': int async funct h(...)", 2 },
{ "`action` como NOME comum continua valendo (nao e mais reservada)",
  "action = 5\nreaction = 2\npost(action + reaction)\n", "7", NULL, 0 },
{ "@static continua valendo junto com o modificador colado",
  "Entity M() {\n    @static\n    funct velha(a) {\n        return a\n    }\n"
  "    static funct nova(a) {\n        return a\n    }\n}\n"
  "post(M.velha(1), M.nova(2))\n", "1 2", NULL, 0 },
{ "@NonNull continua valendo",
  "@NonNull\nfunct f(v) {\n    return v\n}\nf(Null)\n",
  "", "nonnull: parametro 'v' em 'f' nao pode ser Null", 1 },
{ "funct dentro de decorador de rota (a cabeca com decorador conhece funct)",
  "import jinker\nfrom jinker import Jinker\n"
  "Object app = Jinker(__name__)\n"
  "@app.get(\"/x\")\nint async funct h() {\n    return 1\n}\n"
  "post(\"registrou\")\n", "registrou", NULL, 0 },
{ "faltou funct: a mensagem diz a palavra nova",
  "int async LoginHandler(data) {\n    return 1\n}\n",
  "", "faltou 'funct' antes de 'LoginHandler': int async funct LoginHandler(...)", 2 },
/* `static` valia pra funct DE DENTRO tambem: a pendencia do compilador nao era
 * consumida, entao toda funct declarada no corpo herdava a marca — e se o
 * primeiro parametro dela se chamasse `self`, ele era descartado, com um
 * "takes 0 positional arguments" sem relacao visivel com o `static` de fora. */
{ "static nao vaza pra funct aninhada (decorador)",
  "@static\nfunct outer(a) {\n    funct inner(self) {\n        return self\n    }\n"
  "    return inner(a)\n}\npost(outer(7))\n", "7", NULL, 0 },
{ "static nao vaza pra funct aninhada (modificador colado)",
  "static funct outer(a) {\n    funct inner(self) {\n        return self\n    }\n"
  "    return inner(a)\n}\npost(outer(7))\n", "7", NULL, 0 },
{ "nonnull nao vaza pra funct aninhada",
  "nonnull funct outer(a) {\n    funct inner(x) {\n        return x\n    }\n"
  "    return inner(Null)\n}\npost(outer(1))\n", "Null", NULL, 0 },
{ "'def' manda escrever funct",
  "def f(a) {\n    return a\n}\n", "", "a funcao se declara com 'funct'", 2 },
/* ── tipo de retorno: TODO tipo vale, 2026-09-08 ───────────────────────────
 * Era uma lista branca de dois (`int`/`bool`), com `str`/`flo` aceitos calados
 * e o resto recusado. Uma lista que aceita `dict` e recusa `list` e arbitraria;
 * agora o que decide e a POSICAO — o que vem colado antes do `funct` e o tipo
 * de retorno, incluindo nome de classe. So `int` e `bool` mudam o
 * comportamento; os outros declaram e nao coagem. */
{ "char funct devolve o valor sem coagir",
  "char funct f() {\n    return \"a\"\n}\npost(f())\n", "a", NULL, 0 },
{ "list funct vale como retorno",
  "list funct f() {\n    return [1, 2]\n}\npost(f())\n", "[1, 2]", NULL, 0 },
{ "nome de classe vale como tipo de retorno",
  "class P() {\n    funct __init__(self, n) {\n        self.n = n\n    }\n}\n"
  "P funct cria(n) {\n    return P(n)\n}\npost(cria(7).n)\n", "7", NULL, 0 },
/* O apelido resolve pro canonico: `string` E `str`. */
{ "apelido de tipo vale como retorno",
  "string funct f() {\n    return \"oi\"\n}\npost(f())\n", "oi", NULL, 0 },
/* ── o apelido na CABECA dentro de classe, 2026-09-08 ──────────────────────
 * `public static string funct main()` nao era reconhecido como cabeca de
 * funct: casava com a regra de campo e nascia um CAMPO chamado `string`, do
 * tipo `static`. O metodo sumia da classe e o arquivo rodava sem erro nenhum e
 * sem fazer nada — foi assim que ele apareceu. */
{ "apelido de tipo na cabeca dentro de classe nao vira campo",
  "public class C() {\n    public static string funct main() {\n        post(\"rodou\")\n    }\n}\n"
  "C.main()\n", "rodou", NULL, 0 },
{ "static colado registra o metodo como estatico",
  "class C() {\n    static funct m() {\n        post(\"ok\")\n    }\n}\nC.m()\n", "ok", NULL, 0 },
{ "static continua valendo depois do tipo",
  "class C() {\n    str static funct m() {\n        post(\"ok\")\n    }\n}\nC.m()\n", "ok", NULL, 0 },

/* ── campo `static`, 2026-09-08 ────────────────────────────────────────────
 * Campo de classe: avaliado UMA vez na declaracao, lido como `Classe.x`, por
 * nome solto dentro do corpo e dos metodos da propria classe, e por
 * `self.x` na instancia. Sem ele, `App.mapp` nao existia e um metodo static
 * nao tinha como enxergar o campo — o arquivo dele rodava sem fazer nada. */
{ "campo static: Classe.x e nome solto em metodo static",
  "class C(){\n    static int x = 7\n    static funct le(){ return x }\n}\npost(C.x, C.le())\n", "7 7", NULL, 0 },
{ "campo static: modificadores em qualquer ordem",
  "class C(){\n    static public int x = 7\n    private static int y = 8\n}\npost(C.x, C.y)\n", "7 8", NULL, 0 },
{ "campo static sem inicializador nasce Null",
  "class C(){\n    static int x\n}\npost(C.x)\n", "Null", NULL, 0 },
{ "campo static e UM so: instancias leem e a reescrita e da classe",
  "class K(){\n    static int n = 0\n    funct inc(self){ K.n = K.n + 1 }\n}\na = K()\nb = K()\na.inc()\nb.inc()\npost(K.n, a.n, b.n)\n", "2 2 2", NULL, 0 },
/* Parametro com o mesmo nome GANHA do campo static; o herdado le-se por
 * `self.`/`Classe.` (o nome solto cobre os static da propria classe). */
{ "campo static: local ganha, heranca por self. e Classe.",
  "class B(){\n    static int fundo = 1\n}\nclass C(B){\n    static int x = 7\n"
  "    funct m(self, x){ return x }\n    funct s(self){ return self.x + self.fundo }\n"
  "    static funct t(){ return x + C.fundo }\n}\nc = C()\npost(c.m(99), c.s(), C.fundo, C.t())\n",
  "99 8 1 8", NULL, 0 },
{ "campo static nao entra no __init__ sintetizado",
  "class P(){\n    static int total = 0\n    str nome\n    int idade = 3\n}\np = P(\"ana\")\npost(p.nome, p.idade, P.total)\n",
  "ana 3 0", NULL, 0 },
/* ── decorador `@obj.metodo()` DENTRO da classe, 2026-09-08 ────────────────
 * Era descartado pelo compilador: o metodo compilava sem registro nenhum e a
 * rota nunca existia, calada. Agora vale nas tres posicoes — funct solta, em
 * cima da classe e em cima do metodo — pelo MESMO protocolo (register). */
{ "decorador @obj.m() nas tres posicoes: funct, classe, metodo",
  "class Reg(){\n    funct __init__(self){ self.v = [] }\n"
  "    funct rota(self, c){ self.c = c\n        return self }\n"
  "    funct register(self, h){ addEnd(self.v, self.c)\n        return h }\n}\nr = Reg()\n"
  "@r.rota(\"/funct\")\nfunct f(){ return 1 }\n"
  "@r.rota(\"/classe\")\nclass H(){\n    funct handler(self){ return 1 }\n}\n"
  "class D(){\n    @r.rota(\"/dentro\")\n    static funct h(){ return 1 }\n"
  "    @r.rota(\"/inst\")\n    funct i(self){ return 1 }\n}\npost(r.v)\n",
  "['/funct', '/classe', '/dentro', '/inst']", NULL, 0 },
/* O arquivo dele: campo static + decorador no corpo usando o campo. */
{ "decorador no corpo da classe le campo static da propria classe",
  "class Reg(){\n    funct __init__(self){ self.v = [] }\n"
  "    funct rota(self, c){ self.c = c\n        return self }\n"
  "    funct register(self, h){ addEnd(self.v, self.c)\n        return h }\n}\n"
  "public class App(){\n    public static object mapp = Reg()\n"
  "    @mapp.rota(\"/opa\")\n    public string static funct handler(data){ return \"ok\" }\n}\n"
  "post(App.mapp.v)\n",
  "['/opa']", NULL, 0 },
/* Sem o `static` o decorador nao tem como ler o campo — e a mensagem diz o
 * que falta, em vez de um NameError apontando pra linha da classe. */
{ "decorador no corpo lendo campo de INSTANCIA e recusado com a palavra certa",
  "class Reg(){\n    funct rota(self, c){ return self }\n}\n"
  "class App(){\n    object mapp = Reg()\n    @mapp.rota(\"/x\")\n    static funct h(){ return 1 }\n}\n",
  "", "campo de instancia", 2 },
/* A ordem dos modificadores do CAMPO e de quem escreve, como na cabeca de
 * funct. `private object static nome` era lido com `static` como o NOME do
 * campo: virava campo de instancia chamado `static`, calado, e `C.nome` nao
 * existia. As dez ordens abaixo tem que dar o mesmo. */
{ "campo static: modificador antes OU depois do tipo, qualquer ordem",
  "class C(){\n"
  "    static object a = 1\n"
  "    private static object b = 2\n"
  "    static private object c = 3\n"
  "    private object static d = 4\n"
  "    object static e = 5\n"
  "    object private static f = 6\n"
  "    static object private g = 7\n"
  "    object static private h = 8\n"
  "    static public i = 9\n"
  "}\npost(C.a, C.b, C.c, C.d, C.e, C.f, C.g, C.h, C.i)\n",
  "1 2 3 4 5 6 7 8 9", NULL, 0 },
{ "funct: modificador depois do tipo continua valendo",
  "class C(){\n    object private static funct m(){ return 1 }\n}\npost(C.m())\n", "1", NULL, 0 },

/* ── import por caminho entre aspas (`import '../x.ps'`), 2026-09-06 ───────
 * A string e o especificador, como no TypeScript: com `/` ou extensao da
 * linguagem e caminho relativo ao arquivo que importa; sem isso e nome de
 * modulo (motor ou lib instalada). Os casos com arquivo de verdade estao em
 * teste/cobre_stdlib.ps; aqui, o que nao precisa de arquivo. */
{ "import por caminho inexistente cita o caminho como foi escrito",
  "import './nao_existe.ps'\n", "", "ImportError: No module named './nao_existe.ps'", 1 },
{ "import 'nome' sem barra nem extensao e nome de modulo, nao caminho",
  "import 'pasta_que_nao_existe'\n", "", "ImportError: No module named 'pasta_que_nao_existe'", 1 },
{ "import por caminho cujo arquivo nao serve de nome de variavel exige `as`",
  "import 'sub/meu-mod.ps'\n", "", "'meu-mod' nao serve de nome de variavel: ligue com `as`", 2 },
{ "import entre aspas vazio e erro de sintaxe",
  "import ''\n", "", "import entre aspas vazio", 2 },
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
  "funct f(x) {\n"
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
/* `copy()` num PoolFile vindo de `os.loadFile` (binário). O caminho inteiro do
 * copy() nunca tinha sido exercitado por caso nenhum da suíte (gcov: linha
 * ##### na função). Desde 2026-09-09 o mesmo `copy()` vale no PoolFile aberto
 * por `open()` — ver "TODO ARQUIVO É PoolFile", abaixo. */
{ "PoolFile.copy() copia o conteudo",
  "import os\n"
  "using open(\"o.png\", \"wb\") as f { f.write(\"\\x89PNG\\r\\n\\x1a\\n\\x00\\x00\\x00\\rIHDR\") }\n"
  "a = os.loadFile(\"o.png\")\n"
  "a.copy(\"d.png\")\n"
  "using open(\"d.png\", \"rb\") as f { post(len(f.read())) }\n", "17", NULL, 0 },

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

/* ── PARAMETRO TIPADO: o tipo vem ANTES do nome, 2026-09-09 ────────────────
 * Ordem do dono: "o tipo em args o tipo sempre primeiro que a variavel, igual
 * no java". Antes NENHUMA forma passava: `str corpo` dava "'str' e palavra
 * reservada", `String corpo` dava "faltou ')'", `corpo: str` idem. O tipo so
 * CHECA — argumento errado e recusado, nunca convertido (mesma regra da
 * declaracao de variavel). A checagem mora num lugar so na VM e e chamada de
 * TODO ponto que monta frame: chamada direta, metodo, static na Entity,
 * argumento nomeado, lambda, gerador e async. */
{ "parametro tipado: o tipo vem antes do nome",
  "funct saudacao(str nome, int vezes) { return nome * vezes }\n"
  "post(saudacao(\"oi \", 3))\n", "oi oi oi ", NULL, 0 },
{ "parametro tipado recusa o tipo errado — nao converte",
  "funct saudacao(str nome, int vezes) { return nome * vezes }\n"
  "post(saudacao(5, 3))\n", "",
  "AttributedValueError: parâmetro nome de saudacao() esperava str, recebeu int", 1 },
{ "apelido do tipo vale no parametro (String e str, Integer e int)",
  "funct f(String s, Integer n) { return s * n }\npost(f(\"a\", 2))\n",
  "aa", NULL, 0 },
{ "int NAO aceita flo no parametro",
  "funct f(int n) { post(n) }\nf(1.5)\n", "",
  "AttributedValueError: parâmetro n de f() esperava int, recebeu flo", 1 },
{ "flo NAO aceita int no parametro",
  "funct f(flo x) { post(x) }\nf(1)\n", "",
  "AttributedValueError: parâmetro x de f() esperava flo, recebeu int", 1 },
{ "bool NAO aceita int no parametro",
  "funct f(bool b) { post(b) }\nf(1)\n", "",
  "AttributedValueError: parâmetro b de f() esperava bool, recebeu int", 1 },
{ "tipar e opcional e por parametro — pode misturar",
  "funct mist(str a, b, int c) { post(a, b, c) }\nmist(\"a\", [1], 2)\n",
  "a [1] 2", NULL, 0 },
{ "o tipo vale no argumento NOMEADO",
  "funct f(str nome, int vezes) { return nome * vezes }\n"
  "post(f(vezes=2, nome=\"ei \"))\n", "ei ei ", NULL, 0 },
{ "argumento nomeado de tipo errado e recusado",
  "funct f(str nome, int vezes) { return nome * vezes }\n"
  "post(f(vezes=\"x\", nome=\"ei \"))\n", "",
  "AttributedValueError: parâmetro vezes de f() esperava int, recebeu str", 1 },
{ "valor padrao convive com o tipo",
  "funct pad(str a, int n = 2) { return a * n }\npost(pad(\"x\"), pad(\"x\", 3))\n",
  "xx xxx", NULL, 0 },
{ "o tipo vale no METODO da Entity",
  "Entity P() {\n  str nome\n  public str funct diz(self, str saud) { return saud + self.nome }\n}\n"
  "P p = P(\"Ana\")\npost(p.diz(\"ola \"))\np.diz(9)\n",
  "ola Ana",
  "AttributedValueError: parâmetro saud de diz() esperava str, recebeu int", 1 },
{ "o tipo vale no metodo static chamado na Entity",
  "Entity P() {\n  str nome\n  @static\n  public static funct cria(str nome) { return P(nome) }\n}\n"
  "post(P.cria(\"Ana\").nome)\nP.cria(1)\n", "Ana",
  "AttributedValueError: parâmetro nome de cria() esperava str, recebeu int", 1 },
{ "o tipo vale na lambda",
  "g = funct(int n) { return n + 1 }\npost(g(2))\ng(\"x\")\n", "3",
  "AttributedValueError: parâmetro n de <funct>() esperava int, recebeu str", 1 },
{ "o tipo vale no gerador",
  "funct gera(int n) { for each i in range(n) { yield i } }\n"
  "post(list(gera(3)))\npost(list(gera(\"x\")))\n", "[0, 1, 2]",
  "AttributedValueError: parâmetro n de gera() esperava int, recebeu str", 1 },
{ "o tipo vale na async funct",
  "async funct af(str s) { post(s) }\ngather([af(1)])\n", "",
  "AttributedValueError: parâmetro s de af() esperava str, recebeu int", 1 },
{ "nome de Entity serve de tipo, e subclasse passa (como em Java)",
  "Entity Animal() { str nome }\nEntity Cachorro(Animal) { }\n"
  "funct fala(Animal a) { post(a.nome) }\nfala(Cachorro(\"Rex\"))\n",
  "Rex", NULL, 0 },
{ "Entity errada no lugar de outra e recusada",
  "Entity Animal() { str nome }\nEntity Carro() { str nome }\n"
  "funct fala(Animal a) { post(a.nome) }\nfala(Carro(\"Fusca\"))\n", "",
  "AttributedValueError: parâmetro a de fala() esperava Animal, recebeu Carro", 1 },
{ "PoolFile serve de tipo de parametro — o handle do open() E PoolFile",
  "funct pega(PoolFile f) { post(type(f)) }\n"
  "using open(\"/tmp/ps_param_tipo.txt\", \"w\") as f { pega(f) }\n",
  "PoolFile", NULL, 0 },
{ "char no parametro pede UM caractere",
  "funct f(char c) { post(c) }\nf(\"a\")\nf(\"ab\")\n", "a",
  "AttributedValueError: parâmetro c de f() esperava char, recebeu str", 1 },
{ "list/dict/tup no parametro conferem o container certo",
  "funct lst(list xs, dict d, tup t) { post(len(xs), len(d), len(t)) }\n"
  "lst([1], {\"a\": 1}, (1, 2))\nlst([1], {\"a\": 1}, [1, 2])\n", "1 1 2",
  "AttributedValueError: parâmetro t de lst() esperava tup, recebeu list", 1 },
/* A ordem invertida dava "faltou ')' na declaracao da funct" — mensagem que
 * fala de um parentese que ninguem esqueceu. Quem escreve `x: int` inverteu a
 * ordem, e o erro tem que dizer isso. */
{ "`x: tipo` NAO existe no parametro — e o erro diz a ordem certa",
  "funct f(x: int) { post(x) }\n", "",
  "no parametro o tipo vem ANTES do nome: escreva `funct f(int x)`", 2 },
{ "a mesma inversao na lambda tambem e apontada",
  "g = funct(x: int) { return x }\n", "",
  "no parametro o tipo vem ANTES do nome: escreva `funct(int x)`", 2 },
{ "parentese que falta de verdade continua dizendo que falta",
  "funct f(a\n", "", "faltou ')' na declaracao da funct", 2 },
{ "parametro sem tipo continua valendo",
  "funct f(a, b = 10) { return a + b }\npost(f(5), f(5, 1))\n", "15 6", NULL, 0 },

/* `f is PoolFile` seguia falso pro handle do open(): a tabela de tipos so
 * aceitava OBJ_POOLFILE, contra a regra "todo arquivo e PoolFile" — o mesmo
 * `type()` ja respondia "PoolFile" pros dois. */
{ "open() e PoolFile tambem pro `is` e pra declaracao tipada",
  "using open(\"/tmp/ps_param_tipo2.txt\", \"w\") as f {\n"
  "    post(f is PoolFile)\n"
  "    PoolFile g = f\n"
  "    post(type(g))\n"
  "}\n", "True\nPoolFile", NULL, 0 },

/* ── HIERARQUIA DE EXCECOES, 2026-09-11 ────────────────────────────────────
 * O tipo da excecao era uma STRING solta, digitada a mao em cada um dos ~1000
 * sitios de erro, e o `catch` comparava os dois nomes por igualdade literal —
 * o comentario do proprio motor admitia: "Aqui nao ha hierarquia, o catch
 * compara o nome". Medido antes do conserto:
 *     catch (Exception e)  nao pegava NADA — nem erro de arquivo, nem
 *                          `1 + "a"`, nem um `raise ValueError` explicito
 *     catch (OSError e)    nao pegava FileNotFoundError
 * e o motor emite esses nomes de subclasse DE PROPOSITO (`tipo_do_errno`).
 * Agora a tabela EXCECOES[] diz o pai de cada tipo num lugar so, e o `catch`
 * compila pra OP_EXC_CASA, que sobe a cadeia. */
{ "catch (Exception e) pega qualquer erro",
  "try { post(1 + \"a\") }\ncatch (Exception e) { post(\"pegou\") }\n",
  "pegou", NULL, 0 },
{ "catch (Exception e) pega raise explicito",
  "try { raise ValueError(\"x\") }\ncatch (Exception e) { post(\"pegou\") }\n",
  "pegou", NULL, 0 },
{ "catch (Exception e) pega erro de sistema",
  "import os\ntry { os.loadFile(\"/nao/existe/xyz.txt\") }\ncatch (Exception e) { post(\"pegou\") }\n",
  "pegou", NULL, 0 },
{ "OSError pega FileNotFoundError — o motor emite a subclasse de proposito",
  "import os\ntry { os.loadFile(\"/nao/existe/xyz.txt\") }\ncatch (OSError e) { post(\"pegou\") }\n",
  "pegou", NULL, 0 },
{ "LookupError pega IndexError",
  "try { post([1][9]) }\ncatch (LookupError e) { post(\"pegou\") }\n", "pegou", NULL, 0 },
{ "LookupError pega KeyError",
  "try { post({\"a\": 1}[\"z\"]) }\ncatch (LookupError e) { post(\"pegou\") }\n", "pegou", NULL, 0 },
{ "ArithmeticError pega ZeroDivisionError",
  "try { post(1 / 0) }\ncatch (ArithmeticError e) { post(\"pegou\") }\n", "pegou", NULL, 0 },
{ "ValueError pega AttributedValueError da declaracao tipada",
  "try { int x = \"7\" }\ncatch (ValueError e) { post(\"pegou\") }\n", "pegou", NULL, 0 },
{ "RuntimeError pega RecursionError",
  "funct f() { return f() }\ntry { f() }\ncatch (RuntimeError e) { post(\"pegou\") }\n",
  "pegou", NULL, 0 },
/* A hierarquia so ACRESCENTA: quem nao e da linhagem continua nao pegando. */
{ "TypeError NAO pega ValueError",
  "try { raise ValueError(\"x\") }\ncatch (TypeError e) { post(\"NAO DEVIA\") }\n", "",
  "ValueError: x", 1 },
{ "OSError NAO pega TypeError",
  "try { post(1 + \"a\") }\ncatch (OSError e) { post(\"NAO DEVIA\") }\n", "",
  "TypeError: unsupported operand type(s) for +", 1 },
{ "IOError segue IRMAO de OSError, nao pai — a doc do open() promete isso",
  "import os\ntry { os.loadFile(\"/nao/existe/xyz.txt\") }\ncatch (IOError e) { post(\"NAO DEVIA\") }\n",
  "", "FileNotFoundError:", 1 },
/* Um inventario de 14 agentes cruzou os tipos que o motor EMITE contra a
 * tabela e achou 15 de fora. `NotImplementedError` e `TimeoutError` eram da
 * propria linguagem e escapavam do `catch (Exception e)`; os outros 13 sao
 * nomes de erro do postgres, agora todos com `DatabaseError` de pai.
 *
 * SEM CASO AQUI, e a razao: `NotImplementedError` so nasce ao IMPORTAR um
 * modulo que nao compila, e o runner nao consegue importar — ele roda o caso
 * num tmpfile sem extensao, e a busca de modulo olha a pasta do SCRIPT, nao o
 * cwd (medido: o `.ps` escrito esta la, `os.isfile` responde True, e o import
 * ainda diz "No module named"). Conferido a mao, fora do runner:
 *     try { import <modulo que nao compila> } catch (Exception e) { ... }
 * pega, e com RuntimeError tambem. `TimeoutError` precisa de rede lenta. */
{ "catch do tipo EXATO continua pegando, e o catch sem tipo tambem",
  "import os\ntry { os.loadFile(\"/nao/x\") } catch (FileNotFoundError e) { post(\"exato\") }\n"
  "try { os.loadFile(\"/nao/x\") } catch (e) { post(\"sem tipo\") }\n",
  "exato\nsem tipo", NULL, 0 },

/* ── loadFile NAO TEM LISTA DE EXTENSAO, 2026-09-11 ─────────────────────────
 * Havia um vetor com 12 extensoes binarias, e pedir `mode='rb'` fora dela era
 * recusado: "loadFile: mode='rb' nao aceita extensao '.xlsm'". `.xlsm`,
 * `.7z`, `.tar`, `.wav` e arquivo SEM extensao nao abriam. Uma linguagem nao
 * tem lista de arquivos que pode abrir. Agora o modo manda, e sem modo quem
 * decide e o conteudo (byte NUL nos primeiros 8 KB). */
{ "mode='rb' abre extensao que nao esta em lista nenhuma",
  "import os\nusing open(\"a.xlsm\", \"w\") as f { f.write(\"planilha\") }\n"
  "a = os.loadFile(\"a.xlsm\", mode=\"rb\")\npost(type(a), a.ext)\n",
  "PoolFile .xlsm", NULL, 0 },
{ "mode='rb' abre arquivo SEM extensao",
  "import os\nusing open(\"semext\", \"w\") as f { f.write(\"conteudo\") }\n"
  "a = os.loadFile(\"semext\", mode=\"rb\")\npost(type(a))\n",
  "PoolFile", NULL, 0 },
{ "mode='r' le como texto ate o que a lista chamava de binario",
  "import os\nusing open(\"z.png\", \"w\") as f { f.write(\"nao sou png\") }\n"
  "post(os.loadFile(\"z.png\", mode=\"r\"))\n",
  "nao sou png", NULL, 0 },
{ "sem mode, quem decide e o CONTEUDO: byte NUL e binario",
  "import os\nusing open(\"c1.dat\", \"wb\") as f { f.write(\"AB\\x00CD\") }\n"
  "using open(\"c2.dat\", \"w\") as f { f.write(\"texto puro\") }\n"
  "post(type(os.loadFile(\"c1.dat\")), type(os.loadFile(\"c2.dat\")))\n",
  "PoolFile str", NULL, 0 },
{ "mode que nao existe diz que o VALOR nao serve, e lista os que servem",
  "import os\nusing open(\"m.txt\", \"w\") as f { f.write(\"x\") }\n"
  "os.loadFile(\"m.txt\", mode=\"xyz\")\n", "",
  "use 'r' ou 'rb'", 1 },

/* ── CLI ── */
{ "--check não executa o script",
  "post(\"NAO DEVIA RODAR\")\n", "NAO DEVIA RODAR", NULL, 0 },
};
const int NC_LINGUAGEM = N_CASOS(CASOS_LINGUAGEM);
