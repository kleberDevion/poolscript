#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Gera a doc de builtins e dos métodos de string, list e dict a partir das specs.

    PYTHONPATH=.:src python3 scripts/gera_doc.py

Materializa docs/<grupo>/<nome>/<nome>.md (pasta por item, o padrão das libs)
para builtins, string, list e dict + o índice de cada grupo.
Os exemplos de cada spec são validados pela suíte (test_docs_exemplos.py):
editar a spec e regenerar é o fluxo — editar o .md na mão se perde no próximo
gerar.
"""
import sys
from pathlib import Path

RAIZ = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(RAIZ / "scripts"))

from scripts.doc_specs_builtins import BUILTINS   # noqa: E402
from scripts.doc_specs_string import STRMET       # noqa: E402
from scripts.doc_specs_list import LISTMET        # noqa: E402
from scripts.doc_specs_dict import DICTMET        # noqa: E402


def bloco_exemplos(exemplos):
    partes = []
    for codigo, saida in exemplos:
        partes.append("```ps\n" + codigo + "\n```\n")
        partes.append("```saida\n" + saida + "\n```\n")
    return "\n".join(partes)


def gera_item(nome, spec, pasta, indice_rel):
    md = [f"# `{spec['sig']}`", "", spec["resumo"], ""]
    if spec["params"]:
        md += ["## Parâmetros", "", "| nome | tipo | default | nota |", "|---|---|---|---|"]
        for p, t, d, nota in spec["params"]:
            md.append(f"| `{p}` | {t} | {d} | {nota} |")
        md.append("")
    md += ["## Retorno", "", spec["ret"], ""]
    if spec["erros"]:
        md += ["## Erros", ""]
        for tipo, quando in spec["erros"]:
            md.append(f"- **{tipo}** — {quando}")
        md.append("")
    if spec["ex"]:
        md += ["## Exemplos", "", bloco_exemplos(spec["ex"])]
    if spec["bordas"]:
        md += ["## Bordas", ""]
        for b in spec["bordas"]:
            md.append(f"- {b}")
        md.append("")
    md.append(f"[← índice]({indice_rel})")
    md.append("")
    destino = pasta / nome
    destino.mkdir(parents=True, exist_ok=True)
    (destino / f"{nome}.md").write_text("\n".join(md), encoding="utf-8")


def gera_indice(titulo, intro, specs, arquivo):
    md = [f"# {titulo}", "", intro, "",
          f"**{len(specs)} no total** — a contagem sai da fonte "
          "(`vm/poolscript_vm.c`), não da memória de ninguém. Cada exemplo "
          "das páginas roda nos DOIS motores pela suíte "
          "(`tests/test_docs_exemplos.py`): doc errada quebra o teste.", "",
          "| nome | assinatura | o que faz |", "|---|---|---|"]
    for nome in sorted(specs):
        s = specs[nome]
        md.append(f"| [`{nome}`]({nome}/{nome}.md) | `{s['sig']}` | {s['resumo']} |")
    md.append("")
    arquivo.write_text("\n".join(md), encoding="utf-8")


def main():
    pb = RAIZ / "docs" / "builtins"
    ps = RAIZ / "docs" / "string"
    pl = RAIZ / "docs" / "list"
    pd = RAIZ / "docs" / "dict"
    for nome, spec in BUILTINS.items():
        gera_item(nome, spec, pb, "../builtins.md")
    for nome, spec in STRMET.items():
        gera_item(nome, spec, ps, "../string.md")
    for nome, spec in LISTMET.items():
        gera_item(nome, spec, pl, "../list.md")
    for nome, spec in DICTMET.items():
        gera_item(nome, spec, pd, "../dict.md")
    gera_indice("Builtins da PoolScript",
                "Funções disponíveis em qualquer `.ps`, sem import.",
                BUILTINS, pb / "builtins.md")
    gera_indice("Métodos de string",
                "Chamados direto no valor: `\"texto\".metodo()`. Número também "
                "recebe método de string por conversão automática "
                "(`(150).isdigit()`), exceto `len`.",
                STRMET, ps / "string.md")
    gera_indice("Métodos de list",
                "Chamados direto no valor: `l.metodo()`. A MAIORIA muta a "
                "lista no lugar e devolve `null` — não encadeia. Para uma "
                "cópia modificada, use os builtins `sorted(l)`/`reversed(l)`.",
                LISTMET, pl / "list.md")
    gera_indice("Métodos de dict",
                "Chamados direto no valor: `d.metodo()`. Lembre: `x in d` "
                "testa a **chave**; para procurar um **valor**, "
                "`x in d.value()`.",
                DICTMET, pd / "dict.md")
    print(f"gerados: {len(BUILTINS)} builtins + {len(STRMET)} string + "
          f"{len(LISTMET)} list + {len(DICTMET)} dict")


if __name__ == "__main__":
    main()
