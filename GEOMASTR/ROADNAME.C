#include "graphint.h"
#define MAXLABELLINES	4096  
#define MAXPOINTSINLABEL 8160


#define MAXSYMBOLLINES	4096  
#define MAXSHIELDSDISPLAYED	2730   
#define MAX_SHIELD_ID 24

static	HANDLE	hSymbolLines[MAXSYMBOLLINES];
static	COLORREF	SymbolLineColor[MAXSYMBOLLINES];
static	USHORT	nSymbolLines=0;  
static	POINT	ShieldDisplayPoint[MAXSHIELDSDISPLAYED];
static	char	ShieldID[MAXSHIELDSDISPLAYED][MAX_SHIELD_ID]; 
static	short	NumShieldsDisplayed=0;

typedef struct	{int	NumPoints;
				 int	Streets[4];
				 short	HollowStreetWidth;
				 short	Order;
				 short  OneWay;
				 COLORREF	OutlineColor;
				 COLORREF	FillColor;
				}STREETHEADER;
typedef STREETHEADER	*LPSTREETHEADER;

static	struct	{long npnts; short width, desc, OneWay, order; COLORREF color;} HollowLineHeader;          


int	ShowHollowStreet=1;

#include "gmextern.h"

void LinkLabelLines (short Line1,short Line2,short Type2, short Type1);
void LinkSymbolLines (short Line1,short Line2,short Type2, short Type1);
BOOL DoesSymConnectToMiddleOfAnother (HPFPOINT	pPoint,short skip,short SymNum);

void ResetStreetLabels (void)
{   
	UINT	i;
	
	ShowHollowStreet = GetGlobalLVal2 ("[%SHOWHOLLOWSTREETS]",1);
	if (!CurTheme || CurTheme->ID != GF_STREET_TEXT_THEME)
		return;
	if (CurTheme->hhLabelLines)
	{
		LPHANDLE phLabelLines = GlobalLock (CurTheme->hhLabelLines);

		for (i=0;i< CurTheme->nLabelLines; i++)
			GSSiGlobFree (&phLabelLines[i]);
		GSSiGlobUlFree (&CurTheme->hhLabelLines);
	}
	CurTheme->nLabelLines = 0;
	return;
}

BOOL AddToStreetSegmentList (HPPOINT Points, int np,int Width,int Order,COLORREF FillColor,COLORREF OutlineColor)        //Points[np-1]
{   
	short	ConnectedTo = -1, HowConnected;
	short	i,j,k;   
	long	NewNumPoints;
	LPSTREETHEADER	pStreet;
//	LPSHORT	pNumPoints; 
//	LPLONG	pStreets;  
//	LPSHORT	pHollowStreetWidth;	
	LPPOINT	pPoints;
	LPHANDLE	phLabelLines;
	BOOL	rtn=TRUE;

//	if (CurStreetNumbers[0] != 8373)
//		return TRUE;
	CurStreetNumbers[3] = CurState;
	if (!CurTheme->hhLabelLines)
		CurTheme->hhLabelLines = GSSiGlobAlloc (1729,GMEM_MOVEABLE,sizeof(HANDLE)*MAXLABELLINES);
	phLabelLines = GlobalLock (CurTheme->hhLabelLines);
	for (i=0;i<CurTheme->nLabelLines;i++)
	{   
		pStreet = (LPSTREETHEADER)GlobalLock (phLabelLines[i]);  
//		pStreets   = (LPLONG)(pNumPoints+1);   
//		pHollowStreetWidth = (LPSHORT)(pStreets+4);
//		pPoints = (LPPOINT)(pHollowStreetWidth+1); 
		pPoints = (LPPOINT)(pStreet+1);
		if (!_fmemcmp (CurStreetNumbers,pStreet->Streets,16) &&
			StreetOneWay == pStreet->OneWay &&
			Order == pStreet->Order && Width == pStreet->HollowStreetWidth &&
			FillColor == pStreet->FillColor && OutlineColor == pStreet->OutlineColor &&
			pStreet->NumPoints+np < MAXPOINTSINLABEL-2)
		{
			if (pPoints[0].x == Points[0].x && pPoints[0].y == Points[0].y)
			{
				if (ConnectedTo != -1)
				{
					GlobalUnlock (phLabelLines[i]);
					LinkLabelLines (ConnectedTo,i,1,HowConnected);
					goto Exit;  
				} 
				NewNumPoints = pStreet->NumPoints+np;
				GlobalUnlock (phLabelLines[i]);
		    	phLabelLines[i] = GSSiGlobalReAlloc (0,phLabelLines[i] ,sizeof(STREETHEADER) + sizeof(POINT)*NewNumPoints,GHND);
				pStreet = (LPSTREETHEADER)GlobalLock (phLabelLines[i]);  
//				pStreets   = (LPLONG)(pNumPoints+1);   
//				pHollowStreetWidth = (LPSHORT)(pStreets+4);
				pPoints = (LPPOINT)(pStreet+1); 
				_fmemmove (&pPoints[np-1],&pPoints[0],pStreet->NumPoints*sizeof(POINT)); 
				k = np - 1;
				for (j=0;j<np-1;j++)
					pPoints[j] = Points[k--];       //pPoints[*pNumPoints-1]
				(pStreet->NumPoints)+=(np-1);  
				GlobalUnlock (phLabelLines[i]);
				HowConnected = 1;
				ConnectedTo = i;
				continue;
			}
			if (pPoints[0].x == Points[np-1].x && pPoints[0].y == Points[np-1].y)
			{
				if (ConnectedTo != -1)
				{
					GlobalUnlock (phLabelLines[i]); 
					LinkLabelLines (ConnectedTo,i,1,HowConnected);
					goto Exit;  
				}
				NewNumPoints = pStreet->NumPoints+np;
				GlobalUnlock (phLabelLines[i]);
		    	phLabelLines[i] = GSSiGlobalReAlloc (0,phLabelLines[i] ,sizeof(STREETHEADER) + sizeof(POINT)*NewNumPoints,GHND);
				pStreet = (LPSTREETHEADER)GlobalLock (phLabelLines[i]);  
//				pStreets   = (LPLONG)(pNumPoints+1);   
//				pHollowStreetWidth = (LPSHORT)(pStreets+4);
				pPoints = (LPPOINT)(pStreet+1); 
				_fmemmove (&pPoints[np-1],&pPoints[0],pStreet->NumPoints*sizeof(POINT));
				for (j=0;j<np-1;j++)
					pPoints[j] = Points[j];
				(pStreet->NumPoints)+=(np-1);  
				GlobalUnlock (phLabelLines[i]); 
				HowConnected = 1;
				ConnectedTo = i;
				continue;
			}
			if (pPoints[(pStreet->NumPoints)-1].x == Points[0].x && pPoints[(pStreet->NumPoints)-1].y == Points[0].y)
			{
				if (ConnectedTo != -1)
				{
					GlobalUnlock (phLabelLines[i]); 
					LinkLabelLines (ConnectedTo,i,2,HowConnected);
					goto Exit;  
				}
				NewNumPoints = pStreet->NumPoints+np;
				GlobalUnlock (phLabelLines[i]);
		    	phLabelLines[i] = GSSiGlobalReAlloc (0,phLabelLines[i] ,sizeof(STREETHEADER) + sizeof(POINT)*NewNumPoints,GHND);
				pStreet = (LPSTREETHEADER)GlobalLock (phLabelLines[i]);  
		//		pStreets   = (LPLONG)(pNumPoints+1);   
		//		pHollowStreetWidth = (LPSHORT)(pStreets+4);
				pPoints = (LPPOINT)(pStreet+1); 
				for (j=0;j<np-1;j++)
					pPoints[pStreet->NumPoints+j] = Points[j+1];   //pPoints[*pNumPoints-2]
				(pStreet->NumPoints)+=(np-1);  
				GlobalUnlock (phLabelLines[i]); 
				HowConnected = 2;
				ConnectedTo = i;
				continue;
			}
			if (pPoints[(pStreet->NumPoints)-1].x == Points[np-1].x && pPoints[(pStreet->NumPoints)-1].y == Points[np-1].y)
			{
				if (ConnectedTo != -1)
				{
					GlobalUnlock (phLabelLines[i]); 
					LinkLabelLines (ConnectedTo,i,2,HowConnected);
					goto Exit;  
				}
				NewNumPoints = pStreet->NumPoints+np;
				GlobalUnlock (phLabelLines[i]);
		    	phLabelLines[i] = GSSiGlobalReAlloc (0,phLabelLines[i] ,sizeof(STREETHEADER) + sizeof(POINT)*NewNumPoints,GHND);
				pStreet = (LPSTREETHEADER)GlobalLock (phLabelLines[i]);  
		//		pStreets   = (LPLONG)(pNumPoints+1);   
		//		pHollowStreetWidth = (LPSHORT)(pStreets+4);
				pPoints = (LPPOINT)(pStreet+1); 
				k = np - 2;
				for (j=0;j<np-1;j++)
					pPoints[pStreet->NumPoints+j] = Points[k--];
				(pStreet->NumPoints)+=(np-1);  
				GlobalUnlock (phLabelLines[i]); 
				HowConnected = 2;
				ConnectedTo = i;
				continue;
			}
		}
		GlobalUnlock (phLabelLines[i]); 
	} 
	if (ConnectedTo == -1 && CurTheme->nLabelLines < MAXLABELLINES && np < MAXPOINTSINLABEL)
	{
		phLabelLines[CurTheme->nLabelLines] = GSSiGlobAlloc (0,GMEM_MOVEABLE,sizeof(STREETHEADER) + sizeof(POINT) * np);		
		pStreet = (LPSTREETHEADER)GlobalLock (phLabelLines[CurTheme->nLabelLines]);
		pStreet->NumPoints = np; 
	//	pStreets = (LPLONG)(pNumPoints+1);  
		_fmemcpy (pStreet->Streets,CurStreetNumbers,16);   
	//	pHollowStreetWidth = (LPSHORT)(pStreets+4);
		pPoints = (LPPOINT)(pStreet+1); 
		pStreet->HollowStreetWidth = Width;
		pStreet->OneWay = StreetOneWay;
		pStreet->Order = Order;
		pStreet->FillColor = FillColor;
		pStreet->OutlineColor = OutlineColor;
		_fmemcpy (pPoints,Points,np*sizeof(POINT));
		GlobalUnlock (phLabelLines[CurTheme->nLabelLines++]);
	}
	rtn = TRUE;
Exit: 
	GlobalUnlock (CurTheme->hhLabelLines);
	return rtn;
}

