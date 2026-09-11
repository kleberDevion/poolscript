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

/* ── save= : o corpo vai pro DISCO, nao pra memoria (2026-09-11) ───────────
 * `request.get(url)` guarda o corpo inteiro: baixar 237 MB custava 480 MB de
 * RSS, porque o corpo vira `char*` e depois vira string da linguagem — existe
 * duas vezes. Com `save=` o pedaco vai direto pro arquivo e nada acumula:
 * o mesmo download passou a 17,6 MB de pico.
 *
 * O primeiro conserto tinha um SIGSEGV: `ncorpo` guardava o total BAIXADO
 * enquanto `corpo` era uma string vazia, e o `monta_response` fazia
 * `memcpy(destino, corpo, ncorpo)` — 64 MB lidos de um buffer de 1 byte.
 * Com corpo pequeno nem sinal havia: o `.content` voltava com heap do proprio
 * processo (medido: 4096 bytes de ponteiros e lixo em vez dos dados). Agora
 * `ncorpo` descreve SEMPRE o que esta no buffer, e o total fica em `nbaixado`.
 * O caso abaixo e o que pegaria a volta do vazamento. */
{ "save= com conexao recusada devolve NetworkError, sem crash",
  "import request\nimport os\n"
  "r = request.get(\"http://127.0.0.1:1/x\", save=\"s.bin\")\n",
  "", "NetworkError", 1 },
{ "save= que nao pode ser criado diz o caminho, e nao envia nada",
  "import request\n"
  "request.get(\"http://127.0.0.1:1/x\", save=\"/pasta/que/nao/existe/s.bin\")\n",
  "", "No such file or directory", 1 },
{ "save= exige str",
  "import request\nrequest.get(\"http://127.0.0.1:1/x\", save=7)\n",
  "", "save must be str, not int", 1 },
/* Sem save=, o motor cria o arquivo do corpo ANTES de conectar (mkstemp na
 * pasta corrente). Conexao recusada tem que apaga-lo: o runner roda cada caso
 * numa pasta nova, entao a contagem so pode ser zero. */
{ "sem save=, conexao recusada nao deixa arquivo do motor na pasta",
  "import os\nimport request\n"
  "try { request.get(\"http://127.0.0.1:1/x\") } catch (e) { pass }\n"
  "post(os.cmd(\"ls -a . | grep -c ps_resposta_\", true).strip())\n",
  "0", NULL, 0 },



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
  "Null", NULL, 0 },
{ "jwt com assinatura adulterada e recusado",
  "import jwt\n"
  "t = jwt.gen({\"a\": 1}, \"k\", \"HS256\")\n"
  "p = t.split(\".\")\n"
  "post(jwt.check(p[0] + \".\" + p[1] + \".xxxxxxxx\", \"k\"))\n",
  "Null", NULL, 0 },
{ "jwt com payload trocado e recusado",
  "import jwt\nimport hash\n"
  "t = jwt.gen({\"admin\": false}, \"k\", \"HS256\")\n"
  "p = t.split(\".\")\n"
  "outro = hash.b64encode(\"{\\\"admin\\\": true}\").replace(\"=\", \"\")\n"
  "post(jwt.check(p[0] + \".\" + outro + \".\" + p[2], \"k\"))\n",
  "Null", NULL, 0 },

