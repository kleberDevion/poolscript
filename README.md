# PoolScript

Linguagem de programação **híbrida (dinâmica/estática)** — legibilidade do
Python com a estrutura de blocos do JS/C (indentação **ou** chaves, no mesmo
arquivo). Compila pra bytecode e roda na **PSVM**, máquina virtual em **C** —
o binário `pool`, sem runtime externo.

## Documentação

A doc completa — essência, como funciona, **instalação** e **guia de deploy** —
está em **[`poolscript.md`](poolscript.md)**.

- Sintaxe da linguagem: [`docs/LANGUAGE.md`](docs/LANGUAGE.md)
- Referência das libs (por método): [`docs/INDEX.md`](docs/INDEX.md)
- Editor (VS Code, IntelliJ, Neovim): [`docs/lsp.md`](docs/lsp.md)

## Começo rápido

```bash
sudo install -m755 dist/pool-linux /usr/local/bin/pool   # binário pronto
pool examples/01_hello.ps
```

Compilando do fonte (gcc + libs de dev: postgresql, mysql, mongoc, openssl):

```bash
make -f rebuild/Makefile pool     # gera ./pool
make -f rebuild/Makefile check    # compila e roda a suíte em C
```