BOOL AddToLayeredSymbolList (HPFPOINT Points,long np,int SymNum)
{   
	short	ConnectedTo = -1, HowConnected;
	short	i,j,k;
	HPFPOINT	pPoints;
	LPSHORT	pNumPoints, pSymNum;
	LPTHEME *pIsCenterline; 

	for (i=0;i<nSymbolLines;i++)
	{   
		pNumPoints = (LPSHORT)GlobalLock (hSymbolLines[i]);  
		pSymNum   = (LPSHORT)(pNumPoints+1); 
		pIsCenterline = (LPTHEME*)(pSymNum+1);  
		pPoints = (HPFPOINT)(pIsCenterline+1);
		if (*pSymNum == SymNum && *pNumPoints+np < MAXPOINTSINLABEL-2)
		{
			if (pPoints[0].x == Points[0].x && pPoints[0].y == Points[0].y)
			{
				if (ConnectedTo != -1)
				{
					GlobalUnlock (hSymbolLines[i]);
					LinkSymbolLines (ConnectedTo,i,1,HowConnected);
					goto Exit;  
				}
				_fmemmove (&pPoints[np-1],&pPoints[0],*pNumPoints*sizeof(FPOINT)); 
				k = np - 1;
				for (j=0;j<np-1;j++)
					pPoints[j] = Points[k--];       //pPoints[*pNumPoints-1]
				(*pNumPoints)+=(np-1);  
				GlobalUnlock (hSymbolLines[i]);
				HowConnected = 1;
				ConnectedTo = i;
				continue;
			}
			if (pPoints[0].x == Points[np-1].x && pPoints[0].y == Points[np-1].y)
			{
				if (ConnectedTo != -1)
				{
					GlobalUnlock (hSymbolLines[i]); 
					LinkSymbolLines (ConnectedTo,i,1,HowConnected);
					goto Exit;  
				}
				_fmemmove (&pPoints[np-1],&pPoints[0],*pNumPoints*sizeof(FPOINT));
				for (j=0;j<np-1;j++)
					pPoints[j] = Points[j];
				(*pNumPoints)+=(np-1);  
				GlobalUnlock (hSymbolLines[i]); 
				HowConnected = 1;
				ConnectedTo = i;
				continue;
			}
			if (pPoints[(*pNumPoints)-1].x == Points[0].x && pPoints[(*pNumPoints)-1].y == Points[0].y)
			{
				if (ConnectedTo != -1)
				{
					GlobalUnlock (hSymbolLines[i]); 
					LinkSymbolLines (ConnectedTo,i,2,HowConnected);
					goto Exit;  
				}
				for (j=0;j<np-1;j++)
					pPoints[*pNumPoints+j] = Points[j+1];   //pPoints[*pNumPoints-2]
				(*pNumPoints)+=(np-1);  
				GlobalUnlock (hSymbolLines[i]); 
				HowConnected = 2;
				ConnectedTo = i;
				continue;
			}
			if (pPoints[(*pNumPoints)-1].x == Points[np-1].x && pPoints[(*pNumPoints)-1].y == Points[np-1].y)
			{
				if (ConnectedTo != -1)
				{
					GlobalUnlock (hSymbolLines[i]); 
					LinkSymbolLines (ConnectedTo,i,2,HowConnected);
					goto Exit;  
				}
				k = np - 2;
				for (j=0;j<np-1;j++)
					pPoints[*pNumPoints+j] = Points[k--];
				(*pNumPoints)+=(np-1);  
				GlobalUnlock (hSymbolLines[i]); 
				HowConnected = 2;
				ConnectedTo = i;
				continue;
			}
		}
		GlobalUnlock (hSymbolLines[i]); 
	} 
	if (ConnectedTo == -1 && nSymbolLines < MAXSYMBOLLINES)
	{
		hSymbolLines[nSymbolLines] = GSSiGlobAlloc (0,GMEM_MOVEABLE,MAXPOINTSINLABEL*sizeof(FPOINT)+2*sizeof(short)+sizeof(LPTHEME*));		
		pNumPoints = (LPSHORT)GlobalLock (hSymbolLines[nSymbolLines]);
		*pNumPoints = np; 
		pSymNum   = (LPSHORT)(pNumPoints+1); 
		pIsCenterline = (LPTHEME*)(pSymNum+1);   
		*pIsCenterline = StreetCenterline;
		pPoints = (HPFPOINT)(pIsCenterline+1);
		*pSymNum = SymNum;
		hmemcpy (pPoints,Points,np*sizeof(FPOINT)); //Points[np-1]
		if (HaveVarFillColor)
			SymbolLineColor[nSymbolLines] = GlobalColors[0];
		else
			SymbolLineColor[nSymbolLines] = -1;
		GlobalUnlock (hSymbolLines[nSymbolLines++]);
	}
Exit: 
	return TRUE;
} 

void AddShieldToDisplayedList (POINT Point,LPSTR Text,LPSTR Dir)
{   
	char	ThisShieldID[128];
	
	sprintf (ThisShieldID,"%s%s",Text,Dir);   
	ThisShieldID[31] = 0;
	if (NumShieldsDisplayed >= MAXSHIELDSDISPLAYED)
		MessageBox (0,"Number of shields exceeds maximum",NULL,MB_ICONEXCLAMATION);
	else
	{
		ShieldDisplayPoint[NumShieldsDisplayed] = Point;
		_fstrncpy (ShieldID[NumShieldsDisplayed++],ThisShieldID,MAX_SHIELD_ID-1); 
	}
	return;   
}

int HaveShieldWithinMinDist (POINT Point,double MinDistBetweenShields,LPSTR Text,LPSTR Dir)  
{   
	USHORT	i; 
	char	ThisShieldID[128];
	char	str[256];
	DPOINT DPoint = ScreenPtToBasePt (Point);

	sprintf (ThisShieldID,"%s%s",Text,Dir);   
	ThisShieldID[31] = 0;
	if (ShieldSaveFile[0] != 0)
	{
		sprintf (str,"[GRIDID]\t[GRIDCELLID]\t%f\t%f\t%s",DPoint.x,DPoint.y,ThisShieldID);
		ExpandText (str);
		AppendFile (ShieldSaveFile,str);
		return 2;
	}
	if (!MinDistBetweenShields)
		return 0;
	for (i=0;i<NumShieldsDisplayed;i++)
	{
		if (!_fstricmp (ThisShieldID,ShieldID[i]) && idist (Point,ShieldDisplayPoint[i]) < MinDistBetweenShields)
			return 1;
	}
	return 0;
}

