# Instalação do PoolScript v0.5.2

## Instalação automática (recomendado)

### Linux / macOS
```bash
chmod +x install.sh
./install.sh
```

### Windows
Duplo-clique em `install.bat` ou execute no CMD:
```cmd
install.bat
```

O instalador faz tudo automaticamente:
1. **Remove versões antigas** do PoolScript (Python + extensão VSCode + pastas residuais)
2. **Instala** o pacote Python `poolscript` (comandos `pool` e `psl`)
3. **Instala** a extensão VSCode (tema Dracula + syntax highlighting)

## Desinstalação

### Linux / macOS
```bash
./uninstall.sh
```

### Windows
```cmd
uninstall.bat
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

## Instalação manual

Se preferir não usar o script:

```bash
# Python
pip install --user .

# VSCode
code --install-extension vscode-poolscript/poolscript.poolscript-0.5.2.vsix
```
