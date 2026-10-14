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
    "Jinker\0"
    "channel\0"
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
    "socket\0"
    "static_folder\0"
    "static_url\0"
    "JinkerResponse\0"
    "cookie\0"
    "nome,valor,path=\"/\",max_age=Null,httponly=True,secure=False,samesite=\"Lax\",domain=Null\0"
    "header\0"
    "key,value\0"
    "Object of type byte is not JSON serializable\0"
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
    "__universal__\0"
    "type\0"
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
    { 55, 62, 62, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 55, 403, 403, 410, 1, 4, { 70, 111, 0, 152, 194, 235, 277, 319, 361 } },
    { 55, 761, 761, 410, 1, 4, { 452, 490, 0, 528, 567, 605, 644, 683, 722 } },
    { 55, 765, 765, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 55, 805, 805, 0, 0, -1, { 0, 0, 776, 0, 0, 0, 0, 0, 0 } },
    { 55, 1135, 1135, 410, 1, 4, { 810, 850, 0, 890, 931, 971, 1012, 1053, 1094 } },
    { 55, 1458, 1458, 410, 1, 4, { 1141, 1180, 0, 1219, 1259, 1298, 1338, 1378, 1418 } },
    { 55, 1772, 1772, 410, 1, 4, { 1463, 1501, 0, 1539, 1578, 1616, 1655, 1694, 1733 } },
    { 55, 2101, 2101, 2107, 1, 5, { 1776, 1816, 0, 1856, 1897, 1937, 1978, 2019, 2060 } },
    { 55, 2162, 2162, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 55, 2169, 2169, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 55, 2183, 2183, 0, 0, -1, { 0, 0, 776, 0, 0, 0, 0, 0, 0 } },
    { 2194, 2209, 2209, 2216, 2, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2194, 2303, 2303, 2310, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2194, 2365, 2365, 2370, 1, 2, { 0, 0, 0, 0, 0, 0, 2320, 0, 0 } },
    { 2194, 2387, 2387, 2392, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2194, 2756, 2756, 2763, 1, 1, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 2706 } },
    { 2194, 2797, 2797, 0, 0, -1, { 2768, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 3142, 3154, 3154, 3161, 1, 1, { 2809, 2850, 0, 2891, 2933, 2974, 3016, 3058, 3100 } },
    { 3142, 3497, 3497, 3502, 1, 2, { 3180, 3219, 0, 3258, 3298, 3337, 3377, 3417, 3457 } },
    { 3142, 3898, 3898, 3910, 1, 1, { 3525, 3571, 0, 3617, 3664, 3710, 3757, 3804, 3851 } },
    { 3142, 3919, 3919, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 3142, 4274, 4274, 4282, 1, 1, { 3933, 3975, 0, 4017, 4060, 4102, 4145, 4188, 4231 } },
    { 3142, 4646, 4646, 3910, 1, 1, { 4289, 4333, 0, 4377, 4422, 4466, 4511, 4556, 4601 } },
    { 4656, 3497, 3497, 4667, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 4656, 4670, 4670, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 4656, 4993, 4993, 4998, 1, 2, { 4676, 4715, 0, 4754, 4794, 4833, 4873, 4913, 4953 } },
    { 4656, 5014, 5014, 5020, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 4656, 5034, 5034, 5041, 0, 4, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 4656, 5102, 5102, 5109, 0, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5138, 4993, 4993, 4998, 1, 2, { 4676, 4715, 0, 4754, 4794, 4833, 4873, 4913, 4953 } },
    { 5138, 5014, 5014, 5020, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5138, 5149, 5149, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5138, 2387, 2387, 5154, 1, 4, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5198, 5206, 5206, 5219, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5198, 5238, 5238, 5219, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5198, 5250, 5250, 5219, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5198, 5265, 5265, 5219, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5198, 5279, 5279, 5219, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5198, 5287, 5287, 5219, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5198, 5296, 5296, 5219, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5198, 5304, 5304, 5219, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5652, 5660, 5660, 5304, 1, 1, { 5311, 5353, 0, 5395, 5438, 5480, 5523, 5566, 5609 } },
    { 5652, 5993, 5993, 5304, 1, 1, { 5668, 5708, 0, 5748, 5789, 5829, 5870, 5911, 5952 } },
    { 5652, 6003, 6003, 5304, 1, 1, { 5668, 5708, 0, 5748, 5789, 5829, 5870, 5911, 5952 } },
    { 5652, 6009, 6009, 0, 0, -1, { 0, 0, 776, 0, 0, 0, 0, 0, 0 } },
    { 5652, 5034, 5034, 5304, 1, 1, { 6017, 6058, 0, 6099, 6141, 6182, 6224, 6266, 6308 } },
    { 5652, 6675, 6675, 6681, 1, 2, { 6350, 6390, 0, 6430, 6471, 6511, 6552, 6593, 6634 } },
    { 5652, 6699, 6699, 6703, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 6726, 4670, 4670, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 6726, 6741, 6741, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 6726, 6748, 6748, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 6726, 7096, 7096, 7104, 1, 2, { 6755, 6797, 0, 6839, 6882, 6924, 6967, 7010, 7053 } },
    { 6726, 7118, 7118, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7127, 4670, 4670, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7127, 7096, 7096, 7104, 1, 2, { 6755, 6797, 0, 6839, 6882, 6924, 6967, 7010, 7053 } },
    { 7127, 7138, 7138, 7150, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7127, 7161, 7161, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7127, 7170, 7170, 7180, 0, 1, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 2706 } },
    { 7127, 7187, 7187, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7127, 7196, 7196, 0, 0, -1, { 2768, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7127, 7206, 7206, 0, 0, -1, { 2768, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7215, 7224, 7224, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7215, 4670, 4670, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7215, 7547, 7547, 7552, 1, 1, { 7230, 7269, 0, 7308, 7348, 7387, 7427, 7467, 7507 } },
    { 7215, 403, 403, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7215, 7560, 7560, 0, 0, -1, { 0, 0, 776, 0, 0, 0, 0, 0, 0 } },
    { 7215, 7881, 7881, 7552, 1, 1, { 7564, 7603, 0, 7642, 7682, 7721, 7761, 7801, 7841 } },
    { 7215, 805, 805, 0, 0, -1, { 0, 0, 776, 0, 0, 0, 0, 0, 0 } },
    { 7215, 7886, 7886, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7215, 7891, 7891, 7896, 0, 1, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 0 } },
    { 7215, 7901, 7901, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7215, 7910, 7910, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7215, 8197, 8197, 8202, 0, 1, { 7920, 7959, 0, 7998, 8038, 8077, 8117, 8157, 0 } },
    { 7215, 8215, 8215, 0, 0, -1, { 2768, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7215, 8220, 8220, 8226, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7215, 8610, 8610, 8621, 1, 1, { 8235, 8288, 8341, 0, 0, 8394, 8448, 8502, 8556 } },
    { 8628, 8639, 8639, 8648, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8628, 8653, 8653, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8628, 8659, 8659, 8664, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8628, 8673, 8673, 8684, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8726, 4670, 4670, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8726, 8734, 8734, 8739, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8726, 8748, 8748, 0, 0, -1, { 2768, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8726, 7891, 7891, 9029, 0, 1, { 0, 8752, 8791, 8830, 8870, 8909, 8949, 0, 8989 } },
    { 8726, 9341, 9341, 9029, 0, 1, { 0, 9036, 9079, 9122, 9166, 9209, 9253, 0, 9297 } },
    { 8726, 7901, 7901, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8726, 9350, 9350, 9357, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8726, 9372, 9372, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8726, 9383, 9383, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8726, 8220, 8220, 9713, 1, 1, { 9388, 9428, 0, 9468, 9509, 9549, 9590, 9631, 9672 } },
    { 9719, 805, 805, 0, 0, -1, { 0, 0, 776, 0, 0, 0, 0, 0, 0 } },
    { 9719, 9350, 9350, 9727, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 9719, 8197, 8197, 7886, 1, 1, { 7920, 7959, 0, 7998, 8038, 8077, 8117, 8157, 9740 } },
    { 9719, 9780, 9780, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 9788, 7224, 7224, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 9788, 7560, 7560, 0, 0, -1, { 0, 0, 776, 0, 0, 0, 0, 0, 0 } },
    { 9788, 805, 805, 0, 0, -1, { 0, 0, 776, 0, 0, 0, 0, 0, 0 } },
    { 9788, 8197, 8197, 9799, 1, 1, { 7920, 7959, 0, 7998, 8038, 8077, 8117, 8157, 9740 } },
    { 9788, 8215, 8215, 0, 0, -1, { 2768, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 9807, 9821, 9821, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10034, 10043, 10043, 0, 1, 1, { 0, 0, 9826, 9860, 9895, 9929, 9964, 0, 9999 } },
    { 10034, 10047, 10047, 0, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10034, 10054, 10054, 0, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10034, 10063, 10063, 10070, 1, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10034, 10103, 10103, 0, 1, 1, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 2706 } },
    { 10034, 10107, 10107, 0, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10034, 10112, 10112, 0, 1, 1, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 2706 } },
    { 10034, 10234, 10234, 0, 0, 1, { 10116, 10145, 0, 0, 0, 0, 0, 10174, 10204 } },
    { 10034, 10239, 10239, 0, 1, 1, { 10116, 10145, 0, 0, 0, 0, 0, 10174, 10204 } },
    { 10034, 10249, 10249, 0, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10034, 10499, 10499, 0, 1, 1, { 0, 0, 0, 10256, 10317, 10377, 0, 0, 10438 } },
    { 10034, 10503, 10503, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10034, 10510, 10510, 0, 1, 1, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 2706 } },
    { 10034, 4667, 4667, 0, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10034, 10514, 10514, 0, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10034, 10843, 10843, 0, 0, 1, { 0, 0, 0, 10520, 10601, 10681, 0, 0, 10762 } },
    { 10034, 10950, 10950, 0, 1, 1, { 10847, 10881, 0, 0, 0, 0, 0, 10915, 0 } },
    { 10034, 10954, 10954, 0, 0, 1, { 10116, 10145, 0, 0, 0, 0, 0, 10174, 10204 } },
    { 10034, 11236, 11236, 0, 0, 1, { 10959, 10998, 0, 11037, 11077, 11116, 11156, 11196, 0 } },
    { 10034, 11241, 10843, 0, 0, 1, { 0, 0, 0, 10520, 10601, 10681, 0, 0, 10762 } },
    { 10034, 11246, 11246, 0, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10034, 11250, 11250, 0, 1, -1, { 10116, 10145, 0, 0, 0, 0, 0, 10174, 10204 } },
    { 10034, 11254, 11254, 0, 1, -1, { 10116, 10145, 0, 0, 0, 0, 0, 10174, 10204 } },
    { 10034, 11258, 11258, 0, 1, 1, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 2706 } },
    { 10034, 11579, 11579, 11584, 1, 3, { 11262, 11301, 0, 11340, 11380, 11419, 11459, 11499, 11539 } },
    { 10034, 12010, 12010, 0, 1, 1, { 11613, 11662, 0, 11711, 11761, 11810, 11860, 11910, 11960 } },
    { 10034, 1458, 1458, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10034, 12014, 12014, 12018, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10034, 12037, 12037, 0, 1, 3, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 2706 } },
    { 10034, 12407, 12407, 0, 1, 1, { 12043, 12088, 12133, 0, 12178, 12223, 12269, 12315, 12361 } },
    { 10034, 12797, 12797, 0, 1, 1, { 12417, 12464, 12511, 0, 12558, 12605, 12653, 12701, 12749 } },
    { 10034, 12935, 12935, 0, 1, 1, { 12809, 12840, 0, 0, 0, 0, 0, 12871, 12903 } },
    { 10034, 13194, 13194, 0, 1, 2, { 0, 0, 12944, 12985, 13027, 13068, 13110, 0, 13152 } },
    { 10034, 13474, 13474, 0, 1, 1, { 0, 0, 13200, 13245, 13291, 13336, 13382, 0, 13428 } },
    { 10034, 13480, 13480, 0, 1, 1, { 10116, 10145, 0, 0, 0, 0, 0, 10174, 10204 } },
    { 10034, 13487, 13487, 0, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10034, 13817, 13817, 0, 1, 2, { 13491, 13537, 13583, 0, 0, 13629, 13676, 13723, 13770 } },
    { 10034, 13821, 13821, 0, 0, 1, { 10116, 10145, 0, 0, 0, 0, 0, 10174, 10204 } },
    { 10034, 9821, 9821, 0, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10034, 13825, 13825, 0, 0, -1, { 10116, 10145, 0, 0, 0, 0, 0, 10174, 10204 } },
    { 13829, 13834, 13834, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13829, 13845, 13845, 13852, 1, 2, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 2706 } },
    { 13829, 14223, 14223, 6699, 1, 1, { 0, 13872, 13930, 13988, 14047, 14105, 0, 0, 14164 } },
    { 13829, 14232, 14232, 14238, 1, 3, { 0, 13872, 13930, 13988, 14047, 14105, 0, 0, 14164 } },
    { 13829, 14614, 14614, 14621, 0, 2, { 14260, 14310, 0, 14360, 14411, 14461, 14512, 14563, 0 } },
    { 13829, 15017, 15017, 15026, 1, 3, { 14654, 14714, 14774, 0, 0, 14834, 0, 14895, 14956 } },
    { 13829, 15051, 15051, 15062, 0, 1, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 0 } },
    { 13829, 15072, 15072, 14238, 1, 3, { 0, 13872, 13930, 13988, 14047, 14105, 0, 0, 14164 } },
    { 13829, 15077, 15077, 6699, 1, 1, { 0, 13872, 13930, 13988, 14047, 14105, 0, 0, 14164 } },
    { 13829, 10510, 10510, 15288, 0, 2, { 15081, 15115, 0, 15149, 15184, 15218, 0, 15253, 0 } },
    { 13829, 15313, 15313, 14238, 1, 3, { 0, 13872, 13930, 13988, 14047, 14105, 0, 0, 14164 } },
    { 13829, 15319, 15319, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13829, 15327, 15327, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13829, 15335, 15335, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13829, 15343, 15343, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13829, 15351, 15351, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13829, 15359, 15359, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13829, 15367, 15367, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13829, 15375, 15375, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13829, 15681, 15681, 15686, 1, 1, { 15383, 15442, 0, 0, 0, 15501, 0, 15561, 15621 } },
    { 13829, 10950, 10950, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13829, 15692, 15692, 13852, 1, 2, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 2706 } },
    { 13829, 15698, 15698, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13829, 16001, 16001, 16008, 0, 1, { 15704, 15746, 15788, 15830, 15873, 15915, 0, 15958, 0 } },
    { 13829, 16019, 16019, 16029, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13829, 16080, 16080, 16090, 1, 1, { 15704, 15746, 15788, 15830, 15873, 15915, 0, 15958, 16037 } },
    { 13829, 16094, 16094, 16107, 1, 1, { 15704, 15746, 15788, 15830, 15873, 15915, 0, 15958, 16037 } },
    { 13829, 16109, 16109, 16107, 1, 1, { 15704, 15746, 15788, 15830, 15873, 15915, 0, 15958, 16037 } },
    { 13829, 16122, 16122, 16130, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13829, 16147, 16147, 14238, 1, 3, { 0, 13872, 13930, 13988, 14047, 14105, 0, 0, 14164 } },
    { 13829, 16153, 16153, 14238, 1, 3, { 0, 13872, 13930, 13988, 14047, 14105, 0, 0, 14164 } },
    { 13829, 16160, 16160, 13852, 1, 2, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 2706 } },
    { 13829, 16166, 16166, 16090, 1, 1, { 15704, 15746, 15788, 15830, 15873, 15915, 0, 15958, 16037 } },
    { 13829, 16177, 16177, 16184, 0, 2, { 15704, 15746, 15788, 15830, 15873, 15915, 0, 15958, 0 } },
    { 13829, 16205, 16205, 16008, 0, 1, { 15704, 15746, 15788, 15830, 15873, 15915, 0, 15958, 0 } },
    { 13829, 6675, 6675, 16184, 0, 2, { 15704, 15746, 15788, 15830, 15873, 15915, 0, 15958, 0 } },
    { 13829, 16212, 16212, 16223, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13829, 16613, 16613, 16624, 1, 3, { 16238, 16300, 16362, 0, 0, 16424, 0, 16487, 16550 } },
    { 13829, 16650, 16650, 16008, 0, 1, { 15704, 15746, 15788, 15830, 15873, 15915, 0, 15958, 0 } },
    { 13829, 16656, 16656, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13829, 16665, 16665, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13829, 16671, 16671, 16681, 1, 2, { 15704, 15746, 15788, 15830, 15873, 15915, 0, 15958, 0 } },
    { 13829, 16700, 16700, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13829, 16706, 16706, 16712, 1, 1, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 2706 } },
    { 7224, 17060, 17060, 17067, 1, 1, { 16720, 16762, 16804, 16846, 16889, 16931, 0, 16974, 17017 } },
    { 7224, 17416, 17416, 15686, 1, 1, { 17069, 17118, 17167, 0, 0, 17216, 17266, 17316, 17366 } },
    { 7224, 17788, 17788, 17799, 1, 1, { 17423, 17468, 0, 17513, 17559, 17604, 17650, 17696, 17742 } },
    { 7224, 18126, 18126, 17799, 1, 1, { 17801, 17841, 0, 17881, 17922, 17962, 18003, 18044, 18085 } },
    { 7224, 18134, 18134, 18142, 1, 3, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 2706 } },
    { 7224, 761, 761, 18171, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7224, 10510, 10510, 17067, 1, 1, { 18175, 18214, 18253, 18292, 18332, 18371, 0, 18411, 18451 } },
    { 7224, 18642, 18642, 18646, 0, 1, { 0, 18491, 0, 0, 0, 18528, 0, 18566, 18604 } },
    { 7224, 18985, 18985, 18991, 1, 3, { 18653, 18694, 18735, 18776, 18818, 18859, 0, 18901, 18943 } },
    { 7224, 19343, 19343, 19349, 1, 2, { 19011, 19052, 19093, 19134, 19176, 19217, 0, 19259, 19301 } },
    { 7224, 19706, 19706, 17067, 1, 1, { 19366, 19408, 19450, 19492, 19535, 19577, 0, 19620, 19663 } },
    { 7224, 19713, 19713, 19717, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 20130, 20142, 20142, 0, 1, 1, { 19729, 19773, 19817, 19861, 19906, 19950, 19995, 20040, 20085 } },
    { 20130, 20550, 20550, 0, 1, 1, { 20149, 20193, 20237, 20281, 20326, 20370, 20415, 20460, 20505 } },
    { 20130, 20958, 20958, 0, 1, 1, { 20557, 20601, 20645, 20689, 20734, 20778, 20823, 20868, 20913 } },
    { 20130, 21375, 21375, 0, 1, 1, { 20965, 21010, 21055, 21100, 21146, 21191, 21237, 21283, 21329 } },
    { 20130, 21383, 21383, 0, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 21394, 21399, 21399, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 21394, 21408, 21408, 21413, 0, 3, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 0 } },
    { 21394, 21447, 21447, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 21394, 21457, 21457, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 21394, 21461, 21461, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 21394, 21466, 21466, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 21394, 21476, 21476, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 21394, 21489, 21489, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10234, 8653, 8653, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10234, 14223, 14223, 21495, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10234, 7547, 7547, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10234, 761, 761, 21501, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10234, 15077, 15077, 21495, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10234, 21520, 21520, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10234, 21526, 21526, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10234, 10950, 10950, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10234, 21531, 21531, 21501, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10234, 21875, 21875, 21882, 1, 1, { 21535, 21577, 21619, 21661, 21704, 0, 21746, 21789, 21832 } },
    { 10234, 21888, 21888, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10234, 21894, 21894, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 21901, 11236, 11236, 21908, 0, 1, { 10959, 10998, 0, 11037, 11077, 11116, 11156, 11196, 0 } },
    { 22275, 22280, 22280, 9713, 1, 1, { 21918, 21962, 0, 22006, 22051, 22095, 22140, 22185, 22230 } },
    { 22275, 22290, 22290, 22300, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 22275, 22305, 22305, 22311, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 22275, 22337, 22337, 22343, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 22275, 22349, 22349, 22300, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 22356, 55, 55, 22363, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 22356, 22437, 22437, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 22356, 2194, 2194, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 22356, 22451, 22451, 8648, 1, 1, { 0, 0, 0, 0, 0, 0, 2320, 0, 0 } },
    { 22356, 22792, 22792, 22799, 1, 2, { 22459, 22500, 0, 22541, 22583, 22624, 22666, 22708, 22750 } },
    { 2365, 23027, 23027, 23033, 1, 1, { 22824, 22864, 0, 0, 0, 0, 22904, 22945, 22986 } },
    { 2365, 23038, 23038, 21888, 1, 2, { 0, 0, 0, 0, 0, 0, 2320, 0, 0 } },
    { 23048, 22305, 22305, 23052, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23048, 23065, 23065, 23069, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10954, 23099, 23099, 23106, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10954, 8653, 8653, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10954, 14223, 14223, 23106, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10954, 7547, 7547, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10954, 14232, 14232, 23106, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10954, 23458, 23458, 23465, 1, 1, { 23111, 23160, 23209, 0, 0, 23258, 23308, 23358, 23408 } },
    { 10954, 15077, 15077, 23106, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10954, 15313, 15313, 23471, 1, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10954, 23493, 23493, 23500, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10954, 10950, 10950, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10954, 21531, 21531, 23507, 0, 1, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 2706 } },
    { 10954, 23512, 23512, 23106, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10954, 23519, 23519, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10954, 23527, 23527, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23532, 3142, 3142, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23532, 4656, 4656, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23532, 5138, 5138, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23577, 11236, 11236, 23583, 1, 1, { 10959, 10998, 0, 11037, 11077, 11116, 11156, 11196, 23537 } },
    { 23577, 11579, 11579, 23592, 1, 2, { 11262, 11301, 0, 11340, 11380, 11419, 11459, 11499, 11539 } },
    { 23577, 7891, 7891, 23583, 1, 1, { 23613, 23652, 0, 23691, 23731, 23770, 23810, 23850, 23890 } },
    { 23577, 23512, 23512, 23930, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23577, 24311, 24311, 23583, 1, 1, { 24002, 24040, 0, 24078, 24117, 24155, 24194, 24233, 24272 } },
    { 23577, 8220, 8220, 24315, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 24689, 24692, 24692, 7886, 1, 1, { 24364, 24404, 0, 24444, 24485, 24525, 24566, 24607, 24648 } },
    { 24689, 25007, 25007, 25011, 1, 2, { 24698, 24736, 0, 24774, 24813, 24851, 24890, 24929, 24968 } },
    { 24689, 2763, 2763, 21908, 0, 1, { 25033, 25072, 0, 25111, 25151, 25190, 25230, 25270, 25310 } },
    { 24689, 7547, 7547, 25350, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 24689, 25358, 25358, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 24689, 25660, 25660, 25668, 0, 1, { 25362, 25404, 0, 25446, 25489, 25531, 25574, 25617, 0 } },
    { 24689, 26010, 26010, 7886, 1, 1, { 25677, 25718, 0, 25759, 25801, 25842, 25884, 25926, 25968 } },
    { 24689, 26350, 26350, 26357, 1, 2, { 26017, 26058, 0, 26099, 26141, 26182, 26224, 26266, 26308 } },
    { 24689, 26374, 26374, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 24689, 26706, 26706, 7886, 1, 1, { 26381, 26421, 0, 26461, 26502, 26542, 26583, 26624, 26665 } },
    { 24689, 27045, 27045, 7886, 1, 1, { 26712, 26753, 0, 26794, 26836, 26877, 26919, 26961, 27003 } },
    { 24689, 27401, 27401, 27410, 1, 2, { 27052, 27095, 0, 27138, 27182, 27225, 27269, 27313, 27357 } },
    { 24689, 27726, 27726, 21908, 0, 1, { 27425, 27462, 0, 27499, 27537, 27574, 27612, 27650, 27688 } },
    { 24689, 28054, 28054, 28060, 1, 2, { 27729, 27769, 0, 27809, 27850, 27890, 27931, 27972, 28013 } },
    { 24689, 7881, 7881, 25350, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 24689, 28428, 28428, 805, 1, 1, { 28079, 28122, 0, 28165, 28209, 28252, 28296, 28340, 28384 } },
    { 24689, 28802, 28802, 805, 1, 1, { 28437, 28482, 0, 28527, 28573, 28618, 28664, 28710, 28756 } },
    { 24689, 29162, 29162, 29171, 1, 2, { 28813, 28856, 0, 28899, 28943, 28986, 29030, 29074, 29118 } },
    { 24689, 29190, 29190, 25350, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 24689, 29522, 29522, 29528, 1, 2, { 29197, 29237, 0, 29277, 29318, 29358, 29399, 29440, 29481 } },
    { 24689, 29824, 29824, 29828, 1, 3, { 29544, 29590, 0, 0, 0, 29636, 29683, 29730, 29777 } },
    { 24689, 8215, 8215, 7886, 1, 1, { 29857, 29896, 0, 29935, 29975, 30014, 30054, 30094, 30134 } },
    { 24689, 30174, 30174, 30179, 0, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 24689, 30200, 30200, 30210, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 30237, 30244, 30244, 30252, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 30237, 30341, 30341, 30347, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 30377, 30384, 30384, 30391, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 30377, 23065, 23065, 30452, 1, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 30377, 8659, 8659, 30550, 1, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 30986, 30992, 30992, 6009, 1, 1, { 30645, 30687, 0, 30729, 30772, 30814, 30857, 30900, 30943 } },
    { 30986, 31333, 31333, 6009, 1, 1, { 31000, 31041, 0, 31082, 31124, 31165, 31207, 31249, 31291 } },
    { 30986, 5660, 5660, 31340, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 30986, 5993, 5993, 31340, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 30986, 6003, 6003, 31340, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 30986, 5034, 5034, 31340, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 30986, 6675, 6675, 31355, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 30986, 6699, 6699, 31384, 3, 4, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31415, 403, 403, 31423, 1, -1, { 70, 111, 0, 152, 194, 235, 277, 319, 361 } },
    { 31415, 761, 761, 31423, 1, -1, { 452, 490, 0, 528, 567, 605, 644, 683, 722 } },
    { 31415, 31838, 31838, 31843, 1, -1, { 31521, 31560, 0, 31599, 31639, 31678, 31718, 31758, 31798 } },
    { 31415, 1135, 1135, 31423, 1, -1, { 810, 850, 0, 890, 931, 971, 1012, 1053, 1094 } },
    { 31415, 1458, 1458, 31423, 1, -1, { 1141, 1180, 0, 1219, 1259, 1298, 1338, 1378, 1418 } },
    { 31415, 1772, 1772, 31423, 1, -1, { 1463, 1501, 0, 1539, 1578, 1616, 1655, 1694, 1733 } },
    { 31415, 32238, 32238, 32249, 1, 1, { 31873, 31918, 0, 31963, 32009, 32054, 32100, 32146, 32192 } },
    { 2162, 32253, 32253, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2162, 32593, 32593, 32598, 1, 1, { 32260, 32307, 32354, 0, 0, 32401, 32449, 32497, 32545 } },
    { 2162, 4670, 4670, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2162, 30244, 30244, 32598, 1, 1, { 32606, 32656, 32706, 0, 0, 32756, 32807, 32858, 32909 } },
    { 2162, 33335, 33335, 32598, 1, 1, { 32960, 33013, 33066, 0, 0, 33119, 33173, 33227, 33281 } },
    { 2162, 33346, 33346, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2162, 33353, 33353, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2162, 33357, 33357, 0, 0, -1, { 2768, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2162, 33364, 33364, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2162, 33371, 33371, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2162, 33383, 33383, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2162, 33395, 33395, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2162, 33407, 33407, 33418, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2162, 33444, 33444, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2162, 33455, 33455, 33462, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2162, 33475, 33475, 0, 0, -1, { 2768, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2162, 33481, 33481, 33486, 1, 2, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 2706 } },
    { 2162, 33505, 33505, 33486, 1, 2, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 2706 } },
    { 2162, 2387, 2387, 33514, 1, 2, { 15704, 15746, 0, 15830, 15873, 15915, 0, 15958, 16037 } },
    { 2162, 33530, 33530, 33514, 1, 2, { 15704, 15746, 0, 15830, 15873, 15915, 0, 15958, 16037 } },
    { 2162, 33538, 33538, 33545, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2162, 33569, 33569, 33581, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2162, 33586, 33586, 33597, 3, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2162, 33617, 33617, 33628, 1, 1, { 0, 0, 2458, 2507, 2557, 2606, 2656, 0, 0 } },
    { 2162, 33636, 33636, 33645, 1, 1, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 2706 } },
    { 2162, 9821, 9821, 0, 0, -1, { 2768, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 34073, 34081, 34081, 34099, 1, 3, { 33649, 33709, 33769, 0, 0, 33829, 33890, 33951, 34012 } },
    { 34073, 34536, 34536, 34550, 1, 3, { 34140, 34196, 34252, 0, 0, 34308, 34365, 34422, 34479 } },
    { 34073, 34587, 34587, 34599, 2, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 34073, 34653, 34653, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 34073, 34671, 34671, 34679, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 34073, 35078, 35078, 32598, 1, 1, { 34689, 34737, 0, 34785, 34834, 34882, 34931, 34980, 35029 } },
    { 34073, 35481, 35481, 35495, 1, 1, { 35092, 35140, 0, 35188, 35237, 35285, 35334, 35383, 35432 } },
    { 34073, 35917, 35917, 35495, 1, 1, { 35504, 35555, 0, 35606, 35658, 35709, 35761, 35813, 35865 } },
    { 34073, 35934, 35934, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 34073, 35946, 35946, 35958, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 34073, 36375, 36375, 36390, 1, 1, { 35978, 36027, 0, 36076, 36126, 36175, 36225, 36275, 36325 } },
    { 34073, 36792, 36792, 36806, 1, 2, { 36403, 36451, 0, 36499, 36548, 36596, 36645, 36694, 36743 } },
    { 34073, 36836, 36836, 36850, 1, 2, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 2706 } },
    { 34073, 36873, 36873, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 34073, 36892, 36892, 36898, 1, 1, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 2706 } },
    { 34073, 36900, 36900, 36898, 1, 1, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 2706 } },
    { 34073, 36906, 36906, 36921, 1, 1, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 2706 } },
    { 34073, 36930, 36930, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 34073, 37340, 37340, 37355, 1, 1, { 36943, 36992, 0, 37041, 37091, 37140, 37190, 37240, 37290 } },
    { 34073, 37720, 37720, 37730, 1, 1, { 37363, 37407, 0, 37451, 37496, 37540, 37585, 37630, 37675 } },
    { 34073, 37740, 37740, 37750, 1, 1, { 15704, 15746, 0, 15830, 15873, 15915, 0, 15958, 16037 } },
    { 34073, 37760, 37760, 37770, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 34073, 37795, 37795, 37805, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 34073, 37830, 37830, 36898, 1, 1, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 2706 } },
    { 34073, 37836, 37836, 36898, 1, 1, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 2706 } },
    { 34073, 37842, 37842, 33628, 1, 1, { 0, 0, 2458, 2507, 2557, 2606, 2656, 0, 0 } },
    { 34073, 2162, 2162, 37860, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 34073, 37893, 37893, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 38245, 30244, 30244, 38253, 1, 1, { 37904, 37946, 0, 37988, 38031, 38073, 38116, 38159, 38202 } },
    { 13487, 13834, 13834, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13487, 38262, 38262, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13487, 13845, 13845, 38271, 1, 2, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 2706 } },
    { 13487, 14223, 14223, 6699, 1, 1, { 38291, 38334, 0, 38377, 38421, 38464, 38508, 38552, 38596 } },
    { 13487, 14232, 14232, 14238, 1, 3, { 38640, 38680, 0, 38720, 38761, 38801, 38842, 38883, 38924 } },
    { 13487, 39319, 39319, 14621, 0, 2, { 38965, 39015, 0, 39065, 39116, 39166, 39217, 39268, 0 } },
    { 13487, 15017, 15017, 39588, 1, 1, { 39326, 39369, 0, 0, 0, 39412, 39456, 39500, 39544 } },
    { 13487, 15051, 15051, 15062, 0, 1, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 2706 } },
    { 13487, 15072, 15072, 14238, 1, 3, { 39595, 39634, 0, 39673, 39713, 39752, 39792, 39832, 39872 } },
    { 13487, 5660, 5660, 39912, 1, 1, { 5311, 5353, 0, 5395, 5438, 5480, 5523, 5566, 5609 } },
    { 13487, 39919, 39919, 39926, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13487, 40307, 40307, 10234, 1, 1, { 39935, 39981, 40027, 40073, 40120, 0, 40166, 40213, 40260 } },
    { 13487, 761, 761, 40318, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13487, 40329, 40329, 40318, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13487, 15077, 15077, 6699, 1, 1, { 38291, 38334, 0, 38377, 38421, 38464, 38508, 38552, 38596 } },
    { 13487, 15313, 15313, 14238, 1, 3, { 40338, 40378, 0, 40418, 40459, 40499, 40540, 40581, 40622 } },
    { 13487, 15319, 15319, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13487, 15327, 15327, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13487, 15335, 15335, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13487, 40663, 40663, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13487, 15343, 15343, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13487, 40673, 40673, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13487, 15351, 15351, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13487, 40686, 40686, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13487, 40696, 40696, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13487, 15359, 15359, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13487, 15367, 15367, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13487, 15375, 15375, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13487, 15681, 15681, 15686, 1, 1, { 40708, 40755, 40802, 0, 0, 40849, 40897, 40945, 40993 } },
    { 13487, 10950, 10950, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13487, 15692, 15692, 38271, 1, 2, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 2706 } },
    { 13487, 15698, 15698, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13487, 16001, 16001, 16008, 0, 1, { 41041, 41082, 0, 41123, 41165, 41206, 41248, 41290, 0 } },
    { 13487, 16019, 16019, 16029, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13487, 6003, 6003, 39912, 1, 1, { 5668, 5708, 0, 5748, 5789, 5829, 5870, 5911, 5952 } },
    { 13487, 16080, 16080, 16090, 1, 1, { 41332, 41353, 0, 41374, 41396, 41417, 41439, 41461, 41483 } },
    { 13487, 16094, 16094, 16107, 1, 1, { 41505, 41552, 0, 41599, 41647, 41694, 41742, 41790, 41838 } },
    { 13487, 16109, 16109, 16107, 1, 1, { 41886, 41933, 0, 41980, 42028, 42075, 42123, 42171, 42219 } },
    { 13487, 16122, 16122, 42267, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13487, 16147, 16147, 14238, 1, 3, { 42286, 42326, 0, 42366, 42407, 42447, 42488, 42529, 42570 } },
    { 13487, 16153, 16153, 14238, 1, 3, { 42611, 42652, 0, 42693, 42735, 42776, 42818, 42860, 42902 } },
    { 13487, 16160, 16160, 38271, 1, 2, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 2706 } },
    { 13487, 16166, 16166, 16090, 1, 1, { 41332, 41353, 0, 41374, 41396, 41417, 41439, 41461, 41483 } },
    { 13487, 16177, 16177, 16184, 0, 2, { 42944, 42973, 0, 43002, 43032, 43061, 43091, 43121, 0 } },
    { 13487, 16205, 16205, 16008, 0, 1, { 43151, 43192, 0, 43233, 43275, 43316, 43358, 43400, 0 } },
    { 13487, 6675, 6675, 16184, 0, 2, { 42944, 42973, 0, 43002, 43032, 43061, 43091, 43121, 0 } },
    { 13487, 16212, 16212, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13487, 16613, 16613, 43716, 1, 1, { 43442, 43487, 0, 0, 0, 43532, 43578, 43624, 43670 } },
    { 13487, 16650, 16650, 16008, 0, 1, { 43724, 43764, 0, 43804, 43845, 43885, 43926, 43967, 0 } },
    { 13487, 6699, 6699, 44008, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13487, 16656, 16656, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13487, 16665, 16665, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13487, 16671, 16671, 44385, 1, 1, { 44021, 44066, 44111, 44156, 44202, 0, 44247, 44293, 44339 } },
    { 13487, 16700, 16700, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13487, 16706, 16706, 16712, 1, 1, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 2706 } },
    { 44773, 44777, 44777, 805, 1, 1, { 44392, 44439, 0, 44486, 44534, 44581, 44629, 44677, 44725 } },
    { 44773, 44790, 44790, 44795, 0, 1, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 0 } },
    { 44773, 44805, 44805, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 44814, 44825, 44825, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 44814, 8220, 8220, 44831, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 44814, 44846, 8220, 9713, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 44854, 44864, 44864, 44868, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 44854, 7891, 7891, 44874, 0, 1, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 2706 } },
    { 44854, 7901, 7901, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 44887, 44825, 44825, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 44887, 8220, 8220, 44831, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 44887, 44846, 8220, 9713, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13821, 14223, 14223, 23106, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13821, 14232, 14232, 23106, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13821, 15077, 15077, 23106, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13821, 15313, 15313, 23471, 1, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13821, 10950, 10950, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
};
