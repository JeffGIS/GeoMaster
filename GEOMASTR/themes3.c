#include "graphint.h"

static struct key	{
			double Y;
			double X;
			long Refno;
			} SVKey;
static DPOINT	SVData;

static struct {
		POINT	Point;
		short	Sequence;
		} DisperseKey;  
static struct {
		short 	FileNum,
				SubFile;
		USHORT	FileInIndex;
		long	Segment;
		WORD	Offset, Element;  
		long	Refno;   
		short	Desc;
		float	Size;
		POINT	DisplayPoint;
		DPOINT	PointLocWorld; 
		long	Num;
		ULONG	MSLink;
		char	Prefix[33], UDI[65];
		} PointDispersionData;    



static	HWND	hWndFieldList=0;
static	LPRECT	pFLRect; 
static	HANDLE	hFLDB=0,hFieldsDLT;
static	short	FieldsDisplayOpt=0;  
static	BOOL	AutoCloseFieldDB=FALSE;
static FARPROC	lpfnFIELDSMsgProc;
static	char	GetFieldOutput[1024];
static	LPSTR	pFieldsTitle=0;

#include "gmextern.h"

void ThemeDisplayScatterDiagram ()
#if ENABLETRACE
{GSSiEnterProg (1266);
#endif
{   time_t	ltime;
	HBRUSH	BkBrush, ScatterBrush;
	HPEN	DotPen, OldPen;
	double	Xrange, Yrange, Ymin, Ymax;
	int		BoxWidth, BoxHeight;
	int		st;
	int		X, Y, w;
	RECT	rect;


	if (!CurTheme->DisplayScatterDiagram)
{
#if ENABLETRACE
GSSiExitProg (1266);
#endif
		return;
}
	FillRectPoly (CurView->hDC,&CurTheme->ScatterBox,CurTheme->IBBGColor);
	ltime = 0; 
     
	Ymax = CurTheme->Ymax; 
	Ymin = CurTheme->Ymin;
	if (!CurTheme->NumVals)
{
#if ENABLETRACE
GSSiExitProg (1266);
#endif
		return;			
}
	Xrange = CurTheme->Xmax - CurTheme->Xmin;
	if (Xrange == 0)
{
#if ENABLETRACE
GSSiExitProg (1266);
#endif
		return;
}
	Yrange = Ymax - Ymin;  
	if (Yrange == 0)
{
#if ENABLETRACE
GSSiExitProg (1266);
#endif
		return;
}
	w=2;
	if (Printing) w=6;
	DotPen = CreatePen (PS_SOLID,w,CurTheme->ScatterColor);
	OldPen = SelectObject (CurView->hDC,DotPen);
	BoxWidth = CurTheme->ScatterBox.right - CurTheme->ScatterBox.left;
	BoxHeight = CurTheme->ScatterBox.bottom - CurTheme->ScatterBox.top;
	ScatterBrush = CreateSolidBrush (CurTheme->ScatterColor);
	if (!CurTheme->hScatterFile && *CurTheme->ScatterFile)
		CurTheme->hScatterFile = BT_OPEN (CurTheme->ScatterFile, ltime, BT_READ, 0);

	if (CurTheme->hScatterFile)
	{
		st = BT_FIND (CurTheme->hScatterFile,(LPSTR)&SVKey,BT_FIRST,BT_ANY,(LPSTR)&SVData);
		while (!st)
		{
    		X = CurTheme->ScatterBox.left + IDNINT (((SVKey.X - CurTheme->Xmin)*BoxWidth)/Xrange);
    		SVKey.Y = min (SVKey.Y,CurTheme->YLimit);
    		Y = CurTheme->ScatterBox.bottom - IDNINT (((SVKey.Y - Ymin)/Yrange)*BoxHeight);
	/*		if (Printing)
			{
				w = 2;
				rect.left = X - w;
				rect.right = X + w;
				rect.top = X - w;
				rect.bottom = X + w;
				FillRect (CurView->hDC,&rect,ScatterBrush);
			}
			else
			{*/
				MoveToEx (CurView->hDC,X,Y,0);
				LineTo (CurView->hDC,X,Y);
			/*} */

			st = BT_FIND (CurTheme->hScatterFile,(LPSTR)&SVKey,BT_NEXT,BT_ANY,(LPSTR)&SVData);
		}
		BT_CLOSE (CurTheme->hScatterFile);
	}
	CurTheme->hScatterFile=0;

	DeleteObject (ScatterBrush);
	SelectObject (CurView->hDC,OldPen);
	DeleteObject (DotPen);
{
#if ENABLETRACE
GSSiExitProg (1266);
#endif
	return;
}

#if ENABLETRACE
}
#endif
}

