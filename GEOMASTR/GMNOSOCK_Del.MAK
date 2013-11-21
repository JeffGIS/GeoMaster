# Microsoft Visual C++ generated build script - Do not modify

PROJ = GMNOSOCK
DEBUG = 0
PROGTYPE = 0
CALLER = 
ARGS = basic1.gmc /WD g:\gmvoters
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
CFLAGS_D_WEXE = /nologo /f- /Gs /G3 /FPc /Zp1 /Zi /AL /Gt0 /Gx- /Od /OV8 /D "_DEBUG" /D "ENABLETRACEx" /D "CHECKMEM" /FR /Fd"GMNOSOCK.PDB"
CFLAGS_R_WEXE = /nologo /f- /Gs /G3 /FPc /Zp1 /Zd /ALu /Gt0 /Gx- /Ob2 /OV8 /D "NDEBUG" /D "ENABLETRACEx" /D "CHECKMEMx" /FR 
LFLAGS_D_WEXE = /NOLOGO /NOD /NOE /PACKC:61440 /FARCALL /STACK:25600 /SEG:512 /ALIGN:64 /ONERROR:NOEXE /CO /MAP /LINE  
LFLAGS_R_WEXE = /NOLOGO /NOD /PACKC:61440 /STACK:25600 /SEG:512 /ALIGN:64 /ONERROR:NOEXE /MAP /LINE  
LIBS_D_WEXE = oldnames libw llibcew odbc commdlg.lib ddeml.lib shell.lib toolhelp.lib 
LIBS_R_WEXE = oldnames libw llibcew odbc commdlg.lib ddeml.lib mmsystem.lib shell.lib toolhelp.lib 
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
		DICTEDIT.SBR


MMSYSTEM_DEP = 

WINEXEC_DEP = 

ULTGRD_DEP = 

CRYPTO16_DEP = 

WINSOCK_DEP = 

VFW_DEP = 

CWCONFIG_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\crypto16.h \
	c:\gssi\prog\include\gmextern.h


GRAPHIC1_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


GRAPHIC2_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


GWCONFIG_RCDEP = c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\address.h \
	c:\gssi\prog\include\cddemo.h \
	c:\gssi\prog\geomastr\earth.ico \
	c:\gssi\prog\geomastr\icon1.ico \
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
	c:\gssi\prog\geomastr\cursor4.cur \
	c:\gssi\prog\geomastr\handmove.cur \
	c:\gssi\prog\geomastr\show.cur \
	c:\gssi\prog\geomastr\drawing.cur \
	c:\gssi\prog\geomastr\togglevi.cur \
	c:\gssi\prog\geomastr\settag.cur


SHR_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


BT_CREAT_DEP = c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gmextern.h


COMDLG_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\cddemo.h \
	c:\gssi\prog\include\gmextern.h


ADDRESS1_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


CREATCFG_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


ODBCMAIN_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\shapefil.h \
	c:\gssi\prog\include\gmextern.h


GRAPHIC3_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


UMIO_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


VARSTUFF_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


GWD_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


OFFSET_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h \
	c:\gssi\prog\include\offsetmn.h \
	c:\gssi\prog\include\polycom.h \
	c:\gssi\prog\include\offdefs.h \
	c:\gssi\prog\include\polybt.h


POLY_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h \
	c:\gssi\prog\include\offsetmn.h \
	c:\gssi\prog\include\polycom.h \
	c:\gssi\prog\include\offdefs.h \
	c:\gssi\prog\include\polybt.h \
	c:\gssi\prog\include\cnstnt.h


COMPUT1_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


HASH_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h


MS_MAPL_DEP = c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h


GRAPHIC4_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


DLGLOOK_DEP = c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gmextern.h


CVTCOORD_DEP = c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


WRITEAVI_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
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
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


UGRID_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\ugtable.h \
	c:\gssi\prog\include\gmextern.h


UGRIDHLT_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\ugtable.h \
	c:\gssi\prog\include\gmextern.h


STD_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


LOADPR~1_DEP = c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\gmextern.h \
	c:\gssi\prog\include\proj.h


T2PROJ~1_DEP = c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\proj.h


TRANPR~1_DEP = c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\proj.h


CONNAD83_DEP = c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h


DDE_DEP = c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h


