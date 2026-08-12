# PoolScript

Linguagem de programação **híbrida (dinâmica/estática)** — legibilidade do
Python com a estrutura de blocos do JS/C (indentação **ou** chaves, no mesmo
arquivo). Roda em **dois motores em paridade**: o interpretador em Python
(autoridade) e a **PSVM** em C (binário `pool`, sem Python instalado).

## Documentação

A doc completa — essência, como funciona, **instalação (INTERP e PSVM)** e
**guia de deploy** — está em **[`poolscript.md`](poolscript.md)**.

- Sintaxe da linguagem: [`LANGUAGE.md`](LANGUAGE.md)
- Referência das libs (por método): [`docs/INDEX.md`](docs/INDEX.md)
- Editor / LSP (VS Code, IntelliJ, Neovim): [`docs/lsp.md`](docs/lsp.md)

## Começo rápido

```bash
pip install -e .          # comandos `pool` (roda) e `psl` (pacotes) no PATH
pool examples/01_hello.ps
```

Feito por Kleber Santana de Oliveira.
