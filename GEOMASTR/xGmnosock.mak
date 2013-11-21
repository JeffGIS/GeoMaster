# Microsoft Visual C++ generated build script - Do not modify

PROJ = GMNOSOCK
DEBUG = 1
PROGTYPE = 0
CALLER = 
ARGS = basic1.gmc /WD d:\hcutm
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
CFLAGS_D_WEXE = /nologo /f- /G3 /Zp1 /Zi /ALu /Gt8 /Gx- /Od /D "_DEBUG" /FR /Fd"GWIZ.PDB"
CFLAGS_R_WEXE = /nologo /f- /Gs /G3 /FPc /Zp1 /W3 /ALu /Gt8 /Gx- /Oi /Ol /Ot /Ox /Ob2 /D "NDEBUG" /FR 
LFLAGS_D_WEXE = /NOLOGO /NOD /PACKC:61440 /STACK:20480 /SEG:256 /ALIGN:64 /ONERROR:NOEXE /CO /MAP /LINE  
LFLAGS_R_WEXE = /NOLOGO /NOD /PACKC:61440 /STACK:20480 /SEG:256 /ALIGN:32 /ONERROR:NOEXE /CO /MAP /LINE  
LIBS_D_WEXE = oldnames libw llibcew odbc commdlg.lib ddeml.lib mmsystem.lib shell.lib 
LIBS_R_WEXE = oldnames libw llibcew odbc commdlg.lib ddeml.lib mmsystem.lib shell.lib 
RCFLAGS = /nologo 
RESFLAGS = /nologo /k
RUNFLAGS = 
DEFFILE = 
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
		WINSOCK.SBR \
		LOADLIBS.SBR \
		OFFSET.SBR \
		POLY.SBR \
		COMPUT1.SBR \
		HASH.SBR \
		MS_MAPL.SBR \
		GRAPHIC4.SBR \
		DLGLOOK.SBR \
		CVTCOORD.SBR \
		GRAPHIC5.SBR \
		WRITEAVI.SBR \
		INFOBOX.SBR \
		UGRID.SBR \
		UGRIDHLT.SBR \
		STD.SBR \
		DLL.SBR \
		VIDEO.SBR \
		MCI.SBR \
		LOADPR~1.SBR \
		T2PROJ~1.SBR \
		TRANPR~1.SBR \
		CONNAD83.SBR \
		DDE.SBR \
		PCXTOBMP.SBR \
		DOCUMENT.SBR \
		TRANSLAT.SBR \
		DICT.SBR \
		PRINT.SBR \
		GRAPHIC6.SBR \
		DYNAEDIT.SBR \
		NETWORKS.SBR \
		TRANLAT2.SBR \
		ADDRESS2.SBR \
		ROUTE.SBR \
		CRIME.SBR \
		QUAD.SBR \
		SMOOTH.SBR \
		DIGITIZE.SBR \
		PEOPLNET.SBR \
		THEMES.SBR \
		GRAPHIC7.SBR \
		EXPORT.SBR \
		VISIBLE.SBR \
		REPORT.SBR \
		FLOATAP.SBR \
		CLIDATA.SBR \
		GRAPHIC8.SBR \
		FUNCTION.SBR \
		TRANLAT3.SBR \
		GRAPHIC9.SBR \
		DXFIN.SBR \
		THMMSGP.SBR \
		FISH.SBR


DIBAPI_DEP = 

VFW_DEP = 

MMSYSTEM_DEP = 

WINEXEC_DEP = 

ULTGRD_DEP = 

GCTP16_DEP = 

CWCONFIG_DEP = c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\geomastr\graphics.h \
	c:\gssi\prog\geomastr\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\geomastr\address.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\winexec.h \
	c:\msvc\include\winsock.h \
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\mci.h


GRAPHIC1_DEP = c:\gssi\prog\include\config.h \
	c:\gssi\prog\geomastr\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\geomastr\graphics.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\geomastr\address.h \
	c:\gssi\prog\include\cddemo.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\dyndlg.h \
	c:\gssi\prog\include\aviout.h \
	c:\gssi\prog\include\dibapi.h


GRAPHIC2_DEP = c:\gssi\prog\geomastr\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\geomastr\graphics.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dyndlg.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\geomastr\address.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\translat.h


