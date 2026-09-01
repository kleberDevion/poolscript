/*
 * Servidor LSP da PoolScript, sobre `vscode-languageserver`.
 *
 * POR QUE ESTE ARQUIVO SUBSTITUIU O `lsp/servidor.ps`:
 *
 * O servidor anterior era escrito em PoolScript e implementava o protocolo à
 * mão. Ele anunciava QUATRO capacidades (sync, completion, hover, semantic
 * tokens) e respondia `-32601` pra todo o resto — sem "ir pra definição", sem
 * outline, sem símbolos, sem signature help, sem rename. E o pouco que fazia,
 * fazia adivinhando com busca de string no texto cru, o que produzia:
 *
 *     import json as js   ->  `js.` dava ZERO sugestão (o `as` era ignorado)
 *     import random       ->  ZERO (lib instalada em ~/.poolscript/libs não
 *                             era catalogada; havia quatro instaladas aqui)
 *     regex.sub("(", )    ->  o parêntese DENTRO DA STRING quebrava o detector
 *     f(  com action f    ->  função LOCAL não oferecia parâmetro nenhum
 *     regex.sub("a","b",  ->  reoferecia os cinco parâmetros, inclusive os
 *                             dois já passados
 *
 * Nada disso é bug de detalhe: é o que acontece quando se reimplementa
 * protocolo e análise léxica em vez de usar o que existe.
 *
 * A DIVISÃO AGORA É ESTA, e ela é a razão do arquivo:
 *
 *   protocolo   `vscode-languageserver`, a implementação de REFERÊNCIA. Sync
 *               incremental, capacidades, cancelamento — nada disso é nosso.
 *   análise     O MOTOR. `pool --tokens` dá o lexer de verdade (string e
 *               comentário são TOKENS, então parêntese dentro deles nunca se
 *               confunde com chamada), `pool --metadata` dá módulos/tipos/
 *               métodos lidos das tabelas do VM, `pool --check` dá o
 *               diagnóstico. Aqui não há uma segunda gramática.
 *   prosa       `docs/<escopo>/<nome>/<nome>.md`, a mesma página que o
 *               `scripts/audita_doc.ps` confere contra o motor.
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
  if (t.t === 'IDENT') return true;
  return t.t === 'KW' && (!!META.modulos[t.v] || !!META.tipos[t.v]);
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
const CACHE_ARQ = new Map();     // caminho -> {mtime, membros}

function membrosDeArquivo(caminho) {
  if (!caminho) return [];
  let mtime;
  try { mtime = fs.statSync(caminho).mtimeMs; } catch (_) { return []; }
  const c = CACHE_ARQ.get(caminho);
  if (c && c.mtime === mtime) return c.membros;

  let texto;
  try { texto = fs.readFileSync(caminho, 'utf8'); } catch (_) { return []; }
  const bruto = motor(['--tokens'], texto);
  let toks = [];
  if (bruto.trim().startsWith('[')) { try { toks = JSON.parse(bruto); } catch (_) { toks = []; } }

  const membros = [];
  const vistos = new Set();
  let prof = 0;
  for (let i = 0; i < toks.length; i++) {
    const t = toks[i];
    if (t.t === 'LBRACE' || t.t === 'INDENT') prof++;
    else if (t.t === 'RBRACE' || t.t === 'DEDENT') prof = Math.max(0, prof - 1);
    if (prof !== 0) continue;                    /* só o nível de topo */
    const kw = t.t === 'KW' ? t.v : '';
    const decl = (kw === 'action' || kw === 'reaction') ? 'action'
               : (kw === 'class' || kw === 'Class' || kw === 'Entity') ? 'class' : '';
    if (!decl) continue;
    const ident = toks[i + 1];
    if (!ident || ident.t !== 'IDENT') continue;
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
  CACHE_ARQ.set(caminho, { mtime, membros });
  return membros;
}

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
    if (!ident || ident.t !== 'IDENT') continue;
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
    if (toks[i].t !== 'IDENT' || toks[i].v !== nome) continue;
    if (!(toks[i + 1].t === 'OP' && toks[i + 1].v === '=')) continue;
    const d = toks[i + 2];
    if (d.t === 'STR') { tipo = 'str'; continue; }
    if (d.t === 'INT') { tipo = 'int'; continue; }
    if (d.t === 'FLOAT') { tipo = 'flo'; continue; }
    if (d.t === 'LBRACK') { tipo = 'list'; continue; }
    if (d.t === 'LBRACE') { tipo = 'dict'; continue; }
    /* `x = mod.membro(...)` — o tipo é o retorno declarado */
    if (d.t === 'IDENT' && toks[i + 3] && toks[i + 3].t === 'DOT' && toks[i + 4]) {
      for (const m of membrosDoNome(doc, d.v)) {
        if (m.nome === toks[i + 4].v && m.retorna) { tipo = m.retorna; break; }
      }
    }
  }
  return tipo;
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
  if (!r || r.ok) { conexao.sendDiagnostics({ uri: doc.uri, diagnostics: [] }); return; }
  const linha = Math.max(0, (r.linha || 1) - 1);
  const col = Math.max(0, (r.coluna || 1) - 1);
  conexao.sendDiagnostics({
    uri: doc.uri,
    diagnostics: [{
      severity: DiagnosticSeverity.Error,
      range: { start: { line: linha, character: col }, end: { line: linha, character: col + 1 } },
      message: `${r.tipo}: ${r.msg}`,
      source: 'poolscript',
    }],
  });
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

  /* `alvo.` — membros do que o alvo designa */
  const mDot = /([A-Za-z_][A-Za-z0-9_.]*)\.\s*$/.exec(linha);
  if (mDot) {
    const alvo = mDot[1];
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

  /* sem receptor: módulos importados, símbolos do arquivo, módulos do motor */
  const itens = [];
  for (const [ligado, e] of importsDe(doc)) {
    itens.push({
      label: ligado,
      kind: CompletionItemKind.Module,
      detail: e.mod === ligado ? 'modulo' : `modulo ${e.mod} (as ${ligado})`,
    });
  }
  for (const s of simbolosDoDoc(doc)) {
    if (!s.topo) continue;
    itens.push({
      label: s.nome,
      kind: s.kind === 'class' ? CompletionItemKind.Class : CompletionItemKind.Function,
      detail: s.kind === 'action' ? assinatura(s) : `class ${s.nome}`,
    });
  }
  const jaTem = new Set(itens.map((i) => i.label));
  for (const mo of Object.keys(META.modulos || {})) {
    if (jaTem.has(mo)) continue;
    itens.push({
      label: mo,
      kind: CompletionItemKind.Module,
      detail: `modulo — ${(META.modulos[mo] || []).length} membros (precisa de import)`,
    });
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
  return simbolosDoDoc(doc).map((s) => ({
    name: s.nome,
    kind: s.kind === 'class' ? SymbolKind.Class : SymbolKind.Function,
    range: { start: { line: s.linha, character: 0 },
             end: { line: s.linha, character: s.coluna + s.nome.length } },
    selectionRange: { start: { line: s.linha, character: s.coluna },
                      end: { line: s.linha, character: s.coluna + s.nome.length } },
    detail: s.kind === 'action' ? assinatura(s) : '',
  }));
});

docs.listen(conexao);
conexao.listen();
