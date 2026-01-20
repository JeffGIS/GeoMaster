#include "graphint.h"
#include "extrndb.h"
#include "gmextern.h"

BOOL InProfile=FALSE;
static	LPVIEWPORT	LastProfileLocVP = 0;

 
void SmoothProfile (HPDPOINT pProfile,long np)
{   
	long	n = max(0, CurTheme->ProfileSmoothOption);
	long	i,j;
	double	y, w,totw; 
	
	if (n && np)
	{
		HANDLE	h2=GSSiGlobAlloc(GAIDNO 0,GMEM_MOVEABLE,sizeof(DPOINT)*np);
		HPDPOINT	pProfile2=(HPDPOINT)GlobalLock (h2);
		
		hmemmove ((HPSTR)pProfile2,(HPSTR)pProfile,sizeof(DPOINT)*np);
		for (i=0;i<np;i++)
		{ 
			y=0;
			totw = 0;
			for (j=i-n;j<=i+n;j++) 
			{   
				w = 1.0/(double)(labs (i-j) + 1);
				y += w * pProfile2[min(np-1,max(0,j))].y; 
				totw += w;
			}    
			pProfile[i].y = y/totw;                 
			
		}
		GSSiGlobUlFree (&h2);
	}			
	return;
}  

BOOL ProfileAndCrossSectionSettings (BOOL SetDefault)
{
	LPTHEME	SaveTheme = CurTheme; 
	BOOL	rtn=FALSE;
				
	if (CurView->pTheme && CurView->pTheme->ID == GF_PROFILE_THEME)
	{
		CurTheme = CurView->pTheme; 
	}
	else
		return FALSE;
	if (SetDefault)
	{  
		if (CurTheme->CrossSectionSpacing <= 0) 
		{   
			CurTheme->DistanceBetweenProfilePoints = 1 * FTM;
			CurTheme->CrossSectionSpacing = 25.0 * FTM;
			CurTheme->CrossSectionWidth[0] = CurTheme->CrossSectionWidth[1] = 500.0 * FTM; 
			rtn=TRUE;
		}
	}
	else
	{
		rtn = EditTheme (CurView->hWnd,0); 
	}		
	CurTheme = SaveTheme;
	return rtn;
}

BOOL CreateCrossSection (short vpid,short opt) //opt=0(Clear xsection),1=draw xsection
{
	LPVIEWPORT	pSaveVP=CurView;  
	LPTHEME		pSaveTheme=CurTheme;
	double	CrossSectionLength=100; 
	DPOINT	Points[2];
	HPEN	hRedPen,hOldPen;
	
	SetViewport (vpid);
	
	if (CurView->pTheme && CurView->pTheme->ID == GF_PROFILE_THEME)
	{
		Points[0] = CurView->ProfileCrossSection[0] = dnewpt (CurView->LastProfilePoint,CurView->LastProfileAZ+HALFPI,CurTheme->CrossSectionWidth[0]);
		Points[1] = CurView->ProfileCrossSection[1] = dnewpt (CurView->LastProfilePoint,CurView->LastProfileAZ+HALFPI,-CurTheme->CrossSectionWidth[1]);
		if (CurView->pTheme && CurView->pTheme->ID == GF_PROFILE_THEME)
		{   
			SetViewport (CurView->pTheme->TargetViewport);
			RemoveLinkedCursors ();
			SaveDC (CurView->hDC);
			SetDisplayMode (CurView->hDC, GF_SCREENMODE);  
		    GSSiDeleteObject(&CurView->hRgn);
		    CurView->hRgn = CreateVPRgn(FALSE,FALSE);
		    SelectClipRgn (CurView->hDC,CurView->hRgn);
		    GSSiDeleteObject(&CurView->hRgn);    
			if (CurView->Bitmap && EqualRect(&CurView->Rect,&CurView->BitmapRect))
				RestoreScreen2 (CurView->hDC,CurView->Bitmap,CurView->BitmapID,FALSE);
	        DestroySavedScreen (&CurView->Bitmap,CurView->BitmapID);
	        if (opt)
	        {
				CurView->Bitmap = SaveScreen2 (CurView->hWnd,CurView->hDC, CurView->Rect,CurView,&CurView->BitmapID);
				CurView->BitmapRect = CurView->Rect;  
				hRedPen = CreatePen (PS_SOLID,(int)(2*DeviceToScreenFactor()),RGB(255,0,0));
				hOldPen = SelectObject(CurView->hDC, hRedPen);
				GWPolylineD (CurView->hDC,Points,2,0);  
				SelectObject (CurView->hDC,hOldPen);   
				GSSiDeleteObject (&hRedPen); 
			} 
			RestoreDC (CurView->hDC,-1);
		}  
	}
	CurTheme = pSaveTheme;
	CurView = pSaveVP;
	return TRUE;
} 

BOOL ZoomToProfile(LPVIEWPORT pVP)
{
	char cmd[256];
	DPOINT BP, EP;
	BOOL rtn = FALSE;
	double dist;

	if (pVP->ProfileInCrossSection)
	{
		HANDLE hPnts = GSSiGlobAlloc(GAIDNO 1811,GMEM_MOVEABLE, 2 * sizeof(DPOINT));
		LPDPOINT pPoints = GlobalLock(hPnts);

		BP = pVP->ProfileCrossSection[0];
		EP = pVP->ProfileCrossSection[1];
		pPoints[0] = BP;
		pPoints[1] = EP;
		dist = GetPolyLengthD(pPoints, 2);
		GlobalUnlock(hPnts);
		sprintf(cmd, "$VP(SETVAL,Primary Viewport,ROTATION,$MACRO([%%DL]macros\\rotatetohorv.txt,$AZM(%f %f,%f %f),H))", BP.x, BP.y, EP.x, EP.y);
		ProcessText(cmd);
		SetCurView(pViewports[0]);
		ZoomToPolyPoints(hPnts, 2,  0.02, TRUE);
		GSSiGlobFree(&hPnts);
		CurView = pVP;
		rtn = TRUE;
	}
	else if (pVP->hProfileRoute[0])
	{
		HPDPOINT	pRoute = (HPDPOINT)GlobalLock(pVP->hProfileRoute[0]);
		int nRoutePoints = pVP->nProfileRoute[0];
		dist = GetPolyLengthD(pRoute, nRoutePoints);
		BP = pRoute[0];
		EP = pRoute[nRoutePoints - 1];
		GlobalUnlock(pVP->hProfileRoute[0]);
		sprintf(cmd, "$VP(SETVAL,Primary Viewport,ROTATION,$MACRO([%%DL]macros\\rotatetohorv.txt,$AZM(%f %f,%f %f),H))", BP.x, BP.y, EP.x, EP.y);
		ProcessText(cmd);
		SetCurView(pViewports[0]);
		ZoomToPolyPoints(pVP->hProfileRoute[0], pVP->nProfileRoute[0], 0.02, TRUE);
		CurView = pVP;
		rtn = TRUE;
	}
	return rtn;
}
BOOL CreateNextCrossSection (int Direction)
{   
	BOOL		rtn=FALSE;

	if (CurView->pTheme && CurView->pTheme->ID == GF_PROFILE_THEME)
	{
		HPDPOINT	pRoute=(HPDPOINT)GlobalLock (CurView->hProfileRouteSave);
		double		Dist = CurView->LastProfileDist + Direction * CurView->pTheme->CrossSectionSpacing;
	
		if (Dist < 0 || Dist > GetPolyLengthD (pRoute,CurView->nProfileRoutePointsSave))
			goto Exit;			
		CurView->LastProfilePoint = PointAtDistOnPoly (pRoute,CurView->nProfileRoutePointsSave,Dist,&CurView->LastProfileAZ,NULL);
		CurView->LastProfileDist = Dist;  
		rtn = TRUE;  
Exit:
		GlobalUnlock (CurView->hProfileRouteSave);
	}
	return rtn; 
} 

HANDLE OpenProfileDataFile (LPMNMXCORD Bounds)
{   
	char	File[256]="[%PROFILEFILE]";  
	HANDLE	hDB = 0; 
	LPGWDHEADER	lpGWDHead; 
	LPPROFILEFILEDATA	pData;
	long	Offset;
	short	Seq;
	
	ExpandText (File);
	if (GSSiLength (File) < 32)
		return 0;     
	hDB = OpenGWDatabase (File,BT_READ);
	if (!hDB)
		return 0;
	if (Bounds)
	{	
	    lpGWDHead = (LPGWDHEADER)GlobalLock (hDB); 
		pData = (LPPROFILEFILEDATA)&lpGWDHead->GWDData; 
		while (!BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&Seq,BT_NEXT,BT_ANY,(LPSTR)&Offset))
		{ 
			FillGWDData (lpGWDHead,Offset); 
			if (pData->BPZ > 0)
			{  
				Bounds->ymn = min (Bounds->ymn,pData->BPZ);
				Bounds->ymx = max (Bounds->ymx,pData->BPZ);
			}
			if (pData->EPZ > 0)
			{
				Bounds->ymn = min (Bounds->ymn,pData->EPZ);
				Bounds->ymx = max (Bounds->ymx,pData->EPZ);
			}
		} 
		GlobalUnlock (hDB); 
	}
	return hDB;
}

BOOL PickProfileRectangles (DPOINT PickPointBase,int PickAp,LPDOUBLE pNearDist)
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
	   		HANDLE hDB = OpenProfileDataFile (NULL);     
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

