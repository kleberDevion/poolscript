'use strict';
const vscode = require('vscode');
const path   = require('path');
const fs     = require('fs');
const { spawn } = require('child_process');

// ─── Stdlib metadata ──────────────────────────────────────────────────────────
// Gerado por bridge/gen_stdlib_metadata.py a partir da stdlib REAL do
// poolscript (introspecção, não cópia manual) — rodar esse script de novo
// sempre que a stdlib mudar (nova lib, nova função, rename, assinatura nova).
// Antes disso era uma cópia manual (STDLIB_MEMBERS hardcoded aqui), que ficava
// dessincronizada sempre que a stdlib mudava e ninguém lembrava de atualizar
// os dois lados — exatamente o "autocomplete funciona pra um módulo e pra
// outro não" reportado.
const KIND_MAP = {
    function: vscode.CompletionItemKind.Function,
    method:   vscode.CompletionItemKind.Method,
    class:    vscode.CompletionItemKind.Class,
    property: vscode.CompletionItemKind.Property,
};

let stdlibMetadata = {};
let STDLIB_MEMBERS = {};
// Classes devolvidas por factory functions da stdlib (WsConnection de
// request.ws_connect, Response de request.get/post, JinkerResponse de
// jsonify/render, ...) — gen_stdlib_metadata.py introspecciona a classe de
// retorno de cada função via type hint e coloca aqui embaixo de
// "__classes__". Sem isso, `conn = request.ws_connect(...); conn.<TAB>`
// não tinha member-completion nenhuma: STDLIB_MEMBERS só cobre membros de
// MÓDULO (request.X), não da instância devolvida por uma chamada.
let STDLIB_CLASSES = {};
function loadStdlibMetadata() {
    try {
        const p = path.join(__dirname, 'bridge', 'stdlib_metadata.json');
        if (!fs.existsSync(p)) {
            console.warn(`[poolscript] stdlib_metadata.json não encontrado em ${p} — autocomplete de membros de módulo (os.X, psodbc.X, ...) fica sem sugestões. Rode: python bridge/gen_stdlib_metadata.py`);
            return;
        }
        stdlibMetadata = JSON.parse(fs.readFileSync(p, 'utf-8'));
        const members = {};
        for (const [libName, entries] of Object.entries(stdlibMetadata)) {
            if (libName === '__classes__' || libName === '__builtins__') continue;
            // Inclui pseudo-módulos de builtin singleton (ex: "Parsing", ver
            // gen_stdlib_metadata.py) — mesmo tratamento de "os"/"date"/etc,
            // então "Parsing.<TAB>" cai no MESMO tier 1 de completion sem
            // nenhum código extra aqui.
            members[libName] = entries.map(e => ({
                name: e.name,
                kind: KIND_MAP[e.kind] || vscode.CompletionItemKind.Function,
                detail: e.detail,
                returns: e.returns || null,
            }));
        }
        STDLIB_MEMBERS = members;

        const classes = {};
        for (const [className, entries] of Object.entries(stdlibMetadata.__classes__ || {})) {
            classes[className] = entries.map(e => ({
                name: e.name,
                kind: KIND_MAP[e.kind] || vscode.CompletionItemKind.Method,
                detail: e.detail,
            }));
        }
        STDLIB_CLASSES = classes;

        // Builtins globais (sem import) — gerados por introspecção real de
        // GLOBAL_BUILTINS (builtins.py). A lista GLOBAL_BUILTINS hardcoded
        // logo no topo deste arquivo cobre também builtins registrados só
        // no interpreter.py (post/input/map/filter/sleep/...), que não
        // passam por builtins.py — por isso é um MERGE (nome novo entra,
        // nome já existente na lista fixa não é sobrescrito), não substituição.
        const seenBuiltins = new Set(GLOBAL_BUILTINS.map(b => b.name));
        for (const b of (stdlibMetadata.__builtins__ || [])) {
            if (seenBuiltins.has(b.name)) continue;
            seenBuiltins.add(b.name);
            GLOBAL_BUILTINS.push({ name: b.name, detail: b.detail });
        }
    } catch (e) {
        console.warn(`[poolscript] falha ao carregar stdlib_metadata.json: ${e}`);
    }
}

// ─── Helpers ──────────────────────────────────────────────────────────────────
function escapeRegex(s) { return s.replace(/[.*+?^${}()|[\]\\]/g, '\\$&'); }

/** Resolve o nome de import (com apelido) de volta pra chave da stdlib, ex:
 * `import request as r` → resolveStdlibKey('r', localData) === 'request'.
 * Compartilhado entre completion de módulo (request.X) e inferência de tipo
 * de retorno (x = request.ws_connect(...)), pra não duplicar a lógica de
 * apelido nos dois lugares. */
function resolveStdlibKey(objName, localData) {
    let stdlibKey = objName.replace(/^_/, ''); // ex: _json → json
    if (!STDLIB_MEMBERS[stdlibKey] && localData) {
        const imp = localData.imports.find(i => i.mode === 'import' && (i.alias || i.module) === objName);
        if (imp && STDLIB_MEMBERS[imp.module]) stdlibKey = imp.module;
    }
    return stdlibKey;
}

/** Monta uma assinatura legível a partir dos dados reais vindos da bridge Python. */
function formatFunctionSignature(fn) {
    if (!fn) return '';
    const defaults = new Set(fn.defaults || []);
    const params = (fn.params || []).map(p => defaults.has(p) ? `${p}=…` : p);
    const staticPrefix = fn.isStatic ? '@static ' : '';
    const asyncPrefix  = fn.isAsync ? 'async ' : '';
    const rtPrefix      = fn.returnType ? `${fn.returnType} ` : '';
    return `${staticPrefix}${asyncPrefix}${rtPrefix}action ${fn.name}(${params.join(', ')})`;
}

// ─── Builtins globais da linguagem (sempre disponíveis, sem import) ──────────
// Espelha GLOBAL_BUILTINS (builtins.py) + globals.define(...) (interpreter.py)
// do poolscript-lang. Sem isso, coisas como post()/len()/addEnd() nunca
// apareciam no autocomplete — exatamente o problema reportado.
const GLOBAL_BUILTINS = [
    { name: 'post',        detail: 'post(...) — imprime os argumentos (join por espaço)' },
    { name: 'post.flush',  detail: 'post.flush(texto, delay=0.05) — efeito de digitação' },
    { name: 'input',       detail: 'input(prompt="") → str' },
    { name: 'open',        detail: 'open(path, modo) → FileHandle' },
    { name: 'len',         detail: 'len(x) → int' },
    { name: 'range',       detail: 'range(...) → lista de inteiros' },
    { name: 'type',        detail: 'type(x) → str' },
    { name: 'id',          detail: 'id(x) → int (endereço de memória)' },
    { name: 'hex',         detail: 'hex(n) → str, ex: "0xff"' },
    { name: 'bin',         detail: 'bin(n) → str, ex: "0b1010"' },
    { name: 'oct',         detail: 'oct(n) → str, ex: "0o17"' },
    { name: 'ord',         detail: 'ord(char) → int' },
    { name: 'chr',         detail: 'chr(n) → str (1 caractere)' },
    { name: 'abs',         detail: 'abs(n) → valor absoluto' },
    { name: 'round',       detail: 'round(n) → arredondamento' },
    { name: 'sum',         detail: 'sum(lista) → soma dos itens' },
    { name: 'min',         detail: 'min(...) → menor valor' },
    { name: 'max',         detail: 'max(...) → maior valor' },
    { name: 'sorted',      detail: 'sorted(lista) → lista ordenada' },
    { name: 'reversed',    detail: 'reversed(lista) → lista invertida' },
    { name: 'enumerate',   detail: 'enumerate(lista) → [(0,x), (1,y), ...]' },
    { name: 'zip',         detail: 'zip(a, b, ...) → [(a0,b0), ...]' },
    { name: 'map',         detail: 'map(lista, fn) → lista transformada' },
    { name: 'filter',      detail: 'filter(lista, fn) → lista filtrada' },
    { name: 'addEnd',      detail: 'addEnd(lista, valor) — adiciona ao final (in-place)' },
    { name: 'removeEnd',   detail: 'removeEnd(lista) → último item removido (ou null se vazia)' },
    { name: 'addStart',    detail: 'addStart(lista, valor) — adiciona ao início (in-place)' },
    { name: 'removeStart', detail: 'removeStart(lista) → primeiro item removido (ou null se vazia)' },
    { name: 'sleep',       detail: 'sleep(segundos) — pausa a execução' },
    { name: 'gather',      detail: 'gather(f1, f2, ...) — aguarda múltiplos futures async' },
    { name: 'load',        detail: 'load(...)' },
    { name: 'str',         detail: 'str(x) → converte para texto' },
    { name: 'int',         detail: 'int(x) → converte para inteiro' },
    { name: 'flo',         detail: 'flo(x) → converte para número decimal' },
    { name: 'bool',        detail: 'bool(x) → converte para booleano' },
];


/**
 * Coleta todas as linhas do corpo de um bloco Entity (indentado ou {}).
 * Retorna as linhas coletadas e o índice da próxima linha após o bloco.
 */
function collectBlock(lines, startLine, baseIndent, usesBraces) {
    const body = [];
    let i = startLine;
    if (usesBraces) {
        let depth = 1; // abrimos 1 { na linha da Entity
        while (i < lines.length) {
            const bl = lines[i];
            for (const ch of bl) {
                if (ch === '{') depth++;
                else if (ch === '}') depth--;
            }
            if (depth <= 0) { i++; break; } // fechou o bloco da Entity
            body.push(bl);
            i++;
        }
    } else {
        while (i < lines.length) {
            const bl = lines[i];
            if (bl.trim() === '') { body.push(bl); i++; continue; }
            const indent = (bl.match(/^( *)/) || ['', ''])[1].length;
            if (indent <= baseIndent) break;
            body.push(bl);
            i++;
        }
    }
    return { body, nextLine: i };
}

/**
 * Extrai métodos (actions) e propriedades (self.x) de um corpo de Entity.
 * __init__ é ignorado como método mas suas atribuições self.x são coletadas.
 */
function extractEntityMembers(bodyLines) {
    const members = [];
    const seen = new Set();

    for (const line of bodyLines) {
        const t = line.trim();

        // action/reaction dentro da Entity — 'reaction' é sinônimo de 'action'
        // no lexer real (lexer.py: KW inclui os dois), mas esse extrator
        // regex de fallback só conhecia 'action' — por isso um método
        // declarado com 'reaction' sumia do hover/completion sempre que o
        // bridge Python (parser de verdade) não estava disponível/atualizado.
        let m = t.match(/^(?:async\s+)?(?:action|reaction)\s+([A-Za-z_]\w*)\s*\(([^)]*)\)/);
        if (m && m[1] !== '__init__') {
            if (!seen.has(m[1])) {
                const params = m[2].split(',')
                    .map(p => p.trim())
                    .filter(p => p && p !== 'self');
                members.push({
                    name:   m[1],
                    kind:   vscode.CompletionItemKind.Method,
                    detail: `action ${m[1]}(${params.join(', ')})`
                });
                seen.add(m[1]);
            }
            continue;
        }

        // self.prop = ...  (dentro de qualquer método, inclusive __init__)
        m = t.match(/^self\.([A-Za-z_]\w*)\s*=/);
        if (m && !seen.has(m[1])) {
            members.push({
                name:   m[1],
                kind:   vscode.CompletionItemKind.Property,
                detail: `property ${m[1]}`
            });
            seen.add(m[1]);
        }
    }
    return members;
}

