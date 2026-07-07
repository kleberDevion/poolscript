@echo off
REM PoolScript Uninstaller - Windows
echo Desinstalando PoolScript...

python -m pip uninstall -y poolscript 2>nul

for /f "tokens=*" %%s in ('python -c "import site; print(site.getusersitepackages())" 2^>nul') do (
    if exist "%%s\poolscript" rmdir /s /q "%%s\poolscript" 2>nul
    for /d %%d in ("%%s\poolscript-*.dist-info") do rmdir /s /q "%%d" 2>nul
    for /d %%d in ("%%s\poolscript-*.egg-info") do rmdir /s /q "%%d" 2>nul
)

where code >nul 2>nul
if %errorlevel% equ 0 (
    code --uninstall-extension poolscript.poolscript 2>nul
    code --uninstall-extension poolscript.poolscript-language 2>nul
)
for /d %%d in ("%USERPROFILE%\.vscode\extensions\poolscript*") do rmdir /s /q "%%d" 2>nul

echo PoolScript removido.
pause
