"""
Módulo `hash` da PoolScript — hash de senhas.

Uso:
    import hash

    senha_hash = hash.crypt(senha)
    ok = hash.check(senha_hash, senha_digitada)
"""
import hashlib
import hmac
import os
import base64


def crypt(senha: str) -> str:
    """Gera hash seguro da senha com salt aleatório."""
    salt = os.urandom(32)
    key = hashlib.pbkdf2_hmac("sha256", str(senha).encode("utf-8"), salt, 310000)
    return base64.b64encode(salt + key).decode("utf-8")


def check(senha_hash: str, senha_digitada: str) -> bool:
    """Verifica se a senha digitada bate com o hash salvo."""
    try:
        raw = base64.b64decode(senha_hash.encode("utf-8"))
        salt = raw[:32]
        key_salvo = raw[32:]
        key_novo = hashlib.pbkdf2_hmac("sha256", str(senha_digitada).encode("utf-8"), salt, 310000)
        return hmac.compare_digest(key_salvo, key_novo)
    except Exception:
        return False


EXPORTS = {
    "crypt": crypt,
    "check": check,
}