// ─── Parser de arquivo .ps (regex) ────────────────────────────────────────────
// Usado só como FALLBACK quando a bridge Python (parser real) não está
// disponível ou falhou ao parsear (ver PythonBridge / WorkspaceSymbolProvider
// abaixo). Continua existindo pra a extensão nunca ficar 100% cega.
function parsePoolScript(content) {
    const symbols   = []; // { name, kind, members?, detail? }
    const imports   = []; // { module, alias, names, line, mode }
    const variables = []; // { name, line, col }

    const lines = content.split('\n');
    const RESERVED = new Set([
        'if','elif','else','for','while','return','action','reaction',
        'Entity','Class','class','import','from','true','false','null',
        'none','and','or','not','in','is','count','each'
    ]);

    let i = 0;
    while (i < lines.length) {
        const raw = lines[i];
        const t   = raw.trim();

        // ── import X [as Y] ──────────────────────────────────────────
        let m = t.match(/^import\s+([A-Za-z_]\w*)(?:\s+as\s+([A-Za-z_]\w*))?$/);
        if (m) {
            imports.push({ module: m[1], alias: m[2] || null, names: [], line: i, mode: 'import' });
            i++; continue;
        }

        // ── from X import Y [as Z], ... ──────────────────────────────
        m = t.match(/^from\s+([\w./]+)\s+import\s+(.+)$/);
        if (m) {
            const names = m[2].split(',').map(n => {
                const parts = n.trim().split(/\s+as\s+/);
                return { original: parts[0].trim(), alias: parts[1]?.trim() ?? null };
            });
            imports.push({ module: m[1], names, line: i, mode: 'from' });
            i++; continue;
        }

        // ── Entity / Class ────────────────────────────────────────────
        m = t.match(/^(?:Entity|Class|class)\s+([A-Za-z_]\w*)\s*\([^)]*\)\s*([:{])?/);
        if (m) {
            const entityName  = m[1];
            const baseIndent  = (raw.match(/^( *)/) || ['', ''])[1].length;
            const usesBraces  = t.endsWith('{');
            const { body, nextLine } = collectBlock(lines, i + 1, baseIndent, usesBraces);
            const members = extractEntityMembers(body);
            symbols.push({ name: entityName, kind: vscode.CompletionItemKind.Class, members });
            i = nextLine;
            continue;
        }

        // ── action/reaction global — 'reaction' é um alias de 'action' no
        // lexer real (poolscript/lexer.py: KEYWORDS inclui os dois), então
        // precisa ser reconhecido aqui igual — ver nota em extractEntityMembers.
        m = t.match(/^(?:@\w+(?:\([^)]*\))?\s+)*(?:async\s+)?(?:action|reaction)\s+([A-Za-z_]\w*)\s*\(([^)]*)\)/);
        if (m) {
            const fnName = m[1];
            const params = m[2].split(',').map(p => p.trim()).filter(Boolean);
            symbols.push({
                name:   fnName,
                kind:   vscode.CompletionItemKind.Function,
                detail: `action ${fnName}(${params.join(', ')})`
            });
        }

        // ── atribuição de variável simples ────────────────────────────
        m = t.match(/^([A-Za-z_]\w*)\s*=\s*(.+)$/);
        if (m && !RESERVED.has(m[1])) {
            variables.push({ name: m[1], line: i, col: raw.indexOf(m[1]) });
        }

        i++;
    }

    return { symbols, imports, variables };
}

// ─── PythonBridge ──────────────────────────────────────────────────────────────
// Processo Python persistente que lexa+parseia arquivos .ps com o parser REAL
// da linguagem (bridge/poolscript_pkg — cópia local de lexer.py/parser.py).
// Fala um protocolo simples: uma linha JSON por request, uma linha JSON por
// response, correlacionadas por "id". NUNCA executa o código do usuário —
// só análise estática, igual à garantia que o Pylance dá pro Python.
class PythonBridge {
    constructor() {
        this.available = false;
        this.proc = null;
        this.nextId = 1;
        this.pending = new Map();
        this._buffer = '';
        this._warned = false;
        this._disposed = false;
        // Antes: um crash com código de saída != 0 desligava o bridge PRA
        // SEMPRE pelo resto da sessão do VS Code (só reiniciava uma vez, e só
        // se o processo saísse com código 0 — o que quase nunca acontece num
        // crash de verdade). Um travamento isolado no início virava "cai pro
        // regex" permanentemente, sem aviso nenhum depois do 1º popup. Agora
        // sempre tenta de novo, com backoff — o popup de aviso ainda aparece
        // só uma vez (não fica martelando o usuário), mas a tentativa de
        // religar nunca para.
        this._restartAttempts = 0;
        this._start();
    }

    _pythonPath() {
        return vscode.workspace.getConfiguration('poolscript').get('pythonPath', 'python');
    }

    _start() {
        if (this._disposed) return;
        const scriptPath = path.join(__dirname, 'bridge', 'analyze.py');
        if (!fs.existsSync(scriptPath)) {
            this.available = false;
            return;
        }
        let proc;
        try {
            proc = spawn(this._pythonPath(), [scriptPath], { stdio: ['pipe', 'pipe', 'pipe'] });
        } catch (e) {
            this._onUnavailable(String(e && e.message || e));
            this._scheduleRestart();
            return;
        }
        this.proc = proc;
        this.available = true;

        proc.on('error', (e) => {
            this._onUnavailable(String(e && e.message || e));
            this._scheduleRestart();
        });
        proc.stdout.setEncoding('utf-8');
        proc.stdout.on('data', (chunk) => this._onData(chunk));
        proc.stderr.on('data', () => { /* mensagens não estruturadas — ignoradas */ });
        proc.on('exit', (code) => {
            this.available = false;
            for (const [, p] of this.pending) p.resolve(null);
            this.pending.clear();
            if (this._disposed) return;
            if (code !== 0) this._onUnavailable(`processo de análise saiu com código ${code}`);
            this._scheduleRestart();
        });
    }

    /** Backoff exponencial com teto de 30s — nunca desiste de vez, só
     * espaça as tentativas pra não martelar o sistema num crash-loop. */
    _scheduleRestart() {
        if (this._disposed) return;
        this._restartAttempts++;
        const delay = Math.min(30000, 1000 * Math.pow(2, Math.min(this._restartAttempts, 5)));
        setTimeout(() => { if (!this._disposed) this._start(); }, delay);
    }

    _onUnavailable(reason) {
        this.available = false;
        if (this._warned || this._disposed) return;
        this._warned = true;
        vscode.window.showWarningMessage(
            `PoolScript: não foi possível iniciar o analisador Python (${reason}). ` +
            `O IntelliSense vai funcionar em modo básico (sem diagnósticos/definições precisas) até religar. ` +
            `Configure "poolscript.pythonPath" nas settings se o Python não estiver no PATH.`
        );
    }

    _onData(chunk) {
        this._buffer += chunk;
        let idx;
        while ((idx = this._buffer.indexOf('\n')) !== -1) {
            const line = this._buffer.slice(0, idx);
            this._buffer = this._buffer.slice(idx + 1);
            if (!line.trim()) continue;
            let msg;
            try { msg = JSON.parse(line); } catch (_) { continue; }
            // Resposta válida chegou — o processo está saudável de verdade
            // (não só "spawned", mas respondendo). Zera o contador de
            // backoff e o aviso, pra um crash futuro poder avisar de novo.
            this._restartAttempts = 0;
            this._warned = false;
            const pending = this.pending.get(msg.id);
            if (pending) { this.pending.delete(msg.id); pending.resolve(msg); }
        }
    }

    /** Envia {path, text}, devolve a resposta da bridge ou null (indisponível/timeout). */
    analyze(filePath, text) {
        if (!this.available || !this.proc || !this.proc.stdin.writable) return Promise.resolve(null);
        const id = this.nextId++;
        return new Promise((resolve) => {
            const timer = setTimeout(() => {
                if (this.pending.has(id)) { this.pending.delete(id); resolve(null); }
            }, 5000);
            this.pending.set(id, { resolve: (msg) => { clearTimeout(timer); resolve(msg); } });
            try {
                this.proc.stdin.write(JSON.stringify({ id, path: filePath, text }) + '\n');
            } catch (_) {
                clearTimeout(timer);
                this.pending.delete(id);
                resolve(null);
            }
        });
    }

    dispose() {
        this._disposed = true;
        if (this.proc) { try { this.proc.kill(); } catch (_) {} }
    }
}

/** Converte a resposta JSON da bridge (functions/entities/variables/imports/
 * references) no mesmo formato { symbols, imports, variables } já consumido
 * pelo resto da extensão, preservando os dados crus (fn/entity/line/col) pra
 * uso de Definition/References/SignatureHelp/Hover. */
// A bridge Python usa a convenção do parser real: line/col 1-based (igual ao
// que aparece nas mensagens de erro do CLI). O VS Code (Position/Range) usa
// 0-based pra linha E coluna. O extrator regex (parsePoolScript, modo
// fallback) já produz line/col 0-based nativamente. Pra todo consumidor
// (Linter/Definition/References/Hover/Completion) poder tratar fileData de
// forma uniforme não importa a origem, normalizamos AQUI, na fronteira —
// convertendo pra 0-based uma única vez, ao montar o fileData a partir da
// resposta da bridge.
function toZeroBasedLine(line) { return Math.max(0, (line || 1) - 1); }
function toZeroBasedCol(col)   { return Math.max(0, (col  || 1) - 1); }

function bridgeResultToFileData(result) {
    const symbols = [];

    for (const fn of result.functions || []) {
        symbols.push({
            name: fn.name,
            kind: vscode.CompletionItemKind.Function,
            detail: formatFunctionSignature(fn),
            line: toZeroBasedLine(fn.line), col: toZeroBasedCol(fn.col),
            fn,
        });
    }

    for (const en of result.entities || []) {
        const members = [
            // __init__ não é chamado via ponto depois de instanciar
            // (p.__init__() não faz sentido) — não entra no completion de membro
            ...(en.methods || []).filter(m => m.name !== '__init__').map(m => ({
                name: m.name,
                kind: vscode.CompletionItemKind.Method,
                detail: formatFunctionSignature(m),
                line: toZeroBasedLine(m.line), col: toZeroBasedCol(m.col),
                fn: m,
            })),
            ...(en.fields || []).map(f => ({
                name: f.name,
                kind: vscode.CompletionItemKind.Property,
                detail: `property ${f.name}${f.type ? ': ' + f.type : ''}`,
                line: toZeroBasedLine(f.line), col: toZeroBasedCol(f.col),
            })),
        ];
        symbols.push({
            name: en.name,
            kind: vscode.CompletionItemKind.Class,
            members,
            line: toZeroBasedLine(en.line), col: toZeroBasedCol(en.col),
            entity: en,
        });
    }

    const variables = (result.variables || []).map(v => ({
        name: v.name, line: toZeroBasedLine(v.line), col: toZeroBasedCol(v.col), declaredType: v.declaredType,
        scope: v.scope || 'module',
        // Calculado uma vez pelo parser REAL (analyze.py: infer_type) — ver
        // getVariableType, que agora resolve tipo a partir disso em vez de
        // regex em cima do texto bruto.
        inferredType: v.inferredType || null,
    }));

    // Escopos (função/método): usados pra só sugerir variáveis locais
    // realmente visíveis a partir da posição do cursor — sem isso, TODA
    // variável do arquivo inteiro aparecia junto, mesmo a de outra função.
    const scopes = (result.scopes || []).map(s => ({
        id: s.id,
        startLine: toZeroBasedLine(s.startLine),
        endLine: toZeroBasedLine(s.endLine),
    }));

    const imports = (result.imports || []).map(imp => ({
        module: imp.module,
        alias: imp.alias,
        names: (imp.names || []).map(n => ({ original: n.name, alias: n.alias })),
        line: toZeroBasedLine(imp.line),
        mode: imp.mode,
        level: imp.level || 0, // 0 = absoluto; N = `from .x`/`from ..pkg.x` (N pontos)
    }));

    const references = {};
    for (const [name, occurrences] of Object.entries(result.references || {})) {
        references[name] = occurrences.map(o => ({ line: toZeroBasedLine(o.line), col: toZeroBasedCol(o.col) }));
    }

    return { symbols, imports, variables, references, scopes };
}

