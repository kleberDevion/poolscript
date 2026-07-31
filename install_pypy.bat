@echo off
:: PoolScript — Instalação com PyPy (Windows)
:: ==========================================
setlocal

echo PoolScript — Instalação com PyPy
echo =================================

:: Detecta PyPy
set PYPY=
for %%c in (pypy3 pypy pypy3.exe) do (
    where %%c >nul 2>&1 && (
        set PYPY=%%c
        goto :found
    )
)

echo PyPy não encontrado. Instalando com Python como fallback.
set PYPY=python
goto :install

:found
echo PyPy encontrado: %PYPY%

:install
echo.
echo Instalando dependências...
%PYPY% -m pip install --quiet requests PyJWT websockets python-dotenv cryptography 2>nul

echo Instalando PoolScript...
%PYPY% -m pip install --quiet -e "%~dp0"

echo.
echo Instalação concluída!
%PYPY% -m poolscript --version
