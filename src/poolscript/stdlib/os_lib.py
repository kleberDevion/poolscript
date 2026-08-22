"""
Módulo `os` da PoolScript.
Funções: pathFile, pathFolder, getenv, loadFile.
"""
from __future__ import annotations
import os as _os
import shutil as _shutil
from pathlib import Path

TEXT_EXTENSIONS  = {".txt", ".json", ".csv", ".html", ".xml", ".md", ".ps", ".yaml", ".yml", ".toml", ".ini", ".log"}
BINARY_EXTENSIONS = {".pdf", ".docx", ".xlsx", ".png", ".jpg", ".jpeg", ".gif", ".bmp", ".webp", ".zip", ".mp3", ".mp4"}


class PoolFile:
    """Arquivo binário carregado — suporta move, copy, delete."""

    def __init__(self, path: Path):
        self._path = path
        self.name     = path.name
        self.ext      = path.suffix.lower()
        self.size     = path.stat().st_size
        self._bytes   = path.read_bytes()

    # --- operações ---

    def move(self, destino: str) -> "PoolFile":
        """Move o arquivo para destino. Retorna novo PoolFile."""
        dest = Path(destino)
        dest.parent.mkdir(parents=True, exist_ok=True)
        _shutil.move(str(self._path), str(dest))
        self._path = dest
        return PoolFile(dest)

    def copy(self, destino: str) -> "PoolFile":
        """Copia o arquivo para destino. Retorna novo PoolFile."""
        dest = Path(destino)
        dest.parent.mkdir(parents=True, exist_ok=True)
        _shutil.copy2(str(self._path), str(dest))
        return PoolFile(dest)

    def delete(self) -> bool:
        """Deleta o arquivo do disco."""
        self._path.unlink(missing_ok=True)
        return True

    def bytes(self) -> bytes:
        """Retorna os bytes brutos do arquivo."""
        return self._bytes

    def save(self, path: str = None) -> "PoolFile":
        """Grava o conteúdo em disco. Com `path`, salva lá; sem `path`, salva na
        pasta do script em execução com o nome do próprio arquivo. Retorna o
        PoolFile do destino."""
        if path:
            dest = Path(path)
        else:
            base = _SCRIPT_DIR if _SCRIPT_DIR is not None else Path.cwd()
            dest = base / self.name
        dest.parent.mkdir(parents=True, exist_ok=True)
        dest.write_bytes(self._bytes)
        self._path = dest
        return PoolFile(dest)

    def path(self) -> str:
        """Retorna o caminho absoluto atual."""
        return str(self._path.resolve())

    def __repr__(self):
        return f"<PoolFile '{self.name}' ({self.size} bytes)>"


# Diretório e ARQUIVO do script .ps em execução — injetado pelo interpreter
_SCRIPT_DIR: "Path | None" = None
_SCRIPT_FILE: "str | None" = None


def _set_script_dir(path: str):
    """Chamado pelo interpreter ao iniciar — define o diretório base e o
    arquivo de entrada (este último usado pelo auto-reload do jinker)."""
    global _SCRIPT_DIR, _SCRIPT_FILE
    if not path or path.startswith("<"):
        return
    _SCRIPT_DIR = Path(path).resolve().parent
    _SCRIPT_FILE = str(Path(path).resolve())


def _search_roots():
    """Retorna os diretórios raiz para busca: script dir + cwd."""
    roots = []
    if _SCRIPT_DIR is not None:
        roots.append(_SCRIPT_DIR)
    cwd = Path.cwd()
    if cwd not in roots:
        roots.append(cwd)
    return roots


def pathFile(name: str) -> str:
    """Retorna caminho absoluto de um arquivo.
    Busca a partir do diretório do .ps em execução, depois do cwd.
    """
    for root in _search_roots():
        target = root / name
        if target.is_file():
            return str(target.resolve())
        for found in root.rglob(name):
            if found.is_file():
                return str(found.resolve())
    raise FileNotFoundError(f"arquivo não encontrado: {name}")


def pathFolder(name: str) -> str:
    """Retorna caminho absoluto de uma pasta."""
    for root in _search_roots():
        target = root / name
        if target.is_dir():
            return str(target.resolve())
        for found in root.rglob(name):
            if found.is_dir():
                return str(found.resolve())
    raise FileNotFoundError(f"pasta não encontrada: {name}")


