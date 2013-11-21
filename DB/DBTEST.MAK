# Microsoft Visual C++ generated build script - Do not modify

PROJ = DBTEST
DEBUG = 0
PROGTYPE = 0
CALLER = 
ARGS = 
DLLS = 
D_RCDEFINES = -d_DEBUG
R_RCDEFINES = -dNDEBUG
ORIGIN = MSVC
ORIGIN_VER = 1.00
PROJPATH = C:\GSSI\PROG\DB\
USEMFC = 0
CC = cl
CPP = cl
CXX = cl
CCREATEPCHFLAG = 
CPPCREATEPCHFLAG = 
CUSEPCHFLAG = 
CPPUSEPCHFLAG = 
FIRSTC = BT_CREAT.C  
FIRSTCPP =             
RC = rc
CFLAGS_D_WEXE = /nologo /G2 /W3 /Zi /AL /Od /D "_DEBUG" /FR /GA /Fd"DBTEST.PDB"
CFLAGS_R_WEXE = /nologo /W3 /AL /O1 /D "NDEBUG" /FR /GA 
LFLAGS_D_WEXE = /NOLOGO /NOD /PACKC:61440 /STACK:10240 /ALIGN:16 /ONERROR:NOEXE /CO  
LFLAGS_R_WEXE = /NOLOGO /NOD /PACKC:61440 /STACK:10240 /ALIGN:16 /ONERROR:NOEXE  
LIBS_D_WEXE = oldnames libw llibcew commdlg.lib 
LIBS_R_WEXE = oldnames libw llibcew commdlg.lib olecli.lib olesvr.lib shell.lib 
RCFLAGS = /nologo
RESFLAGS = /nologo
RUNFLAGS = 
DEFFILE = DBTEST.DEF
OBJS_EXT = 
LIBS_EXT = ..\WINCAP\DIBAPI.LIB 
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
SBRS = BT_CREAT.SBR \
		DBTEST.SBR \
		SHR.SBR \
		COMDLG.SBR \
		DLGLOOK.SBR


DIBAPI_DEP = 

BT_CREAT_DEP = c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\shr.h


DBTEST_DEP = c:\gssi\prog\db\dbtest.h \
	c:\gssi\prog\db\compress.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\gwd.h


SHR_DEP = c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\p_tol.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\dibapi.h


DBTEST_RCDEP = c:\gssi\prog\db\dbtest.h \
	c:\gssi\prog\db\compress.h \
	c:\gssi\prog\db\testload.dlg \
	c:\gssi\prog\db\compress.dlg


COMDLG_DEP = c:\gssi\prog\include\cddemo.h \
	c:\gssi\prog\include\shr.h


DLGLOOK_DEP = 

all:	$(PROJ).EXE $(PROJ).BSC

BT_CREAT.OBJ:	BT_CREAT.C $(BT_CREAT_DEP)
	$(CC) $(CFLAGS) $(CCREATEPCHFLAG) /c BT_CREAT.C

DBTEST.OBJ:	DBTEST.C $(DBTEST_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c DBTEST.C

SHR.OBJ:	..\SHR\SHR.C $(SHR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\SHR\SHR.C

DBTEST.RES:	DBTEST.RC $(DBTEST_RCDEP)
	$(RC) $(RCFLAGS) $(RCDEFINES) -r DBTEST.RC

COMDLG.OBJ:	..\SHR\COMDLG.C $(COMDLG_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\SHR\COMDLG.C

DLGLOOK.OBJ:	..\SHR\DLGLOOK.C $(DLGLOOK_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\SHR\DLGLOOK.C


$(PROJ).EXE::	DBTEST.RES

$(PROJ).EXE::	BT_CREAT.OBJ DBTEST.OBJ SHR.OBJ COMDLG.OBJ DLGLOOK.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
BT_CREAT.OBJ +
DBTEST.OBJ +
SHR.OBJ +
COMDLG.OBJ +
DLGLOOK.OBJ +
$(OBJS_EXT)
$(PROJ).EXE
$(MAPFILE)
c:\msvc\lib\+
c:\msvc\mfc\lib\+
c:\gdb400\msvc1x\+
..\WINCAP\DIBAPI.LIB+
$(LIBS)
$(DEFFILE);
<<
	link $(LFLAGS) @$(PROJ).CRF
	$(RC) $(RESFLAGS) DBTEST.RES $@
	@copy $(PROJ).CRF MSVC.BND

$(PROJ).EXE::	DBTEST.RES
	if not exist MSVC.BND 	$(RC) $(RESFLAGS) DBTEST.RES $@

run: $(PROJ).EXE
	$(PROJ) $(RUNFLAGS)


$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<
