/*
 * Análise do documento a partir da ÁRVORE do motor (`pool --ast`).
 *
 * POR QUE ESTE ARQUIVO SUBSTITUIU A ANÁLISE QUE HAVIA NO `server.js`.
 *
 * O servidor respondia casando PADRÃO em cima do texto: uma expressão regular
 * pra `self.`, outra pra `alvo.`, uma varredura de token escrita à mão pra
 * achar parâmetro, outra pra achar Entity, outra pra saber se o cursor estava
 * dentro de chamada. Cada forma nova que o usuário escrevia era um ramo novo
 * escrito à mão — e por isso sempre faltava um:
 *
 *     variável declarada três linhas acima      não era sugerida
 *     método dentro de classe                   não era sugerido
 *     `a.b.c`                                   não resolvia
 *     membro de lib instalada                   não aparecia
 *     módulo NÃO importado                      aparecia (não devia)
 *
 * Isso não é análise, é remendo, e remendo não termina. Quem sabe o que é
 * variável, de quem é o membro e o que está em escopo naquela linha é o
 * PARSER — o mesmo que compila o programa. Ele publica a árvore em
 * `pool --ast`, e este arquivo responde a partir dela.
 *
 * NÃO HÁ UMA EXPRESSÃO REGULAR NESTE ARQUIVO, e é de propósito: toda regex
 * aqui seria uma segunda gramática, que é exatamente o que estava errado.
 * Onde é preciso olhar o texto (o que o usuário digitou até o cursor), é
 * varredura de caractere, que não inventa estrutura.
 *
 * O QUE ELE PRODUZ, numa passada só pela árvore:
 *
 *   escopos     cada `action` e cada `Entity` viram um escopo com intervalo de
 *               LINHAS e a lista do que ligam (parâmetro, variável, laço,
 *               desempacotamento, import, action e Entity aninhadas)
 *   entidades   nome, pais, campos e métodos, com `private` e posição
 *   imports     nome ligado -> módulo do motor ou arquivo .ps
 */
'use strict';

/* ── utilidades de texto SEM regex ─────────────────────────────────────── */

function ehLetra(c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c === '_'
      || c.charCodeAt(0) > 127;          /* acento é letra num identificador */
}
function ehDigito(c) { return c >= '0' && c <= '9'; }
function ehNomeChar(c) { return ehLetra(c) || ehDigito(c); }

/* A CADEIA imediatamente antes do cursor: `a.b.c.` -> ["a","b","c"], com
 * `terminaEmPonto` dizendo se o usuário acabou de digitar o ponto.
 *
 * Varre pra trás caractere a caractere. Não usa regex porque não precisa
 * decidir nada de estrutura: só ler nome, ponto, nome. */
function cadeiaAntes(linha, coluna) {
  let i = coluna;
  const partes = [];
  let parcial = '';

  /* o pedaço que está sendo digitado agora (pode ser vazio) */
  let fim = i;
  while (i > 0 && ehNomeChar(linha[i - 1])) i--;
  parcial = linha.slice(i, fim);

  for (;;) {
    if (i === 0 || linha[i - 1] !== '.') break;
    i--;                                        /* passa o ponto */

    /* `random.asterisco().` — o `)` vem antes do nome. Pula o grupo casado
     * andando pra trás; sem isto a cadeia parava no primeiro parêntese e o
     * que devia sugerir os membros do retorno não sugeria nada. */
    while (i > 0 && (linha[i - 1] === ' ' || linha[i - 1] === '\t')) i--;
    if (i > 0 && linha[i - 1] === ')') {
      let prof = 0;
      while (i > 0) {
        const c = linha[i - 1];
        if (c === ')') prof++;
        else if (c === '(') { prof--; if (prof === 0) { i--; break; } }
        i--;
      }
      if (prof !== 0) break;                    /* parêntese sem par: desiste */
    }

    fim = i;
    while (i > 0 && ehNomeChar(linha[i - 1])) i--;
    const seg = linha.slice(i, fim);
    if (seg === '') break;
    partes.unshift(seg);
  }
  return { partes, parcial, terminaEmPonto: partes.length > 0 };
}

/* ── a árvore ───────────────────────────────────────────────────────────── */

const FILHOS = ['a', 'b', 'c', 'e'];
const LISTAS = ['lista', 'lista2', 'alias'];

function cada(no, fn) {
  if (!no || typeof no !== 'object') return;
  for (const k of FILHOS) if (no[k]) fn(no[k], k);
  for (const k of LISTAS) {
    const v = no[k];
    if (!v) continue;
    for (const x of v) if (x) fn(x, k);
  }
}

/* Até onde a subárvore vai.
 *
 * `l2` é a linha do `}`, que o parser registra no `Block`. Onde ele não
 * existe, o fim é a maior linha alcançada. A diferença importa: o último
 * COMANDO de uma action não é o fim dela — entre ele e o `}` costuma haver a
 * linha em branco onde o cursor está. */
function ultimaLinha(no) {
  let m = no && no.l ? no.l : 0;
  if (no && no.l2 && no.l2 > m) m = no.l2;
  cada(no, (f) => { const u = ultimaLinha(f); if (u > m) m = u; });
  return m;
}

