"""Pipeline completo em C — `.ps` → lexer → parser → compilador → VM.

Este é o teste que fecha o laço da migração. Os outros comparam ARTEFATOS
(tokens, AST, bytecode) contra as implementações Python transitórias; este
compara a **saída da execução** contra o **interpretador**, que é a
autoridade semântica da linguagem.

A distinção importa: `vm/compiler.py` é código de transição escrito durante a
migração e já teve bug próprio (action aninhada virava global, divergindo do
interpretador). Casar só com ele propagaria seus erros para o C. Aqui não há
intermediário — o C roda sozinho e o resultado tem que bater com a referência.

`post` é nativo em C e escreve por `printf`, então a captura é no nível do
descritor de arquivo; `redirect_stdout` não enxergaria nada.
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

if not hasattr(vm, "executa_fonte"):
    pytest.skip("VM sem pipeline em C (recompile)", allow_module_level=True)


def via_c(src):
    """Roda tudo em C e devolve as linhas impressas."""
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


def via_interpretador(src):
    i = Interpreter(source=src, filename="<test>")
    with redirect_stdout(io.StringIO()):
        i.run(parse_source(src, "<test>"))
    return i.output


def mesmo(src):
    assert via_c(src) == via_interpretador(src)


# ── números ─────────────────────────────────────────────────────────────────
@pytest.mark.parametrize("src", [
    "post(2 + 3 * 4)", "post((2 + 3) * 4)", "post(10 - 3 - 2)",
    "post(7 / 2)", "post(7 % 3)", "post(0 - 5)",
    "post(1.5 + 2.5)",
])
def test_aritmetica(src):
    mesmo(src)


@pytest.mark.parametrize("src", [
    "post(5 ^ 3)", "post(5 | 2)", "post(6 & 3)",
    "post(1 << 4)", "post(256 >> 4)", "post(~5)",
    "post(2 + 3 << 1)",
])
def test_bitwise(src):
    mesmo(src)


@pytest.mark.parametrize("src", [
    "post(3 < 5)", "post(5 <= 5)", "post(2 == 2)", "post(2 != 3)",
])
def test_comparacao(src):
    mesmo(src)


def test_null_igual_zero():
    """Regra do spec, implementada nos dois lados de forma independente."""
    mesmo("post(Null == 0)")


# ── strings ─────────────────────────────────────────────────────────────────
@pytest.mark.parametrize("src", [
    'post("oi")', 'post("a" + "b")', 'post("ação")',
    'post("abc" == "abc")', 'post("abc" < "abd")',
])
def test_strings(src):
    mesmo(src)


# ── variáveis e fluxo ───────────────────────────────────────────────────────
def test_declaracao_e_composta():
    mesmo("int x = 10\nx += 5\npost(x)")


@pytest.mark.parametrize("n", [15, 8, 3])
def test_if_elif_else(n):
    mesmo(f"int n = {n}\nif (n > 10) {{\n post(1)\n}} elif (n > 5) {{\n post(2)\n}} else {{\n post(3)\n}}")


def test_while():
    mesmo("int i = 0\nint s = 0\nwhile (i < 5) {\n s += i\n i += 1\n}\npost(s)")


# ── funções ─────────────────────────────────────────────────────────────────
def test_action():
    mesmo("action dobro(n) {\n return n * 2\n}\npost(dobro(21))")


def test_action_sem_return():
    mesmo("action nada() {\n int x = 1\n}\npost(nada())")


def test_recursao():
    mesmo("action fib(n) {\n if (n < 2) {\n  return n\n }\n return fib(n-1) + fib(n-2)\n}\npost(fib(15))")


def test_locais_isolados():
    mesmo("action f(a) {\n int b = a * 2\n return b\n}\npost(f(5))\npost(f(7))")


def test_action_aninhada_e_local():
    """Action interna é local da que a contém — não vaza pro global.

    Foi aqui que o compilador Python divergiu do interpretador (emitia
    STORE_GLOBAL). Este teste valida a semântica, não o formato do bytecode.
    """
    mesmo("action ext() {\n action int_() {\n  return 1\n }\n return int_()\n}\npost(ext())")


# ── estruturas ──────────────────────────────────────────────────────────────
@pytest.mark.parametrize("src", [
    "post([1, 2, 3])", "post([])", "post([1,2,3][1])", "post([1,2,3][-1])",
    'd = {"a": 1, "b": 2}\npost(d["a"])',
    'post([[1,2],[3,4]][1][0])',
])
def test_estruturas(src):
    mesmo(src)


@pytest.mark.parametrize("src", [
    "post(len([1,2,3]))", 'post(len("pool"))', 'post(len({"a":1}))',
])
def test_len_nativo(src):
    mesmo(src)


# ── lote A: for each, break, continue, ++/-- ───────────────────────────────
@pytest.mark.parametrize("src", [
    "for each i in [1,2,3] {\n post(i)\n}",
    "for each i in [] {\n post(i)\n}\npost(\"fim\")",
    "l = [10,20]\nfor each x in l {\n post(x)\n}",
    "for each i in [1,2]:\n    post(i)",
    "for each par in [[1,2],[3,4]] {\n post(par[0])\n}",
    's = "abc"\nfor each c in s {\n post(c)\n}',
    'for each c in "abc" {\n post(c)\n}',      # literal direto (correção)
    'for each c in "abc":\n    post(c)',
])
def test_for_each(src):
    mesmo(src)


def test_for_each_aninhado():
    mesmo("for each i in [1,2] {\n for each j in [3,4] {\n  post(j)\n }\n}")


def test_for_each_acumula():
    mesmo("int s = 0\nfor each i in [1,2,3,4] {\n s += i\n}\npost(s)")


@pytest.mark.parametrize("src", [
    "for each i in [1,2,3,4] {\n if (i == 3) {\n  break\n }\n post(i)\n}",
    "for each i in [1,2,3,4] {\n if (i == 2) {\n  continue\n }\n post(i)\n}",
    "int i = 0\nwhile (i < 10) {\n i += 1\n if (i == 3) {\n  break\n }\n post(i)\n}",
    "int i = 0\nwhile (i < 5) {\n i += 1\n if (i == 2) {\n  continue\n }\n post(i)\n}",
])
def test_break_continue(src):
    mesmo(src)


def test_break_em_for_each_limpa_a_pilha():
    """`break` sai por fora do ITER_NEXT, que é quem descarta o estado.

    Regressão: sem limpar (container, indice) na saída, o lixo sobrava na
    pilha e corrompia o laço EXTERNO — o teste aninhado entrava em laço
    infinito.
    """
    mesmo("for each i in [1,2] {\n for each j in [1,2,3] {\n  if (j == 2) {\n   break\n  }\n  post(j)\n }\n post(i)\n}")


def test_continue_aninhado():
    mesmo("for each i in [1,2] {\n for each j in [1,2,3] {\n  if (j == 2) {\n   continue\n  }\n  post(j)\n }\n}")


@pytest.mark.parametrize("src", [
    "int x = 5\nx++\npost(x)",
    "int x = 5\nx--\npost(x)",
    "int x = 0\nwhile (x < 3) {\n x++\n}\npost(x)",
])
def test_postfix(src):
    mesmo(src)


def test_for_each_nao_aceita_dict():
    """A linguagem restringe a lista/tupla/string — a VM segue a referência."""
    src = 'd = {"a": 1}\nfor each k in d {\n post(k)\n}'
    with pytest.raises(RuntimeError):
        via_c(src)
    with pytest.raises(Exception):
        via_interpretador(src)


# ── lote B: interpolação, tupla, slice ─────────────────────────────────────
@pytest.mark.parametrize("src", [
    'g = 25\npost("Clima: " {g} "C")',
    'n = 7\npost("v=" {n})',
    'a = 2\npost("r=" {a + 3})',
    'a = 1\nb = 2\npost("a" {a} "b" {b} "c")',
    'x = Null\npost("v=" {x})',
    'l = [1,2]\npost("l=" {l})',
    'action g(n) {\n post("v=" {n})\n}\ng(7)',
])
def test_interpolacao(src):
    """BUILD_STR usa as MESMAS regras de texto do `post`.

    Duplicar a formatação faria a interpolação divergir da impressão sem
    ninguém notar (`null` vs `None`, `4.0` vs `4`).
    """
    mesmo(src)


@pytest.mark.parametrize("src", [
    "t = (1, 2, 3)\npost(t)",
    "t = (1,)\npost(t)",          # tupla de 1 imprime `(1,)`
    "t = ()\npost(t)",
    "t = (10, 20)\npost(t[0])\npost(t[-1])",
    "post(len((1,2,3)))",
    "for each x in (1,2,3) {\n post(x)\n}",
    "post((1,2) == (1,2))",
])
def test_tupla(src):
    """Tupla compartilha o layout de lista; muda a etiqueta e a impressão."""
    mesmo(src)


@pytest.mark.parametrize("src", [
    "l = [1,2,3,4,5]\npost(l[1:3])",
    "l = [1,2,3,4,5]\npost(l[:2])",
    "l = [1,2,3,4,5]\npost(l[2:])",
    "l = [1,2,3,4,5]\npost(l[::-1])",
    "l = [1,2,3,4,5]\npost(l[::2])",
    "l = [1,2,3,4,5]\npost(l[1:4:2])",
    "l = [1,2,3,4,5]\npost(l[-2:])",
    "l = [1,2]\npost(l[0:99])",      # limites saturam, não estouram
    "t = (1,2,3,4)\npost(t[1:3])",
])
def test_slice_lista(src):
    mesmo(src)


def test_slice_string():
    mesmo('s = "poolscript"\npost(s[0:4])\npost(s[4:])\npost(s[::-1])')


def test_slice_passo_zero_da_erro():
    with pytest.raises(RuntimeError):
        via_c("post([1,2,3][::0])")


# ── lote C: escopo dinâmico, `global`, parâmetro com padrão ────────────────
def test_atribuicao_modifica_variavel_externa():
    """Atribuir dentro de uma função MODIFICA a externa, se ela existir.

    Não é a regra do Python (que exigiria `global`). O interpretador usa
    `Scope.set`, que sobe a cadeia de escopos; só cria local se o nome não
    existir em lugar nenhum. Como isso depende do estado em runtime, o
    compilador emite LOAD_NAME/STORE_NAME e a decisão fica na VM.
    """
    mesmo("x = 1\naction f() {\n x = 99\n}\nf()\npost(x)")


def test_atribuicao_cria_local_se_externa_nao_existe():
    src = "action f() {\n y = 99\n}\nf()\npost(y)"
    with pytest.raises(RuntimeError):
        via_c(src)
    with pytest.raises(Exception):
        via_interpretador(src)


@pytest.mark.parametrize("src", [
    "x = 1\naction f(x) {\n x = 99\n}\nf(5)\npost(x)",      # parâmetro sombreia
    "x = 1\naction f() {\n int x = 99\n}\nf()\npost(x)",    # tipada cria local
    "x = 1\naction f() {\n action g() {\n  x = 99\n }\n g()\n}\nf()\npost(x)",
    "x = 7\naction f() {\n return x * 2\n}\npost(f())",
    "action f() {\n z = 5\n return z + 1\n}\npost(f())",
    "x = Null\naction f() {\n x = 9\n}\nf()\npost(x)",       # Null conta como "existe"
])
def test_resolucao_de_nome(src):
    mesmo(src)


def test_ler_variavel_nao_definida_e_erro():
    """Não pode devolver Null calado — um typo viraria `null` silencioso."""
    with pytest.raises(RuntimeError):
        via_c("post(zzz)")
    with pytest.raises(Exception):
        via_interpretador("post(zzz)")


@pytest.mark.parametrize("src", [
    "x = 1\naction f() {\n global x\n x = 99\n}\nf()\npost(x)",
    "a = 1\nb = 2\naction f() {\n global a, b\n a = 10\n b = 20\n}\nf()\npost(a)\npost(b)",
    "c = 0\naction inc() {\n global c\n c += 1\n}\ninc()\ninc()\npost(c)",
])
def test_global(src):
    mesmo(src)


@pytest.mark.parametrize("src", [
    "action f(a, b=10) {\n return a + b\n}\npost(f(1))\npost(f(1,2))",
    "action f(a, b=1, c=2) {\n return a+b+c\n}\npost(f(0))\npost(f(0,10))\npost(f(0,10,100))",
    'action f(s="oi") {\n return s\n}\npost(f())\npost(f("tchau"))',
    "action f(a, b=2*3) {\n return a + b\n}\npost(f(1))",
    "action f(a=Null) {\n return a\n}\npost(f())\npost(f(5))",
    "action f(a=1, b=2) {\n return a+b\n}\npost(f())\npost(f(10))\npost(f(10,20))",
])
def test_parametro_com_padrao(src):
    """O default é avaliado no CALLEE, não no call site.

    O compilador não sabe qual função será chamada (alvo dinâmico), então
    quem conhece os defaults é o próprio protótipo: o prólogo consulta
    quantos argumentos chegaram e só avalia os que faltaram.
    """
    mesmo(src)


# ── lote D: f-string e argumento nomeado ───────────────────────────────────
@pytest.mark.parametrize("src", [
    'nome = "pool"\npost(f"Ola, {nome}!")',
    'post(f"texto puro")',
    'x = 5\npost(f"{x}")',
    'a = 1\nb = 2\npost(f"a={a} b={b}")',
    'x = 3\npost(f"dobro={x * 2}")',
    'post(f"{{literal}}")',                 # {{ }} escapam
    'x = Null\npost(f"v={x}")',
    'l = [1,2]\npost(f"l={l}")',
    'action g(n) {\n return f"n={n}"\n}\npost(g(9))',
    'for each i in [1,2,3] {\n post(f"i={i}")\n}',
    'action d(n) { return n*2 }\nx=5\npost(f"r={d(x)}")',
    'post(f"")',
    "d = {\"k\": 7}\npost(f\"v={d['k']}\")",
    'l=[[1,2]]\npost(f"v={l[0][1]}")',
])
def test_fstring(src):
    """O template é cortado em tempo de COMPILAÇÃO.

    O interpretador reparseia o trecho `{expr}` a cada avaliação; aqui ele
    vira bytecode normal, como se estivesse escrito fora da string — o custo
    sai do laço.
    """
    mesmo(src)


@pytest.mark.parametrize("src", [
    "action f(a, b) {\n return a - b\n}\npost(f(a=10, b=3))",
    "action f(a, b) {\n return a - b\n}\npost(f(b=3, a=10))",     # fora de ordem
    "action f(a, b) {\n return a - b\n}\npost(f(10, b=3))",       # misto
    "action f(a, b=100) {\n return a + b\n}\npost(f(a=1))\npost(f(a=1, b=2))",
    'action g(nome="x") {\n return nome\n}\npost(g(nome="pool"))',
    "action f(a=9) {\n return a\n}\npost(f(a=Null))",             # Null preenche o slot
])
def test_argumento_nomeado(src):
    """A VM reposiciona pelos nomes de parâmetro do protótipo.

    O call site não pode fazer isso: o alvo da chamada só se conhece em
    runtime.
    """
    mesmo(src)


@pytest.mark.parametrize("src", [
    "action f(a, b=5, c=10) {\n return a+b+c\n}\npost(f(1, c=100))",
    "action f(a=1, b=2, c=3) {\n return a+b+c\n}\npost(f(c=30, a=10))",
])
def test_nomeado_pode_deixar_buraco(src):
    """`f(1, c=100)` pula o `b`, que recebe o default dele.

    Regressão: a primeira versão contava argumentos, o que assumia um
    prefixo contíguo e recusava o buraco. Passou a checar o SLOT (sentinela
    UNSET), que trata o caso naturalmente.
    """
    mesmo(src)


def test_nomeado_sobrescreve_posicional():
    """`f(1, a=2)` devolve 2 — recusar seria mais restritivo que a linguagem."""
    mesmo("action f(a) {\n return a\n}\npost(f(1, a=2))")


def test_nomeado_inexistente_da_erro():
    src = "action f(a) {\n return a\n}\npost(f(zzz=1))"
    with pytest.raises(RuntimeError):
        via_c(src)
    with pytest.raises(Exception):
        via_interpretador(src)


# ── lote E: try/catch/finally e raise ──────────────────────────────────────
@pytest.mark.parametrize("src", [
    "try {\n post(1)\n} catch (e) {\n post(2)\n}",
    'try {\n post(1/0)\n} catch (e) {\n post("pegou")\n}',
    "try {\n post(1/0)\n} catch (e) {\n post(e)\n}",       # mensagem é observável
    "try {\n post(1)\n} catch (e) {\n post(2)\n} finally {\n post(3)\n}",
    "try {\n post(1/0)\n} catch (e) {\n post(2)\n} finally {\n post(3)\n}",
    'try {\n post({"a":1}["z"])\n} catch (e) {\n post("chave")\n}',
    'try {\n post(zzz)\n} catch (e) {\n post("nome")\n}',
    "try {\n post(5 % 0)\n} catch (e) {\n post(e)\n}",
])
def test_try_catch(src):
    mesmo(src)


@pytest.mark.parametrize("src", [
    'action f() {\n return 1/0\n}\ntry {\n post(f())\n} catch (e) {\n post("pego")\n}',
    'action a() {\n return 1/0\n}\naction b() {\n return a()\n}\ntry {\n post(b())\n} catch (e) {\n post("pego")\n}',
    "action f() {\n try {\n  return 1/0\n } catch (e) {\n  return -1\n }\n}\npost(f())",
])
def test_erro_atravessa_frames(src):
    """O handler guarda fp/sp/locals_top E o protótipo corrente.

    Sem gravar o protótipo, o catch retomava no lugar errado quando o erro
    vinha de uma função chamada dentro do try — `frames[fp]` guarda o estado
    do CHAMADOR, não o do frame ativo.
    """
    mesmo(src)


@pytest.mark.parametrize("src", [
    'try {\n raise "meu erro"\n} catch (e) {\n post(e)\n}',
    'try {\n raise 42\n} catch (e) {\n post(e)\n}',
])
def test_raise(src):
    mesmo(src)


@pytest.mark.parametrize("src", [
    'try {\n try {\n  post(1/0)\n } catch (e) {\n  post("interno")\n }\n} catch (e) {\n post("externo")\n}',
    'for each i in [1,2] {\n try {\n  post(1/0)\n } catch (e) {\n  post(i)\n }\n}',
])
def test_try_aninhado_e_em_loop(src):
    mesmo(src)


@pytest.mark.parametrize("src", [
    "post([1,2][9])",       # avisa e devolve Null
    'post("ab"[9])',
    "post([1,2][-9])",
    "post([1,2][-1])",      # negativo válido continua indexando
])
def test_indice_fora_do_intervalo_nao_trava(src):
    """Spec: `IndexOutOfBoundsWarning` — avisa no stderr e devolve Null.

    A VM levantava erro, o que era mais restritivo que a linguagem. Só
    apareceu ao comparar a saída com o interpretador.
    """
    mesmo(src)


# ── lote F: match/case ─────────────────────────────────────────────────────
@pytest.mark.parametrize("src", [
    'x = 1\nmatch x {\n case 1 { post("um") }\n case _ { post("outro") }\n}',
    'x = 5\nmatch x {\n case 1 { post("um") }\n case _ { post("outro") }\n}',
    'x = 1\nmatch x:\n    case 1:\n        post("um")\n    case _:\n        post("outro")',
    's = "abc"\nmatch s {\n case "abc" { post(1) }\n case _ { post(2) }\n}',
    'x = 42\nmatch x {\n case v { post(v) }\n}',
    'x = 0 - 10\nmatch x {\n case -10 { post("neg") }\n case _ { post("nao") }\n}',
    'x = True\nmatch x {\n case True { post(1) }\n case Null { post(2) }\n case _ { post(3) }\n}',
    'x = 99\nmatch x {\n case 1 { post("um") }\n}\npost("fim")',
])
def test_match_basico(src):
    mesmo(src)


@pytest.mark.parametrize("x,esperado", [(1, "a"), (2, "a"), (3, "a"), (9, "b")])
def test_match_or(x, esperado):
    """Padrão `1 | 2 | 3`.

    Regressão: a última alternativa não é duplicada — o booleano dela já é o
    resultado. A primeira versão saltava incondicionalmente pro caminho de
    falha depois dela, ignorando o acerto.
    """
    mesmo(f'x = {x}\nmatch x {{\n case 1 | 2 | 3 {{ post("a") }}\n case _ {{ post("b") }}\n}}')


@pytest.mark.parametrize("src", [
    'l = [1,2]\nmatch l {\n case [1, 2] { post("casou") }\n case _ { post("nao") }\n}',
    'l = [1,2,3]\nmatch l {\n case [1, 2] { post("casou") }\n case _ { post("nao") }\n}',
    'l = [1,[2,3]]\nmatch l {\n case [1, [2, 3]] { post("casou") }\n case _ { post("nao") }\n}',
])
def test_match_lista(src):
    mesmo(src)


@pytest.mark.parametrize("src", [
    'd = {"a": 1}\nmatch d {\n case {a: 1} { post("casou") }\n case _ { post("nao") }\n}',
    'd = {"a": 9}\nmatch d {\n case {a: 1} { post("casou") }\n case _ { post("nao") }\n}',
    'd = {"z": 1}\nmatch d {\n case {a: 1} { post("casou") }\n case _ { post("nao") }\n}',
])
def test_match_dict(src):
    """Chave ausente TESTA falso — não levanta erro como o INDEX_GET faria."""
    mesmo(src)


@pytest.mark.parametrize("x", [9, 2])
def test_match_guarda(x):
    mesmo(f'x = {x}\nmatch x {{\n case v if v > 5 {{ post("maior") }}\n case _ {{ post("menor") }}\n}}')


@pytest.mark.parametrize("src", [
    'action f(n) {\n match n {\n  case 0 { return "zero" }\n  case _ { return "outro" }\n }\n}\npost(f(0))\npost(f(7))',
    'for each i in [1,2,3] {\n match i {\n  case 2 { post("dois") }\n  case _ { post(i) }\n }\n}',
])
def test_match_em_funcao_e_loop(src):
    mesmo(src)


# ── erros param a execução com mensagem, não corrompem ──────────────────────
def test_divisao_por_zero():
    with pytest.raises(RuntimeError):
        via_c("post(1 / 0)")


def test_tipo_incompativel():
    with pytest.raises(RuntimeError):
        via_c('post("a" + 1)')


def test_no_nao_compilavel_da_erro_claro():
    """Nó ainda não migrado para com erro explícito, não com bytecode torto."""
    with pytest.raises(NotImplementedError):
        via_c("async action f() {\n return 1\n}\npost(await f())\npost(1)")


def test_erro_de_sintaxe():
    with pytest.raises(SyntaxError):
        via_c("x = (1")


# ── objetos: Entity, herança, base() ────────────────────────────────────────
# O `self` é parâmetro explícito no `action`, então a chamada de método só
# precisa empurrá-lo como argumento 0 — não há slot mágico no frame.

@pytest.mark.parametrize("src", [
    # campo posto no __init__ e lido de fora
    'Entity P():\n    action __init__(self, n):\n        self.n = n\npost(P(7).n)',
    # método com argumento
    'Entity P():\n    action __init__(self, n):\n        self.n = n\n'
    '    action soma(self, k):\n        return self.n + k\npost(P(7).soma(3))',
    # mutação de campo depois de construído
    'Entity P():\n    action __init__(self, n):\n        self.n = n\n'
    'p = P(1)\np.n = 9\npost(p.n)',
    # instâncias não compartilham estado
    'Entity P():\n    action __init__(self, n):\n        self.n = n\n'
    'a = P(1)\nb = P(2)\na.n = 50\npost(a.n)\npost(b.n)',
    # método chama outro método pelo self
    'Entity P():\n    action __init__(self):\n        self.v = 2\n'
    '    action dobro(self):\n        return self.v * 2\n'
    '    action quadruplo(self):\n        return self.dobro() * 2\npost(P().quadruplo())',
    # Entity dentro de estrutura de dados
    'Entity P():\n    action __init__(self, n):\n        self.n = n\n'
    'l = [P(1), P(2)]\nfor each p in l {\n post(p.n)\n}',
    # sintaxe com chaves e o alias `class`
    'class P() {\n action __init__(self, n) {\n  self.n = n\n }\n}\npost(P(4).n)',
])
def test_entity(src):
    mesmo(src)


@pytest.mark.parametrize("src", [
    # método herdado sem override
    'Entity A():\n    action oi(self):\n        return "oi"\n'
    'Entity B(A):\n    action __init__(self, v):\n        self.v = v\n'
    'b = B(1)\npost(b.oi())\npost(b.v)',
    # override vence o do pai, e o pai continua com o seu
    'Entity A():\n    action f(self):\n        return "A"\n'
    'Entity B(A):\n    action f(self):\n        return "B"\npost(A().f())\npost(B().f())',
    # base() passa o self atual pro __init__ do pai
    'Entity A():\n    action __init__(self, x):\n        self.x = x\n'
    'Entity B(A):\n    action __init__(self, x, y):\n        base(x)\n        self.y = y\n'
    'b = B(1, 2)\npost(b.x)\npost(b.y)',
    # três níveis, cada um chamando o base do seu pai
    'Entity A():\n    action __init__(self, n):\n        self.n = n\n'
    '    action falar(self):\n        return "..."\n'
    'Entity M(A):\n    action __init__(self, n):\n        base(n)\n'
    'Entity C(M):\n    action __init__(self, n):\n        base(n)\n'
    '    action falar(self):\n        return "Au!"\n'
    'c = C("Rex")\npost(c.n)\npost(c.falar())',
    # base() sem argumento, e pai sem __init__ nenhum
    'Entity A():\n    action oi(self):\n        return "oi"\n'
    'Entity B(A):\n    action __init__(self):\n        base()\n        self.v = 5\n'
    'b = B()\npost(b.oi())\npost(b.v)',
    # o argumento de base() é uma expressão avaliada no escopo da filha
    'Entity A():\n    action __init__(self, v):\n        self.v = v\n'
    'Entity B(A):\n    action __init__(self, a, b):\n        base(a + b)\npost(B(3, 4).v)',
    # método do pai enxerga campo escrito pela filha
    'Entity A():\n    action ver(self):\n        return self.z\n'
    'Entity B(A):\n    action __init__(self):\n        self.z = 11\npost(B().ver())',
])
def test_entity_heranca(src):
    mesmo(src)


def test_campo_inexistente_para_com_erro():
    """Ler campo que não existe é erro, não Null silencioso."""
    with pytest.raises(RuntimeError):
        via_c("Entity P():\n    action __init__(self):\n        self.a = 1\npost(P().b)")


def test_base_sem_heranca_e_noop():
    """`base()` numa Entity SEM pai não tem o que inicializar: é no-op, como
    no interpretador. Fora de Entity continua sendo erro."""
    assert via_c("Entity A():\n    action __init__(self):\n        base()\npost(1)") == ["1"]
    with pytest.raises((NotImplementedError, RuntimeError, SyntaxError)):
        via_c("base()")
