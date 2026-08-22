"""
Módulo `sockets` da PoolScript — a API do módulo `socket` do Python, inteira,
na linguagem. Sockets TCP/UDP/UNIX crus (cliente E servidor), resolução de
nomes, conversões de endereço e byte-order.

Uso:
    import sockets

    s = sockets.socket(sockets.AF_INET, sockets.SOCK_STREAM)
    s.connect(("example.com", 80))
    s.sendall("GET / HTTP/1.0\\r\\n\\r\\n")
    dados = s.recv(4096)          # bytes
    s.close()

O espelho é 1:1 com o Python: mesmos nomes de funções/métodos/constantes,
mesmos formatos de retorno (endereço = tup (host, porta); recv devolve bytes;
accept devolve (conexao, endereco)). `send`/`sendall`/`sendto` aceitam str
(vira UTF-8) ou bytes.
"""
import socket as _py


# ── conversões PS <-> Python ────────────────────────────────────────────────

def _como_bytes(dados, quem):
    if isinstance(dados, (bytes, bytearray)):
        return bytes(dados)
    if isinstance(dados, str):
        return dados.encode("utf-8")
    raise TypeError(f"{quem}: esperava str ou bytes")


def _como_addr(addr, quem):
    """Endereço no formato do Python: (host, porta) — aceita tup ou list."""
    if isinstance(addr, (tuple, list)):
        return tuple(addr)
    if isinstance(addr, str):        # AF_UNIX: caminho
        return addr
    raise TypeError(f"{quem}: endereço deve ser (host, porta) ou caminho (AF_UNIX)")


# ── o objeto socket ─────────────────────────────────────────────────────────

class PoolSocket:
    """Espelho do socket.socket do Python — mesmos métodos, mesmos retornos."""

    # `type` é CAMPO do objeto (como no Python) — este marcador faz o
    # interpretador NÃO interceptar `.type` com o método universal type().
    _ps_type_e_campo = True

    def __init__(self, family=_py.AF_INET, type=_py.SOCK_STREAM, proto=0, _interno=None):
        self._s = _interno if _interno is not None else _py.socket(family, type, proto)

    def _viva(self, quem):
        """Erro CLARO (e igual ao da VM) pra operação em socket fechado —
        senão o Python cuspia '[Errno 9] Bad file descriptor'."""
        if self._s.fileno() < 0:
            raise RuntimeError(f"{quem}: socket fechado")

    # atributos (sem parênteses no Python; aqui métodos ficam disponíveis também)
    @property
    def family(self):  return int(self._s.family)

    @property
    def type(self):    return int(self._s.type)

    @property
    def proto(self):   return int(self._s.proto)

    # ── servidor ──
    def bind(self, addr):
        self._viva("bind")
        self._s.bind(_como_addr(addr, "bind"))

    def listen(self, backlog=None):
        self._viva("listen")
        self._s.listen() if backlog is None else self._s.listen(int(backlog))

    def accept(self):
        self._viva("accept")
        conn, addr = self._s.accept()
        return (PoolSocket(_interno=conn), tuple(addr) if isinstance(addr, tuple) else addr)

    # ── cliente ──
    def connect(self, addr):
        self._viva("connect")
        self._s.connect(_como_addr(addr, "connect"))

    def connect_ex(self, addr):
        self._viva("connect_ex")
        return int(self._s.connect_ex(_como_addr(addr, "connect_ex")))

    # ── dados ──
    def send(self, dados, flags=0):
        self._viva("send")
        return int(self._s.send(_como_bytes(dados, "send"), int(flags)))

    def sendall(self, dados, flags=0):
        self._viva("sendall")
        self._s.sendall(_como_bytes(dados, "sendall"), int(flags))

    def sendto(self, dados, addr, flags=0):
        self._viva("sendto")
        return int(self._s.sendto(_como_bytes(dados, "sendto"), int(flags),
                                  _como_addr(addr, "sendto")))

    def recv(self, bufsize, flags=0):
        self._viva("recv")
        return self._s.recv(int(bufsize), int(flags))

    def recvfrom(self, bufsize, flags=0):
        self._viva("recvfrom")
        dados, addr = self._s.recvfrom(int(bufsize), int(flags))
        return (dados, tuple(addr) if isinstance(addr, tuple) else addr)

    # ── controle ──
    def close(self):
        self._s.close()

    def shutdown(self, how):
        self._viva("shutdown")
        self._s.shutdown(int(how))

    def setsockopt(self, level, optname, value):
        self._viva("setsockopt")
        self._s.setsockopt(int(level), int(optname),
                           value if isinstance(value, (bytes, bytearray)) else int(value))

    def getsockopt(self, level, optname, buflen=None):
        self._viva("getsockopt")
        if buflen is None:
            return int(self._s.getsockopt(int(level), int(optname)))
        return self._s.getsockopt(int(level), int(optname), int(buflen))

    def settimeout(self, t):
        self._viva("settimeout")
        self._s.settimeout(None if t is None else float(t))

    def gettimeout(self):
        return self._s.gettimeout()

    def setblocking(self, flag):
        self._viva("setblocking")
        self._s.setblocking(bool(flag))

    def getblocking(self):
        return bool(self._s.getblocking())

    # ── info ──
    def getsockname(self):
        self._viva("getsockname")
        a = self._s.getsockname()
        return tuple(a) if isinstance(a, tuple) else a

    def getpeername(self):
        self._viva("getpeername")
        a = self._s.getpeername()
        return tuple(a) if isinstance(a, tuple) else a

    def fileno(self):
        return int(self._s.fileno())

    def detach(self):
        return int(self._s.detach())

    def dup(self) -> "PoolSocket":
        self._viva("dup")
        return PoolSocket(_interno=self._s.dup())

    # `using ... as` da PoolScript fecha no fim do bloco
    def __enter__(self):  return self

    def __exit__(self, *a):
        self._s.close()
        return False

    def __repr__(self):
        try:
            return (f"<socket family={int(self._s.family)} type={int(self._s.type)} "
                    f"proto={int(self._s.proto)} fd={self._s.fileno()}>")
        except OSError:
            return "<socket fechado>"


