"""Builtins nativos em C — mesma saída que o interpretador.

A camada 4 da migração. Cada builtin aqui é código C que substitui uma função
Python; a checagem é a mesma do resto: rodar os dois motores e comparar a
saída, com o **interpretador como autoridade**.

Cobre também o caminho de erro de cada um. Builtin que falha tem que levantar
igual e ser capturável por `try` — não abortar a execução, que era o
comportamento antigo da VM.
"""
import io
import os
import sys
from contextlib import redirect_stdout

import pytest

from poolscript.interpreter import Interpreter
from poolscript.parser import parse_source

vm = pytest.importorskip(
    "poolscript.vm.poolscript_vm",
    reason="extensão C não compilada — rode: ./rebuild_vm.sh",
)


def via_interpretador(src):
    i = Interpreter(source=src, filename="<test>")
    with redirect_stdout(io.StringIO()):
        i.run(parse_source(src, "<test>"))
    return i.output


def via_c(src):
    sys.stdout.flush()
    r, w = os.pipe()
    original = os.dup(1)
    try:
        os.dup2(w, 1)
        os.close(w)
        try:
            vm.executa_fonte(src)
        finally:
            sys.stdout.flush()
            os.dup2(original, 1)
    finally:
        os.close(original)
    with os.fdopen(r, "rb") as f:
        return f.read().decode("utf-8").splitlines()


# Fonte `.ps` é multilinha; `chr(10)` evita a confusão de escape ao montar os
# casos, que já custou um lote inteiro de teste com "\n" literal no meio.
NL = chr(10)


def mesmo(src):
    assert via_c(src) == via_interpretador(src)


def ambos_falham(src):
    with pytest.raises(Exception):
        via_interpretador(src)
    with pytest.raises(Exception):
        via_c(src)


@pytest.mark.parametrize("src", [
    'post(true < 3)',            # bool comparado com int
    'post(false < true)',        # bool com bool
    'post(true + 1)',            # bool em aritmética -> int
    'post(true + true + false)',
    'post(true * 5, false * 5)',
    'post((1 < 2) < 3)',         # resultado bool volta pra comparação
    'post(true - 1, true / 2, true % 2)',
])
def test_bool_conta_como_int(src):
    """Python: bool é subclasse de int. A VM tratava bool como tipo à parte
    em comparação/aritmética ('tipos incompativeis'); agora coage 0/1 igual
    ao interp. (achado pela varredura diferencial)"""
    mesmo(src)


# ═════════════════════════ conversão e tipo ═════════════════════════════════

@pytest.mark.parametrize("src", [
    "post(str(1))", "post(str(1.5))", "post(str(True))", "post(str(Null))",
    'post(str("ab"))', "post(str([1,2]))", 'post(str([1,"a"]))',
    'post(str({"a":1}))', "post(str((1,2)))", 'post(str(1) + "x")',
])
def test_str(src):
    mesmo(src)


@pytest.mark.parametrize("src", [
    'post(int("42"))', "post(int(3.9))", "post(int(-3.9))", "post(int(True))",
    'post(int("  7 "))', "post(int(1))", 'post(int("-5"))', 'post(int("+5"))',
])
def test_int(src):
    """Trunca para zero (não arredonda) e aceita espaço em volta, como o Python."""
    mesmo(src)


@pytest.mark.parametrize("src", [
    'post(flo("1.5"))', "post(flo(2))", "post(flo(1.5))", "post(flo(True))",
])
def test_flo(src):
    mesmo(src)


@pytest.mark.parametrize("src", [
    "post(bool(0))", 'post(bool(""))', "post(bool([]))", "post(bool([1]))",
    "post(bool(Null))", "post(bool(1))", "post(bool({}))",
])
def test_bool(src):
    mesmo(src)


@pytest.mark.parametrize("src", [
    "post(type(1))", "post(type(1.5))", 'post(type("a"))', "post(type(True))",
    "post(type(Null))", "post(type([1]))", 'post(type({"a":1}))', "post(type((1,2)))",
])
def test_type(src):
    mesmo(src)


# ═════════════════════════ numéricos ════════════════════════════════════════

@pytest.mark.parametrize("src", [
    "post(abs(-3))", "post(abs(-3.5))", "post(abs(3))", "post(abs(True))", "post(abs(0))",
])
def test_abs(src):
    mesmo(src)


@pytest.mark.parametrize("src", [
    "post(round(3.7))", "post(round(3.14159, 2))", "post(round(3))",
    # meio-para-o-par, que é o do Python: 2.5 vira 2, não 3
    "post(round(2.5))", "post(round(3.5))", "post(round(-2.5))",
    "post(round(0.5))", "post(round(1.5))",
    # 2.675 é 2.67499… em binário, então arredonda pra baixo nos dois
    "post(round(2.675, 2))",
])
def test_round(src):
    mesmo(src)


@pytest.mark.parametrize("src", [
    "post(hex(255))", "post(hex(-255))", "post(hex(0))",
    "post(bin(10))", "post(bin(0))", "post(bin(-5))",
    "post(oct(15))", "post(oct(0))",
])
def test_bases(src):
    """O sinal vem antes do prefixo: `-0xff`, não `0x-ff`."""
    mesmo(src)


@pytest.mark.parametrize("src", [
    'post(ord("A"))', 'post(ord("ç"))', 'post(ord("λ"))',
    "post(chr(65))", "post(chr(955))", "post(chr(128512))",
])
def test_ord_chr(src):
    """Trabalham em codepoint, não em byte: `ord(\"ç\")` é 231, não 195."""
    mesmo(src)


def test_float_usa_a_menor_representacao():
    """`1/3` é "0.3333333333333333" — `%.17g` daria um dígito de lixo a mais."""
    mesmo("post(1/3)")
    mesmo("post(0.1 + 0.2)")
    mesmo("post(1.0)")
    mesmo("post(1/7)")
    assert via_c("post(1/3)") == ["0.3333333333333333"]


# ═════════════════════════ sequências ═══════════════════════════════════════

@pytest.mark.parametrize("src", [
    "post(range(3))", "post(range(1,4))", "post(range(0,10,2))",
    "post(range(3,0,-1))", "post(range(0))", "post(range(-3))",
    "post(range(5,1))", "post(range(10,0,-3))", "post(len(range(100)))",
])
def test_range(src):
    mesmo(src)


@pytest.mark.parametrize("src", [
    "post(list())", 'post(list("ab"))', "post(list([1,2]))",
    "post(list((1,2)))", 'post(list({"a":1}))', 'post(list(""))',
])
def test_list(src):
    mesmo(src)


@pytest.mark.parametrize("src", [
    "post(sum([1,2,3]))", "post(sum([1.5,2]))", "post(sum([]))",
    "post(sum([1,2.5,3]))", "post(sum([True,1]))", "post(sum(range(100)))",
])
def test_sum(src):
    mesmo(src)


@pytest.mark.parametrize("src", [
    "post(min([3,1,2]))", "post(max([3,1,2]))", "post(min(3,1))", "post(max(1,2,3))",
    "post(min([1.5,2]))", 'post(min(["b","a"]))', 'post(max("a","b"))',
    'post(max("ab"))', "post(min([1,2],[3]))",
])
def test_min_max(src):
    """Um argumento itera; vários comparam os próprios. Lista contra lista é
    lexicográfico, então `min([1,2],[3])` é `[1,2]`."""
    mesmo(src)


@pytest.mark.parametrize("src", [
    "post(sorted([3,1,2]))", 'post(sorted(["b","a"]))', "post(sorted([]))",
    "post(sorted((3,1)))", "post(sorted([2.5,1]))", "post(sorted(reversed([1,2,3])))",
])
def test_sorted(src):
    mesmo(src)


@pytest.mark.parametrize("src", [
    "post(reversed([1,2,3]))", 'post(reversed("ab"))', "post(reversed([]))",
])
def test_reversed(src):
    mesmo(src)


@pytest.mark.parametrize("src", [
    'post(enumerate(["a","b"]))', "post(enumerate([]))",
    'post(zip([1,2],["a","b"]))', 'post(zip([1,2,3],["a"]))',
    "post(zip())", "post(zip([1],[2],[3]))",
])
def test_enumerate_zip(src):
    mesmo(src)


@pytest.mark.parametrize("src", [
    "l = [1]\naddEnd(l, 2)\npost(l)",
    "l = [1,2]\npost(removeEnd(l))\npost(l)",
    "l = [1]\naddStart(l, 0)\npost(l)",
    "l = [1,2]\npost(removeStart(l))\npost(l)",
    "l = []\naddEnd(l,1)\naddEnd(l,2)\naddStart(l,0)\npost(l)",
    # remover de vazia devolve Null, não é erro
    "l = []\npost(removeEnd(l))",
    "l = []\npost(removeStart(l))",
])
def test_mutacao_de_lista(src):
    mesmo(src)


# ═════════════════════════ map / filter ════════════════════════════════════
# Primeiros builtins que REENTRAM na VM: recebem uma action da PoolScript e a
# chamam item a item, a partir de C. O laço aninhado roda acima da marca
# d'água de sp/locals/frames publicada por quem chamou.

@pytest.mark.parametrize("src", [
    "action d(x) {\n return x * 2\n}\npost(map([1,2,3], d))",
    "action d(x) {\n return x * 2\n}\npost(map([], d))",
    "action n(x) {\n}\npost(map([1,2], n))",
    'action f(x) {\n return x\n}\npost(map([1,"a",True], f))',
    # a action chamada chama outra
    "action s(x) {\n return x + 1\n}\naction t(x) {\n return s(x) * 10\n}\npost(map([1,2], t))",
    # e recursão dentro do map
    "action r(x) {\n if x <= 1 {\n  return 1\n }\n return x * r(x-1)\n}\npost(map([3,4,5], r))",
    # map de map
    "action d(x) {\n return x * 2\n}\npost(map(map([1,2], d), d))",
    # builtin nativo como argumento
    "f = abs\npost(map([1,-2,3], f))",
])
def test_map(src):
    mesmo(src)


@pytest.mark.parametrize("src", [
    "action p(x) {\n return x > 2\n}\npost(filter([1,2,3,4], p))",
    "action p(x) {\n return False\n}\npost(filter([1,2,3], p))",
    "action p(x) {\n return True\n}\npost(filter([1,2,3], p))",
    "action p(x) {\n return x\n}\npost(filter([0,1,2], p))",
    "action p(x) {\n return True\n}\npost(filter([], p))",
])
def test_filter(src):
    mesmo(src)


@pytest.mark.parametrize("src", [
    "post(map(1, str))", "post(map([1]))", "post(filter([1], 5))", "post(map([1], 5))",
])
def test_map_filter_erro(src):
    """Segundo argumento não-chamável era `[]` silencioso no interpretador."""
    ambos_falham(src)


def test_erro_dentro_da_action_do_map_propaga():
    src = ("action bad(x) {\n return x / 0\n}\n"
           'try {\n post(map([1], bad))\n} catch (e) {\n post("pego")\n}\n')
    mesmo(src)
    assert via_c(src) == ["pego"]


# ═════════════════════════ ordem do dict ════════════════════════════════════
# O dict da VM é compacto (denso em ordem de inserção + tabela de índices).
# Com tabela hash pura as chaves saíam na ordem do hash e a saída de um `.ps`
# deixava de ser reproduzível.

@pytest.mark.parametrize("src", [
    'post({"b":1,"a":2})',
    'post({"z":1,"y":2,"x":3})',
    'post({"a":1,"b":2,"c":3,"d":4,"e":5})',
    'post({1:"a",2:"b"})',
    'post({1.5:"a", True:"b"})',
    'd = {}\nd["b"] = 1\nd["a"] = 2\npost(d)',
    'post(list({"b":1,"a":2}))',
    # sobrescrever não muda a posição na ordem
    'd = {"a":1,"b":2}\nd["a"] = 9\npost(d)',
])
def test_dict_preserva_ordem_de_insercao(src):
    mesmo(src)


def test_dict_grande_mantem_ordem_apos_crescer():
    """Passa de várias realocações — a ordem tem que sobreviver ao rehash."""
    src = ("d = {}\ni = 0\nwhile i < 200 {\n d[i] = i\n i++\n}\n"
           "post(len(d))\npost(list(d)[0])\npost(list(d)[199])\n")
    mesmo(src)


