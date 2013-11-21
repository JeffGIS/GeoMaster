# Microsoft Developer Studio Generated NMAKE File, Based on geomastr.dsp
!IF "$(CFG)" == ""
CFG=geomastr - Win32 Debug
!MESSAGE No configuration specified. Defaulting to geomastr - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "geomastr - Win32 Release" && "$(CFG)" != "geomastr - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
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
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "geomastr - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\geomaster32u.exe" "$(OUTDIR)\geomastr.bsc"


CLEAN :
	-@erase "$(INTDIR)\address1.obj"
	-@erase "$(INTDIR)\address1.sbr"
	-@erase "$(INTDIR)\address2.obj"
	-@erase "$(INTDIR)\address2.sbr"
	-@erase "$(INTDIR)\address3.obj"
	-@erase "$(INTDIR)\address3.sbr"
	-@erase "$(INTDIR)\AddressMsgProc.obj"
	-@erase "$(INTDIR)\AddressMsgProc.sbr"
	-@erase "$(INTDIR)\adjlon.obj"
	-@erase "$(INTDIR)\adjlon.sbr"
	-@erase "$(INTDIR)\alberfor.obj"
	-@erase "$(INTDIR)\alberfor.sbr"
	-@erase "$(INTDIR)\alberinv.obj"
	-@erase "$(INTDIR)\alberinv.sbr"
	-@erase "$(INTDIR)\alconfor.obj"
	-@erase "$(INTDIR)\alconfor.sbr"
	-@erase "$(INTDIR)\alconinv.obj"
	-@erase "$(INTDIR)\alconinv.sbr"
	-@erase "$(INTDIR)\azimfor.obj"
	-@erase "$(INTDIR)\azimfor.sbr"
	-@erase "$(INTDIR)\aziminv.obj"
	-@erase "$(INTDIR)\aziminv.sbr"
	-@erase "$(INTDIR)\bci.obj"
	-@erase "$(INTDIR)\bci.sbr"
	-@erase "$(INTDIR)\bigmem.obj"
	-@erase "$(INTDIR)\bigmem.sbr"
	-@erase "$(INTDIR)\br_gctp.obj"
	-@erase "$(INTDIR)\br_gctp.sbr"
	-@erase "$(INTDIR)\Bt_creat.obj"
	-@erase "$(INTDIR)\Bt_creat.sbr"
	-@erase "$(INTDIR)\ByteSwap.obj"
	-@erase "$(INTDIR)\ByteSwap.sbr"
	-@erase "$(INTDIR)\cdstuff.obj"
	-@erase "$(INTDIR)\cdstuff.sbr"
	-@erase "$(INTDIR)\CLIDATA.OBJ"
	-@erase "$(INTDIR)\CLIDATA.SBR"
	-@erase "$(INTDIR)\cmdtonum.obj"
	-@erase "$(INTDIR)\cmdtonum.sbr"
	-@erase "$(INTDIR)\COMDLG.obj"
	-@erase "$(INTDIR)\COMDLG.sbr"
	-@erase "$(INTDIR)\Comput1.obj"
	-@erase "$(INTDIR)\Comput1.sbr"
	-@erase "$(INTDIR)\CONNAD83.OBJ"
	-@erase "$(INTDIR)\CONNAD83.SBR"
	-@erase "$(INTDIR)\convert.obj"
	-@erase "$(INTDIR)\convert.sbr"
	-@erase "$(INTDIR)\CONVRT32.OBJ"
	-@erase "$(INTDIR)\CONVRT32.SBR"
	-@erase "$(INTDIR)\copydib.obj"
	-@erase "$(INTDIR)\copydib.sbr"
	-@erase "$(INTDIR)\cproj.obj"
	-@erase "$(INTDIR)\cproj.sbr"
	-@erase "$(INTDIR)\Creatcfg.obj"
	-@erase "$(INTDIR)\Creatcfg.sbr"
	-@erase "$(INTDIR)\CVTCOORD.OBJ"
	-@erase "$(INTDIR)\CVTCOORD.SBR"
	-@erase "$(INTDIR)\cwconfig.obj"
	-@erase "$(INTDIR)\cwconfig.sbr"
	-@erase "$(INTDIR)\dbfopen.obj"
	-@erase "$(INTDIR)\dbfopen.sbr"
	-@erase "$(INTDIR)\DDE.OBJ"
	-@erase "$(INTDIR)\DDE.SBR"
	-@erase "$(INTDIR)\DGNFILE.OBJ"
	-@erase "$(INTDIR)\DGNFILE.SBR"
	-@erase "$(INTDIR)\DIBUTIL.OBJ"
	-@erase "$(INTDIR)\DIBUTIL.SBR"
	-@erase "$(INTDIR)\DICT.OBJ"
	-@erase "$(INTDIR)\DICT.SBR"
	-@erase "$(INTDIR)\DICTEDIT.OBJ"
	-@erase "$(INTDIR)\DICTEDIT.SBR"
	-@erase "$(INTDIR)\digitize.obj"
	-@erase "$(INTDIR)\digitize.sbr"
	-@erase "$(INTDIR)\DLGLOOK.OBJ"
	-@erase "$(INTDIR)\DLGLOOK.SBR"
	-@erase "$(INTDIR)\Document.obj"
	-@erase "$(INTDIR)\Document.sbr"
	-@erase "$(INTDIR)\dtm.obj"
	-@erase "$(INTDIR)\dtm.sbr"
	-@erase "$(INTDIR)\DTM2.OBJ"
	-@erase "$(INTDIR)\DTM2.SBR"
	-@erase "$(INTDIR)\Dxfin.obj"
	-@erase "$(INTDIR)\Dxfin.sbr"
	-@erase "$(INTDIR)\dynaedit.obj"
	-@erase "$(INTDIR)\dynaedit.sbr"
	-@erase "$(INTDIR)\edgmatch.obj"
	-@erase "$(INTDIR)\edgmatch.sbr"
	-@erase "$(INTDIR)\eqconfor.obj"
	-@erase "$(INTDIR)\eqconfor.sbr"
	-@erase "$(INTDIR)\eqconinv.obj"
	-@erase "$(INTDIR)\eqconinv.sbr"
	-@erase "$(INTDIR)\equifor.obj"
	-@erase "$(INTDIR)\equifor.sbr"
	-@erase "$(INTDIR)\equiinv.obj"
	-@erase "$(INTDIR)\equiinv.sbr"
	-@erase "$(INTDIR)\exception.obj"
	-@erase "$(INTDIR)\exception.sbr"
	-@erase "$(INTDIR)\export.obj"
	-@erase "$(INTDIR)\export.sbr"
	-@erase "$(INTDIR)\EXPORT2.OBJ"
	-@erase "$(INTDIR)\EXPORT2.SBR"
	-@erase "$(INTDIR)\FIIO_Mem.obj"
	-@erase "$(INTDIR)\FIIO_Mem.sbr"
	-@erase "$(INTDIR)\FILE.OBJ"
	-@erase "$(INTDIR)\FILE.SBR"
	-@erase "$(INTDIR)\fish.obj"
	-@erase "$(INTDIR)\fish.sbr"
	-@erase "$(INTDIR)\floatap.obj"
	-@erase "$(INTDIR)\floatap.sbr"
	-@erase "$(INTDIR)\for_init.obj"
	-@erase "$(INTDIR)\for_init.sbr"
	-@erase "$(INTDIR)\freeim.obj"
	-@erase "$(INTDIR)\freeim.sbr"
	-@erase "$(INTDIR)\FUNCSUBS.obj"
	-@erase "$(INTDIR)\FUNCSUBS.sbr"
	-@erase "$(INTDIR)\function.obj"
	-@erase "$(INTDIR)\function.sbr"
	-@erase "$(INTDIR)\FUNCTN1.OBJ"
	-@erase "$(INTDIR)\FUNCTN1.SBR"
	-@erase "$(INTDIR)\functn2.obj"
	-@erase "$(INTDIR)\functn2.sbr"
	-@erase "$(INTDIR)\FUNCTN3.OBJ"
	-@erase "$(INTDIR)\FUNCTN3.SBR"
	-@erase "$(INTDIR)\FUNDEFS.OBJ"
	-@erase "$(INTDIR)\FUNDEFS.SBR"
	-@erase "$(INTDIR)\gctp.obj"
	-@erase "$(INTDIR)\gctp.sbr"
	-@erase "$(INTDIR)\GetSDChipID.obj"
	-@erase "$(INTDIR)\GetSDChipID.sbr"
	-@erase "$(INTDIR)\GM32.obj"
	-@erase "$(INTDIR)\GM32.sbr"
	-@erase "$(INTDIR)\GM32LIB.OBJ"
	-@erase "$(INTDIR)\GM32LIB.SBR"
	-@erase "$(INTDIR)\gnomfor.obj"
	-@erase "$(INTDIR)\gnomfor.sbr"
	-@erase "$(INTDIR)\gnominv.obj"
	-@erase "$(INTDIR)\gnominv.sbr"
	-@erase "$(INTDIR)\goodfor.obj"
	-@erase "$(INTDIR)\goodfor.sbr"
	-@erase "$(INTDIR)\goodinv.obj"
	-@erase "$(INTDIR)\goodinv.sbr"
	-@erase "$(INTDIR)\gps.obj"
	-@erase "$(INTDIR)\gps.sbr"
	-@erase "$(INTDIR)\graphic1.obj"
	-@erase "$(INTDIR)\graphic1.sbr"
	-@erase "$(INTDIR)\graphic2.obj"
	-@erase "$(INTDIR)\graphic2.sbr"
	-@erase "$(INTDIR)\Graphic3.obj"
	-@erase "$(INTDIR)\Graphic3.sbr"
	-@erase "$(INTDIR)\graphic4.obj"
	-@erase "$(INTDIR)\graphic4.sbr"
	-@erase "$(INTDIR)\GRAPHIC5.OBJ"
	-@erase "$(INTDIR)\GRAPHIC5.SBR"
	-@erase "$(INTDIR)\Graphic6.obj"
	-@erase "$(INTDIR)\Graphic6.sbr"
	-@erase "$(INTDIR)\graphic7.obj"
	-@erase "$(INTDIR)\graphic7.sbr"
	-@erase "$(INTDIR)\graphic8.obj"
	-@erase "$(INTDIR)\graphic8.sbr"
	-@erase "$(INTDIR)\GRAPHIC9.obj"
	-@erase "$(INTDIR)\GRAPHIC9.sbr"
	-@erase "$(INTDIR)\graphic_functions.obj"
	-@erase "$(INTDIR)\graphic_functions.sbr"
	-@erase "$(INTDIR)\graphica.obj"
	-@erase "$(INTDIR)\graphica.sbr"
	-@erase "$(INTDIR)\graphicb.obj"
	-@erase "$(INTDIR)\graphicb.sbr"
	-@erase "$(INTDIR)\graphicc.obj"
	-@erase "$(INTDIR)\graphicc.sbr"
	-@erase "$(INTDIR)\GRAPHICD.OBJ"
	-@erase "$(INTDIR)\GRAPHICD.SBR"
	-@erase "$(INTDIR)\GraphMsgProc.obj"
	-@erase "$(INTDIR)\GraphMsgProc.sbr"
	-@erase "$(INTDIR)\GraphMsgProc2.obj"
	-@erase "$(INTDIR)\GraphMsgProc2.sbr"
	-@erase "$(INTDIR)\grfunc.obj"
	-@erase "$(INTDIR)\grfunc.sbr"
	-@erase "$(INTDIR)\gspline.obj"
	-@erase "$(INTDIR)\gspline.sbr"
	-@erase "$(INTDIR)\gssigdi.obj"
	-@erase "$(INTDIR)\gssigdi.sbr"
	-@erase "$(INTDIR)\gvnspfor.obj"
	-@erase "$(INTDIR)\gvnspfor.sbr"
	-@erase "$(INTDIR)\gvnspinv.obj"
	-@erase "$(INTDIR)\gvnspinv.sbr"
	-@erase "$(INTDIR)\GWCONFIG.res"
	-@erase "$(INTDIR)\gwd.obj"
	-@erase "$(INTDIR)\gwd.sbr"
	-@erase "$(INTDIR)\hamfor.obj"
	-@erase "$(INTDIR)\hamfor.sbr"
	-@erase "$(INTDIR)\haminv.obj"
	-@erase "$(INTDIR)\haminv.sbr"
	-@erase "$(INTDIR)\HASH.OBJ"
	-@erase "$(INTDIR)\HASH.SBR"
	-@erase "$(INTDIR)\hbird.obj"
	-@erase "$(INTDIR)\hbird.sbr"
	-@erase "$(INTDIR)\hotspots.obj"
	-@erase "$(INTDIR)\hotspots.sbr"
	-@erase "$(INTDIR)\imolwfor.obj"
	-@erase "$(INTDIR)\imolwfor.sbr"
	-@erase "$(INTDIR)\imolwinv.obj"
	-@erase "$(INTDIR)\imolwinv.sbr"
	-@erase "$(INTDIR)\Infobox.obj"
	-@erase "$(INTDIR)\Infobox.sbr"
	-@erase "$(INTDIR)\inv_init.obj"
	-@erase "$(INTDIR)\inv_init.sbr"
	-@erase "$(INTDIR)\lamazfor.obj"
	-@erase "$(INTDIR)\lamazfor.sbr"
	-@erase "$(INTDIR)\lamazinv.obj"
	-@erase "$(INTDIR)\lamazinv.sbr"
	-@erase "$(INTDIR)\lamccfor.obj"
	-@erase "$(INTDIR)\lamccfor.sbr"
	-@erase "$(INTDIR)\lamccinv.obj"
	-@erase "$(INTDIR)\lamccinv.sbr"
	-@erase "$(INTDIR)\license.obj"
	-@erase "$(INTDIR)\license.sbr"
	-@erase "$(INTDIR)\LkmToHbirdImage.obj"
	-@erase "$(INTDIR)\LkmToHbirdImage.sbr"
	-@erase "$(INTDIR)\makearea.obj"
	-@erase "$(INTDIR)\makearea.sbr"
	-@erase "$(INTDIR)\mapcopy.obj"
	-@erase "$(INTDIR)\mapcopy.sbr"
	-@erase "$(INTDIR)\memalloc.obj"
	-@erase "$(INTDIR)\memalloc.sbr"
	-@erase "$(INTDIR)\merfor.obj"
	-@erase "$(INTDIR)\merfor.sbr"
	-@erase "$(INTDIR)\merinv.obj"
	-@erase "$(INTDIR)\merinv.sbr"
	-@erase "$(INTDIR)\mgrs.obj"
	-@erase "$(INTDIR)\mgrs.sbr"
	-@erase "$(INTDIR)\millfor.obj"
	-@erase "$(INTDIR)\millfor.sbr"
	-@erase "$(INTDIR)\millinv.obj"
	-@erase "$(INTDIR)\millinv.sbr"
	-@erase "$(INTDIR)\minilzo.obj"
	-@erase "$(INTDIR)\minilzo.sbr"
	-@erase "$(INTDIR)\molwfor.obj"
	-@erase "$(INTDIR)\molwfor.sbr"
	-@erase "$(INTDIR)\molwinv.obj"
	-@erase "$(INTDIR)\molwinv.sbr"
	-@erase "$(INTDIR)\mrsid.obj"
	-@erase "$(INTDIR)\mrsid.sbr"
	-@erase "$(INTDIR)\MS_MAPL.OBJ"
	-@erase "$(INTDIR)\MS_MAPL.SBR"
	-@erase "$(INTDIR)\nad_cvt.obj"
	-@erase "$(INTDIR)\nad_cvt.sbr"
	-@erase "$(INTDIR)\nad_init.obj"
	-@erase "$(INTDIR)\nad_init.sbr"
	-@erase "$(INTDIR)\nad_intr.obj"
	-@erase "$(INTDIR)\nad_intr.sbr"
	-@erase "$(INTDIR)\nadcon.obj"
	-@erase "$(INTDIR)\nadcon.sbr"
	-@erase "$(INTDIR)\NETWORKS.obj"
	-@erase "$(INTDIR)\NETWORKS.sbr"
	-@erase "$(INTDIR)\obleqfor.obj"
	-@erase "$(INTDIR)\obleqfor.sbr"
	-@erase "$(INTDIR)\obleqinv.obj"
	-@erase "$(INTDIR)\obleqinv.sbr"
	-@erase "$(INTDIR)\Odbcmain.obj"
	-@erase "$(INTDIR)\Odbcmain.sbr"
	-@erase "$(INTDIR)\Offset.obj"
	-@erase "$(INTDIR)\Offset.sbr"
	-@erase "$(INTDIR)\omerfor.obj"
	-@erase "$(INTDIR)\omerfor.sbr"
	-@erase "$(INTDIR)\omerinv.obj"
	-@erase "$(INTDIR)\omerinv.sbr"
	-@erase "$(INTDIR)\ORAFILE.OBJ"
	-@erase "$(INTDIR)\ORAFILE.SBR"
	-@erase "$(INTDIR)\orthfor.obj"
	-@erase "$(INTDIR)\orthfor.sbr"
	-@erase "$(INTDIR)\orthinv.obj"
	-@erase "$(INTDIR)\orthinv.sbr"
	-@erase "$(INTDIR)\orthos.obj"
	-@erase "$(INTDIR)\orthos.sbr"
	-@erase "$(INTDIR)\paksz.obj"
	-@erase "$(INTDIR)\paksz.sbr"
	-@erase "$(INTDIR)\PCXTOBMP.OBJ"
	-@erase "$(INTDIR)\PCXTOBMP.SBR"
	-@erase "$(INTDIR)\peoplnet.obj"
	-@erase "$(INTDIR)\peoplnet.sbr"
	-@erase "$(INTDIR)\pj_errno.obj"
	-@erase "$(INTDIR)\pj_errno.sbr"
	-@erase "$(INTDIR)\pj_malloc.obj"
	-@erase "$(INTDIR)\pj_malloc.sbr"
	-@erase "$(INTDIR)\pj_open_lib.obj"
	-@erase "$(INTDIR)\pj_open_lib.sbr"
	-@erase "$(INTDIR)\pngrid.obj"
	-@erase "$(INTDIR)\pngrid.sbr"
	-@erase "$(INTDIR)\pointlist.obj"
	-@erase "$(INTDIR)\pointlist.sbr"
	-@erase "$(INTDIR)\polarst.obj"
	-@erase "$(INTDIR)\polarst.sbr"
	-@erase "$(INTDIR)\POLY.OBJ"
	-@erase "$(INTDIR)\POLY.SBR"
	-@erase "$(INTDIR)\polyfor.obj"
	-@erase "$(INTDIR)\polyfor.sbr"
	-@erase "$(INTDIR)\polyinv.obj"
	-@erase "$(INTDIR)\polyinv.sbr"
	-@erase "$(INTDIR)\Print.obj"
	-@erase "$(INTDIR)\Print.sbr"
	-@erase "$(INTDIR)\PROFILE.OBJ"
	-@erase "$(INTDIR)\PROFILE.SBR"
	-@erase "$(INTDIR)\psfor.obj"
	-@erase "$(INTDIR)\psfor.sbr"
	-@erase "$(INTDIR)\psinv.obj"
	-@erase "$(INTDIR)\psinv.sbr"
	-@erase "$(INTDIR)\quad.obj"
	-@erase "$(INTDIR)\quad.sbr"
	-@erase "$(INTDIR)\report.obj"
	-@erase "$(INTDIR)\report.sbr"
	-@erase "$(INTDIR)\reportgc.obj"
	-@erase "$(INTDIR)\reportgc.sbr"
	-@erase "$(INTDIR)\ROADNAME.OBJ"
	-@erase "$(INTDIR)\ROADNAME.SBR"
	-@erase "$(INTDIR)\robfor.obj"
	-@erase "$(INTDIR)\robfor.sbr"
	-@erase "$(INTDIR)\robinv.obj"
	-@erase "$(INTDIR)\robinv.sbr"
	-@erase "$(INTDIR)\route.obj"
	-@erase "$(INTDIR)\route.sbr"
	-@erase "$(INTDIR)\serio.obj"
	-@erase "$(INTDIR)\serio.sbr"
	-@erase "$(INTDIR)\SHPFILE.OBJ"
	-@erase "$(INTDIR)\SHPFILE.SBR"
	-@erase "$(INTDIR)\shr.obj"
	-@erase "$(INTDIR)\shr.sbr"
	-@erase "$(INTDIR)\signinv.obj"
	-@erase "$(INTDIR)\signinv.sbr"
	-@erase "$(INTDIR)\sinfor.obj"
	-@erase "$(INTDIR)\sinfor.sbr"
	-@erase "$(INTDIR)\sininv.obj"
	-@erase "$(INTDIR)\sininv.sbr"
	-@erase "$(INTDIR)\smooth.obj"
	-@erase "$(INTDIR)\smooth.sbr"
	-@erase "$(INTDIR)\somfor.obj"
	-@erase "$(INTDIR)\somfor.sbr"
	-@erase "$(INTDIR)\sominv.obj"
	-@erase "$(INTDIR)\sominv.sbr"
	-@erase "$(INTDIR)\sphdz.obj"
	-@erase "$(INTDIR)\sphdz.sbr"
	-@erase "$(INTDIR)\ssp.obj"
	-@erase "$(INTDIR)\ssp.sbr"
	-@erase "$(INTDIR)\STD.OBJ"
	-@erase "$(INTDIR)\STD.SBR"
	-@erase "$(INTDIR)\sterfor.obj"
	-@erase "$(INTDIR)\sterfor.sbr"
	-@erase "$(INTDIR)\sterinv.obj"
	-@erase "$(INTDIR)\sterinv.sbr"
	-@erase "$(INTDIR)\stplnfor.obj"
	-@erase "$(INTDIR)\stplnfor.sbr"
	-@erase "$(INTDIR)\stplninv.obj"
	-@erase "$(INTDIR)\stplninv.sbr"
	-@erase "$(INTDIR)\T2PROJ~1.OBJ"
	-@erase "$(INTDIR)\T2PROJ~1.SBR"
	-@erase "$(INTDIR)\tdspl.obj"
	-@erase "$(INTDIR)\tdspl.sbr"
	-@erase "$(INTDIR)\testMemIO.obj"
	-@erase "$(INTDIR)\testMemIO.sbr"
	-@erase "$(INTDIR)\themes.obj"
	-@erase "$(INTDIR)\themes.sbr"
	-@erase "$(INTDIR)\themes2.obj"
	-@erase "$(INTDIR)\themes2.sbr"
	-@erase "$(INTDIR)\THEMES3.OBJ"
	-@erase "$(INTDIR)\THEMES3.SBR"
	-@erase "$(INTDIR)\THMMSGP.OBJ"
	-@erase "$(INTDIR)\THMMSGP.SBR"
	-@erase "$(INTDIR)\tiff.obj"
	-@erase "$(INTDIR)\tiff.sbr"
	-@erase "$(INTDIR)\tmfor.obj"
	-@erase "$(INTDIR)\tmfor.sbr"
	-@erase "$(INTDIR)\tminv.obj"
	-@erase "$(INTDIR)\tminv.sbr"
	-@erase "$(INTDIR)\toolbar.obj"
	-@erase "$(INTDIR)\toolbar.sbr"
	-@erase "$(INTDIR)\tools.obj"
	-@erase "$(INTDIR)\tools.sbr"
	-@erase "$(INTDIR)\Tran.obj"
	-@erase "$(INTDIR)\Tran.sbr"
	-@erase "$(INTDIR)\tranlat2.obj"
	-@erase "$(INTDIR)\tranlat2.sbr"
	-@erase "$(INTDIR)\tranlat3.obj"
	-@erase "$(INTDIR)\tranlat3.sbr"
	-@erase "$(INTDIR)\tranmerc.obj"
	-@erase "$(INTDIR)\tranmerc.sbr"
	-@erase "$(INTDIR)\TRANPR~1.OBJ"
	-@erase "$(INTDIR)\TRANPR~1.SBR"
	-@erase "$(INTDIR)\translat.obj"
	-@erase "$(INTDIR)\translat.sbr"
	-@erase "$(INTDIR)\traverse.obj"
	-@erase "$(INTDIR)\traverse.sbr"
	-@erase "$(INTDIR)\UGRID.OBJ"
	-@erase "$(INTDIR)\UGRID.SBR"
	-@erase "$(INTDIR)\Ugridhlt.obj"
	-@erase "$(INTDIR)\Ugridhlt.sbr"
	-@erase "$(INTDIR)\Umio.obj"
	-@erase "$(INTDIR)\Umio.sbr"
	-@erase "$(INTDIR)\UNDO.OBJ"
	-@erase "$(INTDIR)\UNDO.SBR"
	-@erase "$(INTDIR)\untfz.obj"
	-@erase "$(INTDIR)\untfz.sbr"
	-@erase "$(INTDIR)\ups.obj"
	-@erase "$(INTDIR)\ups.sbr"
	-@erase "$(INTDIR)\USBSDK.obj"
	-@erase "$(INTDIR)\USBSDK.sbr"
	-@erase "$(INTDIR)\utm.obj"
	-@erase "$(INTDIR)\utm.sbr"
	-@erase "$(INTDIR)\utmfor.obj"
	-@erase "$(INTDIR)\utmfor.sbr"
	-@erase "$(INTDIR)\utminv.obj"
	-@erase "$(INTDIR)\utminv.sbr"
	-@erase "$(INTDIR)\vandgfor.obj"
	-@erase "$(INTDIR)\vandgfor.sbr"
	-@erase "$(INTDIR)\vandginv.obj"
	-@erase "$(INTDIR)\vandginv.sbr"
	-@erase "$(INTDIR)\VARDEF.OBJ"
	-@erase "$(INTDIR)\VARDEF.SBR"
	-@erase "$(INTDIR)\VARSTUF2.OBJ"
	-@erase "$(INTDIR)\VARSTUF2.SBR"
	-@erase "$(INTDIR)\VARSTUFF.OBJ"
	-@erase "$(INTDIR)\VARSTUFF.SBR"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\vehicle.obj"
	-@erase "$(INTDIR)\vehicle.sbr"
	-@erase "$(INTDIR)\vehicle_server.obj"
	-@erase "$(INTDIR)\vehicle_server.sbr"
	-@erase "$(INTDIR)\viewport.obj"
	-@erase "$(INTDIR)\viewport.sbr"
	-@erase "$(INTDIR)\visible.obj"
	-@erase "$(INTDIR)\visible.sbr"
	-@erase "$(INTDIR)\VOTERS.OBJ"
	-@erase "$(INTDIR)\VOTERS.SBR"
	-@erase "$(INTDIR)\wivfor.obj"
	-@erase "$(INTDIR)\wivfor.sbr"
	-@erase "$(INTDIR)\wivinv.obj"
	-@erase "$(INTDIR)\wivinv.sbr"
	-@erase "$(INTDIR)\Writeavi.obj"
	-@erase "$(INTDIR)\Writeavi.sbr"
	-@erase "$(INTDIR)\wviifor.obj"
	-@erase "$(INTDIR)\wviifor.sbr"
	-@erase "$(INTDIR)\wviiinv.obj"
	-@erase "$(INTDIR)\wviiinv.sbr"
	-@erase "$(OUTDIR)\geomaster32u.exe"
	-@erase "$(OUTDIR)\geomaster32u.map"
	-@erase "$(OUTDIR)\geomaster32u.pdb"
	-@erase "$(OUTDIR)\geomastr.bsc"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /Zp2 /ML /vd0 /GR /GX /Zi /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "NO_STRICT" /D "XPORABOVE" /D _WIN32_WINNT=0x0502 /D "USEMRSID" /D "CHECKMEMx" /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\GWCONFIG.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/o"$(OUTDIR)\geomastr.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\address1.sbr" \
	"$(INTDIR)\address2.sbr" \
	"$(INTDIR)\address3.sbr" \
	"$(INTDIR)\AddressMsgProc.sbr" \
	"$(INTDIR)\adjlon.sbr" \
	"$(INTDIR)\alberfor.sbr" \
	"$(INTDIR)\alberinv.sbr" \
	"$(INTDIR)\alconfor.sbr" \
	"$(INTDIR)\alconinv.sbr" \
	"$(INTDIR)\azimfor.sbr" \
	"$(INTDIR)\aziminv.sbr" \
	"$(INTDIR)\bci.sbr" \
	"$(INTDIR)\bigmem.sbr" \
	"$(INTDIR)\br_gctp.sbr" \
	"$(INTDIR)\Bt_creat.sbr" \
	"$(INTDIR)\ByteSwap.sbr" \
	"$(INTDIR)\cdstuff.sbr" \
	"$(INTDIR)\CLIDATA.SBR" \
	"$(INTDIR)\cmdtonum.sbr" \
	"$(INTDIR)\COMDLG.sbr" \
	"$(INTDIR)\Comput1.sbr" \
	"$(INTDIR)\CONNAD83.SBR" \
	"$(INTDIR)\convert.sbr" \
	"$(INTDIR)\CONVRT32.SBR" \
	"$(INTDIR)\copydib.sbr" \
	"$(INTDIR)\cproj.sbr" \
	"$(INTDIR)\Creatcfg.sbr" \
	"$(INTDIR)\CVTCOORD.SBR" \
	"$(INTDIR)\cwconfig.sbr" \
	"$(INTDIR)\dbfopen.sbr" \
	"$(INTDIR)\DDE.SBR" \
	"$(INTDIR)\DGNFILE.SBR" \
	"$(INTDIR)\DIBUTIL.SBR" \
	"$(INTDIR)\DICT.SBR" \
	"$(INTDIR)\DICTEDIT.SBR" \
	"$(INTDIR)\digitize.sbr" \
	"$(INTDIR)\DLGLOOK.SBR" \
	"$(INTDIR)\Document.sbr" \
	"$(INTDIR)\dtm.sbr" \
	"$(INTDIR)\DTM2.SBR" \
	"$(INTDIR)\Dxfin.sbr" \
	"$(INTDIR)\dynaedit.sbr" \
	"$(INTDIR)\edgmatch.sbr" \
	"$(INTDIR)\eqconfor.sbr" \
	"$(INTDIR)\eqconinv.sbr" \
	"$(INTDIR)\equifor.sbr" \
	"$(INTDIR)\equiinv.sbr" \
	"$(INTDIR)\exception.sbr" \
	"$(INTDIR)\export.sbr" \
	"$(INTDIR)\EXPORT2.SBR" \
	"$(INTDIR)\FIIO_Mem.sbr" \
	"$(INTDIR)\FILE.SBR" \
	"$(INTDIR)\fish.sbr" \
	"$(INTDIR)\floatap.sbr" \
	"$(INTDIR)\for_init.sbr" \
	"$(INTDIR)\freeim.sbr" \
	"$(INTDIR)\FUNCSUBS.sbr" \
	"$(INTDIR)\function.sbr" \
	"$(INTDIR)\FUNCTN1.SBR" \
	"$(INTDIR)\functn2.sbr" \
	"$(INTDIR)\FUNCTN3.SBR" \
	"$(INTDIR)\FUNDEFS.SBR" \
	"$(INTDIR)\gctp.sbr" \
	"$(INTDIR)\GetSDChipID.sbr" \
	"$(INTDIR)\GM32.sbr" \
	"$(INTDIR)\GM32LIB.SBR" \
	"$(INTDIR)\gnomfor.sbr" \
	"$(INTDIR)\gnominv.sbr" \
	"$(INTDIR)\goodfor.sbr" \
	"$(INTDIR)\goodinv.sbr" \
	"$(INTDIR)\gps.sbr" \
	"$(INTDIR)\graphic1.sbr" \
	"$(INTDIR)\graphic2.sbr" \
	"$(INTDIR)\Graphic3.sbr" \
	"$(INTDIR)\graphic4.sbr" \
	"$(INTDIR)\GRAPHIC5.SBR" \
	"$(INTDIR)\Graphic6.sbr" \
	"$(INTDIR)\graphic7.sbr" \
	"$(INTDIR)\graphic8.sbr" \
	"$(INTDIR)\GRAPHIC9.sbr" \
	"$(INTDIR)\graphic_functions.sbr" \
	"$(INTDIR)\graphica.sbr" \
	"$(INTDIR)\graphicb.sbr" \
	"$(INTDIR)\graphicc.sbr" \
	"$(INTDIR)\GRAPHICD.SBR" \
	"$(INTDIR)\GraphMsgProc.sbr" \
	"$(INTDIR)\GraphMsgProc2.sbr" \
	"$(INTDIR)\grfunc.sbr" \
	"$(INTDIR)\gspline.sbr" \
	"$(INTDIR)\gssigdi.sbr" \
	"$(INTDIR)\gvnspfor.sbr" \
	"$(INTDIR)\gvnspinv.sbr" \
	"$(INTDIR)\gwd.sbr" \
	"$(INTDIR)\hamfor.sbr" \
	"$(INTDIR)\haminv.sbr" \
	"$(INTDIR)\HASH.SBR" \
	"$(INTDIR)\hbird.sbr" \
	"$(INTDIR)\hotspots.sbr" \
	"$(INTDIR)\imolwfor.sbr" \
	"$(INTDIR)\imolwinv.sbr" \
	"$(INTDIR)\Infobox.sbr" \
	"$(INTDIR)\inv_init.sbr" \
	"$(INTDIR)\lamazfor.sbr" \
	"$(INTDIR)\lamazinv.sbr" \
	"$(INTDIR)\lamccfor.sbr" \
	"$(INTDIR)\lamccinv.sbr" \
	"$(INTDIR)\license.sbr" \
	"$(INTDIR)\LkmToHbirdImage.sbr" \
	"$(INTDIR)\makearea.sbr" \
	"$(INTDIR)\mapcopy.sbr" \
	"$(INTDIR)\memalloc.sbr" \
	"$(INTDIR)\merfor.sbr" \
	"$(INTDIR)\merinv.sbr" \
	"$(INTDIR)\mgrs.sbr" \
	"$(INTDIR)\millfor.sbr" \
	"$(INTDIR)\millinv.sbr" \
	"$(INTDIR)\minilzo.sbr" \
	"$(INTDIR)\molwfor.sbr" \
	"$(INTDIR)\molwinv.sbr" \
	"$(INTDIR)\mrsid.sbr" \
	"$(INTDIR)\MS_MAPL.SBR" \
	"$(INTDIR)\nad_cvt.sbr" \
	"$(INTDIR)\nad_init.sbr" \
	"$(INTDIR)\nad_intr.sbr" \
	"$(INTDIR)\nadcon.sbr" \
	"$(INTDIR)\NETWORKS.sbr" \
	"$(INTDIR)\obleqfor.sbr" \
	"$(INTDIR)\obleqinv.sbr" \
	"$(INTDIR)\Odbcmain.sbr" \
	"$(INTDIR)\Offset.sbr" \
	"$(INTDIR)\omerfor.sbr" \
	"$(INTDIR)\omerinv.sbr" \
	"$(INTDIR)\ORAFILE.SBR" \
	"$(INTDIR)\orthfor.sbr" \
	"$(INTDIR)\orthinv.sbr" \
	"$(INTDIR)\orthos.sbr" \
	"$(INTDIR)\paksz.sbr" \
	"$(INTDIR)\PCXTOBMP.SBR" \
	"$(INTDIR)\peoplnet.sbr" \
	"$(INTDIR)\pj_errno.sbr" \
	"$(INTDIR)\pj_malloc.sbr" \
	"$(INTDIR)\pj_open_lib.sbr" \
	"$(INTDIR)\pngrid.sbr" \
	"$(INTDIR)\pointlist.sbr" \
	"$(INTDIR)\polarst.sbr" \
	"$(INTDIR)\POLY.SBR" \
	"$(INTDIR)\polyfor.sbr" \
	"$(INTDIR)\polyinv.sbr" \
	"$(INTDIR)\Print.sbr" \
	"$(INTDIR)\PROFILE.SBR" \
	"$(INTDIR)\psfor.sbr" \
	"$(INTDIR)\psinv.sbr" \
	"$(INTDIR)\quad.sbr" \
	"$(INTDIR)\report.sbr" \
	"$(INTDIR)\reportgc.sbr" \
	"$(INTDIR)\ROADNAME.SBR" \
	"$(INTDIR)\robfor.sbr" \
	"$(INTDIR)\robinv.sbr" \
	"$(INTDIR)\route.sbr" \
	"$(INTDIR)\serio.sbr" \
	"$(INTDIR)\SHPFILE.SBR" \
	"$(INTDIR)\shr.sbr" \
	"$(INTDIR)\signinv.sbr" \
	"$(INTDIR)\sinfor.sbr" \
	"$(INTDIR)\sininv.sbr" \
	"$(INTDIR)\smooth.sbr" \
	"$(INTDIR)\somfor.sbr" \
	"$(INTDIR)\sominv.sbr" \
	"$(INTDIR)\sphdz.sbr" \
	"$(INTDIR)\ssp.sbr" \
	"$(INTDIR)\STD.SBR" \
	"$(INTDIR)\sterfor.sbr" \
	"$(INTDIR)\sterinv.sbr" \
	"$(INTDIR)\stplnfor.sbr" \
	"$(INTDIR)\stplninv.sbr" \
	"$(INTDIR)\T2PROJ~1.SBR" \
	"$(INTDIR)\tdspl.sbr" \
	"$(INTDIR)\testMemIO.sbr" \
	"$(INTDIR)\themes.sbr" \
	"$(INTDIR)\themes2.sbr" \
	"$(INTDIR)\THEMES3.SBR" \
	"$(INTDIR)\THMMSGP.SBR" \
	"$(INTDIR)\tiff.sbr" \
	"$(INTDIR)\tmfor.sbr" \
	"$(INTDIR)\tminv.sbr" \
	"$(INTDIR)\toolbar.sbr" \
	"$(INTDIR)\tools.sbr" \
	"$(INTDIR)\Tran.sbr" \
	"$(INTDIR)\tranlat2.sbr" \
	"$(INTDIR)\tranlat3.sbr" \
	"$(INTDIR)\tranmerc.sbr" \
	"$(INTDIR)\TRANPR~1.SBR" \
	"$(INTDIR)\translat.sbr" \
	"$(INTDIR)\traverse.sbr" \
	"$(INTDIR)\UGRID.SBR" \
	"$(INTDIR)\Ugridhlt.sbr" \
	"$(INTDIR)\Umio.sbr" \
	"$(INTDIR)\UNDO.SBR" \
	"$(INTDIR)\untfz.sbr" \
	"$(INTDIR)\ups.sbr" \
	"$(INTDIR)\USBSDK.sbr" \
	"$(INTDIR)\utm.sbr" \
	"$(INTDIR)\utmfor.sbr" \
	"$(INTDIR)\utminv.sbr" \
	"$(INTDIR)\vandgfor.sbr" \
	"$(INTDIR)\vandginv.sbr" \
	"$(INTDIR)\VARDEF.SBR" \
	"$(INTDIR)\VARSTUF2.SBR" \
	"$(INTDIR)\VARSTUFF.SBR" \
	"$(INTDIR)\vehicle.sbr" \
	"$(INTDIR)\vehicle_server.sbr" \
	"$(INTDIR)\viewport.sbr" \
	"$(INTDIR)\visible.sbr" \
	"$(INTDIR)\VOTERS.SBR" \
	"$(INTDIR)\wivfor.sbr" \
	"$(INTDIR)\wivinv.sbr" \
	"$(INTDIR)\Writeavi.sbr" \
	"$(INTDIR)\wviifor.sbr" \
	"$(INTDIR)\wviiinv.sbr"