// ─── WorkspaceSymbolProvider ──────────────────────────────────────────────────
class WorkspaceSymbolProvider {
    constructor(bridge) {
        this.bridge = bridge;
        this.fileData = new Map(); // fsPath → { symbols, imports, variables, references, errors, ok }
        this._indexWorkspace();
        vscode.workspace.onDidDeleteFiles(e => e.files.forEach(u => this.fileData.delete(u.fsPath)));
        vscode.workspace.onDidCreateFiles(e => e.files.forEach(u => this._parseUri(u)));
        // Renomear arquivo/pasta no Explorer NÃO disparava atualização de
        // import em nenhum outro arquivo do projeto (buraco real reportado —
        // Pylance/outros LSPs fazem isso). onWillRenameFiles monta os edits
        // ANTES do rename efetivar (aplicado atomicamente junto pelo VS Code);
        // onDidRenameFiles só realoca a entrada em fileData pro fsPath novo.
        vscode.workspace.onWillRenameFiles(e => {
            e.waitUntil(this._buildRenameImportsEdit(e.files));
        });
        vscode.workspace.onDidRenameFiles(e => {
            for (const { oldUri, newUri } of e.files) {
                const data = this.fileData.get(oldUri.fsPath);
                if (data) {
                    this.fileData.delete(oldUri.fsPath);
                    this.fileData.set(newUri.fsPath, data);
                }
                this._parseUri(newUri);
            }
        });
    }

    async _indexWorkspace() {
        this.fileData.clear();
        const files = await vscode.workspace.findFiles('**/*.{ps,psl}', '**/node_modules/**');
        for (const f of files) await this._parseUri(f);
    }

    async _parseUri(uri) {
        try {
            const doc = await vscode.workspace.openTextDocument(uri);
            await this.reparse(doc);
        } catch (_) {}
    }

    /** Reanalisa um documento (via bridge Python, com fallback regex) e
     * atualiza o índice. Retorna os dados novos. */
    async reparse(doc) {
        if (doc.languageId !== 'poolscript') return null;
        const text = doc.getText();
        let data = null;

        if (this.bridge && this.bridge.available) {
            const result = await this.bridge.analyze(doc.uri.fsPath, text);
            if (result) {
                if (result.ok) {
                    data = bridgeResultToFileData(result);
                    data.errors = [];
                    data.ok = true;
                } else {
                    // Parse falhou: mantém os ERROS reais (linha/coluna do parser
                    // de verdade), mas cai pro extrator regex só pra completion/
                    // hover não pararem 100% enquanto há um erro de digitação.
                    const fb = parsePoolScript(text);
                    data = {
                        symbols: fb.symbols, imports: fb.imports, variables: fb.variables,
                        references: null, scopes: [],
                        errors: result.errors || [],
                        ok: false,
                    };
                }
            }
        }

        if (!data) {
            // Bridge indisponível — modo 100% regex (comportamento anterior).
            const fb = parsePoolScript(text);
            data = {
                symbols: fb.symbols, imports: fb.imports, variables: fb.variables,
                references: null, scopes: [],
                errors: null, // null = "sem diagnóstico real disponível" (linter usa heurística antiga)
                ok: false,
            };
        }

        this.fileData.set(doc.uri.fsPath, data);
        return data;
    }

    /** Retorna símbolo pelo nome: local → import → workspace */
    getSymbolByName(name, uri) {
        const local = this.fileData.get(uri?.fsPath);
        if (local) {
            const s = local.symbols.find(s => s.name === name);
            if (s) return s;

            // resolve via import
            for (const imp of local.imports) {
                if (imp.mode === 'import' && (imp.alias || imp.module) === name) {
                    const ms = this._getModuleSymbols(imp.module, uri.fsPath, imp.level);
                    return ms ? { name, kind: vscode.CompletionItemKind.Module, members: ms } : null;
                }
                if (imp.mode === 'from') {
                    const found = imp.names.find(n => (n.alias || n.original) === name);
                    if (found) {
                        const ms = this._getModuleSymbols(imp.module, uri.fsPath, imp.level);
                        return ms?.find(s => s.name === found.original) ?? null;
                    }
                }
            }
        }
        // busca global
        for (const [, data] of this.fileData) {
            const s = data.symbols.find(s => s.name === name);
            if (s) return s;
        }
        return null;
    }

    /** Converte fsPath → nome de módulo dotted, relativo à raiz do workspace
     * (só suporta import absoluto, level=0 — é o que _fsPathToModuleName
     * consegue inferir sem saber de onde o import parte). Ex:
     * <root>/utils/helpers.ps → "utils.helpers". Usado por
     * _buildRenameImportsEdit pra saber o nome antigo/novo de um arquivo ou
     * pasta renomeada, e comparar contra imp.module dos arquivos indexados. */
    _fsPathToModuleName(fsPath) {
        const folder = vscode.workspace.getWorkspaceFolder(vscode.Uri.file(fsPath));
        const base = folder ? folder.uri.fsPath
            : (vscode.workspace.workspaceFolders && vscode.workspace.workspaceFolders[0]
                ? vscode.workspace.workspaceFolders[0].uri.fsPath : null);
        if (!base) return null;
        let rel = path.relative(base, fsPath);
        if (!rel || rel.startsWith('..')) return null;
        rel = rel.replace(/\.(ps|psl)$/, '');
        return rel.split(path.sep).filter(Boolean).join('.');
    }

    /** Monta o WorkspaceEdit que atualiza "import X"/"from X import Y" em
     * TODO arquivo indexado que referencia o módulo/pasta renomeado — cobre
     * tanto o próprio arquivo (module === oldModule) quanto qualquer coisa
     * dentro de uma pasta renomeada (module.startsWith(oldModule + '.')).
     * Só cobre import ABSOLUTO (level=0); import relativo (`from .x`) não
     * muda com o rename porque a estrutura relativa entre os arquivos
     * continua a mesma. */
    async _buildRenameImportsEdit(files) {
        const edit = new vscode.WorkspaceEdit();
        for (const { oldUri, newUri } of files) {
            const oldModule = this._fsPathToModuleName(oldUri.fsPath);
            const newModule = this._fsPathToModuleName(newUri.fsPath);
            if (!oldModule || !newModule || oldModule === newModule) continue;

            for (const [fsPath, data] of this.fileData) {
                const uri = fsPath === oldUri.fsPath ? oldUri : vscode.Uri.file(fsPath);
                for (const imp of data.imports || []) {
                    if (imp.level > 0) continue;
                    if (imp.module !== oldModule && !imp.module.startsWith(oldModule + '.')) continue;
                    const newModuleName = newModule + imp.module.slice(oldModule.length);
                    try {
                        const doc = await vscode.workspace.openTextDocument(uri);
                        const lineText = doc.lineAt(imp.line).text;
                        const col = lineText.indexOf(imp.module);
                        if (col >= 0) {
                            edit.replace(uri, new vscode.Range(imp.line, col, imp.line, col + imp.module.length), newModuleName);
                        }
                    } catch (_) { /* arquivo pode ter sido fechado/movido nesse meio-tempo */ }
                }
            }
        }
        return edit;
    }

    /** Base dir de resolução de import, espelhando Interpreter._exec_import
     * (poolscript-lang): `level>0` (`from .x`/`from ..pkg.x`) sobe (level-1)
     * diretórios a partir da pasta do arquivo que importa; `level===0`
     * (import absoluto) resolve a partir da raiz do workspace — o mais perto
     * que dá de "a pasta do script de entrada" fora de uma execução real. */
    _importBaseDir(currentFsPath, level) {
        if (level > 0) {
            let dir = currentFsPath ? path.dirname(currentFsPath) : null;
            for (let i = 0; dir && i < level - 1; i++) dir = path.dirname(dir);
            return dir;
        }
        if (currentFsPath) {
            const folder = vscode.workspace.getWorkspaceFolder(vscode.Uri.file(currentFsPath));
            if (folder) return folder.uri.fsPath;
        }
        const folders = vscode.workspace.workspaceFolders;
        if (folders && folders.length) return folders[0].uri.fsPath;
        return currentFsPath ? path.dirname(currentFsPath) : null;
    }

    /** Acha o fsPath de um módulo .ps/.psl pelo nome dotted (ex: "pkg.sub"),
     * respeitando `level` (0 = absoluto, a partir da raiz do workspace; N =
     * relativo, a partir da pasta do arquivo atual). Cada segmento antes do
     * último vira uma subpasta. Faz fallback pra busca por basename em
     * qualquer arquivo indexado — mantém extensões antigas funcionando
     * mesmo fora da estrutura de pastas exata. */
    _resolveModuleFsPath(moduleName, currentFsPath, level = 0) {
        const parts = (moduleName || '').split('.').filter(Boolean);
        if (parts.length) {
            const base = this._importBaseDir(currentFsPath, level);
            if (base) {
                const dir = parts.length > 1 ? path.join(base, ...parts.slice(0, -1)) : base;
                const fileName = parts[parts.length - 1];
                for (const ext of ['.ps', '.psl']) {
                    const candidate = path.join(dir, fileName + ext);
                    if (this.fileData.has(candidate)) return candidate;
                }
            }
        }
        const fileName = parts[parts.length - 1] || moduleName;
        for (const [fsPath] of this.fileData) {
            if (path.basename(fsPath, path.extname(fsPath)) === fileName) return fsPath;
        }
        return null;
    }

    /** Busca símbolos de um módulo pelo nome do arquivo .ps/.psl */
    _getModuleSymbols(moduleName, currentFsPath, level = 0) {
        const fsPath = this._resolveModuleFsPath(moduleName, currentFsPath, level);
        return fsPath ? this.fileData.get(fsPath).symbols : null;
    }

    /** fsPaths de todos os módulos .ps/.psl que `imports` realmente importa
     * (relativo a `currentFsPath`) — usado pra nunca sugerir símbolo/variável
     * de um arquivo que não foi importado. */
    getImportedFsPaths(imports, currentFsPath) {
        const result = new Set();
        for (const imp of imports || []) {
            const fsPath = this._resolveModuleFsPath(imp.module, currentFsPath, imp.level);
            if (fsPath) result.add(fsPath);
        }
        return result;
    }

    /** Todos os símbolos do workspace sem duplicatas */
    getAllSymbols() {
        const seen = new Set();
        const result = [];
        for (const [, data] of this.fileData) {
            for (const s of data.symbols) {
                if (!seen.has(s.name)) { seen.add(s.name); result.push(s); }
            }
        }
        return result;
    }

    /** Acha onde `name` foi declarado (function/Entity/member/variável), em
     * qualquer arquivo indexado — prioriza o arquivo atual. Usado pelo
     * Definition/References. Devolve {fsPath, line, col} ou null. */
    findDeclaration(name, preferredFsPath) {
        const order = [];
        if (preferredFsPath && this.fileData.has(preferredFsPath)) order.push(preferredFsPath);
        for (const fsPath of this.fileData.keys()) {
            if (fsPath !== preferredFsPath) order.push(fsPath);
        }
        for (const fsPath of order) {
            const data = this.fileData.get(fsPath);
            const sym = data.symbols.find(s => s.name === name);
            if (sym && typeof sym.line === 'number') {
                return { fsPath, line: sym.line, col: sym.col ?? 0 };
            }
            for (const s of data.symbols) {
                if (!s.members) continue;
                const m = s.members.find(mm => mm.name === name);
                if (m && typeof m.line === 'number') return { fsPath, line: m.line, col: m.col ?? 0 };
            }
            const v = data.variables.find(vv => vv.name === name);
            if (v && typeof v.line === 'number') return { fsPath, line: v.line, col: v.col ?? 0 };
        }
        return null;
    }
}

