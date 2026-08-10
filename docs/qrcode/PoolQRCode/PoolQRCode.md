# `PoolQRCode`

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

[← índice](../qrcode.md)
