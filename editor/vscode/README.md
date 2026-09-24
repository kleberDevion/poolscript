# Jinga

Suporte à linguagem **Jinga** no VS Code: realce, autocomplete que sabe o
tipo, hover com a documentação do motor, ir-para-definição, diagnóstico e
depurador.

O cérebro da extensão é um **servidor LSP** (`server.js` + `analise.js`), e ele
pergunta tudo ao binário `jinga`: módulos, membros de cada tipo, tokens do
realce e o diagnóstico. Quem responde o que existe na linguagem é sempre o
motor — não há uma segunda lista aqui pra sair do lugar.

## O que ela faz

- **Realce** de `.pr`, gerado a partir do `jinga --metadata`
  (tipos, builtins e exceções saem do motor, não de uma lista escrita à mão).
- **Autocomplete por tipo**: `"texto".` oferece os métodos de `str`, `[1, 2].`
  os de `list`, e assim por diante; argumentos nomeados, membros de módulo e
  nomes importados entram na lista — inclusive os que vieram por
  `from modulo import *`.
- **Hover** com a assinatura e a prosa da documentação instalada.
- **Ir para a definição** de funct, Entity e nome importado, no arquivo onde
  ele foi declarado.
- **Diagnóstico** enquanto você escreve, com o mesmo erro que `jinga --check`
  daria.
- **Depurador**: pontos de parada, passo a passo, pilha e variáveis.
- **Temas**: `Jinga C# Dark` e `Jinga One Dark`.

## Comandos

| Comando | O que faz |
|---|---|
| `Jinga: Rodar arquivo` | roda o arquivo em foco com o `jinga` |
| `Jinga: Depurar arquivo` | roda sob o depurador do motor |
| `Jinga: Gráfico de execução` | abre o gráfico de execução do arquivo |

## Configurações

| Chave | Padrão | O que é |
|---|---|---|
| `jinga.lsp.ativo` | `true` | liga o servidor de linguagem; desligado, sobra o realce da gramática |
| `jinga.jinga` | `""` | caminho do binário `jinga`; vazio usa o do PATH |

## O que ela precisa

O binário `jinga` instalado (`sudo make install` no repositório da linguagem,
ou o `instalar.sh`). O hover lê a documentação instalada junto com ele, em
`<prefixo>/share/jinga/docs` — sem ela o servidor funciona, mas o hover fica
sem a prosa.

## Referência

- [Editores — VS Code, JetBrains e Neovim](https://github.com/kleberDevion/poolscript/blob/main/docs/lsp.md):
  como o servidor é montado e como ligá-lo nos outros editores.
- [Documentação da linguagem](https://github.com/kleberDevion/poolscript/blob/main/docs/Jinga.md).