// ─── PoolScriptAnalyzer ───────────────────────────────────────────────────────
class PoolScriptAnalyzer {
    constructor(document, position, provider) {
        this.document = document;
        this.position = position;
        this.provider = provider;
        this.line = document.lineAt(position.line).text;
    }

    getMemberAccess() {
        const until = this.line.substring(0, this.position.character);
        const m = until.match(/(\w+)\.(\w*)$/);
        return m ? { obj: m[1], member: m[2] } : null;
    }

    getVariableType(varName) {
        // 'self' → encontra a Entity mais próxima acima
        if (varName === 'self') {
            const lines = this.document.getText().split('\n');
            for (let i = this.position.line; i >= 0; i--) {
                const m = lines[i].match(/(?:Entity|Class|class)\s+([A-Za-z_]\w*)/);
                if (m) return m[1];
            }
            return null;
        }

        const localData = this.provider.fileData.get(this.document.uri.fsPath);

        // Caminho principal: usa o "inferredType" calculado pelo parser REAL
        // (analyze.py: infer_type), a partir da árvore — não regex em cima
        // do texto. Pega a atribuição mais recente antes do cursor pra esse
        // nome (a árvore garante que é a atribuição de verdade, não um match
        // por acaso dentro de string/comentário/outra função).
        if (localData && localData.ok && Array.isArray(localData.variables)) {
            const candidates = localData.variables.filter(v => v.name === varName && v.line <= this.position.line);
            if (candidates.length) {
                const latest = candidates.reduce((a, b) => (b.line > a.line ? b : a));
                const it = latest.inferredType;
                if (!it) return null; // parser já disse: não é dict/call — não arrisca ficar num tipo de uma atribuição mais antiga
                if (it.kind === 'dict') return 'json_dict';
                if (it.kind === 'call') return this._resolveCalleeType(it.callee, localData);
                return null;
            }
        }

        // Fallback: bridge indisponível (fileData.ok === false, sem
        // inferredType nenhum) — regex antigo em cima do texto bruto,
        // menos preciso mas melhor que nada enquanto há erro de digitação.
        const textBefore = this.document.getText(
            new vscode.Range(0, 0, this.position.line, this.position.character)
        );
        const lines = textBefore.split('\n').reverse();
        for (const ln of lines) {
            let dm = ln.match(new RegExp(`\\b${escapeRegex(varName)}\\s*=\\s*([A-Za-z_]\\w*)\\.([A-Za-z_]\\w*)\\s*\\(`));
            if (dm) {
                const resolved = this._resolveCalleeType([dm[1], dm[2]], localData);
                if (resolved) return resolved;
            }
            let m = ln.match(new RegExp(`\\b${escapeRegex(varName)}\\s*=\\s*([A-Za-z_]\\w*)\\s*\\(`));
            if (m) {
                const resolved = this._resolveCalleeType([m[1]], localData);
                if (resolved) return resolved;
            }
            if (ln.match(new RegExp(`\\b${escapeRegex(varName)}\\s*=\\s*\\{`))) return 'json_dict';
        }
        return null;
    }

    /** Resolve um "callee" (caminho pontilhado da chamada do lado direito de
     * uma atribuição, ex: ["Pessoa"] ou ["request","ws_connect"] ou
     * ["modulo","Pessoa"]) pro tipo real: instância de Entity local, classe
     * da stdlib, ou objeto devolvido por factory function da stdlib.
     * Compartilhado entre o caminho principal (AST real) e o fallback
     * (regex) — a lógica de resolução é a mesma, só muda de onde o
     * "callee" veio. */
    _resolveCalleeType(callee, localData) {
        if (!callee || !callee.length) return null;
        if (callee.length === 1) {
            const sym = this.provider.getSymbolByName(callee[0], this.document.uri);
            if (sym?.kind === vscode.CompletionItemKind.Class) return callee[0];
            return null;
        }
        const [first, second] = callee;
        // objeto devolvido por função da stdlib: request.ws_connect(...) etc.
        // — resolve via "returns" do metadata (introspecção real, ver
        // gen_stdlib_metadata.py) — cobre tanto função-fábrica quanto classe
        // exportada direto (ex: jinker.Jinker).
        const stdlibKey = resolveStdlibKey(first, localData);
        const member = (STDLIB_MEMBERS[stdlibKey] || []).find(mm => mm.name === second);
        if (member?.returns && STDLIB_CLASSES[member.returns]) {
            return { stdlibClass: member.returns };
        }
        // módulo LOCAL — modulo.MinhaClasse(...): "import modulo" (não
        // "from modulo import Classe") + instanciar via prefixo do módulo.
        if (localData) {
            const modImp = localData.imports.find(i =>
                i.mode === 'import' && (i.alias || i.module) === first);
            if (modImp) {
                const ms = this.provider._getModuleSymbols(modImp.module, this.document.uri.fsPath, modImp.level);
                const sym = ms && ms.find(s => s.name === second);
                if (sym?.kind === vscode.CompletionItemKind.Class) return second;
            }
        }
        return null;
    }

    getDictKeys(varName) {
        const text = this.document.getText();
        const keys = [];
        const re = new RegExp(`\\b${escapeRegex(varName)}\\s*=\\s*\\{([\\s\\S]*?)\\}`, 'g');
        let m;
        while ((m = re.exec(text)) !== null) {
            const kr = /"([^"]+)"\s*:/g;
            let km;
            while ((km = kr.exec(m[1])) !== null) keys.push(km[1]);
        }
        return keys;
    }
}

/** Acha a chamada envolvente do cursor (nome da função + índice do argumento
 * atual), varrendo o texto pra trás contando parênteses/vírgulas — mesma
 * técnica usada por outras extensões simples de linguagem sem type-checker
 * completo. */
function findEnclosingCall(document, position) {
    const text = document.getText(new vscode.Range(0, 0, position.line, position.character));
    let depth = 0;
    let argIndex = 0;
    for (let i = text.length - 1; i >= 0; i--) {
        const ch = text[i];
        if (ch === ')' || ch === ']') depth++;
        else if (ch === '(' ) {
            if (depth === 0) {
                const before = text.slice(0, i);
                const m = before.match(/([A-Za-z_]\w*)\s*$/);
                if (!m) return null;
                return { name: m[1], argIndex };
            }
            depth--;
        } else if (ch === '[') {
            if (depth > 0) depth--;
        } else if (ch === ',' && depth === 0) {
            argIndex++;
        }
    }
    return null;
}

/** Resolve os dados de função (fn real da bridge) de um símbolo, incluindo o
 * caso especial de instanciar uma Entity (usa os params do __init__, sem
 * 'self'). */
function resolveCallableFn(sym) {
    if (!sym) return null;
    if (sym.fn) return sym.fn;
    if (sym.kind === vscode.CompletionItemKind.Class) {
        const init = (sym.members || []).find(m => m.name === '__init__');
        if (init && init.fn) {
            return { ...init.fn, name: sym.name, params: init.fn.params.filter(p => p !== 'self') };
        }
    }
    return null;
}

/** Acha o escopo (função/método) que contém `line`, preferindo o de menor
 * alcance caso mais de um bata (não deveria acontecer hoje, já que
 * PoolScript não aninha funções na prática, mas é seguro por garantia). */
function findEnclosingScopeId(fileData, line) {
    if (!fileData || !fileData.scopes || !fileData.scopes.length) return null;
    let best = null;
    for (const s of fileData.scopes) {
        if (line >= s.startLine && line <= s.endLine) {
            if (!best || (s.endLine - s.startLine) < (best.endLine - best.startLine)) best = s;
        }
    }
    return best ? best.id : null;
}

/** true se tudo antes do cursor, na linha atual, for espaço em branco —
 * ou seja, o cursor está no INÍCIO lógico de um statement (onde keywords
 * como `if`/`while`/`import` fazem sentido). Fora disso (no meio de uma
 * expressão, depois de `=`, dentro de uma chamada, etc.) essas keywords
 * não são sugestões válidas — só um valor/identificador é.
 *
 * Ignora a palavra parcial já digitada antes de checar: sem isso, o
 * momento em que você digita "re" (pra completar "reaction") já teria "re"
 * como texto não-branco antes do cursor, e a tier inteira de keywords de
 * início de statement (action/reaction/if/while/Entity/...) sumia assim
 * que a primeira letra era digitada — só aparecia com Ctrl+Space numa
 * linha 100% vazia, antes de digitar qualquer coisa. */
function isStatementStartContext(document, position) {
    const prefix = document.getText(new vscode.Range(position.line, 0, position.line, position.character));
    const withoutPartialWord = prefix.replace(/[A-Za-z_]\w*$/, '');
    return /^\s*$/.test(withoutPartialWord);
}

/** Decompõe o texto já digitado depois de "import "/"from " em
 * {level, descend, partial}. `level` = nº de pontos no início (0 = import
 * absoluto; N = `from .x`/`from ..pkg.x`, igual ao `level` do ImportStmt no
 * parser). `descend` = segmentos JÁ completos (viram subpastas a percorrer).
 * `partial` = pedaço final ainda sendo digitado (filtro de prefixo). Ex:
 * "..pkg.su" → level=2, descend=["pkg"], partial="su". */
function parseImportTyped(typed) {
    const dots = (typed.match(/^\.*/) || [''])[0];
    const level = dots.length;
    const rest = typed.slice(level);
    const segs = rest.length ? rest.split('.') : [''];
    return { level, descend: segs.slice(0, -1), partial: segs[segs.length - 1] };
}

/** True quando o cursor está logo depois de "import " ou "from " (com ou sem
 * um pedaço de módulo/caminho já digitado, inclusive pontos de import
 * relativo) — é onde sugerir lib/pasta/arquivo. */
function getImportModuleContext(document, position) {
    const prefix = document.getText(new vscode.Range(position.line, 0, position.line, position.character));
    let m = prefix.match(/^\s*import\s+([\w.]*)$/);
    if (m) return parseImportTyped(m[1]);
    m = prefix.match(/^\s*from\s+([\w.]*)$/);
    if (m) return parseImportTyped(m[1]);
    return null;
}

// ─── PoolScriptLinter ─────────────────────────────────────────────────────────
class PoolScriptLinter {
    constructor() {
        this.collection = vscode.languages.createDiagnosticCollection('poolscript');
    }

    lint(document, provider) {
        if (document.languageId !== 'poolscript') return;
        const fileData = provider.fileData.get(document.uri.fsPath);
        if (!fileData) { this.collection.set(document.uri, []); return; }

        const diags = [];
        const text = document.getText();

        // 1. Diagnósticos de sintaxe: reais (bridge) quando disponíveis,
        //    senão a heurística antiga como rede de segurança.
        if (Array.isArray(fileData.errors)) {
            for (const err of fileData.errors) {
                const lineNo = Math.max(0, Math.min(document.lineCount - 1, err.line - 1));
                const lineText = document.lineAt(lineNo).text;
                const col = Math.max(0, (err.col || 1) - 1);
                const endCol = Math.max(col + 1, Math.min(lineText.length, col + 1));
                diags.push(new vscode.Diagnostic(
                    new vscode.Range(lineNo, col, lineNo, endCol),
                    err.message,
                    vscode.DiagnosticSeverity.Error));
            }
        } else {
            diags.push(...this._checkSyntaxFallback(document, text));
        }

        // 2. Imports não usados → cinza (Hint + DiagnosticTag.Unnecessary)
        for (const imp of fileData.imports) {
            const entries = imp.mode === 'import'
                ? [{ name: imp.alias || imp.module, line: imp.line }]
                : imp.names.map(n => ({ name: n.alias || n.original, line: imp.line }));

            for (const { name, line } of entries) {
                const count = this._usageCount(fileData, document, text, name, line, false);
                if (count === 0) {
                    const lineText = document.lineAt(line).text;
                    const col = Math.max(0, lineText.indexOf(name));
                    const range = new vscode.Range(line, col, line, col + name.length);
                    const d = new vscode.Diagnostic(range,
                        `'${name}' importado mas nunca usado.`,
                        vscode.DiagnosticSeverity.Hint);
                    d.tags = [vscode.DiagnosticTag.Unnecessary]; // ← cinza no editor
                    diags.push(d);
                }
            }
        }

        // 3. Variáveis atribuídas mas nunca usadas → cinza
        for (const v of fileData.variables) {
            const count = this._usageCount(fileData, document, text, v.name, v.line, true);
            if (count <= 1) {
                const range = new vscode.Range(v.line, v.col, v.line, v.col + v.name.length);
                const d = new vscode.Diagnostic(range,
                    `'${v.name}' atribuída mas nunca usada.`,
                    vscode.DiagnosticSeverity.Hint);
                d.tags = [vscode.DiagnosticTag.Unnecessary];
                diags.push(d);
            }
        }

        this.collection.set(document.uri, diags);
    }