/* ── taxonomia de erro: os tipos que o motor levanta ─────────────────────────
 *
 * Três decisões de API tomadas em 28/08 (notas/DECISOES-API.md), travadas aqui
 * porque o TIPO do erro é contrato: quem escreve `catch (X e)` depende dele. */
{ "divisao por zero tem tipo proprio, nao o balde geral",
  /* I8: era `TypeError: divisão por zero: division by zero` — tipo que
   * não dizia nada sobre divisão, e mensagem duplicada em dois idiomas. */
  "try {\n"
  "    post(1 / 0)\n"
  "} catch (ZeroDivisionError e) {\n"
  "    post(\"pegou pelo tipo\")\n"
  "} catch (e) {\n"
  "    post(\"caiu no generico\")\n"
  "}\n", "pegou pelo tipo", NULL, 0 },
{ "resto por zero tambem, inteiro e float",
  "n = 0\n"
  "for each e in [\"1 % 0\", \"1.5 % 0.0\", \"1 / 0\", \"7 / 0.0\"] {\n"
  "    pass\n"
  "}\n"
  "try { post(1 % 0) } catch (ZeroDivisionError e) { n = n + 1 }\n"
  "try { post(1.5 % 0.0) } catch (ZeroDivisionError e) { n = n + 1 }\n"
  "try { post(7 / 0.0) } catch (ZeroDivisionError e) { n = n + 1 }\n"
  "post(n)\n", "3", NULL, 0 },
{ "a mensagem de tipo incompativel diz QUAIS tipos",
  /* I14: era só "'+' entre tipos incompativeis" — o usuário tinha que adivinhar
   * qual dos dois lados estava errado. Hoje o texto é o do CPython, que ainda
   * nomeia os dois e ainda diz qual é o estranho: `can only concatenate str
   * (not "int") to str`. */
  "try {\n"
  "    post(\"a\" + 1)\n"
  "} catch (e) {\n"
  "    post(\"can only concatenate str (not \\\"int\\\") to str\" in str(e))\n"
  "}\n", "True", NULL, 0 },
{ "os cinco operadores nomeiam os tipos",
  /* O texto virou o do CPython, e ele não é um só: sequência à esquerda tem
   * frase própria em `+` e em `*`. O que este caso cobra continua sendo o
   * mesmo — que os DOIS tipos apareçam, com o nome que o `type()` devolve. */
  "n = 0\n"
  "try { post(\"a\" + 1) } catch (e) { if \"str\" in str(e) and \"int\" in str(e) { n = n + 1 } }\n"
  "try { post(1 - \"a\") } catch (e) { if \"'int' and 'str'\" in str(e) { n = n + 1 } }\n"
  "try { post([1] * \"a\") } catch (e) { if \"non-int of type 'str'\" in str(e) { n = n + 1 } }\n"
  "try { post(\"a\" / 2) } catch (e) { if \"'str' and 'int'\" in str(e) { n = n + 1 } }\n"
  "try { post(\"a\" % []) } catch (e) { if \"'str' and 'list'\" in str(e) { n = n + 1 } }\n"
  "post(n)\n", "5", NULL, 0 },
{ "AttributedValueError escrito certo",
  /* I7: o tipo era publicado com a grafia errada (`Atributted`).
   *
   * O caso mudou de EXPRESSÃO, não de intenção: `"a" + 1` levantava
   * AttributedValueError, e não devia — somar tipos que não somam é TypeError,
   * como no Python. AttributedValueError ficou com o que lhe cabe, que é
   * atribuir valor incompatível a variável TIPADA. É isso que ele prova
   * agora, e a grafia continua cobrada. */
  "try {\n"
  "    str x = 10\n"
  "} catch (AttributedValueError e) {\n"
  "    post(\"pegou\")\n"
  "}\n", "pegou", NULL, 0 },
{ "'+' entre tipos que nao somam e TypeError, nao AttributedValueError",
  "try {\n"
  "    post(\"a\" + 1)\n"
  "} catch (TypeError e) {\n"
  "    post(\"pegou\")\n"
  "}\n", "pegou", NULL, 0 },
{ "pool --check sai != 0 quando o arquivo nao compila",
  /* I16: saía 0 SEMPRE, então `pool --check f.ps || exit 1` nunca disparava —
   * e o --check roda no editor a cada tecla e no `psl install` de pacote de
   * terceiro. */
  "import os\n"
  "import sys\n"
  "using open(\"quebrado.ps\", \"w\") as f {\n"
  "    f.write(\"post(\\n\")\n"
  "}\n"
  "using open(\"bom.ps\", \"w\") as f {\n"
  "    f.write(\"post(1)\\n\")\n"
  "}\n"
  "os.cmd(\"'\" + sys.executable + \"' --check quebrado.ps > /dev/null 2>&1; echo $? > rq\")\n"
  "os.cmd(\"'\" + sys.executable + \"' --check bom.ps > /dev/null 2>&1; echo $? > rb\")\n"
  "post(int(open(\"rq\").read().strip()) != 0, int(open(\"rb\").read().strip()) == 0)\n",
  "True True", NULL, 0 },

