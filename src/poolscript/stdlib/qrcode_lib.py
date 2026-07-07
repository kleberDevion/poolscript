"""
Módulo `qrcode` da PoolScript.

Requer: pip install qrcode[pil]

Espelha a API da lib `qrcode` do Python — mesmos nomes e parâmetros
sempre que possível, para quem já conhece a lib original.

Uso:
    import qrcode

    # estilo Python — qrcode.make(data) retorna a imagem
    img = qrcode.make("https://meusite.com")
    img.save("qr.png")

    # gera e salva direto, com nome
    qrcode.gen("https://meusite.com", save="qr.png")

    # gera em memória com nome customizado
    file = qrcode.gen("https://meusite.com", name="meu_qrcode.png")
    file.save("pasta/onde_quiser.png")

    # controle total — igual à classe QRCode do Python
    qr = qrcode.QRCode(version=None, error_correction="L", box_size=10, border=4)
    qr.add_data("dados")
    qr.make(fit=True)
    img = qr.make_image(fill_color="black", back_color="white")
    img.save("qr.png")

    # customizado
    qrcode.gen("dados", save="qr.png", size=10, border=2,
               color="black", bg="white", error_correction="H")
"""
from __future__ import annotations
import io


# ── Níveis de correção de erro — mesmos nomes da lib Python ───────────────────
# L = 7%, M = 15%, Q = 25%, H = 30% de tolerância a dano/sujeira
ERROR_CORRECT_L = "L"
ERROR_CORRECT_M = "M"
ERROR_CORRECT_Q = "Q"
ERROR_CORRECT_H = "H"


def _resolve_error_correction(level: str):
    """Converte 'L'/'M'/'Q'/'H' para a constante real da lib qrcode."""
    import qrcode as _qr
    mapping = {
        "L": _qr.constants.ERROR_CORRECT_L,
        "M": _qr.constants.ERROR_CORRECT_M,
        "Q": _qr.constants.ERROR_CORRECT_Q,
        "H": _qr.constants.ERROR_CORRECT_H,
    }
    return mapping.get(str(level).upper(), _qr.constants.ERROR_CORRECT_L)


class QRPoolFile:
    """Arquivo de QR Code em memória — usado quando save= não é fornecido.

    name pode ser customizado via gen(data, name="meu.png").
    """
    def __init__(self, data: bytes, name: str = "qrcode.png"):
        self._data = data
        self.name  = name
        self.ext   = "." + name.rsplit(".", 1)[-1] if "." in name else ".png"
        self.size  = len(data)
        self.content_type = "image/png" if self.ext == ".png" else f"image/{self.ext[1:]}"

    def bytes(self) -> bytes:
        return self._data

    def save(self, path: str) -> "QRPoolFile":
        from pathlib import Path as _P
        target = _P(path)
        # se path for uma pasta, usa o self.name
        if target.is_dir() or path.endswith(("/", "\\")):
            target = target / self.name
        target.write_bytes(self._data)
        return self

    def __repr__(self):
        return f"<QRCode '{self.name}' {self.size} bytes>"


class PoolQRCode:
    """
    Wrapper de qrcode.QRCode — mesma API da lib Python:

        qr = qrcode.QRCode(version=None, error_correction="L", box_size=10, border=4)
        qr.add_data("texto")
        qr.make(fit=True)
        img = qr.make_image(fill_color="black", back_color="white")
    """
    def __init__(self, version=None, error_correction: str = "L",
                 box_size: int = 10, border: int = 4):
        import qrcode as _qr
        self._qr = _qr.QRCode(
            version=version,
            error_correction=_resolve_error_correction(error_correction),
            box_size=box_size,
            border=border,
        )

    def add_data(self, data) -> None:
        import json as _json
        if isinstance(data, (dict, list)):
            data = _json.dumps(data, ensure_ascii=False)
        self._qr.add_data(str(data))

    def make(self, fit: bool = True) -> None:
        self._qr.make(fit=fit)

    def make_image(self, fill_color: str = "black", back_color: str = "white",
                    name: str = "qrcode.png") -> "QRImage":
        img = self._qr.make_image(fill_color=fill_color, back_color=back_color)
        return QRImage(img, name=name)

    def clear(self) -> None:
        self._qr.clear()

    def __repr__(self):
        return "<PoolQRCode>"