"$(OUTDIR)\geomastr.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib Ws2_32.lib cryp2005.lib vfw32.lib winmm.lib urlmon.lib setupapi.lib mpr.lib crypto32.lib comctl32.lib /nologo /version:32.8 /stack:0x400000 /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\geomaster32u.pdb" /map:"$(INTDIR)\geomaster32u.map" /debug /machine:I386 /def:".\GWCONFIG.DEF" /out:"$(OUTDIR)\geomaster32u.exe" /libpath:"c:\gssi\prog\lib" /mapinfo:LINES 
DEF_FILE= \
	".\GWCONFIG.DEF"
LINK32_OBJS= \
	"$(INTDIR)\address1.obj" \
	"$(INTDIR)\address2.obj" \
	"$(INTDIR)\address3.obj" \
	"$(INTDIR)\AddressMsgProc.obj" \
	"$(INTDIR)\adjlon.obj" \
	"$(INTDIR)\alberfor.obj" \
	"$(INTDIR)\alberinv.obj" \
	"$(INTDIR)\alconfor.obj" \
	"$(INTDIR)\alconinv.obj" \
	"$(INTDIR)\azimfor.obj" \
	"$(INTDIR)\aziminv.obj" \
	"$(INTDIR)\bci.obj" \
	"$(INTDIR)\bigmem.obj" \
	"$(INTDIR)\br_gctp.obj" \
	"$(INTDIR)\Bt_creat.obj" \
	"$(INTDIR)\ByteSwap.obj" \
	"$(INTDIR)\cdstuff.obj" \
	"$(INTDIR)\CLIDATA.OBJ" \
	"$(INTDIR)\cmdtonum.obj" \
	"$(INTDIR)\COMDLG.obj" \
	"$(INTDIR)\Comput1.obj" \
	"$(INTDIR)\CONNAD83.OBJ" \
	"$(INTDIR)\convert.obj" \
	"$(INTDIR)\CONVRT32.OBJ" \
	"$(INTDIR)\copydib.obj" \
	"$(INTDIR)\cproj.obj" \
	"$(INTDIR)\Creatcfg.obj" \
	"$(INTDIR)\CVTCOORD.OBJ" \
	"$(INTDIR)\cwconfig.obj" \
	"$(INTDIR)\dbfopen.obj" \
	"$(INTDIR)\DDE.OBJ" \
	"$(INTDIR)\DGNFILE.OBJ" \
	"$(INTDIR)\DIBUTIL.OBJ" \
	"$(INTDIR)\DICT.OBJ" \
	"$(INTDIR)\DICTEDIT.OBJ" \
	"$(INTDIR)\digitize.obj" \
	"$(INTDIR)\DLGLOOK.OBJ" \
	"$(INTDIR)\Document.obj" \
	"$(INTDIR)\dtm.obj" \
	"$(INTDIR)\DTM2.OBJ" \
	"$(INTDIR)\Dxfin.obj" \
	"$(INTDIR)\dynaedit.obj" \
	"$(INTDIR)\edgmatch.obj" \
	"$(INTDIR)\eqconfor.obj" \
	"$(INTDIR)\eqconinv.obj" \
	"$(INTDIR)\equifor.obj" \
	"$(INTDIR)\equiinv.obj" \
	"$(INTDIR)\exception.obj" \
	"$(INTDIR)\export.obj" \
	"$(INTDIR)\EXPORT2.OBJ" \
	"$(INTDIR)\FIIO_Mem.obj" \
	"$(INTDIR)\FILE.OBJ" \
	"$(INTDIR)\fish.obj" \
	"$(INTDIR)\floatap.obj" \
	"$(INTDIR)\for_init.obj" \
	"$(INTDIR)\freeim.obj" \
	"$(INTDIR)\FUNCSUBS.obj" \
	"$(INTDIR)\function.obj" \
	"$(INTDIR)\FUNCTN1.OBJ" \
	"$(INTDIR)\functn2.obj" \
	"$(INTDIR)\FUNCTN3.OBJ" \
	"$(INTDIR)\FUNDEFS.OBJ" \
	"$(INTDIR)\gctp.obj" \
	"$(INTDIR)\GetSDChipID.obj" \
	"$(INTDIR)\GM32.obj" \
	"$(INTDIR)\GM32LIB.OBJ" \
	"$(INTDIR)\gnomfor.obj" \
	"$(INTDIR)\gnominv.obj" \
	"$(INTDIR)\goodfor.obj" \
	"$(INTDIR)\goodinv.obj" \
	"$(INTDIR)\gps.obj" \
	"$(INTDIR)\graphic1.obj" \
	"$(INTDIR)\graphic2.obj" \
	"$(INTDIR)\Graphic3.obj" \
	"$(INTDIR)\graphic4.obj" \
	"$(INTDIR)\GRAPHIC5.OBJ" \
	"$(INTDIR)\Graphic6.obj" \
	"$(INTDIR)\graphic7.obj" \
	"$(INTDIR)\graphic8.obj" \
	"$(INTDIR)\GRAPHIC9.obj" \
	"$(INTDIR)\graphic_functions.obj" \
	"$(INTDIR)\graphica.obj" \
	"$(INTDIR)\graphicb.obj" \
	"$(INTDIR)\graphicc.obj" \
	"$(INTDIR)\GRAPHICD.OBJ" \
	"$(INTDIR)\GraphMsgProc.obj" \
	"$(INTDIR)\GraphMsgProc2.obj" \
	"$(INTDIR)\grfunc.obj" \
	"$(INTDIR)\gspline.obj" \
	"$(INTDIR)\gssigdi.obj" \
	"$(INTDIR)\gvnspfor.obj" \
	"$(INTDIR)\gvnspinv.obj" \
	"$(INTDIR)\gwd.obj" \
	"$(INTDIR)\hamfor.obj" \
	"$(INTDIR)\haminv.obj" \
	"$(INTDIR)\HASH.OBJ" \
	"$(INTDIR)\hbird.obj" \
	"$(INTDIR)\hotspots.obj" \
	"$(INTDIR)\imolwfor.obj" \
	"$(INTDIR)\imolwinv.obj" \
	"$(INTDIR)\Infobox.obj" \
	"$(INTDIR)\inv_init.obj" \
	"$(INTDIR)\lamazfor.obj" \
	"$(INTDIR)\lamazinv.obj" \
	"$(INTDIR)\lamccfor.obj" \
	"$(INTDIR)\lamccinv.obj" \
	"$(INTDIR)\license.obj" \
	"$(INTDIR)\LkmToHbirdImage.obj" \
	"$(INTDIR)\makearea.obj" \
	"$(INTDIR)\mapcopy.obj" \
	"$(INTDIR)\memalloc.obj" \
	"$(INTDIR)\merfor.obj" \
	"$(INTDIR)\merinv.obj" \
	"$(INTDIR)\mgrs.obj" \
	"$(INTDIR)\millfor.obj" \
	"$(INTDIR)\millinv.obj" \
	"$(INTDIR)\minilzo.obj" \
	"$(INTDIR)\molwfor.obj" \
	"$(INTDIR)\molwinv.obj" \
	"$(INTDIR)\mrsid.obj" \
	"$(INTDIR)\MS_MAPL.OBJ" \
	"$(INTDIR)\nad_cvt.obj" \
	"$(INTDIR)\nad_init.obj" \
	"$(INTDIR)\nad_intr.obj" \
	"$(INTDIR)\nadcon.obj" \
	"$(INTDIR)\NETWORKS.obj" \
	"$(INTDIR)\obleqfor.obj" \
	"$(INTDIR)\obleqinv.obj" \
	"$(INTDIR)\Odbcmain.obj" \
	"$(INTDIR)\Offset.obj" \
	"$(INTDIR)\omerfor.obj" \
	"$(INTDIR)\omerinv.obj" \
	"$(INTDIR)\ORAFILE.OBJ" \
	"$(INTDIR)\orthfor.obj" \
	"$(INTDIR)\orthinv.obj" \
	"$(INTDIR)\orthos.obj" \
	"$(INTDIR)\paksz.obj" \
	"$(INTDIR)\PCXTOBMP.OBJ" \
	"$(INTDIR)\peoplnet.obj" \
	"$(INTDIR)\pj_errno.obj" \
	"$(INTDIR)\pj_malloc.obj" \
	"$(INTDIR)\pj_open_lib.obj" \
	"$(INTDIR)\pngrid.obj" \
	"$(INTDIR)\pointlist.obj" \
	"$(INTDIR)\polarst.obj" \
	"$(INTDIR)\POLY.OBJ" \
	"$(INTDIR)\polyfor.obj" \
	"$(INTDIR)\polyinv.obj" \
	"$(INTDIR)\Print.obj" \
	"$(INTDIR)\PROFILE.OBJ" \
	"$(INTDIR)\psfor.obj" \
	"$(INTDIR)\psinv.obj" \
	"$(INTDIR)\quad.obj" \
	"$(INTDIR)\report.obj" \
	"$(INTDIR)\reportgc.obj" \
	"$(INTDIR)\ROADNAME.OBJ" \
	"$(INTDIR)\robfor.obj" \
	"$(INTDIR)\robinv.obj" \
	"$(INTDIR)\route.obj" \
	"$(INTDIR)\serio.obj" \
	"$(INTDIR)\SHPFILE.OBJ" \
	"$(INTDIR)\shr.obj" \
	"$(INTDIR)\signinv.obj" \
	"$(INTDIR)\sinfor.obj" \
	"$(INTDIR)\sininv.obj" \
	"$(INTDIR)\smooth.obj" \
	"$(INTDIR)\somfor.obj" \
	"$(INTDIR)\sominv.obj" \
	"$(INTDIR)\sphdz.obj" \
	"$(INTDIR)\ssp.obj" \
	"$(INTDIR)\STD.OBJ" \
	"$(INTDIR)\sterfor.obj" \
	"$(INTDIR)\sterinv.obj" \
	"$(INTDIR)\stplnfor.obj" \
	"$(INTDIR)\stplninv.obj" \
	"$(INTDIR)\T2PROJ~1.OBJ" \
	"$(INTDIR)\tdspl.obj" \
	"$(INTDIR)\testMemIO.obj" \
	"$(INTDIR)\themes.obj" \
	"$(INTDIR)\themes2.obj" \
	"$(INTDIR)\THEMES3.OBJ" \
	"$(INTDIR)\THMMSGP.OBJ" \
	"$(INTDIR)\tiff.obj" \
	"$(INTDIR)\tmfor.obj" \
	"$(INTDIR)\tminv.obj" \
	"$(INTDIR)\toolbar.obj" \
	"$(INTDIR)\tools.obj" \
	"$(INTDIR)\Tran.obj" \
	"$(INTDIR)\tranlat2.obj" \
	"$(INTDIR)\tranlat3.obj" \
	"$(INTDIR)\tranmerc.obj" \
	"$(INTDIR)\TRANPR~1.OBJ" \
	"$(INTDIR)\translat.obj" \
	"$(INTDIR)\traverse.obj" \
	"$(INTDIR)\UGRID.OBJ" \
	"$(INTDIR)\Ugridhlt.obj" \
	"$(INTDIR)\Umio.obj" \
	"$(INTDIR)\UNDO.OBJ" \
	"$(INTDIR)\untfz.obj" \
	"$(INTDIR)\ups.obj" \
	"$(INTDIR)\USBSDK.obj" \
	"$(INTDIR)\utm.obj" \
	"$(INTDIR)\utmfor.obj" \
	"$(INTDIR)\utminv.obj" \
	"$(INTDIR)\vandgfor.obj" \
	"$(INTDIR)\vandginv.obj" \
	"$(INTDIR)\VARDEF.OBJ" \
	"$(INTDIR)\VARSTUF2.OBJ" \
	"$(INTDIR)\VARSTUFF.OBJ" \
	"$(INTDIR)\vehicle.obj" \
	"$(INTDIR)\vehicle_server.obj" \
	"$(INTDIR)\viewport.obj" \
	"$(INTDIR)\visible.obj" \
	"$(INTDIR)\VOTERS.OBJ" \
	"$(INTDIR)\wivfor.obj" \
	"$(INTDIR)\wivinv.obj" \
	"$(INTDIR)\Writeavi.obj" \
	"$(INTDIR)\wviifor.obj" \
	"$(INTDIR)\wviiinv.obj" \
	"$(INTDIR)\GWCONFIG.res" \
	"..\GM32lib\FreeImage.lib" \
	"..\MrSid\Geo_DSDK\lib\Release_md\lti_dsdk_cdll.lib" \
	"..\DGNlib7\Release\DGNlib7_32.lib"