# ── construtores ────────────────────────────────────────────────────────────

def socket(family=_py.AF_INET, type=_py.SOCK_STREAM, proto=0) -> "PoolSocket":
    """sockets.socket(family, type, proto) — igual ao Python."""
    return PoolSocket(int(family), int(type), int(proto))


def create_connection(addr, timeout=None, source_address=None) -> "PoolSocket":
    kw = {}
    if timeout is not None:
        kw["timeout"] = float(timeout)
    if source_address is not None:
        kw["source_address"] = _como_addr(source_address, "create_connection")
    return PoolSocket(_interno=_py.create_connection(_como_addr(addr, "create_connection"), **kw))


def create_server(addr, backlog=None, reuse_port=False) -> "PoolSocket":
    kw = {"reuse_port": bool(reuse_port)}
    if backlog is not None:
        kw["backlog"] = int(backlog)
    return PoolSocket(_interno=_py.create_server(_como_addr(addr, "create_server"), **kw))


def socketpair():
    a, b = _py.socketpair()
    return (PoolSocket(_interno=a), PoolSocket(_interno=b))


# ── resolução de nomes ──────────────────────────────────────────────────────

def gethostname():
    return _py.gethostname()


def getfqdn(name=""):
    return _py.getfqdn(str(name))


def gethostbyname(name):
    return _py.gethostbyname(str(name))


def gethostbyname_ex(name):
    nome, aliases, addrs = _py.gethostbyname_ex(str(name))
    return (nome, list(aliases), list(addrs))


def gethostbyaddr(addr):
    nome, aliases, addrs = _py.gethostbyaddr(str(addr))
    return (nome, list(aliases), list(addrs))


def getaddrinfo(host, port, family=0, type=0, proto=0, flags=0):
    saida = []
    for fam, tp, pr, canon, sa in _py.getaddrinfo(host, port, int(family), int(type),
                                                  int(proto), int(flags)):
        saida.append((int(fam), int(tp), int(pr), canon,
                      tuple(sa) if isinstance(sa, tuple) else sa))
    return saida


def getnameinfo(addr, flags=0):
    host, serv = _py.getnameinfo(_como_addr(addr, "getnameinfo"), int(flags))
    return (host, serv)


def getservbyname(name, proto=None):
    return int(_py.getservbyname(str(name)) if proto is None
               else _py.getservbyname(str(name), str(proto)))


def getservbyport(port, proto=None):
    return _py.getservbyport(int(port)) if proto is None else _py.getservbyport(int(port), str(proto))


def getprotobyname(name):
    return int(_py.getprotobyname(str(name)))


# ── conversões de endereço e byte-order ─────────────────────────────────────

def htons(x):  return int(_py.htons(int(x)))
def htonl(x):  return int(_py.htonl(int(x)))
def ntohs(x):  return int(_py.ntohs(int(x)))
def ntohl(x):  return int(_py.ntohl(int(x)))


