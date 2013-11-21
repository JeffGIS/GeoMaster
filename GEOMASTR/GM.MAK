# Microsoft Visual C++ generated build script - Do not modify

PROJ = GM
DEBUG = 0
PROGTYPE = 0
CALLER = 
ARGS = mporth.gwc /C
DLLS = 
D_RCDEFINES = /d_DEBUG 
R_RCDEFINES = /dNDEBUG 
ORIGIN = MSVC
ORIGIN_VER = 1.00
PROJPATH = C:\GSSI\PROG\GEOMASTR\
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
CFLAGS_D_WEXE = /nologo /f- /Zp1 /Zi /AL /Gt16 /Gx- /Od /D "_DEBUG" /FR /GA /Fd"GWIZ.PDB"
CFLAGS_R_WEXE = /nologo /f- /G3 /Zp1 /W3 /ALu /Gt16 /Gx- /Ob2 /D "NDEBUG" /FR 
LFLAGS_D_WEXE = /NOLOGO /NOD /PACKC:61440 /STACK:11264 /ALIGN:16 /ONERROR:NOEXE /CO /MAP /LINE  
LFLAGS_R_WEXE = /NOLOGO /NOD /PACKC:61440 /STACK:12288 /ALIGN:16 /ONERROR:NOEXE /CO /MAP /LINE  
LIBS_D_WEXE = odbc odbc oldnames libw llibcew commdlg.lib ddeml.lib olecli.lib olesvr.lib shell.lib 
LIBS_R_WEXE = oldnames libw llibcew commdlg.lib ddeml.lib shell.lib 
RCFLAGS = /nologo 
RESFLAGS = /nologo /k
RUNFLAGS = 
DEFFILE = GWCONFIG.DEF
OBJS_EXT = 
LIBS_EXT = ..\..\..\ODBCSDK\LIB\ODBC.LIB ..\..\..\MSVC\LIB\WINSOCK.LIB ..\WINCAP\DIBAPI.LIB \
	..\..\..\MSVC\LIB\MMSYSTEM.LIB ..\..\..\VFWDK\LIB\VFW.LIB ..\SHR\WINEXEC.LIB ..\LIB\ULTGRD.LIB \
	..\GCTP2\GCTP16.LIB 
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
		ODBCMAIN.SBR \
		GRAPHIC3.SBR \
		UMIO.SBR \
		VARSTUFF.SBR \
		GWD.SBR \
		LOADLIBS.SBR \
		COMPUT1.SBR \
		OFFSET.SBR \
		POLY.SBR \
		MS_MAPL.SBR \
		HASH.SBR \
		GRAPHIC4.SBR \
		DLGLOOK.SBR \
		CVTCOORD.SBR \
		GRAPHIC5.SBR \
		WRITEAVI.SBR \
		INFOBOX.SBR \
		UGRID.SBR \
		UGRIDHLT.SBR \
		STD.SBR \
		VIDEO.SBR \
		MCI.SBR \
		DLL.SBR \
		CONNAD83.SBR \
		LOADPR~1.SBR \
		T2PROJ~1.SBR \
		TRANPR~1.SBR \
		DDE.SBR


ODBC_DEP = 

WINSOCK_DEP = 

DIBAPI_DEP = 

MMSYSTEM_DEP = 

VFW_DEP = 

WINEXEC_DEP = 

ULTGRD_DEP = 

GCTP16_DEP = 

CWCONFIG_DEP = c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\geomastr\graphics.h \
	c:\gssi\prog\geomastr\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\geomastr\address.h \
	c:\gssi\prog\include\client.h \
	c:\msvc\include\winsock.h \
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\dyndlg.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\mci.h


GRAPHIC1_DEP = c:\gssi\prog\include\config.h \
	c:\gssi\prog\geomastr\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\geomastr\graphics.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\cddemo.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\dyndlg.h \
	c:\gssi\prog\include\aviout.h \
	c:\gssi\prog\include\dibapi.h


GRAPHIC2_DEP = c:\gssi\prog\geomastr\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\geomastr\graphics.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dyndlg.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\geomastr\address.h


