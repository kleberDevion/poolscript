# Instalação do PoolScript

## Instalação

```bash
pip install -e .
```

Isso registra os comandos `pool` e `psl` no PATH.

## Desinstalação

```bash
pip uninstall poolscript
```

## Pré-requisitos
- **Python 3.10+** ([python.org](https://python.org))
- **VSCode** (opcional, só pra extensão) com o comando `code` no PATH
  - VSCode → `Ctrl+Shift+P` → `Shell Command: Install 'code' command in PATH`

## Teste após instalar
```bash
pool --version
pool examples/01_hello.ps
```

## Extensão VSCode (opcional)

```bash
code --install-extension psl-poolscript-vsix/psl-poolscript-1.4.1.vsix
```
