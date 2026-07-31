"""Módulo `os` em C — mesma resposta que o `os_lib.py`.

Diferente dos outros módulos migrados, este mexe no disco: quase todo caso
precisa de uma árvore de arquivos montada antes e de um diretório corrente
apontando pra ela. Por isso cada caso roda **duas vezes numa caixa limpa** —
uma para o interpretador, outra para a VM —, senão o primeiro motor deixaria
o disco alterado para o segundo e a comparação mediria a ordem, não o
comportamento.

A comparação é feita sobre o **stdout inteiro**, não sobre `interp.output`:
`os.warn` e `os.ipmach` escrevem direto no terminal e não passam pelo `post`.
"""
import io
import os
import sys
from contextlib import redirect_stdout

import pytest

from poolscript.interpreter import Interpreter
from poolscript.parser import parse_source

vm = pytest.importorskip(
    "poolscript.vm.poolscript_vm",
    reason="extensão C não compilada — rode: ./rebuild_vm.sh",
)

NL = chr(10)


# ── a caixa ─────────────────────────────────────────────────────────────────

@pytest.fixture
def caixa(tmp_path, monkeypatch):
    """Devolve uma função que monta a árvore de teste num diretório novo.

    O nome do diretório termina em `caixa` de propósito: alguns casos checam
    `os.cwd()` pelo sufixo.
    """
    contador = {"n": 0}

    def monta():
        contador["n"] += 1
        raiz = tmp_path / ("t%d" % contador["n"]) / "caixa"
        (raiz / "sub" / "fundo").mkdir(parents=True)
        (raiz / "dados.txt").write_text("linha um" + NL + "linha dois", encoding="utf-8")
        (raiz / "cfg.json").write_text('{"a": 1, "b": [2, 3]}', encoding="utf-8")
        (raiz / "tab.csv").write_text("nome,idade" + NL + "ana,30" + NL + "bo,7" + NL, encoding="utf-8")
        (raiz / "rag.csv").write_text("a,b,c" + NL + "1,2" + NL + "1,2,3,4" + NL, encoding="utf-8")
        (raiz / "asp.csv").write_text('a,b' + NL + '"x,y",2' + NL + '"di""z",3' + NL, encoding="utf-8")
        (raiz / "cru.txt").write_text("sem quebra no fim", encoding="utf-8")
        (raiz / "quebrado.json").write_text("{nao e json", encoding="utf-8")
        (raiz / "img.png").write_bytes(bytes([137, 80, 78, 71, 13, 10, 26, 10, 0, 1]))
        (raiz / "sub" / "dentro.txt").write_text("x", encoding="utf-8")
        (raiz / "sub" / "fundo" / "fundo.txt").write_text("y", encoding="utf-8")
        return raiz

    monkeypatch.setenv("PS_TESTE_X", "valor-x")
    monkeypatch.delenv("PS_NAO_EXISTE_ZZZ", raising=False)
    return monta


def via_interpretador(src, dirbase):
    velho = os.getcwd()
    os.chdir(dirbase)
    try:
        buf = io.StringIO()
        i = Interpreter(source=src, filename=str(dirbase / "t.ps"))
        with redirect_stdout(buf):
            i.run(parse_source(src, "<t>"))
        return buf.getvalue().splitlines()
    finally:
        os.chdir(velho)


def via_c(src, dirbase):
    velho = os.getcwd()
    os.chdir(dirbase)
    sys.stdout.flush()
    r, w = os.pipe()
    original = os.dup(1)
    try:
        os.dup2(w, 1)
        os.close(w)
        try:
            # o caminho é o que dá ao `pathFile` a pasta do script como raiz
            # de busca — sem ele a VM só enxergaria o diretório corrente
            vm.executa_fonte(src, str(dirbase / "t.ps"))
        finally:
            sys.stdout.flush()
            os.dup2(original, 1)
    finally:
        os.close(original)
        os.chdir(velho)
    with os.fdopen(r, "rb") as f:
        return f.read().decode("utf-8", "replace").splitlines()


@pytest.fixture
def mesmo(caixa):
    def _mesmo(src):
        esperado = via_interpretador(src, caixa())
        obtido = via_c(src, caixa())
        assert obtido == esperado
    return _mesmo