"$(OUTDIR)\geomaster32u.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\geomaster.exe" "$(OUTDIR)\geomastr.bsc"


CLEAN :
	-@erase "$(INTDIR)\address1.obj"
	-@erase "$(INTDIR)\address1.sbr"
	-@erase "$(INTDIR)\address2.obj"
	-@erase "$(INTDIR)\address2.sbr"
	-@erase "$(INTDIR)\address3.obj"
	-@erase "$(INTDIR)\address3.sbr"
	-@erase "$(INTDIR)\AddressMsgProc.obj"
	-@erase "$(INTDIR)\AddressMsgProc.sbr"
	-@erase "$(INTDIR)\adjlon.obj"
	-@erase "$(INTDIR)\adjlon.sbr"
	-@erase "$(INTDIR)\alberfor.obj"
	-@erase "$(INTDIR)\alberfor.sbr"
	-@erase "$(INTDIR)\alberinv.obj"
	-@erase "$(INTDIR)\alberinv.sbr"
	-@erase "$(INTDIR)\alconfor.obj"
	-@erase "$(INTDIR)\alconfor.sbr"
	-@erase "$(INTDIR)\alconinv.obj"
	-@erase "$(INTDIR)\alconinv.sbr"
	-@erase "$(INTDIR)\azimfor.obj"
	-@erase "$(INTDIR)\azimfor.sbr"
	-@erase "$(INTDIR)\aziminv.obj"
	-@erase "$(INTDIR)\aziminv.sbr"
	-@erase "$(INTDIR)\bci.obj"
	-@erase "$(INTDIR)\bci.sbr"
	-@erase "$(INTDIR)\bigmem.obj"
	-@erase "$(INTDIR)\bigmem.sbr"
	-@erase "$(INTDIR)\br_gctp.obj"
	-@erase "$(INTDIR)\br_gctp.sbr"
	-@erase "$(INTDIR)\Bt_creat.obj"
	-@erase "$(INTDIR)\Bt_creat.sbr"
	-@erase "$(INTDIR)\ByteSwap.obj"
	-@erase "$(INTDIR)\ByteSwap.sbr"
	-@erase "$(INTDIR)\cdstuff.obj"
	-@erase "$(INTDIR)\cdstuff.sbr"
	-@erase "$(INTDIR)\CLIDATA.OBJ"
	-@erase "$(INTDIR)\CLIDATA.SBR"
	-@erase "$(INTDIR)\cmdtonum.obj"
	-@erase "$(INTDIR)\cmdtonum.sbr"
	-@erase "$(INTDIR)\COMDLG.obj"
	-@erase "$(INTDIR)\COMDLG.sbr"
	-@erase "$(INTDIR)\Comput1.obj"
	-@erase "$(INTDIR)\Comput1.sbr"
	-@erase "$(INTDIR)\CONNAD83.OBJ"
	-@erase "$(INTDIR)\CONNAD83.SBR"
	-@erase "$(INTDIR)\convert.obj"
	-@erase "$(INTDIR)\convert.sbr"
	-@erase "$(INTDIR)\CONVRT32.OBJ"
	-@erase "$(INTDIR)\CONVRT32.SBR"
	-@erase "$(INTDIR)\copydib.obj"
	-@erase "$(INTDIR)\copydib.sbr"
	-@erase "$(INTDIR)\cproj.obj"
	-@erase "$(INTDIR)\cproj.sbr"
	-@erase "$(INTDIR)\Creatcfg.obj"
	-@erase "$(INTDIR)\Creatcfg.sbr"
	-@erase "$(INTDIR)\CVTCOORD.OBJ"
	-@erase "$(INTDIR)\CVTCOORD.SBR"
	-@erase "$(INTDIR)\cwconfig.obj"
	-@erase "$(INTDIR)\cwconfig.sbr"
	-@erase "$(INTDIR)\dbfopen.obj"
	-@erase "$(INTDIR)\dbfopen.sbr"
	-@erase "$(INTDIR)\DDE.OBJ"
	-@erase "$(INTDIR)\DDE.SBR"
	-@erase "$(INTDIR)\DGNFILE.OBJ"
	-@erase "$(INTDIR)\DGNFILE.SBR"
	-@erase "$(INTDIR)\DIBUTIL.OBJ"
	-@erase "$(INTDIR)\DIBUTIL.SBR"
	-@erase "$(INTDIR)\DICT.OBJ"
	-@erase "$(INTDIR)\DICT.SBR"
	-@erase "$(INTDIR)\DICTEDIT.OBJ"
	-@erase "$(INTDIR)\DICTEDIT.SBR"
	-@erase "$(INTDIR)\digitize.obj"
	-@erase "$(INTDIR)\digitize.sbr"
	-@erase "$(INTDIR)\DLGLOOK.OBJ"
	-@erase "$(INTDIR)\DLGLOOK.SBR"
	-@erase "$(INTDIR)\Document.obj"
	-@erase "$(INTDIR)\Document.sbr"
	-@erase "$(INTDIR)\dtm.obj"
	-@erase "$(INTDIR)\dtm.sbr"
	-@erase "$(INTDIR)\DTM2.OBJ"
	-@erase "$(INTDIR)\DTM2.SBR"
	-@erase "$(INTDIR)\Dxfin.obj"
	-@erase "$(INTDIR)\Dxfin.sbr"
	-@erase "$(INTDIR)\dynaedit.obj"
	-@erase "$(INTDIR)\dynaedit.sbr"
	-@erase "$(INTDIR)\edgmatch.obj"
	-@erase "$(INTDIR)\edgmatch.sbr"
	-@erase "$(INTDIR)\eqconfor.obj"
	-@erase "$(INTDIR)\eqconfor.sbr"
	-@erase "$(INTDIR)\eqconinv.obj"
	-@erase "$(INTDIR)\eqconinv.sbr"
	-@erase "$(INTDIR)\equifor.obj"
	-@erase "$(INTDIR)\equifor.sbr"
	-@erase "$(INTDIR)\equiinv.obj"
	-@erase "$(INTDIR)\equiinv.sbr"
	-@erase "$(INTDIR)\exception.obj"
	-@erase "$(INTDIR)\exception.sbr"
	-@erase "$(INTDIR)\export.obj"
	-@erase "$(INTDIR)\export.sbr"
	-@erase "$(INTDIR)\EXPORT2.OBJ"
	-@erase "$(INTDIR)\EXPORT2.SBR"
	-@erase "$(INTDIR)\FIIO_Mem.obj"
	-@erase "$(INTDIR)\FIIO_Mem.sbr"
	-@erase "$(INTDIR)\FILE.OBJ"
	-@erase "$(INTDIR)\FILE.SBR"
	-@erase "$(INTDIR)\fish.obj"
	-@erase "$(INTDIR)\fish.sbr"
	-@erase "$(INTDIR)\floatap.obj"
	-@erase "$(INTDIR)\floatap.sbr"
	-@erase "$(INTDIR)\for_init.obj"
	-@erase "$(INTDIR)\for_init.sbr"
	-@erase "$(INTDIR)\freeim.obj"
	-@erase "$(INTDIR)\freeim.sbr"
	-@erase "$(INTDIR)\FUNCSUBS.obj"
	-@erase "$(INTDIR)\FUNCSUBS.sbr"
	-@erase "$(INTDIR)\function.obj"
	-@erase "$(INTDIR)\function.sbr"
	-@erase "$(INTDIR)\FUNCTN1.OBJ"
	-@erase "$(INTDIR)\FUNCTN1.SBR"
	-@erase "$(INTDIR)\functn2.obj"
	-@erase "$(INTDIR)\functn2.sbr"
	-@erase "$(INTDIR)\FUNCTN3.OBJ"
	-@erase "$(INTDIR)\FUNCTN3.SBR"
	-@erase "$(INTDIR)\FUNDEFS.OBJ"
	-@erase "$(INTDIR)\FUNDEFS.SBR"
	-@erase "$(INTDIR)\gctp.obj"
	-@erase "$(INTDIR)\gctp.sbr"
	-@erase "$(INTDIR)\GetSDChipID.obj"
	-@erase "$(INTDIR)\GetSDChipID.sbr"
	-@erase "$(INTDIR)\GM32.obj"
	-@erase "$(INTDIR)\GM32.sbr"
	-@erase "$(INTDIR)\GM32LIB.OBJ"
	-@erase "$(INTDIR)\GM32LIB.SBR"
	-@erase "$(INTDIR)\gnomfor.obj"
	-@erase "$(INTDIR)\gnomfor.sbr"
	-@erase "$(INTDIR)\gnominv.obj"
	-@erase "$(INTDIR)\gnominv.sbr"
	-@erase "$(INTDIR)\goodfor.obj"
	-@erase "$(INTDIR)\goodfor.sbr"
	-@erase "$(INTDIR)\goodinv.obj"
	-@erase "$(INTDIR)\goodinv.sbr"
	-@erase "$(INTDIR)\gps.obj"
	-@erase "$(INTDIR)\gps.sbr"
	-@erase "$(INTDIR)\graphic1.obj"
	-@erase "$(INTDIR)\graphic1.sbr"
	-@erase "$(INTDIR)\graphic2.obj"
	-@erase "$(INTDIR)\graphic2.sbr"
	-@erase "$(INTDIR)\Graphic3.obj"
	-@erase "$(INTDIR)\Graphic3.sbr"
	-@erase "$(INTDIR)\graphic4.obj"
	-@erase "$(INTDIR)\graphic4.sbr"
	-@erase "$(INTDIR)\GRAPHIC5.OBJ"
	-@erase "$(INTDIR)\GRAPHIC5.SBR"
	-@erase "$(INTDIR)\Graphic6.obj"
	-@erase "$(INTDIR)\Graphic6.sbr"
	-@erase "$(INTDIR)\graphic7.obj"
	-@erase "$(INTDIR)\graphic7.sbr"
	-@erase "$(INTDIR)\graphic8.obj"
	-@erase "$(INTDIR)\graphic8.sbr"
	-@erase "$(INTDIR)\GRAPHIC9.obj"
	-@erase "$(INTDIR)\GRAPHIC9.sbr"
	-@erase "$(INTDIR)\graphic_functions.obj"
	-@erase "$(INTDIR)\graphic_functions.sbr"
	-@erase "$(INTDIR)\graphica.obj"
	-@erase "$(INTDIR)\graphica.sbr"
	-@erase "$(INTDIR)\graphicb.obj"
	-@erase "$(INTDIR)\graphicb.sbr"
	-@erase "$(INTDIR)\graphicc.obj"
	-@erase "$(INTDIR)\graphicc.sbr"
	-@erase "$(INTDIR)\GRAPHICD.OBJ"
	-@erase "$(INTDIR)\GRAPHICD.SBR"
	-@erase "$(INTDIR)\GraphMsgProc.obj"
	-@erase "$(INTDIR)\GraphMsgProc.sbr"
	-@erase "$(INTDIR)\GraphMsgProc2.obj"
	-@erase "$(INTDIR)\GraphMsgProc2.sbr"
	-@erase "$(INTDIR)\grfunc.obj"
	-@erase "$(INTDIR)\grfunc.sbr"
	-@erase "$(INTDIR)\gspline.obj"
	-@erase "$(INTDIR)\gspline.sbr"
	-@erase "$(INTDIR)\gssigdi.obj"
	-@erase "$(INTDIR)\gssigdi.sbr"
	-@erase "$(INTDIR)\gvnspfor.obj"
	-@erase "$(INTDIR)\gvnspfor.sbr"
	-@erase "$(INTDIR)\gvnspinv.obj"
	-@erase "$(INTDIR)\gvnspinv.sbr"
	-@erase "$(INTDIR)\GWCONFIG.res"
	-@erase "$(INTDIR)\gwd.obj"
	-@erase "$(INTDIR)\gwd.sbr"
	-@erase "$(INTDIR)\hamfor.obj"
	-@erase "$(INTDIR)\hamfor.sbr"
	-@erase "$(INTDIR)\haminv.obj"
	-@erase "$(INTDIR)\haminv.sbr"
	-@erase "$(INTDIR)\HASH.OBJ"
	-@erase "$(INTDIR)\HASH.SBR"
	-@erase "$(INTDIR)\hbird.obj"
	-@erase "$(INTDIR)\hbird.sbr"
	-@erase "$(INTDIR)\hotspots.obj"
	-@erase "$(INTDIR)\hotspots.sbr"
	-@erase "$(INTDIR)\imolwfor.obj"
	-@erase "$(INTDIR)\imolwfor.sbr"
	-@erase "$(INTDIR)\imolwinv.obj"
	-@erase "$(INTDIR)\imolwinv.sbr"
	-@erase "$(INTDIR)\Infobox.obj"
	-@erase "$(INTDIR)\Infobox.sbr"
	-@erase "$(INTDIR)\inv_init.obj"
	-@erase "$(INTDIR)\inv_init.sbr"
	-@erase "$(INTDIR)\lamazfor.obj"
	-@erase "$(INTDIR)\lamazfor.sbr"
	-@erase "$(INTDIR)\lamazinv.obj"
	-@erase "$(INTDIR)\lamazinv.sbr"
	-@erase "$(INTDIR)\lamccfor.obj"
	-@erase "$(INTDIR)\lamccfor.sbr"
	-@erase "$(INTDIR)\lamccinv.obj"
	-@erase "$(INTDIR)\lamccinv.sbr"
	-@erase "$(INTDIR)\license.obj"
	-@erase "$(INTDIR)\license.sbr"
	-@erase "$(INTDIR)\LkmToHbirdImage.obj"
	-@erase "$(INTDIR)\LkmToHbirdImage.sbr"
	-@erase "$(INTDIR)\makearea.obj"
	-@erase "$(INTDIR)\makearea.sbr"
	-@erase "$(INTDIR)\mapcopy.obj"
	-@erase "$(INTDIR)\mapcopy.sbr"
	-@erase "$(INTDIR)\memalloc.obj"
	-@erase "$(INTDIR)\memalloc.sbr"
	-@erase "$(INTDIR)\merfor.obj"
	-@erase "$(INTDIR)\merfor.sbr"
	-@erase "$(INTDIR)\merinv.obj"
	-@erase "$(INTDIR)\merinv.sbr"
	-@erase "$(INTDIR)\mgrs.obj"
	-@erase "$(INTDIR)\mgrs.sbr"
	-@erase "$(INTDIR)\millfor.obj"
	-@erase "$(INTDIR)\millfor.sbr"
	-@erase "$(INTDIR)\millinv.obj"
	-@erase "$(INTDIR)\millinv.sbr"
	-@erase "$(INTDIR)\minilzo.obj"
	-@erase "$(INTDIR)\minilzo.sbr"
	-@erase "$(INTDIR)\molwfor.obj"
	-@erase "$(INTDIR)\molwfor.sbr"
	-@erase "$(INTDIR)\molwinv.obj"
	-@erase "$(INTDIR)\molwinv.sbr"
	-@erase "$(INTDIR)\mrsid.obj"
	-@erase "$(INTDIR)\mrsid.sbr"
	-@erase "$(INTDIR)\MS_MAPL.OBJ"
	-@erase "$(INTDIR)\MS_MAPL.SBR"
	-@erase "$(INTDIR)\nad_cvt.obj"
	-@erase "$(INTDIR)\nad_cvt.sbr"
	-@erase "$(INTDIR)\nad_init.obj"
	-@erase "$(INTDIR)\nad_init.sbr"
	-@erase "$(INTDIR)\nad_intr.obj"
	-@erase "$(INTDIR)\nad_intr.sbr"
	-@erase "$(INTDIR)\nadcon.obj"
	-@erase "$(INTDIR)\nadcon.sbr"
	-@erase "$(INTDIR)\NETWORKS.obj"
	-@erase "$(INTDIR)\NETWORKS.sbr"
	-@erase "$(INTDIR)\obleqfor.obj"
	-@erase "$(INTDIR)\obleqfor.sbr"
	-@erase "$(INTDIR)\obleqinv.obj"
	-@erase "$(INTDIR)\obleqinv.sbr"
	-@erase "$(INTDIR)\Odbcmain.obj"
	-@erase "$(INTDIR)\Odbcmain.sbr"
	-@erase "$(INTDIR)\Offset.obj"
	-@erase "$(INTDIR)\Offset.sbr"
	-@erase "$(INTDIR)\omerfor.obj"
	-@erase "$(INTDIR)\omerfor.sbr"
	-@erase "$(INTDIR)\omerinv.obj"
	-@erase "$(INTDIR)\omerinv.sbr"
	-@erase "$(INTDIR)\ORAFILE.OBJ"
	-@erase "$(INTDIR)\ORAFILE.SBR"
	-@erase "$(INTDIR)\orthfor.obj"
	-@erase "$(INTDIR)\orthfor.sbr"
	-@erase "$(INTDIR)\orthinv.obj"
	-@erase "$(INTDIR)\orthinv.sbr"
	-@erase "$(INTDIR)\orthos.obj"
	-@erase "$(INTDIR)\orthos.sbr"
	-@erase "$(INTDIR)\paksz.obj"
	-@erase "$(INTDIR)\paksz.sbr"
	-@erase "$(INTDIR)\PCXTOBMP.OBJ"
	-@erase "$(INTDIR)\PCXTOBMP.SBR"
	-@erase "$(INTDIR)\peoplnet.obj"
	-@erase "$(INTDIR)\peoplnet.sbr"
	-@erase "$(INTDIR)\pj_errno.obj"
	-@erase "$(INTDIR)\pj_errno.sbr"
	-@erase "$(INTDIR)\pj_malloc.obj"
	-@erase "$(INTDIR)\pj_malloc.sbr"
	-@erase "$(INTDIR)\pj_open_lib.obj"
	-@erase "$(INTDIR)\pj_open_lib.sbr"
	-@erase "$(INTDIR)\pngrid.obj"
	-@erase "$(INTDIR)\pngrid.sbr"
	-@erase "$(INTDIR)\pointlist.obj"
	-@erase "$(INTDIR)\pointlist.sbr"
	-@erase "$(INTDIR)\polarst.obj"
	-@erase "$(INTDIR)\polarst.sbr"
	-@erase "$(INTDIR)\POLY.OBJ"
	-@erase "$(INTDIR)\POLY.SBR"
	-@erase "$(INTDIR)\polyfor.obj"
	-@erase "$(INTDIR)\polyfor.sbr"
	-@erase "$(INTDIR)\polyinv.obj"
	-@erase "$(INTDIR)\polyinv.sbr"
	-@erase "$(INTDIR)\Print.obj"
	-@erase "$(INTDIR)\Print.sbr"
	-@erase "$(INTDIR)\PROFILE.OBJ"
	-@erase "$(INTDIR)\PROFILE.SBR"
	-@erase "$(INTDIR)\psfor.obj"
	-@erase "$(INTDIR)\psfor.sbr"
	-@erase "$(INTDIR)\psinv.obj"
	-@erase "$(INTDIR)\psinv.sbr"
	-@erase "$(INTDIR)\quad.obj"
	-@erase "$(INTDIR)\quad.sbr"
	-@erase "$(INTDIR)\report.obj"
	-@erase "$(INTDIR)\report.sbr"
	-@erase "$(INTDIR)\reportgc.obj"
	-@erase "$(INTDIR)\reportgc.sbr"
	-@erase "$(INTDIR)\ROADNAME.OBJ"
	-@erase "$(INTDIR)\ROADNAME.SBR"
	-@erase "$(INTDIR)\robfor.obj"
	-@erase "$(INTDIR)\robfor.sbr"
	-@erase "$(INTDIR)\robinv.obj"
	-@erase "$(INTDIR)\robinv.sbr"
	-@erase "$(INTDIR)\route.obj"
	-@erase "$(INTDIR)\route.sbr"
	-@erase "$(INTDIR)\serio.obj"
	-@erase "$(INTDIR)\serio.sbr"
	-@erase "$(INTDIR)\SHPFILE.OBJ"
	-@erase "$(INTDIR)\SHPFILE.SBR"
	-@erase "$(INTDIR)\shr.obj"
	-@erase "$(INTDIR)\shr.sbr"
	-@erase "$(INTDIR)\signinv.obj"
	-@erase "$(INTDIR)\signinv.sbr"
	-@erase "$(INTDIR)\sinfor.obj"
	-@erase "$(INTDIR)\sinfor.sbr"
	-@erase "$(INTDIR)\sininv.obj"
	-@erase "$(INTDIR)\sininv.sbr"
	-@erase "$(INTDIR)\smooth.obj"
	-@erase "$(INTDIR)\smooth.sbr"
	-@erase "$(INTDIR)\somfor.obj"
	-@erase "$(INTDIR)\somfor.sbr"
	-@erase "$(INTDIR)\sominv.obj"
	-@erase "$(INTDIR)\sominv.sbr"
	-@erase "$(INTDIR)\sphdz.obj"
	-@erase "$(INTDIR)\sphdz.sbr"
	-@erase "$(INTDIR)\ssp.obj"
	-@erase "$(INTDIR)\ssp.sbr"
	-@erase "$(INTDIR)\STD.OBJ"
	-@erase "$(INTDIR)\STD.SBR"
	-@erase "$(INTDIR)\sterfor.obj"
	-@erase "$(INTDIR)\sterfor.sbr"
	-@erase "$(INTDIR)\sterinv.obj"
	-@erase "$(INTDIR)\sterinv.sbr"
	-@erase "$(INTDIR)\stplnfor.obj"
	-@erase "$(INTDIR)\stplnfor.sbr"
	-@erase "$(INTDIR)\stplninv.obj"
	-@erase "$(INTDIR)\stplninv.sbr"
	-@erase "$(INTDIR)\T2PROJ~1.OBJ"
	-@erase "$(INTDIR)\T2PROJ~1.SBR"
	-@erase "$(INTDIR)\tdspl.obj"
	-@erase "$(INTDIR)\tdspl.sbr"
	-@erase "$(INTDIR)\testMemIO.obj"
	-@erase "$(INTDIR)\testMemIO.sbr"
	-@erase "$(INTDIR)\themes.obj"
	-@erase "$(INTDIR)\themes.sbr"
	-@erase "$(INTDIR)\themes2.obj"
	-@erase "$(INTDIR)\themes2.sbr"
	-@erase "$(INTDIR)\THEMES3.OBJ"
	-@erase "$(INTDIR)\THEMES3.SBR"
	-@erase "$(INTDIR)\THMMSGP.OBJ"
	-@erase "$(INTDIR)\THMMSGP.SBR"
	-@erase "$(INTDIR)\tiff.obj"
	-@erase "$(INTDIR)\tiff.sbr"
	-@erase "$(INTDIR)\tmfor.obj"
	-@erase "$(INTDIR)\tmfor.sbr"
	-@erase "$(INTDIR)\tminv.obj"
	-@erase "$(INTDIR)\tminv.sbr"
	-@erase "$(INTDIR)\toolbar.obj"
	-@erase "$(INTDIR)\toolbar.sbr"
	-@erase "$(INTDIR)\tools.obj"
	-@erase "$(INTDIR)\tools.sbr"
	-@erase "$(INTDIR)\Tran.obj"
	-@erase "$(INTDIR)\Tran.sbr"
	-@erase "$(INTDIR)\tranlat2.obj"
	-@erase "$(INTDIR)\tranlat2.sbr"
	-@erase "$(INTDIR)\tranlat3.obj"
	-@erase "$(INTDIR)\tranlat3.sbr"
	-@erase "$(INTDIR)\tranmerc.obj"
	-@erase "$(INTDIR)\tranmerc.sbr"
	-@erase "$(INTDIR)\TRANPR~1.OBJ"
	-@erase "$(INTDIR)\TRANPR~1.SBR"
	-@erase "$(INTDIR)\translat.obj"
	-@erase "$(INTDIR)\translat.sbr"
	-@erase "$(INTDIR)\traverse.obj"
	-@erase "$(INTDIR)\traverse.sbr"
	-@erase "$(INTDIR)\UGRID.OBJ"
	-@erase "$(INTDIR)\UGRID.SBR"
	-@erase "$(INTDIR)\Ugridhlt.obj"
	-@erase "$(INTDIR)\Ugridhlt.sbr"
	-@erase "$(INTDIR)\Umio.obj"
	-@erase "$(INTDIR)\Umio.sbr"
	-@erase "$(INTDIR)\UNDO.OBJ"
	-@erase "$(INTDIR)\UNDO.SBR"
	-@erase "$(INTDIR)\untfz.obj"
	-@erase "$(INTDIR)\untfz.sbr"
	-@erase "$(INTDIR)\ups.obj"
	-@erase "$(INTDIR)\ups.sbr"
	-@erase "$(INTDIR)\USBSDK.obj"
	-@erase "$(INTDIR)\USBSDK.sbr"
	-@erase "$(INTDIR)\utm.obj"
	-@erase "$(INTDIR)\utm.sbr"
	-@erase "$(INTDIR)\utmfor.obj"
	-@erase "$(INTDIR)\utmfor.sbr"
	-@erase "$(INTDIR)\utminv.obj"
	-@erase "$(INTDIR)\utminv.sbr"
	-@erase "$(INTDIR)\vandgfor.obj"
	-@erase "$(INTDIR)\vandgfor.sbr"
	-@erase "$(INTDIR)\vandginv.obj"
	-@erase "$(INTDIR)\vandginv.sbr"
	-@erase "$(INTDIR)\VARDEF.OBJ"
	-@erase "$(INTDIR)\VARDEF.SBR"
	-@erase "$(INTDIR)\VARSTUF2.OBJ"
	-@erase "$(INTDIR)\VARSTUF2.SBR"
	-@erase "$(INTDIR)\VARSTUFF.OBJ"
	-@erase "$(INTDIR)\VARSTUFF.SBR"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\vehicle.obj"
	-@erase "$(INTDIR)\vehicle.sbr"
	-@erase "$(INTDIR)\vehicle_server.obj"
	-@erase "$(INTDIR)\vehicle_server.sbr"
	-@erase "$(INTDIR)\viewport.obj"
	-@erase "$(INTDIR)\viewport.sbr"
	-@erase "$(INTDIR)\visible.obj"
	-@erase "$(INTDIR)\visible.sbr"
	-@erase "$(INTDIR)\VOTERS.OBJ"
	-@erase "$(INTDIR)\VOTERS.SBR"
	-@erase "$(INTDIR)\wivfor.obj"
	-@erase "$(INTDIR)\wivfor.sbr"
	-@erase "$(INTDIR)\wivinv.obj"
	-@erase "$(INTDIR)\wivinv.sbr"
	-@erase "$(INTDIR)\Writeavi.obj"
	-@erase "$(INTDIR)\Writeavi.sbr"
	-@erase "$(INTDIR)\wviifor.obj"
	-@erase "$(INTDIR)\wviifor.sbr"
	-@erase "$(INTDIR)\wviiinv.obj"
	-@erase "$(INTDIR)\wviiinv.sbr"
	-@erase "$(OUTDIR)\geomaster.exe"
	-@erase "$(OUTDIR)\geomaster.map"
	-@erase "$(OUTDIR)\geomastr.bsc"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /Zp2 /ML /w /W0 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "CHECKMEM" /D "NO_STRICT" /D "XPORABOVE" /D "USEMRSID" /D _WIN32_WINNT=0x0502 /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\GWCONFIG.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/o"$(OUTDIR)\geomastr.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\address1.sbr" \
	"$(INTDIR)\address2.sbr" \
	"$(INTDIR)\address3.sbr" \
	"$(INTDIR)\AddressMsgProc.sbr" \
	"$(INTDIR)\adjlon.sbr" \
	"$(INTDIR)\alberfor.sbr" \
	"$(INTDIR)\alberinv.sbr" \
	"$(INTDIR)\alconfor.sbr" \
	"$(INTDIR)\alconinv.sbr" \
	"$(INTDIR)\azimfor.sbr" \
	"$(INTDIR)\aziminv.sbr" \
	"$(INTDIR)\bci.sbr" \
	"$(INTDIR)\bigmem.sbr" \
	"$(INTDIR)\br_gctp.sbr" \
	"$(INTDIR)\Bt_creat.sbr" \
	"$(INTDIR)\ByteSwap.sbr" \
	"$(INTDIR)\cdstuff.sbr" \
	"$(INTDIR)\CLIDATA.SBR" \
	"$(INTDIR)\cmdtonum.sbr" \
	"$(INTDIR)\COMDLG.sbr" \
	"$(INTDIR)\Comput1.sbr" \
	"$(INTDIR)\CONNAD83.SBR" \
	"$(INTDIR)\convert.sbr" \
	"$(INTDIR)\CONVRT32.SBR" \
	"$(INTDIR)\copydib.sbr" \
	"$(INTDIR)\cproj.sbr" \
	"$(INTDIR)\Creatcfg.sbr" \
	"$(INTDIR)\CVTCOORD.SBR" \
	"$(INTDIR)\cwconfig.sbr" \
	"$(INTDIR)\dbfopen.sbr" \
	"$(INTDIR)\DDE.SBR" \
	"$(INTDIR)\DGNFILE.SBR" \
	"$(INTDIR)\DIBUTIL.SBR" \
	"$(INTDIR)\DICT.SBR" \
	"$(INTDIR)\DICTEDIT.SBR" \
	"$(INTDIR)\digitize.sbr" \
	"$(INTDIR)\DLGLOOK.SBR" \
	"$(INTDIR)\Document.sbr" \
	"$(INTDIR)\dtm.sbr" \
	"$(INTDIR)\DTM2.SBR" \
	"$(INTDIR)\Dxfin.sbr" \
	"$(INTDIR)\dynaedit.sbr" \
	"$(INTDIR)\edgmatch.sbr" \
	"$(INTDIR)\eqconfor.sbr" \
	"$(INTDIR)\eqconinv.sbr" \
	"$(INTDIR)\equifor.sbr" \
	"$(INTDIR)\equiinv.sbr" \
	"$(INTDIR)\exception.sbr" \
	"$(INTDIR)\export.sbr" \
	"$(INTDIR)\EXPORT2.SBR" \
	"$(INTDIR)\FIIO_Mem.sbr" \
	"$(INTDIR)\FILE.SBR" \
	"$(INTDIR)\fish.sbr" \
	"$(INTDIR)\floatap.sbr" \
	"$(INTDIR)\for_init.sbr" \
	"$(INTDIR)\freeim.sbr" \
	"$(INTDIR)\FUNCSUBS.sbr" \
	"$(INTDIR)\function.sbr" \
	"$(INTDIR)\FUNCTN1.SBR" \
	"$(INTDIR)\functn2.sbr" \
	"$(INTDIR)\FUNCTN3.SBR" \
	"$(INTDIR)\FUNDEFS.SBR" \
	"$(INTDIR)\gctp.sbr" \
	"$(INTDIR)\GetSDChipID.sbr" \
	"$(INTDIR)\GM32.sbr" \
	"$(INTDIR)\GM32LIB.SBR" \
	"$(INTDIR)\gnomfor.sbr" \
	"$(INTDIR)\gnominv.sbr" \
	"$(INTDIR)\goodfor.sbr" \
	"$(INTDIR)\goodinv.sbr" \
	"$(INTDIR)\gps.sbr" \
	"$(INTDIR)\graphic1.sbr" \
	"$(INTDIR)\graphic2.sbr" \
	"$(INTDIR)\Graphic3.sbr" \
	"$(INTDIR)\graphic4.sbr" \
	"$(INTDIR)\GRAPHIC5.SBR" \
	"$(INTDIR)\Graphic6.sbr" \
	"$(INTDIR)\graphic7.sbr" \
	"$(INTDIR)\graphic8.sbr" \
	"$(INTDIR)\GRAPHIC9.sbr" \
	"$(INTDIR)\graphic_functions.sbr" \
	"$(INTDIR)\graphica.sbr" \
	"$(INTDIR)\graphicb.sbr" \
	"$(INTDIR)\graphicc.sbr" \
	"$(INTDIR)\GRAPHICD.SBR" \
	"$(INTDIR)\GraphMsgProc.sbr" \
	"$(INTDIR)\GraphMsgProc2.sbr" \
	"$(INTDIR)\grfunc.sbr" \
	"$(INTDIR)\gspline.sbr" \
	"$(INTDIR)\gssigdi.sbr" \
	"$(INTDIR)\gvnspfor.sbr" \
	"$(INTDIR)\gvnspinv.sbr" \
	"$(INTDIR)\gwd.sbr" \
	"$(INTDIR)\hamfor.sbr" \
	"$(INTDIR)\haminv.sbr" \
	"$(INTDIR)\HASH.SBR" \
	"$(INTDIR)\hbird.sbr" \
	"$(INTDIR)\hotspots.sbr" \
	"$(INTDIR)\imolwfor.sbr" \
	"$(INTDIR)\imolwinv.sbr" \
	"$(INTDIR)\Infobox.sbr" \
	"$(INTDIR)\inv_init.sbr" \
	"$(INTDIR)\lamazfor.sbr" \
	"$(INTDIR)\lamazinv.sbr" \
	"$(INTDIR)\lamccfor.sbr" \
	"$(INTDIR)\lamccinv.sbr" \
	"$(INTDIR)\license.sbr" \
	"$(INTDIR)\LkmToHbirdImage.sbr" \
	"$(INTDIR)\makearea.sbr" \
	"$(INTDIR)\mapcopy.sbr" \
	"$(INTDIR)\memalloc.sbr" \
	"$(INTDIR)\merfor.sbr" \
	"$(INTDIR)\merinv.sbr" \
	"$(INTDIR)\mgrs.sbr" \
	"$(INTDIR)\millfor.sbr" \
	"$(INTDIR)\millinv.sbr" \
	"$(INTDIR)\minilzo.sbr" \
	"$(INTDIR)\molwfor.sbr" \
	"$(INTDIR)\molwinv.sbr" \
	"$(INTDIR)\mrsid.sbr" \
	"$(INTDIR)\MS_MAPL.SBR" \
	"$(INTDIR)\nad_cvt.sbr" \
	"$(INTDIR)\nad_init.sbr" \
	"$(INTDIR)\nad_intr.sbr" \
	"$(INTDIR)\nadcon.sbr" \
	"$(INTDIR)\NETWORKS.sbr" \
	"$(INTDIR)\obleqfor.sbr" \
	"$(INTDIR)\obleqinv.sbr" \
	"$(INTDIR)\Odbcmain.sbr" \
	"$(INTDIR)\Offset.sbr" \
	"$(INTDIR)\omerfor.sbr" \
	"$(INTDIR)\omerinv.sbr" \
	"$(INTDIR)\ORAFILE.SBR" \
	"$(INTDIR)\orthfor.sbr" \
	"$(INTDIR)\orthinv.sbr" \
	"$(INTDIR)\orthos.sbr" \
	"$(INTDIR)\paksz.sbr" \
	"$(INTDIR)\PCXTOBMP.SBR" \
	"$(INTDIR)\peoplnet.sbr" \
	"$(INTDIR)\pj_errno.sbr" \
	"$(INTDIR)\pj_malloc.sbr" \
	"$(INTDIR)\pj_open_lib.sbr" \
	"$(INTDIR)\pngrid.sbr" \
	"$(INTDIR)\pointlist.sbr" \
	"$(INTDIR)\polarst.sbr" \
	"$(INTDIR)\POLY.SBR" \
	"$(INTDIR)\polyfor.sbr" \
	"$(INTDIR)\polyinv.sbr" \
	"$(INTDIR)\Print.sbr" \
	"$(INTDIR)\PROFILE.SBR" \
	"$(INTDIR)\psfor.sbr" \
	"$(INTDIR)\psinv.sbr" \
	"$(INTDIR)\quad.sbr" \
	"$(INTDIR)\report.sbr" \
	"$(INTDIR)\reportgc.sbr" \
	"$(INTDIR)\ROADNAME.SBR" \
	"$(INTDIR)\robfor.sbr" \
	"$(INTDIR)\robinv.sbr" \
	"$(INTDIR)\route.sbr" \
	"$(INTDIR)\serio.sbr" \
	"$(INTDIR)\SHPFILE.SBR" \
	"$(INTDIR)\shr.sbr" \
	"$(INTDIR)\signinv.sbr" \
	"$(INTDIR)\sinfor.sbr" \
	"$(INTDIR)\sininv.sbr" \
	"$(INTDIR)\smooth.sbr" \
	"$(INTDIR)\somfor.sbr" \
	"$(INTDIR)\sominv.sbr" \
	"$(INTDIR)\sphdz.sbr" \
	"$(INTDIR)\ssp.sbr" \
	"$(INTDIR)\STD.SBR" \
	"$(INTDIR)\sterfor.sbr" \
	"$(INTDIR)\sterinv.sbr" \
	"$(INTDIR)\stplnfor.sbr" \
	"$(INTDIR)\stplninv.sbr" \
	"$(INTDIR)\T2PROJ~1.SBR" \
	"$(INTDIR)\tdspl.sbr" \
	"$(INTDIR)\testMemIO.sbr" \
	"$(INTDIR)\themes.sbr" \
	"$(INTDIR)\themes2.sbr" \
	"$(INTDIR)\THEMES3.SBR" \
	"$(INTDIR)\THMMSGP.SBR" \
	"$(INTDIR)\tiff.sbr" \
	"$(INTDIR)\tmfor.sbr" \
	"$(INTDIR)\tminv.sbr" \
	"$(INTDIR)\toolbar.sbr" \
	"$(INTDIR)\tools.sbr" \
	"$(INTDIR)\Tran.sbr" \
	"$(INTDIR)\tranlat2.sbr" \
	"$(INTDIR)\tranlat3.sbr" \
	"$(INTDIR)\tranmerc.sbr" \
	"$(INTDIR)\TRANPR~1.SBR" \
	"$(INTDIR)\translat.sbr" \
	"$(INTDIR)\traverse.sbr" \
	"$(INTDIR)\UGRID.SBR" \
	"$(INTDIR)\Ugridhlt.sbr" \
	"$(INTDIR)\Umio.sbr" \
	"$(INTDIR)\UNDO.SBR" \
	"$(INTDIR)\untfz.sbr" \
	"$(INTDIR)\ups.sbr" \
	"$(INTDIR)\USBSDK.sbr" \
	"$(INTDIR)\utm.sbr" \
	"$(INTDIR)\utmfor.sbr" \
	"$(INTDIR)\utminv.sbr" \
	"$(INTDIR)\vandgfor.sbr" \
	"$(INTDIR)\vandginv.sbr" \
	"$(INTDIR)\VARDEF.SBR" \
	"$(INTDIR)\VARSTUF2.SBR" \
	"$(INTDIR)\VARSTUFF.SBR" \
	"$(INTDIR)\vehicle.sbr" \
	"$(INTDIR)\vehicle_server.sbr" \
	"$(INTDIR)\viewport.sbr" \
	"$(INTDIR)\visible.sbr" \
	"$(INTDIR)\VOTERS.SBR" \
	"$(INTDIR)\wivfor.sbr" \
	"$(INTDIR)\wivinv.sbr" \
	"$(INTDIR)\Writeavi.sbr" \
	"$(INTDIR)\wviifor.sbr" \
	"$(INTDIR)\wviiinv.sbr"

