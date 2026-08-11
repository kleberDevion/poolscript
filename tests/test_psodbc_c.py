"""Módulo `psodbc`/`db` em C — sqlite e postgres (ps_db.c, libs C).

sqlite roda sempre (offline). postgres precisa de um servidor: a fixture sobe
um cluster temporário com o `pg_ctl`/`initdb` do sistema e derruba no fim;
se os binários não existirem, os testes de postgres são pulados.

Byte a byte não se aplica (é banco), então a comparação é a saída do `.ps`
contra o interpretador (psycopg2/sqlite3) — mesmos valores, mesmos tipos, e o
mesmo nome de classe de erro (UndefinedTable etc., que o `catch (Tipo e)` usa).
"""
import io
import os
import shutil
import socket
import subprocess
import sys
import time
from contextlib import redirect_stdout

import pytest

from poolscript.interpreter import Interpreter
from poolscript.parser import parse_source

vm = pytest.importorskip(
    "poolscript.vm.poolscript_vm",
    reason="extensão C não compilada — rode: ./rebuild_vm.sh",
)

NL = chr(10)


def _interp(src, cwd):
    velho = os.getcwd(); os.chdir(cwd)
    try:
        buf = io.StringIO()
        i = Interpreter(source=src, filename=str(cwd / "t.ps"))
        with redirect_stdout(buf):
            i.run(parse_source(src, "<t>"))
        return ("OK", buf.getvalue())
    except Exception:
        return ("ERR", "")
    finally:
        os.chdir(velho)


def _c(src, cwd):
    velho = os.getcwd(); os.chdir(cwd)
    sys.stdout.flush()
    r, w = os.pipe(); orig = os.dup(1)
    try:
        os.dup2(w, 1); os.close(w)
        try:
            vm.executa_fonte(src, str(cwd / "t.ps"))
            st = "OK"
        except Exception:
            st = "ERR"
        finally:
            sys.stdout.flush(); os.dup2(orig, 1)
    finally:
        os.close(orig); os.chdir(velho)
    with os.fdopen(r, "rb") as f:
        out = f.read().decode("utf-8", "replace")
    return (st, out) if st == "OK" else ("ERR", "")


@pytest.fixture
def mesmo(tmp_path):
    def _m(src):
        a = tmp_path / "ia"; b = tmp_path / "vb"
        for d in (a, b):
            if not d.exists():
                d.mkdir()
        assert _c(src, b) == _interp(src, a)
    return _m


IMP = "import db" + NL


# ── sqlite (sempre) ──────────────────────────────────────────────────────────

def test_sqlite_fluxo(mesmo):
    mesmo(IMP + 'c = db.connect(driver="sqlite", base="t.db")' + NL + 'k = c.cursor()' + NL
          + 'k.execute("CREATE TABLE u (id INTEGER PRIMARY KEY, nome TEXT, nota REAL)")' + NL
          + 'k.execute("INSERT INTO u (nome,nota) VALUES (?, ?)", ("ana", 9.5))' + NL
          + 'k.execute("INSERT INTO u (nome,nota) VALUES (?, ?)", ("bo", 7))' + NL
          + 'post(k.execute("SELECT * FROM u ORDER BY id").fetchall())' + NL + 'c.close()')


@pytest.mark.parametrize("frag", [
    'post(k.execute("SELECT nome FROM u").fetchone())',
    'post(k.execute("SELECT * FROM u WHERE id=99").fetchone())',
    'post(k.execute("SELECT * FROM u").fetchmany(1))',
])
def test_sqlite_fetch(frag, mesmo):
    mesmo(IMP + 'c = db.connect(driver="sqlite", base="t.db")' + NL + 'k = c.cursor()' + NL
          + 'k.execute("CREATE TABLE u (id INTEGER PRIMARY KEY, nome TEXT)")' + NL
          + 'k.execute("INSERT INTO u (nome) VALUES (?)", ("x",))' + NL
          + frag + NL + 'c.close()')


@pytest.mark.parametrize("frag", [
    # rowcount é @property (sem parêntese): DML devolve linhas afetadas,
    # SELECT no sqlite devolve -1 (mesma regra do DBAPI que o interp usa)
    'k.execute("INSERT INTO u (nome) VALUES (\'a\'), (\'b\'), (\'c\')")' + NL + 'post(k.rowcount)',
    'k.execute("SELECT * FROM u")' + NL + 'post(k.rowcount)',
    'k.execute("UPDATE u SET nome=\'z\'")' + NL + 'post(k.rowcount)',
    'k.execute("DELETE FROM u")' + NL + 'post(k.rowcount)',
])
def test_sqlite_rowcount(frag, mesmo):
    """`cursor.rowcount` — era gap de paridade: existia no interp, faltava no
    VM (o usuário bateu nisso com `mouse.rowcount`)."""
    mesmo(IMP + 'c = db.connect(driver="sqlite", base="t.db")' + NL + 'k = c.cursor()' + NL
          + 'k.execute("CREATE TABLE u (id INTEGER PRIMARY KEY, nome TEXT)")' + NL
          + 'k.execute("INSERT INTO u (nome) VALUES (\'x\')")' + NL
          + frag + NL + 'c.close()')


