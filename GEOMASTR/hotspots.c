#include "graphint.h"

static HOTSPOTDATA	HSData;
static LPHOTSPOTDATA	pHSData=&HSData;
 
#include "gmextern.h"
static BOOL haveSecondsInSample = FALSE;
static int  secondsInSample;

void ClearSecondsInSample(void)
{
	haveSecondsInSample = FALSE;
}

long GetSecondsInSample (void)
{
	long	rtn=TimeRangeEnd-TimeRangeBeg;
	time_t  systime; 
	time_t	time=TimeRangeBeg+1;
	USHORT	i,j; 
	double	TODFactor=1,TotHours=0,TotDays=0,NumDaysIncluded=0;
	struct	tm	tmtime;	

	if (haveSecondsInSample)
		return secondsInSample;
	time = TimeRangeBeg + 1;
	while (time < TimeRangeEnd)
	{
		int midDay = time + 43200;
		if (WantYear(midDay))
		{
			if (WantMonth(midDay))
			{
				if (WantDOW(midDay))
				{
					TotDays++;
					if (OtherDayFilters(midDay))
						NumDaysIncluded++;
				}
			}
		}

		time += 86400;
	}

	for (i=0;i<*pNumViewports;i++)
	{
		if (!_fstrnicmp (pViewports[i]->Name,"Time of Day",11))
		{
			if (pViewports[i]->pTheme && pViewports[i]->pTheme->IsActive && pViewports[i]->pTheme->VPDisplayed)
			{ 
				for (j=0;j<pViewports[i]->pTheme->NumClass;j++)
					if (!pViewports[i]->pTheme->ClassStatus[j])
						TotHours++; 
				if (pViewports[i]->pTheme->NumClass)
					TODFactor = TotHours/pViewports[i]->pTheme->NumClass;
			}
		}
	}
	secondsInSample = TODFactor*NumDaysIncluded * 86400;
	haveSecondsInSample = TRUE;
	return secondsInSample;
}
BOOL WantYear (int iMidDay)
{
	BOOL rtn = TRUE;
	char str[256], midDayC[32];
	double rval;

	for (int i = 0; i<*pNumViewports; i++)
	{
		if (pViewports[i]->pTheme && pViewports[i]->pTheme->IsActive && pViewports[i]->pTheme->VPDisplayed && !_fstrnicmp(pViewports[i]->Name, "Year", 4))
		{
			sprintf(midDayC, "%i", iMidDay);
			strcpy(str, pViewports[i]->pTheme->Value);
			REPLAC(str, "[MINTIME]", midDayC, 255);
			ExpandText(str);
			rval = atof(str);
			for (int j = 0; j < pViewports[i]->pTheme->NumClass; j++)
				if (pViewports[i]->pTheme->ClassStatus[j] && rval >= pViewports[i]->pTheme->ClassMin[j] && rval <= pViewports[i]->pTheme->ClassMax[j])
					return FALSE;
		}
	}
	return rtn;
}
BOOL WantMonth(int iMidDay)
{
	BOOL rtn = TRUE;
	char str[256], midDayC[32];
	double rval;

	for (int i = 0; i<*pNumViewports; i++)
	{
		if (pViewports[i]->pTheme && pViewports[i]->pTheme->IsActive && pViewports[i]->pTheme->VPDisplayed && !_fstrnicmp(pViewports[i]->Name, "Month", 11))
		{
			sprintf(midDayC, "%i", iMidDay);
			strcpy(str, pViewports[i]->pTheme->Value);
			REPLAC(str, "[MINTIME]", midDayC, 255);
			ExpandText(str);
			rval = atof(str);
			for (int j = 0; j < pViewports[i]->pTheme->NumClass; j++)
				if (pViewports[i]->pTheme->ClassStatus[j] && rval >= pViewports[i]->pTheme->ClassMin[j] && rval <= pViewports[i]->pTheme->ClassMax[j])
					return FALSE;
		}
	}
	return rtn;
}
BOOL WantDOW(int iMidDay)
{
	BOOL rtn = TRUE;
	char str[256], midDayC[32];
	double rval;

	for (int i = 0; i<*pNumViewports; i++)
	{
		if (pViewports[i]->pTheme && pViewports[i]->pTheme->IsActive && pViewports[i]->pTheme->VPDisplayed && !_fstrnicmp(pViewports[i]->Name, "Day of Week", 11))
		{
			sprintf(midDayC, "%i", iMidDay);
			strcpy(str, pViewports[i]->pTheme->Value);
			REPLAC(str, "[MINTIME]", midDayC, 255);
			ExpandText(str);
			rval = atof(str);
			for (int j = 0; j < pViewports[i]->pTheme->NumClass; j++)
				if (pViewports[i]->pTheme->ClassStatus[j] && rval >= pViewports[i]->pTheme->ClassMin[j] && rval <= pViewports[i]->pTheme->ClassMax[j])
					return FALSE;
		}
	}
	return rtn;
}
BOOL OtherDayFilters(int iMidDay)
{
	BOOL rtn = TRUE;
	char str[256], midDayC[32];
	double rval;

	for (int i = 0; i<*pNumViewports; i++)
	{
		if (pViewports[i]->pTheme && pViewports[i]->pTheme->IsActive && pViewports[i]->pTheme->VPDisplayed && pViewports[i]->pTheme->isDayFilter)
		{
			sprintf(midDayC, "%i", iMidDay);
			strcpy(str, pViewports[i]->pTheme->Value);
			REPLAC(str, "[MINTIME]", midDayC, 255);
			ExpandText(str);
			rval = atof(str);
			for (int j = 0; j < pViewports[i]->pTheme->NumClass; j++)
				if (pViewports[i]->pTheme->ClassStatus[j] && rval >= pViewports[i]->pTheme->ClassMin[j] && rval <= pViewports[i]->pTheme->ClassMax[j])
					return FALSE;
		}
	}
	return rtn;
}
double HotSpotMean(HANDLE hGrid, ULONG lgrid)
{
	double mean=0, n=lgrid;
	HPLONG	pGrid=(HPLONG)GlobalLock (hGrid);
	 
	if (!n)
		return 0;	
	while (lgrid--)
	{
		mean += (double)*pGrid; 
		pGrid++;
	}
	GlobalUnlock (hGrid);
	return mean/n;
} 