# ═════════════════════════ erros ════════════════════════════════════════════

@pytest.mark.parametrize("src", [
    'post(int("x"))', 'post(int("0x1f"))', "post(int(Null))", 'post(int(""))',
    "post(flo(Null))", 'post(flo("abc"))', 'post(flo("1.5abc"))',
    "post(chr(-1))", "post(chr(1114112))", 'post(ord("ab"))', 'post(ord(""))',
    "post(ord(1))", "post(hex(1.5))", 'post(bin("a"))', 'post(abs("a"))',
    "post(len(1))", "post(str())", "post(str(1,2))", 'post(round("a"))',
    'post(sum([1,"a"]))', "post(sum(1))", 'post(sorted([1,"a"]))', "post(sorted(1))",
    "post(min([]))", "post(max([]))", "post(range())", "post(range(1,2,0))",
    "post(list(1))", "post(addEnd(1,2))", "post(removeEnd(1))",
    "post(zip(1))", "post(enumerate(1))", "post(reversed(1))",
])
def test_erro_de_builtin(src):
    ambos_falham(src)


@pytest.mark.parametrize("src", [
    'try {\n post(len(1))\n} catch (e) {\n post("pego")\n}',
    'try {\n post(int("x"))\n} catch (e) {\n post("pego")\n}',
    'try {\n post(min([]))\n} catch (e) {\n post("pego")\n}',
])
def test_erro_de_builtin_e_capturavel(src):
    """A VM saía por `return` no erro de builtin, escapando do `try`."""
    mesmo(src)
    assert via_c(src) == ["pego"]


# ═════════════════════════ métodos de string ═══════════════════════════════
# O despacho: `GET_MEMBER` numa string devolve um OBJ_METODO_NAT — o método
# preso ao valor. É objeto (e não chamada direta) porque `f = texto.upper`
# tem que poder circular como valor antes de ser chamado.
#
# Tudo aqui trabalha em CODEPOINT, não em byte: a string é UTF-8, e `"ção"`
# tem 3 caracteres e 5 bytes. Caixa cobre ASCII + Latin-1 + Latin Estendido-A,
# que é a faixa do português.

@pytest.mark.parametrize("src", [
    'post("ab".upper())',
    'post("AB".lower())',
    'post("ção".upper())',
    'post("ÇÃO".lower())',
    'post("olá mundo".title())',
    'post("olá mundo".capitalize())',
    'post("aBc".swapcase())',
    'post("AbC".casefold())',
    'post("".upper())',
    'post("a1b".title())',
])
def test_str_caixa(src):
    mesmo(src)


@pytest.mark.parametrize("src", [
    'post("  x  ".strip())',
    'post("xxaxx".strip("x"))',
    'post("  x".lstrip())',
    'post("x  ".rstrip())',
    'post("".strip())',
    'post("   ".strip())',
    'post("abcba".strip("ab"))',
    'post("\\tx\\n".strip())',
])
def test_str_apara(src):
    """`strip(chars)` é um CONJUNTO de caracteres, não um prefixo."""
    mesmo(src)


@pytest.mark.parametrize("src", [
    'post("abc".startswith("ab"))',
    'post("abc".startswith("z"))',
    'post("abc".endswith("bc"))',
    'post("abc".contains("b"))',
    'post("abc".has("z"))',
    'post("abcabc".find("c"))',
    'post("abcabc".rfind("c"))',
    'post("abc".find("z"))',
    'post("abc".index("b"))',
    'post("abcabc".count("a"))',
    'post("abc".count(""))',
    'post("ção".len())',
    'post("".len())',
    'post("çãoção".find("o"))',
])
def test_str_busca(src):
    """`find`/`index` devolvem posição em codepoint, não em byte."""
    mesmo(src)


@pytest.mark.parametrize("src", [
    'post("abc".isalpha())',
    'post("ção".isalpha())',
    'post("123".isdigit())',
    'post("a1".isalnum())',
    'post("  ".isspace())',
    'post("AB".isupper())',
    'post("ab".islower())',
    'post("Ab Cd".istitle())',
    'post("ab".isascii())',
    'post("ção".isascii())',
    'post("".isalpha())',
    'post("12".isnumeric())',
    'post("a".isprintable())',
    'post("AÇÃO".isupper())',
])
def test_str_teste(src):
    """isupper/islower/istitle exigem ao menos um caractere com caixa."""
    mesmo(src)


@pytest.mark.parametrize("src", [
    'post("a,b,c".split(","))',
    'post("a b  c".split())',
    'post("a,b,c".split(",",1))',
    'post("".split(","))',
    'post(",a,".split(","))',
    'post("  ".split())',
    'post("a\\nb\\nc".splitlines())',
    'post("a\\r\\nb".splitlines())',
    'post("".splitlines())',
    'post("-".join(["a","b"]))',
    'post("".join(["a","b"]))',
    'post(",".join([]))',
    'post("aXbXc".replace("X","-"))',
    'post("aXbXc".replace("X","-",1))',
    'post("abc".replace("z","y"))',
    'post("a".replace("","x"))',
    'post("ab".replace("","-"))',
    'post("aXb".replace("X",""))',
])
def test_str_quebra(src):
    """`split()` sem separador quebra em corrida de branco e descarta bordas
    vazias; com separador, cada ocorrência gera campo, inclusive vazio."""
    mesmo(src)


@pytest.mark.parametrize("src", [
    'post("a=b".partition("="))',
    'post("a=b=c".partition("="))',
    'post("a=b=c".rpartition("="))',
    'post("abc".partition("z"))',
    'post("abc".rpartition("z"))',
    'post("pre-x".removeprefix("pre-"))',
    'post("x.txt".removesuffix(".txt"))',
    'post("x".removeprefix("z"))',
])
def test_str_particao(src):
    """`partition` devolve sempre 3 partes; sem separador o texto vai pra frente
    e no `rpartition` vai pro fim."""
    mesmo(src)


@pytest.mark.parametrize("src", [
    'post("ab".ljust(5))',
    'post("ab".rjust(5))',
    'post("ab".center(6))',
    'post("ab".center(5))',
    'post("abc".center(7))',
    'post("a".center(4))',
    'post("ab".ljust(5,"*"))',
    'post("5".zfill(3))',
    'post("-5".zfill(4))',
    'post("abc".zfill(2))',
    'post("ção".ljust(5,"."))',
    'post("ção".center(7,"-"))',
    'post("a\\tb".expandtabs(4))',
    'post("a\\tb".expandtabs())',
])
def test_str_preenchimento(src):
    """Largura em codepoint. `center` joga a sobra ímpar pra esquerda quando
    largura e folga são ambas ímpares — é a fórmula do CPython."""
    mesmo(src)


@pytest.mark.parametrize("src", [
    'post("a".naoexiste())',
    'post("a".upper(1))',
    'post("a".split(""))',
    'post("a".index("z"))',
    'post("a".rindex("z"))',
    'post("a".join([1]))',
    'post("a".ljust("x"))',
    'post("a".center(3,"xy"))',
    'post("a".partition(""))',
    'post("a".count())',
    'post("a".startswith(1))',
    'post([1].upper())',
])
def test_str_erro(src):
    ambos_falham(src)


def test_metodo_de_string_como_valor():
    """`f = texto.upper` circula antes de ser chamado, igual a método de Entity."""
    mesmo('f = "abc".upper\npost(f())')


def test_encadeamento():
    mesmo('post("  Ab  ".strip().lower())')
    mesmo('post("-".join("a,b".split(",")))')
    mesmo('post("x".upper().len())')


@pytest.mark.parametrize("src", [
    "post((1).upper())", "post((150).isdigit())", 'post((1.5).contains("."))',
    "post((123).zfill(5))",
])
def test_numero_recebe_metodo_de_string(src):
    """`(150).isdigit()` vale sem str() na frente. `len` e `bool` ficam de fora.

    O número não vem do `input()` — esse sempre devolveu str. Vem de declaração
    tipada (`int x = "150"`) ou de `int(v)` explícito.
    """
    mesmo(src)


@pytest.mark.parametrize("src", ["post((12).len())", "post(True.isdigit())", "post([1].upper())"])
def test_sem_conversao_automatica(src):
    ambos_falham(src)


def test_metodo_de_string_como_callback():
    """Junta as duas peças novas: reentrância na VM e método nativo ligado."""
    mesmo('action u(x) {\n return x.upper()\n}\npost(map(["a","b","ç"], u))')
    mesmo('action p(x) {\n return x.isdigit()\n}\npost(filter(["1","a","2"], p))')



# ═════════════════════════ métodos de list e dict ══════════════════════════
# Mesma máquina dos métodos de string: `GET_MEMBER` devolve um OBJ_METODO_NAT,
# agora com o campo `tabela` dizendo de onde veio (str / list / dict / universal).

@pytest.mark.parametrize("src", [
    'd={"a":1,"b":2}\npost(d.keys())',
    'd={"a":1}\npost(d.values())',
    'd={"a":1}\npost(d.items())',
    'd={"a":1,"b":2}\npost(d.items())',
    'd={}\npost(d.keys())',
    'd={}\npost(d.len())',
    'd={"a":1}\npost(d.get("a"))',
    'd={"a":1}\npost(d.get("z"))',
    'd={"a":1}\npost(d.get("z",9))',
    'd={"a":1}\npost(d.has("a"))',
    'd={"a":1}\npost(d.contains("z"))',
    'd={"a":1}\npost(d.len())',
    'd={"a":1,"b":2}\npost(d.pop("a"))\npost(d)',
    'd={"a":1}\npost(d.pop("z",7))',
    'd={"a":1}\nd.update({"b":2})\npost(d)',
    'd={"a":1}\nd.update(d)\npost(d)',
    'd={"a":1}\nd.clear()\npost(d)',
    'd={"a":1}\ne=d.copy()\ne["b"]=2\npost(d)\npost(e)',
    'd={"b":1,"a":2}\npost(d.keys())',
    'd={"a":1,"b":2}\nd.pop("a")\nd["c"]=3\npost(d.keys())',
])
def test_dict_metodos(src):
    """keys/values/items percorrem o array denso, então saem em ordem de inserção."""
    mesmo(src)


@pytest.mark.parametrize("src", [
    'l=[1,2]\nl.append(3)\npost(l)',
    'l=[1,2]\npost(l.pop())\npost(l)',
    'l=[1,2,3]\npost(l.pop(0))',
    'l=[1,2,3]\npost(l.pop(-1))',
    'l=[1,2]\nl.insert(0,9)\npost(l)',
    'l=[1,2]\nl.insert(99,9)\npost(l)',
    'l=[1,2]\nl.insert(-1,9)\npost(l)',
    'l=[1,2,1]\nl.remove(1)\npost(l)',
    'l=[3,1,2]\nl.sort()\npost(l)',
    'l=[]\nl.sort()\npost(l)',
    'l=["b","a"]\nl.sort()\npost(l)',
    'l=[1,2]\nl.reverse()\npost(l)',
    'l=[]\nl.reverse()\npost(l)',
    'l=[1,2,1]\npost(l.index(2))',
    'l=[1,2,1]\npost(l.count(1))',
    'l=[1,2]\nl.clear()\npost(l)',
    'l=[1]\nl.extend([2,3])\npost(l)',
    'l=[1]\nl.extend(l)\npost(l)',
    'l=[1]\nm=l.copy()\nm.append(2)\npost(l)\npost(m)',
    'post([1,2].len())',
])
def test_list_metodos(src):
    mesmo(src)


@pytest.mark.parametrize("src", [
    'post([1,2].type())',
    'post("a".type())',
    'post((1).type())',
    'post((1.5).type())',
    'post({"a":1}.type())',
    'post(Null.type())',
    'post(True.type())',
    'post((1,2).type())',
])
def test_metodo_type_universal(src):
    """`.type()` vale em qualquer valor. Cuidado: devolve nomes DIFERENTES do
    builtin `type(x)` — `dict`/`tup` aqui contra `json`/`tuple` lá. É
    inconsistência do interpretador, reproduzida de propósito."""
    mesmo(src)