/* ── bytes: os 42 métodos do VALOR, conferidos contra o CPython ───────────
 *
 * `str(e).split(" (linha ")[0]` aparece em todo catch daqui: `catch (e)` liga
 * `e` a uma STRING que já traz " (linha N)" no fim. O que se compara com o
 * CPython é a MENSAGEM; prender o número da linha no esperado faria o caso
 * reprovar por alguém ter inserido um comentário acima dele.
 *
 * POR QUE ESTES CASOS EXISTEM: `bytes` tinha três métodos — `decode`, `hex` e
 * `len`. Faltava tudo o que serve pra CONFERIR conteúdo binário: `find`,
 * `startswith`, `split`, `count`, e o `in`. Escrevendo o teste do vazamento
 * de pilha do jinker foi preciso converter a resposta inteira pra hexadecimal
 * e varrer a string de dois em dois bytes, porque `0 in resposta` não existia.
 * Conferência que precisa de rodeio é conferência que ninguém escreve.
 *
 * O ORÁCULO É O CPYTHON, não a nossa implementação. Cada esperado abaixo foi
 * colhido rodando o mesmo caso no `python3` — os 114 casos da varredura saíram
 * byte a byte iguais. Comparar a saída com ela mesma não prova nada.
 *
 * As diferenças entre `bytes` e `str` que estes casos travam, porque são as
 * que se erra copiando o método do str:
 *
 *   - caixa (`upper`/`lower`/`title`) mexe SÓ no ASCII;
 *   - `splitlines` quebra em \n, \r e \r\n e MAIS NADA;
 *   - `find`/`count`/`index` aceitam um INTEIRO de 0 a 255;
 *   - `strip(chars)` é CONJUNTO de bytes, não prefixo;
 *   - indexar dá inteiro, fatiar dá bytes, iterar dá inteiro.
 */
{ "bytes: find/rfind/index/count, com bytes e com inteiro",
  "b = \"Hello, World\".encode()\n"
  "post(b.find(\"o\".encode()), b.find(111), b.find(\"zz\".encode()))\n"
  "post(b.find(\"o\".encode(), 5), b.find(\"o\".encode(), 0, 5), b.rfind(\"o\".encode()))\n"
  "post(b.index(\"o\".encode()), b.count(108), \"aaaa\".encode().count(\"aa\".encode()))\n"
  /* agulha vazia conta POSIÇÕES, que são len+1 — é o que o Python faz */
  "post(\"aaaa\".encode().count(\"\".encode()), \"\".encode().count(\"\".encode()))\n",
  "4 4 -1\n8 4 8\n4 3 2\n5 1", NULL, 0 },
{ "bytes: index sem achar levanta com a frase do CPython",
  /* a frase do bytes NÃO é a do str: "subsection not found", não "substring" */
  "try {\n"
  "    \"abc\".encode().index(\"zz\".encode())\n"
  "} catch (ValueError e) {\n"
  "    post(str(e).split(\" (linha \")[0])\n"
  "}\n"
  "try {\n"
  "    \"abc\".encode().find(\"z\")\n"
  "} catch (TypeError e) {\n"
  "    post(str(e).split(\" (linha \")[0])\n"
  "}\n"
  "try {\n"
  "    \"abc\".encode().find(300)\n"
  "} catch (ValueError e) {\n"
  "    post(str(e).split(\" (linha \")[0])\n"
  "}\n",
  "subsection not found\n"
  "argument should be integer or bytes-like object, not 'str'\n"
  "byte must be in range(0, 256)", NULL, 0 },
{ "bytes: startswith/endswith com tupla e com faixa",
  "b = \"abcdef\".encode()\n"
  "post(b.startswith(\"abc\".encode()), b.endswith(\"def\".encode()))\n"
  "post(b.startswith((\"x\".encode(), \"ab\".encode())))\n"
  "post(b.startswith(\"cd\".encode(), 2), b.endswith(\"cd\".encode(), 0, 4))\n"
  "try {\n"
  "    b.startswith(\"a\")\n"
  "} catch (TypeError e) {\n"
  "    post(str(e).split(\" (linha \")[0])\n"
  "}\n",
  "True True\nTrue\nTrue True\n"
  "startswith first arg must be bytes or a tuple of bytes, not str", NULL, 0 },
{ "bytes: caixa mexe SO no ASCII",
  /* 0xc0/0xe0 são À/à em latin-1 e o `str` do Python os trocaria; o `bytes`
   * NÃO, e copiar o método do str aqui corromperia dado binário */
  "import bytes\n"
  "post(\"hello\".encode().upper(), \"HELLO\".encode().lower())\n"
  "post(\"hello wOrld 3ab\".encode().title())\n"
  "post(\"hELLO\".encode().capitalize(), \"Hello\".encode().swapcase())\n"
  "post(bytes.fromhex(\"c0e0\").upper() == bytes.fromhex(\"c0e0\"))\n",
  "b'HELLO' b'hello'\nb'Hello World 3Ab'\nb'Hello' b'hELLO'\nTrue", NULL, 0 },
{ "bytes: predicados, e o vazio",
  /* vazio é False em todos MENOS no isascii — detalhe do Python que só
   * aparece quando alguém passa uma resposta vazia */
  "import bytes\n"
  "v = \"\".encode()\n"
  "post(\"abc\".encode().isalpha(), \"123\".encode().isdigit(), \"a1\".encode().isalnum())\n"
  "post(\" \\t\".encode().isspace(), \"ABC\".encode().isupper(), \"abc\".encode().islower())\n"
  "post(\"123\".encode().isupper(), \"Hello World\".encode().istitle(), \"A1b\".encode().istitle())\n"
  "post(v.isalpha(), v.isdigit(), v.isupper(), v.istitle())\n"
  "post(v.isascii(), \"abc\".encode().isascii(), bytes.fromhex(\"80\").isascii())\n",
  "True True True\nTrue True True\nFalse True False\n"
  "False False False False\nTrue True False", NULL, 0 },
{ "bytes: strip e o CONJUNTO de bytes",
  /* `chars` é conjunto, não prefixo: b\"xyaXbyx\".strip(b\"xy\") é b\"aXb\" */
  "post(\"  \\t a b \\n \".encode().strip())\n"
  "post(\"xyaXbyx\".encode().strip(\"xy\".encode()))\n"
  "post(\"xyaXbyx\".encode().lstrip(\"xy\".encode()), \"xyaXbyx\".encode().rstrip(\"xy\".encode()))\n"
  "post(\"abc\".encode().strip(\"\".encode()))\n"
  "post(\"Hello\".encode().removeprefix(\"He\".encode()), \"Hello\".encode().removeprefix(\"zz\".encode()))\n",
  "b'a b'\nb'aXb'\nb'aXbyx' b'xyaXb'\nb'abc'\nb'llo' b'Hello'", NULL, 0 },
{ "bytes: split, rsplit e o separador vazio",
  "post(\"a-b-c\".encode().split(\"-\".encode()))\n"
  "post(\"a-b-c\".encode().split(\"-\".encode(), 1), \"a-b-c\".encode().rsplit(\"-\".encode(), 1))\n"
  "post(\"  a  b \\t c \\n \".encode().split())\n"
  "post(\" a b c \".encode().split(Null, 1), \" a b c \".encode().rsplit(Null, 1))\n"
  "try {\n"
  "    \"abc\".encode().split(\"\".encode())\n"
  "} catch (ValueError e) {\n"
  "    post(str(e).split(\" (linha \")[0])\n"
  "}\n",
  "[b'a', b'b', b'c']\n"
  "[b'a', b'b-c'] [b'a-b', b'c']\n"
  "[b'a', b'b', b'c']\n"
  "[b'a', b'b c '] [b' a b', b'c']\n"
  "empty separator", NULL, 0 },
{ "bytes: splitlines quebra so em \\n, \\r e \\r\\n",
  /* o `str` também quebra em \\v e \\f; o `bytes` NÃO, e copiar o do str daria
   * linha a mais em dado binário */
  "import bytes\n"
  "post(\"a\\nb\\r\\nc\".encode().splitlines())\n"
  "post(\"a\\nb\".encode().splitlines(true))\n"
  "post(bytes.fromhex(\"610b620c63\").splitlines())\n",
  "[b'a', b'b', b'c']\n[b'a\\n', b'b']\n[b'a\\x0bb\\x0cc']", NULL, 0 },
{ "bytes: partition, rpartition e o lado em que sobra",
  /* sem achar, `partition` põe tudo no PRIMEIRO e `rpartition` no ÚLTIMO */
  "post(\"a=b=c\".encode().partition(\"=\".encode()))\n"
  "post(\"a=b=c\".encode().rpartition(\"=\".encode()))\n"
  "post(\"abc\".encode().partition(\"=\".encode()))\n"
  "post(\"abc\".encode().rpartition(\"=\".encode()))\n",
  "(b'a', b'=', b'b=c')\n(b'a=b', b'=', b'c')\n"
  "(b'abc', b'', b'')\n(b'', b'', b'abc')", NULL, 0 },
{ "bytes: join e replace, inclusive com agulha vazia",
  /* agulha vazia no replace enfia a troca entre cada byte E nas duas pontas */
  "post(\"-\".encode().join([\"a\".encode(), \"b\".encode(), \"c\".encode()]))\n"
  "post(\"\".encode().join([\"a\".encode(), \"b\".encode()]))\n"
  "post(\"aaa\".encode().replace(\"a\".encode(), \"b\".encode()))\n"
  "post(\"aaa\".encode().replace(\"a\".encode(), \"b\".encode(), 2))\n"
  "post(\"abc\".encode().replace(\"\".encode(), \"-\".encode()))\n"
  "try {\n"
  "    \"\".encode().join([\"a\".encode(), 1])\n"
  "} catch (TypeError e) {\n"
  "    post(str(e).split(\" (linha \")[0])\n"
  "}\n",
  "b'a-b-c'\nb'ab'\nb'bbb'\nb'bba'\nb'-a-b-c-'\n"
  "sequence item 1: expected a bytes-like object, int found", NULL, 0 },
{ "bytes: ljust, rjust, center, zfill e expandtabs",
  /* no `center`, com sobra ímpar E largura ímpar, o byte a mais fica na
   * ESQUERDA — b\"ab\".center(7) é b\"***ab**\", não b\"**ab***\" */
  "post(\"ab\".encode().ljust(5, \".\".encode()), \"ab\".encode().rjust(5, \".\".encode()))\n"
  "post(\"ab\".encode().center(7, \"*\".encode()))\n"
  "post(\"abcdef\".encode().ljust(3))\n"
  "post(\"42\".encode().zfill(8), \"-42\".encode().zfill(8), \"abc\".encode().zfill(2))\n"
  "post(\"a\\tbc\\td\".encode().expandtabs(4))\n"
  "try {\n"
  "    \"ab\".encode().center(20, \"xy\".encode())\n"
  "} catch (TypeError e) {\n"
  "    post(str(e).split(\" (linha \")[0])\n"
  "}\n",
  "b'ab...' b'...ab'\nb'***ab**'\nb'abcdef'\n"
  "b'00000042' b'-0000042' b'abc'\nb'a   bc  d'\n"
  "center() argument 2 must be a byte string of length 1, not bytes", NULL, 0 },
{ "bytes: maketrans e translate, com e sem delete",
  "t = \"\".encode().maketrans(\"abc\".encode(), \"xyz\".encode())\n"
  "post(t.len(), \"abcabc\".encode().translate(t))\n"
  "post(\"abcabc\".encode().translate(Null, \"b\".encode()))\n"
  "try {\n"
  "    \"abc\".encode().translate(\"ab\".encode())\n"
  "} catch (ValueError e) {\n"
  "    post(str(e).split(\" (linha \")[0])\n"
  "}\n"
  "try {\n"
  "    \"\".encode().maketrans(\"a\".encode(), \"bc\".encode())\n"
  "} catch (ValueError e) {\n"
  "    post(str(e).split(\" (linha \")[0])\n"
  "}\n",
  "256 b'xyzxyz'\nb'acac'\n"
  "translation table must be 256 characters long\n"
  "maketrans arguments must have same length", NULL, 0 },
{ "bytes: hex com separador e agrupamento",
  /* `n` positivo agrupa da DIREITA, negativo da ESQUERDA — a regra do Python,
   * porque número em hexadecimal se alinha pelo dígito menos significativo */
  "import bytes\n"
  "b = bytes.fromhex(\"01020304050607\")\n"
  "post(b.hex())\n"
  "post(b.hex(\"_\", 3))\n"
  "post(b.hex(\"_\", -3))\n"
  "post(bytes.fromhex(\"deadbeef\").hex(\"-\"))\n"
  "try {\n"
  "    b.hex(\"--\")\n"
  "} catch (ValueError e) {\n"
  "    post(str(e).split(\" (linha \")[0])\n"
  "}\n",
  "01020304050607\n01_020304_050607\n010203_040506_07\nde-ad-be-ef\n"
  "sep must be length 1.", NULL, 0 },
{ "bytes: os operadores de sequencia",
  /* Indexar dá INTEIRO e fatiar dá bytes; iterar dá INTEIRO. É o que faz
   * `if x == 0` funcionar direto pra procurar byte NUL num corpo de resposta —
   * antes disso a única saída era converter tudo pra hexadecimal. */
  "b = \"Hello\".encode()\n"
  "post(len(b), b[0], b[0:2], b[0:1], b[-1])\n"
  "post(b + \"!\".encode(), b * 2, 2 * \"ab\".encode(), \"ab\".encode() * 0)\n"
  "post(\"ell\".encode() in b, 101 in b, 1 in b)\n"
  "post(\"abc\".encode() < \"abd\".encode(), \"ab\".encode() < \"abc\".encode())\n"
  "post(sorted([\"b\".encode(), \"a\".encode()]))\n"
  "vs = []\n"
  "for each x in \"abc\".encode() {\n"
  "    addEnd(vs, x)\n"
  "}\n"
  "post(vs, list(\"abc\".encode()))\n",
  "5 72 b'He' b'H' 111\n"
  "b'Hello!' b'HelloHello' b'abab' b''\n"
  "True True False\nTrue True\n[b'a', b'b']\n"
  "[97, 98, 99] [97, 98, 99]", NULL, 0 },
{ "bytes: os operadores recusam com a frase do CPython",
  "b = \"ab\".encode()\n"
  "try {\n"
  "    post(b * \"x\")\n"
  "} catch (TypeError e) {\n"
  "    post(str(e).split(\" (linha \")[0])\n"
  "}\n"
  "try {\n"
  "    post(b < \"x\")\n"
  "} catch (TypeError e) {\n"
  "    post(str(e).split(\" (linha \")[0])\n"
  "}\n"
  "try {\n"
  "    post(\"x\" in b)\n"
  "} catch (TypeError e) {\n"
  "    post(str(e).split(\" (linha \")[0])\n"
  "}\n"
  "try {\n"
  "    post(300 in b)\n"
  "} catch (ValueError e) {\n"
  "    post(str(e).split(\" (linha \")[0])\n"
  "}\n",
  "can't multiply sequence by non-int of type 'str'\n"
  "'<' not supported between instances of 'bytes' and 'str'\n"
  "a bytes-like object is required, not 'str'\n"
  "byte must be in range(0, 256)", NULL, 0 },
{ "bytes: procurar byte NUL sem rodeio",
  /* O caso de uso que fez tudo isto existir: conferir se um corpo de resposta
   * tem byte de memória vazada. Antes só dava pra fazer em hexadecimal, de
   * dois em dois caracteres. As três formas abaixo têm que concordar. */
  "import bytes\n"
  "resp = \"ok\".encode() + bytes.fromhex(\"00\") + \"lixo\".encode()\n"
  "limpa = \"ok, sem nada estranho\".encode()\n"
  "porin = [0 in resp, 0 in limpa]\n"
  "porfind = [resp.find(0) >= 0, limpa.find(0) >= 0]\n"
  "porlaco = []\n"
  "for each alvo in [resp, limpa] {\n"
  "    achou = false\n"
  "    for each x in alvo {\n"
  "        if x == 0 {\n"
  "            achou = true\n"
  "        }\n"
  "    }\n"
  "    addEnd(porlaco, achou)\n"
  "}\n"
  "post(porin, porfind, porlaco)\n"
  "post(porin == porfind and porfind == porlaco)\n",
  "[True, False] [True, False] [True, False]\nTrue", NULL, 0 },

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
  "[{'a': 1, 'b': Null}]", NULL, 0 },

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
  /* A primeira linha e JSON e mantem `null` minusculo — e a especificacao do
   * formato. A segunda e o `post` da LINGUAGEM, que imprime `Null`. Este caso
   * e o que trava a distincao: se alguem uniformizar os dois, ele reprova. */
  "{\"a\": [1, 2], \"b\": null}\n{'a': [1, 2], 'b': Null}", NULL, 0 },
{ "json aninhado demais e recusado na leitura",
  "import json\n"
  "post(json.parse(\"[\" * 200 + \"]\" * 200))\n",
  "", "aninhado demais", 1 },
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
  /* `fullmatch` devolve BOOL, nao objeto de match. Este caso comparava
   * `bool != null` e passava porque `false == null` era True — passava por
   * ACIDENTE, e so apareceu quando Null virou igual so a Null. Agora confere
   * o booleano direto, que e o contrato de verdade. */
  "post(p.fullmatch(\"123\"), p.fullmatch(\"12a\"), p.findall(\"a1b22c333\"))\n",
  "True False ['1', '22', '333']", NULL, 0 },
{ "regex sub e split",
  "import regex\n"
  "post(regex.sub(\"[0-9]+\", \"#\", \"a1b22\"), regex.split(\",\", \"a,b,c\"))\n",
  "a#b# ['a', 'b', 'c']", NULL, 0 },
{ "regex com backtracking explosivo para, nao trava",
  "import regex\n"
  "post(regex.match(\"(a+)+$\", \"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa!\"))\n",
  "", "RuntimeError: regex: backtracking demais", 1 },