def test_sqlite_tipo_e_url(mesmo):
    mesmo(IMP + 'c = db.connect(url="sqlite:///s.db")' + NL + 'post(type(c))' + NL + 'c.close()')


def test_param_solto_erra_claro(tmp_path):
    # `(x)` sem vírgula é um valor solto, não uma sequência. Antes a VM
    # descartava em silêncio e o `%s`/`?` vazava cru pro banco; agora erra
    # claro. (o `(x,)` COM vírgula é tupla e funciona — coberto acima.)
    src = (IMP + 'c = db.connect(driver="sqlite", base="t.db")' + NL
           + 'k = c.cursor()' + NL
           + 'k.execute("CREATE TABLE u (email TEXT)")' + NL
           + 'k.execute("SELECT * FROM u WHERE email = ?", ("a@x.com"))' + NL)
    st, _ = _c(src, tmp_path)
    assert st == "ERR"


def test_query_curto_e_longo(mesmo):
    mesmo(IMP + 'c = db.connect(driver="sqlite", base="q.db")' + NL
          + 'c.cursor().execute("CREATE TABLE a (x INTEGER)")' + NL + 'c.close()' + NL
          + 'db.query(base="q.db", cmd="INSERT INTO a VALUES (5)")' + NL
          + 'post(db.query(base="q.db", cmd="SELECT * FROM @t", table="a"))')


def test_query_select_vazio_e_null(mesmo):
    mesmo(IMP + 'post(db.query(base="v.db", cmd="SELECT 1 AS n WHERE 1=0"))')


def test_driver_desconhecido_erra(mesmo):
    mesmo(IMP + 'try { db.connect(driver="xyz") } catch (e) { post("driver ruim") }')


# ── postgres (fixture com cluster temporário) ────────────────────────────────

def _acha_pgbin():
    for base in ("/usr/lib/postgresql",):
        if not os.path.isdir(base):
            continue
        for v in sorted(os.listdir(base), reverse=True):
            b = os.path.join(base, v, "bin")
            if os.path.isfile(os.path.join(b, "initdb")):
                return b
    return None


@pytest.fixture(scope="module")
def pg(tmp_path_factory):
    pgbin = _acha_pgbin()
    if not pgbin:
        pytest.skip("PostgreSQL (initdb) não instalado")
    data = tmp_path_factory.mktemp("pgdata")
    datadir = str(data / "d")
    subprocess.run([os.path.join(pgbin, "initdb"), "-D", datadir, "-U", "pooluser",
                    "--auth=trust"], capture_output=True)
    s = socket.socket(); s.bind(("127.0.0.1", 0)); porta = s.getsockname()[1]; s.close()
    opts = "-p %d -c listen_addresses=127.0.0.1 -c unix_socket_directories=/tmp" % porta
    proc = subprocess.run([os.path.join(pgbin, "pg_ctl"), "-D", datadir, "-o", opts,
                           "-l", os.path.join(datadir, "log"), "start"],
                          capture_output=True, text=True)
    if proc.returncode != 0:
        pytest.skip("não consegui subir o postgres de teste")
    # espera aceitar conexão
    for _ in range(50):
        try:
            with socket.create_connection(("127.0.0.1", porta), timeout=0.2):
                break
        except OSError:
            time.sleep(0.1)
    psql = os.path.join(pgbin, "psql")
    subprocess.run([psql, "-h", "/tmp", "-p", str(porta), "-U", "pooluser", "-d", "postgres",
                    "-c", "CREATE DATABASE testdb;"], capture_output=True)
    subprocess.run([psql, "-h", "/tmp", "-p", str(porta), "-U", "pooluser", "-d", "testdb",
                    "-c", "CREATE TABLE u (id serial primary key, nome text, nota real);"
                          "INSERT INTO u (nome,nota) VALUES ('ana',9.5),('bo',7);"],
                   capture_output=True)
    yield porta
    subprocess.run([os.path.join(pgbin, "pg_ctl"), "-D", datadir, "stop", "-m", "immediate"],
                   capture_output=True)


