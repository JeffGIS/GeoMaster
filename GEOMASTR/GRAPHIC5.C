#include "graphint.h"
#include "std.h"  

#include "gmextern.h"

#define	MAX_USE_SHIELDS	128

static	int	InterStateShield,USShield,StateShield,CountyHwyShield,CountyRdShield,TransCanShield,ProvinceRtShield;
static	LPSHORT	CARed, CAGreen, CABlue, CAIntensity;
static long		MaxRects;
static long		NumRects;  
static short	UseShieldSymbols[MAX_USE_SHIELDS]; 
static short	NumUseShields=0;   

BOOL SymbolInUseShieldsList (short symnum)
{   
	static	BOOL	First = TRUE; 
	UINT	i; 
	char	str[1024];
	
	if (symnum == -9999)
		First = TRUE;
	if (First)
	{   
		_fstrcpy (str,"[%DL]");
		ExpandText (str);
		if (!*str)
			return FALSE;
		First = FALSE; 
		NumUseShields = 0;
		GetGlobalCVal ("[%USESHIELDSYMBOLS]",str,"A10,A11,A12,A13,A15,A20,A21,A22,A23,A24,A25,A30,A31,A32,A33,A34,A35,DOTCLASS0,DOTCLASS1,DOTCLASS2,DOTCLASS3,DOTCLASS4,DOTCLASS5,DOTCLASS6,DOTCLASS7");
		{
			LPSTR pSym=str, pEnd;
			
			do 
			{
				if ((pEnd = _fstrchr (pSym,',')))
					*pEnd++ = 0;   
				UseShieldSymbols[NumUseShields++] = GetDictSymbolNumber(pSym);
				if (NumUseShields == MAX_USE_SHIELDS)
					break; 
				pSym = pEnd;
			}
			while (pEnd);
		}
	}
	for (i=0;i<NumUseShields;i++)
		if (symnum == UseShieldSymbols[i])
			return TRUE;
	return FALSE;
}



void AdjustColors (void)
#if ENABLETRACE
{GSSiEnterProg (723);
#endif
{
  DLGPROC lpfnCOLOR_ADJUSTMsgProc; 
  BOOL	rc;
  short red, green,blue, intensity; 
  
  red = GetGlobalLVal ("[%ORTHORED]");
  green = GetGlobalLVal ("[%ORTHOGREEN]");
  blue = GetGlobalLVal ("[%ORTHOBLUE]");
  intensity = GetGlobalLVal ("[%ORTHOINTENSITY]");
  CARed = &red;
  CABlue = &blue;
  CAGreen = &green;
  CAIntensity = &intensity;	
  lpfnCOLOR_ADJUSTMsgProc = MakeProcInstance((DLGPROC)COLOR_ADJUSTMsgProc, hInst);
  rc=DialogBox(hInst, (LPSTR)"COLOR_ADJUST", hWndMain, lpfnCOLOR_ADJUSTMsgProc);
  FreeProcInstance(lpfnCOLOR_ADJUSTMsgProc);
{
#if ENABLETRACE
GSSiExitProg (723);
#endif
  return;
}
#if ENABLETRACE
}
#endif
}




                    
 