@pytest.mark.parametrize("src", [
    'post([].pop())',
    'post([1].pop(9))',
    'post([1].remove(9))',
    'post([1].index(9))',
    'post([1].append())',
    'post([1].sort(1))',
    'post({"a":1}.pop("z"))',
    'post({"a":1}.update([1]))',
    'post([1].extend(1))',
    'post([1].insert("a",1))',
    'post("a".append(1))',
    'post([1].keys())',
    'post({"a":1}.append(1))',
    'post((1).keys())',
    'post(Null.keys())',
    'post([1].type(1))',
])
def test_metodo_colecao_erro(src):
    ambos_falham(src)


def test_mutacao_por_metodo_e_visivel_fora():
    """`l.append()` altera a lista original, não uma cópia."""
    mesmo("l = [1]" + NL + "action f(x) {" + NL + " x.append(2)" + NL + "}" + NL + "f(l)" + NL + "post(l)")
    mesmo('d = {}' + NL + 'action g(x) {' + NL + ' x.update({"a": 1})' + NL + '}' + NL + 'g(d)' + NL + 'post(d)')


# ═════════════════════════ módulos nativos ═════════════════════════════════
# `import` compila para IMPORT_MOD, que resolve na tabela de módulos da VM em
# runtime — o compilador não conhece essa tabela. `from x import a` vira
# IMPORT_MOD + GET_MEMBER, reusando a mesma máquina de membro.
#
# Só módulo nativo de nome simples: `import a.b`, import relativo e arquivo
# `.ps` do usuário param com erro explícito (precisam de carregador de módulo).

@pytest.mark.parametrize("src", [
    'import json\npost(json.stringify({"a":1}))',
    'import json\npost(json.stringify([1,"a",True,Null]))',
    'import json\npost(json.stringify({"n":1.5,"l":[1,2],"d":{"x":Null}}))',
    'import json\npost(json.stringify([]))',
    'import json\npost(json.stringify({}))',
    'import json\npost(json.stringify("ção"))',
    'import json\npost(json.stringify({"t":"as\\"pas"}))',
    'import json\npost(json.stringify(150.0))',
    'import json\npost(json.parse("[1, 2, 3]")[2])',
    'import json\npost(json.parse("true"))',
    'import json\npost(json.parse("null"))',
    'import json\npost(json.parse("1.5"))',
    'import json\npost(json.parse("{\\"n\\": -1.5e2}")["n"])',
    'import json\npost(json.parse("  {\\"a\\" : [1, {\\"b\\": null}] }  ")["a"][1]["b"])',
    'import json\npost(json.parse("\\"\\\\u00e7\\""))',
    'import json\npost(json.parse("[]"))',
    'import json\npost(json.parse([1,2]))',
    'import json\npost(json.stringify(json.parse("{\\"b\\":1,\\"a\\":2}")))',
    'from json import stringify\npost(stringify([1,2]))',
    'from json import stringify as sj, parse\npost(sj([1]))\npost(parse("[2]")[0])',
    'import json as j\npost(j.stringify("oi"))',
    'import json\nf=json.stringify\npost(f([1]))',
    'import json\npost(map([[1],[2]], json.stringify))',
    'import date\npost(date.hora(1))',
    'import date\npost(date.hora(0,30))',
    'import date\npost(date.hora(0,0,1))',
    'import date\npost(date.hora(1,30,2))',
    'import date\npost(date.hora())',
    'from date import hora\npost(hora(2))',
    'import date\npost(date.timestamp() > 1700000000)',
    'import date\npost(date.today().len())',
    'import date\npost(date.time().len())',
    'import date\npost(date.datahora().len())',
    'import date\npost(date.now().len())',
])
def test_modulos_nativos(src):
    mesmo(src)


@pytest.mark.parametrize("src", [
    'import naoexiste',
    'import json\npost(json.naoexiste())',
    'import json\npost(json.parse("{"))',
    'import json\npost(json.parse("[1,"))',
    'import json\npost(json.parse(""))',
    'import json\npost(json.parse("lixo"))',
    'import json\npost(json.parse("1 2"))',
    'import json\npost(json.parse("{1: 2}"))',
    'import json\npost(json.parse("[1] x"))',
    'import json\npost(json.parse(1))',
    'import json\npost(json.parse())',
    'import date\npost(date.today(1))',
    'import date\npost(date.hora("a"))',
])
def test_modulo_erro(src):
    ambos_falham(src)


def test_json_recusa_controle_cru_na_string():
    """JSON estrito, igual ao `json.loads`: `"a<LF>b"` cru é erro."""
    ambos_falham('import json' + NL + 'post(json.parse("' + chr(92) + '"a' + NL + 'b' + chr(92) + '""))')


@pytest.mark.parametrize("src", [
    "import a.b" + NL + "post(1)",
    "from . import x" + NL + "post(1)",
    "import mod_que_nao_existe_xyz" + NL + "post(1)",
])
def test_import_ainda_nao_suportado_para_com_erro(src):
    """Nunca gera bytecode que finge ter importado.

    O `jinker` morava aqui enquanto não era migrado; com as 18 libs portadas,
    o caso vira um módulo inexistente. O que o teste protege é o
    comportamento: módulo que a VM não tem para a execução, não devolve um
    objeto vazio."""
    with pytest.raises(Exception):
        via_c(src)


# ═════════════════════════ regex ═══════════════════════════════════════════
# Motor próprio em `ps_regex.c` — backtracking, como o `re` do Python. A
# escolha importa: um autômato daria OUTRA resposta em `(a|ab)c`, e o
# requisito aqui é casar a semântica, não só achar um casamento.
#
# Classes e `.` trabalham em CODEPOINT: `.` não pode partir um UTF-8 no meio.

@pytest.mark.parametrize("src", [
    'import regex\npost(regex.findall("\\\\d+", "a1b22c333"))',
    'import regex\npost(regex.findall("(\\\\w)(\\\\d)", "a1 b2"))',
    'import regex\npost(regex.findall("(\\\\d+)", "a1b22"))',
    'import regex\npost(regex.findall("x", "aaa"))',
    'import regex\npost(regex.findall("a*", "bab"))',
    'import regex\npost(regex.sub("(\\\\w+)@(\\\\w+)", "\\\\2:\\\\1", "eu@casa e tu@la"))',
    'import regex\npost(regex.sub("\\\\d", "#", "a1b2", 1))',
    'import regex\npost(regex.sub("^", ">", "abc"))',
    'import regex\npost(regex.split(",", "a,b,,c"))',
    'import regex\npost(regex.split("\\\\s*,\\\\s*", "a , b,c"))',
    'import regex\npost(regex.split("x", "abc"))',
    'import regex\npost(regex.match("[a-z]+", "abc"))',
    'import regex\npost(regex.match("[a-z]+", "abC"))',
    'import regex\npost(regex.search("ção", "a ção b"))',
    'import regex\npost(regex.findall("\\\\w+", "olá mundo ção"))',
    'import regex\npost(regex.sub("[^\\\\d]", "", "a1b2c3"))',
    'import regex\npost(regex.findall("(a|ab)c", "abc"))',
    'import regex\npost(regex.sub("(a)(b)?", "[\\\\1|\\\\2]", "ab a"))',
    'import regex\npost(regex.escape("a.b*c"))',
    'from regex import match, findall, sub, split, escape\npost(findall("\\\\d", "a1"))',
    'import regex as rx\npost(rx.match("a+", "aaa"))',
    'post("a1b2".findall("\\\\d"))',
    'post("123".match("\\\\d+"))',
    'post("abc".match("\\\\d+"))',
    'post("hello world".sub("\\\\s+", "_"))',
    'post("a1b2".sub("\\\\d", "#"))',
    'post("ção".findall("\\\\w+"))',
    'post("a.b".findall("[.]"))',
    'import regex\npost(regex.findall("a{2,3}", "aaaa"))',
    'import regex\npost(regex.findall("a+?", "aaa"))',
    'import regex\npost(regex.match("^abc$", "abc"))',
    'import regex\npost(regex.findall("(?:ab)+", "ababab"))',
])
def test_regex(src):
    mesmo(src)


@pytest.mark.parametrize("src", [
    'import regex\npost(regex.match("(", "a"))',
    'import regex\npost(regex.match("[a", "a"))',
    'import regex\npost(regex.match("a{2,1}", "a"))',
    'import regex\npost(regex.match("*a", "a"))',
    'import regex\npost(regex.match("a\\\\1", "a"))',
    'import regex\npost(regex.match("a", 1))',
    'import regex\npost(regex.findall(1, "a"))',
    'import regex\npost(regex.sub("a"))',
    'post("a".match())',
    'post("a".sub("a"))',
])
def test_regex_erro(src):
    """Padrão inválido e construção não suportada param com erro — nunca
    casam errado em silêncio."""
    ambos_falham(src)


B = chr(92) + chr(92)   # "\\" no fonte .ps -> uma barra pro motor de regex

@pytest.mark.parametrize("src", [
    'import regex' + NL + 'post(regex.search("(?#coment)abc", "xabc"))',   # comentário inline
    'import regex' + NL + 'post(regex.match("(?>a)b", "ab"))',             # grupo atômico
    'import regex' + NL + 'post(regex.match("[a' + B + 'D]", "a"))',       # \\D negado dentro de []
])
def test_regex_construcao_nao_suportada(src):
    """O interpretador (que é o `re` do Python) aceita; o motor em C não.

    Para com erro explícito em vez de casar errado. (lookahead/lookbehind, `\\b`,
    retrovisor, grupo nomeado e flags JÁ foram implementados — ver
    test_regex_recursos_avancados.)
    """
    via_interpretador(src)                      # o Python resolve
    with pytest.raises(Exception):              # o C recusa, e diz por quê
        via_c(src)


@pytest.mark.parametrize("src", [
    # retrovisor
    'import regex' + NL + 'post(regex.search("(' + B + 'w+) ' + B + '1", "hello hello"))',
    'import regex' + NL + 'post(regex.search("(' + B + 'w+) ' + B + '1", "hello world"))',
    'import regex' + NL + 'post(regex.findall("(a)(b)' + B + '2' + B + '1", "abba x abba"))',
    'import regex' + NL + 'post(regex.sub("(' + B + 'w+)@(' + B + 'w+)", "' + B + '2.' + B + '1", "user@host"))',
    # grupo nomeado (tratado como numerado)
    'import regex' + NL + 'post(regex.findall("(?P<n>' + B + 'd+)", "a1b22c333"))',
    # flags inline global + escopo
    'import regex' + NL + 'post(regex.findall("(?i)ab", "AB ab Ab aB"))',
    'import regex' + NL + 'post(regex.search("(?s)a.b", "a' + B + 'nb"))',
    'import regex' + NL + 'post(regex.findall("(?m)^' + B + 'd+", "12' + B + 'n34' + B + 'n5"))',
    'import regex' + NL + 'post(regex.search("(?i:hello) world", "HELLO world"))',
    'import regex' + NL + 'post(regex.search("(?i:hello) world", "HELLO WORLD"))',
    # ancoras
    'import regex' + NL + 'post(regex.findall("' + B + 'bcat' + B + 'b", "cat category cat"))',
    'import regex' + NL + 'post(regex.search("' + B + 'Ahi", "hi there"))',
    'import regex' + NL + 'post(regex.search("end' + B + 'Z", "the end"))',
    # lookahead / lookbehind (inclusive unicode e combinado)
    'import regex' + NL + 'post(regex.findall("' + B + 'd+(?=px)", "10px 20em 30px"))',
    'import regex' + NL + 'post(regex.findall("' + B + 'd+(?!px)", "10px 20em 30px"))',
    'import regex' + NL + 'post(regex.findall("(?<=@)' + B + 'w+", "a@host b@srv"))',
    'import regex' + NL + 'post(regex.findall("(?<=' + B + 'w)' + B + 'd", "a1 b2 3c"))',
    'import regex' + NL + 'post(regex.search("(?<![a-z])cat", "bobcat"))',
    # IGNORECASE unicode (Latin-1) + literal nao-ASCII com quantificador
    'import regex' + NL + 'post(regex.search("(?i)caf' + chr(233) + '", "um CAF' + chr(201) + '"))',
    'import regex' + NL + 'post(regex.findall("' + chr(233) + '+", "caf' + chr(233) + chr(233) + " x" + chr(233) + '"))',
])
def test_regex_recursos_avancados(src):
    """Retrovisor, grupo nomeado, flags (global+escopo), âncoras (\\b/\\A/\\Z),
    lookahead/lookbehind e IGNORECASE unicode: o motor C bate com o `re` do
    Python (diferencial)."""
    assert via_c(src) == via_interpretador(src)


