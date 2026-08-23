# sockets — sockets de rede crus (a API do `socket` do Python)

A lib `sockets` traz **toda a API do módulo `socket` do Python** pra
PoolScript: sockets TCP, UDP e UNIX crus — cliente **e** servidor — mais
resolução de nomes, conversões de endereço/byte-order e as constantes.
É a camada por baixo de qualquer protocolo: com ela você implementa o SEU
protocolo, do zero, sem framework no meio.

```
import sockets
```

Não confundir com o **jinker** (framework web HTTP/WebSocket) — o jinker é um
SERVIDOR pronto; a `sockets` é o metal cru por baixo, pra quando você quer
controlar cada byte da conexão.

O espelho é **1:1 com o Python**: mesmos nomes, mesmos formatos —

- endereço = tup `(host, porta)` (IPv6: `(host, porta, flowinfo, scope_id)`;
  `AF_UNIX`: caminho str);
- `recv`/`recvfrom` devolvem **bytes**; `send`/`sendall`/`sendto` aceitam
  **str** (vira UTF-8) ou **bytes**;
- `accept()` devolve `(conexao, endereco)`; `recvfrom()` devolve
  `(dados, endereco)`.

No VM em C, a I/O bloqueante (connect/accept/recv/send) roda no pool de
threads e **cede a fibra** — dentro de um handler do jinker um socket lento
NÃO trava os outros requests (o mesmo async uniforme de DB/HTTP/mail).

---

## O objeto socket

`sockets.socket(family, type, proto)` cria o socket (defaults: `AF_INET`,
`SOCK_STREAM`, `0` — um TCP/IPv4).

| Método | O que faz |
|---|---|
| `bind(endereco)` | prende a um endereço local |
| `listen(backlog)` | vira servidor (backlog opcional) |
| `accept()` | espera conexão → `(conexao, endereco)` |
| `connect(endereco)` | conecta (erro se falhar) |
| `connect_ex(endereco)` | conecta; devolve `0` ok ou o errno (não levanta) |
| `send(dados, flags)` | envia; devolve quantos bytes FORAM |
| `sendall(dados, flags)` | envia TUDO (insiste até acabar) |
| `sendto(dados, endereco)` | UDP: envia pro endereço; devolve bytes enviados |
| `recv(bufsize, flags)` | recebe até bufsize → bytes |
| `recvfrom(bufsize, flags)` | UDP: → `(bytes, endereco)` |
| `close()` | fecha (métodos depois disso: erro "socket fechado") |
| `shutdown(how)` | encerra um lado: `SHUT_RD`/`SHUT_WR`/`SHUT_RDWR` |
| `setsockopt(level, opt, valor)` | opção do socket (int ou bytes) |
| `getsockopt(level, opt, buflen)` | lê opção → int (ou bytes, com buflen) |
| `settimeout(t)` | prazo em segundos; `null` = bloqueante; `0` = não-bloqueante |
| `gettimeout()` | o prazo atual (flo) ou `null` |
| `setblocking(flag)` | `true` = `settimeout(null)`; `false` = `settimeout(0)` |
| `getblocking()` | `true` se bloqueante (prazo `null` ou > 0) |
| `getsockname()` | endereço local → tup |
| `getpeername()` | endereço do outro lado → tup |
| `fileno()` | o fd cru (−1 se fechado) |
| `detach()` | devolve o fd e SOLTA a posse (fileno vira −1) |
| `dup()` | duplica → outro socket |

Campos (sem parênteses, como no Python): **`family`**, **`type`**, **`proto`**.

`using sockets.socket() as s { ... }` fecha sozinho no fim do bloco.

O timeout estourado dá erro com **"timed out"** na mensagem — capturável com
`try/catch`.

## Funções do módulo

| Função | O que faz |
|---|---|
| `socket(family, type, proto)` | cria um socket |
| `create_connection(endereco, timeout, source_address)` | TCP já conectado (atalho) |
| `create_server(endereco, backlog, reuse_port)` | TCP já em listen com `SO_REUSEADDR` |
| `socketpair()` | par conectado (AF_UNIX) → `(a, b)` |
| `gethostname()` | nome desta máquina |
| `getfqdn(nome)` | nome totalmente qualificado |
| `gethostbyname(nome)` | nome → IP (str) |
| `gethostbyname_ex(nome)` | → `(nome, [aliases], [ips])` |
| `gethostbyaddr(ip)` | IP → `(nome, [aliases], [ips])` |
| `getaddrinfo(host, porta, family, type, proto, flags)` | resolução completa → lista de `(family, type, proto, canonname, endereco)` |
| `getnameinfo(endereco, flags)` | reverso → `(host, servico)` |
| `getservbyname(servico, proto)` | `"http","tcp"` → `80` |
| `getservbyport(porta, proto)` | `80,"tcp"` → `"http"` |
| `getprotobyname(nome)` | `"tcp"` → `6` |
| `htons(x)` / `htonl(x)` / `ntohs(x)` / `ntohl(x)` | byte-order host↔rede |
| `inet_aton(ip)` / `inet_ntoa(bytes)` | IPv4 str ↔ 4 bytes |
| `inet_pton(family, ip)` / `inet_ntop(family, bytes)` | v4/v6 str ↔ bytes |
| `setdefaulttimeout(t)` / `getdefaulttimeout()` | prazo padrão pros sockets NOVOS |
| `has_dualstack_ipv6()` | dá pra servir v4+v6 num socket só? |
| `if_nameindex()` / `if_nametoindex(n)` / `if_indextoname(i)` | interfaces de rede |

