# Microsoft Developer Studio Project File - Name="test" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=test - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "test.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "test.mak" CFG="test - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "test - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "test - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "test - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "test - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ  /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ  /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "test - Win32 Release"
# Name "test - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\Version2\aa.c
# End Source File
# Begin Source File

SOURCE=..\Version2\altaz.c
# End Source File
# Begin Source File

SOURCE=..\Version2\angles.c
# End Source File
# Begin Source File

SOURCE=..\Version2\annuab.c
# End Source File
# Begin Source File

SOURCE=..\Version2\arcdot.c
# End Source File
# Begin Source File

SOURCE=..\Version2\constel.c
# End Source File
# Begin Source File

SOURCE=..\Version2\deflec.c
# End Source File
# Begin Source File

SOURCE=..\Version2\deltat.c
# End Source File
# Begin Source File

SOURCE=..\Version2\diurab.c
# End Source File
# Begin Source File

SOURCE=..\Version2\diurpx.c
# End Source File
# Begin Source File

SOURCE=..\Version2\dms.c
# End Source File
# Begin Source File

SOURCE=..\Version2\domoon.c
# End Source File
# Begin Source File

SOURCE=..\Version2\ear404.c
# End Source File
# Begin Source File

SOURCE=..\Version2\epsiln.c
# End Source File
# Begin Source File

SOURCE=..\Version2\fk4fk5.c
# End Source File
# Begin Source File

SOURCE=..\Version2\gplan.c
# End Source File
# Begin Source File

SOURCE=..\Version2\jup404.c
# End Source File
# Begin Source File

SOURCE=..\Version2\kepler.c
# End Source File
# Begin Source File

SOURCE=..\Version2\kfiles.c
# End Source File
# Begin Source File

SOURCE=..\Version2\lightt.c
# End Source File
# Begin Source File

SOURCE=..\Version2\lonlat.c
# End Source File
# Begin Source File

SOURCE=..\Version2\mar404.c
# End Source File
# Begin Source File

SOURCE=..\Version2\mer404.c
# End Source File
# Begin Source File

SOURCE=..\Version2\mlat404.c
# End Source File
# Begin Source File

SOURCE=..\Version2\mlr404.c
# End Source File
# Begin Source File

SOURCE=..\Version2\nep404.c
# End Source File
# Begin Source File

SOURCE=..\Version2\nutate.c
# End Source File
# Begin Source File

SOURCE=..\Version2\plu404.c
# End Source File
# Begin Source File

SOURCE=..\Version2\precess.c
# End Source File
# Begin Source File

SOURCE=..\Version2\refrac.c
# End Source File
# Begin Source File

SOURCE=..\Version2\rplanet.c
# End Source File
# Begin Source File

SOURCE=..\Version2\rstar.c
# End Source File
# Begin Source File

SOURCE=..\Version2\sat404.c
# End Source File
# Begin Source File

SOURCE=..\Version2\sidrlt.c
# End Source File
# Begin Source File

SOURCE=..\Version2\sun.c
# End Source File
# Begin Source File

SOURCE=..\Version2\tdb.c
# End Source File
# Begin Source File

SOURCE=..\Version2\trnsit.c
# End Source File
# Begin Source File

SOURCE=..\Version2\ura404.c
# End Source File
# Begin Source File

SOURCE=..\Version2\vearth.c
# End Source File
# Begin Source File

SOURCE=..\Version2\ven404.c
# End Source File
# Begin Source File

SOURCE=..\Version2\zatan2.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
