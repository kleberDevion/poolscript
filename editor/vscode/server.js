'use strict';

const {
  createConnection, ProposedFeatures, TextDocuments, TextDocumentSyncKind,
  CompletionItemKind, DiagnosticSeverity, SymbolKind, MarkupKind,
} = require('vscode-languageserver/node');
const { TextDocument } = require('vscode-languageserver-textdocument');
const { execFileSync } = require('child_process');
const fs = require('fs');
const path = require('path');

const conexao = createConnection(ProposedFeatures.all);
const docs = new TextDocuments(TextDocument);

/* ── o motor ─────────────────────────────────────────────────────────────
 *
 * `POOL` vem do cliente (initializationOptions) ou do PATH. É o mesmo binário
 * que o usuário roda: apontar pra outro faria o completion descrever um motor
 * que não é o dele. */
let POOL = 'pool';
let RAIZ_DOC = null;

function motor(args, entrada) {
  try {
    return execFileSync(POOL, args, {
      input: entrada === undefined ? '' : entrada,
      encoding: 'utf8',
      maxBuffer: 32 * 1024 * 1024,
      timeout: 10000,
    });
  } catch (e) {
    /* `--check` sai com 1 quando o programa não compila: a saída ainda é o
     * JSON que interessa, então erro de código de saída não é erro aqui. */
    if (e && typeof e.stdout === 'string') return e.stdout;
    return '';
  }
}

/* Módulos, tipos e membros, das tabelas do VM. Lido uma vez. */
let META = { modulos: {}, tipos: {}, acesso: {} };
function carregaMeta() {
  const bruto = motor(['--metadata']);
  if (bruto.trim().startsWith('{')) {
    try { META = JSON.parse(bruto); } catch (_) { /* fica o vazio */ }
  }
}

/* Onde `docs/` está: ao lado do binário instalado, ou na raiz do repositório. */
function raizDoc() {
  if (RAIZ_DOC !== null) return RAIZ_DOC;
  let base = '.';
  try {
    const onde = execFileSync('sh', ['-c', `command -v ${POOL}`], { encoding: 'utf8' }).trim();
    if (onde) base = path.dirname(fs.realpathSync(onde));
  } catch (_) { /* usa o cwd */ }
  const cands = [
    path.join(base, '..', 'share', 'poolscript', 'docs'),
    path.join(base, 'docs'),
    'docs',
  ];
  RAIZ_DOC = cands.find((c) => { try { return fs.statSync(c).isDirectory(); } catch (_) { return false; } }) || '';
  return RAIZ_DOC;
}

/* O primeiro parágrafo de prosa da página do membro, ou "". */
const RESUMOS = new Map();
function resumoDe(escopo, nome) {
  const chave = `${escopo}/${nome}`;
  if (RESUMOS.has(chave)) return RESUMOS.get(chave);
  RESUMOS.set(chave, '');
  const raiz = raizDoc();
  if (!raiz) return '';
  const p = path.join(raiz, escopo, nome, `${nome}.md`);
  let texto;
  try { texto = fs.readFileSync(p, 'utf8'); } catch (_) { return ''; }
  const partes = [];
  let passouTitulo = false;
  for (const linha of texto.split('\n')) {
    const t = linha.trim();
    if (!passouTitulo) { if (t.startsWith('# ')) passouTitulo = true; continue; }
    if (t === '') { if (partes.length) break; continue; }
    if (t.startsWith('#') || t.startsWith('|') || t.startsWith('```')) break;
    partes.push(t);
  }
  RESUMOS.set(chave, partes.join(' '));
  return RESUMOS.get(chave);
}

/* ── tokens: a única fonte de verdade sobre o que é código ────────────────
 *
 * `pool --tokens` devolve a lista do LEXER. `STR` e `COMMENT` chegam como UM
 * token cada, com o conteúdo dentro — então parêntese, ponto e vírgula dentro
 * de string ou comentário simplesmente NÃO EXISTEM como pontuação. Todos os
 * defeitos de detecção do servidor antigo somem por construção. */
const CACHE_TOK = new Map();     // uri -> {versao, toks}

function tokensDe(doc) {
  const c = CACHE_TOK.get(doc.uri);
  if (c && c.versao === doc.version) return c.toks;
  const bruto = motor(['--tokens'], doc.getText());
  let toks = [];
  if (bruto.trim().startsWith('[')) {
    try { toks = JSON.parse(bruto); } catch (_) { toks = []; }
  }
  /* o lexer conta linha/coluna de 1; o LSP de 0 */
  for (const t of toks) { t.l0 = t.l - 1; t.c0 = t.c - 1; }
  CACHE_TOK.set(doc.uri, { versao: doc.version, toks });
  return toks;
}

/* Um token pode ser NOME de módulo/tipo mesmo sendo palavra da linguagem:
 * `json`, `str`, `list`, `dict`, `tup` chegam do lexer como `KW`, não `IDENT`.
 * Exigir `IDENT` fazia `import json as js` não entrar na tabela de imports —
 * e o `js.` continuava dando zero, agora por outro motivo. */
function ehNome(t) {
  if (!t) return false;
  if (t.t === 'IDENT' || t.t === 'IDENT_UPPER') return true;
  return t.t === 'KW' && (!!META.modulos[t.v] || !!META.tipos[t.v]);
}

/* IDENT **ou** IDENT_UPPER.
 *
 * O lexer separa os dois porque o nome de Entity começa com maiúscula
 * (docs/linguagem/01-estrutura-lexica.md §1.4). Este arquivo exigia `IDENT`
 * em `simbolosDoDoc` e em `membrosDeArquivo` — e com isso TODA Entity, de
 * todo arquivo, era invisível pro servidor. Não era um detalhe de completion:
 * era a razão de `self.` não oferecer nada, de `c = Conta(...)` seguido de
 * `c.` não oferecer nada, e de o nome da classe nem aparecer na lista. */
function ehIdent(t) { return !!t && (t.t === 'IDENT' || t.t === 'IDENT_UPPER'); }

