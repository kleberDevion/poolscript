"""setup.py — só existe pra compilação nativa com mypyc (ver build_native.py).

A instalação normal (`pip install -e .`) usa o pyproject.toml e não passa por
aqui com extensões: sem POOLSCRIPT_COMPILE=1, ext_modules fica vazio e este
arquivo é um no-op. `build_native.py` seta a env var e roda
`setup.py build_ext --inplace` pra gerar os .pyd/.so ao lado dos .py.
"""
import os
from setuptools import setup

ext_modules = []
if os.environ.get("POOLSCRIPT_COMPILE") == "1":
    from mypyc.build import mypycify
    ext_modules = mypycify(
        [
            "src/poolscript/lexer.py",
            "src/poolscript/parser.py",
            "src/poolscript/interpreter.py",
        ],
        opt_level="2",
    )

setup(ext_modules=ext_modules)
