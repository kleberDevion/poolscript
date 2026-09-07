# Objetos internos da linguagem

Tipos que você **não cria na mão** — cada um é o que uma lib te
entrega pronto (o `type(obj)` mostra esse nome). Um índice só,
todos juntos.

Cinco nomes desta lista **não são tipo da VM**, e a página de cada um explica o
que existe no lugar: `DataEntityMeta`, `MiddlewareRegistrar`, `PoolIp`,
`QRCode` e `Route`. A lista viva de tipos sai de `pool --metadata`.

Onde uma página mostra **valor padrão** de parâmetro (`.emit(payload=None, …)`,
`.options(subset=None)`, `.make(fit=True)`), o padrão é o que está escrito
aqui: o `--metadata` publica os nomes dos parâmetros, mas **não** os defaults
de função de módulo. Omitir esses argumentos funciona.

| Objeto | Vem da lib |
|---|---|
| [`ChannelManager`](ChannelManager.md) | `jinker` |
| [`ChannelStatus`](ChannelStatus.md) | `jinker` |
| [`CorsConfig`](CorsConfig.md) | `jinker` |
| [`DataEntityMeta`](DataEntityMeta.md) | `datasentity` |
| [`DbConnection`](../psodbc/DbConnection/DbConnection.md) | `psodbc` |
| [`DbCursor`](../psodbc/DbCursor/DbCursor.md) | `psodbc` |
| [`Jinker`](../jinker/Jinker/Jinker.md) | `jinker` |
| [`JinkerRequest`](JinkerRequest.md) | `jinker` |
| [`JinkerResponse`](../jinker/JinkerResponse/JinkerResponse.md) | `jinker` |
| [`MailMessage`](../mail/MailMessage/MailMessage.md) | `mail` |
| [`MailReader`](../mail/MailReader/MailReader.md) | `mail` |
| [`MailServer`](../mail/MailServer/MailServer.md) | `mail` |
| [`ManpuFile`](../manpu/ManpuFile/ManpuFile.md) | `manpu` |
| [`ManpuResult`](ManpuResult.md) | `manpu` |
| [`MiddlewareRegistrar`](MiddlewareRegistrar.md) | `jinker` |
| [`MongoCollection`](../psodbc/MongoCollection/MongoCollection.md) | `psodbc` |
| [`MongoConnection`](../psodbc/MongoConnection/MongoConnection.md) | `psodbc` |
| [`PoolConnection`](PoolConnection.md) | `sqlite3` |
| [`PoolCursor`](PoolCursor.md) | `sqlite3` |
| [`PoolFile`](../os/PoolFile/PoolFile.md) | `os` |
| [`PoolFileUpload`](PoolFileUpload.md) | `jinker` |
| [`PoolIp`](PoolIp.md) | `jinker` |
| [`PoolQRCode`](PoolQRCode.md) | `qrcode` |
| [`QRCode`](QRCode.md) | `qrcode` |
| [`QRImage`](../qrcode/QRImage/QRImage.md) | `qrcode` |
| [`QRPoolFile`](QRPoolFile.md) | `qrcode` |
| [`RequestProxy`](RequestProxy.md) | `jinker` |
| [`Response`](../request/Response/Response.md) | `request` |
| [`Route`](Route.md) | `jinker` |
| [`SocketEmitter`](SocketEmitter.md) | `jinker` |
| [`SocketNamespace`](SocketNamespace.md) | `jinker` |
| [`WsConnection`](../request/WsConnection/WsConnection.md) | `request` |
