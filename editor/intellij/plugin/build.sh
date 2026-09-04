#!/bin/bash
# Compila o plugin do IntelliJ contra STUBS (só as assinaturas usadas) e
# empacota SEM eles — em runtime as classes reais do IDEA assumem. Não precisa
# de Gradle nem do SDK do IntelliJ: só de um javac.
#
# POR QUE ISTO VOLTOU PRO REPOSITÓRIO: o fonte morava em `ideia-icons/` e foi
# apagado junto com centenas de arquivos no commit d91f2e9. O `.jar` continuou
# instalado na máquina e funcionando — então nada acusou. Só que sem o fonte o
# plugin não se reconstrói, não acompanha a gramática e não se instala em outra
# máquina: existia um binário, não uma entrega.
#
#     editor/intellij/plugin/build.sh          # gera dist/poolscript-icons.jar
#     make intellij                            # gera e INSTALA no IDEA
set -e
cd "$(dirname "$0")"

# O javac do PATH serve (o plugin usa só assinaturas, não a API interna). O do
# JBR do IDEA continua valendo por JAVAC=... pra quem quiser casar a versão.
JAVAC=${JAVAC:-javac}
command -v "$JAVAC" >/dev/null 2>&1 || { echo "build.sh: falta o javac (JAVAC=<caminho> pra apontar outro)" >&2; exit 1; }

# `since-build 231` = IDEA 2023.1, que roda em Java 17.
ALVO=${ALVO:-17}

rm -rf out dist && mkdir -p out dist
"$JAVAC" --release "$ALVO" -nowarn -d out $(find stubs src -name '*.java')
rm -rf out/com                      # descarta os stubs do jar
cp -r resources/* out/
(cd out && zip -qr ../dist/poolscript-icons.jar .)
echo "dist/poolscript-icons.jar pronto"