GWCONFIG_RCDEP = c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\geomastr\address.h \
	c:\gssi\prog\include\tagedit.h \
	c:\gssi\prog\include\cddemo.h \
	c:\gssi\prog\geomastr\earth.ico \
	c:\gssi\prog\geomastr\bitmap2.bmp \
	c:\gssi\prog\geomastr\tranbm.bmp \
	c:\gssi\prog\citytour\pat2.bmp \
	c:\gssi\prog\geomastr\bitmap3.bmp \
	c:\gssi\prog\citytour\blue_bit.bmp \
	c:\gssi\prog\citytour\green_bi.bmp \
	c:\gssi\prog\citytour\bitmap2.bmp \
	c:\gssi\prog\citytour\bitmap4.bmp \
	c:\gssi\prog\citytour\up_arrow.bmp \
	c:\gssi\prog\citytour\down_arr.bmp \
	c:\gssi\prog\citytour\bmp00002.bmp \
	c:\gssi\prog\citytour\bmp00006.bmp \
	c:\gssi\prog\citytour\bmp00001.bmp \
	c:\gssi\prog\citytour\left_arr.bmp \
	c:\gssi\prog\citytour\bmp00003.bmp \
	c:\gssi\prog\citytour\right_ar.bmp \
	c:\gssi\prog\citytour\bmp00008.bmp \
	c:\gssi\prog\citytour\bmp00004.bmp \
	c:\gssi\prog\citytour\bitmap3.bmp \
	c:\gssi\prog\citytour\bmp00005.bmp \
	c:\gssi\prog\citytour\bmp00007.bmp \
	c:\gssi\prog\geomastr\cursor1.cur \
	c:\gssi\prog\geomastr\larrows.cur


SHR_DEP = c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\p_tol.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\dibapi.h


BT_CREAT_DEP = c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\shr.h


COMDLG_DEP = c:\gssi\prog\include\cddemo.h \
	c:\gssi\prog\include\shr.h


ADDRESS1_DEP = c:\gssi\prog\geomastr\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\geomastr\graphics.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\geomastr\address.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\std.h


CREATCFG_DEP = c:\gssi\prog\geomastr\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\geomastr\graphics.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dyndlg.h \
	c:\gssi\prog\geomastr\gw.h


ODBCMAIN_DEP = c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\bt.h


GRAPHIC3_DEP = c:\gssi\prog\geomastr\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\geomastr\graphics.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\offsetmn.h \
	c:\gssi\prog\include\polycom.h


UMIO_DEP = c:\msvc\include\winsock.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\shr.h


VARSTUFF_DEP = c:\gssi\prog\geomastr\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\geomastr\graphics.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h


GWD_DEP = c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\gw.h \
	c:\gssi\prog\include\bt.h


COMPUT1_DEP = c:\gssi\prog\include\shr.h


COMPUT1_DEP = c:\gssi\prog\include\xll.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\p_tol.h


OFFSET_DEP = c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\offsetmn.h \
	c:\gssi\prog\include\polycom.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\polybt.h \
	c:\gssi\prog\include\p_tol.h \
	c:\gssi\prog\include\offdefs.h


POLY_DEP = c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\polybt.h \
	c:\gssi\prog\include\offsetmn.h \
	c:\gssi\prog\include\polycom.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\offdefs.h \
	c:\gssi\prog\include\p_tol.h \
	c:\gssi\prog\include\cnstnt.h


MS_MAPL_DEP = c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\mapl.h


HASH_DEP = c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\shr.h


GRAPHIC4_DEP = c:\gssi\prog\include\config.h \
	c:\gssi\prog\geomastr\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\geomastr\graphics.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\cddemo.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\mci.h \
	c:\gssi\prog\include\dyndlg.h \
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\offsetmn.h


DLGLOOK_DEP = 

CVTCOORD_DEP = c:\gssi\prog\include\shr.h


GRAPHIC5_DEP = c:\vfwdk\inc\vfw.h \
	c:\vfwdk\inc\compman.h \
	c:\vfwdk\inc\compddk.h \
	c:\vfwdk\inc\drawdib.h \
	c:\vfwdk\inc\avifmt.h \
	c:\vfwdk\inc\mmreg.h \
	c:\vfwdk\inc\avifile.h \
	c:\vfwdk\inc\aviiface.h \
	c:\vfwdk\inc\mciwnd.h \
	c:\vfwdk\inc\msvideo.h \
	c:\vfwdk\inc\avicap.h \
	c:\vfwdk\inc\msacm.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\geomastr\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\geomastr\graphics.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dyndlg.h \
	c:\gssi\prog\geomastr\address.h \
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h


WRITEAVI_DEP = c:\vfwdk\inc\vfw.h \
	c:\vfwdk\inc\compman.h \
	c:\vfwdk\inc\compddk.h \
	c:\vfwdk\inc\drawdib.h \
	c:\vfwdk\inc\avifmt.h \
	c:\vfwdk\inc\mmreg.h \
	c:\vfwdk\inc\avifile.h \
	c:\vfwdk\inc\aviiface.h \
	c:\vfwdk\inc\mciwnd.h \
	c:\vfwdk\inc\msvideo.h \
	c:\vfwdk\inc\avicap.h \
	c:\vfwdk\inc\msacm.h \
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\shr.h


