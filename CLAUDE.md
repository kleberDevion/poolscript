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

## O acesso ao C é total, e a lib pede `import` (decisão dele, 2026-11-17)

Duas coisas diferentes, não se misturam:

- **Assinatura do C**: a forma do tipo, a declaração, o `sizeof`, o `struct`, o
  ponteiro — **isso vale sempre**, em qualquer programa, sem `import`.
- **Lib**: para chamar a função de verdade, **o `import` é obrigatório**, e
  vale para **qualquer lib C**: cabeçalho (`import <stdio.h>`), `.so`
  (`import c "libfoo.so"`), `.dll` (`"libfoo.dll"`), `.dylib`, ou só o nome
  (`"libfoo"`, resolve no caminho do sistema). Sem ele o nome não existe
  (`NameError`).

**Não existe lista de libc limitando o que o programa alcança** — o acesso é
total a qualquer `.so`/DLL. A tabela do C (`vm/c_padrao.decl`, 735 nomes) é a
**assinatura** (o tipo) desses nomes, não uma licença: nome da lib importada
que não está na tabela chama igual, com a declaração escrita no programa.

- **Diagnóstico do C é do `cc`** (decisão dele, 2026-11-17): a Jinga **não
  reimplementa** diagnóstico de C. Tipo errado, aridade, layout de `struct`,
  segfault, ponteiro inválido, uso depois de `free` — quem decide é o
  `cc`/`clang`. A Jinga **repassa** a declaração (importador de `.h` no build,
  oráculo C/Jinga) e **mostra a frase dele**; o `--check` confere só o que
  sabe sem rodar (tipo conhecido, aridade, formato do `printf`). O resto é
  trabalho de trouxa e não se faz.

Detalhe e o que muda na fatia 1b: `sessionsAI/PLANO-LINKAGEM-C.md`, decisões
7 e 8, a seção 2.2 e a 4.0.

## A ordem das frentes (decisão dele, 2026-11-17)

1. Linkagem com o C (em curso, `obra/c2`);
2. generics;
3. melhoria de performance da linguagem;
4. as libs reescritas em Jinga e mandadas para o repositório de libs dele
   (`kleberDevion/libs`), para o binário emagrecer e programa complexo
   competir com Go e Rust.

Uma frente por vez, na ordem. `sessionsAI/VISAO-2026-11-17.md` tem o resto.

## O binário do PATH (decisão dele, 2026-11-12)

**Todo binário estável sai para o PATH na hora, com `sudo make install`.** Os
portões chamam `jinga` (do PATH) milhares de vezes; com o binário velho lá,
eles não terminam ou dão resultado errado, e o defeito parece ser do motor.

Em 2026-11-12 o PATH estava em 20.0.1 (de 29/10) e o código em 20.0.27. O
portão da gramática e o P3 "travavam" — levou uma sessão para descobrir que
era só o binário.

Antes de rodar qualquer portão, e depois de todo commit que mexa no motor:

1. `jinga --version` e `./pool --version` têm que dizer o MESMO número;
2. se não, `sudo make install` antes de tudo;
3. no codespace também (`sudo make install`, e o `npm install` do editor).
