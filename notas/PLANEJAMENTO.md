# Planejamento (decisão do Kleber, 2026-11-05)

Ordem escrita por ele nesta data. Substitui, a partir daqui, o que a seção 6.2
de `JINGA-TUDO.md` ainda diz sobre "próximos" (Plano 1 / biblioteca do C antes
do resto). O método da correção não muda: continua o de `CLAUDE.md`.

## Ordem

1. **Terminar as correções.** A lista `notas/CORRECAO-BASE.md` chega a zero
   falha nos pontos. O método é o que já está escrito: um item por vez, plano
   antes, aprovação dele, teste novo no mesmo commit, rodar com JIT e sem,
   portões no `make check`. Lista zerada, portões verdes e duas sondas
   seguidas sem defeito novo (`notas/SONDA-METODO.md`) fecham esta fase.
2. **Revisar a performance das correções.** Medir como o projeto já mede
   (os casos de bench e o `perf stat` registrados em `JINGA-TUDO.md` 6.2),
   com JIT e sem, contra o que estava antes das correções. Se piorou,
   melhorar até voltar ao que estava, ou ficar melhor. Se não piorou,
   manter como estava. Não se mexe em performance que as correções não
   pioraram.
3. **Trilha de desenvolvimento.** Só depois das duas fases acima. As adições
   de linguagem estão abaixo. No meio de um item da trilha, erro que aparecer
   se corrige e se revisa antes de seguir; se a performance piorar, fazer o
   necessário para manter o que estava ou melhorar. É a regra que ele escreveu
   em `afz1.pr`.
4. **Ramo de integração com o C** (`obra/c1`). Só depois de não haver nada
   comprometedor ao projeto, como ele definiu nesta decisão: nenhuma falha
   nos pontos da correção, e a performance das correções não deixada pior.
   Até lá o ramo não se mexe e não entra em merge.

Nada da trilha e nada do C entra enquanto a fase 1 não fechar.

## De onde veio a trilha

Dois arquivos dele, fora deste repositório, copiados aqui como decisão, sem
implementar:

- `/home/kleber/Projetos/ProjetosJinga/afazeres/afz1.pr`
- `/home/kleber/Projetos/ProjetosJinga/afazeres/sintaxeGENERICS.pr`

### Antes da lista numerada

**StartTLS** na lib `mail` e na lib `sockets`. Motivo escrito por ele: erro
de handshake.

### Lista, depois de tudo revisado e passado

A ordem é a dele, do mais importante para o menos. O item 5 é o menos
importante.

1. **Arrays**
2. **Tipos**
3. **Generics** — o que `sintaxeGENERICS.pr` pede:
   - tipo parametrizado em variável e em funct, no molde do assegurador do
     Java, aceitando tipo nativo e tipo criado (a base que ele citou é
     `type nome = valor`);
   - a funct tipada só devolve, por impressão, `return`, `yield` ou `raise`,
     o tipo definido; outro tipo é erro;
   - apelidos saem (`Integer` e os outros). Ficam os padrões: `str`, `list`,
     `tup`, `bool`, `flo`, `int`, `byte`, `dict`;
   - o nome do tipo criado é o que ele definir, quando definir.
4. **`static` livre** em qualquer expressão de variável, array ou objeto.
5. **Segundo acesso de membro `::`.** O acesso atual, `.`, permanece. Este
   item é o menos importante.

Cada item da lista só começa depois que o anterior fechou: erros do
desenvolvimento corrigidos, correção revisada, e a performance mantida ou
melhor, pela mesma regra da fase 2.