class QRImage:
    """Wrapper da imagem PIL retornada por make_image — estilo Python."""
    def __init__(self, pil_img, name: str = "qrcode.png"):
        self._img = pil_img
        self.name = name

    def save(self, path: str) -> "QRImage":
        """Salva a imagem. Se path for pasta, usa self.name."""
        from pathlib import Path as _P
        target = _P(path)
        if target.is_dir() or path.endswith(("/", "\\")):
            target = target / self.name
        self._img.save(str(target))
        return self

    def resize(self, width: int, height: int) -> "QRImage":
        """Redimensiona a imagem (equivalente ao qr32 do gen())."""
        from PIL import Image as _Image
        self._img = self._img.resize((int(width), int(height)), _Image.NEAREST)
        return self

    def to_file(self) -> "QRPoolFile":
        """Converte para QRPoolFile (bytes em memória)."""
        buf = io.BytesIO()
        self._img.save(buf, format="PNG")
        buf.seek(0)
        return QRPoolFile(buf.read(), name=self.name)

    def __repr__(self):
        return f"<QRImage '{self.name}'>"


def make(data, error_correction: str = "L", box_size: int = 10,
         border: int = 4, fill_color: str = "black", back_color: str = "white",
         name: str = "qrcode.png") -> "QRImage":
    """
    Atalho estilo Python: `qrcode.make(data)`.
    Retorna QRImage — chame .save(path) para salvar.

    Equivalente ao `qrcode.make("texto")` da lib Python original.
    """
    try:
        qr = PoolQRCode(error_correction=error_correction, box_size=box_size, border=border)
        qr.add_data(data)
        qr.make(fit=True)
        return qr.make_image(fill_color=fill_color, back_color=back_color, name=name)
    except ImportError:
        raise RuntimeError("qrcode não instalado. Execute: pip install qrcode[pil]")


def gen(data, save: str = None, size: int = 10,
        border: int = 4, color: str = "black", bg: str = "white",
        name: str = None, error_correction: str = "L",
        qr32: tuple = None):
    """
    Gera um QR Code — função de alto nível da PoolScript.

    Parâmetros:
        data             — conteúdo do QR (str, dict ou list — dict/list vira JSON)
        save             — caminho pra salvar (.png, .jpg). Se for pasta, usa `name`.
        size             — tamanho das caixinhas (box_size), default 10
        border           — borda branca ao redor, default 4
        color            — cor do QR, default "black"
        bg               — cor do fundo, default "white"
        name             — nome do arquivo quando retornado em memória
                           (default "qrcode.png"). Também usado se `save`
                           apontar para uma pasta.
        error_correction — "L" (7%) "M" (15%) "Q" (25%) "H" (30%), default "L"
        qr32             — (largura, altura) — redimensiona a imagem final

    Retorna:
        QRPoolFile — sempre. Tem .save(path), .bytes(), .name, .size, .ext
        Se `save=` foi fornecido, o arquivo já foi salvo nesse caminho
        E o QRPoolFile retornado aponta para ele.
    """
    try:
        file_name = name or "qrcode.png"

        qr = PoolQRCode(error_correction=error_correction, box_size=size, border=border)
        qr.add_data(data)
        qr.make(fit=True)
        img = qr.make_image(fill_color=color, back_color=bg, name=file_name)

        if qr32 is not None:
            w, h = int(qr32[0]), int(qr32[1])
            img.resize(w, h)

        if save:
            img.save(save)
            from pathlib import Path as _P
            from .os_lib import PoolFile
            saved_path = _P(save)
            if saved_path.is_dir() or save.endswith(("/", "\\")):
                saved_path = saved_path / file_name
            return PoolFile(saved_path)

        return img.to_file()

    except ImportError:
        raise RuntimeError(
            "qrcode não instalado. Execute: pip install qrcode[pil]"
        )
    except Exception as e:
        raise RuntimeError(f"Erro ao gerar QR Code: {e}")


EXPORTS = {
    "gen":  gen,
    "make": make,           # estilo Python — qrcode.make(data)
    "QRCode": PoolQRCode,    # estilo Python — qrcode.QRCode(...)
    "ERROR_CORRECT_L": ERROR_CORRECT_L,
    "ERROR_CORRECT_M": ERROR_CORRECT_M,
    "ERROR_CORRECT_Q": ERROR_CORRECT_Q,
    "ERROR_CORRECT_H": ERROR_CORRECT_H,
}
