"""lib `sockets` — a API do módulo socket do Python na PoolScript, IGUAL nos
dois motores: objeto socket (TCP/UDP/UNIX), resolução de nomes, conversões e
constantes. Cada programa roda no interpretador E no `pool` (VM em C) e o
stdout tem que bater byte a byte. VM em C não se pula (skip = falso verde)."""
import os
import subprocess
import sys
from pathlib import Path

import pytest

RAIZ = Path(__file__).resolve().parent.parent
POOL = RAIZ / "pool"
SRC = RAIZ / "src"

assert POOL.exists(), "binário pool não compilado — rode ./rebuild_vm.sh (VM em C não se pula: skip = falso verde)"


def _roda(cmd, tmp_path, src, nome):
    ps = tmp_path / nome
    ps.write_text(src, encoding="utf-8")
    env = dict(os.environ, PYTHONPATH=str(SRC))
    return subprocess.run(cmd + [str(ps)], capture_output=True, text=True,
                          env=env, timeout=30)


def _paridade(tmp_path, src, esperado=None):
    ri = _roda([sys.executable, "-m", "poolscript"], tmp_path, src, "i.ps")
    rv = _roda([str(POOL)], tmp_path, src, "v.ps")
    assert ri.returncode == 0, ("interp:", ri.stdout + ri.stderr)
    assert rv.returncode == 0, ("VM:", rv.stdout + rv.stderr)
    assert rv.stdout == ri.stdout, ("VM:", repr(rv.stdout), "INTERP:", repr(ri.stdout))
    if esperado is not None:
        assert rv.stdout == esperado, ("saida:", repr(rv.stdout), "esperado:", repr(esperado))


def test_constantes(tmp_path):
    _paridade(tmp_path, """import sockets
post(sockets.AF_INET, sockets.AF_INET6, sockets.AF_UNIX)
post(sockets.SOCK_STREAM, sockets.SOCK_DGRAM, sockets.SOCK_RAW)
post(sockets.SOL_SOCKET, sockets.SO_REUSEADDR, sockets.TCP_NODELAY)
post(sockets.SHUT_RD, sockets.SHUT_WR, sockets.SHUT_RDWR)
post(sockets.IPPROTO_TCP, sockets.IPPROTO_UDP)
post(sockets.MSG_PEEK, sockets.AI_PASSIVE, sockets.NI_NUMERICHOST)
post(sockets.INADDR_ANY, sockets.SOMAXCONN > 0)
""")


def test_conversoes(tmp_path):
    _paridade(tmp_path, """import sockets
post(sockets.htons(1), sockets.ntohs(sockets.htons(1)))
post(sockets.htonl(1), sockets.ntohl(sockets.htonl(1)))
post(sockets.inet_ntoa(sockets.inet_aton("10.20.30.40")))
post(len(sockets.inet_pton(sockets.AF_INET, "1.2.3.4")))
post(len(sockets.inet_pton(sockets.AF_INET6, "::1")))
post(sockets.inet_ntop(sockets.AF_INET, sockets.inet_pton(sockets.AF_INET, "8.8.4.4")))
""", "256 1\n16777216 1\n10.20.30.40\n4\n16\n8.8.4.4\n")


def test_resolucao(tmp_path):
    _paridade(tmp_path, """import sockets
post(sockets.gethostbyname("localhost"))
post(sockets.getservbyname("http", "tcp"))
post(sockets.getservbyport(80, "tcp"))
post(sockets.getprotobyname("tcp"))
infos = sockets.getaddrinfo("127.0.0.1", 80, sockets.AF_INET, sockets.SOCK_STREAM)
post(len(infos) > 0)
primeira = infos[0]
post(primeira[0] == sockets.AF_INET, primeira[4][0], primeira[4][1])
ni = sockets.getnameinfo(("127.0.0.1", 80), sockets.NI_NUMERICHOST)
post(ni[0])
post(len(sockets.gethostname()) > 0)
""", "127.0.0.1\n80\nhttp\n6\nTrue\nTrue 127.0.0.1 80\n127.0.0.1\nTrue\n")


def test_tcp_loopback(tmp_path):
    _paridade(tmp_path, """import sockets
import bytes
srv = sockets.socket(sockets.AF_INET, sockets.SOCK_STREAM)
srv.setsockopt(sockets.SOL_SOCKET, sockets.SO_REUSEADDR, 1)
post(srv.getsockopt(sockets.SOL_SOCKET, sockets.SO_REUSEADDR) != 0)
srv.bind(("127.0.0.1", 0))
srv.listen(4)
porta = srv.getsockname()[1]
post(porta > 0)

cli = sockets.socket()
cli.settimeout(5)
post(cli.gettimeout())
cli.connect(("127.0.0.1", porta))
par = srv.accept()
conn = par[0]
post(par[1][0])

cli.sendall("ola servidor")
post(str(conn.recv(64).decode()))
enviados = conn.send(bytes.new("resposta"))
post(enviados)
post(str(cli.recv(64).decode()))
post(cli.getpeername()[1] == porta)
conn.close(); cli.close(); srv.close()
post(srv.fileno())
""", "True\nTrue\n5.0\n127.0.0.1\nola servidor\n8\nresposta\nTrue\n-1\n")


