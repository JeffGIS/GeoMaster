#include "graphint.h"   
#include "extrndb.h"

#include "gmextern.h"

static double	ShrinkFactor=1;


BOOL CreateInfoBoxGMD (LPSTR Name)
{
	return TRUE;
}

DPOINT	ConvertTAGDPoint (DPOINT FromPt,short FromCoordStyle, short ToCoordStyle)
{
	short	SaveStyle = TAGBox.CoordStyle; 
	DPOINT	ToPt;
	POINT	WinPt;
	
	TAGBox.CoordStyle = FromCoordStyle;
	WinPt = TAGPtToWinPt (FromPt);
	TAGBox.CoordStyle = ToCoordStyle;
	ToPt = WinPtToTAGPt (WinPt);
	TAGBox.CoordStyle = SaveStyle;
	return ToPt;
}
	
void ClearTAG (HDC hDC)
{
	FillRectPoly (hDC,&TAGBox.rect,WindowColor);
	return;
}

void InfoBoxInit (LPTAGBOX pTAGBox)
{
	_fmemset (pTAGBox,0,sizeof(TAGBOX));          
	pTAGBox->Version = 1;
	pTAGBox->Flags.AutoEdit = TRUE;
	pTAGBox->TXheight = 0.05;                  
    pTAGBox->LogFont.lfHeight = 10;
    pTAGBox->LogFont.lfWidth = 0;
    pTAGBox->LogFont.lfEscapement = 0;
    pTAGBox->LogFont.lfOrientation = 0;
    pTAGBox->LogFont.lfWeight = 400;
    pTAGBox->LogFont.lfItalic = 0;
    pTAGBox->LogFont.lfUnderline = 0;
    pTAGBox->LogFont.lfStrikeOut = 0;
    pTAGBox->LogFont.lfCharSet = ANSI_CHARSET;
    pTAGBox->LogFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
    pTAGBox->LogFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    pTAGBox->LogFont.lfQuality = DEFAULT_QUALITY;
    pTAGBox->LogFont.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
    _fstrcpy(pTAGBox->LogFont.lfFaceName, "Arial Rounded MT Bold");
    _fstrcpy(pTAGBox->LogFont.lfFaceName, "Arial");
    pTAGBox->BGcolor = RGB (255,255,255);
	pTAGBox->TXcolor = RGB (0,0,0);
	pTAGBox->BorderColor = RGB(255,255,255); 
	pTAGBox->PointerColor = RGB(0,0,0); 
	pTAGBox->BorderStyle = 0; 
	pTAGBox->Margin = 5;
	pTAGBox->BGstyle = 0;
	pTAGBox->CoordStyle = 1;
 	pTAGBox->PLstyle=0;
	pTAGBox->PLwidth=1;
	pTAGBox->Just=1; 
	pTAGBox->hTAGDB=0;
	pTAGBox->DataFileType=0;
    pTAGBox->DataFile[0]='\0';
    pTAGBox->text[0]='\0';
	return;
}
int GetTextWH (HDC hDC,LPSTR str,LPSIZE pSize,float f)
{
	LPSTR	pLoc;
	SIZE	size;

	pSize->cx = 0;
	pSize->cy = 0;
	while ((pLoc = strchr (str,'\r')))
	{
		*pLoc = 0;
		GetTextExtentPoint32(hDC,str,strlen(str),&size);
		*pLoc++ = '\r';
		pSize->cx = max (pSize->cx,size.cx);
		pSize->cy += size.cy;
		str = pLoc;
	}
	GetTextExtentPoint32(hDC,str,strlen(str),&size);
	pSize->cx = max (pSize->cx,size.cx);
	pSize->cy += size.cy;

	return pSize->cx+pSize->cy*f;
}

void BlockText (HDC hDC,LPSTR str,int maxlen,float f)
{
	HANDLE	hMem=GSSiGlobAlloc (0,GMEM_MOVEABLE,maxlen+1024);
	LPSTR	newstr = GlobalLock (hMem);
	SIZE	txSize;
	int		perim, minperim=INT_MAX;
	int		numspace=0;
	int		maxcomb=1, test;
	BYTE		i,j,mini=0;
#define	MAXSPACE	4
	LPSTR	SpacePos[MAXSPACE];
	LPSTR	pos=newstr;
	char	spacechar;

	strcpy (newstr,str);
	OneSpace (newstr);
	spacechar = ' ';
	if (strchr (newstr,'\r'))
		spacechar = '\r';
	while (numspace < MAXSPACE && (SpacePos[numspace] = strchr (pos,spacechar)))
		pos = SpacePos[numspace++]+1;
	if (numspace)
	{
		for (i=0;i<numspace;i++)
			maxcomb *= 2;
		for (i=0;i<maxcomb;i++)
		{
			for (j=0;j<numspace;j++)
			{
				if (GetBit (7-j,&i))
					*SpacePos[j] = '\r';
				else
					*SpacePos[j] = ' ';
			}
			perim = GetTextWH (hDC,newstr,&txSize,f);
			if (perim < minperim)
			{
				minperim = perim;
				mini = i;
			}
		}
		for (j=0;j<numspace;j++)
		{
			if (GetBit (7-j,&mini))
				*SpacePos[j] = '\r';
			else
				*SpacePos[j] = ' ';
		}
		strcpy (str,newstr);
	}
	GSSiGlobUlFree (&hMem);
	return;
}

LPHANDLE YellowTextBox (HWND hWnd, LPSTR instr, POINT WinPoint,LPRECT pRect,LPRECT DisplayInRect,BOOL Transparent,int Style)
{   
	DPOINT	TagPoint;
	HDC hDC=GetDC (hWnd);
	TAGBOX	SaveTAGBox=TAGBox; 
	HANDLE	rtn=0;  
	RECT	WRect; 
	short	ii;
	HANDLE	hStr=GSSiGlobAlloc (0,GMEM_MOVEABLE,4096);
	LPSTR	str=GlobalLock (hStr);
	BOOL	DoNotMove = FALSE;

	if (DisplayInRect == (LPRECT)1)
	{
		DoNotMove = TRUE;
		DisplayInRect = 0;
	}
	strcpy (str,instr);
	if (!strchr (str,'$'))
		ExpandText (str);
	
	if (hWndBasic)
		ii=1;
	_fmemset (&TAGBox,0,sizeof(TAGBOX));          
	TAGBox.Version = 1;
	TAGBox.TXheight = GetGlobalDVal2 ("[%INFOTEXTHEIGHT]",0.016);                  
    _fstrcpy(TAGBox.LogFont.lfFaceName, "Arial");
    TAGBox.BGcolor = TAGBox.PointerColor = TAGBox.PointerColor = GetGlobalLVal2 ("[%INFOBOXCOLOR]",RGB (255,255,0));
	TAGBox.TXcolor = RGB (0,0,0);
	TAGBox.BorderColor = GetGlobalLVal2 ("[%INFOBOXBORDERCOLOR]",RGB(255,255,0)); 
	TAGBox.Margin = 15;
	TAGBox.PLstyle = 3;
	TAGBox.PLwidth = 2;
	TAGBox.Shape = Style;
	TAGBox.BorderStyle =1;//2;
	TAGBox.Shadow = FALSE;
	if (Transparent && GetGlobalBVal2 ("[%TRANSPARENTIDBOX]",FALSE))
		TAGBox.BGstyle = 2;
	else
		TAGBox.BGstyle = 1;
	TAGBox.Just=2; 
    _fstrcpy (TAGBox.text,str); 
	TAGBox.CoordStyle = 2;
	TAGBox.TAGPoint = TagPoint = WinPtToTAGPt (WinPoint);
	TAGBox.center = TagPoint;
	if (Style)
	{
		TAGBox.CoordStyle = 4;
		TAGBox.TAGPoint = TagPoint = WinPtToTAGPt (WinPoint);
		TAGBox.center = TagPoint;
		TAGBox.Margin = 25;
		BlockText (hDC,TAGBox.text,sizeof(TAGBox.text),1.5);
		TAGBox.center.y+=(0.5-TAGBox.center.y)*0.12;
		TAGBox.center.x+=(0.5-TAGBox.center.x)*0.1;
		TAGBox.PLwidth = 3;
	}

	if (ResetTAGBox (hDC,1)) 
	{
		if (DisplayInRect)
			TAGBox.rect = *DisplayInRect;
		else if (!DoNotMove)
		{
			GetClientRect (hWnd,&WRect);
			if (TAGBox.rect.left < WRect.left)
			{
				TAGBox.rect.right += WRect.left - TAGBox.rect.left;
				TAGBox.rect.left = WRect.left;
			} 
			if (TAGBox.rect.right > WRect.right)
			{
				TAGBox.rect.left -= TAGBox.rect.right - WRect.right;
				TAGBox.rect.right = WRect.right;
			} 
			if (TAGBox.rect.top < WRect.top)
			{
				TAGBox.rect.bottom += WRect.top - TAGBox.rect.top;
				TAGBox.rect.top = WRect.top;
			} 
			if (TAGBox.rect.bottom > WRect.bottom)
			{
				TAGBox.rect.top -= TAGBox.rect.bottom - WRect.bottom;
				TAGBox.rect.bottom = WRect.bottom;
			}
		}
       	TAGBox.center = WinPtToTAGPt (RectMid(&TAGBox.rect));  
       	if (pRect)
       	{
			DrawTAG(hWnd,hDC,TRUE,TRUE); 
			*pRect = TAGBox.rect;  
			DestroySavedScreen (&TAGBox.before,0);
		}
		else  
			DrawTAG(hWnd,hDC,2,TRUE);  
    }
    ReleaseDC (hWnd,hDC);      
    rtn = TAGBox.before;
	UnloadReport (&TAGBox.hReport); 
    TAGBox = SaveTAGBox;
	GSSiGlobUlFree (&hStr);
	return rtn;
}

 

BOOL CreateTAGBoxFromFile (HDC hDC,DPOINT BasePoint,LPSTR File,int item,double MoveFactor)
{
    BOOL	nRc; 
    
    if (!LoadInfoBox (File,&TAGBox))
    	return FALSE; 
    if (item == -2)
    	TAGBox.Flags.UseOrigSize = TRUE;
	TAGBox.ViewportID = CurView->ID;
    TAGBox.BlowUpBounds = BlowUpRec.FromBounds;
    if (TAGBox.Flags.UseOrigSize)
    {
    	TAGBox.rect = BlowUpRec.ToWinRect;
    	TAGBox.bmWidth = TAGBox.rect.right - TAGBox.rect.left + 1;
    	TAGBox.bmHeight = TAGBox.rect.bottom - TAGBox.rect.top + 1;
    	BasePoint = MinMaxMidPointD (&TAGBox.BlowUpBounds);
    }  
    TAGBox.TAGPoint = TAGBox.center = BasePoint; 
    TAGBox.TXheight = -fabs(TAGBox.TXheight);
	if (!CreateTAGBox (hDC,BasePoint,item))
		return FALSE;
	if (MoveFactor)
	{
		double	MoveDistScreen = min (TAGBox.bmWidth,TAGBox.bmHeight) * MoveFactor;
		DPOINT	VPMidPoint = RectMidD (&CurView->ScreenRect);
		DPOINT	TAGMidPointScreen = BasePtToScreenPtD (&BasePoint);
		double	AZ = getazd (&TAGMidPointScreen,&VPMidPoint);
		DPOINT	MidPointScreen;
		double	MinDiff=1000;
		int		i, mini=0;
		double	UseAZ[4]={HALFPI/2,3*HALFPI/2,5*HALFPI/2,7*HALFPI/2};

		MinDiff = fabs (AZ - UseAZ[0]);
		for (i=1;i<4;i++)
		{
			double d = fabs (AZ - UseAZ[i]);

			if (d < MinDiff)
			{
				MinDiff = d;
				mini = i;
			}
		}
		MidPointScreen = dnewpt (TAGMidPointScreen,UseAZ[mini],MoveDistScreen);
		TAGBox.center = ScreenPtDToBasePt (MidPointScreen);
	}
	if (TAGBox.Flags.AutoEdit)
	{
		DLGPROC lpfnTAGEDITMsgProc;
						
		setDoPaint( FALSE);
		lpfnTAGEDITMsgProc = MakeProcInstance((DLGPROC)TAGEDITMsgProc, hInst);
		nRc = DialogBox(hInst, (LPSTR)"TAGEDIT", hWndMain, lpfnTAGEDITMsgProc);
		FreeProcInstance(lpfnTAGEDITMsgProc);
	} 
	else
		nRc = 1;
  	if (nRc)
  	{
		 SetDisplayMode (CurView->hDC, GF_MAPMODE);
		 if (!ResetTAGBox (hDC,2))
		 	return FALSE;
		 if (TAGBox.Flags.AutoMove) 
		 {
       	 	DrawTAG(CurView->hWnd, hDC,2,FALSE);  
		 	DoSave=TRUE;  
		 	TBNum = 0;
	        PostMessage(hWndMain, GSSI_ADDGF, GF_MOVE_TAG, CurView->ID); 
	     }
       	 else
       	 {  
       	 	BOOL	SaveDDC = DoDisplayConfigs;
			DoDisplayConfigs = TRUE;
        	SaveTAG(0);
       	 	DrawTAG(CurView->hWnd, hDC,FALSE,FALSE);
			SaveFullWindowBitmap (hWndMain);
       	 	DoDisplayConfigs = SaveDDC;
		 } 
		 return TRUE; 
	}
	return FALSE;
}