DOCUMENT_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


TRANSLAT_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


DICT_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


PRINT_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h \
	c:\gssi\prog\include\cddemo.h


GRAPHIC6_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


DYNAEDIT_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\dyndlg.h \
	c:\gssi\prog\include\gmextern.h


NETWORKS_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


TRANLAT2_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


ADDRESS2_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


ROUTE_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


QUAD_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


SMOOTH_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h


DIGITIZE_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


PEOPLNET_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


THEMES_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


GRAPHIC7_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


EXPORT_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


VISIBLE_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


REPORT_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


FLOATAP_DEP = c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gmextern.h


CLIDATA_DEP = c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


GRAPHIC8_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


FUNCTION_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


TRANLAT3_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


GRAPHIC9_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


DXFIN_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


THMMSGP_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


FISH_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


GPS_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\gps\garmin.h \
	c:\gssi\prog\include\gmextern.h


EDGMATCH_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


GRAPHICA_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


TRAVERSE_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


DTM_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\bci.h \
	c:\gssi\prog\include\gmextern.h


TIFF_DEP = c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h


ORTHOS_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


FUNCSUBS_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


CONVERT_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h


FUNCTN2_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


MAPCOPY_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


GRFUNC_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


GRAPHICB_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


CDSTUFF_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


TRAN_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


CMDTONUM_DEP = c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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


STPLNFOR_DEP = c:\gssi\prog\gctpc\cproj.h \
	c:\gssi\prog\gctpc\proj.h


STPLNINV_DEP = c:\gssi\prog\gctpc\cproj.h \
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


REPORTGC_DEP = c:\gssi\prog\gctpc\cproj.h \
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
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\dibutil.h \
	c:\gssi\prog\include\gmextern.h


FILE_DEP = c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


GSPLINE_DEP = c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h \
	c:\gssi\prog\include\bigmemln.h \
	c:\gssi\prog\include\bigmem.h


SSP_DEP = c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
	c:\msvc\include\winsock.h \
	c:\msvc\include\tchar.h \
	c:\gssi\prog\include\gssitype.h \
	c:\gssi\prog\include\gmlimits.h \
	c:\gssi\prog\include\compute.h


TEMP3_DEP = c:\gssi\prog\include\graphint.h


TEMP3_DEP = c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h


THEMES2_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


HOTSPOTS_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


VEHICLE_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


DBFOPEN_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\shapefil.h


ADDRESS3_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


EXPORT2_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


FUNDEFS_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


SHPFILE_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\shapefil.h \
	c:\gssi\prog\include\gmextern.h


ORAFILE_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


GM32LIB_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h \
	c:\gssi\prog\include\wownt16.h


DGNFILE_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


GRAPHICC_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


VARSTUF2_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


FUNCTN3_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


BCI_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\bci.h


TDSPL_DEP = c:\gssi\prog\include\bci.h


TOOLS_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\bci.h


UNDO_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


VARDEF_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h


DICTEDIT_DEP = c:\gssi\prog\include\graphint.h \
	c:\gssi\prog\include\shr.h \
	c:\progra~1\micros~4\include\winuser.h \
	c:\progra~1\micros~4\include\pshpack2.h \
	c:\progra~1\micros~4\include\poppack.h \
	c:\progra~1\micros~4\include\tvout.h \
	c:\progra~1\micros~4\include\guiddef.h \
	c:\progra~1\micros~4\include\winuser.inl \
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
	c:\gssi\prog\include\dibapi.h \
	c:\gssi\prog\include\std.h \
	c:\gssi\prog\include\translat.h \
	c:\gssi\prog\include\gps.h \
	c:\gssi\prog\include\umio.h \
	c:\gssi\prog\include\proj.h \
	c:\gssi\prog\include\winexec.h \
	c:\gssi\prog\include\extrndb.h \
	c:\gssi\prog\include\dgn7.h \
	c:\gssi\prog\include\dxfcom.h \
	c:\gssi\prog\include\pnet.h \
	c:\gssi\prog\include\mapl.h \
	c:\gssi\prog\include\hash.h \
	c:\gssi\prog\include\bt.h \
	c:\gssi\prog\include\gwd.h \
	c:\gssi\prog\include\xll.h \
	c:\gssi\prog\geomastr\gw.h \
	c:\gssi\prog\include\client.h \
	c:\gssi\prog\include\gmextern.h


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

