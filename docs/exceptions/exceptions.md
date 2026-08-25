# Exceptions — erros, `raise` e `catch`

Um erro na PoolScript tem um **tipo** (um nome) e uma **mensagem**. Você pode
deixá-lo **propagar** (para o programa com uma mensagem limpa — `catch` é
opcional) ou **capturar** com `try/catch`.

---

## `raise` — levantar um erro

```
raise "algo deu errado"           // tipo RuntimeError, com essa mensagem
raise NetworkError("a conexão caiu")   // tipo LIVRE: qualquer NomeAssim vira o tipo
raise MinhaFalha                  // só o tipo, sem mensagem
```

Regra: um nome que **começa com maiúscula** vira o **tipo** do erro — você
inventa o nome que quiser (não existe lista fixa). `raise "texto"` (ou uma
variável) manda só a mensagem, e o tipo é `RuntimeError`.

**Sem `catch`**, o erro propaga e para o programa com a mensagem — nunca é um
crash:

```
raise NetworkError("caiu")
// NetworkError: caiu
//   em app.ps, linha 1
```

---

## `catch` — capturar (opcional)

O **tipo** e a **variável do erro** são os dois opcionais:

```
try { ... } catch (NetworkError e) { post("caiu:", e) }   // tipo + variável
try { ... } catch (NetworkError)   { post("caiu") }        // só o tipo, sem var
try { ... } catch (e)              { post("qualquer:", e) } // captura tudo, com var
try { ... } catch ()               { post("qualquer") }     // captura tudo, sem var
```

Vários `catch` em cadeia + `finally` opcional:

```
try {
    conn = psodbc.connect(driver="postgres", ...)
} catch (DatabaseError e) {
    post("banco:", e)
} catch (NetworkError e) {
    post("rede:", e)
} finally {
    post("sempre roda")
}
```

Se nenhum `catch` casa o tipo, o erro **continua propagando** (um `try` de fora
ainda pode pegá-lo) — `catch (KeyError)` não vira um catch-tudo silencioso.

---

## Os tipos que o motor levanta

| tipo | quando acontece |
|---|---|
| `SomeValueUnexpected` | operação inválida de **valor/tipo**: `1/0`, `1%0`, `"a" - 1`, `int("a")`, argumento errado de builtin. É o erro **geral** de valor. |
| `KeyError` | chave inexistente num dict: `d["naoexiste"]` |
| `ImportError` | módulo não encontrado: `import naoexiste` |
| `ConversionError` | falha de conversão (`Parsing`) |
| `NetworkError` | falha de rede/conexão (`request`, http) |
| `DatabaseError` | erro de banco (`psodbc`) |
| `TimeoutError` | tempo esgotado (`request` com `timeout=`) |
| `IOError` / `OSError` | arquivo / sistema (`os`) |
| `MemoryError` | sem memória |
| `RuntimeError` | variável não definida; `raise "texto"`; erro genérico de runtime |
| *(o seu)* | qualquer nome que você levantar com `raise Nome("msg")` |

> **Nota — índice fora da lista:** `lista[999]` **não** levanta erro; devolve
> `null` e emite um `IndexOutOfBoundsWarning` no terminal (é aviso, não exceção).

---

## Exemplo completo

```
import request

action buscar(url=str) {
    try {
        r = request.get(url=url, timeout=5)
        return r.json()
    } catch (TimeoutError) {
        post("demorou demais")
        return null
    } catch (NetworkError e) {
        post("sem rede:", e)
        return null
    }
}
```
