const vscode = require('vscode');
const fs = require('fs');
const path = require('path');
const os = require('os');

// ─────────────────────────────────────────────────────────────────────────
// metadata da stdlib
// ─────────────────────────────────────────────────────────────────────────
let META = { modules: {}, classes: {} };

function carregaMetadata(ctx) {
  try {
    const p = path.join(ctx.extensionPath, 'bridge', 'metadata.json');
    META = JSON.parse(fs.readFileSync(p, 'utf8'));
  } catch (e) {
    console.error('[poolscript] metadata.json não carregou:', e.message);
    META = { modules: {}, classes: {} };
  }
}

// mapeia um escalar declarado pro "tipo-classe" que carrega os métodos.
// str -> PoolStr (que o metadata já introspecta com os métodos de string).
const ESCALAR_CLASSE = { str: 'PoolStr' };

// palavras-chave da linguagem (espelha o lexer) — pro completion de topo e hover.
const KEYWORDS = [
  'if', 'else', 'elif', 'while', 'for', 'each', 'in', 'is', 'and', 'or', 'not',
  'action', 'reaction', 'return', 'continue', 'break', 'model', 'enum', 'async',
  'await', 'try', 'catch', 'as', 'with', 'using', 'import', 'from', 'str', 'int',
  'flo', 'bool', 'list', 'dict', 'json', 'tup', 'class', 'Class', 'Entity', 'self',
  'private', 'public', 'match', 'case', 'yield', 'raise', 'finally', 'count',
  'global', 'true', 'false', 'Null', 'post', 'input',
];

// ─────────────────────────────────────────────────────────────────────────
// parsing leve de PoolScript (regex) — símbolos locais e de libs instaladas
// ─────────────────────────────────────────────────────────────────────────
// Não é o parser real (esse vive na linguagem); é um extrator rápido e
// suficiente pra completion: nomes + parâmetros + membros de classe.

function parseParams(assinatura) {
  // "a, b, c=1, d=\"x\"" -> [{name:'a',opt:false}, {name:'c',opt:true}, ...]
  const out = [];
  let depth = 0, atual = '', partes = [];
  for (const ch of assinatura) {
    if ('([{'.includes(ch)) depth++;
    else if (')]}'.includes(ch)) depth--;
    if (ch === ',' && depth === 0) { partes.push(atual); atual = ''; }
    else atual += ch;
  }
  if (atual.trim()) partes.push(atual);
  for (let p of partes) {
    p = p.trim();
    if (!p || p === 'self') continue;
    const m = p.match(/^([A-Za-z_]\w*)/);
    if (m) out.push({ name: m[1], opt: p.includes('=') });
  }
  return out;
}