BOOL SelectTAGTemplate (void)
{
	return TRUE;
}

BOOL CreateTAGBox (HDC hDC, DPOINT TagPoint, int item)
{   
	POINT	WinPoint;  
	double	dist, azm;
	
	AssignTAGInstance (0); 
	TAGBox.TAGPointScr = BasePtToWinPt (&TagPoint);
	switch (TAGBox.CoordStyle)
	{
		case 0:
			dist=ldistp (TAGBox.TAGPoint,TAGBox.center);
			if (dist)
			{
				azm = getazd (&TAGBox.TAGPoint,&TAGBox.center); 
				TAGBox.center = dnewpt (TagPoint,azm,dist);
			}
			else 
				TAGBox.center = TagPoint;
			TAGBox.TAGPoint = TagPoint;
		break;
		
		default: 
			if (TAGBox.CoordStyle == 1)
				TAGBox.TAGPoint = WinPtToTAGPt (TAGBox.TAGPointScr);
			else
				TAGBox.TAGPoint = WinPtToTAGPt (TAGBox.TAGPointScr);
		    if (TAGBox.Flags.UseOrigSize)
		    {
		    	POINT	IBCenterWin = RectMid (&TAGBox.rect); 
				TAGBox.center = WinPtToTAGPt (IBCenterWin);    
		    }
		    else
				TAGBox.center = WinPtToTAGPt (TAGBox.TAGPointScr);    
		break;
	}
	if (item >= 0)
	{
		TAGBox.Refno = PickList[item].Refno;
		_fstrcpy (TAGBox.Prefix,PickList[item].Prefix);
		_fstrcpy (TAGBox.UDI,PickList[item].UDI);  
		SetPickGlobals (item);
		ExpandText (TAGBox.SQL);
	}
	if (!ResetTAGBox (hDC,1))
		return FALSE;
	TAGBox.before = 0;
	TAGBox.ViewportID = CurView->ID;
	UnloadReport (&TAGBox.hReport);
	return TRUE;
}

BOOL ResetTAGBox (HDC hDC,short From)
{   DWORD	TextExt;
	int		TextWidth;
	HFONT	OldFont, TagFont;
	LPSTR	lpText, lpReport, lpEndReport;
	int		iLogPixsY, MaxTextWidth=0, BorderAdjust=0;
	float	Pct; 
	POINT	ScreenPoint; 
	RECT	ReportRect;
	LPOPENFILEDATA	FilePtr;
	LPOPENSQLDATA	SQLPtr;
	LPFIELDINFO	lpFieldInfo; 
	HANDLE		SaveHandle;
	RECT	Rect;   
	HANDLE	hMEM=GSSiGlobAlloc ( 381,GMEM_MOVEABLE,1024+1024+4096+256);
	LPSTR	ExpLine, Line, ExpandArea, ReportName,str;
	BOOL	rtn=TRUE;
	double	Factor, IBFactor;
	SIZE	txSize;
	
	SaveDC (hDC);
	Line = GlobalLock (hMEM);
	if (TAGBox.ViewportID > 0 && !VPIsActive (TAGBox.ViewportID))
		goto Exit;
	ExpLine = Line + 1024;
	ExpandArea = ExpLine + 1024;  
	ReportName = ExpandArea + 4096;
	str = ReportName + 128;
	
	InInfoBox=TRUE;	
	if (TAGBox.CoordStyle == 0 && TAGBox.PLstyle && From > 1)
	{
		if (!PtInWBounds (&TAGBox.TAGPoint))
		{
			rtn = FALSE;
			goto Exit;
		}
		CurrentPoint = TAGBox.TAGPoint;
	}
	else if (TAGBox.CoordStyle == 1 && TAGBox.PLstyle && From > 1)
	{
		TAGBox.TAGPointScr = TAGPtToWinPt (TAGBox.TAGPoint);
		if (!PtInRect (&CurView->ScreenRect,TAGBox.TAGPointScr))
		{
			rtn = FALSE;
			goto Exit;
		}
		CurrentPoint = TAGBox.TAGPoint;
	}
	if (TAGBox.CoordStyle == 2) 
		Rect = MainRect;
	else if (TAGBox.CoordStyle == 3) 
		Rect = MainRect;
	else
        Rect = CurView->Rect;
    
    if (InExpandIB)
	    ShrinkFactor = 100.0;
    else
	    ShrinkFactor = 1.0;
    
    TAGBox.hTAGDB=0;
    if (TAGBox.DataFile[0])
    {   
    	if (_fstrstr (TAGBox.DataFile,"PICKDATA.GMD"))
    	{   
    		short SaveMaxPick = MaxPick;

			UseUserPickAp =FALSE;
			SystemPickAp = 0;	
			MaxPick=1;  
            PickItems (CurView->hWnd,TAGBox.TAGPoint);  
			UseUserPickAp =TRUE;  
			MaxPick = SaveMaxPick;
        }    
		OpenDataFile (TAGBox.DataFile,TAGBox.SQL,BT_READ,&TAGBox.hTAGDB);
	}
    
	CurrentRefno=TAGBox.Refno;
	ltoa (CurrentRefno,str,10);
	SetGlobalValue ("%INT_REFNO",str); 
	SetGlobalValue ("%PREFIX",TAGBox.Prefix);
	SetUDIValue (TAGBox.Prefix,TAGBox.UDI); 
	strncpy0 (CurrentPrefix,TAGBox.Prefix,MAX_PREFIX_LEN);
	strncpy0 (CurrentUDI,TAGBox.UDI,MAX_UDI_LEN);
	_fstrcpy (ExpandArea,TAGBox.text);
	if (strnicmp (ExpandArea,"$REPORT(",8))
		ExpandText (ExpandArea);
    if (TAGBox.Flags.ExpandText && From == 1)
    {   
    	_fstrncpy (TAGBox.text,ExpandArea,sizeof(TAGBox.text));
       	CloseDataFile(TRUE,&TAGBox.hTAGDB); 
       	*TAGBox.DataFile = 0;
		TAGBox.Flags.ExpandText = FALSE;
	}
	SetDisplayMode (hDC, GF_SCREENMODE);
    /*iLogPixsY = GetDeviceCaps(hDC, LOGPIXELSY);
    TAGBox.LogFont.lfHeight = -1 * (iLogPixsY * TAGBox.TXheight / 72); */
    if (TAGBox.Factor)  
    	Factor = TAGBox.Factor;
    else
    	Factor = 1;  
    if ((IBFactor = GetGlobalDVal2 ("[%INFOBOXFACTOR]",1)) != 1)
    	TAGBox.bmWidthD = TAGBox.bmHeightD = 0;
    Factor *= IBFactor;
    if (TAGBox.TXheight > 0 || Printing)
    	TAGBox.LogFont.lfHeight = -labs(IDNINT(TAGFontHtToPixels (TAGBox.TXheight*Factor,TAGBox.CoordStyle)));
    else
		TAGBox.TXheight = fabs (PixelsToTAGFontHt(&TAGBox.LogFont,TAGBox.CoordStyle));
	TAGBox.LogFont.lfOrientation = TAGBox.LogFont.lfEscapement;
    TagFont = CreateFontIndirect((LPLOGFONT)&TAGBox.LogFont);
    OldFont = SelectObject(hDC, TagFont);
    lpText = ExpandArea;
    TAGBox.bmWidth = TAGBox.bmHeight =0;  
    if (!TAGBox.Flags.UseOrigSize)
    while (NextLine (&lpText,Line,0))
    {    
	    if (_fstrnicmp(Line,"$REPORT(",8))
		{
    		_fstrcpy(ExpLine,Line);
    		ExpandText (ExpLine);
			Truncate(ExpLine);
			if (!ExpLine[0])
			{
        		ExpLine[0]=' ';
        		ExpLine[1]='\0';
			}
		}
	    if (!_fstrnicmp(Line,"$REPORT(",8))
	    {   
	    	if (!TAGBox.hReport)
	    	{   
	    		lpReport = Line + 8;
	    		lpEndReport = _fstrchr(lpReport,')');
	    		if (lpEndReport)
	    		{                                                    
	    			*lpEndReport = '\0';
		    		_fstrcpy (ReportName,lpReport); 
		    		*lpEndReport = ')';
					TAGBox.hReport = LoadReport (ReportName); 
					if (!TAGBox.hReport)
					{
						 MessageBox(GetFocus(),ReportName,
										"Unable to open report file",MB_OK|MB_ICONEXCLAMATION);
						goto Exit;
					}
				}
			} 
			DisplayReport (hDC,TAGBox.hReport,Rect,Rect, Factor*DeviceToScreenFactor(), TAGBox.Refno,&ReportRect);  
//			ReportRect = SizeReport (hDC,TAGBox.hReport,TAGBox.rect,TRUE);
			TAGBox.bmWidth = max (TAGBox.bmWidth,ReportRect.right - ReportRect.left + 1);
			TAGBox.bmHeight += ReportRect.bottom - ReportRect.top + 1;
	    }
	    else if (!_fstrnicmp(Line,"$BITMAP(",8))
	    {   
			int	height,width,nLines=0; 
			LPSTR	lpComma;
			double	factor;
			LPSTR	pPar;
	
		 	lpReport = Line+8;
			lpEndReport = MatchLev (lpReport,')');
			if (lpEndReport)
			{                                                    
				*lpEndReport = 0;
				if ((lpComma=MatchLev (lpReport,',')))
				{
					*lpComma++=0;
					nLines = atoi (lpComma);
				}
	    		_fstrcpy (ExpLine,(LPSTR)(Line+8));
	    		*lpEndReport = ')';
				ExpandText (ExpLine);
				if ((pPar=strrchr (ExpLine,'(')))
				{
					float	fwidth, fheight;

					*pPar++=0;
					lpComma = strchr (pPar,',');
					*lpComma = 0;
					fwidth = atof (pPar);
					fheight = atof (lpComma+1);
					height = fheight / CurView->BaseUnitsPerPixel;
					width  = fwidth / CurView->BaseUnitsPerPixel;
				}
				else
				{
					if (!GetDIBDimensions(ExpLine,&height,&width))
						goto TextOut;
					height *= DeviceToScreenFactor();
					width  *= DeviceToScreenFactor();
				}
				height *= Factor;
				width  *= Factor;
				if (nLines)
				{   
					factor = (double)width / height;
					height = abs (TAGBox.LogFont.lfHeight) * nLines;
					width = factor * height;
				}
				TAGBox.bmWidth = max(TAGBox.bmWidth,width);
				TAGBox.bmHeight += height;
				TAGBox.bmHeight += abs(TAGBox.LogFont.lfHeight)/3;  
			}
	    }
	    else
	    {
	TextOut:
			GetTextExtentPoint32 (hDC,ExpLine,_fstrlen(ExpLine),&txSize);
			MaxTextWidth = max (MaxTextWidth,txSize.cx);
			TAGBox.bmHeight += txSize.cy;  
		}
	} 
	Pct = (float)TAGBox.Margin/100.0 + (float) TAGBox.BorderStyle/350.0;
	if (TAGBox.BorderStyle == 4)
		Pct += (float) TAGBox.BorderStyle/350.0; 
	TAGBox.incx = max(MaxTextWidth,TAGBox.bmWidth) * Pct;
	TAGBox.incy = TAGBox.bmHeight * Pct;
	MaxTextWidth += 2 * TAGBox.incx;
	TAGBox.bmWidth = max(MaxTextWidth,TAGBox.bmWidth+TAGBox.incx * 2);
	TAGBox.bmHeight += TAGBox.incy * 2;  
	if (TAGBox.BorderStyle==4)
	{
		double	width = (3)*DeviceToScreenFactor()*ShrinkFactor;
		int inc = width + BorderAdjust + TAGBox.incx;
		TAGBox.bmWidth += inc * 2;
		TAGBox.bmHeight += inc * 2;  
	}
	if (TAGBox.bmWidthD > 0 /*&& TAGBox.Flags.FixedSize*/)
	{   double	NewWidth;
		NewWidth = TAGBox.bmWidthD * (Rect.right - Rect.left);  
		if (NewWidth < TAGBox.bmWidth || InExpandIB)
			ShrinkFactor = min (ShrinkFactor,NewWidth/(double)TAGBox.bmWidth);
		else
			TAGBox.bmWidth = NewWidth;
	}
	else if (TAGBox.CoordStyle == 1)
		TAGBox.bmWidthD = (double)TAGBox.bmWidth / (Rect.right - Rect.left);
	if ((TAGBox.bmHeightD > 0/* && TAGBox.Flags.FixedSize*/) || InExpandIB)
	{	double NewHeight;
	
	    NewHeight = TAGBox.bmHeightD * (Rect.bottom - Rect.top);
	    if (NewHeight < TAGBox.bmHeight || InExpandIB)
			ShrinkFactor = min (ShrinkFactor,NewHeight/(double)TAGBox.bmHeight);
		else
			TAGBox.bmHeight = NewHeight;
	}
	else if (TAGBox.CoordStyle == 1)
		TAGBox.bmHeightD = (double)TAGBox.bmHeight / (Rect.bottom - Rect.top);
	if (TAGBox.BorderStyle==4)
		BorderAdjust = ShrinkFactor * DeviceToScreenFactor();
	TAGBox.bmHeight += 2 * BorderAdjust;  
	TAGBox.bmWidth  += 2 * BorderAdjust;
	TAGBox.bmHeight = IDNINT (TAGBox.bmHeight * ShrinkFactor);  
	TAGBox.bmWidth = IDNINT (TAGBox.bmWidth * ShrinkFactor);
	TAGBox.incx *= ShrinkFactor;
	TAGBox.incy *= ShrinkFactor;
    SelectObject(hDC, OldFont);
    DeleteObject(TagFont);
	TAGBox.before=0;
	TAGBox.after=0;
	ScreenPoint = TAGPtToWinPt (TAGBox.center);
	if (TAGBox.CoordStyle == 2 || TAGBox.CoordStyle == 4)
		TAGBox.TAGPointScr = TAGPtToWinPt (TAGBox.TAGPoint);
	else
		TAGBox.TAGPointScr = BasePtToWinPt (&TAGBox.TAGPoint);
	//TAGBox.TAGPointScr = BasePtToWinPt (&TAGBox.TAGPoint); 
    if (TAGBox.Flags.UseOrigSize)
    {
    	TAGBox.bmWidth = TAGBox.rect.right - TAGBox.rect.left + 1;
    	TAGBox.bmHeight = TAGBox.rect.bottom - TAGBox.rect.top + 1;
    }  
	else
	{
		TAGBox.rect.left = ScreenPoint.x - TAGBox.bmWidth/2;
		TAGBox.rect.right = TAGBox.rect.left + TAGBox.bmWidth;
		TAGBox.rect.top = ScreenPoint.y - TAGBox.bmHeight/2;
		TAGBox.rect.bottom = TAGBox.rect.top + TAGBox.bmHeight; 
	}
	if (TAGBox.hTAGDB)
	{
    	CloseDataFile(TRUE,&TAGBox.hTAGDB);
    }  
Exit:
	RestoreDC (hDC,-1);
	GSSiGlobUlFree (&hMEM);
    InInfoBox=FALSE;
	return rtn;
}

