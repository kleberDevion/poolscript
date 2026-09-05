/*
 * Servidor LSP da PoolScript, sobre `vscode-languageserver`.
 *
 * A DIVISÃO, e ela é a razão do arquivo:
 *
 *   protocolo   `vscode-languageserver`, a implementação de REFERÊNCIA. Sync
 *               incremental, capacidades, cancelamento — nada disso é nosso.
 *   estrutura   `pool --ast`. A ÁRVORE DO PARSER, o mesmo que compila o
 *               programa. Quem sabe o que é variável, de quem é o membro e o
 *               que está em escopo naquela linha é ele. Vive em `analise.js`.
 *   tabelas     `pool --metadata` — módulos, tipos e métodos, lidos das
 *               tabelas do VM.
 *   diagnóstico `pool --check`.
 *   prosa       `docs/<escopo>/<nome>/<nome>.md`, a mesma página que o
 *               `scripts/audita_doc.ps` confere contra o motor.
 *
 * NÃO HÁ UMA EXPRESSÃO REGULAR NESTE ARQUIVO NEM NO `analise.js`.
 *
 * Antes havia, e era o defeito: o servidor casava PADRÃO em cima do texto —
 * uma regex pra `self.`, outra pra `alvo.`, uma varredura de token escrita à
 * mão pra achar parâmetro, outra pra achar Entity. Cada forma nova que o
 * usuário escrevia era um ramo novo escrito à mão, e por isso sempre faltava
 * um: variável declarada três linhas acima, método dentro de classe, `a.b.c`,
 * membro de lib instalada, módulo não importado aparecendo. Toda regex aqui é
 * uma segunda gramática — que é exatamente o que estava errado.
 *
 * Nenhuma lista de módulo, método ou lib é digitada neste arquivo.
 */
'use strict';

const {
  createConnection, ProposedFeatures, TextDocuments, TextDocumentSyncKind,
  CompletionItemKind, DiagnosticSeverity, SymbolKind, MarkupKind,
} = require('vscode-languageserver/node');
const { TextDocument } = require('vscode-languageserver-textdocument');
const { execFileSync } = require('child_process');
const fs = require('fs');
const path = require('path');
const A = require('./analise.js');

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
    /* `--check` e `--ast` saem com 1 quando o programa não compila: a saída
     * ainda é o JSON que interessa — e no caso do `--ast` vem com a árvore
     * PARCIAL, que é o que permite completar enquanto se digita. */
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

/* ── a árvore do documento ───────────────────────────────────────────────
 *
 * Uma chamada ao motor por versão do documento. O índice (escopos, entidades,
 * imports) sai daí numa passada só. */
const CACHE_IDX = new Map();     // uri -> {versao, idx}

function arvoreDe(texto) {
  const bruto = motor(['--ast'], texto);
  if (!bruto.trim().startsWith('{')) return null;
  try { return (JSON.parse(bruto) || {}).arvore; } catch (_) { return null; }
}

function indiceDe(doc) {
  const c = CACHE_IDX.get(doc.uri);
  if (c && c.versao === doc.version) return c.idx;
  const idx = A.indexa(arvoreDe(doc.getText()));
  CACHE_IDX.set(doc.uri, { versao: doc.version, idx });
  return idx;
}

/* ── o índice PARA O CURSOR ──────────────────────────────────────────────
 *
 * `self.` sozinho NÃO É PROGRAMA VÁLIDO — e é exatamente o que está escrito no
 * instante em que se pede completion. O parser para ali, e a árvore parcial
 * pode nem conter o método que envolve o cursor: era por isso que `self.`
 * respondia vazio no arquivo sendo digitado e respondia certo num arquivo
 * pronto.
 *
 * A saída é a que todo servidor de linguagem usa: ANTES de pedir a árvore,
 * emenda um identificador no ponto do cursor. `self.` vira
 * `self.__ps_cursor__`, que parseia, e a estrutura ao redor volta inteira.
 *
 * Não é adivinhar texto: é uma emenda de um caractere numa posição conhecida,
 * e quem decide o que a linha significa continua sendo o parser.
 */
const MARCA_CURSOR = '__ps_cursor__';
const CACHE_CUR = new Map();     // uri -> {versao, linha, col, idx}

/* Toda linha que TERMINA em ponto ganha o nome que falta.
 *
 * Não é só a linha do cursor: um arquivo em edição costuma ter mais de um
 * ponto solto (o que se está escrevendo agora, e o que se deixou pela metade
 * três linhas acima). Enquanto QUALQUER um deles quebra o parse, a árvore
 * chega mutilada e o cursor perde a estrutura que o envolve — foi assim que
 * `c.` respondia vazio por causa de um `self.` inacabado noutro método. */
