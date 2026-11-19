/*
 * A biblioteca do C na Jinga (fatia 1b): `#include "stdio.h"` libera o nome, e
 * a chamada vai de verdade pela libffi.
 *
 * O defeito que estes casos fixam: o `include` ligava a constante e o NULL,
 * mas a FUNÇÃO não virava valor chamável — `printf(...)` dava
 * `NameError: name 'printf' is not defined` com o cabeçalho importado. E a
 * chamada, ao passar uma string, mandava o `chars` no lugar do ENDEREÇO do
 * ponteiro, que é o que a libffi lê: segfault dentro do `strlen`.
 */
#include "ps_teste.h"
const Caso CASOS_C[] = {
    /* ── o nome existe só com o cabeçalho dele (o import é obrigatório) ──────── */
    {"C: strlen sem assinatura e NameError",
     "post(strlen(\"abc\"))\n",
     "", "NameError: name 'strlen' is not defined", 2},
    {"C: assinatura escrita e import c libc chamam strlen",
     "import c \"libc\"\n"
     "int strlen(const char *s)\n"
     "str a = \"abc\"\n"
     "post(strlen(a))\n",
     "3", NULL, 0},
    {"C: a mesma chamada com o nome da .so",
     "import c \"libc.so.6\"\n"
     "int strlen(const char *s)\n"
     "post(strlen(\"quatro\"))\n",
     "6", NULL, 0, NULL, 0, "linux"},
    /* ── variádica: o formato diz o tipo de cada argumento ──────────────────── */
    {"C: printf variadico com %d e %s",
     "import c \"libc\"\n"
     "int printf(const char *format, ...)\n"
     "printf(\"n=%d s=%s\\n\", 42, \"ana\")\n",
     "n=42 s=ana", NULL, 0},
    {"C: printf com mais argumentos do que o formato pede e TypeError",
     "import c \"libc\"\n"
     "int printf(const char *format, ...)\n"
     "printf(\"%d\\n\", 1, 2)\n",
     "", "TypeError: printf: o formato pede 1 argumento depois dele, e vieram 2", 1},
    /* ── devolucao: inteiro, comparacao e void ──────────────────────────────── */
    {"C: strcmp devolve negativo como o C",
     "import c \"libc\"\n"
     "int strcmp(const char *a, const char *b)\n"
     "post(strcmp(\"abc\", \"abd\"))\n",
     "-1", NULL, 0},
    {"C: funcao void (free de NULL) devolve Null e nao estoura",
     "import c \"libc\"\n"
     "void free(void *p)\n"
     "post(free(Null))\n",
     "Null", NULL, 0},
    /* ── argumento sem conversao: a frase e da Jinga, antes do C ────────────── */
    {"C: strlen(42) e TypeError com o tipo que o C pede",
     "import c \"libc\"\n"
     "int strlen(const char *s)\n"
     "post(strlen(42))\n",
     "", "TypeError: strlen: argumento 1 sem conversao para const char *", 1},
    /* ── a assinatura escrita no programa, sem catalogo de nomes ────────────── */
    {"C: a funcao escrita no programa chama o simbolo da lib",
     "import c \"libc\"\n"
     "int abs(int x)\n"
     "post(abs(-7))\n",
     "7", NULL, 0},
    {"C: .so propria, criada no programa, chamada pela assinatura",
     "import os\n"
     "import c \"./libjinga_c_prova.so\"\n"
     "int jinga_c_prova(int x)\n"
     "os.writeFile(\"p.c\", \"int jinga_c_prova(int x) { return x + 1; }\\n\")\n"
     "os.run([\"cc\", \"-shared\", \"-fPIC\", \"-o\", \"libjinga_c_prova.so\", \"p.c\"], capture=True)\n"
     "post(jinga_c_prova(41))\n",
     "42", NULL, 0, NULL, 0, "unix"},
    {"C: int main() com corpo valido compila e a Jinga segue",
     "int main(void) { return 0; }\n"
     "post(\"ok\")\n",
     "ok", NULL, 0},
    {"C: variavel com tipo do C (const char *)",
     "const char *s = \"abc\"\n"
     "post(s)\n",
     "abc", NULL, 0},
    {"C: variavel com typedef do C (size_t)",
     "size_t n = 3\n"
     "post(n)\n",
     "3", NULL, 0},
    {"C: variavel do C sem valor nasce com o zero do tipo",
     "const char *s\n"
     "post(s)\n",
     "Null", NULL, 0},
    {"C: int n = 3 continua sendo a declaracao da linguagem",
     "int n = 3\n"
     "post(n)\n",
     "3", NULL, 0},    /* ── sizeof e cast: a assinatura do C ───────────────────────────────────── */
    {"C: sizeof de tipos do C bate com o cc",
     "post(sizeof(int), sizeof(char *), sizeof(size_t), sizeof(double))\n",
     "4 8 8 8", NULL, 0},
    {"C: sizeof(struct tm)",
     "post(sizeof(struct tm))\n",
     "56", NULL, 0},
    {"C: cast trunca no tamanho do tipo como o C",
     "post((char)321)\n"
     "post((unsigned char)255)\n"
     "post((short)70000)\n",
     "65\n255\n4464", NULL, 0},
    {"C: cast para ponto flutuante e para inteiro",
     "post((double)3)\n"
     "post((int)3.9)\n",
     "3.0\n3", NULL, 0},    /* ── struct, union, array e offsetof ───────────────────────────────────── */
    {"C: struct declarado no programa, sizeof bate com o cc",
     "struct ponto { int x; int y }\n"
     "post(sizeof(struct ponto))\n",
     "8", NULL, 0},
    {"C: union declarada no programa",
     "union u { int a; float b }\n"
     "post(\"ok\")\n",
     "ok", NULL, 0},
    {"C: offsetof do campo",
     "#include \"stddef.h\"\n"
     "struct ponto { int x; int y }\n"
     "post(offsetof(struct ponto, y))\n",
     "4", NULL, 0},
    {"C: array do C com inicializador e indice",
     "int v[3] = {1, 2, 3}\n"
     "post(v[1])\n",
     "2", NULL, 0},    /* ── typedef e enum do C ───────────────────────────────────────────────── */
    {"C: typedef do C da nome novo ao tipo",
     "typedef unsigned long tamanho\n"
     "tamanho n = 5\n"
     "post(n)\n",
     "5", NULL, 0},
    {"C: enum do C vale pelos nomes soltos",
     "enum d { A, B = 5, C }\n"
     "post(A, B, C)\n",
     "0 5 6", NULL, 0},    /* ── ponteiro: &x e *p ─────────────────────────────────────────────────── */
    {"C: &x e *p de um int",
     "int n = 3\n"
     "int *p = &n\n"
     "post(*p)\n",
     "3", NULL, 0},
    {"C: *p enxerga a escrita nova na variavel",
     "int n = 3\n"
     "int *p = &n\n"
     "n = 9\n"
     "post(*p)\n",
     "9", NULL, 0},
    {"C: &x e *p de um double",
     "double d = 1.5\n"
     "double *q = &d\n"
     "post(*q)\n",
     "1.5", NULL, 0},
    {"C: for e while no corpo da funcao",
     "int acumula(int a, int b) {\n"
     "    int s;\n"
     "    int i;\n"
     "    s = 0;\n"
     "    for (i = 0; i < a; i = i + 1) {\n"
     "        s = s + b;\n"
     "    }\n"
     "    while (s > 100) {\n"
     "        s = s - 1;\n"
     "    }\n"
     "    return s;\n"
     "}\n"
     "post(acumula(3, 4))\n",
     "12", NULL, 0},
    {"C: comentario de linha e seta",
     "int pega(void) {\n"
     "    struct par { int a; int b; };\n"
     "    struct par v;\n"
     "    struct par *p;\n"
     "    v.a = 7; // comentario do C\n"
     "    p = &v;\n"
     "    return p->a + 1;\n"
     "}\n"
     "post(pega())\n",
     "8", NULL, 0},
    {"C: funct da Jinga no meio de duas funcoes de C",
     "int dobro(int x) {\n"
     "    return x * 2;\n"
     "}\n"
     "int funct triplo(int x) {\n"
     "    return x * 3\n"
     "}\n"
     "int mais(int x) {\n"
     "    return dobro(x) + 1;\n"
     "}\n"
     "post(mais(4))\n"
     "post(triplo(4))\n",
     "9\n12", NULL, 0},
    {"C: define colado em cima da funcao",
     "#define DOBRO(x) ((x) * 2)\n"
     "int usa(int x) {\n"
     "    return DOBRO(x);\n"
     "}\n"
     "post(usa(5))\n",
     "10", NULL, 0},
    {"C: struct do arquivo a funcao de C enxerga",
     "struct caixa { int a; int b; };\n"
     "int soma_caixa(void) {\n"
     "    struct caixa v;\n"
     "    v.a = 3;\n"
     "    v.b = 4;\n"
     "    return v.a + v.b;\n"
     "}\n"
     "post(soma_caixa())\n",
     "7", NULL, 0},
    {"C: *lista continua o spread da linguagem",
     "lista = [1, 2]\n"
     "post(*lista)\n",
     "1 2", NULL, 0},
};
const int NC_C = N_CASOS(CASOS_C);
