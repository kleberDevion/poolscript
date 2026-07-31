# -*- mode: python ; coding: utf-8 -*-
from PyInstaller.utils.hooks import collect_submodules

# hiddenimports OBRIGATÓRIOS — sem eles o exe quebra em runtime:
# 1. poolscript.stdlib.* é carregado por importlib.import_module (lazy, ver
#    stdlib/__init__.py) — invisível pra análise estática do PyInstaller.
# 2. '9f55d279af223608ae4a__mypyc' é a lib compartilhada dos módulos
#    compilados com mypyc (.pyd) — importada em nível C pelos shims,
#    também invisível pra análise. Só existe se build_native.py rodou;
#    o try abaixo mantém o build funcionando no modo interpretado também.
#
# 'src' precisa estar no sys.path ANTES do collect_submodules('poolscript') —
# sem isso ele não acha o pacote (mesmo com pathex=['src'] no Analysis abaixo,
# que só vale pro modulegraph, não pro import real que collect_submodules faz)
# e retorna vazio, silenciosamente. Isso só não quebrava quando quem buildava
# tinha `pip install -e .` feito (poolscript já visível no sys.path global).
import sys as _sys
_sys.path.insert(0, 'src')

_hidden = collect_submodules('poolscript')
try:
    import importlib
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
    icon='installer/assets/pool.ico',
)