def _CONN(porta):
    return ('db.connect(driver="postgres", host="127.0.0.1", port=%d, '
            'user="pooluser", database="testdb")' % porta)


def test_pg_select(pg, mesmo):
    mesmo(IMP + 'c = ' + _CONN(pg) + NL + 'k = c.cursor()' + NL
          + 'post(k.execute("SELECT * FROM u ORDER BY id").fetchall())' + NL + 'c.close()')


def test_pg_tipos(pg, mesmo):
    """int/float/str/null voltam tipados, como o psycopg2."""
    mesmo(IMP + 'c = ' + _CONN(pg) + NL + 'k = c.cursor()' + NL
          + 'd = k.execute("SELECT * FROM u ORDER BY id").fetchall()' + NL
          + 'post(d[0]["id"], d[0]["nota"], d[0]["nome"])' + NL + 'c.close()')


def test_pg_param_percent_s(pg, mesmo):
    """postgres usa %s (estilo psycopg2), não ?."""
    mesmo(IMP + 'c = ' + _CONN(pg) + NL + 'k = c.cursor()' + NL
          + 'post(k.execute("SELECT nome FROM u WHERE nota > %s", (8,)).fetchall())' + NL
          + 'c.close()')


def test_pg_url(pg, mesmo):
    mesmo(IMP + 'c = db.connect(url="postgres://pooluser@127.0.0.1:%d/testdb")' % pg + NL
          + 'post(c.cursor().execute("SELECT nota FROM u ORDER BY id").fetchall())' + NL
          + 'c.close()')


@pytest.mark.parametrize("frag,tipo", [
    ('SELECT * FROM naoexiste', 'UndefinedTable'),
    ('SELECT xyz FROM u', 'UndefinedColumn'),
])
def test_pg_erro_classe_exata(pg, frag, tipo, mesmo):
    """O erro do postgres vem com o nome de classe da psycopg2 (via SQLSTATE),
    então `catch (UndefinedTable e)` pega igual nos dois motores."""
    mesmo(IMP + 'c = ' + _CONN(pg) + NL
          + 'try { c.cursor().execute("' + frag + '") } catch (' + tipo + ' e) { post("pego") }'
          + NL + 'c.close()')


# ── mysql/mariadb (fixture com instância temporária) ─────────────────────────

def _acha(bins):
    for b in bins:
        pth = shutil.which(b) or ("/usr/sbin/" + b if os.path.exists("/usr/sbin/" + b) else None)
        if pth:
            return pth
    return None


