#include "graphint.h"

#include "gmextern.h"

static HPEN	UnHighlightPen;
static HPEN	HighlightPenHP;
static HPEN	UnHighlightPenHP;
static BOOL	HaveBasePens;
static HPEN	hProPen[2][8]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static short	LastHalfTone=0;
static short	NewObOpen=0;  
static COLORREF	RandColors[256];
static long	NumRequests=0;
static LPPICKDATA	pPickList;    
static char	Applications[3][12]={"LakeMaster","SportMap","GeoMaster"};

#define	MAX_GRIDS	18
typedef struct {long GridID,GridMinX,GridMinY,GridMaxX,GridMaxY,GridWidth,GridHeight,nGridCol,nGridRow;
				double	MetersPerPixel;} GRIDDEF;
static	GRIDDEF	GridDefs[MAX_GRIDS];

#define	MAX_RECS_IN_BLOCK	64
#define MAX_RECS_IN_INDEX	32
typedef	struct	{long GridID,GridCellID;} GRIDCELLDEF;
typedef struct	{long GridCellID, loc;} GRIDINDEXREC;
typedef struct	{long TileWidth,TileHeight,NumPerIndexRec,NumIndexLevs,FirstIndexLoc,NumRecs;} GRIDFILEHEADER;



short GenList (HWND hWndDlg,UINT CntlID,BOOL ComboBox,long Loc,LPSTR Args,LPSTR CurrentValue,LPBOOL pUseBAR,LPINT MaxLen,LPINT NumItems)
#if ENABLETRACE
{GSSiEnterProg (1135);
#endif
{
	WORD Num=0, Inc;
	long	BegVal,IncVal; 
	DWORD	CBData;
	short	Item, DefaultItem=-1;  
	char	str[1024];
	HANDLE	hMem;
	LPSTR	ParLoc, pEnd, pBAR,Arg1, Arg2, Arg3, Arg4;
	
	Args+=9;
	if (!(pEnd = _fstrrchr (Args,')')))
{
#if ENABLETRACE
GSSiExitProg (1135);
#endif
		return 0;  
}
	*pEnd = 0;
	hMem =  GSSiGlobAlloc (1077,GMEM_MOVEABLE,2048*4);
	Arg1 = GlobalLock (hMem);
	Arg2 = Arg1 + 2048;
	Arg3 = Arg2 + 2048;
	Arg4 = Arg3 + 2048;
	if (!(ParLoc = MatchLev (Args,','))) goto Exit;
	_fstrcpy (Arg2,(LPSTR)(ParLoc+1));
	*ParLoc = '\0';
	_fstrcpy (Arg1,Args);
	if (!(ParLoc = MatchLev (Arg2,','))) goto Exit;
	_fstrcpy (Arg3,(LPSTR)(ParLoc+1));
	*ParLoc = '\0';
	if (!(ParLoc = MatchLev (Arg3,','))) goto Exit;
	_fstrcpy (Arg4,(LPSTR)(ParLoc+1));
	*ParLoc = '\0';
	ExpandText (Arg1);
	ExpandText (Arg2);
	ExpandText (Arg3);
	Num = atol (Arg1);
	BegVal = atol (Arg2);
	IncVal = atol (Arg3);
	for (Inc =0;Inc<Num;Inc++,BegVal+=IncVal)
	{	
		SetGlobalValueLong ("%GENLISTVAL",BegVal);
		_fstrcpy (str,Arg4);
		ExpandText (str);
		if ((pBAR = _fstrchr (str,'|')))
		{   
			LPSTR	lchr=LastChr (pBAR);
			
			if (ComboBox)
				*pBAR++ = 0;
			else
				*pBAR++ = '\t';
			if (*lchr == ';')
				*lchr = 0;
			*pUseBAR = TRUE;	
		}
 		CBData = MAKELPARAM ((WORD)Loc,Inc+1); 
 		if (ComboBox)
 		{
	        Item = SendDlgItemMessage (hWndDlg,CntlID,CB_ADDSTRING,0,(LPARAM)str); 
	     	SendDlgItemMessage (hWndDlg,CntlID,CB_SETITEMDATA,(WPARAM)Item,(LPARAM)CBData); 
	     	if (!_fstricmp (pBAR,CurrentValue))
	     		DefaultItem = Item;
	    }
	    else
	        Item = SendDlgItemMessage (hWndDlg,CntlID,LB_ADDSTRING,0,(LPARAM)str); 
		if (NumItems)
			(*NumItems)++;
		if (MaxLen)
		{
			if ((pBAR = strchr (str,'\t')))
				(*MaxLen) = max (*MaxLen,pBAR-str);
			else
				(*MaxLen) = max (*MaxLen,strlen (str));
		}
	}
Exit:
	GSSiGlobUlFree (&hMem);
{
#if ENABLETRACE
GSSiExitProg (1135);
#endif
	return DefaultItem;
}
#if ENABLETRACE
}
#endif
}

