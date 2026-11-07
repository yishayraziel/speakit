; Inno Setup script for Anal-Log (Windows, 64-bit)
#define AppName "Anal-Log"
#ifndef AppVersion
#define AppVersion "0.1.0"
#endif

[Setup]
AppId={{3E9B7C21-5A4D-4F80-B6E2-91C0D47A8F15}
AppName={#AppName}
AppVersion={#AppVersion}
AppPublisher=Yishay Raziel
AppPublisherURL=https://yraziel.com
DefaultDirName={autopf}\Yishay Raziel\Anal-Log
DisableProgramGroupPage=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
OutputDir=Output
OutputBaseFilename=Anal-Log_win_setup
Compression=lzma2
SolidCompression=yes
WizardStyle=modern

[Components]
Name: "vst3"; Description: "VST3 plugin"; Types: full compact custom; Flags: fixed
Name: "standalone"; Description: "Standalone app"; Types: full

[Files]
Source: "..\build\SPEAKIT_artefacts\Release\VST3\Anal-Log.vst3\*"; DestDir: "{commoncf64}\VST3\Anal-Log.vst3"; Components: vst3; Flags: recursesubdirs ignoreversion
Source: "..\build\SPEAKIT_artefacts\Release\Standalone\Anal-Log.exe"; DestDir: "{app}"; Components: standalone; Flags: ignoreversion
Source: "..\docs\Anal-Log_Manual.pdf"; DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist

[Icons]
Name: "{autoprograms}\Anal-Log"; Filename: "{app}\Anal-Log.exe"; Components: standalone
Name: "{autoprograms}\Anal-Log Manual"; Filename: "{app}\Anal-Log_Manual.pdf"
