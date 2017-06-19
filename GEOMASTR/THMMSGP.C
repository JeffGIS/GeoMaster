#include "graphint.h"
#include "extrndb.h"

static	COLORREF	SaveClassColor[MAX_THEME_CLASSES]; 
static	DLGPROC lpfnSELECTVALUESMsgProc;
static	BOOL	SaveColorsInUse=FALSE;
static	COLORREF	SaveBKGColor;     
static	char	ViewportStatusMessages[MAX_VIEWPORTS][128];

#include "gmextern.h"

#define MAXHOTSPOTDIMENSION	500


/*					ThemeHighlightKey.Class = WantClass;
					ThemeHighlightKey.Refno = LONG_MIN;
					if (BT_FIND (CurTheme->hHighlightFile,(LPSTR)&ThemeHighlightKey,BT_FIRST,BT_GE,(LPSTR)&ThemeHighlightData))
						ThemeHighlightKey.Class=-10;
					while (ContinueProcessing && ThemeHighlightKey.Class == WantClass)
					{   
							_fmemset (&PickList[0],0,sizeof(PICKDATA));
						    PickList[0].ViewID = CurView->ID;
							PickList[0].FileNum = ThemeHighlightData.FileNum;  
							PickList[0].SubFile = ThemeHighlightData.SubFile;
							PickList[0].FileInIndex = ThemeHighlightData.FileInIndex;
							PickList[0].Segment = ThemeHighlightData.Segment;
							PickList[0].Refno = ThemeHighlightKey.Refno;
							PickList[0].Desc = 0;
							PickList[0].Offset = ThemeHighlightData.Offset; 
							PickList[0].Element = ThemeHighlightData.Element; 
							PickList[0].Length = ThemeHighlightData.Length; 
							PickList[0].Area = ThemeHighlightData.Area; 
							SaveNThemes = CurView->NumThemes;
							CurView->NumThemes = 0;
							ProcessPickedItem (0,FALSE); 
							PickList[0].HiPrecis = PolyIsHiPrecis;
							CurTheme = SaveTheme;
							CurView->NumThemes = SaveNThemes; 
							PickList[0].Type = 2; 
							if (CurrentType == GF_LINE || CurrentType == GF_POLYLINE) 
							{
								PickList[0].Type = 2; 
								if (GetPolyPoints ((LPPICKDATAHEADER)&PickList[0],FALSE,&nPnts,&hPoly))  
								{   
									HPDPOINT lpPoints=(HPDPOINT)GlobalLock (hPoly);
									
									PickList[0].Length = GetPolyLengthD (lpPoints,nPnts);
		                            GSSiGlobUlFree (&hPoly);
								}
							}
							else if (CurrentType == GF_AREA) 
							{
								PickList[0].Type = 3; 
								if (GetPolyPoints ((LPPICKDATAHEADER)&PickList[0],FALSE,&nPnts,&hPoly))  
								{   
									HPDPOINT lpPoints=(HPDPOINT)GlobalLock (hPoly);
									
									PickList[0].Area = ComputeProjectedAreaAreaD (lpPoints,nPnts,&PickList[0].Length);
		                            GSSiGlobUlFree (&hPoly);
								}
							}
							else if (CurrentType == GF_POINT)
								PickList[0].Type = 1; 
							else if (CurrentType == GF_TEXT)
								PickList[0].Type = 4;  
							PickList[0].Desc = CurrentDesc;   
							PickList[0].MSLink = CurMSLink;
							_fstrcpy (PickList[0].Prefix,CurrentTAG);
							_fstrcpy (PickList[0].UDI,CurrentUDI);
			     			AddToHighlightList (ThemeHighlightKey.Refno,&PickList[0],TRUE); 
				     	}
						if (BT_FIND (CurTheme->hHighlightFile,(LPSTR)&ThemeHighlightKey,BT_NEXT,BT_ANY,(LPSTR)&ThemeHighlightData))
							ThemeHighlightKey.Class=-10;
	                    PctBox (GetDlgItem(hWndDlg,IDC_STATUS), TotItems, CurLoc++,0);
					}
				}*/ 

BOOL SetThemeQuan (LPTHEME pTheme,LPSTR FileName)
{
	THEMEHIGHLIGHTKEY	ThemeHighlightKey;
	THEMEHIGHLIGHTDATA	ThemeHighlightData;
	BOOL	rtn=FALSE;
	char	DefStr[]="ClassNum(B2),ClassID(C64),ClassColor(B4),ClassCount(B4),Type(C1),Acres(R8),SQFeet(R8),Feet(R8),Miles(R8)";
	HANDLE	hDB, hPoly;
    LPGWDHEADER lpGWDHead;
	LPTHEME	SaveTheme = CurTheme;
	short	i;
	int	nPnts;
   	HCURSOR	hcurSave; 
	double	Acres, Feet;

	if (!pTheme)
		return FALSE;
	if (!CreateGWDDatabase (FileName,1,FALSE,0,1,DefStr))
		return FALSE;
	CurTheme = pTheme;
	if (!OpenThemeHighlightFile (BT_READ))
		goto Exit;
	
	hDB = OpenGWDatabase (FileName,BT_WRITE);
	if (!hDB)
		return FALSE;
	hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT)); 
    lpGWDHead = (LPGWDHEADER)GlobalLock (hDB);  
	for (i=0;i<pTheme->NumClass;i++)
	{
		double	TotLength=0;
		double	TotArea=0;
		short	ClassPlus1 = i+1;

		SetFieldValFromCharAndName(lpGWDHead, "ClassNum", (LPSTR)&ClassPlus1, TRUE, TRUE);
		SetFieldValFromCharAndName(lpGWDHead, "ClassID", (LPSTR)&pTheme->ClassBM[i], TRUE, TRUE);
		SetFieldValFromCharAndName(lpGWDHead, "ClassColor", (LPSTR)&pTheme->ClassColor[i], TRUE, TRUE);
		SetFieldValFromCharAndName(lpGWDHead, "ClassCount", (LPSTR)&pTheme->ClassCount[i], TRUE, TRUE);
		ThemeHighlightKey.Class = i;
		ThemeHighlightKey.Refno = LONG_MIN;
		if (BT_FIND (CurTheme->hHighlightFile,(LPSTR)&ThemeHighlightKey,BT_FIRST,BT_GE,(LPSTR)&ThemeHighlightData))
			ThemeHighlightKey.Class=-10;
		while (ThemeHighlightKey.Class == i)
		{ 
			_fmemset (&PickList[0],0,sizeof(PICKDATA));
			PickList[0].ViewID = pTheme->TargetViewport;
			PickList[0].ConfigID = CurrentConfig;
			PickList[0].FileNum = ThemeHighlightData.FileNum;  
			PickList[0].SubFile = ThemeHighlightData.SubFile;
			PickList[0].FileInIndex = ThemeHighlightData.FileInIndex;
			PickList[0].Segment = ThemeHighlightData.Segment;
			PickList[0].Refno = ThemeHighlightKey.Refno;
			PickList[0].Desc = 0;
			PickList[0].Offset = ThemeHighlightData.Offset; 
			PickList[0].Element = ThemeHighlightData.Element; 
			if (ThemeHighlightData.Type == GF_LINE)
			{
				if (GetPolyPoints ((LPPICKDATAHEADER)&PickList[0],FALSE,&nPnts,&hPoly))  
				{   
					HPDPOINT lpPoints=(HPDPOINT)GlobalLock (hPoly);
					
					ThemeHighlightData.Length = GetPolyLengthD (lpPoints,nPnts);
		            GSSiGlobUlFree (&hPoly);
				}
			}
			else if (ThemeHighlightData.Type == GF_AREA) 
			{
				PickList[0].Type = 3; 
				if (GetPolyPoints ((LPPICKDATAHEADER)&PickList[0],FALSE,&nPnts,&hPoly))  
				{   
					HPDPOINT lpPoints=(HPDPOINT)GlobalLock (hPoly);
					
					ThemeHighlightData.Area = ComputeProjectedAreaAreaD (lpPoints,nPnts,&PickList[0].Length);
		            GSSiGlobUlFree (&hPoly);
				}
			}
			TotLength += ThemeHighlightData.Length; 
			TotArea += ThemeHighlightData.Area; 
			if (BT_FIND (CurTheme->hHighlightFile,(LPSTR)&ThemeHighlightKey,BT_NEXT,BT_ANY,(LPSTR)&ThemeHighlightData))
				ThemeHighlightKey.Class=-10;
		}
		Acres = ConvertArea (TotArea,6);
		Feet = ConvertDist (TotLength,1);
		SetFieldValFromCharAndName(lpGWDHead, "Feet", (LPSTR)&TotLength, TRUE, TRUE);
		SetFieldValFromCharAndName(lpGWDHead, "Acres", (LPSTR)&Acres, TRUE, TRUE);
		GWDAddRecord (lpGWDHead,0,0);
	}
	GlobalUnlock (hDB);
	CloseGWDatabase (hDB); 
	CloseThemeHighlightFile ();
	GSSiSetCursor(hcurSave);
	rtn = TRUE;
Exit:
	CurTheme = SaveTheme;
	return rtn;
}

int ThemeCheckOffsetAreas (void)
{
	int	rtn=1;
	POINT	Point;
	COLORREF	Color, AreaColor=0;

	if (!CurView->PassID && !Pick)
		return rtn;
	switch (CurrentType)
	{
		case GF_POINT:
			SaveDC (CurView->hDC);
	        SetDisplayMode (CurView->hDC,GF_SCREENMODE);
			Point = BasePtToScreenPt (&CurPointLocD);
			if (PtInRect (&CurView->ScreenRect,Point))
				Color = GetPixel (CurView->hDC,Point.x,Point.y);
			else
				Color = 1;
			if (Color == AreaColor || Color == CLR_INVALID)
				rtn = 1;
			else
				rtn = 0;
			RestoreDC (CurView->hDC,-1);
			break;
		default:
			break;
	}

	return rtn;
}
 
BOOL ThemeCreateOffsetAreas (double Offset)
{
	if (CurTheme->FidAreas == HFILE_ERROR || CurTheme->Pass)
		return FALSE; 
	BigWrite (CurTheme->FidAreas,(HPSTR)&CurrentRefno,4,-1);
	BigWrite (CurTheme->FidAreas,(HPSTR)&CurrentType,2,-1);
	BigWrite (CurTheme->FidAreas,(HPSTR)&CurrentDesc,2,-1);
	BigWrite (CurTheme->FidAreas,(HPSTR)&Offset,8,-1);
	BigWrite (CurTheme->FidAreas,(HPSTR)&nPoly,4,-1);
	if (nPoly > 1)
	{
		LPINT	pNumPoints = (LPINT)GlobalLock (hPolyPartLen);
	
		BigWrite (CurTheme->FidAreas,(HPSTR)pNumPoints,(nPoly+1)*sizeof(int),-1);
		GlobalUnlock (hPolyPartLen);
	}
	BigWrite (CurTheme->FidAreas,(HPSTR)&nCurPoints,4,-1);
	BigWrite (CurTheme->FidAreas,(HPSTR)lpDCurPoints,nCurPoints * (long)sizeof(DPOINT),-1);  
	CurTheme->NumAreas++;
	return TRUE;
}

BOOL ThemeDisplayOffsetAreas (void)
{
	if (!CurTheme->Pass)
	{   
	    UINT		iarea;
		HANDLE		hPoints=0,hPolyPartLen=0;
	    HPDPOINT	lpPoints;
		long		NumPoints,Refno;
		short		Type, Desc;
		UINT		ip;
		LPINT		pNumPoints;
		int			np, iclass;
		double		Offset, ValD,ClassMin,ClassMax;
		HPEN		OldPen,hPen;
		int			width,ipass,npass=1;
		COLORREF	color;
		RECT		SaveRect;
		static		BOOL		checkIn = TRUE;
			
		SaveDC (CurView->hDC);
	    GSSiDeleteObject(&CurView->hRgn);
        CurView->hRgn = CreateVPRgn(FALSE,FALSE);
        SelectClipRgn (CurView->hDC,CurView->hRgn);
        GSSiDeleteObject(&CurView->hRgn);
        SetDisplayMode (CurView->hDC, GF_TEXTMODE);
	    CurTheme->FidAreas = GSSiOpenFile (CurTheme->ScatterFile,0,OF_READ);  
	    if (CurTheme->FidAreas != HFILE_ERROR)
		{
			color = 0;
			for (iclass=CurTheme->NumClass-1;iclass>=0;iclass--)
			{	
				ClassMin = CurTheme->ClassMin[iclass];
										
				color = CurTheme->ClassColor[iclass];
				for (ipass=0;ipass<npass;ipass++)
				{
					GSSillseek (CurTheme->FidAreas,0,0);
					PIASizeFactor = 1;
					for (iarea=0;iarea<CurTheme->NumAreas;iarea++)
					{ 
						BigRead (CurTheme->FidAreas,(HPSTR)&Refno,4);
						BigRead (CurTheme->FidAreas,(HPSTR)&Type,2);  
						BigRead (CurTheme->FidAreas,(HPSTR)&Desc,2);  
						BigRead (CurTheme->FidAreas,(HPSTR)&Offset,8);  
						BigRead (CurTheme->FidAreas,(HPSTR)&nPoly,4); 
						Offset *= ClassMin;
						if (nPoly > 1)
						{
							hPolyPartLen = GSSiGlobAlloc (0,GMEM_MOVEABLE,(nPoly+1)*sizeof(int));
							pNumPoints = (LPINT)GlobalLock (hPolyPartLen);
						
							BigRead (CurTheme->FidAreas,(HPSTR)pNumPoints,(nPoly+1)*sizeof(int));
							GlobalUnlock (hPolyPartLen);
						}
						BigRead (CurTheme->FidAreas,(HPSTR)&NumPoints,4);  
						hPoints = GSSiGlobAlloc (0,GMEM_MOVEABLE,NumPoints * sizeof(DPOINT));
						lpPoints = (HPDPOINT)GlobalLock (hPoints);
						BigRead (CurTheme->FidAreas,(HPSTR)lpPoints,NumPoints * sizeof(DPOINT)); 
						width = 2*IDNINT (Offset/CurView->BaseUnitsPerPixel);
						ValD = Round(Offset,CurTheme->RoundTo);
						if (!ipass)
						{
							if (Type == GF_AREA)
							{   
								HPPOINT	lpPoints32;
								HANDLE	Handle = GSSiGlobAlloc ( 768,GMEM_MOVEABLE,(long)NumPoints * (long)sizeof(POINT));
								UINT	i;
								HBRUSH	hBrush = CreateSolidBrush (color);
								HBRUSH	hBrushOld = SelectObject (CurView->hDC,hBrush);

								lpPoints32 = (HPPOINT)GlobalLock (Handle);
								for (i=0;i<NumPoints;i++)
									lpPoints32[i] = BasePtToWinPt (&lpPoints[i]);   
								Polygon (CurView->hDC,lpPoints32,NumPoints); //lpPoints32[3]
								GSSiGlobUlFree (&Handle);
								SelectObject (CurView->hDC,hBrushOld);
								GSSiDeleteObject (&hBrush);
							}

							if (CurTheme->FlatEndOffsetLine)
							{
								LOGBRUSH	lb;
								long	rtn; 
								
								lb.lbStyle = BS_SOLID;
								lb.lbColor = color;
								lb.lbHatch = 0;
								hPen = ExtCreatePen (PS_GEOMETRIC|PS_SOLID|PS_ENDCAP_FLAT|PS_JOIN_BEVEL,width,&lb,0,0);
							}
							else
								hPen = CreatePen (PS_SOLID,(int)width,color);
						}
						else
							hPen = CreatePen (PS_SOLID,width-2,RGB(255,255,255));
						OldPen = SelectObject (CurView->hDC,hPen);
						if (nPoly <= 1)
						{
							np = NumPoints;
							pNumPoints = &np;
							nPoly = 1;
						}
						else
							pNumPoints = (LPINT)GlobalLock (hPolyPartLen);
	//					SaveRect = CurView->DrawRect;
	//					InflateRect (&CurView->DrawRect,width,width);
						for (ip=0;ip<nPoly;ip++,pNumPoints++)
						{   
							HPPOINT	lpPoints32;
							HANDLE	Handle = GSSiGlobAlloc ( 768,GMEM_MOVEABLE,((long)*pNumPoints+16L) * (long)sizeof(POINT));
							UINT	i, nPnts2=1;
							BOOL	LastIn, ThisIn;
							POINT	LastPoint;

							lpPoints32 = (HPPOINT)GlobalLock (Handle);
							lpPoints32[0] = BasePtToWinPt (lpPoints++);
							if (checkIn)
								LastIn = PtInDrawRect (lpPoints32[0]);
							else
								LastIn = TRUE;
							LastPoint = lpPoints32[0];
							for (i=1;i<*pNumPoints;i++,lpPoints++)
							{
								lpPoints32[nPnts2] = BasePtToWinPt (lpPoints); 
								if (nPnts2 > 0 && (abs(lpPoints32[nPnts2].x - lpPoints32[nPnts2-1].x) + abs(lpPoints32[nPnts2].y - lpPoints32[nPnts2-1].y) > 5))
								{
									if (checkIn)
										ThisIn = PtInDrawRect (lpPoints32[nPnts2]);
									else
										ThisIn = TRUE;
									if (LastIn && !ThisIn)
									{
										Polyline (CurView->hDC,lpPoints32,nPnts2+1);
										LastPoint = lpPoints32[nPnts2];
										lpPoints32[0] = LastPoint;
										nPnts2 = 1;
									}
									else if (!LastIn && ThisIn)
									{
										lpPoints32[1] = lpPoints32[nPnts2];
										lpPoints32[0] = LastPoint;
										nPnts2 = 2;
									}
									else if (LastIn && ThisIn)
										nPnts2++;
									else
									{
										RECT	rect, outRect;
										POINT	savePoint  = lpPoints32[nPnts2];

										RectInit (&rect);
										AddPointToRect (LastPoint,&rect);
										AddPointToRect (savePoint,&rect);
										if (IntersectRect (&outRect,&CurView->DrawRect,&rect))
										{
											lpPoints32[0] = LastPoint;
											lpPoints32[1] = savePoint;
											Polyline (CurView->hDC,lpPoints32,2);
										}
										LastPoint = savePoint;
										lpPoints32[0] = LastPoint;
										nPnts2 = 1;
									}
									LastIn = ThisIn;
								}
								else if (!nPnts2)
								{
									if (checkIn)
										LastIn = PtInDrawRect (lpPoints32[nPnts2]);
									else
										LastIn = TRUE;
									if (LastIn)
										nPnts2++;
									else
										LastPoint = lpPoints32[nPnts2];
								}
							}
							if (nPnts2 > 1)
								Polyline (CurView->hDC,lpPoints32,nPnts2); //lpPoints32[3]
							GSSiGlobUlFree (&Handle);
							if (ip)
								lpPoints++;
						}
	//					CurView->DrawRect = SaveRect;
						GSSiGlobUlFree (&hPoints);
						GSSiGlobUlFree (&hPolyPartLen);
						SelectObject (CurView->hDC,OldPen);
						GSSiDeleteObject (&hPen);
					}
				}
			}
		    GSSiClose (CurTheme->FidAreas);
		    CurTheme->FidAreas = HFILE_ERROR;
		    GSSiRemoveAndClear (CurTheme->ScatterFile);
		}
		RestoreDC (CurView->hDC,-1);
	}
	return TRUE;
}

