#include "CONFIG.h"
#include "graphint.h" 
#include "dict.h"
#define WIN31
#include "commdlg.h"
#include "dlgs.h"
#include "cddemo.h"
#include "GW.h"     
#include "MCI.H"
#include "resource.h"
#include <dyndlg.h>
#include <math.h> 
#include <float.h>   
#include <mmsystem.h>    
#include <time.h>
#include "dibapi.h"
int		symsiz = 0; 
double	SymRandom=1;
HBITMAP	hSaveLegend = NULL;
float	SymWidth=0.08;
HBITMAP	hSavePick=NULL;
RECT	SavePickRect;   
LPSTR	CrimeName;

extern	COLORREF	WindowColor; 
extern	BOOL	GetMaskArea,OutlineZoomArea,MaskZoomArea,IgnoreBounds,AutoClearOffset,DisableMarginPan,MaskOffsetLine;
extern	int		NumPicked, MaxPick;
extern	PICKDATA	PickList[MAXPICKITEMS];
extern	long	PickedStreets[MAXPICKITEMS][4];
extern	double		WidthFactor;
extern	HWND	hWndMain;
extern	HANDLE	hInst;
extern	MNMXCORD	ZoomBoxRect;
extern	BOOL        WindowIsZoomed;
extern	long		RefMktVal;
extern  HPEN		h0Pen, hRedPen;
extern	HBRUSH		hRedBrush, hGreenBrush, hBlueBrush, hBackBrush1;
extern	LPVISLIST	CurVis;
extern	LPVIEWPORT	CurView, pViewports[MAX_VIEWPORTS], pViewportsD[MAX_VIEWPORTS];
extern	HANDLE		hViewports[MAX_VIEWPORTS];
extern	int			NumViewports, NumViewportsToDisplay, DisplayViewID;
extern	LPTHEME		CurTheme;
extern	LPVOID		TranFileToBase, TranBaseToFile, TranBMToBase, TranBaseToBM;
extern	HANDLE		hTranFileToBase, hTranBaseToFile, hTranBMToBase, hTranBaseToBM;
extern	LPORTHO		CurOrtho;
extern	BOOL		Printing, PrintMerging;
extern	int			nPnts;
extern	int			PickAp;
extern	double		PickApW;
extern	BOOL		UpdateSegment;
extern	int			CommandViewport;
extern	BOOL		AddLBUTTON; 
extern	int		 	idTimer, Fid, FidConfig;
extern	long		UsedDescOffset,TranPointOffset, ColorPaletteOffset, GraphicsOffset, ContinuationOffset; 
extern	long		NewPrimeOffset;
extern	long		LenQuadSeg, LenQuad;

void ClearPick (void)
{
	if (hSavePick)
		DeleteObject (hSavePick);
	if (hSaveLegend)
		DeleteObject (hSaveLegend);   
	hSavePick=NULL;
	hSaveLegend=NULL;
	return;
}

BOOL CrimeZoom (void)
{   long	ZoomRef;
	FILE	*Fid; 
	char	str[258];

	Fid = fopen (CrimeName,"r");
	if (!Fid) return FALSE;
		  
	fgetss (str,256,Fid);
	fgetss (str,256,Fid);
	fgetss (str,256,Fid);
	fgetss (str,256,Fid); 
	ZoomRef = atol (&str[1]); 
	fclose (Fid);
	if (MaskZoomArea || OutlineZoomArea)
		GetMaskArea = TRUE;
	else
		GetMaskArea = FALSE;
	if (!PickByRefno(ZoomRef,NULL,NULL,-1))
		return FALSE;
	SetMaskArea(NumPicked-1,0);
	ZoomToPickedItem(0,100,TRUE,FALSE); 
	return TRUE;
} 

