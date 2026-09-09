# `JinkerResponse.cookie(nome, valor, path="/", max_age=Null, httponly=true, secure=false, samesite="Lax", domain=Null)`

Acrescenta um `Set-Cookie` à resposta. Devolve o próprio `JinkerResponse`
(encadeável).

```
cookie(nome: str, valor: str, path: str = "/", max_age: int = Null,
       httponly: bool = true, secure: bool = false,
       samesite: str = "Lax", domain: str = Null) -> JinkerResponse
```

## Parâmetros

São **oito**, nesta ordem, e os nomes são exatamente estes — os dois primeiros
em português, como o motor os declara:

| # | nome | tipo | default | o que faz |
|---|---|---|---|---|
| 1 | `nome` | `str` | — | o nome do cookie; **obrigatório** |
| 2 | `valor` | `str` | — | o conteúdo do cookie; **obrigatório** |
| 3 | `path` | `str` | `"/"` | vira `Path=`; caminho em que o navegador devolve o cookie |
| 4 | `max_age` | `int` | `Null` | vira `Max-Age=`, em **segundos**. Sem ele, o cookie morre quando o navegador fecha |
| 5 | `httponly` | `bool` | `true` | vira `HttpOnly`; o cookie fica invisível pro código da página |
| 6 | `secure` | `bool` | `false` | vira `Secure`; o cookie só viaja em HTTPS |
| 7 | `samesite` | `str` | `"Lax"` | vira `SameSite=`; `"Strict"`, `"Lax"` ou `"None"` |
| 8 | `domain` | `str` | `Null` | vira `Domain=`; sem ele, vale só pro host que respondeu |

Não existe nenhum outro nome. Em especial **não existe `token`**, `name`,
`value`, `expires`, `age` nem `same_site`: o token é o `valor`, e qualquer
outro nome é
`TypeError: 'token' is an invalid keyword argument for cookie()`.

**A ordem só importa em argumento posicional.** Nomeado vai em qualquer ordem:
`cookie("a", "b", samesite="Strict", path="/x", max_age=10)` é idêntico a
`cookie("a", "b", path="/x", max_age=10, samesite="Strict")`. `nome` e `valor`
também aceitam nome (`cookie(nome="sid", valor=t)`), e aí a chamada inteira
pode ser nomeada.

Menos de 2 ou mais de 8 argumentos é `TypeError`:
`cookie() takes at most 8 arguments (9 given)`.

## Retorno

**`JinkerResponse`** — **o próprio objeto**, não um cookie nem um cabeçalho.
`resp.cookie(...) == resp` é `true` e `type(...)` devolve `"JinkerResponse"`.

Por isso encadeia, e cada chamada **acrescenta** um cookie em vez de
substituir o anterior:

```ps
return JinkerResponse()
       .json({"ok": true})
       .cookie("sid", token, max_age=3600)
       .cookie("tema", "escuro", httponly=false)
```

Sai na resposta, um cabeçalho por cookie:

```
Set-Cookie: sid=<token>; Path=/; Max-Age=3600; SameSite=Lax; HttpOnly
Set-Cookie: tema=escuro; Path=/; SameSite=Lax
```

> `Set-Cookie` é o único cabeçalho de resposta que **se repete**. Por isso ele
> não passa pelo [`.header()`](../header/header.md), que é um dicionário — lá
> o segundo cookie sobrescreveria o primeiro e sumiria sem erro.

## O formato da linha gerada

Os atributos saem sempre nesta ordem, independente da ordem em que você os
passou, e o que estiver no default **não é escrito**:

```
<nome>=<valor>[; Path=…][; Max-Age=…][; Domain=…][; SameSite=…][; HttpOnly][; Secure]
```

- `Path=` sai quando `path` não é string vazia;
- `Max-Age=` sai quando `max_age` é um `int` **maior ou igual a zero**;
- `Domain=` sai quando `domain` é uma string não vazia;
- `SameSite=` sai quando `samesite` não é string vazia — **o valor vai como
  veio**, o motor não confere: `samesite="Banana"` gera
  `SameSite=Banana` e quem recusa é o navegador;
- `HttpOnly` e `Secure` são bandeiras: aparecem, ou não aparecem.