BOOL GetDMSMajorMinorInc (double DegDifx,LPDOUBLE pMajor, LPDOUBLE pMinor, LPSHORT pFormat)
#if ENABLETRACE
{GSSiEnterProg (1281);
#endif
{   
	double	MajorInc, MinorInc, SecDif,MIN=(double)1/60,SEC=MIN/60;
	short	DegDif, MinDif, fmt=0;
	
	GetDMS (DegDifx,&DegDif,&MinDif,&SecDif); 
	if (DegDif>10)
	{
		MajorInc = 2;
		MinorInc = 1;
	}
	else if (DegDif>2)
	{
		MajorInc = 1;
		MinorInc = MIN*10;
	}
	else
	{   
		fmt = 1;
		MinDif += DegDif*60;
		if (MinDif > 40)
    	{
    		MajorInc = MIN*20;
    		MinorInc = MIN*2;
    	}
		else if (MinDif > 20)
    	{
    		MajorInc = MIN*10;
    		MinorInc = MIN;
    	}
    	else if (MinDif > 10)
    	{
    		MajorInc = MIN*5;
    		MinorInc = MIN;
    	}
    	else if (MinDif > 5)
    	{
    		MajorInc = MIN * 2;
    		MinorInc = SEC * 10;
    	}
    	else if (MinDif > 2)
    	{
    		MajorInc = MIN;
    		MinorInc = SEC * 10;
    	}
    	else 
    	{   
    		fmt = 2;
    		SecDif += 60 * MinDif;
    		if (SecDif > 100)
    		{
	    		MajorInc = SEC * 30;
	    		MinorInc = SEC * 10;
    		}
    		else if (SecDif > 40)
    		{
	    		MajorInc = SEC * 20;
	    		MinorInc = SEC * 10;
    		}
    		else if (SecDif > 20)
    		{
	    		MajorInc = SEC * 10;
	    		MinorInc = SEC * 1;
    		}
    		else if (SecDif > 10)
    		{
	    		MajorInc = SEC * 5;
	    		MinorInc = SEC * 1;
    		}
    		else if (SecDif > 5)
    		{
	    		MajorInc = SEC * 2;
	    		MinorInc = SEC * 1;
    		}
    		else if (SecDif > 2)
    		{
	    		MajorInc = SEC;
	    		MinorInc = SEC/10;
	    	}
	    	else
	    	{
	    		fmt = 3;
	    		MajorInc = SEC / 2;
	    		MinorInc = SEC / 10; 
	    	}
	    }
	}  
	*pMajor = MajorInc;
	*pMinor = MinorInc; 
	*pFormat = fmt;
{
#if ENABLETRACE
GSSiExitProg (1281);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}
	
 
void DisplayCoordGridThemeLegend(short From)
#if ENABLETRACE
{GSSiEnterProg (1282);
#endif
{    
	short	CurViewID = CurView->ID, reps, h,w;    
	double	BaseDistPerPixel, MaxWidth, BarWidth, step;
	POINT	Points[5], MinorPoints[2];  
	long	IntVal;  
	char	txt[128], str[16], fmt[16]=" %.0f mile "; 
	HPEN	OldPen, LinePen, RedPen, WhitePen;    
	BOOL	DoMinorTics = GetGlobalBVal2 ("[%MINORTICS]",FALSE);
	
//	SetCurView (pViewports[CurTheme->TargetViewport-1]); //tempdebu
	SetViewport (CurTheme->TargetViewport);   
	SaveDC (CurView->hDC);
	SetDisplayMode (CurView->hDC, GF_SCREENMODE);
    GSSiDeleteObject(&CurView->hRgn);
	CurView->hRgn = CreateVPRgn (2,FALSE);
  	SelectClipRgn (CurView->hDC,CurView->hRgn);
  	GSSiDeleteObject(&CurView->hRgn);
    SetTextColor (CurView->hDC,0);  
	switch (CurTheme->GridID)
	{
	case 0: //latlon
    if (CurView->MarginPan && !Printing)
	{   
		POINT	Point;
		RECT	Rect;
		short	symnum, width, height;
	    LPSYMBOL    CurSymbol;
	    HANDLE	hSymbol;
	    BOOL	SaveHVFC=HaveVarFillColor;
	    HBRUSH	hOldBrush; 
	    double	Rot=0, QuaterPY=HALFPI/2.0;
		
		
	    width = CurView->Rect.right - CurView->ScreenRect.right -2;
	    height = CurView->Rect.bottom - CurView->ScreenRect.bottom -2;
		symnum = GetDictSymbolNumber ("ARROW1");
		hSymbol = GetDictSymDesc (symnum,0);
		hOldBrush = SelectObject (CurView->hDC,GetStockObject(WHITE_BRUSH));
	    HaveVarFillColor = TRUE; 
	    GlobalColors[0]=RGB(255,255,255);
	    Point.x=(CurView->Rect.right + CurView->ScreenRect.right)/2;
	    Point.y=(CurView->Rect.top + CurView->Rect.bottom)/2;  
		DisplayPointSymbol (hSymbol,CurView->hDC, -height, width,Rot, &Point,0,FALSE,0,0,FALSE,FALSE,0,0);  
	    Point.y=(CurView->Rect.top + CurView->ScreenRect.top)/2;  
		Rot = QuaterPY;					
		DisplayPointSymbol (hSymbol,CurView->hDC, -height, width,Rot, &Point,0,FALSE,0,0,FALSE,FALSE,0,0);  
	    Point.x=(CurView->Rect.left + CurView->ScreenRect.right)/2;
		Rot += QuaterPY;					
		DisplayPointSymbol (hSymbol,CurView->hDC, -height, width,Rot, &Point,0,FALSE,0,0,FALSE,FALSE,0,0);  
	    Point.x=(CurView->Rect.left + CurView->ScreenRect.left)/2;
		Rot += QuaterPY;					
		DisplayPointSymbol (hSymbol,CurView->hDC, -height, width,Rot, &Point,0,FALSE,0,0,FALSE,FALSE,0,0);  
	    Point.y=(CurView->Rect.top + CurView->Rect.bottom)/2;  
		Rot += QuaterPY;					
		DisplayPointSymbol (hSymbol,CurView->hDC, -height, width,Rot, &Point,0,FALSE,0,0,FALSE,FALSE,0,0);  
	    Point.y=(CurView->Rect.bottom + CurView->ScreenRect.bottom)/2;
		Rot += QuaterPY;					
		DisplayPointSymbol (hSymbol,CurView->hDC, -height, width,Rot, &Point,0,FALSE,0,0,FALSE,FALSE,0,0);  
	    Point.x=(CurView->Rect.left + CurView->Rect.right)/2;  
		Rot += QuaterPY;					
		DisplayPointSymbol (hSymbol,CurView->hDC, -height, width,Rot, &Point,0,FALSE,0,0,FALSE,FALSE,0,0);  
	    Point.x=(CurView->Rect.right + CurView->ScreenRect.right)/2;
		Rot += QuaterPY;					
		DisplayPointSymbol (hSymbol,CurView->hDC, -height, width,Rot, &Point,0,FALSE,0,0,FALSE,FALSE,0,0);  
		DestroySymbol (hSymbol); 
		HaveVarFillColor = SaveHVFC;
		SelectObject (CurView->hDC,hOldBrush);
    }
    else if (!CurView->Rotation && (!Printing || From == 3))
    {   
    	DPOINT	Point1, Point2;
    	char	Dir; 
    	short	Deg1, Deg2, Min1, Min2, DegDif, MinDif, Format, Pass;
    	double	Rot, pct, Sec1, Sec2, SecDif, finc, MajorInc, MinorInc, MajorX, MinorX, LastX;
    	HANDLE	hMem=GSSiGlobAlloc (1011,GMEM_MOVEABLE,512*sizeof(double));
    	LPDOUBLE	XTics=(LPDOUBLE)GlobalLock (hMem);
    	LPDOUBLE	YTics=XTics+256;  
    	short	nXTics=0, nYTics=0, i, j;  
    	short	ticwidthfactor = IDNINT(DeviceToScreenFactor());
    	
    	if (DoMinorTics)
    		ticwidthfactor = 1;
    	
		LinePen = CreatePen (PS_SOLID,(int)IDNINT(DeviceToScreenFactor()),0);
		RedPen = CreatePen (PS_SOLID,ticwidthfactor,RGB(0,0,0));
		WhitePen = CreatePen (PS_SOLID,ticwidthfactor*3,RGB(255,255,255));
		OldPen = SelectObject (CurView->hDC,LinePen);
	    SetBkMode(CurView->hDC, OPAQUE);  
	    SetBkColor (CurView->hDC,CurView->BackGroundColor); 
	    
	    Points[0].x = CurView->ScreenRect.left;  
        Points[0].y = CurView->ScreenRect.bottom;
	    Points[1].x = CurView->ScreenRect.left;  
        Points[1].y = CurView->ScreenRect.top;
	    Points[2].x = CurView->ScreenRect.right;  
        Points[2].y = CurView->ScreenRect.top;
	    Points[3].x = CurView->ScreenRect.right;  
        Points[3].y = CurView->ScreenRect.bottom;
	    Points[4]   = Points[0];
	    Polyline (CurView->hDC,Points,5);
        
    	w = (long)CurView->ScreenRect.right - (long)CurView->ScreenRect.left;
		h = ((long)CurView->Rect.bottom - (long)CurView->ScreenRect.bottom)/2;// - 5*DeviceToScreenFactor();

    	Point1.x = CurView->NewBounds.xmn;
    	Point1.y = CurView->NewBounds.ymn;
    	Point2.x = CurView->NewBounds.xmx;
    	Point2.y = CurView->NewBounds.ymn;
    	Dir = 'W';
	    Points[0].y = CurView->ScreenRect.bottom - 4 * DeviceToScreenFactor();
	    Points[1].y = ((long)CurView->ScreenRect.bottom + (long)CurView->Rect.bottom)/2; 
	    MinorPoints[0].y = CurView->ScreenRect.bottom - 1 * DeviceToScreenFactor();
	    MinorPoints[1].y = ((long)CurView->ScreenRect.bottom + (long)CurView->Rect.bottom)/2 - 3 * DeviceToScreenFactor(); 
	    Pass = 2;
    	
    	while (Pass--)
    	{
	    	ConvertCoord (&Point1,1,2);  
	    	GetDMS (Point1.x,&Deg1,&Min1,&Sec1);
	    	ConvertCoord (&Point2,1,2);
	    	GetDMS (Point2.x,&Deg2,&Min2,&Sec2); 
	    	GetDMSMajorMinorInc (fabs (Point1.x-Point2.x),&MajorInc,&MinorInc,&Format);
	    	finc = fmod (Point1.x,MajorInc); 
	    	MajorX = Point1.x-finc;
	    	GetDMS (MajorX,&DegDif,&MinDif,&SecDif); 
	    	MajorX -= MajorInc;
		    while (MajorX < Point2.x)
		    {
			    pct = (MajorX - Point1.x) / (Point2.x - Point1.x);
			    Points[0].x = CurView->ScreenRect.left + w * pct; 
			    Points[1].x = Points[0].x;
		    	GetDMS (MajorX,&Deg1,&Min1,&Sec1);
		    	switch (Format)
		    	{
		    		case 0:
			    		sprintf (txt,"%c%iD",Dir,abs(Deg1));
		    			break;
		    		case 1:
			    		sprintf (txt,"%c%iD %2iM",Dir,abs(Deg1),Min1);
		    			break;
		    		case 2:
			    		sprintf (txt,"%c%iD %2iM %2.0fS",Dir,abs(Deg1),Min1,Sec1);
			    		break;
			    	case 3:
			    		sprintf (txt,"%c%iD %2iM %2.1fS",Dir,abs(Deg1),Min1,Sec1);
			    		break; 
			    }
			    if (Pass)
			    	XTics[nXTics++] = MajorX;
				SelectObject (CurView->hDC,WhitePen);
			    Polyline (CurView->hDC,Points,2);
				SelectObject (CurView->hDC,RedPen);
			    Polyline (CurView->hDC,Points,2);
				SelectObject (CurView->hDC,LinePen);
				MinorX = MajorX + MinorInc;
				LastX = MajorX;
				MajorX += MajorInc;
				while (MinorX < min (MajorX,Point2.x))
				{ 
				    if (Pass && DoMinorTics)
				    	XTics[nXTics++] = MinorX;
				    pct = (MinorX - Point1.x) / (Point2.x - Point1.x);
				    MinorPoints[0].x = CurView->ScreenRect.left + w * pct; 
				    MinorPoints[1].x = MinorPoints[0].x;
				    Polyline (CurView->hDC,MinorPoints,2);
				    MinorX += MinorInc;
				} 
				if (LastX > Point1.x)
					DispText (CurView->hDC,FALSE,Points[1].x-10,Points[1].x+10, Points[1].y,0, 2,2,
					 		  h,1,1,2, FALSE,0,txt,0,FALSE,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0);
			} 
		    Points[0].y = CurView->ScreenRect.top + 4 * DeviceToScreenFactor();
		    Points[1].y = (CurView->ScreenRect.top + CurView->Rect.top)/2; 
		    MinorPoints[0].y = CurView->ScreenRect.top + 1 * DeviceToScreenFactor();
		    MinorPoints[1].y = (CurView->ScreenRect.top + CurView->Rect.top)/2 + 3 * DeviceToScreenFactor(); 
	    	Point1.x = CurView->NewBounds.xmn;
	    	Point1.y = CurView->NewBounds.ymx;
	    	Point2.x = CurView->NewBounds.xmx;
	    	Point2.y = CurView->NewBounds.ymx;
		}
		
    	w = (long)CurView->ScreenRect.bottom - (long)CurView->ScreenRect.top;
		h = ((long)CurView->Rect.right - (long)CurView->ScreenRect.right)/2;// - 5*DeviceToScreenFactor();

    	Dir = 'N';    
    	Rot = HALFPI;
    	Point1.x = CurView->NewBounds.xmn;
    	Point1.y = CurView->NewBounds.ymn;
    	Point2.x = CurView->NewBounds.xmn;
    	Point2.y = CurView->NewBounds.ymx;
	    Points[0].x = CurView->ScreenRect.left + 4 * DeviceToScreenFactor();
	    Points[1].x = (CurView->ScreenRect.left + CurView->Rect.left)/2; 
	    MinorPoints[0].x = CurView->ScreenRect.left + 1 * DeviceToScreenFactor();
	    MinorPoints[1].x = (CurView->ScreenRect.left + CurView->Rect.left)/2 + 3 * DeviceToScreenFactor(); 
	    Pass = 2;
    	
    	while (Pass--)
    	{
	    	ConvertCoord (&Point1,1,2);  
	    	GetDMS (Point1.y,&Deg1,&Min1,&Sec1);
	    	ConvertCoord (&Point2,1,2);
	    	GetDMS (Point2.y,&Deg2,&Min2,&Sec2); 
	    	GetDMSMajorMinorInc (fabs (Point1.y-Point2.y),&MajorInc,&MinorInc,&Format);
	    	finc = fmod (Point1.y,MajorInc); 
	    	MajorX = Point1.y-finc;
	    	GetDMS (MajorX,&DegDif,&MinDif,&SecDif); 
	    	MajorX -= MajorInc;
		    while (MajorX < Point2.y)
		    {
			    pct = (MajorX - Point1.y) / (Point2.y - Point1.y);
			    Points[0].y = CurView->ScreenRect.bottom - w * pct; 
			    Points[1].y = Points[0].y;
		    	GetDMS (MajorX,&Deg1,&Min1,&Sec1);
		    	switch (Format)
		    	{
		    		case 0:
			    		sprintf (txt,"%c%iD",Dir,abs(Deg1));
		    			break;
		    		case 1:
			    		sprintf (txt,"%c%iD %2iM",Dir,abs(Deg1),Min1);
		    			break;
		    		case 2:
			    		sprintf (txt,"%c%iD %2iM %2.0fS",Dir,abs(Deg1),Min1,Sec1);
			    		break;
			    	case 3:
			    		sprintf (txt,"%c%iD %2iM %2.1fS",Dir,abs(Deg1),Min1,Sec1);
			    		break; 
			    }
			    if (Pass)
			    	YTics[nYTics++] = MajorX;
				SelectObject (CurView->hDC,WhitePen);
			    Polyline (CurView->hDC,Points,2);
				SelectObject (CurView->hDC,RedPen);
			    Polyline (CurView->hDC,Points,2);
				SelectObject (CurView->hDC,LinePen);
				MinorX = MajorX + MinorInc;
				LastX = MajorX;
				MajorX += MajorInc;
				while (MinorX < min (MajorX,Point2.y))
				{ 
				    if (Pass && DoMinorTics)
				    	YTics[nYTics++] = MinorX;
				    pct = (MinorX - Point1.y) / (Point2.y - Point1.y);
				    MinorPoints[0].y = CurView->ScreenRect.bottom - w * pct; 
				    MinorPoints[1].y = MinorPoints[0].y;
				    Polyline (CurView->hDC,MinorPoints,2);
				    MinorX += MinorInc;
				} 
				if (LastX > Point1.y)
					DispText (CurView->hDC,FALSE,Points[1].x-10,Points[1].x+10, Points[1].y, 0,2,2,
					 		  h,1,1,2, FALSE,Rot,txt,0,FALSE,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0);
			} 

		    Points[0].x = CurView->ScreenRect.right - 4 * DeviceToScreenFactor();
		    Points[1].x = (CurView->ScreenRect.right + CurView->Rect.right)/2; 
		    MinorPoints[0].x = CurView->ScreenRect.right - 1 * DeviceToScreenFactor();
		    MinorPoints[1].x = (CurView->ScreenRect.right + CurView->Rect.right)/2 - 3 * DeviceToScreenFactor(); 
	    	Point1.x = CurView->NewBounds.xmx;
	    	Point1.y = CurView->NewBounds.ymn;
	    	Point2.x = CurView->NewBounds.xmx;
	    	Point2.y = CurView->NewBounds.ymx;
		}  
		
		for (i=0;i<nXTics;i++)
			for (j=0;j<nYTics;j++)
			{   
				DPOINT	TicPointD;
				POINT	TicPoint;
				
				TicPointD.x = XTics[i];
				TicPointD.y = YTics[j];
		    	ConvertCoord (&TicPointD,2,1);  
				TicPoint = BasePtToWinPt (&TicPointD);
				if (PtInRect (&CurView->ScreenRect,TicPoint))
				{   
					Points[0] = Points[1] = TicPoint;
					Points[0].x -= 3*DeviceToScreenFactor();
					Points[1].x += 3*DeviceToScreenFactor();
					Points[2] = Points[3] = TicPoint;
					Points[2].y -= 3*DeviceToScreenFactor();
					Points[3].y += 3*DeviceToScreenFactor();
					SelectObject (CurView->hDC,WhitePen);
				    Polyline (CurView->hDC,Points,2);
				    Polyline (CurView->hDC,&Points[2],2);
					SelectObject (CurView->hDC,RedPen);
				    Polyline (CurView->hDC,Points,2); 
				    Polyline (CurView->hDC,&Points[2],2); 
				}
			}
			
		SelectObject (CurView->hDC,OldPen);
		DeleteObject (LinePen); 
		DeleteObject (RedPen); 
		DeleteObject (WhitePen); 
		GSSiGlobUlFree (&hMem);
	}
	break;
	case 1://GoogleMaps
		{
			int		iZoom;

			iZoom = GetGlobalLVal2 ("[%WANTGRID]",0);
			if (iZoom > 0)
				iZoom--;
			else
				iZoom = CurTheme->GridZoom>0?CurTheme->GridZoom:GetGoogleZoomForSCale (CurView->Scale)-abs(CurTheme->GridZoom);
			
			if (From == 4)
			{
				int		zoom, tilex, tiley;
				int		nrows, ncols, irow, icol, i;
				double	scale;
				MNMXCORD	tileBoundsll, tileBoundsur, bounds, boundsBase, boundsLL,gBounds,BoundsInGoogleProjection;
				double  displayScale = GetGoogleScaleForZoom (iZoom);
				double	tileScale, tileWidthHeight = 256 * displayScale;
				DPOINT	gPoint, dPoints[4];
				POINT	sPoints[4], cp;
				char	txt[512];
				int		iTileXLL, iTileYLL;
				int		iTileXUR, iTileYUR;
				char	quadKey[24];
				HFILE	fidGridOut = HFILE_ERROR;
				char	GridOutFile[MAX_PATH];

				unsigned int msize = MapSize(iZoom);
				int		 maxrowcol = msize/256;
				int		 maxSubfileRowcol = maxrowcol/16;
				int		 iSubfile, iSubfileRow, iSubfileCol;
				BOOL	doDisplay = TRUE;

				if (iZoom < 12)
					maxSubfileRowcol = maxrowcol;
				itoa (iZoom+1,txt,10);
				if (!GetGridDef (txt))
					break;
				if (GetGlobalCVal ("[%GRIDOUTPUTFILE]",GridOutFile,0))
				{
					if (ExistFile (GridOutFile))
					{
						fidGridOut = GSSiOpenFile (GridOutFile,0,OF_READWRITE);
						GSSillseek (fidGridOut,0,2);
					}
					else
					{
						fidGridOut = GSSiOpenFile (GridOutFile,0,OF_CREATE);
						fputstring ("GRIDID\tGROW\tGCOL\tSUBFILE\tSFROW\tSFCOL\tSFTILEY\tSFTILEX\tGRIDCELLID\tGRIDBOUNDS",fidGridOut);
					}
					doDisplay = FALSE;
				}
				if (displayScale > 0)
				{
					HPEN hPen = CreatePen (PS_SOLID,1,0);
					HPEN hOldPen = SelectObject (CurView->hDC,hPen);

					if (ConvertRectCoord (&BoundsInGoogleProjection,&CurView->WBounds, 1,SphericalMercatorPROJECTION))
					{
						DPOINT	Point;
						int GridCelID;
						int googleRow;

						AddjustSphericalMercatorBounds (&BoundsInGoogleProjection);
						InflateBounds (&BoundsInGoogleProjection,-1.0);
						Point.x = BoundsInGoogleProjection.xmn;
						Point.y = BoundsInGoogleProjection.ymn;
						GetGoogleTileBoundsFromPointAndZoom (iZoom,Point,&tileBoundsll,&iTileYLL,&iTileXLL,&googleRow);
						Point.x = BoundsInGoogleProjection.xmx;
						Point.y = BoundsInGoogleProjection.ymx;
						GetGoogleTileBoundsFromPointAndZoom (iZoom,Point,&tileBoundsur,&iTileYUR,&iTileXUR,&googleRow);
						nrows = iTileYUR - iTileYLL + 1;
						ncols = iTileXUR - iTileXLL + 1;
						for (irow=0;irow<nrows;irow++)
						{
							for (icol=0;icol<ncols;icol++)
							{
								bounds.ymn = tileBoundsll.ymn + irow * tileWidthHeight;
								bounds.ymx = bounds.ymn + tileWidthHeight;
								bounds.xmn = tileBoundsll.xmn + icol * tileWidthHeight;
								bounds.xmx = bounds.xmn + tileWidthHeight;
								tileScale = GetGoogleTileBoundsFromPointAndZoom (iZoom,MinMaxMidPointD (&bounds),&gBounds,&tiley,&tilex,&googleRow);
								ConvertRectCoord (&boundsBase, &gBounds,-1,1);
								ConvertRectCoord (&boundsLL, &gBounds,-1,2);
								if (!IntersectBounds (&CurView->WBounds,&boundsBase,0))
									continue;
								BoundsToPoints (&boundsBase, dPoints,0);
								cp.x = cp.y = 0;
								for (i=0;i<4;i++)
								{
									sPoints[i] = BasePtToScreenPt (&dPoints[i]);
									cp.x += sPoints[i].x;
									cp.y += sPoints[i].y;
								}
								cp.x /= 4;
								cp.y /= 4;
								//TileXYToQuadKey(tilex,tiley,iZoom,quadKey,24);
								//TextOut (CurView->hDC,cp.x-35,cp.y+16,quadKey,strlen(quadKey));
								if ( maxSubfileRowcol)
								{
									iSubfileRow = tiley / maxSubfileRowcol;
									iSubfileCol = tilex / maxSubfileRowcol;
								}
								else
									iSubfileRow = iSubfileCol = 0;
								iSubfile = iSubfileRow * 16 + iSubfileCol;
								GridCelID = (tiley%maxSubfileRowcol) * maxSubfileRowcol + (tilex%maxSubfileRowcol);

								if (fidGridOut != HFILE_ERROR)
								{
									//sprintf (txt,"%i\t%i\t%.2f %.2f %.2f %.2f",iZoom,GridCelID,gBounds.xmn,gBounds.ymn,gBounds.xmx,gBounds.ymx);
									sprintf (txt,"%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%.2f %.2f %.2f %.2f\t%.6f %.6f %.6f %.6f",iZoom,tiley,tilex,iSubfile,iSubfileRow,iSubfileCol,tiley%maxSubfileRowcol,tilex%maxSubfileRowcol,GridCelID,gBounds.xmn,gBounds.ymn,gBounds.xmx,gBounds.ymx,
																								boundsLL.xmn,boundsLL.ymn,boundsLL.xmx,boundsLL.ymx);
									fputstring (txt,fidGridOut);
								}
								else if (GetGlobalBVal2 ("[%CREATETGOUTPUT]",FALSE))
									DisplayTileGraphics (CurView->hDC,"F:\\MNVector",iZoom,tilex,tiley,sPoints);
								else
								{
									Polygon (CurView->hDC,sPoints,4);
									if (CurTheme->ShowValue)
									{
										RECT rect;
										POINT center;
										
										TileXYToQuadKey(tilex,tiley,iZoom,quadKey,24);
										SetGlobalValueLong("%GRIDZOOM", iZoom);
										SetGlobalValueLong("%GRIDX",tilex);
										SetGlobalValueLong("%GRIDY", tiley);
										SetGlobalValueLong("%GRIDID", GridCelID);
										SetGlobalValue("%GRIDQUADKEY", quadKey);
										strcpy(txt, CurTheme->DataDisplayMacro);
										ExpandText(txt);
										RectInit(&rect);
										for (i = 0; i < 4;i++)
											AddPointToRect(sPoints[i], &rect);
										center = RectMid(&rect);
										DrawTextEx(CurView->hDC, txt, strlen(txt), &rect, DT_CENTER|DT_CALCRECT, 0);
										CenterRectOnPoint(&rect, center);
										DrawTextEx(CurView->hDC, txt, strlen(txt), &rect, DT_CENTER, 0);
									}
								}
							}
						}
						SelectObject (CurView->hDC,hOldPen);
						DeleteObject (hPen);
					}
				}
				if (fidGridOut != HFILE_ERROR)
				{
					GSSiClose (fidGridOut);
					SetGlobalValue ("%GRIDOUTPUTFILE","");
					MessageBox (0,"Done","",MB_OK);
				}
			}
		}
		break;
	}

	RestoreDC (CurView->hDC,-1);
//	SetCurView (pViewports[CurViewID-1]); //tempdebu
    SetViewport (CurViewID);
	
{
#if ENABLETRACE
GSSiExitProg (1282);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}
   
HANDLE	CreateListFile (LPSTR File,short lenListData,BOOL SortOnData)
#if ENABLETRACE
{GSSiEnterProg (1283);
#endif
{
 	int		i;
	BTVARDESC BTVar[2], *pVars;
	int		NumFields, Reclen, len;
	long	Refno, Offset;
	long	TotFileLen;
	GWDHEADER16 GWDHead; 
	LPGWDHEADER16	lpGWDHead;
	GWFLDINFO GWFldInfo;
	LPGWFLDINFO	lpGWFldInfo;
	HANDLE hBT, hDB;
	HFILE	FidData;
	int		ibeg,NumVars;
	OFSTRUCTGM	OFStruct;
	GWFLDINFO FldInfo;
	LPSTR	lpDot;  

	 lpGWDHead = &GWDHead; 
    _fmemset (lpGWDHead,0,sizeof(GWDHEADER16));

	 FidData = GSSiOpenFile (File,&OFStruct,OF_CREATE);
	 GWDHead.NumFields=0;
	 GWDHead.NumIndex=1; 
	 if (SortOnData)
		 GWDHead.NumIndex=2; 
	 GWDHead.Version=1;
	 GWDHead.NumIndexFields[0]=1;
	 GWDHead.IndexFields[0][0]=0;
	 GWDHead.NumIndexFields[1]=1;
	 GWDHead.IndexFields[1][0]=4;
	 BigWrite (FidData,(HPSTR)&GWDHead,sizeof(GWDHEADER16),-1);
	ibeg = 0;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"Refno");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 8;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"Prefix");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 32;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"UDI");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;
	
	FldInfo.Len = 32;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"Symbol");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;
	
	FldInfo.Len = min(2,lenListData);
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"ListData");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;
	
	 GWDHead.Reclen=ibeg; 
	 GWDHead.TimeStamp = 0;
	 GSSillseek (FidData,0,0);
	 BigWrite (FidData,(HPSTR)&GWDHead,sizeof(GWDHEADER16),-1);
 	 GSSillseek (FidData,0,2);
			     
	 NumVars = 1;
			
	 BTVar[0].BT_VARLEN=4;
	 BTVar[0].BT_VARTYP=BT_INTEGER;
	 BTVar[0].BT_VAROFF=0;
	 lpDot = _fstrrchr (File,'.');
	 _fstrcpy (lpDot,".in1");	
	 BT_CREATE (File, 4, FALSE, 1, 1,BTVar,FALSE, 0, 0, FALSE);
 	 GSSiClose (FidData);
	 _fstrcpy (lpDot,".gmd");	

     hDB = OpenGWDatabase (File,BT_WRITE);
{
#if ENABLETRACE
GSSiExitProg (1283);
#endif
     return hDB; 
}
#if ENABLETRACE
}
#endif
}  

