/*
 * Casos de COBERTURA — gerados a partir de duas fontes, não escritos à mão:
 *
 *  1. o corpus de testes do repositório (commit f90845d), de onde saíram os
 *     programas `.ps` E a resposta esperada de cada um; só entraram aqui os
 *     que a VM de hoje confirma — divergência virou investigação, não caso.
 *  2. a matriz de INTERAÇÃO: cada feature nova exercitada dentro de cada
 *     contexto da linguagem (funct, método, aninhada, for, while, try,
 *     finally, gerador, async, using, match, bloco `:` e bloco `{}`).
 *     Passar isolado não prova nada — o que quebra é a combinação.
 */
#include "ps_teste.h"

const Caso CASOS_COBERTURA[] = {
{ "git: test_async_deep #0",
  "async funct lenta() {\n"
  "    sleep(0.3)\n"
  "    return 1\n"
  "}\n"
  "f = lenta()\n"
  "post(\"chamou\")\n"
  "\n",
  "chamou", NULL, 0 },
{ "git: test_async_deep #1",
  "async funct lenta(n) {\n"
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
  "async funct falha() {\n"
  "    x = 1 / 0\n"
  "    return x\n"
  "}\n"
  "f = falha()\n"
  "sleep(0.05)\n"
  "post(\"nao_lancou_ainda\")\n"
  "\n",
  "nao_lancou_ainda", NULL, 0 },
{ "git: test_async_deep #7",
  "async funct ident(n) {\n"
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
{ "git: test_interpreter #61",
  "x = Null > 0\n"
  "post(x)\n"
  "\n",
  "", "'>' not supported between instances of 'Null' and 'int'", 1 },
{ "git: test_interpreter #62",
  "lista = [1, 2, 3]\n"
  "x = lista[99]\n"
  "post(x)\n"
  "\n",
  "", "list index out of range", 1 },
{ "git: test_vm_c #231",
  "post([1, 2][9])\n"
  "\n",
  "", "list index out of range", 1 },
{ "git: test_vm_c #232",
  "post(\"ab\"[9])\n"
  "\n",
  "", "string index out of range", 1 },
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
  "funct inc() {\n"
  "    a = a + 1\n"
  "    return a\n"
  "}\n"
  "post(inc(), inc())\n"
  "\n",
  "2 3", NULL, 0 },
{ "matriz: closure_self em topo",
  "x = 5\n"
  "funct le() {\n"
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
  "async funct d(n) {\n"
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
{ "matriz: char em funct",
  "funct f() {\n"
  "    char c = 64\n"
  "    post(c)\n"
  "}\n"
  "f()\n"
  "\n",
  "@", NULL, 0 },
{ "matriz: char_texto em funct",
  "funct f() {\n"
  "    char c = \"ç\"\n"
  "    post(c, len(c))\n"
  "}\n"
  "f()\n"
  "\n",
  "ç 1", NULL, 0 },
{ "matriz: closure em funct",
  "funct f() {\n"
  "    a = 1\n"
  "    funct inc() {\n"
  "        a = a + 1\n"
  "        return a\n"
  "    }\n"
  "    post(inc(), inc())\n"
  "}\n"
  "f()\n"
  "\n",
  "2 3", NULL, 0 },
{ "matriz: closure_self em funct",
  "funct f() {\n"
  "    x = 5\n"
  "    funct le() {\n"
  "        return x\n"
  "    }\n"
  "    post(le())\n"
  "}\n"
  "f()\n"
  "\n",
  "5", NULL, 0 },
{ "matriz: startswith em funct",
  "funct f() {\n"
  "    post(\"abc\".startswith((\"z\", \"a\")))\n"
  "}\n"
  "f()\n"
  "\n",
  "True", NULL, 0 },
{ "matriz: index_faixa em funct",
  "funct f() {\n"
  "    l = [1, 2, 3, 2]\n"
  "    post(l.index(2, 2))\n"
  "}\n"
  "f()\n"
  "\n",
  "3", NULL, 0 },
{ "matriz: encode em funct",
  "funct f() {\n"
  "    post(\"café\".encode(\"latin-1\"))\n"
  "}\n"
  "f()\n"
  "\n",
  "b'caf\\xe9'", NULL, 0 },
{ "matriz: decode em funct",
  "funct f() {\n"
  "    post(\"café\".encode(\"latin-1\").decode(\"latin-1\"))\n"
  "}\n"
  "f()\n"
  "\n",
  "café", NULL, 0 },
{ "matriz: num_base em funct",
  "funct f() {\n"
  "    post(0x1F, 0b101, 1_000, 1e3)\n"
  "}\n"
  "f()\n"
  "\n",
  "31 5 1000 1000.0", NULL, 0 },
{ "matriz: unicode em funct",
  "funct f() {\n"
  "    post(\"Ω\".lower(), \"ß\".upper())\n"
  "}\n"
  "f()\n"
  "\n",
  "ω SS", NULL, 0 },
{ "matriz: isdigit em funct",
  "funct f() {\n"
  "    post(\"²\".isdigit(), \"²\".isdecimal())\n"
  "}\n"
  "f()\n"
  "\n",
  "True False", NULL, 0 },
{ "matriz: regex_split em funct",
  "funct f() {\n"
  "    import regex\n"
  "    post(regex.split(r\"(,)\", \"a,b\"))\n"
  "}\n"
  "f()\n"
  "\n",
  "['a', ',', 'b']", NULL, 0 },
{ "matriz: regex_sub em funct",
  "funct f() {\n"
  "    import regex\n"
  "    post(regex.sub(r\"(\\d)\", r\"[\\g<1>]\", \"a1\"))\n"
  "}\n"
  "f()\n"
  "\n",
  "a[1]", NULL, 0 },
{ "matriz: json_emoji em funct",
  "funct f() {\n"
  "    import json\n"
  "    post(json.parse(\"{\\\"a\\\": \\\"\\\\ud83d\\\\ude00\\\"}\"))\n"
  "}\n"
  "f()\n"
  "\n",
  "{'a': '😀'}", NULL, 0 },
{ "matriz: await_lista em funct",
  "funct f() {\n"
  "    async funct d(n) {\n"
  "        return n * 2\n"
  "    }\n"
  "    post(await [d(1), d(2)])\n"
  "}\n"
  "f()\n"
  "\n",
  "[2, 4]", NULL, 0 },
{ "matriz: int_inf em funct",
  "funct f() {\n"
  "    try {\n"
  "        post(int(flo(\"inf\")))\n"
  "    } catch(e) {\n"
  "        post(\"ok-erro\")\n"
  "    }\n"
  "}\n"
  "f()\n"
  "\n",
  "ok-erro", NULL, 0 },
{ "matriz: nul em funct",
  "funct f() {\n"
  "    post(len(\"a\\x00b\"))\n"
  "}\n"
  "f()\n"
  "\n",
  "3", NULL, 0 },
{ "matriz: fatia em funct",
  "funct f() {\n"
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
  "    funct inc() {\n"
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
  "    funct le() {\n"
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
  "    async funct d(n) {\n"
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
  "    funct inc() {\n"
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
  "    funct le() {\n"
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
  "    async funct d(n) {\n"
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
  "    funct inc() {\n"
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
  "    funct le() {\n"
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
  "    async funct d(n) {\n"
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
  "    funct inc() {\n"
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
  "    funct le() {\n"
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
  "    async funct d(n) {\n"
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
  "    funct inc() {\n"
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
  "    funct le() {\n"
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
  "    async funct d(n) {\n"
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
  "    funct m(self) {\n"
  "        char c = 64\n"
  "        post(c)\n"
  "    }\n"
  "}\n"
  "K().m()\n"
  "\n",
  "@", NULL, 0 },
{ "matriz: char_texto em metodo",
  "Entity K() {\n"
  "    funct m(self) {\n"
  "        char c = \"ç\"\n"
  "        post(c, len(c))\n"
  "    }\n"
  "}\n"
  "K().m()\n"
  "\n",
  "ç 1", NULL, 0 },
{ "matriz: closure em metodo",
  "Entity K() {\n"
  "    funct m(self) {\n"
  "        a = 1\n"
  "        funct inc() {\n"
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
  "    funct m(self) {\n"
  "        x = 5\n"
  "        funct le() {\n"
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
  "    funct m(self) {\n"
  "        post(\"abc\".startswith((\"z\", \"a\")))\n"
  "    }\n"
  "}\n"
  "K().m()\n"
  "\n",
  "True", NULL, 0 },
{ "matriz: index_faixa em metodo",
  "Entity K() {\n"
  "    funct m(self) {\n"
  "        l = [1, 2, 3, 2]\n"
  "        post(l.index(2, 2))\n"
  "    }\n"
  "}\n"
  "K().m()\n"
  "\n",
  "3", NULL, 0 },
{ "matriz: encode em metodo",
  "Entity K() {\n"
  "    funct m(self) {\n"
  "        post(\"café\".encode(\"latin-1\"))\n"
  "    }\n"
  "}\n"
  "K().m()\n"
  "\n",
  "b'caf\\xe9'", NULL, 0 },
{ "matriz: decode em metodo",
  "Entity K() {\n"
  "    funct m(self) {\n"
  "        post(\"café\".encode(\"latin-1\").decode(\"latin-1\"))\n"
  "    }\n"
  "}\n"
  "K().m()\n"
  "\n",
  "café", NULL, 0 },
{ "matriz: num_base em metodo",
  "Entity K() {\n"
  "    funct m(self) {\n"
  "        post(0x1F, 0b101, 1_000, 1e3)\n"
  "    }\n"
  "}\n"
  "K().m()\n"
  "\n",
  "31 5 1000 1000.0", NULL, 0 },
{ "matriz: unicode em metodo",
  "Entity K() {\n"
  "    funct m(self) {\n"
  "        post(\"Ω\".lower(), \"ß\".upper())\n"
  "    }\n"
  "}\n"
  "K().m()\n"
  "\n",
  "ω SS", NULL, 0 },
{ "matriz: isdigit em metodo",
  "Entity K() {\n"
  "    funct m(self) {\n"
  "        post(\"²\".isdigit(), \"²\".isdecimal())\n"
  "    }\n"
  "}\n"
  "K().m()\n"
  "\n",
  "True False", NULL, 0 },
{ "matriz: regex_split em metodo",
  "Entity K() {\n"
  "    funct m(self) {\n"
  "        import regex\n"
  "        post(regex.split(r\"(,)\", \"a,b\"))\n"
  "    }\n"
  "}\n"
  "K().m()\n"
  "\n",
  "['a', ',', 'b']", NULL, 0 },
{ "matriz: regex_sub em metodo",
  "Entity K() {\n"
  "    funct m(self) {\n"
  "        import regex\n"
  "        post(regex.sub(r\"(\\d)\", r\"[\\g<1>]\", \"a1\"))\n"
  "    }\n"
  "}\n"
  "K().m()\n"
  "\n",
  "a[1]", NULL, 0 },
{ "matriz: json_emoji em metodo",
  "Entity K() {\n"
  "    funct m(self) {\n"
  "        import json\n"
  "        post(json.parse(\"{\\\"a\\\": \\\"\\\\ud83d\\\\ude00\\\"}\"))\n"
  "    }\n"
  "}\n"
  "K().m()\n"
  "\n",
  "{'a': '😀'}", NULL, 0 },
{ "matriz: await_lista em metodo",
  "Entity K() {\n"
  "    funct m(self) {\n"
  "        async funct d(n) {\n"
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
  "    funct m(self) {\n"
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
  "    funct m(self) {\n"
  "        post(len(\"a\\x00b\"))\n"
  "    }\n"
  "}\n"
  "K().m()\n"
  "\n",
  "3", NULL, 0 },
{ "matriz: fatia em metodo",
  "Entity K() {\n"
  "    funct m(self) {\n"
  "        post(\"abcdef\"[999999999999999999999:])\n"
  "    }\n"
  "}\n"
  "K().m()\n"
  "\n",
  "", NULL, 0 },
{ "matriz: char em aninhada",
  "funct fora() {\n"
  "    funct dentro() {\n"
  "        char c = 64\n"
  "        post(c)\n"
  "    }\n"
  "    dentro()\n"
  "}\n"
  "fora()\n"
  "\n",
  "@", NULL, 0 },
{ "matriz: char_texto em aninhada",
  "funct fora() {\n"
  "    funct dentro() {\n"
  "        char c = \"ç\"\n"
  "        post(c, len(c))\n"
  "    }\n"
  "    dentro()\n"
  "}\n"
  "fora()\n"
  "\n",
  "ç 1", NULL, 0 },
{ "matriz: closure em aninhada",
  "funct fora() {\n"
  "    funct dentro() {\n"
  "        a = 1\n"
  "        funct inc() {\n"
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
  "funct fora() {\n"
  "    funct dentro() {\n"
  "        x = 5\n"
  "        funct le() {\n"
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
  "funct fora() {\n"
  "    funct dentro() {\n"
  "        post(\"abc\".startswith((\"z\", \"a\")))\n"
  "    }\n"
  "    dentro()\n"
  "}\n"
  "fora()\n"
  "\n",
  "True", NULL, 0 },
{ "matriz: index_faixa em aninhada",
  "funct fora() {\n"
  "    funct dentro() {\n"
  "        l = [1, 2, 3, 2]\n"
  "        post(l.index(2, 2))\n"
  "    }\n"
  "    dentro()\n"
  "}\n"
  "fora()\n"
  "\n",
  "3", NULL, 0 },
{ "matriz: encode em aninhada",
  "funct fora() {\n"
  "    funct dentro() {\n"
  "        post(\"café\".encode(\"latin-1\"))\n"
  "    }\n"
  "    dentro()\n"
  "}\n"
  "fora()\n"
  "\n",
  "b'caf\\xe9'", NULL, 0 },
{ "matriz: decode em aninhada",
  "funct fora() {\n"
  "    funct dentro() {\n"
  "        post(\"café\".encode(\"latin-1\").decode(\"latin-1\"))\n"
  "    }\n"
  "    dentro()\n"
  "}\n"
  "fora()\n"
  "\n",
  "café", NULL, 0 },
{ "matriz: num_base em aninhada",
  "funct fora() {\n"
  "    funct dentro() {\n"
  "        post(0x1F, 0b101, 1_000, 1e3)\n"
  "    }\n"
  "    dentro()\n"
  "}\n"
  "fora()\n"
  "\n",
  "31 5 1000 1000.0", NULL, 0 },
{ "matriz: unicode em aninhada",
  "funct fora() {\n"
  "    funct dentro() {\n"
  "        post(\"Ω\".lower(), \"ß\".upper())\n"
  "    }\n"
  "    dentro()\n"
  "}\n"
  "fora()\n"
  "\n",
  "ω SS", NULL, 0 },
{ "matriz: isdigit em aninhada",
  "funct fora() {\n"
  "    funct dentro() {\n"
  "        post(\"²\".isdigit(), \"²\".isdecimal())\n"
  "    }\n"
  "    dentro()\n"
  "}\n"
  "fora()\n"
  "\n",
  "True False", NULL, 0 },
{ "matriz: regex_split em aninhada",
  "funct fora() {\n"
  "    funct dentro() {\n"
  "        import regex\n"
  "        post(regex.split(r\"(,)\", \"a,b\"))\n"
  "    }\n"
  "    dentro()\n"
  "}\n"
  "fora()\n"
  "\n",
  "['a', ',', 'b']", NULL, 0 },
{ "matriz: regex_sub em aninhada",
  "funct fora() {\n"
  "    funct dentro() {\n"
  "        import regex\n"
  "        post(regex.sub(r\"(\\d)\", r\"[\\g<1>]\", \"a1\"))\n"
  "    }\n"
  "    dentro()\n"
  "}\n"
  "fora()\n"
  "\n",
  "a[1]", NULL, 0 },
{ "matriz: json_emoji em aninhada",
  "funct fora() {\n"
  "    funct dentro() {\n"
  "        import json\n"
  "        post(json.parse(\"{\\\"a\\\": \\\"\\\\ud83d\\\\ude00\\\"}\"))\n"
  "    }\n"
  "    dentro()\n"
  "}\n"
  "fora()\n"
  "\n",
  "{'a': '😀'}", NULL, 0 },
{ "matriz: await_lista em aninhada",
  "funct fora() {\n"
  "    funct dentro() {\n"
  "        async funct d(n) {\n"
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
  "funct fora() {\n"
  "    funct dentro() {\n"
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
  "funct fora() {\n"
  "    funct dentro() {\n"
  "        post(len(\"a\\x00b\"))\n"
  "    }\n"
  "    dentro()\n"
  "}\n"
  "fora()\n"
  "\n",
  "3", NULL, 0 },
{ "matriz: fatia em aninhada",
  "funct fora() {\n"
  "    funct dentro() {\n"
  "        post(\"abcdef\"[999999999999999999999:])\n"
  "    }\n"
  "    dentro()\n"
  "}\n"
  "fora()\n"
  "\n",
  "", NULL, 0 },
{ "matriz: char em gerador",
  "funct g() {\n"
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
  "funct g() {\n"
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
  "funct g() {\n"
  "    a = 1\n"
  "    funct inc() {\n"
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
  "funct g() {\n"
  "    x = 5\n"
  "    funct le() {\n"
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
  "funct g() {\n"
  "    post(\"abc\".startswith((\"z\", \"a\")))\n"
  "    yield 1\n"
  "}\n"
  "for each _v in g() {\n"
  "    _u = _v\n"
  "}\n"
  "\n",
  "True", NULL, 0 },
{ "matriz: index_faixa em gerador",
  "funct g() {\n"
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
  "funct g() {\n"
  "    post(\"café\".encode(\"latin-1\"))\n"
  "    yield 1\n"
  "}\n"
  "for each _v in g() {\n"
  "    _u = _v\n"
  "}\n"
  "\n",
  "b'caf\\xe9'", NULL, 0 },
{ "matriz: decode em gerador",
  "funct g() {\n"
  "    post(\"café\".encode(\"latin-1\").decode(\"latin-1\"))\n"
  "    yield 1\n"
  "}\n"
  "for each _v in g() {\n"
  "    _u = _v\n"
  "}\n"
  "\n",
  "café", NULL, 0 },
{ "matriz: num_base em gerador",
  "funct g() {\n"
  "    post(0x1F, 0b101, 1_000, 1e3)\n"
  "    yield 1\n"
  "}\n"
  "for each _v in g() {\n"
  "    _u = _v\n"
  "}\n"
  "\n",
  "31 5 1000 1000.0", NULL, 0 },
{ "matriz: unicode em gerador",
  "funct g() {\n"
  "    post(\"Ω\".lower(), \"ß\".upper())\n"
  "    yield 1\n"
  "}\n"
  "for each _v in g() {\n"
  "    _u = _v\n"
  "}\n"
  "\n",
  "ω SS", NULL, 0 },
{ "matriz: isdigit em gerador",
  "funct g() {\n"
  "    post(\"²\".isdigit(), \"²\".isdecimal())\n"
  "    yield 1\n"
  "}\n"
  "for each _v in g() {\n"
  "    _u = _v\n"
  "}\n"
  "\n",
  "True False", NULL, 0 },
{ "matriz: regex_split em gerador",
  "funct g() {\n"
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
  "funct g() {\n"
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
  "funct g() {\n"
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
  "funct g() {\n"
  "    async funct d(n) {\n"
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
  "funct g() {\n"
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
  "funct g() {\n"
  "    post(len(\"a\\x00b\"))\n"
  "    yield 1\n"
  "}\n"
  "for each _v in g() {\n"
  "    _u = _v\n"
  "}\n"
  "\n",
  "3", NULL, 0 },
{ "matriz: fatia em gerador",
  "funct g() {\n"
  "    post(\"abcdef\"[999999999999999999999:])\n"
  "    yield 1\n"
  "}\n"
  "for each _v in g() {\n"
  "    _u = _v\n"
  "}\n"
  "\n",
  "", NULL, 0 },
{ "matriz: char em async",
  "async funct a() {\n"
  "    char c = 64\n"
  "    post(c)\n"
  "    return 1\n"
  "}\n"
  "post(await a())\n"
  "\n",
  "@\n1", NULL, 0 },
{ "matriz: char_texto em async",
  "async funct a() {\n"
  "    char c = \"ç\"\n"
  "    post(c, len(c))\n"
  "    return 1\n"
  "}\n"
  "post(await a())\n"
  "\n",
  "ç 1\n1", NULL, 0 },
{ "matriz: closure em async",
  "async funct a() {\n"
  "    a = 1\n"
  "    funct inc() {\n"
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
  "async funct a() {\n"
  "    x = 5\n"
  "    funct le() {\n"
  "        return x\n"
  "    }\n"
  "    post(le())\n"
  "    return 1\n"
  "}\n"
  "post(await a())\n"
  "\n",
  "5\n1", NULL, 0 },
{ "matriz: startswith em async",
  "async funct a() {\n"
  "    post(\"abc\".startswith((\"z\", \"a\")))\n"
  "    return 1\n"
  "}\n"
  "post(await a())\n"
  "\n",
  "True\n1", NULL, 0 },
{ "matriz: index_faixa em async",
  "async funct a() {\n"
  "    l = [1, 2, 3, 2]\n"
  "    post(l.index(2, 2))\n"
  "    return 1\n"
  "}\n"
  "post(await a())\n"
  "\n",
  "3\n1", NULL, 0 },
{ "matriz: encode em async",
  "async funct a() {\n"
  "    post(\"café\".encode(\"latin-1\"))\n"
  "    return 1\n"
  "}\n"
  "post(await a())\n"
  "\n",
  "b'caf\\xe9'\n1", NULL, 0 },
{ "matriz: decode em async",
  "async funct a() {\n"
  "    post(\"café\".encode(\"latin-1\").decode(\"latin-1\"))\n"
  "    return 1\n"
  "}\n"
  "post(await a())\n"
  "\n",
  "café\n1", NULL, 0 },
{ "matriz: num_base em async",
  "async funct a() {\n"
  "    post(0x1F, 0b101, 1_000, 1e3)\n"
  "    return 1\n"
  "}\n"
  "post(await a())\n"
  "\n",
  "31 5 1000 1000.0\n1", NULL, 0 },
{ "matriz: unicode em async",
  "async funct a() {\n"
  "    post(\"Ω\".lower(), \"ß\".upper())\n"
  "    return 1\n"
  "}\n"
  "post(await a())\n"
  "\n",
  "ω SS\n1", NULL, 0 },
{ "matriz: isdigit em async",
  "async funct a() {\n"
  "    post(\"²\".isdigit(), \"²\".isdecimal())\n"
  "    return 1\n"
  "}\n"
  "post(await a())\n"
  "\n",
  "True False\n1", NULL, 0 },
{ "matriz: regex_split em async",
  "async funct a() {\n"
  "    import regex\n"
  "    post(regex.split(r\"(,)\", \"a,b\"))\n"
  "    return 1\n"
  "}\n"
  "post(await a())\n"
  "\n",
  "['a', ',', 'b']\n1", NULL, 0 },
{ "matriz: regex_sub em async",
  "async funct a() {\n"
  "    import regex\n"
  "    post(regex.sub(r\"(\\d)\", r\"[\\g<1>]\", \"a1\"))\n"
  "    return 1\n"
  "}\n"
  "post(await a())\n"
  "\n",
  "a[1]\n1", NULL, 0 },
{ "matriz: json_emoji em async",
  "async funct a() {\n"
  "    import json\n"
  "    post(json.parse(\"{\\\"a\\\": \\\"\\\\ud83d\\\\ude00\\\"}\"))\n"
  "    return 1\n"
  "}\n"
  "post(await a())\n"
  "\n",
  "{'a': '😀'}\n1", NULL, 0 },
{ "matriz: await_lista em async",
  "async funct a() {\n"
  "    async funct d(n) {\n"
  "        return n * 2\n"
  "    }\n"
  "    post(await [d(1), d(2)])\n"
  "    return 1\n"
  "}\n"
  "post(await a())\n"
  "\n",
  "[2, 4]\n1", NULL, 0 },
{ "matriz: int_inf em async",
  "async funct a() {\n"
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
  "async funct a() {\n"
  "    post(len(\"a\\x00b\"))\n"
  "    return 1\n"
  "}\n"
  "post(await a())\n"
  "\n",
  "3\n1", NULL, 0 },
{ "matriz: fatia em async",
  "async funct a() {\n"
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
  "    funct inc() {\n"
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
  "    funct le() {\n"
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
  "    async funct d(n) {\n"
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
  "        funct inc() {\n"
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
  "        funct le() {\n"
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
  "        async funct d(n) {\n"
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
