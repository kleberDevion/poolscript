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
      const f = fs.readdirSync(libs).find((x) => x.endsWith('.ps')) || '';
      alguma = f.slice(0, f.length - '.ps'.length);
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
    'int async action entra(data) {',
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
      hov(12, 9, col(9, 'return')),             hov(13, 7, col(7, 'action')),
      hov(14, 12, col(12, 'post')),             hov(15, 1, 0),
      hov(16, 7, col(7, 'data')),               hov(17, 7, col(7, 'entra')),
      hov(18, 2, col(2, 'Rota')),               hov(19, 9, col(9, 'jsonify')),
      hov(20, 14, col(14, 'request.get') + 'request.'.length),
      hov(21, 6, col(6, 'post')),
    ]);
    conf('hover em `if` traz a secao da doc da linguagem', valor(m, 10).includes('Condicional'), valor(m, 10).slice(0, 120));
    conf('hover em `for` acha `for each` (dois tokens KW vizinhos)', valor(m, 11).includes('for each'), valor(m, 11).slice(0, 120));
    conf('hover em `return` acha a secao 6.3', valor(m, 12).includes('Retorno'), valor(m, 12).slice(0, 120));
    conf('hover em `action` acha a secao 6.1', valor(m, 13).includes('Definição'), valor(m, 13).slice(0, 120));
    conf('hover em `post` (builtin) traz a pagina do builtin', valor(m, 14).includes('post('), valor(m, 14).slice(0, 120));
    conf('hover na variavel diz o TIPO construido e a linha', valor(m, 15).includes('Jinker mapping') && valor(m, 15).includes('linha 2'), valor(m, 15));
    conf('hover no parametro diz de que action ele e', valor(m, 16).includes('parâmetro de `entra`'), valor(m, 16));
    conf('hover na action mostra `int async action` e o decorador',
         valor(m, 17).includes('int async action entra(data)') && valor(m, 17).includes('@mapping.post'), valor(m, 17));
    conf('hover no model lista os campos', valor(m, 18).includes('email: str(length=60)') && valor(m, 18).includes('senha: str'), valor(m, 18));
    conf('hover em nome vindo de `from` mostra a assinatura do modulo, nao "N membros"',
         valor(m, 19).includes('jinker.jsonify(') && !valor(m, 19).includes('membros'), valor(m, 19));
    conf('hover em `request.get` traz a prosa de docs/jinker/request/get',
         valor(m, 20).includes('request.get(') && valor(m, 20).split('\n').length > 3, valor(m, 20).slice(0, 200));
    conf('hover em `post` de `@mapping.post` traz a prosa de docs/jinker/post',
         valor(m, 21).includes('mapping.post(') && valor(m, 21).includes('POST'), valor(m, 21).slice(0, 200));
  }

  /* ── 10. CLASSE, HERANÇA, `self` — nada disso funcionava ────────────────
   *
   * Medido antes: `self.` dava ZERO, `c = Conta(...)` + `c.` dava ZERO, e a
   * lista sem receptor não tinha nem o parâmetro da action nem o nome da
   * classe. Uma linha explicava as três: o nome de Entity é `IDENT_UPPER` no
   * lexer, e o servidor exigia `IDENT` — então toda Entity de todo arquivo
   * era invisível. */
  const OO = [
    'import mail',
    'Entity Base() {',
    '    public action ping(self) { return "pong" }',
    '}',
    'Entity Conta(Base) {',
    '    saldo: int',
    '    dono: str',
    '    public action deposita(self, valor) {',
    '        self.',
    '    }',
    '    private action log(self) { return "x" }',
    '}',
    'action principal(quantia, cliente) {',
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
    conf('a lista sem receptor tem os PARAMETROS da action',
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

  /* ── 11. `f` sugeria `flask` ─────────────────────────────────────────────
   *
   * O servidor despejava TODO módulo do motor na lista de qualquer ponto do
   * arquivo, com "(precisa de import)" no detalhe. Módulo que o arquivo não
   * importou não é candidato a nada: é ruído com cara de sugestão. O lugar
   * deles é depois do `import`. */
  {
    const m = await conversa('action f() {\n    \n}\n', [compl(2, 1, 4)]);
    const L = rotulos(resp(m, 2));
    conf('modulo NAO importado nao entra na lista (era `f` -> `flask`)',
         !L.includes('flask') && !L.includes('smtplib'), L);
  }
  {
    const m = await conversa('import \n', [compl(2, 0, 7)]);
    const L = rotulos(resp(m, 2));
    conf('depois de `import` os modulos do motor APARECEM',
         L.includes('flask') && L.includes('json') && L.includes('os'), L.slice(0, 8));
  }

  /* ── 12. Entity de OUTRO arquivo, pelo import ───────────────────────────── */
  {
    const dir = path.join(os.tmpdir(), 'ps_lsp_t');
    fs.writeFileSync(path.join(dir, 'modelo.ps'),
      'Entity Usuario() {\n    nome: str\n    public action saudacao(self) { return "oi" }\n}\n');
    const m = await conversa('import modelo\nu = modelo.Usuario("ana")\nu.\n', [compl(2, 2, 2)]);
    const L = rotulos(resp(m, 2));
    conf('Entity de arquivo importado expoe os membros dela',
         L.includes('nome') && L.includes('saudacao'), L);
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
      'Entity Motor() {',
      '    public action ligar(self) { return 1 }',
      '}',
      'Entity Carro() {',
      '    motor: Motor',
      '    private action interna(self) { return 0 }',
      '    public action andar(self, marcha) {',
      '        velocidade = marcha * 10',
      '        self.',
      '    }',
      '}',
      'action principal(quantos) {',
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
    conf('parametro da action e sugerido', L.includes('quantos'), L);
    conf('as classes do arquivo sao sugeridas',
         L.includes('Carro') && L.includes('Motor'), L);
  }
  {
    /* O "teto" que ele apontou: a lib expõe a classe, e a classe expõe os
     * métodos dela. Parar na classe é o teto. */
    const libs = process.env.HOME ? path.join(process.env.HOME, '.poolscript', 'libs') : '';
    let comClasse = '';
    let classe = '';
    try {
      for (const f of fs.readdirSync(libs)) {
        if (!f.endsWith('.ps')) continue;
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
      console.log('  PULOU o teto de exposicao — nenhuma lib com class em ~/.poolscript/libs');
    } else {
      const src = `import ${comClasse}\ninst = ${comClasse}.${classe}()\ninst.\n`;
      const m = await conversa(src, [compl(2, 2, 5)]);
      const L = rotulos(resp(m, 2));
      conf(`sem teto: \`${comClasse}.${classe}()\` expoe os METODOS da classe`,
           L.length > 0, L.slice(0, 8));
    }
  }

  console.log('');
  if (falhas) { console.log(`lsp: ${feitos} checagens, ${falhas} FALHARAM`); process.exit(1); }
  console.log(`lsp: ${feitos} checagens, todas passaram`);
}

main().catch((e) => { console.error(e); process.exit(1); });