function remendaPontosSoltos(texto) {
  const linhas = texto.split('\n');
  let mexeu = false;
  for (let i = 0; i < linhas.length; i++) {
    let fim = linhas[i].length;
    while (fim > 0 && (linhas[i][fim - 1] === ' ' || linhas[i][fim - 1] === '\t'
                       || linhas[i][fim - 1] === '\r')) fim--;
    if (fim > 0 && linhas[i][fim - 1] === '.') {
      linhas[i] = linhas[i].slice(0, fim) + MARCA_CURSOR + linhas[i].slice(fim);
      mexeu = true;
    }
  }
  return mexeu ? linhas.join('\n') : texto;
}

/* Fecha o que ficou aberto: `soma(` vira `soma()`, `regex.sub("a", ` vira
 * `regex.sub("a", )`.
 *
 * Chamada sem fechar NÃO É PROGRAMA VÁLIDO, e é o que está escrito quando se
 * pede a assinatura ou o nome do próximo argumento — o parser não produz o nó
 * `Call`, e sem ele não há o que responder.
 *
 * QUEM CONTA OS PARÊNTESES É O LEXER (`pool --tokens`), não uma varredura de
 * caractere: string e comentário chegam como UM token cada, então o `(` de
 * dentro de `"a("` não existe aqui. Contar no texto cru erraria exatamente no
 * caso que o teste cobre. */
function fechaAbertos(texto) {
  const bruto = motor(['--tokens'], texto);
  if (!bruto.trim().startsWith('[')) return texto;
  let toks;
  try { toks = JSON.parse(bruto); } catch (_) { return texto; }
  const pilha = [];
  const par = { LPAREN: ')', LBRACK: ']', LBRACE: '}' };
  const fecha = { RPAREN: 'LPAREN', RBRACK: 'LBRACK', RBRACE: 'LBRACE' };
  for (const t of toks) {
    if (par[t.t]) { pilha.push(t.t); continue; }
    if (fecha[t.t]) {
      /* fecha o que estiver aberto; desemparelhado é erro do usuário, não
       * nosso — e aí não há o que remendar */
      if (pilha.length && pilha[pilha.length - 1] === fecha[t.t]) pilha.pop();
    }
  }
  if (!pilha.length) return texto;
  let cauda = '';
  for (let i = pilha.length - 1; i >= 0; i--) cauda += par[pilha[i]];
  return texto + cauda;
}

function indiceNoCursor(doc, pos) {
  const c = CACHE_CUR.get(doc.uri);
  if (c && c.versao === doc.version && c.linha === pos.line && c.col === pos.character) return c.idx;

  const texto = doc.getText();
  const off = doc.offsetAt(pos);
  /* Emenda o nome que falta DEPOIS do ponto — e só quando ele falta mesmo.
   *
   * Com `self.|obj` (cursor entre o ponto e um nome que já existe) a emenda
   * COLAVA: virava `self.__ps_cursor__obj`, e o completion oferecia um membro
   * com esse nome. O membro seguinte já fecha o `MemberAccess`, então aqui não
   * há nada a remendar. */
  const depois = off < texto.length ? texto[off] : '';
  const nomeChar = depois !== '' && (
    (depois >= 'a' && depois <= 'z') || (depois >= 'A' && depois <= 'Z') ||
    (depois >= '0' && depois <= '9') || depois === '_' || depois.charCodeAt(0) > 127);
  const comCursor = (off > 0 && texto[off - 1] === '.' && !nomeChar)
    ? texto.slice(0, off) + MARCA_CURSOR + texto.slice(off)
    : texto;
  const idx = A.indexa(arvoreDe(fechaAbertos(remendaPontosSoltos(comCursor))));
  CACHE_CUR.set(doc.uri, { versao: doc.version, linha: pos.line, col: pos.character, idx });
  return idx;
}

/* Durante UM pedido de completion vale o índice do cursor (com a emenda);
 * fora dele, o do documento como está. Uma variável em vez de passar o índice
 * por seis funções — o servidor atende um pedido por vez, e ela é limpa no
 * `finally` de quem a pôs. */
let IDX_FORCADO = null;
function idxDoc(doc) { return IDX_FORCADO || indiceDe(doc); }

/* ── tokens: só pra saber se o cursor está dentro de string/comentário ───
 *
 * `STR` e `COMMENT` chegam como UM token cada, com o conteúdo dentro — então
 * a pergunta "estou dentro de texto livre?" se responde sem olhar caractere
 * nenhum, e sem regex. */
const CACHE_TOK = new Map();

function tokensDe(doc) {
  const c = CACHE_TOK.get(doc.uri);
  if (c && c.versao === doc.version) return c.toks;
  const bruto = motor(['--tokens'], doc.getText());
  let toks = [];
  if (bruto.trim().startsWith('[')) {
    try { toks = JSON.parse(bruto); } catch (_) { toks = []; }
  }
  for (const t of toks) { t.l0 = t.l - 1; t.c0 = t.c - 1; }
  CACHE_TOK.set(doc.uri, { versao: doc.version, toks });
  return toks;
}

function dentroDeTextoLivre(doc, pos) {
  for (const t of tokensDe(doc)) {
    if (t.l0 !== pos.line) continue;
    if (t.t !== 'STR' && t.t !== 'COMMENT') continue;
    if (pos.character > t.c0 && pos.character <= t.c0 + t.n) return true;
  }
  return false;
}

