# PoolScript Pro

Suporte à linguagem **PoolScript** no VS Code.

## O que vem aqui dentro, e o que não vem

Esta extensão **não tem cérebro**. São ~70 linhas de JavaScript cuja única
função é levantar o `poolscript-lsp` e falar LSP com ele. O VS Code só carrega
extensão com ponto de entrada JavaScript — essa é a regra do editor, não uma
escolha do projeto.

Completion, hover, diagnóstico e realce semântico vêm todos do **servidor
escrito em PoolScript** (`lsp/servidor.ps` no repositório da linguagem), que
lê o modelo de tipos do próprio binário (`pool --metadata`) e a prosa das
páginas de `docs/`. Nada de dado digitado à mão: se o motor muda, a sugestão
muda junto.

## Requisito

O binário da linguagem precisa estar instalado — é ele que traz o
`poolscript-lsp`:

```bash
git clone <repo> && cd poolscript-lang
sudo make install
```

Isso põe `pool`, `psl`, `poolscript-lsp` e as páginas de `docs/` em
`/usr/local`. Sem as páginas o servidor sobe, mas responde sem explicação
nenhuma.

## Configuração

Nenhuma é necessária: a extensão acha o `poolscript-lsp` no PATH sozinha.

Duas existem para quem mexe no servidor:

```json
{
  // roda o servidor direto do repositório, sem instalar
  "poolscript.lsp.comando": ["pool", "/caminho/do/repo/lsp/servidor.ps"],

  // desliga o servidor; sobra o realce da gramática, que é declarativo
  "poolscript.lsp.ativo": false
}
```

## Quando não funcionar

O painel **Saída → PoolScript** mostra a conversa com o servidor. É o primeiro
lugar a olhar — foi ele que revelou um enquadramento de `Content-Length`
contado em caracteres em vez de bytes, que derrubava o servidor na primeira
resposta com acento.

Depois de reinstalar a extensão ou o binário, o VS Code precisa **recarregar a
janela** (`Ctrl+Shift+P` → *Developer: Reload Window*): ele mantém o processo
antigo do servidor em memória.
