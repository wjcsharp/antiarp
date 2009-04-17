# Microsoft Developer Studio Project File - Name="WonArp" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=WonArp - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "sanarp.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sanarp.mak" CFG="WonArp - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "WonArp - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "WonArp - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "WonArp - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /Gz /W3 /O2 /I "$(DDKPATH)\inc" /I "$(DDKPATH)\inc\ddk" /I "$(DDKPATH)\inc\ddk\wdm" /FI"$(DDKPATH)\inc\warning.h" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D _X86_=1 /D i386=1 /D "STD_CALL" /D CONDITION_HANDLING=1 /D NT_UP=1 /D NT_INST=0 /D WIN32=100 /D _NT1X_=100 /D WINNT=1 /D _WIN32_WINNT=0x0500 /D _WIN32_IE=0x0400 /D WIN32_LEAN_AND_MEAN=1 /D DEVL=1 /D FPO=0 /D _DLL=1 /D "DRIVER" /D "_IDWBUILD" /D DBG=0 /FR /YX /FD /Zel -cbstring /QIfdiv- /QI0f /GF /Oxs /c
# ADD CPP /nologo /Gz /W3 /O2 /I "$(DDKPATH)\inc" /I "$(DDKPATH)\inc\ddk" /I "$(DDKPATH)\inc\ddk\wdm" /FI"$(DDKPATH)\inc\warning.h" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D _X86_=1 /D i386=1 /D "STD_CALL" /D CONDITION_HANDLING=1 /D NT_UP=1 /D NT_INST=0 /D WIN32=100 /D _NT1X_=100 /D WINNT=1 /D _WIN32_WINNT=0x0500 /D _WIN32_IE=0x0400 /D WIN32_LEAN_AND_MEAN=1 /D DEVL=1 /D FPO=0 /D _DLL=1 /D "DRIVER" /D "_IDWBUILD" /D DBG=0 /FR /YX /FD /Zel -cbstring /QIfdiv- /QI0f /GF /Oxs /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wdm.lib /nologo /base:"0x10000" /version:4.0 /entry:"DriverEntry@8" /subsystem:windows /pdb:none /machine:I386 /nodefaultlib /out:"release\WonArp.sys" /libpath:"$(DDKPATH)\libfre\i386" /libpath:"$(DDKPATH)\lib\i386" -MERGE:_PAGE=PAGE -MERGE:_TEXT=.text -MERGE:.rdata=.text -SECTION:INIT,d -OPT:REF -FORCE:MULTIPLE -RELEASE -FULLBUILD -IGNORE:4001,4037,4039,4049,4065,4070,4078,4087,4089,4096 -osversion:4.00 -optidata -driver -align:0x20 -subsystem:native,4.00 -debug:notmapped,minimal
# ADD LINK32 ntoskrnl.lib hal.lib ndis.lib /nologo /base:"0x10000" /version:4.0 /entry:"DriverEntry@8" /subsystem:windows /pdb:none /machine:I386 /nodefaultlib /out:"..\release\WonArp.sys" /libpath:"$(DDKPATH)\libfre\i386" /libpath:"$(DDKPATH)\lib\i386" -MERGE:_PAGE=PAGE -MERGE:_TEXT=.text -MERGE:.rdata=.text -SECTION:INIT,d -OPT:REF -FORCE:MULTIPLE -RELEASE -FULLBUILD -IGNORE:4001,4037,4039,4049,4065,4070,4078,4087,4089,4096 -osversion:4.00 -optidata -driver -align:0x20 -subsystem:native,4.00 -debug:notmapped,minimal
# Begin Custom Build - Finishing up...
IntDir=.\Release
TargetPath=\ARPFW2\release\WonArp.sys
TargetName=WonArp
InputPath=\ARPFW2\release\WonArp.sys
SOURCE="$(InputPath)"

"$(IntDir)\$(TargetName).nms" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(SIWPATH)\nmsym -translate:source,package,always $(TargetPath) 
	rebase -b 10000 -x junk $(TargetPath) 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "WonArp - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /Gz /W3 /Z7 /Oi /Gy /I "$(DDKPATH)\inc" /I "$(DDKPATH)\inc\ddk" /I "$(DDKPATH)\inc\ddk\wdm" /FI"$(DDKPATH)\inc\warning.h" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D _X86_=1 /D i386=1 /D "STD_CALL" /D CONDITION_HANDLING=1 /D NT_UP=1 /D NT_INST=0 /D WIN32=100 /D _NT1X_=100 /D WINNT=1 /D _WIN32_WINNT=0x0500 /D _WIN32_IE=0x0400 /D WIN32_LEAN_AND_MEAN=1 /D DEVL=1 /D FPO=0 /D "NDEBUG" /D _DLL=1 /D "DRIVER" /D "_IDWBUILD" /D "RDRDBG" /D "SRVDBG" /D DBG=1 /FR /YX /FD /Zel -cbstring /QIfdiv- /QI0f /GF /QIf /c
