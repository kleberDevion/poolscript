"""
setup.py com suporte a compilação nativa via mypyc.

Uso:
    pip install .                 # instalação normal (Python puro)
    pip install . --config-settings compile=true   # com mypyc
    python setup.py build_ext --inplace            # compila no lugar

mypyc compila lexer, parser e interpreter para extensões C,
reduzindo o tempo de execução em 3-5x.
"""
import os
import sys

# Compilar com mypyc apenas se explicitamente solicitado
# ou se POOLSCRIPT_COMPILE=1 estiver definido
COMPILE = (
    os.environ.get("POOLSCRIPT_COMPILE") == "1"
    or "--compile" in sys.argv
)

if COMPILE:
    try:
        from mypyc.build import mypycify
        from setuptools import setup

        # Módulos a compilar — ordenados por dependência
        modules = [
            "poolscript/lexer.py",
            "poolscript/parser.py",
            "poolscript/interpreter.py",
        ]

        # Ajustar paths para src/
        src_modules = [f"src/{m}" for m in modules]

        print("[PoolScript] Compilando com mypyc...")
        setup(
            ext_modules=mypycify(
                src_modules,
                opt_level="2",   # -O2
                debug_level="0",
            )
        )
        print("[PoolScript] Compilação nativa concluída.")
    except ImportError:
        print("[PoolScript] mypyc não encontrado. Instalando Python puro.")
        from setuptools import setup
        setup()
    except Exception as e:
        print(f"[PoolScript] Falha na compilação mypyc: {e}")
        print("[PoolScript] Usando Python puro como fallback.")
        from setuptools import setup
        setup()
else:
    from setuptools import setup
    setup()
