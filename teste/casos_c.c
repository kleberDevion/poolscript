/*
 * O C na Jinga (Plano 1): a biblioteca padrão do C em todo arquivo, sem
 * import. Cada caso tem a saída exata; o que muda de sistema (o texto do
 * PRId64, o endereço de um ponteiro) não entra na saída.
 *
 * Lote C1, fatia 1: chamar a função do C pelo nome, as constantes, o NULL
 * (que é o Null da Jinga), o ponteiro opaco que uma função devolve (FILE *,
 * char *), o errno, e os erros: argumento do tipo errado, quantidade, o
 * formato do printf, o nome que fica de fora, o tipo usado como valor.
 */
#include "ps_teste.h"

const Caso CASOS_C[] = {
/* ── chamar pelo nome ─────────────────────────────────────────────────── */
{ "C: printf e puts pelo nome, sem import (exemplo 1 do plano)",
  "printf(\"olá, %s! %d + %d = %d\\n\", \"mundo\", 2, 3, 2 + 3)\n"
  "puts(\"o puts põe a quebra de linha sozinho\")\n",
  "olá, mundo! 2 + 3 = 5\no puts põe a quebra de linha sozinho", NULL, 0 },
{ "C: nome nos dois vale o da Jinga; o do C pelo outro nome (exemplo 9 do plano)",
  "printf(\"%.4f %d %ld\\n\", sqrt(2.0), round(2.5), lround(2.5))\n"
  "post(abs(-3), pow(2, 10), labs(-3), llabs(-4))\n",
  "1.4142 2 3\n3 1024 3 4", NULL, 0 },
{ "C: post e printf saem na ordem em que foram chamados",
  "post(1)\nprintf(\"2\\n\")\npost(3)\n",
  "1\n2\n3", NULL, 0 },
{ "C: conversões do printf (largura, precisão, hexa, char, %%, *)",
  "printf(\"%5.2f|%-4d|%x|%X|%o|%c%c|%%|%*d|%e\\n\", 3.14159, 7, 255, 255, 8, 74, 105, 4, 42, 1234.5)\n",
  " 3.14|7   |ff|FF|10|Ji|%|  42|1.234500e+03", NULL, 0 },
{ "C: inteiro dá a volta no tipo do C; o grande entra no unsigned long long",
  "printf(\"%u %hhd %llu %d\\n\", -1, 300, 18446744073709551615, True)\n",
  "4294967295 44 18446744073709551615 1", NULL, 0 },
{ "C: o retorno vira valor da Jinga (int, flo, unsigned grande, ponteiro)",
  "post(strlen(\"jinga\"), sqrt(16.0), atoi(\"42\") + 1, strtol(\"ff\", Null, 16), toupper(97))\n"
  "post(strtoull(\"18446744073709551615\", NULL, 10), isdigit(65))\n",
  "5 4.0 43 255 65\n18446744073709551615 0", NULL, 0 },
{ "C: função do C como valor (variável, map) e impressa com o protótipo",
  "f = sqrt\npost(f(81.0), map([1.0, 4.0], sqrt))\npost(strlen, type(strlen))\n",
  "9.0 [1.0, 2.0]\n<funct do C size_t strlen(const char *s)> funct", NULL, 0 },
{ "C: espalhar uma lista na função do C",
  "list a = [\"%d-%d\\n\", 1, 2]\nprintf(*a)\n",
  "1-2", NULL, 0 },
{ "C: chamada dentro de funct e em laço com o código de máquina",
  "funct raiz(int n) {\n    return sqrt(n * 1.0)\n}\n"
  "flo s = 0.0\nint i = 0\nwhile i < 3000 {\n    s = s + fabs(-1.5)\n    i++\n}\n"
  "post(raiz(81), s)\n",
  "9.0 4500.0", NULL, 0 },
/* ── constantes, NULL, valores ───────────────────────────────────────── */
{ "C: constantes com o valor do sistema; NULL é o Null",
  "post(EOF, INT_MAX, CHAR_BIT, ULLONG_MAX, INT8_MIN, NULL)\n"
  "post(EXIT_SUCCESS, EXIT_FAILURE, SEEK_SET, DBL_MANT_DIG)\n",
  "-1 2147483647 8 18446744073709551615 -128 Null\n0 1 0 53", NULL, 0 },
{ "C: constante de texto (PRId64) monta o formato",
  "printf(\"%\" + PRId64 + \"|%\" + PRIx32 + \"\\n\", 5, 255)\n",
  "5|ff", NULL, 0 },
{ "C: stdout é FILE *, e o fprintf escreve nele",
  "post(type(stdout))\nfprintf(stdout, \"[%s]\\n\", \"ok\")\n",
  "FILE *\n[ok]", NULL, 0 },
/* ── ponteiro opaco, texto do C, errno ───────────────────────────────── */
{ "C: errno e strerror (exemplo 10 do plano); o ponteiro nulo é Null",
  "f = fopen(\"ps_c_nao_existe.txt\", \"r\")\n"
  "if f == NULL {\n    printf(\"errno %d: %s\\n\", errno, strerror(errno))\n}\n"
  "post(f == Null, f)\n",
  "errno 2: No such file or directory\nTrue Null", NULL, 0 },
{ "C: str() traz o texto do C de volta",
  "char_p = strerror(2)\npost(type(char_p), str(char_p).upper())\n",
  "char * NO SUCH FILE OR DIRECTORY", NULL, 0 },
{ "C: str() de dado do C que não é texto sai descrito, como na f-string; dentro de lista também",
  "str d = str(stdout)\n"
  "post(d.startswith(\"(FILE *) 0x\"), d == f\"{stdout}\", str([strerror(2)]).startswith(\"[(char *) 0x\"))\n",
  "True True True", NULL, 0 },
{ "C: arquivo pelo FILE * (fopen, fprintf, fclose, remove)",
  "f = fopen(\"ps_c_notas.txt\", \"w\")\n"
  "fprintf(f, \"%s=%.1f\\n\", \"media\", 8.25)\n"
  "post(fclose(f))\n"
  "import os\npost(os.readFile(\"ps_c_notas.txt\"))\n"
  "post(remove(\"ps_c_notas.txt\"), os.exists(\"ps_c_notas.txt\"))\n",
  "0\nmedia=8.2\n\n0 False", NULL, 0 },
/* ── nome do programa ganha do C ─────────────────────────────────────── */
{ "C: variável do programa esconde o nome do C",
  "int time = 5\npost(time)\n", "5", NULL, 0 },
{ "C: funct do programa esconde a função do C",
  "funct free() {\n    return \"minha\"\n}\npost(free())\n", "minha", NULL, 0 },
/* ── erros ──────────────────────────────────────────────────────────── */
{ "C: %d do printf com texto é TypeError com a conversão",
  "printf(\"%d\\n\", \"dez\")\n", "", "TypeError: printf: %d espera int, recebeu str", 1 },
{ "C: %s do printf com número",
  "printf(\"%s\\n\", 5)\n", "", "TypeError: printf: %s espera const char *, recebeu int", 1 },
{ "C: formato pede mais argumentos do que vieram",
  "printf(\"%d %d\\n\", 1)\n", "", "TypeError: printf: o formato pede 2 argumentos depois dele, e veio 1", 1 },
{ "C: formato pede menos argumentos do que vieram",
  "printf(\"%d\\n\", 1, 2)\n", "", "TypeError: printf: o formato pede 1 argumento depois dele, e vieram 2", 1 },
{ "C: conversão desconhecida no formato",
  "printf(\"%k\\n\", 1)\n", "", "ValueError: printf: conversão desconhecida no formato: %k", 1 },
{ "C: quantidade de argumentos com a frase dos nativos",
  "strlen(1, 2)\n", "", "TypeError: strlen() takes exactly one argument (2 given)", 1 },
{ "C: argumento do tipo errado diz o tipo do C",
  "strlen(5)\n", "", "TypeError: strlen(): o argumento 1 é const char * no C, e veio int", 1 },
{ "C: flo onde o C pede inteiro não converte calado",
  "abs_c = labs\nabs_c(2.5)\n", "", "TypeError: labs(): o argumento 1 é long no C, e veio flo", 1 },
{ "C: str da Jinga não entra onde o C escreve (strtok)",
  "str s = \"a b\"\nstrtok(s, \" \")\n", "",
  "TypeError: strtok() altera o texto (char *), e o str da Jinga só entra como const char *: copie pra um buffer", 1 },
{ "C: argumento nomeado na função do C",
  "printf(format=\"x\\n\")\n", "", "TypeError: printf() takes no keyword arguments", 1 },
{ "C: o que fica de fora é NameError com o motivo",
  "vprintf(\"x\", 1)\n", "",
  "NameError: vprintf fica de fora: o va_list só existe dentro de uma função variádica do C (doc: o C na Jinga)", 2 },
{ "C: gets saiu do C11",
  "gets(1)\n", "", "NameError: gets fica de fora: saiu do C11, porque lia sem limite e passava do fim do buffer; use fgets", 2 },
{ "C: tipo do C escrito onde vai valor",
  "post(size_t)\n", "",
  "NameError: size_t é tipo do C (stddef.h): vale em declaração, cast e sizeof, e não como valor", 2 },
{ "C: nome que não é da Jinga nem do C continua NameError",
  "post(printff(1))\n", "", "NameError: name 'printff' is not defined", 2 },
};

const int NC_C = N_CASOS(CASOS_C);
