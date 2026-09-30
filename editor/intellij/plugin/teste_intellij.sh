#!/bin/sh
# Confere o plugin do IntelliJ contra os jars REAIS do IDEA e do LSP4IJ — o
# que os stubs não conferem. Stub que não casa com o binário só quebra em
# runtime, calado; aqui é o runtime: as classes reais, o LSP4J real e o
# `jinga-lsp` instalado respondendo a um `initialize` + `completion`.
#
# POR QUE ISTO EXISTE: o bundle TextMate e o servidor LSP eram cadastrados à
# mão no IDEA, e o cadastro da máquina ainda dizia `*.ps` — o `.pr` abria sem
# realce e sem completion, e nada acusava. Agora o plugin declara os dois, e
# este teste prova que a declaração LIGA de verdade.
#
#     editor/intellij/plugin/teste_intellij.sh
#
# PULA (dizendo o motivo e quantas checagens deixaram de rodar) sem IDEA/JBR,
# sem o LSP4IJ instalado ou sem javac — nunca finge que passou. O CI do macOS
# e o do Windows não têm IDEA: lá sai o PULOU. Como o teste do Neovim, mede o
# `jinga-lsp` INSTALADO (ou o atalho antigo `poolscript-lsp`, se só ele existir
# — a mesma ordem do plugin).
#
# De um sistema pro outro muda ONDE cada coisa mora:
#   Linux    o IDEA em /opt/idea* ou no Toolbox; os plugins em
#            ~/.local/share/JetBrains/<produto><versão>/
#   macOS    o IDEA é um .app e a casa dele é o Contents/ (o java em
#            jbr/Contents/Home/bin); os plugins em ~/Library/Application
#            Support/JetBrains/<produto><versão>/plugins/
#   Windows  (MSYS2) o IDEA em C:\Program Files\JetBrains ou no Toolbox, em
#            %LOCALAPPDATA%; os plugins em
#            %APPDATA%\JetBrains\<produto><versão>\plugins\. O java e o javac
#            de lá são programas NATIVOS: recebem caminho C:\... e o
#            classpath separado por `;`.
set -u

AQUI=$(cd "$(dirname "$0")" && pwd)
JAR="$AQUI/dist/jinga-icons.jar"
# Quantas checagens o teste faz: uma por `confere("` do ConfereLsp4ij.java. É
# o número que o PULOU conta como não rodado.
CHECAGENS=$(grep -c 'confere("' "$AQUI/teste/ConfereLsp4ij.java")
pulou() {
  echo "PULOU o teste do IntelliJ ($CHECAGENS checagens nao rodaram) — $1"
  exit 0
}

# o sistema: no MSYS2 do Windows o `uname -s` responde UCRT64_NT-...,
# MINGW64_NT-... ou MSYS_NT-...
SO=$(uname -s)
case $SO in
  *_NT*) WIN=1 ;;
  *)     WIN=0 ;;
esac
# `nativo`: o caminho como o java o recebe, e SEP, o separador do classpath.
# No Windows é C:\... com `;`, e `posix` faz o contrário: o C:\... de uma
# variável do Windows (ou do IDEA_HOME) vira o /c/... que o shell usa.
if [ "$WIN" = 1 ]; then
  SEP=';'
  nativo() { cygpath -w "$1"; }
  posix() { [ -z "$1" ] || cygpath -u "$1"; }
else
  SEP=':'
  nativo() { printf '%s\n' "$1"; }
fi

# a pasta bin do JBR que vem no IDEA: jbr/bin no Linux e no Windows (lá o
# java é java.exe), jbr/Contents/Home/bin no macOS; falha se não há java
jbr_bin() {
  for b in "$1/jbr/bin" "$1/jbr/Contents/Home/bin"; do
    if [ -x "$b/java" ] || [ -x "$b/java.exe" ]; then
      printf '%s\n' "$b"
      return 0
    fi
  done
  return 1
}