function parsePoolSource(text) {
  const sym = { functions: [], classes: {}, enums: [], vars: [], aliases: {} };
  const linhas = text.split('\n');
  let classeAtual = null, classeDepth = 0, depth = 0;

  for (let i = 0; i < linhas.length; i++) {
    const linha = linhas[i];
    const semComentario = linha.replace(/(#|\/\/).*$/, '');

    // import MOD as ALIAS  → aliases[ALIAS] = MOD  (pra `nv.` resolver dotenv)
    const impAs = semComentario.match(/^\s*import\s+([A-Za-z_]\w*)\s+as\s+([A-Za-z_]\w*)/);
    if (impAs) sym.aliases[impAs[2]] = impAs[1];

    // action/reaction NOME(params)
    const act = semComentario.match(/\b(?:async\s+)?(?:int\s+|str\s+|flo\s+|bool\s+)?(?:action|reaction)\s+([A-Za-z_]\w*)\s*\(([^)]*)\)/);
    if (act) {
      const alvo = classeAtual
        ? (sym.classes[classeAtual].members)
        : sym.functions;
      alvo.push({ name: act[1], kind: classeAtual ? 'method' : 'function', params: parseParams(act[2]) });
    }

    // Entity/class/Class NOME  (abre um corpo de classe)
    const cls = semComentario.match(/\b(?:private\s+|public\s+)?(?:Entity|class|Class)\s+([A-Za-z_]\w*)/);
    if (cls) {
      classeAtual = cls[1];
      classeDepth = depth;
      if (!sym.classes[classeAtual]) sym.classes[classeAtual] = { members: [] };
    }

    // enum NOME
    const en = semComentario.match(/\benum\s+([A-Za-z_]\w*)/);
    if (en) sym.enums.push(en[1]);

    // atribuição de topo/local: [TIPO] nome = RHS  (guarda o RHS bruto p/ inferência)
    const asg = semComentario.match(/^\s*(?:(str|int|flo|bool|list|json|dict)\s+)?([A-Za-z_]\w*)\s*=\s*(.+)$/);
    if (asg && !semComentario.match(/[=!<>]=/)) {
      sym.vars.push({ name: asg[2], declaredType: asg[1] || null, rhs: asg[3].trim(), line: i });
    }

    // conta chaves pra saber quando a classe fecha
    for (const ch of semComentario) {
      if (ch === '{') depth++;
      else if (ch === '}') { depth--; if (classeAtual && depth <= classeDepth) classeAtual = null; }
    }
  }
  return sym;
}

// símbolos das libs instaladas (parseadas uma vez, cacheadas)
let LIBS_INSTALADAS = {};   // nomeLib -> sym

function raizesDeLib() {
  const raizes = [];
  const home = process.env.POOLSCRIPT_HOME;
  if (home) raizes.push(path.join(home, 'libs'));
  raizes.push(path.join(os.homedir(), '.poolscript', 'libs'));
  return raizes;
}

function indexaLibsInstaladas() {
  LIBS_INSTALADAS = {};
  for (const raiz of raizesDeLib()) {
    let entradas;
    try { entradas = fs.readdirSync(raiz); } catch (e) { continue; }
    for (const nome of entradas) {
      const full = path.join(raiz, nome);
      try {
        const st = fs.statSync(full);
        let arquivo = null;
        if (st.isFile() && /\.(ps|psl|p)$/.test(nome)) arquivo = full;
        else if (st.isDirectory()) {
          for (const cand of [nome + '.ps', 'main.ps', 'index.ps', '__init__.ps']) {
            if (fs.existsSync(path.join(full, cand))) { arquivo = path.join(full, cand); break; }
          }
        }
        if (!arquivo) continue;
        const libNome = nome.replace(/\.(ps|psl|p)$/, '');
        LIBS_INSTALADAS[libNome] = parsePoolSource(fs.readFileSync(arquivo, 'utf8'));
      } catch (e) { /* ignora lib ilegível */ }
    }
  }
}

// ─────────────────────────────────────────────────────────────────────────
// resolução de tipo (o coração do "type-aware")
// ─────────────────────────────────────────────────────────────────────────

// Quebra um receptor ("psodbc.connect(...)", "conn.cursor()", "x") numa cadeia
// de segmentos [{name, call}], ignorando o conteúdo dos parênteses.
function parseChain(expr) {
  const segs = [];
  let i = 0, nome = '', call = false;
  while (i < expr.length) {
    const ch = expr[i];
    if (ch === '(') {
      // pula o grupo balanceado
      let d = 1; i++;
      while (i < expr.length && d > 0) { if (expr[i] === '(') d++; else if (expr[i] === ')') d--; i++; }
      call = true;
      continue;
    }
    if (ch === '.') {
      if (nome) segs.push({ name: nome, call });
      nome = ''; call = false; i++;
      continue;
    }
    if (/[A-Za-z0-9_]/.test(ch)) { nome += ch; i++; continue; }
    // caractere inesperado (índice, operador) -> cadeia não resolvível
    return null;
  }
  if (nome) segs.push({ name: nome, call });
  return segs.length ? segs : null;
}

// Um "tipo" é: {module}, {cls} (nome de classe stdlib OU local), {scalar}, ou null.
function membrosDoTipo(tipo, doc) {
  if (!tipo) return null;
  if (tipo.module) {
    const m = META.modules[tipo.module];
    if (m) return m.members.map(x => ({ ...x, from: tipo.module }));
    // lib instalada?
    const lib = LIBS_INSTALADAS[tipo.module];
    if (lib) return lib.functions.map(f => ({ ...f, from: tipo.module }));
    return null;
  }
  if (tipo.scalar) {
    const c = ESCALAR_CLASSE[tipo.scalar];
    if (c && META.classes[c]) return META.classes[c].members;
    return null;
  }
  if (tipo.cls) {
    if (META.classes[tipo.cls]) return META.classes[tipo.cls].members;
    // classe local (Entity do usuário ou de lib instalada / arquivo atual)
    const local = simbolosLocais(doc);
    if (local.classes[tipo.cls]) return local.classes[tipo.cls].members;
    for (const lib of Object.values(LIBS_INSTALADAS))
      if (lib.classes[tipo.cls]) return lib.classes[tipo.cls].members;
    return null;
  }
  return null;
}

// dado um membro (do metadata ou local) e o tipo em que ele foi achado,
// devolve o tipo do RESULTADO (pra encadear a.b.c).
function tipoDoMembro(membro) {
  if (!membro) return null;
  const r = membro.returns;
  if (!r) return null;
  if (r === 'str') return { scalar: 'str' };
  if (['int', 'flo', 'bool', 'list', 'dict', 'tup', 'bytes'].includes(r)) return { scalar: r };
  return { cls: r };  // nome de classe -> encadeia
}

// resolve uma cadeia de segmentos pro tipo final.
function resolveChain(segs, doc, ateLinha) {
  if (!segs || !segs.length) return null;
  let tipo = tipoDoSegmentoBase(segs[0], doc, ateLinha);
  for (let i = 1; i < segs.length; i++) {
    const membros = membrosDoTipo(tipo, doc);
    if (!membros) return null;
    const m = membros.find(x => x.name === segs[i].name);
    if (!m) return null;
    tipo = tipoDoMembro(m);
  }
  return tipo;
}

function tipoDoSegmentoBase(seg, doc, ateLinha) {
  let nome = seg.name;
  // alias de import (`import dotenv as nv` -> nv resolve dotenv)
  const alias = simbolosLocais(doc).aliases[nome];
  if (alias) nome = alias;
  // módulo importado?
  if (!seg.call && (META.modules[nome] || LIBS_INSTALADAS[nome])) return { module: nome };
  // construtor de classe: NomeClasse(...) -> instância
  if (seg.call) {
    if (META.classes[nome]) return { cls: nome };
    const local = simbolosLocais(doc);
    if (local.classes[nome]) return { cls: nome };
    // função local que retorna algo? (sem type info -> desconhecido)
    return null;
  }
  // variável: acha a última atribuição antes da linha e infere o RHS
  return tipoDaVar(nome, doc, ateLinha);
}

function tipoDaVar(nome, doc, ateLinha) {
  const local = simbolosLocais(doc);
  let achada = null;
  for (const v of local.vars) {
    if (v.name === nome && (ateLinha === undefined || v.line < ateLinha)) {
      if (!achada || v.line > achada.line) achada = v;
    }
  }
  if (!achada) return null;
  if (achada.declaredType) {
    const dt = achada.declaredType;
    if (dt === 'str') return { scalar: 'str' };
    if (['int', 'flo', 'bool', 'list', 'dict'].includes(dt)) return { scalar: dt };
  }
  // infere do RHS: literal ou cadeia
  const rhs = achada.rhs;
  if (/^["'f]/.test(rhs)) return { scalar: 'str' };
  if (/^\[/.test(rhs)) return { scalar: 'list' };
  if (/^\{/.test(rhs)) return { scalar: 'dict' };
  // caso especial: connect(driver="mongo"/...) -> MongoConnection
  if (/\bconnect\s*\(/.test(rhs) && /driver\s*=\s*["']mongo/.test(rhs)) return { cls: 'MongoConnection' };
  const segs = parseChain(rhs);
  if (segs) return resolveChain(segs, doc, achada.line);
  return null;
}

// cache dos símbolos locais por versão do documento
let _cacheSym = { key: null, sym: null };
function simbolosLocais(doc) {
  const key = doc.uri.toString() + '@' + doc.version;
  if (_cacheSym.key === key) return _cacheSym.sym;
  const sym = parsePoolSource(doc.getText());
  _cacheSym = { key, sym };
  return sym;
}

// ─────────────────────────────────────────────────────────────────────────
// providers
// ─────────────────────────────────────────────────────────────────────────

function kindDe(m) {
  if (m.kind === 'class') return vscode.CompletionItemKind.Class;
  if (m.kind === 'property') return vscode.CompletionItemKind.Property;
  if (m.kind === 'value') return vscode.CompletionItemKind.Variable;
  return vscode.CompletionItemKind.Method;
}

function itemDeMembro(m) {
  const it = new vscode.CompletionItem(m.name, kindDe(m));
  if (m.sig) it.detail = m.sig;
  else if (m.params) it.detail = `${m.name}(${m.params.map(p => p.name + (p.opt ? '?' : '')).join(', ')})`;
  if (m.returns) it.detail = (it.detail || m.name) + ` -> ${m.returns}`;
  // doc rico no popup ao lado da sugestão (code block + descrição)
  if (m.doc) {
    const md = new vscode.MarkdownString();
    md.appendCodeblock((it.detail || m.name), 'poolscript');
    md.appendMarkdown('\n' + m.doc);
    it.documentation = md;
  }
  // método/função com () — insere os parênteses e põe o cursor dentro
  if (m.kind === 'method' || m.kind === 'function') {
    it.insertText = new vscode.SnippetString(`${m.name}($0)`);
  }
  return it;
}

// item de topo com doc rico (keyword/builtin/tipo) a partir do spec do metadata
function itemComDoc(nome, kind, spec, etiqueta) {
  const it = new vscode.CompletionItem(nome, kind);
  it.detail = spec.sig || nome;
  const md = new vscode.MarkdownString();
  if (spec.sig) md.appendCodeblock(spec.sig, 'poolscript');
  if (etiqueta) md.appendMarkdown(`_${etiqueta}_\n`);
  if (spec.resumo) md.appendMarkdown('\n' + spec.resumo);
  if (spec.ex && spec.ex.length) {
    md.appendMarkdown('\n\n**Exemplo**\n');
    md.appendCodeblock(Array.isArray(spec.ex[0]) ? spec.ex[0][0] : spec.ex[0], 'poolscript');
  }
  it.documentation = md;
  return it;
}

// texto do receptor imediatamente antes de um '.' na posição
function receptorAntesDoPonto(linhaAteCursor) {
  // pega o maior sufixo que parece uma cadeia terminando em '.'
  const m = linhaAteCursor.match(/([A-Za-z_]\w*(?:\s*\([^()]*\))?(?:\.[A-Za-z_]\w*(?:\s*\([^()]*\))?)*)\.\s*$/);
  return m ? m[1] : null;
}

// acha a chamada que ENVOLVE o cursor: nome da função + se já há '=' no arg atual
function chamadaEnvolvente(linhaAteCursor) {
  let depth = 0, i = linhaAteCursor.length - 1;
  let argAtual = '';
  for (; i >= 0; i--) {
    const ch = linhaAteCursor[i];
    if (ch === ')') depth++;
    else if (ch === '(') { if (depth === 0) break; depth--; }
    else if (depth === 0) argAtual = ch + argAtual;
  }
  if (i < 0) return null;
  // nome (cadeia) antes do '('
  const antes = linhaAteCursor.slice(0, i);
  const mm = antes.match(/([A-Za-z_]\w*(?:\.[A-Za-z_]\w*)*)\s*$/);
  if (!mm) return null;
  // separa argumentos por vírgula no nível 0 pra pegar o arg atual
  const ultimoArg = argAtual.split(',').pop();
  return { chain: mm[1], argJaTemIgual: /=/.test(ultimoArg), argParcial: ultimoArg.trim() };
}

// resolve uma cadeia "a.b" pra o membro-função (pra pegar params do arg nomeado)
function funcaoDaChain(chainStr, doc) {
  const partes = chainStr.split('.');
  if (partes.length === 1) {
    // função de módulo? não (módulo precisa de ponto). função local?
    const local = simbolosLocais(doc);
    const f = local.functions.find(x => x.name === partes[0]);
    if (f) return f;
    for (const lib of Object.values(LIBS_INSTALADAS)) {
      const lf = lib.functions.find(x => x.name === partes[0]);
      if (lf) return lf;
    }
    return null;
  }
  // módulo.func ou var.metodo
  const baseNome = simbolosLocais(doc).aliases[partes[0]] || partes[0];
  if (META.modules[baseNome]) {
    return (META.modules[baseNome].members || []).find(x => x.name === partes[1]);
  }
  if (LIBS_INSTALADAS[baseNome]) {
    return (LIBS_INSTALADAS[baseNome].functions || []).find(x => x.name === partes[1]);
  }
  // var.metodo: resolve o tipo da var e acha o método
  const tipo = tipoDaVar(baseNome, doc);
  const membros = membrosDoTipo(tipo, doc);
  return membros ? membros.find(x => x.name === partes[partes.length - 1]) : null;
}

const completionProvider = {
  provideCompletionItems(doc, pos) {
    const linhaAteCursor = doc.lineAt(pos.line).text.slice(0, pos.character);

    // ── 1. member completion: depois de um '.' ──
    const recv = receptorAntesDoPonto(linhaAteCursor);
    if (recv) {
      const segs = parseChain(recv);
      const tipo = resolveChain(segs, doc, pos.line);
      const membros = membrosDoTipo(tipo, doc);
      if (!membros) return [];                 // tipo desconhecido -> NADA (regra de ouro)
      return membros.map(itemDeMembro);
    }

    // ── 2. argumento nomeado: dentro de uma chamada, sugere `param=` ──
    const call = chamadaEnvolvente(linhaAteCursor);
    if (call && !call.argJaTemIgual) {
      const fn = funcaoDaChain(call.chain, doc);
      if (fn && fn.params && fn.params.length) {
        const itens = fn.params.map(p => {
          const it = new vscode.CompletionItem(p.name + '=', vscode.CompletionItemKind.Field);
          it.detail = `argumento de ${call.chain}()`;
          it.insertText = new vscode.SnippetString(`${p.name}=$0`);
          it.sortText = '0' + p.name;          // params antes do resto
          return it;
        });
        // ainda oferece nomes de topo junto (o usuário pode querer uma var)
        return itens.concat(completionDeTopo(doc));
      }
    }

    // ── 3. topo: módulos, símbolos locais, keywords ──
    return completionDeTopo(doc);
  },
};

function completionDeTopo(doc) {
  const itens = [];
  // módulos da stdlib
  for (const nome of Object.keys(META.modules)) {
    const it = new vscode.CompletionItem(nome, vscode.CompletionItemKind.Module);
    it.detail = 'módulo';
    itens.push(it);
  }
  // libs instaladas
  for (const nome of Object.keys(LIBS_INSTALADAS)) {
    const it = new vscode.CompletionItem(nome, vscode.CompletionItemKind.Module);
    it.detail = 'lib instalada';
    itens.push(it);
  }
  // símbolos locais
  const local = simbolosLocais(doc);
  for (const f of local.functions) {
    const it = new vscode.CompletionItem(f.name, vscode.CompletionItemKind.Function);
    it.detail = `${f.name}(${f.params.map(p => p.name).join(', ')})`;
    it.insertText = new vscode.SnippetString(`${f.name}($0)`);
    itens.push(it);
  }
  for (const c of Object.keys(local.classes))
    itens.push(new vscode.CompletionItem(c, vscode.CompletionItemKind.Class));
  for (const e of local.enums)
    itens.push(new vscode.CompletionItem(e, vscode.CompletionItemKind.Enum));
  const vistos = new Set();
  for (const v of local.vars) {
    if (vistos.has(v.name)) continue;
    vistos.add(v.name);
    itens.push(new vscode.CompletionItem(v.name, vscode.CompletionItemKind.Variable));
  }
  // builtins globais (post, len, str...) com doc rico
  const bts = (META.builtins || {});
  for (const nome of Object.keys(bts))
    itens.push(itemComDoc(nome, vscode.CompletionItemKind.Function, bts[nome], 'builtin'));
  // keywords — com doc rico quando o metadata tem
  const kws = (META.keywords || {});
  for (const k of KEYWORDS) {
    if (kws[k]) itens.push(itemComDoc(k, vscode.CompletionItemKind.Keyword, kws[k], 'palavra-chave'));
    else itens.push(new vscode.CompletionItem(k, vscode.CompletionItemKind.Keyword));
  }
  return itens;
}

// Hover bonito: assinatura em BLOCO DE CÓDIGO (colorido pelo highlight, não
// texto cinza) + a etiqueta do tipo + descrição + exemplo. É o mesmo formato
// que o Pylance usa: título code, corpo markdown.
function hoverRico(sig, etiqueta, descricao, ex) {
  const md = new vscode.MarkdownString();
  if (sig) md.appendCodeblock(sig, 'poolscript');
  if (etiqueta) md.appendMarkdown(`_${etiqueta}_\n`);
  if (descricao) md.appendMarkdown('\n' + descricao + '\n');
  if (ex && ex.length) {
    md.appendMarkdown('\n**Exemplo**\n');
    for (const par of ex) {
      const codigo = Array.isArray(par) ? par[0] : par;
      const saida = Array.isArray(par) ? par[1] : null;
      md.appendCodeblock(codigo, 'poolscript');
      if (saida) md.appendCodeblock(saida, 'text');
    }
  }
  return new vscode.Hover(md);
}

const hoverProvider = {
  provideHover(doc, pos) {
    const range = doc.getWordRangeAtPosition(pos, /[A-Za-z_]\w*/);
    if (!range) return null;
    const palavra = doc.getText(range);
    const linhaAntes = doc.lineAt(pos.line).text.slice(0, range.start.character);

    // 1. membro depois de '.': resolve o tipo do receptor
    if (linhaAntes.trimEnd().endsWith('.')) {
      const base = receptorAntesDoPonto(linhaAntes.trimEnd());
      if (base) {
        const tipo = resolveChain(parseChain(base), doc, pos.line);
        const membros = membrosDoTipo(tipo, doc);
        const m = membros && membros.find(x => x.name === palavra);
        if (m) {
          const dono = (tipo && (tipo.module || tipo.cls || tipo.scalar)) || '';
          const sig = (dono ? dono + '.' : '') + (m.sig || m.name) + (m.returns ? ' -> ' + m.returns : '');
          return hoverRico(sig, m.kind === 'property' ? 'propriedade' : m.kind, m.doc, null);
        }
      }
    }
    // 2. keyword / tipo / builtin — docs ricas (sig + resumo + exemplo)
    if (META.keywords && META.keywords[palavra]) {
      const s = META.keywords[palavra];
      return hoverRico(s.sig || palavra, 'palavra-chave', s.resumo, s.ex);
    }
    if (META.types && META.types[palavra]) {
      const s = META.types[palavra];
      return hoverRico(s.sig || palavra, 'tipo', s.resumo, s.ex);
    }
    if (META.builtins && META.builtins[palavra]) {
      const s = META.builtins[palavra];
      return hoverRico(s.sig || palavra, 'builtin', s.resumo, s.ex);
    }
    // 3. módulo (resolve alias) / lib instalada
    const modReal = simbolosLocais(doc).aliases[palavra] || palavra;
    if (META.modules[modReal]) {
      const cab = palavra + (modReal !== palavra ? `  (import ${modReal})` : '');
      return hoverRico(cab, 'módulo da stdlib', null, null);
    }
    if (LIBS_INSTALADAS[palavra]) return hoverRico(palavra, 'lib instalada', null, null);
    // 4. símbolo local do arquivo
    const local = simbolosLocais(doc);
    const f = local.functions.find(x => x.name === palavra);
    if (f) return hoverRico(`action ${f.name}(${f.params.map(p => p.name + (p.opt ? '?' : '')).join(', ')})`, 'ação local', null, null);
    if (local.classes[palavra]) return hoverRico(`class ${palavra}`, 'classe local', null, null);
    if (local.enums.includes(palavra)) return hoverRico(`enum ${palavra}`, 'enum local', null, null);
    return null;
  },
};

// ─────────────────────────────────────────────────────────────────────────
function activate(ctx) {
  carregaMetadata(ctx);
  indexaLibsInstaladas();

  const sel = { language: 'poolscript' };
  ctx.subscriptions.push(
    vscode.languages.registerCompletionItemProvider(sel, completionProvider, '.', '=', '('),
    vscode.languages.registerHoverProvider(sel, hoverProvider),
  );
  // re-indexa libs instaladas se elas mudarem
  const watcher = vscode.workspace.createFileSystemWatcher('**/.poolscript/libs/**');
  watcher.onDidCreate(indexaLibsInstaladas);
  watcher.onDidChange(indexaLibsInstaladas);
  watcher.onDidDelete(indexaLibsInstaladas);
  ctx.subscriptions.push(watcher);
}

function deactivate() {}

module.exports = { activate, deactivate };

// exposto só pra teste (harness Node) — não usado pelo VS Code
module.exports.__test__ = {
  parseChain, resolveChain, tipoDaVar, membrosDoTipo, parsePoolSource,
  getMeta: () => META,
};