    /** Conta ocorrências de `name`: usa references reais da bridge quando
     * disponíveis (precisas, ignoram strings/comentários), senão regex sobre
     * o texto (comportamento antigo). `includeDeclLine` replica a semântica
     * original (imports contam só usos fora da linha de import; variáveis
     * contam TODAS as ocorrências, incluindo a própria atribuição). */
    _usageCount(fileData, document, text, name, declLine, includeDeclLine) {
        if (fileData.references) {
            return (fileData.references[name] || []).length;
        }
        const re = new RegExp(`\\b${escapeRegex(name)}\\b`, 'g');
        if (includeDeclLine) {
            return (text.match(re) || []).length;
        }
        const allLines = text.split('\n');
        let count = 0;
        for (let li = 0; li < allLines.length; li++) {
            if (li === declLine) continue;
            re.lastIndex = 0;
            while (re.exec(allLines[li]) !== null) count++;
        }
        return count;
    }

    _checkSyntaxFallback(document, text) {
        const diags = [];
        const lines = text.split('\n');

        // ── Chaves/parênteses/colchetes não fechados ───────────────
        const OPEN  = { '{': '}', '(': ')', '[': ']' };
        const CLOSE = new Set(['}', ')', ']']);
        const stack = [];
        let inStr     = null;
        let inTriple  = false;

        for (let li = 0; li < lines.length; li++) {
            const ln = lines[li];
            let ci = 0;
            while (ci < ln.length) {
                const ch  = ln[ci];
                const ch2 = ln.slice(ci, ci + 3);

                if (inTriple) {
                    if (ch2 === '"""') { inTriple = false; ci += 3; continue; }
                    ci++; continue;
                }
                if (inStr) {
                    if (ch === inStr && ln[ci - 1] !== '\\') inStr = null;
                    ci++; continue;
                }
                if (ch2 === '"""') { inTriple = true; ci += 3; continue; }
                if (ch === '"' || ch === "'") { inStr = ch; ci++; continue; }
                if (ch === '/' && ln[ci + 1] === '/') break; // comentário

                if (OPEN[ch])   { stack.push({ ch, li, ci }); }
                else if (CLOSE.has(ch)) {
                    if (stack.length && OPEN[stack[stack.length - 1].ch] === ch) {
                        stack.pop();
                    } else {
                        diags.push(new vscode.Diagnostic(
                            new vscode.Range(li, ci, li, ci + 1),
                            `'${ch}' sem abertura correspondente.`,
                            vscode.DiagnosticSeverity.Error));
                    }
                }
                ci++;
            }
        }
        for (const { ch, li, ci } of stack) {
            diags.push(new vscode.Diagnostic(
                new vscode.Range(li, ci, li, ci + 1),
                `'${ch}' nunca fechado.`,
                vscode.DiagnosticSeverity.Error));
        }

        // ── Verificações linha a linha ─────────────────────────────
        for (let li = 0; li < lines.length; li++) {
            const t = lines[li].trim();
            if (!t || t.startsWith('//')) continue;

            // action/reaction sem parênteses — dois testes: tem "action nome"
            // (ou "reaction nome", sinônimo no lexer real), mas NÃO tem "... nome("
            const kwKind = /^(?:@\w+(?:\([^)]*\))?\s+)*(action|reaction)\s+\w+/.exec(t);
            if (kwKind &&
                !new RegExp(`^(?:@\\w+(?:\\([^)]*\\))?\\s+)*${kwKind[1]}\\s+\\w+\\s*\\(`).test(t)) {
                const col = lines[li].search(new RegExp(`\\b${kwKind[1]}\\b`));
                diags.push(new vscode.Diagnostic(
                    new vscode.Range(li, col, li, col + kwKind[1].length),
                    `Assinatura de '${kwKind[1]}' sem parênteses.`,
                    vscode.DiagnosticSeverity.Warning));
            }

            // if/while/for sem bloco (sem : ou {)
            if (/^(?:if|elif|while|for)\b/.test(t) && !/[:{]$/.test(t) && !t.endsWith('{')) {
                const col = lines[li].search(/\b(?:if|elif|while|for)\b/);
                const kw  = (t.match(/^(if|elif|while|for)/) || [])[1] || 'bloco';
                diags.push(new vscode.Diagnostic(
                    new vscode.Range(li, col, li, col + kw.length),
                    `'${kw}' sem ':' ou '{' no final.`,
                    vscode.DiagnosticSeverity.Warning));
            }
        }

        return diags;
    }

    clear(uri) { this.collection.delete(uri); }
}

// ─── DefinitionProvider ────────────────────────────────────────────────────────
class PoolScriptDefinitionProvider {
    constructor(provider) { this.provider = provider; }

    provideDefinition(document, position) {
        const range = document.getWordRangeAtPosition(position);
        if (!range) return null;
        const word = document.getText(range);
        const decl = this.provider.findDeclaration(word, document.uri.fsPath);
        if (!decl) return null;
        const uri = decl.fsPath === document.uri.fsPath ? document.uri : vscode.Uri.file(decl.fsPath);
        // decl.line/decl.col já vêm 0-based (normalizados em bridgeResultToFileData
        // pro modo bridge; o modo fallback regex já produz 0-based nativamente).
        const pos = new vscode.Position(Math.max(0, decl.line), Math.max(0, decl.col));
        return new vscode.Location(uri, pos);
    }
}

// ─── ReferenceProvider ─────────────────────────────────────────────────────────
class PoolScriptReferenceProvider {
    constructor(provider) { this.provider = provider; }

    provideReferences(document, position) {
        const range = document.getWordRangeAtPosition(position);
        if (!range) return [];
        const word = document.getText(range);
        const locations = [];
        for (const [fsPath, data] of this.provider.fileData) {
            const refs = data.references && data.references[word];
            if (!refs || !refs.length) continue;
            const uri = fsPath === document.uri.fsPath ? document.uri : vscode.Uri.file(fsPath);
            for (const r of refs) {
                // r.line/r.col já vêm 0-based (normalizados em bridgeResultToFileData).
                locations.push(new vscode.Location(uri, new vscode.Position(Math.max(0, r.line), Math.max(0, r.col))));
            }
        }
        return locations;
    }
}

// ─── SignatureHelpProvider ─────────────────────────────────────────────────────
class PoolScriptSignatureHelpProvider {
    constructor(provider) { this.provider = provider; }

    provideSignatureHelp(document, position) {
        const call = findEnclosingCall(document, position);
        if (!call) return null;
        const sym = this.provider.getSymbolByName(call.name, document.uri);
        const fn = resolveCallableFn(sym);
        if (!fn) return null;

        const sig = new vscode.SignatureInformation(formatFunctionSignature(fn));
        sig.parameters = (fn.params || []).map(p => new vscode.ParameterInformation(p));

        const help = new vscode.SignatureHelp();
        help.signatures = [sig];
        help.activeSignature = 0;
        help.activeParameter = fn.params.length
            ? Math.min(call.argIndex, fn.params.length - 1)
            : 0;
        return help;
    }
}

/** Mapeia o CompletionItemKind (usado internamente pra symbols/members desde
 * bridgeResultToFileData) pro SymbolKind equivalente — reaproveitado por
 * DocumentSymbolProvider, WorkspaceSymbolSearchProvider e CallHierarchy. */
function completionKindToSymbolKind(kind) {
    switch (kind) {
        case vscode.CompletionItemKind.Function: return vscode.SymbolKind.Function;
        case vscode.CompletionItemKind.Method:   return vscode.SymbolKind.Method;
        case vscode.CompletionItemKind.Class:    return vscode.SymbolKind.Class;
        case vscode.CompletionItemKind.Property: return vscode.SymbolKind.Property;
        case vscode.CompletionItemKind.Variable: return vscode.SymbolKind.Variable;
        case vscode.CompletionItemKind.Module:   return vscode.SymbolKind.Module;
        default: return vscode.SymbolKind.Variable;
    }
}

// ─── DocumentSymbolProvider (Outline / breadcrumbs) ─────────────────────────────
class PoolScriptDocumentSymbolProvider {
    constructor(provider) { this.provider = provider; }

    provideDocumentSymbols(document) {
        const data = this.provider.fileData.get(document.uri.fsPath);
        if (!data) return [];
        const scopeEndLine = new Map((data.scopes || []).map(s => [s.id, s.endLine]));
        return data.symbols
            .filter(sym => typeof sym.line === 'number')
            .map(sym => this._toSymbol(document, sym, scopeEndLine));
    }

    _toSymbol(document, sym, scopeEndLine) {
        const kind = completionKindToSymbolKind(sym.kind);
        const selRange = new vscode.Range(sym.line, sym.col, sym.line, sym.col + sym.name.length);

        // endLine real vem do scope (funções/métodos têm um; Entity usa o
        // maior endLine entre seus métodos, senão fica só na própria linha).
        let endLine = sym.line;
        if (sym.fn && scopeEndLine.has(sym.fn.scopeId)) {
            endLine = scopeEndLine.get(sym.fn.scopeId);
        } else if (sym.members && sym.members.length) {
            for (const m of sym.members) {
                if (m.fn && scopeEndLine.has(m.fn.scopeId)) endLine = Math.max(endLine, scopeEndLine.get(m.fn.scopeId));
            }
        }
        endLine = Math.min(Math.max(endLine, sym.line), document.lineCount - 1);
        const fullRange = new vscode.Range(sym.line, 0, endLine, document.lineAt(endLine).text.length);

        const children = (sym.members || [])
            .filter(m => typeof m.line === 'number')
            .map(m => this._toSymbol(document, m, scopeEndLine));

        return new vscode.DocumentSymbol(sym.name, sym.detail || '', kind, fullRange, selRange, children);
    }
}

// ─── WorkspaceSymbolSearchProvider (Ctrl+T / Go to Symbol in Workspace) ─────────
// Nome diferente da classe WorkspaceSymbolProvider (o índice interno acima)
// de propósito — essa aqui é a implementação real da API
// vscode.languages.registerWorkspaceSymbolProvider.
class PoolScriptWorkspaceSymbolSearchProvider {
    constructor(provider) { this.provider = provider; }

    provideWorkspaceSymbols(query) {
        const q = (query || '').toLowerCase();
        const results = [];
        for (const [fsPath, data] of this.provider.fileData) {
            const uri = vscode.Uri.file(fsPath);
            for (const sym of data.symbols) {
                if (typeof sym.line !== 'number') continue;
                if (q && !sym.name.toLowerCase().includes(q)) continue;
                results.push(new vscode.SymbolInformation(
                    sym.name, completionKindToSymbolKind(sym.kind), '',
                    new vscode.Location(uri, new vscode.Position(sym.line, sym.col))));
            }
            for (const sym of data.symbols) {
                for (const m of sym.members || []) {
                    if (typeof m.line !== 'number') continue;
                    if (q && !m.name.toLowerCase().includes(q)) continue;
                    results.push(new vscode.SymbolInformation(
                        m.name, completionKindToSymbolKind(m.kind), sym.name,
                        new vscode.Location(uri, new vscode.Position(m.line, m.col))));
                }
            }
        }
        return results;
    }
}

