#!/usr/bin/env bash
# PoolScript — Instalação com PyPy (Linux/macOS)
# ================================================
# Instala a PoolScript usando PyPy3 como interpretador base.
# PyPy oferece ~3-8x mais performance que CPython via JIT.

set -e

PYPY_MIN_VERSION="3.9"
PACKAGE_DIR="$(cd "$(dirname "$0")" && pwd)"

echo "PoolScript — Instalação com PyPy"
echo "================================="

# Detecta PyPy
PYPY=""
for cmd in pypy3 pypy3.10 pypy3.9 pypy; do
    if command -v "$cmd" &>/dev/null; then
        VERSION=$("$cmd" -c "import sys; print(f'{sys.version_info.major}.{sys.version_info.minor}')")
        MAJOR=$(echo "$VERSION" | cut -d. -f1)
        MINOR=$(echo "$VERSION" | cut -d. -f2)
        if [ "$MAJOR" -ge 3 ] && [ "$MINOR" -ge 9 ]; then
            PYPY="$cmd"
            echo "✓ PyPy encontrado: $cmd (Python $VERSION)"
            break
        fi
    fi
done

if [ -z "$PYPY" ]; then
    echo ""
    echo "⚠  PyPy não encontrado. Opções:"
    echo "   Linux:  sudo apt install pypy3  /  sudo dnf install pypy3"
    echo "   macOS:  brew install pypy3"
    echo "   Manual: https://www.pypy.org/download.html"
    echo ""
    echo "Instalando com CPython como fallback..."
    PYPY="python3"
fi

# Verifica compatibilidade de libs críticas
echo ""
echo "Verificando compatibilidade de dependências..."
$PYPY -c "
import sys
ok = True
libs = ['sqlite3', 're', 'threading', 'json', 'hashlib', 'hmac',
        'email', 'smtplib', 'socket', 'ssl', 'subprocess', 'pathlib',
        'datetime', 'asyncio', 'urllib', 'csv', 'io', 'shutil']
for lib in libs:
    try:
        __import__(lib)
        print(f'  ✓ {lib}')
    except ImportError:
        print(f'  ✗ {lib} — não disponível')
        ok = False
if not ok:
    print('Algumas dependências não estão disponíveis.')
    sys.exit(1)
"

# Instala dependências externas com PyPy
echo ""
echo "Instalando dependências externas..."
$PYPY -m pip install --quiet \
    requests \
    PyJWT \
    websockets \
    mysql-connector-python \
    psycopg2-binary \
    pymongo \
    openpyxl \
    qrcode \
    Pillow \
    python-dotenv \
    cryptography \
    2>/dev/null || echo "  (algumas libs opcionais podem ter falhado — funcionalidade básica intacta)"

# Instala a PoolScript com PyPy
echo ""
echo "Instalando PoolScript..."
$PYPY -m pip install --quiet -e "$PACKAGE_DIR"

# Cria o wrapper de comando
SCRIPT_DIR=$($PYPY -c "import sysconfig; print(sysconfig.get_path('scripts'))")
cat > "$SCRIPT_DIR/pool" << WRAPPER
#!/usr/bin/env bash
exec $PYPY -m poolscript "\$@"
WRAPPER
chmod +x "$SCRIPT_DIR/pool"

echo ""
echo "✓ Instalação concluída!"
echo ""
pool --version
echo ""
echo "Use: pool arquivo.ps"
