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
 * `__name__`, `PoolFile`, `byte`, as exceções e os módulos que nascem sem
 * import (`Parsing`) — a mesma lista que a partida do programa usa. */
int ps_nome_pre_ligado(const char *nome);

/* O membro do módulo nativo mais parecido com `membro` (o "Did you mean" que
 * a VM dá rodando), ou NULL. */
const char *ps_nativo_sugestao(const char *mod, const char *membro);

#endif
