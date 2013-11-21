# Microsoft Visual C++ generated build script - Do not modify

PROJ = GMNOSOCK
DEBUG = 1
PROGTYPE = 0
CALLER = 
ARGS = lkmaster.gmc /WD c:\lkmaswi
DLLS = 
D_RCDEFINES = /d_DEBUG 
R_RCDEFINES = /dNDEBUG 
ORIGIN = MSVC
ORIGIN_VER = 1.00
PROJPATH = C:\GSSI\PROG\GEOMASTR\
USEMFC = 0
CC = cl
CPP = 
CXX = cl
CCREATEPCHFLAG = 
CPPCREATEPCHFLAG = 
CUSEPCHFLAG = 
CPPUSEPCHFLAG = 
FIRSTC = CWCONFIG.C  
FIRSTCPP =             
RC = 
CFLAGS_D_WEXE = /nologo /f- /G3 /FPc /Zi /AL /Gt0 /Gx- /Od /OV8 /D "_DEBUG" /D "ENABLETRACEx" /D "CHECKMEM" /FR /Zn /Fd"GMNOSOCK.PDB"
CFLAGS_R_WEXE = /nologo /f- /Gs /G3 /FPc /Zd /ALu /Gt0 /Gx- /Ob2 /OV8 /D "NDEBUG" /D "ENABLETRACEx" /D "CHECKMEMx" /FR 
LFLAGS_D_WEXE = /NOLOGO /NOD /NOE /PACKC:61440 /PACKD:65534 /FARCALL /STACK:26368 /SEG:512 /ALIGN:128 /ONERROR:NOEXE /CO /MAP /LINE  
LFLAGS_R_WEXE = /NOLOGO /NOD /PACKC:61440 /STACK:29184 /SEG:1024 /ALIGN:128 /ONERROR:NOEXE /MAP /LINE  
LIBS_D_WEXE = oldnames libw llibcew odbc commdlg.lib ddeml.lib shell.lib toolhelp.lib 
LIBS_R_WEXE = oldnames libw llibcew odbc commdlg.lib ddeml.lib mmsystem.lib shell.lib toolhelp.lib 
RCFLAGS = /nologo 
RESFLAGS = /nologo /k /k
RUNFLAGS = 
DEFFILE = GWCONFIG.DEF
OBJS_EXT = 
LIBS_EXT = ..\..\..\MSVC\LIB\MMSYSTEM.LIB ..\SHR\WINEXEC.LIB ..\LIB\ULTGRD.LIB ..\LIB\CRYPTO16.LIB \
	..\..\..\MSVC\LIB\WINSOCK.LIB ..\..\..\VFWDK\LIB\VFW.LIB 
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
		FISH.SBR \
		GPS.SBR \
		EDGMATCH.SBR \
		GRAPHICA.SBR \
		TRAVERSE.SBR \
		DTM.SBR \
		TIFF.SBR \
		ORTHOS.SBR \
		FUNCSUBS.SBR \
		CONVERT.SBR \
		FUNCTN2.SBR \
		MAPCOPY.SBR \
		GRFUNC.SBR \
		GRAPHICB.SBR \
		CDSTUFF.SBR \
		TRAN.SBR \
		CMDTONUM.SBR \
		GCTP.SBR \
		FOR_INIT.SBR \
		INV_INIT.SBR \
		UNTFZ.SBR \
		ALBERFOR.SBR \
		ALBERINV.SBR \
		ALCONFOR.SBR \
		ALCONINV.SBR \
		AZIMFOR.SBR \
		AZIMINV.SBR \
		BR_GCTP.SBR \
		CPROJ.SBR \
		EQCONINV.SBR \
		EQUIFOR.SBR \
		EQUIINV.SBR \
		GNOMFOR.SBR \
		GNOMINV.SBR \
		GOODFOR.SBR \
		GOODINV.SBR \
		GVNSPFOR.SBR \
		GVNSPINV.SBR \
		HAMFOR.SBR \
		HAMINV.SBR \
		IMOLWFOR.SBR \
		IMOLWINV.SBR \
		LAMAZFOR.SBR \
		LAMAZINV.SBR \
		LAMCCFOR.SBR \
		LAMCCINV.SBR \
		MERFOR.SBR \
		MERINV.SBR \
		MILLFOR.SBR \
		MILLINV.SBR \
		MOLWFOR.SBR \
		MOLWINV.SBR \
		OBLEQFOR.SBR \
		OBLEQINV.SBR \
		OMERFOR.SBR \
		OMERINV.SBR \
		ORTHFOR.SBR \
		ORTHINV.SBR \
		PAKSZ.SBR \
		POLYFOR.SBR \
		POLYINV.SBR \
		PSFOR.SBR \
		PSINV.SBR \
		ROBFOR.SBR \
		ROBINV.SBR \
		SINFOR.SBR \
		SININV.SBR \
		SOMFOR.SBR \
		SOMINV.SBR \
		SPHDZ.SBR \
		STERFOR.SBR \
		STERINV.SBR \
		STPLNFOR.SBR \
		STPLNINV.SBR \
		TMFOR.SBR \
		TMINV.SBR \
		UTMFOR.SBR \
		UTMINV.SBR \
		VANDGFOR.SBR \
		VANDGINV.SBR \
		WIVFOR.SBR \
		WVIIFOR.SBR \
		WVIIINV.SBR \
		REPORTGC.SBR \
		EQCONFOR.SBR \
		WIVINV.SBR \
		BIGMEM.SBR \
		DIBUTIL.SBR \
		FILE.SBR \
		COPYDIB.SBR \
		VIEWPORT.SBR \
		GSPLINE.SBR \
		SSP.SBR \
		MEMALLOC.SBR \
		TEMP3.SBR \
		THEMES2.SBR \
		HOTSPOTS.SBR \
		VEHICLE.SBR \
		DBFOPEN.SBR \
		ADDRESS3.SBR \
		EXPORT2.SBR \
		FUNDEFS.SBR \
		SHPFILE.SBR \
		ORAFILE.SBR \
		GM32LIB.SBR \
		DGNFILE.SBR \
		GRAPHICC.SBR \
		VARSTUF2.SBR \
		FUNCTN3.SBR \
		BCI.SBR \
		TDSPL.SBR \
		TOOLS.SBR \
		UNDO.SBR \
		VARDEF.SBR \
		DICTEDIT.SBR \
		ROADNAME.SBR \
		SOLAR.SBR \
		VOTERS.SBR \
		MAKEAREA.SBR \
		THEMES3.SBR \
		FUNCTN1.SBR \
		GRAPHICD.SBR \
		DTM2.SBR \
		PROFILE.SBR \
		CONVRT32.SBR