void DrawTAG (HWND hWnd, HDC hDC, BOOL MoveMode, BOOL Restore)
{   HPEN	hTBPen=0, hTXPen=0, hOldPen=0, OldPen,BorderPen=0;
	HBRUSH	hTBBrush=0, hOldBrush=0;    
	HRGN	hRgn;
	POINT	Points[9];
	LONG	Dist, MinDist;
    HDC     hdcMem;
	POINT	ScreenPoint, SegPoint;
	static	BOOL	HavePL;
	int		i, OldMode=GetROP2 (hDC), LineWidth=1;
	HFONT	TagFont, OldFont;
	long	OldTextColor=-1;
	LPSTR	lpText;
	int		Twidth, Theight, x, y, xt;
	double	yr,theightreal,hfactor,BorderAdjust=0;
	DWORD	TextExt;
	RECT	Rect, SaveRect;
	int		iLogPixsY;
	double	width;
	LOGBRUSH	NDB;
	HBITMAP	hBM=0;
	int		OldBKMode=0,ii,RegionType;
	LPSTR	ExpLine, Line, ExpandArea, str;
	LPSTR	lpReport, lpEndReport;
	LPOPENFILEDATA	FilePtr;
	LPOPENSQLDATA	SQLPtr;
	LPFIELDINFO	lpFieldInfo; 
	HANDLE		SaveHandle, hMEM=GSSiGlobAlloc ( 382,GMEM_MOVEABLE,1024+1024+4096+256);
	BOOL	Save=FALSE, InMoveMode = MoveMode;   
	double	Factor, IBFactor;
	SIZE	txSize;
	int	Elwh;
	HANDLE	hPointer;
	double	RoundingFactors[5]={2000,20,10,5,1};
	double	RoundingFactor;
	LPRECT	pRect=0;

	RoundingFactor = RoundingFactors[TAGBox.Shape]/2;

	
	if (TAGBox.ViewportID > 0)
	{
		if (!VPIsActive (TAGBox.ViewportID))
			return;
		SetViewport (TAGBox.ViewportID);
		if (CurView->DisplayInParent && CurView->Parent)            	
			 SetViewport(CurView->Parent);  
	} 
	SaveDC (hDC);
	SetDisplayMode (CurView->hDC, GF_SCREENMODE); 
	Line = GlobalLock (hMEM);
	ExpLine = Line + 1024;
	ExpandArea = ExpLine + 1024;
	str = ExpandArea + 4096;
	InInfoBox=TRUE;   
	InDrawTAG = TRUE;
    if (MoveMode == 2)
    {
    	MoveMode = FALSE;
    	Save = TRUE;
    }
    TAGBox.hTAGDB=0;
    if (TAGBox.DataFile[0])
    {
    	if (_fstrstr (TAGBox.DataFile,"PICKDATA.GMD"))
    	{   
    		short SaveMaxPick = MaxPick;

			UseUserPickAp =FALSE;
			SystemPickAp = 0;	
			MaxPick=1;  
            PickItems (CurView->hWnd,TAGBox.TAGPoint);  
			UseUserPickAp =TRUE;  
			MaxPick = SaveMaxPick;
        }    
		OpenDataFile (TAGBox.DataFile,TAGBox.SQL,BT_READ,&TAGBox.hTAGDB);
	}
    
	CurrentRefno=TAGBox.Refno;
	ltoa (CurrentRefno,str,10);
	SetGlobalValue ("%INT_REFNO",str); 
	SetGlobalValue ("%PREFIX",TAGBox.Prefix);
	SetUDIValue (TAGBox.Prefix,TAGBox.UDI);
	switch (TAGBox.CoordStyle)
	{
		case 0:
        	hRgn = CreateVPRgn (FALSE,FALSE);
			pRect = 0;
        	break;
		case 2:
        case 4:
			hRgn = CreateRectRgnIndirect (&ClientRect);
			pRect = &ClientRect;
			break;	
        default:
			hRgn = CreateRectRgnIndirect (&MainRect);	
			pRect = &ClientRect;
			break;
	}
    RegionType = SelectClipRgn (hDC,hRgn);
	ScreenPoint = TAGPtToWinPt (TAGBox.center);
	if (TAGBox.CoordStyle == 2 || TAGBox.CoordStyle == 4)
		TAGBox.TAGPointScr = TAGPtToWinPt (TAGBox.TAGPoint);
	else
		TAGBox.TAGPointScr = BasePtToWinPt (&TAGBox.TAGPoint);
	TAGBox.rect.left = ScreenPoint.x - TAGBox.bmWidth/2;
	TAGBox.rect.right = TAGBox.rect.left + TAGBox.bmWidth;
	TAGBox.rect.top = ScreenPoint.y - TAGBox.bmHeight/2;
	TAGBox.rect.bottom = TAGBox.rect.top + TAGBox.bmHeight;
	if (TAGBox.before)
	{
	    if (Restore)
	    	RestoreTAG(hDC);
	    DestroySavedScreen (&TAGBox.before,0);
   	}
	TAGBox.RestorePoint.x = TAGBox.rect.left-1;
	TAGBox.RestorePoint.y = TAGBox.rect.top-1;
	Rect.left = TAGBox.RestorePoint.x;
	Rect.top = TAGBox.RestorePoint.y;
	Rect.right = Rect.left + TAGBox.bmWidth + 2;
	Rect.bottom = Rect.top + TAGBox.bmHeight + 2;  
	if (TAGBox.Shadow)
	{
		int	inc = ShadowInc;//max((ShadowInc * (Rect.right - Rect.left)) / (CurView->ScreenRect.right - CurView->ScreenRect.left),3*DeviceToScreenFactor());
		Rect.bottom += inc;
		Rect.right += inc;
	}
	HavePL=FALSE; 
	if (MoveMode)
	{
		RECT	SaveRect = Rect;

		if (TAGBox.PLstyle)
			AddPointToRect (TAGBox.TAGPointScr,&SaveRect);
		TAGBox.before = SaveScreen2 (hWnd,hDC,SaveRect,0,0);
		hTBPen = CreatePen (PS_SOLID,0,RGB(0,0,0));
	}
	else 
	{
		hTBPen = CreatePen (PS_SOLID,0,TAGBox.BGcolor);
		TAGBox.before = 0;
	}
	if (Save)
	{
		RECT	SaveRect = Rect;

		if (TAGBox.PLstyle)
			AddPointToRect (TAGBox.TAGPointScr,&SaveRect);
	    DestroySavedScreen (&TAGBox.before,0);
		TAGBox.before = SaveScreen2 (hWnd,hDC,SaveRect,0,0);
	}  
	Rect = TAGBox.rect;
	if (!MoveMode && TAGBox.Shadow)
		DisplayShadow (hDC,&TAGBox.rect,ShadowInc);//,max((ShadowInc * (Rect.right - Rect.left)) / (CurView->ScreenRect.right - CurView->ScreenRect.left),2));
	hOldPen = SelectObject (hDC,hTBPen);
	Points[0].x=TAGBox.rect.left;
	Points[0].y=TAGBox.rect.bottom;
	Points[2].x=TAGBox.rect.left;
	Points[2].y=TAGBox.rect.top;
	Points[4].x=TAGBox.rect.right;
	Points[4].y=TAGBox.rect.top;
	Points[6].x=TAGBox.rect.right;
	Points[6].y=TAGBox.rect.bottom;
	Points[8]=Points[0];
	Points[1]=MidPoint(Points[0],Points[2]);
	Points[3]=MidPoint(Points[2],Points[4]);
	Points[5]=MidPoint(Points[4],Points[6]);
	Points[7]=MidPoint(Points[6],Points[8]);
	if (MoveMode)
	{
		if (!Restore)
			Polyline (hDC,(LPPOINT)Points,9);
	}
	else
	{	
		if (TAGBox.BGstyle <=1)
		{
			//hTBBrush = CreateSolidBrush(TAGBox.BGcolor); 
			hTBBrush = CreateGMBrush (TAGBox.BGcolor,-2,hDC);
			//OldMode = SetROP2 (hDC,R2_COPYPEN);
		}
		else
		{
			hBM = LoadBitmap(hInst,"TRANBM");
			hTBBrush =  CreatePatternBrush(hBM);
			OldTextColor = SetTextColor(hDC,TAGBox.BGcolor);
			OldMode = R2_MASKPEN; 
			OldMode = SetROP2(hDC,OldMode);

			
		}
		hOldBrush = SelectObject (hDC,hTBBrush);
//		width = min(MainRect.right-MainRect.left,MainRect.bottom-MainRect.top) * (float)TAGBox.BorderStyle/350.0;
		if (TAGBox.BorderStyle!=4)
			width = (TAGBox.BorderStyle*2-1)*DeviceToScreenFactor();  
		else
			width = (3)*DeviceToScreenFactor()*ShrinkFactor;
		width = max (TAGBox.PLwidth,width);
		LineWidth = width;	
		OldPen = SelectObject (hDC,GetStockObject(NULL_PEN));
		Elwh  = (RECTWIDTH(&TAGBox.rect) + RECTHEIGHT(&TAGBox.rect))/RoundingFactor;
		if (TAGBox.BGstyle)
		{
			switch (TAGBox.Shape)
			{
			case 0:
				Polygon (hDC,(LPPOINT)Points,9);
				break;
			default:
				RoundRect (hDC,TAGBox.rect.left,TAGBox.rect.top,TAGBox.rect.right,TAGBox.rect.bottom,Elwh,Elwh);
				SetROP2(hDC,DisplayRasterOpt);
//				SelectObject (hDC,GetStockObject (NULL_BRUSH));
//				RoundRect (hDC,TAGBox.rect.left,TAGBox.rect.top,TAGBox.rect.right,TAGBox.rect.bottom,Elwh,Elwh);
				break;
			}
		}
		if (OldTextColor >= 0)
			SetTextColor(hDC,OldTextColor); 
		if (OldMode)
			SetROP2(hDC,OldMode);
		MinDist = LONG_MAX;
		for (i=0;i<9;i++)
		{
			Dist = idist (TAGBox.TAGPointScr,Points[i]);
			if (Dist < MinDist)
			{	MinDist = Dist;
				TAGBox.ConnectPoint = Points[i];
			}
		}
		if (TAGBox.TAGPointScr.x < Points[0].x && TAGBox.TAGPointScr.y > Points[2].y && TAGBox.TAGPointScr.y < Points[0].y)
			TAGBox.ConnectPoint = Points[1];
		else if (TAGBox.TAGPointScr.x > Points[4].x && TAGBox.TAGPointScr.y > Points[2].y && TAGBox.TAGPointScr.y < Points[0].y)
			TAGBox.ConnectPoint = Points[5];
		else if (TAGBox.TAGPointScr.y < Points[2].y && TAGBox.TAGPointScr.x > Points[0].x && TAGBox.TAGPointScr.x < Points[6].x)
			TAGBox.ConnectPoint = Points[3];
		else if (TAGBox.TAGPointScr.y > Points[0].y && TAGBox.TAGPointScr.x > Points[0].x && TAGBox.TAGPointScr.x < Points[6].x)
			TAGBox.ConnectPoint = Points[7];
		hPointer = ShowPointerLine (hDC,TAGBox.rect,TAGBox.TAGPointScr,TAGBox.ConnectPoint,&HavePL,MoveMode,LineWidth,Elwh,Elwh,pRect);
		if (hPointer)
		{
			HRGN    NewRgn; 
			LPPOINT	pPPoints=(LPPOINT)GlobalLock (hPointer);
			
			NewRgn = CreatePolygonRgn (pPPoints,3,ALTERNATE);
			GSSiGlobUlFree (&hPointer);
			CombineRgn (NewRgn,hRgn,NewRgn,RGN_DIFF);
			SelectClipRgn (hDC,NewRgn); 
			GSSiDeleteObject(&NewRgn);
		}
        if (TAGBox.BorderStyle)
        {
		//FillRect (CurView->hDC,&CurView->ScreenRect,GetStockObject (GRAY_BRUSH));
			BorderPen = CreatePen (PS_SOLID,(int)IDNINT(width),TAGBox.BorderColor);
			SelectObject (hDC,BorderPen);
			Rect = TAGBox.rect;
			InflateRect (&Rect,(int)-IDNINT(width/2-1*DeviceToScreenFactor()),(int)-IDNINT(width/2-1*DeviceToScreenFactor()));
			SelectObject(hDC,GetStockObject(NULL_BRUSH));
			switch (TAGBox.Shape)
			{
			case 0:
				DrawRectPoly (hDC,&Rect,0);
				break;
			default:
				RoundRect (hDC,Rect.left,Rect.top,Rect.right,Rect.bottom,Elwh,Elwh);
				break;
			}
		}
		SelectClipRgn (hDC,hRgn); 
		SelectObject(hDC,OldPen);
		GSSiDeleteObject (&BorderPen);
//        DeleteObject(BorderPen);
		if (TAGBox.BorderStyle==4)
		{   
			HPEN	BorderPen2,hOldPen;
			Rect = TAGBox.rect;  
			BorderAdjust = ShrinkFactor * DeviceToScreenFactor();
			InflateRect (&Rect,
			             (int) -IDNINT(width + BorderAdjust + TAGBox.incx),
			             (int) -IDNINT(width + BorderAdjust + TAGBox.incy));
			width = 1*DeviceToScreenFactor()*ShrinkFactor;
			BorderPen2 = CreatePen (PS_SOLID,(int)IDNINT(width),TAGBox.BorderColor); 
			hOldPen = SelectObject(hDC,BorderPen2);
			SelectObject (hDC,hOldPen);
			switch (TAGBox.Shape)
			{
			case 0:
				FillRectColor (hDC,&Rect,TAGBox.InnerColor);
				DrawRectPoly (hDC,&Rect,0);  
				break;
			default:
				hTBBrush = CreateGMBrush (TAGBox.InnerColor,-2,hDC);
				hOldBrush = SelectObject (hDC,hTBBrush);
				RoundRect (hDC,Rect.left,Rect.top,Rect.right,Rect.bottom,Elwh,Elwh);
				SelectObject (hDC,hOldBrush);
				break;
			}
			DeleteObject (BorderPen2);
			LineWidth++;
		}	
		if(OldBKMode) SetBkMode(hDC,OldBKMode);
		OldBKMode=SetBkMode(hDC,TRANSPARENT);
/*        iLogPixsY = GetDeviceCaps(hDC, LOGPIXELSY);
	    TAGBox.LogFont.lfHeight = -1 * (iLogPixsY * TAGBox.TXheight / 72);*/
	    if (TAGBox.Factor) 
	    	Factor = TAGBox.Factor;
	    else
	    	Factor = 1; 
	    if ((IBFactor = GetGlobalDVal2 ("[%INFOBOXFACTOR]",1)) != 1)
    		TAGBox.bmWidthD = TAGBox.bmHeightD = 0;
	    	
	    Factor *= IBFactor;
	    if (!Printing && TAGBox.TXheight < 0)
			TAGBox.TXheight = fabs (PixelsToTAGFontHt(&TAGBox.LogFont,TAGBox.CoordStyle));
//	    	TAGBox.TXheight = -TAGBox.TXheight;
	    else  
	    	TAGBox.LogFont.lfHeight = -labs(IDNINT(TAGFontHtToPixels (TAGBox.TXheight*Factor,TAGBox.CoordStyle))); 
	    theightreal =  TAGBox.LogFont.lfHeight * ShrinkFactor;
	    TAGBox.LogFont.lfHeight *= ShrinkFactor;
	    //TAGBox.LogFont.lfHeight -= -BorderAdjust;
	    if (!TAGBox.LogFont.lfHeight) 
	    	TAGBox.LogFont.lfHeight = 1;
	    hfactor = theightreal / TAGBox.LogFont.lfHeight;
	    if (Printing)
	    {       
	    	char	str[64];  
	    	int		FontOffset;
	    	
	    	_fstrcpy (str,"[%PRINT_FONT_OFFSET]");
	    	ExpandText(str);
	    	FontOffset = atoi(str);
	    	TAGBox.LogFont.lfHeight += FontOffset; 
	    }
		TAGBox.LogFont.lfOrientation = TAGBox.LogFont.lfEscapement;
        TagFont = CreateFontIndirect((LPLOGFONT)&TAGBox.LogFont);
        OldTextColor=SetTextColor(hDC,TAGBox.TXcolor);
        OldFont = SelectObject(hDC, TagFont); 
        
	    lpText = ExpandArea;
		x = Rect.left+ TAGBox.incx;
		y = Rect.top + TAGBox.incy; 
		yr = y;  
		SaveRect = TAGBox.rect;
		InflateRect (&Rect,(int)-IDNINT(TAGBox.incx+BorderAdjust),(int)-IDNINT(TAGBox.incy+BorderAdjust));
		TAGBox.rect = Rect;

   		_fstrcpy (ExpandArea,TAGBox.text);
 		if (_fstrnicmp(ExpandArea,"$REPORT(",8))
    		ExpandText (ExpandArea);
    	TAGBox.rect = SaveRect;
	    while (NextLine (&lpText,Line,0))
	    {   
	    	_fstrcpy(ExpLine,Line);
		    if (_fstrnicmp(ExpLine,"$REPORT(",8))
			{
	    		ExpandText (ExpLine);
	    		Truncate (ExpLine);
				if (!ExpLine[0])
				{
	        		ExpLine[0]=' ';
	        		ExpLine[1]='\0';
				}
			}
		    if (!_fstrnicmp(Line,"$REPORT(",8))
		    {   
				if (TAGBox.Factor) 
	    			Factor = TAGBox.Factor;
				else
	    			Factor = 1; 
				if (TAGBox.hReport)
					DisplayReport (hDC,TAGBox.hReport,Rect,SaveRect, Factor*DeviceToScreenFactor(), TAGBox.Refno,0);  
		    }
	
		    else if (!_fstrnicmp(Line,"$BITMAP(",8))
		    {   
				LPSTR	lpComma, pPar;
				double	factor;
			 	BITMAPFILEHEADER bmfHead;
				LPBITMAPINFOHEADER	pDibInfo;
				HANDLE	hDibInfo, hImage; 
				RECT	BMRect;
				int	height,width, nLines=0;
		
		 		lpReport = Line+8;
				lpEndReport = MatchLev (lpReport,')');
				if (lpEndReport)
				{                                                    
					*lpEndReport = '\0';
					if ((lpComma=MatchLev (Line,',')))
					{
						*lpComma++=0;
						nLines = atoi (lpComma);
					}
		    		_fstrcpy (ExpLine,lpReport);
		    		*lpEndReport = ')';
					ExpandText (ExpLine);
					if ((pPar=strrchr (ExpLine,'(')))
					{
						float	fwidth, fheight;

						*pPar++=0;
						lpComma = strchr (pPar,',');
						*lpComma = 0;
						fwidth = atof (pPar);
						fheight = atof (lpComma+1);
						height = fheight / CurView->BaseUnitsPerPixel;
						width  = fwidth / CurView->BaseUnitsPerPixel;
					}
					else
					{
						if (!GetDIBDimensions(ExpLine,&height,&width))
							goto TextOut;
						height *= DeviceToScreenFactor();
						width  *= DeviceToScreenFactor();
					}
					height *= Factor;
					width  *= Factor;
					if (nLines)
					{   
						factor = (double)width / height;
						height = abs (TAGBox.LogFont.lfHeight) * nLines;
						width = factor * height;
					} 
					else
					{
						height *= ShrinkFactor;
						width *= ShrinkFactor;
					}
					BMRect.left = x;
					BMRect.top = y;// + abs(TAGBox.LogFont.lfHeight)/3;
					BMRect.right = TAGBox.rect.right - TAGBox.incx;
					BMRect.bottom = BMRect.top + height;
					BMRect = Rect;
					BMRect.bottom = BMRect.top + height;
					if (!DisplayBMFileInRect (hDC,ExpLine,BMRect,TRUE))
						goto TextOut;
					y = yr = BMRect.bottom + abs(TAGBox.LogFont.lfHeight)/3;  
				}
		    }
			
			else
			{  
		TextOut:
				GetTextExtentPoint32 (hDC,ExpLine,_fstrlen(ExpLine),&txSize);
				Twidth = txSize.cx;
				Theight = txSize.cy;
				if (TAGBox.Just == 3)
					xt = TAGBox.rect.right - Twidth;
				else if (TAGBox.Just == 2)
					xt = TAGBox.rect.left + ((TAGBox.rect.right - TAGBox.rect.left) - Twidth) / 2;
				else
					xt = x;

					if (TAGBox.Flags.ShadowText)
						TextOutWithShadow(hDC, xt, y, ExpLine, _fstrlen(ExpLine), 1, TAGBox.FontShadowColor);
					else
						ii = TextOut(hDC, xt, y, ExpLine, _fstrlen(ExpLine));
				

				yr += Theight * hfactor;
				y = IDNINT (yr);
			}
		}
		if (OldTextColor >= 0)
        	SetTextColor(hDC,OldTextColor);
        SelectObject (hDC,OldFont);
        DeleteObject(TagFont);
		SetBkMode(hDC,OldBKMode);
	}
	if (hOldPen) SelectObject (hDC,hOldPen);
	if (hTBPen) DeleteObject (hTBPen);
	if (hOldBrush) SelectObject (hDC,hOldBrush);
	if (hBM) DeleteObject(hBM);
	if (hTBBrush) DeleteObject (hTBBrush);
	if (TAGBox.Flags.UseOrigSize)
	{
    	BoundsToWinRect (&TAGBox.BlowUpBounds,&Rect);
		{   
			COLORREF	Color;
			BYTE		PatByt;  
			COLORREF	color;
			PATBYTE		PatByte;      
	
			PatByte.Transparent = 1;  
			PatByte.BGOpt = 0;
			PatByte.Pattern = 2;
			_fmemmove (&PatByt,&PatByte,1);
			Color = RGBW (192,192,192,PatByt);
			FillRectPoly (hDC,&Rect,Color);
		//	RoundRect (hDC,Rect.left,Rect.top,Rect.right,Rect.bottom,10,10);
		} 
    }
	if (!Printing && !MoveMode && TAGBox.Flags.CloseIcon)
	{
		RECT	Rect=TAGBox.rect; 
		HPEN	hSavePen, hBlackPenDW = CreatePen (PS_SOLID,2,0);

	    SaveDC (hDC);
		SetDisplayMode (hDC, GF_SCREENMODE); 
	    /*SetMapMode    (hDC, MM_TEXT );
	    SetWindowOrgEx  ( hDC, 0, 0,0 );
	    SetViewportOrgEx( hDC, 0, 0,0 );*/    
	    SelectClipRgn (hDC,0);
		SetBkMode (hDC,TRANSPARENT);
					     	
		Rect.top += 1;
		Rect.right -= 1;
		Rect.bottom = Rect.top + 15;  
		Rect.left = Rect.right - 15;
		FillRect (hDC,&Rect,GetStockObject(LTGRAY_BRUSH));
		//	FrameRect (hDC,&Rect,GetStockObject(BLACK_BRUSH));   
		hSavePen = SelectObject (hDC,hBlackPenDW);
		MoveToEx (hDC, Rect.left+4, Rect.bottom-4,0);
		LineTo (hDC, Rect.right-4,Rect.top+4);
		MoveToEx (hDC, Rect.left+4, Rect.top+4,0);
		LineTo (hDC, Rect.right-4,Rect.bottom-4);
		SelectObject (hDC,hSavePen);
		GSSiDeleteObject(&hBlackPenDW);
		
		Draw3DBorder(hDC, &Rect,-UP_3D, FALSE); 
	} 

//	ShowPointerLine (hDC,TAGBox.rect,TAGBox.TAGPointScr,TAGBox.ConnectPoint,&HavePL,MoveMode,LineWidth);
	if (TAGBox.hTAGDB)
	{
    	CloseDataFile(TRUE,&TAGBox.hTAGDB);
    }
Exit:
	GSSiGlobUlFree (&hMEM);
    InInfoBox=FALSE; 
    InDrawTAG = FALSE;
    GSSiDeleteObject(&hRgn);
    RestoreDC (hDC,-1);  
	if (!(InMoveMode || Restore))
		UnloadReport (&TAGBox.hReport);
	if (!Restore)
		SaveFullWindowBitmap (hWnd);
	return;

}
HANDLE ShowPointerLine (HDC hDC,RECT rect,POINT endpoint, POINT begpoint, BOOL *HavePL,BOOL MoveMode,int LineWidth,int Elww,int Elwh,LPRECT pClipRect)
{	HPEN	hOldPen, hSolidPen, hDashPen;
	static	POINT	LastBP, LastEP; 
	double	TipWidth;
	HANDLE	hPointer=0;
	int		rc;

   
    if (!TAGBox.PLstyle) return 0; 
    
    TipWidth = min (rect.right - rect.left,rect.bottom - rect.top);
    TipWidth /= 5;
	if (*HavePL)
		hPointer = DrawTAGPointerLine(hDC, LastBP, LastEP, MoveMode, LineWidth, TipWidth, TAGBox.PLstyle, TAGBox.PointerColor, TAGBox.BorderStyle, TAGBox.BorderColor);
	LastBP = begpoint;
	LastEP = endpoint;
	if (PtInRect(&rect,endpoint))
		*HavePL = FALSE;
	else
	{
//		DrawPointerLine (hDC,begpoint,endpoint,hDashPen,hSolidPen,10);
    	SaveDC (hDC);
		{   HRGN    NewRgn, OvrLapRgn; 
		    RECT	TBRect=rect;
		    
		    InflateRect (&TBRect,-(int)IDNINT(1*DeviceToScreenFactor()+LineWidth/2),-(int)IDNINT(1*DeviceToScreenFactor()+LineWidth/2)); 
			if (!pClipRect)
				pClipRect = &MainRect;
		    NewRgn = CreateRectRgnIndirect (pClipRect);
			switch (TAGBox.Shape)
			{
			case 0:
				OvrLapRgn = CreateRectRgnIndirect (&TBRect);
				break;
			default:
				OvrLapRgn = CreateRoundRectRgn (TBRect.left,TBRect.top,TBRect.right,TBRect.bottom,Elww,Elwh);
				break;
			}
        	rc = CombineRgn (NewRgn,NewRgn,OvrLapRgn,RGN_DIFF);
	  		SelectClipRgn (hDC,NewRgn); 
            DeleteObject (OvrLapRgn);  
            DeleteObject (NewRgn);
	  	}
		hPointer = DrawTAGPointerLine(hDC, RectMid(&rect), endpoint, MoveMode, LineWidth, TipWidth, TAGBox.PLstyle, TAGBox.PointerColor, TAGBox.BorderStyle, TAGBox.BorderColor);
    	RestoreDC (hDC,-1);
		*HavePL = TRUE;
	}

	return hPointer;
}

