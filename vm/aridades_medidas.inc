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
    "DbConnection\0"
    "close\0"
    "commit\0"
    "cursor\0"
    "DbCursor\0"
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
    "fetchall\0"
    "fetchmany\0"
    "size=1\0"
    "fetchone\0"
    "'int' object is not callable\0"
    "rowcount\0"
    "delete() argument 1 must be str, not int\0"
    "delete() argument 1 must be str, not flo\0"
    "delete() argument 1 must be str, not list\0"
    "delete() argument 1 must be str, not tup\0"
    "delete() argument 1 must be str, not dict\0"
    "delete() argument 1 must be str, not byte\0"
    "delete() argument 1 must be str, not bool\0"
    "delete() argument 1 must be str, not Null\0"
    "Jinker\0"
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
    "conn() argument 1 must be str, not int\0"
    "conn() argument 1 must be str, not flo\0"
    "conn() argument 1 must be str, not list\0"
    "conn() argument 1 must be str, not tup\0"
    "conn() argument 1 must be str, not dict\0"
    "conn() argument 1 must be str, not byte\0"
    "conn() argument 1 must be str, not bool\0"
    "conn() argument 1 must be str, not Null\0"
    "conn\0"
    "host,port=Null,tls=False\0"
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
    "rollback\0"
    "PoolCursor\0"
    "executemany\0"
    "sql,params\0"
    "lastrowid\0"
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
    "cors() argument 1 must be Jinker, not int\0"
    "cors() argument 1 must be Jinker, not flo\0"
    "cors() argument 1 must be Jinker, not str\0"
    "cors() argument 1 must be Jinker, not list\0"
    "cors() argument 1 must be Jinker, not tup\0"
    "cors() argument 1 must be Jinker, not dict\0"
    "cors() argument 1 must be Jinker, not byte\0"
    "cors() argument 1 must be Jinker, not bool\0"
    "cors() argument 1 must be Jinker, not Null\0"
    "cors\0"
    "*apps,options=Null,origins=Null,permiser=Null,local=True\0"
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
    "request\0"
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
    "os\0"
    "chdir() argument 1 must be str, not int\0"
    "chdir() argument 1 must be str, not flo\0"
    "chdir() argument 1 must be str, not list\0"
    "chdir() argument 1 must be str, not tup\0"
    "chdir() argument 1 must be str, not dict\0"
    "chdir() argument 1 must be str, not byte\0"
    "chdir() argument 1 must be str, not bool\0"
    "chdir() argument 1 must be str, not Null\0"
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
    "ERROR_CORRECT_H\0"
    "ERROR_CORRECT_L\0"
    "ERROR_CORRECT_M\0"
    "ERROR_CORRECT_Q\0"
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
    "socket\0"
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
    "sockets\0"
    "AF_INET\0"
    "AF_INET6\0"
    "AF_PACKET\0"
    "AF_UNIX\0"
    "AF_UNSPEC\0"
    "AI_ADDRCONFIG\0"
    "AI_ALL\0"
    "AI_CANONNAME\0"
    "AI_NUMERICHOST\0"
    "AI_NUMERICSERV\0"
    "AI_PASSIVE\0"
    "AI_V4MAPPED\0"
    "INADDR_ANY\0"
    "INADDR_BROADCAST\0"
    "INADDR_LOOPBACK\0"
    "IPPROTO_ICMP\0"
    "IPPROTO_IP\0"
    "IPPROTO_RAW\0"
    "IPPROTO_TCP\0"
    "IPPROTO_UDP\0"
    "IPV6_V6ONLY\0"
    "IP_ADD_MEMBERSHIP\0"
    "IP_DROP_MEMBERSHIP\0"
    "IP_MULTICAST_LOOP\0"
    "IP_MULTICAST_TTL\0"
    "IP_TTL\0"
    "MSG_DONTROUTE\0"
    "MSG_DONTWAIT\0"
    "MSG_OOB\0"
    "MSG_PEEK\0"
    "MSG_TRUNC\0"
    "MSG_WAITALL\0"
    "NI_DGRAM\0"
    "NI_NAMEREQD\0"
    "NI_NOFQDN\0"
    "NI_NUMERICHOST\0"
    "NI_NUMERICSERV\0"
    "SHUT_RD\0"
    "SHUT_RDWR\0"
    "SHUT_WR\0"
    "SOCK_DGRAM\0"
    "SOCK_RAW\0"
    "SOCK_SEQPACKET\0"
    "SOCK_STREAM\0"
    "SOL_SOCKET\0"
    "SOMAXCONN\0"
    "SO_BROADCAST\0"
    "SO_DONTROUTE\0"
    "SO_ERROR\0"
    "SO_KEEPALIVE\0"
    "SO_LINGER\0"
    "SO_OOBINLINE\0"
    "SO_RCVBUF\0"
    "SO_RCVTIMEO\0"
    "SO_REUSEADDR\0"
    "SO_REUSEPORT\0"
    "SO_SNDBUF\0"
    "SO_SNDTIMEO\0"
    "SO_TYPE\0"
    "TCP_KEEPCNT\0"
    "TCP_KEEPIDLE\0"
    "TCP_KEEPINTVL\0"
    "TCP_NODELAY\0"
    "create_connection(): AF_INET address must be tuple, not int\0"
    "create_connection(): AF_INET address must be tuple, not flo\0"
    "create_connection(): AF_INET address must be tuple, not str\0"
    "create_connection(): AF_INET address must be tuple, not dict\0"
    "create_connection(): AF_INET address must be tuple, not byte\0"
    "create_connection(): AF_INET address must be tuple, not bool\0"
    "create_connection(): AF_INET address must be tuple, not Null\0"
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
    "'list' object is not callable\0"
    "argv\0"
    "executable\0"
    "exit\0"
    "code=Null\0"
    "platform\0"
    "stderr\0"
    "stdin\0"
    "stdout\0"
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
    { 55, 68, 68, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 55, 74, 74, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 55, 81, 81, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 88, 68, 68, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 88, 438, 438, 446, 1, 2, { 97, 139, 0, 181, 224, 266, 309, 352, 395 } },
    { 88, 460, 460, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 88, 469, 469, 479, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 88, 486, 486, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 88, 524, 524, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 866, 873, 873, 880, 1, 4, { 533, 574, 0, 615, 657, 698, 740, 782, 824 } },
    { 866, 1231, 1231, 880, 1, 4, { 922, 960, 0, 998, 1037, 1075, 1114, 1153, 1192 } },
    { 866, 1235, 1235, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 866, 1275, 1275, 0, 0, -1, { 0, 0, 1246, 0, 0, 0, 0, 0, 0 } },
    { 866, 1605, 1605, 880, 1, 4, { 1280, 1320, 0, 1360, 1401, 1441, 1482, 1523, 1564 } },
    { 866, 1928, 1928, 880, 1, 4, { 1611, 1650, 0, 1689, 1729, 1768, 1808, 1848, 1888 } },
    { 866, 2242, 2242, 880, 1, 4, { 1933, 1971, 0, 2009, 2048, 2086, 2125, 2164, 2203 } },
    { 866, 2571, 2571, 2577, 1, 5, { 2246, 2286, 0, 2326, 2367, 2407, 2448, 2489, 2530 } },
    { 866, 2632, 2632, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 866, 2646, 2646, 0, 0, -1, { 0, 0, 1246, 0, 0, 0, 0, 0, 0 } },
    { 2657, 2672, 2672, 2679, 2, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2657, 2766, 2766, 2773, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2657, 2828, 2828, 2833, 1, 2, { 0, 0, 0, 0, 0, 0, 2783, 0, 0 } },
    { 2657, 2850, 2850, 2855, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2657, 3219, 3219, 3226, 1, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 2657, 3231, 3231, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 3576, 3588, 3588, 3595, 1, 1, { 3243, 3284, 0, 3325, 3367, 3408, 3450, 3492, 3534 } },
    { 3576, 3931, 3931, 3936, 1, 2, { 3614, 3653, 0, 3692, 3732, 3771, 3811, 3851, 3891 } },
    { 3576, 4332, 4332, 4344, 1, 1, { 3959, 4005, 0, 4051, 4098, 4144, 4191, 4238, 4285 } },
    { 3576, 4353, 4353, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 3576, 4708, 4708, 4716, 1, 1, { 4367, 4409, 0, 4451, 4494, 4536, 4579, 4622, 4665 } },
    { 3576, 5080, 5080, 4344, 1, 1, { 4723, 4767, 0, 4811, 4856, 4900, 4945, 4990, 5035 } },
    { 5090, 3931, 3931, 5101, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5090, 68, 68, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5090, 5421, 5421, 5426, 1, 3, { 5104, 5143, 0, 5182, 5222, 5261, 5301, 5341, 5381 } },
    { 5090, 5451, 5451, 5457, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5090, 5471, 5471, 5478, 0, 4, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5090, 5539, 5539, 5546, 0, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5575, 5421, 5421, 5426, 1, 3, { 5104, 5143, 0, 5182, 5222, 5261, 5301, 5341, 5381 } },
    { 5575, 5451, 5451, 5457, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5575, 5586, 5586, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5575, 2850, 2850, 5591, 1, 4, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5635, 5643, 5643, 5656, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5635, 5675, 5675, 5656, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5635, 5687, 5687, 5656, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5635, 5702, 5702, 5656, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5635, 5716, 5716, 5656, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5635, 5724, 5724, 5656, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5635, 5733, 5733, 5656, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5635, 5741, 5741, 5656, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 6089, 6097, 6097, 5741, 1, 1, { 5748, 5790, 0, 5832, 5875, 5917, 5960, 6003, 6046 } },
    { 6089, 6430, 6430, 5741, 1, 1, { 6105, 6145, 0, 6185, 6226, 6266, 6307, 6348, 6389 } },
    { 6089, 6440, 6440, 5741, 1, 1, { 6105, 6145, 0, 6185, 6226, 6266, 6307, 6348, 6389 } },
    { 6089, 6446, 6446, 0, 0, -1, { 0, 0, 1246, 0, 0, 0, 0, 0, 0 } },
    { 6089, 5471, 5471, 5741, 1, 1, { 6454, 6495, 0, 6536, 6578, 6619, 6661, 6703, 6745 } },
    { 6089, 7112, 7112, 7118, 1, 2, { 6787, 6827, 0, 6867, 6908, 6948, 6989, 7030, 7071 } },
    { 6089, 7136, 7136, 7140, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7163, 68, 68, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7163, 74, 74, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7163, 81, 81, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7163, 438, 438, 446, 1, 2, { 97, 139, 0, 181, 224, 266, 309, 352, 395 } },
    { 7163, 7178, 7178, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7187, 68, 68, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7187, 438, 438, 446, 1, 2, { 97, 139, 0, 181, 224, 266, 309, 352, 395 } },
    { 7187, 7198, 7198, 7210, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7187, 460, 460, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7187, 469, 469, 479, 0, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 7187, 486, 486, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7187, 7221, 7221, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7187, 524, 524, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7231, 7240, 7240, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7231, 68, 68, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7231, 7563, 7563, 7568, 1, 1, { 7246, 7285, 0, 7324, 7364, 7403, 7443, 7483, 7523 } },
    { 7231, 873, 873, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7231, 7576, 7576, 0, 0, -1, { 0, 0, 1246, 0, 0, 0, 0, 0, 0 } },
    { 7231, 7897, 7897, 7568, 1, 1, { 7580, 7619, 0, 7658, 7698, 7737, 7777, 7817, 7857 } },
    { 7231, 1275, 1275, 0, 0, -1, { 0, 0, 1246, 0, 0, 0, 0, 0, 0 } },
    { 7231, 7902, 7902, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7231, 7907, 7907, 7912, 0, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 0 } },
    { 7231, 7917, 7917, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7231, 7926, 7926, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7231, 8213, 8213, 8218, 0, 1, { 7936, 7975, 0, 8014, 8054, 8093, 8133, 8173, 0 } },
    { 7231, 8231, 8231, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7231, 8236, 8236, 8242, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7231, 8626, 8626, 8637, 1, 1, { 8251, 8304, 8357, 0, 0, 8410, 8464, 8518, 8572 } },
    { 8644, 8655, 8655, 8664, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8644, 8669, 8669, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8644, 8675, 8675, 8680, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8644, 8689, 8689, 8700, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8742, 68, 68, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8742, 8750, 8750, 8755, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8742, 8764, 8764, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8742, 7907, 7907, 9045, 0, 1, { 0, 8768, 8807, 8846, 8886, 8925, 8965, 0, 9005 } },
    { 8742, 9357, 9357, 9045, 0, 1, { 0, 9052, 9095, 9138, 9182, 9225, 9269, 0, 9313 } },
    { 8742, 7917, 7917, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8742, 9366, 9366, 9373, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8742, 9388, 9388, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8742, 9399, 9399, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8742, 8236, 8236, 9729, 1, 1, { 9404, 9444, 0, 9484, 9525, 9565, 9606, 9647, 9688 } },
    { 9735, 1275, 1275, 0, 0, -1, { 0, 0, 1246, 0, 0, 0, 0, 0, 0 } },
    { 9735, 9366, 9366, 9743, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 9735, 8213, 8213, 7902, 1, 1, { 7936, 7975, 0, 8014, 8054, 8093, 8133, 8173, 9756 } },
    { 9735, 9796, 9796, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 9804, 7240, 7240, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 9804, 7576, 7576, 0, 0, -1, { 0, 0, 1246, 0, 0, 0, 0, 0, 0 } },
    { 9804, 1275, 1275, 0, 0, -1, { 0, 0, 1246, 0, 0, 0, 0, 0, 0 } },
    { 9804, 8213, 8213, 9815, 1, 1, { 7936, 7975, 0, 8014, 8054, 8093, 8133, 8173, 9756 } },
    { 9804, 8231, 8231, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10031, 10040, 10040, 0, 1, 1, { 0, 0, 9823, 9857, 9892, 9926, 9961, 0, 9996 } },
    { 10031, 10044, 10044, 0, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10031, 10051, 10051, 0, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10031, 10060, 10060, 10067, 1, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10031, 10100, 10100, 0, 1, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 10031, 10104, 10104, 0, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10031, 10109, 10109, 0, 1, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 10031, 10231, 10231, 0, 0, 1, { 10113, 10142, 0, 0, 0, 0, 0, 10171, 10201 } },
    { 10031, 10236, 10236, 0, 1, 1, { 10113, 10142, 0, 0, 0, 0, 0, 10171, 10201 } },
    { 10031, 10246, 10246, 0, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10031, 10496, 10496, 0, 1, 1, { 0, 0, 0, 10253, 10314, 10374, 0, 0, 10435 } },
    { 10031, 10500, 10500, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10031, 10507, 10507, 0, 1, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 10031, 5101, 5101, 0, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10031, 10511, 10511, 0, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10031, 10840, 10840, 0, 0, 1, { 0, 0, 0, 10517, 10598, 10678, 0, 0, 10759 } },
    { 10031, 10947, 10947, 0, 1, 1, { 10844, 10878, 0, 0, 0, 0, 0, 10912, 0 } },
    { 10031, 10951, 10951, 0, 0, 1, { 10113, 10142, 0, 0, 0, 0, 0, 10171, 10201 } },
    { 10031, 11233, 11233, 0, 0, 1, { 10956, 10995, 0, 11034, 11074, 11113, 11153, 11193, 0 } },
    { 10031, 11238, 10840, 0, 0, 1, { 0, 0, 0, 10517, 10598, 10678, 0, 0, 10759 } },
    { 10031, 11243, 11243, 0, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10031, 11247, 11247, 0, 1, -1, { 10113, 10142, 0, 0, 0, 0, 0, 10171, 10201 } },
    { 10031, 11251, 11251, 0, 1, -1, { 10113, 10142, 0, 0, 0, 0, 0, 10171, 10201 } },
    { 10031, 11255, 11255, 0, 1, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 10031, 11576, 11576, 11581, 1, 3, { 11259, 11298, 0, 11337, 11377, 11416, 11456, 11496, 11536 } },
    { 10031, 12007, 12007, 0, 1, 1, { 11610, 11659, 0, 11708, 11758, 11807, 11857, 11907, 11957 } },
    { 10031, 1928, 1928, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10031, 12011, 12011, 12015, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10031, 12034, 12034, 0, 1, 3, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 10031, 12404, 12404, 0, 1, 1, { 12040, 12085, 12130, 0, 12175, 12220, 12266, 12312, 12358 } },
    { 10031, 12794, 12794, 0, 1, 1, { 12414, 12461, 12508, 0, 12555, 12602, 12650, 12698, 12746 } },
    { 10031, 12932, 12932, 0, 1, 1, { 12806, 12837, 0, 0, 0, 0, 0, 12868, 12900 } },
    { 10031, 13191, 13191, 0, 1, 2, { 0, 0, 12941, 12982, 13024, 13065, 13107, 0, 13149 } },
    { 10031, 13471, 13471, 0, 1, 1, { 0, 0, 13197, 13242, 13288, 13333, 13379, 0, 13425 } },
    { 10031, 13477, 13477, 0, 1, 1, { 10113, 10142, 0, 0, 0, 0, 0, 10171, 10201 } },
    { 10031, 13484, 13484, 0, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10031, 13814, 13814, 0, 1, 2, { 13488, 13534, 13580, 0, 0, 13626, 13673, 13720, 13767 } },
    { 10031, 13818, 13818, 0, 0, 1, { 10113, 10142, 0, 0, 0, 0, 0, 10171, 10201 } },
    { 10031, 13822, 13822, 0, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10031, 13827, 13827, 0, 0, -1, { 10113, 10142, 0, 0, 0, 0, 0, 10171, 10201 } },
    { 13831, 13836, 13836, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13831, 13847, 13847, 13854, 1, 2, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 13831, 14225, 14225, 7136, 1, 1, { 0, 13874, 13932, 13990, 14049, 14107, 0, 0, 14166 } },
    { 13831, 14234, 14234, 14240, 1, 3, { 0, 13874, 13932, 13990, 14049, 14107, 0, 0, 14166 } },
    { 13831, 14616, 14616, 14623, 0, 2, { 14262, 14312, 0, 14362, 14413, 14463, 14514, 14565, 0 } },
    { 13831, 15019, 15019, 15028, 1, 3, { 14656, 14716, 14776, 0, 0, 14836, 0, 14897, 14958 } },
    { 13831, 15053, 15053, 15064, 0, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 0 } },
    { 13831, 15074, 15074, 14240, 1, 3, { 0, 13874, 13932, 13990, 14049, 14107, 0, 0, 14166 } },
    { 13831, 15079, 15079, 7136, 1, 1, { 0, 13874, 13932, 13990, 14049, 14107, 0, 0, 14166 } },
    { 13831, 10507, 10507, 15290, 0, 2, { 15083, 15117, 0, 15151, 15186, 15220, 0, 15255, 0 } },
    { 13831, 15315, 15315, 14240, 1, 3, { 0, 13874, 13932, 13990, 14049, 14107, 0, 0, 14166 } },
    { 13831, 15321, 15321, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13831, 15329, 15329, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13831, 15337, 15337, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13831, 15345, 15345, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13831, 15353, 15353, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13831, 15361, 15361, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13831, 15369, 15369, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13831, 15377, 15377, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13831, 15683, 15683, 15688, 1, 1, { 15385, 15444, 0, 0, 0, 15503, 0, 15563, 15623 } },
    { 13831, 10947, 10947, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13831, 15694, 15694, 13854, 1, 2, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 13831, 15700, 15700, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13831, 16003, 16003, 16010, 0, 1, { 15706, 15748, 15790, 15832, 15875, 15917, 0, 15960, 0 } },
    { 13831, 16021, 16021, 16031, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13831, 16082, 16082, 16092, 1, 1, { 15706, 15748, 15790, 15832, 15875, 15917, 0, 15960, 16039 } },
    { 13831, 16096, 16096, 16109, 1, 1, { 15706, 15748, 15790, 15832, 15875, 15917, 0, 15960, 16039 } },
    { 13831, 16111, 16111, 16109, 1, 1, { 15706, 15748, 15790, 15832, 15875, 15917, 0, 15960, 16039 } },
    { 13831, 16124, 16124, 16132, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13831, 16149, 16149, 14240, 1, 3, { 0, 13874, 13932, 13990, 14049, 14107, 0, 0, 14166 } },
    { 13831, 16155, 16155, 14240, 1, 3, { 0, 13874, 13932, 13990, 14049, 14107, 0, 0, 14166 } },
    { 13831, 16162, 16162, 13854, 1, 2, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 13831, 16168, 16168, 16092, 1, 1, { 15706, 15748, 15790, 15832, 15875, 15917, 0, 15960, 16039 } },
    { 13831, 16179, 16179, 16186, 0, 2, { 15706, 15748, 15790, 15832, 15875, 15917, 0, 15960, 0 } },
    { 13831, 16207, 16207, 16010, 0, 1, { 15706, 15748, 15790, 15832, 15875, 15917, 0, 15960, 0 } },
    { 13831, 7112, 7112, 16186, 0, 2, { 15706, 15748, 15790, 15832, 15875, 15917, 0, 15960, 0 } },
    { 13831, 16214, 16214, 16225, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13831, 16615, 16615, 16626, 1, 3, { 16240, 16302, 16364, 0, 0, 16426, 0, 16489, 16552 } },
    { 13831, 16652, 16652, 16010, 0, 1, { 15706, 15748, 15790, 15832, 15875, 15917, 0, 15960, 0 } },
    { 13831, 16658, 16658, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13831, 16667, 16667, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13831, 16673, 16673, 16683, 1, 2, { 15706, 15748, 15790, 15832, 15875, 15917, 0, 15960, 0 } },
    { 13831, 16702, 16702, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13831, 16708, 16708, 16714, 1, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 7240, 17062, 17062, 17069, 1, 1, { 16722, 16764, 16806, 16848, 16891, 16933, 0, 16976, 17019 } },
    { 7240, 17418, 17418, 15688, 1, 1, { 17071, 17120, 17169, 0, 0, 17218, 17268, 17318, 17368 } },
    { 7240, 17790, 17790, 17801, 1, 1, { 17425, 17470, 0, 17515, 17561, 17606, 17652, 17698, 17744 } },
    { 7240, 18128, 18128, 17801, 1, 1, { 17803, 17843, 0, 17883, 17924, 17964, 18005, 18046, 18087 } },
    { 7240, 18136, 18136, 18144, 1, 3, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 7240, 1231, 1231, 18173, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7240, 10507, 10507, 17069, 1, 1, { 18177, 18216, 18255, 18294, 18334, 18373, 0, 18413, 18453 } },
    { 7240, 18644, 18644, 18648, 0, 1, { 0, 18493, 0, 0, 0, 18530, 0, 18568, 18606 } },
    { 7240, 18987, 18987, 18993, 1, 3, { 18655, 18696, 18737, 18778, 18820, 18861, 0, 18903, 18945 } },
    { 7240, 19345, 19345, 19351, 1, 2, { 19013, 19054, 19095, 19136, 19178, 19219, 0, 19261, 19303 } },
    { 7240, 19708, 19708, 17069, 1, 1, { 19368, 19410, 19452, 19494, 19537, 19579, 0, 19622, 19665 } },
    { 7240, 19715, 19715, 19719, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 20132, 20144, 20144, 0, 1, 1, { 19731, 19775, 19819, 19863, 19908, 19952, 19997, 20042, 20087 } },
    { 20132, 20552, 20552, 0, 1, 1, { 20151, 20195, 20239, 20283, 20328, 20372, 20417, 20462, 20507 } },
    { 20132, 20960, 20960, 0, 1, 1, { 20559, 20603, 20647, 20691, 20736, 20780, 20825, 20870, 20915 } },
    { 20132, 21377, 21377, 0, 1, 1, { 20967, 21012, 21057, 21102, 21148, 21193, 21239, 21285, 21331 } },
    { 20132, 21385, 21385, 0, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 21396, 21401, 21401, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 21396, 21410, 21410, 21415, 0, 3, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 0 } },
    { 21396, 21449, 21449, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 21396, 21459, 21459, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 21396, 21463, 21463, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 21396, 21468, 21468, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 21396, 21478, 21478, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 21396, 21491, 21491, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10231, 8669, 8669, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10231, 14225, 14225, 21497, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10231, 7563, 7563, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10231, 1231, 1231, 21503, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10231, 15079, 15079, 21497, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10231, 21522, 21522, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10231, 21528, 21528, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10231, 10947, 10947, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10231, 21533, 21533, 21503, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10231, 21877, 21877, 21884, 1, 1, { 21537, 21579, 21621, 21663, 21706, 0, 21748, 21791, 21834 } },
    { 10231, 21890, 21890, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10231, 21896, 21896, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 21903, 11233, 11233, 21910, 0, 1, { 10956, 10995, 0, 11034, 11074, 11113, 11153, 11193, 0 } },
    { 22277, 22282, 22282, 9729, 1, 1, { 21920, 21964, 0, 22008, 22053, 22097, 22142, 22187, 22232 } },
    { 22277, 22292, 22292, 22302, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 22277, 22307, 22307, 22313, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 22277, 22339, 22339, 22345, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 22277, 22351, 22351, 22302, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 22358, 866, 866, 22365, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 22358, 22439, 22439, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 22358, 2657, 2657, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 22358, 22836, 22836, 22841, 0, -1, { 22453, 22495, 22537, 22579, 22622, 22664, 22707, 22750, 22793 } },
    { 22358, 22898, 22898, 8664, 1, 1, { 0, 0, 0, 0, 0, 0, 2783, 0, 0 } },
    { 22358, 23239, 23239, 23246, 1, 2, { 22906, 22947, 0, 22988, 23030, 23071, 23113, 23155, 23197 } },
    { 22358, 23271, 23271, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2828, 23482, 23482, 23488, 1, 1, { 23279, 23319, 0, 0, 0, 0, 23359, 23400, 23441 } },
    { 2828, 23493, 23493, 21890, 1, 2, { 0, 0, 0, 0, 0, 0, 2783, 0, 0 } },
    { 23503, 22307, 22307, 23507, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23503, 23520, 23520, 23524, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10951, 23554, 23554, 23561, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10951, 8669, 8669, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10951, 14225, 14225, 23561, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10951, 7563, 7563, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10951, 14234, 14234, 23561, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10951, 23913, 23913, 23920, 1, 1, { 23566, 23615, 23664, 0, 0, 23713, 23763, 23813, 23863 } },
    { 10951, 15079, 15079, 23561, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10951, 15315, 15315, 23926, 1, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10951, 23948, 23948, 23955, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10951, 10947, 10947, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10951, 21533, 21533, 23962, 0, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 10951, 23967, 23967, 23561, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10951, 23974, 23974, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10951, 23982, 23982, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23987, 3576, 3576, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23987, 5090, 5090, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23987, 5575, 5575, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23992, 7231, 7231, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23992, 24320, 24320, 7902, 1, 1, { 23995, 24035, 0, 24075, 24116, 24156, 24197, 24238, 24279 } },
    { 23992, 24635, 24635, 24639, 1, 2, { 24326, 24364, 0, 24402, 24441, 24479, 24518, 24557, 24596 } },
    { 23992, 7563, 7563, 24661, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23992, 24669, 24669, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23992, 24971, 24971, 24979, 0, 1, { 24673, 24715, 0, 24757, 24800, 24842, 24885, 24928, 0 } },
    { 23992, 25321, 25321, 7902, 1, 1, { 24988, 25029, 0, 25070, 25112, 25153, 25195, 25237, 25279 } },
    { 23992, 25661, 25661, 25668, 1, 2, { 25328, 25369, 0, 25410, 25452, 25493, 25535, 25577, 25619 } },
    { 23992, 25685, 25685, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23992, 26017, 26017, 7902, 1, 1, { 25692, 25732, 0, 25772, 25813, 25853, 25894, 25935, 25976 } },
    { 23992, 26356, 26356, 7902, 1, 1, { 26023, 26064, 0, 26105, 26147, 26188, 26230, 26272, 26314 } },
    { 23992, 26712, 26712, 26721, 1, 2, { 26363, 26406, 0, 26449, 26493, 26536, 26580, 26624, 26668 } },
    { 23992, 27037, 27037, 21910, 0, 1, { 26736, 26773, 0, 26810, 26848, 26885, 26923, 26961, 26999 } },
    { 23992, 27365, 27365, 27371, 1, 2, { 27040, 27080, 0, 27120, 27161, 27201, 27242, 27283, 27324 } },
    { 23992, 7897, 7897, 24661, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23992, 27739, 27739, 1275, 1, 1, { 27390, 27433, 0, 27476, 27520, 27563, 27607, 27651, 27695 } },
    { 23992, 28113, 28113, 1275, 1, 1, { 27748, 27793, 0, 27838, 27884, 27929, 27975, 28021, 28067 } },
    { 23992, 28473, 28473, 28482, 1, 2, { 28124, 28167, 0, 28210, 28254, 28297, 28341, 28385, 28429 } },
    { 23992, 28501, 28501, 24661, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23992, 28833, 28833, 28839, 1, 2, { 28508, 28548, 0, 28588, 28629, 28669, 28710, 28751, 28792 } },
    { 23992, 29135, 29135, 29139, 1, 3, { 28855, 28901, 0, 0, 0, 28947, 28994, 29041, 29088 } },
    { 23992, 8231, 8231, 7902, 1, 1, { 29168, 29207, 0, 29246, 29286, 29325, 29365, 29405, 29445 } },
    { 23992, 29485, 29485, 29490, 0, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23992, 29511, 29511, 29521, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 29548, 29555, 29555, 29563, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 29548, 29652, 29652, 29658, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 29688, 29695, 29695, 0, 0, -1, { 0, 0, 1246, 0, 0, 0, 0, 0, 0 } },
    { 29688, 29711, 29711, 0, 0, -1, { 0, 0, 1246, 0, 0, 0, 0, 0, 0 } },
    { 29688, 29727, 29727, 0, 0, -1, { 0, 0, 1246, 0, 0, 0, 0, 0, 0 } },
    { 29688, 29743, 29743, 0, 0, -1, { 0, 0, 1246, 0, 0, 0, 0, 0, 0 } },
    { 29688, 29759, 29759, 29766, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 29688, 23520, 23520, 29827, 1, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 29688, 8675, 8675, 29925, 1, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 30361, 30367, 30367, 6446, 1, 1, { 30020, 30062, 0, 30104, 30147, 30189, 30232, 30275, 30318 } },
    { 30361, 30708, 30708, 6446, 1, 1, { 30375, 30416, 0, 30457, 30499, 30540, 30582, 30624, 30666 } },
    { 30361, 6097, 6097, 30715, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 30361, 6430, 6430, 30715, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 30361, 6440, 6440, 30715, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 30361, 5471, 5471, 30715, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 30361, 7112, 7112, 30730, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 30361, 7136, 7136, 30759, 3, 4, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23271, 873, 873, 30790, 1, -1, { 533, 574, 0, 615, 657, 698, 740, 782, 824 } },
    { 23271, 1231, 1231, 30790, 1, -1, { 922, 960, 0, 998, 1037, 1075, 1114, 1153, 1192 } },
    { 23271, 31205, 31205, 31210, 1, -1, { 30888, 30927, 0, 30966, 31006, 31045, 31085, 31125, 31165 } },
    { 23271, 1605, 1605, 30790, 1, -1, { 1280, 1320, 0, 1360, 1401, 1441, 1482, 1523, 1564 } },
    { 23271, 1928, 1928, 30790, 1, -1, { 1611, 1650, 0, 1689, 1729, 1768, 1808, 1848, 1888 } },
    { 23271, 2242, 2242, 30790, 1, -1, { 1933, 1971, 0, 2009, 2048, 2086, 2125, 2164, 2203 } },
    { 23271, 31605, 31605, 31616, 1, 1, { 31240, 31285, 0, 31330, 31376, 31421, 31467, 31513, 31559 } },
    { 31620, 31627, 31627, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31620, 31967, 31967, 31972, 1, 1, { 31634, 31681, 31728, 0, 0, 31775, 31823, 31871, 31919 } },
    { 31620, 68, 68, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31620, 29555, 29555, 31972, 1, 1, { 31980, 32030, 32080, 0, 0, 32130, 32181, 32232, 32283 } },
    { 31620, 32709, 32709, 31972, 1, 1, { 32334, 32387, 32440, 0, 0, 32493, 32547, 32601, 32655 } },
    { 31620, 32720, 32720, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31620, 32727, 32727, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31620, 32731, 32731, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31620, 32738, 32738, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31620, 32745, 32745, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31620, 32757, 32757, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31620, 32769, 32769, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31620, 32781, 32781, 32792, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31620, 32818, 32818, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31620, 32829, 32829, 32836, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31620, 32849, 32849, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31620, 32855, 32855, 32860, 1, 2, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 31620, 32879, 32879, 32860, 1, 2, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 31620, 2850, 2850, 32888, 1, 2, { 15706, 15748, 0, 15832, 15875, 15917, 0, 15960, 16039 } },
    { 31620, 32904, 32904, 32888, 1, 2, { 15706, 15748, 0, 15832, 15875, 15917, 0, 15960, 16039 } },
    { 31620, 32912, 32912, 32919, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31620, 32943, 32943, 32955, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31620, 32960, 32960, 32971, 3, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31620, 32991, 32991, 33002, 1, 1, { 0, 0, 2921, 2970, 3020, 3069, 3119, 0, 0 } },
    { 31620, 33010, 33010, 33019, 1, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 31620, 13822, 13822, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33031, 33031, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33039, 33039, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33048, 33048, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33058, 33058, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33066, 33066, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33076, 33076, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33090, 33090, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33097, 33097, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33110, 33110, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33125, 33125, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33140, 33140, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33151, 33151, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33163, 33163, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33174, 33174, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33191, 33191, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33207, 33207, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33220, 33220, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33231, 33231, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33243, 33243, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33255, 33255, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33267, 33267, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33279, 33279, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33297, 33297, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33316, 33316, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33334, 33334, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33351, 33351, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33358, 33358, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33372, 33372, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33385, 33385, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33393, 33393, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33402, 33402, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33412, 33412, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33424, 33424, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33433, 33433, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33445, 33445, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33455, 33455, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33470, 33470, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33485, 33485, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33493, 33493, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33503, 33503, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33511, 33511, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33522, 33522, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33531, 33531, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33546, 33546, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33558, 33558, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33569, 33569, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33579, 33579, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33592, 33592, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33605, 33605, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33614, 33614, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33627, 33627, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33637, 33637, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33650, 33650, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33660, 33660, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33672, 33672, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33685, 33685, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33698, 33698, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33708, 33708, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33720, 33720, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33728, 33728, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33740, 33740, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33753, 33753, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 33767, 33767, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 34203, 34203, 34221, 1, 3, { 33779, 33839, 33899, 0, 0, 33959, 34020, 34081, 34142 } },
    { 33023, 34658, 34658, 34672, 1, 3, { 34262, 34318, 34374, 0, 0, 34430, 34487, 34544, 34601 } },
    { 33023, 34709, 34709, 34721, 2, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 34775, 34775, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 34793, 34793, 34801, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 35200, 35200, 31972, 1, 1, { 34811, 34859, 0, 34907, 34956, 35004, 35053, 35102, 35151 } },
    { 33023, 35603, 35603, 35617, 1, 1, { 35214, 35262, 0, 35310, 35359, 35407, 35456, 35505, 35554 } },
    { 33023, 36039, 36039, 35617, 1, 1, { 35626, 35677, 0, 35728, 35780, 35831, 35883, 35935, 35987 } },
    { 33023, 36056, 36056, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 36068, 36068, 36080, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 36497, 36497, 36512, 1, 1, { 36100, 36149, 0, 36198, 36248, 36297, 36347, 36397, 36447 } },
    { 33023, 36914, 36914, 36928, 1, 2, { 36525, 36573, 0, 36621, 36670, 36718, 36767, 36816, 36865 } },
    { 33023, 36958, 36958, 36972, 1, 2, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 33023, 36995, 36995, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 37014, 37014, 37020, 1, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 33023, 37022, 37022, 37020, 1, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 33023, 37028, 37028, 37043, 1, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 33023, 37052, 37052, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 37462, 37462, 37477, 1, 1, { 37065, 37114, 0, 37163, 37213, 37262, 37312, 37362, 37412 } },
    { 33023, 37842, 37842, 37852, 1, 1, { 37485, 37529, 0, 37573, 37618, 37662, 37707, 37752, 37797 } },
    { 33023, 37862, 37862, 37872, 1, 1, { 15706, 15748, 0, 15832, 15875, 15917, 0, 15960, 16039 } },
    { 33023, 37882, 37882, 37892, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 37917, 37917, 37927, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 37952, 37952, 37020, 1, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 33023, 37958, 37958, 37020, 1, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 33023, 37964, 37964, 33002, 1, 1, { 0, 0, 2921, 2970, 3020, 3069, 3119, 0, 0 } },
    { 33023, 31620, 31620, 37982, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33023, 38015, 38015, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 38367, 29555, 29555, 38375, 1, 1, { 38026, 38068, 0, 38110, 38153, 38195, 38238, 38281, 38324 } },
    { 13484, 13836, 13836, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13484, 38384, 38384, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13484, 13847, 13847, 38393, 1, 2, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 13484, 14225, 14225, 7136, 1, 1, { 38413, 38456, 0, 38499, 38543, 38586, 38630, 38674, 38718 } },
    { 13484, 14234, 14234, 14240, 1, 3, { 38762, 38802, 0, 38842, 38883, 38923, 38964, 39005, 39046 } },
    { 13484, 39441, 39441, 14623, 0, 2, { 39087, 39137, 0, 39187, 39238, 39288, 39339, 39390, 0 } },
    { 13484, 15019, 15019, 39710, 1, 1, { 39448, 39491, 0, 0, 0, 39534, 39578, 39622, 39666 } },
    { 13484, 15053, 15053, 15064, 0, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 13484, 15074, 15074, 14240, 1, 3, { 39717, 39756, 0, 39795, 39835, 39874, 39914, 39954, 39994 } },
    { 13484, 6097, 6097, 40034, 1, 1, { 5748, 5790, 0, 5832, 5875, 5917, 5960, 6003, 6046 } },
    { 13484, 40041, 40041, 40048, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13484, 40429, 40429, 10231, 1, 1, { 40057, 40103, 40149, 40195, 40242, 0, 40288, 40335, 40382 } },
    { 13484, 1231, 1231, 40440, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13484, 40451, 40451, 40440, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13484, 15079, 15079, 7136, 1, 1, { 38413, 38456, 0, 38499, 38543, 38586, 38630, 38674, 38718 } },
    { 13484, 15315, 15315, 14240, 1, 3, { 40460, 40500, 0, 40540, 40581, 40621, 40662, 40703, 40744 } },
    { 13484, 15321, 15321, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13484, 15329, 15329, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13484, 15337, 15337, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13484, 40785, 40785, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13484, 15345, 15345, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13484, 40795, 40795, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13484, 15353, 15353, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13484, 40808, 40808, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13484, 40818, 40818, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13484, 15361, 15361, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13484, 15369, 15369, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13484, 15377, 15377, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13484, 15683, 15683, 15688, 1, 1, { 40830, 40877, 40924, 0, 0, 40971, 41019, 41067, 41115 } },
    { 13484, 10947, 10947, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13484, 15694, 15694, 38393, 1, 2, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 13484, 15700, 15700, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13484, 16003, 16003, 16010, 0, 1, { 41163, 41204, 0, 41245, 41287, 41328, 41370, 41412, 0 } },
    { 13484, 16021, 16021, 16031, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13484, 6440, 6440, 40034, 1, 1, { 6105, 6145, 0, 6185, 6226, 6266, 6307, 6348, 6389 } },
    { 13484, 16082, 16082, 16092, 1, 1, { 41454, 41475, 0, 41496, 41518, 41539, 41561, 41583, 41605 } },
    { 13484, 16096, 16096, 16109, 1, 1, { 41627, 41674, 0, 41721, 41769, 41816, 41864, 41912, 41960 } },
    { 13484, 16111, 16111, 16109, 1, 1, { 42008, 42055, 0, 42102, 42150, 42197, 42245, 42293, 42341 } },
    { 13484, 16124, 16124, 42389, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13484, 16149, 16149, 14240, 1, 3, { 42408, 42448, 0, 42488, 42529, 42569, 42610, 42651, 42692 } },
    { 13484, 16155, 16155, 14240, 1, 3, { 42733, 42774, 0, 42815, 42857, 42898, 42940, 42982, 43024 } },
    { 13484, 16162, 16162, 38393, 1, 2, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 13484, 16168, 16168, 16092, 1, 1, { 41454, 41475, 0, 41496, 41518, 41539, 41561, 41583, 41605 } },
    { 13484, 16179, 16179, 16186, 0, 2, { 43066, 43095, 0, 43124, 43154, 43183, 43213, 43243, 0 } },
    { 13484, 16207, 16207, 16010, 0, 1, { 43273, 43314, 0, 43355, 43397, 43438, 43480, 43522, 0 } },
    { 13484, 7112, 7112, 16186, 0, 2, { 43066, 43095, 0, 43124, 43154, 43183, 43213, 43243, 0 } },
    { 13484, 16214, 16214, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13484, 16615, 16615, 43838, 1, 1, { 43564, 43609, 0, 0, 0, 43654, 43700, 43746, 43792 } },
    { 13484, 16652, 16652, 16010, 0, 1, { 43846, 43886, 0, 43926, 43967, 44007, 44048, 44089, 0 } },
    { 13484, 7136, 7136, 44130, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13484, 16658, 16658, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13484, 16667, 16667, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13484, 16673, 16673, 44507, 1, 1, { 44143, 44188, 44233, 44278, 44324, 0, 44369, 44415, 44461 } },
    { 13484, 16702, 16702, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13484, 16708, 16708, 16714, 1, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 44895, 44899, 44899, 1275, 1, 1, { 44514, 44561, 0, 44608, 44656, 44703, 44751, 44799, 44847 } },
    { 44895, 44942, 44942, 0, 0, -1, { 0, 0, 0, 44912, 0, 0, 0, 0, 0 } },
    { 44895, 44947, 44947, 0, 0, -1, { 0, 0, 1246, 0, 0, 0, 0, 0, 0 } },
    { 44895, 44958, 44958, 44963, 0, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 0 } },
    { 44895, 44973, 44973, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 44895, 44982, 44982, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 44895, 44989, 44989, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 44895, 44995, 44995, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 45002, 45013, 45013, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 45002, 8236, 8236, 45019, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 45002, 45034, 8236, 9729, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 45042, 45052, 45052, 45056, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 45042, 7907, 7907, 45062, 0, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 45042, 7917, 7917, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 45075, 45013, 45013, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 45075, 8236, 8236, 45019, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 45075, 45034, 8236, 9729, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13818, 14225, 14225, 23561, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13818, 14234, 14234, 23561, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13818, 15079, 15079, 23561, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13818, 15315, 15315, 23926, 1, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13818, 10947, 10947, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
};
