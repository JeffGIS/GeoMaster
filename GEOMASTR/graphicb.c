#include "graphint.h"

#include "gmextern.h"

#define	MAXPARTS	32
#define MAXHITS		4096
#define MAXOFFSETS	262136L

static	char	OrderNumber[20];
static	POINT   ZBPoints[5];
static	HCURSOR	MoveCursors[10];   
static	short	nBlockingPoints=0;
static	HANDLE	hBlockingPoints=0;
static	HFILE	NewRefFid=HFILE_ERROR;
static	HANDLE	hURT=0;
static	int		Hits[MAXHITS];

BOOL CurrentMidPointArea (LPFLOAT MidPointAZ,LPFLOAT Length,BOOL Clip, LPMNMXCORL pMinMax,LPPOINT pPoint,LPSHORT pNum,short From)
#if ENABLETRACE
{GSSiEnterProg (1142);
#endif
{   POINT	MidPoint, TestPoint, LastPoint, Point,FPoint;
	LPSHORT	ipnt;   
	HPDPOINT	lpDPoints;
	DWORD	/*nPnts,*/ i, cenpt;
	HPPOINTS lpPoints;
	double	X=0, Y=0, dist; 
	short	ib,ie, np=0;
              
    if (*pNum)
{
#if ENABLETRACE
GSSiExitProg (1142);
#endif
    	return FALSE;  
}
    *pNum = 1; 
    if (From == 1 && BlockCompletelyInWindow (CurrentItemMinMax))
    {   
    	FPoint.x = CurrentItemMinMax.xmn;
    	FPoint.y = CurrentItemMinMax.ymn;
   		Point = FileCoordToWinCoord(FPoint);
		AddPointToMinMaxL (Point,pMinMax);
    	FPoint.x = CurrentItemMinMax.xmx;
    	FPoint.y = CurrentItemMinMax.ymx;
   		Point = FileCoordToWinCoord(FPoint);
		AddPointToMinMaxL (Point,pMinMax);
		MidPoint = MinMaxMidPointL(pMinMax);    
		*pPoint = MidPoint;
{
#if ENABLETRACE
GSSiExitProg (1142);
#endif
    	return TRUE;
}
    }
    if (HiPrecis) 
    {
    	lpDPoints = lpDCurPoints;
		LastPoint = BasePtToWinPt (lpDPoints+nPnts-1);   
	}
    else 
    {
	    lpPoints = lpCurPoints;
   		LastPoint = FileCoordToWinCoord(POINTStoPOINT(*(lpPoints+nPnts-1)));
	}
	for (i=0;i<nPnts;i++) 
	{   
		if (HiPrecis)
			Point = BasePtToWinPt (lpDPoints++);   
		else
    		Point = FileCoordToWinCoord(POINTStoPOINT(*lpPoints++));
    	if (!Clip || PtInDrawRect (Point))
    	{   
    		dist = idist (Point,LastPoint);
	        *Length += dist;
    		np++;
			AddPointToMinMaxL (Point,pMinMax);
			if (From == 3)
			{
		    	X += Point.x * dist;
		        Y += Point.y * dist;
		    }
			else
			{
		    	X += Point.x;
		        Y += Point.y;
		    }
		    LastPoint = Point;
		}
	}
    if (!np)
    {	
{
#if ENABLETRACE
GSSiExitProg (1142);
#endif
    	return (FALSE);
}
    }    
	if (From == 3 && *Length)
	{
	    MidPoint.x = (X / *Length);
	    MidPoint.y = (Y / *Length); 
	}
	else
	{
	    MidPoint.x = X / np;
	    MidPoint.y = Y / np; 
		MidPoint = AveragePoint (MidPoint, MinMaxMidPointL(pMinMax));    
	}
	*pPoint = MidPoint;
{
#if ENABLETRACE
GSSiExitProg (1142);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL CurrentMidPoint (int Type,double Just,LPFLOAT MidPointAZ,LPFLOAT Length,LPSHORT Height,BOOL Clip, LPMNMXCORL pMinMax,LPPOINT pPoint,LPDPOINT pWPoint,LPSHORT pNum,short From,LPDOUBLE pWantDist,BOOL RotateToScreen)
#if ENABLETRACE
{GSSiEnterProg (1143);
#endif
{ 
	//Just >1 or <-1 is pixels left or right. <1 or >-1 is pct times line length
	POINT	MidPoint, Point, LastWPoint, LastPoint,NextWPoint;
	DPOINT  StartPoint, AZPt1, AZPt2;
	LPSHORT	ipnt;
	DWORD		/*nPnts,*/ i, j, cenpt;
	HPPOINTS lpPoints;
	HPDPOINT lpDPoints;  
	BOOL	IsIn;
	long	X, Y; 
	int		nAvePt=0;
	long	ib,ie, np=0;
	BOOL	LastPinB;
	static	long	beginpoint; 
	DPOINT	IntPoints[4],LastDPoint;   
	DPOINT	MidPointD;
	short	nint,ii; 
	double	StartDist, EndDist, IntDist[4], AZ, MidDist;
	static	long	dbugref=31771924;
	
	if (CurrentRefno == dbugref)
		i=1;
    
    if (!*pNum) 
    	beginpoint = 0; 
	else if (StreetCenterline)
		beginpoint = nPnts;
    if (beginpoint >= nPnts)
{
#if ENABLETRACE
GSSiExitProg (1143);
#endif
				return FALSE;
}
    if (StreetCenterline)
	{
	   (*pNum)++;  

{
#if ENABLETRACE
GSSiExitProg (1143);
#endif
				return TRUE;
}
	}
    
    *Length = 0; 
    *MidPointAZ = 0;
	MinMaxInitL (pMinMax);
//    if (Type == GF_AREA) 
//    	Type = GF_POLYLINE;
    *Height = 0;
    if (Type == GF_AREA)
{
#if ENABLETRACE
GSSiExitProg (1143);
#endif
    	return CurrentMidPointArea (MidPointAZ,Length,Clip,pMinMax,pPoint,pNum,From);  
}
    (*pNum)++;  
	switch (Type)
	{
		case GF_POINT:
			if (*pNum>1)
{
#if ENABLETRACE
GSSiExitProg (1143);
#endif
				return FALSE;
}
			if (pWPoint)
				*pWPoint = CurPointLocD;
   			Point = BasePtToScreenPt (&CurPointLocD);   
	    	if (PtInRect (&CurView->ScreenRect,Point))
	    	{
				AddPointToMinMaxL (Point,pMinMax);
				if (RotateToScreen)
					*MidPointAZ = -CurView->Rotation;
				else
    				*MidPointAZ = PTRot;
				*pPoint = Point; 
				*Height = CurPointSize*ThemeWidthFactor;
				InflateMinMaxL (pMinMax,(*Height)/2);
{
#if ENABLETRACE
GSSiExitProg (1143);
#endif
				return TRUE; 
}
			}
			else
{
#if ENABLETRACE
GSSiExitProg (1143);
#endif
				return FALSE;
}
		case GF_POLYLINE:
		case GF_LINE: 
		case GF_CURVE:
		case GF_TEXT:
			break;
		default:
{
#if ENABLETRACE
GSSiExitProg (1143);
#endif
			return (FALSE);
}
	}
	if (Just && HiPrecis)
	{
		double	TotDist = GetPolyLengthD (lpDCurPoints, nPnts);
    	double	AZ, AtDist;

		if (Just < 0)
		{
			if (Just >= -1)
				AtDist = TotDist/2 - ((TotDist/2) * -Just);
			else
				AtDist = CurView->BaseUnitsPerPixel * -Just;
		}
		else if (Just <= 1)
			AtDist = TotDist/2 + (TotDist/2 * Just);
		else
			AtDist = TotDist - CurView->BaseUnitsPerPixel * fabs (Just); 

		MidPointD = PointAtDistOnPoly (lpDCurPoints, nPnts,AtDist,&AZ,0);
		MidPoint = BasePtToScreenPt (&MidPointD);
		*MidPointAZ= AZ+CurView->Rotation;
		if (!pWPoint)
			goto Exit;
		if (Just < 0)
			AtDist = 0;
		else
			AtDist = TotDist;
		*pWPoint = PointAtDistOnPoly (lpDCurPoints, nPnts,AtDist,0,0);
		goto Exit;
	}
    if (Clip)
    {
		long	npts = nPnts - beginpoint;
		HANDLE hPoly = GSSiGlobAlloc (0,GMEM_MOVEABLE,sizeof(DPOINT)*npts);
		HPDPOINT	ScreenPoints = GlobalLock (hPoly);
		DPOINT	BasePt;

		j = beginpoint;
		if (HiPrecis)
		{
			for (i=0;i<npts;i++)
				ScreenPoints[i] = BasePtToScreenPtD (&lpDCurPoints[j++]);
		}
		else
		{
			for (i=0;i<npts;i++)
			{
				BasePt = FilePtToBasePt(POINTStoPOINT(lpCurPoints[j++]));
				ScreenPoints[i] = BasePtToScreenPtD (&BasePt);
			}
		}
		if (PtInRect (&CurView->ScreenRect,DPointToPoint (ScreenPoints[0])))
			StartDist = 0;
		else
			StartDist = -1;
		EndDist = 0;
		for (i=1;i<npts;i++)
		{
			nint = GetScreenInt (&ScreenPoints[i-1],&ScreenPoints[i],IntDist);
			switch (nint)
			{
			case 0:
				EndDist += ldistp (ScreenPoints[i-1],ScreenPoints[i]);
				break;
			case 1:
				if (StartDist < 0)
				{
					StartDist = EndDist + IntDist[0];
					EndDist += ldistp (ScreenPoints[i-1],ScreenPoints[i]);
				}
				else
				{
					EndDist += IntDist[0];
					beginpoint += i;
					goto GetMid;
				}
				break;
			case 2:
				EndDist	  = StartDist + IntDist[1];
				StartDist += IntDist[0];
				beginpoint += i;
				goto GetMid;
			}
		}
		beginpoint += i;
GetMid:
		MidDist = (StartDist + EndDist) / 2;
		MidPointD = PointAtDistOnPoly (ScreenPoints,npts,MidDist,&AZ,0); 
		AZ = TWOPI - AZ;
		*MidPointAZ = AZ;
		MidPoint = DPointToPoint (MidPointD);
		*Length = EndDist;
		if (pWantDist)
			*pWantDist = EndDist - StartDist;
		GSSiGlobUlFree (&hPoly);
		goto Exit;
	}
/*
	    ib = -1;
	    if (HiPrecis)
	    	lpDPoints = lpDCurPoints+beginpoint;
	    else
		    lpPoints = lpCurPoints+beginpoint;   
		if (beginpoint)
		{
			LastPinB = FALSE; 
			if (HiPrecis)
				LastDPoint = lpDCurPoints[beginpoint-1];  
			else
	    		LastPoint = POINTStoPOINT(lpCurPoints[beginpoint-1]); 
		}
		else
			LastPinB = TRUE;
    	for (i=beginpoint;i<nPnts;i++) 
    	{   
    		if (HiPrecis)
    		{

    			if ((IsIn = PointInWBounds (lpDPoints)))
    				Point = BasePtToScreenPt (lpDPoints);
				else
				{   
					if (i)
					{
						nint = GetWBoundsInt (&LastDPoint,lpDPoints,IntPoints);
						for (j=0;j<nint;j++)
						{
							Point = BasePtToScreenPt (&IntPoints[j]);
							AddPointToMinMaxL (Point,pMinMax);  
							np++;  
							ie = i+1;
						}
						if (nint)  
						{
							AZPt1 = LastDPoint;
							AZPt2 = *lpDPoints;
							*MidPointAZ = getazd (ProjectBasePt (&AZPt1),ProjectBasePt (&AZPt2)); 
						}
					}
					else if (i<nPnts-1)
					{
						nint = GetWBoundsInt (lpDPoints,lpDPoints+1,IntPoints);
						for (j=0;j<nint;j++)
						{
							Point = BasePtToScreenPt (&IntPoints[j]);
							AddPointToMinMaxL (Point,pMinMax);  
							np++;  
							ie = i+1;
						}
						if (nint)
						{
							AZPt1 = *lpDPoints;
							AZPt2 = *(lpDPoints+1);
							*MidPointAZ = getazd (ProjectBasePt (&AZPt1),ProjectBasePt (&AZPt2)); 
						}
					}
				} 

    			LastDPoint = *lpDPoints++;
    		}   
    		else 
    		{   
    			POINT	point = POINTStoPOINT(*lpPoints);
    			
   			    if (FileProjectionType)
   			    	ProjectFilePt (&point);
    			if ((IsIn = PtInBounds (point)))
	    			Point = FileCoordToWinCoord(POINTStoPOINT(*lpPoints)); 
				else
				{   
					//DPOINT	BasePt =  FilePtToBasePt(*lpPoints);
					
					if (i)
					{   
						POINT	ThisPoint = POINTStoPOINT (*lpPoints);

						nint = GetBoundsInt (&LastPoint,&ThisPoint,IntPoints);
						for (j=0;j<nint;j++)
						{
							Point = BasePtToWinPt (&IntPoints[j]);
							AddPointToMinMaxL (Point,pMinMax);  
							np++;  
							ie = i+1;
						}
						if (nint)  
						{
							AZPt1 = FilePtToBasePt(LastPoint);
							AZPt2 = FilePtToBasePt(POINTStoPOINT(*lpPoints));
							*MidPointAZ = getazd (ProjectBasePt (&AZPt1),ProjectBasePt (&AZPt2)); 
						}
					}
					else if (i<nPnts-1)
					{  
						DPOINT	BasePt2 = FilePtToBasePt(POINTStoPOINT(*(lpPoints+1))),BasePt =  FilePtToBasePt(POINTStoPOINT(*lpPoints));
						
						nint = GetWBoundsInt (&BasePt,&BasePt2,IntPoints);
						for (j=0;j<nint;j++)
						{
							Point = BasePtToWinPt (&IntPoints[j]);
							AddPointToMinMaxL (Point,pMinMax);  
							np++;  
							ie = i+1;
						}
						if (nint)
						{
							*MidPointAZ = getazd (ProjectBasePt (&BasePt),ProjectBasePt (&BasePt2)); 
						}
					}
				} 
	    		LastPoint = POINTStoPOINT(*lpPoints++); 
	    	}
	    	if (IsIn)
	    	{
				AddPointToMinMaxL (Point,pMinMax);
	    		if (ib <0)
	    			ib = i; 
	    		np++;
	    		ie = i+1;
	    	}
	    	else
	    	{ 
	    		if (ib >= 0)
	    		{    
	    			if (np == 1) 
	    			{
	    				ie = i+1;
	    				np++;
	    			}
	    			else
	    				ie = i;
	    			break;
	    		} 
	    	} 
	    }
    }
    else
    {
    	ib = 0;
    	ie = nPnts;  
    	np = nPnts;
    }           
	beginpoint = ie+1;
    if (!np)
    {	
{
#if ENABLETRACE
GSSiExitProg (1143);
#endif
    	return (FALSE);
}
    }    
	if (ib < 0)
	{
		*pPoint = MinMaxMidPointL (pMinMax);   
{
#if ENABLETRACE
GSSiExitProg (1143);
#endif
	return TRUE;
}
    }
    if (HiPrecis)
    {   
    	lpDPoints = lpDCurPoints;   
    	lpDPoints += ib;
		Point = BasePtToScreenPt (lpDPoints);   
    }
    else 
    {
	    lpPoints = lpCurPoints; 
	    lpPoints += ib;
   		Point = FileCoordToWinCoord(POINTStoPOINT(*lpPoints));
	}
	AddPointToMinMaxL (Point,pMinMax);
    X = Point.x;
    Y = Point.y; 
	nAvePt = 1;
    if (np == 1)
    {    
    	if (HiPrecis)
    	{
	    	if (ib)
	    	{
			    LastDPoint = *lpDPoints--; 
			    StartPoint = *lpDPoints++; 
		 	}
		 	else
		 	{ 
			    StartPoint = *lpDPoints++; 
			    LastDPoint = *lpDPoints; 
		    }
		}
	    else
	    {  
	    	if (ib)
	    	{
			    LastDPoint = SPointToDPoint(*lpPoints--); 
			    StartPoint = SPointToDPoint(*lpPoints++); 
		 	}
		 	else
		 	{ 
			    StartPoint = SPointToDPoint(*lpPoints++); 
			    LastDPoint = SPointToDPoint(*lpPoints); 
		    }
		}  
	}
    else
    {   
    	if (HiPrecis) 
    	{
	    	LastWPoint = BasePtToScreenPt (lpDPoints);
    		StartPoint = *lpDPoints++;
    	}
    	else 
    	{
    		LastWPoint = FileCoordToWinCoord(POINTStoPOINT(*lpPoints));
	    	StartPoint = PointToDPoint(POINTStoPOINT(*lpPoints++));  
	    }
	    LastDPoint = StartPoint;
	    cenpt = ib + np/2;
	    for (i=ib+1;i<ie;i++)
	    {   
	    	if (HiPrecis) 
	    		Point = BasePtToScreenPt (lpDPoints);
	    	else
		    	Point = FileCoordToWinCoord(POINTStoPOINT(*lpPoints)); 
			AddPointToMinMaxL (Point,pMinMax);
		    if (i==cenpt)
		    {
		    	if (np % 2 || (i+1)>=ie)
		    	{
		    		MidPoint = AveragePoint (LastWPoint,Point); 
		    		AZPt2 = WinPtToBasePt (Point); 
		    	} 
		    	else 
		    	{
		    		MidPoint = Point;
			    	if (HiPrecis) 
			    		AZPt2 = *(LPDPOINT)(lpDPoints+1);
			    	else
				    	AZPt2 = FilePtToBasePt(POINTStoPOINT(*(lpPoints+1))); 
		    	}
		    	AZPt1 = WinPtToBasePt (LastWPoint);
		    	*MidPointAZ = getazd (ProjectBasePt (&AZPt1),ProjectBasePt (&AZPt2)); 
				nAvePt = -1;
		    }
	        *Length += idist (Point,LastWPoint);
			if (nAvePt >= 0)
			{
			   	X += Point.x;
				Y += Point.y;
				nAvePt++;

				if (pWantDist && *pWantDist && *Length >*pWantDist)
				{
	        		beginpoint = i; 
	    			MidPoint = AveragePoint (LastWPoint,Point); 
		    		AZPt1 = WinPtToBasePt (LastWPoint);
		    		AZPt2 = WinPtToBasePt (Point);
		    		*MidPointAZ = getazd (ProjectBasePt (&AZPt1),ProjectBasePt (&AZPt2)); 
					nAvePt = -1;
				} 
				LastWPoint = Point;
	    		if (HiPrecis) 
	    			LastDPoint = *lpDPoints++;
	    		else
		    		LastDPoint = PointToDPoint(POINTStoPOINT(*lpPoints++)); 
			}
	    } 
    }         
    if (np == 1)
    {   
    	if (StartPoint.x != LastPoint.x || StartPoint.y != LastPoint.y)
    		*MidPointAZ = getazd (ProjectBasePt (&StartPoint),ProjectBasePt (&LastDPoint));
    }  
    if (nAvePt > 0)
    {
	    MidPoint.x = X / nAvePt;
	    MidPoint.y = Y / nAvePt;  
    	MidPoint = MinMaxMidPointL (pMinMax);  
    	*Length = WinDistToWorldDist (idist (*(LPPOINT)pMinMax,*(LPPOINT)&pMinMax->xmx)); 
	}
	*/
Exit:
	*pPoint = MidPoint;
{
#if ENABLETRACE
GSSiExitProg (1143);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

double ComputeAreaArea (HPPOINT lpPoints,long nPnts,LPDOUBLE pPerim)
#if ENABLETRACE
{GSSiEnterProg (1144);
#endif
{
	HANDLE handle=GSSiGlobAlloc (1242,GMEM_MOVEABLE,nPnts*sizeof(DPOINT));
	HPDPOINT	lpDpoints=(HPDPOINT)GlobalLock (handle);
	DWORD	i;   
	double	Area;
	
	for (i=0;i<nPnts;i++)
		lpDpoints[i] = FilePtToBasePt(lpPoints[i]);
	Area = ComputeAreaAreaD (lpDpoints,nPnts,pPerim);
	GSSiGlobUlFree (&handle);
{
#if ENABLETRACE
GSSiExitProg (1144);
#endif
	return Area;
}
#if ENABLETRACE
}
#endif
}

BOOL ShowOrthoRes (double Res)
#if ENABLETRACE
{GSSiEnterProg (1145);
#endif
{   
	short	x,y; 
	char	str[32],fmt[32];  
	         
	if (!WantOrthoResDisplay || !CurView)
{
#if ENABLETRACE
GSSiExitProg (1145);
#endif
		return FALSE;                   
}
	if (CurView->ID != *pCommandViewport)
{
#if ENABLETRACE
GSSiExitProg (1145);
#endif
		return FALSE;  
}
    SaveDC (CurView->hDC);
	SetDisplayMode (CurView->hDC, GF_SCREENMODE); 
/*    SetMapMode    (CurView->hDC, MM_TEXT );
    SetWindowOrgEx  ( CurView->hDC, 0, 0,0 );
    SetViewportOrgEx( CurView->hDC, 0, 0,0 ); */   
    SelectClipRgn (CurView->hDC,0);
	SetBkMode (CurView->hDC,OPAQUE);
	SetTextColor (CurView->hDC,0);  
	SetBkColor (CurView->hDC,RGB(255,255,255)); 
	sprintf (fmt,"%%.%if",WantOrthoResDisplay);
	sprintf (str,fmt,Res);
	x = CurView->Rect.left + 4*(CurView->DrawRect.right - CurView->DrawRect.left)/5;
	y = CurView->DrawRect.bottom + 2;
	TextOut (CurView->hDC,x,y,str,_fstrlen(str));
    RestoreDC (CurView->hDC,-1);
{
#if ENABLETRACE
GSSiExitProg (1145);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

short CoordDisplayMacro (POINT MousePoint)
#if ENABLETRACE
{GSSiEnterProg (1146);
#endif
{	 
	int		iclass; 
    
	if (!CurView->pTheme)
{
#if ENABLETRACE
GSSiExitProg (1146);
#endif
		return 0;    
}
	if (CurView->pTheme->ID != PF_COORD_DISPLAY)
{
#if ENABLETRACE
GSSiExitProg (1146);
#endif
		return 0;
}
	PickList[0].Desc = -1;
	PickList[0].ConfigID = CurrentConfig;
	PickList[0].ViewID = CurView->ID;
	PickList[0].PickedPoint = PickList[0].BeginPoint = PointToDPoint(MousePoint);
	PickList[0].FileNum = -1;
	PickList[0].Type = 4;
	_fstrcpy (PickList[0].Prefix,"%COORDDISPLAY");
{
#if ENABLETRACE
GSSiExitProg (1146);
#endif
	return 1;
}
#if ENABLETRACE
}
#endif
}

short DistDisplayMacro (POINT MousePoint)
#if ENABLETRACE
{GSSiEnterProg (1147);
#endif
{	 
	int		iclass; 
    
	if (!CurView->pTheme)
{
#if ENABLETRACE
GSSiExitProg (1147);
#endif
		return 0;    
}
	if (CurView->pTheme->ID != GF_DISTANCE_THEME)
{
#if ENABLETRACE
GSSiExitProg (1147);
#endif
		return 0;
}
	PickList[0].Desc = -1;
	PickList[0].ConfigID = CurrentConfig;
	PickList[0].ViewID = CurView->ID;
	PickList[0].PickedPoint = PickList[0].BeginPoint = PointToDPoint(MousePoint);
	PickList[0].FileNum = -1;
	PickList[0].Type = 4;
	_fstrcpy (PickList[0].Prefix,"%DISTDISPLAY");
{
#if ENABLETRACE
GSSiExitProg (1147);
#endif
	return 1;
}
#if ENABLETRACE
}
#endif
}

short LegendMacro (POINT MousePoint)
{	 
	int		iclass; 
    
	if (!CurView->pTheme)
		return 0;    
	PickList[0].Desc = -1;
	PickList[0].ConfigID = CurrentConfig;
	PickList[0].ViewID = CurView->ID;
	PickList[0].FileNum = -1;
	PickList[0].Type = 4;
	PickList[0].PickedPoint = PickList[0].BeginPoint = PointToDPoint(MousePoint);
	_fstrcpy (PickList[0].Prefix,"%LEGEND");
	_fstrcpy (PickList[0].UDI,"%LEGEND");
	return 1;
}

BOOL CopyHighlightedRecords (BOOL DeleteFromSource,BOOL ShowStatus,BOOL AssignNewRefs,HANDLE hTran,BOOL ConvertCurves)
#if ENABLETRACE
{GSSiEnterProg (1148);
#endif
{
	long	Refno, n=0, TotLen; 
	short	pos=BT_FIRST, WhichFile=1;
	HIGHLIGHTDATA	HighlightData;   
	HCURSOR	hcurSave; 
	long	debugref=1404967;
	short	ii;
	BOOL	rtn=TRUE;
	
	
	if (DeleteFromSource)
		WhichFile = 2;
	if (ShowStatus)
		CreateStatusWind (CurView->hWnd,1,0);
    else 
    {
		hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT)); 
	}
		
    TotLen = BT_NUM_IN_INDEX (hHighlight); 
    UpdateItem = 0;  
	ConvertCurvesToPolylines = ConvertCurves;   
	hTranReorg = hTran;
	while (ContinueProcessing && !BT_FIND (hHighlight,(LPSTR)&Refno,pos,BT_ANY,(LPSTR)&HighlightData)) 
	{   
		n++; 
		if (n == 535)
			ii=1;
		if (pos == BT_FIRST)
		{
			pos = BT_NEXT;   
	  		SetConfig (HighlightData.PD.ConfigID);
			SetViewport(HighlightData.PD.ViewID);
			if (CurView->UpdateFile)
			{
			   	_fstrcpy (PltName,CurView->lpFiles[CurView->UpdateFile-1]);
			   	PltType = CurView->FileType[CurView->UpdateFile-1];
			   	if (PltType == 4)  
					if (!OpenChronoIndex (PltName,&EditBounds)) 
						break;
			}
		}
		if (Refno == debugref)
			ii=1;
		PickList[0] = HighlightData.PD;
		if (AssignNewRefs)
		{
			PickList[0].Refno = GetNewRefno(PltName,0,0,0,0);
			ChangeRefnoTo = PickList[0].Refno;
		}
		UpdateRecord (0,PickList[0].Desc,PickList[0].Prefix,PickList[0].UDI,0,0,WhichFile,-1);
		ChangeRefnoTo = LONG_MIN;
		if (ShowStatus)
			StatusWindowUpdate ("","Copying Map Data", TotLen, n);
    } 
    hTranReorg = 0;
	ConvertCurvesToPolylines = ConvertCurves; 
	rtn = ContinueProcessing;
    ContinueProcessing = TRUE; 
    CloseChronoIndex ();
	if (ShowStatus)
		DestroyStatusWindow (0);
	else
	{
		GSSiSetCursor(hcurSave);
	}
{
#if ENABLETRACE
GSSiExitProg (1148);
#endif
    return rtn;
}
#if ENABLETRACE
}
#endif
}

BOOL CopySelectedRecords (LPSTR File,BOOL ShowStatus,BOOL AssignNewRefs,HANDLE hTran,BOOL ConvertCurves)
#if ENABLETRACE
{GSSiEnterProg (1148);
#endif
{
	long	Refno, n=0, TotLen; 
	short	pos=BT_FIRST, Option=1;
	HIGHLIGHTDATA	HighlightData;   
	HCURSOR	hcurSave; 
	long	debugref=1404967;
	short	ii;
	BOOL	rtn=TRUE;
	HFILE	Fid;
	
	
	SetViewport (*pCommandViewport);
	if (!CurView->UpdateFile)
		return FALSE;
	_fstrcpy (PltName,CurView->lpFiles[CurView->UpdateFile-1]);
	PltType = CurView->FileType[CurView->UpdateFile-1];
	if (PltType == 4)  
		if (!OpenChronoIndex (PltName,&EditBounds)) 
			return FALSE;
	if (ShowStatus)
		CreateStatusWind (CurView->hWnd,1,0);
    else 
    {
		hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT)); 
	}
		
    TotLen = GSSiLength (File); 
	if (TotLen <= 0)
		return FALSE;
	Fid = GSSiOpenFile (File,0,OF_READ);
    UpdateItem = 0;  
	ConvertCurvesToPolylines = ConvertCurves;   
	hTranReorg = hTran;
	Processing = TRUE;
	while (ContinueProcessing && BigRead (Fid,&lUpdateBuf,4) == 4)
	{   
		n++; 
		if (n == 148068)
			ii=1;
		if (lUpdateBuf > 0)
		{
			HPSTR	pBuf;

			hUpdateBuf = GSSiGlobAlloc ( 665,GMEM_MOVEABLE,MAXREORGBUF); 
			pBuf = GlobalLock (hUpdateBuf);
			BigRead (Fid,pBuf,lUpdateBuf);
			GlobalUnlock (hUpdateBuf);
			if (AssignNewRefs)
				ChangeRefnoTo = GetNewRefno(PltName,0,0,0,0);
			if (!UpdateRecordCopy (Option))
				break;
			Option = 0;
			ChangeRefnoTo = LONG_MIN;
			GSSiGlobFree (&hUpdateBuf);
		}
		if (ShowStatus)
			ContinueProcessing = StatusWindowUpdate ("","Copying Map Data", TotLen, GSSillseek (Fid,0,1));
    } 
	UpdateRecordCopy (2);
    hTranReorg = 0;
	GSSiClose (Fid);
	ConvertCurvesToPolylines = ConvertCurves; 
	rtn = ContinueProcessing;
    ContinueProcessing = TRUE; 
	Processing = FALSE;
    CloseChronoIndex ();
	if (ShowStatus)
		DestroyStatusWindow (0);
	else
	{
		GSSiSetCursor(hcurSave);
	}
{
#if ENABLETRACE
GSSiExitProg (1148);
#endif
    return rtn;
}
#if ENABLETRACE
}
#endif
}

BOOL AreaInArea (LPSTR TAGOrRef,LPSTR ThemeVP,LPSTR AinAGMD,double MinPCT)
#if ENABLETRACE
{GSSiEnterProg (1149);
#endif
{   
	char	DefStr[]="BoundaryRef(B4),PCT(R4),PCTInNone(R4),InRef(B4),InTAG(C42),BoundaryTAG(C42),NumInBoundary(B4)";
	short	NumFields=7, NumIndexFields=1;
	LPSTR	pColon=_fstrchr (TAGOrRef,':');
	long	AreaRef, Num, InRefno;
	BOOL	st; 
	HANDLE	hDB;
	LPGWDHEADER lpGWDHead;
	HIGHLIGHTDATA	HighlightData;   
	char	BoundaryTAG[44], InTAG[44];
	
	if (!ExistFile (AinAGMD))
	{
		if (!CreateGWDDatabase (AinAGMD,1,FALSE,NumFields,NumIndexFields,DefStr))
{
#if ENABLETRACE
GSSiExitProg (1149);
#endif
			return FALSE;
}
	} 
	ClearPolyOff(FALSE);
	if (pColon)
	{  
		*pColon++ = 0;
		st = PickByRefno (0,TAGOrRef,pColon,-1); 
		pColon--;
		*pColon = ':';
		AreaRef = PickList[0].Refno;
	}
	else 
	{
		AreaRef = atol (TAGOrRef);
		st = PickByRefno (AreaRef,0,0,-1);
	}
	sprintf (BoundaryTAG,"%s:%s",PickList[0].Prefix,PickList[0].UDI);
	if (!SelectAreaToOffsetFile(0, 0, 0))
{
#if ENABLETRACE
GSSiExitProg (1149);
#endif
		return FALSE;
}
	ClearHighlightList(FALSE);
	Num = HighlightInArea (CurView->hWnd,0,TRUE,FALSE,0);
    hDB = OpenGWDatabase (AinAGMD,BT_WRITE);
    if (!hDB)
{
#if ENABLETRACE
GSSiExitProg (1149);
#endif
    	return (FALSE);
}
    lpGWDHead = (LPGWDHEADER)GlobalLock (hDB);
    SetFieldValFromCharAndName(lpGWDHead,"BoundaryRef",(LPSTR)&AreaRef,TRUE);
	if (Num)
	{
		BT_FIND (hHighlight,(LPSTR)&InRefno,BT_FIRST,BT_ANY,(LPSTR)&HighlightData);
		InRefno = HighlightData.PD.Refno;
		sprintf (InTAG,"%s:%s",HighlightData.PD.Prefix,HighlightData.PD.UDI);
	}
	else
	{
		InRefno = 0;
		*InTAG = 0;
	}
   	SetFieldValFromCharAndName(lpGWDHead,"InRef",(LPSTR)&InRefno,TRUE);
    SetFieldValFromCharAndName(lpGWDHead,"InTAG",(LPSTR)InTAG,FALSE);
    SetFieldValFromCharAndName(lpGWDHead,"BoundaryTAG",(LPSTR)BoundaryTAG,FALSE);
    SetFieldValFromCharAndName(lpGWDHead,"NumInBoundary",(LPSTR)&Num,TRUE);
	st = GWDReplaceRecord (lpGWDHead,0,0,-1); 
    GlobalUnlock (hDB); 
    CloseGWDatabase (hDB); 
	ClearPolyOff(FALSE);
	ClearHighlightList(FALSE);
{
#if ENABLETRACE
GSSiExitProg (1149);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}  

BOOL ThemeInArea (LPSTR TAGOrRef,LPSTR ThemeVPName,LPSTR TinAGMD,double MinPCT,long Precision)
#if ENABLETRACE
{GSSiEnterProg (1150);
#endif
{   
	char	DefStr[]="BoundaryRef(B4),PCT(R4),PCTInNoClass(R4),InClassNum(B2),InClassDesc(C42),BoundaryTAG(C42),NumClassInBoundary(B2)";
	short	NumFields=7, NumIndexFields=2;
	LPSTR	pColon=_fstrchr (TAGOrRef,':');
	long	AreaRef, Num, InRefno;
	BOOL	st; 
	HANDLE	hDB;
	LPGWDHEADER lpGWDHead;
	HIGHLIGHTDATA	HighlightData;   
	char	BoundaryTAG[44], InTAG[44]; 
	HDC		SaveDCMem = hdcMemMap, SaveDC;  
	BOOL	SaveMemMap = MemMap, SaveMaskOffsetLine = MaskOffsetLine, SaveBleedThrough=BleedThrough;
	DWORD	SaveMMW = MemMapWidth, SaveMMH = MemMapHeight; 
    HBITMAP	SaveMemBitmap=hMemBitmap;
    LPVIEWPORT	ThemeVP, SaveVP=CurView; 
    short	ClassNo;
	BOOL	Err;
    float	PCT, PCTInNoClass;
    double	TotCount=0;
    BOOL	rtn=FALSE;    
    long	CountInAllClasses=0;
	short	i;
	
	SetViewport (*pCommandViewport);	
	SaveDC = CurView->hDC;
	if (!ExistFile (TinAGMD))
	{
		if (!CreateGWDDatabase (TinAGMD,1,FALSE,NumFields,NumIndexFields,DefStr))
			goto Exit;
	}
	ThemeVP = SetVPFromName (ThemeVPName,&Err);
	if (Err)
		goto Exit;
	 
	ClearPolyOff(FALSE);
	if (pColon)
	{  
		*pColon++ = 0;
		st = PickByRefno (0,TAGOrRef,pColon,-1); 
		pColon--;
		*pColon = ':';
		AreaRef = PickList[0].Refno;
	}
	else 
	{
		AreaRef = atol (TAGOrRef);
		st = PickByRefno (AreaRef,0,0,-1);
	} 
	if (!st)
		goto Exit;
	if (!SelectAreaToOffsetFile(0, 0, 0))
		goto Exit;
	sprintf (BoundaryTAG,"%s:%s",PickList[0].Prefix,PickList[0].UDI);  

	MaskOffsetLine = TRUE;	
	BleedThrough = FALSE; 
	if (Precision > 0)
	{   
		UniqueColorInc = 1; 
		ComputePCTBKGColor = 0;
		MemMap = TRUE;
		MemMapWidth = Precision;
		MemMapHeight = Precision;   
		hdcMemMap = CreateCompatibleDC(CurView->hDC);    
		hMemBitmap = CreateCompatibleBitmap (CurView->hDC,(int)MemMapWidth,(int)MemMapHeight); 
		CurView->hDC = hdcMemMap;
		hbmpOld = SelectObject(hdcMemMap, hMemBitmap); 
	}
	else 
	{
		UniqueColorInc = 1;
		ComputePCTBKGColor = RGB (210,210,225);
		ComputePCTBKGColor = GetNextUniqueColor (&ComputePCTBKGColor);
		UniqueColorInc = 5000;
		hdcMemMap = CurView->hDC; 
	}
    SetMainRect (CurView->hWnd,CurView->hDC,0,0);
    SetupViewports (hWndMain,hdcMemMap,0,MainRect,0);
	
	*MemMapName = 0; 
    CurView->CurZoomAreaRef = PickList[0].Refno;
	ZoomToRect(PickList[0].Rect,TRUE);  
    
    MaskOffsetLine = SaveMaskOffsetLine;
    if (Precision > 0)
    {
		SelectObject(hdcMemMap,hbmpOld);
	    DeleteDC (hdcMemMap);  
	    DeleteObject (hMemBitmap);  
	}
	else
		WaitForKeystroke (TRUE);
    SetupViewports (hWndMain,SaveDC,0,MainRect,0);
    hdcMemMap = SaveDCMem;  
	MemMap = SaveMemMap;
	MemMapWidth = SaveMMW;
	MemMapHeight = SaveMMH;
	hMemBitmap = SaveMemBitmap;
	BleedThrough = SaveBleedThrough;
    hDB = OpenGWDatabase (TinAGMD,BT_WRITE);
    if (!hDB)
    	goto Exit;
    lpGWDHead = (LPGWDHEADER)GlobalLock (hDB);
    SetFieldValFromCharAndName(lpGWDHead,"BoundaryRef",(LPSTR)&AreaRef,TRUE);
    SetFieldValFromCharAndName(lpGWDHead,"BoundaryTAG",(LPSTR)BoundaryTAG,FALSE);
    SetFieldValFromCharAndName(lpGWDHead,"NumClassInBoundary",(LPSTR)&ThemeVP->pTheme->NumClass,TRUE);
   	TotCount = CurTheme->NumNonMask;  
	for (i=0;i<ThemeVP->pTheme->NumClass;i++) 
		CountInAllClasses += CurTheme->ClassCount[i];
	if (!TotCount) 
	   	PCTInNoClass = (TotCount - CountInAllClasses)/TotCount;
	else
	   	PCTInNoClass = (TotCount - CountInAllClasses)/TotCount;
	if (ThemeVP->pTheme->NumClass)
	{   
		double	MaxCount=-1;
		
		for (i=0;i<ThemeVP->pTheme->NumClass;i++)
		{   
			ClassNo = i+1;
		   	SetFieldValFromCharAndName(lpGWDHead,"InClassNum",(LPSTR)&ClassNo,TRUE);
		    SetFieldValFromCharAndName(lpGWDHead,"InClassDesc",(LPSTR)ThemeVP->pTheme->ClassBM[i],FALSE);
			if (!TotCount)
				PCT = 0;
			else
				PCT = CurTheme->ClassCount[i] / TotCount; 
			PCT *= -1;
		   	SetFieldValFromCharAndName(lpGWDHead,"PCT",(LPSTR)&PCT,TRUE); 
		   	SetFieldValFromCharAndName(lpGWDHead,"PCTInNoClass",(LPSTR)&PCTInNoClass,TRUE); 
		   	if (fabs (PCT) > MinPCT)
				st = GWDReplaceRecord (lpGWDHead,0,0,-1); 
		}
	}
	else
	{
		PCT = 0; 
		ClassNo=0;
	   	SetFieldValFromCharAndName(lpGWDHead,"PCT",(LPSTR)&PCT,TRUE);
	   	SetFieldValFromCharAndName(lpGWDHead,"PCTInNoClass",(LPSTR)&PCTInNoClass,TRUE); 
		InRefno = 0;
		*InTAG = 0;
	   	SetFieldValFromCharAndName(lpGWDHead,"InClassNum",(LPSTR)&ClassNo,TRUE);
	    SetFieldValFromCharAndName(lpGWDHead,"InClassDesc",(LPSTR)"",FALSE);
		st = GWDReplaceRecord (lpGWDHead,0,0,-1); 
	}
    GlobalUnlock (hDB); 
    CloseGWDatabase (hDB); 
	ClearPolyOff(FALSE);
	ClearHighlightList(FALSE); 
	rtn = TRUE;
Exit:
	SetCurView ( SaveVP);
{
#if ENABLETRACE
GSSiExitProg (1150);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}  

BOOL GetTextString (HWND hWnd,LPSTR String,int lenstring, LPSTR Title,LPSTR ListFile,LPSTR InitVal,long AutoInc,BOOL DropDown,BOOL Sorted)
#if ENABLETRACE
{GSSiEnterProg (1152);
#endif
{
	HANDLE hString, hTitle;
	FARPROC lpfnTEXTSTRINGMsgProc;
	int		nRc;
	char	InitValc[64];
	BOOL	SaveDoPaint = DoPaint;
	HANDLE	hStr=GSSiGlobAlloc (0,GMEM_MOVEABLE,4096);  
	LPSTR	str=GlobalLock (hStr),Arg5;
	
	if (GetGlobalCVal ("[%NEXTCVAL]",str,0))
	{
		LPSTR pComma = MatchLev (str,',');
		
		if (pComma)
		{
			*pComma++ = 0;  
			SetGlobalValue ("%NEXTCVAL",pComma); 
		}
		else 
			SetGlobalValue ("%NEXTCVAL",""); 
		_fstrcpy (String,str);
		GSSiGlobUlFree (&hStr);
		return TRUE;
	}
	DoPaint = FALSE;
	_fstrcpy (str,String); 
	AutoIncIntVal = AutoInc;
	pTEXTSTRING=str;
	pTEXTSTRINGTITLE=Title; 
	pTEXTSTRINGLIST=ListFile; 
	*InitValc = 0;
	if (InitVal)  
		pINITVAL=InitVal;
	else
		pINITVAL=str;
	maxTEXTSTRING=lenstring;
	EnableWindow (hWnd,FALSE); 
	if (DropDown) 
	{
		lpfnTEXTSTRINGMsgProc = MakeProcInstance((FARPROC)TEXTSTRINGMsgProc, hInst);
		if (Sorted)
			nRc = DialogBox(hInst, (LPSTR)"TEXTSTRING_SORTED", hWnd, lpfnTEXTSTRINGMsgProc); 
		else
			nRc = DialogBox(hInst, (LPSTR)"TEXTSTRING", hWnd, lpfnTEXTSTRINGMsgProc); 
	}
	else
	{
		hSelectItemsArgs = GSSiGlobAlloc (1250,GHND,1024);
		Arg5 = GlobalLock (hSelectItemsArgs);   
		*Arg5 = 'N';
		Arg5[1] = 'N';
		_fstrcpy (&Arg5[2],ListFile);
		_fstrcpy (&Arg5[850],Title);
		GlobalUnlock (hSelectItemsArgs);
		lpfnTEXTSTRINGMsgProc = MakeProcInstance((FARPROC)SELECTITEMSMsgProc, hInst);
		if (Sorted)
			nRc = DialogBox(hInst, (LPSTR)"SELECTITEMS", hWnd, lpfnTEXTSTRINGMsgProc);
		else
			nRc = DialogBox(hInst, (LPSTR)"SELECTITEMS_NOTSORTED", hWnd, lpfnTEXTSTRINGMsgProc);
		GSSiGlobFree (&hSelectItemsArgs);
	}	
	FreeProcInstance(lpfnTEXTSTRINGMsgProc);  
	EnableWindow (hWnd,TRUE);
	if (nRc)
		_fstrcpy (String,str);
	DoPaint = SaveDoPaint; 
	GSSiGlobUlFree (&hStr);
{
#if ENABLETRACE
GSSiExitProg (1152);
#endif
	return (nRc);
}
#if ENABLETRACE
}
#endif
}

BOOL GetTextStringML (HWND hWnd,LPSTR String,int lenstring, LPSTR Title,LPSTR InitVal)
#if ENABLETRACE
{GSSiEnterProg (1153);
#endif
{
	HANDLE hString, hTitle;
	FARPROC lpfnTEXTSTRINGMsgProc;
	int		nRc;
	char	str[256], InitValc[64];
	BOOL	SaveDoPaint = DoPaint;
	
	DoPaint = FALSE;
	_fstrcpy (str,String); 
	AutoIncIntVal = 0;
	pTEXTSTRING=str;
	pTEXTSTRINGTITLE=Title;  
	pTEXTSTRINGLIST=0;
	*InitValc = 0;
	if (InitVal)  
		pINITVAL=InitVal;
	else
		pINITVAL=str;
	maxTEXTSTRING=lenstring;
	EnableWindow (hWnd,FALSE);
	lpfnTEXTSTRINGMsgProc = MakeProcInstance((FARPROC)TEXTSTRINGMsgProc, hInst);
	nRc = DialogBox(hInst, (LPSTR)"TEXTSTRING_ML", hWnd, lpfnTEXTSTRINGMsgProc);
	FreeProcInstance(lpfnTEXTSTRINGMsgProc);
	EnableWindow (hWnd,TRUE);
	if (nRc)
		_fstrcpy (String,str);
	DoPaint = SaveDoPaint;
{
#if ENABLETRACE
GSSiExitProg (1153);
#endif
	return (nRc);
}
#if ENABLETRACE
}
#endif
}

BOOL SetScale (LPSTR Scale)
#if ENABLETRACE
{GSSiEnterProg (1154);
#endif
{   
	BOOL	rtn=TRUE;
	if (!_fstricmp (Scale,"REMOVE"))
	{
		CurView->WindowZoomedToOrtho=FALSE; 
		CurView->OrthoRes=0;   
		SetGlobalValue("%SCALE","");
//		GMEnableMenuItem(hWndMain, IDM_Z_IN, MF_BYCOMMAND | MF_ENABLED);
//		GMEnableMenuItem(hWndMain, IDM_Z_OUT, MF_BYCOMMAND | MF_ENABLED);
//		GMEnableMenuItem(hWndMain, IDM_Z_WINDOW, MF_BYCOMMAND | MF_ENABLED);
	}
	else
	{     
		 CurView->OrthoRes = -atof (Scale);
	     CurView->WindowZoomedToOrtho=TRUE;  

//		GMEnableMenuItem(hWndMain, IDM_Z_IN, MF_BYCOMMAND | MF_DISABLED| MF_GRAYED);
//		GMEnableMenuItem(hWndMain, IDM_Z_OUT, MF_BYCOMMAND | MF_DISABLED| MF_GRAYED);
//		GMEnableMenuItem(hWndMain, IDM_Z_WINDOW, MF_BYCOMMAND | MF_DISABLED| MF_GRAYED);
	}
{
#if ENABLETRACE
GSSiExitProg (1154);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}  

BOOL ScaleIsSet (BOOL DisplayMess)
{
	if (CurView->WindowZoomedToOrtho && CurView->OrthoRes < 0)
	{
		if (DisplayMess)
		{   
			char	Mess[256];
			LPVIEWPORT	SaveVP=CurView;
			_fstrcpy (Mess,"You cannot zoom in or out because a scale of [%SCALE] is set.\r\nDo you wish to remove the scale?");
			ExpandText (Mess);
			if (GSSiMessageBox (Mess,"",MB_ICONEXCLAMATION|MB_YESNO,0) == IDYES)
			{   
				CurView = SaveVP;
				CurView->WindowZoomedToOrtho = FALSE;
				SetScale ("REMOVE");
				return FALSE;
			}
		} 
		return TRUE;
	}
	else
		return FALSE;
}



BOOL GetPolyWRect (HANDLE hPoly,long NumPolyPoints,LPRECT pRect)
#if ENABLETRACE
{GSSiEnterProg (1156);
#endif
{    
	LPDPOINT	pPoint = (HPDPOINT)GlobalLock (hPoly);  
	
	RectInit (pRect);
	while (NumPolyPoints--)
		 AddPointToRect (BasePtToWinPt (pPoint++),pRect);
    GlobalUnlock (hPoly);
{
#if ENABLETRACE
GSSiExitProg (1156);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL GetPolyRect (HANDLE hPoly,long NumPolyPoints,LPRECT pRect)
#if ENABLETRACE
{GSSiEnterProg (1156);
#endif
{    
	LPPOINT	pPoint = (HPPOINT)GlobalLock (hPoly);  
	
	RectInit (pRect);
	while (NumPolyPoints--)
		 AddPointToRect (*pPoint++,pRect);
    GlobalUnlock (hPoly);
{
#if ENABLETRACE
GSSiExitProg (1156);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL RestoreFullWindowBitmap (void)   
{ 
	if (hFullWindowBitMap && (int)hFullWindowBitMap != -1)
	{
		HDC	hDC = GetDC (hWndMain);
    
		MergeImageIntoViewport(0, 0, 0, 0);
		SaveDC(hDC);
		SelectClipRgn (hDC,0);
		RestoreScreen (hDC,hFullWindowBitMap,FullWindowBitMapRect);	     
		ReleaseDC (hWndMain,hDC);
		RedisplayLastPrompt (); 
	    NotifyFunction ((LPVIEWPORT)-1,GF_REDRAW);  
		//DisplayAllToolbars  (1);
		BackgroundUpdateMessage ("!REDISPLAY!");
		RestoreDC (hDC,-1);
		GdiFlush ();
		return TRUE;
	}
	else
		return FALSE;
}  


BOOL SaveFullWindowBitmap (HWND hWnd)
#if ENABLETRACE
{GSSiEnterProg (1157);
#endif
{   
	HDIB	hDIB; 
	HDC		hDC; 
	LPVIEWPORT	SaveVP=CurView;
	
	if ((int)hFullWindowBitMap == -1)
		return FALSE;
	if (hWnd == (HWND)-1)
	{
		if (hFullWindowBitMap)
			return TRUE;
		hWnd = 0;
	}
	if (MapServer || InDisplayProcessing || !CurrentConfig || !CurView)// || HaveScreenBuffer (0)) 
// || IsIconic (hWnd))
{
#if ENABLETRACE
GSSiExitProg (1157);
#endif
		return FALSE;
}   
	GdiFlush ();
	
	SetViewport (*pCommandViewport);
	ClearFullWindowBitmap (hWnd);	
	if (!hWnd)
	{
		if (!*pNumViewports)
			return FALSE;
		NormalRect (&FullWindowBitMapRect); 
		if (CurView->hDC && !IsRectEmpty(&FullWindowBitMapRect))
			hFullWindowBitMap = SaveScreen (CurView->hDC, FullWindowBitMapRect);
		CurView = SaveVP;
		return TRUE; 
	}
	else if ((hDC = HaveScreenBuffer (&FullWindowBitMapRect)))
	{
		NormalRect (&FullWindowBitMapRect); 
		hFullWindowBitMap = SaveScreen (hDC, FullWindowBitMapRect);   
	}
	else if (!WindowIsCovered (hWnd,1))
	{    
		HDC	hDC = GetDC (hWnd);
		
		//GetWindowRect (hWnd,&FullWindowBitMapRect);
		GetClientRect (hWnd,&FullWindowBitMapRect);
		NormalRect (&FullWindowBitMapRect); 
		hFullWindowBitMap = SaveScreen (hDC, FullWindowBitMapRect);
		ReleaseDC (hWnd,hDC);
//		hDIB = BitmapToDIB (hFullWindowBitMap, 0);
//		SaveDIB (hDIB,"c:\\test.bmp");
//		DestroyDIB (hDIB);   
		CurView = SaveVP;
{
#if ENABLETRACE
GSSiExitProg (1157);
#endif
		return TRUE; 
}
	}
	else
		hFullWindowBitMap = 0;    
	CurView = SaveVP;
{
#if ENABLETRACE
GSSiExitProg (1157);
#endif
	return FALSE;
}
#if ENABLETRACE
}
#endif
}
 
BOOL DisplayPolyOff (void)
#if ENABLETRACE
{GSSiEnterProg (1158);
#endif
{
 DPOINT	BasePoint;
 BOOL	Cancel;
 LPTHEME	pTheme;
 UINT	npx, i, j;
 HPDPOINT	lpDpoint;
 HPPOINT	lpPoint, lpPoly;
 char	cCor[32], str[64];
 HANDLE	hPoly;
 HANDLE hPolyPartLen = 0;
 LPSTR	lpColon;
 HPEN	hPen, OldPen;
 OFSTRUCTGM OFStruct;
 HFILE	FidAO;
 LPSTR	pFile; 
 int	OffLineWidth, w;
 long	OffLineColor;
 int	NumEntries;
 short	Version;
 HIGHLIGHTAREAHEADER	Header;
 BOOL	showBorder=TRUE, doFill = FALSE;
    
   	if (!hAreaOffFile)
{
#if ENABLETRACE
GSSiExitProg (1158);
#endif
   		return FALSE;
}
	pFile = GlobalLock (hAreaOffFile); 
    if (!ExistFile(pFile))  
    {	
    	GlobalUnlock (hAreaOffFile);
{
#if ENABLETRACE
GSSiExitProg (1158);
#endif
    	return FALSE;               
}
    }
    FidAO = GSSiOpenFile (pFile,&OFStruct,OF_READ);
    if (FidAO == HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (1158);
#endif
    	return FALSE;
}
//	needs fixin
	BigRead (FidAO,&Version,2);
	BigRead (FidAO,&NumEntries,4);
    while (NumEntries-- && BigRead (FidAO,(HPSTR)&Header,sizeof(HIGHLIGHTAREAHEADER)) == sizeof(HIGHLIGHTAREAHEADER))
    {
    	long	Refno;
    	
	    hPoly = GSSiGlobAlloc (1251,GMEM_MOVEABLE,(Header.np+1)*sizeof(POINT));
	    lpPoint = (HPPOINT) GlobalLock (hPoly);
	    lpPoly = lpPoint;  
		if (Header.nPoly > 1)
			hPolyPartLen = GSSiGlobAlloc(1789, GMEM_MOVEABLE, Header.nPoly*sizeof(int));
		else
			hPolyPartLen = 0;
	    if (MaskOffsetLine && CurView->ID == Header.VPID)
	    {   
			LPMNMXCORD	lpRect; 
	    	
	    	ClearMaskArea ();
			CurView->NumMaskAreaParts = Header.nPoly - 1;
			CurView->hMaskArea = GSSiGlobAlloc(1252, GMEM_MOVEABLE, sizeof(MNMXCORD)+(long)Header.np*sizeof(DPOINT)+CurView->NumMaskAreaParts*sizeof(int));
		    CurView->NumMaskPoints = Header.np; 
		    CurView->MaskAreaRefno = Header.Refno;
		    lpRect = (LPMNMXCORD) GlobalLock (CurView->hMaskArea);
		    *lpRect = Header.Bounds; 
		    lpRect++;
		    lpDpoint = (LPDPOINT) lpRect;  
		    BigRead (FidAO,lpDpoint,Header.np*sizeof(DPOINT));
			for (i=0;i<Header.np;i++,lpPoint++,lpDpoint++)
				*lpPoint = BasePtToWinPt(lpDpoint);
			if (Header.nPoly > 1)
			{
				BigRead(FidAO, lpDpoint, Header.nPoly*sizeof(int));
			}
			GlobalUnlock(CurView->hMaskArea);
		}
		else if (CurView->ID != Header.VPID || !CurViewActive() ||!RectInWBounds (&Header.Bounds,1))
		{
			GSSillseek (FidAO,Header.np*sizeof(DPOINT),1);
			if (Header.nPoly > 1)
				GSSillseek (FidAO,Header.nPoly*sizeof(int),1);
			goto Next;
		}
		else
		{
			HANDLE	hPoints = GSSiGlobAlloc(0, GMEM_MOVEABLE, Header.np*sizeof(DPOINT));

			lpDpoint = (LPDPOINT)GlobalLock(hPoints);
			BigRead(FidAO, lpDpoint, Header.np*sizeof(DPOINT));
			for (i = 0; i<Header.np; i++, lpPoint++, lpDpoint++)
				*lpPoint = BasePtToWinPt(lpDpoint);
			GSSiGlobUlFree(&hPoints);
			if (Header.nPoly > 1)
			{
				LPINT pPolyPartLen = GlobalLock(hPolyPartLen);
				BigRead(FidAO, pPolyPartLen, Header.nPoly*sizeof(int));
				GlobalUnlock(hPolyPartLen);
			}
		}
		SetDisplayMode (CurView->hDC, GF_TEXTMODE);
		if (!FileMode)
		{
		    GSSiDeleteObject(&CurView->hRgn);
		    CurView->hRgn = CreateVPRgn(FALSE,FALSE);
		    SelectClipRgn (CurView->hDC,CurView->hRgn);
		    GSSiDeleteObject(&CurView->hRgn);  
		}
		OffLineWidth = GetGlobalDVal2 ("[%OFFSETLINEWIDTH]",6);
		if (OffLineWidth < 0)
			w = IDNINT(-OffLineWidth * BaseDistToWinDist);
		else if (OffLineWidth > 0)
			w = IDNINT(OffLineWidth*DeviceToScreenFactor);
		else
			w = IDNINT(Header.Offset * BaseDistToWinDist * 2);
		OffLineColor = GetGlobalLVal2 ("[%OFFSETLINECOLOR]",RGB(255,0,0));    
		if (OffLineColor >= 0)
		{
			HBITMAP		hbmp = LoadBitmap(hInst,MAKEINTRESOURCE(PatBMP[1]));
			LOGBRUSH	lb;
			int			OldMode = SetROP2(CurView->hDC,R2_MERGEPEN);  
			int			OldTextColor = SetTextColor (CurView->hDC,OffLineColor);     
			int			OldBKColor = SetBkColor (CurView->hDC,RGB(255,255,255));

			lb.lbStyle = BS_SOLID;
			lb.lbColor = OffLineColor;
			lb.lbHatch = (ULONG_PTR)hbmp;

			if (Header.Type == 3)
				hPen = ExtCreatePen (PS_GEOMETRIC|PS_SOLID|PS_ENDCAP_ROUND|PS_JOIN_ROUND,w,&lb,0,0);
			else
				hPen = ExtCreatePen (PS_GEOMETRIC|PS_SOLID|PS_ENDCAP_FLAT|PS_JOIN_ROUND,w,&lb,0,0);

		    OldPen = SelectObject (CurView->hDC,hPen); 
		    if (OutlineZoomArea && !ComputePCTTheme)
			{
				if (Header.Type == 3)//area
				{
					HBRUSH hBrush, hOldBrush;

					hBrush = CreatePatternBrush(hbmp);
					//hBrush = CreateSolidBrush (OffLineColor);
					hOldBrush = SelectObject (CurView->hDC,hBrush);
					//GWPolygonD(CurView->hDC, lpPoly, Header.np, Header.nPoly, hPolyPartLen, 0, showBorder,doFill, 0);
					if (Header.nPoly < 2)
					{
						i = Polygon(CurView->hDC, lpPoly, Header.np);
						SetROP2(CurView->hDC, OldMode);
						SelectObject(CurView->hDC, GetStockObject(NULL_BRUSH));
						i = Polygon(CurView->hDC, lpPoly, Header.np);
						SelectObject(CurView->hDC, hOldBrush);
						DeleteObject(hBrush);
					}
					else
					{
						LPINT pPartLen = GlobalLock(hPolyPartLen);

						SelectObject(CurView->hDC, GetStockObject(NULL_PEN));
						i = Polygon(CurView->hDC, lpPoly, Header.np);
						SelectObject(CurView->hDC, hPen);
						for (j = 0; j < 2; j++)
						{
							LPPOINT pPoint = lpPoly;
							for (i = 0; i < Header.nPoly; i++)
							{
								Polyline(CurView->hDC, pPoint, pPartLen[i]);
								pPoint += pPartLen[i];
								if (i)
									pPoint++;
							}
							SetROP2(CurView->hDC, OldMode);
						}
						GlobalUnlock(hPolyPartLen);
					}
				}
				else
					i=Polyline (CurView->hDC,lpPoly,Header.np);
			}
	        DeleteObject (hbmp); 
		    SelectObject (CurView->hDC,OldPen);
		    DeleteObject (hPen);
			SetROP2(CurView->hDC,OldMode);
			SetTextColor (CurView->hDC,OldTextColor);     
			SetBkColor (CurView->hDC,OldBKColor);
		} 
	Next:
	    GSSiGlobUlFree (&hPoly);
		GSSiGlobFree (&hPolyPartLen);
	}
	GSSiClose(FidAO);
   	GlobalUnlock (hAreaOffFile);
	ClearFullWindowBitmap (0);
{
#if ENABLETRACE
GSSiExitProg (1158);
#endif
	return TRUE; 
}
#if ENABLETRACE
}
#endif
}

BOOL HaveAreaOffFile (int Set)
{
	LPSTR	pFile;
	static	BOOL	HaveOffFile=FALSE;

	switch (Set)
	{
	case 0:
		return HaveOffFile;
	case 1:
		HaveOffFile = TRUE;
		break;
	case 2:
		HaveOffFile = FALSE;
		break;
	}
	return HaveOffFile;
}

void ClearPolyOff (BOOL KeepMask)
#if ENABLETRACE
{GSSiEnterProg (1159);
#endif
{
 LPSTR	pFile; 

	HaveAreaOffFile (2);
   	if (!KeepMask)
   		ClearMaskArea (); 
   	if (!hAreaOffFile)
{
#if ENABLETRACE
GSSiExitProg (1159);
#endif
   		return;
}
	pFile = GlobalLock (hAreaOffFile); 
    if (ExistFile(pFile))
    	GSSiRemove(pFile);
    GlobalUnlock (hAreaOffFile);
{
#if ENABLETRACE
GSSiExitProg (1159);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}

BOOL ChangeAreaOffset (double NewOffset)
{
	int		i;
	HFILE	FidAO;
	LPSTR	pFile;
	short	Version;
	int		NumEntries;
	HANDLE	hEntries;
	LPHAINDEX	pEntries;
	HIGHLIGHTAREAHEADER	Header;

	if (!HaveAreaOffFile(0))
		return FALSE;
	pFile = GlobalLock (hAreaOffFile);

	FidAO = GSSiOpenFile (pFile,NULL,OF_READWRITE);
   	GlobalUnlock (hAreaOffFile);
	if (FidAO == HFILE_ERROR)
    	return FALSE;
	BigRead (FidAO,&Version,2);
	BigRead (FidAO,&NumEntries,4);
	hEntries = GSSiGlobAlloc (0,GMEM_MOVEABLE,NumEntries*sizeof(HAINDEX));
	pEntries = GlobalLock (hEntries);
	GSSillseek (FidAO,-NumEntries*sizeof(HAINDEX),2);
	BigRead (FidAO,pEntries,NumEntries*sizeof(HAINDEX));
	for (i=0;i<NumEntries;i++)
	{
		GSSillseek (FidAO,pEntries[i].Offset,0);
		BigRead (FidAO,(HPSTR)&Header,sizeof(HIGHLIGHTAREAHEADER));
		Header.Offset = NewOffset;
		GSSillseek (FidAO,pEntries[i].Offset,0);
		BigWrite (FidAO,(HPSTR)&Header,sizeof(HIGHLIGHTAREAHEADER),-1);
	}
	GSSiGlobUlFree (&hEntries);
	GSSiClose (FidAO);
	return TRUE;
}

BOOL AddAreaToOffsetFile (long Refno,int Type,int np, HPDPOINT lpDPoint,int nPoly,LPINT pPolyPartLen,double Offset)
#if ENABLETRACE
{GSSiEnterProg (1160);
#endif
{
	MNMXCORD Rect; 
	HFILE	FidAO;
	OFSTRUCTGM	OFStruct;
	long	loc; 
	DWORD	i;  
	DPOINT	BasePoint;    
	LPSTR	pFile;
	short	Version=1;
	int		NumEntries;
	HANDLE	hIndex;
	LPHAINDEX	pIndex;
	HIGHLIGHTAREAHEADER Header;
	UINT	OpenOpt=OF_READWRITE;
	
	DBoundsInit (&Rect);
	if (!hAreaOffFile)
	{
		hAreaOffFile = GSSiGlobAlloc (1253,GMEM_MOVEABLE,256);
		pFile = GlobalLock (hAreaOffFile); 
		GSSiGetTempFileName (0,"gm",0,pFile);
	}
	else
		pFile = GlobalLock (hAreaOffFile); 
	if (AutoClearOffset)
    	GSSiRemove (pFile);
    FidAO = GSSiOpenFile(pFile,&OFStruct,OF_READWRITE);
    if (FidAO == HFILE_ERROR)
	{
    	FidAO = GSSiOpenFile(pFile,&OFStruct,OF_CREATE_NODELETE);
		NumEntries = 0;
		BigWrite (FidAO,&Version,2,-1);
		BigWrite (FidAO,&NumEntries,4,-1);
		hIndex = GSSiGlobAlloc (0,GMEM_MOVEABLE,(NumEntries+1)*sizeof(HAINDEX));
		pIndex = GlobalLock (hIndex);
		loc = 6;
	}
	else
	{
		BigRead (FidAO,&Version,2);
		BigRead (FidAO,&NumEntries,4);
		GSSillseek (FidAO,-NumEntries*sizeof(HAINDEX),2);
		hIndex = GSSiGlobAlloc (0,GMEM_MOVEABLE,(NumEntries+1)*sizeof(HAINDEX));
		pIndex = GlobalLock (hIndex);
		BigRead (FidAO,pIndex,NumEntries*sizeof(HAINDEX));
		loc = GSSillseek (FidAO,-NumEntries*sizeof(HAINDEX),2);
	}
    pIndex[NumEntries].VPID = CurView->ID;
	pIndex[NumEntries].Offset = loc;
	Header.np = np;
	GetPolyBoundsD2 (lpDPoint,np,&Header.Bounds,Type);
	Header.VPID = CurView->ID;
	Header.Refno = Refno;
	Header.nPoly = nPoly;
	Header.Offset = Offset;
	Header.Type = Type;
	NumEntries++;
	BigWrite (FidAO,&Header,sizeof(HIGHLIGHTAREAHEADER),-1);
	BigWrite (FidAO,lpDPoint,Header.np*sizeof(DPOINT),-1);
	if (Header.nPoly > 1)
		BigWrite (FidAO,pPolyPartLen,Header.nPoly*sizeof(int),-1);
	BigWrite (FidAO,pIndex,NumEntries*sizeof(HAINDEX),-1);
	GSSillseek (FidAO,2,0);
	BigWrite (FidAO,&NumEntries,4,-1);
	GSSiGlobUlFree (&hIndex);
    GSSiClose(FidAO); 
    GlobalUnlock (hAreaOffFile);
	HaveAreaOffFile (1);
{
#if ENABLETRACE
GSSiExitProg (1160);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}


void ShowZoomBox (HDC hDC,POINT StartPoint,POINT LastPoint, BOOL *HaveBox, BOOL Clip,short Function)
#if ENABLETRACE
{GSSiEnterProg (1162);
#endif
{   HPEN        hWidePen, hOldPen;
    short     OldMode, width=3;
    
	SaveDC (CurView->hDC);
	SetDisplayMode (CurView->hDC,GF_SCREENMODE);
    if (!FileMode && Clip)
    {
	    GSSiDeleteObject(&CurView->hRgn);
        CurView->hRgn = CreateVPRgn(FALSE,FALSE);
        SelectClipRgn (CurView->hDC,CurView->hRgn);
        GSSiDeleteObject(&CurView->hRgn);
    }  
    else
    	SelectClipRgn (CurView->hDC,0);
    if (Function == GF_ZOOM_VARRECT)
    	width = 0;
    hWidePen = CreatePen (PS_SOLID,width,RGB(0,0,0));
    OldMode = SetROP2(hDC,R2_NOT);
    hOldPen = SelectObject (hDC,hWidePen);
    if (*HaveBox) Polyline (hDC,(LPPOINT)&ZBPoints,5);
    ZBPoints[0].x=StartPoint.x;
    ZBPoints[0].y=StartPoint.y;
    ZBPoints[1].x=StartPoint.x;
    ZBPoints[1].y=LastPoint.y;
    ZBPoints[2].x=LastPoint.x;
    ZBPoints[2].y=LastPoint.y;
    ZBPoints[3].x=LastPoint.x;
    ZBPoints[3].y=StartPoint.y;
    ZBPoints[4]=ZBPoints[0];
    Polyline (hDC,(LPPOINT)&ZBPoints,5);
    *HaveBox = TRUE;
    SelectObject (hDC,hOldPen);
    SetROP2(hDC,OldMode);
    DeleteObject (hWidePen);
	RestoreDC (CurView->hDC,-1);
{
#if ENABLETRACE
GSSiExitProg (1162);
#endif
    return;
}

#if ENABLETRACE
}
#endif
}

void ShowZoomBoxClear (HDC hDC, BOOL *HaveBox)
#if ENABLETRACE
{GSSiEnterProg (1163);
#endif
{   HPEN        hWidePen, hOldPen;
    short     OldMode;

    hWidePen = CreatePen (PS_SOLID,3,RGB(0,0,0));
    OldMode = SetROP2(hDC,R2_NOT);
    hOldPen = SelectObject (hDC,hWidePen);
    if (*HaveBox) Polyline (hDC,(LPPOINT)&ZBPoints,5);
    *HaveBox = FALSE;
    SelectObject (hDC,hOldPen);
    SetROP2(hDC,OldMode);
    DeleteObject (hWidePen);
{
#if ENABLETRACE
GSSiExitProg (1163);
#endif
    return;
}

#if ENABLETRACE
}
#endif
}

short PickListAdd (short FileNum,short SubFile, USHORT FileInIndex,long CurrentSeg,
				  long CurrentRefno,short CurrentDesc,int PolyID,
				  int InType, double PCT,double OffDist,LPDOUBLE AZ, double Length,
				  WORD CurrentItem,WORD CurrentElement,
				  DPOINT BeginPoint, DPOINT EndPoint,DPOINT PickedPoint,DPOINT NodePoint,
				  long NearPoint,double Area, LPMNMXCORD Rect,
				  POINT BeginPointFile, POINT EndPointFile, POINT PickedPointFile, long NumPoints, float Elev,
				  LPDPOINT	FromPoint, LPDPOINT ToPoint)
#if ENABLETRACE
{GSSiEnterProg (1164);
#endif
{	short  i,j; 
	int	Type=PickTypeFromSysType (abs(InType));
	BOOL	HiPrecis = TRUE;
	
	if (InType < 0)
		HiPrecis = FALSE;
    if (CurrentRefno == DoNotPickThisRefno)
{
#if ENABLETRACE
GSSiExitProg (1164);
#endif
    	return 0;  
}
/*    if (PickOnlyEndPoints)
    {   
    	double	MinDist, d; 
    	
    	MinDist = ldistp (CurPickPoint,BeginPoint);
    	d = ldistp (CurPickPoint,EndPoint);
    	if (d < MinDist)
    	{
    		MinDist = d;
    		PickedPoint = EndPoint;
    	}
    	else
    		PickedPoint = BeginPoint;
    	if (MinDist > PickApW)
{
#if ENABLETRACE
GSSiExitProg (1164);
#endif
    		return 0; 
}
    	OffDist = -MinDist;
    }*/	
    if (PickOnlyNodePoints)
    {   
    	double	MinDist, d; 
    	
    	MinDist = ldistp (CurPickPoint,NodePoint);
    	if (MinDist > PickApW)
{
#if ENABLETRACE
GSSiExitProg (1164);
#endif
    		return 0; 
}
    	OffDist = -MinDist; 
    	PickedPoint = NodePoint;
    }	
    if (PickOnlyHighlighted)
    {
		
		if (!BT_FIND (hHighlight,(LPSTR)&CurrentRefno,BT_FIRST,BT_EQ,0))
		{
			if (PickOnlyHighlighted == 2)
{
#if ENABLETRACE
GSSiExitProg (1164);
#endif
				return 0;
}
		}
		else
		{
			if (PickOnlyHighlighted == 1)
{
#if ENABLETRACE
GSSiExitProg (1164);
#endif
				return 0;
}
		}
    }
    NumPicked = max (NumPicked,0); 
Sort:
	for (i=NumPicked-1;i>=0;i--)
	{   
		if (CurrentRefno == PickList[i].Refno)   // 12/7/99   
		{ 
			if (Type == 4)
				PickList[i].HasText = TRUE;  
			if (fabs(OffDist)>=fabs(PickList[i].OffDist))
{
#if ENABLETRACE
GSSiExitProg (1164);
#endif
				return 0; 
}     
			if (i+1 < NumPicked) 
			{
				for (j=i;j<NumPicked;j++)
				{
					PickList[j]=PickList[j+1];
					_fmemmove (PickedStreets[j],PickedStreets[j+1],28);
				}
			}
			NumPicked--; 
			goto Sort; 
		}
		if (OffDist == 9999999.0 && PickList[i].OffDist == 9999999.0 && fabs(Area) > fabs(PickList[i].Area))
			goto Next;
		if (fabs(OffDist)==fabs(PickList[i].OffDist) &&
			Length < PickList[i].Length) goto Insert; //allows segs 1 file coord long to be picked
		if (fabs(OffDist)<=fabs(PickList[i].OffDist)) goto Insert;  
Next:;
	}
	 
	if (NumPicked >= MaxPick)
{
#if ENABLETRACE
GSSiExitProg (1164);
#endif
		return 0; 
}
Insert: 
	if (NumPicked >= MaxPick) 
	{
		for (j=0;j<i;j++)
		{
			PickList[j]=PickList[j+1];
			_fmemmove (PickedStreets[j],PickedStreets[j+1],28);
		}
	}
	else
	{
		i++;
		for (j=NumPicked;i<j;j--)
		{
			PickList[j]=PickList[j-1];
			_fmemmove (PickedStreets[j],PickedStreets[j-1],28);
		}
	}
	_fmemset (&PickList[i],0,sizeof(PICKDATA));
	PickList[i].ViewID = CurView->ID;  
	PickList[i].ConfigID = CurrentConfig;
	PickList[i].HiPrecis = HiPrecis;
	PickList[i].FileNum = FileNum;  
	PickList[i].SubFile = SubFile;
	PickList[i].FileInIndex = FileInIndex;
	PickList[i].Segment = CurrentSeg;
	PickList[i].Refno = CurrentRefno;
	PickList[i].MSLink = CurMSLink;
	PickList[i].Desc = CurrentDesc;  
	PickList[i].PolyID = PolyID;   
	PickList[i].Blocked = ItemIsBlocked;
	PickList[i].Type = Type;
	if (Type == 4)
		PickList[i].HasText = TRUE;  
	PickList[i].IsDeleted = ItemIsDeleted;
	PickList[i].PCT = PCT + 1000*PolyID;
	PickList[i].OffDist = OffDist;   
	PickList[i].BPAZ = AZ[0];
	PickList[i].PPAZ = AZ[1];
	PickList[i].EPAZ = AZ[2];
	PickList[i].Length = Length;
	PickList[i].Offset = CurrentItem; 
	PickList[i].Element = CurrentElement;
	PickList[i].BeginPoint = BeginPoint; 
	PickList[i].EndPoint = EndPoint;  
	PickList[i].NodePoint = NodePoint; 
	PickList[i].FromPoint = *FromPoint;
	PickList[i].ToPoint = *ToPoint; 
	PickList[i].NearPoint = NearPoint;  
	PickList[i].BeginPointFile = BeginPointFile; 
	PickList[i].EndPointFile = EndPointFile;  
	PickList[i].PickedPointFile = PickedPointFile;  
	PickList[i].PickedPoint = PickedPoint;  
	PickList[i].Area = Area;
	PickList[i].Rect = *Rect; 
//	FileBoundsToWBounds (&CurrentItemMinMax,&PickList[i].Rect); 
	PickList[i].NumPoints = NumPoints;  
	PickList[i].Elev = Elev;
	_fmemcpy (PickedStreets[i],CurStreetNumbers,sizeof(CurStreetNumbers));
	PickedStreets[i][4] = CurState;
	PickedStreets[i][5] = FromStreet;
	PickedStreets[i][6] = ToStreet;
	GetVal ("%PREFIX",PickList[i].Prefix);
	GetVal ("%UDI",PickList[i].UDI); 
	{  //dbug
		PICKDATA PD=PickList[i];
		short	ii=1;
	}
	if (NumPicked < MaxPick)
		NumPicked++; 
{
#if ENABLETRACE
GSSiExitProg (1164);
#endif
	return i+1;
}
#if ENABLETRACE
}
#endif
}

void AddSavedPolys (void)
#if ENABLETRACE
{GSSiEnterProg (1165);
#endif
{
	LPSAVEPOLY pSavedPolys;

	if (!hSavedPolys)
	{
		NumSavedPolys = 1;
		hSavedPolys = GSSiGlobAlloc (1254,GHND,sizeof(SAVEPOLY));
	}
	else if (NumSavedPolys < 4096)
	{
		NumSavedPolys++;
		hSavedPolys = GSSiGlobalReAlloc (0,hSavedPolys,NumSavedPolys*sizeof(SAVEPOLY),GHND);
	}
	else
		return;
	pSavedPolys = (LPSAVEPOLY)GlobalLock (hSavedPolys);
	pSavedPolys += (NumSavedPolys-1);
	pSavedPolys->hSavePoly = hSavePoly; 
	hSavePoly = 0;
	pSavedPolys->hSavePolyParts = hSavePolyParts;
	hSavePolyParts = 0;
	pSavedPolys->nSavePoly = nSavePoly;
	nSavePoly = 0;
	pSavedPolys->hSavePolyElev = hSavePolyElev;
	hSavePolyElev = 0;
	pSavedPolys->Type = SavePolyType;
	GlobalUnlock (hSavedPolys);
{
#if ENABLETRACE
GSSiExitProg (1165);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}
void CreateCursors (void)
#if ENABLETRACE
{GSSiEnterProg (1166);
#endif
{   
	DefaultCursor = LoadCursor(0, IDC_ARROW);
	hLockedCursor = LoadCursor (hInst,"CURSOR_LOCKED"); 
	hPickAPCursor = LoadCursor (hInst,"PICK_CURSOR_AP"); 
	hPickNearCursor = LoadCursor (hInst,"PICK_CURSOR_NEAR"); 
	hDrawingCursor = LoadCursor (hInst,"DRAWING"); 
//	hDigCursor = LoadCursor (hInst,"DIGITIZE_CURSOR");
	hDigCursor = 0; 
	hWaitCursor = LoadCursor (0,IDC_WAIT);
	IDc_SIZENWSE = LoadCursor (0,IDC_SIZENWSE);
	IDc_SIZE = LoadCursor (0,IDC_SIZEALL);
	IDc_SIZEWE = LoadCursor (0,IDC_SIZEWE);
	IDc_SIZENS = LoadCursor (0,IDC_SIZENS);
	IDc_SIZENESW = LoadCursor (0,IDC_SIZENESW);
	MoveCursors[0]=0;
	MoveCursors[1]=IDc_SIZE;
	MoveCursors[2]=IDc_SIZEWE;
	MoveCursors[3]=IDc_SIZENWSE;
	MoveCursors[4]=IDc_SIZENS;
	MoveCursors[5]=IDc_SIZENESW;
	MoveCursors[6]=IDc_SIZEWE;
	MoveCursors[7]=IDc_SIZENWSE;
	MoveCursors[8]=IDc_SIZENS;
	MoveCursors[9]=IDc_SIZENESW;
{
#if ENABLETRACE
GSSiExitProg (1166);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

void DestroyCursors (void)
#if ENABLETRACE
{GSSiEnterProg (1167);
#endif
{   
	GSSiSetCursor (LoadCursor(0,IDC_ARROW));
	DestroyCursor (hLockedCursor);
	DestroyCursor (hPickAPCursor);
	DestroyCursor (hPickNearCursor);
	DestroyCursor (hDrawingCursor);
	if (hDigCursor) 
	{
		short ii=DestroyCursor (hDigCursor);
		if (!ii)
			ii=1;
	}
{
#if ENABLETRACE
GSSiExitProg (1167);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}  

void SetMoveCursor (int Mode)
#if ENABLETRACE
{GSSiEnterProg (1168);
#endif
{   
	SetCurs (MoveCursors[Mode],TRUE);
{
#if ENABLETRACE
GSSiExitProg (1168);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}  

BOOL GetFontFileName (LPSTR FontName,LPSTR FileName)
#if ENABLETRACE
{GSSiEnterProg (1169);
#endif
{   
	HFILE	Fid;
	OFSTRUCTGM	OFStruct;
	char	str[130];
	LPSTR	pSC;
	
	*FileName = 0;
	Fid = GSSiOpenFile ("[%DL]fonts.txt",&OFStruct,OF_READ);
	if (Fid != HFILE_ERROR)
	{   
		while (fgetstring (str,128,Fid))
		{
			if ((pSC = _fstrchr (str,';')))
			{
				*pSC++ = 0;
				if (!_fstricmp (FontName,str))
				{
					_fstrcpy (FileName,pSC);
					break;
				} 
			}
		}
		GSSiClose (Fid);
		if (*FileName)
{
#if ENABLETRACE
GSSiExitProg (1169);
#endif
			return TRUE;
}
	}
	sprintf (FileName,"%s.TTF",FontName);
{
#if ENABLETRACE
GSSiExitProg (1169);
#endif
	return FALSE;
}
#if ENABLETRACE
}
#endif
}

BOOL LoadBlockingPoints (LPSTR File)
#if ENABLETRACE
{GSSiEnterProg (1170);
#endif
{    
	LPBLOCKINGPOINT	pBlockingPoints;
	HFILE	Fid;
	char	str[130];  
	BOOL	rtn=TRUE;
	
	nBlockingPoints = 0;
	GSSiGlobFree (&hBlockingPoints);
	if (!File)
{
#if ENABLETRACE
GSSiExitProg (1170);
#endif
		return TRUE;
}
	if (!*File)
{
#if ENABLETRACE
GSSiExitProg (1170);
#endif
		return TRUE;  
}
	if ((Fid = GSSiOpenFile (File,0,OF_READ)) == HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (1170);
#endif
		return FALSE;
}
	hBlockingPoints = GSSiGlobAlloc (1255,GMEM_MOVEABLE,4096);
	pBlockingPoints = (LPBLOCKINGPOINT)GlobalLock (hBlockingPoints);
	while (fgetstring (str,128,Fid)) 
	{   
		if (sscanf (str,"%Flf %Flf %Flf",&pBlockingPoints->Point.x,&pBlockingPoints->Point.y,&pBlockingPoints->Dist) != 3)
			rtn = FALSE;
		nBlockingPoints++;
	}
	GlobalUnlock (hBlockingPoints);
	hBlockingPoints = GSSiGlobalReAlloc (0,hBlockingPoints,sizeof(BLOCKINGPOINT)*nBlockingPoints,GMEM_MOVEABLE);
{
#if ENABLETRACE
GSSiExitProg (1170);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}
 
BOOL PointIsBlocked (LPDPOINT pPoint,short desc)
#if ENABLETRACE
{GSSiEnterProg (1171);
#endif
{   
	LPBLOCKINGPOINT	pBlockingPoints;  
	USHORT	i;
	
	BOOL	rtn = FALSE;
	if (!hBlockingPoints)
{
#if ENABLETRACE
GSSiExitProg (1171);
#endif
		return rtn;
}
	pBlockingPoints = (LPBLOCKINGPOINT)GlobalLock (hBlockingPoints); 
	for (i=0;i<nBlockingPoints;i++,pBlockingPoints++)
		if (ldistp (*pPoint,pBlockingPoints->Point) < pBlockingPoints->Dist)
		{
			rtn = TRUE;
			break;
		}
	GlobalUnlock (hBlockingPoints); 
{
#if ENABLETRACE
GSSiExitProg (1171);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}  

double GetAreaAZ (long nPnts,HANDLE hAreaPoints)
#if ENABLETRACE
{GSSiEnterProg (1172);
#endif
{
	DPOINT		MidPt, DPoint;
	HPDPOINT	pPoints=(HPDPOINT)GlobalLock (hAreaPoints);
	HANDLE		hAZ = GSSiGlobAlloc (1256,GMEM_MOVEABLE,nPnts*sizeof(DPOINT));
	HANDLE		hDist = GSSiGlobAlloc (1257,GMEM_MOVEABLE,nPnts*sizeof(double));
	HPDOUBLE	pAZ=(HPDOUBLE)GlobalLock (hAZ);
	HPDOUBLE	pDist=(HPDOUBLE)GlobalLock (hDist);
	MNMXCORD 	Bounds;  
	double		MinSize, MinSizeAngle=0, xdist, ydist,azinc, Size;
	long		i, deg, j;
	
	DBoundsInit (&Bounds);
	MidPt = AverageDPoints (pPoints,nPnts); 
	for (i=0;i<nPnts;i++)
	{   
		AddDPointToMinMax (&pPoints[i],&Bounds);
		pAZ[i] = getazd (&MidPt,&pPoints[i]);
		pDist[i] = ldistp (MidPt,pPoints[i]);
	}
	ydist = LDIST (Bounds.xmn,Bounds.ymn,Bounds.xmn,Bounds.ymx);
	xdist = LDIST (Bounds.xmn,Bounds.ymn,Bounds.xmx,Bounds.ymn);
	MinSize = xdist * ydist;
	if (xdist < ydist)
		MinSize = -MinSize;
	for (deg = 10;deg <= 90;deg+=10)
	{
		for (j=-1;j<2;j+=2)
		{
			DBoundsInit (&Bounds);
			azinc = deg * RADDEG * j;
			for (i=0;i<nPnts;i++)
			{   
				DPoint = dnewpt (MidPt,pAZ[i]+azinc,pDist[i]);
				AddDPointToMinMax (&DPoint,&Bounds);
			}
			ydist = LDIST (Bounds.xmn,Bounds.ymn,Bounds.xmn,Bounds.ymx);
			xdist = LDIST (Bounds.xmn,Bounds.ymn,Bounds.xmx,Bounds.ymn);
			Size = xdist * ydist;
			if (xdist < ydist)
				Size = -Size;
			if (fabs (Size) < fabs (MinSize))
			{
				MinSize = Size;
				MinSizeAngle = azinc;
			}
		}
	}
	GlobalUnlock (hAreaPoints);
	GSSiGlobUlFree (&hAZ);
	GSSiGlobUlFree (&hDist);
	if (MinSize > 0)
{
#if ENABLETRACE
GSSiExitProg (1172);
#endif
		return MinSizeAngle;
}
	else
{
#if ENABLETRACE
GSSiExitProg (1172);
#endif
		return (MinSizeAngle+HALFPI);
}
#if ENABLETRACE
}
#endif
}

BOOL CreateSymbolPlot (LPSTR ParentName,LPSTR OutFile,LPSTR SymType)
#if ENABLETRACE
{GSSiEnterProg (1173);
#endif
{
    short	Parent = GetDictSymbolNumber (ParentName);   
    short	nChildren=0;
    HANDLE	hChildren=0;
    short	Type=1, i, ii;
	HANDLE	hGRText, hBT; 
	LPGRTEXT	lpGRText;  
	char	SortName[128], str[128];
	BTVARDESC	BTVar[2];   
	LPSHORT	pChild; 
	short	SymNum, pos=BT_FIRST;  
	long	Refno;
	HANDLE	hSymDesc=0;
	short	NumSyms=0, nPerRow, irow=0, isym=0, icol; 
	MNMXCORD	Bounds;
	BOOL	rtn=FALSE;  
	DPOINT	DP; 
	char	SymName[66];
    
    if (!Parent)
{
#if ENABLETRACE
GSSiExitProg (1173);
#endif
    	return FALSE;
}

	GSSiGetTempFileName (0,"gm",0,SortName); 
			
	BTVar[0].BT_VARTYP=BT_CHAR;
	BTVar[0].BT_VARLEN=64;
	BTVar[0].BT_VAROFF=0;
	BT_CREATE (SortName, 2, FALSE, 1, 1, (LPBTVARDESC)BTVar, FALSE, 0, 0, FALSE);
	hBT = BT_OPEN (SortName,0, BT_WRITE, 0);
	GetSymDictChildren (Parent,&nChildren,&hChildren,Type,TRUE);
	nPerRow = nChildren / 10; 

	pChild = (LPSHORT)GlobalLock (hChildren);
	for (i=0;i<nChildren;i++,pChild++)
	{  
		GetDictSymName (*pChild,SymName);
		if (_fstrlen (SymName) > 1)
			BT_PUT (hBT,SymName,(LPSTR)pChild);
	}
	GSSiGlobUlFree (&hChildren);
	hGRText = GSSiGlobAlloc (1258,GHND,sizeof(GRTEXT));
	lpGRText = (LPGRTEXT)GlobalLock (hGRText); 
	lpGRText->version = 1;    
	lpGRText->length = sizeof(GRTEXT);
    lpGRText->FontNum = 0;
    _fstrcpy (lpGRText->cHeight,"50M"); 
   	lpGRText->hJust = 1;
	lpGRText->vJust = 3;  
	GlobalUnlock (hGRText);
	
	Bounds.xmn = Bounds.ymn = 0;
	Bounds.xmx = (long)nPerRow * 500 + 500;
	Bounds.ymx = 5500;
	if (!CreateNewMap (OutFile,&Bounds,0,0,0,0,0,0,FALSE))
		goto Exit;  
	DP.x = 50;
	DP.y = 5450;
	while (isym < 10000 && !BT_FIND (hBT,SymName,pos,BT_ANY,(LPSTR)&SymNum))
	{   
		if (isym == 261)
			ii=1;
		sprintf (str,"%i %i:%s",isym,SymNum,SymName);
		SetWindowText (hWndMain,str);
		pos = BT_NEXT;
		if ((icol = isym++ % nPerRow)) 
			DP.x += 500;
		else
		{
			DP.x = 50;
			DP.y = 5450-irow++*500;
		}	
		AddToSymList (SymNum,&NumSyms,&hSymDesc); 
		lpGRText = (LPGRTEXT)GlobalLock (hGRText);
	    sprintf (lpGRText->Text,"\r\n\r\n%s",SymName); 
	    lpGRText->ltext = _fstrlen (lpGRText->Text)+1;
	    lpGRText->ltext += lpGRText->ltext % 2;  
	    GlobalUnlock (hGRText);
	    Refno = GetNewRefno(PltName,0,0,0,0);
	    AddPointToMap (DP,Refno,0,SymNum,50,0,0,hGRText,0,0,0,-1,-1,-1,TRUE,TRUE,0,0);
	}
	AddPointToMap (DP,0,0,0,0,0,0,0,0,0,0,0,0,0,TRUE,TRUE,0,0);
    CloseMap(TRUE);  
	AddSymToMap (NumSyms,hSymDesc,0,0); 
    DestroySymList (&NumSyms,&hSymDesc); 
    ForceRefIndex = ForceTAGIndex = FALSE;
	rtn = TRUE;
	 
Exit:
	BT_CLOSEANDDELETE (&hBT);
	GSSiGlobFree (&hGRText);
{
#if ENABLETRACE
GSSiExitProg (1173);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}

double SquareAZ (double AZ,short nWantAZ,LPDOUBLE WantAZ)
#if ENABLETRACE
{GSSiEnterProg (1174);
#endif
{
	short i, inear=0;
	double	dAZ,NearAZ=fabs(AZ-WantAZ[0]);    
	double	tol=WantAZ[1]/4;
	
	for (i=1;i<nWantAZ;i++)
	{ 
		dAZ = fabs(AZ - WantAZ[i]);
		if (dAZ < NearAZ)
		{
			NearAZ = dAZ;
			inear = i;
		}
	} 
	if (NearAZ < tol)
{
#if ENABLETRACE
GSSiExitProg (1174);
#endif
		return WantAZ[inear];
}
	else
{
#if ENABLETRACE
GSSiExitProg (1174);
#endif
		return AZ;
}
#if ENABLETRACE
}
#endif
}

BOOL MakePolySquare (short Type,LPLONG pnP,LPHANDLE phPoly)
#if ENABLETRACE
{GSSiEnterProg (1175);
#endif
{
	double	A1, A2, AZ=0, AZ45DEG=HALFPI/2; 
	double	WantAZ[9]; 
	HANDLE	hNewPoly = GSSiGlobAlloc (1259,GMEM_MOVEABLE,*pnP * sizeof(DPOINT));
	HPDPOINT	OrigPoints=(HPDPOINT)GlobalLock (*phPoly);
	HPDPOINT	NewPoints=(HPDPOINT)GlobalLock (hNewPoly);
	short	nWantAZ=9;
	short	IRC; 
	USHORT	i, ib, ie; 
	DPOINT	Point1, Point2;
	
	for (i=0;i<nWantAZ;i++,AZ+=AZ45DEG)
		WantAZ[i] = AZ;
	
	if (Type == 2)
	{
		NewPoints[0] = OrigPoints[0];
		ib = 1; 
		ie = *pnP - 1;  
		NewPoints[ie] = OrigPoints[ie];
		A1 = getazd (&OrigPoints[0],&OrigPoints[1]);
		Point1 = AverageDPoint (OrigPoints[0],OrigPoints[1]);
	}
	else
	{
		ib = 0;
		ie = *pnP;
		if (SameDPoint (&OrigPoints[ie-1],&OrigPoints[0]))
			(*pnP)--; 
		ie = *pnP;
		A1 = getazd (&OrigPoints[ie-1],&OrigPoints[0]);
		Point1 = AverageDPoint (OrigPoints[ie-1],OrigPoints[0]);
	}
	A1 = SquareAZ (A1,nWantAZ,WantAZ);
	for (i = ib; i<ie; i++)
	{   
		if (Type == 3 && i+1 == ie)
		{
			A2 = getazd (&OrigPoints[ie-1],&OrigPoints[0]); 
			Point2 = AverageDPoint (OrigPoints[ie-1],OrigPoints[0]);
		}
		else
		{
			A2 = getazd (&OrigPoints[i],&OrigPoints[i+1]); 
			Point2 = AverageDPoint (OrigPoints[i],OrigPoints[i+1]);
		}
		A2 = SquareAZ (A2,nWantAZ,WantAZ);
		IRC = LIN_SEC (Point1.x,Point1.y,A1,
					   Point2.x,Point2.y,A2,
					   &NewPoints[i].x, &NewPoints[i].y);
        if (IRC != 0)
        	NewPoints[i] = OrigPoints[i];   
        if (!i)
        	OrigPoints[ie] = NewPoints[ie] = NewPoints[0];   
        
        A1 = A2;
        Point1 = Point2; 
    }
    GSSiGlobUlFree (phPoly);
    GlobalUnlock (hNewPoly);
    *phPoly = hNewPoly;
{
#if ENABLETRACE
GSSiExitProg (1175);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL AdjustPolygon (short Opt,LPSTR PointList,double ReduceDist)
#if ENABLETRACE
{GSSiEnterProg (1176);
#endif
{   //opt=1	convert low precis to hi precis
	//opt=2 make square
	//opt=3 moves nearest polyline point to snap point and (opt) moves all other points same dist
	//opt=4 reverse
	//opt=5 New bp and ep in pointlist
	//opt=6 Reduces a line to 2 points if those 2 points are greater than total line dist - reducedist
	//opt=7 Thins a line by removeing segments less than ReduceDist
	//opt=8 Removes duplicate points
	short	pos=BT_FIRST;
	HIGHLIGHTDATA	HighlightData;
    LPTHEME pTheme;                                        
	long	iref,ii;  
	static	long debugref=-2145529886;
	short	nParts, nareas; 
	HANDLE	hIndex;
	LPLONG	pIndex;
	LPINT	pPolyParts;
	char	CRef[64]; 
	long	NumItems, Done=0;
	BOOL	DoUpdate=TRUE;
    HPDPOINT    lpDpoint, lpUpdatePolyPoints;
	HANDLE	hPoly;
	UINT	i;
	
	if (Opt == 5)
		goto SkipHltList;
	if (Opt < -100)
	{
		DoUpdate = FALSE;
		Opt += 100;
	} 
    NumItems = BT_NUM_IN_INDEX(hHighlight);  
    if (!NumItems)
{
#if ENABLETRACE
GSSiExitProg (1176);
#endif
    	return FALSE;
}
	if (Opt > 0)
	{
		CreateStatusWind (hWndMain,1,0);
		StatusWindowUpdate ("Convert to Hi-Precision",0, 0,0);
	}
	while (ContinueProcessing && !BT_FIND (hHighlight,(LPSTR)&iref,pos,BT_ANY,(LPSTR)&HighlightData))
	{   
//		ltoa (iref,CRef,10);
//		SetWindowText (hWndMain,CRef);  
		if (iref == debugref)
			ii=1;
		PickList[0]=HighlightData.PD; 
		if (DoUpdate)
		{
			RemoveFromHighlightList (iref,2);
		    ShowPickedItem (hWndMain,0); 
			RemoveFromHighlightList (iref,0); 
		}
		else
			pos = BT_NEXT; 
SkipHltList:
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
        	GlobalUnlock (hUpdatePoly);  
		}
		else
		{
			if (PickList[0].Type != 2 && PickList[0].Type != 3)
				continue;
		    SetConfig (PickList[0].ConfigID);
		    SetViewport (PickList[0].ViewID);
			pTheme = AddTheme (GF_SAVEPOLYPARTS_THEME);
			CurView->PassID = 4; 
			ProcessSelectedTheme = CurView->NumThemes;
			ProcessPickedItem (0,FALSE); 
			ProcessSelectedTheme = 0;
			WantElement = LONG_MAX;               
			DeleteTheme (pTheme);
			nParts = GetSavedPolys (); 
	//		if (hCurvePoints)
	//			continue;
		    if (hSavePoly)
		    {   LPMNMXCORD lpRect;
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
		        	hmemmove ((HPSTR)pMultiPoly,(HPSTR)pPolyParts,(long)nUpdateMultiPolygon*sizeof(int)); 
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
				DestroySavedPolys ();
				if (Opt == 5)
					goto Next;
	        }
	    }
Next:
    	switch (abs(Opt))
    	{
    		case 1:
	           	if (PickList[0].Type == 2)
	           		UpdateItem = -6;
	           	else
					UpdateItem = -5; 
				break;
			case 2: 
				MakePolySquare (PickList[0].Type,&nUpdatePolyPoints,&hUpdatePoly);
	           	if (PickList[0].Type == 2)
	           		UpdateItem = 6;
	           	else
					UpdateItem = 5; 
				if (PickList[0].HiPrecis)
					UpdateItem = UpdateItem * 10 + 1;
				break;   
			case 3: 
				HiPrecis = PickList[0].HiPrecis;
				lpDCurPoints = (HPDPOINT)GlobalLock (hUpdatePoly);
				nPnts = nUpdatePolyPoints; 
				if (PickList[0].Type == 3) 
				{
					while (nPnts>1 && SameDPoint (&lpDCurPoints[0], &lpDCurPoints[nPnts-1]))
						nPnts--; 
				}
				MovePolyLine (PolySnapPoint);
	           	if (PickList[0].Type == 3) 
	           	{   
	           		lpDCurPoints[nPnts] = lpDCurPoints[0];
	           		nPnts++; 
	           		nUpdatePolyPoints = nPnts;
	           		UpdateItem = 5;
	           	}
	           	else
					UpdateItem = 6; 
				if (PickList[0].HiPrecis)
					UpdateItem = UpdateItem * 10 + 1;
	           	if (PickList[0].Type == 5) 
	           		UpdateItem = 172;
	        	GlobalUnlock (hUpdatePoly); 
				break;   
			case 4: 
				hUpdatePoly = ReversePoints (nUpdatePolyPoints,hUpdatePoly);
	           	if (PickList[0].Type == 2)
	           		UpdateItem = 6;
	           	else
					UpdateItem = 5; 
				if (PickList[0].HiPrecis)
					UpdateItem = UpdateItem * 10 + 1;
				break;   
			case 5: 
				{
					HPDPOINT Points = GlobalLock (hUpdatePoly);
					HANDLE	hNewPoints;
					long	np = GetPointsFromList (PointList,&hNewPoints);

					if (np == 2)
					{
						HPDPOINT NewPoints = GlobalLock (hNewPoints);

						Points[0] = NewPoints[0];
						Points[nUpdatePolyPoints-1] = NewPoints[1];	
						GSSiGlobUlFree (&hNewPoints);
					}
					GlobalUnlock (hUpdatePoly);
	           		if (PickList[0].Type == 2)
	           			UpdateItem = 6;
	           		else
						UpdateItem = 5; 
					if (PickList[0].HiPrecis)
						UpdateItem = UpdateItem * 10 + 1;
				}
				break; 
			case 6:
				{
					HPDPOINT Points = (HPDPOINT)GlobalLock (hUpdatePoly);
					double LineLength = GetPolyLengthD (Points,nUpdatePolyPoints);
					BOOL	DoUp=FALSE;
					if (nUpdatePolyPoints > 2)
					{
						for (i=1;i<nUpdatePolyPoints;i++)
							if (ldistp (Points[i-1],Points[i]) > LineLength - ReduceDist)
							{
								DoUp = TRUE;
								Points[0] = Points[i-1];
								Points[1] = Points[i];
								nUpdatePolyPoints = 2;
	           					if (PickList[0].Type == 2)
	           						UpdateItem = 6;
	           					else
									UpdateItem = 5; 
								if (PickList[0].HiPrecis)
									UpdateItem = UpdateItem * 10 + 1;
								break;
							}
					}
					GlobalUnlock (hUpdatePoly);
					if (!DoUp)
						goto SkipUpdate;
				}
				break;
			case 7:
				{
					HPDPOINT Points = (HPDPOINT)GlobalLock (hUpdatePoly);
					long	 NumNewPoints = nUpdatePolyPoints;

					ThinPoly (&nUpdatePolyPoints, Points, ReduceDist);
					GlobalUnlock (hUpdatePoly);
					if (NumNewPoints == nUpdatePolyPoints)
						goto SkipUpdate;
	           		if (PickList[0].Type == 2)
	           			UpdateItem = 6;
	           		else
						UpdateItem = 5; 
					if (PickList[0].HiPrecis)
						UpdateItem = UpdateItem * 10 + 1;
				}
				break;
			case 8:
				{
					HPDPOINT Points = (HPDPOINT)GlobalLock (hUpdatePoly);
					long	 NumNewPoints = nUpdatePolyPoints;
			
					RemoveDupPolyPoints (&nUpdatePolyPoints, Points, ReduceDist);
					GlobalUnlock (hUpdatePoly);
					if (NumNewPoints == nUpdatePolyPoints)
						goto SkipUpdate;
	           		if (PickList[0].Type == 2)
	           			UpdateItem = 6;
	           		else
						UpdateItem = 5; 
					if (PickList[0].HiPrecis)
						UpdateItem = UpdateItem * 10 + 1;
				}
				break;
		} 
		if (DoUpdate)
			UpdateRecord (0,PickList[0].Desc,PickList[0].Prefix,PickList[0].UDI,0,0,1,0);
		else
		{ 
			int OldMode = SetROP2(CurView->hDC,R2_NOT); 
				
			lpDCurPoints = (HPDPOINT)GlobalLock (hUpdatePoly);
			TempPolylineD (CurView->hDC,lpDCurPoints,(short)nUpdatePolyPoints,0,0); 
			GlobalUnlock (hUpdatePoly);  
		    SetROP2(CurView->hDC,OldMode);   
		}
	    ShowPickedItem (hWndMain,0); 
SkipUpdate:
		GSSiGlobFree (&hUpdatePoly); 
		if (Opt > 0)
			StatusWindowUpdate (0,0, NumItems,Done++);
	}
	if (Opt > 0 && Opt!=5)
		DestroyStatusWindow(0); 
	ContinueProcessing = TRUE;   
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

void TempPolylineD (HDC hDC, HPDPOINT lpPoints, short nPnts, LPSTR TopText, LPSTR BottomText)
{
	HANDLE hPoints=GSSiGlobAlloc (0,GMEM_MOVEABLE,(long)nPnts*sizeof(DPOINT));
	HPPOINT	pPoints=(HPPOINT)GlobalLock (hPoints);
	USHORT	i;

    SaveDC (CurView->hDC);
	SetDisplayMode (CurView->hDC, GF_SCREENMODE); 
//    SetMapMode    (CurView->hDC, MM_TEXT );
    SelectClipRgn (CurView->hDC,0);
	for (i=0;i<nPnts;i++)
		pPoints[i] = BasePtToWinPt(&lpPoints[i]);
	TempPolyline (hDC,pPoints, nPnts,TopText,BottomText);
    GSSiGlobUlFree (&hPoints);                               
    RestoreDC (CurView->hDC,-1);
	return;
}

 

BOOL CreateBPW (LPSTR Name,	MNMXCORD MnMx, double Res)
#if ENABLETRACE
{GSSiEnterProg (1178);
#endif
{   
	HANDLE	hMem=GSSiGlobAlloc (1263,GMEM_MOVEABLE,512);
	LPSTR	BPWName=GlobalLock (hMem);
	LPSTR	str = BPWName+128;
	HFILE	FidBPW;
	HPSTR	lpDot;
	
	_fstrcpy (BPWName,Name);
	if ((lpDot=_fstrrchr(BPWName,'.')))
		*lpDot = 0;
	_fstrcat (BPWName,".BPW");
	makedirectories (BPWName,FALSE,FALSE);
	FidBPW = GSSiOpenFile (BPWName,0,OF_CREATE); 
	sprintf (str,"%.14lg",Res);
	fputstring (str,FidBPW);
	fputstring ("0",FidBPW);
	fputstring ("0",FidBPW);
	sprintf (str,"%.14lg",-Res);
	fputstring (str,FidBPW);
	sprintf (str,"%.14lg",MnMx.xmn);
	fputstring (str,FidBPW);
	sprintf (str,"%.14lg",MnMx.ymx);
	fputstring (str,FidBPW);
	GSSiClose (FidBPW);    
	GSSiGlobUlFree (&hMem);
{
#if ENABLETRACE
GSSiExitProg (1178);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
} 
BOOL CreateLLTranFile (LPSTR Name)
#if ENABLETRACE
{GSSiEnterProg (1178);
#endif
{   
	HANDLE	hMem=GSSiGlobAlloc (1263,GHND,512);
	LPSTR	BPWName=GlobalLock (hMem);
	LPSTR	str = BPWName+128;
	HFILE	Fid;
	HPSTR	lpDot;
	DPOINT	ScreenPointD[4];
	DPOINT	BasePointD[4];
	DPOINT	LLPointD[4];
	HANDLE	hTran;
	float	RSQMIN;
	DPOINT	testPointIn, testPointOut;
//#include "c:\temp\msptran.h"
	
	_fstrcpy (BPWName,Name);
	if ((lpDot=_fstrrchr(BPWName,'.')))
		*lpDot = 0;
	_fstrcat (BPWName,"Tran.h");
	Fid = GSSiOpenFile (BPWName,0,OF_CREATE); 

	ScreenPointD[0].x = 0;
	ScreenPointD[0].y = 0;	
	ScreenPointD[1].x = 0;
	ScreenPointD[1].y = MemMapHeight-1;
	ScreenPointD[2].x = MemMapWidth-1;
	ScreenPointD[2].y = MemMapHeight-1;
	ScreenPointD[3].x = MemMapWidth-1;
	ScreenPointD[3].y = 0;	

	BasePointD[0] = LLPointD[0] = ScreenPtDToBasePt (ScreenPointD[0]);
	BasePointD[1] = LLPointD[1] = ScreenPtDToBasePt (ScreenPointD[1]);
	BasePointD[2] = LLPointD[2] = ScreenPtDToBasePt (ScreenPointD[2]);
	BasePointD[3] = LLPointD[3] = ScreenPtDToBasePt (ScreenPointD[3]);

	ConvertCoord (&LLPointD[0],1,2);
	ConvertCoord (&LLPointD[1],1,2);
	ConvertCoord (&LLPointD[2],1,2);
	ConvertCoord (&LLPointD[3],1,2);

	ScreenPointD[0].x = 0;
	ScreenPointD[0].y = MemMapHeight-1;	
	ScreenPointD[1].x = 0;
	ScreenPointD[1].y = 0;
	ScreenPointD[2].x = MemMapWidth-1;
	ScreenPointD[2].y = 0;
	ScreenPointD[3].x = MemMapWidth-1;
	ScreenPointD[3].y = MemMapHeight-1;	
	hTran = STRANPoints (0,LLPointD,ScreenPointD,4,&RSQMIN,1,NULL); 
	WriteTranCHeader (hTran,Fid);
	testPointIn.x = -93.229222;
	testPointIn.y = 44.889645;
	testPointOut = TranPoint (&testPointIn,hTran);
	testPointIn.x = -93.243911;
	testPointIn.y = 44.887535;
	testPointOut = TranPoint (&testPointIn,hTran);
	testPointIn.x = -93.224591;
	testPointIn.y = 44.871252;
	testPointOut = TranPoint (&testPointIn,hTran);
	testPointIn.x = -93.204307;
	testPointIn.y = 44.893080;
	testPointOut = TranPoint (&testPointIn,hTran);
	CloseTRANS2 (&hTran);
	GSSiClose (Fid);    
	GSSiGlobUlFree (&hMem);
{
#if ENABLETRACE
GSSiExitProg (1178);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
} 

BOOL CreateBPWFromCPT (LPSTR ImageFile)
{   
	MNMXCORD MnMx;
	double Res;
	HDIB32	hDib;
	LPBITMAPINFOHEADER	pDibInfo;
	BOOL	rtn=FALSE;
	LPSTR	pDot;
	HANDLE	hTran;
	char	CPTFile[MAX_PATH], BPWFile[MAX_PATH];
	DPOINT	BMPPoint, BMPPoint1, WPoint, WPoint1;
	double	WDist, BMPDist;

	strcpy (CPTFile,ImageFile);
	if (!(pDot = strrchr (CPTFile,'.')))
		return FALSE;
	*pDot = 0;
	strcpy (BPWFile,CPTFile);
	strcat (CPTFile,".cpt");
	strcat (BPWFile,".bpw");
	if (!(hDib = LoadDIB32 (ImageFile,TRUE)))
		return rtn;
	pDibInfo = (LPBITMAPINFOHEADER) GetDibHeader (hDib);
	DestroyDIB32(hDib,FALSE);
	DBoundsInit (&MnMx);
	hTran = LoadTranFile(CPTFile,1,2,0,0);
	BMPPoint.x = 0;
	BMPPoint.y = 0;
	BMPPoint1 = BMPPoint;
	WPoint = WPoint1 = TranPoint (&BMPPoint,hTran);
	AddDPointToMinMax (&WPoint,&MnMx);
	BMPPoint.x = pDibInfo->biWidth;
	BMPPoint.y = 0;
	WPoint = TranPoint (&BMPPoint,hTran);
	AddDPointToMinMax (&WPoint,&MnMx);
	BMPPoint.x = 0;
	BMPPoint.y = pDibInfo->biHeight;
	WPoint = TranPoint (&BMPPoint,hTran);
	AddDPointToMinMax (&WPoint,&MnMx);
	BMPPoint.x = pDibInfo->biWidth;
	BMPPoint.y = pDibInfo->biHeight;
	WPoint = TranPoint (&BMPPoint,hTran);
	AddDPointToMinMax (&WPoint,&MnMx);
	BMPDist = ldistp (BMPPoint1,BMPPoint);
	WDist = ldistp (WPoint1,WPoint);
	Res = WDist / BMPDist;
	rtn = CreateBPW (BPWFile,MnMx,Res);
	return rtn;
}
BOOL CreateBPWFromViewport (LPSTR OutFile)
{   
	MNMXCORD MnMx;
	double Res;
	BOOL	rtn=FALSE;
	LPSTR	pDot;
	HANDLE	hTran;
	POINT	BMPPoint, BMPPoint1;
	DPOINT  WPoint, WPoint1;
	double	WDist, BMPDist;

	DBoundsInit (&MnMx);
	BMPPoint.x = CurView->DrawRect.left;
	BMPPoint.y = CurView->DrawRect.bottom;
	BMPPoint1 = BMPPoint;
	WPoint = WPoint1 = ScreenPtToBasePt (BMPPoint);
	AddDPointToMinMax (&WPoint,&MnMx);
	BMPPoint.x = CurView->DrawRect.left;
	BMPPoint.y = CurView->DrawRect.top;
	WPoint = ScreenPtToBasePt (BMPPoint);
	AddDPointToMinMax (&WPoint,&MnMx);
	BMPPoint.x = CurView->DrawRect.right;
	BMPPoint.y = CurView->DrawRect.bottom;
	WPoint = ScreenPtToBasePt (BMPPoint);
	AddDPointToMinMax (&WPoint,&MnMx);
	BMPPoint.x = CurView->DrawRect.right;
	BMPPoint.y = CurView->DrawRect.top;
	WPoint = ScreenPtToBasePt (BMPPoint);
	AddDPointToMinMax (&WPoint,&MnMx);
	BMPDist = idist (BMPPoint1,BMPPoint);
	WDist = ldistp (WPoint1,WPoint);
	Res = WDist / BMPDist;
	rtn = CreateBPW (OutFile,MnMx,Res);
	return rtn;
}

long GetNextRefno (LPLONG StartRefno,long UniqueRefno,BOOL Update)
																							#if ENABLETRACE
																							{GSSiEnterProg (1179);
																							#endif
{    
	long	Refno;
	
	if (UniqueRefno)
	{
		BOOL	Opened; 
		long 	EndRef, BeginRef=*StartRefno;
	    
	    if (UsesUsedRef ())
	    {
			if (!OpenUsedRefTable (TRUE,&Opened))
			{
																								{
																								#if ENABLETRACE
																								GSSiExitProg (1179);
																								#endif
				return 0;
																								}
			}
			if (BT_FIND (hURT,(LPSTR)&BeginRef,BT_FIRST,BT_GE,(LPSTR)&EndRef))
			{ 
				if (BT_FIND (hURT,(LPSTR)&BeginRef,BT_LAST,BT_ANY,(LPSTR)&EndRef))
					Refno = *StartRefno;
				else
				{
					if (EndRef >= *StartRefno)
						Refno = EndRef + 1;
					else
						Refno = *StartRefno;
				}
			}
			else if (BeginRef <= *StartRefno)
				Refno = EndRef+1;             
			else 
			{
				if (BT_FIND (hURT,(LPSTR)&BeginRef,BT_PRIOR,BT_ANY,(LPSTR)&EndRef))
					Refno = *StartRefno;
				else if (EndRef >= *StartRefno)
					Refno = EndRef + 1;
				else
					Refno = *StartRefno;
			}
					
			if (Update)
				AddRefToUsedRefTable (Refno);
			CloseUsedRefTable (Opened);
		}
		else        
		{
			Refno = GetNewRefno(PltName,0,0,0,0); 
		}
	}
	else
		Refno = (*StartRefno)++;
	SetIntRefno (Refno);
																							{
																							#if ENABLETRACE
																							GSSiExitProg (1179);
																							#endif
	return Refno;
																							}
																							#if ENABLETRACE
																							}
																							#endif
}

BOOL RemoveRefno (long NewRefno,LPLONG StartRefno,BOOL UniqueRefno)
#if ENABLETRACE
{GSSiEnterProg (1180);
#endif
{
{
#if ENABLETRACE
GSSiExitProg (1180);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL AddRefToUsedRefTable (long Refno)
#if ENABLETRACE
{GSSiEnterProg (1181);
#endif
{   
	BOOL	rtn=FALSE, Opened; 
	long 	EndRef, PrevEndRef, NextEndRef, StartRef=Refno, NextRef=Refno;
	
	if (!OpenUsedRefTable (TRUE,&Opened))
{
#if ENABLETRACE
GSSiExitProg (1181);
#endif
		return FALSE;
}
	if (BT_FIND (hURT,(LPSTR)&NextRef,BT_FIRST,BT_GE,(LPSTR)&NextEndRef)) 
	{    
		NextRef = LONG_MAX;
		if (BT_FIND (hURT,(LPSTR)&StartRef,BT_LAST,BT_ANY,(LPSTR)&PrevEndRef))
			goto AddIt; 
	}
	else if (NextRef > Refno)
	{
		if (BT_FIND (hURT,(LPSTR)&StartRef,BT_PRIOR,BT_GE,(LPSTR)&PrevEndRef))
			goto AddIt;
	}
	else
		goto Exit;
	if (PrevEndRef >= Refno)
		goto Exit;
	if (Refno == PrevEndRef+1)
	{   
		rtn = TRUE;  
		if (NextRef == Refno+1)
		{
			Refno = NextEndRef;
			BT_DELETE (hURT,(LPSTR)&NextRef,(LPSTR)&NextEndRef,FALSE);
		}
		BT_PUT (hURT,(LPSTR)&StartRef,(LPSTR)&Refno);
		goto Exit;
	}
AddIt:
	rtn = TRUE; 
	EndRef = Refno;
	if (NextRef == Refno+1)
	{
		EndRef = NextEndRef;
		BT_DELETE (hURT,(LPSTR)&NextRef,(LPSTR)&NextEndRef,FALSE);
	}
	BT_PUT (hURT,(LPSTR)&Refno,(LPSTR)&EndRef);   
Exit:
	CloseUsedRefTable (Opened);
{
#if ENABLETRACE
GSSiExitProg (1181);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}

BOOL AddRangeToUsedRefTable (long StartRefno, long EndRefno)
#if ENABLETRACE
{GSSiEnterProg (1181);
#endif
{   
	BOOL	rtn=FALSE, Opened; 
	long 	EndRef, NextEndRef, NextRef=StartRefno;
	
	if (!OpenUsedRefTable (TRUE,&Opened))
{
#if ENABLETRACE
GSSiExitProg (1181);
#endif
		return FALSE;
}
	if (!BT_FIND (hURT,(LPSTR)&NextRef,BT_FIRST,BT_GE,(LPSTR)&NextEndRef)) 
	{    
		if (NextRef <= EndRefno)
			goto Exit;
	}
	rtn = TRUE; 
	BT_PUT (hURT,(LPSTR)&StartRefno,(LPSTR)&EndRefno);   
Exit:
	CloseUsedRefTable (Opened);
{
#if ENABLETRACE
GSSiExitProg (1181);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}

BOOL RefInUsedRefTable (long Refno)
#if ENABLETRACE
{GSSiEnterProg (1182);
#endif
{
	BOOL	rtn=FALSE, Opened; 
	long 	NextEndRef, PrevEndRef, NextRef=Refno, StartRef;
	
	if (!OpenUsedRefTable (FALSE,&Opened))
{
#if ENABLETRACE
GSSiExitProg (1182);
#endif
		return FALSE;
}
	if (BT_FIND (hURT,(LPSTR)&NextRef,BT_FIRST,BT_GE,(LPSTR)&NextEndRef)) 
	{    
		if (BT_FIND (hURT,(LPSTR)&StartRef,BT_LAST,BT_ANY,(LPSTR)&PrevEndRef))
			goto Exit; 
	}
	else if (NextRef > Refno)
	{
		if (BT_FIND (hURT,(LPSTR)&StartRef,BT_PRIOR,BT_GE,(LPSTR)&PrevEndRef))
			goto Exit;
	}
	else
	{
		rtn = TRUE;  
		goto Exit;
	}
	if (PrevEndRef >= Refno)
		rtn = TRUE;
Exit:
	CloseUsedRefTable (Opened);
{
#if ENABLETRACE
GSSiExitProg (1182);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}

BOOL DumpUsedRefTable (LPSTR File)
#if ENABLETRACE
{GSSiEnterProg (1183);
#endif
{
	BOOL	Opened; 
	short	pos=BT_FIRST;
	long 	NextEndRef, NextRef;
	char	str[32];
	HFILE	Fid;
	
	if (!OpenUsedRefTable (FALSE,&Opened))
{
#if ENABLETRACE
GSSiExitProg (1183);
#endif
		return FALSE;  
}
	Fid = GSSiOpenFile (File,0,OF_CREATE);
	while (!BT_FIND (hURT,(LPSTR)&NextRef,pos,BT_ANY,(LPSTR)&NextEndRef)) 
	{   
		pos = BT_NEXT;
		sprintf (str,"%12ld %12ld %ld",NextRef,NextEndRef,NextEndRef-NextRef+1);
		fputstring (str,Fid);
	}
	GSSiClose (Fid);
	CloseUsedRefTable (Opened);
{
#if ENABLETRACE
GSSiExitProg (1183);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
} 

BOOL OpenUsedRefTable (BOOL Update,LPBOOL Opened)
																							#if ENABLETRACE
																							{GSSiEnterProg (1184);
																							#endif
{   
    HANDLE	hMem=GSSiGlobAlloc (1264,GMEM_MOVEABLE,256);
    LPSTR	File = GlobalLock (hMem);
    static  BOOL    OpenMode=0;  
	BOOL	SaveShareEnabled = ShareEnabled, SaveKFO = KeepFilesOpen, SaveUndoEnabled = UndoEnabled;
    BOOL	SaveAllowCache=AllowCache;
    BOOL	rtn=TRUE; 
    short   Mode=BT_READ;
    
    AllowCache = FALSE; 
    UndoEnabled = FALSE;   
	CloseAllRequestedFiles (TRUE);
    KeepFilesOpen = FALSE;
	*Opened = 0; 
	if (Update)
		Mode = BT_WRITE;
	_fstrcpy (File,"[%DL]usedref.btr");
	ExpandText (File);
	ShareEnabled=TRUE; 
Top:
	if (hURT && (OpenMode==Update || Mode==BT_WRITE)) 
		goto Exit;
    BT_CLOSE (hURT);
    if (!(hURT = BT_OPEN (File,0,Mode,0)))
    {   
    	if (!ExistFile (File))
    	{
	    	if (CreateUsedRefTable (FALSE))
    			goto Top;
    	}
    	rtn = FALSE;
        goto Exit;
    }  
    OpenMode = Update;  
    *Opened = 1;
Exit: 
	ShareEnabled = SaveShareEnabled; 
	AllowCache = SaveAllowCache;
	UndoEnabled = SaveUndoEnabled;
    KeepFilesOpen = SaveKFO;
	GSSiGlobUlFree (&hMem);    
																							{
																							#if ENABLETRACE
																							GSSiExitProg (1184);
																							#endif
    return rtn;
																							}
																							#if ENABLETRACE
																							}
																							#endif
}
  
void CloseUsedRefTable (BOOL Opened)
#if ENABLETRACE
{GSSiEnterProg (1185);
#endif
{  
    if (!Opened)
{
#if ENABLETRACE
GSSiExitProg (1185);
#endif
    	return;
}
    BT_CLOSE (hURT);
    hURT = 0;
{
#if ENABLETRACE
GSSiExitProg (1185);
#endif
    return;
}
#if ENABLETRACE
}
#endif
} 

BOOL CreateUsedRefTable (BOOL Replace)
#if ENABLETRACE
{GSSiEnterProg (1186);
#endif
{
	char	File[128]="[%DL]usedref.btr";
	long	BeginRef=0, EndRef=1000000; 
	
	BTVARDESC	BTVar[2];   
	BTVar[0].BT_VARTYP=BT_INTEGER;
	BTVar[0].BT_VARLEN=4;
	BTVar[0].BT_VAROFF=0;
	if (!Replace && ExistFile (File))
{
#if ENABLETRACE
GSSiExitProg (1186);
#endif
		return FALSE;
}
	BT_CREATE (File, 4, FALSE, 1, 1, (LPBTVARDESC)BTVar, FALSE, 0, 0, FALSE);  
	hURT = BT_OPEN (File,0,BT_WRITE,0); 
	BT_PUT (hURT,(LPSTR)&BeginRef,(LPSTR)&EndRef);
	BT_CLOSE (hURT);
	hURT = 0;
{
#if ENABLETRACE
GSSiExitProg (1186);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}
	
long GetNewRefno (LPSTR EditFile,LPSTR Prefix, LPSTR UDI,LPSHORT pSymNum,LPBOOL pRedefine)
																							#if ENABLETRACE
																							{GSSiEnterProg (1187);
																							#endif
{   int		n,ii;
	long	TLID, Delay=5000000; 
	char	str[MAX_PATH]; 
	OFSTRUCTGM	OFStruct;
	unsigned frequency=1000, duration=100; 
	char	NewRefnoPath[MAX_PATH]; 
	extern	BOOL    ShareEnabled;
	BOOL	SaveShareEnabled = ShareEnabled, SaveKFO = KeepFilesOpen, SaveUndoEnabled = UndoEnabled;  
	BOOL	SaveAllowCache = AllowCache;
	BOOL	Update=TRUE, DoRedefine=FALSE; //disable redefine - can cause data loss if inadvertent
	LPSTR	lpBS;
	HIGHLIGHTDATA	HighlightData;
	
	//disable redefine - can cause data loss if inadvertent
	if ((DWORD)Prefix == 1)
		Update = FALSE;
	if (pRedefine)
	{   
		if (hHighlight && DoRedefine)
		{
			if (BT_NUM_IN_INDEX (hHighlight) == 1)
			{
				BT_FIND (hHighlight,(LPSTR)&TLID,BT_FIRST,BT_ANY,(LPSTR)&HighlightData);
				*pRedefine = TRUE;     
				_fstrcpy (Prefix,HighlightData.PD.Prefix);
				_fstrcpy (UDI,HighlightData.PD.UDI);   
				*pSymNum = HighlightData.PD.Desc;
				SetIntRefno (TLID); 
																							{
																							#if ENABLETRACE
																							GSSiExitProg (1187);
																							#endif
				return TLID;
																							}
			} 
		}
		*pRedefine = FALSE;
	}
    GetGlobalCVal ("[%NEWREFFILE]",str,EditFile);
	ExpandText (str);
	_fstrlwr (str); 
	if (strstr(str,"usedref.btr"))
	{
 		long	StartRefno = GetGlobalLVal2 ("[%STARTREFNO]",1000000);
		TLID = GetNextRefno (&StartRefno,TRUE,Update);
		SetIntRefno (TLID); 
																							{
																							#if ENABLETRACE
																							GSSiExitProg (1187);
																							#endif
		return(TLID);
																							}
	}
	_fullpath (NewRefnoPath,str,MAX_PATH);
	lpBS = _fstrrchr (NewRefnoPath,'\\');
	*lpBS = 0;
	_fstrcat (NewRefnoPath,"\\nextref.txt");
Open: 
	ShareEnabled = TRUE;
	UndoEnabled = FALSE;
	KeepFilesOpen = FALSE;
	AllowCache = FALSE;
	if (NewRefFid == HFILE_ERROR)
		NewRefFid = GSSiOpenFile (NewRefnoPath,&OFStruct,OF_READWRITE);  
	if (NewRefFid == HFILE_ERROR)
	{   
		char	mess[256];
		
		if (OFStruct.nErrCode < 4)
		{
			if (!GetGlobalCVal ("[%INITIALREFNO]",str,0))
			{
				sprintf (mess,"Nextref file %s does not exist. Do you want to create it?",NewRefnoPath);
				if (MessageBox (GetFocus(),mess,"Failed to assign refno",MB_ICONEXCLAMATION|MB_YESNO) == IDNO)  
				{
					TLID = 0;
					goto Exit;
				}
				_fstrcpy (str,"0");
			}
			NewRefFid = GSSiOpenFile (NewRefnoPath,&OFStruct,OF_CREATE);
			if (NewRefFid == HFILE_ERROR)
			{
				sprintf (mess,"Cannot create file '%s'",NewRefnoPath);
				MessageBox (GetFocus(),mess,"Failed to assign refno",MB_ICONEXCLAMATION);
				TLID = 0;
				goto Exit;
			}
			fputstring (str,NewRefFid);
			GSSiClose (NewRefFid);  
			NewRefFid = HFILE_ERROR; 
			goto Open;
		
		}
		
		sprintf (mess,"Cannot open file '%s'",NewRefnoPath);
		MessageBox (GetFocus(),mess,"Failed to assign refno",MB_ICONEXCLAMATION); 
		TLID = 0;
		goto Exit;
	}
	fgetstring (str,12,NewRefFid);
	TLID = atol(str);
	TLID++;
	if (!TLID)
		TLID++;
	GSSillseek(NewRefFid,0,0);
	if (Update)
	{   
		ltoa(TLID,str,10);
		fputstring(str,NewRefFid); 
		GSSiClose(NewRefFid);
		NewRefFid = HFILE_ERROR; 
	}
Exit:
	SetIntRefno (TLID); 
	ShareEnabled = SaveShareEnabled; 
	AllowCache = SaveAllowCache;
	UndoEnabled = SaveUndoEnabled;
    KeepFilesOpen = SaveKFO;
																							{
																							#if ENABLETRACE
																							GSSiExitProg (1187);
																							#endif
	return(TLID);
																							}
																							#if ENABLETRACE
																							}
																							#endif
}

long GetNewRefnoNoUpdate (LPSTR EditFile)
																							#if ENABLETRACE
																							{GSSiEnterProg (1188);
																							#endif
{   
	// returns refno without updating table - also locks newref file until released by call to GetNewRefno or 
	// call to this routine with NULL  
	BOOL	UsingUsedRefBTR = UsesUsedRef ();  
	static	BOOL	Opened;
	
	
	if (EditFile) 
	{   
			
		if (UsingUsedRefBTR)
		{
			if (!OpenUsedRefTable (TRUE,&Opened))
																							{
																							#if ENABLETRACE
																							GSSiExitProg (1188);
																							#endif
				return 0; 
																							}
		}
																							{
																							#if ENABLETRACE
																							GSSiExitProg (1188);
																							#endif
		return GetNewRefno (EditFile,(LPSTR)1, 0,0,0);
																							}
	}
	else if (UsingUsedRefBTR) 
		CloseUsedRefTable (Opened);
	else
	{
		if (NewRefFid != HFILE_ERROR)
			GSSiClose (NewRefFid);
		NewRefFid = HFILE_ERROR;
	}
																							{
																							#if ENABLETRACE
																							GSSiExitProg (1188);
																							#endif
	return 0;
																							}
																							#if ENABLETRACE
																							}
																							#endif
} 

BOOL UsesUsedRef (void)
																							#if ENABLETRACE
																							{GSSiEnterProg (1189);
																							#endif
{    
	BOOL	rtn=FALSE;
	HANDLE	hstr=GSSiGlobAlloc (1265,GMEM_MOVEABLE,128);
	LPSTR	str=GlobalLock (hstr);
	
    GetGlobalCVal ("[%NEWREFFILE]",str,0);
	_fstrlwr (str); 
	if (strstr(str,"usedref.btr"))
		rtn = TRUE;   
	GSSiGlobUlFree (&hstr);
																							{
																							#if ENABLETRACE
																							GSSiExitProg (1189);
																							#endif
	return rtn;
																							}
																							#if ENABLETRACE
																							}
																							#endif
}   

void GetComputerID (void)
{
	return;
}

BOOL MakeVPFullScreen (int vpid, int Opt)
{
	double Factor;

	if (!SetViewport (vpid))
		return FALSE; 
	if (CurView->DisplayedFullScreen && Opt != 1)
	{
		CurView->TagPoint = CurView->TagPointSave;
		CurView->Width = CurView->SaveWidth; 
		CurView->Height = CurView->SaveHeight; 
		CurView->Parent = CurView->SaveParent;
		CurView->NewBounds = CurView->WBounds = CurView->SaveWBounds;
		CurView->DisplayedFullScreen = 0;   
		*pCommandViewport = CurView->PreviousCommandVP;
//		Factor = ((double)CurView->SaveWidth + CurView->SaveHeight)/200.0;
//		CurView->Scale *= Factor;
	}
	else if(!CurView->DisplayedFullScreen && Opt) 
	{   
		CurView->TagPointSave = CurView->TagPoint;
		CurView->SaveWidth = CurView->Width;
		CurView->SaveHeight = CurView->Height;
		CurView->SaveWBounds = CurView->WBounds;
		CurView->SaveParent = CurView->Parent;
		CurView->TagPoint.x = 0;
		CurView->TagPoint.y = 0;
		CurView->Width = 100; 
		CurView->Height = 100;
		CurView->Parent = 0;
//		Factor = 200.0 / ((double)CurView->SaveWidth + CurView->SaveHeight);
//		CurView->Scale *= Factor;
		CurView->DisplayedFullScreen = 1;  
		CurView->PreviousCommandVP = *pCommandViewport;  
		*pCommandViewport = CurView->ID;

	}
    SetupViewports (CurView->hWnd,CurView->hDC,0,MainRect,0); 
	DisplayCycle++;
	SetBounds(CurView->hWnd,CurView->hDC);
	RedisplayWindow ();
	return TRUE;
}  

BOOL HavePixelTheme (void)
{   
	UINT	itheme;
	
	for (itheme=0;itheme<CurView->NumThemes;itheme++)
	{	 
		if (CurView->pThemes[itheme]->DataType == THEMEDATATYPE_PIXEL &&
			CurView->pThemes[itheme]->IsActive && 
			CurView->pThemes[itheme]->VPDisplayed)

			return TRUE;
	}
	return FALSE;
} 

short FirstPixelPass (void)
{   
	UINT	itheme; 
	LPTHEME	SaveTheme=CurTheme;
	
	for (itheme=0;itheme<CurView->NumThemes;itheme++)
	{	 
		if (CurView->pThemes[itheme]->DataType == THEMEDATATYPE_PIXEL &&
			CurView->pThemes[itheme]->IsActive && 
			CurView->pThemes[itheme]->VPDisplayed) 
			{
				CurTheme = CurView->pThemes[itheme];
            	if (ThemeNeedsDataPass (TRUE)) 
            	{   
            		CurTheme = SaveTheme;
					return 0;
				}
			}
	}  
	CurTheme = SaveTheme;
	return 1;
} 


BOOL ProcessVPPixelThemes (void)
{   
	UINT	row,col;  
	POINT	WinPt; 
	DPOINT	BasePt;  
	short	ii;
	short	FirstPass,SavePass = CurView->PassID;
	MSG     msg;   
	
	if (!HavePixelTheme ())
		return FALSE; 
	CurItemHLTShow=-1;
    SaveDC (CurView->hDC);
	SetDisplayMode (CurView->hDC, GF_SCREENMODE); 
/*    SetMapMode    (CurView->hDC, MM_TEXT );
    SetWindowOrgEx  ( CurView->hDC, 0, 0,0 );
    SetViewportOrgEx( CurView->hDC, 0, 0,0 );*/    
    SelectClipRgn (CurView->hDC,0);
	CurrentType = GF_PIXEL;
	FirstPass = FirstPixelPass(); 
	if (FirstPass == 1)
		ThemeBeginDisplayPass(TRUE,CurView->ID);
	
	for (CurView->PassID=FirstPass;CurView->PassID<2;CurView->PassID++)
	{
		for (row=CurView->DrawRect.top; row<=CurView->DrawRect.bottom; row++)
		{
			for (col=CurView->DrawRect.left; col<=CurView->DrawRect.right; col++)
			{ 
				WinPt.x = col;
				WinPt.y = row;
				CurrentPoint = WinPtToBasePt (WinPt);
				if (SetDisplayChar (CurView->hDC,GF_PIXEL,0,0,0,0) > 0)
				{
					if (CurView->PassID && ThemePointColor >= 0)
						SetPixel (CurView->hDC,WinPt.x,WinPt.y,ConvertColor(ThemePointColor,ThemePointUseHalfTone)); 
				}
			}
	
			while (GSSiPeekMessage(&msg,0,0,0,PM_REMOVE))
			{
				if (msg.message == WM_PAINT)
				{
				  TranslateMessage(&msg);
				  DispatchMessage(&msg);
				}
	    		else if (msg.message == WM_KEYDOWN && msg.wParam == 27)  
	    		{   
	    			HaltMapDisplay (FALSE,TRUE);
					return FALSE;
				}
			}
		}
		if (!CurView->PassID) 
		{
			 ThemeEndDataPass(TRUE);
			 ThemeBeginDisplayPass(TRUE,CurView->ID);
		}
		else
			ThemeEndDisplayPass (FALSE,TRUE,FALSE);
	} 
	CurView->PassID = SavePass;
    RestoreDC (CurView->hDC,-1);
	return TRUE;
}
 
void SetViewportForCommand (int Message, WPARAM wParam)
{   
	switch (Message)
	{ 
		case GF_EXECUTE:
			SetViewport (wParam);
		break;
		default:
		break;
	}
	return;
}


BOOL ReadTranRecord (void)
#if ENABLETRACE
{GSSiEnterProg (691);
#endif
{
	char		TranData[140];
	double 		XINCH[4],YINCH[4],XBASE[4],YBASE[4], XWIN[4],YWIN[4];
	float		RSQMIN;
	LPSTR		lpCor; 
	POINT		TestPoint1={0,0}, TestPoint2={100,0};
	DPOINT		TestPointD1, TestPointD2;
   	int	i; 
   	WORD		nRead;

	GSSillseek(FidMap,TranPointOffset,0);
	BigRead (FidMap,(HPSTR)&nRead,2);
    BigRead (FidMap,(HPSTR)&TranData,nRead-4); 
    lpCor =(LPSTR)  &TranData;
    XINCH[0] = dread (lpCor,16); 
    lpCor+=16;
    XINCH[2] = dread (lpCor,16);
    lpCor+=16;
    YINCH[0] = dread (lpCor,16);
    lpCor+=16;
    YINCH[1] = dread (lpCor,16);
    lpCor+=16;
    XBASE[0] = dread (lpCor,16);
    lpCor+=16;
    XBASE[2] = dread (lpCor,16);
    lpCor+=16;
    YBASE[0] = dread (lpCor,16);
    lpCor+=16;
    YBASE[1] = dread (lpCor,16);
    XINCH[1] = XINCH[0];
    XINCH[3] = XINCH[2];
    YINCH[2] = YINCH[1];
    YINCH[3] = YINCH[0];
    XBASE[1] = XBASE[0];
    XBASE[3] = XBASE[2];
    YBASE[2] = YBASE[1];
    YBASE[3] = YBASE[0];  
    AdjustMapControlPoints (CurView->ID, FileNum, XBASE,YBASE);
	CloseTRANS2 (&hTranFileToBase); 
	CloseTRANS2 (&hTranBaseToFile);  
	CloseTRANS2 (&hTranFileToVP);  
	if (XBASE[0] > XBASE[2])
	{
	/*	double SaveX = XBASE[0];
		XBASE[0] = XBASE[2];
		XBASE[1] = XBASE[2];
		XBASE[2] = SaveX;
		XBASE[3] = SaveX;  */  
		for (i=0;i<4;i++)
		{
			XBASE[i] = XINCH[i];
			YBASE[i] = YINCH[i];
		}
	} 
     if(!CurView->hTranBaseToVP) 
		CreateBaseToVPTran (CurView->DrawRect);  
	 for (i=0;i<4;i++)
	 	TRANS2 (XBASE[i],YBASE[i],&XWIN[i],&YWIN[i],CurView->hTranBaseToVP); 
    hTranFileToBase = STRAN2 (1628,XINCH,YINCH,XBASE,YBASE,4,(LPFLOAT)&RSQMIN,1,0);
    hTranBaseToFile = STRAN2 (1629,XBASE,YBASE,XINCH,YINCH,4,(LPFLOAT)&RSQMIN,1,0);    
    hTranFileToVP  = STRAN2 (1630,XINCH,YINCH,XWIN,YWIN,4,(LPFLOAT)&RSQMIN,1,0);
	TestPoint1.x = CurFileMinMax.xmn;
	TestPoint1.y = CurFileMinMax.ymn; 
	TestPoint2 = TestPoint1;
	TestPoint2.x += 100;
    TestPointD1 = FilePtToBasePt (TestPoint1); 
    TestPointD2 = FilePtToBasePt (TestPoint2); 
/*    if (TestPointD1.x >= TestPointD2.x || TestPointD1.y != TestPointD2.y)
    {
    	double Scale = ldistp (TestPointD1,TestPointD2)/100;  
    	POINT	FilePoints[2];
    	DPOINT	BasePoints[2];
    	FilePoints[0].x = CurFileMinMax.xmn;
    	FilePoints[0].y = CurFileMinMax.ymn;
    	FilePoints[1].x = CurFileMinMax.xmx; 
    	FilePoints[1].y = CurFileMinMax.ymx;
    	BasePoints[0].x = FilePoints[0].x;
    	BasePoints[0].y = FilePoints[0].y;
    	BasePoints[1].x = BasePoints[0].x + ((double) FilePoints[1].x - (double) FilePoints[0].x) * Scale;
    	BasePoints[1].y = BasePoints[0].y + ((double) FilePoints[1].y - (double) FilePoints[0].y) * Scale;
		CloseTRANS2 (&hTranFileToBase); 
		CloseTRANS2 (&hTranBaseToFile); 
		XINCH[0] = XINCH[1] = FilePoints[0].x; 
		XINCH[2] = XINCH[3] = FilePoints[2].x; 
		YINCH[0] = YINCH[3] = FilePoints[0].y;  
		YINCH[1] = YINCH[2] = FilePoints[1].y;
		XBASE[0] = XBASE[1] = BasePoints[0].x;
		XBASE[2] = XBASE[3] = BasePoints[1].x;
		YBASE[0] = YBASE[3] = BasePoints[0].y;
		YBASE[1] = YBASE[2] = BasePoints[1].y;
	    hTranFileToBase = STRAN2 (XINCH,YINCH,XBASE,YBASE,4,(LPFLOAT)&RSQMIN,1,0);
	    hTranBaseToFile = STRAN2 (XBASE,YBASE,XINCH,YINCH,4,(LPFLOAT)&RSQMIN,1,0);    
    } */
    if (ReorgFile) 
    {
    	DPOINT DPoint[4], DPointFrom[4]; 
    	double	dist1, dist2;
		double Minx = DBL_MAX, Miny = DBL_MAX, Maxx = -DBL_MAX,	Maxy = -DBL_MAX;
        
		for (i=0;i<4;i++)
		{	                              
            DPoint[i].x = XBASE[i];
            DPoint[i].y = YBASE[i];    
            DPointFrom[i] = DPoint[i];
			DPoint[i] = TranPointReorg (&DPoint[i],hTranReorg);            
			Minx = min (Minx,DPoint[i].x);
			Maxx = max (Maxx,DPoint[i].x);
			Miny = min (Miny,DPoint[i].y);
			Maxy = max (Maxy,DPoint[i].y);
        }
	    XBASE[0] = Minx;
	    YBASE[0] = Miny;
	    XBASE[1] = Minx;
	    YBASE[1] = Maxy;
	    XBASE[2] = Maxx;
	    YBASE[2] = Maxy;
	    XBASE[3] = Maxx;
	    YBASE[3] = Miny;
	    dist1=Maxx-Minx;
	    dist2=Maxy-Miny;  
	    if (dist1 > dist2)
	    {  
	    	XINCH[0] = -32000;
	    	XINCH[2] =  32000; 
	    	YINCH[1] =  IDNINT (dist2/dist1 * 32000);
	    	YINCH[0] =  -YINCH[1]; 
	    }
	    else
	    {  
	    	YINCH[0] = -32000;
	    	YINCH[1] =  32000; 
	    	XINCH[2] =  IDNINT (dist1/dist2 * 32000);
	    	XINCH[0] =  -XINCH[2];
	    }
	    XINCH[1]=XINCH[0];
	    XINCH[3]=XINCH[2];
	    YINCH[2]=YINCH[1];
	    YINCH[3]=YINCH[0]; 
	    dist1 = ldistp (DPointFrom[0],DPointFrom[2]);
	    dist2 = ldistp (DPoint[0],DPoint[2]);
	    ReorgSizeFactor = dist2/dist1;
		CloseTRANS2 (&hTranBaseToFileReorg);
        hTranBaseToFileReorg = STRAN2 (1631,XBASE,YBASE,XINCH,YINCH,4,(LPFLOAT)&RSQMIN,1,0); 
    }
    else if (CurView->FileProjectionType)
    { 
    	DPOINT DPoint; 
	                              
		for (i=0;i<4;i++)
		{	                              
            DPoint.x = XINCH[i];
            DPoint.y = YINCH[i];
            ProjectFilePtD (&DPoint);
            XINCH[i] = DPoint.x;
            YINCH[i] = DPoint.y;
        }
		CloseTRANS2 (&hTranFileToBase); 
		CloseTRANS2 (&hTranBaseToFile);
		CloseTRANS2 (&hTranFileToVP);
	    if(!CurView->hTranBaseToVP) 
			CreateBaseToVPTran (CurView->DrawRect);  
		for (i=0;i<4;i++)
		 	TRANS2 (XBASE[i],YBASE[i],&XWIN[i],&YWIN[i],CurView->hTranBaseToVP); 
        hTranFileToBase = STRAN2 (1632,XINCH,YINCH,XBASE,YBASE,4,(LPFLOAT)&RSQMIN,1,0);
        hTranBaseToFile = STRAN2 (1633,XBASE,YBASE,XINCH,YINCH,4,(LPFLOAT)&RSQMIN,1,0); 
	    hTranFileToVP  = STRAN2 (1634,XINCH,YINCH,XWIN,YWIN,4,(LPFLOAT)&RSQMIN,1,0);
    }

{
#if ENABLETRACE
GSSiExitProg (691);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
} 

BOOL FileBoundsToWBounds (LPMINMAX pFileBounds,LPMNMXCORD pWBounds)
{    
	DPOINT	BasePt;
	POINT	FilePt;
	
	DBoundsInit (pWBounds);
	
	FilePt.x = pFileBounds->xmn;
	FilePt.y = pFileBounds->ymn;
	BasePt = FilePtToBasePt (FilePt);
	AddDPointToMinMax (&BasePt,pWBounds);
	FilePt.y = pFileBounds->ymx;
	BasePt = FilePtToBasePt (FilePt);
	AddDPointToMinMax (&BasePt,pWBounds);
	FilePt.x = pFileBounds->xmx;
	BasePt = FilePtToBasePt (FilePt);
	AddDPointToMinMax (&BasePt,pWBounds);
	FilePt.y = pFileBounds->ymn;
	BasePt = FilePtToBasePt (FilePt);
	AddDPointToMinMax (&BasePt,pWBounds);

	return TRUE;
} 

BOOL FileBoundsLToWBounds (LPMNMXCORL pFileBounds,LPMNMXCORD pWBounds)
{    
	DPOINT	BasePt,CenterPoint;
	POINT	FilePt;
	MNMXCORD	Bounds;

	DBoundsInit (&Bounds);
	
	FilePt.x = pFileBounds->xmn;
	FilePt.y = pFileBounds->ymn;
	BasePt = FilePtToBasePt (FilePt);
	AddDPointToMinMax (&BasePt,&Bounds);
	FilePt.y = pFileBounds->ymx;
	BasePt = FilePtToBasePt (FilePt);
	AddDPointToMinMax (&BasePt,&Bounds);
	FilePt.x = pFileBounds->xmx;
	BasePt = FilePtToBasePt (FilePt);
	AddDPointToMinMax (&BasePt,&Bounds);
	FilePt.y = pFileBounds->ymn;
	BasePt = FilePtToBasePt (FilePt);
	AddDPointToMinMax (&BasePt,&Bounds);
	//keep orig cp
	CenterPoint = MinMaxMidPointD (&Bounds);
	pWBounds->xmn = CenterPoint.x - (Bounds.xmx - Bounds.xmn)/2;
	pWBounds->xmx = CenterPoint.x + (Bounds.xmx - Bounds.xmn)/2;
	pWBounds->ymn = CenterPoint.y - (Bounds.ymx - Bounds.ymn)/2;
	pWBounds->ymx = CenterPoint.y + (Bounds.ymx - Bounds.ymn)/2;
	return TRUE;
} 

BOOL BoundsToWinRect (LPMNMXCORD pBounds,LPRECT pRect)
{   
	DPOINT	DPoint; 
	POINT	Point;
	
	RectInit (pRect);
	
	DPoint.x = pBounds->xmn;
	DPoint.y = pBounds->ymn;
	Point = BasePtToWinPt (&DPoint);
    AddPointToRect (Point,pRect);
	DPoint.y = pBounds->ymx;
	Point = BasePtToWinPt (&DPoint);
    AddPointToRect (Point,pRect);
	DPoint.x = pBounds->xmx;
	Point = BasePtToWinPt (&DPoint);
    AddPointToRect (Point,pRect);
	DPoint.y = pBounds->ymn;
	Point = BasePtToWinPt (&DPoint);
    AddPointToRect (Point,pRect);
    
    return TRUE;
}

BOOL WBoundsToScreenBounds (LPMNMXCORD pWBounds,LPMNMXCORD pSBounds)
{
	DPOINT	DPoint, Point; 
	
	DBoundsInit (pSBounds);
	
	DPoint.x = pWBounds->xmn;
	DPoint.y = pWBounds->ymn;
	Point = BasePtToScreenPtD (&DPoint);
	AddDPointToMinMax (&Point,pSBounds);

 	DPoint.y = pWBounds->ymx;
	Point = BasePtToScreenPtD (&DPoint);
	AddDPointToMinMax (&Point,pSBounds);
	DPoint.x = pWBounds->xmx;
	Point = BasePtToScreenPtD (&DPoint);
	AddDPointToMinMax (&Point,pSBounds);
	DPoint.y = pWBounds->ymn;
	Point = BasePtToScreenPtD (&DPoint);
	AddDPointToMinMax (&Point,pSBounds);
	return TRUE;
}

BOOL BoundsToScreenRect (LPMNMXCORD pBounds,LPRECT pRect)
{   
	DPOINT	DPoint; 
	POINT	Point;
	
	RectInit (pRect);
	
	DPoint.x = pBounds->xmn;
	DPoint.y = pBounds->ymn;
	Point = BasePtToScreenPt (&DPoint);
    AddPointToRect (Point,pRect);
	DPoint.y = pBounds->ymx;
	Point = BasePtToScreenPt (&DPoint);
    AddPointToRect (Point,pRect);
	DPoint.x = pBounds->xmx;
	Point = BasePtToScreenPt (&DPoint);
    AddPointToRect (Point,pRect);
	DPoint.y = pBounds->ymn;
	Point = BasePtToScreenPt (&DPoint);
    AddPointToRect (Point,pRect);
    
    return TRUE;
}

BOOL BoundsToRect (LPMNMXCORD pBounds,LPRECT pRect)
{   
	pRect->left = IDNINT(pBounds->xmn);
	pRect->bottom = IDNINT(pBounds->ymx);
	pRect->top = IDNINT(pBounds->ymn);
 	pRect->right = IDNINT(pBounds->xmx);
    
    return TRUE;
}

BOOL WinRectToBounds (LPRECT pRect,LPMNMXCORD pBounds)
{
	DPOINT	DPoint; 
	POINT	Point;
	
	DBoundsInit (pBounds); 
	Point.x = pRect->left;
	Point.y = pRect->bottom;
	DPoint = WinPtToBasePt (Point);
	AddDPointToMinMax (&DPoint,pBounds);
	Point.y = pRect->top;
	DPoint = WinPtToBasePt (Point);
	AddDPointToMinMax (&DPoint,pBounds);
	Point.x = pRect->right;
	DPoint = WinPtToBasePt (Point);
	AddDPointToMinMax (&DPoint,pBounds);
	Point.y = pRect->bottom;
	DPoint = WinPtToBasePt (Point);
	AddDPointToMinMax (&DPoint,pBounds);
	
	return TRUE;
}

BOOL PointsInSameGroup (long Point1,long Point2,long nPoints,LPLONG PrevList, LPLONG NextList)
{   
	ULONG	i;
	long	ID;
	
	ID = Point1;
	while (PrevList[ID])
	{
		ID = PrevList[ID];
		if (ID == Point2)
			return TRUE;
	}
	ID = Point1;
	while (NextList[ID])
	{
		ID = NextList[ID];
		if (ID == Point2)
			return TRUE;
	}
	return FALSE;
}
 
void AddPointToLinkList (long Point1,long Point2,LPLONG PrevList,LPLONG NextList)
{   
	
	while (NextList[Point1])
		Point1 = NextList[Point1];
	
	while (PrevList[Point2])
		Point2 = PrevList[Point2];
		
	NextList[Point1] = Point2; 
	PrevList[Point2] = Point1;
	return;
}

long SplitGroups (long NumPoints,HPLONG PrevPT,HPLONG NextPT,long MaxPointsPerGroup)
{   
	long	i,j,lastj,n;

Top:	
	for (i=1;i<NumPoints+1;i++)
		if (!PrevPT[i])
		{
			n = 1;
			j = lastj = i;  
			while (j)
			{
				if (n >= MaxPointsPerGroup)
				{
					PrevPT[j] = 0;
					NextPT[lastj] = 0; 
					goto Top;
				}
				lastj = j;
				j = NextPT[j];
			}
		}
	return 0;
}
 
long GetPointGroups (long NumPoints,HPDPOINT Points,double MaxGroupDist,LPHANDLE phPointGroups,LPHANDLE phGroupBounds,long MaxPointsPerGroup)
{
	long NumGroups=0, NextGroupStart; 
	HPMNMXCORD	GroupBounds; 
	HPLONG	PointGroup;
	ULONG	i,j, MinPt; 
	short	Dummy=0; 
	double	Dist, MinDist; 
	DPOINT	ZeroPt={0,0};
	HANDLE	hBT;
	struct	{float Dist;
			 long	P1, P2;} Key;
	BTVARDESC	BTVar[3]; 
	HANDLE	hTmp=GSSiGlobAlloc (1267,GMEM_MOVEABLE,256);
	LPSTR	TempName=GlobalLock (hTmp);  
	HANDLE	hNextPt=GSSiGlobAlloc (1268,GHND,sizeof(long)*(NumPoints+1));
	HPLONG	NextPT=(HPLONG)GlobalLock (hNextPt);
	HANDLE	hPrevPt=GSSiGlobAlloc (1269,GHND,sizeof(long)*(NumPoints+1));
	HPLONG	PrevPT=(HPLONG)GlobalLock (hPrevPt);
	
	GSSiGetTempFileName (0,"gm",0,TempName); 
			
	BTVar[0].BT_VARTYP=BT_REAL;
	BTVar[0].BT_VARLEN=4;
	BTVar[0].BT_VAROFF=0;
	BTVar[1].BT_VARTYP=BT_INTEGER;
	BTVar[1].BT_VARLEN=4;
	BTVar[1].BT_VAROFF=4;
	BTVar[2].BT_VARTYP=BT_INTEGER;
	BTVar[2].BT_VARLEN=4;
	BTVar[2].BT_VAROFF=8;
	BT_CREATE (TempName, 2, FALSE, 3, 1, (LPBTVARDESC)BTVar, FALSE, 0, 0, FALSE);
	hBT = BT_OPEN (TempName,0, BT_WRITE, 0); 
	GSSiGlobUlFree (&hTmp);
	for (i=0;i<NumPoints-1;i++)
	{
		Key.P1 = i+1;
		for (j=i+1;j<NumPoints;j++)
		{ 
			Dist = ldistp (Points[i],Points[j]); 
			if (Dist <= MaxGroupDist)
			{
				Key.Dist = Dist;
				Key.P2	 = j+1; 
				BT_PUT (hBT,(LPSTR)&Key,(LPSTR)&Dummy);
			}
		}
	}
	
	while (!BT_FIND (hBT,(LPSTR)&Key,BT_FIRST,BT_ANY,(LPSTR)&Dummy))
	{   
		if (!PointsInSameGroup (Key.P1,Key.P2,NumPoints,PrevPT,NextPT))
			AddPointToLinkList (Key.P1,Key.P2,PrevPT,NextPT);
		BT_DELETE (hBT,(LPSTR)&Key,(LPSTR)&Dummy,FALSE);
	}
	BT_CLOSEANDDELETE (&hBT); 
	SplitGroups (NumPoints,PrevPT,NextPT,MaxPointsPerGroup);
	for (i=1;i<NumPoints+1;i++)
		if (!PrevPT[i])
			NumGroups++;
	*phPointGroups = GSSiGlobAlloc (1270,GMEM_MOVEABLE,sizeof(long)*NumPoints);  
	PointGroup = (HPLONG)GlobalLock (*phPointGroups);
	*phGroupBounds = GSSiGlobAlloc (1271,GMEM_MOVEABLE,sizeof(MNMXCORD)*NumGroups);
	GroupBounds = (HPMNMXCORD)GlobalLock (*phGroupBounds);
	for (i=0;i<NumGroups;i++)
	{
		DBoundsInit (&GroupBounds[i]); 
		MinDist = DBL_MAX;
		for (j=0;j<NumPoints;j++)
			if (PrevPT[j+1] >= 0)
			{
				Dist = ldistp (Points[j],ZeroPt);
				if (Dist < MinDist)
				{
					MinDist = Dist;
					MinPt = j+1;
				}
			}
		NextGroupStart = MinPt;
		while (PrevPT[NextGroupStart]) 
			NextGroupStart = PrevPT[NextGroupStart];
		j = NextGroupStart;
		do
		{                 
			AddDPointToMinMax (&Points[j-1],&GroupBounds[i]); 
			PointGroup[j-1] = i;
			PrevPT[j] = -1;
			j = NextPT[j];
		} while (j);
		
	}
	GlobalUnlock (*phGroupBounds);  
	GlobalUnlock (*phPointGroups);
	GSSiGlobUlFree (&hNextPt);
	GSSiGlobUlFree (&hPrevPt);
	return NumGroups;
}


USHORT GroupPoints (LPSTR InFile,LPSTR SQL,LPSTR idfieldname,LPSTR xfieldname,LPSTR yfieldname,LPSTR keyfieldname,
				  double MinDist,double MaxDist,double GroupDist,long MaxPoints,LPSTR BoundsFile,LPSTR PointToBoundsFile)
{   
	USHORT	nGroups=0, i; 
	long	ip;
	HANDLE	hSQL=0; 
	long	nPoints=0; 
	HANDLE	hPoints=GSSiGlobAlloc (1272,GMEM_MOVEABLE,USHRT_MAX);
	HPDPOINT	Points=(HPDPOINT)GlobalLock (hPoints);
	HANDLE	hPointID=GSSiGlobAlloc (1273,GMEM_MOVEABLE,USHRT_MAX);
	HPLONG	PointID=(HPLONG)GlobalLock (hPointID);
	HANDLE	hStr=GSSiGlobAlloc (1274,GMEM_MOVEABLE,1024);
	LPSTR	str=GlobalLock (hStr);
	HANDLE	hBounds=0, hPointBoundAssignment=0;
	HFILE	Fid;
	HPMNMXCORD	pBounds;
	HPLONG	pBoundsID; 
	long	id;

    if (!OpenDataFile (InFile,SQL,BT_READ,&hSQL))
        return 0;
	while (FetchDBRec (hSQL))
	{   
		if (idfieldname && *idfieldname)  
		{
			GetValFromOpenFiles (idfieldname,str,1024);
			id = atol (str);
		} 
		else
			id = nPoints;
		GetValFromOpenFiles (xfieldname,str,1024);
		Points[nPoints].x = atof (str);
		GetValFromOpenFiles (yfieldname,str,1024);
		Points[nPoints].y = atof (str);  
		PointID[nPoints] = id;
		nPoints++;
	}
	CloseDataFile (FALSE,&hSQL);  
	if (nPoints)
	{
		nGroups = BoundsFromPoints (nPoints,Points,MinDist,MaxDist,GroupDist,MaxPoints,&hBounds,&hPointBoundAssignment); 
		Fid = GSSiOpenFile (BoundsFile,0,OF_CREATE);
		if (Fid != HFILE_ERROR)
		{   
			fputstring ("GROUPBOUNDS",Fid);
			pBounds = (HPMNMXCORD)GlobalLock (hBounds);
			for (i=0;i<nGroups;i++,pBounds++)
			{
				sprintf (str,"%f %f %f %f",pBounds->xmn,pBounds->ymn,pBounds->xmx,pBounds->ymx);
				fputstring (str,Fid);
			} 
			GlobalUnlock (hBounds);
			GSSiClose (Fid); 
		}
		if (*PointToBoundsFile)
		{ 
			Fid = GSSiOpenFile (PointToBoundsFile,0,OF_CREATE);
			if (Fid != HFILE_ERROR)
			{   
				fputstring ("PointNum,BoundsID",Fid);
				pBoundsID = (HPLONG)GlobalLock (hPointBoundAssignment);
				for (ip=0;ip<nPoints;ip++)
				{
					sprintf (str,"%ld,%ld",PointID[ip],pBoundsID[ip]+1);
					fputstring (str,Fid);
				} 
				GlobalUnlock (hPointBoundAssignment);
				GSSiClose (Fid); 
			}
		}  
	}
	GSSiGlobUlFree (&hStr);
	GSSiGlobUlFree (&hPoints); 
	GSSiGlobUlFree (&hPointID);
	GSSiGlobFree (&hBounds);
	GSSiGlobFree (&hPointBoundAssignment);
	return nGroups;
}

long SetPointBoundsID (long NumPoints,long groupid,long boundsid,HPLONG PointBoundsID,HANDLE hPointGroups)
{   
	HPLONG	PointGroups=(HPLONG)GlobalLock (hPointGroups); 
	long	i,n=0;
	
	for (i=0;i<NumPoints;i++) 
		if (PointGroups[i] == groupid)
		{
			PointBoundsID[i] = boundsid; 
			n++;
		}
	GlobalUnlock (hPointGroups);
	return n;    
}          

USHORT BoundsFromPoints (long NumPoints,HPDPOINT Points,double MinDist,double MaxDist,double GroupDist,long MaxPointsPerBounds,
					   				    LPHANDLE phBounds,LPHANDLE phPointBoundAssignment)
{
	USHORT		NumBounds=0;
	HPLONG		BoundsID; 
	HPMNMXCORD	Bounds, GroupBounds;  
	MNMXCORD	TestBounds;
	ULONG		i,j;
	HANDLE		hBoundsPoints=GSSiGlobAlloc (1275,GMEM_MOVEABLE,(long)sizeof(long)*USHRT_MAX);
	LPLONG		NumBoundsPoints = (LPLONG)GlobalLock (hBoundsPoints);
	long		NumGroups;
	HANDLE		hPointGroups, hGroupBounds;
	
	*phPointBoundAssignment = GSSiGlobAlloc (1276,GMEM_MOVEABLE,sizeof(long)*NumPoints);
	BoundsID = (HPLONG)GlobalLock (*phPointBoundAssignment);    
	*phBounds = GSSiGlobAlloc (1277,GMEM_MOVEABLE,(long)sizeof(MNMXCORD)*USHRT_MAX);
	Bounds = (HPMNMXCORD)GlobalLock (*phBounds);
	
	NumGroups = GetPointGroups (NumPoints,Points,GroupDist,&hPointGroups,&hGroupBounds,MaxPointsPerBounds);
	
	GroupBounds = (HPMNMXCORD)GlobalLock (hGroupBounds);
	for (i=0;i<NumGroups;i++)
	{
		for (j=0;j<NumBounds;j++)
		{
			if (BoundsInBounds (&GroupBounds[i],&Bounds[j],1))
			{
				SetPointBoundsID (NumPoints,i,j,BoundsID,hPointGroups); 
				NumBoundsPoints[j]++;
				goto NextPoint;
			}
		} 
		for (j=0;j<NumBounds;j++)
		{   
			TestBounds = Bounds[j];    
			AddMinMaxD (&TestBounds,&GroupBounds[i]);
			if (TestBounds.xmx - TestBounds.xmn <= MaxDist && 
				TestBounds.ymx - TestBounds.ymn <= MaxDist)
			{   
				Bounds[j] = TestBounds;
				SetPointBoundsID (NumPoints,i,j,BoundsID,hPointGroups); 
				NumBoundsPoints[j]++;
				goto NextPoint;
			}
		}
		NumBoundsPoints[NumBounds] = 1;
		SetPointBoundsID (NumPoints,i,NumBounds,BoundsID,hPointGroups); 
		Bounds[NumBounds++] = GroupBounds[i];   
NextPoint:;
	}
	for (i=0;i<NumBounds;i++)
	{
/*		Bounds[i].xmn -= GroupDist;
		Bounds[i].ymn -= GroupDist;
		Bounds[i].xmx += GroupDist;
		Bounds[i].ymx += GroupDist;*/ 
		DPOINT	MidP = MinMaxMidPointD (&Bounds[i]);
		if (Bounds[i].xmx - Bounds[i].xmn < MinDist) 
		{
			Bounds[i].xmn = MidP.x - MinDist/2;
			Bounds[i].xmx = MidP.x + MinDist/2;
		}
		if (Bounds[i].ymx - Bounds[i].ymn < MinDist) 
		{
			Bounds[i].ymn = MidP.y - MinDist/2;
			Bounds[i].ymx = MidP.y + MinDist/2;
		}
	} 
	GSSiGlobUlFree (&hGroupBounds);
	GSSiGlobUlFree (&hBoundsPoints);
	GlobalUnlock (*phBounds);
	GlobalUnlock (*phPointBoundAssignment);
	return NumBounds;
}

int PickTypeFromSysType (int SysType)
{
	switch (SysType)
	{    
		case 1:
		case GF_POINT:
			return 1;
			break;  
		case 2:
		case GF_LINE:
		case GF_POLYLINE:
			return 2;
			break; 
		case 3:
		case GF_AREA:
			return 3;     
			break;
		case 4:
		case GF_TEXT:
			return 4;
			break;
		case 5:
		case GF_CURVE:
			return 5;
			break;
	} 
	return 0;
}

int SysTypeFromPickType (int PickType)
{
	switch (PickType)
	{    
		case 1:
		case GF_POINT:
			return GF_POINT;
			break;  
		case 2:
		case GF_LINE:
		case GF_POLYLINE:
			return GF_POLYLINE;
			break; 
		case 3:
		case GF_AREA:
			return GF_AREA;     
			break;
		case 4:
		case GF_TEXT:
			return GF_TEXT;
			break;
		case 5:
		case GF_CURVE:
			return GF_CURVE;
			break;
	}
	return PickType;
}

int LevelTypeFromPickType (int PickType)
{
	switch (PickType)
	{    
		case 1:
		case GF_POINT:
			return 1;
			break;  
		case 2:
		case GF_LINE:
		case GF_POLYLINE:
			return 1;
			break; 
		case 3:
		case GF_AREA:
			return 0;     
			break;
		case 4:
		case GF_TEXT:
			return 2;
			break;
		case 5:
		case GF_CURVE:
			return 1;
			break;
	}
	return 1;
}

BOOL FAR PASCAL OWNERLOCMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{ 	int		st, choice, n,idx,ifile,i; 
	BOOL	HaveWildcard;
	char	str[256];
	static	char Wildcard[128]="";
	static	char WantStr[128]=""; 
	char	WantStr2[128],propkeyfld[64];
	LPGWDHEADER lpGWDHead;
    LPOPENFILEDATA  FilePtr;
    LPOPENSQLDATA   SQLPtr;
	long	Offset;
	static	HANDLE	hDBOwner=0;
	static	HANDLE	hSaveBM=0;  
	static	BOOL	First;  
	static	short	iOwner=-1, SaveProperty=-1; 
	short	Item;
	TAGKEY TAGKey;  
	LPSTR	pTAB; 
	HWND	hwndCtl;
	static	char	NameDelim;
	UINT	nPrompts=4;
	UINT	PrmtDat[4*3] = {
							  IDC_OWNERNAMEWC,PRMT_IDC_OWNERNAMEWC,0,
							  IDC_OWNERNAME, PRMT_IDC_OWNERNAME,0,  
							  IDC_OWNERLIST, PRMT_IDC_OWNERLIST,0,
							  IDC_PROPERTYLIST, PRMT_IDC_PROPERTYLIST,0
							};


 int	BRtn;
 if (Message==WM_INITDIALOG)
 {  
 	UINT	pw=0, pn=1, pm=2; 
 	HWND	hwnd;
 	short	ii;
 	
 	InitDlgPrompts (hWndDlg);
 	while (nPrompts--)
 	{   
 		hwnd = GetDlgItem(hWndDlg,PrmtDat[pw]);
 		if (!hwnd)
 			ii=1;
	 	SetDlgPrompt (hwnd,PrmtDat[pn],PrmtDat[pm]);
	 	pw+=3;
	 	pn+=3; 
	 	pm+=3;
	}
 }
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
 	return (BRtn);
 switch(Message)
   {
 	case WM_DESTROY: 
	 	InitDlgPrompts (0);    
	 	break;
    case WM_INITDIALOG:  
	{   
		short	nTabs;
		HANDLE	hList;
		LPSHORT	pTabs;

    	ClearDlgPrompts (); 
    	hSaveBM = EnterBlockingWindow (hWndDlg);
		First = TRUE;
		SendDlgItemMessage (hWndDlg,IDC_AUTOHIGHLIGHT,BM_SETCHECK,AutoHighlight,0L);
		GetGlobalCVal ("[%OWNERDELIM]",str,","); 
		if (!_fstricmp (str,"\' \'"))
			NameDelim = ' ';
		else
			NameDelim = *str;					
		GetGlobalCVal ("[%OWNERTABS]",str,"170 2000");
		nTabs = GetIntsFromList (str,&hList); 
		if (nTabs)
		{
			pTabs = (LPSHORT)GlobalLock (hList);
			SendDlgItemMessage (hWndDlg,IDC_OWNERLIST,LB_SETTABSTOPS,nTabs,(LPARAM)pTabs);
			GSSiGlobUlFree (&hList);
		}
		GetGlobalCVal ("[%PROPERTYTABS]",str,"80 200 250 2000");
		nTabs = GetIntsFromList (str,&hList); 
		if (nTabs)
		{
			pTabs = (LPSHORT)GlobalLock (hList);
			SendDlgItemMessage (hWndDlg,IDC_PROPERTYLIST,LB_SETTABSTOPS,nTabs,(LPARAM)pTabs);
			GSSiGlobUlFree (&hList);
		}
        cwCenter(hWndDlg, 0); 
        GetGlobalCVal ("[%OWNERDB]",str,"[%DL]attribut\\name.gmd");
	    if (!OpenDataFile (str, "",BT_READ, &hDBOwner))
	    {
	        MessageBox( GetFocus(), str,"Cannot open owner database", MB_OK);
	        return FALSE;
	    }  
	    SQLPtr = (LPOPENSQLDATA) GlobalLock (hDBOwner);
	    FilePtr = (LPOPENFILEDATA) GlobalLock (SQLPtr->OFHandle);
		lpGWDHead = (LPGWDHEADER)GlobalLock (FilePtr->FileHandle);    
		n = lpGWDHead->NumIndex;
		GlobalUnlock (FilePtr->FileHandle);
		GlobalUnlock (SQLPtr->OFHandle); 
		GlobalUnlock (hDBOwner);                            
		if (!n)    
	    {
	        MessageBox( GetFocus(), str,"Name index does not exist", MB_OK);
			CloseDataFile (TRUE,&hDBOwner); 
	        return FALSE;
	    }  
	    SetDlgItemText (hWndDlg,IDC_OWNERNAMEWC,Wildcard);            
	    SetDlgItemText (hWndDlg,IDC_OWNERNAME,WantStr);    
		PostMessage(GetDlgItem(hWndDlg,IDC_OWNERLIST), LB_SETCURSEL,iOwner,0); 
		PostMessage(hWndDlg, WM_COMMAND, IDC_SETPROPLIST, 0L);
	 	PostMessage(GetDlgItem(hWndDlg,IDC_PROPERTYLIST), LB_SETSEL,TRUE,SaveProperty);  
	} 

        break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(LOWORD(wParam))
           {
			case IDC_OWNERNAME:
			case IDC_OWNERNAMEWC: 
		    	 SetDlgItemText (hWndDlg,IDC_PROPMESSAGE,""); 
				 hwndCtl = (HWND) LOWORD(lParam);
                 switch (HIWORD(wParam))
                 {	case EN_CHANGE: 
                 	{   
						SendDlgItemMessage (hWndDlg,IDC_OWNERLIST,LB_RESETCONTENT,0,0);
						SendDlgItemMessage (hWndDlg,IDC_PROPERTYLIST,LB_RESETCONTENT,0,0);   
						HaveWildcard = GetDlgItemText (hWndDlg,IDC_OWNERNAMEWC,Wildcard,128);
                 		_fmemset (str,0,42);
           		      	GetDlgItemText (hWndDlg,IDC_OWNERNAME,str,42);
           		      	n = _fstrlen (str);
           		      	if (n)
           		      	{   
           		      		short	pos=BT_FIRST, cond=BT_GE;
           		      		BOOL	GetNext = TRUE;   
           		      		short	NameIndex=1, NameLen=0; 
       		      			MSG     msg;   
                            
                            if (!_fstricmp (str,"*"))
                            {
                            	n = 0;
                            	*str = 0;
                            }
           		      		_fstrcpy (WantStr,str);
						    SQLPtr = (LPOPENSQLDATA) GlobalLock (hDBOwner);
						    FilePtr = (LPOPENFILEDATA) GlobalLock (SQLPtr->OFHandle);
							lpGWDHead = (LPGWDHEADER)GlobalLock (FilePtr->FileHandle);                             
							for (i=0;i<abs(lpGWDHead->NumIndexFields[NameIndex]);i++)
								NameLen += lpGWDHead->pFldInfo[lpGWDHead->IndexFields[NameIndex][i]].Len;
			        		while (GetNext && !BT_FIND (lpGWDHead->BTHandle[NameIndex],(LPSTR)str,pos,cond,(LPSTR)&Offset))
			        		{
			        			LPSTR	pComma=_fstrchr (str,NameDelim);
			        			
			        			pos = BT_NEXT;
			        			cond = BT_ANY; 
			        			
			        			if (!*WantStr)
			        				pComma = str;
			        			str[NameLen]=0;   
			        			Truncate (str);
			        			if (!_fstrnicmp (WantStr,str,n))
			        			{   
			        				BOOL	ShowThisOwner = TRUE;
			        				
			        				if (HaveWildcard)
			        				{   
			        					LPSTR	WC=Wildcard;
			        					
			        					GetDlgItemText (hWndDlg,IDC_OWNERNAMEWC,Wildcard,128);
			        					if (!pComma)
			        						ShowThisOwner = FALSE;
			        					else while (WC)
			        					{   
			        						LPSTR	pEnd=_fstrchr (WC,',');
			        						
			        						if (pEnd)
			        							*pEnd++=0;
			        						if (!_fstrstr (pComma,WC))
			        							ShowThisOwner = FALSE; 
			        						WC = pEnd;
			        					}
			        				}
			        				if (ShowThisOwner)
			        				{
										FillGWDData (lpGWDHead,Offset); 
							        	SQLPtr->st = 0;  
							        	SQLPtr->Offset = Offset; 
							        	SQLPtr->lastreadtime = ULONG_MAX;
							        	GetGlobalCVal ("[%OWNERDISPLAY]",str,"[NAMETAXPAYER]$CHR(1)[NAMEADDR1] [NAMEADDR2] [NAMECITY],[NAMESTATE]$CHR(1)[NAMEACCTOWNER]");
					        			ExpandText (str); 
					        			OneSpace (str); 
					        			ReplaceChar (str,0x01,0x09);
					        		//	sprintf (_fstrchr(str,0),"\t%ld",Offset);  
					        		/*	if ()
					        			{
					        			
								        _fstrcpy (WantStr2,pTAB);
								        ReplaceChar (str,'\t','-');
										SendDlgItemMessage (hWndDlg,IDC_PROPERTYLIST,LB_RESETCONTENT,0,0);
								        GetGlobalCVal ("[%PROPERTYDB]",str,"[%DL]attribut\\master.gmd");
									    if (!OpenDataFile (str, "",BT_READ, &hDBProp))
									    {
									        MessageBox( GetFocus(), str,"Cannot open owner database", MB_OK);
									    }
									    else
									    {               
						   		      		short	IDIndex=2, IDLen=0; 
						   		      		short	pos=BT_FIRST, cond=BT_GE;
						   		      		BOOL	GetNext=TRUE;
								           		      		
										    SQLPtr = (LPOPENSQLDATA) GlobalLock (hDBProp);
										    FilePtr = (LPOPENFILEDATA) GlobalLock (SQLPtr->OFHandle);
											lpGWDHead = (LPGWDHEADER)GlobalLock (FilePtr->FileHandle);                             
											for (i=0;i<abs(lpGWDHead->NumIndexFields[IDIndex]);i++)
												IDLen += lpGWDHead->pFldInfo[lpGWDHead->IndexFields[IDIndex][i]].Len;
											_fstrncpy (lpGWDHead->pKeys[IDIndex],WantStr2,IDLen);
							        		while (GetNext && !BT_FIND (lpGWDHead->BTHandle[IDIndex],lpGWDHead->pKeys[IDIndex],pos,cond,(LPSTR)&Offset))
							        		{
							        			pos = BT_NEXT;
							        			cond = BT_ANY;
											        			
							        			lpGWDHead->pKeys[IDIndex][IDLen]=0;   
							        			Truncate (lpGWDHead->pKeys[IDIndex]);
							        			if (!_fstricmp (WantStr2,lpGWDHead->pKeys[IDIndex]))
							        			{
													FillGWDData (lpGWDHead,Offset); 
										        	SQLPtr->st = 0;  
										        	SQLPtr->Offset = Offset; 
										        	SQLPtr->lastreadtime = ULONG_MAX;
										        	GetGlobalCVal ("[%PROPERTYDISPLAY]",str,"[GEOACCT]$CHR(1)[PROPADDR] [STREET] [DIRECTION] [SFX]$CHR(1)[DEEDVOLUME]/[DEEDPAGE]$CHR(1)[ACRES]");
								        			ExpandText (str); 
								        			OneSpace (str); 
								        			ReplaceChar (str,0x01,0x09); 
								        			_fstrcat (str,"\t[GEOACCT]");
								        			ExpandText (str);
									 				if ((idx=SendDlgItemMessage (hWndDlg,IDC_PROPERTYLIST,LB_ADDSTRING,0,(LPARAM)str)) ==
									 					LB_ERRSPACE) 
									 					GetNext = FALSE; 
									 			} 
									 			else
									 				GetNext = FALSE;
							        		}
							        		GlobalUnlock (FilePtr->FileHandle); 
							           		GlobalUnlock (SQLPtr->OFHandle);
							        		GlobalUnlock (hDBProp); 
											CloseDataFile (TRUE,&hDBProp); 
					        			
					        			
					        			}*/
					        			
					        			
						 				if ((idx=SendDlgItemMessage (hWndDlg,IDC_OWNERLIST,LB_ADDSTRING,0,(LPARAM)str)) ==
						 					LB_ERRSPACE) 
						 					GetNext = FALSE;
						 			} 
					 			}
					 			else
					 				GetNext = FALSE;
				 				if (GSSiPeekMessage(&msg,hwndCtl,WM_KEYDOWN,WM_KEYDOWN,PM_NOREMOVE))
				 					GetNext = FALSE;	                                                             
			        		}
			        		GlobalUnlock (FilePtr->FileHandle); 
			           		GlobalUnlock (SQLPtr->OFHandle);
			        		GlobalUnlock (hDBOwner);
                         }
                         if (First && SaveProperty >= 0) 
						 	PostMessage(GetDlgItem(hWndDlg,IDC_PROPERTYLIST), LB_SETSEL,TRUE,SaveProperty);  
                         First = FALSE;
                 	}
                 }
                 break;
			     
            case IDC_PROPERTYLIST: 
                 switch(HIWORD(wParam))
                 {   
					case LBN_SELCHANGE:
					{   
						HANDLE	hItems;
						int n=GetLBSelectedItems (hWndDlg,IDC_PROPERTYLIST,&hItems);   
						LPINT	pItem;
						
				    	SetDlgItemText (hWndDlg,IDC_PROPMESSAGE,""); 
			    		EnableWindow (GetDlgItem(hWndDlg,IDOK),FALSE);
						if (n>0)  
						{
							pItem=(LPINT)GlobalLock (hItems);   
							if (n==1)
							{   
								short	DoZoom=0;
								
						 		ClearHighlightList(FALSE); 
						 		while (n--)
						 		{   
						 			SendDlgItemMessage(hWndDlg,IDC_PROPERTYLIST,LB_GETTEXT,*pItem++,(DWORD)str);
						 			pTAB = _fstrrchr (str,'\t');
						 			pTAB++; 
						 			Strip (pTAB,' ');
									if (PickByRefno (0,"PINA",pTAB,-1))
									{   
										if (PickList[0].IsDeleted) 
											DoZoom = -1;
										else
										{
											AddToHighlightList (PickList[0].Refno,&PickList[0],TRUE); 
											DoZoom=1;
										}
									}
								} 
							    if (DoZoom>0)
							    {
						    		EnableWindow (GetDlgItem(hWndDlg,IDOK),TRUE);
							    	PostMessage(hWndMain, WM_COMMAND, IDM_Z_HLTLIMITS, 0L);
							    } 
							    else if (DoZoom < 0)
							    	SetDlgItemText (hWndDlg,IDC_PROPMESSAGE,"Property is marked deleted"); 
							    else
							    	SetDlgItemText (hWndDlg,IDC_PROPMESSAGE,"Property not found"); 
							}
							GSSiGlobUlFree (&hItems);
						}
					}
					break;
					                 	 
					case LBN_DBLCLK:  
					PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);
					IgnoreLbutton = TRUE;
					break; 
		 		 }
		 		 break;
		 		     
            case IDC_OWNERLIST: 
                 switch(HIWORD(wParam))
                 {   
                 	 case LBN_SELCHANGE: 
				    	SetDlgItemText (hWndDlg,IDC_PROPMESSAGE,""); 
						PostMessage(hWndDlg, WM_COMMAND, IDC_SETPROPLIST, 0L);
		 		 }
		 		 break;
		 	
		 	case IDC_SETPROPLIST:
		 	{
		 		 
		    	Item=SendDlgItemMessage(hWndDlg,IDC_OWNERLIST,LB_GETCURSEL,0,0);
		        if (Item != LB_ERR)
		        {    
		        	HANDLE	hDBProp=0;
						        	
			        SendDlgItemMessage(hWndDlg,IDC_OWNERLIST,LB_GETTEXT,Item,(DWORD)str);
			        pTAB = _fstrrchr (str,'\t');
			        *pTAB++ = 0;   
			        _fstrcpy (WantStr2,pTAB);
			        ReplaceChar (str,'\t','-');
			       //SetDlgItemText (hWndDlg,IDC_OWNERNAME,str); 
					SendDlgItemMessage (hWndDlg,IDC_PROPERTYLIST,LB_RESETCONTENT,0,0);
			        GetGlobalCVal ("[%PROPERTYDB]",str,"[%DL]attribut\\master.gmd");
				    if (!OpenDataFile (str, "",BT_READ, &hDBProp))
				    {
				        MessageBox( GetFocus(), str,"Cannot open property database", MB_OK);
				    }
				    else
				    {               
	   		      		short	IDIndex=2, IDLen=0; 
	   		      		short	pos=BT_FIRST, cond=BT_GE;
	   		      		BOOL	GetNext=TRUE;
			           		      		
					    SQLPtr = (LPOPENSQLDATA) GlobalLock (hDBProp);
					    FilePtr = (LPOPENFILEDATA) GlobalLock (SQLPtr->OFHandle);
						lpGWDHead = (LPGWDHEADER)GlobalLock (FilePtr->FileHandle);                             
						for (i=0;i<abs(lpGWDHead->NumIndexFields[IDIndex]);i++)
							IDLen += lpGWDHead->pFldInfo[lpGWDHead->IndexFields[IDIndex][i]].Len;
						_fstrncpy (lpGWDHead->pKeys[IDIndex],WantStr2,IDLen);
		        		while (GetNext && !BT_FIND (lpGWDHead->BTHandle[IDIndex],lpGWDHead->pKeys[IDIndex],pos,cond,(LPSTR)&Offset))
		        		{
		        			pos = BT_NEXT;
		        			cond = BT_ANY;
						        			
		        			lpGWDHead->pKeys[IDIndex][IDLen]=0;   
		        			Truncate (lpGWDHead->pKeys[IDIndex]);
		        			if (!_fstricmp (WantStr2,lpGWDHead->pKeys[IDIndex]))
		        			{
								FillGWDData (lpGWDHead,Offset); 
					        	SQLPtr->st = 0;  
					        	SQLPtr->Offset = Offset; 
					        	SQLPtr->lastreadtime = ULONG_MAX;
					        	GetGlobalCVal ("[%PROPERTYDISPLAY]",str,"[GEOACCT]$CHR(1)[PROPADDR] [STREET] [DIRECTION] [SFX]$CHR(1)[DEEDVOLUME]/[DEEDPAGE]$CHR(1)[ACRES]");
			        			ExpandText (str); 
			        			OneSpace (str); 
					        	GetGlobalCVal ("[%PROPERTYKEYFIELD]",propkeyfld,"[GEOACCT]");
			        			ReplaceChar (str,0x01,0x09); 
			        			sprintf (_fstrchr(str,0),"\t%s",propkeyfld);
			        			ExpandText (str);
				 				if ((idx=SendDlgItemMessage (hWndDlg,IDC_PROPERTYLIST,LB_ADDSTRING,0,(LPARAM)str)) ==
				 					LB_ERRSPACE) 
				 					GetNext = FALSE; 
				 			} 
				 			else
				 				GetNext = FALSE;
		        		}
		        		GlobalUnlock (FilePtr->FileHandle); 
		           		GlobalUnlock (SQLPtr->OFHandle);
		        		GlobalUnlock (hDBProp); 
						CloseDataFile (TRUE,&hDBProp); 
				    }
	     	 	}  
	     	}
     	 	break; 
                 	 	
		 	case IDOK:
		 	{   
		 		HANDLE	hItems; 
				int	 n=GetLBSelectedItems (hWndDlg,IDC_PROPERTYLIST,&hItems);
				LPINT	pItem=(LPINT)GlobalLock (hItems);  
				LPSTR	pTAB; 
				BOOL	Err=FALSE;
				
				if (n == 1)
					SaveProperty = *pItem;
				else
					SaveProperty = -1;
				iOwner = SendDlgItemMessage(hWndDlg,IDC_OWNERLIST,LB_GETCURSEL,0,0);
		 		SendDlgItemMessage(hWndDlg,IDC_OWNERLIST,LB_GETTEXT,iOwner,(DWORD)str); 
		 		if ((pTAB = _fstrchr (str,'\t')))
		 			*pTAB = 0;
				SetGlobalValue ("%SELECTEDOWNER",str);
		 		ClearHighlightList(FALSE); 
		    	AutoHighlight=SendDlgItemMessage(hWndDlg,IDC_AUTOHIGHLIGHT,BM_GETCHECK,0,0); 
		 		while (n--)
		 		{   
		 			SendDlgItemMessage(hWndDlg,IDC_PROPERTYLIST,LB_GETTEXT,*pItem++,(DWORD)str);
		 			pTAB = _fstrrchr (str,'\t');
		 			pTAB++; 
		 			Strip (pTAB,' ');
					if (PickByRefno (0,"PINA",pTAB,-1))
					{
						if (PickList[0].Type != 6 && AutoHighlight) 
							AddToHighlightList (PickList[0].Refno,&PickList[0],TRUE); 
					}
					else 
					{
						Err = TRUE;
				    	SetDlgItemText (hWndDlg,IDC_PROPMESSAGE,"Property not found"); 
				    }
				} 
				GSSiGlobUlFree (&hItems);
				CloseDataFile (TRUE,&hDBOwner);
				if (Err)
					break; 
			    GetDlgItemText (hWndDlg,IDC_OWNERNAME,WantStr,sizeof(WantStr)-1); 
			    GetDlgItemText (hWndDlg,IDC_OWNERNAMEWC,Wildcard,sizeof(Wildcard)-1);            
	            GSSiEndDialog(hWndDlg, TRUE,hSaveBM);  
			    //PostMessage(hWndMain, WM_COMMAND, IDM_Z_HLTLIMITS, 0L);
	            
	        }
		 	     break;   
		 	     
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
				 CloseDataFile (TRUE,&hDBOwner); 
                 GSSiEndDialog(hWndDlg, FALSE,hSaveBM);
                 break;
           }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
}  

int GetWIHits (LPINT Hits,UINT nParts,LPINT nPartOffsets,LPHANDLE hPartOffsets,int MaxHits,HWND hwndCtl)
{
    int	nHits = 0;
	int	Order[32];
	UINT	i,j,k,ihit, ipart, nValidParts=0,ii;
	LPLONG	pOff1, pOff2;
	BOOL	HaveHit;
	MSG		msg;

	Order[0] = 0;
	for (ipart=1;ipart<nParts;ipart++)
	{
		for (i=0;i<ipart;i++)
		{
			if (nPartOffsets[ipart] < nPartOffsets[Order[i]])
			{
				for (j=ipart;j>i;j--)
					Order[j] = Order[j-1];
				Order[i] = ipart;
				goto Inserted;
			}
		}
		Order[ipart] = ipart;
Inserted:;
	}

	for (ipart=0;ipart<nParts;ipart++)
		if (nPartOffsets[ipart] < LONG_MAX)
			nValidParts++;

	if (!nValidParts)
		return 0;
	pOff1 = GlobalLock (hPartOffsets[Order[0]]);
	for (i=0;i<nPartOffsets[Order[0]];i++)
	{
		HaveHit = TRUE;
		for (j=1;j<nValidParts;j++)
		{
			pOff2 = GlobalLock (hPartOffsets[Order[j]]);
			for (k=0;k<nPartOffsets[Order[j]];k++)
				if (*pOff1 == *pOff2++)
					goto Next;
			HaveHit = FALSE;
Next:
			GlobalUnlock (hPartOffsets[Order[j]]);
			if (!HaveHit)
				goto NextOff; 
		}
		if (nHits >= MaxHits)
			break;
		for (ihit =0;ihit<nHits;ihit++)
			if (Hits[ihit] == *pOff1)
				goto NextOff;
		Hits[nHits++] = *pOff1;
		if (!*pOff1)
			ii=1;
NextOff:
		pOff1++;
		if (GSSiPeekMessage(&msg,hwndCtl,WM_KEYDOWN,WM_KEYDOWN,PM_NOREMOVE))
			break;	                                                             
	}
	GlobalUnlock (hPartOffsets[Order[0]]);
	return nHits;
}

BOOL FAR PASCAL OWNERLOCMsgProc2(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{ 	int		st, choice, n,idx,ifile,i; 
	int		Trigger=3;
	BOOL	HaveWildcard;
	char	str[1024], teststr[1024],lastowner[4096];
	static	char Wildcard[128]="";
	static	char WantStr[128]=""; 
	char	WantStr2[128],propkeyfld[64];
	LPGWDHEADER lpGWDHeadNameIndex, lpGWDHeadName, lpGWDHeadProperty;
    LPOPENFILEDATA  FilePtrNameIndex, FilePtrName, FilePtrProperty;
    LPOPENSQLDATA   SQLPtrNameIndex, SQLPtrName, SQLPtrProperty;
	long	Offset;
	int		nParts, nPartsTest, PartLen[MAXPARTS], PartLenTest[MAXPARTS];
	LPSTR	PartLoc[MAXPARTS], PartLocTest[MAXPARTS], pTab;
	static	HANDLE	hDBNameIndex=0, hDBName=0, hDBProperty=0;
	static	HANDLE	hSaveBM=0;  
	static	BOOL	First;  
	static	short	iOwner=-1, SaveProperty=-1; 
	short	Item;
	char	propprefix[32];
	TAGKEY TAGKey;  
	LPSTR	pTAB; 
	HWND	hwndCtl;
	int		nowner;
	static	char	NameDelim;
	UINT	nPrompts=4;
	UINT	PrmtDat[4*3] = {
							  IDC_OWNERNAMEWC,PRMT_IDC_OWNERNAMEWC,0,
							  IDC_OWNERNAME, PRMT_IDC_OWNERNAME,0,  
							  IDC_OWNERLIST, PRMT_IDC_OWNERLIST,0,
							  IDC_PROPERTYLIST, PRMT_IDC_PROPERTYLIST,0
							};


 int	BRtn;
 if (Message==WM_INITDIALOG)
 {  
 	UINT	pw=0, pn=1, pm=2; 
 	HWND	hwnd;
 	short	ii;
 	
 	InitDlgPrompts (hWndDlg);
 	while (nPrompts--)
 	{   
 		hwnd = GetDlgItem(hWndDlg,PrmtDat[pw]);
 		if (!hwnd)
 			ii=1;
	 	SetDlgPrompt (hwnd,PrmtDat[pn],PrmtDat[pm]);
	 	pw+=3;
	 	pn+=3; 
	 	pm+=3;
	}
 }
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
 	return (BRtn);
 switch(Message)
   {
 	case WM_DESTROY: 
	 	InitDlgPrompts (0);    
	 	break;
    case WM_INITDIALOG:  
	{   
		int	nTabs;
		HANDLE	hList;
		LPINT	pTabs;

    	ClearDlgPrompts (); 
    	//hSaveBM = EnterBlockingWindow (hWndDlg);
		First = TRUE;
		SendDlgItemMessage (hWndDlg,IDC_AUTOHIGHLIGHT,BM_SETCHECK,AutoHighlight,0L);
		GetGlobalCVal ("[%OWNERTABS]",str,"250 2000");
		nTabs = GetIntsFromList (str,&hList); 
		if (nTabs)
		{
			pTabs = (LPINT)GlobalLock (hList);
			SendDlgItemMessage (hWndDlg,IDC_OWNERLIST,LB_SETTABSTOPS,nTabs,(LPARAM)pTabs);
			GSSiGlobUlFree (&hList);
		}
		GetGlobalCVal ("[%PROPERTYTABS]",str,"80 200 250 2000");
		nTabs = GetIntsFromList (str,&hList); 
		if (nTabs)
		{
			pTabs = (LPINT)GlobalLock (hList);
			SendDlgItemMessage (hWndDlg,IDC_PROPERTYLIST,LB_SETTABSTOPS,nTabs,(LPARAM)pTabs);
			GSSiGlobUlFree (&hList);
		}
        cwCenter(hWndDlg, 0); 
        GetGlobalCVal ("[%NAMEINDEXDB]",str,"[%DL]attribut\\nameindex.gmd");
	    if (!OpenDataFile (str, "",BT_READ, &hDBNameIndex))
	    {
	        MessageBox( GetFocus(), str,"Cannot open name index database", MB_OK);
	        PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
	        return FALSE;
	    }  
		CloseDataFile (TRUE,&hDBNameIndex);
        GetGlobalCVal ("[%NAMEDB]",str,"[%DL]attribut\\name.gmd");
	    if (!OpenDataFile (str, "",BT_READ, &hDBName))
	    {
	        MessageBox( GetFocus(), str,"Cannot open name database", MB_OK);
	        PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
	        return FALSE;
	    }  
		CloseDataFile (TRUE,&hDBName);
		GetGlobalCVal ("[%PROPERTYDB]",str,"[%DL]attribut\\master.gmd");
		if (!OpenDataFile (str, "",BT_READ, &hDBProperty))
		{
			MessageBox( GetFocus(), str,"Cannot open property database", MB_OK);
	        PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
	        return FALSE;
		}
		CloseDataFile (TRUE,&hDBProperty);
	    SetDlgItemText (hWndDlg,IDC_OWNERNAMEWC,Wildcard);            
		PostMessage(GetDlgItem(hWndDlg,IDC_OWNERLIST), LB_SETCURSEL,iOwner,0); 
		PostMessage(hWndDlg, WM_COMMAND, IDC_SETPROPLIST, 0L);
	 	PostMessage(GetDlgItem(hWndDlg,IDC_PROPERTYLIST), LB_SETSEL,TRUE,SaveProperty);  
	} 

        break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(LOWORD(wParam))
           {
			case IDC_OWNERNAMEWC: 
		    	 SetDlgItemText (hWndDlg,IDC_PROPMESSAGE,""); 
				 hwndCtl = (HWND) LOWORD(lParam);
                 switch (HIWORD(wParam))
                 {	case EN_CHANGE: 
                 	{   
						int	nParts, ipart,iparttest, nOwners;
						LPWORDINDEX	pRec;
						WORDINDEX	Rec;
						HANDLE	hPartOffsets[MAXPARTS];
						int		nPartOffsets[MAXPARTS];
						int		nHits;
       		      		BOOL	GetNext = TRUE;   
   		      			MSG     msg; 
						char	Name[66];

						SetDlgItemText (hWndDlg,IDC_NUMOWNERS,"Searching..."); 
				        GetGlobalCVal ("[%NAMEINDEXDB]",str,"[%DL]attribut\\nameindex.gmd");
						OpenDataFile (str, "",BT_READ, &hDBNameIndex);
 						SQLPtrNameIndex = (LPOPENSQLDATA) GlobalLock (hDBNameIndex);
						FilePtrNameIndex = (LPOPENFILEDATA) GlobalLock (SQLPtrNameIndex->OFHandle);
						lpGWDHeadNameIndex = (LPGWDHEADER)GlobalLock (FilePtrNameIndex->FileHandle); 
                        pRec = (LPWORDINDEX)&lpGWDHeadNameIndex->GWDData;
						SendDlgItemMessage (hWndDlg,IDC_OWNERLIST,LB_RESETCONTENT,0,0);
						SendDlgItemMessage (hWndDlg,IDC_PROPERTYLIST,LB_RESETCONTENT,0,0);   
           		      	GetDlgItemText (hWndDlg,IDC_OWNERNAMEWC,Name,64);
						nParts = GetNameParts (Name,PartLoc,PartLen,MAXPARTS);
           		      	
 	           		    for (ipart=0;ipart<nParts;ipart++)
						{
                            nPartOffsets[ipart] = 0;
							hPartOffsets[ipart] = 0;
						}
          		      	for (ipart=0;ipart<nParts;ipart++)
           		      	{   
           		      		short	pos=BT_FIRST, cond=BT_GE;
							int		nOffsets, nSeqRec;
                            
							if (PartLen[ipart] >= Trigger)
							{
								strncpy (Rec.Word,PartLoc[ipart],8);
								Rec.Seq = SHRT_MIN;
			        			while (GetNext && !BT_FIND (lpGWDHeadNameIndex->BTHandle[0],(LPSTR)&Rec,pos,cond,(LPSTR)&Offset))
			        			{
									LPLONG	pOffsets;

									pos = BT_NEXT;
			        				cond = BT_ANY;  
			        				
									FillGWDData (lpGWDHeadNameIndex,Offset); 
			        				if (strnicmp (pRec->Word,PartLoc[ipart],min(8,PartLen[ipart])))
										break;
									nSeqRec = -(pRec->Seq + 1);
			        				nOffsets = nSeqRec * 100 + pRec->nOffsets;
									if (!nPartOffsets[ipart])
										hPartOffsets[ipart] = GSSiGlobAlloc (0,GMEM_MOVEABLE,MAXOFFSETS * sizeof(int));
									pOffsets = (LPLONG)GlobalLock (hPartOffsets[ipart]);
									if (nPartOffsets[ipart] + pRec->nOffsets >= MAXOFFSETS)
										GetNext = FALSE;
									else
									{
										pOffsets += nPartOffsets[ipart];
										memmove (pOffsets,pRec->Offsets,pRec->nOffsets*sizeof(long));
										pOffsets += pRec->nOffsets;
										nPartOffsets[ipart] += pRec->nOffsets;
										while (GetNext && nSeqRec--)
										{
											BT_FIND (lpGWDHeadNameIndex->BTHandle[0],(LPSTR)pRec,pos,cond,(LPSTR)&Offset);
											FillGWDData (lpGWDHeadNameIndex,Offset); 
											if (nPartOffsets[ipart] + pRec->nOffsets >= MAXOFFSETS)
												GetNext = FALSE;
											else
											{
												memmove (pOffsets,pRec->Offsets,pRec->nOffsets*sizeof(long));
												pOffsets += pRec->nOffsets;
												nPartOffsets[ipart] += pRec->nOffsets;
											}
										}
									}
									GlobalUnlock (hPartOffsets[ipart]);
				 					if (GSSiPeekMessage(&msg,hwndCtl,WM_KEYDOWN,WM_KEYDOWN,PM_NOREMOVE))
				 						GetNext = FALSE;	                                                             
								}
								if (!nPartOffsets[ipart])
									GetNext = FALSE;
							}
							else
								nPartOffsets[ipart] = INT_MAX;
	                 	}

						if (GetNext)
							nHits = GetWIHits (Hits,nParts,nPartOffsets,hPartOffsets,MAXHITS,hwndCtl);
	           			for (ipart=0;ipart<nParts;ipart++)
							GSSiGlobFree (&hPartOffsets[ipart]);
 						GlobalUnlock (FilePtrNameIndex->FileHandle); 
						GlobalUnlock (SQLPtrNameIndex->OFHandle);
						GlobalUnlock (hDBNameIndex);
						CloseDataFile (TRUE,&hDBNameIndex);
						GetGlobalCVal ("[%NAMEDB]",str,"[%DL]attribut\\name.gmd");
						OpenDataFile (str, "",BT_READ, &hDBName);
						SQLPtrName = (LPOPENSQLDATA) GlobalLock (hDBName);
						FilePtrName = (LPOPENFILEDATA) GlobalLock (SQLPtrName->OFHandle);
						lpGWDHeadName = (LPGWDHEADER)GlobalLock (FilePtrName->FileHandle); 
						nowner = 0;
						while (GetNext && nHits--)
						{
							BOOL	ShowThisOwner = TRUE;

							if (Hits[nHits] <= sizeof (GWDHEADER16))
								continue;
							FillGWDData (lpGWDHeadName,Hits[nHits]); 
							SQLPtrName->st = 0;  
							SQLPtrName->Offset = Hits[nHits]; 
							SQLPtrName->lastreadtime = ULONG_MAX;
			        		if (ShowThisOwner)
			        		{
								if (nowner == 81)
									idx = 0;
								GetGlobalCVal ("[%NAMEDISPLAY]",str,"[NAMETAXPAYER]$CHR(1)[NAMEADDR1] [NAMEADDR2] [NAMECITY],[NAMESTATE]$CHR(1)[NAMEACCTOWNER]");
								ExpandText (str); 
								OneSpace (str); 
								ReplaceChar (str,0x01,0x09);
 								strcpy (teststr,str);
								if ((pTab = strchr (teststr,'\t')))
 									*pTab = 0;
								nPartsTest = GetNameParts (teststr,PartLocTest,PartLenTest,MAXPARTS);
								for (ipart = 0;ipart < nParts;ipart++)
								{
									for (iparttest=0;iparttest<nPartsTest;iparttest++)
									{
										if (!strnicmp (PartLoc[ipart],PartLocTest[iparttest],PartLen[ipart]))
										{
											*PartLocTest[iparttest] = 0;
											goto FoundPart;
										}
									}
									ShowThisOwner = FALSE;
									break;
FoundPart:							;
								}
								if (ShowThisOwner)
								{
									nowner++;
									strcpy (lastowner,str);
						 			if ((idx=SendDlgItemMessage (hWndDlg,IDC_OWNERLIST,LB_ADDSTRING,0,(LPARAM)str)) ==
						 				LB_ERRSPACE) 
						 				GetNext = FALSE;
								}
							} 
				 			if (GSSiPeekMessage(&msg,hwndCtl,WM_KEYDOWN,WM_KEYDOWN,PM_NOREMOVE))
				 				GetNext = FALSE;	                                                             
						}
						GlobalUnlock (FilePtrName->FileHandle); 
						GlobalUnlock (SQLPtrName->OFHandle);
						GlobalUnlock (hDBName);
						CloseDataFile (TRUE,&hDBName);
						if (GetNext)
						{
							nOwners=SendDlgItemMessage(hWndDlg,IDC_OWNERLIST,LB_GETCOUNT,0,0);
							sprintf (str,"%ld selected",nOwners);
						}
						else
							strcpy (str,"Search cancelled");
		    			SetDlgItemText (hWndDlg,IDC_NUMOWNERS,str); 
					 }
					 if (First && SaveProperty >= 0) 
						PostMessage(GetDlgItem(hWndDlg,IDC_PROPERTYLIST), LB_SETSEL,TRUE,SaveProperty);  
					 First = FALSE;
                 }
                 break;
			     
            case IDC_PROPERTYLIST: 
                 switch(HIWORD(wParam))
                 {   
					case LBN_SELCHANGE:
					{   
						HANDLE	hItems;
						int n=GetLBSelectedItems (hWndDlg,IDC_PROPERTYLIST,&hItems);   
						LPINT	pItem;
						
				    	SetDlgItemText (hWndDlg,IDC_PROPMESSAGE,""); 
			    		EnableWindow (GetDlgItem(hWndDlg,IDOK),FALSE);
						if (n>0)  
						{
							pItem=(LPINT)GlobalLock (hItems);   
							//if (n==1)
							{   
								short	DoZoom=0;

								GetGlobalCVal ("[%PROPERTYPREFIX]",propprefix,"PINA");
								
						 		ClearHighlightList(FALSE); 
						 		while (n--)
						 		{   
						 			SendDlgItemMessage(hWndDlg,IDC_PROPERTYLIST,LB_GETTEXT,*pItem++,(DWORD)str);
						 			pTAB = _fstrrchr (str,'\t');
						 			pTAB++; 
						 			Strip (pTAB,' ');
									if (PickByRefno (0,propprefix,pTAB,-1))
									{   
										if (PickList[0].IsDeleted) 
											DoZoom = -1;
										else
										{
											AddToHighlightList (PickList[0].Refno,&PickList[0],TRUE); 
											DoZoom=1;
										}
									}
								} 
							    if (DoZoom>0)
							    {
						    		EnableWindow (GetDlgItem(hWndDlg,IDOK),TRUE);
							    	PostMessage(hWndMain, WM_COMMAND, IDM_Z_HLTLIMITS, 0L);
							    } 
							    else if (DoZoom < 0)
							    	SetDlgItemText (hWndDlg,IDC_PROPMESSAGE,"Property is marked deleted"); 
							    else
							    	SetDlgItemText (hWndDlg,IDC_PROPMESSAGE,"Property not found"); 
							}
							GSSiGlobUlFree (&hItems);
						}
					}
					break;
					                 	 
					case LBN_DBLCLK:  
					PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);
					IgnoreLbutton = TRUE;
					break; 
		 		 }
		 		 break;
		 		     
            case IDC_OWNERLIST: 
                 switch(HIWORD(wParam))
                 {   
                 	 case LBN_SELCHANGE: 
				    	SetDlgItemText (hWndDlg,IDC_PROPMESSAGE,""); 
						PostMessage(hWndDlg, WM_COMMAND, IDC_SETPROPLIST, 0L);
		 		 }
		 		 break;
		 	
		 	case IDC_SETPROPLIST:
		 	{
			    GetGlobalCVal ("[%PROPERTYDB]",str,"[%DL]attribut\\master.gmd");
				if (!OpenDataFile (str, "",BT_READ, &hDBProperty))
				{
				    MessageBox( GetFocus(), str,"Cannot open property database", MB_OK);
				}
				else
				{
		 			HANDLE	hItems=0; 
					int	 n=GetLBSelectedItems (hWndDlg,IDC_OWNERLIST,&hItems);
					LPINT	pItem;
					
					SendDlgItemMessage (hWndDlg,IDC_PROPERTYLIST,LB_RESETCONTENT,0,0);
					if (n)
					{
						pItem = (LPINT)GlobalLock (hItems);  
						while (n--)
						{    
							
							Item = *pItem++;
							SendDlgItemMessage(hWndDlg,IDC_OWNERLIST,LB_GETTEXT,Item,(DWORD)str);
							if ((pTAB = _fstrrchr (str,'\t')))
							{
								*pTAB++ = 0;   
								_fstrcpy (WantStr2,pTAB);
								ReplaceChar (str,'\t','-');
							   //SetDlgItemText (hWndDlg,IDC_OWNERNAME,str); 
								{               
	   		      					short	IDIndex=GetGlobalLVal2 ("[%PROPERTYINDEX]",0), IDLen=0; 
	   		      					short	pos=BT_FIRST, cond=BT_GE;
	   		      					BOOL	GetNext=TRUE;
									char	propindexfld[66];
			           		      					
									SQLPtrProperty = (LPOPENSQLDATA) GlobalLock (hDBProperty);
									FilePtrProperty = (LPOPENFILEDATA) GlobalLock (SQLPtrProperty->OFHandle);
									lpGWDHeadProperty = (LPGWDHEADER)GlobalLock (FilePtrProperty->FileHandle);                             
									//for (i=0;i<abs(lpGWDHeadProperty->NumIndexFields[IDIndex]);i++)
									//	IDLen += lpGWDHeadProperty->pFldInfo[lpGWDHeadProperty->IndexFields[IDIndex][i]].Len;
									//_fstrncpy (lpGWDHeadProperty->pKeys[IDIndex],WantStr2,IDLen);
					        		GetGlobalCVal ("[%PROPERTYINDEXFIELD]",propindexfld,"[PID]");
							 		SetFieldValFromCharAndName(lpGWDHeadProperty,propindexfld,(LPSTR)WantStr2,FALSE);
									GWDFormKey(lpGWDHeadProperty,IDIndex,TRUE,0,0);

		        					while (GetNext && !BT_FIND (lpGWDHeadProperty->BTHandle[IDIndex],lpGWDHeadProperty->pKeys[IDIndex],pos,cond,(LPSTR)&Offset))
		        					{
										char	CharKeyVal[128];

										pos = BT_NEXT;
		        						cond = BT_ANY;
						        						
		        						//lpGWDHeadProperty->pKeys[IDIndex][IDLen]=0;   
		        						//Truncate (lpGWDHeadProperty->pKeys[IDIndex]);
						                GMDGetCharKeyVal (lpGWDHeadProperty,IDIndex,0,CharKeyVal); 
		        						if (!_fstricmp (WantStr2,CharKeyVal))
		        						{
											FillGWDData (lpGWDHeadProperty,Offset); 
					        				SQLPtrProperty->st = 0;  
					        				SQLPtrProperty->Offset = Offset; 
					        				SQLPtrProperty->lastreadtime = ULONG_MAX;
					        				GetGlobalCVal ("[%PROPERTYDISPLAY]",str,"[GEOACCT]$CHR(1)[PROPADDR] [STREET] [DIRECTION] [SFX]$CHR(1)[DEEDVOLUME]/[DEEDPAGE]$CHR(1)[ACRES]");
			        						ExpandText (str); 
			        						OneSpace (str); 
					        				//GetGlobalCVal ("[%PROPERTYKEYFIELD]",propkeyfld,"[GEOACCT]");
			        						ReplaceChar (str,0x01,0x09); 
			        						//sprintf (_fstrchr(str,0),"\t%s",propkeyfld);
			        						//ExpandText (str);
				 							if ((idx=SendDlgItemMessage (hWndDlg,IDC_PROPERTYLIST,LB_ADDSTRING,0,(LPARAM)str)) ==
				 								LB_ERRSPACE) 
				 								GetNext = FALSE; 
				 						} 
				 						else
				 							GetNext = FALSE;
		        					}
		        					GlobalUnlock (FilePtrProperty->FileHandle); 
		           					GlobalUnlock (SQLPtrProperty->OFHandle);
		        					GlobalUnlock (hDBProperty); 
								}
							}
						}
					}
					CloseDataFile (TRUE,&hDBProperty); 
					GSSiGlobUlFree (&hItems);
	     	 	}  
	     	}
     	 	break; 
                 	 	
		 	case IDOK:
		 	{   
		 		HANDLE	hItems; 
				int	 n=GetLBSelectedItems (hWndDlg,IDC_PROPERTYLIST,&hItems);
				LPINT	pItem=(LPINT)GlobalLock (hItems);  
				LPSTR	pTAB; 
				BOOL	Err=TRUE;
				
				if (n == 1)
					SaveProperty = *pItem;
				else
					SaveProperty = -1;
				iOwner = SendDlgItemMessage(hWndDlg,IDC_OWNERLIST,LB_GETCURSEL,0,0);
		 		SendDlgItemMessage(hWndDlg,IDC_OWNERLIST,LB_GETTEXT,iOwner,(DWORD)str); 
		 		if ((pTAB = _fstrchr (str,'\t')))
		 			*pTAB = 0;
				SetGlobalValue ("%SELECTEDOWNER",str);
		 		ClearHighlightList(FALSE); 
		    	AutoHighlight=SendDlgItemMessage(hWndDlg,IDC_AUTOHIGHLIGHT,BM_GETCHECK,0,0); 
		 		while (n--)
		 		{   
		 			SendDlgItemMessage(hWndDlg,IDC_PROPERTYLIST,LB_GETTEXT,*pItem++,(DWORD)str);
		 			pTAB = _fstrrchr (str,'\t');
		 			pTAB++; 
		 			Strip (pTAB,' ');
					GetGlobalCVal ("[%PROPERTYPREFIX]",propprefix,"PINA");
					if (PickByRefno (0,propprefix,pTAB,-1))
					{
						if (PickList[0].Type != 6 && AutoHighlight) 
							AddToHighlightList (PickList[0].Refno,&PickList[0],TRUE); 
						Err=FALSE;
					}
					else 
					{
				    	SetDlgItemText (hWndDlg,IDC_PROPMESSAGE,"Property not found"); 
				    }
				} 
				GSSiGlobUlFree (&hItems);
				if (Err)
					break; 
			    GetDlgItemText (hWndDlg,IDC_OWNERNAME,WantStr,sizeof(WantStr)-1); 
			    GetDlgItemText (hWndDlg,IDC_OWNERNAMEWC,Wildcard,sizeof(Wildcard)-1);            
	            GSSiEndDialog(hWndDlg, 2,hSaveBM); 
				ExpandBounds (&HLTBounds, LocationOffset);
				ZoomToRect(HLTBounds,FALSE);  
//			    PostMessage(hWndMain, WM_COMMAND, IDM_Z_HLTLIMITS, 0L);
	            
	        }
		 	     break;   
		 	     
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
				 CloseDataFile (TRUE,&hDBName); 
                 GSSiEndDialog(hWndDlg, FALSE,hSaveBM);
                 break;
           }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
}      

BOOL FAR PASCAL OWNERLOCMsgProc3(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{ 	int		st, choice, n,idx,ifile,i; 
	int		Trigger=3;
	BOOL	HaveWildcard;
	char	str[1024], teststr[1024],lastowner[4096];
	static	char Wildcard[128]="";
	static	char WantStr[128]=""; 
	char	WantStr2[128],propkeyfld[64];
	LPGWDHEADER lpGWDHeadNameIndex, lpGWDHeadName, lpGWDHeadProperty;
    LPOPENFILEDATA  FilePtrNameIndex, FilePtrName, FilePtrProperty;
    LPOPENSQLDATA   SQLPtrNameIndex, SQLPtrName, SQLPtrProperty;
	long	Offset;
	int		nParts, nPartsTest, PartLen[MAXPARTS], PartLenTest[MAXPARTS];
	LPSTR	PartLoc[MAXPARTS], PartLocTest[MAXPARTS], pTab;
	static	HANDLE	hDBNameIndex=0, hDBName=0, hDBProperty=0;
	static	HANDLE	hSaveBM=0;  
	static	BOOL	First;  
	static	short	iOwner=-1, SaveProperty=-1; 
	short	Item;
	char	propprefix[32];
	TAGKEY TAGKey;  
	LPSTR	pTAB; 
	HWND	hwndCtl;
	int		nowner;
	static	char	NameDelim;
	UINT	nPrompts=4;
	UINT	PrmtDat[4*3] = {
							  IDC_OWNERNAMEWC,PRMT_IDC_OWNERNAMEWC,0,
							  IDC_OWNERNAME, PRMT_IDC_OWNERNAME,0,  
							  IDC_OWNERLIST, PRMT_IDC_OWNERLIST,0,
							  IDC_PROPERTYLIST, PRMT_IDC_PROPERTYLIST,0
							};


 int	BRtn;
 if (Message==WM_INITDIALOG)
 {  
 	UINT	pw=0, pn=1, pm=2; 
 	HWND	hwnd;
 	short	ii;
 	
 	InitDlgPrompts (hWndDlg);
 	while (nPrompts--)
 	{   
 		hwnd = GetDlgItem(hWndDlg,PrmtDat[pw]);
 		if (!hwnd)
 			ii=1;
	 	SetDlgPrompt (hwnd,PrmtDat[pn],PrmtDat[pm]);
	 	pw+=3;
	 	pn+=3; 
	 	pm+=3;
	}
 }
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
 	return (BRtn);
 switch(Message)
   {
 	case WM_DESTROY: 
	 	InitDlgPrompts (0);    
	 	break;
    case WM_INITDIALOG:  
	{   
		int	nTabs;
		HANDLE	hList;
		LPINT	pTabs;

    	ClearDlgPrompts (); 
    	//hSaveBM = EnterBlockingWindow (hWndDlg);
		First = TRUE;
		SendDlgItemMessage (hWndDlg,IDC_AUTOHIGHLIGHT,BM_SETCHECK,AutoHighlight,0L);
		GetGlobalCVal ("[%OWNERTABS]",str,"250 2000");
		nTabs = GetIntsFromList (str,&hList); 
		if (nTabs)
		{
			pTabs = (LPINT)GlobalLock (hList);
			SendDlgItemMessage (hWndDlg,IDC_OWNERLIST,LB_SETTABSTOPS,nTabs,(LPARAM)pTabs);
			GSSiGlobUlFree (&hList);
		}
		GetGlobalCVal ("[%PROPERTYTABS]",str,"80 200 250 2000");
		nTabs = GetIntsFromList (str,&hList); 
		if (nTabs)
		{
			pTabs = (LPINT)GlobalLock (hList);
			SendDlgItemMessage (hWndDlg,IDC_PROPERTYLIST,LB_SETTABSTOPS,nTabs,(LPARAM)pTabs);
			GSSiGlobUlFree (&hList);
		}
        cwCenter(hWndDlg, 0); 
        GetGlobalCVal ("[%NAMEINDEXDB]",str,"[%DL]attribut\\nameindex.gmd");
	    if (!OpenDataFile (str, "",BT_READ, &hDBNameIndex))
	    {
	        MessageBox( GetFocus(), str,"Cannot open name index database", MB_OK);
	        PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
	        return FALSE;
	    }  
		CloseDataFile (TRUE,&hDBNameIndex);
        GetGlobalCVal ("[%NAMEDB]",str,"[%DL]attribut\\name.gmd");
	    if (!OpenDataFile (str, "",BT_READ, &hDBName))
	    {
	        MessageBox( GetFocus(), str,"Cannot open name database", MB_OK);
	        PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
	        return FALSE;
	    }  
		CloseDataFile (TRUE,&hDBName);
		GetGlobalCVal ("[%PROPERTYDB]",str,"[%DL]attribut\\master.gmd");
		if (!OpenDataFile (str, "",BT_READ, &hDBProperty))
		{
			MessageBox( GetFocus(), str,"Cannot open property database", MB_OK);
	        PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
	        return FALSE;
		}
		CloseDataFile (TRUE,&hDBProperty);
	    SetDlgItemText (hWndDlg,IDC_OWNERNAMEWC,Wildcard);            
		PostMessage(GetDlgItem(hWndDlg,IDC_OWNERLIST), LB_SETCURSEL,iOwner,0); 
		PostMessage(hWndDlg, WM_COMMAND, IDC_SETPROPLIST, 0L);
	 	PostMessage(GetDlgItem(hWndDlg,IDC_PROPERTYLIST), LB_SETSEL,TRUE,SaveProperty);  
	} 

        break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(LOWORD(wParam))
           {
			case IDC_OWNERNAMEWC: 
		    	 SetDlgItemText (hWndDlg,IDC_PROPMESSAGE,""); 
				 hwndCtl = (HWND) LOWORD(lParam);
                 switch (HIWORD(wParam))
                 {	case EN_CHANGE: 
                 	{   
						int	nParts, ipart,iparttest, nOwners;
						LPWORDINDEX	pRec;
						WORDINDEX	Rec;
						HANDLE	hPartOffsets[MAXPARTS];
						int		nPartOffsets[MAXPARTS];
						int		nHits;
       		      		BOOL	GetNext = TRUE;   
   		      			MSG     msg; 
						char	Name[66];

						SetDlgItemText (hWndDlg,IDC_NUMOWNERS,"Searching..."); 
				        GetGlobalCVal ("[%NAMEINDEXDB]",str,"[%DL]attribut\\nameindex.gmd");
						OpenDataFile (str, "",BT_READ, &hDBNameIndex);
 						SQLPtrNameIndex = (LPOPENSQLDATA) GlobalLock (hDBNameIndex);
						FilePtrNameIndex = (LPOPENFILEDATA) GlobalLock (SQLPtrNameIndex->OFHandle);
						lpGWDHeadNameIndex = (LPGWDHEADER)GlobalLock (FilePtrNameIndex->FileHandle); 
                        pRec = (LPWORDINDEX)&lpGWDHeadNameIndex->GWDData;
						SendDlgItemMessage (hWndDlg,IDC_OWNERLIST,LB_RESETCONTENT,0,0);
						SendDlgItemMessage (hWndDlg,IDC_PROPERTYLIST,LB_RESETCONTENT,0,0);   
           		      	GetDlgItemText (hWndDlg,IDC_OWNERNAMEWC,Name,64);
						nParts = GetNameParts (Name,PartLoc,PartLen,MAXPARTS);
           		      	
 	           		    for (ipart=0;ipart<nParts;ipart++)
						{
                            nPartOffsets[ipart] = 0;
							hPartOffsets[ipart] = 0;
						}
          		      	for (ipart=0;ipart<nParts;ipart++)
           		      	{   
           		      		short	pos=BT_FIRST, cond=BT_GE;
							int		nOffsets, nSeqRec;
                            
							if (PartLen[ipart] >= Trigger)
							{
								strncpy (Rec.Word,PartLoc[ipart],8);
								Rec.Seq = SHRT_MIN;
			        			while (GetNext && !BT_FIND (lpGWDHeadNameIndex->BTHandle[0],(LPSTR)&Rec,pos,cond,(LPSTR)&Offset))
			        			{
									LPLONG	pOffsets;

									pos = BT_NEXT;
			        				cond = BT_ANY;  
			        				
									FillGWDData (lpGWDHeadNameIndex,Offset); 
			        				if (strnicmp (pRec->Word,PartLoc[ipart],min(8,PartLen[ipart])))
										break;
									nSeqRec = -(pRec->Seq + 1);
			        				nOffsets = nSeqRec * 100 + pRec->nOffsets;
									if (!nPartOffsets[ipart])
										hPartOffsets[ipart] = GSSiGlobAlloc (0,GMEM_MOVEABLE,MAXOFFSETS * sizeof(int));
									pOffsets = (LPLONG)GlobalLock (hPartOffsets[ipart]);
									if (nPartOffsets[ipart] + pRec->nOffsets >= MAXOFFSETS)
										GetNext = FALSE;
									else
									{
										pOffsets += nPartOffsets[ipart];
										memmove (pOffsets,pRec->Offsets,pRec->nOffsets*sizeof(long));
										pOffsets += pRec->nOffsets;
										nPartOffsets[ipart] += pRec->nOffsets;
										while (GetNext && nSeqRec--)
										{
											BT_FIND (lpGWDHeadNameIndex->BTHandle[0],(LPSTR)pRec,pos,cond,(LPSTR)&Offset);
											FillGWDData (lpGWDHeadNameIndex,Offset); 
											if (nPartOffsets[ipart] + pRec->nOffsets >= MAXOFFSETS)
												GetNext = FALSE;
											else if (GSSiPeekMessage(&msg,hwndCtl,WM_KEYDOWN,WM_KEYDOWN,PM_NOREMOVE))
				 								GetNext = FALSE;	                                                             
											else
											{
												memmove (pOffsets,pRec->Offsets,pRec->nOffsets*sizeof(long));
												pOffsets += pRec->nOffsets;
												nPartOffsets[ipart] += pRec->nOffsets;
											}
										}
									}
									GlobalUnlock (hPartOffsets[ipart]);
				 					if (GSSiPeekMessage(&msg,hwndCtl,WM_KEYDOWN,WM_KEYDOWN,PM_NOREMOVE))
				 						GetNext = FALSE;	                                                             
								}
								if (!nPartOffsets[ipart])
									GetNext = FALSE;
							}
							else
								nPartOffsets[ipart] = INT_MAX;
	                 	}

						if (GetNext)
							nHits = GetWIHits (Hits,nParts,nPartOffsets,hPartOffsets,MAXHITS,hwndCtl);
	           			for (ipart=0;ipart<nParts;ipart++)
							GSSiGlobFree (&hPartOffsets[ipart]);
 						GlobalUnlock (FilePtrNameIndex->FileHandle); 
						GlobalUnlock (SQLPtrNameIndex->OFHandle);
						GlobalUnlock (hDBNameIndex);
						CloseDataFile (TRUE,&hDBNameIndex);
						GetGlobalCVal ("[%NAMEDB]",str,"[%DL]attribut\\name.gmd");
						OpenDataFile (str, "",BT_READ, &hDBName);
						SQLPtrName = (LPOPENSQLDATA) GlobalLock (hDBName);
						FilePtrName = (LPOPENFILEDATA) GlobalLock (SQLPtrName->OFHandle);
						lpGWDHeadName = (LPGWDHEADER)GlobalLock (FilePtrName->FileHandle); 
						nowner = 0;
						while (GetNext && nHits--)
						{
							BOOL	ShowThisOwner = TRUE;

							if (Hits[nHits] <= sizeof (GWDHEADER16))
								continue;
							FillGWDData (lpGWDHeadName,Hits[nHits]); 
							SQLPtrName->st = 0;  
							SQLPtrName->Offset = Hits[nHits]; 
							SQLPtrName->lastreadtime = ULONG_MAX;
			        		if (ShowThisOwner)
			        		{
								if (nowner == 81)
									idx = 0;
								GetGlobalCVal ("[%NAMEDISPLAY]",str,"[NAMETAXPAYER]$CHR(1)[NAMEADDR1] [NAMEADDR2] [NAMECITY],[NAMESTATE]$CHR(1)[NAMEACCTOWNER]");
								ExpandText (str); 
								OneSpace (str); 
								ReplaceChar (str,0x01,0x09);
 								strcpy (teststr,str);
								if ((pTab = strchr (teststr,'\t')))
 									*pTab = 0;
								nPartsTest = GetNameParts (teststr,PartLocTest,PartLenTest,MAXPARTS);
								for (ipart = 0;ipart < nParts;ipart++)
								{
									for (iparttest=0;iparttest<nPartsTest;iparttest++)
									{
										if (!strnicmp (PartLoc[ipart],PartLocTest[iparttest],PartLen[ipart]))
										{
											*PartLocTest[iparttest] = 0;
											goto FoundPart;
										}
									}
									ShowThisOwner = FALSE;
									break;
FoundPart:							;
								}
								if (ShowThisOwner)
								{
									nowner++;
									strcpy (lastowner,str);
						 			if ((idx=SendDlgItemMessage (hWndDlg,IDC_OWNERLIST,LB_ADDSTRING,0,(LPARAM)str)) ==
						 				LB_ERRSPACE) 
						 				GetNext = FALSE;
								}
							} 
				 			if (GSSiPeekMessage(&msg,hwndCtl,WM_KEYDOWN,WM_KEYDOWN,PM_NOREMOVE))
				 				GetNext = FALSE;	                                                             
						}
						GlobalUnlock (FilePtrName->FileHandle); 
						GlobalUnlock (SQLPtrName->OFHandle);
						GlobalUnlock (hDBName);
						CloseDataFile (TRUE,&hDBName);
						if (GetNext)
						{
							nOwners=SendDlgItemMessage(hWndDlg,IDC_OWNERLIST,LB_GETCOUNT,0,0);
							sprintf (str,"%ld selected",nOwners);
						}
						else
							strcpy (str,"Search cancelled");
		    			SetDlgItemText (hWndDlg,IDC_NUMOWNERS,str); 
					 }
					 if (First && SaveProperty >= 0) 
						PostMessage(GetDlgItem(hWndDlg,IDC_PROPERTYLIST), LB_SETSEL,TRUE,SaveProperty);  
					 First = FALSE;
                 }
                 break;
			     
            case IDC_PROPERTYLIST: 
                 switch(HIWORD(wParam))
                 {   
					case LBN_SELCHANGE:
					{   
						HANDLE	hItems;
						int n=GetLBSelectedItems (hWndDlg,IDC_PROPERTYLIST,&hItems);   
						LPINT	pItem;
						
				    	SetDlgItemText (hWndDlg,IDC_PROPMESSAGE,""); 
			    		EnableWindow (GetDlgItem(hWndDlg,IDOK),FALSE);
						if (n>0)  
						{
							pItem=(LPINT)GlobalLock (hItems);   
							//if (n==1)
							{   
								short	DoZoom=0;
								char	propmacro[256];

								GetGlobalCVal ("[%PROPERTYMACRO]",propmacro,0);
								
						 		while (n--)
						 		{   
						 			SendDlgItemMessage(hWndDlg,IDC_PROPERTYLIST,LB_GETTEXT,*pItem++,(DWORD)str);
						 			pTAB = _fstrrchr (str,'\t');
						 			pTAB++; 
						 		    SetGlobalValue ("%PROPERTYKEYVALUE",pTAB);
									ExpandText (propmacro);
								} 
							}
							GSSiGlobUlFree (&hItems);
						}
					}
					break;
					                 	 
					case LBN_DBLCLK:  
					PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);
					IgnoreLbutton = TRUE;
					break; 
		 		 }
		 		 break;
		 		     
            case IDC_OWNERLIST: 
                 switch(HIWORD(wParam))
                 {   
                 	 case LBN_SELCHANGE: 
				    	SetDlgItemText (hWndDlg,IDC_PROPMESSAGE,""); 
						PostMessage(hWndDlg, WM_COMMAND, IDC_SETPROPLIST, 0L);
		 		 }
		 		 break;
		 	
		 	case IDC_SETPROPLIST:
		 	{
			    GetGlobalCVal ("[%PROPERTYDB]",str,"[%DL]attribut\\master.gmd");
				if (!OpenDataFile (str, "",BT_READ, &hDBProperty))
				{
				    MessageBox( GetFocus(), str,"Cannot open property database", MB_OK);
				}
				else
				{
		 			HANDLE	hItems=0; 
					int	 n=GetLBSelectedItems (hWndDlg,IDC_OWNERLIST,&hItems);
					LPINT	pItem;
					
					SendDlgItemMessage (hWndDlg,IDC_PROPERTYLIST,LB_RESETCONTENT,0,0);
					if (n)
					{
						pItem = (LPINT)GlobalLock (hItems);  
						while (n--)
						{    
							
							Item = *pItem++;
							SendDlgItemMessage(hWndDlg,IDC_OWNERLIST,LB_GETTEXT,Item,(DWORD)str);
							if ((pTAB = _fstrrchr (str,'\t')))
							{
								*pTAB++ = 0;   
								_fstrcpy (WantStr2,pTAB);
								ReplaceChar (str,'\t','-');
							   //SetDlgItemText (hWndDlg,IDC_OWNERNAME,str); 
								{               
	   		      					short	IDIndex=GetGlobalLVal2 ("[%PROPERTYINDEX]",0), IDLen=0; 
	   		      					short	pos=BT_FIRST, cond=BT_GE;
	   		      					BOOL	GetNext=TRUE;
									char	propindexfld[66];
			           		      					
									SQLPtrProperty = (LPOPENSQLDATA) GlobalLock (hDBProperty);
									FilePtrProperty = (LPOPENFILEDATA) GlobalLock (SQLPtrProperty->OFHandle);
									lpGWDHeadProperty = (LPGWDHEADER)GlobalLock (FilePtrProperty->FileHandle);                             
									//for (i=0;i<abs(lpGWDHeadProperty->NumIndexFields[IDIndex]);i++)
									//	IDLen += lpGWDHeadProperty->pFldInfo[lpGWDHeadProperty->IndexFields[IDIndex][i]].Len;
									//_fstrncpy (lpGWDHeadProperty->pKeys[IDIndex],WantStr2,IDLen);
					        		GetGlobalCVal ("[%PROPERTYINDEXFIELD]",propindexfld,"[PID]");
							 		SetFieldValFromCharAndName(lpGWDHeadProperty,propindexfld,(LPSTR)WantStr2,FALSE);
									GWDFormKey(lpGWDHeadProperty,IDIndex,TRUE,0,0);

		        					while (GetNext && !BT_FIND (lpGWDHeadProperty->BTHandle[IDIndex],lpGWDHeadProperty->pKeys[IDIndex],pos,cond,(LPSTR)&Offset))
		        					{
										char	CharKeyVal[128];

										pos = BT_NEXT;
		        						cond = BT_ANY;
						        						
		        						//lpGWDHeadProperty->pKeys[IDIndex][IDLen]=0;   
		        						//Truncate (lpGWDHeadProperty->pKeys[IDIndex]);
						                GMDGetCharKeyVal (lpGWDHeadProperty,IDIndex,0,CharKeyVal); 
		        						if (!_fstricmp (WantStr2,CharKeyVal))
		        						{
											FillGWDData (lpGWDHeadProperty,Offset); 
					        				SQLPtrProperty->st = 0;  
					        				SQLPtrProperty->Offset = Offset; 
					        				SQLPtrProperty->lastreadtime = ULONG_MAX;
					        				GetGlobalCVal ("[%PROPERTYDISPLAY]",str,"[GEOACCT]$CHR(1)[PROPADDR] [STREET] [DIRECTION] [SFX]$CHR(1)[DEEDVOLUME]/[DEEDPAGE]$CHR(1)[ACRES]");
			        						ExpandText (str); 
			        						OneSpace (str); 
					        				GetGlobalCVal ("[%PROPERTYKEYFIELD]",propkeyfld,"[GEOACCT]");
			        						ReplaceChar (str,0x01,0x09); 
			        						sprintf (_fstrchr(str,0),"\t%s",propkeyfld);
			        						ExpandText (str);
				 							if ((idx=SendDlgItemMessage (hWndDlg,IDC_PROPERTYLIST,LB_ADDSTRING,0,(LPARAM)str)) ==
				 								LB_ERRSPACE) 
				 								GetNext = FALSE; 
				 						} 
				 						else
				 							GetNext = FALSE;
		        					}
		        					GlobalUnlock (FilePtrProperty->FileHandle); 
		           					GlobalUnlock (SQLPtrProperty->OFHandle);
		        					GlobalUnlock (hDBProperty); 
								}
							}
						}
					}
					CloseDataFile (TRUE,&hDBProperty); 
					GSSiGlobUlFree (&hItems);
	     	 	}  
	     	}
     	 	break; 
                 	 	
		 	case IDOK:
		 	{   
		 		HANDLE	hItems; 
				int	 n=GetLBSelectedItems (hWndDlg,IDC_PROPERTYLIST,&hItems);
				LPINT	pItem=(LPINT)GlobalLock (hItems);  
				LPSTR	pTAB; 
				BOOL	Err=TRUE;
				
				if (n == 1)
					SaveProperty = *pItem;
				else
					SaveProperty = -1;
				iOwner = SendDlgItemMessage(hWndDlg,IDC_OWNERLIST,LB_GETCURSEL,0,0);
		 		SendDlgItemMessage(hWndDlg,IDC_OWNERLIST,LB_GETTEXT,iOwner,(DWORD)str); 
		 		if ((pTAB = _fstrchr (str,'\t')))
		 			*pTAB = 0;
				SetGlobalValue ("%SELECTEDOWNER",str);
		 		ClearHighlightList(FALSE); 
		    	AutoHighlight=SendDlgItemMessage(hWndDlg,IDC_AUTOHIGHLIGHT,BM_GETCHECK,0,0); 
		 		while (n--)
		 		{   
		 			SendDlgItemMessage(hWndDlg,IDC_PROPERTYLIST,LB_GETTEXT,*pItem++,(DWORD)str);
		 			pTAB = _fstrrchr (str,'\t');
		 			pTAB++; 
		 			Strip (pTAB,' ');
					GetGlobalCVal ("[%PROPERTYPREFIX]",propprefix,"PINA");
					if (PickByRefno (0,propprefix,pTAB,-1))
					{
						if (PickList[0].Type != 6 && AutoHighlight) 
							AddToHighlightList (PickList[0].Refno,&PickList[0],TRUE); 
						Err=FALSE;
					}
					else 
					{
				    	SetDlgItemText (hWndDlg,IDC_PROPMESSAGE,"Property not found"); 
				    }
				} 
				GSSiGlobUlFree (&hItems);
				if (Err)
					break; 
			    GetDlgItemText (hWndDlg,IDC_OWNERNAME,WantStr,sizeof(WantStr)-1); 
			    GetDlgItemText (hWndDlg,IDC_OWNERNAMEWC,Wildcard,sizeof(Wildcard)-1);            
	            GSSiEndDialog(hWndDlg, 2,hSaveBM); 
				ExpandBounds (&HLTBounds, LocationOffset);
				ZoomToRect(HLTBounds,FALSE);  
//			    PostMessage(hWndMain, WM_COMMAND, IDM_Z_HLTLIMITS, 0L);
	            
	        }
		 	     break;   
		 	     
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
				 CloseDataFile (TRUE,&hDBName); 
                 GSSiEndDialog(hWndDlg, FALSE,hSaveBM);
                 break;
           }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
}      

BOOL FAR PASCAL OWNERLOCMsgProc4(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{ 	int		st, choice, n,idx,ifile,i; 
	int		Trigger=3;
	BOOL	HaveWildcard;
	char	str[1024], teststr[1024],lastowner[4096];
	static	char Wildcard[128]="";
	static	char WantStr[128]=""; 
	char	WantStr2[128],propkeyfld[64];
	LPGWDHEADER lpGWDHeadNameIndex, lpGWDHeadName, lpGWDHeadProperty;
    LPOPENFILEDATA  FilePtrNameIndex, FilePtrName, FilePtrProperty;
    LPOPENSQLDATA   SQLPtrNameIndex, SQLPtrName, SQLPtrProperty;
	long	Offset;
	int		nParts, nPartsTest, PartLen[MAXPARTS], PartLenTest[MAXPARTS];
	LPSTR	PartLoc[MAXPARTS], PartLocTest[MAXPARTS], pTab;
	static	HANDLE	hDBNameIndex=0, hDBName=0, hDBProperty=0;
	static	HANDLE	hSaveBM=0;  
	static	BOOL	First;  
	static	short	iOwner=-1, SaveProperty=-1; 
	short	Item;
	char	propprefix[32];
	TAGKEY TAGKey;  
	LPSTR	pTAB; 
	HWND	hwndCtl;
	int		nowner;
	static	char	NameDelim;
	UINT	nPrompts=4;
	UINT	PrmtDat[4*3] = {
							  IDC_OWNERNAMEWC,PRMT_IDC_OWNERNAMEWC,0,
							  IDC_OWNERNAME, PRMT_IDC_OWNERNAME,0,  
							  IDC_OWNERLIST, PRMT_IDC_OWNERLIST,0,
							  IDC_PROPERTYLIST, PRMT_IDC_PROPERTYLIST,0
							};


 int	BRtn;
 if (Message==WM_INITDIALOG)
 {  
 	UINT	pw=0, pn=1, pm=2; 
 	HWND	hwnd;
 	short	ii;
 	
 	InitDlgPrompts (hWndDlg);
 	while (nPrompts--)
 	{   
 		hwnd = GetDlgItem(hWndDlg,PrmtDat[pw]);
 		if (!hwnd)
 			ii=1;
	 	SetDlgPrompt (hwnd,PrmtDat[pn],PrmtDat[pm]);
	 	pw+=3;
	 	pn+=3; 
	 	pm+=3;
	}
 }
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
 	return (BRtn);
 switch(Message)
   {
 	case WM_DESTROY: 
	 	InitDlgPrompts (0);    
	 	break;
    case WM_INITDIALOG:  
	{   
		int	nTabs;
		HANDLE	hList;
		LPINT	pTabs;

    	ClearDlgPrompts (); 
    	//hSaveBM = EnterBlockingWindow (hWndDlg);
		First = TRUE;
		SendDlgItemMessage (hWndDlg,IDC_AUTOHIGHLIGHT,BM_SETCHECK,AutoHighlight,0L);
		GetGlobalCVal ("[%OWNERTABS]",str,"250 2000");
		nTabs = GetIntsFromList (str,&hList); 
		if (nTabs)
		{
			pTabs = (LPINT)GlobalLock (hList);
			SendDlgItemMessage (hWndDlg,IDC_OWNERLIST,LB_SETTABSTOPS,nTabs,(LPARAM)pTabs);
			GSSiGlobUlFree (&hList);
		}
		GetGlobalCVal ("[%PROPERTYTABS]",str,"80 200 250 2000");
		nTabs = GetIntsFromList (str,&hList); 
		if (nTabs)
		{
			pTabs = (LPINT)GlobalLock (hList);
			SendDlgItemMessage (hWndDlg,IDC_PROPERTYLIST,LB_SETTABSTOPS,nTabs,(LPARAM)pTabs);
			GSSiGlobUlFree (&hList);
		}
        cwCenter(hWndDlg, 0); 
        GetGlobalCVal ("[%NAMEINDEXDB]",str,"[%DL]attribut\\nameindex.gmd");
	    if (!OpenDataFile (str, "",BT_READ, &hDBNameIndex))
	    {
	        MessageBox( GetFocus(), str,"Cannot open name index database", MB_OK);
	        PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
	        return FALSE;
	    }  
		CloseDataFile (TRUE,&hDBNameIndex);
        GetGlobalCVal ("[%NAMEDB]",str,"[%DL]attribut\\name.gmd");
	    if (!OpenDataFile (str, "",BT_READ, &hDBName))
	    {
	        MessageBox( GetFocus(), str,"Cannot open name database", MB_OK);
	        PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
	        return FALSE;
	    }  
		CloseDataFile (TRUE,&hDBName);
		GetGlobalCVal ("[%PROPERTYDB]",str,"[%DL]attribut\\master.gmd");
		if (!OpenDataFile (str, "",BT_READ, &hDBProperty))
		{
			MessageBox( GetFocus(), str,"Cannot open property database", MB_OK);
	        PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
	        return FALSE;
		}
		CloseDataFile (TRUE,&hDBProperty);
	    SetDlgItemText (hWndDlg,IDC_OWNERNAMEWC,Wildcard);            
		PostMessage(GetDlgItem(hWndDlg,IDC_OWNERLIST), LB_SETCURSEL,iOwner,0); 
		PostMessage(hWndDlg, WM_COMMAND, IDC_SETPROPLIST, 0L);
	 	PostMessage(GetDlgItem(hWndDlg,IDC_PROPERTYLIST), LB_SETSEL,TRUE,SaveProperty);  
	} 

        break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(LOWORD(wParam))
           {
			case IDC_OWNERNAMEWC: 
		    	 SetDlgItemText (hWndDlg,IDC_PROPMESSAGE,""); 
				 hwndCtl = (HWND) LOWORD(lParam);
                 switch (HIWORD(wParam))
                 {	case EN_CHANGE: 
                 	{   
						int	nParts, ipart,iparttest, nOwners;
						LPWORDINDEX	pRec;
						WORDINDEX	Rec;
						HANDLE	hPartOffsets[MAXPARTS];
						int		nPartOffsets[MAXPARTS];
						int		nHits;
       		      		BOOL	GetNext = TRUE;   
   		      			MSG     msg; 
						char	Name[66];

						SetDlgItemText (hWndDlg,IDC_NUMOWNERS,"Searching..."); 
				        GetGlobalCVal ("[%NAMEINDEXDB]",str,"[%DL]attribut\\nameindex.gmd");
						OpenDataFile (str, "",BT_READ, &hDBNameIndex);
 						SQLPtrNameIndex = (LPOPENSQLDATA) GlobalLock (hDBNameIndex);
						FilePtrNameIndex = (LPOPENFILEDATA) GlobalLock (SQLPtrNameIndex->OFHandle);
						lpGWDHeadNameIndex = (LPGWDHEADER)GlobalLock (FilePtrNameIndex->FileHandle); 
                        pRec = (LPWORDINDEX)&lpGWDHeadNameIndex->GWDData;
						SendDlgItemMessage (hWndDlg,IDC_OWNERLIST,LB_RESETCONTENT,0,0);
						SendDlgItemMessage (hWndDlg,IDC_PROPERTYLIST,LB_RESETCONTENT,0,0);   
           		      	GetDlgItemText (hWndDlg,IDC_OWNERNAMEWC,Name,64);
						nParts = GetNameParts (Name,PartLoc,PartLen,MAXPARTS);
           		      	
 	           		    for (ipart=0;ipart<nParts;ipart++)
						{
                            nPartOffsets[ipart] = 0;
							hPartOffsets[ipart] = 0;
						}
          		      	for (ipart=0;ipart<nParts;ipart++)
           		      	{   
           		      		short	pos=BT_FIRST, cond=BT_GE;
							int		nOffsets, nSeqRec;
                            
							if (PartLen[ipart] >= Trigger)
							{
								strncpy (Rec.Word,PartLoc[ipart],8);
								Rec.Seq = SHRT_MIN;
			        			while (GetNext && !BT_FIND (lpGWDHeadNameIndex->BTHandle[0],(LPSTR)&Rec,pos,cond,(LPSTR)&Offset))
			        			{
									LPLONG	pOffsets;

									pos = BT_NEXT;
			        				cond = BT_ANY;  
			        				
									FillGWDData (lpGWDHeadNameIndex,Offset); 
			        				if (strnicmp (pRec->Word,PartLoc[ipart],min(8,PartLen[ipart])))
										break;
									nSeqRec = -(pRec->Seq + 1);
			        				nOffsets = nSeqRec * 100 + pRec->nOffsets;
									if (!nPartOffsets[ipart])
										hPartOffsets[ipart] = GSSiGlobAlloc (0,GMEM_MOVEABLE,MAXOFFSETS * sizeof(int));
									pOffsets = (LPLONG)GlobalLock (hPartOffsets[ipart]);
									if (nPartOffsets[ipart] + pRec->nOffsets >= MAXOFFSETS)
										GetNext = FALSE;
									else
									{
										pOffsets += nPartOffsets[ipart];
										memmove (pOffsets,pRec->Offsets,pRec->nOffsets*sizeof(long));
										pOffsets += pRec->nOffsets;
										nPartOffsets[ipart] += pRec->nOffsets;
										while (GetNext && nSeqRec--)
										{
											BT_FIND (lpGWDHeadNameIndex->BTHandle[0],(LPSTR)pRec,pos,cond,(LPSTR)&Offset);
											FillGWDData (lpGWDHeadNameIndex,Offset); 
											if (nPartOffsets[ipart] + pRec->nOffsets >= MAXOFFSETS)
												GetNext = FALSE;
											else if (GSSiPeekMessage(&msg,hwndCtl,WM_KEYDOWN,WM_KEYDOWN,PM_NOREMOVE))
				 								GetNext = FALSE;	                                                             
											else
											{
												memmove (pOffsets,pRec->Offsets,pRec->nOffsets*sizeof(long));
												pOffsets += pRec->nOffsets;
												nPartOffsets[ipart] += pRec->nOffsets;
											}
										}
									}
									GlobalUnlock (hPartOffsets[ipart]);
				 					if (GSSiPeekMessage(&msg,hwndCtl,WM_KEYDOWN,WM_KEYDOWN,PM_NOREMOVE))
				 						GetNext = FALSE;	                                                             
								}
								if (!nPartOffsets[ipart])
									GetNext = FALSE;
							}
							else
								nPartOffsets[ipart] = INT_MAX;
	                 	}

						if (GetNext)
							nHits = GetWIHits (Hits,nParts,nPartOffsets,hPartOffsets,MAXHITS,hwndCtl);
	           			for (ipart=0;ipart<nParts;ipart++)
							GSSiGlobFree (&hPartOffsets[ipart]);
 						GlobalUnlock (FilePtrNameIndex->FileHandle); 
						GlobalUnlock (SQLPtrNameIndex->OFHandle);
						GlobalUnlock (hDBNameIndex);
						CloseDataFile (TRUE,&hDBNameIndex);
						GetGlobalCVal ("[%NAMEDB]",str,"[%DL]attribut\\name.gmd");
						OpenDataFile (str, "",BT_READ, &hDBName);
						SQLPtrName = (LPOPENSQLDATA) GlobalLock (hDBName);
						FilePtrName = (LPOPENFILEDATA) GlobalLock (SQLPtrName->OFHandle);
						lpGWDHeadName = (LPGWDHEADER)GlobalLock (FilePtrName->FileHandle); 
						nowner = 0;
						while (GetNext && nHits--)
						{
							BOOL	ShowThisOwner = TRUE;

							if (Hits[nHits] <= sizeof (GWDHEADER16))
								continue;
							FillGWDData (lpGWDHeadName,Hits[nHits]); 
							SQLPtrName->st = 0;  
							SQLPtrName->Offset = Hits[nHits]; 
							SQLPtrName->lastreadtime = ULONG_MAX;
			        		if (ShowThisOwner)
			        		{
								if (nowner == 81)
									idx = 0;
								GetGlobalCVal ("[%NAMEDISPLAY]",str,"[NAMETAXPAYER]$CHR(1)[NAMEADDR1] [NAMEADDR2] [NAMECITY],[NAMESTATE]$CHR(1)[NAMEACCTOWNER]");
								ExpandText (str); 
								OneSpace (str); 
								ReplaceChar (str,0x01,0x09);
 								strcpy (teststr,str);
								if ((pTab = strchr (teststr,'\t')))
 									*pTab = 0;
								nPartsTest = GetNameParts (teststr,PartLocTest,PartLenTest,MAXPARTS);
								for (ipart = 0;ipart < nParts;ipart++)
								{
									for (iparttest=0;iparttest<nPartsTest;iparttest++)
									{
										if (!strnicmp (PartLoc[ipart],PartLocTest[iparttest],PartLen[ipart]))
										{
											*PartLocTest[iparttest] = 0;
											goto FoundPart;
										}
									}
									ShowThisOwner = FALSE;
									break;
FoundPart:							;
								}
								if (ShowThisOwner)
								{
									nowner++;
									strcpy (lastowner,str);
						 			if ((idx=SendDlgItemMessage (hWndDlg,IDC_OWNERLIST,LB_ADDSTRING,0,(LPARAM)str)) ==
						 				LB_ERRSPACE) 
						 				GetNext = FALSE;
								}
							} 
				 			if (GSSiPeekMessage(&msg,hwndCtl,WM_KEYDOWN,WM_KEYDOWN,PM_NOREMOVE))
				 				GetNext = FALSE;	                                                             
						}
						GlobalUnlock (FilePtrName->FileHandle); 
						GlobalUnlock (SQLPtrName->OFHandle);
						GlobalUnlock (hDBName);
						CloseDataFile (TRUE,&hDBName);
						if (GetNext)
						{
							nOwners=SendDlgItemMessage(hWndDlg,IDC_OWNERLIST,LB_GETCOUNT,0,0);
							sprintf (str,"%ld selected",nOwners);
						}
						else
							strcpy (str,"Search cancelled");
		    			SetDlgItemText (hWndDlg,IDC_NUMOWNERS,str); 
					 }
					 if (First && SaveProperty >= 0) 
						PostMessage(GetDlgItem(hWndDlg,IDC_PROPERTYLIST), LB_SETSEL,TRUE,SaveProperty);  
					 First = FALSE;
                 }
                 break;
			     
            case IDC_OWNERLIST: 
                 switch(HIWORD(wParam))
                 {   
                 	 case LBN_SELCHANGE: 
					 {

				    	int nSel = SendDlgItemMessage(hWndDlg,IDC_OWNERLIST,LB_GETSELCOUNT,0,0);

						switch (nSel)
						{
						case 0:
							EnableWindow (GetDlgItem (hWndDlg,IDC_DETAILEDREPORT),FALSE);
							EnableWindow (GetDlgItem (hWndDlg,IDC_SUMMARYREPORT),FALSE);
							break;
						case 1:
							EnableWindow (GetDlgItem (hWndDlg,IDC_DETAILEDREPORT),TRUE);
							EnableWindow (GetDlgItem (hWndDlg,IDC_SUMMARYREPORT),FALSE);
							break;
						default:
							EnableWindow (GetDlgItem (hWndDlg,IDC_DETAILEDREPORT),FALSE);
							EnableWindow (GetDlgItem (hWndDlg,IDC_SUMMARYREPORT),TRUE);
							break;

						}
					 }

		 		 }
		 		 break;
		 	
				 case IDC_DETAILEDREPORT:
				 {
			 		HANDLE	hItems; 
					int	 n=GetLBSelectedItems (hWndDlg,IDC_OWNERLIST,&hItems);

					if (n)
					{
						LPINT	pItems=GlobalLock (hItems);
						char	cmd[128];
						
			 			SendDlgItemMessage(hWndDlg,IDC_OWNERLIST,LB_GETTEXT,*pItems,(DWORD)str);
						pTAB = strrchr (str,'\t');
						pTAB++; 
						sprintf (cmd,"[CNUMBER]=%s;$REPORT([%DL]police\\reports\\genrep_new.txt,SCROLL);",pTAB);
						ExpandText (cmd);
						GSSiGlobUlFree (&hItems);
					}

				 }
					 break;

				 case IDC_SUMMARYREPORT:
				 {
			 		HANDLE	hItems; 
					int	 n=GetLBSelectedItems (hWndDlg,IDC_OWNERLIST,&hItems);

					if (n)
					{
						LPINT	pItems=GlobalLock (hItems);
						char	cmd[128];
						
						strcpy (cmd,"[%REFLIST]=%PICKED_UDI;");
						ProcessText (cmd);
			 			while (n--)
						{
							SendDlgItemMessage(hWndDlg,IDC_OWNERLIST,LB_GETTEXT,*pItems++,(DWORD)str);
							pTAB = strrchr (str,'\t');
							pTAB++; 
							sprintf (cmd,"[%REFLIST]=[%REFLIST]|%s|",pTAB);
							ProcessText (cmd);
						}
						sprintf (cmd,"$REPORT([%DL]police\\reports\\summary.txt,SCROLL);");
						ExpandText (cmd);
						GSSiGlobUlFree (&hItems);
					}

				 }
					 break;

            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
				 CloseDataFile (TRUE,&hDBName); 
                 GSSiEndDialog(hWndDlg, FALSE,hSaveBM);
                 break;
           }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
}      

