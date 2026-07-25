"""
Gerenciador de pacotes da PoolScript — comandos `psl install/uninstall/list/registry`.

Três modos de instalação:
  - comando global  (padrão):        psl install foo.ps
  - lib importável  (`-asLib`):      psl install foo.ps -asLib
  - lib Python      (`-py`):         psl install foo -py

Alvos terminados em `.ps` são resolvidos como arquivo local; qualquer outro
nome (sem `-py`) é procurado num índice de registro configurável pelo
usuário (`psl registry set-url <url>`), sem depender de nenhum índice de
terceiros.

Estado fica em `~/.poolscript/` (ou `$POOLSCRIPT_HOME`, usado pelos testes
para nunca tocar o home real):

    commands/   cópias de .ps instaladas como comando global
    libs/       cópias de .ps instaladas como lib importável
    bin/        shims .cmd (esse dir entra no PATH do usuário)
    installed.json      o que está instalado, de onde, quando
    config.json          {"registry_url": "..."}
    registry_cache.json   cache do índice remoto
"""
from __future__ import annotations

import json
import os
import sys
import time
from dataclasses import dataclass
from pathlib import Path

_CACHE_TTL_SECONDS = 3600


class PkgmgrError(Exception):
    """Erro amigável do gerenciador de pacotes — nunca deve virar traceback."""


@dataclass
class Paths:
    home: Path
    commands: Path
    libs: Path
    bin: Path
    installed_file: Path
    config_file: Path
    cache_file: Path


def _home() -> Path:
    override = os.environ.get("POOLSCRIPT_HOME")
    if override:
        return Path(override)
    return Path.home() / ".poolscript"


def _paths() -> Paths:
    home = _home()
    paths = Paths(
        home=home,
        commands=home / "commands",
        libs=home / "libs",
        bin=home / "bin",
        installed_file=home / "installed.json",
        config_file=home / "config.json",
        cache_file=home / "registry_cache.json",
    )
    for d in (paths.commands, paths.libs, paths.bin):
        d.mkdir(parents=True, exist_ok=True)
    return paths


# ── installed.json ──────────────────────────────────────────────────────────

def _load_installed() -> dict:
    p = _paths()
    if not p.installed_file.exists():
        return {"commands": {}, "libs": {}, "py": {}}
    try:
        return json.loads(p.installed_file.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError):
        return {"commands": {}, "libs": {}, "py": {}}


def _save_installed(data: dict) -> None:
    p = _paths()
    p.installed_file.write_text(json.dumps(data, indent=2, ensure_ascii=False), encoding="utf-8")


# ── config.json ──────────────────────────────────────────────────────────────

def _load_config() -> dict:
    p = _paths()
    if not p.config_file.exists():
        return {}
    try:
        return json.loads(p.config_file.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError):
        return {}


def _save_config(data: dict) -> None:
    p = _paths()
    p.config_file.write_text(json.dumps(data, indent=2, ensure_ascii=False), encoding="utf-8")


# ── registro (índice remoto) ──────────────────────────────────────────────────

def registry_set_url(url: str) -> None:
    cfg = _load_config()
    cfg["registry_url"] = url
    _save_config(cfg)


def registry_show() -> str:
    cfg = _load_config()
    url = cfg.get("registry_url")
    if not url:
        return "nenhum registro configurado — use `psl registry set-url <url>`"
    return f"registro atual: {url}"


def _fetch_registry_index(url: str) -> dict:
    import urllib.request
    import urllib.error

    try:
        with urllib.request.urlopen(url, timeout=10) as resp:
            raw = resp.read().decode("utf-8")
    except urllib.error.URLError as e:
        raise PkgmgrError(f"não foi possível buscar o registro em {url}: {e}") from None
    try:
        index = json.loads(raw)
    except json.JSONDecodeError as e:
        raise PkgmgrError(f"registro em {url} não é um JSON válido: {e}") from None
    if not isinstance(index, dict):
        raise PkgmgrError(f"registro em {url} tem formato inválido (esperado objeto nome→url)")
    return index


def _registry_index() -> dict:
    cfg = _load_config()
    url = cfg.get("registry_url")
    if not url:
        raise PkgmgrError(
            "nenhum registro configurado — use `psl registry set-url <url>` "
            "ou instale a partir de um arquivo local (nome.ps)"
        )

    p = _paths()
    if p.cache_file.exists():
        try:
            cached = json.loads(p.cache_file.read_text(encoding="utf-8"))
            if cached.get("url") == url and (time.time() - cached.get("fetched_at", 0)) < _CACHE_TTL_SECONDS:
                return cached["index"]
        except (OSError, json.JSONDecodeError, KeyError):
            pass

    index = _fetch_registry_index(url)
    p.cache_file.write_text(
        json.dumps({"url": url, "fetched_at": time.time(), "index": index}, ensure_ascii=False),
        encoding="utf-8",
    )
    return index


