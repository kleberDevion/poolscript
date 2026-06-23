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
        import mypy
        print(f"✅ mypyc disponível — mypy {mypy.__version__}")
        return True
    except ImportError:
        print("❌ mypyc não encontrado. Instale com: pip install mypy")
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
    print(f"✅ {removed} artefatos removidos")

def compile_native():
    if not check():
        sys.exit(1)
    
    print("\n[PoolScript] Compilando lexer, parser e interpreter com mypyc...")
    print("Isso pode levar 1-3 minutos.\n")
    
    env = os.environ.copy()
    env["POOLSCRIPT_COMPILE"] = "1"
    
    result = subprocess.run(
        [sys.executable, "setup.py", "build_ext", "--inplace"],
        env=env,
        cwd=os.path.dirname(os.path.abspath(__file__))
    )
    
    if result.returncode == 0:
        print("\n[PoolScript] ✅ Compilação concluída!")
        print("A PoolScript agora roda com código nativo C.")
        print("Execute 'pool --version' para confirmar.")
    else:
        print("\n[PoolScript] ❌ Falha na compilação.")
        print("Verifique os requisitos e tente novamente.")
        sys.exit(1)

if __name__ == "__main__":
    if "--check" in sys.argv:
        check()
    elif "--clean" in sys.argv:
        clean()
    else:
        compile_native()
