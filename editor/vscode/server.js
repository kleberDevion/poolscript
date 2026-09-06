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

/* A página `docs/<escopo>/<nome>/<nome>.md`: o título (a assinatura, que a
 * doc tira do código) e o primeiro parágrafo de prosa. `{titulo:'', resumo:''}`
 * quando não há página.
 *
 * `escopo` é um CAMINHO, e pode ser uma lista de candidatos — a regra da doc é
 * "o caminho diz o namespace" (`docs/jinker/request/get/get.md` documenta
 * `jinker.request.get`), e o servidor só sabe o TIPO do que está na cadeia
 * (`RequestProxy`), não a pasta. Então quem chama passa de onde o tipo veio
 * (`jinker/request`, depois `jinker`, depois o nome do tipo) e a primeira
 * página que existir responde. Antes o escopo era só o nome do tipo, e
 * `request.get`, `cors.origins` e `mapping.post` saíam sem prosa nenhuma —
 * a página existia, o servidor procurava em `docs/RequestProxy/`. */
const PAGINAS = new Map();
function paginaDe(escopo, nome) {
  if (Array.isArray(escopo)) {
    for (const e of escopo) { const p = paginaDe(e, nome); if (p.titulo || p.resumo) return p; }
    return { titulo: '', resumo: '' };
  }
  const chave = `${escopo}/${nome}`;
  if (PAGINAS.has(chave)) return PAGINAS.get(chave);
  const vazio = { titulo: '', resumo: '' };
  PAGINAS.set(chave, vazio);
  const raiz = raizDoc();
  if (!raiz) return vazio;
  const p = path.join(raiz, escopo, nome, `${nome}.md`);
  let texto;
  try { texto = fs.readFileSync(p, 'utf8'); } catch (_) { return vazio; }
  const partes = [];
  let titulo = '';
  for (const linha of texto.split('\n')) {
    const t = linha.trim();
    if (!titulo) { if (t.startsWith('# ')) titulo = t.slice(2).trim(); continue; }
    if (t === '') { if (partes.length) break; continue; }
    if (t.startsWith('#') || t.startsWith('|') || t.startsWith('```')) break;
    partes.push(t);
  }
  const pg = { titulo, resumo: partes.join(' ') };
  PAGINAS.set(chave, pg);
  return pg;
}
function resumoDe(escopo, nome) { return paginaDe(escopo, nome).resumo; }

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