GWCONFIG_RCDEP = c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\geomastr\address.h \
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
	c:\gssi\prog\citytour\align_bo.bmp \
	c:\gssi\prog\citytour\bmp00012.bmp \
	c:\gssi\prog\citytour\align_ho.bmp \
	c:\gssi\prog\citytour\bmp00013.bmp \
	c:\gssi\prog\citytour\align_le.bmp \
	c:\gssi\prog\citytour\bmp00010.bmp \
	c:\gssi\prog\citytour\align_ri.bmp \
	c:\gssi\prog\citytour\bmp00009.bmp \
	c:\gssi\prog\citytour\align_to.bmp \
	c:\gssi\prog\citytour\bmp00011.bmp \
	c:\gssi\prog\citytour\align_ve.bmp \
	c:\gssi\prog\citytour\bmp00014.bmp \
	c:\gssi\prog\citytour\tucheck.bmp \
	c:\gssi\prog\citytour\tdcheck.bmp \
	c:\gssi\prog\geomastr\tdcombo.bmp \
	c:\gssi\prog\geomastr\tucombo.bmp \
	c:\gssi\prog\geomastr\tdedit.bmp \
	c:\gssi\prog\geomastr\tuedit.bmp \
	c:\gssi\prog\citytour\tdgroup.bmp \
	c:\gssi\prog\citytour\tugroup.bmp \
	c:\gssi\prog\geomastr\hlp-dwn.bmp \
	c:\gssi\prog\geomastr\hlp-up.bmp \
	c:\gssi\prog\citytour\tdframe.bmp \
	c:\gssi\prog\citytour\tuframe.bmp \
	c:\gssi\prog\citytour\tdlist.bmp \
	c:\gssi\prog\citytour\tulist.bmp \
	c:\gssi\prog\citytour\tdicon.bmp \
	c:\gssi\prog\citytour\tuicon.bmp \
	c:\gssi\prog\citytour\tdpointr.bmp \
	c:\gssi\prog\citytour\tupointr.bmp \
	c:\gssi\prog\citytour\tdpush.bmp \
	c:\gssi\prog\citytour\tupush.bmp \
	c:\gssi\prog\citytour\bitmap_r.bmp \
	c:\gssi\prog\citytour\turadio.bmp \
	c:\gssi\prog\citytour\tdrect.bmp \
	c:\gssi\prog\citytour\turect.bmp \
	c:\gssi\prog\citytour\standard.bmp \
	c:\gssi\prog\citytour\bmp00015.bmp \
	c:\gssi\prog\citytour\bmp00016.bmp \
	c:\gssi\prog\citytour\bmp00017.bmp \
	c:\gssi\prog\citytour\static_d.bmp \
	c:\gssi\prog\citytour\static_u.bmp \
	c:\gssi\prog\citytour\numeric_.bmp \
	c:\gssi\prog\citytour\bmp00018.bmp \
	c:\gssi\prog\citytour\pickcurs.bmp \
	c:\gssi\prog\citytour\mono1.bmp \
	c:\gssi\prog\citytour\mono2.bmp \
	c:\gssi\prog\citytour\mono3.bmp \
	c:\gssi\prog\citytour\bmp00019.bmp \
	c:\gssi\prog\citytour\bmp00020.bmp \
	c:\gssi\prog\citytour\mono4.bmp \
	c:\gssi\prog\citytour\mono5.bmp \
	c:\gssi\prog\geomastr\cursor1.cur \
	c:\gssi\prog\geomastr\larrows.cur \
	c:\gssi\prog\geomastr\pick_cur.cur \
	c:\gssi\prog\geomastr\cursor_l.cur \
	c:\gssi\prog\geomastr\cur00001.cur \
	c:\gssi\prog\geomastr\digitize.cur


SHR_DEP = c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\p_tol.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\dibapi.h


BT_CREAT_DEP = c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h


COMDLG_DEP = c:\gssi\prog\include\cddemo.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h


ADDRESS1_DEP = c:\gssi\prog\geomastr\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h \
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
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\geomastr\graphics.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dyndlg.h \
	c:\gssi\prog\geomastr\gw.h


ODBCMAIN_DEP = c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\bt.h


GRAPHIC3_DEP = c:\gssi\prog\geomastr\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\geomastr\graphics.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\offsetmn.h \
	c:\gssi\prog\include\polycom.h


UMIO_DEP = c:\msvc\include\winsock.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h


VARSTUFF_DEP = c:\gssi\prog\geomastr\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\geomastr\graphics.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\dibapi.h


GWD_DEP = c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\gw.h \
	c:\gssi\prog\include\bt.h


WINSOCK_DEP = c:\msvc\include\winsock.h


OFFSET_DEP = c:\gssi\prog\include\shr.h


OFFSET_DEP = c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\offsetmn.h \
	c:\gssi\prog\include\polycom.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\polybt.h \
	c:\gssi\prog\include\p_tol.h \
	c:\gssi\prog\include\offdefs.h


