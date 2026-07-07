"""Testes profundos de `async`/`await`.

O objetivo aqui é verificar a SEMÂNTICA observável de async/await da
PoolScript (implementada sobre `PoolFuture` + `ThreadPoolExecutor`, ver
interpreter.py) contra o que se espera de qualquer linguagem com
async/await (igual ao Python): não bloqueante na chamada, bloqueante só
no `await`, propagação de exceção, composição (nested await, gather de
lista), independência entre chamadas, e as operações de baixo nível do
future (`.done()` / `.result(timeout=)`).

Cobre casos que os 7 testes rasos anteriores (test_v5_features.py) não
cobriam: concorrência real medida por tempo, exceção assíncrona, listas
mistas com exceção, encadeamento, estado de `self`, recursão, timeout.
"""
from __future__ import annotations

import time

import pytest

from poolscript.interpreter import run_source, Interpreter, PoolFuture, PoolRuntimeError
from poolscript.parser import parse_source


def run(src: str):
    return run_source(src, "<test>")


# ───────────────────────── não-bloqueante / concorrência real ───────────────

def test_async_call_returns_immediately_without_blocking():
    # Chamar uma async action não deve bloquear — só o await bloqueia.
    start = time.perf_counter()
    out = run(
        "async action lenta() {\n"
        "    sleep(0.3)\n"
        "    return 1\n"
        "}\n"
        "f = lenta()\n"
        "post(\"chamou\")\n"
    )
    elapsed = time.perf_counter() - start
    assert out == ["chamou"]
    assert elapsed < 0.25  # não esperou o sleep(0.3) interno


def test_two_async_actions_run_concurrently():
    # Duas actions async que dormem 0.3s cada devem rodar em paralelo:
    # await das duas juntas deve custar ~0.3s, não ~0.6s (seriado).
    start = time.perf_counter()
    out = run(
        "async action lenta(n) {\n"
        "    sleep(0.3)\n"
        "    return n\n"
        "}\n"
        "a = lenta(1)\n"
        "b = lenta(2)\n"
        "post(await a)\n"
        "post(await b)\n"
    )
    elapsed = time.perf_counter() - start
    assert out == ["1", "2"]
    assert elapsed < 0.55, f"esperado ~0.3s (paralelo), levou {elapsed:.2f}s"


def test_many_async_actions_run_concurrently():
    start = time.perf_counter()
    out = run(
        "async action lenta(n) {\n"
        "    sleep(0.25)\n"
        "    return n\n"
        "}\n"
        "fs = [lenta(1), lenta(2), lenta(3), lenta(4)]\n"
        "rs = await fs\n"
        "post(rs)\n"
    )
    elapsed = time.perf_counter() - start
    assert out == ["[1, 2, 3, 4]"]
    assert elapsed < 0.6, f"esperado ~0.25s (paralelo), levou {elapsed:.2f}s"


# ───────────────────────── exceção só aparece no await ──────────────────────

def test_exception_does_not_surface_before_await():
    # A chamada da async action não deve lançar nada — só o await.
    src = (
        "async action falha() {\n"
        "    x = 1 / 0\n"
        "    return x\n"
        "}\n"
        "f = falha()\n"
        "sleep(0.05)\n"          # dá tempo da thread terminar (com erro)
        "post(\"nao_lancou_ainda\")\n"
    )
    assert run(src) == ["nao_lancou_ainda"]


def test_exception_surfaces_on_await_as_pool_runtime_error():
    with pytest.raises(PoolRuntimeError):
        run(
            "async action falha() {\n"
            "    return 1 / 0\n"
            "}\n"
            "f = falha()\n"
            "await f\n"
        )


def test_exception_message_preserved_through_await():
    try:
        run(
            "async action falha() {\n"
            "    return 1 / 0\n"
            "}\n"
            "await falha()\n"
        )
        assert False, "deveria ter levantado PoolRuntimeError"
    except PoolRuntimeError as e:
        assert "divis" in e.msg.lower() or "division" in e.msg.lower()


# ───────────────────────── await de lista (gather-like) ─────────────────────

def test_await_list_with_mixed_ready_and_future_values():
    out = run(
        "async action dobra(n) {\n"
        "    return n * 2\n"
        "}\n"
        "fs = [dobra(1), 99, dobra(3)]\n"
        "rs = await fs\n"
        "post(rs)\n"
    )
    assert out == ["[2, 99, 6]"]


def test_await_list_with_exception_in_the_middle_raises():
    with pytest.raises(PoolRuntimeError):
        run(
            "async action ok(n) { return n }\n"
            "async action falha() { return 1 / 0 }\n"
            "fs = [ok(1), falha(), ok(3)]\n"
            "await fs\n"
        )


