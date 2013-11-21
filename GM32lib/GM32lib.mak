# Microsoft Developer Studio Generated NMAKE File, Based on GM32lib.dsp
!IF "$(CFG)" == ""
CFG=GM32lib - Win32 Debug
!MESSAGE No configuration specified. Defaulting to GM32lib - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "GM32lib - Win32 Release" && "$(CFG)" != "GM32lib - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "GM32lib.mak" CFG="GM32lib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "GM32lib - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "GM32lib - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "GM32lib - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\GM32lib.dll"


CLEAN :
	-@erase "$(INTDIR)\GM32lib.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\GM32lib.dll"
	-@erase "$(OUTDIR)\GM32lib.exp"
	-@erase "$(OUTDIR)\GM32lib.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GM32LIB_EXPORTS" /Fp"$(INTDIR)\GM32lib.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\GM32lib.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\GM32lib.pdb" /machine:I386 /def:".\GM32lib.def" /out:"$(OUTDIR)\GM32lib.dll" /implib:"$(OUTDIR)\GM32lib.lib" 
DEF_FILE= \
	".\GM32lib.def"
LINK32_OBJS= \
	"$(INTDIR)\GM32lib.obj"

"$(OUTDIR)\GM32lib.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "GM32lib - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\GM32lib.dll"


CLEAN :
	-@erase "$(INTDIR)\GM32lib.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\GM32lib.dll"
	-@erase "$(OUTDIR)\GM32lib.exp"
	-@erase "$(OUTDIR)\GM32lib.ilk"
	-@erase "$(OUTDIR)\GM32lib.lib"
	-@erase "$(OUTDIR)\GM32lib.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GM32LIB_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\GM32lib.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\GM32lib.pdb" /debug /machine:I386 /def:".\GM32lib.def" /out:"$(OUTDIR)\GM32lib.dll" /implib:"$(OUTDIR)\GM32lib.lib" /pdbtype:sept 
DEF_FILE= \
	".\GM32lib.def"
LINK32_OBJS= \
	"$(INTDIR)\GM32lib.obj"

"$(OUTDIR)\GM32lib.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("GM32lib.dep")
!INCLUDE "GM32lib.dep"
!ELSE 
!MESSAGE Warning: cannot find "GM32lib.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "GM32lib - Win32 Release" || "$(CFG)" == "GM32lib - Win32 Debug"
SOURCE=.\GM32lib.c

"$(INTDIR)\GM32lib.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