void ThemeEndDataPass(BOOL PixelThemesOnly)
#if ENABLETRACE
{GSSiEnterProg (1273);
#endif
{   int itheme, iclass, inc, st;
	double Ymin, Ymax, Rem, ClassRange;
	time_t ltime;
	long	num, NumInClass, DesiredNumInClass;
    
    RemoveDataPassMessage ();
	for (itheme=0;itheme<CurView->NumThemes;itheme++)
	{
		CurTheme=CurView->pThemes[itheme];
		if (!CurTheme->IsActive|| !CurTheme->VPDisplayed || CurTheme->ID == PF_COORD_DISPLAY || CurTheme->ID == PF_BOUNDS_DISPLAY)
			goto NextTheme;  
		if (PixelThemesOnly && CurTheme->DataType != THEMEDATATYPE_PIXEL)
			goto NextTheme;
		if (!PixelThemesOnly && CurTheme->DataType == THEMEDATATYPE_PIXEL)
			goto NextTheme;
        if (CurTheme->hScatterFile) 
        {
        	if (CurTheme->ID == GF_STREET_TEXT_THEME || CurTheme->ID == GF_STREET_ADDRESS_THEME)
		    	GSSiGlobFree (&CurTheme->hScatterFile);
		    else
	        	BT_CLOSE (CurTheme->hScatterFile);
	    }
		CurTheme->hScatterFile=0;
		CloseThemeDataFile(FALSE);
		GSSiClose (CurTheme->FidAreas);
		CurTheme->FidAreas = HFILE_ERROR;
		SetShowValDB (0);
		GSSiGlobFree (&CurTheme->hVisList);
		if (!CurTheme->NumVals) goto NextTheme;
		CurTheme->Ymax = min (CurTheme->Ymax,CurTheme->YLimit);
		if (CurTheme->WantDataPass)
		{
			if (!CurTheme->Recompute)
				CurTheme->WantDataPass=FALSE;
			if (CurTheme->ClassType != 3 && CurTheme->Ymin == CurTheme->Ymax)
				CurTheme->NumClass = 1;
			else
				CurTheme->NumClass = CurTheme->NumDesiredClass;
			for (iclass=0;iclass<CurTheme->NumClass;iclass++)
			if (CurTheme->ComputeClassBoundaries)
			{   
				switch (CurTheme->ClassType)
				{
					case 1:
						if (!CurTheme->RoundTo)
							CurTheme->RoundTo = 1;
						Ymin = CurTheme->Ymin-CurTheme->RoundTo/2;
						Ymax = CurTheme->Ymax; 
						CurTheme->NumClass = min (CurTheme->NumClass,1+IDNINT((min(1000,(Ymax-CurTheme->Ymin)/CurTheme->RoundTo))));
						ClassRange = (Ymax-CurTheme->Ymin+CurTheme->RoundTo)/CurTheme->NumClass;
						for (iclass=0;iclass<CurTheme->NumClass;iclass++)
						{
							
							CurTheme->ClassMin[iclass] = 
								Round (Ymin + iclass * ClassRange+CurTheme->RoundTo*0.1,CurTheme->RoundTo);
							CurTheme->ClassMax[iclass] =  
								Round (Ymin + (iclass+1) * ClassRange-CurTheme->RoundTo*0.1,CurTheme->RoundTo);
							if (iclass)
							{
								if (CurTheme->ClassMin[iclass] - CurTheme->ClassMax[iclass-1] <
									CurTheme->RoundTo/10)
									CurTheme->ClassMin[iclass] += CurTheme->RoundTo;
							}
		                 }
                    break;
	                    
                    case 2:
                        
                        if (!*CurTheme->ScatterFile)
                        	break;
						CurTheme->NumClass = min (CurTheme->NumClass,1+IDNINT((min(1000,(CurTheme->Ymax-CurTheme->Ymin)/CurTheme->RoundTo))));
						if (!CurTheme->NumClass)
							break;
                        DesiredNumInClass = CurTheme->NumVals/CurTheme->NumClass;
						ltime = 0; 
						if (!CurTheme->hScatterFile)
							CurTheme->hScatterFile = BT_OPEN (CurTheme->ScatterFile, ltime, BT_READ, 0);
                        num = 0;
                        iclass = 0; 
                        NumInClass = 0;
			    		CurTheme->ClassMin[0]=CurTheme->Ymin; 
					    st = BT_FIND (CurTheme->hScatterFile,(LPSTR)&SVKey,BT_FIRST,BT_ANY,(LPSTR)&SVData);
					    while (!st)
					    {   
					    	num++;
					    	if (NumInClass > DesiredNumInClass && SVKey.Y > CurTheme->ClassMin[iclass])
					    	{
					    		CurTheme->ClassMax[iclass]=SVKey.Y;
					    		CurTheme->ClassMin[iclass+1]=SVKey.Y + CurTheme->RoundTo;  
					    		NumInClass = 0;
					    		if (num <CurTheme->NumVals) 
					    			iclass++;
					    	}  
					    	else
					    		NumInClass++;
					    		
						    st = BT_FIND (CurTheme->hScatterFile,(LPSTR)&SVKey,BT_NEXT,BT_ANY,(LPSTR)&SVData);
						}
						CurTheme->NumClass = min (CurTheme->NumClass,iclass+1);
					    BT_CLOSE (CurTheme->hScatterFile);
						CurTheme->hScatterFile=0;
			    		CurTheme->ClassMax[CurTheme->NumClass-1]=CurTheme->Ymax;

                    break;
	                    
                    case 3:
                    break;
				}
				for (iclass=0;iclass<CurTheme->NumClass;iclass++)
				{
					CurTheme->ClassMin[iclass] = Round(CurTheme->ClassMin[iclass],CurTheme->RoundTo);
					CurTheme->ClassMax[iclass] = Round(CurTheme->ClassMax[iclass],CurTheme->RoundTo);
					CurTheme->ClassMin[iclass] -= CurTheme->RoundTo * 0.1;
					CurTheme->ClassMax[iclass] += CurTheme->RoundTo * 0.1;
				}
			}
		} 
		
NextTheme:
	;
	}
{
#if ENABLETRACE
GSSiExitProg (1273);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}
void ThemePickScatterDiagram (POINT PickPoint)
#if ENABLETRACE
{GSSiEnterProg (1267);
#endif
{    
	HBRUSH	BkBrush;
	HPEN	DotPen, OldPen;
	double	Xrange, Yrange;
	int		BoxWidth, BoxHeight;
	int		st;
	int		X, Y;
	int		PickAp = 3;

	if (!CurView->pTheme)
{
#if ENABLETRACE
GSSiExitProg (1267);
#endif
		return;
}
	CurTheme = CurView->pTheme;
	if (!CurTheme->DisplayScatterDiagram)
{
#if ENABLETRACE
GSSiExitProg (1267);
#endif
		return;
}

	if (!CurTheme->hScatterFile)
		CurTheme->hScatterFile= BT_OPEN (CurTheme->ScatterFile, 0, BT_READ, 0);

	Xrange = CurTheme->Xmax - CurTheme->Xmin;
	Yrange = CurTheme->Ymax - CurTheme->Ymin; 
	if (Xrange == 0 || Yrange == 0)
{
#if ENABLETRACE
GSSiExitProg (1267);
#endif
		return;
}
	BoxWidth = CurTheme->ScatterBox.right - CurTheme->ScatterBox.left;
	BoxHeight = CurTheme->ScatterBox.bottom - CurTheme->ScatterBox.top;

    st = BT_FIND (CurTheme->hScatterFile,(LPSTR)&SVKey,BT_FIRST,BT_ANY,(LPSTR)&SVData);
    while (!st)
    {
    	X = CurTheme->ScatterBox.left + IDNINT (((SVKey.X - CurTheme->Xmin)*BoxWidth)/Xrange);
    	SVKey.Y = min (SVKey.Y,CurTheme->YLimit);
    	Y = CurTheme->ScatterBox.bottom - IDNINT (((SVKey.Y - CurTheme->Ymin)/Yrange)*BoxHeight);
		if (abs (X - PickPoint.x) <= PickAp && abs (Y - PickPoint.y) <= PickAp)
			LinkScatterPointToMap (X,Y,SVData,FALSE);
	    st = BT_FIND (CurTheme->hScatterFile,(LPSTR)&SVKey,BT_NEXT,BT_ANY,(LPSTR)&SVData);
	}
    BT_CLOSE (CurTheme->hScatterFile);
	CurTheme->hScatterFile=0;

{
#if ENABLETRACE
GSSiExitProg (1267);
#endif
	return;
}

#if ENABLETRACE
}
#endif
}

void LinkScatterPointToMap (int X,int Y,DPOINT MapPointD,BOOL clip)
#if ENABLETRACE
{GSSiEnterProg (1268);
#endif
{   POINT ScatterPoint;
	HPEN  ArrowPen;
	LPVIEWPORT SaveView;
	POINT MapPoint;

	ScatterPoint.x = X;
	ScatterPoint.y = Y;
	SaveView = CurView;
	SetCurView (pViewports[CurTheme->TargetViewport-1]);
	SaveDC (CurView->hDC);
    SetDisplayMode (CurView->hDC, GF_MAPMODE);
  	if (!clip)
		SelectClipRgn (CurView->hDC,0);
    /*if (!PtInBounds(MapPoint)) CenterWindow (CurView->hWnd,WinPtToBasePt(MapPoint),FALSE);*/

    MapPoint = BasePtToWinPt (&MapPointD);
/*    LPtoDP (CurView->hDC,&MapPoint,1);*/
    SetDisplayMode (CurView->hDC, GF_TEXTMODE);
    SetCurView (SaveView);

	ArrowPen = CreatePen (PS_SOLID,0,RGB(0,0,0));
	DrawPointerLine (CurView->hDC, ScatterPoint, MapPoint,ArrowPen,ArrowPen,10,0);
	DeleteObject (ArrowPen);
	RestoreDC (CurView->hDC,-1);

{
#if ENABLETRACE
GSSiExitProg (1268);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

BOOL ThemeSetData (int Type, long iref, int desc, LPSTR TAG, LPSTR UDI)
#if ENABLETRACE
{GSSiEnterProg (1265);
#endif
{	long MktVal;
	short	status;
	double ValD, Rem;
	float  Length;  
	float	MidPointAZ;
	char	Value[256]; 
	BOOL	rtn=FALSE;

	if (!CurTheme->IsActive|| !CurTheme->VPDisplayed)
{
#if ENABLETRACE
GSSiExitProg (1265);
#endif
		return (FALSE);  
}
	HaltReport = FALSE;  
	ThemeDisplayPass = 0;
	switch (CurTheme->ID)
	{   
		case GF_STREET_ADDRESS_THEME:
		case GF_STREET_TEXT_THEME:   
		case GF_TRANSFORM_THEME:
		case GF_BOUNDS_DISPLAY_THEME:     
		case GF_DISTANCE_THEME: 
		case GF_COORDGRID_THEME:    
		case GF_DYNAMIC_SEG_THEME:
		case GF_PROFILE_THEME:
		case GF_PROFILE_LINK_THEME:
		case GF_2D_THEME:         
		case GF_COMPARE_VIEWPORTS_THEME:
		case GF_GRAPHICS_FUNCTION_THEME:     
		case GF_POLYINFO_THEME: 
		case GF_TIME_DISPLAY_THEME:
		case GF_NORTH_ARROW_THEME:
		case GF_CITY_THEME:
			rtn=TRUE;
			break;
		
				
		case GF_POINT_IN_AREA_THEME:
			switch (Type)
			{
				case GF_AREA:
				
				if (CurTheme->SymNum2 == -1)
				{
					if (_fstricmp (TAG,&CurTheme->Contents2[1]))
						break;
				}
				else if (CurTheme->hVisList2)
				{   
					LPVISLIST	SaveVis=CurVis; 
					BOOL		WantDesc;
					
					CurVis = (LPVISLIST)GlobalLock (CurTheme->hVisList2);
					WantDesc = GetVisibility (desc);
					GlobalUnlock (CurTheme->hVisList2);
					CurVis = SaveVis;
					if (!WantDesc)
						break;
				}
				
				CurTheme->NumAreas++;
				goto ProcessPoint;
			}
			break; 
			
/*		case GF_OFFSETAREA_THEME:
			if (CurTheme->SymNum == -1)
			{
				if (_fstricmp (TAG,&CurTheme->Contents[1]))
					break;
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
					break;
			}
			SwitchThemeSHPFile ();		
			ResetFileChangeTime (CurTheme->hThemeDB);
			ValD = GetNumericFieldData (CurTheme->hThemeDB,
										&CurTheme->Field,CurTheme->FieldFun,CurTheme->MultiValOption,CurTheme->Value,iref,Type,&status,CurTheme->DataFileID);
			if (status == 1 && CurTheme->MissOpt == 4)
			{
				status = 0;
				ValD = 0;
			}
			if (status)
				break; 
			ThemeCreateOffsetAreas (ValD);
			break;
*/
		case GF_HOTSPOT_THEME: 
		case GF_OFFSETAREA_THEME:
		case GF_SINGLE_VALUE_THEME:  
ProcessPoint:
			if (CurTheme->SymNum == -1)
			{
				if (_fstricmp (TAG,&CurTheme->Contents[1]))
					break;
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
					break;
			}
			if (CurTheme->ID == GF_HOTSPOT_THEME)
			{
				AddItemToHotSpot (Type);
				break;
			}
			SwitchThemeSHPFile ();		
			ResetFileChangeTime (CurTheme->hThemeDB);
			ValD = GetNumericFieldData (CurTheme->hThemeDB,
										&CurTheme->Field,CurTheme->FieldFun,CurTheme->MultiValOption,CurTheme->Value,iref,Type,&status,CurTheme->DataFileID);
			if (status == 1 && CurTheme->MissOpt == 4)
			{
				status = 0;
				ValD = 0;
			}
			if (status)
				break; 
			if (CurTheme->ZeroIsMissing && ValD == 0)
				break;
			if (CurTheme->ID == GF_OFFSETAREA_THEME)
				ThemeCreateOffsetAreas (ValD);
			if (ValD && CurTheme->FieldCorrection)
			{   
				double	Area,Dist;
				mnmxCor MinMax;
				
				switch (CurTheme->DataType)
				{
					case 0:
						if (HiPrecis)
							Area = ComputeProjectedAreaAreaD (lpDCurPoints,nCurPoints,&Dist); 
						else 
							Area = ComputeProjectedAreaArea (lpCurPoints,nCurPoints,&Dist);  
						Area = ConvertArea (Area,CurTheme->FieldCorrection); 
						if (!Area)
							goto Exit;
						ValD /= Area;
					break; 
					default:
					case 1:
					break;
					case 2:
						if (HiPrecis)
							ComputeProjectedAreaAreaD (lpDCurPoints,-nCurPoints,&Dist); 
						else 
							ComputeProjectedAreaArea (lpCurPoints,-nCurPoints,&Dist);  
						Dist = ConvertDist (Dist,CurTheme->FieldCorrection); 
						if (!Dist)
							goto Exit;
						ValD /= Dist;
					break;
				}
			}
			ValD = Round(ValD,CurTheme->RoundTo);
			
			SVKey.Y = ValD;
			SVKey.X = rand();
			SVKey.Refno = iref;
			if (CurTheme->DisplayScatterDiagram)
			{
				MNMXCORL MinMax; 
				POINT	WinPoint;
				short	n=0, Height;
				
				if (CurrentMidPoint (Type,0,&MidPointAZ,&Length,&Height,FALSE,&MinMax,&WinPoint,0,&n,2,NULL,0))
					SVData = WinPtToBasePt(WinPoint);
			}
			if (CurTheme->hScatterFile)
				BT_PUT (CurTheme->hScatterFile,(LPSTR)&SVKey,(LPSTR)&SVData);
			CurTheme->Xmin = min (CurTheme->Xmin,SVKey.X);
			CurTheme->Ymin = min (CurTheme->Ymin,SVKey.Y);
			CurTheme->Xmax = max (CurTheme->Xmax,SVKey.X);
			CurTheme->Ymax = max (CurTheme->Ymax,SVKey.Y);
			CurTheme->NumVals++;

			rtn = TRUE;
    		break;   
    	
		case GF_CONNECTION_LINE_THEME:
		case GF_SINGLE_NONNUM_VALUE_THEME:
			rtn = TRUE;
			break;    		

		case GF_TWO_VALUE_THEME:
			break;
	} 
Exit:
	if (HaltReport)
		CurTheme->IsActive = FALSE;
{
#if ENABLETRACE
GSSiExitProg (1265);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}

BOOL DisplayPointInAreaThemes (short CurrentVPID)
{   
	UINT	iview; 
	LPTHEME	SaveTheme=CurTheme, PointTheme, AreaTheme;
	
    for (iview=0;iview<*pNumViewports;iview++) 
    {
        if (pViewportsD[iview]->pTheme && pViewportsD[iview]->pTheme->ID == GF_POINT_IN_AREA_THEME)
        {   
        	CurTheme = AreaTheme = pViewportsD[iview]->pTheme;  
			if (!CurTheme->IsActive ||
			    !CurTheme->VPDisplayed)
			    continue;
        	if (CurTheme->TargetViewport == CurrentVPID)
        	{
	        	if (!CurTheme->Pass && CurTheme->FidAreas != HFILE_ERROR) 
	        		GSSiClose (CurTheme->FidAreas);
	        		CurTheme->FidAreas = HFILE_ERROR;
	        }
	        else if (CurTheme->PointInAreaPointThemeVPID == CurrentVPID)
	        {
        	    PointTheme=pViewports[CurrentVPID-1]->pTheme;
	        	if (!CurTheme->Pass)
	        	{   
	        		HANDLE		hAreas=GSSiGlobAlloc (0,GHND,sizeof(THEMEAREAHEADER)*(CurTheme->NumAreas+1));
	        		LPTHEMEAREAHEADER	pAreaHeader=(LPTHEMEAREAHEADER)GlobalLock (hAreas);    
	        		UINT		iarea;   
	        		HANDLE		hMaskAccelerator=0;      
	        		HPDPOINT	pAreaPoints;
					short		SavePIASizeFactor = PIASizeFactor; 
					long		NumAreas = CurTheme->NumAreas;
	//				LPPIAAStruct pPIAA=(LPPIAAStruct)GlobalLock (hAccelerator);  
					HANDLE	hDB=0;
					LPGWDHEADER lpGWDHead; 

	                CurTheme->FidAreas = GSSiOpenFile (CurTheme->ScatterFile,0,OF_READ);  
	                if (CurTheme->FidAreas != HFILE_ERROR)
	                {
		                PIASizeFactor = 1;
		        		for (iarea=0;iarea<CurTheme->NumAreas;iarea++)
		        		{ 
		        			BigRead (CurTheme->FidAreas,(HPSTR)&pAreaHeader[iarea].Refno,4);
		        			BigRead (CurTheme->FidAreas,(HPSTR)&pAreaHeader[iarea].NumPoints,4);  
		        			pAreaHeader[iarea].hPoints = GSSiGlobAlloc (0,GMEM_MOVEABLE,pAreaHeader[iarea].NumPoints * sizeof(DPOINT));
		        			pAreaPoints = (HPDPOINT)GlobalLock (pAreaHeader[iarea].hPoints);
		        			BigRead (CurTheme->FidAreas,(HPSTR)pAreaPoints,pAreaHeader[iarea].NumPoints * sizeof(DPOINT));   
				      		GlobalUnlock (pAreaHeader[iarea].hPoints);
				      		GetPolyBoundsD (pAreaHeader[iarea].hPoints,pAreaHeader[iarea].NumPoints,&pAreaHeader[iarea].Bounds,TYPE_AREA);
		        		}
		        		GSSiClose (CurTheme->FidAreas);
		        		CurTheme->FidAreas = HFILE_ERROR;
		        		GSSiRemoveAndClear (CurTheme->ScatterFile);
		        		CurTheme = PointTheme; 
						if (OpenPointDispersionFile (BT_READ))
				        {   
				        	short	pos=BT_FIRST;  
				        	LPHANDLE	phAccelerator; 
				        	long		nRecs = BT_NUM_IN_INDEX (CurTheme->hDisperseFile);
				        	long		nLoaded=0;
	
	
							CreateStatusWind (hWndMain,-1,"Totaling by Area");
							StatusWindowUpdate (0,"Step 1", nRecs, nLoaded);
							while (ContinueProcessing && !BT_FIND (CurTheme->hDisperseFile,(LPSTR)&DisperseKey,pos,BT_ANY,(LPSTR)&PointDispersionData))
							{   
								pos = BT_NEXT;
		    		    		for (iarea=0;iarea<NumAreas;iarea++)   
				        		{
				        			if (PointInBounds (PointDispersionData.PointLocWorld,&pAreaHeader[iarea].Bounds))
				        				pAreaHeader[iarea].NumPointsInBounds++;	
								}
								StatusWindowUpdate (0,0, nRecs, ++nLoaded);
							}
							pos = BT_FIRST;   
							nLoaded = 0;
							StatusWindowUpdate (0,"Step 2", nRecs, nLoaded);
							while (ContinueProcessing && !BT_FIND (CurTheme->hDisperseFile,(LPSTR)&DisperseKey,pos,BT_ANY,(LPSTR)&PointDispersionData))
							{   
								pos = BT_NEXT;
		    		    		for (iarea=0;iarea<NumAreas;iarea++)   
				        		{
				        			if (PointInBounds (PointDispersionData.PointLocWorld,&pAreaHeader[iarea].Bounds))
				        			{
					        			pAreaPoints = (HPDPOINT)GlobalLock (pAreaHeader[iarea].hPoints);
							      		if (pAreaHeader[iarea].NumPointsInBounds > 10)
							      			phAccelerator = &pAreaHeader[iarea].hAccelerator;
							      		else
							      			phAccelerator = 0;
										if (POINT_IN_AREAD (PointDispersionData.PointLocWorld,pAreaHeader[iarea].NumPoints,pAreaPoints,1,0,0,phAccelerator))
					        				pAreaHeader[iarea].NumPointsInArea++;
							      		GlobalUnlock (pAreaHeader[iarea].hPoints);
					        		}	
								}
								StatusWindowUpdate (0,0, nRecs, ++nLoaded);
							}
							ClosePointDispersionFile (FALSE);
							DestroyStatusWindow(0);  
		        		}  
		        		CurTheme = AreaTheme;
		        		for (iarea=0;iarea<NumAreas;iarea++)   
		        		{   
		        			GSSiGlobFree (&pAreaHeader[iarea].hAccelerator);
		        			GSSiGlobFree (&pAreaHeader[iarea].hPoints);  
		        		}
	        			CreatePointInAreaDB (CurTheme->DataFile);
					    hDB = OpenGWDatabase (CurTheme->DataFile,BT_WRITE);
					    if (hDB)
					    {   
					    	LPPIADB	pPIADB;
					    	
						    lpGWDHead = (LPGWDHEADER)GlobalLock (hDB);  
						    pPIADB = (LPPIADB)lpGWDHead->GWDData;
			        		for (iarea=0;iarea<NumAreas;iarea++)  
			        		{   
			        			pPIADB->Refno = pAreaHeader[iarea].Refno;
			        			pPIADB->TotCount = pAreaHeader[iarea].NumPointsInArea;
			        			pPIADB->TotValue = pAreaHeader[iarea].TotalValueInArea;
		        				GWDReplaceRecord (lpGWDHead,0,0,-1);
		        			} 
		        			GlobalUnlock (hDB);
						    CloseGWDatabase (hDB);   
						    CurTheme->Pass=1;          
						    if (Display && !Printing)
						    	RedisplayWindow ();
					    }
		        	} 
	        		GSSiGlobUlFree (&hAreas);   
	        		PIASizeFactor = SavePIASizeFactor;
	        	}
	        }
        }  
    } 
    CurTheme = SaveTheme;
	return TRUE;
}

double ThemeComputeCurLength (void)
{
	double	d = 0;

	if (CurTheme->ComputeAreaAndLength && PolyIsHiPrecis)
	{
		switch (CurrentType)
		{
		case GF_AREA:
			ComputeAreaAreaD (lpDCurPoints,nPnts,&d);
			break;
		case GF_POLYLINE:
		case GF_LINE:
			d = GetPolyLengthD (lpDCurPoints,nPnts);
			break;
		}
	}
	return d;
}

double ThemeComputeCurArea (void)
{
	double	d = 0, Perimeter;
	MNMXCORD	Bounds;
	DPOINT	MidPoint;

	if (CurTheme->ComputeAreaAndLength && PolyIsHiPrecis)
	{
		switch (CurrentType)
		{
		case GF_POINT:
  			SetGlobalValueDPoint ("%PICKED_MIDPOINT",CurPointLocD);
			break;
		case GF_TEXT:
  			SetGlobalValueDPoint ("%PICKED_MIDPOINT",TXLoc);
			break;
		case GF_AREA:
			d = ComputeAreaAreaD (lpDCurPoints,nPnts,&Perimeter);
	   		GetPolyBoundsD2 (lpDCurPoints,nPnts,&Bounds,GF_AREA);
   			SetGlobalValueBounds ("%PICKED_BOUNDS",&Bounds);
			MidPoint = MinMaxMidPointD (&Bounds);
   			SetGlobalValueDPoint ("%PICKED_MIDPOINT",MidPoint);
			break;
		case GF_POLYLINE:
		case GF_LINE:
		case GF_CURVE:
	   		GetPolyBoundsD2 (lpDCurPoints,nPnts,&Bounds,GF_POLYLINE);
   			SetGlobalValueBounds ("%PICKED_BOUNDS",&Bounds);
			MidPoint = ComputePolylineMidpoint2 (lpDCurPoints,nPnts);
   			SetGlobalValueDPoint ("%PICKED_MIDPOINT",MidPoint);
			break;
		}
	}
	return d;
}


short DispersePoint (long Refno,int Class,int desc,LPVOID pValue)
#if ENABLETRACE
{GSSiEnterProg (1302);
#endif
{   
	short	st, MaxSequence;
	double	size; 
	POINT	OrigPoint;
	THEMEHIGHLIGHTKEY	ThemeHighlightKey;
	THEMEHIGHLIGHTDATA	ThemeHighlightData;
	BOOL	UseSymRect=TRUE;
	
	if (CurTheme->hHighlightFile && CurrentType != GF_PIXEL)
	{
		ThemeHighlightData.FileNum = FileNum;  
		ThemeHighlightData.SubFile = SubFile;  
		ThemeHighlightData.FileInIndex = FileInIndex;
		ThemeHighlightData.Segment = ItemSeg;
		ThemeHighlightData.Offset = CurrentItem; 
		ThemeHighlightData.Element = CurElement;
		switch (CurrentType)
		{
		case GF_POINT:
		case GF_TEXT:
			ThemeHighlightData.Point = CurPointLocD;
			break;
		default:
			ThemeHighlightData.Point.x = ThemeHighlightData.Point.y = 0;
			break;
		}
		ThemeHighlightData.Length = ThemeComputeCurLength ();
		ThemeHighlightData.Area = ThemeComputeCurArea ();
		ThemeHighlightData.Type = CurrentType;
		ThemeHighlightData.Desc = CurrentDesc;
		strncpy0 (ThemeHighlightData.Prefix,CurrentPrefix,MAX_PREFIX_LEN);
		strncpy0 (ThemeHighlightData.UDI,CurrentUDI,MAX_UDI_LEN);
		_fmemmove (ThemeHighlightData.Value,pValue,min(MAX_THEME_VALUE_LEN,CurTheme->ValueLen));
		ThemeHighlightKey.Refno = Refno; 
		ThemeHighlightKey.Class = Class; 
		FileBoundsToWBounds (&CurrentItemMinMax,&ThemeHighlightData.Bounds); 
/*		debugvalue++;
		if (debugvalue > 47)
			debugvalue++;*/
		BT_PUT (CurTheme->hHighlightFile,(LPSTR)&ThemeHighlightKey,(LPSTR)&ThemeHighlightData);
	}
			
	if (CurrentType != GF_POINT)
{
#if ENABLETRACE
GSSiExitProg (1302);
#endif
		return 0;
}
	if (!CurTheme->DispersePoints || !CurTheme->hDisperseFile)
{
#if ENABLETRACE
GSSiExitProg (1302);
#endif
		return 0; 
}
	DisperseKey.Point = CurPointLoc;
	if (CurTheme->DispersePoints == 1)
	{
		DisperseKey.Sequence = SHRT_MAX;
		if ((st = BT_FIND (CurTheme->hDisperseFile,(LPSTR)&DisperseKey,BT_FIRST,BT_GT,(LPSTR)&PointDispersionData)))
			st = BT_FIND (CurTheme->hDisperseFile,(LPSTR)&DisperseKey,BT_LAST,BT_ANY,(LPSTR)&PointDispersionData);
		else
			st = BT_FIND (CurTheme->hDisperseFile,(LPSTR)&DisperseKey,BT_PRIOR,BT_ANY,(LPSTR)&PointDispersionData);
		if (st) 
			DisperseKey.Sequence = 1;   
		else if (DisperseKey.Point.x == CurPointLoc.x && DisperseKey.Point.y == CurPointLoc.y)
			DisperseKey.Sequence++;                                                           
		else
			DisperseKey.Sequence = 1;  
		PointDispersionData.Num = 0;
	}
	else
	{   
		DisperseKey.Sequence = Class;
		if (BT_FIND (CurTheme->hDisperseFile,(LPSTR)&DisperseKey,BT_FIRST,BT_EQ,(LPSTR)&PointDispersionData))
			PointDispersionData.Num = 0; 
	}
	DisperseKey.Point = CurPointLoc;  

	PointDispersionData.FileNum = FileNum;  
	PointDispersionData.SubFile = SubFile;  
	PointDispersionData.FileInIndex = FileInIndex;
	PointDispersionData.Segment = ItemSeg;
	PointDispersionData.Offset = CurrentItem; 
	PointDispersionData.Element = CurElement;
	PointDispersionData.Refno = Refno; 
	PointDispersionData.Desc = desc; 
	PointDispersionData.Size = CurPointSize; 
	PointDispersionData.PointLocWorld = CurPointLocD;
	PointDispersionData.MSLink = CurMSLink;
	strcpy (PointDispersionData.Prefix,CurrentPrefix);
	strcpy (PointDispersionData.UDI,CurrentUDI);
	PointDispersionData.Num++;
    OrigPoint = CurPointLoc;
	if (CurTheme->DispersePoints == 1)  
	{  
		RECT	SymRect;   
		long	RectMax;
		
		if (ThemePointSym)
		{
			if (ThemePointSize < 0) 
			{
				UseSymRect = FALSE;
				size = -ThemePointSize *DeviceToScreenFactor;
			}
			else
				size = ThemePointSize / CurView->BaseUnitsPerPixel;
			size = min(max (size*GraphicsPointFactor,1),MaxPointSize);
		}
		else
			size = CurPointSize*ThemeWidthFactor*GraphicsPointFactor; 
		if (UseSymRect)
		{
		    SymRect = GetSymRect (desc);
			RectMax = max ((long)SymRect.right - (long)SymRect.left,(long)SymRect.bottom - (long)SymRect.top);   
			size *= (double)RectMax/200;
		} 
		PointDispersionData.Size = size; 
    	CurPointLoc = AddjustPointLoc (CurPointLoc,size,DisperseKey.Sequence);
	} 
	PointDispersionData.DisplayPoint = CurPointLoc;
	CurTheme->MaxDispersion = max (CurTheme->MaxDispersion,abs (CurPointLoc.x-OrigPoint.x));
	BT_PUT (CurTheme->hDisperseFile,(LPSTR)&DisperseKey,(LPSTR)&PointDispersionData);
{
#if ENABLETRACE
GSSiExitProg (1302);
#endif
	return (CurTheme->DispersePoints);
}
#if ENABLETRACE
}
#endif
} 

BOOL PickDispersedPoints (DPOINT PickPointBase,int PickAp,LPDOUBLE pNearDist)
#if ENABLETRACE
{GSSiEnterProg (1307);
#endif
{   
	long	StartX, EndX;
	short	st, i, Item; 
	DPOINT	NearPoint; 
	double	d, MinDist, AZ[3]={0,0,0};
	MNMXCORD Rect;
	POINT	FilePoint, PickPoint;
	HPDPOINT	HLTAreaPoints;	
	LPTHEME	SaveTheme=CurTheme;
	LPVIEWPORT	pSaveVP = CurView;
		
    if (CurView->Type == SUBVIEWPORT && CurView->Parent)
     	SetCurView (pViewports[CurView->Parent-1]);
	for (i=0;i<CurView->NumThemes;i++)
	{
		CurTheme = CurView->pThemes[i];
		if (!CurTheme->IsActive ||
		    !CurTheme->VPDisplayed)
		    continue;
		if (OpenPointDispersionFile (BT_READ))
        {   
        	if (hHighlightArea)
        	{
			    LPMNMXCORD	lpRect = (LPMNMXCORD) GlobalLock (hHighlightArea); 
			    
			    PickPoint = BasePtToWinPt ((LPDPOINT)lpRect);  
			    StartX = PickPoint.x - CurTheme->MaxDispersion;
			    PickPoint = BasePtToWinPt ((LPDPOINT)&lpRect->xmx);  
			    EndX = PickPoint.x + CurTheme->MaxDispersion;
			    lpRect++;
    			HLTAreaPoints = (LPDPOINT) lpRect;  
        	}
        	else
        	{
			    PickPoint = BasePtToWinPt (&PickPointBase);
			    if (!PickAp)
				{   
					d = *pNearDist/CurView->BaseUnitsPerPixel;
					d = min (d,USHRT_MAX);
					PickAp = IDNINT(d);
				}
				StartX = PickPoint.x - PickAp - CurTheme->MaxDispersion; 
				EndX = PickPoint.x + PickAp + CurTheme->MaxDispersion;
			} 
			StartX = max (StartX,SHRT_MIN);	
			DisperseKey.Point.x = StartX; 
			DisperseKey.Point.y = SHRT_MIN;
			DisperseKey.Sequence = 0;
			st = BT_FIND (CurTheme->hDisperseFile,(LPSTR)&DisperseKey,BT_FIRST,BT_GT,(LPSTR)&PointDispersionData);
			while (!st && DisperseKey.Point.x <= EndX)
			{   
				RECT	SymRect;
				long	RectMax;
				double	SymbolSize;
				
	        	if (hHighlightArea)
	        	{
					NearPoint = PointDispersionData.PointLocWorld; //WinPtToBasePt(DisperseKey.Point);
					FilePoint = DisperseKey.Point;
					NumPicked = 0; 
					if (POINT_IN_AREAD (NearPoint,nHighlightAreaPoints,HLTAreaPoints,1,0,0,hHighlightAreaAccelerator))
					{  
						PickListAdd (PointDispersionData.FileNum,PointDispersionData.SubFile,PointDispersionData.FileInIndex,
									 PointDispersionData.Segment,PointDispersionData.Refno,
									 PointDispersionData.Desc,0,
									 1,0,MinDist,AZ,0,PointDispersionData.Offset,PointDispersionData.Element,
								     NearPoint,NearPoint,NearPoint,NearPoint,0,0,&Rect,
								     FilePoint,FilePoint,FilePoint,1,NULL_ELEV,&NearPoint,&NearPoint); 
						PickList[0].IsDispersed = TRUE;
						strcpy (PickList[0].Prefix,PointDispersionData.Prefix);
						strcpy (PickList[0].UDI,PointDispersionData.UDI);
						ProcessPickedItem (0,FALSE); 
						PickList[0].Desc = CurrentDesc;
						//GetVal ("%PREFIX",PickList[0].Prefix);
						//GetVal ("%UDI",PickList[0].UDI);
						if (UnHighlightInArea) 
							RemoveFromHighlightList (PickList[0].Refno,0);
						else
							AddToHighlightList (PickList[0].Refno,&PickList[0],TRUE);
					}
				}
				else
				{
					NearPoint = WinPtToBasePt(PointDispersionData.DisplayPoint);   
					FilePoint = PointDispersionData.DisplayPoint;
					d = (double)PointDispersionData.Size * CurView->BaseUnitsPerPixel; 
				    SymRect = GetSymRect (PointDispersionData.Desc);
					RectMax = max ((long)SymRect.right - (long)SymRect.left,(long)SymRect.bottom - (long)SymRect.top);   
					SymbolSize = PointDispersionData.Size*(double)RectMax/200;
	                d = max (d,SymbolSize*CurView->BaseUnitsPerPixel);
					Rect.xmn = NearPoint.x - d;
					Rect.xmx = NearPoint.x + d;
					Rect.ymn = NearPoint.y - d;
					Rect.ymx = NearPoint.y + d;  
					AZ[0]=AZ[1]=AZ[2]=0;
					MinDist = idist (PointDispersionData.DisplayPoint,PickPoint)*CurView->BaseUnitsPerPixel;
					if (PickAp && MinDist <= max(d,PickApW))
					{
						if ((Item = PickListAdd (PointDispersionData.FileNum,PointDispersionData.SubFile,PointDispersionData.FileInIndex,
									 PointDispersionData.Segment,PointDispersionData.Refno,
									 PointDispersionData.Desc,0,
									 1,0,MinDist,AZ,0,PointDispersionData.Offset,PointDispersionData.Element,
								     NearPoint,NearPoint,NearPoint,NearPoint,0,0,&Rect,
								     FilePoint,FilePoint,FilePoint,1,NULL_ELEV,&NearPoint,&NearPoint)))
						{   
							PICKDATA PD=PickList[Item-1]; //dbug    
							short	ii;
							
							PickList[Item-1].IsDispersed = TRUE;
							strcpy (PickList[0].Prefix,PointDispersionData.Prefix);
							strcpy (PickList[0].UDI,PointDispersionData.UDI);
							ProcessPickedItem (Item-1,FALSE); 
							PickList[Item-1].Desc = CurrentDesc;
							GetVal ("%PREFIX",PickList[Item-1].Prefix);
							GetVal ("%UDI",PickList[Item-1].UDI); 
							PD=PickList[Item-1]; //dbug
							ii=1;
						}
						
						if (NumPicked == MaxPick && pNearDist)
						{
							*pNearDist = min (*pNearDist,PickList[0].OffDist);
							d = *pNearDist/CurView->BaseUnitsPerPixel;
							d = min (d,USHRT_MAX);
							PickAp = IDNINT(d);
							EndX = PickPoint.x + PickAp + CurTheme->MaxDispersion; 
						} 
					}
				}
				st = BT_FIND (CurTheme->hDisperseFile,(LPSTR)&DisperseKey,BT_NEXT,BT_ANY,(LPSTR)&PointDispersionData);
			}
			BT_CLOSE (CurTheme->hDisperseFile);
			CurTheme->hDisperseFile = 0;    
			if (hHighlightArea)
				GlobalUnlock (hHighlightArea);
		}
	}
	CurTheme = SaveTheme;
	CurView = pSaveVP;
{
#if ENABLETRACE
GSSiExitProg (1307);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL CreatePointDispersionFile (void)
#if ENABLETRACE
{GSSiEnterProg (1308);
#endif
{   
	LPSTR pName;
	BTVARDESC	BTVar[3];	
	
	CurTheme->hDisperseFileName = GSSiGlobAlloc (1022,GMEM_MOVEABLE,256);
	pName = GlobalLock (CurTheme->hDisperseFileName);
	GSSiGetTempFileName (0,"gmp",0,pName);
	BTVar[0].BT_VARTYP=BT_INTEGER;
	BTVar[0].BT_VARLEN=4;
	BTVar[0].BT_VAROFF=0;
	BTVar[1].BT_VARTYP=BT_INTEGER;
	BTVar[1].BT_VARLEN=4;
	BTVar[1].BT_VAROFF=4;
	BTVar[2].BT_VARTYP=BT_INTEGER;
	BTVar[2].BT_VARLEN=2;
	BTVar[2].BT_VAROFF=8;
	BT_CREATE (pName, sizeof(PointDispersionData), FALSE, 3, 1, (LPBTVARDESC)BTVar, FALSE, 0, 0, FALSE);
	GlobalUnlock (CurTheme->hDisperseFileName);    
	CurTheme->MaxDispersion=0;
{
#if ENABLETRACE
GSSiExitProg (1308);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}
BOOL ClosePointDispersionFile (BOOL CloseAll)
#if ENABLETRACE
{GSSiEnterProg (1309);
#endif
{   
	short	st, Desc, LastDesc = 0,NumClass,ClassID[MAX_THEME_CLASSES+2];
	long	ClassCounts[MAX_THEME_CLASSES+2]; 
	POINT	PointLoc, CurPoint, Points[2];  
	short	pos = BT_FIRST, MaxCount, i, j, Order[MAX_THEME_CLASSES+2], classsym;  
	double	MaxSize, Size, ActualSize;   
	HPEN	hPen, OldPen, hDashPen;
	char	str[32]; 
	BOOL	First=TRUE;
	double	DisperseFactor;
	
	switch (CurTheme->ACCDisperse)
	{
		case 2:
			DisperseFactor = 0;
			break;
		case 1:
			DisperseFactor = 0.02;
			break;
		default:
			DisperseFactor = 1;
	}
	if (CurTheme->DispersePoints == 2)
	{   
		SaveDC (CurView->hDC);
		CreateClassPens ();
		hPen = SelectObject(CurView->hDC, GetStockObject(BLACK_PEN)); 
		hDashPen = CreatePen (PS_DOT,1,0);
    	SetDisplayMode (CurView->hDC, GF_TEXTMODE);
	    GSSiDeleteObject(&CurView->hRgn);
        CurView->hRgn = CreateVPRgn(FALSE,FALSE);
        SelectClipRgn (CurView->hDC,CurView->hRgn);
        GSSiDeleteObject(&CurView->hRgn); 
        switch (CurTheme->AccumPointSymbolOpt)
        {
        	case 0:
				Desc = GetDictSymbolNumber ("CIRCLE");  
				break;
        	case 1:
				Desc = GetDictSymbolNumber ("SQUARE");  
				break;
		}
		NumClass=0;   
		CurPoint.x = SHRT_MAX;
		CurPoint.y = SHRT_MIN;
Next:    
		{
			st = BT_FIND (CurTheme->hDisperseFile,(LPSTR)&DisperseKey,pos,BT_ANY,(LPSTR)&PointDispersionData);
			pos = BT_NEXT; 
			if (st || CurPoint.x != DisperseKey.Point.x || CurPoint.y != DisperseKey.Point.y)
			{   
				if (!First)
				{
					Points[0] = CurPoint; 
					j=NumClass; 
					if (CurTheme->AccumPointBaseSize > 0)
						MaxSize = CurTheme->AccumPointBaseSize*DeviceToScreenFactor;
					else if (CurTheme->AccumPointBaseSize < 0)
						MaxSize = -CurTheme->AccumPointBaseSize * BaseDistToWinDist; 
					else
						MaxSize = PointDispersionData.Size;
					GetClassOrder (NumClass,ClassCounts,Order);
					while (j--)
					{   
						RECT	SymRect;
						double	RectMax;
						
						i = Order[j];
						ThemePointColor = GlobalColors[0]=RGB(255,0,0); 
						ThemePointUseHalfTone = CurTheme->UseHalfTone; 
						switch (ClassID[i])
						{
							case -1:
								if (CurTheme->NoDataBrush)
									SelectObject (CurView->hDC,CurTheme->NoDataBrush);
								break;
							case -2: 
								if (CurTheme->InvalidDataBrush)
									SelectObject (CurView->hDC,CurTheme->InvalidDataBrush);
								break;
							default:
								SelectObject (CurView->hDC,CurTheme->ClassBrush[ClassID[i]]);
								classsym = CurTheme->ClassSymbol[ClassID[i]];
								ThemePointColor = GlobalColors[0]=CurTheme->ClassColor[ClassID[i]]; 
								break;
						}
						if (CurTheme->AccumPointSymbolOpt == 2)  
						{
							if (classsym)
								Desc = classsym; 
							else
								Desc = LastDesc;
						}
			    		HaveVarFillColor = TRUE;
						Size = DispersePointSize (MaxSize,MaxCount); 
					    SymRect = GetSymRect (Desc);
						RectMax = max ((long)SymRect.right - (long)SymRect.left,(long)SymRect.bottom - (long)SymRect.top);   
						ActualSize = (Size*(double)RectMax*DisperseFactor)/200;
						PointLoc = AddjustPointLoc (CurPoint,ActualSize,i+1);
						Size = DispersePointSize (MaxSize,ClassCounts[i]);
						Points[1]=PointLoc; 
						OldPen = SelectObject (CurView->hDC,hDashPen); 
						if (CurTheme->AccumPointLink)
							Polyline (CurView->hDC,Points,2); 
						SelectObject (CurView->hDC,OldPen);
						DisplayPointItem (CurView->hDC,PointLoc,Size,0,Desc,0);
						ltoa (ClassCounts[i],str,10);
						if (IDNINT(Size) && CurTheme->AccumPointText)
						{
							COLORREF	OldColor = SetTextColor (CurView->hDC,NearestColor (ThemePointColor, RGB(255,255,255),0));
						    SetBkMode (CurView->hDC,TRANSPARENT); 
							DispText (CurView->hDC,FALSE,PointLoc.x,PointLoc.x, PointLoc.y,0, 2,2,
							  		ActualSize/2,1,1,2, FALSE,0,str,0,FALSE,0,0,-1,0,0,0,0,0,CurTheme->UseHalfTone,0,0,0,0,0,0);  
							SetTextColor (CurView->hDC,OldColor);
						}
					}
				} 
				First = FALSE;
				NumClass = MaxCount = 0;
				MaxSize = 0;
				CurPoint = DisperseKey.Point; 
			}
			MaxSize = max (MaxSize,PointDispersionData.Size);   
			LastDesc = PointDispersionData.Desc;
			ClassID[NumClass] = DisperseKey.Sequence;
			ClassCounts[NumClass++] = PointDispersionData.Num;
			MaxCount = max (MaxCount,PointDispersionData.Num);
			if (!st)
				goto Next; 
		}
		SelectObject(CurView->hDC,hPen); 
		DeleteObject (hDashPen);
		
		RestoreDC (CurView->hDC,-1);  
	}
	BT_CLOSE (CurTheme->hDisperseFile);
	CurTheme->hDisperseFile = 0;
	DestroyClassPens();
{
#if ENABLETRACE
GSSiExitProg (1309);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
} 

double DispersePointSize (double InSize, long Count)
#if ENABLETRACE
{GSSiEnterProg (1310);
#endif
{   
	float	Size;
	
	switch (CurTheme->AccumPointSizeOpt)
	{
		case 2:
			Size = InSize * sqrt ((double)(Count)); 
            break;
		case 3:
			Size = InSize * pow ((double)(Count),0.3333333333); 
            break;
        case 1:  
			Size = InSize * Count; 
            break;
		case 5:
			Size = InSize * (1+log ((double)(Count))); 
            break;
		case 4:
			Size = InSize * (1+log10 ((double)(Count))); 
            break;
		case 0:
			Size = InSize;
			break;
	}  
	Size *= GraphicsPointFactor;
{
#if ENABLETRACE
GSSiExitProg (1310);
#endif
	return Size;
}
#if ENABLETRACE
}
#endif
}
        
BOOL OpenPointDispersionFile (short mode)
#if ENABLETRACE
{GSSiEnterProg (1311);
#endif
{   
	LPSTR	pName;
	
	if (!CurTheme->DispersePoints)
{
#if ENABLETRACE
GSSiExitProg (1311);
#endif
		return FALSE;
}
	if (!CurTheme->hDisperseFileName)
	{
		if (mode == BT_READ)
{
#if ENABLETRACE
GSSiExitProg (1311);
#endif
			return FALSE;
}
		CreatePointDispersionFile ();
	}
	pName = GlobalLock (CurTheme->hDisperseFileName);
	CurTheme->hDisperseFile = BT_OPEN (pName, 0, mode, 0); 
	if (mode == BT_WRITE)
		BT_CLEAR (CurTheme->hDisperseFile);
	GlobalUnlock (CurTheme->hDisperseFileName);
{
#if ENABLETRACE
GSSiExitProg (1311);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}
BOOL DeletePointDispersionFile (void)
#if ENABLETRACE
{GSSiEnterProg (1312);
#endif
{   
	LPSTR pName; 
	OFSTRUCTGM	OFStruct;
	
	if (!CurTheme->hDisperseFileName)
{
#if ENABLETRACE
GSSiExitProg (1312);
#endif
		return FALSE;
}
	pName = GlobalLock (CurTheme->hDisperseFileName);
	GSSiRemove (pName); 
	GSSiGlobUlFree (&CurTheme->hDisperseFileName);
{
#if ENABLETRACE
GSSiExitProg (1312);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
} 

BOOL FAR PASCAL CENFIELDSMsgProc(HWND hWndDlg,int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (620);
#endif
{    
	LPGWDHEADER lpGWDHead;
    LPOPENFILEDATA  FilePtr;
    LPOPENSQLDATA   SQLPtr;
	LPFIELDINFO lpFieldInfo;  
	BOOL	ShowDesc;
    int     nItems, TabStops[2]={50,1500}; 
	char	str[1024], Title[1024]; 
	static	char	SearchString[128]="";
	static 	char	WantTableName[32];
	int		i;
	static	BOOL	ListIsTables=TRUE;   
	static	HANDLE	hKeyDB=0;
	HANDLE	hText;  
	HANDLE	hItems;
	LPINT	lpItems;
	LPSTR	pText, pTab, pEnd; 
   	long	Offset;
  	BOOL	First; 
  	static	short	TextFld=4;  

 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (620);
#endif
 	return (BRtn);
}
 switch(Message)
   {
    case WM_INITDIALOG: 
        SendDlgItemMessage (hWndDlg,IDC_OUTPUTREPLACE,(UINT)BM_SETCHECK,TRUE,(LPARAM)0L);
        SendDlgItemMessage (hWndDlg,IDC_FIELDS,LB_SETTABSTOPS,2,(LPARAM)&TabStops);
        if (TextFld == 4)
        {
	        SendDlgItemMessage (hWndDlg,IDC_AREATYPEBLKGRP,(UINT)BM_SETCHECK,TRUE,(LPARAM)0L);
		    if (!OpenDataFile ("[%DL]attribut\\SF3KEY.GMD","",BT_READ,&hKeyDB))  
		    {   
		    	GSSiMsgBox (hWndDlg,"Unable to open SF3KEY.GMD",0,MB_ICONEXCLAMATION,0);
		        PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
		    	break; 
		    }
		}
		else 
		{
	         SendDlgItemMessage (hWndDlg,IDC_AREATYPEBLOCK,(UINT)BM_SETCHECK,TRUE,(LPARAM)0L);
		     if (!OpenDataFile ("[%DL]attribut\\SF1KEY.GMD","",BT_READ,&hKeyDB)) 
			 {   
				GSSiMsgBox (hWndDlg,"Unable to open SF1KEY.GMD",0,MB_ICONEXCLAMATION,0);
			    PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
				break; 
			 }
		}
    	hWndFieldList = hWndDlg;  
 		if (pFLRect)
 		{   
 			MoveWindow(hWndDlg, pFLRect->left,pFLRect->top, pFLRect->right-pFLRect->left, pFLRect->bottom-pFLRect->top, FALSE);
 		} 
		SetDlgItemText (hWndDlg,IDC_SEARCHSTRING,SearchString);
 		if (!ListIsTables)
 			goto ShowTable;
  	case GSSI_REINITDIALOG:
		
		 break;                              

    case WM_CLOSE:
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break;

    case WM_COMMAND:
#if WIN32
        switch(LOWORD(wParam))
#else
        switch(wParam)
#endif
           {  
           	  case IDCANCEL:
			     CloseDataFile (TRUE, &hKeyDB); 
                 EndDialog(hWndDlg, FALSE);
			     hWndFieldList = 0;
		         break;  
              
              case IDC_AREATYPEBLOCK:
			     CloseDataFile (TRUE, &hKeyDB); 
			     if (!OpenDataFile ("[%DL]attribut\\SF1KEY.GMD","",BT_READ,&hKeyDB)) 
				 {   
					GSSiMsgBox (hWndDlg,"Unable to open SF1KEY.GMD",0,MB_ICONEXCLAMATION,0);
				    PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
					break; 
				 }
			     TextFld = 5;
              	 break;
              	 
              case IDC_AREATYPEBLKGRP:  
              case IDC_AREATYPETRACT:
			     CloseDataFile (TRUE, &hKeyDB); 
			     if (!OpenDataFile ("[%DL]attribut\\SF3KEY.GMD","",BT_READ,&hKeyDB)) 
				 {   
					GSSiMsgBox (hWndDlg,"Unable to open SF3KEY.GMD",0,MB_ICONEXCLAMATION,0);
				    PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
					break; 
				 }
			     TextFld = 4;
              	 break;
              	 
              case IDC_COMPUTETITLE:
           	  case IDOK: 
           	  {
					nItems = GetLBSelectedItems (hWndDlg,IDC_FIELDS,&hItems); 
					if (!nItems)
						break;   
           	        hText = GSSiGlobAlloc ( 263,GHND,USHRT_MAX);
	                pText = GlobalLock (hText);  
					lpItems = (LPINT)GlobalLock (hItems); 
					if (wParam == IDC_COMPUTETITLE)
	            	{   
	            		short	NumSpace, NumTargetSpace=0;
	            		LPSTR	pstr;  
	            		short	NumSpaceLoc[64];
	            		
	            		_fmemset (NumSpaceLoc,0,sizeof(NumSpaceLoc));
						*Title = 0;
	                	SendDlgItemMessage(hWndDlg,IDC_FIELDS,LB_GETTEXT,*lpItems,(LPARAM)str); 
	                	pTab = _fstrchr (str,'\t');
	                	pTab++;  
	                	pstr = pTab;
	                	while (*pstr++ == ' ')
	                		NumTargetSpace++;  
	                	for (i=0;i<*lpItems;i++)
	                	{
		                	SendDlgItemMessage(hWndDlg,IDC_FIELDS,LB_GETTEXT,i,(LPARAM)str);
		                	pTab = _fstrchr (str,'\t');
		                	pTab++;
		                	NumSpace = 0;
		                	pstr = pTab;
		                	while (*pstr++ == ' ')
		                		NumSpace++; 
		                	if (i<2 || (NumSpace < NumTargetSpace && _fstrnicmp (pTab,"Total:",6)))
		                	{ 
			                	if ((pEnd = _fstrrchr (pTab,'\t'))) 
			                		*pEnd = 0; 
			                	if (NumSpaceLoc[NumSpace])
			                		Title[NumSpaceLoc[NumSpace]] = 0;
			                	else
			                		NumSpaceLoc[NumSpace] = _fstrlen (Title);
			                	if (i)
			                		_fstrcat (Title,"\r\n"); 
			                	_fstrcat (Title,pTab);
			                	OneSpace (Title); 
			                	if (*LastChr (Title) == ']')
			                	{
			                		if ((pEnd = _fstrrchr (Title,'[')))
			                			*pEnd = 0;
			                	}
			                }
		                }
	            	}
					First = TRUE;
					*pText = 0;
					while (nItems--)
					{   
	                	SendDlgItemMessage(hWndDlg,IDC_FIELDS,LB_GETTEXT,*lpItems++,(LPARAM)str); 
	                	if (*str != '\t')
	                	{   
	                		LPSTR	pVarID;
	                		
							pTab = _fstrchr (str,'\t');
							*pTab++ = 0;  
							pVarID = pTab;
		                	if ((pEnd = _fstrrchr (pVarID,'\t'))) 
		                		*pEnd = 0; 
		                	OneSpace (pVarID); 
							if (First) 
							{
								First=FALSE;
								sprintf (pText,"[%s]",str);  
								if (wParam == IDC_COMPUTETITLE)  
								{   
									if (nItems)
										_fstrcat (Title,"\r\nSum of the following:");
									sprintf (_fstrchr (Title,0),"\r\n%s",pVarID);
								}
							}
							else
							{  
								short l=_fstrlen (pText);
								
								if (*pText != '$')
								{
									_fmemmove (&pText[5],pText,l);
									_fstrncpy (pText,"$SUM(",5);
								}
								sprintf (_fstrchr (pText,0),",[%s]",str); 
								if (wParam == IDC_COMPUTETITLE)
									sprintf (_fstrchr (Title,0),"\r\n%s",pVarID);
							}
		                }
					} 
					if (*pText == '$')
						_fstrcat (pText,")");	
					GSSiGlobUlFree (&hItems); 
					if (wParam == IDC_COMPUTETITLE)
					{   
						SetDlgItemText (hWndDlg,IDC_EDITTITLE,Title);
						GSSiGlobUlFree (&hText); 
						break;
					}
	            	if (CensusValueField) 
	            	{
						if (SendDlgItemMessage (hWndDlg,IDC_OUTPUTREPLACE,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L))
							SetDlgItemText (CensusValueWnd,CensusValueField,pText);
						else if (SendDlgItemMessage (hWndDlg,IDC_OUTPUTAPPEND,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L)) 
						{
							GetDlgItemText (CensusValueWnd,CensusValueField,str,sizeof(str)-_fstrlen(pText)-1);
							_fstrcat (str,pText);
							SetDlgItemText (CensusValueWnd,CensusValueField,str); 
						}
					}
	            	if (CensusTitleField)
	            	{
	            		GetDlgItemText (hWndDlg,IDC_EDITTITLE,Title,sizeof(Title)-1);
	            		SetDlgItemText (CensusValueWnd,CensusTitleField,Title);      
	            	}
					GSSiGlobUlFree (&hText); 
			     	CloseDataFile (TRUE, &hKeyDB); 
                 	EndDialog(hWndDlg, FALSE);
			     	hWndFieldList = 0; 
			     }
		         break;  
         
           	  case IDC_SHOWCENSUSDOC:
           	  	 if (TextFld==4)
           	  	 	ProcessText ("$OPENAPP([%DL]attribut\\sf3.pdf)");
           	  	 else
           	  	 	ProcessText ("$OPENAPP([%DL]attribut\\sf1.pdf)");
           	  	 break;
           	  	 
           	  case IDC_SHOWTABLES: 
           	  {
				short	pos=BT_FIRST; 
				long	Offset; 
				short	HaveSearchString;
                
                ListIsTables = TRUE;
                GetDlgItemText (hWndDlg,IDC_SEARCHSTRING,SearchString,sizeof(SearchString)-1);  
                OneSpace (SearchString);
                HaveSearchString = _fstrlen (SearchString);
				SendDlgItemMessage (hWndDlg,IDC_FIELDS,LB_RESETCONTENT,0,0);
				SQLPtr = (LPOPENSQLDATA)GlobalLock (hKeyDB);
				FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle);
				lpGWDHead = (LPGWDHEADER)GlobalLock (FilePtr->FileHandle);
				while (!BT_FIND (lpGWDHead->BTHandle[1],(LPSTR)str,pos,BT_ANY, (LPSTR)&Offset))
				{   
					LPSTR	pID, pstr, pEnd, CountLoc;  
					short	lstr;   
					char	OutLine[256], TableName[32];
					
					if (pos == BT_FIRST)
						pos = BT_NEXT;
					else
					{   
					 	FillGWDData (lpGWDHead,Offset);
			 			GMDGetCharFieldVal (lpGWDHead,1,str);
			 			_fstrcpy (TableName,str); 
			 			pID = _fstrchr (str,0)+1;  
						_fstrcat (str,"\t"); 
			 			GMDGetCharFieldVal (lpGWDHead,TextFld,pID);
			 			if ((!_fstrnicmp (TableName,"PCT",3) || !_fstrnicmp (TableName,"HCT",3)) && 
		          			!SendDlgItemMessage (hWndDlg,IDC_AREATYPETRACT,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L))
		          			goto NextTable;
			 			if (HaveSearchString)
			 			{   
			 				char	SearchString2[128];
			 				LPSTR	pString = SearchString2;
			 				
			 				_fstrcpy (SearchString2,SearchString);
			 				while (*pString)
			 				{
			 					if ((pEnd = _fstrchr (pString,' ')))
			 						*pEnd++ = 0;
			 					else
			 						pEnd = _fstrchr (pString,0);
			 					if (!_fstrstr (pID,pString))
					 				goto NextTable;
			 					pString = pEnd;
			 				}
			 			}
						pstr = str;
						CountLoc = pID;
						while ((lstr = _fstrlen (CountLoc)) > 100)
						{   
							pEnd = &CountLoc[100];
							while (*pEnd != ' ')
								pEnd--;
							*pEnd = 0;   
							sprintf (OutLine,"%s\t%s",pstr,TableName);
							SendDlgItemMessage (hWndDlg,IDC_FIELDS,LB_ADDSTRING,0,(LPARAM)(OutLine)); 
							pstr = pEnd; 
							*pstr = '\t';                
							CountLoc = pstr;
						}
						sprintf (OutLine,"%s\t%s",pstr,TableName);
						SendDlgItemMessage (hWndDlg,IDC_FIELDS,LB_ADDSTRING,0,(LPARAM)(OutLine)); 
			NextTable:;					
					}
				}
				GlobalUnlock (FilePtr->FileHandle);           
				GlobalUnlock (SQLPtr->OFHandle);
				GlobalUnlock (hKeyDB);
			}
			break;
			           	  
	          case IDC_FIELDS:
	          switch(HIWORD(wParam))
	          {
                case LBN_SELCHANGE:
	            {
	            	short	pos=BT_FIRST, cond=BT_GE, ltab; 
	            	
					nItems = GetLBSelectedItems (hWndDlg,IDC_FIELDS,&hItems);    
					if (ListIsTables)
					{   
						if (nItems > 1)
						{
							GSSiGlobFree (&hItems);
							break;
						} 
						lpItems = (LPINT)GlobalLock (hItems);
	                	SendDlgItemMessage(hWndDlg,IDC_FIELDS,LB_GETTEXT,*lpItems,(LPARAM)str);   
	                	pTab = _fstrrchr (str,'\t');
	                	pTab++;
	                	_fstrcpy (WantTableName,pTab);
	                	ListIsTables = FALSE; 
	                	GSSiGlobUlFree (&hItems);
	          ShowTable:
	          			pos=BT_FIRST;
	          			cond=BT_GE;
						SendDlgItemMessage (hWndDlg,IDC_FIELDS,LB_RESETCONTENT,0,0);
						SQLPtr = (LPOPENSQLDATA)GlobalLock (hKeyDB);
						FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle);
						lpGWDHead = (LPGWDHEADER)GlobalLock (FilePtr->FileHandle);
		    			SetFieldToMinVal (lpGWDHead,0);
				 		SetFieldValFromCharAndName(lpGWDHead,"TABLE",WantTableName,FALSE);
			            GWDFormKey(lpGWDHead,2,TRUE,0,0);
			            ltab = _fstrlen (WantTableName); 
						while (!BT_FIND (lpGWDHead->BTHandle[2],lpGWDHead->pKeys[2],pos,cond, (LPSTR)&Offset))
						{   
							LPSTR	pID, pstr, pEnd, CountLoc;  
							short	lstr;   
							char	OutLine[256], TableName[32]; 
							if (_fstrnicmp (WantTableName,lpGWDHead->pKeys[2],ltab))
								break;
							cond = BT_ANY;
							pos = BT_NEXT;
						 	FillGWDData (lpGWDHead,Offset);
				 			GMDGetCharFieldVal (lpGWDHead,2,str);
				 			_fstrcpy (TableName,str); 
				 			pID = _fstrchr (str,0)+1;  
							_fstrcat (str,"\t"); 
				 			GMDGetCharFieldVal (lpGWDHead,TextFld,pID); 
							pstr = str;
							CountLoc = pID;
							while ((lstr = _fstrlen (CountLoc)) > 100)
							{   
								pEnd = &CountLoc[100];
								while (*pEnd != ' ')
									pEnd--;
								*pEnd = 0;   
								sprintf (OutLine,"%s\t%s",pstr,TableName);
								SendDlgItemMessage (hWndDlg,IDC_FIELDS,LB_ADDSTRING,0,(LPARAM)(OutLine)); 
								pstr = pEnd; 
								*pstr = '\t';                
								CountLoc = pstr;
							}
							sprintf (OutLine,"%s\t%s",pstr,TableName);
							SendDlgItemMessage (hWndDlg,IDC_FIELDS,LB_ADDSTRING,0,(LPARAM)(OutLine)); 
						}
						GlobalUnlock (FilePtr->FileHandle);           
						GlobalUnlock (SQLPtr->OFHandle);
						GlobalUnlock (hKeyDB);
	                	break;
					}
					else
			        	PostMessage(hWndDlg, WM_COMMAND, IDC_COMPUTETITLE, 0L);
					GSSiGlobFree (&hItems);
	             }
                 break;
           	 }
             break; 
           }
         break;
    default:
{
#if ENABLETRACE
GSSiExitProg (620);
#endif
        return FALSE;
}
   } 
{
#if ENABLETRACE
GSSiExitProg (620);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL GetFieldList (HWND hWnd,HANDLE hDB,LPRECT rect, LPSTR Title,LPSTR Outvar)
{
    char	Dialog[12]="FIELDS";  
    int		nRc;

	if (!hDB)
    	return FALSE;
    hFLDB = hDB; 
    pFLRect = rect;  
    AutoCloseFieldDB = TRUE;
    FieldsDisplayOpt = 5; 
	pFieldsTitle = Title;
	lpfnFIELDSMsgProc = (DLGPROC) MakeProcInstance((FARPROC)FIELDSMsgProc, hInst);
    nRc = DialogBox(hInst, (LPSTR)Dialog, hWnd, lpfnFIELDSMsgProc);
    FreeProcInstance(lpfnFIELDSMsgProc);  
	if (nRc)
		SetGlobalValue4 (Outvar,GetFieldOutput,TRUE);  
 	pFieldsTitle = 0;
    return nRc;
}
  
BOOL DisplayFieldList (HWND hWnd,HANDLE hDB,LPRECT rect, short type,LPSTR Title)
#if ENABLETRACE
{GSSiEnterProg (618);
#endif
{
    LPOPENFILEDATA  FilePtr;
    LPOPENSQLDATA   SQLPtr;
    char	Dialog[12]="FIELDS";  
	HWND	hWndDlg;
	BOOL	rtn=FALSE;
    
    if (!hDB && (!type || type == -2))
{
#if ENABLETRACE
GSSiExitProg (618);
#endif
    	return FALSE;
}
    pFLRect = rect;  
    AutoCloseFieldDB = FALSE;
    switch (type) 
    {   
    	case 1:
    	hFLDB = 0;
    	_fstrcpy (Dialog,"GLOBALS");  
    	break;
    	case 3:
    	hFLDB = (HANDLE)2;
    	_fstrcpy (Dialog,"GLOBALS2");  
    	break;
    	case 2:
    	hFLDB = (HANDLE)1;
    	hFieldsDLT = hDB;
    	_fstrcpy (Dialog,"FIELDS"); 
    	break; 
    	case -2: //same as 0 but close db on field dialog close
    	AutoCloseFieldDB = TRUE;
    	type = 0;
    	case -1: 
    	case 0:
    	{
			SQLPtr = (LPOPENSQLDATA)GlobalLock (hDB);
			FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle);
    		if (FilePtr->Type == GMCENSUS_DATAFILE) 
    		{
    			type = 4;
    			_fstrcpy (Dialog,"CENFIELDS");  
    		}
			GlobalUnlock (SQLPtr->OFHandle);
			GlobalUnlock (hDB);
    	}	
    	break;
    }
    FieldsDisplayOpt = type; 
    if (type == 4)   
    {   
    	short	nRc;
    	
		lpfnFIELDSMsgProc = (DLGPROC) MakeProcInstance((FARPROC)CENFIELDSMsgProc, hInst);
        nRc = DialogBox(hInst, (LPSTR)Dialog, hWnd, lpfnFIELDSMsgProc);
        FreeProcInstance(lpfnFIELDSMsgProc);    
{
#if ENABLETRACE
GSSiExitProg (618);
#endif
    	return nRc;
}   
	}
	else
		lpfnFIELDSMsgProc = (DLGPROC) MakeProcInstance((FARPROC)FIELDSMsgProc, hInst);
	if (!hWndFieldList)
	{
	    hFLDB = hDB; 
		hWndDlg = CreateDialog(hInst, (LPSTR)Dialog,hWnd, lpfnFIELDSMsgProc);
		if (hWndDlg)
		{
			rtn = TRUE;
			if (hWndDlg && Title && *Title)
				SetWindowText (hWndDlg,Title);
		}
	}
{
#if ENABLETRACE
GSSiExitProg (618);
#endif
    return rtn;
}
#if ENABLETRACE
}
#endif
} 

void DestroyFieldList (void)
#if ENABLETRACE
{GSSiEnterProg (619);
#endif
{   
	if (hWndFieldList)
	{
		DestroyWindow(hWndFieldList);
		hWndFieldList = 0;
		FreeProcInstance((FARPROC) lpfnFIELDSMsgProc);
	}
{
#if ENABLETRACE
GSSiExitProg (619);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

BOOL FAR PASCAL FIELDSMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (620);
#endif
{    
    LPOPENFILEDATA  FilePtr;
    LPOPENSQLDATA   SQLPtr;
	LPFIELDINFO lpFieldInfo;  
	char	str[512]; 
	int		i;
	static	BOOL	AddSpaces=TRUE, AddBrackets=TRUE, AddCommas=FALSE, AddTabs=FALSE, AddQuotes=FALSE, FirstShow, ShowDesc=FALSE, AddNewLine=FALSE;

 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (620);
#endif
 	return (BRtn);
}
 switch(Message)
   {
    case WM_INITDIALOG: 
    	hWndFieldList = hWndDlg;  
 		if (pFLRect)
 		{   
 			
 			MoveWindow(hWndDlg, pFLRect->left,pFLRect->top, pFLRect->right-pFLRect->left, pFLRect->bottom-pFLRect->top, FALSE);
 		}
		FirstShow = TRUE;
		if (FieldsDisplayOpt == 5)
		{
			if (pFieldsTitle)
				SetWindowText (hWndDlg,pFieldsTitle);
			ShowWindow(GetDlgItem(hWndDlg,IDOK),SW_SHOW);   
			ShowWindow(GetDlgItem(hWndDlg,IDCANCEL),SW_SHOW);   
			ShowWindow(GetDlgItem(hWndDlg,IDC_SPACES),SW_HIDE);   
			ShowWindow(GetDlgItem(hWndDlg,IDC_BRACKETFIELDS),SW_HIDE);   
			ShowWindow(GetDlgItem(hWndDlg,IDC_COMMASBTWNFIELDS),SW_HIDE);   
			ShowWindow(GetDlgItem(hWndDlg,IDC_QUOTESAROUNDFIELDS),SW_HIDE);   
			ShowWindow(GetDlgItem(hWndDlg,IDC_TABBTWNFIELDS),SW_HIDE);   
			ShowWindow(GetDlgItem(hWndDlg,IDC_ADDNEWLINE),SW_HIDE);   
			SendDlgItemMessage (hWndDlg,IDC_BRACKETFIELDS,(UINT)BM_SETCHECK,TRUE,(LPARAM)0L); 
			SendDlgItemMessage (hWndDlg,IDC_COMMASBTWNFIELDS,(UINT)BM_SETCHECK,TRUE,(LPARAM)0L); 
		}
		else
		{
			SendDlgItemMessage (hWndDlg,IDC_SPACES,(UINT)BM_SETCHECK,AddSpaces,(LPARAM)0L); 
			SendDlgItemMessage (hWndDlg,IDC_BRACKETFIELDS,(UINT)BM_SETCHECK,AddBrackets,(LPARAM)0L); 
			SendDlgItemMessage (hWndDlg,IDC_COMMASBTWNFIELDS,(UINT)BM_SETCHECK,AddCommas,(LPARAM)0L); 
			SendDlgItemMessage (hWndDlg,IDC_QUOTESAROUNDFIELDS,(UINT)BM_SETCHECK,AddQuotes,(LPARAM)0L); 
			SendDlgItemMessage(hWndDlg, IDC_TABBTWNFIELDS, (UINT)BM_SETCHECK, AddTabs, (LPARAM)0L);
			SendDlgItemMessage(hWndDlg, IDC_ADDNEWLINE, (UINT)BM_SETCHECK, AddNewLine, (LPARAM)0L);
		}
    	if (!hFLDB)
    	{
	        SendDlgItemMessage (hWndDlg,IDC_FIELDS,LB_ADDSTRING,0,(LPARAM)((LPSTR) "[%UDI]"));
	        SendDlgItemMessage (hWndDlg,IDC_FIELDS,LB_ADDSTRING,0,(LPARAM)((LPSTR) "[%INT_REFNO]"));
	        SendDlgItemMessage (hWndDlg,IDC_FIELDS,LB_ADDSTRING,0,(LPARAM)((LPSTR) "[%MSLINK]"));
    		break;
    	}  
    	if (hFLDB==(HANDLE)2)
		{   
		    int     TabStops[2]={150,500}; 
		    BOOL	Expand=!SendDlgItemMessage (hWndDlg,IDC_RAWMODE,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L);

			SendDlgItemMessage (hWndDlg,IDC_FIELDS,LB_SETTABSTOPS,2,(LPARAM)&TabStops);
		    
		    ListGlobals (hWndDlg,IDC_FIELDS,Expand);
    		break;
    	}  
    	if (hFLDB == (HANDLE)1)
    	{   
			VARPNT	VarPtr;
    		short	i;
			LPSHORT	nDLTvar;
			LPHANDLE	DLTVar; 
			LPSHORT	DLTStart,DLTLen,DLTType;
			
			if (hFieldsDLT)
			{
				nDLTvar = (LPSHORT)GlobalLock (hFieldsDLT);
				DLTVar = (LPHANDLE)(nDLTvar + 1);
				DLTStart = (LPSHORT)(DLTVar + MAXDLTVAR);
				DLTLen = (LPSHORT)(DLTStart + MAXDLTVAR);  
				DLTType = (LPSHORT)(DLTLen + MAXDLTVAR);  
	    		for (i=0;i<*nDLTvar;i++)
	    		{
					VarPtr = (VARPNT)GlobalLock (DLTVar[i]);
			        sprintf (str,"%s",VarPtr->Name);
			        SendDlgItemMessage (hWndDlg,IDC_FIELDS,LB_ADDSTRING,0,(LPARAM)((LPSTR)str));
					GlobalUnlock (DLTVar[i]);
				}
				GlobalUnlock (hFieldsDLT);
			}
			break;
    	} 
    	EnableWindow (GetDlgItem(hWndDlg,IDC_SHOWDESCRIPT),TRUE);
    	EnableWindow (GetDlgItem(hWndDlg,IDC_SHOWDATA),TRUE);

  	case GSSI_REINITDIALOG:
		SendDlgItemMessage (hWndDlg,IDC_FIELDS,LB_RESETCONTENT,0,0);
		SQLPtr = (LPOPENSQLDATA)GlobalLock (hFLDB);
		FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle);
		lpFieldInfo = &FilePtr->FldInfo; 

		for (i=0;i<FilePtr->NumFields;i++,lpFieldInfo++) 
		{   
			*str = 0;
			if (ShowDesc)
			    CreateGMTextHeader (lpFieldInfo, str);
	        	//sprintf (str,"%s - %i",lpFieldInfo->name,lpFieldInfo->length);
	        else
	        	sprintf (str,"%s",lpFieldInfo->name); 
	        SendDlgItemMessage (hWndDlg,IDC_FIELDS,LB_ADDSTRING,0,(LPARAM)((LPSTR) str));
		}           
		GlobalUnlock (SQLPtr->OFHandle);
		GlobalUnlock (hFLDB);
		if (FieldsDisplayOpt == -1 && FilePathHandle)
		{
			LPFILEPATH	FilePathPtr = (LPFILEPATH)GlobalLock (FilePathHandle);
			LPHANDLE	lpFileHandle; 
            BOOL		FoundIt = FALSE;
		    short		isql = FilePathPtr->NumFiles;	

			while (isql--)
			{
				lpFileHandle = &FilePathPtr->FileHandle;  
				lpFileHandle += isql;
				if (*lpFileHandle)
				{   
					SQLPtr = (LPOPENSQLDATA)GlobalLock(*lpFileHandle);
					FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
					if (!_fstricmp ("shp",SQLPtr->IDName))
					{   
						FoundIt = TRUE;
						lpFieldInfo = &FilePtr->FldInfo; 
						for (i=0;i<FilePtr->NumFields;i++,lpFieldInfo++) 
						{   
							if (ShowDesc)
					        	sprintf (str,"SHP.%s - %i",lpFieldInfo->name,lpFieldInfo->length);
					        else
					        	sprintf (str,"SHP.%s",lpFieldInfo->name); 
					        SendDlgItemMessage (hWndDlg,IDC_FIELDS,LB_ADDSTRING,0,(LPARAM)((LPSTR) str));
						} 
					} 
               		GlobalUnlock (SQLPtr->OFHandle);
               		GlobalUnlock (*lpFileHandle);
               	}
				if (FoundIt)
				{
					ShowDesc = FALSE;
					break;
				}
            }
            GlobalUnlock (FilePathHandle);
         }
		ShowDesc = FALSE;
		break;

    case WM_CLOSE:
		 AddSpaces = SendDlgItemMessage (hWndDlg,IDC_SPACES,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L);     
		 AddBrackets = SendDlgItemMessage (hWndDlg,IDC_BRACKETFIELDS,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L);     
		 AddCommas = SendDlgItemMessage (hWndDlg,IDC_COMMASBTWNFIELDS,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L);     
		 AddQuotes = SendDlgItemMessage (hWndDlg,IDC_QUOTESAROUNDFIELDS,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L);     
		 AddTabs = SendDlgItemMessage(hWndDlg, IDC_TABBTWNFIELDS, (UINT)BM_GETCHECK, (WPARAM)0, (LPARAM)0L);
		 AddNewLine = SendDlgItemMessage(hWndDlg, IDC_ADDNEWLINE, (UINT)BM_GETCHECK, (WPARAM)0, (LPARAM)0L);
		 DestroyWindow(hWndDlg);
         break;  

    case WM_DESTROY:
		 if (AutoCloseFieldDB && hFLDB > (HANDLE)2)
		 	CloseDataFile (TRUE, &hFLDB);  
		 hFLDB = 0;
	     hWndFieldList = 0;
		 break;

    case WM_COMMAND:
#if WIN32
        switch(LOWORD(wParam))
#else
        switch(wParam)
#endif
           { 
			  case IDOK:
				  {
					  HANDLE	hText;
					  LPSTR		pText;

					  OpenClipboard (hWndDlg);
					  hText  = GetClipboardData(CF_TEXT);
					  if (hText)
					  {
							IgnoreLock = TRUE;
						    pText = GlobalLock (hText);
							strcpy (GetFieldOutput,pText);
							GlobalUnlock (hText);
							IgnoreLock = FALSE;
					  }
					  CloseClipboard ();
					  EndDialog (hWndDlg,1);
				  }
				  break;
			  case IDCANCEL:
				  EndDialog (hWndDlg,0);
				  break;
           	  case IDC_SHOWDESCRIPT:
				ShowDesc = TRUE;
           	    PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L);
				break;
			
			  case IDC_SHOWDATA:
				  {
					  HANDLE	hStr = GSSiGlobAlloc (0,GMEM_MOVEABLE,4096*2);
					  LPSTR pstr=GlobalLock (hStr);

					if (FirstShow)
					{
						RECT	Rect;
						int     TabStops[2]={150,500}; 

						FirstShow = FALSE;
						GetWindowRect (hWndDlg,&Rect);
				   		MoveWindow(hWndDlg, Rect.left,Rect.top, 2*(Rect.right-Rect.left), Rect.bottom-Rect.top, TRUE);
						GetWindowRect (GetDlgItem(hWndDlg,IDC_FIELDS),&Rect);
						ScreenRectToClientRect (hWndDlg,&Rect);
				   		MoveWindow(GetDlgItem(hWndDlg,IDC_FIELDS), Rect.left,Rect.top, 2*(Rect.right-Rect.left), Rect.bottom-Rect.top, TRUE);
         				SendDlgItemMessage (hWndDlg,IDC_FIELDS,LB_SETTABSTOPS,2,(LPARAM)&TabStops);
					}
				    FetchDBRec (hFLDB);
					SQLPtr = (LPOPENSQLDATA)GlobalLock (hFLDB);
					FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle);
					lpFieldInfo = &FilePtr->FldInfo; 
			        SendDlgItemMessage (hWndDlg,IDC_FIELDS,LB_RESETCONTENT,0,0);

					for (i=0;i<FilePtr->NumFields;i++,lpFieldInfo++) 
					{   
        				sprintf (pstr,"%s\t[%s]",lpFieldInfo->name,lpFieldInfo->name); 
						ExpandText (pstr);
						SendDlgItemMessage (hWndDlg,IDC_FIELDS,LB_ADDSTRING,0,(LPARAM)((LPSTR) pstr));
					}   
					GSSiGlobUlFree (&hStr);
					GlobalUnlock (SQLPtr->OFHandle);
					GlobalUnlock (hFLDB);
				  }
				  break;

			  case IDC_RAWMODE:	
		    	if (hFLDB==(HANDLE)2)
				{   
				    int     TabStops[2]={150,500}; 
				    BOOL	Expand=!SendDlgItemMessage (hWndDlg,IDC_RAWMODE,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L);

			        SendDlgItemMessage (hWndDlg,IDC_FIELDS,LB_RESETCONTENT,0,0);
		         	SendDlgItemMessage (hWndDlg,IDC_FIELDS,LB_SETTABSTOPS,2,(LPARAM)&TabStops);
				    
				    ListGlobals (hWndDlg,IDC_FIELDS,Expand);
		    	}  
        		break;   
        		
        	  case IDC_SPACES:  
        	  case IDC_TABBTWNFIELDS:
        	  case IDC_COMMASBTWNFIELDS:
        	  case IDC_BRACKETFIELDS:
			  case IDC_QUOTESAROUNDFIELDS:
			  case IDC_ADDNEWLINE:
				  goto OutputFields;
        	  	
	          case IDC_FIELDS:
	          switch(HIWORD(wParam))
	          {
                case LBN_SELCHANGE:
     OutputFields:
	            {
	            	HANDLE	hText;  
	            	HANDLE	hFields;
	            	LPINT	lpItems;
					LPSTR	pTab;
	            	int	nItems; 
	            	LPSTR	pText; 
	            	BOOL	First,KeepMemLengthSave=KeepMemLength; 
	            	char	quote[2]="";
					char	fieldType='C';
					short	fieldLen;
	            	
	            	
	            	KeepMemLength = TRUE;  
	            	hText = GSSiGlobAlloc ( 261,GHND,USHRT_MAX);
           	        KeepMemLength = KeepMemLengthSave;
	                nItems=SendDlgItemMessage(hWndDlg,IDC_FIELDS,LB_GETSELCOUNT,0,0);  
	                if (!nItems)
	                	break;
	                hFields = GSSiGlobAlloc ( 262,GHND,nItems*4);
	                lpItems = (LPINT)GlobalLock(hFields);
	                SendDlgItemMessage(hWndDlg,IDC_FIELDS,LB_GETSELITEMS,nItems,(LPARAM)lpItems); 
					pText = GlobalLock (hText);  
	            	if (hFldAppendWnd)
	            	{   
	                	SendDlgItemMessage(hWndDlg,IDC_FIELDS,LB_GETTEXT,*lpItems,(LPARAM)pText); 
	            		SetWindowText (hFldAppendWnd,pText);
						GSSiGlobUlFree (&hText);
	            		break;
	            	}
	            	OpenClipboard (hWndDlg);
					EmptyClipboard();
					First = TRUE;
					while (nItems--)
					{   
						if (First)
							First=FALSE;
						else
						{   
							if (SendDlgItemMessage(hWndDlg, IDC_TABBTWNFIELDS, (UINT)BM_GETCHECK, (WPARAM)0, (LPARAM)0L))
							{
								_fstrcat(pText, "$CHR(9)");
								pText += 7;
							}
							if (SendDlgItemMessage(hWndDlg, IDC_COMMASBTWNFIELDS, (UINT)BM_GETCHECK, (WPARAM)0, (LPARAM)0L))
								_fstrcat (pText++,",");
		                    if (SendDlgItemMessage (hWndDlg,IDC_SPACES,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L))
								_fstrcat (pText++," ");
							if (SendDlgItemMessage(hWndDlg, IDC_ADDNEWLINE, (UINT)BM_GETCHECK, (WPARAM)0, (LPARAM)0L))
							{
								_fstrcat(pText, "\r\n");
								pText += 2;
							}
						}
						SendDlgItemMessage(hWndDlg, IDC_FIELDS, LB_GETTEXT, *lpItems++, (LPARAM)str);
						if (hFLDB == (HANDLE)1)
							fieldType = 'C';
						else
							GetFieldInfoFromName(hFLDB, str, &fieldType, &fieldLen);
						if (SendDlgItemMessage(hWndDlg, IDC_QUOTESAROUNDFIELDS, (UINT)BM_GETCHECK, (WPARAM)0, (LPARAM)0L) && fieldType == 'C')
							_fstrcpy(quote, "'");
						else
							*quote = 0;
						if ((pTab = strchr(str, '\t')))
							*pTab = 0;
						if (SendDlgItemMessage (hWndDlg,IDC_BRACKETFIELDS,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L))	 
							sprintf (pText,"%s[%s]%s",quote,str,quote); 
						else
							sprintf (pText,"%s%s%s",quote,str,quote);
	                	pText = _fstrchr (pText,0);
					} 
					GSSiGlobUlFree (&hFields); 
					GlobalUnlock (hText);
					#if CHECKMEM
						GSSiRemoveMem (hText);
					#endif
					SetClipboardData(CF_TEXT, hText);
			        CloseClipboard();
	             }
                 break;
           	 }
             break; 
           }
         break;
    default:
{
#if ENABLETRACE
GSSiExitProg (620);
#endif
        return FALSE;
}
   } 
{
#if ENABLETRACE
GSSiExitProg (620);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
}

 

