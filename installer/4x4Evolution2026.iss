; 4x4 Evolution 2026 Edition - all-in-one installer
; Game (Build 57, from a personal disc) + evo2026 HD renderer + HD texture pack.
; Build: ISCC.exe 4x4Evolution2026.iss   (Inno Setup 6.4+)

#define AppName      "4x4 Evolution 2026 Edition"
#define AppVersion   "1.1.0"
#define AppPublisher "MontyLabs"
#define AppExe       "4x4.exe"

[Setup]
AppId={{9DA5E70C-AD34-47A6-ABD8-9FCF8877AC7E}
AppName={#AppName}
AppVersion={#AppVersion}
AppVerName={#AppName} {#AppVersion}
AppPublisher={#AppPublisher}
AppComments=4x4 Evolution Build 57 with the evo2026 native-HD renderer
DefaultDirName={sd}\Games\4x4 Evolution
DefaultGroupName=4x4 Evolution 2026
DisableProgramGroupPage=yes
UsePreviousAppDir=yes
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog
MinVersion=6.1
WizardStyle=modern
DisableWelcomePage=no
WizardImageFile=art\wiz_small_dpi.bmp,art\wiz_big.bmp
WizardSmallImageFile=art\hdr_small.bmp,art\hdr_big.bmp
SetupIconFile=art\game.ico
UninstallDisplayIcon={app}\{#AppExe}
UninstallDisplayName={#AppName}
Compression=lzma2/max
SolidCompression=yes
LZMAUseSeparateProcess=yes
; Split into slices under 25 MB (setup.exe + .bin files must stay together).
DiskSpanning=yes
DiskSliceSize=24000000
OutputDir=output
OutputBaseFilename=4x4Evolution2026-Setup
VersionInfoVersion=1.0.0.0
VersionInfoDescription={#AppName} Setup

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Messages]
WelcomeLabel2=This will install 4x4 Evolution (Build 57) with the evo2026 renderer: native-resolution 3D, borderless fullscreen, 16x anisotropic filtering and AI-upscaled HD textures.%n%nNo disc, patch or extra setup needed.%n%nBuilt from a personal copy of the game for personal use - please don't redistribute it.

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"

[Files]
; Game, patched to Build 57, with evo2026 installed as TRIDX7.DLL (original kept as TRIDX7.original.dll)
Source: "payload\game\*"; DestDir: "{app}"; Excludes: "\system\*,\textures\*"; Flags: ignoreversion recursesubdirs createallsubdirs
; Settings/profiles: keep the user's copies on reinstall (keys are enforced below in [INI])
Source: "payload\game\system\METAL.INI"; DestDir: "{app}\system"; Flags: onlyifdoesntexist
Source: "payload\game\system\RENDER.INI"; DestDir: "{app}\system"; Flags: onlyifdoesntexist
Source: "payload\game\system\evo2026.ini"; DestDir: "{app}\system"; Flags: onlyifdoesntexist
; HD textures (PNG is already compressed)
Source: "payload\game\textures\*"; DestDir: "{app}\textures"; Flags: ignoreversion nocompression

[INI]
; The renderer type is chosen by DLL file name; these must be exactly so.
Filename: "{app}\system\METAL.INI"; Section: "Graphics"; Key: "rendererDLLPath"; String: "tridx7.dll"
Filename: "{app}\system\METAL.INI"; Section: "Graphics"; Key: "useDirect3D"; String: "1"
Filename: "{app}\system\METAL.INI"; Section: "Graphics"; Key: "gameBPP"; String: "32"
Filename: "{app}\system\METAL.INI"; Section: "Graphics"; Key: "gamePIXX"; String: "640"
Filename: "{app}\system\METAL.INI"; Section: "Graphics"; Key: "gamePIXY"; String: "480"
; evo2026 settings live in system\evo2026.ini (the game rewrites render.ini on exit).
; Keys are only added when missing, so the user's tweaks survive a reinstall/upgrade.
Filename: "{app}\system\evo2026.ini"; Section: "evo2026"; Key: "Width"; String: "0"; Flags: createkeyifdoesntexist
Filename: "{app}\system\evo2026.ini"; Section: "evo2026"; Key: "Height"; String: "0"; Flags: createkeyifdoesntexist
Filename: "{app}\system\evo2026.ini"; Section: "evo2026"; Key: "Window"; String: "2"; Flags: createkeyifdoesntexist
Filename: "{app}\system\evo2026.ini"; Section: "evo2026"; Key: "Aspect"; String: "1"; Flags: createkeyifdoesntexist
Filename: "{app}\system\evo2026.ini"; Section: "evo2026"; Key: "VSync"; String: "1"; Flags: createkeyifdoesntexist
Filename: "{app}\system\evo2026.ini"; Section: "evo2026"; Key: "Anisotropy"; String: "16"; Flags: createkeyifdoesntexist
Filename: "{app}\system\evo2026.ini"; Section: "evo2026"; Key: "Filter2D"; String: "1"; Flags: createkeyifdoesntexist
Filename: "{app}\system\evo2026.ini"; Section: "evo2026"; Key: "Widescreen"; String: "1"; Flags: createkeyifdoesntexist
Filename: "{app}\system\evo2026.ini"; Section: "evo2026"; Key: "HudToEdges"; String: "1"; Flags: createkeyifdoesntexist
Filename: "{app}\system\evo2026.ini"; Section: "evo2026"; Key: "ReplaceTextures"; String: "1"; Flags: createkeyifdoesntexist
Filename: "{app}\system\evo2026.ini"; Section: "evo2026"; Key: "DumpTextures"; String: "0"; Flags: createkeyifdoesntexist
Filename: "{app}\system\evo2026.ini"; Section: "evo2026"; Key: "Trace"; String: "0"; Flags: createkeyifdoesntexist

[Registry]
; Stop Windows from DPI-scaling (blurring) the game window.
Root: HKCU; Subkey: "Software\Microsoft\Windows NT\CurrentVersion\AppCompatFlags\Layers"; ValueType: string; ValueName: "{app}\{#AppExe}"; ValueData: "~ HIGHDPIAWARE"; Flags: uninsdeletevalue

[Icons]
Name: "{group}\4x4 Evolution"; Filename: "{app}\{#AppExe}"; WorkingDir: "{app}"
Name: "{group}\evo2026 Settings"; Filename: "{win}\notepad.exe"; Parameters: """{app}\system\evo2026.ini"""; WorkingDir: "{app}"
Name: "{group}\Read Me"; Filename: "{win}\notepad.exe"; Parameters: """{app}\evo2026\README.txt"""; WorkingDir: "{app}"
Name: "{group}\{cm:UninstallProgram,{#AppName}}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\4x4 Evolution"; Filename: "{app}\{#AppExe}"; WorkingDir: "{app}"; Tasks: desktopicon

[Run]
Filename: "{win}\notepad.exe"; Parameters: """{app}\evo2026\README.txt"""; Description: "View the Read Me"; Flags: postinstall nowait skipifsilent unchecked
Filename: "{app}\{#AppExe}"; WorkingDir: "{app}"; Description: "{cm:LaunchProgram,4x4 Evolution}"; Flags: postinstall nowait skipifsilent

[UninstallDelete]
Type: files; Name: "{app}\system\evo2026.ini"
Type: files; Name: "{app}\system\evo2026.log"
Type: filesandordirs; Name: "{app}\textures_dump"
Type: files; Name: "{app}\system\evo2026.log"

[Code]
function StartsWithText(const S, Prefix: String): Boolean;
begin
  Result := (Prefix <> '') and (CompareText(Copy(S, 1, Length(Prefix)), Prefix) = 0);
end;

function NextButtonClick(CurPageID: Integer): Boolean;
var
  Dir: String;
  InProgramFiles: Boolean;
begin
  Result := True;
  if CurPageID = wpSelectDir then
  begin
    Dir := AddBackslash(WizardDirValue);
    InProgramFiles := StartsWithText(Dir, AddBackslash(ExpandConstant('{commonpf32}')));
    // the 64-bit Program Files constant only exists on 64-bit Windows
    if (not InProgramFiles) and IsWin64 then
      InProgramFiles := StartsWithText(Dir, AddBackslash(ExpandConstant('{commonpf64}')));
    if InProgramFiles then
    begin
      Result := MsgBox('4x4 Evolution saves its settings and profiles inside its own folder. ' +
        'Under Program Files, Windows redirects those writes to a hidden VirtualStore copy, ' +
        'which makes settings confusing to change.' + #13#10#13#10 +
        'A folder like C:\Games\4x4 Evolution is recommended. Install here anyway?',
        mbConfirmation, MB_YESNO) = IDYES;
    end;
  end;
end;
