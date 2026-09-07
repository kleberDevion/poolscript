# `QRCode` — o tipo se chama `PoolQRCode`

`qrcode.QRCode()` existe e funciona, mas o **tipo** dele não se chama `QRCode`:

```ps
import qrcode
post(type(qrcode.QRCode()))    # PoolQRCode
```

`type()` nunca devolve `QRCode`, e o nome não aparece em `pool --metadata`.
Esta página duplicava, sob um nome que não existe, o que está em
[`PoolQRCode`](PoolQRCode.md) — é lá que a API está documentada.

[← índice](objetos-internos.md)
