# Microsoft Visual C++ generated build script - Do not modify

PROJ = GWIZ
DEBUG = 1
PROGTYPE = 0
CALLER = 
ARGS = \test1.gwc
DLLS = 
D_RCDEFINES = /d_DEBUG 
R_RCDEFINES = /dNDEBUG 
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
FIRSTC = CWCONFIG.C  
FIRSTCPP =             
RC = rc
CFLAGS_D_WEXE = /nologo /f- /G3 /FPc /Zp1 /Zi /AL /Od /D "_DEBUG" /GA /Fd"GWIZ.PDB"
CFLAGS_R_WEXE = /nologo /f- /Gs /G3 /FPc /Zp1 /W3 /AL /O2 /Ox /Ob2 /D "NDEBUG" /FR /GA 
LFLAGS_D_WEXE = /NOLOGO /NOD /PACKC:61440 /STACK:10240 /ALIGN:16 /ONERROR:NOEXE /CO  
LFLAGS_R_WEXE = /NOLOGO /NOD /PACKC:61440 /STACK:10240 /ALIGN:16 /ONERROR:NOEXE  
LIBS_D_WEXE = oldnames libw llibcew commdlg.lib ddeml.lib olecli.lib olesvr.lib shell.lib 
LIBS_R_WEXE = oldnames libw llibcew commdlg.lib olecli.lib olesvr.lib shell.lib 
RCFLAGS = /nologo 
RESFLAGS = /nologo /k
RUNFLAGS = 
DEFFILE = GWCONFIG.DEF
OBJS_EXT = 
LIBS_EXT = ..\..\..\MSVC\LIB\COMMDLG.LIB 
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
SBRS = CWCONFIG.SBR \
		GRAPHIC1.SBR \
		GRAPHIC2.SBR \
		SHR.SBR \
		BT_CREAT.SBR \
		COMDLG.SBR \
		ADDRESS1.SBR \
		CREATCFG.SBR \
		CLIDDE.SBR \
		DDE_MAIN.SBR \
		CLIDATA.SBR \
		CLIENT.SBR \
		DDE.SBR


COMMDLG_DEP = 

CWCONFIG_DEP = c:\gssi\prog\include\shr.h \
	c:\gssi\prog\gwiz\address.h \
	c:\gssi\prog\gwiz\gw.h \
	c:\gssi\prog\gwiz\graphics.h \
	c:\gssi\prog\gwiz\graphint.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\ddemlsv.h \
	c:\msvc\include\winsock.h \
	c:\gssi\prog\include\dyndlg.h \
	c:\gssi\prog\include\umio.h


GRAPHIC1_DEP = c:\gssi\prog\include\config.h \
	c:\gssi\prog\gwiz\graphint.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\gwiz\graphics.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\cddemo.h \
	c:\gssi\prog\include\dyndlg.h


GRAPHIC2_DEP = c:\gssi\prog\gwiz\graphint.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\gwiz\graphics.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dyndlg.h \
	c:\gssi\prog\gwiz\gw.h \
	c:\gssi\prog\include\tagedit.h


GWCONFIG_RCDEP = c:\gssi\prog\gwiz\gw.h \
	c:\gssi\prog\gwiz\address.h \
	c:\gssi\prog\include\tagedit.h \
	c:\gssi\prog\include\cddemo.h \
	c:\gssi\prog\gwiz\gwiz.ico


SHR_DEP = c:\gssi\prog\include\shr.h


BT_CREAT_DEP = c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\shr.h


COMDLG_DEP = c:\gssi\prog\include\cddemo.h \
	c:\gssi\prog\include\shr.h


ADDRESS1_DEP = c:\gssi\prog\gwiz\address.h \
	c:\gssi\prog\include\bt.h


CREATCFG_DEP = c:\gssi\prog\gwiz\graphint.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\gwiz\graphics.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dyndlg.h


CLIDDE_DEP = c:\gssi\prog\include\clires.h \
	c:\gssi\prog\include\client.h


