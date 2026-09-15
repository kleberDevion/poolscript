/*
 * Fila de trabalho, e hoje ela está VAZIA: os 29 achados da caça de
 * 2026-08-25 foram todos corrigidos. Eles continuam aqui como regressão —
 * cada um trava um comportamento que já é o certo.
 *
 * O cabeçalho antigo dizia "falha até o motor fazer certo" enquanto os 29
 * passavam. A fila tinha esvaziado sozinha e o arquivo continuava anunciando
 * dívida que não existia; pior, um caso que quebrasse de verdade ficaria
 * indistinguível de pendência velha.
 *
 * COMO ADICIONAR UMA PENDÊNCIA: escreva o caso com o comportamento CERTO e
 * ponha `1` no último campo (`pendente`). Aí ele TEM que falhar — e no dia em
 * que o motor acertar, o runner acusa "JA FUNCIONA" e manda tirar daqui. Não
 * dá pra esquecer nem pra esvaziar em silêncio.
 *
 * Onde o interpretador (que não existe mais) e a VM discordavam, a decisão
 * está no comentário de cada bloco.
 */
#include "ps_teste.h"

const Caso CASOS_PENDENTES[] = {
/* ── igualdade e unário ──────────────────────────────────────────────────
 * Este caso afirmava a igualdade "nullish": `null == 0` True, e portanto
 * `null == false` também, senão a transitividade quebrava DENTRO do motor.
 * O raciocínio estava certo e a PREMISSA é que caiu — em 28/08 o dono decidiu
 * que Null é igual só a Null, como o `None` do Python (I1/I2/I3).
 *
 * O que derrubou a premissa foi o custo prático: `if x == 0` entrava com x
 * valendo Null, e todo teste escrito como `if os.cmd(...) != 0` virava falso
 * verde PERMANENTE, porque `Null != 0` era False. Isso já escondeu defeito
 * nesta suíte. */
{ "Null é igual só a Null",
  "post(false == null, null == false, true == null, null == null)\n",
  "False False False True", NULL, 0 },

/* bool participa da aritmética em todo lugar (`true + 1` == 2); o menos
 * unário não podia ser a exceção. */
{ "menos unário em bool",
  "x = true\npost(-x, -false)\n", "-1 0", NULL, 0 },

/* ── Unicode ─────────────────────────────────────────────────────────────
 * A tabela de caixa da VM só cobria ASCII + Latin-1/Ext-A. Grego e cirílico
 * passavam intactos, e `isalpha` dizia False pra letra. */
{ "lower/upper em grego e cirílico",
  "post(\"Ω\".lower(), \"б\".upper(), \"б\".isalpha())\n", "ω Б True", NULL, 0 },
{ "dígitos não-ASCII",
  "post(\"²\".isdigit(), \"٣\".isdecimal(), \"½\".isnumeric())\n", "True True True", NULL, 0 },
{ "string vazia é ascii e imprimível",
  "post(\"\".isascii(), \"\".isprintable())\n", "True True", NULL, 0 },
{ "startswith/endswith com tupla de opções",
  "post(\"olá\".startswith((\"x\", \"o\")), \"olá\".endswith((\"z\", \"á\")))\n", "True True", NULL, 0 },

/* ── coleções ───────────────────────────────────────────────────────────── */
{ "index com posição inicial",
  "l = [1, 2, 3, 2]\npost(l.index(2), l.index(2, 2))\n", "1 3", NULL, 0 },
{ "índice booleano vale 0/1",
  "l = [10, 20]\npost(l[true], l[false], \"ab\"[true])\n", "20 10 b", NULL, 0 },
{ "limite de fatia acima do int64 satura",
  "s = \"abcdef\"\npost(s[999999999999999999999:], s[:999999999999999999999])\n", " abcdef", NULL, 0 },

/* ── herança / Entity ───────────────────────────────────────────────────── */
{ "base() em Entity sem pai é erro",
  "Entity B() {\n"
  "    funct __init__(self) {\n"
  "        base()\n"
  "        self.m = 2\n"
  "    }\n"
  "}\n"
  "b = B()\n",
  "", "base()", 2 },
{ "base() com argumento nomeado",
  "Entity A() {\n"
  "    funct __init__(self, x) {\n"
  "        self.x = x\n"
  "    }\n"
  "}\n"
  "Entity B(A) {\n"
  "    funct __init__(self) {\n"
  "        base(x=5)\n"
  "    }\n"
  "}\n"
  "b = B()\n"
  "post(b.x)\n",
  "5", NULL, 0 },
{ "campo private do pai visível no método do pai",
  "Entity P() {\n"
  "    private s: int\n"
  "    funct mostra(self) {\n"
  "        return self.s\n"
  "    }\n"
  "}\n"
  "Entity F(P) {\n"
  "    private s: int\n"
  "}\n"
  "f = F(7)\n"
  "post(f.mostra())\n",
  "7", NULL, 0 },
{ "@dataentity aponta o parâmetro certo que falta",
  "@dataentity\n"
  "Entity P() {\n"
  "    a: int\n"
  "    b: int = 2\n"
  "    c: int\n"
  "}\n"
  "p = P(1)\n",
  "", "'c'", 2 },

/* ── string com byte NUL ─────────────────────────────────────────────────
 * `\x00` truncava tudo depois dele: perda silenciosa de dado. */
{ "NUL no meio da string não trunca",
  "post(len(\"a\\x00b\"))\n", "3", NULL, 0 },

/* ── números ────────────────────────────────────────────────────────────── */
{ "int() de infinito é erro, não INT64_MIN",
  "post(int(flo(\"inf\")))\n", "", "OverflowError: cannot convert flo infinity to integer", 1 },
{ "int() de NaN é erro",
  "post(int(flo(\"nan\")))\n", "", "NaN", 1 },

/* ── funções de ordem superior e arquivo ─────────────────────────────────── */
{ "map confere a aridade da funct",
  "funct f(x, y) {\n"
  "    return x\n"
  "}\n"
  "post(map([1, 2], f))\n", "", "f() missing 1 required positional argument: 'y'", 1 },
{ "open() de diretório é erro",
  "f = open(\"/tmp\")\npost(f.read())\n", "", "[Errno 21] Is a directory", 1 },
{ "writelines com bytes grava os bytes",
  "p = \"/tmp/ps_teste_wl.bin\"\nusing open(p, \"wb\") as f { f.writelines([\"a\".encode()]) }\n"
  "using open(p, \"rb\") as f { post(f.read()) }\n",
  "b'a'", NULL, 0 },

/* ── encode/decode ──────────────────────────────────────────────────────── */
{ "encode respeita o encoding pedido",
  "post(\"café\".encode(\"latin-1\"))\n", "b'caf\\xe9'", NULL, 0 },
{ "decode de bytes inválidos é erro",
  "import bytes\nb = bytes.new([255, 254])\npost(b.decode())\n", "", "decode", 1 },
{ "decode com encoding inexistente é erro",
  "b = \"abc\".encode()\npost(b.decode(\"naoexiste\"))\n", "", "encoding", 1 },

/* ── import ─────────────────────────────────────────────────────────────── */
/* Um arquivo que importa a si mesmo NÃO pode entrar em recursão infinita.
 * O corpo roda duas vezes (uma como programa, outra como módulo) — é o mesmo
 * que o Python faz com `import t` dentro de `t.py`; o que se garante aqui é
 * que termina, sem laço nem estouro de pilha. */
{ "arquivo que importa a si mesmo termina",
  "import ps_auto_import\npost(\"ok\")\n", "ok\nok", NULL, 0, "ps_auto_import.ps" },

/* ── json ───────────────────────────────────────────────────────────────── */
{ "json.parse monta emoji de par surrogate",
  "import json\npost(json.parse(\"{\\\"a\\\": \\\"\\\\ud83d\\\\ude00\\\"}\"))\n",
  "{'a': '😀'}", NULL, 0 },

/* ── regex ──────────────────────────────────────────────────────────────── */
{ "sub interpreta \\n e \\t no replacement",
  "import regex\npost(len(regex.sub(\"-\", r\"\\n\", \"a-b\")))\n", "3", NULL, 0 },
{ "sub substitui \\g<1> e \\g<nome>",
  "import regex\npost(regex.sub(r\"(\\d)\", r\"[\\g<1>]\", \"a1b2\"))\n", "a[1]b[2]", NULL, 0 },
{ "split mantém os grupos de captura",
  "import regex\npost(regex.split(r\"(,)\", \"a,b,c\"))\n", "['a', ',', 'b', ',', 'c']", NULL, 0 },
{ "split com padrão vazio separa caractere a caractere",
  "import regex\npost(regex.split(\"\", \"abc\"))\n", "['', 'a', 'b', 'c', '']", NULL, 0 },

/* ── `if __name__ == "main"` como EXPRESSÃO ──────────────────────────────────
 * O guard de entrada é reconhecido pela FORMA (vira `OP_SKIP_IF_IMPORT`), não
 * avaliando a condição. As três formas do `if` (chave na mesma linha, Allman,
 * Allman com parênteses) já entram — os casos estão em `casos_linguagem.c`, no
 * bloco do ponto de entrada. O que ainda falta é a condição valer o mesmo
 * FORA do `if`: dentro dele é verdadeira, avaliada sozinha é falsa. Uma das
 * duas está errada, e quem lê o programa não tem como saber qual. */
{ "__name__ == \"main\" vale o mesmo dentro e fora do if",
  "post(__name__ == \"main\")\nif __name__ == \"main\" {\n    post(\"entrou\")\n}\n",
  "True\nentrou", NULL, 0, NULL, 1 },

/* ── async chamada por NOME ──────────────────────────────────────────────
 * `f(1)` numa `async funct` devolve um future (a fibra roda no await/gather);
 * `f(a=1)` roda o corpo INLINE e devolve o valor — o OP_CALL_KW nunca olhou
 * `eh_async`. O resultado sai certo nos casos simples, e e por isso que o
 * furo passou despercebido: so a diferenca de tipo denuncia. Medido em
 * 2026-09-14 ao fechar `*args`/`**kwarg` (a chamada por nome ganhou gerador,
 * mas a fibra guarda argumentos crus e o binding nomeado nao cabe nela). */
{ "async por nome devolve future, como a chamada posicional",
  "async funct f(a, b=2) {\n    return [a, b]\n}\npost(type(f(1)), type(f(a=1)))\n",
  "future future", NULL, 0, NULL, 1 },

/* ── revisao de 236eba8 (`*args`/`**kwarg`), 2026-09-14 ──────────────────
 * Cada caso abaixo foi medido VERMELHO contra o ./pool 15.90.35 antes de
 * entrar; a saida esperada e a que a regra do proprio motor no caminho
 * vizinho implica (a frase esta no comentario de cada um). */

/* Recursao que passa por callback do C (map, filter, decorador) empilha
 * `vm_executa_base` na pilha do C sem o teto de frames da VM: morre por
 * SIGSEGV. A recursao direta (`return f(x)`) da RecursionError, rc=1. */
{ "variadico: recursao via callback do C (map) e RecursionError, nao morte por sinal",
  "funct f(x) {\n    return map([x], f)\n}\npost(f(1))\n",
  "", "RecursionError: maximum recursion depth exceeded", 1, NULL, 1 },
{ "variadico: recursao via callback do C (filter) e RecursionError, nao morte por sinal",
  "funct f(x) {\n    return filter([x], f)\n}\npost(f(1))\n",
  "", "RecursionError: maximum recursion depth exceeded", 1, NULL, 1 },
{ "variadico: recursao via decorador e RecursionError, nao morte por sinal",
  "funct d(f) {\n    @d\n    funct g() {\n        return 1\n    }\n    return g\n}\n@d\nfunct h() {\n    return 2\n}\npost(h())\n",
  "", "RecursionError: maximum recursion depth exceeded", 1, NULL, 1 },

/* Gerador declarado como metodo: `o.conta(n=2)` (CALL_KW) cria o gerador;
 * `o.conta(2)`, `o.conta(*[2])` e `for each` (OP_CALL bound) entram no corpo
 * e dao "RuntimeError: yield fora de gerador". Esperado: o mesmo `[0, 1]` do
 * caminho por nome nos quatro. */
{ "variadico: gerador como metodo chamado pela instancia cria o gerador nos 4 caminhos",
  "Entity G() {\n    funct conta(self, n) {\n        for each i in range(n) {\n            yield i\n        }\n    }\n}\n"
  "o = G()\npost(list(o.conta(2)))\npost(list(o.conta(*[2])))\nfor each x in o.conta(2) {\n    post(x)\n}\npost(list(o.conta(n=2)))\n",
  "[0, 1]\n[0, 1]\n0\n1\n[0, 1]", NULL, 0, NULL, 1 },

/* `funct __init__(*args)` SEM self: `E(1, 2)` devolve a TUP `(1, 2)` no lugar
 * da instancia, sem erro — o slot 0 (self) e o slot do vararg coincidem e a
 * tup sobrescreve a instancia. A regra medida no vizinho `funct __init__(x)`
 * sem self ("__init__() takes 1 positional argument but 2 were given" pra
 * `E(1)`) e que a instancia ENTRA como 1o posicional; com `*args` ela cai na
 * tup e `E(...)` devolve a instancia. Se o dono decidir que `__init__` sem
 * `self` e erro na declaracao (doc 7.2.1: `funct __init__(self, …)`), troque a
 * expectativa — hoje nem o vizinho `__init__(x)` e recusado na declaracao. */
{ "variadico: __init__(*args) sem self devolve a instancia, nao a tup dos argumentos",
  "Entity E() {\n    funct __init__(*args) {\n        post(len(args), type(args[0]))\n    }\n}\ne = E(1, 2)\npost(type(e))\n",
  "3 E\nE", NULL, 0, NULL, 1 },

/* `async funct` aninhada que captura variavel do escopo de fora: a fibra
 * nasce sem as celulas da closure e o await falha com "RuntimeError: upvalue
 * fora da faixa (bug do compilador)". A `funct` sincrona gemea devolve
 * `[1, 10]`. */
{ "variadico: async funct aninhada com captura roda no await como a sincrona",
  "funct externa(v) {\n    async funct interna(a) {\n        return [a, v]\n    }\n    return interna\n}\nf = externa(10)\npost(await f(1))\n",
  "[1, 10]", NULL, 0, NULL, 1 },

/* `async funct` como metodo de instancia roda INLINE e devolve o valor nos
 * quatro caminhos (posicional, `*lista`, nome, bound solto) — so a funct
 * solta e a `@static` viram fibra. Doc 6.8: "uma funct marcada async NAO roda
 * na chamada: devolve um future". Irmao do caso `async por nome` acima. */
{ "variadico: async como metodo de instancia devolve future nos 4 caminhos de chamada",
  "Entity C() {\n    async funct m(self, a) {\n        return a\n    }\n}\nc = C()\nf = c.m\n"
  "post(type(c.m(1)), type(c.m(*[1])), type(c.m(a=1)), type(f(1)))\n",
  "future future future future", NULL, 0, NULL, 1 },

/* `@static funct s(self)` com um argumento a mais: OP_CALL, CALL_EX e CALL_KW
 * escondem o buraco do self ("takes 0 positional arguments but 1 was
 * given"); o callback do C e o decorador contam o buraco ("takes 1 ... but 2
 * were given"). Doc 14.1: o self "e dropado" na chamada estatica. */
{ "variadico: callback do C esconde o buraco do @static na frase de aridade, como o OP_CALL",
  "Entity C() {\n    @static\n    funct s(self) {\n        return 1\n    }\n}\npost(map([7], C.s))\n",
  "", "TypeError: s() takes 0 positional arguments but 1 was given", 1, NULL, 1 },
{ "variadico: decorador esconde o buraco do @static na frase de aridade, como o OP_CALL",
  "Entity C() {\n    @static\n    funct s(self) {\n        return 1\n    }\n}\n@C.s\nfunct h() {\n    return 1\n}\n",
  "", "TypeError: s() takes 0 positional arguments but 1 was given", 1, NULL, 1 },

/* O atalho do `for each` sobre `range(...)` (vm/ps_compiler.c, `usa_range`)
 * ignora a estrela e passa a lista inteira como limite: "TypeError: 'list'
 * object cannot be interpreted as an integer". `list(range(*l))` espalha. */
{ "espalhar: for each em range(*l) espalha como list(range(*l))",
  "l = [1, 4]\npost(list(range(*l)))\nfor each i in range(*l) {\n    post(i)\n}\n",
  "[1, 2, 3]\n1\n2\n3", NULL, 0, NULL, 1 },

/* `liga_args` tem `int marcado[256]` (vm/poolscript_vm.c): funct com 257
 * parametros fixos compila, passa no `--check` e TODA chamada — inclusive a
 * posicional, que rodava antes de 236eba8 — da "TypeError: f() tem parametros
 * demais (maximo 256)" na linha da CHAMADA. O limite e da declaracao, entao
 * a recusa tem que ser la: SyntaxError, rc=2, `--check` reprova. A frase e a
 * do motor com o prefixo dos erros de declaracao; ajuste se o dono a mudar. */
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
  "", "SyntaxError: f() tem parametros demais (maximo 256)", 2, NULL, 1 },

