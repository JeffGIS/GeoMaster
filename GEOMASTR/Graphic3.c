#include "graphint.h"
#include "extrndb.h" 

#include "gmextern.h"


typedef	struct	{char	Name[32];
				 long	Refno;
				}MUNICDATA1;
				
typedef	struct	{char	Name[32];
			 	long	FIPS;
			 	long	x,y;
			 	}MUNICDATA2; 
			 	
struct	{char Name[34]; MNMXCORD Bounds;} OtherZoomRec;
static MUNICDATA1	MunicData1;
static MUNICDATA2	MunicData2;








BOOL PickCurve (HPDPOINT lpPointsIn,long nPnts,LPDPOINT PC,LPDPOINT POC, LPDPOINT PT)
#if ENABLETRACE
{GSSiEnterProg (1015);
#endif
{
	DPOINT	BeginPoint, EndPoint, PickedPoint,NodePoint, WPoint, LastWPoint, NearPoint; 
	HPDPOINT lpPoints = lpPointsIn;
	POINT	BeginPointFile, EndPointFile, PickedPointFile;
	DWORD	i, mini;
	double	PCT, OffDist, TotDistW, AZ[3], MinDist, Dist, AZAdjust;
	BOOL	First=TRUE, rtn=FALSE;   
	MNMXCORD	Rect;    
	short	st;
	long	FromPtID;

	if (PickNET)
	{
		if (!RefInNet (CurrentRefno))
{
#if ENABLETRACE
GSSiExitProg (1015);
#endif
			return FALSE;
}
	}
	st = RCURVE(&PC->x,&PC->y,&POC->x,&POC->y,&PT->x,&PT->y,&CurRP.x,&CurRP.y,&CurCLEN); 
    if (hHighlightArea)
{
#if ENABLETRACE
GSSiExitProg (1015);
#endif
    	return (PickPolyInAreaD(1,lpPoints,nPnts,0,0,0,0,0,0,0));
}
	
	TotDistW=0;   
	BeginPoint = *lpPoints; 
	if (CurCLEN < 0)
		AZAdjust = HALFPI;
	else
		AZAdjust = -HALFPI; 
	AZ[0] = LTWOPI(getazd (&CurRP,PC)+AZAdjust);
	AZ[2] = LTWOPI(getazd (&CurRP,PT)+AZAdjust);
	MinDist = DBL_MAX;
	DBoundsInit (&Rect);
	for (i=0;i<nPnts;i++,lpPoints++)
	{   
		AddDPointToMinMax (lpPoints,&Rect);
		WPoint = *lpPoints;
		if (!First) 
			TotDistW += ldistp(LastWPoint,WPoint);
		else
			First = FALSE;
		LastWPoint = WPoint;  
		Dist = ldistp (PickPointBase,WPoint);
		if (Dist < MinDist)
		{
			MinDist = Dist;
			NearPoint = WPoint;
			mini = i;
		}
	}
	EndPoint = LastWPoint;
    if (PickingByRefno)
    {
    	PCT = 0.5;
    	OffDist = 0;
    	goto Add;
    }
	if (!LineInBoundsD (nPnts,lpPointsIn,TotDistW,&PCT,&OffDist,&AZ[1],
					    &CurView->WBounds,&PickedPoint,&FromPtID))
		goto Exit;
    if (WantOnlyShapePoints) 
    {
		MinDist = ldistp (PickPointBase,lpPointsIn[0]);
		NearPoint = lpPointsIn[0];
		mini = 0;
		Dist = ldistp (PickPointBase,lpPointsIn[nPnts-1]);
		if (Dist < MinDist)
		{
			MinDist = Dist;
			NearPoint = lpPointsIn[nPnts-1];
			mini = nPnts-1;
		}
	   	PickedPoint = NearPoint;  
    	OffDist = ldistp (PickPointBase,PickedPoint);  
    	if (OffDist > PickApW)
    		goto Exit;
    }
	for (i=0; i<NumPicked; i++)
	{
		if (CurrentRefno == PickList[i].Refno)
		{
			if (fabs(OffDist)<fabs(PickList[i].OffDist))
				PickListDelete((short)i);
			else
				goto Exit;
		}
	} 
Add: 
	rtn = TRUE;  
	if (!PickingByRefno)
	{
		PickedPointFile = BasePtToFilePt (PickedPoint);
		BeginPointFile = BasePtToFilePt (*PC);
		EndPointFile = BasePtToFilePt (*PT);
	}
	PickListAdd (FileNum,SubFile,FileInIndex,ItemSeg,CurrentRefno,CurrentDesc,0,
				   GF_CURVE,PCT,OffDist,AZ,fabs(CurCLEN),CurrentItem,CurElement,
				   *PC,*PT,PickedPoint,*POC,mini,0,&Rect,
				   BeginPointFile, EndPointFile,PickedPointFile,nPnts,NULL_ELEV,PC,PT);
Exit:
{
#if ENABLETRACE
GSSiExitProg (1015);
#endif
	return (rtn);
}
#if ENABLETRACE
}
#endif
} 

BOOL PickedItemMinMax (int Item, LPMNMXCORD lpRect)
#if ENABLETRACE
{GSSiEnterProg (694);
#endif
{	LPSHORT		ipnt, EndItem;
    HANDLE 		hpltBuf=0;
	LPSTR		LPpltBuf;
	ITEM		*ItemHeader;
	HDC			hDC;
	OFSTRUCTGM	OFStruct;
	BOOL		rtn=FALSE, SaveDisplay, SavePick, SavePT = ProcessThemes; 
	WORD		nBytes, nRead;
	
	if (!FastPick || FidMap == HFILE_ERROR ||
		PickList[Item].FileNum != FastPickFileNum ||
		PickList[Item].FileInIndex != FastPickFII)
	{   
		if (FastPick)
		{
			FastPickFileNum = PickList[Item].FileNum; 
			FastPickFII = PickList[Item].FileInIndex;
		}
	    if (!GetPickName (Item))
{
#if ENABLETRACE
GSSiExitProg (694);
#endif
	    	return FALSE;
}
	
		if (!PickName[0])
{
#if ENABLETRACE
GSSiExitProg (694);
#endif
			return FALSE;
}
		_fstrcpy (PltName,PickName);
	
		CloseMap (FALSE); 
		if (!OpenMap (CurView->hWnd,CurView->hDC))
{
#if ENABLETRACE
GSSiExitProg (694);
#endif
			return FALSE; 
}
	} 
	if (MapType == MT_SHP)
	{
	    CurView->PassID=4;
    	CurrentSHPRec = PickList[Item].Segment;
    	SHPRecOffset = GetSHPRecordOffset (CurrentSHPRec,FALSE);
		rtn = ReadSHPRecordHeader (FidMap,SHPRecOffset,lpRect);
		SavePick = Pick;
		Pick=FALSE; 
		IgnoreBounds = TRUE;
		ItemIsDeleted = FALSE; 
		SaveDisplay = Display;
		Display = FALSE; 
		ProcessThemes = FALSE;
	    ProcessSHPRecord (0,FidMap,CurrentSHPRec); 
	    ProcessThemes = SavePT;    
	    Display = SaveDisplay;  
	    Pick = SavePick;
		ProcessSingleItem=HaveFirstHeader=FALSE;
	    IgnoreBounds = FALSE;  
		goto Exit;
	}
	if (MapType == MT_PERSONAL_GEO_DB)
	{
	    CurView->PassID=4;
    	CurrentSHPRec = PickList[Item].Segment;
    	SHPRecOffset = GetSHPRecordOffset (CurrentSHPRec,FALSE);
		rtn = ReadPGDBRecordHeader (lpRect);
		SavePick = Pick;
		Pick=FALSE; 
		IgnoreBounds = TRUE;
		ItemIsDeleted = FALSE; 
		SaveDisplay = Display;
		Display = FALSE; 
		ProcessThemes = FALSE;
	    ProcessSHPRecord (0,FidMap,CurrentSHPRec); 
	    ProcessThemes = SavePT;    
	    Display = SaveDisplay;  
	    Pick = SavePick;
		ProcessSingleItem=HaveFirstHeader=FALSE;
	    IgnoreBounds = FALSE;  
		goto Exit;
	}
	if (MapType == MT_FILE_GEO_DB)
	{
	    CurView->PassID=4;
    	CurrentSHPRec = PickList[Item].Segment;
    	SHPRecOffset = GetSHPRecordOffset (CurrentSHPRec,FALSE);
		rtn = ReadPGDBRecordHeader (lpRect);
		SavePick = Pick;
		Pick=FALSE; 
		IgnoreBounds = TRUE;
		ItemIsDeleted = FALSE; 
		SaveDisplay = Display;
		Display = FALSE; 
		ProcessThemes = FALSE;
	    ProcessSHPRecord (0,FidMap,CurrentSHPRec); 
	    ProcessThemes = SavePT;    
	    Display = SaveDisplay;  
	    Pick = SavePick;
		ProcessSingleItem=HaveFirstHeader=FALSE;
	    IgnoreBounds = FALSE;  
		goto Exit;
	}
	else if (MapType == MT_ORA)
	{
	    CurView->PassID=4;
    	CurrentORARec = PickList[Item].Segment;
    	ORARecOffset = GetORARecordOffset (CurrentORARec,FALSE);
		rtn = ReadORARecordHeader (FidMap,ORARecOffset,lpRect);   
		SavePick = Pick;
		Pick=FALSE; 
		IgnoreBounds = TRUE;
		ItemIsDeleted = FALSE; 
		SaveDisplay = Display;
		Display = FALSE; 
		ProcessThemes = FALSE;
	    ProcessORARecord (0,FidMap,CurrentORARec); 
	    ProcessThemes = SavePT;    
	    Display = SaveDisplay;  
	    Pick = SavePick;
		ProcessSingleItem=HaveFirstHeader=FALSE;
	    IgnoreBounds = FALSE;  
		goto Exit;
	}
	else if (MapType == MT_GMD)
	{
	    CurView->PassID=4;
    	CurrentGMDRec = PickList[Item].Segment;
    	GMDRecOffset = GetGMDRecordOffset (CurrentGMDRec,FALSE);
		rtn = ReadGMDRecordHeader (FidMap,ORARecOffset,lpRect);   
		SavePick = Pick;
		Pick=FALSE; 
		IgnoreBounds = TRUE;
		ItemIsDeleted = FALSE; 
		SaveDisplay = Display;
		Display = FALSE; 
		ProcessThemes = FALSE;
	    ProcessGMDRecord (0,(HANDLE)FidMap,CurrentGMDRec); 
	    ProcessThemes = SavePT;    
	    Display = SaveDisplay;  
	    Pick = SavePick;
		ProcessSingleItem=HaveFirstHeader=FALSE;
	    IgnoreBounds = FALSE;  
		goto Exit;
	}
	else if (MapType == MT_DGN7)
	{
	    CurView->PassID=4;
    	CurrentDGNRec = PickList[Item].Segment;
    	rtn = GetDGNRecordBounds (CurrentDGNRec,lpRect);
		SavePick = Pick;
		Pick=FALSE; 
		IgnoreBounds = TRUE;
		ItemIsDeleted = FALSE; 
		SaveDisplay = Display;
		Display = FALSE; 
		ProcessThemes = FALSE;
		ProcessDGNRecord (CurView->hDC,CurrentDGNRec); 
	    ProcessThemes = SavePT;    
	    Display = SaveDisplay;  
	    Pick = SavePick;
		ProcessSingleItem=HaveFirstHeader=FALSE;
	    IgnoreBounds = FALSE;  
		goto Exit;
	}
	else if (MapType == MT_GPX)
	{
		CurView->PassID=4;
    	CurrentGPXRec = PickList[Item].Segment;
    	GetGPXRecordBounds (CurrentDGNRec,lpRect);
	}
	
    if (GSSillseek (FidMap,PickList[Item].Segment,0) != PickList[Item].Segment)
    	goto Exit2;
    nRead = BigRead (FidMap,(HPSTR)&nBytes,2);
    if (nRead != 2)
    	goto Exit2; 
    if (!nBytes)
    {
    	InvalidItem (0,TRUE);
    	goto Exit;
    }
    hpltBuf = GSSiGlobAlloc ( 170,GMEM_MOVEABLE,(DWORD)nBytes+4);
    LPpltBuf = GlobalLock (hpltBuf);
    nRead = BigRead (FidMap,LPpltBuf,nBytes); 
    if (nRead != nBytes || PickList[Item].Offset > nRead) 
    {
    	InvalidItem (0,TRUE);
    	goto Exit;
    }
    ipnt = (LPSHORT)(LPpltBuf + PickList[Item].Offset);
    ItemHeader = (LPITEM) ipnt;  
    if (InvalidItem (ItemHeader,TRUE))
    	goto Exit;
	GetItemMinMax (&ItemHeader->MinMax,lpRect); 
    ProcessSingleItem = TRUE; 
	if (ItemHeader->Len) 
	{
	    EndItem = ipnt + abs(ItemHeader->Len);
	    EndItem+=6;
	    *EndItem = 0; 
	} 
	SavePick = Pick;
	Pick=FALSE; 
	IgnoreBounds = TRUE;
	ItemIsDeleted = FALSE; 
	SaveDisplay = Display;
	Display = FALSE; 
	ProcessThemes = FALSE;
    ContinuationOffset = -1;
	while (ProcessGraphicsRec (0,ipnt,LPpltBuf,nRead))
    {
		GSSiGlobUlFree (&hpltBuf);
	    GSSillseek (FidMap,ContinuationOffset,0);
	    nRead = BigRead (FidMap,(HPSTR)&nBytes,2);
	    hpltBuf = GSSiGlobAlloc ( 319,GMEM_MOVEABLE,(DWORD)nBytes+2);
	    LPpltBuf = GlobalLock (hpltBuf);
	    nRead = BigRead (FidMap,LPpltBuf,nBytes);
		ipnt = (LPSHORT)LPpltBuf;
    } 
    ProcessThemes = SavePT;    
    Display = SaveDisplay;  
    Pick = SavePick;
	ProcessSingleItem=HaveFirstHeader=FALSE;
    IgnoreBounds = FALSE;  
    if (ItemIsDeleted == 2) 
    {
		rtn = TRUE;
    	CurrentType = GF_DELETE;
    }
    else if (ItemIsDeleted)
    	rtn = FALSE;
    else 
    {
		if (PickingByRefno)
		{   
			PickList[Item].Refno = CurrentRefno;
			PickList[Item].Desc = CurrentDesc;
			PickList[Item].PickedPoint.x = (lpRect->xmn + lpRect->xmx)/2;    
			PickList[Item].PickedPoint.y = (lpRect->ymn + lpRect->ymx)/2;
		}    
	    rtn = TRUE; 
	}
Exit:
    Pick=FALSE;         
	GSSiGlobUlFree (&hpltBuf);
Exit2:
	if (!FastPick)
		CloseMap(FALSE);
	ItemIsDeleted = FALSE; 
{
#if ENABLETRACE
GSSiExitProg (694);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}  

int	PickItems (HWND hWnd,DPOINT InPickPointBase) 
#if ENABLETRACE
{GSSiEnterProg (695);
#endif
{
	HCURSOR	hcurSave;  
	int		Rtn; 
	BOOL	SavePickPointSymbol=PickPointSymbol;
	
    PickPointBase = InPickPointBase;
    
	if (!MemMap && hWnd)
	{	
		hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT)); 
		HaltMapDisplay (FALSE,TRUE);  
	}
    SetPickAp(&PickPointSymbol); 
    
    Rtn = PickItems2 (hWnd,PickPointBase,FALSE,TRUE,TRUE);
    
    if (Rtn)
    {
    	if (PickList[Rtn-1].OffDist < 9999999 && PickList[Rtn-1].OffDist > PickApW)
    		Rtn = 0;
    	else
    		SetPickGlobals (Rtn-1);
    }
	if (!MemMap && hWnd)
		GSSiSetCursor (hcurSave);   
	PickPointSymbol = SavePickPointSymbol;
{
#if ENABLETRACE
GSSiExitProg (695);
#endif
	return Rtn;
}
#if ENABLETRACE
}
#endif
}

