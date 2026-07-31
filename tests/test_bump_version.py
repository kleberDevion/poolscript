"""Testes da regra de versionamento com rollover em base 100."""
import importlib.util
import sys
from pathlib import Path

import pytest

_spec = importlib.util.spec_from_file_location(
    "bump_version", Path(__file__).parent.parent / "bump_version.py"
)
bump_version = importlib.util.module_from_spec(_spec)
sys.modules["bump_version"] = bump_version
_spec.loader.exec_module(bump_version)

bump = bump_version.bump
validate = bump_version.validate


# ── incremento simples, sem rollover ────────────────────────────────────────
def test_bump_patch():
    assert bump("8.2.18") == "8.2.19"


def test_bump_patch_from_zero():
    assert bump("1.0.0") == "1.0.1"


def test_bump_patch_to_99():
    assert bump("8.2.98") == "8.2.99"


# ── rollover do 3º dígito: 99 -> vira 0 e soma no 2º (100 nunca aparece) ────
def test_rollover_patch():
    assert bump("8.2.99") == "8.3.0"


def test_rollover_patch_ext_version():
    assert bump("1.4.99") == "1.5.0"


def test_hundred_never_appears_in_patch():
    for _ in range(250):
        version = bump("8.2.99")
        assert not version.endswith(".100")


# ── rollover do 2º dígito: 99.99 -> vira 0.0 e soma no 1º ───────────────────
def test_rollover_minor():
    assert bump("8.99.99") == "9.0.0"


def test_rollover_minor_only_when_patch_also_rolls():
    # 2º dígito em 99 mas 3º ainda tem espaço — nada de rollover no 1º
    assert bump("8.99.5") == "8.99.6"


def test_sequence_across_both_rollovers():
    version = "8.98.98"
    seen = [version]
    for _ in range(4):
        version = bump(version)
        seen.append(version)
    assert seen == ["8.98.98", "8.98.99", "8.99.0", "8.99.1", "8.99.2"]


# ── nenhum dígito passa de 99 em nenhum ponto de uma sequência longa ────────
def test_no_digit_exceeds_99_over_long_run():
    version = "8.98.90"
    for _ in range(500):
        version = bump(version)
        _, minor, patch = (int(p) for p in version.split("."))
        assert minor <= 99, f"2º dígito estourou em {version}"
        assert patch <= 99, f"3º dígito estourou em {version}"


def test_bump_result_is_always_valid():
    version = "8.99.95"
    for _ in range(100):
        version = bump(version)
        assert validate(version) is None


# ── validate() reprova versões fora da regra ────────────────────────────────
def test_validate_accepts_normal():
    assert validate("8.2.18") is None


def test_validate_accepts_99():
    assert validate("8.99.99") is None


def test_validate_rejects_patch_100():
    assert validate("8.2.100") is not None


def test_validate_rejects_minor_100():
    assert validate("8.100.2") is not None


def test_validate_message_mentions_the_digit():
    assert "3º dígito" in validate("8.2.100")
    assert "2º dígito" in validate("8.100.2")


# ── as versões reais do repo respeitam a regra e estão sincronizadas ────────
def test_repo_lang_versions_are_valid_and_in_sync():
    found = bump_version.read_versions(bump_version.LANG_FILES)
    assert len(set(found.values())) == 1, f"versões da linguagem dessincronizadas: {found}"
    assert validate(next(iter(found.values()))) is None


def test_repo_ext_version_is_valid():
    found = bump_version.read_versions(bump_version.EXT_FILES)
    assert validate(next(iter(found.values()))) is None