def test_regex_catastrofica_para_em_vez_de_travar():
    """`(a+)+b` sem casar é explosão exponencial: tem que abortar, não pendurar."""
    src = ('import regex' + NL
           + 'post(regex.match("(a+)+b", "' + "a" * 30 + '"))')
    with pytest.raises(Exception):
        via_c(src)


# ═════════════════════════ `is`, `in` e TypeName ═══════════════════════════
# `TypeName` virou um valor imediato (`V_TIPO`), sem alocação. `is` e `in`
# ganharam opcode próprio com a negação no argumento — uma instrução em vez
# de operação + NOT.
#
# `is` decide em runtime: contra TypeName compara TIPO, contra Entity compara
# a classe, contra o resto compara valor.

@pytest.mark.parametrize("src", [
    'x=5\npost(x is int)',
    'x="a"\npost(x is str)',
    'x=1.5\npost(x is flo)',
    'x=True\npost(x is bool)',
    'x=[1]\npost(x is list)',
    'x={"a":1}\npost(x is json)',
    'x={"a":1}\npost(x is dict)',
    'x=(1,)\npost(x is tup)',
    'x=5\npost(x is str)',
    'x=True\npost(x is int)',
    'x=Null\npost(x is Null)',
    'x=5\npost(x not is str)',
    'x=5\npost(x is not str)',
    'post(1.5 is int)',
    'post([1] is tup)',
    'post((1,) is list)',
    'post(str is type)',
    'post(5 is type)',
    'post(str == str)',
    'post(str == int)',
    'post(type(str))',
    'post(str)',
    'post([str, int])',
    'post(type(abs))',
    'x=5\nif x is int {\n post("eh int")\n}',
    'l=[1,"a"]\nfor each v in l {\n if v is str {\n  post(v)\n }\n}',
    'post(2 in [1,2])',
    'post(5 in [1,2])',
    'post("a" in "cab")',
    'post("z" not in "cab")',
    'post("a" in {"a":1})',
    'post("z" in {"a":1})',
    'post(1 in (1,2))',
    'post(1 in [])',
    'Entity P():\n    action __init__(self):\n        self.a=1\npost(type(P()))',
    'Entity P():\n    action __init__(self):\n        self.a=1\np=P()\npost(p.type())',
])
def test_is_in_typename(src):
    mesmo(src)


def test_type_de_instancia_usa_o_nome_da_entity():
    """Era `PoolEntityInstance` no interpretador — nome de classe Python
    vazando numa linguagem que não tem essa classe."""
    src = ('Entity Cliente():' + NL + '    action __init__(self):' + NL
           + '        self.n = 1' + NL + 'post(type(Cliente()))')
    mesmo(src)
    assert via_c(src) == ["Cliente"]


# ═════════════════════════ `count` e `count each` ══════════════════════════
# Três formas: `count T in x`, `count T(v) in x` e a infixa `T(v) count in x`.
# `count each` roda um bloco por ocorrência, com `self` e `_count` valendo o
# TOTAL desde a primeira volta (é pré-calculado), mais `_match` e `_index`.
#
# O container decide a iteração: lista pelos itens, dict pelos VALORES (com a
# chave no lugar do índice), string por caractere, e int vira texto — daí
# `count int in 555` ser 3.

@pytest.mark.parametrize("src", [
    'l=[1,"a",2,True]\npost(count int in l)',
    'l=[1,7,7,2]\npost(count int(7) in l)',
    'l=[1,7,7,2]\npost(int(7) count in l)',
    'l=[1,"a"]\npost(count str in l)',
    'l=[]\npost(count int in l)',
    'l=[1.5,1]\npost(count flo in l)',
    'l=[[1],[2]]\npost(count list in l)',
    'l=[{},{}]\npost(count json in l)',
    'l=[{},{}]\npost(count dict in l)',
    'l=[(1,),(2,)]\npost(count tup in l)',
    'l=[True,1]\npost(count bool in l)',
    'l="abc"\npost(count char in l)',
    'l="a b"\npost(count char in l)',
    'd={"a":1,"b":"x"}\npost(count int in d)',
    'post(count int in 5)',
    'post(count int in 555)',
    'post(count int(5) in 5155)',
    'post(count int in Null)',
    'post(count str in Null)',
    'post(count int in 5.5)',
    'action f() {\n return [1,2]\n}\npost(count int in f())',
    'l=[1,2,3]\nx = count each int in l\npost(x)',
    'l=[1,2,3]\ncount each int in l {\n post("achei")\n}',
    'l=[1,"a",2]\ncount each int in l {\n post(_match)\n}',
    'l=[1,"a",2]\ncount each int in l {\n post(_index)\n}',
    'l=[1,2]\ncount each int in l {\n post(self)\n}',
    'l=[1,2]\ncount each int in l {\n post(_count)\n}',
    'l=[]\ncount each int in l {\n post("nunca")\n}\npost("fim")',
    'l=[1,2]\ncount each int in l {\n break\n}\npost("ok")',
    'l=[1,2,3]\ncount each int in l {\n if _index == 1 {\n  continue\n }\n post(_match)\n}',
    'l=[1,2]\ncount each int in l {\n count each int in l {\n  post(_match)\n }\n}',
    'd={"a":1}\ncount each int in d {\n post(_index)\n}',
    's="ab"\ncount each str in s {\n post(_match)\n}',
    'action g() {\n l=[1,2]\n count each int in l {\n  post(self)\n }\n}\ng()',
])
def test_count(src):
    mesmo(src)


@pytest.mark.parametrize("src", [
    "post(count int in post)",
    "post(count int in True)",
    "post(count int in abs)",
    "l=[1]" + NL + "count each int in post {" + NL + " post(1)" + NL + "}" + NL + 'post("fim")',
])
def test_count_em_nao_iteravel_e_zero(src):
    """`count` é uma pergunta: a resposta pra algo sem itens é nenhum, não erro."""
    mesmo(src)


def test_count_each_avalia_o_container_uma_vez():
    """O total e a lista de ocorrências saem da MESMA avaliação — senão
    `count each int in f()` chamaria `f` duas vezes."""
    src = ("chamadas = 0" + NL
           + "action f() {" + NL + " chamadas = chamadas + 1" + NL + " return [1, 2]" + NL + "}" + NL
           + "count each int in f() {" + NL + " post(_match)" + NL + "}" + NL
           + "post(chamadas)")
    mesmo(src)
    assert via_c(src) == ["1", "2", "1"]


# ═════════════════════ decoradores e campos tipados ════════════════════════
# Os decoradores são resolvidos em COMPILAÇÃO: os três que a linguagem define
# mudam como a action é gerada, não o que ela devolve. Decorador desconhecido
# não registra a action nenhuma — por isso chamar depois dá "variável não
# definida", e não "rodou sem o decorador".
#
# `@dataentity` é só um marcador: quem gera o `__init__` são os campos
# tipados (`nome: str`), com ou sem o decorador.

@pytest.mark.parametrize("src", [
    '@static\naction f() {\n return 1\n}\npost(f())',
    '@NonNull\naction g(a) {\n return a\n}\npost(g(1))',
    '@NonNull\naction g(a, b=2) {\n return a+b\n}\npost(g(1))',
    'Entity U():\n    @static\n    action tri(n):\n        return n*3\npost(U.tri(4))',
    'Entity P():\n    nome: str\n    idade: int\np=P("k",1)\npost(p.nome)\npost(p.idade)',
    'Entity P():\n    nome: str\n    idade: int\np=P(nome="k",idade=1)\npost(p.nome)',
    '@dataentity\nEntity P():\n    nome: str\np=P(nome="k")\npost(p.nome)',
    'Entity P():\n    nome: str = "x"\np=P()\npost(p.nome)',
    'Entity P():\n    nome: str = "x"\np=P("y")\npost(p.nome)',
    'Entity P():\n    a: int\n    b: int = 2\np=P(1)\npost(p.a)\npost(p.b)',
    'Entity P():\n    a: int\n    b: int\np=P(1, b=2)\npost(p.b)',
    'Entity P():\n    a: int\n    b: int = 9\np=P(a=1)\npost(p.b)',
    'Entity P():\n    n: str\n    action oi(self):\n        return self.n\npost(P("z").oi())',
    'Entity P():\n    n: str\n    action __init__(self):\n        self.n = "proprio"\npost(P().n)',
    'Entity A():\n    x: int\nEntity B(A):\n    action __init__(self, x):\n        base(x)\npost(B(7).x)',
    'Entity P():\n    a: int\np=P(z=1)\npost(p.a)',
    'action f(a, b) {\n return a+b\n}\npost(f(1, b=2))',
    'Entity P():\n    action __init__(self, x, y):\n        self.x = x\n        self.y = y\np=P(1, y=2)\npost(p.y)',
])
def test_decoradores(src):
    mesmo(src)


@pytest.mark.parametrize("src", [
    '@NonNull\naction g(a) {\n return a\n}\npost(g(Null))',
    '@NonNull\naction g(a, b=Null) {\n return a\n}\npost(g(1))',
    '@NonNull\naction g(a, b) {\n return a\n}\npost(g(1, Null))',
    'action f(a) {\n return a\n}\npost(f(z=1))',
    'Entity P():\n    a: int\np=P(z=1)\npost(p.z)',
])
def test_decorador_erro(src):
    ambos_falham(src)


def test_decorador_desconhecido_nao_registra_a_action():
    """Não é "roda sem o decorador": a action não chega a existir."""
    ambos_falham("@qualquer" + NL + "action f() {" + NL + " return 1" + NL + "}" + NL + "post(f())")


def test_nonnull_checa_depois_do_default():
    """Um default que avalie pra Null também viola — por isso o CHECK_NONNULL
    é emitido DEPOIS do prólogo, não na entrada da action."""
    ambos_falham("@NonNull" + NL + "action g(a = Null) {" + NL + " return a" + NL + "}" + NL + "post(g())")


def test_init_gerado_nao_sobrescreve_o_escrito():
    """Campo tipado + `__init__` próprio: o escrito à mão ganha."""
    src = ('Entity P():' + NL + '    n: str' + NL
           + '    action __init__(self):' + NL + '        self.n = "proprio"' + NL
           + 'post(P().n)')
    mesmo(src)
    assert via_c(src) == ["proprio"]


# ═════════════════════════ módulo datasentity ══════════════════════════════
# Quarto módulo nativo. `dataentity` devolve a própria Entity — o trabalho
# real (o `__init__` a partir dos campos tipados) é do compilador.

@pytest.mark.parametrize("src", [
    'from datasentity import dataentity, asdict, astuple, aslist, asjson\n@dataentity\nEntity P():\n    nome: str\n    idade: int\np=P(nome="k",idade=1)\npost(asdict(p))',
    'from datasentity import dataentity, asdict, astuple, aslist, asjson\n@dataentity\nEntity P():\n    nome: str\n    idade: int\np=P(nome="k",idade=1)\npost(astuple(p))',
    'from datasentity import dataentity, asdict, astuple, aslist, asjson\n@dataentity\nEntity P():\n    nome: str\n    idade: int\np=P(nome="k",idade=1)\npost(aslist(p))',
    'from datasentity import dataentity, asdict, astuple, aslist, asjson\n@dataentity\nEntity P():\n    nome: str\n    idade: int\np=P(nome="k",idade=1)\npost(asjson(p))',
    'from datasentity import dataentity, asdict, astuple, aslist, asjson\n@dataentity\nEntity P():\n    nome: str\n    idade: int\np=P(nome="k",idade=1)\npost(p.nome)',
])
def test_datasentity(src):
    mesmo(src)


@pytest.mark.parametrize("src", [
    'from datasentity import dataentity, asdict, astuple, aslist, asjson\npost(asdict(1))',
    'from datasentity import dataentity, asdict, astuple, aslist, asjson\npost(astuple("x"))',
    'from datasentity import dataentity, asdict, astuple, aslist, asjson\npost(asdict())',
])
def test_datasentity_erro(src):
    ambos_falham(src)