BOOL PickCrime (HWND hWnd, WORD Message, WORD wParam, LONG lParam)
{HDC hDC;
 char key;
 POINT	MousePoint;
 DPOINT	BasePoint;
 char	NearRec[256];
 static	BOOL	HaveDown=FALSE;

 switch (Message)
   {
   	case GF_INIT:
   		HaveDown = TRUE;
       	AddLBUTTON = TRUE;
   		break;

    case WM_LBUTTONDOWN: 
    	HaveDown = TRUE;
    	return FALSE;
    	
    case WM_LBUTTONUP:
    	if (!HaveDown)
    		return FALSE;
    	HaveDown = FALSE;
    	MousePoint.x = LOWORD(lParam);
	    MousePoint.y = HIWORD(lParam);
	    if (!CurView->hTranWinToBase) break;
	    BasePoint=WinPtToBasePt(MousePoint); 
		if (GetNearestCrime (BasePoint,NearRec))
		{
			DisplayCrime (NearRec);  
			PickList[0].Desc = -1;
			_fstrcpy (PickList[0].Prefix,"CRIME"); 
			_fstrcpy (PickList[0].UDI,"[CONTROL_NUMBER]");
			ExpandText (PickList[0].UDI);
	    	SetViewport(CommandViewport);
		    DisplayPickedItems (hWnd,1,TRUE,NULL);
		}
		else
		    if (hSaveLegend)
		    {
				LPVIEWPORT	SaveView;
				
		    	SaveView = CurView;
		    	SetViewport (3);
				RestoreScreen (CurView->hDC,hSaveLegend,CurView->DrawRect);
				DeleteObject (hSaveLegend);
				hSaveLegend = NULL;
				CurView = SaveView;
			}
		
		break;

    default:
    	return (FALSE);
    }
    return (TRUE);
} 

BOOL GetNearestCrime (DPOINT PickPoint,LPSTR NearRec)
{
	FILE	*Fid; 
	char	str[258]; 
	double	mindist, dist;
	long	minloc, loc;
	int		NumClass;
	UINT	i;
	DPOINT	minpoint;	
	HCURSOR	hcurSave;
	
	Fid = fopen (CrimeName,"r");
	if (!Fid) return FALSE;
	
	hcurSave = GSSiSetCursor(LoadCursor(NULL, IDC_WAIT));
 	fgetss (str,256,Fid); 
	NumClass = atoi (&str[1]);
	fgetss (str,256,Fid); 
	fgetss (str,256,Fid); 
	fgetss (str,256,Fid); 
	fgetss (str,256,Fid); 
	for (i=0;i<NumClass;i++)
		fgetss (str,256,Fid); 
	fgetss (str,256,Fid); 
	mindist = DBL_MAX;
	loc = ftell(Fid);
	i=0;
	srand (1); 
	while (fgetss (str,256,Fid))
	{   DPOINT	DPoint;
		LPSTR	Paren; 
		
		if (str[0] == '"')
		{   
			i++;
	        Paren = _fstrchr (str,',');
	        Paren++;
	        Paren++;
	        DPoint.x = atof (Paren);
	        Paren = _fstrchr (Paren,',');
	        Paren++;
	        Paren++;
	        DPoint.y = atof (Paren);
	        RandomizePoint (&DPoint,i,WinDistToWorldDist ((double)symsiz)*SymRandom);
	        dist = ldistp (DPoint,PickPoint);
	        if (dist < mindist)
	        {
	        	mindist = dist;
	        	minloc = loc;
	        	minpoint = DPoint;
	        } 
		}
		loc = ftell(Fid);
	} 
	GSSiSetCursor (hcurSave);
	if (mindist <= symsiz * CurView->BaseUnitsPerPixel * 2)
	{
		fseek (Fid,minloc,SEEK_SET);
		fgetss (NearRec,256,Fid); 
		fclose (Fid);
		hSavePick = PointAt (CurView->hDC,BasePtToWinPt(&minpoint),hSavePick,&SavePickRect);
		return TRUE; 
	}
	else
	{
		fclose (Fid);
		if (hSavePick)
		{
			RestoreScreen (CurView->hDC,hSavePick,SavePickRect);
			DeleteObject (hSavePick);
			hSavePick = NULL;
		}
		return FALSE; 
	}

}  

