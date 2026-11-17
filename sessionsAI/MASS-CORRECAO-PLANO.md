# Plano de correção em massa — 2026-11-16

> **Fechamento (2026-11-16):** a catraca `teste/portoes_conhecidas.txt` está
> **vazia** (0 linhas). O estado item a item está na seção 6, no fim. As
> contagens da seção 1 são o retrato do início do dia; ficam como histórico.

O que ainda falta, decomposto item a item, com **um agente `.md` por item**. Cada
agente é um arquivo de ação autossuficiente; os agentes **trocam conteúdo entre
si** (o mecanismo está em [`TROCA.md`](TROCA.md) e em
[`agentes/A00-mestre.md`](agentes/A00-mestre.md)).

Tudo se mede pela mesma fonte que já existe: a catraca `teste/portoes_conhecidas.txt`
(portões P1–P4, gramática). O plano não inventa contagem nova.

---

## 1. O que falta hoje (medido, não estimado)

`grep -v '^#' teste/portoes_conhecidas.txt | cut -f4 | sort | uniq -c`

| item                       | linhas na catraca | portão         | o que é                                                                                                                                                     |
| -------------------------- | ----------------: | -------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **motor-11**               |               243 | `p2`           | aridade/assinatura dos tipos da jinker no checador: `RequestProxy` (118), `Response` (64), `PoolFileUpload` (33), `WsConnection` (25), `RouteRegistrar` (3) |
| **doc-20**                 |                16 | `roda`/`check` | exemplos de jinker/json/mail/psodbc que só passam com server/bd vivo                                                                                        |
| **doc-26**                 |                12 | `titulo`       | assinatura do título da página divergindo do motor                                                                                                          |
| **doc-13**                 |                 4 | `check`        | `docs/libs_utilitarias.md` recusado pelo `--check`                                                                                                          |
| **doc-5 / doc-6 / doc-19** |             3+2+2 | `roda`         | `sockets` (detach, connect_ex, getpeername, setsockopt) e `psodbc/query`                                                                                    |
| **doc-21**                 |                 1 | `roda`         | `qrcode/QRImage/to_file`                                                                                                                                    |
| **motor-12**               |                 1 | `p2`           | `sockets.TCP_KEEPALIVE`                                                                                                                                     |
| **motor-21**               |                 1 | `gramatica`    | palavra reservada como valor (caso da suíte)                                                                                                                |

Fora da catraca, as **decisões pendentes** (não são código, são escolha):

| item     | pergunta                                                           |
| -------- | ------------------------------------------------------------------ |
| motor-41 | `mail` com STARTTLS: implementar o handshake na mão ou exigir lib? |
| motor-42 | TLS em `sockets`: mesma pergunta                                   |
| motor-45 | posição do erro **dentro** do campo da f-string (parte 2)          |

Regra do repositório que vale para todo agente: cada item fechado sai da catraca
na mesma entrega (portão reprova falha conhecida que sumiu) e vira **um commit**
com o bump de `vm/ps_versao.h` + `docs/Jinga.md`.

---

## 2. Os agentes

Um arquivo por item em [`agentes/`](agentes/). Cada um tem: escopo fechado, o
que **não** é dele, a medida de "pronto", e o bloco `== ENTREGA ==` que ele
publica.

| agente                                                | item         | entrega                                          |
| ----------------------------------------------------- | ------------ | ------------------------------------------------ |
| [A00-mestre](agentes/A00-mestre.md)                   | —            | roda a rodada, agrega entregas, libera a catraca |
| [A11-requestproxy](agentes/A11-requestproxy.md)       | motor-11a    | 118 linhas `RequestProxy`                        |
| [A12-response](agentes/A12-response.md)               | motor-11b    | 64 linhas `Response`                             |
| [A13-poolfileupload](agentes/A13-poolfileupload.md)   | motor-11c    | 33 linhas `PoolFileUpload`                       |
| [A14-wsconnection](agentes/A14-wsconnection.md)       | motor-11d    | 25 linhas `WsConnection` + 3 `RouteRegistrar`    |
| [A15-aridade](agentes/A15-aridade.md)                 | motor-11     | `MEDE-ARIDADES.PR` como fonte única das aridades |
| [A20-jinker-exemplos](agentes/A20-jinker-exemplos.md) | doc-20       | 16 linhas                                        |
| [A26-titulos](agentes/A26-titulos.md)                 | doc-26       | 12 linhas `titulo`                               |
| [A13-utilitarias](agentes/A13-utilitarias.md)         | doc-13       | 4 linhas `check`                                 |
| [A05-sockets](agentes/A05-sockets.md)                 | doc-5, doc-6 | 5 linhas `roda`                                  |
| [A19-psodbc](agentes/A19-psodbc.md)                   | doc-19       | 2 linhas `roda`                                  |
| [A21-qrcode](agentes/A21-qrcode.md)                   | doc-21       | 1 linha `roda`                                   |
| [A12-keepalive](agentes/A12-keepalive.md)             | motor-12     | 1 linha `p2`                                     |
| [A21-reservada](agentes/A21-reservada.md)             | motor-21     | 1 linha `gramatica`                              |
| [A41-mail-starttls](agentes/A41-mail-starttls.md)     | motor-41     | **decisão** + entrega condicional                |
| [A42-sockets-tls](agentes/A42-sockets-tls.md)         | motor-42     | **decisão** + entrega condicional                |
| [A45-fstring-pos](agentes/A45-fstring-pos.md)         | motor-45     | **decisão** + entrega condicional                |

