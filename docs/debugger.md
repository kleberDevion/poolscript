# Depurador

O motor fala **Debug Adapter Protocol** — o mesmo protocolo que o VS Code usa
para qualquer depurador. Quem responde onde a execução está, que frames
existem e qual variável mora em qual slot é a VM, não a extensão: é a mesma
divisão do LSP, e pelo mesmo motivo. Uma cópia dessa lógica em JavaScript
erraria no dia em que o compilador mudasse a numeração dos slots.

---

## 1. No VS Code

Abra um `.ps`, clique na canaleta para pôr o breakpoint e aperte **F5**. Não
precisa de `launch.json`: sem um, a extensão monta a configuração com o arquivo
em foco.

Com `launch.json`, os campos são:

```json
{
  "type": "poolscript",
  "request": "launch",
  "name": "Depurar o arquivo aberto",
  "programa": "${file}",
  "pararNaEntrada": false,
  "args": [],
  "cwd": "${workspaceFolder}"
}
```

| Campo | O que faz |
|---|---|
| `programa` | o `.ps` a depurar (obrigatório) |
| `pararNaEntrada` | para na primeira linha, antes de rodar qualquer coisa |
| `args` | argumentos do programa — é o que o `sys.argv` dele devolve |
| `cwd` | pasta de trabalho; o padrão é a do arquivo |

A saída do programa (`post`, `os.stdout`) vai para o painel **PoolScript
(depuração)**. Ela não passa pelo protocolo de propósito — ver a seção 4.

## 2. Na linha de comando

```bash
pool --debug <porta> <arquivo.ps> [args do programa]
```

O motor escuta em `127.0.0.1:<porta>` e **não executa a primeira instrução
antes de o editor conectar e terminar o aperto de mão**. Sem essa espera, os
breakpoints chegariam depois de o programa já ter passado por eles.

Só `127.0.0.1`: a porta aceita comandos que controlam a execução de código.

## 3. O que ele faz

- **Breakpoint** por arquivo e linha. O caminho é comparado pelo fim, porque o
  editor manda absoluto e o protótipo guarda o caminho como veio na linha de
  comando; exigir igualdade literal fazia todo ponto ser ignorado em silêncio.
- **Passo a passo**: `next` (por cima), `stepIn` (entrando), `stepOut` (até
  voltar), `continue`, `pause`. O passo anda de **linha em linha**, não de
  instrução em instrução.
- **Pilha de chamadas** com nome da função e linha de cada quadro.
- **Variáveis**: os locais **vivos naquele ponto** e os globais.
- **Parada automática na exceção não capturada**, em cima da linha que quebrou
  e com o frame ainda montado — depois desse ponto a VM desmonta os quadros
  para propagar o erro, e a pilha que interessa some.
- **Gráfico de execução** (seção 5).

### Por que "vivos naquele ponto"

`nlocals` é marca d'água e os slots são **reaproveitados** entre blocos: o slot
2 é `x` dentro de um `if` e `y` no `for` seguinte. Por isso o compilador grava,
para cada nome, a faixa de bytecode em que ele vale (`PSVarDbg`), e o painel
mostra só os nomes cuja faixa cobre o `ip` do quadro. Uma lista plana
`nome[slot]` mostraria o nome de uma variável com o valor de outra — e sem
nenhum erro visível.

## 4. Por que socket, e não stdin/stdout

O DAP e a saída do programa dividiriam o mesmo fluxo. Um `post()` no meio de
uma mensagem quebra o enquadramento `Content-Length`, e a sessão morre sem
explicação. Com socket separado, o `post` do usuário continua indo para o
stdout dele, como sempre foi.

## 5. Gráfico de execução

Por onde o programa passou, e onde ele quebrou. O motor acumula as arestas
(linha → linha) durante a execução; **aresta repetida vira contador**, então um
laço de um milhão de voltas é uma seta com peso `1000000×`, não um milhão de
setas. O teto é de 4096 arestas distintas: passando disso, arestas novas param
de ser registradas e as existentes seguem contando, de modo que o caminho
quente continua correto.

No VS Code ele abre sozinho quando o programa quebra, com a linha da falha em
vermelho. Sem erro, abra pela paleta: **PoolScript: Gráfico de execução**.

Pelo protocolo, é o pedido `poolscriptGrafico` (extensão nossa ao DAP), que
responde:

```json
{
  "nos":     [ { "proto": 1, "linha": 5, "nome": "divide", "arquivo": "...", "quebrou": true } ],
  "arestas": [ { "de": {"proto":0,"linha":10}, "para": {"proto":1,"linha":5}, "vezes": 2 } ],
  "quebrou": { "proto": 1, "linha": 5 }
}
```

O pedido continua respondendo **depois** do evento `terminated`: o gráfico só
está completo no fim, e o motor segue atendendo até o editor desconectar.

## 6. O que ele ainda NÃO faz

Estes comandos são **recusados** com motivo, não respondidos em branco:

- `evaluate` — sem janela de watch nem valor no hover.
- `setVariable` — não dá para alterar variável durante a pausa.
- breakpoint **condicional**, por contagem, e `logpoint`.
- `exceptionInfo` e configuração de quais exceções param (só a não capturada
  para, sempre).
- Fibras (`async funct`) aparecem como uma thread só, a principal.

## 7. Testes

`teste/depurador.ps` sobe o motor e fala DAP com ele igual o editor faria — 22
verificações, dentro do `make check`. Os alvos são `teste/alvo_debug.ps` e
`teste/alvo_debug_erro.ps`, e o **número das linhas neles é parte do teste**:
mexer nos arquivos quebra a suíte de propósito, que é o jeito de garantir que o
breakpoint para na linha certa e não numa linha qualquer que por acaso funcione.

O lado da extensão é conferido em `editor/vscode/teste_servidor.js`.