@pytest.fixture
def ambos_falham(caixa):
    def _ambos(src):
        with pytest.raises(Exception):
            via_interpretador(src, caixa())
        with pytest.raises(Exception):
            via_c(src, caixa())
    return _ambos


IMP = "import os" + NL


# ── existência, tipo e tamanho ──────────────────────────────────────────────

@pytest.mark.parametrize("src", [
    'post(os.exists("dados.txt"))',
    'post(os.exists("nada.txt"))',
    'post(os.exists("sub"))',
    'post(os.exists("sub/fundo"))',
    'post(os.isfile("dados.txt"), os.isdir("dados.txt"))',
    'post(os.isdir("sub"), os.isfile("sub"))',
    'post(os.isfile("sub/dentro.txt"))',
    'post(os.size("dados.txt"))',
])
def test_consulta(src, mesmo):
    mesmo(IMP + src)


@pytest.mark.parametrize("src", [
    'post(os.size("nada.txt"))',
    'post(os.exists(5))',
    'post(os.mkdir(5))',
])
def test_consulta_invalida(src, ambos_falham):
    ambos_falham(IMP + src)


# ── loadFile: o tipo sai da extensão ────────────────────────────────────────

@pytest.mark.parametrize("src", [
    'post(os.loadFile("dados.txt"))',
    'post(os.loadFile("cru.txt"))',
    'post(os.loadFile("dados.txt", "utf-8"))',
])
def test_loadfile_texto(src, mesmo):
    mesmo(IMP + src)


@pytest.mark.parametrize("src", [
    'post(os.loadFile("cfg.json"))',
    'post(os.loadFile("cfg.json")["a"])',
    'post(type(os.loadFile("cfg.json")))',
])
def test_loadfile_json_vira_dict(src, mesmo):
    """`.json` não volta como texto: volta já decodificado."""
    mesmo(IMP + src)


@pytest.mark.parametrize("src", [
    'post(os.loadFile("tab.csv"))',
    'post(os.loadFile("tab.csv")[0]["nome"])',
    'post(os.loadFile("asp.csv"))',
    'post(os.loadFile("rag.csv"))',
])
def test_loadfile_csv_vira_lista_de_dict(src, mesmo):
    """Igual ao `csv.DictReader`: aspas protegem a vírgula, `""` é uma aspa
    literal, linha curta completa com `null` e o que sobra cai na chave
    `null`."""
    mesmo(IMP + src)


@pytest.mark.parametrize("src", [
    'post(type(os.loadFile("img.png")))',
    'post(os.loadFile("img.png", "rb").name)',
])
def test_loadfile_binario_vira_poolfile(src, mesmo):
    mesmo(IMP + src)


@pytest.mark.parametrize("src", [
    'post(os.loadFile("nada.txt"))',
    'post(os.loadFile("img.png", "utf-8"))',      # binário forçado a texto
    'post(os.loadFile("dados.txt", "rb"))',       # texto forçado a binário
    'post(os.loadFile("quebrado.json"))',
])
def test_loadfile_recusa(src, ambos_falham):
    ambos_falham(IMP + src)


# ── PoolFile ────────────────────────────────────────────────────────────────

@pytest.mark.parametrize("src", [
    'f = os.loadFile("img.png")' + NL + 'post(f.name, f.ext, f.size)',
    'f = os.loadFile("img.png")' + NL + 'post(f.name.upper())',
    'f = os.loadFile("img.png")' + NL + 'post(f.size + 1)',
    'f = os.loadFile("img.png")' + NL + 'post(len(f.bytes()), f.bytes()[0])',
    'f = os.loadFile("img.png")' + NL + 'post(f.path().endswith("img.png"))',
])
def test_poolfile_campos(src, mesmo):
    """`name`, `ext` e `size` são campos — sem parêntese. `bytes` e `path`
    são métodos."""
    mesmo(IMP + src)


@pytest.mark.parametrize("src", [
    'f = os.loadFile("img.png")' + NL + 'g = f.copy("dup.png")' + NL
    + 'post(g.name, os.exists("dup.png"), os.exists("img.png"))',
    'f = os.loadFile("img.png")' + NL + 'g = f.move("outro.png")' + NL
    + 'post(g.name, os.exists("outro.png"), os.exists("img.png"))',
    'f = os.loadFile("img.png")' + NL + 'f.move("sub/nova/x.png")' + NL
    + 'post(os.exists("sub/nova/x.png"))',
    'f = os.loadFile("img.png")' + NL + 'post(f.delete(), os.exists("img.png"))',
])
def test_poolfile_operacoes(src, mesmo):
    """`move` e `copy` criam a pasta de destino se ela não existir."""
    mesmo(IMP + src)


