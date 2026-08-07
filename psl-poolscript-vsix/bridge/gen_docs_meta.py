#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Gera docs_meta.json — dados ricos p/ os hovers da extensão (estilo Pylance).

Fonte: as MESMAS specs que geram a doc (scripts/doc_specs_*.py). Assim o card
do editor e a doc do site nunca divergem. Rode quando editar as specs:

    python3 psl-poolscript-vsix/bridge/gen_docs_meta.py
"""
import json
import sys
from pathlib import Path

RAIZ = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(RAIZ / "scripts"))

from doc_specs_builtins import BUILTINS   # noqa: E402
from doc_specs_string import STRMET       # noqa: E402
from doc_specs_keywords import KEYWORDS_DOC, TYPES_DOC  # noqa: E402


def conv(spec):
    return {
        "sig": spec["sig"],
        "resumo": spec["resumo"],
        "params": [{"nome": p, "tipo": t, "default": d, "nota": nota}
                   for (p, t, d, nota) in spec["params"]],
        "ret": spec["ret"],
        "erros": [{"tipo": t, "quando": q} for (t, q) in spec["erros"]],
        "ex": [{"codigo": c, "saida": s} for (c, s) in spec["ex"]],
        "bordas": list(spec["bordas"]),
    }


def main():
    out = {
        "builtins": {nome: conv(s) for nome, s in BUILTINS.items()},
        "string":   {nome: conv(s) for nome, s in STRMET.items()},
        "keywords": {nome: conv(s) for nome, s in KEYWORDS_DOC.items()},
        "types":    {nome: conv(s) for nome, s in TYPES_DOC.items()},
    }
    destino = Path(__file__).resolve().parent / "docs_meta.json"
    destino.write_text(json.dumps(out, ensure_ascii=False, indent=1), encoding="utf-8")
    print(f"docs_meta.json: {len(out['builtins'])} builtins + {len(out['string'])} métodos de string "
          f"+ {len(out['keywords'])} keywords + {len(out['types'])} tipos")


if __name__ == "__main__":
    main()
