# `os.ipmach()`

Descobre o **IP da máquina** na rede local, imprime no terminal e devolve como
string.

```
os.ipmach() -> str
```

---

## Uso

```
import os

ip = os.ipmach()
# imprime: [info] IP da sua máquina: 192.168.0.15
post("meu ip:", ip)
```

---

## Como funciona

Abre uma conexão UDP "de mentira" pra `8.8.8.8` só pra descobrir qual IP local
o sistema usaria pra sair na rede — é o IP da sua interface ativa. Se não
conseguir (sem rede), devolve `"127.0.0.1"`.

Útil pra mostrar em qual IP seu servidor jinker está acessível na rede local:

```
import os
ip = os.ipmach()
post(f"servidor acessível em http://{ip}:8080")
```

---

## Relacionados

- [jinker](../../jinker/jinker.md) — servir na rede (`host="0.0.0.0"`)