POLY_DEP = c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\polybt.h \
	c:\gssi\prog\include\offsetmn.h \
	c:\gssi\prog\include\polycom.h \
	c:\gssi\prog\include\offdefs.h \
	c:\gssi\prog\include\p_tol.h \
	c:\gssi\prog\include\cnstnt.h


COMPUT1_DEP = c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\include\p_tol.h


HASH_DEP = c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\mapl.h


MS_MAPL_DEP = c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\mapl.h


GRAPHIC4_DEP = c:\gssi\prog\include\config.h \
	c:\gssi\prog\geomastr\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\geomastr\graphics.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\offsetmn.h \
	c:\gssi\prog\include\cddemo.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\mci.h \
	c:\gssi\prog\include\dyndlg.h \
	c:\gssi\prog\include\dibapi.h


DLGLOOK_DEP = c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h


CVTCOORD_DEP = c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h


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
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\geomastr\graphics.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dyndlg.h \
	c:\gssi\prog\geomastr\address.h \
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\p_tol.h


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
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h


INFOBOX_DEP = c:\gssi\prog\geomastr\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\geomastr\graphics.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dyndlg.h \
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\geomastr\gw.h


UGRID_DEP = c:\gssi\prog\include\ugtable.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h


UGRIDHLT_DEP = c:\gssi\prog\geomastr\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\geomastr\graphics.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\ugtable.h


STD_DEP = c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\mapl.h


DLL_DEP = c:\gssi\prog\citytour\graphint.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\citytour\graphics.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\citytour\address.h \
	c:\gssi\prog\include\geospan.h \
	c:\gssi\prog\include\mci.h \
	c:\gssi\prog\include\cddemo.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\dibapi.h


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
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\geomastr\graphics.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\mci.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\geospan.h \
	c:\gssi\prog\geomastr\address.h


MCI_DEP = c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h \
	c:\vfwdk\inc\vfw.h \
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


LOADPR~1_DEP = c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\proj.h


T2PROJ~1_DEP = c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\proj.h


TRANPR~1_DEP = c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\proj.h


CONNAD83_DEP = 

DDE_DEP = c:\gssi\prog\include\ddemlsv.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\huge.h


PCXTOBMP_DEP = c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h


DOCUMENT_DEP = c:\vfwdk\inc\vfw.h \
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
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\geomastr\graphics.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dyndlg.h \
	c:\gssi\prog\geomastr\address.h \
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h


TRANSLAT_DEP = c:\gssi\prog\geomastr\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\geomastr\graphics.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\geomastr\address.h \
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\pnet.h


DICT_DEP = c:\gssi\prog\geomastr\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\geomastr\graphics.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dict.h


PRINT_DEP = c:\gssi\prog\include\config.h \
	c:\gssi\prog\geomastr\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\geomastr\graphics.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\cddemo.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\mci.h \
	c:\gssi\prog\include\dyndlg.h \
	c:\gssi\prog\include\dibapi.h


GRAPHIC6_DEP = c:\gssi\prog\include\config.h \
	c:\gssi\prog\geomastr\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\geomastr\graphics.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\cddemo.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\p_tol.h \
	c:\gssi\prog\include\mci.h \
	c:\gssi\prog\include\dyndlg.h \
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\geomastr\address.h \
	c:\gssi\prog\include\winexec.h


DYNAEDIT_DEP = c:\gssi\prog\geomastr\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\geomastr\graphics.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dyndlg.h


NETWORKS_DEP = c:\vfwdk\inc\vfw.h \
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
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\geomastr\graphics.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dyndlg.h \
	c:\gssi\prog\geomastr\address.h \
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\dict.h


TRANLAT2_DEP = c:\gssi\prog\geomastr\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\geomastr\graphics.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\geomastr\address.h \
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\winexec.h


ADDRESS2_DEP = c:\gssi\prog\geomastr\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\geomastr\graphics.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\geomastr\address.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\std.h


ROUTE_DEP = c:\vfwdk\inc\vfw.h \
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
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\geomastr\graphics.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dyndlg.h \
	c:\gssi\prog\geomastr\address.h \
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\p_tol.h


CRIME_DEP = c:\gssi\prog\include\config.h \
	c:\gssi\prog\geomastr\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\geomastr\graphics.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\cddemo.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\mci.h \
	c:\gssi\prog\include\dyndlg.h \
	c:\gssi\prog\include\dibapi.h


QUAD_DEP = c:\gssi\prog\geomastr\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\geomastr\graphics.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\cddemo.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\dyndlg.h \
	c:\gssi\prog\include\aviout.h \
	c:\gssi\prog\include\dibapi.h