BOOL AddToListFile (long Refno,LPSTREETTEXTDATA	pStreetData)
#if ENABLETRACE
{GSSiEnterProg (1284);
#endif
{   
	LPGWDHEADER	lpGWDHead;    
	LPLONG		pRefno;     
	char		str[256];
	
	if (!pStreetData->hListDB)
{
#if ENABLETRACE
GSSiExitProg (1284);
#endif
		return FALSE;   
}
	lpGWDHead = (LPGWDHEADER)GlobalLock (pStreetData->hListDB); 
	pRefno = (LPLONG)&lpGWDHead->GWDData;
	*pRefno = Refno;
	SetFieldValFromCharAndName(lpGWDHead, "Symbol", "", FALSE, TRUE);
	SetFieldValFromCharAndName(lpGWDHead, "Prefix", CurrentPrefix, FALSE, TRUE);
	SetFieldValFromCharAndName(lpGWDHead, "UDI", CurrentUDI, FALSE, TRUE);
	_fstrcpy (str,pStreetData->ListData);
	SetFieldValFromCharAndName(lpGWDHead, "ListData", str, FALSE, TRUE);
	GWDAddRecord (lpGWDHead,0,0);      
	GlobalUnlock (pStreetData->hListDB); 
{
#if ENABLETRACE
GSSiExitProg (1284);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

void DisplayCoordPrintText (void)
#if ENABLETRACE
{GSSiEnterProg (1285);
#endif
{   
	short	h,w;
	LPCOORDINATEDISPLAY	CD; 
	POINT	Points[3];  
	char	txt[256]; 
	
	CD = (LPCOORDINATEDISPLAY)CurTheme;
	if (!CD->PrintText)
{
#if ENABLETRACE
GSSiExitProg (1285);
#endif
		return;  
}
	CurTheme->FidDelayedText = HFILE_ERROR;
	_fstrcpy (txt,CD->PrintText);
	ExpandText (txt);
	SaveDC (CurView->hDC);
	SetDisplayMode (CurView->hDC, GF_SCREENMODE);
    GSSiDeleteObject(&CurView->hRgn);
    CurView->hRgn = CreateVPRgn(FALSE,FALSE);
    SelectClipRgn (CurView->hDC,CurView->hRgn);
    GSSiDeleteObject(&CurView->hRgn);    
    SetTextColor (CurView->hDC,0);     
	FillRectPoly (CurView->hDC,&CurView->ScreenRect,ConvertColor(CurView->BackGroundColor,-1));  
	h = (((CurView->ScreenRect.bottom - CurView->ScreenRect.top) / 2)-3*DeviceToScreenFactor());
	w = (h+4*DeviceToScreenFactor());
    SetBkMode(CurView->hDC, OPAQUE);  
    SetBkColor (CurView->hDC,CurView->BackGroundColor);
    Points[0].y = (CurView->ScreenRect.bottom + CurView->ScreenRect.top) / 2;
    Points[0].x = (CurView->ScreenRect.left + CurView->ScreenRect.right) / 2; 
	DispText (CurView->hDC,FALSE,Points[0].x-10,Points[0].x+10, Points[0].y,0, 2,2,
			  h*2+6,1,1,2, FALSE,0,txt,0,FALSE,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0); 
	RestoreDC (CurView->hDC,-1);
{
#if ENABLETRACE
GSSiExitProg (1285);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}
  
void SBHash (HDC hDC,int nbars,int left,int top,long Width,long Height)
{
	POINT	Points[4];
	short	i; 
	HBRUSH	hBrush; 
	
	SaveDC (CurView->hDC);
	hBrush = SelectObject (hDC,GetStockObject(BLACK_BRUSH));
	
//	nbars *= DeviceToScreenFactor();
	Points[0].x = left;
	Points[0].y = top;
	Points[1].x = left;
	Points[1].y = top + Height;
	Points[2].x = left+Width;
	Points[2].y = top + Height;
	Points[3].x = left+Width;
	Points[3].y = top; 
	Polygon (hDC,Points,4);  
	if (hBrush)
		SelectObject (hDC,hBrush);
	RestoreDC (hDC,-1);
	return;
	Polyline (hDC,Points,2);
	Points[1].x = left + Width;
	Points[1].y = top;
	Polyline (hDC,Points,2);
	Points[1].x = left + Width;
	for (i=0;i<nbars;i++)
	{   
		Points[0].y = top + ((i+1) * Height)/nbars; 
		Points[1].y = Points[0].y;
		Polyline (hDC,Points,2);
	}
	Points[0] = Points[1];
	Points[1].y = top;
	Polyline (hDC,Points,2);
	return;
}

void GetDistDecimals (double Dist,LPSTR DistC)
{   
	short	l,n;   
	LPSTR	pEnd;
	
	sprintf (DistC,"%.2f",Dist);
	l = _fstrlen (DistC);
	if (l<7)
		return;
	pEnd = LastChr (DistC);
	n = min (2,l - 7); 
	if (n==1)
		n=2;
	pEnd -= n;
	*pEnd = 0;
	return;
}

void DisplayNorthArrowLegend (int From)
{
	static	BOOL	InDNA=FALSE;
	if (InDNA || !CurTheme || !CurView)
		return;
	{
		LPVIEWPORT	SaveVP=CurView;
		LPVISLIST	SaveVis=CurVis;
		int			SaveFile = CurView->CurFile;
		BOOL SaveDisableHalt = DisableHalt;

/*		switch (From)
		{
		case 2:
		case 0:
		case 4:
			return;
		}*/
		if (CurTheme->TargetViewport)
			CurView->Rotation = pViewports[CurTheme->TargetViewport-1]->Rotation;
		InDNA = TRUE;
		DisableHalt = TRUE;
		if (CurView->NumFiles)
		{
			char	File[MAX_PATH];
			int		RegionType;

			strcpy (File,CurView->lpFiles[0]);
			if (MapFileType (File) == MT_IMAGE)
			{
				HDIB32	hDib = LoadDIB32 (File,TRUE);
				MNMXCORD	BitmapBounds, SaveBounds;
    			
				if (hDib)
				{
					double	SaveRot = CurView->Rotation;
					XFORM	SavexForm; 
					long	SaveCycle = DisplayCycle;

			    	GetImageBounds (File,hDib,&BitmapBounds,&CurView->NewBounds);
					DestroyDIB32 (hDib,FALSE);
					SaveBounds = CurView->NewBounds;
				    CurView->HaveBounds = FALSE;
					SetScaleAndMidpointFromBounds (CurView);
					DisplayCycle++;
					SetBounds (CurView->hWnd,CurView->hDC);
					SavexForm = CurView->xForm;
					CurView->Rotation = 0;
					CurView->NewBounds = CurView->WBounds = SaveBounds;
					SetScaleAndMidpointFromBounds (CurView);
					DisplayCycle++;
					SetBounds (CurView->hWnd,CurView->hDC);
					SaveDC (CurView->hDC);
					ResetViewport (TRUE,FALSE); 
					CurView->Rotation = SaveRot;
					CurView->xForm = SavexForm;
					SetDisplayMode (CurView->hDC, GF_TEXTMODE);
					//CurView->Rotation = 0; northarrow image not rotating with this line
					GSSiDeleteObject(&CurView->hRgn);
					CurView->hRgn = CreateVPRgn(FALSE,FALSE);
					RegionType = SelectClipRgn (CurView->hDC,CurView->hRgn); 
					GSSiDeleteObject(&CurView->hRgn);    
					DisplayBMFileInVP32 (CurView->hDC, File,0,0);
					RestoreDC (CurView->hDC,-1);
					CurView->Rotation = SaveRot;
					DisplayCycle = SaveCycle;
				}
			}
		}
	//	RedisplayViewport (TRUE,TRUE);
		DisableHalt = SaveDisableHalt;
		InDNA = FALSE;
		CurView = SaveVP;
		CurVis = SaveVis;
		CurView->CurFile = SaveFile;
	}
	return;
}

void DisplayDistanceThemeLegend(short From,double ThisDist,double AZ,double TotDist)
#if ENABLETRACE
{GSSiEnterProg (1286);
#endif
{    
	short	CurViewID = CurView->ID, reps, h,w, Type;    
	double	BaseDistPerPixel, MaxWidth, BarWidth, step;
	POINT	Points[3];  
	long	IntVal,DisplayVPWidth;  
	char	txt[64], str[16], fmt[16]=" %.0f %s ";
	char	DistC[32], TotDistC[32], BearingC[32]; 
	char	DegC[8],MinC[8],SecC[16], PreDir[4], PostDir[4]; 
	HPEN	OldPen, LinePen;
    HFONT	hFont, hFontOld;
	int	ilog,RegionType;
	double	xlog;
	
	if (!CurView->hDC)
		return;
	SaveDC (CurView->hDC);
	SetDisplayMode (CurView->hDC, GF_SCREENMODE);
    GSSiDeleteObject(&CurView->hRgn);
    CurView->hRgn = CreateVPRgn(FALSE,FALSE);
    RegionType = SelectClipRgn (CurView->hDC,CurView->hRgn); 
    GSSiDeleteObject(&CurView->hRgn);    
    if (RegionType == NULLREGION && !PrintingToMF)
    	goto Exit;
    SetTextColor (CurView->hDC,0);     
	FillRectPoly (CurView->hDC,&CurView->ScreenRect,ConvertColor(CurView->BackGroundColor,-1));  
	h = (((CurView->ScreenRect.bottom - CurView->ScreenRect.top)/2)-2*DeviceToScreenFactor());
	w = (h+4*DeviceToScreenFactor());
	if (From == 2)
	{    
		short	DistUnits = abs(CurTheme->ValConv);
		
		if (!DistUnits)
			DistUnits = OutDistUnits;
		h = min (32,0.9 * (CurView->ScreenRect.bottom - CurView->ScreenRect.top));
		Points[0] = RectMid (&CurView->ScreenRect); 
		if (TotDist > 0)
		{
			GetDistDecimals (ConvertDist(ThisDist,DistUnits),DistC);
			GetDistDecimals (ConvertDist(TotDist+ThisDist,DistUnits),TotDistC);
			sprintf (txt,"%s (%s total) %s", DistC,TotDistC,DistUnitOpts[DistUnits-1]); 
		} 
		else if (TotDist < 0)
		{
			GetDistDecimals (ConvertDist(ThisDist,DistUnits),DistC);
			AZToBear (AZ,PreDir,DegC,MinC,SecC,PostDir); 
			sprintf (BearingC,"%s %s %s %s %s",PreDir,DegC,MinC,SecC,PostDir);
			sprintf (txt,"%s %s    %s", DistC,DistUnitOpts[DistUnits-1],BearingC); 
		}
		else
		{
			GetDistDecimals (ConvertDist(ThisDist,DistUnits),DistC);
			sprintf (txt,"%s %s", DistC,DistUnitOpts[DistUnits-1]); 
		}
	    SetBkMode(CurView->hDC, OPAQUE);  
	    SetBkColor (CurView->hDC,CurView->BackGroundColor);
		DispText (CurView->hDC,FALSE,Points[0].x-10,Points[0].x+10, Points[0].y,0, 2,2,
				  h,1,1,2, FALSE,0,txt,0,FALSE,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0); 
		goto Exit;
	} 
	DisplayVPWidth = CurView->ScreenRect.right - CurView->ScreenRect.left;
    SetViewport (CurTheme->TargetViewport);  
    if (CurView->NewBounds.xmx < CurView->NewBounds.xmn)
    	goto Exit;
    if (Printing)
    	Type = 2;
    else
    	Type = CurTheme->ClassType;  
	switch (Type)
	{
		case 1:
		{   DPOINT	Point1,Point2; 
			double	BaseDist,WinDist;
		
			LinePen = CreatePen (PS_SOLID,(int)IDNINT(DeviceToScreenFactor()*3),CurTheme->ScatterColor);
			OldPen = SelectObject (CurView->hDC,LinePen);
			Point1.x = CurView->NewBounds.xmn;
			Point1.y = CurView->NewBounds.ymn;
			Point2.x = CurView->NewBounds.xmx;
			Point2.y = CurView->NewBounds.ymx; 
			BaseDist = GetBaseDist (&Point1,&Point2);
			Point1.x = CurView->ScreenRect.left;
			Point1.y = CurView->ScreenRect.bottom;
			Point2.x = CurView->ScreenRect.right;
			Point2.y = CurView->ScreenRect.top; 
			WinDist = ldistp (Point1,Point2);
		    BaseDistPerPixel = BaseDist/WinDist;	
		    BaseDistPerPixel = ConvertDist (BaseDistPerPixel,CurTheme->ValConv);  
			if (!BaseDistPerPixel)
				goto Exit;
		    SetViewport (CurViewID);
		    MaxWidth = ((long)CurView->ScreenRect.right - (long)CurView->ScreenRect.left) * BaseDistPerPixel;
		    reps = 0;
		    if (MaxWidth > 1)            
		    {   
		    	step = 10.0;
		    	while (MaxWidth > 1)
		    	{
		    		MaxWidth /= 10.0;
		    		reps++;
		    	}
		    	IntVal = MaxWidth * 10;
			    reps--;
		    	if (IntVal > 1 || reps)
		    		_fstrcpy (fmt," %.0f %s ");
		    }
		    else
		    {   
		    	step = 0.1;    
		    	_fstrcpy (fmt," %.1f %s ");
		    	while (MaxWidth < 1)
		    	{
		    		MaxWidth *= 10.0;
		    		reps++;
		    	} 
		    	IntVal = MaxWidth; 
		    	itoa (reps,str,10);
		    	fmt[3] = *str;
		    }
		    MaxWidth = IntVal; 
		    while (reps--)
		    	MaxWidth *= step;
		    BarWidth = MaxWidth;
		    Points[0].y = (CurView->ScreenRect.bottom + CurView->ScreenRect.top) / 2;
		    Points[0].x = (CurView->ScreenRect.left + CurView->ScreenRect.right) / 2; 
		    Points[1].y = Points[0].y;
		    Points[1].x = Points[0].x + IDNINT(BarWidth/(2 * BaseDistPerPixel));
		    Polyline (CurView->hDC,Points,2);
		    Points[2] = Points[1];  
		    Points[2].x -= w;
		    Points[2].y -= h;
		    Polyline (CurView->hDC,&Points[1],2);
		    Points[2].y += h*2;   
		    Polyline (CurView->hDC,&Points[1],2);
		    Points[1].x = Points[0].x - IDNINT(BarWidth/(2 * BaseDistPerPixel));
		    Polyline (CurView->hDC,Points,2);
		    Points[2] = Points[1];  
		    Points[2].x += w;
		    Points[2].y -= h;
		    Polyline (CurView->hDC,&Points[1],2);
		    Points[2].y += h*2;   
		    Polyline (CurView->hDC,&Points[1],2); 
		    sprintf (txt,fmt,BarWidth,DistUnitOpts[max(0,CurTheme->ValConv-1)]);
			SelectObject (CurView->hDC,OldPen);
			DeleteObject (LinePen);
		    SetBkMode(CurView->hDC, OPAQUE);  
		    SetBkColor (CurView->hDC,CurView->BackGroundColor);
			DispText (CurView->hDC,FALSE,Points[0].x-10,Points[0].x+10, Points[0].y,0, 2,2,
					  h*2+6,1,1,2, FALSE,0,txt,0,FALSE,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0); 
		}  
		break;
		default: 
		{   
			long	VPWidth = (long)CurView->ScreenRect.right - (long)CurView->ScreenRect.left;
		    double  BaseDistPerPixel = (CurView->NewBounds.xmx - CurView->NewBounds.xmn)/VPWidth;
		    double  BaseDistPerUnit  = -CurView->OrthoRes; 
		    double	MinorInc, MajorInc,ScaleDist=CurView->ScaleDist;  
		    long	BarHeight, BarWidth, Width, Height;
		    RECT	Bar;   
		    short	x=0,y=0, xmid, ymid, ScaleDistUnits=CurView->ScaleDistUnits;  
		    int		NumHash=5, NumMinorInc=5, StartMinorInc, NumMajorInc=3, i, j,Left, Right,Top, startinc=0;
		    int		sign=1, UpInc, LowInc;  
		    long	Dist1=1, Dist2=5, DistFactor=1;
		    short	NumDec;
		    
		    if (!DevicePixelsPerInch || !BaseDistPerPixel)
		    	break;
 			if (!CurView->WindowZoomedToOrtho || CurView->OrthoRes >= 0)
 			{   
 				double	Min, Max;
 				
 				if (CurTheme->ValConv <= 0)
 				{ 
 					if (ConvertDist (DisplayVPWidth * BaseDistPerPixel,4) < GetGlobalDVal2 ("[%SWITCHTOMILESAT]",1.0))
 						CurTheme->ValConv = -1;
 					else
 						CurTheme->ValConv = -4;
 				}
 				ScaleDistUnits = abs (CurTheme->ValConv);  
 				ScaleDist = ConvertDist (BaseDistPerPixel * DevicePixelsPerInch,ScaleDistUnits);
 				ilog = log10 (ScaleDist);
 				Min = pow (10,ilog);
 				Max = pow (10,ilog+1);
 				if (ScaleDist < Max/2)
 				{
 					UpInc = 5;
 					LowInc = 10;
 					Max /= 2;
 				}
 				else 
 				{
 					UpInc = 10;
 					LowInc = 5;
 					Min = Max / 2;
 				} 
 				if (ScaleDist - Min < Max - ScaleDist)  
 				{
 					NumMinorInc = LowInc;
 					ScaleDist = Min;     
 				}
 				else
 				{
 					NumMinorInc = UpInc;
 					ScaleDist = Max;
 				}
 				BaseDistPerUnit = ConvertInDist (ScaleDist,ScaleDistUnits);
 			} 
 			else
 			{   
 				ScaleDistUnits++;
				i = 1; 
				StartMinorInc = NumMinorInc;
				while (fmod (log10 (ScaleDist/NumMinorInc),1) && i < 30)
				{   
					i++;
					j = i/2;
					NumMinorInc = max (StartMinorInc+ sign*j,3); 
					sign = -1 * sign; 
				}
			}
 			SetBkMode(CurView->hDC, TRANSPARENT);  
		    SetViewport (CurViewID);   
			VPWidth = CurView->ScreenRect.right - CurView->ScreenRect.left;
			MinorInc = BaseDistPerUnit / NumMinorInc;
			MajorInc = BaseDistPerUnit;
			BarHeight = (CurView->ScreenRect.bottom - CurView->ScreenRect.top)/3; 
			NumMajorInc = 1;
			BarWidth = IDNINT ((NumMajorInc + 1) * MajorInc / BaseDistPerPixel);   
			while (BarWidth > VPWidth)
			{   
				ScaleDist /= 10;
				MajorInc /= 10;
				MinorInc /= 10;
				BarWidth = IDNINT ((NumMajorInc + 1) * MajorInc / BaseDistPerPixel);   
			}
			ilog = xlog = log10 (ScaleDist);
			if (ScaleDist < 1)
				NumDec = abs (ilog)+1;
			else
				NumDec = 0;
			while (BarWidth < VPWidth -  1.5 * IDNINT (MajorInc / BaseDistPerPixel))
			{
				NumMajorInc++;
				BarWidth = IDNINT ((NumMajorInc + 1) * MajorInc / BaseDistPerPixel);
			}	
			if (NumMajorInc > 10 && NumMajorInc%2)
				NumMajorInc--;
			BarWidth = IDNINT ((NumMajorInc + 1) * MajorInc / BaseDistPerPixel);
			Bar.left = CurView->ScreenRect.left + (CurView->ScreenRect.right - CurView->ScreenRect.left)/2 - BarWidth/2;
			Bar.right = Bar.left + BarWidth; 
			Bar.bottom = CurView->ScreenRect.bottom - BarHeight; 
			Bar.top = Bar.bottom - BarHeight;    
			xmid = Bar.left + (Bar.right - Bar.left) / 2;    
			ymid = Bar.top  + (Bar.bottom - Bar.top) / 2;
			LinePen = CreatePen (PS_SOLID,(int)IDNINT(DeviceToScreenFactor()),CurTheme->ScatterColor);
			OldPen = SelectObject (CurView->hDC,LinePen);
		    Points[0].y = Bar.bottom;
		    Points[0].x = Bar.left; 
		    Points[1].y = Bar.bottom;
		    Points[1].x = Bar.right; 
		    Polyline (CurView->hDC,Points,2);  
		    Points[0].y -= BarHeight;
		    Points[1].y = Points[0].y;
		    Polyline (CurView->hDC,Points,2);  
		    Points[1].y += BarHeight;
		    Points[1].x = Points[0].x;
		    Polyline (CurView->hDC,Points,2);  
		    Points[0].x = Points[1].x = Bar.right;
		    Polyline (CurView->hDC,Points,2);  
	    	Width = IDNINT (MinorInc/BaseDistPerPixel); 
	    	Height = BarHeight/2;
		    for (i=0;i<NumMinorInc;i++)
		    {   
		    	j = i;
		    	Left = IDNINT (Bar.left + (i*MinorInc) / BaseDistPerPixel);
		    	Right = IDNINT (Bar.left + ((i+1)*MinorInc) / BaseDistPerPixel);
		    	Top  = Bar.top + j%2 * Height;
			    SBHash (CurView->hDC,NumHash,Left,Top,Right-Left,Height);
		    }
		    
	    	Width = IDNINT (MajorInc/BaseDistPerPixel); 
			if (CurTheme->ScaleBarTextFactor <= 0)
				CurTheme->ScaleBarTextFactor = 0.7;
			CurTheme->TitleFont.lfHeight = -BarHeight * CurTheme->ScaleBarTextFactor;
			hFont = CreateFontIndirect((PLOGFONT)&CurTheme->TitleFont);
			hFontOld = SelectObject(CurView->hDC, hFont);    
			SetTextAlign (CurView->hDC,TA_CENTER|TA_BOTTOM);  
		    SetTextColor (CurView->hDC,0);     
			y = Bar.top;
		    x = Bar.left; 
		    if (startinc)
		    	_fstrcpy (txt,"0");
			else	
			    RWRITE (ScaleDist,NumDec, txt); 
//				ltoa (IDNINT(ScaleDist),txt,10);
			ExtTextOut (CurView->hDC,x,y,0,0,txt,_fstrlen(txt),0);
		    for (i=0;i<NumMajorInc;i++)
		    {    
		    	j++;
		    	Left = IDNINT (Bar.left + ((i+1)*MajorInc) / BaseDistPerPixel);
		    	Right = IDNINT (Bar.left + ((i+2)*MajorInc) / BaseDistPerPixel);
		    	Top  = Bar.top + j%2 * Height;
			    SBHash (CurView->hDC,NumHash,Left,Top,Right-Left,Height);
			    RWRITE ((i+startinc) * ScaleDist,NumDec, txt); 
//				ltoa ((i+startinc) * IDNINT(ScaleDist),txt,10);
				x = Left;
				if (NumMajorInc < 10 || i < 2 || (i-1)%2)
					ExtTextOut (CurView->hDC,x,y,0,0,txt,_fstrlen(txt),0);
		    }
		    x = Bar.right; 
		    RWRITE ((i+startinc) * ScaleDist,NumDec, txt); 
//			ltoa ((i+startinc) * IDNINT(ScaleDist),txt,10);
			ExtTextOut (CurView->hDC,x,y,0,0,txt,_fstrlen(txt),0);
			SelectObject (CurView->hDC,OldPen);
			DeleteObject (LinePen);
	
			_fstrcpy (txt,DistUnitOpts[ScaleDistUnits-1]); 
			_fstrlwr (&txt[1]);  
			x = xmid;
			y = Bar.bottom;
			SetTextAlign (CurView->hDC,TA_CENTER|TA_TOP);
			ExtTextOut (CurView->hDC,x,y,0,0,txt,_fstrlen(txt),0); 
			SelectObject(CurView->hDC, hFontOld);
			GSSiDeleteObject (&hFont);
		}
		break;
	} 
Exit:
    SetViewport (CurViewID);   
	RestoreDC (CurView->hDC,-1);
	
{
#if ENABLETRACE
GSSiExitProg (1286);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

HANDLE ExpandThemeValues (HANDLE hBT,LPSTR filePathname)
#if ENABLETRACE
{GSSiEnterProg (1289);
#endif
{   
	HANDLE	hBTNew;       
	char	Value[256], NewValue[130];
	short	pos=BT_FIRST;
	int		iclass;
	int		ii=0;
	
	hBTNew = CreateUniqueList (128,filePathname);
    
    while (!BT_FIND (hBT,Value,pos,BT_ANY,(LPSTR)&iclass))
    {
    	pos = BT_NEXT;  
    	ExpandText (Value);  
    	_fstrncpy (NewValue,Value,128);
    	BT_PUT (hBTNew,NewValue,(LPSTR)&iclass);
    } 
    BT_CLOSEANDDELETE (&hBT);
{
#if ENABLETRACE
GSSiExitProg (1289);
#endif
	return hBTNew;
}
#if ENABLETRACE
}
#endif
}

COLORREF GetNextUniqueColor (LPLONG pColor)
{
	POINT	p=RectMid (&CurView->DrawRect);
	long	LastColor = *pColor;
	long	Color, ii;
	
	(*pColor)+=UniqueColorInc; 
	if ((long)SetPixel (CurView->hDC,p.x,p.y,*pColor) < 0)
		return *pColor;
	while ((Color=GetPixel (CurView->hDC,p.x,p.y)) != *pColor && *pColor > LastColor)
	{
		LastColor = *pColor;
		(*pColor)++; 
		ii=SetPixel (CurView->hDC,p.x,p.y,*pColor);
	}
	return *pColor;  
}

BOOL BeginThemePCTByArea (void)
#if ENABLETRACE
{GSSiEnterProg (1290);
#endif
{   
	UINT	iclass;    
	long	NextColor;
	
	if (!CurTheme)
{
#if ENABLETRACE
GSSiExitProg (1290);
#endif
		return FALSE;
}
	if (!CurTheme->PCTByArea)
{
#if ENABLETRACE
GSSiExitProg (1290);
#endif
		return FALSE;
}
//	if (Printing || CurTheme->PCTDisplayCycle == DisplayCycle)
	if (CurTheme->PCTDisplayCycle == DisplayCycle)
{
#if ENABLETRACE
GSSiExitProg (1290);
#endif
		return FALSE; 
}
	if (!Printing)
		CurTheme->NumNonMask = 0;  
	SaveDC (CurView->hDC);
    SetDisplayMode (CurView->hDC, GF_TEXTMODE);
  	SelectClipRgn (CurView->hDC,0);
	NextColor = 31; 
	ComputePCTMaskColor = GetNextUniqueColor (&NextColor);
	for (iclass=0;iclass<MAX_THEME_CLASSES;iclass++) 
	{   
		if (!SaveColorsInUse)
			SaveClassColor[iclass] = CurTheme->ClassColor[iclass];
		CurTheme->ClassColor[iclass] = GetNextUniqueColor (&NextColor); 
		if (!Printing)
			CurTheme->ClassCount[iclass] = 0;                      
	}
	SaveColorsInUse = TRUE;
	RestoreDC (CurView->hDC,-1);
{
#if ENABLETRACE
GSSiExitProg (1290);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL ComputeThemePCTByArea (BOOL CloseAll)
#if ENABLETRACE
{GSSiEnterProg (1291);
#endif
{   
	COLORREF	Color;   
	UINT	x,y,iclass;
   	HCURSOR	hcurSave; 
	 
	if (!CurTheme)
{
#if ENABLETRACE
GSSiExitProg (1291);
#endif
		return FALSE;
}
	if (!CurTheme->PCTByArea)
{
#if ENABLETRACE
GSSiExitProg (1291);
#endif
		return FALSE;
}
	if (CloseAll)
	{
		ComputePCTTheme=0;
{
#if ENABLETRACE
GSSiExitProg (1291);
#endif
		return FALSE;           
}
	}
//	if (Printing || CurTheme->PCTDisplayCycle == DisplayCycle)
	if (CurTheme->PCTDisplayCycle == DisplayCycle)
{
#if ENABLETRACE
GSSiExitProg (1291);
#endif
		return FALSE;
}
	
	CurTheme->PCTDisplayCycle = DisplayCycle;
	
	if (!Printing)
	{   
		DisplayPolyOff ();
		if (!CurView->hMaskArea)
		{   
			MaskRect = CurView->DrawRect;
		}
		else
	        DisplayMaskArea(); 
		if (!PrintMsgWnd)
		{	
			hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT)); 
		}
	    SetDisplayMode (CurView->hDC, GF_TEXTMODE);
		y = MaskRect.top;
		while (y <= MaskRect.bottom)
		{
			x = MaskRect.left;
			while (x <= MaskRect.right)
			{   
				Color = GetPixel (CurView->hDC,x,y);
				if (Color != ComputePCTMaskColor)
				{
					CurTheme->NumNonMask++;
					for (iclass=0;iclass<CurTheme->NumClass;iclass++)
					{
						if (Color == CurTheme->ClassColor[iclass])
						{   
							CurTheme->ClassCount[iclass]++;
							break;
						}
					}
				}				
				x++;
			}
			y++;
		}
	}
	for (iclass=0;iclass<MAX_THEME_CLASSES;iclass++) 
		CurTheme->ClassColor[iclass] = SaveClassColor[iclass]; 
	SaveColorsInUse = FALSE; 
	if (!PrintMsgWnd)
	{
		GSSiSetCursor(hcurSave);
	}	
{
#if ENABLETRACE
GSSiExitProg (1291);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
} 

void ClearCompareDC (LPTHEME CurTheme,BOOL Close)
{
	if (CurTheme->CompareAttributes && CurTheme->CompareDC)
	{
		if (Close)
		{
			CurTheme->CompareBitmap = SelectObject (CurTheme->CompareDC,CurTheme->CompareBitmapOld);
			GSSiDeleteObject (&CurTheme->CompareBitmap);
			DeleteDC (CurTheme->CompareDC);
			CurTheme->CompareDC = 0;
		}
		if (pViewports[CurTheme->TargetViewport-1])
			pViewports[CurTheme->TargetViewport-1]->pTheme->CompareDC = 0;
		if (pViewports[CurTheme->DataType-1])
			pViewports[CurTheme->DataType-1]->pTheme->CompareDC = 0;
	}
	return;
}

void CompareViewportsThemeLegend (short From,short FromVPID)	  
{
	RECT	Rect=CurView->DrawRect; 
    short	BorderWidth=0;
	HBITMAP	hBitmap, hBitmapTemp;
	BITMAP	bm;
	HANDLE	hBits;
	LPBYTE	pBits;
	int		lbits;
	RECT	WindowRect;
    
    SaveDC (CurView->hDC);
	SetDisplayMode (CurView->hDC, GF_TEXTMODE);
  	SelectClipRgn (CurView->hDC,0); 
  	if (CurView->BorderPct > 0)
		BorderWidth = -IDNINT ((double)((MaxDimension * 2 * CurView->BorderPct)/100));
	InflateRect (&Rect,BorderWidth,BorderWidth);   
    switch (From)
    {   
    	case 0:
		    FillRectPoly (CurView->hDC,&Rect,CurView->BackGroundColor);
   			CurTheme->HaveVP[0] = FALSE;
    		CurTheme->HaveVP[1] = FALSE;
			ClearCompareDC (CurTheme,TRUE);
			if (CurTheme->CompareAttributes)
			{
				GetWindowRect (CurView->hWnd,&WindowRect);
				CurTheme->CompareDC = CreateCompatibleDC(CurView->hDC); 
				CurTheme->CompareBitmap = CreateCompatibleBitmap (CurView->hDC,WindowRect.right-WindowRect.left+1,WindowRect.bottom-WindowRect.top-1); 
				CurTheme->CompareBitmapOld = SelectObject (CurTheme->CompareDC,CurTheme->CompareBitmap);
				if (pViewports[CurTheme->TargetViewport-1]->pTheme)
					pViewports[CurTheme->TargetViewport-1]->pTheme->CompareDC = CurTheme->CompareDC;
				if (pViewports[CurTheme->DataType-1]->pTheme)
					pViewports[CurTheme->DataType-1]->pTheme->CompareDC = CurTheme->CompareDC;
				SetDisplayMode (CurTheme->CompareDC, GF_TEXTMODE);
			  	SelectClipRgn (CurTheme->CompareDC,0); 
			    FillRectPoly (CurTheme->CompareDC,&Rect,CurView->BackGroundColor);
			}
    		goto Exit; 
    	case 1:
    		if (FromVPID == CurTheme->TargetViewport)
    			CurTheme->HaveVP[0] = TRUE;
    		else if (FromVPID == CurTheme->DataType)
    			CurTheme->HaveVP[1] = TRUE; 
    		goto Exit;
    	case 2: 
    	case 3:
    		goto Exit;
    	case 4: 
    		if (CurTheme->HaveVP[0] && CurTheme->HaveVP[1])
    		{
				RECT	Rect1;
				RECT	Rect2;

				if (!CurTheme->CompareAttributes)
				{
					Rect1 = pViewports[CurTheme->TargetViewport-1]->DrawRect;
					Rect2 = pViewports[CurTheme->DataType-1]->DrawRect;
				}
				else
				{
					LPVIEWPORT	vp1=pViewports[CurTheme->TargetViewport-1];
					LPVIEWPORT	vp2=pViewports[CurTheme->DataType-1];

					if (vp1->pTheme)
						Rect1 = pViewports[vp1->pTheme->TargetViewport-1]->DrawRect;
					if (vp2->pTheme)
						Rect2 = pViewports[vp2->pTheme->TargetViewport-1]->DrawRect;
				}
				{
					RECT	Rect3=pViewports[CurTheme->DisplayViewport-1]->DrawRect;
					UINT	x1,x2,x3,y1,y2,y3, w=Rect1.right - Rect1.left +1, h=Rect1.bottom - Rect1.top + 1;
					COLORREF	DeleteColor=RGB(255,0,0), AddColor=RGB(0,0,255), ChangeColor=0, Color1, Color2, BkColor;
					COLORREF	SaveColor1=GetPixel (CurView->hDC,Rect1.left,Rect1.top);
					COLORREF	SaveColor2=GetPixel (CurView->hDC,Rect1.left+1,Rect1.top);
					COLORREF	SaveColor3=GetPixel (CurView->hDC,Rect1.left+2,Rect1.top);
					HDC		hDCMem;
					int		bytesperpixel,ii;
					LPUSHORT	p2_1,p2_2,p2_3;
					LPLONG		p4_1,p4_2,p4_3;

					SetPixel (CurView->hDC,Rect1.left,Rect1.top,DeleteColor);
					SetPixel (CurView->hDC,Rect1.left+1,Rect1.top,AddColor);
					SetPixel (CurView->hDC,Rect1.left+2,Rect1.top,CurView->BackGroundColor);
					GetWindowRect (CurView->hWnd,&WindowRect);
					hDCMem = CreateCompatibleDC(CurView->hDC); 
					hBitmapTemp = CreateCompatibleBitmap (CurView->hDC,WindowRect.right-WindowRect.left+1,WindowRect.bottom-WindowRect.top-1); 
					GetObject(hBitmapTemp, sizeof(bm), (LPSTR)&bm);
					hBitmap = SelectObject (hDCMem,hBitmapTemp);
					ii=BitBlt(hDCMem, 0, 0, bm.bmWidth,bm.bmHeight,CurView->hDC, 0,0, SRCCOPY);
					hBitmapTemp = SelectObject (hDCMem,hBitmap);
					lbits = bm.bmHeight*bm.bmWidthBytes;
					hBits = GSSiGlobAlloc (0,GMEM_MOVEABLE,lbits);
					pBits = (LPBYTE)GlobalLock (hBits);
					GetBitmapBits(hBitmapTemp,lbits,pBits);
					bytesperpixel = bm.bmBitsPixel / 8;
					switch (bytesperpixel)
					{
					case 2:
						p2_1 = (LPUSHORT)(pBits + (Rect1.top * bm.bmWidthBytes + Rect1.left * bytesperpixel));
						DeleteColor = *p2_1++;
						AddColor = *p2_1++;
						BkColor = *p2_1;
						break;
					case 4:
						p4_1 = (LPLONG)(pBits + (Rect1.top * bm.bmWidthBytes + Rect1.left * bytesperpixel));
						DeleteColor = *p4_1++;
						AddColor = *p4_1++;
						BkColor = *p4_1;
						break;
					}
					SetPixel (CurView->hDC,Rect1.left,Rect1.top,SaveColor1);
					SetPixel (CurView->hDC,Rect1.left+1,Rect1.top,SaveColor2);
					SetPixel (CurView->hDC,Rect1.left+2,Rect1.top,SaveColor3);
					hBitmap = SelectObject (hDCMem,hBitmapTemp);
					if (CurTheme->CompareDC)
					{
						SetDisplayMode (CurTheme->CompareDC, GF_TEXTMODE);
			  			SelectClipRgn (CurTheme->CompareDC,0); 
						ii=BitBlt(hDCMem, 0, 0, bm.bmWidth,bm.bmHeight,CurTheme->CompareDC, 0,0, SRCCOPY);
					}
					else
						ii=BitBlt(hDCMem, 0, 0, bm.bmWidth,bm.bmHeight,CurView->hDC, 0,0, SRCCOPY);
				    FillRectPoly (hDCMem,&Rect,CurView->BackGroundColor);
					hBitmapTemp = SelectObject (hDCMem,hBitmap);
					GetBitmapBits(hBitmapTemp,lbits,pBits);

					y1 = Rect1.top;
					y2 = Rect2.top;
					y3 = Rect3.top;
					while (y1 <= Rect1.bottom)
					{
						x1 = Rect1.left; 
						x2 = Rect2.left; 
						x3 = Rect3.left; 
						switch (bytesperpixel)
						{
						case 2:
							p2_1 = (LPUSHORT)(pBits + (y1 * bm.bmWidthBytes + x1 * bytesperpixel));
							p2_2 = (LPUSHORT)(pBits + (y2 * bm.bmWidthBytes + x2 * bytesperpixel));
							p2_3 = (LPUSHORT)(pBits + (y3 * bm.bmWidthBytes + x3 * bytesperpixel));
							break;
						case 4:
							p4_1 = (LPLONG)(pBits + (y1 * bm.bmWidthBytes + x1 * bytesperpixel));
							p4_2 = (LPLONG)(pBits + (y2 * bm.bmWidthBytes + x2 * bytesperpixel));
							p4_3 = (LPLONG)(pBits + (y3 * bm.bmWidthBytes + x3 * bytesperpixel));
							break;
						}
						while (x1 <= Rect1.right)
						{   
							switch (bytesperpixel)
							{
							case 2:
								if (*p2_1 != *p2_2)
								{
									if (*p2_1 == BkColor)
										*p2_3 = DeleteColor;
									else if (*p2_2 == BkColor)
										*p2_3 = AddColor; 
									else
										*p2_3 = ChangeColor; 
								}
								p2_1++;
								p2_2++;
								p2_3++;
								break;
							case 4:
								if (*p4_1 != *p4_2)
								{
									if (*p4_1 == BkColor)
										*p4_3 = DeleteColor;
									else if (*p4_2 == BkColor)
										*p4_3 = AddColor; 
									else
										*p4_3 = ChangeColor; 
								}
								p4_1++;
								p4_2++;
								p4_3++;
								break;
							}
							x1++;
							x2++;
							x3++;
						}
						y1++; 
						y2++;
						y3++;
					}
  					CurTheme->HaveVP[0] = FALSE;
    				CurTheme->HaveVP[1] = FALSE;
					SetBitmapBits(hBitmapTemp,lbits,pBits);
					hBitmap = SelectObject (hDCMem,hBitmapTemp);
					GSSiGlobUlFree (&hBits);
					ii=BitBlt(CurView->hDC, CurView->DrawRect.left, CurView->DrawRect.top,
							  CurView->DrawRect.right-CurView->DrawRect.left+1,CurView->DrawRect.bottom-CurView->DrawRect.top+1,
							  hDCMem, CurView->DrawRect.left,CurView->DrawRect.top,
							  SRCCOPY);
//					ii=BitBlt(CurView->hDC, 0,0, bm.bmWidth,bm.bmHeight,
//							  hDCMem, 0,0,
//							  SRCCOPY);
					SelectObject (hDCMem,hBitmap);
					GSSiDeleteObject (&hBitmapTemp);
					DeleteDC (hDCMem);
					ClearCompareDC (CurTheme,FALSE);

				}
    		}
    		goto Exit; 
    } 
Exit:
	RestoreDC (CurView->hDC,-1);
	return;
}
   
void Display2DThemeLegend (short From)
#if ENABLETRACE
{GSSiEnterProg (1292);
#endif
{   int		xmargin, ymargin;
	long	h, w;
	int		width, x, y, fHeight, MaxTextWidth, twidth, i, j;
	HBRUSH	BkBrush;
	int		iclass;
	SIZE	txSize;
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
	LPTHEME	pThemeX, pThemeY, SaveTheme=CurTheme;
	short	nClassX, nClassY, VPNumX=CurTheme->DataFileType, VPNumY=CurTheme->DataType;
	short	pos=BT_FIRST, cval; 
	COLORREF	Color;
	THEMEHIGHLIGHTKEY	ThemeHighlightKeyX,ThemeHighlightKeyY;
	THEMEHIGHLIGHTDATA	ThemeHighlightData;    
	double	xinc, yinc;
	RECT	Rect=CurView->DrawRect;  
	HCURSOR	hcurSave;    
	BOOL	Opened1=FALSE, Opened2=FALSE;
    
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
    pThemeX = pViewports[VPNumX]->pTheme;
    pThemeY = pViewports[VPNumY]->pTheme;
    if (!pThemeX || !pThemeY)
    	goto Exit;  
    CurTheme = pThemeX; 
    if (!CurTheme->hHighlightFile)
    {
		if (!OpenThemeHighlightFile (BT_READ))
		{
			CurTheme = SaveTheme;
			goto Exit;
		} 
		Opened1 = TRUE;
	}
    hBTX = CurTheme->hHighlightFile;
    CurTheme = pThemeY;
    if (!CurTheme->hHighlightFile)
    {
		if (!OpenThemeHighlightFile (BT_READ))
		{   
		    CurTheme = pThemeX;
			CloseThemeHighlightFile ();
			CurTheme = SaveTheme;
			goto Exit;
		}
		Opened2 = TRUE;
	}
    hBTY = CurTheme->hHighlightFile;
	CurTheme = SaveTheme;
    nClassX = pThemeX->NumClass;
    nClassY = pThemeY->NumClass; 
    if (!hBTX || !hBTY || !nClassX || !nClassY)
    	goto Exit; 
	hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT)); 
    _fmemset (Counts,0,MAX_THEME_CLASSES*MAX_THEME_CLASSES*sizeof(long));
	while (!BT_FIND (hBTX,(LPSTR)&ThemeHighlightKeyX,pos,BT_ANY,(LPSTR)&ThemeHighlightData))
    {   
    	pos = BT_NEXT; 
    	if (ThemeHighlightKeyX.Class > -1)  
    	{
    		for (iclass=0;iclass<nClassY;iclass++)
    		{   
    			ThemeHighlightKeyY.Class = iclass;
    			ThemeHighlightKeyY.Refno = ThemeHighlightKeyX.Refno;
    			if (!BT_FIND (hBTY,(LPSTR)&ThemeHighlightKeyY,BT_FIRST,BT_EQ,(LPSTR)&ThemeHighlightData)) 
    			{ 
    				Counts[ThemeHighlightKeyX.Class][ThemeHighlightKeyY.Class]++;
    				continue;
    			}
    		}
    	}
    }  
	GSSiSetCursor(hcurSave); 
	if (Opened1)
	{
	    CurTheme = pThemeX;
		CloseThemeHighlightFile (); 
	}
	if (Opened2)
	{
	    CurTheme = pThemeY;
		CloseThemeHighlightFile (); 
	}
	CurTheme = SaveTheme;
    
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
    	 
    	_fstrcpy (Text,pThemeX->ClassBM[i]);
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





BOOL ThemeCommonCode (HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam, HANDLE hSQL)
#if ENABLETRACE
{GSSiEnterProg (1294);
#endif
{
	int	idesc, i, n;
	LPVIEWPORT	SaveView;
	COLORREF	Color;  
	char		Contents[36], str[32];
	
 switch(Message)
   { 
   
//    case GSSI_REINITDIALOG: 12/6/99 causing contents to reset in streetnametheme
    case WM_INITDIALOG: 
    	if (CurTheme->SymNum > 0)
    	{   
       		LPVIEWPORT	SaveView=CurView;
       		
       		if (CurTheme->TargetViewport && CurTheme->ID != GF_BOUNDS_DISPLAY_THEME)
	    		SetCurView (pViewports[CurTheme->TargetViewport-1]);
			GetSymbolName (CurTheme->SymNum,Contents,0,0,0);
			SetCurView (SaveView);
   			SetDlgItemText(hWndDlg,SV_CONTENTS_LIST,Contents);
    	}
    	else 
   			SetDlgItemText(hWndDlg,SV_CONTENTS_LIST,CurTheme->Contents);
case GSSI_REINITDIALOG:
        SendDlgItemMessage (hWndDlg,SV_COLOR_SCHEME,LB_RESETCONTENT,0,0);
		SendDlgItemMessage (hWndDlg,SV_COLOR_SCHEME,LB_ADDSTRING,0,(LPARAM)((LPSTR)"User Defined"));
		SendDlgItemMessage (hWndDlg,SV_COLOR_SCHEME,LB_ADDSTRING,0,(LPARAM)((LPSTR)"Greys"));
		SendDlgItemMessage (hWndDlg,SV_COLOR_SCHEME,LB_ADDSTRING,0,(LPARAM)((LPSTR)"Reds"));
		SendDlgItemMessage (hWndDlg,SV_COLOR_SCHEME,LB_ADDSTRING,0,(LPARAM)((LPSTR)"Greens"));
		SendDlgItemMessage (hWndDlg,SV_COLOR_SCHEME,LB_ADDSTRING,0,(LPARAM)((LPSTR)"Blues"));
		SendDlgItemMessage (hWndDlg,SV_COLOR_SCHEME,LB_ADDSTRING,0,(LPARAM)((LPSTR)"Mixed"));
        i=SendDlgItemMessage (hWndDlg,SV_TARGET,CB_RESETCONTENT,0,0);
		for (i=0;i<*pNumViewports;i++)
		{
			SendDlgItemMessage (hWndDlg,SV_TARGET,CB_ADDSTRING,0,
								(LPARAM)pViewports[i]->Name);
		}
		SendDlgItemMessage (hWndDlg,SV_TARGET,CB_SETCURSEL,
							CurTheme->TargetViewport-1,0);
       	SendDlgItemMessage (hWndDlg,SV_DISPLAY_DIST,BM_SETCHECK,CurTheme->DisplayDistance,0L);
       	SendDlgItemMessage (hWndDlg,SV_DISPLAY_PCT,BM_SETCHECK,CurTheme->DisplayPCT,0L);
		SendDlgItemMessage(hWndDlg, SV_DISPLAY_COUNT, BM_SETCHECK, CurTheme->DisplayCount, 0L);
		SendDlgItemMessage(hWndDlg, SV_DAY_FILTER, BM_SETCHECK, CurTheme->isDayFilter, 0L);
		SendDlgItemMessage(hWndDlg, SV_APPEND_COUNT, BM_SETCHECK, CurTheme->AppendCount, 0L);
       	SendDlgItemMessage (hWndDlg,SV_PCTBYAREA,BM_SETCHECK,CurTheme->PCTByArea,0L);
       	SendDlgItemMessage (hWndDlg,SV_INVERT,BM_SETCHECK,CurTheme->InvertLegend,0L);
       	SendDlgItemMessage (hWndDlg,IDC_FILLROW,BM_SETCHECK,CurTheme->FillRow,0L);
       	SendDlgItemMessage (hWndDlg,SV_FLIP,BM_SETCHECK,CurTheme->FlipLegend,0L);
       	SendDlgItemMessage (hWndDlg,SV_FACTOR,BM_SETCHECK,CurTheme->FactorLegend,0L);   
       	SendDlgItemMessage (hWndDlg,SV_HIDENULLCLASSES,BM_SETCHECK,CurTheme->HideNullClasses,0L);   
       	SendDlgItemMessage (hWndDlg,SV_COMPRESSNULLCLASSES,BM_SETCHECK,CurTheme->CompressNullClasses,0L);   
       	SendDlgItemMessage (hWndDlg,IDC_CENTERTEXT,BM_SETCHECK,CurTheme->CenterText,0L); 
       	ltoa (IDNINT(CurTheme->ShowValAZ * RADtoDEG),str,10);
       	SetDlgItemText (hWndDlg,IDC_SHOWVALROT,str);
       	
       	SendDlgItemMessage (hWndDlg,IDC_SETCOLOR,BM_SETCHECK,(!CurTheme->NotSetColor),0L);   
       	
{
#if ENABLETRACE
GSSiExitProg (1294);
#endif
		return FALSE;
}
	   
 	case WM_DESTROY: 
	 	InitDlgPrompts (0);    
		DestroyFieldList ();
 		return FALSE;
 		 
    case WM_COMMAND:
         switch(LOWORD(wParam))
           {
            case SV_TARGET:
              switch(HIWORD(wParam))
              {
               case CBN_DBLCLK:
               case CBN_SELCHANGE:
               {
               		LPVIEWPORT	SaveView;
               		short	n,i;
            
		    		if (CurTheme->TargetViewport)
		    		{
			    		SaveView = CurView; 
			    		SetCurView (pViewports[CurTheme->TargetViewport-1]);
						RemoveThemeFromVP (CurView,CurTheme);
			    		SetCurView (SaveView);
			    	}  
			    }
		           break;  
		      }    		  		    
              break;
            
            case SV_CONTENTS_LIST:
               { 
              	switch(HIWORD(wParam))
                {
	                 case CBN_DROPDOWN:
	                 {
	                 	short	idesc, iparent, i, j;
                	    LPSTR	pTAGList;  
                	    HANDLE	hPar=GSSiGlobAlloc (1015,GMEM_MOVEABLE,4096);
                	    short	nPar=0;
                	    LPSHORT	pPar, pPar2; 
                	    char	str[64], SymbolName[34];
                	    BOOL	IsPar;  
                	    LPVIEWPORT	SaveVP=CurView;
	                 	 
		                CurTheme->TargetViewport=SendDlgItemMessage(hWndDlg,SV_TARGET,
											       CB_GETCURSEL,0,0)+1;
			            SendDlgItemMessage (hWndDlg,SV_CONTENTS_LIST,CB_RESETCONTENT,0,0); 
			            if (!CurTheme->TargetViewport) break;
			            
			            if (CurTheme->ID != GF_BOUNDS_DISPLAY_THEME)
			            { 
				            SetViewport (CurTheme->TargetViewport);
				            SelectVisList (FALSE);
				        }
		        		//GetVisList (hWndDlg,0,-SV_CONTENTS_LIST,0,-1);
	 	                //SendDlgItemMessage (hWndDlg,SV_CONTENTS_LIST,CB_ADDSTRING,0,(LPARAM)((LPSTR) "(ALL)"));
				    	for (idesc=1;idesc<3201;idesc++) 
				    	{
							if (CurView->CurVisType[idesc])
							{   char	SymbolName[34];
							
								GetSymbolName (idesc,SymbolName,&iparent,0,&IsPar);
								if (SymbolName[0] && !IsPar)
								{
			 	                	SendDlgItemMessage (hWndDlg,SV_CONTENTS_LIST,CB_ADDSTRING,0,(LPARAM)((LPSTR) SymbolName));
			 	                	pPar = (LPSHORT)GlobalLock (hPar);
			 	                	for (i=0;i<nPar;i++,pPar++)
			 	                		if (iparent == *pPar)
			 	                			goto GotPar;
			 	                	*pPar = iparent;
			 	                	nPar++;
			 	            GotPar: GlobalUnlock (hPar);  
			 	                }
		 	                }
						}     
 	                	pPar = (LPSHORT)GlobalLock (hPar);
 	                	for (i=0;i<nPar;i++,pPar++)
 	                	{
							GetSymbolName (*pPar,SymbolName,&iparent,0,0);
							if (SymbolName[0])
							{   
								sprintf (str,"(%s)",SymbolName);
			 	                SendDlgItemMessage (hWndDlg,SV_CONTENTS_LIST,CB_ADDSTRING,0,(LPARAM)((LPSTR)str));
		 	                	pPar2 = (LPSHORT)GlobalLock (hPar);
		 	                	for (j=0;j<nPar;j++,pPar2++)
		 	                		if (iparent == *pPar2)
		 	                			goto GotPar2;
		 	                	*pPar2 = iparent;
		 	                	nPar++;
		 	            GotPar2: GlobalUnlock (hPar); 
		 	                }
		 	            } 
 	            		GSSiGlobUlFree (&hPar);
                	    
                	    if (CurView->hTAGList)
                	    {
							pTAGList = GlobalLock(CurView->hTAGList);
							while (*pTAGList)
							{   
								char	tag[16];
								
								sprintf (tag,"-%s",pTAGList);
		 	               		SendDlgItemMessage (hWndDlg,SV_CONTENTS_LIST,CB_ADDSTRING,0,(LPARAM)((LPSTR)tag));
								pTAGList+=10;
							}
							GlobalUnlock (CurView->hTAGList);
						}
                        SetCurView (SaveVP);
					}
				    break; 
				}
                break;
               }
            
           	case IDC_BGCOLOR: 
           		 Color = CurTheme->BGColor; 
            	 if (GetColor(hWndDlg,&Color))
            	 	CurTheme->BGColor = Color;
           		 break;
                 
           	case IDC_VALBGCOLOR:
           		 Color = CurTheme->IBBGColor;  
            	 if (GetColor(hWndDlg,&Color))
            	 	CurTheme->IBBGColor = Color;
           		 break;
                 
           	case IDC_TITBGCOLOR:  
           		 Color = CurTheme->TitleBoxBG;
            	 if (GetColor(hWndDlg,&Color))
            	 	CurTheme->TitleBoxBG = Color;
           		 break;
                 
            case IDC_TITLEFONT:
            	 CurTheme->TitleFont.lfHeight = 10;
            	 GetFont (hWndDlg, &CurTheme->TitleFont, &CurTheme->TitleTextColor,0,0);
            	 break;
            	 
            case IDC_VALUEFONT:  
            	 CurTheme->ClassFont1.lfHeight = 10;
            	 GetFont (hWndDlg, &CurTheme->ClassFont1, &CurTheme->IBTextColor[0],0,0); 
            	 break;
            	      
            case IDC_PCTFONT:  
            	 CurTheme->ClassFont2.lfHeight = 10;
            	 GetFont (hWndDlg, &CurTheme->ClassFont2, &CurTheme->IBTextColor[1],0,0); 
            	 break;
            	      
            case IDC_SHOWVALFONT:  
            	 GetFont (hWndDlg, &CurTheme->ShowValueFont, &CurTheme->ShowValueTextColor,&CurTheme->ShowValueShadowColor,0); 
				 if (CurTheme->ShowValueFont.lfStrikeOut)
					 CurTheme->UseShadowColor = TRUE;
            	 break;
            	      
            case IDC_SETSQL:      
            {    
                 HANDLE hMem;
                 LPSTR  lpStr;
                 
                 hMem = GSSiGlobAlloc (1016,GHND,4096);
                 lpStr = GlobalLock (hMem); 
                 GetDlgItemText (hWndDlg,IDC_SQL,lpStr,1024);
                 if (GetSQLWhereClause (hWndDlg, hSQL, lpStr))
                 	SetDlgItemText (hWndDlg,IDC_SQL,lpStr);    
                 GSSiGlobUlFree (&hMem);
                 break;
            }
            case IDC_LOAD_THEME:  
            {
            	HFILE	Fid;  
            	char	File[128]=""; 
				OFSTRUCTGM	OFStruct;
            	LPTHEME	NewTheme; 
            	short	i,n;
            	LPVIEWPORT	SaveView;
            	
             	 _fstrcpy (gszFilter,"GeoMaster Themes(*.thm)|*.THM|");
				 if (GetFileName3(hWndDlg,File,0,IDS_FILETHM))
				 {   
					 Fid = GSSiOpenFile (File,(LPOFSTRUCTGM)&OFStruct,OF_READ); 
					 ReadObject (&Fid,TRUE,&NewTheme,0); 
					 GSSiClose (Fid);

					if (CurView->pTheme)
					{   
						CurTheme = CurView->pTheme;
			    		SaveView = CurView;
			    		SetCurView (pViewports[CurTheme->TargetViewport-1]);
						RemoveThemeFromVP (CurView,CurTheme);
			    		SetCurView (SaveView);
			   	    	CloseObject (CurView->pTheme);
			   	    } 
                     CurTheme = NewTheme;
                     CurView->pTheme = NewTheme;
				 	 if (CurTheme->TargetViewport > *pNumViewports) 
				 	 	CurTheme->TargetViewport = 1; 
				 	 if (pViewports[CurTheme->TargetViewport-1]->NumThemes)
	    		 	 	pViewports[CurTheme->TargetViewport-1]->NumThemes--;
					 CurTheme->DisplayViewport = CurView->ID;
					 CurTheme->IsActive=FALSE;
	                 PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L); 
				 }
			}
            	 break;
            	 
            case IDC_SAVE_THEME:  
 
            case IDOK:  
            	 GetDlgItemText (hWndDlg,SV_DATABASE_LIST,CurTheme->DataFile,sizeof(CurTheme->DataFile)-1);
                 CurTheme->TargetViewport=SendDlgItemMessage(hWndDlg,SV_TARGET,
									       CB_GETCURSEL,0,0)+1;  
	             //CurTheme->DisplayViewport = CurView->ID;
	    		 SaveView = CurView;
            	 CurTheme->DisplayDistance = SendDlgItemMessage (hWndDlg,SV_DISPLAY_DIST,BM_GETCHECK,0,0L);  
				 CurTheme->DisplayCount = SendDlgItemMessage(hWndDlg, SV_DISPLAY_COUNT, BM_GETCHECK, 0, 0L);
				 CurTheme->isDayFilter = SendDlgItemMessage(hWndDlg, SV_DAY_FILTER, BM_GETCHECK, 0, 0L);
				 CurTheme->AppendCount = SendDlgItemMessage(hWndDlg, SV_APPEND_COUNT, BM_GETCHECK, 0, 0L);
            	 CurTheme->DisplayPCT = SendDlgItemMessage (hWndDlg,SV_DISPLAY_PCT,BM_GETCHECK,0,0L);  
            	 CurTheme->PCTByArea = SendDlgItemMessage (hWndDlg,SV_PCTBYAREA,BM_GETCHECK,0,0L);  
            	 CurTheme->InvertLegend = SendDlgItemMessage (hWndDlg,SV_INVERT,BM_GETCHECK,0,0L);  
            	 CurTheme->FillRow = SendDlgItemMessage (hWndDlg,IDC_FILLROW,BM_GETCHECK,0,0L);  
            	 CurTheme->FlipLegend = SendDlgItemMessage (hWndDlg,SV_FLIP,BM_GETCHECK,0,0L);  
            	 CurTheme->FactorLegend = SendDlgItemMessage (hWndDlg,SV_FACTOR,BM_GETCHECK,0,0L);  
            	 CurTheme->NotSetColor = !SendDlgItemMessage (hWndDlg,IDC_SETCOLOR,BM_GETCHECK,0,0L);  
            	 CurTheme->HideNullClasses = SendDlgItemMessage (hWndDlg,SV_HIDENULLCLASSES,BM_GETCHECK,0,0L);  
            	 CurTheme->CompressNullClasses = SendDlgItemMessage (hWndDlg,SV_COMPRESSNULLCLASSES,BM_GETCHECK,0,0L);  
            	 CurTheme->CenterText = SendDlgItemMessage (hWndDlg,IDC_CENTERTEXT,BM_GETCHECK,0,0L); 
            	 GetDlgItemText (hWndDlg,IDC_SHOWVALROT,str,16); 
            	 CurTheme->ShowValAZ = atof (str) * DEGtoRAD;
 	    		 SetCurView (pViewports[CurTheme->TargetViewport-1]);
	    		 for (i=0,n=0;i<CurView->NumThemes;i++)
	    		 { 
	    			if (CurView->pThemes[i]==CurTheme)
	    				n++;
	    		 }
	    		 if (!n)
					 AddThemeToVP (CurView,CurTheme);
	    		 SetCurView (SaveView);
				 GetDlgItemText (hWndDlg,SV_CONTENTS_LIST,Contents,34);
				 SetThemeContents (CurTheme,Contents);
                 if (CurTheme->NumClass)
                 	CurTheme->IsActive = TRUE;
{
#if ENABLETRACE
GSSiExitProg (1294);
#endif
                 return FALSE; 
}
                 
		    default:
{
#if ENABLETRACE
GSSiExitProg (1294);
#endif
		        return FALSE;
}
		

           }
         break; 

    default:
{
#if ENABLETRACE
GSSiExitProg (1294);
#endif
        return FALSE;
}
   }  
{
#if ENABLETRACE
GSSiExitProg (1294);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
}  

int	 GetThemeClass (LPTHEME CTheme,LPSTR Value,BOOL UseSelectionList)
{
	int	iclass=0,ClassNo2,st;
	LPTHEME	SaveTheme = CurTheme;

	CurTheme = CTheme;
	switch (CurTheme->ID)
	{
		case GF_SINGLE_VALUE_THEME: 
		{
			double	ValD = atof (Value);
			double	ClassMin, ClassMax;

			ValD = Round (ValD,CurTheme->RoundTo);
			
			if (ValD > CurTheme->YLimit)
				ValD = CurTheme->YLimit;
			for (iclass=0;iclass<CurTheme->NumClass;iclass++)
			{	
				GetClassMinMax (iclass,&ClassMin,&ClassMax);
								
				if (ValD>=ClassMin && ValD<=ClassMax)
				{   
					if (CurTheme->ClassStatus[iclass])
					{
						CurTheme = SaveTheme;
						return -(iclass+1);  
					}
					CurTheme = SaveTheme;
					return iclass+1;
				}
			}
			iclass = 0;
		}
		break;
		case GF_SINGLE_NONNUM_VALUE_THEME:
		{
			char	KeyVal[256];

			if (!CurTheme->hScatterFile && CurTheme->ScatterFile[0])  
				CurTheme->hScatterFile = BT_OPEN (CurTheme->ScatterFile,0, BT_READ, 0);
			_fstrncpy (KeyVal,Value,GetBTKeyLen(CurTheme->hScatterFile));
			if (!BT_FIND (CurTheme->hScatterFile,KeyVal,BT_FIRST,BT_EQ,(LPSTR)&iclass))
			{   
				if (!iclass || iclass > MAX_THEME_CLASSES)
					break;
	GotClass:
				if (CurTheme->ClassStatus[iclass-1])
					iclass = -iclass;
				break;
			}
			else
			{
				_fstrncpy (KeyVal,"[",GetBTKeyLen(CurTheme->hScatterFile));  
				st = BT_FIND (CurTheme->hScatterFile,KeyVal,BT_FIRST,BT_GE,(LPSTR)&ClassNo2);
				while (!st)
				{
					if (*KeyVal == '[')
					{   
						ExpandText (KeyVal);
						if (!_fstrcmp (KeyVal,Value))
						{
							iclass = ClassNo2;
							goto GotClass;
						}
					}
					else
						st = 1;
					st = BT_FIND (CurTheme->hScatterFile,KeyVal,BT_NEXT,BT_ANY,(LPSTR)&ClassNo2);
				}
			}
		}
			break;
		default:
			break;
	}
	CurTheme = SaveTheme;
	return iclass;
}

BOOL SetThemeContents (LPTHEME CurTheme,LPSTR Contents) 
{   
	LPVIEWPORT	SaveView = CurView;

	if (Contents[0] == '-')
	{
		_fstrncpy (CurTheme->Contents,Contents,9);
		CurTheme->Contents[9]=0;
		CurTheme->SymNum = -1;  
	}
	else if (Contents[0] == '$')
	{
		_fstrncpy (CurTheme->Contents,Contents,9);
		CurTheme->Contents[9]=0;
		CurTheme->SymNum = -2;  
	}
	else if (!_fstricmp (Contents,"(ALL)"))
	{
		_fstrncpy (CurTheme->Contents,Contents,9);
		CurTheme->SymNum = 0;  
	}
	else
	{   
		LPSTR	lpContents=Contents;
		if (CurTheme->ID != GF_BOUNDS_DISPLAY_THEME)
			SetViewport (CurTheme->TargetViewport);
		if (*lpContents == '(')
		{
		 	LPSTR	lpEnd;
							 	
		 	lpEnd = _fstrchr (Contents,0);
		 	lpEnd--;
		 	if (*lpEnd == ')')
		 	{
		 		lpContents++;
		 		*lpEnd = 0;
		 	} 
		}
		*CurTheme->Contents	=0; 
		{
		    LPVISLIST	SaveVis=CurVis; 
		    HANDLE		handle;
				    
		    handle=GSSiGlobAlloc (1017,GHND,sizeof(VISLIST));
			CurVis = (LPVISLIST)GlobalLock (handle); 
		    InitVis ();
			CurTheme->SymNum=GetSymbolNum (lpContents);
			CurVis = SaveVis;
			GSSiGlobUlFree (&handle); 
		}
	}
	SetCurView (SaveView); 
	return TRUE;
}        

BOOL GetValsFromTable(HWND hWnd)
#if ENABLETRACE
{GSSiEnterProg (1295);
#endif
{
	
	GSSiGlobFree (&hVALUELIST); 
	if (hWndSELVAL)
{
#if ENABLETRACE
GSSiExitProg (1295);
#endif
		return FALSE;
}
	lpfnSELECTVALUESMsgProc = MakeProcInstance((DLGPROC)SELECTVALUESMsgProc, hInst);
	CreateDialog(hInst, (LPSTR)"SELECTVALUES", hWnd, lpfnSELECTVALUESMsgProc);
//	DialogBox(hInst, (LPSTR)"SELECTVALUES", hWnd, lpfnSELECTVALUESMsgProc);
//	FreeProcInstance(lpfnSELECTVALUESMsgProc);
{
#if ENABLETRACE
GSSiExitProg (1295);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

void DestroySELECTVALUES (void)
#if ENABLETRACE
{GSSiEnterProg (1296);
#endif
{   
	GSSiGlobFree (&hVALUELIST);
	if (!hWndSELVAL)
{
#if ENABLETRACE
GSSiExitProg (1296);
#endif
		return;
}
	DestroyWindow(hWndSELVAL);
	hWndSELVAL = 0;
	FreeProcInstance((DLGPROC) lpfnSELECTVALUESMsgProc);
{
#if ENABLETRACE
GSSiExitProg (1296);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}






BOOL SetProcessStatusTitle (LPSTR Title)
{   
	USHORT	i;
	
	if (!Title)
	{
		for (i=0;i<MAX_VIEWPORTS;i++)
			*ViewportStatusMessages[i]=0;   
		return TRUE;
	}
	_fstrcpy (ViewportStatusMessages[CurView->ID-1],Title);
	return TRUE;
} 

void DisplayDataPassMessage (void)
{   
	if (*ViewportStatusMessages[CurView->ID-1])
		CreateProcessStatusWindow (CurView->hWnd,ViewportStatusMessages[CurView->ID-1]); 
	return;
}
   
void RemoveDataPassMessage (void)
{   
	DestroyProcessStatusWindow ();
	return;
}

BOOL ThemeNeedsDataPass (BOOL PixelThemesOnly,BOOL forceDataPass)
#if ENABLETRACE
{GSSiEnterProg (1319);
#endif
{	BTVARDESC	BTVar[3];
	int	i;
	OFSTRUCTGM	OFStruct;
	char		str[128];
    
    if (CurTheme->DisplayViewport && CurTheme->DisplayViewport <= *pNumViewports)
    	CurTheme->VPDisplayed = pViewports[CurTheme->DisplayViewport-1]->Active;
	if (!CurTheme->VPDisplayed || !CurTheme->IsActive ||
		 CurTheme->ID == PF_COORD_DISPLAY || CurTheme->ID == PF_BOUNDS_DISPLAY)
{
#if ENABLETRACE
GSSiExitProg (1319);
#endif
		return (FALSE);
}   
	if ((PixelThemesOnly && CurTheme->DataType != THEMEDATATYPE_PIXEL) || 
		(!PixelThemesOnly && CurTheme->DataType == THEMEDATATYPE_PIXEL))
{
#if ENABLETRACE
GSSiExitProg (1319);
#endif
		return (FALSE);
}   
	if (CurTheme->PCTByArea && CurTheme->PCTDisplayCycle != DisplayCycle)
		ComputePCTTheme = CurTheme;	                                     
	else
		ComputePCTTheme = 0;	                                     
	if (!forceDataPass && !CurTheme->WantDataPass) 
	{
	    if (CurTheme->ID != GF_GRAPHICS_FUNCTION_THEME && CurTheme->DataFileType != MSACCESS_DATAFILE)
	    {
	    	OpenThemeDataFile ();
            if (CurTheme->hThemeDB && GetDBType (CurTheme->hThemeDB) != GMTEXT_DATAFILE)
            	 GetDBFieldInfo (&CurTheme->Field,CurTheme->hThemeDB);
			if (CurTheme->SymNum > 0)
			{
				GSSiGlobFree (&CurTheme->hVisList);
				CurTheme->hVisList = SetThemeVisList (CurTheme->SymNum);   
				if (CurTheme->DisplayViewport ==  19)
				{
					BOOL rtn = ItemProcessedByTheme ("",376);
					ii = rtn;
				}
			}
			CloseThemeDataFile(FALSE);
	    }
{
#if ENABLETRACE
GSSiExitProg (1319);
#endif
		return FALSE;
}   
	}
	if ((!forceDataPass && !CurTheme->Recompute && CurTheme->NumClass) && CurTheme->ID != GF_OFFSETAREA_THEME)
{
#if ENABLETRACE
GSSiExitProg (1319);
#endif
		return (FALSE);
}
	if (*CurTheme->RefValChar)
	{
		_fstrcpy (str,CurTheme->RefValChar);
		ExpandText (str);  
		CurTheme->RefValDbl = atof (str);
	}
	CurTheme->Xmin = DBL_MAX;
	CurTheme->Xmax = -DBL_MAX;
	if (CurTheme->ZeroBased)
		CurTheme->Ymin = 0;
	else
		CurTheme->Ymin = DBL_MAX;
	CurTheme->Ymax = -DBL_MAX;
	CurTheme->NumVals = 0;
	if (CurTheme->SymNum > 0)
	{
		GSSiGlobFree (&CurTheme->hVisList);
		CurTheme->hVisList = SetThemeVisList (CurTheme->SymNum);   
	}
	if (CurTheme->ID == GF_HOTSPOT_THEME)
	{   
		LPHOTSPOTDATA	pHSData = &CurTheme->HotSpotData;
		double dw;//, shrinkfactor=1;                             
		LPVIEWPORT	SaveVP = CurView; 
		long	GridSize;
	    double  BASEX[4], BASEY[4], HSX[4], HSY[4];  
	    double	BaseDToWinD=1, d1, d2;
	    DPOINT	dp1, dp2,HotSpotPoint[2];
	    float	RSQMIN;
		
		GSSiGlobFree (&pHSData->hGrid);
		GSSiGlobFree (&pHSData->hMask);
		CloseTRANS2 (&pHSData->hTranBaseToHotSpot);
        
		SetViewport (CurTheme->TargetViewport);   
	    if (CurView->DisplayInParent && CurView->Parent> 0) 
	    	CurView = pViewports[CurView->Parent-1]; 
/*        dp1.x = CurView->WBounds.xmn;
        dp1.y = CurView->WBounds.ymn;
        dp2.x = CurView->WBounds.xmx;
        dp2.y = CurView->WBounds.ymx;
        d1 = ldistp (dp1,dp2);
        dp1.x = CurView->DrawRect.left;
        dp1.y = CurView->DrawRect.right;
        dp2.x = CurView->DrawRect.top;
        dp2.y = CurView->DrawRect.bottom;
        d2 = ldistp (dp1,dp2);
        if (d1)
        	BaseDToWinD = d2/d1;
        
		dw = BaseDToWinD * pHSData->Radius;
		if (dw > 128)
			shrinkfactor = dw / 128;
		pHSData->MaskWidth = max (2,IDNINT (dw * shrinkfactor));*/   
		if (!Printing)
		if (CurView->DrawRect.right - CurView->DrawRect.left > CurView->DrawRect.bottom - CurView->DrawRect.top)  
		{
			pHSData->GridWidth = MAXHOTSPOTDIMENSION;
			pHSData->GridHeight = pHSData->GridWidth * (double)(CurView->DrawRect.bottom - CurView->DrawRect.top)/(double)(CurView->DrawRect.right - CurView->DrawRect.left);
		}
		else
		{
			pHSData->GridHeight = MAXHOTSPOTDIMENSION;
			pHSData->GridWidth = pHSData->GridHeight * (double)(CurView->DrawRect.right - CurView->DrawRect.left)/(double)(CurView->DrawRect.bottom - CurView->DrawRect.top);
		}
//		pHSData->GridWidth = ((CurView->DrawRect.right - CurView->DrawRect.left) * shrinkfactor)/pHSData->Granularity;
//		pHSData->GridHeight = ((CurView->DrawRect.bottom - CurView->DrawRect.top) * shrinkfactor)/pHSData->Granularity;  
		GridSize =  (long)pHSData->GridWidth * (long)pHSData->GridHeight * 4; 
		pHSData->hGrid = GSSiGlobAlloc (1023,GHND,GridSize);
		pHSData->TotalIncidents=0; 
		pHSData->SecondsRepresented=GetSecondsInSample();
		HSX[0]=HSX[1]=0;
		HSX[2]=HSX[3]=pHSData->GridWidth-1;
		HSY[0]=HSY[3]=0;
		HSY[1]=HSY[2]=pHSData->GridHeight-1;
		BASEX[0]=BASEX[1]=CurView->NewBounds.xmn;
		BASEX[2]=BASEX[3]=CurView->NewBounds.xmx;
		BASEY[0]=BASEY[3]=CurView->NewBounds.ymn;
		BASEY[1]=BASEY[2]=CurView->NewBounds.ymx;
		pHSData->hTranBaseToHotSpot = STRAN2 (1657,BASEX,BASEY,HSX,HSY,4,&RSQMIN,1,0);  
		dp1 = dp2 = MinMaxMidPointD (&CurView->NewBounds); 
		_fstrcpy (str,CurTheme->ClassDefValSQL);
		ExpandText (str);
		ExpandText (str);
		pHSData->Radius = atof (str); 
		if (!pHSData->Radius)
			pHSData->Radius = 500;
		dp2.x += pHSData->Radius; 
		HotSpotPoint[0] = TranPoint (&dp1,pHSData->hTranBaseToHotSpot);
		HotSpotPoint[1] = TranPoint (&dp2,pHSData->hTranBaseToHotSpot);  
		pHSData->MaskWidth = max (1,ldistp (HotSpotPoint[0],HotSpotPoint[1]));
		pHSData->hMask = GSSiGlobAlloc (1024,GMEM_MOVEABLE,(long)pHSData->MaskWidth * (long)pHSData->MaskWidth * 4); 
		CurTheme->Ymin = 0;
		CurTheme->Ymax = 0;
		SetupHotSpotMask (pHSData->MaskWidth,pHSData->hMask,pHSData->DecayOpt);  
    	OpenThemeDataFile ();
		SetCurView (SaveVP);
	}
	else if (CurTheme->ID == GF_OFFSETAREA_THEME)
	{  
		if (CurTheme->ScatterFile[0])
			GSSiRemove (CurTheme->ScatterFile);
		GSSiGetTempFileName (0,"gmt",0,(LPSTR)CurTheme->ScatterFile);
		CurTheme->FidAreas = GSSiOpenFile (CurTheme->ScatterFile,NULL,OF_CREATE);
		CurTheme->NumAreas = 0;
	   	OpenThemeDataFile ();
	}
	else
	{
		BT_CLOSEANDDELETE (&CurTheme->hScatterFile); 
		if (CurTheme->DisplayScatterDiagram || CurTheme->ClassType == 2)
		{
			if (CurTheme->ScatterFile[0])
				GSSiRemove (CurTheme->ScatterFile);
			GSSiGetTempFileName (0,"gmt",0,(LPSTR)CurTheme->ScatterFile);
		
			BTVar[0].BT_VARTYP=BT_REAL;
			BTVar[0].BT_VARLEN=8;
			BTVar[0].BT_VAROFF=0;
			BTVar[1].BT_VARTYP=BT_REAL;
			BTVar[1].BT_VARLEN=8;
			BTVar[1].BT_VAROFF=8;
			BTVar[2].BT_VARTYP=BT_INTEGER;
			BTVar[2].BT_VARLEN=4;
			BTVar[2].BT_VAROFF=16;
			BT_CREATE (CurTheme->ScatterFile, 16, FALSE, 3, 1, (LPBTVARDESC)BTVar, FALSE, 0, 0, FALSE);
			CurTheme->hScatterFile = BT_OPEN (CurTheme->ScatterFile,0, BT_WRITE, 0);  
		}
	    if (CurTheme->DataFileType != MSACCESS_DATAFILE)
	    {
	    	OpenThemeDataFile ();
            if (CurTheme->hThemeDB && GetDBType (CurTheme->hThemeDB) != GMTEXT_DATAFILE)
            	 GetDBFieldInfo (&CurTheme->Field,CurTheme->hThemeDB);
	    }
    }
{
#if ENABLETRACE
GSSiExitProg (1319);
#endif
	return(TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL SetVisibilityFromTheme (LPSTR Name,LPSTR SymPrefix,short setopt)
#if ENABLETRACE
{GSSiEnterProg (1325);
#endif
{   
	HFILE		Fid;  
	OFSTRUCTGM	OFStruct;
	LPTHEME		pTheme, SaveTheme=CurTheme; 
	short		pos=BT_FIRST, idesc;
	char		SymName[128],Data[128];
	
	Fid = GSSiOpenFile (Name,(LPOFSTRUCTGM)&OFStruct,OF_READ);  
	if (Fid == HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (1325);
#endif
		return FALSE;
}
	ReadObject (&Fid,FALSE,&pTheme,0); 
	GSSiClose (Fid);
	if (pTheme->ScatterFile)
	{
		pTheme->hScatterFile = BT_OPEN (pTheme->ScatterFile,0, BT_READ, 0); 
		while (!BT_FIND (pTheme->hScatterFile,SymName,pos,BT_ANY,Data))
		{
			char	SymName2[128];

			sprintf (SymName2,"%s%s",SymPrefix,SymName);
			pos = BT_NEXT;
			if ((idesc = GetSymbolNum (SymName2)))
			{
				if (GetVisibility(idesc) != setopt)
					ToggleVisibility (idesc);
			}
		} 
	}	
	CloseObject (pTheme);  
	CurTheme = SaveTheme;
	Pickability = FALSE;
    TurnOffAutoVis (TRUE);		   
{
#if ENABLETRACE
GSSiExitProg (1325);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
} 
 
BOOL GetSymListFromTheme (LPSTR Name,LPSTR SymList,int maxlen)
#if ENABLETRACE
{GSSiEnterProg (1325);
#endif
{   
	HFILE		Fid;  
	OFSTRUCTGM	OFStruct;
	LPTHEME		pTheme, SaveTheme=CurTheme; 
	short		pos=BT_FIRST;
	char		SymName[128],Data[128];
	int			Totlen=0,len;
	BOOL		rtn=FALSE;
	
	*SymList = 0;
	Fid = GSSiOpenFile (Name,(LPOFSTRUCTGM)&OFStruct,OF_READ);  
	if (Fid == HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (1325);
#endif
		return FALSE;
}
	if (ReadObject (&Fid,FALSE,&pTheme,0))
	{
		rtn = TRUE;
		GSSiClose (Fid);
		if (pTheme->ScatterFile)
		{
			pTheme->hScatterFile = BT_OPEN (pTheme->ScatterFile,0, BT_READ, 0); 
			while (!BT_FIND (pTheme->hScatterFile,SymName,pos,BT_ANY,Data))
			{
				char	SymName2[128];

				len = strlen (SymName) + 3;
				pos = BT_NEXT;
				if (Totlen + len > maxlen)
				{
					rtn = FALSE;
					break;
				}
				sprintf (&SymList[Totlen],"'%s',",SymName);
				Totlen += len;
			} 
		}
		Totlen = max (0,Totlen-1);
		SymList[Totlen] = 0;
		CloseObject (pTheme);  
		CurTheme = SaveTheme;
	}
	else
		GSSiClose (Fid);
{
#if ENABLETRACE
GSSiExitProg (1325);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}  

void BoundsDisplayTheme (BOOL Init)
#if ENABLETRACE
{GSSiEnterProg (1329);
#endif
{
	short		SaveNum, SaveDVI, itheme;    
	static		BOOL	InBDT=FALSE;
	
    if (InBDT)
{
#if ENABLETRACE
GSSiExitProg (1329);
#endif
    	return;
}
    InBDT = TRUE;
	for (itheme=0;itheme<CurView->NumThemes;itheme++)
	{
		CurTheme = CurView->pThemes[itheme];
		BoundsDisplayTheme2 (Init);
	}
	InBDT = FALSE;
{
#if ENABLETRACE
GSSiExitProg (1329);
#endif
	return;
}
#if ENABLETRACE
}
#endif
} 

void ClearChildVP (short VPID)
{
	UINT	i;
	
	for (i=0;i<*pNumViewports;i++)
	{
		if (pViewports[i]->Parent == VPID) 
		{
			SaveDC (pViewports[i]->hDC);
			SetDisplayMode (pViewports[i]->hDC, GF_TEXTMODE);
		  	SelectClipRgn (pViewports[i]->hDC,0);
		    FillRectPoly (pViewports[i]->hDC,&pViewports[i]->Rect,WindowColor);
	       	DisplayTAGs2 (pViewports[i]->hDC,2,0);
	       	ClearChildVP (pViewports[i]->ID);  
	       	RestoreDC (pViewports[i]->hDC,-1);
	    }
	}
	return;
}

void BoundsDisplayTheme2 (BOOL Init)
#if ENABLETRACE
{GSSiEnterProg (1330);
#endif
{
	LPVIEWPORT	SaveVP=CurView;
	DPOINT		WPoint;  
	BOOL		HaveRef, ClearVP=FALSE;
	LPSAVESCREEN	pSaveScreen;
	LPBOUNDSDISPLAY lpBoundsDisplay;
	MNMXCORD	WBounds, PKBounds;
	POINT		BegPoint, EndPoint;
	HPEN		ArrowPen; 
	long		SaveDispC; 
	static		LPTHEME		InBDTheme=0; 
	LPTHEME		SaveIBDT; 
	int	RegionType;
	DWORD	Err;

	if (CurTheme->ID != GF_BOUNDS_DISPLAY_THEME ||
		!CurTheme->IsActive|| 
		!CurTheme->VPDisplayed)
{
#if ENABLETRACE
GSSiExitProg (1330);
#endif
		return; 
}
	if (CurTheme == InBDTheme)
{
#if ENABLETRACE
GSSiExitProg (1330);
#endif
		return; 
}
	lpBoundsDisplay = (LPBOUNDSDISPLAY)&CurTheme->ClassBM; 
	if (Init) 
	{
		lpBoundsDisplay->SavedScreen = 0;
		lpBoundsDisplay->CurAreaRef = LONG_MAX;
{
#if ENABLETRACE
GSSiExitProg (1330);
#endif
		return;
}
	}   
//	SetCurView (pViewports[CurTheme->TargetViewport-1]); //tempdebu
	SetViewport (CurTheme->TargetViewport);
    if (CurView->DisplayInParent && CurView->Parent > 0) 
    	CurView = pViewports[CurView->Parent-1]; 
	WBounds = CurView->NewBounds;
//	SetCurView (pViewports[CurTheme->DisplayViewport-1]); //tempdebu
	SetViewport (CurTheme->DisplayViewport);
	if (CurTheme->NumVals == DisplayCycle && !CurView->LinkedTo)  
	{
		SetCurView (SaveVP);  
{
#if ENABLETRACE
GSSiExitProg (1330);
#endif
		return;
}
	}
	SaveIBDT = InBDTheme;  
	InBDTheme = CurTheme;
	CurTheme->NumVals = DisplayCycle;
	CurView->BoundsDisplayVP = CurTheme->TargetViewport;   
	if (*lpBoundsDisplay->BoundsGlobal) 
	{   
		HANDLE	hStr=GSSiGlobAlloc (1026,GMEM_MOVEABLE,256);
		LPSTR	pStr=GlobalLock (hStr); 
		BOOL	err;
		
		_fstrcpy (pStr,lpBoundsDisplay->BoundsGlobal);
		ExpandText (pStr); 
		CurView->NewBounds = atobounds (pStr,&err); 
		SetScaleAndMidpointFromBounds (CurView);
		GSSiGlobUlFree (&hStr);
		if (err)
			ClearVP = TRUE; 
		else
			SetBoundsRect2 (CurView->DrawRect,CurView->hDC);
		DestroySavedScreen (&CurView->Bitmap,CurView->BitmapID);
   	}
	else if (!CurView->hTranVPToBase) 
	{
		if (lpBoundsDisplay->VPBoundsOpt != 1) 
			GetVisBounds (&CurView->NewBounds,CurView->hDC);   //MOD:020313
		SetBoundsRect2 (CurView->DrawRect,CurView->hDC); 
	}  
	if (!CurView->Bitmap) 
	{     
		lpBoundsDisplay->SavedScreen = 0;
		lpBoundsDisplay->CurAreaRef = LONG_MAX;
	}
	HaveRef = FALSE;
	if (*lpBoundsDisplay->CurAreaRefGlobal && !lpBoundsDisplay->IgnoreClear)
	{   
		long		WantRef;
		LPVIEWPORT	SaveView=CurView;
		LPTHEME		SaveTheme=CurTheme;     
		LPTHEME		pTheme;
			
		WantRef = GetGlobalLVal3 (lpBoundsDisplay->CurAreaRefGlobal,LONG_MAX);
		if (WantRef != LONG_MAX)
		{
			if (PickByRefno(WantRef,0,0,-100))
			{
			    SetConfig (PickList[0].ConfigID);
			    SetViewport (PickList[0].ViewID);
				pTheme = AddTheme (GF_SAVEPOLYPARTS_THEME);
				CurView->PassID = 4;
				ProcessSelectedTheme = CurView->NumThemes;
				ProcessPickedItem (0,FALSE);        		
				ProcessSelectedTheme = 0;
				DeleteTheme (pTheme);  
				GetSavedPolys ();  
					
		   		if (hSavePoly)
				{   LPMNMXCORD lpRect;
					HPDPOINT	lpDpoint; 
					BOOL	rtn;  
					DPOINT	AreaPoints[5]; 
					LPINT	pPolyParts; 
					int		nPParts;
					short	nareas; 
					long	nPnts;
						
		            BoundsToPoints (&WBounds,AreaPoints,0);   
		            AreaPoints[4] = AreaPoints[0];
                    if (hSavePolyParts)
                    {
                    	pPolyParts = (LPINT)GlobalLock (hSavePolyParts);
                    	nareas = *pPolyParts++; 
                    }
                    else
                    { 
                        nareas=1;
                        nPParts = nSavePoly;  
                        pPolyParts = &nPParts;
                    }
		            lpRect = (LPMNMXCORD) GlobalLock (hSavePoly);  
		            HaveRef = BoundsInBounds (&WBounds,lpRect,0);
		            lpRect++;
		            lpDpoint = (HPDPOINT) lpRect; 
		            while (nareas-- && !HaveRef)
		            {   
		            	
			            HaveRef = PolyInArea (0,*pPolyParts,lpDpoint,0,5,AreaPoints,1,0,InclusionOpt,0);
			            lpDpoint += (*pPolyParts+1);
			            pPolyParts++;
			        }
		            GSSiGlobUlFree (&hSavePoly);
		            GSSiGlobUlFree (&hSavePolyParts);
		        } 
		        DestroySavedPolys ();
			}
		} 
		CurTheme = SaveTheme;
		SetCurView (SaveView);
	}
	if (CurTheme->SymNum && !HaveRef)
	{   
		long	SaveDisplayCycle = DisplayCycle;
		LPTHEME	SaveTheme=CurTheme;     
		short	SaveMaxPick = MaxPick, SavePP = PickPerim;     
		BOOL	SaveDH = DisableHalt; 
//PICKDATA	pd;			
		WPoint =  MinMaxMidPointD (&WBounds);
		UseUserPickAp =FALSE;
		SystemPickAp = 0;// 3/20/2007 -5;	
		MaxPick = 1;// 10;  
		DisableHalt=TRUE;
		PickPerim = 0; 
		ClearMaskArea ();
		{
			short SaveNumVehicles=NumVehicles; 
			BOOL	SaveIPL=IgnorePrevLayers;
			HANDLE	hSaveVis = GSSiGlobAlloc (0,GMEM_MOVEABLE,sizeof(VISLIST));
			LPVISLIST	pSaveVis = GlobalLock (hSaveVis);
			BOOL	UseSymbolWidth;

			SelectVisList (TRUE);
			*pSaveVis = *CurVis;
			NumVehicles = 0;
			memset (CurVis->VisBits,0,sizeof(CurVis->VisBits));
			ToggleVisibility (CurTheme->SymNum);
			SetParentVisibility (CurTheme->SymNum,TRUE,-1); 
			//IgnorePrevLayers = TRUE;
//			HaveRef = PickItems (CurView->hWnd,WPoint);
		    SetPickAp(&UseSymbolWidth);  
			HaveRef = PickItems2 (CurView->hWnd,WPoint,FALSE,TRUE,FALSE);
			NumVehicles = SaveNumVehicles;
			IgnorePrevLayers = SaveIPL;
			*CurVis = *pSaveVis;
			GSSiGlobUlFree (&hSaveVis);
		} 
//if(NumPicked)
//	pd=PickList[NumPicked-1];		
		PickPerim = SavePP; 
		DisableHalt=SaveDH;
		MaxPick = SaveMaxPick;
		UseUserPickAp =TRUE;
		CurTheme = SaveTheme;
	}
	if (HaveRef)
	{	
		long	SaveDisplayCycle = DisplayCycle;
		LPTHEME	SaveTheme=CurTheme; 
			    
    	while (NumPicked--)
    	{   
    		if (SymInContents (PickList[NumPicked].Desc,CurTheme->SymNum,CurTheme->Contents))
    		{
	    		ProcessPickedItem (NumPicked,FALSE);
				CurTheme = SaveTheme;	
	    		ProcessText (CurTheme->Title);
				CurTheme = SaveTheme;	
	    		if (PickList[NumPicked].Refno != lpBoundsDisplay->CurAreaRef)  
	    		{
		    		short SaveNumVP=NumViewportsToDisplay,n;
		    		BOOL	DisplayVP[MAX_VIEWPORTS]; 
		    		HANDLE	hMem = GSSiGlobAlloc (1027,GMEM_MOVEABLE,1024);
		    		LPSTR	CacheFile=GlobalLock (hMem), SaveScreenFile=CacheFile+256; 
		    		LPOFSTRUCTGM	pOFStruct=(LPOFSTRUCTGM) (SaveScreenFile + 256);  
		    		long	SaveSize;
			    		 
		    		lpBoundsDisplay->CurAreaRef = PickList[NumPicked].Refno;
		    		n=SaveNumVP;
		    		while (n--)
		    			DisplayVP[n] = pViewports[n]->Display;
		    		if (!Printing)
		    		{   
//			    		_fstrcpy (SaveScreenFile,"[%DL]savescrn");  
//			    		ExpandText (SaveScreenFile);
			    		GetTempDir (SaveScreenFile);  
			    		_fstrcat (SaveScreenFile,"\\gmsavesc");
			    		GSSiMakeDir (SaveScreenFile,&Err);
			    		sprintf (_fstrchr(SaveScreenFile,0),"\\%lx.scr",lpBoundsDisplay->CurAreaRef);
			        	DestroySavedScreen (&CurView->Bitmap,CurView->BitmapID);
		                if (GetCacheFile (CacheFile, SaveScreenFile,FALSE, 0))
		                {   
		                	long	UpdateTime;
							MNMXCORD	Bounds;

		                	if ((CurView->Bitmap = ReadSavedScreen (CacheFile,&UpdateTime,&Bounds)))
		                	{
		                		pSaveScreen = (LPSAVESCREEN)GlobalLock (CurView->Bitmap);
	                			pSaveScreen->pVP = CurView; 
	                			CurView->BitmapID = pSaveScreen->ID;
		                		if (CurView->Rect.left != pSaveScreen->Rect.left ||
		                			CurView->Rect.right != pSaveScreen->Rect.right ||
		                			CurView->Rect.top != pSaveScreen->Rect.top ||
		                			CurView->Rect.bottom != pSaveScreen->Rect.bottom ||
									CurView->WBounds.xmn != Bounds.xmn ||
									CurView->WBounds.xmx != Bounds.xmx ||
									CurView->WBounds.ymn != Bounds.ymn ||
									CurView->WBounds.ymx != Bounds.ymx)
		                		{
		                			GlobalUnlock (CurView->Bitmap);
		                			DestroySavedScreen (&CurView->Bitmap,CurView->BitmapID); 
		                			RemoveCacheFile (SaveScreenFile);
		                		}
		                		else
		                		{
		                			GlobalUnlock (CurView->Bitmap);
		                		}
		                	}
		                } 
		            } 
		            PKBounds = PickList[NumPicked].Rect;
					InflateBounds (&PKBounds,CurTheme->RefValDbl);
	                if (Printing || !CurView->Bitmap)
	                {   
	                	BOOL SaveOZA = OutlineZoomArea, SaveGMA = GetMaskArea, SaveMOL = MaskOffsetLine, SaveDH=DisableHalt; 
	                	LPVIEWPORT	SaveVP2;
		                	
						OutlineZoomArea = CurTheme->ZeroIsMissing;   
						MaskOffsetLine = CurTheme->AddCommas;
		            	if (MaskOffsetLine || OutlineZoomArea)
		            		GetMaskArea = TRUE;
						SetMaskArea(NumPicked,0,1);
						DisableHalt = TRUE; 
						DisplayCycle--; 
						SaveVP2 = CurView; 
					    CurView->CurZoomAreaRef = 0;
			    		ZoomToRect(PKBounds,TRUE); 
			    		SetCurView (SaveVP2);
			    		DisableHalt = SaveDH; 
			    		if (!Printing)
			    		{ 
							DestroySavedScreen (&CurView->Bitmap,CurView->BitmapID);
			                CurView->Bitmap = SaveScreen2 (CurView->hWnd,CurView->hDC, CurView->Rect,CurView,&CurView->BitmapID);
			                CurView->BitmapRect = CurView->Rect;
			                WriteSavedScreen (SaveScreenFile,CurView->Bitmap,&CurView->WBounds);
		                	GetCacheFile (CacheFile, SaveScreenFile,TRUE, 0); 
		                	GSSiRemove (SaveScreenFile);
		                }
	                	OutlineZoomArea = SaveOZA;
						GetMaskArea = SaveGMA;  
						MaskOffsetLine = SaveMOL;
		            }
		            else 
		            {
		            	BOOL SaveDisplay=Display;
			            	
		            	Display = FALSE;
						DisplayCycle--;
					    CurView->CurZoomAreaRef = 0;
		            	ZoomToRect(PKBounds,2);
		            	Display = SaveDisplay;                    
		            	DisplayTAGs2 (CurView->hDC,1,0); 
		            }
	                CurView->BitmapRect = CurView->Rect;
		    		NumViewportsToDisplay = SaveNumVP;
		    		n=SaveNumVP;
		    		while (n--)
		    			pViewports[n]->Display = DisplayVP[n]; 
		    		NumPicked = 0; 
		    		GSSiGlobUlFree (&hMem);
		    	}
	    	}
		}
		DisplayCycle = SaveDisplayCycle;
	}
	else if (CurTheme->RefValDbl < 0)
	{	
		long	SaveDisplayCycle = DisplayCycle; 
   		short SaveNumVP=NumViewportsToDisplay,n;
   		BOOL	DisplayVP[MAX_VIEWPORTS]; 
		
		n=SaveNumVP;
		while (n--)
			DisplayVP[n] = pViewports[n]->Display; 
		if (CurTheme->RefValDbl < -1000000)
			GetVisBounds (&PKBounds,CurView->hDC);
		else 
		{			
			if (CurView->Bitmap && BoundsInBounds (&WBounds,&CurView->NewBounds,0)) 
				PKBounds = CurView->NewBounds; 
			else
			{
				DestroySavedScreen (&CurView->Bitmap,CurView->BitmapID);
	        	PKBounds = WBounds;
				InflateBounds (&PKBounds,-CurTheme->RefValDbl);
			}
		}
        if (Printing || !CurView->Bitmap)
        {   
        	BOOL SaveOZA = OutlineZoomArea, SaveGMA = GetMaskArea, SaveMOL = MaskOffsetLine; 
        	LPVIEWPORT	SaveVP2;
		                	
			OutlineZoomArea = CurTheme->ZeroIsMissing;   
			MaskOffsetLine = CurTheme->AddCommas;
        	if (MaskOffsetLine || OutlineZoomArea)
        		GetMaskArea = TRUE;
			SetMaskArea(NumPicked,0,1);
			DisableHalt = TRUE; 
			DisplayCycle--; 
			SaveVP2 = CurView; 
		    CurView->CurZoomAreaRef = 0;
    		ZoomToRect(PKBounds,TRUE); 
    		SetCurView (SaveVP2);
    		DisableHalt = FALSE; 
    		if (!Printing)
    		{ 
				DestroySavedScreen (&CurView->Bitmap,CurView->BitmapID);
                CurView->Bitmap = SaveScreen2 (CurView->hWnd,CurView->hDC, CurView->Rect,CurView,&CurView->BitmapID);
                CurView->BitmapRect = CurView->Rect;
/*                WriteSavedScreen (SaveScreenFile,CurView->Bitmap);
            	GetCacheFile (CacheFile, SaveScreenFile,TRUE, 0); 
            	GSSiRemove (SaveScreenFile);*/
            } 
        	OutlineZoomArea = SaveOZA;
			GetMaskArea = SaveGMA; 
			MaskOffsetLine = SaveMOL;
        }
        else 
        {
        	BOOL SaveDisplay=Display;
			            	
        	Display = FALSE;
			DisplayCycle--;
		    CurView->CurZoomAreaRef = 0;
        	ZoomToRect(PKBounds,2);
        	Display = SaveDisplay;                    
        	DisplayTAGs2 (CurView->hDC,1,0); 
        }
        CurView->BitmapRect = CurView->Rect;
		NumViewportsToDisplay = SaveNumVP;
		n=SaveNumVP;
		while (n--)
			pViewports[n]->Display = DisplayVP[n]; 
		NumPicked = 0; 
		DisplayCycle = SaveDisplayCycle;
    }
	else if  (CurTheme->SymNum || *lpBoundsDisplay->CurAreaRefGlobal)  
		ClearVP = TRUE; 
	if (lpBoundsDisplay->IgnoreClear)
		ClearVP = FALSE;
	SaveDC (CurView->hDC);
	SetDisplayMode (CurView->hDC, GF_TEXTMODE);
	if (ClearVP)
	{   
		RECT	Rect=CurView->Rect;
			
	  	SelectClipRgn (CurView->hDC,0);
		InflateRect (&Rect,1,1);
		if (*lpBoundsDisplay->CurAreaRefGlobal)
			SetGlobalValueLong (lpBoundsDisplay->CurAreaRefGlobal,LONG_MAX);
	    FillRectPoly (CurView->hDC,&Rect,WindowColor);
       	DisplayTAGs2 (CurView->hDC,2,0);
       	ClearChildVP (CurView->ID); 
	}
    GSSiDeleteObject(&CurView->hRgn);
	CurView->hRgn = CreateVPRgn(FALSE,FALSE);
  	RegionType = SelectVPClipRgn (CurView->hRgn);
  	GSSiDeleteObject(&CurView->hRgn);
	if (!ClearVP && (PrintingToMF || RegionType != NULLREGION))
	{
		LPVIEWPORT	SaveVP = CurView;
		DPOINT	BoundsPoly[5];
		short	i;

		SetViewport (CurTheme->TargetViewport);
		RectToDPoints (&CurView->ScreenRect,BoundsPoly);
		for (i=0;i<4;i++)
		{
			BoundsPoly[i] = TranPoint (&BoundsPoly[i],CurView->hTranScreenToVP);
			BoundsPoly[i] = TranPoint (&BoundsPoly[i],CurView->hTranVPToBase);
		}
		BoundsPoly[4] = BoundsPoly[0];
		CurView = SaveVP;
		if (!InOpenMap)
			ShowZoomArea (CurView->hDC,BoundsPoly,
					               &lpBoundsDisplay->SavedScreen,
					               &lpBoundsDisplay->SavedRect,
								   lpBoundsDisplay->BoxPoints);

	}
	RestoreDC (CurView->hDC,-1);  
	SetCurView (SaveVP);  
	InBDTheme = SaveIBDT;
{
#if ENABLETRACE
GSSiExitProg (1330);
#endif
	return;
}
#if ENABLETRACE
}
#endif
} 

BOOL GetDifferenceValue (LPTHEME pDiffTheme,long ValueColor,LPSTR Value)
{   
	short	pos=BT_FIRST;
	int		Color;
	
	pDiffTheme->hScatterFile = BT_OPEN (pDiffTheme->ScatterFile,0, BT_READ, 0); 
	while (!BT_FIND (pDiffTheme->hScatterFile,Value,pos,BT_ANY,(LPSTR)&Color))
	{   
		if (Color == ValueColor)
		{   
			BT_CLOSE (pDiffTheme->hScatterFile);
			pDiffTheme->hScatterFile = 0;
			return TRUE;
		}
		pos = BT_NEXT;
	}
	*Value = 0;
	BT_CLOSE (pDiffTheme->hScatterFile);
	pDiffTheme->hScatterFile = 0;
	return FALSE;
}

BOOL GetNextPointInAp (LPPOINT pPoint,int iPickAP,LPBOOL pFirst,LPRECT pRect)
{   
	static	short	Ring;  
	static	double	NextAZ;  
	static	POINT	MidPoint;
	double	AZInc;
	POINT	NewPt;
	
	if (*pFirst)
	{
		*pFirst = FALSE; 
		Ring = 1;
		NextAZ = 0;  
		MidPoint = *pPoint;
	} 
	do
	{
		if (NextAZ >= 2 * PY)
		{
			Ring++;
			NextAZ = 0;
		} 
		if (Ring > iPickAP)
			return FALSE;
		NewPt = newpt (MidPoint, NextAZ,Ring);
		AZInc = 1.0/(2 * PY * Ring);
		NextAZ += AZInc; 
	} 
	while (!PtInRect (pRect,NewPt));
	*pPoint = NewPt;
	return TRUE;
}

BOOL GetValueDifference (POINT Point,LPHANDLE phBox,LPVIEWPORT *ppVP,LPLONG pValueColor1,LPLONG pValueColor2)
{   
	LPSAVESCREEN	pSaveScreen;
	COLORREF Color;
	POINT	Point1,Point2, ScreenPoint=Point, OrigPoint;
	char	str[512], Value1[256], Value2[256]; 
	short	iPickAP, inc, i, XInc, YInc;  
	LPTHEME	pDiffTheme, SaveTheme=CurTheme;    
	BOOL	First;
	HDC		hDC=CurView->hDC;
	LPVIEWPORT	SaveVP=CurView;
    
    if (!CurView->pTheme)
    	return FALSE;
	if (CurView->pTheme->CompareDC)
		hDC = CurView->pTheme->CompareDC;
	else
		return FALSE;
	SaveDC (CurView->hDC);
    SetDisplayMode (CurView->hDC, GF_TEXTMODE);
  	SelectClipRgn (CurView->hDC,0);
    CurTheme = CurView->pTheme;
	CurView = pViewports[pViewports[CurTheme->TargetViewport-1]->pTheme->TargetViewport-1];
	Point = OrigPoint = CurView->LastCursorPos;
    SetDisplayMode (hDC, GF_TEXTMODE);
  	SelectClipRgn (hDC,0);
    iPickAP = SetPickAp(0)/2;
    First = TRUE;   
    do
    {
	    Color  = GetPixel (hDC,Point.x,Point.y);
    }
    while (Color == CurView->BackGroundColor && GetNextPointInAp (&Point,iPickAP,&First,&CurView->DrawRect));
	if (Color == CurView->BackGroundColor) 
		goto Exit;
    pDiffTheme = pViewports[CurTheme->TargetViewport-1]->pTheme;
	if (pDiffTheme->ID == GF_SINGLE_NONNUM_VALUE_THEME && !pDiffTheme->NumDesiredClass)
    	goto HaveTheme;
	goto Exit;
HaveTheme:  
	CurView = SaveVP;
	RemoveLinkedCursors ();
	Point1 = pViewports[pViewports[CurTheme->TargetViewport-1]->pTheme->TargetViewport-1]->LastCursorPos;
	Point2 = pViewports[pViewports[CurTheme->DataType-1]->pTheme->TargetViewport-1]->LastCursorPos;
	XInc = Point.x - OrigPoint.x;
	YInc = Point.y - OrigPoint.y;
	Point1.x += XInc;
	Point1.y += YInc;  
	Point2.x += XInc;
	Point2.y += YInc;  
    *pValueColor1 = GetPixel (hDC,Point1.x,Point1.y);	
    *pValueColor2 = GetPixel (hDC,Point2.x,Point2.y);
//    ScreenPoint = Point;
//    ClientToScreen (CurView->hWnd,(LPPOINT)&ScreenPoint);
//    SetCursorPosGM (ScreenPoint.x,ScreenPoint.y,TRUE);
    GetDifferenceValue (pDiffTheme,*pValueColor1,Value1);
    GetDifferenceValue (pDiffTheme,*pValueColor2,Value2);
    sprintf (str,"%s\r\n%s",Value1,Value2);	
//    Point.y -= 10;
	*phBox = YellowTextBox (CurView->hWnd,str,ScreenPoint,0,0,FALSE,0);  
	if (*phBox)
	{
		pSaveScreen = (LPSAVESCREEN)GlobalLock (*phBox);
		pSaveScreen->UserID = 0;
        GlobalUnlock (*phBox); 
    }
	if (ppVP)
		*ppVP = CurView;
Exit:
	CurTheme = SaveTheme;
	CurView = SaveVP;
	RestoreDC (CurView->hDC,-1);
	return FALSE;
} 
