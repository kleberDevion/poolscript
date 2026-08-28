# Coordenação entre sessões que mexem neste repositório

Há mais de uma sessão de agente trabalhando aqui ao mesmo tempo. Sessão morre;
arquivo fica. Este é o registro durável — o canal ao vivo é o `SendMessage`
entre as sessões, mas o que vale é o que está escrito aqui.

## A regra

**Quem chegou primeiro no arquivo fica com ele.** Quem chegar depois se desloca
e anota abaixo. Colisão em `git` custa mais que a espera.

## Divisão em vigor (2026-08-28)

| sessão | arquivos | assunto |
|---|---|---|
| `poolscript-lang-00` | `teste/leis.ps`, `vm/*.c`, `Makefile`, `.github/` | leis (fase 1) e portões (fase 0) |
| `poolscript-lang-01` | `teste/casos_*.c`, `teste/geradores/*.ps` | fase 2 — remover a fotografia |
| — | `teste/casos_robustez.c` | **travado**: precisa de decisão de API do dono (fase 3) |

## A regra que custou tempo, e que vale pros três

**Prove o achado antes de consertar.** A auditoria da suíte que circulou em
28/08 tem pelo menos um falso positivo:

> nº 1 — "`teste/confere_metadata.ps` não tem nenhum `sys.exit`; reporta FALHA
> e sai 0. Rodado em `Makefile:323`. FALSO-VERDE (grave)."

Ele TEM `sys.exit(1)`, na linha 209, com `falhas = falhas + 1` em cada bloco.
Verificado plantando uma divergência de verdade em `METODOS_BYTES`:

```
{ "hex", met_b_hex, NULL }   ->   { "hex", met_b_hex, "obrigatorio" }
```

Resultado: `rc=1` e a mensagem exata
`bytes.hex -> metadata exige 1 parametro(s), motor aceita sem nenhum`.

O protocolo, então: **plante o defeito que o item diz não ser pego, e veja o
portão reprovar.** Se ele reprovar, o item é stale — anote e siga. Consertar o
que não está quebrado é churn, e aqui já custou tempo demais.

## As duas regras do dono, que valem pra qualquer sessão

1. **Nada de falso verde.** Sem skip que conte como sucesso, sem `catch` que
   engole erro, sem teste ajustado pro bug.
2. **Design de API é dele.** Nome, assinatura e semântica da linguagem não se
   mexem sem ele mandar. Arquitetura de teste, build e módulo interno é nossa —
   e empurrar isso pra ele é negligência com cara de deferência.
