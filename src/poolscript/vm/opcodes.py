"""
Conjunto de instruções da VM da PoolScript.

Formato: bytecode é uma lista PLANA de int, dois slots por instrução —
`[opcode, arg, opcode, arg, ...]`. Instrução de tamanho fixo (não varint)
porque:

  1. decodificar vira `op = code[ip]; arg = code[ip+1]` — sem desvio;
  2. `list[int]` é exatamente o que o mypyc compila pra array de inteiros
     nativo em C, sem boxing;
  3. no porte pra C vira `int32_t code[]` sem mudar nada da estrutura.

Instruções que não usam argumento gravam 0 no slot — desperdiça 4 bytes por
instrução e paga barato em troca de decodificação sem ramificação.

A ordem dos opcodes importa: os mais quentes (medidos no fib/loop) ficam com
os menores números, pra ficarem no topo do if/elif do loop de execução.
"""
from __future__ import annotations

# ── carga/armazenamento (os mais frequentes) ───────────────────────────────
LOAD_CONST    = 0   # arg = índice no pool de constantes
LOAD_LOCAL    = 1   # arg = slot do local (índice, não nome)
STORE_LOCAL   = 2
LOAD_GLOBAL   = 3   # arg = índice na tabela de globais (resolvido em compilação)
STORE_GLOBAL  = 4

# ── aritmética ─────────────────────────────────────────────────────────────
ADD           = 5
SUB           = 6
MUL           = 7
DIV           = 8
MOD           = 9
NEG           = 10

# ── comparação ─────────────────────────────────────────────────────────────
LT            = 11
GT            = 12
LE            = 13
GE            = 14
EQ            = 15
NE            = 16

# ── controle de fluxo ──────────────────────────────────────────────────────
JUMP          = 17  # arg = endereço absoluto (não offset — simplifica o patch)
JUMP_IF_FALSE = 18  # consome o topo da pilha
POP_TOP       = 19

# ── funções ────────────────────────────────────────────────────────────────
CALL          = 20  # arg = quantidade de argumentos
RETURN        = 21
MAKE_FUNCTION = 22  # arg = índice da constante que guarda o CodeObj

# ── bitwise ────────────────────────────────────────────────────────────────
BIT_OR        = 23
BIT_XOR       = 24
BIT_AND       = 25
LSHIFT        = 26
RSHIFT        = 27
BIT_NOT       = 28

# ── estruturas de dados nativas ────────────────────────────────────────────
BUILD_LIST    = 30  # arg = quantos itens consumir da pilha
BUILD_DICT    = 31  # arg = quantos PARES consumir (2*arg valores)
INDEX_GET     = 32  # pilha: alvo, indice -> valor
INDEX_SET     = 33  # pilha: alvo, indice, valor -> (nada)

# ── iteração ───────────────────────────────────────────────────────────────
# Pilha: [.., container, indice]. Se indice < len: empurra o item e
# incrementa o índice. Se acabou: descarta container+indice e salta pro arg.
# Evita criar um objeto iterador — o par (container, índice) na pilha basta
# para list, string e dict.
ITER_NEXT     = 34
DUP           = 35   # duplica o topo (usado por x++ em posição de expressão)

# ── construção de valores compostos ────────────────────────────────────────
BUILD_STR     = 36  # arg = quantos valores concatenar como texto (interpolação)
BUILD_TUPLE   = 37  # arg = quantos itens
SLICE         = 38  # pilha: alvo, inicio, fim, passo (Null = ausente)

# Prólogo de parâmetro com valor padrão. Consome do topo da pilha o ÍNDICE do
# parâmetro e salta pro `arg` se aquele slot JÁ FOI PREENCHIDO pela chamada.
#
# Checa o slot (sentinela UNSET), não uma contagem de argumentos: argumento
# nomeado pode deixar BURACOS (`f(1, c=100)` pula o `b`), e contagem
# assumiria um prefixo contíguo. `f(a=Null)` conta como preenchido — Null é
# valor, UNSET é ausência.
#
# O default é avaliado no CALLEE: o compilador não sabe qual função será
# chamada, então quem conhece os defaults é o próprio protótipo.
JUMP_IF_SET   = 39

# ── nomes de resolução DINÂMICA ────────────────────────────────────────────
# Atribuição simples dentro de uma função não cria local automaticamente: se
# a variável já existe num escopo externo, ela é MODIFICADA lá (é o que
# `Scope.set` do interpretador faz, subindo a cadeia). Só cria local se o
# nome não existir em lugar nenhum.
#
# Um compilador estático não sabe qual dos dois vai acontecer, então a
# decisão fica em runtime. Ambos consomem do topo o ÍNDICE DO LOCAL e usam
# `arg` como índice da global.
#
# Parâmetro e declaração tipada (`int x = 1`) NÃO passam por aqui — esses
# são local com certeza e usam LOAD_LOCAL/STORE_LOCAL direto.
LOAD_NAME     = 40
STORE_NAME    = 41