MMSYSTEM_DEP = 

WINEXEC_DEP = 

ULTGRD_DEP = 

CRYPTO16_DEP = 

WINSOCK_DEP = 

VFW_DEP = 

CWCONFIG_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\crypto16.h \
	c:\gssi\prog\include\gmextern.h


GRAPHIC1_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\gmextern.h


GRAPHIC2_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\gmextern.h


GWCONFIG_RCDEP = c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\cddemo.h \
	c:\gssi\prog\geomastr\earth.ico \
	c:\gssi\prog\geomastr\icon1.ico \
	c:\gssi\prog\geomastr\names.ico \
	c:\gssi\prog\geomastr\home.ico \
	c:\gssi\prog\geomastr\assoc.ico \
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
	c:\gssi\prog\citytour\mono4.bmp \
	c:\gssi\prog\citytour\mono5.bmp \
	c:\gssi\prog\citytour\label.bmp \
	c:\gssi\prog\citytour\symbol.bmp \
	c:\gssi\prog\citytour\toolbar1.bmp \
	c:\gssi\prog\citytour\bitmap9.bmp \
	c:\gssi\prog\citytour\s_temp.bmp \
	c:\gssi\prog\citytour\sportmap.bmp \
	c:\gssi\prog\citytour\arrowq~1.bmp \
	c:\gssi\prog\citytour\bitmap11.bmp \
	c:\gssi\prog\citytour\mono6.bmp \
	c:\gssi\prog\citytour\mono7.bmp \
	c:\gssi\prog\citytour\phone1.bmp \
	c:\gssi\prog\citytour\bitmap13.bmp \
	c:\gssi\prog\citytour\bitmap14.bmp \
	c:\gssi\prog\citytour\bitmap15.bmp \
	c:\gssi\prog\citytour\bitmap16.bmp \
	c:\gssi\prog\citytour\gmlogow.bmp \
	c:\gssi\prog\geomastr\larrows.cur \
	c:\gssi\prog\geomastr\pick_cur.cur \
	c:\gssi\prog\geomastr\cursor_l.cur \
	c:\gssi\prog\geomastr\cur00001.cur \
	c:\gssi\prog\geomastr\digitize.cur \
	c:\gssi\prog\geomastr\label_cu.cur \
	c:\gssi\prog\geomastr\symbol_c.cur \
	c:\gssi\prog\geomastr\measure_.cur \
	c:\gssi\prog\geomastr\waypoint.cur \
	c:\gssi\prog\geomastr\cur00002.cur \
	c:\gssi\prog\geomastr\zoomin.cur \
	c:\gssi\prog\geomastr\hmove.cur \
	c:\gssi\prog\geomastr\handmove.cur \
	c:\gssi\prog\geomastr\show.cur \
	c:\gssi\prog\geomastr\drawing.cur \
	c:\gssi\prog\geomastr\togglevi.cur \
	c:\gssi\prog\geomastr\settag.cur


SHR_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\gmextern.h


BT_CREAT_DEP = c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gmextern.h


COMDLG_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\cddemo.h \
	c:\gssi\prog\include\gmextern.h


ADDRESS1_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\gmextern.h


CREATCFG_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\gmextern.h


ODBCMAIN_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\shapefil.h \
	c:\gssi\prog\include\gmextern.h


GRAPHIC3_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\gmextern.h


UMIO_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\gmextern.h


VARSTUFF_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\shapefil.h \
	c:\gssi\prog\include\gmextern.h


GWD_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\shapefil.h \
	c:\gssi\prog\include\gmextern.h


OFFSET_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\gmextern.h \
	c:\gssi\prog\include\offsetmn.h \
	c:\gssi\prog\include\polycom.h \
	c:\gssi\prog\include\offdefs.h \
	c:\gssi\prog\include\polybt.h


POLY_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\gmextern.h \
	c:\gssi\prog\include\offsetmn.h \
	c:\gssi\prog\include\polycom.h \
	c:\gssi\prog\include\offdefs.h \
	c:\gssi\prog\include\polybt.h \
	c:\gssi\prog\include\cnstnt.h


COMPUT1_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\gmextern.h \
	c:\gssi\prog\include\xll.h


HASH_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h


MS_MAPL_DEP = c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h


GRAPHIC4_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\gmextern.h


DLGLOOK_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\gmextern.h


CVTCOORD_DEP = c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\p_tol.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\gmextern.h


GRAPHIC5_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\gmextern.h


WRITEAVI_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\vfwdk\inc\compman.h \
	c:\vfwdk\inc\compddk.h \
	c:\vfwdk\inc\vfw.h \
	c:\vfwdk\inc\drawdib.h \
	c:\vfwdk\inc\avifmt.h \
	c:\vfwdk\inc\mmreg.h \
	c:\vfwdk\inc\avifile.h \
	c:\vfwdk\inc\aviiface.h \
	c:\vfwdk\inc\mciwnd.h \
	c:\vfwdk\inc\msvideo.h \
	c:\vfwdk\inc\avicap.h \
	c:\vfwdk\inc\msacm.h \
	c:\gssi\prog\include\gmextern.h


