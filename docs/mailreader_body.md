# Corpo de e-mail no `MailReader` — o que tinha, o que faltava, e por quê

Isso aqui não é um tutorial fofinho. É a explicação direta do que estava
quebrado no `MailReader`, por que estava quebrado daquele jeito específico,
e o que foi feito pra resolver. Se você só quer o código, pula pra "API".
Se quer entender *por que* o código é assim e não de outro jeito óbvio-mas-errado,
leia tudo.

## 1. O bug conceitual: `.search()` nunca baixava o corpo

O `.search()` pedia ao servidor só isto:

```
FETCH <id> (RFC822.HEADER)
```

`RFC822.HEADER` é um comando IMAP que diz pro servidor: "me manda só os
cabeçalhos dessa mensagem — From, Subject, Date, etc." O corpo nunca sai do
servidor. Não é um bug de parsing, não é um bug de decodificação, é o
protocolo fazendo exatamente o que foi pedido: **nada de body foi
solicitado, nada de body voltou.**

Isso não foi um erro de implementação — foi uma decisão de design que
ficou incompleta. `RFC822.HEADER` existe porque baixar corpo inteiro de
toda mensagem que bate num filtro é caro: se seu `SUBJECT` bater em 300
e-mails, você não quer 300 fetches completos só pra montar uma lista com
remetente e assunto. Isso é o comportamento certo pra `.search()`. O que
faltava era **uma segunda via** pra quando você realmente quer o corpo.

## 2. Duas vias, não uma — e a diferença importa

Tinha duas formas óbvias de resolver isso, e as duas foram implementadas
porque servem casos diferentes. Se você usar a errada pro seu caso, vai
funcionar, só vai ser mais lento ou mais chato de escrever do que
precisava.

### `.body(id)` — sob demanda, um e-mail por vez

Exige um `.select()` antes (senão levanta) e faz um `FETCH <id> (RFC822)` —
a mensagem INTEIRA — extraindo o corpo dela.

Uso:

```
emails = reader.select("INBOX").search("SUBJECT", "fatura")
for each e in emails:
    if "urgente" in e["subject"]:
        corpo = reader.body(e["id"])   // só busca o corpo do que interessa
```

Isso é o caminho certo quando a busca pode trazer muita coisa e você só
vai realmente ler o corpo de uma fração. O `id` que você passa aqui é
exatamente o mesmo que `.search()` já te devolveu — é a chave que liga
"esse e-mail bateu no filtro" com "agora eu quero o conteúdo dele". Não
tem estado escondido, não tem cache, é um fetch novo no servidor toda vez
que você chama. Se você chamar `.body()` duas vezes pro mesmo id, faz
duas viagens de rede. Isso é intencional — a lib não vai adivinhar se
você quer cache ou não.

### `.search(..., include_body=True)` — tudo de uma vez

O `FETCH` da busca passa a pedir `(RFC822)` em vez de `(RFC822.HEADER)`, e
cada item do resultado ganha a chave `"body"`.

Uso:

```
emails = reader.select("INBOX").search("UNSEEN", include_body=true)
for each e in emails:
    post(e["subject"] ": " e["body"])
```

Isso é o caminho certo quando você já sabe, antes de rodar a busca, que
vai processar o corpo de tudo que voltar — por exemplo, um script que lê
todo e-mail não-lido e extrai algum dado. Aqui não tem segunda ida ao
servidor por e-mail: o `fetch` já pede `RFC822` completo na primeira (e
única) passada.

**A escolha errada mais comum vai ser usar `include_body=True` numa busca
ampla tipo `"ALL"` sem `limit`.** Se sua caixa tem 5000 mensagens, isso
baixa 5000 corpos completos de uma vez. Ninguém vai te impedir de fazer
isso, mas o servidor IMAP e sua conexão vão sentir. Combine
`include_body` com `limit` se não tiver certeza do volume.

## 3. Como o corpo é extraído — e por que HTML puro não vira texto

O motor anda pelas partes MIME da mensagem, pula os anexos, guarda a
primeira `text/plain` e a primeira `text/html`, e devolve a `text/plain` se
existir — senão a `text/html`, senão `""`.

Regras, sem meio-termo:

- **`text/plain` sempre ganha de `text/html` quando os dois existem.** A
  maioria dos clientes de e-mail manda `multipart/alternative` com as
  duas versões da mesma mensagem — pegar a versão texto é o que qualquer
  script quer, porque HTML cru cheio de `<div style="...">` não serve pra
  nada em log ou em `post()`.
- **Se só existir HTML, você recebe HTML cru.** Essa função não faz
  strip de tags, não roda parser de HTML, não tenta converter pra texto
  legível. Por quê? Porque "converter HTML pra texto legível direito" é
  um problema em si, e enfiar isso aqui seria resolver um problema que
  ninguém pediu, com uma solução que ia estar errada pra metade dos
  casos (tabelas, links, formatação). Se você precisa disso, trate o
  `body` retornado como HTML no seu próprio código.
- **Anexos são pulados.** Uma parte com `Content-Disposition: attachment`
  nunca vira corpo, mesmo que o `Content-Type` seja `text/plain` (sim,
  isso acontece — gente anexa `.txt` e `.csv`). Se checássemos só
  `content_type == "text/plain"` sem olhar `Content-Disposition`, um
  anexo `.txt` de 2MB ia virar "o corpo do e-mail" por acidente.
