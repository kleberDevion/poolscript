/*
 * .xlsx — ver ps_xlsx.h. ZIP à mão sobre zlib + XML com expat.
 */
#define _GNU_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <zlib.h>
#include <expat.h>

#include "ps_xlsx.h"

/* ── grade ──────────────────────────────────────────────────────────────── */
void ps_grade_init(PSGrade *g) { memset(g, 0, sizeof(*g)); }

void ps_grade_libera(PSGrade *g)
{
    for (int r = 0; r < g->nlin; r++) {
        for (int c = 0; c < g->ncols[r]; c++) free(g->celulas[r][c]);
        free(g->celulas[r]);
        free(g->tipos[r]);
    }
    free(g->celulas);
    free(g->tipos);
    free(g->ncols);
    ps_grade_init(g);
}

static int grade_garante_linha(PSGrade *g, int lin)
{
    if (lin < g->nlin) return 0;
    if (lin + 1 > g->cap) {
        int nc = g->cap < 8 ? 8 : g->cap;
        while (nc < lin + 1) nc *= 2;
        /* Cada campo é PUBLICADO assim que o realloc dele dá certo.
         *
         * Guardar os três pra publicar no fim parecia mais limpo e era
         * ponteiro pendurado: o realloc que dá certo JÁ liberou o bloco
         * antigo, então o `free` no erro de um dos outros deixava
         * `g->celulas` apontando pra memória morta — e o destrutor passa lá
         * liberando de novo. Mesma forma do C1 no compilador. */
        char ***nce = realloc(g->celulas, sizeof(char **) * (size_t)nc);
        if (!nce) return -1;
        g->celulas = nce;
        char **nt = realloc(g->tipos, sizeof(char *) * (size_t)nc);
        if (!nt) return -1;
        g->tipos = nt;
        int *nnc = realloc(g->ncols, sizeof(int) * (size_t)nc);
        if (!nnc) return -1;
        g->ncols = nnc;
        g->cap = nc;
    }
    for (int r = g->nlin; r <= lin; r++) { g->celulas[r] = NULL; g->tipos[r] = NULL; g->ncols[r] = 0; }
    g->nlin = lin + 1;
    return 0;
}

int ps_grade_set(PSGrade *g, int lin, int col, const char *valor, char tipo)
{
    if (grade_garante_linha(g, lin) != 0) return -1;
    if (col >= g->ncols[lin]) {
        /* Publica cada um na hora — ver `grade_garante_linha` acima. Este
         * ponto foi o que a varredura de falha de alocação pegou: leitura de
         * bloco já liberado em `ps_xlsx_escreve` e liberação inválida no
         * finalizador (`make oom`, xlsx, alocação 140). */
        char **nn = realloc(g->celulas[lin], sizeof(char *) * (size_t)(col + 1));
        if (!nn) return -1;
        g->celulas[lin] = nn;
        char *nt = realloc(g->tipos[lin], sizeof(char) * (size_t)(col + 1));
        if (!nt) return -1;
        g->tipos[lin] = nt;
        for (int c = g->ncols[lin]; c <= col; c++) { g->celulas[lin][c] = NULL; g->tipos[lin][c] = 0; }
        g->ncols[lin] = col + 1;
    }
    free(g->celulas[lin][col]);
    g->celulas[lin][col] = strdup(valor ? valor : "");
    g->tipos[lin][col] = tipo;
    return g->celulas[lin][col] ? 0 : -1;
}

char ps_grade_tipo(const PSGrade *g, int lin, int col)
{
    if (lin < 0 || lin >= g->nlin || col < 0 || col >= g->ncols[lin]) return 0;
    return g->tipos[lin][col];
}

const char *ps_grade_get(const PSGrade *g, int lin, int col)
{
    if (lin < 0 || lin >= g->nlin || col < 0 || col >= g->ncols[lin]) return "";
    return g->celulas[lin][col] ? g->celulas[lin][col] : "";
}

/* ── ZIP: leitura ───────────────────────────────────────────────────────── */
/* Só o necessário pra xlsx: DEFLATE e STORED, sem ZIP64. */
static uint32_t le32(const unsigned char *p) { return p[0]|(p[1]<<8)|(p[2]<<16)|((uint32_t)p[3]<<24); }
static uint16_t le16(const unsigned char *p) { return (uint16_t)(p[0]|(p[1]<<8)); }