/* ── as mensagens sao as do CPython, e os caminhos CONCORDAM ─────────────────
 *
 * Estes casos nao guardam a saida de ontem: cada um trava uma INVARIANTE que
 * ja foi violada de verdade. O que eles cobram:
 *
 *   1. o erro acusa o tipo que o usuario ESCREVEU (nao o que a VM coagiu);
 *   2. tipo de excecao novo e capturavel pelo nome;
 *   3. caminhos diferentes do mesmo erro dao a MESMA resposta.
 *
 * O item 3 e o que mais reincidiu: o `range` discordava de si mesmo entre o
 * builtin e o `for each`; o `KeyError` tinha tres redacoes; o desempacotamento
 * com estrela e sem estrela usavam tipos de excecao diferentes. */

{ "o operador acusa o tipo ESCRITO, nao o coagido",
  /* `true - "a"` respondia `int - str`. O bool vira int antes da mensagem, e
   * quem lia procurava um int que nao existe no codigo. */
  "try {\n"
  "    post(true - \"a\")\n"
  "} catch (e) {\n"
  "    post(\"bool\" in str(e))\n"
  "}\n", "True", NULL, 0 },

{ "comparar lista aninhada culpa o par de DENTRO",
  /* dizia \"tipos incompativeis\" das duas LISTAS, que se comparam muito bem */
  "try {\n"
  "    post([1] < [\"a\"])\n"
  "} catch (e) {\n"
  "    post(\"'int' and 'str'\" in str(e))\n"
  "}\n", "True", NULL, 0 },

