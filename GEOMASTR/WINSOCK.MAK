# Microsoft Visual C++ generated build script - Do not modify

PROJ = WINSOCK
DEBUG = 0
PROGTYPE = 1
CALLER = 
ARGS = 
DLLS = 
D_RCDEFINES = -d_DEBUG
R_RCDEFINES = -dNDEBUG
ORIGIN = MSVC
ORIGIN_VER = 1.00
PROJPATH = C:\GSSI\PROG\GWIZ\
USEMFC = 0
CC = cl
CPP = cl
CXX = cl
CCREATEPCHFLAG = 
CPPCREATEPCHFLAG = 
CUSEPCHFLAG = 
CPPUSEPCHFLAG = 
FIRSTC = GWIZSOCK.C  
FIRSTCPP =             
RC = rc
CFLAGS_D_WDLL = /nologo /W3 /FR /G2 /Zi /D_DEBUG /Od /GD /ALw
CFLAGS_R_WDLL = /nologo /W3 /FR /O1 /DNDEBUG /GD /ALw
LFLAGS_D_WDLL = /NOLOGO /ONERROR:NOEXE /NOD /PACKC:61440 /CO /NOE /ALIGN:16 /MAP:FULL
LFLAGS_R_WDLL = /NOLOGO /ONERROR:NOEXE /NOD /PACKC:61440 /NOE /ALIGN:16 /MAP:FULL
LIBS_D_WDLL = oldnames libw commdlg shell olecli olesvr ldllcew
LIBS_R_WDLL = oldnames libw commdlg shell olecli olesvr ldllcew
RCFLAGS = /nologo
RESFLAGS = /nologo
RUNFLAGS = 
DEFFILE = GWIZSOCK.DEF
OBJS_EXT = 
LIBS_EXT = 
!if "$(DEBUG)" == "1"
CFLAGS = $(CFLAGS_D_WDLL)
LFLAGS = $(LFLAGS_D_WDLL)
LIBS = $(LIBS_D_WDLL)
MAPFILE = nul
RCDEFINES = $(D_RCDEFINES)
!else
CFLAGS = $(CFLAGS_R_WDLL)
LFLAGS = $(LFLAGS_R_WDLL)
LIBS = $(LIBS_R_WDLL)
MAPFILE = nul
RCDEFINES = $(R_RCDEFINES)
!endif
!if [if exist MSVC.BND del MSVC.BND]
!endif
SBRS = GWIZSOCK.SBR


GWIZSOCK_DEP = c:\msvc\include\winsock.h


all:	$(PROJ).DLL $(PROJ).BSC

GWIZSOCK.OBJ:	GWIZSOCK.C $(GWIZSOCK_DEP)
	$(CC) $(CFLAGS) $(CCREATEPCHFLAG) /c GWIZSOCK.C


$(PROJ).DLL::	GWIZSOCK.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
GWIZSOCK.OBJ +
$(OBJS_EXT)
$(PROJ).DLL
$(MAPFILE)
c:\msvc\lib\+
c:\msvc\mfc\lib\+
c:\qcwin\lib\+
c:\msc\lib\+
c:\odbcsdk\lib\+
c:\vfwdk\lib\+
$(LIBS)
$(DEFFILE);
<<
	link $(LFLAGS) @$(PROJ).CRF
	$(RC) $(RESFLAGS) $@
	implib /nowep $(PROJ).LIB $(PROJ).DLL


run: $(PROJ).DLL
	$(PROJ) $(RUNFLAGS)


$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<
