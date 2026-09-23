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
 *               `scripts/audita_doc.pr` confere contra o motor.
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
  CompletionItemKind, DiagnosticSeverity, SymbolKind, MarkupKind, MessageType,
} = require('vscode-languageserver/node');
const { TextDocument } = require('vscode-languageserver-textdocument');
const { execFileSync } = require('child_process');
const fs = require('fs');
const path = require('path');
const url = require('url');
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
/* O último motivo de o motor não ter rodado (binário ausente, stderr). Sem
 * isto o servidor subia, respondia tudo e devolvia listas VAZIAS sem um
 * aviso — "o LSP não sugere nada" no IntelliJ, sem deixar rastro. */
let ULTIMO_ERRO_MOTOR = '';
let META_ERRO = '';

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
    ULTIMO_ERRO_MOTOR = (e && e.code === 'ENOENT') ? 'nao achei o binario'
                      : ((e && (e.stderr || e.message)) ? String(e.stderr || e.message).trim() : 'sem resposta');
    return '';
  }
}

/* Módulos, tipos e membros, das tabelas do VM. Lido uma vez. */
let META = { modulos: {}, tipos: {}, acesso: {}, excecoes: [] };
function carregaMeta() {
  ULTIMO_ERRO_MOTOR = '';
  const bruto = motor(['--metadata']);
  if (bruto.trim().startsWith('{')) {
    try { META = JSON.parse(bruto); return; } catch (_) { /* fica o vazio */ }
  }
  META_ERRO = 'PoolScript: nao consegui rodar o motor `' + POOL + ' --metadata`'
            + (ULTIMO_ERRO_MOTOR ? ' (' + ULTIMO_ERRO_MOTOR + ')' : '')
            + '. Sem ele a completion fica vazia: aponte o binario em initializationOptions.pool '
            + 'ou ponha o `pool` no PATH do editor.';
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
  const t0 = bruto.trim();
  if (!t0.startsWith('[') && !t0.startsWith('{')) return texto;
  let toks;
  try {
    const j = JSON.parse(bruto);
    toks = Array.isArray(j) ? j : (Array.isArray(j.tokens) ? j.tokens : null);
  } catch (_) { return texto; }
  if (!toks) return texto;
  const pilha = [];
  const par = { LPAREN: ')', LBRACK: ']', LBRACE: '}' };
  const fecha = { RPAREN: 'LPAREN', RBRACK: 'LBRACK', RBRACE: 'LBRACE' };
  for (const t of toks) {
    /* o que está DENTRO de uma f-string (`em: "fstring"`) já está contado no
     * token da própria f-string: contar de novo o `(` de `f"{g(1)}"` fecharia
     * um parêntese que não está aberto */
    if (t.em) continue;
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
  /* A palavra PELA METADE, onde o parser não aceita nome solto: no corpo de
   * uma Entity, `f` sozinho é `dentro de Entity entra 'funct', decorador ou
   * campo` — e o erro derruba a árvore INTEIRA. O editor ficava sem nada pra
   * sugerir bem enquanto se digita: era por isso que o construtor, os campos
   * e os métodos da própria classe sumiam. Se a árvore voltou vazia, tira o
   * fragmento e tenta de novo. O `--ast` extra só roda nesse caso. */
  let arvore = arvoreDe(fechaAbertos(remendaPontosSoltos(comCursor)));
  if (!arvore || !(arvore.lista || []).length) {
    const sem = semParcialNoCursor(comCursor, off);
    if (sem !== null) {
      const alt = arvoreDe(fechaAbertos(remendaPontosSoltos(sem)));
      if (alt && (alt.lista || []).length) arvore = alt;
    }
  }
  const idx = A.indexa(arvore);
  CACHE_CUR.set(doc.uri, { versao: doc.version, linha: pos.line, col: pos.character, idx });
  return idx;
}

/* O texto sem o pedaço de nome que está sendo digitado logo antes do cursor.
 * `null` quando não há fragmento nenhum ali. */
function semParcialNoCursor(texto, off) {
  let i = off;
  while (i > 0) {
    const c = texto[i - 1];
    const nome = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                 (c >= '0' && c <= '9') || c === '_' || c.charCodeAt(0) > 127;
    if (!nome) break;
    i--;
  }
  return i === off ? null : texto.slice(0, i) + texto.slice(off);
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
  /* O motor devolve `{"tokens":[…],"erros":[…]}`; a forma antiga era um array
   * puro, e com erro de lexer vinha `[]` — o realce morria por um caractere.
   * As duas formas são aceitas: a vsix pode estar rodando um motor mais velho
   * que o instalado. */
  const t0 = bruto.trim();
  if (t0.startsWith('[') || t0.startsWith('{')) {
    try {
      const j = JSON.parse(bruto);
      toks = Array.isArray(j) ? j : (Array.isArray(j.tokens) ? j.tokens : []);
    } catch (_) { toks = []; }
  }
  for (const t of toks) { t.l0 = t.l - 1; t.c0 = t.c - 1; }
  CACHE_TOK.set(doc.uri, { versao: doc.version, toks });
  return toks;
}

/* O cursor está dentro de um `{...}` de f-string? Ali é CÓDIGO, não texto: o
 * motor manda o intervalo de cada interpolação no token da f-string (`interp`),
 * medido pelo lexer. Sem isto, hover, definição e completion paravam na aspa —
 * `f"ola, {nome.upper()}"` era uma string e mais nada. */
function dentroDeInterpolacao(t, pos) {
  for (const r of t.interp || []) {
    const depoisDoInicio = pos.line > r.l - 1 || (pos.line === r.l - 1 && pos.character >= r.c - 1);
    const antesDoFim = pos.line < r.l2 - 1 || (pos.line === r.l2 - 1 && pos.character <= r.c2 - 1);
    if (depoisDoInicio && antesDoFim) return true;
  }
  return false;
}

function dentroDeTextoLivre(doc, pos) {
  for (const t of tokensDe(doc)) {
    if (t.t !== 'STR' && t.t !== 'FSTRING' && t.t !== 'BYTES' && t.t !== 'COMMENT') continue;
    if (t.t === 'FSTRING' && dentroDeInterpolacao(t, pos)) return false;
    if (t.l0 !== pos.line) continue;
    if (pos.character > t.c0 && pos.character <= t.c0 + t.n) return true;
  }
  return false;
}

/* ── arquivos importados ─────────────────────────────────────────────────
 *
 * A ordem de resolução é a da linguagem (docs/linguagem/09-imports.md §9.5):
 * stdlib, lib global instalada, arquivo do projeto. */
/* `$POOLSCRIPT_HOME/libs`, ou `~/.poolscript/libs` — a regra do motor
 * (`pasta_libs`) e do `psl`. O editor só olhava o HOME: com `POOLSCRIPT_HOME`
 * definido, o completion descrevia uma pasta de libs que o motor não usa. */
function pastaLibs() {
  const over = process.env.POOLSCRIPT_HOME;
  if (over) return path.join(over, 'libs');
  const h = process.env.HOME;
  return h ? path.join(h, '.poolscript', 'libs') : '';
}

/* `import poolscript.libs.random`: o nome qualificado da lib instalada. */
const PREFIXO_LIBS = 'poolscript.libs.';

function libsInstaladas() {
  try {
    return fs.readdirSync(pastaLibs())
      .filter((f) => f.endsWith('.pr'))
      .map((f) => f.slice(0, -3));
  } catch (_) { return []; }
}

/* `dirDoc` é a pasta de QUEM importa; `dirScript`, a do arquivo aberto no
 * editor (o que o usuário roda). São a mesma coisa no documento, e diferem
 * quando se resolve o import de dentro de um módulo — que é o que o `*`
 * atravessa. O motor procura nas duas (`acha_modulo_ps_em`). */
function arquivoDoImport(mod, dirDoc, aspas, pontos, dirScript) {
  /* `import 'caminho/alvo.pr'`: absoluto como está, senão relativo à pasta do
   * documento; como escrito e com a extensão — a mesma busca do motor. */
  if (aspas && A.especificadorEhCaminho(mod)) {
    const base = path.isAbsolute(mod) ? mod : (dirDoc ? path.join(dirDoc, mod) : '');
    if (!base) return '';
    for (const ext of ['', '.pr']) {
      try { if (fs.statSync(base + ext).isFile()) return base + ext; } catch (_) { /* segue */ }
    }
    return '';
  }
  /* `from ..pkg.m import x`: relativo à pasta de quem importa, subindo uma
   * pasta por ponto além do primeiro, e nunca cai nas libs — a regra do
   * motor. Sem isto os pontos eram jogados fora e `..pkg.m` virava `pkg.m`. */
  if (pontos > 0) {
    if (!dirDoc) return '';
    let base = dirDoc;
    for (let i = 1; i < pontos; i++) base = path.dirname(base);
    for (const ext of ['.pr']) {
      const p = path.join(base, mod.split('.').join(path.sep) + ext);
      try { if (fs.statSync(p).isFile()) return p; } catch (_) { /* segue */ }
    }
    return '';
  }
  /* Lib instalada primeiro. Na pasta de libs o nome vai COM os pontos
   * (`libs/a.b.pr`), como no motor — o editor procurava `libs/a/b.pr`, que o
   * motor nunca carrega. O prefixo `poolscript.libs.` sai antes: é o nome
   * qualificado da mesma lib. */
  const libs = pastaLibs();
  if (libs) {
    const nomeLib = mod.startsWith(PREFIXO_LIBS) && mod.length > PREFIXO_LIBS.length
      ? mod.slice(PREFIXO_LIBS.length) : mod;
    const p = path.join(libs, nomeLib + '.pr');
    try { if (fs.statSync(p).isFile()) return p; } catch (_) { /* segue */ }
  }
  const rel = mod.split('.').join(path.sep) + '.pr';
  for (const base of [dirDoc, dirScript]) {
    if (!base) continue;
    const p = path.join(base, rel);
    try { if (fs.statSync(p).isFile()) return p; } catch (_) { /* segue */ }
  }
  return '';
}

/* O índice de OUTRO arquivo .pr — pela mesma árvore, pelo mesmo caminho. */
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

/* O módulo do motor que a referência designa, ou null. Import relativo e
 * caminho entre aspas nunca são módulo do motor: `import './json.pr'` é o
 * arquivo, de propósito — a mesma decisão do `modulo_nativo_de` da VM. */
function nativoDe(mod, aspas, pontos) {
  if (pontos > 0 || (aspas && A.especificadorEhCaminho(mod))) return null;
  return META.modulos[mod] ? mod : null;
}

/* O que um nome importado designa: módulo do motor, ou o índice do arquivo.
 * `linha` é a do cursor: o nome que nenhum import ligou ainda pode ter vindo
 * de um `*`, e só vale se o arquivo não o ligou ali (ver `alvoDaEstrela`). */
function alvoDoImport(doc, nome, linha) {
  const idx = idxDoc(doc);
  const imp = idx.imports.get(nome);
  /* o `*` também vale contra um import NOMEADO: quem vem por último no
   * arquivo é que liga o nome (ver `ligacaoDeTopo`) */
  const est = alvoDaEstrela(doc, nome, linha);
  if (est) return est;
  if (!imp) {
    /* Módulo do motor que nasce ligado, sem import (`Parsing`): a lista vem do
     * `--metadata` do motor. Sem isto o editor só resolvia `x.membro` com `x`
     * vindo de `import`, e `Parsing.` ficava mudo em todo lugar. */
    const semImport = (META.modulos_sem_import || []).includes(nome);
    return semImport && META.modulos[nome] ? { tipo: 'modulo', mod: nome } : null;
  }
  const nativo = nativoDe(imp.mod, imp.aspas, imp.pontos);
  if (nativo && !imp.membro) return { tipo: 'modulo', mod: imp.mod };
  const dirDoc = pastaDoDoc(doc);
  const arq = arquivoDoImport(imp.mod, dirDoc, imp.aspas, imp.pontos, dirDoc);
  if (imp.membro) {
    /* `from mod import X` — o nome ligado é o MEMBRO, não o módulo */
    if (nativo) return { tipo: 'membro_modulo', mod: imp.mod, membro: imp.membro };
    return { tipo: 'membro_arquivo', arquivo: arq, membro: imp.membro };
  }
  if (arq) return { tipo: 'arquivo', arquivo: arq };
  return null;
}

/* ── o que um módulo EXPORTA ─────────────────────────────────────────────
 *
 * UMA regra, a do motor, pra tudo que pergunta "o que este módulo tem":
 * `from m import <cursor>`, `m.<cursor>` e os nomes que um `*` traz. Antes
 * eram duas regras escritas à mão, e as duas mentiam: `_x` sumia (o motor o
 * exporta) e a funct `private` aparecia (o motor recusa com "existe, mas é
 * private"). O editor oferecia o que o programa não roda.
 *
 *   módulo do motor   todo membro da tabela do VM (`--metadata`)
 *   arquivo .pr       o que ele liga no topo (`topo`, ver `analise.js`),
 *                     menos o `private`; o nome que ELE importa sai também,
 *                     resolvido até a origem, e os `*` dele são expandidos
 *
 * Cada item leva de onde veio: `arquivo`/`linha`/`coluna` (declarado num
 * .pr, com `declarado` = o nome lá dentro) ou `mod`/`membro` (membro de módulo
 * do motor) — é isso que o hover e o ir-pra-definição usam.
 *
 * `pilha` são os arquivos em expansão AGORA: `a` com `*` de `b` e `b` com `*`
 * de `a` é ciclo, e deste ponto o arquivo repetido não traz nada — o mesmo
 * corte do `estrela_nomes_de` da VM. */
function exportadosDe(ref, dirModulo, dirScript, pilha) {
  let mod = ref.mod || '';
  let pontos = ref.pontos || 0;
  /* do completion o módulo chega como TEXTO (`..pkg.m`): os pontos da frente
   * são o nível relativo */
  if (!ref.aspas) while (mod.startsWith('.')) { pontos++; mod = mod.slice(1); }
  if (!mod) return [];
  const nativo = nativoDe(mod, ref.aspas, pontos);
  if (nativo) {
    return META.modulos[nativo].map((m) => Object.assign(
      { kind: m.kind === 'value' ? 'campo' : 'action', escopo: nativo, tipo: retornoVisivel(m.retorna) },
      m, { mod: nativo, membro: m.nome }));
  }
  const arq = arquivoDoImport(mod, dirModulo, ref.aspas, pontos, dirScript);
  return arq ? exportadosDeArquivo(arq, dirScript, pilha || new Set()) : [];
}

function exportadosDeArquivo(caminho, dirScript, pilha) {
  let abs = caminho;
  try { abs = fs.realpathSync(caminho); } catch (_) { /* fica o caminho */ }
  if (pilha.has(abs)) return [];
  const idx = indiceDeArquivo(caminho);
  if (!idx) return [];
  const sub = new Set(pilha).add(abs);
  const dirMod = path.dirname(abs);
  /* `private` sai pelo NOME, em qualquer ponto da lista: é como o motor corta */
  const privados = new Set(idx.topo.filter((b) => b.privado).map((b) => b.nome));
  /* nome religado mais abaixo fica com a ÚLTIMA ligação — é ela que o
   * programa tem quando o arquivo termina de rodar; a ordem é a da primeira */
  const porNome = new Map();
  const poe = (x) => { if (x && x.nome && !privados.has(x.nome)) porNome.set(x.nome, x); };
  for (const b of idx.topo) {
    if (b.estrela) {
      for (const x of exportadosDe(b.estrela, dirMod, dirScript, sub)) poe(x);
      continue;
    }
    const local = Object.assign({}, b, { arquivo: caminho, declarado: b.nome });
    if (!b.imp) { poe(local); continue; }
    /* nome que o arquivo IMPORTA e reexporta: resolvido até a origem, com o
     * nome com que foi ligado aqui */
    if (!b.imp.membro) {
      const nativo = nativoDe(b.imp.mod, b.imp.aspas, b.imp.pontos);
      poe(Object.assign(local, { kind: 'modulo', modNativo: nativo || undefined,
        modArquivo: nativo ? undefined : arquivoDoImport(b.imp.mod, dirMod, b.imp.aspas, b.imp.pontos, dirScript) }));
      continue;
    }
    const origem = exportadosDe(b.imp, dirMod, dirScript, sub).find((x) => x.nome === b.imp.membro);
    poe(origem ? Object.assign({}, origem, { nome: b.nome }) : Object.assign(local, { kind: 'variavel' }));
  }
  return [...porNome.values()];
}

/* Os membros de topo de um arquivo .pr importado — o que `mod.` e
 * `from mod import` oferecem. É a regra do motor: `exportadosDeArquivo`. */
function membrosDeArquivo(caminho, dirScript) {
  return exportadosDeArquivo(caminho, dirScript, new Set());
}

/* ── quem ganha: a ORDEM do arquivo ──────────────────────────────────────
 *
 * O motor liga na ordem em que o arquivo roda, e o `*` é uma ligação como
 * outra qualquer. Medido no `./pool` antes de virar código:
 *
 *   x = 5            + `from m import *` embaixo  -> na linha de baixo x é o
 *                                                    do módulo
 *   `from m import *`+ x = 5 embaixo              -> x é a variável
 *   `from m import *`+ `funct soma()` embaixo     -> entre os dois, `soma` é
 *                                                    o do módulo; da linha da
 *                                                    funct pra baixo, a funct
 *                                                    (nome de funct NÃO é
 *                                                    hoisted por cima do `*`)
 *
 * DENTRO de uma funct a linha não decide: a funct roda depois de o arquivo
 * inteiro ter carregado, então vale a última ligação do topo — `funct f()
 * { return x }` lá em cima devolve o x do módulo quando o `*` está no fim do
 * arquivo, e a variável quando é ela que vem por último. */

/* Até que linha do topo contar, pra um cursor na linha `linha`. Dentro de
 * funct/método, o arquivo inteiro. */
function linhaDeOrdem(idx, linha) {
  if (linha === undefined) return Infinity;
  const L = linha + 1;
  for (const e of idx.escopos) {
    if (e.tipo !== 'modulo' && e.ini <= L && L <= e.fim) return Infinity;
  }
  return linha;
}

/* O nome é LOCAL de uma funct/método que contém a linha? Parâmetro e variável
 * de dentro ganham sempre: o slot é da funct, o topo do arquivo não alcança. */
function ligadoLocal(idx, nome, linha) {
  if (linha === undefined) return false;
  const L = linha + 1;
  for (const e of idx.escopos) {
    if (e.tipo === 'modulo' || !(e.ini <= L && L <= e.fim)) continue;
    if (e.liga.some((b) => b.nome === nome)) return true;
  }
  return false;
}

/* A ÚLTIMA ligação do nome no TOPO do arquivo até a linha `ate`:
 * `{ linha, b }` pro que o arquivo liga (funct, class, variável, import) e
 * `{ linha, estrela, item }` quando quem liga por último é um `*`. */
function ligacaoDeTopo(doc, nome, ate) {
  const idx = idxDoc(doc);
  const dir = pastaDoDoc(doc);
  let melhor = null;
  for (const b of idx.topo || []) {
    const linha = b.estrela ? b.estrela.linha : b.linha;
    if (linha > ate) continue;
    if (b.estrela) {
      const item = exportadosDe(b.estrela, dir, dir, new Set()).find((m) => m.nome === nome);
      if (item) melhor = { linha, estrela: b.estrela, item };
      continue;
    }
    if (b.nome === nome) melhor = { linha, b };
  }
  return melhor;
}

/* O alvo de um nome trazido por `*`, no formato de `alvoDoImport` — ou null
 * quando não é a estrela que vale ali (é local da funct, ou o arquivo religou
 * o nome depois dela). */
function alvoDaEstrela(doc, nome, linha) {
  const idx = idxDoc(doc);
  if (!(idx.estrelas || []).length || ligadoLocal(idx, nome, linha)) return null;
  const lig = ligacaoDeTopo(doc, nome, linhaDeOrdem(idx, linha));
  if (!lig || !lig.estrela) return null;
  const x = lig.item;
  if (x.mod && x.membro) return { tipo: 'membro_modulo', mod: x.mod, membro: x.membro };
  if (x.kind === 'modulo') {
    if (x.modNativo) return { tipo: 'modulo', mod: x.modNativo };
    return x.modArquivo ? { tipo: 'arquivo', arquivo: x.modArquivo } : null;
  }
  return { tipo: 'membro_arquivo', arquivo: x.arquivo, membro: x.declarado || x.nome, def: x };
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
  /* Entity trazida por `*`: só a class que o arquivo EXPORTA (a `private
   * class` não vem), e só quando é a estrela que liga o nome no fim do
   * arquivo — é lá que uma Entity é usada, depois de tudo carregado */
  const alvoE = alvoDaEstrela(doc, nome);
  if (alvoE && alvoE.tipo === 'membro_arquivo' && alvoE.def && alvoE.def.kind === 'class') {
    const ix = indiceDeArquivo(alvoE.arquivo);
    const e = ix && ix.entidades.find((y) => y.nome === alvoE.membro);
    if (e) return e;
  }
  return null;
}

/* Membros de uma Entity COM herança. `interno` = o cursor está dentro dela,
 * então `private` conta — a mesma regra que a VM impõe em runtime. */
/* O membro `static` que o nome SOLTO alcança de dentro da classe `ent` — o
 * que `App.x` alcança de fora, `x` alcança de dentro (campo e método static,
 * da classe e dos pais). Parâmetro ou variável do método com o mesmo nome
 * ganha do campo, como no motor: quem chama confere o local antes. */
function estaticoSolto(doc, ent, nome) {
  if (!ent) return null;
  return membrosDaEntidade(doc, ent.nome, true).find((m) => m.nome === nome && m.estatica) || null;
}

/* A ligação `nome` visível na linha que NÃO é do módulo (parâmetro ou
 * variável do próprio método/funct): é a que sombreia o membro static. */
function ligacaoLocal(idx, nome, linha) {
  for (const b of A.visiveisEm(idx, linha)) {
    if (b.nome !== nome) continue;
    const esc = idx.escopos.find((s) => s.liga.includes(b));
    if (esc && esc.tipo !== 'modulo') return b;
  }
  return null;
}

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

  const alvo = alvoDoImport(doc, nome, linha);
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
      const alvoC = alvoDoImport(doc, cons.nome, linha);
      if (alvoC && alvoC.tipo === 'membro_modulo') {
        for (const m of META.modulos[alvoC.mod] || []) {
          if (m.nome !== alvoC.membro) continue;
          const a = alvoDoRetorno(m.retorna, { mod: alvoC.mod, membro: alvoC.membro });
          if (a) return a;
        }
      }
      if (alvoC && alvoC.tipo === 'membro_arquivo' && achaEntidade(doc, alvoC.membro))
        return { tipo: 'entity', nome: alvoC.membro, interno: false };
    }
    if (cons.mod) {
      const alvoM = alvoDoImport(doc, cons.mod, linha);
      if (alvoM && alvoM.arquivo) {
        const ix = indiceDeArquivo(alvoM.arquivo);
        if (ix && ix.entidades.some((e) => e.nome === cons.nome))
          return { tipo: 'entity', nome: cons.nome, interno: false };
      }
      if (alvoM && alvoM.mod && META.modulos[alvoM.mod]) {
        for (const m of META.modulos[alvoM.mod]) {
          if (m.nome !== cons.nome) continue;
          const via = { mod: alvoM.mod, membro: cons.nome };
          const a = alvoDoRetorno(m.retorna, via);
          if (a) return a;
          const t = tipoEncadeado(m.retorna);
          if (t) return { tipo: 'tipo_motor', nome: t, via };
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
      else if (v.k === 'Literal' && typeof v.texto === 'string') {
        /* `h = b"q"` chega com o mesmo `texto` que `"q"`; o `--ast` marca
         * `lit: "bytes"` e é isso que separa `decode` de `upper` */
        achado = { literal: v.lit === 'bytes' ? 'byte' : 'str' };
      }
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
  return membrosDe(doc, alvoDaCadeia(doc, partes, linha), linha);
}

/* O ALVO que a cadeia `a.b.c` designa (Entity, módulo, tipo do motor,
 * universal…), ou null quando um passo não existe. Separado de
 * `membrosDaCadeia` porque o hover precisa saber se o alvo é `universal`
 * (tipo desconhecido — cabe listar candidatos) ou uma Entity/módulo onde o
 * membro simplesmente não existe (aí é erro de digitação, fica mudo). */
function alvoDaCadeia(doc, partes, linha) {
  if (!partes.length) return null;
  let alvo = tipoDoNome(doc, partes[0], linha);
  if (!alvo) return null;

  for (let i = 1; i < partes.length; i++) {
    const passo = partes[i];
    const membros = membrosDe(doc, alvo, linha);
    const m = membros.find((x) => x.nome === passo);
    if (!m) return null;
    /* desce um nível: o tipo do membro é o que ele devolve ou declara. A
     * procedência (`via`: de que módulo/membro o tipo saiu) desce junto —
     * é ela que diz em que pasta da doc está a prosa do próximo membro. */
    const t = tipoEncadeado(m.retorna || m.tipo);
    const modBase = alvo.via ? alvo.via.mod : (alvo.tipo === 'import' && alvo.alvo ? alvo.alvo.mod : null);
    const via = modBase ? { mod: modBase, membro: passo } : undefined;
    const uniao = !t ? alvoDoRetorno(m.retorna || m.tipo, via) : null;
    if (t && META.tipos[t]) alvo = { tipo: 'tipo_motor', nome: t, via };
    else if (uniao) alvo = uniao;
    else if (t && achaEntidade(doc, t)) alvo = { tipo: 'entity', nome: t, interno: false };
    else if (m.kind === 'class' && achaEntidade(doc, m.nome)) alvo = { tipo: 'entity', nome: m.nome, interno: false };
    /* membro existe, retorno desconhecido (`request.get(...).`): universais */
    else alvo = { tipo: 'universal' };
  }
  return alvo;
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
  if (alvo.tipo === 'uniao') {
    /* Retorno com mais de um tipo (`os.run` devolve int, str ou Process): os
     * membros dos dois lados, sem repetir, cada um dizendo de qual tipo veio.
     * O universal entra uma vez só, no fim. */
    const vistos = new Set();
    const juntos = [];
    for (const t of alvo.nomes) {
      const escopo = alvo.via ? [`${alvo.via.mod}/${alvo.via.membro}`, alvo.via.mod, t] : t;
      for (const m of META.tipos[t] || []) {
        if (vistos.has(m.nome)) continue;
        vistos.add(m.nome);
        juntos.push(Object.assign({ kind: 'action', escopo, de: t }, m));
      }
    }
    for (const m of META.tipos.__universal__ || []) {
      if (vistos.has(m.nome)) continue;
      vistos.add(m.nome);
      juntos.push(Object.assign({ kind: 'action', escopo: '__universal__' }, m));
    }
    return juntos;
  }
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
    /* `m.`: o que o módulo exporta, pela regra única do motor */
    if (a.tipo === 'modulo') return exportadosDe({ mod: a.mod }, '', '', new Set());
    if (a.tipo === 'arquivo') return membrosDeArquivo(a.arquivo, pastaDoDoc(doc));
    if (a.tipo === 'membro_modulo') {
      for (const m of META.modulos[a.mod] || []) {
        const t = tipoEncadeado(m.retorna);
        if (m.nome === a.membro && t && META.tipos[t])
          return (META.tipos[t] || [])
            .map((x) => Object.assign({ kind: 'action', escopo: [`${a.mod}/${a.membro}`, a.mod, t] }, x));
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

/* Um parâmetro, escrito como no fonte: tipo ANTES do nome
 * (`funct f(str corpo, int n = 2)`). Sem tipo sai só o nome.
 * A regra mora AQUI porque o rótulo aparece em dois lugares — a assinatura da
 * completion/hover e o signatureHelp — e cada um tinha a sua cópia. */
function rotuloParam(p) {
  const nome = p.tipo ? `${p.tipo} ${p.nome}` : p.nome;
  return (p.default === null || p.default === undefined) ? nome : `${nome} = ${p.default}`;
}

/* O `retorna` do motor é MEDIDO (scripts/mede_retornos.pr) e vem em três
 * formas: um tipo (`DbCursor`), uma união (`dict|Null`, cada lado medido) e
 * `*` (o tipo do conteúdo guardado: `d.get`, `json.parse`). `*` não é nome de
 * tipo, então não aparece na assinatura; e encadear `cur.fetchone().` só tem
 * por onde ir pelo lado que tem membros — o `Null` da união não tem nenhum. */
function retornoVisivel(ret) {
  return ret && ret !== '*' ? ret : '';
}

function tipoEncadeado(ret) {
  if (!ret || ret === '*') return '';
  const lados = ret.split('|').filter((x) => x !== 'Null');
  return lados.length === 1 ? lados[0] : '';
}

/* Os lados de um retorno com MAIS DE UM tipo (`int|str|Process`), só os que o
 * motor publica como tipo. Antes qualquer união virava "desconhecido" e o
 * completion caía nos universais: `p = os.run(..., capture="live")` + `p.`
 * oferecia só `type`, e o mesmo valia pra `psodbc.connect(...)`. */
function tiposDaUniao(ret) {
  if (!ret || ret === '*') return [];
  return ret.split('|').filter((x) => x !== 'Null' && META.tipos && META.tipos[x]);
}

/* O alvo de um retorno: um tipo só, uma UNIÃO de tipos, ou nada. */
function alvoDoRetorno(ret, via) {
  const t = tipoEncadeado(ret);
  if (t && META.tipos[t]) return { tipo: 'tipo_motor', nome: t, via };
  const lados = tiposDaUniao(ret);
  if (lados.length === 1) return { tipo: 'tipo_motor', nome: lados[0], via };
  if (lados.length > 1) return { tipo: 'uniao', nomes: lados, via };
  return null;
}

function assinatura(m) {
  const ps = (m.params || []).map(rotuloParam);
  const ret = retornoVisivel(m.retorna) ? ` -> ${m.retorna}` : '';
  if (m.kind === 'campo') return `${m.tipo ? m.tipo + ' ' : ''}${m.nome}`;
  if (m.kind === 'class') return `class ${m.nome}`;
  /* módulo que um arquivo importa e reexporta (`import os as o` no topo dele) */
  if (m.kind === 'modulo') return `modulo ${m.nome}`;
  return `${m.nome}(${ps.join(', ')})${ret}`;
}

function itemDeMembro(m, deOnde) {
  const priv = (m.privado ? ' · private' : '')
             + (m.estatica ? ' · static' : '') + (m.nonnull ? ' · nonnull' : '');
  const herd = deOnde && m.de && m.de !== deOnde ? ` · de ${m.de}` : '';
  const kind = m.kind === 'campo' ? CompletionItemKind.Field
             : m.kind === 'class' ? CompletionItemKind.Class
             : m.kind === 'modulo' ? CompletionItemKind.Module
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
  let receptor = null;                       /* `"a,b".split(` — literal como receptor */
  let cur = callee;
  while (cur) {
    if (cur.k === 'Name') { partes.unshift(cur.texto); break; }
    if (cur.k === 'MemberAccess') { partes.unshift(cur.texto); cur = cur.a; continue; }
    if (cur.k === 'Literal' && typeof cur.texto === 'string' && partes.length) {
      receptor = cur.lit === 'bytes' ? 'byte' : 'str';
      break;
    }
    return null;
  }
  const args = melhor.lista || [];
  const nomeados = new Set();
  let posicionais = 0;
  for (const a of args) {
    if (a.texto) nomeados.add(a.texto);
    else if (a.i2 === 2) continue;           /* `**d` espalha NOMEADOS, não conta como posicional */
    else posicionais++;
  }
  const chamado = (receptor ? receptor + '.' : '') + partes.join('.');
  return { partes, receptor, chamado, posicionais, nomeados };
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

/* Os parâmetros de um nome que veio de IMPORT — nomeado (`from m import f`)
 * ou trazido por um `*`. Módulo do motor: a tabela do `--metadata`. Arquivo
 * `.pr`: a assinatura da declaração, que o item exportado já traz. Sem isto o
 * signatureHelp ficava mudo em `f(` justamente no nome que veio de fora. */
function paramsDoImportado(doc, alvo) {
  if (!alvo) return [];
  if (alvo.tipo === 'membro_modulo') {
    const m = (META.modulos[alvo.mod] || []).find((x) => x.nome === alvo.membro);
    return m ? (m.params || []) : [];
  }
  if (alvo.tipo === 'membro_arquivo') {
    const d = alvo.def
      || (alvo.arquivo ? membrosDeArquivo(alvo.arquivo, pastaDoDoc(doc)).find((x) => x.nome === alvo.membro) : null);
    return d ? (d.params || []) : [];
  }
  return [];
}

function paramsDoChamado(doc, ch, linha) {
  if (ch.receptor) {                         /* `"a,b".split(` / `b"x".decode(` */
    const m = membrosDe(doc, { tipo: 'tipo_motor', nome: ch.receptor }, linha)
                .find((x) => x.nome === ch.partes[ch.partes.length - 1]);
    return m ? (m.params || []) : [];
  }
  if (ch.partes.length > 1) {
    const membros = membrosDaCadeia(doc, ch.partes.slice(0, -1), linha);
    const m = membros.find((x) => x.nome === ch.partes[ch.partes.length - 1]);
    return m ? (m.params || []) : [];
  }
  /* o `*` que vale NESTA linha vem antes do que o arquivo declara mais
   * abaixo — a ordem do motor; `alvoDaEstrela` devolve null quando não é ele
   * que liga o nome ali */
  const est = alvoDaEstrela(doc, ch.chamado, linha);
  if (est) return paramsDoImportado(doc, est);
  const idx = idxDoc(doc);
  for (const b of A.visiveisEm(idx, linha)) {
    if (b.nome === ch.chamado && b.params) return b.params;
  }
  const e = achaEntidade(doc, ch.chamado);
  if (e) return e.membros.filter((m) => m.nome === '__init__')
                         .map((m) => m.params || []).flat();
  return paramsDoImportado(doc, alvoDoImport(doc, ch.chamado, linha));
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
    /* a versão do MOTOR que o servidor usa (`PoolScript 15.91.21 [PSVM]` →
     * `15.91.21`): era `'3'` fixo, e com dois servidores diferentes no ar
     * (o embutido na vsix e o instalado) não havia como ver qual respondia */
    serverInfo: { name: 'poolscript-lsp', version: versaoDoMotor() },
  };
});

function versaoDoMotor() {
  const partes = String(motor(['--version']) || '').split(' ');
  const v = partes.find((p) => p.length > 0 && [...p].every((ch) => (ch >= '0' && ch <= '9') || ch === '.'));
  return v || 'desconhecida';
}

/* Depois do aperto de mão, o que o `initialize` descobriu e não pode ficar
 * calado: sem o motor, o cliente recebe a mensagem (balão no IntelliJ, toast
 * no VS Code, linha no Neovim) e o detalhe vai pro log dele. Notificações,
 * nunca request — o servidor continua sem pedir nada ao cliente. */
conexao.onInitialized(() => {
  if (!META_ERRO) return;
  /* `window/showMessage` (notificação), e não o `showWarningMessage` da lib,
   * que vira `window/showMessageRequest` — um request ao cliente. */
  conexao.sendNotification('window/showMessage', { type: MessageType.Warning, message: META_ERRO });
  conexao.console.warn(META_ERRO + ' | PATH=' + (process.env.PATH || '') + ' | cwd=' + process.cwd());
});

/* diagnóstico: quem decide é o `--check` do motor, não uma segunda gramática.
 *
 * O buffer vai pelo stdin, e o `--path` diz DE QUAL arquivo ele é: sem isso o
 * motor não sabe onde o arquivo mora, não acha os módulos vizinhos e pula a
 * conferência entre arquivos calado — `util.naoexiste()` de um `import util`
 * ao lado não aparecia no editor. */
function diagnostica(doc) {
  const caminho = caminhoDoDoc(doc.uri);
  const args = caminho ? ['--check', '--path', caminho] : ['--check'];
  const bruto = motor(args, doc.getText());
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
    /* A tipagem estática lista TODOS os erros do arquivo em `erros` (o
     * primeiro também vem nos campos de cima). Erro de sintaxe é um só. */
    const lista = Array.isArray(r.erros) && r.erros.length
      ? r.erros
      : [{ tipo: r.tipo, msg: r.msg, linha: r.linha, coluna: r.coluna }];
    lista.slice().reverse().forEach((e) => {
      const linha = Math.max(0, (e.linha || 1) - 1);
      const col = Math.max(0, (e.coluna || 1) - 1);
      diags.unshift({
        severity: DiagnosticSeverity.Error,
        range: { start: { line: linha, character: col }, end: { line: linha, character: col + 1 } },
        message: `${e.tipo}: ${e.msg}`,
        source: 'poolscript',
      });
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

/* O caminho do documento no disco ('' se a URI não é de arquivo). Aceita
 * `file:///…`, `file:/…` (uma barra só — o fallback do LSP4J) e o
 * percent-encoding (`%20`, `%C3%A7`): o IntelliJ manda a URI codificada, e
 * cortar `file://` na unha deixava a pasta com `%20` dentro — sem os vizinhos
 * no `import`. */
function caminhoDoDoc(uri) {
  try { return url.fileURLToPath(uri); } catch (_) { return ''; }
}

/* A pasta do documento no disco ("" se a URI não é de arquivo). */
function pastaDoDoc(doc) {
  const c = caminhoDoDoc(doc.uri);
  return c ? path.dirname(c) : '';
}

/* A URI de um caminho do disco, codificada como o cliente exige: `'file://'
 * + caminho` cru, com espaço ou acento, era rejeitado pelo IntelliJ. */
function uriDe(caminho) {
  return url.pathToFileURL(path.resolve(caminho)).href;
}

/* Os membros que `from X import …` pode trazer: o que X EXPORTA — módulo do
 * motor, lib instalada ou arquivo `.pr` ao lado —, pela regra do motor. O
 * editor não oferece o que o `import` vai recusar. */
function membrosParaImport(doc, mod, aspas) {
  const dir = pastaDoDoc(doc);
  return exportadosDe({ mod, aspas }, dir, dir, new Set());
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

/* Dentro das aspas de um import: arquivos `.pr` e pastas a partir da pasta do
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
    const meu = path.basename(caminhoDoDoc(doc.uri));
    let ents = [];
    try { ents = fs.readdirSync(base, { withFileTypes: true }); } catch (_) { ents = []; }
    for (const e of ents) {
      if (e.name.startsWith('.') || (!sub && e.name === meu)) continue;
      if (e.isDirectory()) poe(e.name, CompletionItemKind.Folder, 'pasta', e.name + '/');
      else if (e.name.endsWith('.pr')) poe(e.name, CompletionItemKind.File, 'arquivo .pr');
    }
    poe('..', CompletionItemKind.Folder, 'pasta acima', '../');
  }
  return itens;
}

/* Dentro de um `import`/`from`. Dois casos, decididos pelos TOKENS da linha
 * (o lexer), nunca pelo texto:
 *
 *  - `from X import <cursor>`: os MEMBROS de X — módulo do motor, lib
 *    instalada ou arquivo `.pr` — menos os já listados antes do cursor. Era o
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
  /* `from m import …` e `PUSH m GET …` são a MESMA forma escrita de dois
   * jeitos: muda só a palavra que separa o módulo dos nomes */
  const ehFrom = antes.length > 0 && antes[0].t === 'KW' && (antes[0].v === 'from' || antes[0].v === 'PUSH');
  const separa = ehFrom && antes[0].v === 'PUSH' ? 'GET' : 'import';
  const iImp = antes.findIndex((t, i) => i > 0 && t.t === 'KW' && t.v === separa);
  const ehNome = (t) => t.t === 'DOT' || t.t.startsWith('IDENT');

  if (ehFrom && iImp > 0) {
    /* `from 'x/y.pr' import <cursor>`: o módulo é a STRING. Entre `from` e
     * `import` todo token é caminho do módulo — inclusive palavra-chave: o
     * lexer entrega `json` como KW, e filtrar só IDENT deixava `from json
     * import ` sem módulo nenhum. */
    const aspas = antes.length > 1 && antes[1].t === 'STR';
    const mod = aspas ? antes[1].v
      : antes.slice(1, iImp).filter((t) => ehNome(t) || t.t === 'KW').map((t) => t.v).join('');
    const jaTem = new Set(antes.slice(iImp + 1).filter((t) => t.t.startsWith('IDENT')).map((t) => t.v));
    const itens = membrosParaImport(doc, mod, aspas).filter((m) => !jaTem.has(m.nome)).map((m) => itemDeMembro(m, mod));
    /* `*` traz tudo que o módulo exporta. Só no lugar do PRIMEIRO nome: o
     * motor recusa `from m import a, *` ("não se mistura com uma lista") */
    if (iImp === antes.length - 1) {
      itens.unshift({ label: '*', kind: CompletionItemKind.Keyword, sortText: '0',
                      detail: `todos os nomes que ${mod} exporta (sem ligar o nome ${mod})` });
    }
    return itens;
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
    const meu = path.basename(caminhoDoDoc(doc.uri));
    let ents = [];
    try { ents = fs.readdirSync(base, { withFileTypes: true }); } catch (_) { ents = []; }
    for (const e of ents) {
      if (e.name.startsWith('.') || e.name === meu) continue;
      if (e.isDirectory()) poe(e.name, 'pasta');
      else if (e.name.endsWith('.pr')) poe(e.name.slice(0, -3), 'arquivo .pr');
    }
  }
  return itens;
}

/* `"a,b".` — o token antes do ponto é um LITERAL: os membros são os do tipo
 * dele — `str` para STR e FSTRING, `byte` para BYTES (`b"x".decode`). Só
 * literal de texto/bytes: `]` e `}` podem ser índice ou fim de literal, e o
 * tipo de `x[0]` ninguém sabe aqui. */
function receptorLiteral(doc, pos) {
  const toks = tokensDe(doc).filter((t) => t.l0 === pos.line && t.n > 0 && t.c0 + t.n <= pos.character);
  const n = toks.length;
  if (n < 2 || toks[n - 1].t !== 'DOT') return null;
  const t = toks[n - 2].t;
  if ((t === 'STR' || t === 'FSTRING') && META.tipos.str) return 'str';
  if (t === 'BYTES' && META.tipos.byte) return 'byte';
  return null;
}

/* Receptor de tipo DESCONHECIDO (`lines[1].decode`, `head.decode` com `head`
 * vindo de um índice): o hover não inventa o tipo — lista os tipos do motor
 * que TÊM um membro com esse nome, os declaráveis (`tipos_nomes`) primeiro,
 * com a prosa do primeiro que tiver página. Antes devolvia nada, e "nada" era
 * lido como "esse método não existe". Tudo sai do `--metadata`. */
function hoverCandidatos(nome) {
  const decl = META.tipos_nomes || [];
  const cands = [];
  for (const t of Object.keys(META.tipos || {})) {
    if (t === '__universal__') continue;
    const m = (META.tipos[t] || []).find((x) => x.nome === nome);
    if (m) cands.push({ t, m });
  }
  if (!cands.length) return null;
  const ordem = (t) => { const i = decl.indexOf(t); return i < 0 ? decl.length : i; };
  cands.sort((a, b) => (ordem(a.t) - ordem(b.t)) || a.t.localeCompare(b.t));
  const TETO = 6;
  const linhas = cands.slice(0, TETO).map((c) => c.t + '.' + assinatura(c.m));
  const resto = cands.length > TETO ? ' · e mais ' + (cands.length - TETO) : '';
  let prosa = '';
  for (const c of cands) { prosa = resumoDe(c.t, nome); if (prosa) break; }
  return md('```ps\n' + linhas.join('\n') + '\n```\n\nreceptor de tipo desconhecido · '
            + cands.length + (cands.length === 1 ? ' tipo tem' : ' tipos têm') + ' `' + nome + '`' + resto
            + (prosa ? '\n\n' + prosa : ''));
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

/* O valor de um parâmetro de campo de model, como foi escrito: literal
 * (o `--ast` traz `lit` e o valor), lista de literais, `-n`, ou o nome de
 * tipo/model do `of=`. */
function valorDeParamTxt(v) {
  if (!v) return '';
  if (v.k === 'Literal') {
    if (v.lit === 'str') return JSON.stringify(v.texto || '');
    if (v.lit === 'bool') return v.i ? 'true' : 'false';
    if (v.lit === 'int' || v.lit === 'flo') return String(v.lit === 'int' ? v.i : v.d);
    return v.texto || '…';
  }
  if (v.k === 'ListLiteral') return '[' + (v.lista || []).map(valorDeParamTxt).join(', ') + ']';
  if (v.k === 'UnaryOp') return (v.texto || '') + valorDeParamTxt(v.a);
  if (v.k === 'Name') return v.texto || '';
  return '…';
}

/* `(length=20, regex="…")` do campo `f` de um model, como no fonte; '' sem
 * parâmetros. `length` mora em `i2`; os outros são os nós de `lista`. */
function paramsDeCampoTxt(f) {
  const ps = [];
  if (f.i2 >= 0) ps.push('length=' + f.i2);
  for (const e of f.lista || []) if (e && e.texto) ps.push(e.texto + '=' + valorDeParamTxt(e.a));
  return ps.length ? '(' + ps.join(', ') + ')' : '';
}

/* Os parâmetros do campo de `model`, como a doc 08 §8.1.2 os lista: nome →
 * o que confere. É a tabela do motor (ps_parser.c, `PARAMS`), escrita uma
 * vez aqui pro completion. */
const PARAMS_MODEL = [
  ['length',   'máximo de caracteres (str), dígitos (int) ou itens (list)'],
  ['regex',    'o valor inteiro casa o padrão (str)'],
  ['in',       'o valor é um dos da lista'],
  ['not_in',   'o valor não é nenhum da lista (proibidos)'],
  ['min',      'mínimo, inclusivo (int, flo)'],
  ['max',      'máximo, inclusivo (int, flo)'],
  ['optional', 'pode faltar ou vir null'],
  ['of',       'tipo de cada item (list): str, int, flo, bool, dict ou um model'],
];

/* O cursor está entre os parênteses de um campo de `model`? Então os itens
 * são os parâmetros que ainda não estão na linha; senão null. Lido nos
 * TOKENS do motor, não no texto: a linha do campo é `nome : tipo (` antes
 * do cursor, e o `{` aberto mais próximo acima é o de um `model Nome() {`.
 * Sem árvore de propósito — a linha pela metade (`x: str(`) não parseia, e
 * é exatamente quando o completion é pedido. */
function paramsDeCampoDeModel(doc, pos) {
  /* sem os tokens de leiaute (NEWLINE/INDENT/DEDENT): o INDENT da linha do
   * campo vem antes do nome e não é o nome */
  const toks = tokensDe(doc).filter((t) => t.t !== 'NEWLINE' && t.t !== 'INDENT' && t.t !== 'DEDENT');
  const ehNome = (t) => t && (t.t === 'IDENT' || t.t === 'IDENT_UPPER' || t.t === 'KW');
  /* os tokens desta linha, antes do cursor */
  const antes = toks.filter((t) => t.l0 === pos.line && t.c0 < pos.character);
  if (antes.length < 4 || !ehNome(antes[0]) || antes[1].t !== 'COLON' || !ehNome(antes[2])
      || antes[3].t !== 'LPAREN') return null;
  if (antes.slice(4).some((t) => t.t === 'RPAREN')) return null;
  /* o `{` aberto mais próximo acima, saltando blocos fechados */
  let i = toks.findIndex((t) => t === antes[0]);
  let prof = 0, abre = -1;
  for (i--; i >= 0; i--) {
    if (toks[i].t === 'RBRACE') prof++;
    else if (toks[i].t === 'LBRACE') { if (prof === 0) { abre = i; break; } prof--; }
  }
  if (abre < 4) return null;
  const cab = toks.slice(abre - 4, abre);          /* model Nome ( ) */
  if (!(cab[0].t === 'KW' && cab[0].v === 'model' && ehNome(cab[1])
        && cab[2].t === 'LPAREN' && cab[3].t === 'RPAREN')) return null;
  /* parâmetros que a linha já tem: nome seguido de `=` */
  const jaTem = new Set();
  for (let k = 4; k + 1 < antes.length; k++)
    if (ehNome(antes[k]) && antes[k + 1].t === 'OP' && antes[k + 1].v === '=') jaTem.add(antes[k].v);
  return PARAMS_MODEL.filter(([nome]) => !jaTem.has(nome)).map(([nome, oque], i) => ({
    label: `${nome}=`, kind: CompletionItemKind.Variable, detail: oque, sortText: String(i),
  }));
}

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
  /* Dentro dos parênteses de um CAMPO de `model` (`nome: str(<cursor>`): os
   * parâmetros que validam o dado — `length`, `regex`, `in`, `not_in`, `min`,
   * `max`, `optional`, `of` — menos os que a linha já tem. Vem antes da
   * chamada porque isto não é chamada: `str(` aqui é o tipo do campo. */
  if (!cad.partes.length && dentroDeParenteses(doc, p.position)) {
    const ps = paramsDeCampoDeModel(doc, p.position);
    if (ps) return ps;
  }

  const ch = dentroDeParenteses(doc, p.position) ? chamadaEm(doc, p.position) : null;
  if (ch && !cad.partes.length) {
    const ps = paramsDoChamado(doc, ch, p.position.line);
    /* `*args`/`**kwarg` não existem como nomeado (`args=` cai no dict, e
     * `*args=` é SyntaxError): não se oferecem */
    const faltam = ps.filter((x, i) => i >= ch.posicionais && !ch.nomeados.has(x.nome)
                                       && !String(x.nome).startsWith('*'));
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
  /* Os membros `static` da classe (e dos pais) pelo nome SOLTO: o motor os
   * resolve de dentro de qualquer método e do corpo da classe, e o editor
   * não os oferecia — "nada é visível se não tiver self". Um local com o
   * mesmo nome já entrou acima e ganha (o `poe` não repete). */
  if (ent) {
    for (const m of membrosDaEntidade(doc, ent.nome, true)) {
      if (!m.estatica) continue;
      const det = m.kind === 'action'
        ? 'static ' + assinatura(m) + ' — de ' + m.de
        : 'static ' + (m.tipo ? m.tipo + ' ' : '') + m.nome + ' — de ' + m.de;
      poe(m.nome, m.kind === 'action' ? CompletionItemKind.Method : CompletionItemKind.Field, det, '0');
    }
  }

  /* Dentro do corpo de uma Entity/class, o CONSTRUTOR. Não é palavra
   * reservada nem builtin — é convenção de nome —, então não vinha de tabela
   * nenhuma do motor e o editor nunca o oferecia. Só entra se a Entity ainda
   * não tem um. */
  if (ent && !ent.membros.some((m) => m.nome === '__init__')) {
    poe('__init__', CompletionItemKind.Constructor,
        `construtor de ${ent.nome} — roda ao instanciar`, '0');
  }

  for (const e of idx.entidades) {
    poe(e.nome, CompletionItemKind.Class,
        `class ${e.nome}` + (e.bases.length ? `(${e.bases.join(', ')})` : ''), '1');
  }
  for (const [ligado, imp] of idx.imports) {
    poe(ligado, CompletionItemKind.Module,
        imp.membro ? `${imp.membro} de ${imp.mod}`
                   : (imp.mod === ligado ? 'modulo' : `modulo ${imp.mod} (as ${ligado})`), '2');
  }
  /* Os nomes que cada `*` do topo trouxe — o que o módulo EXPORTA, pela regra
   * do motor. Da última estrela pra primeira: com dois módulos trazendo o
   * mesmo nome, vale o de baixo. O que o arquivo já ligou (acima) ganha. */
  const dirDoc = pastaDoDoc(doc);
  const KIND_EXP = Object.assign({ function: CompletionItemKind.Function, value: CompletionItemKind.Variable,
                                   campo: CompletionItemKind.Variable, modulo: CompletionItemKind.Module }, KIND);
  for (let i = (idx.estrelas || []).length - 1; i >= 0; i--) {
    const est = idx.estrelas[i];
    for (const x of exportadosDe(est, dirDoc, dirDoc, new Set())) {
      const funcao = x.kind === 'action' || x.kind === 'function';
      poe(x.nome, KIND_EXP[x.kind] || CompletionItemKind.Variable,
          (funcao ? assinatura({ nome: x.nome, params: x.params }) : x.kind) + ` · de ${est.mod} (*)`, '2');
    }
  }
  /* builtins e palavras-chave: as tabelas do motor (`--metadata` publica a
   * `BUILTINS[]` da VM e a `KEYWORDS[]` do lexer). Não apareciam — `post`,
   * `len`, `action`, `if` nunca eram sugeridos sem receptor. */
  for (const b of META.builtins || []) {
    poe(b.nome, CompletionItemKind.Function, 'builtin · ' + assinatura({ nome: b.nome, params: b.params }), '3');
  }
  for (const k of META.keywords || []) poe(k, CompletionItemKind.Keyword, 'palavra-chave', '4');
  /* As exceções são valores da linguagem (`raise erro`, `X is Exception`) e
   * vêm da MESMA tabela que o `catch` consulta no motor. */
  for (const x of META.excecoes || []) {
    poe(x.nome, CompletionItemKind.Class, 'exceção · ' + (x.pai ? 'filha de ' + x.pai : 'raiz da árvore'), '3');
  }
  /* `true`, `false` e `Null` são LITERAIS no lexer (tokens BOOL e NULL), não
   * entradas de `KEYWORDS[]` — por isso não chegavam aqui pelo `--metadata` e
   * o editor nunca sugeria booleano nenhum. `__name__` é global que o
   * compilador liga, não palavra reservada, e some pelo mesmo motivo. */
  for (const [lit, det] of LITERAIS) poe(lit, CompletionItemKind.Constant, det, '3');
  poe('__name__', CompletionItemKind.Constant,
      'no arquivo principal vale "main"; num módulo importado, o nome do módulo', '3');
  /* `static` e `nonnull` NÃO são palavra reservada de propósito (valem por
   * posição, só colados na cabeça da funct), então não vêm em META.keywords —
   * e sem isto o editor jamais os ofereceria. */
  for (const m of MODIFICADORES) poe(m, CompletionItemKind.Keyword, 'modificador de funct', '4');
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
  const rotulos = ps.map(rotuloParam);
  /* `*args` absorve todo posicional excedente: do índice dele em diante o
   * ativo FICA nele, em vez de pular pro `**kwarg` */
  const iVar = ps.findIndex((x) => String(x.nome).startsWith('*') && !String(x.nome).startsWith('**'));
  const ativo = iVar >= 0 && ch.posicionais >= iVar
    ? iVar : Math.min(ch.posicionais, Math.max(0, ps.length - 1));
  return {
    signatures: [{
      label: `${ch.chamado}(${rotulos.join(', ')})`,
      parameters: rotulos.map((r) => ({ label: r })),
    }],
    activeSignature: 0,
    activeParameter: ativo,
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

/* Os modificadores COLADOS na cabeça da funct. Não são palavra reservada (o
 * lexer os entrega como IDENT, e `static = 1` continua valendo), então tudo
 * que o editor sabe sobre eles vem daqui — não de `META.keywords`. */
const MODIFICADORES = ['static', 'nonnull', 'NonNull'];

/* Os literais da linguagem. O lexer os entrega como token próprio (BOOL e
 * NULL), não como palavra reservada, então não estão na `KEYWORDS[]` que o
 * `--metadata` publica — e sem esta lista o editor não sugeria nem `true`. */
const LITERAIS = [
  ['true',  'literal booleano'],
  ['false', 'literal booleano'],
  ['Null',  'literal de ausência de valor'],
];

/* O token está na CABEÇA de uma declaração de funct? Anda pra frente na mesma
 * linha atravessando os outros modificadores; se chegar em `funct` (ou nas
 * grafias antigas), sim. É a mesma forma que o parser aceita, e é o que separa
 * `static funct m()` de uma variável chamada `static`. */
function ehCabecaDeFunct(doc, tok) {
  const toks = tokensDe(doc).filter((t) => t.l0 === tok.l0 && t.c0 >= tok.c0 && t.n > 0);
  const passa = ['public', 'private', 'async', 'str', 'int', 'flo', 'bool'];
  for (const t of toks) {
    if (t === tok) continue;
    if (t.t === 'KW' && ['funct', 'action', 'reaction'].includes(t.v)) return true;
    if (MODIFICADORES.includes(t.v) || (t.t === 'KW' && passa.includes(t.v))) continue;
    return false;
  }
  return false;
}

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

/* O hover de um nome que veio de import — nomeado (`from m import f`) ou
 * trazido por um `*`. É o mesmo texto nos dois: pro motor é a mesma ligação. */
function hoverDoImportado(doc, alvo, nome, linha) {
  /* nome de ARQUIVO: a declaração dele, lida do item exportado — a mesma
   * regra que decide se ele existe */
  const def = alvo.tipo === 'membro_arquivo'
    ? (alvo.def || (alvo.arquivo ? membrosDeArquivo(alvo.arquivo, pastaDoDoc(doc)).find((x) => x.nome === alvo.membro) : null))
    : null;
  /* reexportado de módulo do motor (o arquivo fez `from json import parse`) */
  const nat = alvo.tipo === 'membro_modulo' ? alvo : (def && def.mod ? { mod: def.mod, membro: def.membro } : null);
  if (nat) {
    const m = (META.modulos[nat.mod] || []).find((x) => x.nome === nat.membro);
    if (m) {
      const cab = nat.mod + '.' + (m.kind === 'value'
        ? m.nome + (retornoVisivel(m.retorna) ? ' -> ' + m.retorna : '')
        : assinatura(m));
      const prosa = resumoDe(nat.mod, nat.membro);
      return md('```ps\n' + cab + '\n```' + (prosa ? '\n\n' + prosa : ''));
    }
  }
  if (def && def.arquivo) {
    const onde = '`' + path.basename(def.arquivo) + '` · linha ' + (def.linha + 1);
    if (def.kind === 'action') {
      const cab = (def.estatica ? 'static ' : '') + (def.nonnull ? 'nonnull ' : '')
                + (def.tipo ? def.tipo + ' ' : '') + (def.async ? 'async ' : '') + 'funct '
                + assinatura({ nome: def.declarado || def.nome, params: def.params });
      return md('```ps\n' + cab + '\n```\n\nfunct · de ' + onde);
    }
    if (def.kind === 'class') return md('```ps\nclass ' + (def.declarado || def.nome) + '\n```\n\nclass · de ' + onde);
    return md('```ps\n' + (def.tipo ? def.tipo + ' ' : '') + (def.declarado || def.nome) + '\n```\n\n'
              + def.kind + ' · de ' + onde);
  }
  const n = membrosDe(doc, { tipo: 'import', alvo }, linha).length;
  const de = alvo.arquivo ? `\n\n_de ${alvo.arquivo}_` : '';
  return md('```ps\nimport ' + (alvo.mod || alvo.arquivo || nome) + '\n```\n\n' + n + ' membros' + de);
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
  /* `static`/`nonnull` chegam como IDENT (não são reservadas), então não
   * passam pela porta de palavra-chave abaixo: sem esta exceção, o hover em
   * cima do modificador colado devolvia nada. */
  if (tok && MODIFICADORES.includes(tok.v) && !aposPonto && ehCabecaDeFunct(doc, tok)) {
    return hoverDeKeyword(doc, tok);
  }
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

  /* Membro de um receptor que a cadeia de nomes NÃO lê: literal (`"x".encode`,
   * `b"x".decode`) ou expressão (`lines[1].decode`). O `aposPonto` diz que há
   * receptor; o literal responde pelo tipo dele, o resto vira candidatos. */
  if (!partes.length && aposPonto) {
    const lit = receptorLiteral(doc, { line: p.position.line, character: tok.c0 });
    if (lit) {
      const m = membrosDe(doc, { tipo: 'tipo_motor', nome: lit }, p.position.line)
                  .find((x) => x.nome === nome);
      if (!m) return null;
      const prosa = m.escopo ? resumoDe(m.escopo, m.nome) : '';
      return md('```ps\n' + lit + '.' + assinatura(m) + '\n```' + (prosa ? '\n\n' + prosa : ''));
    }
    return hoverCandidatos(nome);
  }

  if (partes.length) {                       /* `alvo.membro` */
    const alvo = alvoDaCadeia(doc, partes, p.position.line);
    const membros = membrosDe(doc, alvo, p.position.line);
    const m = membros.find((x) => x.nome === nome);
    /* receptor conhecido mas de tipo desconhecido (`head = lines[0]`): os
     * candidatos por nome; membro inexistente numa Entity/módulo: nada */
    if (!m) return alvo && alvo.tipo === 'universal' ? hoverCandidatos(nome) : null;
    const dono = partes[partes.length - 1];
    const prosa = m.escopo ? resumoDe(m.escopo, m.nome) : '';
    const herd = m.de && m.de !== dono ? `\n\nherdado de \`${m.de}\`` : '';
    return md('```ps\n' + (m.privado ? 'private ' : '') + (m.estatica ? 'static ' : '')
              + (m.nonnull ? 'nonnull ' : '') + dono + '.' + assinatura(m)
              + '\n```' + herd + (prosa ? '\n\n' + prosa : ''));
  }

  /* O `*` que liga o nome NESTA linha vem antes do que o arquivo declara mais
   * abaixo: o motor liga na ordem do arquivo, e nome de funct não é hoisted
   * por cima da estrela (medido no ./pool). Quando não é a estrela que vale
   * ali, `alvoDaEstrela` devolve null e o hover segue como sempre. */
  const est = alvoDaEstrela(doc, nome, p.position.line);
  if (est) return hoverDoImportado(doc, est, nome, p.position.line);

  const idx = indiceDe(doc);
  const e = idx.entidades.find((x) => x.nome === nome);
  if (e) {
    return md('```ps\nclass ' + e.nome + (e.bases.length ? '(' + e.bases.join(', ') + ')' : '')
              + '\n```\n\n' + e.membros.length + ' membros');
  }
  /* nome solto dentro da classe = membro `static` dela (ou de um pai), a não
   * ser que um parâmetro/variável do método o sombreie */
  {
    const ent = A.entidadeEm(idx, p.position.line);
    const st = ent && !ligacaoLocal(idx, nome, p.position.line) ? estaticoSolto(doc, ent, nome) : null;
    if (st) {
      const herd = st.de !== ent.nome ? `\n\nherdado de \`${st.de}\`` : '';
      const cab = st.kind === 'action'
        ? 'static ' + (st.nonnull ? 'nonnull ' : '') + (st.retorna ? st.retorna + ' ' : '') + 'funct ' + assinatura(st)
        : 'static ' + (st.tipo ? st.tipo + ' ' : '') + st.nome;
      return md('```ps\n' + cab + '\n```\n\n' + (st.kind === 'action' ? 'método' : 'campo')
                + ' static de `' + st.de + '` · declarad' + (st.kind === 'action' ? 'o' : 'o')
                + ' na linha ' + (st.linha + 1) + ' — pelo nome solto ou `' + st.de + '.' + st.nome + '`' + herd);
    }
  }
  for (const b of A.visiveisEm(idx, p.position.line)) {
    if (b.nome !== nome) continue;
    const no = b.no || {};
    const onde = 'linha ' + (b.linha + 1);
    if (b.kind === 'action') {
      /* a ordem dos modificadores é livre na linguagem; aqui sai a canônica
       * da doc (6.4): visibilidade, static/nonnull, tipo, async, funct */
      const cab = (b.estatica ? 'static ' : '') + (b.nonnull ? 'nonnull ' : '')
                + (b.tipo ? b.tipo + ' ' : '') + (b.async ? 'async ' : '') + 'funct '
                + assinatura({ nome: b.nome, params: b.params });
      const dec = decoradorDe(idx, b.nome, b.linha);
      return md('```ps\n' + (dec ? dec + '\n' : '') + cab + '\n```\n\nfunct · declarada na ' + onde);
    }
    if (b.kind === 'parametro') {
      const esc = idx.escopos.find((s) => s.liga.includes(b));
      const dono = esc ? (esc.tipo === 'metodo' ? esc.entidade + '.' + esc.nome : esc.nome) : '';
      /* como no fonte: o tipo declarado antes do nome (`byte raw`) */
      return md('```ps\n' + (b.tipo ? b.tipo + ' ' : '') + b.nome + '\n```\n\nparâmetro'
                + (dono ? ' de `' + dono + '`' : ''));
    }
    if (b.kind === 'model') {
      const campos = (no.lista || []).filter((f) => f && f.texto)
        .map((f) => f.texto + ': ' + (f.texto2 || '') + paramsDeCampoTxt(f));
      return md('```ps\nmodel ' + b.nome + '() { ' + campos.join(', ') + ' }\n```\n\nmodel · declarado na ' + onde);
    }
    /* variável: o tipo é o declarado (`str x`) ou o construído (`x = Jinker(...)`) */
    const t = tipoDoNome(doc, nome, p.position.line);
    const tipo = b.tipo || (t && t.tipo !== 'import' && t.nome) || '';
    return md('```ps\n' + (tipo ? tipo + ' ' : '') + b.nome + '\n```\n\n' + b.kind + ' · declarada na ' + onde);
  }
  /* Exceção do motor: a cadeia de pais e os filhos saem de `META.excecoes`
   * (a tabela do `catch`). Não há página por nome em docs/exceptions/<nome>/;
   * a árvore inteira está em exceptions.md. */
  const exc = (META.excecoes || []).find((x) => x.nome === nome);
  if (exc) {
    const pais = [];
    for (let pai = exc.pai; pai; ) {
      pais.push(pai);
      const acima = (META.excecoes || []).find((x) => x.nome === pai);
      pai = acima ? acima.pai : null;
    }
    const filhos = (META.excecoes || []).filter((x) => x.pai === nome).map((x) => x.nome);
    const linha = 'exceção · ' + (pais.length ? 'filha de ' + pais.join(' → ') : 'raiz da árvore')
                + (filhos.length ? ' · filhos: ' + filhos.join(', ') : '');
    const pagina = path.join(raizDoc(), 'exceptions', 'exceptions.md');
    return md('```ps\n' + nome + '\n```\n\n' + linha + '\n\n`catch (' + nome + ' e)` pega ' + nome
              + (filhos.length ? ' e os descendentes' : '') + ' · [árvore de exceções](' + pagina + ')');
  }
  const alvo = alvoDoImport(doc, nome, p.position.line);
  if (alvo) return hoverDoImportado(doc, alvo, nome, p.position.line);
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
        const uri = dono && dono.arquivo ? uriDe(dono.arquivo) : doc.uri;
        return { uri, range: { start: { line: m.linha, character: m.coluna },
                               end: { line: m.linha, character: m.coluna + nome.length } } };
      }
    }
    if (alvo && alvo.tipo === 'import' && alvo.alvo.arquivo) {
      for (const m of membrosDeArquivo(alvo.alvo.arquivo, pastaDoDoc(doc))) {
        /* reexportado: a declaração está no arquivo de ORIGEM (`m.arquivo`);
         * membro de módulo do motor não tem linha pra onde ir */
        if (m.nome !== nome || !m.arquivo) continue;
        return { uri: uriDe(m.arquivo),
                 range: { start: { line: m.linha, character: m.coluna },
                          end: { line: m.linha, character: m.coluna + nome.length } } };
      }
    }
    return null;
  }

  /* a estrela que liga o nome NESTA linha responde antes do que o arquivo
   * declara mais abaixo — a ordem do motor, a mesma do hover */
  const est = alvoDaEstrela(doc, nome, p.position.line);
  if (est) return definicaoDoImportado(doc, est, nome);

  const e = idx.entidades.find((x) => x.nome === nome);
  if (e) return { uri: doc.uri,
                  range: { start: { line: e.linha, character: e.coluna },
                           end: { line: e.linha, character: e.coluna + nome.length } } };
  /* nome solto dentro da classe = membro `static` dela (ou de um pai), se
   * nenhum local do método o sombreia — a mesma precedência do motor */
  {
    const ent = A.entidadeEm(idx, p.position.line);
    const st = ent && !ligacaoLocal(idx, nome, p.position.line) ? estaticoSolto(doc, ent, nome) : null;
    if (st) {
      const dono = achaEntidade(doc, st.de);
      const uri = dono && dono.arquivo ? uriDe(dono.arquivo) : doc.uri;
      return { uri, range: { start: { line: st.linha, character: st.coluna },
                             end: { line: st.linha, character: st.coluna + nome.length } } };
    }
  }
  for (const b of A.visiveisEm(idx, p.position.line)) {
    if (b.nome !== nome) continue;
    return { uri: doc.uri,
             range: { start: { line: b.linha, character: 0 },
                      end: { line: b.linha, character: nome.length } } };
  }
  const alvo = alvoDoImport(doc, nome, p.position.line);
  return definicaoDoImportado(doc, alvo, nome);
});

/* Onde um nome importado foi DECLARADO — nomeado (`from m import f`) ou
 * trazido por um `*`. Num reexporte o arquivo da declaração não é o `m`, e é
 * nele que o editor abre. */
function definicaoDoImportado(doc, alvo, nome) {
  if (!alvo) return null;
  if (alvo.tipo === 'membro_arquivo') {
    const def = alvo.def
      || (alvo.arquivo ? membrosDeArquivo(alvo.arquivo, pastaDoDoc(doc)).find((x) => x.nome === alvo.membro) : null);
    if (def && def.arquivo) {
      return { uri: uriDe(def.arquivo),
               range: { start: { line: def.linha, character: def.coluna },
                        end: { line: def.linha, character: def.coluna + (def.declarado || nome).length } } };
    }
  }
  if (alvo.arquivo) {
    return { uri: uriDe(alvo.arquivo),
             range: { start: { line: 0, character: 0 }, end: { line: 0, character: 0 } } };
  }
  return null;
}

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
        detail: (m.privado ? 'private ' : '') + (m.estatica ? 'static ' : '')
                + (m.nonnull ? 'nonnull ' : '') + assinatura(m),
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
