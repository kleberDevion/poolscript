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
# PULA (dizendo o motivo) sem IDEA/JBR ou sem o LSP4IJ instalado — nunca finge
# que passou. Como o teste do Neovim, mede o `jinga-lsp` INSTALADO (ou o
# atalho antigo `poolscript-lsp`, se só ele existir — a mesma ordem do plugin).
set -u

AQUI=$(cd "$(dirname "$0")" && pwd)
JAR="$AQUI/dist/jinga-icons.jar"

# o IDEA: IDEA_HOME, senão /opt/idea*, senão o Toolbox
IDEA=${IDEA_HOME:-}
if [ -z "$IDEA" ]; then
  for d in /opt/idea*/ "$HOME"/.local/share/JetBrains/Toolbox/apps/*/ch-*/*/ ; do
    [ -x "$d/jbr/bin/java" ] && { IDEA=${d%/}; break; }
  done
fi
if [ -z "$IDEA" ] || [ ! -x "$IDEA/jbr/bin/java" ]; then
  echo "PULOU o teste do IntelliJ — nao achei o IDEA (IDEA_HOME=... pra apontar)"
  exit 0
fi
# o LSP4IJ instalado (plugin do marketplace)
LSP4IJ=${LSP4IJ_LIB:-}
if [ -z "$LSP4IJ" ]; then
  for d in "$HOME"/.local/share/JetBrains/*/lsp4ij/lib; do
    [ -d "$d" ] && { LSP4IJ=$d; break; }
  done
fi
if [ -z "$LSP4IJ" ] || ! ls "$LSP4IJ"/lsp4ij-*.jar >/dev/null 2>&1; then
  echo "PULOU o teste do IntelliJ — o plugin LSP4IJ nao esta instalado (LSP4IJ_LIB=... pra apontar)"
  exit 0
fi
JAVA="$IDEA/jbr/bin/java"
JAVAC="$IDEA/jbr/bin/javac"
command -v "$JAVAC" >/dev/null 2>&1 || JAVAC=javac
command -v "$JAVAC" >/dev/null 2>&1 || { echo "PULOU o teste do IntelliJ — sem javac"; exit 0; }

[ -f "$JAR" ] || "$AQUI/build.sh" >/dev/null || { echo "  FALHOU o build do plugin"; exit 1; }

TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
CP="$JAR:$LSP4IJ/*:$IDEA/lib/*"
if ! "$JAVAC" -nowarn -cp "$CP" -d "$TMP" "$AQUI/teste/ConfereLsp4ij.java" 2>"$TMP/javac.txt"; then
  echo "  FALHOU compilar o teste contra os jars reais:"
  head -20 "$TMP/javac.txt"
  exit 1
fi
LSP=$(command -v jinga-lsp 2>/dev/null || command -v poolscript-lsp 2>/dev/null || true)
MOTOR=$(command -v jinga 2>/dev/null || command -v pool 2>/dev/null || true)
"$JAVA" -cp "$TMP:$CP" ConfereLsp4ij "$JAR" "$LSP" "$MOTOR"