void TrackLine (HDC hDC,DPOINT DPoint1, DPOINT DPoint2, LPHANDLE pHandle)
#if ENABLETRACE
{GSSiEnterProg (727);
#endif
{   POINT	Point1,Point2;
	typedef struct {DPOINT Point1, Point2;} LASTLINE;
	typedef LASTLINE	FAR	*LPLASTLINE;
	LPLASTLINE	pLastLine;  
	int	OldMode;
    
    
  	SetDisplayMode (hDC, GF_TEXTMODE);
	SelectClipRgn (hDC,0);
	OldMode = SetROP2(hDC,R2_NOT); 
	if (*pHandle)
	{
		pLastLine = (LPLASTLINE)GlobalLock(*pHandle);
		Point1 = BasePtToWinPt(&pLastLine->Point1);			
		Point2 = BasePtToWinPt(&pLastLine->Point2);			
		MoveToEx (hDC,Point1.x,Point1.y,0);
		LineTo (hDC,Point2.x,Point2.y); 
		GlobalUnlock (*pHandle); 
	} 
	else
		*pHandle = GSSiGlobAlloc ( 336,GMEM_MOVEABLE,sizeof(LASTLINE));
	pLastLine = (LPLASTLINE)GlobalLock(*pHandle); 
	pLastLine->Point1 = DPoint1;
	pLastLine->Point2 = DPoint2;
	Point1 = BasePtToWinPt(&pLastLine->Point1);			
	Point2 = BasePtToWinPt(&pLastLine->Point2);
	SelectObject (hDC,GetStockObject(BLACK_PEN));			
	MoveToEx (hDC,Point1.x,Point1.y,0);
	LineTo (hDC,Point2.x,Point2.y); 
	GlobalUnlock (*pHandle); 
	SetROP2(hDC,OldMode);
{
#if ENABLETRACE
GSSiExitProg (727);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}
  
HANDLE GetClipBitmap (HWND hWnd)
#if ENABLETRACE
{GSSiEnterProg (728);
#endif
{  	 
	LPBITMAPINFO	lpBM;
	HANDLE hDIB;

     if (!OpenClipboard (hWnd))
{
#if ENABLETRACE
GSSiExitProg (728);
#endif
     	return 0;
}
     hDIB = GetClipboardData (CF_DIB); 
     lpBM = (LPBITMAPINFO)GlobalLock(hDIB);
//     SaveDIB(hDIB,"C:\\TEST.BMP");
     GlobalUnlock(hDIB);
	 CloseClipboard();
{
#if ENABLETRACE
GSSiExitProg (728);
#endif
	 return hDIB;  
}
#if ENABLETRACE
}
#endif
} 
HANDLE GetNextHighlightArea (long AreaNum,LPMNMXCORD pBounds,LPINT pType,LPINT pNumPnts,LPINT pnPoly,LPHANDLE phPolyPartLen,LPDOUBLE pOffset,int OpenClose)
//OpenClose (0 open close on exit,1 open leave open on exit,2 use opened file,3 close file)
#if ENABLETRACE
{GSSiEnterProg (729);
#endif
{ 
	OFSTRUCTGM OFStruct;
	static	HFILE	FidAO; 
	long	Num=0;
	LPMNMXCORD	lpRect; 
	MNMXCORD	Rect;
	int	np, i;   
	long	Refno;
	short	ID;  
	LPDPOINT	lpDpoint;
	HANDLE	hArea=0, hMaskArea;  
	long	NumMaskPoints;
	LPSTR	pFile; 
	HPSTR	pNew,pMask;
	HIGHLIGHTAREAHEADER Header;
	static	HANDLE	hEntries=0; 
	static	int	NumEntries;
	LPHAINDEX	pEntries;
	short	Version;
	
	if (phPolyPartLen)
		*phPolyPartLen = 0;
	if (pnPoly )
		*pnPoly = 1;
	if (OpenClose == 3)
	{
		GSSiGlobFree (&hEntries);
		GSSiClose(FidAO);
		return 0;
	}
	*pType = 3;
    if (AreaNum == -1)
    {
    	if (!CurView)
    		return 0;
	    if (CurView->DisplayInParent && CurView->Parent>0)  
	    {
	    	hMaskArea = pViewports[CurView->Parent-1]->hMaskArea; 
	    	NumMaskPoints=pViewports[CurView->Parent-1]->NumMaskPoints;   
	    }
	    else
	    {
	    	hMaskArea = CurView->hMaskArea; 
	    	NumMaskPoints=CurView->NumMaskPoints;   
	    }
    	if (!hMaskArea || !NumMaskPoints)
    		return 0;
	    *pNumPnts = NumMaskPoints; 
	    hArea = GSSiGlobAlloc ( 337,GHND,sizeof(MNMXCORD)+NumMaskPoints*sizeof(DPOINT)); 
	    pNew = (HPSTR)GlobalLock (hArea);
	    pMask = (HPSTR)GlobalLock (hMaskArea);
	    hmemmove (pNew,pMask,sizeof(MNMXCORD)+NumMaskPoints*sizeof(DPOINT));
	    GlobalUnlock (hArea);
	    GlobalUnlock (hMaskArea);
{
#if ENABLETRACE
GSSiExitProg (729);
#endif
    	return hArea;  
}
	}    
    
    if (pBounds)
    {   
    	if (AreaNum)
{
#if ENABLETRACE
GSSiExitProg (729);
#endif
    		return hArea;
}
    	np = 4;
	    *pNumPnts = np;
	    hArea = GSSiGlobAlloc ( 337,GHND,sizeof(MNMXCORD)+np*sizeof(DPOINT));
	    lpRect = (LPMNMXCORD) GlobalLock (hArea);
	    *lpRect = *pBounds; 
	    lpRect++;
	    lpDpoint = (LPDPOINT) lpRect;  
	    lpDpoint->x = pBounds->xmn;
	    lpDpoint++->y = pBounds->ymn;
	    lpDpoint->x = pBounds->xmn;
	    lpDpoint++->y = pBounds->ymx;
	    lpDpoint->x = pBounds->xmx;
	    lpDpoint++->y = pBounds->ymx;
	    lpDpoint->x = pBounds->xmx;
	    lpDpoint->y = pBounds->ymn;
		GlobalUnlock (hArea);
{
#if ENABLETRACE
GSSiExitProg (729);
#endif
    	return hArea;  
}
    }
	if (!hAreaOffFile)
{
#if ENABLETRACE
GSSiExitProg (729);
#endif
    	return hArea;  
}
	if (OpenClose == 0 || OpenClose == 1)
	{
		FidAO = HFILE_ERROR;
		pFile = GlobalLock (hAreaOffFile); 
		if (!ExistFile(pFile))
		{
    		GlobalUnlock (hAreaOffFile);
	{
#if ENABLETRACE
GSSiExitProg (729);
#endif
    	return hArea;
}
		}  
		FidAO = GSSiOpenFile (pFile,&OFStruct,OF_READ);
   		GlobalUnlock (hAreaOffFile);
	    if (FidAO == HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (729);
#endif
    	return hArea;
}
		BigRead (FidAO,&Version,2);
		BigRead (FidAO,&NumEntries,4);
		hEntries = GSSiGlobAlloc (0,GMEM_MOVEABLE,NumEntries*sizeof(HAINDEX));
		pEntries = GlobalLock (hEntries);
		GSSillseek (FidAO,-NumEntries*sizeof(HAINDEX),2);
		BigRead (FidAO,pEntries,NumEntries*sizeof(HAINDEX));
		GlobalUnlock (hEntries);
	}
    if (FidAO == HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (729);
#endif
    	return hArea;
}
	pEntries = GlobalLock (hEntries);
	for (i=0;i<NumEntries;i++)
	{
	    if (CurView->ID == pEntries[i].VPID)
	    	Num++;  
	    if (Num == AreaNum)
		{
			GSSillseek (FidAO,pEntries[i].Offset,0);
			GlobalUnlock (hEntries);
			goto Load;
		}
	}
	GlobalUnlock (hEntries);
	goto Exit;
Load:
    if (BigRead (FidAO,(HPSTR)&Header,sizeof(HIGHLIGHTAREAHEADER)) == sizeof(HIGHLIGHTAREAHEADER))
    {
	    *pType = Header.Type;
		if (pOffset)
			*pOffset = Header.Offset;
	    hArea = GSSiGlobAlloc ( 338,GHND,sizeof(MNMXCORD)+(long)Header.np*sizeof(DPOINT));
	    *pNumPnts = Header.np;
	    lpRect = (LPMNMXCORD) GlobalLock (hArea);
	    *lpRect = Header.Bounds; 
	    lpRect++;
	    lpDpoint = (LPDPOINT) lpRect;  
	    
		BigRead (FidAO,(HPSTR)lpDpoint,(long)Header.np * sizeof(DPOINT)); 
		if (Header.nPoly > 1 && pnPoly && phPolyPartLen)
		{
			LPINT pPolyPartLen;

			*pnPoly = Header.nPoly;
			*phPolyPartLen = GSSiGlobAlloc (1780,GMEM_MOVEABLE,*pnPoly * sizeof(int));
			pPolyPartLen = GlobalLock (*phPolyPartLen);
			BigRead (FidAO,(HPSTR)pPolyPartLen,(long)Header.nPoly * sizeof(int)); 
			GlobalUnlock (*phPolyPartLen);
		}
		else if (Header.nPoly > 1)
			GSSillseek (FidAO,1,Header.nPoly * sizeof(int));
		GlobalUnlock (hArea);
	}
Exit:
	if (!OpenClose)
	{
		GSSiGlobFree (&hEntries);
		GSSiClose(FidAO);
	}
{
#if ENABLETRACE
GSSiExitProg (729);
#endif
	return hArea; 
}
#if ENABLETRACE
}
#endif
}

BOOL CreateHighlightOutput (HWND hWnd, HWND StatusWnd,LPSTR outPath,BOOL tabDlm)
#if ENABLETRACE
{GSSiEnterProg (730);
#endif
{   long	nrecs;
	HCURSOR	hcurSave;
	char	str[256], fullp[128]; 
	LPSTR	pFile; 
	BOOL	OutToScreen=FALSE;
	
	hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT)); 
    GetGlobalCVal ("[%HLTOUTPUTFILE]",HLTOutPath,outPath);
    if (_fstricmp(HLTOutPath,"SCREEN")) 
    {
    	_fstrcpy(HLTOutPathScreen,HLTOutPath);  
    	if (!_fstrchr (HLTOutPathScreen,'.'))
    		_fstrcat (HLTOutPathScreen,".txt");
    }
    else 
    {   
    	if (!hScreenFile)
    	{
	    	hScreenFile = GSSiGlobAlloc ( 339,GMEM_MOVEABLE,256);
    		pFile = GlobalLock (hScreenFile);
        	GSSiGetTempFileName (0,"gm",0,pFile); 
	    }
	    else
    		pFile = GlobalLock (hScreenFile);
    	_fstrcpy(HLTOutPathScreen,pFile);  
    	GlobalUnlock (hScreenFile);  
    	OutToScreen = TRUE;
    }
    switch (HLTOUTFormat)
    {
    	case 0:
			nrecs = OutputToFile (HLTOutPathScreen,TRUE,HLTOutDataFile, HLTOutSQL, HLTOutFields,0,0,0,TRUE,FALSE,OutToScreen,FALSE,FALSE,0,StatusWnd,hWnd,tabDlm);
			break;
		case 1:
			nrecs = OutputHLTAreas (HLTOUTFormat,HLTOutPath);
			break;
	}
	GSSiSetCursor (hcurSave);
	
	UGridPickMacro[0]=0;
    if (_fstricmp(HLTOutPath,"SCREEN")) 
    {   
    	if (hWnd)
    	{   
    		ExpandText (HLTOutPathScreen);
			sprintf (str,"%ld records written to %s",nrecs,_fullpath(fullp,HLTOutPathScreen,sizeof(fullp)));
			GSSiMsgBox (hWndMain,str," ",0,0);
		} 
	}
	else 
        ShowGrid(hWnd,0,0);
{
#if ENABLETRACE
GSSiExitProg (730);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}                               

BOOL SetStuffFromText (LPSTR Text,int MaxTextLen,LPHANDLE phStuff)
{
	int lText = max (strlen (Text)+1,MaxTextLen+1);
	LPSHORT	pStuff, pStuff0, pStuffL, StuffLoc;  
	LPSTR	pCmd;
		

	*phStuff = 0;

	if (lText < 2)
		return FALSE;

	lText += lText % 2;
	if (lText > 256)
		return FALSE;
	*phStuff = GSSiGlobAlloc ( 342,GHND,lText+6);
	pStuff = (LPSHORT)GlobalLock (*phStuff); 
	pStuff0 = pStuff++; 
	*pStuff++=40;
	pStuffL = pStuff++;
	pCmd = (LPSTR)pStuff;
	_fstrcpy (pCmd,Text);  
	*pStuffL = lText;
	*pStuff0 = 2+2+*pStuffL; 
	GlobalUnlock (*phStuff);
	return TRUE;
}
	

BOOL SetNewMacro (short Type)
#if ENABLETRACE
{GSSiEnterProg (731);
#endif
{                 
	HANDLE	hGRCmd=GSSiGlobAlloc ( 340,GMEM_MOVEABLE,2048);
	LPSTR	pGRCmd=GlobalLock (hGRCmd);  
	LPVIEWPORT	SaveVP=CurView;  
	BOOL	rtn=TRUE;
		
	switch (Type)
	{
		case 1:
			_fstrcpy (pGRCmd,"[%NEW_POINT_MACRO]");
			break; 
		case 2:
			_fstrcpy (pGRCmd,"[%NEW_LINE_MACRO]");
			break;
		case 3:
			_fstrcpy (pGRCmd,"[%NEW_AREA_MACRO]");
			break; 
	}
	ExpandText (pGRCmd);
	if (!ContinueProcessing)
	{
		rtn = FALSE; 
		SetContinueProcessing ( TRUE);
	}
	GSSiGlobUlFree (&hGRCmd); 
	SetCurView ( SaveVP);
{
#if ENABLETRACE
GSSiExitProg (731);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}

BOOL SetNewStuff (short Type,LPHANDLE hStuff)
#if ENABLETRACE
{GSSiEnterProg (732);
#endif
{                 
	LPSHORT	pStuff, pStuff0, pStuffL, StuffLoc;   
	HANDLE	hGRCmd=GSSiGlobAlloc ( 341,GMEM_MOVEABLE,1024);
	LPSTR	pCmd, pGRCmd=GlobalLock (hGRCmd);  
	short	MaxCMDLen;  
	LPVIEWPORT	SaveVP=CurView;
		
	*hStuff = GSSiGlobAlloc ( 342,GHND,1024);
	pStuff = (LPSHORT)GlobalLock (*hStuff); 
	pStuff0 = pStuff++;  
	switch (Type)
	{
		case 1:
			_fstrcpy (pGRCmd,"[%NEW_POINT_GRCMD]");
		 	MaxCMDLen = GetGlobalLVal ("[%NEW_POINT_MAXCMD]"); 
			break; 
		case 2:
			_fstrcpy (pGRCmd,"[%NEW_LINE_GRCMD]");
		 	MaxCMDLen = GetGlobalLVal ("[%NEW_LINE_MAXCMD]"); 
			break;
		case 3:
			_fstrcpy (pGRCmd,"[%NEW_AREA_GRCMD]");
		 	MaxCMDLen = GetGlobalLVal ("[%NEW_AREA_MAXCMD]"); 
			break; 
	}
	ExpandText (pGRCmd);
	*pStuff++=40;
	pStuffL = pStuff++;
	pCmd = (LPSTR)pStuff;
	_fstrcpy (pCmd,pGRCmd);  
	SetContinueProcessing ( TRUE);
//	ExpandText (pCmd);        
	if (!ContinueProcessing) 
	{
		GSSiGlobUlFree (hStuff);
		GSSiGlobUlFree (&hGRCmd); 
		SetCurView ( SaveVP);
{
#if ENABLETRACE
GSSiExitProg (732);
#endif
		return FALSE;
}
	}
	*pStuffL = _fstrlen (pCmd);
	*pStuffL += *pStuffL%2;
	*pStuffL = min (256,max (MaxCMDLen,*pStuffL+4));
	*pStuff0 = 2+2+*pStuffL;    
	
	if (Type == 2 && GetGlobalBVal2 ("[%NEW_LINE_ADDSTREETS]",FALSE))
	{
		StuffLoc = pStuff0;
	    StuffLoc += (*pStuff0/2)+1;
	    (*pStuff0) += 18;
	    *StuffLoc++ = 10;
	//    _fmemmove (StuffLoc,StreetNums,16);
    } 
	GlobalUnlock (*hStuff);
	GSSiGlobUlFree (&hGRCmd); 
	SetCurView ( SaveVP);
{
#if ENABLETRACE
GSSiExitProg (732);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
} 

BOOL SetNewText (short InType,LPHANDLE hText,LPHANDLE hTextTPL,LPDPOINT pTPLOffsetPoint)
#if ENABLETRACE
{GSSiEnterProg (733);
#endif
{                 
	LPVIEWPORT	SaveVP=CurView;
	LPGRTEXT	lpGRText; 
	LPUMTEXTTPL	lpTextTPL;
	DPOINT		DPoint; 
	short		style,Type = abs (InType);
	char		str[256];
	
	*hText = GSSiGlobAlloc ( 343,GHND,sizeof(GRTEXT));
   	lpGRText = (LPGRTEXT)GlobalLock (*hText); 
	lpGRText->version = 1;    
	lpGRText->length = sizeof(GRTEXT);   
	switch (Type)
	{
		case 1:
	   	lpGRText->vJust = GetGlobalLVal2 ("[%NEW_POINT_TEXT_VJUST]",1);
	   	lpGRText->hJust = GetGlobalLVal2 ("[%NEW_POINT_TEXT_HJUST]",1);
	    lpGRText->ltext = GetGlobalLVal2 ("[%NEW_POINT_TEXT_MAXLEN]",-1); 
	    lpGRText->weight = GetGlobalLVal2 ("[%NEW_POINT_TEXT_WEIGHT]",1); 
	    lpGRText->italic = GetGlobalBVal2 ("[%NEW_POINT_TEXT_ITALIC]",FALSE); 
	    lpGRText->FontNum = GetGlobalLVal2 ("[%NEW_POINT_FONTNUM]",0);
	    lpGRText->FlipForEasyReading = GetGlobalBVal2 ("[%NEW_POINT_EZREAD]",TRUE); 
	    if (!lpGRText->ltext || !GetGlobalBVal ("[%NEW_POINT_WANTTEXT]"))
	    {
	    	GSSiGlobUlFree (hText);  
			SetCurView ( SaveVP);
{
#if ENABLETRACE
GSSiExitProg (733);
#endif
	    	return TRUE;
}
	    }
	    if (InType > 0) 
	    	GetGlobalCVal ("[%NEW_POINT_TEXT_HEIGHT]",str,0);
	    else
		    ExpandGlobalRaw ("%NEW_POINT_TEXT_HEIGHT",TRUE,str,256);
	    if (*str)
	    	_fstrcpy (lpGRText->cHeight,str);
	    else
	    	_fstrcpy (lpGRText->cHeight,"0.2"); 
	    if (InType > 0) 
	    	GetGlobalCVal ("[%NEW_POINT_TEXT_COLOR]",str,0);
	    else
		    ExpandGlobalRaw ("%NEW_POINT_TEXT_COLOR",TRUE,str,256);
    	_fstrcpy (lpGRText->cColor,str);
	    if (InType > 0)
	    	GetGlobalCVal ("[%NEW_POINT_TEXT]",str,0);
	    else
		    ExpandGlobalRaw ("%NEW_POINT_TEXT",TRUE,str,256);
	    _fstrcpy (lpGRText->Text,str);
	    style = GetGlobalLVal2 ("[%NEW_POINT_TPL_STYLE]",0);
	    if (style)        
	    {
			*hTextTPL = GSSiGlobAlloc ( 344,GHND,sizeof(UMTEXTTPL));
		   	lpTextTPL = (LPUMTEXTTPL)GlobalLock (*hTextTPL); 
		   	if (pTPLOffsetPoint)
		   	{
				GetGlobalPVal ("[%NEW_POINT_TPL_POINT1]",0,&DPoint);
				lpTextTPL->points[0] = DPointToFPoint (SubtractPoint (&DPoint,pTPLOffsetPoint));
				GetGlobalPVal ("[%NEW_POINT_TPL_POINT2]",0,&DPoint);
				lpTextTPL->points[1] = DPointToFPoint (SubtractPoint (&DPoint,pTPLOffsetPoint)); 
			}
			lpTextTPL->style1 = style;    
			GlobalUnlock (*hTextTPL);  
	    }
	    else
	    	*hTextTPL = 0;
	    break;
		case 2: 
	   	lpGRText->vJust = GetGlobalLVal ("[%NEW_LINE_TEXT_VJUST]");
	   	lpGRText->hJust = GetGlobalLVal ("[%NEW_LINE_TEXT_HJUST]");
	    lpGRText->ltext = GetGlobalLVal2 ("[%NEW_LINE_TEXT_MAXLEN]",-1); 
	    lpGRText->weight = GetGlobalLVal2 ("[%NEW_LINE_TEXT_WEIGHT]",1); 
	    lpGRText->italic = GetGlobalBVal2 ("[%NEW_LINE_TEXT_ITALIC]",FALSE); 
	    lpGRText->FontNum = GetGlobalLVal2 ("[%NEW_LINE_FONTNUM]",0); 
	    lpGRText->FlipForEasyReading = GetGlobalBVal2 ("[%NEW_LINE_EZREAD]",TRUE); 
	    if (!lpGRText->ltext || !GetGlobalBVal ("[%NEW_LINE_WANTTEXT]"))
	    {
	    	GSSiGlobUlFree (hText);  
			SetCurView ( SaveVP);
{
#if ENABLETRACE
GSSiExitProg (733);
#endif
	    	return TRUE;
}
	    } 
	    if (InType > 0)
	    	GetGlobalCVal ("[%NEW_LINE_TEXT_HEIGHT]",str,0);
	    else
		    ExpandGlobalRaw ("%NEW_LINE_TEXT_HEIGHT",TRUE,str,256);
	    if (*str)
	    	_fstrcpy (lpGRText->cHeight,str);
	    else
	    	_fstrcpy (lpGRText->cHeight,"0.2");
	    if (InType > 0)
	    	GetGlobalCVal ("[%NEW_LINE_TEXT_COLOR]",str,0);
	    else
		    ExpandGlobalRaw ("%NEW_LINE_TEXT_COLOR",TRUE,str,256);
    	_fstrcpy (lpGRText->cColor,str);
	    if (InType > 0) 
	    	GetGlobalCVal ("[%NEW_LINE_TEXT]",str,0); 
	    else
		    ExpandGlobalRaw ("%NEW_LINE_TEXT",TRUE,str,256);
	    _fstrcpy (lpGRText->Text,str);
	    style = GetGlobalLVal2 ("[%NEW_LINE_TPL_STYLE]",0);
	    if (style)        
	    {
			*hTextTPL = GSSiGlobAlloc ( 345,GHND,sizeof(UMTEXTTPL));
		   	lpTextTPL = (LPUMTEXTTPL)GlobalLock (*hTextTPL); 
			GetGlobalPVal ("[%NEW_LINE_TPL_POINT1]",0,(LPDPOINT)&lpTextTPL->points[0]);
			GetGlobalPVal ("[%NEW_LINE_TPL_POINT2]",0,(LPDPOINT)&lpTextTPL->points[1]);
			lpTextTPL->style1 = style;    
			GlobalUnlock (*hTextTPL);  
	    }
	    else
	    	*hTextTPL = 0;
	    break;
		case 3:
	   	lpGRText->vJust = GetGlobalLVal ("[%NEW_AREA_TEXT_VJUST]");
	   	lpGRText->hJust = GetGlobalLVal ("[%NEW_AREA_TEXT_HJUST]");
	    lpGRText->ltext = GetGlobalLVal2 ("[%NEW_AREA_TEXT_MAXLEN]",-1); 
	    lpGRText->weight = GetGlobalLVal2 ("[%NEW_AREA_TEXT_WEIGHT]",1); 
	    lpGRText->italic = GetGlobalBVal2 ("[%NEW_AREA_TEXT_ITALIC]",FALSE); 
	    lpGRText->FontNum = GetGlobalLVal2 ("[%NEW_AREA_FONTNUM]",0); 
	    lpGRText->FlipForEasyReading = GetGlobalBVal2 ("[%NEW_AREA_EZREAD]",TRUE); 
	    if (!lpGRText->ltext || !GetGlobalBVal ("[%NEW_AREA_WANTTEXT]"))
	    {
	    	GSSiGlobUlFree (hText);  
			SetCurView ( SaveVP);
{
#if ENABLETRACE
GSSiExitProg (733);
#endif
	    	return TRUE;
}
	    } 
	    if (InType > 0)
	    	GetGlobalCVal ("[%NEW_AREA_TEXT_HEIGHT]",str,0);
	    else
		    ExpandGlobalRaw ("%NEW_AREA_TEXT_HEIGHT",TRUE,str,256);
	    if (*str)
	    	_fstrcpy (lpGRText->cHeight,str);
	    else
	    	_fstrcpy (lpGRText->cHeight,"0.2");
	    if (InType > 0)
	    	GetGlobalCVal ("[%NEW_AREA_TEXT_COLOR]",str,0);
	    else
		    ExpandGlobalRaw ("%NEW_AREA_TEXT_COLOR",TRUE,str,256);
    	_fstrcpy (lpGRText->cColor,str);
	    if (InType > 0) 
	    	GetGlobalCVal ("[%NEW_AREA_TEXT]",str,0); 
	    else
		    ExpandGlobalRaw ("%NEW_AREA_TEXT",TRUE,str,256);
	    _fstrcpy (lpGRText->Text,str);
	    style = GetGlobalLVal2 ("[%NEW_AREA_TPL_STYLE]",0);
	    if (style)        
	    {
			*hTextTPL = GSSiGlobAlloc ( 345,GHND,sizeof(UMTEXTTPL));
		   	lpTextTPL = (LPUMTEXTTPL)GlobalLock (*hTextTPL); 
			GetGlobalPVal ("[%NEW_AREA_TPL_POINT1]",0,(LPDPOINT)&lpTextTPL->points[0]);
			GetGlobalPVal ("[%NEW_AREA_TPL_POINT2]",0,(LPDPOINT)&lpTextTPL->points[1]);
			lpTextTPL->style1 = style;    
			GlobalUnlock (*hTextTPL);  
	    }
	    else
	    	*hTextTPL = 0;
	    break;
	}
   	GlobalUnlock (*hText);  
	SetCurView ( SaveVP);
{
#if ENABLETRACE
GSSiExitProg (733);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}  

BOOL GetNewText (short Type,HANDLE hText,HANDLE hTextTPL)
#if ENABLETRACE
{GSSiEnterProg (734);
#endif
{                 
	LPVIEWPORT	SaveVP=CurView;
	LPGRTEXT	lpGRText; 
	LPUMTEXTTPL	lpTextTPL; 
	short		style;
	char		str[64];
	
	if (!hText)
{
#if ENABLETRACE
GSSiExitProg (734);
#endif
		return FALSE;
}
   	lpGRText = (LPGRTEXT)GlobalLock (hText); 
	switch (Type)
	{
		case 1:    
		SetGlobalValueLong ("%NEW_POINT_TEXT_VJUST",lpGRText->vJust);
		SetGlobalValueLong ("%NEW_POINT_TEXT_HJUST",lpGRText->hJust);
		SetGlobalValueLong ("%NEW_POINT_TEXT_MAXLEN",lpGRText->ltext);
	    SetGlobalValue ("%NEW_POINT_TEXT_HEIGHT",lpGRText->cHeight);
	    SetGlobalValue ("%NEW_POINT_TEXT",lpGRText->Text);
		SetGlobalValueLong ("%NEW_POINT_FONTNUM",lpGRText->FontNum);
	    SetGlobalValueLong ("%NEW_POINT_EZREAD",lpGRText->FlipForEasyReading); 
   
	    if (!hTextTPL) 
	    	SetGlobalValueLong ("%NEW_POINT_TPL_STYLE",0);
	    else
	    {
		   	lpTextTPL = (LPUMTEXTTPL)GlobalLock (hTextTPL); 
	    	SetGlobalValueLong ("%NEW_POINT_TPL_STYLE",lpTextTPL->style1);
//			lpTextTPL->points[0] = GetGlobalPVal ("[%NEW_POINT_TPL_POINT1]",0);
//			lpTextTPL->points[1] = GetGlobalPVal ("[%NEW_POINT_TPL_POINT2]",0);
			GlobalUnlock (hTextTPL);  
	    }
	    break;
		case 2:    
		SetGlobalValueLong ("%NEW_LINE_TEXT_VJUST",lpGRText->vJust);
		SetGlobalValueLong ("%NEW_LINE_TEXT_HJUST",lpGRText->hJust);
		SetGlobalValueLong ("%NEW_LINE_TEXT_MAXLEN",lpGRText->ltext);
	    SetGlobalValue ("%NEW_LINE_TEXT_HEIGHT",lpGRText->cHeight);
	    SetGlobalValue ("%NEW_LINE_TEXT",lpGRText->Text);   
		SetGlobalValueLong ("%NEW_LINE_FONTNUM",lpGRText->FontNum);
	    SetGlobalValueLong ("%NEW_LINE_EZREAD",lpGRText->FlipForEasyReading); 
	    if (!hTextTPL) 
	    	SetGlobalValueLong ("%NEW_LINE_TPL_STYLE",0);
	    else
	    {
		   	lpTextTPL = (LPUMTEXTTPL)GlobalLock (hTextTPL); 
	    	SetGlobalValueLong ("%NEW_LINE_TPL_STYLE",lpTextTPL->style1);
//			lpTextTPL->points[0] = GetGlobalPVal ("[%NEW_POINT_TPL_POINT1]",0);
//			lpTextTPL->points[1] = GetGlobalPVal ("[%NEW_POINT_TPL_POINT2]",0);
			GlobalUnlock (hTextTPL);  
	    }
	    break;
	    
		case 3:    
		SetGlobalValueLong ("%NEW_AREA_TEXT_VJUST",lpGRText->vJust);
		SetGlobalValueLong ("%NEW_AREA_TEXT_HJUST",lpGRText->hJust);
		SetGlobalValueLong ("%NEW_AREA_TEXT_MAXLEN",lpGRText->ltext);
	    SetGlobalValue ("%NEW_AREA_TEXT_HEIGHT",lpGRText->cHeight);
	    SetGlobalValue ("%NEW_AREA_TEXT",lpGRText->Text);   
		SetGlobalValueLong ("%NEW_AREA_FONTNUM",lpGRText->FontNum);
	    SetGlobalValueLong ("%NEW_AREA_EZREAD",lpGRText->FlipForEasyReading); 
	    if (!hTextTPL) 
	    	SetGlobalValueLong ("%NEW_AREA_TPL_STYLE",0);
	    else
	    {
		   	lpTextTPL = (LPUMTEXTTPL)GlobalLock (hTextTPL); 
	    	SetGlobalValueLong ("%NEW_AREA_TPL_STYLE",lpTextTPL->style1);
//			lpTextTPL->points[0] = GetGlobalPVal ("[%NEW_POINT_TPL_POINT1]",0);
//			lpTextTPL->points[1] = GetGlobalPVal ("[%NEW_POINT_TPL_POINT2]",0);
			GlobalUnlock (hTextTPL);  
	    }
	    break;
	    
	}
   	GlobalUnlock (hText);  
{
#if ENABLETRACE
GSSiExitProg (734);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}  

HANDLE SetTimeStampHandle (LPSTR pTimeStamp)
{
	HANDLE hTimeStamp=0;
	LPTIMESTAMP	lpTimeStamp;
	LPSTR	pDash;

	if (*pTimeStamp)
	{
		pDash = strchr (pTimeStamp,'-');
		if (pDash)
			*pDash = 0;
		hTimeStamp = GSSiGlobAlloc ( 346,GHND,sizeof(TIMESTAMP));
		lpTimeStamp = (LPTIMESTAMP)GlobalLock (hTimeStamp); 
		lpTimeStamp->version = 1;
		lpTimeStamp->length = sizeof(TIMESTAMP);
		strcpy (lpTimeStamp->StartTime,pTimeStamp);
		if (pDash)
		{
			strcpy (lpTimeStamp->EndTime,pDash+1);
			*pDash = '-';
		}
		GlobalUnlock (hTimeStamp);
	}
	return hTimeStamp;
}

BOOL SetNewTime (short Type,LPHANDLE hTimeStamp)
#if ENABLETRACE
{GSSiEnterProg (735);
#endif
{                 
	LPTIMESTAMP	lpTimeStamp; 
	BOOL		rtn;
	
	GSSiGlobFree (hTimeStamp);

	*hTimeStamp = GSSiGlobAlloc ( 346,GHND,sizeof(TIMESTAMP));
	lpTimeStamp = (LPTIMESTAMP)GlobalLock (*hTimeStamp); 
	lpTimeStamp->version = 1;
	lpTimeStamp->length = sizeof(TIMESTAMP);
	switch (Type)
	{
		case 1:
    		_fstrcpy (lpTimeStamp->StartTime,"[%NEW_POINT_START_TIME]");
    		_fstrcpy (lpTimeStamp->EndTime,"[%NEW_POINT_END_TIME]");
    		break;
		case 2:
    		_fstrcpy (lpTimeStamp->StartTime,"[%NEW_LINE_START_TIME]");
    		_fstrcpy (lpTimeStamp->EndTime,"[%NEW_LINE_END_TIME]");
    		break;
		case 3:
    		_fstrcpy (lpTimeStamp->StartTime,"[%NEW_AREA_START_TIME]");
    		_fstrcpy (lpTimeStamp->EndTime,"[%NEW_AREA_END_TIME]");
    		break;
    }
    ExpandText (lpTimeStamp->StartTime);
    if (!ContinueProcessing || !*lpTimeStamp->StartTime)
		GSSiGlobUlFree (hTimeStamp);
	else 
    {   
	    ExpandText (lpTimeStamp->EndTime);
		GlobalUnlock (*hTimeStamp);
		if (!ContinueProcessing)
			GSSiGlobFree (hTimeStamp);
	}
	rtn = ContinueProcessing;
	SetContinueProcessing ( TRUE);
{
#if ENABLETRACE
GSSiExitProg (735);
#endif
    return rtn;
}
#if ENABLETRACE
}
#endif
}

BOOL ShowCurrentDist (short From,LPDPOINT LastPoint,LPDPOINT AtPoint,double TotDist)
#if ENABLETRACE
{GSSiEnterProg (736);
#endif
{   
	short	itheme;
	LPVIEWPORT	SaveVP=CurView;
	double	ThisDist=0, AZ=0;   
	
	if (From == 2) 
	{
		ThisDist = ldistp (*AtPoint,*LastPoint); 
		AZ = getazd (LastPoint,AtPoint); 
	}
	for (itheme=0;itheme<CurView->NumThemes;itheme++)
	{
		CurTheme=CurView->pThemes[itheme];
		if (CurTheme->IsActive &&
		    CurTheme->VPDisplayed &&
		    CurTheme->ID == GF_DISTANCE_THEME) 
		{   
			SetViewport (CurTheme->DisplayViewport);  
    		DisplayDistanceThemeLegend (From,ThisDist,AZ,TotDist);     
    	}
    } 
    SetCurView ( SaveVP);
{
#if ENABLETRACE
GSSiExitProg (736);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
} 

short CompressCoordinates (LPUSHORT pNpnts,HPDPOINT lpDPoint,LPHANDLE phCompressedCoord)
{   
	MNMXCORD	Bounds;    
	USHORT		i;
	double		MaxD, Units;
	LPDOUBLE	pAccuracy;
	LPDPOINT	pBasePoint;
	HPSHORT		pOffsets2;
	HPLONG		pOffsets4; 
	long		n=0, size;
	 
	*phCompressedCoord = 0;
	if (!CompressAccuracy)	
		return 0;
	DBoundsInit (&Bounds);
	for (i=0;i<*pNpnts;i++) 
		 AddDPointToMinMax (&lpDPoint[i],&Bounds);
	MaxD = max (Bounds.xmx - Bounds.xmn,Bounds.ymx - Bounds.ymn);
	Units = MaxD / CompressAccuracy;
	if (Units < (double)USHRT_MAX)
	{   
		size = sizeof(double)+(long)sizeof(DPOINT)+(long)(1+(*pNpnts)*2)*(long)sizeof(short); 
		if (size%16)
			size += 16 - (size%16);
		*phCompressedCoord = GSSiGlobAlloc (0,GMEM_MOVEABLE,size);
		pAccuracy = (LPDOUBLE)GlobalLock (*phCompressedCoord);
		*pAccuracy = CompressAccuracy;
		pBasePoint = (LPDPOINT)(pAccuracy + 1); 
		*pBasePoint = MinMaxMidPointD (&Bounds);
		pOffsets2 = (HPSHORT)(pBasePoint + 1); 
		*pOffsets2++ = *pNpnts;
		for (i=0;i<*pNpnts;i++)
		{
			pOffsets2[n++] = (lpDPoint[i].x - pBasePoint->x)/CompressAccuracy;
			pOffsets2[n++] = (lpDPoint[i].y - pBasePoint->y)/CompressAccuracy;
		}
		GlobalUnlock (*phCompressedCoord); 
		*pNpnts = size/16;
		return 2;
	}
	else if (Units < (double)LONG_MAX)
	{
		return 4;
	}
	return 0;
}

BOOL AddPolyToMap (int nPolyIn, LPINT lpnPntsIn,LPHANDLE phDPoints,int Type, long NewRefno,HANDLE hTimeStamp,int ipen, int idesc,LPSHORT Stuff,
					LPSTR Prefix, LPSTR UDI,long AreaColor,long PenColorIn, int PenWidthIn,
					HANDLE hGRText,HANDLE hTextTPL,int nCurvePoints, LPSHORT CurvePoints,BOOL HiPrecis,LPDOUBLE pRouteOffset)
#if ENABLETRACE
{GSSiEnterProg (737);
#endif
{   
	HANDLE	hPoints, hDPoints, hhPoints=0,hnPoints=0; 
	LPHANDLE	phPoints, lphDPoints; 
	LPINT	pnPnts, lpnPnts,pnPoints;
	HPPOINTS	lpPoints, lpPoint;
    HPDPOINT lpDPoint;  
   	DPOINT LastDPoint, FirstDPoint, TXPoint, DPoint; 
   	double	rot;
   	POINTS	LastPoint;
	POINT	Point;
    HANDLE	hBuf, hRec;
    HPSTR	pBuf, pBufBeg, MinMaxLoc;
	char	Text[256];
	int i, j;
	short id, ii, itemlen;
	int	  nPoly;
	mnmxCor	MinMax; 
	int	TotPoints=0; 
	long	TotP=0, nElevPts=0;
	long	Offset, CurRecLenLoc, loc, endsegloc, PenColor,lbuf=0, RecLen, item_len, PointSize=4;  
	short	PenWidth=PenWidthIn;
	static	long	lastloc,iii=-1;
	int		CurRecLen, ltag,  nsame=0;  
	char	tag[80];
	LPSTR	Rec; 
	BOOL	First; 
	LPGRTEXT		pGRText;
	LPUMTEXTTPL		pTextTPL;
	GRTEXTHEADER	GRTextHead;
   	DPOINT POC,PT;
	long	lbuftemp=0, TextColor=0;   
	HANDLE	hTemp=0;
	BOOL	Deleted=FALSE; 
	short	TypeLevel=Type;  
	BYTE	idbyte, compression;
   	HANDLE	hCompressedCoord;
	BOOL	Opened;
	
	if (Type == 4) //shapes that go on line level
	{
		TypeLevel = 1;
		Type = 0;
	}
	if (pRouteOffset) //lines on area level (routes)
	{
		HiPrecis = TRUE;
		TypeLevel = 0;
	}
	if (!nPolyIn)
		Deleted = TRUE;
	
    ForceRefIndex = TRUE; 
    ForceTAGIndex = TRUE; 
    if (HiPrecis)
    	PointSize = 16;

//	CurView->HaveBounds=FALSE; 
	PltType = 2;
	if (!idesc || !OpenMap (0,0))
	{
	    ForceRefIndex = FALSE; 
	    ForceTAGIndex = FALSE; 
{
#if ENABLETRACE
GSSiExitProg (737);
#endif
		return(FALSE); 
}
	}
	if (!hQuadTree)
		ii=1;
	EditBounds = CurView->FileMNMX;
	CurView->FileProjectionType=0;
	PenColor = PenColorIn;
	nPoly = abs (nPolyIn); 
	lpnPnts = lpnPntsIn;
	MinMaxInit (&MinMax);
	TotP = 0; 
	if (nPolyIn)
	{
		if (nPolyIn < 0) 
		{
			for (i=0,pnPnts=lpnPnts;i<nPoly;i++,pnPnts++) 
				TotPoints += *pnPnts;
			TotPoints += nPoly - 1;
			PenColor = -1;
			nPoly = 1; 
			lpnPnts = &TotPoints;   
		}
		lphDPoints = phDPoints;
		hhPoints = GSSiGlobAlloc ( 347,GHND,(nPoly+1)*sizeof(HANDLE));
		phPoints = (LPHANDLE)GlobalLock (hhPoints);
		hnPoints = GSSiGlobAlloc ( 348,GHND,nPoly*sizeof(int));
		pnPoints = (LPINT)GlobalLock (hnPoints);
		for (i=0,pnPnts=lpnPnts;i<nPoly;i++,pnPnts++,phPoints++,phDPoints++,pnPoints++) 
		{
			*phPoints = GSSiGlobAlloc ( 349,GHND,(long)*pnPnts*sizeof(POINTS));
			lpPoints = (LPPOINTS) GlobalLock(*phPoints);
			lpPoint = lpPoints; 
			if (HiPrecis == 2) 
			{
				HPDPOINT3D	lpDPoint3D = (HPDPOINT3D)GlobalLock(*phDPoints); 
				long	np = *pnPnts; 
				
				nElevPts = np;
				hTemp = GSSiGlobAlloc ( 350,GMEM_MOVEABLE,np*sizeof(DPOINT));
				lpDPoint = (HPDPOINT)GlobalLock(hTemp);
				while (np--)
				{
					lpDPoint->x = lpDPoint3D->x;
					lpDPoint++->y = lpDPoint3D++->y;
				}
				GlobalUnlock (hTemp);
				lpDPoint = (HPDPOINT)GlobalLock(hTemp);
			}
			else
				lpDPoint = (HPDPOINT)GlobalLock(*phDPoints); //lpDPoint[18]
			if (!TotP)
				FirstDPoint = *lpDPoint;
			for (j=0;j<*pnPnts;j++,lpDPoint++)
			{   
				switch (TotP)
				{
					case 1:
						POC = *lpDPoint;
						break;
					case 2:
						PT = *lpDPoint;
						break;
					default:
						break;
				}
				if (!PointInEditBounds (lpDPoint))
				{
					GlobalUnlock (*phDPoints);  
					GlobalUnlock (hhPoints); 
					GSSiGlobUlFree (phPoints);
					phPoints = (LPHANDLE)GlobalLock (hhPoints);
					for (j=0;j<i;j++,phPoints++) 
						GSSiGlobFree (phPoints);
				    GSSiGlobUlFree (&hhPoints);
				    GSSiGlobUlFree (&hnPoints);
				    ForceRefIndex = FALSE; 
				    ForceTAGIndex = FALSE; 
				    GSSiGlobUlFree (&hTemp);
{
#if ENABLETRACE
GSSiExitProg (737);
#endif
					return FALSE;
}
				}
				if (!i && j==1)
				{
					TXPoint = MidPointD (FirstDPoint,*lpDPoint);
					rot = getazd (&FirstDPoint,lpDPoint);
				}
				*lpPoint = POINTtoPOINTS(BasePtToFilePt(*lpDPoint)); 
	//			if (lpDPoint->x <= 0 || lpDPoint->y <=0)
	//				ii=1;
				if (Type && (i || j) && !HiPrecis &&
					(lpPoint->x == LastPoint.x && lpPoint->y == LastPoint.y))
					nsame++;
				else 
				{
					TotP++; 
					(*pnPoints)++;
					LastPoint = *lpPoint;   
					AddPointToMinMax (POINTStoPOINT(*lpPoint),&MinMax);
					lpPoint++; 
				}
			} 
			GlobalUnlock (*phDPoints); 
			GlobalUnlock(*phPoints);
		    GSSiGlobUlFree (&hTemp);
	    }
	    GlobalUnlock (hhPoints);  
	    GlobalUnlock (hnPoints); 
	    pnPoints = (LPINT)GlobalLock (hnPoints); //new npoints array needed if dup points removed  
	    lpnPnts = pnPoints;
	    if (Type == 3)
	    {  
			MNMXCORD MinMaxD;     	
			
			if (TotP == 2)
			{
				double dist = ldistp (FirstDPoint,POC);
				double az = getazd (&FirstDPoint,&POC);
				HPDPOINT points = (HPDPOINT)GlobalLock(*lphDPoints);
				
				PT = dnewpt (FirstDPoint,az,-dist);
				TotP = 3;
				(*lpnPnts)++;
				points[0] = PT;
				points[1] = POC;
				points[2] = PT;
				GlobalUnlock (*lphDPoints);
				FirstDPoint = PT;
			}
	    	if (TotP == 3 && CurveMNMX (FirstDPoint,POC,PT,&MinMaxD))
	    	{   
	    		DPoint.x = MinMaxD.xmn;
	    		DPoint.y = MinMaxD.ymn;
				if (!PointInEditBounds (&DPoint))
				{ 
OutOfBoundsExit:
					if (hhPoints)
					{
						phPoints = (LPHANDLE)GlobalLock (hhPoints);
						while (*phPoints)
							GSSiGlobUlFree (phPoints++);
						GSSiGlobUlFree (&hhPoints);
					}
				    GSSiGlobUlFree (&hnPoints);
				    ForceRefIndex = FALSE; 
				    ForceTAGIndex = FALSE; 
{
#if ENABLETRACE
GSSiExitProg (737);
#endif
					return FALSE;
}
				}
				Point = BasePtToFilePt(DPoint);
				AddPointToMinMax (Point,&MinMax);
	    		DPoint.x = MinMaxD.xmn;
	    		DPoint.y = MinMaxD.ymx;
				if (!PointInEditBounds (&DPoint))
					goto OutOfBoundsExit;
				Point = BasePtToFilePt(DPoint);
				AddPointToMinMax (Point,&MinMax);
	    		DPoint.x = MinMaxD.xmx;
	    		DPoint.y = MinMaxD.ymx;
				if (!PointInEditBounds (&DPoint))
					goto OutOfBoundsExit;
				Point = BasePtToFilePt(DPoint);
				AddPointToMinMax (Point,&MinMax);
	    		DPoint.x = MinMaxD.xmx;
	    		DPoint.y = MinMaxD.ymn;
				if (!PointInEditBounds (&DPoint))
					goto OutOfBoundsExit;
				Point = BasePtToFilePt(DPoint);
				AddPointToMinMax (Point,&MinMax);
	    	}
			else
	    		TypeLevel = Type = 1;
	    }
	}
	ltag=0; 
	if (Prefix) 
	{   
		if (*Prefix && _fstricmp (Prefix,"REFNO"))
		{
			_fstrcpy (tag,Prefix);
			_fstrcat (tag,":");
			_fstrcat (tag,UDI);
			ltag = _fstrlen (tag);
			if (ltag%2) ltag++;
		}
	}
	item_len = 2+4+ltag + 2+2; 
	if (hTimeStamp)
		item_len += 10; 
	if (Stuff)
		item_len += *Stuff;  
	if (pRouteOffset)
		item_len += 14; 
	else
	{
		if (AreaColor != -1 || PenColor != -1)
			item_len += 2;
		if (AreaColor != -1)
			item_len += 12;
		if (PenColorIn != -1)
			item_len += 10; 
	}
	if (nCurvePoints)
		item_len += 2 + nCurvePoints * 2;
	if (hGRText)
	{   
		short	ltxt;
		
		pGRText = (LPGRTEXT)GlobalLock (hGRText); 
		if (pGRText->ltext > 0)
			ltxt = pGRText->ltext; 
		else
		{
			_fstrcpy (Text,pGRText->Text);
			ExpandText (Text);
			ltxt = max (2,_fstrlen (Text));
		} 
		ltxt += ltxt % 2;
		item_len += (2 + sizeof(GRTEXTHEADER) + ltxt);
		_fstrcpy (Text,pGRText->cColor);
		ExpandText (Text);
        TextColor = atol (Text);
		GlobalUnlock (hGRText);
		if (TextColor > 0)
			item_len += 6;  
	}
	if (hTextTPL)
		item_len += (2 + sizeof(UMTEXTTPL));
	if (nPoly)
	{ 
		if (nPoly > 1)
			if (!Type)
				item_len += (2 + 4 + 4*nPoly);//accounts for polypartlen rec
		for (i=0,pnPnts=lpnPnts;i<nPoly;i++,pnPnts++)
			item_len += 4 + (long)*pnPnts*PointSize; 
//		if (Type == 3 && *lpnPnts == 2)
//			item_len += PointSize;
		if (nPolyIn < 0) 
		{
			for (i=0,pnPnts=pnPoints;i<abs(nPolyIn);i++,pnPnts++)
				item_len += 4 + (long)*pnPnts*PointSize; 
		}		
		if (!Type)
			item_len+=2*nPoly; 
		if (Type == 1 && TotP == 2 && HiPrecis)
			item_len -= 2; // type is GF_LINE
	}
	else
		item_len += 2;  
	if (HiPrecis == 2)
		item_len += (2 + 2 + nElevPts*4);
	hBuf = GSSiGlobAlloc ( 351,GMEM_MOVEABLE,item_len+1024);
	pBuf = GlobalLock (hBuf);
	pBufBeg = pBuf;
	
	id = 12 + 256 * ipen;
	BufWrite(&pBuf,&lbuf,(HPSTR)&id,2);
	MinMaxLoc = pBuf;
	BufWrite(&pBuf,&lbuf,(HPSTR)&MinMax,8);
	item_len /= 2;
	if (item_len > 16000)
		itemlen=0;
	else
		itemlen = -item_len;
	BufWrite(&pBuf,&lbuf,(HPSTR)&itemlen,2); 
	id = 9 + 256 * ltag;
	BufWrite(&pBuf,&lbuf,(HPSTR)&id,2);
	BufWrite(&pBuf,&lbuf,(HPSTR)&NewRefno,4); 
	if (ltag)
		BufWrite(&pBuf,&lbuf,tag,ltag);
	id = 8;
	BufWrite(&pBuf,&lbuf,(HPSTR)&id,2);
	BufWrite(&pBuf,&lbuf,(HPSTR)&idesc,2);  
	if (hTimeStamp)
	{   
		char	str[256]; 
		long	StartTime, EndTime;
		LPTIMESTAMP	lpTimeStamp=(LPTIMESTAMP)GlobalLock (hTimeStamp); 
		
		id = 37;
		BufWrite(&pBuf,&lbuf,(HPSTR)&id,2);
		_fstrcpy (str,lpTimeStamp->StartTime);
		ExpandText (str);
		StartTime = atol (str); 
		if (StartTime == 1)     
		{
			time_t	systime;     
			time (&systime);
			StartTime = systime;
		}
		BufWrite(&pBuf,&lbuf,(HPSTR)&StartTime,4);
		_fstrcpy (str,lpTimeStamp->EndTime);
		ExpandText (str);
		EndTime = atol (str); 
		if (EndTime <= StartTime)
			EndTime = StartTime;
		BufWrite(&pBuf,&lbuf,(HPSTR)&EndTime,4);  
		GlobalUnlock (hTimeStamp);
		MinFileTime = min (MinFileTime,StartTime);
	    MaxFileTime = max (MaxFileTime,EndTime);		
	} 
	else
	{
		MinFileTime = min (MinFileTime,0);
		MaxFileTime = LONG_MAX;
	}
	if (Stuff)
	{   
		item_len = *Stuff++;
		BufWrite(&pBuf,&lbuf,(HPSTR)Stuff,item_len);
	}
	if (AreaColor != -1)
	{
		id = 22;
		BufWrite(&pBuf,&lbuf,(HPSTR)&id,2);
		id = 0;
		BufWrite(&pBuf,&lbuf,(HPSTR)&id,2);
		BufWrite(&pBuf,&lbuf,(HPSTR)&AreaColor,4);
		BufWrite(&pBuf,&lbuf,(HPSTR)&AreaColor,4);
	}
	if (pRouteOffset)
	{
		id = 151;
		BufWrite(&pBuf,&lbuf,(HPSTR)&id,2);
		BufWrite(&pBuf,&lbuf,(HPSTR)pRouteOffset,8);
		BufWrite(&pBuf,&lbuf,(HPSTR)&PenColor,4);
	}
    else if (PenColor != -1)
	{
		if (HiPrecis)
			id = 121;
		else
			id = 21;
		BufWrite(&pBuf,&lbuf,(HPSTR)&id,2);
		BufWrite(&pBuf,&lbuf,(HPSTR)&PenColor,4);
		BufWrite(&pBuf,&lbuf,(HPSTR)&PenWidth,2);
		id = 0;
		BufWrite(&pBuf,&lbuf,(HPSTR)&id,2);
	}
    if (nCurvePoints)
    {   
		id = 28;
		BufWrite(&pBuf,&lbuf,(HPSTR)&id,2);
		BufWrite(&pBuf,&lbuf,(HPSTR)&nCurvePoints,2); 
		BufWrite(&pBuf,&lbuf,(HPSTR)CurvePoints,2*nCurvePoints); 
    }
    
	if (nPoly > 1 && !Type)
	{   
		if (HiPrecis)
			id = 227;
		else
			id = 228;
		BufWrite(&pBuf,&lbuf,(HPSTR)&id,2);
		BufWrite(&pBuf,&lbuf,(HPSTR)&nPoly,4);
		BufWrite(&pBuf,&lbuf,(HPSTR)lpnPnts,4*nPoly);
	}
    
    if (HiPrecis)
    {   
    	HANDLE	hElev=0;
    	HPFLOAT	pElev;
    	
		phDPoints = lphDPoints;
		
		for (i=0,pnPnts=lpnPnts;i<nPoly;i++,pnPnts++,phDPoints++)
		{   
			if (HiPrecis == 2) 
			{
				HPDPOINT3D	lpDPoint3D = (HPDPOINT3D)GlobalLock(*phDPoints); 
				long	np = *pnPnts;
				hTemp = GSSiGlobAlloc ( 352,GMEM_MOVEABLE,np*sizeof(DPOINT));
				lpDPoint = (HPDPOINT)GlobalLock(hTemp);
				hElev= GSSiGlobAlloc ( 353,GMEM_MOVEABLE,np*sizeof(float));
				pElev = (HPFLOAT)GlobalLock(hElev);
				while (np--)
				{
					lpDPoint->x = lpDPoint3D->x;
					lpDPoint++->y = lpDPoint3D->y; 
					*pElev++ = lpDPoint3D++->z;
				} 
				GlobalUnlock (hElev);
				GlobalUnlock (hTemp);
				lpDPoint = (HPDPOINT)GlobalLock(hTemp);
			}
			else
				lpDPoint = (HPDPOINT)GlobalLock(*phDPoints);
			if (hElev)
			{
				pElev = (HPFLOAT)GlobalLock(hElev);
				id = 38; 
				BufWrite(&pBuf,&lbuf,(HPSTR)&id,2);
				BufWrite(&pBuf,&lbuf,(HPSTR)pnPnts,2);
				BufWrite(&pBuf,&lbuf,(HPSTR)pElev,(long)*pnPnts*4);   
				GSSiGlobUlFree (&hElev);
			}  
			switch (Type)
			{   
				case 1:  
					if (nPoly == 1 && *pnPnts == 2)
					{
						id = 41;
						BufWrite(&pBuf,&lbuf,(HPSTR)&id,2);
						BufWrite(&pBuf,&lbuf,(HPSTR)lpDPoint,(long)*pnPnts*16);   
					}
					else
					{
					   	USHORT	npt=*pnPnts;
					
						idbyte = 61; 
						compression = CompressCoordinates (&npt,lpDPoint,&hCompressedCoord);
						BufWrite(&pBuf,&lbuf,(HPSTR)&idbyte,1);
						BufWrite(&pBuf,&lbuf,(HPSTR)&compression,1);
						BufWrite(&pBuf,&lbuf,(HPSTR)&npt,2); 
						if (hCompressedCoord)   
						{   
							HPDPOINT	lpDPointCompressed = (HPDPOINT)GlobalLock (hCompressedCoord);
							
							BufWrite(&pBuf,&lbuf,(HPSTR)lpDPointCompressed,(long)npt*16);   
							GSSiGlobUlFree (&hCompressedCoord);
						}
						else
							BufWrite(&pBuf,&lbuf,(HPSTR)lpDPoint,(long)npt*16);   
					}
				break; 
				
				case 3:
					id = 172; 
					TypeLevel = Type = 1;
					BufWrite(&pBuf,&lbuf,(HPSTR)&id,2);
					BufWrite(&pBuf,&lbuf,(HPSTR)pnPnts,2);
					BufWrite(&pBuf,&lbuf,(HPSTR)lpDPoint,(long)*pnPnts*16);   
				break;
				 
				case 0:	
				{
				   	USHORT	npt=*pnPnts;
	
					idbyte = 51;  
					compression = CompressCoordinates (&npt,lpDPoint,&hCompressedCoord);
					BufWrite(&pBuf,&lbuf,(HPSTR)&idbyte,1);
					BufWrite(&pBuf,&lbuf,(HPSTR)&compression,1);
					BufWrite(&pBuf,&lbuf,(HPSTR)&ipen,2);
					BufWrite(&pBuf,&lbuf,(HPSTR)&npt,2);
					if (hCompressedCoord)   
					{   
						HPDPOINT	lpDPointCompressed = (HPDPOINT)GlobalLock (hCompressedCoord);
							
						BufWrite(&pBuf,&lbuf,(HPSTR)lpDPointCompressed,(long)npt*16);   
						GSSiGlobUlFree (&hCompressedCoord);
					}
					else
						BufWrite(&pBuf,&lbuf,(HPSTR)lpDPoint,(long)npt*16); 
				}  
				break;  
			} 
			GlobalUnlock (*phDPoints);  
			GSSiGlobUlFree (&hTemp);
		}  
    }
    else if (nPolyIn)
    {
		phPoints = (LPHANDLE)GlobalLock (hhPoints);
		for (i=0,pnPnts=lpnPnts;i<nPoly;i++,pnPnts++,phPoints++)
		{   
			lpPoints = (LPPOINTS)GlobalLock (*phPoints);
			switch (Type)
			{   
				case 1:
					id = 6;
					BufWrite(&pBuf,&lbuf,(HPSTR)&id,2);
					BufWrite(&pBuf,&lbuf,(HPSTR)pnPnts,2);
					BufWrite(&pBuf,&lbuf,(HPSTR)lpPoints,(long)*pnPnts*4);   
				break; 
				
				case 3:
					id = 171; 
					TypeLevel = Type = 1;
					BufWrite(&pBuf,&lbuf,(HPSTR)&id,2);
					BufWrite(&pBuf,&lbuf,(HPSTR)pnPnts,2);
					BufWrite(&pBuf,&lbuf,(HPSTR)lpPoints,(long)*pnPnts*4);   
				break;
				 
				case 0:		
					id = 5;
					BufWrite(&pBuf,&lbuf,(HPSTR)&id,2);
					BufWrite(&pBuf,&lbuf,(HPSTR)&ipen,2);
					BufWrite(&pBuf,&lbuf,(HPSTR)pnPnts,2);
					BufWrite(&pBuf,&lbuf,(HPSTR)lpPoints,(long)*pnPnts*4); 
				break;  
			}
			GlobalUnlock (*phPoints); 
		}  
		GlobalUnlock (hhPoints);
    }
    
	if (nPolyIn < 0) 
	{
		if (PenColorIn != -1)
		{   
			if (HiPrecis)
				id = 121;
			else
				id = 21;
			BufWrite(&pBuf,&lbuf,(HPSTR)&id,2);
			BufWrite(&pBuf,&lbuf,(HPSTR)&PenColorIn,4);
			BufWrite(&pBuf,&lbuf,(HPSTR)&PenWidth,2);
			id = 0;
			BufWrite(&pBuf,&lbuf,(HPSTR)&id,2);
		}
  
		phPoints = (LPHANDLE)GlobalLock (hhPoints);
		lpPoints = (LPPOINTS)GlobalLock (*phPoints);
		for (i=0,pnPnts=pnPoints;i<abs(nPolyIn);i++,pnPnts++)
		{   
			id = 6;
			BufWrite(&pBuf,&lbuf,(HPSTR)&id,2);
			BufWrite(&pBuf,&lbuf,(HPSTR)pnPnts,2);
			BufWrite(&pBuf,&lbuf,(HPSTR)lpPoints,(long)*pnPnts*4); 
			lpPoints += (*pnPnts);
			if (i)
				lpPoints++;  
		}  
		GlobalUnlock (*phPoints); 
		GlobalUnlock (hhPoints);
    } 
    if (!nPolyIn)
    {
		id = 24;
		BufWrite(&pBuf,&lbuf,(HPSTR)&id,2);
	}
	if (hTextTPL)
	{   
		DPOINT	pt1, pt2;
		
		id = 192;
		BufWrite(&pBuf,&lbuf,(HPSTR)&id,2);
		pTextTPL = (LPUMTEXTTPL)GlobalLock (hTextTPL);
		BufWrite(&pBuf,&lbuf,(HPSTR)pTextTPL,sizeof(UMTEXTTPL));
		GlobalUnlock (hTextTPL);    
	}
	if (TextColor > 0)
	{
		id = 25;
		BufWrite(&pBuf,&lbuf,(HPSTR)&id,2);
		BufWrite(&pBuf,&lbuf,(HPSTR)&TextColor,4);
	}
    
	if (hGRText)
	{   
		LPSTR	pText;
		double	dHeight, twidth, xmove, ymove;
		UINT	isize;  
		long	lbuftemp=0;
		DPOINT	pt1, pt2;
		short	NumLines, iline, NumUpLines, MaxLineLen; 
		
		id = 19;
		BufWrite(&pBuf,&lbuf,(HPSTR)&id,2);
		pGRText = (LPGRTEXT)GlobalLock (hGRText);  
		_fmemset (&GRTextHead,0,sizeof(GRTEXTHEADER));
		GRTextHead.lText = max (0,pGRText->ltext);
		if (pGRText->UltiMapStyle)
		{
			if (pGRText->hJust < -6)
				GRTextHead.hJust = 2;
			else if (pGRText->hJust > 6)
				GRTextHead.hJust = 0;
			else
				GRTextHead.hJust = 1; 
			GRTextHead.hJust2 = 15 - IDNINT (fabs((double)pGRText->hJust * 15)/99); 
		}   
		else
		{
			GRTextHead.hJust = 2 - pGRText->hJust;
			GRTextHead.hJust2 = 0;
		}			
		GRTextHead.vJust = pGRText->vJust;          
		GRTextHead.italic = pGRText->italic;
		GRTextHead.Weight = pGRText->weight;
		GRTextHead.UltiMapStyle = pGRText->UltiMapStyle;
		GRTextHead.FlipForEasyReading = pGRText->FlipForEasyReading;
		_fstrcpy (Text,pGRText->cHeight);
		ExpandText (Text);
		dHeight = atof (Text);
		GRTextHead.FontNum = pGRText->FontNum;              
		GRTextHead.HeightIsPixels = 0;              
		switch (*LastChr (Text))
		{
			case 'F':
			case 'f':
				dHeight = ConvertInDist (dHeight,1);
			break;
			case 'M':
			case 'm':
				dHeight = ConvertInDist (dHeight,2);
			break;
			case 'p': 
				dHeight = dHeight / BaseDistToWinDist;
			break;
			case 'P':
				GRTextHead.HeightIsPixels = 1;              
			break; 
		}
		if (fabs(dHeight) < 64)
		{
			GRTextHead.HeightPrecision = 3;
			isize = IDNINT (dHeight * 1000);
		}    
		else if (fabs(dHeight) < 640)
		{
			GRTextHead.HeightPrecision = 2;
			isize = IDNINT (dHeight * 100);
		}    
		else if (fabs(dHeight) < 6400)
		{
			GRTextHead.HeightPrecision = 1;
			isize = IDNINT (dHeight * 10);
		}  
		else  
			isize = IDNINT(dHeight);
		GRTextHead.Height = isize;
		_fstrcpy (Text,pGRText->Text);
		if (GRTextHead.lText <= 0)
		{
			ExpandText (Text);
			GRTextHead.lText = max (2,_fstrlen(Text)); 
		}
		GRTextHead.lText += GRTextHead.lText % 2;
		if (GRTextHead.lText == 92)
			ii=1;	
		BufWrite(&pBuf,&lbuf,(HPSTR)&GRTextHead,sizeof(GRTEXTHEADER)); 
		BufWrite(&pBuf,&lbuf,(HPSTR)Text,GRTextHead.lText);
		GlobalUnlock (hGRText); 
	}
	if (AreaColor != -1 || PenColorIn != -1)
	{
		id = 23;
		BufWrite(&pBuf,&lbuf,(HPSTR)&id,2);
	} 
	
	id=13;
	BufWrite(&pBuf,&lbuf,(HPSTR)&id,2);
	Offset = -1;
	BufWrite(&pBuf,&lbuf,(HPSTR)&Offset,4);
	id = 0;
	BufWrite(&pBuf,&lbuf,(HPSTR)&id,2);

	RecomputeMinMax (&MinMax,pBufBeg,lbuf,0,0); 
	BufWrite(&MinMaxLoc,&lbuftemp,(HPSTR)&MinMax,8);   
    
    GlobalUnlock (hBuf); 
    pBuf = GlobalLock (hBuf); 
    First = TRUE;  
    hRec = 0; 
    if (NewRefno == 201)
    	ii=1; 
    if (Type && hGRText)
    {
    	if (!SymbolIsVisible (idesc))
    		Type = 2;
    }
//BOOL SplitRec (HPSHORT *pBuf,LPLONG plBuf,LPSHORT *Rec,LPLONG plRec,
//			   LPHANDLE phBuf, LPHANDLE phRem, BOOL AddLink)
    while (SplitRec (&(HPSHORT)pBuf,&lbuf,&(LPSHORT)Rec,&RecLen,&hBuf,&hRec,TRUE))
    {
		loc = GetFileConnectOffset(TypeLevel,idesc,MinMax,RecLen,-1,Rec); 
		if (First)
		{
			First = FALSE;
			CurrentItem = loc - CurrentSeg -2;
			ItemSeg = CurrentSeg;
		}    
	    GSSiGlobUlFree (&hBuf);
	}
	if (hhPoints)
	{
		phPoints = (LPHANDLE)GlobalLock (hhPoints);
		while (*phPoints)
			GSSiGlobFree (phPoints++);
		GSSiGlobUlFree (&hhPoints);
	}
   	FileInIndex=0;
   	CurrentRefno=NewRefno;
   	CurrentItemMinMax = MinMax;
	ForceRefIndex = ForceTAGIndex = TRUE;
	if (!hRefIdx || BT_OPEN_FOR_WRITE (hRefIdx) <= 0)
	{
		CloseRefIndex (TRUE);
		Opened = TRUE;
		OpenRefIndex (FALSE);
	}
	else
		Opened = FALSE;
   	BuildRefIndex(TRUE,Deleted);
   	if (Prefix  && *Prefix && _fstricmp (Prefix,"REFNO"))	
		BuildTAGIndex (Prefix,UDI,0,NewRefno,Deleted);
//	CloseMap ();
	if (Opened)
		CloseRefIndex (TRUE);
    ForceRefIndex = FALSE; 
	ForceTAGIndex = FALSE; 
    GSSiGlobUlFree (&hnPoints);
{
#if ENABLETRACE
GSSiExitProg (737);
#endif
	return(TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL SplitRec (HPSHORT *pBuf,LPLONG plBuf,LPSHORT *Rec,LPLONG plRec,LPHANDLE phBuf, LPHANDLE phRem, BOOL AddLink)
#if ENABLETRACE
{GSSiEnterProg (738);
#endif
{   
	LPBYTE	Pcode;   
	short	SaveAreaPen, CurPcode, PointSize;   
	USHORT	nPnts, RemPnts, nPntInBlock;
	HPSHORT	ipnt;
	HPUSHORT pnPnts;        
	LPLONG	longpnt;
	HPSTR	fpnt; 
	long	CurLen, EndLen, len, lhead;   
	HPUSHORT pRem;
	BOOL	Split=FALSE;

	if (*plBuf <=0)
{
#if ENABLETRACE
GSSiExitProg (738);
#endif
		return FALSE; 
}
	if (*phRem)
	{
		*pBuf = (HPSHORT)GlobalLock (*phRem);
		*phBuf = *phRem;
	}
	*Rec = *pBuf;
	if (*plBuf <32000)
	{
NoSplit:
		*plRec = *plBuf;
		*plBuf = 0;
{
#if ENABLETRACE
GSSiExitProg (738);
#endif
		return TRUE;  
}
	}  
	
    ipnt = *pBuf; 
    fpnt = (HPSTR)ipnt;    
         
    while (*ipnt != 0)
    { 
    	CurLen = (LPSTR)ipnt - (LPSTR)fpnt;
    	Pcode = (LPBYTE)ipnt;  
    	CurPcode = *Pcode;
    	ipnt++;
        PointSize = 4;	
	    switch (*Pcode)
        {   
			case 93: // removed record
	        case 92: /* deleted record */
			case 12: /* item minmax */
			{	
				ipnt += 4;
				*ipnt = 0;
				ipnt++;
		    }
		    break;
	
            case 4: /* put line */
	 		{
	 			nPnts = 2;
				goto DoPolyline;
			}
            break;
            
            case 41: /* put line */
	 		{
	 			nPnts = 2;
            	PointSize = 16;
				goto DoPolyline;
			}
            break;
            
            case 51:
            	PointSize = 16;
            case 5: /* put area */

	 			SaveAreaPen = *ipnt++;
            	goto Next; 
            	
            case 61:
            case 135:
            	PointSize = 16;
            case 35:
            case 6: /* put polyline */
        
	 		{
	 Next:	    pnPnts = ipnt++; 
	 			nPnts = *pnPnts;
	 DoPolyline:  
	 			EndLen = CurLen + (long) nPnts * PointSize; 
	 			if (EndLen >= 32000)
	 			{
	 				Split = TRUE;
	 				if (CurLen > 0)
	 				{    
	 					*plBuf -= CurLen;
	 					*phRem = GSSiGlobAlloc ( 354,GMEM_MOVEABLE,*plBuf);
	 					pRem = (HPSHORT)GlobalLock (*phRem);
	 					len = *plBuf/2;
	 					*pBuf = (HPSHORT)Pcode;
	 					while (len--)
	 						*pRem++ = *(*pBuf)++;   
	 					GlobalUnlock (*phRem);
	 					ipnt = (HPSHORT)Pcode;
	 					*plRec = CurLen;
	 				}
	 				else
	 				{   
	 					ipnt=(HPSHORT)Pcode;
	 					if (*Pcode == 5 || *Pcode == 51)
	 						*ipnt++ = 36; 
	 					if (PointSize == 4)
	 						*ipnt++ = 35; 
	 					else
	 						*ipnt++ = 135; 
	 					nPntInBlock = (7500 * 4)/PointSize;
	 					RemPnts = *pnPnts - nPntInBlock;
	 					*ipnt++ = nPntInBlock;
	 					ipnt += ((PointSize/2)*nPntInBlock);  
	 					*pBuf = ipnt;
	 					*plRec = (LPSTR)ipnt - (LPSTR)fpnt;
	 					if (CurPcode == 5 || CurPcode == 51)
	 						lhead = 6;
	 					else
	 						lhead = 4;  
	 					*plBuf -= (*plRec-lhead);
	 					*phRem = GSSiGlobAlloc ( 355,GMEM_MOVEABLE,*plBuf);
	 					pRem = (HPSHORT)GlobalLock (*phRem); 
	 					*pRem++ = CurPcode;
	 					len = *plBuf/2;
	 					if (CurPcode == 5 || CurPcode == 51)
	 					{
	 						len -= 3;
	 						*pRem++ = SaveAreaPen;
	 					}
	 					else
	 						len -= 2;
	 					*pRem++ = RemPnts;
	 					while (len--)
	 						*pRem++ = *(*pBuf)++;   
	 					GlobalUnlock (*phRem); 
	 				}
	 				if (AddLink)
	 				{
	 					*ipnt++ = 13;
	 					longpnt = (LPLONG)ipnt;
	 					*longpnt = -1;  
	 					ipnt += 2;
	 					*ipnt = 0;  
	 					*plRec += 8; 
	 				}
{
#if ENABLETRACE
GSSiExitProg (738);
#endif
	 				return TRUE;
}
	 			}
	 		    ipnt = ipnt + nPnts * (PointSize/2); 
	 		    
	 		    
			}
			break;
            
            case 13:
            	goto NoSplit;

            default: 
            	SkipSubRec (Pcode,&ipnt,0);
            break;

		}
    }
	if (!Split)
		goto NoSplit;
	
	
{
#if ENABLETRACE
GSSiExitProg (738);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
} 

BOOL DeletePickedItem (int Item,short OldType,short NewType)
#if ENABLETRACE
{GSSiEnterProg (739);
#endif
{   
	HFILE	FidDel; 
	OFSTRUCTGM	OFStruct;
	BOOL	Opened = FALSE, rtn=FALSE;
	short	ii;
	long	Loc;
	struct	{BYTE	Type, Pen;} ID;   
	REFINDEXDATA	RefIdxData;

	if (PickList[Item].ViewID)
	{
		GetPickName (Item);   
		if (MapFileType(PickName, 0, 0) != MT_PLT)
			goto Exit;
		FidDel = GSSiOpenFile (PickName,(LPOFSTRUCTGM)&OFStruct,OF_READWRITE);  
		Opened = TRUE;
	} 
	else
		FidDel = FidMap;                
	
	if (FidDel != HFILE_ERROR)
	{   
		Loc = PickList[Item].Segment + PickList[Item].Offset + 2;
		if (GSSillseek (FidDel,Loc,0)==Loc)
		{
			if (BigRead (FidDel,(HPSTR)&ID,2)==2)
			{
				if (ID.Type == OldType)
				{
					GSSillseek (FidDel,Loc,0);
					ID.Type =NewType;
					ii=BigWrite (FidDel,(HPSTR)&ID,2,-1);
					rtn = TRUE;
					if (*PickList[Item].Prefix) 
					{
						BOOL OpenedTAGList, SaveFTI=ForceTAGIndex;
						
						_fstrcpy (PltName,PickName);
						ForceTAGIndex = TRUE;
						OpenedTAGList = OpenTAGIndex (FALSE,StoreTAGBounds);
						
						DeleteFromTAGList (PickList[Item].Prefix,PickList[Item].UDI,PickList[Item].Refno);
						if (OpenedTAGList)
							CloseTAGIndex ();
						ForceTAGIndex = SaveFTI;
						if (NewType == 93)
						{	
							BOOL SaveFRI = ForceRefIndex;
							
							ForceRefIndex = TRUE;	
					    	OpenRefIndex (FALSE);
							BT_DELETE (hRefIdx,(LPSTR)&PickList[Item].Refno,(LPSTR)&RefIdxData,FALSE);
					    	CloseRefIndex (TRUE); 
					    	ForceRefIndex = SaveFRI;
					    }
					}   
					
				}  
			}
		}
		if (Opened)
			GSSiClose (FidDel);
	}
Exit: 
{
#if ENABLETRACE
GSSiExitProg (739);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}

BOOL DeletePickedItem2 (int Item)
#if ENABLETRACE
{GSSiEnterProg (739);
#endif
{   
	short	OldType=12,NewType=92; 
	HFILE	FidDel; 
	OFSTRUCTGM	OFStruct;
	BOOL	Opened = FALSE, rtn=TRUE;
	short	ii;
	long	Loc;
	struct	{BYTE	Type, Pen;} ID;   

	if (PickList[0].IsDeleted)
	{
		OldType = 92;
		NewType = 12;
	}
	if (!CurView->UpdateFile || ((PickList[0].FileNum+1) == CurView->UpdateFile)) 
		rtn = DeletePickedItem (Item,OldType,NewType);
	else
	{   
		short	DeleteType;
   				
		switch (PickList[0].Type)   //Point","Line","Area","Text","Curve"
		{
			case 1:
			case 2:            
			case 4:
			case 5:
			default:
				DeleteType = 1;
				break;
			case 3:
				DeleteType = 0;
		}
		_fstrcpy (PltName,CurView->lpFiles[CurView->UpdateFile-1]);
		PltType = 2;
		AddPolyToMap (0,0, 0,DeleteType,PickList[0].Refno,0,-1,PickList[0].Desc,0,PickList[0].Prefix,PickList[0].UDI,-1,-1,0,0,0,0,0,FALSE,0);
		CloseMap(TRUE);
		CloseRefIndex(FALSE);    		
		ForceRefIndex = ForceTAGIndex = FALSE;
	}

	
{
#if ENABLETRACE
GSSiExitProg (739);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}
BOOL AddPointToMap (DPOINT DPoint,long NewRefno,HANDLE hTimeStamp,int idesc,double size, double rot,LPSHORT Stuff,HANDLE hGRText,HANDLE hTextTPL,
					LPSTR Prefix, LPSTR UDI,long AreaColor,long PenColorIn, short PenWidth,BOOL BatchMode,BOOL HiPrecis,LPHANDLE phBuf,LPLONG plBuf)
#if ENABLETRACE
{GSSiEnterProg (740);
#endif
{   
	long	lbuftemp=0;
	short	i, j, id, item_len, ii, nPoly;
	DPOINT	pt1, pt2;
	static	mnmxCor		MinMax;
	mnmxCor	PtMinMax; 
	long	Offset, CurRecLenLoc, loc, endsegloc, PenColor, StartTime, EndTime, lBufBeg;      
	static	long	lastloc,iii=-1;
	short		CurRecLen, ltag;  
	long	TextColor=0;
	LPGRTEXT		pGRText;
	LPUMTEXTTPL		pTextTPL;
	GRTEXTHEADER	GRTextHead;
	static	HANDLE	hBuf=0;
	HPSTR	pBuf, pBufBeg, MinMaxLoc; 
	static	long	lbuf=0, call=0;
	char	Text[1024];
	char	tag[80];
	short	TxtType=1;  
	POINT	Point;
	static	debugref=980758;
	static	short	BufType, BufDesc=0;  
#pragma pack(1)
	struct {
			short	id;
			float	size,
				  	rot;
			POINTS	point;} PointData;
	struct {
			short	id;
			double	size,
				  	rot;
			DPOINT	point;} PointDataD;
#pragma pack()	
	if (NewRefno == debugref)
		call++;
	call++; 
	if (hGRText && !SymbolIsVisible (idesc))
		TxtType = 2;

//	CurView->HaveBounds=FALSE;
	if (!idesc && (!hBuf || !lbuf))
{
#if ENABLETRACE
GSSiExitProg (740);
#endif
		return FALSE;
}   
	    ForceRefIndex = ForceTAGIndex = TRUE; 
		if (!OpenMap (CurView->hWnd,0))
		{
		    ForceRefIndex = ForceTAGIndex = FALSE; 
	{
	#if ENABLETRACE
	GSSiExitProg (740);
	#endif
			return(FALSE); 
	}
		} 
		EditBounds = CurView->FileMNMX; 
	if (!idesc)
	{
		if (hBuf && lbuf)
		{
			pBuf = GlobalLock (hBuf); 
			pBuf += lbuf;
			id=13;
			BufWrite(&pBuf,&lbuf,(HPSTR)&id,2);
			Offset = -1;
			BufWrite(&pBuf,&lbuf,(HPSTR)&Offset,4);
			id = 0;
			BufWrite(&pBuf,&lbuf,(HPSTR)&id,2);
			GlobalUnlock (hBuf);
			pBuf = GlobalLock (hBuf);
			loc = GetFileConnectOffset(BufType,BufDesc,MinMax,(short)lbuf,-1,pBuf);
			GSSiGlobUlFree (&hBuf); 
			lbuf = 0; 
			CloseMap (TRUE);
		}
    	ForceRefIndex = ForceTAGIndex = FALSE; 
{
#if ENABLETRACE
GSSiExitProg (740);
#endif
		return (FALSE);
}
	}
	
	CurView->FileProjectionType=0;
	PenColor = PenColorIn;
	if (!PointInEditBounds (&DPoint))   
	{
	    ForceRefIndex = ForceTAGIndex = FALSE; 
	
{
#if ENABLETRACE
GSSiExitProg (740);
#endif
		return FALSE;
}
	}
	Point = BasePtToFilePt(DPoint);
	    
    if (hBuf &&
    	(!PtInMinMax (Point,CurMinMax) ||
    	 TxtType != BufType || 
    	 (BufDesc && idesc != BufDesc) ||
    	 lbuf > 4096))
    {
		pBuf = GlobalLock (hBuf); 
		pBuf += lbuf;
		id=13;
		BufWrite(&pBuf,&lbuf,(HPSTR)&id,2);
		Offset = -1;
		BufWrite(&pBuf,&lbuf,(HPSTR)&Offset,4);
		id = 0;
		BufWrite(&pBuf,&lbuf,(HPSTR)&id,2);
		GlobalUnlock (hBuf);
		pBuf = GlobalLock (hBuf);
		loc = GetFileConnectOffset(BufType,BufDesc,MinMax,(short)lbuf,-1,pBuf);
		GSSiGlobUlFree (&hBuf); 
		lbuf = 0; 
	}
	    
    if (!hBuf)
    {
		hBuf = GSSiGlobAlloc ( 356,GMEM_MOVEABLE,8192);  
		lbuf = 0;   
		BufType = TxtType;
		if (hDescBlock)
			BufDesc = idesc;
		else
			BufDesc = 0;
		MinMaxInit (&MinMax);
    }
	pBuf = GlobalLock (hBuf);
	pBuf += lbuf;
	MinMaxInit (&PtMinMax);
	if (size > 0)
	{   
		double	halfsize=size/2;
		RECT	SymRect = GetSymRect (idesc);
		
		pt1 = dnewpt (DPoint,rot,halfsize*((double)SymRect.right)/100);
		pt2 = dnewpt (pt1,rot+HALFPI,-halfsize*((double)SymRect.top)/100);  
		AddPointToMinMax (BasePtToFilePt(pt2),&PtMinMax);             
		pt2 = dnewpt (pt1,rot-HALFPI,halfsize*((double)SymRect.bottom)/100);
		AddPointToMinMax (BasePtToFilePt(pt2),&PtMinMax);   
		pt1 = dnewpt (DPoint,rot,halfsize*((double)SymRect.left)/1000);
		pt2 = dnewpt (pt1,rot+HALFPI,-halfsize*((double)SymRect.top)/100);  
		AddPointToMinMax (BasePtToFilePt(pt2),&PtMinMax);   
		pt2 = dnewpt (pt1,rot-HALFPI,halfsize*((double)SymRect.bottom)/100);
		AddPointToMinMax (BasePtToFilePt(pt2),&PtMinMax);   
    }
    else
		AddPointToMinMax (Point,&PtMinMax); 
	PtMinMax.xmn--;
	PtMinMax.xmx++;
	PtMinMax.ymn--;
	PtMinMax.ymx++;  
	AddMinMax (&MinMax,&PtMinMax);
	ltag = 0;
	if (Prefix) 
	{   
		if (*Prefix)
		{
			_fstrcpy (tag,Prefix);
			_fstrcat (tag,":");
			_fstrcat (tag,UDI);
			ltag = _fstrlen (tag);
			if (ltag%2) ltag++;  
		}
	}
	item_len = 2+4+ltag + 2+2;  
	if (hTimeStamp)
		item_len += 10; 
	if (Stuff)
		item_len += *Stuff;  
	if (AreaColor != -1 || PenColor != -1)
		item_len += 2;
	if (AreaColor != -1)
		item_len += 12;
	if (PenColorIn != -1)
		item_len += 10;  
	if (hGRText)
	{   
		short	ltxt;
		
		pGRText = (LPGRTEXT)GlobalLock (hGRText);   
		if (pGRText->ltext > 0)
			ltxt = pGRText->ltext; 
		else
		{
			_fstrcpy (Text,pGRText->Text);
			ExpandText (Text);
			ltxt = max (2,_fstrlen (Text));
		} 
		ltxt += ltxt % 2;
		item_len += (2 + sizeof(GRTEXTHEADER) + ltxt);  
		_fstrcpy (Text,pGRText->cColor);
		ExpandText (Text);
        TextColor = atol (Text);
		GlobalUnlock (hGRText);
		if (TextColor > 0)
			item_len += 6;  
	}
	if (hTextTPL)
		item_len += (2 + sizeof(UMTEXTTPL)); 
	if (HiPrecis)
		item_len += sizeof(PointDataD); 
	else
		item_len += sizeof(PointData); 

	id = 12;
	pBufBeg = pBuf;
	lBufBeg = lbuf;
	BufWrite(&pBuf,&lbuf,(HPSTR)&id,2);
	
	MinMaxLoc = pBuf;
	BufWrite(&pBuf,&lbuf,(HPSTR)&PtMinMax,8);
	item_len /= 2;
	item_len = -item_len;
	BufWrite(&pBuf,&lbuf,(HPSTR)&item_len,2); 
	id = 9 + 256 * ltag;
	BufWrite(&pBuf,&lbuf,(HPSTR)&id,2);
	BufWrite(&pBuf,&lbuf,(HPSTR)&NewRefno,4); 
	if (ltag)
		BufWrite(&pBuf,&lbuf,tag,ltag);
	id = 8;
	BufWrite(&pBuf,&lbuf,(HPSTR)&id,2);
	BufWrite(&pBuf,&lbuf,(HPSTR)&idesc,2); 
	if (hTimeStamp)
	{   
		char	str[256];
		LPTIMESTAMP	lpTimeStamp=(LPTIMESTAMP)GlobalLock (hTimeStamp); 
		
		id = 37;
		BufWrite(&pBuf,&lbuf,(HPSTR)&id,2);
		_fstrcpy (str,lpTimeStamp->StartTime);
		ExpandText (str);
		StartTime = atol (str);
		if (StartTime == 1)     
		{
			time_t	systime;     
			time (&systime);
			StartTime = systime;
		}
		BufWrite(&pBuf,&lbuf,(HPSTR)&StartTime,4);
		_fstrcpy (str,lpTimeStamp->EndTime);
		ExpandText (str);
		EndTime = atol (str); 
		if (EndTime <= StartTime)
			EndTime = StartTime;
		BufWrite(&pBuf,&lbuf,(HPSTR)&EndTime,4);  
		GlobalUnlock (hTimeStamp);   
		MinFileTime = min (MinFileTime,StartTime);
	    MaxFileTime = max (MaxFileTime,EndTime);		
	} 
	else
	{
		MinFileTime = min (MinFileTime,0);
		MaxFileTime = LONG_MAX;
	}
	if (Stuff)
	{   
		if (*Stuff)
		{
			item_len = *Stuff++;
			BufWrite(&pBuf,&lbuf,(HPSTR)Stuff,item_len); 
		}
	}
	if (AreaColor != -1)
	{
		id = 22;
		BufWrite(&pBuf,&lbuf,(HPSTR)&id,2);
		id = 0;
		BufWrite(&pBuf,&lbuf,(HPSTR)&id,2);
		BufWrite(&pBuf,&lbuf,(HPSTR)&AreaColor,4);
		BufWrite(&pBuf,&lbuf,(HPSTR)&AreaColor,4);
	}
	if (PenColor != -1)
	{    
		if (HiPrecis)
			id = 121;
		else
			id = 21;
		BufWrite(&pBuf,&lbuf,(HPSTR)&id,2);
		BufWrite(&pBuf,&lbuf,(HPSTR)&PenColor,4);
		BufWrite(&pBuf,&lbuf,(HPSTR)&PenWidth,2);
		id = 0;
		BufWrite(&pBuf,&lbuf,(HPSTR)&id,2);
	}
    if (HiPrecis)
    {
		PointDataD.id = 120;
		PointDataD.size = size;
		PointDataD.rot = rot;
		PointDataD.point = DPoint;
		BufWrite(&pBuf,&lbuf,(HPSTR)&PointDataD,sizeof(PointDataD));
	}
	else    
    {
		PointData.id = 20;
		PointData.size = size;
		PointData.rot = rot;
		PointData.point = POINTtoPOINTS(Point);
		BufWrite(&pBuf,&lbuf,(HPSTR)&PointData,sizeof(PointData));
	}    
	if (hTextTPL)
	{   
		DPOINT	pt1, pt2;
		
		id = 192;
		BufWrite(&pBuf,&lbuf,(HPSTR)&id,2);
		pTextTPL = (LPUMTEXTTPL)GlobalLock (hTextTPL);
		BufWrite(&pBuf,&lbuf,(HPSTR)pTextTPL,sizeof(UMTEXTTPL));
		GlobalUnlock (hTextTPL);    
	}
	if (TextColor > 0)
	{
		id = 25;
		BufWrite(&pBuf,&lbuf,(HPSTR)&id,2);
		BufWrite(&pBuf,&lbuf,(HPSTR)&TextColor,4);
	}
    
	if (hGRText)
	{   
		LPSTR	pText;
		double	dHeight, twidth, xmove, ymove;
		UINT	isize;  
		short	NumLines, iline, NumUpLines, MaxLineLen; 
		
		id = 19;
		BufWrite(&pBuf,&lbuf,(HPSTR)&id,2);
		pGRText = (LPGRTEXT)GlobalLock (hGRText);  
		_fmemset (&GRTextHead,0,sizeof(GRTEXTHEADER)); 
		GRTextHead.lText = max (0,pGRText->ltext);
		if (pGRText->UltiMapStyle)
		{
			if (pGRText->hJust < -6)
				GRTextHead.hJust = 2;
			else if (pGRText->hJust > 6)
				GRTextHead.hJust = 0;
			else
				GRTextHead.hJust = 1; 
			GRTextHead.hJust2 = 15 - IDNINT (fabs((double)pGRText->hJust * 15)/99); 
		}   
		else
		{
			GRTextHead.hJust = 2 - pGRText->hJust;
			GRTextHead.hJust2 = 0;
		}			
		GRTextHead.vJust = pGRText->vJust;   
		GRTextHead.italic = pGRText->italic;
		GRTextHead.Weight = pGRText->weight;
		GRTextHead.Opaque = pGRText->Opaque;
		GRTextHead.shadow = pGRText->Shadow;
		GRTextHead.UltiMapStyle = pGRText->UltiMapStyle;
		GRTextHead.FlipForEasyReading = pGRText->FlipForEasyReading;
		_fstrcpy (Text,pGRText->cHeight);
		ExpandText (Text);
		dHeight = atof (Text);
		GRTextHead.FontNum = pGRText->FontNum;              
		GRTextHead.HeightIsPixels = 0;              
		switch (*LastChr (Text))
		{
			case 'F':
			case 'f':
				dHeight = ConvertInDist (dHeight,1);
			break;
			case 'M':
			case 'm':
				dHeight = ConvertInDist (dHeight,2);
			break;
			case 'p': 
				dHeight = dHeight / BaseDistToWinDist;
			break;
			case 'P':
				GRTextHead.HeightIsPixels = 1;              
			break; 
		}
		if (fabs(dHeight) < 64)
		{
			GRTextHead.HeightPrecision = 3;
			isize = IDNINT (dHeight * 1000);
		}    
		else if (fabs(dHeight) < 640)
		{
			GRTextHead.HeightPrecision = 2;
			isize = IDNINT (dHeight * 100);
		}    
		else if (fabs(dHeight) < 6400)
		{
			GRTextHead.HeightPrecision = 1;
			isize = IDNINT (dHeight * 10);
		}  
		else  
			isize = IDNINT(dHeight);
		GRTextHead.Height = isize;

		_fstrcpy (Text,pGRText->Text);
		ExpandText (Text);
		if (GRTextHead.lText <= 0)
			GRTextHead.lText = max (2,_fstrlen(Text)); 
		GRTextHead.lText += GRTextHead.lText % 2;
		BufWrite(&pBuf,&lbuf,(HPSTR)&GRTextHead,sizeof(GRTEXTHEADER)); 
		BufWrite(&pBuf,&lbuf,Text,GRTextHead.lText);
		GlobalUnlock (hGRText); 
        
	}
	if (AreaColor != -1 || PenColorIn != -1)
	{
		id = 23;
		BufWrite(&pBuf,&lbuf,(HPSTR)&id,2);
	} 
	
	RecomputeMinMax (&PtMinMax,pBufBeg,lbuf-lBufBeg,0,0); 
	BufWrite(&MinMaxLoc,&lbuftemp,(HPSTR)&PtMinMax,8);   
	AddMinMax (&MinMax,&PtMinMax);
	
	if (!BatchMode || !PtInMinMax (Point,CurMinMax))
	{
		id=13;
		BufWrite(&pBuf,&lbuf,(HPSTR)&id,2);
		Offset = -1;
		BufWrite(&pBuf,&lbuf,(HPSTR)&Offset,4);
		id = 0;
		BufWrite(&pBuf,&lbuf,(HPSTR)&id,2);
		
		GlobalUnlock (hBuf);
		pBuf = GlobalLock (hBuf); 
/*		if (!BatchMode && phBuf && hTextTPL)
		{
			pTextTPL = GlobalLock (hTextTPL); 
			if (pTextTPL->style1 < 0)
			{
				GlobalUnlock (hTextTPL);    
				*phBuf = hBuf;
				hBuf = 0;
				*plBuf = lbuf;
				lbuf = 0;
{
#if ENABLETRACE
GSSiExitProg (740);
#endif
				return TRUE; 
}
			}
			GlobalUnlock (hTextTPL);    
		}*/ 
/*		if (!BatchMode) 
		{   
			MSG	msg; 
			HANDLE	hSaveName=GSSiGlobAlloc ( 357,GMEM_MOVEABLE,256);
			LPSTR	pSaveName = GlobalLock (hSaveName);
			BOOL SaveFRI=ForceRefIndex, SaveFTI=ForceTAGIndex;

		    ForceRefIndex = ForceTAGIndex = FALSE; 
		    ProcessGraphicsRec (CurView->hDC,(LPSHORT) pBuf,(LPSTR) pBuf); 
			_fstrcpy (pSaveName,PltName);
			GlobalUnlock (hSaveName);
	    	CloseMap (FALSE);
			msg.wParam = 0; 
			while (msg.message != WM_KEYDOWN)
			{
				GetMessage (&msg,hWndMain,0,0);
				if (msg.message == GF_REDRAW)
				{
					pSaveName = GlobalLock (hSaveName); 
					_fstrcpy (PltName,pSaveName);
					GlobalUnlock (hSaveName);
					OpenMap (CurView->hWnd,0);
				    ProcessGraphicsRec (CurView->hDC,(LPSHORT) pBuf,(LPSTR) pBuf); 
				   	CloseMap (FALSE);
				}
				else
				{
				    TranslateMessage(&msg);
				    DispatchMessage(&msg);
				} 
			} 
			pSaveName = GlobalLock (hSaveName); 
			_fstrcpy (PltName,pSaveName);
			GSSiGlobUlFree (&hSaveName);
		    ForceRefIndex = SaveFRI;
		    ForceTAGIndex = SaveFTI;
			OpenMap (CurView->hWnd,0);
	    }*/
		loc = GetFileConnectOffset(BufType,BufDesc,MinMax,(short)lbuf,-1,pBuf);
		GSSiGlobUlFree (&hBuf);
		lbuf = 0;
	}  
	else
		GlobalUnlock (hBuf);
    
	CurrentItem = loc - CurrentSeg -2;    
	ItemSeg = CurrentSeg;
	    
   	FileInIndex=0;
   	CurrentRefno=NewRefno;
	BuildTAGIndex (Prefix,UDI,0,NewRefno,FALSE);
   	BuildRefIndex(TRUE,FALSE);
//	CloseMap ();
    ForceRefIndex = ForceTAGIndex = FALSE; 
{
#if ENABLETRACE
GSSiExitProg (740);
#endif
	return(TRUE);
}
#if ENABLETRACE
}
#endif
}

void RedrawActiveFunctions (short Cmd)
#if ENABLETRACE
{GSSiEnterProg (741);
#endif
{   int i;
    HANDLE	hNext, hCmdStr;
	LPCMDSTRING    pCmdStr;
	
	if (!CurView)
{
#if ENABLETRACE
GSSiExitProg (741);
#endif
		return;
}
	if (!CurViewActive())
{
#if ENABLETRACE
GSSiExitProg (741);
#endif
		return;
}
	hCmdStr = CurView->FunStackHandle;
    while (hCmdStr)
    { 
    	pCmdStr = (LPCMDSTRING)GlobalLock (hCmdStr);   
    	if (!pCmdStr)
    	{
    		if (hCmdStr == CurView->FunStackHandle)
    			CurView->FunStackHandle = 0;	
{
#if ENABLETRACE
GSSiExitProg (741);
#endif
    		return;                         
}
    	}
    	hNext = pCmdStr->PrevHandle;
    	ProcessGraphicsFunction2 (pCmdStr->CurFun,CurView->hWnd,Cmd,0,0);
    	GlobalUnlock (hCmdStr);
    	hCmdStr = hNext;
    }
{
#if ENABLETRACE
GSSiExitProg (741);
#endif
	return;
}
#if ENABLETRACE
}
#endif
} 

 

 


BOOL DisplayMarker (DPOINT Dpoint,int Ininc, LPSTR txt,double Insize,double AZ,COLORREF color,BOOL PreventColission,
					BOOL Border, LPHANDLE phSaveScreen,LPLONG pScreenID,LPRECT pRect,LPRECT pTextRect,LPRECT pFlagRect)
#if ENABLETRACE
{GSSiEnterProg (743);
#endif
{
	RECT	Rect;
	POINT	point;
	short	inc,Symbol=0;
	short	vjust=2; 
	double	dist,size=fabs(Insize);  
	HBRUSH	hBrush, hOldBrush=0;
	BOOL	SaveHVFC=HaveVarFillColor;
	COLORREF	SaveGC=GlobalColors[0];   
	HRGN	hRgn;  
	BOOL	Shadow=2;
	int		ii;

	if (GetTextColor (CurView->hDC) == RGB(255,255,255))
		Shadow = FALSE;
	
	inc = abs(Ininc)%1000;   
	if (Insize < 0)
		vjust = 1;
	
	if (!DisplayMarkers || !CurView)
{
#if ENABLETRACE
GSSiExitProg (743);
#endif
		return 0;                        
}
	
    SaveDC (CurView->hDC);
	if (Shadow)
	   	SetBkMode (CurView->hDC,TRANSPARENT); 

    SetDisplayMode (CurView->hDC, GF_TEXTMODE);
	hRgn = CreateVPRgn (FALSE,FALSE);
  	SelectClipRgn (CurView->hDC,hRgn);
  	GSSiDeleteObject(&hRgn);
	hBrush = CreateSolidBrush (color);
	hOldBrush = SelectObject (CurView->hDC,hBrush);
	if (color != (COLORREF)-1)
	{
		HaveVarFillColor = TRUE; 
		GlobalColors[0] = color;
	}
	point = BasePtToWinPt (&Dpoint); 
	if (!inc)
	{   
		if (txt)
		{
			char	sizestr[256];
			DWORD	TextExt;

			if (!GetGlobalCVal ("[%VEHICLESIZESTRING]",sizestr,0))
				strcpy(sizestr,txt);
			TextExt = 
						DispText (CurView->hDC,TRUE,point.x-inc*2,point.x+inc*2, -100, 0,2,2,
				  		size,1,1,100, FALSE,AZ,sizestr,0,PreventColission,Shadow,RGB(255,255,255),-1,0,0,0,0,0,0,0,0,0,0,pRect,0); 
			inc = max ((UINT)HIWORD(TextExt),(UINT)LOWORD(TextExt))+4; 
		}
		else
			inc = 2;   
	}
	inc += inc%2;
	Rect.left = point.x-inc/2;
	Rect.right = point.x+inc/2;
	Rect.top = point.y-inc/2;
	Rect.bottom = point.y+inc/2; 
	if (phSaveScreen)
	{
		RECT SaveRect=Rect;
		
    	InflateRect (&SaveRect,2,2);
		*phSaveScreen = SaveScreen2 (CurView->hWnd,CurView->hDC,SaveRect,CurView,pScreenID);
	}
	else if (pSymbolRect)
		UnionRect (pSymbolRect,&Rect,pSymbolRect);
	if (Ininc > 0)
	{
		FillRectPoly(CurView->hDC, &Rect, ConvertColor(color,-1));
		if (Border>0)
			DrawRectPoly(CurView->hDC, &Rect, GetStockObject(BLACK_PEN));
	}
	if (Border < 0)
		Symbol = -Border;	
	if (txt) 
	{   
//		dist = (CurView->BaseUnitsPerPixel*size*GetDeviceCaps(CurView->hDC, LOGPIXELSY)*_fstrlen(txt))/2; 
//		Dpoint = dnewpt (Dpoint,AZ,dist); 
//		point = BasePtToWinPt (Dpoint);
		DispText (CurView->hDC,FALSE,point.x-inc*2,point.x+inc*2, point.y, -(inc+4),2,vjust,
			  		size,1,1,100, FALSE,AZ,txt,Symbol+10000,PreventColission,Shadow,RGB(255,255,255),-1,0,0,0,0,0,0,0,0,0,pTextRect,pRect,pFlagRect); 
	} 
	if (hOldBrush)
		SelectObject (CurView->hDC,hOldBrush); 
	DeleteObject (hBrush);
	RestoreDC (CurView->hDC,-1);
	HaveVarFillColor = SaveHVFC;
	GlobalColors[0] = SaveGC;
{
#if ENABLETRACE
GSSiExitProg (743);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
} 





BOOL SnapPickedItem (int Item)
#if ENABLETRACE
{GSSiEnterProg (1176);
#endif
{   //opt=1	convert low precis to hi precis
	//opt=2 make square
	short	pos=BT_FIRST;
	HIGHLIGHTDATA	HighlightData;
    LPTHEME pTheme;                                        
	long	iref,ii;       
	USHORT	nPnts2;
	static	long debugref=-2145529886;
	short	nParts, nareas; 
	HANDLE	hIndex;
	LPLONG	pIndex;
	LPINT	pPolyParts;
	char	CRef[64]; 
	long	NumItems, Done=0;  
	HPDPOINT	lpUpdatePolyPoints;
	
	if (PickList[Item].Type != 2 && PickList[Item].Type != 3 && PickList[Item].Type != 5)
		return FALSE;
 	SetConfig (PickList[Item].ConfigID);
    SetViewport (PickList[Item].ViewID); 
    if (CurView->UpdateFile == PickList[Item].FileNum+1)
		return SnapPickedItemInPlace (Item);
	if (PickList[0].Type == 5)
	{
       	hUpdateMultiPolygon = 0;
       	nUpdateMultiPolygon = 0; 
       	nUpdatePolyPoints = 3;
    	hUpdatePoly = GSSiGlobAlloc (1261,GMEM_MOVEABLE,((long)nUpdatePolyPoints+nUpdateMultiPolygon) * sizeof(DPOINT));
    	lpUpdatePolyPoints = (HPDPOINT)GlobalLock (hUpdatePoly); 
    	lpUpdatePolyPoints[0] = PickList[0].BeginPoint;
    	lpUpdatePolyPoints[1] = PickList[0].NodePoint;
    	lpUpdatePolyPoints[2] = PickList[0].EndPoint;
	    if (SnapEnd == 2 || SnapEnd == 4)
	    	lpUpdatePolyPoints[2] = SnapPoint; 
	    else
	    	lpUpdatePolyPoints[0] = SnapPoint;
    	GlobalUnlock (hUpdatePoly);   
   		UpdateItem = 172;
	}
	else
	{
		pTheme = AddTheme (GF_SAVEPOLYPARTS_THEME);
		CurView->PassID = 4; 
		ProcessSelectedTheme = CurView->NumThemes;
		ProcessPickedItem (0,FALSE); 
		ProcessSelectedTheme = 0;
		WantElement = LONG_MAX;               
		DeleteTheme (pTheme);
		nParts = GetSavedPolys (); 
	    if (hSavePoly)
	    {   LPMNMXCORD lpRect;
	        HPDPOINT    lpDpoint, lpUpdatePolyPoints;
	        HANDLE	hOutPoint=0, hOrigPoint=0, hLinks=0;
			                            
			                            
	        nPnts = nSavePoly; 
	        lpRect = (LPMNMXCORD) GlobalLock (hSavePoly);
	        lpRect++;
	        lpDpoint = (LPDPOINT) lpRect; 
	    	nUpdatePolyPoints = nPnts; 
	        if (hSavePolyParts)
	        {   
	        	LPUSHORT	pMultiPoly;
	            	
	        	pPolyParts = (LPINT)GlobalLock (hSavePolyParts);
	        	nUpdateMultiPolygon = *pPolyParts++; 
	        	hUpdateMultiPolygon = GSSiGlobAlloc (1260,GMEM_MOVEABLE,((long)nUpdateMultiPolygon) * sizeof(USHORT)); 
	        	pMultiPoly = (HPUSHORT)GlobalLock (hUpdateMultiPolygon); 
	        	hmemmove ((HPSTR)pMultiPoly,(HPSTR)pPolyParts,(long)nUpdateMultiPolygon*sizeof(USHORT)); 
	        	GlobalUnlock (hUpdateMultiPolygon);
	        	GlobalUnlock (hSavePolyParts);
	        } 
	        else 
	        {
	        	hUpdateMultiPolygon = 0;
	        	nUpdateMultiPolygon = 0; 
	        }
	    	hUpdatePoly = GSSiGlobAlloc (1261,GMEM_MOVEABLE,((long)nUpdatePolyPoints+nUpdateMultiPolygon) * sizeof(DPOINT));
	    	lpUpdatePolyPoints = (HPDPOINT)GlobalLock (hUpdatePoly); 
	    	hmemmove ((HPSTR)lpUpdatePolyPoints,(HPSTR)lpDpoint,nUpdatePolyPoints*sizeof(DPOINT));
	    	GlobalUnlock (hUpdatePoly);  
	    	GlobalUnlock (hSavePoly);
			HiPrecis = TRUE;
			lpDCurPoints = (HPDPOINT)GlobalLock (hUpdatePoly);
			nPnts = nUpdatePolyPoints; 
			if (PickList[Item].Type == 3) 
			{
				while (nPnts>1 && SameDPoint (&lpDCurPoints[0], &lpDCurPoints[nPnts-1]))
					nPnts--; 
			} 
			nPnts2 = nPnts;
			pnPnts = &nPnts2;
			SnapPolyLine (); 
			nPnts = nPnts2;
	       	if (PickList[Item].Type == 3) 
	       	{   
	       		lpDCurPoints[nPnts] = lpDCurPoints[0];
	       		nPnts++; 
	       		UpdateItem = 5;
	       	}
	       	else
				UpdateItem = 6; 
	   		nUpdatePolyPoints = nPnts;
			if (PickList[Item].HiPrecis)
				UpdateItem = UpdateItem * 10 + 1;
	    	GlobalUnlock (hUpdatePoly); 
	    }
	    else
	    	return FALSE;
		DestroySavedPolys ();
	}
	UpdateRecord (Item,PickList[Item].Desc,PickList[Item].Prefix,PickList[Item].UDI,0,0,1,Item);
	GSSiGlobFree (&hUpdatePoly); 
    ShowPickedItem (hWndMain,Item); 
{
#if ENABLETRACE
GSSiExitProg (1176);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
} 

BOOL SnapPickedItemInPlace (int Item)
#if ENABLETRACE
{GSSiEnterProg (746);
#endif
{	LPSHORT		ipnt, EndItem;  
	short		SaveInt;
    HANDLE 		hpltBuf=0;
	LPSTR		LPpltBuf;
	LPITEM		ItemHeader;
	HDC			hDC;
	OFSTRUCTGM	OFStruct;
	POINT		CenterPoint, WinPoint;
	BOOL		SaveDisplay=Display, Rtn=FALSE;   
	HANDLE		hVisList=0;
	LPVISLIST	SaveVis=CurVis;
	LPTHEME		pTheme;
	
    GetPickName (Item);

	if (!PickName[0])
{
#if ENABLETRACE
GSSiExitProg (746);
#endif
		return(FALSE);
}
	_fstrcpy (PltName,PickName);

	CloseMap (FALSE);
	if (!OpenMap (CurView->hWnd,0))
{
#if ENABLETRACE
GSSiExitProg (746);
#endif
		return(FALSE);
}
	
	hVisList=GSSiGlobAlloc ( 358,GHND,sizeof(VISLIST));
	CurVis = (LPVISLIST)GlobalLock (hVisList); 
	CurVis->hVisList=hVisList;
	InitVis ();
	
	SetConfig (PickList[Item].ConfigID);
	SetViewport (PickList[Item].ViewID);
	SnapType = SysTypeFromPickType (PickList[Item].Type);
    GSSillseek (FidMap,PickList[Item].Segment,0);
    nRead = BigRead (FidMap,(HPSTR)&nBytes,2);
    hpltBuf = GSSiGlobAlloc ( 359,GMEM_MOVEABLE,(DWORD)nBytes);
    LPpltBuf = GlobalLock (hpltBuf);
    nRead = BigRead (FidMap,LPpltBuf,nBytes);
    if (nRead != nBytes || PickList[Item].Offset > nRead) 
    {
    	InvalidItem (0,TRUE);
    	goto Exit;
    }
    ipnt = (LPSHORT)(LPpltBuf + PickList[Item].Offset);
    ItemHeader = (LPITEM) ipnt;
    if (InvalidItem (ItemHeader,TRUE))
    	goto Exit;   
    if (!ItemHeader->Len)
    	goto Exit; 
    NewBounds = ItemHeader->MinMax;
    EndItem = ipnt + abs(ItemHeader->Len);  
    EndItem+=6;
    SaveInt = *EndItem;
    *EndItem = 0;
    Display = FALSE; 
    SnapPCT = PickList[Item].PCT;
	pTheme = AddTheme (GF_SNAP_POLY_THEME);  
	IgnoreBounds = TRUE;
	CurView->PassID = 4;
	ProcessSelectedTheme = CurView->NumThemes;
    ProcessGraphicsRec (0,ipnt,LPpltBuf,nRead); 
	ProcessSelectedTheme = 0;
    IgnoreBounds = FALSE;
	DeleteTheme (pTheme); 
	ItemHeader->MinMax = NewBounds;
	*EndItem = SaveInt;  
    GSSillseek (FidMap,PickList[Item].Segment+2,0);
    BigWrite (FidMap,(HPSTR)LPpltBuf,nBytes,-1);  
    Rtn = TRUE;
Exit:
	CloseMap(FALSE);
	GSSiGlobUlFree (&hpltBuf);
	Display = SaveDisplay;  
	GSSiGlobUlFree (&hVisList);
	CurVis = SaveVis;
{
#if ENABLETRACE
GSSiExitProg (746);
#endif
	return Rtn;
}
#if ENABLETRACE
}
#endif
}

BOOL SnapPolyLine ()
#if ENABLETRACE
{GSSiEnterProg (747);
#endif
{
	HPPOINTS	lpPoints;
	POINT	NewPoint;
    
    if (HiPrecis)
{
#if ENABLETRACE
GSSiExitProg (747);
#endif
    	return (SnapPolyLineD());
}
    if (SnapEnd>2)
    	NewPoint = SnapPointFile;
    else
	    NewPoint = BasePtToFilePt (SnapPoint);
	lpPoints = lpCurPoints; 
    if (SnapEnd == 2 || SnapEnd == 4)
    	lpPoints+=(nPnts-1);
    *lpPoints = POINTtoPOINTS(NewPoint);   
	NewBounds.xmn = min (NewBounds.xmn,lpPoints->x);
	NewBounds.ymn = min (NewBounds.ymn,lpPoints->y);
	NewBounds.xmx = max (NewBounds.xmx,lpPoints->x);
	NewBounds.ymx = max (NewBounds.ymx,lpPoints->y);
    
{
#if ENABLETRACE
GSSiExitProg (747);
#endif
	return(TRUE);
}
#if ENABLETRACE
}
#endif
} 

BOOL SnapPolyLineD (void)
#if ENABLETRACE
{GSSiEnterProg (748);
#endif
{
	HPDPOINT	lpPoints;
	DPOINT	NewPoint;
	POINT	NewPointFile;
	long	i, nRemove, np;
    
	NewPoint = SnapPoint;
    SnapPointFile = BasePtToFilePt (SnapPoint);
	lpPoints = lpDCurPoints; 
	if (pnPnts && nPnts > 2)
	{   
		double	d=0;
		double	ln = GetPolyLengthD (lpPoints,nPnts);
		double	SnapDist = ln * SnapPCT;
		HPSHORT	pNullCode;
		
		switch (SnapEnd)
		{
			case 1:
			case 3: 
			{
				for (i=1;i<nPnts;i++)
				{ 
					d += ldistp (lpPoints[i-1],lpPoints[i]);
					if (d > SnapDist)
					{
						nRemove = i - 1;
						if (nRemove)
						{
							*pnPnts -= nRemove;
							nPnts = *pnPnts;
							hmemmove ((HPSTR)&lpPoints[0],(HPSTR)&lpPoints[nRemove],(long)*pnPnts * sizeof(DPOINT));
							pNullCode = (HPSHORT)&lpPoints[*pnPnts];
							for (i=0;i<(nRemove*sizeof(DPOINT)/2);i++,pNullCode++)
								*pNullCode = 36; 
						}
						break;
					}
				}
			}
			break;
			case 2:
			case 4: 
			{   
				SnapDist = ln - SnapDist;
				for (i=nPnts-1;i;i--)
				{ 
					d += ldistp (lpPoints[i-1],lpPoints[i]);
					if (d > SnapDist)
					{
						nRemove = (nPnts-1) - i;
						if (nRemove)
						{
							*pnPnts -= nRemove;
							nPnts = *pnPnts;
							pNullCode = (HPSHORT)&lpPoints[*pnPnts];
							for (i=0;i<(nRemove*sizeof(DPOINT)/2);i++,pNullCode++)
								*pNullCode = 36; 
						}
						break;
					}
				}
			}
			break;
		}
	} 
	if (SnapType == 5 || SnapType == GF_CURVE)
	{
		np = 3;
		lpPoints = pCurveBP;
	}
	else
		np = nPnts;
    if (SnapEnd == 2 || SnapEnd == 4)
    	lpPoints+=(np-1);
    *lpPoints = NewPoint;
    NewPointFile = BasePtToFilePt (SnapPoint);
	NewBounds.xmn = min (NewBounds.xmn,NewPointFile.x);
	NewBounds.ymn = min (NewBounds.ymn,NewPointFile.y);
	NewBounds.xmx = max (NewBounds.xmx,NewPointFile.x);
	NewBounds.ymx = max (NewBounds.ymx,NewPointFile.y);
    
{
#if ENABLETRACE
GSSiExitProg (748);
#endif
	return(TRUE);
}
#if ENABLETRACE
}
#endif
} 


  
void RemoveVPRedef (void)
{
	int	i;

	CloseNewObjects ();
    CurView->NumNewObjects = 0;   
	if (CurView->hPenRedef)
		GSSiGlobUlFree (&CurView->hPenRedef);
    CurView->hPenRedef=0;
	_fmemset (CurView->NewObjectMap,0,sizeof(CurView->NewObjectMap));
	memset(CurView->HaveLayerColor, 0, sizeof(CurView->HaveLayerColor));
	ConfigChangesMade = TRUE;
	return;
}







void DisplayFunctionStack(void)
#if ENABLETRACE
{GSSiEnterProg (753);
#endif
{   
 HDC hDC;
 char key;     
 int	st;
    
    if (CurView && !CurView->DisplayFunStack)
    {
    	if (CurView->lpfnFUNSTACKMsgProc)
         	PostMessage(CurView->FunStackWnd, WM_CLOSE, 0, 0L);
{
#if ENABLETRACE
GSSiExitProg (753);
#endif
        return;
}
    }
    if (CurView && !CurView->lpfnFUNSTACKMsgProc)
    {
		CurView->lpfnFUNSTACKMsgProc = MakeProcInstance((DLGPROC)FUNSTACKMsgProc, hInst);
		CreateDialog(hInst, (LPSTR)"FUNSTACK", CurView->hWnd,(DLGPROC) CurView->lpfnFUNSTACKMsgProc);
	}
	else if (CurView)
         PostMessage(CurView->FunStackWnd, WM_COMMAND, IDOK, CurView->ID);
	
{
#if ENABLETRACE
GSSiExitProg (753);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}



long GetGFCmdID (LPSTR Name)
#if ENABLETRACE
{GSSiEnterProg (756);
#endif
{
	long	ID=ConvertCMDToNum(Name);
	if (ID >0)
		ID = 0;
{
#if ENABLETRACE
GSSiExitProg (756);
#endif
	return -ID;
}
#if ENABLETRACE
}
#endif
}

     
void AddUserPopup (HMENU hNewMenu)
#if ENABLETRACE
{GSSiEnterProg (757);
#endif
{
	LPINT	pNumPops;
	LPHMENU	phMenu;
	
	if (!hNewPopups)
{
#if ENABLETRACE
GSSiExitProg (757);
#endif
		return;
}
	pNumPops = (LPINT)GlobalLock (hNewPopups);
	pNumPops++;
	phMenu = (LPHMENU)pNumPops;
	pNumPops--;
	phMenu += *pNumPops;
	(*pNumPops)++;
	*phMenu = hNewMenu;
	GlobalUnlock (hNewPopups);
{
#if ENABLETRACE
GSSiExitProg (757);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

void DestroyUserPopups (LPHANDLE hPop)
#if ENABLETRACE
{GSSiEnterProg (758);
#endif
{
	LPINT	pNumPops;
	int		NumPops;  
	LPHMENU	phMenu;
	
	if (!*hPop)
{
#if ENABLETRACE
GSSiExitProg (758);
#endif
		return;
}
	pNumPops = (LPINT)GlobalLock (*hPop);
	NumPops = *pNumPops++;
	phMenu = (LPHMENU)pNumPops;
	phMenu += (NumPops-1);
	while (NumPops--)
		DestroyMenu (*phMenu--); 
	GSSiGlobUlFree (hPop);
{
#if ENABLETRACE
GSSiExitProg (758);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}	 

BOOL EditMenu (HWND hWnd,HANDLE hMenuName)
#if ENABLETRACE
{GSSiEnterProg (759);
#endif
{   
	char	str[512], mess[512];
	LPSTR	MenuName; 
	UINT	ierr;
	OFSTRUCTGM	OFStruct;
	HFILE	Fid;
	
	if (!hMenuName)
{
#if ENABLETRACE
GSSiExitProg (759);
#endif
		return FALSE;
}
	MenuName = GlobalLock (hMenuName); 
	if ((Fid =GSSiOpenFile (MenuName,&OFStruct,OF_READWRITE)) == HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (759);
#endif
		return FALSE;  
}
	GSSiClose (Fid);
	GlobalUnlock (hMenuName);
	GMEdit (hWnd,OFStruct.szPathName);
/*	_fstrcpy (str,"NOTEPAD.EXE ");
	_fstrcat (str,OFStruct.szPathName); 
	ExpandText (str);
	if ((ierr = WinExec (str,SW_SHOWMAXIMIZED)) < 32)
	{
		sprintf (mess,"Error loading editor: %i",(int) ierr);
		GSSiMsgBox (hWndMain,mess,0,0,0);  
	}*/
//	if (!GMLoadMenu (hWnd,MenuName))
//	     	GSSiMsgBox( GetFocus(),"Failed to load menu file",0, MB_OK|MB_ICONEXCLAMATION);
{
#if ENABLETRACE
GSSiExitProg (759);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}  

BOOL ReloadMainMenu (void)
{
	BOOL rtn=FALSE;

	if (hStartupMenu)
	{
         LPSTR	StartupMenu=GlobalLock (hStartupMenu);
          
		 rtn = GMLoadMenu (hWndMain,StartupMenu);
		 GlobalUnlock (hStartupMenu);
	}
	return rtn;
}

BOOL GMLoadMenu (HWND hWnd,LPSTR Name)
#if ENABLETRACE
{GSSiEnterProg (760);
#endif
{               
	HFILE	MFid;
	OFSTRUCTGM	OFStruct;
	HMENU	hMenu=0, hMenuOld; 
	int		line=0, l;  
	long	NewLen;
	LPLONG	pNumCmd, pCmdOffset; 
	LPSHORT	pNumPop; 
	LPSTR	pCmd;
	char	Name2[MAX_PATH]="[%DL]";
	
	if (SysMenu || !Name || !*Name)
{
#if ENABLETRACE
GSSiExitProg (760);
#endif
		return TRUE;
}
	MFid = GSSiOpenFile (Name,&OFStruct,OF_READ); 
	if (MFid == HFILE_ERROR)
	{
		if (!NameContainsDL (Name))
		{
			_fstrcat (Name2,Name);
			MFid = GSSiOpenFile (Name2,&OFStruct,OF_READ); 
		}
		if (MFid == HFILE_ERROR)
		{   
			char	mess[256];
			
			sprintf (mess,"Menu file %s not found or locked",Name);
			GSSiMsgBox (GetFocus(),mess,0,MB_ICONEXCLAMATION,0);
{
#if ENABLETRACE
GSSiExitProg (760);
#endif
			return FALSE;
}
		} 
		else
			AddToMacroStack(2, 0, Name2, 0,0);

	}
	else
		AddToMacroStack(2, 0, Name, 0, 0);

	GSSiGlobFree (&hUserCmd);
	phWhichCmdList = &hUserCmd;
	hUserCmd = GSSiGlobAlloc ( 360,GHND,USHRT_MAX);
	hNewPopups = GSSiGlobAlloc ( 361,GHND,4096); 
	
	hMenu = GMCreateMenu (MFid,&line,Name); 
	GSSiClose (MFid); 
	GetCmdID (0,0);
	if (!hMenu)
{
#if ENABLETRACE
GSSiExitProg (760);
#endif
		return FALSE;   
}
	hMenuOld = GetMenu (hWnd);
	SetMenu (hWnd,hMenu); 
	hUserMenu = hMenu;
	DestroyUserPopups (&hCurPopups); 
	if (hMenuOld)
		DestroyMenu (hMenuOld);  
	hCurPopups = hNewPopups; 
	hNewPopups = 0;
	pNumCmd = (LPLONG)GlobalLock (hUserCmd);
	if (!*pNumCmd)
		GSSiGlobUlFree (&hUserCmd);
	else
	{
		pCmdOffset = pNumCmd;
		pCmdOffset += *pNumCmd;
		pCmd = (LPSTR)pNumCmd;
		pCmd += *pCmdOffset;
		l = _fstrlen (pCmd);
		NewLen = *pCmdOffset + l + 1;
		GlobalUnlock (hUserCmd);
		hUserCmd = GSSiGlobalReAlloc (0,hUserCmd,NewLen,GMEM_MOVEABLE);
//		pNumCmd = GlobalLock (hUserCmd);
	}
{
#if ENABLETRACE
GSSiExitProg (760);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}
 
HMENU GMCreateMenu (HFILE MFid,LPINT line,LPSTR FileName)
#if ENABLETRACE
{GSSiEnterProg (761);
#endif
{   LPSTR	lpStr1, lpStr2, lpMenuText, lpEnd,lpCmdID;
	HANDLE	hStr=GSSiGlobAlloc ( 362,GMEM_MOVEABLE,6*1024);
	LPSTR	str=GlobalLock (hStr);
	LPSTR	OrigLine=str+1024, mess=OrigLine+1024, mess2=mess+1024, Exp=mess2+1024, MenuText=Exp+1024;   
	BOOL	lastblank, InQuote; 
	HMENU	hMenu=0, hMenu2;
	UINT	CmdID; 
	int		BeginLine=*line, checked; 
	HFILE	FidLast=HFILE_ERROR;
	OFSTRUCTGM	OFStruct;
	BOOL	SkipLine=FALSE;
	BOOL	first = TRUE;
	int		startLoc = GSSillseek (MFid,0,1);
	short	ii;

Top:	
	while (fgetstring (str,1020,MFid))
	{  
		if (stricmp (str,"BEGIN") && first && GetDebug ())
		{
			sprintf (str,"MENUITEM \"Edit Menu\", \"[C]=$EDITFILE(%s,T)\"",FileName);
			GSSillseek (MFid,startLoc,0);
			first = FALSE;
		}
		startLoc = GSSillseek (MFid,0,1);
		(*line)++;  
		_fstrcpy (OrigLine,str);
		lpStr1 = lpStr2 = str;
		lastblank = TRUE; 
		InQuote = FALSE;
		while (*lpStr1)
		{
			if ((*lpStr1 == ' ' || *lpStr1 == '\t') && !InQuote)
			{
				if (lastblank)
					lpStr1++;
				else
				{
					lastblank = TRUE;
					*lpStr2++ = ' ';
					lpStr1++;
				}
			}
			else if (*lpStr1 == '"')
			{
				if (InQuote)
					InQuote = FALSE;
				else
					InQuote = TRUE;
				*lpStr2++ = *lpStr1++;
			}
			else
			{ 
				*lpStr2++ = *lpStr1++;
				lastblank = FALSE;		
			}
					
		} 
		*lpStr2 = 0;
		if (!_fstrncmp (str,"IF(",3))
		{   
			BOOL	irc;
			
			_fstrcpy (Exp,str+3);
			*LastChr (Exp) = 0;
			if (LogicP (Exp,&irc))
				SkipLine = FALSE;
			else
				SkipLine = TRUE;
		}
		else if (!_fstrncmp (str,"ELSE",4))
		{ 
			SkipLine = !SkipLine;
		}
		else if (!_fstrncmp (str,"ENDIF",5))
		{ 
			SkipLine = FALSE;
		}
		else if (SkipLine)
		{
			ii=1;
		}
		else if (!_fstrncmp (str,"INCLUDE ",8))
		{   
			LPSTR	pSpace = _fstrchr(&str[8],' ');
			if (pSpace)
			{
				*pSpace++ = 0;
				_fstrcpy(IncludeFileArg,pSpace);
			}
			else 			
				*IncludeFileArg = 0;
			FidLast = MFid;
			MFid = GSSiOpenFile (&str[8],&OFStruct,OF_READ);
			if (MFid == HFILE_ERROR)
				MFid = FidLast;
			goto Top;
		}
		else if (!_fstrncmp (str,"BEGIN",5))
		{
			if (*line == 1)
			{
				if (!hMenu)
					hMenu = CreateMenu();
			}
			else 
			{
				hMenu = CreatePopupMenu();  
				AddUserPopup (hMenu);
			}
		}
		else if (!_fstrncmp (str,"END",3)) 
		{   
			GSSiGlobUlFree (&hStr);
			if (FidLast != HFILE_ERROR)
			{   
				GSSiClose (MFid);
				MFid = FidLast;
				FidLast = HFILE_ERROR;  
			}
{
#if ENABLETRACE
GSSiExitProg (761);
#endif
			return hMenu;
}
		}
		else if (!_fstrncmp (str,"POPUP",5)) 
		{   
			BOOL 	Enabled=TRUE;
			
			hMenu2 = GMCreateMenu (MFid,line,FileName);
			if (!hMenu2) 
			{
				GSSiGlobUlFree (&hStr);
{
#if ENABLETRACE
GSSiExitProg (761);
#endif
				return FALSE;   
}
			}
			lpMenuText = _fstrchr (str,'"');
			if (!lpMenuText)
				goto RtnFalse;
			lpMenuText++;
			lpEnd = _fstrchr (lpMenuText,'"');
			if (!lpEnd)
				goto RtnFalse;
			*lpEnd = 0; 
			_fstrcpy (MenuText,lpMenuText);
			ExpandText (MenuText);
			lpEnd = _fstrrchr (MenuText,'^');
			if (lpEnd) 
			{
				*lpEnd++ = 0; 
				Enabled = atob (lpEnd);
			}
			if (Enabled && *MenuText)
				AppendMenu(hMenu, MF_ENABLED | MF_POPUP, (UINT)hMenu2,MenuText);
			else if (*MenuText)
				AppendMenu(hMenu, MF_GRAYED | MF_POPUP, (UINT)hMenu2,MenuText);
		}
		else if (!_fstrncmp (str,"MENUITEM",8)) 
		{   
			BOOL 	Enabled=TRUE;

			if (!_fstrncmp (&str[9],"SEPARATOR",9))
				AppendMenu(hMenu, MF_SEPARATOR, 0,0);
			else
			{
				lpMenuText = _fstrchr (str,'"');
				if (!lpMenuText) goto RtnFalse;
				lpMenuText++;
				lpEnd = _fstrchr (lpMenuText,'"');
				if (!lpEnd) goto RtnFalse;
				*lpEnd = 0; 
				lpEnd++;
				if (*lpEnd != ',')
				{   
					sprintf (mess2,"Error loading menu %s",FileName);
					sprintf (mess,"Missing comma on line %i\n\r%s",*line,OrigLine);
		 			GSSiMsgBox(GetFocus(), mess, mess2,MB_ICONEXCLAMATION|MB_OK,0);
					GSSiGlobUlFree (&hStr);
{
#if ENABLETRACE
GSSiExitProg (761);
#endif
					return 0;
}
				} 
				lpCmdID = ++lpEnd;  
				if (*lpCmdID == ' ') lpCmdID++; 
				if (!*lpCmdID)
				{   
					sprintf (mess2,"Error loading menu %s",FileName);
					sprintf (mess,"Missing command ID on line %i\n\r%s",*line,OrigLine);
		 			GSSiMsgBox(GetFocus(), mess, mess2,MB_ICONEXCLAMATION|MB_OK,0);
					GSSiGlobUlFree (&hStr);
{
#if ENABLETRACE
GSSiExitProg (761);
#endif
					return 0; 
}
				}
				lpEnd = MatchLev (lpCmdID,',');
				if (!lpEnd)
					lpEnd = _fstrchr (lpCmdID,0);
				else
					*lpEnd = 0;
				REPLAC (lpMenuText,"\\t","\t",1024); 
				_fstrcpy (MenuText,lpMenuText);
				ExpandText (MenuText); 
				lpEnd = _fstrrchr (MenuText,'^');   
				if (lpEnd) 
				{
					*lpEnd++ = 0; 
					Enabled = atob (lpEnd);
				}   
				checked = 0;
				lpEnd = _fstrrchr (MenuText,'[');   
				if (lpEnd && strnicmp (lpEnd,"[%DL]",5)) 
				{
					*lpEnd++ = 0; 
					if (atob (lpEnd))
						checked = MF_CHECKED;
				}  
				CmdID = GetCmdID (lpCmdID,MenuText);
				if (!CmdID)
				{   
					sprintf (mess2,"Error loading menu %s",FileName);
					sprintf (mess,"Invalid command ID '%s' on line %i\n\r%s",lpCmdID,*line,OrigLine);
		 			GSSiMsgBox(GetFocus(), mess, mess2,MB_ICONEXCLAMATION|MB_OK,0);
					GSSiGlobUlFree (&hStr);
{
#if ENABLETRACE
GSSiExitProg (761);
#endif
					return 0;
}
				}  
				if (Enabled && *MenuText)
				{
					if (!hMenu)
						hMenu = CreateMenu();
					AppendMenu(hMenu, MF_ENABLED|checked, CmdID,MenuText); 
				}
				else if (*MenuText)
					AppendMenu(hMenu, MF_GRAYED, CmdID,MenuText);
			}
		} 
	}
	if (FidLast != HFILE_ERROR)
	{   
		GSSiClose (MFid);
		MFid = FidLast;
		FidLast = HFILE_ERROR;
		goto Top;
	} 
	sprintf (mess,"Missing END statement for BEGIN at line %i",BeginLine);
	sprintf (mess2,"Error loading menu %s",FileName);
	GSSiMsgBox(GetFocus(), mess, mess2,MB_ICONEXCLAMATION|MB_OK,0);
	GSSiGlobUlFree (&hStr);
{
#if ENABLETRACE
GSSiExitProg (761);
#endif
	return FALSE;
}
RtnFalse:
	sprintf (mess,"Error in menu file on line %i\n\r%s",*line,OrigLine);
	sprintf (mess2,"Error loading menu %s",FileName);
	GSSiMsgBox(GetFocus(), mess, mess2,MB_ICONEXCLAMATION|MB_OK,0);
	GSSiGlobUlFree (&hStr);
{
#if ENABLETRACE
GSSiExitProg (761);
#endif
	return FALSE;
}
#if ENABLETRACE
}
#endif
} 

LPVIEWPORT GetVPByName (LPSTR Name)
#if ENABLETRACE
{GSSiEnterProg (762);
#endif
{   
	int	iview;
	
	for (iview=0;iview<*pNumViewports;iview++)
	{   
    	if (!_fstrcmp (Name,pViewports[iview]->Name))
{
#if ENABLETRACE
GSSiExitProg (762);
#endif
    		return (pViewports[iview]);
}
    }
{
#if ENABLETRACE
GSSiExitProg (762);
#endif
    return 0;
}
#if ENABLETRACE
}
#endif
}

long OutputHLTAreas (int HLTOUTFormat,LPSTR HLTOutPath)
#if ENABLETRACE
{GSSiEnterProg (763);
#endif
{                                                      
	long	nareas=0;  
	HIGHLIGHTDATA	HighlightData; 
	long	Refno; 
	int		pos=BT_FIRST; 
	LPTHEME	pTheme;                                        
	OFSTRUCTGM	OFStruct;
	HFILE	FidOut;
	char	txt[128];  
	DPOINT	CP;
	
	FidOut = GSSiOpenFile (HLTOutPath,&OFStruct,OF_CREATE);
		
   	while (!BT_FIND (hHighlight,(LPSTR)&Refno,pos,BT_ANY,(LPSTR)&HighlightData))  
   	{
   		pos=BT_NEXT;
   		PickList[0]=HighlightData.PD;
    	if (PickList[0].Type == 3)
    	{   
			SetConfig (PickList[0].ConfigID);
		    SetViewport (PickList[0].ViewID);
    		pTheme = AddTheme (GF_SAVEPOLY_THEME);
    		CurView->PassID = 4;
			ProcessSelectedTheme = CurView->NumThemes;
			ProcessPickedItem (0,FALSE); 
			ProcessSelectedTheme = 0;       		
    		DeleteTheme (pTheme); 
    		while (GetSavedPolys ())
    		if (hSavePoly)
    		{   LPMNMXCORD lpRect;
				HPDPOINT	lpDpoint;
    		    
    		    nareas++;  
    		    CP.x=0;
    		    CP.y=0;
                nPnts = nSavePoly; 
                lpRect = (LPMNMXCORD) GlobalLock (hSavePoly);
                lpRect++;
                lpDpoint = (LPDPOINT) lpRect;
                while (nPnts--)
                {
                	CP.x+=lpDpoint->x;
                	CP.y+=lpDpoint++->y;
                }  
                GlobalUnlock (hSavePoly);
                nPnts = nSavePoly; 
                CP.x/=nPnts;
                CP.y/=nPnts;
    		    sprintf (txt,"%10ld%28.16E%28.16E",nareas,CP.x,CP.y);  
    		    fputstring (txt,FidOut);
                lpRect = (LPMNMXCORD) GlobalLock (hSavePoly);
                lpRect++;
                lpDpoint = (LPDPOINT) lpRect;
                while (nPnts--)
                {   
                	double x,y;
                	
                	x=lpDpoint->x;
                	y=lpDpoint->y;  
                	lpDpoint++;
                	sprintf (txt,"%28.16E%28.16E",x,y); 
                	fputstring (txt,FidOut);
                }  
                fputstring ("END",FidOut);
                GSSiGlobUlFree (&hSavePoly);
             }
         }
    }
    fputstring ("END",FidOut);
	GSSiClose (FidOut);
{
#if ENABLETRACE
GSSiExitProg (763);
#endif
	return nareas;
}
#if ENABLETRACE
}
#endif
}

BOOL EditRectInit (short InfoLen)
#if ENABLETRACE
{GSSiEnterProg (767);
#endif
{
	if (CurView->hEditRect)
{
#if ENABLETRACE
GSSiExitProg (767);
#endif
		return FALSE;
}
	CurView->HaveEditRect = FALSE; 
	CurView->EditRectInfoSize = InfoLen;
	CurView->hEditRect = GSSiGlobAlloc ( 363,GHND,USHRT_MAX);
	CurView->EditRectSize = sizeof(RECT) + InfoLen;  
	CurView->MaxEditRect = USHRT_MAX / CurView->EditRectSize;
	CurView->NumEditRects = 0;
{
#if ENABLETRACE
GSSiExitProg (767);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

void CloseEditRect (LPVIEWPORT CurView)
#if ENABLETRACE
{GSSiEnterProg (768);
#endif
{
	GSSiGlobFree (&CurView->hEditRect);
	CurView->HaveEditRect = FALSE;
{
#if ENABLETRACE
GSSiExitProg (768);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

void InvertRotatedRect (HDC hDC,LPEDITRECTINFO pEditRect)
{   
	double	DAZ = (double)pEditRect->AZ/1000;
	POINT	Points[4], TempPoint; 
	int		OldMode = SetROP2(hDC,R2_NOT); 
	HBRUSH	OldBrush;
    
    TempPoint = newptscreen (pEditRect->MidPoint,DAZ,-pEditRect->Width/2); 
    Points[0] = newptscreen (TempPoint,DAZ+HALFPI,-pEditRect->Height/2);
    Points[1] = newptscreen (TempPoint,DAZ+HALFPI,pEditRect->Height/2);
    TempPoint = newptscreen (pEditRect->MidPoint,DAZ,pEditRect->Width/2); 
    Points[2] = newptscreen (TempPoint,DAZ+HALFPI,pEditRect->Height/2);
    Points[3] = newptscreen (TempPoint,DAZ+HALFPI,-pEditRect->Height/2);
	OldBrush = SelectObject (hDC,GetStockObject (BLACK_BRUSH));
	Polygon (hDC,Points,4);
	SetROP2(hDC,OldMode); 
	SelectObject (hDC,OldBrush);
	return;
}		
   

BOOL EditRectTrack (POINTS pointS)
#if ENABLETRACE
{GSSiEnterProg (769);
#endif
{  
	short	n=CurView->NumEditRects;
	HRGN	hRgn;  
	BOOL	rtn=FALSE;
	POINT	MidPoint, point=POINTStoPOINT(pointS);
	double	dist,mindist=DBL_MAX;
	LPEDITRECTINFO	pRectInfo; 
	UINT	EditRectPrompt;
	
    SetDisplayMode (CurView->hDC, GF_SCREENMODE);
    GSSiDeleteObject(&CurView->hRgn);
    CurView->hRgn = CreateVPRgn (FALSE,FALSE);
    SelectClipRgn (CurView->hDC,CurView->hRgn);
    GSSiDeleteObject(&CurView->hRgn);  
	if (CurView->HaveEditRect)
		InvertRotatedRect (CurView->hDC,&CurEditRect);		
	if (!CurView->hEditRect)
{
#if ENABLETRACE
GSSiExitProg (769);
#endif
		return FALSE;
}
	pRectInfo = (LPEDITRECTINFO)GlobalLock(CurView->hEditRect); 
	CurView->HaveEditRect = FALSE;
	while (n--)
	{ 
		if (PtInRect((LPRECT)pRectInfo,point))
		{
			//SetDisplayMode (CurView->hDC, GF_TEXTMODE);   
			CurEditRect = *pRectInfo;
			MidPoint = RectMid ((LPRECT)pRectInfo);
			dist = idist (point,MidPoint);
			if (dist < mindist)
			{   
				mindist = dist;
				CurEditRect = *pRectInfo;
				switch (pRectInfo->AddID)
				{  
					case 0:
					case 1:
					case 2:
					case 3:
						EditRectPrompt = PRMT_EDITHOUSENUM;
						break;
						
					case 4:
					case 5:
					case 6:
					case 7:
						EditRectPrompt = PRMT_EDITSTREETNAME;
						break;
					case 8:
						EditRectPrompt = PRMT_EDITSPEED;
						break;
					case 9:
						EditRectPrompt = PRMT_EDITTRAFVOL;
						break;
					case 10:
						EditRectPrompt = PRMT_EDITONEWAY;
						break;
					case 11:
						EditRectPrompt = PRMT_EDITLANES;
				}
		    	SetPrompt (EditRectPrompt,TRUE); 
				CurView->HaveEditRect = TRUE;
				rtn=TRUE;
			}	
		}
		pRectInfo++;
	}
	if (rtn)
		InvertRotatedRect (CurView->hDC,&CurEditRect);	
	GlobalUnlock (CurView->hEditRect); 
{
#if ENABLETRACE
GSSiExitProg (769);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}

BOOL AddEditRect(LPRECT Rect, LPEDITRECTINFO pInfo)
#if ENABLETRACE
{GSSiEnterProg (770);
#endif
{
	LPRECT	pRect;  
	LPEDITRECTINFO	radd;
    
    if (!CurView->hEditRect || (CurView->NumEditRects >= CurView->MaxEditRect))
{
#if ENABLETRACE
GSSiExitProg (770);
#endif
    	return FALSE;
}
	radd = (LPEDITRECTINFO)GlobalLock(CurView->hEditRect); 
	radd += CurView->NumEditRects;
	*radd = *pInfo;
	radd->Rect = *Rect;
	CurView->NumEditRects++;
	GlobalUnlock(CurView->hEditRect);
{
#if ENABLETRACE
GSSiExitProg (770);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
} 

void DisplayStreetText(void)
#if ENABLETRACE
{GSSiEnterProg (771);
#endif
{   
	struct {long StreetNum; float Length;} SNSelectKey;
	typedef struct {POINT Point; float AZ;short Layer, State, height,symnum;long Refno;COLORREF TextColor,ShadowColor;} SNSELECTDATA;
	SNSELECTDATA	SNSelectData,LastData;  
	struct {short	Layer, Number, State; long StreetNum;} SortKey;
	HANDLE	hSort=0,hSelect=0;
	short	pos=BT_FIRST;  
	DPOINT	DPoint; 
	LPDPOINT	lpDPoint=&DPoint;
	CITIESKEY1	CitiesKey1; 
	CITIESDATA1	CitiesData1;
	CITIESDATA4	CitiesData4;
	CITIESKEY2	CitiesKey2; 
	char	Name[260]; 
	BTVARDESC	BTVar[4];
	TIGER1_PEOPLENET	Tiger1PN; 
	OFSTRUCTGM	OFStruct;
	long	Offset, LastNum;     
	char	SelectFile[144], SortFile[144],SaveFontName[LF_FACESIZE+2];
	short	n;    
	COLORREF	CityCircleColor, CityTextColor, StreetTextColor[16]; 
	long	MinDisplayPop,MinDisplayPop2;    
	double	MinTextSize, LayerStreetTextSize[16];
	int		HaveState=-1;
	int		pos2, cond2;   
	DWORD	TextDisplayed;  
	double	TextFactor;    
	clock_t	starttime;
	//LPSHORT	pnMP;
	long	nMP; 
	LPMIDPOINT	pMidPoint; 
	HFILE	Cities4FID;
	int	isize, Yoff;
	double Size, Radius, Width; 
	POINT	Point;   
	BOOL	DoCircle,UseLayerSize=TRUE, First;
	COLORREF	OldColor; 
	int		NumStates, i, istate, laststate;
	long	StateOffsets[70], lastpop, FIPS,ii;
	long		lbuf = 500*sizeof(CitiesData4), lenread;
	HANDLE		hBuf=0;
	LPCITIESDATA4	pBuf, pCitiesData4;
	int			nbuf,NumDisplayCitiesOpt,Weight,RegionType; 
	long		MaxCitiesDisplayed,NumCitiesDisplayed; 
	HRGN		hRgn;
	HFILE		FidOut=HFILE_ERROR;
    LPSTREETTEXTDATA	pStreetData=(LPSTREETTEXTDATA)CurTheme->ClassBM;
	HCURSOR	hcurSave;
	LPVIEWPORT	SaveVP=CurView;    
	double	ThemeTextSize, TextSize; 
	COLORREF	White=RGB(255,255,255);
	BOOL	PreventOverlap = !pStreetData->AllowOverlap;
	BOOL	ShowCityCircle = GetGlobalBVal2 ("[%SHOWCITYCIRCLE]",TRUE);
	BOOL	ShadowCityText = GetGlobalBVal2 ("[%SHADOWCITYTEXT]",TRUE);
	short	CityTextVJust=1;  
	BOOL	SaveFHT; 
	long	MinDistBetweenNames = GetGlobalLVal2 ("[%MINDISTBETWEENNAMES]",0);
	POINT	NamePoints[16];
	short	NumNamePoints;   
	float	Length;
	
	if (!ShowCityCircle)
		CityTextVJust = 2;  
	if (!ContinueProcessing) 
{
#if ENABLETRACE
GSSiExitProg (771);
#endif
		return;
}
	GetGlobalCVal ("[%LABELFONT]",SaveFontName,0);
	if (DoTime)
		starttime=GetTickCount(); 
	hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT)); 
//	SetCurView ( pViewports[CurTheme->TargetViewport-1]); //tempdebu
	SetViewport(CurTheme->TargetViewport);
	SaveDC (CurView->hDC);
	SetDisplayMode (CurView->hDC, GF_TEXTMODE);   
	hRgn = CreateVPRgn (FALSE,FALSE);
  	RegionType = SelectClipRgn (CurView->hDC,hRgn);
  	GSSiDeleteObject(&hRgn);   
  	if (RegionType == NULLREGION && !PrintingToMF)
  		goto Exit2;
	CityCircleColor = GetGlobalLVal ("[CITY_CIRCLE_COLOR]");
	CityTextColor = GetGlobalLVal ("[CITY_TEXT_COLOR]");
	MinDisplayPop = GetGlobalLVal ("[MIN_DISPLAY_POP]");
	MinTextSize = GetGlobalDVal2 ("[MIN_TEXT_SIZE]",5); 
	LayerStreetTextSize[0] = StreetTextSize; 
	StreetTextColor[0] = GetGlobalLVal ("[STREET_TEXT_COLOR]");
	
	for (i=1;i<16;i++)
	{
		sprintf (Name,"[STREET_TEXT_SIZE_%i]",i);
		LayerStreetTextSize[i] = GetGlobalDVal2 (Name,StreetTextSize); 
		sprintf (Name,"[STREET_TEXT_COLOR_%i]",i);
		StreetTextColor[i] = GetGlobalLVal2 (Name,StreetTextColor[0]); 
	}
	TextFactor = GetGlobalDVal2 ("[TEXT_FACTOR]",1); 
	NumDisplayCitiesOpt = GetGlobalLVal ("[NUM_CITIES_TO_DISPLAY]");
	MaxCitiesDisplayed = NumDisplayCitiesOpt; 
	if (MaxCitiesDisplayed <= 0)
		MaxCitiesDisplayed = LONG_MAX;
	
	AddTextRect(0);
	pos = BT_FIRST; 
	
	NumStates = 0; 
	NumCitiesDisplayed=0;
	if (!pStreetData->ShowCities)
		goto DoStreets;  
	for (i=1;i<74;i++)
	{
		if (HaveStates[i])
			NumStates++;
	}
	if (!NumStates || PeopleNet)
		NumStates = 73;
	OldColor = SetTextColor (CurView->hDC,ConvertColor(CityTextColor,CurTheme->UseHalfTone));
	if (NumStates > 5 && !GetGlobalBVal2("[%CREATECITIES]", FALSE))
	{
		Cities4FID = GSSiOpenFile ("[%CITYDATALOC]cities4.dat",&OFStruct,OF_READ); 
		if (Cities4FID == HFILE_ERROR)
			goto DoStreets;
		istate = 1;
	}
	else 
	{   
		long off;
		
		Cities4FID = GSSiOpenFile ("[%CITYDATALOC]cities5.dat",&OFStruct,OF_READ);
		if (Cities4FID == HFILE_ERROR) goto DoStreets;
		off = sizeof(StateOffsets);
		ii=GSSillseek (Cities4FID,-off,2);
		ii=BigRead (Cities4FID,(HPSTR)StateOffsets,sizeof(StateOffsets));
		istate = 69;
	}
	GetGlobalCVal ("[CITY_TEXT_FONT]",Name,"Arial");
	SetGlobalValue("%LABELFONT",Name);
	
	hBuf = GSSiGlobAlloc ( 364,GMEM_MOVEABLE,lbuf);
	pBuf = (LPCITIESDATA4)GlobalLock (hBuf);

	if (GetGlobalBVal2("[%CREATECITIES]", FALSE))
	{
		MinDisplayPop = 0;
		NumStates = 0;
		memset(HaveStates, 0x1, sizeof(HaveStates));
		FidOut = GSSiOpenFile("[%DL]cities6.dat", &OFStruct, OF_CREATE);
	}
	while (istate)
	{	
		if (NumStates <= 5)
		{   
			if (istate == 60)
				goto NextState;
			if (HaveStates[istate])
			{
				if (GSSillseek (Cities4FID,StateOffsets[istate],0) == HFILE_ERROR)
					goto NextState;
			}
			else
				goto NextState;
		}
		lastpop = LONG_MAX; 
		if (NumDisplayCitiesOpt>0)
			MinDisplayPop2=0;
		else
			MinDisplayPop2=MinDisplayPop;
		while((lenread=BigRead (Cities4FID,(HPSTR)pBuf,(UINT)lbuf)))
		{   
			nbuf = lenread/sizeof(CitiesData4);  
			pCitiesData4 = pBuf;
			while (nbuf--)
			{   
				if (pCitiesData4->Pop < MinDisplayPop && NumDisplayCitiesOpt<0 && NumCitiesDisplayed < MaxCitiesDisplayed)
				{
					MinDisplayPop2 = 0;
					MaxCitiesDisplayed = -NumDisplayCitiesOpt;   
					NumDisplayCitiesOpt = 0;
				}
					
				if (pCitiesData4->Pop >= MinDisplayPop2 && pCitiesData4->Pop <= lastpop)
				{   
					long	CvtErr=0;
					
					lastpop = pCitiesData4->Pop; 
					if (FidOut != HFILE_ERROR)
						CvtErr = ConvertCoord (&pCitiesData4->LatLong,2,1);
					if (!CvtErr && PtInWBounds(&pCitiesData4->LatLong))
					{   
						if (FidOut != HFILE_ERROR)
							BigWrite (FidOut,(HPSTR)pCitiesData4,sizeof(CitiesData4),-1);
						Point = BasePtToWinPt (&pCitiesData4->LatLong);
						Size = (log (pCitiesData4->Pop)-log (MinDisplayPop+1))*TextFactor + MinTextSize;
						Size = max (Size,MinTextSize);
						Weight = 300 + (Size - MinTextSize) * 100;
						Radius = Size/2;
						Width = Size/MinTextSize - 1; 
						Yoff =  Radius+Width/2; 
						DoCircle=FALSE;
						if (CurView->HaveOrthos && AutoOpaque)
					    	SetBkMode (CurView->hDC,OPAQUE);
					    else
					    	SetBkMode (CurView->hDC,TRANSPARENT); 
						if (!DispText (CurView->hDC,FALSE,Point.x-10,Point.x+10, Point.y-Yoff, 0,2,CityTextVJust, Radius/20, 1,1,
								  Weight, FALSE, -CurView->Rotation,pCitiesData4->Name,0,TRUE,ShadowCityText,RGB(255,255,255),-1,0,0,0,0,0,CurTheme->UseHalfTone,0,0,0,0,0,0))
						{   
							//if (PeopleNet) 
							if (ShowCityCircle)
							if (DispText (CurView->hDC,FALSE,Point.x-10,Point.x+10, Point.y+Yoff, 0,2,3, Radius/20, 1,1,
								  Weight, FALSE,  -CurView->Rotation,pCitiesData4->Name,0,TRUE,ShadowCityText,RGB(255,255,255),-1,0,0,0,0,0,CurTheme->UseHalfTone,0,0,0,0,0,0))
								  DoCircle=TRUE;
						}
						else
						{
							if (++NumCitiesDisplayed >= MaxCitiesDisplayed)
								MinDisplayPop2 = LONG_MAX;
 							if (ShowCityCircle)
								DoCircle=TRUE; 
						}
						if (DoCircle)
						{
    				//		hPoints = CreateCirclePoly (pCitiesData4->LatLong,Radius*BaseDistToWinDist,&nPoints);
							DrawCircle (CurView->hDC,Point,Radius,(short)IDNINT(Width),ConvertColor(CityCircleColor,CurTheme->UseHalfTone));
							DrawCircle (CurView->hDC,Point,max(0,Radius-1),1,RGB(255,255,255));
						}
					}
				} 
			pCitiesData4++;
			}
		} 
NextState:	istate--;
	} 
	if (FidOut != HFILE_ERROR)
		GSSiClose (FidOut);
	GSSiGlobUlFree (&hBuf);
	GSSiClose (Cities4FID);  
DoStreets:
	if (DisplayStreetLabels (FALSE))
		goto Exit;
	SetTextColor (CurView->hDC,OldColor); 
	if (!CurTheme->hScatterFile)
		goto Exit;                 
	BTVar[0].BT_VARTYP=BT_INTEGER;
	BTVar[0].BT_VARLEN=4;
	BTVar[0].BT_VAROFF=0;
	BTVar[1].BT_VARTYP=BT_REAL;
	BTVar[1].BT_VARLEN=4;
	BTVar[1].BT_VAROFF=4;
	GSSiGetTempFileName (0,"gms",0,(LPSTR)SelectFile);
	BT_CREATE (SelectFile, sizeof(SNSELECTDATA), FALSE, 2, 1, (LPBTVARDESC)BTVar, FALSE, 0, 0, FALSE);
	hSelect = BT_OPEN (SelectFile,0, BT_WRITE, 0);
    if (ncalls1 != ncalls2)
    	ncalls1=0;
	ncalls1++;
	OpenShields (); 
	if (App && CurView->ID != *pCommandViewport && !pStreetData->NameSource)
		ShieldsOnly = TRUE;
	pMidPoint = (LPMIDPOINT)GlobalLock (CurTheme->hScatterFile);
	nMP = CurTheme->NumMidpoint;
	if (!nMP)
		goto EndStreets;
	NumStreetMidPoints = nMP;
	if (!ByState && !pStreetData->NameSource)
		GetTrueStreetName (-1,Name,0,0);
	while(nMP--)
	{   
		if (!ByState || pMidPoint->State >= 0)
		{   
			if (pStreetData->NameSource)
				BlankStreet = 0;
			else if (ByState)
				GetTrueStreetName (-1,Name,pMidPoint->State,0);
			if (pMidPoint->ref != BlankStreet && !PtInTextRect(pMidPoint->Point))
			{   
				SNSelectKey.StreetNum = pMidPoint->ref;
				SNSelectKey.Length = -pMidPoint->Length;
				SNSelectData.Layer = pMidPoint->Layer;
				SNSelectData.State = pMidPoint->State;
				SNSelectData.Point = pMidPoint->Point;
				SNSelectData.height = pMidPoint->height;
				SNSelectData.symnum = pMidPoint->symnum; 
				SNSelectData.Refno = pMidPoint->Refno;
				SNSelectData.TextColor = pMidPoint->TextColor;
				SNSelectData.ShadowColor = pMidPoint->ShadowColor;
				SNSelectData.Refno = pMidPoint->Refno;
				if (pStreetData->HorizontalText)
					SNSelectData.AZ = -CurView->Rotation;
				else
					SNSelectData.AZ = pMidPoint->AZ;  
				BT_PUT (hSelect,(LPSTR)&SNSelectKey,(LPSTR)&SNSelectData); 
			} 
		}
		pMidPoint++;
	}  
	BTVar[0].BT_VARTYP=BT_INTEGER;
	BTVar[0].BT_VARLEN=2;
	BTVar[0].BT_VAROFF=0;
	BTVar[1].BT_VARTYP=BT_INTEGER;
	BTVar[1].BT_VARLEN=2;
	BTVar[1].BT_VAROFF=2;
	BTVar[2].BT_VARTYP=BT_INTEGER;
	BTVar[2].BT_VARLEN=2;
	BTVar[2].BT_VAROFF=4;
	BTVar[3].BT_VARTYP=BT_INTEGER;
	BTVar[3].BT_VARLEN=4;
	BTVar[3].BT_VAROFF=6;
	GSSiGetTempFileName (0,"gms",0,(LPSTR)SortFile);
	BT_CREATE (SortFile, 4, FALSE, 4, 1, (LPBTVARDESC)BTVar, FALSE, 0, 0, FALSE);
	hSort = BT_OPEN (SortFile,0, BT_WRITE, 0);
	pos = BT_FIRST; 
	LastNum = -1;
	n=0; 
/*	First = TRUE;
	while (!BT_FIND (hSelect,(LPSTR)&SNSelectKey,pos,BT_ANY,(LPSTR)&SNSelectData))
	{
		pos=BT_NEXT;    
		if (LastNum < 0)    
			LastNum = SNSelectKey.StreetNum; 
		if ((!First && pStreetData->ShowAllElements) || SNSelectKey.StreetNum != LastNum)
		{   
			SortKey.Layer = LastData.Layer;
			SortKey.Number = n;
			SortKey.StreetNum = LastNum;
			SortKey.State = LastData.State;
			BT_PUT (hSort,(LPSTR)&SortKey,(LPSTR)&n);
			n=1;
		} 
		else
			n++;   
		LastNum = SNSelectKey.StreetNum;
		LastData = SNSelectData; 
		First = FALSE;
	}
	if (n)
	{    
		SortKey.Layer = LastData.Layer;
		SortKey.State = LastData.State;
		SortKey.Number = n;
		SortKey.StreetNum = LastNum;
		BT_PUT (hSort,(LPSTR)&SortKey,(LPSTR)&n);
    } */
	First = TRUE;
	while (!BT_FIND (hSelect,(LPSTR)&SNSelectKey,pos,BT_ANY,(LPSTR)&SNSelectData))
	{
		pos=BT_NEXT;    
		if (pStreetData->ShowAllElements && !MinDistBetweenNames)  
		{
			NumNamePoints = 1;
			goto SelectSegment; 
		}
		if (SNSelectKey.StreetNum != LastNum)
		{
			NumNamePoints = 1;
			NamePoints[0] = SNSelectData.Point;
		}
		else if (MinDistBetweenNames && NumNamePoints < 16)
		{   
			double	d, mindist=DBL_MAX;
			USHORT	ipoint;
			
			for (ipoint = 0;ipoint < NumNamePoints;ipoint++)
			{
				d = idist (SNSelectData.Point,NamePoints[ipoint]);
				if (d < mindist)
				{                  
					mindist = d;
					NamePoints[NumNamePoints] = SNSelectData.Point;
				}
			}
			if (mindist > MinDistBetweenNames)  
				NumNamePoints++; 
			else 
				goto SkipSegment;
		}
		else
			goto SkipSegment;	
SelectSegment:
		{   
			SortKey.Layer = SNSelectData.Layer;
			SortKey.Number = NumNamePoints;
			SortKey.StreetNum = SNSelectKey.StreetNum;
			SortKey.State = SNSelectData.State;
			BT_PUT (hSort,(LPSTR)&SortKey,(LPSTR)&SNSelectKey.Length);
		} 
SkipSegment:  
		LastNum = SNSelectKey.StreetNum;
		LastData = SNSelectData; 
		First = FALSE;
	}
	if (pStreetData->NameSource)
	{
		pStreetData->hNameFile2 = BT_OPEN (pStreetData->NameFile2,0, BT_READ, 0);   
	}
//	OpenSymDict ("TIGER.gsd", OF_READ);     
	pos = BT_FIRST; 
	HaveState = -1;
	if (pStreetData->UseFont && *pStreetData->StreetTextFont.lfFaceName)
	{
		SetGlobalValue("%LABELFONT",pStreetData->StreetTextFont.lfFaceName);
		ThemeTextSize = pStreetData->StreetTextFont.lfHeight*DeviceToScreenFactor();
		UseLayerSize = FALSE; 
	} 
	if (pStreetData->ScaleText)
	{
		UseLayerSize = FALSE;
		ThemeTextSize = pStreetData->TextSize * BaseDistToWinDist;  
	}
	DispText (0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-1,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0); 
	SaveFHT = ForceHalfTone;
	ForceHalfTone = TRUE;
	while (!BT_FIND (hSort,(LPSTR)&SortKey,pos,BT_ANY,(LPSTR)&Length))
	{   
		pos=BT_NEXT;
		SNSelectKey.StreetNum = SortKey.StreetNum;
		SNSelectKey.Length = Length; 
		pos2 = BT_FIRST;
		cond2 = BT_GE;
		TextDisplayed = FALSE;
		while (!TextDisplayed)
		{   
			TextDisplayed = TRUE;
			if (!BT_FIND (hSelect,(LPSTR)&SNSelectKey,pos2,cond2,(LPSTR)&SNSelectData))
			{   
				pos2 = BT_NEXT;
				cond2 = BT_ANY;
				if (SNSelectKey.StreetNum == SortKey.StreetNum && SNSelectData.State == SortKey.State)
				{   
					if (!PtInTextRect(SNSelectData.Point))
					{   
						if (GetStreetThemeName (SNSelectKey.StreetNum,Name,SNSelectData.State))
						{   
							POINT	Point; 
							int		symbol;
							float	ShieldSizeFactor=1,ShieldTextFactor=1;
							
							Point = SNSelectData.Point;
							if (pStreetData->IgnoreShields || (ShieldsOnly && !SymbolInUseShieldsList (SNSelectData.symnum))) 
								symbol = 0;
							else
								symbol=ShieldType (Name,&ShieldSizeFactor,&ShieldTextFactor); 
							if ((symbol && DisplayShields) || pStreetData->NameSource || !ShieldsOnly)// || SortKey.State == 73)   
							{   
								COLORREF	OldColor;
								short		OldBKMode;
								
								if (CurView->Rotation)
								{
									DPOINT DPoint = ScreenPtToBasePt (Point);

									Point = BasePtToWinPt (&DPoint);
								}
								if (!HaveShieldWithinMinDist (Point,0,Name,""))
								{
									if (symbol)
										OldColor = SetTextColor (CurView->hDC,ConvertColor(StreetTextColor[SNSelectData.Layer],CurTheme->UseHalfTone));  
									//else if (*pStreetData->ListFile)
									//	OldColor = SetTextColor (CurView->hDC,0);  
									else if (PeopleNet)
										OldColor = SetTextColor (CurView->hDC,ConvertColor(StreetTextColor[SNSelectData.Layer],CurTheme->UseHalfTone));
									else 
										OldColor = SetTextColor (CurView->hDC,ConvertColor(pStreetData->TextColor,CurTheme->UseHalfTone));
									if (pStreetData->BackgroundOpt || (CurView->HaveOrthos && AutoOpaque)) 
									{
                                		OldBKMode = SetBkMode(CurView->hDC, OPAQUE);  
										OldColor = SetTextColor (CurView->hDC,ConvertColor(pStreetData->TextColor,CurTheme->UseHalfTone));
									}
									else
							    		OldBKMode = SetBkMode (CurView->hDC,TRANSPARENT);
									if (symbol && ShieldsOnly)
									{
							    		short start = GetSymbolTextStart (symbol);
										if (start && _fstrlen(&Name[start])>6)
											goto SkipText;
									} 
									if (UseLayerSize)
									{
                                		Weight = FW_MEDIUM; 
                                		if (Printing && GetTextColor(CurView->hDC) == White)
                                			Weight = 900;
                                		TextSize = LayerStreetTextSize[SNSelectData.Layer]*ShieldTextFactor;
									}
									else 
									{
                                		Weight = 2;
                                		TextSize = IDNINT (ThemeTextSize * ShieldTextFactor);  
									}
									if (TextSize)
										TextDisplayed=DispText (CurView->hDC,FALSE,Point.x-10,Point.x+10, Point.y,SNSelectData.height,2,pStreetData->VJust+1,
										  TextSize*ShieldFactor,ShieldSizeFactor*ShieldFactor,ShieldTextFactor,
										  Weight, pStreetData->Italic,SNSelectData.AZ,Name,symbol,PreventOverlap,
										  pStreetData->Shadow,SNSelectData.ShadowColor,GetTextColor(CurView->hDC),0,0,0,0,0,CurTheme->UseHalfTone,0,0,0,0,0,0);
									else
										TextDisplayed = FALSE;
							SkipText: 
									if (pStreetData->ShowAllElements && !MinDistBetweenNames)
										TextDisplayed = FALSE;
									SetTextColor (CurView->hDC,OldColor); 
									SetBkMode(CurView->hDC, OldBKMode); 
								}
							} 
						} 
					}
				}
				else
					TextDisplayed=TRUE;
			} 
			else
				TextDisplayed=TRUE;
		}
	} 
	ForceHalfTone = SaveFHT;
EndStreets:
	SetGlobalValue("%LABELFONT",SaveFontName);
	DispText (0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-1,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0);
	AddTextRect(0);
    
	if (pStreetData->NameSource)
		BT_CLOSEANDDELETE (&pStreetData->hNameFile2);   
//	*pStreetData->NameFile2 = 0;  
 
	GSSiGlobUlFree (&CurTheme->hScatterFile);   
	CloseStreetNameTable();
	CloseGSStreetNames();	
//	CloseSymDict();
Exit: 
	if (DoTime)
		TextTime += GetTickCount()-starttime;  
	BT_CLOSEANDDELETE (&hSelect); 
	ncalls2++;
	BT_CLOSEANDDELETE (&hSort);
	CloseShields();
Exit2: 
	RestoreDC (CurView->hDC,-1);
	GSSiSetCursor (hcurSave);      
	SetCurView ( SaveVP);
{
#if ENABLETRACE
GSSiExitProg (771);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

BOOL GetStreetThemeName (long InNum,LPSTR Name,short State)
#if ENABLETRACE
{GSSiEnterProg (772);
#endif
{
    LPSTREETTEXTDATA	pStreetData=(LPSTREETTEXTDATA)CurTheme->ClassBM;  
    short	status;
    long	iref=0,Num=InNum; 
    BOOL	rtn;      
    LPSTR	pSpace;
    
	if (!pStreetData->NameSource)      
	{
		rtn = GetTrueStreetName (Num,Name,State,0); 
		if (*Name == '?')
			rtn = FALSE; 
		else
			ConvertSpanishText (Name);
{
#if ENABLETRACE
GSSiExitProg (772);
#endif
		return rtn;
}
	}
	if (Num >= 1000000000)
		Num -= 1000000000;
	*Name = 0;
	if (BT_FIND (pStreetData->hNameFile2,(LPSTR)&Num,BT_FIRST,BT_EQ,Name))
{
#if ENABLETRACE
GSSiExitProg (772);
#endif
		return FALSE;
}   
	if (InNum >= 1000000000)
	{
		if ((pSpace = _fstrrchr (Name,' ')))
			*pSpace = 0;
	}
	Name[GetBTDataLen(pStreetData->hNameFile2)] = 0;
	ConvertSpanishText (Name);

{
#if ENABLETRACE
GSSiExitProg (772);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}  


BOOL OpenShields (void)
#if ENABLETRACE
{GSSiEnterProg (773);
#endif
{   
	char	str[132], SymName[64], IDText[256];     
	HFILE	Fid;
	OFSTRUCTGM	OFStruct;   
	LPSHIELDS	pShields;
	BOOL	Error=FALSE;
	LPSTR	pParen,pComma, pEnd; 
	short	i;
	
	GetGlobalCVal ("[%SHIELDFILE]",str,"shields.txt");
	Fid = GSSiOpenFile (str,&OFStruct,OF_READ); 
	if (Fid == HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (773);
#endif
		return TRUE;
}
	hShields = GSSiGlobAlloc ( 365,GMEM_MOVEABLE,USHRT_MAX);
	nShields = 0;
	pShields = (LPSHIELDS)GlobalLock (hShields);
	while (fgetstring (str,128,Fid))
	{   
		Truncate (str);
		if (!*str) goto Next;
		ExpandText (str);
		ConvertCharBtwnDoubleQuotes (str,' ','|');
		if (sscanf (str,"%s %ld %s",&SymName,&pShields->TextColor,IDText) != 3)
		{ 
			Error = TRUE;
		}
		else  
		{
			RemoveDoubleQuotes (IDText);
			ReplaceChar (IDText,'|',' ');
		}
		if ((pEnd = _fstrchr (IDText,'!')))
			*pEnd++ = 0; 
		_fstrcpy (pShields->IDText,IDText);
		if (pEnd)
			_fstrcpy (pShields->ProcessText,pEnd);
		else
			*pShields->ProcessText = 0;
		pShields->TextFactor = pShields->SizeFactor = 1;
		if ((pParen = _fstrchr (SymName,'(')))
		{
			*pParen++ = 0;  
			if ((pComma = _fstrchr (pParen,',')))
			{
				*pComma++ = 0;
				pShields->TextFactor = atof (pComma);
			}
			pShields->SizeFactor = atof (pParen);   
		}
		 
		if (!(pShields->Symbol = GetDictSymbolNumber(SymName)))
		{
			Error = TRUE;
		}
		if (!Error)
		{   
			pShields->TextStart = _fstrlen (pShields->IDText);
		}
		pShields++;
		nShields++;
Next:;
	}
	GlobalUnlock (hShields); 
	GSSiClose (Fid);  
	if (Error)
	{
		nShields = 0;
		GSSiGlobFree (&hShields);
	}
{
#if ENABLETRACE
GSSiExitProg (773);
#endif
	return Error;
}

/*		USShield=GetDictSymbolNumber("USSHLD");
		StateShield=GetDictSymbolNumber("STATSHLD");
		CountyHwyShield=GetDictSymbolNumber("CHWYSHLD");
		CountyRdShield=GetDictSymbolNumber("CRDSHLD");
		TransCanShield=GetDictSymbolNumber("TRCSHLD");
		ProvinceRtShield=GetDictSymbolNumber("PRTSHLD");
		if (!_fstrncmp (Name,"I- ",3))
			type = InterStateShield;
		else if (!_fstrncmp (Name,"US Hwy ",7))
			type = USShield;
		else if (!_fstrncmp (Name,"State Hwy ",10))
			type = StateShield;	
		else if (!_fstrncmp (Name,"ROUTE ",6))
			type = StateShield;	
		else if (!_fstrncmp (Name,"County Hwy ",11))
			type = CountyHwyShield;	
		else if (!_fstrncmp (Name,"County Rd ",10))
			type = CountyRdShield;	
		else if (!_fstrncmp (Name,"Trans-Canada ",13))
			type = TransCanShield;	
		else if (!_fstrncmp (Name,"Province Rt ",12))
			type = ProvinceRtShield;
		break;
		
		case 2: //MNDOT
		if (!_fstrncmp (Name,"I",1))
			type = InterStateShield;
		else if (!_fstrncmp (Name,"US",2))
			type = USShield;
		else if (!_fstrncmp (Name,"MN",2))
			type = StateShield;	
		else if (*Name)
			type = CountyRdShield;	
		break;     
		
 		if (symbol == InterStateShield)  
			start = 3;
		else if (symbol == USShield)  
			start = 7;
		else if (symbol == StateShield)  
		{
			if (*InText == 'S')
				start = 10;
			else
				start = 6;
		}
		else if (symbol == CountyHwyShield)  
			start = 11;
		else if (symbol == CountyRdShield)  
			start = 10;
		else if (symbol == TransCanShield)  
			start = 13;
		else if (symbol == ProvinceRtShield)  
			start = 12;
		else
			start = 0;   
		if (start && _fstrlen(&InText[start])>4)
		{
			start = 0;
			symbol = 0;
		}    
		break;
		
		case 2:
		if (symbol == InterStateShield)  
			start = 1;
		else if (symbol == USShield)  
			start = 2;
		else if (symbol == StateShield)  
			start = 2;
		else if (symbol == CountyRdShield)  
			start = 0;
		break;*/
#if ENABLETRACE
}
#endif
}

void CloseShields (void)
#if ENABLETRACE
{GSSiEnterProg (774);
#endif
{
	GSSiGlobFree (&hShields); 
	nShields = 0;
{
#if ENABLETRACE
GSSiExitProg (774);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

short ShieldType (LPSTR Name,LPFLOAT pSizeFactor,LPFLOAT pTextFactor)
#if ENABLETRACE
{GSSiEnterProg (775);
#endif
{   
	LPSHIELDS	pShields; 
	UINT		i; 
	short		lname = _fstrlen (Name);
	
	
	if (!hShields)
{
#if ENABLETRACE
GSSiExitProg (775);
#endif
		return 0;
} 
	*ShieldDir = 0;
	pShields = (LPSHIELDS)GlobalLock (hShields);  
	for (i=0;i<nShields;i++,pShields++)
	{
		if (!_fstrnicmp (Name,pShields->IDText,pShields->TextStart))
		{   
			LPSTR	pLastSpace = _fstrrchr (&Name[pShields->TextStart],' ');
			
			if (pLastSpace)
			{
				LPSTR	pBeg = FirstNonBlank (&Name[pShields->TextStart]);

				if (pBeg)
				{
					*pLastSpace = 0;
					pBeg = strchr (pBeg,' ');
					*pLastSpace = ' ';
					if (pBeg)
						goto Exit;
				}
				if (!_fstricmp (pLastSpace," NW") ||
				    !_fstricmp (pLastSpace," NE") ||
				    !_fstricmp (pLastSpace," SW") ||
				    !_fstricmp (pLastSpace," SE") ||
				    !_fstricmp (pLastSpace," N") ||
				    !_fstricmp (pLastSpace," E") ||
				    !_fstricmp (pLastSpace," S") ||
				    !_fstricmp (pLastSpace," W") ||
				    !_fstricmp (pLastSpace," NORTH") ||
				    !_fstricmp (pLastSpace," EAST") ||
				    !_fstricmp (pLastSpace," SOUTH") ||
				    !_fstricmp (pLastSpace," WEST")  
				    )
				    {
				    	switch (ShowShieldDir)   
				    	{
					    	case 0:
					    		*pLastSpace = 0;  
					    		break;
					    	case 2:
					    		*pLastSpace++ = 0; 
							    if (!_fstricmp (pLastSpace,"N"))
							    	_fstrcpy (ShieldDir,"NORTH");
							    else if (!_fstricmp (pLastSpace,"E"))
							    	_fstrcpy (ShieldDir,"EAST");
							    else if (!_fstricmp (pLastSpace,"S"))
							    	_fstrcpy (ShieldDir,"SOUTH");
							    else if (!_fstricmp (pLastSpace,"W"))
							    	_fstrcpy (ShieldDir,"WEST");
							    else
    					    		_fstrcpy (ShieldDir,pLastSpace); 
					    		break;
					    } 
				    	lname = _fstrlen (Name);
				    }
			}
			if (lname > pShields->TextStart && lname - pShields->TextStart < 6)
			{    
				*pSizeFactor = pShields->SizeFactor;
				*pTextFactor = pShields->TextFactor;  
				_fstrupr (Name);
				GlobalUnlock (hShields); 
{
#if ENABLETRACE
GSSiExitProg (775);
#endif
				return (i+1);
} 
			}
		}
	}
Exit:
	GlobalUnlock (hShields);
{
#if ENABLETRACE
GSSiExitProg (775);
#endif
	return 0;
}
#if ENABLETRACE
}
#endif
}

BOOL DisplayShield (LPSTR Name,POINT Point)
#if ENABLETRACE
{GSSiEnterProg (776);
#endif
{
{
#if ENABLETRACE
GSSiExitProg (776);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
} 

int GetSymbolTextStart (int symbol)
#if ENABLETRACE
{GSSiEnterProg (777);
#endif
{
	LPSHIELDS	pShields; 
	short		i=nShields, start=0;
	
	
	if (!hShields || !symbol)
{
#if ENABLETRACE
GSSiExitProg (777);
#endif
		return 0;
}
	pShields = (LPSHIELDS)GlobalLock (hShields);
	pShields += (symbol-1);  
	start = pShields->TextStart;
	GlobalUnlock (hShields);
{
#if ENABLETRACE
GSSiExitProg (777);
#endif
	return start;
}
#if ENABLETRACE
}
#endif
} 

BOOL ProcessShieldText (int symbol,LPSTR Text)
#if ENABLETRACE
{GSSiEnterProg (777);
#endif
{
	LPSHIELDS	pShields; 
	short		i=nShields, start=0;  
	char	Text2[128];
	
	
	if (!hShields || symbol <= 0 || symbol > nShields)
{
#if ENABLETRACE
GSSiExitProg (777);
#endif
		return FALSE;
}
	pShields = (LPSHIELDS)GlobalLock (hShields);
	pShields += (symbol-1);  
	if (*pShields->ProcessText)
	{
		SetGlobalValue ("%SHIELDTEXT",Text);				
		_fstrcpy (Text,pShields->ProcessText);
		ExpandText (Text);
	}
	GlobalUnlock (hShields);
{
#if ENABLETRACE
GSSiExitProg (777);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
} 

int GetShieldSymbol (int shield) 
#if ENABLETRACE
{GSSiEnterProg (778);
#endif
{
	LPSHIELDS	pShields; 
	short		i=nShields, symbol;
	
	
	if (!hShields || !shield)
{
#if ENABLETRACE
GSSiExitProg (778);
#endif
		return 0;
}
	pShields = (LPSHIELDS)GlobalLock (hShields);
	pShields += (shield-1);  
	symbol = pShields->Symbol;
	GlobalUnlock (hShields);
{
#if ENABLETRACE
GSSiExitProg (778);
#endif
	return symbol;
}
#if ENABLETRACE
}
#endif
} 

BOOL SetShieldTextColor (HDC hDC,int symbol,COLORREF *pOldColor,short UseHalfTone)
#if ENABLETRACE
{GSSiEnterProg (779);
#endif
{
	LPSHIELDS	pShields; 
	short		start=0; 
	
	if (!hShields || !symbol)
{
#if ENABLETRACE
GSSiExitProg (779);
#endif
		return FALSE;
}
	pShields = (LPSHIELDS)GlobalLock (hShields);  
	pShields += (symbol-1);  
	*pOldColor = SetTextColor (hDC,ConvertColor(pShields->TextColor,UseHalfTone));  
	GlobalUnlock (hShields);
{
#if ENABLETRACE
GSSiExitProg (779);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}  

void RemoveColorFromRect_old (HDC hDC,COLORREF RemoveColor,LPRECT pRect)
#if ENABLETRACE
{GSSiEnterProg (780);
#endif
{   
	int 	x,y;
	long	Color, LastColor=-1;
	long	ii;
	
	y = pRect->top;
	while (y <= pRect->bottom)
	{
		x = pRect->left; 
		while (x <= pRect->right)
		{   
			Color = GetPixel (hDC,x,y);
			if (Color != -1)
				ii=1;
			if (Color != RemoveColor)
				LastColor = Color;
			else if (LastColor >= 0)
				SetPixel (hDC,x,y,LastColor);
			x++;
		}
		y++;
	}
{
#if ENABLETRACE
GSSiExitProg (780);
#endif
	return;
}
#if ENABLETRACE
}
#endif
} 

void RemoveColorFromRect (HDC hDC,COLORREF RemoveColor,LPRECT pRect)
#if ENABLETRACE
{GSSiEnterProg (780);
#endif
{
	static	int	jj=0;

	if (jj)
	{
		RemoveColorFromRect_old  (hDC,RemoveColor,pRect);
		return;
	}
{   
	UINT 	x,i;
	int		w=RECTWIDTH (pRect), h=RECTHEIGHT(pRect);
	long	Color, LastColor=-1;
	long	ii;
	BITMAP	bm;
	HBITMAP	hBM, hBMTemp;
	LPBYTE	pbits;
    HDC		hDC2 = CreateCompatibleDC(hDC); 
	HBITMAP	hBM2 = CreateCompatibleBitmap (hDC,1,1);
	BITMAPINFO	BitmapInfo;
	LPRGBTRIPLE	pRGBTriple;
	LPRGBQUAD	pRGBQuad;
	BOOL	Changed = FALSE;

	memset (&BitmapInfo,0,sizeof(BITMAPINFO));
	GetObject (hBM2,sizeof(BITMAP),&bm);
	GSSiDeleteObject (&hBM2);
	BitmapInfo.bmiHeader.biSize = sizeof (BITMAPINFOHEADER);
	BitmapInfo.bmiHeader.biPlanes = bm.bmPlanes;
	BitmapInfo.bmiHeader.biBitCount = bm.bmBitsPixel;
	BitmapInfo.bmiHeader.biHeight = h;
	BitmapInfo.bmiHeader.biWidth = w;
	hBMTemp = CreateDIBSection(hDC2,&BitmapInfo,0,&pbits,0,0);

	GetObject (hBMTemp,sizeof(BITMAP),&bm);
	hBM = SelectObject (hDC2,hBMTemp);
	ii=BitBlt (hDC2,0,0,w,h,hDC,pRect->left,pRect->top,SRCCOPY);
	
	switch (bm.bmBitsPixel)
	{
	case 24:
		for (i=0;i<h;i++)
		{
			pRGBTriple = (LPRGBTRIPLE)(pbits + (i*bm.bmWidthBytes));
			for (x=0;x<w;x++,pRGBTriple++)
			{
				Color = COLORREFFromRGBTRIPLE (*pRGBTriple);
				if (Color != RemoveColor)
					LastColor = Color;
				else if (LastColor >= 0)
				{
					Changed = TRUE;
					*pRGBTriple = RGBTRIPLEFromCOLORREF (LastColor);
				}
			}
		}
		break;
	case 32:
		for (i=0;i<h;i++)
		{
			pRGBQuad = (LPRGBQUAD)(pbits + (i*bm.bmWidthBytes));
			for (x=0;x<w;x++,pRGBQuad++)
			{
				Color = COLORREFFromRGBQUAD (*pRGBQuad);
				if (Color != RemoveColor)
					LastColor = Color;
				else if (LastColor >= 0)
				{
					Changed = TRUE;
					*pRGBQuad = RGBQUADFromCOLORREF (LastColor);
				}
			}
		}
		break;
	default:
		break;
	}
	if (Changed)
	{
		RECT	rct;

		rct.left = 0;
		rct.right = w;
		rct.bottom = h;
		rct.top = 0;
	//	InvertRect (hDC2,&rct);
		ii=BitBlt (hDC,pRect->left,pRect->top,w,h,hDC2,0,0,SRCCOPY);
	}
	SelectObject (hDC2,hBM);
	DeleteDC (hDC2);
	GSSiDeleteObject (&hBMTemp);
{
#if ENABLETRACE
GSSiExitProg (780);
#endif
	return;
}
#if ENABLETRACE
}
#endif
} 
}

void LoadDynamicText (int x,int y,double AZ,double size,LPSTR pText,int ltext)
{
	HANDLE	hStr;
	LPSTR	str; 
	DPOINT	WPoint;   
	POINT	ScreenPoint;
	double	sizew;
	
	if (!StoreDynText)
		return; 
	hStr = GSSiGlobAlloc ( 366,GMEM_MOVEABLE,1024);
	str = GlobalLock (hStr);  
	ScreenPoint.x = x;
	ScreenPoint.y = y;
	WPoint = WinPtToBasePt (ScreenPoint); 
	sizew = WinDistToWorldDist (size);
	pText[ltext]=0;
	sprintf (str,"%ld,%f,%f,%f,%f,%s",ShowValRef,WPoint.x,WPoint.y,AZ,sizew,pText);
	AppendFile ("[%DL]needarea\\textdump.txt",str);
	GSSiGlobUlFree (&hStr);
	return;
}


BOOL PtInTextRect (POINT pt)
#if ENABLETRACE
{GSSiEnterProg (782);
#endif
{ 
	RECT	*pRects;
	BOOL	rtn=TRUE;
	int		i;

	if (!hRects)
	{
		rtn = FALSE;
		goto Exit;
	}
	pRects=(RECT *)GlobalLock(hRects);
	for (i=0;i<NumRects;i++,pRects++)
	{
		if (PtInRect(pRects,pt))
			goto RtnTrue;
	}
	rtn = FALSE;
RtnTrue:
	GlobalUnlock (hRects);
Exit:
{
#if ENABLETRACE
GSSiExitProg (782);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
} 

BOOL AddTextRect(LPRECT Rect)
{
	return AddTextRect2 (Rect,FALSE);
}

BOOL AddTextRect2(LPRECT Rect,BOOL ForceAdd)
#if ENABLETRACE
{GSSiEnterProg (783);
#endif
{
	USHORT	i;
	HPRECT	pRects;
	RECT	NewRect; 
	BOOL	rtn=TRUE;

    if (!Rect)
    {
    	GSSiGlobFree (&hRects);
{
#if ENABLETRACE
GSSiExitProg (783);
#endif
    	return TRUE;
}
    }
	if (!hRects)
	{
		MaxRects=USHRT_MAX;
		hRects=GSSiGlobAlloc ( 370,GHND,((long)MaxRects)*sizeof(RECT));
		NumRects=0;
	}
	pRects=(LPRECT)GlobalLock(hRects);
	for (i=0;i<NumRects;i++)
	{
		if (IntersectRect (&NewRect,Rect,&pRects[i]))
		{
			rtn = FALSE;
			break;
		}
	}
	if ((rtn || ForceAdd) && NumRects<MaxRects)
		pRects[NumRects++] = *Rect;
	GlobalUnlock(hRects);
{
#if ENABLETRACE
GSSiExitProg (783);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
} 

void ProjectFilePtS (LPPOINTS pPoint)
{
	POINT p;
	
	p.x = pPoint->x;
	p.y = pPoint->y;

	ProjectFilePt (&p);
	pPoint->x = p.x;
	pPoint->y = p.y;
	return;
}

void ProjectFilePt (LPPOINT pPoint)
#if ENABLETRACE
{GSSiEnterProg (784);
#endif
{   
	DPOINT	DPoint;
	
	switch (CurView->FileProjectionType)
	{
		case 0:
{
#if ENABLETRACE
GSSiExitProg (784);
#endif
			return;
}
		case 1: /* lat long normalization */  
			if (CurView->LLNormFactor)
				pPoint->x = IDNINT (pPoint->x * CurView->LLNormFactor);
{
#if ENABLETRACE
GSSiExitProg (784);
#endif
			return;  
}
		case 2: //transformation
			DPoint = FilePtToBasePtNP (*pPoint);
			DPoint = TranPoint (&DPoint,hTranProjection);  
			*pPoint = BasePtToFilePtNP (DPoint);
{
#if ENABLETRACE
GSSiExitProg (784);
#endif
			return;
}
		case 3: /* alternate projection */
{
#if ENABLETRACE
GSSiExitProg (784);
#endif
			return;
}
	}
{
#if ENABLETRACE
GSSiExitProg (784);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

void UnProjectFilePt (LPPOINT pPoint)
#if ENABLETRACE
{GSSiEnterProg (786);
#endif
{   
	DPOINT	DPoint;
	
	switch (CurView->FileProjectionType)
	{
		case 0:
{
#if ENABLETRACE
GSSiExitProg (786);
#endif
			return;
}
		case 1: /* lat long normalization */  
			if (CurView->LLNormFactor)
				pPoint->x = IDNINT (pPoint->x / CurView->LLNormFactor);
{
#if ENABLETRACE
GSSiExitProg (786);
#endif
			return;
}
		case 2: //transformation
			DPoint = FilePtToBasePt (*pPoint);
			DPoint = TranPoint (&DPoint,hTranProjectionReverse);  
			*pPoint = BasePtToFilePt (DPoint);
{
#if ENABLETRACE
GSSiExitProg (786);
#endif
			return;
}
		case 3: /* alternate projection */
{
#if ENABLETRACE
GSSiExitProg (786);
#endif
			return;
}
	}
{
#if ENABLETRACE
GSSiExitProg (786);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

void UnProjectFilePtD (LPDPOINT pPoint)
#if ENABLETRACE
{GSSiEnterProg (786);
#endif
{   
	DPOINT	DPoint;
	
	switch (CurView->FileProjectionType)
	{
		case 0:
{
#if ENABLETRACE
GSSiExitProg (786);
#endif
			return;
}
		case 1: /* lat long normalization */  
			if (CurView->LLNormFactor)
				pPoint->x /= CurView->LLNormFactor;
{
#if ENABLETRACE
GSSiExitProg (786);
#endif
			return;
}
		case 2: //transformation
			DPoint = FilePtToBasePtD (*pPoint);
			DPoint = TranPoint (&DPoint,hTranProjectionReverse);  
			*pPoint = BasePtToFilePtD (DPoint);
{
#if ENABLETRACE
GSSiExitProg (786);
#endif
			return;
}
		case 3: /* alternate projection */
{
#if ENABLETRACE
GSSiExitProg (786);
#endif
			return;
}
	}
{
#if ENABLETRACE
GSSiExitProg (786);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

void ProjectFilePtD (LPDPOINT pPoint)
#if ENABLETRACE
{GSSiEnterProg (787);
#endif
{   
	DPOINT	DPoint;
	
	switch (CurView->FileProjectionType)
	{
		case 0:
{
#if ENABLETRACE
GSSiExitProg (787);
#endif
			return;
}
		case 1: /* lat long normalization */ 
			if (CurView->LLNormFactor)
				pPoint->x = pPoint->x * CurView->LLNormFactor;
{
#if ENABLETRACE
GSSiExitProg (787);
#endif
			return;
}
		case 2: //transformation
			DPoint = FilePtToBasePtNPD (*pPoint);
			DPoint = TranPoint (&DPoint,hTranProjection);  
			*pPoint = BasePtToFilePtNPD (DPoint);
{
#if ENABLETRACE
GSSiExitProg (787);
#endif
			return;
}
		case 3: /* alternate projection */  
//			DPoint = FilePtToBasePt (*pPoint);
//			ConvertCoord (&DPoint);
//			*pPoint = ProjectedPtToFilePt (DPoint);
{
#if ENABLETRACE
GSSiExitProg (787);
#endif
			return;
}
	}
{
#if ENABLETRACE
GSSiExitProg (787);
#endif
	return;
}
#if ENABLETRACE
}
#endif
} 

void SetLocalProjection (LPMNMXCORD pBounds)
#if ENABLETRACE
{GSSiEnterProg (788);
#endif
{
	double	d1,d2,f, MidX, MidY, Dist;
	DPOINT	p1,p2;                    
	
	CurView->LLNormFactor = 1;
	if (CurView->FileProjectionType != 1 || PRJ_TYPE[1] != 0) 
	{
		MetersPerDegree = 1;
{
#if ENABLETRACE
GSSiExitProg (788);
#endif
		return; 
}    
	} 
    MidX = (pBounds->xmn + pBounds->xmx) / 2;
    MidY = (pBounds->ymn + pBounds->ymx) / 2; 
    Dist = (pBounds->xmx - pBounds->xmn + pBounds->ymx - pBounds->ymn) / 2;
	p1.x = MidX - Dist;
	p1.y = MidY;
	p2.x = MidX + Dist;
	p2.y = MidY;
	d1=ArcDistance(p1,p2);
	p1.y = MidY - Dist;
	p1.x = MidX;
	p2.y = MidY + Dist;
	p2.x = MidX;
	d2=ArcDistance(p1,p2);  
	if (d2)
		CurView->LLNormFactor = d1/d2;
	else
		CurView->LLNormFactor = 1;
	if (Dist)
		MetersPerDegree = d2/(2.0 * Dist);
	if (LLNormFactor)
		CurView->LLNormFactor = LLNormFactor;   
	p1.x = pBounds->xmn;
	p2.x = pBounds->xmx;
	p1.y = p2.y = MidY;
	Dist = p2.x - p1.x;
	if (Dist)
		CurView->MetersPerDegreeX = ArcDistance (p1,p2) / Dist;
	else
		CurView->MetersPerDegreeX = 1;
{
#if ENABLETRACE
GSSiExitProg (788);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

void ProcessZoomMacroFile(HDC hDC)
#if ENABLETRACE
{GSSiEnterProg (789);
#endif
{   
	HFILE	Fid=HFILE_ERROR;  
	HANDLE	hMem = GSSiGlobAlloc ( 371,GMEM_MOVEABLE,512);
	LPSTR	str=GlobalLock (hMem);
	LPSTR	lpStr; 
	
	ZOOMLevelUsed = 0;
	if (!hDC)
		goto Exit;
	CurView->ZMScale = GetViewportScale (hDC); 
	if (ShowScale == CurView->ID)
	{
		sprintf (str,"%lf",CurView->ZMScale);
		SetWindowText (hWndMain,str);
	} 
	if (CurView->DisableZoomMacro && !AlwaysUseZoomMacro)
		goto Exit;
	lpStr=str;  
	_fstrcpy (str,CurView->VisName);
	ExpandText (str);
	_fstrupr (str);
	if (!_fstrstr (str,".TXT"))
		goto Exit;
	Fid=GSSiOpenFile(CurView->VisName,0,OF_READ);
	if (Fid==HFILE_ERROR)
		goto Exit;   
	ProcessZoomMacroFile2 (hDC,Fid);
	GSSiClose (Fid);
Exit: 
	GSSiGlobUlFree (&hMem);
{
#if ENABLETRACE
GSSiExitProg (789);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}


void ProcessZoomMacroFile2(HDC hDC,HFILE Fid)
#if ENABLETRACE
{GSSiEnterProg (789);
#endif
{   
	HANDLE	hMem = GSSiGlobAlloc ( 371,GMEM_MOVEABLE,1024);
	LPSTR	str=GlobalLock (hMem);
	LPSTR	lpSpace, lpC; 
	long	start, end; 
	int		nlines;  
	double	MacroScale, Factor;  
	BOOL	HaveALL, HaveEXIT,HaveZoomLevel=FALSE;  
	short	SaveCmdVP; 
	
	if (!hDC)
		goto Exit;
	if (!fgetstring (str,1020,Fid))
		goto Exit;  
	InZoomMacro = TRUE;
	if (!_fstricmp (str,"MILES"))
		Factor = 1.0;
	else
	{
		Factor = MFT;
		GSSillseek (Fid,0,0);
	}
		
Next:
	HaveALL = FALSE; 
	HaveEXIT = FALSE;
	start = GSSillseek(Fid,0,1);
	nlines=0;
	while (fgetstring(str,1020,Fid))
	{  
		switch (*str)
		{   
			case 0:
			case ' ':
			case '\t':
			case '#': 
				nlines++;
			break;
			
			default:  
				if (!_fstricmp ("ALL",str))
					HaveALL = TRUE; 
				else if (!_fstricmp ("EXIT",str))
					HaveEXIT = TRUE; 
				else
				{   
					MacroScale = atof (str) * Factor;
					if ((!HaveZoomLevel && (ZOOMLevelUsed == ZOOMLevelWanted || MacroScale > CurView->ZMScale)) || HaveALL)
					{
						HaveZoomLevel = TRUE; 
						goto GotMacro;       
					}
					ZOOMLevelUsed++;
				}
				start = GSSillseek(Fid,0,1);
				nlines = 0;
			break;
		}	 
	}
	if (!HaveZoomLevel)
	{
		HaveZoomLevel = TRUE;
		goto GotMacro;
	}
	if (!HaveEXIT)
		goto Exit;
GotMacro: 
	GSSillseek (Fid,start,0);
	SaveCmdVP = *pCommandViewport;
	*pCommandViewport = CurView->ID;
	while (nlines--)
	{
		fgetstring(str,1020,Fid); 
		if (!ExecuteCommandString (str))
		{
			*pCommandViewport = SaveCmdVP; 
			goto Exit;
		}
	}
	*pCommandViewport = SaveCmdVP; 
	if (HaveALL) 
	{
		HaveZoomLevel = FALSE; 
		goto Next; 
	}
	if (HaveEXIT)
		goto Exit;
	goto Next;
Exit:  
	InZoomMacro = FALSE;
	GSSiGlobUlFree (&hMem);
{
#if ENABLETRACE
GSSiExitProg (789);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

BOOL FAR PASCAL COLOR_ADJUSTMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (724);
#endif
{ 
	HWND	hScroll;
	int		ipos;
	LPVIEWPORT	SaveViewport;

 switch(Message)
   {
    case WM_INITDIALOG:
         cwCenter(hWndDlg, 0);
         hScroll = GetDlgItem (hWndDlg,IDC_RED_ADJUST);
         SetScrollRange (hScroll,SB_CTL,0,200,FALSE);
         SetScrollPos (hScroll,SB_CTL,*CARed+100,TRUE);
         hScroll = GetDlgItem (hWndDlg,IDC_GREEN_ADJUST);
         SetScrollRange (hScroll,SB_CTL,0,200,FALSE);
         SetScrollPos (hScroll,SB_CTL,*CAGreen+100,TRUE);
         hScroll = GetDlgItem (hWndDlg,IDC_BLUE_ADJUST);
         SetScrollRange (hScroll,SB_CTL,0,200,FALSE);
         SetScrollPos (hScroll,SB_CTL,*CABlue+100,TRUE);
         hScroll = GetDlgItem (hWndDlg,IDC_INTENSITY);
         SetScrollRange (hScroll,SB_CTL,0,200,FALSE);
         SetScrollPos (hScroll,SB_CTL,*CAIntensity+100,TRUE);

		 break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

	case WM_HSCROLL:
		hScroll = (HWND)lParam;
		ipos = GetScrollPos (hScroll,SB_CTL);
		switch (wParam)
		{
		  case SB_LINEDOWN:
		  ipos = min (200,ipos+10);
		  break;
			
		  case SB_LINEUP:
		  ipos = max (0,ipos-10);
	  	  break;
			
	  	  case SB_THUMBPOSITION:
	  	  ipos = LOWORD(lParam);
		  break;
		  
		  default:
{
#if ENABLETRACE
GSSiExitProg (724);
#endif
		  	return FALSE;
}
		}
		SetScrollPos (hScroll,SB_CTL,(int)ipos,TRUE);
	    break;
	         
    case WM_COMMAND:
         switch(LOWORD(wParam))
           {
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
                 EndDialog(hWndDlg, FALSE);
                 break;
            
            case IDC_RESET: 
				 SetScrollPos (GetDlgItem(hWndDlg,IDC_RED_ADJUST),SB_CTL,100,TRUE);
				 SetScrollPos (GetDlgItem(hWndDlg,IDC_GREEN_ADJUST),SB_CTL,100,TRUE);
				 SetScrollPos (GetDlgItem(hWndDlg,IDC_BLUE_ADJUST),SB_CTL,100,TRUE);
				 SetScrollPos (GetDlgItem(hWndDlg,IDC_INTENSITY),SB_CTL,100,TRUE);
            	 break;
            	 
            case IDC_APPLY:
            case IDOK: 
				CloseOrthos(TRUE);
            	 *CARed = GetScrollPos (GetDlgItem(hWndDlg,IDC_RED_ADJUST),SB_CTL) -100;
            	 *CAGreen = GetScrollPos (GetDlgItem(hWndDlg,IDC_GREEN_ADJUST),SB_CTL) -100;
            	 *CABlue = GetScrollPos (GetDlgItem(hWndDlg,IDC_BLUE_ADJUST),SB_CTL) -100;
            	 *CAIntensity = GetScrollPos (GetDlgItem(hWndDlg,IDC_INTENSITY),SB_CTL) -100;
            	 SaveViewport=CurView;
				 SetGlobalValueLong ("%ORTHORED",*CARed);  	
				 SetGlobalValueLong ("%ORTHOGREEN",*CAGreen);  	
				 SetGlobalValueLong ("%ORTHOBLUE",*CABlue);  	
				 SetGlobalValueLong ("%ORTHOINTENSITY",*CAIntensity);  	
			     PostMessage(hWndMain, WM_COMMAND, IDM_Z_REDRAW, 0L);
            	 if (wParam == IDOK)
	                EndDialog(hWndDlg, TRUE); 
                 SetCurView (SaveViewport);
                 break;    
		}
		break;

    default:
{
#if ENABLETRACE
GSSiExitProg (724);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (724);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
}

int AddPuertoRicoToCities4x (int i)
{
	CITIESDATA4	cd;
	char	str[256];
	HFILE Cities4FID = GSSiOpenFile ("cities4.dat",0,OF_CREATE);
//	HFILE Fid = GSSiOpenFile ("prcities.txt",0,OF_READ);
	HFILE Fid = GSSiOpenFile ("prcities_sorted.txt",0,OF_READ);
	if (Cities4FID == HFILE_ERROR)
		return 0;
/*	while (BigRead (Cities4FID,&cd,sizeof(cd))==sizeof(cd))
	{
		sprintf (str,"%i\t%s\t%f\t%f",-cd.Pop,cd.Name,cd.LatLong.x,cd.LatLong.y);
		fputstring (str,FidOut);
	}
	GSSiClose (FidOut);
	GSSillseek (Cities4FID,0,2);*/
	fgetstring (str,250,Fid);
	while (fgetstring (str,250,Fid))
	{
		LPSTR pLoc=strchr (str,'\t');
		LPSTR pLoc2;

		*pLoc++ = 0;
		cd.Pop = -atoi (str);
		pLoc2 = strchr (pLoc,'\t');
		*pLoc2++ = 0;
		strcpy (cd.Name,pLoc);
		cd.LatLong.x = atof (pLoc2);
		pLoc = strchr (pLoc2,'\t');
		*pLoc++ = 0;
		cd.LatLong.y = atof (pLoc);
		strcpy (cd.State,"");
		BigWrite (Cities4FID,&cd,sizeof(cd),-1);
	}
	GSSiClose (Fid);
	GSSiClose (Cities4FID);

	return 0;
}



