"""
Módulo `jwt` da PoolScript — geração e verificação de tokens JWT.

Uso:
    import jwt

    token = jwt.gen({"user_id": 1}, "minha_chave", algorithm="HS256")
    payload = jwt.check(token, "minha_chave")
"""
import base64
import hashlib
import hmac
import json
import time


def _b64encode(data: bytes) -> str:
    return base64.urlsafe_b64encode(data).rstrip(b"=").decode("utf-8")


def _b64decode(s: str) -> bytes:
    padding = 4 - len(s) % 4
    if padding != 4:
        s += "=" * padding
    return base64.urlsafe_b64decode(s)


def gen(payload: dict, secret: str, algorithm: str = "HS256") -> str:
    """Gera um token JWT."""
    header = {"alg": algorithm, "typ": "JWT"}
    header_b64 = _b64encode(json.dumps(header, separators=(",", ":")).encode())
    payload_b64 = _b64encode(json.dumps(payload, separators=(",", ":")).encode())
    msg = f"{header_b64}.{payload_b64}"
    sig = hmac.new(secret.encode(), msg.encode(), hashlib.sha256).digest()
    return f"{msg}.{_b64encode(sig)}"


def check(token: str, secret: str) -> dict | None:
    """Verifica o token e retorna o payload. Retorna None se inválido ou expirado."""
    try:
        parts = token.split(".")
        if len(parts) != 3:
            return None
        header_b64, payload_b64, sig_b64 = parts
        msg = f"{header_b64}.{payload_b64}"
        expected_sig = hmac.new(secret.encode(), msg.encode(), hashlib.sha256).digest()
        if not hmac.compare_digest(_b64decode(sig_b64), expected_sig):
            return None
        payload = json.loads(_b64decode(payload_b64))
        # Verifica expiração se tiver "exp"
        if "exp" in payload:
            if time.time() > payload["exp"]:
                return None
        return payload
    except Exception:
        return None


EXPORTS = {
    "gen": gen,
    "check": check,
}
