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
 * A linguagem já trata `null == 0` como True (igualdade "nullish"). Como
 * `false == 0` também é True, `null == false` TEM que ser True, senão a
 * igualdade perde a transitividade dentro do próprio motor. */
{ "null == false é True",
  "post(false == null, null == false, true == null)\n", "True True False", NULL, 0 },

/* bool participa da aritmética em todo lugar (`true + 1` == 2); o menos
 * unário não podia ser a exceção. */
{ "menos unário em bool",
  "x = true\npost(-x, -false)\n", "-1 0", NULL, 0 },

/* ── Unicode ─────────────────────────────────────────────────────────────
 * A tabela de caixa da VM só cobria ASCII + Latin-1/Ext-A. Grego e cirílico
 * passavam intactos, e `isalpha` dizia False pra letra. */
{ "lower/upper em grego e cirílico",
  "post(\"Ω\".lower(), \"б\".upper(), \"б\".isalpha())\n", "ω Б True", NULL, 0 },
{ "ß maiúsculo expande para SS",
  "post(\"ß\".upper())\n", "SS", NULL, 0 },
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
  "    action __init__(self) {\n"
  "        base()\n"
  "        self.m = 2\n"
  "    }\n"
  "}\n"
  "b = B()\n",
  NULL, "base()", -1 },
{ "base() com argumento nomeado",
  "Entity A() {\n"
  "    action __init__(self, x) {\n"
  "        self.x = x\n"
  "    }\n"
  "}\n"
  "Entity B(A) {\n"
  "    action __init__(self) {\n"
  "        base(x=5)\n"
  "    }\n"
  "}\n"
  "b = B()\n"
  "post(b.x)\n",
  "5", NULL, 0 },
{ "campo private do pai visível no método do pai",
  "Entity P() {\n"
  "    private s: int\n"
  "    action mostra(self) {\n"
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
  NULL, "'c'", -1 },

/* ── string com byte NUL ─────────────────────────────────────────────────
 * `\x00` truncava tudo depois dele: perda silenciosa de dado. */
{ "NUL no meio da string não trunca",
  "post(len(\"a\\x00b\"))\n", "3", NULL, 0 },

/* ── números ────────────────────────────────────────────────────────────── */
{ "int() de infinito é erro, não INT64_MIN",
  "post(int(flo(\"inf\")))\n", NULL, "infinito", -1 },
{ "int() de NaN é erro",
  "post(int(flo(\"nan\")))\n", NULL, "NaN", -1 },

/* ── funções de ordem superior e arquivo ─────────────────────────────────── */
{ "map confere a aridade da action",
  "action f(x, y) {\n"
  "    return x\n"
  "}\n"
  "post(map([1, 2], f))\n", NULL, "faltando argumento", -1 },
{ "open() de diretório é erro",
  "f = open(\"/tmp\")\npost(f.read())\n", NULL, "diretório", -1 },
{ "writelines com bytes grava os bytes",
  "p = \"/tmp/ps_teste_wl.bin\"\nusing open(p, \"wb\") as f { f.writelines([\"a\".encode()]) }\n"
  "using open(p, \"rb\") as f { post(f.read()) }\n",
  "b'a'", NULL, 0 },

/* ── encode/decode ──────────────────────────────────────────────────────── */
{ "encode respeita o encoding pedido",
  "post(\"café\".encode(\"latin-1\"))\n", "b'caf\\xe9'", NULL, 0 },
{ "decode de bytes inválidos é erro",
  "import bytes\nb = bytes.new([255, 254])\npost(b.decode())\n", NULL, "decode", -1 },
{ "decode com encoding inexistente é erro",
  "b = \"abc\".encode()\npost(b.decode(\"naoexiste\"))\n", NULL, "encoding", -1 },

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

};
const int NC_PENDENTES = N_CASOS(CASOS_PENDENTES);