SMOOTH_DEP = c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h


DIGITIZE_DEP = c:\gssi\prog\geomastr\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\geomastr\graphics.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h


PEOPLNET_DEP = c:\gssi\prog\geomastr\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\geomastr\graphics.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\geomastr\address.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\dibapi.h


THEMES_DEP = c:\gssi\prog\include\config.h \
	c:\gssi\prog\geomastr\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\geomastr\graphics.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\cddemo.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\mci.h \
	c:\gssi\prog\include\dyndlg.h \
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\geomastr\address.h


GRAPHIC7_DEP = c:\gssi\prog\include\config.h \
	c:\gssi\prog\geomastr\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\geomastr\graphics.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\cddemo.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\p_tol.h \
	c:\gssi\prog\include\mci.h \
	c:\gssi\prog\include\dyndlg.h \
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\geomastr\address.h \
	c:\gssi\prog\include\winexec.h


EXPORT_DEP = c:\gssi\prog\geomastr\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\geomastr\graphics.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\geomastr\address.h \
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\16two32.h


VISIBLE_DEP = c:\gssi\prog\include\config.h \
	c:\gssi\prog\geomastr\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\geomastr\graphics.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\cddemo.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\dyndlg.h \
	c:\gssi\prog\include\aviout.h \
	c:\gssi\prog\include\dibapi.h


REPORT_DEP = c:\gssi\prog\geomastr\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\geomastr\graphics.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\geomastr\address.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\dibapi.h


FLOATAP_DEP = c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h


CLIDATA_DEP = c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\client.h


GRAPHIC8_DEP = c:\gssi\prog\include\config.h \
	c:\gssi\prog\geomastr\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\geomastr\graphics.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\cddemo.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\p_tol.h \
	c:\gssi\prog\include\mci.h \
	c:\gssi\prog\include\dyndlg.h \
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\geomastr\address.h \
	c:\gssi\prog\include\winexec.h


FUNCTION_DEP = c:\gssi\prog\geomastr\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\geomastr\graphics.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\geomastr\address.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\dibapi.h


TRANLAT3_DEP = c:\gssi\prog\geomastr\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\geomastr\graphics.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\geomastr\address.h \
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\winexec.h


GRAPHIC9_DEP = c:\gssi\prog\include\config.h \
	c:\gssi\prog\geomastr\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\geomastr\graphics.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\cddemo.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\p_tol.h \
	c:\gssi\prog\include\mci.h \
	c:\gssi\prog\include\dyndlg.h \
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\geomastr\address.h \
	c:\gssi\prog\include\winexec.h


DXFIN_DEP = c:\gssi\prog\geomastr\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\geomastr\graphics.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\geomastr\address.h \
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\offsetmn.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\hash.h


THMMSGP_DEP = c:\gssi\prog\include\config.h \
	c:\gssi\prog\geomastr\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\geomastr\graphics.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\cddemo.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\mci.h \
	c:\gssi\prog\include\dyndlg.h \
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\geomastr\address.h


FISH_DEP = c:\gssi\prog\include\config.h \
	c:\gssi\prog\geomastr\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\geomastr\graphics.h \
	c:\gssi\prog\include\wmf.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\cddemo.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\p_tol.h \
	c:\gssi\prog\include\mci.h \
	c:\gssi\prog\include\dyndlg.h \
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\geomastr\address.h \
	c:\gssi\prog\include\winexec.h


all:	$(PROJ).EXE $(PROJ).BSC

.OBJ:	 $(CWCONFIG_DEP)
	$(CC) $(CFLAGS) $(CCREATEPCHFLAG) /c 