# o IDEA: IDEA_HOME, senão onde cada sistema o instala, senão o Toolbox
IDEA=${IDEA_HOME:-}
if [ -z "$IDEA" ]; then
  case $SO in
    Darwin)
      # o .app em /Applications, em ~/Applications (onde o Toolbox 2 põe) e
      # no Toolbox antigo
      set -- "/Applications/IntelliJ IDEA"*.app/Contents/ \
             "$HOME/Applications/IntelliJ IDEA"*.app/Contents/ \
             "$HOME/Library/Application Support/JetBrains/Toolbox/apps/"*/ch-*/*/*.app/Contents/ ;;
    *_NT*)
      # o instalador põe em C:\Program Files\JetBrains; o Toolbox 2, em
      # %LOCALAPPDATA%\Programs; o antigo, em %LOCALAPPDATA%\JetBrains\Toolbox
      PF=$(posix "${PROGRAMFILES:-${ProgramFiles:-}}")
      LA=$(posix "${LOCALAPPDATA:-}")
      set -- "$PF/JetBrains/IntelliJ IDEA"*/ "$LA/Programs/IntelliJ IDEA"*/ \
             "$LA/JetBrains/Toolbox/apps/"*/ch-*/*/ ;;
    *)
      set -- /opt/idea*/ "$HOME"/.local/share/JetBrains/Toolbox/apps/*/ch-*/*/ ;;
  esac
  for d in "$@"; do
    jbr_bin "$d" >/dev/null && { IDEA=${d%/}; break; }
  done