function arquivoDoImport(mod, dirDoc, aspas) {
  /* `import 'caminho/alvo.ps'`: absoluto como está, senão relativo à pasta do
   * documento; como escrito e com as extensões — a mesma busca do motor. */
  if (aspas && A.especificadorEhCaminho(mod)) {
    const base = path.isAbsolute(mod) ? mod : (dirDoc ? path.join(dirDoc, mod) : '');
    if (!base) return '';
    for (const ext of ['', '.ps', '.psl', '.p']) {
      try { if (fs.statSync(base + ext).isFile()) return base + ext; } catch (_) { /* segue */ }
    }
    return '';
  }
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
  const arq = arquivoDoImport(imp.mod, dirDoc, imp.aspas);
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
    /* `string s = ...`: o apelido vira o tipo do motor pela tabela que o
     * `--metadata` publica (tipos_apelidos), sem lista aqui */
    const bt = (b.tipo && (META.tipos_apelidos || {})[b.tipo]) || b.tipo;
    if (bt && META.tipos[bt]) return { tipo: 'tipo_motor', nome: bt };
    if (b.tipo && achaEntidade(doc, b.tipo)) return { tipo: 'entity', nome: b.tipo, interno: false };
    /* `Rota.` / `Cor.` — model e enum do arquivo: os membros estão no nó */
    if (b.kind === 'model' && b.no) return { tipo: 'model', no: b.no };
    if (b.kind === 'enum' && b.no) return { tipo: 'enum', no: b.no };
    break;
  }
  /* `c = Conta(...)` / `u = mod.Usuario(...)`: o tipo é o que foi construído */
  const cons = construidoPor(idx, nome, linha);
  if (cons && cons.literal) {
    return META.tipos[cons.literal] ? { tipo: 'tipo_motor', nome: cons.literal } : { tipo: 'universal' };
  }
  if (cons) {
    if (achaEntidade(doc, cons.nome)) return { tipo: 'entity', nome: cons.nome, interno: false };
    /* `x = Rota(...)` (model do arquivo) e `v = f()` (action do arquivo: o
     * tipo declarado do retorno, `str action f()`; sem ele, os universais) */
    if (!cons.mod) {
      for (const b of A.visiveisEm(idx, linha)) {
        if (b.nome !== cons.nome) continue;
        if (b.kind === 'model' && b.no) return { tipo: 'model', no: b.no };
        if (b.kind === 'action') {
          return b.tipo && META.tipos[b.tipo] ? { tipo: 'tipo_motor', nome: b.tipo } : { tipo: 'universal' };
        }
        break;
      }
    }
    /* `mapping = Jinker(__name__)` com `from jinker import Jinker`: o nome
     * construído é um membro de módulo ligado pelo `from`. Este ramo não
     * existia — só `x = jinker.Jinker(...)` (abaixo) consultava o `retorna`
     * do motor, e a forma com `from`, que é a da doc, caía no `return null`:
     * `mapping.` sem sugestão nenhuma, `@mapping.` idem, e o VS Code caía nas
     * palavras soltas do arquivo. Fontes: o construtor vem do nó Call da
     * árvore; o vínculo Jinker → jinker, do ImportStmt; o tipo da instância,
     * de `modulos.jinker[].retorna` do `--metadata`. */
    if (!cons.mod) {
      const alvoC = alvoDoImport(doc, cons.nome);
      if (alvoC && alvoC.tipo === 'membro_modulo') {
        for (const m of META.modulos[alvoC.mod] || []) {
          if (m.nome === alvoC.membro && m.retorna && META.tipos[m.retorna])
            return { tipo: 'tipo_motor', nome: m.retorna, via: { mod: alvoC.mod, membro: alvoC.membro } };
        }
      }
      if (alvoC && alvoC.tipo === 'membro_arquivo' && achaEntidade(doc, alvoC.membro))
        return { tipo: 'entity', nome: alvoC.membro, interno: false };
    }
    if (cons.mod) {
      const alvoM = alvoDoImport(doc, cons.mod);
      if (alvoM && alvoM.arquivo) {
        const ix = indiceDeArquivo(alvoM.arquivo);
        if (ix && ix.entidades.some((e) => e.nome === cons.nome))
          return { tipo: 'entity', nome: cons.nome, interno: false };
      }
      if (alvoM && alvoM.mod && META.modulos[alvoM.mod]) {
        for (const m of META.modulos[alvoM.mod]) {
          if (m.nome === cons.nome && m.retorna)
            return { tipo: 'tipo_motor', nome: m.retorna, via: { mod: alvoM.mod, membro: cons.nome } };
        }
      }
    }
  }
  /* Nome que EXISTE (variável, parâmetro, variável de laço) mas cujo tipo
   * ninguém sabe: ao menos o que todo valor tem (`type`…). Devolver nada
   * fazia `for each x in …` + `x.` ficar mudo. */
  for (const b of A.visiveisEm(idx, linha)) if (b.nome === nome) return { tipo: 'universal' };
  return null;
}

/* O que TODO valor da linguagem tem — a tabela `__universal__` do motor. */
function universais() {
  return (META.tipos.__universal__ || []).map((m) => Object.assign({ kind: 'action', escopo: '__universal__' }, m));
}

/* O que a atribuição mais recente antes da linha CONSTRUIU: `x = Foo(...)`
 * ou `x = mod.Foo(...)`. Sai da árvore, não do texto. */