// ─── DocumentHighlightProvider ──────────────────────────────────────────────────
// Realça toda ocorrência do símbolo sob o cursor NESTE arquivo — mesmo dado
// de `references` já usado por Find All References, só filtrado a 1 arquivo.
class PoolScriptDocumentHighlightProvider {
    constructor(provider) { this.provider = provider; }

    provideDocumentHighlights(document, position) {
        const range = document.getWordRangeAtPosition(position);
        if (!range) return [];
        const word = document.getText(range);
        const data = this.provider.fileData.get(document.uri.fsPath);
        const refs = data && data.references && data.references[word];
        if (!refs) return [];
        return refs.map(r => new vscode.DocumentHighlight(
            new vscode.Range(r.line, r.col, r.line, r.col + word.length)));
    }
}

// ─── FoldingRangeProvider ────────────────────────────────────────────────────────
// PoolScript aceita bloco em dois estilos (ver Language-configuration): `{ }`
// e indentação com `:` (estilo Python). Cobre os dois, ignorando `{`/`}` que
// aparecem dentro de strings/comentários (mesma varredura char-a-char que o
// linter fallback usa pra achar chaves não fechadas).
class PoolScriptFoldingRangeProvider {
    provideFoldingRanges(document) {
        const lines = [];
        for (let i = 0; i < document.lineCount; i++) lines.push(document.lineAt(i).text);
        const ranges = [];

        const stack = [];
        let inStr = null, inTriple = false;
        for (let li = 0; li < lines.length; li++) {
            const ln = lines[li];
            let ci = 0;
            while (ci < ln.length) {
                const ch = ln[ci];
                const ch3 = ln.slice(ci, ci + 3);
                if (inTriple) { if (ch3 === '"""') { inTriple = false; ci += 3; continue; } ci++; continue; }
                if (inStr) { if (ch === inStr && ln[ci - 1] !== '\\') inStr = null; ci++; continue; }
                if (ch3 === '"""') { inTriple = true; ci += 3; continue; }
                if (ch === '"' || ch === "'") { inStr = ch; ci++; continue; }
                if (ch === '/' && ln[ci + 1] === '/') break;
                if (ch === '{') stack.push(li);
                else if (ch === '}') {
                    const start = stack.pop();
                    if (start !== undefined && li > start) ranges.push(new vscode.FoldingRange(start, li, vscode.FoldingRangeKind.Region));
                }
                ci++;
            }
        }

        for (let li = 0; li < lines.length; li++) {
            const raw = lines[li];
            const t = raw.trim();
            if (!t.endsWith(':')) continue;
            const baseIndent = (raw.match(/^( *)/) || ['', ''])[1].length;
            let end = li, j = li + 1;
            while (j < lines.length) {
                const bl = lines[j];
                if (bl.trim() === '') { j++; continue; }
                const indent = (bl.match(/^( *)/) || ['', ''])[1].length;
                if (indent <= baseIndent) break;
                end = j; j++;
            }
            if (end > li) ranges.push(new vscode.FoldingRange(li, end, vscode.FoldingRangeKind.Region));
        }

        return ranges;
    }
}

// ─── RenameProvider (F2) ─────────────────────────────────────────────────────────
// Renomear = substituir TODAS as `references` do nome, no workspace inteiro —
// exatamente o mesmo dado que ReferenceProvider já usa pra Find All References.
class PoolScriptRenameProvider {
    constructor(provider) { this.provider = provider; }

    prepareRename(document, position) {
        const range = document.getWordRangeAtPosition(position);
        if (!range) throw new Error('Nada para renomear aqui.');
        return range;
    }

    provideRenameEdits(document, position, newName) {
        if (!/^[A-Za-z_]\w*$/.test(newName)) {
            throw new Error("Nome inválido — use letras, números e '_', sem começar com número.");
        }
        const range = document.getWordRangeAtPosition(position);
        if (!range) return null;
        const word = document.getText(range);

        const edit = new vscode.WorkspaceEdit();
        let any = false;
        for (const [fsPath, data] of this.provider.fileData) {
            const refs = data.references && data.references[word];
            if (!refs || !refs.length) continue;
            const uri = fsPath === document.uri.fsPath ? document.uri : vscode.Uri.file(fsPath);
            for (const r of refs) {
                any = true;
                edit.replace(uri, new vscode.Range(r.line, r.col, r.line, r.col + word.length), newName);
            }
        }
        return any ? edit : null;
    }
}

// ─── CodeActionProvider (Quick Fixes) ────────────────────────────────────────────
// Corrige os dois tipos de diagnóstico que o PoolScriptLinter já produz:
// import/variável não usados (apaga a linha) e "action sem parênteses"
// (insere '()' vazio) do fallback de sintaxe.
class PoolScriptCodeActionProvider {
    provideCodeActions(document, _range, context) {
        const actions = [];
        for (const diag of context.diagnostics) {
            if (diag.source && diag.source !== 'poolscript') continue;
            if (diag.tags && diag.tags.includes(vscode.DiagnosticTag.Unnecessary)) {
                const isImport = diag.message.includes('importado');
                const action = new vscode.CodeAction(
                    isImport ? "Remover import não usado" : "Remover atribuição não usada",
                    vscode.CodeActionKind.QuickFix);
                action.diagnostics = [diag];
                action.edit = new vscode.WorkspaceEdit();
                action.edit.delete(document.uri, document.lineAt(diag.range.start.line).rangeIncludingLineBreak);
                actions.push(action);
            } else if (diag.message.includes('sem parênteses')) {
                const action = new vscode.CodeAction('Adicionar parênteses vazios', vscode.CodeActionKind.QuickFix);
                action.diagnostics = [diag];
                action.edit = new vscode.WorkspaceEdit();
                const lineEnd = document.lineAt(diag.range.end.line).text.length;
                action.edit.insert(document.uri, new vscode.Position(diag.range.end.line, lineEnd), '()');
                actions.push(action);
            }
        }
        return actions;
    }
}

// ─── CallHierarchyProvider ────────────────────────────────────────────────────────
// "Quem chama isso" / "o que isso chama", construído em cima do mesmo par
// references+scopes: uma chamada "de fora" (incoming) é toda ocorrência do
// nome cujo escopo (findEnclosingScopeId) pertence a OUTRA função; uma
// chamada "de dentro" (outgoing) é toda referência a outro símbolo conhecido
// cuja linha cai dentro do próprio range de escopo do item.
class PoolScriptCallHierarchyProvider {
    constructor(provider) { this.provider = provider; }

    _toItem(uri, name, sym, container) {
        const wordRange = new vscode.Range(sym.line, sym.col, sym.line, sym.col + name.length);
        const kind = container ? vscode.SymbolKind.Method : vscode.SymbolKind.Function;
        const detail = container ? `método de ${container}` : formatFunctionSignature(sym.fn);
        return new vscode.CallHierarchyItem(kind, name, detail, uri, wordRange, wordRange);
    }

    _findCallableAt(document, position) {
        const range = document.getWordRangeAtPosition(position);
        if (!range) return null;
        const word = document.getText(range);
        const data = this.provider.fileData.get(document.uri.fsPath);
        if (!data) return null;
        let sym = data.symbols.find(s => s.name === word && s.fn);
        let container = null;
        if (!sym) {
            for (const s of data.symbols) {
                const m = (s.members || []).find(mm => mm.name === word && mm.fn);
                if (m) { sym = m; container = s.name; break; }
            }
        }
        return sym ? { word, sym, container } : null;
    }

    _ownScope(data, item) {
        return (data.scopes || []).find(s =>
            data.symbols.some(sym =>
                (sym.fn && sym.fn.scopeId === s.id && sym.name === item.name && sym.line === item.range.start.line) ||
                (sym.members || []).some(m => m.fn && m.fn.scopeId === s.id && m.name === item.name && m.line === item.range.start.line)));
    }

    prepareCallHierarchy(document, position) {
        const hit = this._findCallableAt(document, position);
        return hit ? this._toItem(document.uri, hit.word, hit.sym, hit.container) : null;
    }

    provideCallHierarchyIncomingCalls(item) {
        const byCaller = new Map();
        for (const [fsPath, data] of this.provider.fileData) {
            const refs = (data.references && data.references[item.name]) || [];
            for (const r of refs) {
                const isOwnDecl = fsPath === item.uri.fsPath && r.line === item.range.start.line && r.col === item.range.start.character;
                if (isOwnDecl) continue;
                const scopeId = findEnclosingScopeId(data, r.line);
                if (!scopeId) continue; // chamada em nível de módulo — sem "função chamadora" pra listar

                let callerSym = data.symbols.find(s => s.fn && s.fn.scopeId === scopeId);
                let callerContainer = null;
                if (!callerSym) {
                    for (const s of data.symbols) {
                        const m = (s.members || []).find(mm => mm.fn && mm.fn.scopeId === scopeId);
                        if (m) { callerSym = m; callerContainer = s.name; break; }
                    }
                }
                if (!callerSym) continue;

                const key = `${fsPath}::${callerSym.name}::${callerSym.line}`;
                if (!byCaller.has(key)) {
                    byCaller.set(key, { item: this._toItem(vscode.Uri.file(fsPath), callerSym.name, callerSym, callerContainer), ranges: [] });
                }
                byCaller.get(key).ranges.push(new vscode.Range(r.line, r.col, r.line, r.col + item.name.length));
            }
        }
        return [...byCaller.values()].map(({ item: callerItem, ranges }) => new vscode.CallHierarchyIncomingCall(callerItem, ranges));
    }

    provideCallHierarchyOutgoingCalls(item) {
        const data = this.provider.fileData.get(item.uri.fsPath);
        if (!data) return [];
        const ownScope = this._ownScope(data, item);
        if (!ownScope) return [];

        const byCallee = new Map();
        for (const [name, occ] of Object.entries(data.references || {})) {
            for (const r of occ) {
                if (r.line < ownScope.startLine || r.line > ownScope.endLine) continue;
                const isOwnDecl = name === item.name && r.line === item.range.start.line && r.col === item.range.start.character;
                if (isOwnDecl) continue;

                const decl = this.provider.findDeclaration(name, item.uri.fsPath);
                if (!decl) continue;
                const declData = this.provider.fileData.get(decl.fsPath);
                if (!declData) continue;
                let calleeSym = declData.symbols.find(s => s.name === name && s.line === decl.line && s.fn);
                let calleeContainer = null;
                if (!calleeSym) {
                    for (const s of declData.symbols) {
                        const m = (s.members || []).find(mm => mm.name === name && mm.line === decl.line && mm.fn);
                        if (m) { calleeSym = m; calleeContainer = s.name; break; }
                    }
                }
                if (!calleeSym) continue; // referência a variável/campo, não a função — fora da call hierarchy

                const key = `${decl.fsPath}::${name}::${decl.line}`;
                if (!byCallee.has(key)) {
                    byCallee.set(key, { item: this._toItem(vscode.Uri.file(decl.fsPath), name, calleeSym, calleeContainer), ranges: [] });
                }
                byCallee.get(key).ranges.push(new vscode.Range(r.line, r.col, r.line, r.col + name.length));
            }
        }
        return [...byCallee.values()].map(({ item: calleeItem, ranges }) => new vscode.CallHierarchyOutgoingCall(calleeItem, ranges));
    }
}

// ─── InlayHintsProvider (nomes de parâmetro em chamadas) ─────────────────────────
// `foo(1, 2)` → `foo(a: 1, b: 2)`, igual ao que Pylance faz pra chamadas
// Python. Detecção de chamada é por regex linha-a-linha (não tenta resolver
// chamadas que atravessam múltiplas linhas) — suficiente pro caso comum.
class PoolScriptInlayHintsProvider {
    constructor(provider) { this.provider = provider; }