void DisplayCrime (LPSTR CrimeRec)
{   
	LPSTR	Paren, lpEnd; 
	int		x,y;
	DWORD	TextExt;
	int		Twidth, Theight, maxwidth;  
	char	Text[256]; 
	float	linespace=1.25;  
	HFONT	OldFont, TagFont;
	LOGFONT	LogFont;
	
	_fstrcat (CrimeRec,","); 
	SetViewport (3); 
	SaveDC (CurView->hDC);  
    LogFont.lfWidth = 0;
    LogFont.lfEscapement = 0;
    LogFont.lfOrientation = 0;
    LogFont.lfUnderline = 0;
    LogFont.lfStrikeOut = 0;
    LogFont.lfCharSet = ANSI_CHARSET;
	LogFont.lfHeight = IDNINT (15.0/200.0 * (double)(CurView->DrawRect.right - CurView->DrawRect.left));
	LogFont.lfWeight = FW_DONTCARE;
	LogFont.lfItalic = FALSE;
	LogFont.lfCharSet = ANSI_CHARSET;
	LogFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
	LogFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	LogFont.lfQuality = PROOF_QUALITY;
	LogFont.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	_fstrcpy(LogFont.lfFaceName, "Arial");
	TagFont = CreateFontIndirect((LPLOGFONT)&LogFont);
	OldFont = SelectObject(CurView->hDC, TagFont);
	
    SetDisplayMode (CurView->hDC, GF_TEXTMODE);
    if (!hSaveLegend)
    	hSaveLegend = SaveScreen (CurView->hDC,CurView->DrawRect);
	CurView->hRgn = CreateVPRgn(FALSE);
  	SelectClipRgn (CurView->hDC,CurView->hRgn);
  	DeleteObject(CurView->hRgn);
    FillRectPoly (CurView->hDC,&CurView->DrawRect,WindowColor);
    Paren = _fstrchr (CrimeRec,',');
    Paren++;
    Paren = _fstrchr (Paren,',');
    Paren++;
    Paren = _fstrchr (Paren,',');
    Paren++;
    Paren = _fstrchr (Paren,',');
    Paren++;
    Paren = _fstrchr (Paren,',');  
    Paren++;
    Paren++;  
    lpEnd = _fstrchr (Paren,'"');
    *lpEnd = '\0'; 
    
    x = CurView->DrawRect.left + 2;
    y = CurView->DrawRect.top + 20;
    _fstrcpy (Text,"Case ");
    TextExt = GetTextExtent (CurView->hDC,Text,_fstrlen(Text));
    maxwidth = LOWORD(TextExt);
    Theight = HIWORD(TextExt);
	TextOut(CurView->hDC, x, y, Text, _fstrlen(Text)); 
	y += Theight * linespace;
    _fstrcpy (Text,"Crime ");
    TextExt = GetTextExtent (CurView->hDC,Text,_fstrlen(Text));
    maxwidth = max (LOWORD(TextExt),maxwidth);
	TextOut(CurView->hDC, x, y, Text, _fstrlen(Text)); 
	y += Theight * linespace;
    _fstrcpy (Text,"Date ");
    TextExt = GetTextExtent (CurView->hDC,Text,_fstrlen(Text));
    maxwidth = max (LOWORD(TextExt),maxwidth);
	TextOut(CurView->hDC, x, y, Text, _fstrlen(Text)); 
	y += Theight * linespace;
    _fstrcpy (Text,"Time ");
    TextExt = GetTextExtent (CurView->hDC,Text,_fstrlen(Text));
    maxwidth = max (LOWORD(TextExt),maxwidth);
	TextOut(CurView->hDC, x, y, Text, _fstrlen(Text)); 
	y += Theight * linespace;
    _fstrcpy (Text,"Addr ");
    TextExt = GetTextExtent (CurView->hDC,Text,_fstrlen(Text));
    maxwidth = max (LOWORD(TextExt),maxwidth);
	TextOut(CurView->hDC, x, y, Text, _fstrlen(Text)); 
	y += Theight * linespace;

    x = CurView->DrawRect.left + 2 + maxwidth + 2;
    y = CurView->DrawRect.top + 20;
	TextOut(CurView->hDC, x, y, Paren, _fstrlen(Paren));
	
	lpEnd++;
    Paren = _fstrchr (lpEnd,',');  
    Paren++;
    Paren++;  
    lpEnd = _fstrchr (Paren,'"');
    *lpEnd = '\0';   
    
    SetGlobalValue("CONTROL_NUMBER",Paren);
    
	y += Theight * linespace; 
	lpEnd++;
    Paren = _fstrchr (lpEnd,',');  
    Paren++;
    Paren++;  
    lpEnd = _fstrchr (Paren,'"');
    *lpEnd = '\0'; 
	TextOut(CurView->hDC, x, y, Paren, _fstrlen(Paren));
    
	y += Theight * linespace; 
	lpEnd++;
    Paren = _fstrchr (lpEnd,',');  
    Paren++;
    Paren++;  
    lpEnd = _fstrchr (Paren,'"');
    *lpEnd = '\0'; 
	TextOut(CurView->hDC, x, y, Paren, _fstrlen(Paren));
    
	y += Theight * linespace; 
	lpEnd++;
    Paren = _fstrchr (lpEnd,',');  
    Paren++;
    Paren++;  
    lpEnd = _fstrchr (Paren,'"');
    *lpEnd = '\0'; 
	TextOut(CurView->hDC, x, y, Paren, _fstrlen(Paren));
    
	y += Theight * linespace; 
	lpEnd++;
    Paren = _fstrchr (lpEnd,',');  
    Paren++;
    Paren++;  
    lpEnd = _fstrchr (Paren,'"');
    *lpEnd = '\0'; 
	TextOut(CurView->hDC, x, y, Paren, _fstrlen(Paren)); 
	
	SelectObject (CurView->hDC,OldFont);
    DeleteObject (TagFont);
    RestoreDC (CurView->hDC,-1);
	
	return;
}

