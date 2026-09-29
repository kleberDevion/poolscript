# Instala a Jinga no Windows: o jinga.exe (e os comandos jpkg, pool e psl),
# as DLLs dos clientes de banco, o servidor do editor (jinga-lsp) e a doc.
#
# Vai pra pasta do usuário, sem pedir administrador:
#   %LOCALAPPDATA%\Programs\Jinga\bin     jinga.exe, os .cmd e as DLLs
#   %LOCALAPPDATA%\Programs\Jinga\share   o servidor do editor e a doc
# e a pasta bin entra no PATH do usuário (vale nos terminais abertos depois).
#
# De dentro do pacote (jinga-windows-x86_64.zip, feito pelo build_bundle.sh)
# ou do repositório com o pool.exe compilado, instala o que está ao lado:
#
#   powershell -ExecutionPolicy Bypass -File instalar.ps1
#   powershell -ExecutionPolicy Bypass -File instalar.ps1 -Remover
#   powershell -ExecutionPolicy Bypass -File instalar.ps1 -Prefixo D:\jinga
#
# Sem nada ao lado, baixa o pacote do último release (`$env:PACOTE` troca a
# origem: o release só responde sem login enquanto o repositório é público).
param(
    [switch]$Remover,
    [string]$Prefixo = (Join-Path $env:LOCALAPPDATA 'Programs\Jinga')
)
$ErrorActionPreference = 'Stop'

$repo = if ($env:REPO) { $env:REPO } else { 'https://github.com/kleberDevion/poolscript' }
$pacoteUrl = if ($env:PACOTE) { $env:PACOTE } else { "$repo/releases/latest/download/jinga-windows-x86_64.zip" }
$bin = Join-Path $Prefixo 'bin'
$share = Join-Path $Prefixo 'share\jinga'

# o PATH do usuário, sem a pasta bin (pra tirar) ou com ela (pra pôr)
function Troca-Path([bool]$poe) {
    $atual = [Environment]::GetEnvironmentVariable('Path', 'User')
    $partes = @()
    if ($atual) { $partes = $atual.Split(';') | Where-Object { $_ -and ($_.TrimEnd('\') -ne $bin.TrimEnd('\')) } }
    if ($poe) { $partes = @($bin) + $partes }
    [Environment]::SetEnvironmentVariable('Path', ($partes -join ';'), 'User')
}

if ($Remover) {
    if (Test-Path $Prefixo) { Remove-Item -Recurse -Force $Prefixo }
    Troca-Path $false
    Write-Output 'removido.'
    exit 0
}

# a origem: o pacote ao lado, o repositório compilado, ou o release baixado
$aqui = $PSScriptRoot
$temp = $null
if (Test-Path (Join-Path $aqui 'bin\jinga.exe')) {
    $origem = 'pacote'
} elseif (Test-Path (Join-Path $aqui 'pool.exe')) {
    $origem = 'repositorio'
} else {
    $temp = Join-Path ([IO.Path]::GetTempPath()) ('jinga-' + [Guid]::NewGuid())
    New-Item -ItemType Directory -Path $temp | Out-Null
    $zip = Join-Path $temp 'jinga.zip'
    Write-Output "baixando $pacoteUrl"
    try {
        Invoke-WebRequest -Uri $pacoteUrl -OutFile $zip -UseBasicParsing
    } catch {
        Write-Error ("nao consegui baixar o pacote ($($_.Exception.Message)). Com o repositorio privado, " +
                     'baixe o jinga-windows-x86_64.zip logado e rode este instalar.ps1 de dentro dele.')
        exit 1
    }
    Expand-Archive -Path $zip -DestinationPath $temp
    $aqui = Join-Path $temp 'jinga-windows-x86_64'
    $origem = 'pacote'
}

New-Item -ItemType Directory -Force -Path $bin, (Join-Path $share 'lsp'), (Join-Path $share 'docs') | Out-Null

if ($origem -eq 'pacote') {
    Copy-Item -Force (Join-Path $aqui 'bin\*') $bin
    Copy-Item -Recurse -Force (Join-Path $aqui 'share\jinga\*') $share
} else {
    Copy-Item -Force (Join-Path $aqui 'pool.exe') (Join-Path $bin 'jinga.exe')
    Get-ChildItem (Join-Path $aqui 'editor\vscode\*.js') |
        Where-Object { $_.Name -ne 'extension.js' -and $_.Name -ne 'teste_servidor.js' } |
        ForEach-Object { Copy-Item -Force $_.FullName (Join-Path $share 'lsp') }
    $nm = Join-Path $aqui 'editor\vscode\node_modules'
    if (Test-Path $nm) { Copy-Item -Recurse -Force $nm (Join-Path $share 'lsp') }
    Copy-Item -Recurse -Force (Join-Path $aqui 'docs\*') (Join-Path $share 'docs')
    # os comandos: o jpkg é o próprio jinga (subcomandos install/list...), e
    # pool/psl são os nomes de antes do rename
    $lsp = "@echo off`r`nrem Servidor LSP da Jinga. Ver docs/lsp.md.`r`n" +
           "where node >nul 2>nul || (echo jinga-lsp precisa do node 1>&2 & exit /b 1)`r`n" +
           "if `"%~1`"==`"`" (node `"%~dp0..\share\jinga\lsp\server.js`" --stdio) else (node `"%~dp0..\share\jinga\lsp\server.js`" %*)`r`n"
    foreach ($c in 'jpkg', 'pool', 'psl') {
        [IO.File]::WriteAllText((Join-Path $bin "$c.cmd"), "@`"%~dp0jinga.exe`" %*`r`n")
    }
    foreach ($c in 'jinga-lsp', 'poolscript-lsp') {
        [IO.File]::WriteAllText((Join-Path $bin "$c.cmd"), $lsp)
    }
}
if ($temp) { Remove-Item -Recurse -Force $temp }

Troca-Path $true
$env:Path = "$bin;$env:Path"
& (Join-Path $bin 'jinga.exe') --version
Write-Output "instalado em $Prefixo (a pasta bin entrou no PATH do usuario: abra um terminal novo)."
if (-not (Get-Command node -ErrorAction SilentlyContinue)) {
    Write-Output 'o servidor do editor (jinga-lsp) precisa do node: instale pelo nodejs.org ou `winget install OpenJS.NodeJS`.'
}