/* Palavra que pode ser TIPO numa declaração de campo (`str nome`, `B dono`). */
const TIPOS_KW = ['str', 'int', 'flo', 'bool', 'char', 'list', 'json', 'dict', 'tup'];
function ehTipo(t) {
  if (!t) return false;
  if (t.t === 'IDENT_UPPER') return true;
  return t.t === 'KW' && TIPOS_KW.indexOf(t.v) >= 0;
}

/* Índice do `}` que casa com o `{` em `iAbre`. */
function fechaBloco(toks, iAbre) {
  let prof = 0;
  for (let k = iAbre; k < toks.length; k++) {
    if (toks[k].t === 'LBRACE') prof++;
    else if (toks[k].t === 'RBRACE') { prof--; if (prof === 0) return k; }
  }
  return toks.length - 1;
}

/* Índice do primeiro token que COMEÇA em ou depois da posição. */
function corteEm(toks, pos) {
  let i = 0;
  while (i < toks.length) {
    const t = toks[i];
    if (t.l0 > pos.line || (t.l0 === pos.line && t.c0 >= pos.character)) return i;
    i++;
  }
  return toks.length;
}

/* O token que CONTÉM a posição, ou null. Serve pra saber se o cursor está
 * dentro de string/comentário — onde não se completa nada. */
function tokenEm(toks, pos) {
  for (const t of toks) {
    if (t.l0 !== pos.line) continue;
    if (pos.character > t.c0 && pos.character <= t.c0 + t.n) return t;
  }
  return null;
}

function dentroDeTextoLivre(toks, pos) {
  const t = tokenEm(toks, pos);
  return !!t && (t.t === 'STR' || t.t === 'COMMENT');
}

/* ── imports do documento: o `as` e as libs que não são do motor ───────────
 *
 * Sai dos TOKENS, então `# import x as y` num comentário não conta.
 * Devolve  nomeLigado -> {mod, arquivo}. `arquivo` é o .ps quando o import
 * não é módulo do motor — a ordem de resolução é a da linguagem
 * (docs/linguagem/09-imports.md §9.5): stdlib, lib global, arquivo do projeto. */
function pastaLibs() {
  const h = process.env.HOME;
  return h ? path.join(h, '.poolscript', 'libs') : '';
}

/* Os nomes instalados em `~/.poolscript/libs` — o que `import <nome>` acha. */
function libsInstaladas() {
  try {
    return fs.readdirSync(pastaLibs())
      .filter((f) => f.endsWith('.ps'))
      .map((f) => f.slice(0, -3));
  } catch (_) { return []; }
}

function arquivoDoImport(mod, dirDoc) {
  const rel = mod.split('.').join(path.sep) + '.ps';
  const libs = pastaLibs();
  for (const base of [libs, dirDoc]) {
    if (!base) continue;
    const p = path.join(base, rel);
    try { if (fs.statSync(p).isFile()) return p; } catch (_) { /* segue */ }
  }
  return '';
}

function importsDe(doc) {
  const toks = tokensDe(doc);
  const dirDoc = doc.uri.startsWith('file://') ? path.dirname(doc.uri.slice(7)) : '';
  const tab = new Map();
  for (let i = 0; i < toks.length; i++) {
    const t = toks[i];
    const ehImport = t.t === 'KW' && (t.v === 'import' || t.v === 'PUSH' || t.v === 'push');
    if (!ehImport) continue;
    /* caminho pontuado */
    const segs = [];
    let j = i + 1;
    while (j < toks.length && (toks[j].t === 'IDENT' || toks[j].t === 'KW')) {
      if (toks[j].t === 'KW' && (toks[j].v === 'as' || toks[j].v === 'from')) break;
      segs.push(toks[j].v);
      if (j + 1 < toks.length && toks[j + 1].t === 'DOT') { j += 2; continue; }
      j++;
      break;
    }
    if (!segs.length) continue;
    const mod = segs.join('.');
    let ligado = segs[segs.length - 1];
    if (j < toks.length && toks[j].t === 'KW' && toks[j].v === 'as'
        && j + 1 < toks.length && (toks[j + 1].t === 'IDENT' || toks[j + 1].t === 'KW')) {
      ligado = toks[j + 1].v;      /* ← o `as`, que o servidor antigo perdia */
    }
    tab.set(ligado, {
      mod,
      arquivo: META.modulos[mod] ? '' : arquivoDoImport(mod, dirDoc),
    });
  }
  return tab;
}

/* Os símbolos de topo de OUTRO arquivo .ps — o que `import` dele oferece.
 *
 * Também pelos tokens do motor: uma `action` citada num comentário do arquivo
 * importado não vira membro. */
const CACHE_ARQ = new Map();     // caminho -> {mtime, membros, ents}

function leArquivo(caminho) {
  if (!caminho) return { membros: [], ents: [] };
  let mtime;
  try { mtime = fs.statSync(caminho).mtimeMs; } catch (_) { return { membros: [], ents: [] }; }
  const c = CACHE_ARQ.get(caminho);
  if (c && c.mtime === mtime) return c;

  let texto;
  try { texto = fs.readFileSync(caminho, 'utf8'); } catch (_) { return { membros: [], ents: [] }; }
  const bruto = motor(['--tokens'], texto);
  let toks = [];
  if (bruto.trim().startsWith('[')) { try { toks = JSON.parse(bruto); } catch (_) { toks = []; } }

  const ents = entidadesEmToks(toks);
  for (const e of ents) e.arquivo = caminho;   /* "ir pra definição" atravessa arquivo */
  const dentroDeEntity = (i) => ents.some((e) => i > e.iIni && i <= e.iFim);

  const membros = [];
  const vistos = new Set();
  let prof = 0;
  for (let i = 0; i < toks.length; i++) {
    const t = toks[i];
    if (t.t === 'LBRACE' || t.t === 'INDENT') prof++;
    else if (t.t === 'RBRACE' || t.t === 'DEDENT') prof = Math.max(0, prof - 1);
    const kw = t.t === 'KW' ? t.v : '';
    const decl = (kw === 'action' || kw === 'reaction') ? 'action'
               : (kw === 'class' || kw === 'Class' || kw === 'Entity') ? 'class' : '';
    if (!decl) continue;
    /* o método de uma Entity não é membro do MÓDULO — mas a Entity é, e ela
     * abre um bloco, então contar só `prof === 0` deixava a classe de fora
     * quando o `{` dela ficava na linha de baixo. */
    if (decl === 'action' && (prof !== 0 || dentroDeEntity(i))) continue;
    const ident = toks[i + 1];
    if (!ehIdent(ident)) continue;
    const nome = ident.v;
    if (nome.startsWith('_') || vistos.has(nome)) continue;
    vistos.add(nome);
    membros.push({
      nome,
      params: decl === 'action' ? paramsDaDeclaracao(toks, i + 2) : [],
      retorna: null,
      kind: decl,
      linha: ident.l - 1,
      coluna: ident.c - 1,
    });
  }
  const r = { mtime, membros, ents };
  CACHE_ARQ.set(caminho, r);
  return r;
}