/* Extrai a entrada de nome `nome` do ZIP em memória. `*saida` (malloc) e
 * `*nsaida`. 0 se achou, -1 se não. */
static int zip_extrai(const unsigned char *zip, size_t nzip, const char *nome,
                      char **saida, size_t *nsaida)
{
    /* acha o End Of Central Directory (assina 0x06054b50), do fim pra trás */
    if (nzip < 22) return -1;
    size_t i = nzip - 22;
    for (;;) {
        if (le32(zip + i) == 0x06054b50) break;
        if (i == 0) return -1;
        i--;
    }
    uint16_t nent = le16(zip + i + 10);
    uint32_t cd = le32(zip + i + 16);
    size_t p = cd;
    for (int e = 0; e < nent; e++) {
        if (p + 46 > nzip || le32(zip + p) != 0x02014b50) return -1;
        uint16_t metodo = le16(zip + p + 10);
        uint32_t comp = le32(zip + p + 20);
        uint32_t orig = le32(zip + p + 24);
        uint16_t nlen = le16(zip + p + 28);
        uint16_t elen = le16(zip + p + 30);
        uint16_t clen = le16(zip + p + 32);
        uint32_t lho = le32(zip + p + 42);
        /* Tudo que vem do arquivo é tratado como hostil: cada deslocamento e
         * cada tamanho é conferido contra `nzip` ANTES de ser usado, e as
         * contas são em `size_t` — `lho + 30` em 32 bits dava a volta com
         * `lho = 0xFFFFFFFF` e passava na checagem. Sem isto o `memcmp` do
         * nome (até 65535 bytes), o `memcpy` do STORED e o `inflate` liam
         * fora do buffer num `.xlsx` corrompido. */
        if ((size_t)p + 46 + nlen > nzip) return -1;
        int bate = (nlen == strlen(nome)) && memcmp(zip + p + 46, nome, nlen) == 0;
        if (bate) {
            if (lho == 0xFFFFFFFFu) return -1;            /* marcador zip64: não suportado */
            size_t lh = lho;
            /* local header: recalcula os campos de nome/extra locais */
            if (lh + 30 > nzip || le32(zip + lh) != 0x04034b50) return -1;
            uint16_t lnlen = le16(zip + lh + 26);
            uint16_t lelen = le16(zip + lh + 28);
            size_t off = lh + 30 + (size_t)lnlen + (size_t)lelen;
            if (off > nzip || comp > nzip - off) return -1;
            if (metodo == 0 && orig != comp) return -1;   /* STORED: tamanhos iguais por definição */
            const unsigned char *dados = zip + off;
            char *out = malloc((size_t)orig + 1);
            if (!out) return -1;
            if (metodo == 0) {                       /* STORED */
                memcpy(out, dados, orig);
            } else {                                  /* DEFLATE */
                z_stream zs; memset(&zs, 0, sizeof(zs));
                if (inflateInit2(&zs, -15) != Z_OK) { free(out); return -1; }
                zs.next_in = (Bytef *)dados; zs.avail_in = comp;
                zs.next_out = (Bytef *)out; zs.avail_out = orig;
                int rc = inflate(&zs, Z_FINISH);
                inflateEnd(&zs);
                if (rc != Z_STREAM_END && rc != Z_OK) { free(out); return -1; }
            }
            out[orig] = '\0';
            *saida = out; *nsaida = orig;
            return 0;
        }
        p += 46 + nlen + elen + clen;
    }
    return -1;
}

static unsigned char *le_arquivo_todo(const char *caminho, size_t *n)
{
    FILE *f = fopen(caminho, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END); long t = ftell(f); rewind(f);
    if (t < 0) { fclose(f); return NULL; }
    unsigned char *b = malloc((size_t)t + 1);
    if (!b) { fclose(f); return NULL; }
    size_t lidos = fread(b, 1, (size_t)t, f);
    fclose(f);
    b[lidos] = 0;
    *n = lidos;
    return b;
}

/* ── parse do XML (expat) ───────────────────────────────────────────────── */
/* pequeno buffer de string */
/* `falhou` existe porque estes acumuladores são usados dentro dos callbacks do
 * expat, que não têm como devolver erro. O realloc que falha marca a flag e
 * para de escrever; quem montou o Buf confere a flag no fim. Sem isso, o
 * `x = realloc(x, …)` perdia o bloco antigo e o memcpy seguinte escrevia em
 * NULL. */
