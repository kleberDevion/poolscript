#!/usr/bin/env bash
# Rebuild LIMPO do binário.
#
# Este script existia porque a VM também era compilada como extensão C do
# Python (`setup_vm.py build_ext`), e havia dois alvos que podiam ficar
# dessincronizados. Não existe mais extensão nem Python: o `pool` é o único
# alvo, e o rebuild é `make`.
#
# Continua sendo LIMPO de propósito: apagar antes evita o caso de editar um
# .c e o make decidir que não precisa recompilar por causa de timestamp com
# granularidade de segundo. Já aconteceu aqui — divergência fantasma que sumia
# sozinha porque o teste media código que não existia mais.
#
#   ./rebuild_vm.sh              # rebuild limpo
#   ./rebuild_vm.sh --check      # rebuild + suíte
#   ./rebuild_vm.sh --install    # rebuild + suíte + instala no sistema (sudo)
set -e
cd "$(dirname "$0")"

make -s limpa
make -s pool
echo "pool: ok  ($(./pool --version))"

for arg in "$@"; do
    case "$arg" in
        --check)
            make -s check
            ;;
        --install)
            make -s check
            sudo make -s install
            ;;
        *)
            echo "argumento desconhecido: $arg (use --check ou --install)" >&2
            exit 1
            ;;
    esac
done