function membrosDeArquivo(caminho) { return leArquivo(caminho).membros; }
function entidadesDeArquivo(caminho) { return leArquivo(caminho).ents; }

/* Os parâmetros de uma declaração, a partir do índice do `(`. */
function paramsDaDeclaracao(toks, iAbre) {
  if (!toks[iAbre] || toks[iAbre].t !== 'LPAREN') return [];
  const ps = [];
  let prof = 0;
  let nome = '';
  let padrao = null;
  let depoisDoIgual = false;
  for (let k = iAbre + 1; k < toks.length; k++) {
    const t = toks[k];
    if (t.t === 'LPAREN' || t.t === 'LBRACK' || t.t === 'LBRACE') { prof++; continue; }
    if (t.t === 'RPAREN' && prof === 0) break;
    if (t.t === 'RPAREN' || t.t === 'RBRACK' || t.t === 'RBRACE') { prof--; continue; }
    if (prof !== 0) continue;
    if (t.t === 'COMMA') {
      if (nome) ps.push({ nome, default: padrao });
      nome = ''; padrao = null; depoisDoIgual = false;
      continue;
    }
    if (t.t === 'OP' && t.v === '=') { depoisDoIgual = true; continue; }
    if (depoisDoIgual) { if (padrao === null) padrao = t.v; continue; }
    /* o ÚLTIMO IDENT antes da vírgula é o nome: `int x` tem o tipo na frente */
    if (t.t === 'IDENT') nome = t.v;
  }
  if (nome) ps.push({ nome, default: padrao });
  return ps;
}

/* ── Entity: campos, métodos, visibilidade e herança ──────────────────────
 *
 * Nada disto existia. O servidor indexava `action` de topo e mais nada, então
 * as três coisas que ele cobrou — classe, herança, `self` — não tinham como
 * funcionar: não havia onde procurar a resposta.
 *
 * Cada entrada guarda o INTERVALO DE TOKENS do corpo. É por ele que se sabe
 * em qual Entity o cursor está — e portanto o que `self.` deve oferecer,
 * incluindo o que é `private` (de dentro, private é visível; é a regra que a
 * VM impõe em runtime, e o completion tem que contar a mesma história). */

/* Campos que o CORPO de um método cria: `self.x = …` e a declaração com
 * visibilidade `private str nome = …` (docs/linguagem/07-entity.md §7.6.1).
 * Sem isto, uma Entity que monta tudo no `__init__` — que é como ele
 * escreve — não teria campo nenhum. */
function camposDoMetodo(toks, iIni, iFim, membros) {
  const jaTem = new Set(membros.map((m) => m.nome));
  for (let i = iIni; i < iFim; i++) {
    const t = toks[i];
    const anterior = toks[i - 1];
    /* `private str name = nome` */
    if (ehTipo(t) && ehIdent(toks[i + 1])
        && toks[i + 2] && toks[i + 2].t === 'OP' && toks[i + 2].v === '='
        && anterior && anterior.t === 'KW'
        && (anterior.v === 'private' || anterior.v === 'public')) {
      const nt = toks[i + 1];
      if (!jaTem.has(nt.v)) {
        jaTem.add(nt.v);
        membros.push({ nome: nt.v, kind: 'campo', privado: anterior.v === 'private',
                       tipo: t.v, linha: nt.l - 1, coluna: nt.c - 1 });
      }
      continue;
    }
    /* `self.x = …` — campo dinâmico (§7.2) */
    if (t.t === 'KW' && t.v === 'self' && toks[i + 1] && toks[i + 1].t === 'DOT'
        && ehIdent(toks[i + 2]) && toks[i + 3] && toks[i + 3].t === 'OP'
        && /^[-+*/%]?=$/.test(toks[i + 3].v)) {
      const nt = toks[i + 2];
      if (!jaTem.has(nt.v)) {
        jaTem.add(nt.v);
        membros.push({ nome: nt.v, kind: 'campo', privado: false, tipo: '',
                       linha: nt.l - 1, coluna: nt.c - 1 });
      }
    }
  }
}

/* Membros declarados entre `{` e `}` de uma Entity. */
function membrosNoCorpo(toks, iIni, iFim) {
  const membros = [];
  let priv = false;
  for (let i = iIni + 1; i < iFim; i++) {
    const t = toks[i];
    if (t.t === 'KW' && (t.v === 'private' || t.v === 'public')) {
      priv = t.v === 'private';
      continue;
    }
    /* método: [public|private] [async|tipo]* action|reaction NOME(...) {...} */
    if (t.t === 'KW' && (t.v === 'action' || t.v === 'reaction')) {
      const nt = toks[i + 1];
      if (ehIdent(nt)) {
        const ps = paramsDaDeclaracao(toks, i + 2);
        membros.push({
          nome: nt.v, kind: 'action', privado: priv,
          /* `self` não é argumento de quem chama: `c.deposita(v)` passa UM. */
          params: ps.filter((x) => x.nome !== 'self'),
          linha: nt.l - 1, coluna: nt.c - 1,
        });
      }
      let j = i + 1;
      while (j < iFim && toks[j].t !== 'LBRACE') j++;
      if (j < iFim) {
        const corpo = Math.min(fechaBloco(toks, j), iFim);
        camposDoMetodo(toks, j, corpo, membros);
        i = corpo;
      }
      priv = false;
      continue;
    }
    /* campo `nome: tipo [= valor]` */
    if (ehIdent(t) && toks[i + 1] && toks[i + 1].t === 'COLON') {
      const tt = toks[i + 2];
      membros.push({ nome: t.v, kind: 'campo', privado: priv,
                     tipo: tt ? tt.v : '', linha: t.l - 1, coluna: t.c - 1 });
      priv = false;
      continue;
    }
    /* campo `tipo nome [= valor]` — a outra ordem (§7.2) */
    if (ehTipo(t) && ehIdent(toks[i + 1])) {
      const nt = toks[i + 1];
      membros.push({ nome: nt.v, kind: 'campo', privado: priv,
                     tipo: t.v, linha: nt.l - 1, coluna: nt.c - 1 });
      priv = false;
      continue;
    }
    if (t.t !== 'NEWLINE' && t.t !== 'INDENT' && t.t !== 'DEDENT'
        && t.t !== 'AT' && t.t !== 'KW') priv = false;
  }
  return membros;
}

