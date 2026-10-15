/*
 * Motor de expressão regular da Jinga, em C puro.
 *
 * Sem dependência da VM — recebe e devolve buffers de bytes. É o que permite
 * `regex_lib`, `parsing_lib` e os métodos `match`/`findall`/`sub` de string
 * rodarem em C puro.
 *
 * Backtracking recursivo, não autômato. A escolha é deliberada: a semântica
 * de backtracking (ganância, ordem de alternativas, quais grupos ficam
 * preenchidos) é o requisito, e os dois motores têm que dar a mesma resposta
 * — um autômato daria outra em casos como `(a|ab)c`.
 *
 * A sintaxe é a do módulo `re` do Python (a referência, frase por frase, nos
 * erros também): classes, quantificadores gulosos, preguiçosos e possessivos,
 * alternância, grupos (capturante, `(?:...)`, nomeado `(?P<n>...)`, atômico
 * `(?>...)`), retrovisor `\N` e `(?P=n)`, condicional `(?(n)sim|não)`,
 * comentário `(?#...)`, âncoras `^ $ \A \Z \z \b \B`, `\d \w \s` e negações
 * pelo Unicode, lookahead e lookbehind (este de largura fixa), os escapes
 * `\a \f \n \r \t \v \\ \xHH \uHHHH \UHHHHHHHH` e octal, e as flags `(?aimsux)`
 * global (só no começo) e com escopo `(?ims-ims:...)`. Além do `re`, a forma
 * `(?<nome>...)` do grupo nomeado. Fora: `\N{nome}` (falta a tabela de nomes
 * do Unicode) e a flag `L` (é de padrão em bytes).
 */
#ifndef PS_REGEX_H
#define PS_REGEX_H

typedef struct PSRegex PSRegex;

/* Posições de início/fim de cada grupo (0..ngrupos), em BYTES. -1 = não
 * participou. Os vetores são do PSRegex (do tamanho do padrão, sem teto de
 * grupos) e valem até a próxima busca com ele. */
typedef struct {
    int *inicio;
    int *fim;
    int  ngrupos;             /* quantos grupos capturantes o padrão tem */
} RxCaptura;

/* Compila. Devolve NULL e escreve em `erro` se o padrão for inválido ou usar
 * construção não suportada. */
PSRegex *ps_regex_compila(const char *padrao, int len, char *erro, int erro_cap);
void     ps_regex_free(PSRegex *r);

/* Procura a partir de `de`. Devolve 1 se achou (preenchendo `cap`, com o
 * grupo 0 = casamento inteiro), 0 se não achou, -1 em estouro de
 * backtracking. */
int ps_regex_busca(PSRegex *r, const char *s, int len, int de, RxCaptura *cap);

/* A mesma busca, sem aceitar o casamento VAZIO que comece em `proibido` (-1 =
 * aceita): é o passo seguinte a um casamento vazio em p, que no `re` (desde o
 * 3.7) recomeça em p e só aceita lá um casamento não vazio. Pular um caractere
 * perdia o "a" de `findall("a*?", "aaa")`. */
int ps_regex_busca_vazio(PSRegex *r, const char *s, int len, int de, int proibido, RxCaptura *cap);

/* Casa a string INTEIRA (equivalente ao `fullmatch`). */
int ps_regex_casa_tudo(PSRegex *r, const char *s, int len, RxCaptura *cap);

/* A posição do caractere seguinte ao que começa em `pos` (pos + os bytes
 * dele em UTF-8); no fim do texto, pos + 1. É o passo de quem procura de novo
 * e de quem anda depois de um casamento vazio: andar um BYTE recomeçava no
 * meio do caractere (`[^ç]` casava o resto do "ç"). */
int ps_regex_proximo(const char *s, int len, int pos);

int ps_regex_ngrupos(const PSRegex *r);

/* Índice do grupo nomeado `(?P<nome>...)`, ou -1 se não existe. O nome é
 * guardado na compilação pra o `\g<nome>` do sub() e o `.group("nome")`
 * poderem resolver — antes o nome era lido e jogado fora. */
int ps_regex_grupo_por_nome(const PSRegex *r, const char *nome, int len);
/* Nome do grupo `g` (1..ngrupos), ou NULL se o grupo não tem nome. */
const char *ps_regex_nome_do_grupo(const PSRegex *r, int g);

/* O nome serve pra grupo? A regra do `re` (`str.isidentifier`), a mesma do
 * `(?P<nome>...)`, pro `\g<nome>` do texto de troca. */
int ps_regex_nome_valido(const char *s, int len);

#endif /* PS_REGEX_H */
