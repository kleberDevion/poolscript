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

/* ── CLI ── */
{ "--check não executa o script",
  "post(\"NAO DEVIA RODAR\")\n", "NAO DEVIA RODAR", NULL, 0 },
};
const int NC_LINGUAGEM = N_CASOS(CASOS_LINGUAGEM);
