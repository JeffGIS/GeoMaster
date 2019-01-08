#include "graphint.h" 
#include "dibapi.h"
#include "FreeImage.h"
#include <sys\types.h>
#include <sys\stat.h>         

int testvalue (int i);
int CheckBTFID (HANDLE h);
HDIB32 BitmapToDIB_32(HBITMAP hBitmap, HPALETTE hPal);

#define	MAX_CONNECTED_PROCESSES	32

static	HWND hWndConnected[MAX_CONNECTED_PROCESSES];
static	int	 NumConnectedProcesses = 0;
static	char VPCTypes[][24]={"Format","Plan View","Profile View","Container View","-not yet implemented-","Menu","Legend Viewport","Index","Legend Image","Sub Viewport"};


#include "gmextern.h"

HFILE GetCfgFid (LPSTR Name,LPSHORT pVersion)
#if ENABLETRACE
{GSSiEnterProg (1003);
#endif
{
	OFSTRUCTGM	OFStruct;
	HFILE		Fid;
	short		Signature;
	
	*pVersion = 0;
	Fid = GSSiOpenFile (Name,(LPOFSTRUCTGM)&OFStruct,OF_READ); 
	if (Fid == HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (1003);
#endif
		return Fid;
}   
    GSSillseek(Fid,(LONG)-(4),2);
				
    BigRead (Fid,(HPSTR)&Signature,2);
    BigRead (Fid,(HPSTR)pVersion,2);
    if (Signature != 28052)
    {    
        MessageBox( GetFocus(), "This is not a valid configuration file",Name, MB_OK);
        goto BadFile;
    }
    if (*pVersion > CURRENT_CFG_VERSION)
    {    
        MessageBox( GetFocus(), "This configuration file version is not recognized",Name, MB_OK);
        goto BadFile;
    }
    if (*pVersion > 5)
    	Fid = DecompressCfgFile (Fid,*pVersion);
    GSSillseek(Fid,0,0);   
{
#if ENABLETRACE
GSSiExitProg (1003);
#endif
    return Fid;
}
    
BadFile:
    GSSiClose (Fid); 
    *pVersion = 0;
{
#if ENABLETRACE
GSSiExitProg (1003);
#endif
    return HFILE_ERROR;
}
#if ENABLETRACE
}
#endif
}

short GetCfgVersion (LPSTR Name)
#if ENABLETRACE
{GSSiEnterProg (1004);
#endif
{
	HFILE		Fid;
	short		Version;
	
	Fid = GetCfgFid (Name,&Version); 
	if (Fid == HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (1004);
#endif
		return 0;
}
    GSSiClose (Fid);
{
#if ENABLETRACE
GSSiExitProg (1004);
#endif
    return Version;
}
#if ENABLETRACE
}
#endif
}

