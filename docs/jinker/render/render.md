# `render(caminho)` / `render(pasta, arquivo)`

Lê um arquivo do disco e devolve um
[`JinkerResponse`](../JinkerResponse/JinkerResponse.md) com o **MIME type
correto detectado pela extensão** (`.html` → `text/html`, `.png` →
`image/png`, `.pdf` → `application/pdf`, etc.). Serve pra entregar páginas
HTML, imagens, PDFs — qualquer arquivo.

```
from jinker import render

render(caminho: str) -> JinkerResponse
render(pasta: str, arquivo: str) -> JinkerResponse
```

---

## As duas formas de passar o caminho (equivalentes)

```
return render("paginas/index.html")     // caminho completo, um argumento
return render("paginas", "index.html")  // pasta + arquivo separados
```

São **exatamente a mesma coisa**: internamente
`render("paginas", "index.html")` só junta os dois com `/` →
`"paginas/index.html"`. Escolha a que ler melhor.

A forma de dois argumentos brilha quando a pasta é fixa e o arquivo é variável:

```
action pagina() {
    nome = request.path_param("nome")
    return render("paginas", f"{nome}.html")   // paginas/<nome>.html
}
```

---

## Onde `render()` procura o arquivo

Um caminho **relativo** é resolvido, nesta ordem, contra:

1. **A pasta do `.ps` em execução** (onde está seu `app.ps`);
2. **O diretório atual** (`cwd`, de onde você rodou `pool`).

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

```
return render("web/index.html")    // text/html
return render("web/logo.png")      // image/png, bytes
return render("docs/manual.pdf")   // application/pdf, bytes
```

---

## Arquivo inexistente → 404 (não estoura)

Se o arquivo não existir, `render()` devolve uma resposta **404** com uma
página de erro em vez de lançar exceção — então é seguro usar direto no
`return`:

```
@app.route("/", methods=cors.options(["GET"]))
action index() {
    return render("web/index.html")   // se não existir, vira 404 automático
}
```

---

## Relacionados

- [`jsonify`](../jsonify/jsonify.md) — resposta JSON (o outro atalho comum)
- [`JinkerResponse`](../JinkerResponse/JinkerResponse.md) — o objeto devolvido
- Arquivos estáticos e `static_folder` — ver a [visão geral do jinker](../jinker.md)