def loadFile(name: str, encoding: str = None) -> "PoolFile":
    """Carrega um arquivo.

    Sem mode — automático pelo extension:
        Binário (pdf, jpg, png, docx...) → PoolFile
        Texto (txt, json, csv, html...)  → string/dict

    mode="rb"    → força binário — só aceita extensões binárias
    mode="utf-8" → força texto   — só aceita extensões texto
    """
    import json as _json
    import csv as _csv

    file_path = Path(pathFile(name))
    ext = file_path.suffix.lower()

    # ── Validação de mode manual ──────────────────────────────────────
    mode = encoding
    if mode == "rb":
        if ext not in BINARY_EXTENSIONS:
            raise TypeError(
                f"loadFile: mode='rb' não aceita extensão '{ext}' — "
                f"use para: {', '.join(sorted(BINARY_EXTENSIONS))}"
            )
        return PoolFile(file_path)

    if mode is not None and mode != "rb":
        # qualquer encoding (utf-8, latin-1 etc) → força texto
        if ext in BINARY_EXTENSIONS:
            raise TypeError(
                f"loadFile: mode='{mode}' não aceita extensão '{ext}' — "
                f"arquivos binários devem usar mode='rb'"
            )

    # ── Automático ───────────────────────────────────────────────────
    encoding = mode if (mode and mode != "rb") else "utf-8"

    if ext in BINARY_EXTENSIONS:
        return PoolFile(file_path)

    if ext == ".json":
        with open(file_path, encoding=encoding) as f:
            return _json.load(f)

    if ext == ".csv":
        with open(file_path, encoding=encoding, newline="") as f:
            reader = _csv.DictReader(f)
            return [dict(row) for row in reader]

    with open(file_path, encoding=encoding) as f:
        return f.read()



# ── Cores ANSI ────────────────────────────────────────────────────────────────
_COLORS = {
    "red":     "\033[91m",
    "green":   "\033[92m",
    "yellow":  "\033[93m",
    "blue":    "\033[94m",
    "magenta": "\033[95m",
    "cyan":    "\033[96m",
    "white":   "\033[97m",
    "reset":   "\033[0m",
}


def warn(text: str = "", color: str = "yellow"):
    """Emite mensagem colorida no terminal.
    
    os.warn("Pasta criada", color="blue")
    os.warn(color="red", text="Erro!")
    """
    c = _COLORS.get(color.lower(), _COLORS["yellow"])
    print(f"{c}{text}{_COLORS['reset']}")


def ipmach() -> str:
    """Retorna e exibe o IP da máquina."""
    import socket as _socket
    try:
        s = _socket.socket(_socket.AF_INET, _socket.SOCK_DGRAM)
        s.connect(("8.8.8.8", 80))
        ip = s.getsockname()[0]
        s.close()
    except Exception:
        ip = "127.0.0.1"
    blue = _COLORS["blue"]
    reset = _COLORS["reset"]
    print(f"{blue}[info] IP da sua máquina: {ip}{reset}")
    return ip


def readFile(path: str, encoding: str = "utf-8") -> str:
    """Lê um arquivo de texto e devolve a string (UTF-8 por padrão)."""
    from pathlib import Path as _P
    return _P(path).read_text(encoding=encoding)


def writeFile(path: str, content, encoding: str = "utf-8") -> str:
    """Escreve `content` (str ou bytes) num arquivo, criando a pasta se preciso.
    Devolve o caminho escrito."""
    from pathlib import Path as _P
    p = _P(path)
    if p.parent and str(p.parent) not in ("", "."):
        p.parent.mkdir(parents=True, exist_ok=True)
    if isinstance(content, (bytes, bytearray)):
        p.write_bytes(bytes(content))
    else:
        p.write_text(str(content), encoding=encoding)
    return str(p)


def mkdir(path: str, exist_ok: bool = False):
    """Cria diretório. exist_ok=true não dá erro se já existir."""
    import os as _os
    _os.makedirs(path, exist_ok=exist_ok)


def rmdir(path: str, force: bool = False):
    """Remove diretório. force=true remove mesmo com conteúdo."""
    import shutil as _shutil
    import os as _os
    if force:
        _shutil.rmtree(path)
    else:
        _os.rmdir(path)