typedef struct { char *b; size_t n, cap; int falhou; } Buf;

/* sharedStrings.xml: <si><t>texto</t></si>. Coletamos o texto de cada <si>. */
static void buf_add(Buf *b, const char *s, int n) {
    if (b->falhou) return;
    if (b->n + (size_t)n + 1 > b->cap) {
        size_t c=b->cap<64?64:b->cap; while(c<b->n+n+1)c*=2;
        char *nb = realloc(b->b, c);
        if (!nb) { b->falhou = 1; return; }
        b->b=nb; b->cap=c;
    }
    memcpy(b->b + b->n, s, (size_t)n); b->n += n; b->b[b->n]=0;
}

typedef struct {
    char **strs; int nstrs, cap;      /* shared strings */
    Buf cur; int em_t;
} SSTCtx;

static void XMLCALL sst_ini(void *u, const XML_Char *nome, const XML_Char **at) {
    (void)at; SSTCtx *c = u;
    if (strcmp(nome,"t")==0) { c->em_t=1; c->cur.n=0; if(c->cur.b) c->cur.b[0]=0; }
    else if (strcmp(nome,"si")==0) { c->cur.n=0; if(c->cur.b) c->cur.b[0]=0; }
}
static void XMLCALL sst_txt(void *u, const XML_Char *s, int len) {
    SSTCtx *c = u; if (c->em_t) buf_add(&c->cur, s, len);
}
static void XMLCALL sst_fim(void *u, const XML_Char *nome) {
    SSTCtx *c = u;
    if (strcmp(nome,"t")==0) c->em_t=0;
    else if (strcmp(nome,"si")==0) {
        if (c->nstrs+1 > c->cap) {
            size_t nc = c->cap<16?16:c->cap*2;
            char **ns = realloc(c->strs, sizeof(char*)*nc);
            /* Callback do expat não devolve erro; marca no `cur` e para, como
             * o `buf_add`. Sem isso, `c->strs = NULL` perdia a tabela inteira
             * de strings e a linha seguinte escrevia em NULL. */
            if (!ns) { c->cur.falhou = 1; return; }
            c->strs = ns; c->cap = nc;
        }
        c->strs[c->nstrs++] = strdup(c->cur.b ? c->cur.b : "");
    }
}

/* sheet1.xml: <row r=N><c r="A1" t="s"><v>idx</v></c>...</row> */
typedef struct {
    PSGrade *g;
    char **sst; int nsst;
    int lin, col;          /* posição corrente (0-based) */
    char tipo;             /* 's' shared, 'n'/outros = direto */
    Buf v; int em_v;
    int erro;
} SheetCtx;

/* "A1" -> col 0-based, lin 0-based via a parte numérica */
static void ref_para_rc(const char *ref, int *col, int *lin) {
    int c = 0, i = 0;
    while (ref[i] >= 'A' && ref[i] <= 'Z') { c = c*26 + (ref[i]-'A'+1); i++; }
    *col = c > 0 ? c - 1 : 0;
    *lin = ref[i] ? atoi(ref+i) - 1 : 0;
}

static void XMLCALL sh_ini(void *u, const XML_Char *nome, const XML_Char **at) {
    SheetCtx *s = u;
    if (strcmp(nome,"c")==0) {
        s->tipo = 'n';
        const char *ref = NULL;
        for (int i=0; at[i]; i+=2) {
            if (strcmp(at[i],"r")==0) ref=at[i+1];
            else if (strcmp(at[i],"t")==0) s->tipo = at[i+1][0];
        }
        if (ref) { int c,l; ref_para_rc(ref,&c,&l); s->col=c; s->lin=l; }
    } else if (strcmp(nome,"v")==0 || strcmp(nome,"t")==0) {
        s->em_v=1; s->v.n=0; if(s->v.b) s->v.b[0]=0;
    }
}
static void XMLCALL sh_txt(void *u, const XML_Char *t, int len) {
    SheetCtx *s = u; if (s->em_v) buf_add(&s->v, t, len);
}
static void XMLCALL sh_fim(void *u, const XML_Char *nome) {
    SheetCtx *s = u;
    if (strcmp(nome,"v")==0 || strcmp(nome,"t")==0) {
        s->em_v=0;
        const char *val = s->v.b ? s->v.b : "";
        char tg;
        if (s->tipo=='s') {                 /* índice na shared strings */
            int idx = atoi(val);
            val = (idx>=0 && idx<s->nsst) ? s->sst[idx] : "";
            tg = 's';
        } else if (s->tipo=='n' || s->tipo==0) {
            tg = 'n';                        /* número */
        } else {
            tg = 's';                        /* inlineStr, str, bool, etc */
        }
        if (ps_grade_set(s->g, s->lin, s->col, val, tg) != 0) s->erro=1;
    } else if (strcmp(nome,"c")==0) {
        s->col++;                           /* fallback se a próxima não tem ref */
    }
}

