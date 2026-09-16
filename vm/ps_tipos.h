/*
 * Consulta à tabela de tipos (`ps_tipos.def`) para quem não é a VM: parser,
 * compilador e `--metadata`. A VM monta a própria `TIPOS[]` (com os testes de
 * pertinência) do mesmo `.def`.
 */
#ifndef PS_TIPOS_H
#define PS_TIPOS_H

#include <string.h>

enum {
#define PS_TIPO(suf, nome, aceita, decl, expr, count, model) PS_TIPO_##suf,
#define PS_APELIDO(grafia, suf)
#include "ps_tipos.def"
#undef PS_TIPO
#undef PS_APELIDO
    PS_TIPO__N
};

typedef struct {
    const char *nome;
    int cod;
    unsigned char decl, expr, count, model;
} PSTipoInfo;

static const PSTipoInfo PS_TIPOS_INFO[] = {
#define PS_TIPO(suf, nome, aceita, decl, expr, count, model) \
    { nome, PS_TIPO_##suf, decl, expr, count, model },
#define PS_APELIDO(grafia, suf)
#include "ps_tipos.def"
#undef PS_TIPO
#undef PS_APELIDO
};

static const struct { const char *grafia; int cod; } PS_TIPOS_APELIDOS[] = {
#define PS_TIPO(suf, nome, aceita, decl, expr, count, model)
#define PS_APELIDO(grafia, suf) { grafia, PS_TIPO_##suf },
#include "ps_tipos.def"
#undef PS_TIPO
#undef PS_APELIDO
};

#define PS_N_TIPOS    ((int)(sizeof(PS_TIPOS_INFO) / sizeof(PS_TIPOS_INFO[0])))
#define PS_N_APELIDOS ((int)(sizeof(PS_TIPOS_APELIDOS) / sizeof(PS_TIPOS_APELIDOS[0])))

/* O tipo por nome canônico ou apelido; NULL se o nome não é tipo da tabela
 * (aí pode ser classe, Entity ou objeto nativo — quem decide é o chamador). */
static inline const PSTipoInfo *ps_tipo_info(const char *nome)
{
    if (!nome) return NULL;
    for (int i = 0; i < PS_N_TIPOS; i++)
        if (strcmp(PS_TIPOS_INFO[i].nome, nome) == 0) return &PS_TIPOS_INFO[i];
    for (int i = 0; i < PS_N_APELIDOS; i++)
        if (strcmp(PS_TIPOS_APELIDOS[i].grafia, nome) == 0)
            return &PS_TIPOS_INFO[PS_TIPOS_APELIDOS[i].cod];
    return NULL;
}

/* O código do tipo (a ordem da tabela, que é a do bytecode), ou -1. */
static inline int ps_tipo_codigo(const char *nome)
{
    const PSTipoInfo *t = ps_tipo_info(nome);
    return t ? t->cod : -1;
}

/* O nome canônico (`String` → `str`), ou NULL se não é tipo da tabela. */
static inline const char *ps_tipo_canonico(const char *nome)
{
    const PSTipoInfo *t = ps_tipo_info(nome);
    return t ? t->nome : NULL;
}

#endif /* PS_TIPOS_H */
