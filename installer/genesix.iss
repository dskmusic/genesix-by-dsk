; Genesix by DSK - Inno Setup installer. Build: iscc /DAppVersion=1.0.0 /DArtDir=<artefacts\Release> installer\genesix.iss
#ifndef AppVersion
  #define AppVersion "0.0.0"
#endif
#ifndef ArtDir
  #define ArtDir "..\build\Genesix_artefacts\Release"
#endif

[Setup]
AppId={{6E3B0C51-8F7D-4C2A-9A0E-6B1D5D2C9E47}
AppName=Genesix by DSK
AppVersion={#AppVersion}
AppPublisher=DSK Music
AppPublisherURL=https://dskmusic.com
DefaultDirName={autopf}\DSK Music\Genesix
DefaultGroupName=DSK Music
OutputBaseFilename=Genesix-by-DSK-win-x64-setup
OutputDir=..\dist
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
Compression=lzma2
SolidCompression=yes
LicenseFile=..\LICENSE
DisableProgramGroupPage=yes

[Components]
Name: "vst3"; Description: "VST3 plugin"; Types: full compact custom; Flags: fixed
Name: "standalone"; Description: "Standalone application"; Types: full

[Files]
Source: "{#ArtDir}\VST3\Genesix by DSK.vst3\*"; DestDir: "{commoncf64}\VST3\Genesix by DSK.vst3"; Components: vst3; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#ArtDir}\Standalone\Genesix by DSK.exe"; DestDir: "{app}"; Components: standalone; Flags: ignoreversion

[Icons]
Name: "{group}\Genesix by DSK"; Filename: "{app}\Genesix by DSK.exe"; Components: standalone
