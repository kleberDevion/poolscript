# -*- coding: utf-8 -*-
"""Cards de hover das KEYWORDS e dos TIPOS da PoolScript (estilo Pylance).

Mesma estrutura das specs de builtins/string (gera_doc.py / gen_docs_meta.py):
nome -> dict(sig, resumo, params=[(nome,tipo,default,nota)], ret,
erros=[(Tipo,quando)], ex=[(codigo,saida)], bordas=[...]).

Para keyword/tipo, `params`/`ret`/`erros` normalmente vêm vazios — o que importa
é `sig` (a forma de uso), `resumo` e `ex`. Os exemplos são executados pelo
binário na verificação, então não podem ser falsos.
"""

def _kw(sig, resumo, ex=None, bordas=None):
    return dict(sig=sig, resumo=resumo, params=[], ret="", erros=[],
                ex=ex or [], bordas=bordas or [])


KEYWORDS_DOC = {
    # ── controle de fluxo ────────────────────────────────────────────────
    "if": _kw(
        "if (cond) { ... } elif outra: else { ... }",
        "Executa o bloco quando a condição é verdadeira. Aceita `:` ou `{}` e "
        "encadeia com `elif`/`else`.",
        ex=[('if (5 > 3) { post("maior") }', "maior")]),
    "elif": _kw(
        "if (a) { } elif (b) { } else { }",
        "Senão-se: testado só quando os `if`/`elif` anteriores falharam.",
        ex=[('x = 2\nif (x == 1) { post("um") } elif (x == 2) { post("dois") }', "dois")]),
    "else": _kw(
        "if (cond) { } else { }",
        "Bloco executado quando nenhuma condição anterior foi verdadeira.",
        ex=[('if (false) { post("a") } else { post("b") }', "b")]),
    "while": _kw(
        "while (cond) { ... }",
        "Repete o bloco enquanto a condição for verdadeira.",
        ex=[('i = 0\nwhile (i < 3) { post(i)\n i++ }', "0\n1\n2")]),
    "for": _kw(
        "for each item in colecao: ...",
        "Itera sobre lista, tupla ou string. Sempre acompanhado de `each`.",
        ex=[('for each n in [1, 2, 3] { post(n) }', "1\n2\n3")]),
    "each": _kw(
        "for each item in colecao:",
        "Parte obrigatória do `for each` — nomeia o item da iteração atual.",
        ex=[('for each c in "ab" { post(c) }', "a\nb")]),
    "in": _kw(
        "for each x in lista   |   x in colecao",
        "No `for each`, a coleção a percorrer. Como operador, testa pertencimento "
        "(retorna bool).",
        ex=[('post(2 in [1, 2, 3])', "True")]),
    "break": _kw(
        "break",
        "Sai imediatamente do laço `for`/`while` mais interno.",
        ex=[('for each n in [1, 2, 3] { if (n == 2) { break }\n post(n) }', "1")]),
    "continue": _kw(
        "continue",
        "Pula para a próxima iteração do laço, ignorando o resto do bloco.",
        ex=[('for each n in [1, 2, 3] { if (n == 2) { continue }\n post(n) }', "1\n3")]),
    "match": _kw(
        "match valor { case 1: ...  case _: ... }",
        "Casa um valor contra padrões (`case`). `case _` é o padrão coringa. Cada "
        "`case` aceita corpo em `{ }` ou em `:` + linha indentada.",
        ex=[('x = 2\nmatch x { case 1 { post("um") } case 2 { post("dois") } case _ { post("outro") } }', "dois")]),
    "case": _kw(
        "case <padrao>: ...",
        "Um ramo do `match`. Casa por valor, lista, dict, ou captura em variável; "
        "`case _` pega o resto.",
        ex=[('match 5 { case 5 { post("cinco") } }', "cinco")]),

    # ── funções e classes ────────────────────────────────────────────────
    "action": _kw(
        "action nome(a, b=default) { return ... }",
        "Declara uma função. Pode ter tipo de retorno (`int action f()`) e "
        "parâmetros com default.",
        ex=[('action somar(a, b) { return a + b }\npost(somar(2, 3))', "5")]),
    "reaction": _kw(
        "reaction nome(a, b) { return ... }",
        "Sinônimo de `action` — declara uma função, idêntico em tudo.",
        ex=[('reaction dobro(x) { return x * 2 }\npost(dobro(4))', "8")]),
    "return": _kw(
        "return <expr>   |   return;",
        "Devolve um valor da função e encerra. `return;` (sem valor) devolve Null "
        "(ou o total, dentro de `count each`).",
        ex=[('action id(x) { return x }\npost(id(42))', "42")]),
    "yield": _kw(
        "yield <expr>",
        "Torna a `action` um gerador: produz valores sob demanda, um por `yield`, "
        "sem montar a lista inteira na memória.",
        ex=[('action conta() { yield 1\n yield 2 }\nfor each v in conta() { post(v) }', "1\n2")]),
    "Entity": _kw(
        "Entity Nome() { action __init__(self, ...) { } }",
        "Declara uma classe (com herança, métodos, `private`/`public`, `@static`). "
        "O nome é seguido de `()` — dentro dos parênteses vão as Entities-pai.",
        ex=[('Entity Ponto() {\n action __init__(self, x) { self.x = x }\n}\np = Ponto(7)\npost(p.x)', "7")]),
    "class": _kw(
        "class Nome() { ... }",
        "Apelido de `Entity` — declara uma classe.",
        ex=[('class Caixa() { action __init__(self, v) { self.v = v } }\npost(Caixa(3).v)', "3")]),
    "Class": _kw(
        "Class Nome() { ... }",
        "Apelido de `Entity` (com C maiúsculo) — declara uma classe.",
        ex=[]),
    "self": _kw(
        "action metodo(self, ...) { self.campo }",
        "A instância atual — primeiro parâmetro de todo método de Entity. Acessa e "
        "define os campos com `self.campo`.",
        ex=[('Entity C() {\n action __init__(self, n) { self.n = n }\n action get(self) { return self.n }\n}\npost(C(9).get())', "9")]),
    "private": _kw(
        "private saldo: int = 0   |   private reaction _log(self)",
        "Marca um campo ou método da Entity como PRIVADO: só acessível de dentro "
        "de um método da própria classe. Acessar de fora dá erro — encapsulamento "
        "real, enforçado nos dois motores.",
        ex=[('Entity Conta() {\n'
             ' private saldo: int = 0\n'
             ' public reaction deposita(self, v) { self.saldo = self.saldo + v }\n'
             ' public reaction ver(self) { return self.saldo }\n'
             '}\n'
             'c = Conta()\n'
             'c.deposita(50)\n'
             'post(c.ver())', "50")],
        bordas=["`c.saldo` de fora da classe dá erro (`'saldo' é private de Conta`) "
                "— use um método público pra expor o valor."]),
    "public": _kw(
        "public dono: str = \"...\"   |   public reaction metodo(self)",
        "Marca um campo ou método da Entity como PÚBLICO — acessível de fora. É o "
        "padrão: não pôr `private`/`public` equivale a `public`.",
        ex=[('Entity Pessoa() {\n'
             ' public reaction __init__(self, nome) { self.nome = nome }\n'
             '}\n'
             'p = Pessoa("Ana")\n'
             'post(p.nome)', "Ana")]),
    "model": _kw(
        'model Nome { campo: tipo }',
        "Define um modelo de dados que valida um dict contra campos tipados.",
        ex=[]),

    # ── módulos ──────────────────────────────────────────────────────────
    "import": _kw(
        "import modulo   |   import modulo as apelido",
        "Carrega uma lib (stdlib, arquivo local .ps/.psl/.p ou lib instalada) e "
        "vincula o namespace.",
        ex=[('import json\npost(json.stringify([1, 2]))', "[1, 2]")]),
    "from": _kw(
        "from modulo import nome1, nome2",
        "Importa nomes específicos de um módulo direto pro escopo atual.",
        ex=[('from json import stringify\npost(stringify([1, 2, 3]))', "[1, 2, 3]")]),
    "as": _kw(
        "import modulo as apelido   |   from m import x as y",
        "Dá um apelido ao módulo ou nome importado.",
        ex=[('import json as j\npost(j.stringify(true))', "true")]),
    "PUSH": _kw(
        "PUSH modulo GET nome",
        "Sintaxe alternativa de import: `PUSH os GET getenv` ≡ "
        "`from os import getenv`.",
        ex=[]),
    "GET": _kw(
        "PUSH modulo GET nome",
        "Parte do import `PUSH ... GET ...` — nomeia o que trazer do módulo. "
        "(Também é o verbo HTTP em rotas da jinker.)",
        ex=[]),
    "global": _kw(
        "global nome",
        "Declara que o nome referencia a variável global, não uma local nova.",
        ex=[]),

    # ── erros ────────────────────────────────────────────────────────────
    "try": _kw(
        "try { ... } catch (e) { ... }",
        "Executa o bloco protegendo contra erros; se estourar, cai no `catch`.",
        ex=[('try { x = 1 / 0 } catch (e) { post("peguei") }', "peguei")]),
    "catch": _kw(
        "catch (e) { ... }   |   catch (Tipo e) { ... }",
        "Captura o erro lançado no `try`. `e` é a mensagem; pode filtrar por tipo "
        "(`catch (ConnectionError e)`).",
        ex=[('try { raise "ops" } catch (e) { post(e) }', "ops")]),
    "finally": _kw(
        "try { } catch (e) { } finally { }",
        "Bloco que executa SEMPRE ao sair do try/catch — com ou sem erro.",
        ex=[('try { post("a") } catch (e) { } finally { post("b") }', "a\nb")]),
    "raise": _kw(
        'raise "mensagem"',
        "Lança um erro, interrompendo a execução (capturável por `try`/`catch`).",
        ex=[('try { raise "falhou" } catch (e) { post(e) }', "falhou")]),
    "using": _kw(
        "using <recurso> as nome { ... }",
        "Context manager: usa um recurso e o FECHA automaticamente ao sair do "
        "bloco, mesmo se der erro (ex: `using open(...) as f`).",
        ex=[]),

    # ── operadores lógicos / valores ─────────────────────────────────────
    "and": _kw(
        "a and b",
        "E lógico: verdadeiro só quando ambos os lados são verdadeiros.",
        ex=[('post(true and false)', "False")]),
    "or": _kw(
        "a or b",
        "OU lógico: verdadeiro quando pelo menos um lado é verdadeiro.",
        ex=[('post(false or true)', "True")]),
    "not": _kw(
        "not x",
        "Negação lógica: inverte o valor booleano.",
        ex=[('post(not false)', "True")]),
    "Not": _kw(
        "Not x",
        "Apelido de `not` (com N maiúsculo) — negação lógica.",
        ex=[]),
    "is": _kw(
        "x is int   |   x is None   |   x not is None",
        "Testa o TIPO de um valor (`is int`/`is str`/...) ou identidade com "
        "`None`/`Null`. Combina com `not` (`not is`).",
        ex=[('post(5 is int)', "True")]),
    "count": _kw(
        "count int(7) in colecao   |   count int in colecao",
        "Conta ocorrências tipadas em lista/dict/string/número. Retorna `int` "
        "(0 é falsy), então combina direto em `if`.",
        ex=[('post(count int(7) in [7, 7, 7])', "3")]),
    "await": _kw(
        "await <futuro>",
        "Aguarda o resultado de uma `async action`/`reaction` (ou de uma lista "
        "delas) e devolve o valor pronto.",
        ex=[]),
    "async": _kw(
        "async action f() { ... }",
        "Marca a função pra rodar em paralelo (thread pool); a chamada devolve um "
        "futuro que você resolve com `await`.",
        ex=[]),
}