function construidoPor(idx, nome, linha) {
  let achado = null;
  const anda = (no) => {
    if (!no || typeof no !== 'object') return;
    const ehAtrib = (no.k === 'Assignment' || no.k === 'VarDecl') && no.texto === nome;
    if (ehAtrib && no.l - 1 <= linha && no.a) {
      const v = no.a;
      if (v.k === 'Call' && v.a) {
        const callee = v.a;
        if (callee.k === 'Name') achado = { nome: callee.texto, mod: null };
        else if (callee.k === 'MemberAccess' && callee.a && callee.a.k === 'Name')
          achado = { nome: callee.texto, mod: callee.a.texto };
      }
      /* LITERAL: `nome = "ana"` é str, `xs = [1]` é list, `d = {}` é dict —
       * o nó da árvore diz qual. (Número e bool saem como `Literal` sem
       * texto e não têm tabela de métodos; ficam sem tipo.) */
      else if (v.k === 'Literal' && typeof v.texto === 'string') achado = { literal: 'str' };
      else if (v.k === 'ListLiteral') achado = { literal: 'list' };
      else if (v.k === 'DictLiteral') achado = { literal: 'dict' };
      else if (v.k === 'TupleLiteral') achado = { literal: 'tup' };
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
    /* desce um nível: o tipo do membro é o que ele devolve ou declara. A
     * procedência (`via`: de que módulo/membro o tipo saiu) desce junto —
     * é ela que diz em que pasta da doc está a prosa do próximo membro. */
    const t = m.retorna || m.tipo || '';
    const modBase = alvo.via ? alvo.via.mod : (alvo.tipo === 'import' && alvo.alvo ? alvo.alvo.mod : null);
    if (t && META.tipos[t]) alvo = { tipo: 'tipo_motor', nome: t, via: modBase ? { mod: modBase, membro: passo } : undefined };
    else if (t && achaEntidade(doc, t)) alvo = { tipo: 'entity', nome: t, interno: false };
    else if (m.kind === 'class' && achaEntidade(doc, m.nome)) alvo = { tipo: 'entity', nome: m.nome, interno: false };
    /* membro existe, retorno desconhecido (`request.get(...).`): universais */
    else alvo = { tipo: 'universal' };
  }
  return membrosDe(doc, alvo, linha);
}

function membrosDe(doc, alvo, linha) {
  if (!alvo) return [];
  if (alvo.tipo === 'universal') return universais();
  if (alvo.tipo === 'model') {
    return (alvo.no.lista || []).filter((f) => f && f.k === 'ModelField')
      .map((f) => ({ nome: f.texto, kind: 'campo', tipo: f.texto2 || '', linha: f.l - 1, coluna: f.c - 1 }));
  }
  if (alvo.tipo === 'enum') {
    return (alvo.no.lista || []).filter((m) => m && m.k === 'EnumMember')
      .map((m) => ({ nome: m.texto, kind: 'campo', tipo: '', linha: m.l - 1, coluna: m.c - 1 }));
  }
  if (alvo.tipo === 'entity') return membrosDaEntidade(doc, alvo.nome, !!alvo.interno);
  if (alvo.tipo === 'tipo_motor') {
    /* onde está a prosa: pela procedência (`jinker/Jinker`, `jinker`) antes
     * do nome do tipo — ver `paginaDe` */
    const escopo = alvo.via
      ? [`${alvo.via.mod}/${alvo.via.membro}`, alvo.via.mod, alvo.nome]
      : alvo.nome;
    return (META.tipos[alvo.nome] || []).concat(META.tipos.__universal__ || [])
      .map((m) => Object.assign({ kind: 'action', escopo }, m));
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
          return (META.tipos[m.retorna] || [])
            .map((x) => Object.assign({ kind: 'action', escopo: [`${a.mod}/${a.membro}`, a.mod, m.retorna] }, x));
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
      completionProvider: { triggerCharacters: ['.', '(', '/', "'", '"'], resolveProvider: false },
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

/* A pasta do documento no disco ("" se a URI não é de arquivo). */
function pastaDoDoc(doc) {
  return doc.uri.startsWith('file://') ? path.dirname(doc.uri.slice(7)) : '';
}

/* Os membros que `from X import …` pode trazer: os do módulo do motor, ou os
 * de topo do arquivo `.ps` (lib instalada ou arquivo ao lado). */
function membrosParaImport(doc, mod, aspas) {
  if (META.modulos[mod]) {
    return META.modulos[mod].map((m) => Object.assign(
      { kind: m.kind === 'value' ? 'campo' : 'action', escopo: mod, tipo: m.retorna || '' }, m));
  }
  const arq = arquivoDoImport(mod, pastaDoDoc(doc), aspas);
  return arq ? membrosDeArquivo(arq) : [];
}

/* A aspa que abriu o especificador de `import`/`from`/`PUSH` e ainda não
 * fechou antes do cursor — o índice dela na linha, ou -1. Sem regex: a
 * primeira palavra decide, a primeira aspa depois dela abre, a mesma aspa
 * depois dela fecha. */
function aspaAbertaDoImport(linha) {
  const t = linha.trimStart();
  const kw = ['import ', 'from ', 'PUSH '].find((k) => t.startsWith(k));
  if (!kw) return -1;
  const ini = linha.length - t.length + kw.length;
  const i1 = linha.indexOf("'", ini);
  const i2 = linha.indexOf('"', ini);
  const i = i1 < 0 ? i2 : (i2 < 0 ? i1 : Math.min(i1, i2));
  if (i < 0) return -1;
  return linha.indexOf(linha[i], i + 1) < 0 ? i : -1;
}

/* Dentro das aspas de um import: arquivos `.ps` e pastas a partir da pasta do
 * documento (ou da subpasta já digitada), e — enquanto não há `/` — os
 * módulos do motor e as libs instaladas, que também vêm entre aspas. O rótulo
 * é só o último segmento: é o que o editor substitui. */
function completaCaminhoImport(doc, parcial) {
  const corte = parcial.lastIndexOf('/');
  const sub = corte < 0 ? '' : parcial.slice(0, corte + 1);
  const itens = [];
  const jaTem = new Set();
  const poe = (label, kind, detail, insertText) => {
    if (jaTem.has(label)) return;
    jaTem.add(label);
    const it = { label, kind, detail };
    if (insertText) it.insertText = insertText;
    itens.push(it);
  };
  if (!sub) {
    for (const m of Object.keys(META.modulos || {})) {
      if (m.indexOf('.') < 0) poe(m, CompletionItemKind.Module, `modulo do motor — ${(META.modulos[m] || []).length} membros`);
    }
    for (const nome of libsInstaladas()) poe(nome, CompletionItemKind.Module, 'lib instalada');
  }
  const dir = pastaDoDoc(doc);
  const base = path.isAbsolute(sub) ? sub : (dir ? path.join(dir, sub) : '');
  if (base) {
    const meu = doc.uri.startsWith('file://') ? path.basename(doc.uri.slice(7)) : '';
    let ents = [];
    try { ents = fs.readdirSync(base, { withFileTypes: true }); } catch (_) { ents = []; }
    for (const e of ents) {
      if (e.name.startsWith('.') || (!sub && e.name === meu)) continue;
      if (e.isDirectory()) poe(e.name, CompletionItemKind.Folder, 'pasta', e.name + '/');
      else if (e.name.endsWith('.ps')) poe(e.name, CompletionItemKind.File, 'arquivo .ps');
    }
    poe('..', CompletionItemKind.Folder, 'pasta acima', '../');
  }
  return itens;
}

/* Dentro de um `import`/`from`. Dois casos, decididos pelos TOKENS da linha
 * (o lexer), nunca pelo texto:
 *
 *  - `from X import <cursor>`: os MEMBROS de X — módulo do motor, lib
 *    instalada ou arquivo `.ps` — menos os já listados antes do cursor. Era o
 *    defeito da tela dele: `from mail import Mia` devolvia a lista de
 *    MÓDULOS, com `mail` e `multipart` dentro, porque este ramo não
 *    distinguia os dois lados do `import`.
 *  - `import <cursor>` / `from <cursor>` / `import pasta.<cursor>`: módulos do
 *    motor, libs instaladas e os ARQUIVOS e PASTAS ao lado do documento (ou
 *    dentro da pasta já digitada). Arquivo e pasta não apareciam. */
function completaImport(doc, p) {
  const linha = linhaAte(doc, p.position);
  /* `import '<cursor>` / `from '<cursor>`: dentro das aspas — o caminho
   * parcial é o texto entre a aspa e o cursor */
  const aspa = aspaAbertaDoImport(linha);
  if (aspa >= 0) return completaCaminhoImport(doc, linha.slice(aspa + 1));

  const parcial = A.cadeiaAntes(linha, p.position.character).parcial;
  const iniParcial = p.position.character - parcial.length;
  const antes = tokensDe(doc).filter((t) => t.l0 === p.position.line && t.n > 0 && t.c0 + t.n <= iniParcial);
  const ehFrom = antes.length > 0 && antes[0].t === 'KW' && antes[0].v === 'from';
  const iImp = antes.findIndex((t, i) => i > 0 && t.t === 'KW' && t.v === 'import');
  const ehNome = (t) => t.t === 'DOT' || t.t.startsWith('IDENT');

  if (ehFrom && iImp > 0) {
    /* `from 'x/y.ps' import <cursor>`: o módulo é a STRING */
    const aspas = antes.length > 1 && antes[1].t === 'STR';
    const mod = aspas ? antes[1].v : antes.slice(1, iImp).filter(ehNome).map((t) => t.v).join('');
    const jaTem = new Set(antes.slice(iImp + 1).filter((t) => t.t.startsWith('IDENT')).map((t) => t.v));
    return membrosParaImport(doc, mod, aspas).filter((m) => !jaTem.has(m.nome)).map((m) => itemDeMembro(m, mod));
  }

  /* o `pasta.` já digitado antes do cursor, se houver */
  let k = antes.length;
  while (k > 0 && ehNome(antes[k - 1])) k--;
  const prefixo = antes.slice(k).map((t) => t.v).join('');
  const sub = prefixo.split('.').filter((s) => s !== '');

  const itens = [];
  const jaTem = new Set();
  const poe = (label, detail, doc_) => {
    if (jaTem.has(label)) return;
    jaTem.add(label);
    const it = { label, kind: CompletionItemKind.Module, detail };
    if (doc_) it.documentation = { kind: MarkupKind.Markdown, value: doc_ };
    itens.push(it);
  };
  if (!sub.length) {
    for (const m of Object.keys(META.modulos || {})) {
      if (m.indexOf('.') >= 0) continue;
      poe(m, `modulo do motor — ${(META.modulos[m] || []).length} membros`, resumoDe(m, m));
    }
    for (const nome of libsInstaladas()) poe(nome, 'lib instalada');
  }
  /* arquivos e pastas: ao lado do documento, ou dentro de `sub` */
  const dir = pastaDoDoc(doc);
  if (dir) {
    const base = path.join(dir, ...sub);
    const meu = doc.uri.startsWith('file://') ? path.basename(doc.uri.slice(7)) : '';
    let ents = [];
    try { ents = fs.readdirSync(base, { withFileTypes: true }); } catch (_) { ents = []; }
    for (const e of ents) {
      if (e.name.startsWith('.') || e.name === meu) continue;
      if (e.isDirectory()) poe(e.name, 'pasta');
      else if (e.name.endsWith('.ps')) poe(e.name.slice(0, -3), 'arquivo .ps');
    }
  }
  return itens;
}

/* `"a,b".` — o token antes do ponto é uma STRING: os membros são os de `str`.
 * Só literal de texto: `]` e `}` podem ser índice ou fim de literal, e o tipo
 * de `x[0]` ninguém sabe aqui. */
function receptorLiteral(doc, pos) {
  const toks = tokensDe(doc).filter((t) => t.l0 === pos.line && t.n > 0 && t.c0 + t.n <= pos.character);
  const n = toks.length;
  if (n < 2 || toks[n - 1].t !== 'DOT') return null;
  if (toks[n - 2].t === 'STR' && META.tipos.str) return 'str';
  return null;
}

conexao.onCompletion((p) => {
  const doc = docs.get(p.textDocument.uri);
  if (!doc) return [];
  /* dentro de string ou comentário não se completa nada — menos a string de
   * um `import '…'`, que é onde o caminho se escreve */
  if (dentroDeTextoLivre(doc, p.position) && !emImport(doc, p.position)) return [];
  IDX_FORCADO = indiceNoCursor(doc, p.position);
  try { return completa(doc, p); } finally { IDX_FORCADO = null; }
});

function completa(doc, p) {

  /* `import <cursor>` — AQUI é onde os módulos do motor e as libs instaladas
   * fazem sentido, e só aqui. Num ponto qualquer do arquivo, módulo que não
   * foi importado é ruído com cara de sugestão. */
  if (emImport(doc, p.position)) return completaImport(doc, p);

  const cad = A.cadeiaAntes(linhaAte(doc, p.position), p.position.character);

  /* `"a,b".` — o receptor é um LITERAL de texto: o token antes do ponto é STR */
  const lit = receptorLiteral(doc, p.position);
  if (lit) return membrosDe(doc, { tipo: 'tipo_motor', nome: lit }, p.position.line).map((m) => itemDeMembro(m, lit));

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
  /* builtins e palavras-chave: as tabelas do motor (`--metadata` publica a
   * `BUILTINS[]` da VM e a `KEYWORDS[]` do lexer). Não apareciam — `post`,
   * `len`, `action`, `if` nunca eram sugeridos sem receptor. */
  for (const b of META.builtins || []) {
    poe(b.nome, CompletionItemKind.Function, 'builtin · ' + assinatura({ nome: b.nome, params: b.params }), '3');
  }
  for (const k of META.keywords || []) poe(k, CompletionItemKind.Keyword, 'palavra-chave', '4');
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

/* ── hover ────────────────────────────────────────────────────────────────
 *
 * Cada resposta sai de uma fonte, e a fonte é dita aqui:
 *   palavra-chave      -> `pool --tokens` diz que é KW; a prosa é a SEÇÃO de
 *                         docs/linguagem/NN-*.md cujo título traz a palavra em
 *                         crase (`## 5.1. Condicional — \`if\` / \`elif\`…`),
 *                         ou a página do builtin (docs/builtins/post/post.md)
 *   variável           -> tipo do que foi declarado/construído (árvore +
 *                         `--metadata`) e a linha da declaração (árvore)
 *   parâmetro          -> a action dona, do índice de escopos (árvore)
 *   action do arquivo  -> `int async action f(...)` do nó ActionDecl, e o
 *                         decorador em cima (DecoratorStmt), da árvore
 *   model do arquivo   -> os campos do ModelDecl, da árvore
 *   nome vindo de from -> assinatura do membro no `--metadata` e a página
 *                         docs/<mod>/<membro>/<membro>.md
 *   `alvo.membro`      -> assinatura da tabela do VM e a página achada pelo
 *                         CAMINHO (`jinker/request/get`), ver `paginaDe`
 *
 * O que havia: keyword caía no `return null` (o hover nunca perguntava ao
 * lexer que token era); variável saía "x / variavel"; `jsonify` saía como
 * "import jinker / 6 membros" — o 6 era a contagem de membros do tipo de
 * RETORNO; `request.get` saía sem prosa por procurar em docs/RequestProxy/. */
function md(valor) { return { contents: { kind: MarkupKind.Markdown, value: valor } }; }

/* O token do lexer sob o cursor: `[c0, c0+n)`, o PRIMEIRO caractere incluso.
 * (`dentroDeTextoLivre` usa `>` de propósito: o cursor logo antes de uma
 * string não está dentro dela. Aqui o cursor no `i` de `if` está no `if`.) */
function tokenSob(doc, pos) {
  for (const t of tokensDe(doc)) {
    if (t.l0 === pos.line && pos.character >= t.c0 && pos.character < t.c0 + t.n) return t;
  }
  return null;
}

/* As seções de um arquivo de docs/linguagem: título, os spans em crase do
 * título (é por eles que a keyword se acha) e as linhas do corpo. Título
 * dentro de cerca de código não conta. Cache por mtime. */
const SECOES = new Map();
function secoesDe(arq) {
  let st;
  try { st = fs.statSync(arq); } catch (_) { return []; }
  const c = SECOES.get(arq);
  if (c && c.mtime === st.mtimeMs) return c.secoes;
  let texto;
  try { texto = fs.readFileSync(arq, 'utf8'); } catch (_) { return []; }
  const secoes = [];
  let cerca = false;
  let atual = null;
  for (const linha of texto.split('\n')) {
    const t = linha.trim();
    if (t.startsWith('```')) { cerca = !cerca; if (atual) atual.corpo.push(linha); continue; }
    if (!cerca && t.startsWith('#')) {
      let j = 0;
      while (t[j] === '#') j++;
      const titulo = t.slice(j).trim();
      const pedacos = titulo.split('`');
      atual = { titulo, spans: pedacos.filter((_, k) => k % 2 === 1).map((s) => s.trim()), corpo: [] };
      secoes.push(atual);
      continue;
    }
    if (atual) atual.corpo.push(linha);
  }
  SECOES.set(arq, { mtime: st.mtimeMs, secoes });
  return secoes;
}

/* A primeira seção, na ordem dos arquivos, cujo título traz `candidato` em
 * crase. Sem lista digitada: a doc da linguagem é a fonte. */
function secaoDaLinguagem(candidato) {
  const raiz = raizDoc();
  if (!raiz) return null;
  const dir = path.join(raiz, 'linguagem');
  let arqs;
  try { arqs = fs.readdirSync(dir).filter((f) => f.endsWith('.md')).sort(); } catch (_) { return null; }
  for (const f of arqs) {
    for (const s of secoesDe(path.join(dir, f))) if (s.spans.includes(candidato)) return s;
  }
  return null;
}

/* O que se mostra de uma seção: o título, o primeiro bloco ```ps quando ele
 * vem ANTES da prosa (5.1, 6.1 e 10.2 abrem com o exemplo), e o primeiro
 * parágrafo — bullets ficam um por linha, senão viravam um parágrafo só. */
function trechoDaSecao(s) {
  const prosa = [];
  let bloco = null;
  let blocoPronto = '';
  for (const linha of s.corpo) {
    const t = linha.trim();
    if (bloco !== null) {
      if (t.startsWith('```')) { blocoPronto = '```ps\n' + bloco.join('\n') + '\n```'; bloco = null; if (prosa.length) break; }
      else bloco.push(linha);
      continue;
    }
    if (t.startsWith('```')) { if (prosa.length || blocoPronto) break; bloco = []; continue; }
    if (t === '') { if (prosa.length) break; continue; }
    if (t.startsWith('|') || t.startsWith('#')) break;
    prosa.push(t);
  }
  let texto = '';
  for (const p of prosa) texto += (texto === '' ? '' : (p.startsWith('-') || p.startsWith('*') ? '\n' : ' ')) + p;
  return s.titulo + (blocoPronto ? '\n\n' + blocoPronto : '') + (texto ? '\n\n' + texto : '');
}

/* Keyword sob o cursor. Candidatos: a palavra unida à vizinha se ela também é
 * KW na mesma linha (`for each`, `count each`), depois a palavra só. */
function hoverDeKeyword(doc, tok) {
  const toks = tokensDe(doc);
  const i = toks.indexOf(tok);
  const ant = i > 0 ? toks[i - 1] : null;
  const seg = i >= 0 ? toks[i + 1] : null;
  const cands = [];
  if (ant && ant.t === 'KW' && ant.l0 === tok.l0) cands.push(ant.v + ' ' + tok.v);
  if (seg && seg.t === 'KW' && seg.l0 === tok.l0) cands.push(tok.v + ' ' + seg.v);
  cands.push(tok.v);
  for (const c of cands) {
    const s = secaoDaLinguagem(c);
    if (s) return md('```ps\n' + c + '\n```\n\n' + trechoDaSecao(s));
  }
  return null;
}

/* `@app.post(...)` em cima da action `nome` declarada na linha `linha`, ou "".
 * O DecoratorStmt embrulha a action num Block (`.b.lista[0]`). */
function decoradorDe(idx, nome, linha) {
  let achado = '';
  const anda = (no) => {
    if (!no || typeof no !== 'object' || achado) return;
    if (no.k === 'DecoratorStmt' && no.a && no.b) {
      const dentro = (no.b.lista || []).some((x) => x && x.k === 'ActionDecl' && x.texto === nome && x.l - 1 === linha);
      if (dentro) { achado = '@' + (no.a.lista || []).map((x) => x && x.texto).filter(Boolean).join('.'); return; }
    }
    A.cada(no, anda);
  };
  anda(idx.arvore || null);
  return achado;
}

conexao.onHover((p) => {
  const doc = docs.get(p.textDocument.uri);
  if (!doc) return null;
  if (dentroDeTextoLivre(doc, p.position)) return null;

  /* palavra-chave — o lexer é quem diz; `json`/`str` são também módulo/tipo
   * do motor, e aí a resposta é a do motor. Depois de um `.` o token com
   * nome de keyword é MEMBRO (`app.post`, `d.post`): o lexer marca `post`
   * como KW em qualquer posição, e sem esta cláusula o hover em
   * `@mapping.post` mostrava o builtin de imprimir. */
  const tok = tokenSob(doc, p.position);
  const toksAqui = tokensDe(doc);
  const antes = tok ? toksAqui[toksAqui.indexOf(tok) - 1] : null;
  const aposPonto = !!(antes && antes.t === 'DOT' && antes.l0 === tok.l0);
  if (tok && tok.t === 'KW' && !aposPonto && !META.modulos[tok.v]) {
    if (META.tipos[tok.v]) {
      return md('```ps\n' + tok.v + '\n```\n\ntipo · ' + (META.tipos[tok.v] || []).length + ' métodos');
    }
    const pg = paginaDe('builtins', tok.v);
    if (pg.titulo) return md('```ps\n' + pg.titulo.split('`').join('') + '\n```' + (pg.resumo ? '\n\n' + pg.resumo : ''));
    return hoverDeKeyword(doc, tok);
  }

  const { partes, nome } = nomeSob(doc, p.position);
  if (!nome) return null;

  if (partes.length) {                       /* `alvo.membro` */
    const membros = membrosDaCadeia(doc, partes, p.position.line);
    const m = membros.find((x) => x.nome === nome);
    if (!m) return null;
    const dono = partes[partes.length - 1];
    const prosa = m.escopo ? resumoDe(m.escopo, m.nome) : '';
    const herd = m.de && m.de !== dono ? `\n\nherdado de \`${m.de}\`` : '';
    return md('```ps\n' + (m.privado ? 'private ' : '') + dono + '.' + assinatura(m)
              + '\n```' + herd + (prosa ? '\n\n' + prosa : ''));
  }

  const idx = indiceDe(doc);
  const e = idx.entidades.find((x) => x.nome === nome);
  if (e) {
    return md('```ps\nclass ' + e.nome + (e.bases.length ? '(' + e.bases.join(', ') + ')' : '')
              + '\n```\n\n' + e.membros.length + ' membros');
  }
  for (const b of A.visiveisEm(idx, p.position.line)) {
    if (b.nome !== nome) continue;
    const no = b.no || {};
    const onde = 'linha ' + (b.linha + 1);
    if (b.kind === 'action') {
      /* a ordem dos modificadores é livre na linguagem; aqui sai a canônica
       * da doc (6.4): tipo, async, action */
      const cab = (b.tipo ? b.tipo + ' ' : '') + (b.async ? 'async ' : '') + 'action '
                + assinatura({ nome: b.nome, params: b.params });
      const dec = decoradorDe(idx, b.nome, b.linha);
      return md('```ps\n' + (dec ? dec + '\n' : '') + cab + '\n```\n\naction · declarada na ' + onde);
    }
    if (b.kind === 'parametro') {
      const esc = idx.escopos.find((s) => s.liga.includes(b));
      const dono = esc ? (esc.tipo === 'metodo' ? esc.entidade + '.' + esc.nome : esc.nome) : '';
      return md('```ps\n' + b.nome + '\n```\n\nparâmetro' + (dono ? ' de `' + dono + '`' : ''));
    }
    if (b.kind === 'model') {
      const campos = (no.lista || []).filter((f) => f && f.texto)
        .map((f) => f.texto + ': ' + (f.texto2 || '') + (f.i2 >= 0 ? '(length=' + f.i2 + ')' : ''));
      return md('```ps\nmodel ' + b.nome + '() { ' + campos.join(', ') + ' }\n```\n\nmodel · declarado na ' + onde);
    }
    /* variável: o tipo é o declarado (`str x`) ou o construído (`x = Jinker(...)`) */
    const t = tipoDoNome(doc, nome, p.position.line);
    const tipo = b.tipo || (t && t.tipo !== 'import' && t.nome) || '';
    return md('```ps\n' + (tipo ? tipo + ' ' : '') + b.nome + '\n```\n\n' + b.kind + ' · declarada na ' + onde);
  }
  const alvo = alvoDoImport(doc, nome);
  if (alvo) {
    if (alvo.tipo === 'membro_modulo') {
      const m = (META.modulos[alvo.mod] || []).find((x) => x.nome === alvo.membro);
      if (m) {
        const cab = alvo.mod + '.' + (m.kind === 'value'
          ? m.nome + (m.retorna ? ' -> ' + m.retorna : '')
          : assinatura(m));
        const prosa = resumoDe(alvo.mod, alvo.membro);
        return md('```ps\n' + cab + '\n```' + (prosa ? '\n\n' + prosa : ''));
      }
    }
    const n = membrosDe(doc, { tipo: 'import', alvo }, p.position.line).length;
    const de = alvo.arquivo ? `\n\n_de ${alvo.arquivo}_` : '';
    return md('```ps\nimport ' + (alvo.mod || alvo.arquivo || nome) + '\n```\n\n' + n + ' membros' + de);
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