void DrawPointerLine (HDC hDC,POINT begpoint,POINT endpoint,HPEN LinePen, HPEN TipPen,
					  int TipWidth,int ToPointOffset)
{   POINT	Points[3];
	HPEN	hOldPen;
	HBRUSH	OldBrush, hBrush;  
	HBITMAP	hbmp;
	double	az,len;  
	short	OldMode=0;

	az = getaz (endpoint,begpoint);
	len = idist (begpoint,endpoint);
	if (TipWidth < 0)
		TipWidth = IDNINT(((double)-TipWidth/100)*len);
	else
		TipWidth = min (len,TipWidth);
	if (ToPointOffset < 0)
	{
		endpoint = newpt(begpoint,az+PY,-ToPointOffset);
		TipWidth = -ToPointOffset/5;
	}
	else if (ToPointOffset)
	{
		if (len <= ToPointOffset)
			return; 
		endpoint = newpt(begpoint,az+PY,len-ToPointOffset);
	}
	hOldPen = SelectObject (hDC,LinePen);    
	Points[0].x=begpoint.x;
	Points[0].y=begpoint.y;
	Points[1].x=endpoint.x;
	Points[1].y=endpoint.y;
	Polyline (hDC,(LPPOINT)Points,2); 
	if (LinePen != TipPen)
		SelectObject (hDC,TipPen);
	Points[0]=newpt(endpoint,az+HALFPI/2,TipWidth);
	Points[2]=newpt(endpoint,az-HALFPI/2,TipWidth);
	Polyline (hDC,(LPPOINT)Points,3);    
	
	SelectObject (hDC,hOldPen);
	return;
}

