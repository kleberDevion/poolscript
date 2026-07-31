/*
 * MongoDB — ver ps_mongo.h. Ponte JSON<->BSON via libbson.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mongoc/mongoc.h>

#include "ps_mongo.h"

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
                  int um_so, char **json, char *erro, size_t ecap)
{
    bson_error_t be;
    bson_t *q = bson_de_json(query_json, &be);
    if (!q) { snprintf(erro, ecap, "erro de banco de dados: query invalida: %s", be.message); return -1; }
    mongoc_collection_t *c = mongoc_client_get_collection(m->cli, m->dbname, col);
    mongoc_cursor_t *cur = mongoc_collection_find_with_opts(c, q, NULL, NULL);

    /* monta um array JSON com os docs */
    size_t cap = 256, n = 1;
    char *out = malloc(cap);
    out[0] = '['; out[1] = 0;
    const bson_t *doc;
    int primeiro = 1;
    while (mongoc_cursor_next(cur, &doc)) {
        size_t jlen;
        char *dj = bson_as_relaxed_extended_json(doc, &jlen);
        if (!dj) continue;
        if (n + jlen + 4 > cap) { while (n + jlen + 4 > cap) cap *= 2; out = realloc(out, cap); }
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