## Constantes

As mesmas do Python — famílias `AF_INET`, `AF_INET6`, `AF_UNIX`, `AF_UNSPEC`,
`AF_PACKET`; tipos `SOCK_STREAM`, `SOCK_DGRAM`, `SOCK_RAW`, `SOCK_SEQPACKET`;
opções `SOL_SOCKET`, `SO_REUSEADDR`, `SO_REUSEPORT`, `SO_KEEPALIVE`,
`SO_BROADCAST`, `SO_LINGER`, `SO_RCVBUF`, `SO_SNDBUF`, `SO_ERROR`,
`SO_RCVTIMEO`, `SO_SNDTIMEO`, `TCP_NODELAY`, `TCP_KEEPIDLE`...; protocolos
`IPPROTO_TCP`, `IPPROTO_UDP`, `IPPROTO_IP`, `IPPROTO_ICMP`; shutdown
`SHUT_RD`, `SHUT_WR`, `SHUT_RDWR`; flags `MSG_PEEK`, `MSG_WAITALL`,
`MSG_DONTWAIT`...; getaddrinfo `AI_PASSIVE`, `AI_CANONNAME`,
`NI_NUMERICHOST`...; e `SOMAXCONN`, `INADDR_ANY`, `IP_TTL`,
`IP_ADD_MEMBERSHIP`, `IPV6_V6ONLY` etc.

---

## Exemplos

Conversões e resolução (saída verificada nos dois motores):

![exemplo 1](../assets/sockets__sockets_ex1.png)

<details><summary>código</summary>

```ps
import sockets
post(sockets.htons(1), sockets.ntohs(sockets.htons(1)))
post(sockets.inet_ntoa(sockets.inet_aton("10.20.30.40")))
post(sockets.inet_ntop(sockets.AF_INET, sockets.inet_pton(sockets.AF_INET, "8.8.4.4")))
post(sockets.gethostbyname("localhost"))
post(sockets.getservbyname("http", "tcp"))
post(sockets.getprotobyname("tcp"))
```

</details>

```saida
256 1
10.20.30.40
8.8.4.4
127.0.0.1
80
6
```

Servidor + cliente TCP no mesmo script (o `connect` completa contra o backlog
do `listen`, então dá pra testar sem thread):

![exemplo 2](../assets/sockets__sockets_ex2.png)

<details><summary>código</summary>

```ps
import sockets
srv = sockets.socket(sockets.AF_INET, sockets.SOCK_STREAM)
srv.setsockopt(sockets.SOL_SOCKET, sockets.SO_REUSEADDR, 1)
srv.bind(("127.0.0.1", 0))          // porta 0 = o sistema escolhe
srv.listen(4)
porta = srv.getsockname()[1]

cli = sockets.socket()
cli.settimeout(5)
cli.connect(("127.0.0.1", porta))
par = srv.accept()                  // (conexao, endereco)
conn = par[0]

cli.sendall("ola servidor")
post(str(conn.recv(64).decode()))
conn.sendall("resposta")
post(str(cli.recv(64).decode()))
conn.close(); cli.close(); srv.close()
```

</details>

```saida
ola servidor
resposta
```

UDP (datagramas, sem conexão):

![exemplo 3](../assets/sockets__sockets_ex3.png)

<details><summary>código</summary>

```ps
import sockets
u1 = sockets.socket(sockets.AF_INET, sockets.SOCK_DGRAM)
u1.bind(("127.0.0.1", 0))
porta = u1.getsockname()[1]

u2 = sockets.socket(sockets.AF_INET, sockets.SOCK_DGRAM)
post(u2.sendto("datagrama", ("127.0.0.1", porta)))
r = u1.recvfrom(64)                 // (bytes, endereco)
post(str(r[0].decode()), r[1][0])
u1.close(); u2.close()
```

</details>

```saida
9
datagrama 127.0.0.1
```

Timeout capturável:

![exemplo 4](../assets/sockets__sockets_ex4.png)

<details><summary>código</summary>

```ps
import sockets
u = sockets.socket(sockets.AF_INET, sockets.SOCK_DGRAM)
u.bind(("127.0.0.1", 0))
u.settimeout(0.2)
try {
    u.recv(16)                      // ninguém manda nada -> estoura o prazo
} catch (e) {
    post("timed out" in str(e))
}
u.close()
```

</details>

```saida
True
```

Cliente HTTP na unha (é literalmente o exemplo clássico do Python):

![exemplo 5](../assets/sockets__sockets_ex5.png)

<details><summary>código</summary>

```ps
import sockets

s = sockets.create_connection(("example.com", 80), 10)
s.sendall("GET / HTTP/1.0\r\nHost: example.com\r\n\r\n")
resposta = s.recv(4096)
post(str(resposta.decode())[0:15])   // "HTTP/1.0 200 OK"
s.close()
```

</details>