BOOL DoesSymConnectToMiddleOfAnother (HPFPOINT	pPoint,short skip,short SymNum)
{
	short	i,j;
	HPFPOINT	pPoints;
	LPSHORT	pNumPoints, pSymNum;
	LPTHEME *pIsCenterline; 
    
   // return FALSE;
	for (i=0;i<nSymbolLines;i++)
	{   
		if (i != skip)
		{
			pNumPoints = (LPSHORT)GlobalLock (hSymbolLines[i]);  
			pSymNum   = (LPSHORT)(pNumPoints+1); 
			pIsCenterline = (LPTHEME*)(pSymNum+1);  
			if (*pSymNum == SymNum)
			{ 
				pPoints = (HPFPOINT)(pIsCenterline+1);
				for (j=1;j<*pNumPoints-1;j++)
					if (pPoint->x == pPoints[j].x && pPoint->y == pPoints[j].y)
					{
						GlobalUnlock (hSymbolLines[i]);  
						return TRUE;
					}
			}
			GlobalUnlock (hSymbolLines[i]);  
		}
	} 
	return FALSE;
}

BOOL DisplayLayeredSymbols (HDC hDC,BOOL Clear)
{
	short	i;
	HPFPOINT	pPoints;
	BOOL	WantPreSym, WantPostSym;	
 	LPSHORT	pNumPoints, pSymNum;
	LPTHEME *pIsCenterline; 
   
    if (!nSymbolLines)
    	return FALSE;
	if (!Clear)    
	{   
		WaitCursor (1);
	 	SaveDC (hDC);
		InitRecord (CurView->hDC); 
		SetROP2(CurView->hDC,DisplayRasterOpt);
		SetDisplayMode (CurView->hDC, GF_TEXTMODE); 
		GSSiDeleteObject(&CurView->hRgn);
		CurView->hRgn = CreateVPRgn(FALSE,FALSE);
		SelectClipRgn (CurView->hDC,CurView->hRgn);
		GSSiDeleteObject(&CurView->hRgn);
		_fmemset (CurStreetNumbers,0,sizeof(CurStreetNumbers));
		for (i=0;i<nSymbolLines;i++)
		{   
			if ((int)SymbolLineColor[i] > -1)
			{
				HaveVarFillColor = TRUE;
				ThemePointColor = GlobalColors[0]= SymbolLineColor[i];
			}
			else
				HaveVarFillColor = FALSE;
			pNumPoints = (LPSHORT)GlobalLock (hSymbolLines[i]);  
			pSymNum   = (LPSHORT)(pNumPoints+1); 
			pIsCenterline = (LPTHEME*)(pSymNum+1);  
			pPoints = (HPFPOINT)(pIsCenterline+1); //pPoints[2]
			WantPreSym = !DoesSymConnectToMiddleOfAnother (pPoints,i,*pSymNum); 
			WantPostSym = !DoesSymConnectToMiddleOfAnother (&pPoints[*pNumPoints-1],i,*pSymNum); 
			PlotLinearItem (hDC,pPoints,*pNumPoints,-*pSymNum,0,0,0,WantPreSym,WantPostSym,0); //pPoints[1]  
			if (*pIsCenterline)
			{   
				HPPOINT	lpPoints16;
				HANDLE	Handle = GSSiGlobAlloc ( 768,GMEM_MOVEABLE,((long)*pNumPoints+16L) * (long)sizeof(POINT));
				short	j; 
				BOOL	SaveUseFlatEndPolyline=UseFlatEndPolyline;
				LPTHEME	SaveTheme = CurTheme;
				
				CurTheme = *pIsCenterline;
				UseFlatEndPolyline = TRUE;
				lpPoints16 = (HPPOINT)GlobalLock (Handle);
				for (j=0;j<*pNumPoints;j++,pPoints++)
					lpPoints16[j] = FPointToPoint (*pPoints);   
			    GWPolylineScreen2 (hDC,lpPoints16,*pNumPoints,*pSymNum); 
			    GSSiGlobUlFree (&Handle);   
			    UseFlatEndPolyline = SaveUseFlatEndPolyline;
				CurTheme = SaveTheme;
            }
			GlobalUnlock (hSymbolLines[i]); 
		} 
	 	RestoreDC (hDC,-1); 
		WaitCursor (-1);
	}
	for (i=0;i<nSymbolLines;i++) 
		GSSiGlobFree(&hSymbolLines[i]);
    nSymbolLines = 0;
	return TRUE;
}