"$(OUTDIR)\geomastr.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib Ws2_32.lib vfw32.lib winmm.lib urlmon.lib setupapi.lib mpr.lib comctl32.lib /nologo /stack:0x400000 /subsystem:windows /pdb:none /map:"$(INTDIR)\geomaster.map" /debug /machine:I386 /def:".\GWCONFIG.DEF" /out:"$(OUTDIR)\geomaster.exe" /libpath:"c:\gssi\prog\lib" 
DEF_FILE= \
	".\GWCONFIG.DEF"
LINK32_OBJS= \
	"$(INTDIR)\address1.obj" \
	"$(INTDIR)\address2.obj" \
	"$(INTDIR)\address3.obj" \
	"$(INTDIR)\AddressMsgProc.obj" \
	"$(INTDIR)\adjlon.obj" \
	"$(INTDIR)\alberfor.obj" \
	"$(INTDIR)\alberinv.obj" \
	"$(INTDIR)\alconfor.obj" \
	"$(INTDIR)\alconinv.obj" \
	"$(INTDIR)\azimfor.obj" \
	"$(INTDIR)\aziminv.obj" \
	"$(INTDIR)\bci.obj" \
	"$(INTDIR)\bigmem.obj" \
	"$(INTDIR)\br_gctp.obj" \
	"$(INTDIR)\Bt_creat.obj" \
	"$(INTDIR)\ByteSwap.obj" \
	"$(INTDIR)\cdstuff.obj" \
	"$(INTDIR)\CLIDATA.OBJ" \
	"$(INTDIR)\cmdtonum.obj" \
	"$(INTDIR)\COMDLG.obj" \
	"$(INTDIR)\Comput1.obj" \
	"$(INTDIR)\CONNAD83.OBJ" \
	"$(INTDIR)\convert.obj" \
	"$(INTDIR)\CONVRT32.OBJ" \
	"$(INTDIR)\copydib.obj" \
	"$(INTDIR)\cproj.obj" \
	"$(INTDIR)\Creatcfg.obj" \
	"$(INTDIR)\CVTCOORD.OBJ" \
	"$(INTDIR)\cwconfig.obj" \
	"$(INTDIR)\dbfopen.obj" \
	"$(INTDIR)\DDE.OBJ" \
	"$(INTDIR)\DGNFILE.OBJ" \
	"$(INTDIR)\DIBUTIL.OBJ" \
	"$(INTDIR)\DICT.OBJ" \
	"$(INTDIR)\DICTEDIT.OBJ" \
	"$(INTDIR)\digitize.obj" \
	"$(INTDIR)\DLGLOOK.OBJ" \
	"$(INTDIR)\Document.obj" \
	"$(INTDIR)\dtm.obj" \
	"$(INTDIR)\DTM2.OBJ" \
	"$(INTDIR)\Dxfin.obj" \
	"$(INTDIR)\dynaedit.obj" \
	"$(INTDIR)\edgmatch.obj" \
	"$(INTDIR)\eqconfor.obj" \
	"$(INTDIR)\eqconinv.obj" \
	"$(INTDIR)\equifor.obj" \
	"$(INTDIR)\equiinv.obj" \
	"$(INTDIR)\exception.obj" \
	"$(INTDIR)\export.obj" \
	"$(INTDIR)\EXPORT2.OBJ" \
	"$(INTDIR)\FIIO_Mem.obj" \
	"$(INTDIR)\FILE.OBJ" \
	"$(INTDIR)\fish.obj" \
	"$(INTDIR)\floatap.obj" \
	"$(INTDIR)\for_init.obj" \
	"$(INTDIR)\freeim.obj" \
	"$(INTDIR)\FUNCSUBS.obj" \
	"$(INTDIR)\function.obj" \
	"$(INTDIR)\FUNCTN1.OBJ" \
	"$(INTDIR)\functn2.obj" \
	"$(INTDIR)\FUNCTN3.OBJ" \
	"$(INTDIR)\FUNDEFS.OBJ" \
	"$(INTDIR)\gctp.obj" \
	"$(INTDIR)\GetSDChipID.obj" \
	"$(INTDIR)\GM32.obj" \
	"$(INTDIR)\GM32LIB.OBJ" \
	"$(INTDIR)\gnomfor.obj" \
	"$(INTDIR)\gnominv.obj" \
	"$(INTDIR)\goodfor.obj" \
	"$(INTDIR)\goodinv.obj" \
	"$(INTDIR)\gps.obj" \
	"$(INTDIR)\graphic1.obj" \
	"$(INTDIR)\graphic2.obj" \
	"$(INTDIR)\Graphic3.obj" \
	"$(INTDIR)\graphic4.obj" \
	"$(INTDIR)\GRAPHIC5.OBJ" \
	"$(INTDIR)\Graphic6.obj" \
	"$(INTDIR)\graphic7.obj" \
	"$(INTDIR)\graphic8.obj" \
	"$(INTDIR)\GRAPHIC9.obj" \
	"$(INTDIR)\graphic_functions.obj" \
	"$(INTDIR)\graphica.obj" \
	"$(INTDIR)\graphicb.obj" \
	"$(INTDIR)\graphicc.obj" \
	"$(INTDIR)\GRAPHICD.OBJ" \
	"$(INTDIR)\GraphMsgProc.obj" \
	"$(INTDIR)\GraphMsgProc2.obj" \
	"$(INTDIR)\grfunc.obj" \
	"$(INTDIR)\gspline.obj" \
	"$(INTDIR)\gssigdi.obj" \
	"$(INTDIR)\gvnspfor.obj" \
	"$(INTDIR)\gvnspinv.obj" \
	"$(INTDIR)\gwd.obj" \
	"$(INTDIR)\hamfor.obj" \
	"$(INTDIR)\haminv.obj" \
	"$(INTDIR)\HASH.OBJ" \
	"$(INTDIR)\hbird.obj" \
	"$(INTDIR)\hotspots.obj" \
	"$(INTDIR)\imolwfor.obj" \
	"$(INTDIR)\imolwinv.obj" \
	"$(INTDIR)\Infobox.obj" \
	"$(INTDIR)\inv_init.obj" \
	"$(INTDIR)\lamazfor.obj" \
	"$(INTDIR)\lamazinv.obj" \
	"$(INTDIR)\lamccfor.obj" \
	"$(INTDIR)\lamccinv.obj" \
	"$(INTDIR)\license.obj" \
	"$(INTDIR)\LkmToHbirdImage.obj" \
	"$(INTDIR)\makearea.obj" \
	"$(INTDIR)\mapcopy.obj" \
	"$(INTDIR)\memalloc.obj" \
	"$(INTDIR)\merfor.obj" \
	"$(INTDIR)\merinv.obj" \
	"$(INTDIR)\mgrs.obj" \
	"$(INTDIR)\millfor.obj" \
	"$(INTDIR)\millinv.obj" \
	"$(INTDIR)\minilzo.obj" \
	"$(INTDIR)\molwfor.obj" \
	"$(INTDIR)\molwinv.obj" \
	"$(INTDIR)\mrsid.obj" \
	"$(INTDIR)\MS_MAPL.OBJ" \
	"$(INTDIR)\nad_cvt.obj" \
	"$(INTDIR)\nad_init.obj" \
	"$(INTDIR)\nad_intr.obj" \
	"$(INTDIR)\nadcon.obj" \
	"$(INTDIR)\NETWORKS.obj" \
	"$(INTDIR)\obleqfor.obj" \
	"$(INTDIR)\obleqinv.obj" \
	"$(INTDIR)\Odbcmain.obj" \
	"$(INTDIR)\Offset.obj" \
	"$(INTDIR)\omerfor.obj" \
	"$(INTDIR)\omerinv.obj" \
	"$(INTDIR)\ORAFILE.OBJ" \
	"$(INTDIR)\orthfor.obj" \
	"$(INTDIR)\orthinv.obj" \
	"$(INTDIR)\orthos.obj" \
	"$(INTDIR)\paksz.obj" \
	"$(INTDIR)\PCXTOBMP.OBJ" \
	"$(INTDIR)\peoplnet.obj" \
	"$(INTDIR)\pj_errno.obj" \
	"$(INTDIR)\pj_malloc.obj" \
	"$(INTDIR)\pj_open_lib.obj" \
	"$(INTDIR)\pngrid.obj" \
	"$(INTDIR)\pointlist.obj" \
	"$(INTDIR)\polarst.obj" \
	"$(INTDIR)\POLY.OBJ" \
	"$(INTDIR)\polyfor.obj" \
	"$(INTDIR)\polyinv.obj" \
	"$(INTDIR)\Print.obj" \
	"$(INTDIR)\PROFILE.OBJ" \
	"$(INTDIR)\psfor.obj" \
	"$(INTDIR)\psinv.obj" \
	"$(INTDIR)\quad.obj" \
	"$(INTDIR)\report.obj" \
	"$(INTDIR)\reportgc.obj" \
	"$(INTDIR)\ROADNAME.OBJ" \
	"$(INTDIR)\robfor.obj" \
	"$(INTDIR)\robinv.obj" \
	"$(INTDIR)\route.obj" \
	"$(INTDIR)\serio.obj" \
	"$(INTDIR)\SHPFILE.OBJ" \
	"$(INTDIR)\shr.obj" \
	"$(INTDIR)\signinv.obj" \
	"$(INTDIR)\sinfor.obj" \
	"$(INTDIR)\sininv.obj" \
	"$(INTDIR)\smooth.obj" \
	"$(INTDIR)\somfor.obj" \
	"$(INTDIR)\sominv.obj" \
	"$(INTDIR)\sphdz.obj" \
	"$(INTDIR)\ssp.obj" \
	"$(INTDIR)\STD.OBJ" \
	"$(INTDIR)\sterfor.obj" \
	"$(INTDIR)\sterinv.obj" \
	"$(INTDIR)\stplnfor.obj" \
	"$(INTDIR)\stplninv.obj" \
	"$(INTDIR)\T2PROJ~1.OBJ" \
	"$(INTDIR)\tdspl.obj" \
	"$(INTDIR)\testMemIO.obj" \
	"$(INTDIR)\themes.obj" \
	"$(INTDIR)\themes2.obj" \
	"$(INTDIR)\THEMES3.OBJ" \
	"$(INTDIR)\THMMSGP.OBJ" \
	"$(INTDIR)\tiff.obj" \
	"$(INTDIR)\tmfor.obj" \
	"$(INTDIR)\tminv.obj" \
	"$(INTDIR)\toolbar.obj" \
	"$(INTDIR)\tools.obj" \
	"$(INTDIR)\Tran.obj" \
	"$(INTDIR)\tranlat2.obj" \
	"$(INTDIR)\tranlat3.obj" \
	"$(INTDIR)\tranmerc.obj" \
	"$(INTDIR)\TRANPR~1.OBJ" \
	"$(INTDIR)\translat.obj" \
	"$(INTDIR)\traverse.obj" \
	"$(INTDIR)\UGRID.OBJ" \
	"$(INTDIR)\Ugridhlt.obj" \
	"$(INTDIR)\Umio.obj" \
	"$(INTDIR)\UNDO.OBJ" \
	"$(INTDIR)\untfz.obj" \
	"$(INTDIR)\ups.obj" \
	"$(INTDIR)\USBSDK.obj" \
	"$(INTDIR)\utm.obj" \
	"$(INTDIR)\utmfor.obj" \
	"$(INTDIR)\utminv.obj" \
	"$(INTDIR)\vandgfor.obj" \
	"$(INTDIR)\vandginv.obj" \
	"$(INTDIR)\VARDEF.OBJ" \
	"$(INTDIR)\VARSTUF2.OBJ" \
	"$(INTDIR)\VARSTUFF.OBJ" \
	"$(INTDIR)\vehicle.obj" \
	"$(INTDIR)\vehicle_server.obj" \
	"$(INTDIR)\viewport.obj" \
	"$(INTDIR)\visible.obj" \
	"$(INTDIR)\VOTERS.OBJ" \
	"$(INTDIR)\wivfor.obj" \
	"$(INTDIR)\wivinv.obj" \
	"$(INTDIR)\Writeavi.obj" \
	"$(INTDIR)\wviifor.obj" \
	"$(INTDIR)\wviiinv.obj" \
	"$(INTDIR)\GWCONFIG.res" \
	"..\GM32lib\FreeImage.lib" \
	"..\MrSid\Geo_DSDK\lib\Release_md\lti_dsdk_cdll.lib" \
	"..\DGNlib7\Release\DGNlib7_32.lib"