/* As Entities de uma lista de tokens. */
function entidadesEmToks(toks) {
  const out = [];
  for (let i = 0; i < toks.length; i++) {
    const t = toks[i];
    if (!(t.t === 'KW' && (t.v === 'Entity' || t.v === 'class' || t.v === 'Class'))) continue;
    const nt = toks[i + 1];
    if (!ehIdent(nt)) continue;
    const bases = [];
    let j = i + 2;
    if (toks[j] && toks[j].t === 'LPAREN') {          /* `Entity Conta(Base)` */
      j++;
      while (j < toks.length && toks[j].t !== 'RPAREN') {
        if (ehIdent(toks[j])) bases.push(toks[j].v);
        j++;
      }
      j++;
    }
    while (j < toks.length && toks[j].t !== 'LBRACE' && toks[j].t !== 'RBRACE') j++;
    if (!toks[j] || toks[j].t !== 'LBRACE') continue;
    const fim = fechaBloco(toks, j);
    out.push({
      nome: nt.v, bases, iIni: j, iFim: fim,
      linha: nt.l - 1, coluna: nt.c - 1,
      membros: membrosNoCorpo(toks, j, fim),
    });
    i = fim;                                          /* não reentra no corpo */
  }
  return out;
}

const CACHE_ENT = new Map();     // uri -> {versao, ents}
function entidadesDe(doc) {
  const c = CACHE_ENT.get(doc.uri);
  if (c && c.versao === doc.version) return c.ents;
  const ents = entidadesEmToks(tokensDe(doc));
  CACHE_ENT.set(doc.uri, { versao: doc.version, ents });
  return ents;
}

/* A Entity cujo CORPO contém o cursor — quem responde o `self.`. */
function entidadeEm(doc, pos) {
  const toks = tokensDe(doc);
  const i = corteEm(toks, pos);
  for (const e of entidadesDe(doc)) if (i > e.iIni && i <= e.iFim) return e;
  return null;
}

/* Acha a Entity pelo nome: neste documento ou num arquivo importado. */
function achaEntidade(doc, nome) {
  for (const e of entidadesDe(doc)) if (e.nome === nome) return e;
  for (const [, imp] of importsDe(doc)) {
    if (!imp.arquivo) continue;
    for (const e of entidadesDeArquivo(imp.arquivo)) if (e.nome === nome) return e;
  }
  return null;
}

/* Membros de uma Entity **com herança**, na ordem em que o Python resolve:
 * o da própria classe vence o do pai. `interno` = o cursor está dentro da
 * classe, então `private` conta. */
function membrosDaEntidade(doc, nome, interno, vistos) {
  vistos = vistos || new Set();
  if (vistos.has(nome)) return [];      /* herança circular não trava o editor */
  vistos.add(nome);
  const e = achaEntidade(doc, nome);
  if (!e) return [];
  const out = [];
  const jaTem = new Set();
  for (const m of e.membros) {
    /* `__init__` é o construtor: chama-se escrevendo `Conta(...)`, nunca
     * `c.__init__(...)`. Oferecê-lo é ruído em toda lista de membro. */
    if (m.nome === '__init__') continue;
    if (m.privado && !interno) continue;
    if (jaTem.has(m.nome)) continue;
    jaTem.add(m.nome);
    out.push(Object.assign({ de: nome }, m));
  }
  for (const b of e.bases) {
    /* herdado: o `private` do pai NÃO é visível nem de dentro do filho */
    for (const m of membrosDaEntidade(doc, b, false, vistos)) {
      if (jaTem.has(m.nome)) continue;
      jaTem.add(m.nome);
      out.push(m);
    }
  }
  return out;
}

/* Nomes visíveis no ponto do cursor: parâmetros da action que o contém e o
 * que foi ligado antes dele. Era a outra metade do "não sugere nada": mesmo
 * com as classes indexadas, digitar dentro de uma action não oferecia nem o
 * parâmetro que está no cabeçalho três linhas acima. */
function escopoLocal(doc, pos) {
  const toks = tokensDe(doc);
  const fim = corteEm(toks, pos);
  const out = new Map();

  for (let i = 0; i < fim; i++) {
    const t = toks[i];
    if (!(t.t === 'KW' && (t.v === 'action' || t.v === 'reaction'))) continue;
    let j = i + 1;
    if (ehIdent(toks[j])) j++;
    if (!toks[j] || toks[j].t !== 'LPAREN') continue;
    const ps = paramsDaDeclaracao(toks, j);
    let k = j;
    while (k < toks.length && toks[k].t !== 'LBRACE') k++;
    if (k >= toks.length) continue;
    if (k < fim && fim <= fechaBloco(toks, k)) {
      for (const p of ps) out.set(p.nome, 'parametro');
    }
  }

  for (let i = 0; i + 1 < fim; i++) {
    const t = toks[i];
    if (!ehIdent(t)) continue;
    const nx = toks[i + 1];
    const atribui = nx.t === 'OP' && nx.v === '=';
    const doLaco  = nx.t === 'KW' && nx.v === 'in'
                 && toks[i - 1] && toks[i - 1].t === 'KW' && toks[i - 1].v === 'each';
    /* `a, b = …` — o alvo de desempacotamento também é nome ligado */
    const emTupla = nx.t === 'COMMA' && (() => {
      for (let k = i + 1; k < fim && k < i + 12; k++) {
        if (toks[k].t === 'OP' && toks[k].v === '=') return true;
        if (toks[k].t === 'NEWLINE' || toks[k].t === 'LPAREN') return false;
      }
      return false;
    })();
    if ((atribui || doLaco || emTupla) && !out.has(t.v)) out.set(t.v, 'variavel');
  }
  return out;
}

