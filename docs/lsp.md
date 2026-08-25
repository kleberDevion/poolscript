# Editores — a PoolScript em VS Code, JetBrains e Neovim

O suporte a editor da linguagem vive na extensão **psl-poolscript**
(`psl-poolscript-vsix/`), que é **autossuficiente**: o realce vem de uma
gramática TextMate e o completion vem de um modelo de tipos embutido
(`bridge/metadata.json`). Não há serviço externo pra instalar nem processo
extra pra subir.

O que a extensão entrega:

- **completion type-aware** — a cadeia `conn = psodbc.connect()` →
  `DbConnection` → `conn.cursor().` → `fetchall/fetchone/...`; `self.` dentro
  de `Entity`; o `request.` do jinker (proxy) vs a lib `request` (verbos);
  argumento nomeado dentro da chamada; `from ._arquivo import <TAB>` com os
  exports reais do arquivo — e tipo desconhecido **não sugere nada** (zero
  método falso);
- **hover** — assinatura + resumo + exemplo, da mesma fonte da doc;
- **realce** completo da sintaxe (`syntaxes/poolscript.tmLanguage.json`).

## VS Code

Instale o `.vsix` do repositório:

```bash
code --install-extension psl-poolscript-vsix/psl-poolscript-1.5.22.vsix
```

`.ps`, `.psl` e `.p` passam a ter realce, completion e hover.

## IntelliJ IDEA (e demais IDEs JetBrains)

As cores saem do **bundle TextMate** — o IDEA lê a gramática da extensão do
VS Code como está:

1. `Settings → Editor → TextMate Bundles` → `+`.
2. Selecione a pasta `psl-poolscript-vsix/` do repositório (o IDEA lê o
   `package.json` + `syntaxes/poolscript.tmLanguage.json`).
3. `.ps`/`.psl`/`.p` ganham o realce completo da linguagem.

## Neovim

Registrar o tipo de arquivo já dá o realce via TextMate/Treesitter externo:

```lua
vim.filetype.add({ extension = { ps = "poolscript", psl = "poolscript", p = "poolscript" } })
```

## Por dentro (pra quem mexe no repositório)

- O cérebro do completion é `psl-poolscript-vsix/extension.js`; o modelo de
  tipos que ele consulta é `psl-poolscript-vsix/bridge/metadata.json`.
- O binário expõe a mesma informação em JSON com `pool --metadata` (módulos,
  membros, tipos e métodos, tudo lido das tabelas do próprio VM) — é a fonte
  pra manter o `metadata.json` em dia sem escrever nada à mão.
- O E2E da extensão (`psl-poolscript-vsix/test/runTest.js`) sobe o VS Code
  REAL e exercita completion e hover num arquivo de verdade.
