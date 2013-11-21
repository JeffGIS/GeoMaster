# Microsoft Visual C++ generated build script - Do not modify

PROJ = BASIC
DEBUG = 1
PROGTYPE = 0
CALLER = 
ARGS = 
DLLS = 
D_RCDEFINES = -d_DEBUG
R_RCDEFINES = -dNDEBUG
ORIGIN = MSVC
ORIGIN_VER = 1.00
PROJPATH = C:\GSSI\PROG\GARMIN\
USEMFC = 0
CC = cl
CPP = cl
CXX = cl
CCREATEPCHFLAG = 
CPPCREATEPCHFLAG = 
CUSEPCHFLAG = 
CPPUSEPCHFLAG = 
FIRSTC = BASIC.C     
FIRSTCPP =             
RC = rc
CFLAGS_D_WEXE = /nologo /G2 /W3 /Zi /AL /Od /D "_DEBUG" /FR /GA /Fd"BASIC.PDB"
CFLAGS_R_WEXE = /nologo /W3 /AM /O1 /D "NDEBUG" /FR /GA 
LFLAGS_D_WEXE = /NOLOGO /ONERROR:NOEXE /NOD /PACKC:61440 /CO /ALIGN:16 /STACK:10240
LFLAGS_R_WEXE = /NOLOGO /ONERROR:NOEXE /NOD /PACKC:61440 /ALIGN:16 /STACK:10240
LIBS_D_WEXE = oldnames libw commdlg shell olecli olesvr llibcew
LIBS_R_WEXE = oldnames libw commdlg shell olecli olesvr mlibcew
RCFLAGS = /nologo
RESFLAGS = /nologo
RUNFLAGS = 
DEFFILE = BASIC.DEF
OBJS_EXT = 
LIBS_EXT = 
!if "$(DEBUG)" == "1"
CFLAGS = $(CFLAGS_D_WEXE)
LFLAGS = $(LFLAGS_D_WEXE)
LIBS = $(LIBS_D_WEXE)
MAPFILE = nul
RCDEFINES = $(D_RCDEFINES)
!else
CFLAGS = $(CFLAGS_R_WEXE)
LFLAGS = $(LFLAGS_R_WEXE)
LIBS = $(LIBS_R_WEXE)
MAPFILE = nul
RCDEFINES = $(R_RCDEFINES)
!endif
!if [if exist MSVC.BND del MSVC.BND]
!endif
SBRS = BASIC.SBR \
		DLGLOOK.SBR


BASIC_DEP = c:\gssi\prog\garmin\basic.h \
	c:\gssi\prog\garmin\garmin.h


BASIC_RCDEP = c:\gssi\prog\garmin\basic.h


DLGLOOK_DEP = c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h


all:	$(PROJ).EXE $(PROJ).BSC

BASIC.OBJ:	BASIC.C $(BASIC_DEP)
	$(CC) $(CFLAGS) $(CCREATEPCHFLAG) /c BASIC.C

BASIC.RES:	BASIC.RC $(BASIC_RCDEP)
	$(RC) $(RCFLAGS) $(RCDEFINES) -r BASIC.RC

DLGLOOK.OBJ:	..\SHR\DLGLOOK.C $(DLGLOOK_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\SHR\DLGLOOK.C


$(PROJ).EXE::	BASIC.RES

$(PROJ).EXE::	BASIC.OBJ DLGLOOK.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
BASIC.OBJ +
DLGLOOK.OBJ +
$(OBJS_EXT)
$(PROJ).EXE
$(MAPFILE)
c:\msvc\lib\+
c:\msvc\mfc\lib\+
$(LIBS)
$(DEFFILE);
<<
	link $(LFLAGS) @$(PROJ).CRF
	$(RC) $(RESFLAGS) BASIC.RES $@
	@copy $(PROJ).CRF MSVC.BND

$(PROJ).EXE::	BASIC.RES
	if not exist MSVC.BND 	$(RC) $(RESFLAGS) BASIC.RES $@

run: $(PROJ).EXE
	$(PROJ) $(RUNFLAGS)


$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<
