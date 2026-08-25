#!/usr/bin/env bash
# Rebuild LIMPO da extensão C.
#
# `setup_vm.py build_ext` sozinho não basta: o setuptools decide recompilar
# comparando timestamps com granularidade de SEGUNDO. Editar um .c e
# recompilar dentro do mesmo segundo deixa o .so velho no lugar — e o teste
# passa a medir código que não existe mais. Aconteceu duas vezes aqui, com
# divergências fantasma que sumiam sozinhas.
set -e
cd "$(dirname "$0")"
rm -rf build/lib.linux-* build/temp.linux-* src/poolscript/vm/*.so
python3 setup_vm.py build_ext --inplace "$@"

# O binário standalone sai do MESMO .c, mas por outro caminho de build. Sem
# refazer os dois, um fica velho e o teste passa medindo código que já mudou —
# a mesma armadilha do .so desatualizado, só que com dois alvos agora.
echo "--- binario standalone ---"
make -s pool && echo "pool: ok"