# Chamada com argumento nomeado: `f(a, base="x")`.
# Pilha: callee, <todos os valores em ordem>, <tupla com os nomes dos N
# últimos>. `arg` = total de argumentos. A VM olha os nomes de parâmetro do
# protótipo para reposicionar cada nomeado — o compilador não pode fazer
# isso porque o alvo da chamada só se conhece em runtime.
CALL_KW       = 42

# ── tratamento de erro ─────────────────────────────────────────────────────
# SETUP_TRY registra um handler (endereço do primeiro `catch`) numa pilha
# própria, junto do estado a restaurar: profundidade de frame, de pilha e do
# pool de locais. Sem gravar esses três, o desenrolamento deixaria lixo e o
# `catch` rodaria com a pilha de outra função.
SETUP_TRY     = 43
POP_TRY       = 44   # saída normal do bloco: descarta o handler
RAISE         = 45   # levanta com o valor do topo como mensagem
PUSH_ERR_TYPE = 46   # empurra o tipo do erro corrente (pra `catch (Tipo e)`)

# ── apoio ao `match` ───────────────────────────────────────────────────────
JUMP_IF_TRUE  = 47   # simétrico do JUMP_IF_FALSE; padrão `1 | 2` precisa dele
LEN           = 48   # tamanho de lista/tupla/string/dict (padrão de lista)
HAS_KEY       = 49   # pilha: dict, chave -> bool (padrão de dict, sem estourar)

# ── objetos (Entity) ───────────────────────────────────────────────────────
MAKE_CLASS    = 50  # arg = índice do descritor de classe; pais vêm da pilha
GET_MEMBER    = 51  # arg = const com o nome; pilha: alvo -> campo ou método ligado
SET_MEMBER    = 52  # arg = const com o nome; pilha: alvo, valor -> ()
LOAD_SELF     = 53  # empurra o `self` do frame (slot 0) — usado por base()

# `base(args)`: pilha [classe_pai, self, arg1..argN], arg = N.
# Busca `__init__` a partir da CLASSE PAI, não pela instância — pela
# instância a busca acharia o override da filha e recursaria pra sempre.
CALL_BASE     = 54

# Duplica os DOIS do topo: [a, b] -> [a, b, a, b]. Existe para `l[i] += 1`:
# o par (container, índice) precisa ser lido e depois reescrito, e re-avaliar
# as expressões chamaria `f()` duas vezes em `l[f()] += 1`.
DUP2          = 55

# `import json` / `from date import today`.
# arg = índice da constante com o NOME do módulo. A VM resolve na tabela de
# módulos nativos e empurra o objeto-módulo; quem decide o que fazer com ele
# (ligar o namespace inteiro ou puxar membros) é o STORE_NAME/GET_MEMBER que
# o compilador emite depois. Resolver em runtime é obrigatório: o compilador
# não conhece a tabela de módulos da VM.
IMPORT_MOD    = 56

# `x is str` / `x in l` e as formas negadas.
# `is` compara TIPO quando a direita é um TypeName, e identidade/valor caso
# contrário — a decisão é em runtime porque só ali se sabe o que veio.
IS_OP         = 57   # arg: 0 = `is`, 1 = `is not`
IN_OP         = 58   # arg: 0 = `in`, 1 = `not in`

# Empurra uma referência de TIPO (`str`, `int`, `dict`…). arg = TIPO_*.
LOAD_TIPO     = 59

# `count <tipo> in x` e `count each`.
# arg = tipo | (tem_valor << 8): o valor filtrado é opcional, e empacotá-lo
# no argumento evita um opcode só pra dizer "sem valor".
# Pilha: container, valor -> total (COUNT) ou lista de pares (COUNT_PARES).
COUNT         = 60
COUNT_PARES   = 61   # [(indice, item), ...] — o `count each` itera sobre isso

# `@NonNull`: nenhum parâmetro pode chegar Null. arg = quantos checar.
# Emitido DEPOIS do prólogo de defaults, senão um default Null passaria batido.
CHECK_NONNULL = 62

# `model U { nome: str(60) }`. arg = índice do descritor de model no
# programa; o objeto é criado em runtime e guardado como global.
MAKE_MODEL    = 63

# `a, b = ...` / `a, *r = ...`.
# Consome a sequência do topo e empurra os N valores EM ORDEM INVERSA (o
# último alvo fica no topo), pra sequência de STOREs sair na ordem natural.
# arg = n_alvos | (star_index+1) << 8; star_index+1 = 0 quer dizer sem `*`.
UNPACK        = 64