# ───────────────────────── nested / encadeado ───────────────────────────────

def test_nested_await_async_calling_async():
    out = run(
        "async action interno(n) {\n"
        "    return n + 1\n"
        "}\n"
        "async action externo(n) {\n"
        "    v = await interno(n)\n"
        "    return v * 10\n"
        "}\n"
        "post(await externo(4))\n"
    )
    assert out == ["50"]


def test_async_recursion():
    out = run(
        "async action fat(n) {\n"
        "    if (n <= 1) {\n"
        "        return 1\n"
        "    }\n"
        "    prev = await fat(n - 1)\n"
        "    return n * prev\n"
        "}\n"
        "post(await fat(5))\n"
    )
    assert out == ["120"]


# ───────────────────────── async em Entity / self ───────────────────────────

def test_async_method_accesses_entity_state():
    out = run(
        "Entity Conta():\n"
        "    action __init__(self, saldo):\n"
        "        self.saldo = saldo\n"
        "    async action depositar(self, valor):\n"
        "        self.saldo = self.saldo + valor\n"
        "        return self.saldo\n"
        "c = Conta(100)\n"
        "post(await c.depositar(50))\n"
        "post(c.saldo)\n"
    )
    assert out == ["150", "150"]


def test_async_method_multiple_instances_independent_state():
    out = run(
        "Entity Contador():\n"
        "    action __init__(self, inicio):\n"
        "        self.n = inicio\n"
        "    async action inc(self):\n"
        "        self.n = self.n + 1\n"
        "        return self.n\n"
        "a = Contador(0)\n"
        "b = Contador(100)\n"
        "post(await a.inc())\n"
        "post(await b.inc())\n"
    )
    assert out == ["1", "101"]


# ───────────────────────── independência entre chamadas ─────────────────────

def test_independent_futures_per_call_no_shared_state():
    out = run(
        "async action ident(n) {\n"
        "    sleep(0.05)\n"
        "    return n\n"
        "}\n"
        "a = ident(1)\n"
        "b = ident(2)\n"
        "c = ident(3)\n"
        "post(await c)\n"
        "post(await b)\n"
        "post(await a)\n"
    )
    # Mesmo aguardando fora de ordem, cada future guarda o resultado da SUA
    # própria chamada (não há vazamento/compartilhamento entre futures).
    assert out == ["3", "2", "1"]


# ───────────────────────── retorno vazio / None ─────────────────────────────

def test_async_action_with_no_return_yields_none():
    out = run(
        "async action semRetorno() {\n"
        "    x = 1 + 1\n"
        "}\n"
        "post(await semRetorno())\n"
    )
    assert out == ["null"]


def test_async_bare_return_yields_none():
    out = run(
        "async action vazio() {\n"
        "    return\n"
        "}\n"
        "post(await vazio())\n"
    )
    assert out == ["null"]


# ───────────────────────── .done() / .result(timeout=) ──────────────────────

def test_future_done_false_then_true():
    out = run(
        "async action lenta() {\n"
        "    sleep(0.2)\n"
        "    return 1\n"
        "}\n"
        "f = lenta()\n"
        "post(f.done())\n"
        "await f\n"
        "post(f.done())\n"
    )
    assert out == ["False", "True"]


def test_future_result_same_as_await():
    out = run(
        "async action valor() {\n"
        "    return 42\n"
        "}\n"
        "f = valor()\n"
        "post(f.result())\n"
    )
    assert out == ["42"]


def test_future_result_timeout_raises():
    with pytest.raises(PoolRuntimeError):
        run(
            "async action lenta() {\n"
            "    sleep(1)\n"
            "    return 1\n"
            "}\n"
            "f = lenta()\n"
            "f.result(0.01)\n"
        )


# ───────────────────────── await de valor comum (pass-through) ─────────────

def test_await_plain_value_passthrough():
    assert run("post(await 42)") == ["42"]
    assert run('post(await "texto")') == ["texto"]
    assert run("post(await Null)") == ["null"]


# ───────────────────────── tipo de retorno (int/bool) + async ──────────────

def test_async_int_reaction_error_returns_500_like_sync():
    out = run(
        "async int reaction f() {\n"
        "    return 1 / 0\n"
        "}\n"
        "post(await f())\n"
    )
    assert out == ["500"]


def test_async_bool_reaction_error_returns_false_like_sync():
    out = run(
        "async bool reaction f() {\n"
        "    return 1 / 0\n"
        "}\n"
        "post(await f())\n"
    )
    assert out == ["False"]