# ADD CPP /nologo /Gz /W3 /Z7 /Oi /Gy /I "$(DDKPATH)\inc" /I "$(DDKPATH)\inc\ddk" /I "$(DDKPATH)\inc\ddk\wdm" /FI"$(DDKPATH)\inc\warning.h" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D _X86_=1 /D i386=1 /D "STD_CALL" /D CONDITION_HANDLING=1 /D NT_UP=1 /D NT_INST=0 /D WIN32=100 /D _NT1X_=100 /D WINNT=1 /D _WIN32_WINNT=0x0500 /D _WIN32_IE=0x0400 /D WIN32_LEAN_AND_MEAN=1 /D DEVL=1 /D FPO=0 /D "NDEBUG" /D _DLL=1 /D "DRIVER" /D "_IDWBUILD" /D "RDRDBG" /D "SRVDBG" /D DBG=1 /FR /YX /FD /Zel -cbstring /QIfdiv- /QI0f /GF /QIf /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wdm.lib /nologo /base:"0x10000" /version:4.0 /entry:"DriverEntry@8" /subsystem:windows /pdb:none /debug /debugtype:both /machine:I386 /nodefaultlib /out:"debug\WonArp.sys" /libpath:"$(DDKPATH)\libchk\i386" /libpath:"$(DDKPATH)\lib\i386" -MERGE:_PAGE=PAGE -MERGE:_TEXT=.text -MERGE:.rdata=.text -SECTION:INIT,d -OPT:REF -FORCE:MULTIPLE -RELEASE -FULLBUILD -IGNORE:4001,4037,4039,4049,4065,4070,4078,4087,4089,4096 -osversion:4.00 -optidata -driver -align:0x20 -subsystem:native,4.00 -debug:notmapped,FULL
# ADD LINK32 ntoskrnl.lib hal.lib ndis.lib /nologo /base:"0x10000" /version:4.0 /entry:"DriverEntry@8" /subsystem:windows /pdb:none /debug /debugtype:both /machine:I386 /nodefaultlib /out:"..\debug\WonArp.sys" /libpath:"$(DDKPATH)\libchk\i386" /libpath:"$(DDKPATH)\lib\i386" -MERGE:_PAGE=PAGE -MERGE:_TEXT=.text -MERGE:.rdata=.text -SECTION:INIT,d -OPT:REF -FORCE:MULTIPLE -RELEASE -FULLBUILD -IGNORE:4001,4037,4039,4049,4065,4070,4078,4087,4089,4096 -osversion:4.00 -optidata -driver -align:0x20 -subsystem:native,4.00 -debug:notmapped,FULL
# Begin Custom Build - Finishing up...
IntDir=.\Debug
TargetPath=\ARPFW2\debug\WonArp.sys
TargetName=WonArp
InputPath=\ARPFW2\debug\WonArp.sys
SOURCE="$(InputPath)"

"$(IntDir)\$(TargetName).nms" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(SIWPATH)\nmsym -translate:source,package,always $(TargetPath) 
	copy $(TargetPath) $(WINDIR)\system32\drivers 
	
# End Custom Build

!ENDIF 

# Begin Target

# Name "WonArp - Win32 Release"
# Name "WonArp - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\GatewayCheck.c
# End Source File
# Begin Source File

SOURCE=.\IoCtrl.c
# End Source File
# Begin Source File

SOURCE=.\miniport4.c
# End Source File
# Begin Source File

SOURCE=.\miniport51.c
# End Source File
# Begin Source File

SOURCE=.\Packet.c
# End Source File
# Begin Source File

SOURCE=.\protocol4.c
# End Source File
# Begin Source File

SOURCE=.\protocol5.c
# End Source File
# Begin Source File

SOURCE=.\WonArp.c
# End Source File
# Begin Source File

SOURCE=.\WonArp.rc
# End Source File
# Begin Source File

SOURCE=.\SendRecv4.c
# End Source File
# Begin Source File

SOURCE=.\SendRecv5.c
# End Source File
# Begin Source File

SOURCE=.\UserShareEvent.c
# End Source File
# Begin Source File

SOURCE=.\UserShareMemory.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\GatewayCheck.h
# End Source File
# Begin Source File

SOURCE=.\Miniport4.h
# End Source File
# Begin Source File

SOURCE=.\Miniport51.h
# End Source File
# Begin Source File

SOURCE=.\Packet.h
# End Source File
# Begin Source File

SOURCE=.\precomp.h
# End Source File
# Begin Source File

SOURCE=.\Protocol4.h
# End Source File
# Begin Source File

SOURCE=.\Protocol5.h
# End Source File
# Begin Source File

SOURCE=.\WonArp.h
# End Source File
# Begin Source File

SOURCE=.\Share.h
# End Source File
# Begin Source File

SOURCE=.\structs.h
# End Source File
# Begin Source File

SOURCE=.\UserShareEvent.h
# End Source File
# Begin Source File

SOURCE=.\UserShareMemory.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
