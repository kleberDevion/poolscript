#!/usr/bin/env sh
# ============================================================================
#  pooler — instalador do PoolScript (Linux / macOS)
#
#  - detecta OS + arquitetura
#  - SEMPRE pergunta qual motor instalar (menu):
#        INTERP  = interpretador em Python (portátil, precisa Python 3.10+)
#        PSVM    = binário em C (rápido, sem Python) — Linux x86-64
#  - antes de instalar, REMOVE qualquer instalação anterior do PATH do usuário
#    (binário ELF pool/psl OU o INTERP via pip) — reinstalação sempre limpa
#  - resolve as libs do PSVM (apt/dnf/pacman); se não puder, usa o BUNDLE
#    portátil (carrega as .so junto) — nunca falha por lib faltando
#
#  Uso:
#     ./installer/pooler.sh                 # menu interativo
#     ./installer/pooler.sh --engine psvm --yes
#     ./installer/pooler.sh --engine interp
#     POOLER_BINDIR=/usr/local/bin ./installer/pooler.sh --system
# ============================================================================
set -eu

# ── localização e flags ─────────────────────────────────────────────────────
REPO="$(cd "$(dirname "$0")/.." 2>/dev/null && pwd || echo /tmp)"
ENGINE=""
ASSUME_YES=0
BINDIR="${POOLER_BINDIR:-$HOME/.local/bin}"

# Onde buscar quando rodar STANDALONE (curl | sh, fora do repositório)
REPO_URL="${POOLER_REPO_URL:-https://github.com/kleberDevion/poolscript-lang}"
REL_URL="$REPO_URL/releases/latest/download"     # assets do último release
# "no repo" = tem o código-fonte ao lado; senão é standalone (baixa tudo)
if [ -f "$REPO/pyproject.toml" ]; then IN_REPO=1; else IN_REPO=0; fi

while [ $# -gt 0 ]; do
  case "$1" in
    --engine) ENGINE="${2:-}"; shift 2 ;;
    --engine=*) ENGINE="${1#*=}"; shift ;;
    --yes|-y) ASSUME_YES=1; shift ;;
    --system) BINDIR="/usr/local/bin"; shift ;;
    -h|--help) grep '^#' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
    *) echo "opção desconhecida: $1"; exit 1 ;;
  esac
done

c_bold=$(printf '\033[1m'); c_dim=$(printf '\033[2m'); c_grn=$(printf '\033[92m')
c_red=$(printf '\033[91m'); c_off=$(printf '\033[0m')
say()  { printf '%s\n' "$*"; }
ok()   { printf '%s✓%s %s\n' "$c_grn" "$c_off" "$*"; }
warn() { printf '%s!%s %s\n' "$c_red" "$c_off" "$*"; }

# ── 1. detecta OS + arquitetura ──────────────────────────────────────────────
OS="$(uname -s)"; ARCH="$(uname -m)"
case "$OS" in
  Linux)  OS_NOME="Linux" ;;
  Darwin) OS_NOME="macOS" ;;
  *)      OS_NOME="$OS" ;;
esac
# PSVM só tem binário pra Linux x86-64 (local em dist/ ou baixado do release)
PSVM_OK=0
[ "$OS" = "Linux" ] && { [ "$ARCH" = "x86_64" ] || [ "$ARCH" = "amd64" ]; } && PSVM_OK=1

say "${c_bold}PoolScript · pooler${c_off}"
say "${c_dim}sistema: $OS_NOME $ARCH${c_off}"

