"""Módulo `sqlite3` em C — mesma resposta que o `sqlite3_lib.py`.

Igual ao `os`: mexe em disco, então cada caso roda duas vezes num diretório
limpo — uma pro interpretador, outra pra VM — e a comparação é sobre o stdout
inteiro. O arquivo `.db` de um motor nunca é visto pelo outro; o que se
compara é o comportamento, não o banco.

O que este arquivo protege que não é óbvio:

- a VM abre transação implícita antes de DML, como o `sqlite3` do Python —
  sem isso `rollback()` não desfaz nada (a sqlite crua fica em autocommit);
- `using` numa conexão COMITA antes de fechar (contrato do `__exit__`);
- todo erro da sqlite sai como `DatabaseError`, o nome que o
  `catch (DatabaseError e)` compara.
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

NL = chr(10)


def via_interpretador(src, dirbase):
    velho = os.getcwd()
    os.chdir(dirbase)
    try:
        buf = io.StringIO()
        i = Interpreter(source=src, filename=str(dirbase / "t.ps"))
        with redirect_stdout(buf):
            i.run(parse_source(src, "<t>"))
        return buf.getvalue().splitlines()
    finally:
        os.chdir(velho)


def via_c(src, dirbase):
    velho = os.getcwd()
    os.chdir(dirbase)
    sys.stdout.flush()
    r, w = os.pipe()
    original = os.dup(1)
    try:
        os.dup2(w, 1)
        os.close(w)
        try:
            vm.executa_fonte(src, str(dirbase / "t.ps"))
        finally:
            sys.stdout.flush()
            os.dup2(original, 1)
    finally:
        os.close(original)
        os.chdir(velho)
    with os.fdopen(r, "rb") as f:
        return f.read().decode("utf-8", "replace").splitlines()


@pytest.fixture
def mesmo(tmp_path):
    def _mesmo(src):
        a = tmp_path / "interp"
        b = tmp_path / "vm"
        a.mkdir()
        b.mkdir()
        assert via_c(src, b) == via_interpretador(src, a)
    return _mesmo


IMP = "import sqlite3" + NL
PREP = (IMP + 'c = sqlite3.connect("t.db")' + NL + 'k = c.cursor()' + NL
        + 'k.execute("CREATE TABLE u (id INTEGER PRIMARY KEY, nome TEXT, nota REAL)")' + NL
        + 'k.execute("INSERT INTO u (nome, nota) VALUES (?, ?)", ("ana", 9.5))' + NL
        + 'k.execute("INSERT INTO u (nome, nota) VALUES (?, ?)", ("bo", 7.0))' + NL)


# ── objetos e repr ──────────────────────────────────────────────────────────

@pytest.mark.parametrize("src", [
    IMP + 'c = sqlite3.connect("t.db")' + NL + 'post(c)' + NL + 'post(type(c))' + NL + 'c.close()',
    IMP + 'c = sqlite3.connect("t.db")' + NL + 'k = c.cursor()' + NL
        + 'post(k)' + NL + 'post(type(k))' + NL + 'c.close()',
])
def test_repr_e_tipo(src, mesmo):
    mesmo(src)


# ── execute e fetch ─────────────────────────────────────────────────────────

@pytest.mark.parametrize("src", [
    PREP + 'post(k.execute("SELECT * FROM u").fetchall())' + NL + 'c.close()',
    PREP + 'post(k.execute("SELECT nome FROM u WHERE id = ?", (2,)).fetchone())' + NL + 'c.close()',
    PREP + 'post(k.execute("SELECT * FROM u WHERE id = 99").fetchone())' + NL + 'c.close()',
    PREP + 'post(k.execute("SELECT * FROM u").fetchmany(1))' + NL
         + 'post(k.fetchmany(9))' + NL + 'c.close()',
    IMP + 'c = sqlite3.connect("t.db")' + NL
        + 'post(c.execute("SELECT 1 + 1 AS soma, ?", ("oi",)).fetchall())' + NL + 'c.close()',
])
def test_select(src, mesmo):
    """Cada linha é um dict {coluna: valor}, na ordem das colunas."""
    mesmo(src)


@pytest.mark.parametrize("src", [
    # DDL devolve null; DML devolve o cursor pra encadear
    IMP + 'c = sqlite3.connect("t.db")' + NL
        + 'post(c.cursor().execute("CREATE TABLE a (x)"))' + NL + 'c.close()',
    # o atalho da conexão devolve o cursor SEMPRE, mesmo em DDL
    IMP + 'c = sqlite3.connect("t.db")' + NL
        + 'c.execute("CREATE TABLE x (a INTEGER)")' + NL
        + 'c.execute("INSERT INTO x VALUES (1)")' + NL + 'c.commit()' + NL
        + 'post(c.execute("SELECT * FROM x").fetchall())' + NL + 'c.close()',
    # PRAGMA é DDL pro wrapper: devolve null mesmo tendo resultado
    IMP + 'c = sqlite3.connect("t.db")' + NL + 'k = c.cursor()' + NL
        + 'post(k.execute("PRAGMA user_version"))' + NL + 'c.close()',
])
def test_ddl_e_atalho(src, mesmo):
    mesmo(src)


@pytest.mark.parametrize("src", [
    # sem resultset pendente: fetchone é null, fetchall é [] — sem erro
    PREP + 'post(k.fetchone())' + NL + 'c.close()',
    PREP + 'post(k.fetchall())' + NL + 'c.close()',
])
def test_fetch_sem_resultset(src, mesmo):
    mesmo(src)


# ── rowcount e lastrowid ────────────────────────────────────────────────────

@pytest.mark.parametrize("src", [
    PREP + 'post(k.rowcount, k.lastrowid)' + NL + 'c.close()',
    PREP + 'k.execute("SELECT * FROM u")' + NL + 'post(k.rowcount)' + NL + 'c.close()',
    PREP + 'k.execute("UPDATE u SET nota = 0")' + NL + 'post(k.rowcount)' + NL + 'c.close()',
    PREP + 'k.execute("DELETE FROM u WHERE id = 1")' + NL + 'post(k.rowcount)' + NL + 'c.close()',
    # lastrowid não muda depois de SELECT
    IMP + 'c = sqlite3.connect("t.db")' + NL + 'k = c.cursor()' + NL
        + 'k.execute("CREATE TABLE q (a INTEGER PRIMARY KEY)")' + NL
        + 'k.execute("INSERT INTO q VALUES (10)")' + NL + 'post(k.lastrowid)' + NL
        + 'k.execute("SELECT * FROM q")' + NL + 'post(k.lastrowid)' + NL + 'c.close()',
])
def test_rowcount_e_lastrowid(src, mesmo):
    """São CAMPOS (sem parêntese). SELECT deixa rowcount em -1."""
    mesmo(src)


def test_executemany_acumula_rowcount(mesmo):
    mesmo(IMP + 'c = sqlite3.connect("t.db")' + NL + 'k = c.cursor()' + NL
          + 'k.execute("CREATE TABLE m (a INTEGER, b TEXT)")' + NL
          + 'k.executemany("INSERT INTO m VALUES (?, ?)", [(1, "x"), (2, "y")])' + NL
          + 'post(k.rowcount)' + NL
          + 'post(k.execute("SELECT * FROM m").fetchall())' + NL + 'c.close()')


# ── transação ───────────────────────────────────────────────────────────────

def test_rollback_desfaz(mesmo):
    """Exige a transação implícita antes do DML — a sqlite crua fica em
    autocommit e o INSERT já teria sido gravado."""
    mesmo(IMP + 'c = sqlite3.connect("t.db")' + NL + 'k = c.cursor()' + NL
          + 'k.execute("CREATE TABLE r (a INTEGER)")' + NL + 'c.commit()' + NL
          + 'k.execute("INSERT INTO r VALUES (1)")' + NL + 'c.rollback()' + NL
          + 'post(k.execute("SELECT * FROM r").fetchall())' + NL + 'c.close()')


def test_using_comita_ao_sair(mesmo):
    mesmo(IMP + 'using sqlite3.connect("u.db") as c {' + NL
          + ' c.execute("CREATE TABLE t (a INTEGER)")' + NL
          + ' c.execute("INSERT INTO t VALUES (5)")' + NL + '}' + NL
          + 'c2 = sqlite3.connect("u.db")' + NL
          + 'post(c2.execute("SELECT * FROM t").fetchall())' + NL + 'c2.close()')


def test_persistencia_entre_conexoes(mesmo):
    mesmo(IMP + 'c = sqlite3.connect("per.db")' + NL
          + 'c.execute("CREATE TABLE z (a INTEGER)")' + NL
          + 'c.execute("INSERT INTO z VALUES (3)")' + NL + 'c.commit()' + NL + 'c.close()' + NL
          + 'c2 = sqlite3.connect("per.db")' + NL
          + 'post(c2.execute("SELECT * FROM z").fetchall())' + NL + 'c2.close()')


# ── tipos ───────────────────────────────────────────────────────────────────

@pytest.mark.parametrize("src", [
    # Null e bool: bool grava como 1/0, Null volta como null
    IMP + 'c = sqlite3.connect("t.db")' + NL + 'k = c.cursor()' + NL
        + 'k.execute("CREATE TABLE n (a, b)")' + NL
        + 'k.execute("INSERT INTO n VALUES (?, ?)", (Null, true))' + NL
        + 'post(k.execute("SELECT * FROM n").fetchall())' + NL + 'c.close()',
    # blob faz a volta como bytes
    IMP + 'c = sqlite3.connect("t.db")' + NL + 'k = c.cursor()' + NL
        + 'k.execute("CREATE TABLE b (d BLOB)")' + NL
        + 'k.execute("INSERT INTO b VALUES (?)", ("ab".encode(),))' + NL
        + 'post(k.execute("SELECT d FROM b").fetchone())' + NL + 'c.close()',
    # parâmetro em lista, não só tupla
    IMP + 'c = sqlite3.connect("t.db")' + NL + 'k = c.cursor()' + NL
        + 'k.execute("CREATE TABLE p (a INTEGER)")' + NL
        + 'k.execute("INSERT INTO p VALUES (?)", [7])' + NL
        + 'post(k.execute("SELECT * FROM p").fetchall())' + NL + 'c.close()',
])
def test_tipos(src, mesmo):
    mesmo(src)


# ── erros ───────────────────────────────────────────────────────────────────

@pytest.mark.parametrize("src", [
    IMP + 'c = sqlite3.connect("t.db")' + NL + 'k = c.cursor()' + NL
        + 'try {' + NL + ' k.execute("SELECT * FROM naoexiste")' + NL
        + '} catch (DatabaseError e) {' + NL + ' post("DatabaseError: " + e)' + NL + '}' + NL
        + 'c.close()',
    IMP + 'c = sqlite3.connect("t.db")' + NL + 'c.close()' + NL
        + 'try {' + NL + ' c.execute("SELECT 1")' + NL
        + '} catch (e) {' + NL + ' post("pego: " + e)' + NL + '}',
    IMP + 'c = sqlite3.connect("t.db")' + NL + 'k = c.cursor()' + NL
        + 'k.execute("CREATE TABLE w (a INTEGER)")' + NL
        + 'try {' + NL + ' k.execute("INSERT INTO w VALUES (?)", (1, 2))' + NL
        + '} catch (DatabaseError e) {' + NL + ' post("params errados")' + NL + '}' + NL
        + 'c.close()',
])
def test_erro_e_databaseerror(src, mesmo):
    mesmo(src)
