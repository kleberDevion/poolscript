"""Módulo `bytes` — criar/converter/manipular sequências de bytes.

Testa o comportamento no interpretador (a autoridade). A paridade byte-a-byte
com a VM em C vive em test_binario_c.py (test_bytes_*).
"""
import pytest

from poolscript.interpreter import Interpreter, PoolRuntimeError
from poolscript.parser import parse_source


def run(src):
    interp = Interpreter(source=src)
    interp.run(parse_source(src))
    return interp.output


def uma(expr):
    """Roda `import bytes` + um único post(expr) e devolve a linha impressa."""
    return run("import bytes" + "\n" + f"post({expr})")[0]


# ── criação ─────────────────────────────────────────────────────────────────

def test_new_de_lista():
    assert uma("bytes.new([72, 105])") == "b'Hi'"


def test_new_de_texto():
    assert uma('bytes.new("Oi")') == "b'Oi'"


def test_new_de_tamanho():
    assert uma("bytes.new(3)") == r"b'\x00\x00\x00'"


def test_new_vazio():
    assert uma("bytes.new()") == "b''"


def test_new_copia_bytes():
    assert uma('bytes.new(bytes.new("ok"))') == "b'ok'"


def test_new_lista_fora_do_range_erra():
    with pytest.raises(PoolRuntimeError) as e:
        run('import bytes\nx = bytes.new([300])')
    assert "inteiros de 0 a 255" in str(e.value)


def test_new_tipo_invalido_erra():
    with pytest.raises(PoolRuntimeError) as e:
        run("import bytes\nx = bytes.new(3.5)")
    assert "não sei criar bytes de flo" in str(e.value)


# ── hex ──────────────────────────────────────────────────────────────────────

def test_fromhex_e_hex():
    assert uma('bytes.hex(bytes.fromhex("48 65 6c 6c 6f"))') == "48656c6c6f"


def test_fromhex_invalido_erra():
    with pytest.raises(PoolRuntimeError) as e:
        run('import bytes\nx = bytes.fromhex("zz")')
    assert "hex inválido" in str(e.value)


# ── base64 ───────────────────────────────────────────────────────────────────

def test_base64_ida_e_volta():
    assert uma('bytes.base64(bytes.new("Hello"))') == "SGVsbG8="
    assert uma('bytes.frombase64("SGVsbG8=")') == "b'Hello'"


# ── inteiros ─────────────────────────────────────────────────────────────────

def test_fromint_minimo():
    assert uma("bytes.hex(bytes.fromint(258))") == "0102"


def test_fromint_largura_e_ordem():
    assert uma("bytes.hex(bytes.fromint(258, 4))") == "00000102"
    assert uma('bytes.hex(bytes.fromint(258, 4, "little"))') == "02010000"


def test_toint_ida_e_volta():
    assert uma("bytes.toint(bytes.fromint(70000, 4))") == "70000"


def test_fromint_nao_cabe_erra():
    with pytest.raises(PoolRuntimeError) as e:
        run("import bytes\nx = bytes.fromint(70000, 1)")
    assert "não cabe em 1 byte" in str(e.value)


# ── lista / índice / fatia ───────────────────────────────────────────────────

def test_tolist():
    assert uma('bytes.tolist(bytes.new("ABC"))') == "[65, 66, 67]"


def test_get_positivo_e_negativo():
    assert uma('bytes.get(bytes.new("ABC"), 0)') == "65"
    assert uma('bytes.get(bytes.new("ABC"), -1)') == "67"


def test_get_fora_do_range_erra():
    with pytest.raises(PoolRuntimeError) as e:
        run('import bytes\nx = bytes.get(bytes.new("ab"), 9)')
    assert "fora do range" in str(e.value)


def test_slice():
    assert uma('bytes.slice(bytes.new("Hello"), 1, 3)') == "b'el'"
    assert uma('bytes.slice(bytes.new("Hello"), 2)') == "b'llo'"


# ── concat / xor ─────────────────────────────────────────────────────────────

def test_concat():
    assert uma('bytes.concat([bytes.new("Hi"), bytes.new("!!")])') == "b'Hi!!'"


def test_concat_item_nao_bytes_erra():
    with pytest.raises(PoolRuntimeError) as e:
        run('import bytes\nx = bytes.concat([bytes.new("a"), 5])')
    assert "não é bytes" in str(e.value)


def test_xor_chave_repetida_e_reversivel():
    # a ^ K ^ K == a
    volta = 'bytes.hex(bytes.xor(bytes.xor(bytes.new("secret"), bytes.new("KEY")), bytes.new("KEY")))'
    assert uma(volta) == "736563726574"   # "secret" em hex


def test_xor_chave_vazia_erra():
    with pytest.raises(PoolRuntimeError) as e:
        run('import bytes\nx = bytes.xor(bytes.new("a"), bytes.new())')
    assert "chave vazia" in str(e.value)
