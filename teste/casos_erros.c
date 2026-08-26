/*
 * Erros: quando a VM tem que RECLAMAR e quando tem que deixar passar.
 *
 * O padrão que mais dói aqui é o silêncio: chamada com argumento faltando
 * devolvendo `null`, nome nomeado errado sumindo, f-string engolindo o erro.
 * Erro engolido é pior que erro barulhento.
 */
#include "ps_teste.h"

const Caso CASOS_ERROS[] = {
/* ── argumento faltando: TEM que reclamar em toda forma de chamada ── */
{ "action: nomeado sem cobrir obrigatório",
  "action f(a, b=2) {\n"
  "    return a\n"
  "}\n"
  "post(f(b=1))\n",
  NULL, "faltando argumento: 'a'", -1 },
{ "método de instância sem argumento",
  "class C() {\n"
  "    action m(self, a) {\n"
  "        return a\n"
  "    }\n"
  "}\n"
  "post(C().m())\n",
  NULL, "faltando argumento: 'a'", -1 },
{ "__init__ sem argumento",
  "class C() {\n"
  "    action __init__(self, a) {\n"
  "        self.a = a\n"
  "    }\n"
  "}\n"
  "x = C()\n",
  NULL, "faltando argumento: 'a'", -1 },
{ "action solta sem argumento",
  "action f(a) {\n"
  "    return a\n"
  "}\n"
  "post(f())\n",
  NULL, "faltando argumento: 'a'", -1 },
{ "argumentos demais",
  "action f(a, b=2) {\n"
  "    return a\n"
  "}\n"
  "post(f(1, 2, 3))\n",
  NULL, "esperava até 2 argumentos, recebeu 3", -1 },

/* ── argumento nomeado que não existe ── */
{ "nomeado inexistente em action",
  "action f(a) {\n"
  "    return a\n"
  "}\n"
  "post(f(1, c=2))\n",
  NULL, "nao corresponde a nenhum parametro", -1 },
{ "nomeado inexistente em método",
  "class C() {\n"
  "    action m(self, a) {\n"
  "        return a\n"
  "    }\n"
  "}\n"
  "post(C().m(1, c=2))\n",
  NULL, "nao corresponde a nenhum parametro", -1 },

/* ── @static: sem ele, não dá pra chamar na classe ── */
{ "método normal chamado na classe",
  "class C() {\n"
  "    action m(self, a) {\n"
  "        return a\n"
  "    }\n"
  "}\n"
  "post(C.m(5))\n",
  NULL, "não tem método estático", -1 },
{ "@static com self na assinatura",
  "class C() {\n"
  "    @static\n"
  "    action m(self, a, b=10) {\n"
  "        return a + b\n"
  "    }\n"
  "}\n"
  "post(C.m(5))\n",
  "15", NULL, 0 },
{ "@static sem self",
  "class C() {\n"
  "    @static\n"
  "    action m(a) {\n"
  "        return a\n"
  "    }\n"
  "}\n"
  "post(C.m(5))\n",
  "5", NULL, 0 },

/* ── f-string: erro no trecho SOBE, não vira texto cru ── */
{ "f-string com nome fora de escopo",
  "post(f\"v: {zzz}\")\n", NULL, "variável não definida: zzz", -1 },
{ "f-string com divisão por zero",
  "post(f\"x {1/0} y\")\n", NULL, "divisão por zero", -1 },
{ "f-string válida continua interpolando",
  "oi = \"ola\"\npost(f\"{oi} mundo\")\n", "ola mundo", NULL, 0 },
{ "f-string escapa chave dobrada",
  "post(f\"{{literal}} e {1 + 1}\")\n", "{literal} e 2", NULL, 0 },
{ "f-string de uma expressão vira string",
  "post(type(f\"{[1, 2]}\"), type(f\"{42}\"))\n", "str str", NULL, 0 },
{ "f-string não devolve a mesma referência",
  "l = [1, 2]\ns = f\"{l}\"\npost(type(s))\npost(l)\n", "str\n[1, 2]", NULL, 0 },

/* ── chave de dict tem que ser imutável ── */
{ "lista como chave de dict",
  "d = {}\nd[[1,2]] = \"a\"\n", NULL, "chave de dict", -1 },
{ "dict como chave de dict",
  "d = {}\nd[{ \"x\": 1 }] = \"b\"\n", NULL, "chave de dict", -1 },
{ "chaves imutáveis seguem valendo",
  "d = {}\nd[\"s\"] = 1\nd[2] = \"b\"\nd[2.5] = \"c\"\nd[true] = \"d\"\nd[(1, 2)] = \"tup\"\n"
  "post(len(d), d[\"s\"], d[2], d[(1, 2)])\n",
  "5 1 b tup", NULL, 0 },

/* ── finally roda em TODA saída ── */
{ "finally com return",
  "action f() {\n    try {\n        return \"do try\"\n    } catch (e) {\n        return \"do catch\"\n"
  "    } finally {\n        post(\"finally\")\n    }\n}\npost(f())\n",
  "finally\ndo try", NULL, 0 },
{ "finally com break",
  "for each i in [1, 2, 3] {\n    try {\n        if (i == 2) { break }\n        post(\"corpo\", i)\n"
  "    } catch (e) {\n        post(\"c\")\n    } finally {\n        post(\"finally\", i)\n    }\n}\npost(\"fim\")\n",
  "corpo 1\nfinally 1\nfinally 2\nfim", NULL, 0 },
{ "finally com continue",
  "for each i in [1, 2] {\n    try {\n        continue\n    } catch (e) {\n        post(\"c\")\n"
  "    } finally {\n        post(\"finally\", i)\n    }\n}\n",
  "finally 1\nfinally 2", NULL, 0 },
{ "finally com raise dentro do catch",
  "try {\n    try {\n        raise ValueError(\"x\")\n    } catch (e) {\n        raise KeyError(\"re\")\n"
  "    } finally {\n        post(\"finally\")\n    }\n} catch (e) {\n    post(\"fora\")\n}\n",
  "finally\nfora", NULL, 0 },
{ "finally nos caminhos normais",
  "try {\n    post(\"ok\")\n} catch (e) {\n    post(\"nao\")\n} finally {\n    post(\"f1\")\n}\n"
  "try {\n    raise ValueError(\"x\")\n} catch (e) {\n    post(\"peguei\")\n} finally {\n    post(\"f2\")\n}\n",
  "ok\nf1\npeguei\nf2", NULL, 0 },

/* ── @NonNull ── */
{ "@NonNull dentro de Entity",
  "class C() {\n"
  "    @NonNull\n"
  "    action f(self, a) {\n"
  "        return a\n"
  "    }\n"
  "}\n"
  "post(C().f(null))\n",
  NULL, "NonNull", -1 },
{ "@NonNull com valor válido passa",
  "class C() {\n"
  "    @NonNull\n"
  "    action f(self, a) {\n"
  "        return a\n"
  "    }\n"
  "}\n"
  "post(C().f(7))\n",
  "7", NULL, 0 },

/* ── comentário e linha vazia abrindo bloco ── */
{ "comentário como 1ª linha do bloco",
  "action f(x) {\n"
  "    // comentário\n"
  "    return x + 1\n"
  "}\n"
  "post(f(1))\n", "2", NULL, 0 },
{ "linha vazia como 1ª linha do bloco",
  "action g(x) {\n"
  "\n"
  "    return x * 2\n"
  "}\n"
  "post(g(3))\n", "6", NULL, 0 },
};
const int NC_ERROS = N_CASOS(CASOS_ERROS);