# `yield v` — suspende o frame e devolve `v` a quem está iterando.
# Só aparece em protótipo marcado como gerador; a marca é do compilador,
# porque quem sabe que a action tem `yield` é quem a percorre.
YIELD         = 65

# Fecha o recurso do `using`, se ele tiver `close`. Consome o topo.
# Silencioso quando não tem: `using` sobre valor comum não é erro.
CLOSE_SE_TEM  = 66

# `not x` / `!x` — devolve BOOL, não o operando. `and`/`or` também: na
# PoolScript `0 or 5` é `True`, não `5`, então o resultado passa por TO_BOOL
# em vez de ficar na pilha como veio.
NOT           = 67
TO_BOOL       = 68

# Tipo declarado: `flo x = 5` guarda 5.0, `int x = "7"` guarda 7. Converte o
# que a linguagem manda converter e reclama do resto — arg = TIPO_*.
COERCE_DECL   = 69
# Tipo de retorno de `int action` / `bool action`: Null vira 0 / True, e o
# resto passa por bool() no caso do bool. arg: 1 = int, 2 = bool.
COERCE_RET    = 70

# Nenhum `catch` casou: repropaga. Consome [mensagem, tipo] da pilha, porque o
# `finally` roda antes e pode ter sobrescrito o erro corrente da VM.
RERAISE       = 71

# ── diversos ───────────────────────────────────────────────────────────────
HALT          = 29

NOMES = {
    LOAD_CONST: "LOAD_CONST", LOAD_LOCAL: "LOAD_LOCAL", STORE_LOCAL: "STORE_LOCAL",
    LOAD_GLOBAL: "LOAD_GLOBAL", STORE_GLOBAL: "STORE_GLOBAL",
    ADD: "ADD", SUB: "SUB", MUL: "MUL", DIV: "DIV", MOD: "MOD", NEG: "NEG",
    LT: "LT", GT: "GT", LE: "LE", GE: "GE", EQ: "EQ", NE: "NE",
    JUMP: "JUMP", JUMP_IF_FALSE: "JUMP_IF_FALSE", POP_TOP: "POP_TOP",
    CALL: "CALL", RETURN: "RETURN", MAKE_FUNCTION: "MAKE_FUNCTION",
    BIT_OR: "BIT_OR", BIT_XOR: "BIT_XOR", BIT_AND: "BIT_AND",
    LSHIFT: "LSHIFT", RSHIFT: "RSHIFT", BIT_NOT: "BIT_NOT",
    BUILD_LIST: "BUILD_LIST", BUILD_DICT: "BUILD_DICT",
    INDEX_GET: "INDEX_GET", INDEX_SET: "INDEX_SET",
    ITER_NEXT: "ITER_NEXT", DUP: "DUP",
    BUILD_STR: "BUILD_STR", BUILD_TUPLE: "BUILD_TUPLE", SLICE: "SLICE",
    JUMP_IF_SET: "JUMP_IF_SET",
    LOAD_NAME: "LOAD_NAME", STORE_NAME: "STORE_NAME", CALL_KW: "CALL_KW",
    SETUP_TRY: "SETUP_TRY", POP_TRY: "POP_TRY", RAISE: "RAISE",
    PUSH_ERR_TYPE: "PUSH_ERR_TYPE",
    JUMP_IF_TRUE: "JUMP_IF_TRUE", LEN: "LEN", HAS_KEY: "HAS_KEY",
    MAKE_CLASS: "MAKE_CLASS", GET_MEMBER: "GET_MEMBER",
    SET_MEMBER: "SET_MEMBER", LOAD_SELF: "LOAD_SELF", CALL_BASE: "CALL_BASE",
    DUP2: "DUP2", IMPORT_MOD: "IMPORT_MOD",
    IS_OP: "IS_OP", IN_OP: "IN_OP", LOAD_TIPO: "LOAD_TIPO",
    COUNT: "COUNT", COUNT_PARES: "COUNT_PARES",
    CHECK_NONNULL: "CHECK_NONNULL", MAKE_MODEL: "MAKE_MODEL",
    UNPACK: "UNPACK", YIELD: "YIELD", CLOSE_SE_TEM: "CLOSE_SE_TEM",
    NOT: "NOT", TO_BOOL: "TO_BOOL",
    COERCE_DECL: "COERCE_DECL", COERCE_RET: "COERCE_RET", RERAISE: "RERAISE",
    HALT: "HALT",
}

# operador do AST → opcode, pra o compilador não precisar de if/elif
BINARIO: dict[str, int] = {
    "+": ADD, "-": SUB, "*": MUL, "/": DIV, "%": MOD,
    "<": LT, ">": GT, "<=": LE, ">=": GE,
    "==": EQ, "===": EQ, "!=": NE, "!==": NE,
    "|": BIT_OR, "^": BIT_XOR, "&": BIT_AND, "<<": LSHIFT, ">>": RSHIFT,
}
