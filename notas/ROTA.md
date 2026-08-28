# Rota de ataque — o que falta, em que ordem, e onde termina

Escrito em 2026-08-28, juntando numa lista só: os itens abertos de
`TAREFAS.md`, os 26 achados da auditoria da suíte, e a dívida de cobertura.

**Por que esta rota existe:** sem ordem e sem linha de chegada, a sessão de
debug não acaba — cada achado abre outro e o trabalho vira caminhada. Aqui cada
item tem **critério de pronto que é um COMANDO**, não uma opinião. Se o comando
sai 0, o item está fechado; se não sai, não está.

**A ordem é por alavancagem, não por gravidade nominal.** Um portão desligado
vem antes de um bug, porque enquanto ele está desligado nenhum conserto é
verificável.

---

## FASE 0 — religar os portões desligados

Nada abaixo vale enquanto um portão reportar verde sem conferir. São mudanças de
uma a dez linhas cada; a fase inteira é uma tarde.

| # | onde | o quê | pronto quando |
|---|---|---|---|
| ~~0.1~~ | `teste/confere_metadata.ps` | **FALSO POSITIVO da auditoria** — ele TEM `sys.exit(1)` na :209, com `falhas = falhas + 1` em cada bloco. Provado plantando `{ "hex", met_b_hex, "obrigatorio" }`: rc=1 e apontou o método | já pronto; ver `notas/COORDENACAO.md` |
| 0.2 | `.github/workflows/ci.yml` :95 | `make avisos AVISOS_NIVEL=2 \|\| true` — o mesmo defeito que `a4fce5f` disse ter corrigido, sobrevivendo no CI | remover o `\|\| true` ou tirar a etapa; sem terceira via |
| 0.3 | `teste/unidade.c` :590 | filtro que não casa nada → `total=0` → exit 0. O `ps_teste.c:376` acertou o mesmo ponto (`return 2`) | `./unidade filtro_que_nao_existe` sair != 0 |
| 0.4 | `teste/oom_varre.ps` :118-121 :171 | não conseguir contar alocações → `continue` silencioso → "nenhuma morte violenta", rc 0 | pular deixa de ser sucesso: N pulados > 0 sai != 0 |
| 0.5 | `teste/jinker_roda.ps` :252-259 | `catch (e) { parou = true }` — DNS, timeout ou bug do cliente aprovam a checagem | conferir que o servidor de fato saiu (pid morto), não que a chamada falhou |
| 0.6 | `teste/e2e/db.ps` :45-57, `teste/e2e/mongo.ps` :5-34 | `try` em volta do corpo inteiro: bug real vira PULOU e sai 0. Falha e ausência de serviço são indistinguíveis **por construção** | sonda de conexão explícita (o padrão certo já existe em `mongo_roda.ps:39-42`) e `confere()` no lugar dos `post()` |
| 0.7 | `teste/leis.ps` | 588 linhas, a única ferramenta de REGRA do repo, e **nenhum alvo ou CI a invoca** | `make leis` existe, entra no CI e reprova com contraexemplo |

**Pronto da fase:** para cada item, plantar o defeito que ele deveria pegar e
ver o portão reprovar. Sem essa prova, o item não conta.

**E a regra vale nos DOIS sentidos**: o 0.1 acima era falso positivo e só se
descobriu porque a prova foi feita antes do conserto. Item de auditoria não é
fato — é hipótese até o defeito plantado reprovar.

---

## FASE 1 — a resposta estrutural: leis que funcionam

`leis.ps` é o caminho certo e está quebrado em quatro pontos, achados na
auditoria. Corrigir vem antes de escrever lei nova — lei que roda errada é pior
que lei que não existe.

| # | bug | efeito |
|---|---|---|
| 1.1 | instanciação por `replace("A"/"B"/"C")` | texto sorteado que CONTÉM essas letras corrompe o programa. `"aAbB"` está na lista de borda: o `replace("B", …)` reescreve dentro do literal já inserido |
| 1.2 | encolhedor recebe a forma já instanciada (:576) em vez do template | o shrink é no-op e rotula o achado com `A=0` falso. A linha :575 é código morto |
| 1.3 | dedup por enunciado | "cortar e juntar reconstrói" existe em `ti` e `li`: se as duas falharem, só uma aparece |
| 1.4 | LCG com módulo 2³² | o bit baixo alterna: `rnd(2)` devolve 0,1,0,1 determinístico — metade dos "sorteios" não sorteia |

Depois de corrigidos, **crescer o conjunto**: hoje são 70 leis. Os domínios sem
nenhuma: float (NaN, infinito, arredondamento), bytes, tupla, dict com chave não
texto, exceção (levantar/pegar/relançar), geradores (`yield`), formatação
(`f""`), datas, `sort` com valores mistos.

**Pronto da fase:** `make leis` com 5000 rodadas sai 0, e plantar um bug no
motor (ex.: fazer `a+b` errar em um caso) faz uma lei reprovar com o
contraexemplo MENOR.

---

## FASE 2 — remover a fotografia que não testa nada

Ordem: primeiro o que é **provadamente lixo**, depois o que é fraco.

