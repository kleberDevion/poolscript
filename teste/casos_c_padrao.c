/*
 * GERADO por teste/geradores/c_padrao.pr — nao edite na mao.
 *
 * Cada caso e um programa em Jinga que usa a biblioteca padrao do C, com a
 * saida que o MESMO programa em C (o par em teste/geradores/c_padrao/) da,
 * compilado com `cc -std=c11`. Caso marcado DIVERGE trava a saida da Jinga,
 * e o comentario diz o que o C da e por que.
 *
 * pares: 4
 */
#include "ps_teste.h"

const Caso CASOS_C_PADRAO[] = {
{ "c_padrao ctype #1: isalnum, isalpha, isdigit e ispunct em letras, dígito e pontuação",
  "import <ctype.h>\nimport <stdio.h>\nfor each ch in \"a1B\?\" {\n    printf(\"%s %d %d %d %d\\n\", ch, isalnum(ord(ch)) != 0, isalpha(ord(ch)) != 0, isdigit(ord(ch)) != 0, ispunct(ord(ch)) != 0)\n}\n",
  "a 1 1 0 0\n1 1 0 1 0\nB 1 1 0 0\n\? 0 0 0 1", NULL, 0 },
{ "c_padrao ctype #2: isblank, iscntrl, isspace no espaço, na tabulação e na quebra de linha",
  "import <ctype.h>\nimport <stdio.h>\nlist cs = [32, 9, 10, 127, ord(\"x\")]\nfor each c in cs {\n    printf(\"%d %d %d\\n\", isblank(c) != 0, iscntrl(c) != 0, isspace(c) != 0)\n}\n",
  "1 0 1\n1 1 1\n0 1 1\n0 1 0\n0 0 0", NULL, 0 },
{ "c_padrao ctype #3: isgraph, isprint, islower, isupper e isxdigit",
  "import <ctype.h>\nimport <stdio.h>\nfor each ch in \"a Z9g~\" {\n    printf(\"%d%d%d%d%d\\n\", isgraph(ord(ch)) != 0, isprint(ord(ch)) != 0, islower(ord(ch)) != 0, isupper(ord(ch)) != 0, isxdigit(ord(ch)) != 0)\n}\n",
  "11101\n01000\n11010\n11001\n11100\n11000", NULL, 0 },
{ "c_padrao ctype #4: tolower e toupper, e o EOF que volta igual",
  "import <ctype.h>\nimport <stdio.h>\nprintf(\"%c%c%c %d %d\\n\", toupper(ord(\"a\")), tolower(ord(\"B\")), toupper(ord(\"1\")), tolower(EOF), toupper(EOF))\n",
  "Ab1 -1 -1", NULL, 0 },
};
const int NC_C_PADRAO = N_CASOS(CASOS_C_PADRAO);