{ "NameError e capturavel pelo nome",
  "try {\n"
  "    post(nao_existe_mesmo)\n"
  "} catch (NameError e) {\n"
  "    post(str(e).startswith(\"name 'nao_existe_mesmo' is not defined\"))\n"
  "}\n", "True", NULL, 0 },

{ "AttributeError e capturavel pelo nome",
  "try {\n"
  "    post(\"abc\".nao_existe_mesmo())\n"
  "} catch (AttributeError e) {\n"
  "    post(\"'str' object has no attribute 'nao_existe_mesmo'\" in str(e))\n"
  "}\n", "True", NULL, 0 },

{ "OverflowError e capturavel pelo nome",
  "try {\n"
  "    post(int(flo(\"inf\")))\n"
  "} catch (OverflowError e) {\n"
  "    post(\"cannot convert flo infinity to integer\" in str(e))\n"
  "}\n", "True", NULL, 0 },

{ "os TRES caminhos de chave ausente dao a mesma resposta",
  /* `d[\"z\"]`, `d.z` e `d.pop(\"z\")`: um dia foram tres redacoes diferentes */
  "d = { \"a\": 1 }\n"
  "vistos = []\n"
  "try { post(d[\"z\"]) } catch (e) { addEnd(vistos, str(e).split(\" (\")[0]) }\n"
  "try { post(d.z) } catch (e) { addEnd(vistos, str(e).split(\" (\")[0]) }\n"
  "try { post(d.pop(\"z\")) } catch (e) { addEnd(vistos, str(e).split(\" (\")[0]) }\n"
  "post(len(vistos), vistos[0] == vistos[1] and vistos[1] == vistos[2], vistos[0])\n",
  "3 True 'z'", NULL, 0 },

