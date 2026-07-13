# sys — Sistema

```
import sys
```

---

## sys.argv

Argumentos passados na linha de comando, depois do nome do arquivo:

```
// $ pool app.ps criar joao
comando = sys.argv[0]   // "criar"
nome    = sys.argv[1]   // "joao"
len(sys.argv)            // quantidade de args
```

Fora do intervalo devolve `Null`, não erro.

---

## sys.exit(code=0)

Encerra o programa imediatamente com o código de saída informado.

```
if (not autenticado) {
    post("acesso negado")
    sys.exit(1)
}
```

---

## sys.platform()

Retorna o sistema operacional atual: `"windows"`, `"linux"` ou `"darwin"`
(macOS).

```
if (sys.platform() == "windows") {
    post("rodando no Windows")
}
```

---

## sys.stdout / sys.stderr

Saída padrão e de erro, com flush automático.

```
sys.stdout.write("sem quebra de linha")
sys.stdout.write("com quebra", end=true)
sys.stdout.writeln("sempre quebra linha")   // atalho de write(..., end=true)

sys.stderr.writeln("mensagem de erro")
```

---

## sys.RelativePath(name)

Busca recursivamente, a partir do diretório de trabalho atual, um arquivo
ou pasta com esse nome. Retorna o caminho absoluto se achar, `Null` se não:

```
caminho = sys.RelativePath("config.json")
if (caminho) {
    post("achou em: " {caminho})
} else {
    post("config.json não encontrado")
}

pasta_assets = sys.RelativePath("assets")
```
