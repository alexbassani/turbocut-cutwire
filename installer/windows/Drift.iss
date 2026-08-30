#ifndef MyAppVersion
  #define MyAppVersion "0.0.0"
#endif

#ifndef MyAppSource
  #define MyAppSource "dist\\bin"
#endif

#define MyAppName "TurboCut"
#define MyAppPublisher "Alex Bassani"
#define MyAppExeName "turbocut.exe"

; TurboCut: o runtime do Visual C++ vai JUNTO no instalador. Sem ele, um Windows
; "limpo" (sem Visual Studio, sem outros apps C++) abre o TurboCut com
; "MSVCP140_1.dll não foi encontrado" — relato real de uma usuária em 29/08/2026.
; O upstream não empacota isso; o CI baixa o redist antes de compilar o instalador.
#define VcRedist "vc_redist.x64.exe"
#if FileExists(AddBackslash(SourcePath) + VcRedist)
  #define HaveVcRedist
#endif

[Setup]
; Never change AppId: it is what lets an installer upgrade an existing install
; in place instead of leaving two copies behind.
; TurboCut fork: own AppId, so it can never upgrade over (or be upgraded by) an
; upstream Drift install on the same machine.
AppId={{DC0EC0C4-090C-4519-A3EF-33E9B201A062}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppSupportURL=https://github.com/cpmdark/turbocut-cutwire/issues
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
Compression=lzma
SolidCompression=yes
WizardStyle=modern
; Path is relative to this script. Without these two, setup runs under the stock
; Inno icon and the Apps & Features entry falls back to a generic one.
SetupIconFile=..\..\resources\windows\drift.ico
WizardImageFile=..\..\resources\windows\wizard-large.bmp
WizardSmallImageFile=..\..\resources\windows\wizard-small.bmp
UninstallDisplayIcon={app}\{#MyAppExeName}
ChangesAssociations=yes
OutputDir=output
OutputBaseFilename=TurboCut-Setup-x64

[Languages]
Name: "brazilianportuguese"; MessagesFile: "compiler:Languages\BrazilianPortuguese.isl"
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
; Recursive: alongside the exe and its Qt runtime this carries the bundled
; effects\, transitions\, effect-templates\ and audio-effects\ package
; directories, which the app resolves relative to the executable.
; CI stages the binary as drift.exe; the install renames it to turbocut.exe.
Source: "{#MyAppSource}\*"; DestDir: "{app}"; Excludes: "\drift.exe"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#MyAppSource}\drift.exe"; DestDir: "{app}"; DestName: "{#MyAppExeName}"; Flags: ignoreversion
#ifdef HaveVcRedist
; Vai para a pasta temporária e é apagado no fim: só serve durante a instalação.
Source: "{#VcRedist}"; DestDir: "{tmp}"; Flags: deleteafterinstall
#endif

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Registry]
Root: HKCR; Subkey: ".drift"; ValueType: string; ValueName: ""; ValueData: "TurboCut.Project"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "TurboCut.Project"; ValueType: string; ValueName: ""; ValueData: "Projeto do TurboCut"; Flags: uninsdeletekey
Root: HKCR; Subkey: "TurboCut.Project\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\{#MyAppExeName},0"
Root: HKCR; Subkey: "TurboCut.Project\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#MyAppExeName}"" ""%1"""

[Run]
#ifdef HaveVcRedist
; Roda ANTES do app: silencioso, sem reiniciar, e só quando falta de verdade.
Filename: "{tmp}\{#VcRedist}"; Parameters: "/install /quiet /norestart"; \
    StatusMsg: "Instalando componentes do Windows (Visual C++)..."; \
    Check: PrecisaVcRedist; Flags: waituntilterminated
#endif
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#MyAppName}}"; Flags: nowait postinstall skipifsilent

#ifdef HaveVcRedist
[Code]
// Checa exatamente o que falhou no relato: a DLL do runtime em System32.
// Se ela existe, o redist já está instalado e pulamos (instalação mais rápida).
function PrecisaVcRedist: Boolean;
begin
  Result := not FileExists(ExpandConstant('{sys}\msvcp140_1.dll'));
end;
#endif
