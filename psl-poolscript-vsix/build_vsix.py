#!/usr/bin/env python3
"""Empacota a extensão num .vsix sem depender de node/npm/vsce.

Um .vsix é só um ZIP com três coisas: `extension/` (o conteúdo do pacote),
`extension.vsixmanifest` (metadados que o VS Code lê na instalação) e
`[Content_Types].xml` (tipos MIME por extensão de arquivo). O manifesto
gerado aqui espelha o que o vsce produz — a estrutura foi conferida contra
um .vsix empacotado pelo vsce de verdade.

Uso:
    python build_vsix.py          # gera psl-poolscript-<versao>.vsix
"""
from __future__ import annotations

import json
import sys
import zipfile
from pathlib import Path
from xml.sax.saxutils import escape

HERE = Path(__file__).parent

# Arquivos/pastas que entram no pacote. Espelha o .vscodeignore: nada de
# __pycache__, .pyc, .vsix antigos ou .vscode/.
INCLUDE = [
    "package.json",
    "extension.js",
    "readme.md",
    "LICENSE.txt",
    "bridge",
    "images",
    "language-configuration",
    "snippets",
    "syntaxes",
    "themes",
]
EXCLUDE_DIRS = {"__pycache__", ".vscode"}
EXCLUDE_SUFFIXES = {".pyc", ".vsix"}

CONTENT_TYPES = (
    '<?xml version="1.0" encoding="utf-8"?>\n'
    '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
    '<Default Extension=".json" ContentType="application/json"/>'
    '<Default Extension=".vsixmanifest" ContentType="text/xml"/>'
    '<Default Extension=".py" ContentType="application/octet-stream"/>'
    '<Default Extension=".js" ContentType="application/javascript"/>'
    '<Default Extension=".png" ContentType="image/png"/>'
    '<Default Extension=".txt" ContentType="text/plain"/>'
    '<Default Extension=".md" ContentType="text/markdown"/>'
    "</Types>"
)

MANIFEST = """<?xml version="1.0" encoding="utf-8"?>
	<PackageManifest Version="2.0.0" xmlns="http://schemas.microsoft.com/developer/vsx-schema/2011" xmlns:d="http://schemas.microsoft.com/developer/vsx-schema-design/2011">
		<Metadata>
			<Identity Language="en-US" Id="{name}" Version="{version}" Publisher="{publisher}" />
			<DisplayName>{display_name}</DisplayName>
			<Description xml:space="preserve">{description}</Description>
			<Tags>{tags}</Tags>
			<Categories>{categories}</Categories>
			<GalleryFlags>Public</GalleryFlags>
			<Properties>
				<Property Id="Microsoft.VisualStudio.Code.Engine" Value="{engine}" />
				<Property Id="Microsoft.VisualStudio.Code.ExtensionDependencies" Value="" />
				<Property Id="Microsoft.VisualStudio.Code.ExtensionPack" Value="" />
				<Property Id="Microsoft.VisualStudio.Code.ExtensionKind" Value="workspace" />
				<Property Id="Microsoft.VisualStudio.Code.LocalizedLanguages" Value="" />
				<Property Id="Microsoft.VisualStudio.Services.GitHubFlavoredMarkdown" Value="true" />
				<Property Id="Microsoft.VisualStudio.Services.Content.Pricing" Value="Free"/>
			</Properties>
			<License>extension/LICENSE.txt</License>
			<Icon>extension/images/icon.png</Icon>
		</Metadata>
		<Installation>
			<InstallationTarget Id="Microsoft.VisualStudio.Code"/>
		</Installation>
		<Dependencies/>
		<Assets>
			<Asset Type="Microsoft.VisualStudio.Code.Manifest" Path="extension/package.json" Addressable="true" />
			<Asset Type="Microsoft.VisualStudio.Services.Content.Details" Path="extension/readme.md" Addressable="true" />
<Asset Type="Microsoft.VisualStudio.Services.Content.License" Path="extension/LICENSE.txt" Addressable="true" />
<Asset Type="Microsoft.VisualStudio.Services.Icons.Default" Path="extension/images/icon.png" Addressable="true" />
		</Assets>
	</PackageManifest>"""


def collect() -> list[Path]:
    """Arquivos que entram no pacote, já filtrados pelo .vscodeignore."""
    files: list[Path] = []
    for entry in INCLUDE:
        path = HERE / entry
        if not path.exists():
            raise SystemExit(f"[erro] não encontrado: {path}")
        if path.is_file():
            files.append(path)
            continue
        for child in sorted(path.rglob("*")):
            if not child.is_file():
                continue
            if EXCLUDE_DIRS & set(child.relative_to(HERE).parts):
                continue
            if child.suffix in EXCLUDE_SUFFIXES:
                continue
            files.append(child)
    return files


def build_manifest(pkg: dict) -> str:
    # o vsce sempre acrescenta essas tags derivadas às declaradas no package.json
    tags = list(pkg.get("keywords", []))
    tags += ["theme", "color-theme", "snippet", pkg["displayName"].split()[0]]
    for lang in pkg.get("contributes", {}).get("languages", []):
        for ext in lang.get("extensions", []):
            tags.append("__ext_" + ext.lstrip("."))

    return MANIFEST.format(
        name=escape(pkg["name"]),
        version=escape(pkg["version"]),
        publisher=escape(pkg["publisher"]),
        display_name=escape(pkg["displayName"]),
        description=escape(pkg["description"]),
        tags=escape(",".join(dict.fromkeys(tags))),
        categories=escape(",".join(pkg.get("categories", []))),
        engine=escape(pkg["engines"]["vscode"]),
    )


def main() -> int:
    # metadata dos hovers/completion sai das specs — regenera pra nunca ir
    # stale no pacote (docs_meta.json = builtins+string; stdlib = módulos).
    import subprocess as _sub
    import os as _os
    # gen_stdlib_metadata importa o pacote `poolscript` (do repo, em ../src) —
    # sem isso o gerador falha ("No module named poolscript") e o metadata sai
    # stale no vsix. Aponta o PYTHONPATH pro src do repo.
    _env = dict(_os.environ)
    _src = HERE.parent / "src"
    _env["PYTHONPATH"] = str(_src) + _os.pathsep + _env.get("PYTHONPATH", "")
    for gen in ("gen_docs_meta.py", "gen_stdlib_metadata.py"):
        try:
            _sub.run([sys.executable, str(HERE / "bridge" / gen)], check=True, env=_env)
        except Exception as e:
            print(f"[aviso] falha ao rodar {gen}: {e} — usando o que já existe")

    pkg = json.loads((HERE / "package.json").read_text(encoding="utf-8"))
    out = HERE / f"{pkg['name']}-{pkg['version']}.vsix"

    files = collect()
    with zipfile.ZipFile(out, "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("extension.vsixmanifest", build_manifest(pkg))
        z.writestr("[Content_Types].xml", CONTENT_TYPES)
        for path in files:
            z.write(path, "extension/" + path.relative_to(HERE).as_posix())

    size_kb = out.stat().st_size / 1024
    print(f"gerado: {out.name}  ({len(files)} arquivos, {size_kb:.0f} KB)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
