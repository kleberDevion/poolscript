# PoolScript for VSCode

Syntax highlighting, real IntelliSense, snippets, file icons, and a tailored **Dracula** theme for `.ps` and `.psl` files.

## Features
- 🧠 **IntelliSense powered by the real PoolScript parser** (v1.3.0+): a bundled Python bridge (`bridge/analyze.py`)
  lexes and parses your code with the exact same parser used by the `pool` CLI — never executes your code, only
  static analysis. This gives:
  - **Real-time diagnostics** — syntax errors shown at the exact line/column the real parser reports, not a
    heuristic guess.
  - **Go to Definition** and **Find All References**, resolved from the real AST (functions, `Entity` classes,
    methods, fields, variables, imports).
  - **Signature Help** — parameter hints while typing a call, including `Entity(...)` construction (uses `__init__`).
  - **Rich Hover** — real signatures (`action nome(a, b=…)`), `Entity` fields/methods, declared types.
  - Unused-import/unused-variable hints computed from real symbol references (no false positives from matches
    inside strings/comments).
  - Falls back gracefully to a lighter regex-based analysis if Python isn't available — nothing breaks, you just
    get fewer precise features. Configure `poolscript.pythonPath` in settings if Python isn't on your PATH.
- 🎨 Dracula color theme tuned for PoolScript scopes
- 🔤 Syntax highlighting for all keywords, types, decorators, f-strings, HTTP verbs and built-ins
- ✂️ Snippets for `if`/`while`/`for each`/`action`/`try`/`using` in BOTH brace `{}` and colon `:` styles
- 📄 File icon for `.ps` / `.psl`
- 🧱 Auto-indent after `:` and after `{`

## Requirements
IntelliSense features require **Python 3.10+** on your PATH (or configured via `poolscript.pythonPath`). No
`pip install` needed — the parser is bundled with the extension.

## Install (.vsix)
```bash
code --install-extension psl-poolscript-1.4.1.vsix
```

Then activate the theme: **Ctrl+K Ctrl+T → "Dracula (PoolScript)"**.

## Build from source
```bash
cd psl-poolscript-vsix
npm install -g @vscode/vsce
vsce package
```

If the PoolScript stdlib changed (new lib, new function, renamed lib/module),
regenerate `bridge/stdlib_metadata.json` before packaging — it's introspected
from the real `poolscript.stdlib` package, not hand-maintained, so member
autocomplete (`os.X`, `psodbc.X`, ...) never goes stale:
```bash
pip install -e /path/to/poolscript-lang   # se ainda não tiver instalado
python bridge/gen_stdlib_metadata.py
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