def test_decorador_pontuado_geral_compila():
    """Decorador com caminho pontuado (`@app.route(...)`) agora é suportado —
    é o que o jinker usa. O compilador avalia a expressão, roda o bloco e
    chama `registrar.register(action)`. Aqui um registrar em PoolScript puro
    prova o mecanismo: `rota()` devolve o próprio objeto, `register()` guarda e
    chama a action."""
    src = ("Entity App():" + NL
           + "    action rota(self, p) { self.p = p" + NL + " return self }" + NL
           + "    action register(self, fn) { fn() }" + NL
           + "app = App()" + NL
           + "@app.rota(\"/x\")" + NL
           + "action h() { post(\"handler\") }" + NL
           + "post(\"fim\")")
    assert via_c(src) == ["handler", "fim"]

# ═══════════════════════════ model e desempacotamento ══════════════════════
# `model` não é chamável: é um esquema, e a validação acontece no `==` entre
# um dict e ele. Chave extra passa; campo ausente, tipo errado ou comprimento
# estourado reprovam.

@pytest.mark.parametrize("src", [
    'model U() {\n nome: str(length=3)\n idade: int\n}\npost({"nome":"ab","idade":1} == U)',
    'model U() {\n nome: str(length=3)\n idade: int\n}\npost({"nome":"abcd","idade":1} == U)',
    'model U() {\n nome: str(length=3)\n idade: int\n}\npost({"nome":"ab"} == U)',
    'model U() {\n nome: str(length=3)\n idade: int\n}\npost(U == {"nome":"ab","idade":1})',
    'model U() {\n nome: str(length=3)\n idade: int\n}\npost({"nome":"ab","idade":1} != U)',
    'model U() {\n nome: str(length=3)\n idade: int\n}\npost(1 == U)',
    'model U() {\n nome: str(length=3)\n idade: int\n}\npost(U)',
    'model U() {\n nome: str(length=3)\n idade: int\n}\npost(type(U))',
    'model U() {\n nome: str(length=3)\n idade: int\n}\npost({"nome":"ab","idade":1,"extra":9} == U)',
    'model M() {\n x: flo\n}\npost({"x":1.5} == M)',
    'model M() {\n x: flo\n}\npost({"x":1} == M)',
    'model M() {\n ativo: bool\n}\npost({"ativo":True} == M)',
    'model M() {\n ativo: bool\n}\npost({"ativo":1} == M)',
    'model M() {\n n: int(length=3)\n}\npost({"n":999} == M)\npost({"n":1000} == M)\npost({"n":-999} == M)',
    'model M() {\n n: str\n}\npost({"n":Null} == M)',
    'model M() {\n n: str(length=3)\n}\npost({"n":"ção"} == M)',
])
def test_model(src):
    mesmo(src)


# `a, b = ...` empurra os valores em ordem INVERSA e deixa os STOREs saírem na
# ordem dos alvos. String desempacota por caractere (respeitando UTF-8).

@pytest.mark.parametrize("src", [
    'a, b = 1, 2\npost(a)\npost(b)',
    'a, b = [1,2]\npost(a)\npost(b)',
    'a, (b, c) = 1, (2, 3)\npost(a)\npost(b)\npost(c)',
    'a, *r = [1,2,3]\npost(a)\npost(r)',
    '*r, b = [1,2,3]\npost(r)\npost(b)',
    'a, *m, z = [1,2,3,4]\npost(a)\npost(m)\npost(z)',
    'a, *r = [1]\npost(r)',
    'x=1\ny=2\nx, y = y, x\npost(x)\npost(y)',
    'a, b = "xy"\npost(a)\npost(b)',
    'a, *r = "abc"\npost(r)',
    'a, b = "çã"\npost(a)\npost(b)',
    'action f() {\n return 1, 2\n}\na, b = f()\npost(a)\npost(b)',
    'action f() {\n return (1,2,3)\n}\na,b,c = f()\npost(c)',
])
def test_desempacotamento(src):
    mesmo(src)


@pytest.mark.parametrize("src", [
    'a, b = 1, 2, 3',
    'a, b = [1]',
    'a, b = 5',
    'a, b, c = "ab"',
    'a, b = Null',
])
def test_desempacotamento_erro(src):
    ambos_falham(src)


def test_troca_sem_temporaria():
    """`x, y = y, x` avalia a direita ANTES de escrever — senão x sobrescreve y."""
    src = "x = 1" + NL + "y = 2" + NL + "x, y = y, x" + NL + "post(x)" + NL + "post(y)"
    mesmo(src)
    assert via_c(src) == ["2", "1"]


# ═════════════════════════ geradores (`yield`) ═════════════════════════════
# Um gerador é um frame CONGELADO: guarda cópia própria dos locais e da pilha
# em vez de apontar pros pools da VM, porque entre dois `next` o programa roda
# qualquer outra coisa e os pools já teriam sido reusados. Retomar é copiar de
# volta, rodar até o `yield`, e copiar de novo pra cá.

@pytest.mark.parametrize("src", [
    'action g() {\n yield 1\n yield 2\n}\nfor each x in g() {\n post(x)\n}',
    'action g() {\n yield 1\n yield 2\n}\npost(list(g()))',
    'action g() {\n for each i in [1,2,3] {\n  yield i*2\n }\n}\npost(list(g()))',
    'action g() {\n x = 0\n while x < 3 {\n  yield x\n  x++\n }\n}\npost(list(g()))',
    'action g() {\n yield 1\n}\npost(g())',
    'action g() {\n yield 1\n}\npost(type(g()))',
    'action g(n) {\n i=0\n while i<n {\n  yield i\n  i++\n }\n}\npost(list(g(4)))',
    'action g() {\n a = 10\n yield a\n a = a + 5\n yield a\n}\npost(list(g()))',
    'action g() {\n for each i in [1,2] {\n  for each j in [10,20] {\n   yield i*j\n  }\n }\n}\npost(list(g()))',
    'action g() {\n yield 1\n yield 2\n}\na = g()\nb = g()\npost(list(a))\npost(list(b))',
    'action g() {\n yield [1,2]\n yield {"a":1}\n}\npost(list(g()))',
    'action f(x) {\n return x*2\n}\naction g() {\n yield f(3)\n}\npost(list(g()))',
    'action g() {\n i=0\n while i<3 {\n  yield i\n  i++\n }\n}\nt=0\nfor each v in g() {\n t = t + v\n}\npost(t)',
    'action g() {\n yield 1\n yield 2\n yield 3\n}\nfor each v in g() {\n if v == 2 {\n  break\n }\n post(v)\n}',
    'action g() {\n yield 1\n}\ntry {\n post(list(g()))\n} catch (e) {\n post("x")\n}',
    'Entity P():\n    a: int\naction g() {\n yield P(1)\n yield P(2)\n}\nfor each p in g() {\n post(p.a)\n}',
])
def test_geradores(src):
    mesmo(src)


@pytest.mark.parametrize("src", [
    'yield 1',
    'action g() {\n yield 1/0\n}\npost(list(g()))',
    'action g() {\n yield 1\n}\nx=g()\npost(x + 1)',
])
def test_gerador_erro(src):
    ambos_falham(src)


def test_action_sem_yield_nao_e_gerador():
    """Sem `yield` no corpo a action é comum: devolve Null, e `list(Null)`
    é erro nos dois — não um gerador vazio."""
    ambos_falham("action g() {" + NL + "}" + NL + "post(list(g()))")


def test_yield_dentro_de_try():
    """O `catch` tem que disparar mesmo quando o erro acontece DEPOIS de
    retomar o gerador.

    A pilha de `try` vive no frame do laço de execução, e retomar começa um
    laço novo — então ela é congelada junto com o gerador, com as posições
    guardadas RELATIVAS à base do frame. Absolutas apontariam pro lugar
    errado, porque a retomada quase nunca cai na mesma posição dos pools.
    """
    src = ('action g() {' + NL + ' try {' + NL + '  yield 1' + NL
           + '  post(1/0)' + NL + ' } catch (e) {' + NL + '  post("pegou")' + NL
           + ' }' + NL + ' yield 9' + NL + '}' + NL + 'post(list(g()))')
    assert via_c(src) == ["pegou", "[1, 9]"]


def test_yield_em_try_aninhado():
    src = ('action g() {' + NL + ' try {' + NL + '  try {' + NL
           + '   yield 1' + NL + '   post(1/0)' + NL + '  } catch (e) {' + NL
           + '   post("interno")' + NL + '  }' + NL + ' } catch (e) {' + NL
           + '  post("externo")' + NL + ' }' + NL + '}' + NL + 'post(list(g()))')
    assert via_c(src) == ["interno", "[1]"]


def test_geradores_sao_independentes():
    """Duas chamadas da mesma action dão dois frames congelados separados."""
    src = ('action g() {' + NL + ' i = 0' + NL + ' while i < 3 {' + NL
           + '  yield i' + NL + '  i++' + NL + ' }' + NL + '}' + NL
           + 'a = g()' + NL + 'b = g()' + NL
           + 'for each x in a {' + NL + ' post(x)' + NL + ' break' + NL + '}' + NL
           + 'post(list(b))')
    mesmo(src)
    assert via_c(src) == ["0", "[0, 1, 2]"]


# ═════════════════════════ módulo hash ═════════════════════════════════════
# SHA-256, HMAC, PBKDF2 e base64 escritos em C — sem OpenSSL. O formato tem
# que ser INTERCAMBIÁVEL com o `hash_lib.py`: um hash gerado no interpretador
# precisa validar no binário, senão trocar de runtime derruba login.

@pytest.mark.parametrize("src", [
    "import hash" + NL + 'h = hash.crypt("abc")' + NL + 'post(hash.check(h, "abc"))',
    "import hash" + NL + 'h = hash.crypt("abc")' + NL + 'post(hash.check(h, "abd"))',
    "import hash" + NL + 'post(hash.check("lixo", "abc"))',
    "import hash" + NL + 'post(hash.check("", "abc"))',
    "import hash" + NL + 'post(hash.check(123, "abc"))',
    "from hash import crypt, check" + NL + 'post(check(crypt("z"), "z"))',
])
def test_hash(src):
    mesmo(src)


def test_salt_muda_o_hash():
    """Dois `crypt` da mesma senha não podem dar o mesmo texto — se derem, o
    salt não está sendo sorteado e o hash vira tabela de consulta."""
    src = ("import hash" + NL + 'a = hash.crypt("x")' + NL
           + 'b = hash.crypt("x")' + NL + "post(a == b)" + NL
           + 'post(hash.check(a, "x"))' + NL + 'post(hash.check(b, "x"))')
    assert via_c(src) == ["False", "True", "True"]


def test_hash_do_python_valida_no_c():
    """O teste que importa: o formato é o mesmo dos dois lados."""
    from poolscript.stdlib.hash_lib import crypt as crypt_py
    h = crypt_py("minhasenha")
    src = ("import hash" + NL + 'post(hash.check("' + h + '", "minhasenha"))' + NL
           + 'post(hash.check("' + h + '", "outra"))')
    assert via_c(src) == ["True", "False"]


def test_hash_do_c_valida_no_python():
    from poolscript.stdlib.hash_lib import check as check_py
    h = via_c("import hash" + NL + 'post(hash.crypt("minhasenha"))')[0]
    assert check_py(h, "minhasenha") is True
    assert check_py(h, "outra") is False


@pytest.mark.parametrize("src,esperado", [
    ('import hash' + NL + 'post(hash.sha256(""))',
     ["e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"]),
    ('import hash' + NL + 'post(hash.sha256("abc"))',
     ["ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"]),
    ('import hash' + NL + 'post(hash.b64encode("PoolScript"))', ["UG9vbFNjcmlwdA=="]),
    ('import hash' + NL + 'post(hash.b64decode("UG9vbFNjcmlwdA=="))', ["PoolScript"]),
    ('import hash' + NL + 'post(hash.b64encode("ção"))', ["w6fDo28="]),
])
def test_hash_vetores_conhecidos(src, esperado):
    """Vetores da RFC — se estes mudarem, a implementação saiu do padrão."""
    assert via_c(src) == esperado


