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


# Só a família HMAC: RS/PS/ES precisam de RSA ou curva elíptica, que esta lib
# não tem. Recusar por nome é melhor que assinar com outro algoritmo.
_ALGS = {"HS256": hashlib.sha256, "HS384": hashlib.sha384, "HS512": hashlib.sha512}


def _b64encode(data: bytes) -> str:
    return base64.urlsafe_b64encode(data).rstrip(b"=").decode("utf-8")


def _b64decode(s: str) -> bytes:
    padding = 4 - len(s) % 4
    if padding != 4:
        s += "=" * padding
    return base64.urlsafe_b64decode(s)


def gen(payload: dict, secret: str, algorithm: str = "HS256") -> str:
    """Gera um token JWT. Só HS256.

    Recusar outro algoritmo não é preciosismo: antes daqui, `algorithm="RS256"`
    era escrito no header e o token era assinado com HS256 mesmo assim. Quem
    verificasse confiando no `alg` tentaria RS256 num token HMAC — é a classe
    de confusão de algoritmo que já rendeu CVE em várias bibliotecas de JWT.
    """
    if algorithm not in _ALGS:
        raise ValueError(
            f"jwt: algoritmo '{algorithm}' não suportado "
            f"(só {', '.join(sorted(_ALGS))})")
    header = {"alg": algorithm, "typ": "JWT"}
    header_b64 = _b64encode(json.dumps(header, separators=(",", ":")).encode())
    payload_b64 = _b64encode(json.dumps(payload, separators=(",", ":")).encode())
    msg = f"{header_b64}.{payload_b64}"
    sig = hmac.new(secret.encode(), msg.encode(), _ALGS[algorithm]).digest()
    return f"{msg}.{_b64encode(sig)}"


def check(token: str, secret: str) -> dict | None:
    """Verifica o token e retorna o payload. Retorna None se inválido ou expirado."""
    try:
        parts = token.split(".")
        if len(parts) != 3:
            return None
        header_b64, payload_b64, sig_b64 = parts
        # O `alg` do header decide o HMAC, mas só a família HS é aceita —
        # confiar no header a ponto de pular a verificação é o buraco clássico
        # de JWT ("alg":"none" e troca de RS por HS).
        alg = json.loads(_b64decode(header_b64)).get("alg")
        if alg not in _ALGS:
            return None
        msg = f"{header_b64}.{payload_b64}"
        expected_sig = hmac.new(secret.encode(), msg.encode(), _ALGS[alg]).digest()
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