BOOL ListCGFViewports (LPSTR Name,HWND hWndDlg,UINT icntl)
#if ENABLETRACE
{GSSiEnterProg (1005);
#endif
{   
	short	iview;
	short	NumViews, cv;  
	LPVIEWPORT	pCurView;  
	HANDLE	hViewports[MAX_VIEWPORTS];

	NumViews = LoadCGFViewports (Name,0,HFILE_ERROR,hViewports,&cv,0);
	if (!*pNumViewports)
{
#if ENABLETRACE
GSSiExitProg (1005);
#endif
		return FALSE;
}
	for (iview = 0;iview<NumViews; iview++)
	{   
		pCurView = (LPVIEWPORT)GlobalLock (hViewports[iview]);
	 	SendDlgItemMessage (hWndDlg,icntl,LB_ADDSTRING,0,(LPARAM)pCurView->Name);
	 	GSSiGlobUlFree (&hViewports[iview]); 
    }

{
#if ENABLETRACE
GSSiExitProg (1005);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}	

void ClearVPFields (LPVIEWPORT CurView)
#if ENABLETRACE
{GSSiEnterProg (1007);
#endif
{
	memset (CurView->hlpIndex,0,sizeof(CurView->hlpIndex));
	CurView->HaveFixedProfileRoute = 0;
	CurView->DisplayCycle = 0;
	CurView->hWndDlg = 0;
	CurView->CurDataRectID = -1;
	CurView->hDataDisplayRect = 0;
	CurView->nDataDisplayRect = 0;
	CurView->DisplayedFullScreen = 0;
	CurView->lpfnFUNSTACKMsgProc = 0;
	CurView->FunStackWnd = 0;
    CurView->NumThemes=0; 
	CurView->hMaskArea = 0;
	CurView->hBackgroundArea = 0;
	CurView->NumMaskPoints = CurView->NumMaskAreaParts = 0;
	CurView->numBackgroundAreaParts = CurView->numBackgroundAreaPoints = 0;
	CurView->hMaskAccelerator[0] = 0;
	CurView->hMaskAccelerator[1] = 0;
	CurView->hMaskAccelerator[2] = 0;
    CurView->hTranVPToBase=0; 
    CurView->hTranBaseToVP=0;
    CurView->hTranVPToScreen=0; 
    CurView->hTranScreenToVP=0;
	CurView->hTranProjectionToScreen = 0;
	CurView->hTranScreenToProjection = 0;
    CurView->hTAGList=0;  
    CurView->hPenRedef=0; 
    CurView->hReport=0;  
    CurView->Bitmap = 0;
	CurView->BitmapID = 0;
    CurView->hBinFileList=0;
    CurView->SubFile=CurView->RestoreFile=0; 
	*CurView->OrigFile = 0;
    CurView->hCursor=CurView->LinkedCursorHandle=0;
    CurView->LastCursor=0;  
    CurView->FunStackHandle=0;   
    CurView->CurrentFunction = 0;   
    CurView->BoundsDisplayCycle = -1;
    CurView->hEditRect=0;    
    CurView->hFileTransIn=CurView->hFileTransOut=0;
    CurView->HaveEditRect=FALSE;  
    CurView->ToolbarHandle = 0;
    CurView->lpBoundsDisplay = 0;  
    memset (CurView->hProfileRoute,0,sizeof(HANDLE)*MAXPROFILEROUTES);
    memset (CurView->hProfileElev,0,sizeof(HANDLE)*MAXPROFILEROUTES);
    memset (CurView->nProfileRoute,0,sizeof(long)*MAXPROFILEROUTES);
    memset (CurView->nProfileElev,0,sizeof(long)*MAXPROFILEROUTES);
    CurView->hProfileRouteSave = 0;
    CurView->ProfileInCrossSection = 0;  
    CurView->hRgn = 0;     
    CurView->hDistanceLine = 0;   
	CurView->hProfileDataRectangles = 0;
	CurView->nProfileDataRectangles = 0;
	CurView->hTransparencyBitmap = 0;
	CurView->hTransparentDC = 0;

{
#if ENABLETRACE
GSSiExitProg (1007);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}		

BOOL CopyCurViewToNew (LPSTR NewName)
{   
	short 	i;
	LPVIEWPORT	EditView = CurView; 
	HANDLE	hVisList; 
	LPVISLIST	FromVis, LastVisList, SaveVis;
	
	hViewports[*pNumViewports]=GSSiGlobAlloc (1303,GHND,sizeof(VIEWPORT)+MAX_VIEWPORT_FILES*MAX_PATH);
	pViewports[*pNumViewports] = (LPVIEWPORT) GlobalLock (hViewports[*pNumViewports]);
	pViewportsD[*pNumViewports]=pViewports[*pNumViewports];  
	*pViewports[*pNumViewports] = *CurView;
	CurView = pViewports[*pNumViewports]; 
	_fstrcpy (CurView->Name,NewName);
	CurView->ID = *pNumViewports+1; 
	for (i=0;i<CurView->NumFiles;i++)
	{
		CurView->lpFiles[i]=(LPSTR) &CurView->EndOfViewport + i * MAX_PATH;
		_fstrcpy (CurView->lpFiles[i],EditView->lpFiles[i]);
	}
				    		
	for (i=0;i<CurView->NumVisList;i++)
	{
		hVisList=GSSiGlobAlloc (1304,GHND,sizeof(VISLIST));
		CurVis =(LPVISLIST) GlobalLock (hVisList);
		if (!i)
		{   
			FromVis = EditView->pVisList1;  
			*CurVis = *FromVis;
			CurVis->hVisList = hVisList;
			CurView->pVisList1=CurVis;
			CurVis->LastVisList = 0;
		}
		else
		{   
			FromVis = (LPVISLIST)FromVis->NextVisList;
			*CurVis = *FromVis;
			CurVis->hVisList = hVisList;
			CurVis->LastVisList = (LPSTR)LastVisList;
			SaveVis = CurVis;
			CurVis =(LPVISLIST) LastVisList;
			CurVis->NextVisList = (LPSTR)SaveVis;
			CurVis = SaveVis;
		}
		CurVis->NextVisList = 0;
		LastVisList = CurVis;
	} 
	for (i=0;i<CurView->NumPickList;i++)
	{
		hVisList=GSSiGlobAlloc (1305,GHND,sizeof(VISLIST));
		CurVis =(LPVISLIST) GlobalLock (hVisList);
		if (!i)
		{
			if (!EditView->pPickList1)
				CurView->NumPickList = 0;
			else
			{
				FromVis = EditView->pPickList1;  
				*CurVis = *FromVis;
				CurVis->hVisList = hVisList;
				CurView->pPickList1=CurVis;
				CurVis->LastVisList = 0;
			}
		}
		else
		{   
			FromVis = (LPVISLIST)FromVis->NextVisList;
			*CurVis = *FromVis;
			CurVis->hVisList = hVisList;
			CurVis->LastVisList = (LPSTR)LastVisList;
			SaveVis = CurVis;
			CurVis =(LPVISLIST) LastVisList;
			CurVis->NextVisList = (LPSTR)SaveVis;
			CurVis = SaveVis;
		}
		CurVis->NextVisList = 0;
		LastVisList = (LPVISLIST)CurVis;
	} 
	if (CurView->pVisListManual)
	{
		hVisList=GSSiGlobAlloc (1306,GHND,sizeof(VISLIST));
		CurVis =(LPVISLIST) GlobalLock (hVisList);    
		*CurVis = *EditView->pVisListManual;
		CurVis->hVisList = hVisList;   
		CurView->pVisListManual = CurVis;
	}
	if (CurView->pPickListManual)
	{
		hVisList=GSSiGlobAlloc (1307,GHND,sizeof(VISLIST));
		CurVis =(LPVISLIST) GlobalLock (hVisList);
		*CurVis = *EditView->pPickListManual;
		CurVis->hVisList = hVisList;   
		CurView->pPickListManual = CurVis;
	}
	CurView->pTheme = 0;      
	CurView->hRgn = 0;
	CurView->hTAGList = 0;
	CurView->NumThemes = 0; 
	CurView->FunStackHandle = 0;
	CurView->BoundsDisplayID = 0;
	CurView->lpBoundsDisplay = 0;     
	_fmemset (CurView->hMaskAccelerator,0,sizeof(CurView->hMaskAccelerator));
	CurView->hMaskArea = CurView->hTranVPToBase = CurView->hTranBaseToVP = CurView->hTranVPToScreen = CurView->hTranScreenToVP =CurView->hReport =
	CurView->LinkedCursorHandle = CurView->hFileTransIn = CurView->hFileTransOut = 0;
	(*pNumViewports)++; 
	SelectVisList (FALSE);
	return TRUE;  
}

BOOL ParentVPInInches (short ID)
{
	if (pViewports[ID-1]->Parent)
	{   
		ID = pViewports[ID-1]->Parent;
		if (pViewports[ID-1]->WidthType == 1)
			return TRUE;
		else 
			return ParentVPInInches (ID);
	}
	return FALSE;
} 

double GetVPParentHeightInInches (short ID)
{
	double Inches=1, factor;
	
	if (pViewports[ID-1]->Parent)
	{   
		ID = pViewports[ID-1]->Parent;
		if (pViewports[ID-1]->WidthType == 1)
			Inches = pViewports[ID-1]->DesiredHeight;
		else
		{
			factor = pViewports[ID-1]->Height / 100;
			Inches = factor * GetVPParentHeightInInches (ID);
		}
	}
	return Inches;
}

double GetVPParentWidthInInches (short ID)
{
	double Inches=1, factor;
	
	if (pViewports[ID-1]->Parent)
	{   
		ID = pViewports[ID-1]->Parent;
		if (pViewports[ID-1]->WidthType == 1)
			Inches = pViewports[ID-1]->DesiredWidth;
		else
		{
			factor = pViewports[ID-1]->Width / 100;
			Inches = factor * GetVPParentWidthInInches (ID);
		}
	}
	return Inches;
}  

short GetFileTypeFromName(LPSTR InName,BOOL OpenFilelist)
{   
	char	Name[256];
	
	_fstrcpy (Name,InName);
	ExpandText (Name);    
	_fstrupr (Name);
	if (_fstrstr (Name,".PLT") || 
		_fstrstr (Name,".SHP") ||
		_fstrstr (Name,".MDB") ||
		_fstrstr (Name,".GDB") ||
		_fstrstr (Name,".ORA") ||
		_fstrstr(Name, ".DGN") ||
		_fstrstr(Name, ".DGN8") ||
		_fstrstr(Name, ".COM"))
		return 1;
	if (_fstrstr (Name,".BMP") || 
		_fstrstr (Name,".PCX") ||
		_fstrstr (Name,".TIF") ||
		_fstrstr (Name,".JPG") ||
		_fstrstr (Name,".SID"))
		return 4;
	if (_fstrstr (Name,".DTM") || 
		_fstrstr(Name, ".TIN") ||
		_fstrstr(Name, ".LA") ||
		_fstrstr(Name, ".LDR"))
		return 8;
	if (_fstrstr (Name,"FILELIST.TXT"))
	{
		if (OpenFilelist)
		{
			HFILE	Fid = GSSiOpenFile (Name,0,OF_READ);
			short	rtn=0;
			
			if (Fid == HFILE_ERROR)
				return 0;
			if (fgetstring (Name,255,Fid))
				rtn = GetFileTypeFromName(Name,TRUE); 
			GSSiClose (Fid);
			return rtn;
		}
		else
			return 2;
	}
	if (_fstrstr (Name,"INDEX"))
	{   
		if (GetMapIndexType (Name) == 4)
			return 2;
		return 3;
	}
	return 1;
}

int AddFileToViewport (LPSTR File)
{ 
	DLGPROC lpfnVPEDITMsgProc;
	int	nRc;
	
	VPAutoFile = File;						
	lpfnVPEDITMsgProc = MakeProcInstance((DLGPROC)VPEDITMsgProc, hInst);
	nRc = DialogBox(hInst, (LPSTR)"VPEDIT", hWndMain,lpfnVPEDITMsgProc);
	FreeProcInstance(lpfnVPEDITMsgProc); 
	VPAutoFile = 0;
	return nRc;
} 

int EditViewportAtCursor (HWND hWnd)
{ 
	DLGPROC lpfnVPEDITMsgProc;
	int	nRc;
	HWND	InFocus = GetParFocus();  
	RECT	Rect;
	POINT	CPoint;
	int		vpid;
		
	GetCursorPos (&CPoint); 
	ScreenToClient (hWnd,&CPoint);
 	vpid = SelectViewport (CPoint,FALSE,FALSE,FALSE);
	lpfnVPEDITMsgProc = MakeProcInstance((DLGPROC)VPEDITMsgProc, hInst);
	nRc = DialogBox(hInst, (LPSTR)"VPEDIT", hWndMain,lpfnVPEDITMsgProc);
	FreeProcInstance(lpfnVPEDITMsgProc); 
	return nRc;
} 
int EditFundirAtCursor (HWND hWnd)
{ 
	FARPROC lpfnVPEDITMsgProc;
	int	nRc=0;
	HWND	InFocus = GetParFocus();  
	RECT	Rect;
	POINT	CPoint;
	int		vpid;
		
	GetCursorPos (&CPoint); 
	ScreenToClient (hWnd,&CPoint);
 	vpid = SelectViewport (CPoint,FALSE,FALSE,FALSE);
	if (CurView->pTheme && CurView->pTheme->ID == GF_GRAPHICS_FUNCTION_THEME && *CurView->pTheme->SQL)
	{
		char file[MAX_PATH];

		sprintf (file,"[%%DL]fundir\\%s.txt",CurView->pTheme->SQL);
		if (ExistFile (file))
		{
			GMEdit (hWnd,file);
			nRc = 1;
		}
	}
	return nRc;
} 

BOOL LoadFormatCfg (LPSTR Name)
{
	short	NumFMTViews, ID;  
	LPVIEWPORT	CopyView, FromView, ToView, pVP;  
	HANDLE	hFMTViewports[MAX_VIEWPORTS];  
	BOOL	AddVP[MAX_VIEWPORTS];
	short	NewVPID[MAX_VIEWPORTS];
	short	SaveID,cv;
	char	SaveName[64];
	UINT	i,j;
	short	FormatVP=-1, OldFormatVP=-1;
	
	_fmemset (NewVPID,0,sizeof(NewVPID));				
	NumFMTViews = LoadCGFViewports (Name,0,HFILE_ERROR,hFMTViewports,&cv,0); 
	
	for (i=0;i<NumFMTViews;i++)
	{    
		pVP = (LPVIEWPORT)GlobalLock (hFMTViewports[i]);
		if (!_fstricmp ("Format",pVP->Name))  
		{
			AddVP[i] = FALSE;
			FormatVP = i;    
		}
		else
			AddVP[i] = TRUE;
		GlobalUnlock (hFMTViewports[i]);
	}
	if (FormatVP < 0)
	{
		GSSiMessageBox (0,"No format viewport in this configuration",0,MB_ICONEXCLAMATION,0);
		return FALSE;
	}
	for (i=0;i<*pNumViewports;i++)
	{
		if (!_fstricmp ("Format",pViewports[i]->Name))
			OldFormatVP = i;
	} 
	if (OldFormatVP < 0)
	{
        hViewports[*pNumViewports]=GSSiGlobAlloc (1312,GHND,sizeof(VIEWPORT)+MAX_VIEWPORT_FILES*MAX_PATH);
        ToView = (LPVIEWPORT) GlobalLock (hViewports[*pNumViewports]); 
        pViewports[*pNumViewports]=ToView;
        pViewportsD[*pNumViewports]=ToView; 
        OldFormatVP = (*pNumViewports)++;
	} 
	else
		ToView = pViewports[OldFormatVP];
	
	NewVPID[FormatVP] = OldFormatVP + 1;
	FromView = (LPVIEWPORT)GlobalLock (hFMTViewports[FormatVP]); 
	*ToView = *FromView; 
    if (!ToView->NumVisList)
    {   HANDLE	hVisList;
    	LPVISLIST	SaveVis=CurVis;
				    	
		ToView->NumVisList = 1;
		hVisList=GSSiGlobAlloc (1313,GHND,sizeof(VISLIST));
		CurVis = (LPVISLIST)GlobalLock (hVisList); 
		CurVis->hVisList=hVisList;
		InitVis ();
		ToView->pVisList1 = CurVis; 
		CurVis = SaveVis;
    }
	ToView->ID = OldFormatVP+1;  
	for (i=0;i<ToView->NumFiles;i++)
	{
		ToView->lpFiles[i]=(LPSTR) &ToView->EndOfViewport + i * MAX_PATH;
		_fstrcpy (ToView->lpFiles[i],FromView->lpFiles[i]);
	}
	GlobalUnlock (hFMTViewports[FormatVP]);  
	
	for (i=0;i<*pNumViewports;i++)
	{
		if (i != OldFormatVP && !pViewports[i]->Parent)
			pViewports[i]->Parent = OldFormatVP + 1;
	} 

	for (i=0;i<*pNumViewports;i++)
	{
		for (j=0;j<NumFMTViews;j++)
		{    
			pVP = (LPVIEWPORT)GlobalLock (hFMTViewports[j]);
			if (!_fstricmp (pViewports[i]->Name,pVP->Name))
			{   
				AddVP[j] = FALSE;   
				NewVPID[j] = i+1;
				pViewports[i]->WidthType = pVP->WidthType;
			    pViewports[i]->TagPointID = pVP->TagPointID;
	            pViewports[i]->TagPoint = pVP->TagPoint;
	            pViewports[i]->Width = pVP->Width;
	            pViewports[i]->Height = pVP->Height;
	            pViewports[i]->BorderPct = pVP->BorderPct;
	            pViewports[i]->Margin = pVP->Margin;
			}
			GlobalUnlock (hFMTViewports[j]);
		}
	} 
    
	for (j=0;j<NumFMTViews;j++)
	{    
		pVP = (LPVIEWPORT)GlobalLock (hFMTViewports[j]);
		if (AddVP[j])
		{
	        hViewports[*pNumViewports]=GSSiGlobAlloc (1314,GHND,sizeof(VIEWPORT)+MAX_VIEWPORT_FILES*MAX_PATH);
	        pViewports[*pNumViewports]=(LPVIEWPORT) GlobalLock (hViewports[*pNumViewports]);
	        pViewportsD[*pNumViewports]=pViewports[*pNumViewports]; 
	        *pViewports[*pNumViewports] = *pVP; 
	        pViewports[*pNumViewports]->ID = *pNumViewports + 1;
			for (i=0;i<pVP->NumFiles;i++)
			{
				pViewports[*pNumViewports]->lpFiles[i]=(LPSTR) &pViewports[*pNumViewports]->EndOfViewport + i * MAX_PATH;
				_fstrcpy (pViewports[*pNumViewports]->lpFiles[i],pVP->lpFiles[i]);
			}  
			(*pNumViewports)++; 
			NewVPID[j] = *pNumViewports;
		}
		GlobalUnlock (hFMTViewports[j]);
	} 
	for (j=0;j<NumFMTViews;j++)
	{    
		pVP = (LPVIEWPORT)GlobalLock (hFMTViewports[j]);
		if (AddVP[j])
		{   
			if (pVP->Parent && NewVPID[pVP->Parent-1])
				pViewports[NewVPID[j]-1]->Parent = NewVPID[pVP->Parent-1]; 
			if (pVP->pTheme)
			{
				pViewports[NewVPID[j]-1]->pTheme->TargetViewport = NewVPID[pVP->pTheme->TargetViewport-1]; 
				pViewports[NewVPID[j]-1]->pTheme->DisplayViewport = pViewports[NewVPID[j]-1]->ID; 
			}
		}
		GlobalUnlock (hFMTViewports[j]);
	} 
	for (i=0;i<NumFMTViews;i++)    
		GSSiGlobFree (&hFMTViewports[i]);
    DetermineVPDisplaySequence (); 
	
	return TRUE;
}


short GetVPIDFromName (LPSTR Name)
{   
	short	ID=0, Inc=0; 
	UINT	i;
	
	if (!_fstricmp (Name,"#CMD"))
		return *pCommandViewport;
Top:
	for (i=0;i<*pNumViewports;i++)
	{
		if (!_fstrcmp (Name,pViewports[i]->Name))
			ID = pViewports[i]->ID;
	}
	if (!ID && !CurrentConfig)
	{
		SetConfig (1); 
		Inc = 1000;
		goto Top;
	}
	if (ID)
		ID += Inc;
	if (Inc)
		SetConfig (0);
	return ID;
} 
void UnallocateConfig ()
#if ENABLETRACE
{GSSiEnterProg (99);
#endif
{   short iview, ifile,i;
    HANDLE handle;
    
    if (!*pNumViewports)
		goto Exit;
	DestroyAllToolbars ();
    RemoveAllInfoBoxRect();
	RemoveDataDisplayRect(0);
    GSSiGlobFree (&hMenuMask);  
    LastVP = 0;
    for (iview=0;iview<*pNumViewports;iview++)
    {   
	    HANDLE	hLast;
		LPCMDSTRING    pCmdStr; 
    	
        SetCurView ( pViewports[iview]);
        if (!CurView)
        	break;
        { 
	    	HANDLE	SaveHandle = CurView->FunStackHandle;
            
		    while (CurView->FunStackHandle)
		    { 
		    	pCmdStr = (LPCMDSTRING)GlobalLock (CurView->FunStackHandle);
		    	if (!pCmdStr)
		    	{
		    		CurView->FunStackHandle	= 0;
		    		break;
		    	}
		    	if (pCmdStr->CurFun)
		    		ProcessGraphicsFunction3 (pCmdStr->CurFun,CurView->hWnd,GF_CLOSE, 0,0); 	
		    	hLast = pCmdStr->PrevHandle; 
		    	GlobalUnlock (CurView->FunStackHandle);
	    		CurView->FunStackHandle = hLast; 
	    	} 
	    	CurView->FunStackHandle = SaveHandle;
	    }
        GSSiDeleteObject(&CurView->hRgn);
    	ClearPolyOff (FALSE);
		ClearBackgroundArea ();
//        SaveDisplayRedefFile(CurView->DisplayRedefFile);
        GSSiGlobFree (&CurView->hPenRedef);
	 	DestroySavedScreen (&CurView->Bitmap,CurView->BitmapID);  
	 	CurView->DisplayFunStack = 0; 
	 	DisplayFunctionStack ();
        for (ifile=0;ifile<CurView->NumFiles;ifile++)
        {
            if (CurView->FileType[ifile]>3 && CurView->hlpIndex[ifile])
            {
                CloseMapIndex (CurView->lpFiles[ifile],
                               CurView->hlpIndex[ifile],FALSE,TRUE);
                CurView->hlpIndex[ifile] = 0;
            }
        }
        CloseObject (CurView->pTheme);  
       	GSSiGlobFree (&CurView->ToolbarHandle);  
		for (i=0;i<MAXPROFILEROUTES;i++)
		{
			GSSiGlobFree (&CurView->hProfileRoute[i]);
			GSSiGlobFree (&CurView->hProfileElev[i]); 
		}
	    GSSiGlobFree (&CurView->hProfileRouteSave);
		GSSiGlobFree (&CurView->hDistanceLine);
		GSSiGlobFree (&CurView->hProfileDataRectangles);
		GSSiGlobFree (&CurView->hMaskAccelerator[0]);
		GSSiGlobFree (&CurView->hMaskAccelerator[1]);
		GSSiGlobFree (&CurView->hMaskAccelerator[2]);

		CurView->nProfileDataRectangles = 0;
        BoundsDisplayDestroy (CurView->lpBoundsDisplay); 
        CurView->lpBoundsDisplay=0;
        
        if (IsBadStringPtr((LPSTR)CurView->pVisList1, sizeof(VISLIST)))
        	CurView->pVisList1 = 0;
        CurVis = CurView->pVisList1;
        if (CurView->pVisListManual == CurVis)
            CurView->pVisListManual=0;
        while (CurVis)
        {
            handle = CurVis->hVisList;
            CurVis =  (LPVISLIST) CurVis->NextVisList;
            GSSiGlobUlFree (&handle);
        }   
        if (IsBadStringPtr((LPSTR)CurView->pPickList1, sizeof(VISLIST)))
        	CurView->pPickList1 = 0;
        CurVis = CurView->pPickList1;  
        if (CurView->pPickListManual == CurVis)
            CurView->pPickListManual=0;
        while (CurVis)
        {
            handle = CurVis->hVisList;
            CurVis =  (LPVISLIST) CurVis->NextVisList;
            GSSiGlobUlFree (&handle);
        }   
        if (CurView->pVisListManual)
        {
            handle = CurView->pVisListManual->hVisList;
            GSSiGlobUlFree (&handle);
        }
        if (CurView->pPickListManual)
        {
            handle = CurView->pPickListManual->hVisList;
            GSSiGlobUlFree (&handle);
        } 
        {
					
		    while (CurView->FunStackHandle)
		    { 
		    	pCmdStr = (LPCMDSTRING)GlobalLock (CurView->FunStackHandle);
		    	if (!pCmdStr)
		    	{
		    		CurView->FunStackHandle	= 0;
		    		break;
		    	}
//		    	if (pCmdStr->CurFun)
//		    		ProcessGraphicsFunction3 (pCmdStr->CurFun,CurView->hWnd,GF_CLOSE, 0,0); 	
		    	hLast = pCmdStr->PrevHandle;
		    	GSSiGlobFree (&pCmdStr->hError);
	    		GSSiGlobUlFree (&CurView->FunStackHandle);
	    		CurView->FunStackHandle = hLast; 
	    	}
		}        
	 	CloseEditRect(CurView);
        GSSiGlobFree (&CurView->hTAGList); 
        UnloadReport (&CurView->hReport);  
            
        DestroySavedScreen (&CurView->LinkedCursorHandle,0);
//	    CloseTRANS2 (&CurView->hTranFormat);
        CloseTRANS2 (&CurView->hTranVPToBase);
        CloseTRANS2 (&CurView->hTranBaseToVP);
        CloseTRANS2 (&CurView->hTranVPToScreen);
        CloseTRANS2 (&CurView->hTranScreenToVP);
       	CloseTRANS2 (&CurView->hFileTransIn); 
       	CloseTRANS2 (&CurView->hFileTransOut);
		CloseTRANS2 (&CurView->hTranProjectionToScreen);
		CloseTRANS2 (&CurView->hTranScreenToProjection);

        GSSiGlobUlFree (&hViewports[iview]);
        pViewports[iview] = 0;
    } 
    if (hViewportsMenu) 
    {
        short ii;
        ii = DestroyMenu (hViewportsMenu);
    }
Exit:
    SetCurView (0);
    hViewportsMenu = 0;
    NumPassiveFun = 0;  
    *pNumViewports = 0;  
    *pCommandViewport = 0;
    ClearTAGs();
	CloseOrthos(TRUE);
    FidConfig = HFILE_ERROR;
//	if (hWndPrompt)
//	    DestroyWindow(hWndPrompt);
//	CloseGEOSPANVideo ();
	SetGlobalValue ("%OPENVIDEOWINDOW","F");
    ConfigLoaded = FALSE;
{
#if ENABLETRACE
GSSiExitProg (99);
#endif
    return;
}
#if ENABLETRACE
}
#endif
} 

BOOL SetMenuDefaultValues (HWND hWnd)
{   
	switch (BitmapSizeOpt)
	{
		case 1:
			CheckMenuItem(GetMenu(hWnd), IDM_SMALLBM, MF_BYCOMMAND | MF_CHECKED);
			CheckMenuItem(GetMenu(hWnd), IDM_LARGEBM, MF_BYCOMMAND | MF_UNCHECKED);  
			break;
		default:
			CheckMenuItem(GetMenu(hWnd), IDM_SMALLBM, MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(GetMenu(hWnd), IDM_LARGEBM, MF_BYCOMMAND | MF_CHECKED);  
	}
	return TRUE;
} 

void LoadFullMenu (HWND hWnd)
{   
	HMENU	hMenuOld = GetMenu (hWnd);  
	HMENU	hMenu = LoadMenu (hInst,"GEOMASTER");
        	
	SetMenu (hWnd,hMenu); 
 	DestroyUserPopups (&hCurPopups);
	hUserMenu = 0;         
	if (hMenuOld)
		DestroyMenu (hMenuOld);
	return;  
}  

HFILE DecompressCfgFile (HFILE FidConfig, int Version)
{   
    long	lenCfg = GSSillseek (FidConfig,0,2),UncompLen;
	HANDLE	handle;
	HANDLE	hTemp;
	HPSTR	pTemp;
	HPSTR	pCfg;  
	short	ConfigDesc,Signature,Version16;   
	int		ii;
	
	if (IsMemFile (FidConfig))
		return FidConfig;
	hTemp = GSSiGlobAlloc (1532,GMEM_MOVEABLE,lenCfg*2);
	pTemp = GlobalLock (hTemp);
    if (Version < 9)
	{
		UncompLen = MAX_CFG_SIZE;
		lenCfg -= 6;
		GSSillseek(FidConfig,lenCfg,0);
	}
	else
	{
		lenCfg -= 10;
		ii=GSSillseek(FidConfig,lenCfg,0);
		GSSilread (FidConfig,&UncompLen,4);
	}
	handle = GSSiGlobAlloc (1533,GMEM_MOVEABLE,UncompLen*2+1024);
	pCfg   = GlobalLock (handle);
    GSSilread (FidConfig,&ConfigDesc,2);
    GSSilread (FidConfig,&Signature,2);
    GSSilread (FidConfig,&Version16,2);
    GSSillseek(FidConfig,0,0);
    BigRead (FidConfig,pTemp,lenCfg);
    GSSiClose (FidConfig);
	lenCfg = DecompressBinaryRecordUnsafe (pCfg,pTemp,lenCfg); 
	pCfg += lenCfg; 
	BufWrite (&pCfg,&lenCfg,(HPSTR)&ConfigDesc,2);
	BufWrite (&pCfg,&lenCfg,(HPSTR)&Signature,2);
	BufWrite (&pCfg,&lenCfg,(HPSTR)&Version16,2);
	GlobalUnlock (handle);
	GSSiGlobUlFree (&hTemp);
	FidConfig = SetMemFile (FidConfig,handle,lenCfg);  
	return FidConfig;
}


BOOL LoadMenuConfig (LPSTR Name, long Offset,HFILE OpenFid,short Version)
{    
	short	iview, nvp;
	HDC		hDC;
	RECT	Rect;
	
	 if (SetConfig(0))
		UnallocateConfig ();
 	 SetConfig (0);
 	 nvp = LoadCGFViewports (Name,Offset,OpenFid,hViewports,pCommandViewport,Version);
 	 if (nvp) 
 	 {  
 	 	_fstrcpy (MenuCFGName,Name);  
 	 	for (iview=0;iview<nvp;iview++)
 	 		pViewports[iview] = (LPVIEWPORT)GlobalLock (hViewports[iview]);  
 	 	*pNumViewports = nvp;
 	    DetermineVPDisplaySequence ();
 	    GetClientRect (hWndMain,&Rect);
 	    hDC = GetDC (hWndMain);
	    SetupViewports (hWndMain,hDC,0,Rect,0);
	    ReleaseDC (hWndMain,hDC);
	 	return TRUE;
	 }
	 else 
	 {
		ConfigDisplayRect.left=ConfigDisplayRect.right = 0;
	 	*MenuCFGName = 0;
	 	SetConfig (1);
	 	return FALSE;   
	 } 
}

HFILE CompressConfig (HFILE Fid,LPSTR Name)
{
	long	ln=GSSillseek(Fid,0,2);
	HANDLE	hTemp = GSSiGlobAlloc (1542,GMEM_MOVEABLE,ln*2+32);
	HPSTR	pTemp = GlobalLock (hTemp); 
	HPSTR	pCompressedRec = pTemp + ln;
	long	CompressedLength;
	
	GSSillseek (Fid,0,0);
	BigRead (Fid,pTemp,ln);
    CompressedLength = CompressBinaryRecord (pTemp,pCompressedRec,ln); 
    GSSiClose (Fid);  
    Fid = GSSiOpenFile (Name,0,OF_CREATE);
	BigWrite (Fid,pCompressedRec,CompressedLength,-1);
	BigWrite (Fid,&ln,4,-1);
	GSSiGlobUlFree (&hTemp);
	return Fid;
}


void SaveMenusInConfig (HFILE Fid, short opt)
#if ENABLETRACE
{GSSiEnterProg (601);
#endif
{
 	short	Length, Version=1, id=OB_SAVEMENUNAME; 
    char	Name[256], Name2[256];
    
    _fstrcpy (Name,MenuCFGName);
    ExpandText (Name);
    if (*Name)
    { 
	    _fullpath (Name2,Name,256);
	    SubstituteDL (Name2,FALSE);
	}
	else
		*Name2 = 0; 
 	BigWrite (Fid,(HPSTR)&id,2,-1);  
 	BigWrite (Fid,(HPSTR)&Version,2,-1);
 	BigWrite (Fid,(HPSTR)&Version,2,-1); 
 	Length = _fstrlen (Name2) + 1;
	BigWrite (Fid,(HPSTR)&Length,2,-1);
	BigWrite (Fid,(HPSTR)Name2,Length,-1);
{
#if ENABLETRACE
GSSiExitProg (601);
#endif
	return;
}
#if ENABLETRACE
}
#endif
} 

void EmbedMenusInConfig (HFILE Fid, LPSTR TempName)
#if ENABLETRACE
{GSSiEnterProg (601);
#endif
{
 	short	Version=1, id=OB_EMBEDMENUS; 
 	long	Length;   
 	HANDLE	hTemp;
 	HFILE	FidTemp;
 	HPSTR	pTemp;

 	BigWrite (Fid,(HPSTR)&id,2,-1);  
 	BigWrite (Fid,(HPSTR)&Version,2,-1);
 	BigWrite (Fid,(HPSTR)&Version,2,-1);
 	FidTemp = GSSiOpenFile (TempName,0,OF_READ);
	if (FidTemp != HFILE_ERROR)
	{
		Length = GSSillseek(FidTemp, 0, 2);
		if (Length)
		{
			GSSillseek(FidTemp, 0, 0);
			BigWrite(Fid, (HPSTR)&Length, 4, -1);
			hTemp = GSSiGlobAlloc(1543, GMEM_MOVEABLE, Length);
			pTemp = GlobalLock(hTemp);
			BigRead(FidTemp, pTemp, Length);
			BigWrite(Fid, pTemp, Length, -1);
			GSSiGlobUlFree(&hTemp);
		}
		GSSiClose(FidTemp);
	}
	else
		ii=1;
{
#if ENABLETRACE
GSSiExitProg (601);
#endif
	return;
}
#if ENABLETRACE
}
#endif
} 

BOOL CreateAllSizes (HWND hWndDlg)
{
	char	str[130];
	int		TotLen;
	LPSTR	pComma;
	HFILE Fid = GSSiOpenFile ("[%%DL]configs\\savesizes.txt",0,OF_READ);
	HDC		OldDC, hMemDC;
	MNMXCORD	LastBound = LastBounds;
	BOOL	SaveBufferedScreen = BufferedScreen;
	RECT	SaveConfigDisplayRect = ConfigDisplayRect;
	HFILE	FidTemp;
	LPSTR	pTempFile;
	int		NumSavedImages=0;
	int		iSavedImage=0;
	int		hdrloc, startimages;
	SAVEDIMAGEDATA	SavedImageData[MAXSAVEDIMAGES];

	if (Fid == HFILE_ERROR)
		return FALSE;
	IgnoreWPC = TRUE;
	memset (SavedImageData,0,sizeof(SavedImageData));
	while (fgetstring (str,128,Fid))
		NumSavedImages++;
	GSSillseek (Fid,0,0);
	NumSavedImages = min (NumSavedImages,MAXSAVEDIMAGES);
	SaveViewports (0);
	GSSiGlobFree (&hSaveCfgImagesFileName);
	hSaveCfgImagesFileName = GSSiGlobAlloc (1670,GMEM_MOVEABLE,MAX_PATH);
	pTempFile = GlobalLock (hSaveCfgImagesFileName);
	GSSiGetTempFileName (0,"gmc",0,(LPSTR)pTempFile); 
	FidTemp = GSSiOpenFile (pTempFile,0,OF_CREATE);
	GlobalUnlock (hSaveCfgImagesFileName);
	BufferedScreen = FALSE;
	SetViewport(*pCommandViewport);
	if (!ValidBounds (&LastBound))
		LastBound = CurView->WBounds;
	OldDC = CurView->hDC;
	BigWrite (FidTemp,(HPSTR)&NumSavedImages,sizeof(int),-1); 
	BigWrite (FidTemp,(HPSTR)&LastBound,sizeof(MNMXCORD),-1); 
	hdrloc = GSSillseek (FidTemp,0,1);
	BigWrite (FidTemp,(HPSTR)SavedImageData,NumSavedImages*sizeof(SAVEDIMAGEDATA),-1); 
	startimages = GSSillseek (FidTemp,0,1);
	TotLen = GSSifilelength (Fid);
	hdcMemMap = CreateCompatibleDC(CurView->hDC);   
    PctBox (GetDlgItem(hWndDlg,IDC_PROGRESS), TotLen,0,0);
	while (ContinueProcessing && iSavedImage < MAXSAVEDIMAGES && fgetstring (str,128,Fid))
	{
		WaitCursor (1);
		SavedImageData[iSavedImage].Offset = GSSillseek (FidTemp,0,1)-startimages;
		if ((pComma = strrchr (str,',')))
		{
			HBITMAP hbmpOld, hbmpOld2, hSaveBitmap;
			RECT	Rect;
			HDIB32	dib;
			HANDLE	hMemDIB;
			int		ImageLen, savewidth, saveheight;
			HPSTR	pImage;

			*pComma++ = 0;
			MemMapWidth = atoi (str);
    		MemMapHeight = atoi (pComma); 
			Rect.left = Rect.top = 0;
			Rect.right = MemMapWidth;
			Rect.bottom = MemMapHeight;
	     	ConfigDisplayRect = Rect;
			SavedImageData[iSavedImage].ClientRect = Rect;
			MemMap = TRUE;
			hMemBitmap = CreateCompatibleBitmap (OldDC,(int)MemMapWidth,(int)MemMapHeight); 
			hbmpOld = SelectObject(hdcMemMap, hMemBitmap);
			if (SetConfig (0))
			{
				SetViewport(*pCommandViewport);
				CurView->hDC = hdcMemMap;
				SetMainRect (CurView->hWnd,CurView->hDC,&Rect,2);
				SetupViewports (CurView->hWnd,CurView->hDC,0,MainRect,-1); 
			}
			SetConfig (1);
			SetViewport(*pCommandViewport);
			CurView->hDC = hdcMemMap;
			SetMainRect (CurView->hWnd,CurView->hDC,&Rect,2);
			SetupViewports (CurView->hWnd,CurView->hDC,0,ConfigDisplayRect,-1); 
			CurView->NewBounds = LastBound;
			SetScaleAndMidpointFromBounds (CurView);
			SetConfig (0);
			PaintMap (CurView->hWnd,hdcMemMap,TRUE,NULL,1);  
			MemMap = FALSE;
			SetViewport(*pCommandViewport);
			SavedImageData[iSavedImage].Rect = ConfigDisplayRect;
			SavedImageData[iSavedImage].Bounds = CurView->WBounds;
			savewidth = RECTWIDTH(&ConfigDisplayRect);
			saveheight = RECTHEIGHT(&ConfigDisplayRect);
			hMemDC = CreateCompatibleDC(OldDC);   
			hSaveBitmap = CreateCompatibleBitmap (OldDC,savewidth,saveheight); 
			hbmpOld2 = SelectObject(hMemDC, hSaveBitmap);
			BitBlt(hMemDC, 0,0,savewidth,saveheight, hdcMemMap, ConfigDisplayRect.left, ConfigDisplayRect.top, SRCCOPY);
			hSaveBitmap = SelectObject(hMemDC, hbmpOld2);
			hMemBitmap = SelectObject(hdcMemMap, hbmpOld);
			dib = BitmapToDIB_32(hSaveBitmap,NULL);
/*			{
				char FileName[MAX_PATH];

				HDIB32	dib2 = BitmapToDIB_32(hMemBitmap,NULL);
				
				sprintf (FileName,"c:\\temp\\testbmp\\%i_%i_window.bmp",MemMapWidth,MemMapHeight);
	    		SaveDIB32 (dib2,FileName,0,0);
				sprintf (FileName,"c:\\temp\\testbmp\\%i_%i.bmp",MemMapWidth,MemMapHeight);
	    		SaveDIB32 (dib,FileName,0,0);
			}*/
			CfgImageFormat = FIF_TIFF;
			hMemDIB = WriteDIBToMem (dib,FIF_TIFF,TIFF_ADOBE_DEFLATE,&ImageLen);
	 		SavedImageData[iSavedImage].ImageLen = ImageLen; 
	 		pImage = GlobalLock (hMemDIB); 
	 		BigWrite (FidTemp,(HPSTR)pImage,ImageLen,-1); 
			GSSiGlobUlFree (&hMemDIB);
			DeleteObject (hMemBitmap); 
			DeleteObject (hSaveBitmap); 
			HaltMapDisplay (FALSE,FALSE);
		}
 		iSavedImage++;
        PctBox (GetDlgItem(hWndDlg,IDC_PROGRESS), TotLen, GSSillseek (Fid,0,1),0);
		WaitCursor (-1);
	}
	GSSillseek (FidTemp,hdrloc,0);
	BigWrite (FidTemp,(HPSTR)SavedImageData,NumSavedImages*sizeof(SAVEDIMAGEDATA),-1); 
	GSSiClose (FidTemp);
	GSSiClose (Fid);
	DeleteDC (hdcMemMap);
	hdcMemMap = 0;
	BufferedScreen = SaveBufferedScreen;
	RestoreViewports ();
	CurView->hDC = OldDC;
	ConfigDisplayRect = SaveConfigDisplayRect;
	if (!ContinueProcessing)
		GSSiGlobFree (&hSaveCfgImagesFileName);
	IgnoreWPC = FALSE;
	return ContinueProcessing;
}

BOOL SaveAllSizes (HFILE FidOut)
{
	return TRUE;
}

void SaveWindowPos (HFILE Fid)
#if ENABLETRACE
{GSSiEnterProg (601);
#endif
{
 	short	Length, Version=1, id=OB_SAVESIZEPOS; 
    RECT	Rect;
    
    GetWindowRect (hWndMain,&Rect);
 	BigWrite (Fid,(HPSTR)&id,2,-1);  
 	BigWrite (Fid,(HPSTR)&Version,2,-1);
 	BigWrite (Fid,(HPSTR)&Version,2,-1); 
 	Length = sizeof (RECT);
	BigWrite (Fid,(HPSTR)&Length,2,-1);
	BigWrite (Fid,(HPSTR)&Rect,Length,-1);
{
#if ENABLETRACE
GSSiExitProg (601);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}



void CloseConfig ()
#if ENABLETRACE
{GSSiEnterProg (102);
#endif
{
   	GSSiClose (FidConfig);
    FidConfig = HFILE_ERROR;
{
#if ENABLETRACE
GSSiExitProg (102);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}

int checkvp(int i)
{
	extern	short idTimer; 
	extern	BOOL	InProfile;
	static	LPVIEWPORT pCheckVP=0; 
	static	LPSTR OrigTagFile=0;
	static	int	SaveX=0,SaveWidth;
	static	docheck=FALSE;
	static	double	SaveScale;
	static	int	ii=0;
	HPEN	hOldPen;
	RECT	LastRect={0,0,0,0};
	return 0;


/*	if (!pViewports)
		return 0;
	if (!CurrentConfig)
		return 0;
	if (!pViewports[0])
		return 0;
	if (!pViewports[19])
		return 0;
	if (pViewports[19]->pTheme)
	{
		if (pViewports[19]->pTheme->hHighlightFile)
			CheckBTFID (pViewports[19]->pTheme->hHighlightFile);
	}
	return 0;
	if (ii==2)
	{
		docheck = TRUE;
	//	SaveX = pViewports[0]->DrawRect.right;
		SaveScale = pViewports[0]->Scale;
		SaveWidth = pViewports[0]->LastWidth;
	}
	if (docheck && CurrentConfig && (SaveWidth != pViewports[0]->LastWidth || SaveScale != pViewports[0]->Scale))
	{
		ii=1;
		SaveScale = pViewports[0]->Scale;
	}
	if (InProfile)
	{
		if (CurView->ID == 1)
			ii=1;
	}
	return 1;
	if (hWndMain)
	{
	    HDC	hDC = GetDC (hWndMain);
		hOldPen = SelectObject (hDC,GetStockObject(BLACK_PEN)); 
		SelectObject (hDC,hOldPen);
		ReleaseDC (hWndMain,hDC);
	}
	return 0;
	if (OrigTagFile && OrigTagFile != TagFile)
		return 0; 
	if (TagFile)
		OrigTagFile = TagFile;  
//	if (pCheckVP && !pCheckVP->Active)
//		return 0;
	if (*FontNames[1] && _fstrnicmp (FontNames[1],"Cou",3))
		return 0;
	if (testvalue (1))
		return 0;
	if (!pNumViewports)
		return 0;
	if (!*pNumViewports && idTimer)
		return 0; 
//return 0;
	if (!*pNumViewports)
		return 0; 
	if (!pViewports[0])
		return 0;
	if (!CurrentConfig && pViewports[2]->Active)
		pCheckVP = pViewports[2];
	return 0;
	*/
}

short SetupViewport (RECT rect,BOOL ShrinkToFit,int Band)
#if ENABLETRACE
{GSSiEnterProg (794);
#endif
{   long RectWidth, RectHeight,Width, Height;
    //short , MidX, MidY;
    double  xfactor, yfactor; 
    int opx, opy;
    double	LogPixsX; 
    BOOL	rtn=FALSE;
	int	ii;

   	if (MemMap)
   		LogPixsX = DevicePixelsPerInch;   
   	else if (InVirtualPrint)
   		LogPixsX = VirtualPrintDPI; 
	else if (Printing && hDCPrinter)
		LogPixsX = (double)GetDeviceCaps(hDCPrinter, LOGPIXELSX);
	else
		LogPixsX = (double)GetDeviceCaps(CurView->hDC, LOGPIXELSX);
	RectWidth = (long)rect.right - (long)rect.left + 1;
    RectHeight = (long)rect.bottom - (long)rect.top + 1;
    if (!CurView->WindowIsZoomed)
    	CurView->HaveBounds=FALSE;
    if (CurView->TagPointType == 1)
    {
        xfactor = max ((double)RectWidth/100, (float)RectHeight/100);
        yfactor = xfactor;
    }   
	else if (CurView->WidthType == 3)
	{
		xfactor = 1;
		yfactor = 1;
	}
    else
    {
        xfactor = (double)RectWidth/100;
        yfactor = (double)RectHeight/100;
    }
	if (!Printing || FileMode || CurView->WidthType == 2 || ShrinkToFit)
	{
	    switch (CurView->TagPointID)
	    {   case 1:
				switch (CurView->WidthType)
				{
				case 1:
					CurView->TagPointActual.x=IDNINT(rect.left);// + (CurView->TagPoint.x / CurView->DesiredWidth) * RectWidth);
					CurView->TagPointActual.y=IDNINT(rect.bottom);// - (CurView->TagPoint.y / CurView->DesiredHeight) * RectHeight);
					opx = IDNINT((long)(rect.right));
					opy = IDNINT((long)(rect.top));
					break;
				case 2:
				case 3:
					CurView->TagPointActual.x=IDNINT(rect.left + CurView->TagPoint.x * xfactor);
					CurView->TagPointActual.y=IDNINT(rect.bottom - CurView->TagPoint.y * yfactor);
					opx = IDNINT((long)(rect.right - CurView->TagPoint.x * xfactor));
					opy = IDNINT((long)(rect.top + CurView->TagPoint.y * yfactor));
					break;
				}
	            break;
	        case 2:
	            CurView->TagPointActual.x=IDNINT(rect.left + CurView->TagPoint.x * xfactor);
	            CurView->TagPointActual.y=IDNINT(rect.top + CurView->TagPoint.y * yfactor);
	            opx = IDNINT(rect.right - CurView->TagPoint.x * xfactor);
	            opy = IDNINT(rect.bottom - CurView->TagPoint.y * yfactor);
	            break;
	        case 3:
	            CurView->TagPointActual.x=IDNINT(rect.right - CurView->TagPoint.x * xfactor);
	            CurView->TagPointActual.y=IDNINT(rect.top + CurView->TagPoint.y * yfactor);
	            opx = IDNINT(rect.left + CurView->TagPoint.x * xfactor);
	            opy = IDNINT(rect.bottom - CurView->TagPoint.y * yfactor);
	            break;
	        case 4:
	            CurView->TagPointActual.x=IDNINT(rect.right - CurView->TagPoint.x * xfactor);
	            CurView->TagPointActual.y=IDNINT(rect.bottom - CurView->TagPoint.y * yfactor);
	            opx = IDNINT(rect.left + CurView->TagPoint.x * xfactor);
	            opy = IDNINT(rect.top + CurView->TagPoint.y * yfactor);
	            break;
	    }
        if (!CurView->Width)
            Width = abs (CurView->TagPointActual.x - opx) +1; 
        else if (CurView->WidthType == 3)
            Width = CurView->Width+1;
		else
            Width = IDNINT((CurView->Width * (double) RectWidth) / 100);
        if (!CurView->Height)
            Height = abs (CurView->TagPointActual.y - opy) +1;
        else if (CurView->WidthType == 3)
            Height = CurView->Height+1;
		else
            Height = IDNINT((CurView->Height * (double) RectHeight) / 100);
	    switch (CurView->TagPointID)
	    {   case 1:
	            CurView->Rect.left = CurView->TagPointActual.x;
	            CurView->Rect.bottom = CurView->TagPointActual.y; 
	            break;
	        case 2:
	            CurView->Rect.left = CurView->TagPointActual.x;
	            CurView->Rect.bottom = CurView->TagPointActual.y + Height - 1; 
	            break;
	        case 3:
	            CurView->Rect.left = CurView->TagPointActual.x - Width + 1;
	            CurView->Rect.bottom = CurView->TagPointActual.y + Height - 1; 
	            break;
	        case 4:
	            CurView->Rect.left = CurView->TagPointActual.x - Width + 1;
	            CurView->Rect.bottom = CurView->TagPointActual.y; 
	            break;
	    }
	    CurView->Rect.right = CurView->Rect.left + Width -1;
	    CurView->Rect.top   = CurView->Rect.bottom - Height +1; 
		if (!CurView->Type)
			ii=1;
	    if (CurView->Parent)
	    {
	    	if (pViewports[CurView->Parent-1]->DesiredWidth)
	    		CurView->DesiredWidth = pViewports[CurView->Parent-1]->DesiredWidth *
	    								((double)CurView->Rect.right - (double)CurView->Rect.left + 1)/
	    								((double)pViewports[CurView->Parent-1]->Rect.right -
	    								 (double)pViewports[CurView->Parent-1]->Rect.left);
	    		CurView->DesiredHeight = pViewports[CurView->Parent-1]->DesiredHeight *
	    								((double)CurView->Rect.bottom - (double)CurView->Rect.top + 1)/
	    								((double)pViewports[CurView->Parent-1]->Rect.bottom -
	    								 (double)pViewports[CurView->Parent-1]->Rect.top);
	    }
	}
	else
	{
	    switch (CurView->TagPointID)
	    {   case 1:
				CurView->Rect.left = rect.left + IDNINT (CurView->TagPoint.x *	LogPixsX);
				CurView->Rect.right = rect.left + IDNINT ((CurView->TagPoint.x + CurView->DesiredWidth) * LogPixsX -1);
				CurView->Rect.bottom = IDNINT (rect.bottom - CurView->TagPoint.y * LogPixsX);
				CurView->Rect.top = IDNINT (rect.bottom - (CurView->TagPoint.y + CurView->DesiredHeight)* LogPixsX +1);
				break;
			case 2:
				CurView->Rect.left = rect.left + IDNINT (CurView->TagPoint.x *	LogPixsX);
				CurView->Rect.right = rect.left + IDNINT ((CurView->TagPoint.x + CurView->DesiredWidth) * LogPixsX -1);
				CurView->Rect.bottom = IDNINT (rect.top + (CurView->TagPoint.y + CurView->DesiredHeight)* LogPixsX);
				CurView->Rect.top = IDNINT (rect.top + (CurView->TagPoint.y)* LogPixsX+1);  
				break;
			case 3:
				CurView->Rect.right = IDNINT (rect.right - CurView->TagPoint.x * LogPixsX -1);
				CurView->Rect.left = IDNINT (rect.right - (CurView->TagPoint.x + CurView->DesiredWidth) * LogPixsX);
				CurView->Rect.bottom = IDNINT (rect.top + (CurView->TagPoint.y + CurView->DesiredHeight)* LogPixsX);
				CurView->Rect.top = IDNINT (rect.top + (CurView->TagPoint.y)* LogPixsX +1); 
				break;
			case 4:
				CurView->Rect.right = IDNINT (rect.right - CurView->TagPoint.x * LogPixsX -1);
				CurView->Rect.left = IDNINT (rect.right - (CurView->TagPoint.x + CurView->DesiredWidth) * LogPixsX);
				CurView->Rect.bottom = IDNINT (rect.bottom - CurView->TagPoint.y * LogPixsX);
				CurView->Rect.top = IDNINT (rect.bottom - (CurView->TagPoint.y + CurView->DesiredHeight)* LogPixsX +1);
				break;
		}
		if (CurView->WidthType == 1 && 
			(CurView->Rect.left < rect.left || CurView->Rect.right > rect.right ||
			CurView->Rect.top < rect.top || CurView->Rect.bottom > rect.bottom))
		{   
			short opt; 
			static	short	lastopt;
			
			if (MemMap || GetGlobalBVal2 ("[%ALWAYSSHRINK]",TRUE))
				opt = IDYES;
			else if (Band)
				opt = lastopt;
			else
				opt =  MessageBox(hWndMain, "This configuration does not fit on the page. Do you want to shrink it to fit?",
					 					 	"Warning", MB_YESNOCANCEL);
			lastopt = opt;
			switch (opt) 
			{
				case IDCANCEL:  
{
#if ENABLETRACE
GSSiExitProg (794);
#endif
					return 0;
}
				case IDYES:  
{
#if ENABLETRACE
GSSiExitProg (794);
#endif
					return -1;
}
			}
		}									
	} 
	CurView->Rect.right = max (CurView->Rect.right,CurView->Rect.left);
	CurView->Rect.bottom = max (CurView->Rect.bottom,CurView->Rect.top);
	
	{   
		short	iv, AddV; 
		RECT	NewRect;
		
	    for (iv = 0; iv<*pNumViewports; iv++)
	    {   
	    	AddV = pViewportsD[iv]->OnPrintAddSpaceToVP;
	        if (AddV == CurView->ID && !pViewportsD[iv]->Active)
	        {   
	        	LPVIEWPORT	SaveVP = CurView;
	        	
	        	if (SetViewport (AddV))
	        	{
		        	SetCurView ( pViewportsD[iv]);
		        	SetupViewport (rect,ShrinkToFit,Band);
		        	SetCurView ( SaveVP);
			        UnionRect (&NewRect,&pViewportsD[iv]->Rect,&CurView->Rect); 
			        CurView->Rect = NewRect;
			    }    
	        }
	    }
	}
	
	
    if (FileMode)
        CurView->DrawRect = CurView->Rect;
    else
    {
        CurView->DrawRect = PctRect (CurView->Rect,-(max(0,CurView->Margin)/*+max(0,CurView->BorderPct)*/));
        if (CurView->BorderPct >= 0)
        	InflateRect (&CurView->DrawRect,-1,-1); 
    } 
    CurView->ZBRect = CurView->DrawRect;
	CurView->ScreenRect = CurView->DrawRect;
	if (CurView->WidthType == 1)
	{
	 	if (!CurView->Type && CurView->AutoSize)
	 	{
	 		CurView->OrthoRes = 0;
		 	CurView->WindowZoomedToOrtho = FALSE; 
		 	CurView->NewBounds = CurView->WBoundsWhenSaved;
			CurView->NewBounds.xmn = -(CurView->DesiredWidth/(CurView->DrawRect.right-CurView->DrawRect.left+1))/2;
			CurView->NewBounds.ymn = -(CurView->DesiredHeight/(CurView->DrawRect.bottom-CurView->DrawRect.top+1))/2;
			CurView->NewBounds.xmx = CurView->DesiredWidth - CurView->NewBounds.xmn;
			CurView->NewBounds.ymx = CurView->DesiredHeight - CurView->NewBounds.ymn;
			GetVisBounds2 (&CurView->NewBounds,CurView->hDC);
			if (CurView->NewBounds.xmn < 0)
			{
				CurView->NewBounds.xmx += CurView->NewBounds.xmn;
				CurView->NewBounds.xmn = 0;
				CurView->NewBounds.ymx += CurView->NewBounds.ymn;
				CurView->NewBounds.ymn = 0;
			}
			CurView->WBounds = CurView->NewBounds;
			SetScaleAndMidpointFromBounds (CurView);

//	ZoomToPointAndScale (CurView->MidPointW,CurView->Scale,Imediate);

		}
	 	else  
	 	{
	 		CurView->OrthoRes = -1;
		 	CurView->WindowZoomedToOrtho=TRUE; 
		 	CurView->NewBounds.xmn=CurView->NewBounds.ymn=0;
		 	CurView->NewBounds.xmx=CurView->DesiredWidth;
		 	CurView->NewBounds.ymx=CurView->DesiredHeight; 
			CurView->WindowIsZoomed = CurView->HaveBounds=FALSE;
		}
	}
	if (CurView->WidthType == 1)
		SetBoundsRect2 (CurView->DrawRect,CurView->hDC);
    if (!CurrentConfig && CurView->ID == *pCommandViewport)
     	ConfigDisplayRect =  CurView->DrawRect;
    rtn = TRUE;
Exit:    
{
#if ENABLETRACE
GSSiExitProg (794);
#endif
    return rtn;
}
#if ENABLETRACE
}
#endif
} 

void SetConfigDisplayRect (void)
{
	int	SaveCurrentConfig = CurrentConfig;

	if (CurrentConfig && SetConfig (0))
	{
		SetViewport(*pCommandViewport);
     	ConfigDisplayRect =  CurView->DrawRect;
	}
	else
	{
		GetClientRect (hWndMain,&ConfigDisplayRect);
		if (DoScreenPrompt)
		{   
	        int	PRHeight = GetGlobalLVal2 ("[%PROMPTHEIGHT]",14); 

	        PromptRect = ConfigDisplayRect;
	        PromptRect.top = PromptRect.bottom-PRHeight;
	        ConfigDisplayRect.bottom -= (PromptRect.bottom - PromptRect.top + 1);
		}
	}
	SetConfig (SaveCurrentConfig);
	return;
}

void RecomputeViewport(RECT OrigRect)
#if ENABLETRACE
{GSSiEnterProg (795);
#endif
{   double  RectWidth, RectHeight;
    short Width, Height;
    RECT    rect; 
    
    if (CurView->Parent)
    {   
    	LPVIEWPORT	SaveVP=CurView;
    	
    	CurView = pViewports[CurView->Parent-1];
    	while (CurView->DisplayInParent && CurView->Parent && (CurView->Parent != CurView->ID))
    		CurView = pViewports[CurView->Parent-1];
    	rect = CurView->Rect;  
    	CurView = SaveVP;
    }
    else             
    	rect = MainRect;
    RectWidth = rect.right - rect.left + 1;
    RectHeight = rect.bottom - rect.top + 1; 
    Width = CurView->Rect.right - CurView->Rect.left + 1; 
//    if (CurView->Width)
        CurView->Width = (float)(100 * (double)Width/RectWidth); 
    Height = CurView->Rect.bottom - CurView->Rect.top + 1;
//    if (CurView->Height)
        CurView->Height = (float)(100 * (double)Height/RectHeight);
    switch (CurView->TagPointID)
    {   case 2:
            CurView->TagPoint.x = 100*(double)(CurView->Rect.left-rect.left)/RectWidth;
            CurView->TagPoint.y = 100*(double)(CurView->Rect.top-rect.top)/RectHeight;
            break;
        case 1:
            CurView->TagPoint.x = 100*(double)(CurView->Rect.left-rect.left)/RectWidth;
            CurView->TagPoint.y = 100-(100*(double)(CurView->Rect.bottom-rect.top)/RectHeight);
            break;
        case 3:
            CurView->TagPoint.x = 100-(100*(double)(CurView->Rect.right-rect.left)/RectWidth);
            CurView->TagPoint.y = 100*(double)(CurView->Rect.top-rect.top)/RectHeight;
            break;
        case 4:
            CurView->TagPoint.x = 100-(100*(double)(CurView->Rect.right-rect.left)/RectWidth);
            CurView->TagPoint.y = 100-(100*(double)(CurView->Rect.bottom-rect.top)/RectHeight);
            break;
    }
{
#if ENABLETRACE
GSSiExitProg (795);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}
 
    

LPVIEWPORT SetLastVP (LPVIEWPORT pVP)
{   
	if (pVP)
		while (pVP->DisplayInParent && pVP->Parent > 0)
			pVP = pViewports[pVP->Parent-1];
	return pVP;
}    

BOOL NeedFullRedisplay(void)
{
	return RedisplayOnly;
}

BOOL GetFormatRect (LPRECT pRect)
{   
	short	SaveConfig = CurrentConfig,i;
	BOOL	rtn=FALSE;
	
	SetConfig (1);  
	for (i=0;i<*pNumViewports;i++)
	{
		if (!pViewports[i]->Type)
		{
			*pRect = pViewports[i]->Rect;  
			rtn = TRUE;
			break;
		}
	}

	SetConfig (CurrentConfig);
	return rtn;
}

BOOL GetFormatDimensions (LPDOUBLE pWidth,LPDOUBLE pHeight)
{   
	short	SaveConfig = CurrentConfig,i;
	BOOL	rtn=FALSE;   
	
	SetConfig (1);  
	for (i=0;i<*pNumViewports;i++)
	{
		if (!pViewports[i]->Type)
		{
			*pWidth = pViewports[i]->DesiredWidth;  
			*pHeight = pViewports[i]->DesiredHeight;  
			rtn = TRUE;
			break;
		}
	}

	SetConfig (CurrentConfig);
	return rtn;
}



LPVIEWPORT SetVPFromName (LPSTR Arg2,LPBOOL pErr)
#if ENABLETRACE
{GSSiEnterProg (607);
#endif
{   
	short	iview;
	LPSTR	pC;
	
	*pErr = FALSE;
	if (!*Arg2 && CurView)
{
#if ENABLETRACE
GSSiExitProg (607);
#endif
		return CurView;  
}
	if (!CurView || !_fstricmp (Arg2,"COMMAND"))
	{
		SetViewport(*pCommandViewport);
{
#if ENABLETRACE
GSSiExitProg (607);
#endif
		return CurView;
}
	}
	if ((pC = strchr (Arg2,':')))
	{
		int config;

		*pC = 0;
		config = atoi (Arg2);
		*pC = ':';
		Arg2 = pC + 1;
		if (config == 0 || config == 1)
			SetConfig (config);
	}
Top:
	for (iview=0;iview<*pNumViewports;iview++)
	{   
		if (!_fstricmp (Arg2,pViewports[iview]->Name))
{
#if ENABLETRACE
GSSiExitProg (607);
#endif
    		return pViewports[iview];
}
	}
	if (!CurrentConfig)
	{
		SetConfig(1);
		goto Top;
	}
	*pErr = TRUE;
{
#if ENABLETRACE
GSSiExitProg (607);
#endif
	return CurView;
}
#if ENABLETRACE
}
#endif
} 

BOOL GetPMName (LPSTR PickMacroFile,LPSTR PMFile)
{  
	if (!_fstrnicmp (PickMacroFile,"FROM CMD",8))
		strncpy0 (PickMFile,pViewports[*pCommandViewport-1]->PickMacroFile,MAX_PATH-1);
	else
		strncpy0 (PickMFile,PickMacroFile,MAX_PATH-1);
	strcpy (PMFile,PickMFile);
	return TRUE;
}

BOOL SetViewport(int iview)
#if ENABLETRACE
{GSSiEnterProg (720);
#endif
{   
	short	ii;
	
	if (iview == -99) //first vp with orthos 
	{   
		SetViewport(*pCommandViewport);
    	if (CurView->HaveOrthos) 
    		goto RtnTrue;
		for (iview=0;iview<*pNumViewports;iview++)
		{   
	    	if (pViewports[iview]->HaveOrthos) 
	    	{
	    		SetCurView ( pViewports[iview]);
	    		goto RtnTrue;
	    	}    
		}
{
#if ENABLETRACE
GSSiExitProg (720);
#endif
		return FALSE;
}
	} 
	if (InDisplayProcessing)
		ii=1;  
	if (iview > 1000)
	{
		SetConfig (1);
		iview -= 1000;
	}
	if (iview > 0 && iview <= *pNumViewports) 
	{
   		SetCurView ( pViewports[iview-1]);   
   		goto RtnTrue;
   	}
   	if (*pNumViewports && *pCommandViewport <= *pNumViewports)
	{
		SetViewport(*pCommandViewport);
		goto RtnTrue; 
	}
{
#if ENABLETRACE
GSSiExitProg (720);
#endif
   	return FALSE;  
}
RtnTrue:   	 
    SetLocalProjection (&CurView->WBounds);
{
#if ENABLETRACE
GSSiExitProg (720);
#endif
    return TRUE;
}
#if ENABLETRACE
}
#endif
}

LPVIEWPORT GetVP(int iview)
#if ENABLETRACE
{GSSiEnterProg (721);
#endif
{   
	short	ii;
	LPVIEWPORT	NewView;
	
	if (iview > 0 && iview <= *pNumViewports) 
	{
   		NewView = pViewports[iview-1];   
{
#if ENABLETRACE
GSSiExitProg (721);
#endif
   		return NewView;
}
   	}
{
#if ENABLETRACE
GSSiExitProg (721);
#endif
   	return pViewports[*pCommandViewport-1];
}
#if ENABLETRACE
}
#endif
}

short SelectViewport (POINT MousePoint,BOOL MapsOnly, BOOL DigitizerInput,BOOL Config1Only)
#if ENABLETRACE
{GSSiEnterProg (91);
#endif
{    
    LPVIEWPORT  SaveView;
    short         ii,iview, gotvp, startvp, endvp, ConfigIn=CurrentConfig;
    
    if (IgnoreSelectVP)
{
#if ENABLETRACE
GSSiExitProg (91);
#endif
    	return TRUE;
}
    SaveView = CurView;
    if (MapsOnly || Config1Only)
    {
    	if (!CurrentConfig)
    		SetConfig (1);
    } 
    else if (PtInInfoBoxRect (MousePoint,&iview))
    {
    	SetViewport(iview);
    	return iview;
    }
    else
		SetConfigFromCursor (MousePoint);
//    CloseMap (FALSE);
    if (DigitizerInput)
    {   
    	SetViewport(*pCommandViewport);
    	CurVis = CurView->pVisList1;   
        SetFocus (CurView->hWnd); 
{
#if ENABLETRACE
GSSiExitProg (91);
#endif
        return CurView->ID;
}
    } 
    gotvp = -1;
    for (iview = 0; iview<*pNumViewports; iview++)  
    {
    	SetCurView ( pViewportsD[iview]);
    	if (CurView->DisplayedFullScreen)
    	{   
    		gotvp = iview;
    		break;
    	}
    } 
    if (gotvp > -1)
    	startvp = endvp = gotvp;
    else
    {
    	startvp = *pNumViewports-1;
    	endvp = 0;
    }
    for (iview = startvp;iview>=endvp; iview--)
    {   
    	SetCurView ( pViewportsD[iview]); 
        if (CurViewActive() && !CurView->DisplayInParent && (!MapsOnly ||
           (CurView->Type == PLANVIEWPORT || CurView->Type == INDEXVIEWPORT || CurView->Type == LEGENDIMAGEVIEWPORT)))
		{
			
			{
				if (PtInRect (&CurView->Rect,MousePoint))
				{   CurVis = CurView->pVisList1;   
					SetFocus (CurView->hWnd);
					SetViewport (CurView->ID);  
					if (!CurView->LinkedTo)
						RemoveLinkedCursors (); 
					if (LastVP && LastVP != CurView)
					{   
						DisplayProfileLoc (0,0,0,TRUE);
						DisplayProfileLink (0);
						NotifyFunction (LastVP,GF_EXIT_VIEWPORT);  
						LastVP = SetLastVP (CurView);
						NotifyFunction (LastVP,GF_ENTER_VIEWPORT);  
					} 
					else
						LastVP = SetLastVP (CurView);  
{
#if ENABLETRACE
GSSiExitProg (91);
#endif
            return CurView->ID;
}
				}
			}
        }
    }
    if (CurrentConfig != ConfigIn)
    	SetConfig (ConfigIn);
    SetCurView ( SaveView);
    RemoveLinkedCursors ();
    NotifyFunction (LastVP,GF_EXIT_VIEWPORT);     
    LastVP = 0; 
{
#if ENABLETRACE
GSSiExitProg (91);
#endif
    return FALSE;
}
#if ENABLETRACE
}
#endif
}



void DeleteAllVPRegions (void)
{   
	USHORT	i;
	
	if (!CurrentConfig)
		SetConfig (1); 
	for (i=0;i<*pNumViewports;i++)
		GSSiDeleteObject (&pViewports[i]->hRgn);
	return;
}



void RemoveVPBitmaps (void) 
{
	UINT	i;
	
	SetConfig (1);  
	for (i=0;i<*pNumViewports;i++)
	{
	 	DestroySavedScreen (&pViewports[i]->Bitmap,pViewports[i]->BitmapID);  
	} 
	return;
}

short GetLayerNumFromName (LPSTR LayerNameOrNum)
{
	short	i;
	
	for (i=0;i<CurView->NumFiles;i++)
	{  
		if (!_fstricmp (LayerNameOrNum,CurView->FileID[i]))
			return i+1;
	}
	return atoi (LayerNameOrNum);
} 

double	GetVPArea (short VPID,LPDOUBLE Perim,LPMNMXCORD pBounds)
#if ENABLETRACE
{GSSiEnterProg (1139);
#endif
{
	LPVIEWPORT SaveVP=CurView; 
	DPOINT	Points[4]; 
	double	Area;
	
	if (!SetViewport (VPID))
{
#if ENABLETRACE
GSSiExitProg (1139);
#endif
		return -1;
}
	BoundsToPoints (&CurView->NewBounds, Points,0); 
	if (pBounds)
		*pBounds = CurView->NewBounds;
	SetCurView ( SaveVP);
	Area = ComputeAreaAreaD (Points,4,Perim);
{
#if ENABLETRACE
GSSiExitProg (1139);
#endif
	return Area;
}
#if ENABLETRACE
}
#endif
}

int VPTypeFromChar (LPSTR CType)
{
	int	rtn = -1;
	int	ntypes = sizeof (VPCTypes) / 24;
	int	i;

	for (i=0;i<ntypes;i++)
		if (!stricmp (CType,VPCTypes[i]))
			rtn = i+1;

	return rtn;
}

BOOL GetVPCType (int i,LPSTR VPCType)
{
	int	ntypes = sizeof (VPCTypes) / 24;

	if (i < 1 || i > ntypes)
		return FALSE;

	strcpy (VPCType,VPCTypes[i-1]);
	return TRUE;
}

LPVIEWPORT CreateNullViewport (LPHANDLE phViewport,LPSTR Name,int ID,LPSTR ParentVP,LPSTR VPType)
{   
	LPVIEWPORT	pCurView;
	LPVISLIST	SaveVis=CurVis;
	HANDLE		hVis;
	BOOL		Err;
	
	*phViewport=GSSiGlobAlloc ( 128,GHND,sizeof(VIEWPORT)+MAX_VIEWPORT_FILES*MAX_PATH);
	pCurView = (LPVIEWPORT)GlobalLock (*phViewport);
	
	pCurView->ID = ID;  
	_fstrcpy(pCurView->Name,Name);
	pCurView->Version = CURRENT_VP_VERSION;
	pCurView->Parent = 0;
	pCurView->Type = 1;
	if (ParentVP)
	{
		LPVIEWPORT	pVP = SetVPFromName (ParentVP,&Err);

		if (!Err)
			pCurView->Parent = pVP->ID;
	}
	if (VPType)
		pCurView->Type = VPTypeFromChar (VPType);
	pCurView->Active = TRUE; 
	pCurView->DesiredHeight = 0;
	pCurView->DesiredWidth = 0;
	pCurView->BackGroundColor = RGB(255,255,255);
	pCurView->Shadow = FALSE;
	pCurView->Margin = 2;
	pCurView->MarginPan=TRUE;
	pCurView->TagPointID = 1;
	pCurView->TagPointType = 2;
	pCurView->TagPoint.x = 1;
	pCurView->TagPoint.y = 1;
	pCurView->WidthType = 2;
	pCurView->Width = 98;
	pCurView->Height = 98;
	pCurView->NewBounds.xmn = -10000;
	pCurView->NewBounds.xmx =  10000;
	pCurView->NewBounds.ymn = -10000;
	pCurView->NewBounds.ymx =  10000;
	pCurView->HaveBounds = FALSE;
    pCurView->WindowIsZoomed = FALSE; 
    pCurView->pTheme = 0;
	pCurView->NumThemes = 0;
	pCurView->NumFiles = 0;
	pCurView->FileType[0]=2;
	pCurView->lpFiles[0]=0;
	pCurView->NumVisList = 1;
	
	hVis = GSSiGlobAlloc ( 129,GHND,sizeof(VISLIST));

	CurVis = pCurView->pVisList1 = (LPVISLIST)GlobalLock (hVis); 
	CurVis->hVisList = hVis;
	InitVis ();
	
	pCurView->StartupFunction=GF_PAN_TO_POINT;
	_fstrcpy(pCurView->FunctionFile,"fundir\\appl1.txt");
	_fstrcpy(pCurView->FunctionDir,"index.txt");
	CurVis = SaveVis;
	return pCurView;
} 

void DestroyViewport (LPHANDLE phVP)
{   
	LPVIEWPORT	pVP;
	short	ifile,i;  
	
	if (*phVP)
	{
		LPVIEWPORT	pCurView = (LPVIEWPORT)GlobalLock (*phVP); 
    	HANDLE	SaveHandle = pCurView->FunStackHandle;
        HANDLE	hLast, handle;
            
	    while (pCurView->FunStackHandle)
	    { 
	    	LPCMDSTRING pCmdStr = (LPCMDSTRING)GlobalLock (pCurView->FunStackHandle);
	    	if (!pCmdStr)
	    	{
	    		pCurView->FunStackHandle	= 0;
	    		break;
	    	}
	    	if (pCmdStr->CurFun)
	    		ProcessGraphicsFunction3 (pCmdStr->CurFun,pCurView->hWnd,GF_CLOSE, 0,0); 	
	    	hLast = pCmdStr->PrevHandle; 
	    	GlobalUnlock (pCurView->FunStackHandle);
    		pCurView->FunStackHandle = hLast; 
    	} 
    	pCurView->FunStackHandle = SaveHandle;
        GSSiDeleteObject(&pCurView->hRgn);
    	ClearPolyOff (FALSE);
		ClearBackgroundArea ();
//        SaveDisplayRedefFile(pCurView->DisplayRedefFile);
        GSSiGlobFree (&pCurView->hPenRedef);
	 	DestroySavedScreen (&pCurView->Bitmap,pCurView->BitmapID);  
	 	pCurView->DisplayFunStack = 0; 
	 	DisplayFunctionStack ();
        for (ifile=0;ifile<pCurView->NumFiles;ifile++)
        {
            if (pCurView->FileType[ifile]>3 && pCurView->hlpIndex[ifile])
            {
                CloseMapIndex (pCurView->lpFiles[ifile],
                               pCurView->hlpIndex[ifile],FALSE,TRUE);
                pCurView->hlpIndex[ifile] = 0;
            }
        }
        CloseObject (pCurView->pTheme);  
       	GSSiGlobFree (&pCurView->ToolbarHandle);   
		for (i=0;i<MAXPROFILEROUTES;i++)
		{
			GSSiGlobFree (&pCurView->hProfileRoute[i]);
			GSSiGlobFree (&pCurView->hProfileElev[i]); 
		}
	    GSSiGlobFree (&pCurView->hProfileRouteSave);
		GSSiGlobFree (&pCurView->hDistanceLine);
        BoundsDisplayDestroy (pCurView->lpBoundsDisplay); 
        pCurView->lpBoundsDisplay=0;
        
        if (IsBadStringPtr((LPSTR)pCurView->pVisList1, sizeof(VISLIST)))
        	pCurView->pVisList1 = 0;
        CurVis = pCurView->pVisList1;
        if (pCurView->pVisListManual == CurVis)
            pCurView->pVisListManual=0;
        while (CurVis)
        {
            handle = CurVis->hVisList;
            CurVis =  (LPVISLIST) CurVis->NextVisList;
            GSSiGlobUlFree (&handle);
        }   
        CurVis = pCurView->pPickList1;  
        if (pCurView->pPickListManual == CurVis)
            pCurView->pPickListManual=0;
        while (CurVis)
        {
            handle = CurVis->hVisList;
            CurVis =  (LPVISLIST) CurVis->NextVisList;
            GSSiGlobUlFree (&handle);
        }   
        if (pCurView->pVisListManual)
        {
            handle = pCurView->pVisListManual->hVisList;
            GSSiGlobUlFree (&handle);
        }
        if (pCurView->pPickListManual)
        {
            handle = pCurView->pPickListManual->hVisList;
            GSSiGlobUlFree (&handle);
        } 
	 	CloseEditRect(pCurView);
        GSSiGlobFree (&pCurView->hTAGList); 
        UnloadReport (&pCurView->hReport);  
            
        DestroySavedScreen (&pCurView->LinkedCursorHandle,0);
        CloseTRANS2 (&pCurView->hTranVPToBase);
        CloseTRANS2 (&pCurView->hTranBaseToVP);
        CloseTRANS2 (&pCurView->hTranVPToScreen);
        CloseTRANS2 (&pCurView->hTranScreenToVP);
       	CloseTRANS2 (&pCurView->hFileTransIn); 
       	CloseTRANS2 (&pCurView->hFileTransOut);
	}
	GSSiGlobUlFree (phVP);
	return;
}

BOOL AddLayerToViewport (LPVIEWPORT pVP,LPSTR LayerName,LPSTR LayerPath)
{
	if (pVP->NumFiles >= MAX_VIEWPORT_FILES)
		return FALSE;
	strncpy (pVP->FileID[pVP->NumFiles],LayerName,MAX_VPFILE_ID);
	pVP->lpFiles[pVP->NumFiles]=&pVP->EndOfViewport + pVP->NumFiles * MAX_PATH;
	strncpy (pVP->lpFiles[pVP->NumFiles],LayerPath,MAX_PATH);
	CurView->FileType[pVP->NumFiles] = GetFileTypeFromName(LayerPath,FALSE);
	switch (CurView->FileType[pVP->NumFiles])
	{
		default:
			break;
		case 1:
			CurView->FileType[pVP->NumFiles] = 2;
			break;
		case 2:
			CurView->FileType[pVP->NumFiles] = 4;
			break;
		case 3:
			CurView->FileType[pVP->NumFiles] = 5; 
			break;
		case 4:
			CurView->FileType[pVP->NumFiles] = 3;
			break;
		case 8:
			CurView->FileType[pVP->NumFiles] = 9;
			break; 
	}
   	pVP->NumFiles++;
	return TRUE;
}

void CloseConnectedProcesses (void)
{
	if (!NumConnectedProcesses)
		return;
	return;
}

BOOL AddConnectedProcess (HWND hProcess,int opt)
{
	if (NumConnectedProcesses+1 >= MAX_CONNECTED_PROCESSES)
		return FALSE;
	hWndConnected[NumConnectedProcesses++] = hProcess;
	return TRUE;
}

BOOL ProcessConnectedCommand (UINT ID)
{
	UINT	rtn;
	HFILE	Fid;
	char	ConFile[MAX_PATH];
	char	TempDir[MAX_PATH];
	OFSTRUCTGM	OFStruct;
	HANDLE	hMem;
	LPSTR	pMem;
	int		lMem;

	GetTempPath (MAX_PATH,TempDir);
	rtn = GetTempFileName (TempDir,"gml",ID,ConFile); 
	if (!rtn)
		return FALSE;
	Fid = OpenFileGM (ConFile,&OFStruct,OF_READ);
	if (Fid == HFILE_ERROR)
		return FALSE;
    HaltMapDisplay(TRUE,FALSE);
	lMem = _llseek (Fid,0,2);
	_llseek (Fid,0,0);
	hMem = GSSiGlobAlloc (0,GMEM_MOVEABLE,lMem);
	pMem = GlobalLock (hMem);
	_lread (Fid,pMem,lMem);
	_lclose (Fid);
	ProcessText (pMem);
	GSSiGlobUlFree (&hMem);
	return TRUE;
}

void ZoomConnectedProcesses (BOOL Remove)
{
	int	i;
	HFILE	Fid;
	static	UINT	ID=0;
	static	char	ConFile[MAX_PATH] = { 0 };
	char	TempDir[MAX_PATH];
	char	Cmd[256];
	OFSTRUCTGM	OFStruct;
	
	if (!NumConnectedProcesses)
		return;
	if (Remove)
	{
		if (ID)
			GSSiRemove (ConFile);
		return;
	}
	if (!ID)
	{
		GetTempPath (MAX_PATH,TempDir);
		ID = GetTempFileName (TempDir,"gml",0,ConFile); 
	}
	Fid = OpenFileGM (ConFile,&OFStruct,OF_CREATE);
	sprintf (Cmd,"$ZOOM(POINTANDSCALE,%f %f,%f,F,COMMAND)",CurView->MidPointW.x,CurView->MidPointW.y,CurView->Scale);
	_lwrite (Fid,Cmd,strlen(Cmd)+1);
	_lclose (Fid);

	for (i=0;i<NumConnectedProcesses;i++)
		PostMessage(hWndConnected[i], GF_PROCESS_CONNECTED_CMD, ID,0); 
	return;
}    

void SendConnectedProcessCommand (HWND hProcessWnd,LPSTR cmd)
{
	int	i;
	HFILE	Fid;
	static	UINT	ID=0;
	static	char	ConFile[MAX_PATH];
	char	TempDir[MAX_PATH];
	char	Cmd[256];
	OFSTRUCTGM	OFStruct;
	
	if (!NumConnectedProcesses)
		return;
	if (!hProcessWnd)
	{
		if (ID)
			GSSiRemove (ConFile);
		return;
	}
	if (!ID)
	{
		GetTempPath (MAX_PATH,TempDir);
		ID = GetTempFileName (TempDir,"gml",0,ConFile); 
	}
	Fid = OpenFileGM (ConFile,&OFStruct,OF_CREATE);
	_lwrite (Fid,Cmd,strlen(cmd)+1);
	_lclose (Fid);

	PostMessage(hProcessWnd, GF_PROCESS_CONNECTED_CMD, ID,0); 
	return;
}    
void SendConnectedProcessMessage (HWND hProcessWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	PostMessage( hProcessWnd, msg, wParam,lParam); 
}

