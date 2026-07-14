"""
Módulo `psodbc` da PoolScript — SQLite, PostgreSQL, MySQL, SQL Server e MongoDB.

# SQLite (modo convencional)
conn = psodbc.query("meu_db.db")
cursor = conn.cursor()
cursor.execute("SELECT * FROM users")
result = cursor.fetchall()
conn.close()

# SQLite (modo curto)
result = psodbc.query(base="meu_db.db", cmd="SELECT * FROM @t", table="users")

# Por parâmetros
conn = psodbc.connect(
    driver="postgres",
    host="localhost",
    port=5432,
    user="admin",
    password="senha",
    database="meu_banco"
)
cursor = conn.cursor()
cursor.execute("SELECT * FROM users")
result = cursor.fetchall()
conn.close()

# Por URL — funciona com qualquer driver suportado
conn = psodbc.connect(url="sqlserver://user:senha@localhost:1433/meu_banco")
conn = psodbc.connect(url="mssql://user:senha@servidor.local/meu_banco")
conn = psodbc.connect(url="postgres://admin:senha@localhost:5432/meu_banco")
conn = psodbc.connect(url="mysql://admin:senha@localhost:3306/meu_banco")
conn = psodbc.connect(url="mongodb://localhost:27017/meu_banco")
conn = psodbc.connect(url="sqlite:///meu_banco.db")

# SQL Server local, autenticação do Windows (sem user/password)
conn = psodbc.connect(driver="sqlserver", host="localhost\\SQLEXPRESS", database="meu_banco")

# MongoDB
conn = psodbc.connect(driver="mongo", host="localhost", port=27017, database="meu_banco")
col = conn.collection("users")
result = col.find({"nome": "ana"})
col.insert({"nome": "leo", "email": "leo@email.com"})
"""
from __future__ import annotations
import sqlite3
import urllib.parse
from typing import Any

_DRIVER_ALIASES = {
    "sqlite": "sqlite", "sqlite3": "sqlite",
    "postgres": "postgres", "postgresql": "postgres", "pg": "postgres",
    "mysql": "mysql", "mariadb": "mysql",
    "mssql": "mssql", "sqlserver": "mssql", "sql server": "mssql", "mssql+pyodbc": "mssql",
    "mongo": "mongo", "mongodb": "mongo", "mongodb+srv": "mongo",
}


# ── Cursor universal ───────────────────────────────────────────────────

class DbCursor:
    def __init__(self, cursor, db_type: str = "sqlite"):
        self._cursor = cursor
        self._db_type = db_type

    def execute(self, sql: str, params: tuple = ()):
        # MySQL usa %s em vez de ? como placeholder
        if self._db_type == "mysql":
            sql = sql.replace("?", "%s")
        self._cursor.execute(sql, params)
        return self

    def fetchall(self) -> list:
        rows = self._cursor.fetchall()
        if self._cursor.description:
            cols = [d[0] for d in self._cursor.description]
            return [dict(zip(cols, row)) for row in rows]
        return list(rows)

    def fetchone(self):
        row = self._cursor.fetchone()
        if row is None:
            return None
        if self._cursor.description:
            cols = [d[0] for d in self._cursor.description]
            return dict(zip(cols, row))
        return row

    def fetchmany(self, size: int = 1) -> list:
        rows = self._cursor.fetchmany(size)
        if self._cursor.description:
            cols = [d[0] for d in self._cursor.description]
            return [dict(zip(cols, row)) for row in rows]
        return list(rows)

    @property
    def rowcount(self) -> int:
        return self._cursor.rowcount

    def close(self):
        self._cursor.close()

    def __repr__(self):
        return f"<db.cursor [{self._db_type}]>"


# ── Connection universal ───────────────────────────────────────────────

class DbConnection:
    def __init__(self, conn, db_type: str = "sqlite"):
        self._conn = conn
        self._db_type = db_type

    def cursor(self) -> DbCursor:
        return DbCursor(self._conn.cursor(), self._db_type)

    def commit(self):
        self._conn.commit()

    def close(self):
        self._conn.close()

    def __repr__(self):
        return f"<db.connection [{self._db_type}]>"


