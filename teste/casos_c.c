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
    {"C: strlen sem o cabecalho da NameError que diz qual importar",
     "post(strlen(\"abc\"))\n",
     "", "NameError: name 'strlen' is not defined. Did you forget to import <string.h>?", 2},
    {"C: com string.h, strlen chama o C e devolve o tamanho",
     "#include \"string.h\"\n"
     "str a = \"abc\"\n"
     "post(strlen(a))\n",
     "3", NULL, 0},
    {"C: import \"string.h\" (sem o #) e a mesma coisa",
     "import \"string.h\"\n"
     "post(strlen(\"quatro\"))\n",
     "6", NULL, 0},
    {"C: include sem o # e a mesma coisa",
     "include \"string.h\"\n"
     "post(strlen(\"xy\"))\n",
     "2", NULL, 0},
    /* ── variádica: o formato diz o tipo de cada argumento ──────────────────── */
    {"C: printf variadico com %d e %s",
     "#include \"stdio.h\"\n"
     "printf(\"n=%d s=%s\\n\", 42, \"ana\")\n",
     "n=42 s=ana", NULL, 0},
    {"C: printf com mais argumentos do que o formato pede e TypeError",
     "#include \"stdio.h\"\n"
     "printf(\"%d\\n\", 1, 2)\n",
     "", "TypeError: printf: o formato pede 1 argumento depois dele, e vieram 2", 1},
    /* ── devolucao: inteiro, comparacao e void ──────────────────────────────── */
    {"C: strcmp devolve negativo como o C",
     "#include \"string.h\"\n"
     "post(strcmp(\"abc\", \"abd\"))\n",
     "-1", NULL, 0},
    {"C: funcao void (free de NULL) devolve Null e nao estoura",
     "#include \"stdlib.h\"\n"
     "post(free(Null))\n",
     "Null", NULL, 0},
    /* ── argumento sem conversao: a frase e da Jinga, antes do C ────────────── */
    {"C: strlen(42) e TypeError com o tipo que o C pede",
     "#include \"string.h\"\n"
     "post(strlen(42))\n",
     "", "TypeError: strlen: argumento 1 sem conversao para const char *", 1},
    /* ── a assinatura escrita no programa (acesso total ao C) ──────────────── */
    {"C: a funcao escrita no programa chama o simbolo do sistema",
     "int abs(int x)\n"
     "post(abs(-7))\n",
     "7", NULL, 0},
    {"C: int main() com corpo e aceito e o corpo nao roda (a funcao e a do C)",
     "int main() { return 0 }\n"
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
     "3", NULL, 0},
};
const int NC_C = N_CASOS(CASOS_C);
