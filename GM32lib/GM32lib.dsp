# Microsoft Developer Studio Project File - Name="GM32lib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=GM32lib - Win32 WaypointDebug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "GM32lib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "GM32lib.mak" CFG="GM32lib - Win32 WaypointDebug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "GM32lib - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "GM32lib - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "GM32lib - Win32 WaypointDebug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "GM32lib - Win32 WaypointProduction" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "GM32lib - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GM32LIB_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GM32LIB_EXPORTS" /D "XPORABOVE" /D "USEFREEIMAGE" /D "USEMRSID" /D "__STDC__" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wow32.lib gdi32.lib setupapi.lib mpr.lib urlmon.lib /nologo /dll /map /debug /machine:I386

!ELSEIF  "$(CFG)" == "GM32lib - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GM32LIB_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GM32LIB_EXPORTS" /D "XPORABOVE" /D "USEFREEIMAGE" /D "USEMRSID" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wow32.lib gdi32.lib setupapi.lib mpr.lib urlmon.lib /nologo /dll /map /debug /machine:I386 /pdbtype:sept /libpath:"c:\gssi\prog\email\blat\blat240\full"

!ELSEIF  "$(CFG)" == "GM32lib - Win32 WaypointDebug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "GM32lib___Win32_WaypointDebug"
# PROP BASE Intermediate_Dir "GM32lib___Win32_WaypointDebug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "GM32lib___Win32_WaypointDebug"
# PROP Intermediate_Dir "GM32lib___Win32_WaypointDebug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GM32LIB_EXPORTS" /D "XPORABOVE" /FR /FD /GZ /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GM32LIB_EXPORTS" /D "XPORABOVE" /D "USEFREEIMAGE" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wow32.lib gdi32.lib setupapi.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wow32.lib gdi32.lib setupapi.lib mpr.lib urlmon.lib /nologo /dll /debug /machine:I386 /pdbtype:sept

!ELSEIF  "$(CFG)" == "GM32lib - Win32 WaypointProduction"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "GM32lib___Win32_WaypointProduction"
# PROP BASE Intermediate_Dir "GM32lib___Win32_WaypointProduction"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "GM32lib___Win32_WaypointProduction"
# PROP Intermediate_Dir "GM32lib___Win32_WaypointProduction"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GM32LIB_EXPORTS" /FR /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GM32LIB_EXPORTS" /D "XPORABOVE" /D "USEFREEIMAGE" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wow32.lib gdi32.lib setupapi.lib /nologo /dll /map /machine:I386
# ADD LINK32 kernel32.lib user32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wow32.lib gdi32.lib setupapi.lib mpr.lib urlmon.lib /nologo /dll /map /machine:I386

!ENDIF 

# Begin Target

# Name "GM32lib - Win32 Release"
# Name "GM32lib - Win32 Debug"
# Name "GM32lib - Win32 WaypointDebug"
# Name "GM32lib - Win32 WaypointProduction"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE="..\Proj-4\proj-4.4.8\src\adjlon.c"
# End Source File
# Begin Source File

SOURCE=.\config.c
# End Source File
# Begin Source File

SOURCE=.\Demo.Rc
# End Source File
# Begin Source File

SOURCE=..\sunang\ipw\src\lib\libunix\src\dysize.c
# End Source File
# Begin Source File

SOURCE=..\sunang\ipw\src\lib\libipw\util\ecalloc.c
# End Source File
# Begin Source File

SOURCE=..\sunang\ipw\src\lib\libipw\sunang\ephemeris.c
# End Source File
# Begin Source File

SOURCE=.\filestuff.c
# End Source File
# Begin Source File

SOURCE=.\freeim.c
# End Source File
# Begin Source File

SOURCE=.\GM32lib.c
# End Source File
# Begin Source File

SOURCE=.\GM32lib.def
# End Source File
# Begin Source File

SOURCE=..\sunang\ipw\src\lib\libipw\sunang\julday.c
# End Source File
# Begin Source File

SOURCE=..\MRGSCoordinates\Mgrs\geotrans2.2.6\dt_cc\mgrs\mgrs.c
# End Source File
# Begin Source File

SOURCE=..\minilzo\minilzo.c
# End Source File
# Begin Source File

SOURCE=.\mrsid.c
# End Source File
# Begin Source File

SOURCE="..\Proj-4\proj-4.4.8\src\nad_cvt.c"
# End Source File
# Begin Source File

SOURCE="..\Proj-4\proj-4.4.8\src\nad_init.c"
# End Source File
# Begin Source File

SOURCE="..\Proj-4\proj-4.4.8\src\nad_intr.c"
# End Source File
# Begin Source File

SOURCE=.\nadcon.c
# End Source File
# Begin Source File

SOURCE="..\Proj-4\proj-4.4.8\src\pj_errno.c"
# End Source File
# Begin Source File

SOURCE="..\Proj-4\proj-4.4.8\src\pj_malloc.c"
# End Source File
# Begin Source File

SOURCE="..\Proj-4\proj-4.4.8\src\pj_open_lib.c"
# End Source File
# Begin Source File

SOURCE=..\MRGSCoordinates\Mgrs\geotrans2.2.6\dt_cc\polarst\polarst.c
# End Source File
# Begin Source File

SOURCE=..\sunang\ipw\src\lib\libipw\sunang\rotate.c
# End Source File
# Begin Source File

SOURCE=..\sunang\ipw\src\lib\libipw\sunang\sunpath.c
# End Source File
# Begin Source File

SOURCE=..\sunang\ipw\src\exp\ucsb\bin\sunlight\sunrise.c
# End Source File
# Begin Source File

SOURCE=..\sunang\ipw\src\lib\libipw\error\syserr.c
# End Source File
# Begin Source File

SOURCE=..\sunang\ipw\src\lib\libipw\sunang\tmconv.c
# End Source File
# Begin Source File

SOURCE=..\MRGSCoordinates\Mgrs\geotrans2.2.6\dt_cc\tranmerc\tranmerc.c
# End Source File
# Begin Source File

SOURCE=..\sunang\ipw\src\lib\libipw\sunang\unixtime.c
# End Source File
# Begin Source File

SOURCE=..\MRGSCoordinates\Mgrs\geotrans2.2.6\dt_cc\ups\ups.c
# End Source File
# Begin Source File

SOURCE=..\garmin\USB\USBSDK.c
# End Source File
# Begin Source File

SOURCE=..\sunang\ipw\src\lib\libipw\error\usrerr.c
# End Source File
# Begin Source File

SOURCE=..\MRGSCoordinates\Mgrs\geotrans2.2.6\dt_cc\utm\utm.c
# End Source File
# Begin Source File

SOURCE=..\sunang\ipw\src\lib\libipw\sunang\weekday.c
# End Source File
# Begin Source File

SOURCE=..\sunang\ipw\src\lib\libipw\sunang\yrday.c
# End Source File
# Begin Source File

SOURCE=..\sunang\ipw\src\exp\ucsb\bin\sunlight\zerobr.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\gm32lib.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\FileOpen.Dlg
# End Source File
# End Group
# Begin Source File

SOURCE=..\MrSid\Geo_DSDK\lib\Release_md\lti_dsdk_cdll.lib
# End Source File
# Begin Source File

SOURCE="..\..\..\Program Files\Microsoft Visual Studio\VC98\Lib\VFW32.LIB"
# End Source File
# Begin Source File

SOURCE=.\FreeImage.lib
# End Source File
# End Target
# End Project