Nenhum agente mexe na catraca de outro item nem em item de outra frente; se
descobrir que o problema é de outro, escreve isso no `PEDIDO` (ver `TROCA.md`) e
entrega o que é seu.

---

## 3. Ordem das rodadas

1. **Rodada 0 — leitura.** Todo agente roda o portão do seu item e transcreve as
   linhas da catraca que são dele para o seu `DIAGNOSTICO`. Sem isso ninguém
   começa.
2. **Rodada 1 — os 243 da motor-11** (A11–A15 em paralelo). É o maior bloco; A15
   publica as aridades primeiro e A11–A14 consomem.
3. **Rodada 2 — doc** (A20, A26, A13-utilitarias, A05, A19, A21-qrcode). Depende
   das aridades certas da rodada 1 (os exemplos da doc chamam os mesmos membros).
4. **Rodada 3 — avulsos** (A12-keepalive, A21-reservada).
5. **Rodada 4 — decisões** (A41, A42, A45): o usuário responde a pergunta; se a
   resposta for "fazer", o agente vira item normais e entra na rodada 5.
6. **Fechamento — A00.** `make check` completo, catraca vazia **das entradas
   fechadas** e nenhuma entrada nova; bump de versão; commit por item.

Rodadas 1–3 são independentes entre si quanto a arquivo; a ordem existe só para
que as aridades da A15 cheguem antes de quem depende delas.

---

## 4. Troca de conteúdo entre agentes

Resumo (o contrato completo está em [`TROCA.md`](TROCA.md)):

- cada agente escreve no **próprio** `.md`, só nos blocos `DIAGNOSTICO`,
  `PEDIDO`, `ENTREGA` e `RESPOSTA`;
- `DEPENDE DE` / `LIBERA PARA` no cabeçalho dizem o fluxo — o mestre copia o
  bloco `== ENTREGA ==` de um para o `== ENTRADA ==` do outro;
- um agente **nunca** edita o `.md` de outro; a troca é sempre mediada (mestre
  copia) ou por `PEDIDO` explícito;
- conteúdo trocado é literal: o mestre não reescreve, só recorta e cola.

---

## 5. Portão do plano

```bash
scripts/verifica_plano_agentes.sh          # cada .md tem os blocos obrigatórios
./pool scripts/confere_checador.pr         # p2 (motor-11, motor-12)
./pool scripts/audita_doc.pr               # titulo (doc-26)
./pool scripts/confere_suite.pr            # p4
make check                                 # tudo junto (A00, fechamento)
```

Enquanto o plano está em curso, `confere_*` pode reprovar por falha **nova**; a
regra é: toda linha nova da catraca tem que apontar para um agente deste plano.

---

## 6. Estado real no fechamento (2026-11-16)

| Item                          | Estado     | Evidência                                                                 |
| ----------------------------- | ---------- | ------------------------------------------------------------------------- |
| motor-11 (A11–A15)            | **fechado** | 243 linhas `p2` removidas; `vm/aridades_medidas.inc` regerado             |
| motor-12 (keepalive)          | **fechado** | 1 linha `p2` removida; `SO_KEEPALIVE(0)` reproduz a recusa do motor       |
| doc-26 (títulos)              | **fechado** | `audita_doc.pr`: nenhuma divergência; 12 linhas `titulo` removidas        |
| doc-13 (utilitárias)          | **fechado** | `audita_exemplos_doc.pr`: 0 falhas; 4 linhas `check` removidas            |
| doc-20 (exemplos jinker/json) | **fechado** | `audita_exemplos_doc`/`confere_saida_doc`: 0 falhas; 16 linhas removidas  |
| doc-5/doc-6 (sockets)         | **fechado** | exemplos e seções "Erros" aceitam os números de Linux/macOS/Windows       |
| doc-19 (`psodbc.query`)       | **fechado** | `cmd=(sql, params)` implementado e testado; 2 linhas removidas            |
| doc-21 (QR no e-mail)         | **fechado** | `attach` aceita `QRPoolFile`; `attach(to_file())` → `True`; 1 linha       |
| motor-21 (reservada)          | **fechado** | parser já corrigido no 20.0.18; gramática escrita alinhada; 1 linha       |
| motor-41 (mail STARTTLS)      | **fechado** | commit 8f07e87 (20.0.48)                                                   |
| motor-45 (f-string)           | **fechado** | 20.0.29                                                                    |
| motor-42 (TLS em sockets)     | **adiado**  | fica para a frente de linkagem com o C: a `sockets` vira wrapper da SSL do C (`.h`/`.so`) e o próprio Dev monta a conexão TLS |

Os 16 arquivos de agente (`A11`…`A45`) não foram usados como mecanismo: o
trabalho foi feito diretamente nos arquivos e medido pelos mesmos portões que
o plano cita. O `scripts/verifica_plano_agentes.sh` citado na seção 5 nunca
existiu no repositório.
