# Troca de conteúdo entre agentes

O contrato que todo agente de [`agentes/`](.) segue. Vale para os agentes deste
plano e para os agentes de sessão antigos (`../session-*.md`), que continuam
sendo só histórico.

## 1. O agente é um `.md`

Um agente é um arquivo, não um processo. O que o faz "agir" é a disciplina:
ele só escreve nos quatro blocos abaixo, com os marcadores exatos, e o resto do
arquivo é estável. Assim o mestre consegue recortar e colar sem interpretar.

```
## CABECALHO
item:         motor-11a
depende_de:   A15-aridade
libera_para:  A20-jinker-exemplos
arquivos:     vm/ps_checador.c
portao:       ./pool scripts/confere_checador.pr
## == DIAGNOSTICO ==
## == PEDIDO ==
## == ENTRADA ==
## == ENTREGA ==
## == RESPOSTA ==
```

## 2. Quem escreve onde

| bloco         | quem escreve                   | conteúdo                                                      |
| ------------- | ------------------------------ | ------------------------------------------------------------- |
| `DIAGNOSTICO` | o agente                       | as linhas da catraca que são do item, medidas, com a data     |
| `PEDIDO`      | o agente                       | algo que precisa de outro agente, com o item e o trecho exato |
| `ENTRADA`     | **só o mestre**                | a entrega de outro agente, colada literal                     |
| `ENTREGA`     | o agente                       | o que mudou, o commit, e as linhas que saíram da catraca      |
| `RESPOSTA`    | o agente que recebeu `ENTRADA` | o que fez com a entrada (consumiu, ignorou, virou pedido)     |

Regras duras:

1. **Um agente nunca edita o `.md` de outro.** Nem para "corrigir".
2. Entrega é **literal**: o mestre recorta a linha entre `== ENTREGA ==` do
   emissor e o próximo marcador `##` e cola em `== ENTRADA ==` do receptor,
   com o cabeçalho `de: <agente>`.
3. `PEDIDO` só vale se citar um agente existente em `agentes/`. Pedido para
   "alguém" não é pedido.
4. Item que muda de dono durante a rodada muda de agente: o dono antigo fecha o
   `ENTREGA` com `transferido para: Axx`, e o novo abre `ENTRADA` com
   `de: Axx`.
5. Conflito: dois agentes dizem que o mesmo arquivo é deles → o mestre decide e
   registra em `DIAGNOSTICO` de ambos com a mesma frase `dono: Axx`.

## 3. Como o mestre roda uma rodada

1. `scripts/verifica_plano_agentes.sh` — todo agente tem os cinco blocos e os
   três campos de cabeçalho.
2. Para cada agente cujo `depende_de` já tem `ENTREGA` preenchida: colar a entrega
   em `ENTRADA`.
3. Para cada agente com `ENTREGA` preenchida: rodar o portão citado; se verde,
   liberar as linhas da catraca que ele fechou.
4. Qualquer `PEDIDO` sem `RESPOSTA` no fim da rodada vira pendência para a rodada
   seguinte — não some.

## 4. O que conta como entrega

Uma entrega é:

```
de:        A11-requestproxy
item:      motor-11a
commit:    <hash>
mudou:     vm/ps_checador.c: aridade de RequestProxy.json/text/query...
saiu da catraca: 118 linhas (`p2 membro:RequestProxy.*`)
portao:    ./pool scripts/confere_checador.pr -> verde
```

Sem `commit` e sem a contagem de linhas que saíram, a entrega não é aceita — não
dá para verificar sem esses dois campos.