/* ── arquivos importados ─────────────────────────────────────────────────
 *
 * A ordem de resolução é a da linguagem (docs/linguagem/09-imports.md §9.5):
 * stdlib, lib global instalada, arquivo do projeto. */
function pastaLibs() {
  const h = process.env.HOME;
  return h ? path.join(h, '.poolscript', 'libs') : '';
}

function libsInstaladas() {
  try {
    return fs.readdirSync(pastaLibs())
      .filter((f) => f.endsWith('.ps'))
      .map((f) => f.slice(0, -3));
  } catch (_) { return []; }
}

function arquivoDoImport(mod, dirDoc) {
  const rel = mod.split('.').join(path.sep) + '.ps';
  for (const base of [pastaLibs(), dirDoc]) {
    if (!base) continue;
    const p = path.join(base, rel);
    try { if (fs.statSync(p).isFile()) return p; } catch (_) { /* segue */ }
  }
  return '';
}

/* O índice de OUTRO arquivo .ps — pela mesma árvore, pelo mesmo caminho. */
const CACHE_ARQ = new Map();     // caminho -> {mtime, idx}

function indiceDeArquivo(caminho) {
  if (!caminho) return null;
  let mtime;
  try { mtime = fs.statSync(caminho).mtimeMs; } catch (_) { return null; }
  const c = CACHE_ARQ.get(caminho);
  if (c && c.mtime === mtime) return c.idx;
  let texto;
  try { texto = fs.readFileSync(caminho, 'utf8'); } catch (_) { return null; }
  const bruto = motor(['--ast'], texto);
  let arvore = null;
  if (bruto.trim().startsWith('{')) {
    try { arvore = (JSON.parse(bruto) || {}).arvore; } catch (_) { arvore = null; }
  }
  const idx = A.indexa(arvore);
  idx.arquivo = caminho;
  CACHE_ARQ.set(caminho, { mtime, idx });
  return idx;
}

/* O que um nome importado designa: módulo do motor, ou o índice do arquivo. */
function alvoDoImport(doc, nome) {
  const idx = idxDoc(doc);
  const imp = idx.imports.get(nome);
  if (!imp) return null;
  if (META.modulos[imp.mod] && !imp.membro) return { tipo: 'modulo', mod: imp.mod };
  const dirDoc = doc.uri.startsWith('file://') ? path.dirname(doc.uri.slice(7)) : '';
  const arq = arquivoDoImport(imp.mod, dirDoc);
  if (imp.membro) {
    /* `from mod import X` — o nome ligado é o MEMBRO, não o módulo */
    if (META.modulos[imp.mod]) return { tipo: 'membro_modulo', mod: imp.mod, membro: imp.membro };
    return { tipo: 'membro_arquivo', arquivo: arq, membro: imp.membro };
  }
  if (arq) return { tipo: 'arquivo', arquivo: arq };
  return null;
}

/* Os membros de topo de um arquivo .ps importado: Entities, actions e as
 * variáveis de módulo. É o que `mod.` oferece. */
function membrosDeArquivo(caminho) {
  const idx = indiceDeArquivo(caminho);
  if (!idx) return [];
  const out = [];
  const jaTem = new Set();
  for (const e of idx.entidades) {
    if (jaTem.has(e.nome)) continue;
    jaTem.add(e.nome);
    out.push({ nome: e.nome, kind: 'class', params: [], linha: e.linha, coluna: e.coluna });
  }
  const mod = idx.escopos.find((s) => s.tipo === 'modulo');
  for (const b of (mod ? mod.liga : [])) {
    if (jaTem.has(b.nome) || b.nome.startsWith('_')) continue;
    jaTem.add(b.nome);
    out.push({ nome: b.nome, kind: b.kind, params: b.params || [], linha: b.linha, coluna: 0 });
  }
  return out;
}

/* ── resolução de CADEIA: `a.b.c` ────────────────────────────────────────
 *
 * Devolve o que a cadeia designa, ou null. É recursivo de propósito: `a.b.c`
 * é `resolve(a).membro(b).membro(c)`, e foi a falta disso que fazia
 * "objeto de outro objeto" não sugerir nada. */
function achaEntidade(doc, nome, idxExtra) {
  if (!nome) return null;
  for (const e of idxDoc(doc).entidades) if (e.nome === nome) return e;
  if (idxExtra) for (const e of idxExtra.entidades) if (e.nome === nome) return e;
  /* Entity de arquivo importado */
  for (const [ligado] of idxDoc(doc).imports) {
    const alvo = alvoDoImport(doc, ligado);
    if (!alvo || !alvo.arquivo) continue;
    const ix = indiceDeArquivo(alvo.arquivo);
    if (!ix) continue;
    for (const e of ix.entidades) if (e.nome === nome) return e;
  }
  return null;
}

