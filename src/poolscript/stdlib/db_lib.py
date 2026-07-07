"""
Módulo `db` da PoolScript — SQLite, PostgreSQL, MySQL e MongoDB.

# SQLite (modo convencional)
conn = db.query("meu_db.db")
cursor = conn.cursor()
cursor.execute("SELECT * FROM users")
result = getAll()
conn.close()

# SQLite (modo curto)
result = db.query(base="meu_db.db", cmd="SELECT * FROM @t", table="users")

# PostgreSQL / MySQL
conn = db.connect(
    type="postgres",
    host="localhost",
    port=5432,
    user="admin",
    password="senha",
    database="meu_banco"
)
cursor = conn.cursor()
cursor.execute("SELECT * FROM users")
result = getAll()
conn.close()

# MongoDB
conn = db.connect(type="mongo", host="localhost", port=27017, database="meu_banco")
col = conn.collection("users")
result = col.find({"nome": "ana"})
col.insert({"nome": "leo", "email": "leo@email.com"})
"""
from __future__ import annotations
import sqlite3
from typing import Any

_last_cursor = None


# ── Cursor universal ───────────────────────────────────────────────────

class DbCursor:
    def __init__(self, cursor, db_type: str = "sqlite"):
        self._cursor = cursor
        self._db_type = db_type

    def execute(self, sql: str, params: tuple = ()):
        global _last_cursor
        try:
            # MySQL usa %s em vez de ? como placeholder
            if self._db_type == "mysql":
                sql = sql.replace("?", "%s")
            self._cursor.execute(sql, params)
            _last_cursor = self._cursor
        except Exception as e:
            return str(e)
        return self

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
        try:
            self._col.insert_one(document)
            return None
        except Exception as e:
            return str(e)

    def insert_many(self, documents: list) -> Any:
        """Insere vários documentos."""
        try:
            self._col.insert_many(documents)
            return None
        except Exception as e:
            return str(e)

    def update(self, query: dict, new_values: dict) -> Any:
        """Atualiza documentos que batem com a query."""
        try:
            self._col.update_many(query, {"$set": new_values})
            return None
        except Exception as e:
            return str(e)

    def remove(self, query: dict) -> Any:
        """Remove documentos que batem com a query."""
        try:
            self._col.delete_many(query)
            return None
        except Exception as e:
            return str(e)

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


# ── getAll() global ────────────────────────────────────────────────────

def getAll() -> Any:
    """Retorna resultado do último cursor.execute()."""
    global _last_cursor
    if _last_cursor is None:
        return None
    try:
        rows = _last_cursor.fetchall()
        if not rows:
            return None
        cols = [d[0] for d in _last_cursor.description]
        return [dict(zip(cols, row)) for row in rows]
    except Exception:
        return None


# ── connect() — PostgreSQL, MySQL, MongoDB ─────────────────────────────

def connect(driver: str = "sqlite", host: str = "localhost", port: int = 0,
            user: str = "", password: str = "", database: str = "",
            base: str = "") -> Any:
    """
    Conecta a um banco de dados.

    type="sqlite"   → base="arquivo.db"
    type="postgres" → host, port(5432), user, password, database
    type="mysql"    → host, port(3306), user, password, database
    type="mongo"    → host, port(27017), user, password, database
    """
    db_type = driver.lower()

    if db_type == "sqlite":
        target = base or database
        try:
            conn = sqlite3.connect(target)
            return DbConnection(conn, "sqlite")
        except sqlite3.Error as e:
            return str(e)

    if db_type in ("postgres", "postgresql", "pg"):
        try:
            import psycopg2
        except ImportError:
            return "Error: instale psycopg2 — pip install psycopg2-binary"
        try:
            conn = psycopg2.connect(
                host=host,
                port=port or 5432,
                user=user,
                password=password,
                dbname=database
            )
            return DbConnection(conn, "postgres")
        except Exception as e:
            return str(e)

    if db_type in ("mysql", "mariadb"):
        try:
            import mysql.connector
        except ImportError:
            return "Error: instale mysql-connector-python — pip install mysql-connector-python"
        try:
            conn = mysql.connector.connect(
                host=host,
                port=port or 3306,
                user=user,
                password=password,
                database=database
            )
            return DbConnection(conn, "mysql")
        except Exception as e:
            return str(e)

    if db_type in ("mongo", "mongodb"):
        try:
            import pymongo
        except ImportError:
            return "Error: instale pymongo — pip install pymongo"
        try:
            if user and password:
                uri = f"mongodb://{user}:{password}@{host}:{port or 27017}/{database}"
            else:
                uri = f"mongodb://{host}:{port or 27017}/"
            client = pymongo.MongoClient(uri)
            db_obj = client[database]
            return MongoConnection(client, db_obj)
        except Exception as e:
            return str(e)

    return f"Error: tipo de banco desconhecido: {driver}. Use sqlite, postgres, mysql ou mongo"


# ── query() — SQLite legado ────────────────────────────────────────────

def query(base: str = "", cmd: Any = None, table: str = "") -> Any:
    """SQLite — modo convencional e modo curto."""
    if cmd is None:
        try:
            conn = sqlite3.connect(base)
            return DbConnection(conn, "sqlite")
        except sqlite3.Error as e:
            return str(e)

    try:
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

    except sqlite3.Error as e:
        return str(e)


EXPORTS = {
    "query": query,
    "connect": connect,
    "getAll": getAll,
}