/* `nonnull static funct f(self, a)`: na chamada estatica o self e dropado
 * (slot UNSET) e o `nonnull` acusa esse buraco como parametro Null —
 * "RuntimeError: nonnull: parametro 'self' em 'f' nao pode ser Null". Sem
 * `nonnull` a mesma chamada devolve 5. */
{ "variadico: nonnull + static com self nao acusa o buraco do self como Null",
  "Entity C() {\n    nonnull static funct f(self, a) {\n        return a\n    }\n    static funct g(self, a) {\n        return a\n    }\n}\n"
  "post(C.g(5))\npost(C.f(5))\n",
  "5\n5", NULL, 0, NULL, 1 },

/* Tipo escrito ANTES da estrela: `int *args` da "'int' e palavra reservada da
 * linguagem e nao pode ser usada como nome de parametro" e `String *args` da
 * "faltou ')' na declaracao da funct" — nenhum diz o conserto. A frase
 * dedicada ja existe pra `*int args` (caso 'variadico: tipo em estrela e
 * recusado' em casos_linguagem.c) e o parser monta o nome com `%s%s %s`. */
{ "variadico: tipo antes da estrela (int *args) recebe a mensagem dedicada do tipo em estrela",
  "funct f(int *args) {\n    return args\n}\n",
  "", "SyntaxError: parametro `*int args` nao aceita tipo: `*args` e sempre tup e `**kwarg` sempre dict — escreva `*args`", 2, NULL, 1 },
{ "variadico: tipo antes da estrela (String *args) recebe a mensagem dedicada do tipo em estrela",
  "funct f(String *args) {\n    return args\n}\n",
  "", "SyntaxError: parametro `*String args` nao aceita tipo: `*args` e sempre tup e `**kwarg` sempre dict — escreva `*args`", 2, NULL, 1 },

};
const int NC_PENDENTES = N_CASOS(CASOS_PENDENTES);