/* Membros de uma Entity COM herança. `interno` = o cursor está dentro dela,
 * então `private` conta — a mesma regra que a VM impõe em runtime. */
function membrosDaEntidade(doc, nome, interno, vistos) {
  vistos = vistos || new Set();
  if (vistos.has(nome)) return [];
  vistos.add(nome);
  const e = achaEntidade(doc, nome);
  if (!e) return [];
  const out = [];
  const jaTem = new Set();
  for (const m of e.membros) {
    if (m.nome === '__init__') continue;      /* chama-se `C(...)`, nunca `c.__init__()` */
    if (m.privado && !interno) continue;
    if (jaTem.has(m.nome)) continue;
    jaTem.add(m.nome);
    out.push(Object.assign({ de: nome }, m));
  }
  for (const b of e.bases) {
    /* herdado: o `private` do pai não é visível nem de dentro do filho */
    for (const m of membrosDaEntidade(doc, b, false, vistos)) {
      if (jaTem.has(m.nome)) continue;
      jaTem.add(m.nome);
      out.push(m);
    }
  }
  return out;
}

/* O TIPO de um nome visível na linha: Entity, módulo, ou tipo do motor. */
function tipoDoNome(doc, nome, linha) {
  const idx = idxDoc(doc);

  if (nome === 'self') {
    const e = A.entidadeEm(idx, linha);
    return e ? { tipo: 'entity', nome: e.nome, interno: true } : null;
  }
  if (achaEntidade(doc, nome)) return { tipo: 'entity', nome, interno: false };

  const alvo = alvoDoImport(doc, nome);
  if (alvo) return { tipo: 'import', alvo };

  /* variável: o tipo vem da declaração (`str x = …`) ou do que foi atribuído */
  for (const b of A.visiveisEm(idx, linha)) {
    if (b.nome !== nome) continue;
    if (b.tipo && META.tipos[b.tipo]) return { tipo: 'tipo_motor', nome: b.tipo };
    if (b.tipo && achaEntidade(doc, b.tipo)) return { tipo: 'entity', nome: b.tipo, interno: false };
    break;
  }
  /* `c = Conta(...)` / `u = mod.Usuario(...)`: o tipo é o que foi construído */
  const cons = construidoPor(idx, nome, linha);
  if (cons) {
    if (achaEntidade(doc, cons.nome)) return { tipo: 'entity', nome: cons.nome, interno: false };
    if (cons.mod) {
      const alvoM = alvoDoImport(doc, cons.mod);
      if (alvoM && alvoM.arquivo) {
        const ix = indiceDeArquivo(alvoM.arquivo);
        if (ix && ix.entidades.some((e) => e.nome === cons.nome))
          return { tipo: 'entity', nome: cons.nome, interno: false };
      }
      if (alvoM && alvoM.mod && META.modulos[alvoM.mod]) {
        for (const m of META.modulos[alvoM.mod]) {
          if (m.nome === cons.nome && m.retorna) return { tipo: 'tipo_motor', nome: m.retorna };
        }
      }
    }
  }
  return null;
}

/* O que a atribuição mais recente antes da linha CONSTRUIU: `x = Foo(...)`
 * ou `x = mod.Foo(...)`. Sai da árvore, não do texto. */
function construidoPor(idx, nome, linha) {
  let achado = null;
  const anda = (no) => {
    if (!no || typeof no !== 'object') return;
    const ehAtrib = (no.k === 'Assignment' || no.k === 'VarDecl') && no.texto === nome;
    if (ehAtrib && no.l - 1 <= linha && no.a && no.a.k === 'Call' && no.a.a) {
      const callee = no.a.a;
      if (callee.k === 'Name') achado = { nome: callee.texto, mod: null };
      else if (callee.k === 'MemberAccess' && callee.a && callee.a.k === 'Name')
        achado = { nome: callee.texto, mod: callee.a.texto };
    }
    A.cada(no, anda);
  };
  anda(idx.arvore || null);
  return achado;
}

/* Resolve a cadeia `a.b.c` e devolve a LISTA DE MEMBROS do que ela designa. */
function membrosDaCadeia(doc, partes, linha) {
  if (!partes.length) return [];
  let alvo = tipoDoNome(doc, partes[0], linha);
  if (!alvo) return [];

  for (let i = 1; i < partes.length; i++) {
    const passo = partes[i];
    const membros = membrosDe(doc, alvo, linha);
    const m = membros.find((x) => x.nome === passo);
    if (!m) return [];
    /* desce um nível: o tipo do membro é o que ele devolve ou declara */
    const t = m.retorna || m.tipo || '';
    if (t && META.tipos[t]) alvo = { tipo: 'tipo_motor', nome: t };
    else if (t && achaEntidade(doc, t)) alvo = { tipo: 'entity', nome: t, interno: false };
    else if (m.kind === 'class' && achaEntidade(doc, m.nome)) alvo = { tipo: 'entity', nome: m.nome, interno: false };
    else return [];
  }
  return membrosDe(doc, alvo, linha);
}

