"""
Registry de bibliotecas padrão da PoolScript.

Cada lib expõe um dict EXPORTS com {nome_função: callable}.
O interpretador consulta REGISTRY para resolver `import os`, `from os import getenv`,
`PUSH os GET getenv`, etc.

Libs são carregadas sob demanda (lazy) — só quando o script importar.
"""
from __future__ import annotations


def _stub(lib_name: str):
    def _factory(name: str):
        def _missing(*args, **kwargs):
            raise NotImplementedError(
                f"`{lib_name}.{name}` ainda não está implementado nesta versão da PoolScript. "
                f"Libs disponíveis: os, json, dotenv, request, mail, date, db, hash, jwt, jinker."
            )
        return _missing
    return _factory


STUB_LIBS = {
    "sqlite": ["connect", "execute", "fetchall", "fetchone", "close"],
    "smtplib": ["SMTP", "SMTP_SSL", "send"],
    "mimetext": ["MIMEText"],
    "multipart": ["MIMEMultipart"],
    "flask": ["Flask", "route", "run"],
}


def _build_stub_exports(lib_name: str) -> dict:
    factory = _stub(lib_name)
    return {fn: factory(fn) for fn in STUB_LIBS[lib_name]}


# Mapa de módulo → função que carrega e retorna EXPORTS
_LAZY_LOADERS: dict[str, str] = {
    "os":      "os_lib",
    "json":    "json_lib",
    "JSON":    "json_lib",
    "dotenv":  "dotenv_lib",
    "mail":    "mail_lib",
    "date":    "date_lib",
    "jinker":  "jinker_lib",
    "psodbc":  "psodbc_lib",
    "db":      "psodbc_lib",
    "hash":    "hash_lib",
    "jwt":     "jwt_lib",
    "request": "request_lib",
    "requests":"request_lib",
    "manpu":   "manpu_lib",
    "mp":      "manpu_lib",
    "regex":   "regex_lib",
    "sqlite3": "sqlite3_lib",
    "qrcode":  "qrcode_lib",
    "qr":      "qrcode_lib",
    "sys":          "sys_lib",
    "datasentity":  "datasentity_lib",
    "dataentity":   "datasentity_lib",
}

# Cache — evita recarregar o mesmo módulo duas vezes
_CACHE: dict[str, dict] = {}


def _load(lib_name: str) -> dict:
    if lib_name in _CACHE:
        return _CACHE[lib_name]
    module_name = _LAZY_LOADERS[lib_name]
    import importlib
    mod = importlib.import_module(f".{module_name}", package=__name__)
    exports = mod.EXPORTS
    # request dentro de rotas jinker usa o proxy da jinker
    # mas ws_connect vem do request_lib real
    if lib_name in ("request", "requests"):
        jinker = importlib.import_module(".jinker_lib", package=__name__)
        request_real = importlib.import_module(".request_lib", package=__name__)
        proxy = jinker.request

        exports = {
            "get_json": proxy.get_json,
            "get": request_real.EXPORTS["get"],
            "post": request_real.EXPORTS["post"],
            "put": request_real.EXPORTS["put"],
            "patch": request_real.EXPORTS["patch"],
            "delete": request_real.EXPORTS["delete"],
            "text": proxy.text,
            "ws_connect": request_real.EXPORTS["ws_connect"],
            "__proxy__": proxy,
        }
    _CACHE[lib_name] = exports
    return exports


def resolve_module(path_parts: list[str]) -> dict | None:
    if not path_parts:
        return None
    head = path_parts[0]
    if head in _LAZY_LOADERS:
        return _load(head)
    if head in STUB_LIBS:
        return _build_stub_exports(head)
    return None


def resolve_name(path_parts: list[str], name: str):
    module = resolve_module(path_parts)
    if module is None:
        return None
    return module.get(name)


def list_libs() -> list[str]:
    return sorted(list(_LAZY_LOADERS.keys()) + list(STUB_LIBS.keys()))
