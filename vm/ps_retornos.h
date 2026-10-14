/*
 * Tipo de retorno de cada nativo — a tabela MEDIDA por scripts/mede_retornos.pr.
 *
 * Uma fonte só pra quem precisa saber o que um nativo devolve: o `--metadata`
 * (editor, doc) e o compilador (tipagem estática). O dono é o nome que o
 * código escreve (`os`, `sys.stdout`, `Parsing`, `builtins`) ou o do `type()`
 * do receptor (`str`, `DbCursor`); o membro é o nome do método, da função ou
 * do campo.
 *
 * O texto devolvido é um de três formatos:
 *   "str"        um tipo só
 *   "dict|Null"  o tipo muda com o argumento ou com o estado; cada lado medido
 *   "*"          o tipo é o do conteúdo guardado (`d.get`, `json.parse`)
 * NULL = o nativo não foi medido (não devolve, ou não existe).
 */
#ifndef PS_RETORNOS_H
#define PS_RETORNOS_H

const char *ps_retorno_de(const char *dono, const char *membro);

/* `nome` é um tipo que algum nativo devolve (`DbCursor`, `_RouteRegistrar`)?
 * É o que faz `DbCursor cur = c.cursor()` declarar um tipo que existe. */
int ps_retorno_tipo_existe(const char *nome);

/* O nativo `dono` (módulo, com o nome escrito no código, ou tipo, com o nome
 * do `type()`) tem o membro? 1 = tem, e é método/função (o retorno da tabela
 * é o da CHAMADA); 2 = tem, e é campo/valor (o retorno é o do próprio
 * membro); 0 = não tem; -1 = `dono` não é módulo nem tipo conhecido. Mora na
 * VM, junto das tabelas que o acesso a membro consulta. */
int ps_nativo_tem_membro(const char *dono, const char *membro);

/* `import nome` liga um módulo NATIVO da linguagem? (Os nativos ganham de
 * qualquer arquivo de mesmo nome — é a ordem de resolução do import.) */
int ps_nativo_eh_modulo(const char *nome);

/* A VM liga `nome` sozinha, sem o programa escrever nada? Builtins,
 * `__name__`, `PoolFile`, `byte` e as exceções — a mesma lista que a partida
 * do programa usa. */
int ps_nome_pre_ligado(const char *nome);

/* O membro mais parecido com `membro` num módulo nativo OU num tipo nativo
 * (`str`, `list`, `byte`…) — o "Did you mean" que a VM dá rodando —, ou
 * NULL. O checador usa o mesmo, pra frase antes de rodar ser a mesma. */
const char *ps_nativo_sugestao(const char *mod, const char *membro);

/* O nome de `nomes` mais parecido com `alvo` (distância de edição pequena),
 * ou NULL — a régua única do "Did you mean", pro checador aplicar aos
 * membros de uma classe do arquivo. */
const char *ps_sugere_nome(const char *alvo, const char **nomes, int n);
const char *ps_sugere_pre_ligado(const char *nome);

/* `catch (pedido e)` pega o erro `levantado`? A tabela de exceções da VM (o
 * pai pega o filho). O checador pergunta se o `try` em volta de um import
 * pega o `ImportError`. */
int ps_excecao_eh(const char *levantado, const char *pedido);

/* O tipo do erro que o `sys.exit` levanta fora de fibra: nenhum `catch` o
 * pega (o sem tipo também não, e nem a rede do `int funct`), os `finally`
 * rodam, e no topo o programa sai com o código pedido, sem mensagem. O nome
 * não é identificador: nenhum programa consegue escrever `catch` dele. */
#define PS_TIPO_SAIDA "<sys.exit>"

/* O sinal que FECHA um gerador suspenso (o laço dono saiu antes, ou o fim do
 * programa): ele volta no `yield` em que parou com este erro, que nenhum
 * `catch` pega, como o `sys.exit`; os `finally` e os `using` abertos rodam, e
 * quem fechou o engole. Todo tipo que começa com `<` é desses: o `catch` sem
 * tipo e a rede do `int funct` deixam passar. */
#define PS_TIPO_FECHA "<gerador.fecha>"

/* Quantos argumentos cada nativo aceita e com que frase ele recusa UM
 * argumento de cada tipo básico — a tabela MEDIDA por
 * scripts/mede_aridades.pr (vm/aridades_medidas.inc). O checador confere a
 * chamada por ela antes de rodar; rodando, quem confere é o próprio nativo.
 * Dono que só existe com serviço (banco, resposta HTTP) não é medido: NULL. */
#include <stddef.h>
#define PS_ARIDADE_NTIPOS 9
typedef struct {
    const char *dono;
    const char *membro;
    const char *nome;        /* o nome que a frase de quantidade usa */
    const char *params;      /* "sub,inicio=0,fim=Null"; NULL = não aceita nome */
    int minimo;
    int maximo;              /* -1 = sem teto */
    const char *recusa[PS_ARIDADE_NTIPOS];   /* int flo str list tup dict byte bool Null; NULL = aceita */
} PSAridade;

/* 1 = o nativo foi medido, e `out` (se não for NULL) recebe a entrada; 0 =
 * não foi. A tabela guarda os textos num bloco só e as posições deles (sem
 * ponteiro em dado inicializado, que é relocação em toda partida); os
 * ponteiros de `out` são montados aqui, apontando pra dentro do bloco. */
int ps_aridade_de(const char *dono, const char *membro, PSAridade *out);

/* A posição de um tipo básico em `recusa`, ou -1. */
int ps_aridade_tipo(const char *tipo);

/* A frase da recusa de quantidade, uma só pra VM e pro checador:
 * `upper() takes no arguments (1 given)`, `round() takes at most 2
 * arguments (3 given)`. */
void ps_frase_aridade(char *buf, size_t cap, const char *nome, int minimo, int maximo, int n);

#endif
