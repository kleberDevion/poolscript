# ============================================================================
#  pooler — instalador do PoolScript (Windows / PowerShell)
#
#  - SEMPRE pergunta qual motor instalar (menu):
#        INTERP = interpretador em Python (recomendado no Windows)
#        PSVM   = binário em C (só se existir um pool.exe publicado)
#  - antes de instalar, REMOVE qualquer instalação anterior (pool.exe/psl.exe do
#    diretório do PoolScript + entrada no PATH do usuário, e o INTERP via pip)
#  - registra pool/psl no PATH do usuário
#
#  Uso (PowerShell):
#     .\installer\pooler.ps1
#     .\installer\pooler.ps1 -Engine interp -Yes
# ============================================================================
param(
  [ValidateSet('', 'interp', 'psvm')] [string]$Engine = '',
  [switch]$Yes
)
$ErrorActionPreference = 'Stop'
$Repo    = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$InstDir = Join-Path $env:LOCALAPPDATA 'Programs\PoolScript'

function Say($m)  { Write-Host $m }
function OK($m)   { Write-Host "OK $m" -ForegroundColor Green }
function Warn($m) { Write-Host "!  $m" -ForegroundColor Red }

# PSVM no Windows só se houver um pool.exe pronto (o VM em C usa libpq/mongoc —
# normalmente só há build Linux; por isso o padrão do Windows é o INTERP)
$PoolExe = Join-Path $Repo 'dist\pool.exe'
$PsvmOk  = Test-Path $PoolExe

Say "PoolScript - pooler"
Say "sistema: Windows ($env:PROCESSOR_ARCHITECTURE)"

# ── remove instalação anterior ───────────────────────────────────────────────
function Remove-Anterior {
  $removeu = $false
  if (Test-Path $InstDir) {
    Remove-Item -Recurse -Force $InstDir -ErrorAction SilentlyContinue
    $removeu = $true
  }
  # tira a pasta do PATH do usuário
  $userPath = [Environment]::GetEnvironmentVariable('Path', 'User')
  if ($userPath -and $userPath.Split(';') -contains $InstDir) {
    $novo = ($userPath.Split(';') | Where-Object { $_ -ne $InstDir }) -join ';'
    [Environment]::SetEnvironmentVariable('Path', $novo, 'User')
    $removeu = $true
  }
  # INTERP via pip
  & py -m pip show poolscript *> $null
  if ($LASTEXITCODE -eq 0) {
    & py -m pip uninstall -y poolscript *> $null
    $removeu = $true
  }
  if ($removeu) { OK "instalacao anterior removida" } else { Say "(nada instalado antes)" }
}

# ── menu: SEMPRE pergunta ────────────────────────────────────────────────────
function Escolhe-Motor {
  if ($Engine) { return $Engine }
  $rec = 'INTERP'
  Say ""
  Say "Qual motor instalar? (recomendado no Windows: $rec)"
  Say "  1) INTERP - Python 3.10+, portatil (recomendado)"
  $psvmTxt = if ($PsvmOk) { "" } else { " (indisponivel: sem pool.exe publicado)" }
  Say "  2) PSVM   - binario em C$psvmTxt"
  if ($Yes) { return 'interp' }
  $r = Read-Host "escolha [1/2]"
  switch ($r) { '2' { 'psvm' } default { 'interp' } }
}

# ── INTERP via pip ───────────────────────────────────────────────────────────
function Instala-Interp {
  $py = Get-Command py -ErrorAction SilentlyContinue
  if (-not $py) { $py = Get-Command python -ErrorAction SilentlyContinue }
  if (-not $py) { Warn "Python 3.10+ nao encontrado - instale de python.org (ou 'winget install Python.Python.3.12')"; exit 1 }
  & $py.Source -m pip install --user --upgrade $Repo
  OK "INTERP instalado via pip (pool, psl, poolscript-lsp)"
  Say "  (o Scripts do Python ja fica no PATH; se 'pool' nao achar, reabra o terminal)"
}

# ── PSVM (copia o pool.exe, se existir) ──────────────────────────────────────
function Instala-Psvm {
  if (-not $PsvmOk) { Warn "PSVM nao tem pool.exe publicado para Windows - caindo pro INTERP"; Instala-Interp; return }
  New-Item -ItemType Directory -Force -Path $InstDir | Out-Null
  Copy-Item $PoolExe (Join-Path $InstDir 'pool.exe') -Force
  Copy-Item $PoolExe (Join-Path $InstDir 'psl.exe')  -Force
  $userPath = [Environment]::GetEnvironmentVariable('Path', 'User')
  if ($userPath -notlike "*$InstDir*") {
    [Environment]::SetEnvironmentVariable('Path', "$InstDir;$userPath", 'User')
  }
  OK "PSVM instalado em $InstDir (pool.exe, psl.exe) + PATH"
}

# ── execução ─────────────────────────────────────────────────────────────────
$eng = Escolhe-Motor
Say ""; Say "instalando: $eng"
Remove-Anterior
switch ($eng) {
  'psvm'   { Instala-Psvm }
  'interp' { Instala-Interp }
  default  { Warn "motor invalido: $eng"; exit 1 }
}
Say ""
OK "pronto - reabra o terminal e rode: pool --version"
