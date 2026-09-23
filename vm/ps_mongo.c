/*
 * MongoDB — ver ps_mongo.h. Ponte JSON<->BSON via libbson.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mongoc/mongoc.h>

#include "ps_mongo.h"
#include "ps_dl.h"

/* O cliente do Mongo NÃO é ligado ao executável: abre no primeiro `connect`
 * (ver ps_dl.h). Era ele que arrastava o gnutls, o SASL e o criptografador do
 * Mongo pra partida de todo programa. Esta lista é TODA função das duas
 * bibliotecas que este arquivo chama, inclusive as que vêm por macro:
 * `BSON_APPEND_DOCUMENT` -> `bson_append_document`, `BSON_APPEND_INT64` ->
 * `bson_append_int64`, `BSON_ITER_HOLDS_DOCUMENT` -> `bson_iter_type`. Faltar
 * uma não passa calado: sem a biblioteca na ligação, o nome não existe. */
#define MG_FUNCS(X) X(bson_append_document) X(bson_append_int64) X(bson_iter_type)       \
    X(bson_as_relaxed_extended_json)                                                     \
    X(bson_destroy) X(bson_free) X(bson_init_from_json) X(bson_init_static)               \
    X(bson_iter_document) X(bson_iter_init) X(bson_iter_next) X(bson_new)                 \
    X(bson_new_from_json) X(mongoc_bulk_operation_destroy) X(mongoc_bulk_operation_execute) \
    X(mongoc_bulk_operation_insert) X(mongoc_client_destroy) X(mongoc_client_get_collection) \
    X(mongoc_client_get_database) X(mongoc_client_new_from_uri)                          \
    X(mongoc_collection_count_documents) X(mongoc_collection_create_bulk_operation_with_opts) \
    X(mongoc_collection_delete_many) X(mongoc_collection_destroy)                         \
    X(mongoc_collection_find_with_opts) X(mongoc_collection_insert_one)                   \
    X(mongoc_collection_update_many) X(mongoc_cursor_destroy) X(mongoc_cursor_error)      \
    X(mongoc_cursor_next) X(mongoc_database_destroy) X(mongoc_init) X(mongoc_uri_destroy)  \
    X(mongoc_uri_new_with_error)

MG_FUNCS(PS_DL_PONTEIRO)

static int  g_mg_estado;
static char g_mg_motivo[320];
static const char *const MG_NOMES[] = { "libmongoc-1.0.so.0", "libmongoc-1.0.so", NULL };

#define bson_append_document dl_bson_append_document
#define bson_append_int64 dl_bson_append_int64
#define bson_iter_type dl_bson_iter_type
#define bson_as_relaxed_extended_json dl_bson_as_relaxed_extended_json
#define bson_destroy dl_bson_destroy
#define bson_free dl_bson_free
#define bson_init_from_json dl_bson_init_from_json
#define bson_init_static dl_bson_init_static
#define bson_iter_document dl_bson_iter_document
#define bson_iter_init dl_bson_iter_init
#define bson_iter_next dl_bson_iter_next
#define bson_new dl_bson_new
#define bson_new_from_json dl_bson_new_from_json
#define mongoc_bulk_operation_destroy dl_mongoc_bulk_operation_destroy
#define mongoc_bulk_operation_execute dl_mongoc_bulk_operation_execute
#define mongoc_bulk_operation_insert dl_mongoc_bulk_operation_insert
#define mongoc_client_destroy dl_mongoc_client_destroy
#define mongoc_client_get_collection dl_mongoc_client_get_collection
#define mongoc_client_get_database dl_mongoc_client_get_database
#define mongoc_client_new_from_uri dl_mongoc_client_new_from_uri
#define mongoc_collection_count_documents dl_mongoc_collection_count_documents
#define mongoc_collection_create_bulk_operation_with_opts dl_mongoc_collection_create_bulk_operation_with_opts
#define mongoc_collection_delete_many dl_mongoc_collection_delete_many
#define mongoc_collection_destroy dl_mongoc_collection_destroy
#define mongoc_collection_find_with_opts dl_mongoc_collection_find_with_opts
#define mongoc_collection_insert_one dl_mongoc_collection_insert_one
#define mongoc_collection_update_many dl_mongoc_collection_update_many
#define mongoc_cursor_destroy dl_mongoc_cursor_destroy
#define mongoc_cursor_error dl_mongoc_cursor_error
#define mongoc_cursor_next dl_mongoc_cursor_next
#define mongoc_database_destroy dl_mongoc_database_destroy
#define mongoc_init dl_mongoc_init
#define mongoc_uri_destroy dl_mongoc_uri_destroy
#define mongoc_uri_new_with_error dl_mongoc_uri_new_with_error

struct PSMongo {
    mongoc_client_t   *cli;
    mongoc_database_t *db;
    char              *dbname;
    int                iniciado;
};

