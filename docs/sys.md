# sys — Sistema

```
import sys
```

---

## sys.argv

`argv[0]` é o **nome do script**; os seus argumentos vêm a partir de `argv[1]`
— a convenção de C, Python e JS:

```
# $ pool app.ps criar joao
script  = sys.argv[0]   # "app.ps"
comando = sys.argv[1]   # "criar"
nome    = sys.argv[2]   # "joao"
len(sys.argv)            # 3  (script + 2 argumentos)
```

Fora do intervalo **levanta** `IndexError: list index out of range` — guarde
com `len(sys.argv)` antes de indexar, contando o script: um programa que precisa
de um argumento exige `len(sys.argv) >= 2`. Ver [`sys/argv`](sys/argv/argv.md).

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
sys.stdout.writeln("sempre quebra linha")   # atalho de write(..., end=true)

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