Argumento opcional com o tipo errado é **ignorado**, sem erro, e o default
vale: `max_age="10"` (string) não gera `Max-Age` nenhum, e `path=7` gera
`Path=/`.

## O default vale mesmo quando você nomeia outro parâmetro

Nomear um parâmetro não mexe nos que você não citou: o `httponly=true` e o
`samesite="Lax"` continuam valendo. Medido no servidor, um `Set-Cookie` por
linha:

```ps
funct exemplos(resp)
{
    resp.cookie("so_nome", "1")
    # Set-Cookie: so_nome=1; Path=/; SameSite=Lax; HttpOnly
    resp.cookie("so_path", "1", path="/p")
    # Set-Cookie: so_path=1; Path=/p; SameSite=Lax; HttpOnly
    resp.cookie("so_maxage", "1", max_age=5)
    # Set-Cookie: so_maxage=1; Path=/; Max-Age=5; SameSite=Lax; HttpOnly
    resp.cookie("so_secure", "1", secure=true)
    # Set-Cookie: so_secure=1; Path=/; SameSite=Lax; HttpOnly; Secure
    resp.cookie("so_samesite", "1", samesite="Strict")
    # Set-Cookie: so_samesite=1; Path=/; SameSite=Strict; HttpOnly
    resp.cookie("so_domain", "1", domain="e.com")
    # Set-Cookie: so_domain=1; Path=/; Domain=e.com; SameSite=Lax; HttpOnly
    return resp
}
```

> **Isto foi um bug até a versão 15.90.12.** Nomear `secure`, `samesite` ou
> `domain` apagava o `HttpOnly` em silêncio: o motor não distinguia "não
> passou" de "passou falso", e o default seguro virava permissivo. Se você
> escreveu `httponly=true` na mão por causa disso, pode tirar — o default já
> entrega. Ficar não muda nada.

Para **desligar** o `HttpOnly` é preciso dizer:

```ps
funct sem_httponly(resp)
{
    resp.cookie("tema", "escuro", httponly=false)
    # Set-Cookie: tema=escuro; Path=/; SameSite=Lax
    return resp
}
```

A chamada toda posicional segue a ordem da tabela acima:

```ps
funct posicional(resp)
{
    resp.cookie("p", "1", "/", 5, true, false, "Strict", "e.com")
    # Set-Cookie: p=1; Path=/; Max-Age=5; Domain=e.com; SameSite=Strict; HttpOnly
    return resp
}
```

## Apagar um cookie

Não há `delete`: apaga-se sobrescrevendo com vida zero, no **mesmo** `path`
usado ao gravar — cookie é identificado por `(nome, domínio, path)`.

```ps
return JinkerResponse().json({"ok": true}).cookie("sid", "", max_age=0)
# Set-Cookie: sid=; Path=/; Max-Age=0; SameSite=Lax; HttpOnly
```

## Erros

- **TypeError** — `cookie("sid", 123)`: `cookie() espera nome e valor em str`.
  `nome` e `valor` são os dois únicos com tipo cobrado; converta com `str(...)`.
- **TypeError** — nome de argumento que não existe, ou mais de 8 argumentos.
- **ValueError** — `\r`, `\n` ou qualquer byte de controle no `nome`, no
  `valor`, no `path`, no `samesite` ou no `domain`:
  `cookie() nao aceita quebra de linha...`. Quebra de linha num cookie fecharia
  a linha do cabeçalho e escreveria outra — é injeção de cabeçalho, e recusar é
  a única resposta certa.
- **ValueError** — a linha montada passou de **1023 bytes**:
  `cookie() ficou grande demais (limite 1023 bytes)`. Ela é recusada, não
  truncada.

## Relacionados

- [`request.cookie(nome)`](../../cookie/cookie.md#ler) — a outra metade: ler o
  cookie que chegou (devolve `str` ou `Null`)
- [cookie e sessão assinada](../../cookie/cookie.md) — login, rota protegida e
  logout com [`jwt`](../../../jwt/jwt.md)
- [`.header(key, value)`](../header/header.md) — cabeçalho de resposta que
  **não** se repete
- [`JinkerResponse`](../JinkerResponse.md) — os outros métodos encadeáveis