/* mongoc_init uma vez por processo */
static int mongo_global = 0;

PSMongo *ps_mongo_conecta(const char *uri, const char *dbname, char *erro, size_t ecap)
{
    PS_DL_CARREGA(g_mg_estado, g_mg_motivo, MG_NOMES, "o modulo mongo", MG_FUNCS, erro, ecap);
    if (g_mg_estado != 1) return NULL;
    if (!mongo_global) { mongoc_init(); mongo_global = 1; }
    bson_error_t be;
    mongoc_uri_t *u = mongoc_uri_new_with_error(uri, &be);
    if (!u) { snprintf(erro, ecap, "falha de conexão: %s", be.message); return NULL; }
    mongoc_client_t *cli = mongoc_client_new_from_uri(u);
    mongoc_uri_destroy(u);
    if (!cli) { snprintf(erro, ecap, "falha de conexão: URI invalida"); return NULL; }
    PSMongo *m = calloc(1, sizeof(PSMongo));
    if (!m) { mongoc_client_destroy(cli); snprintf(erro, ecap, "sem memoria"); return NULL; }
    m->cli = cli;
    m->dbname = strdup(dbname && dbname[0] ? dbname : "test");
    m->db = mongoc_client_get_database(m->cli, m->dbname);
    return m;
}

void ps_mongo_fecha(PSMongo *m)
{
    if (!m) return;
    if (m->db) mongoc_database_destroy(m->db);
    if (m->cli) mongoc_client_destroy(m->cli);
    free(m->dbname);
    free(m);
}

/* JSON -> bson_t (novo). NULL/"" vira documento vazio. */
static bson_t *bson_de_json(const char *json, bson_error_t *be)
{
    if (!json || !json[0]) return bson_new();
    return bson_new_from_json((const uint8_t *)json, -1, be);
}

int ps_mongo_find(PSMongo *m, const char *col, const char *query_json,
                  int um_so, long skip, long limit, const char *sort_json,
                  char **json, char *erro, size_t ecap)
{
    bson_error_t be;
    bson_t *q = bson_de_json(query_json, &be);
    if (!q) { snprintf(erro, ecap, "erro de banco de dados: query invalida: %s", be.message); return -1; }
    /* skip/limit/sort vão pro servidor: ele pula e corta antes de mandar.
     * Sem opts, o cursor trazia a coleção inteira pela rede. */
    bson_t *opts = bson_new();
    if (skip > 0) BSON_APPEND_INT64(opts, "skip", (int64_t)skip);
    long lim = um_so ? 1 : limit;
    if (lim > 0) BSON_APPEND_INT64(opts, "limit", (int64_t)lim);
    if (sort_json) {
        bson_t *s = bson_de_json(sort_json, &be);
        if (!s) {
            bson_destroy(opts); bson_destroy(q);
            snprintf(erro, ecap, "erro de banco de dados: sort invalido: %s", be.message);
            return -1;
        }
        BSON_APPEND_DOCUMENT(opts, "sort", s);
        bson_destroy(s);
    }
    mongoc_collection_t *c = mongoc_client_get_collection(m->cli, m->dbname, col);
    mongoc_cursor_t *cur = mongoc_collection_find_with_opts(c, q, opts, NULL);

    /* monta um array JSON com os docs */
    size_t cap = 256, n = 1;
    char *out = malloc(cap);
    if (!out) {
        mongoc_cursor_destroy(cur); mongoc_collection_destroy(c); bson_destroy(q); bson_destroy(opts);
        snprintf(erro, ecap, "sem memoria");
        return -1;
    }
    out[0] = '['; out[1] = 0;
    const bson_t *doc;
    int primeiro = 1;
    while (mongoc_cursor_next(cur, &doc)) {
        size_t jlen;
        char *dj = bson_as_relaxed_extended_json(doc, &jlen);
        if (!dj) continue;
        if (n + jlen + 4 > cap) {
            while (n + jlen + 4 > cap) cap *= 2;
            /* Por temporária: o realloc que falha devolve NULL sem liberar o
             * antigo, e `out = NULL` perdia o buffer inteiro montado até aqui
             * — mais o `memcpy` em NULL logo abaixo. */
            char *novo = realloc(out, cap);
            if (!novo) {
                bson_free(dj); free(out);
                mongoc_cursor_destroy(cur); mongoc_collection_destroy(c); bson_destroy(q); bson_destroy(opts);
                snprintf(erro, ecap, "sem memoria");
                return -1;
            }
            out = novo;
        }
        if (!primeiro) out[n++] = ',';
        memcpy(out + n, dj, jlen); n += jlen; out[n] = 0;
        bson_free(dj);
        primeiro = 0;
        if (um_so) break;
    }
    int falhou = mongoc_cursor_error(cur, &be);
    mongoc_cursor_destroy(cur);
    mongoc_collection_destroy(c);
    bson_destroy(q);
    bson_destroy(opts);
    if (falhou) { free(out); snprintf(erro, ecap, "erro de banco de dados: %s", be.message); return -1; }
    out[n++] = ']'; out[n] = 0;
    *json = out;
    return 0;
}