@pytest.fixture(scope="module")
def mysql(tmp_path_factory):
    mariadbd = _acha(["mariadbd", "mysqld"])
    instala = _acha(["mariadb-install-db", "mysql_install_db"])
    cliente = _acha(["mariadb", "mysql"])
    if not (mariadbd and instala and cliente):
        pytest.skip("MariaDB/MySQL não instalado")
    data = "/tmp/psmy_t"
    sock = "/tmp/psmy_t.sock"
    shutil.rmtree(data, ignore_errors=True)
    if os.path.exists(sock):
        os.remove(sock)
    os.makedirs(data)
    subprocess.run([instala, "--no-defaults", "--datadir=" + data,
                    "--auth-root-authentication-method=normal"], capture_output=True)
    s = socket.socket(); s.bind(("127.0.0.1", 0)); porta = s.getsockname()[1]; s.close()
    proc = subprocess.Popen([mariadbd, "--no-defaults", "--datadir=" + data,
                             "--socket=" + sock, "--port=%d" % porta, "--bind-address=127.0.0.1"],
                            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    for _ in range(60):
        try:
            with socket.create_connection(("127.0.0.1", porta), timeout=0.2):
                break
        except OSError:
            time.sleep(0.2)
    else:
        proc.terminate(); pytest.skip("mariadb não subiu")
    subprocess.run([cliente, "--socket=" + sock, "-u", "root", "-e",
                    "CREATE DATABASE testdb; USE testdb;"
                    "CREATE TABLE u (id INT AUTO_INCREMENT PRIMARY KEY, nome VARCHAR(50), nota FLOAT);"
                    "INSERT INTO u (nome,nota) VALUES ('ana',9.5),('bo',7);"
                    "CREATE USER 'pool'@'127.0.0.1' IDENTIFIED BY 'x';"
                    "GRANT ALL ON testdb.* TO 'pool'@'127.0.0.1'; FLUSH PRIVILEGES;"],
                   capture_output=True)
    yield porta
    proc.terminate(); proc.wait(timeout=5)


def _MY(porta):
    return ('db.connect(driver="mysql", host="127.0.0.1", port=%d, '
            'user="pool", password="x", database="testdb")' % porta)


def test_mysql_select(mysql, mesmo):
    mesmo(IMP + 'c = ' + _MY(mysql) + NL + 'k = c.cursor()' + NL
          + 'post(k.execute("SELECT * FROM u ORDER BY id").fetchall())' + NL + 'c.close()')


def test_mysql_tipos_e_param(mysql, mesmo):
    mesmo(IMP + 'c = ' + _MY(mysql) + NL + 'k = c.cursor()' + NL
          + 'd = k.execute("SELECT * FROM u WHERE nota > ?", (8,)).fetchall()' + NL
          + 'post(d[0]["id"], d[0]["nota"], d[0]["nome"])' + NL + 'c.close()')


# ── mongo (fixture com mongod temporário) ────────────────────────────────────

@pytest.fixture(scope="module")
def mongo(tmp_path_factory):
    mongod = _acha(["mongod"])
    if not mongod:
        pytest.skip("mongod não instalado")
    data = str(tmp_path_factory.mktemp("mongo"))
    s = socket.socket(); s.bind(("127.0.0.1", 0)); porta = s.getsockname()[1]; s.close()
    proc = subprocess.Popen([mongod, "--dbpath", data, "--port", str(porta),
                             "--bind_ip", "127.0.0.1", "--nounixsocket"],
                            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    for _ in range(60):
        try:
            with socket.create_connection(("127.0.0.1", porta), timeout=0.2):
                break
        except OSError:
            time.sleep(0.2)
    else:
        proc.terminate(); pytest.skip("mongod não subiu")
    time.sleep(0.5)
    yield porta
    proc.terminate(); proc.wait(timeout=5)


def _semear_mongo(porta):
    pm = pytest.importorskip("pymongo")
    c = pm.MongoClient("mongodb://127.0.0.1:%d/" % porta)
    c["testdb"].users.delete_many({})
    c["testdb"].users.insert_many([{"nome": "ana", "idade": 30}, {"nome": "leo", "idade": 25}])
    c.close()


def _MG(porta):
    return 'db.connect(driver="mongo", host="127.0.0.1", port=%d, database="testdb")' % porta


@pytest.fixture
def mesmo_mongo(mongo, tmp_path):
    def _m(src):
        a = tmp_path / "ia"; b = tmp_path / "vb"
        for d in (a, b):
            if not d.exists():
                d.mkdir()
        _semear_mongo(mongo)
        r_i = _interp(src, a)
        _semear_mongo(mongo)
        r_c = _c(src, b)
        assert r_c == r_i
    return _m


def test_mongo_find(mongo, mesmo_mongo):
    mesmo_mongo(IMP + 'c = ' + _MG(mongo) + NL + 'col = c.collection("users")' + NL
                + 'post(col.find({"nome":"ana"}))' + NL + 'c.close()')


def test_mongo_find_one_e_ausente(mongo, mesmo_mongo):
    mesmo_mongo(IMP + 'c = ' + _MG(mongo) + NL + 'col = c.collection("users")' + NL
                + 'post(col.find_one({"nome":"leo"}))' + NL
                + 'post(col.find_one({"nome":"zzz"}))' + NL + 'c.close()')


def test_mongo_count_insert_update_remove(mongo, mesmo_mongo):
    mesmo_mongo(IMP + 'c = ' + _MG(mongo) + NL + 'col = c.collection("users")' + NL
                + 'post(col.count())' + NL
                + 'col.insert({"nome":"bia","idade":40})' + NL + 'post(col.count())' + NL
                + 'col.update({"nome":"ana"}, {"idade":31})' + NL
                + 'post(col.find_one({"nome":"ana"}))' + NL
                + 'col.remove({"nome":"leo"})' + NL + 'post(col.count())' + NL + 'c.close()')


# ── mssql/odbc ───────────────────────────────────────────────────────────────
# O SQL Server não tem repo pro Ubuntu 24.04 (noble), então não há servidor pra
# um diferencial completo de `driver=mssql`. A MÁQUINA ODBC do ps_db.c
# (SQLConnect/SQLExecDirect/SQLFetch + mapeamento de tipo) foi validada à parte
# contra o driver ODBC do PostgreSQL. Aqui cobre-se o que é portável: conectar
# sem servidor erra de forma capturável nos dois motores.

def test_mssql_sem_servidor_erra(mesmo):
    mesmo(IMP + 'try {' + NL
          + ' db.connect(driver="mssql", host="127.0.0.1", port=1, user="sa", password="x", database="d")' + NL
          + '} catch (e) { post("sem servidor") }')