"$(OUTDIR)\geomaster.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("geomastr.dep")
!INCLUDE "geomastr.dep"
!ELSE 
!MESSAGE Warning: cannot find "geomastr.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "geomastr - Win32 Release" || "$(CFG)" == "geomastr - Win32 Debug"
SOURCE=.\address1.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

CPP_SWITCHES=/nologo /Zp2 /ML /vd0 /GR /GX /Zi /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "NO_STRICT" /D "XPORABOVE" /D _WIN32_WINNT=0x0502 /D "USEMRSID" /D "CHECKMEMx" /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\address1.obj"	"$(INTDIR)\address1.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

CPP_SWITCHES=/nologo /Zp2 /ML /w /W0 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "CHECKMEM" /D "NO_STRICT" /D "XPORABOVE" /D "USEMRSID" /D _WIN32_WINNT=0x0502 /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\address1.obj"	"$(INTDIR)\address1.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\address2.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

CPP_SWITCHES=/nologo /Zp2 /ML /vd0 /GR /GX /Zi /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "NO_STRICT" /D "XPORABOVE" /D _WIN32_WINNT=0x0502 /D "USEMRSID" /D "CHECKMEMx" /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\address2.obj"	"$(INTDIR)\address2.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

CPP_SWITCHES=/nologo /Zp2 /ML /w /W0 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "CHECKMEM" /D "NO_STRICT" /D "XPORABOVE" /D "USEMRSID" /D _WIN32_WINNT=0x0502 /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\address2.obj"	"$(INTDIR)\address2.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\address3.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

