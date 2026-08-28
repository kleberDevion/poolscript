/*
 * Bibliotecas que rodam OFFLINE — o alcance que a suíte em Python tinha e a
 * suíte em C perdeu na migração.
 *
 * POR QUE ESTE ARQUIVO EXISTE: medindo as duas suítes com o mesmo método
 * (gcov, -O0, ramo), a suíte em pytest de `db0bcb6` cobria 78,8% de linha e
 * 53,3% de ramo; a suíte em C de hoje, 55,0% e 41,4%. A diferença não estava
 * na linguagem — estava nas BIBLIOTECAS: `ps_db.c`, `ps_pkg.c`, `ps_hash.c`,
 * `ps_http.c` e os 254 nativos `mod_*`/`met_*` que nenhum caso chamava. A
 * suíte antiga testava em processo e alcançava tudo isso; a nova roda só
 * fork/exec de programa `.ps` e empurrou a biblioteca inteira pro `e2e`.
 *
 * O que entra AQUI: o que roda em qualquer máquina, sem serviço externo —
 * hash, jwt, bytes, json, os, regex e o `psodbc` no driver sqlite (que é o
 * mesmo `ps_db.c` do postgres/mysql: muda o backend, não o buffer de
 * resultado nem o caminho de erro).
 *
 * O que NÃO entra: postgres, mysql, mongo, SMTP/IMAP e X11. Esses ficam no
 * `teste/e2e/`, e sua ausência é RELATADA — `PULOU` com o motivo —, nunca
 * silenciada.
 *
 * ORÁCULO EXTERNO, não a própria implementação: os digests de sha256 são os
 * vetores do FIPS 180-4, o base64 é o do RFC 4648, e o JWT confere contra a
 * regra do RFC 7519 (assinatura adulterada e `alg: none` têm que ser
 * recusadas). Comparar a saída com ela mesma não prova nada.
 *
 * Cada caso roda num diretório temporário próprio (o runner faz mkdtemp +
 * chdir), então caminho relativo é seguro e o caso é re-executável.
 */
#include "ps_teste.h"

