# `PoolQRCode`

> **Objeto interno da linguagem** — você não cria `PoolQRCode` na mão:
> é o TIPO de um objeto que a lib `qrcode` te entrega pronto.
> Confira com `type(obj)`, que mostra exatamente este nome.

O construtor de QR code da lib `qrcode`. `qrcode.QRCode()` devolve um destes —
`type()` dele é `PoolQRCode`, não `QRCode`.

```ps
import qrcode
qr = qrcode.QRCode(version=1, error_correction="L", box_size=10, border=4)
qr.add_data("texto")
qr.make()
img = qr.make_image()          # QRImage; o arquivo padrão é qrcode.png
post(type(img), img.name)      # QRImage qrcode.png
```

Todos os parâmetros podem ser omitidos — `qrcode.QRCode()` e `qr.make()` sem
argumento funcionam. Os defaults **não** aparecem em `pool --metadata`: ele
publica os nomes dos parâmetros das funções de módulo, mas não os valores
padrão. Quem quiser o default, é o que está escrito aqui.

## Métodos e propriedades

| Acesso | O que faz |
|---|---|
| `.add_data(data)` |  |
| `.clear()` |  |
| `.make(fit=True)` |  |
| `.make_image(fill_color='black', back_color='white', name='qrcode.png')` |  |

[← índice](objetos-internos.md)
