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
    "PoolFileUpload\0"
    "content_type\0"
    "save() argument 1 must be str, not Null\0"
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
    "to_file\0"
    "QRPoolFile\0"
    "caminho\0"
    "cookie() argument 1 must be str, not int\0"
    "cookie() argument 1 must be str, not flo\0"
    "cookie() argument 1 must be str, not list\0"
    "cookie() argument 1 must be str, not tup\0"
    "cookie() argument 1 must be str, not dict\0"
    "cookie() argument 1 must be str, not byte\0"
    "cookie() argument 1 must be str, not bool\0"
    "cookie() argument 1 must be str, not Null\0"
    "RequestProxy\0"
    "nome\0"
    "file() argument 1 must be str, not int\0"
    "file() argument 1 must be str, not flo\0"
    "file() argument 1 must be str, not list\0"
    "file() argument 1 must be str, not tup\0"
    "file() argument 1 must be str, not dict\0"
    "file() argument 1 must be str, not byte\0"
    "file() argument 1 must be str, not bool\0"
    "file() argument 1 must be str, not Null\0"
    "file\0"
    "field,allowed=Null\0"
    "files() argument 1 must be str, not int\0"
    "files() argument 1 must be str, not flo\0"
    "files() argument 1 must be str, not list\0"
    "files() argument 1 must be str, not tup\0"
    "files() argument 1 must be str, not dict\0"
    "files() argument 1 must be str, not byte\0"
    "files() argument 1 must be str, not bool\0"
    "files() argument 1 must be str, not Null\0"
    "files\0"
    "key\0"
    "get_json\0"
    "header() argument 1 must be str, not int\0"
    "header() argument 1 must be str, not flo\0"
    "header() argument 1 must be str, not list\0"
    "header() argument 1 must be str, not tup\0"
    "header() argument 1 must be str, not dict\0"
    "header() argument 1 must be str, not byte\0"
    "header() argument 1 must be str, not bool\0"
    "header() argument 1 must be str, not Null\0"
    "'dict' object is not callable\0"
    "headers\0"
    "'Null' object is not callable\0"
    "method\0"
    "path_param() argument 1 must be str, not int\0"
    "path_param() argument 1 must be str, not flo\0"
    "path_param() argument 1 must be str, not list\0"
    "path_param() argument 1 must be str, not tup\0"
    "path_param() argument 1 must be str, not dict\0"
    "path_param() argument 1 must be str, not byte\0"
    "path_param() argument 1 must be str, not bool\0"
    "path_param() argument 1 must be str, not Null\0"
    "path_param\0"
    "text\0"
    "'byte' object is not callable\0"
    "Response\0"
    "content\0"
    "content_type() argument 1 must be str, not int\0"
    "content_type() argument 1 must be str, not flo\0"
    "content_type() argument 1 must be str, not list\0"
    "content_type() argument 1 must be str, not tup\0"
    "content_type() argument 1 must be str, not dict\0"
    "content_type() argument 1 must be str, not byte\0"
    "content_type() argument 1 must be str, not bool\0"
    "content_type() argument 1 must be str, not Null\0"
    "esperado\0"
    "decode\0"
    "encoding=Null\0"
    "filename\0"
    "chave\0"
    "chave=Null\0"
    "'bool' object is not callable\0"
    "ok\0"
    "url\0"
    "RouteRegistrar\0"
    "register\0"
    "handler\0"
    "WsConnection\0"
    "on_message\0"
    "callback\0"
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
    { 8644, 7240, 7240, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8644, 8659, 8659, 0, 0, -1, { 0, 0, 1246, 0, 0, 0, 0, 0, 0 } },
    { 8644, 7576, 7576, 0, 0, -1, { 0, 0, 1246, 0, 0, 0, 0, 0, 0 } },
    { 8644, 7897, 7897, 7568, 1, 1, { 7936, 7975, 0, 8014, 8054, 8093, 8133, 8173, 8672 } },
    { 8644, 1275, 1275, 0, 0, -1, { 0, 0, 1246, 0, 0, 0, 0, 0, 0 } },
    { 8644, 8213, 8213, 7568, 1, 1, { 7936, 7975, 0, 8014, 8054, 8093, 8133, 8173, 8672 } },
    { 8644, 8231, 8231, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8712, 8723, 8723, 8732, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8712, 8737, 8737, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8712, 8743, 8743, 8748, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8712, 8757, 8757, 8768, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8810, 68, 68, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8810, 8818, 8818, 8823, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8810, 8832, 8832, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8810, 7907, 7907, 9113, 0, 1, { 0, 8836, 8875, 8914, 8954, 8993, 9033, 0, 9073 } },
    { 8810, 9425, 9425, 9113, 0, 1, { 0, 9120, 9163, 9206, 9250, 9293, 9337, 0, 9381 } },
    { 8810, 7917, 7917, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8810, 9434, 9434, 9441, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8810, 9456, 9456, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8810, 9467, 9467, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 8810, 8236, 8236, 9797, 1, 1, { 9472, 9512, 0, 9552, 9593, 9633, 9674, 9715, 9756 } },
    { 9803, 1275, 1275, 0, 0, -1, { 0, 0, 1246, 0, 0, 0, 0, 0, 0 } },
    { 9803, 9434, 9434, 9811, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 9803, 8213, 8213, 7902, 1, 1, { 7936, 7975, 0, 8014, 8054, 8093, 8133, 8173, 8672 } },
    { 9803, 9824, 9824, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 9832, 7240, 7240, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 9832, 7576, 7576, 0, 0, -1, { 0, 0, 1246, 0, 0, 0, 0, 0, 0 } },
    { 9832, 1275, 1275, 0, 0, -1, { 0, 0, 1246, 0, 0, 0, 0, 0, 0 } },
    { 9832, 8213, 8213, 9843, 1, 1, { 7936, 7975, 0, 8014, 8054, 8093, 8133, 8173, 8672 } },
    { 9832, 8231, 8231, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10184, 2672, 2672, 10197, 1, 1, { 9851, 9892, 0, 9933, 9975, 10016, 10058, 10100, 10142 } },
    { 10184, 10519, 10519, 10524, 1, 2, { 10202, 10241, 0, 10280, 10320, 10359, 10399, 10439, 10479 } },
    { 10184, 10868, 10868, 10524, 1, 2, { 10543, 10583, 0, 10623, 10664, 10704, 10745, 10786, 10827 } },
    { 10184, 1231, 1231, 10874, 1, 1, { 922, 960, 0, 998, 1037, 1075, 1114, 1153, 1192 } },
    { 10184, 10878, 10878, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10184, 2766, 2766, 10874, 1, 1, { 10887, 10928, 0, 10969, 11011, 11052, 11094, 11136, 11178 } },
    { 10184, 11250, 11250, 0, 0, -1, { 0, 0, 0, 0, 0, 11220, 0, 0, 0 } },
    { 10184, 2828, 2828, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 10184, 11288, 11288, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 11258 } },
    { 10184, 7902, 7902, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 11258 } },
    { 10184, 11660, 11660, 10874, 1, 1, { 11295, 11340, 0, 11385, 11431, 11476, 11522, 11568, 11614 } },
    { 10184, 11671, 11671, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 11706, 11715, 11715, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 11676, 0, 0 } },
    { 11706, 8659, 8659, 12104, 1, 1, { 11723, 11770, 0, 11817, 11865, 11912, 11960, 12008, 12056 } },
    { 11706, 12113, 12113, 12120, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 11706, 12134, 12134, 0, 0, -1, { 0, 0, 1246, 0, 0, 0, 0, 0, 0 } },
    { 11706, 1231, 1231, 12143, 1, 1, { 922, 960, 0, 998, 1037, 1075, 1114, 1153, 1192 } },
    { 11706, 10878, 10878, 12149, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 11706, 11250, 11250, 0, 0, -1, { 0, 0, 0, 0, 0, 11220, 0, 0, 0 } },
    { 11706, 2828, 2828, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 11706, 12190, 12190, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 12160, 0 } },
    { 11706, 8213, 8213, 8218, 0, 1, { 7936, 7975, 0, 8014, 8054, 8093, 8133, 8173, 0 } },
    { 11706, 8231, 8231, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 11706, 3219, 3219, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 11706, 3231, 3231, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 11706, 11671, 11671, 0, 0, -1, { 0, 0, 1246, 0, 0, 0, 0, 0, 0 } },
    { 11706, 12193, 12193, 0, 0, -1, { 0, 0, 1246, 0, 0, 0, 0, 0, 0 } },
    { 12197, 12212, 12212, 12221, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 12229, 68, 68, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 12229, 12242, 12242, 12253, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 12229, 2850, 2850, 8732, 1, 1, { 0, 0, 0, 0, 0, 0, 2783, 0, 0 } },
    { 12470, 12479, 12479, 0, 1, 1, { 0, 0, 12262, 12296, 12331, 12365, 12400, 0, 12435 } },
    { 12470, 12483, 12483, 0, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 12470, 12490, 12490, 0, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 12470, 12499, 12499, 12506, 1, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 12470, 12539, 12539, 0, 1, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 12470, 12543, 12543, 0, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 12470, 12548, 12548, 0, 1, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 12470, 12670, 12670, 0, 0, 1, { 12552, 12581, 0, 0, 0, 0, 0, 12610, 12640 } },
    { 12470, 12675, 12675, 0, 1, 1, { 12552, 12581, 0, 0, 0, 0, 0, 12610, 12640 } },
    { 12470, 12685, 12685, 0, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 12470, 12935, 12935, 0, 1, 1, { 0, 0, 0, 12692, 12753, 12813, 0, 0, 12874 } },
    { 12470, 12939, 12939, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 12470, 12946, 12946, 0, 1, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 12470, 5101, 5101, 0, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 12470, 12950, 12950, 0, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 12470, 13279, 13279, 0, 0, 1, { 0, 0, 0, 12956, 13037, 13117, 0, 0, 13198 } },
    { 12470, 13386, 13386, 0, 1, 1, { 13283, 13317, 0, 0, 0, 0, 0, 13351, 0 } },
    { 12470, 13390, 13390, 0, 0, 1, { 12552, 12581, 0, 0, 0, 0, 0, 12610, 12640 } },
    { 12470, 13672, 13672, 0, 0, 1, { 13395, 13434, 0, 13473, 13513, 13552, 13592, 13632, 0 } },
    { 12470, 13677, 13279, 0, 0, 1, { 0, 0, 0, 12956, 13037, 13117, 0, 0, 13198 } },
    { 12470, 13682, 13682, 0, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 12470, 13686, 13686, 0, 1, -1, { 12552, 12581, 0, 0, 0, 0, 0, 12610, 12640 } },
    { 12470, 13690, 13690, 0, 1, -1, { 12552, 12581, 0, 0, 0, 0, 0, 12610, 12640 } },
    { 12470, 13694, 13694, 0, 1, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 12470, 14015, 14015, 14020, 1, 3, { 13698, 13737, 0, 13776, 13816, 13855, 13895, 13935, 13975 } },
    { 12470, 14446, 14446, 0, 1, 1, { 14049, 14098, 0, 14147, 14197, 14246, 14296, 14346, 14396 } },
    { 12470, 1928, 1928, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 12470, 14450, 14450, 14454, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 12470, 14473, 14473, 0, 1, 3, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 12470, 14843, 14843, 0, 1, 1, { 14479, 14524, 14569, 0, 14614, 14659, 14705, 14751, 14797 } },
    { 12470, 15233, 15233, 0, 1, 1, { 14853, 14900, 14947, 0, 14994, 15041, 15089, 15137, 15185 } },
    { 12470, 15371, 15371, 0, 1, 1, { 15245, 15276, 0, 0, 0, 0, 0, 15307, 15339 } },
    { 12470, 15630, 15630, 0, 1, 2, { 0, 0, 15380, 15421, 15463, 15504, 15546, 0, 15588 } },
    { 12470, 15910, 15910, 0, 1, 1, { 0, 0, 15636, 15681, 15727, 15772, 15818, 0, 15864 } },
    { 12470, 15916, 15916, 0, 1, 1, { 12552, 12581, 0, 0, 0, 0, 0, 12610, 12640 } },
    { 12470, 15923, 15923, 0, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 12470, 16253, 16253, 0, 1, 2, { 15927, 15973, 16019, 0, 0, 16065, 16112, 16159, 16206 } },
    { 12470, 16257, 16257, 0, 0, 1, { 12552, 12581, 0, 0, 0, 0, 0, 12610, 12640 } },
    { 12470, 16261, 16261, 0, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 12470, 16266, 16266, 0, 0, -1, { 12552, 12581, 0, 0, 0, 0, 0, 12610, 12640 } },
    { 16270, 16275, 16275, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 16270, 16286, 16286, 16293, 1, 2, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 16270, 16664, 16664, 7136, 1, 1, { 0, 16313, 16371, 16429, 16488, 16546, 0, 0, 16605 } },
    { 16270, 16673, 16673, 16679, 1, 3, { 0, 16313, 16371, 16429, 16488, 16546, 0, 0, 16605 } },
    { 16270, 12113, 12113, 17055, 0, 2, { 16701, 16751, 0, 16801, 16852, 16902, 16953, 17004, 0 } },
    { 16270, 17451, 17451, 17460, 1, 3, { 17088, 17148, 17208, 0, 0, 17268, 0, 17329, 17390 } },
    { 16270, 17485, 17485, 17496, 0, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 0 } },
    { 16270, 17506, 17506, 16679, 1, 3, { 0, 16313, 16371, 16429, 16488, 16546, 0, 0, 16605 } },
    { 16270, 17511, 17511, 7136, 1, 1, { 0, 16313, 16371, 16429, 16488, 16546, 0, 0, 16605 } },
    { 16270, 12946, 12946, 17722, 0, 2, { 17515, 17549, 0, 17583, 17618, 17652, 0, 17687, 0 } },
    { 16270, 17747, 17747, 16679, 1, 3, { 0, 16313, 16371, 16429, 16488, 16546, 0, 0, 16605 } },
    { 16270, 17753, 17753, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 16270, 17761, 17761, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 16270, 17769, 17769, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 16270, 17777, 17777, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 16270, 17785, 17785, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 16270, 17793, 17793, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 16270, 17801, 17801, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 16270, 17809, 17809, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 16270, 18115, 18115, 18120, 1, 1, { 17817, 17876, 0, 0, 0, 17935, 0, 17995, 18055 } },
    { 16270, 13386, 13386, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 16270, 18126, 18126, 16293, 1, 2, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 16270, 18132, 18132, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 16270, 18435, 18435, 18442, 0, 1, { 18138, 18180, 18222, 18264, 18307, 18349, 0, 18392, 0 } },
    { 16270, 18453, 18453, 18463, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 16270, 18514, 18514, 18524, 1, 1, { 18138, 18180, 18222, 18264, 18307, 18349, 0, 18392, 18471 } },
    { 16270, 18528, 18528, 18541, 1, 1, { 18138, 18180, 18222, 18264, 18307, 18349, 0, 18392, 18471 } },
    { 16270, 18543, 18543, 18541, 1, 1, { 18138, 18180, 18222, 18264, 18307, 18349, 0, 18392, 18471 } },
    { 16270, 18556, 18556, 18564, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 16270, 18581, 18581, 16679, 1, 3, { 0, 16313, 16371, 16429, 16488, 16546, 0, 0, 16605 } },
    { 16270, 18587, 18587, 16679, 1, 3, { 0, 16313, 16371, 16429, 16488, 16546, 0, 0, 16605 } },
    { 16270, 18594, 18594, 16293, 1, 2, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 16270, 18600, 18600, 18524, 1, 1, { 18138, 18180, 18222, 18264, 18307, 18349, 0, 18392, 18471 } },
    { 16270, 18611, 18611, 18618, 0, 2, { 18138, 18180, 18222, 18264, 18307, 18349, 0, 18392, 0 } },
    { 16270, 18639, 18639, 18442, 0, 1, { 18138, 18180, 18222, 18264, 18307, 18349, 0, 18392, 0 } },
    { 16270, 7112, 7112, 18618, 0, 2, { 18138, 18180, 18222, 18264, 18307, 18349, 0, 18392, 0 } },
    { 16270, 18646, 18646, 18657, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 16270, 19047, 19047, 19058, 1, 3, { 18672, 18734, 18796, 0, 0, 18858, 0, 18921, 18984 } },
    { 16270, 19084, 19084, 18442, 0, 1, { 18138, 18180, 18222, 18264, 18307, 18349, 0, 18392, 0 } },
    { 16270, 19090, 19090, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 16270, 19099, 19099, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 16270, 19105, 19105, 19115, 1, 2, { 18138, 18180, 18222, 18264, 18307, 18349, 0, 18392, 0 } },
    { 16270, 19134, 19134, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 16270, 19140, 19140, 19146, 1, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 7240, 19494, 19494, 19501, 1, 1, { 19154, 19196, 19238, 19280, 19323, 19365, 0, 19408, 19451 } },
    { 7240, 19850, 19850, 18120, 1, 1, { 19503, 19552, 19601, 0, 0, 19650, 19700, 19750, 19800 } },
    { 7240, 20222, 20222, 20233, 1, 1, { 19857, 19902, 0, 19947, 19993, 20038, 20084, 20130, 20176 } },
    { 7240, 20560, 20560, 20233, 1, 1, { 20235, 20275, 0, 20315, 20356, 20396, 20437, 20478, 20519 } },
    { 7240, 20568, 20568, 20576, 1, 3, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 7240, 1231, 1231, 20605, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 7240, 12946, 12946, 19501, 1, 1, { 20609, 20648, 20687, 20726, 20766, 20805, 0, 20845, 20885 } },
    { 7240, 21076, 21076, 21080, 0, 1, { 0, 20925, 0, 0, 0, 20962, 0, 21000, 21038 } },
    { 7240, 21419, 21419, 21425, 1, 3, { 21087, 21128, 21169, 21210, 21252, 21293, 0, 21335, 21377 } },
    { 7240, 21777, 21777, 21783, 1, 2, { 21445, 21486, 21527, 21568, 21610, 21651, 0, 21693, 21735 } },
    { 7240, 22140, 22140, 19501, 1, 1, { 21800, 21842, 21884, 21926, 21969, 22011, 0, 22054, 22097 } },
    { 7240, 22147, 22147, 22151, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 22564, 22576, 22576, 0, 1, 1, { 22163, 22207, 22251, 22295, 22340, 22384, 22429, 22474, 22519 } },
    { 22564, 22984, 22984, 0, 1, 1, { 22583, 22627, 22671, 22715, 22760, 22804, 22849, 22894, 22939 } },
    { 22564, 23392, 23392, 0, 1, 1, { 22991, 23035, 23079, 23123, 23168, 23212, 23257, 23302, 23347 } },
    { 22564, 23809, 23809, 0, 1, 1, { 23399, 23444, 23489, 23534, 23580, 23625, 23671, 23717, 23763 } },
    { 22564, 23817, 23817, 0, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23828, 23833, 23833, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23828, 23842, 23842, 23847, 0, 3, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 0 } },
    { 23828, 23881, 23881, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23828, 23891, 23891, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23828, 23895, 23895, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23828, 23900, 23900, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23828, 23910, 23910, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 23828, 23923, 23923, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 12670, 8737, 8737, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 12670, 16664, 16664, 12143, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 12670, 7563, 7563, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 12670, 1231, 1231, 23929, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 12670, 17511, 17511, 12143, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 12670, 23948, 23948, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 12670, 23954, 23954, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 12670, 13386, 13386, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 12670, 23959, 23959, 23929, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 12670, 24303, 24303, 24310, 1, 1, { 23963, 24005, 24047, 24089, 24132, 0, 24174, 24217, 24260 } },
    { 12670, 24316, 24316, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 12670, 24322, 24322, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 24329, 13672, 13672, 24336, 0, 1, { 13395, 13434, 0, 13473, 13513, 13552, 13592, 13632, 0 } },
    { 24703, 24708, 24708, 9797, 1, 1, { 24346, 24390, 0, 24434, 24479, 24523, 24568, 24613, 24658 } },
    { 24703, 24718, 24718, 24728, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 24703, 24733, 24733, 24739, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 24703, 24765, 24765, 24771, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 24703, 24777, 24777, 24728, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 24784, 866, 866, 24791, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 24784, 24865, 24865, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 24784, 2657, 2657, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 24784, 25262, 25262, 25267, 0, -1, { 24879, 24921, 24963, 25005, 25048, 25090, 25133, 25176, 25219 } },
    { 24784, 25324, 25324, 8732, 1, 1, { 0, 0, 0, 0, 0, 0, 2783, 0, 0 } },
    { 24784, 25665, 25665, 25672, 1, 2, { 25332, 25373, 0, 25414, 25456, 25497, 25539, 25581, 25623 } },
    { 24784, 25697, 25697, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 2828, 25908, 25908, 11671, 1, 1, { 25705, 25745, 0, 0, 0, 0, 25785, 25826, 25867 } },
    { 2828, 25914, 25914, 24316, 1, 2, { 0, 0, 0, 0, 0, 0, 2783, 0, 0 } },
    { 25924, 24733, 24733, 25928, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 25924, 25941, 25941, 25945, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13390, 25975, 25975, 25982, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13390, 8737, 8737, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13390, 16664, 16664, 25982, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13390, 7563, 7563, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13390, 16673, 16673, 25982, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13390, 26334, 26334, 26341, 1, 1, { 25987, 26036, 26085, 0, 0, 26134, 26184, 26234, 26284 } },
    { 13390, 17511, 17511, 25982, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13390, 17747, 17747, 26347, 1, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13390, 26369, 26369, 26376, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13390, 13386, 13386, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13390, 23959, 23959, 26383, 0, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 13390, 26388, 26388, 25982, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13390, 26395, 26395, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 13390, 26403, 26403, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 26408, 3576, 3576, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 26408, 5090, 5090, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 26408, 5575, 5575, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 26413, 7231, 7231, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 26413, 26741, 26741, 7902, 1, 1, { 26416, 26456, 0, 26496, 26537, 26577, 26618, 26659, 26700 } },
    { 26413, 27056, 27056, 27060, 1, 2, { 26747, 26785, 0, 26823, 26862, 26900, 26939, 26978, 27017 } },
    { 26413, 7563, 7563, 27082, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 26413, 27090, 27090, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 26413, 27392, 27392, 27400, 0, 1, { 27094, 27136, 0, 27178, 27221, 27263, 27306, 27349, 0 } },
    { 26413, 27742, 27742, 7902, 1, 1, { 27409, 27450, 0, 27491, 27533, 27574, 27616, 27658, 27700 } },
    { 26413, 28082, 28082, 28089, 1, 2, { 27749, 27790, 0, 27831, 27873, 27914, 27956, 27998, 28040 } },
    { 26413, 28106, 28106, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 26413, 28438, 28438, 7902, 1, 1, { 28113, 28153, 0, 28193, 28234, 28274, 28315, 28356, 28397 } },
    { 26413, 28777, 28777, 7902, 1, 1, { 28444, 28485, 0, 28526, 28568, 28609, 28651, 28693, 28735 } },
    { 26413, 29133, 29133, 29142, 1, 2, { 28784, 28827, 0, 28870, 28914, 28957, 29001, 29045, 29089 } },
    { 26413, 29458, 29458, 24336, 0, 1, { 29157, 29194, 0, 29231, 29269, 29306, 29344, 29382, 29420 } },
    { 26413, 29786, 29786, 29792, 1, 2, { 29461, 29501, 0, 29541, 29582, 29622, 29663, 29704, 29745 } },
    { 26413, 7897, 7897, 27082, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 26413, 30160, 30160, 1275, 1, 1, { 29811, 29854, 0, 29897, 29941, 29984, 30028, 30072, 30116 } },
    { 26413, 30534, 30534, 1275, 1, 1, { 30169, 30214, 0, 30259, 30305, 30350, 30396, 30442, 30488 } },
    { 26413, 30894, 30894, 30903, 1, 2, { 30545, 30588, 0, 30631, 30675, 30718, 30762, 30806, 30850 } },
    { 26413, 30922, 30922, 27082, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 26413, 31254, 31254, 31260, 1, 2, { 30929, 30969, 0, 31009, 31050, 31090, 31131, 31172, 31213 } },
    { 26413, 31556, 31556, 31560, 1, 3, { 31276, 31322, 0, 0, 0, 31368, 31415, 31462, 31509 } },
    { 26413, 8231, 8231, 7902, 1, 1, { 31589, 31628, 0, 31667, 31707, 31746, 31786, 31826, 31866 } },
    { 26413, 31906, 31906, 31911, 0, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 26413, 31932, 31932, 31942, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31969, 31976, 31976, 31984, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 31969, 32073, 32073, 32079, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 32109, 32116, 32116, 0, 0, -1, { 0, 0, 1246, 0, 0, 0, 0, 0, 0 } },
    { 32109, 32132, 32132, 0, 0, -1, { 0, 0, 1246, 0, 0, 0, 0, 0, 0 } },
    { 32109, 32148, 32148, 0, 0, -1, { 0, 0, 1246, 0, 0, 0, 0, 0, 0 } },
    { 32109, 32164, 32164, 0, 0, -1, { 0, 0, 1246, 0, 0, 0, 0, 0, 0 } },
    { 32109, 32180, 32180, 32187, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 32109, 25941, 25941, 32248, 1, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 32109, 8743, 8743, 32346, 1, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 32782, 32788, 32788, 6446, 1, 1, { 32441, 32483, 0, 32525, 32568, 32610, 32653, 32696, 32739 } },
    { 32782, 33129, 33129, 6446, 1, 1, { 32796, 32837, 0, 32878, 32920, 32961, 33003, 33045, 33087 } },
    { 32782, 6097, 6097, 33136, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 32782, 6430, 6430, 33136, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 32782, 6440, 6440, 33136, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 32782, 5471, 5471, 33136, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 32782, 7112, 7112, 33151, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 32782, 7136, 7136, 33180, 3, 4, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 25697, 873, 873, 33211, 1, -1, { 533, 574, 0, 615, 657, 698, 740, 782, 824 } },
    { 25697, 1231, 1231, 33211, 1, -1, { 922, 960, 0, 998, 1037, 1075, 1114, 1153, 1192 } },
    { 25697, 33626, 33626, 33631, 1, -1, { 33309, 33348, 0, 33387, 33427, 33466, 33506, 33546, 33586 } },
    { 25697, 1605, 1605, 33211, 1, -1, { 1280, 1320, 0, 1360, 1401, 1441, 1482, 1523, 1564 } },
    { 25697, 1928, 1928, 33211, 1, -1, { 1611, 1650, 0, 1689, 1729, 1768, 1808, 1848, 1888 } },
    { 25697, 2242, 2242, 33211, 1, -1, { 1933, 1971, 0, 2009, 2048, 2086, 2125, 2164, 2203 } },
    { 25697, 34026, 34026, 12193, 1, 1, { 33661, 33706, 0, 33751, 33797, 33842, 33888, 33934, 33980 } },
    { 34037, 34044, 34044, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 34037, 34384, 34384, 34389, 1, 1, { 34051, 34098, 34145, 0, 0, 34192, 34240, 34288, 34336 } },
    { 34037, 68, 68, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 34037, 31976, 31976, 34389, 1, 1, { 34397, 34447, 34497, 0, 0, 34547, 34598, 34649, 34700 } },
    { 34037, 35126, 35126, 34389, 1, 1, { 34751, 34804, 34857, 0, 0, 34910, 34964, 35018, 35072 } },
    { 34037, 35137, 35137, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 34037, 35144, 35144, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 34037, 35148, 35148, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 34037, 35155, 35155, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 34037, 35162, 35162, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 34037, 35174, 35174, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 34037, 35186, 35186, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 34037, 35198, 35198, 35209, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 34037, 35235, 35235, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 34037, 35246, 35246, 35253, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 34037, 35266, 35266, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 34037, 35272, 35272, 35277, 1, 2, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 34037, 35296, 35296, 35277, 1, 2, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 34037, 2850, 2850, 35305, 1, 2, { 18138, 18180, 0, 18264, 18307, 18349, 0, 18392, 18471 } },
    { 34037, 35321, 35321, 35305, 1, 2, { 18138, 18180, 0, 18264, 18307, 18349, 0, 18392, 18471 } },
    { 34037, 35329, 35329, 35336, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 34037, 35360, 35360, 35372, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 34037, 35377, 35377, 35388, 3, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 34037, 35408, 35408, 35419, 1, 1, { 0, 0, 2921, 2970, 3020, 3069, 3119, 0, 0 } },
    { 34037, 35427, 35427, 35436, 1, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 34037, 16261, 16261, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 35448, 35448, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 35456, 35456, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 35465, 35465, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 35475, 35475, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 35483, 35483, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 35493, 35493, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 35507, 35507, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 35514, 35514, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 35527, 35527, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 35542, 35542, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 35557, 35557, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 35568, 35568, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 35580, 35580, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 35591, 35591, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 35608, 35608, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 35624, 35624, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 35637, 35637, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 35648, 35648, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 35660, 35660, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 35672, 35672, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 35684, 35684, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 35696, 35696, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 35714, 35714, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 35733, 35733, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 35751, 35751, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 35768, 35768, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 35775, 35775, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 35789, 35789, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 35802, 35802, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 35810, 35810, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 35819, 35819, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 35829, 35829, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 35841, 35841, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 35850, 35850, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 35862, 35862, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 35872, 35872, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 35887, 35887, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 35902, 35902, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 35910, 35910, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 35920, 35920, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 35928, 35928, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 35939, 35939, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 35948, 35948, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 35963, 35963, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 35975, 35975, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 35986, 35986, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 35996, 35996, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 36009, 36009, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 36022, 36022, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 36031, 36031, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 36044, 36044, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 36054, 36054, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 36067, 36067, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 36077, 36077, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 36089, 36089, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 36102, 36102, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 36115, 36115, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 36125, 36125, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 36137, 36137, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 36145, 36145, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 36157, 36157, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 36170, 36170, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 36184, 36184, 0, 0, -1, { 495, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 36620, 36620, 36638, 1, 3, { 36196, 36256, 36316, 0, 0, 36376, 36437, 36498, 36559 } },
    { 35440, 37075, 37075, 37089, 1, 3, { 36679, 36735, 36791, 0, 0, 36847, 36904, 36961, 37018 } },
    { 35440, 37126, 37126, 37138, 2, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 37192, 37192, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 37210, 37210, 37218, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 37617, 37617, 34389, 1, 1, { 37228, 37276, 0, 37324, 37373, 37421, 37470, 37519, 37568 } },
    { 35440, 38020, 38020, 38034, 1, 1, { 37631, 37679, 0, 37727, 37776, 37824, 37873, 37922, 37971 } },
    { 35440, 38456, 38456, 38034, 1, 1, { 38043, 38094, 0, 38145, 38197, 38248, 38300, 38352, 38404 } },
    { 35440, 38473, 38473, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 38485, 38485, 38497, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 38914, 38914, 38929, 1, 1, { 38517, 38566, 0, 38615, 38665, 38714, 38764, 38814, 38864 } },
    { 35440, 39331, 39331, 39345, 1, 2, { 38942, 38990, 0, 39038, 39087, 39135, 39184, 39233, 39282 } },
    { 35440, 39375, 39375, 39389, 1, 2, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 35440, 39412, 39412, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 39431, 39431, 39437, 1, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 35440, 39439, 39439, 39437, 1, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 35440, 39445, 39445, 39460, 1, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 35440, 39469, 39469, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 39879, 39879, 39894, 1, 1, { 39482, 39531, 0, 39580, 39630, 39679, 39729, 39779, 39829 } },
    { 35440, 40259, 40259, 40269, 1, 1, { 39902, 39946, 0, 39990, 40035, 40079, 40124, 40169, 40214 } },
    { 35440, 40279, 40279, 40289, 1, 1, { 18138, 18180, 0, 18264, 18307, 18349, 0, 18392, 18471 } },
    { 35440, 40299, 40299, 40309, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 40334, 40334, 40344, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 40369, 40369, 39437, 1, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 35440, 40375, 40375, 39437, 1, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 35440, 40381, 40381, 35419, 1, 1, { 0, 0, 2921, 2970, 3020, 3069, 3119, 0, 0 } },
    { 35440, 34037, 34037, 40399, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 35440, 40432, 40432, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 40784, 31976, 31976, 40792, 1, 1, { 40443, 40485, 0, 40527, 40570, 40612, 40655, 40698, 40741 } },
    { 15923, 16275, 16275, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 15923, 40801, 40801, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 15923, 16286, 16286, 40810, 1, 2, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 15923, 16664, 16664, 7136, 1, 1, { 40830, 40873, 0, 40916, 40960, 41003, 41047, 41091, 41135 } },
    { 15923, 16673, 16673, 16679, 1, 3, { 41179, 41219, 0, 41259, 41300, 41340, 41381, 41422, 41463 } },
    { 15923, 41858, 41858, 17055, 0, 2, { 41504, 41554, 0, 41604, 41655, 41705, 41756, 41807, 0 } },
    { 15923, 17451, 17451, 42127, 1, 1, { 41865, 41908, 0, 0, 0, 41951, 41995, 42039, 42083 } },
    { 15923, 17485, 17485, 17496, 0, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 15923, 17506, 17506, 16679, 1, 3, { 42134, 42173, 0, 42212, 42252, 42291, 42331, 42371, 42411 } },
    { 15923, 6097, 6097, 42451, 1, 1, { 5748, 5790, 0, 5832, 5875, 5917, 5960, 6003, 6046 } },
    { 15923, 42458, 42458, 42465, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 15923, 42846, 42846, 12670, 1, 1, { 42474, 42520, 42566, 42612, 42659, 0, 42705, 42752, 42799 } },
    { 15923, 1231, 1231, 12149, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 15923, 10878, 10878, 12149, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 15923, 17511, 17511, 7136, 1, 1, { 40830, 40873, 0, 40916, 40960, 41003, 41047, 41091, 41135 } },
    { 15923, 17747, 17747, 16679, 1, 3, { 42857, 42897, 0, 42937, 42978, 43018, 43059, 43100, 43141 } },
    { 15923, 17753, 17753, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 15923, 17761, 17761, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 15923, 17769, 17769, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 15923, 43182, 43182, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 15923, 17777, 17777, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 15923, 43192, 43192, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 15923, 17785, 17785, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 15923, 43205, 43205, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 15923, 43215, 43215, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 15923, 17793, 17793, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 15923, 17801, 17801, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 15923, 17809, 17809, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 15923, 18115, 18115, 18120, 1, 1, { 43227, 43274, 43321, 0, 0, 43368, 43416, 43464, 43512 } },
    { 15923, 13386, 13386, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 15923, 18126, 18126, 40810, 1, 2, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 15923, 18132, 18132, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 15923, 18435, 18435, 18442, 0, 1, { 43560, 43601, 0, 43642, 43684, 43725, 43767, 43809, 0 } },
    { 15923, 18453, 18453, 18463, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 15923, 6440, 6440, 42451, 1, 1, { 6105, 6145, 0, 6185, 6226, 6266, 6307, 6348, 6389 } },
    { 15923, 18514, 18514, 18524, 1, 1, { 43851, 43872, 0, 43893, 43915, 43936, 43958, 43980, 44002 } },
    { 15923, 18528, 18528, 18541, 1, 1, { 44024, 44071, 0, 44118, 44166, 44213, 44261, 44309, 44357 } },
    { 15923, 18543, 18543, 18541, 1, 1, { 44405, 44452, 0, 44499, 44547, 44594, 44642, 44690, 44738 } },
    { 15923, 18556, 18556, 44786, 2, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 15923, 18581, 18581, 16679, 1, 3, { 44805, 44845, 0, 44885, 44926, 44966, 45007, 45048, 45089 } },
    { 15923, 18587, 18587, 16679, 1, 3, { 45130, 45171, 0, 45212, 45254, 45295, 45337, 45379, 45421 } },
    { 15923, 18594, 18594, 40810, 1, 2, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 15923, 18600, 18600, 18524, 1, 1, { 43851, 43872, 0, 43893, 43915, 43936, 43958, 43980, 44002 } },
    { 15923, 18611, 18611, 18618, 0, 2, { 45463, 45492, 0, 45521, 45551, 45580, 45610, 45640, 0 } },
    { 15923, 18639, 18639, 18442, 0, 1, { 45670, 45711, 0, 45752, 45794, 45835, 45877, 45919, 0 } },
    { 15923, 7112, 7112, 18618, 0, 2, { 45463, 45492, 0, 45521, 45551, 45580, 45610, 45640, 0 } },
    { 15923, 18646, 18646, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 15923, 19047, 19047, 46235, 1, 1, { 45961, 46006, 0, 0, 0, 46051, 46097, 46143, 46189 } },
    { 15923, 19084, 19084, 18442, 0, 1, { 46243, 46283, 0, 46323, 46364, 46404, 46445, 46486, 0 } },
    { 15923, 7136, 7136, 46527, 2, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 15923, 19090, 19090, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 15923, 19099, 19099, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 15923, 19105, 19105, 46904, 1, 1, { 46540, 46585, 46630, 46675, 46721, 0, 46766, 46812, 46858 } },
    { 15923, 19134, 19134, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 15923, 19140, 19140, 19146, 1, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 47292, 47296, 47296, 1275, 1, 1, { 46911, 46958, 0, 47005, 47053, 47100, 47148, 47196, 47244 } },
    { 47292, 47339, 47339, 0, 0, -1, { 0, 0, 0, 47309, 0, 0, 0, 0, 0 } },
    { 47292, 47344, 47344, 0, 0, -1, { 0, 0, 1246, 0, 0, 0, 0, 0, 0 } },
    { 47292, 47355, 47355, 47360, 0, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 0 } },
    { 47292, 47370, 47370, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 47292, 47379, 47379, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 47292, 47386, 47386, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 47292, 47392, 47392, 0, 0, -1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 47399, 47410, 47410, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 47399, 8236, 8236, 47416, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 47399, 47431, 8236, 9797, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 47439, 47449, 47449, 47453, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 47439, 7907, 7907, 47459, 0, 1, { 0, 2872, 2921, 2970, 3020, 3069, 3119, 0, 3169 } },
    { 47439, 7917, 7917, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 47472, 47410, 47410, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 47472, 8236, 8236, 47416, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 47472, 47431, 8236, 9797, 1, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 16257, 16664, 16664, 25982, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 16257, 16673, 16673, 25982, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 16257, 17511, 17511, 25982, 1, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 16257, 17747, 17747, 26347, 1, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { 16257, 13386, 13386, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
};