CPP_SWITCHES=/nologo /Zp2 /ML /vd0 /GR /GX /Zi /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "NO_STRICT" /D "XPORABOVE" /D _WIN32_WINNT=0x0502 /D "USEMRSID" /D "CHECKMEMx" /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\address3.obj"	"$(INTDIR)\address3.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

CPP_SWITCHES=/nologo /Zp2 /ML /w /W0 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "CHECKMEM" /D "NO_STRICT" /D "XPORABOVE" /D "USEMRSID" /D _WIN32_WINNT=0x0502 /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\address3.obj"	"$(INTDIR)\address3.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\AddressMsgProc.c

"$(INTDIR)\AddressMsgProc.obj"	"$(INTDIR)\AddressMsgProc.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE="..\Proj-4\proj-4.4.8\src\adjlon.c"

"$(INTDIR)\adjlon.obj"	"$(INTDIR)\adjlon.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\gctpc\alberfor.c

"$(INTDIR)\alberfor.obj"	"$(INTDIR)\alberfor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\gctpc\alberinv.c

"$(INTDIR)\alberinv.obj"	"$(INTDIR)\alberinv.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\gctpc\alconfor.c

"$(INTDIR)\alconfor.obj"	"$(INTDIR)\alconfor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\gctpc\alconinv.c

"$(INTDIR)\alconinv.obj"	"$(INTDIR)\alconinv.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\gctpc\azimfor.c

"$(INTDIR)\azimfor.obj"	"$(INTDIR)\azimfor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\gctpc\aziminv.c

"$(INTDIR)\aziminv.obj"	"$(INTDIR)\aziminv.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE="..\libbci-1.1.0\bci.c"

"$(INTDIR)\bci.obj"	"$(INTDIR)\bci.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\bigmem.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

CPP_SWITCHES=/nologo /Zp2 /ML /vd0 /GR /GX /Zi /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "NO_STRICT" /D "XPORABOVE" /D _WIN32_WINNT=0x0502 /D "USEMRSID" /D "CHECKMEMx" /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\bigmem.obj"	"$(INTDIR)\bigmem.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

CPP_SWITCHES=/nologo /Zp2 /ML /w /W0 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "CHECKMEM" /D "NO_STRICT" /D "XPORABOVE" /D "USEMRSID" /D _WIN32_WINNT=0x0502 /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\bigmem.obj"	"$(INTDIR)\bigmem.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=..\gctpc\br_gctp.c

"$(INTDIR)\br_gctp.obj"	"$(INTDIR)\br_gctp.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\DB\Bt_creat.c

"$(INTDIR)\Bt_creat.obj"	"$(INTDIR)\Bt_creat.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\humbird1\COACT\ByteSwap.c

"$(INTDIR)\ByteSwap.obj"	"$(INTDIR)\ByteSwap.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\cdstuff.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

CPP_SWITCHES=/nologo /Zp2 /ML /vd0 /GR /GX /Zi /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "NO_STRICT" /D "XPORABOVE" /D _WIN32_WINNT=0x0502 /D "USEMRSID" /D "CHECKMEMx" /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\cdstuff.obj"	"$(INTDIR)\cdstuff.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

CPP_SWITCHES=/nologo /Zp2 /ML /w /W0 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "CHECKMEM" /D "NO_STRICT" /D "XPORABOVE" /D "USEMRSID" /D _WIN32_WINNT=0x0502 /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\cdstuff.obj"	"$(INTDIR)\cdstuff.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=..\SHR\CLIDATA.C

"$(INTDIR)\CLIDATA.OBJ"	"$(INTDIR)\CLIDATA.SBR" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\cmdtonum.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

CPP_SWITCHES=/nologo /Zp2 /ML /vd0 /GR /GX /Zi /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "NO_STRICT" /D "XPORABOVE" /D _WIN32_WINNT=0x0502 /D "USEMRSID" /D "CHECKMEMx" /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\cmdtonum.obj"	"$(INTDIR)\cmdtonum.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

CPP_SWITCHES=/nologo /Zp2 /ML /w /W0 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "CHECKMEM" /D "NO_STRICT" /D "XPORABOVE" /D "USEMRSID" /D _WIN32_WINNT=0x0502 /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\cmdtonum.obj"	"$(INTDIR)\cmdtonum.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=..\SHR\COMDLG.c

"$(INTDIR)\COMDLG.obj"	"$(INTDIR)\COMDLG.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\SHR\Comput1.c

"$(INTDIR)\Comput1.obj"	"$(INTDIR)\Comput1.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\SHR\CONNAD83.C

"$(INTDIR)\CONNAD83.OBJ"	"$(INTDIR)\CONNAD83.SBR" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\convert.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

CPP_SWITCHES=/nologo /Zp2 /ML /vd0 /GR /GX /Zi /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "NO_STRICT" /D "XPORABOVE" /D _WIN32_WINNT=0x0502 /D "USEMRSID" /D "CHECKMEMx" /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\convert.obj"	"$(INTDIR)\convert.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

CPP_SWITCHES=/nologo /Zp2 /ML /w /W0 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "CHECKMEM" /D "NO_STRICT" /D "XPORABOVE" /D "USEMRSID" /D _WIN32_WINNT=0x0502 /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\convert.obj"	"$(INTDIR)\convert.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\CONVRT32.C

!IF  "$(CFG)" == "geomastr - Win32 Release"

CPP_SWITCHES=/nologo /Zp2 /ML /vd0 /GR /GX /Zi /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "NO_STRICT" /D "XPORABOVE" /D _WIN32_WINNT=0x0502 /D "USEMRSID" /D "CHECKMEMx" /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\CONVRT32.OBJ"	"$(INTDIR)\CONVRT32.SBR" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

CPP_SWITCHES=/nologo /Zp2 /ML /w /W0 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "CHECKMEM" /D "NO_STRICT" /D "XPORABOVE" /D "USEMRSID" /D _WIN32_WINNT=0x0502 /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\CONVRT32.OBJ"	"$(INTDIR)\CONVRT32.SBR" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=..\SHR\copydib.c

"$(INTDIR)\copydib.obj"	"$(INTDIR)\copydib.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\gctpc\cproj.c

"$(INTDIR)\cproj.obj"	"$(INTDIR)\cproj.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Creatcfg.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

CPP_SWITCHES=/nologo /Zp2 /ML /vd0 /GR /GX /Zi /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "NO_STRICT" /D "XPORABOVE" /D _WIN32_WINNT=0x0502 /D "USEMRSID" /D "CHECKMEMx" /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\Creatcfg.obj"	"$(INTDIR)\Creatcfg.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

CPP_SWITCHES=/nologo /Zp2 /ML /w /W0 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "CHECKMEM" /D "NO_STRICT" /D "XPORABOVE" /D "USEMRSID" /D _WIN32_WINNT=0x0502 /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\Creatcfg.obj"	"$(INTDIR)\Creatcfg.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=..\SHR\CVTCOORD.C

"$(INTDIR)\CVTCOORD.OBJ"	"$(INTDIR)\CVTCOORD.SBR" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\cwconfig.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

CPP_SWITCHES=/nologo /Zp2 /ML /vd0 /GR /GX /Zi /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "NO_STRICT" /D "XPORABOVE" /D _WIN32_WINNT=0x0502 /D "USEMRSID" /D "CHECKMEMx" /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\cwconfig.obj"	"$(INTDIR)\cwconfig.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

CPP_SWITCHES=/nologo /Zp2 /ML /w /W0 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "CHECKMEM" /D "NO_STRICT" /D "XPORABOVE" /D "USEMRSID" /D _WIN32_WINNT=0x0502 /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\cwconfig.obj"	"$(INTDIR)\cwconfig.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=..\shapelib\code\dbfopen.c

"$(INTDIR)\dbfopen.obj"	"$(INTDIR)\dbfopen.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\SHR\DDE.C

"$(INTDIR)\DDE.OBJ"	"$(INTDIR)\DDE.SBR" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\prog16\GM32lib\Demo.Rc
SOURCE=.\DGNFILE.C

"$(INTDIR)\DGNFILE.OBJ"	"$(INTDIR)\DGNFILE.SBR" : $(SOURCE) "$(INTDIR)"


SOURCE=..\SHR\DIBUTIL.C

"$(INTDIR)\DIBUTIL.OBJ"	"$(INTDIR)\DIBUTIL.SBR" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\DICT.C

!IF  "$(CFG)" == "geomastr - Win32 Release"

CPP_SWITCHES=/nologo /Zp2 /ML /vd0 /GR /GX /Zi /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "NO_STRICT" /D "XPORABOVE" /D _WIN32_WINNT=0x0502 /D "USEMRSID" /D "CHECKMEMx" /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\DICT.OBJ"	"$(INTDIR)\DICT.SBR" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

CPP_SWITCHES=/nologo /Zp2 /ML /w /W0 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "CHECKMEM" /D "NO_STRICT" /D "XPORABOVE" /D "USEMRSID" /D _WIN32_WINNT=0x0502 /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\DICT.OBJ"	"$(INTDIR)\DICT.SBR" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\DICTEDIT.C

!IF  "$(CFG)" == "geomastr - Win32 Release"

CPP_SWITCHES=/nologo /Zp2 /ML /vd0 /GR /GX /Zi /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "NO_STRICT" /D "XPORABOVE" /D _WIN32_WINNT=0x0502 /D "USEMRSID" /D "CHECKMEMx" /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\DICTEDIT.OBJ"	"$(INTDIR)\DICTEDIT.SBR" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

CPP_SWITCHES=/nologo /Zp2 /ML /w /W0 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "CHECKMEM" /D "NO_STRICT" /D "XPORABOVE" /D "USEMRSID" /D _WIN32_WINNT=0x0502 /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\DICTEDIT.OBJ"	"$(INTDIR)\DICTEDIT.SBR" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\digitize.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

CPP_SWITCHES=/nologo /Zp2 /ML /vd0 /GR /GX /Zi /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "NO_STRICT" /D "XPORABOVE" /D _WIN32_WINNT=0x0502 /D "USEMRSID" /D "CHECKMEMx" /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\digitize.obj"	"$(INTDIR)\digitize.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

CPP_SWITCHES=/nologo /Zp2 /ML /w /W0 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "CHECKMEM" /D "NO_STRICT" /D "XPORABOVE" /D "USEMRSID" /D _WIN32_WINNT=0x0502 /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\digitize.obj"	"$(INTDIR)\digitize.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=..\SHR\DLGLOOK.C