def _registry_lookup(name: str) -> str:
    index = _registry_index()
    if name not in index:
        raise PkgmgrError(f"pacote '{name}' não encontrado no registro")
    return index[name]


# ── resolução de origem ───────────────────────────────────────────────────────

def resolve_source(target: str) -> tuple[str, str]:
    """Retorna ('local', caminho) ou ('registry', url_do_source)."""
    if target.endswith(".ps"):
        path = Path(target)
        if not path.is_file():
            raise PkgmgrError(f"arquivo não encontrado: {target}")
        return "local", str(path)
    url = _registry_lookup(target)
    return "registry", url


def _read_source_text(kind: str, location: str) -> str:
    if kind == "local":
        return Path(location).read_text(encoding="utf-8")
    import urllib.request
    import urllib.error

    try:
        with urllib.request.urlopen(location, timeout=10) as resp:
            return resp.read().decode("utf-8")
    except urllib.error.URLError as e:
        raise PkgmgrError(f"não foi possível baixar {location}: {e}") from None


def _derive_name(target: str) -> str:
    stem = Path(target).stem if target.endswith(".ps") else target
    return stem


# ── instalação: comando global ───────────────────────────────────────────────

def install_command(target: str, name_override: str | None = None) -> str:
    kind, location = resolve_source(target)
    name = name_override or _derive_name(target)
    source_text = _read_source_text(kind, location)

    p = _paths()
    dest = p.commands / f"{name}.ps"
    dest.write_text(source_text, encoding="utf-8")
    _write_shim(name, dest)

    data = _load_installed()
    data["commands"][name] = {"source": f"{kind}:{location}", "installed_at": _now()}
    _save_installed(data)

    path_note = _ensure_bin_on_path()
    msg = f"comando '{name}' instalado ({dest})"
    if path_note:
        msg += f"\n{path_note}"
    return msg


def uninstall_command(name: str) -> str:
    p = _paths()
    ps_file = p.commands / f"{name}.ps"
    shim = p.bin / f"{name}.cmd"
    removed = False
    for f in (ps_file, shim):
        if f.exists():
            f.unlink()
            removed = True

    data = _load_installed()
    if name in data["commands"]:
        del data["commands"][name]
        _save_installed(data)
        removed = True

    if not removed:
        raise PkgmgrError(f"comando '{name}' não está instalado")
    return f"comando '{name}' removido"


def _write_shim(name: str, ps_path: Path) -> None:
    p = _paths()
    shim = p.bin / f"{name}.cmd"
    shim.write_text(f'@echo off\r\npool "{ps_path}" %*\r\n', encoding="utf-8")


# ── instalação: lib importável ────────────────────────────────────────────────

def install_lib(target: str, name_override: str | None = None) -> str:
    kind, location = resolve_source(target)
    name = name_override or _derive_name(target)
    source_text = _read_source_text(kind, location)

    p = _paths()
    dest = p.libs / f"{name}.ps"
    dest.write_text(source_text, encoding="utf-8")

    data = _load_installed()
    data["libs"][name] = {"source": f"{kind}:{location}", "installed_at": _now()}
    _save_installed(data)

    return f"lib '{name}' instalada — disponível via `import {name}` em qualquer script ({dest})"


def uninstall_lib(name: str) -> str:
    p = _paths()
    ps_file = p.libs / f"{name}.ps"
    removed = False
    if ps_file.exists():
        ps_file.unlink()
        removed = True

    data = _load_installed()
    if name in data["libs"]:
        del data["libs"][name]
        _save_installed(data)
        removed = True

    if not removed:
        raise PkgmgrError(f"lib '{name}' não está instalada")
    return f"lib '{name}' removida"


def global_lib_path(module_name: str) -> Path | None:
    """Usado pelo interpretador para resolver `import <nome>` de libs instaladas globalmente."""
    candidate = _paths().libs / f"{module_name}.ps"
    return candidate if candidate.is_file() else None


# ── instalação: lib Python (pip) ──────────────────────────────────────────────

def install_py(name: str) -> str:
    import subprocess

    print(f"Instalando {name}...")
    result = subprocess.run([sys.executable, "-m", "pip", "install", name], capture_output=False)
    if result.returncode != 0:
        raise PkgmgrError(f"pip install {name} falhou (código {result.returncode})")

    data = _load_installed()
    data["py"][name] = {"source": "pypi", "installed_at": _now()}
    _save_installed(data)
    return f"lib Python '{name}' instalada via pip"


def uninstall_py(name: str) -> str:
    import subprocess

    result = subprocess.run([sys.executable, "-m", "pip", "uninstall", "-y", name], capture_output=False)
    if result.returncode != 0:
        raise PkgmgrError(f"pip uninstall {name} falhou (código {result.returncode})")

    data = _load_installed()
    if name in data["py"]:
        del data["py"][name]
        _save_installed(data)
    return f"lib Python '{name}' removida"


