# `JinkerResponse.status_code` — o código HTTP como campo

`status_code` é um **campo**, não um método: acessa-se **sem parênteses**, e
pode ser **lido e escrito**. É o mesmo número que
[`.status(code)`](../status/status.md) grava.

```
resp.status_code        -> int
resp.status_code = 201  -> grava (não devolve nada)
```

## Retorno

**`int`** — o código HTTP que a resposta vai carregar. Um `JinkerResponse`
recém-criado nasce em `200`.

```ps
r = JinkerResponse()
post(r.status_code)        # 200
r.status_code = 201
post(r.status_code)        # 201
r.status(404)
post(r.status_code)        # 404  — .status() escreve no mesmo campo
```

Não há parâmetro: `resp.status_code()` é
`'int' object is not callable`, porque o campo já devolveu o número antes do
parêntese.

## Ler

Serve pra decidir com base no que já foi montado, sem precisar guardar o
código numa variável à parte:

```ps
r = jsonify({"itens": lista})
if len(lista) == 0 {
    r.status(204)
}
if r.status_code >= 400 {
    registra_falha(r.status_code)
}
```

## Escrever

`resp.status_code = N` é uma **atribuição**: ela grava o campo e o valor da
expressão é o próprio `N`, não a resposta. Por isso **não encadeia** — quem
encadeia é [`.status(code)`](../status/status.md), que devolve o
`JinkerResponse`.

```ps
# atribuição: uma instrução por vez
r = jsonify({"criado": true})
r.status_code = 201
return r

# método: cabe na cadeia
return jsonify({"criado": true}).status(201).header("Location", "/itens/10")
```

| forma | devolve | encadeia |
|---|---|---|
| `resp.status_code` | `int` | — |
| `resp.status_code = 201` | nada (é atribuição) | **não** |
| `resp.status(201)` | `JinkerResponse` (o próprio) | sim |

O valor **tem que ser `int`**. Qualquer outro tipo é
`RuntimeError: status_code espera um int` — `"201"` em string não converte
sozinho.

O motor não confere a faixa: `r.status_code = 999` é aceito e vai pro cliente
como `999`. A tabela de códigos usuais está em
[`.status(code)`](../status/status.md#códigos-http-mais-comuns).

## Não confundir com o `status_code` do módulo `request`

O objeto `Response`, que sai de uma requisição HTTP **de saída**, também tem
`status_code` — mas lá é o código que o **outro** servidor respondeu, e é só
leitura. Aqui é o código que a **sua** rota vai devolver, e é gravável.

## Relacionados

- [`.status(code)`](../status/status.md) — o mesmo campo, pela forma encadeável
- [`.json(data, status)`](../json/json.md) e [`.send(text, status)`](../send/send.md)
  — definem corpo e código na mesma chamada
- [`JinkerResponse`](../JinkerResponse.md) — o objeto inteiro
