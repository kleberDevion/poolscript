# Changelog

## v0.5.2 — Operador `count`

### Linguagem
- Novo operador **`count`** — utilitário multifuncional para validar existência e contar ocorrências tipadas em coleções, strings e números.
- Três formas sintáticas suportadas:
  - `count <tipo>(<valor>) in <alvo>` (prefixo com valor)
  - `<tipo>(<valor>) count in <alvo>` (forma infixa)
  - `count <tipo> in <alvo>` (sem valor — conta todos do tipo)
- Bloco contador: `count each <tipo>(<valor>) in <alvo> { ... }`
  - Define `_match`, `_index`, `_count` no escopo do bloco a cada ocorrência.
  - `return;` (vazio) dentro do bloco devolve o **total acumulado** ao final de todas as iterações para a action que envolve.
  - `return <valor>` interrompe e devolve o valor (curto-circuito).
- Suporte a contagem de:
  - listas/tuplas/dicts (filtra por tipo + valor opcional)
  - substring em strings (`count str("oi") in "oi mundo oi"` → 2)
  - dígitos em ints (`count int(7) in 17717` → 3)
  - extração de dígito de um par (`count int(7) in 67` → 1)
- `count` retorna `int` — `0` é falsy, qualquer `> 0` é truthy, então funciona naturalmente em `if`.
- Combinável com `and`/`or`/`==` em condições compostas:
  `if (count int(1) in a and count int(3) in b == 1) { ... }`

### Testes
- 13 novos testes em `tests/test_v0_5_2_count.py` (unit + 1 programa PoolScript completo com 14 markers).
- Total: **122 testes passando** (109 antigos + 13 novos, sem regressões).

### Documentação
- Seção dedicada ao `count` no `README.md` com todos os exemplos.

---

## v0.5.1 — Correção de tipos e comparações

### Linguagem
- `input()` agora detecta automaticamente tipos primitivos digitados: `int`, `flo`, `bool`, `None`/`Null` e `str`.
- `valor is tipo` agora valida tipos PoolScript reais: `str`, `int`, `flo`, `bool`, `list`, `json`.
- Suporte a `not is`, `is not` e `not in` em condições.
- `if (nome)` e validações múltiplas com `and`/`or` funcionam com variáveis dinâmicas e tipadas.
- Conversores globais `str()`, `int()`, `flo()` e `bool()` disponíveis.

### Testes
- Adicionado programa real `.ps` em `tests/poolscript_cases/v0_5_1_type_checks.ps`.
- Adicionadas regressões que executam código PoolScript para tipos, input, comparações, membership e múltiplas validações.

---

## v0.4.0 — Rodada 3 (Indentação Python-style + Extensão VSCode)

### Linguagem
- **Indentação por `:` (estilo Python)** com regras estritas:
  - Apenas espaços; **TAB é proibido** (gera `SyntaxError`).
  - Cada nível deve ser **exatamente 4 espaços**. 2 ou 3 espaços → erro.
  - Avançar mais de 1 nível por vez → erro.
- **Híbrido**: você pode misturar `{}` e `:` no mesmo arquivo.
  - O lexer emite `[poolscript:warn]` quando detecta mistura, sugerindo padronizar.
  - Limitação: dentro de `{...}` blocos filhos também precisam usar `{...}`
    (a indentação é desligada quando `{` está aberto).
- Mensagens de erro indicam claramente o problema:
  `indentação com TAB não é permitida; use 4 espaços`,
  `indentação deve ser múltiplo de 4 espaços (achou 3)`, etc.

### Extensão VSCode (`vscode-poolscript/`)
- Manifesto completo (`package.json`) com:
  - File icon para `.ps` e `.psl`
  - Linguagem registrada
  - Tema **Dracula (PoolScript)**
  - Snippets para todos os blocos em ambos os estilos
- Grammar TextMate (`syntaxes/poolscript.tmLanguage.json`):
  - Keywords, decorators, f-strings, HTTP verbs, built-ins
  - Tipos primitivos em itálico (estilo Dracula)
  - Identificadores `IDENT_UPPER` (libs/classes) destacados como classes
- Auto-indent inteligente após `:` e `{`
- `.vsix` pronto para instalar: `code --install-extension poolscript-0.4.0.vsix`

### Testes
- **+14 testes** de indentação (`tests/test_v0_4_0_indent.py`)
- Total: **105 testes passando** (91 anteriores + 14 novos)

---

## v0.3.0 — Rodada 2 (Built-ins)
- Lib `mail` (MailServer + MailMessage + attach)
- Lib `date` (now, today, timestamp...)
- Lib `request` revisada (patch, JSON auto, headers default)
- Built-ins globais: `open`, `len`, `range`, `type`
- Bloco `using ... as x { }` (context manager)
- 91 testes passando

## v0.2.0 — Rodada 1 (Bug Fixes)
- `post()` aceita argumentos justapostos sem vírgula
- `request` com User-Agent default (corrige 403)
- `import ... as` / `from ... import ... as` / `PUSH ... as`
- Null/null/None/none unificados
- 70 testes passando
