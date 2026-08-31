/*
 * Inteiro grande (bignum): a VM RESPONDIA ERRADO e seguia, que é pior que
 * errar. `1 << 63` dava negativo, `abs(INT64_MIN)` dava negativo,
 * `int("<32 dígitos>")` dava lixo, e os bitwise recusavam um valor cujo
 * próprio `type()` dizia ser `int`.
 */
#include "ps_teste.h"

const Caso CASOS_INTEIROS[] = {
{ "shift 1<<62 (cabia no int64)",  "post(1 << 62)\n", "4611686018427387904", NULL, 0 },
{ "shift 1<<63 (dava negativo)",   "post(1 << 63)\n", "9223372036854775808", NULL, 0 },
{ "shift 1<<64 (dava 1)",          "post(1 << 64)\n", "18446744073709551616", NULL, 0 },
{ "shift 3<<70 (dava 192)",        "post(3 << 70)\n", "3541774862152233910272", NULL, 0 },
{ "shift 1<<200",                  "post(1 << 200)\n",
  "1606938044258990275541962092341162602522202993782792835301376", NULL, 0 },
{ "shift ida e volta",             "post((1 << 64) >> 64)\n", "1", NULL, 0 },
{ "shift direito de bignum",       "post(9223372036854775808 >> 1)\n", "4611686018427387904", NULL, 0 },
{ "shift direito preserva sinal",  "post(-8 >> 1)\n", "-4", NULL, 0 },
{ "shift direito além de 64 bits", "post(1 >> 100, -1 >> 100)\n", "0 -1", NULL, 0 },
{ "deslocamento negativo é erro",  "post(1 << -1)\n", "", "deslocamento negativo", 1 },

{ "bitwise OR com bignum",  "post(18446744073709551616 | 1)\n", "18446744073709551617", NULL, 0 },
{ "bitwise AND com bignum", "post(18446744073709551617 & 1)\n", "1", NULL, 0 },
{ "bitwise XOR com bignum", "post(18446744073709551616 ^ 1)\n", "18446744073709551617", NULL, 0 },

{ "abs de INT64_MIN",   "post(abs(-9223372036854775808))\n", "9223372036854775808", NULL, 0 },
{ "abs de bignum",      "x = -18446744073709551616\npost(abs(x))\n", "18446744073709551616", NULL, 0 },
{ "abs comum",          "post(abs(-5), abs(5), abs(-2.5))\n", "5 5 2.5", NULL, 0 },

{ "int() de texto de 32 dígitos", "post(int(\"12345678901234567890123456789012\"))\n",
  "12345678901234567890123456789012", NULL, 0 },
{ "int() de texto normal",        "post(int(\"42\"), int(\"-7\"), int(\" 8 \"))\n", "42 -7 8", NULL, 0 },
{ "int() no limite do int64",     "post(int(\"9223372036854775807\"))\n", "9223372036854775807", NULL, 0 },
{ "int() um a mais que o limite", "post(int(\"9223372036854775808\"))\n", "9223372036854775808", NULL, 0 },

{ "sum com bignum",     "x = 18446744073709551616\npost(sum([x, 1]))\n", "18446744073709551617", NULL, 0 },
{ "sorted com bignum",  "x = 18446744073709551616\npost(sorted([x, 1]))\n", "[1, 18446744073709551616]", NULL, 0 },
{ "max/min com bignum", "x = 18446744073709551616\npost(max(x, 1), min(x, 1))\n",
  "18446744073709551616 1", NULL, 0 },
{ "round/int/flo de bignum", "x = 18446744073709551616\npost(round(x), int(x))\n",
  "18446744073709551616 18446744073709551616", NULL, 0 },
{ "type() de bignum diz int", "x = 18446744073709551616\npost(type(x), x.type())\n", "int int", NULL, 0 },

{ "json.parse de inteiro grande não satura",
  "import json\nd = json.parse(\"{\\\"n\\\": 123456789012345678901234567890}\")\npost(d[\"n\"])\n",
  "123456789012345678901234567890", NULL, 0 },
{ "json.parse de número normal",
  "import json\nd = json.parse(\"{\\\"a\\\": 42, \\\"b\\\": -7, \\\"c\\\": 2.5}\")\npost(d[\"a\"], d[\"b\"], d[\"c\"])\n",
  "42 -7 2.5", NULL, 0 },

{ "ordem entre listas e tuplas",
  "post([1] < [2], [1,2] >= [1,2], (1,2) < (1,3))\n", "True True True", NULL, 0 },
{ "sorted de listas",  "post(sorted([[2],[1]]))\n", "[[1], [2]]", NULL, 0 },
};
const int NC_INTEIROS = N_CASOS(CASOS_INTEIROS);
