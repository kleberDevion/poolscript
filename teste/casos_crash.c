/*
 * Casos que MATAVAM o processo da VM — segfault, SIGFPE, OOM.
 *
 * Estes são os mais importantes da suíte: o runner roda em subprocesso
 * exatamente por causa deles. Se a VM morrer de novo, o caso volta a falhar
 * com "MORREU com sinal N" em vez de derrubar a bateria inteira.
 * Origem: caça com agentes, 2026-08-25.
 */
#include "ps_teste.h"

const Caso CASOS_CRASH[] = {
/* ── recursão sem teto: três SIGSEGV da auditoria de engenharia (27/08) ──────
 * Os três eram descida recursiva sem limite. O detector de CICLO existia e não
 * bastava: ele pega `l.append(l)`, e não pega profundidade sem ciclo nem ciclo
 * que fecha acima do vetor de 256 níveis que ele registra. */
{ "parser: 30 mil parenteses aninhados é erro, nao SIGSEGV",
  /* o gerador do caso é o próprio motor: 30 mil `(` na mão não cabe aqui */
  "import os\n"
  "import sys\n"
  "fundo = 30000\n"
  "src = \"post(\" + (\"(\" * fundo) + \"1\" + (\")\" * fundo) + \")\"\n"
  "using open(\"p.pr\", \"w\") as f { f.write(src) }\n"
  "os.cmd(\"'\" + sys.executable + \"' --check p.pr > o.txt 2>&1\")\n"
  "post(\"aninhada demais\" in open(\"o.txt\").read())\n",
  "True", NULL, 0 },
{ "str() de estrutura profunda trunca, nao mata",
  "x = []\n"
  "i = 0\n"
  "while (i < 100000) {\n"
  "    x = [x]\n"
  "    i = i + 1\n"
  "}\n"
  "post(len(str(x)) > 0)\n", "True", NULL, 0 },
{ "ciclo que fecha ACIMA de 256 niveis nao mata",
  /* O detector só registrava os 256 primeiros níveis, mas o contador seguia
   * subindo: um ciclo fechando em 300 não existia pra ele. */
  "raiz = []\n"
  "x = raiz\n"
  "alvo = Null\n"
  "i = 0\n"
  "while (i < 400) {\n"
  "    novo = []\n"
  "    addEnd(x, novo)\n"
  "    x = novo\n"
  "    if (i == 300) {\n"
  "        alvo = novo\n"
  "    }\n"
  "    i = i + 1\n"
  "}\n"
  "addEnd(x, alvo)\n"
  "post(len(str(raiz)) > 0)\n", "True", NULL, 0 },
/* ── recursão que passa pelo C: a folga da pilha do C, não o teto de frames ──
 * Cada callback vindo do C (map, filter, decorador), cada retomada de gerador e
 * cada import aninham um `vm_executa_base` na pilha do C gastando UM frame da
 * VM: o teto de frames nunca chegava e o processo morria com SIGSEGV. A
 * recursão direta dava RecursionError; estas davam sinal (revisão de 236eba8,
 * 2026-09-14). */
{ "variadico: recursao via callback do C (map) e RecursionError, nao morte por sinal",
  "funct f(x) {\n    return map([x], f)\n}\npost(f(1))\n",
  "", "RecursionError: maximum recursion depth exceeded", 1 },
{ "variadico: recursao via callback do C (filter) e RecursionError, nao morte por sinal",
  "funct f(x) {\n    return filter([x], f)\n}\npost(f(1))\n",
  "", "RecursionError: maximum recursion depth exceeded", 1 },
{ "variadico: recursao via decorador e RecursionError, nao morte por sinal",
  "funct d(f) {\n    @d\n    funct g() {\n        return 1\n    }\n    return g\n}\n@d\nfunct h() {\n    return 2\n}\npost(h())\n",
  "", "RecursionError: maximum recursion depth exceeded", 1 },
{ "gerador que consome a si mesmo recursivamente e RecursionError",
  "funct g(n) {\n    for each x in g(n + 1) {\n        yield x\n    }\n}\npost(list(g(0)))\n",
  "", "RecursionError: maximum recursion depth exceeded", 1 },
{ "RecursionError que passa pelo C e capturavel",
  "funct f(x) {\n    return map([x], f)\n}\ntry {\n    f(1)\n} catch (RecursionError e) {\n    post(\"pegou\")\n}\npost(\"segue\")\n",
  "pegou\nsegue", NULL, 0 },
{ "recursao via map dentro de async e RecursionError na pilha menor da fibra",
  "funct f(x) {\n    return map([x], f)\n}\nasync funct t() {\n    return f(1)\n}\npost(await t())\n",
  "", "RecursionError: maximum recursion depth exceeded", 1 },
/* Contrapeso: a folga medida não pode recusar aninhamento real dentro da
 * fibra. Cada nível custa ~13 KB da pilha do C dela (`-fstack-usage`), e com
 * 128 KB cabiam 8 — o 9º já era RecursionError, enquanto no programa principal
 * cabem 601. Por isso FIB_CSTACK subiu pra 256 KB: 8 níveis passam com folga. */
{ "oito callbacks aninhados dentro de async passam (a folga da fibra nao e limite de uso)",
  "funct nivel(n) {\n    if n <= 0 {\n        return 0\n    }\n    return map([n - 1], nivel)[0]\n}\n"
  "async funct t() {\n    return nivel(8)\n}\npost(await t())\n",
  "0", NULL, 0 },
{ "tres map aninhados dentro de async nao dao RecursionError falso",
  "async funct t() {\n    return map([1], funct(x) {\n        return map([x], funct(y) {\n            return map([y], funct(z) {\n                return z\n            })\n        })\n    })\n}\npost(await t())\n",
  "[[[1]]]", NULL, 0 },

{ "aninhamento LEGITIMO continua passando",
  /* o teto não pode virar limite de uso real: 60 blocos e 500 parênteses */
  "x = 0\n"
  "if (x == 0) { if (x == 0) { if (x == 0) { if (x == 0) {\n"
  "    post(\"fundo\")\n"
  "} } } }\n", "fundo", NULL, 0 },

{ "imprime lista que contém a si mesma",
  "l = [1, 2]\nl.append(l)\npost(l)\n",
  "[1, 2, [...]]", NULL, 0 },

{ "imprime dict que contém a si mesmo",
  "d = { \"a\": 1 }\nd[\"eu\"] = d\npost(d)\n",
  "{'a': 1, 'eu': {...}}", NULL, 0 },

{ "imprime ciclo indireto (lista dentro de lista)",
  "t = [1]\nl = [t]\nt.append(l)\npost(t)\n",
  "[1, [[...]]]", NULL, 0 },

{ "compara estruturas mutuamente recursivas",
  "a = [1]\nb = [1]\na.append(b)\nb.append(a)\npost(a == b)\n",
  NULL, NULL, 0 },

{ "contains em estrutura mutuamente recursiva",
  "a = [1]\nb = [1]\na.append(b)\nb.append(a)\npost(a.contains(b))\n",
  NULL, NULL, 0 },

{ "INT64_MIN % -1 (era SIGFPE)",
  "post(-9223372036854775808 % -1)\n",
  "0", NULL, 0 },

{ "zfill com largura de 64 bits (era OOM da máquina)",
  "post(\"a\".zfill(9223372036854775807))\n",
  "", "memória insuficiente: zfill() pediu largura", 1 },

{ "ljust com largura absurda",
  "post(\"a\".ljust(9223372036854775807))\n",
  "", "memória insuficiente: ljust() pediu largura", 1 },

{ "rjust com largura absurda",
  "post(\"a\".rjust(9223372036854775807))\n",
  "", "memória insuficiente: rjust() pediu largura", 1 },

{ "center com largura absurda",
  "post(\"a\".center(9223372036854775807))\n",
  "", "memória insuficiente: center() pediu largura", 1 },

{ "largura normal continua funcionando",
  "post(\"a\".zfill(5))\npost(\"ab\".ljust(5, \"-\"))\n"
  "post(\"ab\".center(6, \".\"))\npost(\"7\".rjust(3, \"0\"))\n",
  "0000a\nab---\n..ab..\n007", NULL, 0 },

/* ── pilha de FIBRA: a página de guarda, cobrada sem subir servidor ──────────
 * A pilha C da fibra virou `mmap` com uma página `PROT_NONE` embaixo. Antes era
 * `malloc`, e recursão funda dentro de uma fibra passava por cima do heap
 * vizinho — corrupção silenciosa, que só aparecia longe dali.
 *
 * O caso não precisa do jinker: `fib_pega_async` usa exatamente a mesma pilha,
 * então uma `async funct` alcança o mesmo código. Sem estes dois casos a
 * guarda não é cobrada por portão nenhum, porque o e2e do jinker é alvo
 * separado e não roda em todo push. */
{ "recursao funda DENTRO de fibra async é erro, nao corrupcao",
  "funct fundo(n) {\n"
  "    if n <= 0 {\n"
  "        return 0\n"
  "    }\n"
  "    return 1 + fundo(n - 1)\n"
  "}\n"
  "async funct dentro() {\n"
  "    return fundo(100000)\n"
  "}\n"
  "try {\n"
  "    f = dentro()\n"
  "    _r = await f\n"
  "    post(\"passou sem reclamar\")\n"
  "} catch (e) {\n"
  "    post(\"recusou\")\n"
  "}\n", "recusou", NULL, 0 },
{ "pool de fibra cheio é erro limpo, nao morte",
  /* O pool é `FIB_HARD` = 8192 fibras concorrentes; passar disso tem que
   * LEVANTAR, não cair.
   *
   * A forma importa: com `await` ENCADEADO o caso levava 27 s (e estourava o
   * timeout de 20 s do runner), porque cada nível cria uma fibra e espera. Com
   * as fibras criadas de uma vez, o mesmo estouro sai em 0,76 s — 20x mais
   * rápido pra provar exatamente a mesma coisa. Teste lento é teste que alguém
   * acaba desligando. */
  "async funct f(n) {\n"
  "    return n\n"
  "}\n"
  "fs = []\n"
  "i = 0\n"
  "try {\n"
  "    while i < 9000 {\n"
  "        addEnd(fs, f(i))\n"
  "        i = i + 1\n"
  "    }\n"
  "    post(len(gather(fs)))\n"
  "} catch (e) {\n"
  "    post(\"recusou\")\n"
  "}\n", "recusou", NULL, 0 },

/* ── aninhamento LEGÍTIMO: o teto não pode virar limite de uso ───────────────
 * Contrapeso dos casos de teto acima. Se alguém apertar `PS_PARSE_PROF_MAX` pra
 * calar um fuzzer, estes reprovam — que é a única defesa contra o conserto
 * preguiçoso de pôr um número menor. */
/* ── gravacao que falha NAO pode virar sucesso ──────────────────────────────
 * Sete sitios do motor faziam `fwrite(...); fclose(f);` sem olhar nenhum dos
 * dois retornos. `os.writeFile("/dev/full", ...)` devolvia o caminho com rc=0;
 * `PoolFile.save`, `Response.save`, `qrcode.save`, `manpu.write` e
 * `upload.save` faziam o mesmo. /dev/full sempre aceita o `fopen` e sempre
 * recusa a escrita, entao ele e o caso.
 *
 * O `fclose` importa tanto quanto o `fwrite`: o buffer da libc so vai pro disco
 * no flush, entao ENOSPC costuma aparecer AO FECHAR. */
{ "os.writeFile que falha levanta",
  "import os\n"
  "try {\n"
  "    os.writeFile(\"/dev/full\", \"x\" * 200000)\n"
  "    post(\"nao levantou\")\n"
  "} catch (e) {\n"
  "    post(\"levantou\")\n"
  "}\n", "levantou", NULL, 0 },
{ "qrcode.save que falha levanta",
  "import qrcode\n"
  "try {\n"
  "    qrcode.make(\"x\").save(\"/dev/full\")\n"
  "    post(\"nao levantou\")\n"
  "} catch (e) {\n"
  "    post(\"levantou\")\n"
  "}\n", "levantou", NULL, 0 },
{ "gravacao normal continua funcionando",
  "import os\n"
  "os.writeFile(\"n.txt\", \"dado\")\n"
  "post(os.readFile(\"n.txt\"))\n", "dado", NULL, 0 },

/* ── largura de format sem teto: TRAVAVA o processo ─────────────────────────
 * `largura = largura * 10 + digito` transbordava o int em silencio, e o laco
 * de preenchimento escrevia byte a byte ate a memoria acabar. O processo nao
 * morria: ficava comendo RAM. O CPython responde MemoryError. */
{ "largura absurda em format nao trava",
  "post(\"{:99999999999d}\".format(1))\n", "",
  "sem memoria na largura do format", 1 },
{ "precisao absurda em format nao trava",
  "post(\"{:.99999999999f}\".format(1.5))\n", "",
  "sem memoria na precisao do format", 1 },
{ "format normal continua funcionando",
  "post(\"{:>8d}\".format(42))\n"
  "post(\"{:.2f}\".format(3.14159))\n"
  "post(\"{:08.3f}\".format(3.14159))\n",
  "      42\n3.14\n0003.142", NULL, 0 },

/* ── copia que falha NAO pode apagar a origem ───────────────────────────────
 * `copia_arquivo` nao conferia `fwrite` nem `fclose`: gravacao que falhava era
 * ignorada, a funcao dizia "copiou" e o `os.move` dava unlink na ORIGEM.
 * Perda de dado em silencio, sem erro nenhum. /dev/full sempre aceita o
 * `fopen` e sempre recusa a escrita, entao ele e o caso. */
{ "os.move que falha mantem a origem",
  "import os\n"
  "using open(\"o.txt\", \"w\") as f { f.write(\"dado\") }\n"
  "houve_erro = false\n"
  "try { os.move(\"o.txt\", \"/dev/full\") } catch (e) { houve_erro = true }\n"
  "post(houve_erro, os.exists(\"o.txt\"))\n", "True True", NULL, 0 },
{ "os.copy que falha nao deixa destino pela metade",
  "import os\n"
  "using open(\"o2.txt\", \"w\") as f { f.write(\"dado\") }\n"
  "houve_erro = false\n"
  "try { os.copy(\"o2.txt\", \"/dev/full\") } catch (e) { houve_erro = true }\n"
  "post(houve_erro, os.exists(\"o2.txt\"))\n", "True True", NULL, 0 },

{ "500 parenteses aninhados sao LEGITIMOS",
  "import os\n"
  "import sys\n"
  "src = \"post(\" + (\"(\" * 500) + \"1\" + (\")\" * 500) + \")\"\n"
  "using open(\"leg.pr\", \"w\") as f { f.write(src) }\n"
  "os.cmd(\"'\" + sys.executable + \"' leg.pr > o.txt 2>&1\")\n"
  "post(open(\"o.txt\").read().strip())\n", "1", NULL, 0 },
{ "estrutura 200 niveis funda imprime inteira",
  "x = [\"fundo\"]\n"
  "i = 0\n"
  "while (i < 200) {\n"
  "    x = [x]\n"
  "    i = i + 1\n"
  "}\n"
  "post(\"fundo\" in str(x))\n", "True", NULL, 0 },
};
const int NC_CRASH = N_CASOS(CASOS_CRASH);
