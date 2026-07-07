"""
setup_vm.py — Compila a extensão C da VM PoolScript

Uso:
    python setup_vm.py build_ext --inplace

Após a compilação, o arquivo poolscript_vm.so (Linux/Mac) ou
poolscript_vm.pyd (Windows) aparece em src/poolscript/vm/.
"""
from setuptools import setup, Extension
from pathlib import Path

vm_ext = Extension(
    name="poolscript.vm.poolscript_vm",
    sources=["src/poolscript/vm/poolscript_vm.c"],
    extra_compile_args=[
        "-O2",          # otimização — switch/case vira tabela de salto
        "-Wall",
        "-Wno-unused-variable",
    ],
)

setup(
    name="poolscript-vm-c",
    version="1.0.0",
    description="Extensão C da VM PoolScript",
    ext_modules=[vm_ext],
    package_dir={"": "src"},
)
