"""
Módulo `sqlite3` da PoolScript.

Uso direto — sem abstração, igual Python puro:

    import sqlite3

    conn = sqlite3.connect("banco.db")
    cursor = conn.cursor()
    cursor.execute("SELECT * FROM users")
    result = cursor.fetchall()
    post(result)
    conn.close()

    # com parâmetros
    cursor.execute("INSERT INTO users (nome) VALUES (?)", ("Kleber",))
    conn.commit()
    conn.close()
"""
from __future__ import annotations
import sqlite3 as _sqlite3


class PoolCursor:
    """Wrapper do cursor SQLite — expõe métodos como atributos."""

    def __init__(self, cursor: _sqlite3.Cursor):
        self._cursor = cursor

    def execute(self, sql: str, params=None):
        if params is not None:
            self._cursor.execute(sql, params)
        else:
            self._cursor.execute(sql)
        # DDL statements (CREATE, DROP, ALTER, PRAGMA etc.) não têm resultset
        # — retorna None em vez de self para não confundir o usuário.
        _sql_upper = sql.lstrip().upper()
        _is_ddl = any(_sql_upper.startswith(kw) for kw in (
            "CREATE", "DROP", "ALTER", "PRAGMA", "ATTACH", "DETACH", "VACUUM",
        ))
        return None if _is_ddl else self

    def executemany(self, sql: str, seq):
        self._cursor.executemany(sql, seq)
        return self

    def fetchall(self) -> list:
        rows = self._cursor.fetchall()
        # converte cada row em dict se tiver description
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

    @property
    def lastrowid(self):
        return self._cursor.lastrowid

    def close(self):
        self._cursor.close()

    def __repr__(self):
        return "<sqlite3.Cursor>"


class PoolConnection:
    """Wrapper da conexão SQLite — expõe métodos como atributos."""

    def __init__(self, conn: _sqlite3.Connection):
        self._conn = conn
        # suporte a acesso por ponto nos resultados
        self._conn.row_factory = None

    def cursor(self) -> PoolCursor:
        return PoolCursor(self._conn.cursor())

    def execute(self, sql: str, params=None) -> PoolCursor:
        """Atalho: conn.execute() sem precisar criar cursor."""
        cur = self._conn.cursor()
        if params is not None:
            cur.execute(sql, params)
        else:
            cur.execute(sql)
        return PoolCursor(cur)

    def commit(self):
        self._conn.commit()
        return self

    def rollback(self):
        self._conn.rollback()
        return self

    def close(self):
        self._conn.close()

    def __enter__(self):
        return self

    def __exit__(self, *_):
        self._conn.commit()
        self._conn.close()

    def __repr__(self):
        return "<sqlite3.Connection>"


def connect(database: str, **kwargs) -> PoolConnection:
    """Abre conexão com banco SQLite. Cria o arquivo se não existir."""
    conn = _sqlite3.connect(database, **kwargs)
    return PoolConnection(conn)


EXPORTS = {
    "connect": connect,
}