int	GetScaleBarUnits (int VPID)
{
	int	units=0,i;
	LPVIEWPORT	pSaveVP=CurView;

	if (SetViewport (VPID))
	{
		for (i=0;i<CurView->NumThemes;i++)
			if (CurView->pThemes[i]->ID == GF_DISTANCE_THEME)
			{
				if (VPIsActive(CurView->pThemes[i]->DisplayViewport))
				{
					units = abs (CurView->pThemes[i]->ValConv);
				}
			}
	}
	CurView = pSaveVP;
	return units;
}

BOOL GetProfileX (LPDPOINT pDPoint,long nRoutePoints,HPDPOINT pRoutePoints,LPDPOINT pPointOnRoute,LPDOUBLE pOffDist,LPDOUBLE pX)
{
	if (!GetPerpendicularOffsetToPoly (pDPoint,nRoutePoints,pRoutePoints,pPointOnRoute,pOffDist,pX,0))
	  return FALSE;
	if (CurTheme->ProfileAlignmentOption)
	{
		LPVIEWPORT	SaveVP=CurView;
		DPOINT	WPoint, ScreenPoint;

		SetViewport (CurView->pTheme->TargetViewport);
		WPoint = *pDPoint;
		ScreenPoint = BasePtToScreenPtD (&WPoint);
		*pX = ScreenPoint.x;
		CurView = SaveVP;
	}
	return TRUE;
}
double ProfileDistConv(double dist,double HorToVertFactor,double az)
{
	double d, daz1, daz2;

	daz1 = fabs (DeltaAZ (0,az));
	daz2 = fabs (DeltaAZ (PY,az));
	d = dist * (1.0 - (1.0 -  HorToVertFactor) * min(daz1,daz2) / HALFPI);
	return d;
}

HPEN CreateTexturePen (int iWidth,LPSTR TextureFile)
{
	LOGBRUSH	lb;
	HDIB32	hDIB;
	BITMAPINFOHEADER DibInfo;	
	LPBITMAPINFOHEADER    lpbi;
	LPSTR pImage,pImage2;
	HANDLE	hBMP;
	ULONG_PTR	pBMP;
	int	ln;
	HPEN	hPen;

	hDIB = LoadDIB32 (TextureFile,FALSE, 0);
	if (!hDIB)
		hPen = CreatePen (PS_SOLID,iWidth,0);
	else
	{
		GetBitmapInfoFromHandle (&DibInfo,hDIB);
		hBMP = GSSiGlobAlloc(GAIDNO 0,GMEM_MOVEABLE,sizeof(BITMAPINFO));
		lpbi = GlobalLock (hBMP);
		*lpbi = DibInfo;
		if (lpbi->biSizeImage == 0)
		{
			lpbi->biSizeImage = ((((lpbi->biWidth * (DWORD)lpbi->biBitCount) + 31) & ~31) >> 3)
				 * lpbi->biHeight;
		}
		ln = lpbi->biSize + lpbi->biSizeImage;
		GlobalUnlock(hBMP);
		hBMP = GSSiGlobalReAlloc (0,hBMP, ln, GMEM_MOVEABLE);
		lpbi = GlobalLock (hBMP);
		pImage = (LPSTR) lpbi + (lpbi->biSize+lpbi->biClrUsed*sizeof(COLORREF));
		pImage2 = FreeImage_GetBits(hDIB);
		memmove (pImage,pImage2,lpbi->biSizeImage);
		GlobalUnlock (hBMP);
		DestroyDIB32 (hDIB,FALSE);
		pBMP = (ULONG_PTR)GlobalLock (hBMP);
		lb.lbStyle = BS_DIBPATTERNPT;
		lb.lbColor = DIB_RGB_COLORS ;
		lb.lbHatch = pBMP;
		hPen = ExtCreatePen (PS_GEOMETRIC|PS_SOLID|PS_ENDCAP_FLAT|PS_JOIN_ROUND,iWidth,&lb,0,0);
		GSSiGlobUlFree (&hBMP);
	}
	return hPen;
}