/* Os nomes que UM nó liga no escopo onde ele está. Enumerado por TIPO DE NÓ —
 * é a lista fechada da gramática, não um palpite sobre o texto. */
function ligacoesDe(no, poe) {
  switch (no.k) {
    case 'Assignment':                                   /* x = 1 */
      poe(no.texto, 'variavel', null, no);
      break;
    case 'VarDecl':                                      /* str x = "a" */
      poe(no.texto, 'variavel', no.texto2, no);
      break;
    case 'ForEachStmt':                                  /* for each it in … */
      poe(no.texto, 'variavel do laco', null, no);
      break;
    case 'ActionDecl':
      poe(no.texto, 'action', no.texto2, no);
      break;
    case 'EntityDecl':
      poe(no.texto, 'class', null, no);
      break;
    case 'ModelDecl':
      poe(no.texto, 'model', null, no);
      break;
    case 'EnumDecl':
      poe(no.texto, 'enum', null, no);
      break;
    case 'UnpackAssignment':                             /* a, b = 1, 2 */
      alvosDoUnpack(no.a, poe, no);
      break;
    case 'ImportStmt':
      /* tratado à parte, em `importsDaArvore` */
      break;
    default:
      break;
  }
}

function alvosDoUnpack(alvo, poe, origem) {
  if (!alvo) return;
  if (alvo.k === 'Name') { poe(alvo.texto, 'variavel', null, origem); return; }
  if (alvo.k === 'UnpackTarget') {
    for (const x of alvo.lista || []) alvosDoUnpack(x, poe, origem);
  }
  /* `l[i], o.x = …` também são alvos, mas ligam MEMBRO, não nome novo */
}

/* ── imports ────────────────────────────────────────────────────────────── */

function importsDaArvore(arvore) {
  const tab = new Map();
  const anda = (no) => {
    if (!no || typeof no !== 'object') return;
    if (no.k === 'ImportStmt') {
      const segs = (no.lista || []).map((x) => x.texto).filter(Boolean);
      if (segs.length) {
        const mod = segs.join('.');
        /* `from mod import a, b` NÃO liga o módulo: liga cada nome pedido. */
        if (no.texto === 'from' || (no.lista2 || []).length) {
          const pedidos = no.lista2 || [];
          const apelidos = no.alias || [];
          for (let i = 0; i < pedidos.length; i++) {
            const nome = pedidos[i] && pedidos[i].texto;
            if (!nome) continue;
            const ap = apelidos[i] && apelidos[i].texto ? apelidos[i].texto : nome;
            tab.set(ap, { mod, membro: nome, de_from: true });
          }
        } else {
          const ligado = no.texto2 ? no.texto2 : segs[segs.length - 1];
          tab.set(ligado, { mod, membro: null, de_from: false });
        }
      }
    }
    cada(no, anda);
  };
  anda(arvore);
  return tab;
}

/* ── Entities ───────────────────────────────────────────────────────────── */

function membrosDaEntity(no) {
  const membros = [];
  const jaTem = new Set();
  const poe = (m) => { if (m.nome && !jaTem.has(m.nome)) { jaTem.add(m.nome); membros.push(m); } };

  for (const f of no.alias || []) {                       /* campos declarados */
    if (f.k !== 'EntityField') continue;
    poe({ nome: f.texto, kind: 'campo', tipo: f.texto2 || '',
          privado: !!f.private, linha: f.l - 1, coluna: f.c - 1 });
  }
  for (const m of no.lista || []) {                       /* métodos */
    if (m.k !== 'ActionDecl') continue;
    const ps = (m.lista || []).filter((p) => p && p.texto && p.texto !== 'self')
                              .map((p) => ({ nome: p.texto, default: null }));
    poe({ nome: m.texto, kind: 'action', params: ps, retorna: m.texto2 || null,
          privado: !!m.private, linha: m.l - 1, coluna: m.c - 1 });
    /* campos que o CORPO cria: `self.x = …` e `private str x = …` */
    const corpo = (no2) => {
      if (!no2 || typeof no2 !== 'object') return;
      if (no2.k === 'MemberAssignment' && no2.a && no2.a.k === 'Name' && no2.a.texto === 'self') {
        poe({ nome: no2.texto, kind: 'campo', tipo: '', privado: false,
              linha: no2.l - 1, coluna: no2.c - 1 });
      }
      if (no2.k === 'FieldDecl') {
        poe({ nome: no2.texto, kind: 'campo', tipo: no2.texto2 || '',
              privado: !!no2.private, linha: no2.l - 1, coluna: no2.c - 1 });
      }
      cada(no2, corpo);
    };
    corpo(m.b);
  }
  return membros;
}

function entidadesDaArvore(arvore) {
  const out = [];
  const anda = (no) => {
    if (!no || typeof no !== 'object') return;
    if (no.k === 'EntityDecl') {
      out.push({
        nome: no.texto,
        bases: (no.lista2 || []).map((x) => x.texto).filter(Boolean),
        membros: membrosDaEntity(no),
        linha: no.l - 1, coluna: no.c - 1,
        ini: no.l, fim: ultimaLinha(no),
      });
    }
    cada(no, anda);
  };
  anda(arvore);
  return out;
}

