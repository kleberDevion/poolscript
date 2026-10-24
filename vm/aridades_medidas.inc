/* GERADO por scripts/mede_aridades.pr — NAO edite a mao.
 *
 * Por nativo: dono, membro, o nome que a frase de quantidade usa, a lista
 * de nomes de parametro (0 = nao aceita nome), quantos argumentos aceita
 * (minimo, maximo; -1 = sem teto) e a frase com que recusa UM argumento de
 * cada tipo basico, na ordem int flo str list tup dict byte bool Null
 * (0 = aceita). MEDIDO: o script chama o nativo. Ordenado por (dono,
 * membro) em ordem de bytes: vm/ps_retornos.c faz bsearch.
 *
 * Cada texto e uma POSICAO em AR_TEXTO (0 = NULL), nao um ponteiro:
 * ponteiro em dado inicializado e relocacao em toda partida do binario.
 *
 *     jinga scripts/mede_aridades.pr
 */
static const char AR_TEXTO[] =
    "\0"
    "CorsConfig\0"
    "local\0"
    "options\0"
    "subset=Null\0"
    "origins\0"
    "permiser\0"
    "Object of type byte is not JSON serializable\0"
    "Jinker\0"
    "channel\0"
    "forAll=Null\0"
    "delete() argument 1 must be str, not int\0"
    "delete() argument 1 must be str, not flo\0"
    "delete() argument 1 must be str, not list\0"
    "delete() argument 1 must be str, not tup\0"
    "delete() argument 1 must be str, not dict\0"
    "delete() argument 1 must be str, not byte\0"
    "delete() argument 1 must be str, not bool\0"
    "delete() argument 1 must be str, not Null\0"
    "delete\0"
    "path,auth=Null,middleware=Null,model=Null\0"
    "get() argument 1 must be str, not int\0"
    "get() argument 1 must be str, not flo\0"
    "get() argument 1 must be str, not list\0"
    "get() argument 1 must be str, not tup\0"
    "get() argument 1 must be str, not dict\0"
    "get() argument 1 must be str, not byte\0"
    "get() argument 1 must be str, not bool\0"
    "get() argument 1 must be str, not Null\0"
    "get\0"
    "middleware\0"
    "'str' object is not callable\0"
    "name\0"
    "patch() argument 1 must be str, not int\0"
    "patch() argument 1 must be str, not flo\0"
    "patch() argument 1 must be str, not list\0"
    "patch() argument 1 must be str, not tup\0"
    "patch() argument 1 must be str, not dict\0"
    "patch() argument 1 must be str, not byte\0"
    "patch() argument 1 must be str, not bool\0"
    "patch() argument 1 must be str, not Null\0"
    "patch\0"
    "post() argument 1 must be str, not int\0"
    "post() argument 1 must be str, not flo\0"
    "post() argument 1 must be str, not list\0"
    "post() argument 1 must be str, not tup\0"
    "post() argument 1 must be str, not dict\0"
    "post() argument 1 must be str, not byte\0"
    "post() argument 1 must be str, not bool\0"
    "post() argument 1 must be str, not Null\0"
    "post\0"
    "put() argument 1 must be str, not int\0"
    "put() argument 1 must be str, not flo\0"
    "put() argument 1 must be str, not list\0"
    "put() argument 1 must be str, not tup\0"
    "put() argument 1 must be str, not dict\0"
    "put() argument 1 must be str, not byte\0"
    "put() argument 1 must be str, not bool\0"
    "put() argument 1 must be str, not Null\0"
    "put\0"
    "route() argument 1 must be str, not int\0"
    "route() argument 1 must be str, not flo\0"
    "route() argument 1 must be str, not list\0"
    "route() argument 1 must be str, not tup\0"
    "route() argument 1 must be str, not dict\0"
    "route() argument 1 must be str, not byte\0"
    "route() argument 1 must be str, not bool\0"
    "route() argument 1 must be str, not Null\0"
    "route\0"
    "path,methods=Null,auth=Null,middleware=Null,model=Null\0"
    "socket() argument 'path' must be str or Null, not int\0"
    "socket() argument 'path' must be str or Null, not flo\0"
    "socket() argument 'path' must be str or Null, not list\0"
    "socket() argument 'path' must be str or Null, not tup\0"
    "socket() argument 'path' must be str or Null, not dict\0"
    "socket() argument 'path' must be str or Null, not byte\0"
    "socket() argument 'path' must be str or Null, not bool\0"
    "socket\0"
    "path=Null,channel=False\0"
    "static_folder\0"
    "static_url\0"
    "JinkerResponse\0"
    "cookie\0"
    "nome,valor,path=\"/\",max_age=Null,httponly=True,secure=False,samesite=\"Lax\",domain=Null\0"
    "header\0"
    "key,value\0"
    "json\0"
    "data,status=Null\0"
    "send\0"
    "text,status=Null\0"
    "'flo' object cannot be interpreted as an integer\0"
    "'str' object cannot be interpreted as an integer\0"
    "'list' object cannot be interpreted as an integer\0"
    "'tup' object cannot be interpreted as an integer\0"
    "'dict' object cannot be interpreted as an integer\0"
    "'byte' object cannot be interpreted as an integer\0"
    "'Null' object cannot be interpreted as an integer\0"
    "status\0"
    "code\0"
    "'int' object is not callable\0"
    "status_code\0"
    "attach() argument 1 must be str, not int\0"
    "attach() argument 1 must be str, not flo\0"
    "attach() argument 1 must be str, not list\0"
    "attach() argument 1 must be str, not tup\0"
    "attach() argument 1 must be str, not dict\0"
    "attach() argument 1 must be str, not byte\0"
    "attach() argument 1 must be str, not bool\0"
    "attach() argument 1 must be str, not Null\0"
    "MailMessage\0"
    "attach\0"
    "arquivo_ou_caminho\0"
    "body() argument 1 must be str, not int\0"
    "body() argument 1 must be str, not flo\0"
    "body() argument 1 must be str, not list\0"
    "body() argument 1 must be str, not tup\0"
    "body() argument 1 must be str, not dict\0"
    "body() argument 1 must be str, not byte\0"
    "body() argument 1 must be str, not bool\0"
    "body() argument 1 must be str, not Null\0"
    "body\0"
    "conteudo,is_html=False\0"
    "fromAddress() argument 1 must be str, not int\0"
    "fromAddress() argument 1 must be str, not flo\0"
    "fromAddress() argument 1 must be str, not list\0"
    "fromAddress() argument 1 must be str, not tup\0"
    "fromAddress() argument 1 must be str, not dict\0"
    "fromAddress() argument 1 must be str, not byte\0"
    "fromAddress() argument 1 must be str, not bool\0"
    "fromAddress() argument 1 must be str, not Null\0"
    "fromAddress\0"
    "endereco\0"
    "get_as_string\0"
    "subject() argument 1 must be str, not int\0"
    "subject() argument 1 must be str, not flo\0"
    "subject() argument 1 must be str, not list\0"
    "subject() argument 1 must be str, not tup\0"
    "subject() argument 1 must be str, not dict\0"
    "subject() argument 1 must be str, not byte\0"
    "subject() argument 1 must be str, not bool\0"
    "subject() argument 1 must be str, not Null\0"
    "subject\0"
    "titulo\0"
    "toAddress() argument 1 must be str, not int\0"
    "toAddress() argument 1 must be str, not flo\0"
    "toAddress() argument 1 must be str, not list\0"
    "toAddress() argument 1 must be str, not tup\0"
    "toAddress() argument 1 must be str, not dict\0"
    "toAddress() argument 1 must be str, not byte\0"
    "toAddress() argument 1 must be str, not bool\0"
    "toAddress() argument 1 must be str, not Null\0"
    "toAddress\0"
    "MailReader\0"
    "id\0"
    "close\0"
    "conn() argument 1 must be str, not int\0"
    "conn() argument 1 must be str, not flo\0"
    "conn() argument 1 must be str, not list\0"
    "conn() argument 1 must be str, not tup\0"
    "conn() argument 1 must be str, not dict\0"
    "conn() argument 1 must be str, not byte\0"
    "conn() argument 1 must be str, not bool\0"
    "conn() argument 1 must be str, not Null\0"
    "conn\0"
    "host,porta=Null\0"
    "login\0"
    "usuario,senha\0"
    "search\0"
    "criterion_type=\"ALL\",term=Null,limit=Null,include_body=False\0"
    "select\0"
    "folder=\"INBOX\",readonly=True\0"
    "MailServer\0"
    "quit\0"
    "to_or_msg,subject=Null,body=Null,html=False\0"
    "Parsing\0"
    "Arrayformatt\0"
    "value,to_type=Null\0"
    "JSONformatt\0"
    "TransientValue\0"
    "Tuplasformatt\0"
    "boolean\0"
    "floating\0"
    "integer\0"
    "string\0"
    "findall() argument 1 must be str, not int\0"
    "findall() argument 1 must be str, not flo\0"
    "findall() argument 1 must be str, not list\0"
    "findall() argument 1 must be str, not tup\0"
    "findall() argument 1 must be str, not dict\0"
    "findall() argument 1 must be str, not byte\0"
    "findall() argument 1 must be str, not bool\0"
    "findall() argument 1 must be str, not Null\0"
    "Pattern\0"
    "findall\0"
    "match() argument 1 must be str, not int\0"
    "match() argument 1 must be str, not flo\0"
    "match() argument 1 must be str, not list\0"
    "match() argument 1 must be str, not tup\0"
    "match() argument 1 must be str, not dict\0"
    "match() argument 1 must be str, not byte\0"
    "match() argument 1 must be str, not bool\0"
    "match() argument 1 must be str, not Null\0"
    "fullmatch\0"
    "match\0"
    "pattern\0"
    "search() argument 1 must be str, not int\0"
    "search() argument 1 must be str, not flo\0"
    "search() argument 1 must be str, not list\0"
    "search() argument 1 must be str, not tup\0"
    "search() argument 1 must be str, not dict\0"
    "search() argument 1 must be str, not byte\0"
    "search() argument 1 must be str, not bool\0"
    "search() argument 1 must be str, not Null\0"
    "split() argument 1 must be str, not int\0"
    "split() argument 1 must be str, not flo\0"
    "split() argument 1 must be str, not list\0"
    "split() argument 1 must be str, not tup\0"
    "split() argument 1 must be str, not dict\0"
    "split() argument 1 must be str, not byte\0"
    "split() argument 1 must be str, not bool\0"
    "split() argument 1 must be str, not Null\0"
    "split\0"
    "string,maxsplit=0\0"
    "sub\0"
    "repl,string,count=Null\0"
    "PoolConnection\0"
    "commit\0"
    "cursor\0"
    "execute() argument 1 must be str, not int\0"
    "execute() argument 1 must be str, not flo\0"
    "execute() argument 1 must be str, not list\0"
    "execute() argument 1 must be str, not tup\0"
    "execute() argument 1 must be str, not dict\0"
    "execute() argument 1 must be str, not byte\0"
    "execute() argument 1 must be str, not bool\0"
    "execute() argument 1 must be str, not Null\0"
    "execute\0"
    "sql,params=()\0"
    "rollback\0"
    "PoolCursor\0"
    "executemany\0"
    "sql,params\0"
    "fetchall\0"
    "fetchmany\0"
    "size=1\0"
    "fetchone\0"
    "lastrowid\0"
    "rowcount\0"
    "PoolFile\0"
    "bytes\0"
    "copy() argument 1 must be str, not int\0"
    "copy() argument 1 must be str, not flo\0"
    "copy() argument 1 must be str, not list\0"
    "copy() argument 1 must be str, not tup\0"
    "copy() argument 1 must be str, not dict\0"
    "copy() argument 1 must be str, not byte\0"
    "copy() argument 1 must be str, not bool\0"
    "copy() argument 1 must be str, not Null\0"
    "copy\0"
    "destino\0"
    "ext\0"
    "move() argument 1 must be str, not int\0"
    "move() argument 1 must be str, not flo\0"
    "move() argument 1 must be str, not list\0"
    "move() argument 1 must be str, not tup\0"
    "move() argument 1 must be str, not dict\0"
    "move() argument 1 must be str, not byte\0"
    "move() argument 1 must be str, not bool\0"
    "move() argument 1 must be str, not Null\0"
    "move\0"
    "path\0"
    "read\0"
    "n=-1\0"
    "readline\0"
    "readlines\0"
    "save() argument 1 must be str, not int\0"
    "save() argument 1 must be str, not flo\0"
    "save() argument 1 must be str, not list\0"
    "save() argument 1 must be str, not tup\0"
    "save() argument 1 must be str, not dict\0"
    "save() argument 1 must be str, not byte\0"
    "save() argument 1 must be str, not bool\0"
    "save\0"
    "caminho=Null\0"
    "size\0"
    "write\0"
    "conteudo\0"
    "writelines() argument 1 must be list or tup, not int\0"
    "writelines() argument 1 must be list or tup, not flo\0"
    "writelines() argument 1 must be list or tup, not str\0"
    "writelines() argument 1 must be list or tup, not dict\0"
    "writelines() argument 1 must be list or tup, not byte\0"
    "writelines() argument 1 must be list or tup, not bool\0"
    "writelines() argument 1 must be list or tup, not Null\0"
    "writelines\0"
    "linhas\0"
    "PoolQRCode\0"
    "add_data\0"
    "data\0"
    "clear\0"
    "make\0"
    "fit=Null\0"
    "make_image\0"
    "fill_color=Null,back_color=Null,name=Null\0"
    "Process\0"
    "kill\0"
    "sinal=15\0"
    "pid\0"
    "read() argument 1 must be int, not flo\0"
    "read() argument 1 must be int, not str\0"
    "read() argument 1 must be int, not list\0"
    "read() argument 1 must be int, not tup\0"
    "read() argument 1 must be int, not dict\0"
    "read() argument 1 must be int, not byte\0"
    "read() argument 1 must be int, not Null\0"
    "n=4096\0"
    "read_err() argument 1 must be int, not flo\0"
    "read_err() argument 1 must be int, not str\0"
    "read_err() argument 1 must be int, not list\0"
    "read_err() argument 1 must be int, not tup\0"
    "read_err() argument 1 must be int, not dict\0"
    "read_err() argument 1 must be int, not byte\0"
    "read_err() argument 1 must be int, not Null\0"
    "read_err\0"
    "resize\0"
    "colunas,linhas\0"
    "returncode\0"
    "wait\0"
    "write() argument 1 must be str, not int\0"
    "write() argument 1 must be str, not flo\0"
    "write() argument 1 must be str, not list\0"
    "write() argument 1 must be str, not tup\0"
    "write() argument 1 must be str, not dict\0"
    "write() argument 1 must be str, not byte\0"
    "write() argument 1 must be str, not bool\0"
    "write() argument 1 must be str, not Null\0"
    "texto\0"
    "QRImage\0"
    "width,height\0"
    "save() argument 1 must be str, not Null\0"
    "to_file\0"
    "QRPoolFile\0"
    "caminho\0"
    "bad operand type for abs(): 'str'\0"
    "bad operand type for abs(): 'list'\0"
    "bad operand type for abs(): 'tup'\0"
    "bad operand type for abs(): 'dict'\0"
    "bad operand type for abs(): 'byte'\0"
    "bad operand type for abs(): 'Null'\0"
    "builtins\0"
    "abs\0"
    "addEnd\0"
    "addStart\0"
    "assert\0"
    "cond,esperado=Null,mensagem=Null\0"
    "bin\0"
    "bool\0"
    "chr\0"
    "'int' object is not iterable\0"
    "'flo' object is not iterable\0"
    "'bool' object is not iterable\0"
    "'Null' object is not iterable\0"
    "dict\0"
    "enumerate\0"
    "filter\0"
    "flo() argument must be a string or a real number, not 'list'\0"
    "flo() argument must be a string or a real number, not 'tup'\0"
    "flo() argument must be a string or a real number, not 'dict'\0"
    "flo() argument must be a string or a real number, not 'Null'\0"
    "flo\0"
    "gather\0"
    "hex\0"
    "input\0"
    "int() argument must be a string, a byte-like object or a real number, not 'list'\0"
    "int() argument must be a string, a byte-like object or a real number, not 'tup'\0"
    "int() argument must be a string, a byte-like object or a real number, not 'dict'\0"
    "int() argument must be a string, a byte-like object or a real number, not 'Null'\0"
    "int\0"
    "object of type 'int' has no len()\0"
    "object of type 'flo' has no len()\0"
    "object of type 'bool' has no len()\0"
    "len\0"
    "list\0"
    "load() argument 1 must be str, not int\0"
    "load() argument 1 must be str, not flo\0"
    "load() argument 1 must be str, not list\0"
    "load() argument 1 must be str, not tup\0"
    "load() argument 1 must be str, not dict\0"
    "load() argument 1 must be str, not byte\0"
    "load() argument 1 must be str, not bool\0"
    "load\0"
    "long\0"
    "map\0"
    "max\0"
    "min\0"
    "oct\0"
    "open() argument 1 must be str, not int\0"
    "open() argument 1 must be str, not flo\0"
    "open() argument 1 must be str, not list\0"
    "open() argument 1 must be str, not tup\0"
    "open() argument 1 must be str, not dict\0"
    "open() argument 1 must be str, not byte\0"
    "open() argument 1 must be str, not bool\0"
    "open() argument 1 must be str, not Null\0"
    "open\0"
    "path,mode=Null,encoding=Null\0"
    "ord() expected string of length 1, but int found\0"
    "ord() expected string of length 1, but flo found\0"
    "ord() expected string of length 1, but list found\0"
    "ord() expected string of length 1, but tup found\0"
    "ord() expected string of length 1, but dict found\0"
    "ord() expected string of length 1, but byte found\0"
    "ord() expected string of length 1, but bool found\0"
    "ord() expected string of length 1, but Null found\0"
    "ord\0"
    "pow\0"
    "base,expo,mod=Null\0"
    "range\0"
    "removeEnd() argument 1 must be list, not int\0"
    "removeEnd() argument 1 must be list, not flo\0"
    "removeEnd() argument 1 must be list, not str\0"
    "removeEnd() argument 1 must be list, not tup\0"
    "removeEnd() argument 1 must be list, not dict\0"
    "removeEnd() argument 1 must be list, not byte\0"
    "removeEnd() argument 1 must be list, not bool\0"
    "removeEnd() argument 1 must be list, not Null\0"
    "removeEnd\0"
    "removeStart() argument 1 must be list, not int\0"
    "removeStart() argument 1 must be list, not flo\0"
    "removeStart() argument 1 must be list, not str\0"
    "removeStart() argument 1 must be list, not tup\0"
    "removeStart() argument 1 must be list, not dict\0"
    "removeStart() argument 1 must be list, not byte\0"
    "removeStart() argument 1 must be list, not bool\0"
    "removeStart() argument 1 must be list, not Null\0"
    "removeStart\0"
    "'int' object is not reversible\0"
    "'flo' object is not reversible\0"
    "'bool' object is not reversible\0"
    "'Null' object is not reversible\0"
    "reversed\0"
    "type str doesn't define __round__ method\0"
    "type list doesn't define __round__ method\0"
    "type tup doesn't define __round__ method\0"
    "type dict doesn't define __round__ method\0"
    "type byte doesn't define __round__ method\0"
    "type Null doesn't define __round__ method\0"
    "round\0"
    "sleep() argument must be int or flo, not str\0"
    "sleep() argument must be int or flo, not list\0"
    "sleep() argument must be int or flo, not tup\0"
    "sleep() argument must be int or flo, not dict\0"
    "sleep() argument must be int or flo, not byte\0"
    "sleep() argument must be int or flo, not Null\0"
    "sleep\0"
    "sorted\0"
    "str\0"
    "sum() argument 1 must be list or tup, not int\0"
    "sum() argument 1 must be list or tup, not flo\0"
    "sum() argument 1 must be list or tup, not str\0"
    "sum() argument 1 must be list or tup, not dict\0"
    "sum() argument 1 must be list or tup, not byte\0"
    "sum() argument 1 must be list or tup, not bool\0"
    "sum() argument 1 must be list or tup, not Null\0"
    "sum\0"
    "tup\0"
    "type\0"
    "zip\0"
    "byte\0"
    "capitalize\0"
    "center\0"
    "width,fillbyte=Null\0"
    "argument should be integer or byte-like object, not 'flo'\0"
    "argument should be integer or byte-like object, not 'str'\0"
    "argument should be integer or byte-like object, not 'list'\0"
    "argument should be integer or byte-like object, not 'tup'\0"
    "argument should be integer or byte-like object, not 'dict'\0"
    "argument should be integer or byte-like object, not 'Null'\0"
    "contains\0"
    "count\0"
    "sub,inicio=0,fim=Null\0"
    "decode() argument 'encoding' must be str, not int\0"
    "decode() argument 'encoding' must be str, not flo\0"
    "decode() argument 'encoding' must be str, not list\0"
    "decode() argument 'encoding' must be str, not tup\0"
    "decode() argument 'encoding' must be str, not dict\0"
    "decode() argument 'encoding' must be str, not byte\0"
    "decode() argument 'encoding' must be str, not bool\0"
    "decode\0"
    "encoding=\"utf-8\",errors=\"strict\"\0"
    "endswith first arg must be byte or a tuple of byte, not int\0"
    "endswith first arg must be byte or a tuple of byte, not flo\0"
    "endswith first arg must be byte or a tuple of byte, not str\0"
    "endswith first arg must be byte or a tuple of byte, not dict\0"
    "endswith first arg must be byte or a tuple of byte, not bool\0"
    "endswith first arg must be byte or a tuple of byte, not Null\0"
    "endswith\0"
    "sufixo,inicio=0,fim=Null\0"
    "expandtabs\0"
    "tabsize=8\0"
    "find\0"
    "has\0"
    "sep must be str or bytes, not int\0"
    "sep must be str or bytes, not flo\0"
    "sep must be str or bytes, not list\0"
    "sep must be str or bytes, not tup\0"
    "sep must be str or bytes, not dict\0"
    "sep must be str or bytes, not bool\0"
    "sep=Null,bytes_per_sep=1\0"
    "index\0"
    "isalnum\0"
    "isalpha\0"
    "isascii\0"
    "isdigit\0"
    "islower\0"
    "isspace\0"
    "istitle\0"
    "isupper\0"
    "join() argument 1 must be list, tup, str or bytes, not int\0"
    "join() argument 1 must be list, tup, str or bytes, not flo\0"
    "join() argument 1 must be list, tup, str or bytes, not dict\0"
    "join() argument 1 must be list, tup, str or bytes, not bool\0"
    "join() argument 1 must be list, tup, str or bytes, not Null\0"
    "join\0"
    "lista\0"
    "ljust\0"
    "lower\0"
    "a byte-like object is required, not 'int'\0"
    "a byte-like object is required, not 'flo'\0"
    "a byte-like object is required, not 'str'\0"
    "a byte-like object is required, not 'list'\0"
    "a byte-like object is required, not 'tup'\0"
    "a byte-like object is required, not 'dict'\0"
    "a byte-like object is required, not 'bool'\0"
    "lstrip\0"
    "chars=Null\0"
    "maketrans\0"
    "de,para\0"
    "a byte-like object is required, not 'Null'\0"
    "partition\0"
    "sep\0"
    "removeprefix\0"
    "p\0"
    "removesuffix\0"
    "replace\0"
    "old,new,count=-1\0"
    "rfind\0"
    "rindex\0"
    "rjust\0"
    "rpartition\0"
    "rsplit\0"
    "sep=Null,maxsplit=-1\0"
    "rstrip\0"
    "splitlines\0"
    "keepends=False\0"
    "startswith first arg must be byte or a tuple of byte, not int\0"
    "startswith first arg must be byte or a tuple of byte, not flo\0"
    "startswith first arg must be byte or a tuple of byte, not str\0"
    "startswith first arg must be byte or a tuple of byte, not dict\0"
    "startswith first arg must be byte or a tuple of byte, not bool\0"
    "startswith first arg must be byte or a tuple of byte, not Null\0"
    "startswith\0"
    "prefixo,inicio=0,fim=Null\0"
    "strip\0"
    "swapcase\0"
    "title\0"
    "translate\0"
    "tabela,delete=Null\0"
    "upper\0"
    "zfill\0"
    "largura\0"
    "base64() argument 1 must be byte, not int\0"
    "base64() argument 1 must be byte, not flo\0"
    "base64() argument 1 must be byte, not str\0"
    "base64() argument 1 must be byte, not list\0"
    "base64() argument 1 must be byte, not tup\0"
    "base64() argument 1 must be byte, not dict\0"
    "base64() argument 1 must be byte, not bool\0"
    "base64() argument 1 must be byte, not Null\0"
    "base64\0"
    "b\0"
    "concat() argument 1 must be list or tup, not int\0"
    "concat() argument 1 must be list or tup, not flo\0"
    "concat() argument 1 must be list or tup, not str\0"
    "concat() argument 1 must be list or tup, not dict\0"
    "concat() argument 1 must be list or tup, not byte\0"
    "concat() argument 1 must be list or tup, not bool\0"
    "concat() argument 1 must be list or tup, not Null\0"
    "concat\0"
    "frombase64() argument 1 must be str, not int\0"
    "frombase64() argument 1 must be str, not flo\0"
    "frombase64() argument 1 must be str, not list\0"
    "frombase64() argument 1 must be str, not tup\0"
    "frombase64() argument 1 must be str, not dict\0"
    "frombase64() argument 1 must be str, not byte\0"
    "frombase64() argument 1 must be str, not bool\0"
    "frombase64() argument 1 must be str, not Null\0"
    "frombase64\0"
    "s\0"
    "fromhex() argument must be str, not int\0"
    "fromhex() argument must be str, not flo\0"
    "fromhex() argument must be str, not list\0"
    "fromhex() argument must be str, not tup\0"
    "fromhex() argument must be str, not dict\0"
    "fromhex() argument must be str, not byte\0"
    "fromhex() argument must be str, not bool\0"
    "fromhex() argument must be str, not Null\0"
    "fromhex\0"
    "fromint\0"
    "n,length=Null,byteorder=Null\0"
    "b,i\0"
    "hex() argument 1 must be byte, not int\0"
    "hex() argument 1 must be byte, not flo\0"
    "hex() argument 1 must be byte, not str\0"
    "hex() argument 1 must be byte, not list\0"
    "hex() argument 1 must be byte, not tup\0"
    "hex() argument 1 must be byte, not dict\0"
    "hex() argument 1 must be byte, not bool\0"
    "hex() argument 1 must be byte, not Null\0"
    "cannot convert 'flo' object to bytes\0"
    "cannot convert 'dict' object to bytes\0"
    "cannot convert 'bool' object to bytes\0"
    "cannot convert 'Null' object to bytes\0"
    "new\0"
    "x=Null\0"
    "slice() argument 1 must be byte, not int\0"
    "slice() argument 1 must be byte, not flo\0"
    "slice() argument 1 must be byte, not str\0"
    "slice() argument 1 must be byte, not list\0"
    "slice() argument 1 must be byte, not tup\0"
    "slice() argument 1 must be byte, not dict\0"
    "slice() argument 1 must be byte, not bool\0"
    "slice() argument 1 must be byte, not Null\0"
    "slice\0"
    "b,ini=Null,fim=Null\0"
    "toint() argument 1 must be byte, not int\0"
    "toint() argument 1 must be byte, not flo\0"
    "toint() argument 1 must be byte, not str\0"
    "toint() argument 1 must be byte, not list\0"
    "toint() argument 1 must be byte, not tup\0"
    "toint() argument 1 must be byte, not dict\0"
    "toint() argument 1 must be byte, not bool\0"
    "toint() argument 1 must be byte, not Null\0"
    "toint\0"
    "b,byteorder=Null\0"
    "tolist() argument 1 must be byte, not int\0"
    "tolist() argument 1 must be byte, not flo\0"
    "tolist() argument 1 must be byte, not str\0"
    "tolist() argument 1 must be byte, not list\0"
    "tolist() argument 1 must be byte, not tup\0"
    "tolist() argument 1 must be byte, not dict\0"
    "tolist() argument 1 must be byte, not bool\0"
    "tolist() argument 1 must be byte, not Null\0"
    "tolist\0"
    "xor\0"
    "dados,chave\0"
    "asdict() argument 1 must be Entity, not int\0"
    "asdict() argument 1 must be Entity, not flo\0"
    "asdict() argument 1 must be Entity, not str\0"
    "asdict() argument 1 must be Entity, not list\0"
    "asdict() argument 1 must be Entity, not tup\0"
    "asdict() argument 1 must be Entity, not dict\0"
    "asdict() argument 1 must be Entity, not byte\0"
    "asdict() argument 1 must be Entity, not bool\0"
    "asdict() argument 1 must be Entity, not Null\0"
    "datasentity\0"
    "asdict\0"
    "asjson() argument 1 must be Entity, not int\0"
    "asjson() argument 1 must be Entity, not flo\0"
    "asjson() argument 1 must be Entity, not str\0"
    "asjson() argument 1 must be Entity, not list\0"
    "asjson() argument 1 must be Entity, not tup\0"
    "asjson() argument 1 must be Entity, not dict\0"
    "asjson() argument 1 must be Entity, not byte\0"
    "asjson() argument 1 must be Entity, not bool\0"
    "asjson() argument 1 must be Entity, not Null\0"
    "asjson\0"
    "aslist() argument 1 must be Entity, not int\0"
    "aslist() argument 1 must be Entity, not flo\0"
    "aslist() argument 1 must be Entity, not str\0"
    "aslist() argument 1 must be Entity, not list\0"
    "aslist() argument 1 must be Entity, not tup\0"
    "aslist() argument 1 must be Entity, not dict\0"
    "aslist() argument 1 must be Entity, not byte\0"
    "aslist() argument 1 must be Entity, not bool\0"
    "aslist() argument 1 must be Entity, not Null\0"
    "aslist\0"
    "astuple() argument 1 must be Entity, not int\0"
    "astuple() argument 1 must be Entity, not flo\0"
    "astuple() argument 1 must be Entity, not str\0"
    "astuple() argument 1 must be Entity, not list\0"
    "astuple() argument 1 must be Entity, not tup\0"
    "astuple() argument 1 must be Entity, not dict\0"
    "astuple() argument 1 must be Entity, not byte\0"
    "astuple() argument 1 must be Entity, not bool\0"
    "astuple() argument 1 must be Entity, not Null\0"
    "astuple\0"
    "dataentity\0"
    "date\0"
    "datahora\0"
    "hora\0"
    "hours=Null,minutes=Null,days=Null\0"
    "monotonic\0"
    "now\0"
    "time\0"
    "timestamp\0"
    "timestamp_ms\0"
    "today\0"
    "chave\0"
    "chave,default=Null\0"
    "items\0"
    "keys\0"
    "pop\0"
    "update() argument 1 must be dict, not int\0"
    "update() argument 1 must be dict, not flo\0"
    "update() argument 1 must be dict, not str\0"
    "update() argument 1 must be dict, not list\0"
    "update() argument 1 must be dict, not tup\0"
    "update() argument 1 must be dict, not byte\0"
    "update() argument 1 must be dict, not bool\0"
    "update() argument 1 must be dict, not Null\0"
    "update\0"
    "outro\0"
    "value\0"
    "values\0"
    "dotenv\0"
    "path=Null\0"
    "b64decode() argument 1 must be str, not int\0"
    "b64decode() argument 1 must be str, not flo\0"
    "b64decode() argument 1 must be str, not list\0"
    "b64decode() argument 1 must be str, not tup\0"
    "b64decode() argument 1 must be str, not dict\0"
    "b64decode() argument 1 must be str, not byte\0"
    "b64decode() argument 1 must be str, not bool\0"
    "b64decode() argument 1 must be str, not Null\0"
    "hash\0"
    "b64decode\0"
    "b64encode\0"
    "dado\0"
    "check\0"
    "senha_hash,senha_digitada\0"
    "crypt\0"
    "senha\0"
    "sha256\0"
    "jinker\0"
    "name=Null,oauth=Null,static_folder=Null,static_url=Null,route_prefix=Null\0"
    "JinkerRequest\0"
    "jsonify\0"
    "render() argument 1 must be str, not int\0"
    "render() argument 1 must be str, not flo\0"
    "render() argument 1 must be str, not list\0"
    "render() argument 1 must be str, not tup\0"
    "render() argument 1 must be str, not dict\0"
    "render() argument 1 must be str, not byte\0"
    "render() argument 1 must be str, not bool\0"
    "render() argument 1 must be str, not Null\0"
    "render\0"
    "folder_or_file,file=Null\0"
    "parse() argument 1 must be str, not int\0"
    "parse() argument 1 must be str, not flo\0"
    "parse() argument 1 must be str, not byte\0"
    "parse() argument 1 must be str, not bool\0"
    "parse() argument 1 must be str, not Null\0"
    "parse\0"
    "text\0"
    "stringify\0"
    "jwt\0"
    "token,secret\0"
    "gen\0"
    "payload,secret,algorithm=Null\0"
    "append\0"
    "item\0"
    "extend() argument 1 must be list or tup, not int\0"
    "extend() argument 1 must be list or tup, not flo\0"
    "extend() argument 1 must be list or tup, not str\0"
    "extend() argument 1 must be list or tup, not dict\0"
    "extend() argument 1 must be list or tup, not byte\0"
    "extend() argument 1 must be list or tup, not bool\0"
    "extend() argument 1 must be list or tup, not Null\0"
    "extend\0"
    "outra\0"
    "item,inicio=0,fim=len\0"
    "insert\0"
    "i,item\0"
    "i=-1\0"
    "remove\0"
    "reverse\0"
    "sort\0"
    "mail\0"
    "load() argument 1 must be str, not Null\0"
    "manpu\0"
    "filepath\0"
    "target,encoding=Null\0"
    "read() argument 1 must be str, not int\0"
    "read() argument 1 must be str, not flo\0"
    "read() argument 1 must be str, not list\0"
    "read() argument 1 must be str, not tup\0"
    "read() argument 1 must be str, not dict\0"
    "read() argument 1 must be str, not byte\0"
    "read() argument 1 must be str, not bool\0"
    "read() argument 1 must be str, not Null\0"
    "value=Null,content=Null,column=Null,celula=Null,amount=Null,target=Null\0"
    "src() argument 1 must be str, not int\0"
    "src() argument 1 must be str, not flo\0"
    "src() argument 1 must be str, not list\0"
    "src() argument 1 must be str, not tup\0"
    "src() argument 1 must be str, not dict\0"
    "src() argument 1 must be str, not byte\0"
    "src() argument 1 must be str, not bool\0"
    "src() argument 1 must be str, not Null\0"
    "src\0"
    "content=Null,column=Null,celula=Null,target=Null\0"
    "chdir() argument 1 must be str, not int\0"
    "chdir() argument 1 must be str, not flo\0"
    "chdir() argument 1 must be str, not list\0"
    "chdir() argument 1 must be str, not tup\0"
    "chdir() argument 1 must be str, not dict\0"
    "chdir() argument 1 must be str, not byte\0"
    "chdir() argument 1 must be str, not bool\0"
    "chdir() argument 1 must be str, not Null\0"
    "os\0"
    "chdir\0"
    "cmd() argument 1 must be str, not int\0"
    "cmd() argument 1 must be str, not flo\0"
    "cmd() argument 1 must be str, not list\0"
    "cmd() argument 1 must be str, not tup\0"
    "cmd() argument 1 must be str, not dict\0"
    "cmd() argument 1 must be str, not byte\0"
    "cmd() argument 1 must be str, not bool\0"
    "cmd() argument 1 must be str, not Null\0"
    "cmd\0"
    "command,capture=False\0"
    "code() argument 1 must be str, not int\0"
    "code() argument 1 must be str, not flo\0"
    "code() argument 1 must be str, not list\0"
    "code() argument 1 must be str, not tup\0"
    "code() argument 1 must be str, not dict\0"
    "code() argument 1 must be str, not byte\0"
    "code() argument 1 must be str, not bool\0"
    "code() argument 1 must be str, not Null\0"
    "src,dst\0"
    "cwd\0"
    "environ() argument 1 must be str, not int\0"
    "environ() argument 1 must be str, not flo\0"
    "environ() argument 1 must be str, not list\0"
    "environ() argument 1 must be str, not tup\0"
    "environ() argument 1 must be str, not dict\0"
    "environ() argument 1 must be str, not byte\0"
    "environ() argument 1 must be str, not bool\0"
    "environ\0"
    "key=Null\0"
    "exists() argument 1 must be str, not int\0"
    "exists() argument 1 must be str, not flo\0"
    "exists() argument 1 must be str, not list\0"
    "exists() argument 1 must be str, not tup\0"
    "exists() argument 1 must be str, not dict\0"
    "exists() argument 1 must be str, not byte\0"
    "exists() argument 1 must be str, not bool\0"
    "exists() argument 1 must be str, not Null\0"
    "exists\0"
    "getenv() argument 1 must be str, not int\0"
    "getenv() argument 1 must be str, not flo\0"
    "getenv() argument 1 must be str, not list\0"
    "getenv() argument 1 must be str, not tup\0"
    "getenv() argument 1 must be str, not dict\0"
    "getenv() argument 1 must be str, not byte\0"
    "getenv() argument 1 must be str, not bool\0"
    "getenv() argument 1 must be str, not Null\0"
    "getenv\0"
    "key,default=Null\0"
    "ipmach\0"
    "isdir() argument 1 must be str, not int\0"
    "isdir() argument 1 must be str, not flo\0"
    "isdir() argument 1 must be str, not list\0"
    "isdir() argument 1 must be str, not tup\0"
    "isdir() argument 1 must be str, not dict\0"
    "isdir() argument 1 must be str, not byte\0"
    "isdir() argument 1 must be str, not bool\0"
    "isdir() argument 1 must be str, not Null\0"
    "isdir\0"
    "isfile() argument 1 must be str, not int\0"
    "isfile() argument 1 must be str, not flo\0"
    "isfile() argument 1 must be str, not list\0"
    "isfile() argument 1 must be str, not tup\0"
    "isfile() argument 1 must be str, not dict\0"
    "isfile() argument 1 must be str, not byte\0"
    "isfile() argument 1 must be str, not bool\0"
    "isfile() argument 1 must be str, not Null\0"
    "isfile\0"
    "loadFile() argument 1 must be str, not int\0"
    "loadFile() argument 1 must be str, not flo\0"
    "loadFile() argument 1 must be str, not list\0"
    "loadFile() argument 1 must be str, not tup\0"
    "loadFile() argument 1 must be str, not dict\0"
    "loadFile() argument 1 must be str, not byte\0"
    "loadFile() argument 1 must be str, not bool\0"
    "loadFile() argument 1 must be str, not Null\0"
    "loadFile\0"
    "path,mode=Null\0"
    "ls() argument 1 must be str, not int\0"
    "ls() argument 1 must be str, not flo\0"
    "ls() argument 1 must be str, not list\0"
    "ls() argument 1 must be str, not tup\0"
    "ls() argument 1 must be str, not dict\0"
    "ls() argument 1 must be str, not byte\0"
    "ls() argument 1 must be str, not bool\0"
    "ls() argument 1 must be str, not Null\0"
    "ls\0"
    "mkdir() argument 1 must be str, not int\0"
    "mkdir() argument 1 must be str, not flo\0"
    "mkdir() argument 1 must be str, not list\0"
    "mkdir() argument 1 must be str, not tup\0"
    "mkdir() argument 1 must be str, not dict\0"
    "mkdir() argument 1 must be str, not byte\0"
    "mkdir() argument 1 must be str, not bool\0"
    "mkdir() argument 1 must be str, not Null\0"
    "mkdir\0"
    "path,exist_ok=Null\0"
    "pathFile() argument 1 must be str, not int\0"
    "pathFile() argument 1 must be str, not flo\0"
    "pathFile() argument 1 must be str, not list\0"
    "pathFile() argument 1 must be str, not tup\0"
    "pathFile() argument 1 must be str, not dict\0"
    "pathFile() argument 1 must be str, not byte\0"
    "pathFile() argument 1 must be str, not bool\0"
    "pathFile() argument 1 must be str, not Null\0"
    "pathFile\0"
    "pathFolder() argument 1 must be str, not int\0"
    "pathFolder() argument 1 must be str, not flo\0"
    "pathFolder() argument 1 must be str, not list\0"
    "pathFolder() argument 1 must be str, not tup\0"
    "pathFolder() argument 1 must be str, not dict\0"
    "pathFolder() argument 1 must be str, not byte\0"
    "pathFolder() argument 1 must be str, not bool\0"
    "pathFolder() argument 1 must be str, not Null\0"
    "pathFolder\0"
    "readFile() argument 1 must be str, not int\0"
    "readFile() argument 1 must be str, not flo\0"
    "readFile() argument 1 must be str, not list\0"
    "readFile() argument 1 must be str, not tup\0"
    "readFile() argument 1 must be str, not dict\0"
    "readFile() argument 1 must be str, not byte\0"
    "readFile() argument 1 must be str, not bool\0"
    "readFile() argument 1 must be str, not Null\0"
    "readFile\0"
    "path,encoding=Null\0"
    "rename\0"
    "rmdir() argument 1 must be str, not int\0"
    "rmdir() argument 1 must be str, not flo\0"
    "rmdir() argument 1 must be str, not list\0"
    "rmdir() argument 1 must be str, not tup\0"
    "rmdir() argument 1 must be str, not dict\0"
    "rmdir() argument 1 must be str, not byte\0"
    "rmdir() argument 1 must be str, not bool\0"
    "rmdir() argument 1 must be str, not Null\0"
    "rmdir\0"
    "path,force=Null\0"
    "run() argument 1 must be list or str, not int\0"
    "run() argument 1 must be list or str, not flo\0"
    "run() argument 1 must be list or str, not dict\0"
    "run() argument 1 must be list or str, not byte\0"
    "run() argument 1 must be list or str, not bool\0"
    "run() argument 1 must be list or str, not Null\0"
    "run\0"
    "args,capture=False,pty=False\0"
    "size() argument 1 must be str, not int\0"
    "size() argument 1 must be str, not flo\0"
    "size() argument 1 must be str, not list\0"
    "size() argument 1 must be str, not tup\0"
    "size() argument 1 must be str, not dict\0"
    "size() argument 1 must be str, not byte\0"
    "size() argument 1 must be str, not bool\0"
    "size() argument 1 must be str, not Null\0"
    "warn\0"
    "text=Null,color=Null\0"
    "writeFile\0"
    "path,content,encoding=Null\0"
    "psodbc\0"
    "connect\0"
    "driver=Null,host=Null,port=Null,user=Null,password=Null,database=Null,base=Null,url=Null\0"
    "query\0"
    "base=Null,cmd=Null,table=Null\0"
    "qrcode\0"
    "QRCode\0"
    "version=Null,error_correction=Null,box_size=Null,border=Null\0"
    "data,save=Null,size=Null,border=Null,color=Null,bg=Null,name=Null,error_correction=Null,qr32=Null\0"
    "data,error_correction=Null,box_size=Null,border=Null,fill_color=Null,back_color=Null,name=Null\0"
    "compile() argument 1 must be str, not int\0"
    "compile() argument 1 must be str, not flo\0"
    "compile() argument 1 must be str, not list\0"
    "compile() argument 1 must be str, not tup\0"
    "compile() argument 1 must be str, not dict\0"
    "compile() argument 1 must be str, not byte\0"
    "compile() argument 1 must be str, not bool\0"
    "compile() argument 1 must be str, not Null\0"
    "regex\0"
    "compile\0"
    "escape() argument 1 must be str, not int\0"
    "escape() argument 1 must be str, not flo\0"
    "escape() argument 1 must be str, not list\0"
    "escape() argument 1 must be str, not tup\0"
    "escape() argument 1 must be str, not dict\0"
    "escape() argument 1 must be str, not byte\0"
    "escape() argument 1 must be str, not bool\0"
    "escape() argument 1 must be str, not Null\0"
    "escape\0"
    "pattern,string\0"
    "pattern,string,maxsplit=Null\0"
    "pattern,repl,string,count=Null\0"
    "request\0"
    "url,headers=Null,body=Null,timeout=Null,stream=Null,max_size=Null,fields=Null,file=Null,save=Null\0"
    "head() argument 1 must be str, not int\0"
    "head() argument 1 must be str, not flo\0"
    "head() argument 1 must be str, not list\0"
    "head() argument 1 must be str, not tup\0"
    "head() argument 1 must be str, not dict\0"
    "head() argument 1 must be str, not byte\0"
    "head() argument 1 must be str, not bool\0"
    "head() argument 1 must be str, not Null\0"
    "head\0"
    "url,headers=Null,timeout=Null\0"
    "ws_connect() argument 1 must be str, not int\0"
    "ws_connect() argument 1 must be str, not flo\0"
    "ws_connect() argument 1 must be str, not list\0"
    "ws_connect() argument 1 must be str, not tup\0"
    "ws_connect() argument 1 must be str, not dict\0"
    "ws_connect() argument 1 must be str, not byte\0"
    "ws_connect() argument 1 must be str, not bool\0"
    "ws_connect() argument 1 must be str, not Null\0"
    "ws_connect\0"
    "url\0"
    "accept\0"
    "bind(): AF_INET address must be tuple, not int\0"
    "bind(): AF_INET address must be tuple, not flo\0"
    "bind(): AF_INET address must be tuple, not str\0"
    "bind(): AF_INET address must be tuple, not dict\0"
    "bind(): AF_INET address must be tuple, not byte\0"
    "bind(): AF_INET address must be tuple, not bool\0"
    "bind(): AF_INET address must be tuple, not Null\0"
    "bind\0"
    "address\0"
    "connect(): AF_INET address must be tuple, not int\0"
    "connect(): AF_INET address must be tuple, not flo\0"
    "connect(): AF_INET address must be tuple, not str\0"
    "connect(): AF_INET address must be tuple, not dict\0"
    "connect(): AF_INET address must be tuple, not byte\0"
    "connect(): AF_INET address must be tuple, not bool\0"
    "connect(): AF_INET address must be tuple, not Null\0"
    "connect_ex(): AF_INET address must be tuple, not int\0"
    "connect_ex(): AF_INET address must be tuple, not flo\0"
    "connect_ex(): AF_INET address must be tuple, not str\0"
    "connect_ex(): AF_INET address must be tuple, not dict\0"
    "connect_ex(): AF_INET address must be tuple, not byte\0"
    "connect_ex(): AF_INET address must be tuple, not bool\0"
    "connect_ex(): AF_INET address must be tuple, not Null\0"
    "connect_ex\0"
    "detach\0"
    "dup\0"
    "family\0"
    "fileno\0"
    "getblocking\0"
    "getpeername\0"
    "getsockname\0"
    "getsockopt\0"
    "level,optname,buflen=Null\0"
    "gettimeout\0"
    "listen\0"
    "backlog=Null\0"
    "proto\0"
    "recv\0"
    "bufsize,flags=Null\0"
    "recvfrom\0"
    "data,flags=Null\0"
    "sendall\0"
    "sendto\0"
    "data,address,flags=Null\0"
    "setblocking\0"
    "flag\0"
    "setsockopt\0"
    "level,optname,value\0"
    "settimeout\0"
    "timeout\0"
    "shutdown\0"
    "how\0"
    "create_connection(): AF_INET address must be tuple, not int\0"
    "create_connection(): AF_INET address must be tuple, not flo\0"
    "create_connection(): AF_INET address must be tuple, not str\0"
    "create_connection(): AF_INET address must be tuple, not dict\0"
    "create_connection(): AF_INET address must be tuple, not byte\0"
    "create_connection(): AF_INET address must be tuple, not bool\0"
    "create_connection(): AF_INET address must be tuple, not Null\0"
    "sockets\0"
    "create_connection\0"
    "address,timeout=Null,source_address=Null\0"
    "create_server(): AF_INET address must be tuple, not int\0"
    "create_server(): AF_INET address must be tuple, not flo\0"
    "create_server(): AF_INET address must be tuple, not str\0"
    "create_server(): AF_INET address must be tuple, not dict\0"
    "create_server(): AF_INET address must be tuple, not byte\0"
    "create_server(): AF_INET address must be tuple, not bool\0"
    "create_server(): AF_INET address must be tuple, not Null\0"
    "create_server\0"
    "address,backlog=Null,reuse_port=Null\0"
    "getaddrinfo\0"
    "host,port,family=Null,type=Null,proto=Null,flags=Null\0"
    "getdefaulttimeout\0"
    "getfqdn\0"
    "name=Null\0"
    "gethostbyaddr() argument 1 must be str, not int\0"
    "gethostbyaddr() argument 1 must be str, not flo\0"
    "gethostbyaddr() argument 1 must be str, not list\0"
    "gethostbyaddr() argument 1 must be str, not tup\0"
    "gethostbyaddr() argument 1 must be str, not dict\0"
    "gethostbyaddr() argument 1 must be str, not byte\0"
    "gethostbyaddr() argument 1 must be str, not bool\0"
    "gethostbyaddr() argument 1 must be str, not Null\0"
    "gethostbyaddr\0"
    "gethostbyname() argument 1 must be str, not int\0"
    "gethostbyname() argument 1 must be str, not flo\0"
    "gethostbyname() argument 1 must be str, not list\0"
    "gethostbyname() argument 1 must be str, not tup\0"
    "gethostbyname() argument 1 must be str, not dict\0"
    "gethostbyname() argument 1 must be str, not byte\0"
    "gethostbyname() argument 1 must be str, not bool\0"
    "gethostbyname() argument 1 must be str, not Null\0"
    "gethostbyname\0"
    "hostname\0"
    "gethostbyname_ex() argument 1 must be str, not int\0"
    "gethostbyname_ex() argument 1 must be str, not flo\0"
    "gethostbyname_ex() argument 1 must be str, not list\0"
    "gethostbyname_ex() argument 1 must be str, not tup\0"
    "gethostbyname_ex() argument 1 must be str, not dict\0"
    "gethostbyname_ex() argument 1 must be str, not byte\0"
    "gethostbyname_ex() argument 1 must be str, not bool\0"
    "gethostbyname_ex() argument 1 must be str, not Null\0"
    "gethostbyname_ex\0"
    "gethostname\0"
    "getnameinfo\0"
    "sockaddr,flags=Null\0"
    "getprotobyname() argument 1 must be str, not int\0"
    "getprotobyname() argument 1 must be str, not flo\0"
    "getprotobyname() argument 1 must be str, not list\0"
    "getprotobyname() argument 1 must be str, not tup\0"
    "getprotobyname() argument 1 must be str, not dict\0"
    "getprotobyname() argument 1 must be str, not byte\0"
    "getprotobyname() argument 1 must be str, not bool\0"
    "getprotobyname() argument 1 must be str, not Null\0"
    "getprotobyname\0"
    "protocolname\0"
    "getservbyname() argument 1 must be str, not int\0"
    "getservbyname() argument 1 must be str, not flo\0"
    "getservbyname() argument 1 must be str, not list\0"
    "getservbyname() argument 1 must be str, not tup\0"
    "getservbyname() argument 1 must be str, not dict\0"
    "getservbyname() argument 1 must be str, not byte\0"
    "getservbyname() argument 1 must be str, not bool\0"
    "getservbyname() argument 1 must be str, not Null\0"
    "getservbyname\0"
    "servicename,protocolname=Null\0"
    "getservbyport\0"
    "port,protocolname=Null\0"
    "has_dualstack_ipv6\0"
    "htonl\0"
    "x\0"
    "htons\0"
    "if_indextoname\0"
    "if_index\0"
    "if_nameindex\0"
    "if_nametoindex() argument 1 must be str, not int\0"
    "if_nametoindex() argument 1 must be str, not flo\0"
    "if_nametoindex() argument 1 must be str, not list\0"
    "if_nametoindex() argument 1 must be str, not tup\0"
    "if_nametoindex() argument 1 must be str, not dict\0"
    "if_nametoindex() argument 1 must be str, not byte\0"
    "if_nametoindex() argument 1 must be str, not bool\0"
    "if_nametoindex() argument 1 must be str, not Null\0"
    "if_nametoindex\0"
    "if_name\0"
    "inet_aton() argument 1 must be str, not int\0"
    "inet_aton() argument 1 must be str, not flo\0"
    "inet_aton() argument 1 must be str, not list\0"
    "inet_aton() argument 1 must be str, not tup\0"
    "inet_aton() argument 1 must be str, not dict\0"
    "inet_aton() argument 1 must be str, not byte\0"
    "inet_aton() argument 1 must be str, not bool\0"
    "inet_aton() argument 1 must be str, not Null\0"
    "inet_aton\0"
    "ip_string\0"
    "inet_ntoa\0"
    "packed_ip\0"
    "inet_ntop\0"
    "address_family,packed_ip\0"
    "inet_pton\0"
    "address_family,ip_string\0"
    "ntohl\0"
    "ntohs\0"
    "setdefaulttimeout\0"
    "family=Null,type=Null,proto=Null\0"
    "socketpair\0"
    "connect() argument 1 must be str, not int\0"
    "connect() argument 1 must be str, not flo\0"
    "connect() argument 1 must be str, not list\0"
    "connect() argument 1 must be str, not tup\0"
    "connect() argument 1 must be str, not dict\0"
    "connect() argument 1 must be str, not byte\0"
    "connect() argument 1 must be str, not bool\0"
    "connect() argument 1 must be str, not Null\0"
    "sqlite3\0"
    "database\0"
    "casefold\0"
    "width,fillchar=Null\0"
    "contains() argument 1 must be str, not int\0"
    "contains() argument 1 must be str, not flo\0"
    "contains() argument 1 must be str, not list\0"
    "contains() argument 1 must be str, not tup\0"
    "contains() argument 1 must be str, not dict\0"
    "contains() argument 1 must be str, not byte\0"
    "contains() argument 1 must be str, not bool\0"
    "contains() argument 1 must be str, not Null\0"
    "count() argument 1 must be str, not int\0"
    "count() argument 1 must be str, not flo\0"
    "count() argument 1 must be str, not list\0"
    "count() argument 1 must be str, not tup\0"
    "count() argument 1 must be str, not dict\0"
    "count() argument 1 must be str, not byte\0"
    "count() argument 1 must be str, not bool\0"
    "count() argument 1 must be str, not Null\0"
    "encode() argument 'encoding' must be str, not int\0"
    "encode() argument 'encoding' must be str, not flo\0"
    "encode() argument 'encoding' must be str, not list\0"
    "encode() argument 'encoding' must be str, not tup\0"
    "encode() argument 'encoding' must be str, not dict\0"
    "encode() argument 'encoding' must be str, not byte\0"
    "encode() argument 'encoding' must be str, not bool\0"
    "encode\0"
    "endswith() argument 1 must be str, not int\0"
    "endswith() argument 1 must be str, not flo\0"
    "endswith() argument 1 must be str, not dict\0"
    "endswith() argument 1 must be str, not byte\0"
    "endswith() argument 1 must be str, not bool\0"
    "endswith() argument 1 must be str, not Null\0"
    "sufixo\0"
    "find() argument 1 must be str, not int\0"
    "find() argument 1 must be str, not flo\0"
    "find() argument 1 must be str, not list\0"
    "find() argument 1 must be str, not tup\0"
    "find() argument 1 must be str, not dict\0"
    "find() argument 1 must be str, not byte\0"
    "find() argument 1 must be str, not bool\0"
    "find() argument 1 must be str, not Null\0"
    "padrao\0"
    "format\0"
    "*valores\0"
    "format_map() argument 1 must be dict, not int\0"
    "format_map() argument 1 must be dict, not flo\0"
    "format_map() argument 1 must be dict, not str\0"
    "format_map() argument 1 must be dict, not list\0"
    "format_map() argument 1 must be dict, not tup\0"
    "format_map() argument 1 must be dict, not byte\0"
    "format_map() argument 1 must be dict, not bool\0"
    "format_map() argument 1 must be dict, not Null\0"
    "format_map\0"
    "chave=Null\0"
    "get_json\0"
    "index() argument 1 must be str, not int\0"
    "index() argument 1 must be str, not flo\0"
    "index() argument 1 must be str, not list\0"
    "index() argument 1 must be str, not tup\0"
    "index() argument 1 must be str, not dict\0"
    "index() argument 1 must be str, not byte\0"
    "index() argument 1 must be str, not bool\0"
    "index() argument 1 must be str, not Null\0"
    "isdecimal\0"
    "isidentifier\0"
    "isnumeric\0"
    "isprintable\0"
    "join() argument 1 must be list or tup, not int\0"
    "join() argument 1 must be list or tup, not flo\0"
    "join() argument 1 must be list or tup, not str\0"
    "join() argument 1 must be list or tup, not dict\0"
    "join() argument 1 must be list or tup, not byte\0"
    "join() argument 1 must be list or tup, not bool\0"
    "join() argument 1 must be list or tup, not Null\0"
    "lstrip() argument 1 must be str, not int\0"
    "lstrip() argument 1 must be str, not flo\0"
    "lstrip() argument 1 must be str, not list\0"
    "lstrip() argument 1 must be str, not tup\0"
    "lstrip() argument 1 must be str, not dict\0"
    "lstrip() argument 1 must be str, not byte\0"
    "lstrip() argument 1 must be str, not bool\0"
    "must be str, not int\0"
    "must be str, not flo\0"
    "must be str, not list\0"
    "must be str, not tup\0"
    "must be str, not dict\0"
    "must be str, not byte\0"
    "must be str, not bool\0"
    "must be str, not Null\0"
    "removeprefix() argument 1 must be str, not int\0"
    "removeprefix() argument 1 must be str, not flo\0"
    "removeprefix() argument 1 must be str, not list\0"
    "removeprefix() argument 1 must be str, not tup\0"
    "removeprefix() argument 1 must be str, not dict\0"
    "removeprefix() argument 1 must be str, not byte\0"
    "removeprefix() argument 1 must be str, not bool\0"
    "removeprefix() argument 1 must be str, not Null\0"
    "removesuffix() argument 1 must be str, not int\0"
    "removesuffix() argument 1 must be str, not flo\0"
    "removesuffix() argument 1 must be str, not list\0"
    "removesuffix() argument 1 must be str, not tup\0"
    "removesuffix() argument 1 must be str, not dict\0"
    "removesuffix() argument 1 must be str, not byte\0"
    "removesuffix() argument 1 must be str, not bool\0"
    "removesuffix() argument 1 must be str, not Null\0"
    "old,new,count=Null\0"
    "rfind() argument 1 must be str, not int\0"
    "rfind() argument 1 must be str, not flo\0"
    "rfind() argument 1 must be str, not list\0"
    "rfind() argument 1 must be str, not tup\0"
    "rfind() argument 1 must be str, not dict\0"
    "rfind() argument 1 must be str, not byte\0"
    "rfind() argument 1 must be str, not bool\0"
    "rfind() argument 1 must be str, not Null\0"
    "rindex() argument 1 must be str, not int\0"
    "rindex() argument 1 must be str, not flo\0"
    "rindex() argument 1 must be str, not list\0"
    "rindex() argument 1 must be str, not tup\0"
    "rindex() argument 1 must be str, not dict\0"
    "rindex() argument 1 must be str, not byte\0"
    "rindex() argument 1 must be str, not bool\0"
    "rindex() argument 1 must be str, not Null\0"
    "must be str or Null, not int\0"
    "must be str or Null, not flo\0"
    "must be str or Null, not list\0"
    "must be str or Null, not tup\0"
    "must be str or Null, not dict\0"
    "must be str or Null, not byte\0"
    "must be str or Null, not bool\0"
    "rstrip() argument 1 must be str, not int\0"
    "rstrip() argument 1 must be str, not flo\0"
    "rstrip() argument 1 must be str, not list\0"
    "rstrip() argument 1 must be str, not tup\0"
    "rstrip() argument 1 must be str, not dict\0"
    "rstrip() argument 1 must be str, not byte\0"
    "rstrip() argument 1 must be str, not bool\0"
    "startswith() argument 1 must be str, not int\0"
    "startswith() argument 1 must be str, not flo\0"
    "startswith() argument 1 must be str, not dict\0"
    "startswith() argument 1 must be str, not byte\0"
    "startswith() argument 1 must be str, not bool\0"
    "startswith() argument 1 must be str, not Null\0"
    "prefixo\0"
    "strip() argument 1 must be str, not int\0"
    "strip() argument 1 must be str, not flo\0"
    "strip() argument 1 must be str, not list\0"
    "strip() argument 1 must be str, not tup\0"
    "strip() argument 1 must be str, not dict\0"
    "strip() argument 1 must be str, not byte\0"
    "strip() argument 1 must be str, not bool\0"
    "pattern,repl\0"
    "translate() argument 1 must be dict, not int\0"
    "translate() argument 1 must be dict, not flo\0"
    "translate() argument 1 must be dict, not str\0"
    "translate() argument 1 must be dict, not list\0"
    "translate() argument 1 must be dict, not tup\0"
    "translate() argument 1 must be dict, not byte\0"
    "translate() argument 1 must be dict, not bool\0"
    "translate() argument 1 must be dict, not Null\0"
    "tabela\0"
    "RelativePath() argument 1 must be str, not int\0"
    "RelativePath() argument 1 must be str, not flo\0"
    "RelativePath() argument 1 must be str, not list\0"
    "RelativePath() argument 1 must be str, not tup\0"
    "RelativePath() argument 1 must be str, not dict\0"
    "RelativePath() argument 1 must be str, not byte\0"
    "RelativePath() argument 1 must be str, not bool\0"
    "RelativePath() argument 1 must be str, not Null\0"
    "sys\0"
    "RelativePath\0"
    "exit\0"
    "code=Null\0"
    "platform\0"
    "sys.stderr\0"
    "flush\0"
    "texto,end=Null\0"
    "writeln\0"
    "sys.stdin\0"
    "raw\0"
    "ligar\0"
    "tamanho=Null\0"
    "sys.stdout\0";
