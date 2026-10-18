# `render(folder_or_file, file=Null)`

Lê um arquivo do disco e devolve um
[`JinkerResponse`](../JinkerResponse/JinkerResponse.md) com o **MIME type
correto detectado pela extensão** (`.html` → `text/html`, `.png` →
`image/png`, `.pdf` → `application/pdf`, etc.). Serve pra entregar páginas
HTML, imagens, PDFs — qualquer arquivo.

```ps
from jinker import render
```

```assinatura
render(folder_or_file: str) -> JinkerResponse            # caminho completo
render(folder_or_file: str, file: str) -> JinkerResponse # pasta + arquivo
```

Os exemplos desta página pressupõem:

```ps contexto
from jinker import Jinker, cors, render
Jinker app = Jinker(__name__)
```

---

## As duas formas de passar o caminho (equivalentes)

```ps
funct index() {
    return render("paginas/index.html")     # caminho completo, um argumento
    return render("paginas", "index.html")  # pasta + arquivo separados
}
```

As duas juntam a pasta e o arquivo com `/` (`"paginas/index.html"`). A
diferença é a trava: na forma de dois argumentos o arquivo tem que ficar
**dentro** da pasta. `render("paginas", "../app.pr")` dá 404, e
`render("pub", "../pub_x/segredo.html")` também, mesmo com a pasta irmã
começando com o mesmo nome.

Por isso a forma de dois argumentos é a certa quando a pasta é fixa e o arquivo
vem de fora:

```ps
funct pagina() {
    nome = request.path_param("nome")
    return render("paginas", f"{nome}.html")   # paginas/<nome>.html
}
```

---

## Onde `render()` procura o arquivo

Um caminho **relativo** é resolvido, nesta ordem, contra:

1. **A pasta do `.pr` em execução** (onde está seu `app.pr`);
2. **O diretório atual** (`cwd`, de onde você rodou `jinga`).

O primeiro que contiver o arquivo vence. Um caminho **absoluto**
(`render("C:/site/x.html")`) é usado como está, sem busca.

**Não existe pasta obrigatória.** `render("index.html")`,
`render("web/index.html")` e `render("qualquer/caminho/pag.html")` funcionam
igual — a "pasta" é só o começo do caminho que você escreveu. (Diferente da
regra fixa de `/static/`, que exige a pasta chamar-se `static`.)

---

## Texto vs. binário — automático

`render()` detecta pela extensão e trata o corpo certo:

- **texto** (`.html`, `.css`, `.js`, `.json`, `.txt`, `.svg`…) → lido como UTF-8;
- **binário** (`.png`, `.jpg`, `.pdf`, `.ico`…) → lido como bytes.

Você não precisa fazer nada — só apontar o arquivo.

```ps
funct arquivo() {
    return render("web/index.html")    # text/html
    return render("web/logo.png")      # image/png, bytes
    return render("docs/manual.pdf")   # application/pdf, bytes
}
```

---

## Arquivo inexistente → 404 (não estoura)

Se o arquivo não existir, `render()` devolve uma resposta **404** com uma
página de erro em vez de lançar exceção — então é seguro usar direto no
`return`:

```ps
@app.route("/", methods=cors.options(["GET"]))
funct index() {
    return render("web/index.html")   # se não existir, vira 404 automático
}
```

---

## Relacionados

- [`jsonify`](../jsonify/jsonify.md) — resposta JSON (o outro atalho comum)
- [`JinkerResponse`](../JinkerResponse/JinkerResponse.md) — o objeto devolvido
- Arquivos estáticos e `static_folder` — ver a [visão geral do jinker](../jinker.md)