BOOL DisplayStreetCenterlines (void)
{
	LPSTREETHEADER	pStreet;
	LPPOINT	pPoints;   
	HPEN	hPen, hOldPen;
	LOGBRUSH	lb;
	COLORREF	WHITE=RGB(255,255,255);
	int		i;
	int		EdgeWidth = 2;
	int		MinWidth = INT_MAX, MaxWidth=INT_MIN, MinOrder=INT_MAX, MaxOrder=INT_MIN, order;
	int		MaxEdgeWidth = INT_MIN;
	BOOL	rtn = TRUE;

	lb.lbStyle = BS_SOLID;
	lb.lbHatch = 0;

	if (CurTheme->hhLabelLines)
	{
		LPHANDLE	phLabelLines = GlobalLock (CurTheme->hhLabelLines);

		switch (ShowHollowStreet)
		{
		case 0:
			rtn = FALSE;
			break;
		case 1:
		case 2:

			SaveDC (CurView->hDC); 
			InitRecord (CurView->hDC); 
			SetROP2(CurView->hDC,DisplayRasterOpt);
			SetDisplayMode (CurView->hDC, GF_TEXTMODE); 
			GSSiDeleteObject(&CurView->hRgn);
			CurView->hRgn = CreateVPRgn(FALSE,FALSE);
			SelectClipRgn (CurView->hDC,CurView->hRgn);

			for (i=0;i<CurTheme->nLabelLines;i++)   
			{
 				pStreet = (LPSTREETHEADER)GlobalLock (phLabelLines[i]); 
				pPoints = (LPPOINT)(pStreet+1);  
				MinWidth = min (MinWidth,pStreet->HollowStreetWidth);
				MaxWidth = max (MaxWidth,pStreet->HollowStreetWidth);
				EdgeWidth = pStreet->HollowStreetWidth / 8 +1;
				MaxEdgeWidth = max (MaxEdgeWidth,EdgeWidth);
				GlobalUnlock (phLabelLines[i]); 
			}
			MaxWidth += MaxEdgeWidth;
			for (i=0;i<CurTheme->nLabelLines;i++)   
			{
 				pStreet = (LPSTREETHEADER)GlobalLock (phLabelLines[i]); 
				pPoints = (LPPOINT)(pStreet+1);  
				MinOrder = min (MinOrder,pStreet->Order);
				MaxOrder = max (MaxOrder,pStreet->Order);
			//	hPen = CreatePen (PS_SOLID,pStreet->HollowStreetWidth+2*DeviceToScreenFactor,pStreet->OutlineColor);  
				if (ShowHollowStreet == 1)
				{
					lb.lbColor = pStreet->OutlineColor;
					EdgeWidth = pStreet->HollowStreetWidth / 8 + 1;
					hPen = ExtCreatePen(PS_GEOMETRIC | PS_SOLID | PS_ENDCAP_FLAT | PS_JOIN_BEVEL, IDNINT(pStreet->HollowStreetWidth + EdgeWidth * 2), &lb, 0, 0);

					hOldPen = SelectObject(CurView->hDC, hPen);
					Polyline(CurView->hDC, pPoints, pStreet->NumPoints);
					SelectObject(CurView->hDC, hOldPen);
					GSSiDeleteObject(&hPen);
				}
				GlobalUnlock (phLabelLines[i]); 
			}
			if (ShowHollowStreet == 1)
			{
				for (order = MinOrder; order <= MaxOrder; order++)
				{
					for (i = 0; i < CurTheme->nLabelLines; i++)
					{
						pStreet = (LPSTREETHEADER)GlobalLock(phLabelLines[i]);
						pPoints = (LPPOINT)(pStreet + 1);
						if (pStreet->Order == order)
						{
							//hPen = CreatePen (PS_SOLID,pStreet->HollowStreetWidth,pStreet->FillColor);  
							lb.lbColor = pStreet->FillColor;
							hPen = ExtCreatePen(PS_GEOMETRIC | PS_SOLID | PS_ENDCAP_FLAT | PS_JOIN_BEVEL, pStreet->HollowStreetWidth, &lb, 0, 0);
							hOldPen = SelectObject(CurView->hDC, hPen);
							Polyline(CurView->hDC, pPoints, pStreet->NumPoints);
							SelectObject(CurView->hDC, hOldPen);
							GSSiDeleteObject(&hPen);
						}
						GlobalUnlock(phLabelLines[i]);
					}
				}
			}

			for (order = MinOrder;order <= MaxOrder;order++)
			{
				for (i=0;i<CurTheme->nLabelLines;i++)   
				{
 					pStreet = (LPSTREETHEADER)GlobalLock (phLabelLines[i]); 
					pPoints = (LPPOINT)(pStreet+1);  
					if (pStreet->Order == order)
					{
						POINT	SaveBP, SaveEP;
						double	AZ;

						hPen = CreatePen (PS_SOLID,pStreet->HollowStreetWidth,pStreet->FillColor);  
						hOldPen = SelectObject (CurView->hDC,hPen); 
						SaveBP = *pPoints;
						SaveEP = pPoints[pStreet->NumPoints-1];
						if (idist (*pPoints,*(pPoints+1)) < MaxWidth + 1)
							*pPoints = MidPoint (*pPoints,*(pPoints+1));
						else
						{
							AZ = getaz (*pPoints,*(pPoints+1));
							*pPoints = newpt (*pPoints,AZ,MaxWidth);
						}
						if (idist (pPoints[pStreet->NumPoints-1],pPoints[pStreet->NumPoints-2]) < MaxWidth + 1)
							pPoints[pStreet->NumPoints-1] = MidPoint (pPoints[pStreet->NumPoints-1],pPoints[pStreet->NumPoints-2]);
						else
						{
							AZ = getaz (pPoints[pStreet->NumPoints-1],pPoints[pStreet->NumPoints-2]);
							pPoints[pStreet->NumPoints-1] = newpt (pPoints[pStreet->NumPoints-1],AZ,MaxWidth);
						}
						if (ShowHollowStreet == 1)
							Polyline (CurView->hDC,pPoints,pStreet->NumPoints); 
						*pPoints = SaveBP;
						pPoints[pStreet->NumPoints-1] = SaveEP;
						SelectObject (CurView->hDC,hOldPen);
						GSSiDeleteObject (&hPen); 
						if (pStreet->OneWay)
						{
							int Width = max(0,min(6,IDNINT(pStreet->HollowStreetWidth-2))) * DeviceToScreenFactor;

							DrawOneWayArrows(CurView->hDC, pStreet->OneWay, pPoints, pStreet->NumPoints, Width);
						}

					}
					GlobalUnlock (phLabelLines[i]); 
				}
			}
			RestoreDC (CurView->hDC,-1);
			break;
		case 3:
			{
				int Width, Height;
				HDC	hDC;
				HBITMAP hBM, hBMOld;
				RECT	FullWindowRect;

				SaveDC (CurView->hDC); 
				InitRecord (CurView->hDC); 
				SetROP2(CurView->hDC,DisplayRasterOpt);
				SetDisplayMode (CurView->hDC, GF_TEXTMODE); 
				GSSiDeleteObject(&CurView->hRgn);
				CurView->hRgn = CreateVPRgn(FALSE,FALSE);
				SelectClipRgn (CurView->hDC,CurView->hRgn);
				GetClientRect (CurView->hWnd,&FullWindowRect);
				hDC = CreateCompatibleDC(CurView->hDC); 
				Height = RECTHEIGHT (&FullWindowRect);
				Width = RECTWIDTH (&FullWindowRect);
				hBM = CreateCompatibleBitmap(CurView->hDC,Width,Height); 
				hBMOld = SelectObject(hDC,hBM);
				SetDisplayMode (hDC, GF_TEXTMODE); 
				SelectClipRgn (hDC,0);
				FillRect (hDC,&FullWindowRect,GetStockObject (WHITE_BRUSH));

				for (i=0;i<CurTheme->nLabelLines;i++)   
				{
 					pStreet = (LPSTREETHEADER)GlobalLock (phLabelLines[i]); 
					pPoints = (LPPOINT)(pStreet+1);  
					MinOrder = min (MinOrder,pStreet->Order);
					MaxOrder = max (MaxOrder,pStreet->Order);
					EdgeWidth = pStreet->HollowStreetWidth/8 + 1;
					hPen = CreatePen (PS_SOLID,IDNINT(pStreet->HollowStreetWidth+EdgeWidth*2),0);  

					hOldPen = SelectObject (hDC,hPen); 
					Polyline (hDC,pPoints,pStreet->NumPoints); 
					SelectObject (hDC,hOldPen);
					GSSiDeleteObject (&hPen); 
					GlobalUnlock (phLabelLines[i]); 
				}
				for (order = MinOrder;order <= MaxOrder;order++)
				{
					for (i=0;i<CurTheme->nLabelLines;i++)   
					{
 						pStreet = (LPSTREETHEADER)GlobalLock (phLabelLines[i]); 
						pPoints = (LPPOINT)(pStreet+1);  
						if (pStreet->Order == order)
						{
							hPen = CreatePen (PS_SOLID,pStreet->HollowStreetWidth,RGB(255,255,255));  
							hOldPen = SelectObject (hDC,hPen); 
							Polyline (hDC,pPoints,pStreet->NumPoints); 
							SelectObject (hDC,hOldPen);
							GSSiDeleteObject (&hPen); 
						}
						GlobalUnlock (phLabelLines[i]); 
					}
				}
				StretchBlt(CurView->hDC, 0,0,Width,Height, hDC, 0, 0,Width,Height, SRCAND);
				SelectObject (hDC,hBMOld);
				GSSiDeleteObject (&hBM);
				DeleteDC(hDC);
				RestoreDC (CurView->hDC,-1);
			}
			break;
		}
		GlobalUnlock (CurTheme->hhLabelLines);
	}

	return rtn;
}