def ls(path: str = ".") -> list:
    """Lista arquivos e pastas do diretório com detalhes."""
    import os as _os
    from pathlib import Path as _P
    result = []
    for item in _os.listdir(path):
        full = _P(path) / item
        result.append({
            "name": item,
            "type": "dir" if full.is_dir() else "file",
            "size": full.stat().st_size if full.is_file() else 0,
        })
    return result


def exists(path: str) -> bool:
    """Verifica se arquivo ou pasta existe."""
    from pathlib import Path as _P
    return _P(path).exists()


def isfile(path: str) -> bool:
    """Verifica se é um arquivo."""
    from pathlib import Path as _P
    return _P(path).is_file()


def isdir(path: str) -> bool:
    """Verifica se é uma pasta."""
    from pathlib import Path as _P
    return _P(path).is_dir()


def rename(src: str, dst: str):
    """Renomeia arquivo ou pasta."""
    import os as _os
    _os.rename(src, dst)


def copy(src: str, dst: str):
    """Copia arquivo."""
    import shutil as _sh
    _sh.copy2(src, dst)


def move(src: str, dst: str):
    """Move arquivo ou pasta."""
    import shutil as _sh
    _sh.move(src, dst)


def size(path: str) -> int:
    """Retorna tamanho do arquivo em bytes."""
    from pathlib import Path as _P
    return _P(path).stat().st_size


def cwd() -> str:
    """Retorna diretório atual."""
    import os as _os
    return _os.getcwd()


def chdir(path: str):
    """Muda o diretório atual."""
    import os as _os
    _os.chdir(path)


def environ(key: str = None) -> "str | dict":
    """Retorna variável de ambiente ou todas se key=None."""
    import os as _os
    if key:
        return _os.environ.get(key)
    return dict(_os.environ)


def cmd(command: str, capture: bool = False):
    """Executa comando no terminal COM shell (interpreta ;, |, $, etc).

        os.cmd("mkdir uploads")
        result = os.cmd("python --version", capture=true)

    ⚠️ NÃO passe dados do usuário aqui — `os.cmd(f"mkdir {nome}")` com
    nome = "x; rm -rf ~" executa o rm. Para isso, use `os.run([...])` (sem shell).
    """
    import subprocess as _sub
    if capture:
        result = _sub.run(command, shell=True, capture_output=True, text=True)
        return result.stdout.strip() or result.stderr.strip()
    else:
        _sub.run(command, shell=True)
        return None


def run(args, capture: bool = False):
    """Executa um comando SEM shell — cada argumento é separado, então dados do
    usuário NÃO conseguem injetar (`;`, `|`, `$`, `&` viram texto literal).

        os.run(["mkdir", nome_do_user])          # seguro: nome nunca injeta
        v = os.run(["python", "--version"], capture=true)

    Aceita também string (dividida respeitando aspas, sem interpretar shell),
    mas a forma com lista é a recomendada."""
    import subprocess as _sub
    import shlex as _shlex
    if isinstance(args, str):
        args = _shlex.split(args)
    if capture:
        result = _sub.run(args, capture_output=True, text=True)
        return result.stdout.strip() or result.stderr.strip()
    _sub.run(args)
    return None


def code(path: str = "."):
    """Abre o editor de código do OS no caminho especificado.
    Detecta: VS Code, Cursor, Zed, nano, vim.
    """
    import subprocess as _sub
    import shutil as _shutil

    editors = ["code", "cursor", "zed", "nano", "vim"]
    for editor in editors:
        if _shutil.which(editor):
            _sub.Popen([editor, path])
            return f"Abrindo {editor}..."
    return "Nenhum editor encontrado"

def getenv(key: str, default=None):
    """Lê variável de ambiente."""
    from .dotenv_lib import load
    load()
    return _os.environ.get(key, default)


EXPORTS = {
    "pathFile":   pathFile,
    "pathFolder": pathFolder,
    "loadFile":   loadFile,
    "readFile":   readFile,
    "writeFile":  writeFile,
    "getenv":     getenv,
    "warn":       warn,
    "ipmach":     ipmach,
    "mkdir":      mkdir,
    "rmdir":      rmdir,
    "ls":         ls,
    "cmd":        cmd,
    "run":        run,
    "code":       code,
    "exists":     exists,
    "isfile":     isfile,
    "isdir":      isdir,
    "rename":     rename,
    "copy":       copy,
    "move":       move,
    "size":       size,
    "cwd":        cwd,
    "chdir":      chdir,
    "environ":    environ,
    "PoolFile":   PoolFile,
}
