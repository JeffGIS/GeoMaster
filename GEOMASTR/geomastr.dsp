# Microsoft Developer Studio Project File - Name="geomastr" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=geomastr - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "geomastr.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "geomastr.mak" CFG="geomastr - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "geomastr - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "geomastr - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "geomastr - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /Zp2 /MT /vd0 /GR /GX /Zi /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "NO_STRICT" /D "XPORABOVE" /D _WIN32_WINNT=0x0502 /D "USEMRSID" /D "CHECKMEMx" /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /FR /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# SUBTRACT BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib Ws2_32.lib cryp2005.lib vfw32.lib winmm.lib urlmon.lib setupapi.lib mpr.lib crypto32.lib comctl32.lib lti_dsdk_cdll.lib psapi.lib Msimg32.lib /nologo /version:32.22 /stack:0x400000 /subsystem:windows /map /debug /machine:I386 /out:"Release/geomaster32y.exe" /libpath:"c:\gssi\prog\lib" /mapinfo:LINES
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /Zp2 /MTd /w /W0 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "CHECKMEM" /D "NO_STRICT" /D "XPORABOVE" /D "USEMRSID" /D _WIN32_WINNT=0x0502 /D "ENABLETRACEx" /D BITS_PER_COLOR=8 /FAs /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# SUBTRACT BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib Ws2_32.lib vfw32.lib winmm.lib urlmon.lib setupapi.lib mpr.lib comctl32.lib lti_dsdk_cdll.lib psapi.lib Msimg32.lib /nologo /stack:0x400000 /subsystem:windows /pdb:none /map /debug /machine:I386 /out:"Debug/geomaster.exe" /libpath:"c:\gssi\prog\lib"

!ENDIF 

# Begin Target

# Name "geomastr - Win32 Release"
# Name "geomastr - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\solar_calculator\Version2\aa.c
# End Source File
# Begin Source File

SOURCE=.\address1.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

# ADD CPP /Zp2
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\address2.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

# ADD CPP /Zp2
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\address3.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

# ADD CPP /Zp2
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\AddressMsgProc.c
# End Source File
# Begin Source File

SOURCE="..\Proj-4\proj-4.4.8\src\adjlon.c"
# End Source File
# Begin Source File

SOURCE=..\gctpc\alberfor.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\alberinv.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\alconfor.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\alconinv.c
# End Source File
# Begin Source File

SOURCE=..\solar_calculator\Version2\altaz.c
# End Source File
# Begin Source File

SOURCE=..\solar_calculator\Version2\angles.c
# End Source File
# Begin Source File

SOURCE=..\solar_calculator\Version2\annuab.c
# End Source File
# Begin Source File

SOURCE=..\solar_calculator\Version2\arcdot.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\azimfor.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\aziminv.c
# End Source File
# Begin Source File

SOURCE=..\email\jwsmtp\base64.cpp
# End Source File
# Begin Source File

SOURCE="..\libbci-1.1.0\bci.c"
# End Source File
# Begin Source File

SOURCE=.\bigmem.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

# ADD CPP /Zp2
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\gctpc\br_gctp.c
# End Source File
# Begin Source File

SOURCE=..\DB\Bt_creat.c
# End Source File
# Begin Source File

SOURCE=..\humbird1\COACT\ByteSwap.c
# End Source File
# Begin Source File

SOURCE=.\cdstuff.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

# ADD CPP /Zp2
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\SHR\CLIDATA.C
# End Source File
# Begin Source File

SOURCE=.\cmdtonum.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

# ADD CPP /Zp2
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\SHR\COMDLG.c
# End Source File
# Begin Source File

SOURCE=..\email\jwsmtp\compat.cpp
# End Source File
# Begin Source File

SOURCE=..\SHR\Comput1.c
# End Source File
# Begin Source File

SOURCE=..\SHR\CONNAD83.C
# End Source File
# Begin Source File

SOURCE=..\solar_calculator\Version2\constel.c
# End Source File
# Begin Source File

SOURCE=.\convert.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

# ADD CPP /Zp2
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CONVRT32.C

!IF  "$(CFG)" == "geomastr - Win32 Release"

!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

# ADD CPP /Zp2
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\SHR\copydib.c
# End Source File
# Begin Source File