.OBJ:	 $(GRAPHIC1_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(GRAPHIC2_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.RES:	 $(GWCONFIG_RCDEP)
	$(RC) $(RCFLAGS) $(RCDEFINES) -r 

.OBJ:	 $(SHR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(BT_CREAT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(COMDLG_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(ADDRESS1_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(CREATCFG_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(ODBCMAIN_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(GRAPHIC3_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(UMIO_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(VARSTUFF_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(GWD_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(WINSOCK_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(LOADLIBS_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(OFFSET_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(POLY_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(COMPUT1_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(HASH_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(MS_MAPL_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(GRAPHIC4_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(DLGLOOK_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(CVTCOORD_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(GRAPHIC5_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(WRITEAVI_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(INFOBOX_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(UGRID_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(UGRIDHLT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(STD_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(DLL_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(VIDEO_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(MCI_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(LOADPR~1_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(T2PROJ~1_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(TRANPR~1_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(CONNAD83_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(DDE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(PCXTOBMP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(DOCUMENT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(TRANSLAT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(DICT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(PRINT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(GRAPHIC6_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(DYNAEDIT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(NETWORKS_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(TRANLAT2_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(ADDRESS2_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(ROUTE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(CRIME_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(QUAD_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(SMOOTH_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(DIGITIZE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(PEOPLNET_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(THEMES_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(GRAPHIC7_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(EXPORT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(VISIBLE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(REPORT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(FLOATAP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(CLIDATA_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(GRAPHIC8_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(FUNCTION_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(TRANLAT3_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(GRAPHIC9_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(DXFIN_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(THMMSGP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(FISH_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 


$(PROJ).EXE::	GWCONFIG.RES

$(PROJ).EXE::	CWCONFIG.OBJ GRAPHIC1.OBJ GRAPHIC2.OBJ SHR.OBJ BT_CREAT.OBJ COMDLG.OBJ \
	ADDRESS1.OBJ CREATCFG.OBJ ODBCMAIN.OBJ GRAPHIC3.OBJ UMIO.OBJ VARSTUFF.OBJ GWD.OBJ \
	WINSOCK.OBJ LOADLIBS.OBJ OFFSET.OBJ POLY.OBJ COMPUT1.OBJ HASH.OBJ MS_MAPL.OBJ GRAPHIC4.OBJ \
	DLGLOOK.OBJ CVTCOORD.OBJ GRAPHIC5.OBJ WRITEAVI.OBJ INFOBOX.OBJ UGRID.OBJ UGRIDHLT.OBJ \
	STD.OBJ DLL.OBJ VIDEO.OBJ MCI.OBJ LOADPR~1.OBJ T2PROJ~1.OBJ TRANPR~1.OBJ CONNAD83.OBJ \
	DDE.OBJ PCXTOBMP.OBJ DOCUMENT.OBJ TRANSLAT.OBJ DICT.OBJ PRINT.OBJ GRAPHIC6.OBJ DYNAEDIT.OBJ \
	NETWORKS.OBJ TRANLAT2.OBJ ADDRESS2.OBJ ROUTE.OBJ CRIME.OBJ QUAD.OBJ SMOOTH.OBJ DIGITIZE.OBJ \
	PEOPLNET.OBJ THEMES.OBJ GRAPHIC7.OBJ EXPORT.OBJ VISIBLE.OBJ REPORT.OBJ FLOATAP.OBJ \
	CLIDATA.OBJ GRAPHIC8.OBJ FUNCTION.OBJ TRANLAT3.OBJ GRAPHIC9.OBJ DXFIN.OBJ THMMSGP.OBJ \
	FISH.OBJ $(OBJS_EXT) $(DEFFILE)
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
WINSOCK.OBJ +
LOADLIBS.OBJ +
OFFSET.OBJ +
POLY.OBJ +
COMPUT1.OBJ +
HASH.OBJ +
MS_MAPL.OBJ +
GRAPHIC4.OBJ +
DLGLOOK.OBJ +
CVTCOORD.OBJ +
GRAPHIC5.OBJ +
WRITEAVI.OBJ +
INFOBOX.OBJ +
UGRID.OBJ +
UGRIDHLT.OBJ +
STD.OBJ +
DLL.OBJ +
VIDEO.OBJ +
MCI.OBJ +
LOADPR~1.OBJ +
T2PROJ~1.OBJ +
TRANPR~1.OBJ +
CONNAD83.OBJ +
DDE.OBJ +
PCXTOBMP.OBJ +
DOCUMENT.OBJ +
TRANSLAT.OBJ +
DICT.OBJ +
PRINT.OBJ +
GRAPHIC6.OBJ +
DYNAEDIT.OBJ +
NETWORKS.OBJ +
TRANLAT2.OBJ +
ADDRESS2.OBJ +
ROUTE.OBJ +
CRIME.OBJ +
QUAD.OBJ +
SMOOTH.OBJ +
DIGITIZE.OBJ +
PEOPLNET.OBJ +
THEMES.OBJ +
GRAPHIC7.OBJ +
EXPORT.OBJ +
VISIBLE.OBJ +
REPORT.OBJ +
FLOATAP.OBJ +
CLIDATA.OBJ +
GRAPHIC8.OBJ +
FUNCTION.OBJ +
TRANLAT3.OBJ +
GRAPHIC9.OBJ +
DXFIN.OBJ +
THMMSGP.OBJ +
FISH.OBJ +
$(OBJS_EXT)
$(PROJ).EXE
$(MAPFILE)
c:\msvc\lib\+
c:\msvc\mfc\lib\+
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
