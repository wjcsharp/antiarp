; 脚本由 Inno Setup 脚本向导 生成！
; 有关创建 Inno Setup 脚本文件的详细资料请查阅帮助文档！

[Setup]
AppName=玩火 AntiArp
AppVerName=玩火 AntiArp For 2000/XP/2003/Vista/2008
AppPublisher=Debugman.com
AppPublisherURL=http://www.Debugman.com
AppSupportURL=http://www.Debugman.com
AppUpdatesURL=http://www.Debugman.com
DefaultDirName={pf}\玩火 AntiArp
DefaultGroupName=玩火 AntiArp
OutputBaseFilename=WonArp
SetupIconFile=..\WonFW\res\WonArpFW.ico
Compression=lzma
SolidCompression=yes
PrivilegesRequired = admin

MinVersion=5.0,5.0
OnlyBelowVersion=5.0,6.0

ArchitecturesAllowed = x86
VersionInfoCompany=Debugman.com
VersionInfoVersion=1.0.0.1
VersionInfoTextVersion=1.0.0.1
VersionInfoCopyright=Debugman.com
VersionInfoDescription=玩火 AntiArp For 2000/XP/2003/Vista/2008
UninstallRestartComputer = yes

[Languages]
Name: "chinese"; MessagesFile: "compiler:Default.isl"
Name: "english"; MessagesFile: "compiler:Languages\English.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
; 防火墙主程序
Source: "..\Release\WonFW.exe";      DestDir: "{app}"; Flags: ignoreversion  uninsrestartdelete onlyifdoesntexist
Source: "..\Release\WonArp.dll";     DestDir: "{app}"; Flags: ignoreversion  uninsrestartdelete onlyifdoesntexist

; 2k/XP/2003 Driver
Source: "..\Release\PTWONARP.inf";     DestDir: "{app}"; Flags: ignoreversion  deleteafterinstall onlyifdoesntexist
Source: "..\Release\MPWONARP.inf";     DestDir: "{app}"; Flags: ignoreversion  deleteafterinstall onlyifdoesntexist
Source: "..\Release\WonArp.sys";       DestDir: "{app}"; Flags: ignoreversion  deleteafterinstall onlyifdoesntexist

; Vista Driver
;Source: "..\Release\WonArp6.inf";     DestDir: "{app}"; Flags: ignoreversion  deleteafterinstall onlyifdoesntexist
;Source: "..\Release\WonArp6.sys";     DestDir: "{app}"; Flags: ignoreversion  deleteafterinstall onlyifdoesntexist


[Icons]
Name: "{group}\玩火 AntiArp"; Filename: "{app}\WonFW.exe.exe"
Name: "{commondesktop}\玩火 AntiArp"; Filename: "{app}\WonFW.exe"; Tasks: desktopicon

[Registry]
; 系统自启动
Root: HKLM; Subkey: "SOFTWARE\Microsoft\Windows\CurrentVersion\Run"; ValueType: string; ValueName: "WonArpFW"; ValueData: "{app}\WonFW.exe /Auto" ; Flags : uninsdeletevalue
; 卸载删除注册表条目
Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Services\WonArp"; Flags: uninsdeletekey dontcreatekey noerror

[Run]

; 禁用数字签名
Filename: "{app}\WonFW.exe"; Parameters: "/DisableSign" ; Flags : runhidden
; 安装驱动
Filename: "{app}\WonFW.exe"; Parameters: "/Install" ; Flags : runhidden ; StatusMsg: "正在安装 玩火 AntiArp，请稍候..."
; 启用数字签名
Filename: "{app}\WonFW.exe"; Parameters: "/EnableSign" ; Flags : runhidden

Filename: "{app}\WonFW.exe"; Description: "{cm:LaunchProgram,玩火 AntiArp}"; Flags: nowait postinstall skipifsilent

[UninstallRun]
; 卸载驱动
Filename: "{app}\WonFW.exe"; Parameters: "/Remove" ; Flags : runhidden  ; StatusMsg: "正在卸载玩火 AntiArp,请手动关闭正在运行的防火墙进程..."




