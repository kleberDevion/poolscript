# jinker — HTTPS e o aviso "não seguro"

Como servir o jinker em **https://** e como fazer o navegador parar de marcar
como "não seguro".

Ligar HTTPS é uma linha:

```ps
app = Jinker(__name__, oauth={tls: true})
if __name__ == "main" { app(debug=false, host="0.0.0.0", port=2000) }
```

Com `tls: true` e nenhum certificado, o jinker **gera um self-signed** na hora
(`.jinkerTls` + `.jinkerTls.key`, já com SAN pra localhost/127.0.0.1/::1). O
servidor sobe em `https://`, mas o navegador vai mostrar **"não seguro"**.

---

## Por que "não seguro" — e por que não é bug

Um certificado self-signed é assinado por ele mesmo, não por uma **autoridade
certificadora (CA)** que o navegador conhece. TLS foi feito assim de propósito:
o navegador só confia em quem uma CA reconhecida garantiu. Um self-signed não
tem essa garantia, então vira aviso — **em qualquer linguagem, qualquer
servidor**. Não é defeito do jinker.

Para sumir com o aviso, o certificado precisa ser **confiável**. Duas rotas,
conforme onde o servidor roda.

---

## Cert e chave: juntos ou separados

Um cert real vem em **dois arquivos**: o certificado e a chave privada. O
jinker aceita os dois modos:

```ps
# separados (o normal — Let's Encrypt, mkcert)
app = Jinker(__name__, oauth={tls: true, cert: "fullchain.pem", key: "privkey.pem"})

# juntos, num PEM só (cert + chave concatenados)
app = Jinker(__name__, oauth={tls: true, cert: "tudo.pem"})
```

Sem `key`, o jinker procura a chave irmã `<base>.key` ao lado do cert
(`cert.pem` → `cert.key`); não achando, assume a chave dentro do próprio cert.

---

## Dev local / LAN → mkcert

Para desenvolver com `https://localhost` (ou o IP da sua rede) **sem aviso**, o
`mkcert` cria uma CA local que o **seu** navegador passa a confiar.

```bash
# 1. dependência do trust store (Linux)
sudo apt install libnss3-tools

# 2. instale o mkcert (binário único): github.com/FiloSottile/mkcert/releases
#    (ou: sudo apt install mkcert, onde existir)

# 3. instala a CA local no sistema/navegador (uma vez só)
mkcert -install

# 4. gera o cert pros hosts que você usa
mkcert localhost 127.0.0.1 192.168.0.10
#    → gera localhost+2.pem e localhost+2-key.pem
```

```ps
app = Jinker(__name__, oauth={
    tls:  true,
    cert: "localhost+2.pem",
    key:  "localhost+2-key.pem"
})
```

No **seu** PC não aparece mais "não seguro". Num **outro** dispositivo (celular,
outro PC), ou você instala a CA do mkcert nele também (o mkcert gera um
`rootCA.pem` pra isso), ou usa a rota de produção abaixo.

---

## Domínio público → Let's Encrypt

Para um domínio de verdade (ex: `api.seusite.com`), o Let's Encrypt emite um
cert que **todo navegador** confia — cadeado verde, zero aviso, de graça.

Requisitos: um **domínio** apontando pro IP do servidor e a **porta 80**
acessível da internet (pro desafio de validação).

```bash
sudo apt install certbot

# emite o cert (para o seu servidor durante a emissão)
sudo certbot certonly --standalone -d api.seusite.com
#   → /etc/letsencrypt/live/api.seusite.com/fullchain.pem
#   → /etc/letsencrypt/live/api.seusite.com/privkey.pem
```

```ps
app = Jinker(__name__, oauth={
    tls:  true,
    cert: "/etc/letsencrypt/live/api.seusite.com/fullchain.pem",
    key:  "/etc/letsencrypt/live/api.seusite.com/privkey.pem"
})
if __name__ == "main" { app(debug=false, host="0.0.0.0", port=443) }
```

**Renovação** (o cert dura 90 dias): `sudo certbot renew` renova; agende num
cron e **reinicie o jinker** depois pra ele carregar o cert novo.

---

## Testar sem navegador

Aceitando o self-signed (equivale ao "prosseguir mesmo assim"):

```bash
curl -k https://127.0.0.1:2000/
```

Validando **de verdade** contra o próprio cert (prova que ele é válido quando
confiado):

```bash
curl --cacert .jinkerTls --resolve localhost:2000:127.0.0.1 https://localhost:2000/
```

---

## Resumo

| Situação | Solução | Aviso some? |
|---|---|---|
| Rodar rápido, sem se importar com aviso | `tls: true` (self-signed automático) | não (é self-signed) |
| Dev no seu PC / LAN, sem aviso | **mkcert** + `cert=`/`key=` | sim, no seu PC |
| Produção com domínio | **Let's Encrypt** + `cert=`/`key=` | sim, pra todos |

Config do `oauth` (incluindo `tls`, `cert`, `key`, `poolip`): ver
[Jinker/Jinker.md](Jinker/Jinker.md).