HANDLE DrawTAGPointerLine (HDC hDC,POINT begpoint,POINT endpoint,BOOL MoveMode,int LineWidth,double TipWidth,
	int PLstyle, COLORREF PointerColor, int BorderStyle, COLORREF BorderColor)
{   LPPOINT	Points; 
	COLORREF	color;
	HPEN	hOldPen=0, hPen=0;
	HBRUSH	OldBrush, hBrush;  
	HBITMAP	hbmp;
	BYTE		PatByt;
	PATBYTE		PatByte;
	double	az;  
	short	type=1, OldMode=0;
	BOOL	TransParent=FALSE;
	HANDLE	hPointer=0;

    switch (PLstyle)
    {
    	case 1:
			hPen = CreatePen (PS_SOLID,(int)IDNINT(1*DeviceToScreenFactor()),PointerColor);
			DrawPointerLine (hDC,begpoint,endpoint,hPen,hPen,(int)IDNINT(5*DeviceToScreenFactor()),0); 
			GSSiDeleteObject (&hPen);
    		break;
    	case 3:
    		TipWidth *=2;
    	case 2:
			hPointer = GSSiGlobAlloc (1592,GMEM_MOVEABLE,3*sizeof(POINT));
			Points = (LPPOINT)GlobalLock (hPointer);
			az = getaz (begpoint,endpoint);
			Points[0]=newpt(begpoint,az+HALFPI,TipWidth);
			Points[2]=newpt(begpoint,az-HALFPI,TipWidth);    
			Points[1]=endpoint;  
/*	    	PatByt = GetWValue (TAGBox.PointerColor); 
	    	_fmemmove (&PatByte,&PatByt,1);
	    	color = ColorWOWidth (TAGBox.PointerColor);
    		if (PatByte.Pattern && !MoveMode)     
    		{
				HBITMAP hbmp = LoadBitmap(hInst, MAKEINTRESOURCE(PatBMP[PatByte.Pattern-1]));  
				hBrush = CreatePatternBrush(hbmp);
		        DeleteObject (hbmp); 
			    SetTextColor (hDC,color);     
			    SetBkColor (hDC,RGB(255,255,255));  
			    SetROP2(hDC,R2_COPYPEN);
			    if (PatByte.Transparent)
			    {
					if (GetROP2 (hDC) != R2_NOT)
						OldMode = SetROP2(hDC,R2_MASKPEN);
					TransParent = TRUE;
			    }
            } 
            else
            	hBrush = CreateSolidBrush (color);*/
			OldMode = GetROP2 (hDC);
			hBrush = CreateGMBrush (PointerColor,-2,hDC);
			OldBrush = SelectObject (hDC,hBrush); 
			SelectObject (hDC,GetStockObject (NULL_PEN));
			Polygon (hDC,(LPPOINT)Points,3);
        	SetROP2(hDC,OldMode);
			SelectObject (hDC,GetStockObject (NULL_BRUSH));
			if (BorderStyle)
			{
				if (BorderStyle<4)
					color = BorderColor;
				else
					color = PointerColor;
				hPen = CreatePen (PS_SOLID,LineWidth,color);
				hOldPen=SelectObject (hDC,hPen);
/*			if (!TransParent)
			{
				hPen = CreatePen (PS_SOLID,LineWidth,color);
				hOldPen=SelectObject (hDC,hPen);
			}
			else 
			{
				hPen = CreatePen (PS_SOLID,IDNINT(DeviceToScreenFactor()),color);
				hOldPen=SelectObject (hDC,hPen);
			}
			else
				hOldPen = SelectObject (hDC,GetStockObject(NULL_PEN));*/
				Polygon (hDC,(LPPOINT)Points,3); 
			}
			SelectObject (hDC,OldBrush); 
	        DeleteObject (hBrush);  
			if (hOldPen)
				SelectObject (hDC,hOldPen);
			GSSiDeleteObject (&hPen);
			GlobalUnlock (hPointer);
		break;
		
	}
	
	return hPointer;
}

