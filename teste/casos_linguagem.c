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
  /* era `int funct` devolvendo a lista: com o tipo da funct valendo pro
   * `return`, a declaração que diz o que ela devolve é `list funct` */
  "list funct main(lista){\n"
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
  "Entity O {\n"
  "    x: int\n"
  "    y: int\n"
  "}\n"
  "o = O(0, 0)\n"
  "o.x, o.y = 5, 6\n"
  "post(o.x, o.y)\n",
  "5 6", NULL, 0 },
{ "alvo em cadeia: membro e depois indice",
  "Entity O {\n"
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
  "private Class Pagamento {\n"
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
   * pior que nao ter encapsulamento, porque parece que tem. Antes de rodar:
   * o valor e um A, e nenhuma classe do arquivo libera o nome. */
  "Class A {\n"
  "    public funct __init__(self, nome) {\n"
  "        private str name = nome\n"
  "    }\n"
  "}\n"
  "post(A(\"ana\").name)\n",
  "", "acesso negado: 'name' e private de A", 2 },
{ "public declarado no construtor NAO barra",
  "Class A {\n"
  "    public funct __init__(self, nome) {\n"
  "        public str name = nome\n"
  "    }\n"
  "}\n"
  "post(A(\"ana\").name)\n",
  "ana", NULL, 0 },
{ "o tipo do campo e conferido como o da variavel",
  "Class A {\n"
  "    public funct __init__(self) {\n"
  "        private int n = 5.9\n"
  "    }\n"
  "}\n"
  "a = A()\n",
  "", "AttributedValueError: campo n de A esperava int, recebeu flo", 2 },
{ "tipo nao escalar guarda sem conferir, como na variavel",
  "Class A {\n"
  "    public funct __init__(self) {\n"
  "        public list itens = [1, 2]\n"
  "    }\n"
  "    public funct ver(self) { return self.itens }\n"
  "}\n"
  "post(A().ver())\n",
  "[1, 2]", NULL, 0 },
{ "a mesma declaracao vale NO CORPO da classe",
  "Class A {\n"
  "    str x = \"a\"\n"
  "    public funct ver(self) { return self.x }\n"
  "}\n"
  "post(A().ver())\n",
  "a", NULL, 0 },
{ "no corpo da classe, sem default, vira parametro do construtor",
  "Class A {\n"
  "    str x\n"
  "    public funct ver(self) { return self.x }\n"
  "}\n"
  "post(A(\"oi\").ver())\n",
  "oi", NULL, 0 },
{ "private no corpo da classe pela forma nova tambem barra",
  "Class A {\n"
  "    private str x = \"a\"\n"
  "}\n"
  "post(A().x)\n",
  "", "acesso negado: 'x' e private de A", 2 },
{ "int funct dentro da Entity continua sendo funct, nao campo",
  /* A condicao que separa `int funct f()` de `str x = 1` no corpo da
   * classe: depois do tipo de RETORNO vem sempre outra palavra da
   * linguagem. */
  "Class A {\n"
  "    int funct f(self) { return 7 }\n"
  "    public async int funct g(self) { return 8 }\n"
  "}\n"
  "a = A()\n"
  "post(a.f(), await a.g())\n",
  "7 8", NULL, 0 },
{ "a ordem 'nome: tipo' dentro da funct diz o conserto",
  /* Era `SyntaxError: expressao invalida` apontando pro ':'. */
  "Class A {\n"
  "    public funct __init__(self, nome) {\n"
  "        private name: str = nome\n"
  "    }\n"
  "}\n",
  "", "dentro de uma funct escreva 'private <tipo> name = <valor>'", 2 },
{ "private sem tipo nenhum nao passa mais calado",
  /* Antes: compilava, e `private` virava um nome inexistente em runtime. */
  "Class A {\n"
  "    public funct __init__(self, nome) {\n"
  "        private name = nome\n"
  "    }\n"
  "}\n",
  "", "'private name = ...' dentro de uma funct de Entity declara campo do objeto: escreva 'private <tipo> name = <valor>'", 2 },
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
  "private Class A {\n"
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
  "if True {\n"
  "    dentro = 5\n"
  "}\n"
  "post(dentro)\n", "", "name 'dentro' is not defined", 1 },
{ "atribuir nome de fora dentro do bloco altera o de fora",
  "x = 1\n"
  "if True {\n"
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
/* ── chave do literal de dict é EXPRESSÃO (regra 8 da tipagem estática) ──
 * Nome sem aspas era caso especial do parser: virava a string "k", então
 * `{tls: true}` e `{"tls": true}` eram iguais e `{k + "x": 1}` era
 * SyntaxError ("faltou ':' no dicionario"). Agora a chave segue a regra do
 * índice (`d[k] = v`): nome é variável, texto vai com aspas, e nome que o
 * arquivo não liga é NameError antes de rodar. */
{ "dict: chave sem aspas é a variável, não o texto",
  "k = \"a\"\npost({k: 1})\n",
  "{'a': 1}", NULL, 0 },
{ "dict: chave com conta e com chamada",
  "k = \"a\"\nfunct f() {\n    return \"z\"\n}\npost({k + \"x\": 2, f(): 1})\n",
  "{'ax': 2, 'z': 1}", NULL, 0 },
{ "dict: chave com aspas segue texto",
  "post({\"tls\": True})\n",
  "{'tls': True}", NULL, 0 },
{ "dict: chave nua que o arquivo não liga é NameError antes de rodar",
  "post({tls: True})\n",
  "", "NameError: name 'tls' is not defined", 2 },
{ "dict: chave nua que é nome de tipo segue o objeto tipo",
  "post({str: 1})\n",
  "{str: 1}", NULL, 0 },
{ "dict: aninhado com chaves de variável",
  "a = \"x\"\nb = \"y\"\npost({a: {b: 1}})\n",
  "{'x': {'y': 1}}", NULL, 0 },
{ "dict: número e bool como chave (true encontra 1)",
  "post({1: \"a\", 2.5: \"b\", True: \"c\"})\n",
  "{1: 'c', 2.5: 'b'}", NULL, 0 },
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
  "t = (1, 2, 3)\nt.append(9)\n",
  "", "AttributeError: 'tup' object has no method 'append'", 2 },
{ "tupla é imutável: sort",
  "t = (1, 2, 3)\nt.sort()\n",
  "", "AttributeError: 'tup' object has no method 'sort'", 2 },
{ "tupla é imutável: copy",
  "t = (1, 2, 3)\nt.copy()\n",
  "", "AttributeError: 'tup' object has no method 'copy'", 2 },
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
  /* `type(Null)` e literalmente "Null" */
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
  "os.writeFile(\"ps_teste_leitura.txt\", \"abcdef\")\n"
  "using open(\"ps_teste_leitura.txt\") as f {\n"
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
  "Entity C {\n"
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
{ "closures do laço guardam a variável da volta delas",
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
  "0 1 2", NULL, 0 },
{ "recursão mútua entre aninhadas",
  "funct mutuo(n) {\n"
  "    funct par(k) {\n"
  "        if k == 0 {\n"
  "            return True\n"
  "        }\n"
  "        return impar(k - 1)\n"
  "    }\n"
  "    funct impar(k) {\n"
  "        if k == 0 {\n"
  "            return False\n"
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
/* ── `async funct` roda QUANDO E CHAMADA ─────────────────────────────────
 *
 * Antes a fibra era criada e ninguem a executava: sem `await`/`gather`/`post`,
 * o corpo nao rodava NUNCA (medido: future criado, sleep(1) no principal,
 * nenhum passo). Dentro do jinker o laco iniciava a fibra orfa sozinho, entao
 * a mesma linha significava coisas diferentes conforme o programa. Agora a
 * chamada roda o corpo ate o primeiro ponto em que ele cede. */
{ "async funct roda ate o primeiro sleep na chamada",
  "async funct t() {\n"
  "    post(\"a\")\n"
  "    sleep(0.05)\n"
  "    post(\"b\")\n"
  "}\n"
  "f = t()\n"
  "post(\"chamou\")\n"
  "v = await f\n"
  "post(\"fim\")\n",
  "a\nchamou\nb\nfim", NULL, 0 },
{ "async funct sem ponto de cedencia roda inteira na chamada",
  "async funct t() {\n"
  "    post(\"corpo\")\n"
  "    return 7\n"
  "}\n"
  "f = t()\n"
  "post(\"depois\")\n"
  "post(await f)\n",
  "corpo\ndepois\n7", NULL, 0 },
{ "gather continua rodando as tarefas junto",
  "async funct t(n) {\n"
  "    sleep(0.2)\n"
  "    return n * 2\n"
  "}\n"
  "post(gather(t(1), t(2), t(3)))\n",
  "[2, 4, 6]", NULL, 0 },
/* ERRO E ERRO: a excecao que acontece ANTES do primeiro ponto de cedencia sobe
 * na hora, no ponto da chamada — e a linha acusada e a do `raise`, nao a do
 * `await` (o traceback vinha da fibra e era jogado fora). */
{ "erro antes do primeiro sleep sobe na propria chamada",
  "async funct t() {\n"
  "    raise ValueError(\"quebrei\")\n"
  "}\n"
  "post(\"antes\")\n"
  "f = t()\n"
  "post(\"nao chega aqui\")\n",
  "antes", "ValueError: quebrei", 1 },
/* ── a tarefa ANDA enquanto o principal espera ───────────────────────────
 *
 * Rodada 3 da Jinga. Num script, a tarefa que cedeu so voltava num `await`:
 * com o principal dormindo 1 s, a de 0,2 s nao terminava — e o mesmo codigo
 * terminava dentro do `jinker`, cujo laco roda as tarefas sozinho. Agora todo
 * ponto em que o principal ESPERA (`sleep`, I/O) roda as tarefas. O fim do
 * programa continua nao esperando ninguem. */
{ "tarefa anda enquanto o principal dorme",
  "async funct t() {\n"
  "    post(\"t antes\")\n"
  "    sleep(0.1)\n"
  "    post(\"t depois\")\n"
  "}\n"
  "f = t()\n"
  "post(\"voltei\")\n"
  "sleep(0.4)\n"
  "post(\"acordei\")\n",
  "t antes\nvoltei\nt depois\nacordei", NULL, 0 },
{ "tarefa anda enquanto o principal espera I/O",
  "import os\n"
  "async funct t() {\n"
  "    sleep(0.1)\n"
  "    post(\"tarefa\")\n"
  "}\n"
  "f = t()\n"
  "os.run([\"sleep\", \"0.4\"], capture=True)\n"
  "post(\"principal\")\n",
  "tarefa\nprincipal", NULL, 0 },
/* A doc prometia que a tarefa roda "ate o primeiro ponto em que cede — sleep,
 * I/O". Num script o I/O nao cedia: rodava inline e travava o principal. */
{ "I/O da tarefa cede na chamada",
  "import os\n"
  "async funct t() {\n"
  "    post(\"a\")\n"
  "    os.run([\"sleep\", \"0.2\"], capture=True)\n"
  "    post(\"b\")\n"
  "}\n"
  "f = t()\n"
  "post(\"chamou\")\n"
  "await f\n"
  "post(\"fim\")\n",
  "a\nchamou\nb\nfim", NULL, 0 },
{ "erro depois de ceder aparece no fim, com o principal dormindo",
  "async funct t() {\n"
  "    sleep(0.1)\n"
  "    raise Exception(\"depois da cedencia\")\n"
  "}\n"
  "f = t()\n"
  "sleep(0.3)\n"
  "post(\"fim\")\n",
  "fim", "ninguem aguardou", 1 },
/* O erro da tarefa saia no stderr e o programa saia com 0: um script de build
 * ou o CI tratava a falha como sucesso. Agora e 1, o mesmo de uma excecao nao
 * pega no principal. `sys.exit(n)` explicito mantem o `n` — e o erro continua
 * sendo impresso. */
{ "erro de tarefa nao lida: sys.exit(0) mantem o 0, e o erro sai mesmo assim",
  "import sys\n"
  "async funct t() {\n"
  "    sleep(0.1)\n"
  "    raise Exception(\"depois da cedencia\")\n"
  "}\n"
  "f = t()\n"
  "sleep(0.3)\n"
  "post(\"fim\")\n"
  "sys.exit(0)\n",
  "fim", "ninguem aguardou", 0 },
{ "erro de tarefa nao lida: sys.exit(3) mantem o 3",
  "import sys\n"
  "async funct t() {\n"
  "    sleep(0.1)\n"
  "    raise Exception(\"depois da cedencia\")\n"
  "}\n"
  "f = t()\n"
  "sleep(0.3)\n"
  "sys.exit(3)\n",
  "", "ninguem aguardou", 3 },
{ "tarefa que termina bem: o programa continua saindo com 0",
  "async funct t() {\n"
  "    sleep(0.1)\n"
  "    post(\"t\")\n"
  "}\n"
  "f = t()\n"
  "sleep(0.3)\n"
  "post(\"fim\")\n",
  "t\nfim", NULL, 0 },

/* ── superinstrucoes: JF_* (comparacao+desvio) e INC/DEC_*_INT (`x++` em int
 * declarado). O comportamento e o mesmo das instrucoes soltas; o que muda e
 * o numero de despachos. Cada caso abaixo passa pelo caminho fundido. */
{ "n++ em int declarado no modulo estoura como o COERCE_DECL estourava",
  "int n = 9223372036854775807\n"
  "n++\n"
  "post(n)\n",
  "", "variável n esperava int, e o valor nao cabe em 64 bits (declare como 'long n'", -1 },
{ "n-- em int declarado no minimo de 64 bits",
  "int n = -9223372036854775807 - 1\n"
  "n--\n",
  "", "variável n esperava int, e o valor nao cabe em 64 bits (declare como 'long n'", -1 },
{ "i++ em int declarado LOCAL estoura com o nome da variavel",
  "funct f() {\n"
  "    int i = 9223372036854775807\n"
  "    i++\n"
  "    return i\n"
  "}\n"
  "f()\n",
  "", "variável i esperava int, e o valor nao cabe em 64 bits (declare como 'long i'", -1 },
{ "n++ em int inferido (sem declaracao) promove a bigint",
  "n = 9223372036854775807\n"
  "n++\n"
  "post(n)\n",
  "9223372036854775808", NULL, 0 },
{ "n++ em long promove a bigint",
  "long n = 9223372036854775807\n"
  "n++\n"
  "post(n)\n",
  "9223372036854775808", NULL, 0 },
{ "x++, x--, x += 1 e x -= 1 em int declarado, global e local",
  "int a = 5\n"
  "a++\n"
  "a += 1\n"
  "a--\n"
  "a -= 1\n"
  "funct f() {\n"
  "    int b = 10\n"
  "    b++\n"
  "    b += 1\n"
  "    b--\n"
  "    b -= 1\n"
  "    b -= 1\n"
  "    return b\n"
  "}\n"
  "post(a, f())\n",
  "5 9", NULL, 0 },
{ "x++ como VALOR (nao statement) continua devolvendo o valor antigo",
  "int a = 5\n"
  "b = a++\n"
  "post(a, b)\n",
  "6 5", NULL, 0 },
{ "comparacao fundida: while e if com int, flo, str, bool e lista",
  "int i = 0\n"
  "while i < 3 { i++ }\n"
  "s = \"\"\n"
  "if 1 < 1.5 { s = s + \"a\" }\n"
  "if \"a\" < \"b\" { s = s + \"b\" }\n"
  "if True < 2 { s = s + \"c\" }\n"
  "if [1, 2] == [1, 2] { s = s + \"d\" }\n"
  "if [1] != [2] { s = s + \"e\" }\n"
  "if 2 ** 70 > 1 { s = s + \"f\" }\n"
  "if 3 >= 3 and 2 <= 2 { s = s + \"g\" }\n"
  "if 1 == 1.0 { s = s + \"h\" }\n"
  "if 5 > 6 { s = s + \"X\" }\n"
  "post(i, s)\n",
  "3 abcdefgh", NULL, 0 },
{ "comparacao fundida com Null levanta a mesma frase da comparacao solta",
  "x = Null\n"
  "if x < 1 { post(1) }\n",
  "", "'<' not supported between instances of 'Null' and 'int'", -1 },
{ "comparacao fundida entre tipos incompativeis levanta como a solta",
  "while \"a\" > 1 { post(1) }\n",
  "", "'>' not supported between instances of 'str' and 'int'", -1 },
{ "condicao que nao e comparacao segue pela verdade do valor",
  "l = [1]\n"
  "n = 0\n"
  "while l { n++\n l = [] }\n"
  "if not n { post(\"X\") }\n"
  "post(n)\n",
  "1", NULL, 0 },

/* ── range decodificado uma vez (RANGE_PREPARA + ITER_RANGE so aritmetica) ── */
{ "range: passo negativo, bool e inteiro grande como limite (lote 7b)",
  "list s = []\n"
  "for each i in range(10, 0, -3) { s.append(i) }\n"
  "for each i in range(True, 3) { s.append(i) }\n"
  "for each i in range(5, 5) { s.append(i) }\n"
  "for each i in range(1, 10 ** 30) {\n"
  "    if i > 3 { break }\n"
  "    s.append(i)\n"
  "}\n"
  "post(s, range(-2 ** 70, -2 ** 70 + 2), range(2 ** 64, 2 ** 64 + 10, 4))\n"
  "try {\n"
  "    for each g in range(2 ** 64, 2 ** 64 + 2) { post(g) }\n"
  "} catch (OverflowError e) {\n"
  "    post(e)\n"
  "}\n",
  "[10, 7, 4, 1, 1, 2, 1, 2, 3] [-1180591620717411303424, -1180591620717411303423] "
  "[18446744073709551616, 18446744073709551620, 18446744073709551624]\n"
  "range() no for each comeca fora de 64 bits: guarde numa lista antes (list r = range(...)) (linha 11)",
  NULL, 0 },
{ "range so aceita inteiro: flo e texto sao erro, sem conversao calada (lote 7b)",
  "for each a in [2.9, \"3\"] {\n"
  "    try {\n"
  "        for each i in range(a) { post(i) }\n"
  "    } catch (TypeError e) {\n"
  "        post(e)\n"
  "    }\n"
  "}\n"
  "post(range(int(\"3\")))\n",
  "'flo' object cannot be interpreted as an integer (linha 3)\n"
  "'str' object cannot be interpreted as an integer (linha 3)\n"
  "[0, 1, 2]", NULL, 0 },
{ "range com str nao numerica levanta como antes, na linha do for each",
  "for each i in range(\"x\") { post(i) }\n",
  "", "'str' object cannot be interpreted as an integer", -1 },
{ "range com passo zero levanta como antes",
  "for each i in range(1, 2, 0) { post(i) }\n",
  "", "range() arg 3 must not be zero", -1 },
{ "range grande com break, continue e return dentro",
  "funct f() {\n"
  "    n = 0\n"
  "    for each i in range(0, 1000000000000000000, 100000000000000000) {\n"
  "        if i == 0 { continue }\n"
  "        n = n + 1\n"
  "        if n == 3 { break }\n"
  "    }\n"
  "    for each j in range(100) {\n"
  "        if j == 7 { return [n, j] }\n"
  "    }\n"
  "    return Null\n"
  "}\n"
  "post(f())\n",
  "[3, 7]", NULL, 0 },
{ "range dentro de gerador com yield no meio do laco",
  "funct g() {\n"
  "    for each i in range(1, 4) {\n"
  "        yield i * 10\n"
  "    }\n"
  "}\n"
  "s = 0\n"
  "for each v in g() { s = s + v }\n"
  "post(s)\n",
  "60", NULL, 0 },

/* ── caminho quente da chamada (posicional exato, sem liga_args) ── */
/* o valor vem de um indice de lista pra o checador estatico nao tipar (um
 * literal `True` ele recusa antes de rodar): aqui e a conferencia de RUNTIME */
{ "chamada exata com parametro tipado passa e o tipo errado levanta a mesma frase",
  "funct f(int a, str b) { return b + str(a) }\n"
  "post(f(1, \"x\"))\n"
  "v = [True][0]\n"
  "post(f(v, \"x\"))\n",
  "x1", "parâmetro a de f() esperava int, recebeu bool", -1 },
{ "chamada com aridade errada continua dando as frases do binding completo",
  "funct f(a, b) { return a }\n"
  "post(f(1))\n",
  "", "missing 1 required positional argument", -1 },
{ "chamada com argumento a mais continua dando a frase do binding completo",
  "funct f(a) { return a }\n"
  "post(f(1, 2))\n",
  "", "takes 1 positional argument but 2 were given", -1 },
{ "metodo, @static, padrao, *args e nomeado seguem funcionando",
  "Entity P {\n"
  "    funct __init__(self, n) { self.n = n }\n"
  "    funct mais(self, k) { return self.n + k }\n"
  "    @static\n"
  "    funct dobro(x) { return x * 2 }\n"
  "}\n"
  "funct d(a, b = 2) { return a + b }\n"
  "funct v(*args) { return len(args) }\n"
  "p = P(5)\n"
  "post(p.mais(1), P.dobro(4), d(1), d(1, 5), d(a=3), v(1, 2, 3), v())\n",
  "6 8 3 6 5 3 0", NULL, 0 },
{ "recursao no caminho quente: fib com parametro tipado",
  "funct fib(int n) {\n"
  "    if n < 2 { return n }\n"
  "    return fib(n - 1) + fib(n - 2)\n"
  "}\n"
  "post(fib(20))\n",
  "6765", NULL, 0 },

/* ── chamada direta entre functs em codigo de maquina (jit_chama) ── */
{ "erro tres frames abaixo de chamadas diretas: sobe com o traceback inteiro",
  "funct c(int x) {\n"
  "    raise Exception(\"fundo\")\n"
  "}\n"
  "funct b(int x) { return c(x) + 1 }\n"
  "funct a(int x) { return b(x) + 1 }\n"
  "post(a(1))\n",
  "", "Exception: fundo", -1 },
{ "try no chamador pega o erro de uma funct chamada direto",
  "funct c(int x) {\n"
  "    raise Exception(\"fundo\")\n"
  "}\n"
  "funct b(int x) { return c(x) }\n"
  "funct a(int x) {\n"
  "    try { return b(x) }\n"
  "    catch (e) { return \"pego\" }\n"
  "}\n"
  "post(a(1), a(2))\n",
  "pego pego", NULL, 0 },
{ "recursao sem fim continua dando RecursionError, nao estouro da pilha C",
  "funct r(int n) { return r(n + 1) }\n"
  "r(0)\n",
  "", "maximum recursion depth exceeded", -1 },
{ "recursao mutua e chamada de dentro de closure",
  "funct par(int n) {\n"
  "    if n == 0 { return True }\n"
  "    return impar(n - 1)\n"
  "}\n"
  "funct impar(int n) {\n"
  "    if n == 0 { return False }\n"
  "    return par(n - 1)\n"
  "}\n"
  "funct fora() {\n"
  "    k = 3\n"
  "    funct dentro() { return par(k) }\n"
  "    return dentro\n"
  "}\n"
  "post(par(10), impar(7), fora()())\n",
  "True True False", NULL, 0 },
{ "parametro tipado errado numa chamada direta levanta a mesma frase",
  "funct f(int a) { return a }\n"
  "funct g(x) { return f(x) }\n"
  "v = [\"a\"][0]\n"
  "post(g(v))\n",
  "", "parâmetro a de f() esperava int, recebeu str", -1 },
{ "locais e resultados atravessam chamadas diretas aninhadas",
  "funct sq(int x) { return x * x }\n"
  "funct soma(int n) {\n"
  "    int s = 0\n"
  "    int i = 0\n"
  "    while i < n {\n"
  "        s = s + sq(i)\n"
  "        i++\n"
  "    }\n"
  "    return s\n"
  "}\n"
  "post(soma(100), sq(12))\n",
  "328350 144", NULL, 0 },
{ "funct com padrao, *args, gerador e metodo continuam certos (nao sao chamada direta)",
  "funct d(a, b = 2) { return a + b }\n"
  "funct v(*args) { return len(args) }\n"
  "funct g() { yield 1\n yield 2 }\n"
  "Entity P {\n"
  "    funct __init__(self, n) { self.n = n }\n"
  "    funct dobro(self) { return self.n * 2 }\n"
  "}\n"
  "funct usa(int k) { return d(k) + v(1, 2, k) + P(k).dobro() }\n"
  "s = 0\n"
  "for each x in g() { s = s + x }\n"
  "post(usa(5), s)\n",
  "20 3", NULL, 0 },
/* A chamada direta e ponto seguro do GC: a coleta acontece no meio da
 * recursao nativa, e a lista de cada chamada, lida depois das chamadas
 * filhas, tem de sobreviver a ela. */
{ "recursao que aloca coleta no meio e a lista de cada chamada sobrevive",
  "funct g(int n) {\n"
  "    l = [n, n + 1, n + 2, n + 3]\n"
  "    if n < 2 { return l[1] - n }\n"
  "    return g(n - 1) + g(n - 2) + l[3] - l[0] - 3\n"
  "}\n"
  "post(g(20))\n",
  "10946", NULL, 0 },
/* O future de uma `tarefa()` solta nao e alcancavel pelo programa: o aviso do
 * fim procurava no heap, e o GC o levava antes. Agora ele e raiz ate o erro ser
 * lido ou avisado. As duas listas grandes forcam coleta. */
{ "erro de tarefa solta nao some com o GC",
  "async funct t() {\n"
  "    sleep(0.1)\n"
  "    raise Exception(\"erro da tarefa solta\")\n"
  "}\n"
  "t()\n"
  "sleep(0.3)\n"
  "l = []\n"
  "for each i in range(300000) {\n"
  "    l.append(str(i) + \"x\")\n"
  "}\n"
  "l = Null\n"
  "m = []\n"
  "for each i in range(300000) {\n"
  "    m.append(str(i) + \"y\")\n"
  "}\n"
  "post(\"fim\")\n",
  "fim", "erro da tarefa solta", 1 },
{ "fim do programa continua sem esperar a tarefa",
  "async funct t() {\n"
  "    sleep(5)\n"
  "    post(\"nunca\")\n"
  "}\n"
  "t()\n"
  "sleep(0.1)\n"
  "post(\"fim\")\n",
  "fim", NULL, 0 },
/* A tarefa roda DENTRO do `sleep` do principal — e pode fazer `import`, que
 * realoca a tabela de protos. O principal volta do builtin com o ponteiro pro
 * proto dele reancorado; sem isso, lia memoria solta (o ASan acusa). */
{ "tarefa que importa enquanto o principal dorme",
  "if __name__ == \"main\" {\n"
  "    async funct t() {\n"
  "        sleep(0.05)\n"
  "        import ps_mod_reanc as m\n"
  "        post(\"importou\", m.X)\n"
  "    }\n"
  "    f = t()\n"
  "    sleep(0.3)\n"
  "    post(\"fim\")\n"
  "}\n"
  "X = 7\n",
  "importou 7\nfim", NULL, 0, "ps_mod_reanc.pr" },
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
  "import sqlite3\nc = sqlite3.connect(\"ps_str1.db\")\npost(\"[\" + str(c) + \"]\")\n",
  "[<sqlite3.Connection>]", NULL, 0 },
{ "str() de arquivo aberto",
  "using open(\"ps_str2.txt\", \"w\") as f {\n"
  "    post(\"[\" + str(f) + \"]\")\n"
  "}\n",
  "[<arquivo ps_str2.txt>]", NULL, 0 },
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
/* As duas listas de opacos ainda divergiam (2026-10-15): o `str()` dava texto
 * vazio pra PoolFile do loadFile, módulo e os registradores do jinker, e
 * nenhuma das duas conhecia o Pattern nem a closure — `post([regex.compile("a")])`
 * imprimia `[]`. Agora a `descreve_obj` é a lista única, e tipo sem descrição
 * própria sai `<nome do tipo>`. O Pattern segue o repr do `re` do Python,
 * com o padrão cortado em 200 caracteres (o `%.200R` do CPython). */
{ "post, str, lista e f-string iguais pra Pattern, closure, PoolFile, modulo, enum, Entity e metodo",
  "import regex\nimport os\nenum Cor {\n    RED\n}\nclass Conta {\n    int saldo = 0\n}\n"
  "funct externo() {\n    int a = 1\n    funct dentro() {\n        return a\n    }\n    return dentro\n}\n"
  "os.writeFile(\"a.txt\", \"x\")\n"
  "list coisas = [regex.compile(\"a+\"), regex.compile(\"(\?i)ção\\\\d\"), externo(), os.loadFile(\"a.txt\", mode=\"rb\"), os, Cor, Conta, \"a\".upper]\n"
  "for each c in coisas {\n    post(c, \"|\" + str(c) + \"|\", [c], f\"{c}\")\n}\n"
  "str longo = str(regex.compile(\"x\" * 300))\npost(len(longo), longo[len(longo) - 3:])\n",
  "regex.compile('a+') |regex.compile('a+')| [regex.compile('a+')] regex.compile('a+')\n"
  "regex.compile('(\?i)ção\\\\d') |regex.compile('(\?i)ção\\\\d')| [regex.compile('(\?i)ção\\\\d')] regex.compile('(\?i)ção\\\\d')\n"
  "<funct #4> |<funct #4>| [<funct #4>] <funct #4>\n"
  "<PoolFile 'a.txt' (1 bytes)> |<PoolFile 'a.txt' (1 bytes)>| [<PoolFile 'a.txt' (1 bytes)>] <PoolFile 'a.txt' (1 bytes)>\n"
  "<modulo os> |<modulo os>| [<modulo os>] <modulo os>\n"
  "<enum Cor> |<enum Cor>| [<enum Cor>] <enum Cor>\n"
  "<Entity Conta> |<Entity Conta>| [<Entity Conta>] <Entity Conta>\n"
  "<metodo> |<metodo>| [<metodo>] <metodo>\n"
  "215 xx)", NULL, 0 },

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
  "if (True) {\n"
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
 * tabela do interpretador — mas eram valor de argumento de UMA lib (a manpu,
 * que saiu no 19.0.0), e
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
  "post([1,2].append(1,2,3))\n", "", "TypeError: append() takes exactly one argument (3 given)", 2 },
{ "metodo de zero argumento recusa argumento",
  "import sockets\ns = sockets.socket()\npost(s.fileno(1,2))\n",
  "", "TypeError: fileno() takes no arguments (2 given)", 2 },
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
{ "isidentifier segue o nome da linguagem: ASCII, _ e digito depois do inicio (lote 7a)",
  "post(\"ção\".isidentifier(), \"a٣\".isidentifier(), \"_x1\".isidentifier(), \"1abc\".isidentifier())\n"
  "post(\"CAFÉ\".isidentifier(), \"Ω\".isidentifier(), \"漢字\".isidentifier(), \"if\".isidentifier(), \"\".isidentifier())\n",
  "False False True False\nFalse False False True False", NULL, 0 },
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
  "if (True) {\n"
  "    if True {\n"
  "        post(\"x\")\n"
  "    }\n"
  "}\n", "x", NULL, 0 },
{ "try/catch ':' dentro de bloco de chaves",
  "if (True) {\n"
  "    try {\n"
  "        raise Boom(\"x\")\n"
  "    } catch(e) {\n"
  "        post(\"peguei\")\n"
  "    }\n"
  "}\n",
  "peguei", NULL, 0 },
{ "for each ':' dentro de bloco de chaves",
  "if (True) {\n"
  "    for each i in range(2) {\n"
  "        post(i)\n"
  "    }\n"
  "}\n", "0\n1", NULL, 0 },
{ "bloco de chaves dentro de bloco ':'",
  "if True {\n"
  "    funct f() {\n"
  "        return 1\n"
  "    }\n"
  "    post(f())\n"
  "}\n", "1", NULL, 0 },
{ "metodo ':' dentro de Entity de chaves",
  "Entity P {\n"
  "    funct m(self) {\n"
  "        return 3\n"
  "    }\n"
  "}\n"
  "post(P().m())\n", "3", NULL, 0 },
{ "metodo de chaves dentro de Entity ':'",
  "Entity Q {\n"
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
/* O recuo só conta logo depois de um `:` no fim da linha (o guard `main`);
 * no resto é estética, também fora de chaves (doc 1.3). Era regra estrita
 * fora das chaves: `int x = 1` seguido de `    post(x)` dava "expressao
 * invalida", 2 espaços "multiplo de 4", TAB "nao e permitida". */
{ "if com ':' acusa o bloco de ':', nao o recuo",
  "if True:\n  post(\"2 espacos\")\n", "", "SyntaxError: bloco com ':' nao existe mais — use '{ }'", 2 },
{ "recuo fora de chaves e estetica: 4, 2, 8 espacos e TAB",
  "int x = 1\n    post(x)\n  post(x + 1)\n        post(x + 2)\n\tpost(x + 3)\n", "1\n2\n3\n4", NULL, 0 },
{ "recuo solto entre declaracoes do topo",
  "# configuracao\nint porta = 8080\n    str host = \"x\"\npost(porta, host)\n", "8080 x", NULL, 0 },
{ "guard main com ':' e recuo de 2, com linha mais recuada dentro",
  "if __name__ == \"main\":\n  post(\"a\")\n      post(\"b\")\n  post(\"c\")\npost(\"fora\")\n",
  "a\nb\nc\nfora", NULL, 0 },
{ "guard main com ':' sem recuo na linha de baixo",
  "if __name__ == \"main\":\npost(\"a\")\n", "", "SyntaxError: faltou indentacao apos ':'", 2 },

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
{ "range recusa texto: so inteiro, sem conversao implicita (lote 7b)",
  "for each i in range(\"3\") {\n"
  "    post(i)\n"
  "}\n", "", "'str' object cannot be interpreted as an integer", 1 },
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
  "char c = \"ç\"\nchar e = 134071\npost(c, len(c), e)\n", "ç 1 𠮷", NULL, 0 },
{ "char recusa mais de um caractere",
  "char c = \"abc\"\n",
  "", "AttributedValueError: variável c esperava char (um caractere), recebeu 3", 2 },
{ "char recusa flutuante",
  "char c = 1.5\n",
  "", "AttributedValueError: variável c esperava char, recebeu flo", 2 },
/* O nome de classe que fixa o tipo de um global vinha da árvore da
 * declaração em compilação, e a compilação por declaração solta essa árvore
 * antes da funct seguinte: a mensagem saía com lixo no lugar de `Bar`. As
 * atribuições de `h` reocupam a memória da declaração anterior. */
{ "tipo fixado por global de dentro de funct sobrevive a declaracao seguinte",
  "g = Null\n"
  "Entity Bar {\n"
  "    funct __init__(self) {\n"
  "        self.x = 1\n"
  "    }\n"
  "}\n"
  "funct f(Bar b) {\n"
  "    global g\n"
  "    g = b\n"
  "}\n"
  "funct h() {\n"
  "    a1 = 1\n    a2 = 2\n    a3 = 3\n    a4 = 4\n    a5 = 5\n    a6 = 6\n"
  "    a7 = 7\n    a8 = 8\n    a9 = 9\n    a10 = 10\n    a11 = 11\n    a12 = 12\n"
  "    int x = g\n"
  "    return x\n"
  "}\n"
  "f(Bar())\n"
  "post(h())\n",
  "", "AttributedValueError: variável x esperava int, recebeu Bar", 2 },
{ "char recusa codepoint invalido",
  "char c = -1\n", "", "nao e um caractere valido", 1 },
/* `char funct` passou a EXISTIR (2026-09-08): todo tipo vale como retorno. O
 * `char` aqui declara o retorno e nao coage — devolve o 1 como esta. Quem
 * coage e a declaracao de VARIAVEL (`char c = 1`), logo acima. */
{ "char funct: devolver int e erro de tipo — fora da declaracao, char e um caractere de texto",
  "char funct f() {\n"
  "    return 1\n"
  "}\npost(f())\n",
  "", "AttributedValueError: retorno de f() esperava char, recebeu int", 2 },
/* ── NENHUMA conversao implicita, 2026-09-09 ───────────────────────────────
 * `int b = "7"` virava 7 e `flo c = 1` virava 1.0 ("conversoes que nao perdem
 * informacao"). Ele nunca pediu isso: "se eu tenho terra eu transformo em
 * cacau em po?". A declaracao CONFERE o tipo e nao converte nada; converter e
 * escrever `int("7")`, `flo(5)`. O `ConversionError` da declaracao de int/flo
 * deixou de existir — sobrou so no `char c = -1`. */
{ "declaracao tipada aceita o tipo EXATO",
  "str a = \"oi\"\nint b = 7\nflo c = 1.0\nbool d = True\npost(a, b, c, d)\n",
  "oi 7 1.0 True", NULL, 0 },
{ "int NAO aceita string numerica: nao converte",
  "int b = \"7\"\n",
  "", "AttributedValueError: variável b esperava int, recebeu str", 2 },
{ "flo NAO aceita int: nao alarga",
  "flo c = 1\n",
  "", "AttributedValueError: variável c esperava flo, recebeu int", 2 },
{ "flo NAO aceita string numerica",
  "flo x = \"1.5\"\n",
  "", "AttributedValueError: variável x esperava flo, recebeu str", 2 },
{ "int recusa texto invalido pelo MESMO erro (nao ha 'quase converteu')",
  "int x = \"abc\"\n",
  "", "AttributedValueError: variável x esperava int, recebeu str", 2 },
{ "conversao e explicita: tipo(valor)",
  "int n = int(\"7\")\nflo f = flo(5)\nstr s = str(42)\npost(n, f, s)\n", "7 5.0 42", NULL, 0 },
{ "long aceita int porque int E inteiro — nao e conversao",
  "long y = 5\npost(y)\n", "5", NULL, 0 },

/* ── libs do motor e o `import` ─────────────────────────────────────────
 * `Parsing` é lib como `json` (até o 16.2.14 nascia ligado, sem import), e
 * `sys.stdout`/`sys.stderr` são atributos, que não se importam. O
 * `--metadata` chegou a anunciá-los pelo nome de REGISTRO (`_stdout`), e aí o
 * editor sugeria `import _stdout`, que só podia dar ImportError. */
{ "Parsing é lib: import, import as e from import",
  "import Parsing\n"
  "import Parsing as P\n"
  "from Parsing import integer\n"
  "post(type(Parsing), Parsing.integer(\"R$ 1.234\"), P.floating(\"19,90\"), integer(\"12 unidades\"))\n",
  "module 1234 19.9 12", NULL, 0 },
{ "Parsing sem import: NameError com a dica do import",
  "post(Parsing.integer(\"5\"))\n",
  "", "NameError: name 'Parsing' is not defined. Did you forget to import 'Parsing'?", 2 },
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
/* motor-3: o `to` usado como VALOR caia no acucar do Parsing (`to <tipo>`) e a
 * frase falava do operador de conversao, que nao e a causa. A declaracao
 * `str to = "x"` ja dizia a frase certa; agora o valor diz a mesma. */
{ "motor-3: o to usado como valor diz que e palavra reservada",
  "from mail import MailServer\nMailServer s = MailServer(\"127.0.0.1\", 25)\ns.send(to, subject=\"x\")\n",
  "", "'to' e palavra reservada da linguagem e nao pode ser usada como nome de variavel", 2 },
{ "motor-3: o to como conversao do Parsing continua valendo",
  "from Parsing import string\npost(string(\"42\", to int))\n", "42", NULL, 0 },
{ "motor-3: o to com tipo que nao existe mantem a frase do tipo",
  "from Parsing import string\npost(string(\"42\", to banana))\n",
  "", "esperado tipo apos 'to'", 2 },
/* motor-17: o `--check` nao sabia o tipo de quatro valores, e aceitava o que
 * o motor recusava rodando. O nome de builtin como valor e `funct`; a
 * chamada de uma `async funct` e `future`; a de uma funct com `yield` e
 * `generator`; e o membro de enum e `int` (o valor do membro). */
{ "motor-17: o nome de builtin usado como valor e funct",
  "int x = len\n", "", "variável x esperava int, recebeu funct", 2 },
{ "motor-17: a chamada de uma async funct e future",
  "int async funct f() { return 1 }\nint x = f()\n",
  "", "variável x esperava int, recebeu future", 2 },
{ "motor-17: a chamada de uma funct com yield e generator",
  "int funct gen() { yield 1 }\nstr s = gen()\n",
  "", "variável s esperava str, recebeu generator", 2 },
{ "motor-17: o membro de enum e int, e o metodo que ele nao tem e recusado",
  "enum Cor { RED, GREEN }\nint x = Cor.RED.foo()\n",
  "", "'int' object has no method 'foo'", 2 },
/* motor-20: o ramo do `match` com `:` e recuo era morto, e as frases dele
 * pediam o `:` que saiu da linguagem. A frase agora e a do `if True:`, que
 * aponta a causa. */
{ "motor-20: o match com dois pontos diz que o bloco com ':' saiu",
  "int x = 1\nmatch x:\n    case 1 { post(\"um\") }\n",
  "", "bloco com ':' nao existe mais — use '{ }'", 2 },
{ "motor-20: o match com chaves continua valendo",
  "int x = 1\nmatch x {\n    case 1 { post(\"um\") }\n}\n", "um", NULL, 0 },
{ "pass no corpo de classe",
  "class Vazia {\n"
  "    pass\n"
  "}\n"
  "post(type(Vazia))\n", "Entity", NULL, 0 },
{ "pass fora de laco NAO e erro (ao contrario de continue)",
  "pass\npost(\"ok\")\n", "ok", NULL, 0 },
/* rc 2 e SyntaxError: e erro de quem escreveu, nao "nao implementado". Ate
 * o L2 do plano das contradicoes saia rc 3 / NotImplementedError. */
{ "continue fora de laco continua sendo erro",
  "continue\n", "", "SyntaxError: 'continue' fora de laco", 2 },

/* ── estilo Allman: a chave na LINHA SEGUINTE ───────────────────────────────
 * FALTAVA NO PORTE (nunca esteve no parser em C, conferido até a 8.2.30):
 * `Entity`/`class`/`funct` aceitavam, porque o cabeçalho deles pula
 * separadores antes de procurar o `{`; `if`/`while`/`for`/`try` não. O MESMO
 * arquivo passava numa construção e falhava na outra — foi o que impedia uma
 * lib real de carregar. */
{ "if com chave na linha seguinte",
  "if (True)\n{\n    post(\"A\")\n}\n", "A", NULL, 0 },
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
  "if True {\n"
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
  "if True:\n    post(\"A\")\n", "", "bloco com ':' nao existe mais", 2 },
{ "bloco com ':' numa linha so tambem e recusado",
  "if True: post(\"A\")\n", "", "bloco com ':' nao existe mais", 2 },
{ "Entity com ':' e recusada",
  "Entity A:\n    funct m(self) { return 1 }\n", "", "bloco com ':' nao existe mais", 2 },
{ "a mensagem diz o que usar no lugar",
  "while True:\n    break\n", "", "use '{ }'", 2 },
{ "dicionario com ':' continua valendo",
  "d = { \"a\": 1, \"b\": 2 }\npost(d[\"a\"], d[\"b\"])\n", "1 2", NULL, 0 },
{ "fatia com ':' continua valendo",
  "post(\"abcdef\"[1:3], [1,2,3][0:2])\n", "bc [1, 2]", NULL, 0 },
{ "campo tipado de Entity com ':' continua valendo",
  "Entity P {\n    nome: str\n}\np = P(\"ana\")\npost(p.nome)\n", "ana", NULL, 0 },

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
  "['a', 'b']\n['a', 'b']", NULL, 0, "ps_mod_clo.pr" },
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
  "1 2 3\n1 2 3", NULL, 0, "ps_mod_cnt.pr" },

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

/* As interpolações são NÓS FILHOS do literal, com a posição real. Enquanto a
 * f-string era um token opaco, o que estava dentro das chaves não existia pra
 * ninguém que anda na árvore — e a captura de closure anda na árvore: uma
 * variável usada SÓ dentro da f-string não era capturada e dava NameError. */
{ "closure captura nome usado so dentro da f-string",
  "funct fora() {\n"
  "    x = 5\n"
  "    funct dentro() {\n"
  "        post(f\"x vale {x}\")\n"
  "    }\n"
  "    return dentro\n"
  "}\n"
  "d = fora()\n"
  "d()\n", "x vale 5", NULL, 0 },
{ "closure captura nome de duas interpolacoes",
  "funct fora() {\n"
  "    a = 1\n"
  "    b = 2\n"
  "    return funct(){ return f\"{a}-{b}\" }\n"
  "}\n"
  "post(fora()())\n", "1-2", NULL, 0 },
{ "erro dentro da f-string aponta a COLUNA da expressao",
  "x = 0\n"
  "post(f\"r: {1 / x}\")\n", "", "|              ^^^", 1 },
{ "chave literal {{ }} nao vira interpolacao",
  "v = 9\npost(f\"{{{v}}}\")\n", "{9}", NULL, 0 },
{ "f-string tripla continua interpolando",
  "v = 4\npost(f\"\"\"a {v} b\"\"\")\n", "a 4 b", NULL, 0 },
{ "f-string com escape antes da chave",
  "v = 2\npost(f\"a\\tb {v}\")\n", "a\tb 2", NULL, 0 },

/* ── input(): fim da entrada é `Null`, linha vazia é `""` ───────────────────
 * O runner roda todo caso com stdin em /dev/null, então aqui a entrada já
 * começa acabada. Sem o `Null`, `while True { input() }` giraria pra sempre
 * quando o outro lado fechasse o cano — foi o que travou o servidor LSP. */
/* ── sys.stdin.raw() — modo cru do terminal, 2026-09-09 ────────────────────
 * A suíte roda com stdin em /dev/null (NUL no Windows: não é terminal, embora
 * o `_isatty` do CRT diga que é), então aqui `raw` só
 * pode ser CONFERIDO no caminho não-tty: devolve false e não estoura, e a
 * leitura segue devolvendo Null no fim. O caminho do terminal não tem teste:
 * a prova era o examples/cobrinha.pr, que precisava de pty e saiu no
 * bea30df2 (teste-12 da lista). */
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
 * decide o modo automatico e o CONTEUDO, e conteudo sem NUL e texto. O PNG
 * vem do proprio motor (qrcode): valido pro sistema, nao so "binario". */
{ "ext do aberto e do carregado sao iguais",
  "import os\nimport qrcode\nqrcode.make(\"oi\").save(\"u3.png\")\n"
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
  "import sys\npost(sys.stdin.raw(True), sys.stdin.raw(False))\n", "False False", NULL, 0 },
{ "raw() nao muda a leitura de pipe: Null no fim",
  "import sys\nsys.stdin.raw(True)\npost(type(sys.stdin.read()))\n", "Null", NULL, 0 },
{ "raw() exige bool",
  "import sys\nsys.stdin.raw(1)\n", "", "raw() espera bool", 1 },
{ "input() no fim da entrada devolve null",
  "post(input())\n", "Null", NULL, 0 },
{ "input() no fim é null, não string vazia",
  "post(input() == Null, input() == \"\")\n", "True False", NULL, 0 },
{ "input() em laço termina no fim da entrada",
  "n = 0\n"
  "while True {\n"
  "    l = input()\n"
  "    if l == Null {\n"
  "        break\n"
  "    }\n"
  "    n = n + 1\n"
  "}\n"
  "post(\"linhas:\", n)\n",
  "linhas: 0", NULL, 0 },

/* ── ponto de entrada: `if __name__ == "main"` ──────────────────────────────
 * Substituiu o `run_selfwith_`. No arquivo executado `__name__` vale "main", e
 * a condição é verdadeira também avaliada; o guard existe pelo import (um
 * `main.pr` importado também tem `__name__ == "main"`), e é reconhecido pela
 * FORMA, só com o literal "main". É o único lugar onde `:` ainda abre bloco;
 * `{ }` vale igual. */
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
  "corpo\ncorpo\nguard", NULL, 0, "ps_guard.pr" },
{ "run_selfwith_ saiu, e a recusa ensina",
  "run_selfwith_(\"main\") {\n    post(1)\n}\n",
  "", "use: if __name__ == \"main\"", 2 },
{ "if normal continua sem aceitar dois-pontos",
  "if 1 == 1:\n    post(1)\n", "", "bloco com ':' nao existe mais", 2 },
{ "if com __name__ mas comparando outra coisa e if normal",
  "x = 1\nif __name__ == x {\n    post(\"nao\")\n} else {\n    post(\"if normal\")\n}\n",
  "if normal", NULL, 0 },
/* A mesma expressão dava True no guard (reconhecido pela forma) e False
 * escrita solta, porque `__name__` valia o caminho do arquivo. */
{ "__name__ == \"main\" vale o mesmo dentro e fora do if",
  "post(__name__ == \"main\")\nif __name__ == \"main\" {\n    post(\"entrou\")\n}\n",
  "True\nentrou", NULL, 0 },
{ "__name__ no arquivo executado vale main",
  "post(__name__)\n", "main", NULL, 0 },
{ "__name__ comparado com variavel que vale main entra no if comum",
  "x = \"main\"\nif __name__ == x {\n    post(\"sim\")\n}\n", "sim", NULL, 0 },
/* Qualquer string literal virava guard: `if __name__ == "banana"` rodava o
 * bloco no arquivo executado e o `else` nunca. */
{ "__name__ comparado com outra string e if comum, nao guard",
  "if __name__ == \"banana\" {\n    post(\"banana\")\n} else {\n    post(\"if comum\")\n}\n",
  "if comum", NULL, 0 },
{ "Jinker(__name__) e Jinker() dao o mesmo nome main",
  "from jinker import Jinker\npost(Jinker(__name__).name, Jinker().name)\n", "main main", NULL, 0 },

/* ── `{` depois de string NAO interpola quando abre bloco ───────────────────
 * `case c if m == 'GET' {` lia o `{` como interpolação da string e o case
 * ficava sem corpo. Já valia pro `for each`; agora vale pro `if`, `while`,
 * `match` e a guarda do `case`. */
{ "case com guarda terminada em string",
  "m = \"GET\"\nmatch m {\n    case c if c == \"GET\" {\n        post(\"pegou\")\n    }\n}\n",
  "pegou", NULL, 0 },
{ "match com sujeito terminado em string",
  "match \"a\" {\n    case \"a\" {\n        post(\"casou\")\n    }\n}\n", "casou", NULL, 0 },
/* motor-2: `match` so e COMANDO quando a linha termina em `:` (ou tem o `{`
 * do bloco). Sem isso, a chamada solta da funcao `match` do `regex` — a forma
 * que docs/regex.md ensina — era lida como o comando, e o erro saia na linha
 * de BAIXO, falando de um `:` que ninguem escreveu. A regra e a do Python. */
{ "motor-2: a chamada solta de match() e chamada, nao o comando match",
  "from regex import match\nmatch(r\"\\d+\", \"123\")\npost(\"depois\")\n",
  "depois", NULL, 0 },
{ "motor-2: match() solto com o retorno guardado tambem e chamada",
  "from regex import match\nbool r = match(r\"\\d+\", \"123\")\npost(r)\n",
  "True", NULL, 0 },
{ "motor-2: match() solto com dict no argumento e chamada, nao comando",
  "from regex import match\nmatch(r\"a\", \"a\")\npost(\"ok\")\n",
  "ok", NULL, 0 },
{ "motor-2: o comando match com chaves continua comando",
  "int x = 1\nmatch x {\n    case 1 { post(\"um\") }\n    case _ { post(\"outro\") }\n}\n",
  "um", NULL, 0 },
{ "motor-2: o comando match com o sujeito entre parenteses continua comando",
  "int x = 1\nmatch (x) {\n    case 1 { post(\"um\") }\n}\n",
  "um", NULL, 0 },
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
  "@app.route(\"/x\")\nfunct h() { return 1 }\n",
  "", "NameError: name 'app' is not defined", 2 },
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
  "class P {\n"
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
  "", "AttributedValueError: variável s esperava str, recebeu int", 2 },
/* F0 do plano da tipagem estatica (2026-09-16): o que o compilador perdia do
 * tipo declarado, calado. */
{ "tipagem estatica: local no slot 299 conserva o tipo (o vetor por slot era fixo em 256)",
  "import os\nimport sys\n"
  "linhas = [\"funct f() {\"]\n"
  "for each i in range(300) {\n"
  "    addEnd(linhas, \"    int v\" + str(i) + \" = \" + str(i))\n"
  "}\n"
  "addEnd(linhas, \"    v299 = \\\"texto\\\"\")\n"
  "addEnd(linhas, \"    return v299\")\n"
  "addEnd(linhas, \"}\")\n"
  "addEnd(linhas, \"f()\")\n"
  "os.writeFile(\"s300.pr\", \"\\n\".join(linhas) + \"\\n\")\n"
  /* sem shell: a aspa simples era sintaxe do sh, e o cmd.exe a toma como
   * parte do nome do programa. `capture=True` devolve o stdout, ou o stderr
   * se nao houve stdout (so um dos dois tem conteudo aqui). */
  "post(os.run([sys.executable, \"s300.pr\"], capture=True).split(\"\\n\")[0])\n",
  "AttributedValueError: variável v299 esperava int, recebeu str", NULL, 0 },
/* Uma tabela de tipos so (vm/ps_tipos.def): `JSON` e `Long` eram apelido num
 * lugar e nome solto noutro — `JSON j = {}` dava NameError e `count JSON` dava
 * "tipo desconhecido", com a doc listando `JSON` como apelido de dict. */
{ "tipagem estatica: JSON e Long abrem declaracao, e o tipo vale na escrita",
  "JSON j = {\"a\": 1}\n"
  "Long y = 5\n"
  "post(j, y)\n"
  "j = [1]\n",
  "", "AttributedValueError: variável j esperava dict, recebeu list", 2 },
{ "tipagem estatica: JSON vale em `is` e em `count`, igual a dict",
  "x = {\"a\": 1}\n"
  "post(x is JSON, count JSON in [{\"a\": 1}, 2], count dict in [{\"a\": 1}, 2])\n",
  "True 1 1", NULL, 0 },
{ "tipagem estatica: parametro tipado reatribuido com outro tipo e recusado",
  "funct f(int n) {\n"
  "    n = \"x\"\n"
  "    return n\n"
  "}\n"
  "f(1)\n",
  "", "AttributedValueError: variável n esperava int, recebeu str", 2 },
{ "tipagem estatica: padrao de parametro tipado e conferido",
  "funct f(str s = 10) {\n"
  "    return s\n"
  "}\n"
  "f()\n",
  "", "AttributedValueError: parâmetro s de f() esperava str, recebeu int", 2 },
{ "tipagem estatica: padrao valido e argumento passam",
  "funct f(str s = \"ok\") {\n"
  "    return s\n"
  "}\n"
  "post(f(), f(\"x\"))\n",
  "ok x", NULL, 0 },
{ "tipagem estatica: slot liberado no fim do bloco nao herda o tipo do nome de antes",
  "funct f() {\n"
  "    if True {\n"
  "        int a = 1\n"
  "    }\n"
  "    b = \"texto\"\n"
  "    return b\n"
  "}\n"
  "post(f())\n",
  "texto", NULL, 0 },
/* Sem conversao implicita (2026-09-09): a reatribuicao confere o tipo EXATO,
 * igual a criacao. `n = "7"` num `int n` era convertido pra 7; agora e erro. */
{ "tipagem estatica: `int n` recusa `n = \"7\"` — nao converte na escrita",
  "int n = 1\n"
  "n = \"7\"\n",
  "", "AttributedValueError: variável n esperava int, recebeu str", 2 },
{ "tipagem estatica: `flo f` recusa int na escrita; flo passa",
  "flo f = 1.0\n"
  "f = 2.0\n"
  "post(f)\n"
  "f = 2\n",
  "", "AttributedValueError: variável f esperava flo, recebeu int", 2 },
{ "tipagem estatica: `list l` recusa string; `dict d` recusa lista",
  "list l = [1]\n"
  "l = \"x\"\n",
  "", "AttributedValueError: variável l esperava list, recebeu str", 2 },
{ "tipagem estatica: Object recebe instancia de classe e o app do jinker",
  "from jinker import Jinker\n"
  "class C {\n"
  "    funct m(self) {\n"
  "        return 1\n"
  "    }\n"
  "}\n"
  "Object app = Jinker(__name__)\n"
  "object c = C()\n"
  "post(c.m())\n",
  "1", NULL, 0 },
/* `Object` e QUALQUER valor (decisao dele, 2026-10-13, 17.0.17: "Object
 * deveria ser o mesmo que qualquer objeto"); so o Null fica de fora, como em
 * todo tipo declarado. Ate o 17.0.16 ele recusava texto, colecao e escalar. */
{ "tipagem estatica: Object aceita qualquer valor, de todo tipo (17.0.17)",
  "Object o = \"x\"\nObject l = [1]\nObject n = 5\nObject b = True\nObject t = (1,)\n"
  "Object y = b\"ab\"\nObject d = {\"k\": 1}\nObject f = 2.5\nObject k = int\n"
  "post(o, l, n, b, t, y, d, f, k)\n",
  "x [1] 5 True (1,) b'ab' {'k': 1} 2.5 int", NULL, 0 },
{ "tipagem estatica: Object recusa o Null antes de rodar (17.0.17)",
  "Object o = Null\n",
  "", "AttributedValueError: variável o esperava Object, recebeu Null", 2 },

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
  "class C { funct m(self) { return 2 } }\n"
  "Object a = nom\nObject b = post\nObject c = C().m\n"
  "post(a(), c())\n", "1 2", NULL, 0 },
{ "Object no parametro aceita funct",
  "funct roda(Object cb) { return cb() }\npost(roda(funct(){ return 7 }))\n",
  "7", NULL, 0 },
{ "Object recusa o Null que so aparece rodando (17.0.17)",
  "funct nada() {\n    return Null\n}\nObject o = nada()\npost(\"nao chega\")\n",
  "", "AttributedValueError: variável o esperava Object, recebeu Null", 1 },

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
  "", "AttributedValueError: variável s esperava str, recebeu int", 2 },
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
  "", "AttributedValueError: variável n esperava int, recebeu str", 2 },
{ "tipagem estatica: `for each` num nome declarado confere cada volta",
  "str s = \"a\"\n"
  "for each s in [1, 2] {\n"
  "    post(s)\n"
  "}\n",
  "", "AttributedValueError: variável s esperava str, recebeu int", 2 },
{ "sem tipo escrito, o tipo e FIXADO na primeira atribuicao: `x = 1` depois `x = \"a\"` e erro",
  "x = 1\n"
  "x = \"a\"\n"
  "post(x)\n",
  "", "AttributedValueError: variável x é int (tipo fixado na primeira atribuição), recebeu str", 2 },
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
  "", "AttributedValueError: variável s esperava str, recebeu int", 2 },
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
{ "Entity sem __init__ e sem campo nao recebe argumento (antes de rodar)",
  "Entity Zero {\n"
  "    funct m(self) {\n"
  "        return 1\n"
  "    }\n"
  "}\n"
  "post(Zero().m())\n"
  "Zero(1, 2, 3)\n",
  "", "TypeError: Zero() takes no arguments (3 given)", 2 },
{ "@static chamado pela instancia: a mensagem diz pra chamar pela Entity",
  "Entity Mat {\n"
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
  "class Reg {\n"
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
  "class Reg {\n"
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
  "    return True\n"
  "}\n",
  "registrou\nregistrou", NULL, 0 },
{ "import sem nome nenhum",
  "import\n", "", "esperado nome de modulo depois de 'import'", 2 },
/* `import .nome`: com ponto na frente nao existe. A frase era a do `import`
 * sozinho — dizia faltar o nome, com o nome escrito ali, e falava `import` ate
 * num PUSH. Agora diz o que FUNCIONA, com os nomes escritos; as duas saidas
 * sugeridas rodam (medido). */
{ "import .m: frase diz o caminho entre aspas e o from, com os nomes escritos",
  "import .sub.m\n", "",
  "import com ponto na frente nao existe (o relativo e so com from); pra ligar o modulo: "
  "import './sub/m.pr' — pra trazer nomes: from .sub.m import nome", 2 },
{ "import ..m: cada ponto a mais sobe uma pasta no caminho sugerido",
  "import ..pkg.m\n", "",
  "pra ligar o modulo: import '../pkg/m.pr' — pra trazer nomes: from ..pkg.m import nome", 2 },
{ "import ...m: dois niveis acima",
  "import ...pkg.m\n", "", "import '../../pkg/m.pr'", 2 },
{ "import .jinga.libs.x: a resposta e a forma sem o ponto",
  "import .jinga.libs.random\n", "",
  "pra lib instalada, sem o ponto: import jinga.libs.random", 2 },
/* `poolscript.libs.` e o nome de antes do rename da linguagem: a mesma
 * resposta, com o nome que a pessoa escreveu */
{ "import .poolscript.libs.x (nome antigo): a mesma resposta",
  "import .poolscript.libs.random\n", "",
  "pra lib instalada, sem o ponto: import poolscript.libs.random", 2 },
{ "PUSH .m: a frase fala PUSH, nao import",
  "PUSH .sub.m GET v\n", "",
  "PUSH com ponto na frente nao existe; pelo caminho, relativo a este arquivo: "
  "PUSH './sub/m.pr' — e os nomes: PUSH './sub/m.pr' GET nome", 2 },
{ "import . sozinho: aponta o from",
  "import .\n", "", "o relativo e so com from: from .modulo import nome", 2 },
/* Palavra de nomes TROCADA entre as formas: `from m import x` / `PUSH m GET x`.
 * Antes: `from m GET *` dizia so "esperado 'import'"; `PUSH m import A` virava
 * OUTRO statement e dava "No module named 'A'"; `import m GET A` dava NameError
 * no `GET`. As tres agora dizem de qual forma e a palavra. */
{ "from m GET *: GET e do PUSH, e a frase guarda o *",
  "from './m.pr' GET *\n", "",
  "`GET` e do PUSH; no from os nomes vem depois de `import`: from m import * — ou PUSH m GET *", 2 },
{ "from m GET nomes: a frase sugere com nome",
  "from m GET A, B\n", "", "from m import nome — ou PUSH m GET nome", 2 },
{ "PUSH m import A: nao vira outro statement nem ImportError de 'A'",
  "PUSH m import A\n", "",
  "`import` e do from; no PUSH os nomes vem depois de `GET`: PUSH m GET nome — ou from m import nome", 2 },
{ "PUSH m as x import *",
  "PUSH m as x import *\n", "", "PUSH m GET * — ou from m import *", 2 },
{ "import m GET A: nao vira NameError no GET",
  "import m GET A\n", "",
  "`GET` e do PUSH; `import m` liga o modulo inteiro. Pra trazer nomes: PUSH m GET nome — ou from m import nome", 2 },
{ "import m GET *",
  "import m GET *\n", "", "Pra trazer nomes: PUSH m GET * — ou from m import *", 2 },
{ "from .m import x continua valendo (o relativo com from nao mudou)",
  "from .nao_existe_mesmo import x\n", "", "ImportError: No module named '.nao_existe_mesmo'", 1 },
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
/* `int funct`/`bool funct` devolvem o sentinela (500 / False) — e o erro SAI
 * JUNTO no stderr. Antes o `try` implicito descartava a mensagem e o erro
 * sumia calado: "o int nao pode engolir erro, o 500 sai junto". */
{ "int funct que falha devolve 500 e o erro sai junto",
  "int funct conta() {\n    raise ValueError(\"falhei\")\n}\npost(conta())\n",
  "500", "int funct conta() devolveu 500 no lugar do erro", 0 },
{ "bool funct que falha devolve False e o erro sai junto",
  "bool funct valida() {\n    raise ValueError(\"falhei\")\n}\npost(valida())\n",
  "False", "bool funct valida() devolveu False no lugar do erro", 0 },
{ "o erro engolido mostra a linha do fonte onde aconteceu",
  "funct f() {\n    x = 1 / 0\n}\nint funct g() {\n    f()\n    return 1\n}\npost(g())\n",
  "500", "  |     x = 1 / 0\n", 0 },
/* O corpo da lambda passada como ARGUMENTO herdava o "estou dentro de um
 * grupo" da chamada, e `if x == "" {` lá dentro virava a string `""`
 * interpolando `{...}`: "faltou '}' na interpolacao" em código válido. A mesma
 * lambda numa variável passava. */
{ "lambda como argumento com if de string vazia",
  "funct g(cb) {\n    return cb(\"\")\n}\n"
  "post(g(funct(x) {\n    if x == \"\" {\n        return \"vazio\"\n    }\n    return \"cheio\"\n}))\n",
  "vazio", NULL, 0 },
{ "lambda como argumento ainda interpola string com {}",
  "h = funct(cb) { return cb(4) }\n"
  "post(h(funct(v) {\n    return \"x=\" {v}\n}))\n", "x=4", NULL, 0 },
/* Parêntese esquecido aberto: rodando, o erro sai no `(` — a linha de baixo
 * é a declaração seguinte engolida, não um parâmetro com nome errado. */
{ "parentese aberto na declaracao culpa o parentese",
  "funct f(\nint funct depois(int a) {\n    return a\n}\n", "",
  "parentese '(' aberto nao foi fechado", 2 },
{ "parentese aberto na lambda culpa o parentese",
  "g = funct(\nint funct depois(int a) {\n    return a\n}\n", "",
  "parentese '(' aberto nao foi fechado", 2 },
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
  "Entity Mat {\n    static funct soma(a, b) {\n        return a + b\n    }\n}\n"
  "post(Mat.soma(1, 2))\n", "3", NULL, 0 },
{ "static colado: pela instancia e erro, e a mensagem fala em static",
  "Entity Mat {\n    static funct soma(a, b) {\n        return a + b\n    }\n}\n"
  "m = Mat()\nm.soma(1, 2)\n",
  "", "RuntimeError: funct 'soma' e static: chame pela Entity (Tipo.soma(...)), nao pela instancia", 1 },
{ "nonnull colado: valor passa, Null nao",
  "nonnull funct e(v) {\n    return v\n}\npost(e(5))\ne(Null)\n",
  "5", "nonnull: parametro 'v' em 'e' nao pode ser Null", 1 },
{ "modificadores colados em qualquer ordem: private static int funct",
  /* lido de dentro da classe: de fora o private barra (o caso seguinte do
   * `static private funct`) */
  "Entity K {\n    private static int funct tres() {\n        return 3\n    }\n"
  "    static int funct usa() {\n        return tres()\n    }\n}\n"
  "post(K.usa())\n", "3", NULL, 0 },
{ "modificadores colados em qualquer ordem: static nonnull funct",
  "Entity K {\n    static nonnull funct eco(x) {\n        return x\n    }\n}\n"
  "post(K.eco(\"ok\"))\n", "ok", NULL, 0 },
{ "modificadores colados em qualquer ordem: nonnull async funct",
  "nonnull async funct f(v) {\n    return v\n}\npost(gather(f(7))[0])\n", "7", NULL, 0 },
{ "static colado tambem vale fora de Entity (marca inofensiva)",
  "static funct f() {\n    return 1\n}\npost(f())\n", "1", NULL, 0 },
/* O que `App.x` alcanca de fora, `x` alcanca de dentro da classe: campo E
 * metodo static, da classe e dos pais, em metodo comum ou static. So o campo
 * entrava; `s()` solto passava no --check e dava NameError rodando. */
{ "metodo static pelo nome solto, de metodo comum e de metodo static",
  "class A {\n    public static int total = 3\n    static funct s() {\n        return 1\n    }\n"
  "    funct n(self) {\n        return s() + total\n    }\n    static funct t() {\n        return s()\n    }\n}\n"
  "post(A().n(), A.t())\n", "4 1", NULL, 0 },
{ "static do PAI pelo nome solto na classe filha (campo e metodo)",
  "class Pai {\n    public static int total = 7\n    static funct s() {\n        return 2\n    }\n}\n"
  "class Filha(Pai) {\n    funct f(self) {\n        return total + s()\n    }\n}\npost(Filha().f())\n",
  "9", NULL, 0 },
{ "static solto: a aridade do metodo e conferida antes de rodar, como em A.s()",
  "class A {\n    static funct s(a) {\n        return a\n    }\n    funct n(self) {\n        return s(1, 2)\n    }\n}\n",
  "", "TypeError: s() takes 1 positional argument but 2 were given", 2 },
{ "metodo COMUM pelo nome solto nao existe: NameError antes de rodar (era so rodando)",
  "class A {\n    funct m(self) {\n        return 1\n    }\n    funct n(self) {\n        return m()\n    }\n}\npost(A().n())\n",
  "", "NameError: name 'm' is not defined. Did you mean: 'self.m'?", 2 },
{ "parametro com o mesmo nome ganha do static (o nome solto e o parametro)",
  "class A {\n    public static int total = 1\n    funct n(self, total) {\n        return total\n    }\n}\npost(A().n(\"p\"))\n",
  "p", NULL, 0 },
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
  "Entity A {\n    public int static funct r(n) {\n        return n\n    }\n}\npost(A.r(2))\n",
  "2", NULL, 0 },
{ "Entity: visibilidade DEPOIS do modificador (`static public funct`)",
  "Entity A {\n    static public funct b(n) {\n        return n\n    }\n}\npost(A.b(3))\n",
  "3", NULL, 0 },
{ "Entity: `int static funct` sem visibilidade nenhuma",
  "Entity A {\n    int static funct d(n) {\n        return n\n    }\n}\npost(A.d(4))\n",
  "4", NULL, 0 },
{ "Entity: `static private funct` mantem o private lido na cabeca",
  "Entity A {\n    static private funct p(n) {\n        return n\n    }\n}\nx = A()\nx.p(1)\n",
  "", "acesso negado: 'p' e private de A (so acessivel de dentro da classe)", 2 },
{ "Entity: campo `int y = 2` continua campo ao lado de funct com modificador",
  "Entity A {\n    static funct m(n) {\n        return n\n    }\n    int y = 2\n}\n"
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
  "long d = \"texto\"\n",
  "", "AttributedValueError: variável d esperava long, recebeu str", 2 },
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
  "Entity A {\n    action m(self) {\n        return 1\n    }\n}\n",
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
  "Entity M {\n    @static\n    funct velha(a) {\n        return a\n    }\n"
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
  "class P {\n    funct __init__(self, n) {\n        self.n = n\n    }\n}\n"
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
  "public class C {\n    public static string funct main() {\n        post(\"rodou\")\n        return \"rodou\"\n    }\n}\n"
  "C.main()\n",
  "rodou", NULL, 0 },
{ "static colado registra o metodo como estatico",
  "class C {\n    static funct m() {\n        post(\"ok\")\n    }\n}\nC.m()\n", "ok", NULL, 0 },
{ "static continua valendo depois do tipo",
  "class C {\n    str static funct m() {\n        post(\"ok\")\n        return \"ok\"\n    }\n}\nC.m()\n",
  "ok", NULL, 0 },

/* ── campo `static`, 2026-09-08 ────────────────────────────────────────────
 * Campo de classe: avaliado UMA vez na declaracao, lido como `Classe.x`, por
 * nome solto dentro do corpo e dos metodos da propria classe, e por
 * `self.x` na instancia. Sem ele, `App.mapp` nao existia e um metodo static
 * nao tinha como enxergar o campo — o arquivo dele rodava sem fazer nada. */
{ "campo static: Classe.x e nome solto em metodo static",
  "class C {\n    static int x = 7\n    static funct le(){ return x }\n}\npost(C.x, C.le())\n", "7 7", NULL, 0 },
{ "campo static: modificadores em qualquer ordem",
  /* o private lido de dentro da classe: de fora ele barra */
  "class C {\n    static public int x = 7\n    private static int y = 8\n"
  "    static int funct le_y() {\n        return y\n    }\n}\npost(C.x, C.le_y())\n", "7 8", NULL, 0 },
{ "campo static sem inicializador nasce Null",
  "class C {\n    static int x\n}\npost(C.x)\n", "Null", NULL, 0 },
{ "campo static e UM so: instancias leem e a reescrita e da classe",
  "class K {\n    static int n = 0\n    funct inc(self){ K.n = K.n + 1 }\n}\na = K()\nb = K()\na.inc()\nb.inc()\npost(K.n, a.n, b.n)\n", "2 2 2", NULL, 0 },
/* Parametro com o mesmo nome GANHA do campo static; o herdado le-se por
 * `self.`/`Classe.` (o nome solto cobre os static da propria classe). */
{ "campo static: local ganha, heranca por self. e Classe.",
  "class B {\n    static int fundo = 1\n}\nclass C(B){\n    static int x = 7\n"
  "    funct m(self, x){ return x }\n    funct s(self){ return self.x + self.fundo }\n"
  "    static funct t(){ return x + C.fundo }\n}\nc = C()\npost(c.m(99), c.s(), C.fundo, C.t())\n",
  "99 8 1 8", NULL, 0 },
{ "campo static nao entra no __init__ sintetizado",
  "class P {\n    static int total = 0\n    str nome\n    int idade = 3\n}\np = P(\"ana\")\npost(p.nome, p.idade, P.total)\n",
  "ana 3 0", NULL, 0 },
/* ── decorador `@obj.metodo()` DENTRO da classe, 2026-09-08 ────────────────
 * Era descartado pelo compilador: o metodo compilava sem registro nenhum e a
 * rota nunca existia, calada. Agora vale nas tres posicoes — funct solta, em
 * cima da classe e em cima do metodo — pelo MESMO protocolo (register). */
{ "decorador @obj.m() nas tres posicoes: funct, classe, metodo",
  "class Reg {\n    funct __init__(self){ self.v = [] }\n"
  "    funct rota(self, c){ self.c = c\n        return self }\n"
  "    funct register(self, h){ addEnd(self.v, self.c)\n        return h }\n}\nr = Reg()\n"
  "@r.rota(\"/funct\")\nfunct f(){ return 1 }\n"
  "@r.rota(\"/classe\")\nclass H {\n    funct handler(self){ return 1 }\n}\n"
  "class D {\n    @r.rota(\"/dentro\")\n    static funct h(){ return 1 }\n"
  "    @r.rota(\"/inst\")\n    funct i(self){ return 1 }\n}\npost(r.v)\n",
  "['/funct', '/classe', '/dentro', '/inst']", NULL, 0 },
/* motor-33: o `--check` so conferia o primeiro nome do decorador. O membro
 * que nao existe (`@app.rota("/x")`, `@os.naoexiste`) e a chamada sem o
 * argumento que a funct pede (`@log()`) passavam no `--check` e quebravam
 * rodando. A expressao do decorador agora e a arvore de qualquer expressao:
 * a mesma frase antes de rodar e rodando (pelo Object), na mesma linha, e a
 * coluna e a do `.` e a do `(`, como em `os.naoexiste` e `log()` (a coluna
 * antes de rodar, pelo JSON do --check, fica no teste/cli_roda.pr). */
{ "motor-33: decorador com membro que nao existe, antes de rodar",
  "import os\n@os.naoexiste\nfunct r() {\n    return 1\n}\n",
  "", "AttributeError: module 'os' has no attribute 'naoexiste'", 2 },
{ "motor-33: decorador chamado sem o argumento que a funct pede, antes de rodar",
  "funct log(f) {\n    return f\n}\n@log()\nfunct r() {\n    return 1\n}\n",
  "", "TypeError: log() missing 1 required positional argument: 'f'", 2 },
{ "motor-33: o membro do decorador rodando (pelo Object), no mesmo lugar",
  "import os\nObject m = os\n@m.naoexiste\nfunct r() {\n    return 1\n}\n",
  "", "AttributeError: module 'os' has no attribute 'naoexiste'", 1 },
{ "motor-33: o membro do decorador rodando aponta o `.`",
  "import os\nObject m = os\n@m.naoexiste\nfunct r() {\n    return 1\n}\n",
  "", ", linha 3\n  | @m.naoexiste\n  |   ^^^", 1 },
{ "motor-33: a chamada do decorador rodando (pelo Object), com a frase de antes de rodar",
  "funct log(f) {\n    return f\n}\nObject g = log\n@g()\nfunct r() {\n    return 1\n}\n",
  "", "TypeError: log() missing 1 required positional argument: 'f'", 1 },
{ "motor-33: a chamada do decorador rodando aponta o `(`",
  "funct log(f) {\n    return f\n}\nObject g = log\n@g()\nfunct r() {\n    return 1\n}\n",
  "", ", linha 5\n  | @g()\n  |   ^^^", 1 },
{ "motor-33: o decorador que existe continua valendo, com e sem parenteses",
  "funct log(f) {\n    return f\n}\nfunct fab(str p) {\n    return log\n}\n"
  "@log\nfunct r() {\n    return \"r\"\n}\n@fab(\"a\")\nfunct s() {\n    return \"s\"\n}\npost(r(), s())\n",
  "r s", NULL, 0 },
/* O arquivo dele: campo static + decorador no corpo usando o campo. */
{ "decorador no corpo da classe le campo static da propria classe",
  "class Reg {\n    funct __init__(self){ self.v = [] }\n"
  "    funct rota(self, c){ self.c = c\n        return self }\n"
  "    funct register(self, h){ addEnd(self.v, self.c)\n        return h }\n}\n"
  "public class App {\n    public static object mapp = Reg()\n"
  "    @mapp.rota(\"/opa\")\n    public string static funct handler(data){ return \"ok\" }\n}\n"
  "post(App.mapp.v)\n",
  "['/opa']", NULL, 0 },
/* Sem o `static` o decorador nao tem como ler o campo — e a mensagem diz o
 * que falta, em vez de um NameError apontando pra linha da classe. */
{ "decorador no corpo lendo campo de INSTANCIA e recusado com a palavra certa",
  "class Reg {\n    funct rota(self, c){ return self }\n}\n"
  "class App {\n    object mapp = Reg()\n    @mapp.rota(\"/x\")\n    static funct h(){ return 1 }\n}\n",
  "", "campo de instancia", 2 },
/* A ordem dos modificadores do CAMPO e de quem escreve, como na cabeca de
 * funct. `private object static nome` era lido com `static` como o NOME do
 * campo: virava campo de instancia chamado `static`, calado, e `C.nome` nao
 * existia. As dez ordens abaixo tem que dar o mesmo. */
{ "campo static: modificador antes OU depois do tipo, qualquer ordem",
  "class C {\n"
  "    static int a = 1\n"
  "    private static int b = 2\n"
  "    static private int c = 3\n"
  "    private int static d = 4\n"
  "    int static e = 5\n"
  "    int private static f = 6\n"
  "    static int private g = 7\n"
  "    int static private h = 8\n"
  "    static public i = 9\n"
  "    static list funct todos() {\n        return [a, b, c, d, e, f, g, h, i]\n    }\n"
  "}\npost(*C.todos())\n",
  "1 2 3 4 5 6 7 8 9", NULL, 0 },
{ "funct: modificador depois do tipo continua valendo",
  "class C {\n    int private static funct m(){ return 1 }\n"
  "    static int funct usa() {\n        return m()\n    }\n}\npost(C.usa())\n",
  "1", NULL, 0 },

/* ── private e static: a declaração que o nome alcança (Plano 6, lote 4) ───
 * "De dentro" é o código ESCRITO no corpo da classe — método, lambda e funct
 * aninhada nele, inicializador de static, decorador do corpo —, não o proto
 * que roda. A primeira classe, na ordem de busca de membro, que declara o
 * nome decide: pública libera; private só libera o código escrito numa classe
 * que o declara, com o valor descendendo dela. O static tem UM dono, e toda
 * grafia que o lê grava nele, com o tipo conferido. */
{ "private static: de fora pela classe e o mesmo acesso negado, antes de rodar",
  "class C {\n    private static int b = 2\n}\nC.b = 20\npost(C.b)\n",
  "", "RuntimeError: acesso negado: 'b' e private de C (so acessivel de dentro da classe)", 2 },
{ "private static: chamar o metodo private pela classe tambem e negado",
  "class C {\n    private static int funct s() {\n        return 5\n    }\n}\npost(C.s())\n",
  "", "RuntimeError: acesso negado: 's' e private de C (so acessivel de dentro da classe)", 2 },
{ "private static: de dentro vale (metodo, lambda, inicializador do corpo)",
  "class C {\n    private static int b = 2\n    private static int c = C.b + 1\n"
  "    private static int funct s() {\n        return 5\n    }\n"
  "    static int funct soma() {\n        C.b = C.b + 1\n"
  "        Object g = int funct() {\n            return C.b + b\n        }\n"
  "        return g() + C.s() + s() + c\n    }\n}\npost(C.soma())\n",
  "19", NULL, 0 },
{ "private static do pai pelo nome solto na filha e negado antes de rodar",
  "class Pai {\n    private static int x = 7\n}\nclass Filha(Pai) {\n"
  "    int funct solto(self) {\n        return x\n    }\n}\nFilha f = Filha()\npost(f.solto())\n",
  "", "RuntimeError: acesso negado: 'x' e private de Pai (so acessivel de dentro da classe)", 2 },
{ "decorador do corpo le o registrador private static da propria classe",
  "class Reg {\n    list v = []\n    str c = \"\"\n"
  "    Reg funct rota(self, str c) {\n        self.c = c\n        return self\n    }\n"
  "    funct register(self, h) {\n        addEnd(self.v, self.c)\n        return h\n    }\n}\n"
  "class App {\n    private static Reg mapp = Reg()\n    @mapp.rota(\"/opa\")\n"
  "    static str funct handler(str data) {\n        return \"ok\"\n    }\n"
  "    static list funct rotas() {\n        return mapp.v\n    }\n}\npost(App.rotas())\n",
  "['/opa']", NULL, 0 },
{ "lambda escrita no metodo alcanca o private da classe",
  "class C {\n    private list itens = [3, 1, 2]\n"
  "    private int funct dobra(self, int v) {\n        return v * 2\n    }\n"
  "    list funct dobrados(self) {\n"
  "        return map(self.itens, int funct(int v) { return self.dobra(v) })\n    }\n}\n"
  "C c = C()\npost(c.dobrados())\n",
  "[6, 2, 4]", NULL, 0 },
{ "funct aninhada no metodo le e grava o private da classe",
  "class C {\n    private int x = 7\n    int funct dobra(self) {\n"
  "        int funct le() {\n            return self.x\n        }\n"
  "        Null funct grava(int v) {\n            self.x = v\n        }\n"
  "        grava(le() * 2)\n        return self.x\n    }\n}\nC c = C()\npost(c.dobra())\n",
  "14", NULL, 0 },
{ "o private do pai nao barra o publico de mesmo nome da filha",
  "class Pai {\n    private int x = 1\n    private str funct nome(self) {\n        return \"pai\"\n    }\n}\n"
  "class Filha(Pai) {\n    int x = 2\n    str funct nome(self) {\n        return \"filha\"\n    }\n}\n"
  "Filha f = Filha()\npost(f.x, f.nome())\n",
  "2 filha", NULL, 0 },
{ "o private do avo nao barra o publico da neta",
  "class A {\n    private str funct nome(self) {\n        return \"avo\"\n    }\n}\nclass B(A) {\n}\n"
  "class C(B) {\n    str funct nome(self) {\n        return \"neta\"\n    }\n}\nC c = C()\npost(c.nome())\n",
  "neta", NULL, 0 },
{ "a frase do acesso negado cita a classe que declara o private",
  "class Pai {\n    private int segredo = 42\n}\nclass Filha(Pai) {\n"
  "    int funct le(self) {\n        return self.segredo\n    }\n}\nFilha f = Filha()\npost(f.le())\n",
  "", "RuntimeError: acesso negado: 'segredo' e private de Pai (so acessivel de dentro da classe)", 2 },
{ "dois pais: o primeiro que declara o nome decide (publico)",
  "class A {\n    private int x = 1\n}\nclass B {\n    int x = 2\n}\nclass C(B, A) {\n}\n"
  "C c = C(5)\npost(c.x)\n",
  "5", NULL, 0 },
{ "dois pais: o primeiro que declara o nome decide (private)",
  "class A {\n    private int x = 1\n}\nclass B {\n    int x = 2\n}\nclass D(A, B) {\n}\n"
  "D d = D(6)\npost(d.x)\n",
  "", "RuntimeError: acesso negado: 'x' e private de A (so acessivel de dentro da classe)", 2 },
{ "metodo do pai le o private dele numa instancia da filha que tambem o declara",
  "class P {\n    private int s = 1\n    int funct mostra(self) {\n        return self.s\n    }\n}\n"
  "class F(P) {\n    private int s = 5\n    int funct mostra_f(self) {\n        return self.s\n    }\n}\n"
  "F f = F()\npost(f.mostra(), f.mostra_f())\n",
  "5 5", NULL, 0 },
{ "self.static += 1 conta no static da classe, nao numa copia por objeto",
  "class Conta {\n    static int total = 0\n    str nome\n"
  "    funct __init__(self, str nome) {\n        self.nome = nome\n        self.total += 1\n    }\n}\n"
  "Conta a = Conta(\"a\")\nConta b = Conta(\"b\")\nConta c = Conta(\"c\")\n"
  "post(Conta.total, a.total, b.total, c.total)\n",
  "3 3 3 3", NULL, 0 },
{ "obj.x = v de fora grava no static da classe",
  "class K {\n    static int n = 0\n}\nK a = K()\nK b = K()\na.n = 5\npost(K.n, a.n, b.n)\n"
  "K.n = 9\npost(K.n, a.n, b.n)\n",
  "5 5 5\n9 9 9", NULL, 0 },
{ "Filha.x = v grava no static do pai, que e o que Filha.x le",
  "class Pai {\n    static int total = 1\n}\nclass Filha(Pai) {\n}\npost(Pai.total, Filha.total)\n"
  "Filha.total = 50\npost(Pai.total, Filha.total)\nPai.total = 7\npost(Pai.total, Filha.total)\n",
  "1 1\n50 50\n7 7", NULL, 0 },
{ "a filha que redeclara o static tem o dela",
  "class Pai {\n    static int k = 1\n}\nclass Filha(Pai) {\n    static int k = 2\n}\n"
  "Filha.k = 20\npost(Pai.k, Filha.k)\nFilha f = Filha()\nf.k = 30\npost(Pai.k, Filha.k, f.k)\n",
  "1 20\n1 30 30", NULL, 0 },
{ "static e campo de instancia de mesmo nome na classe: membro repetido (Plano 6, lote 5)",
  "class K {\n    static int n = 1\n    int n = 2\n}\nK a = K()\npost(K.n, a.n)\n",
  "", "SyntaxError: membro 'n' repetido na classe K (o primeiro esta na linha 2)", 2 },
{ "static tipado: escrita pela classe com outro tipo e recusada antes de rodar",
  "class K {\n    static int n = 0\n}\nK.n = \"texto\"\npost(K.n, type(K.n))\n",
  "", "AttributedValueError: campo n de K esperava int, recebeu str", 2 },
{ "static tipado: pela filha, a frase cita a classe que declara",
  "class K {\n    static int n = 0\n}\nclass F(K) {\n}\nF.n = \"pela filha\"\npost(K.n)\n",
  "", "AttributedValueError: campo n de K esperava int, recebeu str", 2 },
{ "static tipado: o valor que so se sabe rodando e conferido rodando",
  "list l = [\"texto\"]\nclass K {\n    static int n = 0\n}\nK.n = l[0]\npost(K.n, type(K.n))\n",
  "", "AttributedValueError: campo n de K esperava int, recebeu str", 1 },
{ "static tipado: o inicializador tambem e conferido rodando",
  "list l = [\"texto\"]\nclass K {\n    static int n = l[0]\n}\npost(K.n, type(K.n))\n",
  "", "AttributedValueError: campo n de K esperava int, recebeu str", 1 },
{ "static tipado: o tipo vale na leitura pela classe",
  "class K {\n    static int n = 0\n}\nstr s = K.n\npost(s)\n",
  "", "AttributedValueError: variável s esperava str, recebeu int", 2 },
{ "static sem inicializador nasce Null e recebe o tipo dele depois",
  "class K {\n    static int n\n}\npost(K.n, type(K.n))\nK.n = 3\npost(K.n, type(K.n))\n",
  "Null Null\n3 int", NULL, 0 },
{ "base().metodo static do pai chama sem instancia",
  "class A {\n    static int funct dobro(int n) {\n        return n * 2\n    }\n}\n"
  "class B(A) {\n    int funct f(self) {\n        return base().dobro(4)\n    }\n}\nB b = B()\npost(b.f())\n",
  "8", NULL, 0 },
{ "classe declarada dentro de funct: o static solto acha o campo",
  "int funct usa() {\n    class Local {\n        static int n = 5\n"
  "        static int funct le() {\n            return n\n        }\n    }\n"
  "    return Local.le()\n}\npost(usa())\n",
  "5", NULL, 0 },
{ "classe declarada dentro de funct: o metodo enxerga a funct (escopo lexico)",
  "int funct usa() {\n    int k = 5\n    class Local {\n"
  "        static int funct le() {\n            return k\n        }\n"
  "        int funct m(self) {\n            return k + 1\n        }\n    }\n"
  "    Local l = Local()\n    return Local.le() + l.m()\n}\npost(usa())\n",
  "11", NULL, 0 },
{ "lambda na funct enxerga a classe e o enum declarados nela",
  "list funct f() {\n    class L {\n        int x = 1\n    }\n    enum Cor {\n        RED, GREEN\n    }\n"
  "    Object g = funct() {\n        return [L().x, Cor.GREEN]\n    }\n    return g()\n}\npost(f())\n",
  "[1, 1]", NULL, 0 },
{ "private __init__: construir de fora e negado antes de rodar",
  "class C {\n    private funct __init__(self) {\n        post(\"criou\")\n    }\n}\nC b = C()\n",
  "", "RuntimeError: acesso negado: '__init__' e private de C (so acessivel de dentro da classe)", 2 },
{ "private __init__: a fabrica de dentro da classe constroi",
  "class C {\n    int v = 0\n    private funct __init__(self, int v) {\n        self.v = v\n    }\n"
  "    static C funct cria(int v) {\n        return C(v)\n    }\n}\npost(C.cria(4).v)\n",
  "4", NULL, 0 },

/* ── classes: construtor, campos, enum, `is` e redeclaração (Plano 6, lote 5) ──
 * O `is` com classe sobe pela herança e classe com classe é identidade; o
 * nome de tipo à direita segue a regra da declaração. Uma regra decide o
 * construtor (o `--check` e a VM). Campo com valor nasce sempre, antes do
 * `__init__`, uma vez por objeto; o sem valor do pai, lido sem ninguém o
 * gravar, é AttributeError com a causa. Redeclaração, membro repetido e
 * sobrescrita que troca o retorno são erro antes de rodar. Enum como tipo. */
{ "is sobe pela heranca e classe com classe e identidade",
  "class Animal {\n}\nclass Cao(Animal) {\n}\nCao c = Cao()\npost(c is Cao, c is Animal, Animal() is Cao)\n"
  "post(Cao is Cao, Cao is Animal)\n",
  "True True False\nTrue False", NULL, 0 },
{ "is com nome de tipo declaravel segue a regra da declaracao",
  "int a = 1\npost(a is Object, a is int, a is str, \"x\" is String)\npost(a not is str)\n",
  "True True False True\nTrue", NULL, 0 },
{ "todo tipo e Object, como todo tipo e type (17.0.17)",
  "Object k = int\npost(int is Object, ValueError is Object, k is Object, Null is Object, int is type)\n",
  "True True True False True", NULL, 0 },
/* `f = tup; f(x)` recusava ("tipo 'tup' nao pode ser usado como conversor")
 * e `tup(x)` convertia: a tabela medida dos nativos, que chama pelo valor,
 * gravou a recusa, e o --check passou a recusar `tup(tup(x))`. */
{ "tipo como valor converte como a chamada direta: tup e dict tambem (17.0.17)",
  "Object f = tup\nObject g = dict\npost(f([1, 2]), g({\"a\": 1}), tup(tup([0])) == (0,), dict(dict({\"b\": 2})))\n",
  "(1, 2) {'a': 1} True {'b': 2}", NULL, 0 },
{ "is Object com instancia",
  "class A {\n}\nA a = A()\npost(a is Object)\n", "True", NULL, 0 },
{ "is com enum: o valor de um dos membros",
  "enum Cor {\n    RED,\n    GREEN\n}\npost(Cor.RED is Cor, 1 is Cor, 5 is Cor)\n",
  "True True False", NULL, 0 },
{ "construtor: Filha() com campo e sem __init__ usa o gerado dela, nao o __init__ do pai",
  "class Pai {\n    funct __init__(self, int x) {\n        self.x = x * 10\n    }\n}\n"
  "class Filha(Pai) {\n    str nome = \"f\"\n}\nFilha f = Filha()\npost(f.nome)\n",
  "f", NULL, 0 },
{ "construtor gerado herdado do pai e conferido antes de rodar",
  "class Pai {\n    int x\n}\nclass Filha(Pai) {\n}\nFilha f = Filha(\"texto\")\npost(f.x)\n",
  "", "AttributedValueError: campo x de Filha esperava int, recebeu str", 2 },
{ "campo com valor do pai nasce na filha que tem campos proprios",
  "class Pai {\n    int x = 1\n}\nclass Filha(Pai) {\n    str y = \"oi\"\n}\nFilha f = Filha()\npost(f.y)\npost(f.x)\n",
  "oi\n1", NULL, 0 },
{ "campo com valor existe com __init__ escrito",
  "class C {\n    int x = 5\n    funct __init__(self) {\n        post(\"init\")\n    }\n"
  "    int funct le(self) {\n        return self.x\n    }\n}\nC c = C()\npost(c.le())\n",
  "init\n5", NULL, 0 },
{ "base().__init__ com pai sem construtor (static nao gera um): erro antes de rodar",
  "class A {\n    static int k = 1\n}\nclass B(A) {\n    funct __init__(self) {\n        base().__init__()\n    }\n}\n"
  "B b = B()\npost(\"ok\")\n",
  "", "TypeError: base(): a Entity pai 'A' nao tem __init__", 2 },
{ "campo do pai sem valor lido na filha: AttributeError antes de rodar, com a causa",
  "class Pai {\n    int x\n}\nclass Filha(Pai) {\n    str y = \"oi\"\n}\nFilha f = Filha()\npost(f.y)\npost(f.x)\n",
  "", "AttributeError: 'Filha' object has no attribute 'x': o campo x de Pai foi declarado sem valor e nada o "
      "gravou (de um valor na declaracao ou grave self.x no __init__)", 2 },
{ "campo do pai sem valor com __init__ escrito na filha: antes de rodar tambem",
  "class Pai {\n    int x\n}\nclass Filha(Pai) {\n    str y\n    funct __init__(self, str y) {\n        self.y = y\n    }\n}\n"
  "Filha f = Filha(\"a\")\npost(f.x)\n",
  "", "AttributeError: 'Filha' object has no attribute 'x': o campo x de Pai foi declarado sem valor e nada o "
      "gravou (de um valor na declaracao ou grave self.x no __init__)", 2 },
{ "campo do pai sem valor gravado de fora: vale",
  "class Pai {\n    int x\n}\nclass Filha(Pai) {\n    str y = \"oi\"\n}\nFilha f = Filha()\nf.x = 3\npost(f.x)\n",
  "3", NULL, 0 },
{ "campo do pai sem valor gravado pelo construtor do pai via base: vale",
  "class Pai {\n    int x\n}\nclass Filha(Pai) {\n    funct __init__(self) {\n        base(Pai).__init__(7)\n    }\n}\n"
  "Filha f = Filha()\npost(f.x)\n",
  "7", NULL, 0 },
{ "campo sem valor lido no metodo do pai: o Pai() o recebe, sem erro",
  "class Pai {\n    int x\n    int funct dobro(self) {\n        return self.x * 2\n    }\n}\nclass Filha(Pai) {\n    str y = \"oi\"\n}\n"
  "Pai p = Pai(4)\npost(p.dobro())\nFilha f = Filha()\npost(f.y)\n",
  "8\noi", NULL, 0 },
{ "campo sem valor rodando (receptor que o checador nao tipa): a mesma frase",
  "class Pai {\n    int x\n}\nclass Filha(Pai) {\n    str y = \"a\"\n}\nlist l = [Filha()]\npost(l[0].x)\n",
  "", "AttributeError: 'Filha' object has no attribute 'x': o campo x de Pai foi declarado sem valor e nada o "
      "gravou (de um valor na declaracao ou grave self.x no __init__)", 1 },
{ "valor padrao roda uma vez por objeto, antes do __init__; o argumento passado ganha",
  "int cont = 0\nint funct proximo() {\n    cont += 1\n    return cont\n}\nclass C {\n    str nome\n    int id = proximo()\n}\n"
  "C a = C(\"x\")\npost(a.id, cont)\nC b = C(nome=\"y\")\npost(b.id, cont)\nC d = C(\"z\", 50)\npost(d.id, cont)\n",
  "1 1\n2 2\n50 3", NULL, 0 },
{ "valor padrao com a Entity como valor (map)",
  "int cont = 0\nint funct proximo() {\n    cont += 1\n    return cont\n}\nclass C {\n    str nome\n    int id = proximo()\n}\n"
  "list l = list(map([\"p\", \"q\"], C))\npost(l[0].id, l[1].id, cont)\n",
  "1 2 2", NULL, 0 },
{ "valor padrao de campo em classe declarada dentro de funct enxerga a funct",
  "funct fab(int b0) {\n    class L {\n        int v = b0 * 2\n        int funct dobro(self) {\n            return self.v * 2\n        }\n    }\n"
  "    return L()\n}\npost(fab(5).v, fab(5).dobro())\n",
  "10 20", NULL, 0 },
{ "valor padrao na instancia que o registrador de metodo recebe",
  "class Reg {\n    list fs = []\n    funct register(self, Object f) {\n        self.fs.append(f)\n    }\n}\nReg r = Reg()\n"
  "class C {\n    int base0 = 5\n    @r\n    int funct m(self) {\n        return self.base0\n    }\n}\npost(r.fs[0]())\n",
  "5", NULL, 0 },
{ "class declarada duas vezes no mesmo escopo: erro na segunda",
  "class C {\n    int x = 1\n}\nC a = C()\npost(a.x)\nclass C {\n    str y = \"a\"\n}\n",
  "", "SyntaxError: 'C' ja foi declarado na linha 1 (class): um nome, uma declaracao no mesmo escopo", 2 },
{ "class em cada ramo do if/else: escopos diferentes, vale",
  "bool sim = True\nif sim {\n    class C {\n        int x = 1\n    }\n    post(C().x)\n} else {\n    class C {\n        int x = 2\n    }\n"
  "    post(C().x)\n}\n",
  "1", NULL, 0 },
{ "enum com o nome de uma class da mesma funct: erro",
  "funct f() {\n    class L {\n        int a = 1\n    }\n    enum L {\n        A\n    }\n}\n",
  "", "SyntaxError: 'L' ja foi declarado na linha 2 (class): um nome, uma declaracao no mesmo escopo", 2 },
{ "model declarado duas vezes: erro",
  "model M() {\n    nome: str\n}\nmodel M() {\n    idade: int\n}\n",
  "", "SyntaxError: 'M' ja foi declarado na linha 1 (model): um nome, uma declaracao no mesmo escopo", 2 },
{ "class com o nome de uma funct: erro de redeclaracao",
  "funct C() {\n    return 1\n}\nclass C {\n    int x = 1\n}\n",
  "", "SyntaxError: 'C' ja foi declarado na linha 1 (funct): um nome, uma declaracao no mesmo escopo", 2 },
{ "funct declarada de novo continua valendo a ultima",
  "funct f() {\n    return 1\n}\nfunct f() {\n    return 2\n}\npost(f())\n", "2", NULL, 0 },
{ "metodo repetido na classe: erro no segundo",
  "class C {\n    funct oi(self) {\n        post(\"1\")\n    }\n    funct oi(self) {\n        post(\"2\")\n    }\n}\nC().oi()\n",
  "", "SyntaxError: membro 'oi' repetido na classe C (o primeiro esta na linha 2)", 2 },
{ "campo repetido na classe: erro no segundo",
  "class C {\n    int x = 1\n    int x = 2\n}\nC c = C()\npost(c.x)\n",
  "", "SyntaxError: membro 'x' repetido na classe C (o primeiro esta na linha 2)", 2 },
{ "campo e metodo de mesmo nome na classe: erro",
  "class C {\n    int x = 1\n    int funct x(self) {\n        return 2\n    }\n}\n",
  "", "SyntaxError: membro 'x' repetido na classe C (o primeiro esta na linha 2)", 2 },
{ "campo repetido no model: erro",
  "model M() {\n    nome: str\n    nome: int\n}\n",
  "", "SyntaxError: campo 'nome' repetido no model M (o primeiro esta na linha 2)", 2 },
{ "membro repetido no enum: erro",
  "enum E {\n    A,\n    B,\n    A\n}\n",
  "", "SyntaxError: membro 'A' repetido no enum E (o primeiro esta na linha 2)", 2 },
{ "sobrescrita que troca o tipo de retorno: erro na filha",
  "class Pai {\n    int funct m(self) {\n        return 1\n    }\n}\nclass Filha(Pai) {\n    str funct m(self) {\n        return \"texto\"\n    }\n}\n"
  "Pai p = Filha()\nint x = p.m()\npost(x)\n",
  "", "TypeError: m() de Filha devolve str e sobrescreve m() de Pai, que devolve int", 2 },
{ "sobrescrita sem tipo de retorno sobre um metodo tipado: erro",
  "class Pai {\n    str funct m(self) {\n        return \"a\"\n    }\n}\nclass Filha(Pai) {\n    funct m(self) {\n        return 5\n    }\n}\n",
  "", "TypeError: m() de Filha nao declara o retorno e sobrescreve m() de Pai, que devolve str: declare str funct m", 2 },
{ "sobrescrita de um Null funct que devolve valor: erro",
  "class Pai {\n    Null funct m(self) {\n        post(\"pai\")\n    }\n}\nclass Filha(Pai) {\n    int funct m(self) {\n        return 1\n    }\n}\n",
  "", "TypeError: m() de Filha devolve int e sobrescreve m() de Pai, que devolve Null", 2 },
{ "sobrescrita que devolve a subclasse, e o private do pai nao e sobrescrito: valem",
  "class Animal {\n    str nome = \"a\"\n}\nclass Cao(Animal) {\n}\nclass Pai {\n    Animal funct bicho(self) {\n        return Animal()\n    }\n"
  "    private int funct seg(self) {\n        return 1\n    }\n}\nclass Filha(Pai) {\n    Cao funct bicho(self) {\n        return Cao()\n    }\n"
  "    str funct seg(self) {\n        return \"ok\"\n    }\n}\nFilha f = Filha()\npost(f.seg(), type(f.bicho()))\n",
  "ok Cao", NULL, 0 },
{ "enum: o automatico depois do maior int vira inteiro grande",
  "enum E {\n    A = 9223372036854775807,\n    B\n}\npost(E.B, type(E.B))\n",
  "9223372036854775808 int", NULL, 0 },
{ "enum: membro sem valor depois de texto e erro antes de rodar",
  "enum F {\n    A = \"texto\",\n    B\n}\npost(F.B)\n",
  "", "TypeError: enum F: o membro B nao tem valor e o anterior (A) nao e inteiro "
      "— so um inteiro continua a contagem: escreva o valor de B", 2 },
{ "enum: membro sem valor depois de valor nao inteiro que so se sabe rodando",
  "str s = \"t\"\nenum G {\n    A = s,\n    B\n}\npost(G.B)\n",
  "", "TypeError: enum G: o membro B nao tem valor e o anterior (A) nao e inteiro "
      "— so um inteiro continua a contagem: escreva o valor de B", 1 },
{ "enum como tipo de parametro aceita o membro",
  "enum Cor {\n    RED, GREEN\n}\nfunct pinta(Cor c) {\n    post(\"cor\", c)\n}\npinta(Cor.GREEN)\n", "cor 1", NULL, 0 },
{ "enum como tipo: int escrito e recusado antes de rodar",
  "enum Cor {\n    RED,\n    GREEN\n}\nCor c = 5\n", "", "AttributedValueError: variável c esperava Cor, recebeu int", 2 },
{ "enum como tipo: o valor que so se sabe rodando vale se for de um membro",
  "enum Cor {\n    RED,\n    GREEN\n}\nlist l = [1, 7]\nCor c = l[0]\npost(c)\nCor d = l[1]\npost(d)\n",
  "1", "AttributedValueError: variável d esperava Cor, recebeu int", 1 },
{ "membro de enum e constante: erro antes de rodar",
  "enum Cor {\n    RED, GREEN\n}\nCor.RED = 99\npost(Cor.RED)\n",
  "", "AttributeError: membro 'RED' do enum Cor e constante: nao se reatribui", 2 },
{ "enum nao ganha membro novo rodando",
  "enum Cor {\n    RED\n}\nObject e = Cor\ne.NOVO = 1\n",
  "", "AttributeError: enum Cor nao ganha membro novo ('NOVO'): os membros sao os da declaracao", 1 },
{ "asjson desce na Entity de dentro",
  "from datasentity import dataentity, asjson\n@dataentity\nEntity Endereco {\n    rua: str\n}\n@dataentity\nEntity Pessoa {\n"
  "    nome: str\n    end: Endereco\n}\nPessoa p = Pessoa(\"ana\", Endereco(\"x\"))\npost(asjson(p))\n",
  "{\"nome\": \"ana\", \"end\": {\"rua\": \"x\"}}", NULL, 0 },
{ "asdict desce na Entity de dentro, tambem em list; asjson e o asdict serializado",
  "from datasentity import dataentity, asdict, asjson\nimport json\n@dataentity\nEntity Endereco {\n    rua: str\n}\n"
  "@dataentity\nEntity Pessoa {\n    nome: str\n    end: Endereco\n    outros: list\n}\n"
  "Pessoa p = Pessoa(\"ana\", Endereco(\"x\"), [Endereco(\"y\"), 3])\ndict d = asdict(p)\npost(d)\npost(d[\"end\"].rua)\n"
  "post(asjson(p) == json.stringify(asdict(p)))\n",
  "{'nome': 'ana', 'end': {'rua': 'x'}, 'outros': [{'rua': 'y'}, 3]}\nx\nTrue", NULL, 0 },
{ "asdict de Entity que contem a si mesma: RecursionError",
  "from datasentity import asdict\nclass No {\n    list prox = []\n}\nNo a = No()\na.prox.append(a)\ntry {\n    post(asdict(a))\n"
  "} catch (RecursionError e) {\n    post(\"pegou:\", e)\n}\n",
  "pegou: asdict(): Entity aninhada demais (ou que contem a si mesma) (linha 8)", NULL, 0 },

/* ── import por caminho entre aspas (`import '../x.pr'`), 2026-09-06 ───────
 * A string e o especificador, como no TypeScript: com `/` ou extensao da
 * linguagem e caminho relativo ao arquivo que importa; sem isso e nome de
 * modulo (motor ou lib instalada). Os casos com arquivo de verdade estao em
 * teste/cobre_stdlib.pr; aqui, o que nao precisa de arquivo. */
{ "import por caminho inexistente cita o caminho como foi escrito",
  "import './nao_existe.pr'\n", "", "ImportError: No module named './nao_existe.pr'", 1 },
{ "import 'nome' sem barra nem extensao e nome de modulo, nao caminho",
  "import 'pasta_que_nao_existe'\n", "", "ImportError: No module named 'pasta_que_nao_existe'", 1 },
{ "import por caminho cujo arquivo nao serve de nome de variavel exige `as`",
  "import 'sub/meu-mod.pr'\n", "", "'meu-mod' nao serve de nome de variavel: ligue com `as`", 2 },
{ "import entre aspas vazio e erro de sintaxe",
  "import ''\n", "", "import entre aspas vazio", 2 },
{ "from ... import continua valendo",
  "from jinker import cors\npost(type(cors))\n", "CorsConfig", NULL, 0 },

/* ── módulo: o erro nomeia o membro e sugere o parecido ─────────────────── */
{ "membro inexistente nomeia modulo e membro",
  "import json\npost(json.naoexiste)\n",
  "", "AttributeError: module 'json' has no attribute 'naoexiste'", 2 },
{ "membro parecido vira sugestao",
  "import json\npost(json.parsee)\n",
  "", "AttributeError: module 'json' has no attribute 'parsee'. Did you mean: 'parse'?", 2 },

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
  "post(int == \"inteiro\", int == 1, int == Null)\n", "False False False", NULL, 0 },
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
 * flush, que é o `fclose`. Só o Linux tem esse dispositivo (o macOS e o
 * Windows não têm como simular disco cheio sem privilégio), então os três
 * valem só lá — o `testar` dos outros sistemas diz que não os rodou. */
{ "close() acusa o que nao conseguiu gravar",
  "f = open(\"/dev/full\", \"w\")\n"
  "f.write(\"abc\")\n"
  "f.close()\n"
  "post(\"NAO DEVIA CHEGAR\")\n", "", "OSError: [Errno 28] No space left on device", 1, NULL, 0, "linux" },
{ "write() grande acusa gravacao incompleta",
  "f = open(\"/dev/full\", \"w\")\n"
  "f.write(\"x\" * 200000)\n", "", "[Errno 28]", 1, NULL, 0, "linux" },
{ "writelines() acusa e diz qual linha",
  "f = open(\"/dev/full\", \"w\")\n"
  "f.writelines([\"x\" * 200000])\n", "", "OSError: [Errno 28] No space left on device", 1, NULL, 0, "linux" },
{ "write() continua devolvendo quantos bytes gravou",
  "using open(\"ps_t_w.txt\", \"w\") as f {\n"
  "    post(f.write(\"abcde\"), f.write(\"xy\"))\n"
  "}\n", "5 2", NULL, 0 },
{ "close() em arquivo bom nao levanta nada",
  "f = open(\"ps_t_c.txt\", \"w\")\n"
  "f.write(\"ok\")\n"
  "f.close()\n"
  "f.close()\n"
  "post(open(\"ps_t_c.txt\").read())\n", "ok", NULL, 0 },

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

/* `copy()` num PoolFile vindo de `os.loadFile` (binário). O caminho inteiro do
 * copy() nunca tinha sido exercitado por caso nenhum da suíte (gcov: linha
 * ##### na função). Desde 2026-09-09 o mesmo `copy()` vale no PoolFile aberto
 * por `open()` — ver "TODO ARQUIVO É PoolFile", abaixo. */
{ "PoolFile.copy() copia o conteudo",
  "import os\n"
  "import qrcode\n"
  "qrcode.make(\"oi\").save(\"o.png\")\n"
  "a = os.loadFile(\"o.png\")\n"
  "a.copy(\"d.png\")\n"
  "using open(\"d.png\", \"rb\") as f { post(len(f.read())) }\n", "1219", NULL, 0 },

/* ── regex: recursao profunda vira ERRO, nao segfault ───────────────────────
 * O casador é recursivo e gasta um quadro de pilha C por caractere. O teto de
 * PASSOS (2 milhões) não protegia disso: a pilha de 8 MB acaba muito antes, e
 * o processo morria de SIGSEGV, sem mensagem. Achado escrevendo o semeador do
 * fuzzer em Jinga — o motor inteiro caiu casando
 * `"((?:[^"\\]|\\.)*)"` contra um trecho de 29 mil caracteres. */
{ "regex profundo demais e erro, nao morte",
  "import regex\n"
  "alvo = \"x\" * 60000\n"
  "post(regex.findall(\"(?:a|(x))*\", alvo))\n", "", "regex: padrao ou texto profundo demais pra pilha", 1 },
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
 * escreve — o buraco apareceu escrevendo ferramenta EM Jinga, que é onde
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
  "x = fun" "\xc3\xa7" "ao(\n", "", "nome so aceita letra sem acento, digito e _: 'ç'", 2 },
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
  "post(saudacao(5, 3))\n",
  "", "AttributedValueError: parâmetro nome de saudacao() esperava str, recebeu int", 2 },
{ "apelido do tipo vale no parametro (String e str, Integer e int)",
  "funct f(String s, Integer n) { return s * n }\npost(f(\"a\", 2))\n",
  "aa", NULL, 0 },
{ "int NAO aceita flo no parametro",
  "funct f(int n) { post(n) }\nf(1.5)\n",
  "", "AttributedValueError: parâmetro n de f() esperava int, recebeu flo", 2 },
{ "flo NAO aceita int no parametro",
  "funct f(flo x) { post(x) }\nf(1)\n",
  "", "AttributedValueError: parâmetro x de f() esperava flo, recebeu int", 2 },
{ "bool NAO aceita int no parametro",
  "funct f(bool b) { post(b) }\nf(1)\n",
  "", "AttributedValueError: parâmetro b de f() esperava bool, recebeu int", 2 },
{ "tipar e opcional e por parametro — pode misturar",
  "funct mist(str a, b, int c) { post(a, b, c) }\nmist(\"a\", [1], 2)\n",
  "a [1] 2", NULL, 0 },
{ "o tipo vale no argumento NOMEADO",
  "funct f(str nome, int vezes) { return nome * vezes }\n"
  "post(f(vezes=2, nome=\"ei \"))\n", "ei ei ", NULL, 0 },
{ "argumento nomeado de tipo errado e recusado",
  "funct f(str nome, int vezes) { return nome * vezes }\n"
  "post(f(vezes=\"x\", nome=\"ei \"))\n",
  "", "AttributedValueError: parâmetro vezes de f() esperava int, recebeu str", 2 },
{ "valor padrao convive com o tipo",
  "funct pad(str a, int n = 2) { return a * n }\npost(pad(\"x\"), pad(\"x\", 3))\n",
  "xx xxx", NULL, 0 },
{ "o tipo vale no METODO da Entity",
  "Entity P {\n  str nome\n  public str funct diz(self, str saud) { return saud + self.nome }\n}\n"
  "P p = P(\"Ana\")\npost(p.diz(\"ola \"))\np.diz(9)\n",
  "", "AttributedValueError: parâmetro saud de diz() esperava str, recebeu int", 2 },
{ "o tipo vale no metodo static chamado na Entity",
  "Entity P {\n  str nome\n  @static\n  public static funct cria(str nome) { return P(nome) }\n}\n"
  "post(P.cria(\"Ana\").nome)\nP.cria(1)\n",
  "", "AttributedValueError: parâmetro nome de cria() esperava str, recebeu int", 2 },
{ "o tipo vale na lambda",
  "g = funct(int n) { return n + 1 }\npost(g(2))\ng(\"x\")\n", "3",
  "AttributedValueError: parâmetro n de <funct>() esperava int, recebeu str", 1 },
{ "o tipo vale no gerador",
  "funct gera(int n) { for each i in range(n) { yield i } }\n"
  "post(list(gera(3)))\npost(list(gera(\"x\")))\n",
  "", "AttributedValueError: parâmetro n de gera() esperava int, recebeu str", 2 },
{ "o tipo vale na async funct",
  "async funct af(str s) { post(s) }\ngather([af(1)])\n",
  "", "AttributedValueError: parâmetro s de af() esperava str, recebeu int", 2 },
{ "nome de Entity serve de tipo, e subclasse passa (como em Java)",
  "Entity Animal { str nome }\nEntity Cachorro(Animal) { }\n"
  "funct fala(Animal a) { post(a.nome) }\nfala(Cachorro(\"Rex\"))\n",
  "Rex", NULL, 0 },
{ "Entity errada no lugar de outra e recusada",
  "Entity Animal { str nome }\nEntity Carro { str nome }\n"
  "funct fala(Animal a) { post(a.nome) }\nfala(Carro(\"Fusca\"))\n",
  "", "AttributedValueError: parâmetro a de fala() esperava Animal, recebeu Carro", 2 },
{ "PoolFile serve de tipo de parametro — o handle do open() E PoolFile",
  "funct pega(PoolFile f) { post(type(f)) }\n"
  "using open(\"ps_param_tipo.txt\", \"w\") as f { pega(f) }\n",
  "PoolFile", NULL, 0 },
{ "char no parametro pede UM caractere",
  "funct f(char c) { post(c) }\nf(\"a\")\nf(\"ab\")\n",
  "", "AttributedValueError: parâmetro c de f() esperava char, recebeu str", 2 },
{ "list/dict/tup no parametro conferem o container certo",
  "funct lst(list xs, dict d, tup t) { post(len(xs), len(d), len(t)) }\n"
  "lst([1], {\"a\": 1}, (1, 2))\nlst([1], {\"a\": 1}, [1, 2])\n",
  "", "AttributedValueError: parâmetro t de lst() esperava tup, recebeu list", 2 },
/* A ordem invertida dava "faltou ')' na declaracao da funct" — mensagem que
 * fala de um parentese que ninguem esqueceu. Quem escreve `x: int` inverteu a
 * ordem, e o erro tem que dizer isso. */
{ "`x: tipo` NAO existe no parametro — e o erro diz a ordem certa",
  "funct f(x: int) { post(x) }\n", "",
  "no parametro o tipo vem ANTES do nome: escreva `funct f(int x)`", 2 },
{ "a mesma inversao na lambda tambem e apontada",
  "g = funct(x: int) { return x }\n", "",
  "no parametro o tipo vem ANTES do nome: escreva `funct(int x)`", 2 },
/* Rodando, a frase e a do editor: o `(` aberto, no `(`. Dizia "faltou ')' na
 * declaracao da funct" — o mesmo esquecimento com outro nome e outra coluna
 * do que o `--check` mostrava, e a IDE parecia se contradizer. */
{ "parentese que falta de verdade continua dizendo que falta, no abridor",
  "funct f(a\n", "", "parentese '(' aberto nao foi fechado", 2 },
{ "parametro sem tipo continua valendo",
  "funct f(a, b = 10) { return a + b }\npost(f(5), f(5, 1))\n", "15 6", NULL, 0 },

/* `f is PoolFile` seguia falso pro handle do open(): a tabela de tipos so
 * aceitava OBJ_POOLFILE, contra a regra "todo arquivo e PoolFile" — o mesmo
 * `type()` ja respondia "PoolFile" pros dois. */
{ "open() e PoolFile tambem pro `is` e pra declaracao tipada",
  "using open(\"ps_param_tipo2.txt\", \"w\") as f {\n"
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
  "try { post(1 + [\"a\"][0]) }\ncatch (Exception e) { post(\"pegou\") }\n",
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
/* O valor vem de `json.parse`, cujo tipo só se sabe rodando: com o literal
 * `"7"` o erro sai ANTES de rodar, e aí não há `catch` que o pegue. */
{ "ValueError pega AttributedValueError da declaracao tipada",
  "import json\ntry { int x = json.parse(\"\\\"7\\\"\") }\ncatch (ValueError e) { post(\"pegou\") }\n",
  "pegou", NULL, 0 },
{ "RuntimeError pega RecursionError",
  "funct f() { return f() }\ntry { f() }\ncatch (RuntimeError e) { post(\"pegou\") }\n",
  "pegou", NULL, 0 },
/* A hierarquia so ACRESCENTA: quem nao e da linhagem continua nao pegando. */
{ "TypeError NAO pega ValueError",
  "try { raise ValueError(\"x\") }\ncatch (TypeError e) { post(\"NAO DEVIA\") }\n", "",
  "ValueError: x", 1 },
{ "OSError NAO pega TypeError",
  "try { post(1 + [\"a\"][0]) }\ncatch (OSError e) { post(\"NAO DEVIA\") }\n", "",
  "TypeError: unsupported operand type(s) for +: 'int' and 'str' (linha 1)", 1 },
{ "IOError segue IRMAO de OSError, nao pai — a doc do open() promete isso",
  "import os\ntry { os.loadFile(\"/nao/existe/xyz.txt\") }\ncatch (IOError e) { post(\"NAO DEVIA\") }\n",
  "", "FileNotFoundError: [Errno 2] No such file or directory: '/nao/existe/xyz.txt' (linha 2)", 1 },
/* Um inventario de 14 agentes cruzou os tipos que o motor EMITE contra a
 * tabela e achou 15 de fora. `NotImplementedError` e `TimeoutError` eram da
 * propria linguagem e escapavam do `catch (Exception e)`; os outros 13 sao
 * nomes de erro do postgres, agora todos com `DatabaseError` de pai.
 *
 * SEM CASO AQUI, e a razao: `NotImplementedError` so nasce ao IMPORTAR um
 * modulo que nao compila, e o runner nao consegue importar — ele roda o caso
 * num tmpfile sem extensao, e a busca de modulo olha a pasta do SCRIPT, nao o
 * cwd (medido: o `.pr` escrito esta la, `os.isfile` responde True, e o import
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

/* ── L2 DO PLANO DAS CONTRADICOES: compilador, parser, lexer (2026-09-12) ──
 * Toda string abaixo foi MEDIDA no binario antes de entrar. */

/* Erro de PROGRAMA e SyntaxError (rc 2); NotImplementedError (rc 3) fica so
 * pro que a VM nao emite, limite do motor e falta de memoria. Vinte sitios
 * usavam `cerro` (rc 3) pra erro de quem escreveu. */
{ "break fora de laco e SyntaxError, rc 2",
  "break\n", "", "SyntaxError: 'break' fora de laco", 2 },

/* ── `return`/`yield` fora de funct (2026-10-05) ────────────────────────────
 * No topo do arquivo, ou num `if`/laço do topo, o `return` encerrava o
 * programa calado (rc 0, `--check` ok); o `yield` só quebrava rodando. Agora
 * os dois são SyntaxError antes de rodar, como o `break` fora de laço. */
{ "return no topo e SyntaxError antes de rodar",
  "post(1)\nreturn 5\n", "", "SyntaxError: 'return' fora de funct", 2 },
{ "return no topo mostra a linha dele",
  "int x = 1\nif x == 1 {\n    return x\n}\n", "", "  |     return x\n", 2 },
{ "return seco dentro de if do topo e SyntaxError",
  "if True {\n    return\n}\n", "", "SyntaxError: 'return' fora de funct", 2 },
{ "return dentro de while do topo e SyntaxError",
  "int i = 0\nwhile i < 3 {\n    i = i + 1\n    return\n}\n", "", "SyntaxError: 'return' fora de funct", 2 },
{ "return dentro de for each do topo e SyntaxError",
  "for each x in [1, 2] {\n    return x\n}\n", "", "SyntaxError: 'return' fora de funct", 2 },
{ "return; dentro de count each do topo e SyntaxError (encerrava o programa)",
  "count each int in [1, 2] {\n    return;\n}\npost(\"depois\")\n", "", "SyntaxError: 'return' fora de funct", 2 },
{ "yield no topo e SyntaxError antes de rodar",
  "yield 1\n", "", "SyntaxError: 'yield' fora de funct", 2 },
{ "return em funct, funct anonima e metodo continua valendo",
  "int funct f() {\n    return 1\n}\nObject g = funct() {\n    return 2\n}\n"
  "class C {\n    int funct m(self) {\n        return 3\n    }\n}\npost(f(), g(), C().m())\n", "1 2 3", NULL, 0 },
{ "return; no count each dentro de funct continua devolvendo o total",
  "int funct conta() {\n    count each int in [1, \"x\", 3] {\n        return;\n    }\n}\npost(conta())\n", "2", NULL, 0 },
{ "yield em funct continua fazendo gerador",
  "funct g() {\n    yield 1\n    yield 2\n}\nfor each v in g() {\n    post(v)\n}\n", "1\n2", NULL, 0 },

/* ── count: motor-22, motor-24 e motor-34 (19.0.1) ───────────────────────────
 * motor-22: no `count each <tipo> in "texto" {`, o `{` depois do texto era a
 * interpolacao antiga, e o bloco virava parte do texto: o corpo rodava uma
 * vez, calado. O conteiner e lido no modo cabeca, como o iteravel do
 * `for each`. motor-24: o `*` e o `**` no valor do `count` sumiam, e
 * `int(*l) count in xs` contava a lista `l`. motor-34: as quatro formas
 * (`int(7) count in l`, `int(7) in l count`, `count int(7) in l` e
 * `count each int(7) in l`) recusam o nome, o segundo valor e a estrela com a
 * mesma frase, no trecho que causa (o lugar e o fim, pelo JSON do `--check`,
 * ficam no teste/cli_roda.pr). */
{ "motor-22: count each sobre texto escrito roda o bloco em cada letra",
  "count each str in \"abca\" { post(\"dentro\") }\npost(\"fim\")\n", "dentro\ndentro\ndentro\ndentro\nfim", NULL, 0 },
{ "motor-22: _index e _match no count each sobre texto escrito",
  "count each str in \"oba\" { post(_index, _match) }\n", "0 o\n1 b\n2 a", NULL, 0 },
{ "motor-22: return; no count each sobre texto escrito devolve o total",
  "int funct n() {\n    count each str in \"abca\" {\n        return;\n    }\n}\npost(n())\n", "4", NULL, 0 },
{ "motor-22: a forma de expressao sobre texto escrito continua",
  "post(count each str in \"abca\")\n", "4", NULL, 0 },
{ "motor-24: o * no valor do count infixo e erro",
  "list xs = [7, 7, 1]\nlist l = [7]\npost(int(*l) count in xs)\n",
  "", "SyntaxError: 'count' nao aceita '*' no valor: o valor entre parenteses e um so", 2 },
{ "motor-24: o ** no valor do count infixo e erro",
  "list xs = [7, 7, 1]\ndict d = {\"a\": 7}\npost(int(**d) count in xs)\n",
  "", "SyntaxError: 'count' nao aceita '**' no valor: o valor entre parenteses e um so", 2 },
{ "motor-24: o * no valor do count sufixo e erro",
  "list xs = [7, 7, 1]\nlist l = [7]\npost(int(*l) in xs count)\n",
  "", "SyntaxError: 'count' nao aceita '*' no valor: o valor entre parenteses e um so", 2 },
{ "motor-34: o * no valor do count prefixo e a frase das outras formas",
  "list xs = [7, 7, 1]\nlist l = [7]\npost(count int(*l) in xs)\n",
  "", "SyntaxError: 'count' nao aceita '*' no valor: o valor entre parenteses e um so", 2 },
{ "motor-34: o * no valor do count each e a frase das outras formas",
  "list xs = [7, 7, 1]\nlist l = [7]\ncount each int(*l) in xs { post(_match) }\n",
  "", "SyntaxError: 'count' nao aceita '*' no valor: o valor entre parenteses e um so", 2 },
{ "motor-34: o nome no valor do count prefixo e a frase das outras formas",
  "list xs = [7]\npost(count int(v=7) in xs)\n",
  "", "SyntaxError: 'count' nao aceita argumentos nomeados no valor", 2 },
{ "motor-34: o segundo valor no count prefixo e a frase das outras formas",
  "list xs = [7]\npost(count int(7, 8) in xs)\n",
  "", "SyntaxError: 'count' aceita no maximo um valor entre parenteses", 2 },
{ "motor-34: o nome no valor do count infixo e erro",
  "list xs = [7]\npost(int(v=7) count in xs)\n",
  "", "SyntaxError: 'count' nao aceita argumentos nomeados no valor", 2 },
{ "motor-34: o segundo valor no count infixo e erro",
  "list xs = [7]\npost(int(7, 8) count in xs)\n",
  "", "SyntaxError: 'count' aceita no maximo um valor entre parenteses", 2 },
{ "motor-34: o valor unico continua nas quatro formas",
  "list xs = [7, 7, 1]\npost(int(7) count in xs, count int(7) in xs, int in xs count, int(7,) count in xs)\n"
  "count each int(7) in xs { post(_match) }\n", "2 2 3 2\n7\n7", NULL, 0 },

/* ── literais: so `True`, `False` e `Null` (2026-10-05) ──────────────────────
 * A grafia e a que o motor imprime, com a inicial maiuscula. `true`, `false`,
 * `null`, `None` e `none` sao nomes comuns: sem definicao, o NameError sai
 * antes de rodar, com a dica pelo mecanismo geral do "Did you mean" (`None`
 * fica longe de `Null` e nao ganha dica). Cada posicao, com JIT e sem. */
{ "true como valor e NameError com a dica True",
  "bool x = true\n", "", "NameError: name 'true' is not defined. Did you mean: 'True'?", 2 },
{ "false como argumento e NameError com a dica False",
  "post(false)\n", "", "NameError: name 'false' is not defined. Did you mean: 'False'?", 2 },
{ "null em comparacao e NameError com a dica Null",
  "int x = 1\npost(x == null)\n", "", "NameError: name 'null' is not defined. Did you mean: 'Null'?", 2 },
{ "None e NameError sem dica",
  "post(None)\n", "", "NameError: name 'None' is not defined\n", 2 },
{ "none como valor de dict e NameError sem dica",
  "dict d = {\"a\": none}\n", "", "NameError: name 'none' is not defined\n", 2 },
{ "true no padrao de parametro e NameError com a dica",
  "funct f(bool a = true) {\n    return a\n}\npost(f())\n", "", "NameError: name 'true' is not defined. Did you mean: 'True'?", 2 },
{ "a grafia antiga mostra a linha dela",
  "int x = 1\nstr y = null\n", "", "  | str y = null\n", 2 },
{ "True, False e Null valem e imprimem como se escrevem",
  "post(True, False, Null)\npost([True, Null])\n", "True False Null\n[True, Null]", NULL, 0 },
{ "a grafia antiga em texto e comentario fica como esta",
  "post(\"true false null None\")  # true\n", "true false null None", NULL, 0 },
{ "JSON continua minusculo: stringify escreve e parse le true/false/null",
  "import json\npost(json.stringify({\"a\": True, \"b\": Null, \"c\": False}))\n"
  "post(json.parse(\"[true, false, null]\"))\n",
  "{\"a\": true, \"b\": null, \"c\": false}\n[True, False, Null]", NULL, 0 },
/* `json` tambem e nome de TIPO (o dict), e a conferencia do nome solto
 * deixava `json.parse` sem import passar no --check; rodando dava NameError */
{ "json.parse sem import json e NameError antes de rodar, com a dica do import",
  "post(json.parse(\"[1]\"))\n", "", "NameError: name 'json' is not defined. Did you forget to import 'json'?", 2 },
{ "json solto continua sendo o tipo dict, sem import",
  "post(json)\njson d = {\"a\": 1}\npost(d)\n", "dict\n{'a': 1}", NULL, 0 },

/* ── `Null funct`: a funct que nao devolve valor (2026-10-05) ───────────────
 * `static Null funct main()` virava tres comandos (`static`, `Null` e a funct
 * sem tipo) e o erro saia no `static`. Decisao dele: a funct que nao devolve
 * — return seco ou `return Null`, o fim devolve Null, devolver valor e erro, e
 * o `post` de qualquer coisa vale (a regra da saida e de funct com valor). */
{ "static Null funct main vale (o arquivo dele)",
  "import os\n\n\nstatic Null funct main(){\n    \n}\n", "", NULL, 0 },
{ "Null funct: post de qualquer coisa e return seco",
  "Null funct main() {\n    post(\"rodando\", 1)\n    return\n}\nmain()\n", "rodando 1", NULL, 0 },
{ "Null funct: o fim e o return Null devolvem Null",
  "Null funct f() {\n    int x = 1\n}\nNull funct g() {\n    return Null\n}\npost(f(), g())\n", "Null Null", NULL, 0 },
{ "Null funct: devolver valor e erro antes de rodar",
  "Null funct f() {\n    return 1\n}\npost(f())\n", "", "AttributedValueError: retorno de f() esperava Null, recebeu int", 2 },
{ "Null funct: valor que so se sabe rodando e conferido no return",
  "Null funct f(list l) {\n    return l[0]\n}\nf([1])\n", "", "AttributedValueError: retorno de f() esperava Null, recebeu int", 1 },
{ "Null funct em metodo, static e async",
  "class C {\n    Null funct m(self) {\n        post(\"m\")\n    }\n    static Null funct s() {\n        post(\"s\")\n    }\n}\n"
  "Null async funct a() {\n    post(\"a\")\n}\nC().m()\nC.s()\nawait a()\n", "m\ns\na", NULL, 0 },

/* ── dois comandos na mesma linha exigem `;` (2026-10-05) ───────────────────
 * Sem separador a linha se partia calada e o erro caia longe da causa
 * (`int|str u = 1` virava `int|str` e um `u = 1` sem tipo). O erro sai no
 * comeco do segundo comando. */
{ "dois comandos na mesma linha sao erro no segundo",
  "int x = 1 int y = 2\npost(x, y)\n", "", "SyntaxError: dois comandos na mesma linha: separe com ';' ou quebre a linha", 2 },
{ "uniao na declaracao nao se parte calada: o erro cai no | e diz o que nao vale",
  "int|str u = 1\n", "", "SyntaxError: tipo uniao ('int|str') nao vale na declaracao: declare um tipo so", 2 },
{ "com ; dois comandos na mesma linha valem",
  "int x = 1; int y = 2; post(x, y)\n", "1 2", NULL, 0 },
{ "bloco de uma linha com chaves continua valendo",
  "int funct f() { return 1 }\nif True { post(f()) }\n", "1", NULL, 0 },
{ "decoradores em linhas seguidas nao sao comandos colados",
  "Object funct marca(Object f) {\n    return f\n}\n@marca\n@marca\nint funct g() {\n    return 7\n}\npost(g())\n", "7", NULL, 0 },

/* ── o que a regra do separador quebrou na primeira versao (2026-10-05) ──────
 * A sonda achou 13 regressoes: tudo que se partia calado antes passou a dar
 * "dois comandos" numa palavra que nao era a causa, com um `;` que nao
 * conserta nada. A regra agora: o erro cai na causa (o nome solto, a palavra
 * de clausula, o segundo tipo da cabeca), e "dois comandos" so quando sao
 * mesmo dois. */
{ "decorador e metodo na mesma linha dentro da classe sao uma declaracao",
  "class A {\n    @static funct s() { post(\"s\") }\n}\nA.s()\n", "s", NULL, 0 },
{ "decorador com argumento e cabeca inteira na mesma linha, na classe",
  "Object funct marca(str n) {\n    return funct(Object f) { return f }\n}\n"
  "class A {\n    @marca(\"x\") public static str funct h(str d) { return d }\n}\npost(A.h(\"ok\"))\n", "ok", NULL, 0 },
{ "dois membros na mesma linha da classe exigem ; (campo depois de campo tambem)",
  "class C { int x = 1 int y = 2 }\n", "", "SyntaxError: dois membros na mesma linha: separe com ';' ou quebre a linha", 2 },
{ "membros na mesma linha com ; valem, metodo no meio",
  "class C { int x = 1; int funct a(self) { return self.x }; str s = \"z\" }\nC c = C()\npost(c.a(), c.s)\n", "1 z", NULL, 0 },
{ "funct esquecido com static e Null diz a linha montada, no nome",
  "static Null f() { post(1) }\n", "", "SyntaxError: faltou 'funct' antes de 'f': static Null funct f(...)", 2 },
{ "funct esquecido com Null async e com nonnull",
  "Null async f() { post(1) }\n", "", "faltou 'funct' antes de 'f': Null async funct f(...)", 2 },
{ "funct esquecido com classe como tipo de retorno",
  "class Ponto { int x = 0 }\nPonto f() { return Ponto() }\n", "", "faltou 'funct' antes de 'f': Ponto funct f(...)", 2 },
{ "funct esquecido dentro da classe",
  "class C {\n    static Null f() {\n        post(1)\n    }\n}\n", "", "faltou 'funct' antes de 'f': static Null funct f(...)", 2 },
{ "palavra da cabeca digitada errado diz a certa",
  "funtc f() {\n    post(1)\n}\n", "", "SyntaxError: 'funtc' nao existe. Did you mean: 'funct'?", 2 },
{ "nome solto digitado errado e NameError nele, com a palavra-chave na dica",
  "int funct f(int x) {\n    retrun x\n}\npost(f(1))\n", "", "NameError: name 'retrun' is not defined. Did you mean: 'return'?", 2 },
{ "nome solto com letras invertidas acha a palavra (whlie, fro, riase)",
  "int i = 0\nwhlie i < 3 {\n    i += 1\n}\n", "", "NameError: name 'whlie' is not defined. Did you mean: 'while'?", 2 },
{ "nome solto de outra linguagem e NameError nele, nao na palavra seguinte",
  "const int x = 1\n", "", "NameError: name 'const' is not defined. Did you mean: 'count'?", 2 },
{ "nome solto que e tipo: declaracao sem =",
  "class Conta { int v = 0 }\nConta c\n", "", "SyntaxError: declaracao de variavel exige '='", 2 },
{ "nome solto que existe seguido de comando sao dois comandos mesmo",
  "int x = 1\nint retrun = 2\nint funct f() {\n    retrun x\n}\n", "", "SyntaxError: dois comandos na mesma linha: separe com ';' ou quebre a linha", 2 },
{ "uniao no parametro e no campo: a mesma frase, no |",
  "Null funct f(int|str x) { post(x) }\n", "", "tipo uniao ('int|str') nao vale na declaracao", 2 },
{ "else depois do guard diz que o guard nao tem else",
  "if __name__ == \"main\" {\n    post(\"main\")\n} else {\n    post(\"importado\")\n}\n", "", "SyntaxError: o guard 'if __name__ == \"main\"' nao tem 'else'", 2 },
{ "elif depois do else e clausula fora do lugar",
  "if True {\n} else {\n}\nelif False {\n}\n", "", "SyntaxError: 'elif' fora do lugar: vem logo depois do '}' de um if ou elif", 2 },
{ "finally repetido e case fora de match",
  "try {\n    post(1)\n} finally {\n    post(2)\n} finally {\n    post(3)\n}\n", "", "'finally' fora do lugar: vem uma vez so", 2 },
{ "each sem for",
  "each x in [1, 2] {\n    post(x)\n}\n", "", "'each' fora do lugar: o laco e 'for each x in ...' (ou 'count each')", 2 },
{ "ternario sem else e erro no if",
  "int x = 1\nint y = x if True\npost(y)\n", "", "SyntaxError: faltou 'else' no ternario: valor if condicao else outro", 2 },
{ "o if da compreensao continua sendo o filtro",
  "list l = [x for each x in [1, 2, 3] if x > 1]\npost(l)\n", "[2, 3]", NULL, 0 },
{ "modificador depois do tipo na declaracao de campo dentro do construtor",
  "class P {\n    funct __init__(self, str n) {\n        str private name = n\n    }\n    str funct nome(self) { return self.name }\n}\npost(P(\"ana\").nome())\n", "ana", NULL, 0 },
{ "dois tipos de retorno na cabeca e erro no segundo (o Null nao some calado)",
  "Null int funct f() { return 1 }\npost(f())\n", "", "SyntaxError: dois tipos de retorno na cabeca da funct: 'Null' e 'int' (fica um so)", 2 },
{ "dois tipos na lambda tambem",
  "Object h = Null int funct() { return 1 }\n", "", "dois tipos de retorno na cabeca da funct: 'Null' e 'int'", 2 },
{ "modificador digitado errado na cabeca diz o certo",
  "pirvate Null funct m() { post(1) }\n", "", "SyntaxError: modificador 'pirvate' nao existe. Did you mean: 'private'?", 2 },
{ "modificador repetido na cabeca",
  "static static funct f() { post(1) }\n", "", "SyntaxError: 'static' repetido na cabeca da funct", 2 },
{ "public e private na mesma cabeca",
  "public private funct f() { post(1) }\n", "", "SyntaxError: 'public' e 'private' na mesma cabeca: a funct e uma ou a outra", 2 },
{ "Null so e tipo no retorno: variavel",
  "Null x = Null\npost(x)\n", "", "SyntaxError: 'Null' so e tipo no retorno de funct (Null funct f() { ... }): a variavel leva um tipo de valor", 2 },
{ "Null so e tipo no retorno: parametro e campo",
  "Null funct f(Null x) { post(x) }\n", "", "'Null' so e tipo no retorno de funct (Null funct f() { ... }): o parametro leva um tipo de valor", 2 },
{ "Null so e tipo no retorno: campo da classe nas duas grafias",
  "class C { campo: Null = Null }\n", "", "'Null' so e tipo no retorno de funct (Null funct f() { ... }): o campo leva um tipo de valor", 2 },

/* ── o arquivo dele de 2026-10-05: `while x < 10.000.000.000` com o `{` na
 * linha de baixo. O numero com ponto de milhar virava `10.000` + `.000` e o
 * erro saia "esperado nome do membro apos '.'"; e o `{` orfao do cabecalho
 * quebrado virava DICIONARIO, com "faltou ':' no dicionario" em cima do
 * `x++;` — a cascata que ele nao aceita. */
{ "numero com ponto de milhar diz as grafias que valem, no numero inteiro",
  "int x = 10.000.000\n", "", "SyntaxError: numero nao usa '.' como separador de milhar: escreva 10000000 (ou 10_000_000)", 2 },
{ "numero com _ de milhar, float e expoente continuam valendo",
  "int x = 10_000_000\nflo f = 1.5\npost(x, f, 1e3)\n", "10000000 1.5 1000.0", NULL, 0 },
{ "cabecalho quebrado com a chave embaixo: o bloco vai junto, sem erro de dicionario no corpo",
  "int x = 0\nwhile x < 10.000.000\n{\n    x++;\n}\npost(x)\n", "", "separador de milhar", 2 },

/* ── Plano 6, lote 1: o arquivo-fonte ────────────────────────────────────────
 * Número colado em letra ou em dígito que a base não aceita é UM erro, no
 * número inteiro, com o motivo. Antes o número era partido e o resto virava
 * "dois comandos na mesma linha". */
{ "numero binario com digito que nao vale",
  "int x = 0b102\n", "", "SyntaxError: '2' nao vale em numero binario (0b), so 0 e 1: '0b102'", 2 },
{ "numero octal com digito que nao vale",
  "int x = 0o78\n", "", "SyntaxError: '8' nao vale em numero octal (0o), so 0 a 7: '0o78'", 2 },
{ "numero hexadecimal com letra que nao vale",
  "int x = 0x1G\n", "", "SyntaxError: 'G' nao vale em numero hexadecimal (0x), so 0 a 9 e a a f: '0x1G'", 2 },
{ "expoente sem digito",
  "flo f = 1.5e\n", "", "SyntaxError: expoente sem digito em '1.5e': escreva o expoente, como em 1e3 ou 2.5e-3", 2 },
{ "expoente so com o sinal",
  "flo f = 2e+\n", "", "SyntaxError: expoente sem digito em '2e+': escreva o expoente, como em 1e3 ou 2.5e-3", 2 },
{ "nome que comeca com digito",
  "int 1x = 2\n", "", "SyntaxError: '1x' nao e numero nem nome: nome nao comeca com digito", 2 },
{ "numeros validos de toda base continuam valendo",
  "post(0x1F, 0o17, 0b101, 1_000, 2.5e-3, 1e5)\n", "31 15 5 1000 0.0025 100000.0", NULL, 0 },
/* String simples não cruza a quebra de linha (doc 1.6.3): a barra no fim da
 * linha não a continua. Na tripla a barra e a quebra ficam no texto, e a
 * linha conta (o erro de baixo saía uma linha acima). */
{ "barra no fim da linha nao continua a string simples",
  "str s = \"a\\\nb\"\npost(s)\n", "", "SyntaxError: string nao fechada antes da quebra de linha", 2 },
{ "barra no fim da linha da string tripla: fica no texto e a linha conta",
  "str t = '''a\\\nb'''\npost(len(t))\ntry {\n    list l = []\n    post(l[1])\n} catch (IndexError e) {\n    post(e)\n}\n",
  "4\nlist index out of range (linha 6)", "barra no fim da linha", 0 },
/* Arquivo salvo no Windows: a quebra CRLF dentro da string de várias linhas
 * vale `\n` (doc 1.1), e o programa dá o mesmo que salvo com LF. */
{ "CRLF dentro da string tripla vira \\n",
  "str a = '''x\r\nY'''\r\npost(len(a), a == \"x\\nY\")\r\n", "3 True", NULL, 0 },
/* O BOM do UTF-8 no começo é marca de editor; espaço e caractere invisível
 * fora de texto dizem qual são; byte que não é UTF-8 é erro na posição. */
{ "BOM no comeco do arquivo e aceito",
  "\xEF\xBB\xBFint x = 1\npost(x)\n", "1", NULL, 0 },
{ "espaco inquebravel fora de texto diz o codigo",
  "int x\xC2\xA0= 1\n", "", "SyntaxError: espaco invisivel U+00A0 (nao e o espaco comum) fora de texto: troque por um espaco", 2 },
{ "espaco de largura zero fora de texto diz o codigo",
  "int x = 1\xE2\x80\x8B\n", "", "SyntaxError: caractere invisivel U+200B fora de texto: apague", 2 },
{ "fonte em Latin-1 num nome",
  "int a\xe7 = 1\n", "", "SyntaxError: o fonte nao e UTF-8 valido: byte 0xE7 (salve o arquivo como UTF-8)", 2 },
{ "fonte em Latin-1 dentro de string",
  "str s = \"\xe1\"\npost(len(s))\n", "", "SyntaxError: o fonte nao e UTF-8 valido: byte 0xE1 (salve o arquivo como UTF-8)", 2 },
/* `//` depois de código com nome que não existe à direita é comentário do
 * jeito antigo: a frase é a do `//` no começo da linha (doc 1.2). */
{ "// com uma palavra depois de codigo",
  "int x = 10 // comentario\npost(x)\n", "", "SyntaxError: '//' e divisao inteira, nao comentario — comentario e '#' (ou bloco entre tres aspas)", 2 },
{ "// com varias palavras depois de codigo",
  "int x = 1 // um comentario aqui\npost(x)\n", "", "SyntaxError: '//' e divisao inteira, nao comentario — comentario e '#' (ou bloco entre tres aspas)", 2 },
{ "// com nome que existe e divisao inteira",
  "int a = 7\nint b = 2\npost(a // b, 10 // b + 1)\n", "3 6", NULL, 0 },

/* ── Plano 6, lote 2: frases do parser que apontam a causa ───────────────────
 * Quem exige um token olha primeiro QUE token encontrou: o `=` depois de um
 * operando é a comparação escrita com atribuição; o operador invertido diz
 * qual vale. */
{ "= no lugar de == no if",
  "int x = 5\nif x = 5 {\n    post(1)\n}\n", "", "SyntaxError: '=' e atribuicao; pra comparar use '=='", 2 },
{ "= no lugar de == no while",
  "int x = 5\nwhile x = 5 {\n}\n", "", "SyntaxError: '=' e atribuicao; pra comparar use '=='", 2 },
{ "= no lugar de == depois de and",
  "int x = 5\nint y = 3\nif x > 1 and y = 3 {\n}\n", "", "SyntaxError: '=' e atribuicao; pra comparar use '=='", 2 },
{ "= no lugar de == entre parenteses",
  "int x = 5\nif (x = 5) {\n}\n", "", "SyntaxError: '=' e atribuicao; pra comparar use '=='", 2 },
{ "= no lugar de == no ternario",
  "int x = 5\nstr s = \"a\" if x = 5 else \"b\"\n", "", "SyntaxError: '=' e atribuicao; pra comparar use '=='", 2 },
{ "= no lugar de == no return",
  "int x = 5\nbool funct f() {\n    return x = 5\n}\n", "", "SyntaxError: '=' e atribuicao; pra comparar use '=='", 2 },
{ "+= no lugar de == no if",
  "int x = 5\nif x += 5 {\n}\n", "", "SyntaxError: '+=' e atribuicao; pra comparar use '=='", 2 },
{ "=> invertido diz >=",
  "int x = 5\nif x => 5 {\n}\n", "", "SyntaxError: '=>' nao existe: use '>='", 2 },
{ "=< invertido diz <=",
  "int x = 5\nif x =< 5 {\n}\n", "", "SyntaxError: '=<' nao existe: use '<='", 2 },
{ "=! invertido diz !=",
  "int x = 5\nif x =! 5 {\n}\n", "", "SyntaxError: '=!' nao existe: use '!='", 2 },
{ "= = segue expressao invalida (o = de antes nao e operando)",
  "int x = = 1\n", "", "SyntaxError: expressao invalida\n", 2 },
{ "comparacao e atribuicao a indice continuam valendo",
  "int x = 5\nif x == 5 {\n    post(1)\n}\nlist l = [1]\nl[0] = 7\npost(l)\n", "1\n[7]", NULL, 0 },
/* Vírgula esquecida entre itens: o erro é no item, na linha dele; sem o
 * fechador adiante, o grupo é que ficou aberto (o erro no abridor). */
{ "virgula esquecida em lista de varias linhas",
  "list l = [\n    10,\n    20\n    30,\n    40\n]\n", "", "SyntaxError: faltou ',' entre os itens da lista", 2 },
{ "virgula esquecida em dicionario",
  "dict d = {\"a\": 1 \"b\": 2}\n", "", "SyntaxError: faltou ',' entre os pares do dicionario", 2 },
{ "virgula esquecida em tupla",
  "tup t = (1, 2 3)\n", "", "SyntaxError: faltou ',' entre os itens da tupla", 2 },
{ "lista sem fechar continua acusando o abridor",
  "list l = [1, 2\npost(l)\n", "", "SyntaxError: colchete '[' aberto nao foi fechado", 2 },
/* Decisão do dono: dentro de argumentos ou itens, `(` ou `[` separado por
 * espaço do valor de antes pede juntar ou a vírgula. */
{ "( separado por espaco dentro dos argumentos",
  "int x = 1\npost(\"total:\" (x + 1))\n", "",
  "SyntaxError: '(' separado por espaco dentro dos argumentos: junte ao valor de antes (chamada) ou separe com ',' (outro argumento)", 2 },
{ "[ separado por espaco dentro dos itens",
  "list l = [\"a\" [\"b\"]]\n", "",
  "SyntaxError: '[' separado por espaco dentro dos itens: junte ao valor de antes (indice) ou separe com ',' (outro item)", 2 },
{ "juntos continuam chamada e indice; justaposicao com nome continua argumento",
  "list l = [1, 2,]\nint x = 1\npost(\"total:\" x, len(l), l[0], (1 + 2))\n", "total: 1 2 1 3", NULL, 0 },
/* String sem fecho: o erro é na aspa que abriu, e o que ela engoliu não vira
 * erro (ver cli_roda.pr: um erro só na lista do --check). */
{ "string sem fecho dentro de chamada: o erro e da string",
  "int idade = 30\npost(\"voce tem \" + str(idade) + \" anos)\npost(\"fim\")\n", "",
  "SyntaxError: string nao fechada antes da quebra de linha", 2 },
{ "string sem fecho em lista (aspa que fechou cedo): o erro e da string",
  "list l = [\"a\", \"b, \"c\"]\n", "", "SyntaxError: string nao fechada antes da quebra de linha", 2 },
/* Palavra reservada como nome numa declaração com tipo: a frase da
 * palavra reservada, no nome (era "dois comandos" ou o tipo acusado). */
{ "palavra reservada como nome de variavel com tipo",
  "int count = 0\n", "", "SyntaxError: 'count' e palavra reservada da linguagem e nao pode ser usada como nome de variavel", 2 },
{ "post como nome de variavel com tipo",
  "dict post = {}\n", "", "SyntaxError: 'post' e palavra reservada da linguagem e nao pode ser usada como nome de variavel", 2 },
{ "in como nome de variavel com tipo",
  "str in = \"a\"\n", "", "SyntaxError: 'in' e palavra reservada da linguagem e nao pode ser usada como nome de variavel", 2 },
{ "palavra reservada como nome de parametro com tipo acusa o nome",
  "Null funct f(int count) {\n}\n", "", "SyntaxError: 'count' e palavra reservada da linguagem e nao pode ser usada como nome de parametro", 2 },
{ "palavra reservada como nome de campo",
  "class C {\n    int count = 0\n}\n", "", "SyntaxError: 'count' e palavra reservada da linguagem e nao pode ser usada como nome de campo", 2 },
/* A aspa de dentro fecha a string: a frase diz isso, na aspa. */
{ "aspa de dentro fecha a string na declaracao",
  "str s = \"ele disse \"oi\" pra mim\"\n", "",
  "SyntaxError: a aspa de dentro fechou a string: use a outra aspa por fora, ou escape a de dentro com \\", 2 },
{ "aspa de dentro fecha a string na lista",
  "list l = [\"ele disse \"oi\"\", \"b\"]\n", "",
  "SyntaxError: a aspa de dentro fechou a string: use a outra aspa por fora, ou escape a de dentro com \\", 2 },
/* Fechador sem abridor: a frase diz que ele sobrou. */
{ "parentese sobrando",
  "str nome = \"ana\"\npost(\"ola, \" + nome))\n", "", "SyntaxError: ')' sem '(' que abra: sobrou um ')'", 2 },
{ "colchete sobrando",
  "list l = [1, 2, 3]]\n", "", "SyntaxError: ']' sem '[' que abra: sobrou um ']'", 2 },
{ "parentese sobrando no if",
  "int x = 5\nif x > 1) {\n    post(1)\n}\n", "", "SyntaxError: ')' sem '(' que abra: sobrou um ')'", 2 },
{ "chave sobrando depois do bloco",
  "funct f() {\n    post(1)\n}\n}\n", "", "SyntaxError: '}' sem '{' que abra: sobrou um '}'", 2 },
/* Rodando sai o primeiro erro da lista do --check: o do parser na linha 2,
 * não o do lexer na linha 5. A linha é conferida no cli_roda.pr: com ela na
 * expectativa, a frase sem ela (a do diferencial) vira prefixo. */
{ "rodando sai o primeiro erro da lista do --check",
  "post(1)\nint x = = 1\npost(2)\npost(3)\nint y = 1 $ 2\n", "", "SyntaxError: expressao invalida\n", 2 },
{ "decorador sem nome tem frase propria",
  "@\nfunct f() {\n}\n", "", "SyntaxError: esperado nome do decorador depois de '@'", 2 },
{ "erro dentro da f-string sai na linha dele",
  "str x = \"a\"\npost(f'''linha1\n{x + }\nlinha3''')\n", "",
  "SyntaxError: expressao invalida dentro da f-string\n", 2 },
{ "erro do lexer dentro da f-string",
  "post(f\"{1 $ 2}\")\n", "", "SyntaxError: caractere inesperado: '$' dentro da f-string", 2 },

/* ── Plano 6, lote 3: as dicas ───────────────────────────────────────────────
 * Membro de instância pelo nome solto, dentro do método, sugere `self.x`; e a
 * dica nunca troca metade das letras (`valor` virava 'False'). */
{ "campo pelo nome solto no metodo sugere self.campo",
  "class Conta {\n    int saldo = 0\n    Null funct ver(self) {\n        post(saldo)\n    }\n}\nConta c = Conta()\nc.ver()\n",
  "", "NameError: name 'saldo' is not defined. Did you mean: 'self.saldo'?", 2 },
{ "campo do pai, dentro de funct aninhada no metodo, sugere self.campo",
  "class P {\n    int base_v = 1\n}\nclass F(P) {\n    Null funct ver(self) {\n        funct g() {\n            post(base_v)\n        }\n        g()\n    }\n}\nF().ver()\n",
  "", "NameError: name 'base_v' is not defined. Did you mean: 'self.base_v'?", 2 },
{ "self.x gravado em outro metodo tambem e membro: sugere self.x",
  "class A {\n    Null funct __init__(self) {\n        self.total = 1\n    }\n    Null funct ver(self) {\n        post(total)\n    }\n}\nA().ver()\n",
  "", "NameError: name 'total' is not defined. Did you mean: 'self.total'?", 2 },
{ "metodo static nao tem self: o campo de instancia sai sem dica",
  "class A {\n    int base_v = 1\n    @static\n    Null funct s() {\n        post(base_v)\n    }\n}\nA.s()\n",
  "", "NameError: name 'base_v' is not defined\n", 2 },
{ "dica nao troca metade das letras: valor nao vira False",
  "post(valor)\n", "", "NameError: name 'valor' is not defined\n", 2 },
{ "dica nao troca metade das letras: None nao vira Null",
  "bool b = None\npost(b)\n", "", "NameError: name 'None' is not defined\n", 2 },
{ "dica de letras invertidas continua",
  "int valor = 1\npost(vlaor)\n", "", "NameError: name 'vlaor' is not defined. Did you mean: 'valor'?", 2 },
{ "dica de uma letra a menos continua",
  "int total = 1\npost(totl)\n", "", "NameError: name 'totl' is not defined. Did you mean: 'total'?", 2 },

/* ── Plano 6, lote 9: o sys.exit roda os finally (doc 10.2) ──────────────────
 * Nenhum catch o pega (o sem tipo também não, nem a rede do int funct); os
 * finally em volta rodam, e o programa sai com o código pedido. */
{ "sys.exit no try roda o finally e o catch sem tipo nao pega",
  "import sys\ntry {\n    post(\"antes\")\n    sys.exit(3)\n} catch (e) {\n    post(\"pegou\")\n} finally {\n    post(\"finally\")\n}\npost(\"depois\")\n",
  "antes\nfinally", NULL, 3 },
{ "sys.exit nao e pego por catch (Exception e)",
  "import sys\ntry {\n    sys.exit(2)\n} catch (Exception e) {\n    post(\"pegou\")\n}\npost(\"depois\")\n",
  "", NULL, 2 },
{ "sys.exit atravessa a rede do int funct (nao vira 500)",
  "import sys\nint funct f() {\n    try {\n        sys.exit(4)\n    } finally {\n        int z = 0\n    }\n    return 1\n}\nint r = f()\npost(\"nao chega\", r)\n",
  "", NULL, 4 },
{ "sys.exit dentro de callback roda o finally de fora",
  "import sys\nint funct g(int x) {\n    if x == 2 {\n        sys.exit(5)\n    }\n    return x\n}\ntry {\n    list l = map([1, 2, 3], g)\n    post(l)\n} catch (Exception e) {\n    post(\"pegou\")\n} finally {\n    post(\"fin\")\n}\npost(\"depois\")\n",
  "fin", NULL, 5 },
{ "sys.exit dentro de gerador roda o finally dele",
  "import sys\nfunct gen() {\n    try {\n        yield 1\n        sys.exit(6)\n    } finally {\n        post(\"fin gen\")\n    }\n}\nfor each v in gen() {\n    post(v)\n}\npost(\"nao chega\")\n",
  "1\nfin gen", NULL, 6 },
{ "sys.exit dentro de tarefa async roda o finally dela",
  "import sys\nasync funct t() {\n    try {\n        sys.exit(7)\n    } finally {\n        post(\"fin tarefa\")\n    }\n}\nawait t()\npost(\"nao chega\")\n",
  "fin tarefa", NULL, 7 },
{ "sys.exit em tarefa que ninguem aguarda roda o finally dela e sai",
  "import sys\nasync funct t() {\n    try {\n        sleep(0.05)\n        sys.exit(8)\n    } finally {\n        post(\"fin tarefa solta\")\n    }\n}\nt()\nsleep(0.5)\npost(\"nao chega\")\n",
  "fin tarefa solta", NULL, 8 },
{ "sys.exit(0) sai com 0 sem mensagem",
  "import sys\npost(\"ok\")\nsys.exit(0)\npost(\"nao chega\")\n", "ok", NULL, 0 },

/* ── Plano 6, lote 10: o inteiro que não cabe em 64 bits ─────────────────────
 * "esperava int, recebeu int" se contradizia (o type() do valor responde int):
 * retorno, parâmetro e campo dizem o motivo e a palavra, como a variável. */
{ "retorno de int funct com inteiro grande diz que nao cabe em 64 bits",
  "int funct grande() {\n    return 2 ** 70\n}\npost(grande())\n", "500",
  "AttributedValueError: retorno de grande() esperava int, e o valor nao cabe em 64 bits (declare como 'long' pra aceitar inteiro de qualquer tamanho)", 0 },
{ "parametro int com inteiro grande diz que nao cabe em 64 bits",
  "funct f(int n) {\n    return n\n}\npost(f(2 ** 70))\n", "",
  "AttributedValueError: parâmetro n de f() esperava int, e o valor nao cabe em 64 bits (declare como 'long' pra aceitar inteiro de qualquer tamanho)", 1 },
{ "campo int com inteiro grande diz que nao cabe em 64 bits",
  "class C {\n    int x = 0\n}\nC c = C()\nc.x = 2 ** 70\n", "",
  "AttributedValueError: campo x de C esperava int, e o valor nao cabe em 64 bits (declare como 'long' pra aceitar inteiro de qualquer tamanho)", 1 },
{ "parametro int com texto continua dizendo o tipo recebido (rodando)",
  "funct f(int n) {\n    return n\n}\nlist l = [\"a\"]\npost(f(l[0]))\n", "",
  "AttributedValueError: parâmetro n de f() esperava int, recebeu str\n", 1 },

/* ── sonda 2, lote 3: vetores de salto sem teto e f-string inteira ────────── */
{ "65 breaks no mesmo laco e 65 elifs compilam (os vetores de salto crescem)",
  "import os\nimport sys\nstr b = \"int x = 0\\nwhile True {\\n\"\nfor each k in range(65) { b = b + \"    if x == \" + str(k + 100) + \" { break }\\n\" }\n"
  "b = b + \"    x++\\n    if x == 3 { break }\\n}\\npost(x)\\n\"\n"
  "str e = \"int y = 70\\nif y == 0 { post(0) }\\n\"\nfor each k in range(65) { e = e + \"elif y == \" + str(k + 1) + \" { post(\" + str(k + 1) + \") }\\n\" }\n"
  "e = e + \"else { post(\\\"outro\\\") }\\n\"\n"
  "using open(\"b.pr\", \"w\") as f { f.write(b) }\nusing open(\"e.pr\", \"w\") as f { f.write(e) }\n"
  "for each nome in [\"b.pr\", \"e.pr\"] {\n    Process p = os.run([sys.executable, nome], capture=\"live\")\n    str t = p.read(65536)\n    int rc = p.wait()\n    post(nome, rc, t.strip())\n}\n",
  "b.pr 0 3\ne.pr 0 outro", NULL, 0 },
{ "literais de lista, tupla e dict nas bordas do lote (255, 256, 257, 512, 513 itens) e chave repetida entre lotes",
  /* acima de 256 itens o literal e montado em lotes (emite_literal_seq): a
   * ordem, o tipo, o tamanho e o "ultima chave repetida ganha" sao os mesmos */
  "import os\nimport sys\n"
  "str funct uns(int n) {\n    list p = []\n    for each i in range(1, n + 1) { p.append(str(i)) }\n    return \", \".join(p)\n}\n"
  "str funct pares(int n) {\n    list p = []\n    for each i in range(1, n + 1) { p.append(\"\\\"k\" + str(i) + \"\\\": \" + str(i)) }\n    return \", \".join(p)\n}\n"
  "str src = \"\"\n"
  "for each n in [255, 256, 257, 512, 513] {\n"
  "    str s = str(n)\n"
  "    src = src + \"list a\" + s + \" = [\" + uns(n) + \"]\\n\"\n"
  "    src = src + \"tup b\" + s + \" = (\" + uns(n) + \")\\n\"\n"
  "    src = src + \"dict c\" + s + \" = {\" + pares(n) + \", \\\"k1\\\": -1}\\n\"\n"
  "    src = src + \"post(len(a\" + s + \"), a\" + s + \"[\" + str(n - 1) + \"], type(b\" + s + \"), len(b\" + s + \"), b\" + s + \"[\" + str(n - 1) + \"], len(c\" + s + \"), c\" + s + \"[\\\"k\" + s + \"\\\"], c\" + s + \"[\\\"k1\\\"])\\n\"\n"
  "}\n"
  "src = src + \"tup tt = ((\" + uns(300) + \"), (\" + uns(300) + \"))\\npost(type(tt), len(tt), len(tt[1]), tt[1][299])\\n\"\n"
  "using open(\"b.pr\", \"w\") as f { f.write(src) }\n"
  "Process p = os.run([sys.executable, \"b.pr\"], capture=\"live\")\n"
  "str t = p.read(65536)\nint rc = p.wait()\npost(rc)\npost(t.strip())\n",
  "0\n255 255 tup 255 255 255 255 -1\n256 256 tup 256 256 256 256 -1\n257 257 tup 257 257 257 257 -1\n512 512 tup 512 512 512 512 -1\n513 513 tup 513 513 513 513 -1\ntup 2 300 300", NULL, 0 },
/* ── a pilha de valores balanceada em todo caminho (16.2.13) ─────────────────
 * O passo que mede `pilha_max` exige que dois caminhos cheguem a uma mesma
 * instrucao com a mesma profundidade (invariante no binario de debug), e foi
 * ele que achou estes: o `continue` num `case` deixava o sujeito do `match`
 * em cima de [container, indice] e o ITER_NEXT lia lixo ("'int' object is
 * not iterable"); `break`/`continue` dentro de um `finally` no caminho da
 * excecao deixavam a mensagem e o tipo, e o `continue` perdia iteracoes. */
{ "continue dentro de case num for each nao corrompe o laco (o sujeito do match sai da pilha)",
  "for each i in [1, 2, 3] {\n    match i {\n        case 2 { continue }\n        case _ { post(i) }\n    }\n}\npost(\"fim\")\n",
  "1\n3\nfim", NULL, 0 },
{ "break dentro de case num for each sai limpo",
  "for each i in [1, 2, 3] {\n    match i {\n        case 2 { break }\n        case _ { post(i) }\n    }\n}\npost(\"fim\")\n",
  "1\nfim", NULL, 0 },
{ "continue dentro de finally no caminho da excecao roda todas as iteracoes",
  "for each i in [1, 2, 3] {\n    try {\n        raise Boom(\"e\")\n    } finally {\n        post(\"f\", i)\n        continue\n    }\n}\npost(\"fim\")\n",
  "f 1\nf 2\nf 3\nfim", NULL, 0 },
{ "break dentro de finally no caminho da excecao, so quando a excecao acontece",
  "for each i in [1, 2, 3] {\n    try {\n        if i == 2 { raise Boom(\"e\") }\n    } finally {\n        if i == 2 { break }\n    }\n    post(\"i\", i)\n}\npost(\"fim\")\n",
  "i 1\nfim", NULL, 0 },
{ "break dentro de finally abandona o return do try, e a funct segue depois do laco",
  "int funct g() {\n    for each i in [1, 2] {\n        try {\n            return 1\n        } finally {\n            break\n        }\n    }\n    return 5\n}\npost(g())\n",
  "5", NULL, 0 },
{ "break dentro de finally com um try aninhado por dentro, no caminho da excecao",
  "for each i in [1, 2, 3] {\n    try {\n        try {\n            if i == 2 { raise Boom(\"e\") }\n        } finally {\n            post(\"in\", i)\n        }\n    } catch (e) {\n        post(\"c\", i)\n    } finally {\n        if i == 2 { break }\n    }\n}\npost(\"fim\")\n",
  "in 1\nin 2\nc 2\nfim", NULL, 0 },
/* ── o handler limpa o que o bloco do try criou (16.2.14) ────────────────────
 * Os CLEAR do fim do bloco so rodavam no caminho normal: o salto pro handler
 * os pulava, e o slot reaproveitado depois (a variavel do catch, a dica de uma
 * global lida) ainda guardava o valor do local do try. */
{ "global lida no catch vale o que tem, nao o valor de um local do try",
  "str padrao = \"desconhecido\"\nfunct nome_do_usuario(dict d) {\n    try {\n        str prefixo = \"sr. \"\n        str sobrenome = \"silva\"\n        str nome = d[\"nome\"]\n        return prefixo + nome + \" \" + sobrenome\n    } catch (KeyError e) {\n        return padrao\n    }\n}\npost(nome_do_usuario({\"nome\": \"ana\"}))\npost(nome_do_usuario({}))\n",
  "sr. ana silva\ndesconhecido", NULL, 0 },
{ "global int lida no catch depois de dois locais do try",
  "int limite = 10\nint funct f() {\n    try {\n        int a = 1\n        int b = 99\n        raise Boom(\"x\")\n    } catch (e) {\n        return limite\n    }\n}\npost(f())\n",
  "10", NULL, 0 },
{ "globais lidas DEPOIS do try/catch valem o que tem",
  "int limite = 10\nint extra = 1\nint funct f() {\n    try {\n        int a = 1000\n        int b = 77\n        raise Boom(\"x\")\n    } catch (e) {\n    }\n    return limite + extra\n}\npost(f())\n",
  "11", NULL, 0 },
{ "global lida no catch dentro de metodo",
  "int piso = 3\nclass K {\n    int funct m(self) {\n        try {\n            int v = 50\n            raise Boom(\"x\")\n        } catch (e) {\n            return piso\n        }\n    }\n}\npost(K().m())\n",
  "3", NULL, 0 },
{ "post no catch chama o post, nao o valor que um local do try deixou no slot",
  "Null funct f() {\n    try {\n        Object o = len\n        raise Boom(\"x\")\n    } catch (e) {\n        post(\"pegou\")\n    }\n}\nf()\n",
  "pegou", NULL, 0 },
{ "erro saindo do catch pro finally: o post do finally e o post, e o erro novo propaga",
  "Null funct f() {\n    try {\n        raise Quebrou(\"a\")\n    } catch (Quebrou e) {\n        raise Outro(\"do catch\")\n    } finally {\n        post(\"finally\")\n    }\n}\ntry {\n    f()\n} catch (Outro e) {\n    post(\"fora\")\n}\n",
  "finally\nfora", NULL, 0 },
{ "no topo, a variavel do try nao existe depois do bloco quando houve erro",
  "try {\n    int vazou = 5\n    raise Quebrou(\"x\")\n} catch (Quebrou e) {\n    post(\"pegou\")\n}\npost(vazou)\n",
  "pegou", "NameError: name 'vazou' is not defined\n", 1 },
{ "local de um bloco de dentro do try tambem e limpo no handler",
  "int g = 7\nint funct f(bool c) {\n    try {\n        if c {\n            int dentro = 123\n            str outro = \"zzz\"\n        }\n        raise Boom(\"x\")\n    } catch (e) {\n        return g\n    }\n}\npost(f(True))\n",
  "7", NULL, 0 },
/* ── os try ativos crescem (16.2.14) ─────────────────────────────────────────
 * Eram 256 por execucao, e cada `int funct`/`bool funct` ativa gasta um (a
 * rede dela): a recursao de 257 niveis saia "try aninhado demais", a rede do
 * chamador engolia e a conta seguia com 500 no meio. */
{ "int funct e bool funct recursivas passam de 256 niveis com o resultado certo",
  "int funct soma(int n) {\n    if n == 0 { return 0 }\n    return n + soma(n - 1)\n}\npost(soma(300))\nbool funct todos_pares(list l, int i) {\n    if i == len(l) { return True }\n    if l[i] % 2 != 0 { return False }\n    return todos_pares(l, i + 1)\n}\nlist pares = []\nfor each k in range(300) { pares.append(k * 2) }\npost(todos_pares(pares, 0))\npost(soma(5000))\n",
  "45150\nTrue\n12502500", NULL, 0 },
{ "recursao sem fim em int funct e RecursionError que o catch pega (a rede do int nao a engole)",
  "int funct f(int n) {\n    if n == 0 { return 0 }\n    return 1 + f(n - 1)\n}\ntry {\n    post(f(1000000))\n} catch (RecursionError e) {\n    post(\"pego:\", e)\n}\npost(\"segue\")\n",
  "pego: maximum recursion depth exceeded (linha 3)\nsegue", NULL, 0 },
{ "recursao sem fim em int funct sem catch encerra com RecursionError, nao com um numero errado",
  "int funct f(int n) {\n    return 1 + f(n + 1)\n}\npost(f(0))\npost(\"nunca\")\n",
  "", "RecursionError: maximum recursion depth exceeded\n", 1 },
{ "300 try aninhados e um gerador com 20 try em volta do yield (os handlers crescem e voltam todos na retomada)",
  "import os\nimport sys\n"
  "using open(\"t.pr\", \"w\") as f { f.write(\"try {\\n\" * 300 + \"post(1)\\nraise Boom(\\\"x\\\")\\n\" + \"} catch (Outro e) { post(2) }\\n\" * 299 + \"} catch (Boom e) { post(\\\"pego\\\") }\\n\") }\n"
  "using open(\"g.pr\", \"w\") as f { f.write(\"funct g() {\\n\" + \"try {\\n\" * 20 + \"yield 1\\nyield 2\\n\" + \"} finally { post(\\\"f\\\") }\\n\" * 20 + \"}\\nfor each x in g() { post(x) }\\n\") }\n"
  "for each nome in [\"t.pr\", \"g.pr\"] {\n"
  "    Process p = os.run([sys.executable, nome], capture=\"live\")\n"
  "    str t = p.read(65536)\n    int rc = p.wait()\n"
  "    post(nome, rc, t.strip().replace(\"\\n\", \" \"))\n}\n",
  "t.pr 0 1 pego\ng.pr 0 1 2 f f f f f f f f f f f f f f f f f f f f", NULL, 0 },
{ "f-string: o que sobra depois da expressao dentro de {} e erro, nao descarte calado",
  "str nome = \"Ana\"\nstr sobrenome = \"Lima\"\npost(f\"cliente: {nome sobrenome}\")\n", "",
  "SyntaxError: sobrou 'sobrenome' dentro de {} da f-string: cada {} leva uma expressao so", 2 },
{ "f-string: `{x = }`, `{x)}` e nome que nao existe depois da expressao",
  "flo preco = 2.5\npost(f\"debug: {preco = }\")\n", "", "sobrou '= ' dentro de {} da f-string", 2 },
{ "f-string: parentese sobrando dentro das chaves",
  "int qtd = 4\npost(f\"fecha: {qtd)}\")\n", "", "sobrou ')' dentro de {} da f-string", 2 },
{ "f-string: segundo nome dentro das chaves nao some (antes nem NameError dava)",
  "str nome = \"Ana\"\npost(f\"nada: {nome naoexiste}\")\n", "", "sobrou 'naoexiste' dentro de {} da f-string", 2 },
{ "base() com posicional depois de nomeado e SyntaxError, rc 2 (era rc 3)",
  "Entity A { int x\n    funct __init__(self, x, b) { self.x = x } }\n"
  "Entity B(A) {\n    funct __init__(self) { base().__init__(1, b=2, 3) }\n}\nB()\n", "",
  "SyntaxError: argumento posicional depois de nomeado", 2 },
/* Limite do motor continua rc 3, mas a frase diz que e limite — antes o
 * estouro de 32 lacos era SILENCIOSO: o chamador escrevia no laco de fora e
 * break/continue saltavam pro lugar errado. */
{ "33 lacos aninhados: limite do compilador, acusado",
  "for each a1 in [1] {\nfor each a2 in [1] {\nfor each a3 in [1] {\nfor each a4 in [1] {\n"
  "for each a5 in [1] {\nfor each a6 in [1] {\nfor each a7 in [1] {\nfor each a8 in [1] {\n"
  "for each a9 in [1] {\nfor each a10 in [1] {\nfor each a11 in [1] {\nfor each a12 in [1] {\n"
  "for each a13 in [1] {\nfor each a14 in [1] {\nfor each a15 in [1] {\nfor each a16 in [1] {\n"
  "for each a17 in [1] {\nfor each a18 in [1] {\nfor each a19 in [1] {\nfor each a20 in [1] {\n"
  "for each a21 in [1] {\nfor each a22 in [1] {\nfor each a23 in [1] {\nfor each a24 in [1] {\n"
  "for each a25 in [1] {\nfor each a26 in [1] {\nfor each a27 in [1] {\nfor each a28 in [1] {\n"
  "for each a29 in [1] {\nfor each a30 in [1] {\nfor each a31 in [1] {\nfor each a32 in [1] {\n"
  "for each a33 in [1] {\npost(1)\n}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}\n", "",
  "NotImplementedError: limite do compilador: lacos aninhados demais", 3 },
/* Literal em base que estoura o int64 virava INT64_MAX calado: o lexer
 * guardava o texto cru e o parser relia em base 10. */
{ "literal hex/bin maior que int64 vira bignum",
  "post(0xFFFFFFFFFFFFFFFFFF)\npost(0b1111111111111111111111111111111111111111111111111111111111111111111)\n",
  "4722366482869645213695\n147573952589676412927", NULL, 0 },
{ "literal em base pequeno segue int, com separador",
  "post(0x1F, 0o17, 0b101, 0xFF_FF)\nx = 0b101\npost(type(x))\nlong g = 0xFFFFFFFFFFFFFFFFFF\npost(type(g))\n",
  "31 15 5 65535\nint\nint", NULL, 0 },
/* As mensagens de `to` e `count` enumeravam 3 de 9 e 7 de 9 tipos. */
{ "to: a frase enumera a lista inteira",
  "post(to xyz)\n", "",
  "esperado tipo apos 'to': int, float, str, flo, bool, json, list, tup, dict, long ou char", 2 },
{ "to aceita long e char",
  "post(to long, to char)\n", "long char", NULL, 0 },
{ "count: a frase enumera a lista inteira",
  "post(count xyz in [1])\n", "",
  "esperado tipo apos 'count': str, int, flo, bool, list, json, dict, tup, char ou type", 2 },
/* Eram duas listas: o prefixo aceitava `char` e nao `type`, o infixo o
 * contrario. Agora e uma (`eh_tipo_count`), a uniao. */
{ "count aceita char e type nas duas formas",
  "post(count char(\"a\") in \"banana\")\npost(count type in [1, int, str])\npost(char(\"a\") count in \"banana\")\n",
  "3\n2\n3", NULL, 0 },
/* A cabeca "faltou funct" so conhecia str/int/flo/bool. */
{ "list f(x) { diz que faltou funct",
  "list f(x) {\n    return x\n}\n", "",
  "faltou 'funct' antes de 'f': list funct f(...)", 2 },
/* Justaposicao: `int(`/`str(`/`flo(` como argumento colado. So a forma de
 * CHAMADA — o tipo solto nao abre argumento, senao `f(x` engolia a linha de
 * baixo. */
{ "justaposicao aceita int(x), str(x) e flo(x)",
  "post(\"v: \" int(3))\npost(\"n: \" str(2) \" \" flo(1))\n",
  "v:  3\nn:  2   1.0", NULL, 0 },
/* `length` era so um IDENT qualquer: `str(tamanho=10)` passava e o numero
 * ia pro slot de length sem ninguem ter escrito length. */
{ "campo de model: parametro que nao existe e acusado com a lista dos que existem",
  "model M() {\n    nome: str(tamanho=10)\n}\n", "",
  "parametro 'tamanho' nao existe no campo do model: length, regex, in, not_in, min, max, optional, of", 2 },
{ "campo de model com length= segue valendo",
  "model M() {\n    nome: str(length=10)\n}\npost(\"ok\")\n", "ok", NULL, 0 },

/* ── model valida DADO, nao so estrutura (2026-09-20): regex, in/not_in,
 * min/max, optional, of, model aninhado. O `==` responde True/False; a frase
 * de cada falha e medida em teste/jinker_roda.pr (rota `model=`). ──────── */
{ "model: regex no campo str — casa e nao casa",
  "model M() {\n    cep: str(regex=\"^[0-9]{5}-[0-9]{3}$\")\n}\n"
  "post({\"cep\": \"12345-678\"} == M, {\"cep\": \"1234\"} == M)\n", "True False", NULL, 0 },
{ "model: in (so estes) e not_in (proibidos)",
  "model M() {\n    p: str(in=[\"a\", \"b\"])\n    s: str(not_in=[\"x\"])\n}\n"
  "post({\"p\": \"a\", \"s\": \"y\"} == M, {\"p\": \"c\", \"s\": \"y\"} == M, {\"p\": \"a\", \"s\": \"x\"} == M)\n",
  "True False False", NULL, 0 },
{ "model: in com int e bool, com sinal",
  "model M() {\n    n: int(in=[-1, 2])\n    b: bool(in=[True])\n}\n"
  "post({\"n\": -1, \"b\": True} == M, {\"n\": 3, \"b\": True} == M, {\"n\": 2, \"b\": False} == M)\n",
  "True False False", NULL, 0 },
{ "model: min e max em int e flo (faixa inclusiva)",
  "model M() {\n    i: int(min=1, max=3)\n    f: flo(min=0.5)\n}\n"
  "post({\"i\": 3, \"f\": 0.5} == M, {\"i\": 4, \"f\": 1.0} == M, {\"i\": 2, \"f\": 0.1} == M)\n",
  "True False False", NULL, 0 },
{ "model: optional=True pode faltar ou vir Null; presente, o tipo vale",
  "model M() {\n    a: int\n    b: str(optional=True)\n}\n"
  "post({\"a\": 1} == M, {\"a\": 1, \"b\": Null} == M, {\"a\": 1, \"b\": 2} == M, {\"b\": \"x\"} == M)\n",
  "True True False False", NULL, 0 },
{ "model: aninhado (tipo = outro model) e list(of=...) item a item, com length de itens",
  "model E() {\n    r: str\n}\nmodel M() {\n    e: E\n    l: list(of=int, length=2)\n    ms: list(of=E)\n}\n"
  "post({\"e\": {\"r\": \"x\"}, \"l\": [1, 2], \"ms\": [{\"r\": \"y\"}]} == M)\n"
  "post({\"e\": {\"r\": 1}, \"l\": [1], \"ms\": []} == M)\n"
  "post({\"e\": {\"r\": \"x\"}, \"l\": [1, 2, 3], \"ms\": []} == M)\n"
  "post({\"e\": {\"r\": \"x\"}, \"l\": [\"a\"], \"ms\": []} == M)\n"
  "post({\"e\": {\"r\": \"x\"}, \"l\": [], \"ms\": [{\"r\": 2}]} == M)\n"
  "post({\"e\": \"x\", \"l\": [], \"ms\": []} == M)\n",
  "True\nFalse\nFalse\nFalse\nFalse\nFalse", NULL, 0 },
{ "model: dict livre e list sem of aceitam qualquer conteudo, mas exigem o tipo",
  "model M() {\n    d: dict\n    l: list\n}\n"
  "post({\"d\": {\"a\": 1}, \"l\": [1, \"x\"]} == M, {\"d\": [], \"l\": []} == M)\n", "True False", NULL, 0 },
{ "model: o model usado como tipo pode ser declarado DEPOIS",
  "model M() {\n    e: E\n}\nmodel E() {\n    r: str\n}\npost({\"e\": {\"r\": \"x\"}} == M)\n", "True", NULL, 0 },
{ "model: str() e f-string mostram o nome (saia vazio)",
  "model M() {\n    a: int\n}\npost(str(M), f\"{M}\")\n", "<model M> <model M>", NULL, 0 },
{ "model: regex invalida e acusada na linha do campo, antes de rodar",
  "model M() {\n    a: str(regex=\"[\")\n}\n", "", "regex invalida no campo a:", 2 },
{ "model: regex so vale em str",
  "model M() {\n    a: int(regex=\"x\")\n}\n", "", "parametro 'regex' so vale em str (campo a)", 2 },
{ "model: in exige literais do tipo do campo",
  "model M() {\n    a: int(in=[\"x\"])\n}\n", "", "parametro 'in' do campo a: cada valor tem que ser um literal int", 2 },
{ "model: min maior que max e erro",
  "model M() {\n    a: int(min=5, max=1)\n}\n", "", "campo a: min maior que max", 2 },
{ "model: tipo que nao e da tabela nem model e acusado",
  "model M() {\n    a: Nada\n}\n", "", "tipo desconhecido em model: Nada", 2 },
{ "model: of com tipo desconhecido",
  "model M() {\n    a: list(of=Nada)\n}\n", "", "parametro 'of' do campo a: tipo desconhecido Nada", 2 },
{ "model: of so vale em list",
  "model M() {\n    a: str(of=int)\n}\n", "", "parametro 'of' so vale em list (campo a)", 2 },
{ "model: parametro repetido",
  "model M() {\n    a: str(length=1, length=2)\n}\n", "", "parametro 'length' repetido no campo do model", 2 },
{ "model: valor de parametro tem que ser literal",
  "x = 3\nmodel M() {\n    a: int(min=x)\n}\n", "", "parametro 'min' do campo a: so numero literal", 2 },

/* Membro com o nome errado: a frase APONTA o nome incorreto sugerindo o
 * parecido (o "Did you mean" que a frase de modulo ja tinha), antes de rodar
 * e rodando, em tipo nativo e em Entity. Um `raplace` num str do painel
 * dele passou por "o replace sumiu" — a frase so dizia "'str' object".
 * Numa CHAMADA (`x.m()`) a frase diz "method"; numa leitura (`x.m`),
 * "attribute": dizer "attribute" num `.send()` mandava procurar um campo. */
{ "membro errado em str: sugere o certo, antes de rodar",
  "s = \"abc\"\npost(s.raplace(\"a\", \"b\"))\n", "",
  "AttributeError: 'str' object has no method 'raplace'. Did you mean: 'replace'?", 2 },
{ "membro errado em str: a mesma frase rodando (tipo so se sabe na chamada)",
  "funct f(s) {\n    return s.raplace(\"a\", \"b\")\n}\npost(f(\"abc\"))\n", "",
  "AttributeError: 'str' object has no method 'raplace'. Did you mean: 'replace'?", 1 },
{ "membro errado em list sugere o metodo de list",
  "xs = [1]\nxs.apend(2)\n", "", "'list' object has no method 'apend'. Did you mean: 'append'?", 2 },
{ "membro errado numa Entity: sugere o metodo, antes de rodar",
  "class Conta {\n    funct __init__(self) { self.saldo = 1 }\n    funct extrato(self) { return self.saldo }\n}\n"
  "c = Conta()\npost(c.extrat())\n", "",
  "AttributeError: 'Conta' object has no method 'extrat'. Did you mean: 'extrato'?", 2 },
{ "membro errado numa Entity vindo de parametro sem tipo: a mesma frase rodando",
  "class Conta {\n    funct __init__(self) { self.saldo = 1 }\n    funct extrato(self) { return self.saldo }\n}\n"
  "funct f(c) {\n    return c.extrat()\n}\npost(f(Conta()))\n", "",
  "AttributeError: 'Conta' object has no method 'extrat'. Did you mean: 'extrato'?", 1 },
{ "campo errado numa Entity: sugere o campo gravado em self",
  "class Conta {\n    funct __init__(self) { self.saldo = 1 }\n}\nc = Conta()\npost(c.sald)\n", "",
  "'Conta' object has no attribute 'sald'. Did you mean: 'saldo'?", 2 },
{ "metodo static errado pela classe: sugere",
  "class K {\n    static funct dobro(n) { return n * 2 }\n}\npost(K.dobr(2))\n", "",
  "'K' object has no method 'dobr'. Did you mean: 'dobro'?", 2 },
{ "nome sem parecido nenhum: a frase fica como era",
  "s = \"abc\"\npost(s.zzzz())\n", "", "AttributeError: 'str' object has no method 'zzzz'", 2 },

/* O `.type()` saiu no 18.0.0 (motor-14, decisão do Kleber: "o uso deve ser
 * somente type() nao .type()"): fica o builtin `type(x)`. Em todo tipo de
 * valor, `x.type()` é membro que não existe, com a frase de qualquer outro,
 * antes de rodar (rc 2) e rodando (rc 1, o tipo só se sabe na chamada); num
 * dict, `d.type` é a chave, como todo `d.chave`. E o `type(x)` de cada tipo
 * de valor dá o que sempre deu. */
{ ".type() saiu: int", "post(1.type())\n", "", "AttributeError: 'int' object has no method 'type'", 2 },
{ ".type() saiu: str", "post(\"a\".type())\n", "", "AttributeError: 'str' object has no method 'type'", 2 },
{ ".type() saiu: list", "post([1].type())\n", "", "AttributeError: 'list' object has no method 'type'", 2 },
{ ".type() saiu: tup", "post((1, 2).type())\n", "", "AttributeError: 'tup' object has no method 'type'", 2 },
{ ".type() saiu: Null", "post(Null.type())\n", "", "AttributeError: 'Null' object has no method 'type'", 2 },
{ ".type() saiu: instancia de Entity",
  "class A {\n    int x = 1\n}\nA a = A()\npost(a.type())\n", "",
  "AttributeError: 'A' object has no method 'type'", 2 },
{ ".type() saiu: enum",
  "enum Cor {\n    RED,\n    GREEN\n}\npost(Cor.type())\n", "",
  "AttributeError: type object 'Cor' has no attribute 'type'", 2 },
{ ".type() saiu: funct",
  "Null funct g() {\n    post(1)\n}\ng.type()\n", "", "AttributeError: 'funct' object has no method 'type'", 2 },
{ ".type() saiu: funcao de modulo",
  "import sockets\nsockets.socket.type()\n", "", "AttributeError: 'funct' object has no method 'type'", 2 },
{ ".type() saiu: metodo ligado",
  "post(\"a\".upper.type())\n", "", "AttributeError: 'funct' object has no method 'type'", 2 },
{ ".type sem chamar: attribute", "post(1.type)\n", "", "AttributeError: 'int' object has no attribute 'type'", 2 },
{ ".type() saiu: a mesma frase rodando",
  "funct f(x) {\n    return x.type()\n}\npost(f(1))\n", "", "AttributeError: 'int' object has no method 'type'", 1 },
{ ".type() num dict e a chave type", "dict d = {\"a\": 1}\npost(d.type())\n", "", "KeyError: 'type'", 1 },
{ "type(x) de cada tipo de valor",
  "import sockets\nclass A {\n    int x = 1\n}\nenum Cor {\n    RED,\n    GREEN\n}\n"
  "post(type(1), type(\"a\"), type([1]), type((1, 2)), type({\"a\": 1}), type(Null), type(True), type(1.5), type(b\"x\"))\n"
  "post(type(A()), type(A), type(Cor), type(Cor.RED), type(len), type(sockets), type(sockets.socket))\n",
  "int str list tup dict Null bool flo byte\nA Entity enum int funct module funct", NULL, 0 },

/* Nome de módulo não é tipo (motor-15): `bytes d = b"x"` dizia `esperava
 * bytes, recebeu byte`, como se o tipo `bytes` existisse (o valor binário é
 * `byte`; `bytes` é o módulo). O checador perguntava se o nome "tinha o
 * membro type", e pra módulo a resposta era "não tem", não "não sei". */
{ "nome de modulo nao e tipo: bytes", "bytes d = b\"x\"\npost(d)\n", "",
  "AttributedValueError: tipo bytes não existe (variável d)", 2 },
{ "nome de modulo nao e tipo: os", "import os\nos x = os\npost(x)\n", "",
  "AttributedValueError: tipo os não existe (variável x)", 2 },
{ "o tipo do valor binario e byte", "byte d = b\"x\"\npost(type(d))\n", "byte", NULL, 0 },

/* Valor sem membro nenhum (motor-16): funct e Null. O `--check` aceitava
 * qualquer membro deles (`sockets.socket.sendall(s, d)`, a forma do Python
 * de chamar o método pela classe), e o motor recusava rodando. */
{ "membro de funct de modulo: erro antes de rodar",
  "import sockets\nsockets.socket.sendall(1, \"x\")\n", "",
  "AttributeError: 'funct' object has no method 'sendall'", 2 },
{ "membro de funct do arquivo: erro antes de rodar",
  "Null funct g() {\n    post(1)\n}\ng.foo()\n", "", "AttributeError: 'funct' object has no method 'foo'", 2 },
{ "membro lido de metodo ligado: erro antes de rodar",
  "post(\"a\".upper.foo)\n", "", "AttributeError: 'funct' object has no attribute 'foo'", 2 },
{ "membro de Null: erro antes de rodar", "post(Null.foo())\n", "",
  "AttributeError: 'Null' object has no method 'foo'", 2 },
{ "membro do Null que a funct devolve: erro antes de rodar",
  "Null funct f() {\n    post(1)\n}\nf().foo()\n", "", "AttributeError: 'Null' object has no method 'foo'", 2 },
/* O nome escrito e a PRIMEIRA PALAVRA de um membro (`send` -> `send_linux`):
 * pela distancia de edicao ficava longe demais e o erro saia sem dica. Dois
 * membros com a mesma primeira palavra = nao se sabe qual, e nao sugere. */
{ "metodo chamado pela primeira palavra do nome sugere o membro, antes de rodar",
  "class N {\n    funct send_linux(self) { return 1 }\n}\nN().send()\n", "",
  "AttributeError: 'N' object has no method 'send'. Did you mean: 'send_linux'?", 2 },
{ "metodo chamado pela primeira palavra: a mesma frase rodando",
  "class N {\n    funct send_linux(self) { return 1 }\n}\nfunct f(n) {\n    return n.send()\n}\nf(N())\n", "",
  "AttributeError: 'N' object has no method 'send'. Did you mean: 'send_linux'?", 1 },
{ "primeira palavra de dois membros: nao sugere",
  "class N {\n    funct send_linux(self) { return 1 }\n    funct send_mac(self) { return 2 }\n}\nN().send()\n", "",
  "AttributeError: 'N' object has no method 'send'\n", 2 },
{ "leitura sem chamar continua attribute, com a mesma sugestao",
  "class N {\n    funct send_linux(self) { return 1 }\n}\nx = N().send\n", "",
  "AttributeError: 'N' object has no attribute 'send'. Did you mean: 'send_linux'?", 2 },

/* ── L3 DO PLANO DAS CONTRADICOES: regex (2026-09-12) — strings medidas ──── */
/* `a{1}?` era recusado com "multiple repeat": a guarda do preguicoso olhava
 * `min != 1 || max != 1`, e `{1}` deixa os dois em 1. */
{ "a{1}? e o {1} preguicoso, nao repeticao de repeticao",
  "import regex\npost(regex.findall(\"a{1}?\", \"aaa\"))\n", "['a', 'a', 'a']", NULL, 0 },
/* O retrovisor dobrava caixa por BYTE, so A-Z; literal e classe dobravam por
 * codepoint (Latin-1). Uma regra so agora. */
{ "retrovisor com (?i) casa acento nas duas caixas",
  "import regex\npost(regex.findall(\"(?i)(é)\\\\1\", \"éÉ\"))\npost(regex.findall(\"(?i)(ab)\\\\1\", \"abAB\"))\n",
  "['é']\n['ab']", NULL, 0 },
/* Pattern.sub tinha "argument 1" fixo e imprimia o tipo do 1o mesmo quando o
 * errado era o 2o. */
{ "Pattern.sub diz QUAL argumento veio errado",
  "import regex\nregex.compile(\"a\").sub(\"x\", 5)\n", "",
  "TypeError: sub() argument 2 must be str, not int", 1 },
/* Grupo inexistente na substituicao saia TypeError. O nome que nao existe e
 * IndexError, a classe que o `re` da (o resto do texto de troca invalido e
 * ValueError, como o padrao invalido). */
{ "nome de grupo inexistente na substituicao e IndexError, como no re",
  "import regex\ntry { regex.sub(\"(a)\", \"\\\\g<zz>\", \"xax\") } catch (IndexError e) { post(\"pegou:\", e) }\n",
  "pegou: unknown group name 'zz' (linha 2)", NULL, 0 },
/* fullmatch e match sao a mesma funcao, mas cada uma assina o proprio nome. */
{ "fullmatch assina fullmatch no erro, nao match",
  "import regex\nregex.fullmatch(\"a\")\n", "",
  "TypeError: fullmatch() takes exactly 2 arguments (1 given)", 2 },
/* A tabela anunciava `flags` em sete funcoes e nenhuma o lia: por nome dava
 * keyword invalido, por posicao o compile ENGOLIA o 2o argumento calado. */
{ "regex.compile nao aceita mais um 2o argumento que era ignorado",
  "import regex\nregex.compile(\"a\", 0)\n", "",
  "TypeError: compile() takes exactly one argument (2 given)", 2 },
{ "flags= nao existe em regex.match",
  "import regex\nregex.match(\"a\", \"a\", flags=0)\n", "",
  "'flags' is an invalid keyword argument for match()", 2 },
{ "match e fullmatch seguem casando a string inteira",
  "import regex\npost(regex.fullmatch(\"a+\", \"aaa\"), regex.match(\"a\", \"aaa\"))\n",
  "True False", NULL, 0 },

/* ── L4a DO PLANO DAS CONTRADICOES: nativas, aridade e tipo (2026-09-12) ──
 * Strings medidas no binario. As nativas que exigem list/tup diziam "'str'
 * object is not iterable" sobre tipos que o motor ITERA (str, bytes, dict);
 * agora dizem o que pedem e o tipo que veio, numa funcao so (arg_exige_seq
 * / arg_exige_dict). Nada que rodava mudou: str segue recusado. */
{ "sum recusa str dizendo o que pede, nao negando que str itera",
  "sum(\"abc\")\n", "", "TypeError: sum() argument 1 must be list or tup, not str", 2 },
{ "sum recusa bytes pelo mesmo caminho",
  "sum(\"abc\".encode())\n", "", "TypeError: sum() argument 1 must be list or tup, not byte", 2 },
{ "str.join diz o tipo recebido",
  "\", \".join(\"abc\")\n", "", "TypeError: join() argument 1 must be list or tup, not str", 2 },
{ "str.join recusa dict",
  "\", \".join({\"a\": 1})\n", "", "TypeError: join() argument 1 must be list or tup, not dict", 2 },
{ "list.extend recusa str dizendo o que pede",
  "l = [1]\nl.extend(\"ab\")\n", "", "TypeError: extend() argument 1 must be list or tup, not str", 2 },
{ "dict.update recusa list dizendo que pede dict",
  "d = {}\nd.update([1])\n", "", "TypeError: update() argument 1 must be dict, not list", 2 },
{ "translate recusa str dizendo que pede dict, nao 'not subscriptable'",
  "\"abc\".translate(\"xyz\")\n", "", "TypeError: translate() argument 1 must be dict, not str", 2 },
{ "map diz o que pede no 1o argumento",
  "map(5, funct(x) { return x })\n", "", "TypeError: map() argument 1 must be list or tup, not int", 1 },
{ "filter diz o que pede no 1o argumento",
  "filter(\"ab\", funct(x) { return x })\n", "", "TypeError: filter() argument 1 must be list or tup, not str", 1 },
/* dict(): sequencia com tamanho errado dizia "cannot convert ... to a
 * sequence" — o contrario do fato, e sem dizer que precisa de 2. */
{ "dict(): par com tamanho errado diz o tamanho e que precisa de 2",
  "dict([[1, 2, 3]])\n", "", "TypeError: dictionary update sequence element #0 has length 3; 2 is required", 1 },
{ "dict(): par de 1 elemento",
  "dict([[1]])\n", "", "TypeError: dictionary update sequence element #0 has length 1; 2 is required", 1 },
{ "dict(): item que nao e sequencia segue com a frase antiga",
  "dict([5])\n", "", "TypeError: cannot convert dictionary update sequence element #0 to a sequence", 1 },
/* min comparava com `<` e citava `>` na frase. */
{ "min cita '<', que e o que compara",
  "min([1, \"a\"])\n", "", "TypeError: '<' not supported between instances of 'str' and 'int'", 1 },
{ "min com varios argumentos tambem cita '<'",
  "min(1, \"a\")\n", "", "TypeError: '<' not supported between instances of 'str' and 'int'", 1 },
{ "max segue citando '>'",
  "max([1, \"a\"])\n", "", "TypeError: '>' not supported between instances of 'str' and 'int'", 1 },

/* ── L4b DO PLANO DAS CONTRADICOES: numerico e chamadas (2026-09-12) ──────
 * Strings medidas no binario. */
/* hex/bin/oct recusavam inteiro grande com "'int' object cannot be
 * interpreted as an integer" — 'int' e o tipo que eles aceitam. */
{ "hex aceita inteiro grande",
  "post(hex(10 ** 30))\n", "0xc9f2c9cd04674edea40000000", NULL, 0 },
{ "bin com inteiro grande negativo poe o sinal antes do prefixo",
  "post(bin(-(2 ** 70)))\n", "-0b10000000000000000000000000000000000000000000000000000000000000000000000", NULL, 0 },
{ "oct aceita inteiro grande",
  "post(oct(10 ** 30))\n", "0o1447626234640431647336510000000000", NULL, 0 },
{ "hex segue recusando str",
  "hex(\"a\")\n", "", "TypeError: 'str' object cannot be interpreted as an integer", 2 },
/* ~ era o unico bit-a-bit que recusava bignum, e dizia "bad operand type
 * ... 'int'". */
{ "~ aceita inteiro grande como | ^ & << >>",
  "long g = 2 ** 100\npost(~g)\npost(~(-(2 ** 100)))\n",
  "-1267650600228229401496703205377\n1267650600228229401496703205375", NULL, 0 },
{ "~ segue recusando bool",
  "post(~True)\n", "", "TypeError: bad operand type for unary ~: 'bool'", 2 },
/* round conferia as casas duas vezes com respostas diferentes: bool passava
 * com n int e estourava com n flo. */
{ "round(flo, bool) usa bool como 0/1 de casas",
  "post(round(1.5, True), round(1.55, False), round(2.675, 2))\n", "1.5 2.0 2.67", NULL, 0 },
{ "round segue recusando casas que nao sao inteiro",
  "round(1.5, \"a\")\n", "", "TypeError: 'str' object cannot be interpreted as an integer", 1 },
/* A fatia so conferia o passo: `s[1.5:]` tratava o limite como ausente e
 * devolvia a colecao inteira, calada. */
{ "fatia: inicio que nao e inteiro e TypeError, nao 'ausente'",
  "s = \"abcdef\"\ns[1.5:]\n", "", "TypeError: slice indices must be integers or Null", 1 },
{ "fatia: fim que nao e inteiro e TypeError",
  "s = \"abcdef\"\ns[:\"x\"]\n", "", "TypeError: slice indices must be integers or Null", 1 },
{ "fatia: bool, bignum saturado e Null seguem valendo",
  "s = \"abcdef\"\npost(s[1:], s[:2], s[::2], s[True:], s[999999999999999999999:], s[:999999999999999999999])\nl = [1,2,3,4]\npost(l[1:3], l[::-1])\n",
  "bcdef ab ace bcdef  abcdef\n[2, 3] [4, 3, 2, 1]", NULL, 0 },
/* Argumentos demais com default: metodo e funct escondiam o minimo; so a
 * instanciacao e OP_CALL_KW diziam "from N to M". */
{ "metodo com default: argumentos demais diz o intervalo",
  "Entity C {\n    funct soma(self, a, b = 2) {\n        return a + b\n    }\n}\nc = C()\nc.soma(1, 2, 3)\n",
  "", "TypeError: soma() takes from 2 to 3 positional arguments but 4 were given", 2 },
{ "@static com default: argumentos demais diz o intervalo sem contar self",
  "Entity C {\n    @static\n    funct soma(a, b = 2) {\n        return a + b\n    }\n}\nC.soma(1, 2, 3)\n",
  "", "TypeError: soma() takes from 1 to 2 positional arguments but 3 were given", 2 },
{ "funct sem default segue com a frase simples",
  "funct f(a) {\n    return a\n}\npost(f(1, 2))\n",
  "", "TypeError: f() takes 1 positional argument but 2 were given", 2 },
/* map/filter que crescem a origem: erro de iteracao, nao MemoryError. */
{ "map que cresce a origem e RuntimeError",
  "l = [1, 2, 3]\nfunct cresce(x) { l.append(x)\n    return x }\ntry { post(map(l, cresce)) } catch (RuntimeError e) { post(\"pegou:\", e) }\n",
  "pegou: map() cresceu durante a iteracao (linha 4)", NULL, 0 },
{ "filter que cresce a origem nao e MemoryError",
  "l = [1, 2, 3]\nfunct cresce(x) { l.append(x)\n    return True }\ntry { post(filter(l, cresce)) } catch (MemoryError e) { post(\"NAO devia pegar\") }\n",
  "", "RuntimeError: filter() cresceu durante a iteracao", 1 },
/* base() num pai sem __init__ nem campos: o caminho posicional engolia os
 * argumentos calado e o nomeado levantava. Agora os dois levantam. */
{ "base() posicional em pai sem __init__ e TypeError",
  "Entity A {\n}\nEntity B(A) {\n    funct __init__(self, nome) {\n        base().__init__(nome)\n    }\n}\nB(\"som\")\n",
  "", "TypeError: base(): a Entity pai 'A' nao tem __init__", 2 },
{ "base() nomeado em pai sem __init__ da a MESMA frase",
  "Entity A {\n}\nEntity B(A) {\n    funct __init__(self, nome) {\n        base().__init__(nome=nome)\n    }\n}\nB(\"som\")\n",
  "", "TypeError: base(): a Entity pai 'A' nao tem __init__", 2 },
{ "base() em pai com construtor de campos segue funcionando",
  "Entity A {\n    nome: str\n}\nEntity B(A) {\n    funct __init__(self, nome, y) {\n        base().__init__(nome)\n        self.y = y\n    }\n}\nb = B(\"ana\", 2)\npost(b.nome, b.y)\n",
  "ana 2", NULL, 0 },
/* Cabeçalho sem `()` (2026-09-25): o parêntese fica só na instanciação;
 * herança continua entre parênteses. O `()` vazio acusa a forma certa. */
{ "class sem parenteses: Entity, class, Class, private e decorada",
  "funct marca(c) { return c }\nEntity A {\n    x: int\n}\nclass B {\n    funct __init__(self) { self.y = 2 }\n}\nprivate Class C {\n    funct v(self) { return 3 }\n}\n@marca\nclass D {\n    funct w(self) { return 4 }\n}\npost(A(1).x, B().y, C().v(), D().w())\n",
  "1 2 3 4", NULL, 0 },
{ "class X() com () vazio e erro que diz a forma certa",
  "class Ponto() {\n    x: int\n}\n",
  "", "SyntaxError: o '()' fica so na instanciacao: escreva `class Ponto {` (heranca: `class Ponto(Pai) {`)", 2 },
{ "Entity X() com () vazio: a frase usa a palavra escrita",
  "Entity Ponto() {\n    x: int\n}\n",
  "", "escreva `Entity Ponto {` (heranca: `Entity Ponto(Pai) {`)", 2 },
{ "class X( sem fechar acusa o parentese aberto",
  "class X( {\n}\n",
  "", "parentese '(' aberto nao foi fechado", 2 },
{ "class X(1) acusa que falta o nome do pai",
  "class X(1) {\n}\n",
  "", "esperado o nome do pai depois de '(' (heranca: `class X(Pai) {`)", 2 },
{ "heranca continua entre parenteses e a chave pode ir na linha de baixo",
  "Entity A {\n    funct v(self) { return 1 }\n}\nEntity B(A)\n{\n    funct w(self) { return 2 }\n}\nb = B()\npost(b.v(), b.w())\n",
  "1 2", NULL, 0 },
/* `base()` / `base(Pai)` como o super: alcança tudo que o pai tem. */
{ "base(Pai).__init__ com dois pais mira cada um",
  "Entity Motor {\n    funct __init__(self, cavalos) { self.cavalos = cavalos }\n}\nEntity Roda {\n    funct __init__(self, qtd) { self.qtd = qtd }\n}\nEntity Carro(Motor, Roda) {\n    funct __init__(self) {\n        base(Motor).__init__(300)\n        base(Roda).__init__(4)\n    }\n}\nc = Carro()\npost(c.cavalos, c.qtd)\n",
  "300 4", NULL, 0 },
{ "base() sem nome com dois pais e erro que lista os pais",
  "Entity A {\n}\nEntity M {\n}\nEntity B(A, M) {\n    funct __init__(self) { base().__init__() }\n}\n",
  "", "SyntaxError: base() com mais de um pai: diga qual, base(A) ou base(M)", 2 },
{ "base(X) com X que nao e ancestral e erro",
  "Entity A {\n}\nEntity Z {\n}\nEntity B(A) {\n    funct __init__(self) { base(Z).__init__() }\n}\n",
  "", "SyntaxError: 'Z' nao e pai de 'B' (pais: A)", 2 },
{ "base(Avo).__init__ mira o avo pulando o pai",
  "Entity A {\n    funct __init__(self, x) { self.x = x }\n}\nEntity B(A) {\n    funct __init__(self, x) { base().__init__(x * 10) }\n}\nEntity K(B) {\n    funct __init__(self) { base(A).__init__(9) }\n}\npost(K().x, B(1).x)\n",
  "9 10", NULL, 0 },
{ "base().metodo() chama a versao do pai mesmo sobrescrita",
  "Entity A {\n    str funct nome(self) { return \"A\" }\n    funct fala(self, s) { return \"A diz \" + s }\n}\nEntity B(A) {\n    str funct nome(self) { return \"B>\" + base().nome() }\n    funct fala(self, s) { return base(A).fala(s) + \"!\" }\n}\nb = B()\npost(b.nome(), b.fala(\"oi\"))\n",
  "B>A A diz oi!", NULL, 0 },
{ "base().campo le static do pai e campo da instancia",
  "Entity A {\n    static int total = 7\n    funct __init__(self, x) { self.x = x }\n}\nEntity B(A) {\n    funct __init__(self, x) { base().__init__(x) }\n    funct soma(self) { return base().x + base(A).total }\n}\npost(B(1).soma())\n",
  "8", NULL, 0 },
{ "base().campo private do pai continua negado pro filho",
  "Entity A {\n    private int s\n    funct __init__(self) { self.s = 1 }\n}\nEntity B(A) {\n    funct __init__(self) { base().__init__() }\n    funct q(self) { return base().s }\n}\npost(B().q())\n",
  "", "RuntimeError: acesso negado: 's' e private de A (so acessivel de dentro da classe)", 2 },
{ "base(Pai).x = v e erro: atribua pelo self",
  "Entity A {\n    funct __init__(self, x) { self.x = x }\n}\nEntity B(A) {\n    funct __init__(self) { base(A).x = 1 }\n}\n",
  "", "SyntaxError: atribua pelo self: self.x = ... (base(Pai).x le o campo, nao grava)", 2 },
{ "base(x) na forma antiga acusa a nova",
  "Entity A {\n    funct __init__(self, x) { self.x = x }\n}\nEntity B(A) {\n    funct __init__(self, x) { base(x) }\n}\n",
  "", "SyntaxError: base(Pai) precisa de um membro: base(Pai).__init__(...) ou base(Pai).metodo(...)", 2 },
{ "base(1) acusa que so aceita o nome do pai",
  "Entity A {\n}\nEntity B(A) {\n    funct __init__(self) { base(1).__init__() }\n}\n",
  "", "SyntaxError: base(...) recebe so o nome do pai: base().__init__(x) ou base(Pai).__init__(x)", 2 },
{ "base().__init__ com nomeado, espalhado e **kw",
  "Entity A {\n    funct __init__(self, x, y = 2) { self.x = x\n        self.y = y }\n}\nEntity B(A) {\n    funct __init__(self, *a, **k) { base().__init__(*a, **k) }\n}\nEntity C(A) {\n    funct __init__(self) { base().__init__(y=5, x=1) }\n}\nb = B(3, y=4)\nc = C()\npost(b.x, b.y, c.x, c.y)\n",
  "3 4 1 5", NULL, 0 },
{ "base().metodo() em metodo do pai trocado por decorador",
  "funct dobra(f) {\n    funct w(*a, **k) { return f(*a, **k) * 2 }\n    return w\n}\nEntity A {\n    @dobra\n    funct v(self) { return 5 }\n}\nEntity B(A) {\n    funct v(self) { return base().v() + 1 }\n}\npost(B().v())\n",
  "11", NULL, 0 },
{ "base().metodo() tipado encadeia com o checador",
  "Entity A {\n    str funct nome(self) { return \"a\" }\n}\nEntity B(A) {\n    funct __init__(self) { self.k = base().nome().upper() }\n}\npost(B().k)\n",
  "A", NULL, 0 },
{ "base().__init__ com aridade errada e erro antes de rodar, a frase de A(1, 2)",
  "Entity A {\n    funct __init__(self, x) { self.x = x }\n}\nEntity B(A) {\n    funct __init__(self) { base().__init__(1, 2) }\n}\nB()\n",
  "", "TypeError: __init__() takes 2 positional arguments but 3 were given", 2 },
{ "base().nada acusa o membro que o pai nao tem antes de rodar",
  "Entity A {\n    funct __init__(self, x) { self.x = x }\n}\nEntity B(A) {\n    funct __init__(self) { base().__init__(1) }\n    funct q(self) { return base().nada }\n}\nB().q()\n",
  "", "AttributeError: 'A' object has no attribute 'nada'", 2 },
{ "base() fora de Entity acusa",
  "funct f() { base().__init__() }\n",
  "", "SyntaxError: base() fora de Entity com heranca", 2 },
{ "base() numa Entity sem heranca acusa",
  "Entity A {\n    funct __init__(self) { base().__init__() }\n}\n",
  "", "SyntaxError: base() numa Entity sem heranca: nao ha pai pra inicializar", 2 },
{ "base().__init__ em qualquer metodo com self, nao so no __init__",
  "Entity A {\n    funct __init__(self, x) { self.x = x }\n}\nEntity B(A) {\n    funct __init__(self) { self.x = 0 }\n    funct reinicia(self, x) { base().__init__(x) }\n}\nb = B()\nb.reinicia(7)\npost(b.x)\n",
  "7", NULL, 0 },
/* `static funct f(self)` fora de Entity (ele, 2026-09-26): a frase é na
 * declaração e diz a causa — antes cada chamada dizia "missing 1 required
 * positional argument: 'self'". Dentro de Entity o self num static continua
 * o da doc (dropado na chamada pela classe). */
{ "static funct com self fora de Entity acusa na declaracao",
  "static funct IsRegister(self) {\n    return 1\n}\npost(IsRegister())\n",
  "", "TypeError: funct static IsRegister(self, ...) nao recebe self: static nao tem instancia, tire o self", 2 },
{ "static funct sem self fora de Entity roda",
  "static funct g(a) {\n    return a * 2\n}\npost(g(2))\n",
  "4", NULL, 0 },
{ "static com self DENTRO de Entity continua: chamada pela classe dropa o self",
  "Entity C {\n    static funct f(self, a, b=10) {\n        return a + b\n    }\n}\npost(C.f(5), C().f(5))\n",
  "15 15", NULL, 0 },
/* `s[i]` e `s[i:j]` contavam a string inteira do zero a cada acesso: 200 mil
 * índices numa str de 200 mil caracteres com acento não terminavam. A
 * contagem e a última posição ficam guardadas na própria string. */
{ "indexar str grande com acento em laco termina (contagem guardada)",
  "x = \"é\" * 200000\ni = 0\nn = 0\nwhile i < 200000 {\n    if x[i] == \"é\" { n = n + 1 }\n    i = i + 1\n}\npost(n)\n",
  "200000", NULL, 0 },
{ "fatiar str grande com acento em laco termina, inclusive andando pra tras",
  "t = \"ção\" * 50000\ni = 0\nn = 0\nwhile i < 100000 {\n    if t[i:i + 3] == \"ção\" { n = n + 1 }\n    i = i + 1\n}\ni = 149999\ns = \"\"\nwhile i > 149990 {\n    s = s + t[i]\n    i = i - 1\n}\npost(n, s, t[-3:], t[1:4], len(t))\n",
  "33334 oãçoãçoãç ção ãoç 150000", NULL, 0 },

/* ── EXCECOES COMO VALORES (2026-09-12) ──────────────────────────────────────
 * Os nomes da tabela EXCECOES[] (Exception, ValueError, ...) sao globais: um
 * V_TIPO com indice deslocado, ligado na partida como o PoolFile. Antes so
 * existiam dentro de `raise Nome(...)` e `catch (Nome e)`; `post(Exception)`
 * era NameError. Strings medidas no binario. */
{ "excecao e valor: imprime o proprio nome",
  "post(ValueError, Exception, FileNotFoundError)\n", "ValueError Exception FileNotFoundError", NULL, 0 },
{ "is entre excecoes segue a arvore",
  "post(ValueError is Exception, ValueError is ValueError, ValueError is TypeError, FileNotFoundError is OSError, OSError is FileNotFoundError)\n",
  "True True False True False", NULL, 0 },
{ "excecao is type; valor comum nao e excecao",
  "post(ValueError is type, ValueError is str, 1 is ValueError, \"x\" is ValueError)\n",
  "True False False False", NULL, 0 },
{ "== por identidade e pelo nome, como os outros tipos; type() e type",
  "post(ValueError == ValueError, ValueError == Exception, ValueError == \"ValueError\", type(ValueError))\n",
  "True False True type", NULL, 0 },
{ "raise de variavel que guarda excecao: o catch da familia pega",
  "erro = FileNotFoundError\ntry { raise erro } catch (OSError e) { post(\"pegou\", \"[\" + e + \"]\") }\n",
  "pegou [ (linha 2)]", NULL, 0 },
/* A expectativa e a primeira linha INTEIRA do stderr, com a quebra:
 * `KeyError: ` com o espaco no fim, porque a mensagem e vazia. So
 * "KeyError:" era prefixo dela, e sem a quebra ela era prefixo de
 * `KeyError: 'type'` (o `.type()` num dict, 18.0.0): o portao das assercoes
 * recusa prefixo. */
{ "raise de variavel sem catch: o tipo sai no traceback",
  "erro = KeyError\nraise erro\n", "", "KeyError: \n", 1 },
{ "raise Nome maiusculo continua tipo LITERAL, mesmo sendo variavel",
  "Boom = ValueError\ntry { raise Boom } catch (ValueError e) { post(\"NAO DEVIA\") }\n", "", "Boom:  (linha 2)", 1 },
{ "chamar excecao fora do raise e TypeError que diz onde ela vale",
  "x = ValueError(\"boom\")\n", "", "TypeError: ValueError(\"msg\") só vale depois de raise", 1 },
{ "map com excecao como funcao: a mesma frase",
  "map([1], ValueError)\n", "", "TypeError: ValueError(\"msg\") só vale depois de raise", 1 },
{ "chamada por nome numa excecao assina o nome, nao '?'",
  "f = ValueError\nf(a=1)\n", "", "TypeError: ValueError() takes no keyword arguments", 1 },
{ "excecao devolvida de funct, como chave de dict e item de lista",
  "funct f() { return ValueError }\npost(f() is Exception, {KeyError: 1}, [TypeError])\n",
  "True {KeyError: 1} [TypeError]", NULL, 0 },
{ "o e capturado continua str; e is ValueError e False",
  "try { raise ValueError(\"x\") } catch (e) { post(type(e), e is ValueError) }\n", "str False", NULL, 0 },
{ "nome fora da tabela continua NameError",
  "post(MeuErro)\n",
  "", "NameError: name 'MeuErro' is not defined", 2 },
{ "excecao pode ser sombreada como qualquer global",
  "ValueError = 3\npost(ValueError)\n", "3", NULL, 0 },
{ "excecao em f-string e str()",
  "post(f\"tipo: {ValueError}\", str(OSError) + \"!\")\n", "tipo: ValueError OSError!", NULL, 0 },

/* ── DECORADOR DEFINIDO EM .pr (2026-09-12) ───────────────────────────────────
 * Um protocolo so, no OP_DECORA: valor com `.register` registra; chamavel
 * envolve (Null mantem a funct); o resto e TypeError. Antes o compilador
 * chamava `.register` a mao (a funct devolvida por `route()` dava
 * AttributeError) e DESCARTAVA calado todo decorador de um nome que nao
 * fosse static/NonNull/dataentity — junto com a funct embaixo. Strings
 * medidas no binario. */
#define DEC_NET "Entity NET {\n    funct __init__(self) {\n        self.rotas = {}\n    }\n" \
                "    funct route(self, caminho) {\n        funct registra(f) {\n" \
                "            self.rotas[caminho] = f\n            return f\n        }\n" \
                "        return registra\n    }\n}\n"
{ "decorador de lib devolvendo funct registradora (o caso do dono)",
  DEC_NET "app = NET()\n@app.route(\"/x\")\nfunct h() {\n    return \"ok\"\n}\npost(h(), app.rotas[\"/x\"](), len(app.rotas))\n",
  "ok ok 1", NULL, 0 },
{ "@log envolve: o nome passa a valer o wrapper",
  "funct log(f) {\n    funct w() {\n        post(\"antes\")\n        return f()\n    }\n    return w\n}\n@log\nfunct h() {\n    return 1\n}\npost(h())\n",
  "antes\n1", NULL, 0 },
{ "@log() e fabrica: com parenteses o decorador e o que a chamada devolve",
  "funct log() {\n    funct dec(f) {\n        post(\"decorou\")\n        return f\n    }\n    return dec\n}\n@log()\nfunct h() {\n    return 2\n}\npost(h())\n",
  "decorou\n2", NULL, 0 },
{ "decorador que devolve Null mantem a funct",
  "funct reg(f) {\n    post(\"reg\")\n}\n@reg\nfunct h() {\n    return 3\n}\npost(h())\n",
  "reg\n3", NULL, 0 },
{ "decorador que devolve str: o nome vira a str",
  "funct ruim(f) {\n    return \"x\"\n}\n@ruim\nfunct h() {\n    return 1\n}\npost(h)\n",
  "x", NULL, 0 },
{ "@obj.m() que devolve str: nem registra nem envolve",
  "Entity R {\n    funct route(self, c) {\n        return \"oi\"\n    }\n}\napp = R()\n@app.route(\"/x\")\nfunct h() {\n    return 1\n}\n",
  "", "TypeError: decorador @app.route vale 'str', que nao registra (.register) nem envolve (chamavel) a funct", 1 },
{ "@nao_existe e NameError na linha do @, nao funct engolida",
  "@nao_existe\nfunct h() {\n    return 1\n}\npost(h())\n",
  "", "NameError: name 'nao_existe' is not defined", 2 },
{ "@x com x inteiro e TypeError",
  "x = 5\n@x\nfunct h() {\n    return 1\n}\n",
  "", "TypeError: decorador @x vale 'int', que nao registra (.register) nem envolve (chamavel) a funct", 1 },
{ "metodo static na Entity com decorador registrador da lib",
  DEC_NET "Entity App {\n    static object app = NET()\n    @app.route(\"/m\")\n    static funct m() {\n        return \"m\"\n    }\n}\npost(App.app.rotas[\"/m\"]())\n",
  "m", NULL, 0 },
/* ── decorador que ENVOLVE um metodo: o que ele devolve vale como o metodo ──
 * A tabela de metodos da classe guarda o valor; `inst.m` liga o receptor e o
 * chamado recebe o receptor no 1o argumento. Antes era TypeError ("de metodo
 * so pode registrar") — limitacao sem motivo, tirada. */
{ "decorador em metodo: o wrapper vale como o metodo e recebe o self no 1o argumento",
  "funct log(f) {\n    funct w(*args, **kwarg) {\n        post(\"antes\", len(args))\n        return f(*args, **kwarg)\n    }\n    return w\n}\nEntity C {\n    funct __init__(self) {\n        self.v = 7\n    }\n    @log\n    funct m(self, x) {\n        return self.v + x\n    }\n}\nc = C()\npost(c.m(1))\npost(c.m(x=2))\npost(c.m(*[3]))\n",
  "antes 2\n8\nantes 1\n9\nantes 2\n10", NULL, 0 },
{ "decorador em metodo: empilhados compoem de baixo pra cima, como na funct solta",
  "funct a(f) {\n    funct w(*args) {\n        return \"a(\" + f(*args) + \")\"\n    }\n    return w\n}\nfunct b(f) {\n    funct w(*args) {\n        return \"b(\" + f(*args) + \")\"\n    }\n    return w\n}\nEntity C {\n    @a\n    @b\n    funct m(self) {\n        return \"m\"\n    }\n}\npost(C().m())\n",
  "a(b(m))", NULL, 0 },
{ "decorador em metodo static: vale pela classe, e pela instancia segue o erro de static",
  "funct dobra(f) {\n    funct w(*args) {\n        return f(*args) * 2\n    }\n    return w\n}\nEntity C {\n    @dobra\n    static funct s(x) {\n        return x + 1\n    }\n}\npost(C.s(4))\nC().s(4)\n",
  "10", "RuntimeError: funct 's' e static: chame pela Entity (Tipo.s(...)), nao pela instancia", 1 },
{ "decorador em __init__: vale na instanciacao posicional, nomeada e no base()",
  "funct conta(f) {\n    funct w(*args, **kwarg) {\n        post(\"init\", len(args), kwarg)\n        return f(*args, **kwarg)\n    }\n    return w\n}\nEntity A {\n    @conta\n    funct __init__(self, v = 0) {\n        self.v = v\n    }\n}\nEntity B(A) {\n    funct __init__(self) {\n        base().__init__(v=9)\n    }\n}\nEntity D(A) {\n    funct __init__(self) {\n        base().__init__(8)\n    }\n}\npost(A(3).v, A(v=4).v, B().v, D().v)\n",
  "init 2 {}\ninit 1 {'v': 4}\ninit 1 {'v': 9}\ninit 2 {}\n3 4 9 8", NULL, 0 },
{ "decorador em metodo: o metodo envolvido passa por callback do C (map)",
  "funct dobra(f) {\n    funct w(*args) {\n        return f(*args) * 2\n    }\n    return w\n}\nEntity E {\n    funct __init__(self) {\n        self.k = 10\n    }\n    @dobra\n    funct soma(self, x) {\n        return self.k + x\n    }\n}\npost(map([1, 2], E().soma))\n",
  "[22, 24]", NULL, 0 },
{ "decorador em metodo: a filha herda o metodo envolvido, e o private segue de dentro",
  "funct passa(f) {\n    funct w(*args) {\n        return f(*args)\n    }\n    return w\n}\nEntity P {\n    private segredo = 5\n    funct __init__(self) {\n        self.segredo = 5\n    }\n    @passa\n    funct ver(self) {\n        return self.segredo\n    }\n}\nEntity F(P) {\n}\npost(F().ver())\n",
  "5", NULL, 0 },
{ "decorador em metodo: o wrapper e de fora da classe e nao le private",
  "funct espia(f) {\n    funct w(self) {\n        return self.segredo\n    }\n    return w\n}\nEntity Q {\n    private segredo = 1\n    funct __init__(self) {\n        self.segredo = 1\n    }\n    @espia\n    funct ver(self) {\n        return 0\n    }\n}\nQ().ver()\n",
  "", "RuntimeError: acesso negado: 'segredo' e private de Q (so acessivel de dentro da classe)", 1 },
{ "decorador em metodo: registrador e wrapper nas duas ordens, uma instancia por classe",
  "Entity Reg {\n    funct __init__(self) {\n        self.itens = []\n    }\n    funct register(self, f) {\n        self.itens.append(f)\n    }\n}\nr = Reg()\nfunct lock(f) {\n    funct w(*args) {\n        post(\"lock\")\n        return f(*args)\n    }\n    return w\n}\nEntity H {\n    funct __init__(self) {\n        post(\"init H\")\n    }\n    @r\n    @lock\n    funct um(self) {\n        return \"um\"\n    }\n    @lock\n    @r\n    funct dois(self) {\n        return \"dois\"\n    }\n    @r\n    funct tres(self) {\n        return self\n    }\n}\npost(r.itens[0]())\npost(r.itens[1]())\npost(r.itens[2]() == r.itens[2]())\n",
  "init H\nlock\num\ndois\nTrue", NULL, 0 },
{ "decorador em metodo gerador e em metodo async",
  "funct passa(f) {\n    funct w(*args, **kwarg) {\n        return f(*args, **kwarg)\n    }\n    return w\n}\nEntity G {\n    @passa\n    funct gera(self, n) {\n        for each i in range(n) {\n            yield i\n        }\n    }\n    @passa\n    async funct pega(self, x) {\n        return x * 3\n    }\n}\ng = G()\npost(list(g.gera(3)))\npost(await g.pega(5))\n",
  "[0, 1, 2]\n15", NULL, 0 },
/* `chama_valor` (callback do C) deslocava o `self` pelo NOME do 1o parametro
 * sem olhar `static`: `map([7], f)` com `funct f(self, x)` ligava `[Null, 7]`
 * calado, e a fibra contava o buraco. Agora so desloca em `static`, como o
 * OP_CALL — e e o que deixa o metodo envolvido receber o receptor. */
{ "callback do C nao desloca self pelo nome numa funct comum (map erra como f(x))",
  "funct f(self, x) {\n    return [self, x]\n}\npost(map([7], f))\n",
  "", "TypeError: f() missing 1 required positional argument: 'x'", 1 },
{ "async funct com parametro chamado self conta os argumentos como a sincrona",
  "async funct f(self, x) {\n    return [self, x]\n}\npost(await f(1, 2))\n",
  "[1, 2]", NULL, 0 },
{ "decorador em metodo que devolve Null mantem o metodo",
  "funct nada(f) {\n    return Null\n}\nEntity C {\n    @nada\n    funct m(self) {\n        return \"original\"\n    }\n}\npost(C().m())\n",
  "original", NULL, 0 },
{ "@reg sobre Entity: o decorador recebe a funct do 1o metodo",
  "funct reg(f) {\n    post(\"recebeu\", type(f))\n}\n@reg\nEntity H {\n    funct handler(self) {\n        return 1\n    }\n}\npost(H().handler())\n",
  "recebeu funct\n1", NULL, 0 },
{ "@log sobre Entity envolve o 1o metodo",
  "funct log(f) {\n    funct w(*args) {\n        return \"log:\" + f(*args)\n    }\n    return w\n}\n@log\nEntity H {\n    funct handler(self) {\n        return \"h\"\n    }\n}\npost(H().handler())\n",
  "log:h", NULL, 0 },
{ "decoradores empilhados: o de dentro aplica primeiro",
  "funct a(f) {\n    funct wa() {\n        return \"a(\" + f() + \")\"\n    }\n    return wa\n}\nfunct b(f) {\n    funct wb() {\n        return \"b(\" + f() + \")\"\n    }\n    return wb\n}\n@a\n@b\nfunct h() {\n    return \"h\"\n}\npost(h())\n",
  "a(b(h))", NULL, 0 },
{ "@x sem funct ou Entity embaixo e SyntaxError, nao ignorado",
  "@x\ny = 1\n", "", "SyntaxError: decorador sem funct ou Entity embaixo", 2 },
{ "decorador sobre Entity sem metodo e SyntaxError, nao silencio",
  "funct reg(f) {\n    return f\n}\n@reg\nEntity Vazia {\n    x: int\n}\npost(\"fim\")\n",
  "", "SyntaxError: decorador em cima de Entity 'Vazia' sem metodo: nao ha o que registrar", 2 },
/* `app.middleware()` COM parenteses e o registrador do decorador: guardado em
 * `middleware=`, a rota rodava sem middleware nenhum, calada. */
{ "jinker: middleware=app.middleware() com parenteses e recusado no registro da rota",
  "import jinker\napp = jinker.Jinker(name=\"p\")\n@app.middleware()\nfunct mw() {\n    pass\n}\n@app.get(\"/x\", middleware=app.middleware())\nfunct x() {\n    return 1\n}\n",
  "", "TypeError: get(middleware=) recebeu o registrador de app.middleware(): escreva middleware=app.middleware, sem parenteses", 1 },
{ "jinker: @app.route segue registrando e o nome fica definido",
  "import jinker\napp = jinker.Jinker()\n@app.route(\"/x\")\nfunct h() {\n    return \"ok\"\n}\npost(h())\n",
  "ok", NULL, 0 },
#undef DEC_NET

/* ── LITERAL b"..." E O TIPO byte (2026-09-12) ────────────────────────────────
 * `b"..."` era o nome `b` seguido de string (NameError). Agora e um literal
 * de bytes CRUS: \xHH e UM byte, NUL dentro vale, nao-ASCII entra com os
 * bytes UTF-8 do fonte, `\u` e erro. O tipo chama `byte` (o modulo continua
 * `bytes`): `byte x = ...` confere, `x is byte`, `type()` responde `byte`.
 * Strings medidas no binario. */
{ "byte: literal simples e type()",
  "x = b\"abc\"\npost(x, len(x), type(x))\n", "b'abc' 3 byte", NULL, 0 },
{ "byte: NUL no meio conta e imprime",
  "x = b\"a\\x00b\"\npost(len(x), x)\n", "3 b'a\\x00b'", NULL, 0 },
{ "byte: \\xff e UM byte de valor 255",
  "x = b\"\\xff\"\npost(len(x), x[0], x)\n", "1 255 b'\\xff'", NULL, 0 },
{ "byte: octal e \\0",
  "x = b\"\\101\\0\"\npost(x, len(x))\n", "b'A\\x00' 2", NULL, 0 },
{ "byte: nao-ASCII no fonte vira os bytes UTF-8 dele",
  "x = b\"\xc3\xa9\"\npost(len(x), x)\n", "2 b'\\xc3\\xa9'", NULL, 0 },
{ "byte: \\u nao vale em bytes",
  "x = b\"\\u00e9\"\n", "", "SyntaxError: \\u nao vale em bytes: use \\xHH", 2 },
{ "byte: octal acima de 255 e erro",
  "x = b\"\\777\"\n", "", "SyntaxError: octal fora de 0-255 em bytes", 2 },
{ "byte: literal == encode(), e nao == str",
  "post(b\"oi\" == \"oi\".encode(), b\"oi\" == \"oi\")\n", "True False", NULL, 0 },
{ "byte: declaracao tipada aceita bytes",
  "byte x = b\"a\"\npost(x)\n", "b'a'", NULL, 0 },
{ "byte: declaracao tipada recusa str (antes passava calado)",
  "byte y = \"texto\"\n",
  "", "AttributedValueError: variável y esperava byte, recebeu str", 2 },
{ "byte: is byte, sem import",
  "post(b\"a\" is byte, \"a\" is byte, byte is type, type(byte))\n", "True False True type", NULL, 0 },
{ "byte(x) e o mesmo que bytes.new(x)",
  "import bytes\npost(byte(\"a\"), byte(\"a\") == bytes.new(\"a\"), byte(b\"x\"))\n", "b'a' True b'x'", NULL, 0 },
{ "byte: match casa bytes e nao str",
  "match b\"x\" {\n    case \"x\" { post(\"str\") }\n    case b\"x\" { post(\"bytes\") }\n}\n", "bytes", NULL, 0 },
{ "byte: b e B continuam nomes quando nao vem aspa colada",
  "b = 1\npost(b, b\"x\", b, B\"y\")\n", "1 b'x' 1 b'y'", NULL, 0 },
{ "byte: br/rb e cru (nao processa escape)",
  "post(len(br\"\\n\"), rb\"\\n\", len(b\"\\n\"))\n", "2 b'\\\\n' 1", NULL, 0 },
{ "byte: literal triplo guarda a quebra de linha como byte",
  "x = b'''linha1\nlinha2'''\npost(len(x), x)\n", "13 b'linha1\\nlinha2'", NULL, 0 },
{ "byte: vazio",
  "post(b\"\", len(b\"\"))\n", "b'' 0", NULL, 0 },
{ "byte: assinatura PNG via fromhex bate com o literal",
  "import bytes\npost(bytes.fromhex(\"89504e470d0a1a0a\") == b\"\\x89PNG\\r\\n\\x1a\\n\")\n", "True", NULL, 0 },
/* `\"\"\"` pelado e comentario de bloco; com prefixo so pode ser literal. Antes
 * `b\"\"\"ab\"\"\"` virava tres tokens (`b\"\"` + `\"ab\"` + `\"\"`). */
{ "byte: literal triplo com aspas duplas e o mesmo que com simples (e a f-string tambem)",
  "post(b\"\"\"ab\"\"\", f\"\"\"x{1}y\"\"\", len(b\"\"\"ab\"\"\"), b'''ab''' == b\"\"\"ab\"\"\")\n",
  "b'ab' x1y 2 True", NULL, 0 },

/* ── variadicos: `*args` (tup) e `**kwarg` (dict), e o espalhamento `f(*l, **d)` ──
 * Um binding so (`liga_args`) pros sete caminhos de chamada da VM: funct,
 * closure, bound, Entity, base(), callback vindo do C e gerador. O que o
 * dono pediu: `route(self, caminho, **kwarg)` num decorador de lib, sem
 * obrigar quem escreve a rota a montar um dict. */
{ "variadico: *args recebe os posicionais excedentes numa tup (vazia sem excedente)",
  "funct f(a, *args) {\n    post(a, args)\n}\nf(1, 2, 3)\nf(1)\n", "1 (2, 3)\n1 ()", NULL, 0 },
{ "variadico: **kwarg recebe os nomeados sem parametro, na ordem da chamada",
  "funct f(**kwarg) {\n    post(kwarg)\n}\nf(b=2, a=1)\nf()\n", "{'b': 2, 'a': 1}\n{}", NULL, 0 },
{ "variadico: nome que casa com parametro fixo fica nele, o resto cai no kwarg",
  "funct f(a, b=2, **kwarg) {\n    post(a, b, kwarg)\n}\nf(1, c=3, b=5)\n", "1 5 {'c': 3}", NULL, 0 },
{ "variadico: default antes do *args",
  "funct f(a, b=2, *args) {\n    post(a, b, args)\n}\nf(1)\nf(1, 9, 8, 7)\n", "1 2 ()\n1 9 (8, 7)", NULL, 0 },
{ "variadico: os dois juntos, cheios e vazios",
  "funct f(a, *args, **kwarg) {\n    post(a, args, kwarg)\n}\nf(1, 2, x=3)\nf(1)\n",
  "1 (2,) {'x': 3}\n1 () {}", NULL, 0 },
{ "variadico: sem **kwarg o nome desconhecido continua erro",
  "funct f(a) {\n    return a\n}\npost(f(1, x=2))\n",
  "", "TypeError: f() got an unexpected keyword argument 'x'", 2 },
{ "variadico: tipo em estrela e recusado (a estrela ja decide tup/dict)",
  "funct f(*int args) {\n    return args\n}\n",
  "", "SyntaxError: parametro `*int args` nao aceita tipo: `*args` e sempre tup e `**kwarg` sempre dict — escreva `*args`", 2 },
{ "variadico: **kwarg tem que ser o ultimo",
  "funct f(**kwarg, a) {\n    return a\n}\n",
  "", "SyntaxError: `**kwarg` tem que ser o ultimo parametro: mova-o pro fim", 2 },
{ "variadico: parametro comum depois de *args e recusado com o conserto",
  "funct f(*args, x) {\n    return x\n}\n",
  "", "SyntaxError: parametro `x` depois de `*args` nao e permitido: mova `x` pra antes do `*args`", 2 },
{ "variadico: estrela nao tem valor padrao",
  "funct f(*args=1) {\n    return args\n}\n",
  "", "SyntaxError: `*args` nao tem valor padrao: sem argumento a tup vem vazia", 2 },
{ "espalhar: f(*lista), f(*tup) e misto com posicional",
  "funct f(a, b, c) {\n    post(a, b, c)\n}\nl = [1, 2, 3]\nf(*l)\nf(*(4, 5), 6)\nf(0, *[7, 8])\n",
  "1 2 3\n4 5 6\n0 7 8", NULL, 0 },
{ "espalhar: f(**dict), com *lista e com nomeado",
  "funct f(a, b, c=0) {\n    post(a, b, c)\n}\nd = {\"b\": 2, \"c\": 3}\nf(1, **d)\nf(*[1], **{\"b\": 9})\nf(1, b=2, **{\"c\": 5})\n",
  "1 2 3\n1 9 0\n1 2 5", NULL, 0 },
{ "espalhar: a ordem escrita e mantida e o ultimo nomeado ganha",
  "funct f(*args, **kwarg) {\n    post(args, kwarg)\n}\nf(1, *[2, 3], 4, k=1, **{\"k\": 2, \"j\": 3}, m=4)\n",
  "(1, 2, 3, 4) {'k': 2, 'j': 3, 'm': 4}", NULL, 0 },
{ "espalhar: repasse meio(*args, **kwarg) -> alvo(*args, **kwarg)",
  "funct alvo(a, b=1, *args, **kwarg) {\n    post(a, b, args, kwarg)\n}\nfunct meio(*args, **kwarg) {\n    return alvo(*args, **kwarg)\n}\nmeio(1, 2, 3, 4, z=5)\n",
  "1 2 (3, 4) {'z': 5}", NULL, 0 },
{ "espalhar: *x exige list ou tup",
  "funct f(a) {\n    return a\n}\nf(*5)\n", "", "TypeError: argument after * must be a list or tup, not int", 1 },
{ "espalhar: **d exige dict",
  "funct f(a) {\n    return a\n}\nf(**[1])\n", "", "TypeError: argument after ** must be a dict, not list", 1 },
{ "espalhar: chave que nao e str nao vira nome de argumento",
  "funct f(**kwarg) {\n    return kwarg\n}\nf(**{1: 2})\n", "", "TypeError: keywords must be strings", 1 },
{ "espalhar: *x depois de nomeado e recusado",
  "funct f(*args, **kwarg) {\n    return 1\n}\nf(a=1, *[2])\n",
  "", "SyntaxError: argumento `*x` depois de nomeado: mova-o pra antes dos nomeados", 2 },
{ "espalhar: builtin e metodo nativo aceitam *lista",
  "post(max(*[1, 5, 3]))\npost(len(*[\"abc\"]))\npost(\"a-b\".split(*[\"-\"]))\n", "5\n3\n['a', 'b']", NULL, 0 },
/* O caso do dono: decorador de lib guardando os nomeados da rota. */
{ "variadico: decorador de lib com route(self, caminho, **kwarg) e @app.route(\"/y\", **opts)",
  "Entity NET {\n    funct __init__(self) {\n        self.rotas = {}\n        self.opcoes = {}\n    }\n"
  "    funct route(self, caminho, **kwarg) {\n        funct registra(f) {\n            self.rotas[caminho] = f\n"
  "            self.opcoes[caminho] = kwarg\n            return f\n        }\n        return registra\n    }\n}\n"
  "app = NET()\n@app.route(\"/x\", auth=\"jwt\", methods=[\"GET\"])\nfunct h(**kwarg) {\n    return kwarg\n}\n"
  "post(app.opcoes[\"/x\"], app.rotas[\"/x\"](**app.opcoes[\"/x\"]))\n"
  "opts = {\"auth\": \"none\"}\n@app.route(\"/y\", **opts)\nfunct g() {\n    return \"g\"\n}\n"
  "post(app.opcoes[\"/y\"], app.rotas[\"/y\"]())\n",
  "{'auth': 'jwt', 'methods': ['GET']} {'auth': 'jwt', 'methods': ['GET']}\n{'auth': 'none'} g", NULL, 0 },
{ "variadico: lambda, metodo, @static e gerador com *args",
  "g = funct(*args) {\n    return args\n}\npost(g(1, 2))\n"
  "Entity C {\n    funct m(self, *args, **kwarg) {\n        return [args, kwarg]\n    }\n"
  "    @static\n    funct s(*args) {\n        return args\n    }\n}\npost(C().m(1, k=2), C.s(3, 4))\n"
  "funct ger(*args) {\n    for each a in args {\n        yield a * 10\n    }\n}\npost(list(ger(1, 2)))\npost(list(ger(*[3])))\n",
  "(1, 2)\n[(1,), {'k': 2}] (3, 4)\n[10, 20]\n[30]", NULL, 0 },
/* Furo vizinho, medido: a funct que um decorador devolve captura o argumento
 * do decorador (closure) e, chamada por nome, era "'funct' object is not
 * callable" — o CALL_KW so aceitava FUNC. */
{ "variadico: closure com captura chamada por nome",
  "funct r(c) {\n    funct g(f) {\n        return f + c\n    }\n    return g\n}\nh = r(\"/x\")\npost(h(f=\"a\"))\n",
  "a/x", NULL, 0 },
{ "variadico: gerador chamado por nome",
  "funct ger(a, b=1) {\n    yield a\n    yield b\n}\npost(list(ger(b=5, a=2)))\n", "[2, 5]", NULL, 0 },
{ "variadico: lambda aceita valor padrao (mesma regra da declaracao)",
  "g = funct(x=1) {\n    return x\n}\npost(g(), g(5))\n", "1 5", NULL, 0 },
{ "espalhar: base().__init__(**kwarg) e base().__init__(*args)",
  "Entity A {\n    funct __init__(self, x, y=0) {\n        self.x = x\n        self.y = y\n    }\n}\n"
  "Entity B(A) {\n    funct __init__(self, **kwarg) {\n        base().__init__(**kwarg)\n    }\n}\n"
  "Entity C(A) {\n    funct __init__(self, *args) {\n        base().__init__(*args)\n    }\n}\n"
  "b = B(x=1, y=2)\nc = C(3)\npost(b.x, b.y, c.x, c.y)\n", "1 2 3 0", NULL, 0 },
{ "variadico: instanciacao por nome com **kwarg no __init__",
  "Entity P {\n    funct __init__(self, nome, **kwarg) {\n        self.nome = nome\n        self.extra = kwarg\n    }\n}\n"
  "p = P(nome=\"k\", idade=3)\npost(p.nome, p.extra)\n", "k {'idade': 3}", NULL, 0 },
{ "variadico: async funct com *args",
  "async funct f(a, *args) {\n    return [a, args]\n}\npost(await f(1, 2, 3))\n", "[1, (2, 3)]", NULL, 0 },
{ "variadico: callback vindo do C (map) com *args",
  "funct f(x, *args) {\n    return x * 2\n}\npost(map([1, 2], f))\n", "[2, 4]", NULL, 0 },

/* ── um só caminho de chamada: gerador, async e instância em TODA entrada ───
 * OP_CALL, OP_CALL_KW, CALL_EX, base() e o callback vindo do C decidiam cada
 * um sozinho entre gerador, fibra e frame, e cada um esquecia um pedaço (os
 * pendentes da revisão de 236eba8, 2026-09-14). Cada caminho entra aqui como
 * variante própria. */
/* async: `f(1)` devolvia future e `f(a=1)` rodava o corpo inline */
{ "async por nome devolve future, como a chamada posicional",
  "async funct f(a, b=2) {\n    return [a, b]\n}\npost(type(f(1)), type(f(a=1)))\n",
  "future future", NULL, 0 },
{ "async: espalhado por f(**dict) devolve future",
  "async funct f(a) {\n    return a\n}\npost(type(f(**{\"a\": 1})), await f(**{\"a\": 2}))\n",
  "future 2", NULL, 0 },
/* a fibra nascia sem as células da closure: "upvalue fora da faixa" */
{ "variadico: async funct aninhada com captura roda no await como a sincrona",
  "funct externa(v) {\n    async funct interna(a) {\n        return [a, v]\n    }\n    return interna\n}\nf = externa(10)\npost(await f(1))\n",
  "[1, 10]", NULL, 0 },
{ "async: closure async chamada por nome mantem as celulas",
  "funct externa(v) {\n    async funct interna(a) {\n        return [a, v]\n    }\n    return interna\n}\nf = externa(10)\npost(await f(a=1))\n",
  "[1, 10]", NULL, 0 },
/* só a funct solta e a @static viravam fibra; o método rodava inline */
{ "variadico: async como metodo de instancia devolve future nos 4 caminhos de chamada",
  "Entity C {\n    async funct m(self, a) {\n        return a\n    }\n}\nc = C()\nf = c.m\n"
  "post(type(c.m(1)), type(c.m(*[1])), type(c.m(a=1)), type(f(1)))\n",
  "future future future future", NULL, 0 },
{ "async: metodo async como callback do C (map) devolve os futures",
  "Entity C {\n    async funct m(self, a) {\n        return a\n    }\n}\nc = C()\npost(await map([1, 2], c.m))\n",
  "[1, 2]", NULL, 0 },
{ "async: @static async chamada pela Entity",
  "Entity C {\n    @static\n    async funct s(self, a) {\n        return a\n    }\n}\npost(await C.s(3))\n",
  "3", NULL, 0 },
/* a fibra guardava 32 argumentos crus; passar disso era TypeError */
{ "async: *args com 40 argumentos (a fibra nao tem teto de 32)",
  "async funct f(*a) {\n    return len(a)\n}\npost(await f(*list(range(40))))\n",
  "40", NULL, 0 },
{ "async: nome errado e TypeError na chamada, antes do await",
  "async funct f(a) {\n    return a\n}\nf(b=1)\n",
  "", "TypeError: f() got an unexpected keyword argument 'b'", 2 },
{ "async: decorador async devolve um future no lugar da funct",
  "async funct d(f) {\n    return f\n}\n@d\nfunct h() {\n    return 2\n}\ng = await h\npost(g())\n",
  "2", NULL, 0 },
/* gerador: `o.conta(2)` entrava no corpo e dava "yield fora de gerador" */
{ "variadico: gerador como metodo chamado pela instancia cria o gerador nos 4 caminhos",
  "Entity G {\n    funct conta(self, n) {\n        for each i in range(n) {\n            yield i\n        }\n    }\n}\n"
  "o = G()\npost(list(o.conta(2)))\npost(list(o.conta(*[2])))\nfor each x in o.conta(2) {\n    post(x)\n}\npost(list(o.conta(n=2)))\n",
  "[0, 1]\n[0, 1]\n0\n1\n[0, 1]", NULL, 0 },
{ "gerador: metodo gerador ligado solto e como callback do C",
  "Entity G {\n    funct conta(self, n) {\n        for each i in range(n) {\n            yield i\n        }\n    }\n}\n"
  "o = G()\nf = o.conta\npost(list(f(2)))\npost(list(map([2], o.conta)[0]))\n",
  "[0, 1]\n[0, 1]", NULL, 0 },
{ "gerador: funct geradora como callback do C cria o gerador",
  "funct g(n) {\n    for each i in range(n) {\n        yield i\n    }\n}\npost(list(map([2], g)[0]))\n",
  "[0, 1]", NULL, 0 },
{ "gerador: decorador gerador devolve o gerador no lugar da funct",
  "funct d(f) {\n    yield f\n}\n@d\nfunct h() {\n    return 1\n}\npost(list(h)[0]())\n",
  "1", NULL, 0 },
/* instância: o RETURN lia o slot 0 do __init__, que é do 1º parâmetro */
{ "variadico: __init__(*args) sem self devolve a instancia, nao a tup dos argumentos",
  "Entity E {\n    funct __init__(*args) {\n        post(len(args), type(args[0]))\n    }\n}\ne = E(1, 2)\npost(type(e))\n",
  "3 E\nE", NULL, 0 },
{ "instancia: __init__(*args, **kw) sem self recebe a instancia no args",
  "Entity E {\n    funct __init__(*args, **kw) {\n        post(len(args), type(args[0]), kw)\n    }\n}\npost(type(E(1, a=2)))\n",
  "2 E {'a': 2}\nE", NULL, 0 },
{ "instancia: __init__(*args) sem self com E(*lista)",
  "Entity E {\n    funct __init__(*args) {\n        post(len(args), type(args[0]))\n    }\n}\npost(type(E(*[1, 2])))\n",
  "3 E\nE", NULL, 0 },
/* Era "reatribuir o 1o parametro ainda devolve a instancia": `__init__(x)`
 * recebia a instancia em `x` e valia. Metodo sem `self` e erro (7.3) — e o
 * erro sai na DECLARACAO, antes de rodar, dizendo que nome achou no lugar. */
{ "instancia: __init__ com 1o parametro que nao e self e erro na declaracao",
  "Entity E {\n    funct __init__(x) {\n        x = 5\n    }\n}\npost(type(E()))\n",
  "", "TypeError: método __init__(x) sem self: o primeiro parâmetro de um método é self, não 'x' (ou marque static)", 2 },
{ "instancia: __init__(**kw) sem self e erro na declaracao (a instancia entraria como posicional)",
  "Entity E {\n    funct __init__(**kw) {\n        post(kw)\n    }\n}\nE(a=1)\n",
  "", "TypeError: método __init__(**kw) sem self: o primeiro parâmetro de um método é self (ou marque static)", 2 },
/* a Entity como valor era "'Entity' object is not callable" no callback */
{ "instancia: Entity como callback do C (map) instancia",
  "Entity P {\n    funct __init__(self, x) {\n        self.x = x\n    }\n}\npost(map([1, 2], P)[1].x)\n",
  "2", NULL, 0 },
{ "instancia: Entity sem __init__ como callback com argumento e TypeError",
  "Entity Z {\n}\npost(map([1], Z))\n",
  "", "TypeError: Z() takes no arguments (1 given)", 1 },
{ "instancia: __init__ gerador e TypeError",
  "Entity E {\n    funct __init__(self) {\n        yield 1\n    }\n}\nE()\n",
  "", "TypeError: __init__() should return Null, not 'generator'", 1 },
{ "instancia: __init__ async e TypeError, tambem vindo do C",
  "Entity E {\n    async funct __init__(self, x) {\n        post(1)\n    }\n}\nmap([1], E)\n",
  "", "TypeError: __init__() should return Null, not 'future'", 1 },
{ "instancia: base() numa funct sem self e recusado na declaracao",
  "Entity A {\n    funct __init__(self, *a) {\n        self.a = a\n    }\n}\nEntity B(A) {\n    funct __init__(*args) {\n        base().__init__(1)\n    }\n}\n",
  "", "SyntaxError: base() precisa do self: declare `funct __init__(self, ...)` (o self e o objeto que o pai inicializa)", 2 },
{ "instancia: base() entrega o self ao __init__(*args) do pai sem self",
  "Entity A {\n    funct __init__(*a) {\n        post(len(a), type(a[0]))\n    }\n}\nEntity B(A) {\n    funct __init__(self) {\n        base().__init__(1, 2)\n    }\n}\nB()\n",
  "3 B", NULL, 0 },
/* @static: o callback do C e o decorador contavam o buraco do self na frase */
{ "variadico: callback do C esconde o buraco do @static na frase de aridade, como o OP_CALL",
  "Entity C {\n    @static\n    funct s(self) {\n        return 1\n    }\n}\npost(map([7], C.s))\n",
  "", "TypeError: s() takes 0 positional arguments but 1 was given", 1 },
{ "variadico: decorador esconde o buraco do @static na frase de aridade, como o OP_CALL",
  "Entity C {\n    @static\n    funct s(self) {\n        return 1\n    }\n}\n@C.s\nfunct h() {\n    return 1\n}\n",
  "", "TypeError: s() takes 0 positional arguments but 1 was given", 1 },
{ "static: funct @static ligada solta como callback esconde o buraco do self",
  "Entity C {\n    @static\n    funct s(self) {\n        return 1\n    }\n}\nf = C.s\npost(map([7], f))\n",
  "", "TypeError: s() takes 0 positional arguments but 1 was given", 1 },
/* nonnull: o buraco UNSET do self era acusado como parâmetro Null */
{ "variadico: nonnull + static com self nao acusa o buraco do self como Null",
  "Entity C {\n    nonnull static funct f(self, a) {\n        return a\n    }\n    static funct g(self, a) {\n        return a\n    }\n}\n"
  "post(C.g(5))\npost(C.f(5))\n",
  "5\n5", NULL, 0 },
{ "nonnull static: Null no parametro real continua recusado",
  "Entity C {\n    nonnull static funct f(self, a) {\n        return a\n    }\n}\nC.f(Null)\n",
  "", "RuntimeError: nonnull: parametro 'a' em 'f' nao pode ser Null", 1 },
{ "nonnull static como callback do C",
  "Entity C {\n    nonnull static funct f(self, a) {\n        return a\n    }\n}\npost(map([5], C.f))\n",
  "[5]", NULL, 0 },

/* ── atalho do `for each ... in range(...)`: toda forma de ligar `range` ─────
 * O atalho não chama `range`: conta direto. Só vale se o arquivo não liga o
 * nome em lugar nenhum, e a busca via atribuição e import por lista — o
 * `range` do usuário era trocado pelo embutido, calado. */
{ "espalhar: for each em range(*l) espalha como list(range(*l))",
  "l = [1, 4]\npost(list(range(*l)))\nfor each i in range(*l) {\n    post(i)\n}\n",
  "[1, 2, 3]\n1\n2\n3", NULL, 0 },
{ "range: import json as range desliga o atalho",
  "import json as range\nfor each i in range(2) {\n    post(i)\n}\n",
  "", "TypeError: 'module' object is not callable", 1 },
{ "range: parametro de funct chamado range",
  "funct f(range) {\n    for each i in range(2) {\n        post(i)\n    }\n}\nf(funct(n) {\n    return [\"a\"]\n})\n",
  "a", NULL, 0 },
{ "range: parametro de lambda chamado range",
  "g = funct(range) {\n    for each i in range(2) {\n        post(i)\n    }\n}\ng(funct(n) {\n    return [\"b\"]\n})\n",
  "b", NULL, 0 },
{ "range: variavel de catch chamada range",
  "try {\n    raise ValueError(\"x\")\n} catch (range) {\n    for each i in range(2) {\n        post(i)\n    }\n}\n",
  "", "TypeError: 'str' object is not callable", 1 },

/* ── limite de parâmetros: da DECLARAÇÃO, não de cada chamada ────────────────
 * Passava no `--check` e toda chamada dava "tem parametros demais" apontando
 * a linha da chamada. Os fontes com 257 nomes são gerados pelo próprio motor. */
{ "variadico: funct com 257 parametros fixos e recusada na declaracao, nao em cada chamada",
  "funct f("
  "p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, "
  "p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51, p52, p53, p54, p55, p56, p57, p58, p59, p60, "
  "p61, p62, p63, p64, p65, p66, p67, p68, p69, p70, p71, p72, p73, p74, p75, p76, p77, p78, p79, p80, p81, p82, p83, p84, p85, p86, p87, p88, p89, p90, "
  "p91, p92, p93, p94, p95, p96, p97, p98, p99, p100, p101, p102, p103, p104, p105, p106, p107, p108, p109, p110, p111, p112, p113, p114, p115, p116, p117, p118, p119, p120, "
  "p121, p122, p123, p124, p125, p126, p127, p128, p129, p130, p131, p132, p133, p134, p135, p136, p137, p138, p139, p140, p141, p142, p143, p144, p145, p146, p147, p148, p149, p150, "
  "p151, p152, p153, p154, p155, p156, p157, p158, p159, p160, p161, p162, p163, p164, p165, p166, p167, p168, p169, p170, p171, p172, p173, p174, p175, p176, p177, p178, p179, p180, "
  "p181, p182, p183, p184, p185, p186, p187, p188, p189, p190, p191, p192, p193, p194, p195, p196, p197, p198, p199, p200, p201, p202, p203, p204, p205, p206, p207, p208, p209, p210, "
  "p211, p212, p213, p214, p215, p216, p217, p218, p219, p220, p221, p222, p223, p224, p225, p226, p227, p228, p229, p230, p231, p232, p233, p234, p235, p236, p237, p238, p239, p240, "
  "p241, p242, p243, p244, p245, p246, p247, p248, p249, p250, p251, p252, p253, p254, p255, p256, p257"
  ") {\n    return p1\n}\npost(f(*list(range(257))))\n",
  "", "SyntaxError: f() tem parametros demais (maximo 256)", 2 },
{ "parametros: lambda com 257 parametros e recusada na declaracao",
  "import os\nimport sys\nps = []\nfor each i in range(257) {\n    addEnd(ps, \"p\" + str(i))\n}\n"
  "os.writeFile(\"j.pr\", \"g = funct(\" + \", \".join(ps) + \") {\\n    return 1\\n}\\n\")\n"
  "post(os.run([sys.executable, \"j.pr\"], capture=True).split(\"\\n\")[0])\n",
  "SyntaxError: <funct>() tem parametros demais (maximo 256)", NULL, 0 },
{ "parametros: metodo com self + 256 parametros e recusado na declaracao",
  "import os\nimport sys\nps = []\nfor each i in range(256) {\n    addEnd(ps, \"p\" + str(i))\n}\n"
  "os.writeFile(\"j.pr\", \"Entity E {\\n    funct m(self, \" + \", \".join(ps) + \") {\\n        return 1\\n    }\\n}\\n\")\n"
  "post(os.run([sys.executable, \"j.pr\"], capture=True).split(\"\\n\")[0])\n",
  "SyntaxError: m() tem parametros demais (maximo 256)", NULL, 0 },
{ "parametros: Entity com 256 campos e recusada na declaracao",
  "import os\nimport sys\ncs = \"\"\nfor each i in range(256) {\n    cs = cs + \"    c\" + str(i) + \": int\\n\"\n}\n"
  "os.writeFile(\"j.pr\", \"Entity E {\\n\" + cs + \"}\\n\")\n"
  "post(os.run([sys.executable, \"j.pr\"], capture=True).split(\"\\n\")[0])\n",
  "SyntaxError: Entity E: o __init__ gerado dos campos tem parametros demais (maximo 256: self + 255 campos)", NULL, 0 },
{ "parametros: funct com exatamente 256 parametros roda",
  "import os\nimport sys\nps = []\nfor each i in range(256) {\n    addEnd(ps, \"p\" + str(i))\n}\n"
  "os.writeFile(\"j.pr\", \"funct f(\" + \", \".join(ps) + \") {\\n    return p0\\n}\\npost(f(*list(range(256))))\\n\")\n"
  "post(os.run([sys.executable, \"j.pr\"], capture=True).split(\"\\n\")[0])\n",
  "0", NULL, 0 },

/* ── tipo escrito junto da estrela: a mesma frase nas duas ordens ─────────── */
{ "variadico: tipo antes da estrela (int *args) recebe a mensagem dedicada do tipo em estrela",
  "funct f(int *args) {\n    return args\n}\n",
  "", "SyntaxError: parametro `*int args` nao aceita tipo: `*args` e sempre tup e `**kwarg` sempre dict — escreva `*args`", 2 },
{ "variadico: tipo antes da estrela (String *args) recebe a mensagem dedicada do tipo em estrela",
  "funct f(String *args) {\n    return args\n}\n",
  "", "SyntaxError: parametro `*String args` nao aceita tipo: `*args` e sempre tup e `**kwarg` sempre dict — escreva `*args`", 2 },
{ "tipo em estrela: dict **kw",
  "funct f(dict **kw) {\n    return kw\n}\n",
  "", "SyntaxError: parametro `**dict kw` nao aceita tipo: `*args` e sempre tup e `**kwarg` sempre dict — escreva `**kw`", 2 },
{ "tipo em estrela: lambda funct(int *a)",
  "f = funct(int *a) {\n    return a\n}\n",
  "", "SyntaxError: parametro `*int a` nao aceita tipo: `*args` e sempre tup e `**kwarg` sempre dict — escreva `*a`", 2 },
{ "tipo em estrela: metodo m(self, int *a)",
  "Entity E {\n    funct m(self, int *a) {\n        return a\n    }\n}\n",
  "", "SyntaxError: parametro `*int a` nao aceita tipo: `*args` e sempre tup e `**kwarg` sempre dict — escreva `*a`", 2 },

/* ── o frame corrente é raiz: closure e instância durante o C ────────────────
 * O registro do frame só era escrito quando ele CHAMAVA uma funct. O catch lia
 * a closure de lá, e o GC de dentro de um callback do C não via a closure em
 * execução nem a instância recém-criada (valgrind acusava leitura de bloco
 * liberado; a saída saía certa por acaso). */
/* Captura em LAMBDA: só a funct nomeada aninhada virava célula, então
 * `funct soma_de(a) { return funct(b) { return a + b } }` dava
 * "NameError: name 'a' is not defined" — em toda forma de entregar a lambda. */
{ "lambda devolvida captura a variavel de fora",
  "funct soma_de(a) {\n    return funct(b) {\n        return a + b\n    }\n}\nf = soma_de(10)\npost(f(5))\n",
  "15", NULL, 0 },
{ "lambda guardada em variavel, em lista e passada ao callback captura",
  "funct f(a) {\n    g = funct(b) {\n        return a + b\n    }\n    return [g, map([1, 2], funct(b) {\n        return a + b\n    })]\n}\nr = f(10)\npost(r[0](5), r[1])\n",
  "15 [11, 12]", NULL, 0 },
{ "lambda escreve na variavel capturada, como a funct nomeada",
  "funct conta() {\n    n = 0\n    return funct() {\n        n = n + 1\n        return n\n    }\n}\nc = conta()\npost(c(), c(), c())\n",
  "1 2 3", NULL, 0 },
{ "closure pega erro lancado nela mesma e ainda ve as celulas no catch",
  "funct fora() {\n    v = 10\n    funct dentro() {\n        try {\n            raise ValueError(\"x\")\n        } catch (ValueError e) {\n            return v\n        }\n    }\n    return dentro\n}\n"
  "g = fora()\nfunct chama() {\n    return g()\n}\npost(chama())\npost(g())\n",
  "10\n10", NULL, 0 },
{ "closure em execucao sobrevive ao GC de dentro de um callback do C",
  "funct fabrica(k) {\n    funct f(nums) {\n        r = map(nums, funct(v) {\n            for each i in range(40000) {\n                s = str(i) + \"bbbbbbbbbbbbbbbb\"\n            }\n            return v * k\n        })\n        return [r, k]\n    }\n    return f\n}\n"
  "post(fabrica(3)([1, 2]))\n",
  "[[3, 6], 3]", NULL, 0 },
{ "instancia descartada antes de um callback com GC nao e marcada depois de liberada",
  "Entity Ponto {\n    funct __init__(self, x, y) {\n        self.x = x\n    }\n}\n"
  "funct dobro(v) {\n    lixo = \"\"\n    for each i in range(60000) {\n        lixo = str(i) + \"aaaaaaaa\"\n    }\n    return v * 2\n}\n"
  "funct trabalha(nums) {\n    Ponto(1, 2)\n    for each i in range(60000) {\n        s = str(i) + \"bbbbbbbb\"\n    }\n    return map(nums, dobro)\n}\n"
  "post(trabalha([10, 20]))\n",
  "[20, 40]", NULL, 0 },
/* ── FOTOGRAFIAS, decisao aberta (revisao de 236eba8, 2026-09-14) ──────────
 * Os quatro casos abaixo gravam o que o motor faz HOJE, nao o que deve fazer:
 * sao a prova de que o comportamento e este, pra quem decidir ter de onde
 * partir. NAO sao pendentes.
 *
 * Teto da pilha de valores no `f(*lista)`: o CALL_EX confere
 * `sp + len + 2 >= stack_teto` (STACK_SIZE = 65536) e diz "argumentos demais
 * na chamada" sem nome da funct nem o limite. O que passa por essa conferencia
 * entra no frame: a cota dele e a profundidade exata do codigo do chamado
 * (`pilha_max`, 16.2.13) — antes era `ncode/2`, e 65520 itens que cabiam na
 * pilha saiam como RecursionError. Dentro de async o teto e o da fibra
 * (FIB_STACK = 2048), entao 3000 itens ja estouram onde no programa principal
 * cabem 65 mil. */
{ "espalhar: f(*lista) com 65520 itens cabe no CALL_EX e no frame (a cota do frame e a profundidade exata)",
  "funct f(*args) {\n    return len(args)\n}\nl = list(range(65520))\npost(f(*l))\n",
  "65520", NULL, 0 },
{ "espalhar: f(*lista) acima do teto da pilha e RuntimeError sem nome nem limite (fotografia)",
  "funct f(*args) {\n    return len(args)\n}\nl = list(range(100000))\npost(f(*l))\n",
  "", "RuntimeError: argumentos demais na chamada", 1 },
{ "espalhar: dentro de async o teto do f(*lista) e o da fibra, 2048 (fotografia)",
  "funct f(*args) {\n    return len(args)\n}\nasync funct g() {\n    l = list(range(3000))\n    return f(*l)\n}\npost(await g())\n",
  "", "RuntimeError: argumentos demais na chamada", 1 },
/* `static funct f(self, a, b=10)` chamado com 3 posicionais: antes de 236eba8
 * o CALL_KW contava o `self` ("from 2 to 3 ... but 4"); agora esconde o
 * buraco como o OP_CALL ("from 1 to 2 ... but 3"). A doc 14.1 promete
 * "f() takes N positional arguments but M were given" sem fixar N. */
{ "static com self chamado por nome com posicionais demais: frase do CALL_KW = a do OP_CALL (fotografia)",
  "Entity C {\n    static funct f(self, a, b=10) {\n        return a + b\n    }\n}\npost(C.f(1, 2, 3, b=4))\n",
  "", "TypeError: f() takes from 1 to 2 positional arguments but 3 were given", 2 },

/* ── import *: as três grafias e as recusas ──────────────────────────────────
 * `from m import *`, `import m *` e `PUSH m GET *` são a mesma regra: ligam os
 * nomes que o módulo exporta, e não o nome do módulo. Os casos com arquivo
 * `.pr` (private, pré-ligado, bloco, reexportação, ciclo) moram em
 * teste/cobre_stdlib.pr, que monta os arquivos. */
{ "import *: from json import *",
  "from json import *\npost(stringify([1]), parse(\"[2]\"))\n", "[1] [2]", NULL, 0 },
{ "import *: import json *",
  "import json *\npost(stringify([1]))\n", "[1]", NULL, 0 },
{ "import *: PUSH json GET *",
  "PUSH json GET *\npost(stringify([1]))\n", "[1]", NULL, 0 },
{ "import *: modulo entre aspas por nome",
  "from 'json' import *\npost(stringify([1]))\n", "[1]", NULL, 0 },
{ "import *: import m * nao liga o nome do modulo",
  "import regex *\npost(regex)\n",
  "", "NameError: name 'regex' is not defined. Did you forget to import 'regex'?", 2 },
{ "import *: modulo ausente e ImportError na linha do import",
  "post(1)\nfrom naoexiste import *\n", "1", "ImportError: No module named 'naoexiste'", 1 },
{ "import *: dentro de funct e recusado",
  "funct g() {\n    from json import *\n}\n",
  "", "SyntaxError: `*` do import so vale no topo do arquivo; dentro de funct ou bloco nomeie o que usa: from json import a, b", 2 },
{ "import *: dentro de metodo e recusado",
  "Entity E {\n    funct m(self) {\n        import json *\n    }\n}\n",
  "", "SyntaxError: `*` do import so vale no topo do arquivo; dentro de funct ou bloco nomeie o que usa: from json import a, b", 2 },
{ "import *: dentro de lambda e recusado",
  "f = funct() {\n    PUSH json GET *\n}\n",
  "", "SyntaxError: `*` do import so vale no topo do arquivo; dentro de funct ou bloco nomeie o que usa: from json import a, b", 2 },
{ "import *: dentro de if e recusado",
  "if True {\n    from json import *\n}\n",
  "", "SyntaxError: `*` do import so vale no topo do arquivo; dentro de funct ou bloco nomeie o que usa: from json import a, b", 2 },
{ "import *: dentro de try e recusado",
  "try {\n    from json import *\n} catch (ImportError e) {\n    post(1)\n}\n",
  "", "SyntaxError: `*` do import so vale no topo do arquivo; dentro de funct ou bloco nomeie o que usa: from json import a, b", 2 },
{ "import *: dentro do guard e recusado",
  "if __name__ == \"main\" {\n    from json import *\n}\n",
  "", "SyntaxError: `*` do import so vale no topo do arquivo; dentro de funct ou bloco nomeie o que usa: from json import a, b", 2 },
{ "import *: caminho entre aspas dentro de funct cita o caminho",
  "funct g() {\n    from './x.pr' import *\n}\n",
  "", "SyntaxError: `*` do import so vale no topo do arquivo; dentro de funct ou bloco nomeie o que usa: from './x.pr' import a, b", 2 },
{ "import *: import m as x * e recusado",
  "import json as j *\n",
  "", "SyntaxError: `import m as x *` mistura as duas formas: `import m as x` liga o modulo, `import m *` liga os nomes dele — escolha uma", 2 },
{ "import *: PUSH m as x GET * e recusado",
  "PUSH json as j GET *\n",
  "", "SyntaxError: `PUSH m as x GET *` mistura as duas formas: `as` nomeia o modulo, `GET *` liga os nomes dele — escolha uma", 2 },
{ "import *: lista com * no fim e recusada",
  "from json import parse, *\n",
  "", "SyntaxError: `*` traz todos os nomes e nao se mistura com uma lista: escreva so `*` ou so os nomes (a, b)", 2 },
{ "import *: * seguido de lista e recusado",
  "from json import *, parse\n",
  "", "SyntaxError: `*` traz todos os nomes e nao se mistura com uma lista: escreva so `*` ou so os nomes (a, b)", 2 },
{ "import *: * com as e recusado",
  "from json import * as t\n",
  "", "SyntaxError: `*` nao aceita `as`: ele liga cada nome com o proprio nome", 2 },
/* `bytes` exporta `hex`: depois do `*` o nome é o do módulo, como seria com
 * `from bytes import hex` — a linha de cima ainda usa o embutido */
{ "import *: membro do modulo sombreia o builtin de mesmo nome a partir do import",
  "post(hex(255))\nfrom bytes import *\npost(hex(new([255, 1])))\n", "0xff\nff01", NULL, 0 },

/* ── `public`/`private` em QUALQUER declaracao do topo do modulo (2026-09-26) ──
 * Antes so funct e class aceitavam; variavel, desempacotamento, model e enum
 * eram recusados com a frase errada ("so vale antes de class/Entity, de funct
 * ou de '<tipo> <nome> = <valor>'"), e `private int n = 1` no topo era lido
 * como campo de objeto. O que nao sai pelo import esta em cli_roda.pr (dois
 * arquivos); aqui e a declaracao em si e o `private` fora do lugar, que
 * compilava CALADO em funct aninhada. */
{ "private em variavel do topo: roda e o nome existe no arquivo",
  "private x = 1\npost(x)\n", "1", NULL, 0 },
{ "private em variavel tipada do topo e a variavel tipada de sempre",
  "private int n = 1\nn = n + 1\npost(n)\n", "2", NULL, 0 },
{ "private em variavel tipada confere o tipo como a sem modificador",
  "private int n = 1\nn = \"a\"\n", "", "variável n esperava int, recebeu str", 2 },
{ "private com tipo de classe que nao existe: o erro de tipo de sempre",
  "private Conta c = 1\n", "", "tipo Conta não existe (variável c)", 2 },
{ "private em desempacotamento marca os dois nomes",
  "private a, b = 1, 2\npost(a, b)\n", "1 2", NULL, 0 },
{ "private em desempacotamento aninhado",
  "private (a, b), c = (1, 2), 3\npost(a, b, c)\n", "1 2 3", NULL, 0 },
{ "private model do topo valida como sempre",
  "private model M() {\n    x: int\n}\npost({\"x\": 1} == M)\n", "True", NULL, 0 },
{ "private enum do topo",
  "private enum E {\n    A = 1\n}\npost(E.A)\n", "1", NULL, 0 },
{ "private class do topo continua valendo",
  "private class K {\n    funct m(self) {\n        return 2\n    }\n}\npost(K().m())\n", "2", NULL, 0 },
{ "public na variavel do topo e aceito (e o default)",
  "public x = 1\npost(x)\n", "1", NULL, 0 },
{ "o private e do nome: reatribuir depois continua valendo",
  "private x = 1\nx = 2\npost(x)\n", "2", NULL, 0 },
{ "private x += 1 nao e declaracao",
  "x = 1\nprivate x += 1\n", "", "'private' vai na declaracao (private x = ...), nao na atribuicao composta", 2 },
{ "private antes de if nao e declaracao",
  "private if True {\n    post(1)\n}\n", "",
  "'private' so vale numa declaracao: funct, class, model, enum ou variavel (private x = 1, private int x = 1)", 2 },
{ "private sozinho na linha nao e declaracao",
  "private\nx = 1\n", "",
  "'private' so vale numa declaracao: funct, class, model, enum ou variavel (private x = 1, private int x = 1)", 2 },
{ "private funct aninhada em funct nao tem efeito: e erro, nao silencio",
  "funct g() {\n    private funct h() {\n        return 1\n    }\n    return h()\n}\npost(g())\n", "",
  "'private' aqui nao tem efeito: so vale no topo do arquivo (o que nao sai pelo import) ou no corpo da Entity", 2 },
{ "private variavel dentro de funct: erro",
  "funct g() {\n    private x = 1\n}\n", "", "'private' aqui nao tem efeito: so vale no topo do arquivo (o que nao sai pelo import) ou no corpo da Entity", 2 },
{ "private variavel dentro de bloco do topo: erro",
  "if True {\n    private x = 1\n}\n", "", "'private' aqui nao tem efeito: so vale no topo do arquivo (o que nao sai pelo import) ou no corpo da Entity", 2 },
{ "private int x dentro de bloco do topo: erro (nao e campo, nao e o topo)",
  "if True {\n    private int x = 1\n}\n", "", "'private' aqui nao tem efeito: so vale no topo do arquivo (o que nao sai pelo import) ou no corpo da Entity", 2 },
{ "private nome = valor dentro de metodo: a forma do campo leva o tipo",
  "Entity A {\n    funct __init__(self) {\n        private x = 1\n    }\n}\n", "",
  "'private x = ...' dentro de uma funct de Entity declara campo do objeto: escreva 'private <tipo> x = <valor>'", 2 },
{ "private class dentro de funct: erro",
  "funct g() {\n    private class K {\n    }\n}\n", "", "'private' aqui nao tem efeito: so vale no topo do arquivo (o que nao sai pelo import) ou no corpo da Entity", 2 },
{ "private str x = v dentro de metodo de Entity continua campo do objeto",
  "Entity A {\n    funct __init__(self) {\n        private str x = \"a\"\n    }\n    funct le(self) {\n        return self.x\n    }\n}\npost(A().le())\n",
  "a", NULL, 0 },

/* ── NameError com "Did you mean" antes de rodar (2026-09-28) ─────────────
 * O checador so dizia que o nome nao existe; agora sugere o nome parecido
 * que existe no escopo, no arquivo ou entre os que a VM liga sozinha — a
 * mesma regra de distancia do "Did you mean" do AttributeError. */
{ "NameError antes de rodar sugere o nome parecido que existe",
  "valor = 1\nif vlaor == 1 {\n    post(1)\n}\n", "", "NameError: name 'vlaor' is not defined. Did you mean: 'valor'?", 2 },

/* ── funções: padrão, async com yield, saída como valor, sleep(0), nonnull do
 * gerador (Plano 6, lote 6a, 2026-10-08) ──────────────────────────────────
 * O padrão que lia o parâmetro de depois virava Null calado; `async funct`
 * com `yield` virava gerador comum; `post` por apelido ou pelo `map` escapava
 * da regra da saída da funct tipada (decisão dele: erro antes de rodar);
 * `sleep(0)` não cedia numa tarefa; o `nonnull` do gerador só acusava no 1º
 * pedido. */
{ "padrao que le o parametro de depois: erro antes de rodar (lote 6a)",
  "funct f(a = b, int b = 1) {\n    return a\n}\npost(f())\n",
  "", "NameError: o padrão de 'a' lê 'b', um parâmetro que vem depois e ainda não tem valor: "
  "no padrão só valem os parâmetros de antes", 2 },
{ "padrao que le o proprio parametro: erro antes de rodar (lote 6a)",
  "funct g(a = a) {\n    return a\n}\npost(g())\n",
  "", "NameError: o padrão de 'a' lê o próprio 'a', que ainda não tem valor: "
  "no padrão só valem os parâmetros de antes", 2 },
{ "padrao que le o parametro de antes vale (lote 6a)",
  "funct h(int a, int b = a * 2) {\n    return b\n}\npost(h(3), h(3, 1))\n",
  "6 1", NULL, 0 },
{ "async funct com yield: erro antes de rodar, no yield (lote 6a)",
  "async funct g() {\n    sleep(0.01)\n    yield 1\n}\npost(\"nao roda\")\n",
  "", "SyntaxError: 'yield' em async funct: a funct e tarefa (async) ou gerador (yield), nao as duas", 2 },
{ "post como valor numa funct tipada: erro antes de rodar (lote 6a)",
  "int funct f() {\n    Object p = post\n    p(\"texto\")\n    return 1\n}\npost(f())\n",
  "", "AttributedValueError: post como valor em f(), que devolve int: a saída dele não seria "
  "conferida (chame post direto)", 2 },
{ "map(l, post) numa funct tipada: erro antes de rodar (lote 6a)",
  "int funct g() {\n    map([\"via map\"], post)\n    return 2\n}\npost(g())\n",
  "", "AttributedValueError: post como valor em g(), que devolve int: a saída dele não seria "
  "conferida (chame post direto)", 2 },
{ "sys.stdout.write como valor numa funct tipada: erro antes de rodar (lote 6a)",
  "import sys\nstr funct h() {\n    Object w = sys.stdout.write\n    return \"h\"\n}\npost(h())\n",
  "", "AttributedValueError: sys.stdout.write como valor em h(), que devolve str: a saída dele não "
  "seria conferida (chame sys.stdout.write direto)", 2 },
{ "saida como valor vale fora da funct tipada, e a chamada direta e a lambda continuam (lote 6a)",
  "int funct direto() {\n    post(7)\n    return 4\n}\n"
  "funct livre() {\n    Object p = post\n    p(\"livre\")\n}\n"
  "Null funct sem_valor() {\n    Object p = post\n    p(\"null funct\")\n}\n"
  "int funct com_lambda() {\n    map([\"da lambda\"], funct(str x) { post(x) })\n    return 5\n}\n"
  "post(direto())\nlivre()\nsem_valor()\npost(com_lambda())\n",
  "7\n4\nlivre\nnull funct\nda lambda\n5", NULL, 0 },
{ "sleep(0) numa tarefa cede, e sleep de texto diz que aceita int ou flo (lote 6a)",
  "list ordem = []\nasync funct a() {\n    ordem.append(\"a1\")\n    sleep(0)\n    ordem.append(\"a2\")\n}\n"
  "async funct b() {\n    ordem.append(\"b1\")\n    sleep(0)\n    ordem.append(\"b2\")\n}\n"
  "gather(a(), b())\npost(ordem)\ntry {\n    sleep([\"x\"][0])\n} catch (TypeError e) {\n    post(e)\n}\n",
  "['a1', 'b1', 'a2', 'b2']\nsleep() argument must be int or flo, not str (linha 15)", NULL, 0 },
{ "nonnull do gerador acusa na chamada, nao no primeiro pedido (lote 6a)",
  "nonnull funct gen(v) {\n    yield v\n}\ntry {\n    Object g = gen(Null)\n    post(\"nao chega\")\n"
  "} catch (RuntimeError e) {\n    post(e)\n}\npost(list(gen(3)))\n",
  "nonnull: parametro 'v' em 'gen' nao pode ser Null (linha 5)\n[3]", NULL, 0 },

/* ── closures e escopo de bloco (Plano 6, lote 6b, 2026-10-08) ─────────────
 * A escrita a um nome de fora não chegava quando uma funct aninhada citava o
 * nome; a closure criada no laço via o valor da última volta (decisão 4:
 * escopo de verdade, cada volta é uma variável); a funct escrita FORA de um
 * bloco enxergava a variável dele enquanto ele rodava — agora é NameError
 * antes de rodar, com a frase que diz por quê; o parâmetro com o nome de uma
 * variável capturada lá fora gravava nela; a lambda no padrão não capturava
 * o parâmetro de antes; o `nonnull` deixava passar o Null do parâmetro
 * capturado. */
{ "escrita a global chega com uma lambda citando o nome (lote 6b)",
  "int pontos = 0\n"
  "list funct soma_tudo(list l) {\n"
  "    pontos += 100\n"
  "    list r = map(l, funct(int x) { return x + pontos })\n"
  "    return r\n"
  "}\n"
  "post(soma_tudo([1, 2]), pontos)\n",
  "[101, 102] 100", NULL, 0 },
{ "escrita a variavel da funct de fora chega com funct aninhada citando o nome (lote 6b)",
  "int funct a1() {\n"
  "    int total = 0\n"
  "    int funct b() {\n"
  "        total += 1\n"
  "        int funct c() { return total }\n"
  "        return c()\n"
  "    }\n"
  "    return b()\n"
  "}\n"
  "list funct a2() {\n"
  "    int total = 0\n"
  "    int funct b() {\n"
  "        total = 5\n"
  "        int funct c() { return total }\n"
  "        return c()\n"
  "    }\n"
  "    int r = b()\n"
  "    return [r, total]\n"
  "}\n"
  "post(a1(), a2())\n",
  "1 [5, 5]", NULL, 0 },
{ "escrita a global de dentro de um laco com funct citando o nome chega (lote 6b)",
  "int total = 0\n"
  "Null funct f() {\n"
  "    for each i in [1, 2, 3] {\n"
  "        total = total + i\n"
  "        int funct le() { return total }\n"
  "        le()\n"
  "    }\n"
  "}\n"
  "f()\n"
  "post(total)\n",
  "6", NULL, 0 },
{ "atribuir ao nome da propria funct de fora grava nela, com ou sem funct aninhada: erro de tipo (lote 6b)",
  "async int funct a() {\n"
  "    a = 1\n"
  "    int funct inc() {\n"
  "        a = a + 1\n"
  "        return a\n"
  "    }\n"
  "    post(inc(), inc())\n"
  "    return 1\n"
  "}\n"
  "post(await a())\n",
  "", "AttributedValueError: variável a é funct (tipo fixado na primeira atribuição), recebeu int", 2 },
{ "closure no for each da funct guarda a variavel da volta e a declarada no corpo (lote 6b)",
  "list funct f() {\n"
  "    list fs = []\n"
  "    for each i in [1, 2, 3] {\n"
  "        int v = i * 10\n"
  "        fs.append(funct() { return i + v })\n"
  "    }\n"
  "    list r = []\n"
  "    for each g in fs { r.append(g()) }\n"
  "    return r\n"
  "}\n"
  "post(f())\n",
  "[11, 22, 33]", NULL, 0 },
{ "closure no while da funct guarda a volta, com continue e break (lote 6b)",
  "list funct f() {\n"
  "    list fs = []\n"
  "    int c = 0\n"
  "    while c < 5 {\n"
  "        int z = c\n"
  "        c += 1\n"
  "        if z == 1 { continue }\n"
  "        if z == 3 { break }\n"
  "        fs.append(funct() { return z })\n"
  "    }\n"
  "    list r = []\n"
  "    for each g in fs { r.append(g()) }\n"
  "    return r\n"
  "}\n"
  "post(f())\n",
  "[0, 2]", NULL, 0 },
{ "closure na compreensao da funct guarda o item (lote 6b)",
  "list funct f() {\n"
  "    list fs = [funct() { return x } for each x in [1, 2, 3]]\n"
  "    list r = []\n"
  "    for each g in fs { r.append(g()) }\n"
  "    return r\n"
  "}\n"
  "post(f())\n",
  "[1, 2, 3]", NULL, 0 },
{ "funct escrita antes da declaracao, na mesma volta, ve a variavel da volta (lote 6b)",
  "list funct f() {\n"
  "    list r = []\n"
  "    for each i in [1, 2, 3] {\n"
  "        int funct g() { return v }\n"
  "        int v = i\n"
  "        r.append(g())\n"
  "    }\n"
  "    return r\n"
  "}\n"
  "post(f())\n",
  "[1, 2, 3]", NULL, 0 },
{ "variavel declarada fora do laco segue uma so pras closures (lote 6b)",
  "list funct f() {\n"
  "    list fs = []\n"
  "    int t = 0\n"
  "    for each i in [1, 2, 3] {\n"
  "        t = i\n"
  "        fs.append(funct() { return t })\n"
  "    }\n"
  "    list r = []\n"
  "    for each g in fs { r.append(g()) }\n"
  "    return r\n"
  "}\n"
  "post(f())\n",
  "[3, 3, 3]", NULL, 0 },
{ "gerador, tarefa e classe criados no laco da funct guardam a volta (lote 6b)",
  "list funct f() {\n"
  "    list gs = []\n"
  "    list ts = []\n"
  "    list objs = []\n"
  "    for each i in [1, 2] {\n"
  "        funct gen() { yield i }\n"
  "        gs.append(gen())\n"
  "        async int funct t() { return i * 10 }\n"
  "        ts.append(t())\n"
  "        class K {\n"
  "            int funct m(self) { return i * 100 }\n"
  "        }\n"
  "        objs.append(K())\n"
  "    }\n"
  "    return [list(gs[0]), list(gs[1]), await ts[0], await ts[1], objs[0].m(), objs[1].m()]\n"
  "}\n"
  "post(f())\n",
  "[[1], [2], 10, 20, 100, 200]", NULL, 0 },
{ "desempacotamento no for each da funct: a closure captura os dois nomes (lote 6b)",
  "list funct f() {\n"
  "    list fs = []\n"
  "    for each k, v in {\"a\": 1, \"b\": 2}.items() {\n"
  "        fs.append(funct() { return k + str(v) })\n"
  "    }\n"
  "    return [fs[0](), fs[1]()]\n"
  "}\n"
  "post(f())\n",
  "['a1', 'b2']", NULL, 0 },
{ "continue e break no for each com closure: cada volta a sua (lote 6b)",
  "list funct f() {\n"
  "    list fs = []\n"
  "    for each i in [1, 2, 3, 4] {\n"
  "        int v = i\n"
  "        if v == 2 { continue }\n"
  "        fs.append(funct() { return v })\n"
  "        if v == 3 { break }\n"
  "    }\n"
  "    list r = []\n"
  "    for each g in fs { r.append(g()) }\n"
  "    return r\n"
  "}\n"
  "post(f())\n",
  "[1, 3]", NULL, 0 },
{ "dois lacos com o mesmo nome e a declaracao de depois: tres variaveis (lote 6b)",
  "list funct f() {\n"
  "    list fs = []\n"
  "    for each i in [1, 2] {\n"
  "        int v = i * 10\n"
  "        fs.append(funct() { return v })\n"
  "    }\n"
  "    for each j in [3, 4] {\n"
  "        int v = j * 10\n"
  "        fs.append(funct() { return v })\n"
  "    }\n"
  "    int v = 0\n"
  "    list r = []\n"
  "    for each g in fs { r.append(g()) }\n"
  "    return [r, v]\n"
  "}\n"
  "post(f())\n",
  "[[10, 20, 30, 40], 0]", NULL, 0 },
{ "bloco de dentro com o nome que o de fora declara depois: duas variaveis (lote 6b)",
  "list funct f() {\n"
  "    list fs = []\n"
  "    if True {\n"
  "        if True {\n"
  "            int v = 1\n"
  "            fs.append(funct() { return v })\n"
  "        }\n"
  "        int v = 2\n"
  "        fs.append(funct() { return v })\n"
  "    }\n"
  "    return [fs[0](), fs[1]()]\n"
  "}\n"
  "post(f())\n",
  "[1, 2]", NULL, 0 },
{ "erro dentro do bloco: a variavel capturada nele nao passa pro catch (lote 6b)",
  "int funct f() {\n"
  "    list fs = []\n"
  "    try {\n"
  "        if True {\n"
  "            int v = 7\n"
  "            fs.append(funct() { return v })\n"
  "            raise ValueError(\"x\")\n"
  "        }\n"
  "    } catch (ValueError e) {\n"
  "        int v = 9\n"
  "    }\n"
  "    return fs[0]()\n"
  "}\n"
  "post(f())\n",
  "7", NULL, 0 },
{ "laco que sombreia um nome capturado: as closures veem a volta, a de fora volta inteira (lote 6b)",
  "list funct s1() {\n"
  "    int i = 99\n"
  "    int funct h() { return i }\n"
  "    list fs = []\n"
  "    for each i in [1, 2, 3] {\n"
  "        fs.append(funct() { return i })\n"
  "    }\n"
  "    list r = []\n"
  "    for each g in fs { r.append(g()) }\n"
  "    i = 5\n"
  "    return [r, h(), i]\n"
  "}\n"
  "list funct s2() {\n"
  "    int x = 99\n"
  "    int funct h() { return x }\n"
  "    list fs = [funct() { return x } for each x in [1, 2, 3]]\n"
  "    list r = []\n"
  "    for each g in fs { r.append(g()) }\n"
  "    x = 7\n"
  "    return [r, h(), x]\n"
  "}\n"
  "post(s1(), s2())\n",
  "[[1, 2, 3], 5, 5] [[1, 2, 3], 7, 7]", NULL, 0 },
{ "o parametro e a variavel do laco sao da propria funct, nao a de fora de mesmo nome (lote 6b)",
  "list funct f() {\n"
  "    int v = 1\n"
  "    int funct h() { return v }\n"
  "    int funct g(int v) {\n"
  "        v = 5\n"
  "        return v\n"
  "    }\n"
  "    int r = g(2)\n"
  "    return [r, v, h()]\n"
  "}\n"
  "list funct k(int x) {\n"
  "    list fs = [funct() { return x } for each x in [1, 2]]\n"
  "    return [fs[0](), fs[1](), x]\n"
  "}\n"
  "list funct m() {\n"
  "    int i = 99\n"
  "    int funct h() { return i }\n"
  "    list funct g() {\n"
  "        list fs = []\n"
  "        for each i in [1, 2] {\n"
  "            fs.append(funct() { return i })\n"
  "        }\n"
  "        return [fs[0](), fs[1]()]\n"
  "    }\n"
  "    list r = g()\n"
  "    return [r, i, h()]\n"
  "}\n"
  "post(f(), k(9), m())\n",
  "[5, 1, 1] [1, 2, 9] [[1, 2], 99, 99]", NULL, 0 },
{ "redeclarar no bloco o nome que ja existe e a mesma variavel, com closure (lote 6b)",
  "list funct f() {\n"
  "    int x = 1\n"
  "    list fs = []\n"
  "    if True {\n"
  "        int x = 2\n"
  "        fs.append(funct() { return x })\n"
  "    }\n"
  "    return [x, fs[0]()]\n"
  "}\n"
  "list funct g() {\n"
  "    int t = 0\n"
  "    list fs = []\n"
  "    for each i in [1, 2, 3] {\n"
  "        int t = i\n"
  "        fs.append(funct() { return t })\n"
  "    }\n"
  "    return [t, fs[0](), fs[2]()]\n"
  "}\n"
  "post(f(), g())\n",
  "[2, 2] [3, 3, 3]", NULL, 0 },
{ "recursao e recursao mutua de funct declarada num bloco da funct (lote 6b)",
  "int funct f() {\n"
  "    if True {\n"
  "        int funct fat(int k) {\n"
  "            if k <= 1 { return 1 }\n"
  "            return k * fat(k - 1)\n"
  "        }\n"
  "        return fat(5)\n"
  "    }\n"
  "    return 0\n"
  "}\n"
  "bool funct g() {\n"
  "    if True {\n"
  "        bool funct par(int n) {\n"
  "            if n == 0 { return True }\n"
  "            return impar(n - 1)\n"
  "        }\n"
  "        bool funct impar(int n) {\n"
  "            if n == 0 { return False }\n"
  "            return par(n - 1)\n"
  "        }\n"
  "        return par(10)\n"
  "    }\n"
  "    return False\n"
  "}\n"
  "post(f(), g())\n",
  "120 True", NULL, 0 },
{ "bloco de dentro do laco ve a variavel da volta; atribuicao nova no corpo e da volta (lote 6b)",
  "list funct f() {\n"
  "    list fs = []\n"
  "    for each i in [1, 2] {\n"
  "        int a = i\n"
  "        if True {\n"
  "            fs.append(funct() { return a })\n"
  "        }\n"
  "        k = i * 3\n"
  "        fs.append(funct() { return k })\n"
  "    }\n"
  "    list r = []\n"
  "    for each g in fs { r.append(g()) }\n"
  "    return r\n"
  "}\n"
  "post(f())\n",
  "[1, 3, 2, 6]", NULL, 0 },
{ "topo: closure no for each, na funct nomeada do laco e na compreensao guarda a volta (lote 6b)",
  "list fs = []\n"
  "for each i in [1, 2, 3] {\n"
  "    int v = i * 10\n"
  "    fs.append(funct() { return i + v })\n"
  "}\n"
  "list fs2 = []\n"
  "for each n in [1, 2, 3] {\n"
  "    int funct nomeada() { return n * 2 }\n"
  "    fs2.append(nomeada)\n"
  "}\n"
  "list fs3 = [funct() { return x } for each x in [7, 8, 9]]\n"
  "list r = []\n"
  "for each g in fs + fs2 + fs3 { r.append(g()) }\n"
  "post(r)\n",
  "[11, 22, 33, 2, 4, 6, 7, 8, 9]", NULL, 0 },
{ "topo: closure no while guarda a volta, com continue e break (lote 6b)",
  "list fs = []\n"
  "int cont = 0\n"
  "while cont < 5 {\n"
  "    int z = cont\n"
  "    cont += 1\n"
  "    if z == 1 { continue }\n"
  "    if z == 3 { break }\n"
  "    fs.append(funct() { return z })\n"
  "}\n"
  "list r = []\n"
  "for each g in fs { r.append(g()) }\n"
  "post(r)\n",
  "[0, 2]", NULL, 0 },
{ "topo: closure criada num bloco roda depois dele; blocos aninhados e compreensao no bloco (lote 6b)",
  "list fs = []\n"
  "if True {\n"
  "    int w = 5\n"
  "    fs.append(funct() { return w })\n"
  "    if True {\n"
  "        int v = 1\n"
  "        fs.append(funct() { return v + w })\n"
  "    }\n"
  "    int v = 2\n"
  "    fs.append(funct() { return v })\n"
  "}\n"
  "list r = []\n"
  "if True {\n"
  "    int b0 = 100\n"
  "    list gs = [funct() { return b0 + x } for each x in [1, 2]]\n"
  "    for each g in gs { r.append(g()) }\n"
  "}\n"
  "post(fs[0](), fs[1](), fs[2](), r)\n",
  "5 6 2 [101, 102]", NULL, 0 },
{ "topo: gerador, campo e metodo de classe e lambda dentro de lambda no laco guardam a volta (lote 6b)",
  "list gs = []\n"
  "list objs = []\n"
  "list ls = []\n"
  "for each i in [1, 2] {\n"
  "    funct gen() {\n"
  "        yield i\n"
  "        yield i * 10\n"
  "    }\n"
  "    gs.append(gen())\n"
  "    class K {\n"
  "        int v = i\n"
  "        int funct m(self) { return i * 100 }\n"
  "    }\n"
  "    objs.append(K())\n"
  "    ls.append(funct() { return funct() { return i } })\n"
  "}\n"
  "post(list(gs[0]), list(gs[1]), objs[0].v, objs[1].m(), ls[0]()(), ls[1]()())\n",
  "[1, 10] [2, 20] 1 200 1 2", NULL, 0 },
{ "topo: desempacotamento no laco e laco que sombreia uma global capturada (lote 6b)",
  "list fs = []\n"
  "for each k, v in {\"a\": 1, \"b\": 2}.items() {\n"
  "    fs.append(funct() { return k + str(v) })\n"
  "}\n"
  "int i = 99\n"
  "int funct h() { return i }\n"
  "list hs = []\n"
  "for each i in [1, 2] {\n"
  "    hs.append(funct() { return i })\n"
  "}\n"
  "post(fs[0](), fs[1](), i, h(), hs[0](), hs[1]())\n",
  "a1 b2 99 99 1 2", NULL, 0 },
{ "topo: redeclarar no bloco o nome que ja existe e a mesma variavel; o declarado depois e outra (lote 6b)",
  "int x = 1\n"
  "list fs = []\n"
  "if True {\n"
  "    int x = 2\n"
  "    fs.append(funct() { return x })\n"
  "}\n"
  "int t = 0\n"
  "list ts = []\n"
  "for each i in [1, 2, 3] {\n"
  "    int t = i\n"
  "    ts.append(funct() { return t })\n"
  "}\n"
  "list ws = []\n"
  "for each i in [1, 2] {\n"
  "    int w = i\n"
  "    ws.append(funct() { return w })\n"
  "}\n"
  "int w = 0\n"
  "post(x, fs[0](), t, ts[0](), ts[2](), ws[0](), ws[1](), w)\n",
  "2 2 3 3 3 1 2 0", NULL, 0 },
{ "topo: a global lida na closure do bloco segue a global; variavel de bloco sem captura some no fim (lote 6b)",
  "int glob = 1\n"
  "list fs = []\n"
  "for each b in [1] {\n"
  "    fs.append(funct() { return glob + b })\n"
  "}\n"
  "glob = 10\n"
  "post(fs[0]())\n"
  "for each sem in [1] {\n"
  "    int livre = 3\n"
  "}\n"
  "try {\n"
  "    post(livre)\n"
  "} catch (NameError e) {\n"
  "    post(e)\n"
  "}\n",
  "11\nname 'livre' is not defined (linha 12)", NULL, 0 },
{ "funct escrita fora do bloco nao enxerga a variavel dele: topo, if (lote 6b)",
  "int funct le() { return w }\n"
  "if True {\n"
  "    int w = 5\n"
  "    post(le())\n"
  "}\n",
  "", "NameError: name 'w' is not defined: 'w' só existe dentro de um bloco, e esta funct foi escrita "
  "fora dele (declare 'w' antes do bloco)", 2 },
{ "funct escrita fora do laco nao enxerga a variavel dele: topo, for each (lote 6b)",
  "int funct le() { return i }\n"
  "list r = []\n"
  "for each i in [1, 2] {\n"
  "    r.append(le())\n"
  "}\n"
  "post(r)\n",
  "", "NameError: name 'i' is not defined: 'i' só existe dentro de um bloco, e esta funct foi escrita "
  "fora dele (declare 'i' antes do bloco)", 2 },
{ "funct escrita fora do guard nao enxerga a variavel dele (lote 6b)",
  "int funct mostra() { return porta }\n"
  "if __name__ == \"main\" {\n"
  "    int porta = 8080\n"
  "    post(mostra())\n"
  "}\n",
  "", "NameError: name 'porta' is not defined: 'porta' só existe dentro de um bloco, e esta funct foi "
  "escrita fora dele (declare 'porta' antes do bloco)", 2 },
{ "funct escrita fora do bloco nao enxerga a variavel dele: dentro de funct, if (lote 6b)",
  "int funct f() {\n"
  "    int funct le() { return w }\n"
  "    if True {\n"
  "        int w = 5\n"
  "        return le()\n"
  "    }\n"
  "    return 0\n"
  "}\n"
  "post(f())\n",
  "", "NameError: name 'w' is not defined: 'w' só existe dentro de um bloco, e esta funct foi escrita "
  "fora dele (declare 'w' antes do bloco)", 2 },
{ "funct escrita fora do laco nao enxerga a variavel dele: dentro de funct, for each (lote 6b)",
  "list funct f() {\n"
  "    int funct le() { return i }\n"
  "    list r = []\n"
  "    for each i in [1, 2] {\n"
  "        r.append(le())\n"
  "    }\n"
  "    return r\n"
  "}\n"
  "post(f())\n",
  "", "NameError: name 'i' is not defined: 'i' só existe dentro de um bloco, e esta funct foi escrita "
  "fora dele (declare 'i' antes do bloco)", 2 },
{ "funct escrita num bloco irmao nao enxerga a variavel do outro (lote 6b)",
  "int funct f() {\n"
  "    if True {\n"
  "        int w = 1\n"
  "    }\n"
  "    if True {\n"
  "        int funct g() { return w }\n"
  "        return g()\n"
  "    }\n"
  "    return 0\n"
  "}\n"
  "post(f())\n",
  "", "NameError: name 'w' is not defined: 'w' só existe dentro de um bloco, e esta funct foi escrita "
  "fora dele (declare 'w' antes do bloco)", 2 },
{ "funct escrita fora do guard nao enxerga o import feito nele (lote 6b)",
  "if __name__ == \"main\" {\n"
  "    import json\n"
  "}\n"
  "str funct f() { return json.stringify([2]) }\n"
  "post(f())\n",
  "", "NameError: name 'json' is not defined: 'json' só existe dentro de um bloco, e esta funct foi "
  "escrita fora dele (declare 'json' antes do bloco)", 2 },
{ "a funct enxerga a global de fora do bloco, o nome do arquivo declarado depois e o do bloco onde foi escrita (lote 6b)",
  "int w = 100\n"
  "int funct f() {\n"
  "    int funct g() { return w }\n"
  "    if True {\n"
  "        int w = 1\n"
  "    }\n"
  "    return g()\n"
  "}\n"
  "int funct le() { return depois }\n"
  "int depois = 4\n"
  "if __name__ == \"main\" {\n"
  "    int porta = 8080\n"
  "    int funct mostra() { return porta }\n"
  "    post(f(), le(), mostra())\n"
  "}\n",
  "100 4 8080", NULL, 0 },
{ "lambda no padrao captura o parametro de antes (lote 6b)",
  "int funct k2(int b = 1, a = funct(int x) { return x + b }) {\n"
  "    return a(1)\n"
  "}\n"
  "int funct k3(int b = 1, a = funct(int b) { return b * 2 }) {\n"
  "    return a(4)\n"
  "}\n"
  "post(k2(), k2(5), k3())\n",
  "2 6 8", NULL, 0 },
{ "lambda no padrao que captura o parametro de depois: erro antes de rodar (lote 6b)",
  "int funct k(a = funct(int x) { return x + b }, int b = 1) {\n"
  "    return a(1)\n"
  "}\n"
  "post(k())\n",
  "", "NameError: a lambda no padrão de 'a' captura 'b', que ainda não tem valor: no padrão só valem "
  "os parâmetros de antes", 2 },
{ "nonnull recusa o Null do parametro capturado, no padrao e no corpo (lote 6b)",
  "nonnull int funct k4(v, a = funct() { return v }) {\n"
  "    return a()\n"
  "}\n"
  "nonnull int funct f(v) {\n"
  "    int funct g() { return v }\n"
  "    return g()\n"
  "}\n"
  "post(k4(3))\n"
  "try {\n"
  "    post(k4(Null))\n"
  "} catch (RuntimeError e) {\n"
  "    post(e)\n"
  "}\n"
  "try {\n"
  "    post(f(Null))\n"
  "} catch (RuntimeError e) {\n"
  "    post(e)\n"
  "}\n",
  "3\nnonnull: parametro 'v' em 'k4' nao pode ser Null (linha 1)\n"
  "nonnull: parametro 'v' em 'f' nao pode ser Null (linha 4)", NULL, 0 },
{ "laco sem closure que sombreia uma variavel capturada (e o parametro capturado) devolve a de fora intacta (lote 6b)",
  "list funct f() {\n"
  "    int i = 99\n"
  "    int funct h() { return i }\n"
  "    list r = []\n"
  "    for each i in [1, 2, 3] {\n"
  "        r.append(i)\n"
  "    }\n"
  "    return [r, i, h()]\n"
  "}\n"
  "list funct g(int x) {\n"
  "    int funct h() { return x }\n"
  "    list l = [x * 2 for each x in [1, 2, 3]]\n"
  "    return [l, x, h()]\n"
  "}\n"
  "post(f(), g(99))\n",
  "[[1, 2, 3], 99, 99] [[2, 4, 6], 99, 99]", NULL, 0 },
{ "variavel do laco numa funct aninhada nao grava na variavel da funct de fora (lote 6b)",
  "list funct o() {\n"
  "    int i = 99\n"
  "    int funct h() { return i }\n"
  "    list funct f() {\n"
  "        list r = []\n"
  "        for each i in [1, 2, 3] {\n"
  "            r.append(i)\n"
  "        }\n"
  "        list q = [i * 10 for each i in [1, 2]]\n"
  "        return [r, q]\n"
  "    }\n"
  "    list x = f()\n"
  "    return [x, i, h()]\n"
  "}\n"
  "post(o())\n",
  "[[[1, 2, 3], [10, 20]], 99, 99]", NULL, 0 },
{ "laco com closure sobre um nome declarado com tipo confere o tipo, como sem closure (lote 6b)",
  "list funct f() {\n"
  "    int i = 99\n"
  "    list fs = []\n"
  "    for each i in [\"a\", \"b\"] {\n"
  "        fs.append(funct() { return i })\n"
  "    }\n"
  "    return fs\n"
  "}\n"
  "post(len(f()))\n",
  "", "AttributedValueError: variável i esperava int, recebeu str", 2 },
{ "alvo do desempacotamento com o nome de uma global grava nela, com ou sem closure (lote 6b)",
  "int i = 99\n"
  "int funct h() { return i }\n"
  "list funct f() {\n"
  "    list fs = []\n"
  "    for each i, j in [[1, 10], [2, 20]] {\n"
  "        fs.append(funct() { return i })\n"
  "    }\n"
  "    list vs = []\n"
  "    for each g in fs { vs.append(g()) }\n"
  "    return [vs, i, h()]\n"
  "}\n"
  "post(f())\n",
  "[[2, 2], 2, 2]", NULL, 0 },

/* ── geradores e tarefas (Plano 6, lote 6c, 2026-10-09) ─────────────────────
 * Dois geradores que cediam em tarefas diferentes trocavam o estado (o do
 * gerador em curso morava num lugar só da VM); a tarefa criada dentro de
 * outra só começava quando o principal esperava; o erro de dentro do gerador,
 * da funct do `map` e da tarefa saía com a linha de quem consumia, e o aviso
 * da tarefa sem `await` sem linha nenhuma; o `gather` (e o `await` de lista)
 * parava na primeira tarefa que falhava, e a outra saía no fim como "sem
 * await", com rc 1 mesmo com o catch (decisão dele: espera todas e levanta a
 * primeira, com as outras na frase). */
{ "dois geradores que cedem em tarefas diferentes nao trocam dados (lote 6c)",
  "funct gen(int n) {\n"
  "    sleep(0.05)\n"
  "    yield n\n"
  "}\n"
  "async list funct t(int n) {\n"
  "    list l = list(gen(n))\n"
  "    return l\n"
  "}\n"
  "post(gather(t(1), t(2)))\n",
  "[[1], [2]]", NULL, 0 },
{ "geradores que esperam tempos diferentes em tres tarefas (lote 6c)",
  "funct gen(int n, flo s) {\n"
  "    yield n\n"
  "    sleep(s)\n"
  "    yield n * 10\n"
  "}\n"
  "async list funct t(int n, flo s) {\n"
  "    list l = []\n"
  "    for each v in gen(n, s) {\n"
  "        l.append(v)\n"
  "    }\n"
  "    return l\n"
  "}\n"
  "post(gather(t(1, 0.03), t(2, 0.01), t(3, 0.02)))\n",
  "[[1, 10], [2, 20], [3, 30]]", NULL, 0 },
{ "tarefa criada dentro de tarefa comeca na chamada (lote 6c)",
  "list ordem = []\n"
  "async str funct filho() {\n"
  "    ordem.append(\"filho comecou\")\n"
  "    sleep(0.01)\n"
  "    return \"f\"\n"
  "}\n"
  "async str funct pai() {\n"
  "    Object f = filho()\n"
  "    ordem.append(\"pai depois da chamada\")\n"
  "    return await f\n"
  "}\n"
  "Object g = filho()\n"
  "ordem.append(\"principal depois da chamada\")\n"
  "str a = await g\n"
  "str b = await pai()\n"
  "post(a, b, ordem)\n",
  "f f ['filho comecou', 'principal depois da chamada', 'filho comecou', 'pai depois da chamada']",
  NULL, 0 },
{ "tarefa neta cede e volta pra tarefa que a criou (lote 6c)",
  "list ordem = []\n"
  "async int funct neto() {\n"
  "    ordem.append(\"neto 1\")\n"
  "    sleep(0.01)\n"
  "    ordem.append(\"neto 2\")\n"
  "    return 3\n"
  "}\n"
  "async int funct filho() {\n"
  "    ordem.append(\"filho 1\")\n"
  "    Object n = neto()\n"
  "    ordem.append(\"filho 2\")\n"
  "    int r = await n\n"
  "    ordem.append(\"filho 3\")\n"
  "    return r\n"
  "}\n"
  "int r = await filho()\n"
  "post(r, ordem)\n",
  "3 ['filho 1', 'neto 1', 'filho 2', 'neto 2', 'filho 3']", NULL, 0 },
{ "erro antes de ceder na tarefa criada dentro de tarefa sobe na chamada (lote 6c)",
  "async str funct filho() {\n"
  "    raise Cedo(\"antes de ceder\")\n"
  "}\n"
  "async str funct pai() {\n"
  "    try {\n"
  "        Object f = filho()\n"
  "    } catch (Cedo e) {\n"
  "        return \"pegou na chamada: \" + e\n"
  "    }\n"
  "    return \"a chamada nao levantou\"\n"
  "}\n"
  "post(await pai())\n",
  "pegou na chamada: antes de ceder (linha 2)", NULL, 0 },
{ "sys.exit na tarefa criada dentro de tarefa roda o finally do pai e sai (lote 6c)",
  "import sys\n"
  "async Null funct filho() {\n"
  "    sys.exit(3)\n"
  "}\n"
  "async Null funct pai() {\n"
  "    try {\n"
  "        Object f = filho()\n"
  "        post(\"nao chega\")\n"
  "    } finally {\n"
  "        post(\"finally do pai\")\n"
  "    }\n"
  "}\n"
  "Object g = pai()\n"
  "post(\"nao chega no principal\")\n",
  "finally do pai", NULL, 3 },
{ "erro dentro de gerador: o traceback termina na linha do raise (lote 6c)",
  "flo funct ajuda(int x) {\n"
  "    return 10 / x\n"
  "}\n"
  "funct gen(list l) {\n"
  "    for each x in l {\n"
  "        yield ajuda(x)\n"
  "    }\n"
  "}\n"
  "for each v in gen([5, 0]) {\n"
  "    post(v)\n"
  "}\n",
  "2.0", ", linha 2\n  |     return 10 / x", 1 },
{ "erro dentro de gerador: o e do catch diz a linha do raise (lote 6c)",
  "funct gen(list l) {\n"
  "    for each x in l {\n"
  "        yield 10 / x\n"
  "    }\n"
  "}\n"
  "try {\n"
  "    list r = list(gen([5, 0]))\n"
  "} catch (ZeroDivisionError e) {\n"
  "    post(e)\n"
  "}\n",
  "division by zero (linha 3)", NULL, 0 },
{ "erro na funct chamada pelo map: o traceback termina na linha do raise (lote 6c)",
  "flo funct ajuda(int x) {\n"
  "    return 10 / x\n"
  "}\n"
  "list l = map([5, 0], ajuda)\n"
  "post(l)\n",
  "", ", linha 2\n  |     return 10 / x", 1 },
{ "erro na funct chamada pelo map: o e do catch diz a linha do raise (lote 6c)",
  "flo funct ajuda(int x) {\n"
  "    return 10 / x\n"
  "}\n"
  "try {\n"
  "    list l = map([5, 0], ajuda)\n"
  "} catch (ZeroDivisionError e) {\n"
  "    post(e)\n"
  "}\n",
  "division by zero (linha 2)", NULL, 0 },
{ "tarefa sem await: o aviso traz a linha do raise e o programa sai 1 (lote 6c)",
  "async str funct t() {\n"
  "    sleep(0.01)\n"
  "    raise Quebrou(\"sem await\")\n"
  "}\n"
  "Object f = t()\n"
  "sleep(0.05)\n"
  "post(\"fim do principal\")\n",
  "fim do principal",
  ", linha 3\n  |     raise Quebrou(\"sem await\")\n  |     ^^^\n"
  "  em tarefa async que ninguem aguardou (sem `await`)", 1 },
{ "erro de tarefa: o e do catch diz a linha do raise, no principal e no pai (lote 6c)",
  "async str funct t(str s) {\n"
  "    sleep(0.01)\n"
  "    raise Quebrou(s)\n"
  "}\n"
  "async str funct pai() {\n"
  "    try {\n"
  "        str r = await t(\"no filho\")\n"
  "    } catch (Quebrou e) {\n"
  "        return e\n"
  "    }\n"
  "    return \"\"\n"
  "}\n"
  "try {\n"
  "    str r = await t(\"depois do sleep\")\n"
  "} catch (Quebrou e) {\n"
  "    post(e)\n"
  "}\n"
  "post(await pai())\n",
  "depois do sleep (linha 3)\nno filho (linha 3)", NULL, 0 },
{ "erro de tarefa antes de ceder: o e do catch da chamada diz a linha do raise (lote 6c)",
  "async str funct t() {\n"
  "    raise Quebrou(\"na chamada\")\n"
  "}\n"
  "try {\n"
  "    Object f = t()\n"
  "} catch (Quebrou e) {\n"
  "    post(e)\n"
  "}\n",
  "na chamada (linha 2)", NULL, 0 },
{ "gather com duas tarefas que falham: espera as duas e levanta a primeira com a outra (lote 6c)",
  "async str funct t(int n) {\n"
  "    sleep(0.01 * n)\n"
  "    raise Quebrou(\"tarefa \" + str(n))\n"
  "}\n"
  "try {\n"
  "    post(gather(t(1), t(2)))\n"
  "} catch (Quebrou e) {\n"
  "    post(\"pegou:\", e)\n"
  "}\n"
  "try {\n"
  "    post(gather(t(3), t(1)))\n"
  "} catch (Quebrou e) {\n"
  "    post(\"pegou:\", e)\n"
  "}\n"
  "sleep(0.05)\n"
  "post(\"fim\")\n",
  "pegou: tarefa 1 (linha 3); mais 1 tarefa do gather falhou: Quebrou: tarefa 2 (linha 3)\n"
  "pegou: tarefa 3 (linha 3); mais 1 tarefa do gather falhou: Quebrou: tarefa 1 (linha 3)\n"
  "fim", NULL, 0 },
{ "gather de lista com tres falhas em quatro: a frase leva as outras duas (lote 6c)",
  "async str funct t(int n) {\n"
  "    sleep(0.01 * n)\n"
  "    if n == 2 {\n"
  "        return \"ok\"\n"
  "    }\n"
  "    raise Quebrou(\"tarefa \" + str(n))\n"
  "}\n"
  "try {\n"
  "    post(gather([t(1), t(2), t(3), t(4)]))\n"
  "} catch (Quebrou e) {\n"
  "    post(\"pegou:\", e)\n"
  "}\n"
  "post(\"fim\")\n",
  "pegou: tarefa 1 (linha 6); mais 2 tarefas do gather falharam: Quebrou: tarefa 3 (linha 6); "
  "Quebrou: tarefa 4 (linha 6)\nfim", NULL, 0 },
{ "gather sem catch: o erro sai com a outra tarefa na frase (lote 6c)",
  "async str funct t(int n) {\n"
  "    sleep(0.01 * n)\n"
  "    raise Quebrou(\"tarefa \" + str(n))\n"
  "}\n"
  "post(gather(t(1), t(2)))\n",
  "", "Quebrou: tarefa 1 (linha 3); mais 1 tarefa do gather falhou: Quebrou: tarefa 2 (linha 3)", 1 },
{ "gather e await de lista com uma falha so: o erro dela, como no await da tarefa (lote 6c)",
  "async str funct t(int n) {\n"
  "    sleep(0.01 * n)\n"
  "    raise Quebrou(\"tarefa \" + str(n))\n"
  "}\n"
  "async str funct ok(int n) {\n"
  "    sleep(0.01 * n)\n"
  "    return str(n)\n"
  "}\n"
  "try {\n"
  "    post(gather(t(1), ok(2)))\n"
  "} catch (Quebrou e) {\n"
  "    post(\"pegou:\", e)\n"
  "}\n"
  "try {\n"
  "    list r = await [ok(1), t(2)]\n"
  "} catch (Quebrou e) {\n"
  "    post(\"await:\", e)\n"
  "}\n"
  "post(gather(ok(1), t(2)))\n",
  "pegou: tarefa 1 (linha 3)\nawait: tarefa 2 (linha 3)", "Quebrou: tarefa 2\n", 1 },
{ "a mesma tarefa duas vezes no gather e no await de lista conta uma vez (lote 6c)",
  "async str funct t(int n) {\n"
  "    sleep(0.01 * n)\n"
  "    raise Quebrou(\"tarefa \" + str(n))\n"
  "}\n"
  "Object f = t(1)\n"
  "try {\n"
  "    post(gather(f, f, t(2)))\n"
  "} catch (Quebrou e) {\n"
  "    post(\"pegou:\", e)\n"
  "}\n"
  "try {\n"
  "    list r = await [f, f]\n"
  "} catch (Quebrou e) {\n"
  "    post(\"de novo:\", e)\n"
  "}\n",
  "pegou: tarefa 1 (linha 3); mais 1 tarefa do gather falhou: Quebrou: tarefa 2 (linha 3)\n"
  "de novo: tarefa 1 (linha 3)", NULL, 0 },
{ "await de lista e de tupla e o gather delas: espera todas, e a frase diz do await (lote 6c)",
  "async str funct t(int n) {\n"
  "    sleep(0.01 * n)\n"
  "    raise Quebrou(\"tarefa \" + str(n))\n"
  "}\n"
  "async str funct ok(int n) {\n"
  "    sleep(0.01 * n)\n"
  "    return str(n)\n"
  "}\n"
  "async str funct pai() {\n"
  "    try {\n"
  "        list r = await [t(1), t(2)]\n"
  "    } catch (Quebrou e) {\n"
  "        return \"pai pegou: \" + e\n"
  "    }\n"
  "    return \"\"\n"
  "}\n"
  "try {\n"
  "    list r = await [t(2), t(1)]\n"
  "} catch (Quebrou e) {\n"
  "    post(\"pegou:\", e)\n"
  "}\n"
  "post(await pai())\n"
  "post(await [ok(2), \"solto\", ok(1)])\n"
  "post(await (ok(2), ok(1)))\n"
  "try {\n"
  "    tup s = await (t(1), ok(1), t(1))\n"
  "} catch (Quebrou e) {\n"
  "    post(\"tupla:\", e)\n"
  "}\n"
  "sleep(0.05)\n"
  "post(\"fim\")\n",
  "pegou: tarefa 2 (linha 3); mais 1 tarefa do await falhou: Quebrou: tarefa 1 (linha 3)\n"
  "pai pegou: tarefa 1 (linha 3); mais 1 tarefa do await falhou: Quebrou: tarefa 2 (linha 3)\n"
  "['2', 'solto', '1']\n"
  "('2', '1')\n"
  "tupla: tarefa 1 (linha 3); mais 1 tarefa do await falhou: Quebrou: tarefa 1 (linha 3)\n"
  "fim", NULL, 0 },
{ "await de lista sem catch: o erro sai com a outra tarefa na frase (lote 6c)",
  "async str funct t(int n) {\n"
  "    sleep(0.01 * n)\n"
  "    raise Quebrou(\"tarefa \" + str(n))\n"
  "}\n"
  "list r = await [t(1), t(2)]\n"
  "post(r)\n",
  "", "Quebrou: tarefa 1 (linha 3); mais 1 tarefa do await falhou: Quebrou: tarefa 2 (linha 3)", 1 },

/* ── gerador abandonado, saídas de try e using (Plano 6, lote 6d, 2026-10-09)
 * O gerador que o `for each` criava e abandonava (break, return, erro) não
 * rodava o finally nem fechava o using (decisão dele: o laço dono fecha na
 * hora; o guardado em variável segue consumível; o resto fecha no fim do
 * programa). Achados no caminho e consertados junto: `break`/`continue` que
 * saíam de um `try` deixavam o handler armado (o catch velho pegava um erro de
 * fora; 70 mil `continue` davam RecursionError) e o finally que levantava na
 * saída rodava duas vezes; `return`/`break`/`continue` dentro de `using`
 * saíam sem fechar; o erro que atravessava um `using` perdia o tipo (e o
 * `sys.exit` virava RuntimeError); o erro que atravessava um `finally` ou um
 * `using` perdia os quadros de onde foi levantado. */
{ "laco dono fecha o gerador no break: o finally dele roda (lote 6d)",
  "funct gen() {\n"
  "    try {\n"
  "        yield 1\n"
  "        yield 2\n"
  "        yield 3\n"
  "    } finally {\n"
  "        post(\"finally do gerador\")\n"
  "    }\n"
  "}\n"
  "for each v in gen() {\n"
  "    post(\"v\", v)\n"
  "    if v == 2 {\n"
  "        break\n"
  "    }\n"
  "}\n"
  "post(\"depois do laco\")\n",
  "v 1\nv 2\nfinally do gerador\ndepois do laco", NULL, 0 },
{ "laco dono fecha no return e no erro; em variavel segue; o resto fecha no fim do programa (lote 6d)",
  "import os\n"
  "funct gen(str nome) {\n"
  "    try {\n"
  "        yield 1\n"
  "        yield 2\n"
  "    } finally {\n"
  "        post(\"finally\", nome)\n"
  "    }\n"
  "}\n"
  "funct gen_u(str cam) {\n"
  "    using open(cam, \"w\") as h {\n"
  "        h.write(\"dado\")\n"
  "        yield 1\n"
  "        yield 2\n"
  "    }\n"
  "}\n"
  "int funct primeiro() {\n"
  "    for each v in gen(\"return\") {\n"
  "        return v\n"
  "    }\n"
  "    return 0\n"
  "}\n"
  "post(primeiro())\n"
  "try {\n"
  "    for each v in gen(\"erro\") {\n"
  "        raise Quebrou(\"no corpo\")\n"
  "    }\n"
  "} catch (Quebrou e) {\n"
  "    post(\"pegou:\", e)\n"
  "}\n"
  "for each v in gen_u(\"ps_6d_gen_u.txt\") {\n"
  "    break\n"
  "}\n"
  "post(\"using fechou:\", len(os.readFile(\"ps_6d_gen_u.txt\")))\n"
  "Object g = gen(\"variavel\")\n"
  "for each v in g {\n"
  "    break\n"
  "}\n"
  "post(\"variavel segue:\", list(g))\n"
  "Object g2 = gen(\"fim do programa\")\n"
  "for each v in g2 {\n"
  "    break\n"
  "}\n"
  "post(\"antes do fim\")\n",
  "finally return\n1\nfinally erro\npegou: no corpo (linha 26)\nusing fechou: 4\n"
  "finally variavel\nvariavel segue: [2]\nantes do fim\nfinally fim do programa", NULL, 0 },
{ "fecho do gerador: yield no finally e RuntimeError, catch nao pega, sys.exit sai com o codigo (lote 6d)",
  "import sys\n"
  "funct teimoso() {\n"
  "    try {\n"
  "        yield 1\n"
  "    } finally {\n"
  "        yield 2\n"
  "    }\n"
  "}\n"
  "try {\n"
  "    for each v in teimoso() {\n"
  "        break\n"
  "    }\n"
  "} catch (RuntimeError e) {\n"
  "    post(\"pegou:\", e)\n"
  "}\n"
  "funct pega_tudo() {\n"
  "    try {\n"
  "        yield 1\n"
  "    } catch (e) {\n"
  "        post(\"catch nao pode pegar o fecho\")\n"
  "    } finally {\n"
  "        post(\"finally roda\")\n"
  "    }\n"
  "}\n"
  "for each v in pega_tudo() {\n"
  "    break\n"
  "}\n"
  "funct sai() {\n"
  "    try {\n"
  "        yield 1\n"
  "    } finally {\n"
  "        sys.exit(4)\n"
  "    }\n"
  "}\n"
  "for each v in sai() {\n"
  "    break\n"
  "}\n"
  "post(\"nao chega\")\n",
  "pegou: o gerador fez yield enquanto era fechado (yield dentro do finally) (linha 11)\nfinally roda",
  NULL, 4 },
{ "erro dentro de gerador com finally: o traceback termina na linha do raise (lote 6d)",
  "flo funct ajuda(int x) {\n"
  "    return 10 / x\n"
  "}\n"
  "funct gen(list l) {\n"
  "    try {\n"
  "        for each x in l {\n"
  "            yield ajuda(x)\n"
  "        }\n"
  "    } finally {\n"
  "        post(\"finally do gen\")\n"
  "    }\n"
  "}\n"
  "for each v in gen([5, 0]) {\n"
  "    post(v)\n"
  "}\n",
  "2.0\nfinally do gen", ", linha 2\n  |     return 10 / x", 1 },
{ "erro que atravessa um finally: o traceback termina na linha do raise (lote 6d)",
  "flo funct ajuda(int x) {\n"
  "    return 10 / x\n"
  "}\n"
  "list lg = []\n"
  "flo funct f(int x) {\n"
  "    try {\n"
  "        return ajuda(x)\n"
  "    } finally {\n"
  "        lg.append(\"finally de f\")\n"
  "    }\n"
  "}\n"
  "post(f(0))\n",
  "", ", linha 7\n  |         return ajuda(x)", 1 },
{ "erro que atravessa um using: o quadro do corpo e o do raise (lote 6d)",
  "flo funct ajuda(int x) {\n"
  "    return 10 / x\n"
  "}\n"
  "using open(\"ps_6d_g06.txt\", \"w\") as h {\n"
  "    flo z = ajuda(0)\n"
  "}\n",
  "", ", linha 5\n  |     flo z = ajuda(0)", 1 },
{ "break que sai de try leva o handler junto: o raise depois do laco nao cai no finally velho (lote 6d)",
  "for each i in [1, 2] {\n"
  "    try {\n"
  "        break\n"
  "    } finally {\n"
  "        post(\"finally\", i)\n"
  "    }\n"
  "}\n"
  "try {\n"
  "    raise Boom(\"depois do laco\")\n"
  "} catch (Boom e) {\n"
  "    post(\"pegou:\", e)\n"
  "}\n"
  "int n = 0\n"
  "for each j in range(100000) {\n"
  "    try {\n"
  "        n += 1\n"
  "        continue\n"
  "    } finally {\n"
  "        n += 0\n"
  "    }\n"
  "}\n"
  "post(\"voltas:\", n)\n"
  "raise Fim(\"fora de tudo\")\n",
  "finally 1\npegou: depois do laco (linha 9)\nvoltas: 100000", "Fim: fora de tudo\n", 1 },
{ "continue dentro de try/finally num laco de 70 mil voltas (lote 6d)",
  "int n = 0\n"
  "for each j in range(70000) {\n"
  "    try {\n"
  "        n += 1\n"
  "        continue\n"
  "    } finally {\n"
  "        n += 0\n"
  "    }\n"
  "}\n"
  "post(\"voltas:\", n)\n",
  "voltas: 70000", NULL, 0 },
{ "break que sai de try/catch: o raise de fora nao e pego pelo catch velho (lote 6d)",
  "for each i in [1, 2] {\n"
  "    try {\n"
  "        break\n"
  "    } catch (Boom e) {\n"
  "        post(\"catch velho\", e)\n"
  "    }\n"
  "}\n"
  "raise Boom(\"fora de tudo\")\n",
  "", "Boom: fora de tudo\n", 1 },
{ "finally que levanta na saida por return ou break roda uma vez (lote 6d)",
  "str funct f() {\n"
  "    try {\n"
  "        return \"x\"\n"
  "    } finally {\n"
  "        post(\"finally roda\")\n"
  "        raise Boom(\"no finally\")\n"
  "    }\n"
  "}\n"
  "try {\n"
  "    post(f())\n"
  "} catch (Boom e) {\n"
  "    post(\"pegou:\", e)\n"
  "}\n"
  "for each i in [1] {\n"
  "    try {\n"
  "        try {\n"
  "            break\n"
  "        } finally {\n"
  "            post(\"finally do break\")\n"
  "            raise Boom(\"no finally do break\")\n"
  "        }\n"
  "    } catch (Boom e) {\n"
  "        post(\"pegou:\", e)\n"
  "    }\n"
  "}\n",
  "finally roda\npegou: no finally (linha 6)\nfinally do break\npegou: no finally do break (linha 20)",
  NULL, 0 },
{ "using fecha no return, no break e no continue (lote 6d)",
  "import os\n"
  "str CAM = \"ps_6d_using.txt\"\n"
  "Null funct grava() {\n"
  "    using open(CAM, \"w\") as h {\n"
  "        h.write(\"dado\")\n"
  "        return\n"
  "    }\n"
  "}\n"
  "grava()\n"
  "post(\"depois do return:\", len(os.readFile(CAM)))\n"
  "for each i in [1, 2] {\n"
  "    using open(CAM, \"w\") as h2 {\n"
  "        h2.write(\"outro\")\n"
  "        break\n"
  "    }\n"
  "}\n"
  "post(\"depois do break:\", len(os.readFile(CAM)))\n"
  "for each i in [1] {\n"
  "    using open(CAM, \"w\") as h3 {\n"
  "        h3.write(\"mais\")\n"
  "        continue\n"
  "    }\n"
  "}\n"
  "post(\"depois do continue:\", len(os.readFile(CAM)))\n",
  "depois do return: 4\ndepois do break: 5\ndepois do continue: 4", NULL, 0 },
{ "erro que atravessa um using mantem o tipo (lote 6d)",
  "try {\n"
  "    using open(\"ps_6d_tipo.txt\", \"w\") as h {\n"
  "        flo z = 1 / 0\n"
  "    }\n"
  "} catch (ZeroDivisionError e) {\n"
  "    post(\"pegou ZeroDivisionError:\", e)\n"
  "} catch (e) {\n"
  "    post(\"outro tipo:\", e)\n"
  "}\n"
  "try {\n"
  "    using open(\"ps_6d_tipo.txt\", \"w\") as h2 {\n"
  "        raise Quebrou(\"dentro do using\")\n"
  "    }\n"
  "} catch (Quebrou e) {\n"
  "    post(\"pegou Quebrou:\", e)\n"
  "} catch (e) {\n"
  "    post(\"outro tipo:\", e)\n"
  "}\n",
  "pegou ZeroDivisionError: division by zero (linha 3)\npegou Quebrou: dentro do using (linha 12)", NULL, 0 },
{ "sys.exit dentro de using fecha o recurso e sai com o codigo (lote 6d)",
  "import sys\n"
  "using open(\"ps_6d_exit.txt\", \"w\") as h {\n"
  "    h.write(\"x\")\n"
  "    sys.exit(3)\n"
  "}\n"
  "post(\"nao chega\")\n",
  "", NULL, 3 },

/* ── texto e Unicode nos métodos nativos (Plano 6, lote 7a, 2026-10-13) ─────
 * format: o `0` entrava antes do sinal ("00-42"), x/o davam o complemento de
 * dois, a precisão cortava BYTE (UTF-8 inválido), número sem alinhamento ia
 * pra esquerda e o inteiro grande era recusado. replace com listas trocava em
 * cadeia ("ab" virava "aa"), ignorava o count e aceitava ficar sem o new.
 * rsplit testava o branco byte a byte (cortava "à" e "你") e parava calado em
 * 256 cortes. Caixa do Latin Estendido-A trocada ("łódź"), sigma final só no
 * lower, brancos e imprimíveis pela régua errada, translate e chr sem a faixa
 * do Unicode, count de char pulando o espaço pelo primeiro byte. */
{ "format: zero depois do sinal, hexa com sinal, precisao em caractere, numero a direita, inteiro grande (lote 7a)",
  "post(\"[{:05d}]\".format(-42), \"[{:08.2f}]\".format(-3.14159))\n"
  "post(\"[{:x}]\".format(-255), \"[{:o}]\".format(-8), hex(-255), oct(-8))\n"
  "post(\"[{:.2}]\".format(\"漢字かな\"), \"[{:.3}]\".format(\"açúcar\"))\n"
  "post(\"[{:5}]\".format(42), \"[{:>5}]\".format(42))\n"
  "post(\"[{:d}]\".format(2 ** 70))\n",
  "[-0042] [-0003.14]\n[-ff] [-10] -0xff -0o10\n[漢字] [açú]\n[   42] [   42]\n"
  "[1180591620717411303424]", NULL, 0 },
{ "format: preenchimento, alinhamento e o 0 com e sem alinhamento escrito (lote 7a)",
  "post(\"[{:>05d}]\".format(-42), \"[{:<05d}]\".format(-42), \"[{:*>05d}]\".format(-42), \"[{:^07d}]\".format(-42))\n"
  "post(\"[{:X}]\".format(-255), \"[{:b}]\".format(-5), \"[{:06x}]\".format(-255), \"[{:b}]\".format(-9223372036854775807 - 1))\n"
  "post(\"[{:5}]\".format(\"ab\"), \"[{:5}]\".format(1.5), \"[{:5}]\".format(True), \"[{:05}]\".format(\"ab\"), \"[{:08}]\".format(-3.5))\n"
  "post(\"[{:x}]\".format(2 ** 70), \"[{:X}]\".format(-(2 ** 70) - 11), \"[{:030d}]\".format(-(2 ** 70)), \"[{:f}]\".format(2 ** 70))\n"
  "post(\"[{:.1f}]\".format(True), \"[{:d}]\".format(False), \"[{:·>6}]\".format(\"ab\"), \"[{:漢^5}]\".format(\"x\"), \"[{:.0}]\".format(\"abc\"))\n",
  "[00-42] [-4200] [**-42] [00-4200]\n"
  "[-FF] [-101] [-000ff] [-1000000000000000000000000000000000000000000000000000000000000000]\n"
  "[ab   ] [  1.5] [True ] [000ab] [-00003.5]\n"
  "[400000000000000000] [-40000000000000000B] [-00000001180591620717411303424] [1180591620717411303424.000000]\n"
  "[1.0] [0] [····ab] [漢漢x漢漢] []", NULL, 0 },
{ "format: especificador fora da forma e tipo que nao serve sao erro, nunca ignorados (lote 7a)",
  "list ruins = [\"{:.2d}\", \"{:.2}\", \"{:+d}\", \"{:e}\", \"{:,}\", \"{:=5d}\"]\n"
  "list vals = [5, 3.14159, 5, 1.5, 1000, 5]\n"
  "for each i in range(len(ruins)) {\n"
  "    try {\n"
  "        post(ruins[i].format(vals[i]))\n"
  "    } catch (ValueError e) {\n"
  "        post(e)\n"
  "    }\n"
  "}\n"
  "list tipos = [\"{:d}\", \"{:f}\", \"{:x}\"]\n"
  "for each t in tipos {\n"
  "    try {\n"
  "        post(t.format(\"a\"))\n"
  "    } catch (TypeError e) {\n"
  "        post(e)\n"
  "    }\n"
  "}\n",
  "format: precisao nao vale pra inteiro ('d'); pra casas decimais use o tipo 'f' ({:.2f}) (linha 5)\n"
  "format: precisao num flo pede o tipo 'f' ({:.2f}) (linha 5)\n"
  "format: especificador '+d' invalido; a forma e [preenche][<>^][0][largura][.precisao][tipo] (linha 5)\n"
  "format: tipo 'e' nao existe; os tipos sao s d f x X o b (linha 5)\n"
  "format: tipo ',' nao existe; os tipos sao s d f x X o b (linha 5)\n"
  "format: especificador '=5d' invalido; a forma e [preenche][<>^][0][largura][.precisao][tipo] (linha 5)\n"
  "format: 'd' espera int, recebeu str (linha 13)\n"
  "format: 'f' espera numero, recebeu str (linha 13)\n"
  "format: 'x' espera int, recebeu str (linha 13)", NULL, 0 },
{ "replace com listas: em paralelo, count somado, vale o primeiro alvo da lista (lote 7a)",
  "post(\"ab\".replace([\"a\", \"b\"], [\"b\", \"a\"]))\n"
  "post(\"gato e rato\".replace([\"gato\", \"rato\"], [\"rato\", \"gato\"]))\n"
  "post(\"a-b-c_d\".replace([\"-\", \"_\"], \"+\", 1))\n"
  "post(\"a\".replace([\"a\", \"y\"], \"xy\"))\n"
  "post(\"abab\".replace([\"a\", \"ab\"], [\"1\", \"2\"]), \"abab\".replace([\"ab\", \"a\"], [\"1\", \"2\"]), \"a-b\".replace([\"-\"], [\"+\"], 0))\n",
  "ba\nrato e gato\na+b-c_d\nxy\n1b1b 11 a-b", NULL, 0 },
{ "replace com listas: tamanhos, alvo vazio, item que nao e str e o new obrigatorio (lote 7a)",
  "list vruins = [[\"a\", \"b\"], [\"\"]]\n"
  "for each c in vruins {\n"
  "    try {\n"
  "        post(\"x-a\".replace(c, [\"z\"]))\n"
  "    } catch (ValueError e) {\n"
  "        post(e)\n"
  "    }\n"
  "}\n"
  "try {\n"
  "    post(\"x\".replace([1], \"y\"))\n"
  "} catch (TypeError e) {\n"
  "    post(e)\n"
  "}\n"
  "try {\n"
  "    post(\"x\".replace([\"x\"], [1]))\n"
  "} catch (TypeError e) {\n"
  "    post(e)\n"
  "}\n"
  "try {\n"
  "    post(\"x\".replace([\"x\"], 5))\n"
  "} catch (TypeError e) {\n"
  "    post(e)\n"
  "}\n"
  "Object rep = \"banana\".replace; post(rep(\"a\"))\n",
  "replace(): 2 alvo(s) e 1 novo(s); em paralelo, as duas listas tem o mesmo tamanho (linha 4)\n"
  "replace(): o item 0 da lista de alvos e vazio; o alvo vazio vale sozinho: replace(\"\", novo) (linha 4)\n"
  "replace() argument 1 item 0 must be str, not int (linha 10)\n"
  "replace() argument 2 item 0 must be str, not int (linha 15)\n"
  "replace() argument 2 must be str or list, not int (linha 20)",
  "TypeError: replace() takes at least 2 arguments (1 given)", 1 },
{ "rsplit pelo codepoint, sem teto de cortes, separador procurado da direita (lote 7a)",
  "str a = \"voilà\"\n"
  "list ra = a.rsplit(Null, 1)\n"
  "post(ra, ra[0] == \"voilà\", a.split())\n"
  "str b = \"你好 你\"\n"
  "list rb = b.rsplit(Null, 1)\n"
  "post(rb, rb[1] == \"你\", b.split())\n"
  "str s = \"x,\" * 300\n"
  "post(len(s.rsplit(\",\", 1000)), len(s.split(\",\", 1000)), s.rsplit(\",\", 1000)[0].count(\",\"))\n"
  "str w = \"x \" * 300\n"
  "post(len(w.rsplit(Null, 1000)), len(w.split()))\n"
  "post(\"aaa\".rsplit(\"aa\"), \"aaa\".rsplit(\"aa\", 5), b\"aaa\".rsplit(b\"aa\"), \"aaa\".split(\"aa\"))\n"
  "post(\"  a b c  \".rsplit(Null, 1))\n",
  "['voilà'] True ['voilà']\n['你好', '你'] True ['你好', '你']\n301 301 0\n300 300\n"
  "['a', ''] ['a', ''] [b'a', b''] ['', 'a']\n['  a b', 'c']", NULL, 0 },
{ "caixa do Latin Estendido-A pelo par certo de cada letra (lote 7a)",
  "str cidade = \"łódź\"\n"
  "str maiu = \"ŁÓDŹ\"\n"
  "str cavalo = \"kůň\"\n"
  "post(cidade.lower(), cidade.upper(), maiu.upper(), maiu.lower(), cavalo.lower(), cavalo.upper())\n"
  "post(\"ı\".upper(), \"ĸ\".isupper(), \"Ĺ\".upper(), \"ł\".lower(), maiu.lower() == cidade)\n"
  "post(\"İ\".lower(), len(\"İ\".lower()), \"ŉ\".upper(), \"ſ\".upper(), \"Ÿ\".lower(), \"ĺ\".upper(), \"Ž\".lower())\n",
  "łódź ŁÓDŹ ŁÓDŹ łódź kůň KŮŇ\nI False Ĺ ł True\ni\xcc\x87 2 ʼN S ÿ Ĺ ž", NULL, 0 },
{ "caixa e classes de caractere de qualquer lingua, pela tabela do Unicode (lote 7a)",
  "post(\"ș\".upper(), \"ạ\".upper(), \"ϊ\".upper(), \"ａ\".upper(), \"Ա\".lower(), \"ა\".upper(), \"Ꭰ\".lower(), \"ɐ\".upper())\n"
  "post(\"ﬁ\".upper(), \"ǰ\".upper(), \"ᾳ\".upper(), \"ǆ\".title(), \"ǅ\".lower(), \"Ⅷ\".lower(), \"ⓐ\".upper(), \"ꭰ\".casefold())\n"
  "post(\"漢\".isalpha(), \"ǈ\".istitle(), \"ǈ\".isupper(), \"ǈ\".islower(), \"Ⅷ\".isupper(), \"ª\".islower())\n"
  "post(\"一\".isnumeric(), \"一\".isdigit(), \"٣\".isdecimal(), \"²\".isdigit(), \"²\".isdecimal(), \"½\".isnumeric(), \"漢٣\".isalnum())\n"
  "post(\"漢a\".title(), \"ﬁx\".title(), \"ßa\".capitalize(), \"ǆemal\".capitalize(), \"Straße\".swapcase(), \"ΑΣ'\".lower(), \"ΑΣ'Β\".lower())\n"
  "post(\"\\u0378\".isprintable(), \"\\U00020BB7\".isprintable(), \"\\u00ad\".isprintable(), \"\\u2028\".isspace())\n",
  "Ș Ạ Ϊ Ａ ա Ა ꭰ Ɐ\n"
  "FI J\xcc\x8c ΑΙ ǅ ǆ ⅷ Ⓐ Ꭰ\n"
  "True True False False True True\n"
  "True False True True False True True\n"
  "漢A Fix Ssa ǅemal sTRASSE ας' ασ'β\n"
  "False True False True", NULL, 0 },
{ "sigma final em lower, title, capitalize e swapcase; o casefold da sigma (lote 7a)",
  "str w = \"ΟΔΥΣΣΕΥΣ\"\n"
  "post(w.lower(), w.title(), w.capitalize(), w.swapcase(), w.casefold())\n"
  "post(\"ΣΑΣ ΣΑΣ\".lower(), \"Σ\".lower(), \"ΑΣ.\".lower())\n",
  "οδυσσευς Οδυσσευς Οδυσσευς οδυσσευς οδυσσευσ\nσας σας σ ας.", NULL, 0 },
{ "translate e chr na faixa do Unicode, sem substituto (lote 7a)",
  "list destinos = [-1, 1114112, 55296]\n"
  "for each d in destinos {\n"
  "    try {\n"
  "        post(\"abc\".translate({97: d}))\n"
  "    } catch (ValueError e) {\n"
  "        post(e)\n"
  "    }\n"
  "}\n"
  "post(\"abc\".translate({97: 1114111}).encode(), \"abc\".translate({97: Null}))\n"
  "try {\n"
  "    post(chr(56319))\n"
  "} catch (ValueError e) {\n"
  "    post(e)\n"
  "}\n"
  "post(chr(55295).encode(), chr(57344).encode())\n",
  "character mapping must be in range(0x110000) (linha 4)\n"
  "character mapping must be in range(0x110000) (linha 4)\n"
  "character mapping is a surrogate (0xd800..0xdfff), not a character (linha 4)\n"
  "b'\\xf4\\x8f\\xbf\\xbfbc' bc\n"
  "chr() arg is a surrogate (0xd800..0xdfff), not a character (linha 11)\n"
  "b'\\xed\\x9f\\xbf' b'\\xee\\x80\\x80'", NULL, 0 },
{ "count de char: um caractere qualquer, o espaco e o TAB inclusive (lote 7a)",
  "str frase = \"a b\\tc d\"\n"
  "post(char in frase count, len(frase))\n"
  "list vistos = []\n"
  "count each char in frase {\n"
  "    vistos.append(_match)\n"
  "}\n"
  "post(vistos)\n"
  "post(char in [\"a\", \" \", \"ab\", \"\\u00a0\"] count)\n",
  "7 7\n['a', ' ', 'b', '\\t', 'c', ' ', 'd']\n3", NULL, 0 },
{ "isprintable pela regra do Unicode: controle, branco, formatacao invisivel (lote 7a)",
  "post(\"\\x85\".isprintable(), \"\\x9b\".isprintable(), \"\\u2003\".isprintable(), \"\\u200b\".isprintable(), \"\\ufeff\".isprintable(), \"\\x7f\".isprintable())\n"
  "post(\"a b\".isprintable(), \"ç漢\".isprintable(), \"\".isprintable(), \"\\ue000\".isprintable(), \"\\u00ad\".isprintable())\n",
  "False False False False False False\nTrue True True False False", NULL, 0 },
{ "brancos do Unicode no isspace, no split e no strip (lote 7a)",
  "post(\"\\u3000\".isspace(), \"\\u00a0\".isspace(), (\"漢\" + \"\\u3000\" + \"字\").split(), \"漢\\u00a0字\".split(), len(\"\\u3000x\\u3000\".strip()))\n"
  "post(\"\\u1680\\u2000\\u200a\\u202f\\u205f\".isspace(), \"\\u200b\".isspace(), \"x\\u2003y\".split(), \"\".isspace())\n",
  "True True ['漢', '字'] ['漢', '字'] 1\nTrue False ['x', 'y'] False", NULL, 0 },

/* ── números e coleções nos nativos (Plano 6, lote 7b, 2026-10-13) ──────────
 * Um leitor de número só (int, flo, json.parse), sem o buffer de 512 que
 * cortava 600 dígitos calado; int x flo exato no ==, na ordem e na chave de
 * dict; o inteiro grande que não cabe num flo é OverflowError (era inf);
 * igualdade funda medida pela pilha (o teto de 256 dizia "diferente");
 * list x tup não se ordena; bool e inteiro grande em todo argumento inteiro,
 * uma regra só; bytes.fromint/toint/get/slice/new com inteiro grande e sem o
 * `(int)` que cortava; round com casas negativas; date.hora só inteiro. */
{ "int() e flo() pelo leitor unico: sinal, 600 digitos, NUL, digito de outra escrita e byte (lote 7b)",
  "Null funct tenta(str rot, Object f) {\n"
  "    try {\n"
  "        post(rot, f())\n"
  "    } catch (Exception e) {\n"
  "        post(rot, \"ERRO\", e)\n"
  "    }\n"
  "}\n"
  "tenta(\"a\", funct() { return int(\"99999999999999999999 1\") })\n"
  "tenta(\"b\", funct() { return int(\"+99999999999999999999\") })\n"
  "tenta(\"c\", funct() { return str(int(\"9\" * 600)) == \"9\" * 600 })\n"
  "tenta(\"d\", funct() { return int(\"15\\x00\") })\n"
  "tenta(\"e\", funct() { return flo(\"1.5\\x00abc\") })\n"
  "tenta(\"f\", funct() { return flo(\"0x1A\") })\n"
  "tenta(\"g\", funct() { return [int(\"٣٤\"), int(\"４２\"), int(b\"12\"), flo(b\" 2.5 \")] })\n"
  "tenta(\"h\", funct() { return int(b\"1x\") })\n"
  "tenta(\"i\", funct() { return [flo(\"inf\"), flo(\"-Infinity\"), flo(\"5.\"), flo(\".5\"), flo(\"1E-2\")] })\n"
  "tenta(\"j\", funct() { return flo(\"1e\") })\n",
  "a ERRO invalid literal for int() with base 10: '99999999999999999999 1' (linha 8)\n"
  "b 99999999999999999999\n"
  "c True\n"
  "d ERRO invalid literal for int() with base 10: '15\\x00' (linha 11)\n"
  "e ERRO could not convert string to flo: '1.5\\x00abc' (linha 12)\n"
  "f ERRO could not convert string to flo: '0x1A' (linha 13)\n"
  "g [34, 42, 12, 2.5]\n"
  "h ERRO invalid literal for int() with base 10: b'1x' (linha 15)\n"
  "i [inf, -inf, 5.0, 0.5, 0.01]\n"
  "j ERRO could not convert string to flo: '1e' (linha 17)", NULL, 0 },
{ "json.parse pela gramatica do numero do JSON, sem teto de digitos (lote 7b)",
  "import json\n"
  "post(len(str(json.parse(\"9\" * 600))), json.parse(\"1e5\"), json.parse(\"-0\"), json.parse(\"2.5E-1\"))\n"
  "for each t in [\"-\", \"01\", \"1.\"] {\n"
  "    try {\n"
  "        post(json.parse(t))\n"
  "    } catch (Exception e) {\n"
  "        post(e)\n"
  "    }\n"
  "}\n",
  "600 100000.0 0 0.25\n"
  "Expecting value: line 1 column 1 (char 0) (linha 5)\n"
  "Extra data: line 1 column 2 (char 1) (linha 5)\n"
  "Extra data: line 1 column 2 (char 1) (linha 5)", NULL, 0 },
{ "int x flo exato no ==, na ordem, na chave de dict e na divisao (lote 7b)",
  "post(9007199254740993 == 9007199254740992.0, 9007199254740992 == 9007199254740992.0, 2 ** 64 == 2.0 ** 64)\n"
  "post(9007199254740993 > 9007199254740992.0, 2 ** 64 + 1 > 2.0 ** 64, 2 ** 70 < 1e300)\n"
  "dict h = {9007199254740992.0: \"x\"}\n"
  "dict d = {}\n"
  "d[2 ** 64] = \"big\"\n"
  "post(9007199254740993 in h, 9007199254740992 in h, 2.0 ** 64 in d, d.get(2.0 ** 64))\n"
  "post(flo(2 ** 60 + 3) == flo(str(2 ** 60 + 3)), flo(2 ** 70), [10 ** 400 / 10 ** 399, (2 ** 63 - 1) / 3])\n",
  "False True True\nTrue True True\nFalse True True big\n"
  "True 1.1805916207174113e+21 [10.0, 3.0744573456182584e+18]", NULL, 0 },
{ "inteiro grande que nao cabe num flo e OverflowError, nao inf (lote 7b)",
  "list contas = [funct() { return flo(10 ** 400) }, funct() { return 10 ** 400 + 0.5 }, funct() { return 10 ** 400 / 3 }, funct() { return sum([10 ** 400, 0.5]) }, funct() { return pow(10 ** 400, -1) }, funct() { return \"{:.1f}\".format(10 ** 400) }]\n"
  "for each f in contas {\n"
  "    try {\n"
  "        post(f())\n"
  "    } catch (OverflowError e) {\n"
  "        post(e)\n"
  "    }\n"
  "}\n",
  "int too large to convert to flo (linha 1)\n"
  "int too large to convert to flo (linha 1)\n"
  "integer division result too large for a flo (linha 1)\n"
  "int too large to convert to flo (linha 1)\n"
  "int too large to convert to flo (linha 1)\n"
  "int too large to convert to flo (linha 1)", NULL, 0 },
{ "list e tup nao se ordenam entre si; NaN nao se ordena com nada (lote 7b)",
  "for each p in [funct() { return [[1, 2]][0] <= (1, 2) }, funct() { return sorted([[1], (2,)]) }] {\n"
  "    try {\n"
  "        post(p())\n"
  "    } catch (TypeError e) {\n"
  "        post(e)\n"
  "    }\n"
  "}\n"
  "flo nan = flo(\"nan\")\n"
  "post(nan < 1, 1 < nan, nan == nan, [1, 2] == (1, 2))\n",
  "'<=' not supported between instances of 'list' and 'tup' (linha 1)\n"
  "'<' not supported between instances of 'tup' and 'list' (linha 1)\n"
  "False False False False", NULL, 0 },
{ "igualdade de estrutura sem recursao: 300 e 20 mil niveis; ciclicas pela forma (lote 7b)",
  "list funct aninha(int n) {\n"
  "    list x = [0]\n"
  "    for each i in range(n) {\n"
  "        x = [x]\n"
  "    }\n"
  "    return x\n"
  "}\n"
  "list a = aninha(300)\n"
  "list b = aninha(300)\n"
  "post(a == b, a != b, a in [b], [a].count(b), [a].index(b))\n"
  "post(aninha(20000) == aninha(20000), aninha(20000) == aninha(19999))\n"
  "list c = [1]\n"
  "c.append(c)\n"
  "list d = [1]\n"
  "d.append(d)\n"
  "list e = [2]\n"
  "e.append(e)\n"
  "dict f = {\"k\": 1}\n"
  "f[\"eu\"] = f\n"
  "dict g = {\"k\": 1}\n"
  "g[\"eu\"] = g\n"
  "post(c == c, c == d, c == e, f == g)\n",
  "True False True 1 0\nTrue False\nTrue True False True", NULL, 0 },
{ "bool e inteiro grande em todo argumento inteiro, uma regra so (lote 7b)",
  "Null funct tenta(str rot, Object f) {\n"
  "    try {\n"
  "        post(rot, f())\n"
  "    } catch (Exception e) {\n"
  "        post(rot, \"ERRO\", e)\n"
  "    }\n"
  "}\n"
  "tenta(\"a\", funct() { return [[10, 20][True], \"abc\"[True:], [1, 2, 3][::True], [1] * True] })\n"
  "tenta(\"b\", funct() { list l = [10, 20]; return [l.pop(True), l] })\n"
  "tenta(\"c\", funct() { list l = [10, 20]; l.insert(True, 9); return l })\n"
  "tenta(\"d\", funct() { return [\"1\".zfill(True), \"abca\".find(\"a\", True), \"a,b,c\".split(\",\", True), \"aaa\".replace(\"a\", \"b\", True), [5, 6, 5].index(5, True)] })\n"
  "tenta(\"e\", funct() { return [chr(True), range(True), round(2.567, True)] })\n"
  "tenta(\"f\", funct() { return [1, 2][2 ** 70] })\n"
  "tenta(\"g\", funct() { list l = [1]; l[2 ** 70] = 5; return l })\n"
  "tenta(\"h\", funct() { list l = [1, 2]; l.insert(2 ** 70, 9); l.insert(-(2 ** 70), 0); return [l, \"abc\".find(\"a\", 2 ** 70), \"a,b\".split(\",\", 2 ** 70), [1, 2, 3][::2 ** 70]] })\n"
  "tenta(\"i\", funct() { return \"a\".ljust(2 ** 70) })\n"
  "tenta(\"j\", funct() { return chr(2 ** 70) })\n"
  "tenta(\"k\", funct() { return [True in b\"\\x01\", b\"ab\"[True]] })\n",
  "a [20, 'bc', [1, 2, 3], [1]]\n"
  "b [20, [10]]\n"
  "c [10, 9, 20]\n"
  "d ['1', 3, ['a', 'b,c'], 'baa', 2]\n"
  "e ['\\x01', [0], 2.6]\n"
  "f ERRO list index out of range (linha 13)\n"
  "g ERRO list assignment index out of range (linha 14)\n"
  "h [[0, 1, 2, 9], -1, ['a', 'b'], [1]]\n"
  "i ERRO memória insuficiente: ljust() pediu largura 1180591620717411303424 (linha 16)\n"
  "j ERRO chr() arg not in range(0x110000) (linha 17)\n"
  "k [True, 98]", NULL, 0 },
{ "bytes: fromint, toint, get, slice e new com inteiro grande, sem cortar o tamanho; count de byte e long (lote 7b)",
  "import bytes\n"
  "Null funct tenta(str rot, Object f) {\n"
  "    try {\n"
  "        post(rot, f())\n"
  "    } catch (Exception e) {\n"
  "        post(rot, \"ERRO\", e)\n"
  "    }\n"
  "}\n"
  "tenta(\"a\", funct() { return [bytes.fromint(2 ** 63, 8), bytes.fromint(2 ** 64, 9), bytes.fromint(True)] })\n"
  "tenta(\"b\", funct() { return bytes.fromint(1, 4294967297) })\n"
  "tenta(\"c\", funct() { return bytes.fromint(1, -1) })\n"
  "tenta(\"d\", funct() { return [bytes.toint(bytes.fromhex(\"ffffffffffffffff\")), bytes.toint(bytes.fromhex(\"010000000000000000\")), bytes.toint(bytes.fromint(2 ** 70)) == 2 ** 70] })\n"
  "tenta(\"e\", funct() { return [bytes.slice(bytes.new(\"abc\"), 2 ** 64), bytes.get(bytes.new(\"ab\"), True)] })\n"
  "tenta(\"f\", funct() { return bytes.get(bytes.new(\"ab\"), 2 ** 64) })\n"
  "tenta(\"g\", funct() { return bytes.new(2 ** 64) })\n"
  "tenta(\"h\", funct() { return b\"ab\".ljust(10 ** 12) })\n"
  "tenta(\"i\", funct() { return [int(97) count in b\"aab\", int in b\"aab\" count, byte in [1, b\"x\", \"s\"] count, long in [1, 2 ** 70] count] })\n",
  "a [b'\\x80\\x00\\x00\\x00\\x00\\x00\\x00\\x00', b'\\x01\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00', b'\\x01']\n"
  "b ERRO memória insuficiente: fromint() pediu length 4294967297 (linha 10)\n"
  "c ERRO length argument must be non-negative (linha 11)\n"
  "d [18446744073709551615, 18446744073709551616, True]\n"
  "e [b'', 98]\n"
  "f ERRO index out of range (linha 14)\n"
  "g ERRO memória insuficiente: bytes.new() pediu 18446744073709551616 bytes (linha 15)\n"
  "h ERRO memória insuficiente: ljust() pediu largura 1000000000000 (linha 16)\n"
  "i [2, 3, 1, 2]", NULL, 0 },
{ "round com casas negativas, exato; date.hora so com inteiro (lote 7b)",
  "import date\n"
  "post(round(1250, -2), round(1350, -2), round(1234.5, -2), round(-1250, -2), round(2 ** 70 + 7, -1))\n"
  "post(round(0.1, 2 ** 70), round(123.456, -(2 ** 70)), round(1e300, 2) == 1e300)\n"
  "post(date.hora(1, 30), date.hora(days=True), date.hora(2 ** 62))\n"
  "try {\n"
  "    post(date.hora([1.5][0]))\n"
  "} catch (TypeError e) {\n"
  "    post(e)\n"
  "}\n",
  "1200 1400 1200.0 -1200 1180591620717411303430\n"
  "0.1 0.0 True\n"
  "5400 86400 16602069666338596454400\n"
  "'flo' object cannot be interpreted as an integer (linha 6)", NULL, 0 },
{ "sys.exit com inteiro grande sai com o resto por 256 (lote 7b)",
  "import sys\n"
  "sys.exit(2 ** 70 + 3)\n",
  "", NULL, 3 },

/* ── Plano 6, lote 7c (17.0.17): o checador e a chamada com nome ───────────
 * O valor que passa por uma lista (`["a"][0]`) e a funcao guardada em `Object`
 * tem tipo que so se sabe rodando: e o jeito de provar o caminho da VM. */
{ "f-string com formato: o mesmo do format, campo dentro do formato, ':' dentro de colchete e aspas, string tripla (lote 7c)",
  "flo x = 3.14159\n"
  "int n = 42\n"
  "int w = 9\n"
  "str s = \"abc\"\n"
  "dict d = {\"a:b\": 1}\n"
  "post(f\"[{x:.2f}]\", f\"[{n:05d}]\", f\"[{s:>5}]\", f\"[{n:x}]\", f\"[{x:{w}.3f}]\")\n"
  "post(f\"[{s[0:2]}]\", f\"[{d['a:b']}]\", f\"[{n if n > 1 else 0:d}]\")\n"
  "post(f\"\"\"t{x:.1f}\"\"\")\n",
  "[3.14] [00042] [  abc] [2a] [    3.142]\n[ab] [1] [42]\nt3.1", NULL, 0 },
{ "f-string com formato invalido da a frase do format, rodando (lote 7c)",
  "int n = 42\npost(f\"[{n:zz}]\")\n",
  "", "ValueError: format: especificador 'zz' invalido; a forma e [preenche][<>^][0][largura][.precisao][tipo]", 1 },
{ "argumento nomeado repetido na chamada e erro antes de rodar, no segundo (lote 7c)",
  "funct f(int a = 1) {\n    return a\n}\npost(f(a=1, a=2))\n",
  "", "SyntaxError: argumento nomeado 'a' repetido na chamada", 2 },
{ "nomeado num parametro ja dado por posicao e erro: funct (lote 7c)",
  "funct f(int a = 1) {\n    return a\n}\npost(f(1, a=2))\n",
  "", "TypeError: f() got multiple values for argument 'a'", 2 },
{ "nomeado num parametro ja dado por posicao e erro: nativo (lote 7c)",
  "post(\"a,b;c\".split(\",\", sep=\";\"))\n",
  "", "TypeError: split() got multiple values for argument 'sep'", 2 },
{ "parametro obrigatorio vazio na chamada com nome e erro: get (lote 7c)",
  "dict d = {\"a\": 1}\npost(d.get(default=5))\n",
  "", "TypeError: get() missing required argument 'chave'", 2 },
{ "parametro obrigatorio vazio na chamada com nome e erro: center (lote 7c)",
  "post(\"abc\".center(fillchar=\"*\"))\n",
  "", "TypeError: center() missing required argument 'width'", 2 },
{ "format nao aceita nome nenhum (lote 7c)",
  "post(\"{} e {}\".format(b=2))\n",
  "", "TypeError: 'b' is an invalid keyword argument for format()", 2 },
{ "chamada com nome rodando: o vazio com padrao recebe o padrao, o sem padrao e erro, o apelido fala o proprio nome (lote 7c)",
  "Object h = [1].has\n"
  "try {\n"
  "    h()\n"
  "} catch (TypeError e) {\n"
  "    post(e)\n"
  "}\n"
  "Object g = {\"a\": 1}.get\n"
  "try {\n"
  "    post(g(default=5))\n"
  "} catch (TypeError e) {\n"
  "    post(e)\n"
  "}\n"
  "post([5, 2, 7, 2, 9].index(7, fim=3), \"x\".center(5, fillchar=\"*\"))\n",
  "has() takes exactly one argument (0 given) (linha 3)\n"
  "get() missing required argument 'chave' (linha 9)\n"
  "2 **x**", NULL, 0 },
{ "Pattern.sub aceita o count, que a lista de nomes nao tinha (lote 7c)",
  "import regex\nPattern p = regex.compile(\"a\")\npost(p.sub(\"x\", \"aaa\", 2), p.sub(\"x\", \"aaa\"))\n",
  "xxa xxx", NULL, 0 },
{ "--check: atribuicao por indice em str (lote 7c)",
  "str s = \"abc\"\ns[0] = \"X\"\n",
  "", "TypeError: 'str' object does not support item assignment", 2 },
{ "--check: atribuicao por indice em tup (lote 7c)",
  "tup t = (1, 2)\nt[0] = 5\n",
  "", "TypeError: 'tup' object does not support item assignment", 2 },
{ "--check: atribuicao por indice em byte (lote 7c)",
  "byte b = b\"abc\"\nb[0] = 65\n",
  "", "TypeError: 'byte' object does not support item assignment", 2 },
{ "--check: + que a VM recusa, com a frase dela (lote 7c)",
  "str s = \"a\" + 1\n",
  "", "TypeError: can only concatenate str (not \"int\") to str", 2 },
{ "--check: % entre texto e numero nao formata, e recusa como rodando (lote 7c)",
  "post(\"%d\" % 5)\n",
  "", "TypeError: unsupported operand type(s) for %: 'str' and 'int'", 2 },
{ "--check: | entre dicts recusa como rodando (lote 7c)",
  "dict a = {}\ndict b = {}\npost(a | b)\n",
  "", "TypeError: unsupported operand type(s) for |: 'dict' and 'dict'", 2 },
{ "--check: ordem entre list e tup (lote 7c)",
  "post([1] < (1,))\n",
  "", "TypeError: '<' not supported between instances of 'list' and 'tup'", 2 },
{ "--check: atribuicao com operador confere os tipos (lote 7c)",
  "int x = 1\nx += \"a\"\n",
  "", "TypeError: unsupported operand type(s) for +: 'int' and 'str'", 2 },
{ "--check: menos unario em texto (lote 7c)",
  "post(-\"a\")\n",
  "", "TypeError: bad operand type for unary -: 'str'", 2 },
{ "--check: mais unario em lista (lote 7c)",
  "post(+[1])\n",
  "", "TypeError: bad operand type for unary +: 'list'", 2 },
{ "--check: ~ em flo (lote 7c)",
  "post(~1.5)\n",
  "", "TypeError: bad operand type for unary ~: 'flo'", 2 },
{ "mais unario rodando: numero fica, bool vira int, o resto e TypeError (antes passava calado) (lote 7c)",
  "try {\n"
  "    post(+[\"a\"][0])\n"
  "} catch (TypeError e) {\n"
  "    post(e)\n"
  "}\n"
  "post(+True, +2.5, +(-0.0), -(-3))\n",
  "bad operand type for unary +: 'str' (linha 2)\n1 2.5 -0.0 3", NULL, 0 },
{ "--check: tipo do item da lista escrita no for each (lote 7c)",
  "int i = 0\nfor each i in [\"a\", \"b\"] {\n    post(i)\n}\n",
  "", "AttributedValueError: variável i esperava int, recebeu str", 2 },
{ "--check: quantidade de argumento de metodo nativo (lote 7c)",
  "post(\"a\".upper(1))\n",
  "", "TypeError: upper() takes no arguments (1 given)", 2 },
{ "--check: tipo do argumento de builtin, frase medida da VM (lote 7c)",
  "post(len(5))\n",
  "", "TypeError: object of type 'int' has no len()", 2 },
{ "--check: quantidade de argumento de funcao de modulo (lote 7c)",
  "import sys\nsys.exit(1, 2)\n",
  "", "TypeError: exit() takes at most 1 argument (2 given)", 2 },
{ "--check: chr de texto (lote 7c)",
  "post(chr(\"a\"))\n",
  "", "TypeError: 'str' object cannot be interpreted as an integer", 2 },
{ "--check: json.parse de int (lote 7c)",
  "import json\npost(json.parse(5))\n",
  "", "TypeError: parse() argument 1 must be str, not int", 2 },
{ "--check: membro de modulo trazido pelo nome tambem e conferido (lote 7c)",
  "from os import getenv\npost(getenv())\n",
  "", "TypeError: getenv() takes at least 1 argument (0 given)", 2 },
{ "--check: chamada certa de nativo passa (lote 7c)",
  "post(\"a\".upper(), len(\"ab\"), chr(97), \"x\".join([\"a\", \"b\"]))\n",
  "A 2 a axb", NULL, 0 },
{ "'**=' nao existe: a frase diz a forma certa (lote 7c)",
  "int x = 2\nx **= 3\n",
  "", "SyntaxError: '**=' nao existe: escreva x = x ** valor (atribuicao com operador so com +=, -=, *=, /= e %=)", 2 },

/* ── CLI ── */
{ "--check não executa o script",
  "post(\"NAO DEVIA RODAR\")\n", "NAO DEVIA RODAR", NULL, 0 },
};
const int NC_LINGUAGEM = N_CASOS(CASOS_LINGUAGEM);