{ "range com passo 0 e o MESMO erro no builtin e no for each",
  /* saia TypeError num caminho e ValueError no outro */
  "a = \"\"\n"
  "b = \"\"\n"
  "try { post(range(1, 5, 0)) } catch (ValueError e) { a = \"V\" }\n"
  "try { for each i in range(1, 5, 0) { post(i) } } catch (ValueError e) { b = \"V\" }\n"
  "post(a + b)\n", "VV", NULL, 0 },

{ "desempacotar com e sem estrela levanta o MESMO tipo",
  /* sem estrela ja era ValueError; com estrela ficou OutputUnexpectedValues */
  "a = \"\"\n"
  "b = \"\"\n"
  "try { x, y = [1] } catch (ValueError e) { a = \"V\" }\n"
  "try { p, q, *r = [1] } catch (ValueError e) { b = \"V\" }\n"
  "post(a + b)\n", "VV", NULL, 0 },

{ "faltar argumento lista TODOS os que faltam",
  /* citava so o primeiro: quem esquecia tres consertava um por vez */
  "funct f(x, y, z) {\n"
  "    return x\n"
  "}\n"
  "try {\n"
  "    f(1)\n"
  "} catch (e) {\n"
  "    post(\"'y' and 'z'\" in str(e))\n"
  "}\n", "True", NULL, 0 },