void RestoreTAG(HDC hDC)
{   HDC     hdcMem;
	HBITMAP	hbmPrev;

/*	SetDisplayMode (hDC, GF_TEXTMODE);
	hdcMem = CreateCompatibleDC(hDC); 
	if (TAGBox.before)
		hbmPrev = SelectObject(hdcMem, TAGBox.before);

	BitBlt(hDC, TAGBox.RestorePoint.x, TAGBox.RestorePoint.y,TAGBox.bmWidth+2,TAGBox.bmHeight+2,
	       hdcMem, 0, 0,  SRCCOPY);

	if (TAGBox.before)
		SelectObject(hdcMem, hbmPrev);
	DeleteDC(hdcMem);*/
	RestoreScreen2 (hDC, TAGBox.before,0,FALSE);
}

POINT TAGPtToWinPt (DPOINT TagPoint)
{
	POINT Point;

	switch (TAGBox.CoordStyle)
	{
	default:
		case 0:
	    	return (BasePtToWinPt (&TagPoint));
	    	break;
	    case 1:
	    	Point.x = CurView->Rect.left + TagPoint.x * (CurView->Rect.right - CurView->Rect.left);
	    	Point.y = CurView->Rect.top - TagPoint.y * (CurView->Rect.top - CurView->Rect.bottom);
	    	break;  
	    case 2:
	    	Point.x = MainRect.left + TagPoint.x * (MainRect.right - MainRect.left);
	    	Point.y = MainRect.top - TagPoint.y * (MainRect.top - MainRect.bottom);
	    	break;
	    case 4://same as 2 but based on client rect instead of mainrect 
	    	Point.x = ClientRect.left + TagPoint.x * (ClientRect.right - ClientRect.left);
	    	Point.y = ClientRect.top - TagPoint.y * (ClientRect.top - ClientRect.bottom);
	    	break;
	    case 3:
	    	Point.x = TagPoint.x;
	    	Point.y = TagPoint.y;
	    	break;
	    	
	}
   	return (Point);  
}


DPOINT WinPtToTAGPt (POINT WinPoint)
{	DPOINT Point;
	int		height, width;

	switch (TAGBox.CoordStyle)
	{
	default:
	case 0:
	    	return (WinPtToBasePt (WinPoint));
	    	break;
	    case 1:
	    	width = CurView->Rect.right - CurView->Rect.left;
	    	height =  CurView->Rect.top - CurView->Rect.bottom; 
	    	if (!width || !height)
	    	{
	    		Point.x=0;
	    		Point.y=0;
	    	}
	    	else
	    	{
		    	Point.x = (double)(WinPoint.x - CurView->Rect.left)/width;
		    	Point.y =  (double)(CurView->Rect.top-WinPoint.y)/height;
	    	}
	    	break; 
	    case 2:   
	    case 4:
	    {
	    	RECT	Rect;
	    	
	    	if (TAGBox.CoordStyle == 2)
	    		Rect = MainRect;
	    	else
	    		Rect = ClientRect;
	    		 
	    	width = Rect.right - Rect.left;
	    	height =  Rect.top - Rect.bottom; 
	    	if (!width || !height)
	    	{
	    		Point.x=0;
	    		Point.y=0;
	    	}
	    	else
	    	{
		    	Point.x = (double)(WinPoint.x - Rect.left)/width;
		    	Point.y =  (double)(Rect.top-WinPoint.y)/height;
	    	}
	    	break;
	    }	
	    case 3:
	    	Point.x = WinPoint.x;
	    	Point.y = WinPoint.y;
	    	break;
	    	
	}
   	return (Point);

}

BOOL	GetTAG (LONG Refno, LPSTR TAG)
{
	_ltoa (Refno,TAG,10);
	return (TRUE);
} 











double TAGFontHtToPixels (double TAGFontHeight,short CoordStyle)
{   DPOINT	DPoint1, DPoint2;
	POINT	Point1, Point2;
	long	iLogPixsY,iLogPixsX;

	switch (CoordStyle)
	{
	default:
		case 0:
			DPoint1.x = CurView->WBounds.xmn;
			DPoint1.y = CurView->WBounds.ymn;
	    	Point1 = BasePtToWinPt (&DPoint1);
			DPoint2.x = CurView->WBounds.xmn + TAGFontHeight*100;
			DPoint2.y = CurView->WBounds.ymn;
	    	Point2 = BasePtToWinPt (&DPoint2);
	    	return (idist(Point1,Point2)/100);
	    	break;
	    case 1:
	    	return ((TAGFontHeight * (CurView->Rect.bottom - CurView->Rect.top)));
	    	break;
	    case 2:
	    	iLogPixsY = GetDeviceCaps(CurView->hDC, LOGPIXELSY); 
	    	iLogPixsX = MainRect.right - MainRect.left;
	    	return (TAGFontHeight * iLogPixsX);
	    	break;
	    case 3:
	    	iLogPixsY = GetDeviceCaps(CurView->hDC, LOGPIXELSY); 
	    	return (TAGFontHeight * iLogPixsY*min(DeviceToScreenFactor(),1));
	    	break;
	    case 4:
	    	iLogPixsY = GetDeviceCaps(CurView->hDC, LOGPIXELSY); 
	    	iLogPixsX = ClientRect.right - ClientRect.left;
	    	return (TAGFontHeight * iLogPixsX);
	    	break;
	}

}

float PixelsToTAGFontHt (LPLOGFONT lpFont,short CoordStyle)
{   DPOINT	DPoint1, DPoint2;
	POINT	Point1;
	int		height;
	int 	Pixels;     
	float	Xfac,WidthToHeightRatio;
	long	iLogPixsY,iLogPixsX, HRes;
    
    Pixels = lpFont->lfHeight;
	switch (CoordStyle)
	{
	default:
		case 0:
			Point1.x = 0;
			Point1.y = 0;
	    	DPoint1 = WinPtToBasePt (Point1);
			Point1.y = Pixels;
	    	DPoint2 = WinPtToBasePt (Point1);
	    	return (ldistp(DPoint1,DPoint2));
	    	break;
	    case 1: 
	    	iLogPixsY = GetDeviceCaps(CurView->hDC, LOGPIXELSY); 
	   // 	return ((float)Pixels/(float)iLogPixsY);
   // TAGBox.LogFont.lfHeight = -1 * (iLogPixsY * TAGBox.TXheight / 72); 
	    	height = CurView->Rect.bottom - CurView->Rect.top;  
	    	if (!height) return 1.0;
	    	return ((float)Pixels/height);  
	    	break;
	    case 2:
	    	iLogPixsX = MainRect.right - MainRect.left;
	    	return ((float) Pixels / iLogPixsX);
	    	break; 
	    case 3:
	    	iLogPixsY = GetDeviceCaps(CurView->hDC, LOGPIXELSY); 
	    	return ((float) Pixels*max(1,DeviceToScreenFactor()) / ((float)iLogPixsY));
	    	break; 
	    
	    case 4:
	    	iLogPixsX = ClientRect.right - ClientRect.left;
	    	return ((float) Pixels / iLogPixsX);
	    	break;
	}

}  
  
int GetNumInfoBox (void)
{            
	int n=0;
	HFILE	FidTag; 
	HANDLE	hMem=GSSiGlobAlloc ( 385,GMEM_MOVEABLE,sizeof(TAGBOX));
	LPTAGBOX	pTAGBox=(LPTAGBOX)GlobalLock (hMem);
	
	if (*TagFile)
	{
		FidTag = GSSiOpenFile (TagFile,0,OF_READ);
		if (FidTag != HFILE_ERROR)
		{
			while (BigRead (FidTag,(HPSTR)pTAGBox,sizeof(TAGBOX)) == sizeof(TAGBOX))
				if (TAGBox.ViewportID > 0) 
					n++;
			GSSiClose (FidTag); 
		}
	}
	GSSiGlobUlFree (&hMem);
	return n;
} 

void AssignTAGInstance (int PickNum)
{            
	int n=0;
	HFILE	FidTag;
	OFSTRUCTGM	OFStruct;
	TAGBOX	TAGBox2;
	char	Group[34]; 
	LPSTR	lpColon;  
	long	MaxGroup=0;
	short	TAGNum=0;
	
	if (!*TagFile)
		return;                  
	_fstrcpy (Group,TAGBox.Desc);
	if ((lpColon = _fstrchr (Group,':')))
		*lpColon = 0;
	
	FidTag = GSSiOpenFile(TagFile, (LPOFSTRUCTGM)&OFStruct, OF_READ);
	if (FidTag != HFILE_ERROR)
	{
		while (BigRead (FidTag,(HPSTR)&TAGBox2,sizeof(TAGBOX)) == sizeof(TAGBOX))
		{
			TAGNum++;
			if (TAGNum != PickNum && TAGBox2.ViewportID > 0)
			{
				if ((lpColon = _fstrchr (TAGBox2.Desc,':')))
				{
					*lpColon++ = 0;
					if (!_fstricmp (Group,TAGBox2.Desc))
					{
						MaxGroup = max (MaxGroup,atol(lpColon));
					}		
				}
			} 
		}
		GSSiClose (FidTag); 
	}
	MaxGroup++;
	sprintf (TAGBox.Desc,"%s:%ld",Group,MaxGroup);
	return;
}  
 