| # | quantos | o quê | ação |
|---|---|---|---|
| 2.1 | 80 casos | `casos_diferencial.c`: fragmentos TRUNCADOS do extrator. 74 têm como asserção inteira "falta `import os`". 5 têm fonte praticamente vazio | deletar — é fotografia de um bug do extrator, não de comportamento |
| 2.2 | 231 casos | duplicados byte-idênticos entre `casos_cobertura.c` e `casos_diferencial.c` (48% do primeiro) | deletar do `cobertura`, que é o mais novo |
| 2.3 | 3 casos | perderam o sentido em `229f75c` e hoje o nome mente (`casos_linguagem.c:486` "action ':' dentro de bloco" não tem nenhum `:`) | reescrever pro que o nome diz, ou deletar |
| 2.4 | 4 famílias | `casos_equivalencia.c` :20≡:38, :81≡:99, :142≡:160, :203≡:221 — comparam o programa **com ele mesmo**, sempre passam | corrigir o gerador; 22 das 50 famílias têm só 2 formas distintas |
| 2.5 | 644 casos | substring ambígua: `variável não definida: j` casa com `: json`; `: c` com `: c2`. 72 das 362 mensagens são prefixo de outra | alongar a substring até ser única, no gerador |
| 2.6 | 1399 casos | **100% dos casos de erro conferem só stderr** — nunca stdout, nunca rc. Programa que imprime lixo e depois erra certo, passa | passar a conferir rc junto; stdout onde fizer sentido |
| 2.7 | `scripts/regrava_dif.ps` :115 | regrava o esperado da saída de hoje E hardcoda `NULL, …, -1`, degradando a asserção pra sempre. Se o caso passar limpo grava `""`, que o `strstr` casa com tudo | preservar stdout+rc ao regravar; recusar gravar `""` |

**Pronto da fase:** `./testar` continua verde com menos casos, e a cobertura de
ramo **não cai** (o portão da catraca já cobra isso).

---

## FASE 3 — `casos_robustez.c`: o teste está do lado do bug

O mais grave e o que precisa de decisão de design junto.

Hoje as 14 expectativas congelam contagens de defeito: `:36` espera
literalmente `"10217 caladas em 83 metodos: [...]"` — ~10.500 chamadas com
aridade ou tipo errado que o motor **aceita em silêncio**, gravadas como o
resultado esperado. Consertar qualquer uma delas QUEBRA O TESTE. O cabeçalho
promete "erro capturável, não silêncio" e o caso trava o silêncio.

A arquitetura certa (minha, não sua): o caso deixa de gravar contagem e passa a
declarar **quais métodos DEVEM levantar**, falhando enquanto não levantam — que
é o que `casos_pendentes.c` já faz e o runner já suporta (`ps_teste.c:340`).
Assim consertar um método faz o teste FICAR verde, não quebrar.

Quais dos ~10.500 devem levantar é decisão de API — essa parte é sua, e vem em
lista, por tipo, não de uma vez.

**Pronto da fase:** nenhuma expectativa da suíte é uma contagem de defeito.

---

## FASE 4 — o resto de `TAREFAS.md`

Nesta ordem, por alavancagem:

1. **§3.4** ASan: tirar `detect_leaks=0`, anotar as fibras
   (`__sanitizer_start_switch_fiber` — ficou mais grave depois que a pilha
   virou `mmap`), TSan.
2. **§3.11** catálogo de ilogismos **I1–I23** (só I17 fechado) — 22 itens, é o
   maior bloco isolado que sobrou.
3. **§3.3** clang-tidy (instalado) num alvo próprio, em lote por arquivo.
4. **§3.6** matriz de CI (hoje um SO só) e `_Static_assert` no GMP.
5. **§3.9** LICENSE / SECURITY.md / SBOM — **bloqueado em você**:
   libmysqlclient é GPLv2 e entra estático no binário.
6. **§3.2** versão e lockfile de pacote — design de `psl`, seu.
7. **§3.8** `math`, `random`, CSPRNG — feature, sua.
8. **§3.10** código morto da era CPython — só depois que a suíte antiga estiver
   de pé e provar que não precisa dele.

## FASE 5 — cobertura

Hoje: linha 81,6% · função 88,1% · ramo 58,8%. A catraca impede cair. Subir vem
por lei nova (fase 1) e por teste em C direto (`unidade.c`), não por caso novo.

Falta pra 100% de ramo, do pior pro melhor: `ps_mail.c` 16,9 · `ps_guzer.c`
22,3 · `ps_db.c` 45,7 · `main.c` 48,8 · `ps_xlsx.c` 53,2 · `ps_mongo.c` 55,0 ·
`ps_pilha.c` 56,2 · `poolscript_vm.c` 56,4.

---

## Fora da rota, de propósito

- **A suíte antiga em Python.** Ela é o que mediu a dívida de cobertura e o
  alcance in-process, e recuperá-la como arquivo no repositório foi pedido e não
  entregue. Não está numa fase porque não é uma fase: é uma dívida minha, e vem
  antes da FASE 2 (é ela que diz se um caso apagado tinha valor).
- **`bytes.len()`** — feito. Travado em `casos_libs.c` com todas as origens de
  bytes da VM.

---

## Como esta sessão termina

Ela termina quando `make check` for verdadeiro. Hoje ele é verde e:

- um portão dele não reprova nada (0.1);
- um dos testes que ele roda está do lado do bug (fase 3);
- e a única ferramenta de regra do repo não é chamada por ele (0.7).

Fechada a FASE 0, `make check` verde passa a significar alguma coisa. Fechadas a
1 e a 2, ele passa a significar o que diz. Da 3 em diante é dívida conhecida,
não mentira — e dívida conhecida pode esperar.