int ps_xlsx_le(const char *caminho, PSGrade *g, char *erro, size_t ecap)
{
    ps_grade_init(g);
    size_t nzip;
    unsigned char *zip = le_arquivo_todo(caminho, &nzip);
    if (!zip) { snprintf(erro, ecap, "nao consegui abrir '%s'", caminho); return -1; }

    /* shared strings (pode não existir) */
    SSTCtx sc; memset(&sc, 0, sizeof(sc));
    char *ss; size_t nss;
    if (zip_extrai(zip, nzip, "xl/sharedStrings.xml", &ss, &nss) == 0) {
        XML_Parser p = XML_ParserCreate(NULL);
        XML_SetUserData(p, &sc);
        XML_SetElementHandler(p, sst_ini, sst_fim);
        XML_SetCharacterDataHandler(p, sst_txt);
        XML_Parse(p, ss, (int)nss, 1);
        XML_ParserFree(p);
        free(ss);
        /* Os callbacks do expat não devolvem erro, então uma falha de memória
         * lá dentro só chega aqui pela flag. Sem conferir, a tabela de textos
         * viria PELA METADE e a planilha abriria com célula em branco no lugar
         * do dado — flag que ninguém lê é o mesmo silêncio de não ter flag. */
        if (sc.cur.falhou) {
            free(zip);
            for (int i = 0; i < sc.nstrs; i++) free(sc.strs[i]);
            free(sc.strs); free(sc.cur.b);
            snprintf(erro, ecap, "sem memoria lendo os textos de '%s'", caminho);
            return -1;
        }
    }

    /* a primeira planilha */
    char *sheet; size_t nsheet;
    if (zip_extrai(zip, nzip, "xl/worksheets/sheet1.xml", &sheet, &nsheet) != 0) {
        free(zip);
        for (int i=0;i<sc.nstrs;i++) free(sc.strs[i]);
        free(sc.strs); free(sc.cur.b);
        snprintf(erro, ecap, "xlsx sem xl/worksheets/sheet1.xml");
        return -1;
    }
    SheetCtx sh; memset(&sh, 0, sizeof(sh));
    sh.g = g; sh.sst = sc.strs; sh.nsst = sc.nstrs;
    XML_Parser p = XML_ParserCreate(NULL);
    XML_SetUserData(p, &sh);
    XML_SetElementHandler(p, sh_ini, sh_fim);
    XML_SetCharacterDataHandler(p, sh_txt);
    int ok = XML_Parse(p, sheet, (int)nsheet, 1);
    XML_ParserFree(p);
    free(sheet); free(sh.v.b);
    free(zip);
    for (int i=0;i<sc.nstrs;i++) free(sc.strs[i]);
    free(sc.strs); free(sc.cur.b);
    if (ok == XML_STATUS_ERROR || sh.erro) {
        ps_grade_libera(g);
        snprintf(erro, ecap, "xlsx corrompido");
        return -1;
    }
    return 0;
}

/* ── ZIP: escrita ───────────────────────────────────────────────────────── */
/* Mesmo desenho do `Buf`: `out_add` é chamado dezenas de vezes seguidas pra
 * montar o ZIP e não tem como devolver erro em cada uma. Marca e para. */
typedef struct { unsigned char *b; size_t n, cap; int falhou; } Out;
static void out_add(Out *o, const void *d, size_t n) {
    if (o->falhou) return;
    if (o->n + n > o->cap) {
        size_t c=o->cap<4096?4096:o->cap; while(c<o->n+n)c*=2;
        unsigned char *nb = realloc(o->b, c);
        if (!nb) { o->falhou = 1; return; }
        o->b=nb; o->cap=c;
    }
    memcpy(o->b + o->n, d, n); o->n += n;
}
static void out32(Out *o, uint32_t v) { unsigned char b[4]={v&255,(v>>8)&255,(v>>16)&255,(v>>24)&255}; out_add(o,b,4); }
static void out16(Out *o, uint16_t v) { unsigned char b[2]={v&255,(v>>8)&255}; out_add(o,b,2); }

