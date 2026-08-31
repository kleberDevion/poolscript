# Rota de ataque — o que falta, em que ordem, e onde termina

Escrita em 2026-08-28 juntando os itens abertos de `TAREFAS.md`, os 26 achados
da auditoria da suíte e a dívida de cobertura. **Reescrita em 2026-08-31**, e a
reescrita é a parte importante deste cabeçalho.

**Por que esta rota existe:** sem ordem e sem linha de chegada, a sessão de
debug não acaba — cada achado abre outro e o trabalho vira caminhada. Aqui cada
item tem **critério de pronto que é um COMANDO**, não uma opinião.

**Por que ela foi reescrita:** as fases 0, 1 e 3 estavam fechadas no código e
continuavam escritas aqui como abertas. O documento foi lido e as três foram
repassadas como pendência real — meia hora gasta conferindo, uma a uma, que já
estavam prontas. Documento de estado que não é atualizado junto com o código
não é neutro: ele custa mais caro que não existir, porque tem a autoridade de
uma lista e o conteúdo de uma lembrança.

Daí as duas regras deste arquivo:

1. **Item fechado sai da lista** no mesmo commit que o fecha.
2. **Número medido não se escreve aqui.** Cobertura, contagem de caso,
   contagem de divergência: rode o comando. A versão anterior dizia
   `ps_mail.c 16,9%` quando já eram 56,9%, e `ps_guzer.c 22,3%` quando eram
   70,4% — números velhos citados como fato.

---

## FECHADO (o que era a rota, e o comando que prova)

| fase | o que era | prova |
|---|---|---|
| 0 — portões desligados | `\|\| true` no CI, filtro vazio saindo 0, pulo silencioso no OOM, `catch` aprovando parada, `try` global no db/mongo, `leis.ps` que ninguém chamava | `make check` |
| 1 — `leis.ps` quebrado | instanciação por `replace`, encolhedor no-op, dedup por enunciado, LCG com bit baixo alternando | `make leis` |
| 2 — casos que não testam nada | fragmentos truncados, duplicados, substring ambígua, expectativa que só olha stderr, `regrava_dif` degradando asserção | `./pool teste/confere_assercoes.ps` e `./pool teste/confere_duplicados.ps` |
| 3 — `casos_robustez.c` | as expectativas eram CONTAGENS de defeito: consertar o motor quebrava o teste | `./testar robustez` |

Os dois portões da fase 2 são novos e ficam no `make check`:

- **`confere_assercoes.ps`** — nenhum caso pode conferir só o stderr (eram
  1577, sem stdout e sem código de saída), e nenhuma mensagem esperada pode ser
  prefixo de outra mensagem do motor (o runner casa por substring, então
  `MemoryError` aprovava qualquer um dos seis).
- **`confere_duplicados.ps`** — nenhum caso pode repetir o fonte de outro
  (eram 232; 47% do `casos_cobertura.c` era cópia do `casos_diferencial.c`).

E `scripts/fortalece_casos.ps` é a ferramenta que mede e reforça: ela RODA cada
caso pra descobrir o stdout e o rc reais, e **recusa gravar** se o caso deixou
de bater com a mensagem que já estava lá — reforçar asserção e apagá-la são
coisas opostas.

---

## ABERTO

### A. Cobertura

O único trabalho de fôlego que sobrou. O número sai de `make cobertura`, que
lista arquivo por arquivo do pior pro melhor e tem catraca: cair reprova.

Duas coisas que a catraca aprendeu em 31/08 e que valem pra ler o relatório:

- Ela compara **ramos cobertos**, não só percentual. Percentual cai sozinho
  quando entra código novo, e tratar isso como regressão faz o portão gritar à
  toa.
- Por isso o relatório distingue **DILUIU** (entrou código novo sem teste) de
  **CAIU** (teste que sumiu). Só o segundo reprova.

Dívida conhecida do dia: os ~620 ramos que os 42 métodos de `bytes` e as
guardas de `snprintf` acrescentaram são quase todos lado de erro, e teste
nenhum os toma.

### B. Bloqueado em você, não em mim

| item | por quê |
|---|---|
| LICENSE / SECURITY.md / SBOM | escolher a licença da PoolScript. O impedimento técnico saiu: era o `libmysqlclient` (GPLv2) entrando ESTÁTICO, que contaminaria a distribuição inteira. Trocado pelo MariaDB Connector/C (LGPL 2.1), ligado DINAMICAMENTE — nessa forma a LGPL não pede nada além do aviso, e a licença do PoolScript volta a ser escolha livre |
| versão e lockfile de pacote | design do `psl` |
| `math`, `random`, CSPRNG | feature, sua |

### C. Ferramenta, quando der

`TSan`, `clang-tidy` em alvo próprio, matriz de CI (hoje um SO só),
`_Static_assert` no GMP, e o ASan sem `detect_leaks=0` (as fibras precisam de
`__sanitizer_start_switch_fiber`).

---

## Como esta sessão termina

Ela termina quando `make check` for verdadeiro — e hoje ele é. O que muda em
relação a 28/08 não é a cor: é que agora ele confere o stdout e o código de
saída de todo caso de erro, recusa caso duplicado, recusa mensagem ambígua,
roda as leis, roda os exemplos e roda os drivers de loopback.

Daqui pra frente, verde quer dizer verde. O que sobrar aparece em
`make cobertura`, com nome de arquivo e distância pra 100%.