SOURCE=..\dgnlib\cpl_conv.cpp
# End Source File
# Begin Source File

SOURCE=..\dgnlib\cpl_dir.cpp
# End Source File
# Begin Source File

SOURCE=..\dgnlib\cpl_error.cpp
# End Source File
# Begin Source File

SOURCE=..\dgnlib\cpl_multiproc.cpp
# End Source File
# Begin Source File

SOURCE=..\dgnlib\cpl_path.cpp
# End Source File
# Begin Source File

SOURCE=..\dgnlib\cpl_string.cpp
# End Source File
# Begin Source File

SOURCE=..\dgnlib\cpl_vsil_simple.cpp
# End Source File
# Begin Source File

SOURCE=..\dgnlib\cpl_vsisimple.cpp
# End Source File
# Begin Source File

SOURCE=..\gctpc\cproj.c
# End Source File
# Begin Source File

SOURCE=.\Creatcfg.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

# ADD CPP /Zp2
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\SHR\CVTCOORD.C
# End Source File
# Begin Source File

SOURCE=.\cwconfig.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

# ADD CPP /Zp2
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\shapelib\code\dbfopen.c
# End Source File
# Begin Source File

SOURCE=..\SHR\DDE.C
# End Source File
# Begin Source File

SOURCE=..\solar_calculator\Version2\deflec.c
# End Source File
# Begin Source File

SOURCE=..\solar_calculator\Version2\deltat.c
# End Source File
# Begin Source File

SOURCE=..\..\prog16\GM32lib\Demo.Rc
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\dgn7functions.c
# End Source File
# Begin Source File

SOURCE=.\DGNFILE.C
# End Source File
# Begin Source File

SOURCE=..\dgnlib\dgnfloat.cpp
# End Source File
# Begin Source File

SOURCE=..\dgnlib\dgnhelp.cpp
# End Source File
# Begin Source File

SOURCE=..\dgnlib\dgnopen.cpp
# End Source File
# Begin Source File

SOURCE=..\dgnlib\dgnread.cpp
# End Source File
# Begin Source File

SOURCE=..\dgnlib\dgnstroke.cpp
# End Source File
# Begin Source File

SOURCE=..\dgnlib\dgnwrite.cpp
# End Source File
# Begin Source File

SOURCE=..\SHR\DIBUTIL.C
# End Source File
# Begin Source File

SOURCE=.\DICT.C

!IF  "$(CFG)" == "geomastr - Win32 Release"

!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

# ADD CPP /Zp2
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\DICTEDIT.C

!IF  "$(CFG)" == "geomastr - Win32 Release"

!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

# ADD CPP /Zp2
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\digitize.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

# ADD CPP /Zp2
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\solar_calculator\Version2\diurab.c
# End Source File
# Begin Source File

SOURCE=..\solar_calculator\Version2\diurpx.c
# End Source File
# Begin Source File

SOURCE=..\SHR\DLGLOOK.C
# End Source File
# Begin Source File

SOURCE=..\solar_calculator\Version2\dms.c
# End Source File
# Begin Source File

SOURCE=.\Document.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

# ADD CPP /Zp2
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\solar_calculator\Version2\domoon.c
# End Source File
# Begin Source File

SOURCE=.\dtm.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

# ADD CPP /Zp2
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\DTM2.C
# End Source File
# Begin Source File

SOURCE=.\Dxfin.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

# ADD CPP /Zp2
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dynaedit.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

# ADD CPP /Zp2
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\sunang\ipw\src\lib\libunix\src\dysize.c
# End Source File
# Begin Source File

SOURCE=..\solar_calculator\Version2\ear404.c
# End Source File
# Begin Source File

SOURCE=.\edgmatch.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

# ADD CPP /Zp2
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\sunang\ipw\src\lib\libipw\sunang\ephemeris.c
# End Source File
# Begin Source File

SOURCE=..\solar_calculator\Version2\epsiln.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\eqconfor.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\eqconinv.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\equifor.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\equiinv.c
# End Source File
# Begin Source File

SOURCE=.\exception.c
# End Source File
# Begin Source File

SOURCE=.\export.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

# ADD CPP /Zp2
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\EXPORT2.C
# End Source File
# Begin Source File