# ── uninstall automático (sem flag) ──────────────────────────────────────────
def _where_installed(name: str) -> list[str]:
    """Em quais categorias `name` está instalado: 'command', 'lib' e/ou 'py'.

    Olha o installed.json e, como rede de segurança, os arquivos no disco —
    assim um .ps órfão (registro dessincronizado) ainda é encontrado."""
    data = _load_installed()
    found: list[str] = []
    if name in data.get("commands", {}):
        found.append("command")
    if name in data.get("libs", {}):
        found.append("lib")
    if name in data.get("py", {}):
        found.append("py")

    if not found:
        p = _paths()
        if (p.commands / f"{name}.ps").exists():
            found.append("command")
        if (p.libs / f"{name}.ps").exists():
            found.append("lib")
    return found


def uninstall_auto(name: str) -> str:
    """`psl uninstall <nome>` sem flag: descobre sozinho se `name` é comando,
    lib PoolScript ou lib Python, e remove de onde estiver.

    Se estiver em mais de um lugar, não adivinha — pede pra desambiguar com a
    flag certa, pra nunca remover a coisa errada silenciosamente."""
    found = _where_installed(name)

    if not found:
        raise PkgmgrError(
            f"'{name}' não está instalado (nem como comando, nem lib PoolScript, nem lib Python)"
        )
    if len(found) > 1:
        # ASCII apenas — a mensagem passa por print no console Windows (cp1252),
        # que não codifica setas/símbolos unicode.
        raise PkgmgrError(
            f"'{name}' esta instalado em mais de uma categoria ({', '.join(found)}). "
            f"Especifique com a flag: -asLib (lib pool) ou -py (lib Python). "
            f"O comando de mesmo nome sai sem flag, depois que os outros forem removidos."
        )

    only = found[0]
    if only == "command":
        return uninstall_command(name)
    if only == "lib":
        return uninstall_lib(name)
    return uninstall_py(name)


# ── listagem ───────────────────────────────────────────────────────────────────

def list_installed() -> str:
    data = _load_installed()
    lines = []
    for label, key in (("Comandos", "commands"), ("Libs PoolScript", "libs"), ("Libs Python", "py")):
        entries = data.get(key, {})
        lines.append(f"{label}:")
        if not entries:
            lines.append("  (nenhum)")
        else:
            for name, meta in sorted(entries.items()):
                lines.append(f"  {name}  ({meta.get('source', '?')})")
    return "\n".join(lines)


def _now() -> str:
    from datetime import datetime, timezone
    return datetime.now(timezone.utc).isoformat(timespec="seconds")


# ── PATH (Windows) ────────────────────────────────────────────────────────────

def _ensure_bin_on_path() -> str | None:
    """Garante que ~/.poolscript/bin esteja no PATH do usuário (Windows).
    Melhor esforço — nunca levanta exceção, só devolve uma mensagem informativa.

    Nunca mexe no PATH real do sistema quando POOLSCRIPT_HOME foi sobrescrito
    (usado pelos testes e por instalações relocadas) — um diretório temporário
    não deve ser gravado permanentemente no registro do Windows."""
    bin_dir = str(_paths().bin)

    if os.environ.get("POOLSCRIPT_HOME"):
        return f"adicione {bin_dir} ao seu PATH para usar os comandos instalados globalmente"

    if sys.platform != "win32":
        if bin_dir not in os.environ.get("PATH", ""):
            return f"adicione {bin_dir} ao seu PATH para usar os comandos instalados globalmente"
        return None

    try:
        import winreg
    except ImportError:
        return f"adicione {bin_dir} ao seu PATH para usar os comandos instalados globalmente"

    try:
        with winreg.OpenKey(winreg.HKEY_CURRENT_USER, "Environment", 0, winreg.KEY_READ | winreg.KEY_WRITE) as key:
            try:
                current, _ = winreg.QueryValueEx(key, "Path")
            except FileNotFoundError:
                current = ""

            entries = [e for e in current.split(";") if e]
            if bin_dir in entries:
                return None

            new_value = ";".join(entries + [bin_dir])
            winreg.SetValueEx(key, "Path", 0, winreg.REG_EXPAND_SZ, new_value)

        _broadcast_env_change()
        return f"{bin_dir} foi adicionado ao seu PATH (abra um novo terminal para usar comandos instalados)"
    except OSError:
        return (
            f"não foi possível atualizar o PATH automaticamente — "
            f"adicione manualmente: {bin_dir}"
        )


def _broadcast_env_change() -> None:
    try:
        import ctypes
        HWND_BROADCAST = 0xFFFF
        WM_SETTINGCHANGE = 0x1A
        SMTO_ABORTIFHUNG = 0x0002
        result = ctypes.c_long()
        ctypes.windll.user32.SendMessageTimeoutW(
            HWND_BROADCAST, WM_SETTINGCHANGE, 0, "Environment",
            SMTO_ABORTIFHUNG, 5000, ctypes.byref(result),
        )
    except Exception:
        pass