BOOL DisplayStreetLabels (BOOL Clear)
{ 
#define MAXTEXTPOINTS	128
	USHORT	i,j,k, ipos;  
	long	ptin;
	short	nChar, loc, StartPoint, twidth, theight, ichar, dy, ipass, ntxp, EndLine, BegLine;    
	BOOL	Flip;
//	LPSHORT	pNumPoints; 
//	LPLONG	pStreets;
//	LPSHORT	pHollowStreetWidth;
	LPSTREETHEADER	pStreet;
	LPPOINT	pPoints;   
	LOGFONT	LogFont;    
	HFONT	hFont, OldFont=0;    
	SIZE	txSize;
	double	AZ, Dist, SaveDist,NextDist,TotLength, CharWidth, StartDist, dtol, pinc; 
	POINT	Point,  TempPoint; 
	DPOINT	DPoint, DPoint1, DPoint2, FlipPoint[2]; 
	double	TextOffset, TextOffsetBegin;  
	double	MaxDeflection=GetGlobalDVal2 ("[%STREETTEXTMAXDEFLEXTION]",HALFPI/3), MaxD;
	double	CharacterSpacingFactor=GetGlobalDVal2 ("[%STREETTEXTSPACING]",1.05);
	double	MaxTextSize=GetGlobalLVal2 ("[%STREETTEXTMAXSIZE]",14)*DeviceToScreenFactor;
	double	MinTextSize=GetGlobalLVal2 ("[%STREETTEXTMINSIZE]",10)*DeviceToScreenFactor;
	double	StreetTextAdjustment=GetGlobalDVal2 ("[%STREETTEXTVERTICALADJUSTMENT]",0.5);
	long	NameInc=0, LastNameInc;//+1000000000      
	double	StartTextSize, TextSize;
	int		NumTries, MaxTriesB=GetGlobalLVal2 ("[%STREETNAMEMAXTRIES]",10), iTextSize;
	int		MaxTries = MaxTriesB;
    COLORREF	HollowTextColor=GetGlobalLVal2 ("[%HOLLOWTEXTCOLOR]",0);  
    BOOL	debug=FALSE;   
    BOOL	UseShortName = GetGlobalBVal2 ("[%USESHORTNAME]",TRUE);
    short	trinc = MaxTextSize;        
    short	symbol;   
    BOOL	DisplayedText,AvoidIntersections=GetGlobalBVal2 ("[%AVOIDINTERSECTIONS]",TRUE);    
    long	ii;
    double	MinDistBetweenNames = GetGlobalDVal2 ("[%MINDISTBETWEENNAMES]",1000)*DeviceToScreenFactor;
    double	MinDistBetweenShields = GetGlobalDVal2 ("[%MINDISTBETWEENSHIELDS]",100)*DeviceToScreenFactor;
	double	FlipAZ, AZ2, txtfac;
	double	MaxMoveDist, IncDist;
	BOOL	rtn=FALSE;


	if (Clear)
	{
		if (CurTheme->hhLabelLines)
		{
			LPHANDLE	phLabelLines = GlobalLock (CurTheme->hhLabelLines);

			for (i=0;i<CurTheme->nLabelLines;i++) 
				GSSiGlobFree(&phLabelLines[i]);
			CurTheme->nLabelLines = 0;
			GSSiGlobUlFree (&CurTheme->hhLabelLines);
		}
		rtn = TRUE;
		goto Exit;
	}

	if (!MinDistBetweenNames)
		MinDistBetweenNames =1000;
	NumShieldsDisplayed = 0;
	if (CurView && CurTheme && CurTheme->hhLabelLines)
	{
		LPHANDLE	phLabelLines = GlobalLock (CurTheme->hhLabelLines);
		LPSTREETTEXTDATA	pStreetData=(LPSTREETTEXTDATA)CurTheme->ClassBM;   

		if (UseHollowStreets && pStreetData->AllowHollow)
		{
			rtn = TRUE;
			
	debugaddress=&pStreetData->IgnoreShields;
			DisplayStreetCenterlines ();
			{
			HANDLE	hMem=GSSiGlobAlloc (0,GMEM_MOVEABLE,256+256+sizeof(double)*MAXTEXTPOINTS+sizeof(double)*MAXTEXTPOINTS+sizeof(DPOINT)*MAXTEXTPOINTS);   
			LPSTR	StreetsText=GlobalLock (hMem);
			LPSTR	StreetsText2 = StreetsText + 256; 
			LPDOUBLE	TxtAZ = (LPDOUBLE)(StreetsText2 + 256);   
			LPDOUBLE	IndCharWidth = TxtAZ + MAXTEXTPOINTS; 
			LPDPOINT	TxtPoints = (LPDPOINT)(IndCharWidth+MAXTEXTPOINTS);    
			long	ShadowC;
			RECT	TextDisplayRect = CurView->DrawRect;

			InflateRect (&TextDisplayRect,IDNINT(-MaxTextSize*2),IDNINT(-MaxTextSize*2));
			
			FlipAZ  = HALFPI +0.1;
			OpenShields (); 
			if (pStreetData->NameSource)
				pStreetData->hNameFile2 = BT_OPEN (pStreetData->NameFile2,0, BT_READ, 0);   
			SaveDC (CurView->hDC); 
			InitRecord (CurView->hDC); 
			SetROP2(CurView->hDC,DisplayRasterOpt);
			SetDisplayMode (CurView->hDC, GF_TEXTMODE); 
			GSSiDeleteObject(&CurView->hRgn);
			CurView->hRgn = CreateVPRgn(FALSE,FALSE);
			SelectClipRgn (CurView->hDC,CurView->hRgn);


			_fmemset (&LogFont,0,sizeof(LOGFONT));
			GetGlobalCVal ("[%STREETTEXTFONT]",LogFont.lfFaceName,"Arial");
		   // _fstrcpy (LogFont.lfFaceName,"Arial"); 
			if (pStreetData->UseFont)
			{
	    		LogFont = pStreetData->StreetTextFont;
	    		HollowTextColor = pStreetData->TextColor;
			} 
			else if ((ShadowC = GetGlobalLVal2 ("[%SHADOWCOLOR]",-1)) > -1)
			{
	    		pStreetData->Shadow = TRUE;
	    		pStreetData->ShadowColor = ShadowC;
			}
			LogFont.lfOutPrecision = OUT_TT_PRECIS;
			if (pStreetData->NonAntialiased)
				LogFont.lfQuality = NONANTIALIASED_QUALITY;
			else
				LogFont.lfQuality = PROOF_QUALITY;
			SelectObject (CurView->hDC,GetStockObject(BLACK_PEN));
			SetBkMode(CurView->hDC, TRANSPARENT);
     		SetTextColor(CurView->hDC,ConvertColor(HollowTextColor,CurTheme->UseHalfTone));
			ii=CurTheme->nLabelLines;
			for (i=0;i<CurTheme->nLabelLines;i++)   
			{
				float	ShieldSizeFactor=1,ShieldTextFactor=1;
            
				pStreet = (LPSTREETHEADER)GlobalLock (phLabelLines[i]); 
			//	pStreets   = (LPLONG)(pNumPoints+1);
			//	pHollowStreetWidth = (LPSHORT)(pStreets+4);
				pPoints = (LPPOINT)(pStreet+1);  
	//			Polyline (CurView->hDC,pPoints,*pNumPoints);   //pPoints[1]
				EndLine = pStreet->NumPoints-1;
	//			ltoa (IDNINT(*pStreets * DTMContourInterval),StreetsText,10); 
				TotLength = GetPolyLength (pPoints,pStreet->NumPoints);   
				if (TotLength > MinDistBetweenNames * 2 && CurTheme->nLabelLines < MAXLABELLINES)
				{
					DPOINT	p = PointAtDistOnPoly16 (pPoints,pStreet->NumPoints,MinDistBetweenNames,&AZ,&BegLine);
					//LPSHORT	pNumPoints2;
					//LPLONG	pStreets2; 
					//LPSHORT	pHollowStreetWidth2;
					LPSTREETHEADER	pStreet2;
					LPPOINT	pPoints2;  
					short	NumNewPoints = pStreet->NumPoints - BegLine;
					
					pStreet->NumPoints = BegLine + 2;
					phLabelLines[CurTheme->nLabelLines] = GSSiGlobAlloc (0,GMEM_MOVEABLE,sizeof(STREETHEADER) + sizeof(POINT) * NumNewPoints);		
					pStreet2 = (LPSTREETHEADER)GlobalLock (phLabelLines[CurTheme->nLabelLines]);
					*pStreet2 = *pStreet;
					pStreet2->NumPoints = NumNewPoints; 
				//	pStreets2 = (LPLONG)(Street2+1);  
					_fmemcpy (pStreet2->Streets,pStreet->Streets,16);   
				//	pHollowStreetWidth2 = (LPSHORT)(pStreets2+4);   
					pPoints2 = (LPPOINT)(pStreet2+1); 
					_fmemcpy (&pPoints2[1],&pPoints[BegLine+1],(NumNewPoints-1)*sizeof(POINT));
					pPoints[pStreet->NumPoints-1] = *pPoints2 = DPointToPoint (p);  //pPoints[5]   pPoints2[1]
					GlobalUnlock (phLabelLines[CurTheme->nLabelLines++]);   
				}  
				GlobalUnlock (phLabelLines[i]);
			} 
			ii=CurTheme->nLabelLines;
			for (i=0;i<CurTheme->nLabelLines;i++)   
			{
				float	ShieldSizeFactor=1,ShieldTextFactor=1;
            
				pStreet = (LPSTREETHEADER)GlobalLock (phLabelLines[i]); 
		//		pStreets   = (LPLONG)(pNumPoints+1);
		//		pHollowStreetWidth = (LPSHORT)(pStreets+4);
				pPoints = (LPPOINT)(pStreet+1);  
	//			Polyline (CurView->hDC,pPoints,*pNumPoints);   //pPoints[1]
				EndLine = pStreet->NumPoints-1;
	//			ltoa (IDNINT(*pStreets * DTMContourInterval),StreetsText,10); 
				TotLength = GetPolyLength (pPoints,pStreet->NumPoints); 
				NextDist = 0;
				ipos = 0; 
	NextSNum:
				NameInc = 0; 
				LastNameInc = -1;
				NumTries = 0;
				ipass = 0;
				
	NextName:   
				NumTries++;
				if (NumTries > MaxTries)
					goto NextLine;
				if (NameInc != LastNameInc)
				{
					GetStreetThemeName (labs(pStreet->Streets[ipos])+NameInc,StreetsText,(short)pStreet->Streets[3]);
					if ((symbol=ShieldType (StreetsText,&ShieldSizeFactor,&ShieldTextFactor)))
					{
						short start = GetSymbolTextStart (symbol);

						if (start && _fstrlen(&StreetsText[start])<6)
						{
							_fstrcpy (StreetsText2,StreetsText);     
							_fstrcpy (StreetsText,&StreetsText[start]);
							TextSize = StartTextSize = MaxTextSize;
						}
						else
							symbol = 0; 
						if (symbol && (!DisplayShields || pStreetData->IgnoreShields))  
						{
							symbol = 0;
							goto NextLine;
						}
						txtfac = 1;
					}
					else
						txtfac = StreetTextFactor;
				}
				LastNameInc = NameInc;
	   			TextSize = StartTextSize = min(MaxTextSize*txtfac,(pStreet->HollowStreetWidth)*txtfac-2); 
   				TextOffsetBegin = 0;
	   			if (TextSize <  MinTextSize)
	   			{
	   				TextSize = StartTextSize = MaxTextSize;
	   				if (ShowHollowStreet)
	   					TextOffsetBegin = pStreet->HollowStreetWidth + TextSize/2 + 1;    
	   				MaxD = MaxDeflection/2;
	   			} 
	   			else 
	   			{   
	//	   			TextSize = StartTextSize = (*pHollowStreetWidth)*StreetTextFactor-2;
	   				MaxD = MaxDeflection;
	//		     	SetTextColor(CurView->hDC, HollowTextColor);
	   			}
				if (TextSize < MinTextSize) 
				{   
					if (ipass)
					{
						if (NameInc || !UseShortName) 
							goto NextLine; 
						NameInc = 1000000000;     
						TextSize = StartTextSize;
						goto NextName;
					} 
					else
					{
						ipass++;
						TextSize = StartTextSize;
					}
				}
	   			LogFont.lfHeight = IDNINT(TextSize);   
	   			if (LogFont.lfHeight <= 0)
	   				goto NextName2;  
	   			LogFont.lfHeight = -LogFont.lfHeight;
				nChar = min(64,_fstrlen (StreetsText));
				if (!nChar)
					goto NextLine;
				ntxp = nChar+1; 
	   			LogFont.lfEscapement = LogFont.lfOrientation = 0;  
				hFont = CreateFontIndirect((LPLOGFONT)&LogFont); 
				OldFont = SelectObject(CurView->hDC,hFont); 
				GetTextExtentPoint32 (CurView->hDC,StreetsText,nChar,&txSize);
				twidth = txSize.cx; 
				theight = txSize.cy;   
				if (symbol)
					TextOffsetBegin = -theight;
				CharWidth = CharacterSpacingFactor*(double)twidth/nChar;
				MaxMoveDist = TotLength - twidth;
				IncDist = max (CharWidth,MaxMoveDist/MaxTries);
				if (ipass)
				{
					Dist = StartDist = 0; 
					NextDist = (double)twidth*CharacterSpacingFactor + (2*CharWidth);  
				}
				else if (ipos)
				{ 
					Dist = StartDist = NextDist;
					NextDist += twidth*CharacterSpacingFactor + (2*CharWidth);  
				}
				else
				{
					Dist = StartDist = TotLength/2 - (double)twidth*CharacterSpacingFactor/2 - (2*CharWidth)-IncDist; 
					NextDist = TotLength/2 + (double)twidth*CharacterSpacingFactor + (2*CharWidth);  
				}
				for (j=0;j<nChar;j++)
				{
					GetTextExtentPoint32 (CurView->hDC,&StreetsText[j],1,&txSize);
					IndCharWidth[j] = CharacterSpacingFactor*(double)txSize.cx; 
					if (debug)
						IndCharWidth[j] = CharWidth;
				}
				SelectObject(CurView->hDC,OldFont);
				GSSiDeleteObject(&hFont);
				if ((TotLength - (pStreet->HollowStreetWidth)*2) < twidth * CharacterSpacingFactor)
				{
	NextName2:
					TextSize -= 2;   
					goto NextName;
				}
				dtol = CharWidth * 0.1;
		TryAgain:   
				DisplayedText = FALSE; 
				TextOffset = TextOffsetBegin; 
				NumTries++;
				if (NumTries > MaxTries)
					goto NextLine;
				Dist += IncDist; 
				TxtPoints[0] = PointAtDistOnPoly16 (pPoints,pStreet->NumPoints,Dist,&AZ,&EndLine);
				TxtPoints[ntxp-1] = PointAtDistOnPoly16 (pPoints,pStreet->NumPoints,Dist+CharacterSpacingFactor*twidth,&AZ,&EndLine); 
				if (EndLine >= pStreet->NumPoints)
					goto NextName2;
				DPoint1 = WinPtToBasePtD (&TxtPoints[0]);  
				DPoint2 = WinPtToBasePtD (&TxtPoints[ntxp-1]);  
				Flip = FALSE;
				AZ = getazd (ProjectBasePt (&DPoint1),ProjectBasePt (&DPoint2));
				AZ2 = LTWOPI (AZ + CurView->Rotation);
				if (AZ2 > FlipAZ && AZ2 < 3*FlipAZ) 
					Flip = TRUE; 
				for (j=1;j<ntxp;j++)  
				{   
					double	dinc; 
					short	n=0,k=j-1;
					
					if (Flip)  
						k = ntxp-j-1;
					dinc = IndCharWidth[k] - dtol;
					Dist+=IndCharWidth[k];
					do                            //pPoints[*pNumPoints-1]
					{   
						Dist += IndCharWidth[k] - dinc + dtol;
						TxtPoints[j] = PointAtDistOnPoly16 (pPoints,pStreet->NumPoints,Dist,&AZ,&BegLine); 
						if (BegLine >= pStreet->NumPoints) 
						{
							EndLine = pStreet->NumPoints-1;
							goto NextName2; 
						}
						dinc = ldistp (TxtPoints[j-1],TxtPoints[j]) + dtol;  
						n++;
						if (n>100)
						{
							EndLine = pStreet->NumPoints-1;
							goto NextName2; 
						} 
					}while (dinc < IndCharWidth[k]); 
					{
						POINT pt = DPointToPoint (TxtPoints[j]);

						if (!PtInRect (&TextDisplayRect,pt))
							goto TryAgain;
						if (PtInTextRect(pt))
							goto TryAgain;
					}
					TxtAZ[j-1] = getazd (&TxtPoints[j-1],&TxtPoints[j]);
					if (j > 1)
						if (fabs(DeflectionAngle (TxtAZ[j-2],TxtAZ[j-1])) > MaxD)
							goto TryAgain;
				}
				if (symbol && AvoidIntersections)
				{ 
					//if only 1 node point on line in text it is probably an intersection - try again 
					ptin = -1;  
					iTextSize = IDNINT (TextSize/2);
					for (j=1;j<ntxp;j++)  
					{
						RECT	rect;
						
						RectInit (&rect);
						AddPointToRect (DPointToPoint (TxtPoints[j]) ,&rect);
						AddPointToRect (DPointToPoint (TxtPoints[j-1]) ,&rect);
						InflateRect (&rect,iTextSize,iTextSize);  
						if (ntxp < 4)
							ii=1;
						for (k=0;k<pStreet->NumPoints;k++) 
						{
							if (PtInRect (&rect,pPoints[k])) //pPoints[28]
							{
								if (ptin < 0)
									ptin = k;
								else if (ptin != k)
								{
									if (idist (pPoints[ptin],pPoints[k]) > TextSize)
									{
										ptin = -1;
										break;
									}
								} 
							}
						}
								
					}
					if (ptin >= 0)
						goto TryAgain; 
				}
				SaveDist = Dist; 
	//			for (ipass=0;ipass<2;ipass++)
				{
					Dist = StartDist;
					for (ichar = 0;ichar < nChar;ichar++)   //pPoints[21]
					{   
						short	jchar=ichar; 
						DPOINT	AZPt1, AZPt2;
						RECT	txtRect;
						
						RectInit (&txtRect);
						if (Flip)
							jchar = nChar - ichar -1;
						DPoint1 = WinPtToBasePtD (&TxtPoints[ichar]);  
						DPoint2 = WinPtToBasePtD (&TxtPoints[ichar+1]); 
						AddPointToRect (DPointToPoint (TxtPoints[ichar]),&txtRect);
						AddPointToRect (DPointToPoint (TxtPoints[ichar+1]),&txtRect);
	//					DPoint = MidPointD (DPoint1,DPoint2);
						AZPt1 = DPoint1;
						AZPt2 = DPoint2;
						AZ = getazd (ProjectBasePt (&AZPt1),ProjectBasePt (&AZPt2));   
						pinc = 0.25;
						if (Flip)
							pinc = 0.75;
						DPoint = dnewpt (DPoint1,AZ,pinc*ldistp(DPoint1,DPoint2));
	/*			Point = BasePtToWinPt (&DPoint);
				SetPixel (CurView->hDC,Point.x,Point.y,RGB(255,0,0));		 
				SetPixel (CurView->hDC,Point.x-1,Point.y,RGB(255,0,0));		 
				SetPixel (CurView->hDC,Point.x+1,Point.y,RGB(255,0,0));		 
				SetPixel (CurView->hDC,Point.x,Point.y-1,RGB(255,0,0));		 
				SetPixel (CurView->hDC,Point.x,Point.y+1,RGB(255,0,0));		 
				SetPixel (CurView->hDC,Point.x-1,Point.y-1,RGB(255,0,0));		 
				SetPixel (CurView->hDC,Point.x-1,Point.y+1,RGB(255,0,0));		 
				SetPixel (CurView->hDC,Point.x+1,Point.y-1,RGB(255,0,0));		 
				SetPixel (CurView->hDC,Point.x+1,Point.y+1,RGB(255,0,0));*/		 
						if (Flip) 
							AZ += PY;
			   			LogFont.lfEscapement = IDNINT(3600-((LTWOPI(-AZ)/RADDEG)*10));  
			   			if (!LogFont.lfEscapement)
			   				LogFont.lfEscapement = 1; 
	/*			   		if (!(LogFont.lfEscapement % 900))
			   			{
			   				LogFont.lfEscapement-=1; 
			   				TextOffset = TextOffsetBegin;
			   			}
			   			else */
			   				TextOffset = TextOffsetBegin * 1.5;  
	/*			   		if (abs (LogFont.lfEscapement - 900) < 450 ||
			   				abs (LogFont.lfEscapement - 2700) < 450)
		   					TextOffset = TextOffsetBegin * 1.7;*/  
		   				ProjectBasePt (&DPoint);
						DPoint = dnewpt (DPoint,AZ+HALFPI,(theight+TextOffset)*CurView->BaseUnitsPerPixel*CurView->LLNormFactor*StreetTextAdjustment);
						Point = BasePtToWinPt (UnProjectBasePt (&DPoint)); 
						AddPointToRect (Point,&txtRect);
						LogFont.lfOrientation = LogFont.lfEscapement;
						hFont = CreateFontIndirect((LPLOGFONT)&LogFont); 
						SelectObject(CurView->hDC,hFont);
	//				    if (ipass)
						{   
				    		if (symbol)
				    		{    
				    			 short MinSize = max (TextSize,pStreet->HollowStreetWidth * 1.25 * ShieldSizeFactor*ShieldFactor);
				    			 
				    			 if (!ShowHollowStreet)
				    		 		MinSize = 0;
								 SelectObject(CurView->hDC,OldFont);
								 GSSiDeleteObject(&hFont);
								 Point = DPointToPoint (TxtPoints[ntxp/2]);  
								 switch (HaveShieldWithinMinDist (Point,MinDistBetweenShields,StreetsText2,ShieldDir))
								 {
									 case 0:
										 if (DispText (CurView->hDC,FALSE,Point.x-10,Point.x+10, Point.y,IDNINT(theight*ShieldFactor),2,0,
							  					   TextSize * ShieldTextFactor*ShieldFactor,ShieldSizeFactor*ShieldFactor,ShieldTextFactor,2,0,CurView->Rotation,StreetsText2,symbol,TRUE,FALSE,0,
							  					   GetTextColor(CurView->hDC),0,0,0,0,0,CurTheme->UseHalfTone,ShieldDir,MinSize,0,0,0,0))  
										 {
							 				DisplayedText = TRUE; 
							 				AddShieldToDisplayedList (Point,StreetsText2,ShieldDir);
											goto NextLine;
										 }
									 case 1:
										break;
									 case 2:
										goto NextLine;
								 }

				    		}
				    		else if (!ShieldsOnly)
				    		{
								if (pStreetData->Shadow)
								{   
									short	i=1,n=max (1,DeviceToScreenFactor+0.5);
									COLORREF	OldColor = SetTextColor(CurView->hDC, ConvertColor(pStreetData->ShadowColor,CurTheme->UseHalfTone));

									while (i <= n)
									{
										TextOut (CurView->hDC,Point.x+i,Point.y+i,&StreetsText[jchar],1);
										TextOut (CurView->hDC,Point.x-i,Point.y+i,&StreetsText[jchar],1);
										TextOut (CurView->hDC,Point.x-i,Point.y-i,&StreetsText[jchar],1);
										TextOut (CurView->hDC,Point.x+i,Point.y-i,&StreetsText[jchar],1);
										i++;
									} 
									SetTextColor (CurView->hDC,OldColor);
								}
								TextOut (CurView->hDC,Point.x,Point.y,&StreetsText[jchar],1);    
								InflateRect (&txtRect,trinc,trinc);
								AddTextRect2(&txtRect,TRUE);    
								DisplayedText = TRUE;
							}
						}  
						SelectObject(CurView->hDC,OldFont);
						GSSiDeleteObject(&hFont);   
					}
					if (!DisplayedText)  
					{
						Dist = SaveDist;
						goto TryAgain;
					}
				}			
	//			GWPolyline2 (CurView->hDC,&pPoints[BegLine+1],*pNumPoints-BegLine-1,DarkContourSymbol); 
		NextLine: 
				ipos++; 
				if (ipos < 2 && pStreet->Streets[ipos])
					goto NextSNum;
	//			GWPolyline2 (CurView->hDC,pPoints,EndLine+1,DarkContourSymbol); 
				GlobalUnlock (phLabelLines[i]);   
	//			if (!CheckForContinue (TRUE))
	//				break;
			}
			RestoreDC (CurView->hDC,-1);
			if (pStreetData->NameSource)
				BT_CLOSEANDDELETE (&pStreetData->hNameFile2);   
			CloseStreetNameTable();
			CloseGSStreetNames();	
			GSSiGlobUlFree (&hMem);  
			}
		}	
		for (i=0;i<CurTheme->nLabelLines;i++) 
			GSSiGlobFree(&phLabelLines[i]);
		CurTheme->nLabelLines = 0;
		GSSiGlobUlFree (&CurTheme->hhLabelLines);
	}
Exit:
	AddTextRect(NULL);
	CloseShields();
	return rtn;
} 

