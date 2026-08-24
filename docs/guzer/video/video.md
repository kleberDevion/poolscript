# `video(typeinp=, placeholder=, value=, name=, href=, src=, alt=, target=, forid=, action=, methd=, rows=, cols=, onclick=)`

**Reproduz o vídeo NA JANELA** do arquivo do `src=` — de onde você quiser (absoluto, relativo ao script,
ou ao diretório atual). A decodificação usa o **ffmpeg** do sistema
(subprocesso): sem ele instalado, a caixa mostra o aviso e o resto do app segue
normal (`sudo apt install ffmpeg`).

- **vídeo**: os frames entram na caixa do elemento, escalados pro
  `width`/`height` dela; a trilha sonora toca junto (ffplay).
- **áudio**: toca ao abrir a janela; a caixa mostra o `.text()` ou ♪ + o nome
  do arquivo.

```
import guzer
app = guzer.UI("Player")
app.window().stylesheet({ "width": "680", "height": "520" })
app.video(src="filme.mp4").stylesheet({ "width": "640", "height": "360" })
app.audio(src="musica.mp3")
```

[← índice](../guzer.md)
