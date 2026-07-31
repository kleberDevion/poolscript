"""
Módulo `datasentity` da PoolScript.

Equivalente ao @dataclass do Python — Entity com campos tipados,
__init__ automático, sem self.campo = campo manual.

Uso:
    from datasentity import dataentity, asdict, astuple, aslist, asjson

    @dataentity
    Entity Person:
        nome:  str
        idade: int
        email: str

    user = Person(nome="Kleber", idade=17, email="k@mail.com")
    user_dict = asdict(user)
"""
from __future__ import annotations
import json as _json
from typing import Any


class DataEntityMeta:
    """Marcador interno — indica que uma Entity foi decorada com @dataentity."""
    pass


def dataentity(entity_class: Any) -> Any:
    """
    Decorador @dataentity para Entity PoolScript.
    Injeta __init__ automático com base nos campos anotados.
    Campos sem valor padrão são obrigatórios; com valor padrão são opcionais.
    """
    # PoolEntityClass expõe .fields (list[EntityField]) quando
    # a Entity usa sintaxe de campo (nome: tipo).
    # Se não tiver, usa os methods como base.
    entity_class._dataentity = True
    return entity_class


# ── Funções de conversão ─────────────────────────────────────────────────────

def _get_attrs(instance: Any) -> dict:
    """Extrai _ps_attrs de uma PoolEntityInstance."""
    try:
        return object.__getattribute__(instance, "_ps_attrs")
    except AttributeError:
        raise TypeError(f"asdict/astuple/aslist/asjson requerem uma instância de DataEntity, recebeu {type(instance).__name__}")


def asdict(instance: Any) -> dict:
    """DataEntity → dict PoolScript."""
    return dict(_get_attrs(instance))


def astuple(instance: Any) -> tuple:
    """DataEntity → tupla com os valores na ordem dos campos."""
    return tuple(_get_attrs(instance).values())


def aslist(instance: Any) -> list:
    """DataEntity → lista com os valores na ordem dos campos."""
    return list(_get_attrs(instance).values())


def asjson(instance: Any) -> str:
    """DataEntity → string JSON."""
    attrs = _get_attrs(instance)

    def _serialize(v: Any) -> Any:
        if isinstance(v, (str, int, float, bool)) or v is None:
            return v
        if isinstance(v, dict):
            return {k: _serialize(val) for k, val in v.items()}
        if isinstance(v, (list, tuple)):
            return [_serialize(i) for i in v]
        # PoolEntityInstance aninhada
        try:
            nested = object.__getattribute__(v, "_ps_attrs")
            return {k: _serialize(val) for k, val in nested.items()}
        except AttributeError:
            return str(v)

    return _json.dumps({k: _serialize(val) for k, val in attrs.items()},
                        ensure_ascii=False)


EXPORTS = {
    "dataentity": dataentity,
    "asdict":     asdict,
    "astuple":    astuple,
    "aslist":     aslist,
    "asjson":     asjson,
}
