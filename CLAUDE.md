# Ordem em vigor (decisão do Kleber, 2026-10-16)

**Nada novo entra na Jinga até a correção da base que já existe estar
finalizada.** A base é o `main` (17.0.19). Esta ordem só muda por escrito
dele, neste arquivo.

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
5. Quando a lista zerar, roda a sonda de novo pelo método em
   `notas/SONDA-METODO.md`. Duas sondas seguidas sem defeito novo = base
   finalizada. Só então ele decide o que entra.

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

- `notas/CORRECAO-BASE.md`: a lista (a sonda escreve, o conserto risca).
- `notas/SONDA-METODO.md`: como a sonda roda (quem olha o quê, como confirma).
- `JINGA-TUDO.md` 6.2: o estado medido e o histórico de versões.
- As regras de trabalho dele (português, sem emoji, Jinga tipada, só Edit/Write,
  commit só com os arquivos do lote, bump a cada alteração, doc no mesmo
  commit) continuam valendo e estão na memória da sessão.