short GetNextPickFile (LPLONG pStartRef)
#if ENABLETRACE
{GSSiEnterProg (696);
#endif
{    
	BOOL	rtn=0;
	
	if (!DisplayOnlyHLT)
	{
		if (GetNextViewportFile (FALSE))
			rtn = 1;
	}
	else
	{
	    HIGHLIGHTDATA	HighlightData;
	    LPVIEWPORT		SaveVP;  
	    HANDLE			hSaveVP; 
	    short			pos=BT_FIRST, cond=BT_GE; 
    
		if (hHighlight)
	    {
			hSaveVP = GSSiGlobAlloc ( 171,GMEM_MOVEABLE,sizeof(VIEWPORT));
			SaveVP = (LPVIEWPORT)GlobalLock (hSaveVP);
			*SaveVP = *CurView;
	Next:	
			if (!BT_FIND (hHighlight,(LPSTR)pStartRef,pos,cond,(LPSTR)&HighlightData))
			{   
				pos = BT_NEXT;
				cond = BT_ANY;
				if (!RectInWBounds (&HighlightData.PD.Rect,1))
					goto Next;
				PickList[MAXPICKITEMS-1] = HighlightData.PD;
				GetPickName (MAXPICKITEMS-1);  
				_fstrcpy (PltName,PickName);
				PltType = 2;
				rtn = 6;
				(*pStartRef)++;	 
			}
			*CurView = *SaveVP;
			GSSiGlobUlFree (&hSaveVP);
		} 
	}
{
#if ENABLETRACE
GSSiExitProg (696);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
} 

int	PickItems2 (HWND hWnd,DPOINT InPickPointBase,BOOL QuitOnMouseMove,BOOL ResetNP, BOOL SelectVis)
#if ENABLETRACE
{GSSiEnterProg (698);
#endif
{	MNMXCORL	SaveBounds;
	MNMXCORD	SaveWBounds, SaveNewBounds, SavePBounds;
	
	int			iPickAp, idum, iType, DoMultiPass=0,ii; 
	POINT		PickPoint, PickPoint2;
	HANDLE		hRestoreVis = 0;
	LPVISLIST	SaveVis, pRestoreVis;
	LPVIEWPORT	SaveVP;
	POINT		WinPT1, WinPT2; 
	DPOINT		WPoint1, WPoint2, PickPointBase2; 
	BOOL		SavePick=Pick, Rtn; 
	long		NextRef=LONG_MIN;
    BOOL		SavePrint=Printing;
	BOOL		SaveDisplay = Display;

	Display = FALSE;
    
/*    char	str[128];
    static	short	np=1;
    sprintf (str,"Pick %i",np++);
    SetWindowText (hWndMain,str);*/

PickPointBase = InPickPointBase;
	SetGlobalValueReal ("%PICK_POINT_X",PickPointBase.x);
	SetGlobalValueReal ("%PICK_POINT_Y",PickPointBase.y); 
	CurPickPoint = PickPointBase;   
	if (ResetNP)
		NumPicked=0;
    if (PickAp == 0)
    {
		Rtn = PickNearestItems (PickPointBase,SelectVis);
		Display = SaveDisplay;
{
#if ENABLETRACE
GSSiExitProg (698);
#endif
		return (Rtn);
}
	}
	SaveVis = CurVis;
    PrevLayerVP = 0;
    SaveVP = CurView;
	SaveBounds = CurView->Bounds;
	SaveWBounds = CurView->WBounds; 
	SaveNewBounds = CurView->NewBounds;
	
	
	CurView->WBounds.xmn = PickPointBase.x - PickApW;
	CurView->WBounds.xmx = PickPointBase.x + PickApW;
	CurView->WBounds.ymn = PickPointBase.y - PickApW;
	CurView->WBounds.ymx = PickPointBase.y + PickApW;     
	SavePBounds = CurView->WBounds;
    SetLocalProjection (&CurView->WBounds);
	PickPointBase2 = PickPointBase;
	PickPointBase2.x += PickApW; 
	
	if (SelectVis)
		SelectVisList (TRUE);
	if (StopAtFirstInPickMacro)
	{   
		if (CurVis && (CurVis->WantType[1] || CurVis->WantType[2] || CurVis->WantType[3]))
		{
			DoMultiPass = 2;
			hRestoreVis = GSSiGlobAlloc ( 172,GMEM_MOVEABLE,sizeof(VISLIST));
			pRestoreVis = (LPVISLIST)GlobalLock (hRestoreVis);
			*pRestoreVis = *CurVis;
			GlobalUnlock (hRestoreVis); 
			CurVis->WantType[0] = 0; 
		}
	}
	Pick=TRUE;
	PickVehicles (PickPointBase,PickAp,0);  
	if (StopAtFirstInPickMacro && NumPicked>0)
	{   
		int i=NumPicked;
		
		while (i--)
		{
			if (ItemInPickMacro (i))  
				goto Exit; 
			else
				NumPicked--;
		}
	}
NextPass:
    CurView->PassID = 0; 
	CurView->CurFile=-1;
	switch (DoMultiPass)
	{
		case 1: 
			pRestoreVis = (LPVISLIST)GlobalLock (hRestoreVis);
			*CurVis  = *pRestoreVis;
			GlobalUnlock (hRestoreVis); 
			CurVis->WantType[1]=CurVis->WantType[3]=0;  
			CurView->PassID = 2;
			DoMultiPass = 0;
		break;
		case 2:
			CurView->PassID = 3;
			DoMultiPass--;
		break;
	}
		
//	IncrementFile ();
	Printing = TRUE;
	PickName[0] = '\0';
	while ((iType = GetNextPickFile (&NextRef)))
	{   
		CurView->WBounds = SavePBounds;	
		if (PltType == 3)
			idum=0;
		else if (PltType < 5 || (PltType == 5 && !PickOrtho))
		{
			DisplayPlotInit(hWnd,TRUE);
			if (OpenMap (hWnd, (HDC)1))
			{   
				_fstrcpy (PickName,PltName);  
				if (RectInWBounds(&CurView->FileMNMX,0))
				{   
					int	BoundsInc;
					int	itheme;
					LPTHEME	SaveTheme=CurTheme;
					
					MinPickItemWidth = CurView->MaxFileDisplayedPointWidth[FileNum];
					PickPoint = BasePtToFilePt(PickPointBase);
					PickPoint2 = BasePtToFilePt(PickPointBase2); 
					if (!CurView->FileFactor)
						CurView->FileFactor=1;
					iPickAp = max (2,idist(PickPoint,PickPoint2)/CurView->FileFactor);
					PickPoint.x = PickPoint.x/CurView->FileFactor;
					PickPoint.y = PickPoint.y/CurView->FileFactor;
					BoundsInc  = iPickAp + MinPickItemWidth/2;
					CurView->Bounds.xmn = PickPoint.x - BoundsInc;
					CurView->Bounds.ymn = PickPoint.y - BoundsInc;
					CurView->Bounds.xmx = PickPoint.x + BoundsInc;
					CurView->Bounds.ymx = PickPoint.y + BoundsInc;  
					if (MinPickItemWidth)
						FileBoundsLToWBounds (&CurView->Bounds,&CurView->WBounds); 
					for (itheme = 0;itheme<CurView->NumThemes;itheme++)         //pViewports[27]
					{
						CurTheme = CurView->pThemes[itheme];
						if (CurTheme->IsActive && CurTheme->VPDisplayed && !CurTheme->hVisList)
						{
							if (CurTheme->SymNum > 0)  
							{
								GSSiGlobFree (&CurTheme->hVisList);
								CurTheme->hVisList = SetThemeVisList (CurTheme->SymNum);  
							} 
							else if (CurTheme->SymNum == -2) 
							{
								GSSiGlobFree (&CurTheme->hVisList);
								CurTheme->hVisList = ReadVisList (&CurTheme->Contents[1]);  
							}
						}
					}
					CurTheme = SaveTheme;
			        if (iType == 6)
						ProcessPickedItem (MAXPICKITEMS-1,FALSE);
			        else
						while (DisplaySeg (&NULLHDC,FALSE))
						{   
							if (StopAtFirstInPickMacro && NumPicked)  
							{   
								int i=NumPicked;
								
								while (i--)
								{
									if (ItemInPickMacro (i))  
									{
									    CloseMap (FALSE); 
								        if (CurView->SubFile)
								        {
								            _fstrcpy (CurView->lpFiles[CurView->RestoreFile],CurView->OrigFile); 
								            CurView->SubFile =CurView->RestoreFile = 0;
								        } 
										goto Exit; 
									}
									else
										NumPicked = i;
								}
							}
							if (!ContinuePicking (QuitOnMouseMove))  
							{
							    CloseMap (FALSE); 
								goto Exit; 
							}
						}
				} 
				else
				    CloseMap (FALSE); 
			}
			if (!ContinuePicking (QuitOnMouseMove)) 
				goto Exit;   
		
		}
		else if (hOrthos)
		{   
			DPOINT	OrthoPoints[4];
			LPORTHO		CurOrtho = (LPORTHO)GlobalLock (hOrthos);    
			short	OrthoSym;

			OrthoPoints[0].x=CurOrtho->Bounds.xmn;
			OrthoPoints[0].y=CurOrtho->Bounds.ymn;
			OrthoPoints[1].x=CurOrtho->Bounds.xmn;
			OrthoPoints[1].y=CurOrtho->Bounds.ymx;
			OrthoPoints[2].x=CurOrtho->Bounds.xmx;
			OrthoPoints[2].y=CurOrtho->Bounds.ymx;
			OrthoPoints[3].x=CurOrtho->Bounds.xmx;
			OrthoPoints[3].y=CurOrtho->Bounds.ymn;
			if (POINT_IN_AREAD (PickPointBase, 4, OrthoPoints,1,0,0,0))  
			{
				if (PickOrtho)
					_fstrcpy (PickedOrthoName,CurOrtho->OrigName); 
				else if ((OrthoSym = GetDictSymbolNumber ("ORTHOPHOTO")))
				{   
					char	OrthoID[64];
					
					CurrentDesc = OrthoSym;
					_fstrcpy (CurrentPrefix,"ORTHO");  
					GetGlobalCVal ("[%ORTHOID]",OrthoID,"Unknown");
					strncpy0 (CurrentUDI,OrthoID,MAX_UDI_LEN);  
					SetGlobalValue2 (hPrefix,CurrentPrefix,0); 
					SetUDIValue (CurrentPrefix,CurrentUDI);
					PickPolygonD (OrthoPoints,4,0,0,9999999,0); 
				} 
			}
			GlobalUnlock (hOrthos);
		}
	}
	if (DoMultiPass)
		goto NextPass;
	PickItemBeingDigitized ();
	PickDistanceDisplayLines ();
	PickDispersedPoints (PickPointBase,PickAp,0);  
	PickProfileRectangles (PickPointBase,PickAp,0);  
	PickDataDisplayRectangles (PickPointBase,PickAp,0);  
Exit:
	if (hRestoreVis)
	{
		pRestoreVis = (LPVISLIST)GlobalLock (hRestoreVis);
		*CurVis  = *pRestoreVis;
		GSSiGlobUlFree (&hRestoreVis); 
    }
	CloseDisplayedRefs();
	ClosePrevLayers ();
    CloseAllIndexes ();
    CloseMap (FALSE); 
    CloseRefIndex(FALSE);  
    MinPickItemWidth = 0;
    Printing = SavePrint;
    if (Printing)
    	ii=1;
	Pick=SavePick; 
	CurView = SaveVP;
	CurView->Bounds = SaveBounds;
	CurView->WBounds = SaveWBounds; 
	CurView->NewBounds = SaveNewBounds;
//	SetDisplayMode (CurView->hDC,GF_TEXTMODE);
//	SelectClipRgn (CurView->hDC,0);
	CurVis = SaveVis;  
	Display = SaveDisplay;
{
#if ENABLETRACE
GSSiExitProg (698);
#endif
	return (NumPicked);
}

#if ENABLETRACE
}
#endif
} 

BOOL PickDistanceDisplayLines (void)
{   
	HPDPOINT	lpPoints;
	BOOL		rtn;
	LPLONG		pNumDistPoints;
	
	if (!CurView->hDistanceLine)
		return FALSE; 
	CurrentRefno = DISTANCEDISPLAYLINEREF;
	CurrentDesc = -1;
	SetGlobalValue ("%PREFIX","%DISTDSP"); 
	pNumDistPoints = (LPLONG)GlobalLock (CurView->hDistanceLine);
	lpPoints = (HPDPOINT)(pNumDistPoints+1);
	FileNum = -1;
	if (*pNumDistPoints < 0)
	{
		SetGlobalValue ("%UDI","Line"); 
		rtn = PickPolylineD (lpPoints,abs(*pNumDistPoints),0,2,0,0,0);
	}
	else
	{
		SetGlobalValue ("%UDI","Area"); 
		rtn = PickPolygonD (lpPoints,*pNumDistPoints,0,0,0,0);
	}
	GlobalUnlock (CurView->hDistanceLine);
	return rtn;
}

BOOL PickItemBeingDigitized (void)
#if ENABLETRACE
{GSSiEnterProg (699);
#endif
{   
	HPDPOINT	lpPoints;
	BOOL		rtn;
	
	if (!hCurPolyPoints||!nCurPolyPoints)
{
#if ENABLETRACE
GSSiExitProg (699);
#endif
		return FALSE; 
}
	CurrentRefno = ITEMBEINGDIGITIZEDREF;
	lpPoints = (HPDPOINT)GlobalLock (hCurPolyPoints);
	FileNum = -1;
	rtn = PickPolylineD (lpPoints,nCurPolyPoints,0,2,0,0,0);
	GlobalUnlock (hCurPolyPoints);
{
#if ENABLETRACE
GSSiExitProg (699);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}

short GetPickFile (short PickLayerID)
#if ENABLETRACE
{GSSiEnterProg (677);
#endif
{   
	short	i, iview;
    char    File[128], drive[8], dir[128], leaf[40], LastFile[128];
	short PickFile;
	
	if (PickLayerID == -1) //edit file
	{   
		if (CurView->UpdateFile)
			PickFile = (CurView->UpdateFile-1)+CurView->ID*256;
		else
			PickFile = -1;
{
#if ENABLETRACE
GSSiExitProg (677);
#endif
    	return PickFile;
}
    }
	*LastFile=0;
	for (iview=0;iview<*pNumViewports;iview++)
	{   
    	if (pViewports[iview])      
		for (i=0;i<pViewports[iview]->NumFiles;i++)
	    {
	        _fstrcpy (File,pViewports[iview]->lpFiles[i]);
		    ExpandText (File);
		    _splitpath (File,drive,dir,leaf,0); 
	    	sprintf (File,"%s%sglobal.ini",drive,dir); 
	    	if (_fstricmp (LastFile,File))
	    	{
			    LoadGlobalInit (File,FALSE); 
			    _fstrcpy (LastFile,File);
			}
		    if (LayerID == PickLayerID)
		    {
		    	PickFile = i+pViewports[iview]->ID*256;
{
#if ENABLETRACE
GSSiExitProg (677);
#endif
		    	return PickFile;
}
		    }
		}
	}
{
#if ENABLETRACE
GSSiExitProg (677);
#endif
	return -1;
}
#if ENABLETRACE
}
#endif
}

void StartFastPick (short PickLayerID)
#if ENABLETRACE
{GSSiEnterProg (678);
#endif
{   

	FastPick = TRUE;                            
	FastPickFII=-1;  
	if (PickLayerID < 0)
		FastPickFileNum = -PickLayerID;
	else
		FastPickFileNum = GetPickFile (PickLayerID); 
		
{
#if ENABLETRACE
GSSiExitProg (678);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

void EndFastPick (void)
#if ENABLETRACE
{GSSiEnterProg (679);
#endif
{
	if (!InCloseMap)
		CloseMap(FALSE);                 
	FastPickFileNum = -1;
	FastPick = FALSE;
{
#if ENABLETRACE
GSSiExitProg (679);
#endif
	return;
}
#if ENABLETRACE
}
#endif
} 
 
BOOL CreateZoomList (LPSTR Name, HWND hWnd)
#if ENABLETRACE
{GSSiEnterProg (657);
#endif
{   
	HFILE		Fid;
	OFSTRUCTGM	OFStruct;
	char		str[260], TypeFile[128], Ext[8];
	
	ExpandText (Name);
	_splitpath (Name,0,0,0,Ext);
	if (!*Ext)
		_fstrcat (Name,".txt");
    SubstituteDL (Name,FALSE);
	Fid = GSSiOpenFile (Name,&OFStruct,OF_CREATE);
	if (Fid == HFILE_ERROR)
	{
		GSSiMsgBox( hWnd,"Unable to create zoom list file",0, MB_OK|MB_ICONEXCLAMATION,0); 
{
#if ENABLETRACE
GSSiExitProg (657);
#endif
		return FALSE; 
}
	}
	*str = 0;
	if (!GetTextString (hWnd,str,sizeof(str),"Enter the zoom list title",0,0,0,TRUE,TRUE))  
	{
		GSSiClose (Fid);
		Fid = GSSiRemove (Name);
{
#if ENABLETRACE
GSSiExitProg (657);
#endif
		return FALSE;
}
	}
	fputstring (str,Fid);
	GSSiClose (Fid);
	GetCurVal (TypeFile,sizeof(TypeFile),IDS_FILEZMT); 
	Fid = GSSiOpenFile (TypeFile,&OFStruct,OF_READWRITE);  
	if (Fid == HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (657);
#endif
		return TRUE;
}
	GSSillseek (Fid,0,2);
	fputstring ("",Fid);   
	_fstrcat (str,"|");
	_fstrcat (str,Name);
	fputstring (str,Fid);	
	GSSiClose (Fid);	
{
#if ENABLETRACE
GSSiExitProg (657);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

void ZoomToPickedItem (int Item, double Offset,BOOL FromLimits,BOOL Immediate,BOOL AddToView)
#if ENABLETRACE
{GSSiEnterProg (659);
#endif
{   MNMXCORD Rect;
	long	npnts;
	HANDLE	hPnts;
	DPOINT	ScreenPoint;
	HPDPOINT	BasePt;
	int		ScreenWidth, ScreenHeight, i;
	double	AreaWidth, AreaHeight;
	BOOL	UseWidth=TRUE;
	double	Scale;
	DPOINT	MidPoint, Point[2];
    
	if (Item > MAXPICKITEMS)
		return;
	if (!FromLimits && (PickList[Item].Type == 2 || PickList[Item].Type == 3))
	{
		if (!GetPolyPoints ((LPPICKDATAHEADER)&PickList[Item],FALSE,&nPnts,&hPnts))
			goto UseRect;
		DBoundsInit (&Rect);
		BasePt = GlobalLock (hPnts);
		for (i=0;i<nPnts;i++)
		{
			ScreenPoint = BasePtToScreenPtD (&BasePt[i]);
			AddDPointToMinMax (&ScreenPoint,&Rect);
		}
		GSSiGlobUlFree (&hPnts);
		ScreenWidth = CurView->ScreenRect.right - CurView->ScreenRect.left;
		ScreenHeight = CurView->ScreenRect.bottom - CurView->ScreenRect.top;
		AreaWidth = Rect.xmx - Rect.xmn;
		AreaHeight = Rect.ymx - Rect.ymn;
		if (AreaWidth / ScreenWidth < AreaHeight / ScreenHeight)
			UseWidth = FALSE;
		if (UseWidth)
		{
			Point[0].x = Rect.xmn;
			Point[0].y = Rect.ymn;
			Point[1].x = Rect.xmx;
			Point[1].y = Rect.ymn;
			Point[0] = ScreenPtDToBasePt (Point[0]);
			Point[1] = ScreenPtDToBasePt (Point[1]);
			Scale = (Offset * 2 + ldistp (Point[0],Point[1])) / ScreenWidth;
		}
		else
		{
			Point[0].x = Rect.xmn;
			Point[0].y = Rect.ymn;
			Point[1].x = Rect.xmn;
			Point[1].y = Rect.ymx;
			Point[0] = ScreenPtDToBasePt (Point[0]);
			Point[1] = ScreenPtDToBasePt (Point[1]);
			Scale = (Offset*2 + ldistp (Point[0],Point[1])) / ScreenHeight;
		}
   		CurView->CurZoomAreaRef = PickList[Item].Refno;
		MidPoint = MinMaxMidPointD (&PickList[Item].Rect);
		ZoomToPointAndScale (MidPoint,Scale,Immediate);
	}
	else
	{
UseRect:
		Rect = PickList[Item].Rect; 
		SetGlobalValue ("%ZOOM_AREA_ID",PickList[Item].UDI); 
		if (!FromLimits)
		{
			Rect.xmn = (Rect.xmn + Rect.xmx)/2;
			Rect.xmx = Rect.xmn;
			Rect.ymn = (Rect.ymn + Rect.ymx)/2;
			Rect.ymx = Rect.ymn;
		}
		Rect.xmn -= Offset;
		Rect.xmx += Offset;
		Rect.ymn -= Offset;
		Rect.ymx += Offset;   
   		if (AddToView)
   		{    
			AddMinMaxD (&Rect,&CurView->NewBounds); 
			CurView->CurZoomAreaRef = 0;
		}
		else
    		CurView->CurZoomAreaRef = PickList[Item].Refno;
		ZoomToRect(Rect,Immediate);
	}
    
{
#if ENABLETRACE
GSSiExitProg (659);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}  

void SetDestination (LPMNMXCORD pDest)
#if ENABLETRACE
{GSSiEnterProg (660);
#endif
{
	if (pDest)
	{
		HaveDestination = TRUE;
		Destination = *pDest;  
		SetGlobalValueBool ("%USEDESTINATION",TRUE);
	}
	else
		HaveDestination = FALSE;
{
#if ENABLETRACE
GSSiExitProg (660);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}
/*
	MNMXCORD Bounds;
	DPOINT	 Points[4], Point, MidPoint;
	UINT	 i;
	DBoundsInit (&Bounds);
	RectToDPoints (&CurView->ScreenRect,Points);
	MidPoint = RectMidD (&CurView->ScreenRect);
	for (i=0;i<4;i++)
	{
		Point = dnewpt (MidPointW,getazd (&MidPoint,&Points[i])-CurView->Rotation,ldistp (MidPoint,Points[i])*Scale);
		AddDPointToMinMax (&Point,&Bounds);
	}
	ZoomToRect (Bounds,Imediate);
*/

BOOL SetScaleAndMidpointFromBounds (LPVIEWPORT CurView)
{
	MNMXCORD	Bounds;
	DPOINT	RectPoints[4], BoundsPoints[4];
	
	if (CurView->ID == *pCommandViewport)
		LastBounds = CurView->NewBounds;
	Bounds = RotateBounds (&CurView->NewBounds,CurView->Rotation);
	CurView->MidPointW = MinMaxMidPointD (&CurView->NewBounds);
	
	if (IsRectEmpty (&CurView->ScreenRect))
		CurView->ScreenRect = CurView->DrawRect;
	RectToDPoints (&CurView->ScreenRect,RectPoints);
	BoundsToPoints (&Bounds,BoundsPoints,0);
	CurView->Scale = max (ldistp (BoundsPoints[0],BoundsPoints[1])/ldistp (RectPoints[0],RectPoints[1]),
				 ldistp (BoundsPoints[1],BoundsPoints[2])/ldistp (RectPoints[1],RectPoints[2]));
	CurView->LastWidth = CurView->ScreenRect.right - CurView->ScreenRect.left;
	CurView->LastHeight = CurView->ScreenRect.bottom - CurView->ScreenRect.top;
	return TRUE;
}
 
void ZoomToRect(MNMXCORD Rect,BOOL Imediate)
{
	MNMXCORD Bounds;
	DPOINT GPSPoint, MidPointW; 
	char	AVLTrackVehicle[34];
	double Inc, Scale; 

	if (!ValidBounds (&Rect))
		return;  
	if (CurView->ZoomLocked)
		return;
	if (NextZoomIsDestination) 
	{
		HaveDestination = TRUE;
		Destination = Rect;  
		SetGlobalValueBool ("%USEDESTINATION",TRUE);
	}
	DisplayCycle++;
	NextZoomIsDestination = FALSE;
	GetGlobalCVal ("[%AVLFOLLOW]",AVLTrackVehicle,0);
	if (GetVehicleLoc(AVLTrackVehicle,&GPSPoint,CurView->ID)) 
	{   
		if (HaveDestination && GetGlobalBVal ("[%USEDESTINATION]"))
    		AddMinMaxD (&Rect,&Destination);  
		if (!PointInBounds (GPSPoint,&Rect))
		{
    		AddDPointToMinMax (&GPSPoint,&Rect);  
    		Inc = (CurView->WBounds.xmx - CurView->WBounds.xmn) * 0.15;
    		InflateBounds (&Rect,Inc);
    	} 
    }
	CurView->NewBounds = Rect;
	SetScaleAndMidpointFromBounds (CurView);

	ZoomToPointAndScale (CurView->MidPointW,CurView->Scale,Imediate);
	return;
}

BOOL ZoomToPointAndScaleOnlyIfDifferent (DPOINT MidPointW,double Scale,BOOL Imediate)
{
	if (SameDPoint (&MidPointW,&CurView->MidPointW) && Scale == CurView->Scale)
		return FALSE;
	ZoomToPointAndScale (MidPointW,Scale,Imediate);
	return TRUE;
}

void ZoomToPointAndScale (DPOINT MidPointW,double Scale,BOOL Imediate)
#if ENABLETRACE
{GSSiEnterProg (661);
#endif
{   
	LPVIEWPORT	SaveVP=CurView;  
	HDC	hDC; 
	short	i; 
	static	LastDisplayCycle=0;
	
	if (CurView->ZoomLocked)
		return;	
    if (Scale < GetGlobalDVal2 ("[%MINSCALE]",0))
    	Scale = GetGlobalDVal2 ("[%MINSCALE]",0);
	if (!ScaleIsSet(FALSE))
		CurView->Scale = Scale;
	CurView->MidPointW = MidPointW;
	CurView->HaveBounds = TRUE;
	CurView->WindowIsZoomed = TRUE;

	if (Imediate < 0)
	{
		CurView->LastWidth = 0;
		DisplayCycle++;
		SelectVisList(FALSE);
		SetBounds(CurView->hWnd, CurView->hDC);
		DisplayCycle--;
		return;
	}
	ClearFullWindowBitmap(0);
	CloseThemeFiles();

	if (CurView->DisplayCycle < DisplayCycle)
	{   
		HDC	hDCMain = GetDC (CurView->hWnd);
		
		DestroySavedGraphicsFile (0);
		SetSavedGraphicsDC (CurView->hDC);
// moved to GetNextViewportFile		ProcessText (CurView->BeginDisplayCmd);
		LastDisplayCycle = DisplayCycle;
		if (MemMap)
    		hDC = hdcMemMap; 
   		else if (Printing)
    		hDC = CurView->hDC;
    	else
    		hDC = hDCMain;	
		DestroyVehicles (CurView->ID);
		ClearMeterPrompts (hDC);
		hDC = ScreenBufferDC (CurView->hWnd,hDC);
		for (i=0;i<*pNumViewports;i++)  
			pViewports[i]->hDC = hDC;    
		ReleaseDC (CurView->hWnd,hDCMain); 
	}
	NumScreensDisplayed++;
/*	if (ScaleIsSet(FALSE))
	{
		double	BWidth = CurView->WBounds.xmx - CurView->WBounds.xmn;
		double	BHeight = CurView->WBounds.ymx - CurView->WBounds.ymn;
		DPOINT	CenterPoint = MinMaxMidPointD (&Rect);
		
		Rect.xmn = CenterPoint.x - BWidth/2;
		Rect.ymn = CenterPoint.y - BHeight/2;
		Rect.xmx = CenterPoint.x + BWidth/2;
		Rect.ymx = CenterPoint.y + BHeight/2;
    }*/
	if (GetGlobalBVal2 ("[%ZOOMDISABLED]",FALSE))
		goto Exit;
    if (CurView->DisplayInParent && CurView->Parent)
     	SetCurView (pViewports[CurView->Parent-1]);
	if (CurView->OrthoRes >=0 || !CurView->WindowZoomedToOrtho)
	    CurView->WindowZoomedToOrtho = FALSE;   
	DisplayCycle++; 
	SelectVisList (FALSE);
    SetBounds (CurView->hWnd,CurView->hDC); 
    DisplayCycle--; 
	CurView->WindowIsZoomed = TRUE;   
	if (!Display)
		goto Exit;
	if (Imediate)
		RedisplayViewport(Imediate,FALSE);
	else
	{
		InDisplayProcessing = 2;
		PostMessage(CurView->hWnd, WM_COMMAND, IDM_Z_REDRAW, CurView->ID);
	}
	ZoomConnectedProcesses (FALSE);
Exit:
    SetCurView (SaveVP);
{
#if ENABLETRACE
GSSiExitProg (661);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

void ZoomToPointAndDist (DPOINT Point, double dist,BOOL Immediate)
#if ENABLETRACE
{GSSiEnterProg (662);
#endif
{
	MNMXCORD	Rect; 
    
    if (dist)
    {
		Rect.xmn = Point.x - dist;
		Rect.xmx = Point.x + dist;
		Rect.ymn = Point.y - dist;
		Rect.ymx = Point.y + dist;  
	}
	else
	{
	    dist = (CurView->WBounds.xmx - CurView->WBounds.xmn)/2;
	    Rect.xmn = Point.x - dist;
	    Rect.xmx = Point.x + dist;
	    dist = (CurView->WBounds.ymx - CurView->WBounds.ymn)/2;
	    Rect.ymn = Point.y - dist;
	    Rect.ymx = Point.y + dist; 
    }
    CurView->CurZoomAreaRef = 0;
	ZoomToRect(Rect,Immediate);
{
#if ENABLETRACE
GSSiExitProg (662);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

void ComputePickPoint (LPPOINT BPOINT, LPPOINT EPOINT,double Dist,
					   double TotDist,LPDOUBLE PCT, LPDOUBLE OffDist,LPDOUBLE AZ,
				  	   LPMNMXCORL BOUNDS,LPDPOINT PickedPoint,LPPOINT PickAtPoint)
#if ENABLETRACE
{GSSiEnterProg (663);
#endif
{	double	PickY, PickX, AZ1, AZ2, INTX, INTY, dinc, x, LineLen,dist1,dist2;
	long	l;
    
    if (!PickAtPoint)
    {
		PickX = (double)BOUNDS->xmn + ((double)BOUNDS->xmx-(double)BOUNDS->xmn)/2;
		PickY = (double)BOUNDS->ymn + ((double)BOUNDS->ymx-(double)BOUNDS->ymn)/2;  
	}
	else  
	{
		PickX = PickAtPoint->x;
		PickY = PickAtPoint->y;
	}
	AZ1 = getaz (*BPOINT,*EPOINT); 
	*AZ = AZ1;
	AZ2 = LTWOPI (AZ1 + HALFPI);
    LineLen = idist (*BPOINT,*EPOINT);
	LINSEC ((double)BPOINT->x,(double)BPOINT->y,AZ1, PickX,PickY,AZ2,&INTX,&INTY);
	if (LDIST (INTX,INTY,(double)BPOINT->x,(double)BPOINT->y) > LineLen ||
		LDIST (INTX,INTY,(double)EPOINT->x,(double)EPOINT->y)> LineLen)
	{ 
		dist1 = LDIST (PickX,PickY,(double)BPOINT->x,(double)BPOINT->y);
		dist2 = LDIST (PickX,PickY,(double)EPOINT->x,(double)EPOINT->y);
		if (dist1 < dist2)
		{  
			INTX = BPOINT->x;
			INTY = BPOINT->y;
		}
		else 
		{
			INTX = EPOINT->x;
			INTY = EPOINT->y;
		}
	}
	PickedPoint->x = INTX;
	PickedPoint->y = INTY; 
	*OffDist = LDIST (INTX,INTY,PickX,PickY)*FileDistToBaseDist;
	AZ2 = LGETAZ(INTX,INTY,PickX,PickY);
	if (DeflectionAngle (AZ1,AZ2) >0)
		*OffDist = -*OffDist; 
	if (TotDist == 0)
		*PCT = 0;
	else
		*PCT = (Dist + LDIST ((double)BPOINT->x,(double)BPOINT->y,INTX,INTY))/TotDist;
{
#if ENABLETRACE
GSSiExitProg (663);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

void ComputePickPointD (HPDPOINT BPOINT, HPDPOINT EPOINT,double Dist,
					   double TotDist,LPDOUBLE PCT, LPDOUBLE OffDist,LPDOUBLE AZ,
				  	   LPMNMXCORD BOUNDS,LPDPOINT PickedPoint)
#if ENABLETRACE
{GSSiEnterProg (664);
#endif
{	double	PickY, PickX, AZ1, AZ2, INTX, INTY, dinc, x, LineLen,dist1,dist2;
	long	l;

	PickX = PickPointBase.x;//(double)BOUNDS->xmn + ((double)BOUNDS->xmx-(double)BOUNDS->xmn)/2;
	PickY = PickPointBase.y;//(double)BOUNDS->ymn + ((double)BOUNDS->ymx-(double)BOUNDS->ymn)/2;
    if (!(LineLen = ldistp (*BPOINT,*EPOINT)))
    {
    	*AZ = 0;
		*PickedPoint = *BPOINT;
		*OffDist = LDIST (BPOINT->x,BPOINT->y,PickX,PickY);
		if (TotDist == 0)
			*PCT = 0;
		else
			*PCT = Dist/TotDist;
{
#if ENABLETRACE
GSSiExitProg (664);
#endif
		return;
}
	}
    	
	AZ1 = getazd (BPOINT,EPOINT); 
	*AZ = AZ1;
	AZ2 = LTWOPI (AZ1 + HALFPI);
	LINSEC ((double)BPOINT->x,(double)BPOINT->y,AZ1, PickX,PickY,AZ2,&INTX,&INTY);
	if (LDIST (INTX,INTY,(double)BPOINT->x,(double)BPOINT->y) > LineLen ||
		LDIST (INTX,INTY,(double)EPOINT->x,(double)EPOINT->y)> LineLen)
	{ 
		dist1 = LDIST (PickX,PickY,(double)BPOINT->x,(double)BPOINT->y);
		dist2 = LDIST (PickX,PickY,(double)EPOINT->x,(double)EPOINT->y);
		if (dist1 < dist2)
		{  
			INTX = BPOINT->x;
			INTY = BPOINT->y;
		}
		else 
		{
			INTX = EPOINT->x;
			INTY = EPOINT->y;
		}
	}
	PickedPoint->x = INTX;
	PickedPoint->y = INTY; 
	*OffDist = LDIST (INTX,INTY,PickX,PickY);
	AZ2 = LGETAZ(INTX,INTY,PickX,PickY);
	if (DeflectionAngle (AZ1,AZ2) >0) *OffDist = -*OffDist; 
	if (TotDist == 0)
		*PCT = 0;
	else
		*PCT = (Dist + LDIST ((double)BPOINT->x,(double)BPOINT->y,INTX,INTY))/TotDist;
{
#if ENABLETRACE
GSSiExitProg (664);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

double DeflectionAngle (double CurAZ, double AZ)
#if ENABLETRACE
{GSSiEnterProg (665);
#endif
{   double rtn;
	CurAZ = LTWOPI (CurAZ); 
	AZ	  = LTWOPI (AZ);
	if (CurAZ > AZ)
	{
		if (CurAZ - AZ > PY)
			rtn = TWOPI - (CurAZ-AZ);
		else
			rtn = AZ-CurAZ;
	}
	else
	{ 
		if (AZ - CurAZ > PY)
			rtn = (AZ-CurAZ)-TWOPI;
		else
			rtn = AZ-CurAZ;
	}
{
#if ENABLETRACE
GSSiExitProg (665);
#endif
	return (rtn);
}
#if ENABLETRACE
}
#endif
}

LPSHORT FindNextGraphicsLink (HPSHORT ipnt)
#if ENABLETRACE
{GSSiEnterProg (669);
#endif
{	short	idesc, ItemLen, TSize;
	long	*pRefno;
	long	remlen;
	LPBYTE	Pcode;
	short	x1,y1,x2,y2, ipen, i;
	double	TotDist, WantDist, Dist;
	LPPOINT	lpPointSave;
	POINT	CLPointFile;
	LPLONG	pOffset;

        while (*ipnt != 0)
        {   Pcode = (BYTE *) ipnt;
        	ipnt++;

		    switch (*Pcode)
	        {   
				case 93: //removed item
				case 92: /* deleted item */
				case 12: /* item minmax */
				{	ipnt += 4;
					ItemLen = abs(*ipnt);
					ipnt++;
					ipnt+=ItemLen;
                }
                break;

				case 13: /* continuation offset */
				{	
{
#if ENABLETRACE
GSSiExitProg (669);
#endif
					return(ipnt);
}
                }
                break;

	            default: 
	            	SkipSubRec (Pcode,&ipnt,0);
                break;

			}
        }
{
#if ENABLETRACE
GSSiExitProg (669);
#endif
	return(0);
}
#if ENABLETRACE
}
#endif
}


    
BOOL DisplayZoomList (short Opt)
#if ENABLETRACE
{GSSiEnterProg (672);
#endif
{                              
//Opt 0 = display, 1=next, -1=prior
	DLGPROC lpfnZOOMLISTMsgProc;
	short	PickFile, PickLayerID; 
	BOOL	AddToView=FALSE; 
	int		nRc;
	DWORD lParam;
	BOOL	SaveGetMaskArea = GetMaskArea;
	
	AutoZoomNext = Opt;				
	lpfnZOOMLISTMsgProc = MakeProcInstance((DLGPROC)ZOOMLISTMsgProc, hInst);
	nRc = DialogBox(hInst, (LPSTR)"ZOOMLIST", hWndMain, lpfnZOOMLISTMsgProc);
	FreeProcInstance(lpfnZOOMLISTMsgProc);   
	if (nRc == 2)
	{
		AddToView = TRUE;
		nRc = 1;
	}
	if (nRc==1)
	{   
		ClearCurrentCD ();
		SetViewport(*pCommandViewport);
	    if (ZoomRef == -1)
		{   
		    CurView->CurZoomAreaRef = 0;
			ZoomToRect (CurView->NewBounds,FALSE);
	/*						if (CurView->OrthoRes >=0 || !CurView->WindowZoomedToOrtho)
		    	CurView->WindowZoomedToOrtho = FALSE;
		    DisplayCycle++;
		    SetBounds (CurView->hWnd,CurView->hDC);
		    DisplayCycle--;
		    RedisplayViewport(FALSE,FALSE);
			CurView->WindowIsZoomed = TRUE; */
			GetMaskArea = SaveGetMaskArea;
{
#if ENABLETRACE
GSSiExitProg (672);
#endif
			return TRUE;
}
		} 
		if (!(PickLayerID = GetGlobalLVal ("[%PICKLAYERID]")))
			PickFile = -1; 
		else
			PickFile = GetPickFile (PickLayerID);
		if (ZoomRef == LONG_MIN)
		{
			if (PickByRefno(ZoomRef,ZoomPrefix,ZoomUDI,PickFile))
			{
				SetMaskArea(NumPicked-1,0,1);
				SelectAreaToOffsetFile(NumPicked - 1, 0, 0);
	    		ZoomToPickedItem(0,100,TRUE,FALSE,AddToView);
	    	}
	    	else
		      	GSSiMsgBox( GetFocus(),"Unable to find zoom area",
	    			 	    "Error", MB_OK|MB_ICONEXCLAMATION,0);
		}
		else
		{
			if (PickByRefno(ZoomRef,0,0,PickFile))
			{
				SetMaskArea(NumPicked-1,0,1);
				SelectAreaToOffsetFile(NumPicked - 1, 0, 0);
	    		ZoomToPickedItem(0,100,TRUE,FALSE,AddToView);
	    	}
	    	else
		      	GSSiMsgBox( GetFocus(),"Unable to find zoom area",
	    			 	    "Error", MB_OK|MB_ICONEXCLAMATION,0);
		}
	}
	else if (nRc) 
	{    
		lParam = (long)nRc + LONG_MAX;
	    PostMessage(hWndMain, WM_COMMAND, IDM_PROCESSTEXT, lParam);
	}  
	GetMaskArea = SaveGetMaskArea;
{
#if ENABLETRACE
GSSiExitProg (672);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
} 



BOOL Report (LPSTR NameIN, LPSTR ViewportName, LPSTR Prefix, LPSTR UDI, long ref,BOOL LoadOnly,BOOL FitToVP)
#if ENABLETRACE
{GSSiEnterProg (865);
#endif
{   short iview;
	BOOL	Err;
    BOOL    rtn=TRUE;
    LPVIEWPORT  UseView=0, SaveView; 
    LPSTR	pColon;   
    LPVIEWPORT	pVP;
	char	Name[MAX_PATH];
	char	File[MAX_PATH];

	strcpy (Name,NameIN);
     
    SaveView = CurView; 
	strcpy(File, ViewportName);
    if ((pColon = _fstrchr (ViewportName,':')))
    {
    	*pColon++ = 0; 
		pVP = SetVPFromName (ViewportName,&Err); 
		if (!Err) 
		{
			ScrollRect = pVP->DrawRect;
			ClientRectToScreenRect (CurView->hWnd,&ScrollRect);
		}  
    }
    else
    	pColon = ViewportName;
    if (!_fstricmp (pColon,"SCROLL"))
    {
        DisplayScrollReport (Name,Prefix,UDI, ref);
        SetCurView ( SaveView);
{
#if ENABLETRACE
GSSiExitProg (865);
#endif
        return rtn;
}
    }
    else if (!_fstricmp (pColon,"SCROLL2"))
    {
        DisplayScrollReport2 (Name,Prefix,UDI, ref);
        SetCurView ( SaveView);
{
#if ENABLETRACE
GSSiExitProg (865);
#endif
        return rtn;
}
    }
    else if (_fstrstr (pColon,".TXT"))
    {
        ReportToFile (Name,Prefix,UDI, ref,File);
        SetCurView ( SaveView);
{
#if ENABLETRACE
GSSiExitProg (865);
#endif
        return rtn;
}
    }
    else if (!_fstricmp (pColon,"NONE"))
    {
        ProcessMacroReport (Name,Prefix,UDI, ref);
        SetCurView ( SaveView);
{
#if ENABLETRACE
GSSiExitProg (865);
#endif
        return rtn;
}
    }
    for (iview = 0; iview<*pNumViewports; iview++)
    {   
        SetCurView ( pViewports[iview]); 
        if (!_fstricmp (pColon,CurView->Name))  
        {
            UseView = CurView;
            break;
        }
        if (CurView->Type == 6)
        {   
            if (!UseView)
                UseView = CurView;
        }
    }
    if (!UseView)
        goto RtnFalse;
    
    SetCurView (UseView); 
    CurView->FitToWindow = FitToVP;
    if (Prefix)
    	_fstrcpy (CurView->Prefix,Prefix);  
    else
    	*CurView->Prefix = 0;
    if (UDI)                  
    	_fstrcpy (CurView->UDI,UDI); 
    else
    	*CurView->UDI = 0;
    CurView->ReportRefno=ref;
    UnloadReport (&CurView->hReport);  
    CurView->hReport = LoadReport (Name);
    if (!CurView->hReport)
        goto RtnFalse;  
    if (LoadOnly)
    	goto RtnTrue;
    CurView->Active = TRUE;
    CurView->ShrinkToFit=FALSE; 
    CurView->CurZoomAreaRef = 0;
    RedisplayViewport(TRUE,FALSE);
RtnTrue:
    SetCurView (SaveView);
    
{
#if ENABLETRACE
GSSiExitProg (865);
#endif
    return TRUE; 
}
    
RtnFalse:
    SetCurView ( SaveView);
{
#if ENABLETRACE
GSSiExitProg (865);
#endif
    return FALSE;
}
#if ENABLETRACE
}
#endif
}  

void DestroySavedZooms (void)
#if ENABLETRACE
{GSSiEnterProg (866);
#endif
{ 
    LPSAVEDZOOM pSavedZoom;
    LPSHORT   pNumSavedViews;

	if (hSavedZooms)
	{
	    pNumSavedViews = (LPSHORT)GlobalLock (hSavedZooms);
	    pNumSavedViews++;
	    pSavedZoom = (LPSAVEDZOOM)pNumSavedViews;
	    GSSiGlobFree (&pSavedZoom->hMask);
	    GSSiGlobFree (&pSavedZoom->hMaskAccelerator[0]);
	    GSSiGlobFree (&pSavedZoom->hMaskAccelerator[1]);
	    GSSiGlobFree (&pSavedZoom->hMaskAccelerator[2]);
	    GSSiGlobUlFree (&hSavedZooms);
	}
{
#if ENABLETRACE
GSSiExitProg (866);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}
 
BOOL SaveZooms (LPMNMXCORD pBounds)
#if ENABLETRACE
{GSSiEnterProg (867);
#endif
{   
    LPSAVEDZOOM pSavedZoom;
    short iview; 
    LPSHORT   pNumSavedViews; 
    BOOL		rtn=FALSE;
    
    DestroySavedZooms ();
    if (!*pNumViewports)
{
#if ENABLETRACE
GSSiExitProg (867);
#endif
    	return FALSE;
}
    hSavedZooms = GSSiGlobAlloc ( 178,GHND,*pNumViewports*sizeof(SAVEDZOOM)+sizeof(short));
    pNumSavedViews = (LPSHORT)GlobalLock (hSavedZooms);
    *pNumSavedViews = 1;
    pNumSavedViews++;
    pSavedZoom = (LPSAVEDZOOM)pNumSavedViews; 
    if (pBounds)
    {
	    pSavedZoom->HaveBounds = TRUE;
	    pSavedZoom->Bounds = *pBounds; 
	    iview = 0; 
	    rtn = TRUE;
	} 
    else
    {
	    iview = *pCommandViewport-1;
	    rtn = pSavedZoom->HaveBounds = pViewports[iview]->HaveBounds;
	    pSavedZoom->Bounds = pViewports[iview]->WBounds;
	    pSavedZoom->hMask = pViewports[iview]->hMaskArea;
	    _fmemmove (&pSavedZoom->hMaskAccelerator,pViewports[iview]->hMaskAccelerator,sizeof(pSavedZoom->hMaskAccelerator)); 
	    pSavedZoom->NumMaskPoints = pViewports[iview]->NumMaskPoints; 
	    pSavedZoom->NumMaskAreaParts = pViewports[iview]->NumMaskAreaParts; 
	    pViewports[iview]->hMaskArea = 0; 
	    _fmemset (pViewports[iview]->hMaskAccelerator,0,sizeof(pViewports[iview]->hMaskAccelerator));
	    pViewports[iview]->NumMaskPoints = 0;
	    pViewports[iview]->NumMaskAreaParts = 0;
	}
    _fstrcpy (pSavedZoom->Name,pViewports[iview]->Name);
    GlobalUnlock (hSavedZooms);
    
{
#if ENABLETRACE
GSSiExitProg (867);
#endif
    return rtn;
}
#if ENABLETRACE
}
#endif
}

void TransferSavedZoom (void)
#if ENABLETRACE
{GSSiEnterProg (868);
#endif
{   
    LPSAVEDZOOM pSavedZoom;
    LPSHORT   pNumSavedViews;
    short iview, nsv, isv;

    if (!hSavedZooms)
{
#if ENABLETRACE
GSSiExitProg (868);
#endif
    	return; 
}
    
    pNumSavedViews = (LPSHORT)GlobalLock (hSavedZooms);
    nsv = *pNumSavedViews;
    pNumSavedViews++;
    iview = *pCommandViewport-1;
    pSavedZoom = (LPSAVEDZOOM)pNumSavedViews;
    if (ForceBounds || (pSavedZoom->HaveBounds && !pViewports[iview]->HaveBounds))
    {
        pViewports[iview]->WBounds=pSavedZoom->Bounds;  
        pViewports[iview]->NewBounds=pSavedZoom->Bounds;  
        pViewports[iview]->HaveBounds=TRUE;  
        pViewports[iview]->WindowIsZoomed=TRUE;  
        GSSiGlobFree (&pViewports[iview]->hMaskArea); 
        GSSiGlobFree (&pViewports[iview]->hMaskAccelerator[0]); 
        GSSiGlobFree (&pViewports[iview]->hMaskAccelerator[1]); 
        GSSiGlobFree (&pViewports[iview]->hMaskAccelerator[2]); 
        pViewports[iview]->hMaskArea = pSavedZoom->hMask;  
        pSavedZoom->hMask = 0;  
        _fmemset (pSavedZoom->hMaskAccelerator,0,sizeof(pSavedZoom->hMaskAccelerator));
        pViewports[iview]->NumMaskPoints = pSavedZoom->NumMaskPoints;
        pViewports[iview]->NumMaskAreaParts = pSavedZoom->NumMaskAreaParts;
		SetScaleAndMidpointFromBounds (pViewports[iview]);
    }
    GSSiGlobFree (&pSavedZoom->hMask);
    GSSiGlobUlFree (&hSavedZooms);
{
#if ENABLETRACE
GSSiExitProg (868);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}  


BOOL GetVisBounds2 (LPMNMXCORD	pBounds,HDC hDC)
#if ENABLETRACE
{GSSiEnterProg (869);
#endif
{ 
	short	FileNum;
	HANDLE	handle;  
	BOOL	rtn=FALSE;   
	HANDLE	hMem=GSSiGlobAlloc (0,GMEM_MOVEABLE,1024);
	LPSTR	str=GlobalLock (hMem);
	
	SelectVisList (FALSE); 
	if (!CurVis)
		goto Exit;		
	CurView->HaveBounds = TRUE;  
	CurView->PassID = 0;
    for (FileNum=0;FileNum<CurView->NumFiles;FileNum++)
    {    
    	 CurView->CurFile = FileNum;
	     if (!CurVis->FileIsVisible[CurView->CurFile])
	     	goto NextFile; 
	     if (!AutoTypeConvert ())
	     	goto NextFile;
    	 PltType = CurView->FileType[CurView->CurFile];
		 if (PltType<4)
		 {
		     _fstrcpy (PltName,CurView->lpFiles[CurView->CurFile]); 
		     if (!OpenMap ((HWND)1,0))
		     	goto NextFile;
		     CloseMap (FALSE);
		 }
         else if (PltType <= 5) 
         {
			_fstrcpy (str,CurView->lpFiles[CurView->CurFile]);
			ExpandText(str);
		    	
			if (_fstrstr(str,"FILELIST.TXT"))
         	{
				GetBinFileBounds (CurView->lpFiles[CurView->CurFile],pBounds);
         		goto NextFile; 
         	}
         	else
         	{
				handle = OpenMapIndex (CurView->lpFiles[CurView->CurFile],0);
            	CloseMapIndex (CurView->lpFiles[CurView->CurFile],handle,FALSE,TRUE);
            } 
         }
         else if (PltType == 9)  
         	GetMapBounds (CurView->lpFiles[CurView->CurFile],&CurView->FileMNMX);
         else
         	goto NextFile;
		pBounds->xmn = min (pBounds->xmn,CurView->FileMNMX.xmn);
		pBounds->xmx = max (pBounds->xmx,CurView->FileMNMX.xmx);
		pBounds->ymn = min (pBounds->ymn,CurView->FileMNMX.ymn);
		pBounds->ymx = max (pBounds->ymx,CurView->FileMNMX.ymx); 
		rtn = TRUE;
NextFile:;
	} 
Exit: 
	GSSiGlobUlFree (&hMem);
{
#if ENABLETRACE
GSSiExitProg (869);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
} 

BOOL GetVisBounds (LPMNMXCORD	pBounds,HDC hDC)
{   
	short	ParVP = CurView->ID, iview;
	BOOL	rtn=FALSE, SaveDisplay=Display;
	LPVIEWPORT	SaveVP=CurView;
	
	CloseMap (FALSE);
	DBoundsInit (pBounds); 
	
	Display = FALSE;
	if (GetVisBounds2 (pBounds,hDC))
		rtn=TRUE;
	for (iview=0;iview<*pNumViewports;iview++)
	{
		if (pViewportsD[iview]->DisplayInParent &&  pViewportsD[iview]->Parent == ParVP)
		{
			CurView = pViewportsD[iview];
			if (GetVisBounds2 (pBounds,hDC))
				rtn=TRUE;
		}
	}
	CurView = SaveVP;
	Display = SaveDisplay;
	return rtn;
}

BOOL GetLayerBounds (LPMNMXCORD	pBounds,HDC hDC, int LayerID)
#if ENABLETRACE
{GSSiEnterProg (870);
#endif
{
	HANDLE	handle;
	BOOL	SaveHaveBounds=CurView->HaveBounds;
	BOOL	rtn=FALSE; 
	BOOL	SaveDisplay=Display;

	Display = FALSE;
	
	if (LayerID < 0 || LayerID+1 > CurView->NumFiles)
		goto Exit;
	DBoundsInit (pBounds);
	
	FileNum = LayerID;
	CurView->HaveBounds = TRUE;
	PltType = CurView->FileType[FileNum];
	if (PltType<4)
	{
		 _fstrcpy (PltName,CurView->lpFiles[FileNum]); 
		 OpenMap (CurView->hWnd,hDC);
		 CloseMap (FALSE);
	}
	else if (PltType <= 5) 
	{
		if (_fstrstr(CurView->lpFiles[FileNum],"FILELIST.TXT"))
		{
			rtn = GetBinFileBounds (CurView->lpFiles[FileNum],pBounds);
			goto Exit;
		}
		else
		{
			handle = OpenMapIndex (CurView->lpFiles[FileNum],0);
			CloseMapIndex (CurView->lpFiles[FileNum],handle,FALSE,TRUE);
		} 
	}
	else
		goto Exit;
	pBounds->xmn = min (pBounds->xmn,CurView->FileMNMX.xmn);
	pBounds->xmx = max (pBounds->xmx,CurView->FileMNMX.xmx);
	pBounds->ymn = min (pBounds->ymn,CurView->FileMNMX.ymn);
	pBounds->ymx = max (pBounds->ymx,CurView->FileMNMX.ymx); 
	rtn = TRUE;
Exit:
	Display = SaveDisplay;
	CurView->HaveBounds = SaveHaveBounds;
{
#if ENABLETRACE
GSSiExitProg (870);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}

double GetStreetSegLength (long Segid)
{
    long	Offset,Refno,nPnts;
    short	Desc; 
    double	Length=-1; 

    if (hStreetPolys)
    {   
    	if (BT_FIND (hStreetPolys,(LPSTR)&Segid,BT_FIRST,BT_EQ,(LPSTR)&Offset))
    		return FALSE;
    	GSSillseek (FidStreetPolys,Offset,0);
    	BigRead (FidStreetPolys,(HPSTR)&Refno,4);
    	BigRead (FidStreetPolys,(HPSTR)&Desc,2);
    	BigRead (FidStreetPolys,(HPSTR)&nPnts,4); 
    	BigRead (FidStreetPolys,(HPSTR)&Length,8); 
    }
	return Length;
}

BOOL GetSegCoorByPct (long Segid,double PCT,DPOINT *CLPoint,double *AZ,double *Length,BOOL HaveSeg)
#if ENABLETRACE
{GSSiEnterProg (1372);
#endif
{   
	HPSHORT		ipnt, EndItem;
    HANDLE 		hpltBuf=0;
	LPSTR		LPpltBuf;
	ITEM		*ItemHeader;
	HDC			hDC;
	POINT		CenterPoint, WinPoint;
	int			Item=0;  
	BOOL		Opened = FALSE, rtn=FALSE;
	static		BOOL	NeedOpen=TRUE;
	WORD		nBytes, nRead;
    
    if (hStreetPolys)
    {   
    	long	Offset,Refno,nPnts,Size,EndPointNum;
    	short	Desc; 
    	HPDPOINT	Points; 
    	double	AtDist; 
    	HANDLE	hPoly;
    	
    	if (BT_FIND (hStreetPolys,(LPSTR)&Segid,BT_FIRST,BT_EQ,(LPSTR)&Offset))
    		goto Exit;
    	GSSillseek (FidStreetPolys,Offset,0);
    	BigRead (FidStreetPolys,(HPSTR)&Refno,4);
    	BigRead (FidStreetPolys,(HPSTR)&Desc,2);
    	BigRead (FidStreetPolys,(HPSTR)&nPnts,4); 
    	BigRead (FidStreetPolys,(HPSTR)Length,8); 
    	Size = nPnts * sizeof(DPOINT);
    	hPoly = GSSiGlobAlloc (0,GMEM_MOVEABLE,Size);
    	Points = (HPDPOINT)GlobalLock (hPoly);
    	BigRead (FidStreetPolys,(HPSTR)Points,Size); 
    	AtDist = *Length * PCT;
		*CLPoint = PointAtDistOnPoly (Points,nPnts,AtDist,AZ,&EndPointNum);   
    	GSSiGlobUlFree (&hPoly);
		rtn = TRUE;
    	goto Exit;
    }
    if ((NeedOpen || FidMap == HFILE_ERROR) && !HaveSeg)
	{
		if (!PickByRefno (Segid,0,0,PickFileNum))
			goto Failed;

	    GetPickName (Item);

		if (!PickName) goto Failed;
		_fstrcpy (PltName,PickName);

		CloseMap (FALSE); 
		{
			BOOL	SaveWDB = WantDescBlock, omst; 

			WantDescBlock = FALSE;
			omst = OpenMap (CurView->hWnd,CurView->hDC);
	   		WantDescBlock = SaveWDB;  
			if (!omst)
				goto Failed; 
			Opened = TRUE;
		}
	} 
	if (HaveSeg)
    	GSSillseek (FidMap,CurrentSeg,0);
    else
	    GSSillseek (FidMap,PickList[Item].Segment,0);
    nRead = BigRead (FidMap,(HPSTR)&nBytes,2);
    hpltBuf = GSSiGlobAlloc ( 179,GMEM_MOVEABLE,(DWORD)nBytes);
    LPpltBuf = GlobalLock (hpltBuf);
    nRead = BigRead (FidMap,LPpltBuf,nBytes); 
    if (HaveSeg)
	    ipnt =(LPSHORT) (LPpltBuf + CurrentItem);
	else
    	ipnt = (LPSHORT) (LPpltBuf + PickList[Item].Offset);
    ItemHeader = (ITEM *)ipnt;
    EndItem = ipnt + abs(ItemHeader->Len);
    EndItem+=6;
    *EndItem = 0;
    if (!FindPolylinePoint (ipnt,PCT,CLPoint,AZ,Length))
    	goto Failed;
//    *Length = PickList[Item].Length;
/*	if (Driving)
		NeedOpen = FALSE;
	else
		NeedOpen = TRUE;*/
	rtn = TRUE;
Failed:
	GSSiGlobUlFree (&hpltBuf);
	if (Opened)
		CloseMap(FALSE);
Exit:
{
#if ENABLETRACE
GSSiExitProg (1372);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}

BOOL FindPolylinePoint (HPSHORT ipnt,double PCT, DPOINT *CLPoint,double *AZ, double *Length)
{	short	idesc, ItemLen, TSize;
	long	*pRefno;
	long	remlen;
	LPBYTE	Pcode;
	BOOL	Visible=TRUE, TextIsVisible=TRUE, RefIsVisible=TRUE, DescIsVisible=TRUE;
	short	x1,y1,x2,y2, ipen;
	DWORD	i;
	double	TotDist, WantDist, Dist;
	LPPOINTS	lpPointSave; 
	HPDPOINT	lpDPointSave,lpDPoints2;
	POINT	CLPointFile;
	LPPOINTS lpPoints, lpPoints2;

        while (*ipnt != 0)
        {   Pcode = (BYTE *) ipnt;
        	CurElementPnt = ipnt++; 

		    switch (*Pcode)
	        {
	            case 4: /* put line */
		 		{
		 			nPnts = 2;
					goto DoPolyline;
				}
	            break;

	            case 41: /* put line */
		 		{
		 			nPnts = 2;
					goto DoPolylineD;
				}
	            break;

	            case 6: /* put polyline */

		 		{   nPnts = *ipnt;
		 		    ipnt++;
		 DoPolyline:lpPoints2 = (LPPOINTS) ipnt;
		 		    ipnt = ipnt + nPnts * 2;
					TotDist = 0;
					for (i=1;i<nPnts;i++)
						TotDist += idist(POINTStoPOINT(lpPoints2[i-1]),POINTStoPOINT(lpPoints2[i]));
					*Length = TotDist;
					WantDist = TotDist * PCT;
					TotDist = 0;
					for (i=1;i<nPnts;i++)
					{
						Dist = idist(POINTStoPOINT(lpPoints2[i-1]),POINTStoPOINT(lpPoints2[i]));
						if (TotDist + Dist >= WantDist || i==nPnts-1)
						{
							*AZ = getaz(POINTStoPOINT(lpPoints2[i-1]),POINTStoPOINT(lpPoints2[i]));
							Dist = WantDist - TotDist;
							CLPointFile = newpt (POINTStoPOINT(lpPoints2[i-1]),*AZ,Dist);
							*CLPoint = FilePtToBasePt(CLPointFile);
							return(TRUE);
						}
						TotDist+=Dist;
					}
					return (FALSE);
				}
				break;

	            case 61: 

		 		{   nPnts = *ipnt;
		 		    ipnt++;
		 DoPolylineD:lpDPoints2 = (HPDPOINT) ipnt;
		 		    ipnt = ipnt + nPnts * 8;
					TotDist = 0;
					for (i=1;i<nPnts;i++)
						TotDist += ldistp(lpDPoints2[i-1],lpDPoints2[i]);
					*Length = TotDist;
					WantDist = TotDist * PCT;
					TotDist = 0;
					for (i=1;i<nPnts;i++)
					{
						Dist = ldistp(lpDPoints2[i-1],lpDPoints2[i]);
						if (TotDist + Dist >= WantDist || i==nPnts-1)
						{
							*AZ = getazd(&lpDPoints2[i-1],&lpDPoints2[i]);
							Dist = WantDist - TotDist;
							*CLPoint = dnewpt (lpDPoints2[i-1],*AZ,Dist);
							return(TRUE);
						}
						TotDist+=Dist;
					}
					return (FALSE);
				}
				break;

	            default: 
	            	SkipSubRec (Pcode,&ipnt,0);
                break;

			}
        }
	return(FALSE);
}

void AZtoDirection(double az,LPSTR str)
{   
	double	dinc;

	az = LTWOPI(az);
//	CurAZ = az;
	dinc = HALFPI/4;
	if (az < dinc || az > TWOPI - dinc)
		_fstrcpy(str,"East");
	else if (az < dinc*3)
		_fstrcpy(str,"NE");
	else if (az < dinc*5)
		_fstrcpy(str,"North");
	else if (az < dinc*7)
		_fstrcpy(str,"NW");
	else if (az < dinc*9)
		_fstrcpy(str,"West");
	else if (az < dinc*11)
		_fstrcpy(str,"SW");
	else if (az < dinc*13)
		_fstrcpy(str,"South");
	else
		_fstrcpy(str,"SE"); 
	return;

}

BOOL PointInWBounds (HPDPOINT Point)
{
	 if (IgnoreBounds)
		 return TRUE;
	 if (Point->x > CurView->WBounds.xmx ||
	     Point->y > CurView->WBounds.ymx ||
	     Point->x < CurView->WBounds.xmn ||
	     Point->y < CurView->WBounds.ymn) return (FALSE);
	 return (TRUE);
}

BOOL GetNextHLTListItem (BOOL First,BOOL ProcessItem)
#if ENABLETRACE
{GSSiEnterProg (606);
#endif
{ 
    long	Sequence, Ref;
	HIGHLIGHTDATA	HighlightData;
	
	if (First)
		HltFetchPos = BT_FIRST;
	switch (HltFetchSort)
	{
	case 1:
		if (BT_FIND (hHighlight2,(LPSTR)&Sequence,HltFetchPos,BT_ANY,(LPSTR)&Ref))  
		{
			HltFetchPos = BT_FIRST;
	{
#if ENABLETRACE
GSSiExitProg (606);
#endif
			return FALSE;
}
	}
		HltFetchPos = BT_NEXT; 
   		BT_FIND (hHighlight,(LPSTR)&Ref,BT_FIRST,BT_EQ,(LPSTR)&HighlightData);  
		PickList[0]=HighlightData.PD;
		break;
	case 0:
   		if (BT_FIND (hHighlight,(LPSTR)&Ref,HltFetchPos,BT_ANY,(LPSTR)&HighlightData))  
		{
			HltFetchPos = BT_FIRST;
	{
#if ENABLETRACE
GSSiExitProg (606);
#endif
			return FALSE;
}
	}
		HltFetchPos = BT_NEXT; 
		PickList[0]=HighlightData.PD;
		break;
	}
	if (ProcessItem)
		ProcessPickedItem (0,FALSE);        		
	else
		SetPickGlobals (0);
{
#if ENABLETRACE
GSSiExitProg (606);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}  

short ScanForFieldTypes (LPSTR DBName,LPHANDLE phFieldTypes,BOOL DoScan,long NumToScan)
{   
	HANDLE	hSQL=0;
    LPOPENSQLDATA   SQLPtr=0;
    LPOPENFILEDATA  FilePtr;
    LPFIELDINFO lpFieldInfo; 
	LPGWFLDINFO pFieldTypes;
	long	lval, nScanned;
	double	dval;
    USHORT	i;
    HANDLE	hStr=GSSiGlobAlloc (0,GMEM_MOVEABLE,4096);
    LPSTR	str=GlobalLock (hStr);
    short	NumFields=0;
    BOOL	ReScan=FALSE, useFileLength=FALSE;
    
	
    if (!OpenDataFile (DBName,"",BT_READ,&hSQL))
        goto Exit;
    if (!*phFieldTypes)
    	*phFieldTypes = GSSiGlobAlloc ( 243,GHND,USHRT_MAX); 
    pFieldTypes = (LPGWFLDINFO)GlobalLock (*phFieldTypes);
    while (*pFieldTypes->Name)
    	pFieldTypes++;
    SQLPtr = (LPOPENSQLDATA)GlobalLock (hSQL);
    FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle);
    NumFields = FilePtr->NumFields; 
   	lpFieldInfo = &FilePtr->FldInfo;
   	for (i=0;i<FilePtr->NumFields;i++,lpFieldInfo++)
   	{
   		_fstrcpy (pFieldTypes[i].Name,lpFieldInfo->name);
   		switch (lpFieldInfo->type)
   		{
	        default:
	        case SQL_LONGVARCHAR:
	        case SQL_CHAR: 
   			case SQL_VARCHAR:
			case SQL_UNKCHAR:
   				lpFieldInfo->type = BT_CHAR;
   				break;
	        case SQL_NUMERIC:
	        case SQL_FLOAT:
			case SQL_REAL:
			case SQL_DOUBLE:
			case BT_REAL:
   				lpFieldInfo->type = BT_REAL;
   				break;
	        case SQL_INTEGER:
	        case SQL_SMALLINT:
			case SQL_BIGINT:
			case SQL_TINYINT:   
			case SQL_BIT:
	    	case BT_INTEGER:
   				lpFieldInfo->type = BT_INTEGER;
   				break;  
			case SQL_MSSHAPE:
   			case SQL_LONGVARBINARY:
   				lpFieldInfo->type = BT_INTEGER;
   				break;  
   		}
   	}
	if (DoScan)
	{
		CreateStatusWind (hWndMain,1,"Scanning for field defintions");
		if (NumToScan <= 0)
			NumToScan = NumSQLRows(hSQL);
		if (NumToScan <= 0)
		{
			useFileLength = TRUE;
			NumToScan = GSSiLength(DBName);
		}
	Top:
		nScanned = 0;
		while (nScanned<NumToScan && StatusWindowUpdate(0, 0, NumToScan, nScanned) && FetchDBRec(hSQL))
		{
			if (useFileLength)
				nScanned = GetDBPos(hSQL);
			else
				nScanned++;
	   		lpFieldInfo = &FilePtr->FldInfo;
	   		for (i=0;i<FilePtr->NumFields;i++,lpFieldInfo++)
	   		{
	   			if (lpFieldInfo->type == BT_CHAR || SQL_VARCHAR)
	   			{
					if (GetValFromOpenFiles (lpFieldInfo->name,str,4096) > 0) 
					{
		   				switch (pFieldTypes[i].Type)
		   				{   
		   					case 0:
		   						if (IsInteger(str))
		   						{
		   							pFieldTypes[i].Type = BT_INTEGER;  
		   							lval = atol (str);
		   							if (lval < SHRT_MIN || lval > SHRT_MAX)
			   							pFieldTypes[i].Len = 4;
			   						else
			   							pFieldTypes[i].Len = 2;
		   						}	
		   						else if (_fstrchr (str,'.') && IsReal(str))
		   						{
		   							pFieldTypes[i].Type = BT_REAL; 
		   							if (_fstrlen (str) > 8)
		   								pFieldTypes[i].Len = 8; 
		   							else
		   								pFieldTypes[i].Len = 4; 
		   						}
		   						else
		   						{
		   							pFieldTypes[i].Type = BT_CHAR;
		   							pFieldTypes[i].Len = _fstrlen (str);
		   						}	
		   						break;  
		   					case BT_CHAR:
	   							pFieldTypes[i].Len = max (pFieldTypes[i].Len,_fstrlen (str));
		   						break;
		   					case BT_INTEGER:
		   						if (!IsInteger(str))
		   						{   
			   						if (_fstrchr (str,'.') && IsReal(str))
			   						{
			   							pFieldTypes[i].Type = BT_REAL; 
			   							if (_fstrlen (str) > 8)
			   								pFieldTypes[i].Len = 8; 
			   							else
			   								pFieldTypes[i].Len = 4; 
			   						}
			   						else
			   						{
			   							pFieldTypes[i].Type = BT_CHAR;
			   							pFieldTypes[i].Len = max (pFieldTypes[i].Len,_fstrlen (str));
			   						}   
		   							ReScan = TRUE;
		   						}
		   						else if (pFieldTypes[i].Len == 2 && labs (atol (str)) > SHRT_MAX)
		   							pFieldTypes[i].Len = 4;	
	   							break;
	   						case BT_REAL:
		   						if (!IsReal(str))
		   						{
		   							pFieldTypes[i].Type = BT_CHAR;
		   							pFieldTypes[i].Len = max (pFieldTypes[i].Len,_fstrlen (str));
		   							ReScan = TRUE;
		   						}	
		   						else if (pFieldTypes[i].Len == 4 && _fstrlen(str) > 8)
		   							pFieldTypes[i].Len = 8;	
	   							break;
	   					}
	   				}
   					else if (!pFieldTypes[i].Type)
   					{
						pFieldTypes[i].Type = BT_CHAR;
						pFieldTypes[i].Len = 1;

					}
	   			} 
	   			else
	   			{
					pFieldTypes[i].Type = lpFieldInfo->type;
					pFieldTypes[i].Len = lpFieldInfo->length;
	   			}
	   		}  
			
		} 
		if (ReScan)
		{
			ReScan = FALSE;
			GlobalUnlock (SQLPtr->OFHandle);
			GlobalUnlock (hSQL);
			CloseDataFile (TRUE, &hSQL);  
			OpenDataFile (DBName,"",BT_READ,&hSQL);
			SQLPtr = (LPOPENSQLDATA)GlobalLock (hSQL);
			FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle);
			goto Top;  
		}
		ContinueProcessing = TRUE;
		DestroyStatusWindow(0); 
	}
	else
	{
  		lpFieldInfo = &FilePtr->FldInfo;
	   	for (i=0;i<FilePtr->NumFields;i++,lpFieldInfo++)
	   	{
			pFieldTypes[i].Type = lpFieldInfo->type;
			pFieldTypes[i].Len = lpFieldInfo->length;
		}
	}
    GlobalUnlock (SQLPtr->OFHandle);
    GlobalUnlock (hSQL);
	CloseDataFile (TRUE, &hSQL);  
	GlobalUnlock (*phFieldTypes);
Exit:
	GSSiGlobUlFree (&hStr);
	return NumFields;
}

BOOL GetValFromFieldValue(LPSTR FieldName,HANDLE hFieldTypes, HANDLE hValues, LPSTR str)
{
	BOOL rtn = FALSE;
	LPHANDLE phValues;
	LPSTR pValue;
	LPGWFLDINFO pFieldTypes;

	if (!hFieldTypes || !hValues)
		return FALSE;
	phValues = GlobalLock(hValues);
	pFieldTypes = (LPGWFLDINFO)GlobalLock(hFieldTypes);
	while (*pFieldTypes->Name)
	{
		if (!stricmp(pFieldTypes->Name, FieldName))
		{
			pValue = GlobalLock(*phValues);
			strcpy(str, pValue);
			GlobalUnlock(*phValues);
			rtn = TRUE;
			break;
		}
		pFieldTypes++;
		phValues++;
	}
	GlobalUnlock(hValues);
	GlobalUnlock(hFieldTypes);
	return rtn;
}

long OutputToFile(LPSTR File, BOOL Create, LPSTR DBName, LPSTR pSQL, HANDLE hFieldsIN, HANDLE hKeyFields, HANDLE hFieldTypes, HANDLE hValues, BOOL UseHLT, BOOL OutToScreen, int GMHeader, BOOL Compress, BOOL ScanForFieldTypes, long NumToScan, HWND StatusWnd, HWND hWndDlg, BOOL tabDlm)
#if ENABLETRACE
{GSSiEnterProg (603);
#endif
{   
    HANDLE      hSQL; 
    LPINT       lpField, StartField, pNumFields, pNumKeyFields, pFieldID, pKeyFieldID,pFieldIDIN;
	int			NumFieldsIN;
    short       NumFields,n, FieldID, NumKeyFields;
    long        rtn=0, TotRecs=0, AtRec=0;
    LPSTR		str;
    HANDLE		hStr;
    short       st,ifield,j,nKeyFields=1; 
    LPSTR       lpHead, lpRec, lpEnd;
    LPOPENFILEDATA  FilePtr;
    LPOPENSQLDATA   SQLPtr=0;
    LPFIELDINFO lpFieldInfo; 
    HANDLE      SaveHandle, hRec=0, hHead=0;
    HFILE       OutFid=HFILE_ERROR, IndexFid=HFILE_ERROR;     
    long        iref, nRecs, Recloc,maxrowlen;
	LPGWDHEADER lpGWDHead;
	HANDLE  hDB=0;  
	HANDLE		hFields=0, hMyFields=0;
    extern  HANDLE  hHighlight; 
    long	OriginalRecordNumber=0; 
	BOOL	WantXML=GetGlobalBVal2 ("[%WANTXML]",TRUE);
	BOOL	useFileLength = FALSE;
	char	dlm[4]=",";
    
    HIGHLIGHTDATA   HighlightData;
    
    char    IndexFile[MAX_PATH];
    
	if (GMHeader == 2)
		tabDlm = FALSE;
	if (tabDlm)
		strcpy (dlm,"\t");
    hSQL=0;
    if (!OpenDataFile (DBName,pSQL,BT_READ,&hSQL))
{
#if ENABLETRACE
GSSiExitProg (603);
#endif
        return FALSE;   
}
    hStr = GSSiGlobAlloc (0,GMEM_MOVEABLE,4096);
    str = GlobalLock (hStr);
    if (UseHLT)
	{
    	TotRecs = BT_NUM_IN_INDEX (hHighlight); 
		if (!TotRecs)
    		goto Exit; 
	}
    else if (StatusWnd)
    {
    	HCURSOR	OldCursor;
	                	
        OldCursor = GSSiSetCursor (LoadCursor (0,IDC_WAIT)); 
		if (GetDBType(hSQL) == GMTEXT_DATAFILE)
		{
			TotRecs = GSSiLength(DBName);
			useFileLength = TRUE;
		}
		else
    		TotRecs = NumSQLRows (hSQL); 
        GSSiSetCursor (OldCursor);
		if (!TotRecs)
    		goto Exit; 
    }
    hHead = GSSiGlobAlloc ( 180,GHND,USHRT_MAX);
    hRec  = GSSiGlobAlloc ( 181,GMEM_MOVEABLE,USHRT_MAX);   
    lpRec = GlobalLock(hRec);
    lpHead = GlobalLock(hHead);
	if (!GetPathType (File))
		Create = TRUE;
	if (Create)
	{
		OutFid = GSSiOpenFile (File,0,OF_CREATE);
		if (GMHeader < 2)
		{
			if (OutFid == HFILE_ERROR)
			{
	    		GSSiMessageBox (File,"Unable to create output file",MB_ICONEXCLAMATION,0);
	    		goto Exit;
			} 
		}
		if (OutToScreen)
		{ 
			GSSiGetTempFileName (0,"gmi",0,(LPSTR)IndexFile); 
			IndexFid = GSSiOpenFile(IndexFile,0,OF_CREATE);
		}
		else
			IndexFid=HFILE_ERROR; 
        
		SQLPtr = (LPOPENSQLDATA)GlobalLock (hSQL);
		FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle);
		if (!hKeyFields && hFieldsIN)
			hFields = hFieldsIN;
		else
		{
			hMyFields = GSSiGlobAlloc ( 182,GMEM_MOVEABLE,USHRT_MAX);
			hFields = hMyFields; 
			pNumFields = (LPINT)GlobalLock (hFields); 
			*pNumFields = 0;
			pFieldID = pNumFields + 1;
			if (hKeyFields)
			{
				pNumKeyFields = (LPINT)GlobalLock (hKeyFields); 
				nKeyFields = NumKeyFields = *pNumKeyFields;
				pKeyFieldID = pNumKeyFields + 1;
				while (NumKeyFields--)
				{
					*pFieldID++ = *pKeyFieldID++;
					(*pNumFields)++; 
				}
				GlobalUnlock (hKeyFields);
			}
			if (hFieldsIN)
			{
				LPINT	pNumFieldsIN =(LPINT) GlobalLock (hFieldsIN);
				NumFieldsIN = *pNumFieldsIN;
				pFieldIDIN = pNumFieldsIN + 1;
			}
			else 
			{
				FieldID = -1;
				NumFieldsIN = FilePtr->NumFields;  
			}
			for (ifield = 0; ifield < NumFieldsIN; ifield++)
			{   
				if (hFieldsIN)
					FieldID = *pFieldIDIN++;
				else
					FieldID++; 
				if (FieldID < 0)
				{
					*pFieldID++ = FieldID;
					(*pNumFields)++;
				}
				else
				{
					lpFieldInfo = &FilePtr->FldInfo + FieldID;
					if (WantXML || !StringEndsWith(lpFieldInfo->name, "_XML"))
					{
						if (!InFieldIDList(FieldID, hKeyFields) && lpFieldInfo->type != -151)
						{
							*pFieldID++ = FieldID;
							(*pNumFields)++;
						}
					}
				}
			}
			if (hFieldsIN)
				GlobalUnlock (hFieldsIN);
			GlobalUnlock (hFields);
		}
			
		lpFieldInfo = &FilePtr->FldInfo;
		pFieldID = (LPINT)GlobalLock (hFields);
		NumFields = *pFieldID++;   
		for (ifield=0;ifield<NumFields;ifield++,pFieldID++)
		{   
			if (*lpHead)
				_fstrcat (lpHead,dlm);
    		if (*pFieldID == -1)
    			_fstrcat (lpHead,"UNIQUEID(B4)"); 
			else if (*pFieldID < 0)
			{
				if (hFieldTypes && GMHeader)
				{
					LPGWFLDINFO pFieldTypes = (LPGWFLDINFO)GlobalLock(hFieldTypes);
					FIELDINFO fieldInfo;

					memset(&fieldInfo, 0, sizeof(FIELDINFO));
					pFieldTypes += abs(*pFieldID + 2);
					strcpy(fieldInfo.name, pFieldTypes->Name);
					fieldInfo.type = pFieldTypes->Type;
					fieldInfo.length = pFieldTypes->Len;
					if (!CreateGMTextHeader(&fieldInfo, lpHead))
					{
						GlobalUnlock(hFields);
						GlobalUnlock(hFieldTypes);
						goto Exit;
					}
					GlobalUnlock(hFieldTypes);
				}
			}
			else
    		{
	    		lpFieldInfo = &FilePtr->FldInfo + *pFieldID;
				if (GMHeader)
				{   
					if (hFieldTypes)
					{
						LPGWFLDINFO pFieldTypes=(LPGWFLDINFO)GlobalLock (hFieldTypes);

						while (*pFieldTypes->Name)
						{ 
							if (!_fstricmp (pFieldTypes->Name,lpFieldInfo->name))
							{
								lpFieldInfo->type = pFieldTypes->Type;
								lpFieldInfo->length = pFieldTypes->Len; 
								break;
							}
							pFieldTypes++;
						}
						GlobalUnlock (hFieldTypes);
					} 
					if (!CreateGMTextHeader (lpFieldInfo,lpHead)) 
					{
						GlobalUnlock (hFields);
						goto Exit;
					}
				}
				else
				{ 
					if (lpFieldInfo->type == BT_RIGHT_CHAR || lpFieldInfo->type == BT_CHAR ||
						lpFieldInfo->type == SQL_CHAR || lpFieldInfo->type == SQL_VARCHAR || lpFieldInfo->type == SQL_UNKCHAR)
		        		sprintf (_fstrchr(lpHead,0),"\"%s\"",lpFieldInfo->name);
					else
		        		sprintf (_fstrchr(lpHead,0),"%s",lpFieldInfo->name);
				}
			}
		}
		GlobalUnlock (hFields);
		if (GMHeader == 2)
		{   
			GSSiClose (OutFid);
			OutFid = HFILE_ERROR;
			if (!CreateGWDDatabase (File,1,Compress,0,nKeyFields,lpHead))
			{
	    		GSSiMessageBox (File,"Unable to create output file",MB_ICONEXCLAMATION,0);
	    		goto Exit; 
			}
		}
		else		
    		fputstring (lpHead,OutFid);
	}
	if (GMHeader == 2)
	{
		hDB = OpenGWDatabase(File, BT_WRITE);
		{
			LPGWDHEADER	lpGWDHead;

			lpGWDHead = (LPGWDHEADER)GlobalLock(hDB);
			OriginalRecordNumber = BT_NUM_IN_INDEX(lpGWDHead->BTHandle[0]);
			GlobalUnlock(hDB);
		}
		lpGWDHead = (LPGWDHEADER)GlobalLock (hDB);
	}
	else
		GSSillseek (OutFid,0,2);
	nRecs=0;
    maxrowlen=0;
    if (UseHLT)
	    st = BT_FIND (hHighlight,(LPSTR)&iref,BT_FIRST,BT_ANY,(LPSTR)&HighlightData); 
	else
		st = 0;  
	if (StatusWnd == (HWND)1)
		CreateStatusWind (hWndDlg,1,File);
    while (!st && ContinueProcessing)
    {   
    	OriginalRecordNumber++;
    	if (UseHLT)
    	{
	   		PickList[0]=HighlightData.PD;
			ProcessPickedItem (0,FALSE);  
	    } 
		if (useFileLength)
			AtRec = GetDBPos(hSQL);
		else
			AtRec++;
		if (TotRecs && StatusWnd) 
		{
			*str = 0;
	    	//sprintf (str,"%ld records written",AtRec);
	    	//SetWindowText (StatusWnd,str);
			if (StatusWnd == (HWND)1)
				ContinueProcessing = StatusWindowUpdate (0,str, TotRecs,AtRec);
			else     		
	        	PctBox (StatusWnd,TotRecs,AtRec,-1); 
	    }
		else if (!useFileLength)
	    {
	    	sprintf (str,"%ld records written",AtRec);
	    	SetWindowText (hWndDlg,str);
	    }
		if (Create)
		{
			pFieldID = (LPINT)GlobalLock (hFields);
			NumFields = *pFieldID++;  
		}
		else if (GMHeader == 2)
		{
			NumFields = lpGWDHead->NumFields;
			pFieldID = 0;
		}
        *lpRec = 0;
        for (ifield=0;ifield<NumFields;ifield++)
        {   
        	char	FieldName[64]; 
        	short	FieldType;
        	
        	if (pFieldID)
			{
				if (*pFieldID == -1) 
        		{
        			_fstrcpy (FieldName,"UNIQUEID");
        			ltoa (OriginalRecordNumber,str,10);   
        			FieldType = BT_INTEGER;
        		}
				else if (*pFieldID < 0)
				{
					LPGWFLDINFO pFieldTypes = (LPGWFLDINFO)GlobalLock(hFieldTypes);

					pFieldTypes += abs(*pFieldID + 2);
					_fstrcpy(FieldName, pFieldTypes->Name);
					FieldType = pFieldTypes->Type;
					*str = 0;
					if (pFieldTypes->ValueID && hValues)
					{
						int id = pFieldTypes->ValueID-1;
						LPHANDLE phValues = GlobalLock(hValues);
						LPSTR pValues;

						while (id--)
							phValues++;
						pValues = GlobalLock(*phValues);
						strcpy(str, pValues);
						GlobalUnlock(*phValues);
						GlobalUnlock(hValues);
					}
					GlobalUnlock(hFieldTypes);
				}
        		else 
        		{
			    	lpFieldInfo = &FilePtr->FldInfo + *pFieldID;  
		    		_fstrcpy (FieldName,lpFieldInfo->name);   
		    		FieldType = lpFieldInfo->type;
					if (GetValFromOpenFiles (lpFieldInfo->name,str,4096) < 0)
						goto NextHlt;
				} 
			}
			else
        	{
		    	LPGWFLDINFO pFieldInfo = lpGWDHead->pFldInfo + ifield;  

		    	_fstrcpy (FieldName,pFieldInfo->Name);   
		    	FieldType = pFieldInfo->Type;
				if (!stricmp(FieldName, "UNIQUEID"))
					ltoa(OriginalRecordNumber, str, 10);
				else if (GetValFromFieldValue(FieldName,hFieldTypes, hValues, str))
					;
				else if (GetValFromOpenFiles (FieldName,str,4096) < 0)
					goto NextField;
			} 
			if (GMHeader == 2)
			{
		 		 SetFieldValFromCharAndName(lpGWDHead,FieldName,str,FALSE);
			}
			else
			{
	            Truncate (str);
	            if (*lpRec)
	            	_fstrcat (lpRec,dlm);
				if (!tabDlm &&
					(FieldType == BT_RIGHT_CHAR || FieldType == BT_CHAR ||
					FieldType == SQL_CHAR || FieldType == SQL_VARCHAR || FieldType == SQL_UNKCHAR))
		        	sprintf (_fstrchr(lpRec,0),"\"%s\"",str);
		        else
		        	sprintf (_fstrchr(lpRec,0),"%s",str);
		    }
NextField:
			if (pFieldID)
				pFieldID++;
			if (GMHeader == 2 && !Create && ifield+1 == lpGWDHead->NumIndexFields[0])
			{
		    	LPGWFLDINFO pFieldInfo = lpGWDHead->pFldInfo + ifield+1; 
				long	Offset;

		        GWDFormKey(lpGWDHead,0,FALSE,0,0);
         		if (!BT_FIND (lpGWDHead->BTHandle[0],lpGWDHead->pKeys[0],BT_FIRST,BT_EQ,(LPSTR)&Offset))
					FillGWDData (lpGWDHead,Offset);
				else
					memset (&lpGWDHead->GWDData[pFieldInfo->Beg],0,lpGWDHead->Reclen-pFieldInfo->Beg);
			}
        }
NextHlt: 
	    if (hFields)
			GlobalUnlock (hFields);
		if (GMHeader == 2)
		{ 
			GWDReplaceRecord (lpGWDHead,0,NULL,-1); 
			rtn++;
		}
		else if (_fstrlen(lpRec)>1)
		{
	        
	        Recloc = GSSillseek(OutFid,0,1);
	        if (IndexFid != HFILE_ERROR)
	            BigWrite(IndexFid,(HPSTR)&Recloc,sizeof(Recloc),-1); 
	        nRecs++;
	        fputstring (lpRec,OutFid);
	        maxrowlen = max(maxrowlen,(signed)_fstrlen(lpRec)); 
	        rtn++;  
	    }
//	    if (CheckForContinue ())
	    {
			if (UseHLT)
	        	st = BT_FIND (hHighlight,(LPSTR)&iref,BT_NEXT,BT_ANY,(LPSTR)&HighlightData);
	        else
			{
				if (FetchDBRec (hSQL))
					st=0;
				else
					st=1;
			}
		}
			
    }
	if (!ContinueProcessing)
	{
    	GSSiMessageBox ("Operation cancelled by user","",MB_ICONEXCLAMATION,0);
    	ContinueProcessing = TRUE; 
    	rtn = FALSE;
    }
    if (IndexFid != HFILE_ERROR)
    {   long rec;
        GSSillseek (IndexFid,0,0);
        for (rec=0;rec<nRecs;rec++)
        {
            BigRead(IndexFid,(HPSTR)&Recloc,sizeof(Recloc));
            BigWrite(OutFid,(HPSTR)&Recloc,sizeof(Recloc),-1);
        }
        BigWrite(OutFid,(HPSTR)&nRecs,sizeof(nRecs),-1);
        BigWrite(OutFid,(HPSTR)&maxrowlen,sizeof(maxrowlen),-1);
        GSSiClose(IndexFid);
        GSSiRemove(IndexFile);
    } 
    if (OutFid != HFILE_ERROR)     
    	GSSiClose (OutFid);
Exit:
    GSSiGlobUlFree(&hRec);
    GSSiGlobUlFree(&hHead);
    if (SQLPtr)
    {
	    GlobalUnlock (SQLPtr->OFHandle);
	    GlobalUnlock (hSQL);
	}
	if (hDB)
	{
		GlobalUnlock (hDB);
			CloseGWDatabase (hDB);
	} 
	if (StatusWnd == (HWND)1)
		DestroyStatusWindow (0);
    CloseDataFile (TRUE, &hSQL);      
    GSSiGlobFree (&hMyFields);  
    GSSiGlobUlFree (&hStr);
{
#if ENABLETRACE
GSSiExitProg (603);
#endif
    return (rtn);
}
#if ENABLETRACE
}
#endif
}  
RGBTRIPLE COLORREFtoRGBTRIPLE (COLORREF cr)
{
	RGBTRIPLE	rgb;
	
	rgb.rgbtRed = GetRValue (cr);
	rgb.rgbtGreen = GetGValue (cr);
	rgb.rgbtBlue = GetBValue (cr);
	return rgb;
}
void GetFillPatternSignature (HDC hDC,POINT pt,short pixdist,double colordist,LPFILLSIGNATURE pSign)
{ 
	short xoffsets[]={ 0, 0, 1, 1, 1, 0,-1,-1,-1, 0, 1, 2, 2, 2, 2, 2, 1, 0,-1,-2,-2,-2,-2,-2,-1, 0, 1, 2, 3, 3, 3, 3, 3, 3, 3, 2, 1, 0,-1,-2,-3,-3,-3,-3,-3,-3,-3,-2,-1,
					   0, 1, 2, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 2, 1, 0,-1,-2,-3,-4,-4,-4,-4,-4,-4,-4,-4,-4,-3,-2,-1};
	short yoffsets[]={ 0, 1, 1, 0,-1,-1,-1, 0, 1, 2, 2, 2, 1, 0,-1,-2,-2,-2,-2,-2,-1, 0, 1, 2, 2, 3, 3, 3, 3, 2, 1, 0,-1,-2,-3,-3,-3,-3,-3,-3,-3,-2,-1, 0, 1, 2, 3, 3, 3,
					   4, 4, 4, 4, 4, 3, 2, 1, 0,-1,-2,-3,-4,-4,-4,-4,-4,-4,-4,-4,-4,-3,-2,-1, 0, 1, 2, 3, 4, 4, 4, 4};
	RGBTRIPLE	Color;
	USHORT	i = sizeof(xoffsets)/2, j;
	double	MaxColorDist = 50, d;
	
	if (pixdist)
		i = 49;
	pSign->nColors=0;
	while (i--)
	{				
		Color = COLORREFtoRGBTRIPLE (GetPixel (hDC,pt.x+xoffsets[i],pt.y+yoffsets[i])); 
		for (j=0;j<pSign->nColors;j++)
		{ 
			if ((d=RGBDist (Color,pSign->Colors[j])) <= MaxColorDist)  
			{
				pSign->ColorsN[j]++;
				pSign->Colors[j].rgbtRed   = IDNINT(((double)pSign->Colors[j].rgbtRed   * (pSign->ColorsN[j]-1) + (double)Color.rgbtRed)   / pSign->ColorsN[j]);
				pSign->Colors[j].rgbtGreen = IDNINT(((double)pSign->Colors[j].rgbtGreen * (pSign->ColorsN[j]-1) + (double)Color.rgbtGreen) / pSign->ColorsN[j]);
				pSign->Colors[j].rgbtBlue  = IDNINT(((double)pSign->Colors[j].rgbtBlue  * (pSign->ColorsN[j]-1) + (double)Color.rgbtBlue)  / pSign->ColorsN[j]);
				goto NextPixel;
			}	
		} 
		pSign->Colors[pSign->nColors] = Color;
		pSign->ColorsN[pSign->nColors++] = 1;
		if (pSign->nColors >= MAXSIGNATURECOLORS)
			break;  
NextPixel:;
	}
	return;
} 

double CompareFillPatternSignatures (LPFILLSIGNATURE pSign1,LPFILLSIGNATURE pSign2)
{
	double val=0, d, mind;  
	short	i,j, minj;
	
	for (i=0;i<pSign1->nColors;i++)
	{
		mind = DBL_MAX;
		for (j=0;j<pSign2->nColors;j++)
		{
			d = RGBDist (pSign1->Colors[i],pSign2->Colors[j]);
			if (d < mind)
			{
				mind = d;
				minj = j;
			}
		}
		val += mind * pSign2->ColorsN[minj] * pSign1->ColorsN[i];
	}
	for (i=0;i<pSign2->nColors;i++)
	{
		mind = DBL_MAX;
		for (j=0;j<pSign1->nColors;j++)
		{
			d = RGBDist (pSign2->Colors[i],pSign1->Colors[j]);
			if (d < mind)
			{
				mind = d;
				minj = j;
			}
		}
		val += mind * pSign1->ColorsN[minj] * pSign2->ColorsN[i];
	}
	return val;
}   



short OpenLegendFile (void)
{
		LPSTR	pBS;    
		short	i;
		
    	_fstrcpy (LegendFileName,CurView->lpFiles[0]);
    	pBS = _fstrrchr (LegendFileName,'\\');
    	_fstrcpy (pBS,"\\legends.bin");
    	FidMap = GSSiOpenFile (LegendFileName,0,OF_READ);
    	if (FidMap == HFILE_ERROR)
   			nLegends = 0;  
   		else
   		{   
   			BigRead (FidMap,(HPSTR)&nLegends,sizeof(nLegends));
   			for (i=0;i<nLegends;i++)
   			{
   				BigRead (FidMap,(HPSTR)&LegendBounds[i],sizeof(MNMXCORD));
   				BigRead (FidMap,(HPSTR)&LegendSignature[i],sizeof(FILLSIGNATURE)); 
   			} 
   			GSSiClose (FidMap); 
   		}
   		return nLegends;
}

BOOL SelectLegend (LPFILLSIGNATURE pSignature)
{    
	short	iview,i;  
	LPVIEWPORT	SaveVP=CurView;  
	BOOL rtn=TRUE;
	double	d, mind=DBL_MAX;
	short	mini=0;
	
	SetConfig (1); 
	for (iview=0;iview<*pNumViewports;iview++)
	{
		if (pViewportsD[iview]->Type == LEGENDIMAGEVIEWPORT)
		{
			CurView = pViewportsD[iview];
			goto Open;
		}
	}
	return FALSE;
Open:
	if (!OpenLegendFile ())
	{
		CurView = SaveVP;
		return FALSE;
	} 
	for (i=0;i<nLegends;i++)
	{
		d = CompareFillPatternSignatures (pSignature,&LegendSignature[i]);  
		if (d < mind) 
		{
			mind = d;
			mini = i;
		}
	}
    ZoomToRect(LegendBounds[mini],TRUE);
	
	CurView = SaveVP;
	return rtn;
}





HANDLE OpenMapIndex (LPSTR Name,LPMNMXCORD pIndexBounds)
#if ENABLETRACE
{GSSiEnterProg (61);
#endif
{
    OFSTRUCTGM    OFStruct; 
    short     Version;
    HFILE   FidIndex=HFILE_ERROR, FidZM;
    long    Signature, EndOffset;
    HANDLE  Handle;
    LPFILEINDEX lpIndex;
    FILEINDEX FirstIndex;   
    MNMXCORD    TestBounds;
	MNMXCORD	FileBounds;
    char        File[MAX_PATH], drive[8], dir[MAX_PATH], leaf[40], IndexRes[16]; 
	char		inName[MAX_PATH];
    short	SaveUnits;

    strcpy (ImageExtension,".gci");
    _fstrcpy (File,Name);
    ExpandText (File);
Start:
	strcpy(inName, File);
	_splitpath(File, drive, dir, leaf, 0);
    if (!_fstrnicmp (leaf,"index",5))
    	_fstrcpy (IndexRes,&leaf[5]);
    else
    	IndexRes[0]=0;
    UseDGNColors = TRUE;
    SetGlobalValue ("%WANTPASS","");
    SetGlobalValue ("%INDEXRES",IndexRes);
    sprintf (File,"%s%sglobal.ini",drive,dir);
    if (!pIndexBounds && !CurView->SubFile)
		ProcessGlobal ("[%LAYERINIT]");  
	SaveUnits = PRJ_UNITS[3];
	PRJ_UNITS[3] = PRJ_UNITS[1];
    LoadGlobalInit (File,FALSE); 
	LayerUnits = PRJ_UNITS[3];  
	PRJ_UNITS[3] = SaveUnits;
    if (!pIndexBounds && !WantThisPass ())
    {
        GSSiClose (FidIndex);
{
#if ENABLETRACE
GSSiExitProg (61);
#endif
        return 0;
}
    }
    
	if (!pIndexBounds)
		GetViewportScale (CurView->hDC);  
	if (!Pick && Display)
	{
	    sprintf (File,"%s%szoommac.txt",drive,dir);
	    FidZM = GSSiOpenFile (File,0,OF_READ);
	    if (FidZM != HFILE_ERROR)
	    {   
	    	ProcessZoomMacroFile2(CurView->hDC,FidZM);
	    	GSSiClose (FidZM);
	    } 
	}
	//The [%ORTHORES] value may have changed
	_fstrcpy(File, Name);
	ExpandText(File);
	if (stricmp(File, inName))
	{
		goto Start;
	}
	FidIndex = GSSiOpenFile(Name, (LPOFSTRUCTGM)&OFStruct, OF_READ);
	if (FidIndex == HFILE_ERROR)
	{
#if ENABLETRACE
		GSSiExitProg(61);
#endif
		return(0);
	}

    EndOffset = GSSillseek(FidIndex,(LONG)-(6),2);
    BigRead (FidIndex,(HPSTR)&Signature,4);
    BigRead (FidIndex,(HPSTR)&Version,2);
    if (Signature != 80251)
    {    
        GSSiClose (FidIndex); 
        ContinueProcessing=FALSE;
        GSSiMessageBox("This is not a valid index file",File, MB_OK,0);
{
#if ENABLETRACE
GSSiExitProg (61);
#endif
        return(0);
}
    }
    if (Version > 3)
    {   
    BadMap: 
        GSSiClose (FidIndex); 
        ContinueProcessing=FALSE;
        GSSiMessageBox("This index file version is not recognized",OFStruct.szPathName, MB_OK,0);
{
#if ENABLETRACE
GSSiExitProg (61);
#endif
        return(0);
}
    }
    if (Version < 1) goto BadMap; 
    if (Version == 1)
    {
        GSSiClose(FidIndex);
        ConvertIndexV1ToV2(Name); 
        goto Start;
    }
    if (Version > 2)
		strcpy (ImageExtension,".gco");
    GSSillseek(FidIndex,0,0);
    BigRead (FidIndex,(HPSTR)&FileBounds,sizeof(MNMXCORD));
    if (!pIndexBounds && !AdjustFileBounds (CurView->ID,FileNum,&FileBounds,0))
    {
        GSSiClose (FidIndex);
{
#if ENABLETRACE
GSSiExitProg (61);
#endif
        return 0;
}
    }
	if (pIndexBounds)
		*pIndexBounds = FileBounds;
	if (!pIndexBounds)
	{
		CurView->FileMNMX = FileBounds;
    	SetRezoomBounds (FileBounds);   
	    if (!CurView->HaveBounds)
	        CurView->NewBounds = FileBounds;
	    if (!RectInWBounds(&FileBounds,0) &&(!UpdateOrthoIndex||PltType!=5))
	    {
	        GSSiClose (FidIndex);
{
#if ENABLETRACE
GSSiExitProg (61);
#endif
        return 0;
}
	    }
	}
    BigRead (FidIndex, (HPSTR)&FirstIndex,STOREDINDEXLENGTH);
    FirstIndex.EndOffset = EndOffset; 
    FirstIndex.FirstFoundFile=TRUE;
    _fstrcpy(FirstIndex.FileName,OFStruct.szPathName);
    Handle = GSSiGlobAlloc (  51,GMEM_MOVEABLE,sizeof(FILEINDEX)+FirstIndex.Length);
    lpIndex = (LPFILEINDEX)GlobalLock(Handle);
    *lpIndex = FirstIndex;
    lpIndex->FirstIndexFileOffset = GSSillseek (FidIndex,0,1);
    lpIndex->FileInIndex=0; 
    lpIndex->CurrentEntryOffset=-1; 
    if (lpIndex->NumFiles > USHRT_MAX)
    	lpIndex->NumFiles = USHRT_MAX - 1;
    BigRead (FidIndex,&lpIndex->FirstIndex,(size_t)lpIndex->Length);
    lpIndex->NextHeaderOffset = GSSillseek (FidIndex,0,1);
    GSSiClose (FidIndex);  
    TestBounds = lpIndex->Bounds;  
    if (!pIndexBounds)
    {
	    if (!AdjustFileBounds (CurView->ID,FileNum,&TestBounds,0))
	    	goto NotIn;
    
//    if (!RectInWBounds(&TestBounds) &&(!UpdateOrthoIndex||PltType!=5)) 
	    if (!RectInWBounds(&TestBounds,0)&&((!UpdateOrthoIndex&&!PickOrtho&&(*CurView->OrthoDisplayName==0))||PltType!=5))
	    { 
NotIn:   
	        lpIndex->FileInIndex+=lpIndex->NumFiles;
	        if (!(lpIndex=GetNextIndexHeader(&Handle,TRUE)))
{
#if ENABLETRACE
GSSiExitProg (61);
#endif
            return 0;
}
	    } 
	}
    lpIndex->CurrentEntry=0; 
    GlobalUnlock (Handle); 
    LoadIndexParm (Name);
{
#if ENABLETRACE
GSSiExitProg (61);
#endif
    return Handle;  
}
#if ENABLETRACE
}
#endif
}  

short GetMapIndexType (LPSTR Name)
{   
	short	Type = 0;
    OFSTRUCTGM    OFStruct; 
    short     Version;
    HFILE   FidIndex;
    long    Signature, EndOffset;
    HANDLE  Handle;
    LPFILEINDEX lpIndex;
    FILEINDEX FirstIndex;   
    MNMXCORD    TestBounds;
	MNMXCORD	FileBounds;
    char        File[MAX_PATH], drive[8], dir[MAX_PATH], leaf[40], IndexRes[16];
    
    _fstrcpy (File,Name);
    ExpandText (File);
    _splitpath (File,drive,dir,leaf,0); 
    if (!_fstrnicmp (leaf,"index",5))
    	_fstrcpy (IndexRes,&leaf[5]);
    else
    	IndexRes[0]=0;
	FidIndex = GSSiOpenFile(Name, (LPOFSTRUCTGM)&OFStruct, OF_READ);
    if (FidIndex == HFILE_ERROR)
        return(0);                                                
    EndOffset = GSSillseek(FidIndex,(LONG)-(6),2);
    BigRead (FidIndex,(HPSTR)&Signature,4);
    BigRead (FidIndex,(HPSTR)&Version,2);
    if (Signature != 80251 || Version != 2)
    {    
        GSSiClose (FidIndex); 
        return(0);
    }
        
    GSSillseek(FidIndex,0,0);
    BigRead (FidIndex,(HPSTR)&FileBounds,sizeof(MNMXCORD));
    BigRead (FidIndex,(HPSTR) &FirstIndex,STOREDINDEXLENGTH);
    GSSiClose (FidIndex);  
	return FirstIndex.Type;
}

LPFILEINDEX GetNextIndexHeader(LPHANDLE pHandle,BOOL UseCurView)
#if ENABLETRACE
{GSSiEnterProg (62);
#endif
{
    HFILE   FidIndex=0;
    OFSTRUCTGM    OFStruct; 
    LPFILEINDEX lpIndex;
    FILEINDEX CurIndex,NextIndex;
    short	ii;
    
    GlobalUnlock(*pHandle);
    lpIndex = (LPFILEINDEX)GlobalLock(*pHandle);
    CurIndex = *lpIndex;
    GSSiGlobUlFree (pHandle);
Next:
    if (CurIndex.NextHeaderOffset >= CurIndex.EndOffset)
    { 
NotFound:
        *pHandle = 0;  
	   	if (MapFileType (CurIndex.FileName) == MT_DTM)
			DisplayContourLabels (FALSE);
		LoadIndexParm (0); 
        if (FidIndex) 
        {
            GSSiClose(FidIndex);  
            CloseRefIndex (TRUE);
{
#if ENABLETRACE
GSSiExitProg (62);
#endif
        	return 0;
}
		}
    }
    if (!FidIndex)      
        FidIndex = GSSiOpenFile(CurIndex.FileName,&OFStruct,OF_READ); 
    if (CurIndex.NextHeaderOffset == 321)
    	ii=1;
    if (GSSillseek(FidIndex,CurIndex.NextHeaderOffset,0) == HFILE_ERROR)
    	goto NotFound;
    if (BigRead (FidIndex,(HPSTR) &NextIndex,STOREDINDEXLENGTH) != STOREDINDEXLENGTH)
    	goto NotFound;
    NextIndex.NumFiles+=CurIndex.NumFiles; 
    NextIndex.EndOffset = CurIndex.EndOffset; 
    NextIndex.FileInIndex = CurIndex.FileInIndex; 
    NextIndex.FirstFoundFile=CurIndex.FirstFoundFile;
    _fstrcpy(NextIndex.FileName,CurIndex.FileName);
    NextIndex.NextHeaderOffset = GSSillseek (FidIndex,0,1)+NextIndex.Length;
    if (UseCurView)
    {
    	if (!AdjustFileBounds (CurView->ID,FileNum,&NextIndex.Bounds,0))
    		goto GetNextIndex;
	    if (!RectInWBounds(&NextIndex.Bounds,0)&&((!UpdateOrthoIndex&&!PickOrtho&&(*CurView->OrthoDisplayName==0))||PltType!=5))
	    {
GetNextIndex:
	        GSSillseek(FidIndex,NextIndex.NextHeaderOffset,0); 
	        NextIndex.FileInIndex=NextIndex.NumFiles;     
	        CurIndex = NextIndex;
	        goto Next;
	    } 
	}
    *pHandle = GSSiGlobAlloc (  52,GMEM_MOVEABLE,sizeof(FILEINDEX)+NextIndex.Length);
    lpIndex = (LPFILEINDEX)GlobalLock(*pHandle);
    *lpIndex = NextIndex;
    lpIndex->FirstIndexFileOffset = GSSillseek (FidIndex,0,1);
    BigRead (FidIndex,&lpIndex->FirstIndex,(size_t)lpIndex->Length);
    lpIndex->NextHeaderOffset = GSSillseek (FidIndex,0,1);
    lpIndex->CurrentEntryOffset=-1;
    GSSiClose (FidIndex);
    
{
#if ENABLETRACE
GSSiExitProg (62);
#endif
    return lpIndex;
}
#if ENABLETRACE
}
#endif
}

void CloseMapIndex (LPSTR Name,HANDLE hlpFI, BOOL Write,BOOL CloseParmFiles)
#if ENABLETRACE
{GSSiEnterProg (63);
#endif
{ 
    OFSTRUCTGM    OFStruct; 
    short FidIdx;
    short     Version=1;
    long    Signature=80251; 
    long    ii;
    LPFILEINDEX lpFI; 
    
   	if (MapFileType (Name) == MT_DTM)
		DisplayContourLabels (FALSE);
	if (CloseParmFiles)
		LoadIndexParm (0); 
    if (!hlpFI)
{
#if ENABLETRACE
GSSiExitProg (63);
#endif
    	return;
}
    lpFI = (LPFILEINDEX)GlobalLock(hlpFI);
    if (Write)
    { 
        FidIdx = GSSiOpenFile (Name,(LPOFSTRUCTGM) &OFStruct,OF_CREATE);
        BigWrite (FidIdx,(char *)&lpFI->Type,2,-1);
        BigWrite (FidIdx,(char *)&lpFI->NumFiles,2,-1);
        BigWrite (FidIdx,(char *)&lpFI->Length,4,-1);
        BigWrite (FidIdx,(char *)&lpFI->OrthoRes,8,-1);
        BigWrite (FidIdx,&lpFI->FirstIndex,lpFI->Length-sizeof(FILEINDEX),-1);
        BigWrite (FidIdx,(char *)&Signature,4,-1);
        BigWrite (FidIdx,(char *)&Version,2,-1);
        GSSiClose (FidIdx);
    }
    GSSiGlobUlFree (&hlpFI);
{
#if ENABLETRACE
GSSiExitProg (63);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}
BOOL GetNextIndexEntry (LPFILEINDEX lpIndex)
#if ENABLETRACE
{GSSiEnterProg (64);
#endif
{   
    short     ii;
    char    FAR *pnt;
    LPFILEINDEXENTRY	pCurFileIndexEntry=&CurFileIndexEntry;
   	LPSTR	pDot;
    
    if (lpIndex->FileInIndex >= (long)lpIndex->NumFiles)
{
#if ENABLETRACE
GSSiExitProg (64);
#endif
    	return FALSE;
}
    pnt = (char *)(LPFILEINDEXENTRY)&lpIndex->FirstIndex;
    if (lpIndex->CurrentEntryOffset<0)
    { 
        lpIndex->CurrentEntryOffset=0; 
        lpIndex->CurrentEntry = (LPFILEINDEXENTRY)pnt;
        lpIndex->FileInIndex++;
        if (*lpIndex->CurrentEntry->Name != '.') 
        {   
        	
        	_fstrcpy (CurrentOrthoOrigName,lpIndex->CurrentEntry->Name); 
        	if ((pDot = _fstrrchr (CurrentOrthoOrigName,'.')))
        		*pDot = 0;
        }
        goto Exit;
    }                                                       
    pnt += lpIndex->CurrentEntryOffset;
    lpIndex->CurrentEntry = (LPFILEINDEXENTRY)pnt;
    pnt += lpIndex->CurrentEntry->Len;
    lpIndex->CurrentEntryOffset += lpIndex->CurrentEntry->Len;
    lpIndex->CurrentEntry = (LPFILEINDEXENTRY) pnt;
    if (*lpIndex->CurrentEntry->Name != '.') 
    {   
        	
    	_fstrcpy (CurrentOrthoOrigName,lpIndex->CurrentEntry->Name); 
    	if ((pDot = _fstrrchr (CurrentOrthoOrigName,'.')))
    		*pDot = 0;
    }
    lpIndex->FileInIndex++; 
Exit:
	pCurFileIndexEntry->Len = lpIndex->CurrentEntry->Len;
	pCurFileIndexEntry->BMWidth = lpIndex->CurrentEntry->BMWidth;
	pCurFileIndexEntry->BMHeight = lpIndex->CurrentEntry->BMHeight;
	pCurFileIndexEntry->BMBitCount = lpIndex->CurrentEntry->BMBitCount;
	pCurFileIndexEntry->Bounds = lpIndex->CurrentEntry->Bounds;
	_fstrncpy (pCurFileIndexEntry->Name,lpIndex->CurrentEntry->Name,128);
//    _fmemmove (pCurFileIndexEntry,lpIndex->CurrentEntry,lpIndex->CurrentEntry->Len);
{
#if ENABLETRACE
GSSiExitProg (64);
#endif
    return TRUE;
}
#if ENABLETRACE
}
#endif
}

 

BOOL SetAutoPan (BOOL AP)
#if ENABLETRACE
{GSSiEnterProg (92);
#endif
{   BOOL oldAP;

    oldAP = AutoPan;
    AutoPan = AP;
{
#if ENABLETRACE
GSSiExitProg (92);
#endif
    return (oldAP);
}
#if ENABLETRACE
}
#endif
}

short PanWindow (HWND hWnd, int pandir)
#if ENABLETRACE
{GSSiEnterProg (93);
#endif
{     
	double	bmwidth, bmheight;
	double	panaz = pandir * HALFPI;
	double	pandist;

	 DoPaint = TRUE;
     bmwidth = CurView->ScreenRect.right - CurView->ScreenRect.left;
     bmheight = CurView->ScreenRect.bottom - CurView->ScreenRect.top;
	 switch (pandir)
	 {
	 case 0:
	 case 2:
		 pandist = bmwidth / 2;
		 break;
	 default:
		 pandist = bmheight / 2;
		 break;
	 }

  	 CurView->MidPointW = dnewpt (CurView->MidPointW,-CurView->Rotation+panaz,pandist*CurView->Scale);
//	 ProjectBasePt (&CurView->MidPointW);
	 
	 CurView->CurZoomAreaRef = 0;
	 ZoomToPointAndScale (CurView->MidPointW,CurView->Scale,FALSE);

{
#if ENABLETRACE
GSSiExitProg (93);
#endif
     return (0);
}
#if ENABLETRACE
}
#endif
}

POINT WinCoordToFileCoord(POINT WinPoint)
#if ENABLETRACE
{GSSiEnterProg (94);
#endif
{  
     DPOINT BasePt;


     BasePt = WinPtToBasePt (WinPoint);
{
#if ENABLETRACE
GSSiExitProg (94);
#endif
     return (BasePtToFilePt(BasePt));
}
     
#if ENABLETRACE
}
#endif
}

DPOINT WinCoordToFileCoordD(POINT WinPoint)
#if ENABLETRACE
{GSSiEnterProg (95);
#endif
{  
     DPOINT BasePt;

     BasePt = WinPtToBasePt (WinPoint);
{
#if ENABLETRACE
GSSiExitProg (95);
#endif
     return (BasePtToFilePtD(BasePt));
}
     
#if ENABLETRACE
}
#endif
}

void SetBoundsRect2 (RECT Rect, HDC hDC)
#if ENABLETRACE
{GSSiEnterProg (28);
#endif
{    
     DPOINT DPoint1, DPoint2;
     double xscale, yscale,  maxscale, XDist, YDist;
     double rect_width, rect_height;
     double bounds_width, bounds_height;
     double MidX, MidY ;
     POINT  CursorPos;

 //checkvp(1);	
     if (!Printing && !DisableHalt)
     	DoPaint = TRUE;
     CurrentRect = Rect; 
     if (CurView->NewBounds.xmx - CurView->NewBounds.xmn <P_TOL ||
      	 CurView->NewBounds.ymx - CurView->NewBounds.ymn <P_TOL)
     	 CurView->NewBounds = CurView->WBounds;
     if (CurView->NewBounds.xmx - CurView->NewBounds.xmn <P_TOL ||
      	 CurView->NewBounds.ymx - CurView->NewBounds.ymn <P_TOL)
{
#if ENABLETRACE
GSSiExitProg (28);
#endif
      	 return;
}
     MidX = (CurView->NewBounds.xmn + CurView->NewBounds.xmx) / 2;
     MidY = (CurView->NewBounds.ymn + CurView->NewBounds.ymx) / 2; 
     if (CurView->hMaskArea)
     {   
		MNMXCORD	MaskOnScreenRect;
		LPMNMXCORD	lpRect = (LPMNMXCORD) GlobalLock(CurView->hMaskArea);
		double		MinDist, XDist, YDist;
		
		if (lpRect)
		{		 
			if (IntersectBounds (&CurView->NewBounds,lpRect,&MaskOnScreenRect))
			{
				 MinDist = MaskOnScreenRect.xmn - CurView->NewBounds.xmn;
				 MinDist = min (MinDist,MaskOnScreenRect.ymn - CurView->NewBounds.ymn);
				 MinDist = min (MinDist, CurView->NewBounds.xmx - MaskOnScreenRect.xmx);
				 MinDist = min (MinDist,CurView->NewBounds.ymx - MaskOnScreenRect.ymx); 
				 XDist = max ((MaskOnScreenRect.xmx+MinDist) - MidX,MidX - (MaskOnScreenRect.xmn-MinDist));
				 YDist = max ((MaskOnScreenRect.ymx+MinDist) - MidY,MidY - (MaskOnScreenRect.ymn-MinDist));
				 CurView->NewBounds.xmn = MidX - XDist;
				 CurView->NewBounds.xmx = MidX + XDist;
				 CurView->NewBounds.ymn = MidY - YDist;
				 CurView->NewBounds.ymx = MidY + YDist;
			}
		    GlobalUnlock (CurView->hMaskArea);
		}
     }
//     MidPoint.x = (CurView->DrawRect.left + CurView->DrawRect.right) /2;
//     MidPoint.y = (CurView->DrawRect.top + CurView->DrawRect.bottom) /2;
     SetLocalProjection (&CurView->NewBounds);
     rect_width = (long)Rect.right - (long)Rect.left +1.0;//added +1 3/16/99 for peoplenet calculations
     rect_height = (long)Rect.bottom - (long)Rect.top +1.0;// ditto
     rect_width = (long)Rect.right - (long)Rect.left;//removed 5/7/07 for linked viewports and compare
     rect_height = (long)Rect.bottom - (long)Rect.top;// ditto
     bounds_width = CurView->NewBounds.xmx - CurView->NewBounds.xmn;
     bounds_height = CurView->NewBounds.ymx - CurView->NewBounds.ymn; 
     DPoint1.x = -1000;
     DPoint1.y = 0; 
     ProjectFilePtD (&DPoint1);
     DPoint2.x = 1000;
     DPoint2.y = 0; 
     ProjectFilePtD (&DPoint2);
     XDist = ldistp (DPoint1,DPoint2);
     DPoint1.y = -1000;
     DPoint1.x = 0; 
     ProjectFilePtD (&DPoint1);
     DPoint2.y = 1000;
     DPoint2.x = 0; 
     ProjectFilePtD (&DPoint2);
     YDist = ldistp (DPoint1,DPoint2);
     xscale = (XDist/YDist) * bounds_width / rect_width;
     yscale = bounds_height / rect_height;
     maxscale = max (xscale,yscale); 
     xscale = maxscale * (YDist/XDist); 
     yscale = maxscale; 
     if (! PRJ_TYPE[1] && CurView->OrthoRes < 0)
     {
     	xscale = CurView->BaseUnitsPerPixel;
     	yscale *= (XDist/YDist);
     }
     if (CurView->pVisListManual)
        CurVis = CurView->pVisListManual;
     else
     {
         CurVis = CurView->pVisList1;
         if (CurVis)
         {
             while (CurView->VisScale > CurVis->MaxScale)
                if (CurVis->NextVisList)
                    CurVis =(LPVISLIST) CurVis->NextVisList; 
                else
                    break;
         } 
     }
//	 if (!VPBoundsIsLocked ())
	 {
		 CurView->WBounds.xmn = MidX - (rect_width * xscale) / 2;
		 CurView->WBounds.xmx = MidX + (rect_width * xscale) / 2;
		 CurView->WBounds.ymn = MidY - (rect_height * yscale) / 2;
		 CurView->WBounds.ymx = MidY + (rect_height * yscale) / 2;
		 CurView->HaveBounds = TRUE; 
		 CurView->NewBounds=CurView->WBounds;
	 }
     CreateBaseToVPTran (Rect);
     {
     	POINT	WinPoint[2];
     	DPOINT	BasePoint[2];
     	double	d1,d2;
     	
   		WinPoint[0].x = CurView->Rect.left;
   		WinPoint[0].y = CurView->Rect.top;
   		WinPoint[1].x = CurView->Rect.right;
   		WinPoint[1].y = CurView->Rect.bottom;
		WinPoint[0].x = WinPoint[1].x = (CurView->Rect.left+CurView->Rect.right)/2;
   		BasePoint[0] = WinPtToBasePt (WinPoint[0]);
   		BasePoint[1] = WinPtToBasePt (WinPoint[1]);  
   		d1 = ldistp (BasePoint[0],BasePoint[1]);
   		d2 = idist (WinPoint[0],WinPoint[1]);  
   		if (!d1 || !d2)
   			CurView->BaseUnitsPerPixel = 1;
   		else	
   			CurView->BaseUnitsPerPixel = d1/d2;
   		d1 = ArcDistance(BasePoint[0],BasePoint[1]);  
   		switch (PRJ_UNITS[1])
   		{
	   		case 1:
	   			CurView->MetersPerPixel = CurView->BaseUnitsPerPixel * FTM;  
	   		break;
	   		
   			case 4:
   			
	   		if (!d1 || !d2)
	   			CurView->MetersPerPixel = CurView->BaseUnitsPerPixel;
	   		else
	   		{	
	   			DPOINT	worp[2];
	   			POINT	winp[2];  
	   			double	wind,word;
	   			
	   			worp[0].x = CurView->WBounds.xmn;
	   			worp[0].y = (CurView->WBounds.ymn + CurView->WBounds.ymn)/2;
	   			worp[1].x = CurView->WBounds.xmx;
	   			worp[1].y = worp[0].y;
	   			word = ArcDistance(worp[0],worp[1]); 
	   			winp[0].x = CurView->DrawRect.left;
	   			winp[0].y = (CurView->DrawRect.top + CurView->DrawRect.top)/2;
	   			winp[1].x = CurView->DrawRect.right;
	   			winp[1].y = winp[0].y; 
	   			wind = idist (winp[0],winp[1]);
	   			CurView->MetersPerPixel = d1/d2;
	   			CurView->MetersPerPixel = word/wind;   
	   		}
	   		break;
	   		
	   		default:
	   			CurView->MetersPerPixel = CurView->BaseUnitsPerPixel;
	   		break;
	   	}	
	 
   	 }
     if (!(Printing && !PrintMerging) || !CurView->VisScale)
        CurView->VisScale = CurView->Scale = CurView->BaseUnitsPerPixel;
 //checkvp(1);	
	 AdjustWBoundsToTile ();  
 //checkvp(1);	
     if (!Printing && !PrintMerging)
     {
     	GetCursorPos (&CursorPos);
     	SetCursorPos (CursorPos.x,CursorPos.y);
//     	PostMessage (hWndMain,WM_MOUSEMOVE,0,MAKELPARAM (CursorPos.x,CursorPos.y));
     }

{
#if ENABLETRACE
GSSiExitProg (28);
#endif
     return;
}
#if ENABLETRACE
}
#endif
} 
     

DPOINT WinCoordToVPCoord(POINT WinPoint)
#if ENABLETRACE
{GSSiEnterProg (96);
#endif
{    float  xfactor, yfactor;
     long   rect_width, rect_height;
     double bwidth, bheight;
     short    xinc, yinc;
     RECT   Rect;
     DPOINT OutPoint;

     Rect = CurrentRect;

     rect_width = Rect.right - Rect.left;
     rect_height = Rect.bottom - Rect.top;
     if (!rect_width || !rect_height) 
     {
        OutPoint.x = 0;
        OutPoint.y = 0;
{
#if ENABLETRACE
GSSiExitProg (96);
#endif
        return (OutPoint);
}
     }
     xfactor = ((float)WinPoint.x-Rect.left)/rect_width;
     yfactor = (float)(1.0 - ((float)WinPoint.y-Rect.top)/rect_height);
     bwidth = CurView->Bounds.xmx - CurView->Bounds.xmn;
     bheight = CurView->Bounds.ymx - CurView->Bounds.ymn;
     xinc = (short) (xfactor * bwidth);
     OutPoint.x = CurView->Bounds.xmn + xinc;
     yinc = (short) (yfactor * bheight);
     OutPoint.y = CurView->Bounds.ymn + yinc;
{
#if ENABLETRACE
GSSiExitProg (96);
#endif
     return (OutPoint);
}
#if ENABLETRACE
}
#endif
}    

POINT FileCoordToWinCoord(POINT SegPoint)
#if ENABLETRACE
{GSSiEnterProg (97);
#endif
{  
     POINT  OutPoint; 
     DPOINT BasePoint;
     
     BasePoint = FilePtToBasePt(SegPoint);
     OutPoint = BasePtToWinPt (&BasePoint);

{
#if ENABLETRACE
GSSiExitProg (97);
#endif
     return (OutPoint);
}
#if ENABLETRACE
}
#endif
}
BOOL SetRezoomBounds (MNMXCORD Bounds)
#if ENABLETRACE
{GSSiEnterProg (98);
#endif
{                                     
    if (RezoomRect.xmx < RezoomRect.xmn)
    {
        RezoomRect = Bounds;
{
#if ENABLETRACE
GSSiExitProg (98);
#endif
        return TRUE;
}
    }
    else
{
#if ENABLETRACE
GSSiExitProg (98);
#endif
        return FALSE;
}
#if ENABLETRACE
}
#endif
}
void GMMessageBox (UINT Message, UINT Title, UINT Style)
#if ENABLETRACE
{GSSiEnterProg (409);
#endif
{   
	char	Msg[256], Tit[128]; 
	
	DoPaint = FALSE;
	if (!LoadString(hInst, Message, Msg, sizeof(Msg)))
		*Msg=0;
	if (!LoadString(hInst, Title, Tit, sizeof(Tit)))
		*Tit=0;
	GSSiMsgBox (GetFocus(),Msg,Tit,Style,0); 
	DoPaint = TRUE;
{
#if ENABLETRACE
GSSiExitProg (409);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}
                        
HANDLE	GetConnectedItems (BOOL Closed, LPINT pNumPoints)
#if ENABLETRACE
{GSSiEnterProg (718);
#endif
{
	HPDPOINT	pPoints,pPolyPoints[2];
	short	st, UseInt, FirstUseInt=0;
	HANDLE	hPoints;   
	DPOINT	IntPoint, FirstIntPoint;  
	long	Refno, Sequence; 
	BOOL	First=TRUE, Reverse1=FALSE, Reverse2=FALSE, LinesInt;  
	double	D[4], length[2], MinD; 
	long	npnts[2];
	HANDLE	hPoly[2]={0,0}; 
	short	type[2]; 
	double	ExtendDist = GetGlobalDVal2 ("[%EXTENDDISTANCE]",1000);
	double	extend[2];
	DPOINT	BeginPoint[2], EndPoint[2], PickedPoint;
	HIGHLIGHTDATA	HighlightData;

    if (/*!TotHLTPoints || */!hHighlight)
{
#if ENABLETRACE
GSSiExitProg (718);
#endif
    	return 0;
}
    hPoints = GSSiGlobAlloc ( 330,GMEM_MOVEABLE,(long)(TotHLTPoints+2)*sizeof(DPOINT)); 
    pPoints = (HPDPOINT)GlobalLock (hPoints);
    *pNumPoints = 0; 
	st = BT_FIND (hHighlight2,(LPSTR)&Sequence,BT_FIRST,BT_ANY,(LPSTR)&Refno);
	BT_FIND (hHighlight,(LPSTR)&Refno,BT_FIRST,BT_EQ,(LPSTR)&HighlightData); 
    type[1] = HighlightData.PD.Type; 
    length[1] = HighlightData.PD.Length; 
    BeginPoint[1] = HighlightData.PD.BeginPoint;
    EndPoint[1] = HighlightData.PD.EndPoint;
	if (!GetPolyPoints ((LPPICKDATAHEADER)&HighlightData.PD,FALSE,&npnts[1],&hPoly[1]))
		goto ErrorExit;
	if (BT_NUM_IN_INDEX (hHighlight) == 1)
	{
		GSSiGlobUlFree (&hPoints);
		*pNumPoints = npnts[1];
{
#if ENABLETRACE
GSSiExitProg (718);
#endif
    	return hPoly[1];
}
	}
	pPolyPoints[1] = (HPDPOINT)GlobalLock (hPoly[1]); 
	if (type[1] == 2) 
	{
		extend[1] = ExtendDist; 
   		ExtendPoly (npnts[1],pPolyPoints[1],ExtendDist);
   	}
   	else
   		extend[1] = 0; 
	if (!Closed)
		goto GetRemaining;
    if (Closed)
		st = BT_FIND (hHighlight2,(LPSTR)&Sequence,BT_LAST,BT_ANY,(LPSTR)&Refno);
	else
		st = BT_FIND (hHighlight2,(LPSTR)&Sequence,BT_NEXT,BT_ANY,(LPSTR)&Refno);
	BT_FIND (hHighlight,(LPSTR)&Refno,BT_FIRST,BT_EQ,(LPSTR)&HighlightData); 
	type[0] = HighlightData.PD.Type;  
	BeginPoint[0] = HighlightData.PD.BeginPoint;
	EndPoint[0] = HighlightData.PD.EndPoint;
	GetPolyPoints ((LPPICKDATAHEADER)&HighlightData.PD,FALSE,&npnts[0],&hPoly[0]);
	pPolyPoints[0] = (HPDPOINT)GlobalLock (hPoly[0]); 
	length[0] = HighlightData.PD.Length; 
	if (type[0] == 2) 
	{
		extend[0] = ExtendDist; 
	   	ExtendPoly (npnts[0],pPolyPoints[0],ExtendDist); 
	} 
	else
	   	extend[0] = 0;
	LinesInt = IntersectPolys1 (type[0],type[1],npnts[0],pPolyPoints[0],0,npnts[1],pPolyPoints[1],0,0,
								&HighlightData.PD.PickedPoint,&FirstIntPoint,&D[3],&D[0],0);
	FirstUseInt = 0;
	if (LinesInt)
	{	
		if (D[3] >= extend[0] && D[3] < extend[0] + length[0])
			FirstUseInt++;		
		if (D[0] >= extend[1] && D[0] < extend[1] + length[1])
			FirstUseInt++;		
	}
	if (Closed)
	{
		st = BT_FIND (hHighlight2,(LPSTR)&Sequence,BT_FIRST,BT_ANY,(LPSTR)&Refno);
		if (FirstUseInt)	
			pPoints[(*pNumPoints)++] = FirstIntPoint;  
		else  if (D[0] < extend[1])
			pPoints[(*pNumPoints)++] = BeginPoint[1];
		else  
			pPoints[(*pNumPoints)++] = EndPoint[1]; 
	}
	else
	{
		if ((D[3] - extend[0]) > length[0]/2)
		{
			pPoints[(*pNumPoints)++] = BeginPoint[1];
			(*pNumPoints) += GetPointsBetweenDist (npnts[1],pPolyPoints[1],extend[0],D[3],&pPoints[*pNumPoints]); 
		}
		else
		{
			pPoints[(*pNumPoints)++] = EndPoint[1]; 
			(*pNumPoints) += GetPointsBetweenDist (npnts[1],pPolyPoints[1],D[3],extend[0],&pPoints[*pNumPoints]); 
		}
	}
	GlobalUnlock (hPoly[0]);
GetRemaining:
	GlobalUnlock (hPoly[1]);

	while (!st)
	{   
		GSSiGlobFree (&hPoly[0]);
		npnts[0] = npnts[1];
		hPoly[0] = hPoly[1];
		type[0] = type[1]; 
		length[0] = length[1];  
		extend[0] = extend[1];
		BeginPoint[0] = BeginPoint[1];
		EndPoint[0] = EndPoint[1];
		st = BT_FIND (hHighlight2,(LPSTR)&Sequence,BT_NEXT,BT_ANY,(LPSTR)&Refno);
		if (!st)
		{   
			BT_FIND (hHighlight,(LPSTR)&Refno,BT_FIRST,BT_EQ,(LPSTR)&HighlightData); 
			GetPolyPoints ((LPPICKDATAHEADER)&HighlightData.PD,FALSE,&npnts[1],&hPoly[1]);
		    type[1] = HighlightData.PD.Type;  
		    length[1] = HighlightData.PD.Length; 
		    BeginPoint[1] = HighlightData.PD.BeginPoint;
		    EndPoint[1] = HighlightData.PD.EndPoint;
		    pPolyPoints[1] = (HPDPOINT)GlobalLock (hPoly[1]); 
	   		if (type[1] == 2)  
		   		ExtendPoly (npnts[1],pPolyPoints[1],ExtendDist); 
			pPolyPoints[0] = (HPDPOINT)GlobalLock (hPoly[0]); 
			if (*pNumPoints)
				PickedPoint = pPoints[(*pNumPoints)];
			else
				PickedPoint = HighlightData.PD.PickedPoint;
			LinesInt = IntersectPolys1 (type[0],type[1],npnts[0],pPolyPoints[0],0,npnts[1],pPolyPoints[1],0,0,
										&PickedPoint,&IntPoint,&D[1],&D[2],0);
			GlobalUnlock (hPoly[0]);
			GlobalUnlock (hPoly[1]);
			UseInt = 0;
			if (LinesInt)
			{	
				if (D[1] >= extend[0] && D[1] < extend[0] + length[0])
					UseInt++;		
				if (D[2] >= extend[1] && D[2] < extend[1] + length[1])
					UseInt++;		
			}
			if (First && !Closed)
			{
				if (D[1] < extend[0] + length[0]/2)
					D[0] = extend[0] + length[0];
				else
					D[0] = extend[0];
			}
		} 
		else if (Closed)
		{
			D[1] = D[3];
			IntPoint = FirstIntPoint;
			UseInt = FirstUseInt;
		}
		else
		{
			if (D[0] < extend[0] + length[0]/2)
				D[1] = extend[0] + length[0] + extend[0];
			else
				D[1] = 0;
			UseInt = 0;
		}
		First = FALSE;
		pPolyPoints[0] = (HPDPOINT)GlobalLock (hPoly[0]);
		if (D[0] < D[1])
		{
			if (D[0] <= extend[0])
				pPoints[(*pNumPoints)++] = BeginPoint[0];
		}
		else
		{
			if (D[0] >= extend[0] + length[0])
				pPoints[(*pNumPoints)++] = EndPoint[0];
		}
		(*pNumPoints) += GetPointsBetweenDist (npnts[0],pPolyPoints[0],D[0],D[1],&pPoints[*pNumPoints]); 
		GlobalUnlock (hPoly[0]);
		if (UseInt)
			pPoints[(*pNumPoints)++] = IntPoint;
		else
		{
			if (D[0] < D[1])
			{
				if (D[1] > length[0] + extend[0])
					pPoints[(*pNumPoints)++] = EndPoint[0];
			}
			else
			{
				if (D[1] < extend[0])
					pPoints[(*pNumPoints)++] = BeginPoint[0];
			} 
		}
		D[0] = D[2];
	} 
	GSSiGlobFree (&hPoly[0]);
	GlobalUnlock (hPoints);
{
#if ENABLETRACE
GSSiExitProg (718);
#endif
	return hPoints;
} 
ErrorExit:
	GSSiGlobUlFree (&hPoints);  
	GSSiGlobFree (&hPoly[0]);
	GSSiGlobFree (&hPoly[1]);
{
#if ENABLETRACE
GSSiExitProg (718);
#endif
	return hPoints;
}

#if ENABLETRACE
}
#endif
}

long GetPickItemPoints (short item, BOOL Reverse, HPDPOINT *pPoints)
#if ENABLETRACE
{GSSiEnterProg (719);
#endif
{   
	long	NumPoints=0;
	LPTHEME	pTheme; 
    
    if (SysTypeFromPickType(PickList[item].Type) == GF_CURVE)
    {
    	if (Reverse)
    	{
			*(*pPoints)++ = PickList[item].BeginPoint;
			*(*pPoints)++ = PickList[item].NodePoint;
			*(*pPoints)++ = PickList[item].EndPoint;
    	}
    	else
    	{
			*(*pPoints)++ = PickList[item].EndPoint;
			*(*pPoints)++ = PickList[item].NodePoint;
			*(*pPoints)++ = PickList[item].BeginPoint;
    	}
		NumPoints = 3;
    }
	else if (PickList[item].NumPoints > 2)
	{
		SetConfig (PickList[item].ConfigID);
	    SetViewport (PickList[item].ViewID);
		pTheme = AddTheme (GF_SAVEPOLY_THEME);
		CurView->PassID = 4;
		ProcessSelectedTheme = CurView->NumThemes;
		ProcessPickedItem (item,FALSE);        		
		ProcessSelectedTheme = 0;
		DeleteTheme (pTheme);
    	while (GetSavedPolys ())
   		if (hSavePoly)
		{   LPMNMXCORD lpRect;
			LPDPOINT	lpDpoint;  
			long		nPnts;
    		    
            NumPoints = nPnts = nSavePoly; 
            lpRect = (LPMNMXCORD) GlobalLock (hSavePoly);
            lpRect++;
            lpDpoint = (LPDPOINT) lpRect;   
            if (!Reverse)
            {
	            while (nPnts--)
	            {
					**pPoints = *lpDpoint++;
					(*pPoints)++; 
	            }
	        }
	        else 
	        {   
	        	lpDpoint+=(nPnts-1);
	            while (nPnts--)
	            {
					**pPoints = *lpDpoint--;
					(*pPoints)++; 
	            }
	        }
		  	GSSiGlobUlFree (&hSavePoly);
         } 
	}
	else if (Reverse)
	{
		*(*pPoints)++ = PickList[item].EndPoint;
		*(*pPoints)++ = PickList[item].BeginPoint;
		NumPoints = 2;
	}
	else
	{
		*(*pPoints)++ = PickList[item].BeginPoint;
		*(*pPoints)++ = PickList[item].EndPoint;
		NumPoints = 2;
	} 
{
#if ENABLETRACE
GSSiExitProg (719);
#endif
	return NumPoints;
}
#if ENABLETRACE
}
#endif
}

BOOL AddStreetNumber (HPSTR buf,LPLONG len)
{   
	static	int	HaveState=-2;
	static	HANDLE	hBT=0;
	static	HFILE	hData=0;
	HPSHORT	ipnt;
	long	Refno, Offset;
	LPLONG	pStreetNum, pRefno;
	long	remlen, lRest;   
	short	i;
	LPBYTE	Pcode; 
	HPSTR	EndLoc, EndRefno;
	LPSHORT	pItemLen, StreetPnt; 
	BOOL	HaveStreet=FALSE;	
	LPGWDHEADER	lpGWDHead;
    LPSEGDATAGM	pSegdata; 
	
	if (!AddStreetNums)
		return FALSE;
/*	if (CurState <0)
		retrn FALSE;
	if (HaveState != CurState)
	{   
		char	str[8]; 
		OFSTRUCTGM	OFStruct;
				
		if (hData)
			GSSiClose (hData); 
		hData = 0;
		BT_CLOSE (hBT);
		hBT = BT_OPEN ("[STATE]\\tiger1.btr", 0, BT_READ, 0);
		hData = GSSiOpenFile ("[STATE]\\tiger1.dat",&OFStruct,OF_READ);
		HaveState = CurState;
	} */
    EndLoc = buf+*len;
    ipnt = (HPSHORT)EndLoc;
    *ipnt = 0;
    ipnt = (HPSHORT)buf;  
    Pcode = (LPBYTE)ipnt;
    if (*Pcode != 12)
    	return FALSE;
    pItemLen = ipnt;
    pItemLen += 5;
    pRefno = 0;
    while (*ipnt != 0)
    {   Pcode = (LPBYTE)ipnt;
    	ipnt++;

	    switch (*Pcode)
        {   
            case 9: 
            {   
            	int	ltag;
                	
            	pRefno =(LPLONG) ipnt; 
            	Refno = *pRefno;
            	ipnt += 2; 
            	ltag = *++Pcode;
            	if (ltag) 
            	{    
					ipnt = (LPSHORT) ((LPSTR)ipnt + ltag + ltag%2); 
             	} 
             	EndRefno = (LPSTR)ipnt;

            }
            break;   
            
            case 10:  
            {
            	HaveStreet = TRUE;  
            	StreetPnt = ipnt;
            	ipnt += 8;
            }
            break;

            default: 
				SkipSubRec (Pcode,&ipnt,0);
            break;

        } 
    }

	if (!pRefno)
		return FALSE;
		
	lpGWDHead = (LPGWDHEADER)GlobalLock (hDBStreetSegments); 
	pSegdata = (LPSEGDATAGM)&lpGWDHead->GWDData;
    if (!BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&Refno,BT_FIRST,BT_EQ,(LPSTR)&Offset))
    {   
		FillGWDData (lpGWDHead,Offset);  
		if (HaveStreet)
			pStreetNum = (LPLONG)StreetPnt;
		else 
		{
			lRest = *len - (long)(EndRefno-buf);
			hmemmove (EndRefno+18,EndRefno,lRest);
			ipnt = (HPSHORT)EndRefno;
			*ipnt = 10;
			ipnt++; 
			pStreetNum=(LPLONG)ipnt;  
		}
		for (i=0;i<4;i++,pStreetNum++)
			*pStreetNum = pSegdata->StreetNum[i];
		if (!HaveStreet) 
		{
			(*len)+=18;
			if (*pItemLen>0) 
				(*pItemLen)+=9;
			else if (*pItemLen<0)
				(*pItemLen)-=9; 
		} 
		GlobalUnlock (hDBStreetSegments);
		return TRUE; 
	}  
	GlobalUnlock (hDBStreetSegments);
	return FALSE;
}  

BOOL ConvertTAG (HPSTR buf,LPLONG len)
{   
	static	int	HaveState=-2;
	static	HANDLE	hBT=0;
	static	HFILE	hData=0;
	HPSHORT	ipnt;
	long	Refno, Offset;
	LPLONG	pStreetNum, pRefno;
	long	remlen, lRest;   
	short	i;
   	int	ltag, OldLen, NewLen, DiffLen;
	LPBYTE	Pcode, pltag; 
	HPSTR	EndLoc, EndRefno, lpTAG;
	LPSHORT	pItemLen, StreetPnt; 
	BOOL	HaveStreet=FALSE;
	char	str[256];
	LPSTR	pMacro;	
	
	if (!ConvertTAGValues)
		return FALSE;
    EndLoc = buf+*len;
    ipnt = (HPSHORT)EndLoc;
    *ipnt = 0;
    ipnt = (HPSHORT)buf;  
    Pcode = (LPBYTE)ipnt;
    if (*Pcode != 12)
    	return FALSE;
    pItemLen = ipnt;
    pItemLen += 5;
    pRefno = 0;
    while (*ipnt != 0)
    {   Pcode = (LPBYTE)ipnt;
    	ipnt++;

	    switch (*Pcode)
        {   
            case 9: 
            {   
            	pRefno =(LPLONG) ipnt; 
            	Refno = *pRefno;
            	ipnt += 2; 
            	lpTAG = (LPSTR)ipnt;
            	pltag = ++Pcode;  
            	ltag = *pltag; 
            	OldLen = ltag + ltag%2;
				ipnt = (LPSHORT) ((LPSTR)ipnt + OldLen); 
             	EndRefno = (LPSTR)ipnt;
	        	if (ltag) 
	        	{
	        		HPSTR	lpColon = _fstrchr (lpTAG,':');   
	        		short	len;
	        		
	        		if (*lpTAG == '"' || !lpColon || !_fstricmp (lpTAG,"REFNO"))
	        			goto TAGIsRefno;
	        		*lpColon = 0;
					SetGlobalValue2 (hPrefix,lpTAG,0); 
					strncpy0(CurrentPrefix,lpTAG,MAX_PREFIX_LEN);
					len = ltag - (lpColon - lpTAG + 1);
					SetUDIValueLen (lpTAG,++lpColon,len); 
					CurrentUDILen = len;
					strncpy0 (CurrentUDI,lpColon,MAX_UDI_LEN);
	             	if (lpColon)
	             		*(--lpColon) = ':';   
	        	}
	        	else
	        	{   
	TAGIsRefno:		_fstrcpy(CurrentPrefix,"REFNO");
					SetGlobalValue2 (hPrefix,"REFNO",0);
					if (SetRefno)  
						sprintf (str,"%.2f",(((double)CurrentRefno) - (-2146450000))/100); 
					else
						*str = 0;
					SetGlobalValue2 (hUDI,str,0);
					CurrentUDILen = _fstrlen(str);
					strncpy0 (CurrentUDI,str,MAX_UDI_LEN); 
	         	}  
	         	goto GotTAG;
            }
            break;   
            
            default: 
				SkipSubRec (Pcode,&ipnt,0);
            break;

        } 
    }
GotTAG:
	if (!pRefno)
		return FALSE;

	lRest = *len - (long)(EndRefno-buf);    
	pMacro = GlobalLock (hConvertTAGMacro);
	_fstrcpy (str,pMacro);
	GlobalUnlock (hConvertTAGMacro);
	ExpandText (str);  
	NewLen = _fstrlen (str);
	if (!NewLen)
		return FALSE;
	*pltag = NewLen;
	NewLen += NewLen%2;  
	DiffLen = NewLen - OldLen;  
	*len += DiffLen;
	if (DiffLen)
	{
		HANDLE	hRest=GSSiGlobAlloc (0,GMEM_MOVEABLE,lRest);
		HPSTR	pRest = GlobalLock (hRest);
		
		hmemmove (pRest,EndRefno,lRest);
		hmemmove (EndRefno+DiffLen,pRest,lRest);
		GSSiGlobUlFree (&hRest);
	}    
	hmemmove (lpTAG,str,(long)*pltag);
	if (*pItemLen>0) 
		(*pItemLen)+=DiffLen/2;
	else if (*pItemLen<0)
		(*pItemLen)-=DiffLen/2; 
	return TRUE; 
}  



