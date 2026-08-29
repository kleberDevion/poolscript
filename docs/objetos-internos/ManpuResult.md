# `ManpuResult`

> **Objeto interno da linguagem** — você não cria `ManpuResult` na mão:
> é o TIPO de um objeto que a lib `manpu` te entrega pronto.
> Confira com `type(obj)`, que mostra exatamente este nome.

Resultado de operação da manpu.

## Métodos e propriedades

**Nenhum.** `ManpuResult` é um valor que se COMPARA e se TESTA, não um objeto que se
navega — não existe `.status` (tentar acessar dá `AttributeError: 'ManpuResult' object has no attribute 'status'`).

| Uso | Resultado |
|---|---|
| `str(x)` / `post(x)` | o texto do status |
| `x == "Success"` | compara direto com o texto |
| `if x:` | verdadeiro quando deu certo |

[← índice](objetos-internos.md)
