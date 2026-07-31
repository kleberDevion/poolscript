"""VM em C — mesma semântica do interpretador, mais GC correto.

Pula a suíte inteira se a extensão não estiver compilada
(`python setup_vm.py build_ext --inplace`), pra não quebrar quem não
compilou ainda.

Como em test_vm.py, nenhum resultado é escrito à mão: tudo é conferido
contra o interpretador atual.
"""
import io
import os
import sys
from contextlib import redirect_stdout

import pytest

from poolscript.interpreter import Interpreter
from poolscript.parser import parse_source
from poolscript.vm import compila
from poolscript.vm.flatten import achata, para_c

cvm = pytest.importorskip(
    "poolscript.vm.poolscript_vm",
    reason="extensão C não compilada — rode: python setup_vm.py build_ext --inplace",
)


def via_c(src):
    """Roda na VM em C e devolve as linhas impressas.

    `post` e `len` são builtins NATIVOS: a saída sai por `printf` em C, não
    passa pelo Python. Por isso a captura é no nível do descritor de arquivo
    (os.dup2), e não com redirect_stdout — que só intercepta `sys.stdout` do
    Python e não enxergaria nada.
    """
    co, tab = compila(parse_source(src, "<test>"))
    protos = para_c(achata(co))

    r, w = os.pipe()
    fd_original = os.dup(1)
    try:
        os.dup2(w, 1)
        os.close(w)
        try:
            cvm.roda(protos, max(len(tab), 1), dict(tab))
        finally:
            sys.stdout.flush()
            os.dup2(fd_original, 1)
    finally:
        os.close(fd_original)

    partes = []
    with os.fdopen(r, "rb") as f:
        partes.append(f.read())
    texto = b"".join(partes).decode("utf-8")
    return texto.splitlines()


def via_interp(src):
    i = Interpreter(source=src, filename="<test>")
    with redirect_stdout(io.StringIO()):
        i.run(parse_source(src, "<test>"))
    return i.output


def mesmo(src):
    assert via_c(src) == via_interp(src)


# ── números nativos (V_INT / V_FLOAT) ───────────────────────────────────────
@pytest.mark.parametrize("src", [
    "post(2 + 3 * 4)", "post((2 + 3) * 4)", "post(10 - 3 - 2)",
    "post(7 / 2)", "post(7 % 3)", "post(0 - 5)",
    "post(1.5 + 2.5)", "post(1.5 * 2)",
])
def test_aritmetica(src):
    mesmo(src)


@pytest.mark.parametrize("src", [
    "post(3 < 5)", "post(5 <= 5)", "post(3 > 5)", "post(5 >= 5)",
    "post(2 == 2)", "post(2 != 3)",
])
def test_comparacao(src):
    mesmo(src)


def test_null_igual_zero():
    mesmo("post(Null == 0)")


@pytest.mark.parametrize("src", [
    "post(5 ^ 3)", "post(5 | 2)", "post(6 & 3)",
    "post(1 << 4)", "post(256 >> 4)", "post(~5)", "post(2 + 3 << 1)",
])
def test_bitwise(src):
    mesmo(src)


def test_modulo_segue_sinal_do_divisor():
    # semântica do Python, não a do C
    mesmo("post(0 - 7 % 3)")


# ── strings nativas (PSString sob o GC) ─────────────────────────────────────
@pytest.mark.parametrize("src", [
    'post("oi")',
    'post("a" + "b")',
    'post("po" + "ol" + "script")',
    'post("abc" == "abc")',
    'post("abc" == "abd")',
    'post("a" != "b")',
    'post("abc" < "abd")',
    'post("b" > "a")',
    'post("a" <= "a")',
    'post("ação" + " çã")',
    'str s = "pool"\npost(s + "script")',
])
def test_strings(src):
    mesmo(src)


def test_string_vazia_e_falsy():
    mesmo('str s = ""\nif (s) {\n post(1)\n} else {\n post(2)\n}')


def test_concat_em_loop():
    mesmo('str s = ""\nint i = 0\nwhile (i < 5) {\n s = s + "x"\n i += 1\n}\npost(s)')


def test_string_retornada_de_action():
    mesmo('action f() {\n return "vem daqui"\n}\npost(f())')


# ── controle de fluxo e funções ─────────────────────────────────────────────
def test_while():
    mesmo("int i = 0\nint s = 0\nwhile (i < 5) {\n s += i\n i += 1\n}\npost(s)")


@pytest.mark.parametrize("n", [15, 8, 3])
def test_if_elif_else(n):
    mesmo(f"int n = {n}\nif (n > 10) {{\n post(1)\n}} elif (n > 5) {{\n post(2)\n}} else {{\n post(3)\n}}")


def test_action_e_chamadas_aninhadas():
    mesmo("action d(n) {\n return n * 2\n}\npost(d(d(d(3))))")


