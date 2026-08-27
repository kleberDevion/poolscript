# `regex.escape(pattern)`

Escapa os caracteres especiais de regex num texto, pra ele ser usado como
**texto literal** dentro de um padrão. Devolve a versão escapada.

```
regex.escape(string: str) -> str
```

---

## O problema que resolve

Caracteres como `.`, `*`, `+`, `(`, `[` têm significado especial em regex. Se
você quer procurar um texto que **contém** esses caracteres literalmente, tem
que escapá-los — senão o regex os interpreta como comandos.

```
import regex

// quero procurar o texto literal "3.14 (pi)"
alvo = "3.14 (pi)"
padrao = regex.escape(alvo)         // "3\.14\ \(pi\)"

if (regex.search(padrao, texto)) {
    post("achou")
}
```

Sem o `escape`, o `.` casaria qualquer caractere e os `()` seriam um grupo — o
resultado seria errado.

---

## Quando usar

Sempre que o padrão vier de uma **variável** (entrada do usuário, um nome de
arquivo, etc.) e você quer casá-lo **literalmente**:

```
termo = request.get("busca")        // pode ter . * + etc.
padrao = regex.escape(termo)
achados = regex.findall(padrao, documento)
```

---

## Relacionados

- [`regex.search()`](../search/search.md) / [`regex.findall()`](../findall/findall.md) — onde usar o padrão escapado
