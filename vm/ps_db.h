/*
 * Bancos SQL para o módulo psodbc — sqlite, postgres, mysql, mssql(odbc).
 *
 * Uma interface só sobre as libs C dos drivers (libpq, libmysqlclient, unixODBC
 * e a sqlite que já entra). O resultado de um SELECT é bufferizado como uma
 * grade de células com tipo (int/float/str/bool/null), pra a VM montar a lista
 * de dicts com os tipos certos — igual ao que psycopg2/mysql.connector devolvem.
 */
#ifndef PS_DB_H
#define PS_DB_H

#include <stddef.h>
#include <stdint.h>

typedef enum { PS_DB_SQLITE, PS_DB_POSTGRES, PS_DB_MYSQL, PS_DB_MSSQL } PSDbDriver;

typedef enum { PS_CEL_NULL, PS_CEL_INT, PS_CEL_FLOAT, PS_CEL_STR, PS_CEL_BOOL } PSCelTipo;

typedef struct {
    PSCelTipo tipo;
    char     *txt;      /* representação textual (int/float/str); NULL se nulo */
    int64_t   i;
    double    f;
    int       b;
} PSCel;

typedef struct {
    char   **cols;      /* nomes das colunas */
    int      ncols;
    PSCel  **linhas;    /* [linha][coluna] */
    int      nlinhas;
    int      cap;
    int64_t  rowcount;  /* linhas afetadas (DML) ou -1 */
    int      tem_result;/* 1 se foi SELECT (tem colunas) */
} PSDbRes;

typedef struct PSDbConn PSDbConn;

/* Conecta. `base` é o arquivo (sqlite). Devolve NULL e escreve `erro`. */
PSDbConn *ps_db_conecta(PSDbDriver drv, const char *host, int porta,
                        const char *user, const char *senha, const char *db,
                        const char *base, char *erro, size_t ecap);

/* Executa. Em SELECT preenche `res` (bufferizado); em DML preenche rowcount.
 * 0/-1 (erro em `erro`). `params`/`nparams` são substituídos nos `?`/`$n`. */
/* `params` são textos (NULL = SQL NULL); ligados nos `?`/`$n`. */
/* `tipo_out` recebe o nome da classe de erro (ex: "UndefinedTable" no pg);
 * pode ser NULL. */
int ps_db_exec(PSDbConn *c, const char *sql, const char **params, int nparams,
               PSDbRes *res, char *erro, size_t ecap, char *tipo_out, size_t tcap);

void ps_db_res_libera(PSDbRes *r);
void ps_db_fecha(PSDbConn *c);
void ps_db_solta(PSDbConn *c);   /* fecha sem protocolo — caminho do GC */

#endif /* PS_DB_H */
