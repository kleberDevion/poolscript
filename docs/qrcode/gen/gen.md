# `qrcode.gen(data, save=Null, size=10, border=4, color="black", bg="white", name=Null, error_correction="L", qr32=Null)`

Gera um QR Code com controle de tamanho, cores e correção de erro. Devolve um
[`QRPoolFile`](../../objetos-internos/QRPoolFile.md) (com `.save()` e
`.bytes()`); se você passar `save=`, o arquivo já é gravado.

Os exemplos desta página pressupõem:

```ps contexto
import qrcode
str data = "https://meusite.com"
```

```ps
qrcode.gen(data, save=Null, size=10, border=4, color="black",
           bg="white", name=Null, error_correction="L", qr32=Null)
```

| Parâmetro | Padrão | O que é |
|---|---|---|
| `data` | — | conteúdo do QR (str; dict/lista viram JSON) |
| `save` | `Null` | caminho pra salvar (`.png`/`.jpg`); se omitido, fica em memória |
| `size` | `10` | tamanho das "caixinhas" do QR |
| `border` | `4` | largura da borda branca ao redor |
| `color` | `"black"` | cor do QR |
| `bg` | `"white"` | cor do fundo |
| `name` | `Null` | nome do arquivo quando fica em memória |
| `error_correction` | `"L"` | resistência a dano: `L` 7%, `M` 15%, `Q` 25%, `H` 30% |
| `qr32` | `Null` | `(largura, altura)` pra redimensionar a imagem final |

---

## Uso

**Gerar e salvar direto:**

```ps
import qrcode

qrcode.gen("https://meusite.com", save="site.png")
```

**Com cores e tamanho customizados:**

```ps
qrcode.gen(
    "https://meusite.com",
    save="site.png",
    size=15,
    color="navy",
    bg="white",
    error_correction="H"        # aguenta mais dano/sujeira
)
```

**Sem salvar (fica em memória, você grava depois):**

```ps
img = qrcode.gen("texto")
img.save("qr.png")              # grava quando quiser
dados = img.bytes()            # ou pega os bytes (ex: enviar por email)
```

---

## `data` pode ser dict/lista

Se `data` for um dict ou lista, vira JSON automaticamente dentro do QR:

```ps
qrcode.gen({"tipo": "pix", "chave": "ana@x.com"}, save="pix.png")
```

---

## Correção de erro (`error_correction`)

Quanto maior, mais o QR aguenta estar sujo/danificado e ainda ser lido — ao
custo de ficar mais denso:

| Valor | Recupera até | Quando usar |
|---|---|---|
| `"L"` | 7% | tela, ambiente limpo (padrão) |
| `"M"` | 15% | uso geral |
| `"Q"` | 25% | impresso, pode borrar |
| `"H"` | 30% | com logo no meio, ambiente hostil |

---

## Relacionados

- [`qrcode.make()`](../make/make.md) — atalho mais simples
- [`QRImage`](../QRImage/QRImage.md) — o objeto devolvido