SOURCE=..\FileGDBAPI\FGDBStub.c
# End Source File
# Begin Source File

SOURCE=..\GM32lib\FIIO_Mem.cpp
# End Source File
# Begin Source File

SOURCE=..\SHR\FILE.C
# End Source File
# Begin Source File

SOURCE=.\fish.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

# ADD CPP /Zp2
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\solar_calculator\Version2\fk4fk5.c
# End Source File
# Begin Source File

SOURCE=..\SHR\floatap.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\for_init.c
# End Source File
# Begin Source File

SOURCE=..\GM32lib\freeim.c
# End Source File
# Begin Source File

SOURCE=.\FUNCSUBS.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

# ADD CPP /Zp2
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\function.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

# ADD CPP /Zp2
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\FUNCTN1.C

!IF  "$(CFG)" == "geomastr - Win32 Release"

!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

# ADD CPP /Zp2
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\functn2.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

# ADD CPP /Zp2
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\FUNCTN3.C

!IF  "$(CFG)" == "geomastr - Win32 Release"

!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

# ADD CPP /Zp2
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\FUNDEFS.C

!IF  "$(CFG)" == "geomastr - Win32 Release"

!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

# ADD CPP /Zp2
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\gctpc\gctp.c
# End Source File
# Begin Source File

SOURCE=.\GetSDChipID.c
# End Source File
# Begin Source File

SOURCE=..\GM32lib\GM32.c
# End Source File
# Begin Source File

SOURCE=.\GM32LIB.C
# End Source File
# Begin Source File

SOURCE=..\gctpc\gnomfor.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\gnominv.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\goodfor.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\goodinv.c
# End Source File
# Begin Source File

SOURCE=..\solar_calculator\Version2\gplan.c
# End Source File
# Begin Source File

SOURCE=..\gps\gps.c
# End Source File
# Begin Source File

SOURCE=.\graphic1.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

# ADD CPP /Zp2
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\graphic2.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

# ADD CPP /Zp2
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Graphic3.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

# ADD CPP /Zp2
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\graphic4.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

# ADD CPP /Zp2
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\GRAPHIC5.C

!IF  "$(CFG)" == "geomastr - Win32 Release"

!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

# ADD CPP /Zp2
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Graphic6.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

# ADD CPP /Zp2
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\graphic7.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

# ADD CPP /Zp2
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\graphic8.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

# ADD CPP /Zp2
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\GRAPHIC9.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

# ADD CPP /Zp2
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\graphic_functions.c
# End Source File
# Begin Source File

SOURCE=.\graphica.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

# ADD CPP /Zp2
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\graphicb.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

# ADD CPP /Zp2
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\graphicc.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

# ADD CPP /Zp2
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\GRAPHICD.C

!IF  "$(CFG)" == "geomastr - Win32 Release"

!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

# ADD CPP /Zp2
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\GraphMsgProc.c
# End Source File
# Begin Source File

SOURCE=.\GraphMsgProc2.c
# End Source File
# Begin Source File

SOURCE=.\grfunc.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

# ADD CPP /Zp2
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\splines\gspline.c
# End Source File
# Begin Source File

SOURCE=.\gssigdi.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\gvnspfor.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\gvnspinv.c
# End Source File
# Begin Source File

SOURCE=.\GWCONFIG.DEF
# End Source File
# Begin Source File

SOURCE=.\GWCONFIG.RC
# End Source File
# Begin Source File

SOURCE=..\DB\gwd.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\hamfor.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\haminv.c
# End Source File
# Begin Source File

SOURCE=..\SHR\HASH.C
# End Source File
# Begin Source File

SOURCE=.\hbird.c
# End Source File
# Begin Source File

SOURCE=.\hotspots.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

# ADD CPP /Zp2
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\gctpc\imolwfor.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\imolwinv.c
# End Source File
# Begin Source File

SOURCE=.\Infobox.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

# ADD CPP /Zp2
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\gctpc\inv_init.c
# End Source File
# Begin Source File

SOURCE=..\sunang\ipw\src\lib\libipw\sunang\julday.c
# End Source File
# Begin Source File

SOURCE=..\solar_calculator\Version2\jup404.c
# End Source File
# Begin Source File

SOURCE=..\solar_calculator\Version2\kepler.c
# End Source File
# Begin Source File