/* Declarações de topo DO PRÓPRIO documento — para completion, outline e
 * "ir pra definição". */
function simbolosDoDoc(doc) {
  const toks = tokensDe(doc);
  const out = [];
  let prof = 0;
  for (let i = 0; i < toks.length; i++) {
    const t = toks[i];
    if (t.t === 'LBRACE' || t.t === 'INDENT') prof++;
    else if (t.t === 'RBRACE' || t.t === 'DEDENT') prof = Math.max(0, prof - 1);
    const kw = t.t === 'KW' ? t.v : '';
    const decl = (kw === 'action' || kw === 'reaction') ? 'action'
               : (kw === 'class' || kw === 'Class' || kw === 'Entity') ? 'class' : '';
    if (!decl) continue;
    const ident = toks[i + 1];
    if (!ehIdent(ident)) continue;
    out.push({
      nome: ident.v,
      kind: decl,
      topo: prof === 0,
      params: decl === 'action' ? paramsDaDeclaracao(toks, i + 2) : [],
      linha: ident.l - 1,
      coluna: ident.c - 1,
    });
  }
  return out;
}

/* ── a chamada em que o cursor está ───────────────────────────────────────
 *
 * Devolve {chamado, jaPassados, nomeados, iAbre} ou null. Tudo em cima dos
 * tokens: o `(` de dentro de uma string não existe aqui. */
function chamadaEm(doc, pos) {
  const toks = tokensDe(doc);
  const fim = corteEm(toks, pos);

  let prof = 0;
  let iAbre = -1;
  for (let i = fim - 1; i >= 0; i--) {
    const tt = toks[i].t;
    if (tt === 'RPAREN') prof++;
    else if (tt === 'LPAREN') {
      if (prof === 0) { iAbre = i; break; }
      prof--;
    }
  }
  if (iAbre <= 0) return null;

  /* o chamado é a cadeia IDENT (DOT IDENT)* colada antes do `(` */
  const partes = [];
  let j = iAbre - 1;
  let esperaIdent = true;
  while (j >= 0) {
    const t = toks[j];
    if (esperaIdent && ehNome(t)) { partes.unshift(t.v); esperaIdent = false; j--; continue; }
    if (!esperaIdent && t.t === 'DOT') { esperaIdent = true; j--; continue; }
    break;
  }
  if (!partes.length) return null;

  /* o que JÁ foi passado: quantos posicionais e quais nomes */
  let posicionais = 0;
  const nomeados = new Set();
  let prof2 = 0;
  let temAlgo = false;
  for (let k = iAbre + 1; k < fim; k++) {
    const t = toks[k];
    if (t.t === 'LPAREN' || t.t === 'LBRACK' || t.t === 'LBRACE') { prof2++; temAlgo = true; continue; }
    if (t.t === 'RPAREN' || t.t === 'RBRACK' || t.t === 'RBRACE') { prof2--; continue; }
    if (prof2 !== 0) continue;
    if (t.t === 'COMMA') { if (temAlgo) posicionais++; temAlgo = false; continue; }
    if (t.t === 'IDENT' && toks[k + 1] && toks[k + 1].t === 'OP' && toks[k + 1].v === '=') {
      nomeados.add(t.v);
      temAlgo = false;
      k++;
      continue;
    }
    temAlgo = true;
  }
  return { partes, chamado: partes.join('.'), posicionais, nomeados, iAbre };
}

/* Os parâmetros declarados do que está sendo chamado. */
function paramsDoChamado(doc, ch) {
  if (ch.partes.length > 1) {
    const alvo = ch.partes.slice(0, -1).join('.');
    const nome = ch.partes[ch.partes.length - 1];
    for (const m of membrosDoNome(doc, alvo)) if (m.nome === nome) return m.params || [];
    const t = tipoDaVariavel(doc, alvo);
    if (t && META.tipos[t]) {
      for (const m of META.tipos[t]) if (m.nome === nome) return m.params || [];
    }
    return [];
  }
  for (const s of simbolosDoDoc(doc)) if (s.nome === ch.chamado) return s.params || [];
  return [];
}

/* Os membros do que `nome` designa NESTE documento — resolvendo o `as` e as
 * libs instaladas. */
function membrosDoNome(doc, nome) {
  const imp = importsDe(doc);
  if (imp.has(nome)) {
    const e = imp.get(nome);
    if (META.modulos[e.mod]) return META.modulos[e.mod];
    return membrosDeArquivo(e.arquivo);
  }
  if (META.modulos[nome]) return META.modulos[nome];
  return [];
}

/* Tipo de uma variável, pela atribuição mais recente antes do cursor.
 * Só o que dá pra afirmar: literal e retorno declarado de membro conhecido. */