const Caso CASOS_LIBS[] = {

/* ── hash: vetores do FIPS 180-4 e do RFC 4648 ───────────────────────────── */
{ "sha256 do vetor \"abc\" (FIPS 180-4)",
  "import hash\npost(hash.sha256(\"abc\"))\n",
  "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad", NULL, 0 },
{ "sha256 da string vazia (FIPS 180-4)",
  "import hash\npost(hash.sha256(\"\"))\n",
  "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855", NULL, 0 },
{ "sha256 de 448 bits (FIPS 180-4, 2o vetor)",
  "import hash\npost(hash.sha256(\"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq\"))\n",
  "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1", NULL, 0 },
{ "base64 dos vetores do RFC 4648",
  "import hash\n"
  "post(hash.b64encode(\"\"), hash.b64encode(\"f\"), hash.b64encode(\"fo\"))\n"
  "post(hash.b64encode(\"foo\"), hash.b64encode(\"foob\"), hash.b64encode(\"fooba\"))\n"
  "post(hash.b64encode(\"foobar\"))\n",
  " Zg== Zm8=\nZm9v Zm9vYg== Zm9vYmE=\nZm9vYmFy", NULL, 0 },
{ "base64 ida e volta (RFC 4648)",
  "import hash\n"
  "post(hash.b64decode(\"Zm9vYmFy\"), hash.b64decode(\"Zg==\"), hash.b64decode(\"\"))\n",
  "foobar f ", NULL, 0 },
{ "crypt/check: senha certa e errada",
  "import hash\n"
  "h = hash.crypt(\"segredo\")\n"
  "post(h.len() > 20, hash.check(h, \"segredo\"), hash.check(h, \"errada\"))\n",
  "True True False", NULL, 0 },
{ "crypt nao repete o hash (salt por chamada)",
  "import hash\npost(hash.crypt(\"x\") != hash.crypt(\"x\"))\n", "True", NULL, 0 },

/* ── jwt: RFC 7519 — o que importa é o que ele RECUSA ────────────────────── */
{ "jwt ida e volta devolve o payload",
  "import jwt\n"
  "t = jwt.gen({\"sub\": \"ana\", \"n\": 1}, \"chave\", \"HS256\")\n"
  "post(t.count(\".\") == 2, jwt.check(t, \"chave\"))\n",
  "True {'sub': 'ana', 'n': 1}", NULL, 0 },
{ "jwt com a chave errada e recusado",
  "import jwt\n"
  "t = jwt.gen({\"a\": 1}, \"certa\", \"HS256\")\n"
  "post(jwt.check(t, \"errada\"))\n",
  "null", NULL, 0 },
{ "jwt com assinatura adulterada e recusado",
  "import jwt\n"
  "t = jwt.gen({\"a\": 1}, \"k\", \"HS256\")\n"
  "p = t.split(\".\")\n"
  "post(jwt.check(p[0] + \".\" + p[1] + \".xxxxxxxx\", \"k\"))\n",
  "null", NULL, 0 },
{ "jwt com payload trocado e recusado",
  "import jwt\nimport hash\n"
  "t = jwt.gen({\"admin\": false}, \"k\", \"HS256\")\n"
  "p = t.split(\".\")\n"
  "outro = hash.b64encode(\"{\\\"admin\\\": true}\").replace(\"=\", \"\")\n"
  "post(jwt.check(p[0] + \".\" + outro + \".\" + p[2], \"k\"))\n",
  "null", NULL, 0 },

/* ── bytes: a API inteira, offline ───────────────────────────────────────── */
{ "bytes hex ida e volta",
  "import bytes\n"
  "b = bytes.fromhex(\"48656c6c6f\")\n"
  "post(bytes.hex(b), bytes.tolist(b))\n",
  "48656c6c6f [72, 101, 108, 108, 111]", NULL, 0 },
{ "bytes fromint/toint nas duas ordens",
  "import bytes\n"
  "post(bytes.hex(bytes.fromint(258, 2, \"big\")), bytes.hex(bytes.fromint(258, 2, \"little\")))\n"
  "post(bytes.toint(bytes.fromint(258, 2, \"big\"), \"big\"), bytes.toint(bytes.fromint(258, 2, \"little\"), \"little\"))\n",
  "0102 0201\n258 258", NULL, 0 },
{ "bytes xor, slice, get e concat",
  "import bytes\n"
  "a = bytes.fromhex(\"0f0f\")\n"
  "k = bytes.fromhex(\"ff00\")\n"
  "post(bytes.hex(bytes.xor(a, k)), bytes.hex(bytes.slice(a, 0, 1)), bytes.get(a, 1))\n"
  "post(bytes.hex(bytes.concat([a, k])))\n",
  "f00f 0f 15\n0f0fff00", NULL, 0 },
/* `.len()` de bytes — entrou em 28/08 porque era o ÚNICO tipo embutido sem ele.
 *
 * A regra que estes casos travam: `b.len()` tem que devolver EXATAMENTE o que
 * `len(b)` devolve, em toda origem de bytes que a VM tem. Dois números
 * diferentes pra mesma pergunta seria pior que não ter o método. */
{ "bytes.len() conta BYTE, nao codepoint",
  /* o contraste com str é o ponto: bytes não sabem o que é caractere */
  "b = \"ção\".encode()\n"
  "post(b.len(), len(b))\n"
  "post(\"ção\".len(), len(\"ção\"))\n",
  "5 5\n3 3", NULL, 0 },
{ "bytes.len() bate com len() em TODA origem de bytes",
  /* Cada linha é um jeito DIFERENTE de a VM produzir bytes. Se algum caminho
   * devolver um objeto que não é bytes de verdade, o método some e o caso
   * falha aqui em vez de falhar num `try` de alguém. */
  "import bytes\n"
  "import hash\n"
  "casos = [\n"
  "    \"\".encode(),\n"
  "    \"abc\".encode(),\n"
  "    \"ção\".encode(),\n"
  "    (\"x\" * 1000).encode(),\n"
  "    bytes.fromhex(\"48656c6c6f\"),\n"
  "    bytes.fromint(258, 4, \"big\"),\n"
  "    bytes.frombase64(\"Dw8=\"),\n"
  "    bytes.slice(\"abcdef\".encode(), 1, 4),\n"
  "    bytes.concat([\"ab\".encode(), \"cd\".encode()]),\n"
  "    bytes.xor(\"ab\".encode(), \"cd\".encode()),\n"
  "    bytes.new(7),\n"
  "]\n"
  "difere = []\n"
  "tam = []\n"
  "for each b in casos {\n"
  "    addEnd(tam, b.len())\n"
  "    if b.len() != len(b) {\n"
  "        addEnd(difere, str(b.len()) + \"!=\" + str(len(b)))\n"
  "    }\n"
  "}\n"
  "post(tam)\n"
  "post(\"divergiram:\", difere)\n",
  "[0, 3, 5, 1000, 5, 4, 2, 3, 4, 2, 7]\ndivergiram: []", NULL, 0 },
{ "bytes.len() recusa argumento, como os outros len()",
  "try {\n"
  "    post(\"ab\".encode().len(1))\n"
  "} catch (e) {\n"
  "    post(\"recusou\")\n"
  "}\n",
  "recusou", NULL, 0 },
{ "os cinco tipos embutidos respondem .len()",
  /* O caso existe pra a lacuna nao voltar: se alguem criar um tipo novo e
   * esquecer o .len(), esta lista e onde se ve. */
  "post(\"ab\".len(), [1, 2, 3].len(), {\"a\": 1}.len(), (1, 2).len(),\n"
  "     \"abcd\".encode().len())\n",
  "2 3 1 2 4", NULL, 0 },
{ "bytes.len() de um arquivo lido em binario",
  /* PoolFile/loadFile é outra origem de bytes, por caminho diferente */
  "import os\n"
  "using open(\"/tmp/ps_blen.bin\", \"wb\") as f {\n"
  "    f.write(\"abcde\".encode())\n"
  "}\n"
  "b = open(\"/tmp/ps_blen.bin\", \"rb\").read()\n"
  "post(b.len(), len(b), b.len() == len(b))\n"
  "os.cmd(\"rm -f /tmp/ps_blen.bin\")\n",
  "5 5 True", NULL, 0 },

{ "bytes base64 ida e volta",
  "import bytes\n"
  "a = bytes.fromhex(\"0f0f\")\n"
  "post(bytes.base64(a), bytes.hex(bytes.frombase64(\"Dw8=\")))\n",
  "Dw8= 0f0f", NULL, 0 },

/* ── psodbc no driver sqlite: é o MESMO ps_db.c do postgres/mysql ────────── */
{ "psodbc sqlite: cria, insere com parametro e le",
  "import psodbc\n"
  "c = psodbc.connect(driver=\"sqlite\", base=\"t.db\")\n"
  "cur = c.cursor()\n"
  "cur.execute(\"create table t (a int, b text)\")\n"
  "cur.execute(\"insert into t values (?, ?)\", (1, \"um\"))\n"
  "cur.execute(\"insert into t values (?, ?)\", (2, \"dois\"))\n"
  "c.commit()\n"
  "cur.execute(\"select a, b from t order by a\")\n"
  "post(cur.fetchall())\n"
  "cur.close()\n"
  "c.close()\n",
  "[{'a': 1, 'b': 'um'}, {'a': 2, 'b': 'dois'}]", NULL, 0 },
{ "psodbc sqlite: fetchone e fetchmany",
  "import psodbc\n"
  "c = psodbc.connect(driver=\"sqlite\", base=\"t.db\")\n"
  "cur = c.cursor()\n"
  "cur.execute(\"create table t (a int)\")\n"
  "cur.execute(\"insert into t values (1)\")\n"
  "cur.execute(\"insert into t values (2)\")\n"
  "cur.execute(\"insert into t values (3)\")\n"
  "c.commit()\n"
  "cur.execute(\"select a from t order by a\")\n"
  "post(cur.fetchone())\n"
  "cur.execute(\"select a from t order by a\")\n"
  "post(cur.fetchmany(2))\n"
  "c.close()\n",
  "{'a': 1}\n[{'a': 1}, {'a': 2}]", NULL, 0 },
{ "psodbc sqlite: SQL invalido levanta, nao devolve calado",
  "import psodbc\n"
  "c = psodbc.connect(driver=\"sqlite\", base=\"t.db\")\n"
  "cur = c.cursor()\n"
  "try {\n"
  "    cur.execute(\"select * from nao_existe\")\n"
  "    post(\"passou calado\")\n"
  "}\n"
  "catch (e) {\n"
  "    post(\"recusou\")\n"
  "}\n"
  "c.close()\n",
  "recusou", NULL, 0 },
{ "psodbc sqlite: null do banco vira null da linguagem",
  "import psodbc\n"
  "c = psodbc.connect(driver=\"sqlite\", base=\"t.db\")\n"
  "cur = c.cursor()\n"
  "cur.execute(\"create table t (a int, b text)\")\n"
  "cur.execute(\"insert into t (a) values (1)\")\n"
  "c.commit()\n"
  "cur.execute(\"select a, b from t\")\n"
  "post(cur.fetchall())\n"
  "c.close()\n",
  "[{'a': 1, 'b': null}]", NULL, 0 },

/* ── sqlite3: o outro caminho (nativo, dentro do poolscript_vm.c) ────────── */
{ "sqlite3: cria, insere e le",
  "import sqlite3\n"
  "c = sqlite3.connect(\"s.db\")\n"
  "cur = c.cursor()\n"
  "cur.execute(\"create table t (a int)\")\n"
  "cur.execute(\"insert into t values (?)\", (7,))\n"
  "c.commit()\n"
  "cur.execute(\"select a from t\")\n"
  "post(cur.fetchall())\n"
  "c.close()\n",
  "[{'a': 7}]", NULL, 0 },

/* ── os: arquivo de verdade, no diretorio temporario do caso ─────────────── */
{ "os: escreve, le, tamanho e existencia",
  "import os\n"
  "os.writeFile(\"a.txt\", \"oi\")\n"
  "post(os.exists(\"a.txt\"), os.isfile(\"a.txt\"), os.isdir(\"a.txt\"), os.size(\"a.txt\"))\n"
  "post(os.readFile(\"a.txt\"))\n",
  "True True False 2\noi", NULL, 0 },
{ "os: copia, move e apaga",
  "import os\n"
  "os.writeFile(\"a.txt\", \"x\")\n"
  "os.copy(\"a.txt\", \"b.txt\")\n"
  "os.move(\"b.txt\", \"c.txt\")\n"
  "post(os.exists(\"a.txt\"), os.exists(\"b.txt\"), os.exists(\"c.txt\"))\n",
  "True False True", NULL, 0 },
{ "os: mkdir, ls e rmdir",
  "import os\n"
  "os.mkdir(\"d\")\n"
  "os.writeFile(\"d/x.txt\", \"1\")\n"
  "itens = os.ls(\"d\")\n"
  "post(itens.len(), itens[0][\"name\"], itens[0][\"type\"])\n"
  "os.rmdir(\"d\", force=true)\n"
  "post(os.exists(\"d\"))\n",
  "1 x.txt file\nFalse", NULL, 0 },
{ "os: ler arquivo inexistente levanta",
  "import os\n"
  "try {\n"
  "    os.readFile(\"nao_existe.txt\")\n"
  "    post(\"passou calado\")\n"
  "}\n"
  "catch (e) {\n"
  "    post(\"recusou\")\n"
  "}\n",
  "recusou", NULL, 0 },

/* ── json: o teto de profundidade é contrato, nos DOIS sentidos ──────────── */
{ "json ida e volta",
  "import json\n"
  "post(json.stringify({\"a\": [1, 2], \"b\": null}))\n"
  "post(json.parse(\"{\\\"a\\\": [1, 2], \\\"b\\\": null}\"))\n",
  "{\"a\": [1, 2], \"b\": null}\n{'a': [1, 2], 'b': null}", NULL, 0 },
{ "json aninhado demais e recusado na leitura",
  "import json\n"
  "post(json.parse(\"[\" * 200 + \"]\" * 200))\n",
  NULL, "aninhado demais", -1 },
{ "json invalido levanta",
  "import json\n"
  "try {\n"
  "    json.parse(\"{nao e json}\")\n"
  "    post(\"passou calado\")\n"
  "}\n"
  "catch (e) {\n"
  "    post(\"recusou\")\n"
  "}\n",
  "recusou", NULL, 0 },

/* ── regex: compilado, e o teto de backtracking ──────────────────────────── */
{ "regex compile, fullmatch e findall",
  "import regex\n"
  "p = regex.compile(\"[0-9]+\")\n"
  "post(p.fullmatch(\"123\") != null, p.fullmatch(\"12a\") != null, p.findall(\"a1b22c333\"))\n",
  "True False ['1', '22', '333']", NULL, 0 },
{ "regex sub e split",
  "import regex\n"
  "post(regex.sub(\"[0-9]+\", \"#\", \"a1b22\"), regex.split(\",\", \"a,b,c\"))\n",
  "a#b# ['a', 'b', 'c']", NULL, 0 },
{ "regex com backtracking explosivo para, nao trava",
  "import regex\n"
  "post(regex.match(\"(a+)+$\", \"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa!\"))\n",
  NULL, "backtracking", -1 },
};

const int NC_LIBS = N_CASOS(CASOS_LIBS);