def inet_aton(ip):
    return _py.inet_aton(str(ip))


def inet_ntoa(b):
    return _py.inet_ntoa(_como_bytes(b, "inet_ntoa"))


def inet_pton(family, ip):
    return _py.inet_pton(int(family), str(ip))


def inet_ntop(family, b):
    return _py.inet_ntop(int(family), _como_bytes(b, "inet_ntop"))


# ── timeout padrão e utilidades ─────────────────────────────────────────────

def setdefaulttimeout(t):
    _py.setdefaulttimeout(None if t is None else float(t))


def getdefaulttimeout():
    return _py.getdefaulttimeout()


def has_dualstack_ipv6():
    return bool(_py.has_dualstack_ipv6())


def if_nameindex():
    return [(int(i), n) for i, n in _py.if_nameindex()]


def if_nametoindex(nome):
    return int(_py.if_nametoindex(str(nome)))


def if_indextoname(idx):
    return _py.if_indextoname(int(idx))


# ── EXPORTS: funções + constantes (mesmos nomes do Python) ─────────────────

_CONSTANTES = {}
for _nome in (
    # famílias
    "AF_INET", "AF_INET6", "AF_UNIX", "AF_UNSPEC", "AF_PACKET",
    # tipos
    "SOCK_STREAM", "SOCK_DGRAM", "SOCK_RAW", "SOCK_SEQPACKET",
    # níveis / opções
    "SOL_SOCKET", "SO_REUSEADDR", "SO_REUSEPORT", "SO_KEEPALIVE",
    "SO_BROADCAST", "SO_LINGER", "SO_RCVBUF", "SO_SNDBUF", "SO_ERROR",
    "SO_RCVTIMEO", "SO_SNDTIMEO", "SO_OOBINLINE", "SO_DONTROUTE", "SO_TYPE",
    "TCP_NODELAY", "TCP_KEEPIDLE", "TCP_KEEPINTVL", "TCP_KEEPCNT",
    # protocolos
    "IPPROTO_IP", "IPPROTO_TCP", "IPPROTO_UDP", "IPPROTO_ICMP", "IPPROTO_RAW",
    # shutdown
    "SHUT_RD", "SHUT_WR", "SHUT_RDWR",
    # flags de send/recv
    "MSG_PEEK", "MSG_WAITALL", "MSG_DONTWAIT", "MSG_OOB", "MSG_DONTROUTE", "MSG_TRUNC",
    # getaddrinfo/getnameinfo
    "AI_PASSIVE", "AI_CANONNAME", "AI_NUMERICHOST", "AI_NUMERICSERV",
    "AI_ADDRCONFIG", "AI_V4MAPPED", "AI_ALL",
    "NI_NUMERICHOST", "NI_NUMERICSERV", "NI_NOFQDN", "NI_NAMEREQD", "NI_DGRAM",
    # diversos
    "SOMAXCONN", "INADDR_ANY", "INADDR_LOOPBACK", "INADDR_BROADCAST",
    "IP_TTL", "IP_MULTICAST_TTL", "IP_MULTICAST_LOOP", "IP_ADD_MEMBERSHIP",
    "IP_DROP_MEMBERSHIP", "IPV6_V6ONLY",
):
    if hasattr(_py, _nome):
        _CONSTANTES[_nome] = int(getattr(_py, _nome))

EXPORTS = {
    # objeto
    "socket": socket,
    "create_connection": create_connection,
    "create_server": create_server,
    "socketpair": socketpair,
    # resolução
    "gethostname": gethostname,
    "getfqdn": getfqdn,
    "gethostbyname": gethostbyname,
    "gethostbyname_ex": gethostbyname_ex,
    "gethostbyaddr": gethostbyaddr,
    "getaddrinfo": getaddrinfo,
    "getnameinfo": getnameinfo,
    "getservbyname": getservbyname,
    "getservbyport": getservbyport,
    "getprotobyname": getprotobyname,
    # conversões
    "htons": htons, "htonl": htonl, "ntohs": ntohs, "ntohl": ntohl,
    "inet_aton": inet_aton, "inet_ntoa": inet_ntoa,
    "inet_pton": inet_pton, "inet_ntop": inet_ntop,
    # timeout padrão / utilidades
    "setdefaulttimeout": setdefaulttimeout,
    "getdefaulttimeout": getdefaulttimeout,
    "has_dualstack_ipv6": has_dualstack_ipv6,
    "if_nameindex": if_nameindex,
    "if_nametoindex": if_nametoindex,
    "if_indextoname": if_indextoname,
    **_CONSTANTES,
}