function tipoDaVariavel(doc, nome) {
  const toks = tokensDe(doc);
  let tipo = '';
  for (let i = 0; i + 2 < toks.length; i++) {
    if (!ehIdent(toks[i]) || toks[i].v !== nome) continue;
    /* `str s = "a"` — o tipo está DECLARADO, uma palavra antes */
    if (ehTipo(toks[i - 1]) && toks[i + 1].t === 'OP' && toks[i + 1].v === '=') {
      tipo = toks[i - 1].v;
      continue;
    }
    if (!(toks[i + 1].t === 'OP' && toks[i + 1].v === '=')) continue;
    const d = toks[i + 2];
    if (d.t === 'STR') { tipo = 'str'; continue; }
    if (d.t === 'INT') { tipo = 'int'; continue; }
    if (d.t === 'FLOAT') { tipo = 'flo'; continue; }
    if (d.t === 'LBRACK') { tipo = 'list'; continue; }
    if (d.t === 'LBRACE') { tipo = 'dict'; continue; }
    /* `c = Conta(...)` — instância de Entity. Faltava, e era o caso do dia a
     * dia dele: sem isto, `c.` não tinha o que oferecer. */
    if (ehIdent(d) && toks[i + 3] && toks[i + 3].t === 'LPAREN' && achaEntidade(doc, d.v)) {
      tipo = d.v;
      continue;
    }
    /* `x = mod.membro(...)` — o tipo é o retorno declarado; `x = mod.Classe()`
     * é a Entity daquele arquivo */
    if (ehIdent(d) && toks[i + 3] && toks[i + 3].t === 'DOT' && toks[i + 4]) {
      const imp = importsDe(doc);
      if (imp.has(d.v) && imp.get(d.v).arquivo) {
        for (const e of entidadesDeArquivo(imp.get(d.v).arquivo)) {
          if (e.nome === toks[i + 4].v) { tipo = e.nome; break; }
        }
      }
      for (const m of membrosDoNome(doc, d.v)) {
        if (m.nome === toks[i + 4].v && m.retorna) { tipo = m.retorna; break; }
      }
    }
  }
  return tipo;
}

/* Um item de completion pra um membro de Entity. */
function itemDeMembro(m, deOndeVem) {
  const priv = m.privado ? ' · private' : '';
  const herdado = deOndeVem && m.de && m.de !== deOndeVem ? ` · de ${m.de}` : '';
  if (m.kind === 'action') {
    return {
      label: m.nome,
      kind: CompletionItemKind.Method,
      detail: assinatura(m) + priv + herdado,
      sortText: (m.privado ? '1' : '0') + m.nome,
    };
  }
  return {
    label: m.nome,
    kind: CompletionItemKind.Field,
    detail: (m.tipo ? `${m.tipo} ${m.nome}` : m.nome) + priv + herdado,
    sortText: (m.privado ? '1' : '0') + m.nome,
  };
}

function assinatura(m) {
  const ps = (m.params || []).map((p) => (p.default === null || p.default === undefined)
    ? p.nome : `${p.nome}=${p.default}`);
  const ret = m.retorna ? ` -> ${m.retorna}` : '';
  return `${m.nome}(${ps.join(', ')})${ret}`;
}

/* ── o protocolo ─────────────────────────────────────────────────────────── */

conexao.onInitialize((params) => {
  const op = (params.initializationOptions || {});
  if (op.pool) POOL = op.pool;
  carregaMeta();
  return {
    capabilities: {
      textDocumentSync: TextDocumentSyncKind.Incremental,
      completionProvider: { triggerCharacters: ['.', '('], resolveProvider: false },
      hoverProvider: true,
      definitionProvider: true,
      documentSymbolProvider: true,
      signatureHelpProvider: { triggerCharacters: ['(', ','] },
    },
    serverInfo: { name: 'poolscript-lsp', version: '2' },
  };
});

/* diagnóstico: quem decide é o `--check` do motor, não uma segunda gramática */
function diagnostica(doc) {
  const bruto = motor(['--check'], doc.getText());
  let r;
  try { r = JSON.parse(bruto); } catch (_) { return; }
  if (!r) return;

  /* AVISOS: o programa compila, mas alguma coisa quase certamente não é o que
   * se quis — `"C:\pasta"`, onde `\p` não é escape. Vêm no mesmo JSON do
   * `--check` e viram sublinhado amarelo. Sem isto o aviso só apareceria pra
   * quem roda no terminal, e o editor é justamente onde ele seria visto. */
  const diags = (r.avisos || []).map((a) => {
    const l = Math.max(0, (a.linha || 1) - 1);
    const c = Math.max(0, (a.coluna || 1) - 1);
    return {
      severity: DiagnosticSeverity.Warning,
      range: { start: { line: l, character: c }, end: { line: l, character: c + 2 } },
      message: a.msg,
      source: 'poolscript',
    };
  });

  if (!r.ok) {
    const linha = Math.max(0, (r.linha || 1) - 1);
    const col = Math.max(0, (r.coluna || 1) - 1);
    diags.unshift({
      severity: DiagnosticSeverity.Error,
      range: { start: { line: linha, character: col }, end: { line: linha, character: col + 1 } },
      message: `${r.tipo}: ${r.msg}`,
      source: 'poolscript',
    });
  }
  conexao.sendDiagnostics({ uri: doc.uri, diagnostics: diags });
}

docs.onDidChangeContent((e) => diagnostica(e.document));
docs.onDidClose((e) => { CACHE_TOK.delete(e.document.uri); });