def test_b64_invalido():
    ambos_falham("import hash" + NL + 'post(hash.b64decode("!!!"))')


# ═════════════════════════ módulo jwt ══════════════════════════════════════
# HS256. A assinatura cobre os BYTES de `header.payload`, então o JSON precisa
# ser compacto e o base64 precisa ser urlsafe sem padding — um espaço a mais
# ou um `+` no lugar do `-` gera token que não valida em lugar nenhum. Por isso
# os testes comparam o token INTEIRO com o do Python, não só "funciona".

def test_token_e_identico_ao_do_python():
    from poolscript.stdlib.jwt_lib import gen as gen_py
    src = "import jwt" + NL + 'post(jwt.gen({"a":1}, "k"))'
    assert via_c(src) == [gen_py({"a": 1}, "k")]


def test_token_do_python_valida_no_c():
    from poolscript.stdlib.jwt_lib import gen as gen_py
    t = gen_py({"user_id": 7}, "segredo")
    src = ("import jwt" + NL + 'post(jwt.check("' + t + '", "segredo"))' + NL
           + 'post(jwt.check("' + t + '", "outra"))')
    assert via_c(src) == ["{'user_id': 7}", "null"]


def test_token_do_c_valida_no_python():
    from poolscript.stdlib.jwt_lib import check as check_py
    t = via_c("import jwt" + NL + 'post(jwt.gen({"id": 9}, "s"))')[0]
    assert check_py(t, "s") == {"id": 9}
    assert check_py(t, "errada") is None


@pytest.mark.parametrize("src", [
    "import jwt" + NL + 'post(jwt.check("abc", "k"))',
    "import jwt" + NL + 'post(jwt.check("a.b", "k"))',
    "import jwt" + NL + 'post(jwt.check("a.b.c.d", "k"))',
    "import jwt" + NL + 'post(jwt.check("a.b.c", "k"))',
    "import jwt" + NL + "post(jwt.check(1, \"k\"))",
])
def test_token_ruim_devolve_null(src):
    """`check` é uma pergunta: quem chama trata a ausência, não exceção."""
    assert via_c(src) == ["null"]


def test_expiracao():
    src = ("import jwt" + NL + "import date" + NL
           + 'v = jwt.gen({"exp": date.timestamp() + 3600}, "k")' + NL
           + 'x = jwt.gen({"exp": date.timestamp() - 10}, "k")' + NL
           + 'post(jwt.check(v, "k") is Null)' + NL
           + 'post(jwt.check(x, "k") is Null)')
    assert via_c(src) == ["False", "True"]


@pytest.mark.parametrize("alg", ["HS256", "HS384", "HS512"])
def test_familia_hmac_completa(alg):
    """Os três produzem token byte a byte igual ao do Python."""
    from poolscript.stdlib.jwt_lib import gen as gen_py
    src = "import jwt" + NL + 'post(jwt.gen({"a":1}, "k", "' + alg + '"))'
    assert via_c(src) == [gen_py({"a": 1}, "k", alg)]


@pytest.mark.parametrize("alg", ["HS384", "HS512"])
def test_token_de_alg_maior_cruza_os_motores(alg):
    from poolscript.stdlib.jwt_lib import gen as gen_py, check as check_py
    t = gen_py({"u": 9}, "s", alg)
    assert via_c("import jwt" + NL + 'post(jwt.check("' + t + '", "s"))') == ["{'u': 9}"]
    assert check_py(via_c("import jwt" + NL
                          + 'post(jwt.gen({"u": 9}, "s", "' + alg + '"))')[0], "s") == {"u": 9}


@pytest.mark.parametrize("alg", ["RS256", "ES256", "PS256", "none", "", "hs256"])
def test_algoritmo_fora_da_familia_e_recusado(alg):
    """Assinar com HMAC dizendo outra coisa no header é a confusão de
    algoritmo que já rendeu CVE. Recusa por nome, não ignora."""
    ambos_falham("import jwt" + NL + 'post(jwt.gen({"a":1}, "k", "' + alg + '"))')


def test_token_com_alg_none_e_recusado():
    """O ataque clássico: header forjado dizendo `none`, sem assinatura."""
    import base64
    import json as _j
    def b64(b): return base64.urlsafe_b64encode(b).rstrip(b"=").decode()
    h = b64(_j.dumps({"alg": "none", "typ": "JWT"}, separators=(",", ":")).encode())
    p = b64(_j.dumps({"admin": True}, separators=(",", ":")).encode())
    forjado = h + "." + p + "."
    assert via_c("import jwt" + NL + 'post(jwt.check("' + forjado + '", "k"))') == ["null"]


def test_trocar_o_alg_do_header_invalida():
    """Assinatura de HS512 com header remarcado como HS256 não pode passar."""
    import base64
    import json as _j
    from poolscript.stdlib.jwt_lib import gen as gen_py
    t = gen_py({"a": 1}, "k", "HS512")
    _, pay, sig = t.split(".")
    h256 = base64.urlsafe_b64encode(
        _j.dumps({"alg": "HS256", "typ": "JWT"}, separators=(",", ":")).encode()
    ).rstrip(b"=").decode()
    misto = h256 + "." + pay + "." + sig
    assert via_c("import jwt" + NL + 'post(jwt.check("' + misto + '", "k"))') == ["null"]


@pytest.mark.parametrize("src,esperado", [
    ('import json' + NL + 'post(json.stringify({"a":1,"b":[2,3]}, True))', ['{"a":1,"b":[2,3]}']),
    ('import json' + NL + 'post(json.stringify({"a":1,"b":[2,3]}))', ['{"a": 1, "b": [2, 3]}']),
])
def test_json_compacto(src, esperado):
    """O modo compacto existe por causa do jwt — a assinatura é sobre bytes."""
    assert via_c(src) == esperado


# ═════════════════════════ módulos sys e dotenv ════════════════════════════
# `sys.argv` e `sys.stdout` são MEMBROS-VALOR: resolvidos no acesso, não na
# chamada. `sys.stdout` é um namespace aninhado, por isso vira um módulo.

def test_sys_argv_do_binario(tmp_path):
    """Argumentos do usuário são o que vem DEPOIS do arquivo."""
    import subprocess
    f = tmp_path / "a.ps"
    f.write_text("import sys" + NL + "post(len(sys.argv))" + NL
                 + "post(sys.argv[0])" + NL + "post(sys.argv[1])" + NL, encoding="utf-8")
    POOL = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "pool")
    if not os.path.isfile(POOL):
        pytest.skip("binário não compilado")
    r = subprocess.run([POOL, str(f), "um", "dois"], capture_output=True, text=True)
    assert r.stdout.splitlines() == ["2", "um", "dois"]


def test_sys_argv_fora_de_faixa_avisa_como_qualquer_lista():
    """Era o único índice da linguagem que não avisava — `sys.argv` era um
    objeto próprio que engolia o IndexError."""
    mesmo("import sys" + NL + "post(sys.argv[99])")


@pytest.mark.parametrize("src", [
    "import sys" + NL + "post(sys.platform())",
    "import sys" + NL + 'post(sys.RelativePath("nao_existe_xyz_123"))',
    "from sys import platform" + NL + "post(platform())",
])
def test_sys(src):
    mesmo(src)


def test_sys_stdout_e_stderr():
    """`write` sem o 2º argumento não quebra linha; `writeln` sempre quebra.

    Sem `mesmo()`: `sys.stdout.write` escreve direto no descritor, e o
    `via_interpretador` só coleta o que sai por `post`. Comparar daria vazio
    dos dois lados e não provaria nada — então o interpretador é checado
    capturando o stdout de verdade.
    """
    src = ("import sys" + NL + 'sys.stdout.write("a")' + NL
           + 'sys.stdout.write("b", True)' + NL + 'sys.stdout.writeln("c")')
    assert via_c(src) == ["ab", "c"]

    from poolscript.interpreter import Interpreter
    from poolscript.parser import parse_source
    cap = io.StringIO()
    with redirect_stdout(cap):
        Interpreter(source=src, filename="<t>").run(parse_source(src, "<t>"))
    assert cap.getvalue().splitlines() == ["ab", "c"]


def test_sys_argv_vazio_quando_nao_ha_argumento():
    """Sem argumento do usuário a lista é vazia — não Null, não erro."""
    assert via_c("import sys" + NL + "post(len(sys.argv))") == ["0"]


def test_dotenv_sobe_diretorios(tmp_path):
    import subprocess
    (tmp_path / ".env").write_text(
        'CHAVE=valor' + NL + '# comentario' + NL + 'ASPAS="com aspas"' + NL
        + 'VAZIO=' + NL + 'SEMIGUAL' + NL, encoding="utf-8")
    sub = tmp_path / "a" / "b"
    sub.mkdir(parents=True)
    f = sub / "d.ps"
    f.write_text("import dotenv" + NL + "post(dotenv.load())" + NL, encoding="utf-8")
    POOL = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "pool")
    if not os.path.isfile(POOL):
        pytest.skip("binário não compilado")
    r = subprocess.run([POOL, str(f)], capture_output=True, text=True, cwd=str(sub))
    # linha sem `=` é ignorada; aspas são delimitador, não conteúdo
    assert r.stdout.strip() == "{'CHAVE': 'valor', 'ASPAS': 'com aspas', 'VAZIO': ''}"


def test_dotenv_sem_arquivo_devolve_vazio(tmp_path):
    import subprocess
    f = tmp_path / "d.ps"
    f.write_text("import dotenv" + NL + "post(dotenv.load())" + NL, encoding="utf-8")
    POOL = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "pool")
    if not os.path.isfile(POOL):
        pytest.skip("binário não compilado")
    # `/` não tem .env, então subir a partir de um tmp isolado não acha nada
    r = subprocess.run([POOL, str(f)], capture_output=True, text=True, cwd="/tmp")
    assert r.returncode == 0 and r.stdout.strip() in ("{}", "{'': ''}")


# ═════════════════════════ Parsing ═════════════════════════════════════════
# Builtin GLOBAL, não módulo: `Parsing.integer(x)` funciona sem import, e
# `import Parsing` é erro — nos dois motores.
#
# O `parsing_lib.py` devolve `TransientValue`, um embrulho que guarda o tipo
# de origem. Verificado caso a caso: esse tipo SEMPRE coincide com o tipo real
# do valor, inclusive depois de aritmética. O embrulho só aparecia em
# `type(x)`, devolvendo o nome da classe Python — mesmo vazamento de
# `PoolEntityInstance`. Os dois motores agora devolvem o tipo de origem.

@pytest.mark.parametrize("src", [
    'post(Parsing.integer("R$ 1.299"))',
    'post(Parsing.integer(123.7))',
    'post(Parsing.integer(5))',
    'post(Parsing.integer(""))',
    'post(Parsing.floating("1.299,90"))',
    'post(Parsing.floating("12.5"))',
    'post(Parsing.floating(3))',
    'post(Parsing.floating(""))',
    'post(Parsing.floating("1,5"))',
    'post(Parsing.string("  a   b  "))',
    'post(Parsing.string("abc123", "int"))',
    'post(Parsing.string("a9", "flo"))',
    'post(Parsing.boolean("false"))',
    'post(Parsing.boolean("FALSE"))',
    'post(Parsing.boolean("x"))',
    'post(Parsing.boolean("0"))',
    'post(Parsing.boolean(""))',
    'post(Parsing.boolean("null"))',
    'post(Parsing.boolean(1))',
    'post(Parsing.boolean([]))',
    'post(Parsing.boolean([1]))',
    'post(Parsing.JSONformatt("[1,2]"))',
    'post(Parsing.JSONformatt("lixo"))',
    'post(Parsing.JSONformatt({"a":1}))',
    'post(Parsing.JSONformatt(5))',
    'post(Parsing.Arrayformatt("ab"))',
    'post(Parsing.Arrayformatt({"a":1}))',
    'post(Parsing.Arrayformatt([1]))',
    'post(Parsing.Arrayformatt(5))',
    'post(Parsing.Arrayformatt((1,2)))',
    'post(Parsing.Tuplasformatt([1,2]))',
    'post(Parsing.Tuplasformatt("ab"))',
    'post(Parsing.Tuplasformatt(5))',
    'post(Parsing.TransientValue("7","int"))',
    'post(Parsing.TransientValue(7,"str"))',
    'post(Parsing.TransientValue("7","flo"))',
    'x = Parsing.integer("12")\npost(x + 1)\npost(x.type())',
    'x = Parsing.floating("1,5")\npost(x * 2)\npost(x.type())',
    'post(type(Parsing.integer("1")))',
])
def test_parsing(src):
    mesmo(src)