void UpdateGroup (HFILE FidTag)
{   
	TAGBOX	TAGBox2;  
	long	Loc, NextLoc;
	char	Group[34]; 
	LPSTR	lpColon;
	
	return;  //5/7/2003
	                  
	_fstrcpy (Group,TAGBox.Desc);
	if ((lpColon = _fstrchr (Group,':')))
		*lpColon = 0;
	GSSillseek (FidTag,0,0);  
	Loc = 0;
	while (BigRead (FidTag,(HPSTR)&TAGBox2,sizeof(TAGBOX)) == sizeof(TAGBOX)) 
	{
		NextLoc = GSSillseek (FidTag,0,1);
		GSSillseek (FidTag,Loc,0);	
		if (TAGBox2.ViewportID >= 0)
		{                           
			if ((lpColon = _fstrchr (TAGBox2.Desc,':')))
			{
				*lpColon = 0;
				if (!_fstricmp (Group,TAGBox2.Desc))
				{   
					*lpColon = ':';
					TAGBox2.Flags.AutoEdit = TAGBox.Flags.AutoEdit;
					TAGBox2.Flags.AutoMove = TAGBox.Flags.AutoMove;
					TAGBox2.ViewportID = TAGBox.ViewportID;
					TAGBox2.CoordStyle = TAGBox.CoordStyle;
					TAGBox2.BGstyle = TAGBox.BGstyle;
					TAGBox2.PLwidth = TAGBox.PLwidth;
					TAGBox2.PLstyle = TAGBox.PLstyle;
					TAGBox2.BorderStyle = TAGBox.BorderStyle;
					TAGBox2.TXheight = TAGBox.TXheight;
					TAGBox2.Just = TAGBox.Just;
					TAGBox2.incx = TAGBox.incx;
					TAGBox2.incy = TAGBox.incy;
					TAGBox2.BGcolor = TAGBox.BGcolor;
					TAGBox2.TXcolor = TAGBox.TXcolor;
					TAGBox2.BorderColor = TAGBox.BorderColor;
					TAGBox2.PointerColor = TAGBox.PointerColor;
					TAGBox2.Shadow = TAGBox.Shadow;
					TAGBox2.LogFont = TAGBox.LogFont;
				//	_fstrncpy (TAGBox2.Contents,TAGBox.Contents,10);
					_fstrncpy (TAGBox2.DataFile,TAGBox.DataFile,128);
					_fstrncpy (TAGBox2.SQL,TAGBox.SQL,256); 
					if (*TAGBox.DataFile)
						_fstrncpy (TAGBox2.text,TAGBox.text,sizeof(TAGBox.text));
					BigWrite (FidTag,(HPSTR)&TAGBox2,sizeof(TAGBOX),-1);
				}
			}
		}
		GSSillseek (FidTag,NextLoc,0);
		Loc = NextLoc;
	}	
	return;
}

void WriteInfoBoxes (HFILE Fid)
{
	HFILE	FidTag;
	HANDLE	hMem=GSSiGlobAlloc ( 386,GMEM_MOVEABLE,sizeof(TAGBOX));
	LPTAGBOX	pTAGBox=(LPTAGBOX)GlobalLock (hMem);
	
    if (*TagFile) 
    {
		FidTag = GSSiOpenFile (TagFile,0,OF_READ);
		if (FidTag != HFILE_ERROR)
		{
			while (BigRead (FidTag,(HPSTR)pTAGBox,sizeof(TAGBOX)) == sizeof(TAGBOX)) 
				if (TAGBox.ViewportID > 0) 
					BigWrite (Fid,(HPSTR)pTAGBox,sizeof(TAGBOX),-1);
			GSSiClose (FidTag);	
		}
	}
	GSSiGlobUlFree (&hMem);	
	return;
}

void UpdateInfoBoxVPID (LPSHORT NewIDs)
{
	HFILE	FidTag;
	OFSTRUCTGM	OFStruct;
	long	loc1=0,loc2;
	
    if (!*TagFile) 
    	return;
	FidTag = GSSiOpenFile(TagFile, (LPOFSTRUCTGM)&OFStruct, OF_READWRITE);
	if (FidTag != HFILE_ERROR)
	{   
		while (BigRead (FidTag,(HPSTR)&TAGBox,sizeof(TAGBOX)) == sizeof(TAGBOX))
		{
			loc2 = GSSillseek (FidTag,0,1);
			GSSillseek (FidTag,loc1,0); 
			if (TAGBox.ViewportID > 0) 
				TAGBox.ViewportID = NewIDs[TAGBox.ViewportID-1];
			else if (TAGBox.ViewportID < 0) 
				TAGBox.ViewportID = -NewIDs[-TAGBox.ViewportID-1]; 
			BigWrite (FidTag,(HPSTR)&TAGBox,sizeof(TAGBOX),-1);
			loc1 = loc2;
		}
		GSSiClose (FidTag);	
	}	
	return;
}

void SaveTAG(int PickNum)
{
	OFSTRUCTGM	OFStruct;
	HFILE		FidTag;

	if (!*TagFile)
		GSSiGetTempFileName (0,"gmi",0,(LPSTR)TagFile);
	if (PickNum <= 0) 
		AssignTAGInstance (PickNum);
	FidTag = GSSiOpenFile(TagFile, (LPOFSTRUCTGM)&OFStruct, OF_READWRITE);
	if (FidTag == HFILE_ERROR)
		FidTag = GSSiOpenFile(TagFile, (LPOFSTRUCTGM)&OFStruct, OF_CREATE_NODELETE);
	if (PickNum <= 0) 
		TBNum = GSSillseek (FidTag,0,2)/sizeof(TAGBOX) + 1; 
	else
	{
		TBNum = PickNum;
		GSSillseek (FidTag,(long)(PickNum-1)*sizeof(TAGBOX),0);
	} 
    TAGBox.TXheight = fabs(TAGBox.TXheight);
	UnloadReport (&TAGBox.hReport);
	BigWrite (FidTag,(HPSTR)&TAGBox,sizeof(TAGBOX),-1);  
    if ((*TAGBox.PickMacroFile || TAGBox.Flags.CloseIcon) && CurView && TAGBox.ViewportID > 0 &&
    	(TAGBox.ViewportID == CurView->ID ||
    	(pViewports[TAGBox.ViewportID-1]->DisplayInParent && pViewports[TAGBox.ViewportID-1]->Parent == CurView->ID))) 
    	AddInfoBoxRect (&TAGBox.rect,TBNum,CurView->ID);
	UpdateGroup (FidTag);
	GSSiClose (FidTag);
	return;
}

int PickTextBox (LPPOINT MousePoint)
{
	OFSTRUCTGM	OFStruct;
	int			FidTag, i;
	TAGBOX	TAGBoxSave;    
	BOOL	HaveBox=FALSE;

	if (!TagFile[0] || !CurView)
		return (FALSE);
	FidTag = GSSiOpenFile(TagFile, (LPOFSTRUCTGM)&OFStruct, OF_READ);
	if (FidTag<0)return(FALSE);
	i=0;
	while (BigRead (FidTag,(HPSTR)&TAGBox,sizeof(TAGBOX)))
	{   i++;
	    /*TAGBox.before=NULL;*/ 
	    if (TAGBox.ViewportID < 0)
	    	goto NextTB;
/*	    if (TAGBox.CoordStyle < 2 && TAGBox.ViewportID > 0 && TAGBox.ViewportID != CurView->ID)
	    	goto NextTB;*/    
	    SetViewport (TAGBox.ViewportID);  
        TAGBox.TXheight = fabs(TAGBox.TXheight);
		if (ResetTAGBox (CurView->hDC,2))
		{
			if (!MousePoint)
			{
				TAGBoxSave = TAGBox;
				HaveBox = i;
				goto GotOne;
			}
			if (TAGBox.ViewportID >0 && PtInRect (&TAGBox.rect,*MousePoint))
			{   
				TAGBoxSave = TAGBox;
				HaveBox = i;
			}
		}
NextTB:;
	}
GotOne:
	GSSiClose (FidTag);
	if (HaveBox)  
	{
		TAGBox = TAGBoxSave;
		if (!TAGBox.Desc[0]) itoa(HaveBox,TAGBox.Desc,10);
	}
	return(HaveBox);
}

void ClearTAGs()
{
	OFSTRUCTGM	OFStruct;
	int			FidTag;

	if (!*TagFile) return;
	if (ExistFile(TagFile))
	{
		HFILE	FidTag = GSSiOpenFile(TagFile, (LPOFSTRUCTGM)&OFStruct, OF_READ);

		if (FidTag != HFILE_ERROR)
		{
			while (BigRead (FidTag,(HPSTR)&TAGBox,sizeof(TAGBOX)) == sizeof(TAGBOX))
				UnloadReport (&TAGBox.hReport);
			GSSiClose (FidTag);
		}
		GSSiRemove (TagFile);
	}
	*TagFile = 0; 
	RemoveAllInfoBoxRect ();
	return;
}

void DisplayTAGs(HDC hDC)
{
	if (!CurrentConfig || DoSave) return;
	DisplayTAGs2 (hDC,0,0);
	SaveFullWindowBitmap (hWndMain);
	return;
}

void DisplayTAGs2 (HDC hDC,short From,short StartID)
{
	int		nRead;
	int			FidTag;
	int		InVP=0;

	if (!TagFile[0]) return;

    NumTags=0;
    TBNum = 0;
Restart:
	FidTag = GSSiOpenFile (TagFile,0,OF_READ);
	if (FidTag<0) return;  
	if (CurView)
		InVP = CurView->ID;
	if (From == 3)
		DoDisplayConfigs = TRUE;   
	else if (From == 4)
		ScanForReports = TRUE;
	nRead = BigRead(FidTag,(HPSTR)&TAGBox,sizeof(TAGBOX));  
	if (!nRead) goto Exit;
//    SetDisplayMode (hDC, GF_MAPMODE);
	if (TAGBox.Version == 0)
	{
		GSSiClose (FidTag);
		ConvertTAGV0ToV1(TagFile);
		goto Restart;
	}
	ContinueTAGs = 2;
	while (nRead && ContinueTAGs)
	{   
		SetViewport (InVP);
		NumTags++;
		TBNum++;    
	    TAGBox.TXheight = fabs(TAGBox.TXheight);
		if (TBNum < StartID)
			goto Next;
		if (TAGBox.Flags.UseOrigSize && From != 3)
			goto Next;
		if (From == 3)
		{
			if (TAGBox.CoordStyle != 2 && TAGBox.CoordStyle != 4)
			{
				if (TAGBox.Flags.UseOrigSize)
				{
					SetViewport (TAGBox.ViewportID);   
					if (CurView->DisplayInParent && CurView->Parent)            	
						SetViewport(CurView->Parent);
                }
				else
					goto Next;
			} 
			else if (TAGBox.ViewportID < 0)
				goto Next;
		}
		else if (From == 4)
		{   
			InDrawTAG = TRUE;
			if (TAGBox.ViewportID > 0)
				ProcessText (TAGBox.text);
			InDrawTAG = FALSE;
			goto Next;
		} 
		else if (TAGBox.CoordStyle == 2 || TAGBox.CoordStyle == 4)
			goto Next;
	    TAGBox.before=0;
		if (CurView && ((TAGBox.CoordStyle == 2 || TAGBox.CoordStyle == 4) && From == 3) || (CurView->ID == TAGBox.ViewportID && CurViewActive()))
		{   
			if (From && !TAGBox.CoordStyle)
				goto Next;
			if (ResetTAGBox (hDC,2))
			{   
				RECT	Rect;
				
				if (From == 2)
					ClearTAG (hDC);
				else if (From != 1 || !IntersectRect (&Rect,&CurView->Rect,&TAGBox.rect))
				{
				    if (*TAGBox.PickMacroFile || TAGBox.Flags.CloseIcon) 
				    	AddInfoBoxRect (&TAGBox.rect,TBNum,CurView->ID);
					DrawTAG(hWndMain, hDC,FALSE,FALSE); 
				}
			}
	    /*	if (TAGBox.before) 
	    	{
	    		DeleteObject(TAGBox.before);
	    		TAGBox.before=0;
	    	}  */
		}
Next:	nRead = BigRead(FidTag,(HPSTR)&TAGBox,sizeof(TAGBOX));
	} 
Exit:  
	ContinueTAGs = 1;
	GSSiClose(FidTag);  
	DoDisplayConfigs = FALSE;
	ScanForReports = FALSE;
	return;
}