def test_poolfile_atributo_inexistente(ambos_falham):
    ambos_falham(IMP + 'f = os.loadFile("img.png")' + NL + 'post(f.naoexiste)')


@pytest.mark.parametrize("src", [
    'post(os.loadFile("img.png") is PoolFile)',
    'post(os.loadFile("img.png") is os.PoolFile)',
    'post(os.loadFile("dados.txt") is PoolFile)',
    'post("x" is PoolFile)',
    'post(os.loadFile("dados.txt") is str)',
])
def test_poolfile_como_tipo(src, mesmo):
    """`PoolFile` vale como tipo em `is` — global e via módulo."""
    mesmo(IMP + src)


def test_poolfile_nao_vaza_a_classe_do_python(mesmo):
    """Imprimir a referência de tipo tem que dar `PoolFile`, não
    `<class 'poolscript.stdlib.os_lib.PoolFile'>` — o caminho do módulo Python
    não existe na linguagem."""
    mesmo(IMP + 'post(PoolFile)' + NL + 'post(type(PoolFile))')


# ── busca recursiva ─────────────────────────────────────────────────────────

@pytest.mark.parametrize("src", [
    'post(os.pathFile("dados.txt").endswith("dados.txt"))',
    'post(os.pathFile("fundo.txt").endswith("fundo.txt"))',   # dois níveis abaixo
    'post(os.pathFolder("sub").endswith("sub"))',
    'post(os.pathFolder("fundo").endswith("fundo"))',
    'post(os.loadFile("sub/fundo/fundo.txt"))',
    'post(os.loadFile("fundo.txt"))',                          # achado pela busca
])
def test_busca(src, mesmo):
    """`pathFile`/`pathFolder` descem a árvore a partir da pasta do script."""
    mesmo(IMP + src)


@pytest.mark.parametrize("src", [
    'post(os.pathFile("naoexiste.zzz"))',
    'post(os.pathFolder("naoexiste"))',
    'post(os.pathFile("sub"))',        # é pasta, não arquivo
])
def test_busca_sem_resultado(src, ambos_falham):
    ambos_falham(IMP + src)


# ── criar, remover, mover ───────────────────────────────────────────────────

@pytest.mark.parametrize("src", [
    'os.mkdir("nova")' + NL + 'post(os.isdir("nova"))',
    'os.mkdir("a/b/c")' + NL + 'post(os.isdir("a/b/c"))',        # cadeia inteira
    'os.mkdir("sub", true)' + NL + 'post("ok")',
    'os.mkdir("vazia")' + NL + 'os.rmdir("vazia")' + NL + 'post(os.exists("vazia"))',
    'os.rmdir("sub", true)' + NL + 'post(os.exists("sub"))',     # com conteúdo
    'os.copy("dados.txt", "c.txt")' + NL + 'post(os.loadFile("c.txt"))',
    'os.rename("dados.txt", "r.txt")' + NL + 'post(os.exists("dados.txt"), os.exists("r.txt"))',
    'os.move("dados.txt", "m.txt")' + NL + 'post(os.exists("dados.txt"), os.loadFile("m.txt"))',
])
def test_escrita(src, mesmo):
    mesmo(IMP + src)


@pytest.mark.parametrize("src", [
    'os.mkdir("sub")',                 # já existe e sem exist_ok
    'os.rmdir("sub")',                 # tem conteúdo e sem force
    'os.rmdir("naoexiste")',
    'os.rename("naoexiste", "x")',
    'os.copy("naoexiste", "x")',
    'os.move("naoexiste", "x")',
    'os.copy("sub", "x")',             # origem é pasta
])
def test_escrita_recusada(src, ambos_falham):
    """`rmdir` sem `force` só remove pasta vazia, e `mkdir` sem `exist_ok`
    reclama de pasta existente — apagar ou sobrescrever sem o usuário pedir
    é destrutivo demais para ser o padrão."""
    ambos_falham(IMP + src)


# ── listar ──────────────────────────────────────────────────────────────────

