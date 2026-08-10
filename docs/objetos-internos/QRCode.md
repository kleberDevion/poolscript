<!-- gerado: gera_doc_gaps.py — pode regenerar -->
# `QRCode`

> **Objeto interno da linguagem** — você não cria `QRCode` na mão:
> é o TIPO de um objeto que a lib `qrcode` te entrega pronto.
> Confira com `type(obj)`, que mostra exatamente este nome.

Wrapper de qrcode.QRCode — mesma API da lib Python:

    qr = qrcode.QRCode(version=None, error_correction="L", box_size=10, border=4)
    qr.add_data("texto")
    qr.make(fit=True)
    img = qr.make_image(fill_color="black", back_color="white")

## Métodos e propriedades

| Acesso | O que faz |
|---|---|
| `.add_data(data)` |  |
| `.clear()` |  |
| `.make(fit=True)` |  |
| `.make_image(fill_color='black', back_color='white', name='qrcode.png')` |  |

[← índice](objetos-internos.md)
