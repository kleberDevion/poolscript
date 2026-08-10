#!/bin/bash
# Compila o mini-plugin de ícones contra STUBS (só as assinaturas usadas) e
# empacota SEM os stubs — em runtime as classes reais do IDEA assumem.
set -e
cd "$(dirname "$0")"
JAVAC=${JAVAC:-/opt/idea-IU-262.8665.337/jbr/bin/javac}
rm -rf out dist && mkdir -p out dist
"$JAVAC" -d out $(find stubs src -name '*.java')
rm -rf out/com                      # descarta os stubs do jar
cp -r resources/* out/
(cd out && zip -qr ../dist/poolscript-icons.jar .)
echo "dist/poolscript-icons.jar pronto"
