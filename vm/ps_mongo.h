/*
 * MongoDB para o psodbc — via libmongoc/libbson (dinâmicas; não têm `.a`).
 *
 * A ponte com a VM é JSON: a VM já tem parser e serializador JSON próprios,
 * então em vez de iterar BSON à mão, converte-se Value->JSON->BSON na entrada
 * e BSON->JSON(relaxado)->Value na saída. O `_id` do mongo vem como
 * {"$oid": "..."} no JSON relaxado e é removido pela VM, como no wrapper.
 *
 * Todas as funções que devolvem JSON alocam `*json` (o chamador libera).
 */
#ifndef PS_MONGO_H
#define PS_MONGO_H

#include <stddef.h>

typedef struct PSMongo PSMongo;

PSMongo *ps_mongo_conecta(const char *uri, const char *dbname, char *erro, size_t ecap);
void     ps_mongo_fecha(PSMongo *m);

/* find: `*json` recebe um array JSON com os docs (ou "[]"). 0/-1. */
int ps_mongo_find(PSMongo *m, const char *col, const char *query_json,
                  int um_so, char **json, char *erro, size_t ecap);
/* insert/insert_many/update/remove: 0/-1. `doc_json` é objeto (insert) ou
 * array (insert_many). update usa {$set: set_json}. */
int ps_mongo_insert(PSMongo *m, const char *col, const char *doc_json, int muitos,
                    char *erro, size_t ecap);
int ps_mongo_update(PSMongo *m, const char *col, const char *query_json,
                    const char *set_json, char *erro, size_t ecap);
int ps_mongo_remove(PSMongo *m, const char *col, const char *query_json,
                    char *erro, size_t ecap);
long ps_mongo_count(PSMongo *m, const char *col, const char *query_json,
                    char *erro, size_t ecap);

#endif /* PS_MONGO_H */
