#include "graphint.h"

#include "gmextern.h"

#include <sys\types.h>
#include <sys\stat.h>         

BOOL maxIntensity = FALSE;
static	char	Value[512], KeyVal[512], VarVal[1024], MinClassValue[512];
static THEMEHIGHLIGHTKEY	ThemeHighlightKey;
static THEMEHIGHLIGHTDATA	ThemeHighlightData;

                                         
static struct {
		long	MaskAreaRefno;
		long	AreaRefno;
		} ShowValKey;  

typedef struct {
		long	MaskAreaRefno;
		long	AreaRefno; 
		char	Prefix[8],
				UDI[64];
		DPOINT	Point;
		float	Size;
		} SHOWVALDATA;  
typedef SHOWVALDATA	FAR	*LPSHOWVALDATA;

static	HANDLE	hShowValDB=0;
static	char	ShowValDB[256]="";                                         

void SetShowValDB (LPSTR DBName)
{
	if (DBName)
		_fstrcpy (ShowValDB,DBName);
	else
	{
		CloseGWDatabase (hShowValDB);
		hShowValDB = 0;
	}
	return;
}

void SetShowValPoly (long Refno,BOOL Close)
#if ENABLETRACE
{GSSiEnterProg (1225);
#endif
{   
	UINT	i; 
	HPPOINT	pPoint;
	HPPOINTS lpPoints=lpCurPoints;
	HPDPOINT	lpDPoints=lpDCurPoints;
	
	GSSiGlobFree (&ShowVal.hPoints); 
	GSSiGlobFree (&ShowVal.hPolyPartLen); 
	if (Close)
		goto Exit; 
	ShowVal.Refno = Refno;
	ShowVal.nPnts = nPnts;
	ShowVal.hPoints = GSSiGlobAlloc (1323,GMEM_MOVEABLE,(long)sizeof(POINT) * (long)nPnts); 
	pPoint = (HPPOINT)GlobalLock (ShowVal.hPoints);
	for (i=0;i<nPnts;i++,pPoint++) 
	{   
		if (HiPrecis)
			*pPoint = BasePtToScreenPt (lpDPoints++);   
		else
    		*pPoint = FileCoordToWinCoord(POINTStoPOINT(*lpPoints++));
    } 
    GlobalUnlock (ShowVal.hPoints);
    if (hPolyPartLen && nPoly)
    {    
    	LPINT	pPartLen, pPartLen2;
    	
    	ShowVal.nPoly = nPoly;
    	ShowVal.hPolyPartLen = GSSiGlobAlloc ( 309,GMEM_MOVEABLE,(long)nPoly*sizeof(int));
  		pPartLen = (LPINT)GlobalLock (hPolyPartLen);
   		pPartLen2 = (LPINT)GlobalLock (ShowVal.hPolyPartLen);    
   		hmemmove ((HPSTR)pPartLen2,(HPSTR)pPartLen,(long)nPoly*sizeof(int));
   		GlobalUnlock (hPolyPartLen);
   		GlobalUnlock (ShowVal.hPolyPartLen);
    }
    else 
    {
    	ShowVal.nPoly = 0;  
    	ShowVal.hPolyPartLen = 0;
    }
Exit:
{
#if ENABLETRACE
GSSiExitProg (1225);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

BOOL SwitchThemeSHPFile (void)
{   
	LPSTR	CurShpFile, SaveThemeDataFile, DBFullPath;
	BOOL	rtn=FALSE;
	LPOPENSQLDATA	SQLPtr;
	LPOPENFILEDATA	FilePtr;
	HANDLE	hMem=0; 
	LPSTR	pDot=0;  
	long	SaveCSR;
	char DataFile[MAX_PATH];
	
	if (!CurTheme)
		return FALSE;
	strcpy(DataFile, CurTheme->DataFile);
	ExpandText(DataFile);
	if (strstr (DataFile,".GDB("))
		CurTheme->DataFileType = FGDB_DATAFILE;
	if (strstr (DataFile,".MDB("))
		CurTheme->DataFileType = PGDB_DATAFILE;
	if (CurTheme->DataFileType != SHAPE_DATAFILE &&
		CurTheme->DataFileType != ORA_DATAFILE &&
		CurTheme->DataFileType != PGDB_DATAFILE &&
		CurTheme->DataFileType != FGDB_DATAFILE)// &&
		//!_fstrchr(&DataFile[1],'['))
		return FALSE;
	if (strstr (DataFile,"graphics.gmd"))
		return FALSE;
	if (strstr (DataFile,"graphic2.gmd"))
		return FALSE;
	hMem = GSSiGlobAlloc (1523,GMEM_MOVEABLE,1024);
	CurShpFile = GlobalLock (hMem);
	SaveThemeDataFile = CurShpFile + 256;
	DBFullPath = SaveThemeDataFile + 256;
	if (CurTheme->DataFileType == SHAPE_DATAFILE)
	   	GetSHPName (CurShpFile); 
	else if (CurTheme->DataFileType == FGDB_DATAFILE)
	{
	   	GetSHPName (CurShpFile); 
	}
	else if (CurTheme->DataFileType == ORA_DATAFILE)
		GetORAName (CurShpFile);
	else if (CurTheme->DataFileType == PGDB_DATAFILE)
	{
		LPSTR	eqLoc;

		_fstrcpy (CurShpFile,DataFile);
		if (strncmp (CurShpFile,"ODBC|",5) &&
			(eqLoc = strchr (CurShpFile,'=')))
			CurShpFile = eqLoc+1;
	} 
   	if (CurTheme->hThemeDB)
   	{
		SQLPtr = (LPOPENSQLDATA) GlobalLock (CurTheme->hThemeDB);
	    FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
	    _fstrcpy (DBFullPath,FilePtr->fullpath);
		GlobalUnlock (SQLPtr->OFHandle);
		GlobalUnlock (CurTheme->hThemeDB); 
		if (CurTheme->DataFileType == FGDB_DATAFILE)
		{
			if (!stricmp (CurShpFile,DBFullPath))
				goto Exit;
		}
		else
		{
			if ((pDot = _fstrrchr (DBFullPath,'.')))
				*pDot = 0;
			if ((pDot = _fstrrchr (CurShpFile,'.')))
				*pDot = 0;
			if (!_fstricmp (CurShpFile,DBFullPath))
				goto Exit; 
			if (pDot)
				*pDot = '.';
		}
		CloseThemeDataFile(TRUE);
	}
	SubstituteDL(CurShpFile, FALSE);
	SaveCSR = CurrentSHPRec;
	OpenThemeDataFile(CurShpFile);
	CurrentSHPRec = SaveCSR;
Exit:  
	GSSiGlobUlFree (&hMem);
	return rtn;
}

void GetNextBlack (LPLONG pColor)
{
	BYTE r = GetRValue (*pColor);
	BYTE g = GetGValue (*pColor);
	BYTE b = GetBValue (*pColor);//use when classno expanded to 4 bytes
	if (g < r)
		g+=4;//g++;
	else if (r < b) 
		r+=4;//r++; 
	else
		b+=4;
	*pColor = RGB(r,g,b); 
	return;
}
	
long GetNextUniqueValueColor (LPLONG pNextColor,short VP)
{
	POINT	p=RectMid (&pViewports[VP-1]->DrawRect);   //pViewports[17]
	long	LastColor = *pNextColor;
	long	Color, SaveColor,ii;
	
	SaveDC (pViewports[VP-1]->hDC);
    SetDisplayMode (pViewports[VP-1]->hDC, GF_TEXTMODE);
  	SelectClipRgn (pViewports[VP-1]->hDC,0);
	SaveColor = GetPixel (pViewports[VP-1]->hDC,p.x,p.y);
	GetNextBlack (pNextColor);
	if ((long)SetPixel (pViewports[VP-1]->hDC,p.x,p.y,*pNextColor) < 0) 
	{
		if (SaveColor > -1)
			SetPixel (pViewports[VP-1]->hDC,p.x,p.y,SaveColor);
		RestoreDC (pViewports[VP-1]->hDC,-1);
		return *pNextColor;
	}
	while ((Color=GetPixel (pViewports[VP-1]->hDC,p.x,p.y)) != *pNextColor && *pNextColor > LastColor)
	{
		LastColor = *pNextColor;
		GetNextBlack (pNextColor); 
		ii=SetPixel (pViewports[VP-1]->hDC,p.x,p.y,*pNextColor);
	}
	if (SaveColor > -1)
		SetPixel (pViewports[VP-1]->hDC,p.x,p.y,SaveColor);
	RestoreDC (pViewports[VP-1]->hDC,-1);
	return *pNextColor;  
}

BOOL DisplayThemeDirection (void)
{
	DPOINT	Points[3];
	DOUBLE	Az, Dist=0, Len, DistBtwnArrows=50, ArrowWidth=10,inc;
	int		nArrows, i;

	if (!HiPrecis || (CurrentType != GF_LINE && CurrentType != GF_POLYLINE && CurrentType != GF_CURVE))
		return FALSE;
	Len = GetPolyLengthD (lpDCurPoints,nPnts);
	nArrows = Len / (CurView->Scale*DistBtwnArrows);
	inc = Len / (nArrows + 1);
	for (i=0;i<nArrows;i++)
	{
		Dist += inc;
		Points[1] = PointAtDistOnPoly (lpDCurPoints,nPnts,Dist,&Az,0);
		Points[0] = dnewpt (Points[1],Az+1.5*HALFPI,CurView->Scale*ArrowWidth);
		Points[2] = dnewpt (Points[1],Az-1.5*HALFPI,CurView->Scale*ArrowWidth);
		GWPolylineD (CurView->hDC, Points, 3,0);   
	}
	return TRUE;
}

BOOL SymTAGMatchTheme (LPSTR TAG,int desc)
{
	if (CurTheme->SymNum == -1)
	{
		if (_fstricmp (TAG,&CurTheme->Contents[1])) 
			return FALSE;
	}
	else if (CurTheme->hVisList)
	{   
		LPVISLIST	SaveVis=CurVis; 
		BOOL		WantDesc;
		
		CurVis = (LPVISLIST)GlobalLock (CurTheme->hVisList);
		WantDesc = GetVisibility (desc);
		GlobalUnlock (CurTheme->hVisList);
		CurVis = SaveVis;
		if (!WantDesc)
			return FALSE;
	}
	return TRUE;
}

BOOL TestCityRect (LPRECT pTestRect,LPINT pnTestRect,LPRECT pRect)
{
	RECT newRect, rect=*pRect;
	int	i;

	if (CurTheme->CityTextRectInflateFactor > 0)
	{
		float	f=(float)CurTheme->CityTextRectInflateFactor / 10.0;
		int	xinc = RECTWIDTH (&rect)  * f;
		int	yinc = RECTHEIGHT (&rect) * f;

		InflateRect (&rect,xinc,yinc);
	}
	for (i=0;i<*pnTestRect;i++)
		if (IntersectRect (&newRect,&rect,&pTestRect[i]))
			return FALSE;

	pTestRect[(*pnTestRect)++] = rect;
	return TRUE;
}

double CityPopConvert (int pop)
{
	pop -= CurTheme->MinCityPop;
	pop++;
	switch (CurTheme->CityTextSizeOpt)
	{
	default:
		return pop;
	case 1:
		return log10 (pop);
	case 2:
		return log (pop);
	case 3:
		return sqrt (pop);
	case 4:
		return pow ((double)pop,0.4);
	case 5:
		return pow ((double)pop,0.3);
	case 6:
		return pow ((double)pop,0.2);
	}
}

void DisplayCityNames(void)
{
	CITY_NAME_KEY	CityKey;
	CITY_NAME_DATA	CityData;
	int	pos = BT_FIRST;
	double	Size, Radius, Width;
	int		Weight;
	int		Yoff;
	double	TextFactor = GetGlobalDVal2 ("[TEXT_FACTOR]",1); 
	int		CityTextVJust=1;
	int		nDisplayed=0;
	RECT	textRect, fullRect;
	HANDLE	hTestRect = GSSiGlobAlloc (0,GMEM_MOVEABLE,(CurTheme->CityUniqueInc+1)*sizeof(RECT));
	LPRECT	pTestRect = GlobalLock (hTestRect);
	int		nTestRect=0, i;
	double	maxPop = CityPopConvert (CurTheme->MaxCityPopOnScreen);
	char	SaveFont[128];
	
    SetViewport (CurTheme->TargetViewport); 
	SaveDC (CurView->hDC);
	if (!CurTheme->ShowCityCircle)
		CityTextVJust = 2;  

	DispText (0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-1,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0);
	SetTextColor (CurView->hDC,ConvertColor (CurTheme->ShowValueTextColor,CurTheme->UseHalfTone));  
	SetBkMode (CurView->hDC,TRANSPARENT);
	GetGlobalCVal ("[%LABELFONT]",SaveFont,0);
	SetGlobalValue ("%LABELFONT",CurTheme->ShowValueFont.lfFaceName);

	if (CurTheme->hScatterFile)
	{
		while (!BT_FIND (CurTheme->hScatterFile,(LPSTR)&CityKey,pos,BT_ANY,(LPSTR)&CityData))
		{
			int	pop = abs(CityKey.Population);

			pos = BT_NEXT;
			for (i=0;i<nTestRect;i++)
				if (PtInRect (&pTestRect[i],CityData.Point))
					continue;
			Size = CurTheme->CityTextMinSize + 
				   (CurTheme->CityTextMaxSize - CurTheme->CityTextMinSize) *
				   CityPopConvert (pop)/maxPop;
			Size = max (Size,CurTheme->CityTextMinSize);
			Size *= TextFactor;
			Weight = 300 + (Size - CurTheme->CityTextMinSize) * 100;
			Radius = Size/2;
			Width = Size/CurTheme->CityTextMinSize - 1; 
			Yoff =  Radius+Width/2; 
			RectInit (&fullRect);
			RectInit (&textRect);
			DispText (CurView->hDC,2,CityData.Point.x-10,CityData.Point.x+10, CityData.Point.y-Yoff, 0,2,CityTextVJust, Radius/20, 1,1,
				Weight, FALSE, -CurView->Rotation,CityData.Text,0,FALSE,CurTheme->UseShadowColor,CurTheme->ShowValueShadowColor,-1,0,0,0,0,0,CurTheme->UseHalfTone,0,0,0,&textRect,&fullRect,0);
			if (TestCityRect (pTestRect,&nTestRect,&fullRect))
			{
				DispText (CurView->hDC,0,CityData.Point.x-10,CityData.Point.x+10, CityData.Point.y-Yoff, 0,2,CityTextVJust, Radius/20, 1,1,
					Weight, FALSE, -CurView->Rotation,CityData.Text,0,FALSE,CurTheme->UseShadowColor,CurTheme->ShowValueShadowColor,-1,0,0,0,0,0,CurTheme->UseHalfTone,0,0,0,&textRect,&fullRect,0);
					nDisplayed++;
			}
			else if (CityTextVJust != 2)// try below point
			{
				RectInit (&fullRect);
				RectInit (&textRect);
				DispText (CurView->hDC,2,CityData.Point.x-10,CityData.Point.x+10, CityData.Point.y-Yoff, 0,2,3, Radius/20, 1,1,
					Weight, FALSE, -CurView->Rotation,CityData.Text,0,FALSE,CurTheme->UseShadowColor,CurTheme->ShowValueShadowColor,-1,0,0,0,0,0,CurTheme->UseHalfTone,0,0,0,&textRect,&fullRect,0);
				if (TestCityRect (pTestRect,&nTestRect,&fullRect))
				{
					DispText (CurView->hDC,0,CityData.Point.x-10,CityData.Point.x+10, CityData.Point.y-Yoff, 0,2,3, Radius/20, 1,1,
						Weight, FALSE, -CurView->Rotation,CityData.Text,0,FALSE,CurTheme->UseShadowColor,CurTheme->ShowValueShadowColor,-1,0,0,0,0,0,CurTheme->UseHalfTone,0,0,0,&textRect,&fullRect,0);
						nDisplayed++;
				}
			}
			if (CurTheme->MaxCitiesToDisplay && nDisplayed >= CurTheme->MaxCitiesToDisplay)
				break;
		}
	}
	SetGlobalValue ("%LABELFONT",SaveFont);
	GSSiGlobUlFree (&hTestRect);
	RestoreDC (CurView->hDC,-1);
	return;
}

short ThemeSetChar (int Type, long iref, int desc, LPSTR TAG, LPSTR UDI)
#if ENABLETRACE
{GSSiEnterProg (1262);
#endif
{
// Returns -1 if element not processed by this theme
//          0 if element should not be displayed (missing data and theme missopt set to do not display)
//			1 if characteristics set by this theme   
//			2 if processed but graphics char not set (Set Color false)
	long MktVal;   
	static	long	LastDSRef; 
	static	double	StartDSDist, CurDSLength;
	int	 iclass, HaveClass, ClassNo, ClassNo2;
	double	MktValD, MidPointAZ;
	double	ValD, HaveValD,Rem;
	short	status;
	MIDPOINT	MidPoint; 
	LPMIDPOINT	pMidPoint;
	double	AZ;  
	LPSHORT	pInt;   
	POINT	Point;
	NETLINKSKEY	NetLinksKey; 
	NETLINKSDATA	NetLinksData;
	NETMARKERSTHEMEKEY	NetMarkersThemeKey1, NetMarkersThemeKey2;
	double		LastEndMP, NextEndMP, EndMP, ClassMin, ClassMax;
	short		st, st1, st2, Case, ii, ipos, iposinc, lnKey;
	static		ULONG		iposswitch=0;
	BOOL		rc, InClass; 
	BYTE		PatByt;
	PATBYTE		PatByte;
	LPPROFILETHEMEDATA	lpProfileData;  
	double		TrueValD, HaveTrueValD; 
	BOOL		GetFirstClass=TRUE, AtEndOfValues;
	static	long	SymbolNumberColor=0,SymbolNumberColorType=-1;
	short	MinClass=MAX_THEME_CLASSES+1;

    if (desc == 376)
		ii = 1;

    if (!CurTheme)
{
#if ENABLETRACE
GSSiExitProg (1262);
#endif
    	return (-1);
}
	if (CurTheme->ID == PF_COORD_DISPLAY || CurTheme->ID == PF_BOUNDS_DISPLAY ||  CurTheme->ID == GF_CACHE_DISPLAY_THEME ||
		CurTheme->ID == GF_NORTH_ARROW_THEME || CurTheme->ID == GF_PROFILE_LINK_THEME ||
		!CurTheme->IsActive|| !CurTheme->VPDisplayed)
{
#if ENABLETRACE
GSSiExitProg (1262);
#endif
			return (-1);
}
    
    HaltReport = FALSE;
	ThemeDisplayPass = 1;
	*CurTheme->CurValue = 0;
	switch (CurTheme->ID)    
	{ 
		case GF_CITY_THEME:
		{
			CITY_NAME_KEY	CityKey;
			CITY_NAME_DATA	CityData;
			int				pop;

			if (!CurTheme->hScatterFile || CurrentType != GF_POINT)
				goto RtnNotProcessed;
			if (!SymTAGMatchTheme (TAG,desc))
				goto RtnNotProcessed;
			SwitchThemeSHPFile ();		
			status = GetCharFieldData (CurTheme->hThemeDB,
				&CurTheme->Field, iref, CurTheme->FieldFun, CurTheme->Value, CityData.Text, MAX_CITYNAME_LENGTH,CurTheme->DataFileID, 0);
			if (!status)
			{
				status = GetCharFieldData (CurTheme->hThemeDB,
									&CurTheme->Field,iref,CurTheme->FieldFun,CurTheme->ClassBM[0],Value,sizeof(Value),CurTheme->DataFileID,0);
				pop = atoi (Value);
				if (pop >= CurTheme->MinCityPop)
				{
					CityKey.Population = -pop;
					CityKey.Unique = CurTheme->CityUniqueInc++;
					CityData.Point = CurPointLoc;
					CityData.SymNum = desc;
					BT_PUT (CurTheme->hScatterFile,(LPSTR)&CityKey,(LPSTR)&CityData);
					if (CurTheme->CompareAttributes)//auto max pop setting
						CurTheme->MaxCityPopOnScreen = max (CurTheme->MaxCityPopOnScreen,pop);
				}
			}
			goto RtnNoDisplay;
		}
		break;

		case GF_PROFILE_THEME: 
			lpProfileData = (LPPROFILETHEMEDATA)&CurTheme->ClassBM; 
			if (Type == GF_LINE || Type == GF_POLYLINE || Type == GF_CURVE || Type == GF_AREA)
			{   
				HANDLE	hHLT=GSSiGlobAlloc (1325,GMEM_MOVEABLE,sizeof(HIGHLIGHTDATA));
				LPHIGHLIGHTDATA	pHighlightData=(LPHIGHLIGHTDATA)GlobalLock (hHLT);
				
		    	if (!BT_FIND (hHighlight,(LPSTR)&iref,BT_FIRST,BT_EQ,(LPSTR)pHighlightData))
		    	{
		    		lpProfileData->MinSeq = min (lpProfileData->MinSeq,pHighlightData->Sequence);
		    		lpProfileData->MaxSeq = max (lpProfileData->MaxSeq,pHighlightData->Sequence);
		    	}
		    	GSSiGlobUlFree (&hHLT);
		    }
			break;
			
		case GF_DYNAMIC_SEG_THEME:
			GSSiGlobFree (&hDynamicSeg); 
			if (PolyIsHiPrecis && (Type == GF_POLYLINE || Type == GF_CURVE)) 
			{
				double		SegDist;
				if (CurTheme->SymNum == -1)
				{
					if (_fstricmp (TAG,&CurTheme->Contents[1])) 
					{
						LastDSRef = LONG_MAX;
						goto RtnNotProcessed;
					}
	
				}
				else if (CurTheme->hVisList)
				{   
					LPVISLIST	SaveVis=CurVis; 
					BOOL		WantDesc; 
					
					CurVis = (LPVISLIST)GlobalLock (CurTheme->hVisList);
					WantDesc = GetVisibility (desc);
					GlobalUnlock (CurTheme->hVisList);
					CurVis = SaveVis;
					if (!WantDesc)
					{
						LastDSRef = LONG_MAX;
						goto RtnNotProcessed;
					}
				}
				if (iref != LastDSRef)
				{ 
					StartDSDist = 0;
					CurDSLength = GetPolyLengthD (lpDCurPoints,nCurPoints);
				}
				if (StartDSDist >= CurDSLength)
				{
					LastDSRef = LONG_MAX;
					goto RtnNotProcessed;
				} 
				SegDist = max (BaseDistToWinDist*2,CurTheme->RoundTo);
				hDynamicSeg = GetPolyBetweenDist (lpDCurPoints,nCurPoints,StartDSDist,StartDSDist+SegDist,&nDynamicCoord,FALSE,FALSE);
				StartDSDist += SegDist;
				LastDSRef = iref;
			}
			else
				LastDSRef = LONG_MAX;
			goto RtnNotProcessed;
		
		case GF_STREET_ADDRESS_THEME:
		{   
			
			if (!CurTheme->hScatterFile)
				goto RtnNotProcessed;
			pInt = (LPSHORT)GlobalLock (CurTheme->hScatterFile);
			if ((long)(*pInt+2) * sizeof(long) < USHRT_MAX)
			{	
				int	n; 
				LPLONG	pRef;
					
				n = *pInt;
				(*pInt)++;
				pInt++;
				pRef = (LPLONG)pInt;
				pRef += n; 
				*pRef = CurrentRefno;
				GlobalUnlock (CurTheme->hScatterFile);
			} 
			else
			{
				GSSiGlobUlFree (&CurTheme->hScatterFile);
				goto RtnNotProcessed;
			}
		}
		goto RtnNotProcessed;
		
		case GF_POLYINFO_THEME:
			PolyInfoTheme = 0;  
			if (CurrentType == GF_AREA || CurrentType == GF_POLYLINE)
			{   
				DPOINT	IntPoint;
				
				if (HiPrecis) 
					if ((PolyInfoTheme=PolyCrossesItself (CurrentType,1,nPnts,lpDCurPoints,&IntPoint)))
					{ 
						POINT	PointLoc=BasePtToWinPt (&IntPoint);
//                    if (HaveLinkLines (lpDCurPoints,nPnts)) lpDCurPoints[0]
                    	DisplayPointItem (CurView->hDC,PointLoc,15*DeviceToScreenFactor(),0,InvisiblePointSymbol,0);
                    }
			}
			goto RtnNotProcessed;
			
		case GF_BOUNDS_DISPLAY_THEME:
		case GF_TRANSFORM_THEME:
			goto RtnNotProcessed;
			
		case GF_STREET_TEXT_THEME:
		{   
			MNMXCORL MinMax;    
			short	Piece=0,ii;
		    LPSTREETTEXTDATA	pStreetData=(LPSTREETTEXTDATA)CurTheme->ClassBM;
		    static long	DebugRef=222115; 
		    long	MaxBounds;  
		    BOOL	ColorSet=FALSE; 
		    double	WantDist=GetGlobalDVal2 ("[%MINDISTBETWEENNAMES]",0)/2;
			
			if (!SymTAGMatchTheme (TAG,desc))
				goto RtnNotProcessed;
			if (iref == DebugRef)
				ii=1;
			if (FileNum >= MaxTextLayer || LayerID != pStreetData->LayerID || !CurTheme->hScatterFile || 
				/*CurrentType == GF_AREA || 07/11/2006 for lakemaster names on lakes*/ 
				(!pStreetData->NameSource && !pStreetData->MinSize && iref%TextFreq))
				goto RtnNotProcessed;  

			DispersePoint (iref,-1,desc,"Street Name");

			if (TextFreq < 2 && pStreetData->AllowHollow)
			{
				if (UseHollowStreets)
					StreetCenterline = CurTheme;
				else
					StreetCenterline = 0;
			}
			if (*pStreetData->VisMacro)
			{
				HANDLE	hMem=GSSiGlobAlloc (1326,GMEM_MOVEABLE,512);
				LPSTR	pMem=GlobalLock (hMem);
				
				_fstrcpy (pMem,pStreetData->VisMacro);
				ExpandText (pMem);
				rc = atob (pMem);
				GSSiGlobUlFree (&hMem);
				if (!rc)
					goto RtnNoDisplay;
			} 
			if (pStreetData->NameSource && (CurrentType == GF_POINT || CurrentType == GF_AREA))
				pStreetData->IgnoreShields = TRUE;
			if (*pStreetData->ColorMacro)
			{
				HANDLE	hMem=GSSiGlobAlloc (1327,GMEM_MOVEABLE,512);
				LPSTR	pMem=GlobalLock (hMem);
				COLORREF	Color;
				short	R,G,B,W;
				
				_fstrcpy (pMem,pStreetData->ColorMacro);
				ExpandText (pMem);  
				if (*pMem)
				{
					LPSTR pBar = strchr (pMem,'|');

					if (pBar)
					{
						*pBar++ = 0;
						pStreetData->ShadowColor = atol (pBar);
					}
					if (sscanf (pMem,"%ld %hi",&Color,&W) == 1)
						W = 0;
					R = GetRValue(Color);
					G = GetGValue(Color);
					B = GetBValue(Color);
					CurTheme->ClassColor[0] = RGBW(R,G,B,W); 
					CurTheme->NumDesiredClass = 1;  
					CurTheme->DataType = 2;   
					CurTheme->NotSetColor = 0; 
					SelectObject (CurView->hDC,GetStockObject(NULL_BRUSH));
					SelectObject (CurView->hDC,GetStockObject(NULL_PEN));
					AdjustBlue = 253;
					CreateThemePens (CurTheme,TRUE); 
					SetThemeElementCharacteristics (0);  
					ColorSet = TRUE; 
				}
				GSSiGlobUlFree (&hMem); 
			} 
	   		if (!pStreetData->NameSource && !CurStreet && !*CurStreetNumbers)
	   		{
	   			if (ColorSet)
		   			goto RtnProcessed;
	   			else
					goto RtnNotProcessed; 
			}
			MaxBounds = max ((long)CurrentItemMinMax.xmx - (long)CurrentItemMinMax.xmn,(long)CurrentItemMinMax.ymx-(long)CurrentItemMinMax.ymn);
			if (MaxBounds * FileDistToWinDist < pStreetData->MinSize)
	   		{    
	   			mnmxCor	CurrentItemMinMax2=CurrentItemMinMax;
	   			if (ColorSet)
		   			goto RtnProcessed;
	   			else
					goto RtnNotProcessed; 
			}
			if (*pStreetData->DisplayNameMacro)
			{
				HANDLE	hMem=GSSiGlobAlloc (1328,GMEM_MOVEABLE,512);
				LPSTR	pMem=GlobalLock (hMem);
				
				_fstrcpy (pMem,pStreetData->DisplayNameMacro);
				ExpandText (pMem);
				rc = atob (pMem);
				GSSiGlobUlFree (&hMem);
				if (!rc)
		   		{
		   			if (ColorSet)
			   			goto RtnProcessed;
		   			else
						goto RtnNotProcessed; 
				}
			} 
			
			while ((!StreetCenterline || pStreetData->NameSource) && CurrentMidPoint (Type,0,&MidPoint.AZ,&MidPoint.Length,&MidPoint.height,TRUE, &MinMax,&Point,0,&Piece,1,&WantDist,pStreetData->RotateToScreen))
			{   
				if (ColorSet)
				{
					MidPoint.ShadowColor = pStreetData->ShadowColor;
					MidPoint.TextColor = CurTheme->ClassColor[0];
				}
				else
				{
					MidPoint.TextColor = 0;
					MidPoint.ShadowColor = RGB (255,255,255);
				}
            	ipos = 0;  
            	if (Piece == 1)
            		WantDist *= 2;   
				if (pStreetData->ShowAllElements && !pStreetData->NameSource && !CurStreet)
				{
					while (ipos < 4 && CurStreetNumbers[ipos])
						ipos++; 
					ipos = iposswitch % ipos;
					iposinc = -1;
				}
				else if (PrimeNameOnly)
				{
					for (ipos = 3;ipos;ipos--)
						if (ipos < 0)
							break;
				}
				else 
				{
	            	iposinc = 1;
	            }
	            iposswitch++; 
   NextSNUM: 
	   			if (pStreetData->NameSource && !ipos)
	   			{   
	   				long	NameID; 
	   				LPSTR	pSlash, pValue;
					short iname = 0; 
	   				
	   			//	ipos = -1;
			    	_fstrcpy (Value,pStreetData->Expression);
					ExpandTextDataNotFound=FALSE;
			    	ExpandText (Value);
			    	if (!CurTheme->hThemeDB || ExpandTextDataNotFound)
			    	{
			    		if (CurTheme->MissOpt)
			    			goto RtnNoDisplay; 
			    		else
				   		{
				   			if (ColorSet)
					   			goto RtnProcessed;
				   			else
								goto RtnNotProcessed; 
						}
					}
					if (!*Value)
			   		{
			   			if (ColorSet)
				   			goto RtnProcessed;
			   			else
							goto RtnNotProcessed; 
					}
//					GetCharFieldData (CurTheme->hThemeDB,&CurTheme->Statement,
//									  &CurTheme->Field[0],iref,0,CurTheme->Value,Value,&status);
//					if (status || !*Value)
//						goto RtnNotProcessed;
					_fmemset (&CurStreetNumbers[1],0,12);  
					pValue = Value; 
					iname = 0;
					if ((pSlash = _fstrchr (pValue,'/'))) 
					{
						if (pSlash != pValue && _fstrncmp (pSlash-1,"1/2",3))
						{
							*pSlash++ = 0;
							Truncate (pValue);   
							pSlash = FirstNonBlank (pSlash); 
							if (strchr (pSlash,'/') && !strstr (pSlash,"1/2"))
								iname = 2;
							else
								iname = 1;
						} 
						else
							pSlash = 0;
					} 
					_fstrcpy (KeyVal,pValue);
					{
			NextName:
						PadString (KeyVal,0,GetBTKeyLen(pStreetData->hNameFile1));
						if (BT_FIND (pStreetData->hNameFile1,(LPSTR)KeyVal,BT_FIRST,BT_EQ,(LPSTR)&NameID))
						{
			   				HANDLE	hName2=GSSiGlobAlloc (1329,GMEM_MOVEABLE,256);
			   				LPSTR	pName2 = GlobalLock (hName2);
			   				
							if (BT_FIND (pStreetData->hNameFile2,(LPSTR)&NameID,BT_LAST,BT_ANY,pName2))
								NameID=0;  
							GSSiGlobUlFree (&hName2);
							NameID++;
							BT_PUT (pStreetData->hNameFile1,(LPSTR)KeyVal,(LPSTR)&NameID);
							BT_PUT (pStreetData->hNameFile2,(LPSTR)&NameID,(LPSTR)KeyVal); 
						}
		   				MidPoint.ref = NameID;    
						CurStreetNumbers[iname] = NameID;  
						if (pSlash)
						{
							LPSTR	pSlash2 = strchr (pSlash,'/');

							if (iname && pSlash2 && !strstr (pSlash,"1/2"))
							{
								*pSlash2++ = 0;
								_fstrcpy (KeyVal,pSlash);
								pSlash = pSlash2;
								iname = 1;
							}
							else
							{
								_fstrcpy (KeyVal,pSlash);
								pSlash = 0;
								iname = 0;
							}
							goto NextName;
						} 
					}
	   			}
	            else if (CurStreet) 
	            {
	            	ipos = -1;
	            	MidPoint.ref = CurStreet;  
	            }
	            else if (ipos < 0)
		   		{
		   			if (ColorSet)
			   			goto RtnProcessed;
		   			else
						goto RtnNotProcessed; 
				}
				else
	            {
					MidPoint.ref = CurStreetNumbers[ipos]; 
					if (MidPoint.ref < 0 && PrimeNameOnly)//pStreetData->PrimeNameOnly)
					{
						MidPoint.ref = labs (MidPoint.ref);
						ipos =-1;
					}
					else if (!MidPoint.ref)
			   		{
			   			if (ColorSet)
				   			goto RtnProcessed;
			   			else
							goto RtnNotProcessed; 
					}
					MidPoint.ref = labs (MidPoint.ref);  
				}	 
				MidPoint.Layer = FileNum;  
				MidPoint.State = CurState;  
				MidPoint.symnum = desc;
   				MidPoint.Refno = iref;
				if (PtInDrawRect (Point))
				{   
					if (ipos>0 && !ShieldsOnly)   
					{
						DPOINT	BasePt = dnewpt (WinPtToBasePt(Point),MidPoint.AZ,ipos * WinDistToWorldDist (25));
						MidPoint.Point= BasePtToWinPt (&BasePt); 
					}
					else
						MidPoint.Point=Point;
					MidPoint.width = MinMax.xmx - MinMax.xmn;
					pMidPoint = (LPMIDPOINT)GlobalLock (CurTheme->hScatterFile);
					if (CurTheme->NumMidpoint < MaxMidpoints)
					{	
						pMidPoint += CurTheme->NumMidpoint; 
						CurTheme->NumMidpoint++;
						*pMidPoint = MidPoint;
						GlobalUnlock (CurTheme->hScatterFile);
						AddToListFile (iref,pStreetData);  
					} 
					else
					{
						GSSiGlobUlFree (&CurTheme->hScatterFile);
				   		{
				   			if (ColorSet)
					   			goto RtnProcessed;
				   			else
								goto RtnNotProcessed; 
						}
					}
					if (!pStreetData->ShowAllElements && ipos >= 0 && !PrimeNameOnly)
					{
						ipos+=iposinc; 
						if (ipos >= 0 && ipos < 4)
							goto NextSNUM;
					}
				}
			}
	   		{
	   			if (ColorSet)
		   			goto RtnProcessed;
	   			else
					goto RtnNotProcessed; 
			}
		}
			break; 
		case GF_POINT_IN_AREA_THEME: 
			ii=1;         
		case GF_HOTSPOT_THEME:
		case GF_OFFSETAREA_THEME:
		case GF_TIME_DISPLAY_THEME:	
		case GF_SINGLE_VALUE_THEME:
		case GF_AREA_IN_MASK_THEME:
			if (CurTheme->SymNum == -1)
			{
				if (_fstricmp (TAG,&CurTheme->Contents[1])) 
					goto RtnNotProcessed;

			}
			else if (CurTheme->hVisList)
			{   
				LPVISLIST	SaveVis=CurVis; 
				BOOL		WantDesc;
				
				CurVis = (LPVISLIST)GlobalLock (CurTheme->hVisList);
				WantDesc = GetVisibility (desc);
				GlobalUnlock (CurTheme->hVisList);
				CurVis = SaveVis;
				if (!WantDesc)
					goto RtnNotProcessed;
			} 
			if (CurTheme->ID == GF_AREA_IN_MASK_THEME)
			{
				if (!ThemeCreateAreaInMask(FALSE))
					goto RtnNoDisplay;
				break;
			}
			if (CurTheme->ID == GF_HOTSPOT_THEME) 
			{
				switch (CurTheme->HotSpotData.PassThrough)
				{
					case 1:
						goto RtnThemesOnly;
					case 0:
						goto RtnProcessed;
					case 2:
						goto RtnNotProcessed;
				}
				break;
            }
			AdjustBlue = 253;
			CreateThemePens (CurTheme,FALSE); 
			switch (CurTheme->DataType)
			{
				case 0: //area
					switch (CurrentType)
					{
						case GF_TEXT:
						case GF_LINE:
						case GF_CURVE:
						case GF_POLYLINE:
						case GF_POINT:
							goto RtnNotProcessed;
						break;
					} 
					break;
				case 1: //point
					if (CurrentType != GF_POINT && CurrentType != GF_TEXT)
						goto RtnNotProcessed;
					break;
				case 2: //line
					if (CurrentType != GF_LINE && CurrentType != GF_POLYLINE && CurrentType != GF_CURVE)
						goto RtnNotProcessed;
					break;
				case 3: //text  
					if (CurrentType == GF_AREA)
						goto RtnNotProcessed;
					break;
			}
			if (CurTheme->ID == GF_POINT_IN_AREA_THEME && CurTheme->Pass == 0)
			{   
				if (CurTheme->FidAreas == HFILE_ERROR)
					goto RtnNotProcessed; 
				BigWrite (CurTheme->FidAreas,(HPSTR)&iref,4,-1);
				BigWrite (CurTheme->FidAreas,(HPSTR)&nCurPoints,4,-1);
				BigWrite (CurTheme->FidAreas,(HPSTR)lpDCurPoints,nCurPoints * (long)sizeof(DPOINT),-1);  
				CurTheme->NumAreas++;
				switch (CurrentType)
				{
				case GF_AREA:
						SelectObject (CurView->hDC,CurTheme->PointInAreaBrush);
						break;
				}
				//SelectObject (CurView->hDC,CurTheme->PointInAreaPen);  
				goto RtnProcessed;
			}
				
			if (CurTheme->ID == GF_POINT_IN_AREA_THEME && CurTheme->Pass == 0)
			{   
				
				if (!CurTheme->PointInAreaBrush)
				{
					short		ColorInc=1, maxattempts=256, iattempt=0;
					COLORREF	TestColor, SaveColor, GotColor;   
					POINT		pt=RectMid (&CurView->DrawRect);   
					int			r,g,b;
					
					SaveDC (CurView->hDC); 
					SetDisplayMode (CurView->hDC, GF_SCREENMODE); 
				    /*SetMapMode    (CurView->hDC, MM_TEXT );
				    SetWindowOrgEx  ( CurView->hDC, 0,   0,0 );
				    SetViewportOrgEx( CurView->hDC, 0, 0,0 );*/ 
			        SelectClipRgn (CurView->hDC,0);
					SaveColor = GetPixel (CurView->hDC,pt.x,pt.y);
					if (CurView->BackGroundColor)
						ColorInc = -1;
					r=GetRValue (CurView->BackGroundColor);
					g=GetGValue (CurView->BackGroundColor);
					b=GetBValue (CurView->BackGroundColor);  
					r+=ColorInc;
					g+=ColorInc;
					b+=ColorInc;
					TestColor = RGB(r,g,b);  
					SetPixel (CurView->hDC,pt.x,pt.y,TestColor);
					while (iattempt++ < maxattempts && (GotColor=GetPixel (CurView->hDC,pt.x,pt.y)) == CurView->BackGroundColor)
					{ 
						r+=ColorInc;
						g+=ColorInc;
						b+=ColorInc;
						TestColor = RGB(r,g,b);  
						SetPixel (CurView->hDC,pt.x,pt.y,TestColor);
					} 
					GSSiDeleteObject (&CurTheme->PointInAreaBrush);
					GSSiDeleteObject (&CurTheme->PointInAreaPen);
					CurTheme->PointInAreaBrush = CreateSolidBrush (TestColor);
					CurTheme->PointInAreaPen = CreatePen (PS_SOLID,1,TestColor); 
					CurTheme->PointInAreaColor = TestColor; 
					RestoreDC (CurView->hDC,-1);     
				}
				if (CurrentType != GF_AREA || CurTheme->FidAreas == HFILE_ERROR)
					goto RtnNotProcessed; 
				BigWrite (CurTheme->FidAreas,(HPSTR)&iref,4,-1);
				BigWrite (CurTheme->FidAreas,(HPSTR)&nCurPoints,4,-1);
				BigWrite (CurTheme->FidAreas,(HPSTR)lpDCurPoints,nCurPoints * (long)sizeof(DPOINT),-1);  
				CurTheme->NumAreas++;
				SelectObject (CurView->hDC,CurTheme->PointInAreaBrush);
				//SelectObject (CurView->hDC,CurTheme->PointInAreaPen);  
				goto RtnProcessed;
			}
					
			SwitchThemeSHPFile ();		
			ResetFileChangeTime (CurTheme->hThemeDB);
			InClass=FALSE;
NextVal:
			ValD = GetNumericFieldData (CurTheme->hThemeDB,
										&CurTheme->Field,CurTheme->FieldFun,CurTheme->MultiValOption,CurTheme->Value,iref,Type,&status,CurTheme->DataFileID);
			if (status == 1 && CurTheme->MultiValOption > 6 && InClass)
			{
				iclass = HaveClass;
				ValD = HaveValD;
				TrueValD = HaveTrueValD;
				goto SetClassChar;
			}
			if (status == 1 && CurTheme->MissOpt == 4)
			{
				status = 0;
				ValD = 0;
			}
			if (status==1 || (CurTheme->ZeroIsMissing && ValD == 0))
			{   
ProcessMissing: 
				CurTheme->NumMissing++;
				switch (CurTheme->MissOpt)
				{
					case 0:  
						if (CurTheme->NoDataBrush && !CurTheme->NotSetColor)
						{
							SetBkMode (CurView->hDC,OPAQUE);
							SetTextColor (CurView->hDC,0);
							SelectObject (CurView->hDC,CurTheme->NoDataBrush); 
							HaveVarFillColor = TRUE;  
							GlobalColors[0] = RGB(255,0,0);
						}
						else
							ii=1;
						if (DispersePoint (iref,-1,desc,&ValD) == 2)
							goto RtnNoDisplay; 
						else
							goto RtnProcessed;
						break;    
					case 1:
						goto RtnNoDisplay;
						break;
					case 2:
					case 3:
						if (DispersePoint (iref,-1,desc,&ValD) == 2)
							goto RtnNoDisplay; 
						else
							goto RtnNotProcessed;
						break;
				}
				goto RtnNotProcessed;
			}
			if (status==-1)
			{
InvalidSV1:     
				CurTheme->NumInvalid++;
				if (CurTheme->SkipInvalid) 
					goto RtnNoDisplay;
				if (!CurTheme->MarkInvalid) 
				{
					if (CurTheme->NotSetColor)
						goto RtnProcessed;
					else
						goto RtnNoDisplay;// changed 10/13/2009 for Mpls neighborhood monthly map
				//	goto RtnNoDisplay;
				}
				if (CurTheme->InvalidDataBrush)
					SelectObject (CurView->hDC,CurTheme->InvalidDataBrush);
				if (DispersePoint (iref,-2,desc,&ValD) == 2)
					goto RtnNoDisplay;
				else
					goto RtnProcessed;
			}   
			if (CurTheme->MissOpt==3)
				goto RtnNoDisplay;
			if (ValD && CurTheme->FieldCorrection)
			{   
				double	Area,Dist;
				
				switch (CurTheme->DataType)
				{
					case 0:
						if (HiPrecis)
							Area = ComputeProjectedAreaAreaD (lpDCurPoints,nCurPoints,&Dist); 
						else 
							Area = ComputeProjectedAreaArea (lpCurPoints,nCurPoints,&Dist);  
						Area = ConvertArea (Area,CurTheme->FieldCorrection); 
						if (!Area)
							goto InvalidSV1;
						ValD /= Area;
					break;
					case 1:
					break;
					case 2:
						if (HiPrecis)
							ComputeProjectedAreaAreaD (lpDCurPoints,-nCurPoints,&Dist); 
						else 
							ComputeProjectedAreaArea (lpCurPoints,-nCurPoints,&Dist);  
						Dist = ConvertDist (Dist,CurTheme->FieldCorrection); 
						if (!Dist)
							goto InvalidSV1;
						ValD /= Dist;
					break;
				}
			}
			TrueValD = ValD;
			ValD = Round (ValD,CurTheme->RoundTo);
			
			if (ValD > CurTheme->YLimit) ValD = CurTheme->YLimit;
			ftoa (CurTheme->CurValue,ValD);
			for (iclass=0;iclass<CurTheme->NumClass;iclass++)
			{
				if (!CurTheme->ClassStatus[iclass])
				{
					GetClassMinMax (iclass,&ClassMin,&ClassMax);
									
					if (ValD>=ClassMin && ValD<=ClassMax)
					{   
						switch (CurTheme->MultiValOption)
						{
						case 7: //lowest class
							if (!InClass || iclass < HaveClass)
							{
								HaveClass = iclass;
								HaveValD = ValD;
								HaveTrueValD = TrueValD;
					    		InClass=TRUE;
							}
							goto NextVal;
						case 8:
							if (!InClass || iclass > HaveClass)
							{
								HaveClass = iclass;
								HaveValD = ValD;
								HaveTrueValD = TrueValD;
					    		InClass=TRUE;
							}
							goto NextVal;
						}
			    		InClass=TRUE;
						HaveValD = ValD;
						HaveClass = iclass;
			    		break;
					}
				}
			}
SetClassChar:
			if (InClass)
			{
				if (!CurTheme->PCTByArea)
					CurTheme->ClassCount[iclass]++;
				SetThemeElementCharacteristics (iclass);
				if (DispersePoint (iref,iclass,desc,&ValD) == 2)
					goto RtnNoDisplay;  
			}
			if (CurTheme->ShowValue)
			{   
				float	MidPointAZ;
				float	Length;    
				MNMXCORL	MinMax;
				DPOINT	BasePt;
				POINT	WinPoint;
				short	n=0, Height;
				int		Just=0;
				
				if (CurrentMidPoint (Type,Just,&MidPointAZ,&Length,&Height,TRUE, &MinMax,&WinPoint,&ShowVal.WPoint,&n,1,0,0)) 
				{   
					HANDLE hStr = GSSiGlobAlloc (0,GMEM_MOVEABLE,1024);
					LPSTR	str=GlobalLock (hStr);

					ShowVal.pTheme = CurTheme;
					ShowVal.Point = WinPoint; 
					ShowVal.Type = CurTheme->DataType;
					if (CurTheme->DataType == 2)	
						ShowVal.AZ = MidPointAZ;			
					else if (CurTheme->ShowValAZ)
						ShowVal.AZ = CurTheme->ShowValAZ; 
					else
						ShowVal.AZ = -LTWOPI(CurView->Rotation);
					SetShowValPoly (iref,FALSE);
					strcpy (str,ValueConv (TrueValD,CurTheme->ValConv,min(1,CurTheme->RoundTo),CurTheme->AddCommas));
					if (!*ShowVal.Text)
						strcpy (ShowVal.Text,str);
					else
						ExpandText (ShowVal.Text);
					GSSiGlobUlFree (&hStr);
				}
			}
			if (!InClass)
				goto InvalidSV1;
			goto RtnProcessed;
			break;

		case GF_CONNECTION_LINE_THEME:
		case GF_SINGLE_NONNUM_VALUE_THEME:
			if (CurTheme->LayerID && (LayerID != CurTheme->LayerID))
				goto RtnNotProcessed;
			if (desc == 376)
				ii = 1;
			if (CurTheme->SymNum == -1)
			{
				if (_fstricmp (TAG,&CurTheme->Contents[1]))
				{
					if (CurTheme == ComputePCTTheme)
						goto RtnNoDisplay;
					else
						goto RtnNotProcessed;
				}
			}
			else if (CurTheme->hVisList)
			{   
				LPVISLIST	SaveVis=CurVis; 
				BOOL		WantDesc;
				
				CurVis = (LPVISLIST)GlobalLock (CurTheme->hVisList);
				WantDesc = GetVisibility (desc);
				GlobalUnlock (CurTheme->hVisList);
				CurVis = SaveVis;
				if (!WantDesc)
				{
					if (CurTheme == ComputePCTTheme)
						goto RtnNoDisplay;
					else
						goto RtnNotProcessed;
				}
			}
			if (CurTheme->UseFirstSymbol && !SymbolIsVisible(desc))
				goto RtnNotProcessed;
			AdjustBlue = 253;
			CreateThemePens (CurTheme,FALSE); 
			switch (CurTheme->DataType)
			{
				case 0: //area
					switch (CurrentType)
					{
						case GF_TEXT:
						case GF_LINE:
						case GF_CURVE:
						case GF_POLYLINE:
						case GF_POINT:
							goto RtnNotProcessed;
						break;
					} 
					break;
				case 1: //point
					if (CurrentType != GF_POINT && CurrentType != GF_TEXT)
						goto RtnNotProcessed;
					break;
				case 2: //line
					if (CurrentType != GF_LINE && CurrentType != GF_POLYLINE && CurrentType != GF_CURVE)
						goto RtnNotProcessed;
					break;
				case 3: //text  
					if (CurrentType == GF_AREA)
						goto RtnNotProcessed;
					break;
			}

			if (CurTheme->MultiValOption == 4)
				lnKey = 128;
			else
				lnKey = GetBTKeyLen(CurTheme->hScatterFile); 
			ResetFileChangeTime (CurTheme->hThemeDB);
			AtEndOfValues = FALSE;
			if (CurTheme->MultiValOption < 4 &&
				((CurTheme->MultiValOption && !StartAutoClassDef ()) || CurTheme->MultiValOption == 3))
			{  
				
				if (CurTheme->MultiValOption == 2 || CurTheme->MultiValOption == 3)
					MinClass = -1;
NextValue:		
				SwitchThemeSHPFile ();		
				status = GetCharFieldData (CurTheme->hThemeDB,
								  &CurTheme->Field,iref,CurTheme->FieldFun,CurTheme->Value,Value,sizeof(Value),CurTheme->DataFileID,CurTheme->MultiValOption); 
CheckStatus:
				if (status)
				{   
					SetVarChangeTimes (1);
					if (MinClass > MAX_THEME_CLASSES)
						goto ProcessMissing;
					_fstrcpy (Value,MinClassValue);
					AtEndOfValues = TRUE;
					if (CurTheme->MultiValOption != 3)
						status = 0;
				}
				else
				{
					ClassNo = 0; 
					_fstrncpy (KeyVal,Value,lnKey);
					KeyVal[lnKey]=0;
        			if (!BT_FIND (CurTheme->hScatterFile,KeyVal,BT_FIRST,BT_EQ,(LPSTR)&ClassNo))
					{
						if (ClassNo < MinClass && CurTheme->MultiValOption < 2)   
						{
							strncpy0 (MinClassValue,Value,sizeof(MinClassValue)-1);
							MinClass = ClassNo;
						}
						else if (ClassNo > MinClass && CurTheme->MultiValOption == 2)   
						{
							strncpy0(MinClassValue, Value, sizeof(MinClassValue)-1);
							MinClass = ClassNo;
						}
						else if (CurTheme->MultiValOption == 3)
						{
							CurTheme->ClassCount[ClassNo-1]++;
							if (ClassNo > MinClass)
							{
								strncpy0(MinClassValue, Value, sizeof(MinClassValue)-1);
								MinClass = ClassNo;
							}
						}
					}
					else if (StartAutoClassDef ())
					{   
						if (CurTheme->NumClass < CurTheme->NumDesiredClass) 
						{   
							_fstrcpy (CurTheme->ClassBM[CurTheme->NumClass++],KeyVal);
							ClassNo = CurTheme->NumClass; 
						}
						else
						{
							ClassNo = CurTheme->NumDesiredClass;
							_fstrcpy (CurTheme->ClassBM[CurTheme->NumClass-1],"All other values");
						}
						BT_PUT (CurTheme->hScatterFile,KeyVal,(LPSTR)&ClassNo);
						CurTheme->ValueColor = ClassNo;   
						if (CurTheme->MultiValOption == 3)
						{
							CurTheme->ClassCount[ClassNo-1]++;
							if (ClassNo > MinClass)
							{
								strncpy0(MinClassValue, Value, sizeof(MinClassValue)-1);
								MinClass = ClassNo;
							}
						}
					}
					else if (CurTheme->AllValueClass && CurTheme->AllValueClass < CurTheme->NumClass+1)
					{
						strncpy0(MinClassValue, Value, sizeof(MinClassValue)-1);
						MinClass = ClassNo;
					}
					goto NextValue;
			/*	this seems to skip a record
				if (FetchDBRec (CurTheme->hThemeDB))
						goto NextValue;
					else
					{
						status = 1;
						goto CheckStatus;
					}*/
				}
            }
			else
			{	
KeepLooking:
				SwitchThemeSHPFile ();		
				status = GetCharFieldData (CurTheme->hThemeDB,
					&CurTheme->Field,iref,CurTheme->FieldFun,CurTheme->Value,Value,sizeof(Value),CurTheme->DataFileID,CurTheme->MultiValOption);
			} 
			if (status==1)
				goto ProcessMissing; 
			
			if (status==-1)//dont think this can happen 7/21/04
			{   
				if (CurTheme->InvalidDataBrush)
					SelectObject (CurView->hDC,CurTheme->InvalidDataBrush);
				if (DispersePoint (iref,-2,desc,Value) == 2)
					goto RtnNoDisplay;
				else
					goto RtnProcessed;
			}
			if (CurTheme->ShowValue)
			{   
				float	MidPointAZ;
				float  Length; 
				MNMXCORL	MinMax;  
				DPOINT	BasePt;
				POINT	WinPoint;
				short	n=0, Height;
				double	Just = 0;
				LPSTR	pVB;
				
				if ((pVB = strrchr (Value,'|')))
				{
					*pVB++ = 0;
					Just = atof (pVB);
				}
				if (CurrentMidPoint (Type,Just,&MidPointAZ,&Length,&Height,TRUE, &MinMax,&WinPoint,&ShowVal.WPoint,&n,3,0,0)) 
				{    
					ShowVal.pTheme = CurTheme;
					ShowVal.Point = WinPoint; 
					ShowVal.Type = CurTheme->DataType;
					ShowVal.Just = Just;
					if (CurTheme->DataType == 1 || CurTheme->DataType == 2)	
						ShowVal.AZ = MidPointAZ;			
					else if (CurTheme->ShowValAZ)
						ShowVal.AZ = CurTheme->ShowValAZ; 
					else
						ShowVal.AZ = -LTWOPI(CurView->Rotation);
					SetShowValPoly (iref,FALSE);
					if (!*ShowVal.Text)
						strncpy0 (ShowVal.Text, Value, sizeof(ShowVal.Text)-1);
				}
			} 
			strncpy0(CurTheme->CurValue, Value,sizeof(CurTheme->CurValue)-1);
			_fstrncpy (KeyVal,Value,lnKey);
			KeyVal[lnKey]=0;
			ClassNo = 0;
			/*if (CurTheme->DisplayViewport == 19)
			{
				BOOL rtn = ItemProcessedByTheme ("",376);
				ii = rtn;
			}*/
			if (!BT_FIND (CurTheme->hScatterFile,KeyVal,BT_FIRST,BT_EQ,(LPSTR)&ClassNo))
			{   
				if (!CurTheme->NumDesiredClass)
				{
					if (ClassNo < 0)
						ii=1;
					if (desc == 376)
						ii = 1;
					CurTheme->ValueColor = ClassNo;
					AdjustBlue = 256;
					ClassNo = 1;
					CompareDC = CurTheme->CompareDC;
				}
				else if (!ClassNo || ClassNo > MAX_THEME_CLASSES || ClassNo > CurTheme->NumClass)
					goto InvalidSV1;
	GotClass:
				if (CurTheme->ClassStatus[ClassNo-1] && !AtEndOfValues)
					goto NextValue;  
				if (!CurTheme->PCTByArea && !CurTheme->UseStoredCounts)
		    		CurTheme->ClassCount[ClassNo-1]++;
		    	if (CurTheme->UseFirstSymbol && !CurTheme->ClassSymbol[ClassNo-1])
		    		CurTheme->ClassSymbol[ClassNo-1] = desc;
				SetThemeElementCharacteristics (ClassNo-1);
				if (DispersePoint (iref,ClassNo-1,desc,Value) == 2)
					goto RtnNoDisplay;
				if (CurTheme->ShowDirection)
					DisplayThemeDirection ();
			} 
			else if (StartAutoClassDef ())
			{   
				if (!CurTheme->NumDesiredClass)
				{
					if (!CurTheme->FieldFun && !stricmp (CurTheme->Field.name,"SYMNUM"))
					{
						if (SymbolNumberColorType != CurrentLakeType)
						{
							SymbolNumberColor = RGB(0,128,253 + CurrentLakeType);
							SymbolNumberColorType = CurrentLakeType;
							UseSymnumColor = TRUE;
						}
						if (atoi (KeyVal) == 376)
							ii=1;
						ClassNo = SymbolNumberColor + atoi (KeyVal);
						AdjustBlue = 256;
					}
					else
						ClassNo = GetNextUniqueValueColor (&CurTheme->NextValueColor,CurTheme->TargetViewport);
					CompareDC = CurTheme->CompareDC;
				}
				else if (CurTheme->NumClass < CurTheme->NumDesiredClass) 
				{   
					_fstrcpy (CurTheme->ClassBM[CurTheme->NumClass++],KeyVal);
					ClassNo = CurTheme->NumClass; 
				}
				else
				{
					ClassNo = CurTheme->NumDesiredClass;
					_fstrcpy (CurTheme->ClassBM[CurTheme->NumClass-1],"All other values");
				}
				BT_PUT (CurTheme->hScatterFile,KeyVal,(LPSTR)&ClassNo);
				CurTheme->ValueColor = ClassNo;   
				if (!CurTheme->NumDesiredClass)
					ClassNo = 1;
				goto GotClass;
			}
			else
			{
				_fstrncpy (VarVal,"[",lnKey);  
				st = BT_FIND (CurTheme->hScatterFile,VarVal,BT_FIRST,BT_GE,(LPSTR)&ClassNo2);
				while (!st)
				{
					if (*VarVal == '[')
					{   
						ExpandText (VarVal);
						if (!_fstrcmp (VarVal,Value))
						{
							ClassNo = ClassNo2;
							goto GotClass;
						}
					}
					else
						st = 1;
					st = BT_FIND (CurTheme->hScatterFile,VarVal,BT_NEXT,BT_ANY,(LPSTR)&ClassNo2);
				}
			}
			if (!ClassNo)
			{
				if (CurTheme->AllValueClass && CurTheme->AllValueClass < CurTheme->NumClass+1)
				{
					ClassNo = CurTheme->AllValueClass;
					goto GotClass;
				}
				if (!CurTheme->MultiValOption && CurTheme->DataFileType != SHAPE_DATAFILE)
					goto KeepLooking;
				if (CurTheme->SkipInvalid)
					goto RtnNoDisplay;
				goto ProcessMissing; 
			}
			CurTheme->NumVals++;
			if (CurTheme->MissOpt==3)
				goto RtnNoDisplay;
			goto RtnProcessed;
			break;    		

		case GF_TWO_VALUE_THEME:
			goto RtnNotProcessed;
			break;

		case GF_CRIME_THEME:
			goto RtnNotProcessed;
			break;

		case GF_DOCUMENTS_THEME: 
			if (CurTheme->SymNum == -1)
			{
				if (_fstricmp (TAG,&CurTheme->Contents[1]))
				{
{
#if ENABLETRACE
GSSiExitProg (1262);
#endif
					return (-1);
}
				}
			}
			else if (CurTheme->SymNum)
			{
				if (desc != CurTheme->SymNum)
					goto RtnNotProcessed;
			}
			if (HaveDesiredDocType (TAG, UDI))
			{
				SetLineHighlight(CurView->hDC,0);
				SetAreaHighlight(CurView->hDC);
				goto RtnProcessed;
			}
			else
				goto RtnNotProcessed;

		case GF_MOVE_POLY_THEME:
			MovePolyLine(CurTheme->ClassPnt[0]);
			break;

		case GF_SNAP_POLY_THEME:
{
#if ENABLETRACE
GSSiExitProg (1262);
#endif
			return (SnapPolyLine());
}
			break;

		case GF_SAVEPOLY_THEME:
{
#if ENABLETRACE
GSSiExitProg (1262);
#endif
			return (SavePolyLine(1));
}
			break;

		case GF_SAVEPOLYFILE_THEME:
{
#if ENABLETRACE
GSSiExitProg (1262);
#endif
			return (SavePolyLine(2));
}
			break;

		case GF_SAVEPOLYPARTS_THEME:
{
#if ENABLETRACE
GSSiExitProg (1262);
#endif
			return (SavePolyLine(3));
}
			break;

		case GF_UNDRAW_THEME:
			SelectObject (CurView->hDC,GetStockObject(WHITE_PEN));
			goto RtnProcessed;
			break; 
			
        case GF_CONTEST_THEME: 
        {
			ATDATA		AtData;
			
			if (!BT_FIND (hBTNetCon,(LPSTR)&iref,BT_FIRST,BT_EQ,(LPSTR)&AtData))
			{
				if (AtData.NextRef)
					SelectObject (CurView->hDC,CurTheme->ClassPen[0]); 
				else
					SelectObject (CurView->hDC,CurTheme->ClassPen[1]);
				goto RtnProcessed;
			} 
			goto RtnNotProcessed;
        }
        	break;
        	
		case GF_NETMARKER_THEME: 
		{   short	pos; 
			double	MinMP, MaxMP;
            
            if (CurrentType != GF_POLYLINE && CurrentType != GF_LINE) break; 
            if (!CurTheme->hScatterFile)
				goto RtnNotProcessed;
			NetLinksKey.Ref = iref;
			NetLinksKey.NetID = NetworkID;  
			NetLinksKey.Path = 0;  
			rc = -1;
			st = BT_FIND (hBTNetLinks,(LPSTR)&NetLinksKey,BT_FIRST,BT_GE,(LPSTR)&NetLinksData);
			while (!st && NetLinksKey.Ref == iref && NetLinksKey.NetID == NetworkID)
			{   
				rc = 1; 
				NetLinksData.Length = fabs (NetLinksData.Length); 
				MinMP = NetLinksData.MP;
				MaxMP = MinMP + NetLinksData.Length;
				CurStreet = NetLinksKey.Path;
				NetMarkersThemeKey2.Path = NetLinksKey.Path;
				NetMarkersThemeKey2.StartMP = NetLinksData.MP;
				st2 = BT_FIND (CurTheme->hScatterFile,(LPSTR)&NetMarkersThemeKey2,BT_FIRST,BT_GE,
								(LPSTR)&NextEndMP);
				if (st2)
					pos = BT_LAST;
				else
					pos = BT_PRIOR;
				st1 = BT_FIND (CurTheme->hScatterFile,(LPSTR)&NetMarkersThemeKey1,pos,BT_ANY,
								(LPSTR)&LastEndMP);
				if (NetMarkersThemeKey1.Path != NetLinksKey.Path) st1 = 1; 
				if (NetMarkersThemeKey2.Path != NetLinksKey.Path) st2 = 1;   
		SetCase:
				if (st1 && st2)
					Case = 1;
				else 
				{
					if (st1) 
					{ 
						if (fabs ((NetLinksData.MP + NetLinksData.Length)
							- NetMarkersThemeKey2.StartMP) <= NetTOL*2)
							Case = 3;
						else
							Case = 1;
					} 
					else if (NetMarkersThemeKey1.StartMP <= MinMP && LastEndMP >= MaxMP)
						Case = 5;
					else if (st2)
					{  
						if (fabs (NetLinksData.MP - LastEndMP) <= NetTOL*2)
							Case = 2;
						else
							Case = 1;
					
					}
					else
					{  
						if (fabs (NetLinksData.MP - LastEndMP) > NetTOL*2)
							st1 = 1;  
						if (fabs ((NetLinksData.MP + NetLinksData.Length)
							- NetMarkersThemeKey2.StartMP) > NetTOL*2) 
							st2 = 1;
						if (st1 || st2)
							goto SetCase; 
						else
							Case = 4;
					}
				}	
				NetMarkersThemeKey1.Path = NetLinksKey.Path;
				switch (Case)
				{
					case 1:	// insert without connection    
						NetMarkersThemeKey1.Path = NetLinksKey.Path;
						NetMarkersThemeKey1.StartMP = NetLinksData.MP;  
						EndMP = NetLinksData.MP + NetLinksData.Length;
						BT_PUT (CurTheme->hScatterFile,(LPSTR)&NetMarkersThemeKey1,(LPSTR)&EndMP);
						break;
					case 2:	// connects to prior seg
						EndMP = NetLinksData.MP + NetLinksData.Length;
						BT_PUT (CurTheme->hScatterFile,(LPSTR)&NetMarkersThemeKey1,(LPSTR)&EndMP);
						break;
					case 3:	// connects to next seg
						if (BT_DELETE (CurTheme->hScatterFile,(LPSTR)&NetMarkersThemeKey2,(LPSTR)&NextEndMP,FALSE))
							ii=1;
						NetMarkersThemeKey1.StartMP = NetLinksData.MP;
						BT_PUT (CurTheme->hScatterFile,(LPSTR)&NetMarkersThemeKey1,(LPSTR)&NextEndMP);
						break;
					case 4:	// links prior and next seg
						if (BT_DELETE (CurTheme->hScatterFile,(LPSTR)&NetMarkersThemeKey2,(LPSTR)&NextEndMP,FALSE))
							ii=1;
						BT_PUT (CurTheme->hScatterFile,(LPSTR)&NetMarkersThemeKey1,(LPSTR)&NextEndMP);
						break; 
					case 5:
						break;
				}
					
				st = BT_FIND (hBTNetLinks,(LPSTR)&NetLinksKey,BT_NEXT,BT_ANY,(LPSTR)&NetLinksData);
			}
		
		if (rc==1)
			SelectObject(CurView->hDC, CurTheme->ClassPen[0]);

		if (HaltReport)
		{
			CloseThemeDataFile(TRUE);
			CurTheme->IsActive = FALSE; 
		}
{
#if ENABLETRACE
GSSiExitProg (1262);
#endif
		return rc;
}
		}

	}

RtnNotProcessed:
	if (HaltReport)
	{
		CloseThemeDataFile(TRUE);
		CurTheme->IsActive = FALSE; 
	}
{
#if ENABLETRACE
GSSiExitProg (1262);
#endif
	return -1;
}
RtnNoDisplay:
	if (HaltReport)
	{
		CloseThemeDataFile(TRUE);
		CurTheme->IsActive = FALSE; 
	}
{
#if ENABLETRACE
GSSiExitProg (1262);
#endif
	return  0;
}
RtnThemesOnly:
	if (HaltReport)
	{
		CloseThemeDataFile(TRUE);
		CurTheme->IsActive = FALSE; 
	}
{
#if ENABLETRACE
GSSiExitProg (1262);
#endif
	return  3;
}
RtnProcessed:
	if (HaltReport)
	{
		CloseThemeDataFile(TRUE);
		CurTheme->IsActive = FALSE; 
	} 
	else if (CurTheme->NotSetColor)
{
#if ENABLETRACE
GSSiExitProg (1262);
#endif
		return	2;  
}
//	else if (!CurTheme->DataType)
//		HaveVarFillColor = TRUE; 
{
#if ENABLETRACE
GSSiExitProg (1262);
#endif
	return	1;  
}
	
RtnColorNotSet:
	if (HaltReport)
	{
		CloseThemeDataFile(TRUE);
		CurTheme->IsActive = FALSE; 
	} 
//	else if (!CurTheme->DataType)
//		HaveVarFillColor = TRUE; 
{
#if ENABLETRACE
GSSiExitProg (1262);
#endif
	return	1;  
}
	
#if ENABLETRACE
}
#endif
}

BOOL ProcessDataDisplayInput (HWND hWnd,int Message, WPARAM wParam,LPARAM lParam)
{
	char	str[64];
	POINT	Point16;

   	if (CurDataRectID < 0)
   		return FALSE;
/*	GetCursorPos (&Point16);
	ScreenToClient (CurView->hWnd,&Point16);
	CurrentPoint=ScreenPtToBasePt(Point16);*/
    switch (Message)
	{
		case WM_LBUTTONDOWN:
			SetGlobalValue ("%DDACTION","LBDOWN");
			ProcessDataDisplayMacro (CurDataRectID,CurView);
			return TRUE;
		case WM_LBUTTONUP:
			SetGlobalValue ("%DDACTION","LBUP");
			ProcessDataDisplayMacro (CurDataRectID,CurView);
			return TRUE;
		case WM_RBUTTONDOWN:
			SetGlobalValue ("%DDACTION","RBDOWN");
			ProcessDataDisplayMacro (CurDataRectID,CurView);
			return TRUE;
		case WM_RBUTTONUP:
			SetGlobalValue ("%DDACTION","RBUP");
			ProcessDataDisplayMacro (CurDataRectID,CurView);
			return TRUE;
	    case WM_KEYDOWN:
			sprintf (str,"KEY:%c",(char)wParam);    
			SetGlobalValue ("%DDACTION",str);
			ProcessDataDisplayMacro (CurDataRectID,CurView);
			return TRUE;
		case WM_CHAR:
			return TRUE;
	}
	return FALSE;
}

void ProcessDataDisplayMacro (int id,LPVIEWPORT CurView)
{
	LPDATADISPLAYRECT	pDataDisplayRect;
	HFILE	Fid;
	LPVIEWPORT	SaveVP=CurView;

	pDataDisplayRect = GlobalLock (CurView->hDataDisplayRect);
	pDataDisplayRect += id;
	Fid = GSSiOpenFile (CurView->DataDisplayRectFile,0,OF_READ);
	if (Fid != HFILE_ERROR)
	{
		HANDLE	hMacro = GSSiGlobAlloc (1524,GMEM_MOVEABLE,pDataDisplayRect->MacroLen);
		LPSTR	pMacro = GlobalLock (hMacro);

		GSSillseek (Fid,pDataDisplayRect->Offset,0);
		BigRead (Fid,pMacro,pDataDisplayRect->MacroLen);
		GSSiClose2 (&Fid);
		CurrentPoint = pDataDisplayRect->WPoint;
		GlobalUnlock (CurView->hDataDisplayRect);
		ProcessText (pMacro);
		GSSiGlobUlFree (&hMacro);
	}
	else
		GlobalUnlock (SaveVP->hDataDisplayRect);
	CurView = SaveVP;
	return;
}

void InvertDataDisplayRect (int id,LPVIEWPORT CurView,BOOL Enter)
{
	LPDATADISPLAYRECT	pDataDisplayRect;
	RECT	ClientRect;

	if (!CurView->hDataDisplayRect)
		return;
	pDataDisplayRect = GlobalLock (CurView->hDataDisplayRect);
	SaveDC (CurView->hDC);
    SetDisplayMode (CurView->hDC, GF_SCREENMODE);
	SelectClipRgn (CurView->hDC,0);
	ClientRect = pDataDisplayRect[id].Rect;
	ScreenRectToClientRect (CurView->hWnd,&ClientRect);
	InvertRect (CurView->hDC,&ClientRect); 
	if (Enter)
		SetGlobalValue ("%DDACTION","ENTER");
	else
		SetGlobalValue ("%DDACTION","EXIT");
	ProcessDataDisplayMacro (id,CurView);
	RestoreDC (CurView->hDC,-1);
	GlobalUnlock (CurView->hDataDisplayRect);
	return;
}

int PtInDataDisplayRect (POINT Point)
{
	LPDATADISPLAYRECT	pDataDisplayRect;
	int		id=-1;  
	UINT	i;
	
	if (!CurView)
		return -1;
	if (!CurView->nDataDisplayRect)
		return -1;
	pDataDisplayRect = GlobalLock (CurView->hDataDisplayRect);
	for (i=0;i<CurView->nDataDisplayRect;i++)
		if (PtInRect (&pDataDisplayRect[i].Rect,Point))
		{
			id = i; 
			break;
		}
	GlobalUnlock (CurView->hDataDisplayRect);
	return id;
}
 
void RemoveDataDisplayRect (LPVIEWPORT pVP)
{ 
	UINT	ivp;

	if (ConfigLevel)
		return;
	if (pVP)
	{
		if (pVP->nDataDisplayRect)
		{
			GSSiRemoveAndClear (pVP->DataDisplayRectFile);
			GSSiGlobFree (&pVP->hDataDisplayRect);
			pVP->nDataDisplayRect = 0;
		}
	}
	else
	{
		CurDataRectID = -1;
		for (ivp = 0;ivp < *pNumViewports; ivp++)
		{   
			pVP = pViewports[ivp];
			if (pVP->nDataDisplayRect)
			{
				GSSiRemoveAndClear (pVP->DataDisplayRectFile);
				GSSiGlobFree (&pVP->hDataDisplayRect);
				pVP->nDataDisplayRect = 0;
			}
		}
	}
	return;
} 

BOOL AddDataDisplayRect (RECT ScreenRect,DPOINT WPoint,LPSTR InMacro)
{
	LPDATADISPLAYRECT	pDataDisplayRect;
	HFILE	Fid;
	char	Macro[1024];

	if (CurView->nDataDisplayRect >= MAX_DATADISPLAYRECT)
		return FALSE;
	if (!CurView->nDataDisplayRect)
	{
		GSSiGetTempFileName (0,"gmr",0,CurView->DataDisplayRectFile);
		CurView->hDataDisplayRect = GSSiGlobAlloc (1503,GMEM_MOVEABLE,MAX_DATADISPLAYRECT*sizeof(DATADISPLAYRECT));
		Fid = GSSiOpenFile (CurView->DataDisplayRectFile,0,OF_CREATE);
	}
	else
		Fid = GSSiOpenFile (CurView->DataDisplayRectFile,0,OF_READWRITE);
	pDataDisplayRect = GlobalLock (CurView->hDataDisplayRect);
	pDataDisplayRect += CurView->nDataDisplayRect++;
	ClientRectToScreenRect (CurView->hWnd,&ScreenRect);
	pDataDisplayRect->Rect = ScreenRect;
	pDataDisplayRect->WPoint = WPoint;
	pDataDisplayRect->Offset = GSSillseek (Fid,0,2);
	strcpy (Macro,InMacro);
	ExpandText (Macro);
	pDataDisplayRect->MacroLen = strlen (Macro) + 1;
	BigWrite (Fid,Macro,pDataDisplayRect->MacroLen,-1);
	GSSiClose2 (&Fid);
	GlobalUnlock (CurView->hDataDisplayRect);
	return TRUE;
}

BOOL PickDataDisplayRectangles (DPOINT PickPointBase,int PickAp,LPDOUBLE pNearDist)
{   
	LPPROFILEDATARECTANGLE	pProfileDataRectangle;
	BOOL	rtn=FALSE;
	POINT	PickPoint;     
	short	i;
	
	if (!CurView->nProfileDataRectangles)
		return FALSE;
	PickPoint = BasePtToWinPt (&PickPointBase);	
	pProfileDataRectangle = (LPPROFILEDATARECTANGLE)GlobalLock (CurView->hProfileDataRectangles);
	for (i=0;i<CurView->nProfileDataRectangles;i++,pProfileDataRectangle++)
	{ 
		if (POINT_IN_AREA (PickPoint,4, pProfileDataRectangle->Points))
		{
	   		HANDLE hDB = OpenProfileDataFile (0);     
	   		if (hDB)
	   		{
				LPGWDHEADER	lpGWDHead; 
				LPPROFILEFILEDATA	pData;

			    lpGWDHead = (LPGWDHEADER)GlobalLock (hDB); 
				pData = (LPPROFILEFILEDATA)&lpGWDHead->GWDData; 
				FillGWDData (lpGWDHead,pProfileDataRectangle->Offset);  
				if (CurView->pTheme)
				{   
					LPVIEWPORT	SaveVP=CurView;
					
					SetViewport (CurView->pTheme->TargetViewport);
					rtn = TRUE;  
					PickByRefno (pData->Ref,NULL,NULL,-100);
					CurView = SaveVP;    
				}
				GlobalUnlock (hDB); 
				CloseGWDatabase (hDB);
			}
			break;
		}
	}
	GlobalUnlock (CurView->hProfileDataRectangles);	
	return rtn;
}
 
void ShowValue (HDC hDC, BOOL Init)
#if ENABLETRACE
{GSSiEnterProg (1275);
#endif
{   
	float	sizex = ShowValSize;// * DeviceToScreenFactor();
	short	VJust=2, HJust,Weight=100;    
	static	BOOL	First=TRUE;   
	DWORD	TextExt;  
	COLORREF	OldColor,ShadowColor=RGB(255,255,255); 
	BOOL	Shadow=0; 
	HANDLE	hSaveFont=0; 
	LPSTR	pSaveFont,pBar=0;
	short	ii; 
	static	double	FontPixelsToSizeFactor=1;
	static	DPOINT	MaskPoints[4]={0,0,0,0};  
	DPOINT	TextPoints[4];    
	MNMXCORD	DrawBounds;
   	HANDLE	hAccelerator=0;
   	double	TtoAfac; 
   	char	str[64];
	double	SavePIASizeFactor = PIASizeFactor; 
	LPTHEME	SaveTheme=CurTheme;  
	BOOL	UseTestRect=TRUE;
	static	long	debugref=50500067;
	RECT	TextRect;
	int		nMoves, MaxMoves=10;
	
	if (Init) 
	{
		if (First) 
		{
			ShowVal.hPoints = 0;   
			ShowVal.hPolyPartLen = 0;
		}
		else
			GSSiGlobFree (&ShowVal.hPoints); 
		First = FALSE;
		ShowVal.Text[0]=0;
{
#if ENABLETRACE
GSSiExitProg (1275);
#endif
		return;   
}
	}
	if (!*pNumViewports)
	{
#if ENABLETRACE
		GSSiExitProg(1275);
#endif
		return;
	}
	Truncate(ShowVal.Text);
	if (!ShowVal.Text[0])
{
#if ENABLETRACE
GSSiExitProg (1275);
#endif
		return;   
}   
	ShowValRef = ShowVal.Refno;  
//	sprintf (str,"%ld - %f",ShowValRef,PIASizeFactor);
//	SetWindowText (hWndMain,str);
	if (ShowValRef == debugref)
		ii=1;
	if (ShowVal.Text[0] && ShowVal.pTheme)
	{   
	    SaveDC (hDC);
	    SetDisplayMode (hDC, GF_SCREENMODE); 
		SetBkMode (hDC,TRANSPARENT);     
		//GSSiDeleteObject(&CurView->hRgn);
		//CurView->hRgn = CreateVPRgn (FALSE,FALSE);
		//SelectClipRgn (CurView->hDC,0);//CurView->hRgn);
		//GSSiDeleteObject(&CurView->hRgn); 
		CurTheme = ShowVal.pTheme;
	    if (*ShowVal.pTheme->ShowValueFont.lfFaceName)
	    { 
			DispText (0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-1,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0);
			hSaveFont = GSSiGlobAlloc (1330,GMEM_MOVEABLE,256);  
			pSaveFont = GlobalLock (hSaveFont);
			GetGlobalCVal ("[%LABELFONT]",pSaveFont,0);
			SetGlobalValue ("%LABELFONT",ShowVal.pTheme->ShowValueFont.lfFaceName);
			OldColor = SetTextColor (hDC,ConvertColor (ShowVal.pTheme->ShowValueTextColor,ShowVal.pTheme->UseHalfTone));  
			sizex = fabs (PixelsToTAGFontHt (&ShowVal.pTheme->ShowValueFont,3));// * DeviceToScreenFactor(); 
			FontPixelsToSizeFactor = sizex/max(1,abs(ShowVal.pTheme->ShowValueFont.lfHeight));  
			Weight = ShowVal.pTheme->ShowValueFont.lfWeight;
			if (ShowVal.pTheme->ShowValueFont.lfUnderline)
				SetBkMode (hDC,OPAQUE); 
			Shadow = ShowVal.pTheme->ShowValueFont.lfStrikeOut;
			if (ShowVal.pTheme->UseShadowColor)
				ShadowColor = ShowVal.pTheme->ShowValueShadowColor;
			if (Shadow)
				Shadow = 2;
		}
		else
			OldColor = SetTextColor (hDC,ConvertColor (ShowValColor,ShowVal.pTheme->UseHalfTone));
		switch (ShowVal.Type)
		{
			case 2:
				VJust = 1;  
			case 1:
/*		{
			RECT	rect;
		    SaveDC (hDC);
		    SetDisplayMode (hDC, GF_TEXTMODE);  
			rect.left = ShowVal.Point.x-3;
			rect.right = ShowVal.Point.x+3;
			rect.top = ShowVal.Point.y-3;
			rect.bottom = ShowVal.Point.y+3;
			FillRectPoly (hDC,&rect,0);
			RestoreDC (hDC,-1);
		} */
			HJust = 2;
			if (ShowVal.Just == -2)
			{
				HJust = 2;
				UseTestRect = 0;
			}
			else if (ShowVal.Just < 0)
			{
				HJust = 0;
				UseTestRect = 2;
			}
			else if (ShowVal.Just > 0)
			{
				HJust = 4;
				UseTestRect = 2;
			}
			nMoves = 0;
TryAgain:
			if (DispText (hDC,FALSE,ShowVal.Point.x,ShowVal.Point.x, ShowVal.Point.y,0, HJust,VJust,
				  		-sizex,1,1,Weight, FALSE,ShowVal.AZ,ShowVal.Text,0,UseTestRect,Shadow,ShadowColor,-1,0,0,0,0,0,ShowVal.pTheme->UseHalfTone,0,0,ShowVal.pTheme,&TextRect,0,0))
			{
				if (*CurTheme->DataDisplayMacro)
					AddDataDisplayRect (TextRect,ShowVal.WPoint,CurTheme->DataDisplayMacro);
			}
			else if (nMoves++ < MaxMoves)
			{
				HPPOINT	pPoints = (HPPOINT)GlobalLock (ShowVal.hPoints);
				double	Pct = (double)nMoves / (double)MaxMoves;
				double	Length = GetPolyLength (pPoints,ShowVal.nPnts);
				DPOINT	PctPoint = PointAtDistOnPoly16 (pPoints,nPnts,Length*Pct,0,0);
				
				ShowVal.Point = DPointToPoint (PctPoint);
				GlobalUnlock (ShowVal.hPoints);
				goto TryAgain;
			}

			break;
			
			case 0:
			{   
				USHORT	twidth, theight;
				UINT	i;
				BOOL	pass=FALSE;
				double	PolyArea, TextArea, AreaAZ=100;
				float   tsize, mintsize=GetGlobalDVal2 ("[%MINTEXTSIZE]",0.06);//* DeviceToScreenFactor(); 
				HPPOINT	pPoints; 
				HPDPOINT	pAreaPoints;
				DPOINT	MidPt;
				POINT	CenterPoint[7];
				float	PointMaxTSize[7];
				short	NumCenterPoints=-1;
				HANDLE	hAreaPoints; 
				MNMXCORD	AreaBounds;  
				LPSTR	txt;
				HANDLE	htxt; 
				BOOL	AllowTextRotation=GetGlobalBVal2 ("[%ALLOWTEXTROTATION]",TRUE);
				HANDLE	hMaskAccelerator=0;  
				long	MaskAreaRefno=0;  
				BOOL	DeleteMaskAccelerator=FALSE;
    
			    if (CurView->DisplayInParent && CurView->Parent)   
			    {
			    	if (pViewports[CurView->Parent-1]->hMaskArea) 
			    	{
			    		hMaskAccelerator = pViewports[CurView->Parent-1]->hMaskAccelerator[0];     
			    		MaskAreaRefno = pViewports[CurView->Parent-1]->MaskAreaRefno; 
			    	}
			    }
			    else if (CurView->hMaskArea) 
		    	{
					if (!CurView->hMaskAccelerator[0])
					{
						LPMNMXCORD lpRect = (LPMNMXCORD) GlobalLock(CurView->hMaskArea); 
						HPDPOINT lpAreaPoints = (HPDPOINT)(lpRect+1);

						if (lpRect)
						{
							CurView->hMaskAccelerator[0] = PointInAreaAcceleratorSetup (CurView->NumMaskPoints,lpAreaPoints,1,0,0);
							GlobalUnlock (CurView->hMaskArea);
						}
					}
		    		hMaskAccelerator = CurView->hMaskAccelerator[0];
		    		MaskAreaRefno = CurView->MaskAreaRefno;
		    	} 
		    	else
		    	{   
		    		/*TextPoints[0].x = CurView->DrawRect.left;
		    		TextPoints[0].y = CurView->DrawRect.bottom;
		    		TextPoints[1].x = CurView->DrawRect.left;
		    		TextPoints[1].y = CurView->DrawRect.top;
		    		TextPoints[2].x = CurView->DrawRect.right;
		    		TextPoints[2].y = CurView->DrawRect.top;
		    		TextPoints[3].x = CurView->DrawRect.right;
		    		TextPoints[3].y = CurView->DrawRect.bottom;*/ 
		    		/*if (MaskPoints[0].x != CurView->WBounds.xmn ||
		    			MaskPoints[0].y != CurView->WBounds.ymn ||
		    			MaskPoints[1].y != CurView->WBounds.ymx ||
		    			MaskPoints[2].x != CurView->WBounds.xmx)
		    			{
				    		MaskPoints[0].x = CurView->WBounds.xmn;
				    		MaskPoints[0].y = CurView->WBounds.ymn;
				    		MaskPoints[1].x = CurView->WBounds.xmn;
				    		MaskPoints[1].y = CurView->WBounds.ymx;
				    		MaskPoints[2].x = CurView->WBounds.xmx;
				    		MaskPoints[2].y = CurView->WBounds.ymx;
				    		MaskPoints[3].x = CurView->WBounds.xmx;
				    		MaskPoints[3].y = CurView->WBounds.ymn; */
		    		if (MaskPoints[0].x != CurView->ScreenRect.left ||
		    			MaskPoints[0].y != CurView->ScreenRect.top ||
		    			MaskPoints[1].y != CurView->ScreenRect.bottom ||
		    			MaskPoints[2].x != CurView->ScreenRect.right)
		    			{
				    		MaskPoints[0].x = CurView->ScreenRect.left;
				    		MaskPoints[0].y = CurView->ScreenRect.top;
				    		MaskPoints[1].x = CurView->ScreenRect.left;
				    		MaskPoints[1].y = CurView->ScreenRect.bottom;
				    		MaskPoints[2].x = CurView->ScreenRect.right;
				    		MaskPoints[2].y = CurView->ScreenRect.bottom;
				    		MaskPoints[3].x = CurView->ScreenRect.right;
				    		MaskPoints[3].y = CurView->ScreenRect.top;
							GSSiGlobFree (&hShowValMaskAccelerator); 
				      		hShowValMaskAccelerator = PointInAreaAcceleratorSetupWindow (4,MaskPoints,0); 
				      	} 
		      		hMaskAccelerator = hShowValMaskAccelerator;
			    } 
		//	hMaskAccelerator = 0;
				if (*ShowValDB)
				{   
					BOOL	Found=FALSE;
					
					if (!hShowValDB)
					     hShowValDB = OpenGWDatabase (ShowValDB,BT_READ);
					if (hShowValDB)
					{
					 	LPGWDHEADER lpGWDHead = (LPGWDHEADER)GlobalLock (hShowValDB);  
					 	long	Offset;
					 	LPSHOWVALDATA	pSVData=(LPSHOWVALDATA)lpGWDHead->GWDData;
					 	
					 	ShowValKey.MaskAreaRefno = MaskAreaRefno; 
					 	ShowValKey.AreaRefno = ShowVal.Refno;
					 	if (!BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&ShowValKey,BT_FIRST,BT_EQ,(LPSTR)&Offset))
					 	{   
					 		POINT	Point;
					 		
					 		Found = TRUE;
					 		FillGWDData (lpGWDHead,Offset);  
					 		Point = BasePtToWinPt (&pSVData->Point);
					 		if (pSVData->Size)
					 		{  
					 			double	MaxPixels = BaseDistToWinDist * pSVData->Size/2;
					 			double	MaxSize = MaxPixels * FontPixelsToSizeFactor;
					 			
				 				sizex = min (sizex,MaxSize);
					 		}
					 		if (sizex > mintsize)
							DispText (hDC,FALSE,Point.x,Point.x, Point.y,0, 2,VJust,
								  		-sizex,1,1,Weight, FALSE,0,ShowVal.Text,0,UseTestRect,Shadow,ShadowColor,-1,0,0,0,0,0,ShowVal.pTheme->UseHalfTone,0,0,ShowVal.pTheme,&TextRect,0,0);
					 	} 
					 	if (Found) 
					 	{
					 		GlobalUnlock (hShowValDB); 
					 		break;  
					 	}
					 	ShowValKey.MaskAreaRefno = 0; 
					 	ShowValKey.AreaRefno = ShowVal.Refno;
					 	if (!BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&ShowValKey,BT_FIRST,BT_EQ,(LPSTR)&Offset))
					 	{   
					 		POINT	Point;
					 		
					 		Found = TRUE;
					 		FillGWDData (lpGWDHead,Offset);  
					 		Point = BasePtToScreenPt (&pSVData->Point);
							DispText (hDC,FALSE,Point.x,Point.x, Point.y,0, 2,VJust,
								  		-sizex,1,1,Weight, FALSE,0,ShowVal.Text,0,UseTestRect,Shadow,ShadowColor,-1,0,0,0,0,0,ShowVal.pTheme->UseHalfTone,0,0,ShowVal.pTheme,&TextRect,0,0);
					 	}
					 	GlobalUnlock (hShowValDB); 
					 	if (Found)
					 		break;
					}
				}
				if (!ShowVal.hPoints)
					break;  
				mintsize = max (mintsize,0.001);
				htxt = GSSiGlobAlloc (1331,GMEM_MOVEABLE,1024);
				txt = GlobalLock (htxt);
				_fstrcpy (txt,ShowVal.Text);
				pPoints = (HPPOINT)GlobalLock (ShowVal.hPoints);
				hAreaPoints = GSSiGlobAlloc (1332,GMEM_MOVEABLE,(long)nPnts*sizeof(DPOINT));
				pAreaPoints=(HPDPOINT)GlobalLock (hAreaPoints); 
				DBoundsInit (&AreaBounds);
				for (i=0;i<nPnts;i++)
				{
					pAreaPoints[i].x = pPoints[i].x;
					pAreaPoints[i].y = pPoints[i].y;  
					AddDPointToMinMax (&pAreaPoints[i],&AreaBounds);
				}
			    GSSiGlobUlFree (&ShowVal.hPoints);
			    RectToBounds (&CurView->ScreenRect,&DrawBounds);  
			    if (!IntersectBounds (&AreaBounds,&DrawBounds,&AreaBounds))
				{
					GlobalUnlock (hAreaPoints);
			    	goto Exit;    
			    }
			    MidPt = MinMaxMidPointD (&AreaBounds);
				PolyArea = fabs (ComputeAreaAreaD (pAreaPoints,ShowVal.nPnts,0)); 
				tsize = sizex;
				TextExt = DispText (hDC,TRUE,(int)MidPt.x,(int)MidPt.x, -1, 0, 2,VJust,
					  		-tsize,1,1,Weight, FALSE,ShowVal.AZ,txt,0,UseTestRect,Shadow,ShadowColor,-1,0,0,0,0,0,ShowVal.pTheme->UseHalfTone,0,0,ShowVal.pTheme,0,0,0);
				twidth = LOWORD(TextExt)+1; 
				theight = HIWORD(TextExt)+1;     
				TextArea = (double)twidth * (double)theight * 1.1; 
				if (PolyArea)
					TtoAfac  =  TextArea / PolyArea; 
				else
					TtoAfac = 1;
			    if (TtoAfac > 1)
			    	tsize /= sqrt (TtoAfac);
				{
					BOOL	TextInView=TRUE;
					PIASizeFactor = GetGlobalDVal2 ("[%SVAFACTOR]",0.55);   

					if (tsize > mintsize && POINT_IN_AREAD (MidPt, nPnts,pAreaPoints,1,0,0,0))  
					{
						if (DispText (hDC,FALSE,(int)MidPt.x,(int)MidPt.x, (int)MidPt.y,0, 2,VJust,
						  		-tsize,1,1,Weight, FALSE,0,txt,0,UseTestRect,Shadow,ShadowColor,-1,nPnts,hAreaPoints,hAccelerator,ShowVal.nPoly,ShowVal.hPolyPartLen,ShowVal.pTheme->UseHalfTone,pBar,0,ShowVal.pTheme,0,0,0)) 
						{
							GlobalUnlock (hAreaPoints);
					    	goto Exit;    
					    } 
					}
					GSSiGlobFree (&hAccelerator); 
		      		hAccelerator = PointInAreaAcceleratorSetupWindow (nPnts,pAreaPoints,hMaskAccelerator);
					if (hAccelerator && hMaskAccelerator)
					{
						LPPIAAStruct pPIAA=(LPPIAAStruct)GlobalLock (hAccelerator);  
						
						AreaBounds = pPIAA->InBounds; 
						if ((TextInView = pPIAA->HaveInPoints))
						{
							BOOL	HaveCP;
							POINT PIAAPoint = PIAACenter (pPIAA,0,0,FALSE,&HaveCP);
							
							if (HaveCP)
							{
								DPOINT PIAAPointD = PIAAPointToDPoint (PIAAPoint,&pPIAA->Bounds,&pPIAA->Factor,pPIAA->Offset,pPIAA->Type); 
							
							//SetPixel (hDC,PIAAPointD.x,PIAAPointD.y,0);
								ShowVal.Point = DPointToPoint (PIAAPointD);
							}
							else
								TextInView = FALSE;
						}
						GlobalUnlock (hAccelerator); 
                    }
					GlobalUnlock (hAreaPoints);
					if (!TextInView)
						goto Exit; 
				    MidPt = MinMaxMidPointD (&AreaBounds);
				    BoundsToPoints (&AreaBounds,TextPoints,0);
			Next:  
					tsize = sizex;
				    if (PolyArea && TextArea)
				    {   
				    	float	OrigTSize = tsize;
				    	short	ip;
				    	
					    if (TtoAfac > 1)
					    	tsize /= TtoAfac;
					    while (tsize >= mintsize)
						{   
							if (NumCenterPoints < 0)
							{   
								DPOINT TestPoint = PointToDPoint (ShowVal.Point);
								double	factor = 2.0;
				                
				                NumCenterPoints = 0;
								pAreaPoints=(HPDPOINT)GlobalLock (hAreaPoints);    
								if (POINT_IN_AREAD (TestPoint, nPnts,pAreaPoints,1,0,0,&hAccelerator)) 
								{
									PointMaxTSize[NumCenterPoints] = OrigTSize;
									CenterPoint[NumCenterPoints++] = DPointToPoint (TestPoint);
								}
								if (POINT_IN_AREAD (MidPt, nPnts,pAreaPoints,1,0,0,&hAccelerator)) 
								{
									PointMaxTSize[NumCenterPoints] = OrigTSize;
									CenterPoint[NumCenterPoints++] = DPointToPoint (MidPt);
								}
								for (ip=0;ip < 4; ip++)
								{    
									BOOL	st;
									
							      	TestPoint = MidPointD (MidPt,TextPoints[ip]);
									if (hAccelerator)
							      		st = (PointInAreaAccelerator (&TestPoint,hAccelerator) == 1);
							      	else
										st = POINT_IN_AREAD (TestPoint, nPnts,pAreaPoints,1,0,0,0);
									if (st)
									{   
										if (TtoAfac * factor > 1)
											PointMaxTSize[NumCenterPoints] = OrigTSize / (TtoAfac * factor);
										else
											PointMaxTSize[NumCenterPoints] = OrigTSize;
										CenterPoint[NumCenterPoints++] = DPointToPoint (TestPoint);
									}
								}
								GlobalUnlock (hAreaPoints); 
							}
							for (ip=0;ip<NumCenterPoints;ip++)
							{		    	
								//if (tsize <= PointMaxTSize[ip])
								{    
									//SetPixel (hDC,CenterPoint[ip].x,CenterPoint[ip].y,0);
									if (DispText (hDC,FALSE,CenterPoint[ip].x,CenterPoint[ip].x, CenterPoint[ip].y,0, 2,VJust,
									  		-tsize,1,1,Weight, FALSE,0,txt,0,UseTestRect,Shadow,ShadowColor,-1,nPnts,hAreaPoints,hAccelerator,ShowVal.nPoly,ShowVal.hPolyPartLen,ShowVal.pTheme->UseHalfTone,pBar,0,ShowVal.pTheme,0,0,0)) 
										goto Exit;
									if (AllowTextRotation)
									{
										if (AreaAZ > TWOPI) 
										{
											HANDLE	hInPoints=GSSiGlobAlloc (1521,GMEM_MOVEABLE,((long)nPnts)*sizeof(DPOINT));
											HPDPOINT	InPoints=(HPDPOINT)GlobalLock (hInPoints);
											HPDPOINT	AreaPoint = (HPDPOINT)GlobalLock (hAreaPoints);
											DWORD	j;
											long	nInPnts=0;	
											
											for (j=0;j<nPnts;j++)
												if (PointInAreaAccelerator (&AreaPoint[j],hAccelerator))	 
													InPoints[nInPnts++] = AreaPoint[j];
											if (nInPnts > 2)
												AreaAZ = GetAreaAZ (nInPnts,hInPoints);
											else
												AreaAZ = 0;
											GSSiGlobUlFree (&hInPoints); 
											GlobalUnlock (hAreaPoints);
										} 
										if (AreaAZ) 
										{
											if (DispText (hDC,FALSE,CenterPoint[ip].x,CenterPoint[ip].x, CenterPoint[ip].y,0, 2,VJust,
												  		-tsize,1,1,Weight, FALSE,AreaAZ,txt,0,UseTestRect,Shadow,ShadowColor,-1,nPnts,hAreaPoints,hAccelerator,ShowVal.nPoly,ShowVal.hPolyPartLen,ShowVal.pTheme->UseHalfTone,pBar,0,ShowVal.pTheme,0,0,0)) 
												goto Exit;
										}
									}
								}
							} 
							tsize -= 0.01;
						}
					} 
					if (!pass)
					{   
						pass = TRUE;
						//mintsize /= 2;
						if (GetGlobalCVal ("[%MINTEXT]",txt,0)) 
						{
							if ((pBar = _fstrchr (txt,'|')))
								*pBar++ = 0; 
							else
								ExpandText (txt); 
							goto Next;
						}  
					}
					else if (hAccelerator)
					{
						LPPIAAStruct pPIAA=(LPPIAAStruct)GlobalLock (hAccelerator); 
						BOOL	HaveCP;
						POINT PIAAPoint = PIAACenter (pPIAA,0,0,FALSE,&HaveCP);
						if (HaveCP)
						{
							DPOINT	PIAAPointD = PIAAPointToDPoint (PIAAPoint,&pPIAA->Bounds,&pPIAA->Factor,pPIAA->Offset,pPIAA->Type); 
									
							ShowVal.Point = DPointToPoint (PIAAPointD);  
							if (PtInRect (&CurView->ScreenRect,ShowVal.Point))
							{
								if (pBar)
								{
									if (GetGlobalCVal ("[%MINTEXT]",txt,0)) 
									{
										if ((pBar = _fstrchr (txt,'|')))  
										{
											txt = pBar+1;
											ExpandText (txt);
										}
									}
								}
								DispText (hDC,FALSE,ShowVal.Point.x,ShowVal.Point.x, ShowVal.Point.y,0, 2,VJust,
							  			-tsize,1,1,Weight, FALSE,0,txt,0,UseTestRect,Shadow,ShadowColor,-1,nPnts,hAreaPoints,hAccelerator,ShowVal.nPoly,ShowVal.hPolyPartLen,ShowVal.pTheme->UseHalfTone,0,0,ShowVal.pTheme,0,0,0); 
							}
						}
						GlobalUnlock (hAccelerator); 
	                }
 
		Exit:   
					GSSiGlobFree (&hAccelerator); 
					PIASizeFactor = SavePIASizeFactor;
				}
				GSSiGlobUlFree (&htxt);
				GSSiGlobFree (&hAreaPoints);
			}	
			break;
		} 
		SetTextColor (hDC,OldColor);
		RestoreDC (hDC,-1);
	}
	ShowVal.Text[0]=0;
	if (hSaveFont)
	{
		SetGlobalValue ("%LABELFONT",pSaveFont);
		GSSiGlobUlFree (&hSaveFont);
		DispText (0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-1,0,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0); 
	} 
	CurTheme = SaveTheme; 
//	sprintf (str,"Return %f",PIASizeFactor);
//	SetWindowText (hWndMain,str);
{
#if ENABLETRACE
GSSiExitProg (1275);
#endif
	return;
}
#if ENABLETRACE
}
#endif
} 

BOOL CacheDisplayTheme (short from)
#if ENABLETRACE
{GSSiEnterProg (1332);
#endif
{   
	LPVIEWPORT	SaveVP=CurView;
	HANDLE	hMem = GSSiGlobAlloc (1333,GMEM_MOVEABLE,1024);
	LPSTR	CacheFile=GlobalLock (hMem), SaveScreenFile=CacheFile+256; 
	LPOFSTRUCTGM	pOFStruct=(LPOFSTRUCTGM) (SaveScreenFile + 256); 
	LPSAVESCREEN	pSaveScreen; 
	HANDLE	hBitmap; 
	BOOL	rtn=FALSE;  
	DWORD	Err;
	
	if (Printing)
		goto Exit;
	switch (from)
	{   
		default: 
			goto Exit;
		case 2:
		case 0: 
			SetViewport (CurTheme->TargetViewport);
			if (!CurView->CurZoomAreaRef || !*CurView->CurVisibilityID || CurView->DisplayedFullScreen)
				break;
			GetTempDir (SaveScreenFile);  
			_fstrcat (SaveScreenFile,"\\gmsavesc");
			ExpandText (SaveScreenFile);
			GSSiMakeDir (SaveScreenFile,&Err);
			sprintf (_fstrchr(SaveScreenFile,0),"\\%lx\\%s.scr",CurView->CurZoomAreaRef,CurView->CurVisibilityID); 
			if (from == 0)
			{
		        if (GetCacheFile (CacheFile, SaveScreenFile,FALSE, 0))
		        {   
		        	long	UpdateTime;
					MNMXCORD	Bounds;
		        	
		        	if ((hBitmap = ReadSavedScreen (CacheFile, &UpdateTime,&Bounds)))
		        	{   
		        		double	dtime = difftime (UpdateTime,OpenConfigStat.st_mtime); 
                        
		        		pSaveScreen = (LPSAVESCREEN)GlobalLock (hBitmap);
		    			pSaveScreen->pVP = CurView; 
		    			CurView->BitmapID = pSaveScreen->ID;
		        		if (dtime < 0 ||
		        			CurView->Rect.left != pSaveScreen->Rect.left ||
		        			CurView->Rect.right != pSaveScreen->Rect.right ||
		        			CurView->Rect.top != pSaveScreen->Rect.top ||
		        			CurView->Rect.bottom != pSaveScreen->Rect.bottom||
							CurView->WBounds.xmn != Bounds.xmn ||
							CurView->WBounds.xmx != Bounds.xmx ||
							CurView->WBounds.ymn != Bounds.ymn ||
							CurView->WBounds.ymx != Bounds.ymx)
		        		{
		        			GlobalUnlock (hBitmap);
		        			DestroySavedScreen (&hBitmap,CurView->BitmapID); 
		        			RemoveCacheFile (SaveScreenFile);
		        		}
		        		else
		        		{
		        			GlobalUnlock (hBitmap);
	        				RestoreScreen2 (CurView->hDC,hBitmap,CurView->BitmapID,FALSE);
		        			DestroySavedScreen (&hBitmap,CurView->BitmapID); 
	   				        CurView->PassID = 99; 
		        		}
		        	}
		        }
			} 
			else
			{   
				hBitmap = SaveScreen2 (CurView->hWnd,CurView->hDC, CurView->Rect,CurView,&CurView->BitmapID);
		        WriteSavedScreen (SaveScreenFile,hBitmap,&CurView->WBounds);
		    	GetCacheFile (CacheFile, SaveScreenFile,TRUE, 0); 
		    	GSSiRemove (SaveScreenFile); 
		    } 
		    rtn = TRUE;
		break;
	} 
Exit:
	GSSiGlobUlFree (&hMem);
	SetCurView (SaveVP);
{
#if ENABLETRACE
GSSiExitProg (1332);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}


POINT AddjustPointLoc (POINT Point,float Size,int Sequence)
#if ENABLETRACE
{GSSiEnterProg (1331);
#endif
{   
	short	xmove, ymove, shift=0; 
	float	shiftdist;
	
	if (Sequence <= 1)
{
#if ENABLETRACE
GSSiExitProg (1331);
#endif
		return Point;
}
Top:
	if (Sequence == 1)
	{
		xmove = 0;
		ymove = 0;
	}
	else if (Sequence < 4)
	{  
		xmove = -1;
		ymove = Sequence - 2;
	}	
	else if (Sequence < 6)
	{  
		ymove = 1;
		xmove = Sequence - 4;
	}	
	else if (Sequence < 8)
	{  
		xmove = 1;
		ymove = -(Sequence - 6);
	}	
	else if (Sequence < 10)
	{  
		ymove = -1;
		xmove = -(Sequence - 8);
	}	
	else if (Sequence < 14)
	{  
		xmove = -2;
		ymove = Sequence - 11;
	}	
	else if (Sequence < 18)
	{  
		ymove = 2;
		xmove = Sequence - 15;
	}	
	else if (Sequence < 22)
	{  
		xmove = 2;
		ymove = -(Sequence - 19);
	}	
	else if (Sequence < 27)
	{  
		ymove = -2;
		xmove = -(Sequence - 23);
	}	
	else if (Sequence < 32)
	{  
		xmove = -3;
		ymove = Sequence - 28;
	}	
	else if (Sequence < 38)
	{  
		ymove = 3;
		xmove = Sequence - 34;
	}	
	else if (Sequence < 44)
	{  
		xmove = 3;
		ymove = -(Sequence - 40);
	}	
	else if (Sequence < 51)
	{  
		ymove = -3;
		xmove = -(Sequence - 46);
	}	
	else if (Sequence < 58)
	{  
		xmove = -4;
		ymove = Sequence - 53;
	}	
	else if (Sequence < 66)
	{  
		ymove = 4;
		xmove = Sequence - 61;
	}	
	else if (Sequence < 74)
	{  
		xmove = 4;
		ymove = -(Sequence - 69);
	}	
	else if (Sequence < 83)
	{  
		ymove = -4;
		xmove = -(Sequence - 77);
	}	
	else if (Sequence < 92)
	{  
		xmove = -5;
		ymove = Sequence - 86;
	}	
	else if (Sequence < 102)
	{  
		ymove = 5;
		xmove = Sequence - 96;
	}	
	else if (Sequence < 112)
	{  
		xmove = 5;
		ymove = -(Sequence - 106);
	}	
	else if (Sequence < 122)
	{  
		ymove = -5;
		xmove = -(Sequence - 116);
	}	
	else 
	{  
		shift++;
		Sequence -= 121;
		goto Top;
	}
	shiftdist = shift * 0.25 * Size;	
	Point.x += IDNINT (xmove * Size*1.2 + shiftdist);
	Point.y += IDNINT (-ymove * Size*1.2 + shiftdist);
{
#if ENABLETRACE
GSSiExitProg (1331);
#endif
	return Point;
}
#if ENABLETRACE
}
#endif
} 







BOOL SelectThemeClasses (short iclass,BOOL select) 
#if ENABLETRACE
{GSSiEnterProg (1314);
#endif
{
	DLGPROC lpfnDISPLAYSELECTEDCLASSESMsgProc;

	if (!CurView->pTheme)
{
#if ENABLETRACE
GSSiExitProg (1314);
#endif
		return FALSE;
}
	CurTheme = CurView->pTheme;
	if (!CurTheme->IsActive)
{
#if ENABLETRACE
GSSiExitProg (1314);
#endif
		return FALSE;
}
	if (!iclass) //select all
	{
		for (iclass = 0; iclass < CurTheme->NumClass;iclass++)
			CurTheme->ClassStatus[iclass] = !select;
	}
	else if (iclass > 0)
	{
		CurTheme->ClassStatus[iclass-1] = !select;
	}
	else
	{
		setDoPaint(FALSE);
		lpfnDISPLAYSELECTEDCLASSESMsgProc = MakeProcInstance((DLGPROC)DISPLAYSELECTEDCLASSESMsgProc, hInst);
		DialogBox(hInst, (LPSTR)"DISPLAYSELECTEDCLASSES", CurView->hWnd, lpfnDISPLAYSELECTEDCLASSESMsgProc);
		FreeProcInstance(lpfnDISPLAYSELECTEDCLASSESMsgProc);
		setDoPaint(TRUE);
	}
	ClearSecondsInSample();

{
#if ENABLETRACE
GSSiExitProg (1314);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL DisplayThemeGeoCenters (LPTHEME CurTheme)
{
	int		i,j;
	DPOINT	WPoint[MAX_THEME_CLASSES];
	POINT	Point[MAX_THEME_CLASSES+1];
	LPVIEWPORT	SaveVP = CurView;
	HPEN	hPen, hPenWide, hOldPen;
	double	az;

	for (i=0;i<CurTheme->NumClass;i++)
		WPoint[i] = ComputeThemeClassGeoCenter (CurTheme,i);
    SetViewport (CurTheme->TargetViewport); 
    SetDisplayMode (CurView->hDC, GF_SCREENMODE);
	GSSiDeleteObject(&CurView->hRgn);
	CurView->hRgn = CreateVPRgn(FALSE,FALSE);
	SelectClipRgn (CurView->hDC,CurView->hRgn);
	GSSiDeleteObject(&CurView->hRgn); 
	hPenWide = CreatePen (PS_SOLID,3,RGB(255,255,255));
	hPen = CreatePen (PS_SOLID,1,0);
//	FillVPBackground ();
	SelectObject (CurView->hDC,GetStockObject (BLACK_PEN));
	hOldPen = SelectObject (CurView->hDC,hPenWide);
	for (j = 0;j<2;j++)
	{
		for (i=0;i<CurTheme->NumClass;i++)
		{
			Point[i] = BasePtToScreenPt (&WPoint[i]);
			if (i)
			{
				Polyline (CurView->hDC,&Point[i-1],2);
				az = getaz (Point[i-1],Point[i]);
				Point[i+1] = newpt (Point[i], az+(5*HALFPI), 3);
				Polyline (CurView->hDC,&Point[i],2);
				Point[i+1] = newpt (Point[i], az-(5*HALFPI), 3);
				Polyline (CurView->hDC,&Point[i],2);
			}
		}
		SelectObject (CurView->hDC,hPen);
	}
	SelectObject (CurView->hDC,hOldPen);
	GSSiDeleteObject (&hPen);
	GSSiDeleteObject (&hPenWide);
	for (i=0;i<CurTheme->NumClass;i++)
	{
		Point[i] = BasePtToScreenPt (&WPoint[i]);
		TextOutWithShadow (CurView->hDC,Point[i].x,Point[i].y,CurTheme->ClassBM[i],strlen(CurTheme->ClassBM[i]),1,RGB(255,255,255));
	}
	CurView = SaveVP;
	return TRUE;
}

DPOINT ComputeThemeClassGeoCenter (LPTHEME CurTheme,int Class)
#if ENABLETRACE
{GSSiEnterProg (1301);
#endif
{   
	int	n=0;
	DPOINT	GeoCenter={0,0};
	int	pos=BT_FIRST, cond=BT_GE;
	
	if (CurTheme)
	{
		if (OpenThemeHighlightFile (BT_READ))
		{ 
			if (Class < CurTheme->NumClass)
			{
				ThemeHighlightKey.Class = Class;
				ThemeHighlightKey.Refno = LONG_MIN;
				while (!BT_FIND (CurTheme->hHighlightFile,(LPSTR)&ThemeHighlightKey,pos,cond,(LPSTR)&ThemeHighlightData))
				{
					pos  = BT_NEXT;
					cond = BT_ANY;
					if (ThemeHighlightKey.Class != Class)
						break;
					if (ThemeHighlightData.Point.x)
					{
						GeoCenter.x += ThemeHighlightData.Point.x; 
						GeoCenter.y += ThemeHighlightData.Point.y;
						n++;
					}
				}
				if (n)
				{
					GeoCenter.x /= n;
					GeoCenter.y /= n;
				}
			}
			CloseThemeHighlightFile ();
		}
	}
{
#if ENABLETRACE
GSSiExitProg (1301);
#endif
	return GeoCenter;
}
#if ENABLETRACE
}
#endif
}

short GetThemeRefClass (long Refno)
#if ENABLETRACE
{GSSiEnterProg (1301);
#endif
{   
	short	class=-1, iclass;
	
	if (CurTheme)
	{
		if (OpenThemeHighlightFile (BT_READ))
		{ 
			if (CurTheme->NumClass)
			{
				for (iclass=0;iclass<CurTheme->NumClass;iclass++)
				{
					ThemeHighlightKey.Class = iclass;
					ThemeHighlightKey.Refno = Refno;
					if (!BT_FIND (CurTheme->hHighlightFile,(LPSTR)&ThemeHighlightKey,BT_FIRST,BT_EQ,(LPSTR)&ThemeHighlightData))
					{
						class = iclass+1;
						break;
					}
				}
			}
			else
			{
				ThemeHighlightKey.Class = -1;
				ThemeHighlightKey.Refno = Refno;
				if (!BT_FIND (CurTheme->hHighlightFile,(LPSTR)&ThemeHighlightKey,BT_FIRST,BT_EQ,(LPSTR)&ThemeHighlightData))
					class = ThemeHighlightKey.Class+1;
			}
			CloseThemeHighlightFile ();
		}
	}
{
#if ENABLETRACE
GSSiExitProg (1301);
#endif
	return class;
}
#if ENABLETRACE
}
#endif
}

int ShowClassMembers (LPTHEME CurTheme,int Class,POINT fromPt,int *startRef,LPSTR macro)
#if ENABLETRACE
{GSSiEnterProg (1301);
#endif
{   
	int	n=0;
	DPOINT	memberPt;
	int	pos=BT_FIRST, cond=BT_GE;
	
	if (CurTheme)
	{
		if (OpenThemeHighlightFile (BT_READ))
		{ 
			if (Class < CurTheme->NumClass)
			{
				ThemeHighlightKey.Class = Class;
				if (!startRef || *startRef == LONG_MAX)
					ThemeHighlightKey.Refno = LONG_MIN;
				else
					ThemeHighlightKey.Refno = *startRef;
				while (!BT_FIND (CurTheme->hHighlightFile,(LPSTR)&ThemeHighlightKey,pos,cond,(LPSTR)&ThemeHighlightData))
				{
					pos  = BT_NEXT;
					cond = BT_ANY;
					if (ThemeHighlightKey.Class != Class)
						break;
					if (ThemeHighlightData.Point.x)
						memberPt = ThemeHighlightData.Point;
					else
						memberPt = MinMaxMidPointD (&ThemeHighlightData.Bounds);
					LinkScatterPointToMap (fromPt.x,fromPt.y,memberPt,TRUE);
					GdiFlush ();
					if (macro && *macro)
					{
						SetPickGlobalsFromThemeHighlightData (&ThemeHighlightKey,&ThemeHighlightData);
						ProcessText (macro);
					}
					n++;
					if (startRef && *startRef != LONG_MAX)
					{
						*startRef = ThemeHighlightKey.Refno;
						break;
					}
				}
			}
			CloseThemeHighlightFile ();
		}
	}
{
#if ENABLETRACE
GSSiExitProg (1301);
#endif
	return n;
}
#if ENABLETRACE
}
#endif
}

BOOL CreateThemeHighlightFile (void)
#if ENABLETRACE
{GSSiEnterProg (1303);
#endif
{   
	LPSTR pName;
	BTVARDESC	BTVar[2];	
	short	len =  MAX_THEME_VALUE_LEN;
	
	if (CurTheme->ID == GF_SINGLE_VALUE_THEME)
		len = 8;
	CurTheme->ValueLen = len;
	CurTheme->hHighlightFileName = GSSiGlobAlloc (1021,GMEM_MOVEABLE,256);
	pName = GlobalLock (CurTheme->hHighlightFileName);
	GSSiGetTempFileName (0,"gml",0,pName);
	BTVar[0].BT_VARTYP=BT_INTEGER;
	BTVar[0].BT_VARLEN=2;
	BTVar[0].BT_VAROFF=0;
	BTVar[1].BT_VARTYP=BT_INTEGER;
	BTVar[1].BT_VARLEN=4;
	BTVar[1].BT_VAROFF=2;
	BT_CREATE (pName, sizeof(ThemeHighlightData)-MAX_THEME_VALUE_LEN+len, FALSE, 2, 1, (LPBTVARDESC)BTVar, FALSE, 0, 0, FALSE);
	GlobalUnlock (CurTheme->hHighlightFileName);    
{
#if ENABLETRACE
GSSiExitProg (1303);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL OpenThemeHighlightFile (short mode)
#if ENABLETRACE
{GSSiEnterProg (1304);
#endif
{   
	LPSTR	pName;
	

	if (!CurTheme->hHighlightFileName)
	{
		if (mode == BT_READ)
{
#if ENABLETRACE
GSSiExitProg (1304);
#endif
			return FALSE;
}
		CreateThemeHighlightFile ();
	}
	pName = GlobalLock (CurTheme->hHighlightFileName);
	if (pName)
	{
		CurTheme->hHighlightFile = BT_OPEN (pName, 0, mode, 0); 
		if (mode == BT_WRITE)
			BT_CLEAR (CurTheme->hHighlightFile);
	}
	GlobalUnlock (CurTheme->hHighlightFileName);
{
#if ENABLETRACE
GSSiExitProg (1304);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL CloseThemeHighlightFile (void)
#if ENABLETRACE
{GSSiEnterProg (1305);
#endif
{
	if (CurTheme->hHighlightFile)
	{
		BT_CLOSE (CurTheme->hHighlightFile);
		CurTheme->hHighlightFile = 0;
	}
{
#if ENABLETRACE
GSSiExitProg (1305);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
} 

BOOL DeleteThemeHighlightFile (void)
#if ENABLETRACE
{GSSiEnterProg (1306);
#endif
{   
	LPSTR pName; 
	OFSTRUCTGM	OFStruct;
	
	if (!CurTheme->hHighlightFileName)
{
#if ENABLETRACE
GSSiExitProg (1306);
#endif
		return FALSE;
}
	CloseThemeHighlightFile (); 
	pName = GlobalLock (CurTheme->hHighlightFileName);
	if (pName)
		GSSiRemove (pName); 
	GSSiGlobUlFree (&CurTheme->hHighlightFileName);
{
#if ENABLETRACE
GSSiExitProg (1306);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}  

double GetThemeClassDistance (int iclass)
#if ENABLETRACE
{GSSiEnterProg (1326);
#endif
{   
	double	Dist=0, MinDist, Dist2;  
	DPOINT	MidPoint, FromPoint;
	HANDLE	handle;  
	HPDPOINT	pPoint; 
	long	nPoints=0,i,Mini;
	short	pos, cond;
	static	double	ClassDist[MAX_THEME_CLASSES];
	
	if (iclass < 0)
		_fmemset (ClassDist,0,sizeof(double)*MAX_THEME_CLASSES);
	if (iclass < 0)
	{	
		if (!OpenThemeHighlightFile (BT_READ))
{
#if ENABLETRACE
GSSiExitProg (1326);
#endif
			return 0;
}
		for (iclass=0;iclass<CurTheme->NumClass;iclass++)
		{   
			Dist = 0;
			if (CurTheme->ClassCount[iclass])
			{
				handle = GSSiGlobAlloc (1025,GMEM_MOVEABLE,(CurTheme->ClassCount[iclass]+1)*sizeof(DPOINT));
				pPoint = (HPDPOINT)GlobalLock (handle); 
				ThemeHighlightKey.Class = iclass;
				ThemeHighlightKey.Refno = LONG_MIN; 
				nPoints = 0;
				pos = BT_FIRST;
				cond = BT_GE;
				while (!BT_FIND (CurTheme->hHighlightFile,(LPSTR)&ThemeHighlightKey,pos,cond,(LPSTR)&ThemeHighlightData))
				{
					pos = BT_NEXT;
					cond = BT_ANY;
					if (ThemeHighlightKey.Class != iclass)
						break;
					if (nPoints)
					{
						MidPoint.x += ThemeHighlightData.Point.x;
						MidPoint.y += ThemeHighlightData.Point.y;
					}
					else
						MidPoint =  ThemeHighlightData.Point;
					pPoint[nPoints++] = ThemeHighlightData.Point;
				}
				MidPoint.x /= nPoints;
				MidPoint.y /= nPoints;
				GetGlobalPVal ("[%STARTPOINT]",&MidPoint,&FromPoint); 
				while (nPoints)
				{   
					MinDist = DBL_MAX;
					for (i=0;i<nPoints;i++) 
					{
						Dist2 = ldistp (FromPoint,pPoint[i]);
						if (Dist2 < MinDist)
						{
							MinDist = Dist2;
							Mini = i;
						}
					}
					Dist += MinDist;
					FromPoint = pPoint[Mini];
					nPoints--;
					if (Mini < nPoints)
						hmemmove ((HPSTR)&pPoint[Mini],(HPSTR)&pPoint[Mini+1],(long)sizeof(DPOINT)*(nPoints-Mini));
				}
				GSSiGlobUlFree (&handle);
				ClassDist[iclass] = Dist*MFT/5280;
			}
		} 
		CloseThemeHighlightFile (); 
{
#if ENABLETRACE
GSSiExitProg (1326);
#endif
		return 0;
}
	}
{
#if ENABLETRACE
GSSiExitProg (1326);
#endif
	return ClassDist[iclass];
}
#if ENABLETRACE
}
#endif
}
void EndProcessingThemeLegends (void)
#if ENABLETRACE
{GSSiEnterProg (1327);
#endif
{   
	short	iview, itheme;  
	LPVIEWPORT	SaveView = CurView;
	BOOL	SaveInDisplayProcessing=InDisplayProcessing;

	InDisplayProcessing = TRUE;
	
	for (iview = 0;iview < *pNumViewports; iview++)
    {
    	SetCurView (pViewports[iview]); 
        if (CurViewActive())
        {
        	if (CurView->pTheme)
			{
				CurTheme=CurView->pTheme;
				if ((CurTheme->ID == PF_COORD_DISPLAY || CurTheme->IsActive) &&
					CurTheme->ID != GF_BOUNDS_DISPLAY_THEME)
				{
					ThemeDisplayLegend(4, SaveView->ID);
				}
			}
		}
	}
	for (iview = 0;iview < *pNumViewports; iview++)
    {
    	SetCurView (pViewports[iview]); 
        if (CurViewActive())
			DisplayCloseIcon ();
	}
	SetCurView (SaveView);
	InDisplayProcessing = SaveInDisplayProcessing;
{
#if ENABLETRACE
GSSiExitProg (1327);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}
void GetClassOrder (short NumClass,LPLONG ClassCounts,LPSHORT Order)
{   
	short	i,j,n=1;
	
	Order[0] = 0;
	for (i=1;i<NumClass;i++)
	{
		for (j=0;j<n;j++)
		{
			if (ClassCounts[i] < ClassCounts[Order[j]]) 
			{
				_fmemmove (&Order[j+1],&Order[j],(n-j)*2);
				Order[j] = i;
				goto Next;
			}
		}     
		Order[n] = i;
Next:	n++;
	}
	return;
}

BOOL ThemeEndDisplayPass(BOOL CloseAll,BOOL PixelThemesOnly,BOOL FromHalt)
#if ENABLETRACE
{GSSiEnterProg (1274);
#endif
{	int	itheme, iclass,ii; 
    LPTHEME SaveTheme=CurTheme, CacheDisplayTheme=0;
     
	if (CurTheme && CurTheme->ID == GF_COMPARE_VIEWPORTS_THEME)
		ii = 1;
	SetTransparency (0);
                    
	for (itheme=0;itheme<CurView->NumThemes;itheme++)
	{   
		if (!CurView->pThemes[itheme])
			goto NextTheme;
		CurTheme=CurView->pThemes[itheme];  
		if (FromHalt && CurTheme->FidDelayedText>0)
		  	CloseAndDeleteFile ((LPHFILE)&CurTheme->FidDelayedText); 
		if (!CurTheme->IsActive|| !CurTheme->VPDisplayed)
			goto NextTheme;
		if (PixelThemesOnly && CurTheme->DataType != THEMEDATATYPE_PIXEL)
			goto NextTheme;
		if (!PixelThemesOnly && CurTheme->DataType == THEMEDATATYPE_PIXEL)
			goto NextTheme;
		GSSiGlobFree (&CurTheme->hVisList);  
		if (CurTheme->ComputeStoredCounts && !FromHalt)
		 	CurTheme->UseStoredCounts = TRUE;
		switch (CurTheme->ID)
		{   
			case GF_NORTH_ARROW_THEME:
				break;
	        case GF_CACHE_DISPLAY_THEME:
	        	CacheDisplayTheme = CurTheme;
			    break;
	        case GF_CONTEST_THEME:
				GSSiDeleteObject(&CurTheme->ClassPen[0]);
				GSSiDeleteObject(&CurTheme->ClassPen[1]);
	        	BT_CLOSE (hBTNetCon); 
	        	hBTNetCon = 0;
	        	break;
	        	
			case GF_NETMARKER_THEME: 
				CloseNetLinkAndRef (CurTheme->ReScan);  
				CurTheme->ReScan=FALSE;
				BT_CLOSE (CurTheme->hScatterFile);
				CurTheme->hScatterFile = 0; 
				if (NetMarkTimer)
					KillTimer(hWndMain,NetMarkTimer); 
				NetMarkTimer = 0;
				break;
			case GF_CITY_THEME:
				DisplayCityNames();
						 
			case GF_POINT_IN_AREA_THEME:
				ii=1;
			case GF_CONNECTION_LINE_THEME:
			case GF_SINGLE_NONNUM_VALUE_THEME: 
				if (CurTheme->FieldFun && !StartAutoClassDef () && CurTheme->numPreloadedValues <= 0)
					BT_CLOSEANDDELETE (&CurTheme->hScatterFile);
				else
					BT_CLOSE (CurTheme->hScatterFile);
				CurTheme->hScatterFile = 0;
				goto SkipRemove;
			case GF_SINGLE_VALUE_THEME: 
			case GF_TIME_DISPLAY_THEME:
			case GF_TWO_VALUE_THEME: 
				if (FromHalt && !CurTheme->DisplayScatterDiagram)
					GSSiRemoveAndClear (CurTheme->ScatterFile);
SkipRemove:
				CloseThemeHighlightFile ();
				if (FromHalt)
					BT_CLOSE (CurTheme->hDisperseFile);
            	else
					ClosePointDispersionFile (CloseAll);
				CurTheme->hDisperseFile = 0;
				CloseThemeDataFile(FALSE); 
				SetShowValDB (0); 
				if (!FromHalt)
					ProcessShowValMacro (FALSE);
				if (ComputeThemePCTByArea (CloseAll))
				{
					GSSiDeleteObject (&CurTheme->NoDataBrush);
					GSSiDeleteObject (&CurTheme->InvalidDataBrush);
					CurTheme = SaveTheme;
{
#if ENABLETRACE
GSSiExitProg (1274);
#endif
					return FALSE;
}
                }
				break;   
			case GF_STREET_TEXT_THEME:
			{  
			    LPSTREETTEXTDATA	pStreetData=(LPSTREETTEXTDATA)CurTheme->ClassBM;
				CloseThemeDataFile(FALSE); 
				CloseThemeHighlightFile ();
				BT_CLOSEANDDELETE (&pStreetData->hNameFile1);   
				pStreetData->NameFile1[0] = 0;
				BT_CLOSE(pStreetData->hNameFile2); 
				pStreetData->hNameFile2 = 0; 
				if (FromHalt)
				{
					GSSiRemoveAndClear (pStreetData->NameFile1);
					GSSiRemoveAndClear (pStreetData->NameFile2);
				}
				if (pStreetData->hListDB)
					CloseGWDatabase (pStreetData->hListDB);   
				pStreetData->hListDB = 0;
				ProcessAllElements = pStreetData->SavePAE;
				if (CloseAll)
					GSSiGlobFree (&CurTheme->hScatterFile);   
			}
				break;
			case GF_STREET_ADDRESS_THEME: 
				if (StreetEditTimer)
					KillTimer(hWndMain,StreetEditTimer); 
				StreetEditTimer = 0;    
				if (CloseAll)
					GSSiGlobFree (&CurTheme->hScatterFile);   
				break;
				
			case GF_POLYINFO_THEME:
				DisplayCurveFactor = 0;
				break;
		}
	NextTheme:
		GSSiDeleteObject(&CurTheme->NoDataBrush);
		GSSiDeleteObject (&CurTheme->InvalidDataBrush);
	}  
	CurTheme = SaveTheme;
{
#if ENABLETRACE
GSSiExitProg (1274);
#endif
	return FALSE;
}

#if ENABLETRACE
}
#endif
} 
BOOL CloseThemeFiles(void)
#if ENABLETRACE
{GSSiEnterProg (1274);
#endif
{
	int	itheme, iclass,ii; 
    LPTHEME SaveTheme=CurTheme, CacheDisplayTheme=0;
                       
	for (itheme=0;itheme<CurView->NumThemes;itheme++)
	{   
		if (!CurView->pThemes[itheme])
			goto NextTheme;
		CurTheme=CurView->pThemes[itheme];  
		switch (CurTheme->ID)
		{   
			case GF_NORTH_ARROW_THEME:
	        case GF_CACHE_DISPLAY_THEME:
				break;
	        case GF_CONTEST_THEME:
				GSSiDeleteObject(&CurTheme->ClassPen[0]);
				GSSiDeleteObject(&CurTheme->ClassPen[1]);
	        	BT_CLOSEANDDELETE (&hBTNetCon); 
	        	break;
	        	
			case GF_NETMARKER_THEME: 
				CloseNetLinkAndRef (CurTheme->ReScan);  
				BT_CLOSEANDDELETE (&CurTheme->hScatterFile);
				break;
			case GF_CITY_THEME:
						 
			case GF_POINT_IN_AREA_THEME:
				ii=1;
			case GF_CONNECTION_LINE_THEME:
			case GF_SINGLE_NONNUM_VALUE_THEME: 
				if (CurTheme->FieldFun && !StartAutoClassDef () && CurTheme->numPreloadedValues <= 0)
					BT_CLOSEANDDELETE (&CurTheme->hScatterFile);
				else
					BT_CLOSE (CurTheme->hScatterFile);
				CurTheme->hScatterFile = 0;
				goto SkipRemove;
			case GF_SINGLE_VALUE_THEME: 
			case GF_TIME_DISPLAY_THEME:
			case GF_TWO_VALUE_THEME: 
				BT_CLOSE2(&CurTheme->hScatterFile);
				GSSiRemoveAndClear (CurTheme->ScatterFile);
SkipRemove:
				BT_CLOSEANDDELETE (&CurTheme->hHighlightFile);
				GSSiGlobFree (&CurTheme->hHighlightFileName);
				BT_CLOSEANDDELETE (&CurTheme->hDisperseFile);
				CloseThemeDataFile(FALSE); 
	  			CloseAndDeleteFile ((LPHFILE)&CurTheme->FidDelayedText); 
				SetShowValDB (0); 
				break;   
			case GF_STREET_TEXT_THEME:
			{  
			    LPSTREETTEXTDATA	pStreetData=(LPSTREETTEXTDATA)CurTheme->ClassBM;
				CloseThemeDataFile(FALSE); 
				BT_CLOSEANDDELETE (&CurTheme->hHighlightFile);
				BT_CLOSEANDDELETE (&pStreetData->hNameFile1);   
				BT_CLOSEANDDELETE (&pStreetData->hNameFile2); 
				if (pStreetData->hListDB)
					CloseGWDatabase (pStreetData->hListDB);   
				pStreetData->hListDB = 0;
				GSSiGlobFree (&CurTheme->hScatterFile);   
			}
				break;
			case GF_STREET_ADDRESS_THEME: 
				if (StreetEditTimer)
					KillTimer(hWndMain,StreetEditTimer); 
				StreetEditTimer = 0;    
				GSSiGlobFree (&CurTheme->hScatterFile);   
				break;
				
			case GF_POLYINFO_THEME:
				DisplayCurveFactor = 0;
				break;
		}
NextTheme:
	;
	}  
	CurTheme = SaveTheme;
{
#if ENABLETRACE
GSSiExitProg (1274);
#endif
	return FALSE;
}

#if ENABLETRACE
}
#endif
} 

COLORREF RGBI(int r, int g, int b, int i)
{
	BYTE v[4];
	v[0] = r;
	v[1] = g;
	v[2] = b;
	v[3] = i;
	COLORREF *prtn = (COLORREF*)v;
	COLORREF rtn = *prtn;
	return rtn;
}
void CreateThemePens (LPTHEME CurTheme,BOOL AlwaysCreate)
#if ENABLETRACE
{GSSiEnterProg (1270);
#endif
{   
	short	iclass, pattern,ii;
	LPVIEWPORT	SaveVP=CurView;  
	double	WF;
	
	if (!AlwaysCreate && CurTheme->HiPrecis == HiPrecis)
{
#if ENABLETRACE
GSSiExitProg (1270);
#endif
		return;
}
	DestroyThemePens (CurTheme);
//	SetCurView (pViewports[CurTheme->TargetViewport-1]); //tempdebu
    SetViewport (CurTheme->TargetViewport); 
    if (HiPrecis)
    	WF = 1;
    else
    	WF = WidthFactor;


	for (iclass=0;iclass<CurTheme->NumDesiredClass;iclass++)
	{   
		COLORREF	color; 
		int			width=0;
		BYTE		PatByt;
		PATBYTE		PatByte;
				
    	width = PatByt = GetWValue (CurTheme->ClassColor[iclass]); 
		_fmemmove(&PatByte, &PatByt, 1);
		if (CurTheme->DataType != 2)
    		width = 0; 
    	else if (CurTheme->ClassFactor[iclass] > 0)
    		width = CurTheme->ClassFactor[iclass];
    	else if (CurTheme->ClassFactor[iclass] < 0)
		{
			if (PRJ_UNITS[1] == 4)
			{
				DPOINT Pt = NewLatLong(CurView->MidPointW.y,CurView->MidPointW.x,CurTheme->ClassFactor[iclass],HALFPI/2); 

				width = ldistp (CurView->MidPointW,Pt)* BaseDistToWinDist ;
			}
			else
				width = -CurTheme->ClassFactor[iclass] * BaseDistToWinDist;
		}
    	if (CurTheme->DataType)
    		PatByt = 0;
    	if (width > 128)
    		width -= 256;
		if (CurTheme->AbsLineWidth[iclass] < 0)
		{
			if (PRJ_UNITS[1] == 4)
			{
				DPOINT Pt = NewLatLong(CurView->MidPointW.y,CurView->MidPointW.x,CurTheme->AbsLineWidth[iclass],HALFPI/2); 

				width = ldistp (CurView->MidPointW,Pt)* BaseDistToWinDist ;
			}
			else
				width = -CurTheme->AbsLineWidth[iclass] * BaseDistToWinDist;
		}
    	color = ColorWOWidth (CurTheme->ClassColor[iclass]);
        if (!width)
        {  
            if (*PenWIDTH > 0)
            	width = *PenWIDTH;   
//            else
//            	width = 1;
        } 
        else
        	ii=1;
        if (width >= 0)
        	width = IDNINT(((double)(width/*+0.5*/)) * WF * DeviceToScreenFactor() * PenWidthFactor); 
        else if (CurView->BaseUnitsPerPixel)
        	width = IDNINT(((double)-width / CurView->BaseUnitsPerPixel)* WF * DeviceToScreenFactor() * PenWidthFactor); 
		if (ComputePCTTheme || !PatByte.Pattern)
			CurTheme->ClassBrush[iclass]=CreateSolidBrush(ConvertColor(ColorWOWidth (CurTheme->ClassColor[iclass]),CurTheme->UseHalfTone));
		else if (PatByte.Pattern == 5)
			CurTheme->ClassBrush[iclass] = GetStockObject(NULL_BRUSH);
		else if (useGDIPlus)
			CurTheme->ClassBrush[iclass] = CreateTransparentBrush(PatByte.Pattern, color);
		else
		{
			HBITMAP hbmp = hPatBMP[PatByte.Pattern - 1];
			CurTheme->ClassBrush[iclass] = CreatePatternBrush(hbmp);
			//DeleteObject(hbmp);
		}
		CurTheme->ClassPen[iclass]= CreatePen(PS_SOLID,width,ConvertColor(color,CurTheme->UseHalfTone));
	} 
	SetCurView (SaveVP);     
	CurTheme->HiPrecis = HiPrecis;
{
#if ENABLETRACE
GSSiExitProg (1270);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

void DestroyClassPens (void)
#if ENABLETRACE
{GSSiEnterProg (1271);
#endif
{   int	itheme, iclass, i; 
    LPTHEME	SaveCurTheme=CurTheme;  
    
    if (!CurView)
{
#if ENABLETRACE
GSSiExitProg (1271);
#endif
    	return;
}
	for (itheme=0;itheme<CurView->NumThemes;itheme++)
	{
		CurTheme=CurView->pThemes[itheme];
		if (CurTheme)
		{
			DestroyThemePens (CurTheme);
		}
	}
	CurTheme = SaveCurTheme;
{
#if ENABLETRACE
GSSiExitProg (1271);
#endif
	return;
}
#if ENABLETRACE
}
#endif
} 

void DestroyThemePens (LPTHEME CurTheme) 
#if ENABLETRACE
{GSSiEnterProg (1272);
#endif
{
	short iclass;
	BOOL PenInUse = FALSE;
	BOOL BrushInUse = FALSE;     
	HPEN	OldPen=0;
	HBRUSH	OldBrush=0;
	
    if (CurView)
    {
		OldPen = SelectObject (CurView->hDC,GetStockObject(NULL_PEN));
		OldBrush = SelectObject (CurView->hDC,GetStockObject(NULL_BRUSH)); 
	}
	
//	for (iclass=0;iclass<CurTheme->NumDesiredClass;iclass++)
	for (iclass=0;iclass<MAX_THEME_CLASSES;iclass++)
	{   
		if (CurTheme->ClassPen[iclass] == OldPen)
			PenInUse = TRUE;
		if (CurTheme->ClassBrush[iclass] == OldBrush)
			BrushInUse = TRUE;
		if (CurTheme->ClassBrush[iclass] != GetStockObject (NULL_BRUSH))
			GSSiDeleteObject(&CurTheme->ClassBrush[iclass]);
		GSSiDeleteObject(&CurTheme->ClassPen[iclass]);
	}
	CurTheme->HiPrecis = 2; 
	if (OldPen && !PenInUse)
		SelectObject (CurView->hDC,OldPen);
	if (OldBrush && !BrushInUse)
		SelectObject (CurView->hDC,OldBrush);
{
#if ENABLETRACE
GSSiExitProg (1272);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

void SetThemeElementCharacteristics (int iclass) 
#if ENABLETRACE
{GSSiEnterProg (1228);
#endif
{   
	BYTE		PatByt;
	PATBYTE		PatByte;
	HDC			SavDC = CurView->hDC;
	BOOL		fromLegend = FALSE;

	if (!Display)
{
#if ENABLETRACE
GSSiExitProg (1228);
#endif
		return;
}
	if (iclass < 0)
	{
		iclass = -iclass - 1;
		fromLegend = TRUE;
	}
	if (CurTheme->ID == GF_CONNECTION_LINE_THEME)
	{
		if (CurrentType == GF_POINT)
		{
			BOOL	err;
			DPOINT	ToPoint = atopt (CurTheme->CurValue,&err);
			DPOINT	LinePoints[2];
			HPEN	OldPen;

			LinePoints[0] = CurPointLocD;
			LinePoints[1] = ToPoint;
			OldPen = SelectObject (CurView->hDC,GetStockObject (BLACK_PEN));
			GWPolylineD (CurView->hDC,LinePoints,2,0);
			SelectObject (CurView->hDC,OldPen);
		}
		return;
	}
	if (CurTheme->CompareDC)
		CurView->hDC = CurTheme->CompareDC;
	if (CurTheme->ID == GF_SINGLE_NONNUM_VALUE_THEME && !CurTheme->NumDesiredClass)
	{   
		CurTheme->ClassColor[0] = CurTheme->ValueColor; 
		CurTheme->NumDesiredClass = 1;
		CreateThemePens (CurTheme,TRUE);  
		CurTheme->NumDesiredClass = 0;
		iclass = 0;  
	}
	switch (CurTheme->DataType) 
	{
		case THEMEDATATYPE_AREA: 
			if (CurrentType != GF_AREA)
				break;
			goto SetArea;
		break;
					    		
		case THEMEDATATYPE_POINT: 
			if (CurrentType != GF_POINT)
				break;
			if (!CurTheme->UseFirstSymbol && (ThemePointSym = CurTheme->ClassSymbol[iclass])) 
			{
				SetPointSize (&ThemePointSize,CurTheme->SymSizeC); 
				if (*CurTheme->SymbolFont[0])
				{
					ThemePointSym = -ThemePointSym;   
					_fmemmove (CurSymbolFont,CurTheme->SymbolFont,sizeof(CurSymbolFont));
				}
			}
		//	ThemeWidthFactor = GetWValue (CurTheme->ClassColor[iclass]) + 1;  
			if (!CurTheme->NotSetColor) 
			{
				if (CurTheme->ClassFactor[iclass])
					ThemeWidthFactor = CurTheme->ClassFactor[iclass];
				else
					ThemeWidthFactor = 1;
				SelectObject(CurView->hDC, CurTheme->ClassBrush[iclass]);
				HaveVarFillColor = TRUE;  
				ThemePointColor = GlobalColors[0]=CurTheme->ClassColor[iclass];
				ThemePointUseHalfTone = CurTheme->UseHalfTone; 
				SetTextColor (CurView->hDC,CurTheme->ClassColor[iclass]);
			} 
			if (!fromLegend)
				ProcessGraphicsAttributeMacro();

			break;
		case THEMEDATATYPE_LINE: 
			if (CurrentType != GF_LINE && CurrentType != GF_POLYLINE && CurrentType != GF_CURVE)
				break;
			goto SetLine; 
			break; 
		case THEMEDATATYPE_TEXT: //text  
			if (CurrentType != GF_TEXT)
				break;
			if (!CurTheme->NotSetColor) 
			{
				SetTextColor (CurView->hDC,CurTheme->ClassColor[iclass]);
				ThemeTextSizeFactor = CurTheme->ClassFactor[iclass];
			}
			break;
		case THEMEDATATYPE_ALL: //all   
			switch (CurrentType)
			{
				case GF_AREA:
				case GF_POINT:
				case GF_TEXT:
					goto SetArea;
				break;
				default:
					goto SetLine;
				break;
			}
			break; 
		case THEMEDATATYPE_PIXEL:  
			ThemePointColor = CurTheme->ClassColor[iclass];
			ThemePointUseHalfTone = CurTheme->UseHalfTone; 
			goto SetArea;
			break;
	}
	CurView->hDC = SavDC;
{
#if ENABLETRACE
GSSiExitProg (1228);
#endif
	return; 
}

SetLine:
	if (!CurTheme->NotSetColor) 
	{
		SelectObject (CurView->hDC,CurTheme->ClassPen[iclass]);
		HaveVarFillColor = TRUE;  
		ThemePointColor = GlobalColors[0]=CurTheme->ClassColor[iclass]; 
		ThemePointUseHalfTone = CurTheme->UseHalfTone; 
		SetTextColor (CurView->hDC,CurTheme->ClassColor[iclass]);
		if (!fromLegend)
			ProcessGraphicsAttributeMacro();
		CurThemeClass = iclass;
	}
	if (CurTheme->ClassFactor[iclass] > 0)  
		ThemeWidthFactor = CurTheme->ClassFactor[iclass];  
	else if (CurTheme->ClassFactor[iclass] < 0)
		ThemeWidthFactor = -BaseDistToWinDist * CurTheme->ClassFactor[iclass];  
	else if (ItemSymbolWidth > 0)
		ThemeWidthFactor = ItemSymbolWidth;
	else if (ItemSymbolWidth < 0)
		ThemeWidthFactor = -BaseDistToWinDist * ItemSymbolWidth;  
	else
		ThemeWidthFactor = 1;
	if (CurTheme->AbsLineWidth[iclass])
		ItemSymbolWidth = CurTheme->AbsLineWidth[iclass];
	if (ItemSymbolWidth && PRJ_UNITS[1] == 4)
	{
		DPOINT Pt = NewLatLong(CurView->MidPointW.y,CurView->MidPointW.x,ItemSymbolWidth,HALFPI/2); 

		ItemSymbolWidth = -ldistp (CurView->MidPointW,Pt);
	}

	CurView->hDC = SavDC;
{
#if ENABLETRACE
GSSiExitProg (1228);
#endif
	return;
}
		 
SetArea:
	if (CurrentType == GF_AREA && nPoly>1)
	{   
		if (!CurTheme->NotSetColor) 
		{
			SelectObject (CurView->hDC, h0Pen);
			ThemePolyPen = CurTheme->ClassPen[iclass]; 
		}
	}
	else
	{
		if (GetBit (5,(LPSTR)&CurVis->WantType[7])) 
		{   
			short	ii;
			if (!SelectObject (CurView->hDC,hAreaBorderPen[HiPrecis]))
				ii=1;
		}
        else if (!CurTheme->NotSetColor)
    		SelectObject (CurView->hDC,CurTheme->ClassPen[iclass]);
    }
						
	if (!CurTheme->NotSetColor) 
	{
		if (SolidAreas && GetBit (7,(LPSTR)&CurVis->WantType[7]))
			SelectObject (CurView->hDC,CurTheme->ClassBrush[iclass]);
		HaveVarFillColor = TRUE;  
		CurThemeClass = iclass;
		ThemePointColor = GlobalColors[0]=CurTheme->ClassColor[iclass]; 
		SetTextColor (CurView->hDC,CurTheme->ClassColor[iclass]);
		PatByt = GetWValue (CurTheme->ClassColor[iclass]); 
		_fmemmove (&PatByte,&PatByt,1);
	    SetROP2(CurView->hDC,DisplayRasterOpt);
		if (PatByte.Pattern && PatByte.Pattern < 5)
		{
		    SetTextColor (CurView->hDC,ColorWOWidth (CurTheme->ClassColor[iclass]));     
		    SetBkColor (CurView->hDC,RGB(255,255,255));
		    if (PatByte.Transparent)
				SetROP2(CurView->hDC,R2_MASKPEN);
	    } 
		if (!fromLegend)
			ProcessGraphicsAttributeMacro();
	}
	CurView->hDC = SavDC;
{
#if ENABLETRACE
GSSiExitProg (1228);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}  

BOOL CreatePointInAreaDB (LPSTR PIADataFile)
{    
	LPSTR	pDot;    
	BOOL	rtn;   
	char	DefStr[]="Refno(B4),TotCount(B4),TotValue(R8)";
	
	if (!ExistFile (PIADataFile)) 
	{
		GSSiGetTempFileName (0,"gmp",0,PIADataFile);
		if ((pDot= _fstrrchr (PIADataFile,'.')))
			*pDot = 0;
		else
			pDot = _fstrchr (PIADataFile,0);
		_fstrcat (pDot,"\\pia.gmd"); 
	}
	rtn = CreateGWDDatabase (PIADataFile,1,FALSE,0,1,DefStr);
	return rtn;
}





void DisplayTimeLegend (short From)
#if ENABLETRACE
{GSSiEnterProg (1292);
#endif
{   int		xmargin, ymargin;
	long	h, w;
	int		width, height,x, y, fHeight, MaxTextWidth, twidth, i, j;
	HBRUSH	BkBrush;
	SIZE	txSize;
	int		iclass;
	RECT	ClassColorBox;
	HFONT	hfont, hfontOld=0, hfont2;
	char	Text[256], Title[256];
	char	Val1[32], Val2[32];
	long	TotCount;
	float	Pct;
	int		MinFontHeight=2, left,right, tWidth;
	int		inc;
	LPSTR	lpText;
	char	lpLine[256];  
	long	Counts[MAX_THEME_CLASSES][MAX_THEME_CLASSES], nMax;
	HANDLE	hBTX, hBTY;
	short	nClassX=0, nClassY=0, VPNumX=CurTheme->DataFileType, VPNumY=CurTheme->DataType;
	short	pos=BT_FIRST, cval; 
	COLORREF	Color;
	THEMEHIGHLIGHTKEY	ThemeHighlightKey;
	THEMEHIGHLIGHTDATA	ThemeHighlightData;    
	double	xinc, yinc;
	RECT	Rect=CurView->DrawRect;  
	HCURSOR	hcurSave;    
	BOOL	Opened1=FALSE, Opened2=FALSE;
	time_t	systime; 
	struct tm	tmtime;      
	LPDOUBLE	pTimeD;  
	long	SecondsFromStart, SecondsFromMidnite, DaysFromStart, SecondsInDay = 60L*60L*24L;           
	short	Type=1;
	
#define SCATTER_DIAGRAM	1
	
    
    SaveDC (CurView->hDC);
	SetDisplayMode (CurView->hDC, GF_TEXTMODE);
    GSSiDeleteObject(&CurView->hRgn);
    CurView->hRgn = CreateVPRgn (FALSE,FALSE);
    SelectClipRgn (CurView->hDC,CurView->hRgn);
    GSSiDeleteObject(&CurView->hRgn); 
	InflateRect (&Rect,1,1);
    FillRectPoly (CurView->hDC,&Rect,WindowColor);
    if (From < 2)
    	goto Exit;
    
    _fstrcpy (Title,CurTheme->Title);
    ExpandText (Title);
    
	CurTheme->Rect=PctRect (CurView->DrawRect,-CurTheme->Margin);
	xmargin = (((long)CurTheme->Rect.right - CurTheme->Rect.left) * CurTheme->InnerMargin) / 100;
	ymargin = (((long)CurTheme->Rect.bottom - CurTheme->Rect.top) * CurTheme->InnerMargin) / 100;
	h = (((long)CurTheme->Rect.bottom - CurTheme->Rect.top) * CurTheme->TitleHeight) / 100;
	CurTheme->TitleBox.top = CurTheme->Rect.top + ymargin;
	CurTheme->TitleBox.bottom = CurTheme->TitleBox.top + h;
	CurTheme->TitleBox.left = CurTheme->Rect.left + xmargin;
	CurTheme->TitleBox.right = CurTheme->Rect.right - xmargin;
	CurTheme->InfoBox.top = CurTheme->TitleBox.bottom + ymargin;
	CurTheme->InfoBox.bottom = CurTheme->Rect.bottom - ymargin;
	CurTheme->InfoBox.left = CurTheme->Rect.left + xmargin;
	CurTheme->InfoBox.right = CurTheme->Rect.right - ymargin;
	CurTheme->ScatterBox = CurTheme->InfoBox;
	CurTheme->ScatterBox.right = CurTheme->InfoBox.right;

	FillRectPoly (CurView->hDC,&CurTheme->Rect,CurTheme->BGColor);
	FillRectPoly (CurView->hDC,&CurTheme->InfoBox,RGB(255,255,255));
	FillRectPoly (CurView->hDC,&CurTheme->TitleBox,CurTheme->TitleBoxBG);

	width = CurTheme->TitleBox.right - CurTheme->TitleBox.left;
	fHeight = CurTheme->TitleHeight+1;
	twidth = SHRT_MAX;
	while (twidth>width && fHeight>2)
	{   
		fHeight--;
		CurTheme->TitleFont.lfHeight = -MulDiv(fHeight,
											   GetDeviceCaps(CurView->hDC, LOGPIXELSY), 72);
		hfont = CreateFontIndirect((PLOGFONT)&CurTheme->TitleFont);
		hfontOld = SelectObject(CurView->hDC, hfont);
		GetTextExtentPoint32 (CurView->hDC,Title,_fstrlen(Title),&txSize);
		lpText = Title;
		twidth = 0;	
	    while (NextLine (&lpText,lpLine,0))
	    {
			GetTextExtentPoint32 (CurView->hDC,lpLine,_fstrlen(lpLine),&txSize);
			twidth = max (twidth,txSize.cx);
		}  
		SelectObject(CurView->hDC, hfontOld); 
		DeleteObject(hfont);
	}
	x = CurTheme->TitleBox.left + ((CurTheme->TitleBox.right - CurTheme->TitleBox.left) - twidth)/2;
	CurTheme->TitleFont.lfHeight = -MulDiv(CurTheme->TitleHeight-2,
										   GetDeviceCaps(CurView->hDC, LOGPIXELSY), 72);
	hfont = CreateFontIndirect((PLOGFONT)&CurTheme->TitleFont);
	hfontOld = SelectObject(CurView->hDC, hfont); 

    SetTextColor (CurView->hDC,0); 
	lpText = Title;	
	y = CurTheme->TitleBox.top + 1;
    while (NextLine (&lpText,lpLine,0))
    {
		GetTextExtentPoint32 (CurView->hDC,lpLine,_fstrlen(lpLine),&txSize);
		width = txSize.cx;
		x = CurTheme->TitleBox.left + ((CurTheme->TitleBox.right - CurTheme->TitleBox.left) - width)/2;
		TextOut(CurView->hDC, x, y, lpLine,_fstrlen(lpLine));  
		y+= txSize.cy;
	}
	SelectObject(CurView->hDC, hfontOld); 
	DeleteObject(hfont);
    
    if (Type == SCATTER_DIAGRAM)
    {   
    	long	TotDays = (TimeRangeEnd - TimeRangeBeg)/SecondsInDay;
    	POINT	WinPoint;
    	short	isym;
    	
		if (!OpenThemeHighlightFile (BT_READ))
			goto Exit;
    	isym = GetDictSymbolNumber ("CIRCLE");
		width = CurTheme->InfoBox.right - CurTheme->InfoBox.left;
		height = CurTheme->InfoBox.top - CurTheme->InfoBox.bottom;
	    _fmemset (Counts,0,MAX_THEME_CLASSES*MAX_THEME_CLASSES*sizeof(long));
	    hcurSave = GSSiSetCursor (LoadCursor (0,IDC_WAIT));
		while (!BT_FIND (CurTheme->hHighlightFile,(LPSTR)&ThemeHighlightKey,pos,BT_ANY,(LPSTR)&ThemeHighlightData))
	    {   
	    	pos = BT_NEXT;    
	    	pTimeD = (LPDOUBLE)ThemeHighlightData.Value;  
	    	systime = *pTimeD;  
	    	SecondsFromStart = systime - TimeRangeBeg;
	    	DaysFromStart = SecondsFromStart/SecondsInDay;
	    	SecondsFromMidnite = SecondsFromStart % SecondsInDay; 
	    	WinPoint.x = CurTheme->InfoBox.left + (DaysFromStart*width)/TotDays;
	    	WinPoint.y = CurTheme->InfoBox.bottom + (SecondsFromMidnite*height)/SecondsInDay;
			DisplayPointItem (CurView->hDC,WinPoint,5,0,isym,0);
//		tmtime = *localtime (&systime); 
	    }  
		GSSiSetCursor(hcurSave); 
		CloseThemeHighlightFile (); 
    }
    else
    {
	    nMax = 0;
	    for (i=0;i<nClassX;i++)
	    	for (j=0;j<nClassY;j++)
	    		nMax = max (nMax,Counts[i][j]); 
	    if (!nMax)
	    	goto Exit;
		xinc = ((double)CurTheme->InfoBox.right - CurTheme->InfoBox.left)/nClassX;
		yinc = ((double)CurTheme->InfoBox.bottom - CurTheme->InfoBox.top)/nClassY;
	    for (i=0;i<nClassX;i++)
	    {
	    	for (j=0;j<nClassY;j++)
	    	{
				ClassColorBox.left = IDNINT (CurTheme->InfoBox.left + xinc * i);
				ClassColorBox.right = IDNINT (CurTheme->InfoBox.left + xinc * (i+1));
				ClassColorBox.bottom = IDNINT (CurTheme->InfoBox.bottom - yinc * j);
				ClassColorBox.top = IDNINT (CurTheme->InfoBox.bottom - yinc * (j+1)); 
				cval = ((nMax-Counts[i][j]) * 255) / nMax;
				Color = RGB(cval,cval,cval); 
				FillRectPoly (CurView->hDC,&ClassColorBox,Color); 
				if (CurTheme->DisplayCount)
				{
				    SetBkMode(CurView->hDC, TRANSPARENT);  
					CurTheme->ClassFont2.lfHeight = -(ClassColorBox.bottom - ClassColorBox.top - 2* DeviceToScreenFactor()); 
					hfont2 = CreateFontIndirect((PLOGFONT)&CurTheme->ClassFont2);
			        hfontOld = SelectObject(CurView->hDC, hfont2);
					sprintf (Text,"%ld",Counts[i][j]); 
					GetTextExtentPoint32 (CurView->hDC,Text,_fstrlen(Text),&txSize);
					width = txSize.cx;
					x = ClassColorBox.left + ((ClassColorBox.right - ClassColorBox.left) - width)/2; 
					y = ClassColorBox.top + 1* DeviceToScreenFactor(); 
					if (cval >150)
					    SetTextColor (CurView->hDC,0); 
					else
					    SetTextColor (CurView->hDC,RGB(255,255,255));  
					TextOut(CurView->hDC, x, y, Text,_fstrlen(Text)); 
					SelectObject(CurView->hDC, hfontOld); 
					DeleteObject(hfont2);
				}
	    	}
	    }
	    SetTextColor (CurView->hDC,0); 
		fHeight = 12 * DeviceToScreenFactor(); 
		CurTheme->ClassFont1.lfHeight = fHeight;
		hfont = CreateFontIndirect((PLOGFONT)&CurTheme->ClassFont1);
		hfontOld = SelectObject(CurView->hDC, hfont);
		y = ClassColorBox.top - 2*yinc/3 - 2;
	    for (i=0;i<nClassX;i++)
	    {   
	    	LPSTR	pPar;
	    	 
	    	_fstrcpy (Text,CurTheme->ClassBM[i]);
	    	if ((pPar=_fstrrchr (Text,'(')))
	    		*pPar = 0;
			left = IDNINT (CurTheme->InfoBox.left + xinc * i);
			right = IDNINT (CurTheme->InfoBox.left + xinc * (i+1));
	       	GetTextExtentPoint32 (CurView->hDC,Text,_fstrlen(Text),&txSize);
	       	tWidth = txSize.cx; 
	       	x = (left + right) / 2 - tWidth/2;
			TextOut(CurView->hDC, x,y,Text,_fstrlen(Text));
	    }
		SelectObject(CurView->hDC, hfontOld);
		DeleteObject(hfont);
	} 
Exit:
	RestoreDC (CurView->hDC,-1);
{
#if ENABLETRACE
GSSiExitProg (1292);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}  

BOOL DisplayShowValInfobox (void)
{
	return TRUE;
}

BOOL OpenShowValInfoboxFile (void)
{
	return TRUE;
}

void BeginDelayedText (HFILE FidDelayedText)
{   
  	if (CurTheme->FidDelayedText > 0)
  		CloseAndDeleteFile (&CurTheme->FidDelayedText); 
	if (!CurTheme->DelayTextDisplay)
		return;
	CurTheme->FidDelayedText = OpenTempNamedFile ();
	return;
}

void SetThemeSortOrder (void)
{
	int	i,j,k = CurTheme->SortOption;

	if (CurTheme->ID != GF_SINGLE_NONNUM_VALUE_THEME)
		k = 0;

	switch (k)
	{
	case 1: //sort by class description
		CurTheme->SortOrder[0] = 0;
		for (i=1;i<CurTheme->NumClass;i++)
		{
			CurTheme->SortOrder[i] = i;
			for (j=i-1;j>-1;j--)
			{
				if (stricmp (CurTheme->ClassBM[i],CurTheme->ClassBM[CurTheme->SortOrder[j]]) >= 0)
					break;
				CurTheme->SortOrder[j+1] = CurTheme->SortOrder[j];
				CurTheme->SortOrder[j] = i;
			}
		}
		break;
	case 2: //sort by count
		CurTheme->SortOrder[0] = 0;
		for (i=1;i<CurTheme->NumClass;i++)
		{
			CurTheme->SortOrder[i] = i;
			for (j=i-1;j>-1;j--)
			{
				if (CurTheme->ClassCount[i] >= CurTheme->ClassCount[CurTheme->SortOrder[j]])
					break;
				CurTheme->SortOrder[j+1] = CurTheme->SortOrder[j];
				CurTheme->SortOrder[j] = i;
			}
		}
		break;
	default:
		for (i=0;i<CurTheme->NumClass;i++)
			CurTheme->SortOrder[i] = i;
		break;
	}
	return;
}

void ProcessDataPassBeginMacro(void)
{
	if (*CurTheme->BeginDataPassMacro)
	{
		HANDLE hMem = GSSiGlobAlloc(0, GMEM_MOVEABLE, 4096);
		LPSTR pMem = GlobalLock(hMem);

		strcpy(pMem, CurTheme->BeginDataPassMacro);
		ExpandText(pMem);
		GSSiGlobUlFree(&hMem);
	}
	return;
}
void ProcessDisplayPassBeginMacro(void)
{
	if (*CurTheme->BeginDisplayMacro)
	{
		HANDLE hMem = GSSiGlobAlloc(0, GMEM_MOVEABLE, 4096);
		LPSTR pMem = GlobalLock(hMem);

		strcpy(pMem, CurTheme->BeginDisplayMacro);
		ExpandText(pMem);
		GSSiGlobUlFree(&hMem);
	}
	return;
}
void ProcessDisplayPassEndMacro(void)
{
	if (*CurTheme->EndDisplayMacro)
	{
		HANDLE hMem = GSSiGlobAlloc(0, GMEM_MOVEABLE, 4096);
		LPSTR pMem = GlobalLock(hMem);

		strcpy(pMem, CurTheme->EndDisplayMacro);
		ExpandText(pMem);
		GSSiGlobUlFree(&hMem);
	}
	return;
}

void ProcessGraphicsAttributeMacro(void)
{
	if (*CurTheme->GraphicsAttributesMacro)
	{
		HANDLE hMem = GSSiGlobAlloc(0, GMEM_MOVEABLE, 4096);
		LPSTR pMem = GlobalLock(hMem);

		strcpy(pMem, CurTheme->GraphicsAttributesMacro);
		ExpandText(pMem);
		GSSiGlobUlFree(&hMem);
	}
	return;
}

void DisplaySVThemeLegend(short From)
#if ENABLETRACE
{GSSiEnterProg (169);
#endif
{   int		xmargin, ymargin, i,j;
	long	h, w;
	int		height, width, x, y, fHeight, MaxTextWidth, MaxTextHeight, twidth, theight;
	HBRUSH	BkBrush;
	int		iclass;
	RECT	ClassColorBox;
	HFONT	hfont=0, hfontOld=0, hfont2=0;
	SIZE	txSize;
	char	Text[256], Title[256], ExpLine[256];
	char	Val1[32], Val2[32];
	long	TotCount,MaxCount=0;    
	double	MaxCountD=0, TotCountD=0;
	float	Pct;
	int		MinFontHeight=3;
	int		NumCols=1, MaxClassPerCol, icol, irow;
	double	colw;
	LPSTR	lpText;
	char	lpLine[256];        
	LPTHEME	SavePCTTheme; 
	LPVIEWPORT	SaveVP=CurView;
	double	MaskSize, fontfactor=1, ClassMin,ClassMax;  
	short	SaveFF1=CurTheme->ClassFont1.lfHeight;
	short	SaveFF2=CurTheme->ClassFont2.lfHeight;
	RECT	Rect=CurView->DrawRect;  
	short	BeginCount, CountTextWidth,CountTextHeight=0, usedclass, classinc;   
	RECT	UsedBox[MAX_THEME_CLASSES];    
	double	LineSymFactor = BaseDistToWinDist;
	short	nAutoLines = 0, nal;
	short	tw, inc,ii;
	LPSTR	pAt, pCarrot, pDesc;
	LPSTR	pSemiColon; 
	int		RegionType;  
	BOOL	Circular=FALSE;
	POINT	Point;  
	RECT	BMRect;
	char	IconFile[MAX_PATH];
	char	CheckMarkSymbol[32]="check1.bmp";
    
	if (From == 1)
	{
		ProcessDisplayPassBeginMacro();
	}
    if (From == 4)
	{
		ProcessDisplayPassEndMacro();
		if (DrawDelayedText())
		{

		}
		DisplayShowValInfobox ();
	}
    if (!CurTheme->AddCommas)//UseCheckmark)
    	*CheckMarkSymbol = 0;
    SaveDC (CurView->hDC);
    SetDisplayMode (CurView->hDC, GF_TEXTMODE);
    SetBkMode(CurView->hDC, TRANSPARENT);
    if (CurTheme->DisplayDistance) 
    	GetThemeClassDistance (-1);
    GSSiDeleteObject(&CurView->hRgn);
    CurView->hRgn = CreateVPRgn (FALSE,FALSE);
    RegionType = SelectClipRgn (CurView->hDC,CurView->hRgn);
    GSSiDeleteObject(&CurView->hRgn); 
//  	SelectClipRgn (CurView->hDC,0);
//	InflateRect (&Rect,1,1);  
	if (RegionType == NULLREGION && !PrintingToMF)
		goto Exit;
    if (CurTheme->ID == GF_GRAPHICS_FUNCTION_THEME) 
    {
    	switch (From)
    	{   
    		case 0:
    		case 2:
    		case 4:
    			goto Exit;
    		default:
    			break;
    	}
    	FillThemeFromGFMenu (); 
	    FillRectPoly (CurView->hDC,&Rect,WindowColor);
    }
    else
    {
	    if (!CurView->Transparent)
			FillRectPoly (CurView->hDC,&Rect,WindowColor);
    	if (From < 2)
    		goto Exit;
    }
    if (!CurTheme->NumClass)
    	goto Exit; 
	SetThemeSortOrder ();
    if (CurTheme->ClassFont1.lfHeight)  
    	fontfactor = (double)CurTheme->ClassFont2.lfHeight/(double)CurTheme->ClassFont1.lfHeight;
    TotCount = 0;
	SetThemeColorsFromScheme();
	for (iclass=0;iclass<CurTheme->NumClass;iclass++) 
	{
		MaxCount = max (MaxCount,CurTheme->ClassCount[iclass]);
		TotCount += CurTheme->ClassCount[iclass];
		if (CurTheme->DisplayDistance)
		{ 
			MaxCountD = max (MaxCountD,GetThemeClassDistance(iclass));
			TotCountD += GetThemeClassDistance(iclass);
		}
	}
	if (!TotCount && CurTheme->ClearIfNoCount)
	{   
       	DisplayTAGs2 (CurView->hDC,2,0); 
        goto Exit;
	}
	if (CurTheme->NumCols == MAX_THEME_CLASSES) 
		Circular = TRUE;
	else
    	NumCols = max (1,CurTheme->NumCols + 1);
    SetViewport (CurTheme->TargetViewport); 
    MaskSize = SetMaskSizeVar ();
	SetCurView ( SaveVP); 
//    CurView->hRgn = CreateVPRgn (FALSE,FALSE);
//    SelectClipRgn (CurView->hDC,CurView->hRgn);
//    DeleteObject(CurView->hRgn); 
	CurTheme->Rect=PctRect (CurView->DrawRect,-CurTheme->Margin);
	xmargin = (((long)CurTheme->Rect.right - CurTheme->Rect.left) * CurTheme->InnerMargin) / 100;
	ymargin = (((long)CurTheme->Rect.bottom - CurTheme->Rect.top) * CurTheme->InnerMargin) / 100;
	xmargin = min (xmargin,ymargin);
	ymargin = xmargin; 
	CurTheme->ActualXMargin = xmargin/2;
	h = (((long)CurTheme->Rect.bottom - CurTheme->Rect.top) * CurTheme->TitleHeight) / 100;
	CurTheme->TitleBox.top = CurTheme->Rect.top + ymargin;
	CurTheme->TitleBox.bottom = CurTheme->TitleBox.top + h;
	CurTheme->TitleBox.left = CurTheme->Rect.left + xmargin;
	CurTheme->TitleBox.right = CurTheme->Rect.right - xmargin;
	CurTheme->InfoBox.top = CurTheme->TitleBox.bottom + ymargin;
	CurTheme->InfoBox.bottom = CurTheme->Rect.bottom - ymargin;
	CurTheme->InfoBox.left = CurTheme->Rect.left + xmargin;
	CurTheme->InfoBox.right = CurTheme->Rect.right - ymargin;
	CurTheme->ScatterBox = CurTheme->InfoBox;
	w = (((long)CurTheme->InfoBox.right - CurTheme->InfoBox.left) * CurTheme->ScatterWidth) / 100;
	if (!CurTheme->DisplayScatterDiagram) w=0;
	CurTheme->ScatterBox.right = CurTheme->InfoBox.left + w;

	FillRectPoly (CurView->hDC,&CurTheme->Rect,CurTheme->BGColor);
	FillRectPoly (CurView->hDC,&CurTheme->InfoBox,CurTheme->IBBGColor);
	FillRectPoly (CurView->hDC,&CurTheme->TitleBox,CurTheme->TitleBoxBG);
    CurTheme->RadiusPoint = Point = RectMid (&CurTheme->InfoBox); 
    Point.x = CurTheme->InfoBox.right;
    CurTheme->Radius = idist (CurTheme->RadiusPoint,Point);
GetTitleSize: 
	width = CurTheme->TitleBox.right - CurTheme->TitleBox.left - xmargin;
	fHeight = h;
	twidth = theight = SHRT_MAX;
	while ((twidth>width || theight>h) && fHeight>4)
	{   
		fHeight--;
//		CurTheme->TitleFont.lfWeight=FW_BOLD;
		CurTheme->TitleFont.lfHeight = fHeight+1; 
		if (hfontOld && hfont &&(hfontOld == hfont))
			ii=1;
		if (hfontOld) SelectObject(CurView->hDC, hfontOld);
		if (hfont) DeleteObject(hfont);
		hfont = CreateFontIndirect((PLOGFONT)&CurTheme->TitleFont);
		hfontOld = SelectObject(CurView->hDC, hfont);
	    _fstrcpy (Title,CurTheme->Title);
	    ExpandText (Title);
		GetTextExtentPoint32 (CurView->hDC,Title,_fstrlen(Title),&txSize);
		lpText = Title;
		twidth = 0;	
		theight = 0;
		nal = nAutoLines;
	    while (NextLine (&lpText,lpLine,nal))
	    {   
	    	_fstrcpy(ExpLine,lpLine);
	    	ExpandText (ExpLine);
	    	Truncate (ExpLine);
			GetTextExtentPoint32 (CurView->hDC,ExpLine,_fstrlen(ExpLine),&txSize);
			twidth = max (twidth,txSize.cx); 
			theight += txSize.cy;  
			if (nal)
				nal--;
		}
	}
	if (NumCharInString (Title,' ') > 1 && !_fstrchr (Title,'\r') && !nAutoLines && fHeight < h/3) 
	{
		nAutoLines = 3;
		goto GetTitleSize; 
	} 
	if (NumCharInString (Title,' ') > 0 && !_fstrchr (Title,'\r') && nAutoLines==3 && fHeight < h/2) 
	{
		nAutoLines = 2;
		goto GetTitleSize; 
	} 
	SetTextColor (CurView->hDC,CurTheme->TitleTextColor);
	x = CurTheme->TitleBox.left + ((CurTheme->TitleBox.right - CurTheme->TitleBox.left) - twidth)/2;
	y = CurTheme->TitleBox.top + ((CurTheme->TitleBox.bottom - CurTheme->TitleBox.top) - theight)/2;
    _fstrcpy (Title,CurTheme->Title);
    ExpandText (Title);
	lpText = Title;
	nal = nAutoLines;
    while (NextLine (&lpText,lpLine,nal))
    {
    	_fstrcpy(ExpLine,lpLine);
    	ExpandText (ExpLine);
    	Truncate (ExpLine);
		GetTextExtentPoint32 (CurView->hDC,ExpLine,_fstrlen(ExpLine),&txSize);
		width = txSize.cx;
		x = CurTheme->TitleBox.left + ((CurTheme->TitleBox.right - CurTheme->TitleBox.left) - width)/2;
		TextOut(CurView->hDC, x, y, ExpLine,_fstrlen(ExpLine));  
		y+= txSize.cy;
		if (nal)
			nal--;
	}

	if (hfontOld)
		SelectObject(CurView->hDC, hfontOld);
	if (hfont)
		DeleteObject(hfont); 
	hfont = hfontOld = 0;
	ThemeDisplayScatterDiagram();

	/* Display the color boxes */
	if (CurTheme->DisplayScatterDiagram)
		inc = 0; /* boxes connect */
	else
		inc = ymargin / 2;
	if (NumCols > 1)
	{
		MaxClassPerCol = CurTheme->NumClass/NumCols;
		if (CurTheme->NumClass % NumCols)
			MaxClassPerCol++;
	}
	else
		MaxClassPerCol = CurTheme->NumClass;
	h = (CurTheme->ScatterBox.bottom - CurTheme->ScatterBox.top) / MaxClassPerCol;    
	colw = (CurTheme->InfoBox.right - CurTheme->InfoBox.left) / (double)NumCols; 
	w = (colw *  CurTheme->ColorsWidth) / 100;
	SavePCTTheme = ComputePCTTheme;
	ComputePCTTheme = 0;
	AdjustBlue = 253;
	CreateThemePens (CurTheme,FALSE);
	for (i=0;i<CurTheme->NumClass;i++)
	{   
		if (CurTheme->FillRow)  
		{
			irow =  i / NumCols;
			icol = i - irow * NumCols;
		}
		else
		{
			icol = i / MaxClassPerCol;
			irow = i - icol * MaxClassPerCol;
		}
		ClassColorBox.left = IDNINT (CurTheme->ScatterBox.right+xmargin/2 + (NumCols - icol -1) * colw);
		ClassColorBox.right = ClassColorBox.left + w;
		ClassColorBox.bottom = CurTheme->ScatterBox.bottom-inc/2 - (irow * h); 
		ClassColorBox.top = ClassColorBox.bottom - h + inc;
		CurTheme->ClassClrBox[i] = ClassColorBox;
	}
	if (CurTheme->InvertLegend)
	{
		usedclass = CurTheme->NumClass - 1;    
		classinc = -1;
	}
	else
	{
		usedclass = 0;
		classinc = 1;
	}
	for (j=0;j<CurTheme->NumClass;j++)
	{
		i = CurTheme->SortOrder[j];

		iclass = i; 
		ClassColorBox = CurTheme->ClassClrBox[usedclass];
	    SetTextColor (CurView->hDC,ColorWOWidth (CurTheme->ClassColor[iclass]));     
	    SetBkColor (CurView->hDC,RGB(255,255,255));
	    if (w && h && (!CurTheme->HideNullClasses || CurTheme->ClassCount[iclass]>0))
	    {        
			if (CurTheme->ID == GF_GRAPHICS_FUNCTION_THEME && *CurTheme->IconLibrary)
	    	{   
	    		short	minhktsize=(ClassColorBox.bottom - ClassColorBox.top)/3;  
	    		
	    		if (*CheckMarkSymbol)
	    			minhktsize = 0;
	    		_fstrcpy (IconFile,CurTheme->IconLibrary);
	    		if ((pSemiColon = _fstrchr (CurTheme->ClassBM[iclass],';')))
	    			*pSemiColon = 0;
	    		if ((pAt = _fstrchr (CurTheme->ClassBM[iclass],'&')))
	    		{   
	    			short bmh;
	    			
	    			BMRect=ClassColorBox;
	    			*pAt++ = 0;
	    			if ((pCarrot = _fstrchr (CurTheme->ClassBM[iclass],'^')))
	    				pCarrot++;
	    			_fstrcat (IconFile,pAt);
	    			BMRect.bottom -= minhktsize;
					if (pCarrot && !stricmp (pCarrot,"ESC"))
						EscRect = BMRect;
	    			bmh = DisplayBMFileInRect (CurView->hDC,IconFile, BMRect,1);//-2);
	    			if (pCarrot)
	    			{   
	    				short	h,w;
	    				if (*pCarrot != '@') //11/5/2005
	    				{  
		    				SetTextColor (CurView->hDC,0);     
		    				SetBkColor (CurView->hDC,RGB(255,255,255));
							SetBkMode(CurView->hDC, TRANSPARENT);  
							CurTheme->ClassFont2.lfHeight = -max (1,((ClassColorBox.bottom - ClassColorBox.top) - bmh -1)); 
							hfont2 = CreateFontIndirect((PLOGFONT)&CurTheme->ClassFont2);
					        hfontOld = SelectObject(CurView->hDC, hfont2);  
					        GetTextExtentPoint32 (CurView->hDC,pCarrot, _fstrlen(pCarrot),&txSize); 
							h = txSize.cy;  
	                        w = txSize.cx;
		        			TextOut(CurView->hDC, (ClassColorBox.right + ClassColorBox.left)/2 - w/2,
		        								  ClassColorBox.bottom-h, pCarrot, _fstrlen(pCarrot)); 
							if (hfontOld && hfont2 &&(hfontOld == hfont2))
								ii=1;
		        			if (hfontOld)
		        				SelectObject(CurView->hDC, hfontOld);
							GSSiDeleteObject(&hfont2);
	                	}
	    			}
	    			*(pAt-1) = '&';  
	    			if (pSemiColon)
	    				*pSemiColon = ';';
	    		}
			}
	    	else if (CurTheme->DataType == THEMEDATATYPE_AREA || 
	    		CurTheme->DataType == THEMEDATATYPE_TEXT || 
	    		CurTheme->DataType == THEMEDATATYPE_PIXEL ||
				CurTheme->FactorLegend ||
	    		 !CurTheme->ClassSymbol[iclass])
		    {   
		    	RECT	FactoredRect=ClassColorBox;
		    	
		    	if (CurTheme->FactorLegend)
		    	{
		    		if (!MaxCount)
		    			break; 
		    		if (CurTheme->DisplayDistance)
		    			FactoredRect.right = IDNINT ((GetThemeClassDistance(iclass)/MaxCountD)
		    									 * (FactoredRect.right - FactoredRect.left) +
		    									 	FactoredRect.left);
		    		else
		    			FactoredRect.right = IDNINT (((double)CurTheme->ClassCount[iclass]/MaxCount)
		    									 * (FactoredRect.right - FactoredRect.left) +
		    									 	FactoredRect.left);
		    	}
		    	if (!CurTheme->FactorLegend || CurTheme->ClassCount[iclass])
		    	{  
		    		if (Circular) 
		    		{
		    			HANDLE	hPoints=GSSiGlobAlloc (1522,GMEM_MOVEABLE,360*sizeof(POINT));
		    			LPPOINT	Points = (LPPOINT)GlobalLock (hPoints);
		    			short	nPnts;
		    			HBRUSH	OldBrush=SelectObject (CurView->hDC,CurTheme->ClassBrush[iclass]);
		    			HPEN	OldPen = SelectObject (CurView->hDC,GetStockObject(WHITE_PEN));
		    			
		    			nPnts = GetPieSlice (CurTheme->RadiusPoint,CurTheme->Radius,CurTheme->NumClass,iclass,Points);
    					Polygon (CurView->hDC,Points,nPnts);
    					GSSiGlobUlFree (&hPoints); 
    					SelectObject (CurView->hDC,OldBrush); 
    					SelectObject (CurView->hDC,OldPen); 
		    		}
					else
					{
						if (Printing)
							maxIntensity = TRUE; //PDF plots dont handle transparency well
						FillRect(CurView->hDC, &FactoredRect, CurTheme->ClassBrush[iclass]);
						FrameRect(CurView->hDC, &FactoredRect, GetStockObject(BLACK_BRUSH));
						maxIntensity = FALSE;
					}  
				}
			}
			else 
			{   short	symnum=CurTheme->ClassSymbol[iclass];
			
			    if (*CurTheme->SymbolFont[0])
			    {
			    	_fmemmove (CurSymbolFont,CurTheme->SymbolFont,sizeof(CurSymbolFont));
			    	symnum = -symnum;
			    } 
			    if (CurTheme->DataType == 1)
					CurrentType = GF_POINT;
				else
					CurrentType = GF_AREA;
			    nPoly = 0;
				SetThemeElementCharacteristics (-(iclass+1));
				DisplaySymInRect (CurView->hDC,symnum,ClassColorBox,LineSymFactor,TRUE);   
				*CurSymbolFont[0] = 0;
			} 
		}
		if (!CurTheme->CompressNullClasses || !CurTheme->HideNullClasses || CurTheme->ClassCount[iclass]>0) 
			usedclass += classinc;
	}
	if (hfontOld)
		SelectObject(CurView->hDC, hfontOld);
	if (hfont2)
		DeleteObject(hfont2); 
	hfont2 = hfontOld = 0;
    DestroyThemePens (CurTheme); 
    ComputePCTTheme = SavePCTTheme;
	fHeight = h-inc;
	width = colw - (CurTheme->ClassClrBox[0].right - CurTheme->ClassClrBox[0].left) - xmargin;
	height = CurTheme->ClassClrBox[0].bottom - CurTheme->ClassClrBox[0].top;
	MaxTextWidth = MaxTextHeight = SHRT_MAX;
//	CurTheme->ClassFont2.lfItalic=FALSE;
//	CurTheme->ClassFont2.lfWeight=FW_THIN;
	while (MaxTextWidth > width || MaxTextHeight > height)
	{   if (fHeight <= MinFontHeight) goto TooSmall;
		fHeight--;

		MaxTextWidth = MaxTextHeight = 0;
		CurTheme->ClassFont1.lfHeight = fHeight+1;//-MulDiv(fHeight,GetDeviceCaps(CurView->hDC, LOGPIXELSY), 72);
//		CurTheme->ClassFont1.lfWeight=FW_BOLD;

	    TotCount = 0;
		for (iclass=0;iclass<CurTheme->NumClass;iclass++)
		{   
			if (hfontOld && hfont &&(hfontOld == hfont))
				ii=1;
			if (hfontOld) SelectObject(CurView->hDC, hfontOld);
			if (hfont) DeleteObject(hfont);
			hfont = CreateFontIndirect((PLOGFONT)&CurTheme->ClassFont1);
			hfontOld = SelectObject(CurView->hDC, hfont);
			TotCount += CurTheme->ClassCount[iclass];  
			SetGlobalValueLong ("%CLASSCOUNT",CurTheme->ClassCount[iclass]);  
			GetClassMinMax (iclass,&ClassMin,&ClassMax);
			SetGlobalValueReal ("%CLASSMIN",ClassMin);
			SetGlobalValueReal ("%CLASSMAX",ClassMax);
			if (CurTheme->ClassType ==3)
			{ 
				_fstrcpy(Text,CurTheme->ClassBM[iclass]); 
	    		if ((pSemiColon = _fstrchr (Text,';')) && CurTheme->ID == GF_GRAPHICS_FUNCTION_THEME)
	    			*pSemiColon = 0;
				if ((pDesc = MatchLev (Text,'/'))) 
				{
					LPSTR pEnd = _fstrchr (pDesc+1,'/');
					if (pEnd)
					{
						short l = _fstrlen (pEnd);
						_fmemmove (pDesc,pEnd+1,l);
					}
				} 
				ExpandText (Text);
			}
			else
			{
		       	_fstrcpy (Val1,ValueConv ((double)CurTheme->ClassMin[iclass],CurTheme->ValConv,CurTheme->RoundTo,CurTheme->AddCommas));
		       	_fstrcpy (Val2,ValueConv ((double)CurTheme->ClassMax[iclass],CurTheme->ValConv,CurTheme->RoundTo,CurTheme->AddCommas));
	            
	            if (_fstrcmp (Val1,Val2))
		       		wsprintf (Text,"%s to %s",Val1, Val2);
		       	else
		       		_fstrcpy (Text,Val1); 
		       	_fstrcpy (CurTheme->ClassBM[iclass],Text);
		    }
			if (CurTheme->AppendCount && !CurTheme->ClassStatus[iclass])
			{
				if (CurTheme->DisplayDistance)
					sprintf (_fstrchr(Text,0),"%.2f",MaxCountD);
			    else
					sprintf (_fstrchr(Text,0)," (%ld)",MaxCount);
			}
           	GetTextExtentPoint32 (CurView->hDC,Text,_fstrlen(Text),&txSize);
           	MaxTextWidth = max (MaxTextWidth,txSize.cx);
           	theight = txSize.cy;  
       		if (CurTheme->PCTByArea)
            	TotCount = CurTheme->NumNonMask;
           	if (CurTheme->DisplayPCT || CurTheme->DisplayCount)
           	{
				Pct = 100.0;
				if (CurTheme->PCTByArea)
					sprintf (Text,"(%5.1f%%) %.2f acres",Pct,MaskSize*Pct/100); 
				else if (CurTheme->DisplayPCT && CurTheme->DisplayCount)
					sprintf (Text,"%ld (%5.1f%%)",CurTheme->ClassCount[iclass],Pct);
				else if (CurTheme->DisplayPCT)
					sprintf (Text,"%5.1f%%",Pct);
				else if (CurTheme->DisplayCount)
				{
					if (!CurTheme->ClassStatus[iclass])
						sprintf (Text,"%ld",CurTheme->ClassCount[iclass]);
				}
				else if (CurTheme->DisplayDistance)
					sprintf (Text,"%.2f",GetThemeClassDistance(iclass));
				if (hfontOld && hfont2 &&(hfontOld == hfont2))
					ii=1;
				if (hfontOld) SelectObject(CurView->hDC, hfontOld);
				if (hfont2) DeleteObject(hfont2);
				CurTheme->ClassFont2.lfHeight = IDNINT(CurTheme->ClassFont1.lfHeight * fontfactor); 
				hfont2 = CreateFontIndirect((PLOGFONT)&CurTheme->ClassFont2);
		        SelectObject(CurView->hDC, hfont2);
		       	GetTextExtentPoint32 (CurView->hDC,Text,_fstrlen(Text),&txSize);
	           	MaxTextWidth = max (MaxTextWidth,txSize.cx); 
	           	theight += txSize.cy;
	        }
           	MaxTextHeight = max (MaxTextHeight,theight);  
		}
    }

TooSmall:	fHeight *= 0.80;
	if (CurTheme->AppendCount)
	{ 
		if (CurTheme->DisplayDistance)
			sprintf (Text," (%.2f)",MaxCountD);  
		else
			sprintf (Text," (%ld)",MaxCount);
       	GetTextExtentPoint32 (CurView->hDC,Text,_fstrlen(Text),&txSize);
       	CountTextWidth = txSize.cx; 
       	BeginCount = MaxTextWidth - CountTextWidth;
	}
	if (CurTheme->InvertLegend)
	{
		usedclass = CurTheme->NumClass - 1;    
		classinc = -1;
	}
	else
	{
		usedclass = 0;
		classinc = 1;
	}
	for (j=0;j<CurTheme->NumClass;j++)
	{
		iclass = CurTheme->SortOrder[j];
		UsedBox[iclass] = CurTheme->ClassClrBox[usedclass];
       	y = CurTheme->ClassClrBox[usedclass].top+
       		((CurTheme->ClassClrBox[usedclass].bottom-CurTheme->ClassClrBox[usedclass].top)-MaxTextHeight)/2;
		SetGlobalValueLong ("%CLASSCOUNT",CurTheme->ClassCount[iclass]);
		GetClassMinMax (iclass,&ClassMin,&ClassMax);
		SetGlobalValueReal ("%CLASSMIN",ClassMin);
		SetGlobalValueReal ("%CLASSMAX",ClassMax);
		if (CurTheme->ClassType ==3)
		{   
			_fstrcpy(Text,CurTheme->ClassBM[iclass]); 
    		if ((pSemiColon = _fstrchr (Text,';')) && CurTheme->ID == GF_GRAPHICS_FUNCTION_THEME)
    			*pSemiColon = 0;
			if ((pDesc = MatchLev (Text,'/'))) 
			{
				LPSTR pEnd = _fstrchr (pDesc+1,'/');
				if (pEnd)
				{
					short l = _fstrlen (pEnd);
					_fmemmove (pDesc,pEnd+1,l);
				}
			} 
			ExpandText (Text);
		}
		else
		{
	       	_fstrcpy (Val1,ValueConv ((double)CurTheme->ClassMin[iclass],CurTheme->ValConv,CurTheme->RoundTo,CurTheme->AddCommas));
	       	_fstrcpy (Val2,ValueConv ((double)CurTheme->ClassMax[iclass],CurTheme->ValConv,CurTheme->RoundTo,CurTheme->AddCommas));
            if (_fstrcmp (Val1,Val2))
	       		wsprintf (Text,"%s to %s",Val1, Val2);
	       	else
	       		_fstrcpy (Text,Val1); 
	       	_fstrcpy (CurTheme->ClassBM[iclass],Text);
	    } 
		SetTextColor (CurView->hDC,CurTheme->IBTextColor[0]);
        if (hfont) SelectObject(CurView->hDC, hfont);
		x =  CurTheme->ClassClrBox[usedclass].right + xmargin/2; 
		if ((!CurTheme->HideNullClasses || CurTheme->ClassCount[iclass]>0) &&
			  CurTheme->ColorsWidth < 100)
		{   
			if ((pCarrot = _fstrchr (Text,'^')))
				*pCarrot++ = 0; 
			if (CurTheme->CenterText)
			{   
				
		       	GetTextExtentPoint32 (CurView->hDC,Text,_fstrlen(Text),&txSize); 
		       	tw = txSize.cx; 
		       	inc = max (0,(colw - (tw + xmargin))/2);
			}
			else
				inc = 0;
		    SetBkMode(CurView->hDC, TRANSPARENT);
		    if (!pCarrot || !_fstrchr (pCarrot,'&'))
				TextOut(CurView->hDC, x+inc, y, Text, _fstrlen(Text)); 
			if (pCarrot)
			{   
				short	BeginCount2;
				char	CountText[32];   
				LPSTR	pAnd;
				
				if ((pAnd = _fstrchr (pCarrot,'&')))
					*pAnd = 0;
				if (*pCarrot == '[')
				{
					pCarrot++;
					if (*CheckMarkSymbol)
					{
						BMRect = UsedBox[iclass];   
						BMRect.right = BMRect.left + colw;
						BMRect.left = 1 + BMRect.right - CurTheme->ActualXMargin - (colw-(UsedBox[iclass].right - UsedBox[iclass].left));
	    				_fstrcpy (IconFile,CurTheme->IconLibrary);
		    			_fstrcat (IconFile,CheckMarkSymbol);
		    			if (atob (pCarrot))
		    				DisplayBMFileInRect (CurView->hDC,IconFile, BMRect,1);//-2);
					}
					else
					{   
						char	OnOff[3]="  ";
						
						if (atob (pCarrot))
							_fstrcpy (OnOff,"X");  
						sprintf (CountText,"[%s]",OnOff);
					    GetTextExtentPoint32 (CurView->hDC,CountText,_fstrlen(CountText),&txSize);
					    CountTextWidth = txSize.cx; 
					    BeginCount2 = MaxTextWidth - CountTextWidth;
						TextOut(CurView->hDC, x+BeginCount2, y,CountText, _fstrlen(CountText));
					} 
				}
				else
				{ 
					sprintf (CountText,"(%s)",pCarrot);
			       	GetTextExtentPoint32 (CurView->hDC,CountText,_fstrlen(CountText),&txSize);
			       	CountTextWidth = txSize.cx; 
			       	BeginCount2 = MaxTextWidth - CountTextWidth;
					TextOut(CurView->hDC, x+BeginCount2, y,CountText, _fstrlen(CountText)); 
				}
			}
			else if (CurTheme->AppendCount && !CurTheme->ClassStatus[iclass])
			{   
				short	BeginCount2;
				char	CountText[32];
				
				TextOut(CurView->hDC, x+BeginCount, y, " (", 2); 
				if (CurTheme->DisplayDistance)
					sprintf (CountText,"%.2f)",GetThemeClassDistance(iclass)); 
				else
			    	sprintf (CountText,"%ld)",CurTheme->ClassCount[iclass]);
		       	GetTextExtentPoint32 (CurView->hDC,CountText,_fstrlen(CountText),&txSize);
		       	CountTextWidth = txSize.cx; 
		       	CountTextHeight = txSize.cy; 
		       	BeginCount2 = MaxTextWidth - CountTextWidth;
				TextOut(CurView->hDC, x+BeginCount2, y,CountText, _fstrlen(CountText)); 
			} 
		} 
		SetTextColor (CurView->hDC,CurTheme->IBTextColor[1]);
       	GetTextExtentPoint32 (CurView->hDC,Text,_fstrlen(Text),&txSize);
		if (!CurTheme->CompressNullClasses || !CurTheme->HideNullClasses || CurTheme->ClassCount[iclass]>0) 
		{
			usedclass += classinc;
       		y += max (CountTextHeight,txSize.cy);
       	}
		if ((CurTheme->DisplayPCT || CurTheme->DisplayCount)
		 && (!CurTheme->HideNullClasses || CurTheme->ClassCount[iclass]>0))
		{   
			if (TotCount)
				Pct = (100.0 * CurTheme->ClassCount[iclass]) / TotCount; 
			else
				Pct = 0;
			sprintf (Text,"[%%CLASSPCT(%i)]=%5.1f",iclass,Pct);
			ExpandText (Text);
			if (CurTheme->PCTByArea)
				sprintf (Text,"(%5.1f%%) %.2f acres",Pct,MaskSize*Pct/100);
			else if (CurTheme->DisplayPCT && CurTheme->DisplayCount)
			{
				if (!CurTheme->ClassStatus[iclass])
					sprintf (Text,"%ld (%5.1f%%)",CurTheme->ClassCount[iclass],Pct);
			}
			else if (CurTheme->DisplayPCT)
				sprintf (Text,"%5.1f%%",Pct);
			else if (CurTheme->DisplayCount)
			{
				if (!CurTheme->ClassStatus[iclass])
					sprintf (Text,"%ld",CurTheme->ClassCount[iclass]);
			}
			if (hfont2)
	        	SelectObject(CurView->hDC, hfont2);
	       	GetTextExtentPoint32 (CurView->hDC,Text,_fstrlen(Text),&txSize);
			TextOut(CurView->hDC, x+MaxTextWidth/2-txSize.cx/2,
								  y, Text, _fstrlen(Text));    
	       	y += txSize.cy;
		}

	} 

	for (iclass = 0; iclass < CurTheme->NumClass; iclass++)  //give theme editing functions correct final location
	{
		CurTheme->ClassClrBox[iclass] = UsedBox[iclass];
		if (CurTheme->ClassStatus[iclass])
		{
			ClassColorBox = CurTheme->ClassClrBox[iclass];
			ClassColorBox.right += MaxTextWidth;
			DrawUnSelectedClass(iclass, ClassColorBox);
		}
	}


	if (hfontOld && hfont &&(hfontOld == hfont))
		ii=1;
	if (hfontOld)
		SelectObject(CurView->hDC, hfontOld);
	GSSiDeleteObject(&hfont);
	GSSiDeleteObject(&hfont2);
    CurTheme->ClassFont1.lfHeight = SaveFF1;
    CurTheme->ClassFont2.lfHeight = SaveFF2; 
Exit:
	RestoreDC (CurView->hDC,-1);
	SetCurView ( SaveVP);
{
#if ENABLETRACE
GSSiExitProg (169);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}
void CreateClassPens (void)
#if ENABLETRACE
{GSSiEnterProg (1269);
#endif
{   int	itheme, i; 
    LPTHEME	SaveCurTheme=CurTheme;
    
	AdjustBlue = 253;
	for (itheme=0;itheme<CurView->NumThemes;itheme++)
	{
		CurTheme=CurView->pThemes[itheme];
		if (CurTheme)
		{
			if (CurTheme->IsActive && CurTheme->VPDisplayed)
			{
				CurTheme->HiPrecis = 2; 
				CreateThemePens (CurTheme,FALSE);  
			}
		}
	} 
	CurTheme = SaveCurTheme;
{
#if ENABLETRACE
GSSiExitProg (1269);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}  


  