"$(INTDIR)\DLGLOOK.OBJ"	"$(INTDIR)\DLGLOOK.SBR" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Document.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

CPP_SWITCHES=/nologo /Zp2 /ML /vd0 /GR /GX /Zi /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "NO_STRICT" /D "XPORABOVE" /D _WIN32_WINNT=0x0502 /D "USEMRSID" /D "CHECKMEMx" /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\Document.obj"	"$(INTDIR)\Document.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

CPP_SWITCHES=/nologo /Zp2 /ML /w /W0 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "CHECKMEM" /D "NO_STRICT" /D "XPORABOVE" /D "USEMRSID" /D _WIN32_WINNT=0x0502 /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\Document.obj"	"$(INTDIR)\Document.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\dtm.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

CPP_SWITCHES=/nologo /Zp2 /ML /vd0 /GR /GX /Zi /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "NO_STRICT" /D "XPORABOVE" /D _WIN32_WINNT=0x0502 /D "USEMRSID" /D "CHECKMEMx" /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\dtm.obj"	"$(INTDIR)\dtm.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

CPP_SWITCHES=/nologo /Zp2 /ML /w /W0 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "CHECKMEM" /D "NO_STRICT" /D "XPORABOVE" /D "USEMRSID" /D _WIN32_WINNT=0x0502 /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\dtm.obj"	"$(INTDIR)\dtm.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\DTM2.C

"$(INTDIR)\DTM2.OBJ"	"$(INTDIR)\DTM2.SBR" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Dxfin.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

CPP_SWITCHES=/nologo /Zp2 /ML /vd0 /GR /GX /Zi /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "NO_STRICT" /D "XPORABOVE" /D _WIN32_WINNT=0x0502 /D "USEMRSID" /D "CHECKMEMx" /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\Dxfin.obj"	"$(INTDIR)\Dxfin.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

CPP_SWITCHES=/nologo /Zp2 /ML /w /W0 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "CHECKMEM" /D "NO_STRICT" /D "XPORABOVE" /D "USEMRSID" /D _WIN32_WINNT=0x0502 /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\Dxfin.obj"	"$(INTDIR)\Dxfin.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\dynaedit.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

CPP_SWITCHES=/nologo /Zp2 /ML /vd0 /GR /GX /Zi /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "NO_STRICT" /D "XPORABOVE" /D _WIN32_WINNT=0x0502 /D "USEMRSID" /D "CHECKMEMx" /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\dynaedit.obj"	"$(INTDIR)\dynaedit.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

CPP_SWITCHES=/nologo /Zp2 /ML /w /W0 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "CHECKMEM" /D "NO_STRICT" /D "XPORABOVE" /D "USEMRSID" /D _WIN32_WINNT=0x0502 /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\dynaedit.obj"	"$(INTDIR)\dynaedit.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\edgmatch.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

CPP_SWITCHES=/nologo /Zp2 /ML /vd0 /GR /GX /Zi /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "NO_STRICT" /D "XPORABOVE" /D _WIN32_WINNT=0x0502 /D "USEMRSID" /D "CHECKMEMx" /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\edgmatch.obj"	"$(INTDIR)\edgmatch.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

CPP_SWITCHES=/nologo /Zp2 /ML /w /W0 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "CHECKMEM" /D "NO_STRICT" /D "XPORABOVE" /D "USEMRSID" /D _WIN32_WINNT=0x0502 /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\edgmatch.obj"	"$(INTDIR)\edgmatch.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=..\gctpc\eqconfor.c

"$(INTDIR)\eqconfor.obj"	"$(INTDIR)\eqconfor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\gctpc\eqconinv.c

"$(INTDIR)\eqconinv.obj"	"$(INTDIR)\eqconinv.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\gctpc\equifor.c

"$(INTDIR)\equifor.obj"	"$(INTDIR)\equifor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\gctpc\equiinv.c

"$(INTDIR)\equiinv.obj"	"$(INTDIR)\equiinv.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\exception.c

"$(INTDIR)\exception.obj"	"$(INTDIR)\exception.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\export.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

CPP_SWITCHES=/nologo /Zp2 /ML /vd0 /GR /GX /Zi /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "NO_STRICT" /D "XPORABOVE" /D _WIN32_WINNT=0x0502 /D "USEMRSID" /D "CHECKMEMx" /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\export.obj"	"$(INTDIR)\export.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

CPP_SWITCHES=/nologo /Zp2 /ML /w /W0 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "CHECKMEM" /D "NO_STRICT" /D "XPORABOVE" /D "USEMRSID" /D _WIN32_WINNT=0x0502 /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\export.obj"	"$(INTDIR)\export.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\EXPORT2.C

"$(INTDIR)\EXPORT2.OBJ"	"$(INTDIR)\EXPORT2.SBR" : $(SOURCE) "$(INTDIR)"


SOURCE=..\GM32lib\FIIO_Mem.cpp

"$(INTDIR)\FIIO_Mem.obj"	"$(INTDIR)\FIIO_Mem.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\SHR\FILE.C

"$(INTDIR)\FILE.OBJ"	"$(INTDIR)\FILE.SBR" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\fish.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

CPP_SWITCHES=/nologo /Zp2 /ML /vd0 /GR /GX /Zi /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "NO_STRICT" /D "XPORABOVE" /D _WIN32_WINNT=0x0502 /D "USEMRSID" /D "CHECKMEMx" /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\fish.obj"	"$(INTDIR)\fish.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

CPP_SWITCHES=/nologo /Zp2 /ML /w /W0 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "CHECKMEM" /D "NO_STRICT" /D "XPORABOVE" /D "USEMRSID" /D _WIN32_WINNT=0x0502 /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\fish.obj"	"$(INTDIR)\fish.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=..\SHR\floatap.c

"$(INTDIR)\floatap.obj"	"$(INTDIR)\floatap.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\gctpc\for_init.c

"$(INTDIR)\for_init.obj"	"$(INTDIR)\for_init.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\GM32lib\freeim.c

"$(INTDIR)\freeim.obj"	"$(INTDIR)\freeim.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\FUNCSUBS.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

CPP_SWITCHES=/nologo /Zp2 /ML /vd0 /GR /GX /Zi /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "NO_STRICT" /D "XPORABOVE" /D _WIN32_WINNT=0x0502 /D "USEMRSID" /D "CHECKMEMx" /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\FUNCSUBS.obj"	"$(INTDIR)\FUNCSUBS.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

CPP_SWITCHES=/nologo /Zp2 /ML /w /W0 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "CHECKMEM" /D "NO_STRICT" /D "XPORABOVE" /D "USEMRSID" /D _WIN32_WINNT=0x0502 /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\FUNCSUBS.obj"	"$(INTDIR)\FUNCSUBS.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\function.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

CPP_SWITCHES=/nologo /Zp2 /ML /vd0 /GR /GX /Zi /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "NO_STRICT" /D "XPORABOVE" /D _WIN32_WINNT=0x0502 /D "USEMRSID" /D "CHECKMEMx" /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\function.obj"	"$(INTDIR)\function.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

CPP_SWITCHES=/nologo /Zp2 /ML /w /W0 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "CHECKMEM" /D "NO_STRICT" /D "XPORABOVE" /D "USEMRSID" /D _WIN32_WINNT=0x0502 /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\function.obj"	"$(INTDIR)\function.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\FUNCTN1.C

!IF  "$(CFG)" == "geomastr - Win32 Release"

CPP_SWITCHES=/nologo /Zp2 /ML /vd0 /GR /GX /Zi /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "NO_STRICT" /D "XPORABOVE" /D _WIN32_WINNT=0x0502 /D "USEMRSID" /D "CHECKMEMx" /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\FUNCTN1.OBJ"	"$(INTDIR)\FUNCTN1.SBR" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

CPP_SWITCHES=/nologo /Zp2 /ML /w /W0 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "CHECKMEM" /D "NO_STRICT" /D "XPORABOVE" /D "USEMRSID" /D _WIN32_WINNT=0x0502 /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\FUNCTN1.OBJ"	"$(INTDIR)\FUNCTN1.SBR" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\functn2.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

CPP_SWITCHES=/nologo /Zp2 /ML /vd0 /GR /GX /Zi /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "NO_STRICT" /D "XPORABOVE" /D _WIN32_WINNT=0x0502 /D "USEMRSID" /D "CHECKMEMx" /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\functn2.obj"	"$(INTDIR)\functn2.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

CPP_SWITCHES=/nologo /Zp2 /ML /w /W0 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "CHECKMEM" /D "NO_STRICT" /D "XPORABOVE" /D "USEMRSID" /D _WIN32_WINNT=0x0502 /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\functn2.obj"	"$(INTDIR)\functn2.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\FUNCTN3.C

!IF  "$(CFG)" == "geomastr - Win32 Release"

CPP_SWITCHES=/nologo /Zp2 /ML /vd0 /GR /GX /Zi /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "NO_STRICT" /D "XPORABOVE" /D _WIN32_WINNT=0x0502 /D "USEMRSID" /D "CHECKMEMx" /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\FUNCTN3.OBJ"	"$(INTDIR)\FUNCTN3.SBR" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

CPP_SWITCHES=/nologo /Zp2 /ML /w /W0 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "CHECKMEM" /D "NO_STRICT" /D "XPORABOVE" /D "USEMRSID" /D _WIN32_WINNT=0x0502 /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\FUNCTN3.OBJ"	"$(INTDIR)\FUNCTN3.SBR" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\FUNDEFS.C

!IF  "$(CFG)" == "geomastr - Win32 Release"

CPP_SWITCHES=/nologo /Zp2 /ML /vd0 /GR /GX /Zi /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "NO_STRICT" /D "XPORABOVE" /D _WIN32_WINNT=0x0502 /D "USEMRSID" /D "CHECKMEMx" /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\FUNDEFS.OBJ"	"$(INTDIR)\FUNDEFS.SBR" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

CPP_SWITCHES=/nologo /Zp2 /ML /w /W0 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "CHECKMEM" /D "NO_STRICT" /D "XPORABOVE" /D "USEMRSID" /D _WIN32_WINNT=0x0502 /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\FUNDEFS.OBJ"	"$(INTDIR)\FUNDEFS.SBR" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=..\gctpc\gctp.c

"$(INTDIR)\gctp.obj"	"$(INTDIR)\gctp.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\GetSDChipID.c

"$(INTDIR)\GetSDChipID.obj"	"$(INTDIR)\GetSDChipID.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=..\GM32lib\GM32.c

"$(INTDIR)\GM32.obj"	"$(INTDIR)\GM32.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\GM32LIB.C

"$(INTDIR)\GM32LIB.OBJ"	"$(INTDIR)\GM32LIB.SBR" : $(SOURCE) "$(INTDIR)"


SOURCE=..\gctpc\gnomfor.c

"$(INTDIR)\gnomfor.obj"	"$(INTDIR)\gnomfor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\gctpc\gnominv.c

"$(INTDIR)\gnominv.obj"	"$(INTDIR)\gnominv.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\gctpc\goodfor.c

"$(INTDIR)\goodfor.obj"	"$(INTDIR)\goodfor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\gctpc\goodinv.c

"$(INTDIR)\goodinv.obj"	"$(INTDIR)\goodinv.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\gps\gps.c

"$(INTDIR)\gps.obj"	"$(INTDIR)\gps.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\graphic1.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

CPP_SWITCHES=/nologo /Zp2 /ML /vd0 /GR /GX /Zi /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "NO_STRICT" /D "XPORABOVE" /D _WIN32_WINNT=0x0502 /D "USEMRSID" /D "CHECKMEMx" /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\graphic1.obj"	"$(INTDIR)\graphic1.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

CPP_SWITCHES=/nologo /Zp2 /ML /w /W0 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "CHECKMEM" /D "NO_STRICT" /D "XPORABOVE" /D "USEMRSID" /D _WIN32_WINNT=0x0502 /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\graphic1.obj"	"$(INTDIR)\graphic1.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\graphic2.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

CPP_SWITCHES=/nologo /Zp2 /ML /vd0 /GR /GX /Zi /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "NO_STRICT" /D "XPORABOVE" /D _WIN32_WINNT=0x0502 /D "USEMRSID" /D "CHECKMEMx" /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\graphic2.obj"	"$(INTDIR)\graphic2.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

CPP_SWITCHES=/nologo /Zp2 /ML /w /W0 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "CHECKMEM" /D "NO_STRICT" /D "XPORABOVE" /D "USEMRSID" /D _WIN32_WINNT=0x0502 /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\graphic2.obj"	"$(INTDIR)\graphic2.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\Graphic3.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

CPP_SWITCHES=/nologo /Zp2 /ML /vd0 /GR /GX /Zi /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "NO_STRICT" /D "XPORABOVE" /D _WIN32_WINNT=0x0502 /D "USEMRSID" /D "CHECKMEMx" /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\Graphic3.obj"	"$(INTDIR)\Graphic3.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

CPP_SWITCHES=/nologo /Zp2 /ML /w /W0 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "CHECKMEM" /D "NO_STRICT" /D "XPORABOVE" /D "USEMRSID" /D _WIN32_WINNT=0x0502 /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\Graphic3.obj"	"$(INTDIR)\Graphic3.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\graphic4.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

CPP_SWITCHES=/nologo /Zp2 /ML /vd0 /GR /GX /Zi /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "NO_STRICT" /D "XPORABOVE" /D _WIN32_WINNT=0x0502 /D "USEMRSID" /D "CHECKMEMx" /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\graphic4.obj"	"$(INTDIR)\graphic4.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

CPP_SWITCHES=/nologo /Zp2 /ML /w /W0 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "CHECKMEM" /D "NO_STRICT" /D "XPORABOVE" /D "USEMRSID" /D _WIN32_WINNT=0x0502 /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\graphic4.obj"	"$(INTDIR)\graphic4.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\GRAPHIC5.C

!IF  "$(CFG)" == "geomastr - Win32 Release"

CPP_SWITCHES=/nologo /Zp2 /ML /vd0 /GR /GX /Zi /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "NO_STRICT" /D "XPORABOVE" /D _WIN32_WINNT=0x0502 /D "USEMRSID" /D "CHECKMEMx" /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\GRAPHIC5.OBJ"	"$(INTDIR)\GRAPHIC5.SBR" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

CPP_SWITCHES=/nologo /Zp2 /ML /w /W0 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "CHECKMEM" /D "NO_STRICT" /D "XPORABOVE" /D "USEMRSID" /D _WIN32_WINNT=0x0502 /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\GRAPHIC5.OBJ"	"$(INTDIR)\GRAPHIC5.SBR" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\Graphic6.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

CPP_SWITCHES=/nologo /Zp2 /ML /vd0 /GR /GX /Zi /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "NO_STRICT" /D "XPORABOVE" /D _WIN32_WINNT=0x0502 /D "USEMRSID" /D "CHECKMEMx" /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\Graphic6.obj"	"$(INTDIR)\Graphic6.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

CPP_SWITCHES=/nologo /Zp2 /ML /w /W0 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "CHECKMEM" /D "NO_STRICT" /D "XPORABOVE" /D "USEMRSID" /D _WIN32_WINNT=0x0502 /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\Graphic6.obj"	"$(INTDIR)\Graphic6.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\graphic7.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

CPP_SWITCHES=/nologo /Zp2 /ML /vd0 /GR /GX /Zi /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "NO_STRICT" /D "XPORABOVE" /D _WIN32_WINNT=0x0502 /D "USEMRSID" /D "CHECKMEMx" /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\graphic7.obj"	"$(INTDIR)\graphic7.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

CPP_SWITCHES=/nologo /Zp2 /ML /w /W0 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "CHECKMEM" /D "NO_STRICT" /D "XPORABOVE" /D "USEMRSID" /D _WIN32_WINNT=0x0502 /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\graphic7.obj"	"$(INTDIR)\graphic7.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\graphic8.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

CPP_SWITCHES=/nologo /Zp2 /ML /vd0 /GR /GX /Zi /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "NO_STRICT" /D "XPORABOVE" /D _WIN32_WINNT=0x0502 /D "USEMRSID" /D "CHECKMEMx" /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\graphic8.obj"	"$(INTDIR)\graphic8.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

CPP_SWITCHES=/nologo /Zp2 /ML /w /W0 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "CHECKMEM" /D "NO_STRICT" /D "XPORABOVE" /D "USEMRSID" /D _WIN32_WINNT=0x0502 /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\graphic8.obj"	"$(INTDIR)\graphic8.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\GRAPHIC9.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

CPP_SWITCHES=/nologo /Zp2 /ML /vd0 /GR /GX /Zi /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "NO_STRICT" /D "XPORABOVE" /D _WIN32_WINNT=0x0502 /D "USEMRSID" /D "CHECKMEMx" /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\GRAPHIC9.obj"	"$(INTDIR)\GRAPHIC9.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

CPP_SWITCHES=/nologo /Zp2 /ML /w /W0 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "CHECKMEM" /D "NO_STRICT" /D "XPORABOVE" /D "USEMRSID" /D _WIN32_WINNT=0x0502 /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\GRAPHIC9.obj"	"$(INTDIR)\GRAPHIC9.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\graphic_functions.c

"$(INTDIR)\graphic_functions.obj"	"$(INTDIR)\graphic_functions.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\graphica.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

CPP_SWITCHES=/nologo /Zp2 /ML /vd0 /GR /GX /Zi /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "NO_STRICT" /D "XPORABOVE" /D _WIN32_WINNT=0x0502 /D "USEMRSID" /D "CHECKMEMx" /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\graphica.obj"	"$(INTDIR)\graphica.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

CPP_SWITCHES=/nologo /Zp2 /ML /w /W0 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "CHECKMEM" /D "NO_STRICT" /D "XPORABOVE" /D "USEMRSID" /D _WIN32_WINNT=0x0502 /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\graphica.obj"	"$(INTDIR)\graphica.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\graphicb.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

CPP_SWITCHES=/nologo /Zp2 /ML /vd0 /GR /GX /Zi /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "NO_STRICT" /D "XPORABOVE" /D _WIN32_WINNT=0x0502 /D "USEMRSID" /D "CHECKMEMx" /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\graphicb.obj"	"$(INTDIR)\graphicb.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

CPP_SWITCHES=/nologo /Zp2 /ML /w /W0 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "CHECKMEM" /D "NO_STRICT" /D "XPORABOVE" /D "USEMRSID" /D _WIN32_WINNT=0x0502 /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\graphicb.obj"	"$(INTDIR)\graphicb.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\graphicc.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

CPP_SWITCHES=/nologo /Zp2 /ML /vd0 /GR /GX /Zi /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "NO_STRICT" /D "XPORABOVE" /D _WIN32_WINNT=0x0502 /D "USEMRSID" /D "CHECKMEMx" /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\graphicc.obj"	"$(INTDIR)\graphicc.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

CPP_SWITCHES=/nologo /Zp2 /ML /w /W0 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "CHECKMEM" /D "NO_STRICT" /D "XPORABOVE" /D "USEMRSID" /D _WIN32_WINNT=0x0502 /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\graphicc.obj"	"$(INTDIR)\graphicc.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\GRAPHICD.C

!IF  "$(CFG)" == "geomastr - Win32 Release"