INFOBOX_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\gmextern.h


UGRID_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\ugtable.h \
	c:\gssi\prog\include\gmextern.h


UGRIDHLT_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\ugtable.h \
	c:\gssi\prog\include\gmextern.h


STD_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\gmextern.h


T2PROJ~1_DEP = c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\gmextern.h \
	c:\gssi\prog\gctpc\proj.h


TRANPR~1_DEP = c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\proj.h


CONNAD83_DEP = c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h


DDE_DEP = c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\ddemlsv.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\huge.h \
	c:\gssi\prog\include\gmextern.h


PCXTOBMP_DEP = c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h


DOCUMENT_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\gmextern.h


TRANSLAT_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\gmextern.h


DICT_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\gmextern.h


PRINT_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\gmextern.h \
	c:\gssi\prog\include\cddemo.h


GRAPHIC6_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\gmextern.h


DYNAEDIT_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dyndlg.h \
	c:\gssi\prog\include\gmextern.h


NETWORKS_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\gmextern.h


TRANLAT2_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\gmextern.h \
	c:\gssi\prog\include\dibapi.h


ADDRESS2_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\gmextern.h


ROUTE_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\gmextern.h


QUAD_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\gmextern.h


SMOOTH_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h


DIGITIZE_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\gmextern.h


PEOPLNET_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\gmextern.h


THEMES_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\gmextern.h


GRAPHIC7_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\gmextern.h


EXPORT_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\shapefil.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\gmextern.h


VISIBLE_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\shapefil.h \
	c:\gssi\prog\include\gmextern.h


REPORT_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\gmextern.h


FLOATAP_DEP = c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gmextern.h


CLIDATA_DEP = c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


GRAPHIC8_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\gmextern.h


FUNCTION_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


TRANLAT3_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\gmextern.h


GRAPHIC9_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\gmextern.h


DXFIN_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\gmextern.h


THMMSGP_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\gmextern.h


FISH_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\gmextern.h


GPS_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\gps\garmin.h \
	c:\gssi\prog\include\gmextern.h


EDGMATCH_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\gmextern.h


GRAPHICA_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\gmextern.h


TRAVERSE_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\gmextern.h


DTM_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\bci.h \
	c:\gssi\prog\include\gmextern.h


TIFF_DEP = c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h


ORTHOS_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\gmextern.h \
	c:\gssi\prog\include\dibapi.h


FUNCSUBS_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\gmextern.h


CONVERT_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h


FUNCTN2_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\gmextern.h


MAPCOPY_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\gmextern.h


GRFUNC_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\gmextern.h


GRAPHICB_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\gmextern.h


CDSTUFF_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\gmextern.h


TRAN_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\gmextern.h


CMDTONUM_DEP = c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h


GCTP_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


FOR_INIT_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


INV_INIT_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


UNTFZ_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


ALBERFOR_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


ALBERINV_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


ALCONFOR_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


ALCONINV_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


AZIMFOR_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


AZIMINV_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


BR_GCTP_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


CPROJ_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


EQCONINV_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


EQUIFOR_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


EQUIINV_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


GNOMFOR_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


GNOMINV_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


GOODFOR_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


GOODINV_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


GVNSPFOR_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


GVNSPINV_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


HAMFOR_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


HAMINV_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


IMOLWFOR_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


IMOLWINV_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


LAMAZFOR_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


LAMAZINV_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


LAMCCFOR_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


LAMCCINV_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


MERFOR_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


MERINV_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


MILLFOR_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


MILLINV_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


MOLWFOR_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


MOLWINV_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


OBLEQFOR_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


OBLEQINV_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


OMERFOR_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


OMERINV_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


ORTHFOR_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


ORTHINV_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


PAKSZ_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


POLYFOR_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


POLYINV_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


PSFOR_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


PSINV_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


ROBFOR_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


ROBINV_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


SINFOR_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


SININV_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


SOMFOR_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


SOMINV_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


SPHDZ_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


STERFOR_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


STERINV_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


STPLNFOR_DEP = c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


STPLNINV_DEP = c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


TMFOR_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


TMINV_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


UTMFOR_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


UTMINV_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


VANDGFOR_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


VANDGINV_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


WIVFOR_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


WVIIFOR_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


WVIIINV_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


REPORTGC_DEP = c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


EQCONFOR_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


WIVINV_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


BIGMEM_DEP = c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\gmvardef.h


DIBUTIL_DEP = c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\dibutil.h \
	c:\gssi\prog\include\gmextern.h


FILE_DEP = c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\dibutil.h \
	c:\gssi\prog\include\dibapi.h


COPYDIB_DEP = c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\dibutil.h \
	c:\gssi\prog\include\dibapi.h


VIEWPORT_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\gmextern.h


GSPLINE_DEP = c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h


SSP_DEP = c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h


TEMP3_DEP = 

THEMES2_DEP = c:\gssi\prog\include\graphint.h


THEMES2_DEP = c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\gmextern.h


HOTSPOTS_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\gmextern.h


VEHICLE_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\gmextern.h


DBFOPEN_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\shapefil.h


ADDRESS3_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\gmextern.h


EXPORT2_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\gmextern.h


FUNDEFS_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\gmextern.h


SHPFILE_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\shapefil.h \
	c:\gssi\prog\include\gmextern.h


ORAFILE_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\gmextern.h


GM32LIB_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\gmextern.h \
	c:\gssi\prog\include\wownt16.h


DGNFILE_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\gmextern.h


GRAPHICC_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\gmextern.h


VARSTUF2_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\gmextern.h


FUNCTN3_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\gmextern.h


BCI_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\bci.h


