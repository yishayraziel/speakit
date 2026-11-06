; Inno Setup script for Speakit (Windows, 64-bit)
#define AppName "Speakit"
#ifndef AppVersion
#define AppVersion "0.1.0"
#endif

[Setup]
AppId={{A7C41E90-2B63-4D18-9F05-6E8D2C713B44}
AppName={#AppName}
AppVersion={#AppVersion}
AppPublisher=YRAZIEL
AppPublisherURL=https://yraziel.com
DefaultDirName={autopf}\YRAZIEL\Speakit
DisableProgramGroupPage=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
OutputDir=Output
OutputBaseFilename=Speakit_win_setup
Compression=lzma2
SolidCompression=yes
WizardStyle=modern

[Components]
Name: "vst3"; Description: "VST3 plugin"; Types: full compact custom; Flags: fixed
Name: "standalone"; Description: "Standalone app"; Types: full

[Files]
Source: "..\build\SPEAKIT_artefacts\Release\VST3\Speakit.vst3\*"; DestDir: "{commoncf64}\VST3\Speakit.vst3"; Components: vst3; Flags: recursesubdirs ignoreversion
Source: "..\build\SPEAKIT_artefacts\Release\Standalone\Speakit.exe"; DestDir: "{app}"; Components: standalone; Flags: ignoreversion
Source: "..\docs\Speakit_Manual.pdf"; DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist

[Icons]
Name: "{autoprograms}\Speakit"; Filename: "{app}\Speakit.exe"; Components: standalone
Name: "{autoprograms}\Speakit Manual"; Filename: "{app}\Speakit_Manual.pdf"
