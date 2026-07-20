# -*- mode: python ; coding: utf-8 -*-
from PyInstaller.utils.hooks import collect_submodules

# hiddenimports OBRIGATÓRIOS — sem eles o exe quebra em runtime:
# 1. poolscript.stdlib.* é carregado por importlib.import_module (lazy, ver
#    stdlib/__init__.py) — invisível pra análise estática do PyInstaller.
# 2. '9f55d279af223608ae4a__mypyc' é a lib compartilhada dos módulos
#    compilados com mypyc (.pyd) — importada em nível C pelos shims,
#    também invisível pra análise. Só existe se build_native.py rodou;
#    o try abaixo mantém o build funcionando no modo interpretado também.
_hidden = collect_submodules('poolscript')
try:
    import importlib, sys as _sys
    _sys.path.insert(0, 'src')
    importlib.import_module('9f55d279af223608ae4a__mypyc')
    _hidden.append('9f55d279af223608ae4a__mypyc')
except ImportError:
    pass


a = Analysis(
    ['pool_entrypoint.py'],
    pathex=['src'],
    binaries=[],
    datas=[],
    hiddenimports=_hidden,
    hookspath=[],
    hooksconfig={},
    runtime_hooks=[],
    excludes=[],
    noarchive=False,
    optimize=0,
)
pyz = PYZ(a.pure)

exe = EXE(
    pyz,
    a.scripts,
    a.binaries,
    a.datas,
    [],
    name='pool',
    debug=False,
    bootloader_ignore_signals=False,
    strip=False,
    upx=True,
    upx_exclude=[],
    runtime_tmpdir=None,
    console=True,
    disable_windowed_traceback=False,
    argv_emulation=False,
    target_arch=None,
    codesign_identity=None,
    entitlements_file=None,
)