SOURCE=..\solar_calculator\Version2\kfiles.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\lamazfor.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\lamazinv.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\lamccfor.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\lamccinv.c
# End Source File
# Begin Source File

SOURCE=.\license.c
# End Source File
# Begin Source File

SOURCE=..\solar_calculator\Version2\lightt.c
# End Source File
# Begin Source File

SOURCE=..\humbird1\LkmToHbirdImage.c
# End Source File
# Begin Source File

SOURCE=..\solar_calculator\Version2\lonlat.c
# End Source File
# Begin Source File

SOURCE=..\email\jwsmtp\mailer.cpp
# End Source File
# Begin Source File

SOURCE=.\makearea.c
# End Source File
# Begin Source File

SOURCE=.\mapcopy.c
# End Source File
# Begin Source File

SOURCE=..\solar_calculator\Version2\mar404.c
# End Source File
# Begin Source File

SOURCE=..\SHR\memalloc.c
# End Source File
# Begin Source File

SOURCE=..\solar_calculator\Version2\mer404.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\merfor.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\merinv.c
# End Source File
# Begin Source File

SOURCE=..\MRGSCoordinates\Mgrs\geotrans2.2.6\dt_cc\mgrs\mgrs.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\millfor.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\millinv.c
# End Source File
# Begin Source File

SOURCE=..\minilzo\minilzo.c
# End Source File
# Begin Source File

SOURCE=..\solar_calculator\Version2\mlat404.c
# End Source File
# Begin Source File

SOURCE=..\solar_calculator\Version2\mlr404.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\molwfor.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\molwinv.c
# End Source File
# Begin Source File

SOURCE=..\GM32lib\mrsid.c
# End Source File
# Begin Source File

SOURCE=..\SHR\MS_MAPL.C
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

SOURCE=..\GM32lib\nadcon.c
# End Source File
# Begin Source File

SOURCE=..\solar_calculator\Version2\nep404.c
# End Source File
# Begin Source File

SOURCE=.\NETWORKS.c
# End Source File
# Begin Source File

SOURCE=..\solar_calculator\Version2\nutate.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\obleqfor.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\obleqinv.c
# End Source File
# Begin Source File

SOURCE=..\SHR\Odbcmain.c
# End Source File
# Begin Source File

SOURCE=..\OFFSET\Offset.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\omerfor.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\omerinv.c
# End Source File
# Begin Source File

SOURCE=.\ORAFILE.C
# End Source File
# Begin Source File

SOURCE=..\gctpc\orthfor.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\orthinv.c
# End Source File
# Begin Source File

SOURCE=.\orthos.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\paksz.c
# End Source File
# Begin Source File

SOURCE=..\PCX\PCXTOBMP.C
# End Source File
# Begin Source File

SOURCE=.\peoplnet.c
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

SOURCE=..\solar_calculator\Version2\plu404.c
# End Source File
# Begin Source File

SOURCE=.\pngrid.c
# End Source File
# Begin Source File

SOURCE=.\pointlist.c
# End Source File
# Begin Source File

SOURCE=..\MRGSCoordinates\Mgrs\geotrans2.2.6\dt_cc\polarst\polarst.c
# End Source File
# Begin Source File

SOURCE=..\OFFSET\POLY.C
# End Source File
# Begin Source File

SOURCE=..\gctpc\polyfor.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\polyinv.c
# End Source File
# Begin Source File

SOURCE=..\solar_calculator\Version2\precess.c
# End Source File
# Begin Source File

SOURCE=.\Print.c
# End Source File
# Begin Source File

SOURCE=.\PROFILE.C
# End Source File
# Begin Source File

SOURCE=..\gctpc\psfor.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\psinv.c
# End Source File
# Begin Source File

SOURCE=.\quad.c
# End Source File
# Begin Source File

SOURCE=..\solar_calculator\Version2\refrac.c
# End Source File
# Begin Source File

SOURCE=.\report.C
# End Source File
# Begin Source File

SOURCE=..\gctpc\reportgc.c
# End Source File
# Begin Source File

SOURCE=.\ROADNAME.C
# End Source File
# Begin Source File

SOURCE=..\gctpc\robfor.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\robinv.c
# End Source File
# Begin Source File

