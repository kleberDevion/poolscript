"""
Módulo `date` da PoolScript — funções de data e hora.

Uso em PoolScript:
    import date
    agora = date.datahora()        // "23/04/2026 14:30:55"
    hoje  = date.today()           // "23/04/2026"
    hora  = date.time()            // "14:30:55"

    // ou importação direta:
    from date import time, today, datahora
"""
from __future__ import annotations
from datetime import datetime


def time():
    """Hora atual no formato HH:MM:SS."""
    return datetime.now().strftime("%H:%M:%S")


def today():
    """Data atual no formato DD/MM/YYYY."""
    return datetime.now().strftime("%d/%m/%Y")


def datahora():
    """Data + hora no formato DD/MM/YYYY HH:MM:SS."""
    return datetime.now().strftime("%d/%m/%Y %H:%M:%S")


def now():
    """Alias de datahora() — formato ISO 8601."""
    return datetime.now().isoformat(sep=" ", timespec="seconds")


def timestamp():
    """Unix timestamp em segundos."""
    return int(datetime.now().timestamp())


def hora(hours: int = 0, minutes: int = 0, days: int = 0) -> int:
    """Retorna segundos totais — use para somar com timestamp() no JWT exp.
    Exemplo: jwt.gen(payload, key) com exp = date.timestamp() + date.hora(hours=24)
    """
    return int((hours * 3600) + (minutes * 60) + (days * 86400))


EXPORTS = {
    "time": time,
    "today": today,
    "datahora": datahora,
    "now": now,
    "timestamp": timestamp,
    "hora": hora,
}
