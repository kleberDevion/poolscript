@echo off
REM PoolScript Installer - Windows
REM Remove versoes antigas e instala a nova (Python package + extensao VSCode)
setlocal enabledelayedexpansion

set "SCRIPT_DIR=%~dp0"
set "VSIX_FILE=%SCRIPT_DIR%vscode-poolscript\poolscript.poolscript-0.5.2.vsix"

echo =========================================
echo   PoolScript Installer v0.5.2
echo =========================================

REM ---- 1. Detecta Python ----
where python >nul 2>nul
if %errorlevel% neq 0 (
    echo [ERRO] Python nao encontrado no PATH.
    echo Instale Python 3.10+ em https://python.org
    pause
    exit /b 1
)
for /f "tokens=*" %%i in ('python --version') do echo [OK] %%i

REM ---- 2. Remove versoes antigas Python ----
echo.
echo [1/3] Removendo versoes antigas do PoolScript (Python)...
python -m pip uninstall -y poolscript 2>nul
if %errorlevel% neq 0 echo   (nenhuma versao anterior encontrada)

REM Remove pastas residuais site-packages
for /f "tokens=*" %%s in ('python -c "import site; print(site.getusersitepackages())" 2^>nul') do (
    if exist "%%s\poolscript" rmdir /s /q "%%s\poolscript" 2>nul && echo   removido: %%s\poolscript
    for /d %%d in ("%%s\poolscript-*.dist-info") do rmdir /s /q "%%d" 2>nul && echo   removido: %%d
    for /d %%d in ("%%s\poolscript-*.egg-info") do rmdir /s /q "%%d" 2>nul && echo   removido: %%d
)

REM ---- 3. Instala nova versao ----
echo.
echo [2/3] Instalando PoolScript v0.5.2...
python -m pip install --user --upgrade "%SCRIPT_DIR%"
if %errorlevel% neq 0 (
    echo [ERRO] Falha ao instalar.
    pause
    exit /b 1
)
echo [OK] Comandos disponiveis: pool, psl

REM ---- 4. Extensao VSCode ----
echo.
echo [3/3] Instalando extensao VSCode...
where code >nul 2>nul
if %errorlevel% neq 0 (
    echo [SKIP] comando 'code' nao encontrado no PATH.
    echo   Para habilitar: VSCode ^> Ctrl+Shift+P ^> 'Shell Command: Install code in PATH'
    goto :end
)

REM Remove extensoes antigas
for %%e in (poolscript.poolscript poolscript.poolscript-language) do (
    code --list-extensions 2>nul | findstr /i "%%e" >nul
    if !errorlevel! equ 0 (
        echo   removendo extensao antiga: %%e
        code --uninstall-extension %%e 2>nul
    )
)

REM Remove pastas residuais
for /d %%d in ("%USERPROFILE%\.vscode\extensions\poolscript*") do (
    rmdir /s /q "%%d" 2>nul && echo   removida pasta: %%d
)

REM Instala nova
if exist "%VSIX_FILE%" (
    code --install-extension "%VSIX_FILE%" --force
    echo [OK] Extensao VSCode instalada.
) else (
    echo [AVISO] .vsix nao encontrado: %VSIX_FILE%
)

:end
echo.
echo =========================================
echo   Instalacao concluida!
echo =========================================
echo   Teste:  pool --version
echo           pool examples\01_hello.ps
echo.
pause
