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
    { 5090, 5421, 5421, 5426, 1, 2, { 5104, 5143, 0, 5182, 5222, 5261, 5301, 5341, 5381 } },
    { 5090, 5442, 5442, 5448, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5090, 5462, 5462, 5469, 0, 4, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5090, 5530, 5530, 5537, 0, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5566, 5421, 5421, 5426, 1, 2, { 5104, 5143, 0, 5182, 5222, 5261, 5301, 5341, 5381 } },
    { 5566, 5442, 5442, 5448, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5566, 5577, 5577, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5566, 2850, 2850, 5582, 1, 4, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5626, 5634, 5634, 5647, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5626, 5666, 5666, 5647, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5626, 5678, 5678, 5647, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5626, 5693, 5693, 5647, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5626, 5707, 5707, 5647, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5626, 5715, 5715, 5647, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5626, 5724, 5724, 5647, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5626, 5732, 5732, 5647, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 6080, 6088, 6088, 5732, 1, 1, { 5739, 5781, 0, 5823, 5866, 5908, 5951, 5994, 6037 } },
    { 6080, 6421, 6421, 5732, 1, 1, { 6096, 6136, 0, 6176, 6217, 6257, 6298, 6339, 6380 } },
    { 6080, 6431, 6431, 5732, 1, 1, { 6096, 6136, 0, 6176, 6217, 6257, 6298, 6339, 6380 } },
    { 6080, 6437, 6437, 0, 0, -1, { 0, 0, 1246, 0, 0, 0, 0, 0, 0 } },
    { 6080, 5462, 5462, 5732, 1, 1, { 6445, 6486, 0, 6527, 6569, 6610, 6652, 6694, 6736 } },
    { 6080, 7103, 7103, 7109, 1, 2, { 6778, 6818, 0, 6858, 6899, 6939, 6980, 7021, 7062 } },
    { 6080, 7127, 7127, 7131, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7154, 68, 68, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7154, 74, 74, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7154, 81, 81, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7154, 438, 438, 446, 1, 2, { 97, 139, 0, 181, 224, 266, 309, 352, 395 } },
    { 7154, 7169, 7169, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7178, 68, 68, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7178, 438, 438, 446, 1, 2, { 97, 139, 0, 181, 224, 266, 309, 352, 395 } },
    { 7178, 7189, 7189, 7201, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7178, 460, 460, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7178, 469, 469, 479, 0, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 7178, 486, 486, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7178, 7212, 7212, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7178, 524, 524, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7222, 7231, 7231, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7222, 68, 68, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7222, 7554, 7554, 7559, 1, 1, { 7237, 7276, 0, 7315, 7355, 7394, 7434, 7474, 7514 } },
    { 7222, 873, 873, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7222, 7567, 7567, 0, 0, -1, { 0, 0, 1246, 0, 0, 0, 0, 0, 0 } },
    { 7222, 7888, 7888, 7559, 1, 1, { 7571, 7610, 0, 7649, 7689, 7728, 7768, 7808, 7848 } },
    { 7222, 1275, 1275, 0, 0, -1, { 0, 0, 1246, 0, 0, 0, 0, 0, 0 } },
    { 7222, 7893, 7893, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7222, 7898, 7898, 7903, 0, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 0 } },
    { 7222, 7908, 7908, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7222, 7917, 7917, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7222, 8204, 8204, 8209, 0, 1, { 7927, 7966, 0, 8005, 8045, 8084, 8124, 8164, 0 } },
    { 7222, 8222, 8222, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7222, 8227, 8227, 8233, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7222, 8617, 8617, 8628, 1, 1, { 8242, 8295, 8348, 0, 0, 8401, 8455, 8509, 8563 } },
    { 8635, 8646, 8646, 8655, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8635, 8660, 8660, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8635, 8666, 8666, 8671, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8635, 8680, 8680, 8691, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8733, 68, 68, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8733, 8741, 8741, 8746, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8733, 8755, 8755, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8733, 7898, 7898, 9036, 0, 1, { 0, 8759, 8798, 8837, 8877, 8916, 8956, 0, 8996 } },
    { 8733, 9348, 9348, 9036, 0, 1, { 0, 9043, 9086, 9129, 9173, 9216, 9260, 0, 9304 } },
    { 8733, 7908, 7908, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8733, 9357, 9357, 9364, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8733, 9379, 9379, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8733, 9390, 9390, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8733, 8227, 8227, 9720, 1, 1, { 9395, 9435, 0, 9475, 9516, 9556, 9597, 9638, 9679 } },
    { 9726, 1275, 1275, 0, 0, -1, { 0, 0, 1246, 0, 0, 0, 0, 0, 0 } },
    { 9726, 9357, 9357, 9734, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 9726, 8204, 8204, 7893, 1, 1, { 7927, 7966, 0, 8005, 8045, 8084, 8124, 8164, 9747 } },
    { 9726, 9787, 9787, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 9795, 7231, 7231, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 9795, 7567, 7567, 0, 0, -1, { 0, 0, 1246, 0, 0, 0, 0, 0, 0 } },
    { 9795, 1275, 1275, 0, 0, -1, { 0, 0, 1246, 0, 0, 0, 0, 0, 0 } },
    { 9795, 8204, 8204, 9806, 1, 1, { 7927, 7966, 0, 8005, 8045, 8084, 8124, 8164, 9747 } },
    { 9795, 8222, 8222, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10022, 10031, 10031, 0, 1, 1, { 0, 0, 9814, 9848, 9883, 9917, 9952, 0, 9987 } },
    { 10022, 10035, 10035, 0, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10022, 10042, 10042, 0, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10022, 10051, 10051, 10058, 1, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10022, 10091, 10091, 0, 1, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 10022, 10095, 10095, 0, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10022, 10100, 10100, 0, 1, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 10022, 10222, 10222, 0, 0, 1, { 10104, 10133, 0, 0, 0, 0, 0, 10162, 10192 } },
    { 10022, 10227, 10227, 0, 1, 1, { 10104, 10133, 0, 0, 0, 0, 0, 10162, 10192 } },
    { 10022, 10237, 10237, 0, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10022, 10487, 10487, 0, 1, 1, { 0, 0, 0, 10244, 10305, 10365, 0, 0, 10426 } },
    { 10022, 10491, 10491, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10022, 10498, 10498, 0, 1, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 10022, 5101, 5101, 0, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10022, 10502, 10502, 0, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10022, 10831, 10831, 0, 0, 1, { 0, 0, 0, 10508, 10589, 10669, 0, 0, 10750 } },
    { 10022, 10938, 10938, 0, 1, 1, { 10835, 10869, 0, 0, 0, 0, 0, 10903, 0 } },
    { 10022, 10942, 10942, 0, 0, 1, { 10104, 10133, 0, 0, 0, 0, 0, 10162, 10192 } },
    { 10022, 11224, 11224, 0, 0, 1, { 10947, 10986, 0, 11025, 11065, 11104, 11144, 11184, 0 } },
    { 10022, 11229, 10831, 0, 0, 1, { 0, 0, 0, 10508, 10589, 10669, 0, 0, 10750 } },
    { 10022, 11234, 11234, 0, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10022, 11238, 11238, 0, 1, -1, { 10104, 10133, 0, 0, 0, 0, 0, 10162, 10192 } },
    { 10022, 11242, 11242, 0, 1, -1, { 10104, 10133, 0, 0, 0, 0, 0, 10162, 10192 } },
    { 10022, 11246, 11246, 0, 1, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 10022, 11567, 11567, 11572, 1, 3, { 11250, 11289, 0, 11328, 11368, 11407, 11447, 11487, 11527 } },
    { 10022, 11998, 11998, 0, 1, 1, { 11601, 11650, 0, 11699, 11749, 11798, 11848, 11898, 11948 } },
    { 10022, 1928, 1928, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10022, 12002, 12002, 12006, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10022, 12025, 12025, 0, 1, 3, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 10022, 12395, 12395, 0, 1, 1, { 12031, 12076, 12121, 0, 12166, 12211, 12257, 12303, 12349 } },
    { 10022, 12785, 12785, 0, 1, 1, { 12405, 12452, 12499, 0, 12546, 12593, 12641, 12689, 12737 } },
    { 10022, 12923, 12923, 0, 1, 1, { 12797, 12828, 0, 0, 0, 0, 0, 12859, 12891 } },
    { 10022, 13182, 13182, 0, 1, 2, { 0, 0, 12932, 12973, 13015, 13056, 13098, 0, 13140 } },
    { 10022, 13462, 13462, 0, 1, 1, { 0, 0, 13188, 13233, 13279, 13324, 13370, 0, 13416 } },
    { 10022, 13468, 13468, 0, 1, 1, { 10104, 10133, 0, 0, 0, 0, 0, 10162, 10192 } },
    { 10022, 13475, 13475, 0, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10022, 13805, 13805, 0, 1, 2, { 13479, 13525, 13571, 0, 0, 13617, 13664, 13711, 13758 } },
    { 10022, 13809, 13809, 0, 0, 1, { 10104, 10133, 0, 0, 0, 0, 0, 10162, 10192 } },
    { 10022, 13813, 13813, 0, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10022, 13818, 13818, 0, 0, -1, { 10104, 10133, 0, 0, 0, 0, 0, 10162, 10192 } },
    { 13822, 13827, 13827, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13822, 13838, 13838, 13845, 1, 2, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 13822, 14216, 14216, 7127, 1, 1, { 0, 13865, 13923, 13981, 14040, 14098, 0, 0, 14157 } },
    { 13822, 14225, 14225, 14231, 1, 3, { 0, 13865, 13923, 13981, 14040, 14098, 0, 0, 14157 } },
    { 13822, 14607, 14607, 14614, 0, 2, { 14253, 14303, 0, 14353, 14404, 14454, 14505, 14556, 0 } },
    { 13822, 15010, 15010, 15019, 1, 3, { 14647, 14707, 14767, 0, 0, 14827, 0, 14888, 14949 } },
    { 13822, 15044, 15044, 15055, 0, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 0 } },
    { 13822, 15065, 15065, 14231, 1, 3, { 0, 13865, 13923, 13981, 14040, 14098, 0, 0, 14157 } },
    { 13822, 15070, 15070, 7127, 1, 1, { 0, 13865, 13923, 13981, 14040, 14098, 0, 0, 14157 } },
    { 13822, 10498, 10498, 15281, 0, 2, { 15074, 15108, 0, 15142, 15177, 15211, 0, 15246, 0 } },
    { 13822, 15306, 15306, 14231, 1, 3, { 0, 13865, 13923, 13981, 14040, 14098, 0, 0, 14157 } },
    { 13822, 15312, 15312, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13822, 15320, 15320, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13822, 15328, 15328, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13822, 15336, 15336, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13822, 15344, 15344, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13822, 15352, 15352, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13822, 15360, 15360, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13822, 15368, 15368, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13822, 15674, 15674, 15679, 1, 1, { 15376, 15435, 0, 0, 0, 15494, 0, 15554, 15614 } },
    { 13822, 10938, 10938, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13822, 15685, 15685, 13845, 1, 2, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 13822, 15691, 15691, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13822, 15994, 15994, 16001, 0, 1, { 15697, 15739, 15781, 15823, 15866, 15908, 0, 15951, 0 } },
    { 13822, 16012, 16012, 16022, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13822, 16073, 16073, 16083, 1, 1, { 15697, 15739, 15781, 15823, 15866, 15908, 0, 15951, 16030 } },
    { 13822, 16087, 16087, 16100, 1, 1, { 15697, 15739, 15781, 15823, 15866, 15908, 0, 15951, 16030 } },
    { 13822, 16102, 16102, 16100, 1, 1, { 15697, 15739, 15781, 15823, 15866, 15908, 0, 15951, 16030 } },
    { 13822, 16115, 16115, 16123, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13822, 16140, 16140, 14231, 1, 3, { 0, 13865, 13923, 13981, 14040, 14098, 0, 0, 14157 } },
    { 13822, 16146, 16146, 14231, 1, 3, { 0, 13865, 13923, 13981, 14040, 14098, 0, 0, 14157 } },
    { 13822, 16153, 16153, 13845, 1, 2, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 13822, 16159, 16159, 16083, 1, 1, { 15697, 15739, 15781, 15823, 15866, 15908, 0, 15951, 16030 } },
    { 13822, 16170, 16170, 16177, 0, 2, { 15697, 15739, 15781, 15823, 15866, 15908, 0, 15951, 0 } },
    { 13822, 16198, 16198, 16001, 0, 1, { 15697, 15739, 15781, 15823, 15866, 15908, 0, 15951, 0 } },
    { 13822, 7103, 7103, 16177, 0, 2, { 15697, 15739, 15781, 15823, 15866, 15908, 0, 15951, 0 } },
    { 13822, 16205, 16205, 16216, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13822, 16606, 16606, 16617, 1, 3, { 16231, 16293, 16355, 0, 0, 16417, 0, 16480, 16543 } },
    { 13822, 16643, 16643, 16001, 0, 1, { 15697, 15739, 15781, 15823, 15866, 15908, 0, 15951, 0 } },
    { 13822, 16649, 16649, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13822, 16658, 16658, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13822, 16664, 16664, 16674, 1, 2, { 15697, 15739, 15781, 15823, 15866, 15908, 0, 15951, 0 } },
    { 13822, 16693, 16693, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13822, 16699, 16699, 16705, 1, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 7231, 17053, 17053, 17060, 1, 1, { 16713, 16755, 16797, 16839, 16882, 16924, 0, 16967, 17010 } },
    { 7231, 17409, 17409, 15679, 1, 1, { 17062, 17111, 17160, 0, 0, 17209, 17259, 17309, 17359 } },
    { 7231, 17781, 17781, 17792, 1, 1, { 17416, 17461, 0, 17506, 17552, 17597, 17643, 17689, 17735 } },
    { 7231, 18119, 18119, 17792, 1, 1, { 17794, 17834, 0, 17874, 17915, 17955, 17996, 18037, 18078 } },
    { 7231, 18127, 18127, 18135, 1, 3, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 7231, 1231, 1231, 18164, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7231, 10498, 10498, 17060, 1, 1, { 18168, 18207, 18246, 18285, 18325, 18364, 0, 18404, 18444 } },
    { 7231, 18635, 18635, 18639, 0, 1, { 0, 18484, 0, 0, 0, 18521, 0, 18559, 18597 } },
    { 7231, 18978, 18978, 18984, 1, 3, { 18646, 18687, 18728, 18769, 18811, 18852, 0, 18894, 18936 } },
    { 7231, 19336, 19336, 19342, 1, 2, { 19004, 19045, 19086, 19127, 19169, 19210, 0, 19252, 19294 } },
    { 7231, 19699, 19699, 17060, 1, 1, { 19359, 19401, 19443, 19485, 19528, 19570, 0, 19613, 19656 } },
    { 7231, 19706, 19706, 19710, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 20123, 20135, 20135, 0, 1, 1, { 19722, 19766, 19810, 19854, 19899, 19943, 19988, 20033, 20078 } },
    { 20123, 20543, 20543, 0, 1, 1, { 20142, 20186, 20230, 20274, 20319, 20363, 20408, 20453, 20498 } },
    { 20123, 20951, 20951, 0, 1, 1, { 20550, 20594, 20638, 20682, 20727, 20771, 20816, 20861, 20906 } },
    { 20123, 21368, 21368, 0, 1, 1, { 20958, 21003, 21048, 21093, 21139, 21184, 21230, 21276, 21322 } },
    { 20123, 21376, 21376, 0, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 21387, 21392, 21392, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 21387, 21401, 21401, 21406, 0, 3, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 0 } },
    { 21387, 21440, 21440, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 21387, 21450, 21450, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 21387, 21454, 21454, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 21387, 21459, 21459, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 21387, 21469, 21469, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 21387, 21482, 21482, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10222, 8660, 8660, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10222, 14216, 14216, 21488, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10222, 7554, 7554, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10222, 1231, 1231, 21494, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10222, 15070, 15070, 21488, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10222, 21513, 21513, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10222, 21519, 21519, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10222, 10938, 10938, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10222, 21524, 21524, 21494, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10222, 21868, 21868, 21875, 1, 1, { 21528, 21570, 21612, 21654, 21697, 0, 21739, 21782, 21825 } },
    { 10222, 21881, 21881, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10222, 21887, 21887, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 21894, 11224, 11224, 21901, 0, 1, { 10947, 10986, 0, 11025, 11065, 11104, 11144, 11184, 0 } },
    { 22268, 22273, 22273, 9720, 1, 1, { 21911, 21955, 0, 21999, 22044, 22088, 22133, 22178, 22223 } },
    { 22268, 22283, 22283, 22293, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 22268, 22298, 22298, 22304, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 22268, 22330, 22330, 22336, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 22268, 22342, 22342, 22293, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 22349, 866, 866, 22356, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 22349, 22430, 22430, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 22349, 2657, 2657, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 22349, 22827, 22827, 22832, 0, -1, { 22444, 22486, 22528, 22570, 22613, 22655, 22698, 22741, 22784 } },
    { 22349, 22889, 22889, 8655, 1, 1, { 0, 0, 0, 0, 0, 0, 2783, 0, 0 } },
    { 22349, 23230, 23230, 23237, 1, 2, { 22897, 22938, 0, 22979, 23021, 23062, 23104, 23146, 23188 } },
    { 22349, 23262, 23262, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2828, 23473, 23473, 23479, 1, 1, { 23270, 23310, 0, 0, 0, 0, 23350, 23391, 23432 } },
    { 2828, 23484, 23484, 21881, 1, 2, { 0, 0, 0, 0, 0, 0, 2783, 0, 0 } },
    { 23494, 22298, 22298, 23498, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23494, 23511, 23511, 23515, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10942, 23545, 23545, 23552, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10942, 8660, 8660, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10942, 14216, 14216, 23552, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10942, 7554, 7554, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10942, 14225, 14225, 23552, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10942, 23904, 23904, 23911, 1, 1, { 23557, 23606, 23655, 0, 0, 23704, 23754, 23804, 23854 } },
    { 10942, 15070, 15070, 23552, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10942, 15306, 15306, 23917, 1, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10942, 23939, 23939, 23946, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10942, 10938, 10938, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10942, 21524, 21524, 23953, 0, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 10942, 23958, 23958, 23552, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10942, 23965, 23965, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10942, 23973, 23973, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23978, 3576, 3576, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23978, 5090, 5090, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23978, 5566, 5566, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23983, 7222, 7222, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23983, 24311, 24311, 7893, 1, 1, { 23986, 24026, 0, 24066, 24107, 24147, 24188, 24229, 24270 } },
    { 23983, 24626, 24626, 24630, 1, 2, { 24317, 24355, 0, 24393, 24432, 24470, 24509, 24548, 24587 } },
    { 23983, 7554, 7554, 24652, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23983, 24660, 24660, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23983, 24962, 24962, 24970, 0, 1, { 24664, 24706, 0, 24748, 24791, 24833, 24876, 24919, 0 } },
    { 23983, 25312, 25312, 7893, 1, 1, { 24979, 25020, 0, 25061, 25103, 25144, 25186, 25228, 25270 } },
    { 23983, 25652, 25652, 25659, 1, 2, { 25319, 25360, 0, 25401, 25443, 25484, 25526, 25568, 25610 } },
    { 23983, 25676, 25676, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23983, 26008, 26008, 7893, 1, 1, { 25683, 25723, 0, 25763, 25804, 25844, 25885, 25926, 25967 } },
    { 23983, 26347, 26347, 7893, 1, 1, { 26014, 26055, 0, 26096, 26138, 26179, 26221, 26263, 26305 } },
    { 23983, 26703, 26703, 26712, 1, 2, { 26354, 26397, 0, 26440, 26484, 26527, 26571, 26615, 26659 } },
    { 23983, 27028, 27028, 21901, 0, 1, { 26727, 26764, 0, 26801, 26839, 26876, 26914, 26952, 26990 } },
    { 23983, 27356, 27356, 27362, 1, 2, { 27031, 27071, 0, 27111, 27152, 27192, 27233, 27274, 27315 } },
    { 23983, 7888, 7888, 24652, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23983, 27730, 27730, 1275, 1, 1, { 27381, 27424, 0, 27467, 27511, 27554, 27598, 27642, 27686 } },
    { 23983, 28104, 28104, 1275, 1, 1, { 27739, 27784, 0, 27829, 27875, 27920, 27966, 28012, 28058 } },
    { 23983, 28464, 28464, 28473, 1, 2, { 28115, 28158, 0, 28201, 28245, 28288, 28332, 28376, 28420 } },
    { 23983, 28492, 28492, 24652, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23983, 28824, 28824, 28830, 1, 2, { 28499, 28539, 0, 28579, 28620, 28660, 28701, 28742, 28783 } },
    { 23983, 29126, 29126, 29130, 1, 3, { 28846, 28892, 0, 0, 0, 28938, 28985, 29032, 29079 } },
    { 23983, 8222, 8222, 7893, 1, 1, { 29159, 29198, 0, 29237, 29277, 29316, 29356, 29396, 29436 } },
    { 23983, 29476, 29476, 29481, 0, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23983, 29502, 29502, 29512, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 29539, 29546, 29546, 29554, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 29539, 29643, 29643, 29649, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 29679, 29686, 29686, 0, 0, -1, { 0, 0, 1246, 0, 0, 0, 0, 0, 0 } },
    { 29679, 29702, 29702, 0, 0, -1, { 0, 0, 1246, 0, 0, 0, 0, 0, 0 } },
    { 29679, 29718, 29718, 0, 0, -1, { 0, 0, 1246, 0, 0, 0, 0, 0, 0 } },
    { 29679, 29734, 29734, 0, 0, -1, { 0, 0, 1246, 0, 0, 0, 0, 0, 0 } },
    { 29679, 29750, 29750, 29757, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 29679, 23511, 23511, 29818, 1, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 29679, 8666, 8666, 29916, 1, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 30352, 30358, 30358, 6437, 1, 1, { 30011, 30053, 0, 30095, 30138, 30180, 30223, 30266, 30309 } },
    { 30352, 30699, 30699, 6437, 1, 1, { 30366, 30407, 0, 30448, 30490, 30531, 30573, 30615, 30657 } },
    { 30352, 6088, 6088, 30706, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 30352, 6421, 6421, 30706, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 30352, 6431, 6431, 30706, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 30352, 5462, 5462, 30706, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 30352, 7103, 7103, 30721, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 30352, 7127, 7127, 30750, 3, 4, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23262, 873, 873, 30781, 1, -1, { 533, 574, 0, 615, 657, 698, 740, 782, 824 } },
    { 23262, 1231, 1231, 30781, 1, -1, { 922, 960, 0, 998, 1037, 1075, 1114, 1153, 1192 } },
    { 23262, 31196, 31196, 31201, 1, -1, { 30879, 30918, 0, 30957, 30997, 31036, 31076, 31116, 31156 } },
    { 23262, 1605, 1605, 30781, 1, -1, { 1280, 1320, 0, 1360, 1401, 1441, 1482, 1523, 1564 } },
    { 23262, 1928, 1928, 30781, 1, -1, { 1611, 1650, 0, 1689, 1729, 1768, 1808, 1848, 1888 } },
    { 23262, 2242, 2242, 30781, 1, -1, { 1933, 1971, 0, 2009, 2048, 2086, 2125, 2164, 2203 } },
    { 23262, 31596, 31596, 31607, 1, 1, { 31231, 31276, 0, 31321, 31367, 31412, 31458, 31504, 31550 } },
    { 31611, 31618, 31618, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31611, 31958, 31958, 31963, 1, 1, { 31625, 31672, 31719, 0, 0, 31766, 31814, 31862, 31910 } },
    { 31611, 68, 68, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31611, 29546, 29546, 31963, 1, 1, { 31971, 32021, 32071, 0, 0, 32121, 32172, 32223, 32274 } },
    { 31611, 32700, 32700, 31963, 1, 1, { 32325, 32378, 32431, 0, 0, 32484, 32538, 32592, 32646 } },
    { 31611, 32711, 32711, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31611, 32718, 32718, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31611, 32722, 32722, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31611, 32729, 32729, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31611, 32736, 32736, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31611, 32748, 32748, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31611, 32760, 32760, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31611, 32772, 32772, 32783, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31611, 32809, 32809, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31611, 32820, 32820, 32827, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31611, 32840, 32840, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31611, 32846, 32846, 32851, 1, 2, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 31611, 32870, 32870, 32851, 1, 2, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 31611, 2850, 2850, 32879, 1, 2, { 15697, 15739, 0, 15823, 15866, 15908, 0, 15951, 16030 } },
    { 31611, 32895, 32895, 32879, 1, 2, { 15697, 15739, 0, 15823, 15866, 15908, 0, 15951, 16030 } },
    { 31611, 32903, 32903, 32910, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31611, 32934, 32934, 32946, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31611, 32951, 32951, 32962, 3, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31611, 32982, 32982, 32993, 1, 1, { 0, 0, 2921, 2970, 3020, 3069, 3119, 0, 0 } },
    { 31611, 33001, 33001, 33010, 1, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 31611, 13813, 13813, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33022, 33022, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33030, 33030, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33039, 33039, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33049, 33049, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33057, 33057, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33067, 33067, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33081, 33081, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33088, 33088, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33101, 33101, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33116, 33116, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33131, 33131, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33142, 33142, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33154, 33154, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33165, 33165, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33182, 33182, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33198, 33198, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33211, 33211, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33222, 33222, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33234, 33234, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33246, 33246, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33258, 33258, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33270, 33270, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33288, 33288, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33307, 33307, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33325, 33325, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33342, 33342, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33349, 33349, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33363, 33363, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33376, 33376, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33384, 33384, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33393, 33393, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33403, 33403, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33415, 33415, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33424, 33424, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33436, 33436, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33446, 33446, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33461, 33461, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33476, 33476, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33484, 33484, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33494, 33494, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33502, 33502, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33513, 33513, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33522, 33522, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33537, 33537, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33549, 33549, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33560, 33560, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33570, 33570, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33583, 33583, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33596, 33596, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33605, 33605, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33618, 33618, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33628, 33628, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33641, 33641, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33651, 33651, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33663, 33663, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33676, 33676, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33689, 33689, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33699, 33699, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33711, 33711, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33719, 33719, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33731, 33731, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33744, 33744, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 33758, 33758, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 34194, 34194, 34212, 1, 3, { 33770, 33830, 33890, 0, 0, 33950, 34011, 34072, 34133 } },
    { 33014, 34649, 34649, 34663, 1, 3, { 34253, 34309, 34365, 0, 0, 34421, 34478, 34535, 34592 } },
    { 33014, 34700, 34700, 34712, 2, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 34766, 34766, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 34784, 34784, 34792, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 35191, 35191, 31963, 1, 1, { 34802, 34850, 0, 34898, 34947, 34995, 35044, 35093, 35142 } },
    { 33014, 35594, 35594, 35608, 1, 1, { 35205, 35253, 0, 35301, 35350, 35398, 35447, 35496, 35545 } },
    { 33014, 36030, 36030, 35608, 1, 1, { 35617, 35668, 0, 35719, 35771, 35822, 35874, 35926, 35978 } },
    { 33014, 36047, 36047, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 36059, 36059, 36071, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 36488, 36488, 36503, 1, 1, { 36091, 36140, 0, 36189, 36239, 36288, 36338, 36388, 36438 } },
    { 33014, 36905, 36905, 36919, 1, 2, { 36516, 36564, 0, 36612, 36661, 36709, 36758, 36807, 36856 } },
    { 33014, 36949, 36949, 36963, 1, 2, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 33014, 36986, 36986, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 37005, 37005, 37011, 1, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 33014, 37013, 37013, 37011, 1, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 33014, 37019, 37019, 37034, 1, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 33014, 37043, 37043, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 37453, 37453, 37468, 1, 1, { 37056, 37105, 0, 37154, 37204, 37253, 37303, 37353, 37403 } },
    { 33014, 37833, 37833, 37843, 1, 1, { 37476, 37520, 0, 37564, 37609, 37653, 37698, 37743, 37788 } },
    { 33014, 37853, 37853, 37863, 1, 1, { 15697, 15739, 0, 15823, 15866, 15908, 0, 15951, 16030 } },
    { 33014, 37873, 37873, 37883, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 37908, 37908, 37918, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 37943, 37943, 37011, 1, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 33014, 37949, 37949, 37011, 1, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 33014, 37955, 37955, 32993, 1, 1, { 0, 0, 2921, 2970, 3020, 3069, 3119, 0, 0 } },
    { 33014, 31611, 31611, 37973, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33014, 38006, 38006, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 38358, 29546, 29546, 38366, 1, 1, { 38017, 38059, 0, 38101, 38144, 38186, 38229, 38272, 38315 } },
    { 13475, 13827, 13827, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13475, 38375, 38375, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13475, 13838, 13838, 38384, 1, 2, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 13475, 14216, 14216, 7127, 1, 1, { 38404, 38447, 0, 38490, 38534, 38577, 38621, 38665, 38709 } },
    { 13475, 14225, 14225, 14231, 1, 3, { 38753, 38793, 0, 38833, 38874, 38914, 38955, 38996, 39037 } },
    { 13475, 39432, 39432, 14614, 0, 2, { 39078, 39128, 0, 39178, 39229, 39279, 39330, 39381, 0 } },
    { 13475, 15010, 15010, 39701, 1, 1, { 39439, 39482, 0, 0, 0, 39525, 39569, 39613, 39657 } },
    { 13475, 15044, 15044, 15055, 0, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 13475, 15065, 15065, 14231, 1, 3, { 39708, 39747, 0, 39786, 39826, 39865, 39905, 39945, 39985 } },
    { 13475, 6088, 6088, 40025, 1, 1, { 5739, 5781, 0, 5823, 5866, 5908, 5951, 5994, 6037 } },
    { 13475, 40032, 40032, 40039, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13475, 40420, 40420, 10222, 1, 1, { 40048, 40094, 40140, 40186, 40233, 0, 40279, 40326, 40373 } },
    { 13475, 1231, 1231, 40431, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13475, 40442, 40442, 40431, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13475, 15070, 15070, 7127, 1, 1, { 38404, 38447, 0, 38490, 38534, 38577, 38621, 38665, 38709 } },
    { 13475, 15306, 15306, 14231, 1, 3, { 40451, 40491, 0, 40531, 40572, 40612, 40653, 40694, 40735 } },
    { 13475, 15312, 15312, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13475, 15320, 15320, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13475, 15328, 15328, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13475, 40776, 40776, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13475, 15336, 15336, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13475, 40786, 40786, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13475, 15344, 15344, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13475, 40799, 40799, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13475, 40809, 40809, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13475, 15352, 15352, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13475, 15360, 15360, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13475, 15368, 15368, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13475, 15674, 15674, 15679, 1, 1, { 40821, 40868, 40915, 0, 0, 40962, 41010, 41058, 41106 } },
    { 13475, 10938, 10938, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13475, 15685, 15685, 38384, 1, 2, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 13475, 15691, 15691, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13475, 15994, 15994, 16001, 0, 1, { 41154, 41195, 0, 41236, 41278, 41319, 41361, 41403, 0 } },
    { 13475, 16012, 16012, 16022, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13475, 6431, 6431, 40025, 1, 1, { 6096, 6136, 0, 6176, 6217, 6257, 6298, 6339, 6380 } },
    { 13475, 16073, 16073, 16083, 1, 1, { 41445, 41466, 0, 41487, 41509, 41530, 41552, 41574, 41596 } },
    { 13475, 16087, 16087, 16100, 1, 1, { 41618, 41665, 0, 41712, 41760, 41807, 41855, 41903, 41951 } },
    { 13475, 16102, 16102, 16100, 1, 1, { 41999, 42046, 0, 42093, 42141, 42188, 42236, 42284, 42332 } },
    { 13475, 16115, 16115, 42380, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13475, 16140, 16140, 14231, 1, 3, { 42399, 42439, 0, 42479, 42520, 42560, 42601, 42642, 42683 } },
    { 13475, 16146, 16146, 14231, 1, 3, { 42724, 42765, 0, 42806, 42848, 42889, 42931, 42973, 43015 } },
    { 13475, 16153, 16153, 38384, 1, 2, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 13475, 16159, 16159, 16083, 1, 1, { 41445, 41466, 0, 41487, 41509, 41530, 41552, 41574, 41596 } },
    { 13475, 16170, 16170, 16177, 0, 2, { 43057, 43086, 0, 43115, 43145, 43174, 43204, 43234, 0 } },
    { 13475, 16198, 16198, 16001, 0, 1, { 43264, 43305, 0, 43346, 43388, 43429, 43471, 43513, 0 } },
    { 13475, 7103, 7103, 16177, 0, 2, { 43057, 43086, 0, 43115, 43145, 43174, 43204, 43234, 0 } },
    { 13475, 16205, 16205, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13475, 16606, 16606, 43829, 1, 1, { 43555, 43600, 0, 0, 0, 43645, 43691, 43737, 43783 } },
    { 13475, 16643, 16643, 16001, 0, 1, { 43837, 43877, 0, 43917, 43958, 43998, 44039, 44080, 0 } },
    { 13475, 7127, 7127, 44121, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13475, 16649, 16649, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13475, 16658, 16658, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13475, 16664, 16664, 44498, 1, 1, { 44134, 44179, 44224, 44269, 44315, 0, 44360, 44406, 44452 } },
    { 13475, 16693, 16693, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13475, 16699, 16699, 16705, 1, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 44886, 44890, 44890, 1275, 1, 1, { 44505, 44552, 0, 44599, 44647, 44694, 44742, 44790, 44838 } },
    { 44886, 44933, 44933, 0, 0, -1, { 0, 0, 0, 44903, 0, 0, 0, 0, 0 } },
    { 44886, 44938, 44938, 0, 0, -1, { 0, 0, 1246, 0, 0, 0, 0, 0, 0 } },
    { 44886, 44949, 44949, 44954, 0, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 0 } },
    { 44886, 44964, 44964, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 44886, 44973, 44973, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 44886, 44980, 44980, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 44886, 44986, 44986, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 44993, 45004, 45004, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 44993, 8227, 8227, 45010, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 44993, 45025, 8227, 9720, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 45033, 45043, 45043, 45047, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 45033, 7898, 7898, 45053, 0, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 45033, 7908, 7908, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 45066, 45004, 45004, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 45066, 8227, 8227, 45010, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 45066, 45025, 8227, 9720, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13809, 14216, 14216, 23552, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13809, 14225, 14225, 23552, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13809, 15070, 15070, 23552, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13809, 15306, 15306, 23917, 1, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13809, 10938, 10938, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
};