def test_action_sem_return():
    mesmo("action nada() {\n int x = 1\n}\npost(nada())")


def test_recursao():
    mesmo("action fib(n) {\n if (n < 2) {\n  return n\n }\n return fib(n-1) + fib(n-2)\n}\npost(fib(18))")


def test_recursao_profunda():
    """Frames explícitos: recursão da PoolScript não consome pilha do C.

    Aqui NÃO dá pra comparar com o interpretador — o tree-walker recorre na
    pilha do Python e estoura o limite bem antes de 10.000 níveis. Justamente
    por isso o caso importa: é capacidade que só a VM tem.
    """
    assert via_c("action r(n) {\n if (n < 1) {\n  return 0\n }\n return 1 + r(n - 1)\n}\npost(r(10000))") == ["10000"]


def test_locais_nao_vazam():
    mesmo("action f(a) {\n int b = a * 2\n return b\n}\npost(f(5))\npost(f(7))")


# ── GC: o que está vivo NÃO pode ser coletado ───────────────────────────────
# Cada caso gera lixo suficiente pra disparar vários ciclos enquanto mantém
# um valor vivo numa raiz diferente.

_LIXO = 'lixo = "aaaaaaaaaabbbbbbbbbb" + "cccccccccc"'


def test_gc_preserva_global():
    src = (f'str guardada = "SOBREVIVE"\nstr lixo = ""\nint i = 0\n'
           f'while (i < 120000) {{\n {_LIXO}\n i += 1\n}}\npost(guardada)')
    assert via_c(src) == ["SOBREVIVE"]


def test_gc_preserva_local_de_funcao():
    src = ('action trabalha(marca) {\n'
           '    str lixo = ""\n    int i = 0\n'
           f'    while (i < 120000) {{\n        {_LIXO}\n        i += 1\n    }}\n'
           '    return marca + "_intacto"\n}\npost(trabalha("LOCAL"))')
    assert via_c(src) == ["LOCAL_intacto"]


def test_gc_preserva_constante():
    src = (f'str lixo = ""\nint i = 0\n'
           f'while (i < 120000) {{\n {_LIXO}\n i += 1\n}}\npost("CONSTANTE")')
    assert via_c(src) == ["CONSTANTE"]


def test_gc_preserva_valor_na_pilha():
    src = ('action g() {\n    str a = "PRIMEIRA"\n    str lixo = ""\n    int i = 0\n'
           f'    while (i < 120000) {{\n        {_LIXO}\n        i += 1\n    }}\n'
           '    return a + "_" + "SEGUNDA"\n}\npost(g())')
    assert via_c(src) == ["PRIMEIRA_SEGUNDA"]


def test_gc_realmente_coleta():
    """Muito lixo precisa gerar ciclos e liberar objetos — senão o GC não roda."""
    src = (f'str lixo = ""\nint i = 0\n'
           f'while (i < 150000) {{\n {_LIXO}\n i += 1\n}}\npost(1)')
    via_c(src)
    st = cvm.estatisticas()
    assert st["ciclos_gc"] > 0, "GC nunca rodou"
    assert st["objetos_liberados"] > 1000, "GC rodou mas não liberou nada"


# ── limites viram erro claro, não corrupção de memória ──────────────────────
def test_recursao_infinita_da_erro_claro():
    src = "action r(n) {\n return r(n + 1)\n}\npost(r(1))"
    with pytest.raises(RuntimeError, match="estouro"):
        via_c(src)


def test_aridade_errada_da_erro():
    with pytest.raises(RuntimeError, match="argumentos"):
        via_c("action f(a, b) {\n return a\n}\npost(f(1))")


def test_divisao_por_zero_da_erro():
    """A mensagem casa com a do interpretador — ela é OBSERVÁVEL.

    `catch (e) { post(e) }` imprime esse texto, então divergir aqui muda o
    comportamento visível do programa.
    """
    with pytest.raises(RuntimeError, match="divisão por zero"):
        via_c("post(1 / 0)")


def test_tipo_incompativel_da_erro():
    with pytest.raises(RuntimeError, match="incompativeis"):
        via_c('post("a" + 1)')


# ── PSList / PSDict nativos ─────────────────────────────────────────────────
@pytest.mark.parametrize("src", [
    "post([1, 2, 3])",
    'post([1, "dois", 3.5, True])',
    "post([[1, 2], [3, 4]])",
    "post([])",
    'post(["a", "b"])',
])
def test_lista_literal(src):
    mesmo(src)


def test_lista_indexacao():
    mesmo("lista = [10, 20, 30]\npost(lista[0])\npost(lista[2])")


def test_lista_indice_negativo():
    mesmo("lista = [10, 20, 30]\npost(lista[-1])\npost(lista[-3])")


def test_lista_aninhada():
    mesmo("l = [[1, 2], [3, 4]]\npost(l[1][0])")


def test_lista_retornada_de_action():
    mesmo("action f() {\n return [1, 2, 3]\n}\npost(f()[1])")