# ── MongoDB Collection ─────────────────────────────────────────────────

class MongoCollection:
    def __init__(self, col):
        self._col = col

    def find(self, query: dict | None = None) -> list:
        """Busca documentos. Sem query retorna todos."""
        results = list(self._col.find(query or {}))
        for doc in results:
            doc.pop("_id", None)  # remove o _id do mongo pra não confundir
        return results if results else None

    def find_one(self, query: dict | None = None) -> dict | None:
        """Busca um documento."""
        doc = self._col.find_one(query or {})
        if doc:
            doc.pop("_id", None)
        return doc

    def insert(self, document: dict) -> Any:
        """Insere um documento."""
        self._col.insert_one(document)
        return None

    def insert_many(self, documents: list) -> Any:
        """Insere vários documentos."""
        self._col.insert_many(documents)
        return None

    def update(self, query: dict, new_values: dict) -> Any:
        """Atualiza documentos que batem com a query."""
        self._col.update_many(query, {"$set": new_values})
        return None

    def remove(self, query: dict) -> Any:
        """Remove documentos que batem com a query."""
        self._col.delete_many(query)
        return None

    def count(self, query: dict | None = None) -> int:
        return self._col.count_documents(query or {})

    def __repr__(self):
        return f"<db.collection [{self._col.name}]>"


class MongoConnection:
    def __init__(self, client, db):
        self._client = client
        self._db = db

    def collection(self, name: str) -> MongoCollection:
        return MongoCollection(self._db[name])

    def close(self):
        self._client.close()

    def __repr__(self):
        return "<db.connection [mongo]>"


# ── parse de URL de conexão ─────────────────────────────────────────────

def _parse_dsn(dsn: str) -> dict:
    """
    Converte uma URL de conexão (schema://user:pass@host:port/database) num
    dict de parâmetros equivalente aos aceitos por connect().
    """
    parsed = urllib.parse.urlsplit(dsn)
    scheme = parsed.scheme.lower()
    driver = _DRIVER_ALIASES.get(scheme, scheme)

    if driver == "sqlite":
        # sqlite:///caminho/relativo.db  ou  sqlite:////caminho/absoluto.db
        path = parsed.path or parsed.netloc
        return {"driver": "sqlite", "base": path.lstrip("/") or path}

    info = {
        "driver": driver,
        "host": parsed.hostname or "localhost",
        "port": parsed.port or 0,
        "user": urllib.parse.unquote(parsed.username) if parsed.username else "",
        "password": urllib.parse.unquote(parsed.password) if parsed.password else "",
        "database": parsed.path.lstrip("/"),
    }
    if driver == "mongo" and (scheme == "mongodb+srv" or parsed.query):
        info["raw_url"] = dsn  # SRV/opções — repassa a URL original pro pymongo
    return info


def _pick_mssql_odbc_driver(preferred: str = "") -> str:
    """Escolhe o driver ODBC do SQL Server instalado no sistema."""
    if preferred:
        return preferred
    import pyodbc
    installed = pyodbc.drivers()
    for candidate in (
        "ODBC Driver 18 for SQL Server",
        "ODBC Driver 17 for SQL Server",
        "ODBC Driver 13 for SQL Server",
        "SQL Server Native Client 11.0",
        "FreeTDS",
        "SQL Server",
    ):
        if candidate in installed:
            return candidate
    raise RuntimeError(
        "Nenhum driver ODBC de SQL Server encontrado. Instale o 'ODBC Driver 17/18 for SQL Server' "
        "ou informe o nome exato com odbc_driver=\"...\"."
    )


# ── connect() — SQLite, PostgreSQL, MySQL, SQL Server, MongoDB ─────────