CPP_SWITCHES=/nologo /Zp2 /ML /vd0 /GR /GX /Zi /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "NO_STRICT" /D "XPORABOVE" /D _WIN32_WINNT=0x0502 /D "USEMRSID" /D "CHECKMEMx" /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\GRAPHICD.OBJ"	"$(INTDIR)\GRAPHICD.SBR" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

CPP_SWITCHES=/nologo /Zp2 /ML /w /W0 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "CHECKMEM" /D "NO_STRICT" /D "XPORABOVE" /D "USEMRSID" /D _WIN32_WINNT=0x0502 /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\GRAPHICD.OBJ"	"$(INTDIR)\GRAPHICD.SBR" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\GraphMsgProc.c

"$(INTDIR)\GraphMsgProc.obj"	"$(INTDIR)\GraphMsgProc.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\GraphMsgProc2.c

"$(INTDIR)\GraphMsgProc2.obj"	"$(INTDIR)\GraphMsgProc2.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\grfunc.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

CPP_SWITCHES=/nologo /Zp2 /ML /vd0 /GR /GX /Zi /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "NO_STRICT" /D "XPORABOVE" /D _WIN32_WINNT=0x0502 /D "USEMRSID" /D "CHECKMEMx" /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\grfunc.obj"	"$(INTDIR)\grfunc.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

CPP_SWITCHES=/nologo /Zp2 /ML /w /W0 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "CHECKMEM" /D "NO_STRICT" /D "XPORABOVE" /D "USEMRSID" /D _WIN32_WINNT=0x0502 /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\grfunc.obj"	"$(INTDIR)\grfunc.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=..\splines\gspline.c

"$(INTDIR)\gspline.obj"	"$(INTDIR)\gspline.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\gssigdi.c

"$(INTDIR)\gssigdi.obj"	"$(INTDIR)\gssigdi.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=..\gctpc\gvnspfor.c

"$(INTDIR)\gvnspfor.obj"	"$(INTDIR)\gvnspfor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\gctpc\gvnspinv.c

"$(INTDIR)\gvnspinv.obj"	"$(INTDIR)\gvnspinv.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\GWCONFIG.RC

"$(INTDIR)\GWCONFIG.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=..\DB\gwd.c

"$(INTDIR)\gwd.obj"	"$(INTDIR)\gwd.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\gctpc\hamfor.c

"$(INTDIR)\hamfor.obj"	"$(INTDIR)\hamfor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\gctpc\haminv.c

"$(INTDIR)\haminv.obj"	"$(INTDIR)\haminv.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\SHR\HASH.C

"$(INTDIR)\HASH.OBJ"	"$(INTDIR)\HASH.SBR" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\hbird.c

"$(INTDIR)\hbird.obj"	"$(INTDIR)\hbird.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\hotspots.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

CPP_SWITCHES=/nologo /Zp2 /ML /vd0 /GR /GX /Zi /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "NO_STRICT" /D "XPORABOVE" /D _WIN32_WINNT=0x0502 /D "USEMRSID" /D "CHECKMEMx" /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\hotspots.obj"	"$(INTDIR)\hotspots.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

CPP_SWITCHES=/nologo /Zp2 /ML /w /W0 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "CHECKMEM" /D "NO_STRICT" /D "XPORABOVE" /D "USEMRSID" /D _WIN32_WINNT=0x0502 /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\hotspots.obj"	"$(INTDIR)\hotspots.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=..\gctpc\imolwfor.c

"$(INTDIR)\imolwfor.obj"	"$(INTDIR)\imolwfor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\gctpc\imolwinv.c

"$(INTDIR)\imolwinv.obj"	"$(INTDIR)\imolwinv.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Infobox.c

!IF  "$(CFG)" == "geomastr - Win32 Release"

CPP_SWITCHES=/nologo /Zp2 /ML /vd0 /GR /GX /Zi /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "NO_STRICT" /D "XPORABOVE" /D _WIN32_WINNT=0x0502 /D "USEMRSID" /D "CHECKMEMx" /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\Infobox.obj"	"$(INTDIR)\Infobox.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "geomastr - Win32 Debug"

CPP_SWITCHES=/nologo /Zp2 /ML /w /W0 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "USEFREEIMAGE" /D "CHECKMEM" /D "NO_STRICT" /D "XPORABOVE" /D "USEMRSID" /D _WIN32_WINNT=0x0502 /D "ENABLETRACE" /D BITS_PER_COLOR=8 /FAs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\Infobox.obj"	"$(INTDIR)\Infobox.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=..\gctpc\inv_init.c

"$(INTDIR)\inv_init.obj"	"$(INTDIR)\inv_init.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\gctpc\lamazfor.c

"$(INTDIR)\lamazfor.obj"	"$(INTDIR)\lamazfor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\gctpc\lamazinv.c

"$(INTDIR)\lamazinv.obj"	"$(INTDIR)\lamazinv.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\gctpc\lamccfor.c

"$(INTDIR)\lamccfor.obj"	"$(INTDIR)\lamccfor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\gctpc\lamccinv.c

"$(INTDIR)\lamccinv.obj"	"$(INTDIR)\lamccinv.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\license.c

"$(INTDIR)\license.obj"	"$(INTDIR)\license.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=..\humbird1\LkmToHbirdImage.c

"$(INTDIR)\LkmToHbirdImage.obj"	"$(INTDIR)\LkmToHbirdImage.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\makearea.c

"$(INTDIR)\makearea.obj"	"$(INTDIR)\makearea.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mapcopy.c

"$(INTDIR)\mapcopy.obj"	"$(INTDIR)\mapcopy.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=..\SHR\memalloc.c

"$(INTDIR)\memalloc.obj"	"$(INTDIR)\memalloc.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\gctpc\merfor.c

"$(INTDIR)\merfor.obj"	"$(INTDIR)\merfor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\gctpc\merinv.c

"$(INTDIR)\merinv.obj"	"$(INTDIR)\merinv.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\MRGSCoordinates\Mgrs\geotrans2.2.6\dt_cc\mgrs\mgrs.c

"$(INTDIR)\mgrs.obj"	"$(INTDIR)\mgrs.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\gctpc\millfor.c

"$(INTDIR)\millfor.obj"	"$(INTDIR)\millfor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\gctpc\millinv.c

"$(INTDIR)\millinv.obj"	"$(INTDIR)\millinv.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\minilzo\minilzo.c

"$(INTDIR)\minilzo.obj"	"$(INTDIR)\minilzo.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\gctpc\molwfor.c

"$(INTDIR)\molwfor.obj"	"$(INTDIR)\molwfor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\gctpc\molwinv.c

"$(INTDIR)\molwinv.obj"	"$(INTDIR)\molwinv.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\GM32lib\mrsid.c

"$(INTDIR)\mrsid.obj"	"$(INTDIR)\mrsid.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\SHR\MS_MAPL.C

"$(INTDIR)\MS_MAPL.OBJ"	"$(INTDIR)\MS_MAPL.SBR" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE="..\Proj-4\proj-4.4.8\src\nad_cvt.c"

"$(INTDIR)\nad_cvt.obj"	"$(INTDIR)\nad_cvt.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE="..\Proj-4\proj-4.4.8\src\nad_init.c"

"$(INTDIR)\nad_init.obj"	"$(INTDIR)\nad_init.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE="..\Proj-4\proj-4.4.8\src\nad_intr.c"

"$(INTDIR)\nad_intr.obj"	"$(INTDIR)\nad_intr.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\GM32lib\nadcon.c

"$(INTDIR)\nadcon.obj"	"$(INTDIR)\nadcon.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\NETWORKS.c

"$(INTDIR)\NETWORKS.obj"	"$(INTDIR)\NETWORKS.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=..\gctpc\obleqfor.c

"$(INTDIR)\obleqfor.obj"	"$(INTDIR)\obleqfor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\gctpc\obleqinv.c

"$(INTDIR)\obleqinv.obj"	"$(INTDIR)\obleqinv.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\SHR\Odbcmain.c

"$(INTDIR)\Odbcmain.obj"	"$(INTDIR)\Odbcmain.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\OFFSET\Offset.c

"$(INTDIR)\Offset.obj"	"$(INTDIR)\Offset.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\gctpc\omerfor.c

"$(INTDIR)\omerfor.obj"	"$(INTDIR)\omerfor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\gctpc\omerinv.c

"$(INTDIR)\omerinv.obj"	"$(INTDIR)\omerinv.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ORAFILE.C

"$(INTDIR)\ORAFILE.OBJ"	"$(INTDIR)\ORAFILE.SBR" : $(SOURCE) "$(INTDIR)"


SOURCE=..\gctpc\orthfor.c

"$(INTDIR)\orthfor.obj"	"$(INTDIR)\orthfor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\gctpc\orthinv.c

"$(INTDIR)\orthinv.obj"	"$(INTDIR)\orthinv.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\orthos.c

"$(INTDIR)\orthos.obj"	"$(INTDIR)\orthos.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=..\gctpc\paksz.c

"$(INTDIR)\paksz.obj"	"$(INTDIR)\paksz.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\PCX\PCXTOBMP.C

"$(INTDIR)\PCXTOBMP.OBJ"	"$(INTDIR)\PCXTOBMP.SBR" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\peoplnet.c

"$(INTDIR)\peoplnet.obj"	"$(INTDIR)\peoplnet.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE="..\Proj-4\proj-4.4.8\src\pj_errno.c"

"$(INTDIR)\pj_errno.obj"	"$(INTDIR)\pj_errno.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE="..\Proj-4\proj-4.4.8\src\pj_malloc.c"

"$(INTDIR)\pj_malloc.obj"	"$(INTDIR)\pj_malloc.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE="..\Proj-4\proj-4.4.8\src\pj_open_lib.c"

"$(INTDIR)\pj_open_lib.obj"	"$(INTDIR)\pj_open_lib.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\pngrid.c

"$(INTDIR)\pngrid.obj"	"$(INTDIR)\pngrid.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\pointlist.c

"$(INTDIR)\pointlist.obj"	"$(INTDIR)\pointlist.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=..\MRGSCoordinates\Mgrs\geotrans2.2.6\dt_cc\polarst\polarst.c

"$(INTDIR)\polarst.obj"	"$(INTDIR)\polarst.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\OFFSET\POLY.C

"$(INTDIR)\POLY.OBJ"	"$(INTDIR)\POLY.SBR" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\gctpc\polyfor.c

"$(INTDIR)\polyfor.obj"	"$(INTDIR)\polyfor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\gctpc\polyinv.c

"$(INTDIR)\polyinv.obj"	"$(INTDIR)\polyinv.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Print.c

"$(INTDIR)\Print.obj"	"$(INTDIR)\Print.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\PROFILE.C

"$(INTDIR)\PROFILE.OBJ"	"$(INTDIR)\PROFILE.SBR" : $(SOURCE) "$(INTDIR)"


SOURCE=..\gctpc\psfor.c

"$(INTDIR)\psfor.obj"	"$(INTDIR)\psfor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\gctpc\psinv.c

"$(INTDIR)\psinv.obj"	"$(INTDIR)\psinv.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\quad.c

"$(INTDIR)\quad.obj"	"$(INTDIR)\quad.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\report.C

"$(INTDIR)\report.obj"	"$(INTDIR)\report.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=..\gctpc\reportgc.c

"$(INTDIR)\reportgc.obj"	"$(INTDIR)\reportgc.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ROADNAME.C

"$(INTDIR)\ROADNAME.OBJ"	"$(INTDIR)\ROADNAME.SBR" : $(SOURCE) "$(INTDIR)"


SOURCE=..\gctpc\robfor.c

"$(INTDIR)\robfor.obj"	"$(INTDIR)\robfor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\gctpc\robinv.c

"$(INTDIR)\robinv.obj"	"$(INTDIR)\robinv.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\route.c

"$(INTDIR)\route.obj"	"$(INTDIR)\route.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=..\SHR\serio.c

"$(INTDIR)\serio.obj"	"$(INTDIR)\serio.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\SHPFILE.C

"$(INTDIR)\SHPFILE.OBJ"	"$(INTDIR)\SHPFILE.SBR" : $(SOURCE) "$(INTDIR)"


SOURCE=..\SHR\shr.c

"$(INTDIR)\shr.obj"	"$(INTDIR)\shr.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\signinv.c

"$(INTDIR)\signinv.obj"	"$(INTDIR)\signinv.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=..\gctpc\sinfor.c

"$(INTDIR)\sinfor.obj"	"$(INTDIR)\sinfor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\gctpc\sininv.c

"$(INTDIR)\sininv.obj"	"$(INTDIR)\sininv.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\smooth.c

"$(INTDIR)\smooth.obj"	"$(INTDIR)\smooth.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=..\gctpc\somfor.c

"$(INTDIR)\somfor.obj"	"$(INTDIR)\somfor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\gctpc\sominv.c

"$(INTDIR)\sominv.obj"	"$(INTDIR)\sominv.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\gctpc\sphdz.c

"$(INTDIR)\sphdz.obj"	"$(INTDIR)\sphdz.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\splines\ssp.c

"$(INTDIR)\ssp.obj"	"$(INTDIR)\ssp.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\SHR\STD.C

"$(INTDIR)\STD.OBJ"	"$(INTDIR)\STD.SBR" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\gctpc\sterfor.c

"$(INTDIR)\sterfor.obj"	"$(INTDIR)\sterfor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\gctpc\sterinv.c

"$(INTDIR)\sterinv.obj"	"$(INTDIR)\sterinv.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\gctpc\stplnfor.c

"$(INTDIR)\stplnfor.obj"	"$(INTDIR)\stplnfor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\gctpc\stplninv.c

"$(INTDIR)\stplninv.obj"	"$(INTDIR)\stplninv.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\GCTP2\T2PROJ~1.C

"$(INTDIR)\T2PROJ~1.OBJ"	"$(INTDIR)\T2PROJ~1.SBR" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE="..\libbci-1.1.0\tdspl.c"

"$(INTDIR)\tdspl.obj"	"$(INTDIR)\tdspl.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\GM32lib\testMemIO.cpp

"$(INTDIR)\testMemIO.obj"	"$(INTDIR)\testMemIO.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\themes.c

"$(INTDIR)\themes.obj"	"$(INTDIR)\themes.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\themes2.c

"$(INTDIR)\themes2.obj"	"$(INTDIR)\themes2.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\THEMES3.C

"$(INTDIR)\THEMES3.OBJ"	"$(INTDIR)\THEMES3.SBR" : $(SOURCE) "$(INTDIR)"


SOURCE=.\THMMSGP.C

"$(INTDIR)\THMMSGP.OBJ"	"$(INTDIR)\THMMSGP.SBR" : $(SOURCE) "$(INTDIR)"


SOURCE=..\SHR\tiff.c

"$(INTDIR)\tiff.obj"	"$(INTDIR)\tiff.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\gctpc\tmfor.c

"$(INTDIR)\tmfor.obj"	"$(INTDIR)\tmfor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\gctpc\tminv.c

"$(INTDIR)\tminv.obj"	"$(INTDIR)\tminv.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\toolbar.c

"$(INTDIR)\toolbar.obj"	"$(INTDIR)\toolbar.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE="..\libbci-1.1.0\tools.c"

"$(INTDIR)\tools.obj"	"$(INTDIR)\tools.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Tran.c

"$(INTDIR)\Tran.obj"	"$(INTDIR)\Tran.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\tranlat2.c

"$(INTDIR)\tranlat2.obj"	"$(INTDIR)\tranlat2.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\tranlat3.c

"$(INTDIR)\tranlat3.obj"	"$(INTDIR)\tranlat3.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=..\MRGSCoordinates\Mgrs\geotrans2.2.6\dt_cc\tranmerc\tranmerc.c

"$(INTDIR)\tranmerc.obj"	"$(INTDIR)\tranmerc.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\GCTP2\TRANPR~1.C

"$(INTDIR)\TRANPR~1.OBJ"	"$(INTDIR)\TRANPR~1.SBR" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\translat.c

"$(INTDIR)\translat.obj"	"$(INTDIR)\translat.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\traverse.c

"$(INTDIR)\traverse.obj"	"$(INTDIR)\traverse.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=..\SHR\UGRID.C

"$(INTDIR)\UGRID.OBJ"	"$(INTDIR)\UGRID.SBR" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Ugridhlt.c

"$(INTDIR)\Ugridhlt.obj"	"$(INTDIR)\Ugridhlt.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=..\SHR\Umio.c

"$(INTDIR)\Umio.obj"	"$(INTDIR)\Umio.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\SHR\UNDO.C

"$(INTDIR)\UNDO.OBJ"	"$(INTDIR)\UNDO.SBR" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\gctpc\untfz.c

"$(INTDIR)\untfz.obj"	"$(INTDIR)\untfz.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\MRGSCoordinates\Mgrs\geotrans2.2.6\dt_cc\ups\ups.c

"$(INTDIR)\ups.obj"	"$(INTDIR)\ups.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\garmin\USB\USBSDK.c

"$(INTDIR)\USBSDK.obj"	"$(INTDIR)\USBSDK.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\MRGSCoordinates\Mgrs\geotrans2.2.6\dt_cc\utm\utm.c

"$(INTDIR)\utm.obj"	"$(INTDIR)\utm.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\gctpc\utmfor.c

"$(INTDIR)\utmfor.obj"	"$(INTDIR)\utmfor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\gctpc\utminv.c

"$(INTDIR)\utminv.obj"	"$(INTDIR)\utminv.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\gctpc\vandgfor.c

"$(INTDIR)\vandgfor.obj"	"$(INTDIR)\vandgfor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\gctpc\vandginv.c

"$(INTDIR)\vandginv.obj"	"$(INTDIR)\vandginv.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\VARDEF.C

"$(INTDIR)\VARDEF.OBJ"	"$(INTDIR)\VARDEF.SBR" : $(SOURCE) "$(INTDIR)"


SOURCE=..\SHR\VARSTUF2.C

"$(INTDIR)\VARSTUF2.OBJ"	"$(INTDIR)\VARSTUF2.SBR" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\SHR\VARSTUFF.C

"$(INTDIR)\VARSTUFF.OBJ"	"$(INTDIR)\VARSTUFF.SBR" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\vehicle.c

"$(INTDIR)\vehicle.obj"	"$(INTDIR)\vehicle.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\vehicle_server.c

"$(INTDIR)\vehicle_server.obj"	"$(INTDIR)\vehicle_server.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\viewport.c

"$(INTDIR)\viewport.obj"	"$(INTDIR)\viewport.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\visible.c

"$(INTDIR)\visible.obj"	"$(INTDIR)\visible.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\VOTERS.C

"$(INTDIR)\VOTERS.OBJ"	"$(INTDIR)\VOTERS.SBR" : $(SOURCE) "$(INTDIR)"


SOURCE=..\gctpc\wivfor.c

"$(INTDIR)\wivfor.obj"	"$(INTDIR)\wivfor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\gctpc\wivinv.c

"$(INTDIR)\wivinv.obj"	"$(INTDIR)\wivinv.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\SHR\Writeavi.c

"$(INTDIR)\Writeavi.obj"	"$(INTDIR)\Writeavi.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\gctpc\wviifor.c

"$(INTDIR)\wviifor.obj"	"$(INTDIR)\wviifor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\gctpc\wviiinv.c

"$(INTDIR)\wviiinv.obj"	"$(INTDIR)\wviiinv.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