TYPES_DOC = {
    "str": _kw(
        'str nome = "texto"   |   str(x)',
        "Texto (string). Como tipo declarado, cobra/converte; chamado como função, "
        "converte qualquer valor pra texto.",
        ex=[('str s = "oi"\npost(s.upper())', "OI")]),
    "int": _kw(
        "int n = 10   |   int(x)",
        "Número inteiro. Declarado, converte string numérica/float/bool; chamado, "
        "converte (float trunca).",
        ex=[('post(int("7") + 1)', "8")]),
    "flo": _kw(
        "flo x = 2.5   |   flo(x)",
        "Número de ponto flutuante. O nome é `flo`, não `float`.",
        ex=[('post(flo(3))', "3.0")]),
    "bool": _kw(
        "bool ok = True   |   bool(x)",
        "Verdadeiro/falso (`True`/`False`). Conta como `int` (0/1) em aritmética.",
        ex=[('post(bool(0), bool(1))', "False True")]),
    "list": _kw(
        "list xs = [1, 2, 3]   |   list(x)",
        "Lista ordenada e mutável. Tem métodos: `append`, `pop`, `sort`, ...",
        ex=[('list xs = [3, 1, 2]\nxs.sort()\npost(xs)', "[1, 2, 3]")]),
    "json": _kw(
        'json d = {"k": 1}',
        "Objeto chave→valor (dict). `json` e `dict` são o MESMO tipo.",
        ex=[('json d = {"a": 1}\npost(d["a"])', "1")]),
    "dict": _kw(
        'dict d = {"k": 1}',
        "Objeto chave→valor. Apelido de `json` — o mesmo tipo.",
        ex=[('dict d = {"x": 9}\npost(d["x"])', "9")]),
    "tup": _kw(
        "tup t = (1, 2, 3)",
        "Tupla: sequência ordenada e IMUTÁVEL.",
        ex=[('t = (1, 2)\npost(t[0])', "1")]),
    "type": _kw(
        "type(x)",
        "Devolve o nome do tipo de um valor, como string.",
        ex=[('post(type(5), type("a"), type([]))', "int str list")]),
}