void LinkLabelLines (short Line1,short Line2,short Type2, short Type1)
{
//	LPSHORT	pNumPoints1, pNumPoints2, pTempNumPoints;
	LPPOINT	pPoints1, pPoints2, pTempPoints;  
//	LPLONG	pStreets1, pStreets2, pTempStreets; 
	long	NewNumPoints;   
//	LPSHORT	pHollowStreetWidth1, pHollowStreetWidth2, pTempHollowStreetWidth;
	LPSTREETHEADER	pStreet1, pStreet2, pTempStreet;
	USHORT	i,j;  
	HANDLE	hTemp;
	LPHANDLE	phLabelLines = GlobalLock (CurTheme->hhLabelLines);
	
	pStreet1 = (LPSTREETHEADER)GlobalLock (phLabelLines[Line1]); 
//	pStreets1   = (LPLONG)(pNumPoints1+1);
//	pHollowStreetWidth1 = (LPSHORT)(pStreets1+4);
	pPoints1 = (LPPOINT)(pStreet1+1);
	pStreet2 = (LPSTREETHEADER)GlobalLock (phLabelLines[Line2]); 
//	pStreets2   = (LPLONG)(pNumPoints2+1);
//	pHollowStreetWidth2 = (LPSHORT)(pStreets2+4);
	pPoints2 = (LPPOINT)(pStreet2+1);
	NewNumPoints = pStreet1->NumPoints + pStreet2->NumPoints;
	if (NewNumPoints > MAXPOINTSINLABEL)
	{
		GlobalUnlock (phLabelLines[Line1]);
		GlobalUnlock (phLabelLines[Line2]);
		goto Exit;
	}
	if (Type1 == 1 && Type2 == 1)
	{
		hTemp = GSSiGlobAlloc (0,GMEM_MOVEABLE,sizeof(STREETHEADER) + NewNumPoints * sizeof(POINT));	
		pTempStreet = (LPSTREETHEADER)GlobalLock (hTemp);
		*pTempStreet = *pStreet1;
//		pTempStreets = (LPLONG) (pTempNumPoints+1);
	//	_fmemcpy (pTempStreet->Streets,pStreet1->Streets,16);	
	//	pTempHollowStreetWidth = (LPSHORT)(pTempStreets+4);
		pTempPoints = (LPPOINT)(pTempStreet+1);  
		for (i=0,j=pStreet1->NumPoints-1;i<pStreet1->NumPoints;i++,j--)
			pTempPoints[i] = pPoints1[j];  
		pTempStreet->NumPoints = pStreet1->NumPoints; 
//		pTempStreet->HollowStreetWidth = pStreet1->HollowStreetWidth;
		for (i=0;i<pStreet2->NumPoints;i++)
			pTempPoints[pTempStreet->NumPoints + i] = pPoints2[i];  
		pTempStreet->NumPoints += pStreet2->NumPoints;
		GlobalUnlock (hTemp); 
		GSSiGlobUlFree (&phLabelLines[Line1]); 
		phLabelLines[Line1] = hTemp;
		GSSiGlobUlFree (&phLabelLines[Line2]);
		for (i=Line2;i<CurTheme->nLabelLines-1;i++)
			phLabelLines[i] = phLabelLines[i+1];
	}
	else if (Type1 == 1 && Type2 == 2)
	{
		GlobalUnlock (phLabelLines[Line2]);
    	phLabelLines[Line2] = GSSiGlobalReAlloc (0,phLabelLines[Line2] ,sizeof(STREETHEADER) + sizeof(POINT)*NewNumPoints,GMEM_MOVEABLE);
		pStreet2 = (LPSTREETHEADER)GlobalLock (phLabelLines[Line2]); 
//		pStreets2   = (LPLONG)(pNumPoints2+1);
//		pHollowStreetWidth2 = (LPSHORT)(pStreets2+4);
		pPoints2 = (LPPOINT)(pStreet2+1);
		for (i=0;i<pStreet1->NumPoints;i++)
			pPoints2[pStreet2->NumPoints + i] = pPoints1[i];   //pPoints2[21]
		pStreet2->NumPoints += pStreet1->NumPoints;
		GlobalUnlock (phLabelLines[Line2]); 
		GSSiGlobUlFree (&phLabelLines[Line1]);
		for (i=Line1;i<CurTheme->nLabelLines-1;i++)
			phLabelLines[i] = phLabelLines[i+1];
	}
	else if (Type1 == 2 && Type2 == 1)
	{
		GlobalUnlock (phLabelLines[Line1]);
    	phLabelLines[Line1] = GSSiGlobalReAlloc (0,phLabelLines[Line1] ,sizeof(STREETHEADER) + sizeof(POINT)*NewNumPoints,GMEM_MOVEABLE);
		pStreet1 = (LPSTREETHEADER)GlobalLock (phLabelLines[Line1]); 
	//	pStreets1   = (LPLONG)(pNumPoints1+1);
	//	pHollowStreetWidth1 = (LPSHORT)(pStreets1+4);
		pPoints1 = (LPPOINT)(pStreet1+1);
		for (i=0;i<pStreet2->NumPoints;i++)
			pPoints1[pStreet1->NumPoints + i] = pPoints2[i];  
		pStreet1->NumPoints += pStreet2->NumPoints;
		GlobalUnlock (phLabelLines[Line1]); 
		GSSiGlobUlFree (&phLabelLines[Line2]);
		for (i=Line2;i<CurTheme->nLabelLines-1;i++)
			phLabelLines[i] = phLabelLines[i+1];
	}
	else if (Type1 == 2 && Type2 == 2)
	{   
//		Polyline (CurView->hDC,pPoints1,*pNumPoints1);
//		Polyline (CurView->hDC,pPoints2,*pNumPoints2);
		GlobalUnlock (phLabelLines[Line1]);
    	phLabelLines[Line1] = GSSiGlobalReAlloc (0,phLabelLines[Line1] ,sizeof(STREETHEADER) + sizeof(POINT)*NewNumPoints,GMEM_MOVEABLE);
		pStreet1 = (LPSTREETHEADER)GlobalLock (phLabelLines[Line1]); 
//		pStreets1   = (LPLONG)(pNumPoints1+1);
//		pHollowStreetWidth1 = (LPSHORT)(pStreets1+4);
		pPoints1 = (LPPOINT)(pStreet1+1);
		for (i=0,j=pStreet2->NumPoints-1;i<pStreet2->NumPoints;i++,j--)
			pPoints1[pStreet1->NumPoints + i] = pPoints2[j];      //pPoints2[1]  pPoints1[*pNumPoints1-2]  pPoints1[4]   pPoints2[*pNumPoints2-1]
		pStreet1->NumPoints += pStreet2->NumPoints;
//		Polyline (CurView->hDC,pPoints1,*pNumPoints1);
		GlobalUnlock (phLabelLines[Line1]); 
		GSSiGlobUlFree (&phLabelLines[Line2]);
		for (i=Line2;i<CurTheme->nLabelLines-1;i++)
			phLabelLines[i] = phLabelLines[i+1];
	}
	CurTheme->nLabelLines--;
Exit:
	GlobalUnlock (CurTheme->hhLabelLines);

	return;
}