BOOL GetGenListVal (LPSTR Args,int Inc,LPSTR OutString)
#if ENABLETRACE
{GSSiEnterProg (1136);
#endif
{
	char	str[1024];
	long	BegVal,IncVal; 
	HANDLE	hMem=0;
	LPSTR	ParLoc, pEnd, pBAR,Arg1, Arg2, Arg3, Arg4, pLast;
	
	Args+=9;
	if (!(pEnd = _fstrrchr (Args,')')))
{
#if ENABLETRACE
GSSiExitProg (1136);
#endif
		return 0;  
}
	*pEnd = 0;
	hMem =  GSSiGlobAlloc (1078,GMEM_MOVEABLE,2048*4);
	Arg1 = GlobalLock (hMem);
	Arg2 = Arg1 + 2048;
	Arg3 = Arg2 + 2048;
	Arg4 = Arg3 + 2048;
	if (!(ParLoc = MatchLev (Args,','))) goto Exit;
	_fstrcpy (Arg2,(LPSTR)(ParLoc+1));
	*ParLoc = '\0';
	_fstrcpy (Arg1,Args);
	if (!(ParLoc = MatchLev (Arg2,','))) goto Exit;
	_fstrcpy (Arg3,(LPSTR)(ParLoc+1));
	*ParLoc = '\0';
	if (!(ParLoc = MatchLev (Arg3,','))) goto Exit;
	_fstrcpy (Arg4,(LPSTR)(ParLoc+1));
	*ParLoc = '\0';
	ExpandText (Arg1);
	ExpandText (Arg2);
	ExpandText (Arg3);
	BegVal = atol (Arg2);
	IncVal = atol (Arg3);  
	BegVal += IncVal * Inc;
	SetGlobalValueLong ("%GENLISTVAL",BegVal);
	_fstrcpy (str,Arg4);
	ExpandText (str);
	if ((pBAR = _fstrchr (str,'|')))
		*pBAR++ = 0;
	else
		pBAR = str;
	pLast = LastChr (pBAR);
	if (*pLast == ';')
		*pLast = 0;
	_fstrcpy (OutString,pBAR);
Exit:
	GSSiGlobUlFree (&hMem);
{
#if ENABLETRACE
GSSiExitProg (1136);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

void SetLineHighlight(HDC hDC, short mode)
#if ENABLETRACE
{GSSiEnterProg (88);
#endif
{    
    BOOL	HiPrecis2 = HiPrecis;

	if (CurView->Rotation)
		HiPrecis2 = TRUE;
    if (!Display || Pick)
{
#if ENABLETRACE
GSSiExitProg (88);
#endif
    	return;  
}   
	OpenBasePens ();
    if (HiPrecis2)
    {
	    if (mode == 2)
	        SelectObject(hDC, UnHighlightPenHP);
	    else
	        SelectObject(hDC, HighlightPenHP); 
	}
	else
	{
	    if (mode == 2)
	        SelectObject(hDC, UnHighlightPen);
	    else
	        SelectObject(hDC, HighlightPen);
	}
{
#if ENABLETRACE
GSSiExitProg (88);
#endif
    return;
}
#if ENABLETRACE
}
#endif
} 

void SetAreaHighlight(HDC hDC)
#if ENABLETRACE
{GSSiEnterProg (89);
#endif
{
    SetBkMode (hDC,TRANSPARENT);  
    SelectObject(hDC, HighlightBrush);
    if (HighlightWidth < 0) 
    {
    	if (nPoly > 1)
	    	SelectObject(hDC,GetStockObject(NULL_PEN)); 
//    	SelectObject(hDC,GetStockObject(NULL_BRUSH)); 
	}
//    SetTextColor (hDC,RGB(255,0,0));
//    SetBkMode (hDC,TRANSPARENT);
{
#if ENABLETRACE
GSSiExitProg (89);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}

BOOL OpenBasePens(void)
#if ENABLETRACE
{GSSiEnterProg (73);
#endif
{   LOGBRUSH    NDB; 
	COLORREF	Color;
    short i, Width; 
    double	fac=DeviceToScreenFactor;
	BOOL	rtn=FALSE;
    
    if (!HaveBasePens && Display)
    {   
		if (!CurVis || !CurVis->WantType[9])
			fac = 0;
    	hRedBrush =   CreateSolidBrush(RGB(255,   0,   0));
        hGreenBrush = CreateSolidBrush(RGB(  0, 255,   0));
        hBlueBrush =  CreateSolidBrush(RGB(  0,   0, 255));  
        Color = GetGlobalLVal2 ("[%AREABORDERCOLOR]",0);   
        Color = ConvertColor (Color,-1);
        Width = GetGlobalLVal2 ("[%AREABORDERWIDTH]",-1);
		if (Width >= 0)
		{
    		hAreaBorderPen[FALSE] = CreatePen(PS_SOLID,(int)IDNINT(Width*fac * PenWidthFactor*WidthFactor),Color);
    		hAreaBorderPen[TRUE] = CreatePen(PS_SOLID,(int)IDNINT(Width*fac * PenWidthFactor),Color);
		}
		else
		{
    		hAreaBorderPen[FALSE] = 0;
    		hAreaBorderPen[TRUE] = 0;
		}
        h0Pen = GetStockObject (NULL_PEN);
        hRedPen =   CreatePen(PS_SOLID,0,RGB(255,0,0));
        if (*PenWIDTH < 0)
        	Width = abs (*PenWIDTH) * fac  * PenWidthFactor* BaseDistToWinDist;
        else if (*PenWIDTH > 0) 
        	Width = *PenWIDTH * fac * PenWidthFactor;
        else
        	Width = fac * PenWidthFactor;
        if (*PenCOLOR >= 0) 
        	Color = ConvertColor(*PenCOLOR,-1);
        else
        	Color = ConvertColor(0,-1);
        if (PeopleNet) //temp fix for pn - should have 2 dim pens array for hi and low precision 7/30/03  
        	pens[0] =   CreatePen(PS_SOLID,(int)IDNINT(WidthFactor*Width),Color);
        else   
        	pens[0] =   CreatePen(PS_SOLID,Width,Color);   
        if (HighlightWidth < 0)
	        HighlightBrush =   CreatePen(PS_SOLID,
    	                       (int) labs(IDNINT(HighlightWidth*fac * PenWidthFactor)),
        	                   HighlightColor);
        else if (PatternBrush || Printing)
        {   
        	HBITMAP hbmp;
            NDB.lbStyle = BS_HATCHED;
            NDB.lbColor = HighlightColor;
            NDB.lbHatch = HS_DIAGCROSS;
            HighlightBrush =  CreateBrushIndirect(&NDB);
/*			hbmp = LoadBitmap(hInst, IDB_MONO1); does'nt work - no transparent fills
			HighlightBrush = CreatePatternBrush(hbmp);
            DeleteObject (hbmp);*/
        }
        else
            HighlightBrush =   CreateSolidBrush(HighlightColor);
        HighlightPen =   CreatePen(PS_SOLID,
                           (int) labs(IDNINT(HighlightWidth*WidthFactor * PenWidthFactor)),
                           HighlightColor);
        UnHighlightPen =   CreatePen(PS_SOLID,
                           (int) labs(IDNINT(HighlightWidth*WidthFactor * PenWidthFactor)),
                           CurView->BackGroundColor);
		if (HighlightWidth >= 0)
		{
			HighlightPenHP =   CreatePen(PS_SOLID,(int)labs(IDNINT(HighlightWidth * PenWidthFactor)),HighlightColor);
			UnHighlightPenHP =   CreatePen(PS_SOLID,(int)labs(IDNINT(HighlightWidth * PenWidthFactor)),ConvertColor(CurView->BackGroundColor,0));
		}
		else
		{
			int w = IDNINT(-HighlightWidth * BaseDistToWinDist);

			HighlightPenHP =   CreatePen(PS_SOLID,w,HighlightColor);
			UnHighlightPenHP =   CreatePen(PS_SOLID,w,ConvertColor(CurView->BackGroundColor,0));
		}

        CurrentPen = hRedPen;
        for (i=1;i<8;i++)
        	if (PenWIDTH[i] != 0 )
        	{
        		hProPen[TRUE][i]= CreatePen(PS_SOLID,(int) IDNINT(PenWIDTH[i]*fac * PenWidthFactor), ConvertColor(PenCOLOR[i],-1));
        		hProPen[FALSE][i]= CreatePen(PS_SOLID,(int) IDNINT(PenWIDTH[i]*WidthFactor*fac * PenWidthFactor), ConvertColor(PenCOLOR[i],-1));
        	}
		CreateClassPens (); 
		//DestroyRandomBrushes ();                                      
		CreateRandomBrushes (CurView,0);
	    HaveBasePens = TRUE;
		rtn = TRUE;
    }
{
#if ENABLETRACE
GSSiExitProg (73);
#endif
    return rtn;
}
#if ENABLETRACE
}
#endif
}

void CloseBasePens(BOOL DoClose)
#if ENABLETRACE
{GSSiEnterProg (74);
#endif
{   
	short i;
    
    if (CurView)
    {
		SelectObject (CurView->hDC,GetStockObject(NULL_PEN));
		SelectObject (CurView->hDC,GetStockObject(NULL_BRUSH)); 
	}
	
	if (DoClose)
	{
		if (HaveBasePens)
		{
			GSSiDeleteObject (&hRedBrush);
			GSSiDeleteObject (&hGreenBrush);
			GSSiDeleteObject (&hBlueBrush);
			//GSSiDeleteObject (&h0Pen);
			GSSiDeleteObject (&hRedPen);
			GSSiDeleteObject (&HighlightBrush);
			GSSiDeleteObject (&HighlightPen);
			GSSiDeleteObject (&UnHighlightPen);
			GSSiDeleteObject (&HighlightPenHP);
			GSSiDeleteObject (&UnHighlightPenHP);
			GSSiDeleteObject (&hAreaBorderPen[0]);
			GSSiDeleteObject (&hAreaBorderPen[1]);
			GSSiDeleteObject (&pens[0]); 
			for (i=1;i<8;i++)
			{
        		GSSiDeleteObject (&hProPen[TRUE][i]);
        		GSSiDeleteObject (&hProPen[FALSE][i]);
       		}
		} 
		DestroyClassPens();  
		CurrentPen = 0;
		HaveBasePens = FALSE;
	}
{
#if ENABLETRACE
GSSiExitProg (74);
#endif
    return;
}

#if ENABLETRACE
}
#endif
}  

COLORREF COLORREFFromRGBQUAD (RGBQUAD rgbq)
{
	return (RGB(rgbq.rgbRed,rgbq.rgbGreen,rgbq.rgbBlue));
} 

COLORREF COLORREFFromRGBTRIPLE (RGBTRIPLE rgbt)
{
	return (RGB(rgbt.rgbtRed,rgbt.rgbtGreen,rgbt.rgbtBlue));
} 

RGBQUAD RGBQUADFromCOLORREF (COLORREF Color)
{
	RGBQUAD c;
	
	c.rgbBlue = GetBValue (Color);
	c.rgbRed = GetRValue (Color);
	c.rgbGreen = GetGValue (Color);
	c.rgbReserved = 0;
	return c;
}

RGBTRIPLE RGBTRIPLEFromCOLORREF (COLORREF Color)
{
	RGBTRIPLE c;
	
	c.rgbtBlue = GetBValue (Color);
	c.rgbtRed = GetRValue (Color);
	c.rgbtGreen = GetGValue (Color);
	return c;
}

HANDLE LoadColorMap (LPSTR Name,LPSHORT pnPalColors)
{  
	HFILE	Fid = GSSiOpenFile (Name,0,OF_READ); 
	HANDLE	hMapColors;
	RGBQUAD	FAR * pMapColors;  
	COLORREF	MapColors[256];    
	USHORT	i;
		
	*pnPalColors = 0;
	if (Fid == HFILE_ERROR)
		return 0;
	hMapColors = GSSiGlobAlloc (0,GHND,256*sizeof(COLORREF));
    pMapColors = (RGBQUAD *)GlobalLock (hMapColors);
	BigRead (Fid,(HPSTR)pnPalColors,2);
	BigRead (Fid,(HPSTR)MapColors,256*sizeof(COLORREF)); 
	for (i=0;i<256;i++)
		pMapColors[i] = RGBQUADFromCOLORREF (MapColors[i]);
	GlobalUnlock (hMapColors);
	GSSiClose (Fid);
	return hMapColors;
}
     
BOOL AddColorToMap (long Color,LPSHORT pNumMapColors,HANDLE hMapColors)
{
	COLORREF	FAR * pMapColors=(COLORREF	FAR * )GlobalLock (hMapColors);   
	BOOL	new=FALSE;  
	USHORT	i;
	
	if (Color == -1)
		goto Found;
	if (*pNumMapColors >= 256)
		goto Found;
	for (i=0;i<*pNumMapColors;i++)
	{ 
		if (Color == pMapColors[i])
			goto Found;
	} 
	new = TRUE;
	pMapColors[*pNumMapColors] = Color;
	(*pNumMapColors)++;
Found:
	GlobalUnlock (hMapColors);
	return new;
}

BOOL AddSymbolColorsToMap (short idesc,LPSHORT pNumMapColors,HANDLE hMapColors)
{
	USHORT	i; 
	HANDLE	hSymbol = GetDictSymDesc (idesc,0);
    LPSYMBOL    pSym=(LPSYMBOL)GlobalLock (hSymbol); 
	LPHANDLE	phElement;  

	for (i=0,phElement=&pSym->hElement;i<pSym->NumElements;i++,phElement++)
	{ 
		LPELEMENT	pElement;
		
		pElement = (LPELEMENT)GlobalLock (*phElement);
		AddColorToMap (pElement->LineColor,pNumMapColors,hMapColors);
		AddColorToMap (pElement->FillColor,pNumMapColors,hMapColors);
		GlobalUnlock (*phElement);
	}
	GlobalUnlock (hSymbol);  
	DestroySymbol (hSymbol);
	return TRUE;
}

BOOL CreateColorMap (LPSTR Name)
{   
	HFILE	Fid2,Fid = GSSiOpenFile (Name,0,OF_CREATE);
	UINT	i;
	HANDLE	hMapColors;
	short	NumMapColors=0,idesc;   
	COLORREF	FAR * pMapColors, Color;    
	char	TempFile[256];
	char	str[260]; 
	
	if (Fid == HFILE_ERROR)
		return FALSE;     
	hMapColors = GSSiGlobAlloc (0,GHND,256*sizeof(COLORREF));
	AddColorToMap (CurView->BackGroundColor,&NumMapColors,hMapColors);
	CreateRandomBrushes (CurView,0);
	for (i=0;i<NumRandomColors;i++)
	{ 
		AddColorToMap (RandColors[i],&NumMapColors,hMapColors);
	}    
	GSSiGetTempFileName (0,"gm",0,TempFile);
	DumpVisibilityToFile (TempFile,FALSE,0);
	Fid2 = GSSiOpenFile (TempFile,0,OF_READ);
	fgetstring (str,256,Fid2);
	while (fgetstring (str,256,Fid2))
	{
		LPSTR	pSpace=_fstrrchr (str,'\t')-1;
		
		pSpace=_fstrchr (pSpace,'\t')+1;
		idesc = atoi (pSpace);
		AddSymbolColorsToMap (idesc,&NumMapColors,hMapColors);
	}
	GSSiClose (Fid2);
	GSSiRemove (TempFile);  
	OpenShields	 ();
	if (hShields)
	{
		LPSHIELDS pShields = (LPSHIELDS)GlobalLock (hShields);
		  
		for (i=0;i<nShields;i++,pShields++)
			AddSymbolColorsToMap (pShields->Symbol,&NumMapColors,hMapColors);
		GlobalUnlock (hShields);
	}
	for (i=0;i<10;i++)
		AddColorToMap (GlobalColors[i],&NumMapColors,hMapColors);
	for (i=0;i<8;i++) 
	{
		AddColorToMap (PenCOLOR[i],&NumMapColors,hMapColors);
		AddColorToMap (AreaCOLOR[i],&NumMapColors,hMapColors);
	} 
	Color =  GetGlobalLVal ("[CITY_TEXT_COLOR]");
	AddColorToMap (Color,&NumMapColors,hMapColors);
	Color =  GetGlobalLVal ("[STREET_TEXT_COLOR_1]");
	AddColorToMap (Color,&NumMapColors,hMapColors);
	Color =  GetGlobalLVal ("[STREET_TEXT_COLOR_2]");
	AddColorToMap (Color,&NumMapColors,hMapColors);
	Color =  GetGlobalLVal ("[STREET_TEXT_COLOR_3]");
	AddColorToMap (Color,&NumMapColors,hMapColors);
	Color =  GetGlobalLVal ("[STREET_TEXT_COLOR_4]");
	AddColorToMap (Color,&NumMapColors,hMapColors);
    pMapColors = (COLORREF	FAR * )GlobalLock (hMapColors);
	BigWrite (Fid,(HPSTR)&NumMapColors,2,-1);
	BigWrite (Fid,(HPSTR)pMapColors,256*sizeof(COLORREF),-1); 
	GSSiGlobUlFree (&hMapColors);
	GSSiClose (Fid);
	return TRUE;
}

BOOL AddColorToColorMapFile (LPSTR Name,COLORREF Color)
{
	return TRUE;
}

COLORREF GetRandColor (int icolor)
{
	return RandColors[icolor];
} 

void CreateRandomBrushes (LPVIEWPORT pVP,short UseHalfTone)
#if ENABLETRACE
{GSSiEnterProg (75);
#endif
{   
	static	LPVIEWPORT	LastVP=0;
	LOGBRUSH    NDB;
    LPHBRUSH	pRandBrush;
    short	i, j, Inc, PrimeColor[3], PrimeColorMin[3], PrimeColorMax[3];
    static	First=TRUE; 
    COLORREF	Color;
    short	MinColor = GetGlobalLVal2("[%RANDCOLORMIN]",184);
    short	MaxColor = min (255,GetGlobalLVal2("[%RANDCOLORMAX]",255));  
    short	ColorRange = MaxColor - MinColor;
    double	RedFactor = min (1.0,GetGlobalDVal2("[%RANDREDFACTOR]",1.0));
    double	GreenFactor = min (1.0,GetGlobalDVal2("[%RANDGREENFACTOR]",1.0));
    double	BlueFactor = min (1.0,GetGlobalDVal2("[%RANDBLUEFACTOR]",1.0));
    BOOL	SaveRandomAreasAreTransparent;   
    
    if (!CurVis)
{
#if ENABLETRACE
GSSiExitProg (75);
#endif
    	return;
}   
	 
    DestroyRandomBrushes (); 
    
    if ((long)pVP == 1)
    	pVP = LastVP;
    else
    	LastVP = pVP;
    if (pVP && pVP->NumRanColor)
    {
    	NumRandomColors = pVP->NumRanColor;
    	for (i=0;i<3;i++)
    	{	  
	    	PrimeColorMin[i] = (pVP->PrimeColorMin[i] * 255) / 100;
	    	PrimeColorMax[i] = (pVP->PrimeColorMax[i] * 255) / 100;  
		}    	
    }  
    else
    {
		NumRandomColors = GetGlobalLVal2 ("[%NUMRANCOLORS]",17);
		PrimeColorMin[0] = MinColor; 
		PrimeColorMax[0] = IDNINT(MinColor + ColorRange * RedFactor);  
		PrimeColorMin[1] = MinColor; 
		PrimeColorMax[1] = IDNINT(MinColor + ColorRange * GreenFactor);  
		PrimeColorMin[2] = MinColor; 
		PrimeColorMax[2] = IDNINT(MinColor + ColorRange * BlueFactor);
		if (pVP)
		{
	    	for (i=0;i<3;i++)
	    	{	  
		    	pVP->PrimeColorMin[i] = (PrimeColorMin[i] * 100) / 255;
		    	pVP->PrimeColorMax[i] = (PrimeColorMax[i] * 100) / 255;
			}    	
		}  
    }
    NumRandomColors = max (min (NumRandomColors,257),3);
    SaveRandomAreasAreTransparent = RandomAreasAreTransparent;  
    RandomAreasAreTransparent = FALSE;
    if (!hRandBrushes)
    { 
    	First = FALSE;
	    srand(1);
		hRandBrushes = GSSiGlobAlloc (  55,GMEM_MOVEABLE,NumRandomColors*(sizeof(HPEN)+sizeof(HBRUSH)));
		pRandBrush = (LPHBRUSH)GlobalLock (hRandBrushes); 
		for (i=0;i<NumRandomColors;i++,pRandBrush++)
		{   
/*			if (ColorRange)
		    	Color = RGB(IDNINT((MinColor+rand()%ColorRange)*RedFactor),
			                IDNINT((MinColor+rand()%ColorRange)*GreenFactor),
			                IDNINT((MinColor+rand()%ColorRange)*BlueFactor)); 
			else
				Color = RGB (MinColor,MinColor,MinColor); */
			for (j=0;j<3;j++)
			{
				short Range = PrimeColorMax[j] - PrimeColorMin[j];
				
				if (Range)
					Inc = rand()%Range;
				else
					Inc = 0;
				PrimeColor[j] = PrimeColorMin[j] + Inc;
			}
			Color = ConvertColor(RGB(PrimeColor[0],PrimeColor[1],PrimeColor[2]),UseHalfTone); 
			RandColors[i] = Color;
		    *pRandBrush++ = CreatePen (PS_SOLID,0,Color); 
			if (SolidAreas && GetBit (7,(LPSTR)&CurVis->WantType[7]))
			{
				PATBYTE		PatByte;      
	
				PatByte.Transparent = 1;  
				PatByte.BGOpt = 0;
				PatByte.Pattern = 2; 
				if (RandomAreasAreTransparent)
					*pRandBrush = CreatePatBrush (0, Color, &PatByte);
				else
					*pRandBrush = CreateSolidBrush(Color);
			} 
			else
			{ 
				NDB.lbStyle = BS_HATCHED;
				NDB.lbColor = Color;
				NDB.lbHatch	= HS_DIAGCROSS;
				*pRandBrush = CreateBrushIndirect(&NDB);
	        }
	    }  
	    GlobalUnlock (hRandBrushes); 
    }  
    RandomAreasAreTransparent = SaveRandomAreasAreTransparent;
{
#if ENABLETRACE
GSSiExitProg (75);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}

void DestroyRandomBrushes (void)
#if ENABLETRACE
{GSSiEnterProg (76);
#endif
{
    LPHBRUSH	pRandBrush;
    short	i; 
    
    LastHalfTone=0;

    if (!hRandBrushes)
{
#if ENABLETRACE
GSSiExitProg (76);
#endif
    	return;
}
    if (CurView)
    {
		SelectObject (CurView->hDC,GetStockObject(NULL_PEN));
		SelectObject (CurView->hDC,GetStockObject(NULL_BRUSH)); 
	}
	
	pRandBrush = (LPHBRUSH)GlobalLock (hRandBrushes); 
	for (i=0;i<NumRandomColors;i++,pRandBrush++)  
	{
		GSSiDeleteObject (pRandBrush);
		pRandBrush++;
		GSSiDeleteObject (pRandBrush); 
	}
	GSSiGlobUlFree (&hRandBrushes);
{
#if ENABLETRACE
GSSiExitProg (76);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

long	RandLong (long Value, long Range)
#if ENABLETRACE
{GSSiEnterProg (77);
#endif
{   
	long	rtn, IntTot=0;
	unsigned	long	i;
	union {
			char	bytes[4];
		    long	InValue;
	      }	InUnion; 
	union {
			double	Rval;  
			short	ieq[4];
		  } OutUnion;
			
    InUnion.InValue = Value;
    for (i=0;i<4;i++)
    	IntTot += InUnion.bytes[i] * (i+1);
    OutUnion.Rval = 1907719e0 * IntTot + 907633963e0;
 //     IHRAN = MOD (IABS((long)(IEQ[1])*(long)(IEQ[2])*(long)(IEQ[3])),MAX) + 1;  
 	rtn = (long)OutUnion.ieq[0] + (long)OutUnion.ieq[1] + (long)OutUnion.ieq[2] + (long)OutUnion.ieq[3];
 	rtn = labs (rtn) % Range;
{
#if ENABLETRACE
GSSiExitProg (77);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}  

HBRUSH SelectRandomBrush (HDC hDC,long Refno,HPEN *phSavePen,short UseHalfTone)
#if ENABLETRACE
{GSSiEnterProg (78);
#endif
{   
	LPHBRUSH	pRandBrushes; 
	HBRUSH	hOldBrush; 
	short	icolor;
	BOOL	DoHalfTone;  
	
	if (!UseHalfTone)
		DoHalfTone = FALSE;
	else if (ForceHalfTone)
		DoHalfTone = TRUE;
	else if (UseHalfTone < 0)
		DoHalfTone = TRUE;
	else
		DoHalfTone = GetHalfToneVisibility (UseHalfTone);
	if (DoHalfTone != LastHalfTone)
	{
		CreateRandomBrushes ((LPVIEWPORT)1,UseHalfTone);
		LastHalfTone = DoHalfTone;
	}
	icolor = RandLong (Refno,NumRandomColors);
	pRandBrushes = (LPHBRUSH)GlobalLock (hRandBrushes);
	pRandBrushes += (2*icolor);
	if (phSavePen && !*phSavePen)
		*phSavePen = *pRandBrushes;
	pRandBrushes++;
	if (!(SolidAreas && GetBit (7,(LPSTR)&CurVis->WantType[7])))// || !hRandBrushes)
		hOldBrush = GetStockObject (NULL_BRUSH); 
	else
		hOldBrush = SelectObject (hDC,*pRandBrushes);
	GlobalUnlock (hRandBrushes);  
	if (RandomAreasAreTransparent) 
	{
		SetROP2(hDC,R2_MASKPEN);  
	    SetTextColor (hDC,RandColors[icolor]);     
	    SetBkColor (hDC,RGB(255,255,255)); 
	}
//		SetROP2(hDC,R2_MASKPEN);  
Exit:
{
#if ENABLETRACE
GSSiExitProg (78);
#endif
	return hOldBrush;
}
#if ENABLETRACE
}
#endif
}

void OpenNewObjects(void)
#if ENABLETRACE
{GSSiEnterProg (79);
#endif
{   short  i;
    BOOL	HiPrecis2 = HiPrecis;

	if (CurView->Rotation)
		HiPrecis2 = TRUE;
    if (!Display)
{
#if ENABLETRACE
GSSiExitProg (79);
#endif
    	return;
}
    if (HiPrecis2 && NewObOpen == 2) 
{
#if ENABLETRACE
GSSiExitProg (79);
#endif
    	return;
}
    if (!HiPrecis2 && NewObOpen == 1) 
{
#if ENABLETRACE
GSSiExitProg (79);
#endif
    	return;
}
    if (NewObOpen)
    	CloseNewObjects();
    if (HiPrecis2)
    	NewObOpen = 2;
    else
    	NewObOpen = 1;
    for (i=0;i<min(MAX_NEW_OBJECTS,CurView->NumNewObjects);i++)
    	CreateNewObject(i);
{
#if ENABLETRACE
GSSiExitProg (79);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}                    

void CreateNewObject(int i)
#if ENABLETRACE
{GSSiEnterProg (80);
#endif
{
    LOGBRUSH	NDB;
    double		WF=WidthFactor;
    double	fac=DeviceToScreenFactor; 
    short	UseHalfTone=0;
    BOOL	HiPrecis2 = HiPrecis;

	if (CurView->Rotation || UseShortSymbols)
		HiPrecis2 = TRUE;
    
    if (CurView->AlwaysUseHalfTone || i < CurView->HalfToneNewObjectStart)
    	UseHalfTone = -1;
    
	if (!CurVis || !CurVis->WantType[9])
		fac = 0;
    
    if (HiPrecis2)
    	WF = 1.0; 

    if (CurView->NewObject[i].Type==3)
    {   
    	BOOL	WantXHatch=FALSE;
    	
    	if (CurVis)
    		WantXHatch = GetBit (6, (LPSTR)&CurVis->WantType[7]);
		if (SolidAreas && !WantXHatch)
        	CurView->NewObject[i].Handle = CreateGMBrush (RGBW(CurView->NewObject[i].R,CurView->NewObject[i].G,
        													   CurView->NewObject[i].B,IDNINT(CurView->NewObject[i].Width)),
        													   UseHalfTone,0);
		else
		{ 
			NDB.lbStyle = BS_HATCHED;
			NDB.lbColor = ConvertColor(RGB(CurView->NewObject[i].R,CurView->NewObject[i].G,CurView->NewObject[i].B),UseHalfTone);
			NDB.lbHatch	= HS_DIAGCROSS;
			CurView->NewObject[i].Handle = CreateBrushIndirect(&NDB);
        }
		CurView->NewObject[i].ProPen = 0;                                     
	}
    else if (CurView->NewObject[i].Type>0)
    {
    	short IWidth;
    	
    	if (CurView->NewObject[i].ProPen)
    	{   
    		if (hProPen[HiPrecis2][CurView->NewObject[i].ProPen])  
    			CurView->NewObject[i].Handle = hProPen[HiPrecis2][CurView->NewObject[i].ProPen];
{
#if ENABLETRACE
GSSiExitProg (80);
#endif
    		return;
}
    	}
    	if (CurView->NewObject[i].Width<0)
        	IWidth = (int)IDNINT(((double)-CurView->NewObject[i].Width / CurView->BaseUnitsPerPixel)*WF); 
//    		IWidth = (int)IDNINT((-CurView->NewObject[i].Width)/(CurView->FileFactor*FileDistToBaseDist));
    	else 
    		IWidth = IDNINT((CurView->NewObject[i].Width+1) * WF * fac * PenWidthFactor);
        CurView->NewObject[i].Handle=CreatePen (CurView->NewObject[i].Style,IWidth,
                                                ConvertColor(RGB(CurView->NewObject[i].R,
                                                	CurView->NewObject[i].G,
                                                	CurView->NewObject[i].B),UseHalfTone));
    }
{
#if ENABLETRACE
GSSiExitProg (80);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}

void CloseNewObjects(void)
#if ENABLETRACE
{GSSiEnterProg (81);
#endif
{   short i;
    
    NewObOpen = 0;
    if (!CurView)
{
#if ENABLETRACE
GSSiExitProg (81);
#endif
    	return;
}
    for (i=0;i<min(MAX_NEW_OBJECTS,CurView->NumNewObjects);i++) 
    {
        if(CurView->NewObject[i].Handle && !CurView->NewObject[i].ProPen)
            DeleteObject (CurView->NewObject[i].Handle);
        CurView->NewObject[i].Handle = 0;
    }
{
#if ENABLETRACE
GSSiExitProg (81);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}

BOOL UseFullScreen (HWND hWnd,LPRECT pRect)
{   
	RECT	Rect;
	
	if (pViewportsD[0]->Type || !pViewportsD[0]->AutoSize)
		return FALSE; 
	if (pViewportsD[0]->WidthType == 1)
	{  
		double	w,h; 
		HDC	hDC=GetDC (hWnd);
		int hres = GetDeviceCaps(hDC,HORZRES);
		double hsize = GetDeviceCaps(hDC, HORZSIZE) * MFT/100;
							
		if (pRect)
		{
			w = RECTWIDTH(pRect) * hsize/hres;
			h = RECTHEIGHT(pRect) * hsize/hres;
		}
		else
		{
			w = GetGlobalDVal2 ("[%SCREENWIDTH]",hsize); 
			if (IsRectEmpty (&ConfigDisplayRect))
				GetClientRect (hWnd,&Rect);
			else 
				Rect =  ConfigDisplayRect; 
			w *= (Rect.right - Rect.left) / (double)hres;   
			h = (w * (Rect.bottom-Rect.top)) / (Rect.right - Rect.left);
		}
		pViewportsD[0]->DesiredWidth = w;
		pViewportsD[0]->DesiredHeight = h;  
		ReleaseDC (hWnd,hDC);
	}
	else
	{  
		pViewportsD[0]->TagPoint.x = 0;
		pViewportsD[0]->TagPoint.y = 0;
		pViewportsD[0]->Width = 100;
		pViewportsD[0]->Height = 100;
	}
	PrinterHeight = 0; 
	PrinterFormatPixelWidth = 0; 
	InPlotView = 0;
	CheckMenuItem(GetMenu(hWnd), IDM_PLOTVIEW, MF_BYCOMMAND | MF_UNCHECKED);
	GMEnableMenuItem(hWnd, IDM_PLOTVIEW, MF_BYCOMMAND | MF_DISABLED| MF_GRAYED);
	DrawMenuBar (hWnd);
    return TRUE;
} 

BOOL SaveMemMap (void)
{   
	char	mess[144];
	MNMXCORD MnMx;
	double	Res; 
	POINT	WinPoint[2]; 
	HDIB32	hDib32, hDib8; 
	HBITMAP	hTempBM, hOldBM; 
	LPSTR	pDot=_fstrrchr (MemMapName,'.');
	DPOINT	BitmapPoint, WorldPoint;
	double	ScaleX, ScaleY, WorldWidth, WorldHeight;  
								
	SetViewport (*pCommandViewport);
	WinPoint[0].x = WinPoint[0].y = 0;	
	WinPoint[1].x = MemMapWidth-1;
	WinPoint[1].y = MemMapHeight-1;
	*(LPDPOINT)&MnMx = WinPtToBasePt (WinPoint[0]);
	*(LPDPOINT)&MnMx.xmx = WinPtToBasePt (WinPoint[1]);
	Res = ldistp (*(LPDPOINT)&MnMx,*(LPDPOINT)&MnMx.xmx) / idist (WinPoint[0],WinPoint[1]);
	if (GetGlobalBVal2 ("[%CREATEWORLDFILE]",FALSE))
		CreateBPW (MemMapName,MnMx,Res);
	if (GetGlobalBVal2 ("[%CREATELATLONTRANFILE]",FALSE))
		CreateLLTranFile (MemMapName);
/*		hDIB = BitmapToDIB (hMemBitmap, 0);
	SaveDIB (hDIB,MemMapName);
	DestroyDIB (hDIB); */
	hTempBM = CreateCompatibleBitmap (hdcMemMap,10,10);    
	hMemBitmap = SelectObject (hdcMemMap,hTempBM);
	hDib32 = BitmapToDIB32 (hMemBitmap);  
	hOldBM = SelectObject (hdcMemMap,hMemBitmap);
	GSSiDeleteObject(&hTempBM);
	
	BitmapPoint.x = BitmapPoint.y = 0;
	WorldWidth = CurView->WBounds.xmx - CurView->WBounds.xmn;   
	WorldHeight = CurView->WBounds.ymx - CurView->WBounds.ymn;
	ScaleX = WorldWidth/MemMapWidth;
	ScaleY = WorldHeight/MemMapHeight;
	WorldPoint.x = CurView->WBounds.xmn;
	WorldPoint.y = CurView->WBounds.ymx;  

//  		SetGeoTiffData (hDib,&ScaleX,&ScaleY,&BitmapPoint,&WorldPoint);  
//MessageBox (0,"saving",0,MB_OK);           	
	_fstrcpy (mess,MemMapName); 
	if (MemMapRotation)
		hDib32 = GMRotateImageClassic (hDib32,MemMapRotation);
	if (hMemMapColorMap)
    {
    	BITMAPINFOHEADER DibInfo;  
    	RGBQUAD	FAR	*Pallet;
    	HANDLE	hPallet;
    	LPSTR	pName=GlobalLock (hMemMapColorMap);    
    	short	nPalColors;
	    	
    	hPallet = LoadColorMap (pName,&nPalColors); 
    	GlobalUnlock (hMemMapColorMap);    
    	if (hPallet)    
    	{
	    	Pallet = (RGBQUAD	FAR	*)GlobalLock (hPallet);
			GetBitmapInfoFromHandle (&DibInfo,hDib32);
			hDib8 = QuantizeDibEx (hDib32,0,256,nPalColors,Pallet);    
			GMDestroyDIB32 (hDib32);
			GSSiGlobUlFree (&hPallet); 
		}
		else
			hDib8 = hDib32;
	}
	else
		hDib8 = hDib32; 
	SetGeoTiffData (hDib8,&ScaleX,&ScaleY,&BitmapPoint,&WorldPoint);  
	if (_fstricmp (pDot,".gmd"))
		SaveDIB32 (hDib8,MemMapName,-1,0);
	else
	{   
		long	sizeMM;
		HFILE	FidMM;
		HANDLE	hDB;
   			
		_fstrcpy (pDot,".png");
		SaveDIB32 (hDib8,MemMapName,FIF_PNG,0);  
		FidMM = GSSiOpenFile (MemMapName,0,OF_READ);
		sizeMM = GSSifilelength (FidMM);  
		_fstrcpy (pDot,".gmd");
		hDB = OpenGWDatabase (MemMapName,BT_WRITE);  
		if (hDB)
		{
		    LPGWDHEADER lpGWDHead = (LPGWDHEADER)GlobalLock (hDB);
		    LPMAPFILE   pMAPFILE = (LPMAPFILE)&lpGWDHead->GWDData;  
		    
		    sprintf (pDot,"%i.bin",MemMapSubDir);
   			{
				HANDLE	hMem=GSSiGlobAlloc (0,GMEM_MOVEABLE,sizeMM);
				HPSTR	pMem=GlobalLock (hMem);
				HFILE	FidBIN=GSSiOpenFile (MemMapName,0,OF_READWRITE);
				DWORD	loc;
				
				BigRead (FidMM,pMem,sizeMM);
				loc = GSSillseek2 (FidBIN,0,2);
				BigWrite (FidBIN,pMem,sizeMM,-1);
				GSSiClose (FidBIN);  
				GSSiGlobUlFree (&hMem);
				pMAPFILE->ID = MemMapID;   
				pMAPFILE->SubDir = MemMapSubDir;
				pMAPFILE->Size = sizeMM;
				pMAPFILE->Time = GetTickCount() - MemMapStartTime;     
				pMAPFILE->NumPoints = TotPointsProcessed;
				pMAPFILE->Loc = loc;      
				pMAPFILE->LLNormFactor = CurView->LLNormFactor;
				pMAPFILE->MetersPerDegree = MetersPerDegree; 
				pMAPFILE->GridID = MemMapGridID;
				pMAPFILE->GridCellID = MemMapGridCellID;
				pMAPFILE->Scale = CurView->Scale;
				pMAPFILE->Bounds = CurView->WBounds;
				pMAPFILE->Format = 0;
			}
		    GWDReplaceRecord (lpGWDHead,0,0,-1);
			GlobalUnlock (hDB);
		    CloseGWDatabase (hDB);   
		}
		GSSiClose (FidMM);
	}	
	GMDestroyDIB32 (hDib8);
	NumRequests++;
	//sprintf (mess,"%ld requests processed",NumRequests);
	//if (PeopleNet)
	//	SetWindowText (hWndMain,mess);  
	return TRUE;
}   

int MapImageToFile (LPSTR MapImageFile,long MapID,LPSTR OutFile,LPMNMXCORD pBounds,LPHANDLE pImageHandle)
{
	HANDLE	hDB = OpenGWDatabase (MapImageFile,BT_READ);  
	LPSTR	pDot = _fstrrchr (MapImageFile,'.'); 
	long	Offset;  
	int		rtn=FALSE;
	HFILE	FidOut;
	char	txt[32];
	
	if (hDB)
	{
	    LPGWDHEADER lpGWDHead = (LPGWDHEADER)GlobalLock (hDB);
	    LPMAPFILE   pMAPFILE = (LPMAPFILE)&lpGWDHead->GWDData;  
		    
        if (!BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&MapID,BT_FIRST,BT_EQ, (LPSTR)&Offset)) 
        {
        	FillGWDData (lpGWDHead,Offset); 
        	if (pBounds)
        		*pBounds = pMAPFILE->Bounds;
			itoa (pMAPFILE->SubDir,txt,10);
			SetWindowText (hWndMain,txt);
		    sprintf (pDot,"%i.bin",pMAPFILE->SubDir);
			{
				HFILE	FidBIN=GSSiOpenFile (MapImageFile,0,OF_READ);
				
				_fstrcpy (pDot,".gmd");	 
				if (FidBIN != HFILE_ERROR)
				{
					HANDLE	hMem=GSSiGlobAlloc (1588,GMEM_MOVEABLE,pMAPFILE->Size);
					HPSTR	pMem=GlobalLock (hMem);
					
					GSSillseek2 (FidBIN,pMAPFILE->Loc,0);
					BigRead (FidBIN,pMem,pMAPFILE->Size);
		//	pMAPFILE->Format = 1;
					switch (pMAPFILE->Format)
					{
					case 0:
						FidOut = GSSiOpenFile (OutFile,0,OF_CREATE);
						if (FidOut != HFILE_ERROR)
						{
							BigWrite (FidOut,pMem,pMAPFILE->Size,-1);
							GSSiClose (FidOut);  
							AddBMPToCache32 (0,0);
							rtn=TRUE;
	//            			GMEnableMenuItem(hWndMain, IDM_Z_ORTHO, MF_BYCOMMAND | MF_ENABLED);
						}
						rtn = 1;
						break;
					case 1:
						FidOut = GSSiOpenFile (OutFile,0,OF_CREATE);
						if (FidOut != HFILE_ERROR)
						{
							LPBITMAPINFOHEADER pbi = (LPBITMAPINFOHEADER)pMem;
							BITMAPFILEHEADER	FileHeader;
							char	BMType[3] = "BM";
							int	lbin;
							int	lMemDeCmp;
							RGBQUAD	Pallet[256];
				//			HANDLE	hMemCmp = GSSiGlobAlloc (1586,GMEM_MOVEABLE,pbi->biHeight * pbi->biWidth);
				//			HANDLE	hMemCmp = GSSiGlobAlloc (1586,GMEM_MOVEABLE,USHRT_MAX*32);
							LPSTR	pMemCmp = (LPSTR)malloc (USHRT_MAX*32);//GlobalLock (hMemCmp);
							HANDLE	hMem2;
							LPSTR	pMem2;
							int		ClrUsed;

/*							memmove (&FileHeader.bfType,BMType,2);
							FileHeader.bfReserved1 = FileHeader.bfReserved2 = 0;
							FileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 256*sizeof(RGBQUAD);
							FileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 256*sizeof(RGBQUAD) + pbi->biHeight * pbi->biWidth;
							BigWrite (FidOut,&FileHeader,sizeof(BITMAPFILEHEADER),-1);
							ClrUsed = pbi->biClrUsed;
							pbi->biClrUsed= pbi->biClrImportant = 256;
							BigWrite (FidOut,pbi,sizeof(BITMAPINFOHEADER),-1);
							pMem += sizeof(BITMAPINFOHEADER);
							memset (Pallet,0,sizeof(Pallet));
							memmove (Pallet,pMem,ClrUsed*sizeof(RGBQUAD));
							pMem += ClrUsed*sizeof(RGBQUAD);
							lbin = pMAPFILE->Size - (sizeof(BITMAPINFOHEADER) + ClrUsed*sizeof(RGBQUAD));
							BigWrite (FidOut,Pallet,pbi->biClrUsed*sizeof(RGBQUAD),-1);*/
							lbin = DecompressBinaryRecordUnsafe (pMemCmp,pMem,pMAPFILE->Size);
							WriteToHBird (pMem,pMAPFILE->Size);
						//	hMem2 = GSSiGlobAlloc (1587,GMEM_MOVEABLE,lbin);
							pMem2 = malloc (lbin);//GlobalLock (hMem2);
							memmove (pMem2,pMemCmp,lbin);
							lMemDeCmp = DeCompressByteArray (pMem2,pMemCmp,lbin);
							BigWrite (FidOut,pMemCmp,lMemDeCmp,-1);
							GSSiClose (FidOut);  
							free (pMem2);//GSSiGlobUlFree (&hMem2);
						//	GSSiGlobUlFree (&hMemCmp);
							free (pMemCmp);
							rtn = 1;
						}
						break;
					}
					GSSiClose (FidBIN);  
					GSSiGlobUlFree (&hMem);
				}
			} 
		}
		GlobalUnlock (hDB);
	    CloseGWDatabase (hDB);   
	}
	return rtn;
}

BOOL GetGridDef (LPSTR GridName)
{
	char	str[4096];
	LPSTR	pEnd, pLoc=str;
#include "GoogleDef.h"

	if (!GetGlobalCVal ("[%GRIDDEF]",str,0))
		return FALSE;
	if (!stricmp (str,"GoogleMaps"))
	{
		strcpy (str,GoogleGridDef);
		ExpandText (str);
	}
	while ((pEnd = strchr (pLoc,' ')))
	{
		*pEnd++ = 0;
		if (!stricmp (GridName,pLoc))
		{
			if (sscanf (pEnd,"%i %i %i %i %i %i",&GridMinX,&GridMinY,&GridMaxX,&GridMaxY,&GridWidth,&GridHeight) == 6)
			{
				nGridCol = (GridMaxX - GridMinX - 1) / GridWidth + 1;
				nGridRow = (GridMaxY - GridMinY - 1) / GridHeight + 1;
				return TRUE;
			}
			else
				return FALSE;
		}
		pLoc = strchr (pEnd,',');
		if (!pLoc)
			return FALSE;
		else
			pLoc++;
	}
	return FALSE;
}

int DGridCellID (LPDPOINT pPoint)
{
	int ID, Row, Col;

	Col = (int) ((pPoint->x + 1) - GridMinX) / GridWidth;
	Row = (int) ((pPoint->y + 1) - GridMinY) / GridHeight;
	if (Col < 0 || Col >= nGridCol || Row < 0 || Row >= nGridRow)
		return -1;
	ID = Row * nGridCol + Col;
	return ID;
}

BOOL SaveLayerDefFile (LPSTR OutFile,BOOL isFTCChip)
{
	BOOL	rtn=FALSE;
	char	str[4096], File[MAX_PATH];
	LPSTR	pBS;
	LPSTR	pEnd, pLoc=str;
	HFILE	FidOut;
	int	nGrids=0;
	int	ftcCode=9182012;

	if (!GetGlobalCVal ("[%GRIDDEF]",str,0))
		return FALSE;
	strcpy (File,OutFile);
	FidOut = GSSiOpenFile (File,0,OF_CREATE);
	BigWrite (FidOut,&nGrids,4,-1);
	while ((pEnd = strchr (pLoc,' ')))
	{
		{
			if (sscanf (pLoc,"%i %i %i %i %i %i %i",&GridDefs[nGrids].GridID,&GridDefs[nGrids].GridMinX,&GridDefs[nGrids].GridMinY,
													&GridDefs[nGrids].GridMaxX,&GridDefs[nGrids].GridMaxY,&GridDefs[nGrids].GridWidth,&GridDefs[nGrids].GridHeight) == 7)
			{
				GridDefs[nGrids].GridID /= 10;
				GridDefs[nGrids].nGridCol = (GridDefs[nGrids].GridMaxX - GridDefs[nGrids].GridMinX - 1) / GridDefs[nGrids].GridWidth + 1;
				GridDefs[nGrids].nGridRow = (GridDefs[nGrids].GridMaxY - GridDefs[nGrids].GridMinY - 1) / GridDefs[nGrids].GridHeight + 1;
				GridDefs[nGrids].MetersPerPixel = ((double)GridDefs[nGrids].GridWidth)/(GridPixelWidth/4);
				if (nGrids > 1 || GridDefs[0].GridID < 10)
					BigWrite (FidOut,&GridDefs[nGrids],sizeof(GRIDDEF),-1);
				nGrids++;
			}
			else
				goto Exit;
		}
		pLoc = strchr (pEnd,',');
		if (!pLoc)
			break;
		else
			pLoc++;
	}
	rtn = TRUE;
	ii=GSSillseek (FidOut,0,1);
	if (isFTCChip)
		BigWrite (FidOut,&ftcCode,4,-1);
	GSSillseek (FidOut,0,0);
	//nGrids -= 2;
	BigWrite (FidOut,&nGrids,4,-1);
Exit:
	GSSiClose (FidOut);
	return rtn;
}

BOOL MapImageExport (LPSTR MapImageFile,LPSTR Type,LPSTR OutFile,long GridID,LPMNMXCORL pFileBounds,BOOL isBaseMap)
{
	HANDLE	hDB = OpenGWDatabase (MapImageFile,BT_READ);  
	LPSTR	pDot = _fstrrchr (MapImageFile,'.'); 
	long	Offset;  
	int		rtn=FALSE;
	HFILE	FidOut;
	long	MapID, CurSubDir=-1;
	char	txt[32];
	GRIDFILEHEADER	Header;
	GRIDCELLDEF	Grid;
	GRIDINDEXREC	IndexRec, IndexRec2,BlockTerminator={0,-1};
	HFILE	FidBIN=HFILE_ERROR;
	HFILE	FidLev, FidLev2;
	char	TempFile[2][256];
	int		nOutRecs=0, nIndexLevs=0, loc, IndexLen, NumPerIndexRec=MAX_RECS_IN_INDEX;
	int		lev1, lev2;
	int		i, FirstIndex;
	char	GridName[16];

	if (!isBaseMap)
	{
		itoa (GridID,GridName,10);
		if (!GetGridDef (GridName))
			return FALSE;
	}

	MinMaxInitL (pFileBounds);
	//SaveLayerDefFile (OutFile);
	GSSiGetTempFileName (0,"gm",0,TempFile[0]);
	GSSiGetTempFileName (0,"gm",0,TempFile[1]);
	if (hDB)
	{
	    LPGWDHEADER lpGWDHead = (LPGWDHEADER)GlobalLock (hDB);
	    LPMAPFILE   pMAPFILE = (LPMAPFILE)&lpGWDHead->GWDData;  

		FidOut = GSSiOpenFile (OutFile,0,OF_CREATE);
		FidLev = GSSiOpenFile (TempFile[0],0,OF_CREATE);
		if (FidOut != HFILE_ERROR)
		{
			int	cond=BT_FIRST, pos=BT_GE;

			Grid.GridID = GridID;
			Grid.GridCellID = -1;
			while (!BT_FIND (lpGWDHead->BTHandle[1],(LPSTR)&Grid,cond,pos, (LPSTR)&Offset)) 
			{
				MNMXCORL	CellBounds;

				cond = BT_NEXT;
				pos = BT_ANY;
				if (Grid.GridID != GridID)
					break;
       			FillGWDData (lpGWDHead,Offset); 
				if (isBaseMap)
				{
					CellBounds.xmn = IDNINT (pMAPFILE->Bounds.xmn);
					CellBounds.xmx = IDNINT (pMAPFILE->Bounds.xmx);
					CellBounds.ymn = IDNINT (pMAPFILE->Bounds.ymn);
					CellBounds.ymx = IDNINT (pMAPFILE->Bounds.ymx);
				}
				else
					GetGridBounds (Grid.GridCellID,&CellBounds);
				AddMinMaxL (pFileBounds,&CellBounds);
				if (CurSubDir != pMAPFILE->SubDir)
				{
					GSSiClose (FidBIN);
					sprintf (pDot,"%i.bin",pMAPFILE->SubDir);
					FidBIN=GSSiOpenFile (MapImageFile,0,OF_READ);
					CurSubDir = pMAPFILE->SubDir;
					_fstrcpy (pDot,".gmd");	 
				}
				if (FidBIN != HFILE_ERROR)
				{
					HANDLE	hMem=GSSiGlobAlloc (1588,GMEM_MOVEABLE,pMAPFILE->Size);
					HPSTR	pMem=GlobalLock (hMem);
					
					GSSillseek2 (FidBIN,pMAPFILE->Loc,0);
					BigRead (FidBIN,pMem,pMAPFILE->Size);
					loc = GSSillseek (FidOut,0,1);
			//		if (nOutRecs++ % NumPerIndexRec == 0)
					{
						BigWrite (FidOut,&BlockTerminator,8,-1);
						IndexRec.GridCellID = pMAPFILE->GridCellID;
						IndexRec.loc = loc+8;
						BigWrite (FidLev,&IndexRec,8,-1);
						nOutRecs++;
					}
					BigWrite (FidOut,&pMAPFILE->GridCellID,4,-1);
					BigWrite (FidOut,&pMAPFILE->Size,4,-1);
					BigWrite (FidOut,pMem,pMAPFILE->Size,-1);
					GSSiGlobUlFree (&hMem);
				} 
			}
			BigWrite (FidOut,&BlockTerminator,8,-1);
		}
		GSSiClose (FidLev);
		do
		{
			lev1 = nIndexLevs % 2;
			nIndexLevs++;
			lev2 = nIndexLevs % 2;
			FidLev = GSSiOpenFile (TempFile[lev1],0,OF_READ);
			FidLev2 = GSSiOpenFile (TempFile[lev2],0,OF_CREATE);
			IndexLen = GSSillseek (FidLev,0,2) / 8;
			GSSillseek (FidLev,0,0);
			FirstIndex = GSSillseek (FidOut,0,1);
			for (i=0;i<IndexLen;i++)
			{
				BigRead (FidLev,&IndexRec,8);
				if (i % NumPerIndexRec == 0)
				{
					if (i)
						BigWrite (FidOut,&BlockTerminator,8,-1);
					IndexRec2.GridCellID = IndexRec.GridCellID;
					IndexRec2.loc = GSSillseek (FidOut,0,1);
					BigWrite (FidLev2,&IndexRec2,8,-1);
				}
				BigWrite (FidOut,&IndexRec,8,-1);
			}
			IndexRec.GridCellID = 0;
			IndexRec.loc = -1;
			BigWrite (FidOut,&IndexRec,8,-1); //marks end of index
			GSSiClose (FidLev);
			GSSiClose (FidLev2);
		} while (IndexLen > NumPerIndexRec); 

		Header.TileWidth = GridPixelWidth/4;
		Header.TileHeight = GridPixelWidth/4;
		Header.NumPerIndexRec = NumPerIndexRec;
		Header.NumIndexLevs = nIndexLevs;
		Header.FirstIndexLoc = FirstIndex;
		Header.NumRecs = nOutRecs;
		BigWrite (FidOut,&Header,sizeof(Header),-1);
		GSSiClose (FidOut);
		GSSiClose (FidBIN);  
		GlobalUnlock (hDB);
	    CloseGWDatabase (hDB);  
		GSSiRemove (TempFile[0]);
		GSSiRemove (TempFile[1]);
		rtn = TRUE;
	}
	return rtn;
}	

BOOL MapExtractExport (LPSTR MapImageFile,LPSTR Type,LPSTR OutFile,long GridID)
{
	HANDLE	hBT;
	LPSTR	pDot = _fstrrchr (MapImageFile,'.'); 
	long	Offset;  
	int		rtn=FALSE;
	HFILE	FidOut;
	long	MapID;
	char	txt[32];
	GRIDFILEHEADER	Header;
	GRIDCELLDEF	Grid;
	GRIDINDEXREC	IndexRec, IndexRec2,BlockTerminator={0,-1};
	HFILE	FidBIN=GSSiOpenFile (MapImageFile,0,OF_READ);
	HFILE	FidLev, FidLev2;
	char	TempFile[2][256], tempBTFile[256];
	int		nOutRecs=0, nIndexLevs=0, loc, IndexLen, NumPerIndexRec=MAX_RECS_IN_INDEX;
	int		lev1, lev2;
	int		i, FirstIndex,lenTile,GridCellID,iScale;
	char	GridName[16];
	BTVARDESC	BTVar[2];

	if (FidBIN == HFILE_ERROR)
		return FALSE;
	itoa (GridID*10,GridName,10);
	if (!GetGridDef (GridName))
	{
	    GSSiClose (FidBIN); 
		return FALSE;
	}

	/*_lwrite (hfile,(LPCCH)&i,sizeof(int));
	_lwrite (hfile,(LPCCH)&cellID,sizeof(int));
	_lwrite (hfile,(LPCCH)&lenTile,sizeof(int));
	_lwrite (hfile,(LPCCH)pTile,lenTile);*/
	//SaveLayerDefFile (OutFile);
	BTVar[0].BT_VARTYP=BT_INTEGER;
	BTVar[0].BT_VARLEN=4;
	BTVar[0].BT_VAROFF=0;
	BTVar[1].BT_VARTYP=BT_INTEGER;
	BTVar[1].BT_VARLEN=4;
	BTVar[1].BT_VAROFF=4;

	GSSiGetTempFileName (0,"gm",0,tempBTFile);
	BT_CREATE (tempBTFile, 4, FALSE, 2, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
	hBT = BT_OPEN (tempBTFile, 0, BT_WRITE, 0);
	while (BigRead (FidBIN,(LPSTR)&iScale,sizeof(int))==sizeof(int))
	{
		BigRead (FidBIN,(LPSTR)&GridCellID,sizeof(int));
		Offset = GSSillseek (FidBIN,0,1);
		BigRead (FidBIN,(LPSTR)&lenTile,sizeof(int));
		GSSillseek (FidBIN,lenTile,1);
		Grid.GridID = GridID;
		Grid.GridCellID = GridCellID;
		if (iScale == GridID)
			BT_PUT (hBT,(LPSTR)&Grid,(LPSTR)&Offset);
	}
	GSSiGetTempFileName (0,"gm",0,TempFile[0]);
	GSSiGetTempFileName (0,"gm",0,TempFile[1]);
	{

		FidOut = GSSiOpenFile (OutFile,0,OF_CREATE);
		FidLev = GSSiOpenFile (TempFile[0],0,OF_CREATE);
		if (FidOut != HFILE_ERROR)
		{
			int	cond=BT_FIRST, pos=BT_GE;

			Grid.GridID = GridID;
			Grid.GridCellID = -1;
			while (!BT_FIND (hBT,(LPSTR)&Grid,cond,pos, (LPSTR)&Offset)) 
			{
				MNMXCORL	CellBounds;

				cond = BT_NEXT;
				pos = BT_ANY;
				if (Grid.GridID != GridID)
					break;
				GetGridBounds (Grid.GridCellID,&CellBounds);
				GSSillseek2 (FidBIN,Offset,0);
				BigRead (FidBIN,(LPSTR)&lenTile,sizeof(int));
				{
					HANDLE	hMem=GSSiGlobAlloc (1588,GMEM_MOVEABLE,lenTile);
					HPSTR	pMem=GlobalLock (hMem);
					
					BigRead (FidBIN,pMem,lenTile);
					loc = GSSillseek (FidOut,0,1);
			//		if (nOutRecs++ % NumPerIndexRec == 0)
					{
						BigWrite (FidOut,&BlockTerminator,8,-1);
						IndexRec.GridCellID = Grid.GridCellID;
						IndexRec.loc = loc+8;
						BigWrite (FidLev,&IndexRec,8,-1);
						nOutRecs++;
					}
					BigWrite (FidOut,&Grid.GridCellID,sizeof(int),-1);
					BigWrite (FidOut,&lenTile,sizeof(int),-1);
					BigWrite (FidOut,pMem,lenTile,-1);
					GSSiGlobUlFree (&hMem);
				} 
			}
			BigWrite (FidOut,&BlockTerminator,8,-1);
		}
		GSSiClose (FidLev);
		do
		{
			lev1 = nIndexLevs % 2;
			nIndexLevs++;
			lev2 = nIndexLevs % 2;
			FidLev = GSSiOpenFile (TempFile[lev1],0,OF_READ);
			FidLev2 = GSSiOpenFile (TempFile[lev2],0,OF_CREATE);
			IndexLen = GSSillseek (FidLev,0,2) / 8;
			GSSillseek (FidLev,0,0);
			FirstIndex = GSSillseek (FidOut,0,1);
			for (i=0;i<IndexLen;i++)
			{
				BigRead (FidLev,&IndexRec,8);
				if (i % NumPerIndexRec == 0)
				{
					if (i)
						BigWrite (FidOut,&BlockTerminator,8,-1);
					IndexRec2.GridCellID = IndexRec.GridCellID;
					IndexRec2.loc = GSSillseek (FidOut,0,1);
					BigWrite (FidLev2,&IndexRec2,8,-1);
				}
				BigWrite (FidOut,&IndexRec,8,-1);
			}
			IndexRec.GridCellID = 0;
			IndexRec.loc = -1;
			BigWrite (FidOut,&IndexRec,8,-1); //marks end of index
			GSSiClose (FidLev);
			GSSiClose (FidLev2);
		} while (IndexLen > NumPerIndexRec); 

		Header.TileWidth = GridPixelWidth/4;
		Header.TileHeight = GridPixelWidth/4;
		Header.NumPerIndexRec = NumPerIndexRec;
		Header.NumIndexLevs = nIndexLevs;
		Header.FirstIndexLoc = FirstIndex;
		Header.NumRecs = nOutRecs;
		BigWrite (FidOut,&Header,sizeof(Header),-1);
		GSSiClose (FidOut);
	    GSSiClose (FidBIN); 
		GSSiRemove (TempFile[0]);
		GSSiRemove (TempFile[1]);
		BT_CLOSEANDDELETE (&hBT);
		rtn = TRUE;
	}
	return rtn;
}	

int GetMapfileOffset (HFILE Fid)
{
	struct {int version; int id; unsigned int marker;}FileFooter;
	int rtn=0;

	if (Fid == HFILE_ERROR)
		return rtn;
	GSSillseek (Fid,-(int)sizeof(FileFooter),SEEK_END);
	BigRead (Fid,&FileFooter,sizeof(FileFooter));
	if (FileFooter.id == 323498251 && FileFooter.marker == 0xFFFFFFFF)
	{
		GSSillseek (Fid,-(256 + (int)sizeof(FileFooter)),SEEK_END);
		rtn = 256 + (int)sizeof(FileFooter);
	}
	return rtn;
}

MNMXCORL GetImageFileBounds (LPSTR File,int GridID)
{
#define	MAXLEVS	8
	char	GridName[16];
	MNMXCORL FileBounds, CellBounds;
	GRIDFILEHEADER	FileHeader;
	HFILE	FidIndex;
	GRIDINDEXREC	IndexRecordBlock[MAXLEVS][MAX_RECS_IN_BLOCK+1];
	int	Levpos[MAXLEVS];
	int	Curlev=0, nfound=0;
	int	ii;
	
	MinMaxInitL (&FileBounds);
	itoa (GridID,GridName,10);
	if (GetGridDef (GridName))
	{
		FidIndex = GSSiOpenFile (File,0,OF_READ);

		if (FidIndex != HFILE_ERROR)
		{
			int ioff = GetMapfileOffset (FidIndex);
			GSSillseek (FidIndex,-(ioff + sizeof(GRIDFILEHEADER)),SEEK_END);
			BigRead (FidIndex,&FileHeader,sizeof(GRIDFILEHEADER));
			GSSillseek (FidIndex,FileHeader.FirstIndexLoc,SEEK_SET);
Next:
			while (Curlev < FileHeader.NumIndexLevs)
			{
				BigRead (FidIndex,IndexRecordBlock[Curlev],(FileHeader.NumPerIndexRec+1)*sizeof(GRIDINDEXREC));
				GSSillseek (FidIndex,IndexRecordBlock[Curlev][0].loc,SEEK_SET);
				Levpos[Curlev++] = 0;
			}
			Curlev--;
			while (IndexRecordBlock[Curlev][Levpos[Curlev]].loc >= 0)
			{
				GetGridBounds (IndexRecordBlock[Curlev][Levpos[Curlev]].GridCellID,&CellBounds);
				AddMinMaxL (&FileBounds,&CellBounds);
				nfound++;
				Levpos[Curlev]++;
			}
			while (Curlev >= 0 && IndexRecordBlock[Curlev][Levpos[Curlev]].loc < 0)
			{
				Curlev--;
				if (Curlev >= 0)
					Levpos[Curlev]++;
			}
			if (Curlev >= 0)
			{
				GSSillseek (FidIndex,IndexRecordBlock[Curlev][Levpos[Curlev++]].loc,SEEK_SET);
				goto Next;
			}
			GSSiClose (FidIndex);
		}
		if (FileHeader.NumRecs != nfound)
			ii=1;
	}
	return FileBounds;
}

BOOL CreateSymlistFile (LPSTR OutFile)
{
	char	str[260];
	int		numsym=0;
	short	minsym, maxsym;
	int		symnum, Depth, Depth1, Depth2,i;
	char	Symname[80];
	LPSTR	pto;
	BYTE	SymDepth[5000][2];
	int		LParent, AParent;
	short	nChildren;
	HANDLE	hChildren=0;
	LPSHORT	pChild;
	HFILE Fid = GSSiOpenFile ("[%DL]\\symdump.txt",0,OF_CREATE);
	int nsym;

	minsym = 5000;
	maxsym = 0;
	LParent = GetDictSymbolNumber ("DEPTHCON");
	GetSymDictChildren (LParent,&nChildren,&hChildren,2,FALSE);
	if (hChildren)
	{  
		pChild = (LPSHORT)GlobalLock (hChildren);
		while (nChildren--)
		{
			GetDictSymName (*pChild,Symname);
			sprintf (str,"%s\t<on>\t%i\t%i",Symname,LParent,*pChild++);
			fputstring (str,Fid);
		}
		GSSiGlobUlFree (&hChildren);
	}
	AParent = GetDictSymbolNumber ("DEPTHAREAS");
	GetSymDictChildren (AParent,&nChildren,&hChildren,3,FALSE);
	if (hChildren)
	{  
		pChild = (LPSHORT)GlobalLock (hChildren);
		while (nChildren--)
		{
			GetDictSymName (*pChild,Symname);
			sprintf (str,"%s\t<on>\t%i\t%i",Symname,AParent,*pChild++);
			fputstring (str,Fid);
		}
		GSSiGlobUlFree (&hChildren);
	}
	for (i=0;i<3200;i++)
	{
		GetDictSymName (i,Symname);
		strupr (Symname);
		Truncate (Symname);
		if (!strncmp (Symname,"LKC",3) && GetDictSymParent (i) != LParent)
		{
			sprintf (str,"%s\t<on>\t%i\t%i",Symname,GetDictSymParent (i),i);
			fputstring (str,Fid);
		}
		else if (strstr (Symname," TO ") && GetDictSymParent (i) != AParent)
		{
			sprintf (str,"%s\t<on>\t%i\t%i",Symname,GetDictSymParent (i),i);
			fputstring (str,Fid);
		}
		else if (*Symname && IsInteger(Symname) && GetDictSymParent (i) != AParent)
		{
			sprintf (str,"%s\t<on>\t%i\t%i",Symname,GetDictSymParent (i),i);
			fputstring (str,Fid);
		}

	}
	GSSiClose (Fid);

	Fid = GSSiOpenFile ("[%DL]\\symdump_compact.txt",0,OF_READ);
	if (Fid == HFILE_ERROR)
		Fid = GSSiOpenFile ("[%DL]\\symdump.txt",0,OF_READ);
	while (fgetstring (str,128,Fid))
	{
		LPSTR ptab2 = strrchr (str,'\t');
		LPSTR ptab1 = strchr (str,'\t');

		*ptab1 = 0;
		ptab2++;
		symnum = atoi (ptab2);
		if (symnum > 0 && symnum < 3200)
		{
			minsym = min (minsym,symnum);
			maxsym = max (maxsym,symnum);
			strcpy (Symname,strupr(str));
			numsym++;
		}
	}
	GSSillseek (Fid,0,0);
	memset (SymDepth,0,sizeof(SymDepth));
	while (fgetstring (str,128,Fid))
	{
		LPSTR ptab2 = strrchr (str,'\t');
		LPSTR ptab1 = strchr (str,'\t');

		*ptab1 = 0;
		ptab2++;
		symnum = atoi (ptab2);
		strcpy (Symname,strupr(str));
		if (symnum > 0 && symnum < 3200)
		{
			if (!strnicmp (Symname,"LKC",3))
				SymDepth[symnum - minsym][0] = min (255,atoi (&Symname[3]));
			else
			{
				SymDepth[symnum - minsym][0] = SymDepth[symnum - minsym][1] = min (255,atoi (Symname));
				if ((pto = strstr (Symname,"TO")))
					SymDepth[symnum - minsym][1] = min (255,atoi (pto+2));
			}
		}
	}
	GSSiClose (Fid);
	Fid = GSSiOpenFile (OutFile,0,OF_CREATE);
	maxsym = min (maxsym,minsym+2499);
	nsym = maxsym - minsym + 1;
	BigWrite (Fid,&minsym,2,-1);
	BigWrite (Fid,&maxsym,2,-1);
	BigWrite (Fid,SymDepth,nsym*2,-1);
	GSSiClose (Fid);
	return TRUE;
}

BOOL CreateSymlistFile2 (LPSTR OutFile)
{
	char	str[130];
	int		numsym=0;
	short	minsym, maxsym, zeroSym=-1;
	int		symnum, Depth, Depth1, Depth2;
	char	Symname[32];
	LPSTR	pto;
	short	SymDepth[5000][2];
	HFILE Fid = GSSiOpenFile ("[%DL]\\symdump.txt",0,OF_READ);
	int		i, nsym, ngaps=0, badsym=0, zerosym=0;

	minsym = 5000;
	maxsym = 0;
	while (fgetstring (str,128,Fid))
	{
		LPSTR ptab2 = strrchr (str,'\t');
		LPSTR ptab1 = strchr (str,'\t');

		*ptab1 = 0;
		ptab2++;
		symnum = atoi (ptab2);
		if (symnum > 0 && symnum < 3200)
		{
			minsym = min (minsym,symnum);
			maxsym = max (maxsym,symnum);
			strcpy (Symname,strupr(str));
			numsym++;
		}
	}
	GSSillseek (Fid,0,0);
	memset (SymDepth,0,sizeof(SymDepth));
	while (fgetstring (str,128,Fid))
	{
		LPSTR ptab2 = strrchr (str,'\t');
		LPSTR ptab1 = strchr (str,'\t');
		char line[130];

		strcpy (line,str);

		*ptab1 = 0;
		ptab2++;
		symnum = atoi (ptab2);
		strcpy (Symname,strupr(str));
		if (symnum > 0 && symnum < 3200)
		{
			if (!stricmp (Symname,"LKC000"))
				zeroSym = symnum;
			else if (!strnicmp (Symname,"LKC",3))
				SymDepth[symnum - minsym][0] = min (25500,atoi (&Symname[3]));
			else
			{
				SymDepth[symnum - minsym][0] = SymDepth[symnum - minsym][1] = min (25500,atoi (Symname));
				if ((pto = strstr (Symname,"TO")))
					SymDepth[symnum - minsym][1] = min (25500,atoi (pto+2));
				else if (SymDepth[symnum - minsym][0] == 0)
				{
					badsym++;
					AppendFile ("c:\\temp\\badsyms.txt",line);
				}
			}
			if (SymDepth[symnum - minsym][0] == 0 && SymDepth[symnum - minsym][1] == 0)
			{
				zerosym++;
				AppendFile ("c:\\temp\\zerosyms.txt",line);
			}
			if (symnum - minsym > 2000)
				AppendFile ("c:\\temp\\highsyms.txt",line);

		}
	}
	GSSiClose (Fid);
	nsym = maxsym - minsym + 1;
	for (i=0;i<min(2500,nsym);i++)
	{
		if (SymDepth[i][0] == 0 &&
			SymDepth[i][1] == 0)
		{
			char str[32];
			itoa (i+minsym,str,10);
			AppendFile ("c:\\temp\\gaplist.txt",str);
			ngaps++;
		}
	}
	if (nsym >= 3000)
	{
		sprintf (str,"Too many symbols:%i  ngaps:%i  minsym:%i  badsym:%i",nsym,ngaps,minsym,badsym);
		MessageBox (0,str,0,MB_ICONEXCLAMATION);
	}
	else
	{
		sprintf (str,"nsyms:%i  ngaps:%i  minsym:%i  badsym:%i",nsym,ngaps,minsym,badsym);
		MessageBox (0,str,"",MB_OK);
	}
	Fid = GSSiOpenFile (OutFile,0,OF_CREATE);
	maxsym = min (maxsym,minsym+2499);
	BigWrite (Fid,&minsym,2,-1);
	BigWrite (Fid,&maxsym,2,-1);
	BigWrite (Fid,SymDepth,(nsym)*4,-1);
	BigWrite (Fid,&zeroSym,2,-1);
	GSSiClose (Fid);
	return TRUE;
}

BOOL ContourTextExport (LPSTR ContourTextFile,LPSTR Type,LPSTR OutFile,long GridID)
{
	HANDLE	hDB = OpenGWDatabase (ContourTextFile,BT_READ);  
	LPSTR	pDot = _fstrrchr (ContourTextFile,'.'); 
	long	Offset;  
	int		rtn=FALSE;
	HFILE	FidOut;
	long	MapID, CurCellID=-1;
	char	txt[32];
	typedef	struct	{long GridCellID;long Sequence;} GRIDCELLDEF;
	typedef struct	{long GridCellID, loc;} GRIDINDEXREC;
	typedef struct	{long GridID,NumPerIndexRec,NumIndexLevs,FirstIndexLoc,NumRecs;} CTEXTFILEHEADER;
	CTEXTFILEHEADER	Header;
	GRIDCELLDEF	Grid, IndexBeginGrid;
	GRIDINDEXREC	IndexRec, IndexRec2,BlockTerminator={0,-1};
	HFILE	FidBIN=HFILE_ERROR;
	HFILE	FidLev, FidLev2;
	char	TempFile[2][256];
	int		nOutRecs=0, nIndexLevs=0, loc, IndexLen, NumPerIndexRec=MAX_RECS_IN_INDEX;
	int		lev1, lev2;
	int		i, ii,FirstIndex, RecSize=0;
	char	GridName[16];
	BOOL	Done = FALSE;
	LPCTEXTOUTREC	pCTextOutRec;
#define MAX_CTEXT_PER_CELL	4096

	itoa (GridID,GridName,10);
	if (!GetGridDef (GridName))
		return FALSE;
	
	//SaveLayerDefFile (OutFile);
	GSSiGetTempFileName (0,"gm",0,TempFile[0]);
	GSSiGetTempFileName (0,"gm",0,TempFile[1]);
	if (hDB)
	{
	    LPGWDHEADER lpGWDHead = (LPGWDHEADER)GlobalLock (hDB);
		LPSAVECONTEXT	pSaveC = (LPSAVECONTEXT)&lpGWDHead->GWDData; 
		HANDLE	hMem = GSSiGlobAlloc (1591,GMEM_MOVEABLE,MAX_CTEXT_PER_CELL * sizeof(CTEXTOUTREC));
		LPSTR	pMem = GlobalLock (hMem);
		int		nInRec=0, nSkip=1, n=0;

		FidOut = GSSiOpenFile (OutFile,0,OF_CREATE);
		FidLev = GSSiOpenFile (TempFile[0],0,OF_CREATE);
		if (FidOut != HFILE_ERROR)
		{
			int	cond=BT_FIRST;
			int	nRecs = BT_NUM_IN_INDEX (lpGWDHead->BTHandle[0]);
			int	Curloc = 0;
			int CurlocBeginGrid = 0;

			CreateStatusWind (hWndMain,1,"Export Contour Text");
			while (!BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&Grid,cond,BT_ANY, (LPSTR)&Offset)) 
			{
				if (!nInRec)
					IndexBeginGrid = Grid;
Restart:
        		FillGWDData (lpGWDHead,Offset); 
				if (CurCellID != Grid.GridCellID)
				{
					if (cond == BT_FIRST)
						cond = BT_NEXT;
					else
					{
WriteRec:
						loc = GSSillseek (FidOut,0,1);
					//	if (nOutRecs++ % NumPerIndexRec == 0)
						{
							BigWrite (FidOut,&BlockTerminator,8,-1);
							IndexRec.GridCellID = CurCellID;
							IndexRec.loc = loc+8;
							BigWrite (FidLev,&IndexRec,8,-1);
						}
						BigWrite (FidOut,&CurCellID,4,-1);
						BigWrite (FidOut,&RecSize,4,-1);
						if (RecSize > 40000)
							ii=1;
						BigWrite (FidOut,pMem,RecSize,-1);
					}
					CurCellID = Grid.GridCellID;
					IndexBeginGrid = Grid;
					CurlocBeginGrid = Curloc;
					RecSize = 0;
					nInRec = 0;
					nSkip = 1;
					n = 0;
					pCTextOutRec = (LPCTEXTOUTREC) pMem;
					if (Done)
						goto CloseIt;
				} 
				if (nInRec >= MAX_CTEXT_PER_CELL)
				{
					int ii;

					nSkip++;
					ii=BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&IndexBeginGrid,BT_FIRST,BT_EQ, (LPSTR)&Offset);
					RecSize = 0;
					nInRec = 0;
					n = 0;
					pCTextOutRec = (LPCTEXTOUTREC) pMem;
					Curloc = CurlocBeginGrid;
					goto Restart;
				}
				if (!pSaveC->CharNo)
					n++;
				if (n % nSkip > 0)
					ii=1;
				else
				{
					*pCTextOutRec++ = *(LPCTEXTOUTREC)&pSaveC->CharNo;
					RecSize += sizeof (CTEXTOUTREC);
					nInRec++;
					nOutRecs++;
				}
				if (!StatusWindowUpdate (NULL,NULL, nRecs,Curloc++))
					break;
			}
			Done = TRUE;
			goto WriteRec;
		}
CloseIt:
		DestroyStatusWindow(0);  
		BigWrite (FidOut,&BlockTerminator,8,-1);
		GSSiClose (FidLev);
		do
		{
			lev1 = nIndexLevs % 2;
			nIndexLevs++;
			lev2 = nIndexLevs % 2;
			FidLev = GSSiOpenFile (TempFile[lev1],0,OF_READ);
			FidLev2 = GSSiOpenFile (TempFile[lev2],0,OF_CREATE);
			IndexLen = GSSillseek (FidLev,0,2) / 8;
			GSSillseek (FidLev,0,0);
			FirstIndex = GSSillseek (FidOut,0,1);
			for (i=0;i<IndexLen;i++)
			{
				BigRead (FidLev,&IndexRec,8);
				if (i % NumPerIndexRec == 0)
				{
					if (i)
						BigWrite (FidOut,&BlockTerminator,8,-1);
					IndexRec2.GridCellID = IndexRec.GridCellID;
					IndexRec2.loc = GSSillseek (FidOut,0,1);
					BigWrite (FidLev2,&IndexRec2,8,-1);
				}
				BigWrite (FidOut,&IndexRec,8,-1);
			}
			IndexRec.GridCellID = 0;
			IndexRec.loc = -1;
			BigWrite (FidOut,&IndexRec,8,-1); //marks end of index
			GSSiClose (FidLev);
			GSSiClose (FidLev2);
		} while (IndexLen > NumPerIndexRec); 

		Header.GridID = GridID/10;
		Header.NumPerIndexRec = NumPerIndexRec;
		Header.NumIndexLevs = nIndexLevs;
		Header.FirstIndexLoc = FirstIndex;
		Header.NumRecs = nOutRecs;
		BigWrite (FidOut,&Header,sizeof(Header),-1);
		GSSiClose (FidOut);
		GSSiClose (FidBIN);  
		GlobalUnlock (hDB);
	    CloseGWDatabase (hDB);  
		GSSiRemove (TempFile[0]);
		GSSiRemove (TempFile[1]);
		GSSiGlobUlFree (&hMem);
		rtn = TRUE;
	}
	return rtn;
}	

BOOL MapTextExport (LPSTR TextFile,LPSTR Type,LPSTR OutFile)
{
	HFILE	Fid = GSSiOpenFile (TextFile,0,OF_READ);
	long	Offset;  
	int		rtn=FALSE;
	HFILE	FidOut;
	long	MapID, CurCellID=-1;
	char	txt[32];
	typedef	struct	{long GridCellID;long Sequence;} GRIDCELLDEF;
	typedef struct	{long GridCellID, loc;} GRIDINDEXREC;
	typedef struct	{long GridID,NumPerIndexRec,NumIndexLevs,FirstIndexLoc,NumRecs;} CTEXTFILEHEADER;
	CTEXTFILEHEADER	Header;
	GRIDCELLDEF	Grid;
	GRIDINDEXREC	IndexRec, IndexRec2,BlockTerminator={0,-1};
	HFILE	FidBIN=HFILE_ERROR;
	HFILE	FidLev, FidLev2;
	char	TempFile[2][256];
	int		nOutRecs=0, nIndexLevs=0, loc, IndexLen, NumPerIndexRec=MAX_RECS_IN_INDEX;
	int		lev1, lev2;
	int		i, FirstIndex, RecSize=0;
	char	line[1024];
	BOOL	Done = FALSE;
	LPMAPPOINTOUTREC	pMapPointOutRec;

	GSSiGetTempFileName (0,"gm",0,TempFile[0]);
	GSSiGetTempFileName (0,"gm",0,TempFile[1]);
	if (Fid != HFILE_ERROR)
	{
		FidOut = GSSiOpenFile (OutFile,0,OF_CREATE);
		FidLev = GSSiOpenFile (TempFile[0],0,OF_CREATE);
		if (FidOut != HFILE_ERROR)
		{
			int	LastLoc = -1;

			while (fgetstring (line,1020,Fid)) 
			{
				if (!strnicmp (line,"REF=",4))
				{
					int	CurRef=atoi(&line[4]);

					loc = GSSillseek (FidOut,0,1);
					if (LastLoc > -1)
					{
						RecSize -= 2;
						GSSillseek (FidOut,LastLoc,0);
						BigWrite (FidOut,&RecSize,4,-1);
						GSSillseek (FidOut,loc,0);
					}
					BigWrite (FidOut,&BlockTerminator,8,-1);
					IndexRec.GridCellID = CurRef;
					IndexRec.loc = loc+8;
					BigWrite (FidLev,&IndexRec,8,-1);
					BigWrite (FidOut,&CurRef,4,-1);
					LastLoc = GSSillseek (FidOut,0,1);
					BigWrite (FidOut,&RecSize,4,-1);
					RecSize = 0;
					nOutRecs++;
				} 
				else
				{
					fputstring (line,FidOut);
					RecSize += strlen (line) + 2;
				}
			}
			loc = GSSillseek (FidOut,0,1);
			if (LastLoc > -1)
			{
				RecSize -= 2;
				GSSillseek (FidOut,LastLoc,0);
				BigWrite (FidOut,&RecSize,4,-1);
				GSSillseek (FidOut,loc,0);
			}
		}
CloseIt:
		BigWrite (FidOut,&BlockTerminator,8,-1);
		GSSiClose (FidLev);
		do
		{
			lev1 = nIndexLevs % 2;
			nIndexLevs++;
			lev2 = nIndexLevs % 2;
			FidLev = GSSiOpenFile (TempFile[lev1],0,OF_READ);
			FidLev2 = GSSiOpenFile (TempFile[lev2],0,OF_CREATE);
			IndexLen = GSSillseek (FidLev,0,2) / 8;
			GSSillseek (FidLev,0,0);
			FirstIndex = GSSillseek (FidOut,0,1);
			for (i=0;i<IndexLen;i++)
			{
				BigRead (FidLev,&IndexRec,8);
				if (i % NumPerIndexRec == 0)
				{
					if (i)
						BigWrite (FidOut,&BlockTerminator,8,-1);
					IndexRec2.GridCellID = IndexRec.GridCellID;
					IndexRec2.loc = GSSillseek (FidOut,0,1);
					BigWrite (FidLev2,&IndexRec2,8,-1);
				}
				BigWrite (FidOut,&IndexRec,8,-1);
			}
			IndexRec.GridCellID = 0;
			IndexRec.loc = -1;
			BigWrite (FidOut,&IndexRec,8,-1); //marks end of index
			GSSiClose (FidLev);
			GSSiClose (FidLev2);
		} while (IndexLen > NumPerIndexRec); 

		Header.GridID = 0;
		Header.NumPerIndexRec = NumPerIndexRec;
		Header.NumIndexLevs = nIndexLevs;
		Header.FirstIndexLoc = FirstIndex;
		Header.NumRecs = nOutRecs;
		BigWrite (FidOut,&Header,sizeof(Header),-1);
		GSSiClose (FidOut);
		GSSiClose (FidBIN);  
		GSSiClose (Fid);  
		GSSiRemove (TempFile[0]);
		GSSiRemove (TempFile[1]);
		rtn = TRUE;
	}
	return rtn;
}	

BOOL MapImagePointExport (LPSTR PointFile,LPSTR Type,LPSTR OutFile,long GridID)
{
	HANDLE	hDB = OpenGWDatabase (PointFile,BT_READ);  
	long	Offset;  
	int		rtn=FALSE;
	HFILE	FidOut;
	long	MapID, CurCellID=-99;
	char	txt[32];
	typedef	struct	{long GridCellID;long Sequence;} GRIDCELLDEF;
	typedef struct	{long GridCellID, loc;} GRIDINDEXREC;
	typedef struct	{long GridID,NumPerIndexRec,NumIndexLevs,FirstIndexLoc,NumRecs;} CTEXTFILEHEADER;
	CTEXTFILEHEADER	Header;
	GRIDCELLDEF	Grid;
	GRIDINDEXREC	IndexRec, IndexRec2,BlockTerminator={0,-1};
	HFILE	FidBIN=HFILE_ERROR;
	HFILE	FidLev, FidLev2;
	char	TempFile[2][256];
	int		nOutRecs=0, nIndexLevs=0, loc, IndexLen, NumPerIndexRec=MAX_RECS_IN_INDEX;
	int		lev1, lev2;
	int		i, FirstIndex, RecSize=0,nInRec=0;
	char	GridName[16];
	BOOL	Done = FALSE;
	LPMAPPOINTOUTREC	pMapPointOutRec;
#define MAX_POINT_PER_CELL	2048

	itoa (GridID,GridName,10);
	if (!GetGridDef (GridName))
		return FALSE;
	
	//SaveLayerDefFile (OutFile);
	GSSiGetTempFileName (0,"gm",0,TempFile[0]);
	GSSiGetTempFileName (0,"gm",0,TempFile[1]);
	if (hDB)
	{
	    LPGWDHEADER lpGWDHead = (LPGWDHEADER)GlobalLock (hDB);
		LPMAPPOINTS	pMapPoints = (LPMAPPOINTS)&lpGWDHead->GWDData; 
		HANDLE	hMem = GSSiGlobAlloc (1591,GMEM_MOVEABLE,MAX_POINT_PER_CELL * sizeof(MAPPOINTOUTREC));
		LPSTR	pMem = GlobalLock (hMem);

		FidOut = GSSiOpenFile (OutFile,0,OF_CREATE);
		FidLev = GSSiOpenFile (TempFile[0],0,OF_CREATE);
		if (FidOut != HFILE_ERROR)
		{
			int	cond=BT_FIRST;

			while (!BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&Grid,cond,BT_ANY, (LPSTR)&Offset)) 
			{
        		FillGWDData (lpGWDHead,Offset); 
				if (CurCellID != Grid.GridCellID)
				{
					if (cond == BT_FIRST)
					{
						cond = BT_NEXT;
					}
					else
					{
WriteRec:
						loc = GSSillseek (FidOut,0,1);
					//	if (nOutRecs++ % NumPerIndexRec == 0)
						{
							BigWrite (FidOut,&BlockTerminator,8,-1);
							IndexRec.GridCellID = CurCellID;
							IndexRec.loc = loc+8;
							BigWrite (FidLev,&IndexRec,8,-1);
						}
						BigWrite (FidOut,&CurCellID,4,-1);
						BigWrite (FidOut,&RecSize,4,-1);
						BigWrite (FidOut,pMem,RecSize,-1);
					}
					CurCellID = Grid.GridCellID;
					RecSize = 0;
					nInRec = 0;
					pMapPointOutRec = (LPMAPPOINTOUTREC) pMem;
					if (Done)
						goto CloseIt;
				} 
				pMapPointOutRec->ref = pMapPoints->ref;
				pMapPointOutRec->symno = pMapPoints->symnum;
				pMapPointOutRec->hasextendedtext = pMapPoints->hasextendedtext;
				pMapPointOutRec->x = IDNINT (10000*pMapPoints->pctx);
				pMapPointOutRec->y = IDNINT (10000*pMapPoints->pcty);
				strncpy (pMapPointOutRec->PointText,pMapPoints->PointText,64);
				if (nInRec < MAX_POINT_PER_CELL)
				{
					*pMapPointOutRec++;
					RecSize += sizeof (MAPPOINTOUTREC);
					nOutRecs++;
					nInRec++;
				}
				else
				{
					MessageBox (0,"Point record maxed out",0,MB_ICONEXCLAMATION);
					break;
				}
			}
			Done = TRUE;
			goto WriteRec;
		}
CloseIt:
		BigWrite (FidOut,&BlockTerminator,8,-1);
		GSSiClose (FidLev);
		do
		{
			lev1 = nIndexLevs % 2;
			nIndexLevs++;
			lev2 = nIndexLevs % 2;
			FidLev = GSSiOpenFile (TempFile[lev1],0,OF_READ);
			FidLev2 = GSSiOpenFile (TempFile[lev2],0,OF_CREATE);
			IndexLen = GSSillseek (FidLev,0,2) / 8;
			GSSillseek (FidLev,0,0);
			FirstIndex = GSSillseek (FidOut,0,1);
			for (i=0;i<IndexLen;i++)
			{
				BigRead (FidLev,&IndexRec,8);
				if (i % NumPerIndexRec == 0)
				{
					if (i)
						BigWrite (FidOut,&BlockTerminator,8,-1);
					IndexRec2.GridCellID = IndexRec.GridCellID;
					IndexRec2.loc = GSSillseek (FidOut,0,1);
					BigWrite (FidLev2,&IndexRec2,8,-1);
				}
				BigWrite (FidOut,&IndexRec,8,-1);
			}
			IndexRec.GridCellID = 0;
			IndexRec.loc = -1;
			BigWrite (FidOut,&IndexRec,8,-1); //marks end of index
			GSSiClose (FidLev);
			GSSiClose (FidLev2);
		} while (IndexLen > NumPerIndexRec); 

		Header.GridID = GridID/10;
		Header.NumPerIndexRec = NumPerIndexRec;
		Header.NumIndexLevs = nIndexLevs;
		Header.FirstIndexLoc = FirstIndex;
		Header.NumRecs = nOutRecs;
		BigWrite (FidOut,&Header,sizeof(Header),-1);
		GSSiClose (FidOut);
		GSSiClose (FidBIN);  
		GlobalUnlock (hDB);
	    CloseGWDatabase (hDB);  
		GSSiRemove (TempFile[0]);
		GSSiRemove (TempFile[1]);
		GSSiGlobUlFree (&hMem);
		rtn = TRUE;
	}
	return rtn;
}	

BOOL MapImageCityLakeNameExport (LPSTR PointFile,LPSTR Type,LPSTR OutFile,long GridID)
{
	HANDLE	hDB = OpenGWDatabase (PointFile,BT_READ);  
	long	Offset;  
	int		rtn=FALSE;
	HFILE	FidOut;
	long	MapID, CurCellID=-1;
	char	txt[32];
	typedef	struct	{long GridCellID;long Population; long Sequence;} GRIDCELLDEF;
	typedef struct	{long GridCellID, loc;} GRIDINDEXREC;
	typedef struct	{long GridID,NumPerIndexRec,NumIndexLevs,FirstIndexLoc,NumRecs;} CTEXTFILEHEADER;
	CTEXTFILEHEADER	Header;
	GRIDCELLDEF	Grid;
	GRIDINDEXREC	IndexRec, IndexRec2,BlockTerminator={0,-1};
	HFILE	FidBIN=HFILE_ERROR;
	HFILE	FidLev, FidLev2;
	char	TempFile[2][256];
	int		nOutRecs=0, nIndexLevs=0, loc, IndexLen, NumPerIndexRec=MAX_RECS_IN_INDEX;
	int		lev1, lev2;
	int		i, FirstIndex, RecSize, nCellRecs=0,ii,maxname=0;
	char	GridName[16];
	BOOL	Done = FALSE,limitReached = FALSE;
	HANDLE	hGridDef = GSSiGlobAlloc (0,GMEM_MOVEABLE,SHRT_MAX);
	LPSTR	pGridDef = GlobalLock (hGridDef);
	LPCITYLAKENAMEOUTREC 	pMapPointOutRec;
	BOOL	Google = FALSE;
#define MAX_NAMES_PER_CELL	512

	if (!GetGlobalCVal ("[%GRIDDEF]",pGridDef,0))
	{
		GSSiGlobUlFree (&hGridDef);
		return FALSE;
	}
	if (!stricmp (pGridDef,"GoogleMaps"))
		Google = TRUE;
	GSSiGlobUlFree (&hGridDef);

	if (Google)
		itoa (GridID*10,GridName,10);
	else
		itoa (GridID,GridName,10);
	if (!GetGridDef (GridName))
		return FALSE;
	
	//SaveLayerDefFile (OutFile);
	GSSiGetTempFileName (0,"gm",0,TempFile[0]);
	GSSiGetTempFileName (0,"gm",0,TempFile[1]);
	if (hDB)
	{
	    LPGWDHEADER lpGWDHead = (LPGWDHEADER)GlobalLock (hDB);
		LPCITYLAKENAMES	pCLNames = (LPCITYLAKENAMES)&lpGWDHead->GWDData; 
		HANDLE	hMem = GSSiGlobAlloc (1591,GMEM_MOVEABLE,(MAX_NAMES_PER_CELL+1) * sizeof(CITYLAKENAMEOUTREC));
		LPSTR	pMem = GlobalLock (hMem);

		FidOut = GSSiOpenFile (OutFile,0,OF_CREATE);
		FidLev = GSSiOpenFile (TempFile[0],0,OF_CREATE);
		if (FidOut != HFILE_ERROR)
		{
			int	cond=BT_FIRST;

			while (!BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&Grid,cond,BT_ANY, (LPSTR)&Offset)) 
			{
        		FillGWDData (lpGWDHead,Offset);
				if (*TempFile[0] != 'C')
					ii=1;
				if (CurCellID != Grid.GridCellID)
				{
					if (cond == BT_FIRST)
					{
						cond = BT_NEXT;
					}
					else
					{
WriteRec:
						loc = GSSillseek (FidOut,0,1);
					//	if (nOutRecs++ % NumPerIndexRec == 0)
						{
							BigWrite (FidOut,&BlockTerminator,8,-1);
							IndexRec.GridCellID = CurCellID;
							IndexRec.loc = loc+8;
							BigWrite (FidLev,&IndexRec,8,-1);
						}
						BigWrite (FidOut,&CurCellID,4,-1);
						BigWrite (FidOut,&RecSize,4,-1);
						BigWrite (FidOut,pMem,RecSize,-1);
					}
					CurCellID = Grid.GridCellID;
					RecSize = 0;
					nCellRecs = 0;
					pMapPointOutRec = (LPCITYLAKENAMEOUTREC) pMem;
					if (Done)
						goto CloseIt;
				} 
				if (nCellRecs < MAX_NAMES_PER_CELL)
				{
					pMapPointOutRec->Population = abs (pCLNames->Population);
					pMapPointOutRec->type = pCLNames->symnum;
					pMapPointOutRec->nchar = strlen (pCLNames->Name);
					maxname = max (maxname,strlen(pCLNames->Name));
					strncpy (pMapPointOutRec->Name,pCLNames->Name,sizeof(pMapPointOutRec->Name));
					strncpy (pMapPointOutRec->State,pCLNames->State,sizeof(pMapPointOutRec->State));
					pMapPointOutRec->x = IDNINT (10000*pCLNames->pctx);
					pMapPointOutRec->y = IDNINT (10000*pCLNames->pcty);
					*pMapPointOutRec++;
					RecSize += sizeof (CITYLAKENAMEOUTREC);
					nCellRecs++;
					nOutRecs++;
					if (nOutRecs > 159300)
						ii=1;
				}
				else
					limitReached = TRUE;
			}
			Done = TRUE;
			goto WriteRec;
		}
CloseIt:
		BigWrite (FidOut,&BlockTerminator,8,-1);
		GSSiClose (FidLev);
		do
		{
			lev1 = nIndexLevs % 2;
			nIndexLevs++;
			lev2 = nIndexLevs % 2;
			FidLev = GSSiOpenFile (TempFile[lev1],0,OF_READ);
			FidLev2 = GSSiOpenFile (TempFile[lev2],0,OF_CREATE);
			IndexLen = GSSillseek (FidLev,0,2) / 8;
			GSSillseek (FidLev,0,0);
			FirstIndex = GSSillseek (FidOut,0,1);
			for (i=0;i<IndexLen;i++)
			{
				BigRead (FidLev,&IndexRec,8);
				if (i % NumPerIndexRec == 0)
				{
					if (i)
						BigWrite (FidOut,&BlockTerminator,8,-1);
					IndexRec2.GridCellID = IndexRec.GridCellID;
					IndexRec2.loc = GSSillseek (FidOut,0,1);
					BigWrite (FidLev2,&IndexRec2,8,-1);
				}
				BigWrite (FidOut,&IndexRec,8,-1);
			}
			IndexRec.GridCellID = 0;
			IndexRec.loc = -1;
			BigWrite (FidOut,&IndexRec,8,-1); //marks end of index
			GSSiClose (FidLev);
			GSSiClose (FidLev2);
		} while (IndexLen > NumPerIndexRec); 

		Header.GridID = GridID;

		Header.NumPerIndexRec = NumPerIndexRec;
		Header.NumIndexLevs = nIndexLevs;
		Header.FirstIndexLoc = FirstIndex;
		Header.NumRecs = nOutRecs;
		BigWrite (FidOut,&Header,sizeof(Header),-1);
		GSSiClose (FidOut);
		GSSiClose (FidBIN);  
		GlobalUnlock (hDB);
	    CloseGWDatabase (hDB);  
		GSSiRemove (TempFile[0]);
		GSSiRemove (TempFile[1]);
		GSSiGlobUlFree (&hMem);
		rtn = TRUE;
	}
	if (limitReached)
		MessageBox (0,"Limit reached on number of names in a grid",0,MB_ICONEXCLAMATION);
	return rtn;
}	

void RunStartupCommand (int opt)
{   
	char	SUC[256];
	static	BOOL InStartup=FALSE;
	if (hStartupCommand && !InStartup) 
	{   UINT	CmdID;  
		long	lCmdID;              
		LPSTR	pStr=GlobalLock (hStartupCommand);
		LPSTR	pVB = MatchLev (pStr,'|');
		
		InStartup = TRUE;
		if (pVB)
		{
			if (opt == 1)
			{
				*pVB = 0;
				strcpy (SUC,pStr);
				*pVB = '|';
			}
			else
				_fstrcpy (SUC,pVB+1);
		}
		else
			_fstrcpy (SUC,pStr);
		GlobalUnlock (hStartupCommand);					
		if (opt == 1)
		{
			if (pVB)
				ExpandText (SUC);
		}
		else
		{
			ExpandText (SUC);
			lCmdID = GetCmdID (SUC,0);
			if (lCmdID < 0)
			{   
				CmdID = -lCmdID;   
				SetViewport (*pCommandViewport);
				AddGraphicsFunction (CurView->hWnd, CmdID,0);
			}
			else if (lCmdID)
				PostMessage(hWndMain, WM_COMMAND, (WPARAM)lCmdID, 0L); 
		}
		InStartup = FALSE;
	}
	return;
} 

void EndDisplayProcessing (BOOL Final)
#if ENABLETRACE
{GSSiEnterProg (440);
#endif
{   DPOINT	NULLPT; 
	HDIB	hDIB;
	short	ii;
	LPVIEWPORT	SaveVP,SaveVPIn=CurView;
	static	BOOL	First=TRUE;
						     
//							sprintf (mess,"End of dseg %i",DisplayFinOpt);
//							SetWindowText (hWnd,mess);
//BlockSocketProcessing (7);
	if (totContourPoints)
	{
		char mess[128];

		sprintf (mess,"Total Points: %i   Removed Points: %i (%.2f%%)",totContourPoints,trimmedContourPoints,100*((double)trimmedContourPoints/(double)totContourPoints));
		SetWindowText (hWndMain,mess);
	}
	if (Final)
	{   
		short	i;  
		
		SetTransparency (0);
		Blocks=0;
		CloseDisplayedHighlightedRefs();
        IgnoreAVLTimer = FALSE;
		InDisplayProcessing = FALSE; 
		if (MultiZoomLevel)
		{  
			MNMXCORD	Bounds; 
			DWORD	hdd;   
			BOOL	st;
			  
			SetViewport (MultiZoomVP);
        	SetDisplayMode (CurView->hDC,GF_TEXTMODE);
        	SelectClipRgn (CurView->hDC,0);
			MultiZoomLevel--;  
			MultiLevelBounds[MultiZoomLevel] = CurView->WBounds;
			if (MultiZoomLevel)
			{
				hMemBitmap = CreateCompatibleBitmap (CurView->hDC,(int)MemMapWidth,(int)MemMapHeight);
				hbmpMultiLevel[MultiZoomLevel] = SelectObject(hdcMemMap, hMemBitmap);
				CurView->Scale *= 0.5;
				ZoomToPointAndScale (CurView->MidPointW,CurView->Scale,FALSE);
				GdiFlush ();
	         //	Bounds = FactorBounds (&CurView->WBounds,0.5);
			 //	ZoomToRect(Bounds,FALSE);
{
#if ENABLETRACE
GSSiExitProg (440);
#endif
				return;								
}
			} 
			hMemBitmap = CreateCompatibleBitmap (CurView->hDC,(int)MemMapWidth,(int)MemMapHeight);
			hbmpMultiLevel[MultiZoomLevel] = SelectObject(hdcMemMap,hbmpOld); 
			PostMessage(hWndMain, GF_MULTIZOOM_END, 0, 0L);    
//			hdd = GM32DrawDibOpen ();
//			st = GM32DrawDibClose (hdd);  
		}
		DisplayVirtualPrintAreas ();
		RenameCachedFiles (CachePathnameTo);
	}
//	CloseSymDict();
	switch (DisplayFinOpt)
	{
		case 1:
			if (!hSavedPickList) break;  
			pPickList = (LPPICKDATA)GlobalLock(hSavedPickList);
			if (NumSavedPickList > 0)
				_fmemmove (PickList,pPickList,NumSavedPickList*sizeof(PICKDATA)); 
			GSSiGlobUlFree (&hSavedPickList); 
		    DisplayPickedItems (0,NumPicked,TRUE,0,0,FALSE);  
								    
		    break;  
		case 2: //first display of config 
		{
			char	SUC[256];
			static	FirstDisp=TRUE;
			
			if (InImediate)
				goto Exit;
			RunStartupCommand (2);
		    DisplayFinOpt = 0;
			if (!FirstDisp)
				goto Exit;
			if (App)
			{
				if (GetPrivateProfileString (Applications[App-1],"NewData","",SUC,64,"geomastr.ini"))
				{
					if (LoadNewData (SUC))
						WritePrivateProfileString (Applications[App-1],"NewData",0,GMIni); 
				}
			} 
			FirstDisp = FALSE;
		}
		break;
		case 4:
			while (DisplayNetMarkerThemeLegend(FALSE));
			break;
			
		case 5:
			while (DisplayStreetAddresses(FALSE));
			break;
		
		default:                         
		{
			char	str[256];  
			short	i;
			static	BOOL	First=TRUE;
			
			break;
			if (!DoTime)
				break;
/*			sprintf (str,"Display Time = %ld",TotDisplayTime);
			for (i=0;i<256;i++)    
			{
			    if (!First)
			    {
			    	if (TypeTime[i])
			    		sprintf (_fstrchr(str,0),"%i(%ld)",i,TypeTime[i]);
			    }
				TypeTime[i]=0;
			}
			SetWindowText (hWndMain,str);
*/   
			First=FALSE;
			TotDisplayTime=0;
		}
			break;
	} 
	EndProcessingThemeLegends (); 
	if (idTimer)
		ii=1;
	SaveVP = CurView;
//    if (Final < 2) 
//    {
//		DisplayAllVehicles (TRUE); 
//	}
// 	DisplayVehicle (NULLPT,NULLPT,NULLPT,TRUE);  
    DisplayFinOpt = 0;
	if (Final)
	{
		ApplyVPShadows ();
//		ApplyVPHalfTone (); 
	}
    First = FALSE;
	Counter = -1; 
	{   
		short SaveConfigLevel = ConfigLevel;
//NextTAG:		
		if (ConfigLevel)// && (ConfigLevel == SaveConfigLevel))
		{   
			LPSTR	pFile;
			HDC		hDC=GetDC (hWndMain);
			short	iv;
			
	        UnallocateConfig (); 
	        pFile = GlobalLock (hSavedConfig[ConfigLevel-1]);
	        _fstrcpy (CfgName,pFile);
	        GlobalUnlock (hSavedConfig[ConfigLevel-1]); 
	        OpenConfig(hWndMain,hDC);
		    for (iv=0;iv<*pNumViewports;iv++) 
		    {   
		    	SetViewport (iv+1);
	        	SetBoundsRect2 (CurView->DrawRect,CurView->hDC);
            }
		    SetViewport (*pCommandViewport);
	        GSSiRemove (CfgName);
	        ConfigLevel--;
	        if (!ConfigLevel)   
	        {
	        	SetMainRect (hWndMain,hDC,&SaveMainRect,5);
	        	_fstrcpy (CfgName,Lev0CfgName);
	        }
	        ReleaseDC (hWndMain,hDC); 
//	        goto NextTAG;
		}
		if (CurrentConfig && CurView)	
			DisplayTAGs2 (CurView->hDC,3,NextCFGTAG[ConfigLevel]);
		if (ConfigLevel)
			PostMessage(hWndMain, WM_COMMAND, IDM_REDISPLAY, 0L);    
		else if (ResetToViewport)
		{
			SetCurView (ResetToViewport); 
			ResetToViewport = 0;
		} 
	} 
/*	if (FormatViewport)
	{
		if (SetViewport (FormatViewport))
		{
			FormatViewport = 0; 
			RedisplayViewport (TRUE,FALSE);
		}
		else
			FormatViewport = 0;
	}*/	  
Exit:
    if (Final < 2) 
    {
		DisplayCurStreets (FALSE,0);
		if (!DisplayAllVehicles (TRUE,FALSE)) 
		{
			if (Final < 2 && BufferedScreen)
				ShowBufferedScreen (TRUE,TRUE,0,0);
		}
	}
/*	{
		HDC		hDC;
		RECT	Rect; 
		HBITMAP	hBitmap, hTempBM; 
		short	i;   

		SetViewport (*pCommandViewport);
        SetWindowPos (hWndMain,HWND_TOP,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
        hDC  = GetDC (hWndMain);
		SetDisplayMode (hDC, GF_SCREENMODE); 
	    GetClientRect (hWndMain,&Rect);
	  	SelectClipRgn ( hDC,0);
        ii=BitBlt(hDC, 0, 0, Rect.right-Rect.left+1,
                             Rect.bottom-Rect.top+1,
               	  CurView->hDC,
               	  			 Rect.left,Rect.top, SRCCOPY);  
		for (i=0;i<*pNumViewports;i++)  
		{
				pViewports[i]->hDC = hDC;  
				if (pViewports[i]->hWndDlg)
				{
					SendMessage(pViewports[i]->hWndDlg, GSSI_REPOSITION,1, (LPARAM)pViewports[i]); 
				//	ShowWindow (pViewports[i]->hWndDlg,SW_SHOW);
				//	InvalidateRect (pViewports[i]->hWndDlg,0,TRUE);
				}
		}
        ReleaseDC (hWndMain,hDC); 
        DisplayCycle++;  
	}*/
	else if (Final && BufferedScreen && !CurrentConfig)
	{
        HDC	hDC  = GetDC (hWndMain);
		int	i;

		for (i=0;i<*pNumViewports;i++)  
				pViewports[i]->hDC = hDC;  
        ReleaseDC (hWndMain,hDC);
	}
	else if (MemMap && *MemMapName) 
		SaveMemMap ();   
	if (hWndDigControl)
		PostMessage(hWndDigControl, GSSI_REINITDIALOG, 0, 0L);
	if (Final && !ConfigLevel && CurrentConfig && !MemMap && !NumVehicles)
		SaveFullWindowBitmap (hWndMain); 
	if (Final && !ConfigLevel && CurrentConfig && hEndDisplayCommand)
	{
		HANDLE	hCmd=hEndDisplayCommand;  
		HANDLE	hTemp=GSSiGlobAlloc (1589,GMEM_MOVEABLE,4096);
		LPSTR	pCmd=GlobalLock (hCmd);
		LPSTR	pTemp=GlobalLock (hTemp);
		BOOL	DeleteCommand = TRUE;
		
		if (*pCmd == '|') 
		{
			DeleteCommand = FALSE;
			pCmd++; 
			_fstrcpy (pTemp,pCmd);
			pCmd = pTemp;
		}
		hEndDisplayCommand = 0;
		ExpandText (pCmd); 
		if (DeleteCommand)
			GSSiGlobUlFree (&hCmd);
		else
		{
			hEndDisplayCommand = hCmd;
			GlobalUnlock (hCmd);
		}
		GSSiGlobUlFree (&hTemp);
	} 
	if (Final && CurView && !ConfigLevel)
	{
		ProcessText (CurView->EndDisplayCmd);
		DisplayAllToolbars (2);
		ClearToolbarTrackEvents (hWndMain);
	}
	if (Final && OkToContinueTime)
	{
		KillTimer (hWndMain,OKTOCONTINUETIMER);
	 	SetTimer(hWndMain, OKTOCONTINUETIMER, OkToContinueTime, (FARPROC) 0);
	}	
	BlockSocketProcessing (FALSE);
	GdiFlush ();
	if (Final)
		ResetShowOnlyVis ();
{
#if ENABLETRACE
GSSiExitProg (440);
#endif
	return;								
}
#if ENABLETRACE
}
#endif
}





				