SOURCE=..\sunang\ipw\src\lib\libipw\sunang\rotate.c
# End Source File
# Begin Source File

SOURCE=.\route.c
# End Source File
# Begin Source File

SOURCE=..\solar_calculator\Version2\rplanet.c
# End Source File
# Begin Source File

SOURCE=..\solar_calculator\Version2\rstar.c
# End Source File
# Begin Source File

SOURCE=..\solar_calculator\Version2\sat404.c
# End Source File
# Begin Source File

SOURCE=..\SHR\serio.c
# End Source File
# Begin Source File

SOURCE=.\SHPFILE.C
# End Source File
# Begin Source File

SOURCE=..\SHR\shr.c
# End Source File
# Begin Source File

SOURCE=..\solar_calculator\Version2\sidrlt.c
# End Source File
# Begin Source File

SOURCE=.\signinv.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\sinfor.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\sininv.c
# End Source File
# Begin Source File

SOURCE=.\smooth.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\somfor.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\sominv.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\sphdz.c
# End Source File
# Begin Source File

SOURCE=..\splines\ssp.c
# End Source File
# Begin Source File

SOURCE=..\SHR\STD.C
# End Source File
# Begin Source File

SOURCE=..\gctpc\sterfor.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\sterinv.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\stplnfor.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\stplninv.c
# End Source File
# Begin Source File

SOURCE=..\solar_calculator\Version2\sun.c
# End Source File
# Begin Source File

SOURCE=..\sunang\ipw\src\lib\libipw\sunang\sunpath.c
# End Source File
# Begin Source File

SOURCE=..\sunang\ipw\src\exp\ucsb\bin\sunlight\sunrise.c
# End Source File
# Begin Source File

SOURCE=..\GCTP2\T2PROJ~1.C
# End Source File
# Begin Source File

SOURCE=.\tabbed.c
# End Source File
# Begin Source File

SOURCE=..\solar_calculator\Version2\tdb.c
# End Source File
# Begin Source File

SOURCE="..\libbci-1.1.0\tdspl.c"
# End Source File
# Begin Source File

SOURCE=..\GM32lib\testMemIO.cpp
# End Source File
# Begin Source File

SOURCE=.\themes.c
# End Source File
# Begin Source File

SOURCE=.\themes2.c
# End Source File
# Begin Source File

SOURCE=.\THEMES3.C
# End Source File
# Begin Source File

SOURCE=.\THMMSGP.C
# End Source File
# Begin Source File

SOURCE=..\SHR\tiff.c
# End Source File
# Begin Source File

SOURCE=..\sunang\ipw\src\lib\libipw\sunang\tmconv.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\tmfor.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\tminv.c
# End Source File
# Begin Source File

SOURCE=.\toolbar.c
# End Source File
# Begin Source File

SOURCE="..\libbci-1.1.0\tools.c"
# End Source File
# Begin Source File

SOURCE=.\Tran.c
# End Source File
# Begin Source File

SOURCE=.\tranlat2.c
# End Source File
# Begin Source File

SOURCE=.\tranlat3.c
# End Source File
# Begin Source File

SOURCE=..\MRGSCoordinates\Mgrs\geotrans2.2.6\dt_cc\tranmerc\tranmerc.c
# End Source File
# Begin Source File

SOURCE=..\GCTP2\TRANPR~1.C
# End Source File
# Begin Source File

SOURCE=.\translat.c
# End Source File
# Begin Source File

SOURCE=.\traverse.c
# End Source File
# Begin Source File

SOURCE=..\solar_calculator\Version2\trnsit.c
# End Source File
# Begin Source File

SOURCE=..\SHR\UGRID.C
# End Source File
# Begin Source File

SOURCE=.\Ugridhlt.c
# End Source File
# Begin Source File

SOURCE=..\SHR\Umio.c
# End Source File
# Begin Source File

SOURCE=..\SHR\UNDO.C
# End Source File
# Begin Source File

SOURCE=..\sunang\ipw\src\lib\libipw\sunang\unixtime.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\untfz.c
# End Source File
# Begin Source File

SOURCE=..\MRGSCoordinates\Mgrs\geotrans2.2.6\dt_cc\ups\ups.c
# End Source File
# Begin Source File