def test_udp(tmp_path):
    _paridade(tmp_path, """import sockets
u1 = sockets.socket(sockets.AF_INET, sockets.SOCK_DGRAM)
u1.bind(("127.0.0.1", 0))
pu = u1.getsockname()[1]
u2 = sockets.socket(sockets.AF_INET, sockets.SOCK_DGRAM)
n = u2.sendto("datagrama", ("127.0.0.1", pu))
post(n)
r = u1.recvfrom(64)
post(str(r[0].decode()), r[1][0])
u1.close(); u2.close()
""", "9\ndatagrama 127.0.0.1\n")


def test_timeout_e_blocking(tmp_path):
    _paridade(tmp_path, """import sockets
s = sockets.socket()
post(s.gettimeout())
post(s.getblocking())
s.settimeout(0.2)
post(s.gettimeout(), s.getblocking())
s.setblocking(false)
post(s.gettimeout(), s.getblocking())
s.setblocking(true)
post(s.gettimeout(), s.getblocking())
s.close()
// recv com prazo estoura "timed out"
u = sockets.socket(sockets.AF_INET, sockets.SOCK_DGRAM)
u.bind(("127.0.0.1", 0))
u.settimeout(0.2)
try {
    u.recv(16)
    post("NAO devia chegar aqui")
} catch (e) {
    post("timed out" in str(e))
}
u.close()
""", "null\nTrue\n0.2 True\n0.0 False\nnull True\nTrue\n")


def test_connect_ex_e_erro(tmp_path):
    _paridade(tmp_path, """import sockets
c = sockets.socket()
c.settimeout(1)
post(c.connect_ex(("127.0.0.1", 1)) != 0)
c.close()
// operar em socket fechado é erro claro
try {
    c.send("x")
    post("NAO devia")
} catch (e) {
    post("fechado" in str(e))
}
""", "True\nTrue\n")


def test_socketpair_dup_detach(tmp_path):
    _paridade(tmp_path, """import sockets
par = sockets.socketpair()
a = par[0]
b = par[1]
a.sendall("ping")
post(str(b.recv(16).decode()))
d = b.dup()
a.sendall("de novo")
post(str(d.recv(16).decode()))
fd = d.detach()
post(fd >= 0, d.fileno())
a.close(); b.close()
""", "ping\nde novo\nTrue -1\n")


def test_campos_e_using(tmp_path):
    _paridade(tmp_path, """import sockets
s = sockets.socket(sockets.AF_INET, sockets.SOCK_DGRAM)
post(s.family == sockets.AF_INET, s.type == sockets.SOCK_DGRAM, s.proto)
s.close()
using sockets.socket() as w {
    post(w.fileno() >= 0)
}
post(w.fileno())
""", "True True 0\nTrue\n-1\n")


def test_create_connection_server(tmp_path):
    _paridade(tmp_path, """import sockets
srv = sockets.create_server(("127.0.0.1", 0), 8)
porta = srv.getsockname()[1]
cli = sockets.create_connection(("127.0.0.1", porta), 5)
par = srv.accept()
conn = par[0]
cli.sendall("via create")
post(str(conn.recv(32).decode()))
conn.close(); cli.close(); srv.close()
// default timeout
sockets.setdefaulttimeout(2)
post(sockets.getdefaulttimeout())
s = sockets.socket()
post(s.gettimeout())
s.close()
sockets.setdefaulttimeout(null)
post(sockets.getdefaulttimeout())
""", "via create\n2.0\n2.0\nnull\n")


def test_shutdown_e_hostex(tmp_path):
    _paridade(tmp_path, """import sockets
par = sockets.socketpair()
a = par[0]
b = par[1]
a.sendall("fim")
a.shutdown(sockets.SHUT_WR)
post(str(b.recv(16).decode()))
post(len(b.recv(16)))
a.close(); b.close()
ex = sockets.gethostbyname_ex("localhost")
post(len(ex) == 3, "127.0.0.1" in ex[2])
post(sockets.has_dualstack_ipv6() is bool)
ifs = sockets.if_nameindex()
post(len(ifs) > 0)
post(sockets.if_nametoindex(ifs[0][1]) == ifs[0][0])
post(sockets.if_indextoname(ifs[0][0]) == ifs[0][1])
""", "fim\n0\nTrue True\nTrue\nTrue\nTrue\nTrue\n")
