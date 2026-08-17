; ============================================================================
;  PoolScript — instalador Windows (Inno Setup)
;  Gera um setup.exe que:
;    - instala o pool.exe em "Program Files\PoolScript"
;    - registra a pasta no PATH da MÁQUINA (checkbox, ligado por padrão)
;    - remove tudo (inclusive a entrada do PATH) na desinstalação
;  Branding com a logo da Pool (ícone, painel lateral e ícone pequeno).
;
;  Compilar:  ISCC.exe installer\pool_installer.iss
;  (o pool.exe precisa já existir em dist\ — rode o PyInstaller antes)
; ============================================================================

#define MyAppName      "PoolScript"
#define MyAppVersion   "8.2.61"
#define MyAppPublisher "PoolScript"
#define MyAppExeName   "pool.exe"

[Setup]
; AppId identifica o produto para upgrades/desinstalação — NÃO mude entre versões.
AppId={{B7E9C3A1-4F2D-4E8B-9A6C-1D2E3F4A5B6C}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
VersionInfoVersion={#MyAppVersion}

; Instala para todos os usuários -> precisa de admin -> pode mexer no PATH da máquina.
PrivilegesRequired=admin
DefaultDirName={autopf}\{#MyAppName}
DisableProgramGroupPage=yes
DisableDirPage=no

; Só faz sentido em Windows 64 bits (o exe é x64).
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible

; Saída
OutputDir=..\dist
OutputBaseFilename=PoolScript-{#MyAppVersion}-Setup
Compression=lzma2/max
SolidCompression=yes

; Aparência + branding (logo da Pool)
WizardStyle=modern
SetupIconFile=assets\pool.ico
WizardImageFile=assets\wizard_large.bmp
WizardSmallImageFile=assets\wizard_small.bmp
UninstallDisplayIcon={app}\{#MyAppExeName}
UninstallDisplayName={#MyAppName} {#MyAppVersion}

; Necessário para o Windows avisar os processos que o PATH mudou (WM_SETTINGCHANGE)
ChangesEnvironment=yes
; Faz o Inno avisar o Explorer que as associações de arquivo mudaram (atualiza ícones)
ChangesAssociations=yes

[Languages]
Name: "brazilianportuguese"; MessagesFile: "compiler:Languages\BrazilianPortuguese.isl"
Name: "english";             MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "addtopath"; \
  Description: "Adicionar o PoolScript ao PATH do sistema (recomendado — permite rodar 'pool' em qualquer terminal)"; \
  GroupDescription: "Integração com o sistema:"
Name: "associate"; \
  Description: "Associar arquivos .ps ao PoolScript (ícone da Pool no Explorer + duplo-clique executa)"; \
  GroupDescription: "Integração com o sistema:"

[Files]
Source: "..\dist\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion
; A logo em .ico acompanha a instalação para servir de ícone dos arquivos .ps
Source: "assets\pool.ico"; DestDir: "{app}"; Flags: ignoreversion

[Registry]
; ── PATH da máquina ──────────────────────────────────────────────────────────
; Acrescenta {app} ao PATH da máquina — só se a task estiver marcada E ainda não estiver lá.
Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Control\Session Manager\Environment"; \
  ValueType: expandsz; ValueName: "Path"; ValueData: "{olddata};{app}"; \
  Tasks: addtopath; Check: NeedsAddPath(ExpandConstant('{app}'))

; ── Associação do .ps (ícone + abrir) ────────────────────────────────────────
; HKA = HKLM\Software\Classes numa instalação admin (vale pra todos os usuários).
; A extensão .ps aponta para o ProgID "PoolScript.psfile", que define nome,
; ícone (a logo da Pool) e o comando de abrir (roda com o pool.exe).
Root: HKA; Subkey: "Software\Classes\.ps"; ValueType: string; ValueName: ""; \
  ValueData: "PoolScript.psfile"; Flags: uninsdeletevalue; Tasks: associate
Root: HKA; Subkey: "Software\Classes\PoolScript.psfile"; ValueType: string; ValueName: ""; \
  ValueData: "Código-fonte PoolScript"; Flags: uninsdeletekey; Tasks: associate
Root: HKA; Subkey: "Software\Classes\PoolScript.psfile\DefaultIcon"; ValueType: string; ValueName: ""; \
  ValueData: "{app}\pool.ico"; Tasks: associate
Root: HKA; Subkey: "Software\Classes\PoolScript.psfile\shell\open\command"; ValueType: string; ValueName: ""; \
  ValueData: """{app}\{#MyAppExeName}"" ""%1"""; Tasks: associate

[Code]
const
  EnvironmentKey = 'SYSTEM\CurrentControlSet\Control\Session Manager\Environment';

{ Retorna True se a pasta de instalacao ainda NAO esta no PATH (evita duplicar). }
function NeedsAddPath(Param: string): Boolean;
var
  OrigPath: string;
begin
  if not RegQueryStringValue(HKLM, EnvironmentKey, 'Path', OrigPath) then
  begin
    Result := True;
    exit;
  end;
  { compara case-insensitive, cercando com ';' pra casar o segmento inteiro }
  Result := Pos(';' + Uppercase(Param) + ';', ';' + Uppercase(OrigPath) + ';') = 0;
end;

{ Na desinstalacao, remove a pasta do PATH (inclusive se estiver duplicada). }
procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
var
  Path, AppDir: string;
begin
  if CurUninstallStep <> usPostUninstall then
    exit;

  AppDir := ExpandConstant('{app}');
  if not RegQueryStringValue(HKLM, EnvironmentKey, 'Path', Path) then
    exit;

  { cerca com ';' dos dois lados, remove ';AppDir;' e desfaz a cerca }
  Path := ';' + Path + ';';
  StringChangeEx(Path, ';' + AppDir + ';', ';', True);
  Delete(Path, 1, 1);
  if (Length(Path) > 0) and (Path[Length(Path)] = ';') then
    Delete(Path, Length(Path), 1);

  RegWriteExpandStringValue(HKLM, EnvironmentKey, 'Path', Path);
end;
