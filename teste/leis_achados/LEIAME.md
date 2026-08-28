# Contraexemplos guardados das leis

Cada `.ps` aqui é um valor que **já fez uma lei falhar**. Eles rodam em TODO
`make leis`, antes de qualquer sorteio.

## Por que a pasta existe

`teste/leis.ps` sorteia valor novo a cada execução, e o noturno da CI usa a
semente do DIA. Isso é o que faz a lei achar coisa que exemplo congelado não
acha — e é também o que faz o achado **evaporar**: amanhã a semente é outra, e
o mesmo defeito só reaparece por sorte.

O arquivo guardado resolve os dois lados: o sorteio continua explorando, e o
valor que já quebrou uma vez nunca mais passa batido.

É o mesmo desenho de `teste/fuzz_achados/`, pelo mesmo motivo.

## O ciclo, verificado ponta a ponta

1. defeito presente → a lei acha → o contraexemplo vira arquivo aqui;
2. defeito consertado → nada falha, e o arquivo fica;
3. defeito **volta** → o replay reprova ANTES de qualquer sorteio, dizendo
   "voltar a falhar é regressão, não achado novo".

## Regras

- **Não apague um arquivo daqui** sem decidir que aquele defeito pode voltar.
- O nome vem do enunciado da lei. Se o mesmo enunciado cair de novo com outro
  valor, o arquivo existente é mantido — o primeiro contraexemplo é tão bom
  quanto o mais recente, e assim a pasta não cresce sem fim.
- O arquivo é um programa completo: dá pra rodar sozinho com
  `./pool teste/leis_achados/<nome>.ps` e ele imprime `AINDA FALHA` ou `FIM`.