function membrosDe(doc, alvo, linha) {
  if (!alvo) return [];
  if (alvo.tipo === 'entity') return membrosDaEntidade(doc, alvo.nome, !!alvo.interno);
  if (alvo.tipo === 'tipo_motor') {
    return (META.tipos[alvo.nome] || []).concat(META.tipos.__universal__ || [])
      .map((m) => Object.assign({ kind: 'action', escopo: alvo.nome }, m));
  }
  if (alvo.tipo === 'import') {
    const a = alvo.alvo;
    if (a.tipo === 'modulo') {
      return (META.modulos[a.mod] || []).map((m) => Object.assign({ kind: 'action', escopo: a.mod }, m));
    }
    if (a.tipo === 'arquivo') return membrosDeArquivo(a.arquivo);
    if (a.tipo === 'membro_modulo') {
      for (const m of META.modulos[a.mod] || []) {
        if (m.nome === a.membro && m.retorna && META.tipos[m.retorna])
          return (META.tipos[m.retorna] || []).map((x) => Object.assign({ kind: 'action', escopo: m.retorna }, x));
      }
      return [];
    }
    if (a.tipo === 'membro_arquivo') {
      const e = achaEntidade(doc, a.membro);
      if (e) return membrosDaEntidade(doc, e.nome, false);
      return [];
    }
  }
  return [];
}

function assinatura(m) {
  const ps = (m.params || []).map((p) => (p.default === null || p.default === undefined)
    ? p.nome : `${p.nome}=${p.default}`);
  const ret = m.retorna ? ` -> ${m.retorna}` : '';
  if (m.kind === 'campo') return `${m.tipo ? m.tipo + ' ' : ''}${m.nome}`;
  if (m.kind === 'class') return `class ${m.nome}`;
  return `${m.nome}(${ps.join(', ')})${ret}`;
}

function itemDeMembro(m, deOnde) {
  const priv = m.privado ? ' · private' : '';
  const herd = deOnde && m.de && m.de !== deOnde ? ` · de ${m.de}` : '';
  const kind = m.kind === 'campo' ? CompletionItemKind.Field
             : m.kind === 'class' ? CompletionItemKind.Class
             : CompletionItemKind.Method;
  const it = {
    label: m.nome,
    kind,
    detail: assinatura(m) + priv + herd,
    sortText: (m.privado ? '1' : '0') + m.nome,
  };
  if (m.escopo) {
    const prosa = resumoDe(m.escopo, m.nome);
    if (prosa) it.documentation = { kind: MarkupKind.Markdown, value: prosa };
  }
  return it;
}

/* ── a chamada em que o cursor está, PELA ÁRVORE ─────────────────────────
 *
 * O nó `Call` mais interno cuja linha já passou e cujo fecha ainda não veio.
 * O parêntese dentro de string não existe aqui — não porque eu tratei, mas
 * porque o parser nunca o viu como parêntese. */
function chamadaEm(doc, pos) {
  const idx = idxDoc(doc);
  let melhor = null;
  const anda = (no) => {
    if (!no || typeof no !== 'object') return;
    if (no.k === 'Call') {
      const ini = no.l - 1;
      const fim = A.ultimaLinha(no) - 1;
      if (ini <= pos.line && pos.line <= fim) {
        if (!melhor || no.l >= melhor.l) melhor = no;
      }
    }
    A.cada(no, anda);
  };
  anda(idx.arvore || null);
  if (!melhor || !melhor.a) return null;
  const callee = melhor.a;
  const partes = [];
  let cur = callee;
  while (cur) {
    if (cur.k === 'Name') { partes.unshift(cur.texto); break; }
    if (cur.k === 'MemberAccess') { partes.unshift(cur.texto); cur = cur.a; continue; }
    return null;
  }
  const args = melhor.lista || [];
  const nomeados = new Set();
  let posicionais = 0;
  for (const a of args) {
    if (a.texto) nomeados.add(a.texto);
    else posicionais++;
  }
  return { partes, chamado: partes.join('.'), posicionais, nomeados };
}

/* O cursor está mesmo DENTRO de um `(` que ainda não fechou?
 *
 * Contado no LEXER, andando pelos tokens até a posição: parêntese dentro de
 * string ou comentário não existe ali. É a mesma razão de o fecha-grupos usar
 * tokens em vez do texto cru. */
function dentroDeParenteses(doc, pos) {
  let prof = 0;
  for (const t of tokensDe(doc)) {
    if (t.l0 > pos.line || (t.l0 === pos.line && t.c0 >= pos.character)) break;
    if (t.t === 'LPAREN') prof++;
    else if (t.t === 'RPAREN' && prof > 0) prof--;
    /* o fim do comando zera: `f(1)` numa linha e o cursor na de baixo não
     * está dentro de chamada nenhuma */
    else if (t.t === 'LBRACE' || t.t === 'RBRACE') prof = 0;
  }
  return prof > 0;
}