def connect(driver: str = "sqlite", host: str = "localhost", port: int = 0,
            user: str = "", password: str = "", database: str = "",
            base: str = "", url: str = "", odbc_driver: str = "") -> Any:
    """
    Conecta a um banco de dados — por parâmetros ou por URL.

    driver="sqlite"           → base="arquivo.db"
    driver="postgres"         → host, port(5432), user, password, database
    driver="mysql"/"mariadb"  → host, port(3306), user, password, database
    driver="mssql"/"sqlserver"→ host, port(1433), user, password, database
                                 (sem user/password usa autenticação do Windows)
    driver="mongo"            → host, port(27017), user, password, database

    url="sqlserver://user:senha@host:1433/banco"  → mesma coisa, por URL.
    Prefixos aceitos: sqlite://, postgres://, mysql://, mssql:// / sqlserver://,
    mongodb:// / mongodb+srv://.
    """
    raw_url = ""
    dsn = url or (driver if "://" in driver else "")
    if dsn:
        info = _parse_dsn(dsn)
        driver = info["driver"]
        host = info.get("host", host)
        port = info.get("port", port)
        user = info.get("user", user)
        password = info.get("password", password)
        database = info.get("database", database)
        base = info.get("base", base)
        raw_url = info.get("raw_url", "")

    db_type = _DRIVER_ALIASES.get(driver.lower(), driver.lower())

    if db_type == "sqlite":
        target = base or database
        conn = sqlite3.connect(target)
        return DbConnection(conn, "sqlite")

    if db_type == "postgres":
        import psycopg2
        conn = psycopg2.connect(
            host=host,
            port=port or 5432,
            user=user,
            password=password,
            dbname=database
        )
        return DbConnection(conn, "postgres")

    if db_type == "mysql":
        import mysql.connector
        conn = mysql.connector.connect(
            host=host,
            port=port or 3306,
            user=user,
            password=password,
            database=database
        )
        return DbConnection(conn, "mysql")

    if db_type == "mssql":
        import pyodbc
        server = f"{host},{port}" if port else host
        parts = [
            f"DRIVER={{{_pick_mssql_odbc_driver(odbc_driver)}}}",
            f"SERVER={server}",
            f"DATABASE={database}",
        ]
        if user and password:
            parts += [f"UID={user}", f"PWD={password}"]
        else:
            parts.append("Trusted_Connection=yes")
        conn = pyodbc.connect(";".join(parts))
        return DbConnection(conn, "mssql")

    if db_type == "mongo":
        import pymongo
        if raw_url:
            uri = raw_url
        elif user and password:
            uri = f"mongodb://{user}:{password}@{host}:{port or 27017}/{database}"
        else:
            uri = f"mongodb://{host}:{port or 27017}/"
        client = pymongo.MongoClient(uri)
        db_obj = client[database] if database else client.get_default_database()
        return MongoConnection(client, db_obj)

    raise ValueError(
        f"tipo de banco desconhecido: {driver}. Use sqlite, postgres, mysql, mssql (sqlserver) ou mongo"
    )


# ── query() — SQLite legado ────────────────────────────────────────────

def query(base: str = "", cmd: Any = None, table: str = "") -> Any:
    """SQLite — modo convencional e modo curto."""
    if cmd is None:
        conn = sqlite3.connect(base)
        return DbConnection(conn, "sqlite")

    conn = sqlite3.connect(base)
    cur = conn.cursor()

    if isinstance(cmd, tuple):
        sql = cmd[0]
        params = cmd[1] if len(cmd) > 1 else ()
    else:
        sql = cmd
        params = ()

    if table:
        sql = sql.replace("@t", table)

    cur.execute(sql, params)

    if sql.strip().upper().startswith("SELECT"):
        rows = cur.fetchall()
        conn.close()
        if not rows:
            return None
        cols = [d[0] for d in cur.description]
        return [dict(zip(cols, row)) for row in rows]

    conn.commit()
    conn.close()
    return None


EXPORTS = {
    "query": query,
    "connect": connect,
}