DDE_MAIN_DEP = c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\extrndb.h


CLIDATA_DEP = c:\gssi\prog\include\clires.h \
	c:\gssi\prog\include\client.h


CLIENT_DEP = c:\gssi\prog\include\clires.h \
	c:\gssi\prog\include\client.h


DDE_DEP = c:\gssi\prog\include\ddemlsv.h \
	c:\gssi\prog\include\huge.h


all:	$(PROJ).EXE

CWCONFIG.OBJ:	CWCONFIG.C $(CWCONFIG_DEP)
	$(CC) $(CFLAGS) $(CCREATEPCHFLAG) /c CWCONFIG.C

GRAPHIC1.OBJ:	GRAPHIC1.C $(GRAPHIC1_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c GRAPHIC1.C

GRAPHIC2.OBJ:	GRAPHIC2.C $(GRAPHIC2_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c GRAPHIC2.C

GWCONFIG.RES:	GWCONFIG.RC $(GWCONFIG_RCDEP)
	$(RC) $(RCFLAGS) $(RCDEFINES) -r GWCONFIG.RC

SHR.OBJ:	..\SHR\SHR.C $(SHR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\SHR\SHR.C

BT_CREAT.OBJ:	..\DB\BT_CREAT.C $(BT_CREAT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\DB\BT_CREAT.C

COMDLG.OBJ:	..\SHR\COMDLG.C $(COMDLG_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\SHR\COMDLG.C

ADDRESS1.OBJ:	ADDRESS1.C $(ADDRESS1_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ADDRESS1.C

CREATCFG.OBJ:	CREATCFG.C $(CREATCFG_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c CREATCFG.C

CLIDDE.OBJ:	..\SHR\CLIDDE.C $(CLIDDE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\SHR\CLIDDE.C

DDE_MAIN.OBJ:	..\SHR\DDE_MAIN.C $(DDE_MAIN_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\SHR\DDE_MAIN.C

CLIDATA.OBJ:	..\SHR\CLIDATA.C $(CLIDATA_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\SHR\CLIDATA.C

CLIENT.OBJ:	..\SHR\CLIENT.C $(CLIENT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\SHR\CLIENT.C

DDE.OBJ:	DDE.C $(DDE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c DDE.C


$(PROJ).EXE::	GWCONFIG.RES

$(PROJ).EXE::	CWCONFIG.OBJ GRAPHIC1.OBJ GRAPHIC2.OBJ SHR.OBJ BT_CREAT.OBJ COMDLG.OBJ \
	ADDRESS1.OBJ CREATCFG.OBJ CLIDDE.OBJ DDE_MAIN.OBJ CLIDATA.OBJ CLIENT.OBJ DDE.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
CWCONFIG.OBJ +
GRAPHIC1.OBJ +
GRAPHIC2.OBJ +
SHR.OBJ +
BT_CREAT.OBJ +
COMDLG.OBJ +
ADDRESS1.OBJ +
CREATCFG.OBJ +
CLIDDE.OBJ +
DDE_MAIN.OBJ +
CLIDATA.OBJ +
CLIENT.OBJ +
DDE.OBJ +
$(OBJS_EXT)
$(PROJ).EXE
$(MAPFILE)
c:\msvc\lib\+
c:\msvc\mfc\lib\+
..\..\..\MSVC\LIB\COMMDLG.LIB+
$(LIBS)
$(DEFFILE);
<<
	link $(LFLAGS) @$(PROJ).CRF
	$(RC) $(RESFLAGS) GWCONFIG.RES $@
	@copy $(PROJ).CRF MSVC.BND

$(PROJ).EXE::	GWCONFIG.RES
	if not exist MSVC.BND 	$(RC) $(RESFLAGS) GWCONFIG.RES $@

run: $(PROJ).EXE
	$(PROJ) $(RUNFLAGS)


$(PROJ).BSC: $(SBRS)
	bscmake @<<
/o$@ $(SBRS)
<<
