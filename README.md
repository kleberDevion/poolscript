# PoolScript

Doc — **[`poolscript.md`](poolscript.md)**.

- Sintaxe da linguagem: [`docs/LANGUAGE.md`](docs/LANGUAGE.md)
- Referência das libs (por método): [`docs/INDEX.md`](docs/INDEX.md)
- Editor (VS Code, IntelliJ, Neovim): [`docs/lsp.md`](docs/lsp.md)

## Começo rápido

```bash
sudo ./instalar.sh          # binário + servidor LSP + MIME e ícone do .ps
pool examples/01_hello.ps
```

Numa máquina onde ainda não há nada (um WSL Debian novo, por exemplo), o
instalador busca sozinho o que faltar:

```bash
curl -fsSL https://raw.githubusercontent.com/kleberDevion/poolscript-lang/main/instalar.sh | sudo bash
```

Compilando do fonte (gcc + libs de dev: postgresql, mariadb, mongoc, openssl):

```bash
make pool       # gera ./pool
make check      # compila e roda a suíte em C
sudo make install
```