int ps_mongo_insert(PSMongo *m, const char *col, const char *doc_json, int muitos,
                    char *erro, size_t ecap)
{
    bson_error_t be;
    mongoc_collection_t *c = mongoc_client_get_collection(m->cli, m->dbname, col);
    int rc = 0;
    if (muitos) {
        /* doc_json é array [ {...}, {...} ] — parseia cada elemento */
        bson_t arr;
        if (!bson_init_from_json(&arr, doc_json, -1, &be)) {
            mongoc_collection_destroy(c); snprintf(erro,ecap,"erro de banco de dados: %s", be.message); return -1;
        }
        bson_iter_t it;
        bson_iter_init(&it, &arr);
        mongoc_bulk_operation_t *bulk = mongoc_collection_create_bulk_operation_with_opts(c, NULL);
        int algum = 0;
        while (bson_iter_next(&it)) {
            if (!BSON_ITER_HOLDS_DOCUMENT(&it)) continue;
            const uint8_t *d; uint32_t dl;
            bson_iter_document(&it, &dl, &d);
            bson_t sub;
            if (bson_init_static(&sub, d, dl)) { mongoc_bulk_operation_insert(bulk, &sub); algum = 1; }
        }
        bson_t reply;
        if (algum && !mongoc_bulk_operation_execute(bulk, &reply, &be)) { rc = -1; snprintf(erro,ecap,"erro de banco de dados: %s", be.message); }
        bson_destroy(&reply);
        mongoc_bulk_operation_destroy(bulk);
        bson_destroy(&arr);
    } else {
        bson_t *d = bson_de_json(doc_json, &be);
        if (!d) { mongoc_collection_destroy(c); snprintf(erro,ecap,"erro de banco de dados: doc invalido: %s", be.message); return -1; }
        if (!mongoc_collection_insert_one(c, d, NULL, NULL, &be)) { rc = -1; snprintf(erro,ecap,"erro de banco de dados: %s", be.message); }
        bson_destroy(d);
    }
    mongoc_collection_destroy(c);
    return rc;
}

int ps_mongo_update(PSMongo *m, const char *col, const char *query_json,
                    const char *set_json, char *erro, size_t ecap)
{
    bson_error_t be;
    bson_t *q = bson_de_json(query_json, &be);
    if (!q) { snprintf(erro,ecap,"erro de banco de dados: query invalida: %s", be.message); return -1; }
    bson_t *sv = bson_de_json(set_json, &be);
    if (!sv) { bson_destroy(q); snprintf(erro,ecap,"erro de banco de dados: set invalido: '%s' -> %s", set_json?set_json:"(null)", be.message); return -1; }
    bson_t *upd = bson_new();
    bson_append_document(upd, "$set", 4, sv);
    mongoc_collection_t *c = mongoc_client_get_collection(m->cli, m->dbname, col);
    int rc = mongoc_collection_update_many(c, q, upd, NULL, NULL, &be) ? 0 : -1;
    if (rc) snprintf(erro, ecap, "erro de banco de dados: %s", be.message);
    mongoc_collection_destroy(c);
    bson_destroy(q); bson_destroy(sv); bson_destroy(upd);
    return rc;
}

int ps_mongo_remove(PSMongo *m, const char *col, const char *query_json,
                    char *erro, size_t ecap)
{
    bson_error_t be;
    bson_t *q = bson_de_json(query_json, &be);
    if (!q) { snprintf(erro,ecap,"erro de banco de dados: query invalida"); return -1; }
    mongoc_collection_t *c = mongoc_client_get_collection(m->cli, m->dbname, col);
    int rc = mongoc_collection_delete_many(c, q, NULL, NULL, &be) ? 0 : -1;
    if (rc) snprintf(erro, ecap, "erro de banco de dados: %s", be.message);
    mongoc_collection_destroy(c);
    bson_destroy(q);
    return rc;
}

long ps_mongo_count(PSMongo *m, const char *col, const char *query_json,
                    char *erro, size_t ecap)
{
    bson_error_t be;
    bson_t *q = bson_de_json(query_json, &be);
    if (!q) { snprintf(erro,ecap,"erro de banco de dados: query invalida"); return -1; }
    mongoc_collection_t *c = mongoc_client_get_collection(m->cli, m->dbname, col);
    int64_t n = mongoc_collection_count_documents(c, q, NULL, NULL, NULL, &be);
    mongoc_collection_destroy(c);
    bson_destroy(q);
    if (n < 0) { snprintf(erro, ecap, "erro de banco de dados: %s", be.message); return -1; }
    return (long)n;
}