def test_dict_literal_e_acesso():
    mesmo('d = {"a": 1, "b": 2}\npost(d["a"])\npost(d["b"])')


def test_dict_chave_identificador():
    mesmo('d = {nome: "pool"}\npost(d["nome"])')


def test_dict_aninhado():
    mesmo('d = {"x": [1, 2], "y": {"z": 9}}\npost(d["x"][1])\npost(d["y"]["z"])')


def test_string_indexacao():
    mesmo('post("pool"[0])\npost("pool"[-1])')


# ── builtin nativo len() ────────────────────────────────────────────────────
@pytest.mark.parametrize("src", [
    "post(len([1, 2, 3, 4]))",
    'post(len("poolscript"))',
    'post(len({"a": 1, "b": 2, "c": 3}))',
    "post(len([]))",
])
def test_len_nativo(src):
    mesmo(src)


# ── verdade/falsidade das coleções ──────────────────────────────────────────
def test_lista_vazia_e_falsy():
    mesmo("l = []\nif (l) {\n post(1)\n} else {\n post(2)\n}")


def test_lista_com_itens_e_truthy():
    mesmo("l = [0]\nif (l) {\n post(1)\n} else {\n post(2)\n}")


def test_dict_vazio_e_falsy():
    mesmo("d = {}\nif (d) {\n post(1)\n} else {\n post(2)\n}")


# ── GC precisa enxergar DENTRO de listas e dicts ────────────────────────────
# Se a marcação não percorresse os filhos, o conteúdo seria coletado
# enquanto o container segue vivo — e o acesso devolveria lixo.

def test_gc_preserva_conteudo_de_lista_viva():
    src = ('viva = [111, 222, 333]\nstr lixo = ""\nint i = 0\n'
           f'while (i < 120000) {{\n {_LIXO}\n i += 1\n}}\npost(viva[2])')
    assert via_c(src) == ["333"]


def test_gc_preserva_conteudo_de_dict_vivo():
    src = ('vivo = {"chave": "VALOR_INTACTO"}\nstr lixo = ""\nint i = 0\n'
           f'while (i < 120000) {{\n {_LIXO}\n i += 1\n}}\npost(vivo["chave"])')
    assert via_c(src) == ["VALOR_INTACTO"]


def test_gc_preserva_estrutura_aninhada():
    """Marcação precisa descer vários níveis, não só um."""
    src = ('d = {"n": {"n": {"n": [1, [2, [3, [4]]]]}}}\nstr lixo = ""\nint i = 0\n'
           f'while (i < 120000) {{\n {_LIXO}\n i += 1\n}}\n'
           'post(d["n"]["n"]["n"][1][1][1][0])')
    assert via_c(src) == ["4"]


def test_gc_coleta_listas_descartadas():
    src = ('int i = 0\nwhile (i < 60000) {\n lixo = [1, 2, 3, 4, 5, 6, 7, 8]\n i += 1\n}\npost(1)')
    via_c(src)
    st = cvm.estatisticas()
    assert st["ciclos_gc"] > 0
    assert st["objetos_liberados"] > 1000


# ── erros de indexação são claros ───────────────────────────────────────────
def test_indice_fora_do_intervalo_avisa_e_devolve_null():
    """Spec da linguagem: índice fora do intervalo NÃO trava.

    Emite IndexOutOfBoundsWarning no stderr e devolve Null. A primeira
    versão levantava erro, o que era mais restritivo que a linguagem — só
    apareceu ao comparar com o interpretador.
    """
    assert via_c("post([1, 2][9])") == ["null"]
    assert via_c('post("ab"[9])') == ["null"]


def test_chave_inexistente():
    with pytest.raises(RuntimeError, match="chave inexistente"):
        via_c('post({"a": 1}["z"])')


def test_indice_de_lista_precisa_ser_int():
    with pytest.raises(RuntimeError, match="precisa ser int"):
        via_c('post([1, 2]["x"])')


def test_len_em_tipo_invalido():
    with pytest.raises(RuntimeError, match="len"):
        via_c("post(len(5))")


# ── divergência CONHECIDA e intencional ─────────────────────────────────────
def test_null_imprime_null_em_qualquer_profundidade():
    """`Null` é `null` no topo e aninhado, nos dois motores.

    Era divergência: o interpretador delegava ao repr da list do Python e
    `post([Null])` saía "[None]" — `None` vazando numa linguagem que não tem
    `None`. Resolvido do lado Python, com o interpretador renderizando
    coleções sozinho (`stringify`), em vez de a VM copiar o vazamento.
    """
    for src in ('post(Null)', 'post([Null])', 'post({"a": Null})',
                'post([[Null]])', 'post(str(Null))'):
        assert via_c(src) == via_interp(src)
    assert via_c("post([Null])") == ["[null]"]
    assert via_c("post(str(Null))") == ["null"]
