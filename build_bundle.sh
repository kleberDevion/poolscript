#!/usr/bin/env bash
# Monta um bundle PORTÁTIL do binário `pool`: o executável + todas as libs .so
# que ele usa numa pasta `lib/`, com um wrapper que carrega as libs de lá.
# Roda em qualquer Linux x86-64 com glibc compatível SEM precisar de apt install
# (mongoc, libpq-auth, gnutls, krb5, ldap... vão todas juntas).
#
#   ./build_bundle.sh            # gera dist/pool-portable/ + dist/pool-portable.tar.gz
#
# NÃO empacota o núcleo do glibc (libc/libm/libpthread/libdl/librt/libresolv) nem
# o loader: esses TÊM que vir do sistema-alvo, senão o getaddrinfo/DNS (NSS)
# quebra e nada resolve host (nem o Neon). glibc é retrocompatível, então um
# binário compilado em glibc mais nova é o único requisito real do alvo.
set -e
cd "$(dirname "$0")"

BIN=./pool
[ -x "$BIN" ] || { echo "compila primeiro: make pool (ou ./rebuild_vm.sh)"; exit 1; }

OUT=dist/pool-portable
rm -rf "$OUT"; mkdir -p "$OUT/lib"

# núcleo do glibc + loader: fica de FORA (vem do alvo)
EXCLUI='libc\.so|libm\.so|libpthread|libdl\.so|librt\.so|libresolv|ld-linux|linux-vdso'

# copia toda .so que o pool usa (ldd já lista a árvore transitiva achatada)
n=0
while read -r origem; do
    [ -f "$origem" ] || continue
    cp -L "$origem" "$OUT/lib/"
    n=$((n+1))
done < <(ldd "$BIN" | awk '/=> \// {print $3}' | grep -vE "$EXCLUI")

# o binário real + wrapper que aponta pro lib/ do próprio bundle
cp "$BIN" "$OUT/pool.bin"
cat > "$OUT/pool" <<'EOF'
#!/bin/sh
# Wrapper: carrega as libs empacotadas ao lado, depois roda o pool real.
DIR="$(dirname "$(readlink -f "$0")")"
exec env LD_LIBRARY_PATH="$DIR/lib:$LD_LIBRARY_PATH" "$DIR/pool.bin" "$@"
EOF
chmod +x "$OUT/pool"

# psl é o mesmo binário, outro nome (gerencia pacotes) — wrapper idêntico
cp "$OUT/pool" "$OUT/psl"

tar -C dist -czf dist/pool-portable.tar.gz pool-portable

echo "bundle: $OUT ($n libs em lib/)"
echo "tarball: dist/pool-portable.tar.gz ($(du -h dist/pool-portable.tar.gz | cut -f1))"
echo "no VPS: tar xzf pool-portable.tar.gz && ./pool-portable/pool arquivo.ps"