/* CRC32 e deflate via zlib. */
typedef struct { char *nome; unsigned char *comp; size_t ncomp, norig; uint32_t crc; uint32_t off; } ZEntry;

static int zip_add(Out *zip, ZEntry *e, const char *nome, const char *dados, size_t n)
{
    /* Em falha esta função tem que deixar `e` VAZIO: quem chama libera de 0 a
     * i-1 e não toca na entrada que falhou, então o que ficar aqui não é
     * liberado por ninguém. */
    e->nome = NULL; e->comp = NULL; e->ncomp = 0;
    e->nome = strdup(nome);
    if (!e->nome) return -1;
    e->norig = n;
    e->crc = crc32(0, (const Bytef *)dados, (uInt)n);
    /* raw deflate */
    uLongf cap = compressBound((uLong)n) + 16;
    unsigned char *comp = malloc(cap);
    if (!comp) { free(e->nome); e->nome = NULL; return -1; }
    z_stream zs; memset(&zs,0,sizeof(zs));
    deflateInit2(&zs, Z_DEFAULT_COMPRESSION, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY);
    zs.next_in=(Bytef*)dados; zs.avail_in=(uInt)n;
    zs.next_out=comp; zs.avail_out=(uInt)cap;
    deflate(&zs, Z_FINISH);
    e->ncomp = cap - zs.avail_out;
    deflateEnd(&zs);
    e->comp = comp;
    e->off = (uint32_t)zip->n;
    /* local file header */
    out32(zip, 0x04034b50); out16(zip,20); out16(zip,0); out16(zip,8);  /* deflate */
    out16(zip,0); out16(zip,0);                                          /* time/date */
    out32(zip, e->crc); out32(zip,(uint32_t)e->ncomp); out32(zip,(uint32_t)e->norig);
    out16(zip,(uint16_t)strlen(nome)); out16(zip,0);
    out_add(zip, nome, strlen(nome));
    out_add(zip, comp, e->ncomp);
    return 0;
}

static void xml_escapa(Buf *b, const char *s) {
    for (; *s; s++) {
        switch (*s) {
            case '&': buf_add(b,"&amp;",5); break;
            case '<': buf_add(b,"&lt;",4); break;
            case '>': buf_add(b,"&gt;",4); break;
            case '"': buf_add(b,"&quot;",6); break;
            default: buf_add(b, s, 1);
        }
    }
}

/* col 0-based -> "A","B",...,"AA" */
static void col_letra(int col, char *out) {
    char tmp[8]; int n=0; col++;
    while (col>0) { int r=(col-1)%26; tmp[n++]=(char)('A'+r); col=(col-1)/26; }
    for (int i=0;i<n;i++) out[i]=tmp[n-1-i];
    out[n]=0;
}