@pytest.mark.parametrize("src", [
    "import Parsing" + NL + "post(1)",
    "from Parsing import integer" + NL + "post(1)",
])
def test_parsing_nao_e_importavel(src):
    """É builtin global. A VM chegou a aceitar `import Parsing` — mais
    permissiva que a linguagem, que é sempre defeito."""
    ambos_falham(src)


def test_integer_trunca_float_nao_concatena_digitos():
    """`123.7` é 123, não 1237 — a limpeza de dígitos não pode ser aplicada a
    um número que já é número."""
    assert via_c("post(Parsing.integer(123.7))") == ["123"]


def test_floating_entende_separador_brasileiro():
    """`1.299,90`: o ponto é milhar quando há vírgula; só vírgula é decimal."""
    assert via_c('post(Parsing.floating("1.299,90"))') == ["1299.9"]
    assert via_c('post(Parsing.floating("1,5"))') == ["1.5"]
    assert via_c('post(Parsing.floating("12.5"))') == ["12.5"]


def test_boolean_tem_regra_propria_para_texto():
    """`"false"` e `"0"` são falsos aqui, embora string não-vazia seja
    verdadeira em todo o resto da linguagem."""
    assert via_c('post(Parsing.boolean("false"))') == ["False"]
    assert via_c('post(Parsing.boolean("x"))') == ["True"]
    assert via_c('post(bool("false"))') == ["True"]


# ══════════════════ format, isidentifier, maketrans/translate ══════════════
# Subconjunto da mini-linguagem do Python que cobre o uso real: `{}`, `{0}`,
# `{nome}` (via format_map) e specs `[preenche][<^>][0][largura][.prec][tipo]`.
# Spec desconhecido é ERRO, nunca ignorado — formatar diferente do pedido é
# pior que não formatar.

@pytest.mark.parametrize("src", [
    'post("ola {}".format("mundo"))',
    'post("{} e {}".format(1,2))',
    'post("{0} {0}".format("a"))',
    'post("{1} {0}".format("a","b"))',
    'post("{{}}".format())',
    'post("{:>5}|".format("a"))',
    'post("{:<5}|".format("a"))',
    'post("{:^5}|".format("a"))',
    'post("{:5}|".format("a"))',
    'post("{:>5}|".format(7))',
    'post("{:05d}".format(42))',
    'post("{:.2f}".format(3.14159))',
    'post("{:.3f}".format(1))',
    'post("{:x}".format(255))',
    'post("{:X}".format(255))',
    'post("{:o}".format(8))',
    'post("{:b}".format(5))',
    'post("{}".format([1,2]))',
    'post("{}".format(Null))',
    'post("{}".format([Null]))',
    'post("{:*^7}|".format("ab"))',
    'post("abc".isidentifier())',
    'post("1abc".isidentifier())',
    'post("a_b".isidentifier())',
    'post("ção".isidentifier())',
    'post("".isidentifier())',
    'post("a b".isidentifier())',
    'post("_".isidentifier())',
    'post("{x}".format_map({"x":1}))',
    'post("abc".format_map({"x":1}))',
    'post("{a}-{b}".format_map({"a":1,"b":2}))',
    'post("{x}".format_map({"x":Null}))',
    't = "".maketrans("ab","xy")\npost("abc".translate(t))',
    't = {97: 120}\npost("abc".translate(t))',
    'post("abc".maketrans("ab","xy"))',
])
def test_format(src):
    mesmo(src)


def test_format_com_spec_invalido_para():
    ambos_falham('post("{:!!}".format(1))')


def test_format_sem_valor_para():
    ambos_falham('post("{} {}".format(1))')


def test_null_nao_vaza_como_none_no_format():
    """`str.format` do Python chama `str(None)`. O mesmo vazamento que já
    saiu do `post` e do `str()` — e a renderização agora é UMA função
    compartilhada, porque duas cópias divergiram exatamente aqui."""
    assert via_c('post("{}".format([Null]))') == ["[null]"]
    mesmo('post("{}".format([Null]))')


def test_maketrans_devolve_dict_de_codepoint():
    assert via_c('post("".maketrans("ab","xy"))') == ["{97: 120, 98: 121}"]


def test_maketrans_exige_mesmo_tamanho():
    ambos_falham('post("".maketrans("ab","x"))')


# ═════════════════════════ tipo bytes ══════════════════════════════════════
# Reusa o layout da string (mesma alocação, mesmo GC), mas é OUTRO tipo — daí
# `"ab" == "ab".encode()` ser falso. `b[i]` devolve o byte como int, não uma
# fatia de um, e índice fora de faixa é ERRO: a regra de "avisa e devolve
# Null" vale pra lista e string, não pra bytes.

@pytest.mark.parametrize("src", [
    'post("abc".encode())',
    'post("ção".encode())',
    'post("".encode())',
    'post("a".encode("utf-8"))',
    'post("a\\\\nb".encode())',
    'b = "ab".encode()\npost(len(b))',
    'b = "ab".encode()\npost(b[0])',
    'b = "ab".encode()\npost(b[-1])',
    'b = "ab".encode()\npost(type(b))',
    'b = "ab".encode()\npost(b.type())',
    'b = "ab".encode()\npost(b.decode())',
    'b = "ab".encode()\npost(b.hex())',
    'b = "ab".encode()\npost(str(b))',
    'b = "ab".encode()\npost(b == "ab".encode())',
    'b = "ab".encode()\npost(b == "ab")',
    'post("abc".encode() + "d".encode())',
    'post(["a".encode()])',
    'post({"k": "a".encode()})',
])
def test_bytes(src):
    mesmo(src)


@pytest.mark.parametrize("src", [
    'b = "ab".encode()\npost(b[9])',
    'b = "ab".encode()\npost(b[-9])',
    'post("a".encode() + "b")',
    'b = "ab".encode()\nfor each x in b {\n post(x)\n}',
])
def test_bytes_erro(src):
    ambos_falham(src)


def test_bytes_nao_tem_metodo_len():
    """`len(b)` funciona; `b.len()` não — o `.len()` é da string, e a VM
    chegou a oferecer nos dois, mais permissiva que a linguagem."""
    ambos_falham('post("ab".encode().len())')
    assert via_c('post(len("ab".encode()))') == ["2"]


def test_bytes_nao_e_igual_a_string_de_mesmo_conteudo():
    assert via_c('post("ab".encode() == "ab")') == ["False"]
    assert via_c('post("ab".encode() == "ab".encode())') == ["True"]


def test_repr_escapa_como_o_python():
    """Imprimível passa, o resto vira `\\xNN` — é o repr do `bytes`."""
    assert via_c('post("ção".encode())') == ["b'" + chr(92) + "xc3" + chr(92) + "xa7"
                                             + chr(92) + "xc3" + chr(92) + "xa3o'"]


# ═════════════════════════ lógicos: and / or / not ══════════════════════════
# Curto-circuito de verdade (o lado direito não roda quando não precisa) e
# resultado SEMPRE bool: `0 or 5` é True na PoolScript, não 5.

@pytest.mark.parametrize("src", [
    "post(0 and 5)", "post(3 and 5)", "post(0 or 5)", "post(3 or 5)",
    'post("" or "x")', "post([] and 1)", "post(not 3)", "post(not 0)",
    'post(not "")', "post(1 && 2)", "post(0 || 7)", "post(!0)", "post(!1)",
    "post(3 and 5 or 7)", "post(not (1 and 0))",
    'a = "x"' + NL + 'post(a is str and not (a is int))',
])
def test_logicos(src):
    mesmo(src)


def test_and_curto_circuita():
    """`false and f()` não pode executar `f` — efeito colateral apareceria."""
    src = ('action f() {' + NL + ' post("efeito")' + NL + ' return true' + NL + '}' + NL
           + 'post(false and f())' + NL + 'post(true or f())')
    assert via_c(src) == via_interpretador(src) == ["False", "True"]


# ═════════════════════════ resto de divisão com float ═══════════════════════

@pytest.mark.parametrize("src", [
    "post(5.0 % 3)", "post(5 % 3.0)", "post(-1.0 % 3)", "post(7.5 % 2)",
    "post(-7 % 2.5)",
])
def test_mod_float(src):
    """O sinal é o do DIVISOR, como no interpretador — `fmod` puro daria
    -1.0 pra `-1.0 % 3`."""
    mesmo(src)


def test_mod_float_por_zero(  ):
    ambos_falham("post(1.5 % 0)")


# ═════════════════════════ count de subcadeia ═══════════════════════════════

@pytest.mark.parametrize("src", [
    'post(count str("ana") in "ana banana")',
    'post(count str("aa") in "aaaa")',           # sobreposição conta
    'post(count str("ção") in "ação canção")',   # por codepoint, não byte
    'post(count str("z") in "banana")',
    'post(count str("") in "ab")',
    'post(count str in "banana")',               # sem valor: cada caractere
])
def test_count_subcadeia(src):
    mesmo(src)


def test_count_each_return_seco_devolve_o_total():
    """`return;` dentro de `count each` devolve o TOTAL contado à action —
    é a forma curta de "conte e me dê o número". `return valor` interrompe
    com o valor, como qualquer return."""
    src = ('list xs = [7, 7, 7, 8, 7]' + NL + 'action total() {' + NL
           + '    count each int(7) in xs {' + NL + '        return;' + NL
           + '    }' + NL + '}' + NL + 'post(total())')
    assert via_c(src) == via_interpretador(src) == ["4"]


# ═════════════════════════ comparação com Null ══════════════════════════════

@pytest.mark.parametrize("src", [
    "post(Null > 0)", "post(Null < 0)", "post(Null >= 0)", "post(Null <= 0)",
    "post(1 > Null)", "post(-1 < Null)", "post(Null >= Null)",
    'post(Null > "a")', "post(Null == 0)", "post(Null == Null)",
])
def test_null_nao_se_ordena(src):
    """Qualquer `<`/`>`/`<=`/`>=` com Null é False — `if x > 0` com `x` vazio
    simplesmente não entra, em vez de estourar."""
    mesmo(src)


# ═════════════════════════ contains em lista ════════════════════════════════

@pytest.mark.parametrize("src", [
    "post([1,2].contains(2))", "post([1,2].has(9))", "post((1,2).contains(2))",
    'post(["a"].contains("a"))',
])
def test_list_contains(src):
    mesmo(src)


# ═════════════════════════ tipo declarado ═══════════════════════════════════

@pytest.mark.parametrize("src", [
    "flo x = 5" + NL + "post(x, type(x))",       # int sobe pra flo
    'int x = "7"' + NL + "post(x, type(x))",     # str numérica converte
    'flo x = "1.5"' + NL + "post(x)",
    "int x = 3" + NL + "post(x)",
    'str x = "a"' + NL + "post(x)",
    "bool x = true" + NL + "post(x)",
    "list x = (1, 2)" + NL + "post(x, type(x))", # só escalar é checado
])
def test_tipo_declarado_converte(src):
    mesmo(src)


@pytest.mark.parametrize("src", [
    "int x = 5.9",       # não trunca em silêncio
    "str x = 5",
    "bool x = 5",
    'int x = "a"',
    'flo x = "abc"',
])
def test_tipo_declarado_recusa(src):
    ambos_falham(src)


def test_tipo_declarado_captura_por_nome():
    src = ('try {' + NL + ' int x = 5.9' + NL
           + '} catch (AtributtedValueError e) {' + NL + ' post("pego")' + NL + '}')
    assert via_c(src) == ["pego"]


# ═════════════════════════ int action / bool action ═════════════════════════
# As duas declarações são um contrato: NUNCA propagam erro. `int` devolve 500
# no erro e 0 no lugar de Null; `bool` devolve False no erro, True no lugar de
# Null, e passa o resto por bool().