void DisplayCrimeThemeLegend(void)
{   int		xmargin, ymargin;
	long	h, w;
	int		width, height,x, y, fHeight, MaxTextWidth;
	HBRUSH	BkBrush;
	int		iclass;
	RECT	ClassColorBox;
	HFONT	hfont, hfontOld, hfont2;
	DWORD	TextExtent;
	char	Text[256], DateRange[64], AreaName[64];
	char	Val1[32], Val2[32];
	long	TotCount;
	float	Pct;
	int		MinFontHeight=5;
	int		inc;
	FILE	*Fid; 
	char	str[258]; 
	long	FIDloc;  
	int		iLogPixsX;
	LPVIEWPORT	SaveView; 
	LPSTR   Quote;
	UINT	i; 
	LPINT	CrimeID;
	LPLONG	CNum; 
	WORD    wState;
	BOOL	PrintReport=FALSE;

	wState = GetMenuState(GetMenu(hWndMain), IDM_CRIME_INCLUDE_REPORT, MF_BYCOMMAND);
 	if(wState == (wState | MF_CHECKED) && Printing)  
 		PrintReport = TRUE;
	ClearPick ();

	Fid = fopen (CrimeName,"r");
	if (!Fid) return;
		  
	fgetss (str,256,Fid);
	CurTheme->NumClass = atoi (&str[1]);
	fgetss (str,256,Fid); 
    Strip (str,'"');
	_fstrcpy (CurTheme->Title,str);
	fgetss (str,256,Fid); 
	_fstrcpy (AreaName,&str[1]);
	Quote=_fstrchr(AreaName,'"');
	if (Quote) *Quote='\0';
	fgetss (str,256,Fid);
	fgetss (str,256,Fid); 
	_fstrcpy (DateRange,&str[1]);
	Quote=_fstrchr(DateRange,'"');
	if (Quote) *Quote='\0';
	

    SetDisplayMode (CurView->hDC, GF_TEXTMODE);
  	SelectClipRgn (CurView->hDC,NULL);
	CurTheme->Rect=PctRect (CurView->DrawRect,-CurTheme->Margin);
	iLogPixsX = GetDeviceCaps(CurView->hDC, LOGPIXELSX);  
	symsiz = SymWidth * iLogPixsX;
	xmargin = (((long)CurTheme->Rect.right - CurTheme->Rect.left) * CurTheme->InnerMargin) / 100;
	ymargin = (((long)CurTheme->Rect.bottom - CurTheme->Rect.top) * CurTheme->InnerMargin) / 100;
	h = (((long)CurTheme->Rect.bottom - CurTheme->Rect.top) * CurTheme->TitleHeight) / 100;
	CurTheme->TitleBox.top = CurTheme->Rect.top + ymargin;
	CurTheme->TitleBox.bottom = CurTheme->TitleBox.top + h;
	CurTheme->TitleBox.left = CurTheme->Rect.left + xmargin;
	CurTheme->TitleBox.right = CurTheme->Rect.right - xmargin;
	CurTheme->InfoBox.top = CurTheme->TitleBox.bottom + ymargin-5;
	CurTheme->InfoBox.bottom = CurTheme->Rect.bottom - ymargin -5;
	CurTheme->InfoBox.left = CurTheme->Rect.left + xmargin;
	CurTheme->InfoBox.right = CurTheme->Rect.right - ymargin;
	
	CurTheme->ScatterBox = CurTheme->InfoBox;
	w = (((long)CurTheme->InfoBox.right - CurTheme->InfoBox.left) * CurTheme->ScatterWidth) / 100;
	if (!CurTheme->DisplayScatterDiagram) w=0;
	CurTheme->ScatterBox.right = CurTheme->InfoBox.left + w;

	FillRectPoly (CurView->hDC,&CurTheme->Rect,CurTheme->BGColor);
	FillRectPoly (CurView->hDC,&CurTheme->InfoBox,RGB(255,255,255));
	FillRectPoly (CurView->hDC,&CurTheme->TitleBox,CurTheme->TitleBoxBG);

	CurTheme->TitleFont.lfHeight = -MulDiv(CurTheme->TitleHeight,
										   GetDeviceCaps(CurView->hDC, LOGPIXELSY), 72);
	hfont = CreateFontIndirect((PLOGFONT)&CurTheme->TitleFont);
	hfontOld = SelectObject(CurView->hDC, hfont);
	TextExtent = GetTextExtent (CurView->hDC,CurTheme->Title,_fstrlen(CurTheme->Title));
	width = LOWORD (TextExtent);
	height = HIWORD (TextExtent);
	x = CurTheme->TitleBox.left + ((CurTheme->TitleBox.right - CurTheme->TitleBox.left) - width)/2;
	y = CurTheme->TitleBox.top + 1;

	TextOut(CurView->hDC, x, y, CurTheme->Title, _fstrlen(CurTheme->Title));
	TextExtent = GetTextExtent (CurView->hDC,DateRange, _fstrlen(AreaName));
	width = LOWORD (TextExtent);
	x = CurTheme->TitleBox.left + ((CurTheme->TitleBox.right - CurTheme->TitleBox.left) - width)/2;
	TextOut(CurView->hDC, x, (y+=height), AreaName, _fstrlen(AreaName));
	TextExtent = GetTextExtent (CurView->hDC,DateRange, _fstrlen(DateRange));
	width = LOWORD (TextExtent);
	x = CurTheme->TitleBox.left + ((CurTheme->TitleBox.right - CurTheme->TitleBox.left) - width)/2;
	TextOut(CurView->hDC, x, y+height, DateRange, _fstrlen(DateRange));

	ThemeDisplayScatterDiagram();

	/* Display the color boxes */
	if (CurTheme->DisplayScatterDiagram)
		inc = 0; /* boxes connect */
	else
		inc = ymargin / 2;
	ClassColorBox.left = CurTheme->ScatterBox.right+2;
	w = (((long)CurTheme->InfoBox.right - CurTheme->InfoBox.left) * CurTheme->ColorsWidth) / 100;
	ClassColorBox.right = ClassColorBox.left + w;
	ClassColorBox.bottom = CurTheme->ScatterBox.bottom-inc;
	h = CurTheme->ScatterBox.bottom - CurTheme->ScatterBox.top + 1;
	for (iclass=0;iclass<CurTheme->NumClass;iclass++)
	{   POINT	wPoint;
	
		ClassColorBox.top = inc + CurTheme->ScatterBox.top + (iclass * h) / CurTheme->NumClass;
	/*	FillRectPoly (CurView->hDC,&ClassColorBox,CurTheme->ClassColor[iclass]);*/    
		wPoint.x = ClassColorBox.left+symsiz*2;
		wPoint.y = ClassColorBox.top+symsiz*2+ymargin;
	    DrawSymbol (CurView->hDC,(iclass%4)+3300,wPoint,symsiz*2,CurTheme->ClassColor[iclass]);
	
		CurTheme->ClassClrBox[iclass] = ClassColorBox;
        ClassColorBox.bottom=ClassColorBox.top - inc;

	}

	fHeight = 12;
	x =  ClassColorBox.left + symsiz*4 + xmargin;
	width = CurTheme->InfoBox.right - CurTheme->ClassClrBox[0].right - xmargin*2;
	MaxTextWidth = INT_MAX;
	FIDloc = ftell (Fid);
	while (MaxTextWidth > width)
	{   if (fHeight <= MinFontHeight) goto TooSmall;
		fHeight--;
		SelectObject(CurView->hDC, hfontOld);
		if (hfont) DeleteObject(hfont);

		MaxTextWidth = 0;
		CurTheme->ClassFont1.lfHeight = -MulDiv(fHeight,GetDeviceCaps(CurView->hDC, LOGPIXELSY), 72);
		hfont = CreateFontIndirect((PLOGFONT)&CurTheme->ClassFont1);
		hfontOld = SelectObject(CurView->hDC, hfont);
        TotCount = 0;
	    fseek (Fid,FIDloc,SEEK_SET);
		for (iclass=0;iclass<CurTheme->NumClass;iclass++)
		{   CurTheme->ClassCount[iclass]=0;
           	fgetss (Text,256,Fid); 

           	TextExtent = GetTextExtent (CurView->hDC,Text,_fstrlen(Text));
           	MaxTextWidth = max (MaxTextWidth,LOWORD (TextExtent));
		}
    }

TooSmall:	fHeight -=1;
	CurTheme->ClassFont1.lfHeight = -MulDiv(fHeight,GetDeviceCaps(CurView->hDC, LOGPIXELSY), 72);
/*	hfont2 = CreateFontIndirect((PLOGFONT)&CurTheme->ClassFont1);*/
    fseek (Fid,FIDloc,SEEK_SET);
	for (iclass=0;iclass<CurTheme->NumClass;iclass++)
	{
       	y = CurTheme->ClassClrBox[iclass].top+ymargin;
       	_fstrcpy (Val1,ValueConv ((double)CurTheme->ClassMin[iclass],CurTheme->ValConv,CurTheme->RoundTo,CurTheme->AddCommas));
       	_fstrcpy (Val2,ValueConv ((double)CurTheme->ClassMax[iclass],CurTheme->ValConv,CurTheme->RoundTo,CurTheme->AddCommas));

/*        SelectObject(CurView->hDC, hfont);*/
        fgetss (Text,256,Fid); 
        Strip (Text,'"');
		TextOut(CurView->hDC, x, y, Text, _fstrlen(Text));
	}


	fgetss (str,256,Fid); 
	SaveView = CurView;
	SetViewport (CurTheme->TargetViewport);
	CurView->hRgn = CreateVPRgn(FALSE);
  	SelectClipRgn (CurView->hDC,CurView->hRgn);
  	DeleteObject(CurView->hRgn);
	iLogPixsX = GetDeviceCaps(CurView->hDC, LOGPIXELSX);  
	symsiz = SymWidth * iLogPixsX; 
	
	if (hMemPrintCrime)
		GlobalFree (hMemPrintCrime);
	hMemPrintCrime = GSSiGlobAlloc (GMEM_MOVEABLE,UINT_MAX); 
	CrimeID = GlobalLock (hMemPrintCrime); 
	*CrimeID = 0;
	CNum = CrimeID;
	CNum++;
	i=0;
	srand (1); 
	while (fgetss (str,256,Fid))
	{   DPOINT	DPoint;
		POINT	wPoint; 
		LPSTR	Paren; 
		RECT	Rect;
		long	CntlNum; 
		
		
		if (str[0] == '"')
		{   
			i++;
	        iclass = atoi(&str[1])-1;
	        CurTheme->ClassCount[iclass]++; 
	        TotCount++;
	        Paren = _fstrchr (str,',');
	        Paren++;
	        Paren++;
	        DPoint.x = atof (Paren);
	        Paren = _fstrchr (Paren,',');
	        Paren++;
	        Paren++;
	        DPoint.y = atof (Paren); 
	        Paren = _fstrchr (Paren,',');
	        Paren++;
	        Paren++;
	        Paren = _fstrchr (Paren,',');
	        Paren++;
	        Paren++;
	        Paren = _fstrchr (Paren,',');
	        Paren++;
	        Paren++;
	        Paren = _fstrchr (Paren,',');
	        Paren++;
	        Paren++; 
	        CntlNum = atol (Paren);
	        RandomizePoint (&DPoint,i,WinDistToWorldDist ((double)symsiz)*SymRandom);
	        wPoint = BasePtToWinPt (&DPoint);  
	        
	        if (PtInRect (&CurView->DrawRect,wPoint))
	        {   
		    	DrawSymbol (CurView->hDC,(iclass%4)+3300,wPoint,symsiz,CurTheme->ClassColor[iclass]);
		    	if (PrintReport)
		    	{   
		    		char	txt[16]; 
		    		COLORREF	OldColor;
		    		
		        	(*CrimeID)++;
		        	*CNum++ = CntlNum; 
		        	itoa (*CrimeID,txt,10); 
		        	OldColor=SetBkColor(CurView->hDC, RGB(255,255,255));
					DispText (CurView->hDC,wPoint.x-10,wPoint.x+10, wPoint.y, 0,1,3, (float)symsiz/iLogPixsX,
								  100, FALSE, 0,txt,0,FALSE,0,0,-1);  
					SetBkColor(CurView->hDC, OldColor);
		        }
		    }
		}
        
	}
	fclose (Fid); 
	if (hMemPrintCrime)
		GlobalUnlock (hMemPrintCrime); 
	if (!PrintReport)
	{
		GlobalFree (hMemPrintCrime);
		hMemPrintCrime=0;
	}
	CurView = SaveView;
  	SelectClipRgn (CurView->hDC,NULL); 
  	if (TotCount)
	for (iclass=0;iclass<CurTheme->NumClass;iclass++)
	{
		Pct = (100.0 * CurTheme->ClassCount[iclass]) / TotCount;
		sprintf (Text,"%ld (%5.1f%%)",CurTheme->ClassCount[iclass],Pct);
/*        SelectObject(CurView->hDC, hfont2);*/
       	TextExtent = GetTextExtent (CurView->hDC,Text,_fstrlen(Text));
       	y = CurTheme->ClassClrBox[iclass].top+ymargin + 1.15*HIWORD(TextExtent);
		TextOut(CurView->hDC, x+MaxTextWidth/2-LOWORD(TextExtent)/2,
							  y, Text, _fstrlen(Text));
	}
	sprintf (Text,"Total: %ld",TotCount);
   	TextExtent = GetTextExtent (CurView->hDC,Text,_fstrlen(Text));
   	y += 1.15*HIWORD(TextExtent);
	TextOut(CurView->hDC, x+MaxTextWidth/2-LOWORD(TextExtent)/2,
						  y, Text, _fstrlen(Text));

	SelectObject(CurView->hDC, hfontOld);
	DeleteObject(hfont);
/*	DeleteObject(hfont2);*/
	return;
}