conexao.onCompletion((p) => {
  const doc = docs.get(p.textDocument.uri);
  if (!doc) return [];
  const toks = tokensDe(doc);

  /* DENTRO DE STRING OU COMENTÁRIO NÃO SE COMPLETA NADA. O servidor antigo
   * despejava a lista de módulos inteira aqui — era o "vai em qualquer
   * coisa". */
  if (dentroDeTextoLivre(toks, p.position)) return [];

  const linha = doc.getText({
    start: { line: p.position.line, character: 0 },
    end: p.position,
  });

  /* `import <cursor>` / `from <cursor>` — AQUI é onde os módulos do motor
   * fazem sentido, e só aqui.
   *
   * Antes eles entravam na lista de QUALQUER lugar do arquivo, com
   * "(precisa de import)" no detalhe: digitar `f` oferecia `flask`. Um nome
   * que o arquivo não importou não é candidato a nada — é ruído com cara de
   * sugestão. */
  if (/(^|\s)(import|from|PUSH)\s+[A-Za-z0-9_.]*$/.test(linha)) {
    const itens = Object.keys(META.modulos || {})
      .filter((m) => m.indexOf('.') < 0)
      .map((m) => ({
        label: m,
        kind: CompletionItemKind.Module,
        detail: `modulo do motor — ${(META.modulos[m] || []).length} membros`,
        documentation: { kind: MarkupKind.Markdown, value: resumoDe(m, m) },
      }));
    for (const nome of libsInstaladas()) {
      itens.push({ label: nome, kind: CompletionItemKind.Module, detail: 'lib instalada' });
    }
    return itens;
  }

  /* `self.` — os membros da Entity que CONTÉM o cursor, herdados inclusive.
   * De dentro da classe o `private` aparece: é a mesma regra que a VM impõe
   * (docs/linguagem/07-entity.md §7.6), e o completion não pode contar outra
   * história. */
  if (/(^|[^A-Za-z0-9_.])self\.\s*$/.test(linha)) {
    const ent = entidadeEm(doc, p.position);
    if (!ent) return [];
    return membrosDaEntidade(doc, ent.nome, true).map((m) => itemDeMembro(m, ent.nome));
  }

  /* `alvo.` — membros do que o alvo designa */
  const mDot = /([A-Za-z_][A-Za-z0-9_.]*)\.\s*$/.exec(linha);
  if (mDot) {
    const alvo = mDot[1];

    /* variável que guarda uma Entity, ou o nome da própria Entity (membro
     * @static). De FORA da classe, `private` não é oferecido. */
    const tipoEnt = achaEntidade(doc, alvo) ? alvo : tipoDaVariavel(doc, alvo);
    if (tipoEnt && achaEntidade(doc, tipoEnt)) {
      const dentro = (() => { const e = entidadeEm(doc, p.position); return !!e && e.nome === tipoEnt; })();
      return membrosDaEntidade(doc, tipoEnt, dentro).map((m) => itemDeMembro(m, tipoEnt));
    }

    const membros = membrosDoNome(doc, alvo);
    if (membros.length) {
      const imp = importsDe(doc);
      const escopo = imp.has(alvo) ? imp.get(alvo).mod : alvo;
      return membros.map((m) => ({
        label: m.nome,
        kind: m.kind === 'class' ? CompletionItemKind.Class : CompletionItemKind.Function,
        detail: assinatura(m),
        documentation: { kind: MarkupKind.Markdown, value: resumoDe(escopo, m.nome) },
      }));
    }
    const t = tipoDaVariavel(doc, alvo);
    if (t && META.tipos[t]) {
      return (META.tipos[t] || []).concat(META.tipos.__universal__ || []).map((m) => ({
        label: m.nome,
        kind: CompletionItemKind.Method,
        detail: assinatura(m),
        documentation: { kind: MarkupKind.Markdown, value: resumoDe(t, m.nome) },
      }));
    }
    return [];              /* tipo desconhecido: nada de chute */
  }

  /* dentro dos parênteses de uma chamada: os parâmetros QUE AINDA CABEM */
  const ch = chamadaEm(doc, p.position);
  if (ch) {
    const ps = paramsDoChamado(doc, ch);
    const faltam = ps.filter((x, i) => i >= ch.posicionais && !ch.nomeados.has(x.nome));
    if (faltam.length) {
      return faltam.map((x) => ({
        label: `${x.nome}=`,
        kind: CompletionItemKind.Variable,
        detail: x.default === null || x.default === undefined
          ? 'parametro' : `parametro (padrao ${x.default})`,
      }));
    }
    return [];
  }

  /* Sem receptor: o que está REALMENTE em escopo aqui.
   *
   * A ordem é a de utilidade, e o `sortText` a impõe contra a ordenação
   * alfabética do editor: primeiro o que está a três linhas de distância
   * (parâmetro, variável, `self`), depois o do arquivo, depois o importado.
   * Módulo do motor NÃO entra — ver o ramo do `import` acima. */
  const itens = [];
  const jaTem = new Set();
  const poe = (label, kind, detail, ordem) => {
    if (jaTem.has(label)) return;
    jaTem.add(label);
    itens.push({ label, kind, detail, sortText: ordem + label });
  };

  for (const [nome, tipo] of escopoLocal(doc, p.position)) {
    poe(nome, tipo === 'parametro' ? CompletionItemKind.Variable : CompletionItemKind.Variable,
        tipo, '0');
  }

  /* dentro de uma Entity: `self` e os membros dela sem qualificar */
  const ent = entidadeEm(doc, p.position);
  if (ent) {
    poe('self', CompletionItemKind.Keyword, `a instância de ${ent.nome}`, '0');
  }

  for (const e of entidadesDe(doc)) {
    poe(e.nome, CompletionItemKind.Class,
        `class ${e.nome}` + (e.bases.length ? `(${e.bases.join(', ')})` : ''), '1');
  }
  for (const s of simbolosDoDoc(doc)) {
    if (!s.topo || s.kind !== 'action') continue;
    poe(s.nome, CompletionItemKind.Function, assinatura(s), '1');
  }
  for (const [ligado, e] of importsDe(doc)) {
    poe(ligado, CompletionItemKind.Module,
        e.mod === ligado ? 'modulo' : `modulo ${e.mod} (as ${ligado})`, '2');
  }
  return itens;
});

conexao.onSignatureHelp((p) => {
  const doc = docs.get(p.textDocument.uri);
  if (!doc) return null;
  const ch = chamadaEm(doc, p.position);
  if (!ch) return null;
  const ps = paramsDoChamado(doc, ch);
  if (!ps.length) return null;
  const rotulos = ps.map((x) => (x.default === null || x.default === undefined)
    ? x.nome : `${x.nome}=${x.default}`);
  return {
    signatures: [{
      label: `${ch.chamado}(${rotulos.join(', ')})`,
      parameters: rotulos.map((r) => ({ label: r })),
    }],
    activeSignature: 0,
    activeParameter: Math.min(ch.posicionais, Math.max(0, ps.length - 1)),
  };
});

