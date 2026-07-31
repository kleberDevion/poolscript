# qrcode — Geração de QR Code

Requer: `pip install qrcode[pil]`. Espelha a API da lib `qrcode` do
Python — mesmos nomes sempre que possível.

```
import qrcode
// ou: import qr   (alias)
```

---

## qrcode.gen() — função de alto nível

```
// gera em memória
file = qrcode.gen("https://meusite.com")
file.save("qr.png")

// gera e já salva num caminho
qrcode.gen("https://meusite.com", save="qr.png")

// customizado
qrcode.gen(
    "dados",
    save="qr.png",
    size=10,             // box_size — tamanho das caixinhas
    border=2,
    color="black",
    bg="white",
    error_correction="H" // "L" 7%, "M" 15%, "Q" 25%, "H" 30% tolerância a dano
)

// redimensiona a imagem final
qrcode.gen("dados", save="qr.png", qr32=(300, 300))

// dict/list vira JSON automaticamente
qrcode.gen({"user_id": 42, "acao": "checkin"}, save="checkin.png")
```

**Parâmetros de `gen()`**: `data`, `save`, `size=10`, `border=4`,
`color="black"`, `bg="white"`, `name=None`, `error_correction="L"`,
`qr32=None` (tupla largura/altura pra redimensionar).

**Retorno**: sempre um `PoolFile`-like (`QRPoolFile` se ficou em memória,
`PoolFile` se `save=` foi usado) — tem `.save(path)`, `.bytes()`, `.name`,
`.size`, `.ext`.

---

## qrcode.make(data) — estilo Python

Atalho equivalente ao `qrcode.make(data)` da lib original — retorna a
imagem direto, sem passar por arquivo:

```
img = qrcode.make("https://meusite.com")
img.save("qr.png")
img.resize(300, 300)
```

---

## qrcode.QRCode(...) — controle total

Mesma API da classe `QRCode` do Python original:

```
qr = qrcode.QRCode(version=None, error_correction="L", box_size=10, border=4)
qr.add_data("dados")
qr.make(fit=true)
img = qr.make_image(fill_color="black", back_color="white")
img.save("qr.png")

qr.clear()   // limpa pra reusar o mesmo objeto
```

`add_data()` aceita `dict`/`list` também — vira JSON automaticamente.

---

## Constantes de correção de erro

```
qrcode.ERROR_CORRECT_L   // "L" — 7% de tolerância
qrcode.ERROR_CORRECT_M   // "M" — 15%
qrcode.ERROR_CORRECT_Q   // "Q" — 25%
qrcode.ERROR_CORRECT_H   // "H" — 30%
```
