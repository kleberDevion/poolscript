/*
 * Casos que MATAVAM o processo da VM — segfault, SIGFPE, OOM.
 *
 * Estes são os mais importantes da suíte: o runner roda em subprocesso
 * exatamente por causa deles. Se a VM morrer de novo, o caso volta a falhar
 * com "MORREU com sinal N" em vez de derrubar a bateria inteira.
 * Origem: caça com agentes, 2026-08-25.
 */
#include "ps_teste.h"

const Caso CASOS_CRASH[] = {
/* ── sonda 2 (2026-10-06), texto e comparacao: 4 quedas/vazamentos ──────────
 * Cada um derrubava o processo, lia fora do buffer ou comia a memoria da
 * maquina; o conserto e um mecanismo geral (teto conferido na leitura,
 * identidade antes de descer, folga da pilha medida). */
{ "format: indice posicional acima de 2^31 e IndexError, nao segfault",
  "str t = \"{2147483648}\"\npost(t.format(\"primeiro\"))\n", "",
  "IndexError: Replacement index 2147483648 out of range for positional args tuple", 1 },
{ "format_map com indice gigante (args NULL) e IndexError, nao segfault",
  "dict d = {\"a\": 1}\nstr t = \"{2147483648}\"\npost(t.format_map(d))\n", "",
  "IndexError: Replacement index 2147483648 out of range for positional args tuple", 1 },
{ "format: indice que dava a volta em 2^32 nao le o argumento errado",
  "str t = \"{4294967296}|{4294967297}\"\npost(t.format(\"A\", \"B\"))\n", "",
  "IndexError: Replacement index 4294967296 out of range for positional args tuple", 1 },
{ "lista ciclica: l < l responde False sem recursar (identidade)",
  "list l = []\nl.append(l)\nbool r = l < l\npost(r, l <= l, l > l)\n", "False True False", NULL, 0 },
{ "lista ciclica: sort e sorted com o mesmo item nao caem",
  "list l = []\nl.append(l)\nlist m = [l, l]\nm.sort()\npost(len(m), len(sorted(m)))\n", "2 2", NULL, 0 },
{ "listas ciclicas distintas: max e < dao RecursionError, nao segfault",
  "list x = [1]\nlist y = [1]\nx.append(x)\ny.append(y)\ntry {\n    post(len(max([x, y])))\n} catch (RecursionError e) {\n    post(\"max:\", \"comparacao aninhada demais\" in e)\n}\npost(x < y)\n", "max: True",
  "RecursionError: comparacao aninhada demais", 1 },
{ "format {:f} de 1e300 nao le fora do buffer: 308 digitos, sem lixo",
  "str r = \"{:f}\".format(1e300)\npost(len(r), r.startswith(\"10000000000000000525\"), r.endswith(\".000000\"), r.count(\"\\x00\"))\n"
  "post(len(\"{:.200f}\".format(0.1)), \"{:>12.3f}|\".format(3.14159), \"{:08.2f}\".format(2.5))\n",
  "308 True True 0\n202        3.142| 00002.50", NULL, 0 },
{ "expandtabs: largura acima do teto e MemoryError na hora, em str e em bytes",
  "str s = \"a\\tb\"\ntry {\n    s.expandtabs(1000000000000)\n} catch (MemoryError e) {\n    post(\"str:\", \"pediu largura 1000000000000\" in e)\n}\n"
  "byte b = b\"a\\tb\"\ntry {\n    b.expandtabs(4294967300)\n} catch (MemoryError e) {\n    post(\"byte:\", \"pediu largura 4294967300\" in e)\n}\npost(s.expandtabs(4), b.expandtabs(4))\n",
  "str: True\nbyte: True\na   b b'a   b'", NULL, 0 },

/* ── sonda 2, regex: tres quedas do processo ─────────────────────────────── */
{ "regex.sub: \\g<N> acima de 2^31 e invalid group reference, nao segfault",
  "import regex\npost(regex.sub(\"(a)\", r\"[\\g<1>]\", \"xax\"))\n"
  "try {\n    post(regex.sub(\"(a)\", r\"[\\g<2147483648>]\", \"xax\"))\n} catch (e) {\n    post(\"erro:\", e)\n}\n"
  "try {\n    post(regex.sub(\"(a)\", r\"[\\g<4294967295>]\", \"xax\"))\n} catch (e) {\n    post(\"erro:\", e)\n}\n",
  "x[a]x\nerro: invalid group reference 2147483648 at position 4 (linha 4)\nerro: invalid group reference 4294967295 at position 4 (linha 9)", NULL, 0 },
{ "regex dentro de async funct (fibra de 256 KB) e erro de profundidade, nao segfault",
  /* 1500 repeticoes (uns 4500 niveis, abaixo do RX_MAX_PROF de 6000): o
   * casador na fibra mede a folga da pilha e responde — True aqui, ou o
   * RuntimeError de profundidade numa pilha menor — em vez de derrubar o
   * processo. O resultado da fibra tem que ser o do principal. */
  "import regex\nasync funct valida(str texto) {\n    return regex.match(\"(?:[a-z]+,)*[a-z]+\", texto)\n}\n"
  "str csv = \"ab,\" * 1500 + \"fim\"\npost(\"principal:\", regex.match(\"(?:[a-z]+,)*[a-z]+\", csv))\n"
  "try {\n    post(\"async:\", await valida(csv))\n} catch (RuntimeError e) {\n    post(\"async:\", \"profundo demais\" in e)\n}\n",
  "principal: True\nasync: True", NULL, 0 },
{ "regex.compile com 20 mil grupos aninhados e ValueError, nao segfault",
  "import regex\nint n = 20000\nstr p = \"(?:\" * n + \"a\" + \")\" * n\nObject c = regex.compile(p)\npost(\"compilou\")\n", "",
  "ValueError: regex: padrao aninhado demais", 1 },
{ "regex: quantificador {n} acima de 9 digitos e erro, nao estouro calado",
  "import regex\ntry {\n    post(regex.match(\"a{4294967295}\", \"\"))\n} catch (e) {\n    post(\"erro:\", e)\n}\n"
  "try {\n    regex.compile(\"a{3,4294967297}\")\n} catch (e) {\n    post(\"erro:\", e)\n}\npost(regex.match(\"a{2,3}\", \"aaa\"), regex.match(\"a{2}\", \"a\"))\n",
  /* sem posição, como no `re` (lá é OverflowError, que não tem posição) */
  "erro: the repetition number is too large (linha 3)\nerro: the repetition number is too large (linha 8)\nTrue False", NULL, 0 },
/* ── sonda 2, parser e compilador: recursao e tetos (lote 3) ──────────────── */
{ "return dentro de finally nao recompila o finally sem fim (era segfault)",
  "Null funct f() {\n    try {\n        post(\"corpo\")\n    } finally {\n        return\n    }\n}\nf()\npost(\"depois\")\n", "corpo\ndepois", NULL, 0 },
{ "return no try e no finally, break no finally dentro de laco",
  "int funct g() {\n    try {\n        return 1\n    } finally {\n        return 2\n    }\n}\npost(g())\n"
  "for each i in [1, 2] {\n    try {\n        post(\"tenta\", i)\n    } finally {\n        break\n    }\n}\npost(\"fim\")\n", "2\ntenta 1\nfim", NULL, 0 },
{ "return no try com finally roda o finally uma vez, e o fim normal tambem",
  "Null funct h(int n) {\n    try {\n        if n > 0 {\n            post(\"ret\", n)\n            return\n        }\n    } finally {\n        post(\"finally\", n)\n    }\n    post(\"fim\", n)\n}\nh(5)\nh(0)\n",
  "ret 5\nfinally 5\nfinally 0\nfim 0", NULL, 0 },
{ "soma plana de 8000 termos compila e roda (a cadeia a esquerda nao e recursao)",
  /* gerado pelo proprio programa: 8000 termos a mao nao cabem aqui */
  "import os\nimport sys\nstr src = \"int s = 0\" + \" + 1\" * 8000 + \"\\npost(s)\\n\"\n"
  "using open(\"soma.pr\", \"w\") as f { f.write(src) }\n"
  "Process p = os.run([sys.executable, \"soma.pr\"], capture=\"live\")\nstr t = p.read(65536)\nint rc = p.wait()\npost(t.strip(), rc)\n", "8000 0", NULL, 0 },
{ "8000 blocos aninhados e 20000 decoradores sao erro limpo no --check, nunca segfault",
  /* acima do que o parser e os caminhantes do compilador descem: o freio de
   * algum deles fala, em qualquer binário (com ASan o quadro é maior e ele
   * fala antes). Eram 4000 e 5000, que o compilador do 17.0.12 já compila
   * (os 4000 `if`) ou recusa pelo limite de 8 decoradores empilhados. */
  "import os\nimport sys\n"
  "str b = \"if True { \" * 8000 + \"post(1)\" + \" }\" * 8000 + \"\\n\"\n"
  "using open(\"b.pr\", \"w\") as f { f.write(b) }\n"
  "str d = \"funct a(Object f) { return f }\\n\" + \"@a\\n\" * 20000 + \"Null funct h() { post(1) }\\nh()\\n\"\n"
  "using open(\"d.pr\", \"w\") as f { f.write(d) }\n"
  "for each nome in [\"b.pr\", \"d.pr\"] {\n"
  "    Process p = os.run([sys.executable, \"--check\", nome], capture=\"live\")\n"
  "    str t = p.read(65536)\n    int rc = p.wait()\n"
  "    post(nome, rc, \"aninhado demais\" in t)\n}\n", "b.pr 1 True\nd.pr 1 True", NULL, 0 },
{ "lista literal de 65537 itens no topo, em funct e em async funct roda e devolve o tamanho (antes corrompia o heap)",
  /* 65537 > STACK_SIZE: o literal e montado em lotes de 256 (BUILD_LIST 0 +
   * LIST_EXTEND) e a cota de cada frame e a profundidade exata (`pilha_max`),
   * conferida em TODA entrada — programa, funct e fibra (2048 valores) */
  "import os\nimport sys\n"
  "list partes = []\nfor each i in range(1, 65538) { partes.append(str(i)) }\n"
  "str itens = \", \".join(partes)\n"
  "using open(\"t.pr\", \"w\") as f { f.write(\"list l = [\" + itens + \"]\\npost(len(l), l[0], l[65536])\\n\") }\n"
  "using open(\"u.pr\", \"w\") as f { f.write(\"list funct faz() {\\n    list l = [\" + itens + \"]\\n    return l\\n}\\nlist r = faz()\\npost(len(r), r[0], r[65536])\\n\") }\n"
  "using open(\"v.pr\", \"w\") as f { f.write(\"async list funct faz() {\\n    list l = [\" + itens + \"]\\n    return l\\n}\\nlist r = await faz()\\npost(len(r), r[0], r[65536])\\n\") }\n"
  "for each nome in [\"t.pr\", \"u.pr\", \"v.pr\"] {\n"
  "    Process p = os.run([sys.executable, nome], capture=\"live\")\n"
  "    str t = p.read(65536)\n    int rc = p.wait()\n"
  "    post(nome, rc, t.strip())\n}\n",
  "t.pr 0 65537 1 65537\nu.pr 0 65537 1 65537\nv.pr 0 65537 1 65537", NULL, 0 },
{ "chamada com 70000 argumentos e recusada na linha da chamada, com a conta, sem corromper (antes: heap corrompido)",
  /* o frame do programa principal nao cabe nem na pilha vazia (65536): o erro
   * sai antes de rodar, apontando a instrucao que chega ao maximo */
  "import os\nimport sys\n"
  "list partes = []\nfor each i in range(1, 70001) { partes.append(str(i)) }\n"
  "using open(\"a.pr\", \"w\") as f { f.write(\"Null funct f(*a) { post(len(a)) }\\nf(\" + \", \".join(partes) + \")\\npost(\\\"depois\\\")\\n\") }\n"
  "Process p = os.run([sys.executable, \"a.pr\"], capture=\"live\")\n"
  "str e = \"\"\nwhile True {\n    str parte = p.read_err(65536)\n    if parte == \"\" { break }\n    e = e + parte\n}\nstr t = p.read(65536)\nint rc = p.wait()\n"
  "post(rc, t.strip() == \"\", \"RecursionError: pilha de valores insuficiente (70001 valores)\" in e, \"linha 2\" in e)\n",
  "1 True True True", NULL, 0 },
{ "3000 argumentos dentro de async funct: a fibra tem 2048 valores, o erro diz a conta e 1000 cabem",
  "import os\nimport sys\n"
  "list partes = []\nfor each i in range(1, 3001) { partes.append(str(i)) }\n"
  "using open(\"b.pr\", \"w\") as f { f.write(\"Null funct f(*a) { post(len(a)) }\\nasync Null funct g() {\\n    f(\" + \", \".join(partes) + \")\\n}\\nawait g()\\npost(\\\"depois\\\")\\n\") }\n"
  "using open(\"c.pr\", \"w\") as f { f.write(\"Null funct f(*a) { post(len(a)) }\\nasync Null funct g() {\\n    f(\" + \", \".join(partes[0:1000]) + \")\\n}\\nawait g()\\npost(\\\"depois\\\")\\n\") }\n"
  "Process p = os.run([sys.executable, \"b.pr\"], capture=\"live\")\n"
  "str e = \"\"\nwhile True {\n    str parte = p.read_err(65536)\n    if parte == \"\" { break }\n    e = e + parte\n}\nstr t = p.read(65536)\nint rc = p.wait()\n"
  "post(rc, t.strip() == \"\", \"pilha de valores insuficiente (3001 valores)\" in e)\n"
  "Process q = os.run([sys.executable, \"c.pr\"], capture=\"live\")\n"
  "str t2 = q.read(65536)\nint rc2 = q.wait()\n"
  "post(rc2, t2.strip())\n",
  "1 True True\n0 1000\ndepois", NULL, 0 },
/* ── recursão sem teto: três SIGSEGV da auditoria de engenharia (27/08) ──────
 * Os três eram descida recursiva sem limite. O detector de CICLO existia e não
 * bastava: ele pega `l.append(l)`, e não pega profundidade sem ciclo nem ciclo
 * que fecha acima do vetor de 256 níveis que ele registra. */
{ "parser: 100 mil parenteses aninhados (acima do teto) é erro, nao SIGSEGV",
  /* O gerador do caso é o próprio motor: 100 mil `(` na mão não cabe aqui.
   * Sem shell (`os.run` com lista), pra valer igual nos três sistemas.
   *
   * A PROFUNDIDADE é PS_PARSE_PROF_MAX + 1 (vm/ps_parser.c), e não "um número
   * que estoura a pilha". O parser para pela FOLGA MEDIDA da pilha, que segue
   * o `ulimit -s`. Com 30 mil o caso supunha 8 MB: o make 3.81 da Apple sobe o
   * limite da pilha até o teto (64 MB) e não o devolve aos filhos, e dentro do
   * `make check` do macOS 30 mil níveis CABIAM — o `--check` respondia
   * {"ok":true}, o que está certo. Um nível acima do teto de contagem é erro
   * em QUALQUER pilha: se ela não comporta tudo, dispara a folga (o freio
   * contra o SIGSEGV que originou o caso); se comporta, dispara o contador.
   * Subiu o PS_PARSE_PROF_MAX, sobe o `fundo` junto. O código 1 também é
   * conferido: imprimir a mensagem e morrer depois não passa. */
  "import os\n"
  "import sys\n"
  "fundo = 100001\n"
  "src = \"post(\" + (\"(\" * fundo) + \"1\" + (\")\" * fundo) + \")\"\n"
  "using open(\"p.pr\", \"w\") as f { f.write(src) }\n"
  "p = os.run([sys.executable, \"--check\", \"p.pr\"], capture=\"live\")\n"
  "t = \"\"\n"
  "pedaco = p.read(65536)\n"
  "while pedaco != \"\" {\n"
  "    t = t + pedaco\n"
  "    pedaco = p.read(65536)\n"
  "}\n"
  "t = t + p.read_err(65536)\n"
  "rc = p.wait()\n"
  "if rc == 1 and \"aninhada demais\" in t {\n"
  "    post(True)\n"
  "} else {\n"
  "    post(\"rc \" + str(rc) + \": \" + t[0:300])\n"
  "}\n",
  "True", NULL, 0 },
{ "parser: na BORDA do aninhamento o --check da SyntaxError, nunca MemoryError",
  /* O freio de aninhamento é pela folga da pilha, e o `--check` parseia duas
   * vezes: a compilação e a conferência, que parte de um ponto mais RASO. Na
   * borda a segunda aceitava o que a primeira recusou, o erro de sintaxe
   * sumia e saía "MemoryError: sem memoria" na linha 0 (medido: 16
   * profundidades logo abaixo do limite). O caso ACHA a borda por busca
   * binária, então vale em qualquer pilha e sistema, e confere uma JANELA em
   * volta dela: cada resposta tem que ser `ok` ou o erro de sintaxe, nunca
   * outra coisa. A borda não é exata entre processos (no Windows ela variou
   * de um `--check` pro outro: a busca parou em 5335 e o 5335 seguinte deu
   * ok), então o caso não exige que o mesmo número dê sempre o mesmo lado —
   * só que o lado recusado seja o erro de sintaxe. */
  "import os\n"
  "import sys\n"
  "funct checa(n) {\n"
  "    using open(\"b.pr\", \"w\") as f { f.write(\"post(\" + (\"(\" * n) + \"1\" + (\")\" * n) + \")\") }\n"
  "    return os.run([sys.executable, \"--check\", \"b.pr\"], capture=True)\n"
  "}\n"
  "baixo = 1\n"
  "alto = 100001\n"
  "while alto - baixo > 1 {\n"
  "    meio = (baixo + alto) // 2\n"
  "    if \"\\\"ok\\\":true\" in checa(meio) {\n"
  "        baixo = meio\n"
  "    } else {\n"
  "        alto = meio\n"
  "    }\n"
  "}\n"
  "ruins = []\n"
  "k = alto - 20\n"
  "while k <= alto + 20 {\n"
  "    t = checa(k)\n"
  "    if not (\"\\\"ok\\\":true\" in t) and not (\"SyntaxError\" in t and \"aninhada demais\" in t) {\n"
  "        addEnd(ruins, str(k) + \": \" + t[0:150])\n"
  "    }\n"
  "    k = k + 1\n"
  "}\n"
  "post(ruins)\n",
  "[]", NULL, 0 },
{ "str() de estrutura profunda trunca, nao mata",
  "x = []\n"
  "i = 0\n"
  "while (i < 100000) {\n"
  "    x = [x]\n"
  "    i = i + 1\n"
  "}\n"
  "post(len(str(x)) > 0)\n", "True", NULL, 0 },
{ "ciclo que fecha ACIMA de 256 niveis nao mata",
  /* O detector só registrava os 256 primeiros níveis, mas o contador seguia
   * subindo: um ciclo fechando em 300 não existia pra ele. */
  "raiz = []\n"
  "x = raiz\n"
  "alvo = Null\n"
  "i = 0\n"
  "while (i < 400) {\n"
  "    novo = []\n"
  "    addEnd(x, novo)\n"
  "    x = novo\n"
  "    if (i == 300) {\n"
  "        alvo = novo\n"
  "    }\n"
  "    i = i + 1\n"
  "}\n"
  "addEnd(x, alvo)\n"
  "post(len(str(raiz)) > 0)\n", "True", NULL, 0 },
/* ── recursão que passa pelo C: a folga da pilha do C, não o teto de frames ──
 * Cada callback vindo do C (map, filter, decorador), cada retomada de gerador e
 * cada import aninham um `vm_executa_base` na pilha do C gastando UM frame da
 * VM: o teto de frames nunca chegava e o processo morria com SIGSEGV. A
 * recursão direta dava RecursionError; estas davam sinal (revisão de 236eba8,
 * 2026-09-14). */
{ "variadico: recursao via callback do C (map) e RecursionError, nao morte por sinal",
  "funct f(x) {\n    return map([x], f)\n}\npost(f(1))\n",
  "", "RecursionError: maximum recursion depth exceeded", 1 },
{ "variadico: recursao via callback do C (filter) e RecursionError, nao morte por sinal",
  "funct f(x) {\n    return filter([x], f)\n}\npost(f(1))\n",
  "", "RecursionError: maximum recursion depth exceeded", 1 },
{ "variadico: recursao via decorador e RecursionError, nao morte por sinal",
  "funct d(f) {\n    @d\n    funct g() {\n        return 1\n    }\n    return g\n}\n@d\nfunct h() {\n    return 2\n}\npost(h())\n",
  "", "RecursionError: maximum recursion depth exceeded", 1 },
{ "gerador que consome a si mesmo recursivamente e RecursionError",
  "funct g(n) {\n    for each x in g(n + 1) {\n        yield x\n    }\n}\npost(list(g(0)))\n",
  "", "RecursionError: maximum recursion depth exceeded", 1 },
{ "RecursionError que passa pelo C e capturavel",
  "funct f(x) {\n    return map([x], f)\n}\ntry {\n    f(1)\n} catch (RecursionError e) {\n    post(\"pegou\")\n}\npost(\"segue\")\n",
  "pegou\nsegue", NULL, 0 },
{ "recursao via map dentro de async e RecursionError na pilha menor da fibra",
  "funct f(x) {\n    return map([x], f)\n}\nasync funct t() {\n    return f(1)\n}\npost(await t())\n",
  "", "RecursionError: maximum recursion depth exceeded", 1 },
/* Contrapeso: a folga medida não pode recusar aninhamento real dentro da
 * fibra. Cada nível custa ~13 KB da pilha do C dela (`-fstack-usage`), e com
 * 128 KB cabiam 8 — o 9º já era RecursionError, enquanto no programa principal
 * cabem 601. Por isso FIB_CSTACK subiu pra 256 KB: 8 níveis passam com folga. */
{ "oito callbacks aninhados dentro de async passam (a folga da fibra nao e limite de uso)",
  "funct nivel(n) {\n    if n <= 0 {\n        return 0\n    }\n    return map([n - 1], nivel)[0]\n}\n"
  "async funct t() {\n    return nivel(8)\n}\npost(await t())\n",
  "0", NULL, 0 },
{ "tres map aninhados dentro de async nao dao RecursionError falso",
  "async funct t() {\n    return map([1], funct(x) {\n        return map([x], funct(y) {\n            return map([y], funct(z) {\n                return z\n            })\n        })\n    })\n}\npost(await t())\n",
  "[[[1]]]", NULL, 0 },

{ "aninhamento LEGITIMO continua passando",
  /* o teto não pode virar limite de uso real: 60 blocos e 500 parênteses */
  "x = 0\n"
  "if (x == 0) { if (x == 0) { if (x == 0) { if (x == 0) {\n"
  "    post(\"fundo\")\n"
  "} } } }\n", "fundo", NULL, 0 },

{ "imprime lista que contém a si mesma",
  "l = [1, 2]\nl.append(l)\npost(l)\n",
  "[1, 2, [...]]", NULL, 0 },

{ "imprime dict que contém a si mesmo",
  "d = { \"a\": 1 }\nd[\"eu\"] = d\npost(d)\n",
  "{'a': 1, 'eu': {...}}", NULL, 0 },

{ "imprime ciclo indireto (lista dentro de lista)",
  "t = [1]\nl = [t]\nt.append(l)\npost(t)\n",
  "[1, [[...]]]", NULL, 0 },

/* A igualdade não desce pela pilha do C (lote 7b): o par que já está sendo
 * comparado conta como igual, então as duas têm a mesma forma infinita. */
{ "compara estruturas mutuamente recursivas",
  "a = [1]\nb = [1]\na.append(b)\nb.append(a)\npost(a == b)\n",
  "True", NULL, 0 },

{ "contains em estrutura mutuamente recursiva",
  "a = [1]\nb = [1]\na.append(b)\nb.append(a)\npost(a.contains(b))\n",
  "True", NULL, 0 },

{ "INT64_MIN % -1 (era SIGFPE)",
  "post(-9223372036854775808 % -1)\n",
  "0", NULL, 0 },

{ "zfill com largura de 64 bits (era OOM da máquina)",
  "post(\"a\".zfill(9223372036854775807))\n",
  "", "memória insuficiente: zfill() pediu largura", 1 },

{ "ljust com largura absurda",
  "post(\"a\".ljust(9223372036854775807))\n",
  "", "memória insuficiente: ljust() pediu largura", 1 },

{ "rjust com largura absurda",
  "post(\"a\".rjust(9223372036854775807))\n",
  "", "memória insuficiente: rjust() pediu largura", 1 },

{ "center com largura absurda",
  "post(\"a\".center(9223372036854775807))\n",
  "", "memória insuficiente: center() pediu largura", 1 },

{ "largura normal continua funcionando",
  "post(\"a\".zfill(5))\npost(\"ab\".ljust(5, \"-\"))\n"
  "post(\"ab\".center(6, \".\"))\npost(\"7\".rjust(3, \"0\"))\n",
  "0000a\nab---\n..ab..\n007", NULL, 0 },

/* ── pilha de FIBRA: a página de guarda, cobrada sem subir servidor ──────────
 * A pilha C da fibra virou `mmap` com uma página `PROT_NONE` embaixo. Antes era
 * `malloc`, e recursão funda dentro de uma fibra passava por cima do heap
 * vizinho — corrupção silenciosa, que só aparecia longe dali.
 *
 * O caso não precisa do jinker: `fib_pega_async` usa exatamente a mesma pilha,
 * então uma `async funct` alcança o mesmo código. Sem estes dois casos a
 * guarda não é cobrada por portão nenhum, porque o e2e do jinker é alvo
 * separado e não roda em todo push. */
{ "recursao funda DENTRO de fibra async é erro, nao corrupcao",
  "funct fundo(n) {\n"
  "    if n <= 0 {\n"
  "        return 0\n"
  "    }\n"
  "    return 1 + fundo(n - 1)\n"
  "}\n"
  "async funct dentro() {\n"
  "    return fundo(100000)\n"
  "}\n"
  "try {\n"
  "    f = dentro()\n"
  "    _r = await f\n"
  "    post(\"passou sem reclamar\")\n"
  "} catch (e) {\n"
  "    post(\"recusou\")\n"
  "}\n", "recusou", NULL, 0 },
{ "pool de fibra cheio é erro limpo, nao morte",
  /* O pool é `FIB_HARD` = 8192 fibras concorrentes; passar disso tem que
   * LEVANTAR, não cair.
   *
   * A forma importa: com `await` ENCADEADO o caso levava 27 s (e estourava o
   * timeout de 20 s do runner), porque cada nível cria uma fibra e espera. Com
   * as fibras criadas de uma vez, o mesmo estouro sai em 0,76 s — 20x mais
   * rápido pra provar exatamente a mesma coisa. Teste lento é teste que alguém
   * acaba desligando.
   *
   * O `sleep` dentro da tarefa entrou em 2026-09-22, quando `async funct`
   * passou a RODAR na chamada: sem ponto de cedência cada tarefa termina na
   * hora e devolve o slot, o pool nunca enche e o caso deixava de exercitar o
   * teto. Com o `sleep` as 9000 ficam suspensas, que é a situação que o teto
   * existe pra cobrir. */
  "async funct f(n) {\n"
  "    sleep(5)\n"
  "    return n\n"
  "}\n"
  "fs = []\n"
  "i = 0\n"
  "try {\n"
  "    while i < 9000 {\n"
  "        addEnd(fs, f(i))\n"
  "        i = i + 1\n"
  "    }\n"
  "    post(len(gather(fs)))\n"
  "} catch (e) {\n"
  "    post(\"recusou\")\n"
  "}\n", "recusou", NULL, 0 },

/* ── aninhamento LEGÍTIMO: o teto não pode virar limite de uso ───────────────
 * Contrapeso dos casos de teto acima. Se alguém apertar `PS_PARSE_PROF_MAX` pra
 * calar um fuzzer, estes reprovam — que é a única defesa contra o conserto
 * preguiçoso de pôr um número menor. */
/* ── gravacao que falha NAO pode virar sucesso ──────────────────────────────
 * Sete sitios do motor faziam `fwrite(...); fclose(f);` sem olhar nenhum dos
 * dois retornos. `os.writeFile("/dev/full", ...)` devolvia o caminho com rc=0;
 * `PoolFile.save`, `Response.save`, `qrcode.save`, `manpu.write` e
 * `upload.save` faziam o mesmo. /dev/full sempre aceita o `fopen` e sempre
 * recusa a escrita, entao ele e o caso.
 *
 * O `fclose` importa tanto quanto o `fwrite`: o buffer da libc so vai pro disco
 * no flush, entao ENOSPC costuma aparecer AO FECHAR.
 *
 * Aceitar o `fopen` e recusar a gravacao, sem privilegio, so o /dev/full do
 * Linux faz: no Windows "/dev/full" e um caminho comum (\dev\full no drive
 * atual, que o writeFile cria e grava) e no macOS o /dev recusa o arquivo ja
 * no `fopen`. Os do /dev/full valem so no Linux, e sao a unica guarda do ramo
 * `fwrite`/`fclose`. Os irmaos com "pai que e arquivo" valem nos tres
 * sistemas: a gravacao falha no `fopen` (ENOTDIR no Linux e no macOS, ENOENT
 * no Windows), o mesmo contrato ("gravacao que falha levanta") pelo ramo do
 * `fopen`. */
{ "os.writeFile que falha levanta",
  "import os\n"
  "try {\n"
  "    os.writeFile(\"/dev/full\", \"x\" * 200000)\n"
  "    post(\"nao levantou\")\n"
  "} catch (e) {\n"
  "    post(\"levantou\")\n"
  "}\n", "levantou", NULL, 0, NULL, 0, "linux" },
{ "qrcode.save que falha levanta",
  "import qrcode\n"
  "try {\n"
  "    qrcode.make(\"x\").save(\"/dev/full\")\n"
  "    post(\"nao levantou\")\n"
  "} catch (e) {\n"
  "    post(\"levantou\")\n"
  "}\n", "levantou", NULL, 0, NULL, 0, "linux" },
{ "os.writeFile com pai que e arquivo levanta",
  "import os\n"
  "os.writeFile(\"pai\", \"sou arquivo\")\n"
  "try {\n"
  "    os.writeFile(\"pai/filho.txt\", \"x\" * 200000)\n"
  "    post(\"nao levantou\")\n"
  "} catch (e) {\n"
  "    post(\"levantou\")\n"
  "}\n", "levantou", NULL, 0 },
{ "qrcode.save com pai que e arquivo levanta",
  "import os\n"
  "import qrcode\n"
  "os.writeFile(\"pai\", \"sou arquivo\")\n"
  "try {\n"
  "    qrcode.make(\"x\").save(\"pai/q.png\")\n"
  "    post(\"nao levantou\")\n"
  "} catch (e) {\n"
  "    post(\"levantou\")\n"
  "}\n", "levantou", NULL, 0 },
{ "gravacao normal continua funcionando",
  "import os\n"
  "os.writeFile(\"n.txt\", \"dado\")\n"
  "post(os.readFile(\"n.txt\"))\n", "dado", NULL, 0 },

/* ── largura de format sem teto: TRAVAVA o processo ─────────────────────────
 * `largura = largura * 10 + digito` transbordava o int em silencio, e o laco
 * de preenchimento escrevia byte a byte ate a memoria acabar. O processo nao
 * morria: ficava comendo RAM. O CPython responde MemoryError. */
{ "largura absurda em format nao trava",
  "post(\"{:99999999999d}\".format(1))\n", "",
  "MemoryError: memória insuficiente: format() pediu largura 99999999999", 1 },
{ "precisao absurda em format nao trava",
  "post(\"{:.99999999999f}\".format(1.5))\n", "",
  "MemoryError: memória insuficiente: format() pediu precisao 99999999999", 1 },
{ "format normal continua funcionando",
  "post(\"{:>8d}\".format(42))\n"
  "post(\"{:.2f}\".format(3.14159))\n"
  "post(\"{:08.3f}\".format(3.14159))\n",
  "      42\n3.14\n0003.142", NULL, 0 },

/* ── copia que falha NAO pode apagar a origem ───────────────────────────────
 * `copia_arquivo` nao conferia `fwrite` nem `fclose`: gravacao que falhava era
 * ignorada, a funcao dizia "copiou" e o `os.move` dava unlink na ORIGEM.
 * Perda de dado em silencio, sem erro nenhum. /dev/full sempre aceita o
 * `fopen` e sempre recusa a escrita, entao ele e o caso — so no Linux (ver
 * acima). Os irmaos com "pai que e arquivo" valem nos tres sistemas: o rename
 * e a copia de reserva falham, e a origem fica. */
{ "os.move que falha mantem a origem",
  "import os\n"
  "using open(\"o.txt\", \"w\") as f { f.write(\"dado\") }\n"
  "houve_erro = False\n"
  "try { os.move(\"o.txt\", \"/dev/full\") } catch (e) { houve_erro = True }\n"
  "post(houve_erro, os.exists(\"o.txt\"))\n", "True True", NULL, 0, NULL, 0, "linux" },
{ "os.copy que falha nao deixa destino pela metade",
  "import os\n"
  "using open(\"o2.txt\", \"w\") as f { f.write(\"dado\") }\n"
  "houve_erro = False\n"
  "try { os.copy(\"o2.txt\", \"/dev/full\") } catch (e) { houve_erro = True }\n"
  "post(houve_erro, os.exists(\"o2.txt\"))\n", "True True", NULL, 0, NULL, 0, "linux" },
{ "os.move pra pai que e arquivo mantem a origem",
  "import os\n"
  "using open(\"o.txt\", \"w\") as f { f.write(\"dado\") }\n"
  "os.writeFile(\"pai\", \"sou arquivo\")\n"
  "houve_erro = False\n"
  "try { os.move(\"o.txt\", \"pai/o.txt\") } catch (e) { houve_erro = True }\n"
  "post(houve_erro, os.exists(\"o.txt\"))\n", "True True", NULL, 0 },
{ "os.copy pra pai que e arquivo nao cria destino",
  "import os\n"
  "using open(\"o2.txt\", \"w\") as f { f.write(\"dado\") }\n"
  "os.writeFile(\"pai\", \"sou arquivo\")\n"
  "houve_erro = False\n"
  "try { os.copy(\"o2.txt\", \"pai/o2.txt\") } catch (e) { houve_erro = True }\n"
  "post(houve_erro, os.exists(\"o2.txt\"), os.exists(\"pai/o2.txt\"))\n", "True True False", NULL, 0 },
/* A copia que falhou apagava a ENTRADA do destino sem olhar o que era: um
 * link pra /dev/full sumia (e, como root, o proprio /dev/full). So sai a
 * entrada que e o arquivo comum que a copia escreveu. */
{ "copy que falha nao apaga destino que nao e arquivo comum",
  "import os\n"
  "os.run([\"ln\", \"-s\", \"/dev/full\", \"cheio\"], capture=True)\n"
  "using open(\"o3.txt\", \"w\") as f { f.write(\"dado\") }\n"
  "try { os.copy(\"o3.txt\", \"cheio\") } catch (e) { post(\"levantou\") }\n"
  "post(os.exists(\"cheio\"))\n", "levantou\nTrue", NULL, 0, NULL, 0, "linux" },
/* Mover e copiar E apagar a origem. Origem que nao sai era ignorada: o move
 * dizia "ok" e o arquivo ficava nos dois lugares. Cada sistema segura a
 * origem do seu jeito, e o jeito tem que valer TAMBEM pro root (a suite roda
 * como root na VPS, e ali a pasta sem escrita nao segurava nada: o root
 * renomeia e apaga mesmo assim, e o caso reprovava com "False False"):
 * - Linux: um arquivo do /proc. Le-se, mas ninguem o apaga (EACCES pro
 *   usuario, EPERM pro root), e o rename cai no copiar-e-apagar (EXDEV);
 * - macOS: pasta sem escrita (o rename e o unlink falham);
 * - Windows: a origem aberta (o rename e o apagar recusam o arquivo em uso).
 * O ramo por sistema e so o MEIO de segurar a origem; a propriedade conferida
 * e a mesma. */
{ "os.move que nao consegue apagar a origem levanta",
  "import os\n"
  "import sys\n"
  "origem = \"ro/f.txt\"\n"
  "aberto = Null\n"
  "if sys.platform() == \"linux\" {\n"
  "    origem = \"/proc/version\"\n"
  "} else {\n"
  "    os.mkdir(\"ro\")\n"
  "    os.writeFile(origem, \"dado\")\n"
  "    if sys.platform() == \"windows\" {\n"
  "        aberto = open(origem)\n"
  "    } else {\n"
  "        os.run([\"chmod\", \"555\", \"ro\"], capture=True)\n"
  "    }\n"
  "}\n"
  "houve_erro = False\n"
  "try { os.move(origem, \"dest.txt\") } catch (e) { houve_erro = True }\n"
  "if sys.platform() == \"windows\" {\n"
  "    aberto.close()\n"
  "} elif sys.platform() != \"linux\" {\n"
  "    os.run([\"chmod\", \"755\", \"ro\"], capture=True)\n"
  "}\n"
  "post(houve_erro, os.exists(origem))\n", "True True", NULL, 0 },
/* O mesmo defeito no `delete()`: o retorno do unlink era ignorado e ele
 * devolvia True com o arquivo no lugar. Ja apagado continua nao sendo erro; o
 * que existe e nao sai levanta. A origem e segurada como no caso do move. */
{ "PoolFile.delete que nao consegue apagar levanta PermissionError",
  "import os\n"
  "import sys\n"
  "alvo = \"ro/f.txt\"\n"
  "aberto = Null\n"
  "if sys.platform() == \"linux\" {\n"
  "    alvo = \"/proc/version\"\n"
  "} else {\n"
  "    os.mkdir(\"ro\")\n"
  "    os.writeFile(alvo, \"dado\")\n"
  "    if sys.platform() == \"windows\" {\n"
  "        aberto = open(alvo)\n"
  "    } else {\n"
  "        os.run([\"chmod\", \"555\", \"ro\"], capture=True)\n"
  "    }\n"
  "}\n"
  "p = os.loadFile(alvo, mode=\"rb\")\n"
  "houve_erro = False\n"
  "try { p.delete() } catch (PermissionError e) { houve_erro = True }\n"
  "if sys.platform() == \"windows\" {\n"
  "    aberto.close()\n"
  "} elif sys.platform() != \"linux\" {\n"
  "    os.run([\"chmod\", \"755\", \"ro\"], capture=True)\n"
  "}\n"
  "post(houve_erro, os.exists(alvo))\n", "True True", NULL, 0 },
{ "PoolFile.delete de arquivo que ja nao existe nao e erro",
  "import os\n"
  "os.writeFile(\"some.txt\", \"x\")\n"
  "p = os.loadFile(\"some.txt\", mode=\"rb\")\n"
  "os.rename(\"some.txt\", \"outro.txt\")\n"
  "post(p.delete(), os.exists(\"some.txt\"), os.exists(\"outro.txt\"))\n", "True False True", NULL, 0 },
/* CAMINHO ABSOLUTO pela regra do sistema. Era `caminho[0] == '/'` em quatro
 * lugares (os.loadFile/pathFile/pathFolder, `import "..."`, o `file=` do
 * request e o `static_folder` do jinker), e no Windows o absoluto `C:\...`
 * caia na busca pelo nome e nao achava. `os.cwd()` da o absoluto de cada
 * sistema; no Linux e no macOS o caso passa igual antes e depois. */
{ "os.loadFile e os.pathFile com caminho absoluto da pasta corrente",
  "import os\n"
  "os.writeFile(\"abs_alvo.txt\", \"dado\")\n"
  "p = os.cwd() + \"/abs_alvo.txt\"\n"
  "post(os.loadFile(p, mode=\"r\"))\n"
  "post(os.pathFile(p).endswith(\"abs_alvo.txt\"))\n",
  "dado\nTrue", NULL, 0 },
{ "import com caminho absoluto acha o modulo (no Windows, C:/...)",
  "import os\n"
  "import sys\n"
  "os.writeFile(\"mod_abs.pr\", \"funct oi() {\\n    return 42\\n}\\n\")\n"
  "raiz = os.cwd().replace(\"\\\\\", \"/\")\n"
  "os.writeFile(\"principal.pr\", \"import \\\"\" + raiz + \"/mod_abs.pr\\\"\\npost(mod_abs.oi())\\n\")\n"
  "post(os.run([sys.executable, \"principal.pr\"], capture=True))\n",
  "42", NULL, 0 },
/* O rename do Windows (o do CRT) nao substituia um destino que ja existe, e o
 * mesmo `os.rename` que no Linux e no macOS troca o arquivo levantava erro. */
{ "os.rename por cima de um arquivo que existe substitui",
  "import os\n"
  "os.writeFile(\"n.txt\", \"novo\")\n"
  "os.writeFile(\"v.txt\", \"velho\")\n"
  "os.rename(\"n.txt\", \"v.txt\")\n"
  "post(os.readFile(\"v.txt\"), os.exists(\"n.txt\"))\n", "novo False", NULL, 0 },
/* `os.rmdir(force=True)` seguia link: o stat/opendir olhavam o destino, e
 * descer apagava o conteudo DELE — com `arvore/link -> ../alvo`, sumia
 * alvo/precioso.txt. Link (simbolico; no Windows a juncao de pasta) sai como
 * link, sem descer, como no `rm -rf`. O ramo por sistema e so o jeito de
 * criar o link. */
{ "os.rmdir force nao segue link e nao apaga o destino",
  "import os\n"
  "import sys\n"
  "os.mkdir(\"alvo\")\n"
  "os.writeFile(\"alvo/precioso.txt\", \"dado\")\n"
  "os.mkdir(\"arvore\")\n"
  "if sys.platform() == \"windows\" {\n"
  "    os.cmd(\"mklink /J \\\"\" + os.cwd() + \"\\\\arvore\\\\link\\\" \\\"\" + os.cwd() + \"\\\\alvo\\\"\", True)\n"
  "} else {\n"
  "    os.run([\"ln\", \"-s\", \"../alvo\", \"arvore/link\"], capture=True)\n"
  "}\n"
  "post(len(os.ls(\"arvore\")))\n"
  "os.rmdir(\"arvore\", force=True)\n"
  "post(os.exists(\"arvore\"), os.readFile(\"alvo/precioso.txt\"))\n", "1\nFalse dado", NULL, 0 },

{ "500 parenteses aninhados sao LEGITIMOS",
  /* Sem shell (`os.run` com lista), como o dos 100 mil: a aspa simples era
   * sintaxe do sh, e o cmd.exe a toma como parte do nome do programa.
   * `capture=True` devolve o stdout, ou o stderr se nao houve stdout. */
  "import os\n"
  "import sys\n"
  "src = \"post(\" + (\"(\" * 500) + \"1\" + (\")\" * 500) + \")\"\n"
  "using open(\"leg.pr\", \"w\") as f { f.write(src) }\n"
  "post(os.run([sys.executable, \"leg.pr\"], capture=True))\n", "1", NULL, 0 },
{ "estrutura 200 niveis funda imprime inteira",
  "x = [\"fundo\"]\n"
  "i = 0\n"
  "while (i < 200) {\n"
  "    x = [x]\n"
  "    i = i + 1\n"
  "}\n"
  "post(\"fundo\" in str(x))\n", "True", NULL, 0 },
};
const int NC_CRASH = N_CASOS(CASOS_CRASH);