conexao.onHover((p) => {
  const doc = docs.get(p.textDocument.uri);
  if (!doc) return null;
  const toks = tokensDe(doc);
  if (dentroDeTextoLivre(toks, p.position)) return null;
  const t = tokenEm(toks, p.position);
  if (!ehNome(t)) return null;

  /* `alvo.membro` — o ponto antes diz que é membro */
  const i = toks.indexOf(t);
  if (i >= 2 && toks[i - 1].t === 'DOT' && ehNome(toks[i - 2])) {
    const alvo = toks[i - 2].v;

    /* membro de Entity: `self.x`, `c.deposita` */
    const ent = alvo === 'self' ? entidadeEm(doc, p.position)
              : (achaEntidade(doc, alvo) ? achaEntidade(doc, alvo)
                                         : achaEntidade(doc, tipoDaVariavel(doc, alvo)));
    if (ent) {
      for (const m of membrosDaEntidade(doc, ent.nome, true)) {
        if (m.nome !== t.v) continue;
        const cabeca = m.kind === 'action'
          ? `${m.privado ? 'private ' : ''}action ${ent.nome}.${assinatura(m)}`
          : `${m.privado ? 'private ' : ''}${m.tipo ? m.tipo + ' ' : ''}${ent.nome}.${m.nome}`;
        const onde = m.de && m.de !== ent.nome ? `\n\nherdado de \`${m.de}\`` : '';
        return { contents: { kind: MarkupKind.Markdown, value: '```ps\n' + cabeca + '\n```' + onde } };
      }
    }

    const imp = importsDe(doc);
    const escopo = imp.has(alvo) ? imp.get(alvo).mod : alvo;
    for (const m of membrosDoNome(doc, alvo)) {
      if (m.nome !== t.v) continue;
      const prosa = resumoDe(escopo, m.nome);
      return { contents: { kind: MarkupKind.Markdown,
        value: '```ps\n' + escopo + '.' + assinatura(m) + '\n```' + (prosa ? '\n\n' + prosa : '') } };
    }
    return null;
  }
  /* símbolo do próprio arquivo */
  for (const s of simbolosDoDoc(doc)) {
    if (s.nome !== t.v) continue;
    return { contents: { kind: MarkupKind.Markdown,
      value: '```ps\n' + (s.kind === 'action' ? assinatura(s) : `class ${s.nome}`) + '\n```' } };
  }
  /* módulo importado */
  const imp = importsDe(doc);
  if (imp.has(t.v)) {
    const e = imp.get(t.v);
    const n = membrosDoNome(doc, t.v).length;
    const de = e.arquivo ? `\n\n_de ${e.arquivo}_` : '';
    return { contents: { kind: MarkupKind.Markdown,
      value: '```ps\nimport ' + e.mod + (e.mod === t.v ? '' : ' as ' + t.v) + '\n```\n\n' + n + ' membros' + de } };
  }
  return null;
});

conexao.onDefinition((p) => {
  const doc = docs.get(p.textDocument.uri);
  if (!doc) return null;
  const toks = tokensDe(doc);
  const t = tokenEm(toks, p.position);
  if (!ehNome(t)) return null;

  /* nome de módulo importado que é ARQUIVO: abre o arquivo */
  const imp = importsDe(doc);
  if (imp.has(t.v) && imp.get(t.v).arquivo) {
    return { uri: 'file://' + imp.get(t.v).arquivo,
             range: { start: { line: 0, character: 0 }, end: { line: 0, character: 0 } } };
  }
  /* `mod.membro` de um arquivo importado: vai na linha do membro */
  const i = toks.indexOf(t);
  if (i >= 2 && toks[i - 1].t === 'DOT' && ehNome(toks[i - 2])) {
    const alvo = toks[i - 2].v;

    /* membro de Entity — inclusive o HERDADO, que mora na declaração do pai */
    const ent = alvo === 'self' ? entidadeEm(doc, p.position)
              : (achaEntidade(doc, alvo) ? achaEntidade(doc, alvo)
                                         : achaEntidade(doc, tipoDaVariavel(doc, alvo)));
    if (ent) {
      for (const m of membrosDaEntidade(doc, ent.nome, true)) {
        if (m.nome !== t.v) continue;
        const dono = achaEntidade(doc, m.de || ent.nome);
        const uri = (dono && dono.arquivo) ? 'file://' + dono.arquivo : doc.uri;
        return { uri,
                 range: { start: { line: m.linha, character: m.coluna },
                          end: { line: m.linha, character: m.coluna + m.nome.length } } };
      }
    }

    if (imp.has(alvo) && imp.get(alvo).arquivo) {
      for (const m of membrosDeArquivo(imp.get(alvo).arquivo)) {
        if (m.nome !== t.v) continue;
        return { uri: 'file://' + imp.get(alvo).arquivo,
                 range: { start: { line: m.linha, character: m.coluna },
                          end: { line: m.linha, character: m.coluna + m.nome.length } } };
      }
    }
    return null;
  }
  /* declaração no próprio arquivo */
  for (const s of simbolosDoDoc(doc)) {
    if (s.nome !== t.v) continue;
    return { uri: doc.uri,
             range: { start: { line: s.linha, character: s.coluna },
                      end: { line: s.linha, character: s.coluna + s.nome.length } } };
  }
  return null;
});

conexao.onDocumentSymbol((p) => {
  const doc = docs.get(p.textDocument.uri);
  if (!doc) return [];
  const faixa = (l, c, n) => ({ start: { line: l, character: c }, end: { line: l, character: c + n } });
  const out = [];
  const ents = entidadesDe(doc);

  /* A classe vira um nó com os campos e métodos DENTRO dela. Antes o outline
   * era uma lista plana que nem chegava a incluir as classes. */
  for (const e of ents) {
    out.push({
      name: e.nome,
      kind: SymbolKind.Class,
      detail: e.bases.length ? `(${e.bases.join(', ')})` : '',
      range: faixa(e.linha, 0, e.coluna + e.nome.length),
      selectionRange: faixa(e.linha, e.coluna, e.nome.length),
      children: e.membros.map((m) => ({
        name: m.nome,
        kind: m.kind === 'action' ? SymbolKind.Method : SymbolKind.Field,
        detail: (m.privado ? 'private ' : '') + (m.kind === 'action' ? assinatura(m) : (m.tipo || '')),
        range: faixa(m.linha, 0, m.coluna + m.nome.length),
        selectionRange: faixa(m.linha, m.coluna, m.nome.length),
      })),
    });
  }
  for (const s of simbolosDoDoc(doc)) {
    if (!s.topo || s.kind !== 'action') continue;    /* método tem prof > 0 */
    out.push({
      name: s.nome,
      kind: SymbolKind.Function,
      detail: assinatura(s),
      range: faixa(s.linha, 0, s.coluna + s.nome.length),
      selectionRange: faixa(s.linha, s.coluna, s.nome.length),
    });
  }
  return out;
});

docs.listen(conexao);
conexao.listen();