SOURCE=..\solar_calculator\Version2\ura404.c
# End Source File
# Begin Source File

SOURCE=..\garmin\USB\USBSDK.c
# End Source File
# Begin Source File

SOURCE=..\MRGSCoordinates\Mgrs\geotrans2.2.6\dt_cc\utm\utm.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\utmfor.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\utminv.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\vandgfor.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\vandginv.c
# End Source File
# Begin Source File

SOURCE=.\VARDEF.C
# End Source File
# Begin Source File

SOURCE=..\SHR\VARSTUF2.C
# End Source File
# Begin Source File

SOURCE=..\SHR\VARSTUFF.C
# End Source File
# Begin Source File

SOURCE=..\solar_calculator\Version2\vearth.c
# End Source File
# Begin Source File

SOURCE=.\vehicle.c
# End Source File
# Begin Source File

SOURCE=.\vehicle_server.c
# End Source File
# Begin Source File

SOURCE=..\solar_calculator\Version2\ven404.c
# End Source File
# Begin Source File

SOURCE=.\viewport.c
# End Source File
# Begin Source File

SOURCE=.\viscontrol.c
# End Source File
# Begin Source File

SOURCE=.\visible.c
# End Source File
# Begin Source File

SOURCE=.\VOTERS.C
# End Source File
# Begin Source File

SOURCE=..\sunang\ipw\src\lib\libipw\sunang\weekday.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\wivfor.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\wivinv.c
# End Source File
# Begin Source File

SOURCE=..\SHR\Writeavi.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\wviifor.c
# End Source File
# Begin Source File

SOURCE=..\gctpc\wviiinv.c
# End Source File
# Begin Source File

SOURCE=.\XMLProcessing.c
# End Source File
# Begin Source File

SOURCE=..\sunang\ipw\src\lib\libipw\sunang\yrday.c
# End Source File
# Begin Source File

SOURCE=..\solar_calculator\Version2\zatan2.c
# End Source File
# Begin Source File

SOURCE=..\sunang\ipw\src\exp\ucsb\bin\sunlight\zerobr.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\gctpc\cproj.h
# End Source File
# Begin Source File

SOURCE=..\gctpc\proj.h
# End Source File
# Begin Source File

SOURCE=..\INCLUDE\toolbar.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\Citytour\ALIGN_BO.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\ALIGN_HO.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\ALIGN_LE.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\ALIGN_RI.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\ALIGN_TO.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\ALIGN_VE.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\ARROWQ~1.BMP
# End Source File
# Begin Source File

SOURCE=C:\gssi\prog\GEOMASTR\ASSOC.ICO
# End Source File
# Begin Source File

SOURCE=.\ASSOC.ICO
# End Source File
# Begin Source File

SOURCE=..\Citytour\BITMAP11.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\BITMAP13.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\BITMAP14.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\BITMAP15.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\BITMAP16.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\BITMAP17.BMP
# End Source File
# Begin Source File

SOURCE=..\citytour\BITMAP18.BMP
# End Source File
# Begin Source File

SOURCE=..\citytour\BITMAP19.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\BITMAP2.BMP
# End Source File
# Begin Source File

SOURCE=.\BITMAP2.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\BITMAP3.BMP
# End Source File
# Begin Source File

SOURCE=.\BITMAP3.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\BITMAP4.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\BITMAP9.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\BITMAP_R.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\BLUE_BIT.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\BMP00001.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\BMP00002.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\BMP00003.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\BMP00004.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\BMP00005.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\BMP00006.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\BMP00007.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\BMP00008.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\BMP00009.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\BMP00010.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\BMP00011.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\BMP00012.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\BMP00013.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\BMP00014.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\BMP00015.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\BMP00016.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\BMP00017.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\BMP00018.BMP
# End Source File
# Begin Source File

SOURCE=..\citytour\BMP00019.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\BMP00020.BMP
# End Source File
# Begin Source File

SOURCE=..\citytour\BMP00021.BMP
# End Source File
# Begin Source File

SOURCE=..\citytour\BMP00022.BMP
# End Source File
# Begin Source File

SOURCE=..\citytour\BMP00023.BMP
# End Source File
# Begin Source File

SOURCE=..\citytour\BMP00024.BMP
# End Source File
# Begin Source File

