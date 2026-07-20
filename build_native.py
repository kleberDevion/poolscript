#!/usr/bin/env python3
"""
Compilação nativa da PoolScript com mypyc.

Uso:
    python build_native.py           # compila os 3 módulos principais
    python build_native.py --check   # verifica se mypyc está disponível
    python build_native.py --clean   # remove arquivos compilados

Requer:
    pip install mypy   # inclui mypyc
    
    Windows: Visual C++ Build Tools
             https://visualstudio.microsoft.com/visual-cpp-build-tools/
    Linux:   gcc, python3-dev
             sudo apt install build-essential python3-dev
    Mac:     xcode-select --install
"""
import sys
import os
import subprocess
import shutil

MODULES = [
    "src/poolscript/lexer.py",
    "src/poolscript/parser.py",
    "src/poolscript/interpreter.py",
]

def check():
    try:
        from mypyc.build import mypycify
        from mypy.version import __version__ as mypy_version
        print(f"[ok] mypyc disponivel - mypy {mypy_version}")
        return True
    except ImportError as e:
        # Python 3.14 removeu o distutils da stdlib; o mypyc ainda importa
        # `from distutils import ccompiler` — o shim vem do setuptools.
        if "distutils" in str(e):
            print("[erro] mypyc precisa do distutils (removido no Python 3.12+).")
            print("       Instale o setuptools, que fornece o shim: pip install setuptools")
        else:
            print("[erro] mypyc nao encontrado. Instale com: pip install mypy setuptools")
        return False

def clean():
    patterns = ["**/*.so", "**/*.pyd", "build/", "**/*.c"]
    import glob
    removed = 0
    for p in patterns:
        for f in glob.glob(p, recursive=True):
            try:
                if os.path.isdir(f):
                    shutil.rmtree(f)
                else:
                    os.remove(f)
                removed += 1
            except:
                pass
    print(f"[ok] {removed} artefatos removidos")

def compile_native():
    if not check():
        sys.exit(1)
    
    print("\n[PoolScript] Compilando lexer, parser e interpreter com mypyc...")
    print("Isso pode levar 1-3 minutos.\n")
    
    env = os.environ.copy()
    env["POOLSCRIPT_COMPILE"] = "1"

    # vcvarsall.bat (VS 2026 Build Tools) chama 'vswhere.exe' SEM caminho
    # absoluto. Se a pasta do VS Installer não está no PATH, o cmd imprime
    # "'vswhere.exe' não é reconhecido..." no meio da saída — e esse lixo
    # corrompe o parse de variáveis do setuptools, que então falha com
    # "Unable to find a compatible Visual Studio installation." mesmo com
    # o compilador 100% instalado. Garante a pasta no PATH do subprocesso.
    vs_installer = os.path.join(
        os.environ.get("ProgramFiles(x86)", r"C:\Program Files (x86)"),
        "Microsoft Visual Studio", "Installer",
    )
    if os.path.isdir(vs_installer) and vs_installer.lower() not in env.get("PATH", "").lower():
        env["PATH"] = env.get("PATH", "") + os.pathsep + vs_installer

    result = subprocess.run(
        [sys.executable, "setup.py", "build_ext", "--inplace"],
        env=env,
        cwd=os.path.dirname(os.path.abspath(__file__))
    )
    
    if result.returncode == 0:
        print("\n[PoolScript] Compilacao concluida!")
        print("A PoolScript agora roda com codigo nativo C.")
        print("Execute 'pool --version' para confirmar.")
    else:
        print("\n[PoolScript] Falha na compilacao.")
        print("Verifique os requisitos e tente novamente.")
        sys.exit(1)

if __name__ == "__main__":
    if "--check" in sys.argv:
        check()
    elif "--clean" in sys.argv:
        clean()
    else:
        compile_native()
