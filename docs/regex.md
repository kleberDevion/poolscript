# regex — Expressões Regulares

Motor de expressões regulares do próprio `pool` (`vm/ps_regex.c`).

```
from regex import match, search, findall, sub, split, escape
```

---

## match(pattern, string)

Casa a string **inteira** com o padrão. Retorna `bool`.

```
match("\d+", "abc123")   # False — tem letras
match("\d+", "123")      # True
```

---

## search(pattern, string)

Verifica se o padrão existe em qualquer posição da string. Retorna `bool`.

```
search("\d+", "abc123")  # True
```

---

## findall(pattern, string)

Retorna lista com todas as ocorrências do padrão.

```
findall("\d+", "a1b2c3")   # ["1", "2", "3"]
```

---

## sub(pattern, repl, string, count=0)

Substitui ocorrências do padrão. Retorna string (encadeável, tem os
métodos estendidos de string da PoolScript).

```
sub("\s+", "_", "hello world")   # "hello_world"
```

---

## split(pattern, string, maxsplit=0)

Divide a string pelo padrão.

```
split("\s+", "a b  c")   # ["a", "b", "c"]
```

---

## escape(string)

Escapa caracteres especiais de regex.

```
escape("a.b*c")   # "a\.b\*c"
```

---

## Exemplo — validação simples

```
from regex import match

funct email_valido(email) {
    return match(r"[^@\s]+@[^@\s]+\.[^@\s]+", email)
}

post(email_valido("ana@email.com"))   # True
post(email_valido("invalido"))        # False
```
