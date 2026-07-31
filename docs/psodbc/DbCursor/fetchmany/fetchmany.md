# `DbCursor.fetchmany(size=1)`

Devolve até **N** linhas do resultado (uma lista de dicts). Útil pra processar
resultados grandes em blocos, sem carregar tudo na memória.

```
cursor.fetchmany(size: int = 1) -> list
```

---

## Uso

```
cursor.execute("SELECT * FROM muitos_registros")

// processa de 100 em 100
while (true) {
    bloco = cursor.fetchmany(100)
    if (len(bloco) == 0) {
        break
    }
    for each linha in bloco {
        // processa cada uma
    }
}
```

---

## Relacionados

- [`.fetchall()`](../fetchall/fetchall.md) — tudo de uma vez
- [`.fetchone()`](../fetchone/fetchone.md) — uma por vez
