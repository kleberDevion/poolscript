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