/* ── escopos ────────────────────────────────────────────────────────────── */
/*
 * Um escopo por `action` (com os parâmetros) e um pelo módulo. O intervalo é
 * em LINHAS, do início do nó até a última linha da subárvore dele.
 */
function escoposDaArvore(arvore) {
  const escopos = [{ tipo: 'modulo', nome: '', ini: 1, fim: ultimaLinha(arvore), liga: [] }];

  const anda = (no, dono) => {
    if (!no || typeof no !== 'object') return;

    if (no.k === 'ActionDecl') {
      const esc = { tipo: 'action', nome: no.texto, ini: no.l, fim: ultimaLinha(no), liga: [] };
      for (const p of no.lista || []) {
        if (p && p.texto) esc.liga.push({ nome: p.texto, kind: 'parametro', tipo: null, linha: p.l - 1, no: p });
      }
      escopos.push(esc);
      /* a action LIGA O PRÓPRIO NOME no escopo de fora. O nó vai junto (`no`):
       * o hover mostra `int async action f(...)` lendo texto2/async dele, em
       * vez de perder os modificadores que a árvore já tem. */
      dono.liga.push({ nome: no.texto, kind: 'action', tipo: no.texto2 || null, async: !!no.async,
                       linha: no.l - 1, no,
                       params: (no.lista || []).filter((p) => p && p.texto !== 'self')
                                               .map((p) => ({ nome: p.texto, default: null })) });
      cada(no, (f) => { if (f !== no.b) return; anda(f, esc); });
      if (no.b) anda(no.b, esc);
      return;
    }

    if (no.k === 'EntityDecl') {
      dono.liga.push({ nome: no.texto, kind: 'class', tipo: null, linha: no.l - 1 });
      /* o corpo da Entity não liga nomes de módulo: os métodos são membros */
      for (const m of no.lista || []) {
        if (m && m.k === 'ActionDecl') {
          const esc = { tipo: 'metodo', nome: m.texto, entidade: no.texto,
                        ini: m.l, fim: ultimaLinha(m), liga: [] };
          for (const p of m.lista || []) {
            if (p && p.texto) esc.liga.push({ nome: p.texto, kind: 'parametro', tipo: null, linha: p.l - 1, no: p });
          }
          escopos.push(esc);
          if (m.b) anda(m.b, esc);
        }
      }
      return;
    }

    ligacoesDe(no, (nome, kind, tipo, origem) => {
      /* o nó de origem viaja na ligação: o hover de um model lê os campos
       * dele dali, sem varrer a árvore de novo */
      if (nome) dono.liga.push({ nome, kind, tipo, linha: (origem.l || 1) - 1, no: origem });
    });
    cada(no, (f) => anda(f, dono));
  };

  anda(arvore, escopos[0]);
  return escopos;
}

/* ── a fachada ──────────────────────────────────────────────────────────── */

function indexa(arvore) {
  if (!arvore) return { arvore: null, escopos: [], entidades: [], imports: new Map() };
  return {
    /* a árvore VIAJA no índice: quem precisa de uma pergunta que o índice não
     * respondeu (qual chamada contém o cursor, o que `x = Foo()` construiu)
     * varre a mesma árvore, em vez de reconstruir de outro jeito. */
    arvore,
    escopos: escoposDaArvore(arvore),
    entidades: entidadesDaArvore(arvore),
    imports: importsDaArvore(arvore),
  };
}

/* Tudo que está em escopo na LINHA `linha` (base 0). Do mais interno pro mais
 * externo, sem repetir nome — sombra funciona como na linguagem. */
function visiveisEm(idx, linha) {
  const L = linha + 1;
  const dentro = idx.escopos.filter((e) => e.ini <= L && L <= e.fim);
  dentro.sort((x, y) => (y.ini - x.ini));            /* mais interno primeiro */
  const out = [];
  const jaTem = new Set();
  for (const e of dentro) {
    for (const b of e.liga) {
      /* só o que foi ligado ANTES do cursor: sugerir variável de baixo é
       * oferecer o que ainda não existe */
      if (b.linha > linha && b.kind !== 'action' && b.kind !== 'class') continue;
      if (jaTem.has(b.nome)) continue;
      jaTem.add(b.nome);
      out.push(b);
    }
  }
  return out;
}

/* O método/Entity que contém a linha — quem responde o `self.`. */
function entidadeEm(idx, linha) {
  const L = linha + 1;
  for (const e of idx.escopos) {
    if (e.tipo === 'metodo' && e.ini <= L && L <= e.fim) {
      return idx.entidades.find((x) => x.nome === e.entidade) || null;
    }
  }
  for (const e of idx.entidades) if (e.ini <= L && L <= e.fim) return e;
  return null;
}

module.exports = { indexa, visiveisEm, entidadeEm, cadeiaAntes, ultimaLinha, cada };
