#!/bin/bash
# A gramática do bundle do IDEA é CÓPIA da do vsix (fonte única lá) — rode
# este script sempre que a gramática mudar.
cd "$(dirname "$0")"
cp ../psl-poolscript-vsix/syntaxes/poolscript.tmLanguage.json PoolScript.tmbundle/Syntaxes/
echo "gramática sincronizada"