# ── 2. remove instalação anterior (ELF pool/psl OU INTERP via pip) ───────────
limpa_anterior() {
  removeu=0
  # best-effort: nenhuma falha de remoção pode abortar o instalador (set -e)
  set +e
  for d in /usr/local/bin "$HOME/.local/bin" "$BINDIR"; do
    for nome in pool psl poolscript-lsp; do
      alvo="$d/$nome"
      [ -e "$alvo" ] || continue
      # só mexe se for PoolScript de verdade (evita apagar um `pool` alheio)
      if "$alvo" --version 2>/dev/null | grep -qi poolscript \
         || head -c 4 "$alvo" 2>/dev/null | grep -q ELF \
         || grep -qs poolscript "$alvo" 2>/dev/null; then
        if [ -w "$d" ]; then
          rm -f "$alvo" && removeu=1
        elif command -v sudo >/dev/null 2>&1; then
          sudo rm -f "$alvo" && removeu=1 || warn "não removi $alvo (precisa de permissão)"
        else
          warn "não removi $alvo (sem permissão)"
        fi
      fi
    done
  done
  # INTERP via pip (qualquer python do PATH)
  if python3 -m pip show poolscript >/dev/null 2>&1; then
    python3 -m pip uninstall -y poolscript >/dev/null 2>&1
    removeu=1
  fi
  set -e
  [ "$removeu" = 1 ] && ok "instalação anterior removida" || say "${c_dim}(nada instalado antes)${c_off}"
}

# ── 3. menu: SEMPRE pergunta o motor ─────────────────────────────────────────
escolhe_motor() {
  [ -n "$ENGINE" ] && return
  rec="INTERP"; [ "$PSVM_OK" = 1 ] && rec="PSVM"
  say ""
  say "Qual motor instalar? ${c_dim}(recomendado pro seu sistema: $rec)${c_off}"
  say "  ${c_bold}1${c_off}) PSVM   — binário em C, rápido, sem Python $([ "$PSVM_OK" = 1 ] || echo "${c_dim}(indisponível neste OS/arch)${c_off}")"
  say "  ${c_bold}2${c_off}) INTERP — Python 3.10+, portátil"
  if [ "$ASSUME_YES" = 1 ]; then
    ENGINE=$([ "$PSVM_OK" = 1 ] && echo psvm || echo interp); return
  fi
  # lê do terminal mesmo em 'curl | sh' (stdin é o script, não o teclado);
  # sem terminal e sem --yes, assume o recomendado
  printf "escolha [1/2]: "
  if [ -r /dev/tty ]; then read -r resp </dev/tty; else
    resp=""; say "${c_dim}(sem terminal — usando $rec)${c_off}"; fi
  case "$resp" in
    1) ENGINE=psvm ;;
    2) ENGINE=interp ;;
    *) ENGINE=$([ "$PSVM_OK" = 1 ] && echo psvm || echo interp) ;;
  esac
}

# ── 4. libs do PSVM: nativo, senão bundle ────────────────────────────────────
LIBS_APT="libmongoc-1.0-0 libbson-1.0-0 libmongocrypt0 libsnappy1v5 libldap-2.5-0 libsasl2-2 libgssapi-krb5-2 libgnutls30 libzstd1 libltdl7"

instala_deps_psvm() {
  faltando="$("$BINDIR/pool" --version 2>&1 >/dev/null | grep -o 'lib[a-z0-9._-]*' || true)"
  ldd "$BINDIR/pool" 2>/dev/null | grep -q "not found" || { ok "todas as libs presentes"; return 0; }
  warn "faltam libs do sistema — tentando instalar"
  if command -v apt-get >/dev/null 2>&1; then
    sudo apt-get update -qq && sudo apt-get install -y $LIBS_APT && return 0
  elif command -v dnf >/dev/null 2>&1; then
    sudo dnf install -y mongo-c-driver openldap cyrus-sasl krb5-libs gnutls libzstd libtool-ltdl snappy && return 0
  elif command -v pacman >/dev/null 2>&1; then
    sudo pacman -S --noconfirm mongo-c-driver libldap cyrus-sasl krb5 gnutls zstd libtool snappy && return 0
  fi
  return 1
}

