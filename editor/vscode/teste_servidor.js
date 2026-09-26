/*
 * Dirige o servidor LSP como o VS Code faria: escreve mensagens enquadradas na
 * entrada dele e confere as respostas. Não precisa de editor nenhum.
 *
 *     node editor/vscode/teste_servidor.js [caminho-do-jinga]
 *
 * CADA CASO AQUI É UMA RECLAMAÇÃO REPRODUZIDA. Não são testes de feature
 * inventados: são os defeitos que o servidor anterior tinha, medidos antes de
 * trocar, cada um com o número que ele dava.
 */
'use strict';
const { spawn } = require('child_process');
const path = require('path');
const fs = require('fs');
const os = require('os');

const JINGA = process.argv[2] || 'jinga';
const SERVIDOR = path.join(__dirname, 'server.js');
/* as tabelas do motor, pros casos dirigidos por tabela (apelidos de tipo…) */
const META_LOCAL = JSON.parse(require('child_process').execFileSync(JINGA, ['--metadata']).toString());

let falhas = 0;
let feitos = 0;

function conf(nome, cond, detalhe) {
  feitos++;
  if (cond) { console.log('  ok    ' + nome); return; }
  falhas++;
  console.log('  FALHOU ' + nome);
  if (detalhe !== undefined) console.log('         ' + JSON.stringify(detalhe).slice(0, 400));
}

/* Conversa com o servidor: manda o roteiro e devolve as respostas por id.
 *
 * ESPERA as respostas antes de encerrar. A primeira versão mandava o roteiro
 * inteiro de uma vez, `exit` incluído, e o servidor saía ANTES de responder —
 * dez checagens "falhavam" por defeito do harness, não do servidor. Testar
 * cliente de protocolo sem esperar a resposta é medir o relógio, não o
 * servidor. */
/* `op` (opcional) é o que muda de cliente pra cliente: `uri` do documento
 * aberto, `inicializa` (os params inteiros do `initialize`, no lugar dos do
 * VS Code) e `env` do processo do servidor. */
function conversa(texto, pedidos, op) {
  op = op || {};
  const uriDoc = op.uri || URI;
  return new Promise((resolve, reject) => {
    const p = spawn(process.execPath, [SERVIDOR, '--stdio'],
                    { stdio: ['pipe', 'pipe', 'pipe'], env: op.env || process.env });
    let buf = Buffer.alloc(0);
    const msgs = [];
    const esperados = new Set(pedidos.map((q) => q.id).filter((x) => x !== undefined));
    let terminou = false;
    /* Sem pedido nenhum, o caso está esperando o DIAGNÓSTICO — e é ele que
     * diz quando pode encerrar, em vez de um relógio. */
    const esperaDiagnostico = pedidos.length === 0;
    let viuDiagnostico = false;

    const encerra = () => {
      if (terminou) return;
      terminou = true;
      manda({ jsonrpc: '2.0', id: 99, method: 'shutdown', params: null });
      manda({ jsonrpc: '2.0', method: 'exit', params: null });
      setTimeout(() => { try { p.kill(); } catch (_) {} resolve(msgs); }, 250);
    };

    const manda = (o) => {
      const c = Buffer.from(JSON.stringify(o), 'utf8');
      p.stdin.write(`Content-Length: ${c.length}\r\n\r\n`);
      p.stdin.write(c);
    };

    p.stdout.on('data', (d) => {
      buf = Buffer.concat([buf, d]);
      for (;;) {
        const s = buf.indexOf('\r\n\r\n');
        if (s < 0) break;
        const cab = buf.slice(0, s).toString('utf8');
        /* sem regex, como o resto dos .js: acha o cabeçalho e lê o número */
        const k = cab.toLowerCase().indexOf('content-length:');
        if (k < 0) break;
        const n = parseInt(cab.slice(k + 'content-length:'.length).trim(), 10);
        if (!(n >= 0)) break;
        if (buf.length < s + 4 + n) break;
        const msg = JSON.parse(buf.slice(s + 4, s + 4 + n).toString('utf8'));
        msgs.push(msg);
        if (msg.id !== undefined) esperados.delete(msg.id);
        if (msg.method === 'textDocument/publishDiagnostics') viuDiagnostico = true;
        buf = buf.slice(s + 4 + n);
      }
      /* Tudo respondido E o diagnóstico já veio: pode encerrar.
       *
       * Antes era um `setTimeout` de 400/600 ms torcendo pra chegar a tempo, e
       * sob carga (o `make check` inteiro rodando) o `--check` do motor não
       * voltava dentro da janela: duas checagens FALHAVAM sem nada estar
       * quebrado. Teste que chora lobo ensina a ignorar teste. */
      if (esperados.size === 0 && (viuDiagnostico || !esperaDiagnostico) && !terminou)
        setTimeout(encerra, 60);
    });
    p.on('error', reject);

    manda({ jsonrpc: '2.0', id: 1, method: 'initialize',
            params: op.inicializa || { rootUri: null, capabilities: {}, initializationOptions: { jinga: JINGA } } });
    manda({ jsonrpc: '2.0', method: 'initialized', params: {} });
    manda({ jsonrpc: '2.0', method: 'textDocument/didOpen',
            params: { textDocument: { uri: uriDoc, languageId: 'jinga', version: 1, text: texto } } });
    for (const q of pedidos) manda(q);
    /* Sem pedido nenhum, o que se espera é o DIAGNÓSTICO. O teto de 20 s
     * abaixo continua sendo a rede: se ele nunca vier, o caso falha por não
     * ter vindo — não por ter demorado 601 ms numa máquina ocupada. */
    setTimeout(() => { try { p.kill(); } catch (_) {} resolve(msgs); }, 20000);
  });
}

const URI = 'file://' + path.join(os.tmpdir(), 'ps_lsp_t', 'a.pr');

/* As libs instaladas: `~/.jinga/libs`, ou `~/.poolscript/libs` enquanto o
 * `jpkg` não moveu a pasta antiga — o mesmo fallback do motor e do servidor. */
function pastaLibs() {
  if (!process.env.HOME) return '';
  const nova = path.join(process.env.HOME, '.jinga', 'libs');
  try { if (fs.statSync(nova).isDirectory()) return nova; } catch (_) { /* segue */ }
  return path.join(process.env.HOME, '.poolscript', 'libs');
}


const resp = (msgs, id) => msgs.find((m) => m.id === id);
const rotulos = (m) => {
  if (!m || !m.result) return [];
  const its = Array.isArray(m.result) ? m.result : (m.result.items || []);
  return its.map((i) => i.label);
};

const compl = (id, l, c, uri) => ({ jsonrpc: '2.0', id, method: 'textDocument/completion',
  params: { textDocument: { uri: uri || URI }, position: { line: l, character: c } } });

