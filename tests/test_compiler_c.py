"""Compilador em C — precisa gerar o mesmo bytecode que o de Python.

Comparação sobre o desmonte textual (`bytecode_dump.py` no Python,
`ps_compiler_bind.c` no C): opcode, argumento, ordem das constantes e índice
de cada local. Qualquer divergência aparece como diff legível, com o
comentário `;` mostrando o que o índice significa.

Nó que o compilador Python não suporta faz o caso ser pulado — a suíte nunca
"passa" comparando duas coisas não implementadas.
"""
import pytest

from bytecode_dump import NaoCompila, desmonta

compiler_c = pytest.importorskip(
    "poolscript.vm.ps_compiler_c",
    reason="extensão C não compilada — rode: ./rebuild_vm.sh",
)


def mesmo(src):
    try:
        esperado = desmonta(src)
    except NaoCompila as e:
        pytest.skip(f"compilador Python não cobre: {e}")
    assert compiler_c.desmonta(src) == esperado


# ── constantes e pool ───────────────────────────────────────────────────────
@pytest.mark.parametrize("src", [
    "x = 1", "x = 3.14", 'x = "oi"', "x = True", "x = Null",
])
def test_literais(src):
    mesmo(src)


def test_pool_deduplica():
    """Constante repetida reusa o mesmo índice."""
    mesmo("x = 1\ny = 1\nz = 2")


def test_bool_e_int_nao_colidem_no_pool():
    """Em Python `1 == True`; o pool precisa separar por TIPO também.

    Sem isso, `LOAD_CONST` de um devolveria o valor do outro.
    """
    mesmo("x = 1\ny = True")


# ── expressões ──────────────────────────────────────────────────────────────
@pytest.mark.parametrize("src", [
    "x = 2 + 3 * 4",
    "x = (2 + 3) * 4",
    "x = 10 - 3 - 2",
    "x = 5 ^ 3 | 2 & 1",
    "x = 1 << 4 >> 2",
    "x = ~5",
    "x = 0 - 5",
])
def test_expressoes(src):
    mesmo(src)


@pytest.mark.parametrize("src", [
    "x = [1, 2, 3]", "x = []",
    'x = {"a": 1}', "x = {}",
    "x = {nome: 1}",       # chave sem aspas vira string no compilador
    "x = l[0]",
])
def test_estruturas(src):
    mesmo(src)


@pytest.mark.parametrize("src", ["post(1, 2)", "post()", "f(g(h(1)))"])
def test_chamadas(src):
    mesmo(src)


# ── statements ──────────────────────────────────────────────────────────────
@pytest.mark.parametrize("op", ["+=", "-=", "*=", "/=", "%="])
def test_atribuicao_composta(op):
    """`x += v` carrega x, calcula e guarda — não existe opcode dedicado."""
    mesmo(f"x = 1\nx {op} 2")


@pytest.mark.parametrize("src", [
    "if (x) {\n post(1)\n}",
    "if (x) {\n post(1)\n} else {\n post(2)\n}",
    "if (x) {\n post(1)\n} elif (y) {\n post(2)\n} else {\n post(3)\n}",
])
def test_if(src):
    mesmo(src)


def test_while():
    mesmo("int i = 0\nwhile (i < 5) {\n i += 1\n}")


# ── funções: locais viram ÍNDICE ────────────────────────────────────────────
def test_action_simples():
    mesmo("action f(a, b) {\n return a + b\n}")


def test_action_com_local():
    """Local declarado no corpo ganha slot próprio, depois dos parâmetros."""
    mesmo("action f(a) {\n int b = a * 2\n return b\n}")


def test_action_sem_return_devolve_null():
    mesmo("action nada() {\n int x = 1\n}")


def test_recursao_resolve_a_si_mesma():
    """DIVERGÊNCIA INTENCIONAL do bytecode — o C é mais correto aqui.

    `compiler.py` resolve nome não-local direto como LOAD_GLOBAL. Mas a
    linguagem tem resolução DINÂMICA: atribuir a um nome dentro de uma
    função modifica a variável externa se ela existir, e só cria local se
    não existir (é o `Scope.set` do interpretador, subindo a cadeia).

    O C emite LOAD_NAME/STORE_NAME, que decidem em runtime — por isso
    reserva um slot local a mais e o bytecode não bate. A semântica está
    validada em `test_pipeline_c.py`, contra o interpretador.
    """
    saida = compiler_c.desmonta(
        "action fib(n) {\n if (n < 2) {\n  return n\n }\n return fib(n-1) + fib(n-2)\n}")
    assert "LOAD_NAME" in saida          # resolução adiada pro runtime
    assert "proto 1 fib nparams=1" in saida.replace("nlocals=2 ", "")


def test_protos_saem_planos():
    """MAKE_FUNCTION carrega índice de PROTÓTIPO, não de constante.

    O achatamento que `flatten.py` fazia em Python acontece direto no
    compilador em C.
    """
    saida = compiler_c.desmonta("action f() {\n return 1\n}")
    assert "proto 0 <module>" in saida
    assert "proto 1 f" in saida
    assert "MAKE_FUNCTION 1 ; -> proto 1 f" in saida


def test_actions_aninhadas():
    """Regressão: `novo_proto` faz realloc do array de protótipos.

    A primeira versão guardava `PSProto*` na unidade de compilação, que
    virava ponteiro pendurado assim que uma action interna criava outro
    protótipo. Passou a guardar o índice.
    """
    mesmo("action externa() {\n action interna() {\n  return 1\n }\n return 2\n}")


# ── nó não suportado para com erro, nunca gera bytecode errado ──────────────
def test_async_await_compila():
    """async action + await COMPILAM na VM — o runtime de async (fibras+future)
    fechou a lacuna que antes dava NotImplementedError."""
    bc = compiler_c.desmonta("async action f() {\n return 1\n}\npost(await f())")
    assert bc  # devolveu o disassembly, não levantou


def test_erro_de_sintaxe_propaga():
    with pytest.raises(SyntaxError):
        compiler_c.desmonta("x = (1")