.OBJ:	 $(GPS_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(EDGMATCH_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(GRAPHICA_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(TRAVERSE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(DTM_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(TIFF_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(ORTHOS_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(FUNCSUBS_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(CONVERT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(FUNCTN2_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(MAPCOPY_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(GRFUNC_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(GRAPHICB_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(CDSTUFF_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(TRAN_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(CMDTONUM_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(GCTP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(FOR_INIT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(INV_INIT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(UNTFZ_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(ALBERFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(ALBERINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(ALCONFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(ALCONINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(AZIMFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(AZIMINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(BR_GCTP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(CPROJ_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(EQCONINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(EQUIFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(EQUIINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(GNOMFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(GNOMINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(GOODFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(GOODINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(GVNSPFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(GVNSPINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(HAMFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(HAMINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(IMOLWFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(IMOLWINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(LAMAZFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(LAMAZINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(LAMCCFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(LAMCCINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(MERFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(MERINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(MILLFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(MILLINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(MOLWFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(MOLWINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(OBLEQFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(OBLEQINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(OMERFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(OMERINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(ORTHFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(ORTHINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(PAKSZ_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(POLYFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(POLYINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(PSFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(PSINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(ROBFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(ROBINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(SINFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(SININV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(SOMFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(SOMINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(SPHDZ_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(STERFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(STERINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(STPLNFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(STPLNINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(TMFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(TMINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(UTMFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(UTMINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(VANDGFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(VANDGINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(WIVFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(WVIIFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(WVIIINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(REPORTGC_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(EQCONFOR_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(WIVINV_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(BIGMEM_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(DIBUTIL_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(FILE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(COPYDIB_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(VIEWPORT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(GSPLINE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(SSP_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(MEMALLOC_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(TEMP3_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(THEMES2_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(HOTSPOTS_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(VEHICLE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(DBFOPEN_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(ADDRESS3_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(EXPORT2_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(FUNDEFS_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(SHPFILE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(ORAFILE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(GM32LIB_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(DGNFILE_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(GRAPHICC_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(VARSTUF2_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(FUNCTN3_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(BCI_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(TDSPL_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(TOOLS_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(UNDO_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(VARDEF_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 

.OBJ:	 $(DICTEDIT_DEP)
	$(CC) $(CFLAGS) $(CUSEPCHFLAG) /c 


$(PROJ).EXE::	GWCONFIG.RES

$(PROJ).EXE::	CWCONFIG.OBJ GRAPHIC1.OBJ GRAPHIC2.OBJ SHR.OBJ BT_CREAT.OBJ COMDLG.OBJ \
	ADDRESS1.OBJ CREATCFG.OBJ ODBCMAIN.OBJ GRAPHIC3.OBJ UMIO.OBJ VARSTUFF.OBJ GWD.OBJ \
	OFFSET.OBJ POLY.OBJ COMPUT1.OBJ HASH.OBJ MS_MAPL.OBJ GRAPHIC4.OBJ DLGLOOK.OBJ CVTCOORD.OBJ \
	GRAPHIC5.OBJ WRITEAVI.OBJ INFOBOX.OBJ UGRID.OBJ UGRIDHLT.OBJ STD.OBJ LOADPR~1.OBJ \
	T2PROJ~1.OBJ TRANPR~1.OBJ CONNAD83.OBJ DDE.OBJ PCXTOBMP.OBJ DOCUMENT.OBJ TRANSLAT.OBJ \
	DICT.OBJ PRINT.OBJ GRAPHIC6.OBJ DYNAEDIT.OBJ NETWORKS.OBJ TRANLAT2.OBJ ADDRESS2.OBJ \
	ROUTE.OBJ QUAD.OBJ SMOOTH.OBJ DIGITIZE.OBJ PEOPLNET.OBJ THEMES.OBJ GRAPHIC7.OBJ EXPORT.OBJ \
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
	FUNCTN3.OBJ BCI.OBJ TDSPL.OBJ TOOLS.OBJ UNDO.OBJ VARDEF.OBJ DICTEDIT.OBJ $(OBJS_EXT) $(DEFFILE)
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
