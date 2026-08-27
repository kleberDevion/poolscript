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
/* ── recursão sem teto: três SIGSEGV da auditoria de engenharia (27/08) ──────
 * Os três eram descida recursiva sem limite. O detector de CICLO existia e não
 * bastava: ele pega `l.append(l)`, e não pega profundidade sem ciclo nem ciclo
 * que fecha acima do vetor de 256 níveis que ele registra. */
{ "parser: 30 mil parenteses aninhados é erro, nao SIGSEGV",
  /* o gerador do caso é o próprio motor: 30 mil `(` na mão não cabe aqui */
  "import os\n"
  "import sys\n"
  "fundo = 30000\n"
  "src = \"post(\" + (\"(\" * fundo) + \"1\" + (\")\" * fundo) + \")\"\n"
  "using open(\"p.ps\", \"w\") as f { f.write(src) }\n"
  "os.cmd(\"'\" + sys.executable + \"' --check p.ps > o.txt 2>&1\")\n"
  "post(\"aninhada demais\" in open(\"o.txt\").read())\n",
  "True", NULL, 0 },
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

{ "compara estruturas mutuamente recursivas",
  "a = [1]\nb = [1]\na.append(b)\nb.append(a)\npost(a == b)\n",
  NULL, NULL, 0 },

{ "contains em estrutura mutuamente recursiva",
  "a = [1]\nb = [1]\na.append(b)\nb.append(a)\npost(a.contains(b))\n",
  NULL, NULL, 0 },

{ "INT64_MIN % -1 (era SIGFPE)",
  "post(-9223372036854775808 % -1)\n",
  "0", NULL, 0 },

{ "zfill com largura de 64 bits (era OOM da máquina)",
  "post(\"a\".zfill(9223372036854775807))\n",
  NULL, "MemoryError", -1 },

{ "ljust com largura absurda",
  "post(\"a\".ljust(9223372036854775807))\n",
  NULL, "MemoryError", -1 },

{ "rjust com largura absurda",
  "post(\"a\".rjust(9223372036854775807))\n",
  NULL, "MemoryError", -1 },

{ "center com largura absurda",
  "post(\"a\".center(9223372036854775807))\n",
  NULL, "MemoryError", -1 },

{ "largura normal continua funcionando",
  "post(\"a\".zfill(5))\npost(\"ab\".ljust(5, \"-\"))\n"
  "post(\"ab\".center(6, \".\"))\npost(\"7\".rjust(3, \"0\"))\n",
  "0000a\nab---\n..ab..\n007", NULL, 0 },
};
const int NC_CRASH = N_CASOS(CASOS_CRASH);