void LinkSymbolLines (short Line1,short Line2,short Type2, short Type1)
{
	LPSHORT	pNumPoints1, pNumPoints2, pTempNumPoints;
	LPTHEME	*pIsCenterline1, *pIsCenterline2, *pTempIsCenterline;
	LPFPOINT	pPoints1, pPoints2, pTempPoints;  
	LPLONG	pStreets1, pStreets2, pTempStreets;    
	LPSHORT	pSymNum1, pSymNum2, pTempSymNum;
	USHORT	i,j;  
	HANDLE	hTemp;
	
	pNumPoints1 = (LPSHORT)GlobalLock (hSymbolLines[Line1]); 
	pSymNum1   = (LPSHORT)(pNumPoints1+1);
	pIsCenterline1 = (LPTHEME*)(pSymNum1+1);  
	pPoints1 = (HPFPOINT)(pIsCenterline1+1);
	pNumPoints2 = (LPSHORT)GlobalLock (hSymbolLines[Line2]); 
	pSymNum2   = (LPSHORT)(pNumPoints2+1);
	pIsCenterline2 = (LPTHEME*)(pSymNum2+1);  
	pPoints2 = (HPFPOINT)(pIsCenterline2+1);
	if (*pNumPoints1 + *pNumPoints2 > MAXPOINTSINLABEL)
	{
		GlobalUnlock (hSymbolLines[Line1]);
		GlobalUnlock (hSymbolLines[Line2]);
		return;
	}
	if (Type1 == 1 && Type2 == 1)
	{
		hTemp = GSSiGlobAlloc (0,GMEM_MOVEABLE,4096*4);	
		pTempNumPoints = (LPSHORT)GlobalLock (hTemp);
		pTempSymNum = (LPSHORT) (pTempNumPoints+1);  
		pTempIsCenterline = (LPTHEME*)(pTempSymNum + 1);
		pTempPoints = (LPFPOINT)(pTempIsCenterline+1);  
		for (i=0,j=*pNumPoints1-1;i<*pNumPoints1;i++,j--)
			pTempPoints[i] = pPoints1[j];  
		*pTempNumPoints = *pNumPoints1; 
		*pTempSymNum = *pSymNum1; 
		*pTempIsCenterline = *pIsCenterline1;
		for (i=0;i<*pNumPoints2;i++)
			pTempPoints[*pTempNumPoints + i] = pPoints2[i];  
		*pTempNumPoints += *pNumPoints2;
		GlobalUnlock (hTemp); 
		GSSiGlobUlFree (&hSymbolLines[Line1]); 
		hSymbolLines[Line1] = hTemp;
		GSSiGlobUlFree (&hSymbolLines[Line2]);
		for (i=Line2;i<nSymbolLines-1;i++)
		{
			hSymbolLines[i] = hSymbolLines[i+1];
			SymbolLineColor[i] = SymbolLineColor[i+1];
		}

	}
	else if (Type1 == 1 && Type2 == 2)
	{
		for (i=0;i<*pNumPoints1;i++)
			pPoints2[*pNumPoints2 + i] = pPoints1[i];   //pPoints2[21]
		*pNumPoints2 += *pNumPoints1;
		*pIsCenterline2 = *pIsCenterline1;
		GlobalUnlock (hSymbolLines[Line2]); 
		GSSiGlobUlFree (&hSymbolLines[Line1]);
		for (i=Line1;i<nSymbolLines-1;i++)
		{
			hSymbolLines[i] = hSymbolLines[i+1];
			SymbolLineColor[i] = SymbolLineColor[i+1];
		}
	}
	else if (Type1 == 2 && Type2 == 1)
	{
		for (i=0;i<*pNumPoints2;i++)
			pPoints1[*pNumPoints1 + i] = pPoints2[i];  
		*pNumPoints1 += *pNumPoints2;
		*pIsCenterline1 = *pIsCenterline2;
		GlobalUnlock (hSymbolLines[Line1]); 
		GSSiGlobUlFree (&hSymbolLines[Line2]);
		for (i=Line2;i<nSymbolLines-1;i++)
		{
			hSymbolLines[i] = hSymbolLines[i+1];
			SymbolLineColor[i] = SymbolLineColor[i+1];
		}
	}
	else if (Type1 == 2 && Type2 == 2)
	{   
//		Polyline (CurView->hDC,pPoints1,*pNumPoints1);
//		Polyline (CurView->hDC,pPoints2,*pNumPoints2);
		for (i=0,j=*pNumPoints2-1;i<*pNumPoints2;i++,j--)
			pPoints1[*pNumPoints1 + i] = pPoints2[j];      //pPoints2[1]  pPoints1[*pNumPoints1-2]  pPoints1[4]   pPoints2[*pNumPoints2-1]
		*pNumPoints1 += *pNumPoints2;
		*pIsCenterline1 = *pIsCenterline2;
//		Polyline (CurView->hDC,pPoints1,*pNumPoints1);
		GlobalUnlock (hSymbolLines[Line1]); 
		GSSiGlobUlFree (&hSymbolLines[Line2]);
		for (i=Line2;i<nSymbolLines-1;i++)
		{
			hSymbolLines[i] = hSymbolLines[i+1];
			SymbolLineColor[i] = SymbolLineColor[i+1];
		}
	}
	nSymbolLines--;
	return;
}