# baixa um arquivo (curl ou wget) pra saída dada; 0 = ok
baixa() {
  url="$1"; saida="$2"
  if command -v curl >/dev/null 2>&1; then curl -fsSL "$url" -o "$saida"
  elif command -v wget >/dev/null 2>&1; then wget -qO "$saida" "$url"
  else return 1; fi
}

usa_bundle() {
  warn "não deu pra instalar as libs — usando o BUNDLE portátil (carrega tudo junto)"
  dest="$HOME/.local/share/poolscript"; rm -rf "$dest"; mkdir -p "$dest"
  if [ "$IN_REPO" = 1 ] && { [ -d "$REPO/dist/pool-portable" ] || (cd "$REPO" && make -s bundle >/dev/null 2>&1); }; then
    cp -r "$REPO/dist/pool-portable/." "$dest/"
  else
    tmp="$dest/pool-portable.tar.gz"
    baixa "$REL_URL/pool-portable.tar.gz" "$tmp" || { warn "bundle indisponível (publique um release com pool-portable.tar.gz)"; return 1; }
    tar -C "$dest" -xzf "$tmp" && mv "$dest"/pool-portable/* "$dest"/ 2>/dev/null; rm -f "$tmp"
  fi
  chmod +x "$dest/pool" 2>/dev/null || true
  for nome in pool psl; do ln -sf "$dest/pool" "$BINDIR/$nome"; done
  ok "bundle instalado em $dest (sem depender de lib do sistema)"
}

instala_psvm() {
  [ "$PSVM_OK" = 1 ] || { warn "PSVM não tem binário pra $OS_NOME $ARCH — caindo pro INTERP"; instala_interp; return; }
  mkdir -p "$BINDIR"
  if [ "$IN_REPO" = 1 ] && [ -f "$REPO/dist/pool-linux" ]; then
    install -m755 "$REPO/dist/pool-linux" "$BINDIR/pool"
  else
    say "${c_dim}baixando o binário PSVM do release…${c_off}"
    baixa "$REL_URL/pool-linux" "$BINDIR/pool" || {
      warn "não baixei o PSVM (publique um release com pool-linux) — caindo pro INTERP"; instala_interp; return; }
    chmod 755 "$BINDIR/pool"
  fi
  cp "$BINDIR/pool" "$BINDIR/psl"
  ok "PSVM instalado em $BINDIR (pool, psl)"
  instala_deps_psvm || usa_bundle
}

# ── 5. INTERP via pip (local do repo OU direto do GitHub) ────────────────────
instala_interp() {
  command -v python3 >/dev/null 2>&1 || { warn "Python 3.10+ não encontrado — instale o Python primeiro"; exit 1; }
  if [ "$IN_REPO" = 1 ]; then alvo="$REPO"; else alvo="git+$REPO_URL.git"; fi
  python3 -m pip install --user --upgrade "$alvo" >/dev/null 2>&1 \
    || python3 -m pip install --user --break-system-packages --upgrade "$alvo"
  ok "INTERP instalado via pip (pool, psl, poolscript-lsp)"
}

# ── 6. PATH ──────────────────────────────────────────────────────────────────
checa_path() {
  case ":$PATH:" in
    *":$BINDIR:"*) : ;;
    *) warn "adicione ao PATH:  export PATH=\"$BINDIR:\$PATH\"  (no ~/.bashrc)" ;;
  esac
}

# ── execução ─────────────────────────────────────────────────────────────────
escolhe_motor
say ""; say "${c_bold}instalando: $ENGINE${c_off}"
limpa_anterior
case "$ENGINE" in
  psvm)   instala_psvm ;;
  interp) instala_interp ;;
  *)      warn "motor inválido: $ENGINE (use psvm ou interp)"; exit 1 ;;
esac
checa_path
say ""
if command -v pool >/dev/null 2>&1; then
  ok "pronto — $(pool --version 2>/dev/null || echo 'instalado')"
else
  ok "pronto — reabra o terminal e rode: pool --version"
fi
