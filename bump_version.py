#!/usr/bin/env python3
"""Valida e incrementa as versões do projeto seguindo a regra de rollover.

Regra: os dígitos 2 e 3 vão de 0 a 99. Ao chegar em 100 eles zeram e somam
1 no dígito à esquerda (base 100, não semver):

    8.2.18  -> 8.2.19
    8.2.99  -> 8.3.0
    8.99.99 -> 9.0.0

A versão da linguagem vive em TRÊS arquivos que precisam bater entre si
(pyproject.toml, src/poolscript/__init__.py, installer/pool_installer.iss);
a da extensão VS Code vive em psl-poolscript-vsix/package.json e é
independente da linguagem.

Uso:
    python bump_version.py                # valida tudo, não altera nada
    python bump_version.py lang           # +1 na versão da linguagem
    python bump_version.py ext            # +1 na versão da extensão
    python bump_version.py lang ext       # ambas
"""
from __future__ import annotations

import re
import sys
from pathlib import Path

MAX_DIGIT = 99
ROOT = Path(__file__).parent

# (arquivo, regex com um grupo capturando só a versão)
LANG_FILES = [
    (ROOT / "pyproject.toml", re.compile(r'^version\s*=\s*"(\d+\.\d+\.\d+)"', re.M)),
    (ROOT / "src/poolscript/__init__.py", re.compile(r'^__version__\s*=\s*"(\d+\.\d+\.\d+)"', re.M)),
    (ROOT / "installer/pool_installer.iss", re.compile(r'^#define MyAppVersion\s+"(\d+\.\d+\.\d+)"', re.M)),
    # a doc cita a versão no título, no exemplo do `pool --version` e no banner
    # do REPL — todas sobem juntas pra doc nunca ficar defasada (era um problema)
    (ROOT / "docs/PoolScript.md", re.compile(r'PoolScript\s+v(\d+\.\d+\.\d+)')),
]
EXT_FILES = [
    (ROOT / "psl-poolscript-vsix/package.json", re.compile(r'^\s*"version":\s*"(\d+\.\d+\.\d+)"', re.M)),
]


def parse(version: str) -> tuple[int, int, int]:
    major, minor, patch = (int(p) for p in version.split("."))
    return major, minor, patch


def validate(version: str) -> str | None:
    """Devolve mensagem de erro se a versão viola a regra, senão None."""
    _, minor, patch = parse(version)
    if patch > MAX_DIGIT:
        return f"3º dígito {patch} passou de {MAX_DIGIT} (deveria ter virado rollover)"
    if minor > MAX_DIGIT:
        return f"2º dígito {minor} passou de {MAX_DIGIT} (deveria ter virado rollover)"
    return None


def bump(version: str) -> str:
    major, minor, patch = parse(version)
    patch += 1
    if patch > MAX_DIGIT:
        patch = 0
        minor += 1
    if minor > MAX_DIGIT:
        minor = 0
        major += 1
    return f"{major}.{minor}.{patch}"


def read_versions(files) -> dict[Path, str]:
    found = {}
    for path, pattern in files:
        text = path.read_text(encoding="utf-8")
        # um arquivo pode citar a versão em vários lugares (ex: a doc, que a
        # mostra no título, no `pool --version` e no banner do REPL) — todas
        # têm que concordar, senão o sync não teria o que significar.
        matches = pattern.findall(text)
        if not matches:
            raise SystemExit(f"[erro] não achei a versão em {path}")
        distintas = set(matches)
        if len(distintas) > 1:
            raise SystemExit(
                f"[erro] {path.relative_to(ROOT)} tem versões divergentes "
                f"dentro do próprio arquivo: {sorted(distintas)}")
        found[path] = matches[0]
    return found


def check(name: str, files) -> str:
    """Confere que todos os arquivos do grupo têm a MESMA versão e que ela
    respeita a regra. Devolve a versão atual."""
    found = read_versions(files)
    distinct = set(found.values())
    if len(distinct) > 1:
        detail = "\n".join(f"    {p.relative_to(ROOT)}: {v}" for p, v in found.items())
        raise SystemExit(f"[erro] versões de '{name}' estão dessincronizadas:\n{detail}")

    version = distinct.pop()
    problem = validate(version)
    if problem:
        raise SystemExit(f"[erro] versão de '{name}' ({version}) é inválida: {problem}")
    print(f"{name}: {version} (ok)")
    return version


def apply(name: str, files, current: str) -> None:
    new = bump(current)
    for path, pattern in files:
        text = path.read_text(encoding="utf-8")

        # substitui SÓ o grupo 1 (a versão) em TODAS as ocorrências — a doc
        # cita a versão em vários pontos e todos têm que subir juntos.
        def troca(m):
            ini, fim = m.span(1)
            base = m.start()
            return m.group(0)[:ini - base] + new + m.group(0)[fim - base:]

        novo_texto, n = pattern.subn(troca, text)
        path.write_text(novo_texto, encoding="utf-8")
        sufixo = f" ({n}x)" if n > 1 else ""
        print(f"  {path.relative_to(ROOT)}: {current} -> {new}{sufixo}")
    print(f"{name}: {current} -> {new}")


def main() -> int:
    targets = sys.argv[1:]
    unknown = [t for t in targets if t not in {"lang", "ext"}]
    if unknown:
        raise SystemExit(f"[erro] alvo desconhecido: {', '.join(unknown)} (use 'lang' e/ou 'ext')")

    groups = {"lang": LANG_FILES, "ext": EXT_FILES}
    current = {name: check(name, files) for name, files in groups.items()}

    if not targets:
        print("\nnada alterado (rode com 'lang' e/ou 'ext' para incrementar)")
        return 0

    print()
    for name in targets:
        apply(name, groups[name], current[name])
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
