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
    { 10015, 10024, 10024, 0, 1, 1, { 0, 0, 9807, 9841, 9876, 9910, 9945, 0, 9980 } },
    { 10015, 10028, 10028, 0, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10015, 10035, 10035, 0, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10015, 10044, 10044, 10051, 1, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10015, 10084, 10084, 0, 1, 1, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 2706 } },
    { 10015, 10088, 10088, 0, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10015, 10093, 10093, 0, 1, 1, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 2706 } },
    { 10015, 10215, 10215, 0, 0, 1, { 10097, 10126, 0, 0, 0, 0, 0, 10155, 10185 } },
    { 10015, 10220, 10220, 0, 1, 1, { 10097, 10126, 0, 0, 0, 0, 0, 10155, 10185 } },
    { 10015, 10230, 10230, 0, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10015, 10480, 10480, 0, 1, 1, { 0, 0, 0, 10237, 10298, 10358, 0, 0, 10419 } },
    { 10015, 10484, 10484, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10015, 10491, 10491, 0, 1, 1, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 2706 } },
    { 10015, 4667, 4667, 0, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10015, 10495, 10495, 0, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10015, 10824, 10824, 0, 0, 1, { 0, 0, 0, 10501, 10582, 10662, 0, 0, 10743 } },
    { 10015, 10931, 10931, 0, 1, 1, { 10828, 10862, 0, 0, 0, 0, 0, 10896, 0 } },
    { 10015, 10935, 10935, 0, 0, 1, { 10097, 10126, 0, 0, 0, 0, 0, 10155, 10185 } },
    { 10015, 11217, 11217, 0, 0, 1, { 10940, 10979, 0, 11018, 11058, 11097, 11137, 11177, 0 } },
    { 10015, 11222, 10824, 0, 0, 1, { 0, 0, 0, 10501, 10582, 10662, 0, 0, 10743 } },
    { 10015, 11227, 11227, 0, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10015, 11231, 11231, 0, 1, -1, { 10097, 10126, 0, 0, 0, 0, 0, 10155, 10185 } },
    { 10015, 11235, 11235, 0, 1, -1, { 10097, 10126, 0, 0, 0, 0, 0, 10155, 10185 } },
    { 10015, 11239, 11239, 0, 1, 1, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 2706 } },
    { 10015, 11560, 11560, 11565, 1, 3, { 11243, 11282, 0, 11321, 11361, 11400, 11440, 11480, 11520 } },
    { 10015, 11991, 11991, 0, 1, 1, { 11594, 11643, 0, 11692, 11742, 11791, 11841, 11891, 11941 } },
    { 10015, 1458, 1458, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10015, 11995, 11995, 11999, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10015, 12018, 12018, 0, 1, 3, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 2706 } },
    { 10015, 12388, 12388, 0, 1, 1, { 12024, 12069, 12114, 0, 12159, 12204, 12250, 12296, 12342 } },
    { 10015, 12778, 12778, 0, 1, 1, { 12398, 12445, 12492, 0, 12539, 12586, 12634, 12682, 12730 } },
    { 10015, 12916, 12916, 0, 1, 1, { 12790, 12821, 0, 0, 0, 0, 0, 12852, 12884 } },
    { 10015, 13175, 13175, 0, 1, 2, { 0, 0, 12925, 12966, 13008, 13049, 13091, 0, 13133 } },
    { 10015, 13455, 13455, 0, 1, 1, { 0, 0, 13181, 13226, 13272, 13317, 13363, 0, 13409 } },
    { 10015, 13461, 13461, 0, 1, 1, { 10097, 10126, 0, 0, 0, 0, 0, 10155, 10185 } },
    { 10015, 13468, 13468, 0, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10015, 13798, 13798, 0, 1, 2, { 13472, 13518, 13564, 0, 0, 13610, 13657, 13704, 13751 } },
    { 10015, 13802, 13802, 0, 0, 1, { 10097, 10126, 0, 0, 0, 0, 0, 10155, 10185 } },
    { 10015, 13806, 13806, 0, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10015, 13811, 13811, 0, 0, -1, { 10097, 10126, 0, 0, 0, 0, 0, 10155, 10185 } },
    { 13815, 13820, 13820, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13815, 13831, 13831, 13838, 1, 2, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 2706 } },
    { 13815, 14209, 14209, 6699, 1, 1, { 0, 13858, 13916, 13974, 14033, 14091, 0, 0, 14150 } },
    { 13815, 14218, 14218, 14224, 1, 3, { 0, 13858, 13916, 13974, 14033, 14091, 0, 0, 14150 } },
    { 13815, 14600, 14600, 14607, 0, 2, { 14246, 14296, 0, 14346, 14397, 14447, 14498, 14549, 0 } },
    { 13815, 15003, 15003, 15012, 1, 3, { 14640, 14700, 14760, 0, 0, 14820, 0, 14881, 14942 } },
    { 13815, 15037, 15037, 15048, 0, 1, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 0 } },
    { 13815, 15058, 15058, 14224, 1, 3, { 0, 13858, 13916, 13974, 14033, 14091, 0, 0, 14150 } },
    { 13815, 15063, 15063, 6699, 1, 1, { 0, 13858, 13916, 13974, 14033, 14091, 0, 0, 14150 } },
    { 13815, 10491, 10491, 15274, 0, 2, { 15067, 15101, 0, 15135, 15170, 15204, 0, 15239, 0 } },
    { 13815, 15299, 15299, 14224, 1, 3, { 0, 13858, 13916, 13974, 14033, 14091, 0, 0, 14150 } },
    { 13815, 15305, 15305, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13815, 15313, 15313, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13815, 15321, 15321, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13815, 15329, 15329, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13815, 15337, 15337, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13815, 15345, 15345, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13815, 15353, 15353, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13815, 15361, 15361, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13815, 15667, 15667, 15672, 1, 1, { 15369, 15428, 0, 0, 0, 15487, 0, 15547, 15607 } },
    { 13815, 10931, 10931, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13815, 15678, 15678, 13838, 1, 2, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 2706 } },
    { 13815, 15684, 15684, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13815, 15987, 15987, 15994, 0, 1, { 15690, 15732, 15774, 15816, 15859, 15901, 0, 15944, 0 } },
    { 13815, 16005, 16005, 16015, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13815, 16066, 16066, 16076, 1, 1, { 15690, 15732, 15774, 15816, 15859, 15901, 0, 15944, 16023 } },
    { 13815, 16080, 16080, 16093, 1, 1, { 15690, 15732, 15774, 15816, 15859, 15901, 0, 15944, 16023 } },
    { 13815, 16095, 16095, 16093, 1, 1, { 15690, 15732, 15774, 15816, 15859, 15901, 0, 15944, 16023 } },
    { 13815, 16108, 16108, 16116, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13815, 16133, 16133, 14224, 1, 3, { 0, 13858, 13916, 13974, 14033, 14091, 0, 0, 14150 } },
    { 13815, 16139, 16139, 14224, 1, 3, { 0, 13858, 13916, 13974, 14033, 14091, 0, 0, 14150 } },
    { 13815, 16146, 16146, 13838, 1, 2, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 2706 } },
    { 13815, 16152, 16152, 16076, 1, 1, { 15690, 15732, 15774, 15816, 15859, 15901, 0, 15944, 16023 } },
    { 13815, 16163, 16163, 16170, 0, 2, { 15690, 15732, 15774, 15816, 15859, 15901, 0, 15944, 0 } },
    { 13815, 16191, 16191, 15994, 0, 1, { 15690, 15732, 15774, 15816, 15859, 15901, 0, 15944, 0 } },
    { 13815, 6675, 6675, 16170, 0, 2, { 15690, 15732, 15774, 15816, 15859, 15901, 0, 15944, 0 } },
    { 13815, 16198, 16198, 16209, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13815, 16599, 16599, 16610, 1, 3, { 16224, 16286, 16348, 0, 0, 16410, 0, 16473, 16536 } },
    { 13815, 16636, 16636, 15994, 0, 1, { 15690, 15732, 15774, 15816, 15859, 15901, 0, 15944, 0 } },
    { 13815, 16642, 16642, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13815, 16651, 16651, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13815, 16657, 16657, 16667, 1, 2, { 15690, 15732, 15774, 15816, 15859, 15901, 0, 15944, 0 } },
    { 13815, 16686, 16686, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13815, 16692, 16692, 16698, 1, 1, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 2706 } },
    { 7224, 17046, 17046, 17053, 1, 1, { 16706, 16748, 16790, 16832, 16875, 16917, 0, 16960, 17003 } },
    { 7224, 17402, 17402, 15672, 1, 1, { 17055, 17104, 17153, 0, 0, 17202, 17252, 17302, 17352 } },
    { 7224, 17774, 17774, 17785, 1, 1, { 17409, 17454, 0, 17499, 17545, 17590, 17636, 17682, 17728 } },
    { 7224, 18112, 18112, 17785, 1, 1, { 17787, 17827, 0, 17867, 17908, 17948, 17989, 18030, 18071 } },
    { 7224, 18120, 18120, 18128, 1, 3, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 2706 } },
    { 7224, 761, 761, 18157, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7224, 10491, 10491, 17053, 1, 1, { 18161, 18200, 18239, 18278, 18318, 18357, 0, 18397, 18437 } },
    { 7224, 18628, 18628, 18632, 0, 1, { 0, 18477, 0, 0, 0, 18514, 0, 18552, 18590 } },
    { 7224, 18971, 18971, 18977, 1, 3, { 18639, 18680, 18721, 18762, 18804, 18845, 0, 18887, 18929 } },
    { 7224, 19329, 19329, 19335, 1, 2, { 18997, 19038, 19079, 19120, 19162, 19203, 0, 19245, 19287 } },
    { 7224, 19692, 19692, 17053, 1, 1, { 19352, 19394, 19436, 19478, 19521, 19563, 0, 19606, 19649 } },
    { 7224, 19699, 19699, 19703, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 20116, 20128, 20128, 0, 1, 1, { 19715, 19759, 19803, 19847, 19892, 19936, 19981, 20026, 20071 } },
    { 20116, 20536, 20536, 0, 1, 1, { 20135, 20179, 20223, 20267, 20312, 20356, 20401, 20446, 20491 } },
    { 20116, 20944, 20944, 0, 1, 1, { 20543, 20587, 20631, 20675, 20720, 20764, 20809, 20854, 20899 } },
    { 20116, 21361, 21361, 0, 1, 1, { 20951, 20996, 21041, 21086, 21132, 21177, 21223, 21269, 21315 } },
    { 20116, 21369, 21369, 0, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 21380, 21385, 21385, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 21380, 21394, 21394, 21399, 0, 3, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 0 } },
    { 21380, 21433, 21433, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 21380, 21443, 21443, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 21380, 21447, 21447, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 21380, 21452, 21452, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 21380, 21462, 21462, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 21380, 21475, 21475, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10215, 8653, 8653, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10215, 14209, 14209, 21481, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10215, 7547, 7547, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10215, 761, 761, 21487, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10215, 15063, 15063, 21481, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10215, 21506, 21506, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10215, 21512, 21512, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10215, 10931, 10931, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10215, 21517, 21517, 21487, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10215, 21861, 21861, 21868, 1, 1, { 21521, 21563, 21605, 21647, 21690, 0, 21732, 21775, 21818 } },
    { 10215, 21874, 21874, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10215, 21880, 21880, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 21887, 11217, 11217, 21894, 0, 1, { 10940, 10979, 0, 11018, 11058, 11097, 11137, 11177, 0 } },
    { 22261, 22266, 22266, 9713, 1, 1, { 21904, 21948, 0, 21992, 22037, 22081, 22126, 22171, 22216 } },
    { 22261, 22276, 22276, 22286, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 22261, 22291, 22291, 22297, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 22261, 22323, 22323, 22329, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 22261, 22335, 22335, 22286, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 22342, 55, 55, 22349, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 22342, 22423, 22423, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 22342, 2194, 2194, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 22342, 22437, 22437, 8648, 1, 1, { 0, 0, 0, 0, 0, 0, 2320, 0, 0 } },
    { 22342, 22778, 22778, 22785, 1, 2, { 22445, 22486, 0, 22527, 22569, 22610, 22652, 22694, 22736 } },
    { 2365, 23013, 23013, 23019, 1, 1, { 22810, 22850, 0, 0, 0, 0, 22890, 22931, 22972 } },
    { 2365, 23024, 23024, 21874, 1, 2, { 0, 0, 0, 0, 0, 0, 2320, 0, 0 } },
    { 23034, 22291, 22291, 23038, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23034, 23051, 23051, 23055, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10935, 23085, 23085, 23092, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10935, 8653, 8653, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10935, 14209, 14209, 23092, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10935, 7547, 7547, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10935, 14218, 14218, 23092, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10935, 23444, 23444, 23451, 1, 1, { 23097, 23146, 23195, 0, 0, 23244, 23294, 23344, 23394 } },
    { 10935, 15063, 15063, 23092, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10935, 15299, 15299, 23457, 1, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10935, 23479, 23479, 23486, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10935, 10931, 10931, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10935, 21517, 21517, 23493, 0, 1, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 2706 } },
    { 10935, 23498, 23498, 23092, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10935, 23505, 23505, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10935, 23513, 23513, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23518, 3142, 3142, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23518, 4656, 4656, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23518, 5138, 5138, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23563, 11217, 11217, 23569, 1, 1, { 10940, 10979, 0, 11018, 11058, 11097, 11137, 11177, 23523 } },
    { 23563, 11560, 11560, 23578, 1, 2, { 11243, 11282, 0, 11321, 11361, 11400, 11440, 11480, 11520 } },
    { 23563, 7891, 7891, 23569, 1, 1, { 23599, 23638, 0, 23677, 23717, 23756, 23796, 23836, 23876 } },
    { 23563, 23498, 23498, 23916, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23563, 24297, 24297, 23569, 1, 1, { 23988, 24026, 0, 24064, 24103, 24141, 24180, 24219, 24258 } },
    { 23563, 8220, 8220, 24301, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 24675, 24678, 24678, 7886, 1, 1, { 24350, 24390, 0, 24430, 24471, 24511, 24552, 24593, 24634 } },
    { 24675, 24993, 24993, 24997, 1, 2, { 24684, 24722, 0, 24760, 24799, 24837, 24876, 24915, 24954 } },
    { 24675, 2763, 2763, 21894, 0, 1, { 25019, 25058, 0, 25097, 25137, 25176, 25216, 25256, 25296 } },
    { 24675, 7547, 7547, 25336, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 24675, 25344, 25344, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 24675, 25646, 25646, 25654, 0, 1, { 25348, 25390, 0, 25432, 25475, 25517, 25560, 25603, 0 } },
    { 24675, 25996, 25996, 7886, 1, 1, { 25663, 25704, 0, 25745, 25787, 25828, 25870, 25912, 25954 } },
    { 24675, 26336, 26336, 26343, 1, 2, { 26003, 26044, 0, 26085, 26127, 26168, 26210, 26252, 26294 } },
    { 24675, 26360, 26360, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 24675, 26692, 26692, 7886, 1, 1, { 26367, 26407, 0, 26447, 26488, 26528, 26569, 26610, 26651 } },
    { 24675, 27031, 27031, 7886, 1, 1, { 26698, 26739, 0, 26780, 26822, 26863, 26905, 26947, 26989 } },
    { 24675, 27387, 27387, 27396, 1, 2, { 27038, 27081, 0, 27124, 27168, 27211, 27255, 27299, 27343 } },
    { 24675, 27712, 27712, 21894, 0, 1, { 27411, 27448, 0, 27485, 27523, 27560, 27598, 27636, 27674 } },
    { 24675, 28040, 28040, 28046, 1, 2, { 27715, 27755, 0, 27795, 27836, 27876, 27917, 27958, 27999 } },
    { 24675, 7881, 7881, 25336, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 24675, 28414, 28414, 805, 1, 1, { 28065, 28108, 0, 28151, 28195, 28238, 28282, 28326, 28370 } },
    { 24675, 28788, 28788, 805, 1, 1, { 28423, 28468, 0, 28513, 28559, 28604, 28650, 28696, 28742 } },
    { 24675, 29148, 29148, 29157, 1, 2, { 28799, 28842, 0, 28885, 28929, 28972, 29016, 29060, 29104 } },
    { 24675, 29176, 29176, 25336, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 24675, 29508, 29508, 29514, 1, 2, { 29183, 29223, 0, 29263, 29304, 29344, 29385, 29426, 29467 } },
    { 24675, 29810, 29810, 29814, 1, 3, { 29530, 29576, 0, 0, 0, 29622, 29669, 29716, 29763 } },
    { 24675, 8215, 8215, 7886, 1, 1, { 29843, 29882, 0, 29921, 29961, 30000, 30040, 30080, 30120 } },
    { 24675, 30160, 30160, 30165, 0, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 24675, 30186, 30186, 30196, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 30223, 30230, 30230, 30238, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 30223, 30327, 30327, 30333, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 30363, 30370, 30370, 30377, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 30363, 23051, 23051, 30438, 1, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 30363, 8659, 8659, 30536, 1, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 30972, 30978, 30978, 6009, 1, 1, { 30631, 30673, 0, 30715, 30758, 30800, 30843, 30886, 30929 } },
    { 30972, 31319, 31319, 6009, 1, 1, { 30986, 31027, 0, 31068, 31110, 31151, 31193, 31235, 31277 } },
    { 30972, 5660, 5660, 31326, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 30972, 5993, 5993, 31326, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 30972, 6003, 6003, 31326, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 30972, 5034, 5034, 31326, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 30972, 6675, 6675, 31341, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 30972, 6699, 6699, 31370, 3, 4, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31401, 403, 403, 31409, 1, -1, { 70, 111, 0, 152, 194, 235, 277, 319, 361 } },
    { 31401, 761, 761, 31409, 1, -1, { 452, 490, 0, 528, 567, 605, 644, 683, 722 } },
    { 31401, 31824, 31824, 31829, 1, -1, { 31507, 31546, 0, 31585, 31625, 31664, 31704, 31744, 31784 } },
    { 31401, 1135, 1135, 31409, 1, -1, { 810, 850, 0, 890, 931, 971, 1012, 1053, 1094 } },
    { 31401, 1458, 1458, 31409, 1, -1, { 1141, 1180, 0, 1219, 1259, 1298, 1338, 1378, 1418 } },
    { 31401, 1772, 1772, 31409, 1, -1, { 1463, 1501, 0, 1539, 1578, 1616, 1655, 1694, 1733 } },
    { 31401, 32224, 32224, 32235, 1, 1, { 31859, 31904, 0, 31949, 31995, 32040, 32086, 32132, 32178 } },
    { 2162, 32239, 32239, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2162, 32579, 32579, 32584, 1, 1, { 32246, 32293, 32340, 0, 0, 32387, 32435, 32483, 32531 } },
    { 2162, 4670, 4670, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2162, 30230, 30230, 32584, 1, 1, { 32592, 32642, 32692, 0, 0, 32742, 32793, 32844, 32895 } },
    { 2162, 33321, 33321, 32584, 1, 1, { 32946, 32999, 33052, 0, 0, 33105, 33159, 33213, 33267 } },
    { 2162, 33332, 33332, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2162, 33339, 33339, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2162, 33343, 33343, 0, 0, -1, { 2768, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2162, 33350, 33350, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2162, 33357, 33357, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2162, 33369, 33369, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2162, 33381, 33381, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2162, 33393, 33393, 33404, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2162, 33430, 33430, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2162, 33441, 33441, 33448, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2162, 33461, 33461, 0, 0, -1, { 2768, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2162, 33467, 33467, 33472, 1, 2, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 2706 } },
    { 2162, 33491, 33491, 33472, 1, 2, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 2706 } },
    { 2162, 2387, 2387, 33500, 1, 2, { 15690, 15732, 0, 15816, 15859, 15901, 0, 15944, 16023 } },
    { 2162, 33516, 33516, 33500, 1, 2, { 15690, 15732, 0, 15816, 15859, 15901, 0, 15944, 16023 } },
    { 2162, 33524, 33524, 33531, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2162, 33555, 33555, 33567, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2162, 33572, 33572, 33583, 3, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2162, 33603, 33603, 33614, 1, 1, { 0, 0, 2458, 2507, 2557, 2606, 2656, 0, 0 } },
    { 2162, 33622, 33622, 33631, 1, 1, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 2706 } },
    { 2162, 13806, 13806, 0, 0, -1, { 2768, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 34059, 34067, 34067, 34085, 1, 3, { 33635, 33695, 33755, 0, 0, 33815, 33876, 33937, 33998 } },
    { 34059, 34522, 34522, 34536, 1, 3, { 34126, 34182, 34238, 0, 0, 34294, 34351, 34408, 34465 } },
    { 34059, 34573, 34573, 34585, 2, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 34059, 34639, 34639, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 34059, 34657, 34657, 34665, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 34059, 35064, 35064, 32584, 1, 1, { 34675, 34723, 0, 34771, 34820, 34868, 34917, 34966, 35015 } },
    { 34059, 35467, 35467, 35481, 1, 1, { 35078, 35126, 0, 35174, 35223, 35271, 35320, 35369, 35418 } },
    { 34059, 35903, 35903, 35481, 1, 1, { 35490, 35541, 0, 35592, 35644, 35695, 35747, 35799, 35851 } },
    { 34059, 35920, 35920, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 34059, 35932, 35932, 35944, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 34059, 36361, 36361, 36376, 1, 1, { 35964, 36013, 0, 36062, 36112, 36161, 36211, 36261, 36311 } },
    { 34059, 36778, 36778, 36792, 1, 2, { 36389, 36437, 0, 36485, 36534, 36582, 36631, 36680, 36729 } },
    { 34059, 36822, 36822, 36836, 1, 2, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 2706 } },
    { 34059, 36859, 36859, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 34059, 36878, 36878, 36884, 1, 1, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 2706 } },
    { 34059, 36886, 36886, 36884, 1, 1, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 2706 } },
    { 34059, 36892, 36892, 36907, 1, 1, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 2706 } },
    { 34059, 36916, 36916, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 34059, 37326, 37326, 37341, 1, 1, { 36929, 36978, 0, 37027, 37077, 37126, 37176, 37226, 37276 } },
    { 34059, 37706, 37706, 37716, 1, 1, { 37349, 37393, 0, 37437, 37482, 37526, 37571, 37616, 37661 } },
    { 34059, 37726, 37726, 37736, 1, 1, { 15690, 15732, 0, 15816, 15859, 15901, 0, 15944, 16023 } },
    { 34059, 37746, 37746, 37756, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 34059, 37781, 37781, 37791, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 34059, 37816, 37816, 36884, 1, 1, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 2706 } },
    { 34059, 37822, 37822, 36884, 1, 1, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 2706 } },
    { 34059, 37828, 37828, 33614, 1, 1, { 0, 0, 2458, 2507, 2557, 2606, 2656, 0, 0 } },
    { 34059, 2162, 2162, 37846, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 34059, 37879, 37879, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 38231, 30230, 30230, 38239, 1, 1, { 37890, 37932, 0, 37974, 38017, 38059, 38102, 38145, 38188 } },
    { 13468, 13820, 13820, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13468, 38248, 38248, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13468, 13831, 13831, 38257, 1, 2, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 2706 } },
    { 13468, 14209, 14209, 6699, 1, 1, { 38277, 38320, 0, 38363, 38407, 38450, 38494, 38538, 38582 } },
    { 13468, 14218, 14218, 14224, 1, 3, { 38626, 38666, 0, 38706, 38747, 38787, 38828, 38869, 38910 } },
    { 13468, 39305, 39305, 14607, 0, 2, { 38951, 39001, 0, 39051, 39102, 39152, 39203, 39254, 0 } },
    { 13468, 15003, 15003, 39574, 1, 1, { 39312, 39355, 0, 0, 0, 39398, 39442, 39486, 39530 } },
    { 13468, 15037, 15037, 15048, 0, 1, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 2706 } },
    { 13468, 15058, 15058, 14224, 1, 3, { 39581, 39620, 0, 39659, 39699, 39738, 39778, 39818, 39858 } },
    { 13468, 5660, 5660, 39898, 1, 1, { 5311, 5353, 0, 5395, 5438, 5480, 5523, 5566, 5609 } },
    { 13468, 39905, 39905, 39912, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13468, 40293, 40293, 10215, 1, 1, { 39921, 39967, 40013, 40059, 40106, 0, 40152, 40199, 40246 } },
    { 13468, 761, 761, 40304, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13468, 40315, 40315, 40304, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13468, 15063, 15063, 6699, 1, 1, { 38277, 38320, 0, 38363, 38407, 38450, 38494, 38538, 38582 } },
    { 13468, 15299, 15299, 14224, 1, 3, { 40324, 40364, 0, 40404, 40445, 40485, 40526, 40567, 40608 } },
    { 13468, 15305, 15305, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13468, 15313, 15313, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13468, 15321, 15321, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13468, 40649, 40649, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13468, 15329, 15329, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13468, 40659, 40659, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13468, 15337, 15337, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13468, 40672, 40672, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13468, 40682, 40682, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13468, 15345, 15345, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13468, 15353, 15353, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13468, 15361, 15361, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13468, 15667, 15667, 15672, 1, 1, { 40694, 40741, 40788, 0, 0, 40835, 40883, 40931, 40979 } },
    { 13468, 10931, 10931, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13468, 15678, 15678, 38257, 1, 2, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 2706 } },
    { 13468, 15684, 15684, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13468, 15987, 15987, 15994, 0, 1, { 41027, 41068, 0, 41109, 41151, 41192, 41234, 41276, 0 } },
    { 13468, 16005, 16005, 16015, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13468, 6003, 6003, 39898, 1, 1, { 5668, 5708, 0, 5748, 5789, 5829, 5870, 5911, 5952 } },
    { 13468, 16066, 16066, 16076, 1, 1, { 41318, 41339, 0, 41360, 41382, 41403, 41425, 41447, 41469 } },
    { 13468, 16080, 16080, 16093, 1, 1, { 41491, 41538, 0, 41585, 41633, 41680, 41728, 41776, 41824 } },
    { 13468, 16095, 16095, 16093, 1, 1, { 41872, 41919, 0, 41966, 42014, 42061, 42109, 42157, 42205 } },
    { 13468, 16108, 16108, 42253, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13468, 16133, 16133, 14224, 1, 3, { 42272, 42312, 0, 42352, 42393, 42433, 42474, 42515, 42556 } },
    { 13468, 16139, 16139, 14224, 1, 3, { 42597, 42638, 0, 42679, 42721, 42762, 42804, 42846, 42888 } },
    { 13468, 16146, 16146, 38257, 1, 2, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 2706 } },
    { 13468, 16152, 16152, 16076, 1, 1, { 41318, 41339, 0, 41360, 41382, 41403, 41425, 41447, 41469 } },
    { 13468, 16163, 16163, 16170, 0, 2, { 42930, 42959, 0, 42988, 43018, 43047, 43077, 43107, 0 } },
    { 13468, 16191, 16191, 15994, 0, 1, { 43137, 43178, 0, 43219, 43261, 43302, 43344, 43386, 0 } },
    { 13468, 6675, 6675, 16170, 0, 2, { 42930, 42959, 0, 42988, 43018, 43047, 43077, 43107, 0 } },
    { 13468, 16198, 16198, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13468, 16599, 16599, 43702, 1, 1, { 43428, 43473, 0, 0, 0, 43518, 43564, 43610, 43656 } },
    { 13468, 16636, 16636, 15994, 0, 1, { 43710, 43750, 0, 43790, 43831, 43871, 43912, 43953, 0 } },
    { 13468, 6699, 6699, 43994, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13468, 16642, 16642, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13468, 16651, 16651, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13468, 16657, 16657, 44371, 1, 1, { 44007, 44052, 44097, 44142, 44188, 0, 44233, 44279, 44325 } },
    { 13468, 16686, 16686, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13468, 16692, 16692, 16698, 1, 1, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 2706 } },
    { 44759, 44763, 44763, 805, 1, 1, { 44378, 44425, 0, 44472, 44520, 44567, 44615, 44663, 44711 } },
    { 44759, 44776, 44776, 44781, 0, 1, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 0 } },
    { 44759, 44791, 44791, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 44800, 44811, 44811, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 44800, 8220, 8220, 44817, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 44800, 44832, 8220, 9713, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 44840, 44850, 44850, 44854, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 44840, 7891, 7891, 44860, 0, 1, { 0, 2409, 2458, 2507, 2557, 2606, 2656, 0, 2706 } },
    { 44840, 7901, 7901, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 44873, 44811, 44811, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 44873, 8220, 8220, 44817, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 44873, 44832, 8220, 9713, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13802, 14209, 14209, 23092, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13802, 14218, 14218, 23092, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13802, 15063, 15063, 23092, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13802, 15299, 15299, 23457, 1, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13802, 10931, 10931, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
};