static const PSAridadeBruta AR_TAB[] = {
    { 1, 12, 12, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 1, 18, 18, 26, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 1, 38, 38, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 1, 46, 46, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 100, 107, 107, 115, 0, 1, { 0, 0, 0, 0, 0, 0, 55, 0, 0 } },
    { 100, 460, 460, 467, 1, 4, { 127, 168, 0, 209, 251, 292, 334, 376, 418 } },
    { 100, 818, 818, 467, 1, 4, { 509, 547, 0, 585, 624, 662, 701, 740, 779 } },
    { 100, 822, 822, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 100, 862, 862, 0, 0, -1, { 0, 0, 833, 0, 0, 0, 0, 0, 0 } },
    { 100, 1192, 1192, 467, 1, 4, { 867, 907, 0, 947, 988, 1028, 1069, 1110, 1151 } },
    { 100, 1515, 1515, 467, 1, 4, { 1198, 1237, 0, 1276, 1316, 1355, 1395, 1435, 1475 } },
    { 100, 1829, 1829, 467, 1, 4, { 1520, 1558, 0, 1596, 1635, 1673, 1712, 1751, 1790 } },
    { 100, 2158, 2158, 2164, 1, 5, { 1833, 1873, 0, 1913, 1954, 1994, 2035, 2076, 2117 } },
    { 100, 2601, 2601, 2608, 0, 2, { 2219, 2273, 0, 2327, 2382, 2436, 2491, 2546, 0 } },
    { 100, 2632, 2632, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 100, 2646, 2646, 0, 0, -1, { 0, 0, 833, 0, 0, 0, 0, 0, 0 } },
    { 2657, 2672, 2672, 2679, 2, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2657, 2766, 2766, 2773, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2657, 2783, 2783, 2788, 1, 2, { 0, 0, 0, 0, 0, 0, 55, 0, 0 } },
    { 2657, 2805, 2805, 2810, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2657, 3174, 3174, 3181, 1, 1, { 0, 2827, 2876, 2925, 2975, 3024, 3074, 0, 3124 } },
    { 2657, 3215, 3215, 0, 0, -1, { 3186, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 3560, 3572, 3572, 3579, 1, 1, { 3227, 3268, 0, 3309, 3351, 3392, 3434, 3476, 3518 } },
    { 3560, 3915, 3915, 3920, 1, 2, { 3598, 3637, 0, 3676, 3716, 3755, 3795, 3835, 3875 } },
    { 3560, 4316, 4316, 4328, 1, 1, { 3943, 3989, 0, 4035, 4082, 4128, 4175, 4222, 4269 } },
    { 3560, 4337, 4337, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 3560, 4692, 4692, 4700, 1, 1, { 4351, 4393, 0, 4435, 4478, 4520, 4563, 4606, 4649 } },
    { 3560, 5064, 5064, 4328, 1, 1, { 4707, 4751, 0, 4795, 4840, 4884, 4929, 4974, 5019 } },
    { 5074, 3915, 3915, 5085, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5074, 5088, 5088, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5074, 5411, 5411, 5416, 1, 2, { 5094, 5133, 0, 5172, 5212, 5251, 5291, 5331, 5371 } },
    { 5074, 5432, 5432, 5438, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5074, 5452, 5452, 5459, 0, 4, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5074, 5520, 5520, 5527, 0, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5556, 5411, 5411, 5416, 1, 2, { 5094, 5133, 0, 5172, 5212, 5251, 5291, 5331, 5371 } },
    { 5556, 5432, 5432, 5438, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5556, 5567, 5567, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5556, 2805, 2805, 5572, 1, 4, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5616, 5624, 5624, 5637, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5616, 5656, 5656, 5637, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5616, 5668, 5668, 5637, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5616, 5683, 5683, 5637, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5616, 5697, 5697, 5637, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5616, 5705, 5705, 5637, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5616, 5714, 5714, 5637, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5616, 5722, 5722, 5637, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 6070, 6078, 6078, 5722, 1, 1, { 5729, 5771, 0, 5813, 5856, 5898, 5941, 5984, 6027 } },
    { 6070, 6411, 6411, 5722, 1, 1, { 6086, 6126, 0, 6166, 6207, 6247, 6288, 6329, 6370 } },
    { 6070, 6421, 6421, 5722, 1, 1, { 6086, 6126, 0, 6166, 6207, 6247, 6288, 6329, 6370 } },
    { 6070, 6427, 6427, 0, 0, -1, { 0, 0, 833, 0, 0, 0, 0, 0, 0 } },
    { 6070, 5452, 5452, 5722, 1, 1, { 6435, 6476, 0, 6517, 6559, 6600, 6642, 6684, 6726 } },
    { 6070, 7093, 7093, 7099, 1, 2, { 6768, 6808, 0, 6848, 6889, 6929, 6970, 7011, 7052 } },
    { 6070, 7117, 7117, 7121, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7144, 5088, 5088, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7144, 7159, 7159, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7144, 7166, 7166, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7144, 7514, 7514, 7522, 1, 2, { 7173, 7215, 0, 7257, 7300, 7342, 7385, 7428, 7471 } },
    { 7144, 7536, 7536, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7545, 5088, 5088, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7545, 7514, 7514, 7522, 1, 2, { 7173, 7215, 0, 7257, 7300, 7342, 7385, 7428, 7471 } },
    { 7545, 7556, 7556, 7568, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7545, 7579, 7579, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7545, 7588, 7588, 7598, 0, 1, { 0, 2827, 2876, 2925, 2975, 3024, 3074, 0, 3124 } },
    { 7545, 7605, 7605, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7545, 7614, 7614, 0, 0, -1, { 3186, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7545, 7624, 7624, 0, 0, -1, { 3186, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7633, 7642, 7642, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7633, 5088, 5088, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7633, 7965, 7965, 7970, 1, 1, { 7648, 7687, 0, 7726, 7766, 7805, 7845, 7885, 7925 } },
    { 7633, 460, 460, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7633, 7978, 7978, 0, 0, -1, { 0, 0, 833, 0, 0, 0, 0, 0, 0 } },
    { 7633, 8299, 8299, 7970, 1, 1, { 7982, 8021, 0, 8060, 8100, 8139, 8179, 8219, 8259 } },
    { 7633, 862, 862, 0, 0, -1, { 0, 0, 833, 0, 0, 0, 0, 0, 0 } },
    { 7633, 8304, 8304, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7633, 8309, 8309, 8314, 0, 1, { 0, 2827, 2876, 2925, 2975, 3024, 3074, 0, 0 } },
    { 7633, 8319, 8319, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7633, 8328, 8328, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7633, 8615, 8615, 8620, 0, 1, { 8338, 8377, 0, 8416, 8456, 8495, 8535, 8575, 0 } },
    { 7633, 8633, 8633, 0, 0, -1, { 3186, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7633, 8638, 8638, 8644, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7633, 9028, 9028, 9039, 1, 1, { 8653, 8706, 8759, 0, 0, 8812, 8866, 8920, 8974 } },
    { 9046, 9057, 9057, 9066, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 9046, 9071, 9071, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 9046, 9077, 9077, 9082, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 9046, 9091, 9091, 9102, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 9144, 5088, 5088, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 9144, 9152, 9152, 9157, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 9144, 9166, 9166, 0, 0, -1, { 3186, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 9144, 8309, 8309, 9447, 0, 1, { 0, 9170, 9209, 9248, 9288, 9327, 9367, 0, 9407 } },
    { 9144, 9759, 9759, 9447, 0, 1, { 0, 9454, 9497, 9540, 9584, 9627, 9671, 0, 9715 } },
    { 9144, 8319, 8319, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 9144, 9768, 9768, 9775, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 9144, 9790, 9790, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 9144, 9801, 9801, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 9144, 8638, 8638, 10131, 1, 1, { 9806, 9846, 0, 9886, 9927, 9967, 10008, 10049, 10090 } },
    { 10137, 862, 862, 0, 0, -1, { 0, 0, 833, 0, 0, 0, 0, 0, 0 } },
    { 10137, 9768, 9768, 10145, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10137, 8615, 8615, 8304, 1, 1, { 8338, 8377, 0, 8416, 8456, 8495, 8535, 8575, 10158 } },
    { 10137, 10198, 10198, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10206, 7642, 7642, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10206, 7978, 7978, 0, 0, -1, { 0, 0, 833, 0, 0, 0, 0, 0, 0 } },
    { 10206, 862, 862, 0, 0, -1, { 0, 0, 833, 0, 0, 0, 0, 0, 0 } },
    { 10206, 8615, 8615, 10217, 1, 1, { 8338, 8377, 0, 8416, 8456, 8495, 8535, 8575, 10158 } },
    { 10206, 8633, 8633, 0, 0, -1, { 3186, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10433, 10442, 10442, 0, 1, 1, { 0, 0, 10225, 10259, 10294, 10328, 10363, 0, 10398 } },
    { 10433, 10446, 10446, 0, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10433, 10453, 10453, 0, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10433, 10462, 10462, 10469, 1, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10433, 10502, 10502, 0, 1, 1, { 0, 2827, 2876, 2925, 2975, 3024, 3074, 0, 3124 } },
    { 10433, 10506, 10506, 0, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10433, 10511, 10511, 0, 1, 1, { 0, 2827, 2876, 2925, 2975, 3024, 3074, 0, 3124 } },
    { 10433, 10633, 10633, 0, 0, 1, { 10515, 10544, 0, 0, 0, 0, 0, 10573, 10603 } },
    { 10433, 10638, 10638, 0, 1, 1, { 10515, 10544, 0, 0, 0, 0, 0, 10573, 10603 } },
    { 10433, 10648, 10648, 0, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10433, 10898, 10898, 0, 1, 1, { 0, 0, 0, 10655, 10716, 10776, 0, 0, 10837 } },
    { 10433, 10902, 10902, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10433, 10909, 10909, 0, 1, 1, { 0, 2827, 2876, 2925, 2975, 3024, 3074, 0, 3124 } },
    { 10433, 5085, 5085, 0, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10433, 10913, 10913, 0, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10433, 11242, 11242, 0, 0, 1, { 0, 0, 0, 10919, 11000, 11080, 0, 0, 11161 } },
    { 10433, 11349, 11349, 0, 1, 1, { 11246, 11280, 0, 0, 0, 0, 0, 11314, 0 } },
    { 10433, 11353, 11353, 0, 0, 1, { 10515, 10544, 0, 0, 0, 0, 0, 10573, 10603 } },
    { 10433, 11635, 11635, 0, 0, 1, { 11358, 11397, 0, 11436, 11476, 11515, 11555, 11595, 0 } },
    { 10433, 11640, 11242, 0, 0, 1, { 0, 0, 0, 10919, 11000, 11080, 0, 0, 11161 } },
    { 10433, 11645, 11645, 0, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10433, 11649, 11649, 0, 1, -1, { 10515, 10544, 0, 0, 0, 0, 0, 10573, 10603 } },
    { 10433, 11653, 11653, 0, 1, -1, { 10515, 10544, 0, 0, 0, 0, 0, 10573, 10603 } },
    { 10433, 11657, 11657, 0, 1, 1, { 0, 2827, 2876, 2925, 2975, 3024, 3074, 0, 3124 } },
    { 10433, 11978, 11978, 11983, 1, 3, { 11661, 11700, 0, 11739, 11779, 11818, 11858, 11898, 11938 } },
    { 10433, 12409, 12409, 0, 1, 1, { 12012, 12061, 0, 12110, 12160, 12209, 12259, 12309, 12359 } },
    { 10433, 1515, 1515, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10433, 12413, 12413, 12417, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10433, 12436, 12436, 0, 1, 3, { 0, 2827, 2876, 2925, 2975, 3024, 3074, 0, 3124 } },
    { 10433, 12806, 12806, 0, 1, 1, { 12442, 12487, 12532, 0, 12577, 12622, 12668, 12714, 12760 } },
    { 10433, 13196, 13196, 0, 1, 1, { 12816, 12863, 12910, 0, 12957, 13004, 13052, 13100, 13148 } },
    { 10433, 13334, 13334, 0, 1, 1, { 13208, 13239, 0, 0, 0, 0, 0, 13270, 13302 } },
    { 10433, 13593, 13593, 0, 1, 2, { 0, 0, 13343, 13384, 13426, 13467, 13509, 0, 13551 } },
    { 10433, 13873, 13873, 0, 1, 1, { 0, 0, 13599, 13644, 13690, 13735, 13781, 0, 13827 } },
    { 10433, 13879, 13879, 0, 1, 1, { 10515, 10544, 0, 0, 0, 0, 0, 10573, 10603 } },
    { 10433, 13886, 13886, 0, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10433, 14216, 14216, 0, 1, 2, { 13890, 13936, 13982, 0, 0, 14028, 14075, 14122, 14169 } },
    { 10433, 14220, 14220, 0, 0, 1, { 10515, 10544, 0, 0, 0, 0, 0, 10573, 10603 } },
    { 10433, 14224, 14224, 0, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10433, 14229, 14229, 0, 0, -1, { 10515, 10544, 0, 0, 0, 0, 0, 10573, 10603 } },
    { 14233, 14238, 14238, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 14233, 14249, 14249, 14256, 1, 2, { 0, 2827, 2876, 2925, 2975, 3024, 3074, 0, 3124 } },
    { 14233, 14627, 14627, 7117, 1, 1, { 0, 14276, 14334, 14392, 14451, 14509, 0, 0, 14568 } },
    { 14233, 14636, 14636, 14642, 1, 3, { 0, 14276, 14334, 14392, 14451, 14509, 0, 0, 14568 } },
    { 14233, 15018, 15018, 15025, 0, 2, { 14664, 14714, 0, 14764, 14815, 14865, 14916, 14967, 0 } },
    { 14233, 15421, 15421, 15430, 1, 3, { 15058, 15118, 15178, 0, 0, 15238, 0, 15299, 15360 } },
    { 14233, 15455, 15455, 15466, 0, 1, { 0, 2827, 2876, 2925, 2975, 3024, 3074, 0, 0 } },
    { 14233, 15476, 15476, 14642, 1, 3, { 0, 14276, 14334, 14392, 14451, 14509, 0, 0, 14568 } },
    { 14233, 15481, 15481, 7117, 1, 1, { 0, 14276, 14334, 14392, 14451, 14509, 0, 0, 14568 } },
    { 14233, 10909, 10909, 15692, 0, 2, { 15485, 15519, 0, 15553, 15588, 15622, 0, 15657, 0 } },
    { 14233, 15717, 15717, 14642, 1, 3, { 0, 14276, 14334, 14392, 14451, 14509, 0, 0, 14568 } },
    { 14233, 15723, 15723, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 14233, 15731, 15731, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 14233, 15739, 15739, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 14233, 15747, 15747, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 14233, 15755, 15755, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 14233, 15763, 15763, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 14233, 15771, 15771, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 14233, 15779, 15779, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 14233, 16085, 16085, 16090, 1, 1, { 15787, 15846, 0, 0, 0, 15905, 0, 15965, 16025 } },
    { 14233, 11349, 11349, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 14233, 16096, 16096, 14256, 1, 2, { 0, 2827, 2876, 2925, 2975, 3024, 3074, 0, 3124 } },
    { 14233, 16102, 16102, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 14233, 16405, 16405, 16412, 0, 1, { 16108, 16150, 16192, 16234, 16277, 16319, 0, 16362, 0 } },
    { 14233, 16423, 16423, 16433, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 14233, 16484, 16484, 16494, 1, 1, { 16108, 16150, 16192, 16234, 16277, 16319, 0, 16362, 16441 } },
    { 14233, 16498, 16498, 16511, 1, 1, { 16108, 16150, 16192, 16234, 16277, 16319, 0, 16362, 16441 } },
    { 14233, 16513, 16513, 16511, 1, 1, { 16108, 16150, 16192, 16234, 16277, 16319, 0, 16362, 16441 } },
    { 14233, 16526, 16526, 16534, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 14233, 16551, 16551, 14642, 1, 3, { 0, 14276, 14334, 14392, 14451, 14509, 0, 0, 14568 } },
    { 14233, 16557, 16557, 14642, 1, 3, { 0, 14276, 14334, 14392, 14451, 14509, 0, 0, 14568 } },
    { 14233, 16564, 16564, 14256, 1, 2, { 0, 2827, 2876, 2925, 2975, 3024, 3074, 0, 3124 } },
    { 14233, 16570, 16570, 16494, 1, 1, { 16108, 16150, 16192, 16234, 16277, 16319, 0, 16362, 16441 } },
    { 14233, 16581, 16581, 16588, 0, 2, { 16108, 16150, 16192, 16234, 16277, 16319, 0, 16362, 0 } },
    { 14233, 16609, 16609, 16412, 0, 1, { 16108, 16150, 16192, 16234, 16277, 16319, 0, 16362, 0 } },
    { 14233, 7093, 7093, 16588, 0, 2, { 16108, 16150, 16192, 16234, 16277, 16319, 0, 16362, 0 } },
    { 14233, 16616, 16616, 16627, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 14233, 17017, 17017, 17028, 1, 3, { 16642, 16704, 16766, 0, 0, 16828, 0, 16891, 16954 } },
    { 14233, 17054, 17054, 16412, 0, 1, { 16108, 16150, 16192, 16234, 16277, 16319, 0, 16362, 0 } },
    { 14233, 17060, 17060, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 14233, 17069, 17069, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 14233, 17075, 17075, 17085, 1, 2, { 16108, 16150, 16192, 16234, 16277, 16319, 0, 16362, 0 } },
    { 14233, 17104, 17104, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 14233, 17110, 17110, 17116, 1, 1, { 0, 2827, 2876, 2925, 2975, 3024, 3074, 0, 3124 } },
    { 7642, 17464, 17464, 17471, 1, 1, { 17124, 17166, 17208, 17250, 17293, 17335, 0, 17378, 17421 } },
    { 7642, 17820, 17820, 16090, 1, 1, { 17473, 17522, 17571, 0, 0, 17620, 17670, 17720, 17770 } },
    { 7642, 18192, 18192, 18203, 1, 1, { 17827, 17872, 0, 17917, 17963, 18008, 18054, 18100, 18146 } },
    { 7642, 18530, 18530, 18203, 1, 1, { 18205, 18245, 0, 18285, 18326, 18366, 18407, 18448, 18489 } },
    { 7642, 18538, 18538, 18546, 1, 3, { 0, 2827, 2876, 2925, 2975, 3024, 3074, 0, 3124 } },
    { 7642, 818, 818, 18575, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7642, 10909, 10909, 17471, 1, 1, { 18579, 18618, 18657, 18696, 18736, 18775, 0, 18815, 18855 } },
    { 7642, 19046, 19046, 19050, 0, 1, { 0, 18895, 0, 0, 0, 18932, 0, 18970, 19008 } },
    { 7642, 19389, 19389, 19395, 1, 3, { 19057, 19098, 19139, 19180, 19222, 19263, 0, 19305, 19347 } },
    { 7642, 19747, 19747, 19753, 1, 2, { 19415, 19456, 19497, 19538, 19580, 19621, 0, 19663, 19705 } },
    { 7642, 20110, 20110, 17471, 1, 1, { 19770, 19812, 19854, 19896, 19939, 19981, 0, 20024, 20067 } },
    { 7642, 20117, 20117, 20121, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 20534, 20546, 20546, 0, 1, 1, { 20133, 20177, 20221, 20265, 20310, 20354, 20399, 20444, 20489 } },
    { 20534, 20954, 20954, 0, 1, 1, { 20553, 20597, 20641, 20685, 20730, 20774, 20819, 20864, 20909 } },
    { 20534, 21362, 21362, 0, 1, 1, { 20961, 21005, 21049, 21093, 21138, 21182, 21227, 21272, 21317 } },
    { 20534, 21779, 21779, 0, 1, 1, { 21369, 21414, 21459, 21504, 21550, 21595, 21641, 21687, 21733 } },
    { 20534, 21787, 21787, 0, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 21798, 21803, 21803, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 21798, 21812, 21812, 21817, 0, 3, { 0, 2827, 2876, 2925, 2975, 3024, 3074, 0, 0 } },
    { 21798, 21851, 21851, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 21798, 21861, 21861, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 21798, 21865, 21865, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 21798, 21870, 21870, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 21798, 21880, 21880, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 21798, 21893, 21893, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10633, 9071, 9071, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10633, 14627, 14627, 21899, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10633, 7965, 7965, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10633, 818, 818, 21905, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10633, 15481, 15481, 21899, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10633, 21924, 21924, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10633, 21930, 21930, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10633, 11349, 11349, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10633, 21935, 21935, 21905, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10633, 22279, 22279, 22286, 1, 1, { 21939, 21981, 22023, 22065, 22108, 0, 22150, 22193, 22236 } },
    { 10633, 22292, 22292, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10633, 22298, 22298, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 22305, 11635, 11635, 22312, 0, 1, { 11358, 11397, 0, 11436, 11476, 11515, 11555, 11595, 0 } },
    { 22679, 22684, 22684, 10131, 1, 1, { 22322, 22366, 0, 22410, 22455, 22499, 22544, 22589, 22634 } },
    { 22679, 22694, 22694, 22704, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 22679, 22709, 22709, 22715, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 22679, 22741, 22741, 22747, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 22679, 22753, 22753, 22704, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 22760, 100, 100, 22767, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 22760, 22841, 22841, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 22760, 2657, 2657, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 22760, 22855, 22855, 9066, 1, 1, { 0, 0, 0, 0, 0, 0, 55, 0, 0 } },
    { 22760, 23196, 23196, 23203, 1, 2, { 22863, 22904, 0, 22945, 22987, 23028, 23070, 23112, 23154 } },
    { 2783, 23431, 23431, 23437, 1, 1, { 23228, 23268, 0, 0, 0, 0, 23308, 23349, 23390 } },
    { 2783, 23442, 23442, 22292, 1, 2, { 0, 0, 0, 0, 0, 0, 55, 0, 0 } },
    { 23452, 22709, 22709, 23456, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23452, 23469, 23469, 23473, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 11353, 23503, 23503, 23510, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 11353, 9071, 9071, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 11353, 14627, 14627, 23510, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 11353, 7965, 7965, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 11353, 14636, 14636, 23510, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 11353, 23862, 23862, 23869, 1, 1, { 23515, 23564, 23613, 0, 0, 23662, 23712, 23762, 23812 } },
    { 11353, 15481, 15481, 23510, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 11353, 15717, 15717, 23875, 1, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 11353, 23897, 23897, 23904, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 11353, 11349, 11349, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 11353, 21935, 21935, 23911, 0, 1, { 0, 2827, 2876, 2925, 2975, 3024, 3074, 0, 3124 } },
    { 11353, 23916, 23916, 23510, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 11353, 23923, 23923, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 11353, 23931, 23931, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23936, 3560, 3560, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23936, 5074, 5074, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23936, 5556, 5556, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23981, 11635, 11635, 23987, 1, 1, { 11358, 11397, 0, 11436, 11476, 11515, 11555, 11595, 23941 } },
    { 23981, 11978, 11978, 23996, 1, 2, { 11661, 11700, 0, 11739, 11779, 11818, 11858, 11898, 11938 } },
    { 23981, 8309, 8309, 23987, 1, 1, { 24017, 24056, 0, 24095, 24135, 24174, 24214, 24254, 24294 } },
    { 23981, 23916, 23916, 24334, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23981, 24715, 24715, 23987, 1, 1, { 24406, 24444, 0, 24482, 24521, 24559, 24598, 24637, 24676 } },
    { 23981, 8638, 8638, 24719, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 25093, 25096, 25096, 8304, 1, 1, { 24768, 24808, 0, 24848, 24889, 24929, 24970, 25011, 25052 } },
    { 25093, 25411, 25411, 25415, 1, 2, { 25102, 25140, 0, 25178, 25217, 25255, 25294, 25333, 25372 } },
    { 25093, 3181, 3181, 22312, 0, 1, { 25437, 25476, 0, 25515, 25555, 25594, 25634, 25674, 25714 } },
    { 25093, 7965, 7965, 25754, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 25093, 25762, 25762, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 25093, 26064, 26064, 26072, 0, 1, { 25766, 25808, 0, 25850, 25893, 25935, 25978, 26021, 0 } },
    { 25093, 26414, 26414, 8304, 1, 1, { 26081, 26122, 0, 26163, 26205, 26246, 26288, 26330, 26372 } },
    { 25093, 26754, 26754, 26761, 1, 2, { 26421, 26462, 0, 26503, 26545, 26586, 26628, 26670, 26712 } },
    { 25093, 26778, 26778, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 25093, 27110, 27110, 8304, 1, 1, { 26785, 26825, 0, 26865, 26906, 26946, 26987, 27028, 27069 } },
    { 25093, 27449, 27449, 8304, 1, 1, { 27116, 27157, 0, 27198, 27240, 27281, 27323, 27365, 27407 } },
    { 25093, 27805, 27805, 27814, 1, 2, { 27456, 27499, 0, 27542, 27586, 27629, 27673, 27717, 27761 } },
    { 25093, 28130, 28130, 22312, 0, 1, { 27829, 27866, 0, 27903, 27941, 27978, 28016, 28054, 28092 } },
    { 25093, 28458, 28458, 28464, 1, 2, { 28133, 28173, 0, 28213, 28254, 28294, 28335, 28376, 28417 } },
    { 25093, 8299, 8299, 25754, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 25093, 28832, 28832, 862, 1, 1, { 28483, 28526, 0, 28569, 28613, 28656, 28700, 28744, 28788 } },
    { 25093, 29206, 29206, 862, 1, 1, { 28841, 28886, 0, 28931, 28977, 29022, 29068, 29114, 29160 } },
    { 25093, 29566, 29566, 29575, 1, 2, { 29217, 29260, 0, 29303, 29347, 29390, 29434, 29478, 29522 } },
    { 25093, 29594, 29594, 25754, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 25093, 29926, 29926, 29932, 1, 2, { 29601, 29641, 0, 29681, 29722, 29762, 29803, 29844, 29885 } },
    { 25093, 30228, 30228, 30232, 1, 3, { 29948, 29994, 0, 0, 0, 30040, 30087, 30134, 30181 } },
    { 25093, 8633, 8633, 8304, 1, 1, { 30261, 30300, 0, 30339, 30379, 30418, 30458, 30498, 30538 } },
    { 25093, 30578, 30578, 30583, 0, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 25093, 30604, 30604, 30614, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 30641, 30648, 30648, 30656, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 30641, 30745, 30745, 30751, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 30781, 30788, 30788, 30795, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 30781, 23469, 23469, 30856, 1, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 30781, 9077, 9077, 30954, 1, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31390, 31396, 31396, 6427, 1, 1, { 31049, 31091, 0, 31133, 31176, 31218, 31261, 31304, 31347 } },
    { 31390, 31737, 31737, 6427, 1, 1, { 31404, 31445, 0, 31486, 31528, 31569, 31611, 31653, 31695 } },
    { 31390, 6078, 6078, 31744, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31390, 6411, 6411, 31744, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31390, 6421, 6421, 31744, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31390, 5452, 5452, 31744, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31390, 7093, 7093, 31759, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31390, 7117, 7117, 31788, 3, 4, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31819, 460, 460, 31827, 1, -1, { 127, 168, 0, 209, 251, 292, 334, 376, 418 } },
    { 31819, 818, 818, 31827, 1, -1, { 509, 547, 0, 585, 624, 662, 701, 740, 779 } },
    { 31819, 32242, 32242, 32247, 1, -1, { 31925, 31964, 0, 32003, 32043, 32082, 32122, 32162, 32202 } },
    { 31819, 1192, 1192, 31827, 1, -1, { 867, 907, 0, 947, 988, 1028, 1069, 1110, 1151 } },
    { 31819, 1515, 1515, 31827, 1, -1, { 1198, 1237, 0, 1276, 1316, 1355, 1395, 1435, 1475 } },
    { 31819, 1829, 1829, 31827, 1, -1, { 1520, 1558, 0, 1596, 1635, 1673, 1712, 1751, 1790 } },
    { 31819, 32642, 32642, 32653, 1, 1, { 32277, 32322, 0, 32367, 32413, 32458, 32504, 32550, 32596 } },
    { 2601, 32657, 32657, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2601, 32997, 32997, 33002, 1, 1, { 32664, 32711, 32758, 0, 0, 32805, 32853, 32901, 32949 } },
    { 2601, 5088, 5088, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2601, 30648, 30648, 33002, 1, 1, { 33010, 33060, 33110, 0, 0, 33160, 33211, 33262, 33313 } },
    { 2601, 33739, 33739, 33002, 1, 1, { 33364, 33417, 33470, 0, 0, 33523, 33577, 33631, 33685 } },
    { 2601, 33750, 33750, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2601, 33757, 33757, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2601, 33761, 33761, 0, 0, -1, { 3186, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2601, 33768, 33768, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2601, 33775, 33775, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2601, 33787, 33787, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2601, 33799, 33799, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2601, 33811, 33811, 33822, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2601, 33848, 33848, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2601, 33859, 33859, 33866, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2601, 33879, 33879, 0, 0, -1, { 3186, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2601, 33885, 33885, 33890, 1, 2, { 0, 2827, 2876, 2925, 2975, 3024, 3074, 0, 3124 } },
    { 2601, 33909, 33909, 33890, 1, 2, { 0, 2827, 2876, 2925, 2975, 3024, 3074, 0, 3124 } },
    { 2601, 2805, 2805, 33918, 1, 2, { 16108, 16150, 0, 16234, 16277, 16319, 0, 16362, 16441 } },
    { 2601, 33934, 33934, 33918, 1, 2, { 16108, 16150, 0, 16234, 16277, 16319, 0, 16362, 16441 } },
    { 2601, 33942, 33942, 33949, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2601, 33973, 33973, 33985, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2601, 33990, 33990, 34001, 3, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2601, 34021, 34021, 34032, 1, 1, { 0, 0, 2876, 2925, 2975, 3024, 3074, 0, 0 } },
    { 2601, 34040, 34040, 34049, 1, 1, { 0, 2827, 2876, 2925, 2975, 3024, 3074, 0, 3124 } },
    { 2601, 14224, 14224, 0, 0, -1, { 3186, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 34477, 34485, 34485, 34503, 1, 3, { 34053, 34113, 34173, 0, 0, 34233, 34294, 34355, 34416 } },
    { 34477, 34940, 34940, 34954, 1, 3, { 34544, 34600, 34656, 0, 0, 34712, 34769, 34826, 34883 } },
    { 34477, 34991, 34991, 35003, 2, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 34477, 35057, 35057, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 34477, 35075, 35075, 35083, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 34477, 35482, 35482, 33002, 1, 1, { 35093, 35141, 0, 35189, 35238, 35286, 35335, 35384, 35433 } },
    { 34477, 35885, 35885, 35899, 1, 1, { 35496, 35544, 0, 35592, 35641, 35689, 35738, 35787, 35836 } },
    { 34477, 36321, 36321, 35899, 1, 1, { 35908, 35959, 0, 36010, 36062, 36113, 36165, 36217, 36269 } },
    { 34477, 36338, 36338, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 34477, 36350, 36350, 36362, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 34477, 36779, 36779, 36794, 1, 1, { 36382, 36431, 0, 36480, 36530, 36579, 36629, 36679, 36729 } },
    { 34477, 37196, 37196, 37210, 1, 2, { 36807, 36855, 0, 36903, 36952, 37000, 37049, 37098, 37147 } },
    { 34477, 37240, 37240, 37254, 1, 2, { 0, 2827, 2876, 2925, 2975, 3024, 3074, 0, 3124 } },
    { 34477, 37277, 37277, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 34477, 37296, 37296, 37302, 1, 1, { 0, 2827, 2876, 2925, 2975, 3024, 3074, 0, 3124 } },
    { 34477, 37304, 37304, 37302, 1, 1, { 0, 2827, 2876, 2925, 2975, 3024, 3074, 0, 3124 } },
    { 34477, 37310, 37310, 37325, 1, 1, { 0, 2827, 2876, 2925, 2975, 3024, 3074, 0, 3124 } },
    { 34477, 37334, 37334, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 34477, 37744, 37744, 37759, 1, 1, { 37347, 37396, 0, 37445, 37495, 37544, 37594, 37644, 37694 } },
    { 34477, 38124, 38124, 38134, 1, 1, { 37767, 37811, 0, 37855, 37900, 37944, 37989, 38034, 38079 } },
    { 34477, 38144, 38144, 38154, 1, 1, { 16108, 16150, 0, 16234, 16277, 16319, 0, 16362, 16441 } },
    { 34477, 38164, 38164, 38174, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 34477, 38199, 38199, 38209, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 34477, 38234, 38234, 37302, 1, 1, { 0, 2827, 2876, 2925, 2975, 3024, 3074, 0, 3124 } },
    { 34477, 38240, 38240, 37302, 1, 1, { 0, 2827, 2876, 2925, 2975, 3024, 3074, 0, 3124 } },
    { 34477, 38246, 38246, 34032, 1, 1, { 0, 0, 2876, 2925, 2975, 3024, 3074, 0, 0 } },
    { 34477, 2601, 2601, 38264, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 34477, 38297, 38297, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 38649, 30648, 30648, 38657, 1, 1, { 38308, 38350, 0, 38392, 38435, 38477, 38520, 38563, 38606 } },
    { 13886, 14238, 14238, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13886, 38666, 38666, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13886, 14249, 14249, 38675, 1, 2, { 0, 2827, 2876, 2925, 2975, 3024, 3074, 0, 3124 } },
    { 13886, 14627, 14627, 7117, 1, 1, { 38695, 38738, 0, 38781, 38825, 38868, 38912, 38956, 39000 } },
    { 13886, 14636, 14636, 14642, 1, 3, { 39044, 39084, 0, 39124, 39165, 39205, 39246, 39287, 39328 } },
    { 13886, 39723, 39723, 15025, 0, 2, { 39369, 39419, 0, 39469, 39520, 39570, 39621, 39672, 0 } },
    { 13886, 15421, 15421, 39992, 1, 1, { 39730, 39773, 0, 0, 0, 39816, 39860, 39904, 39948 } },
    { 13886, 15455, 15455, 15466, 0, 1, { 0, 2827, 2876, 2925, 2975, 3024, 3074, 0, 3124 } },
    { 13886, 15476, 15476, 14642, 1, 3, { 39999, 40038, 0, 40077, 40117, 40156, 40196, 40236, 40276 } },
    { 13886, 6078, 6078, 40316, 1, 1, { 5729, 5771, 0, 5813, 5856, 5898, 5941, 5984, 6027 } },
    { 13886, 40323, 40323, 40330, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13886, 40711, 40711, 10633, 1, 1, { 40339, 40385, 40431, 40477, 40524, 0, 40570, 40617, 40664 } },
    { 13886, 818, 818, 40722, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13886, 40733, 40733, 40722, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13886, 15481, 15481, 7117, 1, 1, { 38695, 38738, 0, 38781, 38825, 38868, 38912, 38956, 39000 } },
    { 13886, 15717, 15717, 14642, 1, 3, { 40742, 40782, 0, 40822, 40863, 40903, 40944, 40985, 41026 } },
    { 13886, 15723, 15723, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13886, 15731, 15731, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13886, 15739, 15739, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13886, 41067, 41067, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13886, 15747, 15747, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13886, 41077, 41077, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13886, 15755, 15755, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13886, 41090, 41090, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13886, 41100, 41100, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13886, 15763, 15763, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13886, 15771, 15771, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13886, 15779, 15779, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13886, 16085, 16085, 16090, 1, 1, { 41112, 41159, 41206, 0, 0, 41253, 41301, 41349, 41397 } },
    { 13886, 11349, 11349, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13886, 16096, 16096, 38675, 1, 2, { 0, 2827, 2876, 2925, 2975, 3024, 3074, 0, 3124 } },
    { 13886, 16102, 16102, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13886, 16405, 16405, 16412, 0, 1, { 41445, 41486, 0, 41527, 41569, 41610, 41652, 41694, 0 } },
    { 13886, 16423, 16423, 16433, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13886, 6421, 6421, 40316, 1, 1, { 6086, 6126, 0, 6166, 6207, 6247, 6288, 6329, 6370 } },
    { 13886, 16484, 16484, 16494, 1, 1, { 41736, 41757, 0, 41778, 41800, 41821, 41843, 41865, 41887 } },
    { 13886, 16498, 16498, 16511, 1, 1, { 41909, 41956, 0, 42003, 42051, 42098, 42146, 42194, 42242 } },
    { 13886, 16513, 16513, 16511, 1, 1, { 42290, 42337, 0, 42384, 42432, 42479, 42527, 42575, 42623 } },
    { 13886, 16526, 16526, 42671, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13886, 16551, 16551, 14642, 1, 3, { 42690, 42730, 0, 42770, 42811, 42851, 42892, 42933, 42974 } },
    { 13886, 16557, 16557, 14642, 1, 3, { 43015, 43056, 0, 43097, 43139, 43180, 43222, 43264, 43306 } },
    { 13886, 16564, 16564, 38675, 1, 2, { 0, 2827, 2876, 2925, 2975, 3024, 3074, 0, 3124 } },
    { 13886, 16570, 16570, 16494, 1, 1, { 41736, 41757, 0, 41778, 41800, 41821, 41843, 41865, 41887 } },
    { 13886, 16581, 16581, 16588, 0, 2, { 43348, 43377, 0, 43406, 43436, 43465, 43495, 43525, 0 } },
    { 13886, 16609, 16609, 16412, 0, 1, { 43555, 43596, 0, 43637, 43679, 43720, 43762, 43804, 0 } },
    { 13886, 7093, 7093, 16588, 0, 2, { 43348, 43377, 0, 43406, 43436, 43465, 43495, 43525, 0 } },
    { 13886, 16616, 16616, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13886, 17017, 17017, 44120, 1, 1, { 43846, 43891, 0, 0, 0, 43936, 43982, 44028, 44074 } },
    { 13886, 17054, 17054, 16412, 0, 1, { 44128, 44168, 0, 44208, 44249, 44289, 44330, 44371, 0 } },
    { 13886, 7117, 7117, 44412, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13886, 17060, 17060, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13886, 17069, 17069, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13886, 17075, 17075, 44789, 1, 1, { 44425, 44470, 44515, 44560, 44606, 0, 44651, 44697, 44743 } },
    { 13886, 17104, 17104, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13886, 17110, 17110, 17116, 1, 1, { 0, 2827, 2876, 2925, 2975, 3024, 3074, 0, 3124 } },
    { 45177, 45181, 45181, 862, 1, 1, { 44796, 44843, 0, 44890, 44938, 44985, 45033, 45081, 45129 } },
    { 45177, 45194, 45194, 45199, 0, 1, { 0, 2827, 2876, 2925, 2975, 3024, 3074, 0, 0 } },
    { 45177, 45209, 45209, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 45218, 45229, 45229, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 45218, 8638, 8638, 45235, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 45218, 45250, 8638, 10131, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 45258, 45268, 45268, 45272, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 45258, 8309, 8309, 45278, 0, 1, { 0, 2827, 2876, 2925, 2975, 3024, 3074, 0, 3124 } },
    { 45258, 8319, 8319, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 45291, 45229, 45229, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 45291, 8638, 8638, 45235, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 45291, 45250, 8638, 10131, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 14220, 14627, 14627, 23510, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 14220, 14636, 14636, 23510, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 14220, 15481, 15481, 23510, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 14220, 15717, 15717, 23875, 1, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 14220, 11349, 11349, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
};
