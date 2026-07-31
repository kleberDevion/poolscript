"""
setup_vm.py — Compila a extensão C da VM PoolScript

Uso:
    python setup_vm.py build_ext --inplace

Após a compilação, o arquivo poolscript_vm.so (Linux/Mac) ou
poolscript_vm.pyd (Windows) aparece em src/poolscript/vm/.
"""
from setuptools import setup, Extension
from pathlib import Path

FLAGS = [
    "-O2",          # otimização — switch/case vira tabela de salto
    "-Wall",
    "-Wno-unused-variable",
]

vm_ext = Extension(
    name="poolscript.vm.poolscript_vm",
    sources=[
        "vm/ps_lexer.c",
        "vm/ps_ast.c",
        "vm/ps_parser.c",
        "vm/ps_compiler.c",
        "vm/ps_hash.c",
        "vm/ps_regex.c",
        "vm/ps_mail.c",
        "vm/ps_http.c",
        "vm/ps_qr.c",
        "vm/ps_xlsx.c",
        "vm/ps_db.c",
        "vm/ps_mongo.c",
        "vm/ps_jinker.c",
        "vm/poolscript_vm.c",
    ],
    extra_compile_args=FLAGS + ["-DPS_MODULO_PYTHON", "-I/usr/include/postgresql", "-I/usr/include/mysql", "-I/usr/include/libmongoc-1.0", "-I/usr/include/libbson-1.0"],
    library_dirs=["/usr/lib/postgresql/16/lib"],
    libraries=["sqlite3", "pq", "pgcommon", "pgport", "mysqlclient", "odbc", "ssl", "crypto", "png", "expat", "z", "stdc++", "zstd", "ltdl", "ldap", "lber", "gssapi_krb5", "mongoc-1.0", "bson-1.0", "rt"],
)

# Binding TEMPORÁRIO do lexer em C, só pro teste diferencial da transição:
# o mesmo fonte passa pelos dois lexers e os tokens têm que bater um a um.
# `ps_lexer.c` é C puro (não inclui Python.h) — é ele que vai pro binário
# standalone; `ps_lexer_bind.c` é o descartável.
lexer_ext = Extension(
    name="poolscript.vm.ps_lexer_c",
    sources=[
        "vm/ps_lexer.c",
        "vm/ps_lexer_bind.c",
    ],
    extra_compile_args=FLAGS + ["-DPS_MODULO_PYTHON"],
)

parser_ext = Extension(
    name="poolscript.vm.ps_parser_c",
    sources=[
        "vm/ps_lexer.c",
        "vm/ps_ast.c",
        "vm/ps_parser.c",
        "vm/ps_parser_bind.c",
    ],
    extra_compile_args=FLAGS + ["-DPS_MODULO_PYTHON"],
)

# Compilador AST → bytecode em C. `ps_compiler.c` é C puro (não inclui
# Python.h) — vai pro binário standalone; `ps_compiler_bind.c` é o
# descartável do teste diferencial de bytecode.
compiler_ext = Extension(
    name="poolscript.vm.ps_compiler_c",
    sources=[
        "vm/ps_lexer.c",
        "vm/ps_ast.c",
        "vm/ps_parser.c",
        "vm/ps_compiler.c",
        "vm/ps_compiler_bind.c",
    ],
    extra_compile_args=FLAGS + ["-DPS_MODULO_PYTHON"],
)

setup(
    name="poolscript-vm-c",
    version="1.0.0",
    description="Extensão C da VM PoolScript",
    ext_modules=[vm_ext, lexer_ext, parser_ext, compiler_ext],
    package_dir={"": "src"},
)
