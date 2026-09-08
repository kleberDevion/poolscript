# sockets — sockets de rede crus

A lib `sockets` traz os sockets TCP, UDP e UNIX crus — cliente **e** servidor —
mais resolução de nomes, conversões de endereço/byte-order e as constantes.
É a camada por baixo de qualquer protocolo: com ela você implementa o SEU
protocolo, do zero, sem framework no meio.

```
import sockets
```

Não confundir com o **jinker** (framework web HTTP/WebSocket) — o jinker é um
SERVIDOR pronto; a `sockets` é o metal cru por baixo, pra quando você quer
controlar cada byte da conexão.

Os formatos são os do sistema operacional —

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

Campos, lidos sem parênteses: **`family`**, **`type`**, **`proto`**.

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

São **63**, e o valor de cada uma é o do sistema — a lista abaixo é a tabela
`sockets` do motor, inteira. (Antes esta seção era um parágrafo com "…" e
"etc.": nomeava umas e escondia as outras, e quem procurava `IP_TTL` não
achava.)

**Família do endereço** — 1º argumento de `socket()`:

| Constante | Para |
|---|---|
| `AF_INET` | IPv4 |
| `AF_INET6` | IPv6 |
| `AF_UNIX` | soquete local por caminho de arquivo |
| `AF_UNSPEC` | "tanto faz" — só em `getaddrinfo` |
| `AF_PACKET` | quadro cru da camada de enlace (precisa de privilégio) |

**Tipo do soquete** — 2º argumento:

| Constante | Para |
|---|---|
| `SOCK_STREAM` | fluxo confiável e ordenado (TCP) |
| `SOCK_DGRAM` | datagrama solto (UDP) |
| `SOCK_RAW` | pacote cru, sem a camada de transporte |
| `SOCK_SEQPACKET` | datagrama confiável e ordenado |

**Nível e opções** — `setsockopt(nivel, opcao, valor)` / `getsockopt`:

| Constante | O que controla |
|---|---|
| `SOL_SOCKET` | o NÍVEL das opções `SO_*` abaixo |
| `SO_REUSEADDR` | reusar o endereço em `TIME_WAIT` — o que evita "Address already in use" ao reiniciar |
| `SO_REUSEPORT` | vários processos escutando a MESMA porta |
| `SO_KEEPALIVE` | sondar a conexão ociosa pra detectar queda |
| `SO_BROADCAST` | permitir envio pra endereço de difusão |
| `SO_LINGER` | quanto o `close()` espera pelo que falta enviar |
| `SO_RCVBUF` / `SO_SNDBUF` | tamanho do buffer de recepção / envio |
| `SO_ERROR` | lê e LIMPA o erro pendente do soquete |
| `SO_RCVTIMEO` / `SO_SNDTIMEO` | prazo de `recv` / `send` |
| `SO_OOBINLINE` | entregar dado urgente no fluxo normal |
| `SO_DONTROUTE` | não usar a tabela de rotas |
| `SO_TYPE` | só leitura: o tipo com que o soquete nasceu |
| `TCP_NODELAY` | desliga o Nagle — manda o pacote pequeno na hora |
| `TCP_KEEPIDLE` | ocioso antes da primeira sonda |
| `TCP_KEEPINTVL` | intervalo entre sondas |
| `TCP_KEEPCNT` | quantas sondas sem resposta derrubam |
| `IP_TTL` | tempo de vida do pacote (saltos) |
| `IP_MULTICAST_TTL` | idem, para multicast |
| `IP_MULTICAST_LOOP` | receber de volta o que este host enviou |
| `IP_ADD_MEMBERSHIP` / `IP_DROP_MEMBERSHIP` | entrar / sair de um grupo multicast |
| `IPV6_V6ONLY` | soquete IPv6 aceita SÓ IPv6 (não mapeia IPv4) |

**Protocolo** — 3º argumento de `socket()`:

| Constante | Para |
|---|---|
| `IPPROTO_IP` | o padrão do tipo escolhido |
| `IPPROTO_TCP` / `IPPROTO_UDP` / `IPPROTO_ICMP` | forçar o protocolo |
| `IPPROTO_RAW` | cru, com o cabeçalho IP por sua conta |

**Desligar meia conexão** — `shutdown(como)`:

| Constante | Fecha |
|---|---|
| `SHUT_RD` | a leitura |
| `SHUT_WR` | a escrita (o outro lado vê o fim do fluxo) |
| `SHUT_RDWR` | as duas |

**Flags de `send`/`recv`**:

| Constante | O que faz |
|---|---|
| `MSG_PEEK` | lê SEM tirar da fila |
| `MSG_WAITALL` | só volta com tudo o que se pediu |
| `MSG_DONTWAIT` | não bloqueia nesta chamada |
| `MSG_OOB` | dado urgente (fora de banda) |
| `MSG_DONTROUTE` | ignora a tabela de rotas |
| `MSG_TRUNC` | (em `recv`) informa o tamanho REAL, mesmo truncado |

**`getaddrinfo` (`AI_*`) e `getnameinfo` (`NI_*`)**:

| Constante | O que pede |
|---|---|
| `AI_PASSIVE` | endereço pra ESCUTAR (bind), não pra conectar |
| `AI_CANONNAME` | trazer também o nome canônico |
| `AI_NUMERICHOST` | o host já é numérico — não resolver |
| `AI_NUMERICSERV` | o serviço já é número de porta |
| `AI_ADDRCONFIG` | só famílias que a máquina tem configuradas |
| `AI_V4MAPPED` | IPv4 aparece mapeado em IPv6 |
| `AI_ALL` | com `AI_V4MAPPED`, devolve IPv4 **e** IPv6 |
| `NI_NUMERICHOST` | devolver o IP, não o nome |
| `NI_NUMERICSERV` | devolver o número da porta, não o nome do serviço |
| `NI_NOFQDN` | só a primeira parte do nome |
| `NI_NAMEREQD` | falhar se não houver nome (em vez de devolver o IP) |
| `NI_DGRAM` | consultar como UDP |

**Endereços e limites**:

| Constante | Vale |
|---|---|
| `SOMAXCONN` | maior fila de espera que o sistema aceita em `listen()` |
| `INADDR_ANY` | "todas as interfaces" (`0.0.0.0`) |
| `INADDR_LOOPBACK` | só a máquina local (`127.0.0.1`) |
| `INADDR_BROADCAST` | difusão (`255.255.255.255`) |

---

## Exemplos

Conversões e resolução (saída verificada na VM):

```ps
import sockets
post(sockets.htons(1), sockets.ntohs(sockets.htons(1)))
post(sockets.inet_ntoa(sockets.inet_aton("10.20.30.40")))
post(sockets.inet_ntop(sockets.AF_INET, sockets.inet_pton(sockets.AF_INET, "8.8.4.4")))
post(sockets.gethostbyname("localhost"))
post(sockets.getservbyname("http", "tcp"))
post(sockets.getprotobyname("tcp"))
```

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

```ps
import sockets
srv = sockets.socket(sockets.AF_INET, sockets.SOCK_STREAM)
srv.setsockopt(sockets.SOL_SOCKET, sockets.SO_REUSEADDR, 1)
srv.bind(("127.0.0.1", 0))          # porta 0 = o sistema escolhe
srv.listen(4)
porta = srv.getsockname()[1]

cli = sockets.socket()
cli.settimeout(5)
cli.connect(("127.0.0.1", porta))
par = srv.accept()                  # (conexao, endereco)
conn = par[0]

cli.sendall("ola servidor")
post(str(conn.recv(64).decode()))
conn.sendall("resposta")
post(str(cli.recv(64).decode()))
conn.close(); cli.close(); srv.close()
```

```saida
ola servidor
resposta
```

UDP (datagramas, sem conexão):

```ps
import sockets
u1 = sockets.socket(sockets.AF_INET, sockets.SOCK_DGRAM)
u1.bind(("127.0.0.1", 0))
porta = u1.getsockname()[1]

u2 = sockets.socket(sockets.AF_INET, sockets.SOCK_DGRAM)
post(u2.sendto("datagrama", ("127.0.0.1", porta)))
r = u1.recvfrom(64)                 # (bytes, endereco)
post(str(r[0].decode()), r[1][0])
u1.close(); u2.close()
```

```saida
9
datagrama 127.0.0.1
```

Timeout capturável:

```ps
import sockets
u = sockets.socket(sockets.AF_INET, sockets.SOCK_DGRAM)
u.bind(("127.0.0.1", 0))
u.settimeout(0.2)
try {
    u.recv(16)                      # nada chega -> estoura o prazo
} catch (e) {
    post("timed out" in str(e))
}
u.close()
```

```saida
True
```

Cliente HTTP na unha:

```ps
import sockets

s = sockets.create_connection(("example.com", 80), 10)
s.sendall("GET / HTTP/1.0\r\nHost: example.com\r\n\r\n")
resposta = s.recv(4096)
post(str(resposta.decode())[0:15])   # "HTTP/1.0 200 OK"
s.close()
```