void ListTAGs(HWND hWndDlg, int idcCB)
{
	OFSTRUCTGM	OFStruct;
	int		nRead;
	int		FidTag; 
	TAGBOX	TAGBox;


	if (!TagFile[0]) return;

Restart:
	FidTag = GSSiOpenFile(TagFile, (LPOFSTRUCTGM)&OFStruct, OF_READ);
	if (FidTag<0) return;
	nRead = BigRead(FidTag,(HPSTR)&TAGBox,sizeof(TAGBOX));  
	if (!nRead) goto Exit;
	if (TAGBox.Version == 0)
	{
		GSSiClose (FidTag);
		ConvertTAGV0ToV1(TagFile);
		goto Restart;
	}
	while (nRead)
	{   
 		SendDlgItemMessage (hWndDlg,idcCB,CB_ADDSTRING,0,(LPARAM)TAGBox.Desc);
		nRead = BigRead(FidTag,(HPSTR)&TAGBox,sizeof(TAGBOX));
	} 
Exit:
	GSSiClose(FidTag);
	return;
}

BOOL SelectTAG(int n)
{
	OFSTRUCTGM	OFStruct;
	int		nRead;
	int		FidTag;
	BOOL	rtn=FALSE; 


	if (!TagFile[0]) return FALSE;

Restart:
	FidTag = GSSiOpenFile(TagFile, (LPOFSTRUCTGM)&OFStruct, OF_READ);
	if (FidTag<0) return FALSE;
	nRead = BigRead(FidTag,(HPSTR)&TAGBox,sizeof(TAGBOX));  
	if (!nRead) goto Exit;
	if (TAGBox.Version == 0)
	{
		GSSiClose (FidTag);
		ConvertTAGV0ToV1(TagFile);
		goto Restart;
	}
	rtn=TRUE;
	while (nRead && n--)
	{   
		nRead = BigRead(FidTag,(HPSTR)&TAGBox,sizeof(TAGBOX));
	} 
Exit:
    TAGBox.TXheight = fabs(TAGBox.TXheight);
	GSSiClose(FidTag);
	return rtn;
}

short DeleteTAGByName (LPSTR Name)
{
	OFSTRUCTGM	OFStruct;
	int		nRead, l=_fstrlen (Name);
	int		FidTag;
	BOOL	rtn=FALSE; 
	long	Loc=0;


	if (!TagFile[0]) return FALSE;

Restart:
	FidTag = GSSiOpenFile (TagFile,(LPOFSTRUCTGM)&OFStruct,OF_READWRITE);
	if (FidTag<0) return FALSE;
	nRead = BigRead(FidTag,(HPSTR)&TAGBox,sizeof(TAGBOX));  
	if (!nRead) goto Exit;
	if (TAGBox.Version == 0)
	{
		GSSiClose (FidTag);
		ConvertTAGV0ToV1(TagFile);
		goto Restart;
	}
	rtn=TRUE;
	while (BigRead(FidTag,(HPSTR)&TAGBox,sizeof(TAGBOX)))
	{   
		if (l && !_fstrnicmp (Name,TAGBox.Desc,l) && TAGBox.ViewportID > 0)
		{
			GSSillseek (FidTag,Loc,0); 
			TAGBox.ViewportID = -TAGBox.ViewportID;
			BigWrite (FidTag,(HPSTR)&TAGBox,sizeof(TAGBOX),-1);
		}
		Loc = GSSillseek (FidTag,0,1);
	} 
Exit:
	GSSiClose(FidTag);
	return rtn;
}

void ConvertTAGV0ToV1(LPSTR TagFile)
{
	OFSTRUCTGM	OFStruct;
	int		nRead;
	HFILE			FidTag, FidOut;  
	typedef struct
{	
	int		Version;  
	char	Desc[34];
    BOOL	AutoEdit;
	RECT	rect;
	DPOINT	TAGPoint;
	POINT	TAGPointScr;
	POINT	ConnectPoint;
	POINT		RestorePoint;
	DPOINT		center; 
	int			ViewportID;
	int			CoordStyle;
	int			BGstyle;
	int			PLwidth;
	int			PLstyle;
	int			BorderStyle, Margin;
	float		TXheight;
	int			Just;
	int			inc;
	COLORREF	BGcolor;
	COLORREF	TXcolor;
	COLORREF	BorderColor;
	COLORREF	PointerColor;
	HBITMAP		before, after;
	double		bmWidthD,bmHeightD;
	int			bmWidth, bmHeight; 
	BOOL		Shadow;
	LOGFONT		LogFont;
	int			DataFileType; 
	HANDLE		hTAGDB;
	char		DataFile[128];
	char		SQL[256];
	char		text[256];   
	int			SymNum;
	char		Contents[10];
	long		Refno;
	HANDLE		hReport;
	
} TAGBOX_V0;

	TAGBOX_V0	TAGBoxV0;
	char	V0Name[MAX_PATH],NewName[144];
	LPSTR	lpDot;
    
    _fullpath(V0Name,TagFile,sizeof(V0Name)); 
    lpDot = _fstrchr(V0Name,'.');
    if (lpDot) *lpDot=0;
    _fstrcat (V0Name,".ib0"); 
	FidTag = GSSiOpenFile (TagFile,(LPOFSTRUCTGM)&OFStruct,OF_READ);
	GSSiGetTempFileName (0,"gm",0,(LPSTR)NewName); 
	FidOut = GSSiOpenFile (NewName,(LPOFSTRUCTGM)&OFStruct,OF_CREATE);
	if (FidTag<0) return;
	while (nRead = BigRead(FidTag,(HPSTR)&TAGBoxV0,sizeof(TAGBOX_V0)))
	{  
		_fmemmove(&TAGBox,&TAGBoxV0,sizeof(TAGBoxV0));
		_fstrcpy(TAGBox.text,TAGBoxV0.text); 
		TAGBox.Version = 1;
		BigWrite(FidOut,(HPSTR)&TAGBox,sizeof(TAGBOX),-1);   
	}
	GSSiClose(FidTag); 
	GSSiClose(FidOut); 
	GSSiRename(TagFile,V0Name);
	copyfile (TagFile,NewName,FALSE,0,0,0,0,0,0);
	GSSiRemove (NewName);
	return;
}







float GetFontWtoH (HDC hDC, LPLOGFONT lpFont)
{
	float	ratio=1, Width, Height;  
	HFONT	Font, OldFont;
	char	TestText[]="ABCDEF abcdef GHIJKL ghijkl ";
	SIZE	txSize;
	
	lpFont->lfOrientation = lpFont->lfEscapement;
    Font = CreateFontIndirect(lpFont); 
    if (Font)
    {
	    SaveDC (hDC);
	    OldFont = SelectObject(hDC, Font);
		GetTextExtentPoint32 (hDC,TestText,_fstrlen(TestText),&txSize);
		Width = (float) txSize.cx/28;
		Height = txSize.cy;  
		if (Height)
			ratio = Width/Height;
	    SelectObject(hDC, OldFont);
	    DeleteObject(Font);
	    RestoreDC (hDC,-1);
	}
	
	return ratio;
} 
/*
void SaveInfoBoxes (HFILE Fid)
{
	OFSTRUCTGM	OFStruct;
	int		nRead;
	HFILE	FidTag; 
	TAGBOX	TAGBox;
 	short	Length, Version=1, id=OB_SAVEINFOBOX;

	if (!TagFile[0]) return;
	FidTag = GSSiOpenFile (TagFile,(LPOFSTRUCTGM)&OFStruct,OF_READ);
	if (FidTag == HFILE_ERROR) return;
 	BigWrite (Fid,&id,2);  
 	BigWrite (Fid,&Version,2);
 	BigWrite (Fid,&Version,2);
	while (BigRead(FidTag,&TAGBox,sizeof(TAGBOX)) ==  sizeof(TAGBOX))
		BigWrite(Fid,&TAGBox,sizeof(TAGBOX));
	GSSiClose(FidTag);
	return;
}  */
BOOL ProcessInfoboxMacro (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
{   
	POINT	MousePoint;
	short	InfoBoxID;
	short	VPID;
	
	//return FALSE;
	if (Message != WM_RBUTTONUP)
		return FALSE;
   	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
	if ((InfoBoxID = PtInInfoBoxRect (MousePoint,&VPID)))
	{
		ProcessInfoBoxPickMacroFile (InfoBoxID);
		return TRUE;
	}
	return FALSE;
}

void AddInfoBoxRect (LPRECT pRect,short IBNum,short VPID)
{   
	UINT	j,i;
	LPINFOBOXRECT	pIBRect, pIBRect2;
	
	if (ConfigLevel)
		return;	
	if (NumIBRect)
	{
		pIBRect = (LPINFOBOXRECT)GlobalLock (hIBRect); 
		for (i=0;i<NumIBRect;i++)
		{
			if (IBNum == pIBRect->ID)
			{
				if (!pRect)
				{   
					pIBRect2 = pIBRect++;
					for (j=i+1;j<NumIBRect;j++)
						*pIBRect2++ = *pIBRect++;
					NumIBRect--; 
					if (NumIBRect)
						GlobalUnlock (hIBRect);
					else
						GSSiGlobUlFree (&hIBRect);
					return;
				}
				pIBRect->Rect = *pRect;
				pIBRect->VPID = VPID;
				GlobalUnlock (hIBRect);
				return;
			}
		}
		GlobalUnlock (hIBRect);
	}
	else
	    hIBRect = GSSiGlobAlloc ( 387,GMEM_MOVEABLE,USHRT_MAX); 
	if (!pRect)
		return;
	pIBRect = (LPINFOBOXRECT)GlobalLock (hIBRect);
	pIBRect += NumIBRect;
	pIBRect->ID = IBNum;
	pIBRect->VPID = VPID; 
	pIBRect->Rect = *pRect; 
	NumIBRect++;
	GlobalUnlock (hIBRect);
	return;
}

void RemoveInfoBoxRectForVP(short VPID)
{   
	LPINFOBOXRECT	pIBRect, pIBRect2; 
	short	nIBRectNew=0;   
	HANDLE	hIBRectNew;

	if (!hIBRect || ConfigLevel)
		return;
	pIBRect = (LPINFOBOXRECT)GlobalLock (hIBRect); 
	hIBRectNew = GSSiGlobAlloc ( 388,GMEM_MOVEABLE,USHRT_MAX);
	pIBRect2 = (LPINFOBOXRECT)GlobalLock (hIBRectNew);  
	while (NumIBRect--)           
	{
		if (pIBRect->VPID != VPID)  
		{
			*pIBRect2++ = *pIBRect;
			nIBRectNew++;
		} 
		pIBRect++;
	} 
	GSSiGlobUlFree (&hIBRect);
	NumIBRect = nIBRectNew;
	if (NumIBRect)
	{ 
		GlobalUnlock (hIBRectNew);
		hIBRect = hIBRectNew;
	}
	else
		GSSiGlobUlFree (&hIBRectNew);
	return;
} 

void RemoveAllInfoBoxRect (void)
{   
	if (ConfigLevel)
		return;
	GSSiGlobFree (&hIBRect);
	NumIBRect = 0;
	return;
} 

short PtInInfoBoxRect (POINT Point,LPSHORT pVPID)
{
	LPINFOBOXRECT	pIBRect, pIBRect2;
	short	ID=0;   
	UINT	i;
	
	if (!hIBRect)
		return 0;
	pIBRect = (LPINFOBOXRECT)GlobalLock (hIBRect);    
	for (i=0;i<NumIBRect;i++,pIBRect++)
		if (PtInRect (&pIBRect->Rect,Point))
		{
			ID = pIBRect->ID; 
			*pVPID = pIBRect->VPID;
			break;
		}
	GlobalUnlock (hIBRect);
	return ID;
}

BOOL ProcessInfoBoxPickMacroFile (short InfoBoxNum)
{
	if (SelectTAG(InfoBoxNum-1))   
	{   
		TBNum = InfoBoxNum;
		SetGlobalValue ("%INFOBOXTEXT",TAGBox.text);
		ProcessText (TAGBox.PickMacroFile);    
		return TRUE;
	}  
	return FALSE;
}


