# PoolScript for VSCode

Syntax highlighting, snippets, file icons, and a tailored **Dracula** theme for `.ps` and `.psl` files.

## Features
- 🎨 Dracula color theme tuned for PoolScript scopes
- 🔤 Syntax highlighting for all keywords, types, decorators, f-strings, HTTP verbs and built-ins
- ✂️ Snippets for `if`/`while`/`for each`/`action`/`try`/`using` in BOTH brace `{}` and colon `:` styles
- 📄 File icon for `.ps` / `.psl`
- 🧱 Auto-indent after `:` and after `{`

## Install (.vsix)
```bash
code --install-extension poolscript-0.4.0.vsix
```

Then activate the theme: **Ctrl+K Ctrl+T → "Dracula (PoolScript)"**.

## Build from source
```bash
cd vscode-poolscript
npm install -g @vscode/vsce
vsce package
```

## Snippets
| Prefix | Expands to |
|---|---|
| `if:` | `if cond:` block (Python style) |
| `if{` | `if cond { ... }` block (brace style) |
| `while:` / `for:` / `action:` / `try:` | colon-style blocks |
| `using` | `using open(...) as f { ... }` |
| `imp` / `from` | import statements |

## Notes
- PoolScript v0.4.0 supports both block styles. Inside a `{}` block, child blocks must also use `{}` (the lexer disables indentation tracking when `{` is open).
- Indentation must be exactly **4 spaces**. Tabs are rejected.