void DisplayProfileThemeLegend(short From)
#if ENABLETRACE
{GSSiEnterProg (1288);
#endif
{    
	short	CurViewID = CurView->ID, reps, h,w, isurf, nSurf=1;    
	double	BaseDistPerPixel, MaxWidth, BarWidth, step;
	POINT	Points[3];  
	long	IntVal;  
	char	txt[64], str[16], fmt[16]=" %.0f mile "; 
	HPEN	OldPen, LinePen;  
	HBRUSH	hOldBrush, hBrush;
	HANDLE	hProfile, hProfileD[2]={0,0}, hPolyPoints, hRoute, hSurf[2], hTran;
	long	nPolyPoints=0, nProfilePoints[2]={256,256}, nRoutePoints=0, nptosmooth;
	int		nRoutes, iRoute=0;
	HPDPOINT	pProfileD, pBeginSmooth,pPoly, pRoute, pRouteSave;
	HPPOINT     pProfile;
	LPPROFILETHEMEDATA	lpProfileData;   
	HIGHLIGHTDATA	HighlightData,HighlightData1;
	long	Seq, Refno; 
	short	pos=BT_FIRST, cond=BT_EQ, IncID;
	double	LenRoute, IncDist, Dist, ElevRange, MinElev, MaxElev, MidElev, MinDist,d; 
	MNMXCORD Bounds; 
	DPOINT	Point, OpenEnd, Point1, Point2;
	long	i=0, nGrid;  
	RECT	ProfileRect; 
	short	MaxXText=10, MaxYText=10;    
	long	Ranges[]={10,20,25,40,50,100,200,500,1000,5000,10000,100000}; 
	long	RangeInc[]={1,1,5,5,5,10,10,50,100,500,1000,10000};
	short	nRanges=12;  
	HPEN	hPen, hOldPen;
	BOOL	Reverse=FALSE, HaveOpenEnd=FALSE;  
	POINT	p, GridPoint[2];   
	DWORD	TextExt;
   	short np=0;  
   	char	SurfName[MAX_PATH];  
   	double	GridSpace,PixelsPerElevUnit=1,PixelsPerDistUnit=1,SymMarkerHeight,GridInc; 
   	COLORREF	ProfileColor[2]={0,RGB(0,255,0)}; 
   	COLORREF	PipeBrushColor=RGB(0,0,255), PipePenColor=0, MHColor=RGB(255,0,0),SymMarkerColor=RGB(64,64,64);
	LPPROFILESYMBOLS	pProfileSymbols;
	USHORT	iProfileSym;
   	HANDLE	hDB=0;  
   	short	SurfType[2];
	POINT	WLine[2], AreaPoints[4];
	BOOL	AlignWithRoute=CurTheme->ProfileAlignmentOption;
	POINT	ScreenPoint;
	int		ndp,twidth;
	SIZE	txSize;
	double	HorToVertFactor;
	static	int	VertScaleFactor=0;
	int		nConnectedPipes, maxPoints3D=0,	nPnt3D, iWidth;
	LOGFONT	LogFont, LogFontRing, LogFontSump;
   	HFONT	OldFont,hFontSump=0, hFontRing=0, hFont1=0;
	
	ProfileAndCrossSectionSettings (TRUE);	
//	if (From != 3 && From != 4)
	if (InShowZoomArea || From != 4)
		return;
//	GetGlobalCVal ("[%DTMNAME]",SurfName,"[%DL]attribut\\ot1.dtm");
	InProfile=TRUE;
 	DisplayProfileLoc (NULL,NULL,NULL,FALSE);
	CreateCrossSection (CurView->ID,CurView->ProfileInCrossSection); 
	GridSpace = CurTheme->DistanceBetweenProfilePoints; 
	_fstrcpy (SurfName,CurTheme->DataFile);
	lpProfileData = (LPPROFILETHEMEDATA)&CurTheme->ClassBM;    
	SaveDC (CurView->hDC);
	SetDisplayMode (CurView->hDC, GF_TEXTMODE);  
    SetTextColor (CurView->hDC,0);     
	SelectClipRgn(CurView->hDC, CurView->hRgn);
	FillRectPoly(CurView->hDC, &CurView->Rect, ConvertColor(CurView->BackGroundColor, -1));
	CurView->hRgn = CreateVPRgn(FALSE, FALSE);
	GSSiDeleteObject(&CurView->hRgn);
	SelectClipRgn(CurView->hDC, CurView->hRgn);
	GSSiDeleteObject(&CurView->hRgn);
	if (CurView->HaveFixedProfileRoute || (hHighlight && lpProfileData->MinSeq <= lpProfileData->MaxSeq))
	{
        hSurf[0] = DTMOpen (SurfName,DBL_MAX,BT_READ,&SurfType[0],0);
        if (!hSurf[0])
        {
        	goto Exit;
        } 
        if (*CurTheme->SQL)
        {
	        hSurf[1] = DTMOpen (CurTheme->SQL,DBL_MAX,BT_READ,&SurfType[1],0); 
	        if (hSurf[1])
	        	nSurf = 2;
        }
        else
        	hSurf[1] = 0; 
		h = (((CurView->DrawRect.bottom - CurView->DrawRect.top) / 2)-2*DeviceToScreenFactor());
		w = (h+4*DeviceToScreenFactor()); 
		Seq = lpProfileData->MinSeq; 
		if (CurView->HaveFixedProfileRoute)
		{
			nRoutes = CurView->nProfileRoutes;
			hRoute = CurView->hProfileRoute[0];
			nRoutePoints = CurView->nProfileRoute[0];
		   	pRoute = (HPDPOINT)GlobalLock (hRoute);
		}
	   	else if (CurView->ProfileInCrossSection)  
	   	{
			hRoute = GSSiGlobAlloc(GAIDNO 1012,GMEM_MOVEABLE,(long)2*sizeof(DPOINT));
		   	pRoute = (HPDPOINT)GlobalLock (hRoute);
	   		pRoute[0] = CurView->ProfileCrossSection[0];
	   		pRoute[1] = CurView->ProfileCrossSection[1];  
	   		nRoutePoints = 2;
			AlignWithRoute = FALSE;
	   	}
	   	else 
	   	{
			double begPCT = 0;
			double endPCT = 0;
			char cpct[128];
			int nhlt = BT_NUM_IN_INDEX(hHighlight);
			if (nhlt == 1)
			{
				if (*CurTheme->profilePCTFrom)
				{
					strcpy(cpct, CurTheme->profilePCTFrom);
					ExpandText(cpct);
					begPCT = atof(cpct);
				}
				if (*CurTheme->profilePCTTo)
				{
					strcpy(cpct, CurTheme->profilePCTTo);
					ExpandText(cpct);
					endPCT = atof(cpct);
				}
			}
			nRoutes = 1;
			hRoute = GSSiGlobAlloc(GAIDNO 1012,GMEM_MOVEABLE,(long)4096*sizeof(DPOINT));
		   	pRoute = (HPDPOINT)GlobalLock (hRoute);
		    do 
		    {
		    	if (BT_FIND (hHighlight2,(LPSTR)&Seq,pos,cond,(LPSTR)&Refno))
		    		goto EndRoute;
				BT_FIND (hHighlight,(LPSTR)&Refno,BT_FIRST,BT_EQ,(LPSTR)&HighlightData);
				pos = BT_NEXT;
				cond = BT_ANY;  
				PickList[0]=HighlightData.PD; 
				if (i || Seq == lpProfileData->MaxSeq)
				{   
					if (i && !HaveOpenEnd)
					{
						MinDist = ldistp (HighlightData1.PD.EndPoint,HighlightData.PD.BeginPoint);
						d = ldistp (HighlightData1.PD.EndPoint,HighlightData.PD.EndPoint);
						MinDist = min (d,MinDist);
						d = min (ldistp (HighlightData1.PD.BeginPoint,HighlightData.PD.BeginPoint),
								 ldistp (HighlightData1.PD.BeginPoint,HighlightData.PD.EndPoint));
						if (d < MinDist)   
						{
							OpenEnd = HighlightData1.PD.BeginPoint;
							Reverse = TRUE;
						}
						else
							OpenEnd = HighlightData1.PD.EndPoint;
				   		if (GetPolyPoints ((LPPICKDATAHEADER)&HighlightData1.PD,Reverse,&nPolyPoints,&hPolyPoints, 0))
				   		{
							pPoly = (HPDPOINT)GlobalLock(hPolyPoints);
							if (endPCT > 0)
							{
								double	PolyLength = GetPolyLengthD(pPoly, nPolyPoints);
								double	StartDist = begPCT * PolyLength;
								double	EndDist = endPCT * PolyLength;
								int NumOutPoints = 0;
								HANDLE hPoints = GetPolyBetweenDist(pPoly, nPolyPoints, StartDist, EndDist, &NumOutPoints,FALSE, FALSE);
								pPoly = (HPDPOINT)GlobalLock(hPoints);
								GSSiGlobUlFree(&hPolyPoints);
								hPolyPoints = hPoints;
							}
					   		while (nPolyPoints--)
					   			pRoute[nRoutePoints++] = *pPoly++;
				   			GSSiGlobUlFree (&hPolyPoints);
				   		} 
			   			HaveOpenEnd = TRUE;
					}
					Reverse = FALSE;
					if (HaveOpenEnd)
					{
						MinDist = ldistp (OpenEnd,HighlightData.PD.BeginPoint);
						d = ldistp (OpenEnd,HighlightData.PD.EndPoint);
						if (d < MinDist)
						{
							OpenEnd = HighlightData.PD.EndPoint;
							Reverse = TRUE;
						}
						else
							OpenEnd = HighlightData.PD.BeginPoint;
					}
			   		if (GetPolyPoints ((LPPICKDATAHEADER)&HighlightData.PD,Reverse,&nPolyPoints,&hPolyPoints, 0))
			   		{
				   		pPoly = (HPDPOINT)GlobalLock (hPolyPoints);
						if (endPCT > 0)
						{
							double	PolyLength = GetPolyLengthD(pPoly, nPolyPoints);
							double	StartDist = begPCT * PolyLength;
							double	EndDist = endPCT * PolyLength;
							int NumOutPoints = 0;
							HANDLE hPoints = GetPolyBetweenDist(pPoly, nPolyPoints, StartDist, EndDist, &NumOutPoints, FALSE, FALSE);
							pPoly = (HPDPOINT)GlobalLock(hPoints);
							GSSiGlobUlFree(&hPolyPoints);
							hPolyPoints = hPoints;
						}
						while (nPolyPoints--)
				   			pRoute[nRoutePoints++] = *pPoly++;
			   			GSSiGlobUlFree (&hPolyPoints); 
			   		}
		   		} 
		   		else
		   		{
		   			HighlightData1 = HighlightData;  
		   			i++;
		   		}
	   		} while (Seq < lpProfileData->MaxSeq);
			Reverse = FALSE;
			{
				LPVIEWPORT	SaveVP = CurView;
				POINT ScreenPoint1, ScreenPoint2;
				SetViewport(CurView->pTheme->TargetViewport);
				ScreenPoint1 = BasePtToScreenPt(&pRoute[0]);
				ScreenPoint2 = BasePtToScreenPt(&pRoute[nRoutePoints - 1]);
				CurView = SaveVP;
				if (ScreenPoint1.x > ScreenPoint2.x)
					Reverse = TRUE;
			}
			CurView->nProfileRoutes = nRoutes;
			GSSiGlobFree(&CurView->hProfileRouteSave);
		    CurView->hProfileRouteSave = GSSiGlobAlloc(GAIDNO 1012,GMEM_MOVEABLE,(long)nRoutePoints*sizeof(DPOINT)); 
		    CurView->nProfileRoutePointsSave = nRoutePoints;
		   	GlobalUnlock (hRoute); 
			if (Reverse)
				hRoute = ReversePoints(nRoutePoints, hRoute);
			pRoute = (HPDPOINT)GlobalLock(hRoute);
		   	pRouteSave = (HPDPOINT)GlobalLock (CurView->hProfileRouteSave);  
		   	hmemmove ((HPSTR)pRouteSave,(HPSTR)pRoute,nRoutePoints*sizeof(DPOINT));
		   	GlobalUnlock (CurView->hProfileRouteSave);
	   	}
EndRoute: 
		if (nRoutePoints < 2 || !GridSpace) 
		{
			DTMClose (&hSurf[0]);
			DTMClose (&hSurf[1]);
			goto Exit;
		}
		LenRoute = GetPolyLengthD (pRoute,nRoutePoints);  
		if (!(CurView->ProfileDistUnits = GetScaleBarUnits (CurTheme->TargetViewport)))
		{
			if (LenRoute < 2000)
				CurView->ProfileDistUnits = 1;
			else
				CurView->ProfileDistUnits = 4;
		}
		if (CurView->ProfileInCrossSection)
		{   
			p.x = ((long)CurView->DrawRect.left + CurView->DrawRect.right)/2;
			p.y = CurView->DrawRect.top;
			Dist = ConvertDist (CurView->LastProfileDist,CurView->ProfileDistUnits);
		    sprintf (txt,"Cross Section at %.3f",Dist); 
		    h = 20*DeviceToScreenFactor();
			DispText (CurView->hDC,FALSE,p.x,p.x, p.y,0, 2,3,h,1,1,2, FALSE,0,txt,0,FALSE,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0);
		}
	   	DBoundsInit (&Bounds);
		WaitCursor (1);
		for (isurf=0;isurf<nSurf;isurf++)
		{   
			nProfilePoints[isurf] = 1 + min (CurView->DrawRect.right - CurView->DrawRect.left,LenRoute/GridSpace);
			IncDist = LenRoute/(nProfilePoints[isurf]-1); 
			if (SurfType[isurf] == 22)//22 disables this code
			{
				hProfileD[isurf] = GSSiGlobAlloc(GAIDNO 1013,GMEM_MOVEABLE,(long)MAXPROFILEPOINTS*sizeof(DPOINT));
			   	pProfileD = (HPDPOINT)GlobalLock (hProfileD[isurf]);
			   	Dist = 0; 
		   		Point = PointAtDistOnPoly (pRoute,nRoutePoints,Dist,NULL,NULL);   
		   		pProfileD->x = Dist;
		   		pProfileD++->y = NGIELV_bci (Point,hSurf[isurf],1);
		   		Dist = LenRoute;
		   		Point = PointAtDistOnPoly (pRoute,nRoutePoints,Dist,NULL,NULL);   
		   		pProfileD->x = Dist;
		   		pProfileD->y = NGIELV_bci (Point,hSurf[isurf],1);
		   		GlobalUnlock (hProfileD[isurf]);  
		   		hProfilePoints = hProfileD[isurf];
		   		NumProfilePoints = 2;
				{ 
					LPVISLIST	SaveVis=CurVis;
					short	SaveMaxPick=MaxPick, SaveNFiles, SaveMT, SaveTimer, SavePT;
					HFILE	SaveFid; 
					HANDLE	hSaveVP = GSSiGlobAlloc(GAIDNO 0,GMEM_MOVEABLE,sizeof(VIEWPORT)+256);  
					HANDLE	SavehDTM;
					LPVIEWPORT	pSaveVP = (LPVIEWPORT)GlobalLock (hSaveVP); 
					LPSTR	pSaveFile1=(LPSTR)(pSaveVP+1); 
					long	SavenPnts;
					HPDPOINT	SaveCurPoints; 
					char	SaveCRT[MAX_PREFIX_LEN+2], SaveCRU[MAX_UDI_LEN+2];
					long	SaveCRR;
					short	SaveCRD, SaveCRType;
					BOOL	SaveHP;  
					LPVIEWPORT	SaveVP;  
					MNMXCORD	PolyBounds;
					LPDTMINFO	pDTMInfo=(LPDTMINFO)GlobalLock (hSurf[isurf]); 
					
					GetPolyBoundsD (hRoute,nRoutePoints,&PolyBounds,TYPE_POLYLINE); 
					ExpandBounds (&PolyBounds,1);
					SaveVP = CurView;
					SetConfig (1);
					SetViewport (*pCommandViewport);  
					*pSaveVP = *CurView; 
					SaveDC (CurView->hDC); 
					CurView->NumFiles=1;    
					CurView->SubFile = 0;
					CurView->NumThemes=0;
					_fstrcpy (pSaveFile1,CurView->lpFiles[0]); 
					_fstrcpy (CurView->lpFiles[0],pDTMInfo->TINIndex);  
					CurView->FileType[0] = 4;
					CurVis = &pDTMInfo->VisList;  
					CurView->pVisList1 = CurVis;
					SaveMT = MapType;
					MapType = 0;  
					SavehDTM = hDTM;
					hDTM = 0;
					SaveTimer = idTimer;    
					SaveFid = FidMap; 
					FidMap = HFILE_ERROR; 
					SavePT = PltType;
					idTimer = 0;  
					SaveCurPoints = lpDCurPoints;
					SavenPnts = nPnts;     
					SaveCRR = CurrentRefno;
					SaveCRD = CurrentDesc;
					strncpy0 (SaveCRT,CurrentPrefix,MAX_PREFIX_LEN);
					strncpy0 (SaveCRU,CurrentUDI,MAX_UDI_LEN);
					SaveHP	= HiPrecis; 
					SaveCRType = CurrentType;      
					nProfileRoute = nRoutePoints;
					hProfileRoute = hRoute;
					HighlightInArea (CurView->hWnd,&PolyBounds,TRUE,TRUE,0);
					lpDCurPoints = SaveCurPoints; 
					CurrentType = SaveCRType;
					nPnts = SavenPnts;  
					CurrentRefno = SaveCRR;
					CurrentDesc = SaveCRD;
					strncpy0 (CurrentPrefix,SaveCRT,MAX_PREFIX_LEN);
					strncpy0 (CurrentUDI,SaveCRU,MAX_UDI_LEN);
					HiPrecis = SaveHP;
					FidMap = SaveFid;
					PltType = SavePT; 
					idTimer = SaveTimer;
					MapType = SaveMT;  
					hDTM = SavehDTM;
					CurVis = SaveVis;   
					RestoreDC (CurView->hDC,-1);
					SetViewport (*pCommandViewport);  
					*CurView = *pSaveVP;  
					_fstrcpy (CurView->lpFiles[0],pSaveFile1);
					GSSiGlobUlFree (&hSaveVP);
					CurView = SaveVP;   
					GlobalUnlock (hSurf[isurf]);
				}
				hProfilePoints = 0; 
				nProfilePoints[isurf] = NumProfilePoints;
			   	pProfileD = (HPDPOINT)GlobalLock (hProfileD[isurf]); 
			   	for (i=0;i<nProfilePoints[isurf];i++)
			   	{   
			   		if (pProfileD->y < DBL_MAX)
			   		{  
						AddDPointToMinMax (pProfileD,&Bounds);  
					}
					else
					{   
						if (Bounds.xmn > pProfileD->x)
							Bounds.xmn = pProfileD->x;
						if (Bounds.xmx < pProfileD->x)
							Bounds.xmx = pProfileD->x;
					}  
			   		pProfileD++;
			   	}  
			   	GlobalUnlock (hProfileD[isurf]);  
			}
			else
			{
				LPVIEWPORT pSaveVP = CurView;
				BOOL	haveInPoint = FALSE;
				BOOL	pointIsIn = FALSE;
				int		lastInPoint = 0;
				int		numProfilePoints = 0;

				if (!AlignWithRoute)
					haveInPoint = TRUE;
				hProfileD[isurf] = GSSiGlobAlloc(GAIDNO 1013,GMEM_MOVEABLE,(long)nProfilePoints[isurf]*sizeof(DPOINT));
			   	pProfileD = (HPDPOINT)GlobalLock (hProfileD[isurf]); 
			   	Dist = 0; 
			   	nptosmooth = 0;
			   	pBeginSmooth = pProfileD;
				SetViewport(CurView->pTheme->TargetViewport);
				for (i = 0; i<nProfilePoints[isurf]; i++)
			   	{   
			   		Point = PointAtDistOnPoly (pRoute,nRoutePoints,Dist,NULL,NULL);   
					ScreenPoint = BasePtToScreenPt(&Point);
					if (PtInRect(&CurView->ScreenRect, ScreenPoint))
					{
						haveInPoint = TRUE;
						pointIsIn = TRUE;
						lastInPoint = numProfilePoints;
					}
					else
						pointIsIn = FALSE;
					if (haveInPoint)
					{
						pProfileD->x = Dist;
			   			pProfileD->y = NGIELV_bci (Point,hSurf[isurf],1);
//if (isurf)
//	pProfileD->y -= (42/12.0) / MFT;
						if (pProfileD->y < DBL_MAX && (!AlignWithRoute || haveInPoint))
			   			{  
							if (!AlignWithRoute || pointIsIn)
								AddDPointToMinMax (pProfileD,&Bounds);  
							nptosmooth++;
						}
						else
						{   
							if (Bounds.xmn > pProfileD->x)
								Bounds.xmn = pProfileD->x;
							if (Bounds.xmx < pProfileD->x)
								Bounds.xmx = pProfileD->x;
							
							SmoothProfile (pBeginSmooth,nptosmooth); 
							nptosmooth = 0;
							pBeginSmooth = pProfileD + 1;
						}  
						pProfileD++;
						numProfilePoints++;
					}
					Dist += IncDist;
				}
				CurView = pSaveVP;
				SmoothProfile (pBeginSmooth,nptosmooth); 
			   	GlobalUnlock (hProfileD[isurf]);  
				if (AlignWithRoute)
				{
					nProfilePoints[isurf] = lastInPoint;
				}
				LenRoute = GetPolyLengthDH(hProfileD[isurf], nProfilePoints[isurf]);
//				if (!CheckForContinue (TRUE))
//					break;
		   	}
		}  
		WaitCursor (-1);
	   	GlobalUnlock (hRoute); 
		if (CurView->NumFiles)
		{
			MNMXCORD	PolyBounds;
					
			GetPolyBoundsD (hRoute,nRoutePoints,&PolyBounds,TYPE_POLYLINE); 
			ExpandBounds (&PolyBounds,1);
			nProfileRoute = nRoutePoints;
			hProfileRoute = hRoute;  
			hCurProfile = hProfileD[0];
			nCurProfile = nProfilePoints[0];
			hProfileSymbols = GSSiGlobAlloc(GAIDNO 0,GMEM_MOVEABLE,USHRT_MAX);
			NumProfileSymbols = 0;
			HighlightInArea (CurView->hWnd,&PolyBounds,TRUE,TRUE,0);  
			if (hProfileSymbols)
			{
				pProfileSymbols = (LPPROFILESYMBOLS)GlobalLock (hProfileSymbols); 
				for (iProfileSym=0;iProfileSym<NumProfileSymbols;iProfileSym++,pProfileSymbols++)
				{
					AddDPointToMinMax (&pProfileSymbols->Point,&Bounds);  
				}
				GlobalUnlock (hProfileSymbols);
			}
		}  
	   	if (!ValidBounds (&Bounds)) 
	   	{
			DTMClose (&hSurf[0]);
			DTMClose (&hSurf[1]);
	   		goto Exit; 
	   	} 
	   	hDB = OpenProfileDataFile (&Bounds);
		
	   	ElevRange = Bounds.ymx - Bounds.ymn;  
	   	Bounds.ymx += ElevRange * 0.25;
	   	ElevRange = Bounds.ymx - Bounds.ymn;  
	   	ElevRange = ConvertDist (ElevRange,1);
	   	for (i=0;i<nRanges;i++)
	   		if (ElevRange < Ranges[i])
	   			break;
	   	MinElev = Bounds.ymn * MFT;
	   	MinElev -= fmod (MinElev,(double)RangeInc[i]);
	   	MaxElev = Bounds.ymx * MFT;
	   	MaxElev -= fmod (MaxElev,(double)RangeInc[i]);
	   	MaxElev += RangeInc[i]; 
	   	MaxElev = MinElev + Ranges[i]; 
	   	SymMarkerHeight = (MaxElev - MinElev) * FTM * 0.15;
	   	Bounds.ymn = MinElev * FTM;
	   	Bounds.ymx = MaxElev * FTM; 
	   	IncID = i;
		nGrid = IDNINT((MaxElev - MinElev) / RangeInc[IncID]);
		MidElev = (MaxElev + MinElev) / 2;
	    sprintf (txt,"%.0f",MaxElev);
	    h = 20*DeviceToScreenFactor();
		TextExt = DispText (CurView->hDC,TRUE,0,0, -1,0, 2,2,h,1,1,2, FALSE,0,txt,0,FALSE,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0);
		MaxXText = LOWORD(TextExt); 
		MaxYText = HIWORD(TextExt);
		if (AlignWithRoute)
		{
			LPVIEWPORT	SaveVP=CurView;
			pRoute = (HPDPOINT)GlobalLock(hRoute);

			SetViewport (CurView->pTheme->TargetViewport);
			RectInit (&ProfileRect);
			pProfileD = (HPDPOINT)GlobalLock(hProfileD[0]);
			for (i = 0; i<nProfilePoints[0]; i++)
			{
				DPOINT Point = PointAtDistOnPoly(pRoute, nRoutePoints, pProfileD[i].x, NULL, NULL);
				ScreenPoint = BasePtToScreenPt(&Point);
				AddPointToRect (ScreenPoint,&ProfileRect);
			}
			GlobalUnlock(hProfileD[0]);
			CurView = SaveVP;
			ProfileRect.bottom = CurView->ScreenRect.bottom;
			ProfileRect.top = CurView->ScreenRect.top;
			//Bounds.xmn = ProfileRect.left;
			//Bounds.xmx = ProfileRect.right;
			InflateRect (&ProfileRect,0,-(MaxYText+4));
			if (VertScaleFactor)
			{
				double MidElev = (Bounds.ymx + Bounds.ymn) / 2;

	   			PixelsPerDistUnit = (ProfileRect.right - ProfileRect.left)/(Bounds.xmx - Bounds.xmn);
				Bounds.ymx = MidElev + (((ProfileRect.bottom - ProfileRect.top)/2)/PixelsPerDistUnit)/VertScaleFactor;
				Bounds.ymn = MidElev - (((ProfileRect.bottom - ProfileRect.top)/2)/PixelsPerDistUnit)/VertScaleFactor;
			}
			GlobalUnlock(hRoute);
		}
		else
		{
			ProfileRect = CurView->ScreenRect;
			InflateRect (&ProfileRect,-(MaxXText+4),-(MaxYText+4));
		}
		CurView->ProfileRect = ProfileRect;
		CurView->ProfileBounds = Bounds;
	    hTran = STRANBoundsToRect (&Bounds,&ProfileRect); 
	   	if (ElevRange)
	   		PixelsPerElevUnit = (ProfileRect.bottom - ProfileRect.top)/(Bounds.ymx - Bounds.ymn);
	   	PixelsPerDistUnit = (ProfileRect.right - ProfileRect.left)/(Bounds.xmx - Bounds.xmn);
		HorToVertFactor = (double)PixelsPerDistUnit / (double)PixelsPerElevUnit;
		DTMClose (&hSurf[0]);
		DTMClose (&hSurf[1]);
		Point1.x = Bounds.xmn;
		Point1.y = Bounds.ymn;
		Point2.x = Bounds.xmx;
		Point2.y = Bounds.ymn;
		hPen = CreatePen (PS_DOT,1,RGB(255,212,212));
		hOldPen = SelectObject (CurView->hDC,hPen);
		for (i=0;i<=nGrid;i++)
		{ 
			GridPoint[0] = TRANDPointToPoint (&Point1,hTran);
			GridPoint[1] = TRANDPointToPoint (&Point2,hTran);
		    Polyline (CurView->hDC,GridPoint,2); 
		    Point1.y += (double)RangeInc[IncID] * FTM;
		    Point2.y += (double)RangeInc[IncID] * FTM;
		}
		GridInc = 0.01; 
		ndp = 2;
		nGrid = 101;
		Dist = ConvertDist (LenRoute,CurView->ProfileDistUnits);
		while (nGrid > 100)   
		{
			GridInc *= 10;
			ndp = max (0,ndp-1);
			nGrid = Dist / GridInc;
		} 
		Dist = 0;
		if (AlignWithRoute)
		{
			pProfileD = (HPDPOINT)GlobalLock(hProfileD[0]);
			Point1.x = Dist = pProfileD->x;
			GlobalUnlock(hProfileD[0]);
		}
		else
			Point1.x = Dist;
		Point2.x = Point1.x;
		Point1.y = Bounds.ymn;
		Point2.y = Bounds.ymx;
		for (i=0;i<=nGrid;i++)
		{ 
			GridPoint[0] = TRANDPointToPoint (&Point1,hTran);
			GridPoint[1] = TRANDPointToPoint (&Point2,hTran);
		    Polyline (CurView->hDC,GridPoint,2);  
		    if (i % 2 == 0)
		    {   
		    	double	d = Dist;
		    	
			    SetTextColor (CurView->hDC,0); 
			    p = GridPoint[0];
			    if (CurView->ProfileInCrossSection)
					d -= CurTheme->CrossSectionWidth[0];
				RWRITE (ConvertDist(fabs(d),CurView->ProfileDistUnits),ndp,txt);
				DispText (CurView->hDC,FALSE,p.x,p.x, p.y,0, 2,3,h*0.8,1,1,2, FALSE,0,txt,0,FALSE,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0);
		    }
		    Dist += ConvertInDist (GridInc,CurView->ProfileDistUnits);
/*			if (AlignWithRoute)
			{
				LPVIEWPORT	SaveVP=CurView;

				SetViewport (CurView->pTheme->TargetViewport);
		   		Point = PointAtDistOnPoly (pRoute,nRoutePoints,Dist,NULL,NULL);   
				Point1 = BasePtToScreenPtD (&Point);
				CurView = SaveVP;
			}
			else*/
				Point1.x = Dist;
		    Point2.x = Point1.x;
			Point1.y = Bounds.ymn;
			Point2.y = Bounds.ymx;
		}
/*		{
			double dist = 0;
			while (dist < LenRoute)
			{

			double	d=pPoint->x;
			
			if (SaveView->ProfileInCrossSection)
				d -= CurTheme->CrossSectionWidth[0];
			Dist = ConvertDist (d,SaveView->ProfileDistUnits);
			sprintf (Text,"Dist = %.3f %s, Elevation = %.1f feet, Slope = %.1f percent                ",Dist,_fstrlwr(DistUnitOpts[SaveView->ProfileDistUnits-1]),Elev * MFT,Slope); 
			TextOut(DisplayView->hDC, x, y, Text, _fstrlen(Text)); 
*/
//		SelectObject (CurView->hDC,hOldPen); 
		SelectObject (CurView->hDC,GetStockObject(BLACK_PEN)); 
		DeleteObject (hPen); 
		for (isurf=0;isurf<nSurf;isurf++)
		{
			np = 0;
			hPen = CreatePen (PS_SOLID,(int)IDNINT(DeviceToScreenFactor()),ProfileColor[isurf]);
			hOldPen = SelectObject (CurView->hDC,hPen);      
		    hProfile = GSSiGlobAlloc(GAIDNO 1014,GMEM_MOVEABLE,nProfilePoints[isurf]*sizeof(POINT));
		    pProfile = (HPPOINT)GlobalLock (hProfile);
		   	pProfileD = (HPDPOINT)GlobalLock (hProfileD[isurf]); 
		   	for (i=0;i<nProfilePoints[isurf];i++)
		   	{
		   		if (pProfileD[i].y < DBL_MAX)
		   		{ 
					DPOINT	ProfPoint = pProfileD[i], WPoint, ScreenPointD;

/*					if (AlignWithRoute)
					{
						LPVIEWPORT	SaveVP=CurView;

						SetViewport (CurView->pTheme->TargetViewport);
				  		WPoint = PointAtDistOnPoly (pRoute,nRoutePoints,ProfPoint.x,NULL,NULL); 
						ScreenPointD = BasePtToScreenPtD (&WPoint);
						ProfPoint.x = ScreenPointD.x;
						CurView = SaveVP;
		 			}*/
			     	pProfile[np++] = TRANDPointToPoint (&ProfPoint,hTran);  
			    }
			    else if (np > 1)
			    {
			    	Polyline (CurView->hDC,pProfile,np);
			    	np =0;  
			    }
			    else
			    	np = 0;
		    }
		    if (np > 1)
		    	Polyline (CurView->hDC,pProfile,np);
		    GSSiGlobUlFree (&hProfile);  
		    GlobalUnlock (hProfileD[isurf]);
			SelectObject (CurView->hDC,GetStockObject(BLACK_PEN)); 
			DeleteObject (hPen); 
		}  
	    GSSiDeleteObject(&CurView->hRgn);    
	    CurView->hRgn = CreateVPRgn(FALSE,FALSE);
	    SelectClipRgn (CurView->hDC,CurView->hRgn);
	    GSSiDeleteObject(&CurView->hRgn);    
		if (hProfileSymbols)
		{
			BOOL saveUseGDIPlus = useGDIPlus;

			useGDIPlus = FALSE;
			pProfileSymbols = (LPPROFILESYMBOLS)GlobalLock (hProfileSymbols); 
			for (iProfileSym=0;iProfileSym<NumProfileSymbols;iProfileSym++,pProfileSymbols++)
			{   
				DPOINT	DPoint = pProfileSymbols->Point;
				
				DPoint.y = pProfileSymbols->SurfElev;
				WLine[0] = TRANDPointToPoint (&DPoint,hTran); 
				if (pProfileSymbols->SurfElev <= MidElev*FTM)
					DPoint.y += SymMarkerHeight;
				else
					DPoint.y -= SymMarkerHeight;
				WLine[1] = TRANDPointToPoint(&DPoint, hTran);
				hPen = CreatePen(PS_SOLID, DeviceToScreenFactor(), SymMarkerColor);
				hOldPen = SelectObject (CurView->hDC,hPen);      
		    	Polyline (CurView->hDC,WLine,2);
				SelectObject (CurView->hDC,hOldPen); 
				DeleteObject (hPen);    
				GetDictSymName (abs(pProfileSymbols->idesc),txt);
				{   
					LOGFONT	LogFont;    
				   	HFONT	OldFont,hFont;
					SIZE	txSize;
					
					_fmemset (&LogFont,0,sizeof(LOGFONT));
					LogFont.lfHeight = 11*DeviceToScreenFactor(); 
					LogFont.lfEscapement = LogFont.lfOrientation = 900;   
					LogFont.lfWeight = FW_MEDIUM;
					LogFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
					LogFont.lfQuality = PROOF_QUALITY; 
					_fstrcpy (LogFont.lfFaceName,"Times New Roman");    //Courier Bold New
					hFont = CreateFontIndirect((LPLOGFONT)&LogFont);
					OldFont = SelectObject (CurView->hDC,hFont);
					GetTextExtentPoint32(CurView->hDC, txt, _fstrlen(txt), &txSize);
					if (pProfileSymbols->SurfElev > MidElev*FTM)
						WLine[1].y += txSize.cx + 2;
					else
						WLine[1].y -= 2;
					WLine[1].x -= txSize.cy/2 * DeviceToScreenFactor();
					TextOut (CurView->hDC,WLine[1].x,WLine[1].y,txt,_fstrlen(txt));	
					SelectObject (CurView->hDC,OldFont);
					DeleteObject (hFont);
				} 
				if (pProfileSymbols->idesc < 0)
				{
					int	Symbol = GetDictSymbolNumber ("CIRCLE");
					HANDLE	hSymbol = GetDictSymDesc (Symbol,0); 
					POINT	WinPoint; 
					short	size=pProfileSymbols->Size*PixelsPerElevUnit*FTM;
					
					if (hSymbol) 
					{   
						WinPoint = TRANDPointToPoint (&pProfileSymbols->Point,hTran); 
						DisplayPointSymbol (hSymbol, CurView->hDC, size,size,0, &WinPoint,NULL,FALSE,0,0,FALSE,TRUE,0,0);   
						DestroySymbol (hSymbol);
					}
				}
			}
			GSSiGlobUlFree (&hProfileSymbols);
			useGDIPlus = saveUseGDIPlus;
		}
		if (hDB)
		{
			LPGWDHEADER	lpGWDHead; 
			LPPROFILEFILEDATA	pData;
			long	Offset;
			short	Seq; 
			short	pos = BT_FIRST;
			DPOINT	DLine[2], PointOnRoute, DPoint, DPoint1, DPoint2, DPoint3;
			DPOINT3D	DLine3D[2];
			double	OffDist, PolyDist, az;
			char	txt[256];
			int		xt, yt;
			POINT	midp;
			int		iPass;
			double	maxOffset=1;
			LPPROFILEDATARECTANGLE	pProfileDataRectangles;
			
		   	pRoute = (HPDPOINT)GlobalLock (hRoute);
		    lpGWDHead = (LPGWDHEADER)GlobalLock (hDB); 
			pData = (LPPROFILEFILEDATA)&lpGWDHead->GWDData; 
			CurView->nProfileDataRectangles = 0;
			GSSiGlobFree (&CurView->hProfileDataRectangles);
			CurView->hProfileDataRectangles = GSSiGlobAlloc(GAIDNO 0,GMEM_MOVEABLE,USHRT_MAX);
			pProfileDataRectangles = (LPPROFILEDATARECTANGLE)GlobalLock (CurView->hProfileDataRectangles);
					
			_fmemset (&LogFont,0,sizeof(LOGFONT));
			LogFont.lfHeight = 13*DeviceToScreenFactor(); 
			LogFont.lfWeight = FW_NORMAL;    
			LogFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
			LogFont.lfQuality = PROOF_QUALITY; 
			_fstrcpy (LogFont.lfFaceName,"Times New Roman MT Extra Bold");    //Courier Bold New
			//hFont1 = CreateFontIndirect((LPLOGFONT)&LogFont);
			LogFontRing.lfHeight = 12*DeviceToScreenFactor(); 
			LogFontRing.lfWeight = FW_NORMAL;    
			LogFontRing.lfEscapement = 450;
			LogFontRing.lfOutPrecision = OUT_DEFAULT_PRECIS;
			LogFontRing.lfQuality = PROOF_QUALITY; 
			_fstrcpy (LogFontRing.lfFaceName,"Times New Roman MT");    //Courier Bold New
			hFontRing = CreateFontIndirect((LPLOGFONT)&LogFontRing);
			LogFontSump.lfHeight = 12*DeviceToScreenFactor(); 
			LogFontSump.lfWeight = FW_NORMAL;    
			LogFontSump.lfOutPrecision = OUT_DEFAULT_PRECIS;
			LogFontSump.lfQuality = PROOF_QUALITY; 
			_fstrcpy (LogFontSump.lfFaceName,"Times New Roman MT");    //Courier Bold New
			hFontSump = CreateFontIndirect((LPLOGFONT)&LogFontRing);
			OldFont = SelectObject (CurView->hDC,hFontRing);
			LinkPipesFunction (0,0,0,0,0,0,0);
			for (iPass=0;iPass<2;iPass++)
			{
				pos = BT_FIRST;
				while (!BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&Seq,pos,BT_ANY,(LPSTR)&Offset))
				{  
					pos = BT_NEXT;
					FillGWDData (lpGWDHead,Offset);
					switch (pData->Type)
					{
						case 'L': 
							if (iPass)
								break;
							DPoint.x = pData->BPX;
							DPoint.y = pData->BPY;
							DPoint1.x = pData->BPX;
							DPoint1.y = pData->BPZ;
							DPoint2.x = pData->EPX;
							DPoint2.y = pData->EPZ;
							DLine[0].y = pData->BPZ + pData->Width/2;
							az = getazd (&DPoint2,&DPoint1);
							DPoint3 = dnewpt (DPoint1,az-HALFPI,ProfileDistConv(pData->Width/2,HorToVertFactor,az));
							DLine[0].y = DPoint3.y;
							if (!GetProfileX (&DPoint,nRoutePoints,pRoute,&PointOnRoute,&OffDist,&PolyDist))
								break;
							DLine[0].x = PolyDist; 
							WLine[0] = TRANDPointToPoint (&DLine[0],hTran);
							DLine3D[0].x = WLine[0].x;
							DLine3D[0].y = WLine[0].y;
							DLine3D[0].z = OffDist;
							DPoint.x = pData->EPX;
							DPoint.y = pData->EPY;
							DLine[1].y = pData->EPZ + pData->Width/2;
							az = getazd (&DPoint2,&DPoint1);
							DPoint3 = dnewpt (DPoint2,az-HALFPI,ProfileDistConv(pData->Width/2,HorToVertFactor,az));
							DLine[1].y = DPoint3.y;
							if (!GetProfileX (&DPoint,nRoutePoints,pRoute,&PointOnRoute,&OffDist,&PolyDist))
								break;
							DLine[1].x = PolyDist; 
							WLine[1] = TRANDPointToPoint (&DLine[1],hTran);
							DLine3D[1].x = WLine[1].x;
							DLine3D[1].y = WLine[1].y;
							DLine3D[1].z = OffDist;
							nPnt3D = 2;
							iWidth = IDNINT (pData->Width * MFT *12);
							if (DLine3D[0].z < maxOffset && DLine3D[1].z < maxOffset)
								LinkPipesFunction (1,0,0,&nPnt3D,DLine3D,&iWidth,pData->Material);
							maxPoints3D += nPnt3D;
	//				    	Polyline (CurView->hDC,WLine,2);
							midp = MidPoint (WLine[0],WLine[1]);
				    		AreaFromLineAndOffset (WLine,pData->Width/2*PixelsPerElevUnit,AreaPoints);   
				    		/*hBrush = CreateGMBrush (PipeBrushColor,0,CurView->hDC);
				    		hOldBrush = SelectObject (CurView->hDC,hBrush);
				    		Polygon (CurView->hDC,AreaPoints,4);  
				    		SelectObject (CurView->hDC,hOldBrush);
				    		DeleteObject (hBrush);*/
				    		_fmemmove (pProfileDataRectangles[CurView->nProfileDataRectangles].Points,AreaPoints,sizeof(AreaPoints)); 
				    		pProfileDataRectangles[CurView->nProfileDataRectangles++].Offset = Offset;
							sprintf (txt,"%i\" %s",IDNINT (pData->Width * MFT *12),pData->Material);
							xt = midp.x;
							yt = midp.y + LogFont.lfHeight;
							SelectObject (CurView->hDC,hFont1);
							//TextOutWithShadow (CurView->hDC,xt,yt,txt,strlen(txt),1,RGB(255,255,255));
							break;
						case 'P': 
							if (!iPass)
								break;
							DPoint.x = pData->BPX;
							DPoint.y = pData->BPY;
							if (!GetProfileX (&DPoint,nRoutePoints,pRoute,&PointOnRoute,&OffDist,&PolyDist))
								break;
							if (OffDist > maxOffset)
								break;
							DLine[0].x = PolyDist; 
							DLine[0].y = pData->BPZ;
							WLine[0] = TRANDPointToPoint (&DLine[0],hTran);
							DLine[1] = DLine[0];
							DLine[1].y = pData->EPZ;
							WLine[1] = TRANDPointToPoint (&DLine[1],hTran);
							//hPen = CreatePen (PS_SOLID,pData->Width*PixelsPerElevUnit,MHColor);
							//hPen = CreateTexturePen (pData->Width*PixelsPerElevUnit,"G:\\geomas\\Maplib\\Sewer\\textures\\brick_small.jpg");
							hPen = CreateTexturePen (pData->Width*PixelsPerElevUnit,"[%TEXTUREDIR]yellow.jpg");
							hOldPen = SelectObject (CurView->hDC,hPen);      
				    		Polyline (CurView->hDC,WLine,2);  
				    		AreaFromLineAndOffset (WLine,pData->Width/2*PixelsPerElevUnit,AreaPoints);   
/*				    		hBrush = CreateSolidBrush (MHColor);
				    		hOldBrush = SelectObject (CurView->hDC,hBrush);
				    		Polygon (CurView->hDC,AreaPoints,4);  
				    		SelectObject (CurView->hDC,hOldBrush);
				    		DeleteObject (hBrush);*/
				    		_fmemmove (pProfileDataRectangles[CurView->nProfileDataRectangles].Points,AreaPoints,sizeof(AreaPoints)); 
				    		pProfileDataRectangles[CurView->nProfileDataRectangles++].Offset = Offset;
							SelectObject (CurView->hDC,hOldPen); 
							DeleteObject (hPen); 
							xt = WLine[0].x;
							SelectObject (CurView->hDC,hFontRing);
							sprintf (txt,"%.1f",pData->BPZ*MFT);
							yt = WLine[0].y - LogFontRing.lfHeight;
							TextOutWithShadow (CurView->hDC,xt,yt,txt,strlen(txt),1,RGB(255,255,255));
							sprintf (txt,"%.1f",pData->EPZ*MFT);
							yt = WLine[1].y + LogFontSump.lfHeight * 1.5;
							xt -= LogFontSump.lfHeight * 2;
							SelectObject (CurView->hDC,hFontSump);
							TextOutWithShadow (CurView->hDC,xt,yt,txt,strlen(txt),1,RGB(255,255,255));
							break;
					}
				} 
				if (!iPass)
				{
					nConnectedPipes = LinkPipesFunction (2,0,10,0,0,0,0);
					for (i=0;i<nConnectedPipes;i++)
					{
						HANDLE hPoints3D = GSSiGlobAlloc(GAIDNO 0,GMEM_MOVEABLE,maxPoints3D * sizeof(DPOINT3D));
						LPDPOINT3D pPoints3D = GlobalLock (hPoints3D);
						HANDLE	hPnts;
						LPPOINT	pPnts;
						int	j, xt, yt;
						char	Material[128];
						RECT	bnds;

						_fmemset (&LogFont,0,sizeof(LOGFONT));
						LogFont.lfHeight = 13*DeviceToScreenFactor(); 
						LogFont.lfWeight = FW_NORMAL;    
						LogFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
						LogFont.lfQuality = PROOF_QUALITY; 
						_fstrcpy (LogFont.lfFaceName,"Times New Roman MT Extra Bold");    //Courier Bold New
						hFont1 = CreateFontIndirect((LPLOGFONT)&LogFont);

						LinkPipesFunction (3,i,0,&nPnt3D,pPoints3D,&iWidth,Material);
						hPnts = GSSiGlobAlloc(GAIDNO 0,GMEM_MOVEABLE,nPnt3D*sizeof(POINT));
						pPnts = GlobalLock (hPnts);
						for (j=0;j<nPnt3D;j++)
						{
							pPnts[j].x = pPoints3D[j].x;
							pPnts[j].y = pPoints3D[j].y;
						}

						hPen = CreateTexturePen (PixelsPerElevUnit/12*iWidth*FTM,"[%TEXTUREDIR]grey.jpg");
						//hPen = CreatePen (PS_SOLID,iWidth/2,PipePenColor);
						hOldPen = SelectObject (CurView->hDC,hPen);      
						Polyline (CurView->hDC,pPnts,nPnt3D);
						SelectObject (CurView->hDC,hOldPen);
						DeleteObject (hPen);
						sprintf (txt,"%i\" %s",iWidth,Material);
						bnds = GetPolyBounds (hPnts,nPnt3D);
						xt = (bnds.left + bnds.right) / 2;
						yt = bnds.bottom + 8*DeviceToScreenFactor() + LogFont.lfHeight;
						OldFont = SelectObject (CurView->hDC,hFont1);
						TextOutWithShadow (CurView->hDC,xt,yt,txt,strlen(txt),1,RGB(255,255,255));
						SelectObject (CurView->hDC,OldFont);
						GSSiDeleteObject (&hFont1);
						GSSiGlobUlFree (&hPnts);
						GSSiGlobUlFree (&hPoints3D);
					}
					LinkPipesFunction (0,0,0,0,0,0,0);
				}
			}
			GlobalUnlock (hDB); 
			GlobalUnlock (hRoute);    
			GlobalUnlock (CurView->hProfileDataRectangles);
			SelectObject (CurView->hDC,OldFont);
			GSSiDeleteObject (&hFont1);
			GSSiDeleteObject (&hFontRing);
			GSSiDeleteObject (&hFontSump);
		}
	    GSSiGlobFree (&CurView->hProfileElev[iRoute]); 
	    CurView->hProfileElev[iRoute] = hProfileD[0];  
	    GSSiGlobFree (&hProfileD[1]); 
	    CurView->nProfileElev[iRoute] = nProfilePoints[0];
	    CurView->LinkedTo = CurTheme->TargetViewport;   
	    CloseTRANS2 (&hTran);
	    CloseTRANS2 (&CurView->hFileTransIn);   
	    CloseTRANS2 (&CurView->hTranVPToBase);
	    CloseTRANS2 (&CurView->hTranBaseToVP);
	    CloseTRANS2 (&CurView->hTranVPToScreen);
	    CloseTRANS2 (&CurView->hTranScreenToVP);
	    SetTextColor (CurView->hDC,0); 
	    p.y = ProfileRect.bottom+2;
	    sprintf (txt,"%.0f",MaxElev);
		GetTextExtentPoint32 (CurView->hDC,txt,_fstrlen(txt),&txSize);
	    sprintf (txt,"%.0f",MinElev);
		twidth = txSize.cx+2; 
	    p.x = min (CurView->ScreenRect.right-twidth/2,ProfileRect.right) + 2;
		SaveDC(CurView->hDC);
		SelectClipRgn(CurView->hDC, CurView->hRgn);
		DispText (CurView->hDC,FALSE,p.x,p.x, p.y,0, 1,1,h,1,1,2, FALSE,0,txt,0,FALSE,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0);
	    p.x = max (CurView->ScreenRect.left+twidth/2,ProfileRect.left) - 2;
		DispText (CurView->hDC,FALSE,p.x,p.x, p.y,0, 4,1,h,1,1,2, FALSE,0,txt,0,FALSE,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0);
	    p.y = ProfileRect.top+2;
	    sprintf (txt,"%.0f",MaxElev);
	    p.x = min (CurView->ScreenRect.right-twidth/2,ProfileRect.right) + 2;
		DispText (CurView->hDC,FALSE,p.x,p.x, p.y,0, 1,1,h,1,1,2, FALSE,0,txt,0,FALSE,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0);
	    p.x = max (CurView->ScreenRect.left+twidth/2,ProfileRect.left) - 2;
		DispText (CurView->hDC,FALSE,p.x,p.x, p.y,0, 4,1,h,1,1,2, FALSE,0,txt,0,FALSE,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0);
		RestoreDC(CurView->hDC, -1);
	    
	   // CurView->hFileTransIn = STRANRectToBounds (&ProfileRect,&Bounds); 
	    if (hRoute != CurView->hProfileRoute[iRoute])
			GSSiGlobFree (&CurView->hProfileRoute[iRoute]);
	    CurView->hProfileRoute[iRoute] = hRoute;  
	    CurView->nProfileRoute[iRoute] = nRoutePoints;
	} 
	else
		CurView->LinkedTo = 0; 
Exit:    
	CloseGWDatabase (hDB);
	RestoreDC (CurView->hDC,-1);
	DisplayCloseIcon();	
	InProfile = FALSE;
{
#if ENABLETRACE
GSSiExitProg (1288);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}
void ResetProfileVP(LPVIEWPORT pVp)
{
	if (LastProfileLocVP == pVp)
		LastProfileLocVP = NULL;
	DisplayProfileLink(0);
}
BOOL DisplayProfileLoc (LPPOINT pWinPoint,LPDPOINT BasePoint,LPVIEWPORT pVP,BOOL LoadConfig)
#if ENABLETRACE
{GSSiEnterProg (1316);
#endif
{   
	static	POINT	LastLine[2];
	LPVIEWPORT	SaveVP=CurView;  
	short	OldMode; 
	HPEN	hPen, hOldPen;  
	double	LastDist;
	
	if (LoadConfig) 
	{
		LastProfileLocVP = NULL;
{
#if ENABLETRACE
GSSiExitProg (1316);
#endif
		return FALSE;
}
	}
	if (LastProfileLocVP)
	{ 
		SaveDC (CurView->hDC);
	    SetDisplayMode (CurView->hDC,GF_SCREENMODE);
	    SelectClipRgn (CurView->hDC,0);
		SetCurView(LastProfileLocVP);
		OldMode = SetROP2(CurView->hDC,R2_NOT);
		hPen = CreatePen (PS_SOLID,0,RGB(255,0,0));
		hOldPen = SelectObject (CurView->hDC,hPen);
		Polyline (CurView->hDC,LastLine,2);  
		SelectObject (CurView->hDC,hOldPen);
		DeleteObject (hPen);
    	SetROP2(CurView->hDC,OldMode);
		SetCurView (SaveVP);
		RestoreDC (CurView->hDC,-1);  
	}                        
	if (!pVP) 
	{   
		DisplayProfileInfo (NULL,LastLine[0]);
		LastProfileLocVP = NULL;
{
#if ENABLETRACE
GSSiExitProg (1316);
#endif
		return FALSE;
}
	}
	LastProfileLocVP = pVP;
	if (BasePoint->x < 0)
	{
		LastProfileLocVP = 0;
{
#if ENABLETRACE
GSSiExitProg (1316);
#endif
		return FALSE;
}
	}
	if (pVP->hProfileElev[0])
	{
		LPDPOINT	pProfile = (LPDPOINT)GlobalLock(pVP->hProfileElev[0]);
		pProfile += (pVP->nProfileElev[0]-1);
		LastDist = pProfile->x;
		GlobalUnlock (pVP->hProfileElev[0]);
//		if (BasePoint->x > LastDist)  
		if (pWinPoint->x < pVP->ProfileRect.left || pWinPoint->x > pVP->ProfileRect.right)
		{
			LastProfileLocVP = 0;
{
#if ENABLETRACE
GSSiExitProg (1316);
#endif
			return FALSE;
}
		}
	}
	SaveDC (CurView->hDC); 
    SetDisplayMode (CurView->hDC,GF_SCREENMODE);
    SelectClipRgn (CurView->hDC,0);
	SetCurView (pVP); 
	LastLine[0].x = pWinPoint->x;
	LastLine[1].x = pWinPoint->x;
	LastLine[0].y = CurView->DrawRect.bottom;
	LastLine[1].y = CurView->DrawRect.top;
	OldMode = SetROP2(CurView->hDC,R2_NOT);
	hPen = CreatePen (PS_SOLID,0,RGB(255,0,0));
	hOldPen = SelectObject (CurView->hDC,hPen);
	Polyline (CurView->hDC,LastLine,2);  
	SelectObject (CurView->hDC,hOldPen);
	DeleteObject (hPen);
   	SetROP2(CurView->hDC,OldMode);
	SetCurView (SaveVP);
	RestoreDC (CurView->hDC,-1);  
{
#if ENABLETRACE
GSSiExitProg (1316);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}  

BOOL DisplayProfileLink (LPDPOINT pWinBasePoint)
{
	static	POINT	LastLine[2];
	static	LPVIEWPORT	LastVP=0; 
	LPVIEWPORT	SaveVP=CurView;  
	short	OldMode,i; 
	HPEN	hPen, hOldPen;  
	double	LastDist;
	POINT	WinPoint;
	LPTHEME	pTheme;
	BOOL	rtn=TRUE;
	BOOL	doZoom = FALSE;
	
	if (!pWinBasePoint) 
	{
		if (CurView == LastVP)
			LastVP = NULL;
		return rtn;
	}
	if (!CurView)
		return rtn;
	for (i=0;i<CurView->NumThemes;i++)
	{
		pTheme = CurView->pThemes[i];
		if (pTheme->ID == GF_PROFILE_LINK_THEME && VPIsActive (pTheme->DisplayViewport))
			goto Show;
	}
	return rtn;
Show:
	if (LastVP)
	{ 
		SetCurView (LastVP);
		SaveDC (CurView->hDC);
	    SetDisplayMode (CurView->hDC,GF_SCREENMODE);
	    SelectClipRgn (CurView->hDC,0);
		OldMode = SetROP2(CurView->hDC,R2_NOT);
		hPen = CreatePen (PS_SOLID,0,RGB(255,0,0));
		hOldPen = SelectObject (CurView->hDC,hPen);
		Polyline (CurView->hDC,LastLine,2);  
		SelectObject (CurView->hDC,hOldPen);
		DeleteObject (hPen);
    	SetROP2(CurView->hDC,OldMode);
		SetCurView (SaveVP);
		RestoreDC (CurView->hDC,-1); 
		LastVP = 0;
	} 
	SetViewport (pTheme->DisplayViewport);
	LastVP = CurView;
	WinPoint = BasePtToScreenPt (pWinBasePoint);
	if ((WinPoint.x < CurView->ScreenRect.left || WinPoint.x > CurView->ScreenRect.right) && GetGlobalBVal2("[%AUTOPROFILEPAN]", TRUE))
		doZoom = TRUE;
	{
		DPOINT	MidPoint=*pWinBasePoint;
		DPOINT	CurMidPoint = MinMaxMidPointD (&CurView->WBounds);
		double	Scale = CurView->Scale;
		int		n;
		int		SavePP = PickPerim;
		BOOL	SaveInShowZoomArea;

		MidPoint.y = CurMidPoint.y;
		UseUserPickAp = FALSE;
		PickPerim = 1;
		SystemPickAp = -10000;	
		n = PickItems (CurView->hWnd,CurMidPoint);
		UseUserPickAp = TRUE;
		PickPerim = SavePP;
		if (n)
		{
			int	nPnts;
			HANDLE	hPoly;
			DPOINT	ProfPoints[2];
			DPOINT	Points[2];

			ProfPoints[0].x = ProfPoints[1].x = MidPoint.x;
			ProfPoints[0].y = CurView->WBounds.ymn;
			ProfPoints[1].y = CurView->WBounds.ymx;
			ExtendPoly (2,ProfPoints,100000);
			if (GetPolyPoints ((LPPICKDATAHEADER)&PickList[0],FALSE,&nPnts,&hPoly, 0))
			{  
				HPDPOINT	pPolyPoints=GlobalLock (hPoly);
				DPOINT		IntPoint;
				double		D1, D2, d;
				int	na = IntersectPolys1 (GF_LINE,GF_AREA,2,ProfPoints,0,nPnts,pPolyPoints,0,0,&ProfPoints[0],&IntPoint,&D1,&D2,0);
				
				if (na)
				{
					rtn = TRUE;
					Points[0] = IntPoint;
					if (IntersectPolys1 (GF_LINE,GF_AREA,2,ProfPoints,0,nPnts,pPolyPoints,0,0,&ProfPoints[1],&IntPoint,&D1,&D2,0))
					{
						Points[1] = IntPoint;
						if (Points[0].y < CurView->WBounds.ymn || Points[0].y > CurView->WBounds.ymx ||
							Points[1].y < CurView->WBounds.ymn || Points[1].y > CurView->WBounds.ymx)
							doZoom = TRUE;
						MidPoint = MidPointD (Points[0],Points[1]);
						d = ldistp (Points[0],Points[1]);
						if (d > 0)
						{
							Scale = 1.5 * d / (CurView->ScreenRect.bottom - CurView->ScreenRect.top);
							if (Scale < CurView->Scale * 0.67)
								doZoom = TRUE;
						}
					}
				}
				GSSiGlobUlFree (&hPoly);
			}
		}
		SaveInShowZoomArea = InShowZoomArea;
		InShowZoomArea = TRUE;
		if (doZoom)
		{
			ZoomToPointAndScale(MidPoint, Scale, TRUE);
			LastProfileLocVP = 0;
			rtn = FALSE;
		}
		InShowZoomArea = SaveInShowZoomArea;
	}
	SaveDC (CurView->hDC); 
    SetDisplayMode (CurView->hDC,GF_SCREENMODE);
    SelectClipRgn (CurView->hDC,0);
	LastLine[0].x = LastLine[1].x = WinPoint.x;
	LastLine[0].y = CurView->ScreenRect.bottom;
	LastLine[1].y = CurView->ScreenRect.top;
	if (LastLine[0].x < CurView->ScreenRect.left || LastLine[0].x > CurView->ScreenRect.right)
		LastVP = 0;
	else
	{
		OldMode = SetROP2(CurView->hDC,R2_NOT);
		hPen = CreatePen (PS_SOLID,0,RGB(255,0,0));
		hOldPen = SelectObject (CurView->hDC,hPen);
		Polyline (CurView->hDC,LastLine,2);  
		SelectObject (CurView->hDC,hOldPen);
		DeleteObject (hPen);
   		SetROP2(CurView->hDC,OldMode);
	}
	SetCurView (SaveVP);
	RestoreDC (CurView->hDC,-1);  
	return rtn;
}

BOOL GetProfileElevAndSlope (double Dist,HANDLE hProfile,long nProfile,LPDOUBLE Elev, LPDOUBLE Slope,double NullElev)
#if ENABLETRACE
{GSSiEnterProg (1317);
#endif
{
	LPDPOINT pProfile;
	DWORD	i; 
	double	LastElev=NullElev, LastDist=0, inc;
	
	if (!hProfile || Dist < 0)
{
#if ENABLETRACE
GSSiExitProg (1317);
#endif
		return FALSE;
}
	
	pProfile = (LPDPOINT)GlobalLock (hProfile);
	for (i = 0;i < nProfile;i++,pProfile++)
	{
		if (pProfile->x >= Dist-P_TOL)
		{   
			if (LastElev == NullElev || pProfile->y == NullElev)
				break;          
			inc = (Dist - LastDist)/(pProfile->x - LastDist);
			*Elev =  (LastElev + (pProfile->y - LastElev) * inc);
			*Slope = 100 * (pProfile->y - LastElev)/(pProfile->x - LastDist);
			GlobalUnlock (hProfile);
{
#if ENABLETRACE
GSSiExitProg (1317);
#endif
			return TRUE;
}
		} 
		LastElev = pProfile->y;
		LastDist = pProfile->x;
	}
	GlobalUnlock (hProfile);
{
#if ENABLETRACE
GSSiExitProg (1317);
#endif
	return FALSE;
}
#if ENABLETRACE
}
#endif
} 