{ "erro de aridade diz QUANTOS vieram",
  /* \"espera 1 ou 2 argumentos\" obrigava a contar na mao justamente quem
   * acabou de errar a conta. O CPython tem duas redacoes pra isto; a
   * escolhida e a que 97 dos 131 nomes medidos usam. */
  "try {\n"
  "    post(round(1, 2, 3))\n"
  "} catch (e) {\n"
  "    post(\"(3 given)\" in str(e))\n"
  "}\n", "True", NULL, 0 },

{ "erro de json diz onde",
  "import json\n"
  "try {\n"
  "    post(json.parse(\"{\\\"a\\\":1 \\\"b\\\":2}\"))\n"
  "} catch (ValueError e) {\n"
  "    post(\"line 1 column 8 (char 7)\" in str(e))\n"
  "}\n", "True", NULL, 0 },

{ "from X import Y ausente e ImportError; X.Y ausente e AttributeError",
  /* O Python separa os dois, e a separacao e util: `from` fala de IMPORTAR um
   * nome, `.` fala de ACESSAR um membro. Aqui exigiu opcode proprio
   * (OP_IMPORT_FROM) — sem ele a VM ve a mesma busca nos dois casos e nao tem
   * como saber qual erro levantar. */
  "import json\n"
  "a = \"\"\n"
  "b = \"\"\n"
  "try { post(json.naotem_zz) } catch (AttributeError e) { a = \"A\" }\n"
  "post(a)\n", "A", NULL, 0 },
{ "from X import Y ausente cita o modulo",
  "from json import naotem_zz\n",
  "", "cannot import name 'naotem_zz' from 'json'", 1 },
{ "modulo ausente diz o nome, no texto do CPython",
  "import naoexiste_zz_kd\n",
  "", "No module named 'naoexiste_zz_kd'", 1 },

