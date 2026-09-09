/*
 * EQUIVALENCIA INTERNA — a mesma computação escrita de várias formas.
 *
 * Onde o Python não serve de oráculo (Entity, closure, async, gerador,
 * match, using, count each, herança, private, @static), a linguagem oferece
 * caminhos REDUNDANTES pra dizer a mesma coisa: `for each` x `while` x
 * gerador x recursão; bloco `:` x bloco `{}`; `match` x `if/elif`; closure x
 * parâmetro x Entity; `using` x close() explícito.
 *
 * Se duas formas da mesma conta divergem, uma das duas está quebrada — e não
 * interessa o que o Python faria. Foi assim que apareceu o bug de bloco `:`
 * dentro de bloco `{}`: cada peça passava sozinha, a combinação é que quebrava.
 *
 * GERADO. Para adicionar uma família, edite teste/geradores/equivalencia.ps
 * e rode o gerador de novo — não edite este arquivo.
 */
#include "ps_teste.h"

const Caso CASOS_EQUIVALENCIA[] = {
{ "equiv: soma 0..0 [1/6]",
  "s = 0\n"
  "for each i in range(0) {\n"
  "    s = s + i\n"
  "}\n"
  "post(s)\n"
  "\n",
  "0", NULL, 0 },
{ "equiv: soma 0..0 [2/6]",
  "s = 0\n"
  "n = 0\n"
  "while n < 0 {\n"
  "    s = s + n\n"
  "    n = n + 1\n"
  "}\n"
  "post(s)\n"
  "\n",
  "0", NULL, 0 },
{ "equiv: soma 0..0 [3/6]",
  "funct g() {\n"
  "    for each i in range(0) {\n"
  "        yield i\n"
  "    }\n"
  "}\n"
  "s = 0\n"
  "for each v in g() {\n"
  "    s = s + v\n"
  "}\n"
  "post(s)\n"
  "\n",
  "0", NULL, 0 },
{ "equiv: soma 0..0 [4/6]",
  "funct r(i, acc) {\n"
  "    if i >= 0 {\n"
  "        return acc\n"
  "    }\n"
  "    return r(i + 1, acc + i)\n"
  "}\n"
  "post(r(0, 0))\n"
  "\n",
  "0", NULL, 0 },
{ "equiv: soma 0..0 [5/6]",
  "post(sum(range(0)))\n"
  "\n",
  "0", NULL, 0 },
{ "equiv: soma 0..0 [6/6]",
  "l = []\n"
  "for each i in range(0) {\n"
  "    l.append(i)\n"
  "}\n"
  "post(sum(l))\n"
  "\n",
  "0", NULL, 0 },
{ "equiv: soma 0..1 [1/6]",
  "s = 0\n"
  "for each i in range(1) {\n"
  "    s = s + i\n"
  "}\n"
  "post(s)\n"
  "\n",
  "0", NULL, 0 },
{ "equiv: soma 0..1 [2/6]",
  "s = 0\n"
  "n = 0\n"
  "while n < 1 {\n"
  "    s = s + n\n"
  "    n = n + 1\n"
  "}\n"
  "post(s)\n"
  "\n",
  "0", NULL, 0 },
{ "equiv: soma 0..1 [3/6]",
  "funct g() {\n"
  "    for each i in range(1) {\n"
  "        yield i\n"
  "    }\n"
  "}\n"
  "s = 0\n"
  "for each v in g() {\n"
  "    s = s + v\n"
  "}\n"
  "post(s)\n"
  "\n",
  "0", NULL, 0 },
{ "equiv: soma 0..1 [4/6]",
  "funct r(i, acc) {\n"
  "    if i >= 1 {\n"
  "        return acc\n"
  "    }\n"
  "    return r(i + 1, acc + i)\n"
  "}\n"
  "post(r(0, 0))\n"
  "\n",
  "0", NULL, 0 },
{ "equiv: soma 0..1 [5/6]",
  "post(sum(range(1)))\n"
  "\n",
  "0", NULL, 0 },
{ "equiv: soma 0..1 [6/6]",
  "l = []\n"
  "for each i in range(1) {\n"
  "    l.append(i)\n"
  "}\n"
  "post(sum(l))\n"
  "\n",
  "0", NULL, 0 },
{ "equiv: soma 0..5 [1/6]",
  "s = 0\n"
  "for each i in range(5) {\n"
  "    s = s + i\n"
  "}\n"
  "post(s)\n"
  "\n",
  "10", NULL, 0 },
{ "equiv: soma 0..5 [2/6]",
  "s = 0\n"
  "n = 0\n"
  "while n < 5 {\n"
  "    s = s + n\n"
  "    n = n + 1\n"
  "}\n"
  "post(s)\n"
  "\n",
  "10", NULL, 0 },
{ "equiv: soma 0..5 [3/6]",
  "funct g() {\n"
  "    for each i in range(5) {\n"
  "        yield i\n"
  "    }\n"
  "}\n"
  "s = 0\n"
  "for each v in g() {\n"
  "    s = s + v\n"
  "}\n"
  "post(s)\n"
  "\n",
  "10", NULL, 0 },
{ "equiv: soma 0..5 [4/6]",
  "funct r(i, acc) {\n"
  "    if i >= 5 {\n"
  "        return acc\n"
  "    }\n"
  "    return r(i + 1, acc + i)\n"
  "}\n"
  "post(r(0, 0))\n"
  "\n",
  "10", NULL, 0 },
{ "equiv: soma 0..5 [5/6]",
  "post(sum(range(5)))\n"
  "\n",
  "10", NULL, 0 },
{ "equiv: soma 0..5 [6/6]",
  "l = []\n"
  "for each i in range(5) {\n"
  "    l.append(i)\n"
  "}\n"
  "post(sum(l))\n"
  "\n",
  "10", NULL, 0 },
{ "equiv: soma 0..17 [1/6]",
  "s = 0\n"
  "for each i in range(17) {\n"
  "    s = s + i\n"
  "}\n"
  "post(s)\n"
  "\n",
  "136", NULL, 0 },
{ "equiv: soma 0..17 [2/6]",
  "s = 0\n"
  "n = 0\n"
  "while n < 17 {\n"
  "    s = s + n\n"
  "    n = n + 1\n"
  "}\n"
  "post(s)\n"
  "\n",
  "136", NULL, 0 },
{ "equiv: soma 0..17 [3/6]",
  "funct g() {\n"
  "    for each i in range(17) {\n"
  "        yield i\n"
  "    }\n"
  "}\n"
  "s = 0\n"
  "for each v in g() {\n"
  "    s = s + v\n"
  "}\n"
  "post(s)\n"
  "\n",
  "136", NULL, 0 },
{ "equiv: soma 0..17 [4/6]",
  "funct r(i, acc) {\n"
  "    if i >= 17 {\n"
  "        return acc\n"
  "    }\n"
  "    return r(i + 1, acc + i)\n"
  "}\n"
  "post(r(0, 0))\n"
  "\n",
  "136", NULL, 0 },
{ "equiv: soma 0..17 [5/6]",
  "post(sum(range(17)))\n"
  "\n",
  "136", NULL, 0 },
{ "equiv: soma 0..17 [6/6]",
  "l = []\n"
  "for each i in range(17) {\n"
  "    l.append(i)\n"
  "}\n"
  "post(sum(l))\n"
  "\n",
  "136", NULL, 0 },
{ "equiv: if [1/2]",
  "if true {\n"
  "    post(\"A\")\n"
  "}\n"
  "\n",
  "A", NULL, 0 },
{ "equiv: if [2/2]",
  "if (true)\n"
  "{\n"
  "    post(\"A\")\n"
  "}\n"
  "\n",
  "A", NULL, 0 },
{ "equiv: if/else [1/2]",
  "if false {\n"
  "    post(\"X\")\n"
  "} else {\n"
  "    post(\"A\")\n"
  "}\n"
  "\n",
  "A", NULL, 0 },
{ "equiv: if/else [2/2]",
  "if (false)\n"
  "{\n"
  "    post(\"X\")\n"
  "} else\n"
  "{\n"
  "    post(\"A\")\n"
  "}\n"
  "\n",
  "A", NULL, 0 },
{ "equiv: while [1/2]",
  "n = 0\n"
  "while n < 3 {\n"
  "    n = n + 1\n"
  "}\n"
  "post(n)\n"
  "\n",
  "3", NULL, 0 },
{ "equiv: while [2/2]",
  "n = 0\n"
  "while (n < 3)\n"
  "{\n"
  "    n = n + 1\n"
  "}\n"
  "post(n)\n"
  "\n",
  "3", NULL, 0 },
{ "equiv: for each [1/2]",
  "for each i in [1,2] {\n"
  "    post(i)\n"
  "}\n"
  "\n",
  "1\n2", NULL, 0 },
{ "equiv: for each [2/2]",
  "for each i in [1,2]\n"
  "{\n"
  "    post(i)\n"
  "}\n"
  "\n",
  "1\n2", NULL, 0 },
{ "equiv: funct [1/2]",
  "funct f() {\n"
  "    return 7\n"
  "}\n"
  "post(f())\n"
  "\n",
  "7", NULL, 0 },
{ "equiv: funct [2/2]",
  "funct f()\n"
  "{\n"
  "    return 7\n"
  "}\n"
  "post(f())\n"
  "\n",
  "7", NULL, 0 },
{ "equiv: try [1/2]",
  "try {\n"
  "    raise B(\"x\")\n"
  "} catch (e) {\n"
  "    post(\"peguei\")\n"
  "}\n"
  "\n",
  "peguei", NULL, 0 },
{ "equiv: try [2/2]",
  "try\n"
  "{\n"
  "    raise B(\"x\")\n"
  "} catch (e)\n"
  "{\n"
  "    post(\"peguei\")\n"
  "}\n"
  "\n",
  "peguei", NULL, 0 },
{ "equiv: Entity [1/2]",
  "Entity P() {\n"
  "    funct m(self) {\n"
  "        return 3\n"
  "    }\n"
  "}\n"
  "post(P().m())\n"
  "\n",
  "3", NULL, 0 },
{ "equiv: Entity [2/2]",
  "Entity P()\n"
  "{\n"
  "    funct m(self)\n"
  "    {\n"
  "        return 3\n"
  "    }\n"
  "}\n"
  "post(P().m())\n"
  "\n",
  "3", NULL, 0 },
{ "equiv: match 1 [1/3]",
  "x = 1\n"
  "match x {\n"
  "    case 1 {\n"
  "        post(\"um\")\n"
  "    }\n"
  "    case 2 {\n"
  "        post(\"dois\")\n"
  "    }\n"
  "    case _ {\n"
  "        post(\"outro\")\n"
  "    }\n"
  "}\n"
  "\n",
  "um", NULL, 0 },
{ "equiv: match 1 [2/3]",
  "x = 1\n"
  "match x {\n"
  " case 1 { post(\"um\") }\n"
  " case 2 { post(\"dois\") }\n"
  " case _ { post(\"outro\") }\n"
  "}\n"
  "\n",
  "um", NULL, 0 },
{ "equiv: match 1 [3/3]",
  "x = 1\n"
  "if x == 1 {\n"
  "    post(\"um\")\n"
  "} elif x == 2 {\n"
  "    post(\"dois\")\n"
  "} else {\n"
  "    post(\"outro\")\n"
  "}\n"
  "\n",
  "um", NULL, 0 },
{ "equiv: match 2 [1/3]",
  "x = 2\n"
  "match x {\n"
  "    case 1 {\n"
  "        post(\"um\")\n"
  "    }\n"
  "    case 2 {\n"
  "        post(\"dois\")\n"
  "    }\n"
  "    case _ {\n"
  "        post(\"outro\")\n"
  "    }\n"
  "}\n"
  "\n",
  "dois", NULL, 0 },
{ "equiv: match 2 [2/3]",
  "x = 2\n"
  "match x {\n"
  " case 1 { post(\"um\") }\n"
  " case 2 { post(\"dois\") }\n"
  " case _ { post(\"outro\") }\n"
  "}\n"
  "\n",
  "dois", NULL, 0 },
{ "equiv: match 2 [3/3]",
  "x = 2\n"
  "if x == 1 {\n"
  "    post(\"um\")\n"
  "} elif x == 2 {\n"
  "    post(\"dois\")\n"
  "} else {\n"
  "    post(\"outro\")\n"
  "}\n"
  "\n",
  "dois", NULL, 0 },
{ "equiv: match 9 [1/3]",
  "x = 9\n"
  "match x {\n"
  "    case 1 {\n"
  "        post(\"um\")\n"
  "    }\n"
  "    case 2 {\n"
  "        post(\"dois\")\n"
  "    }\n"
  "    case _ {\n"
  "        post(\"outro\")\n"
  "    }\n"
  "}\n"
  "\n",
  "outro", NULL, 0 },
{ "equiv: match 9 [2/3]",
  "x = 9\n"
  "match x {\n"
  " case 1 { post(\"um\") }\n"
  " case 2 { post(\"dois\") }\n"
  " case _ { post(\"outro\") }\n"
  "}\n"
  "\n",
  "outro", NULL, 0 },
{ "equiv: match 9 [3/3]",
  "x = 9\n"
  "if x == 1 {\n"
  "    post(\"um\")\n"
  "} elif x == 2 {\n"
  "    post(\"dois\")\n"
  "} else {\n"
  "    post(\"outro\")\n"
  "}\n"
  "\n",
  "outro", NULL, 0 },
{ "equiv: captura 0 [1/4]",
  "funct mult(k) {\n"
  "    funct f(x) {\n"
  "        return x * k\n"
  "    }\n"
  "    return f\n"
  "}\n"
  "post(mult(0)(7))\n"
  "\n",
  "0", NULL, 0 },
{ "equiv: captura 0 [2/4]",
  "funct f(x, k) {\n"
  "    return x * k\n"
  "}\n"
  "post(f(7, 0))\n"
  "\n",
  "0", NULL, 0 },
{ "equiv: captura 0 [3/4]",
  "Entity M() {\n"
  "    funct __init__(self, k) {\n"
  "        self.k = k\n"
  "    }\n"
  "    funct ap(self, x) {\n"
  "        return x * self.k\n"
  "    }\n"
  "}\n"
  "post(M(0).ap(7))\n"
  "\n",
  "0", NULL, 0 },
{ "equiv: captura 0 [4/4]",
  "k = 0\n"
  "funct f(x) {\n"
  "    return x * k\n"
  "}\n"
  "post(f(7))\n"
  "\n",
  "0", NULL, 0 },
{ "equiv: captura 3 [1/4]",
  "funct mult(k) {\n"
  "    funct f(x) {\n"
  "        return x * k\n"
  "    }\n"
  "    return f\n"
  "}\n"
  "post(mult(3)(7))\n"
  "\n",
  "21", NULL, 0 },
{ "equiv: captura 3 [2/4]",
  "funct f(x, k) {\n"
  "    return x * k\n"
  "}\n"
  "post(f(7, 3))\n"
  "\n",
  "21", NULL, 0 },
{ "equiv: captura 3 [3/4]",
  "Entity M() {\n"
  "    funct __init__(self, k) {\n"
  "        self.k = k\n"
  "    }\n"
  "    funct ap(self, x) {\n"
  "        return x * self.k\n"
  "    }\n"
  "}\n"
  "post(M(3).ap(7))\n"
  "\n",
  "21", NULL, 0 },
{ "equiv: captura 3 [4/4]",
  "k = 3\n"
  "funct f(x) {\n"
  "    return x * k\n"
  "}\n"
  "post(f(7))\n"
  "\n",
  "21", NULL, 0 },
{ "equiv: captura -2 [1/4]",
  "funct mult(k) {\n"
  "    funct f(x) {\n"
  "        return x * k\n"
  "    }\n"
  "    return f\n"
  "}\n"
  "post(mult(-2)(7))\n"
  "\n",
  "-14", NULL, 0 },
{ "equiv: captura -2 [2/4]",
  "funct f(x, k) {\n"
  "    return x * k\n"
  "}\n"
  "post(f(7, -2))\n"
  "\n",
  "-14", NULL, 0 },
{ "equiv: captura -2 [3/4]",
  "Entity M() {\n"
  "    funct __init__(self, k) {\n"
  "        self.k = k\n"
  "    }\n"
  "    funct ap(self, x) {\n"
  "        return x * self.k\n"
  "    }\n"
  "}\n"
  "post(M(-2).ap(7))\n"
  "\n",
  "-14", NULL, 0 },
{ "equiv: captura -2 [4/4]",
  "k = -2\n"
  "funct f(x) {\n"
  "    return x * k\n"
  "}\n"
  "post(f(7))\n"
  "\n",
  "-14", NULL, 0 },
{ "equiv: contador 3x [1/3]",
  "funct faz() {\n"
  "    n = 0\n"
  "    funct inc() {\n"
  "        n = n + 1\n"
  "        return n\n"
  "    }\n"
  "    return inc\n"
  "}\n"
  "c = faz()\n"
  "post(c(), c(), c())\n"
  "\n",
  "1 2 3", NULL, 0 },
{ "equiv: contador 3x [2/3]",
  "Entity C() {\n"
  "    funct __init__(self) {\n"
  "        self.n = 0\n"
  "    }\n"
  "    funct inc(self) {\n"
  "        self.n = self.n + 1\n"
  "        return self.n\n"
  "    }\n"
  "}\n"
  "c = C()\n"
  "post(c.inc(), c.inc(), c.inc())\n"
  "\n",
  "1 2 3", NULL, 0 },
{ "equiv: contador 3x [3/3]",
  "l = [0]\n"
  "funct inc() {\n"
  "    l[0] = l[0] + 1\n"
  "    return l[0]\n"
  "}\n"
  "post(inc(), inc(), inc())\n"
  "\n",
  "1 2 3", NULL, 0 },
{ "equiv: dobro de 0..4 [1/4]",
  "funct g() {\n"
  "    for each i in range(5) {\n"
  "        yield i * 2\n"
  "    }\n"
  "}\n"
  "post(list(g()))\n"
  "\n",
  "[0, 2, 4, 6, 8]", NULL, 0 },
{ "equiv: dobro de 0..4 [2/4]",
  "l = []\n"
  "for each i in range(5) {\n"
  "    l.append(i * 2)\n"
  "}\n"
  "post(l)\n"
  "\n",
  "[0, 2, 4, 6, 8]", NULL, 0 },
{ "equiv: dobro de 0..4 [3/4]",
  "funct d(x) {\n"
  "    return x * 2\n"
  "}\n"
  "post(map(list(range(5)), d))\n"
  "\n",
  "[0, 2, 4, 6, 8]", NULL, 0 },
{ "equiv: dobro de 0..4 [4/4]",
  "funct d2(x) {\n"
  "    return x * 2\n"
  "}\n"
  "post(map(range(5), d2))\n"
  "\n",
  "[0, 2, 4, 6, 8]", NULL, 0 },
{ "equiv: finally com return [1/2]",
  "funct f() {\n"
  "    try {\n"
  "        return \"R\"\n"
  "    } catch(e) {\n"
  "        post(\"C\")\n"
  "    } finally {\n"
  "        post(\"F\")\n"
  "    }\n"
  "}\n"
  "post(f())\n"
  "\n",
  "F\nR", NULL, 0 },
{ "equiv: finally com return [2/2]",
  "funct f() {\n"
  " try {\n"
  "  return \"R\"\n"
  " } catch (e) {\n"
  "  post(\"C\")\n"
  " } finally {\n"
  "  post(\"F\")\n"
  " }\n"
  "}\n"
  "post(f())\n"
  "\n",
  "F\nR", NULL, 0 },
{ "equiv: finally com break [1/2]",
  "for each i in [1,2,3] {\n"
  "    try {\n"
  "        if i == 2 {\n"
  "            break\n"
  "        }\n"
  "    } catch(e) {\n"
  "        post(\"C\")\n"
  "    } finally {\n"
  "        post(\"F\" + str(i))\n"
  "    }\n"
  "}\n"
  "post(\"fim\")\n"
  "\n",
  "F1\nF2\nfim", NULL, 0 },
{ "equiv: finally com break [2/2]",
  "for each i in [1,2,3] {\n"
  " try {\n"
  "  if (i == 2) { break }\n"
  " } catch (e) {\n"
  "  post(\"C\")\n"
  " } finally {\n"
  "  post(\"F\" + str(i))\n"
  " }\n"
  "}\n"
  "post(\"fim\")\n"
  "\n",
  "F1\nF2\nfim", NULL, 0 },
{ "equiv: finally com continue [1/2]",
  "for each i in [1,2] {\n"
  "    try {\n"
  "        continue\n"
  "    } catch(e) {\n"
  "        post(\"C\")\n"
  "    } finally {\n"
  "        post(\"F\" + str(i))\n"
  "    }\n"
  "}\n"
  "post(\"fim\")\n"
  "\n",
  "F1\nF2\nfim", NULL, 0 },
{ "equiv: finally com continue [2/2]",
  "for each i in [1,2] {\n"
  " try {\n"
  "  continue\n"
  " } catch (e) {\n"
  "  post(\"C\")\n"
  " } finally {\n"
  "  post(\"F\" + str(i))\n"
  " }\n"
  "}\n"
  "post(\"fim\")\n"
  "\n",
  "F1\nF2\nfim", NULL, 0 },
{ "equiv: finally roda com erro propagando [1/2]",
  "funct f() {\n"
  "    try {\n"
  "        raise B(\"x\")\n"
  "    } catch(e) {\n"
  "        post(\"C\")\n"
  "    } finally {\n"
  "        post(\"F\")\n"
  "    }\n"
  "    return \"R\"\n"
  "}\n"
  "post(f())\n"
  "\n",
  "C\nF\nR", NULL, 0 },
{ "equiv: finally roda com erro propagando [2/2]",
  "funct f() {\n"
  " try {\n"
  "  raise B(\"x\")\n"
  " } catch (e) {\n"
  "  post(\"C\")\n"
  " } finally {\n"
  "  post(\"F\")\n"
  " }\n"
  " return \"R\"\n"
  "}\n"
  "post(f())\n"
  "\n",
  "C\nF\nR", NULL, 0 },
{ "equiv: async dobro [1/3]",
  "async funct d(n) {\n"
  "    return n * 2\n"
  "}\n"
  "post(await d(4))\n"
  "\n",
  "8", NULL, 0 },
{ "equiv: async dobro [2/3]",
  "funct d(n) {\n"
  "    return n * 2\n"
  "}\n"
  "post(d(4))\n"
  "\n",
  "8", NULL, 0 },
{ "equiv: async dobro [3/3]",
  "async funct d(n) {\n"
  "    return n * 2\n"
  "}\n"
  "post(gather(d(4))[0])\n"
  "\n",
  "8", NULL, 0 },
{ "equiv: async lista [1/3]",
  "async funct d(n) {\n"
  "    return n * 2\n"
  "}\n"
  "post(await [d(1), d(2), d(3)])\n"
  "\n",
  "[2, 4, 6]", NULL, 0 },
{ "equiv: async lista [2/3]",
  "funct d(n) {\n"
  "    return n * 2\n"
  "}\n"
  "post([d(1), d(2), d(3)])\n"
  "\n",
  "[2, 4, 6]", NULL, 0 },
{ "equiv: async lista [3/3]",
  "async funct d(n) {\n"
  "    return n * 2\n"
  "}\n"
  "post(gather(d(1), d(2), d(3)))\n"
  "\n",
  "[2, 4, 6]", NULL, 0 },
/* Sem conversao implicita (2026-09-09): `int x = "7"` e erro. A equivalencia
 * que sobrevive e a explicita — `int x = int("7")` == `int("7")`. */
{ "equiv: int de '7' [1/2]",
  "int x = int(\"7\")\n"
  "post(x)\n"
  "\n",
  "7", NULL, 0 },
{ "equiv: int de '7' [2/2]",
  "post(int(\"7\"))\n"
  "\n",
  "7", NULL, 0 },
{ "equiv: flo de 1 [1/2]",
  "flo x = flo(1)\n"
  "post(x)\n"
  "\n",
  "1.0", NULL, 0 },
{ "equiv: flo de 1 [2/2]",
  "post(flo(1))\n"
  "\n",
  "1.0", NULL, 0 },
{ "equiv: char de 64 [1/2]",
  "char x = 64\n"
  "post(x)\n"
  "\n",
  "@", NULL, 0 },
{ "equiv: char de 64 [2/2]",
  "post(chr(64))\n"
  "\n",
  "@", NULL, 0 },
{ "equiv: inverter texto [1/3]",
  "post(\"abcdef\"[::-1])\n"
  "\n",
  "fedcba", NULL, 0 },
{ "equiv: inverter texto [2/3]",
  "s = \"\"\n"
  "for each c in \"abcdef\" {\n"
  "    s = c + s\n"
  "}\n"
  "post(s)\n"
  "\n",
  "fedcba", NULL, 0 },
{ "equiv: inverter texto [3/3]",
  "post(\"\".join(reversed(list(\"abcdef\"))))\n"
  "\n",
  "fedcba", NULL, 0 },
{ "equiv: contar caractere [1/3]",
  "post(\"banana\".count(\"a\"))\n"
  "\n",
  "3", NULL, 0 },
{ "equiv: contar caractere [2/3]",
  "n = 0\n"
  "for each c in \"banana\" {\n"
  "    if c == \"a\" {\n"
  "        n = n + 1\n"
  "    }\n"
  "}\n"
  "post(n)\n"
  "\n",
  "3", NULL, 0 },
{ "equiv: contar caractere [3/3]",
  "post(count each char in \"aaa\"[0:3])\n"
  "\n",
  "3", NULL, 0 },
{ "equiv: heranca 0 [1/3]",
  "Entity A() {\n"
  "    funct __init__(self, x) {\n"
  "        self.x = x\n"
  "    }\n"
  "}\n"
  "Entity B(A) {\n"
  "    funct __init__(self, x) {\n"
  "        base(x)\n"
  "    }\n"
  "}\n"
  "post(B(0).x)\n"
  "\n",
  "0", NULL, 0 },
{ "equiv: heranca 0 [2/3]",
  "Entity A() {\n"
  "    funct __init__(self, x) {\n"
  "        self.x = x\n"
  "    }\n"
  "}\n"
  "post(A(0).x)\n"
  "\n",
  "0", NULL, 0 },
{ "equiv: heranca 0 [3/3]",
  "Entity B() {\n"
  "    funct __init__(self, x) {\n"
  "        self.x = x\n"
  "    }\n"
  "    funct pega(self) {\n"
  "        return self.x\n"
  "    }\n"
  "}\n"
  "post(B(0).pega())\n"
  "\n",
  "0", NULL, 0 },
{ "equiv: heranca 5 [1/3]",
  "Entity A() {\n"
  "    funct __init__(self, x) {\n"
  "        self.x = x\n"
  "    }\n"
  "}\n"
  "Entity B(A) {\n"
  "    funct __init__(self, x) {\n"
  "        base(x)\n"
  "    }\n"
  "}\n"
  "post(B(5).x)\n"
  "\n",
  "5", NULL, 0 },
{ "equiv: heranca 5 [2/3]",
  "Entity A() {\n"
  "    funct __init__(self, x) {\n"
  "        self.x = x\n"
  "    }\n"
  "}\n"
  "post(A(5).x)\n"
  "\n",
  "5", NULL, 0 },
{ "equiv: heranca 5 [3/3]",
  "Entity B() {\n"
  "    funct __init__(self, x) {\n"
  "        self.x = x\n"
  "    }\n"
  "    funct pega(self) {\n"
  "        return self.x\n"
  "    }\n"
  "}\n"
  "post(B(5).pega())\n"
  "\n",
  "5", NULL, 0 },
{ "equiv: heranca -3 [1/3]",
  "Entity A() {\n"
  "    funct __init__(self, x) {\n"
  "        self.x = x\n"
  "    }\n"
  "}\n"
  "Entity B(A) {\n"
  "    funct __init__(self, x) {\n"
  "        base(x)\n"
  "    }\n"
  "}\n"
  "post(B(-3).x)\n"
  "\n",
  "-3", NULL, 0 },
{ "equiv: heranca -3 [2/3]",
  "Entity A() {\n"
  "    funct __init__(self, x) {\n"
  "        self.x = x\n"
  "    }\n"
  "}\n"
  "post(A(-3).x)\n"
  "\n",
  "-3", NULL, 0 },
{ "equiv: heranca -3 [3/3]",
  "Entity B() {\n"
  "    funct __init__(self, x) {\n"
  "        self.x = x\n"
  "    }\n"
  "    funct pega(self) {\n"
  "        return self.x\n"
  "    }\n"
  "}\n"
  "post(B(-3).pega())\n"
  "\n",
  "-3", NULL, 0 },
{ "equiv: private lido de dentro [1/3]",
  "Entity P() {\n"
  "    private s: int\n"
  "    funct ve(self) {\n"
  "        return self.s\n"
  "    }\n"
  "}\n"
  "post(P(9).ve())\n"
  "\n",
  "9", NULL, 0 },
{ "equiv: private lido de dentro [2/3]",
  "Entity Q() {\n"
  "    s: int\n"
  "    funct ve(self) {\n"
  "        return self.s\n"
  "    }\n"
  "}\n"
  "post(Q(9).ve())\n"
  "\n",
  "9", NULL, 0 },
{ "equiv: private lido de dentro [3/3]",
  "funct ve(s) {\n"
  "    return s\n"
  "}\n"
  "post(ve(9))\n"
  "\n",
  "9", NULL, 0 },
{ "equiv: static [1/2]",
  "Entity K() {\n"
  "    @static\n"
  "    funct f(self, n) {\n"
  "        return n + 1\n"
  "    }\n"
  "}\n"
  "post(K.f(4))\n"
  "\n",
  "5", NULL, 0 },
{ "equiv: static [2/2]",
  "funct f(n) {\n"
  "    return n + 1\n"
  "}\n"
  "post(f(4))\n"
  "\n",
  "5", NULL, 0 },
{ "equiv: desempacotar par [1/3]",
  "a, b = 1, 2\n"
  "post(a, b)\n"
  "\n",
  "1 2", NULL, 0 },
{ "equiv: desempacotar par [2/3]",
  "t = (1, 2)\n"
  "post(t[0], t[1])\n"
  "\n",
  "1 2", NULL, 0 },
{ "equiv: desempacotar par [3/3]",
  "l = [1, 2]\n"
  "post(l[0], l[1])\n"
  "\n",
  "1 2", NULL, 0 },
{ "equiv: desempacotar troca [1/2]",
  "a = 1\n"
  "b = 2\n"
  "a, b = b, a\n"
  "post(a, b)\n"
  "\n",
  "2 1", NULL, 0 },
{ "equiv: desempacotar troca [2/2]",
  "a = 1\n"
  "b = 2\n"
  "t = a\n"
  "a = b\n"
  "b = t\n"
  "post(a, b)\n"
  "\n",
  "2 1", NULL, 0 },
{ "equiv: enum valor [1/2]",
  "enum Cor { A, B }\n"
  "post(Cor.A, Cor.B)\n"
  "\n",
  "0 1", NULL, 0 },
{ "equiv: enum valor [2/2]",
  "d = {\"A\": 0, \"B\": 1}\n"
  "post(d[\"A\"], d[\"B\"])\n"
  "\n",
  "0 1", NULL, 0 },
{ "equiv: contar 7 em [1, 7, 7, 2, 7] [1/3]",
  "l = [1, 7, 7, 2, 7]\n"
  "post(count int(7) in l)\n"
  "\n",
  "3", NULL, 0 },
{ "equiv: contar 7 em [1, 7, 7, 2, 7] [2/3]",
  "l = [1, 7, 7, 2, 7]\n"
  "n = 0\n"
  "for each x in l {\n"
  "    if x == 7 {\n"
  "        n = n + 1\n"
  "    }\n"
  "}\n"
  "post(n)\n"
  "\n",
  "3", NULL, 0 },
{ "equiv: contar 7 em [1, 7, 7, 2, 7] [3/3]",
  "l = [1, 7, 7, 2, 7]\n"
  "post(l.count(7))\n"
  "\n",
  "3", NULL, 0 },
{ "equiv: contar 9 em [1, 2, 3] [1/3]",
  "l = [1, 2, 3]\n"
  "post(count int(9) in l)\n"
  "\n",
  "0", NULL, 0 },
{ "equiv: contar 9 em [1, 2, 3] [2/3]",
  "l = [1, 2, 3]\n"
  "n = 0\n"
  "for each x in l {\n"
  "    if x == 9 {\n"
  "        n = n + 1\n"
  "    }\n"
  "}\n"
  "post(n)\n"
  "\n",
  "0", NULL, 0 },
{ "equiv: contar 9 em [1, 2, 3] [3/3]",
  "l = [1, 2, 3]\n"
  "post(l.count(9))\n"
  "\n",
  "0", NULL, 0 },
{ "equiv: using fecha o arquivo [1/2]",
  "using open(\"/tmp/ps_eq.txt\", \"w\") as f {\n"
  "    f.write(\"x\")\n"
  "}\n"
  "using open(\"/tmp/ps_eq.txt\") as g {\n"
  "    post(g.read())\n"
  "}\n"
  "\n",
  "x", NULL, 0 },
{ "equiv: using fecha o arquivo [2/2]",
  "f = open(\"/tmp/ps_eq2.txt\", \"w\")\n"
  "f.write(\"x\")\n"
  "f.close()\n"
  "g = open(\"/tmp/ps_eq2.txt\")\n"
  "post(g.read())\n"
  "g.close()\n"
  "\n",
  "x", NULL, 0 },
{ "equiv: montar texto 1 'x' [1/3]",
  "a = 1\n"
  "b = \"x\"\n"
  "post(f\"{a}-{b}\")\n"
  "\n",
  "1-x", NULL, 0 },
{ "equiv: montar texto 1 'x' [2/3]",
  "a = 1\n"
  "b = \"x\"\n"
  "post(str(a) + \"-\" + b)\n"
  "\n",
  "1-x", NULL, 0 },
{ "equiv: montar texto 1 'x' [3/3]",
  "a = 1\n"
  "b = \"x\"\n"
  "post(\"{}-{}\".format(a, b))\n"
  "\n",
  "1-x", NULL, 0 },
{ "equiv: montar texto -3 'ção' [1/3]",
  "a = -3\n"
  "b = \"ção\"\n"
  "post(f\"{a}-{b}\")\n"
  "\n",
  "-3-ção", NULL, 0 },
{ "equiv: montar texto -3 'ção' [2/3]",
  "a = -3\n"
  "b = \"ção\"\n"
  "post(str(a) + \"-\" + b)\n"
  "\n",
  "-3-ção", NULL, 0 },
{ "equiv: montar texto -3 'ção' [3/3]",
  "a = -3\n"
  "b = \"ção\"\n"
  "post(\"{}-{}\".format(a, b))\n"
  "\n",
  "-3-ção", NULL, 0 },
{ "equiv: dois niveis [1/2]",
  "funct n1(a) {\n"
  "    funct n2(b) {\n"
  "        funct n3(c) {\n"
  "            return a + b + c\n"
  "        }\n"
  "        return n3(3)\n"
  "    }\n"
  "    return n2(2)\n"
  "}\n"
  "post(n1(1))\n"
  "\n",
  "6", NULL, 0 },
{ "equiv: dois niveis [2/2]",
  "funct f(a, b, c) {\n"
  "    return a + b + c\n"
  "}\n"
  "post(f(1, 2, 3))\n"
  "\n",
  "6", NULL, 0 },
{ "equiv: ler chave existente [1/3]",
  "d = {\"a\": 1}\n"
  "post(d[\"a\"])\n"
  "\n",
  "1", NULL, 0 },
{ "equiv: ler chave existente [2/3]",
  "d = {\"a\": 1}\n"
  "post(d.get(\"a\"))\n"
  "\n",
  "1", NULL, 0 },
{ "equiv: ler chave existente [3/3]",
  "d = {\"a\": 1}\n"
  "post(d.a)\n"
  "\n",
  "1", NULL, 0 },
{ "equiv: erro capturado vira valor [1/3]",
  "funct f() {\n"
  "    try {\n"
  "        raise B(\"x\")\n"
  "    } catch(e) {\n"
  "        return 0\n"
  "    }\n"
  "}\n"
  "post(f())\n"
  "\n",
  "0", NULL, 0 },
{ "equiv: erro capturado vira valor [2/3]",
  "int funct g() {\n"
  "    return null\n"
  "}\n"
  "post(g())\n"
  "\n",
  "0", NULL, 0 },
{ "equiv: erro capturado vira valor [3/3]",
  "post(0)\n"
  "\n",
  "0", NULL, 0 },
{ "equiv: fatia 0:3 [1/2]",
  "post(\"abcdef\"[0:3])\n"
  "\n",
  "abc", NULL, 0 },
{ "equiv: fatia 0:3 [2/2]",
  "s = \"\"\n"
  "for each i in range(0, 3) {\n"
  "    s = s + \"abcdef\"[i]\n"
  "}\n"
  "post(s)\n"
  "\n",
  "abc", NULL, 0 },
{ "equiv: fatia 1:4 [1/2]",
  "post(\"abcdef\"[1:4])\n"
  "\n",
  "bcd", NULL, 0 },
{ "equiv: fatia 1:4 [2/2]",
  "s = \"\"\n"
  "for each i in range(1, 4) {\n"
  "    s = s + \"abcdef\"[i]\n"
  "}\n"
  "post(s)\n"
  "\n",
  "bcd", NULL, 0 },
{ "equiv: fatia 2:2 [1/2]",
  "post(\"abcdef\"[2:2])\n"
  "\n",
  "", NULL, 0 },
{ "equiv: fatia 2:2 [2/2]",
  "s = \"\"\n"
  "for each i in range(2, 2) {\n"
  "    s = s + \"abcdef\"[i]\n"
  "}\n"
  "post(s)\n"
  "\n",
  "", NULL, 0 },
{ "equiv: ordenar [1/3]",
  "l = [3, 1, 2]\n"
  "l.sort()\n"
  "post(l)\n"
  "\n",
  "[1, 2, 3]", NULL, 0 },
{ "equiv: ordenar [2/3]",
  "post(sorted([3, 1, 2]))\n"
  "\n",
  "[1, 2, 3]", NULL, 0 },
{ "equiv: ordenar [3/3]",
  "post([min([3,1,2]), 2, max([3,1,2])])\n"
  "\n",
  "[1, 2, 3]", NULL, 0 },
};
const int NC_EQUIVALENCIA = N_CASOS(CASOS_EQUIVALENCIA);