int ps_xlsx_escreve(const char *caminho, const PSGrade *g, char *erro, size_t ecap)
{
    /* sheet1.xml com todas as células como inlineStr (t="inlineStr") — evita a
     * tabela de shared strings e ainda abre em qualquer leitor de planilha */
    Buf sheet = {0};
    const char *decl = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>";
    buf_add(&sheet, decl, (int)strlen(decl));
    const char *hdr = "<worksheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\"><sheetData>";
    buf_add(&sheet, hdr, (int)strlen(hdr));
    for (int r=0; r<g->nlin; r++) {
        char lin[32]; int nl=snprintf(lin,sizeof(lin),"<row r=\"%d\">",r+1);
        buf_add(&sheet, lin, nl);
        for (int c=0; c<g->ncols[r]; c++) {
            const char *val = ps_grade_get(g, r, c);
            if (!val || !val[0]) continue;
            char letra[8]; col_letra(c, letra);
            if (ps_grade_tipo(g, r, c) == 'n') {
                char cab[48]; int nc=snprintf(cab,sizeof(cab),"<c r=\"%s%d\"><v>",letra,r+1);
                buf_add(&sheet, cab, nc);
                xml_escapa(&sheet, val);
                buf_add(&sheet, "</v></c>", 8);
            } else {
                char cab[80]; int nc=snprintf(cab,sizeof(cab),"<c r=\"%s%d\" t=\"inlineStr\"><is><t xml:space=\"preserve\">",letra,r+1);
                buf_add(&sheet, cab, nc);
                xml_escapa(&sheet, val);
                buf_add(&sheet, "</t></is></c>", 13);
            }
        }
        buf_add(&sheet, "</row>", 6);
    }
    buf_add(&sheet, "</sheetData></worksheet>", 24);

    static const char *CT =
      "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
      "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">"
      "<Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>"
      "<Default Extension=\"xml\" ContentType=\"application/xml\"/>"
      "<Override PartName=\"/xl/workbook.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml\"/>"
      "<Override PartName=\"/xl/worksheets/sheet1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml\"/>"
      "</Types>";
    static const char *RELS =
      "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
      "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
      "<Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument\" Target=\"xl/workbook.xml\"/>"
      "</Relationships>";
    static const char *WB =
      "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
      "<workbook xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" "
      "xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">"
      "<sheets><sheet name=\"Sheet1\" sheetId=\"1\" r:id=\"rId1\"/></sheets></workbook>";
    static const char *WBR =
      "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
      "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
      "<Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet\" Target=\"worksheets/sheet1.xml\"/>"
      "</Relationships>";

    struct { const char *nome; const char *dados; size_t n; } partes[] = {
        {"[Content_Types].xml", CT, strlen(CT)},
        {"_rels/.rels", RELS, strlen(RELS)},
        {"xl/workbook.xml", WB, strlen(WB)},
        {"xl/_rels/workbook.xml.rels", WBR, strlen(WBR)},
        {"xl/worksheets/sheet1.xml", sheet.b, sheet.n},
    };
    int np = 5;
    Out zip = {0};
    ZEntry ents[5];
    for (int i=0;i<np;i++)
        if (zip_add(&zip, &ents[i], partes[i].nome, partes[i].dados, partes[i].n) != 0) {
            snprintf(erro, ecap, "sem memoria"); free(sheet.b); free(zip.b);
            for (int k=0;k<i;k++){free(ents[k].nome);free(ents[k].comp);}
            return -1;
        }
    /* central directory */
    uint32_t cd_ini = (uint32_t)zip.n;
    for (int i=0;i<np;i++) {
        ZEntry *e=&ents[i];
        out32(&zip,0x02014b50); out16(&zip,20); out16(&zip,20); out16(&zip,0); out16(&zip,8);
        out16(&zip,0); out16(&zip,0);
        out32(&zip,e->crc); out32(&zip,(uint32_t)e->ncomp); out32(&zip,(uint32_t)e->norig);
        out16(&zip,(uint16_t)strlen(e->nome)); out16(&zip,0); out16(&zip,0);
        out16(&zip,0); out16(&zip,0); out32(&zip,0); out32(&zip,e->off);
        out_add(&zip, e->nome, strlen(e->nome));
    }
    uint32_t cd_tam = (uint32_t)zip.n - cd_ini;
    out32(&zip,0x06054b50); out16(&zip,0); out16(&zip,0);
    out16(&zip,(uint16_t)np); out16(&zip,(uint16_t)np);
    out32(&zip,cd_tam); out32(&zip,cd_ini); out16(&zip,0);

    int rc = 0;
    if (zip.falhou || sheet.falhou) {
        snprintf(erro, ecap, "sem memoria montando '%s'", caminho);
        rc = -1;
    } else {
        FILE *f = fopen(caminho, "wb");
        if (!f) { snprintf(erro, ecap, "nao consegui escrever '%s'", caminho); rc=-1; }
        else {
            /* Os três têm que ser conferidos. O `fwrite` só enche o buffer da
             * libc; o disco cheio costuma aparecer no flush, e o flush é o
             * `fclose`. Descartar o retorno dele é gravar, fechar, e o arquivo
             * estar truncado sem ninguém saber. */
            size_t esc = fwrite(zip.b, 1, zip.n, f);
            int erro_fluxo = ferror(f);
            if (fclose(f) != 0 || erro_fluxo || esc != zip.n) {
                snprintf(erro, ecap, "falha gravando '%s' (disco cheio?)", caminho);
                rc = -1;
            }
        }
    }

    free(sheet.b); free(zip.b);
    for (int i=0;i<np;i++){free(ents[i].nome);free(ents[i].comp);}
    return rc;
}
