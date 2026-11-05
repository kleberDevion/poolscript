# Ordem em vigor (decisão do Kleber, 2026-11-05)

A correção da base continua primeiro. O que ele escreveu nesta data, e o
detalhe, estão em `notas/PLANEJAMENTO.md`:

1. Terminar as correções até não haver falha nenhuma nos pontos.
2. Revisar a performance. Piorou com as correções: melhorar até ficar como
   estava, ou melhor. Não piorou: manter.
3. Seguir a trilha de desenvolvimento (as adições de `afz1.pr` e
   `sintaxeGENERICS.pr`, registradas no planejamento).
4. O ramo `obra/c1`, a integração com o C, só depois de não haver nada
   comprometedor: falha nos pontos zerada e a performance não deixada pior.
   Até lá não se mexe nele e não se faz merge.

A base de partida da correção segue o `main` medido em 17.0.19. Esta ordem
só muda por escrito dele, neste arquivo.

## O que a sessão faz

1. Lê `notas/CORRECAO-BASE.md`: é a lista única dos defeitos da base, um por
   item, com o programa que mostra o erro e o que a doc promete.
2. Pega o próximo item da lista e conserta. Lote pequeno, plano antes
   (`EnterPlanMode`), aprovação dele, depois o código.
3. **Erro novo = teste novo.** Todo defeito consertado entra como caso fixo na
   suíte (`teste/casos_*.c`), no MESMO commit do conserto, com a saída exata.
   Defeito achado no caminho entra na lista e no conserto; nunca "pra depois".
4. Antes de dizer "pronto": roda o programa do item, com e sem JIT (`./testar`
   e `JINGA_JIT=0 ./testar`), e mostra a saída. Diz o que rodou e o que não
   rodou. Sem isso, não está pronto.
5. **Os portões rodam a cada commit** (decisão dele de 2026-10-17, depois do
   `forAll`): a doc inteira é teste (P1), o checador e o motor concordam (P2),
   o editor e o motor concordam (P3), e a gramática gera programas (P4).
   Estão em `notas/PORTOES.md`, no `make check`. O que eles acham entra na
   lista, e falha conhecida aponta o item aberto.
6. Quando a lista zerar e os portões estiverem verdes, roda a sonda com
   agentes pelo método em `notas/SONDA-METODO.md`. Duas sondas seguidas sem
   defeito novo = base finalizada. Só então ele decide o que entra.

## O critério

O que a doc de cada módulo promete, o motor cumpre, para qualquer programa:
no `--check` e rodando, com o JIT ligado e desligado. Nenhum programa de
pessoa é régua; a régua é a doc. Onde a doc e o motor divergem, o item diz
qual dos dois está errado e ele decide.

## Proibido

- Propor outro método, outro plano ou outra ordem de trabalho. O método está
  escrito; a sessão executa.
- Recurso novo, sintaxe nova, módulo novo, lib nova.
- Mexer no ramo `obra/c1` (a biblioteca do C, parada) ou fazer merge dele.
- Dizer que algo está pronto sem ter rodado.

## Onde está cada coisa

- `notas/CORRECAO-BASE.md`: a lista (os portões e a sonda escrevem, o conserto risca).
- `notas/PORTOES.md`: os portões mecânicos, o que cada um confere e o que achou na entrada.
- `notas/SONDA-METODO.md`: como a sonda com agentes roda (quem olha o quê, como confirma).
- `JINGA-TUDO.md` 6.2: o estado medido e o histórico de versões.
- As regras de trabalho dele (português, sem emoji, Jinga tipada, só Edit/Write,
  commit só com os arquivos do lote, bump a cada alteração, doc no mesmo
  commit) continuam valendo e estão na memória da sessão.
