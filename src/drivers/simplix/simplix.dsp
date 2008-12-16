# Microsoft Developer Studio Project File - Name="simplix" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=simplix - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "simplix.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "simplix.mak" CFG="simplix - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "simplix - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "simplix - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "simplix - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "simplix_EXPORTS" /YX /FD /c
# ADD CPP /nologo /G6 /W2 /GX /O2 /Ob2 /I "../../../export/include" /I "../../windows/include" /I "../../.." /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "simplix_EXPORTS" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 tgf.lib robottools.lib sg.lib ul.lib /nologo /dll /map /machine:I386 /nodefaultlib:"LIBCD" /libpath:"../../../export/lib" /libpath:"../../windows/lib"
# Begin Special Build Tool
WkspDir=.
TargetDir=.\Release
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(TargetDir)\*.dll C:\Programme\Torcs\drivers\simplix
# End Special Build Tool

!ELSEIF  "$(CFG)" == "simplix - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "simplix_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /G6 /W2 /GX /ZI /Od /I "../../../export/include" /I "../../windows/include" /I "../../.." /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "simplix_EXPORTS" /D "DEBUG" /D "DEBUG_OUT" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 tgf.lib robottools.lib sg.lib ul.lib /nologo /dll /map /debug /machine:I386 /nodefaultlib:"LIBC" /pdbtype:sept /libpath:"../../../export/libd" /libpath:"../../windows/lib"
# Begin Special Build Tool
WkspDir=.
TargetDir=.\Debug
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(TargetDir)\*.dll C:\Programme\Torcs\drivers\simplix
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "simplix - Win32 Release"
# Name "simplix - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\src\unitmain.cpp
# End Source File
# Begin Source File

SOURCE=.\src\simplix.def
# End Source File
# Begin Source File

SOURCE=.\src\unitcarparam.cpp
# End Source File
# Begin Source File

SOURCE=.\src\unitcharacteristic.cpp
# End Source File
# Begin Source File

SOURCE=.\src\unitclothoid.cpp
# End Source File
# Begin Source File

SOURCE=.\src\unitcollision.cpp
# End Source File
# Begin Source File

SOURCE=.\src\unitcommon.cpp
# End Source File
# Begin Source File

SOURCE=.\src\unitcommondata.cpp
# End Source File
# Begin Source File

SOURCE=.\src\unitcubic.cpp
# End Source File
# Begin Source File

SOURCE=.\src\unitcubicspline.cpp
# End Source File
# Begin Source File

SOURCE=.\src\unitdriver.cpp
# End Source File
# Begin Source File

SOURCE=.\src\unitdynarray.cpp
# End Source File
# Begin Source File

SOURCE=.\src\unitfixcarparam.cpp
# End Source File
# Begin Source File

SOURCE=.\src\unitlane.cpp
# End Source File
# Begin Source File

SOURCE=.\src\unitlanepoint.cpp
# End Source File
# Begin Source File

SOURCE=.\src\unitlinalg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\unitlinreg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\unitopponent.cpp
# End Source File
# Begin Source File

SOURCE=.\src\unitparabel.cpp
# End Source File
# Begin Source File

SOURCE=.\src\unitparam.cpp
# End Source File
# Begin Source File

SOURCE=.\src\unitpidctrl.cpp
# End Source File
# Begin Source File

SOURCE=.\src\unitpit.cpp
# End Source File
# Begin Source File

SOURCE=.\src\unitpitparam.cpp
# End Source File
# Begin Source File

SOURCE=.\src\unitsection.cpp
# End Source File
# Begin Source File

SOURCE=.\src\unitstrategy.cpp
# End Source File
# Begin Source File

SOURCE=.\src\unitsysfoo.cpp
# End Source File
# Begin Source File

SOURCE=.\src\unitteammanager.cpp
# End Source File
# Begin Source File

SOURCE=.\src\unittmpcarparam.cpp
# End Source File
# Begin Source File

SOURCE=.\src\unittrack.cpp
# End Source File
# Begin Source File

SOURCE=.\src\unitvec2d.cpp
# End Source File
# Begin Source File

SOURCE=.\src\unitvec3d.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\src\unitcarparam.h
# End Source File
# Begin Source File

SOURCE=.\src\unitcharacteristic.h
# End Source File
# Begin Source File

SOURCE=.\src\unitclothoid.h
# End Source File
# Begin Source File

SOURCE=.\src\unitcollision.h
# End Source File
# Begin Source File

SOURCE=.\src\unitcommon.h
# End Source File
# Begin Source File

SOURCE=.\src\unitcommondata.h
# End Source File
# Begin Source File

SOURCE=.\src\unitcubic.h
# End Source File
# Begin Source File

SOURCE=.\src\unitcubicspline.h
# End Source File
# Begin Source File

SOURCE=.\src\unitdriver.h
# End Source File
# Begin Source File

SOURCE=.\src\unitdynarray.h
# End Source File
# Begin Source File

SOURCE=.\src\unitfixcarparam.h
# End Source File
# Begin Source File

SOURCE=.\src\unitlane.h
# End Source File
# Begin Source File

SOURCE=.\src\unitlanepoint.h
# End Source File
# Begin Source File

SOURCE=.\src\unitlinalg.h
# End Source File
# Begin Source File

SOURCE=.\src\unitlinreg.h
# End Source File
# Begin Source File

SOURCE=.\src\unitopponent.h
# End Source File
# Begin Source File

SOURCE=.\src\unitparabel.h
# End Source File
# Begin Source File

SOURCE=.\src\unitparam.h
# End Source File
# Begin Source File

SOURCE=.\src\unitpidctrl.h
# End Source File
# Begin Source File

SOURCE=.\src\unitpit.h
# End Source File
# Begin Source File

SOURCE=.\src\unitpitparam.h
# End Source File
# Begin Source File

SOURCE=.\src\unitsection.h
# End Source File
# Begin Source File

SOURCE=.\src\unitstrategy.h
# End Source File
# Begin Source File

SOURCE=.\src\unitsysfoo.h
# End Source File
# Begin Source File

SOURCE=.\src\unitteammanager.h
# End Source File
# Begin Source File

SOURCE=.\src\unittmpcarparam.h
# End Source File
# Begin Source File

SOURCE=.\src\unittrack.h
# End Source File
# Begin Source File

SOURCE=.\src\unitvec2d.h
# End Source File
# Begin Source File

SOURCE=.\src\unitvec3d.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\Makefile
# End Source File
# End Target
# End Project