- **Charset por parte, não por mensagem.** Cada parte MIME pode declarar
  seu próprio charset (`Content-Type: text/plain; charset=iso-8859-1`,
  por exemplo). A decodificação usa o charset daquela parte específica e cai
  pra `utf-8` só se a parte não declarar nenhum. Assumir UTF-8 pra
  tudo ia quebrar silenciosamente com e-mail antigo em Latin-1, que ainda
  existe por aí.

## 4. `SUBJECT "texto, com vírgula e ACENTO"` — o que já funcionava e o que não

Pergunta original: dá pra usar `SUBJECT` com termo em maiúscula/minúscula
misturada e com vírgula? Resposta: **sim, sempre funcionou**, e não tem
nada nessa lib fazendo isso funcionar — é o protocolo IMAP.

RFC 3501 define `SEARCH SUBJECT <string>` como *substring match,
case-insensitive*, ponto final. O servidor é quem garante isso, não o
cliente. Vírgula dentro do termo nunca foi problema porque o termo inteiro
vai entre aspas como uma única `astring` do protocolo — vírgula ali é só
um caractere como outro qualquer, não é delimitador de nada.

**O que estava genuinamente quebrado** era outra coisa, que parecia
relacionada mas não era: o charset da busca.

Antes o comando ia sem `CHARSET`:

```
SEARCH SUBJECT "Relatório"
```

Sem `CHARSET` explícito, o default do protocolo é **US-ASCII**. Assunto de e-mail em português quase sempre tem
acento. `"Relatório"` não é ASCII. Dependendo do servidor, isso ou falha
com erro, ou (pior) simplesmente não dá match em nada e você acha que o
e-mail não existe.

Segundo problema, menor mas real: o termo entrava cru dentro das aspas,
sem escapar nada. Se `term` contém uma aspa dupla —
`Assunto com "citação" dentro` — isso gera um comando IMAP com aspas
desbalanceadas e quebra a sintaxe do protocolo, não só "não encontra
nada", quebra o comando inteiro.

**Correção** — o comando passou a ser:

```
SEARCH CHARSET UTF-8 SUBJECT "Relat\u00f3rio"
```

Duas mudanças, cada uma resolvendo um problema diferente:

1. `CHARSET UTF-8` declarado, então acento passa a funcionar contra
   servidores que o suportam na busca (Gmail, Outlook, Yahoo — todo provedor
   grande suporta; é parte do RFC desde 2003, não é feature exótica).
2. O termo é escapado antes de entrar entre aspas — `\` e `"` viram `\\` e
   `\"`, o backslash primeiro, sempre, senão você escapa a aspa que acabou de
   escapar o backslash.

Resumindo a pergunta original: **case misto sempre funcionou, vírgula
sempre funcionou, o que não funcionava era acento — e agora funciona.**

## 5. API — referência rápida

```
reader = mail.MailReader()
reader.conn("gmail.com")
reader.login("user@gmail.com", "senha-de-app")
reader.select("INBOX", true)          // readonly=true por padrão

// busca só metadado (rápido, é o default)
emails = reader.search("SUBJECT", "Relatório, urgente")

// corpo sob demanda, um de cada vez
corpo = reader.body(emails[0]["id"])

// ou corpo já embutido em todos os resultados
com_corpo = reader.select().search("SUBJECT", "fatura", limit=20, include_body=true)
post(com_corpo[0]["body"])

reader.close()
```

| Método | O que baixa | Quando usar |
|---|---|---|
| `.search(...)` | headers only | padrão — listar, filtrar, decidir o que abrir |
| `.search(..., include_body=True)` | headers + corpo de tudo que bateu | você já sabe que vai ler o corpo de todos |
| `.body(id)` | corpo de um e-mail | você só vai abrir alguns dos resultados |

## 6. O que isso NÃO faz (de propósito)

- Não faz parsing de anexos binários pra fora do corpo — `.body()` te dá
  texto, não arquivos anexados. Anexo é outro problema, não resolvido
  aqui.
- Não faz strip de HTML. Já foi dito acima, repetindo porque é a dúvida
  mais óbvia de quem for usar isso: se o e-mail é HTML puro, você recebe
  HTML puro.
- Não combina múltiplos critérios de busca (`SUBJECT` + `FROM` na mesma
  chamada). Isso já era limitação de antes desta mudança e continua
  sendo — não foi tocado aqui.
- Não cacheia corpo nenhum. Cada `.body(id)` é uma viagem nova ao
  servidor. Se você vai reler o mesmo id várias vezes, guarde o resultado
  você mesmo.

## 7. Testes

A suíte em C (`teste/`, rodada por `make check`) cobre:

- `include_body=true` de fato pede `(RFC822)` em vez de `(RFC822.HEADER)`,
  e sem ele o campo `"body"` nem aparece no dict de resultado.
- `.body()` com mensagem `multipart/alternative` (plain + html) devolve o
  plain.
- `.body()` com mensagem só-HTML devolve o HTML cru.
- `.body()` sem `.select()` antes levanta (mesma regra defensiva de
  `.search()`).
- `.body()` com fetch que falha no servidor levanta.
- Escaping de aspas e barra invertida no termo de busca.
- Termo acentuado com vírgula chega intacto no comando `SEARCH`, com
  `CHARSET UTF-8`.
