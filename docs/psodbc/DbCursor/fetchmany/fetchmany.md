# `DbCursor.fetchmany(size=1)`

Devolve até **N** linhas do resultado (uma lista de dicts). Útil pra processar
resultados grandes em blocos, sem carregar tudo na memória.

```assinatura
cursor.fetchmany(size: int = 1) -> list
```

Os exemplos desta página pressupõem:

```ps contexto
import psodbc
# a tabela dos exemplos, com algumas linhas
psodbc.query(base="loja.db", cmd="CREATE TABLE muitos_registros (id INTEGER PRIMARY KEY, valor INT)")
psodbc.query(base="loja.db", cmd="INSERT INTO muitos_registros (valor) VALUES (10), (20), (30), (40), (50), (60), (70), (80), (90), (100), (110), (120)")
DbConnection conn = psodbc.connect(driver="sqlite", base="loja.db")
DbCursor cursor = conn.cursor()
```

---

## Uso

```ps
cursor.execute("SELECT * FROM muitos_registros")

# processa de 100 em 100
while (True) {
    bloco = cursor.fetchmany(100)
    if (len(bloco) == 0) {
        break
    }
    for each linha in bloco {
        # processa cada uma
    }
}
```

---

## Relacionados

- [`.fetchall()`](../fetchall/fetchall.md) — tudo de uma vez
- [`.fetchone()`](../fetchone/fetchone.md) — uma por vez