double HotSpotSDV (double mean,HANDLE hGrid,ULONG lgrid)
{
	double n=lgrid, stdev;
	HPLONG	pGrid=(HPLONG)GlobalLock (hGrid);
	double sumsq=0;
	
	if (n<2)
		return 0;
	while (lgrid--)
	{
		sumsq += pow((double)*pGrid - mean,2); 
		pGrid++;
	}
	GlobalUnlock (hGrid);
	stdev = sqrt (sumsq/(n-1));
	return stdev; 
}   

void DisplayCurrentHotspots (void)
{ 
	UINT	i;
	LPTHEME	SaveTheme = CurTheme;
	  
	for (i=0;i<CurView->NumThemes;i++)
	{   
		if (CurView->pThemes[i]->ID == GF_HOTSPOT_THEME && CurView->pThemes[i]->IsActive && CurView->pThemes[i]->VPDisplayed)
		{
			CurTheme = CurView->pThemes[i];
	 		DisplayHotSpotThemeLegend(2);
	 	}
	 }
	 CurTheme = SaveTheme;
	 return;
}

void DisplayHotSpotThemeLegend(short From)
#if ENABLETRACE
{GSSiEnterProg (1287);
#endif
{    
	char	txt[64], str[256], fmt[16]=" %.0f mile "; 
	HPEN	OldPen, LinePen;
	LPHOTSPOTDATA	pHSData = &CurTheme->HotSpotData;  
	HPSHORT	pMask; 
	HANDLE	hPoints;
	HPLONG	pGrid;  
	long	GridSize;  
	short	x,y, i,j, w=CurView->DrawRect.right - CurView->DrawRect.left;    
	LPPOINT	Points;
	double	factor, hfactor,Mean, Stdev;  
	BOOL	RegionIsNull = FALSE;   
	HCURSOR	hcurSave; 
    
    if (From == 2)
    {
    	LPVIEWPORT	SaveVP = CurView;
    	
		if (CurTheme->hHotSpotBitmap)
		{
			SetViewport (CurTheme->TargetViewport);   
	    	if (CurView->DisplayInParent && CurView->Parent > 0) 
	    		CurView = pViewports[CurView->Parent-1];
			if (CurView->PassID)
			{
				if (CurTheme->HotSpotBounds.xmn == CurView->WBounds.xmn &&
					CurTheme->HotSpotBounds.xmx == CurView->WBounds.xmx &&
					CurTheme->HotSpotBounds.ymn == CurView->WBounds.ymn &&
					CurTheme->HotSpotBounds.ymx == CurView->WBounds.ymx)
				{
					SaveDC(CurView->hDC);
					SetDisplayMode(CurView->hDC, GF_TEXTMODE);
					GSSiDeleteObject(&CurView->hRgn);
					CurView->hRgn = CreateVPRgn(FALSE, FALSE);
					SelectClipRgn(CurView->hDC, CurView->hRgn);
					SelectClipRgn(CurView->hDC, CurView->hRgn);
					GSSiDeleteObject(&CurView->hRgn);
					//SelectClipRgn (CurView->hDC,0);  
					RestoreScreen(CurView->hDC, CurTheme->hHotSpotBitmap, CurView->DrawRect);
					RestoreDC(CurView->hDC, -1);
				}
				else
					GSSiDeleteObject(&CurTheme->hHotSpotBitmap);
			}
			else
				GSSiDeleteObject(&CurTheme->hHotSpotBitmap);

	    	CurView = SaveVP;
        }
    	return;
    }	
    if (From != 3 || w < 1)
    	return;   
	SaveDC (CurView->hDC);
	SetDisplayMode (CurView->hDC, GF_TEXTMODE);
    GSSiDeleteObject(&CurView->hRgn);
    CurView->hRgn = CreateVPRgn(FALSE,FALSE);
    SelectClipRgn (CurView->hDC,CurView->hRgn);
    if (SelectClipRgn (CurView->hDC,CurView->hRgn) == NULLREGION && !PrintingToMF)
     	RegionIsNull = TRUE;
    GSSiDeleteObject(&CurView->hRgn);
    if (RegionIsNull)
    	goto Exit;
	hcurSave = GSSiSetCursor(LoadCursor(NULL, IDC_WAIT)); 
    FillRectPoly (CurView->hDC,&CurView->DrawRect,WindowColor); 
    x = CurView->DrawRect.left;
    y = CurView->DrawRect.bottom - 30; 
	GridSize =  (long)pHSData->GridWidth * (long)pHSData->GridHeight;  
	Mean = HotSpotMean (pHSData->hGrid,GridSize);  
	Stdev = HotSpotSDV (Mean,pHSData->hGrid,GridSize); 
    sprintf (str,"%ld to %ld (Mean %ld, Stdev %ld",(long)CurTheme->Ymin,(long)CurTheme->Ymax,(long)Mean,(long)Stdev);
	TextOut(CurView->hDC, CurView->DrawRect.left +10, CurView->DrawRect.bottom - 25, str,_fstrlen(str));  
    if (pHSData->hMask)
    {   
    	pMask = (HPSHORT)GlobalLock (pHSData->hMask);
    	pMask += (long)pHSData->MaskWidth/2 * (long)pHSData->MaskWidth;  
    	hPoints = GSSiGlobAlloc(GAIDNO 0,GMEM_MOVEABLE,sizeof(POINT)*w);
    	Points = (LPPOINT)GlobalLock (hPoints);
    	j=0;  
    	factor =  (double)pHSData->MaskWidth/w; 
    	hfactor = ((double)(CurView->DrawRect.bottom - CurView->DrawRect.top - 40))/1000;
    	for (i=0;i<w;i++) 
    	{
    		Points[i].x = x++;                                     
    		Points[i].y = y-((pMask[(int)(i*factor)])*hfactor);  
    	}
		LinePen = CreatePen (PS_SOLID,(int)IDNINT(DeviceToScreenFactor()),0);
		OldPen = SelectObject (CurView->hDC,LinePen);
	    Polyline (CurView->hDC,Points,w);  
		SelectObject (CurView->hDC,OldPen);
		DeleteObject (LinePen); 
		GlobalUnlock (pHSData->hMask);
		GSSiGlobUlFree (&hPoints);
	}
	GSSiSetCursor(hcurSave);
Exit:
	RestoreDC (CurView->hDC,-1);
	
{
#if ENABLETRACE
GSSiExitProg (1287);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}        

BOOL SetHotSpotMaskWidth(LPHOTSPOTDATA	pHSData)
{
	static double lastRadius = -1;
	DPOINT	dp1, dp2, HotSpotPoint[2];
	char str[256];

	dp1 = dp2 = MinMaxMidPointD(&CurView->NewBounds);
	_fstrcpy(str, CurTheme->ClassDefValSQL);
	ExpandText(str);
	ExpandText(str);
	pHSData->Radius = atof(str);
	if (pHSData->Radius < 0)
	{
		lastRadius = pHSData->Radius;
		GSSiGlobFree(&pHSData->hMask);
		return FALSE;
	}
	if (!pHSData->Radius)
		pHSData->Radius = 500;
	if (pHSData->Radius == lastRadius)
		return TRUE;
	GSSiGlobFree(&pHSData->hMask);
	lastRadius = pHSData->Radius;
	dp2.x += pHSData->Radius;
	HotSpotPoint[0] = TranPoint(&dp1, pHSData->hTranBaseToHotSpot);
	HotSpotPoint[1] = TranPoint(&dp2, pHSData->hTranBaseToHotSpot);
	pHSData->MaskWidth = max(1, ldistp(HotSpotPoint[0], HotSpotPoint[1]));
	pHSData->hMask = GSSiGlobAlloc(GAIDNO 1024, GMEM_MOVEABLE, (long)pHSData->MaskWidth * (long)pHSData->MaskWidth * 4);
	SetupHotSpotMask(pHSData->MaskWidth, pHSData->hMask, pHSData->DecayOpt);

	return TRUE;
}

void SetupHotSpotMask (short MaskWidth,HANDLE hMask,short DecayOpt)
#if ENABLETRACE
{GSSiEnterProg (1318);
#endif
{   
	HPSHORT pMask=(HPSHORT)GlobalLock (hMask);
	double	d; 
	double	HalfMaskWidth= (double)MaskWidth/2, MaskWD2 = HalfMaskWidth;
	DPOINT	MidPoint={HalfMaskWidth,HalfMaskWidth}, p;
    
    if (HalfMaskWidth < 1)
    	goto Exit;
	switch (DecayOpt)
	{                     
		case 1:
			HalfMaskWidth =  sqrt (HalfMaskWidth);
			break;  
		case 2:
			HalfMaskWidth =  HalfMaskWidth * HalfMaskWidth;  
			break;
	}
	p.y = 0;
	while (p.y < MaskWidth)
	{
		p.x = 0;
		while (p.x < MaskWidth)
		{   
			d = ldistp (p,MidPoint);
			if (d < MaskWD2)
			{
				switch (DecayOpt)
				{                     
					case 1:
						d = sqrt(d);
						break; 
					case 2:
						d = d * d;
						break; 
					case 3:
						d = 0;
						break;
				}
				*pMask = max (1,IDNINT(((HalfMaskWidth - d) / HalfMaskWidth) * 1000)); 
			}
			else
				*pMask = 0;
			pMask++;  
			p.x++;
		}
		p.y++;
	}   
Exit:
	GlobalUnlock (hMask);
{
#if ENABLETRACE
GSSiExitProg (1318);
#endif
	return;
}
#if ENABLETRACE
}
#endif
} 

long GetHotSpotValue (DPOINT Point)
{
	LPHOTSPOTDATA	pHSData;
	DPOINT	HotSpotPoint;
	USHORT	i; 
	HPLONG	pGrid;
	long	rtn=LONG_MAX,loc;
	long	x,y, MaskBegRow, MaskEndRow, MaskBegCol, MaskEndCol,GridBegRow, GridBegCol, GridEndRow, GridEndCol, row, col;
    LPVIEWPORT	SaveVP=CurView;
    
	for (i=0;i<CurView->NumThemes;i++)
	{ 
		if (CurView->pThemes[i]->ID==GF_HOTSPOT_THEME)
			goto FoundTheme;
	} 
	for (i=0;i<*pNumViewports;i++)
	{   
		if (pViewports[i]->Parent == CurView->ID && pViewports[i]->DisplayInParent)
		{
			CurView = pViewports[i];
			for (i=0;i<CurView->NumThemes;i++)
			{ 
				if (CurView->pThemes[i]->ID==GF_HOTSPOT_THEME)
					goto FoundTheme;
			}
		}
	}
	CurView = SaveVP; 
	return rtn;

FoundTheme:	
    pHSData = &CurView->pThemes[i]->HotSpotData;
	HotSpotPoint = TranPoint (&Point,pHSData->hTranBaseToHotSpot);
	x = IDNINT (HotSpotPoint.x);
	y = IDNINT (HotSpotPoint.y);
	GridBegRow = y - pHSData->MaskWidth/2; 
	if (y < 0 || y > pHSData->GridHeight-1 || x < 0 || x > pHSData->GridWidth-1)
		goto Exit;
	loc = y * pHSData->GridWidth + x;
	pGrid = (HPLONG)GlobalLock (pHSData->hGrid);
	rtn = pGrid[loc];
	GlobalUnlock (pHSData->hGrid);  
Exit:
	CurView = SaveVP; 
	return rtn;
}

void AddItemToHotSpot (int Type)
#if ENABLETRACE
{GSSiEnterProg (1320);
#endif
{   
	LPHOTSPOTDATA	pHSData = &CurTheme->HotSpotData;
	DPOINT	HotSpotPoint, Point;
	long	x,y, MaskBegRow, MaskEndRow, MaskBegCol, MaskEndCol,GridBegRow, GridBegCol, GridEndRow, GridEndCol, row, col;
	HPLONG	pGrid, pGridRow;
	HPSHORT	pMask, pMaskRow;  
	long	MaxGrid=LONG_MIN, MinGrid=LONG_MAX,EndCol,ninteration; 
	double	Weight;
	char	CWeight[128]; 
	BOOL	HaveWeight=FALSE, rtn; 
	HANDLE	hPoints;
	HPDPOINT	pArea; 
	MNMXCORD	Bounds;    
	double	NextXIntersect; 
	DWORD	i,ii;     
	static	long	dbref=3562;
	
	DBoundsInit (&Bounds);
	if (Type == GF_AREA)
	{   
		if (CurrentRefno == dbref)
			ii=1;
		_fstrcpy (CWeight,CurTheme->IconLibrary);
		ExpandText (CWeight);
		if (*CWeight)
		{  
			Weight = atof (CWeight);
			HaveWeight = TRUE;
		}	
		hPoints = GSSiGlobAlloc(GAIDNO 0,GMEM_MOVEABLE,(long)nPnts * (long)sizeof(DPOINT));
		pArea = (HPDPOINT)GlobalLock (hPoints); 
		for (i=0;i<nPnts;i++)
		{
			pArea[i] = TranPoint (&lpDCurPoints[i],pHSData->hTranBaseToHotSpot);
			AddDPointToMinMax (&pArea[i],&Bounds);
        } 
        Bounds.xmn = max (0,Bounds.xmn);
        Bounds.xmx = min (pHSData->GridWidth-1,Bounds.xmx);
        Bounds.ymn = max (0,Bounds.ymn);
        Bounds.ymx = min (pHSData->GridHeight-1,Bounds.ymx); 
        MaskBegRow = Bounds.ymn;
        MaskBegCol = Bounds.xmn;  
        MaskEndRow = min ((double)pHSData->GridHeight-1,Bounds.ymx+1);
        MaskEndCol = min ((double)pHSData->GridWidth-2,Bounds.xmx+1);
		pGridRow = (HPLONG)GlobalLock (pHSData->hGrid);
		pGridRow += (long)pHSData->GridWidth * (long)MaskBegRow; 
		row = MaskBegRow; 
		while (row <= MaskEndRow)
		{
			col = MaskBegCol;
			pGrid = pGridRow; 
			NextXIntersect = -P_TOL; 
			ninteration=0;
			while (col < MaskEndCol)
			{   
				ninteration++;
				if (ninteration > 100)
					ii=1;
				Point.x = max ((double)col,NextXIntersect+P_TOL*2);
				Point.y = row;
				rtn = POINT_IN_AREAD (Point, nPnts, pArea,1,0,&NextXIntersect,NULL);  
				if (rtn && NextXIntersect > 100000)
					ii=1;
				if (NextXIntersect < (double)MaskEndCol+1)
					EndCol = min ((double)pHSData->GridWidth-2,NextXIntersect+1);
				else 
				{
					ii=1;
					EndCol = pHSData->GridWidth-2;
				}
				if (rtn)
				{   
					while (col <= EndCol)
					{
						if (HaveWeight)
							pGrid[col] = Weight; 
						else 
							pGrid[col]++; 
						MaxGrid = max (MaxGrid,pGrid[col]); 
						MinGrid = min (MinGrid,pGrid[col]); 
						col++;
					} 
					col--;
				} 
				else
					col = EndCol;
//				col++;
			} 
			row++;
			pGridRow += pHSData->GridWidth;
		}
		GlobalUnlock (pHSData->hGrid);
		pHSData->MaxGridValue = CurTheme->Ymax = max (CurTheme->Ymax,(double)MaxGrid);  
		CurTheme->Ymin = min (CurTheme->Ymin,(double)MinGrid);
/*		pArea = (HPDPOINT)GlobalLock (hPoly);
		rtn = POINT_IN_AREAD (Point, npnts, pArea,&NextXIntersect);
		GlobalUnlock (hPoly);*/ 
		GSSiGlobUlFree (&hPoints);
{
#if ENABLETRACE
GSSiExitProg (1320);
#endif
		return;  
}  
	}
	if (Type != GF_POINT)
{
#if ENABLETRACE
GSSiExitProg (1320);
#endif
		return;  
}  
	_fstrcpy (CWeight,CurTheme->IconLibrary);
	ExpandText (CWeight);
	ExpandText (CWeight);
	if (*CWeight)
	{  
		Weight = atof (CWeight);
		HaveWeight = TRUE;
	}
	if (Weight <= 0)
		return;
	SetHotSpotMaskWidth(pHSData);
	if (pHSData->Radius <= 0)
		return;
	HotSpotPoint = TranPoint (&CurPointLocD,pHSData->hTranBaseToHotSpot);
	x = IDNINT (HotSpotPoint.x);
	y = IDNINT (HotSpotPoint.y);
	GridBegRow = y - pHSData->MaskWidth/2;
	if (GridBegRow < 0)
	{
		MaskBegRow = -GridBegRow;
		GridBegRow = 0;
	}
	else
		MaskBegRow = 0;
	GridEndRow = y + pHSData->MaskWidth/2;
	if (GridEndRow > pHSData->GridHeight-1)
	{
		MaskEndRow = pHSData->MaskWidth - (GridEndRow - (pHSData->GridHeight-1));
		GridEndRow = pHSData->GridHeight-1;
	}
	else
		MaskEndRow = pHSData->MaskWidth-1;
	GridBegCol = x - pHSData->MaskWidth/2;
	if (GridBegCol < 0)
	{
		MaskBegCol = -GridBegCol;
		GridBegCol = 0;
	}
	else
		MaskBegCol = 0;
	GridEndCol = x + pHSData->MaskWidth/2;
	if (GridEndCol > pHSData->GridWidth-1)
	{
		MaskEndCol = pHSData->MaskWidth - (GridEndCol - (pHSData->GridWidth-1));
		GridEndCol = pHSData->GridWidth-1;
	}
	else
		MaskEndCol = pHSData->MaskWidth-1;
	if (pHSData->hMask)
	{
		pMaskRow = (HPSHORT)GlobalLock(pHSData->hMask);
		pMaskRow += (MaskBegRow * pHSData->MaskWidth) + MaskBegCol;
		pGridRow = (HPLONG)GlobalLock(pHSData->hGrid);
		pGridRow += (long)pHSData->GridWidth * (long)GridBegRow + (long)GridBegCol;
		row = MaskBegRow;
		while (row < MaskEndRow)
		{
			col = MaskBegCol;
			pMask = pMaskRow;
			pGrid = pGridRow;
			while (col < MaskEndCol)
			{
				if (HaveWeight)
					*pGrid += (*pMask*Weight);
				else
					*pGrid += (*pMask);
				MaxGrid = max(MaxGrid, *pGrid);
				pGrid++;
				pMask++;
				col++;
			}
			row++;
			pMaskRow += pHSData->MaskWidth;
			pGridRow += pHSData->GridWidth;
		}
		GlobalUnlock(pHSData->hGrid);
		GlobalUnlock(pHSData->hMask);
	}
	pHSData->TotalIncidents++;
	pHSData->MaxGridValue = CurTheme->Ymax = max (CurTheme->Ymax,(double)MaxGrid);
{
#if ENABLETRACE
GSSiExitProg (1320);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

BOOL SaveHotSpotSurface (void)
#if ENABLETRACE
{GSSiEnterProg (1321);
#endif
{
	LPHOTSPOTDATA	pHSData = &CurTheme->HotSpotData;
	long	GridSize;
    HPLONG	pGrid;
    HFILE	Fid;
	
//	if (!pHSData->PromptForSave)     
	if (!*CurTheme->HotSpotSaveTo)
{
#if ENABLETRACE
GSSiExitProg (1321);
#endif
		return FALSE;
}
	Fid = GSSiOpenFile (CurTheme->HotSpotSaveTo,NULL,OF_CREATE);
	if (Fid == HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (1321);
#endif
		return FALSE;	
}
	GridSize =  (long)pHSData->GridWidth * (long)pHSData->GridHeight * 4; 
	pGrid =  (HPLONG)GlobalLock (pHSData->hGrid); 
	BigWrite (Fid,(HPSTR)pHSData,sizeof(HOTSPOTDATA),-1);
	BigWrite (Fid,(HPSTR)pGrid,GridSize,-1);
	GSSiClose2 (&Fid);   
	GlobalUnlock (pHSData->hGrid); 
{
#if ENABLETRACE
GSSiExitProg (1321);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

HANDLE LoadHotSpotSurface (LPSTR File)
#if ENABLETRACE
{GSSiEnterProg (1322);
#endif
{
	LPHOTSPOTDATA	pHSData;
	long	GridSize;
    HPLONG	pGrid;
    HFILE	Fid; 
    HANDLE	handle;
	
	Fid = GSSiOpenFile (File,NULL,OF_READ);
	if (Fid == HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (1322);
#endif
		return FALSE;	
}
	handle = GSSiGlobAlloc(GAIDNO 1334,GMEM_MOVEABLE,sizeof(HOTSPOTDATA));
	pHSData = (LPHOTSPOTDATA)GlobalLock (handle);  
	BigRead (Fid,(HPSTR)pHSData,sizeof(HOTSPOTDATA));
	GridSize =  (long)pHSData->GridWidth * (long)pHSData->GridHeight * 4;   
	pHSData->hGrid = GSSiGlobAlloc(GAIDNO 1335,GMEM_MOVEABLE,GridSize);
	pGrid =  (HPLONG)GlobalLock (pHSData->hGrid); 
	BigRead (Fid,(HPSTR)pGrid,GridSize);   
	GlobalUnlock (pHSData->hGrid);
	GSSiClose2 (&Fid); 
	GlobalUnlock (handle);
{
#if ENABLETRACE
GSSiExitProg (1322);
#endif
	return handle;
}
#if ENABLETRACE
}
#endif
} 

BOOL CompareHotSpotData (LPHOTSPOTDATA pHSData)
#if ENABLETRACE
{GSSiEnterProg (1323);
#endif
{   
	HANDLE	hCompare;
	LPHOTSPOTDATA	pHSDataCT;
    HPLONG	pGrid, pGridCT;
    long	GridSize;  
    double Factor=1;
    char	File[150];   
    LPSTR	pFile=File, pEnd; 
    double	Stdv;  
    BOOL	HaveStdv=FALSE;   
    LPSTR	pPar;
	
	if (!*CurTheme->HotSpotCompareTo)
{
#if ENABLETRACE
GSSiExitProg (1323);
#endif
		return FALSE; 
}
	_fstrcpy (File,CurTheme->HotSpotCompareTo);
	ExpandText (File);
	if ((pPar=_fstrchr (File,'(')))
	{
		*pPar++=0;
		HaveStdv = TRUE;
		Stdv = atof (pPar);
	}
	if (!*File)
{
#if ENABLETRACE
GSSiExitProg (1323);
#endif
		return FALSE;
}
/*	if (*File == '(')
	{
		pEnd = _fstrrchr (File,')');
		*pEnd++ = 0; 
		pEnd++;
		Factor = atof (pEnd);
		pFile++;
	} */
	if (!(hCompare = LoadHotSpotSurface (pFile)))
{
#if ENABLETRACE
GSSiExitProg (1323);
#endif
		return FALSE; 
}
	pHSDataCT = (LPHOTSPOTDATA)GlobalLock (hCompare); 
	Factor = 1;
	switch (GetGlobalLVal2 ("[%HOTSPOTCOMPAREOPTION]",2))
	{
		default:
			break;
		case 1:   
			if (pHSDataCT->TotalIncidents)
				Factor = (double)pHSData->TotalIncidents/(double)pHSDataCT->TotalIncidents;
			break;  
		case 2:
			if (pHSDataCT->SecondsRepresented)
				Factor = (double)pHSData->SecondsRepresented/(double)pHSDataCT->SecondsRepresented;
			break;  
		case 3: 
				if (pHSDataCT->MaxGridValue)
					Factor = (double)pHSData->MaxGridValue/(double)pHSDataCT->MaxGridValue;
			break;  
	}		
	pGridCT = (HPLONG)GlobalLock (pHSDataCT->hGrid);   
	pGrid = (HPLONG)GlobalLock (pHSData->hGrid);     
	CurTheme->Ymax = LONG_MIN;
	CurTheme->Ymin = LONG_MAX;
	if (pHSData->GridWidth == pHSDataCT->GridWidth && pHSData->GridHeight == pHSDataCT->GridHeight)
	{
		GridSize =  (long)pHSData->GridWidth * (long)pHSData->GridHeight; 
		if (GetGlobalLVal2 ("[%HOTSPOTCOMPAREOPTION]",0) == 2)
		{   
			double factor1 = (double)pHSData->SecondsRepresented/(60L*60L*24L*365L);
			double factor2 = (double)pHSDataCT->SecondsRepresented/(60L*60L*24L*365L);
			
			if (factor1 && factor2)
			while (GridSize--)
			{ 
				if (*pGrid || *pGridCT)
					*pGrid = *pGrid/factor1 - *pGridCT/factor2; 
				if (HaveStdv)
				{
					if (*pGrid > 4*Stdv)
						*pGrid = 300;
					else if (*pGrid > 3*Stdv)
						*pGrid = 200;
					else if (*pGrid < -4*Stdv)
						*pGrid = -300;
					else if (*pGrid < -3*Stdv)
						*pGrid = -200;
					else
						*pGrid = 0;
				}
				CurTheme->Ymax = max (CurTheme->Ymax,*pGrid);
				CurTheme->Ymin = min (CurTheme->Ymin,*pGrid);
				pGrid++;
				pGridCT++;
			} 
		}
		else if (GetGlobalLVal2 ("[%HOTSPOTCOMPAREOPTION]",0) == 5)
		{
			while (GridSize--)
			{ 
				(*pGrid) *= (*pGridCT++);
				CurTheme->Ymax = max (CurTheme->Ymax,*pGrid);
				CurTheme->Ymin = min (CurTheme->Ymin,*pGrid);
				pGrid++;
			} 
		} 
		else if (GetGlobalLVal2 ("[%HOTSPOTCOMPAREOPTION]",0) == 6)
		{
			while (GridSize--)
			{ 
				if (*pGrid && *pGridCT)
					(*pGrid) -= (*pGridCT++); 
				else
					*pGrid = 0;
				CurTheme->Ymax = max (CurTheme->Ymax,*pGrid);
				CurTheme->Ymin = min (CurTheme->Ymin,*pGrid);
				pGrid++;
			} 
		} 
		else
		{
			while (GridSize--)
			{ 
				(*pGrid) -= (*pGridCT++)*Factor;
				if (HaveStdv)
				{
					if (*pGrid > 4*Stdv)
						*pGrid = 400;
					else if (*pGrid > 3*Stdv)
						*pGrid = 200;
					else if (*pGrid > 2*Stdv)
						*pGrid = 50;
					else if (*pGrid < -4*Stdv)
						*pGrid = -400;
					else if (*pGrid < -3*Stdv)
						*pGrid = -200;
					else if (*pGrid < -2*Stdv)
						*pGrid = -50;
					else
						*pGrid = 0;
				}
				CurTheme->Ymax = max (CurTheme->Ymax,*pGrid);
				CurTheme->Ymin = min (CurTheme->Ymin,*pGrid);
				pGrid++;
			} 
		} 
	}
	else
		GSSiMessageBox (0,"Unable to compare hot spot surfaces due to unequal dimensions",NULL,MB_ICONEXCLAMATION,0);
	GlobalUnlock (pHSData->hGrid);
	GSSiGlobUlFree (&pHSDataCT->hGrid); 
	GSSiGlobUlFree (&hCompare); 
	
{
#if ENABLETRACE
GSSiExitProg (1323);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

void NormalizeHotSpotToCountPerYear (LPTHEME CurTheme)
{
	LPHOTSPOTDATA	pHSData = &CurTheme->HotSpotData;
	double	factor = (double)pHSData->SecondsRepresented/(60L*60L*24L*365L); 
	long	GridSize =  (long)pHSData->GridWidth * (long)pHSData->GridHeight;  
	long	gmin=LONG_MAX,gmax=LONG_MIN;
	HPLONG	pGrid;
	
	if (pHSData->SecondsRepresented == LONG_MAX)
		return;
	pGrid = (HPLONG)GlobalLock (pHSData->hGrid);
	if (factor)
	{
		while (GridSize--) 
		{
			*pGrid = (double)*pGrid/factor; 
			gmin = min (gmin,*pGrid);
			gmax = max (gmax,*pGrid);
			pGrid++;
		}
		pHSData->MaxGridValue = CurTheme->Ymax = gmax;  
		CurTheme->Ymin = gmin; 
	} 
	pHSData->SecondsRepresented = LONG_MAX;
	GlobalUnlock (pHSData->hGrid);
	return;
}

void DisplayHotSpots (void)
#if ENABLETRACE
{GSSiEnterProg (1324);
#endif
{
	LPHOTSPOTDATA	pHSData = &CurTheme->HotSpotData;
	LPVIEWPORT	SaveVP = CurView;
    COLORREF	Color=RGB(255,0,0);  
    HBRUSH	hBrush=GetStockObject(BLACK_BRUSH);  
    RECT	Rect; 
    short	r=255,g=0,b=0,i,j, RectWidth, RectHeight, x,y;
    double	ScreenToGridFactor=0, MV;
    HPLONG	pGridArray = (HPLONG)GlobalLock (pHSData->hGrid), pGridRow, pGrid;
    
    if ((CurTheme->Ymin < 0 || CurTheme->Ymax > 0) && pHSData->GridWidth)
    {   
    	//NormalizeHotSpotToCountPerYear (CurTheme);
    	CompareHotSpotData (pHSData);
    	SaveHotSpotSurface ();
		SetViewport (CurTheme->TargetViewport);   
	    if (CurView->DisplayInParent && CurView->Parent > 0) 
	    	CurView = pViewports[CurView->Parent-1]; 
		SaveDC (CurView->hDC);
	    SetDisplayMode (CurView->hDC,GF_TEXTMODE);
	    SelectClipRgn (CurView->hDC,0);
	    switch (pHSData->ColorOpt)
	    {
	    	case 0:    
	    		MV = max (-CurTheme->Ymin,CurTheme->Ymax);
	    		break;
	    	case 1:
	    		MV = sqrt (max (-CurTheme->Ymin,CurTheme->Ymax));
	    		break;
	    	case 2:
	    		MV = pow (max (-CurTheme->Ymin,CurTheme->Ymax),2);
	    		break;
	    	case 3:
	    		MV = log (max (-CurTheme->Ymin,CurTheme->Ymax));
	    		break;
	    	case 4:
	    		MV = exp (max (-CurTheme->Ymin,CurTheme->Ymax));
	    		break;
	    } 
	    RectWidth = CurView->DrawRect.right - CurView->DrawRect.left + 1;
	    RectHeight = CurView->DrawRect.bottom - CurView->DrawRect.top + 1;
	    if (RectHeight)
	    	ScreenToGridFactor = (double)pHSData->GridHeight/(double)(RectHeight); 
	    i=0;
    	Rect.top = CurView->DrawRect.bottom; 
		y = CurView->DrawRect.bottom;
	    while (y >= CurView->DrawRect.top)
	    {   
	    	pGridRow = pGridArray + (long)(i*ScreenToGridFactor)*pHSData->GridWidth;   
	    	x = CurView->DrawRect.left;
	    	j=0;
	    	while (x <= CurView->DrawRect.right)
	    	{
	    		pGrid = pGridRow + (long)(j*ScreenToGridFactor); 
	    		{
		    		double	GridVal = *pGrid;
		    		
		    		switch (pHSData->ColorOpt)
		    		{
		    			case 1:
		    				GridVal = sqrt (fabs (GridVal)) * Signof (GridVal);                 
		    				break;
		    			case 2:
		    				GridVal = pow (GridVal,2);
		    				break;
		    			case 3:
		    				GridVal = log (GridVal);
		    				break;
		    			case 4:
		    				GridVal = exp (GridVal);
		    				break;
		    			default:
		    				break;
		    		}
//		    	Rect.left=Rect.right; 
//	    		Rect.right = Rect.left + pHSData->Granularity;  
			    	if (*pGrid>0)  
			    	{
//	    			g = b = 255 * ((1.0-(((double)100-pHSData->ColorMin)/(double)100)*(GridVal / MV))*((double)(100-pHSData->ColorMin)/100));
//	    			g = b = 255 * ((1.0-(((double)100)/(double)100)*(GridVal / MV))*((double)(100)/100));
	    			if (MV)
	    				g = b = 255 * (1.0-(GridVal / MV));
			    	Color = RGB(255,g,b);
		    		SetPixel (CurView->hDC,x,y,Color);
/*		    		hBrush = CreateSolidBrush (Color);
		    		FillRect (CurView->hDC,&Rect,hBrush);
		    		DeleteObject (hBrush); */
					}
			    	else if (*pGrid<0)  
			    	{
//	    			g = r = 255 * ((1.0-(((double)100-pHSData->ColorMin)/(double)100)*(fabs(GridVal) / MV))*((double)(100-pHSData->ColorMin)/100));
	    			if (MV)
	    				g = r = 255 * ((1.0-(((double)100)/(double)100)*(fabs(GridVal) / MV))*((double)(100)/100));
			    	Color = RGB(r,g,255);
		    		SetPixel (CurView->hDC,x,y,Color);
/*		    		hBrush = CreateSolidBrush (Color);
		    		FillRect (CurView->hDC,&Rect,hBrush);
		    		DeleteObject (hBrush);*/
					}
	    		} 
	            j++; 
	            x++;
	        }
	    	i++;
	    	y--;
	    }
	    GSSiDeleteObject (&CurTheme->hHotSpotBitmap);  
		CurTheme->hHotSpotBitmap = SaveScreen (CurView->hDC,CurView->DrawRect);  
		CurTheme->HotSpotBounds = CurView->WBounds;
		RestoreDC (CurView->hDC,-1);  
		SetCurView (SaveVP);
	}
	GlobalUnlock (pHSData->hGrid);
{
#if ENABLETRACE
GSSiExitProg (1324);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