@pytest.mark.parametrize("src", [
    "int action f() { return 1/0 }" + NL + "post(f())",
    "bool action f() { return 1/0 }" + NL + "post(f())",
    "int action f() { }" + NL + "post(f())",
    "bool action f() { }" + NL + "post(f())",
    "bool action f() { return 5 }" + NL + "post(f())",
    "int action f() { return 7 }" + NL + "post(f())",
    "bool reaction f() { return 1/0 }" + NL + "post(f())",
])
def test_tipo_de_retorno(src):
    mesmo(src)


def test_str_action_nao_tem_rede(  ):
    """Só `int` e `bool` engolem erro — `str action` deixa passar."""
    ambos_falham("str action f() { return 1/0 }" + NL + "post(f())")


# ═════════════════════════ catch por tipo repropaga ═════════════════════════

@pytest.mark.parametrize("src", [
    # nenhum catch casa: o erro sobe pro try de fora
    'try {' + NL + ' try { x = 1/0 } catch (KeyError e) { post("k") }' + NL
    + '} catch (e) { post("fora") }',
    # e o TIPO sobrevive à subida
    'try {' + NL + ' try { x = 1/0 } catch (KeyError e) { post("k") }' + NL
    + '} catch (SomeValueUnexpected e) { post("tipo certo") }',
    # casando, nada sobe
    'try { x = 1/0 } catch (SomeValueUnexpected e) { post("s") } catch (e) { post("g") }',
    'try { raise "algo" } catch (SomeValueUnexpected e) { post("s") } catch (e) { post("g") }',
])
def test_catch_nao_casado_repropaga(src):
    mesmo(src)


def test_catch_nao_casado_no_topo_aborta():
    ambos_falham('try { x = 1/0 } catch (KeyError e) { post("k") }')


def test_return_dentro_de_try_desarma_o_handler():
    """`return` de dentro de um `try` abandona o handler. Sem isso, o próximo
    erro do programa — em qualquer lugar — caía no catch de uma action que já
    tinha retornado."""
    src = ('action f() {' + NL + ' try {' + NL + '  return 1' + NL
           + ' } catch (e) { post("catch de f") }' + NL + '}' + NL
           + 'post(f())' + NL + 'post(1/0)')
    ambos_falham(src)
    # e a parte boa ainda sai igual
    src2 = ('action f() {' + NL + ' try {' + NL + '  return 1' + NL
            + ' } catch (e) { post("c") }' + NL + '}' + NL + 'post(f())')
    mesmo(src2)


# ═════════════════════════ nomes de erro ════════════════════════════════════
# O `catch (Tipo e)` só funciona se os DOIS motores derem o mesmo nome pro
# mesmo erro — é o contrato do LANGUAGE.md, não estética de mensagem.

@pytest.mark.parametrize("captura,src", [
    ("SomeValueUnexpected",  "post(1/0)"),
    ("SomeValueUnexpected",  "post(1%0)"),
    ("SomeValueUnexpected",  'post("a" > 1)'),
    ("SomeValueUnexpected",  'post("a" - 1)'),
    ("SomeValueUnexpected",  "post(len(5))"),
    ("SomeValueUnexpected",  'post(int("a"))'),
    ("AtributtedValueError", 'post(1 + "a")'),
    ("KeyError",             'post({"a":1}["z"])'),
    ("OutputUnexpectedValues", "a, b = [1]"),
])
def test_nome_do_erro_casa_nos_dois(captura, src):
    prog = ('try {' + NL + ' ' + src + NL + '} catch (' + captura + ' e) {'
            + NL + ' post("pego")' + NL + '}')
    assert via_c(prog) == via_interpretador(prog) == ["pego"]


# ═════════════════════════ f-string: erro no trecho ESTOURA ═════════════════

@pytest.mark.parametrize("src", [
    'post(f"Ola, {nome}!")',                     # nome indefinido
    'post(f"x {1/0} y")',                        # expressão que estoura
    'a = 1' + NL + 'post(f"{a} {b} {a+1}")',     # mistura: o `b` não existe
])
def test_fstring_trecho_com_erro_estoura(src):
    """Trecho de f-string que estoura LEVANTA o erro, nos dois motores.

    Antes cada trecho tinha uma rede: o que falhasse saía como o texto cru
    entre chaves — `f"oi {nome}"` sem `nome` imprimia `oi {nome}` e o bug de
    quem escreveu sumia em silêncio (foi assim que um `{solucao}` fora de
    escopo virou saída "normal"). Erro engolido é pior que erro barulhento."""
    ambos_falham(src)


@pytest.mark.parametrize("src,esperado", [
    ('nome = "ana"' + NL + 'post(f"Ola, {nome}!")', ["Ola, ana!"]),
    ('post(f"{{literal}} e {1 + 1}")',              ["{literal} e 2"]),
    ('a = 1' + NL + 'post(f"{a} {a + 1}")',         ["1 2"]),
])
def test_fstring_valida_continua_interpolando(src, esperado):
    """O aperto acima não pode ter pegado f-string boa: nome definido,
    expressão e escape `{{`/`}}` seguem funcionando igual nos dois."""
    assert via_c(src) == via_interpretador(src) == esperado


# ═════════════════════════ PUSH ... GET ... ═════════════════════════════════

def test_push_e_import(monkeypatch):
    monkeypatch.setenv("POOL_TEST_VAR", "v1")
    for src, esperado in [
        ("PUSH os" + NL + "post(os.cwd() is str)", ["True"]),
        ('PUSH os GET getenv' + NL + 'post(getenv("POOL_TEST_VAR"))', ["v1"]),
        ('PUSH os GET getenv as ler' + NL + 'post(ler("POOL_TEST_VAR"))', ["v1"]),
        ("PUSH os as sis" + NL + "post(sis.cwd() is str)", ["True"]),
        ('PUSH os as sis GET getenv, cwd' + NL
         + 'post(getenv("POOL_TEST_VAR"), cwd() is str)', ["v1 True"]),
    ]:
        assert via_c(src) == esperado, src


def test_push_get_nao_expoe_o_modulo():
    """Com GET, só os nomes pedidos entram — o módulo em si não."""
    ambos_falham("PUSH os GET getenv" + NL + "post(os.cwd())")


# ═════════════════════════ unpack de gerador ════════════════════════════════

@pytest.mark.parametrize("src", [
    'action g() {' + NL + ' yield 1' + NL + ' yield 2' + NL + '}' + NL
    + 'a, b = g()' + NL + 'post(a, b)',
    'action g() {' + NL + ' yield 1' + NL + ' yield 2' + NL + ' yield 3' + NL + '}' + NL
    + 'a, *r = g()' + NL + 'post(a, r)',
])
def test_unpack_de_gerador(src):
    mesmo(src)


def test_unpack_de_gerador_curto_falha(  ):
    ambos_falham('action g() {' + NL + ' yield 1' + NL + '}' + NL + 'a, b = g()')


# ═════════════════════════ argumento nomeado em nativa ══════════════════════

@pytest.mark.parametrize("src", [
    'import regex' + NL + 'post(regex.sub("a", "b", "aaa", count=2))',
    'str s = "aaa"' + NL + 'post(s.replace("a", "b", count=2))',
    'post("a,b,c".split(",", maxsplit=1))',
    'post("aaa".replace(old="a", new="b", count=1))',
    'import json' + NL + 'post(json.parse(text="[1,2]"))',
])
def test_nativa_aceita_argumento_nomeado(src):
    mesmo(src)


@pytest.mark.parametrize("src", [
    'post("aaa".replace("a", "b", conta=2))',    # nome que não existe
    "post(len(x=[1,2]))",                        # função sem tabela de nomes
])
def test_nativa_recusa_nome_errado(src):
    ambos_falham(src)


# ═════════════════════════ stubs e extras ═══════════════════════════════════

@pytest.mark.parametrize("mod", ["flask", "sqlite", "smtplib", "mimetext", "multipart"])
def test_lib_stub_importa_mas_nao_roda(mod):
    """As stubs existem pra dar erro CLARO na chamada, não no import."""
    assert via_c("import " + mod + NL + 'post("ok")') == ["ok"]


def test_lib_stub_erro_e_capturavel():
    src = ('import flask' + NL + 'try {' + NL + ' flask.run()' + NL
           + '} catch (NotImplemented e) {' + NL + ' post("pego")' + NL + '}')
    assert via_c(src) == ["pego"]


@pytest.mark.parametrize("src", [
    's = "{' + chr(92) + '"x' + chr(92) + '": 42}"' + NL + 'post(s.get_json("x"))',
    's = "[1, 2]"' + NL + 'post(s.get_json())',
    'post("nao json".get_json())',
    'post("nao json".get("k"))',
])
def test_get_json_em_string(src):
    mesmo(src)


def test_post_vazio_nao_imprime_nada():
    assert via_c("post()") == []
    assert via_c('post("")') == [""]


def test_base_sem_heranca_e_noop_nos_dois():
    src = ("Entity A():" + NL + "    action __init__(self):" + NL
           + "        base()" + NL + "post(1)")
    mesmo(src)


def test_decorador_desconhecido_sozinho_e_ignorado():
    mesmo("@qualquer" + NL + 'post("segue")')


# ═════════════════════════ iteração de string é por codepoint ═══════════════
# `iteravel_tam`/`iteravel_item` e o ITER_NEXT contavam BYTES: `list("ção")`
# devolvia 5 pedaços de lixo. Tudo que itera string passa por aqui.

@pytest.mark.parametrize("src", [
    'post(list("ção"))',
    'for each c in "ção" {' + NL + ' post(c)' + NL + '}',
    'post(sorted("bça"))',
    'post(reversed("ção"))',
    'post(enumerate("çá"))',
    'post(list(zip("çã", [1,2])))',
    'post(min("çab"), max("çab"))',
    'a, b, c = "çãx"' + NL + 'post(a, b, c)',
])
def test_iteracao_de_string_por_codepoint(src):
    mesmo(src)


# ═════════════════════════ Parsing ══════════════════════════════════════════
# Builtin global, sem import. As conversões seguem o `parsing_lib.py` à risca —
# inclusive a assimetria: `string(v, "flo")` limpa como TEXTO (só dígitos) e
# `floating(v)` trata vírgula BR, então os dois dão resultados diferentes pro
# mesmo "R$ 1.299,90".

@pytest.mark.parametrize("src", [
    'post(Parsing.string("  a   b  c "))',
    'post(Parsing.string("R$ 1.299,90", "int"))',
    'post(Parsing.string("R$ 1.299,90", "flo"))',    # 129990.0, não 1299.9
    'post(Parsing.integer("R$ 1.299,90"))',
    'post(Parsing.integer(123.7))',                  # trunca, não vira 1237
    'post(Parsing.integer("abc"))',                  # sem dígito: 0
    'post(Parsing.floating("1.299,90"))',            # vírgula BR: 1299.9
    'post(Parsing.floating("1,5"))',
    'post(Parsing.TransientValue("123.7", "int"))',
    'post(Parsing.Arrayformatt("ção"))',
    'post(Parsing.integer("10") + Parsing.integer("5"))',
    'v = Parsing.integer("R$ 50")' + NL + 'if v > 40 {' + NL + ' post("caro")' + NL + '}',
    'post(Parsing)',
])
def test_parsing_conversoes_br(src):
    mesmo(src)


# ── string colorida <cor> com interpolação (regressão de paridade) ──────────
# BUG antigo: `<red>f"...{x}"` na VM NÃO interpolava (embutia o template cru),
# enquanto o interpretador interpolava. Cor só valia pra string literal.

def test_cor_com_fstring_interpola():
    mesmo('io = "ola"\npost(<red>f"cuuuuu {io}")')

def test_cor_com_string_simples_nao_interpola():
    # string comum: {x} é literal nos DOIS (esperado)
    mesmo('io = "ola"\npost(<red>"cuuuuu {io}")')

def test_cor_verde_fstring():
    mesmo('n = 25\npost(<00ff00>f"grau {n}")')

def test_cor_com_interpolacao_justaposta():
    mesmo('x = 7\npost(<blue>"val " {x} " fim")')


# ── `to <tipo>` (acucar do Parsing) — vira a string do nome do tipo ─────────
def test_to_cast_vira_string():
    mesmo('post(to int)')
    mesmo('post(to str)')
    mesmo('post(Parsing.string("  ana  ", to str))')