/* I19: os apelidos sairam. Canonicos: request, qrcode, manpu, psodbc, sqlite3.
 * Modulo que nao existe responde como qualquer outro que nao existe — nao ha
 * caso especial pra apelido morto. */
{ "os cinco apelidos sairam",
  "import os\n"
  "import sys\n"
  "n = 0\n"
  "for each m in [\"db\", \"qr\", \"mp\", \"requests\", \"sqlite\"] {\n"
  "    r = os.run([sys.executable, \"-e\", \"import \" + m], true)\n"
  "    if \"No module named\" in r { n = n + 1 }\n"
  "}\n"
  "post(n)\n", "5", NULL, 0 },
{ "os cinco canonicos continuam",
  "import request\nimport qrcode\nimport manpu\nimport psodbc\nimport sqlite3\n"
  "post(\"ok\")\n", "ok", NULL, 0 },

{ "o nome do tipo na mensagem e o que o type() devolve",
  /* nao adianta copiar o CPython e dizer 'float'/'tuple'/'NoneType': esses
   * tipos nao existem aqui, e citar um deles seria mentira nova */
  "n = 0\n"
  "try { post(1.0 / 0) } catch (e) { if \"flo\" in str(e) { n = n + 1 } }\n"
  "try { post((1,2)[9]) } catch (e) { if \"tup\" in str(e) { n = n + 1 } }\n"
  "try { post(Null + 1) } catch (e) { if \"Null\" in str(e) { n = n + 1 } }\n"
  "post(n)\n", "3", NULL, 0 },
};

const int NC_LIBS = N_CASOS(CASOS_LIBS);