function paramsDoChamado(doc, ch, linha) {
  if (ch.partes.length > 1) {
    const membros = membrosDaCadeia(doc, ch.partes.slice(0, -1), linha);
    const m = membros.find((x) => x.nome === ch.partes[ch.partes.length - 1]);
    return m ? (m.params || []) : [];
  }
  const idx = idxDoc(doc);
  for (const b of A.visiveisEm(idx, linha)) {
    if (b.nome === ch.chamado && b.params) return b.params;
  }
  const e = achaEntidade(doc, ch.chamado);
  if (e) return e.membros.filter((m) => m.nome === '__init__')
                         .map((m) => m.params || []).flat();
  return [];
}

/* ── protocolo ──────────────────────────────────────────────────────────── */

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
    serverInfo: { name: 'poolscript-lsp', version: '3' },
  };
});

/* diagnóstico: quem decide é o `--check` do motor, não uma segunda gramática */
function diagnostica(doc) {
  const bruto = motor(['--check'], doc.getText());
  let r;
  try { r = JSON.parse(bruto); } catch (_) { return; }
  if (!r) return;

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
docs.onDidClose((e) => { CACHE_TOK.delete(e.document.uri); CACHE_IDX.delete(e.document.uri); });

/* A linha até o cursor. Serve pra saber o que o usuário digitou agora — e é
 * só isso que se lê do texto. */
function linhaAte(doc, pos) {
  return doc.getText({ start: { line: pos.line, character: 0 }, end: pos });
}

/* O usuário está escrevendo um `import`? Decidido pela ÁRVORE: se a linha do
 * cursor é a de um ImportStmt. Com a linha pela metade (`import ` sozinho) o
 * parser ainda produz o nó, e é por isso que a árvore parcial importa. */
function emImport(doc, pos) {
  const idx = idxDoc(doc);
  let sim = false;
  const anda = (no) => {
    if (!no || typeof no !== 'object' || sim) return;
    if (no.k === 'ImportStmt' && no.l - 1 === pos.line) { sim = true; return; }
    A.cada(no, anda);
  };
  anda(idx.arvore || null);
  if (sim) return true;
  /* linha que ainda não fecha um ImportStmt (`import ` com nada atrás): o
   * parser não gera nó, então vale a primeira palavra da linha. */
  const t = linhaAte(doc, pos).trim();
  const esp = t.indexOf(' ');
  const prim = esp < 0 ? t : t.slice(0, esp);
  return prim === 'import' || prim === 'from' || prim === 'PUSH';
}

conexao.onCompletion((p) => {
  const doc = docs.get(p.textDocument.uri);
  if (!doc) return [];
  /* dentro de string ou comentário não se completa nada */
  if (dentroDeTextoLivre(doc, p.position)) return [];
  IDX_FORCADO = indiceNoCursor(doc, p.position);
  try { return completa(doc, p); } finally { IDX_FORCADO = null; }
});

function completa(doc, p) {

  /* `import <cursor>` — AQUI é onde os módulos do motor e as libs instaladas
   * fazem sentido, e só aqui. Num ponto qualquer do arquivo, módulo que não
   * foi importado é ruído com cara de sugestão. */
  if (emImport(doc, p.position)) {
    const itens = Object.keys(META.modulos || {})
      .filter((m) => m.indexOf('.') < 0)
      .map((m) => ({
        label: m,
        kind: CompletionItemKind.Module,
        detail: `modulo do motor — ${(META.modulos[m] || []).length} membros`,
        documentation: { kind: MarkupKind.Markdown, value: resumoDe(m, m) },
      }));
    const jaTem = new Set(itens.map((i) => i.label));
    for (const nome of libsInstaladas()) {
      if (jaTem.has(nome)) continue;
      itens.push({ label: nome, kind: CompletionItemKind.Module, detail: 'lib instalada' });
    }
    return itens;
  }

  const cad = A.cadeiaAntes(linhaAte(doc, p.position), p.position.character);

  /* `alvo.` / `a.b.c.` — membros do que a cadeia designa */
  if (cad.terminaEmPonto && cad.partes.length) {
    const membros = membrosDaCadeia(doc, cad.partes, p.position.line);
    const deOnde = cad.partes[cad.partes.length - 1];
    return membros.map((m) => itemDeMembro(m, deOnde));
  }

  /* DENTRO dos parênteses de uma chamada: os parâmetros QUE AINDA CABEM.
   * Vem depois da cadeia porque `f(x.` é membro de `x`, não argumento de `f`.
   *
   * `dentroDeParenteses` é a guarda que faltava: sem ela bastava o nó `Call`
   * ABRANGER a linha do cursor pra o servidor oferecer argumento — e como o
   * remendo fecha os grupos no fim do arquivo, uma chamada podia abranger o
   * arquivo todo. O corpo de um método respondia `["flags="]` em vez do
   * escopo. */
  const ch = dentroDeParenteses(doc, p.position) ? chamadaEm(doc, p.position) : null;
  if (ch && !cad.partes.length) {
    const ps = paramsDoChamado(doc, ch, p.position.line);
    const faltam = ps.filter((x, i) => i >= ch.posicionais && !ch.nomeados.has(x.nome));
    if (faltam.length) {
      return faltam.map((x) => ({
        label: `${x.nome}=`,
        kind: CompletionItemKind.Variable,
        detail: x.default === null || x.default === undefined
          ? 'parametro' : `parametro (padrao ${x.default})`,
      }));
    }
  }

  /* sem receptor: o que está REALMENTE em escopo nesta linha */
  const idx = idxDoc(doc);
  const itens = [];
  const jaTem = new Set();
  const poe = (label, kind, detail, ordem) => {
    if (jaTem.has(label)) return;
    jaTem.add(label);
    itens.push({ label, kind, detail, sortText: ordem + label });
  };

  const KIND = {
    parametro: CompletionItemKind.Variable,
    variavel: CompletionItemKind.Variable,
    'variavel do laco': CompletionItemKind.Variable,
    action: CompletionItemKind.Function,
    class: CompletionItemKind.Class,
    model: CompletionItemKind.Struct,
    enum: CompletionItemKind.Enum,
  };

  for (const b of A.visiveisEm(idx, p.position.line)) {
    const det = b.kind === 'action' ? assinatura({ nome: b.nome, params: b.params })
              : b.tipo ? `${b.tipo} ${b.nome}` : b.kind;
    poe(b.nome, KIND[b.kind] || CompletionItemKind.Variable, det,
        (b.kind === 'parametro' || b.kind === 'variavel' || b.kind === 'variavel do laco') ? '0' : '1');
  }

  const ent = A.entidadeEm(idx, p.position.line);
  if (ent) poe('self', CompletionItemKind.Keyword, `a instância de ${ent.nome}`, '0');

  for (const e of idx.entidades) {
    poe(e.nome, CompletionItemKind.Class,
        `class ${e.nome}` + (e.bases.length ? `(${e.bases.join(', ')})` : ''), '1');
  }
  for (const [ligado, imp] of idx.imports) {
    poe(ligado, CompletionItemKind.Module,
        imp.membro ? `${imp.membro} de ${imp.mod}`
                   : (imp.mod === ligado ? 'modulo' : `modulo ${imp.mod} (as ${ligado})`), '2');
  }
  return itens;
}

conexao.onSignatureHelp((p) => {
  const doc = docs.get(p.textDocument.uri);
  if (!doc) return null;
  /* a assinatura é pedida com a chamada ABERTA — sem o remendo não há nó
   * `Call` na árvore, e não há o que responder */
  IDX_FORCADO = indiceNoCursor(doc, p.position);
  try { return assinaturaEm(doc, p); } finally { IDX_FORCADO = null; }
});

function assinaturaEm(doc, p) {
  const ch = chamadaEm(doc, p.position);
  if (!ch) return null;
  const ps = paramsDoChamado(doc, ch, p.position.line);
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
}

/* O nome sob o cursor, e a cadeia até ele. Varredura de caractere. */
function nomeSob(doc, pos) {
  const linha = doc.getText({
    start: { line: pos.line, character: 0 },
    end: { line: pos.line + 1, character: 0 },
  });
  let i = pos.character;
  let fim = i;
  while (fim < linha.length && A.cadeiaAntes(linha, fim + 1).parcial.length > 0
         && fim + 1 <= linha.length) {
    const prox = linha[fim];
    if (!prox) break;
    const eh = (prox >= 'a' && prox <= 'z') || (prox >= 'A' && prox <= 'Z')
            || (prox >= '0' && prox <= '9') || prox === '_' || prox.charCodeAt(0) > 127;
    if (!eh) break;
    fim++;
  }
  const c = A.cadeiaAntes(linha, fim);
  return { partes: c.partes, nome: c.parcial };
}

conexao.onHover((p) => {
  const doc = docs.get(p.textDocument.uri);
  if (!doc) return null;
  if (dentroDeTextoLivre(doc, p.position)) return null;
  const { partes, nome } = nomeSob(doc, p.position);
  if (!nome) return null;

  if (partes.length) {                       /* `alvo.membro` */
    const membros = membrosDaCadeia(doc, partes, p.position.line);
    const m = membros.find((x) => x.nome === nome);
    if (!m) return null;
    const dono = partes[partes.length - 1];
    const prosa = m.escopo ? resumoDe(m.escopo, m.nome) : '';
    const herd = m.de && m.de !== dono ? `\n\nherdado de \`${m.de}\`` : '';
    return { contents: { kind: MarkupKind.Markdown,
      value: '```ps\n' + (m.privado ? 'private ' : '') + dono + '.' + assinatura(m)
             + '\n```' + herd + (prosa ? '\n\n' + prosa : '') } };
  }

  const idx = indiceDe(doc);
  const e = idx.entidades.find((x) => x.nome === nome);
  if (e) {
    return { contents: { kind: MarkupKind.Markdown,
      value: '```ps\nclass ' + e.nome + (e.bases.length ? '(' + e.bases.join(', ') + ')' : '')
             + '\n```\n\n' + e.membros.length + ' membros' } };
  }
  for (const b of A.visiveisEm(idx, p.position.line)) {
    if (b.nome !== nome) continue;
    const txt = b.kind === 'action' ? assinatura({ nome: b.nome, params: b.params })
              : `${b.tipo ? b.tipo + ' ' : ''}${b.nome}`;
    return { contents: { kind: MarkupKind.Markdown,
      value: '```ps\n' + txt + '\n```\n\n' + b.kind } };
  }
  const alvo = alvoDoImport(doc, nome);
  if (alvo) {
    const n = membrosDe(doc, { tipo: 'import', alvo }, p.position.line).length;
    const de = alvo.arquivo ? `\n\n_de ${alvo.arquivo}_` : '';
    return { contents: { kind: MarkupKind.Markdown,
      value: '```ps\nimport ' + (alvo.mod || alvo.arquivo || nome) + '\n```\n\n' + n + ' membros' + de } };
  }
  return null;
});

conexao.onDefinition((p) => {
  const doc = docs.get(p.textDocument.uri);
  if (!doc) return null;
  const { partes, nome } = nomeSob(doc, p.position);
  if (!nome) return null;
  const idx = indiceDe(doc);

  if (partes.length) {
    /* membro: vai na declaração dele, inclusive no arquivo do pai */
    const alvo = tipoDoNome(doc, partes[0], p.position.line);
    if (alvo && alvo.tipo === 'entity') {
      for (const m of membrosDaEntidade(doc, alvo.nome, true)) {
        if (m.nome !== nome) continue;
        const dono = achaEntidade(doc, m.de || alvo.nome);
        const uri = dono && dono.arquivo ? 'file://' + dono.arquivo : doc.uri;
        return { uri, range: { start: { line: m.linha, character: m.coluna },
                               end: { line: m.linha, character: m.coluna + nome.length } } };
      }
    }
    if (alvo && alvo.tipo === 'import' && alvo.alvo.arquivo) {
      for (const m of membrosDeArquivo(alvo.alvo.arquivo)) {
        if (m.nome !== nome) continue;
        return { uri: 'file://' + alvo.alvo.arquivo,
                 range: { start: { line: m.linha, character: m.coluna },
                          end: { line: m.linha, character: m.coluna + nome.length } } };
      }
    }
    return null;
  }

  const e = idx.entidades.find((x) => x.nome === nome);
  if (e) return { uri: doc.uri,
                  range: { start: { line: e.linha, character: e.coluna },
                           end: { line: e.linha, character: e.coluna + nome.length } } };
  for (const b of A.visiveisEm(idx, p.position.line)) {
    if (b.nome !== nome) continue;
    return { uri: doc.uri,
             range: { start: { line: b.linha, character: 0 },
                      end: { line: b.linha, character: nome.length } } };
  }
  const alvo = alvoDoImport(doc, nome);
  if (alvo && alvo.arquivo) {
    return { uri: 'file://' + alvo.arquivo,
             range: { start: { line: 0, character: 0 }, end: { line: 0, character: 0 } } };
  }
  return null;
});

conexao.onDocumentSymbol((p) => {
  const doc = docs.get(p.textDocument.uri);
  if (!doc) return [];
  /* O outline é mostrado ENQUANTO se digita, então ele também precisa da
   * árvore remendada: com um `self.` pela metade em qualquer método, a árvore
   * crua perde a classe inteira e a barra lateral fica vazia. */
  const idx = A.indexa(arvoreDe(fechaAbertos(remendaPontosSoltos(doc.getText()))));
  const faixa = (l, c, n) => ({ start: { line: l, character: c }, end: { line: l, character: c + n } });
  const out = [];

  for (const e of idx.entidades) {
    out.push({
      name: e.nome,
      kind: SymbolKind.Class,
      detail: e.bases.length ? `(${e.bases.join(', ')})` : '',
      range: faixa(e.linha, 0, e.coluna + e.nome.length),
      selectionRange: faixa(e.linha, e.coluna, e.nome.length),
      children: e.membros.map((m) => ({
        name: m.nome,
        kind: m.kind === 'action' ? SymbolKind.Method : SymbolKind.Field,
        detail: (m.privado ? 'private ' : '') + assinatura(m),
        range: faixa(m.linha, 0, m.coluna + m.nome.length),
        selectionRange: faixa(m.linha, m.coluna, m.nome.length),
      })),
    });
  }
  const mod = idx.escopos.find((s) => s.tipo === 'modulo');
  for (const b of (mod ? mod.liga : [])) {
    if (b.kind !== 'action') continue;
    out.push({
      name: b.nome,
      kind: SymbolKind.Function,
      detail: assinatura({ nome: b.nome, params: b.params }),
      range: faixa(b.linha, 0, b.nome.length),
      selectionRange: faixa(b.linha, 0, b.nome.length),
    });
  }
  return out;
});

docs.listen(conexao);
conexao.listen();