SOURCE=..\citytour\BMP02PCT.BMP
# End Source File
# Begin Source File

SOURCE=..\citytour\BMP06PCT.BMP
# End Source File
# Begin Source File

SOURCE=..\citytour\BMP25PCT.BMP
# End Source File
# Begin Source File

SOURCE=..\citytour\buoy_green.bmp
# End Source File
# Begin Source File

SOURCE=..\Citytour\CHECKED1.BMP
# End Source File
# Begin Source File

SOURCE=..\citytour\CRAYONS.BMP
# End Source File
# Begin Source File

SOURCE=C:\gssi\prog\GEOMASTR\CUR00001.CUR
# End Source File
# Begin Source File

SOURCE=.\CUR00001.CUR
# End Source File
# Begin Source File

SOURCE=C:\gssi\prog\GEOMASTR\CUR00002.CUR
# End Source File
# Begin Source File

SOURCE=.\CUR00002.CUR
# End Source File
# Begin Source File

SOURCE=C:\gssi\prog\GEOMASTR\CURSOR5.CUR
# End Source File
# Begin Source File

SOURCE=.\CURSOR5.CUR
# End Source File
# Begin Source File

SOURCE=.\CURSOR6.CUR
# End Source File
# Begin Source File

SOURCE=.\CURSOR7.CUR
# End Source File
# Begin Source File

SOURCE=C:\gssi\prog\GEOMASTR\CURSOR_L.CUR
# End Source File
# Begin Source File

SOURCE=.\CURSOR_L.CUR
# End Source File
# Begin Source File

SOURCE=C:\gssi\prog\GEOMASTR\DIGITIZE.CUR
# End Source File
# Begin Source File

SOURCE=.\DIGITIZE.CUR
# End Source File
# Begin Source File

SOURCE=..\Citytour\DOWN_ARR.BMP
# End Source File
# Begin Source File

SOURCE=C:\gssi\prog\GEOMASTR\DRAWING.CUR
# End Source File
# Begin Source File

SOURCE=.\DRAWING.CUR
# End Source File
# Begin Source File

SOURCE=C:\gssi\prog\GEOMASTR\EARTH.ICO
# End Source File
# Begin Source File

SOURCE=.\EARTH.ICO
# End Source File
# Begin Source File

SOURCE=..\citytour\GEOMASTE.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\GMLOGOW.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\GREEN_BI.BMP
# End Source File
# Begin Source File

SOURCE=C:\gssi\prog\GEOMASTR\HANDMOVE.CUR
# End Source File
# Begin Source File

SOURCE=.\HANDMOVE.CUR
# End Source File
# Begin Source File

SOURCE=".\HLP-DWN.BMP"
# End Source File
# Begin Source File

SOURCE=".\HLP-UP.BMP"
# End Source File
# Begin Source File

SOURCE=C:\gssi\prog\GEOMASTR\HMOVE.CUR
# End Source File
# Begin Source File

SOURCE=.\HMOVE.CUR
# End Source File
# Begin Source File

SOURCE=C:\gssi\prog\GEOMASTR\HOME.ICO
# End Source File
# Begin Source File

SOURCE=.\HOME.ICO
# End Source File
# Begin Source File

SOURCE=C:\gssi\prog\GEOMASTR\ICO00001.ICO
# End Source File
# Begin Source File

SOURCE=.\ICO00001.ICO
# End Source File
# Begin Source File

SOURCE=C:\gssi\prog\GEOMASTR\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\icon1.ico
# End Source File
# Begin Source File

SOURCE=C:\gssi\prog\GEOMASTR\ICON6.ICO
# End Source File
# Begin Source File

SOURCE=.\ICON6.ICO
# End Source File
# Begin Source File

SOURCE=.\ICON7.ICO
# End Source File
# Begin Source File

SOURCE=..\Citytour\LABEL.BMP
# End Source File
# Begin Source File

SOURCE=C:\gssi\prog\GEOMASTR\LABEL_CU.CUR
# End Source File
# Begin Source File

SOURCE=.\LABEL_CU.CUR
# End Source File
# Begin Source File

SOURCE=C:\gssi\prog\GEOMASTR\LARROWS.CUR
# End Source File
# Begin Source File

SOURCE=.\LARROWS.CUR
# End Source File
# Begin Source File