fi
# IDEA_HOME=C:\... também vale no Windows, e IDEA_HOME=<o .app> no macOS
if [ "$WIN" = 1 ] && [ -n "$IDEA" ]; then IDEA=$(posix "$IDEA"); fi
if [ "$SO" = Darwin ] && [ -d "$IDEA/Contents/lib" ]; then IDEA="$IDEA/Contents"; fi
BIN=$([ -n "$IDEA" ] && jbr_bin "$IDEA")
[ -n "$BIN" ] || pulou "nao achei o IDEA (IDEA_HOME=... pra apontar)"
# o LSP4IJ instalado (plugin do marketplace), na pasta de plugins do sistema
LSP4IJ=${LSP4IJ_LIB:-}
if [ -z "$LSP4IJ" ]; then
  case $SO in
    Darwin) set -- "$HOME/Library/Application Support/JetBrains/"*/plugins/lsp4ij/lib ;;
    *_NT*)  set -- "$(posix "${APPDATA:-}")/JetBrains/"*/plugins/lsp4ij/lib ;;
    *)      set -- "$HOME"/.local/share/JetBrains/*/lsp4ij/lib ;;
  esac
  for d in "$@"; do
    [ -d "$d" ] && { LSP4IJ=$d; break; }
  done
fi
if [ "$WIN" = 1 ] && [ -n "$LSP4IJ" ]; then LSP4IJ=$(posix "$LSP4IJ"); fi
if [ -z "$LSP4IJ" ] || ! ls "$LSP4IJ"/lsp4ij-*.jar >/dev/null 2>&1; then
  pulou "o plugin LSP4IJ nao esta instalado (LSP4IJ_LIB=... pra apontar)"
fi
JAVA="$BIN/java"
JAVAC="$BIN/javac"
if [ -x "$BIN/java.exe" ]; then JAVA="$BIN/java.exe"; JAVAC="$BIN/javac.exe"; fi
command -v "$JAVAC" >/dev/null 2>&1 || JAVAC=javac
command -v "$JAVAC" >/dev/null 2>&1 || pulou "sem javac"
# O /usr/bin/javac do macOS existe mesmo sem JDK: é um atalho pro JDK do
# JAVA_HOME ou pro que o /usr/libexec/java_home acha. Sem nenhum dos dois, a
# falta do JDK saía como "FALHOU compilar", e não como o PULOU que ela é.
if [ "$SO" = Darwin ] && [ "$(command -v "$JAVAC")" = /usr/bin/javac ] &&
   [ -z "${JAVA_HOME:-}" ] && ! /usr/libexec/java_home >/dev/null 2>&1; then
  pulou "sem javac: o /usr/bin/javac do macOS e so um atalho, e nao ha JDK instalado"
fi

[ -f "$JAR" ] || "$AQUI/build.sh" >/dev/null || { echo "  FALHOU o build do plugin"; exit 1; }

TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
# O curinga `/*` fica FORA do `nativo`: o `cygpath -w` troca o `*`, proibido
# em nome de arquivo do Windows, por um caractere da área de uso privado do
# Unicode (U+F02A), e aí o java.exe e o javac.exe não veem curinga nenhum. O
# launcher do java aceita `/` antes do `*` também no Windows.
CP="$(nativo "$JAR")$SEP$(nativo "$LSP4IJ")/*$SEP$(nativo "$IDEA/lib")/*"
if ! "$JAVAC" -nowarn -cp "$CP" -d "$(nativo "$TMP")" "$(nativo "$AQUI/teste/ConfereLsp4ij.java")" 2>"$TMP/javac.txt"; then
  echo "  FALHOU compilar o teste contra os jars reais:"
  head -20 "$TMP/javac.txt"
  exit 1
fi
# o `jinga-lsp` e o `jinga` que o plugin tem que achar. No Windows o
# instalar.ps1 põe `jinga-lsp.cmd` e `jinga.exe`, e o `command -v` do MSYS2
# só completa o `.exe`. Lá o oráculo é a busca do PRÓPRIO Windows, e não a do
# plugin (JingaLspFactory.acha): com a regra do plugin copiada aqui, o teste
# passava com ela errada. Em cada pasta, `nome` + cada extensão da PATHEXT, na
# ordem dela (a padrão põe .EXE antes de .CMD); arquivo sem extensão o Windows
# não executa, e a pasta corrente fica de fora, como no IDEA. Onde a regra do
# plugin difere dessa (o .cmd antes do .exe, o nome sem extensão), o teste
# acusa. As pastas: as do PATH e, por último, a bin do instalar.ps1, que ele
# grava no PATH do usuário — o IDEA a herda, e o shell do MSYS2 não (o
# MSYS2_PATH_TYPE padrão, `minimal`, não herda o PATH do Windows). O nome novo
# em qualquer pasta vem antes do antigo em qualquer pasta, como no `command -v`
# abaixo. O caminho sai em C:\... e com a extensão em minúsculas, como o plugin
# o devolve (o disco do Windows não diferencia maiúscula). Sai por printf: o
# `echo` de um sh XSI (o dash é um) troca o `\b` de `...\Jinga\bin\...` por um
# backspace.
if [ "$WIN" = 1 ]; then
  EXTS=$(printf '%s\n' "${PATHEXT:-.COM;.EXE;.BAT;.CMD;.VBS;.VBE;.JS;.JSE;.WSF;.WSH;.MSC}" |
         tr ';' '\n' | tr '[:upper:]' '[:lower:]')
  INSTALADO=
  [ -z "${LOCALAPPDATA:-}" ] || INSTALADO="$(posix "$LOCALAPPDATA")/Programs/Jinga/bin"
  acha_win() {
    for nome in "$@"; do
      achado=$(printf '%s:%s\n' "$PATH" "$INSTALADO" | tr ':' '\n' | while IFS= read -r pasta; do
        [ -n "$pasta" ] || continue
        pasta=${pasta%/}
        for ext in $EXTS; do
          if [ -f "$pasta/$nome$ext" ]; then cygpath -w "$pasta/$nome$ext"; exit 0; fi
        done
      done)
      if [ -n "$achado" ]; then printf '%s\n' "$achado"; return 0; fi
    done
  }
  LSP=$(acha_win jinga-lsp poolscript-lsp)
  MOTOR=$(acha_win jinga pool)
else
  LSP=$(command -v jinga-lsp 2>/dev/null || command -v poolscript-lsp 2>/dev/null || true)
  MOTOR=$(command -v jinga 2>/dev/null || command -v pool 2>/dev/null || true)
fi
"$JAVA" -cp "$(nativo "$TMP")$SEP$CP" ConfereLsp4ij "$(nativo "$JAR")" "$LSP" "$MOTOR"