    provideInlayHints(document, range) {
        const data = this.provider.fileData.get(document.uri.fsPath);
        if (!data) return [];
        const hints = [];
        const NOT_CALLS = new Set(['if', 'while', 'for', 'elif', 'action', 'reaction', 'Entity', 'Class', 'class', 'import', 'from', 'model', 'match', 'catch']);

        for (let li = range.start.line; li <= range.end.line && li < document.lineCount; li++) {
            const lineText = document.lineAt(li).text;
            const callRe = /([A-Za-z_]\w*)\s*\(/g;
            let m;
            while ((m = callRe.exec(lineText)) !== null) {
                const calleeName = m[1];
                if (NOT_CALLS.has(calleeName)) continue;
                const sym = this.provider.getSymbolByName(calleeName, document.uri);
                const fn = resolveCallableFn(sym);
                if (!fn || !fn.params || !fn.params.length) continue;
                // pula a própria linha de declaração (`action helper(x, y)`) —
                // ali "x"/"y" são os PARÂMETROS, não uma chamada, então não faz
                // sentido anotar "x: x, y: y". sym.line/col apontam pro token
                // 'action' (início da declaração), não pro nome — comparar só
                // a linha já basta na prática (uma call real ao próprio nome
                // na MESMA linha da sua declaração não ocorre).
                if (sym && typeof sym.line === 'number' && sym.line === li) continue;

                const openParenIdx = m.index + m[0].length - 1;
                const args = this._splitArgs(lineText, openParenIdx);
                args.forEach((arg, idx) => {
                    if (idx >= fn.params.length) return;
                    const trimmed = arg.text.trim();
                    if (!trimmed || /^[A-Za-z_]\w*\s*=[^=]/.test(trimmed)) return; // vazio ou já nomeado
                    const hint = new vscode.InlayHint(
                        new vscode.Position(li, arg.start), `${fn.params[idx]}:`, vscode.InlayHintKind.Parameter);
                    hint.paddingRight = true;
                    hints.push(hint);
                });
            }
        }
        return hints;
    }

    /** Divide os argumentos de uma chamada que abre em `openParenIdx`, respeitando
     * aninhamento de ()/[]/{} e strings — pra `foo(bar(1, 2), 3)` não quebrar
     * a vírgula de dentro de bar(...) como se fosse um argumento de foo. */
    _splitArgs(lineText, openParenIdx) {
        const args = [];
        let depth = 0, start = openParenIdx + 1, inStr = null;
        for (let i = openParenIdx; i < lineText.length; i++) {
            const ch = lineText[i];
            if (inStr) { if (ch === inStr && lineText[i - 1] !== '\\') inStr = null; continue; }
            if (ch === '"' || ch === "'") { inStr = ch; continue; }
            if (ch === '(' || ch === '[' || ch === '{') { depth++; continue; }
            if (ch === ')' || ch === ']' || ch === '}') {
                depth--;
                if (depth === 0 && ch === ')') { args.push({ text: lineText.slice(start, i), start }); return args; }
                continue;
            }
            if (ch === ',' && depth === 1) { args.push({ text: lineText.slice(start, i), start }); start = i + 1; }
        }
        return args;
    }
}

// ─── activate ─────────────────────────────────────────────────────────────────
function activate(context) {
    loadStdlibMetadata();
    const bridge   = new PythonBridge();
    const provider = new WorkspaceSymbolProvider(bridge);
    const linter   = new PoolScriptLinter();
    context.subscriptions.push(linter.collection);
    context.subscriptions.push({ dispose: () => bridge.dispose() });

    // ── Reanálise + lint: imediata em open/save, com debounce em edição ──────
    const debounceTimers = new Map();
    async function reparseAndLint(document, immediate) {
        if (document.languageId !== 'poolscript') return;
        const key = document.uri.fsPath;
        if (debounceTimers.has(key)) {
            clearTimeout(debounceTimers.get(key));
            debounceTimers.delete(key);
        }
        const run = async () => {
            await provider.reparse(document);
            linter.lint(document, provider);
        };
        if (immediate) {
            await run();
        } else {
            debounceTimers.set(key, setTimeout(run, 250));
        }
    }

    vscode.workspace.onDidOpenTextDocument(d => reparseAndLint(d, true), null, context.subscriptions);
    vscode.workspace.onDidSaveTextDocument(d => reparseAndLint(d, true), null, context.subscriptions);
    vscode.workspace.onDidChangeTextDocument(
        e => reparseAndLint(e.document, false), null, context.subscriptions);
    vscode.workspace.textDocuments.forEach(d => reparseAndLint(d, true));

    // ── Autocomplete ──────────────────────────────────────────────────────────
    context.subscriptions.push(
        vscode.languages.registerCompletionItemProvider('poolscript', {
            provideCompletionItems(document, position) {
                const localData = provider.fileData.get(document.uri.fsPath);

                // ── "import X" / "from X import Y" — sugere lib/pasta/arquivo ──
                // Sem isso, escrever "import " não mostrava NADA (o buraco
                // original reportado). Agora também entende import relativo
                // (`from .x` / `from ..pkg.x`, ver parseImportTyped) e navega
                // pastas de verdade — cada "." depois de uma pasta re-dispara
                // completion (já é char de trigger) pra listar o conteúdo dela.
                const importCtx = getImportModuleContext(document, position);
                if (importCtx) {
                    const { level, descend, partial } = importCtx;
                    const items = [];
                    const seenNames = new Set();

                    // stdlib só faz sentido pra import ABSOLUTO de topo (sem
                    // pasta ainda digitada) — igual ao Interpreter, que nunca
                    // resolve import relativo contra a stdlib.
                    if (level === 0 && !descend.length) {
                        for (const name of Object.keys(STDLIB_MEMBERS).sort()) {
                            if (!name.startsWith(partial)) continue;
                            const it = new vscode.CompletionItem(name, vscode.CompletionItemKind.Module);
                            it.detail = 'módulo da stdlib PoolScript';
                            items.push(it);
                            seenNames.add(name);
                        }
                    }

                    let dir = provider._importBaseDir(document.uri.fsPath, level);
                    for (const seg of descend) { if (!dir) break; dir = path.join(dir, seg); }
                    if (dir && fs.existsSync(dir)) {
                        let entries = [];
                        try { entries = fs.readdirSync(dir, { withFileTypes: true }); } catch (_) { /* pasta ilegível — sem sugestões locais */ }
                        for (const entry of entries) {
                            const name = entry.name;
                            if (name.startsWith('.') || name === 'node_modules' || name === '__pycache__') continue;
                            if (!name.startsWith(partial)) continue;
                            if (entry.isDirectory()) {
                                if (seenNames.has(name)) continue;
                                seenNames.add(name);
                                const it = new vscode.CompletionItem(name, vscode.CompletionItemKind.Folder);
                                it.detail = 'pasta — digite "." para entrar';
                                items.push(it);
                            } else if (/\.(ps|psl)$/.test(name)) {
                                if (path.join(dir, name) === document.uri.fsPath) continue; // não sugere o próprio arquivo
                                const base = name.replace(/\.(ps|psl)$/, '');
                                if (seenNames.has(base)) continue;
                                seenNames.add(base);
                                const it = new vscode.CompletionItem(base, vscode.CompletionItemKind.File);
                                it.detail = `módulo local (${name})`;
                                items.push(it);
                            }
                        }
                    }
                    return items;
                }

                const analyzer = new PoolScriptAnalyzer(document, position, provider);
                const access   = analyzer.getMemberAccess();

                if (access) {
                    // 1. módulo stdlib conhecido (ex: date.X, os.X, cors.X) — resolve
                    // também apelidos de import (`import os as o` → o.pathFile(...)).
                    let stdlibKey = resolveStdlibKey(access.obj, localData);
                    if (STDLIB_MEMBERS[stdlibKey]) {
                        // sortText preserva a ordem de declaração da lib (ver
                        // gen_stdlib_metadata.py) — sem isso o VS Code cai no
                        // A-Z padrão, que é o "vem como abc" reportado.
                        return STDLIB_MEMBERS[stdlibKey].map((m, idx) => {
                            const it = new vscode.CompletionItem(m.name, m.kind);
                            it.detail = m.detail;
                            it.sortText = String(idx).padStart(4, '0');
                            return it;
                        });
                    }

                    // 1.5 módulo LOCAL (.ps/.psl próprio do projeto) importado
                    // inteiro — "import minhalib" (ou "as apelido") seguido de
                    // "minhalib." — faltava completamente: só stdlib (tier 1)
                    // e instância de Entity via "from X import Y" tinham
                    // completion; "import X; X.<algo>" não devolvia nada.
                    if (localData) {
                        const modImp = localData.imports.find(i =>
                            i.mode === 'import' && (i.alias || i.module) === access.obj);
                        if (modImp) {
                            const ms = provider._getModuleSymbols(modImp.module, document.uri.fsPath, modImp.level);
                            if (ms) {
                                return ms.map(s => {
                                    const it = new vscode.CompletionItem(s.name, s.kind);
                                    it.detail = s.detail || undefined;
                                    return it;
                                });
                            }
                        }
                    }

                    // 2. variável tipada (instância de Entity, ou objeto devolvido
                    // por uma função da stdlib — WsConnection, Response, ...)
                    const type = analyzer.getVariableType(access.obj);
                    if (type === 'json_dict') {
                        return analyzer.getDictKeys(access.obj).map(k => {
                            const it = new vscode.CompletionItem(k, vscode.CompletionItemKind.Field);
                            it.detail = 'dict key';
                            return it;
                        });
                    }
                    if (type && typeof type === 'object' && type.stdlibClass) {
                        const clsMembers = STDLIB_CLASSES[type.stdlibClass] || [];
                        return clsMembers.map(m => {
                            const it = new vscode.CompletionItem(m.name, m.kind);
                            it.detail = m.detail;
                            return it;
                        });
                    }
                    if (type) {
                        const sym = provider.getSymbolByName(type, document.uri);
                        if (sym?.members?.length) {
                            return sym.members.map(m => {
                                const it = new vscode.CompletionItem(m.name, m.kind);
                                it.detail = m.detail;
                                return it;
                            });
                        }
                    }

                    // 3. tipo não resolvido — retorna null para não suprimir outros providers
                    return null;
                }

                // Sugestões globais, com relevância real (igual ao Pylance): o
                // que está no ESCOPO ATUAL do cursor vem primeiro (sortText por
                // tier), e keywords de início de statement só aparecem quando o
                // cursor realmente está no início de um statement — não em toda
                // posição do arquivo de uma vez só.
                const seen  = new Set();
                const items = [];
                const atStatementStart = isStatementStartContext(document, position);
                const currentScopeId = localData ? findEnclosingScopeId(localData, position.line) : null;

                function addItem(label, kind, detail, tier) {
                    if (seen.has(label)) return;
                    seen.add(label);
                    const it = new vscode.CompletionItem(label, kind);
                    it.detail = detail;
                    it.sortText = `${tier}_${label}`;
                    items.push(it);
                }

                // Tier 0: variáveis visíveis no escopo atual (parâmetros/locais da
                // função onde o cursor está, + variáveis de nível de módulo).
                if (localData) {
                    for (const v of localData.variables) {
                        if (v.scope !== 'module' && v.scope !== currentScopeId) continue; // fora de escopo — não sugere
                        addItem(v.name, vscode.CompletionItemKind.Variable,
                            v.declaredType ? `${v.declaredType} ${v.name}` : 'variável local', '0');
                    }
                }

                // Tier 1: funções/Entities declaradas neste arquivo.
                if (localData) {
                    for (const sym of localData.symbols) {
                        addItem(sym.name, sym.kind, sym.detail, '1');
                    }

                    // Tier 2: nomes importados neste arquivo (`from X import Y` → Y
                    // vira identificador direto; `import X [as A]` → X/A é o módulo).
                    for (const imp of localData.imports) {
                        if (imp.mode === 'import') {
                            const name = imp.alias || imp.module;
                            addItem(name, vscode.CompletionItemKind.Module, `módulo importado: ${imp.module}`, '2');
                        } else {
                            for (const n of imp.names) {
                                const name = n.alias || n.original;
                                const stdMember = (STDLIB_MEMBERS[imp.module] || []).find(m => m.name === n.original);
                                addItem(name, stdMember ? stdMember.kind : vscode.CompletionItemKind.Function,
                                    stdMember ? stdMember.detail : `importado de ${imp.module}`, '2');
                            }
                        }
                    }
                }

                // Tier 3: builtins da linguagem (post, len, addEnd, sleep, str, int, ...).
                for (const b of GLOBAL_BUILTINS) {
                    addItem(b.name, vscode.CompletionItemKind.Function, b.detail, '3');
                }

                // Tier 4: keywords-valor (sempre válidas em posição de expressão).
                const VALUE_KEYWORDS = [
                    { label: 'true',  detail: 'booleano verdadeiro' },
                    { label: 'false', detail: 'booleano falso' },
                    { label: 'null',  detail: 'valor nulo' },
                    { label: 'self',  detail: 'instância atual' },
                ];
                for (const kw of VALUE_KEYWORDS) {
                    addItem(kw.label, vscode.CompletionItemKind.Value, kw.detail, '4');
                }

                // Tier 5: keywords de início de statement — só fazem sentido
                // quando o cursor está mesmo no início de um statement (não no
                // meio de "x = ", dentro de uma chamada, etc.).
                if (atStatementStart) {
                    const STATEMENT_KEYWORDS = [
                      { label: 'action',        detail: 'declaração de função' },
                      { label: 'reaction',      detail: 'alias de action' },
                      { label: 'int action',    detail: 'action com retorno int' },
                      { label: 'bool action',   detail: 'action com retorno bool' },
                      { label: 'int reaction',  detail: 'reaction com retorno int' },
                      { label: 'bool reaction', detail: 'reaction com retorno bool' },
                      { label: 'Entity',        detail: 'declaração de classe' },
                      { label: 'class',         detail: 'alias de Entity' },
                      { label: 'model',         detail: 'validação de estrutura' },
                      { label: 'if',            detail: 'condicional' },
                      { label: 'elif',          detail: 'senão se' },
                      { label: 'else',          detail: 'senão' },
                      { label: 'for each',      detail: 'laço em lista/dict' },
                      { label: 'while',         detail: 'laço condicional' },
                      { label: 'return',        detail: 'retornar valor' },
                      { label: 'try',           detail: 'bloco de tentativa' },
                      { label: 'catch',         detail: 'captura de erro' },
                      { label: 'finally',       detail: 'sempre executa' },
                      { label: 'raise',         detail: 'lançar exceção' },
                      { label: 'import',        detail: 'importar módulo' },
                      { label: 'from',          detail: 'importar de módulo' },
                      { label: 'match',         detail: 'match/case' },
                      { label: 'yield',         detail: 'generator' },
                      { label: 'using',         detail: 'context manager' },
                      { label: 'run_selfwith_', detail: 'ponto de entrada' },
                      { label: 'count each',    detail: 'contar ocorrências' },
                      { label: '@dataentity',   detail: 'Entity com campos tipados' },
                      { label: '@NonNull',      detail: 'barrar Null na entrada' },
                      { label: '@static',       detail: 'método estático' },
                    ];
                    for (const kw of STATEMENT_KEYWORDS) {
                        addItem(kw.label, vscode.CompletionItemKind.Keyword, kw.detail, '5');
                    }

                    // Snippet condicional: só faz sentido se o arquivo já
                    // importa jinker (pressupõe `app`/`cors` existindo, via
                    // "@app.route(...)"). Antes era um snippet ESTÁTICO
                    // (snippets/poolscript.json), que não tem como ser
                    // condicional — aparecia em QUALQUER .ps, mesmo sem
                    // jinker nenhum, com o mesmo nome do método real
                    // Jinker.route(), causando confusão sobre de onde vinha.
                    if (localData && localData.imports.some(i => i.module === 'jinker') && !seen.has('jroute')) {
                        seen.add('jroute');
                        // kind=Method (não Snippet) — "editor.snippetSuggestions: none"
                        // (settings, pra parar de misturar snippet estático fora de
                        // contexto) suprimiria isso também se ficasse como Snippet,
                        // mesmo sendo um item corretamente gated por import real.
                        const it = new vscode.CompletionItem('jroute', vscode.CompletionItemKind.Method);
                        it.detail = 'rota Jinker (@app.route + action) — só aparece com "import jinker" no arquivo';
                        it.insertText = new vscode.SnippetString(
                            '@app.route("/${1:caminho}", auth=cors.permiser(), methods=cors.options(["${2:GET}"])) {\n' +
                            '    action ${3:handler}() {\n' +
                            '        ${0:return jsonify({"msg": "ok"}), 200}\n' +
                            '    }\n' +
                            '}'
                        );
                        it.sortText = '5_jroute';
                        items.push(it);
                    }
                }

                // Tier 6: símbolos/variáveis de arquivos que este arquivo
                // REALMENTE importa (nunca de arquivo não importado — era o
                // bug: antes sugeria de qualquer .ps do workspace).
                if (localData) {
                    for (const fsPath of provider.getImportedFsPaths(localData.imports, document.uri.fsPath)) {
                        const data = provider.fileData.get(fsPath);
                        if (!data) continue;
                        for (const sym of data.symbols) {
                            addItem(sym.name, sym.kind, sym.detail, '6');
                        }
                        for (const v of data.variables) {
                            if (v.scope !== 'module') continue; // só top-level "exporta"
                            addItem(v.name, vscode.CompletionItemKind.Variable,
                                v.declaredType ? `${v.declaredType} ${v.name}` : `variável (${path.basename(fsPath)})`, '6');
                        }
                    }
                }

                return items;
            }
        }, '.', ' ')
    );

    // ── Hover ─────────────────────────────────────────────────────────────────
    context.subscriptions.push(
        vscode.languages.registerHoverProvider('poolscript', {
            provideHover(document, position) {
                const range = document.getWordRangeAtPosition(position);
                if (!range) return null;
                const word     = document.getText(range);
                const analyzer = new PoolScriptAnalyzer(document, position, provider);

                // 1. Símbolo real (function/Entity/import) vindo da bridge
                const sym = provider.getSymbolByName(word, document.uri);
                if (sym) {
                    const md = new vscode.MarkdownString();
                    if (sym.fn) {
                        md.appendCodeblock(formatFunctionSignature(sym.fn), 'poolscript');
                        if (sym.fn.docstring) {
                            md.appendMarkdown(`\n\n${sym.fn.docstring}`);
                        }
                    } else if (sym.kind === vscode.CompletionItemKind.Class) {
                        const parents = sym.entity?.parents?.length ? `(${sym.entity.parents.join(', ')})` : '()';
                        md.appendCodeblock(`Entity ${sym.name}${parents}`, 'poolscript');
                        if (sym.members?.length) {
                            md.appendMarkdown(`\n**Membros:** ${sym.members.map(m => `\`${m.name}\``).join(', ')}\n\n`);
                        }
                        md.appendMarkdown('Instâncias de Entity são sempre **Truthy**.');
                    } else {
                        md.appendMarkdown(`**PoolScript** — \`${sym.name}\``);
                        if (sym.members?.length) {
                            md.appendMarkdown(`\n\nMembros: ${sym.members.map(m => `\`${m.name}\``).join(', ')}`);
                        }
                    }
                    return new vscode.Hover(md);
                }

                // 2. Membro (método/campo) de alguma Entity conhecida
                for (const s of provider.getAllSymbols()) {
                    if (!s.members) continue;
                    const m = s.members.find(mm => mm.name === word);
                    if (m) {
                        const md = new vscode.MarkdownString();
                        md.appendCodeblock(m.detail || m.name, 'poolscript');
                        if (m.fn?.docstring) {
                            md.appendMarkdown(`\n\n${m.fn.docstring}`);
                        }
                        md.appendMarkdown(`\nMembro de \`${s.name}\``);
                        return new vscode.Hover(md);
                    }
                }

                // 3. Fallback: tipo inferido de variável local (comportamento antigo)
                const type = analyzer.getVariableType(word);
                if (!type) return null;
                const md = new vscode.MarkdownString();
                if (typeof type === 'object' && type.stdlibClass) {
                    md.appendMarkdown(`**PoolScript** — \`${type.stdlibClass}\` (stdlib)\n\n`);
                    const clsMembers = STDLIB_CLASSES[type.stdlibClass] || [];
                    for (const m of clsMembers) md.appendMarkdown(`- \`${m.detail}\`\n`);
                    return new vscode.Hover(md);
                }
                md.appendMarkdown(`**PoolScript** — \`${type}\`\n\n`);
                md.appendMarkdown('Instâncias de Entity são sempre **Truthy**.');
                return new vscode.Hover(md);
            }
        })
    );

    // ── Go to Definition ───────────────────────────────────────────────────────
    context.subscriptions.push(
        vscode.languages.registerDefinitionProvider('poolscript', new PoolScriptDefinitionProvider(provider))
    );

    // ── Find All References ────────────────────────────────────────────────────
    context.subscriptions.push(
        vscode.languages.registerReferenceProvider('poolscript', new PoolScriptReferenceProvider(provider))
    );

    // ── Signature Help ─────────────────────────────────────────────────────────
    context.subscriptions.push(
        vscode.languages.registerSignatureHelpProvider(
            'poolscript', new PoolScriptSignatureHelpProvider(provider), '(', ','
        )
    );

    // ── Outline / Breadcrumbs ──────────────────────────────────────────────────
    context.subscriptions.push(
        vscode.languages.registerDocumentSymbolProvider('poolscript', new PoolScriptDocumentSymbolProvider(provider))
    );

    // ── Go to Symbol in Workspace (Ctrl+T) ─────────────────────────────────────
    context.subscriptions.push(
        vscode.languages.registerWorkspaceSymbolProvider(new PoolScriptWorkspaceSymbolSearchProvider(provider))
    );

    // ── Highlight de ocorrências do símbolo sob o cursor ───────────────────────
    context.subscriptions.push(
        vscode.languages.registerDocumentHighlightProvider('poolscript', new PoolScriptDocumentHighlightProvider(provider))
    );

    // ── Folding (blocos { } e blocos indentados por ':') ───────────────────────
    context.subscriptions.push(
        vscode.languages.registerFoldingRangeProvider('poolscript', new PoolScriptFoldingRangeProvider())
    );

    // ── Rename Symbol (F2) ──────────────────────────────────────────────────────
    context.subscriptions.push(
        vscode.languages.registerRenameProvider('poolscript', new PoolScriptRenameProvider(provider))
    );

    // ── Code Actions / Quick Fixes ──────────────────────────────────────────────
    context.subscriptions.push(
        vscode.languages.registerCodeActionsProvider('poolscript', new PoolScriptCodeActionProvider(), {
            providedCodeActionKinds: [vscode.CodeActionKind.QuickFix],
        })
    );

    // ── Call Hierarchy ──────────────────────────────────────────────────────────
    context.subscriptions.push(
        vscode.languages.registerCallHierarchyProvider('poolscript', new PoolScriptCallHierarchyProvider(provider))
    );

    // ── Inlay Hints (nomes de parâmetro em chamadas) ────────────────────────────
    context.subscriptions.push(
        vscode.languages.registerInlayHintsProvider('poolscript', new PoolScriptInlayHintsProvider(provider))
    );
}

module.exports = { activate, deactivate: () => {} };
