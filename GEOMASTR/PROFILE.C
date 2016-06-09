#include "graphint.h"
#include "extrndb.h"
#include "gmextern.h"

BOOL InProfile=FALSE;

 
void SmoothProfile (HPDPOINT pProfile,long np)
{   
	long	n=GetGlobalLVal2 ("[%PROFILESMOOTHOPT]",0);
	long	i,j;
	double	y, w,totw; 
	
	if (n && np)
	{
		HANDLE	h2=GSSiGlobAlloc (0,GMEM_MOVEABLE,sizeof(DPOINT)*np);
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
			SetDisplayMode (CurView->hDC, GF_TEXTMODE);  
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

BOOL CreateNextCrossSection (int Direction)
{   
	BOOL		rtn=FALSE;

	if (CurView->pTheme && CurView->pTheme->ID == GF_PROFILE_THEME)
	{
		HPDPOINT	pRoute=(HPDPOINT)GlobalLock (CurView->hProfileRouteSave);
		double		Dist = CurView->LastProfileDist + Direction * CurView->pTheme->CrossSectionSpacing;
	
		if (Dist < 0 || Dist > GetPolyLengthD (pRoute,CurView->nProfileRouteSave))
			goto Exit;			
		CurView->LastProfilePoint = PointAtDistOnPoly (pRoute,CurView->nProfileRouteSave,Dist,&CurView->LastProfileAZ,NULL);
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

	hDIB = LoadDIB32 (TextureFile,FALSE);
	if (!hDIB)
		hPen = CreatePen (PS_SOLID,iWidth,0);
	else
	{
		GetBitmapInfoFromHandle (&DibInfo,hDIB);
		hBMP = GSSiGlobAlloc (0,GMEM_MOVEABLE,sizeof(BITMAPINFO));
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
	long	nPolyPoints=0, nProfilePoints[2]={256,256}, nRoute=0, nptosmooth;
	int		nRoutes, iRoute=0;
	HPDPOINT	pProfileD, pBeginSmooth,pPoly, pRoute, pRouteSave;
	HPPOINT     pProfile;
	LPPROFILETHEMEDATA	lpProfileData;   
	HIGHLIGHTDATA	HighlightData,HighlightData1;
	long	Seq, Refno; 
	short	pos=BT_FIRST, cond=BT_EQ, IncID;
	double	LenRoute, IncDist, Dist, ElevRange, MinElev, MaxElev, MinDist,d; 
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
   	COLORREF	PipeBrushColor=RGB(0,0,255), PipePenColor=0, MHColor=RGB(255,0,0),SymMarkerColor=RGB(0,255,0);
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
    GSSiDeleteObject(&CurView->hRgn);
    CurView->hRgn = CreateVPRgn(FALSE,FALSE);
    SelectClipRgn (CurView->hDC,CurView->hRgn);
    GSSiDeleteObject(&CurView->hRgn);    
    SetTextColor (CurView->hDC,0);     
	FillRectPoly (CurView->hDC,&CurView->Rect,ConvertColor(CurView->BackGroundColor,-1));  
	if (CurView->HaveFixedProfileRoute || (hHighlight && lpProfileData->MinSeq <= lpProfileData->MaxSeq))
	{
        hSurf[0] = DTMOpen (SurfName,DBL_MAX,BT_READ,&SurfType[0]);
        if (!hSurf[0])
        {
        	goto Exit;
        } 
        if (*CurTheme->SQL)
        {
	        hSurf[1] = DTMOpen (CurTheme->SQL,DBL_MAX,BT_READ,&SurfType[1]); 
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
			nRoute = CurView->nProfileRoute[0];
		   	pRoute = (HPDPOINT)GlobalLock (hRoute);
		}
	   	else if (CurView->ProfileInCrossSection)  
	   	{
			hRoute = GSSiGlobAlloc (1012,GMEM_MOVEABLE,(long)2*sizeof(DPOINT));
		   	pRoute = (HPDPOINT)GlobalLock (hRoute);
	   		pRoute[0] = CurView->ProfileCrossSection[0];
	   		pRoute[1] = CurView->ProfileCrossSection[1];  
	   		nRoute = 2;
			AlignWithRoute = FALSE;
	   	}
	   	else 
	   	{
			nRoutes = 1;
			hRoute = GSSiGlobAlloc (1012,GMEM_MOVEABLE,(long)4096*sizeof(DPOINT));
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
				   		if (GetPolyPoints ((LPPICKDATAHEADER)&HighlightData1.PD,Reverse,&nPolyPoints,&hPolyPoints))
				   		{
					   		pPoly = (HPDPOINT)GlobalLock (hPolyPoints);
					   		while (nPolyPoints--)
					   			pRoute[nRoute++] = *pPoly++;
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
			   		if (GetPolyPoints ((LPPICKDATAHEADER)&HighlightData.PD,Reverse,&nPolyPoints,&hPolyPoints))
			   		{
				   		pPoly = (HPDPOINT)GlobalLock (hPolyPoints);
				   		while (nPolyPoints--)
				   			pRoute[nRoute++] = *pPoly++;
			   			GSSiGlobUlFree (&hPolyPoints); 
			   		}
		   		} 
		   		else
		   		{
		   			HighlightData1 = HighlightData;  
		   			i++;
		   		}
	   		} while (Seq < lpProfileData->MaxSeq);
		    GSSiGlobFree (&CurView->hProfileRouteSave);
		    CurView->hProfileRouteSave = GSSiGlobAlloc (1012,GMEM_MOVEABLE,(long)nRoute*sizeof(DPOINT)); 
		    CurView->nProfileRouteSave = nRoute;
		   	GlobalUnlock (hRoute); 
		   	pRoute = (HPDPOINT)GlobalLock (hRoute);
		   	pRouteSave = (HPDPOINT)GlobalLock (CurView->hProfileRouteSave);  
		   	hmemmove ((HPSTR)pRouteSave,(HPSTR)pRoute,nRoute*sizeof(DPOINT));
		   	GlobalUnlock (CurView->hProfileRouteSave);
	   	}
EndRoute: 
		if (nRoute < 2 || !GridSpace) 
		{
			DTMClose (&hSurf[0]);
			DTMClose (&hSurf[1]);
			goto Exit;
		}
		LenRoute = GetPolyLengthD (pRoute,nRoute);  
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
				hProfileD[isurf] = GSSiGlobAlloc (1013,GMEM_MOVEABLE,(long)MAXPROFILEPOINTS*sizeof(DPOINT));
			   	pProfileD = (HPDPOINT)GlobalLock (hProfileD[isurf]);
			   	Dist = 0; 
		   		Point = PointAtDistOnPoly (pRoute,nRoute,Dist,NULL,NULL);   
		   		pProfileD->x = Dist;
		   		pProfileD++->y = NGIELV_bci (Point,hSurf[isurf],1);
		   		Dist = LenRoute;
		   		Point = PointAtDistOnPoly (pRoute,nRoute,Dist,NULL,NULL);   
		   		pProfileD->x = Dist;
		   		pProfileD->y = NGIELV_bci (Point,hSurf[isurf],1);
		   		GlobalUnlock (hProfileD[isurf]);  
		   		hProfilePoints = hProfileD[isurf];
		   		NumProfilePoints = 2;
				{ 
					LPVISLIST	SaveVis=CurVis;
					short	SaveMaxPick=MaxPick, SaveNFiles, SaveMT, SaveTimer, SavePT;
					HFILE	SaveFid; 
					HANDLE	hSaveVP = GSSiGlobAlloc (0,GMEM_MOVEABLE,sizeof(VIEWPORT)+256);  
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
					
					GetPolyBoundsD (hRoute,nRoute,&PolyBounds,TYPE_POLYLINE); 
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
					nProfileRoute = nRoute;
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
				hProfileD[isurf] = GSSiGlobAlloc (1013,GMEM_MOVEABLE,(long)nProfilePoints[isurf]*sizeof(DPOINT));
			   	pProfileD = (HPDPOINT)GlobalLock (hProfileD[isurf]); 
			   	Dist = 0; 
			   	nptosmooth = 0;
			   	pBeginSmooth = pProfileD;
			   	for (i=0;i<nProfilePoints[isurf];i++)
			   	{   
			   		Point = PointAtDistOnPoly (pRoute,nRoute,Dist,NULL,NULL);   
			   		pProfileD->x = Dist;
			   		pProfileD->y = NGIELV_bci (Point,hSurf[isurf],1);
if (isurf)
	pProfileD->y -= (42/12.0) / 3.2808333;
			   		if (pProfileD->y < DBL_MAX)
			   		{  
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
			   		Dist += IncDist;
			   	}  
				SmoothProfile (pBeginSmooth,nptosmooth); 
			   	GlobalUnlock (hProfileD[isurf]);  
//				if (!CheckForContinue (TRUE))
//					break;
		   	}
		}  
		WaitCursor (-1);
	   	GlobalUnlock (hRoute); 
		if (CurView->NumFiles)
		{
			MNMXCORD	PolyBounds;
					
			GetPolyBoundsD (hRoute,nRoute,&PolyBounds,TYPE_POLYLINE); 
			ExpandBounds (&PolyBounds,1);
			nProfileRoute = nRoute;
			hProfileRoute = hRoute;  
			hCurProfile = hProfileD[0];
			nCurProfile = nProfilePoints[0];
			hProfileSymbols = GSSiGlobAlloc (0,GMEM_MOVEABLE,USHRT_MAX);
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

	    sprintf (txt,"%.0f",MaxElev);
	    h = 20*DeviceToScreenFactor();
		TextExt = DispText (CurView->hDC,TRUE,p.x-10,p.x+10, -1,0, 2,2,h,1,1,2, FALSE,0,txt,0,FALSE,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0);
		MaxXText = LOWORD(TextExt); 
		MaxYText = HIWORD(TextExt);
		if (AlignWithRoute)
		{
			LPVIEWPORT	SaveVP=CurView;

			SetViewport (CurView->pTheme->TargetViewport);
			RectInit (&ProfileRect);
			for (i=0;i<nRoute;i++)
			{
				ScreenPoint = BasePtToScreenPt (&pRoute[i]);
				AddPointToRect (ScreenPoint,&ProfileRect);
			}
			CurView = SaveVP;
			ProfileRect.bottom = CurView->ScreenRect.bottom;
			ProfileRect.top = CurView->ScreenRect.top;
			Bounds.xmn = ProfileRect.left;
			Bounds.xmx = ProfileRect.right;
			InflateRect (&ProfileRect,0,-(MaxYText+4));
			if (VertScaleFactor)
			{
				double MidElev = (Bounds.ymx + Bounds.ymn) / 2;

	   			PixelsPerDistUnit = (ProfileRect.right - ProfileRect.left)/(Bounds.xmx - Bounds.xmn);
				Bounds.ymx = MidElev + (((ProfileRect.bottom - ProfileRect.top)/2)/PixelsPerDistUnit)/VertScaleFactor;
				Bounds.ymn = MidElev - (((ProfileRect.bottom - ProfileRect.top)/2)/PixelsPerDistUnit)/VertScaleFactor;
			}
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
			LPVIEWPORT	SaveVP=CurView;

			SetViewport (CurView->pTheme->TargetViewport);
		   	Point = PointAtDistOnPoly (pRoute,nRoute,Dist,NULL,NULL);   
			Point1 = BasePtToScreenPtD (&Point);
			CurView = SaveVP;
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
			if (AlignWithRoute)
			{
				LPVIEWPORT	SaveVP=CurView;

				SetViewport (CurView->pTheme->TargetViewport);
		   		Point = PointAtDistOnPoly (pRoute,nRoute,Dist,NULL,NULL);   
				Point1 = BasePtToScreenPtD (&Point);
				CurView = SaveVP;
			}
			else
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
		    hProfile = GSSiGlobAlloc (1014,GMEM_MOVEABLE,nProfilePoints[isurf]*sizeof(POINT));
		    pProfile = (HPPOINT)GlobalLock (hProfile);
		   	pProfileD = (HPDPOINT)GlobalLock (hProfileD[isurf]); 
		   	for (i=0;i<nProfilePoints[isurf];i++)
		   	{
		   		if (pProfileD[i].y < DBL_MAX)
		   		{ 
					DPOINT	ProfPoint = pProfileD[i], WPoint, ScreenPointD;

					if (AlignWithRoute)
					{
						LPVIEWPORT	SaveVP=CurView;

						SetViewport (CurView->pTheme->TargetViewport);
				  		WPoint = PointAtDistOnPoly (pRoute,nRoute,ProfPoint.x,NULL,NULL); 
						ScreenPointD = BasePtToScreenPtD (&WPoint);
						ProfPoint.x = ScreenPointD.x;
						CurView = SaveVP;
		 			}
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
			pProfileSymbols = (LPPROFILESYMBOLS)GlobalLock (hProfileSymbols); 
			for (iProfileSym=0;iProfileSym<NumProfileSymbols;iProfileSym++,pProfileSymbols++)
			{   
				DPOINT	DPoint = pProfileSymbols->Point;
				
				DPoint.y = pProfileSymbols->SurfElev;
				WLine[0] = TRANDPointToPoint (&DPoint,hTran); 
				DPoint.y += SymMarkerHeight;
				WLine[1] = TRANDPointToPoint (&DPoint,hTran);
				hPen = CreatePen (PS_SOLID,0,SymMarkerColor);
				hOldPen = SelectObject (CurView->hDC,hPen);      
		    	Polyline (CurView->hDC,WLine,2);
				SelectObject (CurView->hDC,hOldPen); 
				DeleteObject (hPen);    
				GetDictSymName (abs(pProfileSymbols->idesc),txt);
				{   
					LOGFONT	LogFont;    
				   	HFONT	OldFont,hFont;
					
					_fmemset (&LogFont,0,sizeof(LOGFONT));
					LogFont.lfHeight = 14*DeviceToScreenFactor(); 
					LogFont.lfEscapement = LogFont.lfOrientation = 900;   
					LogFont.lfWeight = FW_BOLD;    
					LogFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
					LogFont.lfQuality = PROOF_QUALITY; 
					_fstrcpy (LogFont.lfFaceName,"Times New Roman MT Extra Bold");    //Courier Bold New
					hFont = CreateFontIndirect((LPLOGFONT)&LogFont);
					OldFont = SelectObject (CurView->hDC,hFont);
					TextOut (CurView->hDC,(int)(WLine[1].x-7*DeviceToScreenFactor()),WLine[1].y,txt,_fstrlen(txt));	
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
			CurView->hProfileDataRectangles = GSSiGlobAlloc (0,GMEM_MOVEABLE,USHRT_MAX);
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
							if (!GetProfileX (&DPoint,nRoute,pRoute,&PointOnRoute,&OffDist,&PolyDist))
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
							if (!GetProfileX (&DPoint,nRoute,pRoute,&PointOnRoute,&OffDist,&PolyDist))
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
							if (!GetProfileX (&DPoint,nRoute,pRoute,&PointOnRoute,&OffDist,&PolyDist))
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
						HANDLE hPoints3D = GSSiGlobAlloc (0,GMEM_MOVEABLE,maxPoints3D * sizeof(DPOINT3D));
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
						hPnts = GSSiGlobAlloc (0,GMEM_MOVEABLE,nPnt3D*sizeof(POINT));
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
	    p.y = ProfileRect.bottom;
	    sprintf (txt,"%.0f",MaxElev);
		GetTextExtentPoint32 (CurView->hDC,txt,_fstrlen(txt),&txSize);
	    sprintf (txt,"%.0f",MinElev);
		twidth = txSize.cx+2; 
	    p.x = min (CurView->ScreenRect.right-twidth,ProfileRect.right) + 2;
		DispText (CurView->hDC,FALSE,p.x,p.x, p.y,0, 1,2,h,1,1,2, FALSE,0,txt,0,FALSE,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0);
	    p.x = max (CurView->ScreenRect.left+twidth,ProfileRect.left) - 2;
		DispText (CurView->hDC,FALSE,p.x,p.x, p.y,0, 4,2,h,1,1,2, FALSE,0,txt,0,FALSE,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0);
	    p.y = ProfileRect.top;
	    sprintf (txt,"%.0f",MaxElev);
	    p.x = min (CurView->ScreenRect.right-twidth,ProfileRect.right) + 2;
		DispText (CurView->hDC,FALSE,p.x,p.x, p.y,0, 1,2,h,1,1,2, FALSE,0,txt,0,FALSE,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0);
	    p.x = max (CurView->ScreenRect.left+twidth,ProfileRect.left) - 2;
		DispText (CurView->hDC,FALSE,p.x,p.x, p.y,0, 4,2,h,1,1,2, FALSE,0,txt,0,FALSE,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0);
	    
	    
	    CurView->hFileTransIn = STRANRectToBounds (&ProfileRect,&Bounds); 
	    if (hRoute != CurView->hProfileRoute[iRoute])
			GSSiGlobFree (&CurView->hProfileRoute[iRoute]);
	    CurView->hProfileRoute[iRoute] = hRoute;  
	    CurView->nProfileRoute[iRoute] = nRoute;
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

BOOL DisplayProfileLoc (LPPOINT pWinPoint,LPDPOINT BasePoint,LPVIEWPORT pVP,BOOL LoadConfig)
#if ENABLETRACE
{GSSiEnterProg (1316);
#endif
{   
	static	POINT	LastLine[2];
	static	LPVIEWPORT	LastVP=0; 
	LPVIEWPORT	SaveVP=CurView;  
	short	OldMode; 
	HPEN	hPen, hOldPen;  
	double	LastDist;
	
	if (LoadConfig) 
	{
		LastVP = NULL;
{
#if ENABLETRACE
GSSiExitProg (1316);
#endif
		return FALSE;
}
	}
	if (LastVP)
	{ 
		SaveDC (CurView->hDC);
	    SetDisplayMode (CurView->hDC,GF_SCREENMODE);
	    SelectClipRgn (CurView->hDC,0);
		SetCurView (LastVP);
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
		LastVP = NULL;
{
#if ENABLETRACE
GSSiExitProg (1316);
#endif
		return FALSE;
}
	}
	LastVP = pVP;
	if (BasePoint->x < 0)
	{
		LastVP = 0;
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
			LastVP = 0;
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
	if ((WinPoint.x < CurView->ScreenRect.left || WinPoint.x > CurView->ScreenRect.right) && GetGlobalBVal2 ("[%AUTOPROFILEPAN]",TRUE))
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
			if (GetPolyPoints ((LPPICKDATAHEADER)&PickList[0],FALSE,&nPnts,&hPoly))
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
						MidPoint = MidPointD (Points[0],Points[1]);
						d = ldistp (Points[0],Points[1]);
						if (d > 0)
							Scale = 1.5 * d / (CurView->ScreenRect.bottom - CurView->ScreenRect.top);
					}
				}
				GSSiGlobUlFree (&hPoly);
			}
		}
		SaveInShowZoomArea = InShowZoomArea;
		InShowZoomArea = TRUE;
		ZoomToPointAndScale (MidPoint,Scale,TRUE);
		InShowZoomArea = SaveInShowZoomArea;
		rtn = FALSE;
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

