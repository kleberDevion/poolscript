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
    { 388, 395, 395, 402, 1, 4, { 55, 96, 0, 137, 179, 220, 262, 304, 346 } },
    { 388, 753, 753, 402, 1, 4, { 444, 482, 0, 520, 559, 597, 636, 675, 714 } },
    { 388, 757, 757, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 388, 797, 797, 0, 0, -1, { 0, 0, 768, 0, 0, 0, 0, 0, 0 } },
    { 388, 1127, 1127, 402, 1, 4, { 802, 842, 0, 882, 923, 963, 1004, 1045, 1086 } },
    { 388, 1450, 1450, 402, 1, 4, { 1133, 1172, 0, 1211, 1251, 1290, 1330, 1370, 1410 } },
    { 388, 1764, 1764, 402, 1, 4, { 1455, 1493, 0, 1531, 1570, 1608, 1647, 1686, 1725 } },
    { 388, 2093, 2093, 2099, 1, 5, { 1768, 1808, 0, 1848, 1889, 1929, 1970, 2011, 2052 } },
    { 388, 2154, 2154, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 388, 2168, 2168, 0, 0, -1, { 0, 0, 768, 0, 0, 0, 0, 0, 0 } },
    { 2179, 2194, 2194, 2201, 2, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2179, 2288, 2288, 2295, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2179, 2350, 2350, 2355, 1, 2, { 0, 0, 0, 0, 0, 0, 2305, 0, 0 } },
    { 2179, 2372, 2372, 2377, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2179, 2741, 2741, 2748, 1, 1, { 0, 2394, 2443, 2492, 2542, 2591, 2641, 0, 2691 } },
    { 2179, 2782, 2782, 0, 0, -1, { 2753, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 3127, 3139, 3139, 3146, 1, 1, { 2794, 2835, 0, 2876, 2918, 2959, 3001, 3043, 3085 } },
    { 3127, 3482, 3482, 3487, 1, 2, { 3165, 3204, 0, 3243, 3283, 3322, 3362, 3402, 3442 } },
    { 3127, 3883, 3883, 3895, 1, 1, { 3510, 3556, 0, 3602, 3649, 3695, 3742, 3789, 3836 } },
    { 3127, 3904, 3904, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 3127, 4259, 4259, 4267, 1, 1, { 3918, 3960, 0, 4002, 4045, 4087, 4130, 4173, 4216 } },
    { 3127, 4631, 4631, 3895, 1, 1, { 4274, 4318, 0, 4362, 4407, 4451, 4496, 4541, 4586 } },
    { 4641, 3482, 3482, 4652, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 4641, 4655, 4655, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 4641, 4978, 4978, 4983, 1, 2, { 4661, 4700, 0, 4739, 4779, 4818, 4858, 4898, 4938 } },
    { 4641, 4999, 4999, 5005, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 4641, 5019, 5019, 5026, 0, 4, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 4641, 5087, 5087, 5094, 0, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5123, 4978, 4978, 4983, 1, 2, { 4661, 4700, 0, 4739, 4779, 4818, 4858, 4898, 4938 } },
    { 5123, 4999, 4999, 5005, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5123, 5134, 5134, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5123, 2372, 2372, 5139, 1, 4, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5183, 5191, 5191, 5204, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5183, 5223, 5223, 5204, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5183, 5235, 5235, 5204, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5183, 5250, 5250, 5204, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5183, 5264, 5264, 5204, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5183, 5272, 5272, 5204, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5183, 5281, 5281, 5204, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5183, 5289, 5289, 5204, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 5637, 5645, 5645, 5289, 1, 1, { 5296, 5338, 0, 5380, 5423, 5465, 5508, 5551, 5594 } },
    { 5637, 5978, 5978, 5289, 1, 1, { 5653, 5693, 0, 5733, 5774, 5814, 5855, 5896, 5937 } },
    { 5637, 5988, 5988, 5289, 1, 1, { 5653, 5693, 0, 5733, 5774, 5814, 5855, 5896, 5937 } },
    { 5637, 5994, 5994, 0, 0, -1, { 0, 0, 768, 0, 0, 0, 0, 0, 0 } },
    { 5637, 5019, 5019, 5289, 1, 1, { 6002, 6043, 0, 6084, 6126, 6167, 6209, 6251, 6293 } },
    { 5637, 6660, 6660, 6666, 1, 2, { 6335, 6375, 0, 6415, 6456, 6496, 6537, 6578, 6619 } },
    { 5637, 6684, 6684, 6688, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 6711, 4655, 4655, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 6711, 6726, 6726, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 6711, 6733, 6733, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 6711, 7081, 7081, 7089, 1, 2, { 6740, 6782, 0, 6824, 6867, 6909, 6952, 6995, 7038 } },
    { 6711, 7103, 7103, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7112, 4655, 4655, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7112, 7081, 7081, 7089, 1, 2, { 6740, 6782, 0, 6824, 6867, 6909, 6952, 6995, 7038 } },
    { 7112, 7123, 7123, 7135, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7112, 7146, 7146, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7112, 7155, 7155, 7165, 0, 1, { 0, 2394, 2443, 2492, 2542, 2591, 2641, 0, 2691 } },
    { 7112, 7172, 7172, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7112, 7181, 7181, 0, 0, -1, { 2753, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7112, 7191, 7191, 0, 0, -1, { 2753, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7200, 7209, 7209, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7200, 4655, 4655, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7200, 7532, 7532, 7537, 1, 1, { 7215, 7254, 0, 7293, 7333, 7372, 7412, 7452, 7492 } },
    { 7200, 395, 395, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7200, 7545, 7545, 0, 0, -1, { 0, 0, 768, 0, 0, 0, 0, 0, 0 } },
    { 7200, 7866, 7866, 7537, 1, 1, { 7549, 7588, 0, 7627, 7667, 7706, 7746, 7786, 7826 } },
    { 7200, 797, 797, 0, 0, -1, { 0, 0, 768, 0, 0, 0, 0, 0, 0 } },
    { 7200, 7871, 7871, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7200, 7876, 7876, 7881, 0, 1, { 0, 2394, 2443, 2492, 2542, 2591, 2641, 0, 0 } },
    { 7200, 7886, 7886, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7200, 7895, 7895, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7200, 8182, 8182, 8187, 0, 1, { 7905, 7944, 0, 7983, 8023, 8062, 8102, 8142, 0 } },
    { 7200, 8200, 8200, 0, 0, -1, { 2753, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7200, 8205, 8205, 8211, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7200, 8595, 8595, 8606, 1, 1, { 8220, 8273, 8326, 0, 0, 8379, 8433, 8487, 8541 } },
    { 8613, 8624, 8624, 8633, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8613, 8638, 8638, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8613, 8644, 8644, 8649, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8613, 8658, 8658, 8669, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8711, 4655, 4655, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8711, 8719, 8719, 8724, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8711, 8733, 8733, 0, 0, -1, { 2753, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8711, 7876, 7876, 9014, 0, 1, { 0, 8737, 8776, 8815, 8855, 8894, 8934, 0, 8974 } },
    { 8711, 9326, 9326, 9014, 0, 1, { 0, 9021, 9064, 9107, 9151, 9194, 9238, 0, 9282 } },
    { 8711, 7886, 7886, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8711, 9335, 9335, 9342, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8711, 9357, 9357, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8711, 9368, 9368, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8711, 8205, 8205, 9698, 1, 1, { 9373, 9413, 0, 9453, 9494, 9534, 9575, 9616, 9657 } },
    { 9704, 797, 797, 0, 0, -1, { 0, 0, 768, 0, 0, 0, 0, 0, 0 } },
    { 9704, 9335, 9335, 9712, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 9704, 8182, 8182, 7871, 1, 1, { 7905, 7944, 0, 7983, 8023, 8062, 8102, 8142, 9725 } },
    { 9704, 9765, 9765, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 9773, 7209, 7209, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 9773, 7545, 7545, 0, 0, -1, { 0, 0, 768, 0, 0, 0, 0, 0, 0 } },
    { 9773, 797, 797, 0, 0, -1, { 0, 0, 768, 0, 0, 0, 0, 0, 0 } },
    { 9773, 8182, 8182, 9784, 1, 1, { 7905, 7944, 0, 7983, 8023, 8062, 8102, 8142, 9725 } },
    { 9773, 8200, 8200, 0, 0, -1, { 2753, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10000, 10009, 10009, 0, 1, 1, { 0, 0, 9792, 9826, 9861, 9895, 9930, 0, 9965 } },
    { 10000, 10013, 10013, 0, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10000, 10020, 10020, 0, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10000, 10029, 10029, 10036, 1, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10000, 10069, 10069, 0, 1, 1, { 0, 2394, 2443, 2492, 2542, 2591, 2641, 0, 2691 } },
    { 10000, 10073, 10073, 0, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10000, 10078, 10078, 0, 1, 1, { 0, 2394, 2443, 2492, 2542, 2591, 2641, 0, 2691 } },
    { 10000, 10200, 10200, 0, 0, 1, { 10082, 10111, 0, 0, 0, 0, 0, 10140, 10170 } },
    { 10000, 10205, 10205, 0, 1, 1, { 10082, 10111, 0, 0, 0, 0, 0, 10140, 10170 } },
    { 10000, 10215, 10215, 0, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10000, 10465, 10465, 0, 1, 1, { 0, 0, 0, 10222, 10283, 10343, 0, 0, 10404 } },
    { 10000, 10469, 10469, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10000, 10476, 10476, 0, 1, 1, { 0, 2394, 2443, 2492, 2542, 2591, 2641, 0, 2691 } },
    { 10000, 4652, 4652, 0, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10000, 10480, 10480, 0, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10000, 10809, 10809, 0, 0, 1, { 0, 0, 0, 10486, 10567, 10647, 0, 0, 10728 } },
    { 10000, 10916, 10916, 0, 1, 1, { 10813, 10847, 0, 0, 0, 0, 0, 10881, 0 } },
    { 10000, 10920, 10920, 0, 0, 1, { 10082, 10111, 0, 0, 0, 0, 0, 10140, 10170 } },
    { 10000, 11202, 11202, 0, 0, 1, { 10925, 10964, 0, 11003, 11043, 11082, 11122, 11162, 0 } },
    { 10000, 11207, 10809, 0, 0, 1, { 0, 0, 0, 10486, 10567, 10647, 0, 0, 10728 } },
    { 10000, 11212, 11212, 0, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10000, 11216, 11216, 0, 1, -1, { 10082, 10111, 0, 0, 0, 0, 0, 10140, 10170 } },
    { 10000, 11220, 11220, 0, 1, -1, { 10082, 10111, 0, 0, 0, 0, 0, 10140, 10170 } },
    { 10000, 11224, 11224, 0, 1, 1, { 0, 2394, 2443, 2492, 2542, 2591, 2641, 0, 2691 } },
    { 10000, 11545, 11545, 11550, 1, 3, { 11228, 11267, 0, 11306, 11346, 11385, 11425, 11465, 11505 } },
    { 10000, 11976, 11976, 0, 1, 1, { 11579, 11628, 0, 11677, 11727, 11776, 11826, 11876, 11926 } },
    { 10000, 1450, 1450, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10000, 11980, 11980, 11984, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10000, 12003, 12003, 0, 1, 3, { 0, 2394, 2443, 2492, 2542, 2591, 2641, 0, 2691 } },
    { 10000, 12373, 12373, 0, 1, 1, { 12009, 12054, 12099, 0, 12144, 12189, 12235, 12281, 12327 } },
    { 10000, 12763, 12763, 0, 1, 1, { 12383, 12430, 12477, 0, 12524, 12571, 12619, 12667, 12715 } },
    { 10000, 12901, 12901, 0, 1, 1, { 12775, 12806, 0, 0, 0, 0, 0, 12837, 12869 } },
    { 10000, 13160, 13160, 0, 1, 2, { 0, 0, 12910, 12951, 12993, 13034, 13076, 0, 13118 } },
    { 10000, 13440, 13440, 0, 1, 1, { 0, 0, 13166, 13211, 13257, 13302, 13348, 0, 13394 } },
    { 10000, 13446, 13446, 0, 1, 1, { 10082, 10111, 0, 0, 0, 0, 0, 10140, 10170 } },
    { 10000, 13453, 13453, 0, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10000, 13783, 13783, 0, 1, 2, { 13457, 13503, 13549, 0, 0, 13595, 13642, 13689, 13736 } },
    { 10000, 13787, 13787, 0, 0, 1, { 10082, 10111, 0, 0, 0, 0, 0, 10140, 10170 } },
    { 10000, 13791, 13791, 0, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10000, 13796, 13796, 0, 0, -1, { 10082, 10111, 0, 0, 0, 0, 0, 10140, 10170 } },
    { 13800, 13805, 13805, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13800, 13816, 13816, 13823, 1, 2, { 0, 2394, 2443, 2492, 2542, 2591, 2641, 0, 2691 } },
    { 13800, 14194, 14194, 6684, 1, 1, { 0, 13843, 13901, 13959, 14018, 14076, 0, 0, 14135 } },
    { 13800, 14203, 14203, 14209, 1, 3, { 0, 13843, 13901, 13959, 14018, 14076, 0, 0, 14135 } },
    { 13800, 14585, 14585, 14592, 0, 2, { 14231, 14281, 0, 14331, 14382, 14432, 14483, 14534, 0 } },
    { 13800, 14988, 14988, 14997, 1, 3, { 14625, 14685, 14745, 0, 0, 14805, 0, 14866, 14927 } },
    { 13800, 15022, 15022, 15033, 0, 1, { 0, 2394, 2443, 2492, 2542, 2591, 2641, 0, 0 } },
    { 13800, 15043, 15043, 14209, 1, 3, { 0, 13843, 13901, 13959, 14018, 14076, 0, 0, 14135 } },
    { 13800, 15048, 15048, 6684, 1, 1, { 0, 13843, 13901, 13959, 14018, 14076, 0, 0, 14135 } },
    { 13800, 10476, 10476, 15259, 0, 2, { 15052, 15086, 0, 15120, 15155, 15189, 0, 15224, 0 } },
    { 13800, 15284, 15284, 14209, 1, 3, { 0, 13843, 13901, 13959, 14018, 14076, 0, 0, 14135 } },
    { 13800, 15290, 15290, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13800, 15298, 15298, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13800, 15306, 15306, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13800, 15314, 15314, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13800, 15322, 15322, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13800, 15330, 15330, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13800, 15338, 15338, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13800, 15346, 15346, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13800, 15652, 15652, 15657, 1, 1, { 15354, 15413, 0, 0, 0, 15472, 0, 15532, 15592 } },
    { 13800, 10916, 10916, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13800, 15663, 15663, 13823, 1, 2, { 0, 2394, 2443, 2492, 2542, 2591, 2641, 0, 2691 } },
    { 13800, 15669, 15669, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13800, 15972, 15972, 15979, 0, 1, { 15675, 15717, 15759, 15801, 15844, 15886, 0, 15929, 0 } },
    { 13800, 15990, 15990, 16000, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13800, 16051, 16051, 16061, 1, 1, { 15675, 15717, 15759, 15801, 15844, 15886, 0, 15929, 16008 } },
    { 13800, 16065, 16065, 16078, 1, 1, { 15675, 15717, 15759, 15801, 15844, 15886, 0, 15929, 16008 } },
    { 13800, 16080, 16080, 16078, 1, 1, { 15675, 15717, 15759, 15801, 15844, 15886, 0, 15929, 16008 } },
    { 13800, 16093, 16093, 16101, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13800, 16118, 16118, 14209, 1, 3, { 0, 13843, 13901, 13959, 14018, 14076, 0, 0, 14135 } },
    { 13800, 16124, 16124, 14209, 1, 3, { 0, 13843, 13901, 13959, 14018, 14076, 0, 0, 14135 } },
    { 13800, 16131, 16131, 13823, 1, 2, { 0, 2394, 2443, 2492, 2542, 2591, 2641, 0, 2691 } },
    { 13800, 16137, 16137, 16061, 1, 1, { 15675, 15717, 15759, 15801, 15844, 15886, 0, 15929, 16008 } },
    { 13800, 16148, 16148, 16155, 0, 2, { 15675, 15717, 15759, 15801, 15844, 15886, 0, 15929, 0 } },
    { 13800, 16176, 16176, 15979, 0, 1, { 15675, 15717, 15759, 15801, 15844, 15886, 0, 15929, 0 } },
    { 13800, 6660, 6660, 16155, 0, 2, { 15675, 15717, 15759, 15801, 15844, 15886, 0, 15929, 0 } },
    { 13800, 16183, 16183, 16194, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13800, 16584, 16584, 16595, 1, 3, { 16209, 16271, 16333, 0, 0, 16395, 0, 16458, 16521 } },
    { 13800, 16621, 16621, 15979, 0, 1, { 15675, 15717, 15759, 15801, 15844, 15886, 0, 15929, 0 } },
    { 13800, 16627, 16627, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13800, 16636, 16636, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13800, 16642, 16642, 16652, 1, 2, { 15675, 15717, 15759, 15801, 15844, 15886, 0, 15929, 0 } },
    { 13800, 16671, 16671, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13800, 16677, 16677, 16683, 1, 1, { 0, 2394, 2443, 2492, 2542, 2591, 2641, 0, 2691 } },
    { 7209, 17031, 17031, 17038, 1, 1, { 16691, 16733, 16775, 16817, 16860, 16902, 0, 16945, 16988 } },
    { 7209, 17387, 17387, 15657, 1, 1, { 17040, 17089, 17138, 0, 0, 17187, 17237, 17287, 17337 } },
    { 7209, 17759, 17759, 17770, 1, 1, { 17394, 17439, 0, 17484, 17530, 17575, 17621, 17667, 17713 } },
    { 7209, 18097, 18097, 17770, 1, 1, { 17772, 17812, 0, 17852, 17893, 17933, 17974, 18015, 18056 } },
    { 7209, 18105, 18105, 18113, 1, 3, { 0, 2394, 2443, 2492, 2542, 2591, 2641, 0, 2691 } },
    { 7209, 753, 753, 18142, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7209, 10476, 10476, 17038, 1, 1, { 18146, 18185, 18224, 18263, 18303, 18342, 0, 18382, 18422 } },
    { 7209, 18613, 18613, 18617, 0, 1, { 0, 18462, 0, 0, 0, 18499, 0, 18537, 18575 } },
    { 7209, 18956, 18956, 18962, 1, 3, { 18624, 18665, 18706, 18747, 18789, 18830, 0, 18872, 18914 } },
    { 7209, 19314, 19314, 19320, 1, 2, { 18982, 19023, 19064, 19105, 19147, 19188, 0, 19230, 19272 } },
    { 7209, 19677, 19677, 17038, 1, 1, { 19337, 19379, 19421, 19463, 19506, 19548, 0, 19591, 19634 } },
    { 7209, 19684, 19684, 19688, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 20101, 20113, 20113, 0, 1, 1, { 19700, 19744, 19788, 19832, 19877, 19921, 19966, 20011, 20056 } },
    { 20101, 20521, 20521, 0, 1, 1, { 20120, 20164, 20208, 20252, 20297, 20341, 20386, 20431, 20476 } },
    { 20101, 20929, 20929, 0, 1, 1, { 20528, 20572, 20616, 20660, 20705, 20749, 20794, 20839, 20884 } },
    { 20101, 21346, 21346, 0, 1, 1, { 20936, 20981, 21026, 21071, 21117, 21162, 21208, 21254, 21300 } },
    { 20101, 21354, 21354, 0, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 21365, 21370, 21370, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 21365, 21379, 21379, 21384, 0, 3, { 0, 2394, 2443, 2492, 2542, 2591, 2641, 0, 0 } },
    { 21365, 21418, 21418, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 21365, 21428, 21428, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 21365, 21432, 21432, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 21365, 21437, 21437, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 21365, 21447, 21447, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 21365, 21460, 21460, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10200, 8638, 8638, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10200, 14194, 14194, 21466, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10200, 7532, 7532, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10200, 753, 753, 21472, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10200, 15048, 15048, 21466, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10200, 21491, 21491, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10200, 21497, 21497, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10200, 10916, 10916, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10200, 21502, 21502, 21472, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10200, 21846, 21846, 21853, 1, 1, { 21506, 21548, 21590, 21632, 21675, 0, 21717, 21760, 21803 } },
    { 10200, 21859, 21859, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10200, 21865, 21865, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 21872, 11202, 11202, 21879, 0, 1, { 10925, 10964, 0, 11003, 11043, 11082, 11122, 11162, 0 } },
    { 22246, 22251, 22251, 9698, 1, 1, { 21889, 21933, 0, 21977, 22022, 22066, 22111, 22156, 22201 } },
    { 22246, 22261, 22261, 22271, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 22246, 22276, 22276, 22282, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 22246, 22308, 22308, 22314, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 22246, 22320, 22320, 22271, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 22327, 388, 388, 22334, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 22327, 22408, 22408, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 22327, 2179, 2179, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 22327, 22422, 22422, 8633, 1, 1, { 0, 0, 0, 0, 0, 0, 2305, 0, 0 } },
    { 22327, 22763, 22763, 22770, 1, 2, { 22430, 22471, 0, 22512, 22554, 22595, 22637, 22679, 22721 } },
    { 2350, 22998, 22998, 23004, 1, 1, { 22795, 22835, 0, 0, 0, 0, 22875, 22916, 22957 } },
    { 2350, 23009, 23009, 21859, 1, 2, { 0, 0, 0, 0, 0, 0, 2305, 0, 0 } },
    { 23019, 22276, 22276, 23023, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23019, 23036, 23036, 23040, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10920, 23070, 23070, 23077, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10920, 8638, 8638, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10920, 14194, 14194, 23077, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10920, 7532, 7532, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10920, 14203, 14203, 23077, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10920, 23429, 23429, 23436, 1, 1, { 23082, 23131, 23180, 0, 0, 23229, 23279, 23329, 23379 } },
    { 10920, 15048, 15048, 23077, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10920, 15284, 15284, 23442, 1, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10920, 23464, 23464, 23471, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10920, 10916, 10916, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10920, 21502, 21502, 23478, 0, 1, { 0, 2394, 2443, 2492, 2542, 2591, 2641, 0, 2691 } },
    { 10920, 23483, 23483, 23077, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10920, 23490, 23490, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10920, 23498, 23498, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23503, 3127, 3127, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23503, 4641, 4641, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23503, 5123, 5123, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23833, 23836, 23836, 7871, 1, 1, { 23508, 23548, 0, 23588, 23629, 23669, 23710, 23751, 23792 } },
    { 23833, 24151, 24151, 24155, 1, 2, { 23842, 23880, 0, 23918, 23957, 23995, 24034, 24073, 24112 } },
    { 23833, 2748, 2748, 21879, 0, 1, { 24177, 24216, 0, 24255, 24295, 24334, 24374, 24414, 24454 } },
    { 23833, 7532, 7532, 24494, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23833, 24502, 24502, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23833, 24804, 24804, 24812, 0, 1, { 24506, 24548, 0, 24590, 24633, 24675, 24718, 24761, 0 } },
    { 23833, 25154, 25154, 7871, 1, 1, { 24821, 24862, 0, 24903, 24945, 24986, 25028, 25070, 25112 } },
    { 23833, 25494, 25494, 25501, 1, 2, { 25161, 25202, 0, 25243, 25285, 25326, 25368, 25410, 25452 } },
    { 23833, 25518, 25518, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23833, 25850, 25850, 7871, 1, 1, { 25525, 25565, 0, 25605, 25646, 25686, 25727, 25768, 25809 } },
    { 23833, 26189, 26189, 7871, 1, 1, { 25856, 25897, 0, 25938, 25980, 26021, 26063, 26105, 26147 } },
    { 23833, 26545, 26545, 26554, 1, 2, { 26196, 26239, 0, 26282, 26326, 26369, 26413, 26457, 26501 } },
    { 23833, 26870, 26870, 21879, 0, 1, { 26569, 26606, 0, 26643, 26681, 26718, 26756, 26794, 26832 } },
    { 23833, 27198, 27198, 27204, 1, 2, { 26873, 26913, 0, 26953, 26994, 27034, 27075, 27116, 27157 } },
    { 23833, 7866, 7866, 24494, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23833, 27572, 27572, 797, 1, 1, { 27223, 27266, 0, 27309, 27353, 27396, 27440, 27484, 27528 } },
    { 23833, 27946, 27946, 797, 1, 1, { 27581, 27626, 0, 27671, 27717, 27762, 27808, 27854, 27900 } },
    { 23833, 28306, 28306, 28315, 1, 2, { 27957, 28000, 0, 28043, 28087, 28130, 28174, 28218, 28262 } },
    { 23833, 28334, 28334, 24494, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23833, 28666, 28666, 28672, 1, 2, { 28341, 28381, 0, 28421, 28462, 28502, 28543, 28584, 28625 } },
    { 23833, 28968, 28968, 28972, 1, 3, { 28688, 28734, 0, 0, 0, 28780, 28827, 28874, 28921 } },
    { 23833, 8200, 8200, 7871, 1, 1, { 29001, 29040, 0, 29079, 29119, 29158, 29198, 29238, 29278 } },
    { 23833, 29318, 29318, 29323, 0, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23833, 29344, 29344, 29354, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 29381, 29388, 29388, 29396, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 29381, 29485, 29485, 29491, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 29521, 29528, 29528, 29535, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 29521, 23036, 23036, 29596, 1, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 29521, 8644, 8644, 29694, 1, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 30130, 30136, 30136, 5994, 1, 1, { 29789, 29831, 0, 29873, 29916, 29958, 30001, 30044, 30087 } },
    { 30130, 30477, 30477, 5994, 1, 1, { 30144, 30185, 0, 30226, 30268, 30309, 30351, 30393, 30435 } },
    { 30130, 5645, 5645, 30484, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 30130, 5978, 5978, 30484, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 30130, 5988, 5988, 30484, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 30130, 5019, 5019, 30484, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 30130, 6660, 6660, 30499, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 30130, 6684, 6684, 30528, 3, 4, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 30559, 395, 395, 30567, 1, -1, { 55, 96, 0, 137, 179, 220, 262, 304, 346 } },
    { 30559, 753, 753, 30567, 1, -1, { 444, 482, 0, 520, 559, 597, 636, 675, 714 } },
    { 30559, 30982, 30982, 30987, 1, -1, { 30665, 30704, 0, 30743, 30783, 30822, 30862, 30902, 30942 } },
    { 30559, 1127, 1127, 30567, 1, -1, { 802, 842, 0, 882, 923, 963, 1004, 1045, 1086 } },
    { 30559, 1450, 1450, 30567, 1, -1, { 1133, 1172, 0, 1211, 1251, 1290, 1330, 1370, 1410 } },
    { 30559, 1764, 1764, 30567, 1, -1, { 1455, 1493, 0, 1531, 1570, 1608, 1647, 1686, 1725 } },
    { 30559, 31382, 31382, 31393, 1, 1, { 31017, 31062, 0, 31107, 31153, 31198, 31244, 31290, 31336 } },
    { 31397, 31404, 31404, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31397, 31744, 31744, 31749, 1, 1, { 31411, 31458, 31505, 0, 0, 31552, 31600, 31648, 31696 } },
    { 31397, 4655, 4655, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31397, 29388, 29388, 31749, 1, 1, { 31757, 31807, 31857, 0, 0, 31907, 31958, 32009, 32060 } },
    { 31397, 32486, 32486, 31749, 1, 1, { 32111, 32164, 32217, 0, 0, 32270, 32324, 32378, 32432 } },
    { 31397, 32497, 32497, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31397, 32504, 32504, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31397, 32508, 32508, 0, 0, -1, { 2753, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31397, 32515, 32515, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31397, 32522, 32522, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31397, 32534, 32534, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31397, 32546, 32546, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31397, 32558, 32558, 32569, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31397, 32595, 32595, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31397, 32606, 32606, 32613, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31397, 32626, 32626, 0, 0, -1, { 2753, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31397, 32632, 32632, 32637, 1, 2, { 0, 2394, 2443, 2492, 2542, 2591, 2641, 0, 2691 } },
    { 31397, 32656, 32656, 32637, 1, 2, { 0, 2394, 2443, 2492, 2542, 2591, 2641, 0, 2691 } },
    { 31397, 2372, 2372, 32665, 1, 2, { 15675, 15717, 0, 15801, 15844, 15886, 0, 15929, 16008 } },
    { 31397, 32681, 32681, 32665, 1, 2, { 15675, 15717, 0, 15801, 15844, 15886, 0, 15929, 16008 } },
    { 31397, 32689, 32689, 32696, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31397, 32720, 32720, 32732, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31397, 32737, 32737, 32748, 3, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31397, 32768, 32768, 32779, 1, 1, { 0, 0, 2443, 2492, 2542, 2591, 2641, 0, 0 } },
    { 31397, 32787, 32787, 32796, 1, 1, { 0, 2394, 2443, 2492, 2542, 2591, 2641, 0, 2691 } },
    { 31397, 13791, 13791, 0, 0, -1, { 2753, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33224, 33232, 33232, 33250, 1, 3, { 32800, 32860, 32920, 0, 0, 32980, 33041, 33102, 33163 } },
    { 33224, 33687, 33687, 33701, 1, 3, { 33291, 33347, 33403, 0, 0, 33459, 33516, 33573, 33630 } },
    { 33224, 33738, 33738, 33750, 2, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33224, 33804, 33804, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33224, 33822, 33822, 33830, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33224, 34229, 34229, 31749, 1, 1, { 33840, 33888, 0, 33936, 33985, 34033, 34082, 34131, 34180 } },
    { 33224, 34632, 34632, 34646, 1, 1, { 34243, 34291, 0, 34339, 34388, 34436, 34485, 34534, 34583 } },
    { 33224, 35068, 35068, 34646, 1, 1, { 34655, 34706, 0, 34757, 34809, 34860, 34912, 34964, 35016 } },
    { 33224, 35085, 35085, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33224, 35097, 35097, 35109, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33224, 35526, 35526, 35541, 1, 1, { 35129, 35178, 0, 35227, 35277, 35326, 35376, 35426, 35476 } },
    { 33224, 35943, 35943, 35957, 1, 2, { 35554, 35602, 0, 35650, 35699, 35747, 35796, 35845, 35894 } },
    { 33224, 35987, 35987, 36001, 1, 2, { 0, 2394, 2443, 2492, 2542, 2591, 2641, 0, 2691 } },
    { 33224, 36024, 36024, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33224, 36043, 36043, 36049, 1, 1, { 0, 2394, 2443, 2492, 2542, 2591, 2641, 0, 2691 } },
    { 33224, 36051, 36051, 36049, 1, 1, { 0, 2394, 2443, 2492, 2542, 2591, 2641, 0, 2691 } },
    { 33224, 36057, 36057, 36072, 1, 1, { 0, 2394, 2443, 2492, 2542, 2591, 2641, 0, 2691 } },
    { 33224, 36081, 36081, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33224, 36491, 36491, 36506, 1, 1, { 36094, 36143, 0, 36192, 36242, 36291, 36341, 36391, 36441 } },
    { 33224, 36871, 36871, 36881, 1, 1, { 36514, 36558, 0, 36602, 36647, 36691, 36736, 36781, 36826 } },
    { 33224, 36891, 36891, 36901, 1, 1, { 15675, 15717, 0, 15801, 15844, 15886, 0, 15929, 16008 } },
    { 33224, 36911, 36911, 36921, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33224, 36946, 36946, 36956, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33224, 36981, 36981, 36049, 1, 1, { 0, 2394, 2443, 2492, 2542, 2591, 2641, 0, 2691 } },
    { 33224, 36987, 36987, 36049, 1, 1, { 0, 2394, 2443, 2492, 2542, 2591, 2641, 0, 2691 } },
    { 33224, 36993, 36993, 32779, 1, 1, { 0, 0, 2443, 2492, 2542, 2591, 2641, 0, 0 } },
    { 33224, 31397, 31397, 37011, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 33224, 37044, 37044, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 37396, 29388, 29388, 37404, 1, 1, { 37055, 37097, 0, 37139, 37182, 37224, 37267, 37310, 37353 } },
    { 13453, 13805, 13805, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13453, 37413, 37413, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13453, 13816, 13816, 37422, 1, 2, { 0, 2394, 2443, 2492, 2542, 2591, 2641, 0, 2691 } },
    { 13453, 14194, 14194, 6684, 1, 1, { 37442, 37485, 0, 37528, 37572, 37615, 37659, 37703, 37747 } },
    { 13453, 14203, 14203, 14209, 1, 3, { 37791, 37831, 0, 37871, 37912, 37952, 37993, 38034, 38075 } },
    { 13453, 38470, 38470, 14592, 0, 2, { 38116, 38166, 0, 38216, 38267, 38317, 38368, 38419, 0 } },
    { 13453, 14988, 14988, 38739, 1, 1, { 38477, 38520, 0, 0, 0, 38563, 38607, 38651, 38695 } },
    { 13453, 15022, 15022, 15033, 0, 1, { 0, 2394, 2443, 2492, 2542, 2591, 2641, 0, 2691 } },
    { 13453, 15043, 15043, 14209, 1, 3, { 38746, 38785, 0, 38824, 38864, 38903, 38943, 38983, 39023 } },
    { 13453, 5645, 5645, 39063, 1, 1, { 5296, 5338, 0, 5380, 5423, 5465, 5508, 5551, 5594 } },
    { 13453, 39070, 39070, 39077, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13453, 39458, 39458, 10200, 1, 1, { 39086, 39132, 39178, 39224, 39271, 0, 39317, 39364, 39411 } },
    { 13453, 753, 753, 39469, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13453, 39480, 39480, 39469, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13453, 15048, 15048, 6684, 1, 1, { 37442, 37485, 0, 37528, 37572, 37615, 37659, 37703, 37747 } },
    { 13453, 15284, 15284, 14209, 1, 3, { 39489, 39529, 0, 39569, 39610, 39650, 39691, 39732, 39773 } },
    { 13453, 15290, 15290, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13453, 15298, 15298, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13453, 15306, 15306, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13453, 39814, 39814, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13453, 15314, 15314, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13453, 39824, 39824, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13453, 15322, 15322, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13453, 39837, 39837, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13453, 39847, 39847, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13453, 15330, 15330, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13453, 15338, 15338, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13453, 15346, 15346, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13453, 15652, 15652, 15657, 1, 1, { 39859, 39906, 39953, 0, 0, 40000, 40048, 40096, 40144 } },
    { 13453, 10916, 10916, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13453, 15663, 15663, 37422, 1, 2, { 0, 2394, 2443, 2492, 2542, 2591, 2641, 0, 2691 } },
    { 13453, 15669, 15669, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13453, 15972, 15972, 15979, 0, 1, { 40192, 40233, 0, 40274, 40316, 40357, 40399, 40441, 0 } },
    { 13453, 15990, 15990, 16000, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13453, 5988, 5988, 39063, 1, 1, { 5653, 5693, 0, 5733, 5774, 5814, 5855, 5896, 5937 } },
    { 13453, 16051, 16051, 16061, 1, 1, { 40483, 40504, 0, 40525, 40547, 40568, 40590, 40612, 40634 } },
    { 13453, 16065, 16065, 16078, 1, 1, { 40656, 40703, 0, 40750, 40798, 40845, 40893, 40941, 40989 } },
    { 13453, 16080, 16080, 16078, 1, 1, { 41037, 41084, 0, 41131, 41179, 41226, 41274, 41322, 41370 } },
    { 13453, 16093, 16093, 41418, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13453, 16118, 16118, 14209, 1, 3, { 41437, 41477, 0, 41517, 41558, 41598, 41639, 41680, 41721 } },
    { 13453, 16124, 16124, 14209, 1, 3, { 41762, 41803, 0, 41844, 41886, 41927, 41969, 42011, 42053 } },
    { 13453, 16131, 16131, 37422, 1, 2, { 0, 2394, 2443, 2492, 2542, 2591, 2641, 0, 2691 } },
    { 13453, 16137, 16137, 16061, 1, 1, { 40483, 40504, 0, 40525, 40547, 40568, 40590, 40612, 40634 } },
    { 13453, 16148, 16148, 16155, 0, 2, { 42095, 42124, 0, 42153, 42183, 42212, 42242, 42272, 0 } },
    { 13453, 16176, 16176, 15979, 0, 1, { 42302, 42343, 0, 42384, 42426, 42467, 42509, 42551, 0 } },
    { 13453, 6660, 6660, 16155, 0, 2, { 42095, 42124, 0, 42153, 42183, 42212, 42242, 42272, 0 } },
    { 13453, 16183, 16183, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13453, 16584, 16584, 42867, 1, 1, { 42593, 42638, 0, 0, 0, 42683, 42729, 42775, 42821 } },
    { 13453, 16621, 16621, 15979, 0, 1, { 42875, 42915, 0, 42955, 42996, 43036, 43077, 43118, 0 } },
    { 13453, 6684, 6684, 43159, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13453, 16627, 16627, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13453, 16636, 16636, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13453, 16642, 16642, 43536, 1, 1, { 43172, 43217, 43262, 43307, 43353, 0, 43398, 43444, 43490 } },
    { 13453, 16671, 16671, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13453, 16677, 16677, 16683, 1, 1, { 0, 2394, 2443, 2492, 2542, 2591, 2641, 0, 2691 } },
    { 43924, 43928, 43928, 797, 1, 1, { 43543, 43590, 0, 43637, 43685, 43732, 43780, 43828, 43876 } },
    { 43924, 43941, 43941, 43946, 0, 1, { 0, 2394, 2443, 2492, 2542, 2591, 2641, 0, 0 } },
    { 43924, 43956, 43956, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 43965, 43976, 43976, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 43965, 8205, 8205, 43982, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 43965, 43997, 8205, 9698, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 44005, 44015, 44015, 44019, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 44005, 7876, 7876, 44025, 0, 1, { 0, 2394, 2443, 2492, 2542, 2591, 2641, 0, 2691 } },
    { 44005, 7886, 7886, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 44038, 43976, 43976, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 44038, 8205, 8205, 43982, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 44038, 43997, 8205, 9698, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13787, 14194, 14194, 23077, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13787, 14203, 14203, 23077, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13787, 15048, 15048, 23077, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13787, 15284, 15284, 23442, 1, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13787, 10916, 10916, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
};
