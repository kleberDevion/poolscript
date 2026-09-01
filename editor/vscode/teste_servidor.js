/*
 * Dirige o servidor LSP como o VS Code faria: escreve mensagens enquadradas na
 * entrada dele e confere as respostas. Não precisa de editor nenhum.
 *
 *     node editor/vscode/teste_servidor.js [caminho-do-pool]
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

const POOL = process.argv[2] || 'pool';
const SERVIDOR = path.join(__dirname, 'server.js');

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
function conversa(texto, pedidos) {
  return new Promise((resolve, reject) => {
    const p = spawn(process.execPath, [SERVIDOR, '--stdio'], { stdio: ['pipe', 'pipe', 'pipe'] });
    let buf = Buffer.alloc(0);
    const msgs = [];
    const esperados = new Set(pedidos.map((q) => q.id).filter((x) => x !== undefined));
    let terminou = false;

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
        const m = /Content-Length: (\d+)/i.exec(cab);
        if (!m) break;
        const n = parseInt(m[1], 10);
        if (buf.length < s + 4 + n) break;
        const msg = JSON.parse(buf.slice(s + 4, s + 4 + n).toString('utf8'));
        msgs.push(msg);
        if (msg.id !== undefined) esperados.delete(msg.id);
        buf = buf.slice(s + 4 + n);
      }
      /* tudo respondido: dá um respiro pro diagnóstico assíncrono e encerra */
      if (esperados.size === 0 && !terminou) setTimeout(encerra, 400);
    });
    p.on('error', reject);

    manda({ jsonrpc: '2.0', id: 1, method: 'initialize',
            params: { rootUri: null, capabilities: {}, initializationOptions: { pool: POOL } } });
    manda({ jsonrpc: '2.0', method: 'initialized', params: {} });
    manda({ jsonrpc: '2.0', method: 'textDocument/didOpen',
            params: { textDocument: { uri: URI, languageId: 'poolscript', version: 1, text: texto } } });
    for (const q of pedidos) manda(q);
    if (esperados.size === 0) setTimeout(encerra, 600);   /* só diagnóstico */
    setTimeout(() => { try { p.kill(); } catch (_) {} resolve(msgs); }, 20000);
  });
}

const URI = 'file://' + path.join(os.tmpdir(), 'ps_lsp_t', 'a.ps');


const resp = (msgs, id) => msgs.find((m) => m.id === id);
const rotulos = (m) => {
  if (!m || !m.result) return [];
  const its = Array.isArray(m.result) ? m.result : (m.result.items || []);
  return its.map((i) => i.label);
};

const compl = (id, l, c) => ({ jsonrpc: '2.0', id, method: 'textDocument/completion',
  params: { textDocument: { uri: URI }, position: { line: l, character: c } } });

async function main() {
  fs.mkdirSync(path.join(os.tmpdir(), 'ps_lsp_t'), { recursive: true });

  /* ── 1. `import ... as` — dava ZERO sugestão ───────────────────────────── */
  {
    const m = await conversa('import json as js\nx = js.\n', [compl(2, 1, 7)]);
    const L = rotulos(resp(m, 2));
    conf('`import json as js` -> `js.` sugere os membros do json', L.includes('parse') && L.includes('stringify'), L);
  }

  /* ── 2. lib instalada em ~/.poolscript/libs — dava ZERO ────────────────── */
  {
    const libs = process.env.HOME ? path.join(process.env.HOME, '.poolscript', 'libs') : '';
    let alguma = '';
    try {
      alguma = (fs.readdirSync(libs).find((f) => f.endsWith('.ps')) || '').replace(/\.ps$/, '');
    } catch (_) { /* sem libs instaladas */ }
    if (!alguma) {
      console.log('  PULOU lib instalada — nenhuma em ~/.poolscript/libs');
    } else {
      const src = `import ${alguma}\nx = ${alguma}.\n`;
      const m = await conversa(src, [compl(2, 1, 5 + alguma.length)]);
      const L = rotulos(resp(m, 2));
      conf(`lib instalada \`${alguma}\` expoe membros`, L.length > 0, L.slice(0, 6));
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
    const src = 'action soma(a, b=2) {\n    return a\n}\nsoma(';
    const m = await conversa(src, [compl(2, 3, 5)]);
    const L = rotulos(resp(m, 2));
    conf('action LOCAL oferece os parametros dela', L.includes('a=') && L.includes('b='), L);
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
    const src = 'action alfa(x) {\n    return x\n}\naction beta() {\n    return 1\n}\n';
    const m = await conversa(src, [
      { jsonrpc: '2.0', id: 3, method: 'textDocument/documentSymbol', params: { textDocument: { uri: URI } } },
      { jsonrpc: '2.0', id: 4, method: 'textDocument/definition',
        params: { textDocument: { uri: URI }, position: { line: 0, character: 8 } } },
    ]);
    const ds = resp(m, 3);
    const nomes = ds && ds.result ? ds.result.map((s) => s.name) : [];
    conf('documentSymbol lista as actions do arquivo', nomes.includes('alfa') && nomes.includes('beta'), nomes);
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

  /* ── 8. diagnóstico vem do `--check` do motor ──────────────────────────── */
  {
    const m = await conversa('action f(:\n    return 1\n', []);
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

  console.log('');
  if (falhas) { console.log(`lsp: ${feitos} checagens, ${falhas} FALHARAM`); process.exit(1); }
  console.log(`lsp: ${feitos} checagens, todas passaram`);
}

main().catch((e) => { console.error(e); process.exit(1); });