async function main() {
  fs.mkdirSync(path.join(os.tmpdir(), 'ps_lsp_t'), { recursive: true });

  /* ── 1. `import ... as` — dava ZERO sugestão ───────────────────────────── */
  {
    const m = await conversa('import json as js\nx = js.\n', [compl(2, 1, 7)]);
    const L = rotulos(resp(m, 2));
    conf('`import json as js` -> `js.` sugere os membros do json', L.includes('parse') && L.includes('stringify'), L);
  }

  /* ── 2. lib instalada em ~/.jinga/libs — dava ZERO ─────────────────────── */
  {
    const libs = pastaLibs();
    let alguma = '';
    try {
      const f = fs.readdirSync(libs).find((x) => x.endsWith('.pr')) || '';
      alguma = f.slice(0, f.length - '.pr'.length);
    } catch (_) { /* sem libs instaladas */ }
    if (!alguma) {
      console.log('  PULOU lib instalada — nenhuma em ~/.jinga/libs');
    } else {
      const src = `import ${alguma}\nx = ${alguma}.\n`;
      const m = await conversa(src, [compl(2, 1, 5 + alguma.length)]);
      const L = rotulos(resp(m, 2));
      conf(`lib instalada \`${alguma}\` expoe membros`, L.length > 0, L.slice(0, 6));

      /* a forma qualificada liga o MESMO nome e acha o mesmo arquivo */
      const srcQ = `import jinga.libs.${alguma}\nx = ${alguma}.\n`;
      const mQ = await conversa(srcQ, [compl(2, 1, 5 + alguma.length)]);
      const LQ = rotulos(resp(mQ, 2));
      conf(`\`import jinga.libs.${alguma}\` expoe os mesmos membros`,
           LQ.length > 0 && LQ.length === L.length, { qualificado: LQ.slice(0, 6), curto: L.slice(0, 6) });
    }
  }

  /* ── 3. parêntese DENTRO DE STRING quebrava o detector ─────────────────── */
  {
    const src = 'import regex\nregex.sub("(", ';
    const m = await conversa(src, [compl(2, 1, 15)]);
    const L = rotulos(resp(m, 2));
    conf('parentese dentro de string nao confunde a chamada',
         L.length > 0 && L.every((x) => x.endsWith('=')), L);
  }

  /* ── 4. função LOCAL não oferecia parâmetro nenhum ─────────────────────── */
  {
    const src = 'funct soma(a, b=2) {\n    return a\n}\nsoma(';
    const m = await conversa(src, [compl(2, 3, 5)]);
    const L = rotulos(resp(m, 2));
    conf('funct LOCAL oferece os parametros dela', L.includes('a=') && L.includes('b='), L);
  }

  /* ── 5. reoferecia parâmetro JÁ PASSADO ────────────────────────────────── */
  {
    const src = 'import regex\nregex.sub(pattern="a", ';
    const m = await conversa(src, [compl(2, 1, 22)]);
    const L = rotulos(resp(m, 2));
    conf('parametro ja passado por NOME nao volta', !L.includes('pattern='), L);
  }
  {
    const src = 'import regex\nregex.sub("a", "b", ';
    const m = await conversa(src, [compl(2, 1, 20)]);
    const L = rotulos(resp(m, 2));
    conf('dois posicionais ja dados: sobram os de tras',
         !L.includes('pattern=') && !L.includes('repl=') && L.includes('string='), L);
  }

  /* ── 6. completava DENTRO de comentário e de string ────────────────────── */
  {
    const m = await conversa('# uma nota qualquer\n', [compl(2, 0, 12)]);
    conf('dentro de COMENTARIO nao sugere nada', rotulos(resp(m, 2)).length === 0, rotulos(resp(m, 2)).slice(0, 5));
  }
  {
    const m = await conversa('x = "texto aqui"\n', [compl(2, 0, 10)]);
    conf('dentro de STRING nao sugere nada', rotulos(resp(m, 2)).length === 0, rotulos(resp(m, 2)).slice(0, 5));
  }

  /* ── 7. o que NÃO existia: outline, definição, assinatura ──────────────── */
  {
    const src = 'funct alfa(x) {\n    return x\n}\nfunct beta() {\n    return 1\n}\n';
    const m = await conversa(src, [
      { jsonrpc: '2.0', id: 3, method: 'textDocument/documentSymbol', params: { textDocument: { uri: URI } } },
      { jsonrpc: '2.0', id: 4, method: 'textDocument/definition',
        params: { textDocument: { uri: URI }, position: { line: 0, character: 8 } } },
    ]);
    const ds = resp(m, 3);
    const nomes = ds && ds.result ? ds.result.map((s) => s.name) : [];
    conf('documentSymbol lista as functs do arquivo', nomes.includes('alfa') && nomes.includes('beta'), nomes);
    const df = resp(m, 4);
    conf('definition responde (nao e mais -32601)', !!df && !df.error, df);
  }
  {
    const src = 'import regex\nregex.sub("a", ';
    const m = await conversa(src, [
      { jsonrpc: '2.0', id: 5, method: 'textDocument/signatureHelp',
        params: { textDocument: { uri: URI }, position: { line: 1, character: 15 } } },
    ]);
    const sh = resp(m, 5);
    const lbl = sh && sh.result && sh.result.signatures[0] ? sh.result.signatures[0].label : '';
    conf('signatureHelp mostra a assinatura da chamada', lbl.startsWith('regex.sub('), lbl);
    conf('signatureHelp aponta o parametro ATUAL',
         sh && sh.result && sh.result.activeParameter === 1, sh && sh.result && sh.result.activeParameter);
  }
  /* Parametro tipado: o tipo vem antes do nome no fonte, e a assinatura que o
   * editor mostra tem que sair na MESMA ordem — `str corpo`, nao `corpo`. */
  {
    const src = 'funct webhookDoctor(str corpo, int n = 2) {\n    return corpo * n\n}\n'
              + 'webhookDoctor("a")\n'
              + 'webhookDoctor(';
    const m = await conversa(src, [
      { jsonrpc: '2.0', id: 6, method: 'textDocument/signatureHelp',
        params: { textDocument: { uri: URI }, position: { line: 4, character: 14 } } },
      { jsonrpc: '2.0', id: 7, method: 'textDocument/hover',
        params: { textDocument: { uri: URI }, position: { line: 3, character: 4 } } },
    ]);
    const sh = resp(m, 6);
    const lbl = sh && sh.result && sh.result.signatures[0] ? sh.result.signatures[0].label : '';
    conf('signatureHelp mostra o TIPO antes do nome do parametro',
         lbl.includes('str corpo') && lbl.includes('int n'), lbl);
    const hv = resp(m, 7);
    const txt = hv && hv.result && hv.result.contents
      ? (hv.result.contents.value || String(hv.result.contents)) : '';
    conf('hover da funct tipada mostra o tipo do parametro', txt.includes('str corpo'), txt);
  }
  /* `base(A).__init__(` como ULTIMO comando do metodo, com o `}` logo abaixo:
   * a assinatura e a do __init__ do pai. Antes o remendo fechava o `(` no fim
   * do arquivo e o lexer nao sincronizava no `}` menos indentado — nenhuma
   * chamada aberta no fim de um metodo tinha assinatura. */
  {
    const src = 'Entity A {\n    funct __init__(self, x, y) { self.x = x }\n}\n'
              + 'Entity B(A) {\n    funct __init__(self) {\n        base(A).__init__(\n    }\n}\n'
              + 'funct f(a, b) { return 1 }\nfunct g() {\n    f(\n}\n';
    const m = await conversa(src, [
      { jsonrpc: '2.0', id: 8, method: 'textDocument/signatureHelp',
        params: { textDocument: { uri: URI }, position: { line: 5, character: 25 } } },
      { jsonrpc: '2.0', id: 9, method: 'textDocument/signatureHelp',
        params: { textDocument: { uri: URI }, position: { line: 10, character: 6 } } },
      { jsonrpc: '2.0', id: 10, method: 'textDocument/hover',
        params: { textDocument: { uri: URI }, position: { line: 5, character: 9 } } },
    ]);
    const s8 = resp(m, 8);
    const l8 = s8 && s8.result && s8.result.signatures[0] ? s8.result.signatures[0].label : '';
    conf('signatureHelp de `base(A).__init__(` mostra os parametros do __init__ do pai',
         l8.includes('x') && l8.includes('y'), l8);
    const s9 = resp(m, 9);
    const l9 = s9 && s9.result && s9.result.signatures[0] ? s9.result.signatures[0].label : '';
    conf('signatureHelp de `f(` como ultimo comando do metodo (o `}` logo abaixo)',
         l9.startsWith('f('), l9);
    const h10 = resp(m, 10);
    const t10 = h10 && h10.result && h10.result.contents ? (h10.result.contents.value || '') : '';
    conf('hover em `base` explica o pai (secao 7.5.2)', t10.includes('base(Pai)') && t10.includes('pai'), t10.slice(0, 120));
  }
  /* O hover de um VALOR apresenta o tipo dele: o que e, quantos membros, o
   * resumo e o primeiro exemplo da pagina da doc (ele, 2026-09-26: "o hover
   * deve mostrar que e uma classe, e um exemplo interno dela, e isso deve
   * valer para tudo"); apelido de tipo (`String`, `Integer`...) responde
   * como o tipo — antes ficava mudo. A lista de apelidos vem do motor. */
  {
    const src = 'from jinker import Jinker\nnome = Jinker(__name__)\npost(nome)\n'
              + 'n = 1\npost(n)\n'
              + 'Entity Osx {\n    int x\n    funct dobra(self) { return self.x * 2 }\n}\no = Osx(1)\npost(o)\n'
              + 'String s = "a"\npost(s)\n'
              + 'import os\np = os.run(["ls"], capture="live")\npost(p)\n';
    const m = await conversa(src, [
      { jsonrpc: '2.0', id: 12, method: 'textDocument/hover', params: { textDocument: { uri: URI }, position: { line: 2, character: 6 } } },
      { jsonrpc: '2.0', id: 13, method: 'textDocument/hover', params: { textDocument: { uri: URI }, position: { line: 4, character: 6 } } },
      { jsonrpc: '2.0', id: 14, method: 'textDocument/hover', params: { textDocument: { uri: URI }, position: { line: 10, character: 6 } } },
      { jsonrpc: '2.0', id: 15, method: 'textDocument/hover', params: { textDocument: { uri: URI }, position: { line: 11, character: 2 } } },
      { jsonrpc: '2.0', id: 16, method: 'textDocument/hover', params: { textDocument: { uri: URI }, position: { line: 15, character: 6 } } },
    ]);
    const txt = (id) => { const h = resp(m, id); return h && h.result && h.result.contents ? (h.result.contents.value || '') : ''; };
    conf('hover de `nome = Jinker(...)` apresenta o tipo Jinker com a pagina e o exemplo',
         txt(12).includes('Jinker nome') && txt(12).includes('`Jinker` · tipo') && txt(12).includes('from jinker import Jinker'),
         txt(12).slice(0, 200));
    conf('hover de `n = 1` diz `int n` (escalar: so type())',
         txt(13).includes('int n') && txt(13).includes('só `type()`'), txt(13).slice(0, 160));
    conf('hover de instancia de Entity do arquivo mostra a classe e os membros',
         txt(14).includes('Osx o') && txt(14).includes('class Osx {') && txt(14).includes('funct dobra('), txt(14).slice(0, 200));
    conf('hover no apelido `String` responde como o tipo str',
         txt(15).includes('String = str') && txt(15).includes('`str` · tipo'), txt(15).slice(0, 160));
    conf('hover de retorno com mais de um tipo diz cada lado',
         txt(16).includes('str|Process') && txt(16).includes('`Process`'), txt(16).slice(0, 200));
    /* todos os apelidos que o motor publica, nao dois */
    const apelidos = Object.keys(META_LOCAL.tipos_apelidos || {});
    let ok = 0;
    for (const ap of apelidos) {
      const r = await conversa(ap + ' v = Null\n', [
        { jsonrpc: '2.0', id: 17, method: 'textDocument/hover', params: { textDocument: { uri: URI }, position: { line: 0, character: 1 } } },
      ]);
      const h = resp(r, 17); const t = h && h.result && h.result.contents ? (h.result.contents.value || '') : '';
      if (t.includes(ap + ' = ' + META_LOCAL.tipos_apelidos[ap])) ok++;
    }
    conf('hover responde em TODOS os ' + apelidos.length + ' apelidos de tipo do motor', ok === apelidos.length, { ok, apelidos });
  }
  /* hover no campo sem tipo criado no __init__: o tipo inferido, e SEM
   * "herdado de" quando o campo e da propria classe */
  {
    const src = 'import psodbc\nEntity Db {\n    funct __init__(self) {\n        self.con = psodbc.connect("x")\n    }\n'
              + '    funct q(self) {\n        c = self.con.cursor()\n    }\n}\n';
    const m = await conversa(src, [
      { jsonrpc: '2.0', id: 11, method: 'textDocument/hover',
        params: { textDocument: { uri: URI }, position: { line: 6, character: 17 } } },
    ]);
    const h = resp(m, 11);
    const t = h && h.result && h.result.contents ? (h.result.contents.value || '') : '';
    conf('hover de `self.con` (campo sem tipo) mostra o tipo inferido e nao diz "herdado de"',
         t.includes('DbConnection') && t.includes(' con') && t.includes('Db.') && !t.includes('herdado de'),
         t.slice(0, 160));
  }
  /* Variadico: `*args`/`**kwarg` levam a estrela na assinatura que o editor
   * mostra — sem ela, `route(caminho, kwarg)` mentiria sobre como chamar. */
  {
    const src = 'funct route(caminho, *args, **kwarg) {\n    return kwarg\n}\n'
              + 'route("/x")\n'
              + 'route(';
    const m = await conversa(src, [
      { jsonrpc: '2.0', id: 8, method: 'textDocument/signatureHelp',
        params: { textDocument: { uri: URI }, position: { line: 4, character: 6 } } },
      { jsonrpc: '2.0', id: 9, method: 'textDocument/hover',
        params: { textDocument: { uri: URI }, position: { line: 3, character: 2 } } },
    ]);
    const sh = resp(m, 8);
    const lbl = sh && sh.result && sh.result.signatures[0] ? sh.result.signatures[0].label : '';
    conf('signatureHelp mostra *args e **kwarg com a estrela',
         lbl.includes('*args') && lbl.includes('**kwarg'), lbl);
    const hv = resp(m, 9);
    const txt = hv && hv.result && hv.result.contents
      ? (hv.result.contents.value || String(hv.result.contents)) : '';
    conf('hover da funct variadica mostra **kwarg', txt.includes('**kwarg'), txt);
  }
  /* Revisao de 236eba8 (2026-09-14): tres furos do editor com variadico,
   * medidos VERMELHOS antes de entrar aqui. `activeParameter` e
   * `min(posicionais, ps.length - 1)` e `faltam` filtra por indice — nenhum
   * dos dois sabe que `*args` absorve todo posicional excedente, que
   * `*args=`/`**kwarg=` nao existem como nomeado (o motor recusa com
   * SyntaxError) nem que `**d` na chamada e nomeado, nao posicional. */
  {
    /* 2o posicional excedente: o parametro ativo tem que FICAR no `*args`;
     * hoje pula pro `**kwarg` (activeParameter 2). */
    const ult = 'route("/x", 1, 2, ';
    const src = 'funct route(caminho, *args, **kwarg) {\n    return kwarg\n}\n' + ult;
    const m = await conversa(src, [
      { jsonrpc: '2.0', id: 8, method: 'textDocument/signatureHelp',
        params: { textDocument: { uri: URI }, position: { line: 3, character: ult.length } } },
    ]);
    const sh = resp(m, 8);
    const ativo = sh && sh.result ? sh.result.activeParameter : -1;
    conf('signatureHelp: do 2o posicional excedente em diante o ativo continua no *args',
         ativo === 1, { activeParameter: ativo });
  }
  {
    /* completion de nomeado: `*args=` e `**kwarg=` sao SyntaxError no motor,
     * nao podem ser oferecidos; hoje sao os dois unicos itens. */
    const ult = 'route("/x", ';
    const src = 'funct route(caminho, *args, **kwarg) {\n    return kwarg\n}\n' + ult;
    const m = await conversa(src, [compl(9, 3, ult.length)]);
    const L = rotulos(resp(m, 9));
    conf('completion de nomeado nao oferece *args= nem **kwarg=',
         !L.includes('*args=') && !L.includes('**kwarg='), L.slice(0, 6));
  }
  {
    /* `**d` na chamada e nomeado, nao posicional: depois de `f(1, **d, ` o
     * ativo e o `b` (indice 1) e o nomeado que falta inclui `b=`; hoje o
     * `**d` conta como 2o posicional e o `b` some dos dois. */
    const ult = 'f(1, **d, ';
    const src = 'funct f(a, b, c) {\n    return a\n}\nd = {"c": 1}\n' + ult;
    const m = await conversa(src, [
      { jsonrpc: '2.0', id: 10, method: 'textDocument/signatureHelp',
        params: { textDocument: { uri: URI }, position: { line: 4, character: ult.length } } },
      compl(11, 4, ult.length),
    ]);
    const sh = resp(m, 10);
    const ativo = sh && sh.result ? sh.result.activeParameter : -1;
    conf('signatureHelp: **d na chamada nao conta como posicional (ativo fica no b)',
         ativo === 1, { activeParameter: ativo });
    const L = rotulos(resp(m, 11));
    conf('completion de nomeado depois de **d ainda oferece b=', L.includes('b=') && L.includes('c='), L.slice(0, 6));
  }

  /* ── 8. diagnóstico vem do `--check` do motor ──────────────────────────── */
  {
    const m = await conversa('funct f(:\n    return 1\n', []);
    const d = m.find((x) => x.method === 'textDocument/publishDiagnostics');
    conf('erro de sintaxe vira diagnostico', !!d && d.params.diagnostics.length > 0,
         d && d.params.diagnostics[0] && d.params.diagnostics[0].message);
  }
  {
    const m = await conversa('post(1)\n', []);
    const ds = m.filter((x) => x.method === 'textDocument/publishDiagnostics');
    const ultimo = ds[ds.length - 1];
    conf('programa valido nao gera diagnostico', !ultimo || ultimo.params.diagnostics.length === 0,
         ultimo && ultimo.params.diagnostics);
  }
  /* O `--check` diz onde o trecho acusado TERMINA (`l2`/`c2`): o sublinhado
   * cobre a expressao inteira (`"abc"`, colunas 9-13), nao um caractere. */
  {
    const m = await conversa('int x = "abc"\n', []);
    const ds = m.filter((x) => x.method === 'textDocument/publishDiagnostics');
    const lista = ds.length ? ds[ds.length - 1].params.diagnostics : [];
    const r = lista[0] && lista[0].range;
    conf('diagnostico cobre o trecho inteiro que o motor acusou (l2/c2 do --check)',
         !!r && r.start.line === 0 && r.start.character === 8 && r.end.line === 0 && r.end.character === 13,
         r);
  }
  /* A tipagem estática acha TODOS os erros do arquivo antes de rodar e o
   * `--check` os lista em `erros`: cada um vira um diagnóstico, na linha dele. */
  {
    const m = await conversa('str s = 10\nint n = "a"\n', []);
    const ds = m.filter((x) => x.method === 'textDocument/publishDiagnostics');
    const ultimo = ds[ds.length - 1];
    const lista = ultimo ? ultimo.params.diagnostics : [];
    conf('todos os erros de tipo do arquivo viram diagnostico, um por linha',
         lista.length === 2 && lista[0].message.includes('variável s esperava str')
           && lista[0].range.start.line === 0
           && lista[1].message.includes('variável n esperava int')
           && lista[1].range.start.line === 1,
         lista.map((d) => d.range.start.line + ': ' + d.message));
  }
  /* Método sem `self`: o erro é da DECLARAÇÃO (a causa), não do primeiro
   * `self` do corpo (o sintoma) — o sublinhado cai na linha do `funct`, e é
   * um só (o motor não repete o defeito em cascata). */
  {
    const m = await conversa('class C {\n    funct __init__(self) {\n        self.x = 1\n    }\n'
                             + '    funct m() {\n        return self.x\n    }\n}\nC().m()\n', []);
    const ds = m.filter((x) => x.method === 'textDocument/publishDiagnostics');
    const ultimo = ds[ds.length - 1];
    const lista = ultimo ? ultimo.params.diagnostics : [];
    conf('metodo sem self: um diagnostico so, na linha da declaracao',
         lista.length === 1 && lista[0].message.includes('sem self') && lista[0].range.start.line === 4,
         lista.map((d) => d.range.start.line + ': ' + d.message));
  }

  /* O buffer não salvo é conferido COMO SE fosse o arquivo dele (`--check
   * --path <caminho>`): sem o caminho, o motor não sabe onde o arquivo mora,
   * não acha o módulo vizinho e pula a conferência entre arquivos CALADO —
   * `vizinho.naoexiste()` de um `import vizinho` ao lado não aparecia. */
  {
    const dir = path.join(os.tmpdir(), 'ps_lsp_t');
    fs.writeFileSync(path.join(dir, 'vizinho_t.pr'), 'funct soma(a, b)\n{\n    return a + b\n}\n');
    const m = await conversa('import vizinho_t\nvizinho_t.naoexiste()\n', []);
    const ds = m.filter((x) => x.method === 'textDocument/publishDiagnostics');
    const lista = ds.length ? ds[ds.length - 1].params.diagnostics : [];
    conf('erro de membro em modulo vizinho aparece no buffer aberto',
         lista.length === 1 && lista[0].message.includes('naoexiste')
           && lista[0].range.start.line === 1,
         lista.map((d) => d.range.start.line + ': ' + d.message));
  }
  {
    const m = await conversa('import vizinho_t\npost(vizinho_t.soma(1, 2))\n', []);
    const ds = m.filter((x) => x.method === 'textDocument/publishDiagnostics');
    const lista = ds.length ? ds[ds.length - 1].params.diagnostics : [];
    conf('import de modulo vizinho correto nao gera diagnostico', lista.length === 0, lista);
  }
  /* Módulo que não existe: o `--check` acusa com a frase do runtime, então o
   * typo no import passa a aparecer no editor em vez de só ao rodar. */
  {
    const m = await conversa('import vizinho_zz_nao_existe\npost(1)\n', []);
    const ds = m.filter((x) => x.method === 'textDocument/publishDiagnostics');
    const lista = ds.length ? ds[ds.length - 1].params.diagnostics : [];
    conf('import de modulo inexistente vira diagnostico',
         lista.length === 1 && lista[0].message.includes('No module named')
           && lista[0].range.start.line === 0,
         lista.map((d) => d.range.start.line + ': ' + d.message));
  }

  /* ── 8b. retorno com MAIS DE UM tipo ────────────────────────────────────
   *
   * `os.run` devolve int (pid), str (saída) ou Process (`capture="live"`).
   * Qualquer união virava "desconhecido" e o completion caía nos universais:
   * `p.` oferecia só `type`. Agora vêm os membros dos lados que são tipo. */
  {
    const ult = 'p.';
    const m = await conversa('import os\np = os.run(["cat"], capture="live")\n' + ult,
                             [compl(60, 2, ult.length)]);
    const L = rotulos(resp(m, 60));
    conf('completion do processo vivo (uniao int|str|Process)',
         L.includes('write') && L.includes('read') && L.includes('wait')
           && L.includes('pid') && L.includes('returncode'),
         L.slice(0, 12));
  }
  {
    /* o lado `str` da mesma união continua aparecendo: os dois são oferecidos */
    const ult = 'p.';
    const m = await conversa('import os\np = os.run(["cat"], capture="live")\n' + ult,
                             [compl(61, 2, ult.length)]);
    const L = rotulos(resp(m, 61));
    conf('a uniao nao esconde o outro lado (metodos de str juntos)',
         L.includes('upper') && L.includes('strip'), L.slice(0, 12));
  }

  /* ── 8c. o arquivo PELA METADE (é o estado normal de quem digita) ───────
   *
   * Com o parser parando no primeiro erro, a árvore acabava ali e tudo que
   * vinha depois sumia: a variável de cima não era mais conhecida. Agora o
   * statement quebrado é registrado e o parser segue. */
  {
    const ult = 'nome.';
    const m = await conversa('nome = "ana"\nif y > 0 \npost(1)\n' + ult,
                             [compl(62, 3, ult.length)]);
    const L = rotulos(resp(m, 62));
    conf('completion embaixo de linha quebrada ainda conhece a variavel',
         L.includes('upper') && L.includes('strip'), L.slice(0, 8));
  }
  {
    /* string sem fechar: o lexer inteiro morria e o realce/completion iam junto */
    const ult = 'nome.';
    const m = await conversa('nome = "ana"\nx = "sem fechar\n' + ult,
                             [compl(63, 2, ult.length)]);
    const L = rotulos(resp(m, 63));
    conf('completion depois de string sem fechar ainda funciona',
         L.includes('upper'), L.slice(0, 8));
  }

  /* ── 9. AVISO (nao erro) vira sublinhado amarelo ───────────────────────── */
  {
    const m = await conversa('x = "C:\\pasta"\n', []);
    const ds = m.filter((x) => x.method === 'textDocument/publishDiagnostics');
    const ultimo = ds[ds.length - 1];
    const d = ultimo && ultimo.params.diagnostics[0];
    conf('escape invalido vira AVISO (severity 2), nao erro',
         !!d && d.severity === 2 && d.message.includes('escape invalida'), d);
  }

  /* ── 9b. A FORMA REAL do arquivo dele: `from jinker import Jinker` ───────
   *
   * Medido antes: `mapping = Jinker(__name__)` + `mapping.` dava ZERO (o
   * servidor só resolvia `jinker.Jinker(...)`), `@mapping.` idem — e o VS
   * Code, recebendo [], caía nas palavras soltas do arquivo ("itens nada a
   * ver"). Hover em palavra-chave devolvia null (nunca perguntava ao lexer
   * que token era); em variável saía "mapping / variavel"; `jsonify` saía
   * "import jinker / 6 membros"; `request.get` saía sem prosa. */
  const DELE = [
    'from jinker import Jinker, jsonify, request, cors',
    'mapping = Jinker(__name__)',
    'model Rota() {',
    '    email: str(length=60)',
    '    senha: str',
    '}',
    '@mapping.post("/x", model=Rota)',
    'int async funct entra(data) {',
    '    if data == "" {',
    '        return jsonify({"ok": false})',
    '    }',
    '    for each it in [1, 2] {',
    '        post(it)',
    '    }',
    '    return jsonify({"ok": true, "e": request.get("email")})',
    '}',
    'mapping.',
    '@mapping.',
  ];
  const DELE_SRC = DELE.join('\n') + '\n';
  const hov = (id, line, ch) => ({ jsonrpc: '2.0', id, method: 'textDocument/hover',
    params: { textDocument: { uri: URI }, position: { line, character: ch } } });
  const valor = (m, id) => {
    const r = resp(m, id);
    return r && r.result && r.result.contents ? r.result.contents.value : '';
  };
  const col = (line, trecho) => DELE[line].indexOf(trecho);
  {
    const m = await conversa(DELE_SRC, [compl(2, 16, 8), compl(3, 17, 9)]);
    const L1 = rotulos(resp(m, 2));
    conf('`from jinker import Jinker` + `mapping = Jinker()` -> `mapping.` sugere os membros',
         L1.includes('post') && L1.includes('route') && L1.includes('get'), L1);
    const L2 = rotulos(resp(m, 3));
    conf('`@mapping.` (posicao de decorador) sugere os mesmos membros',
         L2.includes('post') && L2.includes('route'), L2);
  }
  {
    const m = await conversa(DELE_SRC, [
      hov(10, 8, col(8, 'if')),                 hov(11, 11, col(11, 'for')),
      hov(12, 9, col(9, 'return')),             hov(13, 7, col(7, 'funct')),
      hov(14, 12, col(12, 'post')),             hov(15, 1, 0),
      hov(16, 7, col(7, 'data')),               hov(17, 7, col(7, 'entra')),
      hov(18, 2, col(2, 'Rota')),               hov(19, 9, col(9, 'jsonify')),
      hov(20, 14, col(14, 'request.get') + 'request.'.length),
      hov(21, 6, col(6, 'post')),
    ]);
    conf('hover em `if` traz a secao da doc da linguagem', valor(m, 10).includes('Condicional'), valor(m, 10).slice(0, 120));
    conf('hover em `for` acha `for each` (dois tokens KW vizinhos)', valor(m, 11).includes('for each'), valor(m, 11).slice(0, 120));
    conf('hover em `return` acha a secao 6.3', valor(m, 12).includes('Retorno'), valor(m, 12).slice(0, 120));
    conf('hover em `funct` acha a secao 6.1', valor(m, 13).includes('Definição'), valor(m, 13).slice(0, 120));
    conf('hover em `post` (builtin) traz a pagina do builtin', valor(m, 14).includes('post('), valor(m, 14).slice(0, 120));
    conf('hover na variavel diz o TIPO construido e a linha', valor(m, 15).includes('Jinker mapping') && valor(m, 15).includes('linha 2'), valor(m, 15));
    conf('hover no parametro diz de que funct ele e', valor(m, 16).includes('parâmetro de `entra`'), valor(m, 16));
    conf('hover na funct mostra `int async funct` e o decorador',
         valor(m, 17).includes('int async funct entra(data)') && valor(m, 17).includes('@mapping.post'), valor(m, 17));
    conf('hover no model lista os campos', valor(m, 18).includes('email: str(length=60)') && valor(m, 18).includes('senha: str'), valor(m, 18));
    conf('hover em nome vindo de `from` mostra a assinatura do modulo, nao "N membros"',
         valor(m, 19).includes('jinker.jsonify(') && !valor(m, 19).includes('membros'), valor(m, 19));
    conf('hover em `request.get` traz a prosa de docs/jinker/request/get',
         valor(m, 20).includes('request.get(') && valor(m, 20).split('\n').length > 3, valor(m, 20).slice(0, 200));
    conf('hover em `post` de `@mapping.post` traz a prosa de docs/jinker/post',
         valor(m, 21).includes('mapping.post(') && valor(m, 21).includes('POST'), valor(m, 21).slice(0, 200));
  }

  /* ── 9a-ter. linha quebrada DENTRO da funct não apaga a funct ─────────────
   * O parser não se recuperava dentro de bloco: um statement quebrado no corpo
   * derrubava a funct inteira da árvore, e o que vinha depois vazava pro topo.
   * Enquanto se digita é exatamente o estado do arquivo — e o completion
   * perdia a variável local que está logo acima do cursor. */
  {
    const QB = [
      'funct calcula(int valor) {',
      '    x = = 1',
      '    total_local = valor * 2',
      '    post(total_l',
      '}',
      'fora = 1',
      'fo',
    ];
    const m = await conversa(QB.join('\n') + '\n', [compl(95, 3, 16), compl(96, 6, 2)]);
    const L = rotulos(resp(m, 95));
    conf('local declarada depois de uma linha quebrada aparece no completion',
         L.includes('total_local'), L);
    /* e do lado de FORA ela não existe: antes a recuperação despejava o resto
     * do corpo no topo, e a local virava global */
    const F = rotulos(resp(m, 96));
    conf('a local da funct quebrada nao vaza pra fora dela',
         F.includes('fora') && !F.includes('total_local'), F);
  }

  /* ── 9a-bis. dentro da f-string é CÓDIGO ──────────────────────────────────
   * A f-string era UM token e UM literal: `{nome.upper()}` não existia pra
   * árvore, então hover, definição e completion paravam na borda da aspa. O
   * motor passou a publicar as interpolações como nós filhos do literal, com
   * a posição real — o índice do servidor anda em `lista` e recebe isto sem
   * saber que veio de dentro de uma string. */
  {
    const FS = [
      'funct saudacao(str nome) {',
      '    return f"ola, {nome.upper()}"',
      '}',
      'total = 3',
      'post(f"sao {tot',
    ];
    const m = await conversa(FS.join('\n') + '\n', [
      { jsonrpc: '2.0', id: 90, method: 'textDocument/hover',
        params: { textDocument: { uri: URI }, position: { line: 1, character: 20 } } },
      { jsonrpc: '2.0', id: 91, method: 'textDocument/completion',
        params: { textDocument: { uri: URI }, position: { line: 4, character: 15 } } },
    ]);
    const h = resp(m, 90);
    const txt = h && h.result && h.result.contents ? h.result.contents.value : '';
    conf('hover no nome DENTRO da f-string diz de que funct ele é',
         txt.includes('nome'), txt);
    const L = rotulos(resp(m, 91));
    conf('completion dentro da f-string oferece as variáveis do arquivo',
         L.includes('total'), L);
  }

  /* ── 9b. exceção: hover diz a família (da tabela do catch, via --metadata) ─ */
  {
    const EXC = ['try {', '    x = 1', '} catch (FileNotFoundError e) {', '    post(e, OSError)', '}'];
    const m = await conversa(EXC.join('\n') + '\n', [
      hov(30, 2, EXC[2].indexOf('FileNotFoundError')),
      hov(31, 3, EXC[3].indexOf('OSError')),
    ]);
    conf('hover em `FileNotFoundError` diz de quem ela e filha',
         valor(m, 30).includes('filha de OSError') && valor(m, 30).includes('Exception'), valor(m, 30));
    conf('hover em `OSError` lista os filhos e a pagina da arvore',
         valor(m, 31).includes('filhos:') && valor(m, 31).includes('FileNotFoundError')
         && valor(m, 31).includes('exceptions.md'), valor(m, 31));
  }

  /* ── 9c. TODO receptor expõe o que é — a matriz de contextos ─────────────
   *
   * Medido antes, 16 de 30 contextos vazios ou errados: `from mail import
   * Mia` devolvia a lista de MÓDULOS (a tela dele); `nome = "ana"` + `nome.`
   * dava zero; `Rota.`/`Cor.` zero; `for each x` + `x.` zero; sem receptor
   * não vinha builtin nem palavra-chave; arquivo e pasta não apareciam no
   * `import`. Cada caso aqui é um desses. */
  {
    const dir = path.join(os.tmpdir(), 'ps_lsp_t');
    fs.mkdirSync(path.join(dir, 'pasta'), { recursive: true });
    fs.writeFileSync(path.join(dir, 'vizinho.pr'), 'funct soma_vizinha(a) {\n    return a\n}\n');
    fs.writeFileSync(path.join(dir, 'pasta', 'dentro.pr'), 'x = 1\n');
    const casos = [
      ['`from mail import Mia` -> MEMBROS do mail, nao modulos (a tela dele)',
       ['from regex import fullmatch, compile', 'from random import asterisco', 'from mail import Mia'], 2, undefined,
       ['MailServer', 'MailMessage', 'MailReader'], ['mail', 'json']],
      ['`from mail import MailServer, ` -> so os que faltam',
       ['from mail import MailServer, '], 0, undefined, ['MailMessage', 'MailReader'], ['MailServer']],
      ['`from jinker import ` -> funcoes E valores (request, cors)',
       ['from jinker import '], 0, undefined, ['Jinker', 'jsonify', 'request', 'cors'], []],
      ['`import ` -> modulos do motor + ARQUIVOS e PASTAS ao lado',
       ['import '], 0, undefined, ['mail', 'vizinho', 'pasta'], ['a']],
      ['`import pasta.` -> os arquivos DENTRO da pasta',
       ['import pasta.'], 0, undefined, ['dentro'], ['mail']],
      ['`from vizinho import ` -> as functs do arquivo ao lado',
       ['from vizinho import '], 0, undefined, ['soma_vizinha'], []],
      /* import por CAMINHO entre aspas (`import '../x.pr'`, como no TypeScript):
       * dentro das aspas vem arquivo, pasta, `..` e os modulos do motor */
      ["`import '` -> arquivos .pr, pastas, `..` e modulos do motor",
       ["import '"], 0, undefined, ['vizinho.pr', 'pasta', '..', 'mail'], ['a.pr']],
      ["`import 'pasta/` -> os arquivos DENTRO da pasta, sem modulos",
       ["import 'pasta/"], 0, undefined, ['dentro.pr'], ['mail', 'vizinho.pr']],
      ["`import 'pas|'` (aspa fechada pelo editor, cursor dentro) -> pasta",
       ["import 'pas'"], 0, 11, ['pasta'], []],
      ["`from './vizinho.pr' import ` -> as functs do arquivo",
       ["from './vizinho.pr' import "], 0, undefined, ['soma_vizinha'], []],
      ["`import './vizinho.pr'` + `vizinho.` -> os membros do arquivo",
       ["import './vizinho.pr'", 'vizinho.'], 1, undefined, ['soma_vizinha'], []],
      ["`import './vizinho.pr' as v` + `v.` -> idem pelo apelido",
       ["import './vizinho.pr' as v", 'v.'], 1, undefined, ['soma_vizinha'], []],
      ['`nome = "ana"` + `nome.` -> metodos de str (tipo pelo LITERAL)',
       ['nome = "ana"', 'nome.'], 1, undefined, ['upper', 'split'], []],
      ['`xs = [1, 2]` + `xs.` -> metodos de list',
       ['xs = [1, 2]', 'xs.'], 1, undefined, ['append'], []],
      ['`d = {"a": 1}` + `d.` -> metodos de dict',
       ['d = {"a": 1}', 'd.'], 1, undefined, ['keys'], []],
      ['`"abc".` direto -> metodos de str',
       ['x = "abc".'], 0, undefined, ['upper'], []],
      ['`Rota.` (model do arquivo) -> campos',
       ['model Rota() {', '    email: str', '}', 'Rota.'], 3, undefined, ['email'], []],
      ['`r = Rota()` + `r.` -> campos',
       ['model Rota() {', '    email: str', '}', 'r = Rota()', 'r.'], 4, undefined, ['email'], []],
      ['`Cor.` (enum do arquivo) -> membros',
       ['enum Cor {', '    AZUL', '    VERDE', '}', 'Cor.'], 4, undefined, ['AZUL', 'VERDE'], []],
      ['`for each x in [1, 2]` + `x.` -> ao menos os universais',
       ['for each x in [1, 2] {', '    x.', '}'], 1, undefined, ['type'], []],
      ['`request.get("x").` (retorno desconhecido) -> universais',
       ['from jinker import request', 'request.get("x").'], 1, undefined, ['type'], []],
      ['`str funct g()` + `v = g()` + `v.` -> metodos de str',
       ['str funct g() {', '    return "a"', '}', 'v = g()', 'v.'], 4, undefined, ['upper'], []],
      ['`string s = "a"` + `s.` -> metodos de str (apelido de tipo, pela tabela do --metadata)',
       ['string s = "a"', 's.'], 1, undefined, ['upper'], []],
      ['sem receptor: nomes do arquivo + import + BUILTINS + PALAVRAS-CHAVE + MODIFICADORES',
       ['import mail', 'total = 1', 'funct soma(a) {', '    return a', '}', 't'], 5, undefined,
       ['total', 'soma', 'mail', 'post', 'len', 'str', 'funct', 'if', 'for', 'static', 'nonnull'],
       ['action', 'reaction']],
      /* As exceções vêm de `--metadata` ("excecoes", a tabela do catch): são
       * valores da linguagem e o sugestor não as tinha. */
      ['sem receptor: as EXCEÇÕES do motor aparecem',
       ['x = Val'], 0, undefined,
       ['ValueError', 'Exception', 'OSError'], []],
      /* LITERAIS e `__name__`: o lexer entrega `true`/`false`/`Null` como token
       * próprio (BOOL/NULL) e `__name__` é global do compilador — nenhum dos
       * quatro está na KEYWORDS[] do --metadata, e o sugestor não os tinha. */
      ['sem receptor: os LITERAIS e `__name__` aparecem',
       ['x = 1', 'x'], 1, undefined, ['true', 'false', 'Null', '__name__'], []],
      ['dentro da Entity o CONSTRUTOR e sugerido',
       ['Entity Conta {', '    saldo: int', '    f'], 2, undefined, ['__init__'], []],
      ['Entity que JA tem __init__ nao sugere outro',
       ['Entity Conta {', '    funct __init__(self) {', '        self.s = 0', '    }', '    f'], 4, undefined,
       [], ['__init__']],
      /* a reforma: `funct` e os modificadores COLADOS. `static`/`nonnull` não são
       * palavra reservada, então nada disso vem de graça do --metadata. */
      ["`static funct` numa Entity: o completion de `Tipo.` diz que e static",
       ['Entity Mat {', '    static funct soma(a, b) {', '        return a + b', '    }', '}', 'Mat.'],
       5, undefined, ['soma'], []],
      ["`@static` (o decorador) marca o metodo igual ao modificador colado",
       ['Entity Mat {', '    @static', '    funct soma(a, b) {', '        return a + b', '    }', '}', 'Mat.'],
       6, undefined, ['soma'], []],
      ["`funct` do arquivo aparece no completion sem receptor",
       ['funct minha(a) {', '    return a', '}', 'm'], 3, undefined, ['minha'], []],
      ['`self.` dentro de `if`, campo `private str nome` do corpo e metodo HERDADO',
       ['class Base {', '    funct b(self) {', '        return 1', '    }', '}', 'class C(Base) {',
        '    private str nome = "a"', '    int n = 1', '    funct __init__(self, x) {', '        self.x = x', '    }',
        '    funct m(self) {', '        if self.n > 0 {', '            self.', '        }', '    }', '}'],
       13, undefined, ['nome', 'n', 'x', 'm', 'b'], []],
      /* O TERCEIRO PONTO (ele, 2026-09-25: "o hover ainda sugere type em todo
       * terceiro . de member access"). Medido: campo criado no __init__ sem
       * tipo (`self.con = psodbc.connect(...)`) ficava sem tipo — `self.con.`
       * só `type`, `self.con.cursor().` NADA; `sys.stdout.` só `type`;
       * `sys.argv[0].` caía nos nomes do arquivo; `str(x).upper().` nada;
       * `self.f().` de metodo sem tipo declarado só `type`. */
      ['campo sem tipo criado no __init__ (`self.con = psodbc.connect(...)`): `self.con.` tem o tipo',
       ['import psodbc', 'Entity Db {', '    funct __init__(self) {', '        self.con = psodbc.connect("x")', '    }',
        '    funct q(self) {', '        self.con.', '    }', '}'],
       6, undefined, ['cursor', 'commit'], []],
      ['terceiro ponto: `self.con.cursor().` lista os metodos do cursor',
       ['import psodbc', 'Entity Db {', '    funct __init__(self) {', '        self.con = psodbc.connect("x")', '    }',
        '    funct q(self) {', '        self.con.cursor().', '    }', '}'],
       6, undefined, ['execute', 'fetchall', 'fetchone'], ['self', 'post']],
      ['campo sem tipo = Entity(...): `self.e.rua.` chega no str',
       ['Entity End {', '    str rua', '}', 'Entity U {', '    funct __init__(self) {', '        self.e = End("r")', '    }',
        '    funct f(self) {', '        self.e.rua.', '    }', '}'],
       8, undefined, ['upper', 'strip'], ['self']],
      ['campo sem tipo = parametro TIPADO do __init__: `self.nome.` e str',
       ['Entity U {', '    funct __init__(self, str n) {', '        self.nome = n', '    }',
        '    funct f(self) {', '        self.nome.', '    }', '}'],
       5, undefined, ['upper'], ['self']],
      ['metodo sem tipo declarado: o retorno vem dos `return` (`self.f().`)',
       ['Entity U {', '    funct f(self) { return "a" }', '    funct g(self) {', '        self.f().', '    }', '}'],
       3, undefined, ['upper'], ['self']],
      ['`sys.stdout.` entra no submodulo (write/writeln/flush)',
       ['import sys', 'sys.stdout.'], 1, undefined, ['write', 'writeln', 'flush'], ['argv']],
      ['`sys.argv[0].` e item de tipo desconhecido: universais, nunca os nomes do arquivo',
       ['import sys', 'minha_var = 1', 'sys.argv[0].'], 2, undefined, ['type'], ['append', 'minha_var', 'sys']],
      ['`str(x).upper().` comeca a cadeia no tipo da conversao',
       ['x = 1', 'str(x).upper().'], 1, undefined, ['lower', 'strip'], ['x']],
      /* `base()` / `base(Pai)`, como o super (07-entity §7.5.2) */
      ['`base().` lista o que o pai tem, com o `__init__`',
       ['Entity A {', '    funct __init__(self, x) { self.x = x }', '    funct fala(self) { return 1 }', '}',
        'Entity B(A) {', '    funct __init__(self) {', '        base().', '    }', '}'],
       6, undefined, ['__init__', 'fala', 'x'], ['self']],
      ['`base(A).` idem, pelo nome',
       ['Entity A {', '    funct __init__(self, x) { self.x = x }', '    funct fala(self) { return 1 }', '}',
        'Entity B(A) {', '    funct __init__(self) {', '        base(A).', '    }', '}'],
       6, undefined, ['__init__', 'fala'], ['self']],
      ['`base(` oferece os PAIS da Entity',
       ['Entity A {', '}', 'Entity M {', '}', 'Entity B(A, M) {', '    funct __init__(self) {', '        base(', '    }', '}'],
       6, undefined, ['A', 'M'], ['B', 'self', 'post']],
      /* Variável tipada pela EXPRESSÃO atribuída, qualquer expressão (ele,
       * 2026-09-26: "usei a lib psodbc e deu type na instancia pra uso do
       * execute"). Antes só `x = Classe(...)`, `x = mod.f(...)` e literal
       * tipavam; `cur = con.cursor()` vinha de OUTRA variável e ficava mudo. */
      ['`cur = con.cursor()` (construído de outra variável) + `cur.` lista execute',
       ['import psodbc', 'con = psodbc.connect("sqlite://x.db")', 'cur = con.cursor()', 'cur.'],
       3, undefined, ['execute', 'fetchall', 'fetchone'], ['con', 'post']],
      ['o mesmo dentro de `static funct` de uma Entity',
       ['import psodbc', 'public Entity main {', '    public static funct mj1() {',
        '        con = psodbc.connect("sqlite://x.db")', '        cur = con.cursor()', '        cur.', '    }', '}'],
       5, undefined, ['execute', 'fetchall'], ['post']],
      ['`v = jinker.request` (valor de módulo, sem chamada) + `v.`',
       ['import jinker', 'v = jinker.request', 'v.'], 2, undefined, ['get_json', 'header', 'json'], ['post']],
      ['`y = x` herda o tipo de `x`',
       ['x = "a"', 'y = x', 'y.'], 2, undefined, ['upper'], ['post']],
      ['`x = x.upper()` tipa o `x` da direita pela atribuição anterior',
       ['x = "a"', 'x = x.upper()', 'x.'], 2, undefined, ['upper'], ['post']],
      ['`app.route("/").` acha a tabela pelo nome sem o sublinhado (`_RouteRegistrar`)',
       ['from jinker import Jinker', 'app = Jinker(__name__)', 'app.route("/").'], 2, undefined, ['register'], ['post']],
      ['`os.PoolFile("a").` — classe do motor (retorno `type`) chamada dá a instância',
       ['import os', 'f = os.PoolFile("a")', 'f.'], 2, undefined, ['read', 'write', 'close'], ['post']],
    ];
    for (const [nome, linhas, line, ch, espera, nao] of casos) {
      const src = linhas.join('\n') + '\n';
      const c = ch !== undefined ? ch : linhas[line].length;
      const m = await conversa(src, [compl(2, line, c)]);
      const L = rotulos(resp(m, 2));
      const faltam = espera.filter((e) => !L.includes(e));
      const sobram = nao.filter((e) => L.includes(e));
      conf(nome, faltam.length === 0 && sobram.length === 0,
           { voltou: L.slice(0, 10), faltam, sobram });
    }
  }

  /* ── 9d. os modificadores COLADOS (`static funct`, `nonnull funct`) ──────
   *
   * Eles não são palavra reservada: o lexer os entrega como IDENT, então não
   * entram em `--metadata` nem passam pela porta de palavra-chave do hover.
   * Tudo que o editor sabe deles é a exceção que confere se ali começa uma
   * cabeça de funct — e é isso que estes três casos medem, incluindo o lado
   * negativo: `static = 1` é uma variável, não um modificador. */
  {
    const MOD = [
      'Entity Mat {',
      '    static funct soma(a, b) {',
      '        return a + b',
      '    }',
      '}',
      'nonnull funct exige(v) {',
      '    return v',
      '}',
      'static = 1',
      '',
    ];
    const m = await conversa(MOD.join('\n'),
      [hov(30, 1, MOD[1].indexOf('static') + 2), hov(31, 5, 2),
       hov(32, 8, 2), compl(33, 9, 0)]);
    conf('hover em `static` colado acha a secao dos modificadores',
         valor(m, 30).includes('static'), valor(m, 30).slice(0, 140));
    conf('hover em `nonnull` colado acha a secao dos modificadores',
         valor(m, 31).includes('nonnull'), valor(m, 31).slice(0, 140));
    conf('hover em `static` que e VARIAVEL nao devolve a secao do modificador',
         !valor(m, 32).includes('Entity'), valor(m, 32).slice(0, 140));
    const L = rotulos(resp(m, 33));
    conf('completion oferece `funct`, `static` e `nonnull`',
         ['funct', 'static', 'nonnull'].every((k) => L.includes(k)), L.slice(0, 12));
  }

  /* ── 10. CLASSE, HERANÇA, `self` — nada disso funcionava ────────────────
   *
   * Medido antes: `self.` dava ZERO, `c = Conta(...)` + `c.` dava ZERO, e a
   * lista sem receptor não tinha nem o parâmetro da funct nem o nome da
   * classe. Uma linha explicava as três: o nome de Entity é `IDENT_UPPER` no
   * lexer, e o servidor exigia `IDENT` — então toda Entity de todo arquivo
   * era invisível. */
  const OO = [
    'import mail',
    'Entity Base {',
    '    public funct ping(self) { return "pong" }',
    '}',
    'Entity Conta(Base) {',
    '    saldo: int',
    '    dono: str',
    '    public funct deposita(self, valor) {',
    '        self.',
    '    }',
    '    private funct log(self) { return "x" }',
    '}',
    'funct principal(quantia, cliente) {',
    '    c = Conta(0, "ana")',
    '    c.',
    '    ',
    '}',
  ].join('\n');
  {
    const m = await conversa(OO, [compl(2, 8, 13), compl(3, 14, 6), compl(4, 15, 4),
      { jsonrpc: '2.0', id: 5, method: 'textDocument/documentSymbol', params: { textDocument: { uri: URI } } }]);
    const S = rotulos(resp(m, 2));
    conf('`self.` lista campo e metodo da propria Entity',
         S.includes('saldo') && S.includes('dono') && S.includes('deposita'), S);
    conf('`self.` mostra o private (de DENTRO ele e visivel)', S.includes('log'), S);
    conf('`self.` traz o HERDADO do pai', S.includes('ping'), S);

    const C = rotulos(resp(m, 3));
    conf('`c.` com `c = Conta(...)` lista os membros da instancia',
         C.includes('saldo') && C.includes('deposita') && C.includes('ping'), C);
    conf('de FORA, o private nao aparece', !C.includes('log'), C);
    conf('`__init__` nao e oferecido como membro', !C.includes('__init__'), C);

    const L = rotulos(resp(m, 4));
    conf('a lista sem receptor tem os PARAMETROS da funct',
         L.includes('quantia') && L.includes('cliente'), L);
    conf('...e a variavel local declarada antes do cursor', L.includes('c'), L);
    conf('...e as Entities do arquivo', L.includes('Conta') && L.includes('Base'), L);

    const ds = resp(m, 5);
    const raiz = ds && ds.result ? ds.result : [];
    const cls = raiz.find((s) => s.name === 'Conta');
    conf('o outline traz a classe com os membros DENTRO dela',
         !!cls && (cls.children || []).map((x) => x.name).includes('deposita'),
         cls && (cls.children || []).map((x) => x.name));
  }

  /* ── 11. módulo NÃO importado aparecia em qualquer lugar ─────────────────
   *
   * O servidor despejava TODO módulo do motor na lista de qualquer ponto do
   * arquivo, com "(precisa de import)" no detalhe. Módulo que o arquivo não
   * importou não é candidato a nada: é ruído com cara de sugestão. O lugar
   * deles é depois do `import`. E módulo de FACHADA (stub que só levantava
   * NotImplemented) não existe mais em lista nenhuma. */
  {
    const m = await conversa('funct f() {\n    \n}\n', [compl(2, 1, 4)]);
    const L = rotulos(resp(m, 2));
    /* `json`/`str`/`list` são também palavra-chave ou builtin, e ESSES
     * entram; o que não pode entrar é módulo que só existe via import */
    conf('modulo NAO importado nao entra na lista sem receptor',
         !L.includes('mail') && !L.includes('regex') && !L.includes('jinker'), L);
  }
  {
    const m = await conversa('import \n', [compl(2, 0, 7)]);
    const L = rotulos(resp(m, 2));
    conf('depois de `import` os modulos do motor APARECEM — e nenhum stub de fachada',
         L.includes('mail') && L.includes('json') && L.includes('os')
         && !L.includes('smtplib') && !L.includes('multipart') && !L.includes('mimetext'), L.slice(0, 8));
  }

  /* ── 12. Entity de OUTRO arquivo, pelo import ───────────────────────────── */
  {
    const dir = path.join(os.tmpdir(), 'ps_lsp_t');
    fs.writeFileSync(path.join(dir, 'modelo.pr'),
      'Entity Usuario {\n    nome: str\n    public funct saudacao(self) { return "oi" }\n}\n');
    const m = await conversa('import modelo\nu = modelo.Usuario("ana")\nu.\n', [compl(2, 2, 2)]);
    const L = rotulos(resp(m, 2));
    conf('Entity de arquivo importado expoe os membros dela',
         L.includes('nome') && L.includes('saudacao'), L);
  }

  /* ── 12b. `import *` e a regra de EXPORTAÇÃO do motor ────────────────────
   *
   * As três grafias (`from m import *`, `import m *`, `PUSH m GET *`) ligam
   * o que o módulo exporta e NÃO ligam o nome do módulo. A regra do que sai é
   * uma só no motor, pra `*`, `from m import x` e `m.x`: o topo do arquivo,
   * sem `private`, com `_x` incluído, e nada do que nasce dentro de bloco.
   * Antes o editor tinha a regra dele: escondia `_x` e oferecia a funct
   * `private` que o import recusa. Os fixtures foram rodados no `./pool`
   * antes de virar caso: `soma_estrela`, `_interno`, `da_ponte`, `de_a` e
   * `de_b` existem no programa; `escondida`, `so_no_bloco` e `estrela_ponte`
   * dão NameError. */
  {
    const dir = path.join(os.tmpdir(), 'ps_lsp_t');
    fs.writeFileSync(path.join(dir, 'estrela_fonte.pr'),
      'funct soma_estrela(a) {\n    return a\n}\nprivate funct escondida() {\n    return 0\n}\n'
      + '_interno = 1\nif true {\n    so_no_bloco = 2\n}\n');
    fs.writeFileSync(path.join(dir, 'estrela_ponte.pr'),
      'import estrela_fonte *\nfunct da_ponte() {\n    return 1\n}\n');
    /* `*` de ida e volta: sem o corte de ciclo o servidor não termina */
    fs.writeFileSync(path.join(dir, 'ciclo_a.pr'), 'from ciclo_b import *\nfunct de_a() {\n    return 1\n}\n');
    fs.writeFileSync(path.join(dir, 'ciclo_b.pr'), 'from ciclo_a import *\nfunct de_b() {\n    return 2\n}\n');
    const def = (id, line, ch) => ({ jsonrpc: '2.0', id, method: 'textDocument/definition',
      params: { textDocument: { uri: URI }, position: { line, character: ch } } });

    {
      const m = await conversa('from json import *\np\n', [compl(2, 1, 1)]);
      const L = rotulos(resp(m, 2));
      conf('`from json import *` -> o completion sem receptor traz `parse` e `stringify`',
           L.includes('parse') && L.includes('stringify'), L.slice(0, 10));
    }
    {
      const m = await conversa('from json import \nPUSH json GET \n', [compl(2, 0, 17), compl(3, 1, 14)]);
      const L1 = rotulos(resp(m, 2));
      conf('`from json import ` oferece `*` (e os membros do json)', L1.includes('*') && L1.includes('parse'), L1);
      const L2 = rotulos(resp(m, 3));
      conf('`PUSH json GET ` oferece `*` (e os membros do json)', L2.includes('*') && L2.includes('parse'), L2);
    }
    {
      const m = await conversa('import json *\nx = json.\n', [compl(2, 1, 9)]);
      const L = rotulos(resp(m, 2));
      conf('`import json *` NAO liga `json`: `json.` nao vira membro de modulo',
           !L.includes('parse') && !L.includes('stringify'), L.slice(0, 10));
    }
    {
      const SRC = ['from estrela_fonte import *', 'soma_estrela(1)', 'escondida()', '_interno', 'x'];
      const m = await conversa(SRC.join('\n') + '\n', [
        hov(3, 1, 2), def(4, 1, 2), hov(5, 2, 2), def(6, 2, 2), hov(7, 3, 2), compl(8, 4, 1),
      ]);
      conf('hover em nome trazido por `*` de arquivo mostra a funct e o arquivo',
           valor(m, 3).includes('funct soma_estrela(a)') && valor(m, 3).includes('estrela_fonte.pr'), valor(m, 3));
      const d = resp(m, 4);
      conf('definicao de nome trazido por `*` vai na declaracao, no arquivo do modulo',
           !!d && !!d.result && d.result.uri.endsWith('/estrela_fonte.pr') && d.result.range.start.line === 0,
           d && d.result);
      conf('`private funct` NAO e resolvida pelo `*` (hover mudo)', valor(m, 5) === '', valor(m, 5));
      const d2 = resp(m, 6);
      conf('`private funct` NAO e resolvida pelo `*` (sem definicao)', !!d2 && !d2.result, d2 && d2.result);
      conf('`_interno` (topo, com sublinhado) E resolvido pelo `*`', valor(m, 7).includes('_interno'), valor(m, 7));
      const L = rotulos(resp(m, 8));
      const faltam = ['soma_estrela', '_interno'].filter((e) => !L.includes(e));
      const sobram = ['escondida', 'so_no_bloco', 'estrela_fonte'].filter((e) => L.includes(e));
      conf('completion depois do `*`: topo e `_x` sim; private, nome de bloco e o modulo nao',
           faltam.length === 0 && sobram.length === 0, { faltam, sobram });
    }
    /* a MESMA regra no `from m import ` e no `m.` */
    const casos = [
      ['`from estrela_fonte import ` -> `*`, topo e `_x`; sem private nem nome de bloco',
       ['from estrela_fonte import '], 0, ['*', 'soma_estrela', '_interno'], ['escondida', 'so_no_bloco']],
      ['`import estrela_fonte` + `estrela_fonte.` -> topo e `_x`; sem private nem nome de bloco',
       ['import estrela_fonte', 'estrela_fonte.'], 1, ['soma_estrela', '_interno'], ['escondida', 'so_no_bloco']],
    ];
    for (const [nome, linhas, line, espera, nao] of casos) {
      const m = await conversa(linhas.join('\n') + '\n', [compl(2, line, linhas[line].length)]);
      const L = rotulos(resp(m, 2));
      const faltam = espera.filter((e) => !L.includes(e));
      const sobram = nao.filter((e) => L.includes(e));
      conf(nome, faltam.length === 0 && sobram.length === 0, { voltou: L.slice(0, 10), faltam, sobram });
    }
    /* `*` que atravessa arquivos (reexporte) e `*` em ciclo, nas outras duas
     * grafias: caminho entre aspas e `PUSH m GET *` */
    {
      const SRC = ["import './estrela_ponte.pr' *", 'PUSH ciclo_a GET *', 'soma_estrela(2)', 's'];
      const m = await conversa(SRC.join('\n') + '\n', [compl(2, 3, 1), def(3, 2, 2)]);
      const L = rotulos(resp(m, 2));
      const faltam = ['soma_estrela', 'da_ponte', 'de_a', 'de_b'].filter((e) => !L.includes(e));
      const sobram = ['escondida', 'estrela_ponte', 'ciclo_a'].filter((e) => L.includes(e));
      conf('`*` transitivo e em ciclo: traz o reexportado e os dois lados do ciclo',
           faltam.length === 0 && sobram.length === 0, { faltam, sobram });
      const d = resp(m, 3);
      conf('definicao de nome reexportado por `*` vai no arquivo de ORIGEM',
           !!d && !!d.result && d.result.uri.endsWith('/estrela_fonte.pr'), d && d.result);
    }

    /* ── 12c. a ORDEM do arquivo, e a assinatura de quem veio pelo `*` ─────
     *
     * Medido no `./pool` antes de virar caso, com um módulo que exporta `x` e
     * `soma_ordem`:
     *
     *   x = 5 / `*`            -> embaixo, x é o do módulo
     *   `*` / x = 5            -> embaixo, x é a variável
     *   funct lendo x, x = 5 e `*` no fim          -> x é o do módulo
     *   `*`, funct lendo x e x = 5 no fim          -> x é a variável
     *   `*` / funct soma_ordem embaixo             -> entre os dois vale o do
     *                                                 módulo; da funct pra
     *                                                 baixo, a funct
     *
     * E o signatureHelp: `f(` de um nome vindo do `*` mostrava assinatura
     * nenhuma, de arquivo `.pr` e de módulo do motor. */
    fs.writeFileSync(path.join(dir, 'estrela_ordem.pr'),
      'x = "do modulo"\nfunct soma_ordem(a) {\n    return a\n}\n');
    const sig = (id, line, ch) => ({ jsonrpc: '2.0', id, method: 'textDocument/signatureHelp',
      params: { textDocument: { uri: URI }, position: { line, character: ch } } });
    const rotuloSig = (m, id) => {
      const r = resp(m, id);
      return r && r.result && r.result.signatures[0] ? r.result.signatures[0].label : '';
    };
    {
      const ult = 'soma_estrela(';
      const m = await conversa('from estrela_fonte import *\n' + ult, [sig(3, 1, ult.length)]);
      conf('signatureHelp de funct vinda de `*` de arquivo .pr mostra a assinatura da declaracao',
           rotuloSig(m, 3) === 'soma_estrela(a)', rotuloSig(m, 3));
    }
    {
      const ult = 'sub(';
      const m = await conversa('from regex import *\n' + ult, [sig(3, 1, ult.length)]);
      const l = rotuloSig(m, 3);
      conf('signatureHelp de funct vinda de `*` de modulo do motor mostra os params do motor',
           l.startsWith('sub(') && l.includes('pattern'), l);
    }
    {
      const m = await conversa('x = 5\nfrom estrela_ordem import *\nx\n', [hov(3, 2, 0), def(4, 2, 0)]);
      conf('`x = 5` e `*` embaixo: na linha de baixo o x e o do MODULO',
           valor(m, 3).includes('estrela_ordem.pr'), valor(m, 3));
      const d = resp(m, 4);
      conf('...e a definicao vai no arquivo do modulo',
           !!d && !!d.result && d.result.uri.endsWith('/estrela_ordem.pr') && d.result.range.start.line === 0,
           d && d.result);
    }
    {
      const m = await conversa('from estrela_ordem import *\nx = 5\nx\n', [hov(3, 2, 0), def(4, 2, 0)]);
      conf('`*` e `x = 5` embaixo: na linha de baixo o x e a VARIAVEL',
           valor(m, 3).includes('linha 2') && !valor(m, 3).includes('estrela_ordem'), valor(m, 3));
      const d = resp(m, 4);
      conf('...e a definicao fica no proprio arquivo, na linha da atribuicao',
           !!d && !!d.result && d.result.uri.endsWith('/a.pr') && d.result.range.start.line === 1,
           d && d.result);
    }
    {
      /* dentro da funct a linha não decide: ela roda com o arquivo já
       * carregado, e vale a ÚLTIMA ligação do topo */
      const m = await conversa('funct f() {\n    return x\n}\nx = 5\nfrom estrela_ordem import *\n', [hov(3, 1, 11)]);
      conf('dentro da funct, com o `*` por ultimo no arquivo, o x e o do MODULO',
           valor(m, 3).includes('estrela_ordem.pr'), valor(m, 3));
    }
    {
      const m = await conversa('from estrela_ordem import *\nfunct f() {\n    return x\n}\nx = 5\n', [hov(3, 2, 11)]);
      conf('dentro da funct, com a variavel por ultimo, o `*` NAO responde pelo x',
           !valor(m, 3).includes('estrela_ordem'), valor(m, 3));
    }
    {
      const SRC = ['from estrela_ordem import *', 'soma_ordem(1)', 'funct soma_ordem(a, b) {',
                   '    return a', '}', 'soma_ordem(2)'];
      const m = await conversa(SRC.join('\n') + '\n', [hov(3, 1, 0), hov(4, 5, 0)]);
      conf('nome de funct NAO e hoisted por cima do `*`: acima da declaracao vale o do modulo',
           valor(m, 3).includes('estrela_ordem.pr'), valor(m, 3));
      conf('...e da linha da declaracao pra baixo vale a funct do arquivo',
           valor(m, 4).includes('funct soma_ordem(a, b)') && !valor(m, 4).includes('estrela_ordem'), valor(m, 4));
    }
  }

  /* ── 13. A LISTA DELE, item por item ─────────────────────────────────────
   *
   * Ele escreveu o que não funcionava, e cada linha vira um caso:
   *
   *   "eu expresso uma variavel e não e sugerida a mim, nem classe, nem
   *    funcção nem nada, objetos de outro objeto, uma funct dentro de classe,
   *    um self, uma lib, se eu escrever um modulo de uma lib sem ter importado
   *    fica sugerindo, as libs instaladas não aparecem, vc fica pondo tetos de
   *    exposição, ex: random tem asterisco, ai vc pois um teto que so expõe
   *    isso, as functs dela não"
   *
   * Tudo aqui é medido com a linha PELA METADE, que é o estado real de quem
   * está digitando — e era justamente o estado em que o servidor respondia
   * vazio. */
  {
    const src = [
      'Entity Motor {',
      '    public funct ligar(self) { return 1 }',
      '}',
      'Entity Carro {',
      '    motor: Motor',
      '    private funct interna(self) { return 0 }',
      '    public funct andar(self, marcha) {',
      '        velocidade = marcha * 10',
      '        self.',
      '    }',
      '}',
      'funct principal(quantos) {',
      '    c = Carro(Motor())',
      '    c.motor.',
      '    ',
      '}',
    ].join('\n');
    const m = await conversa(src, [compl(2, 8, 13), compl(3, 13, 12), compl(4, 14, 4)]);

    const S = rotulos(resp(m, 2));
    conf('digitando `self.` (linha PELA METADE) responde',
         S.includes('motor') && S.includes('andar'), S);
    conf('...e mostra a funcao privada de dentro da propria classe',
         S.includes('interna'), S);

    const CM = rotulos(resp(m, 3));
    conf('objeto de outro objeto: `c.motor.` resolve a cadeia', CM.includes('ligar'), CM);

    const L = rotulos(resp(m, 4));
    conf('variavel local declarada acima e sugerida', L.includes('c'), L);
    conf('parametro da funct e sugerido', L.includes('quantos'), L);
    conf('as classes do arquivo sao sugeridas',
         L.includes('Carro') && L.includes('Motor'), L);
  }
  {
    /* O "teto" que ele apontou: a lib expõe a classe, e a classe expõe os
     * métodos dela. Parar na classe é o teto. */
    const libs = pastaLibs();
    let comClasse = '';
    let classe = '';
    try {
      for (const f of fs.readdirSync(libs)) {
        if (!f.endsWith('.pr')) continue;
        const txt = fs.readFileSync(path.join(libs, f), 'utf8');
        const i = txt.indexOf('class ');
        if (i < 0) continue;
        let j = i + 6;
        let nome = '';
        while (j < txt.length && txt[j] !== '(' && txt[j] !== ' ' && txt[j] !== '\n') nome += txt[j++];
        if (!nome) continue;
        comClasse = f.slice(0, -3); classe = nome; break;
      }
    } catch (_) { /* sem libs */ }
    if (!comClasse) {
      console.log('  PULOU o teto de exposicao — nenhuma lib com class em ~/.jinga/libs');
    } else {
      const src = `import ${comClasse}\ninst = ${comClasse}.${classe}()\ninst.\n`;
      const m = await conversa(src, [compl(2, 2, 5)]);
      const L = rotulos(resp(m, 2));
      conf(`sem teto: \`${comClasse}.${classe}()\` expoe os METODOS da classe`,
           L.length > 0, L.slice(0, 8));
    }
  }

  /* ── o MANIFESTO: o que a extensão promete ao VS Code ────────────────────
   *
   * O botão de rodar não é código do servidor — é contribuição declarada no
   * `package.json`. Um `git` malfeito ou um merge tira a entrada e ninguém
   * percebe: a extensão instala, o realce funciona, e o botão simplesmente
   * não está lá. Aqui a promessa fica travada. */
  {
    const man = JSON.parse(fs.readFileSync(path.join(__dirname, 'package.json'), 'utf8'));
    const c = man.contributes || {};
    const cmds = (c.commands || []).map((x) => x.command);
    conf('o manifesto declara o comando de rodar', cmds.includes('jinga.rodar'), cmds);
    conf('o botão de rodar aparece na barra do editor',
         ((c.menus || {})['editor/title/run'] || []).some((m) => m.command === 'jinga.rodar'),
         Object.keys(c.menus || {}));
    conf('o comando tem atalho (ctrl+f5)',
         (c.keybindings || []).some((k) => k.command === 'jinga.rodar' && k.key),
         (c.keybindings || []).map((k) => k.key));
    conf('o comando só aparece em arquivo da linguagem',
         (c.commands || []).length > 0
         && ((c.menus || {}).commandPalette || []).every((m) => (m.when || '').includes('jinga')),
         (c.menus || {}).commandPalette);
    /* O cliente registra o comando ANTES do `return` que desliga o LSP: quem
     * põe `lsp.ativo: false` não está pedindo pra perder o botão. */
    const cli = fs.readFileSync(path.join(__dirname, 'extension.js'), 'utf8');
    const iReg = cli.indexOf("registerCommand('jinga.rodar'");
    const iOff = cli.indexOf("get('lsp.ativo')");
    conf('o comando é registrado mesmo com o LSP desligado',
         iReg > 0 && iOff > 0 && iReg < iOff, { iReg, iOff });

    /* ── depurador ────────────────────────────────────────────────────────
     * Sem estas entradas o VS Code nem deixa pôr breakpoint num `.pr`: a
     * gengiva do depurador é declarativa, e falta dela não dá erro nenhum —
     * o F5 simplesmente não faz nada, que é o pior modo de quebrar. */
    conf('o manifesto permite breakpoint em .pr',
         (c.breakpoints || []).some((b) => b.language === 'jinga'),
         c.breakpoints);
    const dbg = (c.debuggers || []).find((d) => d.type === 'jinga');
    conf('o manifesto declara o depurador jinga', !!dbg,
         (c.debuggers || []).map((d) => d.type));
    conf('a configuração de launch exige o programa',
         !!dbg && ((dbg.configurationAttributes || {}).launch || {}).required
                  .includes('programa'),
         dbg && dbg.configurationAttributes);
    conf('há configuração inicial pronta (F5 sem launch.json)',
         !!dbg && (dbg.initialConfigurations || []).length > 0,
         dbg && dbg.initialConfigurations);
    conf('a extensão acorda pra depurar',
         (man.activationEvents || []).includes('onDebugResolve:jinga'),
         man.activationEvents);
    conf('o comando do gráfico existe',
         (c.commands || []).some((x) => x.command === 'jinga.grafico'),
         (c.commands || []).map((x) => x.command));
    /* Rodar e Depurar lado a lado no topo do editor: quem está com o arquivo
     * aberto escolhe ali, sem ter que decorar que o atalho global é F5. */
    conf('o botão de depurar fica ao lado do de rodar',
         ((c.menus || {})['editor/title/run'] || [])
           .some((x) => x.command === 'jinga.depurar'),
         ((c.menus || {})['editor/title/run'] || []).map((x) => x.command));
    /* `Rodar | Depurar` inline, em cima do ponto de entrada — o lugar em que o
     * Java põe o `Run | Debug` acima do `main`. */
    conf('há CodeLens de Rodar/Depurar',
         cli.indexOf('registerCodeLensProvider') > 0
         && cli.indexOf("title: 'Rodar'") > 0
         && cli.indexOf("title: 'Depurar'") > 0,
         { lens: cli.indexOf('registerCodeLensProvider') });
    /* A âncora é o `if __name__ == "main"`, não a linha 1: numa linha fixa a
     * lente flutua acima do comentário de cabeçalho, longe do que executa. */
    conf('a CodeLens ancora no ponto de entrada',
         cli.indexOf('__name__') > 0
         && cli.indexOf('linhaDeEntrada') > 0,
         { ancora: cli.indexOf('__name__') });
    conf('o comando de depurar é registrado mesmo com o LSP desligado',
         cli.indexOf("registerCommand('jinga.depurar'") > 0
         && cli.indexOf("registerCommand('jinga.depurar'") < iOff,
         { reg: cli.indexOf("registerCommand('jinga.depurar'"), iOff });

    /* A fábrica do adaptador tem que ESPERAR o motor abrir a porta antes de
     * devolver o descritor: o VS Code conecta na hora, e devolver antes dá
     * "connection refused" sem explicação. */
    conf('a fábrica espera a porta abrir antes de conectar',
         cli.indexOf('esperaPorta') > 0
         && cli.indexOf('await esperaPorta') < cli.indexOf('DebugAdapterServer'),
         { espera: cli.indexOf('await esperaPorta'),
           servidor: cli.indexOf('DebugAdapterServer') });
    /* Porta escolhida pelo sistema, não fixa: porta fixa colide ao depurar
     * dois arquivos ao mesmo tempo. */
    conf('a porta do depurador é pedida ao sistema',
         cli.indexOf('portaLivre') > 0 && cli.indexOf('listen(0') > 0,
         { portaLivre: cli.indexOf('portaLivre') });
    /* O gráfico é buscado no `terminated`, enquanto a sessão ainda responde. */
    conf('o gráfico é capturado antes da sessão fechar',
         cli.indexOf("'terminated'") > 0
         && cli.indexOf("customRequest('jingaGrafico')") > cli.indexOf("'terminated'"),
         { terminated: cli.indexOf("'terminated'"),
           pedido: cli.indexOf("customRequest('jingaGrafico')") });
  }

  /* ── 14. MÉTODOS DE TIPO NO HOVER — parâmetro tipado, literal, receptor sem tipo ──
   * Os três casos que ficavam mudos (medidos no comaly.pr do dono): o tipo do
   * parâmetro era descartado pelo analise.js; o literal não entrava na cadeia
   * de nomes; e receptor de tipo desconhecido só tinha os universais. E a
   * prosa de str/byte, que nunca aparecia porque a doc morava em docs/string
   * e docs/bytes/metodos — agora docs/str e docs/byte, o nome do tipo. */
  {
    const hov = (id, l, c) => ({ jsonrpc: '2.0', id, method: 'textDocument/hover',
      params: { textDocument: { uri: URI }, position: { line: l, character: c } } });
    const txt = (m, id) => { const r = resp(m, id); return r && r.result && r.result.contents
      ? (r.result.contents.value || String(r.result.contents)) : ''; };
    const src = 'funct f(byte raw, str body) {\n'          // 0
              + '    return raw.split(b"\\r\\n", 1)\n'      // 1  raw@11 split@15
              + '}\n'                                       // 2
              + 'h = b"q"\n'                                // 3
              + 'h.decode()\n'                              // 4  decode@2
              + 's = "abc"\n'                               // 5
              + 's.upper()\n'                               // 6  upper@2
              + 'c = "x".encode()\n'                        // 7  encode@8
              + 'x = f"a{s}".upper()\n'                     // 8  upper@12
              + 'd = b"x".decode()\n'                       // 9  decode@9
              + 'lines = ["a", "b"]\n'                      // 10
              + 'e = lines[1].decode()\n'                   // 11 decode@13
              + 'head = lines[0]\n'                         // 12
              + 't = head.decode()\n'                       // 13 decode@9
              + 'Entity C {\n    funct m(self) {\n        return 1\n    }\n}\n'   // 14-18
              + 'k = C()\n'                                 // 19
              + 'k.upper()\n';                              // 20 upper@2
    const m = await conversa(src, [
      hov(31, 1, 16), hov(32, 1, 12), hov(33, 4, 3), hov(34, 6, 3), hov(35, 7, 9), hov(36, 8, 13),
      hov(37, 9, 10), hov(38, 11, 14), hov(39, 13, 10), hov(40, 3, 0), hov(41, 20, 3),
    ]);
    conf('hover em metodo de PARAMETRO TIPADO (`byte raw` -> raw.split) resolve pelo tipo',
         txt(m, 31).includes('raw.split(') && txt(m, 31).includes('-> list'), txt(m, 31));
    conf('hover no metodo do parametro tipado traz a prosa da pagina docs/byte',
         txt(m, 31).includes('Parte no separador'), txt(m, 31));
    conf('hover no parametro mostra o tipo antes do nome (`byte raw`)',
         txt(m, 32).includes('byte raw') && txt(m, 32).includes('parâmetro de `f`'), txt(m, 32));
    conf('variavel de literal b"..." e tipada como byte, nao str (h.decode)',
         txt(m, 33).includes('h.decode(') && txt(m, 33).includes('Volta pra texto'), txt(m, 33));
    conf('hover em metodo de str traz a prosa (docs/str)',
         txt(m, 34).includes('s.upper()') && txt(m, 34).includes('maiúsculas'), txt(m, 34));
    conf('hover em metodo de LITERAL de texto ("x".encode)',
         txt(m, 35).includes('str.encode(') && txt(m, 35).includes('-> byte'), txt(m, 35));
    conf('hover em metodo de f-string (f"...".upper)',
         txt(m, 36).includes('str.upper()'), txt(m, 36));
    conf('hover em metodo de literal de bytes (b"x".decode)',
         txt(m, 37).includes('byte.decode('), txt(m, 37));
    conf('receptor de tipo DESCONHECIDO (lines[1].decode) lista os candidatos por nome',
         txt(m, 38).includes('receptor de tipo desconhecido') && txt(m, 38).includes('byte.decode('), txt(m, 38));
    conf('variavel vinda de indice (head = lines[0]) tambem cai nos candidatos',
         txt(m, 39).includes('receptor de tipo desconhecido') && txt(m, 39).includes('byte.decode('), txt(m, 39));
    conf('hover na variavel de literal de bytes diz `byte h`',
         txt(m, 40).includes('byte h'), txt(m, 40));
    conf('membro inexistente numa Entity NAO vira candidato de outro tipo (k.upper fica mudo)',
         txt(m, 41) === '', txt(m, 41));
  }
  /* `Parsing` nasce ligado, sem import (a VM liga o nome). O editor só
   * resolvia `x.membro` com `x` vindo de import: `Parsing.` ficava mudo em
   * todo lugar — no topo e dentro de classe, que foi onde ele viu. */
  {
    const src = 'x = Parsing.integer("5", to int)\n'           // 0  integer@12
              + 'class K {\n'                               // 1
              + '    funct m(self) {\n'                       // 2
              + '        return Parsing.integer("7", to int)\n' // 3  integer@23
              + '    }\n'                                     // 4
              + '}\n';                                        // 5
    const m = await conversa(src, [hov(46, 0, 14), hov(47, 3, 25), hov(48, 3, 18)]);
    conf('hover em `Parsing.integer` no topo, sem import',
         valor(m, 46).includes('Parsing.integer('), valor(m, 46));
    conf('hover em `Parsing.integer` DENTRO de metodo de classe',
         valor(m, 47).includes('Parsing.integer('), valor(m, 47));
    conf('hover no proprio `Parsing` diz que e modulo',
         valor(m, 48).includes('Parsing'), valor(m, 48));
    const mc = await conversa('class K {\n    funct m(self) {\n        Parsing.\n    }\n}\n', [compl(49, 2, 16)]);
    const lc = rotulos(resp(mc, 49));
    conf('completion `Parsing.` dentro de metodo de classe lista os membros',
         lc.includes('integer') && lc.includes('TransientValue'), lc.slice(0, 8));
  }
  /* completion e signatureHelp com o literal como receptor */
  {
    const m1 = await conversa('h = b"q"\nh.', [compl(42, 1, 2)]);
    conf('completion apos `h.` com h = b"q" lista os metodos de byte',
         rotulos(resp(m1, 42)).includes('decode') && rotulos(resp(m1, 42)).includes('hex'), rotulos(resp(m1, 42)).slice(0, 6));
    const m2 = await conversa('b"q".', [compl(43, 0, 5)]);
    conf('completion logo apos o literal `b"q".` (o span do token BYTES conta o prefixo)',
         rotulos(resp(m2, 43)).includes('decode'), rotulos(resp(m2, 43)).slice(0, 6));
    const sig = (id, l, c) => ({ jsonrpc: '2.0', id, method: 'textDocument/signatureHelp',
      params: { textDocument: { uri: URI }, position: { line: l, character: c } } });
    const m3 = await conversa('y = "a,b".split(', [sig(44, 0, 16)]);
    const l3 = resp(m3, 44) && resp(m3, 44).result && resp(m3, 44).result.signatures[0] ? resp(m3, 44).result.signatures[0].label : '';
    conf('signatureHelp com literal de texto como receptor ("a,b".split()', l3.startsWith('str.split('), l3);
    const m4 = await conversa('z = b"x".decode(', [sig(45, 0, 16)]);
    const l4 = resp(m4, 45) && resp(m4, 45).result && resp(m4, 45).result.signatures[0] ? resp(m4, 45).result.signatures[0].label : '';
    conf('signatureHelp com literal de bytes como receptor (b"x".decode()', l4.startsWith('byte.decode('), l4);
  }
  /* O `retorna` é MEDIDO e tem três formas: tipo, união (`str|Null`) e `*`
   * (tipo do conteúdo). A união aparece inteira no hover; o `*` não é nome de
   * tipo e não aparece; e o encadeamento segue pelo lado que tem membros. */
  {
    const src = 'import os\n'                                // 0
              + 'import json\n'                              // 1
              + 'import jinker\n'                            // 2
              + 'a = os.getenv("X")\n'                       // 3  getenv@7
              + 'b = json.parse("{}")\n'                     // 4  parse@9
              + 'jinker.request.file("f").\n';               // 5
    const m = await conversa(src, [hov(50, 3, 8), hov(51, 4, 10), compl(52, 5, 25)]);
    conf('hover de nativo com retorno em uniao mostra os dois lados (os.getenv -> str|Null)',
         valor(m, 50).includes('-> str|Null'), valor(m, 50));
    conf('hover de nativo cujo retorno e o conteudo (json.parse) nao mostra `-> *`',
         valor(m, 51).includes('json.parse(') && !valor(m, 51).includes('->'), valor(m, 51));
    const l = rotulos(resp(m, 52));
    conf('completion apos retorno `PoolFileUpload|Null` segue pelo PoolFileUpload',
         l.includes('save') && l.includes('content_type'), l.slice(0, 8));
  }

  /* ── clientes que NÃO são o VS Code (LSP4IJ/IntelliJ, Neovim, Helix) ─────
   * O servidor não lê capabilities nem workspaceFolders — mas o LSP4IJ manda
   * a URI com percent-encoding (`%20`, `%C3%A7`) e, no fallback, com uma
   * barra só (`file:/…`); cortar `file://` na unha deixava a pasta com `%20`
   * dentro e sumiam os vizinhos do `import`, e o go-to-definition devolvia
   * URI crua com espaço, que o cliente rejeita. E sem o `jinga` no PATH do
   * editor o servidor respondia VAZIO sem avisar — o sintoma "não sugere
   * nada" do IntelliJ, sem um rastro. */
  {
    const { pathToFileURL, fileURLToPath } = require('url');
    const raiz = path.join(os.tmpdir(), 'ps_lsp_t', 'pasta com espaço');
    fs.mkdirSync(raiz, { recursive: true });
    const vizinhoAbs = path.join(raiz, 'vizinho.pr');
    fs.writeFileSync(vizinhoAbs, 'funct f() {\n    return 1\n}\n');
    const aAbs = path.join(raiz, 'a.pr');
    const uriCod = pathToFileURL(aAbs).href;                         // file:///…/pasta%20com%20espa%C3%A7o/a.pr
    const uriUmaBarra = 'file:/' + uriCod.slice('file:///'.length);  // file:/…  (o fallback do LSP4J)
    const defEm = (id, l, c, uri) => ({ jsonrpc: '2.0', id, method: 'textDocument/definition',
      params: { textDocument: { uri }, position: { line: l, character: c } } });

    /* 1. `initialize` como o LSP4J manda: sem initializationOptions — o
     *    servidor cai no PATH, que aqui tem o binário (`jinga`, ou `pool` numa
     *    instalação de antes do rename: é o `command -v` do servidor que escolhe) */
    const jingaDir = path.dirname(path.resolve(JINGA));
    const envMin = Object.assign({}, process.env, { PATH: jingaDir + path.delimiter + (process.env.PATH || '') });
    const m1 = await conversa('import regex\nx = regex.\n', [compl(60, 1, 10)],
                              { inicializa: { rootUri: null, capabilities: {} }, env: envMin });
    const L1 = rotulos(resp(m1, 60));
    conf('initialize minimo (capabilities vazias, sem initializationOptions) -> `regex.` responde',
         L1.length >= 5, L1.slice(0, 6));

    /* 2. URI com UMA barra */
    const m2 = await conversa('import \n', [compl(61, 0, 7, uriUmaBarra)], { uri: uriUmaBarra });
    const L2 = rotulos(resp(m2, 61));
    conf('URI `file:/…` (uma barra) ainda lista o arquivo vizinho no `import`', L2.includes('vizinho'), L2.slice(0, 8));

    /* 3. URI percent-encoded (pasta com espaço e acento) + definition */
    const m3 = await conversa('from vizinho import f\nf()\n',
                              [compl(62, 0, 7, uriCod), defEm(63, 1, 0, uriCod)], { uri: uriCod });
    const L3 = rotulos(resp(m3, 62));
    conf('URI percent-encoded lista o vizinho no `import`', L3.includes('vizinho'), L3.slice(0, 8));
    const d3 = resp(m3, 63) && resp(m3, 63).result;
    conf('definition devolve URI CODIFICADA (a que o cliente aceita) apontando pro vizinho',
         !!d3 && d3.uri === pathToFileURL(vizinhoAbs).href, d3 && d3.uri);

    /* 4. jinga inexistente: o servidor AVISA em vez de responder vazio calado.
     *    O caminho que ele cita é o de `initializationOptions.jinga` — se a
     *    opção fosse ignorada, o servidor cairia no PATH e a mensagem não
     *    teria esse caminho. */
    const m4 = await conversa('import regex\nx = regex.\n', [compl(64, 1, 10)],
                              { inicializa: { rootUri: null, capabilities: {},
                                              initializationOptions: { jinga: '/nao/existe/jinga' } } });
    const aviso = m4.find((x) => x.method === 'window/showMessage');
    conf('jinga inexistente: o servidor avisa o cliente (window/showMessage) citando o caminho',
         !!aviso && !!aviso.params && String(aviso.params.message).includes('/nao/existe/jinga'),
         aviso && aviso.params);
    conf('...e manda o detalhe em window/logMessage', m4.some((x) => x.method === 'window/logMessage'));
    conf('...e a completion ainda responde (sem travar)', !!resp(m4, 64));

    /* 5. `initializationOptions.pool` é o nome de ANTES do rename: um cliente
     *    antigo (vsix não reempacotada, IntelliJ de ontem) ainda manda ele, e
     *    o servidor tem que honrar — a prova é o mesmo aviso citando o caminho */
    const m5 = await conversa('import regex\nx = regex.\n', [compl(65, 1, 10)],
                              { inicializa: { rootUri: null, capabilities: {},
                                              initializationOptions: { pool: '/nao/existe/pool' } } });
    const avisoPool = m5.find((x) => x.method === 'window/showMessage');
    conf('`initializationOptions.pool` (nome antigo) ainda aponta o binario: o aviso cita ESSE caminho',
         !!avisoPool && !!avisoPool.params && String(avisoPool.params.message).includes('/nao/existe/pool'),
         avisoPool && avisoPool.params);
  }

  /* ── 16. `static` da classe pelo nome SOLTO — o que o motor aceita, o editor mostra ──
   * "quando crio dentro de classes, tudo fica ofuscado, nada é visível se não
   * tiver self". O motor resolve campo E método static (da classe e dos pais)
   * pelo nome solto, de qualquer método; o editor só oferecia depois de
   * `self.`. Parâmetro com o mesmo nome ganha, como no motor. */
  {
    const hov = (id, l, c) => ({ jsonrpc: '2.0', id, method: 'textDocument/hover',
      params: { textDocument: { uri: URI }, position: { line: l, character: c } } });
    const def = (id, l, c) => ({ jsonrpc: '2.0', id, method: 'textDocument/definition',
      params: { textDocument: { uri: URI }, position: { line: l, character: c } } });
    /* (`base` é palavra reservada — o `base(...)` da herança — e não serve de
     * nome de campo; o fixture usa `raiz`) */
    const SRC = [
      'class Pai {',                          // 0
      '    public static int raiz = 1',         // 1
      '    static funct dobro(n) { return n * 2 }',   // 2
      '}',                                      // 3
      'class Api(Pai) {',                       // 4
      '    public static int total = 3',        // 5
      '    static funct s(a) { return a }',     // 6
      '    funct m(self) { return 1 }',         // 7
      '    funct n(self) {',                    // 8
      '        return total + raiz + s(1) + dobro(2)',   // 9
      '    }',                                  // 10
      '    funct p(self, total) {',             // 11
      '        return total',                   // 12
      '    }',                                  // 13
      '}',                                      // 14
    ];
    const cs = (trecho) => SRC[9].indexOf(trecho) + 1;    /* coluna DENTRO do nome */
    const m = await conversa(SRC.join('\n') + '\n', [
      compl(2, 9, 8), hov(3, 9, cs('total')), hov(4, 9, cs('raiz')), hov(5, 9, SRC[9].indexOf('s(')),
      hov(6, 9, cs('dobro')), def(7, 9, cs('total')), def(8, 9, cs('dobro')), hov(9, 12, 16),
    ]);
    const L = rotulos(resp(m, 2));
    const faltam = ['total', 'raiz', 's', 'dobro'].filter((e) => !L.includes(e));
    conf('dentro do método, o completion oferece os `static` da classe e do pai pelo nome solto',
         faltam.length === 0, { faltam });
    conf('...e NÃO oferece o método comum solto (`m` é `self.m`)', !L.includes('m'), L.filter((x) => x === 'm'));
    conf('hover em campo static solto mostra o campo', valor(m, 3).includes('static int total'), valor(m, 3));
    conf('hover em campo static do PAI solto diz de quem herdou',
         valor(m, 4).includes('static int raiz') && valor(m, 4).includes('herdado de `Pai`'), valor(m, 4));
    conf('hover em método static solto mostra a assinatura', valor(m, 5).includes('static funct s(a)'), valor(m, 5));
    conf('hover em método static do PAI solto', valor(m, 6).includes('dobro(n)') && valor(m, 6).includes('Pai'), valor(m, 6));
    const d7 = resp(m, 7) && resp(m, 7).result;
    conf('definição do campo static solto vai na linha da declaração', !!d7 && d7.range.start.line === 5, d7);
    const d8 = resp(m, 8) && resp(m, 8).result;
    conf('definição do método static do pai vai na declaração dele', !!d8 && d8.range.start.line === 2, d8);
    conf('parâmetro com o mesmo nome GANHA do static (hover é o parâmetro)',
         valor(m, 9).includes('parâmetro') && !valor(m, 9).includes('static'), valor(m, 9));
  }

  /* ── 17. `model` com parâmetros de validação (regex, in, min/max, of…) ──
   * O hover mostra os parâmetros como foram escritos; dentro dos parênteses
   * do campo, o completion oferece os que ainda faltam na linha. */
  {
    const hov = (id, l, c) => ({ jsonrpc: '2.0', id, method: 'textDocument/hover',
      params: { textDocument: { uri: URI }, position: { line: l, character: c } } });
    const SRC = [
      'model Endereco() {',                                   // 0
      '    cep: str(regex="^[0-9]{5}-[0-9]{3}$")',            // 1
      '}',                                                    // 2
      'model Usuario() {',                                    // 3
      '    nome: str(length=20, regex="^[a-z]+$")',           // 4
      '    idade: int(min=18, max=120)',                      // 5
      '    papel: str(in=["admin", "user"])',                 // 6
      '    apelido: str(optional=true)',                      // 7
      '    endereco: Endereco',                               // 8
      '    tags: list(of=str, length=3)',                     // 9
      '    saldo: flo(min=-1.5)',                             // 10
      '}',                                                    // 11
      'u = Usuario',                                          // 12
    ];
    const m = await conversa(SRC.join('\n') + '\n', [hov(2, 12, 5)]);
    const h = valor(m, 2);
    conf('hover do model mostra length e regex do campo', h.includes('nome: str(length=20, regex="^[a-z]+$")'), h);
    conf('hover do model mostra min e max', h.includes('idade: int(min=18, max=120)'), h);
    conf('hover do model mostra in com a lista', h.includes('papel: str(in=["admin", "user"])'), h);
    conf('hover do model mostra optional, o model aninhado e o of',
         h.includes('apelido: str(optional=true)') && h.includes('endereco: Endereco')
         && h.includes('tags: list(length=3, of=str)'), h);
    conf('hover do model mostra número negativo', h.includes('saldo: flo(min=-1.5)'), h);
    /* a linha pela metade — o momento em que o completion é pedido; o
     * arquivo NÃO parseia, e o completion tem que responder mesmo assim */
    const M1 = ['model Usuario() {', '    x: str(', '}'];
    const m1 = await conversa(M1.join('\n') + '\n', [compl(3, 1, M1[1].length)]);
    const L3 = rotulos(resp(m1, 3));
    conf('dentro dos parênteses do campo: os 8 parâmetros do model',
         ['length=', 'regex=', 'in=', 'not_in=', 'min=', 'max=', 'optional=', 'of='].every((e) => L3.includes(e)), L3);
    const M2 = ['model Usuario() {', '    y: int(min=1, ', '}'];
    const m2 = await conversa(M2.join('\n') + '\n', [compl(4, 1, M2[1].length)]);
    const L4 = rotulos(resp(m2, 4));
    conf('...menos o que a linha já tem', L4.includes('max=') && !L4.includes('min='), L4);
    /* fora de model, `f(` é chamada, não campo: nada dos 8 */
    const M3 = ['funct str2(a) { return a }', 'z = str2('];
    const m3 = await conversa(M3.join('\n') + '\n', [compl(5, 1, M3[1].length)]);
    const L5 = rotulos(resp(m3, 5));
    conf('fora de model, parênteses de chamada não oferecem os parâmetros do campo', !L5.includes('regex='), L5.slice(0, 6));
  }

  console.log('');
  if (falhas) { console.log(`lsp: ${feitos} checagens, ${falhas} FALHARAM`); process.exit(1); }
  console.log(`lsp: ${feitos} checagens, todas passaram`);
}

main().catch((e) => { console.error(e); process.exit(1); });