@pytest.mark.parametrize("src", [
    'post(os.ls("sub")[0]["type"])',
    'post(len(os.ls(".")) > 0)',
    'l = os.ls("sub")' + NL + 'post(l[0].keys())',
])
def test_ls(src, mesmo):
    """Cada item é um dict com `name`, `type` e `size`."""
    mesmo(IMP + src)


def test_ls_dir_inexistente(ambos_falham):
    ambos_falham(IMP + 'post(os.ls("naoexiste"))')


# ── diretório corrente ──────────────────────────────────────────────────────

@pytest.mark.parametrize("src", [
    'post(os.cwd().endswith("caixa"))',
    'post(os.cwd() is str)',
    'os.chdir("sub")' + NL + 'post(os.cwd().endswith("sub"))',
    'os.chdir("sub")' + NL + 'post(os.exists("dentro.txt"))',
])
def test_cwd_e_chdir(src, mesmo):
    mesmo(IMP + src)


def test_chdir_inexistente(ambos_falham):
    ambos_falham(IMP + 'os.chdir("naoexiste")')


# ── ambiente ────────────────────────────────────────────────────────────────

@pytest.mark.parametrize("src", [
    'post(os.getenv("PS_TESTE_X"))',
    'post(os.getenv("PS_NAO_EXISTE_ZZZ"))',
    'post(os.getenv("PS_NAO_EXISTE_ZZZ", "padrao"))',
    'post(os.getenv("PS_TESTE_X", "padrao"))',
    'post(os.environ("PS_TESTE_X"))',
    'post(os.environ("PS_NAO_EXISTE_ZZZ"))',
    'post(type(os.environ()))',
    'post(os.environ()["PS_TESTE_X"])',
    'post(len(os.environ()) > 3)',
])
def test_ambiente(src, mesmo):
    """`environ()` sem argumento devolve o dict inteiro; com chave, o valor.
    Chave ausente é `null`, não erro."""
    mesmo(IMP + src)


# ── processos ───────────────────────────────────────────────────────────────

@pytest.mark.parametrize("src", [
    'post(os.cmd("echo ola", true))',
    'post(os.cmd("echo a; echo b", true))',      # `;` é do shell
    'post(os.cmd("echo    varios   espacos", true))',
    'post(os.cmd("exit 3", true))',
    'post(os.cmd("ls naoexistezzz 2>&1", true).contains("No such"))',
])
def test_cmd_passa_pelo_shell(src, mesmo):
    mesmo(IMP + src)


@pytest.mark.parametrize("src", [
    'post(os.run(["echo", "oi"], true))',
    'post(os.run("echo oi", true))',
    'post(os.run(["echo", "a b"], true))',       # espaço não vira dois args
    'post(os.run(["sh", "-c", "echo x"], true))',
])
def test_run_nao_passa_pelo_shell(src, mesmo):
    """`run` executa o binário direto. É a forma segura quando algum pedaço do
    comando veio do usuário: `;` e `$` não são interpretados."""
    mesmo(IMP + src)


def test_cmd_sem_captura_devolve_null(caixa):
    """Sem `capture` a saída do processo vai direto pro terminal e a chamada
    devolve `null`. Não dá pra comparar com o interpretador: lá o filho herda
    o descritor 1 real, que o `redirect_stdout` não intercepta."""
    assert via_c(IMP + 'post(os.cmd("echo silencio"))', caixa()) == ["silencio", "null"]


# ── saída no terminal ───────────────────────────────────────────────────────

@pytest.mark.parametrize("src", [
    'os.warn("aviso")',
    'os.warn("aviso", "red")',
    'os.warn("aviso", "roxo")',      # cor desconhecida cai no padrão
    'os.warn()',
    'os.warn(123)',
])
def test_warn(src, mesmo):
    """A cor sai como código ANSI no stdout — o teste compara os bytes, então
    uma troca de cor no meio quebraria aqui."""
    mesmo(IMP + src)


def test_ipmach_devolve_endereco(caixa):
    """Não dá para comparar com o interpretador byte a byte (a rota pode mudar
    entre as duas execuções), então o que se checa é o formato."""
    linhas = via_c(IMP + 'ip = os.ipmach()' + NL + 'post(ip.count(".") == 3)', caixa())
    assert linhas[-1] == "True"