TDSPL_DEP = c:\gssi\prog\include\bci.h


TOOLS_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\bci.h


UNDO_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\gmextern.h


VARDEF_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h


DICTEDIT_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\gmextern.h


ROADNAME_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\gmextern.h


SOLAR_DEP = 

VOTERS_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\gmextern.h


MAKEAREA_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\gmextern.h


THEMES3_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\gmextern.h


FUNCTN1_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


GRAPHICD_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\gmextern.h


DTM2_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\gmextern.h


PROFILE_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\gmextern.h


CONVRT32_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\16two32.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h \
	c:\gssi\prog\include\graphics.h \
	c:\gssi\prog\include\dict.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\geomastr\gw.h


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

OFFSET.OBJ:	..\OFFSET\OFFSET.C $(OFFSET_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\OFFSET\OFFSET.C

POLY.OBJ:	..\OFFSET\POLY.C $(POLY_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\OFFSET\POLY.C

COMPUT1.OBJ:	..\SHR\COMPUT1.C $(COMPUT1_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\SHR\COMPUT1.C

HASH.OBJ:	..\SHR\HASH.C $(HASH_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\SHR\HASH.C

MS_MAPL.OBJ:	..\SHR\MS_MAPL.C $(MS_MAPL_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\SHR\MS_MAPL.C

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

T2PROJ~1.OBJ:	..\GCTP2\T2PROJ~1.C $(T2PROJ~1_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTP2\T2PROJ~1.C

TRANPR~1.OBJ:	..\GCTP2\TRANPR~1.C $(TRANPR~1_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTP2\TRANPR~1.C

CONNAD83.OBJ:	..\SHR\CONNAD83.C $(CONNAD83_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\SHR\CONNAD83.C

DDE.OBJ:	..\SHR\DDE.C $(DDE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\SHR\DDE.C

PCXTOBMP.OBJ:	..\PCX\PCXTOBMP.C $(PCXTOBMP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\PCX\PCXTOBMP.C

DOCUMENT.OBJ:	DOCUMENT.C $(DOCUMENT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c DOCUMENT.C

TRANSLAT.OBJ:	TRANSLAT.C $(TRANSLAT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c TRANSLAT.C

DICT.OBJ:	DICT.C $(DICT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c DICT.C

PRINT.OBJ:	PRINT.C $(PRINT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c PRINT.C

GRAPHIC6.OBJ:	GRAPHIC6.C $(GRAPHIC6_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c GRAPHIC6.C

DYNAEDIT.OBJ:	DYNAEDIT.C $(DYNAEDIT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c DYNAEDIT.C

NETWORKS.OBJ:	NETWORKS.C $(NETWORKS_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c NETWORKS.C

TRANLAT2.OBJ:	TRANLAT2.C $(TRANLAT2_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c TRANLAT2.C

ADDRESS2.OBJ:	ADDRESS2.C $(ADDRESS2_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ADDRESS2.C

ROUTE.OBJ:	ROUTE.C $(ROUTE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ROUTE.C

QUAD.OBJ:	QUAD.C $(QUAD_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c QUAD.C

SMOOTH.OBJ:	SMOOTH.C $(SMOOTH_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c SMOOTH.C

DIGITIZE.OBJ:	DIGITIZE.C $(DIGITIZE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c DIGITIZE.C

PEOPLNET.OBJ:	PEOPLNET.C $(PEOPLNET_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c PEOPLNET.C

THEMES.OBJ:	THEMES.C $(THEMES_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c THEMES.C

GRAPHIC7.OBJ:	GRAPHIC7.C $(GRAPHIC7_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c GRAPHIC7.C

EXPORT.OBJ:	EXPORT.C $(EXPORT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c EXPORT.C

VISIBLE.OBJ:	VISIBLE.C $(VISIBLE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c VISIBLE.C

REPORT.OBJ:	REPORT.C $(REPORT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c REPORT.C

FLOATAP.OBJ:	..\SHR\FLOATAP.C $(FLOATAP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\SHR\FLOATAP.C

CLIDATA.OBJ:	..\SHR\CLIDATA.C $(CLIDATA_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\SHR\CLIDATA.C

GRAPHIC8.OBJ:	GRAPHIC8.C $(GRAPHIC8_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c GRAPHIC8.C

FUNCTION.OBJ:	FUNCTION.C $(FUNCTION_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c FUNCTION.C

TRANLAT3.OBJ:	TRANLAT3.C $(TRANLAT3_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c TRANLAT3.C

GRAPHIC9.OBJ:	GRAPHIC9.C $(GRAPHIC9_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c GRAPHIC9.C

DXFIN.OBJ:	DXFIN.C $(DXFIN_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c DXFIN.C

THMMSGP.OBJ:	THMMSGP.C $(THMMSGP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c THMMSGP.C

FISH.OBJ:	FISH.C $(FISH_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c FISH.C

GPS.OBJ:	..\GPS\GPS.C $(GPS_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GPS\GPS.C

EDGMATCH.OBJ:	EDGMATCH.C $(EDGMATCH_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c EDGMATCH.C

GRAPHICA.OBJ:	GRAPHICA.C $(GRAPHICA_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c GRAPHICA.C

TRAVERSE.OBJ:	TRAVERSE.C $(TRAVERSE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c TRAVERSE.C

DTM.OBJ:	DTM.C $(DTM_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c DTM.C

TIFF.OBJ:	..\SHR\TIFF.C $(TIFF_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\SHR\TIFF.C

ORTHOS.OBJ:	ORTHOS.C $(ORTHOS_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ORTHOS.C

FUNCSUBS.OBJ:	FUNCSUBS.C $(FUNCSUBS_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c FUNCSUBS.C

CONVERT.OBJ:	CONVERT.C $(CONVERT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c CONVERT.C

FUNCTN2.OBJ:	FUNCTN2.C $(FUNCTN2_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c FUNCTN2.C

MAPCOPY.OBJ:	MAPCOPY.C $(MAPCOPY_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c MAPCOPY.C

GRFUNC.OBJ:	GRFUNC.C $(GRFUNC_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c GRFUNC.C

GRAPHICB.OBJ:	GRAPHICB.C $(GRAPHICB_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c GRAPHICB.C

CDSTUFF.OBJ:	CDSTUFF.C $(CDSTUFF_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c CDSTUFF.C

TRAN.OBJ:	TRAN.C $(TRAN_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c TRAN.C

CMDTONUM.OBJ:	CMDTONUM.C $(CMDTONUM_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c CMDTONUM.C

GCTP.OBJ:	..\GCTPC\GCTP.C $(GCTP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\GCTP.C

FOR_INIT.OBJ:	..\GCTPC\FOR_INIT.C $(FOR_INIT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\FOR_INIT.C

INV_INIT.OBJ:	..\GCTPC\INV_INIT.C $(INV_INIT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\INV_INIT.C

UNTFZ.OBJ:	..\GCTPC\UNTFZ.C $(UNTFZ_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\UNTFZ.C

ALBERFOR.OBJ:	..\GCTPC\ALBERFOR.C $(ALBERFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\ALBERFOR.C

ALBERINV.OBJ:	..\GCTPC\ALBERINV.C $(ALBERINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\ALBERINV.C

ALCONFOR.OBJ:	..\GCTPC\ALCONFOR.C $(ALCONFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\ALCONFOR.C

ALCONINV.OBJ:	..\GCTPC\ALCONINV.C $(ALCONINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\ALCONINV.C

AZIMFOR.OBJ:	..\GCTPC\AZIMFOR.C $(AZIMFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\AZIMFOR.C

AZIMINV.OBJ:	..\GCTPC\AZIMINV.C $(AZIMINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\AZIMINV.C

BR_GCTP.OBJ:	..\GCTPC\BR_GCTP.C $(BR_GCTP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\BR_GCTP.C

CPROJ.OBJ:	..\GCTPC\CPROJ.C $(CPROJ_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\CPROJ.C

EQCONINV.OBJ:	..\GCTPC\EQCONINV.C $(EQCONINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\EQCONINV.C

EQUIFOR.OBJ:	..\GCTPC\EQUIFOR.C $(EQUIFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\EQUIFOR.C

EQUIINV.OBJ:	..\GCTPC\EQUIINV.C $(EQUIINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\EQUIINV.C

GNOMFOR.OBJ:	..\GCTPC\GNOMFOR.C $(GNOMFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\GNOMFOR.C

GNOMINV.OBJ:	..\GCTPC\GNOMINV.C $(GNOMINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\GNOMINV.C

GOODFOR.OBJ:	..\GCTPC\GOODFOR.C $(GOODFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\GOODFOR.C

GOODINV.OBJ:	..\GCTPC\GOODINV.C $(GOODINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\GOODINV.C

GVNSPFOR.OBJ:	..\GCTPC\GVNSPFOR.C $(GVNSPFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\GVNSPFOR.C

GVNSPINV.OBJ:	..\GCTPC\GVNSPINV.C $(GVNSPINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\GVNSPINV.C

HAMFOR.OBJ:	..\GCTPC\HAMFOR.C $(HAMFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\HAMFOR.C

HAMINV.OBJ:	..\GCTPC\HAMINV.C $(HAMINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\HAMINV.C

IMOLWFOR.OBJ:	..\GCTPC\IMOLWFOR.C $(IMOLWFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\IMOLWFOR.C

IMOLWINV.OBJ:	..\GCTPC\IMOLWINV.C $(IMOLWINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\IMOLWINV.C

LAMAZFOR.OBJ:	..\GCTPC\LAMAZFOR.C $(LAMAZFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\LAMAZFOR.C

LAMAZINV.OBJ:	..\GCTPC\LAMAZINV.C $(LAMAZINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\LAMAZINV.C

LAMCCFOR.OBJ:	..\GCTPC\LAMCCFOR.C $(LAMCCFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\LAMCCFOR.C

LAMCCINV.OBJ:	..\GCTPC\LAMCCINV.C $(LAMCCINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\LAMCCINV.C

MERFOR.OBJ:	..\GCTPC\MERFOR.C $(MERFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\MERFOR.C

MERINV.OBJ:	..\GCTPC\MERINV.C $(MERINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\MERINV.C

MILLFOR.OBJ:	..\GCTPC\MILLFOR.C $(MILLFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\MILLFOR.C

MILLINV.OBJ:	..\GCTPC\MILLINV.C $(MILLINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\MILLINV.C

MOLWFOR.OBJ:	..\GCTPC\MOLWFOR.C $(MOLWFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\MOLWFOR.C

MOLWINV.OBJ:	..\GCTPC\MOLWINV.C $(MOLWINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\MOLWINV.C

OBLEQFOR.OBJ:	..\GCTPC\OBLEQFOR.C $(OBLEQFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\OBLEQFOR.C

OBLEQINV.OBJ:	..\GCTPC\OBLEQINV.C $(OBLEQINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\OBLEQINV.C

OMERFOR.OBJ:	..\GCTPC\OMERFOR.C $(OMERFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\OMERFOR.C

OMERINV.OBJ:	..\GCTPC\OMERINV.C $(OMERINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\OMERINV.C

ORTHFOR.OBJ:	..\GCTPC\ORTHFOR.C $(ORTHFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\ORTHFOR.C

ORTHINV.OBJ:	..\GCTPC\ORTHINV.C $(ORTHINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\ORTHINV.C

PAKSZ.OBJ:	..\GCTPC\PAKSZ.C $(PAKSZ_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\PAKSZ.C

POLYFOR.OBJ:	..\GCTPC\POLYFOR.C $(POLYFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\POLYFOR.C

POLYINV.OBJ:	..\GCTPC\POLYINV.C $(POLYINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\POLYINV.C

PSFOR.OBJ:	..\GCTPC\PSFOR.C $(PSFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\PSFOR.C

PSINV.OBJ:	..\GCTPC\PSINV.C $(PSINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\PSINV.C

ROBFOR.OBJ:	..\GCTPC\ROBFOR.C $(ROBFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\ROBFOR.C

ROBINV.OBJ:	..\GCTPC\ROBINV.C $(ROBINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\ROBINV.C

SINFOR.OBJ:	..\GCTPC\SINFOR.C $(SINFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\SINFOR.C

SININV.OBJ:	..\GCTPC\SININV.C $(SININV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\SININV.C

SOMFOR.OBJ:	..\GCTPC\SOMFOR.C $(SOMFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\SOMFOR.C

SOMINV.OBJ:	..\GCTPC\SOMINV.C $(SOMINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\SOMINV.C

SPHDZ.OBJ:	..\GCTPC\SPHDZ.C $(SPHDZ_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\SPHDZ.C

STERFOR.OBJ:	..\GCTPC\STERFOR.C $(STERFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\STERFOR.C

STERINV.OBJ:	..\GCTPC\STERINV.C $(STERINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\STERINV.C

STPLNFOR.OBJ:	..\GCTPC\STPLNFOR.C $(STPLNFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\STPLNFOR.C

STPLNINV.OBJ:	..\GCTPC\STPLNINV.C $(STPLNINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\STPLNINV.C

TMFOR.OBJ:	..\GCTPC\TMFOR.C $(TMFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\TMFOR.C

TMINV.OBJ:	..\GCTPC\TMINV.C $(TMINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\TMINV.C

UTMFOR.OBJ:	..\GCTPC\UTMFOR.C $(UTMFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\UTMFOR.C

UTMINV.OBJ:	..\GCTPC\UTMINV.C $(UTMINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\UTMINV.C

VANDGFOR.OBJ:	..\GCTPC\VANDGFOR.C $(VANDGFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\VANDGFOR.C

VANDGINV.OBJ:	..\GCTPC\VANDGINV.C $(VANDGINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\VANDGINV.C

WIVFOR.OBJ:	..\GCTPC\WIVFOR.C $(WIVFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\WIVFOR.C

WVIIFOR.OBJ:	..\GCTPC\WVIIFOR.C $(WVIIFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\WVIIFOR.C

WVIIINV.OBJ:	..\GCTPC\WVIIINV.C $(WVIIINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\WVIIINV.C

REPORTGC.OBJ:	..\GCTPC\REPORTGC.C $(REPORTGC_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\REPORTGC.C

EQCONFOR.OBJ:	..\GCTPC\EQCONFOR.C $(EQCONFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\EQCONFOR.C

WIVINV.OBJ:	..\GCTPC\WIVINV.C $(WIVINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\GCTPC\WIVINV.C

BIGMEM.OBJ:	BIGMEM.C $(BIGMEM_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c BIGMEM.C

DIBUTIL.OBJ:	..\SHR\DIBUTIL.C $(DIBUTIL_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\SHR\DIBUTIL.C

FILE.OBJ:	..\SHR\FILE.C $(FILE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\SHR\FILE.C

COPYDIB.OBJ:	..\SHR\COPYDIB.C $(COPYDIB_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\SHR\COPYDIB.C

VIEWPORT.OBJ:	VIEWPORT.C $(VIEWPORT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c VIEWPORT.C

GSPLINE.OBJ:	..\SPLINES\GSPLINE.C $(GSPLINE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\SPLINES\GSPLINE.C

SSP.OBJ:	..\SPLINES\SSP.C $(SSP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\SPLINES\SSP.C

MEMALLOC.OBJ:	..\SHR\MEMALLOC.C $(MEMALLOC_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\SHR\MEMALLOC.C

TEMP3.OBJ:	TEMP3.C $(TEMP3_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c TEMP3.C

THEMES2.OBJ:	THEMES2.C $(THEMES2_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c THEMES2.C

HOTSPOTS.OBJ:	HOTSPOTS.C $(HOTSPOTS_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c HOTSPOTS.C

VEHICLE.OBJ:	VEHICLE.C $(VEHICLE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c VEHICLE.C

DBFOPEN.OBJ:	..\SHAPELIB\CODE\DBFOPEN.C $(DBFOPEN_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\SHAPELIB\CODE\DBFOPEN.C

ADDRESS3.OBJ:	ADDRESS3.C $(ADDRESS3_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ADDRESS3.C

EXPORT2.OBJ:	EXPORT2.C $(EXPORT2_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c EXPORT2.C

FUNDEFS.OBJ:	FUNDEFS.C $(FUNDEFS_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c FUNDEFS.C

SHPFILE.OBJ:	SHPFILE.C $(SHPFILE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c SHPFILE.C

ORAFILE.OBJ:	ORAFILE.C $(ORAFILE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ORAFILE.C

GM32LIB.OBJ:	GM32LIB.C $(GM32LIB_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c GM32LIB.C

DGNFILE.OBJ:	DGNFILE.C $(DGNFILE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c DGNFILE.C

GRAPHICC.OBJ:	GRAPHICC.C $(GRAPHICC_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c GRAPHICC.C

VARSTUF2.OBJ:	..\SHR\VARSTUF2.C $(VARSTUF2_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\SHR\VARSTUF2.C

FUNCTN3.OBJ:	FUNCTN3.C $(FUNCTN3_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c FUNCTN3.C

BCI.OBJ:	..\LIBBCI~1.0\BCI.C $(BCI_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\LIBBCI~1.0\BCI.C

TDSPL.OBJ:	..\LIBBCI~1.0\TDSPL.C $(TDSPL_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\LIBBCI~1.0\TDSPL.C

TOOLS.OBJ:	..\LIBBCI~1.0\TOOLS.C $(TOOLS_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\LIBBCI~1.0\TOOLS.C

UNDO.OBJ:	..\SHR\UNDO.C $(UNDO_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ..\SHR\UNDO.C

VARDEF.OBJ:	VARDEF.C $(VARDEF_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c VARDEF.C

DICTEDIT.OBJ:	DICTEDIT.C $(DICTEDIT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c DICTEDIT.C

ROADNAME.OBJ:	ROADNAME.C $(ROADNAME_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c ROADNAME.C

SOLAR.OBJ:	SOLAR.C $(SOLAR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c SOLAR.C

VOTERS.OBJ:	VOTERS.C $(VOTERS_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c VOTERS.C

MAKEAREA.OBJ:	MAKEAREA.C $(MAKEAREA_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c MAKEAREA.C

THEMES3.OBJ:	THEMES3.C $(THEMES3_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c THEMES3.C

FUNCTN1.OBJ:	FUNCTN1.C $(FUNCTN1_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c FUNCTN1.C

GRAPHICD.OBJ:	GRAPHICD.C $(GRAPHICD_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c GRAPHICD.C

DTM2.OBJ:	DTM2.C $(DTM2_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c DTM2.C

PROFILE.OBJ:	PROFILE.C $(PROFILE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c PROFILE.C

CONVRT32.OBJ:	CONVRT32.C $(CONVRT32_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c CONVRT32.C


$(PROJ).EXE::	GWCONFIG.RES

$(PROJ).EXE::	CWCONFIG.OBJ GRAPHIC1.OBJ GRAPHIC2.OBJ SHR.OBJ BT_CREAT.OBJ COMDLG.OBJ \
	ADDRESS1.OBJ CREATCFG.OBJ ODBCMAIN.OBJ GRAPHIC3.OBJ UMIO.OBJ VARSTUFF.OBJ GWD.OBJ \
	OFFSET.OBJ POLY.OBJ COMPUT1.OBJ HASH.OBJ MS_MAPL.OBJ GRAPHIC4.OBJ DLGLOOK.OBJ CVTCOORD.OBJ \
	GRAPHIC5.OBJ WRITEAVI.OBJ INFOBOX.OBJ UGRID.OBJ UGRIDHLT.OBJ STD.OBJ T2PROJ~1.OBJ \
	TRANPR~1.OBJ CONNAD83.OBJ DDE.OBJ PCXTOBMP.OBJ DOCUMENT.OBJ TRANSLAT.OBJ DICT.OBJ \
	PRINT.OBJ GRAPHIC6.OBJ DYNAEDIT.OBJ NETWORKS.OBJ TRANLAT2.OBJ ADDRESS2.OBJ ROUTE.OBJ \
	QUAD.OBJ SMOOTH.OBJ DIGITIZE.OBJ PEOPLNET.OBJ THEMES.OBJ GRAPHIC7.OBJ EXPORT.OBJ \
	VISIBLE.OBJ REPORT.OBJ FLOATAP.OBJ CLIDATA.OBJ GRAPHIC8.OBJ FUNCTION.OBJ TRANLAT3.OBJ \
	GRAPHIC9.OBJ DXFIN.OBJ THMMSGP.OBJ FISH.OBJ GPS.OBJ EDGMATCH.OBJ GRAPHICA.OBJ TRAVERSE.OBJ \
	DTM.OBJ TIFF.OBJ ORTHOS.OBJ FUNCSUBS.OBJ CONVERT.OBJ FUNCTN2.OBJ MAPCOPY.OBJ GRFUNC.OBJ \
	GRAPHICB.OBJ CDSTUFF.OBJ TRAN.OBJ CMDTONUM.OBJ GCTP.OBJ FOR_INIT.OBJ INV_INIT.OBJ \
	UNTFZ.OBJ ALBERFOR.OBJ ALBERINV.OBJ ALCONFOR.OBJ ALCONINV.OBJ AZIMFOR.OBJ AZIMINV.OBJ \
	BR_GCTP.OBJ CPROJ.OBJ EQCONINV.OBJ EQUIFOR.OBJ EQUIINV.OBJ GNOMFOR.OBJ GNOMINV.OBJ \
	GOODFOR.OBJ GOODINV.OBJ GVNSPFOR.OBJ GVNSPINV.OBJ HAMFOR.OBJ HAMINV.OBJ IMOLWFOR.OBJ \
	IMOLWINV.OBJ LAMAZFOR.OBJ LAMAZINV.OBJ LAMCCFOR.OBJ LAMCCINV.OBJ MERFOR.OBJ MERINV.OBJ \
	MILLFOR.OBJ MILLINV.OBJ MOLWFOR.OBJ MOLWINV.OBJ OBLEQFOR.OBJ OBLEQINV.OBJ OMERFOR.OBJ \
	OMERINV.OBJ ORTHFOR.OBJ ORTHINV.OBJ PAKSZ.OBJ POLYFOR.OBJ POLYINV.OBJ PSFOR.OBJ PSINV.OBJ \
	ROBFOR.OBJ ROBINV.OBJ SINFOR.OBJ SININV.OBJ SOMFOR.OBJ SOMINV.OBJ SPHDZ.OBJ STERFOR.OBJ \
	STERINV.OBJ STPLNFOR.OBJ STPLNINV.OBJ TMFOR.OBJ TMINV.OBJ UTMFOR.OBJ UTMINV.OBJ VANDGFOR.OBJ \
	VANDGINV.OBJ WIVFOR.OBJ WVIIFOR.OBJ WVIIINV.OBJ REPORTGC.OBJ EQCONFOR.OBJ WIVINV.OBJ \
	BIGMEM.OBJ DIBUTIL.OBJ FILE.OBJ COPYDIB.OBJ VIEWPORT.OBJ GSPLINE.OBJ SSP.OBJ MEMALLOC.OBJ \
	TEMP3.OBJ THEMES2.OBJ HOTSPOTS.OBJ VEHICLE.OBJ DBFOPEN.OBJ ADDRESS3.OBJ EXPORT2.OBJ \
	FUNDEFS.OBJ SHPFILE.OBJ ORAFILE.OBJ GM32LIB.OBJ DGNFILE.OBJ GRAPHICC.OBJ VARSTUF2.OBJ \
	FUNCTN3.OBJ BCI.OBJ TDSPL.OBJ TOOLS.OBJ UNDO.OBJ VARDEF.OBJ DICTEDIT.OBJ ROADNAME.OBJ \
	SOLAR.OBJ VOTERS.OBJ MAKEAREA.OBJ THEMES3.OBJ FUNCTN1.OBJ GRAPHICD.OBJ DTM2.OBJ PROFILE.OBJ \
	CONVRT32.OBJ $(OBJS_EXT) $(DEFFILE)
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
GPS.OBJ +
EDGMATCH.OBJ +
GRAPHICA.OBJ +
TRAVERSE.OBJ +
DTM.OBJ +
TIFF.OBJ +
ORTHOS.OBJ +
FUNCSUBS.OBJ +
CONVERT.OBJ +
FUNCTN2.OBJ +
MAPCOPY.OBJ +
GRFUNC.OBJ +
GRAPHICB.OBJ +
CDSTUFF.OBJ +
TRAN.OBJ +
CMDTONUM.OBJ +
GCTP.OBJ +
FOR_INIT.OBJ +
INV_INIT.OBJ +
UNTFZ.OBJ +
ALBERFOR.OBJ +
ALBERINV.OBJ +
ALCONFOR.OBJ +
ALCONINV.OBJ +
AZIMFOR.OBJ +
AZIMINV.OBJ +
BR_GCTP.OBJ +
CPROJ.OBJ +
EQCONINV.OBJ +
EQUIFOR.OBJ +
EQUIINV.OBJ +
GNOMFOR.OBJ +
GNOMINV.OBJ +
GOODFOR.OBJ +
GOODINV.OBJ +
GVNSPFOR.OBJ +
GVNSPINV.OBJ +
HAMFOR.OBJ +
HAMINV.OBJ +
IMOLWFOR.OBJ +
IMOLWINV.OBJ +
LAMAZFOR.OBJ +
LAMAZINV.OBJ +
LAMCCFOR.OBJ +
LAMCCINV.OBJ +
MERFOR.OBJ +
MERINV.OBJ +
MILLFOR.OBJ +
MILLINV.OBJ +
MOLWFOR.OBJ +
MOLWINV.OBJ +
OBLEQFOR.OBJ +
OBLEQINV.OBJ +
OMERFOR.OBJ +
OMERINV.OBJ +
ORTHFOR.OBJ +
ORTHINV.OBJ +
PAKSZ.OBJ +
POLYFOR.OBJ +
POLYINV.OBJ +
PSFOR.OBJ +
PSINV.OBJ +
ROBFOR.OBJ +
ROBINV.OBJ +
SINFOR.OBJ +
SININV.OBJ +
SOMFOR.OBJ +
SOMINV.OBJ +
SPHDZ.OBJ +
STERFOR.OBJ +
STERINV.OBJ +
STPLNFOR.OBJ +
STPLNINV.OBJ +
TMFOR.OBJ +
TMINV.OBJ +
UTMFOR.OBJ +
UTMINV.OBJ +
VANDGFOR.OBJ +
VANDGINV.OBJ +
WIVFOR.OBJ +
WVIIFOR.OBJ +
WVIIINV.OBJ +
REPORTGC.OBJ +
EQCONFOR.OBJ +
WIVINV.OBJ +
BIGMEM.OBJ +
DIBUTIL.OBJ +
FILE.OBJ +
COPYDIB.OBJ +
VIEWPORT.OBJ +
GSPLINE.OBJ +
SSP.OBJ +
MEMALLOC.OBJ +
TEMP3.OBJ +
THEMES2.OBJ +
HOTSPOTS.OBJ +
VEHICLE.OBJ +
DBFOPEN.OBJ +
ADDRESS3.OBJ +
EXPORT2.OBJ +
FUNDEFS.OBJ +
SHPFILE.OBJ +
ORAFILE.OBJ +
GM32LIB.OBJ +
DGNFILE.OBJ +
GRAPHICC.OBJ +
VARSTUF2.OBJ +
FUNCTN3.OBJ +
BCI.OBJ +
TDSPL.OBJ +
TOOLS.OBJ +
UNDO.OBJ +
VARDEF.OBJ +
DICTEDIT.OBJ +
ROADNAME.OBJ +
SOLAR.OBJ +
VOTERS.OBJ +
MAKEAREA.OBJ +
THEMES3.OBJ +
FUNCTN1.OBJ +
GRAPHICD.OBJ +
DTM2.OBJ +
PROFILE.OBJ +
CONVRT32.OBJ +
$(OBJS_EXT)
$(PROJ).EXE
$(MAPFILE)
c:\msvc\lib\+
c:\msvc\mfc\lib\+
..\..\..\MSVC\LIB\MMSYSTEM.LIB+
..\SHR\WINEXEC.LIB+
..\LIB\ULTGRD.LIB+
..\LIB\CRYPTO16.LIB+
..\..\..\MSVC\LIB\WINSOCK.LIB+
..\..\..\VFWDK\LIB\VFW.LIB+
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