SOURCE=..\Citytour\LEFT_ARR.BMP
# End Source File
# Begin Source File

SOURCE=C:\gssi\prog\GEOMASTR\MAGNIFY.CUR
# End Source File
# Begin Source File

SOURCE=.\MAGNIFY.CUR
# End Source File
# Begin Source File

SOURCE=C:\gssi\prog\GEOMASTR\MEASURE_.CUR
# End Source File
# Begin Source File

SOURCE=.\MEASURE_.CUR
# End Source File
# Begin Source File

SOURCE=..\Citytour\MONO1.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\MONO2.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\MONO3.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\MONO4.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\MONO5.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\MONO6.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\MONO7.BMP
# End Source File
# Begin Source File

SOURCE=C:\gssi\prog\GEOMASTR\NAMES.ICO
# End Source File
# Begin Source File

SOURCE=.\NAMES.ICO
# End Source File
# Begin Source File

SOURCE=..\Citytour\NUMERIC_.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\PAT2.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\PHONE1.BMP
# End Source File
# Begin Source File

SOURCE=C:\gssi\prog\GEOMASTR\PICK_CUR.CUR
# End Source File
# Begin Source File

SOURCE=.\PICK_CUR.CUR
# End Source File
# Begin Source File

SOURCE=..\Citytour\PICKCURS.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\RIGHT_AR.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\S_TEMP.BMP
# End Source File
# Begin Source File

SOURCE=C:\gssi\prog\GEOMASTR\SETTAG.CUR
# End Source File
# Begin Source File

SOURCE=.\SETTAG.CUR
# End Source File
# Begin Source File

SOURCE=C:\gssi\prog\GEOMASTR\SHOW.CUR
# End Source File
# Begin Source File

SOURCE=.\SHOW.CUR
# End Source File
# Begin Source File

SOURCE=..\Citytour\SPORTMAP.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\STANDARD.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\STATIC_D.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\STATIC_U.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\SYMBOL.BMP
# End Source File
# Begin Source File

SOURCE=C:\gssi\prog\GEOMASTR\SYMBOL_C.CUR
# End Source File
# Begin Source File

SOURCE=.\SYMBOL_C.CUR
# End Source File
# Begin Source File

SOURCE=..\Citytour\TDCHECK.BMP
# End Source File
# Begin Source File

SOURCE=.\TDCOMBO.BMP
# End Source File
# Begin Source File

SOURCE=.\TDEDIT.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\TDFRAME.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\TDGROUP.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\TDICON.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\TDLIST.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\TDPOINTR.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\TDPUSH.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\TDRECT.BMP
# End Source File
# Begin Source File

SOURCE=C:\gssi\prog\GEOMASTR\TOGGLEVI.CUR
# End Source File
# Begin Source File

SOURCE=.\TOGGLEVI.CUR
# End Source File
# Begin Source File

SOURCE=..\Citytour\TOOLBAR1.BMP
# End Source File
# Begin Source File

SOURCE=.\TRANBM.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\TUCHECK.BMP
# End Source File
# Begin Source File

SOURCE=.\TUCOMBO.BMP
# End Source File
# Begin Source File

SOURCE=.\TUEDIT.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\TUFRAME.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\TUGROUP.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\TUICON.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\TULIST.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\TUPOINTR.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\TUPUSH.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\TURADIO.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\TURECT.BMP
# End Source File
# Begin Source File

SOURCE=..\Citytour\UP_ARROW.BMP
# End Source File
# Begin Source File

SOURCE=C:\gssi\prog\GEOMASTR\WAYPOINT.CUR
# End Source File
# Begin Source File

SOURCE=.\WAYPOINT.CUR
# End Source File
# Begin Source File

SOURCE=C:\gssi\prog\GEOMASTR\ZOOMIN.CUR
# End Source File
# Begin Source File

SOURCE=.\ZOOMIN.CUR
# End Source File
# Begin Source File

SOURCE=..\Citytour\zoompan8.bmp
# End Source File
# End Group
# Begin Source File

SOURCE=..\GM32lib\FreeImage.lib
# End Source File
# Begin Source File

SOURCE=..\DGNlib7\Release\DGNlib7_32.lib
# End Source File
# End Target
# End Project
