"""
Módulo `dotenv` da PoolScript.
Carrega arquivo .env subindo nos diretórios pai (igual python-dotenv).
"""
from __future__ import annotations
import os
from pathlib import Path


def _find_dotenv(start: Path) -> Path | None:
    """Sobe nos diretórios pai procurando um .env."""
    current = start.resolve()
    while True:
        candidate = current / ".env"
        if candidate.is_file():
            return candidate
        parent = current.parent
        if parent == current:   # chegou na raiz sem achar
            return None
        current = parent


def load(path: str | None = None) -> dict:
    """Carrega .env (KEY=VALUE por linha) para os.environ.
    
    Se `path` não for informado, sobe nos diretórios a partir do cwd
    até encontrar um .env — igual ao comportamento do python-dotenv.
    """
    if path:
        env_path = Path(path)
    else:
        env_path = _find_dotenv(Path.cwd())

    loaded = {}
    if env_path is None or not env_path.is_file():
        return loaded

    for raw in env_path.read_text(encoding="utf-8").splitlines():
        line = raw.strip()
        if not line or line.startswith("#") or "=" not in line:
            continue
        key, _, value = line.partition("=")
        key = key.strip()
        value = value.strip().strip('"').strip("'")
        os.environ.setdefault(key, value)
        loaded[key] = value

    return loaded


EXPORTS = {
    "load": load,
}
