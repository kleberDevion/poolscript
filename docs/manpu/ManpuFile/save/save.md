# `ManpuFile.save()`

Salva as mudanças do arquivo aberto no disco.

```
arq.save() -> ManpuResult
```

---

## Normalmente você NÃO precisa chamar

Dentro de um `using`, o `.save()` roda **automaticamente** ao sair do bloco.
Você só chama explicitamente quando quer **gravar no meio** do fluxo (antes de
continuar mexendo):

```
import manpu as mp

using mp.open(target="dados.csv") as arq {
    arq.write(column=0, cell=0, content="parte 1")
    arq.save()                     // grava agora, sem esperar o fim

    arq.write(column=0, cell=1, content="parte 2")
    // ao sair, salva de novo automaticamente
}
```

---

## Relacionados

- [`ManpuFile.write()`](../write/write.md) — escrever antes de salvar
- [`manpu.open()`](../../open/open.md) — o `using` que salva sozinho