INFOBOX_DEP = c:\gssi\prog\geomastr\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\geomastr\graphics.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dyndlg.h \
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\tagedit.h


UGRID_DEP = c:\gssi\prog\include\ugtable.h \
	c:\gssi\prog\include\ugrid.h \
	c:\gssi\prog\include\shr.h


UGRIDHLT_DEP = c:\gssi\prog\geomastr\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\geomastr\graphics.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\ugtable.h \
	c:\gssi\prog\include\ugrid.h


STD_DEP = c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\mapl.h


VIDEO_DEP = c:\vfwdk\inc\vfw.h \
	c:\vfwdk\inc\compman.h \
	c:\vfwdk\inc\compddk.h \
	c:\vfwdk\inc\drawdib.h \
	c:\vfwdk\inc\avifmt.h \
	c:\vfwdk\inc\mmreg.h \
	c:\vfwdk\inc\avifile.h \
	c:\vfwdk\inc\aviiface.h \
	c:\vfwdk\inc\mciwnd.h \
	c:\vfwdk\inc\msvideo.h \
	c:\vfwdk\inc\avicap.h \
	c:\vfwdk\inc\msacm.h \
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\geomastr\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\geomastr\graphics.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\mci.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\geospan.h \
	c:\gssi\prog\geomastr\address.h


MCI_DEP = c:\vfwdk\inc\vfw.h \
	c:\vfwdk\inc\compman.h \
	c:\vfwdk\inc\compddk.h \
	c:\vfwdk\inc\drawdib.h \
	c:\vfwdk\inc\avifmt.h \
	c:\vfwdk\inc\mmreg.h \
	c:\vfwdk\inc\avifile.h \
	c:\vfwdk\inc\aviiface.h \
	c:\vfwdk\inc\mciwnd.h \
	c:\vfwdk\inc\msvideo.h \
	c:\vfwdk\inc\avicap.h \
	c:\vfwdk\inc\msacm.h \
	c:\vfwdk\inc\digitalv.h \
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\mci.h


DLL_DEP = c:\gssi\prog\citytour\graphint.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\citytour\graphics.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\citytour\address.h \
	c:\gssi\prog\include\geospan.h \
	c:\gssi\prog\include\mci.h \
	c:\gssi\prog\include\cddemo.h \
	c:\gssi\prog\include\dibapi.h


CONNAD83_DEP = 

LOADPR~1_DEP = c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\proj.h


T2PROJ~1_DEP = c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\proj.h


TRANPR~1_DEP = c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\proj.h


DDE_DEP = c:\gssi\prog\include\ddemlsv.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\huge.h


all:	$(PROJ).EXE $(PROJ).BSC

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

ODBCMAIN.OBJ:	..\SHR\ODBCMAIN.C $(ODBCMAIN_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\SHR\ODBCMAIN.C

GRAPHIC3.OBJ:	GRAPHIC3.C $(GRAPHIC3_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c GRAPHIC3.C

UMIO.OBJ:	..\SHR\UMIO.C $(UMIO_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\SHR\UMIO.C

VARSTUFF.OBJ:	..\SHR\VARSTUFF.C $(VARSTUFF_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\SHR\VARSTUFF.C

GWD.OBJ:	..\DB\GWD.C $(GWD_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\DB\GWD.C

LOADLIBS.OBJ:	LOADLIBS.C $(LOADLIBS_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c LOADLIBS.C

COMPUT1.OBJ:	..\SHR\COMPUT1.C $(COMPUT1_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\SHR\COMPUT1.C

OFFSET.OBJ:	..\OFFSET\OFFSET.C $(OFFSET_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\OFFSET\OFFSET.C

POLY.OBJ:	..\OFFSET\POLY.C $(POLY_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\OFFSET\POLY.C

MS_MAPL.OBJ:	..\SHR\MS_MAPL.C $(MS_MAPL_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\SHR\MS_MAPL.C

HASH.OBJ:	..\SHR\HASH.C $(HASH_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\SHR\HASH.C

GRAPHIC4.OBJ:	GRAPHIC4.C $(GRAPHIC4_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c GRAPHIC4.C

DLGLOOK.OBJ:	..\SHR\DLGLOOK.C $(DLGLOOK_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\SHR\DLGLOOK.C

CVTCOORD.OBJ:	..\SHR\CVTCOORD.C $(CVTCOORD_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\SHR\CVTCOORD.C

GRAPHIC5.OBJ:	GRAPHIC5.C $(GRAPHIC5_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c GRAPHIC5.C

WRITEAVI.OBJ:	..\SHR\WRITEAVI.C $(WRITEAVI_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\SHR\WRITEAVI.C

INFOBOX.OBJ:	INFOBOX.C $(INFOBOX_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c INFOBOX.C

UGRID.OBJ:	..\SHR\UGRID.C $(UGRID_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\SHR\UGRID.C

UGRIDHLT.OBJ:	UGRIDHLT.C $(UGRIDHLT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c UGRIDHLT.C

STD.OBJ:	..\SHR\STD.C $(STD_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\SHR\STD.C

VIDEO.OBJ:	VIDEO.C $(VIDEO_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c VIDEO.C

MCI.OBJ:	MCI.C $(MCI_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c MCI.C

DLL.OBJ:	..\CITYTOUR\DLL.C $(DLL_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\CITYTOUR\DLL.C

CONNAD83.OBJ:	..\SHR\CONNAD83.C $(CONNAD83_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\SHR\CONNAD83.C

LOADPR~1.OBJ:	..\GCTP2\LOADPR~1.C $(LOADPR~1_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTP2\LOADPR~1.C

T2PROJ~1.OBJ:	..\GCTP2\T2PROJ~1.C $(T2PROJ~1_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTP2\T2PROJ~1.C

TRANPR~1.OBJ:	..\GCTP2\TRANPR~1.C $(TRANPR~1_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTP2\TRANPR~1.C

DDE.OBJ:	..\SHR\DDE.C $(DDE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\SHR\DDE.C


$(PROJ).EXE::	GWCONFIG.RES

$(PROJ).EXE::	CWCONFIG.OBJ GRAPHIC1.OBJ GRAPHIC2.OBJ SHR.OBJ BT_CREAT.OBJ COMDLG.OBJ \
	ADDRESS1.OBJ CREATCFG.OBJ ODBCMAIN.OBJ GRAPHIC3.OBJ UMIO.OBJ VARSTUFF.OBJ GWD.OBJ \
	LOADLIBS.OBJ COMPUT1.OBJ OFFSET.OBJ POLY.OBJ MS_MAPL.OBJ HASH.OBJ GRAPHIC4.OBJ DLGLOOK.OBJ \
	CVTCOORD.OBJ GRAPHIC5.OBJ WRITEAVI.OBJ INFOBOX.OBJ UGRID.OBJ UGRIDHLT.OBJ STD.OBJ \
	VIDEO.OBJ MCI.OBJ DLL.OBJ CONNAD83.OBJ LOADPR~1.OBJ T2PROJ~1.OBJ TRANPR~1.OBJ DDE.OBJ $(OBJS_EXT) $(DEFFILE)
	echo >NUL @<<$(PROJ).CRF
CWCONFIG.OBJ +
GRAPHIC1.OBJ +
GRAPHIC2.OBJ +
SHR.OBJ +
BT_CREAT.OBJ +
COMDLG.OBJ +
ADDRESS1.OBJ +
CREATCFG.OBJ +
ODBCMAIN.OBJ +
GRAPHIC3.OBJ +
UMIO.OBJ +
VARSTUFF.OBJ +
GWD.OBJ +
LOADLIBS.OBJ +
COMPUT1.OBJ +
OFFSET.OBJ +
POLY.OBJ +
MS_MAPL.OBJ +
HASH.OBJ +
GRAPHIC4.OBJ +
DLGLOOK.OBJ +
CVTCOORD.OBJ +
GRAPHIC5.OBJ +
WRITEAVI.OBJ +
INFOBOX.OBJ +
UGRID.OBJ +
UGRIDHLT.OBJ +
STD.OBJ +
VIDEO.OBJ +
MCI.OBJ +
DLL.OBJ +
CONNAD83.OBJ +
LOADPR~1.OBJ +
T2PROJ~1.OBJ +
TRANPR~1.OBJ +
DDE.OBJ +
$(OBJS_EXT)
$(PROJ).EXE
$(MAPFILE)
c:\msvc\lib\+
c:\msvc\mfc\lib\+
c:\odbcsdk\lib\+
..\..\..\ODBCSDK\LIB\ODBC.LIB+
..\..\..\MSVC\LIB\WINSOCK.LIB+
..\WINCAP\DIBAPI.LIB+
..\..\..\MSVC\LIB\MMSYSTEM.LIB+
..\..\..\VFWDK\LIB\VFW.LIB+
..\SHR\WINEXEC.LIB+
..\LIB\ULTGRD.LIB+
..\GCTP2\GCTP16.LIB+
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
