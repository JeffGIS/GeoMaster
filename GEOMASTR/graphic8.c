#include "graphint.h" 

#include "gmextern.h"

static BOOL	ReducePolyPnts=FALSE;
static	HCURSOR	SpecCursors[32];
static	UINT	nSpecialCursors=0;
static	char	CurWinText[144];
static messageCanceled = FALSE;

double	ConvertRotation (double Rot,LPDPOINT Point,int From,int To)
#if ENABLETRACE
{GSSiEnterProg (925);
#endif
{   
	double	NewRot;  
	DPOINT	NewPoint[2]; 
	double	NewAZ;
	
	NewPoint[0] = *Point;
	NewPoint[0].x -=1;
	NewPoint[1] = *Point;
	NewPoint[1].x +=1;
	
	ConvertCoord(&NewPoint[0],From,To);
	ConvertCoord(&NewPoint[1],From,To);
    NewAZ = getazd (&NewPoint[0],&NewPoint[1]);
	
{
#if ENABLETRACE
GSSiExitProg (925);
#endif
	return LTWOPI (Rot+NewAZ);
}
#if ENABLETRACE
}
#endif
}

double MinAngleToTheRight (double AZ1,double AZ2)
#if ENABLETRACE
{GSSiEnterProg (926);
#endif
{
	double AZDiff;
	
	if (AZ1 < AZ2)
		AZDiff = AZ2 - AZ1;
	else
		AZDiff = AZ2 + (TWOPI - AZ1);
{
#if ENABLETRACE
GSSiExitProg (926);
#endif
	return AZDiff;
}
#if ENABLETRACE
}
#endif
}

int	SaveCurView (int opt)
#if ENABLETRACE
{GSSiEnterProg (927);
#endif
{
	static	LPVIEWPORT	SaveVP;
	
	if (opt)
		SetCurView ( SaveVP);
	else
		SaveVP = CurView;
{
#if ENABLETRACE
GSSiExitProg (927);
#endif
	return 0;
}
#if ENABLETRACE
}
#endif
}
HANDLE CreateElipse (DPOINT CenterPt,double AZM,double axis1, double axis2,LPLONG pnPnts)
#if ENABLETRACE
{GSSiEnterProg (928);
#endif
{
	double	d=FileDistToBaseDist/2; 
	double	DeltaAz, AZM2=0, Radius=(axis1+axis2)/4, diff=(axis2/2-axis1/2)/2,dist;
	UINT	i;
	HANDLE	handle;
	HPDPOINT	pPoint;
	
	DeltaAz = asin (d/Radius); 
	DeltaAz = max (DeltaAz,TWOPI/16000);
	*pnPnts = TWOPI / DeltaAz;
	handle = GSSiGlobAlloc ( 749,GMEM_MOVEABLE,(long)sizeof(DPOINT)*(long)*pnPnts); 
	pPoint = (HPDPOINT)GlobalLock (handle);
	for (i=0;i<*pnPnts;i++)
	{   
		dist = axis1/2 + diff * fabs(sin (AZM2));
		*pPoint++ = dnewpt (CenterPt,LTWOPI(AZM),dist);
		AZM += DeltaAz; 
		AZM2 += DeltaAz;
	} 
	GlobalUnlock (handle);
{
#if ENABLETRACE
GSSiExitProg (928);
#endif
	return handle;
}
#if ENABLETRACE
}
#endif
}  

void GSSiMessageBoxEnable(void)
{
	messageCanceled = FALSE;
}
short GSSiMessageBox (int from,LPSTR Mess,LPSTR Title,UINT icon,LPSTR Position)
#if ENABLETRACE
{GSSiEnterProg (929);
#endif
{   
	short	rtn=0;  
	BOOL	SaveDoPaint=DoPaint(), SaveHBW=HaveBlockingWindow;
	HANDLE	hMem = GSSiGlobAlloc ( 750,GMEM_MOVEABLE,4096);
	LPSTR	pMess = GlobalLock (hMem);
	LPSTR	pTitle = pMess + 2048;
	
	HaveBlockingWindow = TRUE;
	setDoPaint( FALSE);
	if (Mess)
	{ 
		_fstrcpy (pMess,Mess);
		//ExpandText (pMess);
	} 
	else
		pMess = 0;
	if (Title)
	{
		_fstrcpy (pTitle,Title);
		ExpandText (pTitle);
	}
	else
		pTitle = 0;
	if (!messageCanceled || !from)
	{
		if (from > 0)
		{
			if (icon == MB_YESNO)
				icon = MB_YESNOCANCEL;
			if (icon == MB_OK)
				icon = MB_OKCANCEL;
		}
		rtn = GSSiMsgBox(hWndMain, pMess, pTitle, icon, Position);
		if (rtn == IDCANCEL)
		{
			messageCanceled = TRUE;
		}
	}
	GSSiGlobUlFree (&hMem);
	setDoPaint( SaveDoPaint);   
	HaveBlockingWindow = SaveHBW;
{
#if ENABLETRACE
GSSiExitProg (929);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}  

HANDLE CreateCirclePoly (DPOINT CenterPt,double Radius,LPLONG pnPnts,double VectorizationFactor)
#if ENABLETRACE
{GSSiEnterProg (930);
#endif
{

	double	DeltaAz, AZM=0;  
	long	np;
	UINT	i;
	HANDLE	handle;
	HPDPOINT	pPoint;
	
	if (!VectorizationFactor)
		VectorizationFactor = BaseDistToWinDist;
	else if (VectorizationFactor < 0)
		VectorizationFactor = -BaseDistToWinDist/VectorizationFactor;   
	np = (TWOPI * Radius) * VectorizationFactor + 2;
	if (np > 1600)
		np = 1600;
	DeltaAz = TWOPI / np;
	np++;
	*pnPnts = np;
	handle = GSSiGlobAlloc ( 751,GMEM_MOVEABLE,(long)sizeof(DPOINT)*np); 
	pPoint = (HPDPOINT)GlobalLock (handle); 
	for (i=0;i<*pnPnts;i++)
	{
		*pPoint++ = dnewpt (CenterPt,AZM,Radius);
		AZM += DeltaAz;
	} 
	GlobalUnlock (handle);
{
#if ENABLETRACE
GSSiExitProg (930);
#endif
	return handle;
}
#if ENABLETRACE
}
#endif
} 

 

 

void ClearSnapList (void)
#if ENABLETRACE
{GSSiEnterProg (932);
#endif
{   
	nInSnapList = 0;
{
#if ENABLETRACE
GSSiExitProg (932);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}	

BOOL AddToSnappedList (short Item)
#if ENABLETRACE
{GSSiEnterProg (933);
#endif
{   
	switch (nInSnapList)
	{ 
		case 2:
			SnapList[0] = SnapList[1]; 
			nInSnapList--;
		case 0:  
		case 1:
			SnapList[nInSnapList++] = PickList[Item];
		break;
		
	}
{
#if ENABLETRACE
GSSiExitProg (933);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

 

 



 

HANDLE CreateWinPoints (HANDLE hDPoints,LPLONG pnPnts,LPRECT pRect)
#if ENABLETRACE
{GSSiEnterProg (940);
#endif
{ 
	HANDLE handle = GSSiGlobAlloc ( 753,GMEM_MOVEABLE,*pnPnts*sizeof(POINT));
	HPPOINT	pPoints=(HPPOINT)GlobalLock (handle);
	HPDPOINT	pDPoints=(HPDPOINT)GlobalLock (hDPoints);
	DWORD	n=*pnPnts;
	
	while (n--)
	{
		*pPoints = BasePtToWinPt (pDPoints++);
		if (pRect)
			AddPointToRect (*pPoints,pRect);
		pPoints++;
	}
	GlobalUnlock (hDPoints);
	GlobalUnlock (handle);
{
#if ENABLETRACE
GSSiExitProg (940);
#endif
	return handle;
}
#if ENABLETRACE
}
#endif
}

short TranFileBounds (short InOut,LPMNMXCORD pBounds)
#if ENABLETRACE
{GSSiEnterProg (941);
#endif
{   
	HANDLE	TranID=CurView->hFileTransIn;
	short	i;
	
	switch ((int)TranID)
	{
		case 0:
{
#if ENABLETRACE
GSSiExitProg (941);
#endif
			return 0;
}
		case 1: //file exists but has insufficient points
{
#if ENABLETRACE
GSSiExitProg (941);
#endif
			return 2;
}
		default:  
			if (InOut == 2)
				TranID=CurView->hFileTransOut;
			TranBounds (TranID,pBounds); 
{
#if ENABLETRACE
GSSiExitProg (941);
#endif
    		return 1;
}
    }
    
#if ENABLETRACE
}
#endif
}

short TranFilePoint (short InOut,LPDPOINT pDPoint)
#if ENABLETRACE
{GSSiEnterProg (942);
#endif
{   
	switch ((int)CurView->hFileTransIn)
	{
		case 0:
{
#if ENABLETRACE
GSSiExitProg (942);
#endif
			return 0;
}
		case 1: //file exists but has insufficient points
{
#if ENABLETRACE
GSSiExitProg (942);
#endif
			return 2;
}
		default:
			if (InOut == 1)
	    		TRANS2 (pDPoint->x,pDPoint->y,&pDPoint->x,&pDPoint->y,CurView->hFileTransIn);
	    	else
	    		TRANS2 (pDPoint->x,pDPoint->y,&pDPoint->x,&pDPoint->y,CurView->hFileTransOut);
{
#if ENABLETRACE
GSSiExitProg (942);
#endif
    		return 1;
}
    }
    
#if ENABLETRACE
}
#endif
}

BOOL UpdateOrthoIndexWrite (LPSTR Name, double AdjustX, double AdjustY)
#if ENABLETRACE
{GSSiEnterProg (943);
#endif
{
    OFSTRUCTGM    OFStruct; 
    HFILE   FidIndex=0;
    long    EndOffset, NextHeaderOffset, WriteLoc, SaveLoc;
    HANDLE  Handle;
    LPFILEINDEX lpIndex;
    FILEINDEX CurIndex;   
    MNMXCORD    TestBounds;
	MNMXCORD	FileBounds, NewBounds;
    
    FidIndex = GSSiOpenFile (Name,(LPOFSTRUCTGM) &OFStruct,OF_READWRITE);
    if (FidIndex == HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (943);
#endif
        return FALSE;                                                
}
    EndOffset = GSSillseek(FidIndex,(LONG)-(6),2); 
    GSSillseek (FidIndex,0,0);
    BigRead (FidIndex,(HPSTR)&FileBounds,sizeof(MNMXCORD));
    DBoundsInit (&NewBounds);
NextIndex:
	WriteLoc = GSSillseek (FidIndex,0,1); 
    BigRead (FidIndex, (HPSTR)&CurIndex,STOREDINDEXLENGTH);
    Handle = GSSiGlobAlloc ( 754,GMEM_MOVEABLE,sizeof(FILEINDEX)+CurIndex.Length);
    lpIndex = (LPFILEINDEX)GlobalLock(Handle);
    *lpIndex = CurIndex;
    lpIndex->FirstIndexFileOffset = GSSillseek (FidIndex,0,1);
    lpIndex->FileInIndex=0; 
    lpIndex->CurrentEntryOffset=-1;
    BigRead (FidIndex,&lpIndex->FirstIndex,lpIndex->Length);
    NextHeaderOffset = GSSillseek (FidIndex,0,1);     
    DBoundsInit (&CurIndex.Bounds);
    while (GetNextIndexEntry(lpIndex))

/*    {   
        char    TestName[128];
        
        if (*CurEntryName == '.')
        	_fstrcpy (TestName,CurrentOrthoOrigName);
        else
       		_fstrcpy(TestName,CurEntryName); 
        if (!_fstrchr (CurView->OrthoDisplayName,':'))
        {
            LPSTR   lpColon;
            
            lpColon = _fstrchr (TestName,':');
            if (lpColon)
                *lpColon = 0; 
            else
            {
                lpColon = _fstrrchr (TestName,'.');
                if (lpColon)
                    *lpColon = 0; 
            }
        }
        if (_fstricmp(CurView->OrthoDisplayName,TestName))
            goto Next;
    } */


    {    
    	char	TestName[128];
    	LPSTR	pAT;
    	
        if (*lpIndex->CurrentEntry->Name == '.')
        	_fstrcpy (TestName,CurrentOrthoOrigName);
        else
    		_fstrcpy (TestName,lpIndex->CurrentEntry->Name);
    	TruncateAt (TestName,"(.:@");
    	if (!_fstricmp (PickedOrthoName,TestName))
    	{   
    		if (AdjustX < DBL_MAX)
    			AdjustBounds (&lpIndex->CurrentEntry->Bounds,AdjustX,AdjustY);
    		else if ((pAT = _fstrchr (lpIndex->CurrentEntry->Name,'@')))
    		{ 
    			_fstrcpy (pAT,"@-1");
    		}
    	} 
    	AddMinMaxD (&CurIndex.Bounds,&lpIndex->CurrentEntry->Bounds);
    } 
    AddMinMaxD (&NewBounds,&CurIndex.Bounds);
    SaveLoc = GSSillseek (FidIndex,0,1); 
    GSSillseek (FidIndex,WriteLoc,0);
    BigWrite (FidIndex, (HPSTR)&CurIndex,STOREDINDEXLENGTH,-1);
    BigWrite (FidIndex,(HPSTR)&lpIndex->FirstIndex,lpIndex->Length,-1);
    GSSillseek (FidIndex,SaveLoc,0);
    GSSiGlobUlFree (&Handle);
	if (NextHeaderOffset < EndOffset)
		goto NextIndex; 
	GSSillseek (FidIndex,0,0);
	BigWrite (FidIndex,(HPSTR)&NewBounds,sizeof(MNMXCORD),-1);
    GSSiClose (FidIndex);  

    
{
#if ENABLETRACE
GSSiExitProg (943);
#endif
    return TRUE;
}
#if ENABLETRACE
}
#endif
} 

BOOL ConvertSymName (LPSTR CurrentSymName,int from,BOOL Parent, int isym)
#if ENABLETRACE
{GSSiEnterProg (944);
#endif
{   
	static	BOOL	First=TRUE; 
	static	HANDLE	hConvertSym=0;
	char	FromName[10], NewName[34]; 
	
	if (!CurrentSymName)
	{
		First=TRUE; 
		BT_CLOSEANDDELETE (&hConvertSym);     
{
#if ENABLETRACE
GSSiExitProg (944);
#endif
		return TRUE;
}
	} 
	if (from && VisListOpt)
	{
		HANDLE	hMem=GSSiGlobAlloc ( 755,GMEM_MOVEABLE,256);
		LPSTR	pDesc=GlobalLock (hMem);    
		*pDesc=0;
		if (!Parent)
		{
			if (isym)
				GetDictSymDescription (isym,pDesc);
			sprintf (_fstrchr (CurrentSymName,0),"\t%s",pDesc);
		}
		GSSiGlobUlFree (&hMem);
{
#if ENABLETRACE
GSSiExitProg (944);
#endif
		return TRUE;
}
	}
//	if (_fstrlen (CurrentSymName) > 8)
{
#if ENABLETRACE
GSSiExitProg (944);
#endif
		return FALSE;
}
/*	if (First)
	{   
		OFSTRUCTGM	OFStruct;
		HFILE		Fid;  
		char		Name[144], str[52], str2[34];
		BTVARDESC	BTVar[2];
		
		First=FALSE;
		Fid = GSSiOpenFile ("[%DL]symconv.txt",&OFStruct,OF_READ);
		if (Fid == HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (944);
#endif
			return FALSE;
}
		GSSiGetTempFileName (0,"gm",0,(LPSTR)Name);
	
		BTVar[0].BT_VARTYP=BT_CHAR;
		BTVar[0].BT_VARLEN=8;
		BTVar[0].BT_VAROFF=0;
		BT_CREATE (Name, 32, FALSE, 1, 1, (LPBTVARDESC)BTVar, FALSE, 0, 0, FALSE);
		hConvertSym = BT_OPEN (Name, 0, BT_WRITE, 0); 
		while (fgetstring (str,48,Fid))
		{
			Truncate (str);
			_fstrncpy (str2,str,8);
			str2[8]=0;
			Truncate (str2);
			_fstrncpy (FromName,str2,8);  
			BT_PUT (hConvertSym,FromName,&str[9]);
		}
	    BT_CLOSE (hConvertSym);
		GSSiClose (Fid);
		hConvertSym = BT_OPEN (Name, 0, BT_READ, 0);
	}               
	if (hConvertSym)
	{   
		_fstrncpy (FromName,CurrentSymName,8);
		if (!BT_FIND (hConvertSym,FromName,BT_FIRST,BT_EQ,NewName))
		{
			_fstrcpy (CurrentSymName,NewName);
{
#if ENABLETRACE
GSSiExitProg (944);
#endif
			return TRUE;
}
		}
	}
{
#if ENABLETRACE
GSSiExitProg (944);
#endif
	return FALSE;
}*/
#if ENABLETRACE
}
#endif
}

BOOL SelectAreaToOffsetFile (int Item,double Offset,HANDLE hTran)
#if ENABLETRACE
{GSSiEnterProg (945);
#endif
{
 LPTHEME	pTheme;
 int		nPnts;
 LPDPOINT	lpDpoint;
 LPINT		pPolyParts=0;

    if (Item < 0 || Item >=MAXPICKITEMS) 
    	return FALSE;
	if (PickList[Item].Type == 2 || PickList[Item].Type == 3)
	{ 
		HANDLE hPoly=0;
		HANDLE hPolyPartLen=0;
 		int nLoops = GetPolyPointsWithParts ((LPPICKDATAHEADER)&PickList[0],&nPnts,&hPoly,&hPolyPartLen);

		if (nLoops)
		{   
			LPMNMXCORD	pBounds = (LPMNMXCORD)GlobalLock (hPoly);
			
			lpDpoint = (HPDPOINT)(pBounds+1);
			if (hTran)
			{
				int i;
				for (i = 0; i<nPnts; i++)
					lpDpoint[i] = TranPoint(&lpDpoint[i], hTran);
			}
			if (nLoops > 1)
			{
				pPolyParts = GlobalLock(hPolyPartLen);
				pPolyParts++;//first element is npoly
			}
			AddAreaToOffsetFile (PickList[Item].Refno,PickList[Item].Type,nPnts, lpDpoint,nLoops,pPolyParts,Offset);
			if (nLoops > 1)
				GlobalUnlock(hPolyPartLen);
		}
		GSSiGlobUlFree(&hPoly);
		GSSiGlobFree(&hPolyPartLen);

		
/*		SetConfig (PickList[Item].ConfigID);
	    SetViewport (PickList[Item].ViewID);
		pTheme = AddTheme (GF_SAVEPOLY_THEME);
		CurView->PassID = 4; 
		ProcessSelectedTheme = CurView->NumThemes;
		ProcessPickedItem (Item,FALSE); 
		ProcessSelectedTheme = 0;       		
		DeleteTheme (pTheme);
   		while (GetSavedPolys ())
		if (hSavePoly)
		{
			LPMNMXCORD lpRect;
	    	int	nareas;
			
	        nPnts = nSavePoly; 
            if (hSavePolyParts)
            {
            	pPolyParts = (LPINT)GlobalLock (hSavePolyParts);
            	nareas = *pPolyParts++; 
            }
            else
            { 
                nareas=1;
                pPolyParts = 0;
            }
	        lpRect = (LPMNMXCORD) GlobalLock (hSavePoly);
	        lpRect++;
	        lpDpoint = (LPDPOINT) lpRect;
			AddAreaToOffsetFile (PickList[Item].Refno,PickList[Item].Type,nPnts, lpDpoint,nareas,pPolyParts,Offset);
			GSSiGlobUlFree (&hSavePoly);
			
		} */
{
#if ENABLETRACE
GSSiExitProg (945);
#endif
		return TRUE;
}
	}
{
#if ENABLETRACE
GSSiExitProg (945);
#endif
	return FALSE;
}
#if ENABLETRACE
}
#endif
} 

BOOL PolyInMaskAreaFileCoord (short Type,LPLONG pnPnts,LPHANDLE phCoords,HPPOINTS *plpPolyPoints,HPDPOINT *plpDPolyPoints,BOOL HiPrecis)
#if ENABLETRACE
{GSSiEnterProg (946);
#endif
{
	LPMNMXCORD	lpRect; 
	MNMXCORD	AreaBounds;
	HPDPOINT lpDpoints, pPolyPointsD; 
	BOOL	rtn=FALSE;
	DPOINT	DPoint;
	HANDLE	Handle=0; 
	UINT	i,nIn;
	short	nInt; 
	HANDLE	hMaskArea= CurView->hMaskArea;     
	USHORT	NumMaskPoints=CurView->NumMaskPoints; 
	LPHANDLE	phMaskAccelerator=&CurView->hMaskAccelerator[0];
    
    if (CurView->DisplayInParent && CurView->Parent>0)  
    {
    	hMaskArea = pViewports[CurView->Parent-1]->hMaskArea; 
    	NumMaskPoints=pViewports[CurView->Parent-1]->NumMaskPoints; 
    	phMaskAccelerator=&pViewports[CurView->Parent-1]->hMaskAccelerator[0];  
    }
	if (!hMaskArea || !MaskOffsetLine || IgnoreBounds)
{
#if ENABLETRACE
GSSiExitProg (946);
#endif
    	return TRUE;
}
	if (!ComputePCTTheme && (IgnoreBounds || InclusionOpt == 3 || BleedThrough))
{
#if ENABLETRACE
GSSiExitProg (946);
#endif
		return TRUE; 
}   
	DBoundsInit (&AreaBounds);
	if (HiPrecis)
	{
		pPolyPointsD = *plpDPolyPoints;
		for (i=0;i<*pnPnts;i++,pPolyPointsD++)
		{   
			AddDPointToMinMax (pPolyPointsD,&AreaBounds);
		}
		pPolyPointsD = *plpDPolyPoints;
	}
	else
	{
		HPPOINTS	pPolyPoints=*plpPolyPoints;
		
	    Handle = GSSiGlobAlloc ( 756,GMEM_MOVEABLE,(long)*pnPnts*sizeof(DPOINT));
	 	pPolyPointsD = (HPDPOINT) GlobalLock (Handle); 
		for (i=0;i<*pnPnts;i++,pPolyPoints++,pPolyPointsD++)
		{   
			*pPolyPointsD = FilePtToBasePt (POINTStoPOINT(*pPolyPoints));   
			AddDPointToMinMax (pPolyPointsD,&AreaBounds);
		}
		GlobalUnlock (Handle);
	 	pPolyPointsD = (HPDPOINT) GlobalLock (Handle);
	}
	lpRect = (LPMNMXCORD) GlobalLock (hMaskArea); 
	if (BoundsInBounds (&AreaBounds,lpRect,1))
	{   
	    lpRect++;
	    lpDpoints = (LPDPOINT) lpRect; 
	    { 
			DPOINT	BoundsPoints[4];
			double	AreaAZ[4];
		
			BoundsToPoints (&AreaBounds,BoundsPoints,AreaAZ); 
	 		if (PolyInArea (Type,4,(HPDPOINT)&BoundsPoints,AreaAZ,NumMaskPoints,lpDpoints,1,0,1,phMaskAccelerator))
	 			rtn = TRUE;
	 		else if (PolyInArea (Type,4,(HPDPOINT)&BoundsPoints,AreaAZ,NumMaskPoints,lpDpoints,1,0,-1,phMaskAccelerator))
	 			rtn = FALSE;
	 		else
		 		rtn = PolyInArea (Type,*pnPnts,pPolyPointsD,0,NumMaskPoints,lpDpoints,1,0,InclusionOpt,phMaskAccelerator);
	 	}
	}   
	GlobalUnlock (hMaskArea); 
	GSSiGlobUlFree (&Handle);
{
#if ENABLETRACE
GSSiExitProg (946);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}

BOOL PointInMaskAreaWinCoord (POINT Point)
#if ENABLETRACE
{GSSiEnterProg (947);
#endif
{
	LPMNMXCORD	lpRect; 
	HPDPOINT lpDpoints; 
	POINT	BadPointWin;
	BOOL	rtn;
	DPOINT	DPoint, BadPoint;   
	HANDLE	hMaskArea= CurView->hMaskArea; 
	LPHANDLE	pMaskAccelerator=&CurView->hMaskAccelerator[0];    
	USHORT	NumMaskPoints=CurView->NumMaskPoints;
    BOOL	PlotArray=FALSE;

    if (CurView->DisplayInParent && CurView->Parent>0)  
    {
    	hMaskArea = pViewports[CurView->Parent-1]->hMaskArea;
    	pMaskAccelerator=&pViewports[CurView->Parent-1]->hMaskAccelerator[0];  
    	NumMaskPoints=pViewports[CurView->Parent-1]->NumMaskPoints;   
    }
	if (!hMaskArea || !MaskOffsetLine || IgnoreBounds)
{
#if ENABLETRACE
GSSiExitProg (947);
#endif
		return TRUE;
}
	lpRect = (LPMNMXCORD) GlobalLock (hMaskArea);
    lpRect++;
    lpDpoints = (LPDPOINT) lpRect;  
	DPoint = WinPtToBasePt (Point);

	if (PlotArray)
	{
		int row, col;
		POINT	p;
		DPOINT	DPoint;
		MNMXCORD	Bounds;
		RECT	Rect;

		SaveDC (CurView->hDC);
	    SetDisplayMode (CurView->hDC, GF_TEXTMODE);
		GetPolyBoundsD2 (lpDpoints,NumMaskPoints,&Bounds,TYPE_AREA);
		BoundsToScreenRect (&Bounds,&Rect);
		for (row=Rect.top;row<Rect.bottom;row++)
		{
			p.y = row;
			for (col=Rect.left;col<Rect.right;col++)
			{
				p.x = col;
				DPoint = ScreenPtToBasePt (p); 
				if (POINT_IN_AREAD (DPoint,NumMaskPoints,lpDpoints,1,0,0,0))
					SetPixel (CurView->hDC,p.x,p.y,RGB(255,255,0));
				else
					SetPixel (CurView->hDC,p.x,p.y,RGB(255,0,0));
			}
		}
		RestoreDC (CurView->hDC,-1);
	}

	rtn = POINT_IN_AREAD (DPoint,NumMaskPoints,lpDpoints,1,0,0,pMaskAccelerator);
	if (!rtn && GetGlobalPVal ("[%BADCOORDPOINT]",0,&BadPoint))
	{
		if (GetGlobalBVal2 ("[%WANTBADPOINTS]",FALSE))
		{    
			BadPointWin = BasePtToWinPt (&BadPoint);
			if (idist (Point,BadPointWin) < 2)
				rtn = TRUE;
		}
	}
	GlobalUnlock (hMaskArea);
{
#if ENABLETRACE
GSSiExitProg (947);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}  

BOOL PointInMaskAreaWinCoordD (LPDPOINT pDPoint)
#if ENABLETRACE
{GSSiEnterProg (948);
#endif
{
	LPMNMXCORD	lpRect; 
	HPDPOINT lpDpoints; 
	BOOL	rtn;
	DPOINT	DPoint; 
	HANDLE	hMaskArea= CurView->hMaskArea;     
	USHORT	NumMaskPoints=CurView->NumMaskPoints;
	LPHANDLE	pMaskAccelerator=&CurView->hMaskAccelerator[0];    
    
    if (CurView->DisplayInParent && CurView->Parent > 0)  
    {
    	hMaskArea = pViewports[CurView->Parent-1]->hMaskArea; 
    	pMaskAccelerator=&pViewports[CurView->Parent-1]->hMaskAccelerator[0];  
    	NumMaskPoints=pViewports[CurView->Parent-1]->NumMaskPoints;   
    }

	if (!hMaskArea || !MaskOffsetLine || IgnoreBounds)
{
#if ENABLETRACE
GSSiExitProg (948);
#endif
		return TRUE;
}
	lpRect = (LPMNMXCORD) GlobalLock (hMaskArea);
    lpRect++;
    lpDpoints = (LPDPOINT) lpRect;  
	rtn = POINT_IN_AREAD (*pDPoint,NumMaskPoints,lpDpoints,1,0,0,pMaskAccelerator); 
	GlobalUnlock (hMaskArea);
{
#if ENABLETRACE
GSSiExitProg (948);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}  

BOOL PolyInArea (short Type,long nPolyPoints,HPDPOINT pPolyPointsD,HPDOUBLE pPolyAZ,
				 long nAreaPoints,HPDPOINT pAreaPoints,int nPoly,HANDLE hPolyPartLen,
				 short InclusionOpt,LPHANDLE phPIAAcceleratorIn)
#if ENABLETRACE
{GSSiEnterProg (949);
#endif
{
	short		nInt;
	DWORD		i;
	HPDPOINT	pPolyPointsDInit=pPolyPointsD;
	BOOL		rtn=FALSE, HaveIn=FALSE, HaveOut=FALSE;
	HANDLE		hPIAAccelerator[3]={0,0,0}; 
	LPHANDLE	phPIAAccelerator=hPIAAccelerator;
	double		PctInHltAreas;
	MNMXCORD	Bounds;
	
	if (phPIAAcceleratorIn)
		phPIAAccelerator=phPIAAcceleratorIn;	

	if (MinPCTInArea > 0 && HaveAreaOffFile(0))
	{
		GetPolyBoundsD2 (pPolyPointsD,nPolyPoints,&Bounds,Type);
		PctInHltAreas = PercentOfPolyInHighlightAreas (Type,&Bounds,nPolyPoints,pPolyPointsD);
		if (PctInHltAreas < MinPCTInArea)
			return FALSE;
	}
 	nInt=0;
	for (i=0;i<nPolyPoints;i++,pPolyPointsD++)
	{
		if (POINT_IN_AREAD (*pPolyPointsD,nAreaPoints,pAreaPoints,nPoly,hPolyPartLen,0,phPIAAccelerator))
		{
			HaveIn = TRUE;
			switch (InclusionOpt)
			{   
				default: 
				case 0: // both partial and total
					rtn=TRUE;
					goto Done;
				case 1: // total in only
					break;      
				case -1: //total out only
					goto Done;
				case 2: // partial only
					if (HaveOut)
					{
						rtn=TRUE;
						goto Done;
					}
					break;
			}
		}
		else
		{
			HaveOut = TRUE;
			switch (InclusionOpt)
			{   
				default:
				case 0: // both partial and total
					if (HaveIn)
					{
						rtn=TRUE;
						goto Done;
					}
					break;
				case 1: // total in only 	
					goto Done;  
				case -1: //total out only
					break;
				case 2: // partial only
					if (HaveIn)
					{
						rtn=TRUE;
						goto Done;
					}
					break;
			}  
		}
	}  
	nInt = IntersectPolys1 (Type,GF_AREA,nPolyPoints,pPolyPointsDInit,pPolyAZ,nAreaPoints,pAreaPoints,nPoly,hPolyPartLen,0,0,0,0,phPIAAccelerator); 
	if (nInt)
	{   
		switch (InclusionOpt)
		{
			case 1:
			case -1:
				rtn=FALSE;
			break;
			default:
				rtn = TRUE;
			break;
		}
		goto Done;
	}
	if (HaveIn && InclusionOpt == 1)
	{
		rtn = TRUE;
		goto Done;
	}
	if (HaveIn && InclusionOpt == -1)
	{
		rtn = FALSE;
		goto Done;
	}
	if (Type != GF_AREA)
		goto Done;
	if (POINT_IN_AREAD (*pAreaPoints,nPolyPoints,pPolyPointsDInit,1,0,0,0)) 
	{
		if (InclusionOpt != -1)
			rtn = TRUE;
	}
	else if (InclusionOpt == -1)
		rtn = TRUE;

Done:  
	if (!phPIAAcceleratorIn) 
	{
		GSSiGlobFree (&hPIAAccelerator[0]);
		GSSiGlobFree (&hPIAAccelerator[1]);
		GSSiGlobFree (&hPIAAccelerator[2]);
	}

{
#if ENABLETRACE
GSSiExitProg (949);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}

BOOL AdjustFileBounds (short VPID, short FileNo, LPMNMXCORD FileBounds, LPSTR IndexEntryName)
#if ENABLETRACE
{GSSiEnterProg (950);
#endif
{   
	DPOINT	DPoint; 
	MNMXCORD newBounds;
	
	if (FileBounds->xmn > FileBounds->xmx || FileBounds->ymn > FileBounds->ymx)
{
#if ENABLETRACE
GSSiExitProg (950);
#endif
		return FALSE;
}
	DBoundsInit(&newBounds);
		 
	if (FileNo == ConvertLayer)
	{ 
		DPoint.x = FileBounds->xmn;
		DPoint.y = FileBounds->ymn;
		if (ConvertCoord(&DPoint, 0, 1))
		{
#if ENABLETRACE
			GSSiExitProg (950);
#endif
			return FALSE;
		}
		AddDPointToMinMax(&DPoint, &newBounds);
		DPoint.x = FileBounds->xmn;
		DPoint.y = FileBounds->ymx;
		if (ConvertCoord(&DPoint, 0, 1))
		{
#if ENABLETRACE
			GSSiExitProg (950);
#endif
			return FALSE;
		}
		AddDPointToMinMax(&DPoint, &newBounds);
		DPoint.x = FileBounds->xmx;
		DPoint.y = FileBounds->ymn;
		if (ConvertCoord(&DPoint, 0, 1))
		{
#if ENABLETRACE
			GSSiExitProg(950);
#endif
			return FALSE;
		}
		AddDPointToMinMax(&DPoint, &newBounds);
		DPoint.x = FileBounds->xmx;
		DPoint.y = FileBounds->ymx;
		if (ConvertCoord (&DPoint,0,1))
{
#if ENABLETRACE
GSSiExitProg (950);
#endif
			return FALSE;
}
		AddDPointToMinMax(&DPoint, &newBounds);
		*FileBounds = newBounds;
	}
    else if (OrthoAdjustVP == VPID && OrthoAdjustFile == FileNo) 
    {
		if (IndexEntryName)
		{
			if (*PickedOrthoName)
			{   
				HANDLE	hName=GSSiGlobAlloc ( 757,GMEM_MOVEABLE,512);
				LPSTR	pName = GlobalLock (hName), lpChr; 
				short	i;
				
				_fstrcpy (pName,IndexEntryName);
				if ((lpChr = _fstrchr (pName,':')))
					*lpChr=0;
				else if ((lpChr = _fstrrchr (pName,'.')))
					*lpChr=0;
				i = _fstricmp (PickedOrthoName,pName);	
	    		GSSiGlobUlFree (&hName);
				if (i)
{
#if ENABLETRACE
GSSiExitProg (950);
#endif
					return TRUE;
}
			}
    		AdjustBounds (FileBounds,OrthoAdjustX,OrthoAdjustY);
	    Exit:
{
#if ENABLETRACE
GSSiExitProg (950);
#endif
	    	return TRUE;
}
	    } 
        else
        {
	        FileBounds->xmn = -DBL_MAX;
	        FileBounds->xmx = DBL_MAX;
	        FileBounds->ymn = -DBL_MAX;
	        FileBounds->ymx = DBL_MAX;
	    } 
    }
{
#if ENABLETRACE
GSSiExitProg (950);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}
void AdjustMapControlPoints (short VPID, short FileNo, LPDOUBLE XBASE, LPDOUBLE YBASE)
#if ENABLETRACE
{GSSiEnterProg (951);
#endif
{
    short i;
    
/*    if (CurView && !CurView->Type && CurView->AutoSize && PltType == 1)
    {
	    CloseTRANS2 (&CurView->hTranFormat);
		CurView->hTranFormat = STRANBoundsToBounds (&CurView->WBoundsWhenSaved,&CurView->WBounds);
		
		for (i=0;i<4;i++)
		{	
			DPOINT	Point;
			
			Point.x = XBASE[i];
			Point.y = YBASE[i];
			Point = TranPoint (&Point,CurView->hTranFormat);
	        XBASE[i] = Point.x;
	        YBASE[i] = Point.y;                      
	    } 
    } */
    if (ConvertMapVP == VPID && ConvertMapFileNo == FileNo) 
    {
		for (i=0;i<4;i++)
		{	
			DPOINT	Point;
			
			Point.x = XBASE[i];
			Point.y = YBASE[i];
			ComputeNewMapControlPoint (&Point);
	        XBASE[i] = Point.x;
	        YBASE[i] = Point.y;                      
	    }
	}
    else if (OrthoAdjustVP == VPID && OrthoAdjustFile == FileNo) 
    {   
		for (i=0;i<4;i++)
		{	
	    	XBASE[i]+=OrthoAdjustX;
	    	YBASE[i]+=OrthoAdjustY;
	    }
	}
{
#if ENABLETRACE
GSSiExitProg (951);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}





void ConvertMinMax (LPMINMAX pMinMax,HANDLE hTranFrom, HANDLE hTranTo,int From,int To)
#if ENABLETRACE
{GSSiEnterProg (954);
#endif
{   
	DPOINT	FilePointD, WorldPoint, BoundsP[4];
	double	Minx=DBL_MAX,Miny=DBL_MAX,Maxx=-DBL_MAX,Maxy=-DBL_MAX; 
	short	i;
	
	BoundsP[0].x = pMinMax->xmn;
	BoundsP[0].y = pMinMax->ymn;
	BoundsP[1].x = pMinMax->xmn;
	BoundsP[1].y = pMinMax->ymx;
	BoundsP[2].x = pMinMax->xmx;
	BoundsP[2].y = pMinMax->ymx;
	BoundsP[3].x = pMinMax->xmx;
	BoundsP[3].y = pMinMax->ymn; 
	for (i=0;i<4;i++)
	{
	    TRANS2 (BoundsP[i].x,BoundsP[i].y,&WorldPoint.x,&WorldPoint.y,hTranFrom); 
		ConvertCoord(&WorldPoint,From,To); 
		Minx = min (Minx,WorldPoint.x);
		Miny = min (Miny,WorldPoint.y);
		Maxx = max (Maxx,WorldPoint.x);
		Maxy = max (Maxy,WorldPoint.y); 
	}
    TRANS2 (Minx,Miny,&FilePointD.x,&FilePointD.y,hTranTo); 
    pMinMax->xmn = max (SHRT_MIN,IDNINT(FilePointD.x));
    pMinMax->ymn = max (SHRT_MIN,IDNINT(FilePointD.y));
										
    TRANS2 (Maxx,Maxy,&FilePointD.x,&FilePointD.y,hTranTo);
    pMinMax->xmx = min (SHRT_MAX,IDNINT(FilePointD.x));
    pMinMax->ymx = min (SHRT_MAX,IDNINT(FilePointD.y));
{
#if ENABLETRACE
GSSiExitProg (954);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}

void MinMaxToBase (LPMINMAX pMinMax,LPMNMXCORD pMinMaxOut)
#if ENABLETRACE
{GSSiEnterProg (955);
#endif
{   
	DPOINT	FilePointD, WorldPoint, BoundsP[4];
	double	Minx=DBL_MAX,Miny=DBL_MAX,Maxx=-DBL_MAX,Maxy=-DBL_MAX; 
	short	i;
	
	DBoundsInit (pMinMaxOut);
	BoundsP[0].x = pMinMax->xmn;
	BoundsP[0].y = pMinMax->ymn;
	BoundsP[1].x = pMinMax->xmn;
	BoundsP[1].y = pMinMax->ymx;
	BoundsP[2].x = pMinMax->xmx;
	BoundsP[2].y = pMinMax->ymx;
	BoundsP[3].x = pMinMax->xmx;
	BoundsP[3].y = pMinMax->ymn; 
	for (i=0;i<4;i++)
	{
	    TRANS2 (BoundsP[i].x,BoundsP[i].y,&WorldPoint.x,&WorldPoint.y,hTranFileToBase); 
		AddDPointToMinMax (&WorldPoint,pMinMaxOut);
	}
{
#if ENABLETRACE
GSSiExitProg (955);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}

BOOL ConvertRectCoord (LPMNMXCORD pMinMaxOut, LPMNMXCORD pMinMaxIn, short from, short to)
#if ENABLETRACE
{GSSiEnterProg (956);
#endif
{
	DPOINT	DPoint;

	DBoundsInit (pMinMaxOut);

	DPoint.x = pMinMaxIn->xmn;     
	DPoint.y = pMinMaxIn->ymn;     
	ConvertCoord(&DPoint,from,to); 
	AddDPointToMinMax (&DPoint,pMinMaxOut);
	DPoint.x = pMinMaxIn->xmn;     
	DPoint.y = pMinMaxIn->ymx;     
	ConvertCoord(&DPoint,from,to); 
	AddDPointToMinMax (&DPoint,pMinMaxOut);
	DPoint.x = pMinMaxIn->xmx;     
	DPoint.y = pMinMaxIn->ymx;     
	ConvertCoord(&DPoint,from,to); 
	AddDPointToMinMax (&DPoint,pMinMaxOut);
	DPoint.x = pMinMaxIn->xmx;     
	DPoint.y = pMinMaxIn->ymn;     
	ConvertCoord(&DPoint,from,to); 
	AddDPointToMinMax (&DPoint,pMinMaxOut);
{
#if ENABLETRACE
GSSiExitProg (956);
#endif
    return TRUE;
}
#if ENABLETRACE
}
#endif
}
 
BOOL BoundsTotallyInBounds(mnmxCor MinMax,mnmxCor Bounds)
#if ENABLETRACE
{GSSiEnterProg (957);
#endif
{
	if (MinMax.xmn>=Bounds.xmn &&
		MinMax.ymn>=Bounds.ymn &&
		MinMax.xmx<=Bounds.xmx &&
		MinMax.ymx<=Bounds.ymx)
{
#if ENABLETRACE
GSSiExitProg (957);
#endif
		return(TRUE);
}
	else
{
#if ENABLETRACE
GSSiExitProg (957);
#endif
		return(FALSE);
}

#if ENABLETRACE
}
#endif
}

void RandomizePoint (LPDPOINT DPoint,UINT Seed,double MaxDist)
#if ENABLETRACE
{GSSiEnterProg (958);
#endif
{   int ir;
	double pct;
	
	ir = rand();
	pct = (2*(double)ir/RAND_MAX)-1.0; 
	DPoint->x += pct * MaxDist;
	ir = rand();
	pct = (2*(double)ir/RAND_MAX)-1.0; 
	DPoint->y += pct * MaxDist;
/*	DPoint->x += (2*((double)rand()-RAND_MAX/2)/RAND_MAX) * MaxDist;
	DPoint->y += (2*((double)rand()-RAND_MAX/2)/RAND_MAX) * MaxDist;*/
{
#if ENABLETRACE
GSSiExitProg (958);
#endif
	return; 
}
#if ENABLETRACE
}
#endif
}      

double WinDistToWorldDist (double WinDist)
#if ENABLETRACE
{GSSiEnterProg (959);
#endif
{ 
{
#if ENABLETRACE
GSSiExitProg (959);
#endif
	return (WinDist * CurView->BaseUnitsPerPixel);
}
#if ENABLETRACE
}
#endif
}

BOOL PointInEditBounds (LPDPOINT pPoint)
#if ENABLETRACE
{GSSiEnterProg (960);
#endif
{    
	 BOOL rtn=TRUE;
	 
     if (pPoint->x > EditBounds.xmx ||
         pPoint->y > EditBounds.ymx ||
         pPoint->x < EditBounds.xmn ||
         pPoint->y < EditBounds.ymn) rtn = FALSE;
{
#if ENABLETRACE
GSSiExitProg (960);
#endif
     return rtn;
}
#if ENABLETRACE
}
#endif
}

BOOL DisplayEditLimits (void)
#if ENABLETRACE
{GSSiEnterProg (961);
#endif
{
	int	i;
	int	Nump; 
	LPSHORT	lpNump;
	LPDPOINT	lpDpoint; 
	DPOINT	DPoint;
	LPPOINT	lpPoint;
	POINT	Points[5];
    
    if (ForceTAGIndex  || ForceRefIndex)
{
#if ENABLETRACE
GSSiExitProg (961);
#endif
    	return FALSE;
}
    if (ComputePCTTheme)
{
#if ENABLETRACE
GSSiExitProg (961);
#endif
    	return FALSE;
}
    if (CurView->ID == *pCommandViewport)
		GMEnableMenuItem(hWndMain, IDM_Z_EDITLIMITS, MF_BYCOMMAND | MF_DISABLED| MF_GRAYED);
	if (!CurView->UpdateFile)
{
#if ENABLETRACE
GSSiExitProg (961);
#endif
    	return FALSE;
}
    if (!CurVis)
{
#if ENABLETRACE
GSSiExitProg (961);
#endif
    	return FALSE; 
}
    if (!CurVis->FileIsVisible[CurView->UpdateFile-1])
{
#if ENABLETRACE
GSSiExitProg (961);
#endif
    	return FALSE; 
}
    if (!GetLayerBounds (&EditBounds,CurView->hDC, CurView->UpdateFile-1))
{
#if ENABLETRACE
GSSiExitProg (961);
#endif
    	return FALSE; 
}
    if (CurView->ID == *pCommandViewport)
		GMEnableMenuItem(hWndMain, IDM_Z_EDITLIMITS, MF_BYCOMMAND | MF_ENABLED);
	if (!GetGlobalBVal2 ("[%SHOWEDITBOUNDS]",TRUE))
{
#if ENABLETRACE
GSSiExitProg (961);
#endif
		return TRUE;
}
	if (!RectInWBounds (&EditBounds,1))  
		Nump=0; 
	else
	{   
		DPoint.x = max (EditBounds.xmn,CurView->WBounds.xmn);
		DPoint.y = max (EditBounds.ymn,CurView->WBounds.ymn);
		Points[0] = BasePtToWinPt(&DPoint);
		DPoint.y = min (EditBounds.ymx,CurView->WBounds.ymx);
		Points[1] = BasePtToWinPt(&DPoint);
		DPoint.x = min (EditBounds.xmx,CurView->WBounds.xmx);
		Points[2] = BasePtToWinPt(&DPoint);
		DPoint.y = max (EditBounds.ymn,CurView->WBounds.ymn);
		Points[3] = BasePtToWinPt(&DPoint); 
		Points[4] = Points[0]; 
		Nump=5;
    }
    lpPoint = &Points[4];
	SetDisplayMode (CurView->hDC, GF_TEXTMODE);
	if (!FileMode)
	{
	    GSSiDeleteObject(&CurView->hRgn);
	    CurView->hRgn = CreateVPRgn(FALSE,FALSE);
	    SelectClipRgn (CurView->hDC,CurView->hRgn);
	    GSSiDeleteObject(&CurView->hRgn);  
	}
    {   
    	HBRUSH	Brush, OldBrush;
    	HANDLE	hMaskPoly;
    	LPPOINT pMask;
	    HBITMAP		hBM;
	    short	OldMode;
		HPEN	OldPen; 
		LOGBRUSH    NDB;

	    OldPen = SelectObject (CurView->hDC,GetStockObject(NULL_PEN)); 
        NDB.lbStyle = BS_HATCHED;
        NDB.lbColor = RGB(64,64,64);
        NDB.lbHatch = HS_DIAGCROSS;
        Brush =  CreateBrushIndirect(&NDB);
	    OldBrush = SelectObject (CurView->hDC,Brush);
	    
	    hMaskPoly = GSSiGlobAlloc (1740,GMEM_MOVEABLE,(Nump+5)*sizeof(POINT));
	    pMask = (LPPOINT)GlobalLock (hMaskPoly);
	    pMask->x = CurView->DrawRect.left-1;
	    pMask++->y = CurView->DrawRect.bottom+1; 
	    for (i=0;i<Nump;i++,lpPoint--,pMask++)
	    	*pMask = *lpPoint;
	    pMask->x = CurView->DrawRect.left-1;
	    pMask++->y = CurView->DrawRect.bottom+1; 
	    pMask->x = CurView->DrawRect.left-1;
	    pMask++->y = CurView->DrawRect.top-1;
	    pMask->x = CurView->DrawRect.right+1;
	    pMask++->y = CurView->DrawRect.top-1;
	    pMask->x = CurView->DrawRect.right+1;
	    pMask->y = CurView->DrawRect.bottom+1;
	    GlobalUnlock (hMaskPoly);
	    pMask = (LPPOINT)GlobalLock (hMaskPoly);  
	    OldMode = SetBkMode (CurView->hDC,TRANSPARENT);
	    i=Polygon (CurView->hDC,pMask,Nump+5);         
	    GSSiGlobUlFree (&hMaskPoly);
	    SetBkMode (CurView->hDC,OldMode);
	    SelectObject (CurView->hDC,OldBrush);
	    SelectObject (CurView->hDC,OldPen);
	    DeleteObject (Brush);
    }  
{
#if ENABLETRACE
GSSiExitProg (961);
#endif
	return TRUE; 
}
#if ENABLETRACE
}
#endif
}

 

void ComputeIndexOrthoRes (LPFILEINDEX lpIndex)
#if ENABLETRACE
{GSSiEnterProg (966);
#endif
{

    if (lpIndex->CurrentEntry->BMWidth > 1)
    	lpIndex->OrthoRes = (lpIndex->CurrentEntry->Bounds.xmx - lpIndex->CurrentEntry->Bounds.xmn) /
    						(lpIndex->CurrentEntry->BMWidth - 1);
{
#if ENABLETRACE
GSSiExitProg (966);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}
	
BOOL GetOpenFileSymName (int desc,LPSTR Name)
#if ENABLETRACE
{GSSiEnterProg (967);
#endif
{
	if (!hOpenFileSyms)
{
#if ENABLETRACE
GSSiExitProg (967);
#endif
		return FALSE; 
}
	{
		LPSHORT	pSym=(LPSHORT)GlobalLock (hOpenFileSyms);
		short	nSyms=*pSym++;
		UINT	i=0; 
		BOOL	rtn=FALSE;  
		HPSTR	pName;
		
		while (nSyms--) 
		{
			if (*pSym++ == desc)
			{
				pName = GlobalLock (hOpenFileSymNames);
				pName += (32 * (long)i);
				_fstrncpy (Name,pName,32);
				*(Name+32)=0; 
				rtn = TRUE;
				GlobalUnlock (hOpenFileSymNames);
				goto Exit;
			}
			i++;
		}
	Exit:
		GlobalUnlock (hOpenFileSyms);
{
#if ENABLETRACE
GSSiExitProg (967);
#endif
		return rtn;
}
	}
#if ENABLETRACE
}
#endif
}

BOOL CreateOpenFileSymName (void)
#if ENABLETRACE
{GSSiEnterProg (968);
#endif
{   
	LPSHORT	nSyms;
	DestroyOpenFileSymNames ();
//	GSSillseek(Fid,UsedDescOffset,0);
//	ProcessPrimarySeg (0,-Desc,0,FALSE);
	hOpenFileSyms = GSSiGlobAlloc ( 762,GMEM_MOVEABLE,USHRT_MAX);
	hOpenFileSymNames = GSSiGlobAlloc ( 763,GMEM_MOVEABLE,((long)32 * (long)USHRT_MAX)/2);
	nSyms = (LPSHORT)GlobalLock (hOpenFileSyms);
	*nSyms = 0;
	GlobalUnlock (hOpenFileSyms);
{
#if ENABLETRACE
GSSiExitProg (968);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}     

BOOL OpenFileSymListAdd (int desc,LPSTR DescName)
#if ENABLETRACE
{GSSiEnterProg (969);
#endif
{   
	HPSTR	pName;
	LPSHORT	pSym;
	short	nSyms;
	
	if (!hOpenFileSyms)
{
#if ENABLETRACE
GSSiExitProg (969);
#endif
		return FALSE;
}
	pName = GlobalLock (hOpenFileSymNames);
	pSym = (LPSHORT)GlobalLock (hOpenFileSyms);
	(*pSym)++; 
	nSyms = *pSym;
	pSym += nSyms;
	*pSym = desc;
	pName += ((long)(nSyms-1) *32);
	_fstrncpy (pName,DescName,32);
	GlobalUnlock (hOpenFileSyms);
	GlobalUnlock (hOpenFileSymNames);
{
#if ENABLETRACE
GSSiExitProg (969);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}  

void DestroyOpenFileSymNames (void)
#if ENABLETRACE
{GSSiEnterProg (970);
#endif
{
	GSSiGlobFree (&hOpenFileSyms);
	GSSiGlobFree (&hOpenFileSymNames);
{
#if ENABLETRACE
GSSiExitProg (970);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

HBRUSH CreateTransparentBrush(int itrans, COLORREF color)
{
	HBRUSH brush;
	LOGBRUSH	lb;
	int pct[5] = { 94, 50, 25, 6, 0 };
	int transparency = (pct[max(0,min(4,itrans-1))] * 255) / 100;

	int r = GetRValue(color);
	int g = GetGValue(color);
	int b = GetBValue(color);
	lb.lbColor = RGBI(r, g, b, transparency);

	lb.lbStyle = BS_SOLID;
	lb.lbHatch = 0;
	brush = CreateBrushIndirect(&lb);
	return brush;
}

HPEN CreateTransparentPen(int itrans, int width, COLORREF color)
{
	LOGBRUSH lb = { 0 };
	DWORD style = 0;
	int pct[5] = { 94, 50, 25, 6, 0 };
	int transparency = (pct[max(0, min(4, itrans - 1))] * 255) / 100;
	int r = GetRValue(color);
	int g = GetGValue(color);
	int b = GetBValue(color);
	lb.lbColor = RGBI(r, g, b, transparency);

	lb.lbStyle = BS_SOLID;
	HPEN hPen = ExtCreatePen(PS_GEOMETRIC | PS_SOLID | PS_ENDCAP_FLAT | PS_JOIN_ROUND, width, &lb, 0, 0);

	return hPen;
}

COLORREF ColorWithTransparency(COLORREF color, int transparency)
{
	COLORREF rtn = color;

	if (transparency)
	{
		int r = GetRValue(color);
		int g = GetGValue(color);
		int b = GetBValue(color);
		transparency = min(255, transparency);
		color = RGBI(r, g, b, transparency);
	}
	return rtn;
}

HBRUSH CreateGMBrush (COLORREF GMColor,int UseHalfTone,HDC hDC)
#if ENABLETRACE
{GSSiEnterProg (971);
#endif
{   
	BYTE		PatByt;  
	COLORREF	color;
	PATBYTE		PatByte;      
	HBRUSH		brush=0;
				
	PatByt = GetWValue (GMColor);
	_fmemmove (&PatByte,&PatByt,1); 
Top:
	color = ConvertColor(ColorWOWidth (GMColor),UseHalfTone);  
	if (!PatByte.Pattern || PatByte.notUsingPattern)
	{
		if (hDC)
			SetROP2(hDC,DisplayRasterOpt);
		if (PatByte.notUsingPattern)
		{
			LPTRANSBYTE ptb = (LPTRANSBYTE)&PatByte;

			color = ColorWithTransparency(color,ptb->Transparency*2);
		}

{
#if ENABLETRACE
GSSiExitProg (971);
#endif
		return CreateSolidBrush(color);
}
	}
	else if (PatByte.Pattern < 5)
	{
		if (useGDIPlus)
		{
			brush = CreateTransparentBrush(PatByte.Pattern, color);
		}
		else
		{
			HBITMAP hbmp;
			hbmp = hPatBMP[PatByte.Pattern - 1];
			if (!hbmp)
			{
				PatByte.Pattern = 0;
				goto Top;
			}
			brush = CreatePatternBrush(hbmp);
			//DeleteObject(hbmp);
		}
	}
	else if (PatByte.Pattern == 5)
		brush = GetStockObject(HOLLOW_BRUSH);
    if (hDC && brush && !useGDIPlus)
    {   
    	int	MaskOpt=R2_MASKPEN; //use R2_MERGEPEN with 50%BW bitmap for halftone filter
    	
	    SetTextColor (hDC,color);     
	    SetBkColor (hDC,RGB(255,255,255));
	    if (PatByte.Transparent)
			SetROP2(hDC,MaskOpt);  
		else 
		{
			SetROP2(hDC,R2_COPYPEN);
		}
    } 
{
#if ENABLETRACE
GSSiExitProg (971);
#endif
    return brush;
}
#if ENABLETRACE
}
#endif
} 

void DisplayShadow (HDC hDC,LPRECT pRect,short offset)
#if ENABLETRACE
{GSSiEnterProg (972);
#endif
{   
	RECT	ShadowRect;
	COLORREF	Color;
	BYTE		PatByt;  
	COLORREF	color;
	PATBYTE		PatByte;      
	
	if (GetGlobalBVal ("[%SHADOWLEFT]"))
	{
		ShadowRect.left = pRect->left-offset;
		ShadowRect.right = pRect->right-offset;
		ShadowRect.top = pRect->top+offset;
		ShadowRect.bottom = pRect->bottom+offset;  
	}
	else
	{
		ShadowRect.left = pRect->left+offset;
		ShadowRect.right = pRect->right+offset;
		ShadowRect.top = pRect->top+offset;
		ShadowRect.bottom = pRect->bottom+offset;  
	}
	PatByte.Transparent = 1;  
	PatByte.BGOpt = 0;
	PatByte.Pattern = 2;
	_fmemmove (&PatByt,&PatByte,1);
	Color = RGBW (128,128,128,PatByt);
	if (GetROP2 (hDC) != R2_NOT)
		FillRectPoly (hDC,&ShadowRect,Color); 
{
#if ENABLETRACE
GSSiExitProg (972);
#endif
	return;
}
#if ENABLETRACE
}
#endif
} 

BOOL DisplayMaskArea (void)
#if ENABLETRACE
{GSSiEnterProg (973);
#endif
{
 HPDPOINT	lpBasePoint, lpFirstPoint;
 UINT	i, Nump, n; 
 HPDPOINT	lpDpoint;
 HPPOINT	lpPoint;
 HPEN	hPen=0, OldPen=0;
 LPMNMXCORD	lpRect; 
 HANDLE	hPoly;  
 short	OffLineWidth;
 long	OffLineColor;
    
	if (!CurView->hMaskArea) 
{
#if ENABLETRACE
GSSiExitProg (973);
#endif
    	return FALSE;
}
    SaveDC (CurView->hDC);
	OffLineWidth = GetGlobalLVal2 ("[%OFFSETLINEWIDTH]",4);
	OffLineColor = GetGlobalLVal2 ("[%OFFSETLINECOLOR]",RGB(255,0,0));
    Nump = CurView->NumMaskPoints;  
    lpRect = (LPMNMXCORD) GlobalLock (CurView->hMaskArea);
	lpRect++;
	lpFirstPoint = lpBasePoint = (HPDPOINT) lpRect;
	lpRect--;
    hPoly = GSSiGlobAlloc ( 764,GHND,(Nump+1)*sizeof(POINT));
    lpPoint = (HPPOINT) GlobalLock (hPoly);
	if (!RectInWBounds (lpRect,1))  
		Nump=0;  
	RectInit (&MaskRect);
	n = 0;
    for (i=0;i<Nump;i++,lpBasePoint++)
    {
		lpPoint[n] = BasePtToWinPt(lpBasePoint);
		if (n)
		{
			if (idist (lpPoint[n],lpPoint[n-1]) >= 0)//OffLineWidth*2)
				AddPointToRect (lpPoint[n++],&MaskRect); 
		}
		else
			n++;
    }
	lpPoint[n++] = lpPoint[0];
    Nump = n;
	SetDisplayMode (CurView->hDC, GF_TEXTMODE);
	if (!FileMode)
	{
	    GSSiDeleteObject(&CurView->hRgn);
	    CurView->hRgn = CreateVPRgn(FALSE,FALSE);
	    SelectClipRgn (CurView->hDC,CurView->hRgn);
	    GSSiDeleteObject(&CurView->hRgn);  
	}
	if (OffLineColor >= 0 && OffLineWidth > 0)
	{
		hPen = CreatePen (PS_SOLID,(short)IDNINT(OffLineWidth*DeviceToScreenFactor()),OffLineColor);
	    OldPen = SelectObject (CurView->hDC,hPen); 
	    if (OutlineZoomArea && !ComputePCTTheme) 
	    {
	    	if (CurView->NumMaskAreaParts)
	    	{   
	    		LPINT	pPartLen = (LPINT)&lpFirstPoint[CurView->NumMaskPoints];
				short	i,nPoly = CurView->NumMaskAreaParts+1;

        		for (i=0;i<nPoly;i++)
        		{   
					GWPolylineD (CurView->hDC,lpFirstPoint,*pPartLen,0); 
					lpFirstPoint+=*pPartLen++;
					if (i)
						lpFirstPoint++;
        		}   
	    	}
	    	else
		    	i=Polyline (CurView->hDC,lpPoint,Nump);
		} 
	}
    if (MaskOffsetLine && !CurView->Transparent)
    {   
    	HBRUSH	BkBrush, OldBrush;
    	HANDLE	hMaskPoly;
    	HPPOINT pMask;  
    	COLORREF	Color;
    	long	PenColor;
    	HPEN	OldPen,hPen=0;
		BOOL	DeletePen;  
    	
	    if (ComputePCTTheme)   
	    {
	    	Color = ComputePCTMaskColor;
	    	PenColor = -1;
	    }
	    else 
	    {
	    	Color = GetGlobalLVal2 ("[%MASKCOLOR]",CurView->BackGroundColor);
	    	PenColor = -1;
	    }
	     
    	if (PenColor >= 0)
    	{
    		DeletePen = TRUE; 
	    	hPen = CreatePen (PS_SOLID,0,Color);
	    } 
	    else 
	    {
	    	DeletePen = FALSE;
	    	hPen = GetStockObject (NULL_PEN);
	    }
	    OldPen = SelectObject (CurView->hDC,hPen);
	    
	    BkBrush = CreateGMBrush (Color,0,CurView->hDC);
	    OldBrush = SelectObject (CurView->hDC,BkBrush);
	    
	    if (Color == 150994944) //50%BW
	    	SetROP2(CurView->hDC,R2_MERGEPEN);  	
	    else if (Color == 285212672) //50%BW
	    	SetROP2(CurView->hDC,R2_MERGENOTPEN);  	
	    hMaskPoly = GSSiGlobAlloc (1741,GMEM_MOVEABLE,(Nump+5)*sizeof(POINT));
	    pMask = (HPPOINT)GlobalLock (hMaskPoly);
	    pMask->x = CurView->DrawRect.left-1;
	    pMask++->y = CurView->DrawRect.bottom+1; 
	    lpPoint += Nump-1;
	    for (i=0;i<Nump;i++,lpPoint--,pMask++)
	    	*pMask = *lpPoint;
	    pMask->x = CurView->DrawRect.left-1;
	    pMask++->y = CurView->DrawRect.bottom+1; 
	    pMask->x = CurView->DrawRect.left-1;
	    pMask++->y = CurView->DrawRect.top-1;
	    pMask->x = CurView->DrawRect.right+1;
	    pMask++->y = CurView->DrawRect.top-1;
	    pMask->x = CurView->DrawRect.right+1;
	    pMask->y = CurView->DrawRect.bottom+1;
	    GlobalUnlock (hMaskPoly);
	    pMask = (HPPOINT)GlobalLock (hMaskPoly);
	    i=Polygon (CurView->hDC,pMask,Nump+5);
	    GSSiGlobUlFree (&hMaskPoly);
	    SelectObject (CurView->hDC,OldBrush);
	    DeleteObject (BkBrush);
	    SelectObject (CurView->hDC,OldPen);
	    if (hPen && DeletePen)
	    	DeleteObject (hPen);
    }
    if (OldPen)
    {  
	    SelectObject (CurView->hDC,OldPen);
	    if (hPen)
	    	DeleteObject (hPen); 
	}
    GSSiGlobUlFree (&hPoly);
	ClearFullWindowBitmap (0);
Exit:
    GlobalUnlock (CurView->hMaskArea);
	RestoreDC (CurView->hDC,-1);
{
#if ENABLETRACE
GSSiExitProg (973);
#endif
	return TRUE; 
}
#if ENABLETRACE
}
#endif
} 

 

 

BOOL RunHltMacro (LPSTR Macro,long StatusInit,LPSTR Title)
#if ENABLETRACE
{GSSiEnterProg (977);
#endif
{               
    short	pos=BT_FIRST;      
    long	Sequence, Refno, TotNum, CurLoc=1;
    HIGHLIGHTDATA	HighlightData; 
    BOOL	rtn=TRUE; 
    
    if (!ExistFile (Macro))
{
#if ENABLETRACE
GSSiExitProg (977);
#endif
    	return FALSE;
}
	TotNum = BT_NUM_IN_INDEX (hHighlight2);
	if (TotNum > 1 && TotNum >= StatusInit)
		CreateStatusWind (hWndMain,1,0);
	while (!BT_FIND (hHighlight2,(LPSTR)&Sequence,pos,BT_ANY,(LPSTR)&Refno))
	{   
		pos = BT_NEXT;
		BT_FIND (hHighlight,(LPSTR)&Refno,BT_FIRST,BT_EQ,(LPSTR)&HighlightData);
   		PickList[0]=HighlightData.PD;
		SetPickGlobals (0);    
		rtn = ProcessMacroFile (Macro,0,0,0);
		if (!rtn)
			break;
		StatusWindowUpdate (Title,"",TotNum,CurLoc++);
	}
	DestroyStatusWindow(0);  
{
#if ENABLETRACE
GSSiExitProg (977);
#endif
	return rtn; 
}
#if ENABLETRACE
}
#endif
}

BOOL ExecutePointLocationMacro(DPOINT DPoint, LPSTR locatedTo)
#if ENABLETRACE
{
	GSSiEnterProg(978);
#endif
	{
		char	File[MAX_PATH];
		BOOL	rtn = FALSE;

		CurrentPoint = DPoint;
		if (GetGlobalCVal("[%PLMACRO]", File, 0))
			rtn = ProcessMacroFile(File, 0, 0, 0);
		{
#if ENABLETRACE
			GSSiExitProg(978);
#endif
			return rtn;
		}
#if ENABLETRACE
	}
#endif
}

BOOL ExecuteLocationFailedMacro(LPSTR locatedTo,LPSTR errorMess)
#if ENABLETRACE
{
	GSSiEnterProg(978);
#endif
	{
		HANDLE hStr = GSSiGlobAlloc(0, GMEM_MOVEABLE, 4096);
		LPSTR  pStr = GlobalLock(hStr);
		char   file[MAX_PATH];
		BOOL	rtn = FALSE;

		if (GetGlobalCVal("[%PLFMACRO]", file, 0))
		{
			rtn = TRUE;
			sprintf(pStr, "$MACRO(%s,%s,%s)", file, locatedTo,errorMess);
			ProcessText(pStr);
		}
		GSSiGlobUlFree(&hStr);
		{
#if ENABLETRACE
			GSSiExitProg(978);
#endif
			return rtn;
		}
#if ENABLETRACE
	}
#endif
}

BOOL ExecuteItemLocationMacro(short Item)
#if ENABLETRACE
{GSSiEnterProg (978);
#endif
{   
    char	File[128];
    BOOL	rtn=FALSE;
	                  
	SetPickGlobals (Item);
	if (GetGlobalCVal ("[%ILMACRO]",File,0))
		rtn = ProcessMacroFile (File,0,0,0);
{
#if ENABLETRACE
GSSiExitProg (978);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
} 

void LogItemLengthError (long LastItemLen,long LastItemLenActual)
#if ENABLETRACE
{GSSiEnterProg (979);
#endif
{   
	char	mess[256], Name[256]; 
	 
	if (!GetGlobalBVal ("[%LOGERRORS]"))
{
#if ENABLETRACE
GSSiExitProg (979);
#endif
		return; 
}
	sprintf (mess,"itemlen: %ld %ld %ld|",CurItemHeadLoc,LastItemLenActual,LastItemLen);
	_fstrcpy (Name,PltName);
	ExpandText (Name); 
	_fullpath (_fstrchr(mess,0),Name,200);

	AppendFile ("gmerrors.txt",mess);
{
#if ENABLETRACE
GSSiExitProg (979);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}  

BOOL FixGMErrors (LPSTR File)
#if ENABLETRACE
{GSSiEnterProg (980);
#endif
{
	OFSTRUCTGM OFStruct;
	HFILE	Fid, FidMap2;
	char	str[260];
	LPSTR	lpE, lpC;
	short	old,new;
	long	loc;    
	ITEM	Item;
//itemlen: 616118 112 80|E:\LAKSCOUT\DOTROADS\anok.plt
	
	Fid = GSSiOpenFile (File,&OFStruct,OF_READ);
	if (Fid == HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (980);
#endif
		return FALSE; 
}
		
	while (fgetstring (str,256,Fid))
	{
		lpC = _fstrchr (str,':');
		lpC+=2;
		lpE = _fstrchr(lpC,' ');
		*lpE++=0;	
		loc = atol (lpC); 
		lpC = lpE;
		lpE = _fstrchr(lpC,' ');
		*lpE++=0;	
		new = atoi (lpC); 
		lpC = lpE;
		lpE = _fstrchr(lpC,'|');
		*lpE++=0;	
		old = atoi (lpC); 
		FidMap2 = GSSiOpenFile (lpE,&OFStruct,OF_READWRITE);
		GSSillseek (FidMap2,loc,0);
		BigRead (FidMap2,(HPSTR)&Item,sizeof(ITEM));
		GSSiClose (FidMap2);
	}                    
	GSSiClose (Fid);
{
#if ENABLETRACE
GSSiExitProg (980);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL GetRoutePointFromTime (LPSTR File,long StartSeg,double time,LPDPOINT pPoint,LPDOUBLE pAZ,LPDOUBLE pLength,LPLONG pAtSeg)
#if ENABLETRACE
{GSSiEnterProg (981);
#endif
{
    LPROUTEATFILE	pRAF;
	long		AtKey, Offset, LastSeg;   
    short       NumFields, Reclen, len;
    LPGWDHEADER lpGWDHead;
    HANDLE		hDB;  
    double		SpeedFactor = GetGlobalDVal2("[%SPEEDFACTOR]",1.5);
    double		StartCost, LastCost, Minutes=SpeedFactor*time/60, SegCost, PCT, MinDist;  
    BOOL		rtn=FALSE;
    DPOINT		Points[4];

	hDB = OpenGWDatabase (File,BT_READ);  
	if (!hDB)
{
#if ENABLETRACE
GSSiExitProg (981);
#endif
		return FALSE;
}
    lpGWDHead = (LPGWDHEADER)GlobalLock (hDB);
    pRAF = (LPROUTEATFILE)&lpGWDHead->GWDData; 
    if (BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&StartSeg,BT_FIRST,BT_EQ,(LPSTR)&Offset))
    	goto Exit;
    FillGWDData (lpGWDHead,Offset);
    StartCost = pRAF->Cost; 
    if (BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)pAtSeg,BT_FIRST,BT_EQ,(LPSTR)&Offset))
    	goto Exit;
    FillGWDData (lpGWDHead,Offset);
    LastSeg = *pAtSeg; 
    LastCost = pRAF->Cost;
    AtKey = pRAF->LastRef;
	while (fabs (StartCost - pRAF->Cost) < Minutes && 
			!BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&AtKey,BT_FIRST,BT_EQ,(LPSTR)&Offset))
	{   
	    LastCost = pRAF->Cost;
    	LastSeg = pRAF->Ref;
    	FillGWDData (lpGWDHead,Offset);  
	    AtKey = pRAF->LastRef;
	}
	SegCost = ((StartCost - pRAF->Cost)-(StartCost - LastCost)); 
	if (SegCost)
		PCT = (Minutes - (StartCost - LastCost))/SegCost;
	else
		PCT = 0;
	if (!GetSegCoorByPct (LastSeg,0,&Points[0],pAZ,pLength,FALSE))
		goto GetPoint;
	if (!GetSegCoorByPct (LastSeg,1,&Points[1],pAZ,pLength,FALSE))
		goto GetPoint;
	if (!GetSegCoorByPct (pRAF->Ref,0,&Points[2],pAZ,pLength,FALSE))
		goto GetPoint;
	if (!GetSegCoorByPct (pRAF->Ref,1,&Points[3],pAZ,pLength,FALSE))
		goto GetPoint;
	MinDist = ldistp (Points[1],Points[2]);
	MinDist = min (MinDist,ldistp (Points[1],Points[3])); 
	if (ldistp (Points[0],Points[2]) < MinDist || ldistp (Points[0],Points[3]) < MinDist)
		PCT = 1.0 - PCT; 
GetPoint:   
	rtn = GetSegCoorByPct (LastSeg,PCT,pPoint,pAZ,pLength,FALSE);   
	if (rtn)
		*pAtSeg = LastSeg;
Exit:
	GlobalUnlock (hDB);
    CloseGWDatabase (hDB); 
{
#if ENABLETRACE
GSSiExitProg (981);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}

int FillRectColor (HDC hDC,LPRECT Rect,COLORREF Color)
{
    HBRUSH  CurBrush=0, brush;
	int	rtn;    

    brush = CreateGMBrush (Color,0,hDC);
    if (brush)
    	CurBrush = SelectObject (hDC,brush); 
	rtn = FillRect (hDC,Rect,brush);
    if (CurBrush)
	    SelectObject (hDC,CurBrush);
    GSSiDeleteObject (&brush);
	return rtn;
}

short FillRectPoly(HDC hDC, LPRECT Rect, COLORREF Color)
#if ENABLETRACE
{GSSiEnterProg (982);
#endif
{
    POINT   Points[5];
    HBRUSH  CurBrush=0, brush;
    HPEN    CurPen, pen=0;
    short       i;
	int		iw = RECTWIDTH(Rect);
	int		ih = RECTHEIGHT(Rect);
    
    {
    	HDC hdcmain=GetDC (hWndMain);  
    	short	ii;
    	
    	if (hDC == hdcmain)
    		ii=1;
    	ReleaseDC (hWndMain,hdcmain);
    }
    SaveDC (hDC);
    brush = CreateGMBrush (Color,0,hDC);
    if (brush)
    	CurBrush = SelectObject (hDC,brush); 
    pen = CreatePen (PS_SOLID,0,ColorWOWidth(Color));  
    if (GetROP2 (hDC) != R2_MASKPEN)
    	CurPen = SelectObject (hDC,pen);
    else
    	CurPen = SelectObject (hDC,GetStockObject(NULL_PEN));
    Points[0].x = Rect->left;
    Points[0].y = Rect->bottom;
    Points[1].x = Rect->left;
    Points[1].y = Rect->top;    
    Points[2].x = Rect->right;
    Points[2].y = Rect->top;
    Points[3].x = Rect->right;
	Points[3].y = Rect->bottom;
	Points[4] = Points[0];
	if (brush)
    	i = Polygon (hDC,Points,5);  
    else
    	i = Polyline (hDC,Points,5);  
    if (CurBrush)
	    SelectObject (hDC,CurBrush);
	if (CurPen)                         
    	SelectObject (hDC,CurPen);  
    GSSiDeleteObject (&pen); 
    GSSiDeleteObject (&brush);
    RestoreDC (hDC,-1);
{
#if ENABLETRACE
GSSiExitProg (982);
#endif
    return (i);
}

#if ENABLETRACE
}
#endif
}      

BOOL SetCVTFromVis (void)
#if ENABLETRACE
{GSSiEnterProg (983);
#endif
{   
	UINT	idesc; 
	short	type;
	
	if (!GetGlobalBVal ("[%SYMDICTREQUIRED]"))
{
#if ENABLETRACE
GSSiExitProg (983);
#endif
		return FALSE;
}
	SelectVisList (FALSE);
	if (CurVis->WantType[0])
		type =3;
	else
		type =2;
	for (idesc=1;idesc<3201;idesc++) 
	{ 
		if (GetVisibility (idesc))
			CurView->CurVisType[idesc] = type; 
		else
			CurView->CurVisType[idesc] = 0; 
	}
{
#if ENABLETRACE
GSSiExitProg (983);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}


HANDLE GWPolygon2 (HDC hDC, HPPOINTS lpPoints, long npnts, int nPoly, HANDLE hPolyPartLen, int desc,BOOL ShowBorder,BOOL DoFill,LPINT pBorderSymNum)
#if ENABLETRACE
{GSSiEnterProg (990);
#endif
{	HANDLE Handle;
	HPPOINT	lpNewPoints, lpPntNew;
	HPPOINTS	lpPointsIn=lpPoints;
	DWORD	i, NewPnts, LastPartPoint; 
	short	Attempts=0, iPoly;  
	BOOL	ReduceNumPoints=FALSE, FirstPartPoint=TRUE, LinkPoint=FALSE;
	POINT	LastPoint, NextPoint, OutPoint;
	double	SkipDist=1;  
	LPWORD	pPartLen; 
	RECT	Rect, NewRect, BoundsRect;
	BOOL	LastOut;
	static	short	DebugID=6;
	static	long	debugref=39474;
	short	ii; 
	HBRUSH	hOldBrush=0, hBrush=0;
	HPEN	hOldPen=0, hPen=0, hReturnPen=0; 
	BOOL	AreaIsNull = FALSE;
	
	if (!Display || !hDC || ShowBadSyms)
{
#if ENABLETRACE
GSSiExitProg (990);
#endif
		return (0);  
}
//    if (!GetTypeVisibility(8) && SymIsInvis (CurrentDesc))
//    	retrn (0);  
	TotPointsProcessed += npnts;  
	if (CurView->ID == DebugID)
		ii=1;
	MostPoints = max (MostPoints,npnts);
	if (npnts > MaxDisplayPoints)   
	{
		if (CurrentItemMinMax.xmn/CurView->FileFactor >= CurView->Bounds.xmn &&
			CurrentItemMinMax.ymn/CurView->FileFactor >= CurView->Bounds.ymn &&
			CurrentItemMinMax.xmx/CurView->FileFactor <= CurView->Bounds.xmx &&
			CurrentItemMinMax.ymx/CurView->FileFactor <= CurView->Bounds.ymx)
			Attempts = 1;
		else
			Attempts = 0; 
		SkipDist = 0.2/(FileDistToWinDist*CurView->FileFactor);
		ReduceNumPoints = TRUE; 
	}
	/*if (!FileProjectionType && (FileMode || CurView->FileFactor <= 1) && !ReduceNumPoints) 
	{
		if (DoGraphics) 
		{
		    SaveDC (hDC); 
		    AreaIsNull = SetAreaPenAndBrush (hDC,0,desc,ItemIsHighlighted,ShowBorder,&hPen,&hBrush);
		    if (hBrush)
				hOldBrush = SelectObject (hDC,hBrush); 
			else if (ItemIsHighlighted)
				AreaIsNull = FALSE;
			if ((!hPolyPartLen || nPoly < 2) && hPen) 
			{
				hOldPen = SelectObject (hDC,hPen); 
				if (!hOldPen)
					hOldPen = GetStockObject(BLACK_PEN);
			}
			else
				hReturnPen = hPen;
			i = Polygon (hDC,lpPoints,(short)npnts);
			if (hBrush)
			{ 
				if (hOldBrush)
					SelectObject (hDC,hOldBrush); 
				else
			    	SelectObject(hDC, GetStockObject(BLACK_BRUSH));
				if (hBrush != HighlightBrush && hBrush != GetStockObject(NULL_BRUSH))
					GSSiDeleteObject (&hBrush);
			}
			if (hOldPen)
				SelectObject (hDC,hOldPen);
			if (hPen != HighlightPen && hPen != HighlightBrush && hPen != GetStockObject(NULL_PEN) && hReturnPen != hPen)
				GSSiDeleteObject (&hPen);
			RestoreDC (hDC,-1);
		}
	}
	else*/                                 
	{   
		BoundsRect.left = CurView->Bounds.xmn;
		BoundsRect.right = CurView->Bounds.xmx ;
		BoundsRect.top = CurView->Bounds.ymn;
		BoundsRect.bottom = CurView->Bounds.ymx;
		Handle = GSSiGlobAlloc ( 769,GMEM_MOVEABLE,(long)npnts * sizeof(POINT)); 
		lpNewPoints = (HPPOINT)GlobalLock (Handle);  
		if (!CurView->FileFactor) CurView->FileFactor=1;
TryAgain: 
		iPoly = 1;
		LastOut = FALSE;
		if (hPolyPartLen && ReduceNumPoints)
		{
			pPartLen = (LPWORD)GlobalLock (hPolyPartLen);
			LastPartPoint = *pPartLen-1;
			GlobalUnlock (hPolyPartLen);
		}
		else
			LastPartPoint = npnts-1;
		/*if (ReduceNumPoints)
			NewPnts = 0;
		else
			NewPnts = npnts;*/   
		NewPnts = 0;
		for (i=0,lpPntNew = lpNewPoints;i<npnts;i++,lpPoints++)
		{   
			*lpPntNew = POINTStoPOINT(*lpPoints);
			if (CurView->FileProjectionType)
				ProjectFilePt (lpPntNew);
			lpPntNew->x /= CurView->FileFactor;
			lpPntNew->y /= CurView->FileFactor; 
			if (ReduceNumPoints)
			{
				if (LinkPoint) 
				{
					LastPoint = *lpPntNew++;  
					NewPnts++;
					FirstPartPoint = TRUE; 
					LinkPoint = FALSE;
					LastOut = FALSE;
				}
				else if (FirstPartPoint) 
				{
					LastPoint = *lpPntNew++;  
					NewPnts++;
					FirstPartPoint = FALSE; 
					LastOut = FALSE;
				}
				else if (i == LastPartPoint)
				{   
					if (LastOut)
					{
						NewPnts++;
						NextPoint = *lpPntNew;
						*lpPntNew++ = OutPoint;
						*lpPntNew = NextPoint;
					}
					LastPoint = *lpPntNew++;  
					NewPnts++;
					LastOut = FALSE;
					if (iPoly < nPoly)
					{
						pPartLen = (LPWORD)GlobalLock (hPolyPartLen);
						pPartLen += iPoly;
						LastPartPoint = i + *pPartLen;
						if (iPoly > 1)
						{
							LastPartPoint++; 
							LinkPoint = TRUE;
						}
						iPoly++; 
						GlobalUnlock (hPolyPartLen);
					}
				}
				else if (Attempts)
				{   
				
					if (idist (*lpPntNew,LastPoint) > SkipDist)
					{
						NewPnts++;
						LastPoint = *lpPntNew++; 
					} 
					else
						ii=1;
				} 
				else
				{ 
					Rect.left = min (lpPntNew->x,LastPoint.x);
					Rect.right = max (lpPntNew->x,LastPoint.x);
					Rect.top = min (lpPntNew->y,LastPoint.y);
					Rect.bottom = max (lpPntNew->y,LastPoint.y); 
					if (IntersectRect (&NewRect,&Rect,&BoundsRect))
					{   
						if (LastOut)
						{
							NewPnts++;
							NextPoint = *lpPntNew;
							*lpPntNew++ = OutPoint;
							*lpPntNew = NextPoint;
						}
						NewPnts++;
						LastPoint = *lpPntNew++;
						LastOut = FALSE;
					}
					else
					{
						OutPoint = *lpPntNew;
						LastOut = TRUE;      
					}
				}
			}
			else
			{
				if (!i || (lpPntNew->x != (lpPntNew-1)->x || lpPntNew->y != (lpPntNew-1)->y))  
				{
					lpPntNew++;	       
					NewPnts++;
				}
			}
		} 
		if (NewPnts > MaxDisplayPoints && Attempts < 8)
		{
			SkipDist*=2; 
			Attempts++;    
			lpPoints = lpPointsIn;           //lpNewPoints[2]
			goto TryAgain;
		}
		if (DoGraphics) 
		{
		    SaveDC (hDC); 
		    AreaIsNull = SetAreaPenAndBrush (hDC,0,desc,ItemIsHighlighted,ShowBorder,&hPen,&hBrush,pBorderSymNum);
		    if (hBrush)
				hOldBrush = SelectObject (hDC,hBrush); 
			else if (ItemIsHighlighted)
				AreaIsNull = FALSE;
			if ((!hPolyPartLen || nPoly < 2) && hPen) 
			{
				hOldPen = SelectObject (hDC,hPen); 
				if (!hOldPen)
					hOldPen = GetStockObject(BLACK_PEN);
			}
			else
				hReturnPen = hPen;
			if (DoFill)
				i = Polygon (hDC,lpNewPoints,(short)NewPnts); 
			if (hBrush)
			{ 
				if (hOldBrush)
					SelectObject (hDC,hOldBrush); 
				else
			    	SelectObject(hDC, GetStockObject(BLACK_BRUSH));
				if (hBrush != HighlightBrush && hBrush != GetStockObject(NULL_BRUSH))
					GSSiDeleteObject (&hBrush);
			}
			if (hOldPen)
				SelectObject (hDC,hOldPen);
			if (hPen != HighlightPen && hPen != (HPEN)HighlightBrush && hPen != (HPEN)GetStockObject(NULL_PEN) && hReturnPen != hPen)
				GSSiDeleteObject (&hPen);
			RestoreDC (hDC,-1);
		} 
		GSSiGlobUlFree (&Handle);
	}
{
#if ENABLETRACE
GSSiExitProg (990);
#endif
	return (hReturnPen);
}
	
#if ENABLETRACE
}
#endif
}

HANDLE GWPolygon (HDC hDC, HPPOINTS lpPoints, long npnts, int nPoly, HANDLE hPolyPartLen, int desc,BOOL ShowBorder,BOOL DoFill,LPINT pBorderSymNum)
#if ENABLETRACE
{GSSiEnterProg (1383);
#endif
{
	HANDLE	rtn;

	if (CurView->Rotation || UseShortSymbols)
	{
		HANDLE	Handle = GSSiGlobAlloc ( 756,GMEM_MOVEABLE,(long)npnts*sizeof(DPOINT));
		HPDPOINT pPolyPointsD = (HPDPOINT) GlobalLock (Handle);
		UINT	i;

		for (i=0;i<npnts;i++)
			pPolyPointsD[i] = FilePtToBasePt (POINTStoPOINT(lpPoints[i]));   
		SaveDC (hDC);
		SetDisplayMode (hDC, GF_TEXTMODE);
		rtn = GWPolygonD (hDC, pPolyPointsD,  npnts, nPoly, hPolyPartLen, desc,ShowBorder,DoFill,pBorderSymNum);
		RestoreDC (hDC,-1);
		GSSiGlobUlFree (&Handle);
	}
	else
		rtn = GWPolygon2 (hDC, lpPoints,  npnts, nPoly, hPolyPartLen, desc,ShowBorder,DoFill,pBorderSymNum);
{
#if ENABLETRACE
GSSiExitProg (1383);
#endif
	return rtn;
}
	
#if ENABLETRACE
}
#endif
}

BOOL SetAreaPenAndBrush (HDC hDC,LPSYMBOL pSym,int desc,BOOL ItemIsHighlighted,BOOL ShowBorder,LPHPEN phPen,LPHBRUSH phBrush,LPINT pBorderSymbolNum)
{   
//returns TRUE if null brush
	HANDLE	hSymbol=0;     
	short	fac; 
	BOOL	rtn=FALSE;
	double	Width=1;
	
	
	*phPen = 0;
	if (pBorderSymbolNum)
		*pBorderSymbolNum = 0;
	if (phBrush)
		*phBrush = 0;
    if (ItemIsHighlighted)
	{
		if (HighlightWidth < 0)//shows highlighted area with border line 
		{
			*phPen = (HPEN)HighlightBrush; 
			if (phBrush && !GetTypeVisibility(11) && !GetTypeVisibility(12))
				*phBrush = 0;
		}
    }
    else
    {   
		if (UseDGNColors && (MapType == MT_DGN7 || MapType == MT_DGN8))
    	{
			if (phBrush && !hTempBrush && !HaveVarFillColor)
			{
				if (DGNFillColor >= 0) 
				{
					*phBrush = CreateGMBrush (DGNFillColor,desc,hDC);  
					if (!*phBrush)
						rtn = TRUE;
				}
				else if (!GetBit (7,(LPSTR)&CurVis->WantType[7]))
					*phBrush = 0;
			} 
			if (!ShowBorder)
				*phPen = GetStockObject(NULL_PEN);
			else if (HaveVarFillColor)
				*phPen = CreatePen (PS_SOLID,0,ConvertColor(ColorWOWidth(GlobalColors[0]),desc));
			else  
				*phPen = CreatePen (PS_SOLID,0,ConvertColor(DGNColor,desc));  
    	}
    	else// if (!desc /*|| SymbolIsVisible (desc)*/ || HaveVarFillColor)
    	{   
    		if (desc)
    		{
    			if ((hSymbol = GetDictSymDesc (desc,0)))
    				pSym = (LPSYMBOL)GlobalLock (hSymbol);
    			else
    				return FALSE;
    		}	
			if (pSym && pSym->NumElements && pSym->hElement)
			{
				LPHANDLE phElement = &pSym->hElement;
				int	i;

				for (i=0;i<pSym->NumElements;i++,phElement++)
				{
					LPELEMENT pElement = (LPELEMENT)GlobalLock (*phElement);
    					
					if (pElement->Type == 3)
					{
						if (phBrush)
						{
							if (!pSym->InVisible && (!(pElement->FillColorType == SVVARCOLOR) || (!hTempBrush && !HaveVarFillColor)))
							{
    							*phBrush = CreateGMBrush (pElement->FillColor,desc,hDC); 
    							if (!*phBrush)
    								rtn=TRUE;
    						}
							else if ((pElement->FillColorType == SVVARCOLOR) && HaveVarFillColor)
							{
    							*phBrush = CreateGMBrush (GlobalColors[0],desc,hDC); 
    							if (!*phBrush)
    								rtn=TRUE;
    						}
						}
    					if (!CurVis || !CurVis->WantType[9])
							fac = 0; 
						else
							fac = 1;


						Width = pElement->Width;  
						if (TempLineWidth)
							Width = TempLineWidth;
						if (!ItemSymbolWidth) 
						{
							if (Width > 256)
								Width = 1;  
							if (pSym->BaseScale == 15)
								Width = ((pSym->VSize * Width)/100) * FTM * BaseDistToWinDist * PenWidthFactor;
							else if (Width < 0)
								Width = -Width * FTM * BaseDistToWinDist * PenWidthFactor;
							else
							{ 
								fac = ThemeWidthFactor * PenWidthFactor;
							}
						}
						else if (ItemSymbolWidth > 0)
							Width = ItemSymbolWidth * PenWidthFactor * DeviceToScreenFactor();
						else
							Width = -ItemSymbolWidth * BaseDistToWinDist * PenWidthFactor;

						if (!hSpecialPen && (!ShowBorder || pElement->LineColorType ==  SVNULLCOLOR))
		//5/18/06 always show border if defined in dict				if (!hSpecialPen && (!ShowBorder || pElement->LineColor ==  SVNULLCOLOR))   
		//				if (!hSpecialPen && (pElement->LineColorType ==  SVNULLCOLOR))
							*phPen = GetStockObject(NULL_PEN);
						else if (!hSpecialPen && (pElement->LineColorType == SVVARCOLOR && HaveVarFillColor))
							*phPen = CreatePen (PS_SOLID,(int)IDNINT(Width*DeviceToScreenFactor()*fac),ConvertColor(ColorWOWidth(GlobalColors[0]),desc));  
						else if (!hSpecialPen)
							*phPen = CreatePen (PS_SOLID,(int)IDNINT(Width*DeviceToScreenFactor()*fac),ConvertColor(pElement->LineColor,desc)); 
					}
					else if (pElement->Type == 4)
					{
						if (*phPen != GetStockObject(NULL_PEN))
							GSSiDeleteObject (phPen);
						if (pElement->Width < 0 && pBorderSymbolNum)
						{
							*phPen = GetStockObject(NULL_PEN);
							*pBorderSymbolNum = -pElement->Width;
							if (pElement->Reverse)
								*pBorderSymbolNum += 10000;
						}
						else
						{
							if (!hSpecialPen && (!ShowBorder || pElement->LineColorType ==  SVNULLCOLOR))
								*phPen = GetStockObject(NULL_PEN);
							else if (!hSpecialPen && (pElement->LineColorType == SVVARCOLOR && HaveVarFillColor))
								*phPen = CreatePen (PS_SOLID,(int)IDNINT(pElement->Width*DeviceToScreenFactor()*fac),ConvertColor(ColorWOWidth(GlobalColors[0]),desc));  
							else if (!hSpecialPen)
								*phPen = CreatePen (PS_SOLID,(int)IDNINT(max (pElement->Width,0)*DeviceToScreenFactor()*fac),ConvertColor(pElement->LineColor,desc));
						}
					}
					GlobalUnlock (*phElement);
				}
			}
			if (hSymbol)
			{
				GlobalUnlock (hSymbol); 
				DestroySymbol (hSymbol); 
			}
   		}
    } 
    return rtn;
}

BOOL ShowNodePoints (HDC hDC,long nPnts,HPDPOINT pPoints,HANDLE hUnSplinedPoly,long nUnSplinedPoints,BOOL Closed)
{   
	long	i;    
	COLORREF	SelectedColor=RGB(255,0,0);
	COLORREF	NonSelectedColor = 0, Color; 
	short	Size;
	
	if (hUnSplinedPoly)
	{
		pPoints = (HPDPOINT)GlobalLock (hUnSplinedPoly);
		nPnts = nUnSplinedPoints;
	}
	switch (NumSelectedNodes)
	{
		case 0:
			SelectedNodes[0]=SelectedNodes[1]=-1; 
			break;
		case 1:
			SelectedNodes[1]=SelectedNodes[0];
			break;
		case 2:
			break;
	}
	DisplayMarkers = TRUE; 
	for (i=0;i<nPnts;i++) 
	{
		if (SelectedNodes[0] <= SelectedNodes[1])
		{
			if (i >= SelectedNodes[0] && i <= SelectedNodes[1])
				Color = SelectedColor;
			else
				Color = NonSelectedColor;
		}
		else
		{
			if (i >= SelectedNodes[0] || i <= SelectedNodes[1])
				Color = SelectedColor;
			else
				Color = NonSelectedColor;
		}
		if (Color == NonSelectedColor)
			Size = 3;
		else
			Size = 4;
		DisplayMarker (pPoints[i],Size,0,0,0,Color,TRUE,FALSE,0,0,0,0,0); 
		if (i)
			SimplePointer (CurView->hDC, &pPoints[i-1], &pPoints[i],1,0,7,0);
		else if (Closed)
			SimplePointer (CurView->hDC, &pPoints[nPnts-1], &pPoints[i],1,0,7,0);
	}
	DisplayMarkers = FALSE;  
	if (hUnSplinedPoly)
		GlobalUnlock (hUnSplinedPoly);
	         
	return TRUE;
}

BOOL ConvertBMPColors (HDIB32 hDib)
{   
	long	NumColors,i;   
	RGBQUAD	Palette[256]; 
	BOOL	rtn=FALSE;
	
	if (!CurView || !CurView->HalfTone)
		return FALSE;
	if (GetDibPalette (hDib,&NumColors,Palette))
	{
		for (i=0;i<NumColors;i++)
			Palette[i] = RGBQUADFromCOLORREF (ConvertColor (COLORREFFromRGBQUAD(Palette[i]),-1));
		SetDibPalette (hDib,NumColors,Palette);
		rtn = TRUE;
	}
	return rtn;
}

HANDLE GWPolygonD (HDC hDC, HPDPOINT lpPoints, long npnts, int nPoly, HANDLE hPolyPartLen,int desc,BOOL ShowBorder,BOOL DoFill,LPINT pBorderSymNum)
#if ENABLETRACE
{GSSiEnterProg (991);
#endif
{	
	HANDLE Handle;   
	HANDLE	hReverse=0;
	HANDLE	hSymbol;
	LPSYMBOL	pSym;
	HPPOINT	lpNewPoints, lpPntNew;
	HPDPOINT lpPointsIn=lpPoints;
	DWORD	i, NewPnts, LastPartPoint; 
	short	Attempts=0, iPoly;  
	BOOL	ReduceNumPoints=ReducePolyPnts, FirstPartPoint=TRUE, LinkPoint=FALSE;
	POINT	LastPoint, NextPoint, OutPoint;
	double	SkipDist=1;  
	LPWORD	pPartLen; 
	RECT	Rect, NewRect, BoundsRect;
	BOOL	LastOut;
	HBRUSH	hOldBrush=0, hBrush=0;
	HPEN	hOldPen=0, hPen=0, hReturnPen=0; 
	short	ii=0; 
	RECT	AllPointsRect;   
	BOOL	AreaIsNull = FALSE, AddPoint=FALSE;   
	LPSTR	pName;
	HANDLE	hPolyPartStart=0;
	int		ipps;
	static	BOOL	doNotRemoveDups=TRUE;
	
	if (pBorderSymNum)
		*pBorderSymNum = 0;
	if (hDC && CompareDC)
		hDC = CompareDC;
	if (npnts > (long)USHRT_MAX)
		ii=1;
	if (PickingByRefno)
	{   
		npnts = labs (npnts);
		PickList[0].BeginPoint = lpPoints[0];
		PickList[0].EndPoint = lpPoints[npnts-1];
		PickList[0].NumPoints = npnts;
		PickList[0].Area = ComputeAreaAreaD (lpPoints,npnts,&PickList[0].Length);
{
#if ENABLETRACE
GSSiExitProg (991);
#endif
		return (0);  
}   
	}
	TotPointsProcessed += npnts;  
	if (!Display || !hDC || !npnts || ShowBadSyms)
{
#if ENABLETRACE
GSSiExitProg (991);
#endif
		return (0);  
}
//    if (!GetTypeVisibility(8) && SymIsInvis (CurrentDesc))
//    	retrn (0);  
	if (!nPoly && !SameDPoint (&lpPoints[0],&lpPoints[labs(npnts)-1]))
    	AddPoint = TRUE;
	if (npnts < 0)
	{   
		npnts = -npnts;  
		hReverse =  ReversePoints3 (npnts,lpPoints);
		lpPoints = (HPDPOINT)GlobalLock (hReverse);
    } 
    if (SnapPolygons) 
    {   
    	double	SnapTol = GetGlobalDVal2 ("[%SNAPPOLYTOL]",0);
		SnapPolyPoints (npnts,lpPoints,SnapTol);
    	RemoveDupPolyPoints (&npnts,lpPoints, P_TOL);
	}
    SaveDC (hDC);  
    if (desc && desc == PreBuiltMapDesc)
    {
        char	ImageFileName[256];
        POINT	pt; 
        LPSTR	pBS;  
        MNMXCORD	Bounds;
		int		ImageFormat;
		HANDLE	pBMP;
        
		RectInit (&Rect);  
		for (i=0;i<4;i++)
		{
			pt = BasePtToWinPt (&lpPointsIn[i]);
			AddPointToRect (pt,&Rect);
		}   
        _fstrcpy (ImageFileName,PltName);
        ExpandText (ImageFileName);
        pBS = _fstrrchr (ImageFileName,'\\');
		if (GetGlobalBVal2 ("[USE8BIT]",TRUE))
			_fstrcpy (pBS,"\\images_cvt.gmd");
		else
			_fstrcpy (pBS,"\\images.gmd");
        if (!hTempImageFile)
        {
        	hTempImageFile = GSSiGlobAlloc (0,GMEM_MOVEABLE,256);
        	pName = GlobalLock (hTempImageFile); 
       		GSSiGetTempFileName (0,"gml",0,pName);
			strcpy (pName,"c:\\temp\\mapimage.bmp");
        }
        else
        	pName = GlobalLock (hTempImageFile); 
		if ((ImageFormat = MapImageToFile (ImageFileName,CurrentRefno,pName,&Bounds,&pBMP)))
		{ 
			switch (ImageFormat)
			{
			case 1:
				{
		        	HDIB32 hDib=BMPHandleFromEXT (pName);
					LPBITMAPINFOHEADER pbi;
					RGBQUAD	*pal;
/*					HDIB32 hDib24 = FreeImage_ConvertTo24Bits (hDib);
					HDIB32	hDib8 = FreeImage_ColorQuantize (hDib24,FIQ_NNQUANT);
					long	lPalette;
					RGBQUAD	pal[1024];

					GMFIBMPHandleToEXT ("c:\\temp\\mapimage24bit.bmp",hDib,0);
					GMFIBMPHandleToEXT ("c:\\temp\\mapimage8bit.bmp",hDib8,0);
					GMDestroyDIB32 (hDib24);
					GMDestroyDIB32 (hDib8);
					hDib8 = Create8BitBMP (hDib, pal, &lPalette);
					GMFIBMPHandleToEXT ("c:\\temp\\mapimage8bit_mine.bmp",hDib8,0);
					GMDestroyDIB32 (hDib8);*/
		     
	/*	    {
		    	DPOINT p1,p2;
		    	double dist;

		    	p1.x = Bounds.xmn;
		    	p2.x = Bounds.xmx;
		    	p1.y = p2.y = (Bounds.ymn + Bounds.ymx) / 2;
		    	dist = (ArcDistance (p1,p2) * MFT)/5280;
		    	p1.y = Bounds.ymn;
		    	p2.y = Bounds.ymx;
		    	p1.x = p2.x = (Bounds.xmn + Bounds.xmx) / 2;
		    	dist = (ArcDistance (p1,p2) * MFT)/5280;
		    }*/
		    	 
					SetDisplayMode (hDC, GF_TEXTMODE);
					BoundsToWinRect (&Bounds,&BoundsRect); 
					ConvertBMPColors (hDib);
					pbi = FreeImage_GetInfoHeader (hDib);
					pal = FreeImage_GetPalette (hDib);
					LoadHBSymList ("");
					ConvertHBirdColors (pbi->biClrUsed,pal);
	    			DisplayBMInRect32 (hDC,hDib,Rect,FALSE); 
	    			GMDestroyDIB32 (hDib);
				}
				break;
			default:
			case 0:
				{
				}
				break;
			}
        } 
        GlobalUnlock (hTempImageFile);
		RestoreDC (hDC,-1);
{
#if ENABLETRACE
GSSiExitProg (991);
#endif
	return (0);
}
	
	}
    AreaIsNull = SetAreaPenAndBrush (hDC,0,desc,ItemIsHighlighted,ShowBorder,&hPen,&hBrush,pBorderSymNum);
	if (ii==123)
		hBrush = GetStockObject (NULL_BRUSH);
	ii=0;
    if (hBrush)
		hOldBrush = SelectObject (hDC,hBrush); 
	else if (ItemIsHighlighted)
		AreaIsNull = FALSE;
	if ((!hPolyPartLen || nPoly < 2)) 
	{
		if (!hPen)
			hOldPen = SelectObject (hDC,GetStockObject (NULL_PEN));
		else
			hOldPen = SelectObject (hDC,hPen); 
		if (!hOldPen)
			hOldPen = GetStockObject(BLACK_PEN);
		hReturnPen = hPen;
	}
	else
		hReturnPen = hPen;
	SetDisplayMode (hDC, GF_TEXTMODE);
	if (hDC && hDC == CompareDC)
	{
	    GSSiDeleteObject(&CurView->hRgn);
		CurView->hRgn = CreateVPRgn(FALSE,FALSE);
	  	SelectClipRgn (hDC,CurView->hRgn);
	  	GSSiDeleteObject(&CurView->hRgn);
	}
	MostPoints = max (MostPoints,npnts);
	if (npnts > MaxDisplayPoints)   
	{
		if (CurrentItemMinMax.xmn/CurView->FileFactor >= CurView->Bounds.xmn &&
			CurrentItemMinMax.ymn/CurView->FileFactor >= CurView->Bounds.ymn &&
			CurrentItemMinMax.xmx/CurView->FileFactor <= CurView->Bounds.xmx &&
			CurrentItemMinMax.ymx/CurView->FileFactor <= CurView->Bounds.ymx)
			Attempts = 1;
		else
			Attempts = 0; 
		SkipDist = 2;
		ReduceNumPoints = TRUE; 
	}
	if (!ReduceNumPoints) 
	{
		if (DoGraphics)
		{
			POINT	FirstPoint,LastPoint;
			long	np=1;

			if (hPolyPartLen)
			{
				LPINT	pPartLen = (LPINT)GlobalLock (hPolyPartLen);

				hPolyPartStart = GSSiGlobAlloc (1771,GMEM_MOVEABLE,nPoly*sizeof(int)+4);
				pPolyPartStart = GlobalLock (hPolyPartStart);
				pPolyPartStart[0] = pPartLen[0]-1;
				for (ipps = 1;ipps < nPoly;ipps++)
					pPolyPartStart[ipps] = pPolyPartStart[ipps-1] + pPartLen[ipps] + 1;

				GlobalUnlock (hPolyPartLen);
				nPolyPartStart = nPoly;
			}
	
			Handle = GSSiGlobAlloc ( 770,GMEM_MOVEABLE,(npnts+1) * sizeof(POINT)); 
			lpPntNew = lpNewPoints = (HPPOINT)GlobalLock (Handle);  
			*lpPntNew = BasePtToWinPt (lpPoints++);  
			if (abs(lpPntNew->x) == INT_MAX ||
				abs(lpPntNew->y) == INT_MAX)
			{   
				GSSiGlobUlFree (&Handle);
				ReduceNumPoints = TRUE; 
				goto DoReduce;
			}
			FirstPoint = LastPoint = *lpPntNew++;
			for (i=1;i<npnts;i++)
			{
				*lpPntNew = BasePtToWinPt (lpPoints++);   
				if (doNotRemoveDups || (lpPntNew->x != LastPoint.x || lpPntNew->y != LastPoint.y))
				{   
					LastPoint = *lpPntNew;
					lpPntNew++;
					np++;
				}
				else if (nPolyPartStart)
				{
					for (ipps = 0;ipps < nPolyPartStart;ipps++)
					{
						if (pPolyPartStart[ipps] > np)
							pPolyPartStart[ipps]--;
					}
				}
			}
			if (np > 1) 
			{   
				if (AddPoint) 
				{
					*lpPntNew++ = FirstPoint;
					np++;
				}
				if (ShowLinkLines)
				{
					HPEN hPen = SelectObject (hDC,GetStockObject(BLACK_PEN));

					i = Polyline (hDC,lpNewPoints,np); 
					SelectObject (hDC,hPen);
				}
				else if (AreaIsNull && !ItemIsHighlighted)
				{
					if (DoFill && nPoly < 2)
						i = Polyline (hDC,lpNewPoints,np); 
				}
				else if (DoFill)
				{
					HBRUSH	hBrush = GetCurrentObject(hDC, OBJ_BRUSH); 
					LOGBRUSH LogBrush;
					int ln = GetObject(hBrush, sizeof(LOGBRUSH), &LogBrush);

					switch (LogBrush.lbHatch)
					{
					case BS_HATCHED:
						ii = 1;
						break;
					case BS_PATTERN:
						ii = 1;
						break;
					case BS_HOLLOW:
						ii = 1;
						break;
					}
					i = Polygon(hDC, lpNewPoints, np);
				}
				if (DisplayAreaPoints)
				{   
					for (i=0;i<npnts;i++)
						DisplayPoint (hDC,lpNewPoints[i]); 
				}
			}
			GSSiGlobUlFree (&Handle);
		}
	}
	else                                 
	{
DoReduce:   
		BoundsRect = CurView->Rect;
		Handle = GSSiGlobAlloc ( 771,GMEM_MOVEABLE,(long)(npnts+1) * sizeof(POINT)); 
		lpNewPoints = (HPPOINT)GlobalLock (Handle);  
		if (!CurView->FileFactor) CurView->FileFactor=1;
TryAgain: 
		iPoly = 1;
		LastOut = FALSE;
		if (hPolyPartLen && ReduceNumPoints)
		{
			pPartLen = (LPWORD)GlobalLock (hPolyPartLen);
			LastPartPoint = *pPartLen-1;
			GlobalUnlock (hPolyPartLen);
		}
		else
			LastPartPoint = npnts-1;
		if (ReduceNumPoints)
			NewPnts = 0;
		else
			NewPnts = npnts;
		RectInit (&AllPointsRect);  
		lpPoints = lpPointsIn;
		for (i=0,lpPntNew = lpNewPoints;i<npnts;i++,lpPoints++)
		{
			*lpPntNew = BasePtToWinPt (lpPoints);
			AddPointToRect (*lpPntNew,&AllPointsRect);   
			if (ReduceNumPoints)
			{
				if (LinkPoint) 
				{
					LastPoint = *lpPntNew++;  
					NewPnts++;
					FirstPartPoint = TRUE; 
					LinkPoint = FALSE;
					LastOut = FALSE;
				}
				else if (FirstPartPoint) 
				{
					LastPoint = *lpPntNew++;  
					NewPnts++;
					FirstPartPoint = FALSE; 
					LastOut = FALSE;
				}
				else if (i == LastPartPoint)
				{   
					if (LastOut)
					{
						NewPnts++;
						NextPoint = *lpPntNew;
						*lpPntNew++ = OutPoint;
						*lpPntNew = NextPoint;
					}
					LastPoint = *lpPntNew++;  
					NewPnts++;
					LastOut = FALSE;
					if (iPoly < nPoly)
					{
						pPartLen = (LPWORD)GlobalLock (hPolyPartLen);
						pPartLen += iPoly;
						LastPartPoint = i + *pPartLen;
						if (iPoly > 1)
						{
							LastPartPoint++; 
							LinkPoint = TRUE;
						}
						iPoly++; 
						GlobalUnlock (hPolyPartLen);
					}
				}
				else if (Attempts)
				{   
				
					if (idist (*lpPntNew,LastPoint) > SkipDist)
					{
						NewPnts++;
						LastPoint = *lpPntNew++; 
					}
				} 
				else
				{ 
					Rect.left = min (lpPntNew->x,LastPoint.x);
					Rect.right = max (lpPntNew->x,LastPoint.x);
					Rect.top = min (lpPntNew->y,LastPoint.y);
					Rect.bottom = max (lpPntNew->y,LastPoint.y); 
					if (IntersectRect (&NewRect,&Rect,&BoundsRect))
					{   
						if (LastOut)
						{
							NewPnts++;
							NextPoint = *lpPntNew;
							*lpPntNew++ = OutPoint;
							*lpPntNew = NextPoint;
						}
						NewPnts++;
						LastPoint = *lpPntNew++;
						LastOut = FALSE;
					}
					else
					{
						OutPoint = *lpPntNew;
						LastOut = TRUE;      
					}
				}
			}
			else
				lpPntNew++;	
		} 
		if (NewPnts > MaxDisplayPoints && Attempts < 8)
		{
			SkipDist*=2; 
			Attempts++;    
			goto TryAgain;
		}
		if (DoGraphics && IntersectRect (&NewRect,&AllPointsRect,&BoundsRect))
		{
			if (ShowLinkLines)
				i = Polyline (hDC,lpNewPoints,NewPnts); 
			else if (AreaIsNull && !ItemIsHighlighted)
			{
				if (nPoly < 2)
					i = Polyline (hDC,lpNewPoints,NewPnts); 
			}
			else 
				i = Polygon (hDC,lpNewPoints,NewPnts); 
			if (DisplayAreaPoints)
			{   
				for (i=0;i<npnts;i++)
					DisplayPoint (hDC,lpNewPoints[i]); 
			}
		} 
		GSSiGlobUlFree (&Handle);
	}
	if (hBrush)
	{ 
		if (hOldBrush)
			SelectObject (hDC,hOldBrush); 
		else
	    	SelectObject(hDC, GetStockObject(BLACK_BRUSH));
		if (hBrush != HighlightBrush && hBrush != GetStockObject(NULL_BRUSH))
			GSSiDeleteObject (&hBrush);
	}
	if (hOldPen)
		SelectObject (hDC,hOldPen);
	if (hPen != HighlightPen && hPen != (HPEN)HighlightBrush && hPen != (HPEN)GetStockObject(NULL_PEN) && hReturnPen != hPen)
		GSSiDeleteObject (&hPen); 
	if (hDC == CompareDC)
	{
		GSSiDeleteObject (&hReturnPen); 
	}
	RestoreDC (hDC,-1);
	GSSiGlobUlFree (&hReverse);
	GSSiGlobUlFree (&hPolyPartStart);
	nPolyPartStart = 0;
{
#if ENABLETRACE
GSSiExitProg (991);
#endif
	return (hReturnPen);
}
	
#if ENABLETRACE
}
#endif
}

/*HANDLE GWPolygonDFast (HDC hDC, HPDPOINT lpPoints, long npnts, short nPoly, HANDLE hPolyPartLen,int desc,BOOL ShowBorder)
#if ENABLETRACE
{GSSiEnterProg (991);
#endif
{	
	HANDLE Handle;   
	HANDLE	hReverse=0;
	HANDLE	hSymbol;
	LPSYMBOL	pSym;
	HPPOINT	lpNewPoints, lpPntNew;
	HPDPOINT lpPointsIn=lpPoints;
	DWORD	i, NewPnts, LastPartPoint; 
	short	Attempts=0, iPoly;  
	BOOL	ReduceNumPoints=ReducePolyPnts, FirstPartPoint=TRUE, LinkPoint=FALSE;
	POINT	LastPoint, NextPoint, OutPoint;
	double	SkipDist=1;  
	LPWORD	pPartLen; 
	RECT	Rect, NewRect, BoundsRect;
	BOOL	LastOut;
	HBRUSH	hOldBrush=0, hBrush=0;
	HPEN	hOldPen=0, hPen=0, hReturnPen=0; 
	short	ii; 
	RECT	AllPointsRect;   
	BOOL	AreaIsNull = FALSE, AddPoint=FALSE;   
	LPSTR	pName;
	
	if (npnts > (long)USHRT_MAX)
		ii=1;
	if (PickingByRefno)
	{   
		npnts = labs (npnts);
		PickList[0].BeginPoint = lpPoints[0];
		PickList[0].EndPoint = lpPoints[npnts-1];
		PickList[0].NumPoints = npnts;
		PickList[0].Area = ComputeAreaAreaD (lpPoints,npnts,&PickList[0].Length);
{
#if ENABLETRACE
GSSiExitProg (991);
#endif
		return (0);  
}   
	}
	TotPointsProcessed += npnts;  
	if (!Display || !hDC || !npnts || ShowBadSyms)
{
#if ENABLETRACE
GSSiExitProg (991);
#endif
		return (0);  
}
//    if (!GetTypeVisibility(8) && SymIsInvis (CurrentDesc))
//    	retrn (0);  
	if (!nPoly && !SameDPoint (&lpPoints[0],&lpPoints[npnts-1]))
    	AddPoint = TRUE;
	if (npnts < 0)
	{   
		npnts = -npnts;  
		hReverse =  ReversePoints3 (npnts,lpPoints);
		lpPoints = (HPDPOINT)GlobalLock (hReverse);
    } 
    if (SnapPolygons) 
    {   
    	double	SnapTol = GetGlobalDVal2 ("[%SNAPPOLYTOL]",0);
		SnapPolyPoints (npnts,lpPoints,SnapTol);
    	RemoveDupPolyPoints (&npnts,lpPoints, P_TOL);
	}
    SaveDC (hDC);  
    if (desc && desc == PreBuiltMapDesc)
    {
        char	ImageFileName[256];
        POINT	pt; 
        LPSTR	pBS;  
        MNMXCORD	Bounds;
        
		RectInit (&Rect);  
		for (i=0;i<4;i++)
		{
			pt = BasePtToWinPt (&lpPointsIn[i]);
			AddPointToRect (pt,&Rect);
		}   
        _fstrcpy (ImageFileName,PltName);
        ExpandText (ImageFileName);
        pBS = _fstrrchr (ImageFileName,'\\');
        _fstrcpy (pBS,"\\images.gmd");
        if (!hTempImageFile)
        {
        	hTempImageFile = GSSiGlobAlloc (0,GMEM_MOVEABLE,256);
        	pName = GlobalLock (hTempImageFile); 
       		GSSiGetTempFileName (0,"gml",0,pName);
        }
        else
        	pName = GlobalLock (hTempImageFile); 
		if (MapImageToFile (ImageFileName,CurrentRefno,pName,&Bounds))
		{ 
	    	double dist;
        	HDIB32 hDib=BMPHandleFromEXT (pName);
		     
		    {
		    	DPOINT p1,p2;
		    	
		    	p1.x = Bounds.xmn;
		    	p2.x = Bounds.xmx;
		    	p1.y = p2.y = (Bounds.ymn + Bounds.ymx) / 2;
		    	dist = (ArcDistance (p1,p2) * MFT)/5280;
		    	p1.y = Bounds.ymn;
		    	p2.y = Bounds.ymx;
		    	p1.x = p2.x = (Bounds.xmn + Bounds.xmx) / 2;
		    	dist = (ArcDistance (p1,p2) * MFT)/5280;
		    }
		    	 
			SetDisplayMode (hDC, GF_TEXTMODE);
			BoundsToWinRect (&Bounds,&Rect); 
			ConvertBMPColors (hDib);
	    	DisplayBMInRect32 (hDC,hDib,Rect,FALSE); 
	    	GMDestroyDIB32 (hDib);
        } 
        GlobalUnlock (hTempImageFile);
		RestoreDC (hDC,-1);
{
#if ENABLETRACE
GSSiExitProg (991);
#endif
	return (0);
}
	
	}
    AreaIsNull = SetAreaPenAndBrush (hDC,0,desc,ItemIsHighlighted,ShowBorder,&hPen,&hBrush);
    if (hBrush)
		hOldBrush = SelectObject (hDC,hBrush); 
	else if (ItemIsHighlighted)
		AreaIsNull = FALSE;
	if ((!hPolyPartLen || nPoly < 2) && hPen) 
	{
		hOldPen = SelectObject (hDC,hPen); 
		if (!hOldPen)
			hOldPen = GetStockObject(BLACK_PEN);
	}
	else
		hReturnPen = hPen;
	SetDisplayMode (hDC, GF_TEXTMODE);

	MostPoints = max (MostPoints,npnts);
	if (DoGraphics)
	{
		XFORM xForm;
		DPOINT	DPoint1,DPoint2;
		POINT	FirstPoint,LastPoint,Point;
		long	np=1;

		xForm =  SetXFORMFromTRANS (CurView->hTranBaseToVP);
		SetGraphicsMode(CurView->hDC, GM_COMPATIBLE);
		Point.x = Point.y = 0;
		DPoint1 = ScreenPtToBasePt (Point);
		SetWindowOrgEx  ( hDC, DPoint1.x, DPoint1.y,0 );
		SetViewportOrgEx( hDC, CurView->DrawRect.left, CurView->DrawRect.top,0 );
		Point.x = CurView->DrawRect.right;
		Point.y = CurView->DrawRect.bottom;
		DPoint2 = ScreenPtToBasePt (Point);
        SetWindowExtEx  ( hDC, DPoint2.x - DPoint1.x,DPoint2.y - DPoint1.y,0 ); 
       	if (!SetViewportExtEx( hDC, Point.x, Point.y,0 ))
        		ii=1;
//		SetWorldTransform(CurView->hDC, &xForm);
    	SetMapMode    ( hDC, MM_ISOTROPIC );


		Handle = GSSiGlobAlloc ( 770,GMEM_MOVEABLE,(npnts+1) * sizeof(POINT)); 
		lpPntNew = lpNewPoints = (HPPOINT)GlobalLock (Handle);  
		*lpPntNew = DPointToPoint (*lpPoints++);  
		FirstPoint = LastPoint = *lpPntNew++;
		for (i=1;i<npnts;i++,lpPoints++,lpPntNew++)
		{
			lpPntNew->x = lpPoints->x;   
			lpPntNew->y = lpPoints->y;   
		}
		np = npnts;
		if (np > 1) 
		{   
			if (AddPoint) 
			{
				*lpPntNew++ = FirstPoint;
				np++;
			}
			if (ShowLinkLines)
				i = Polyline (hDC,lpNewPoints,np); 
			else if (AreaIsNull && !ItemIsHighlighted)
			{
				if (nPoly < 2)
					i = Polyline (hDC,lpNewPoints,np); 
			}
			else 
				i = Polygon (hDC,lpNewPoints,np); 
			if (DisplayAreaPoints)
			{   
				for (i=0;i<npnts;i++)
					DisplayPoint (hDC,lpNewPoints[i]); 
			}
		}
		GSSiGlobUlFree (&Handle);
	}
	if (hBrush)
	{ 
		if (hOldBrush)
			SelectObject (hDC,hOldBrush); 
		else
	    	SelectObject(hDC, GetStockObject(BLACK_BRUSH));
		if (hBrush != HighlightBrush && hBrush != GetStockObject(NULL_BRUSH))
			GSSiDeleteObject (&hBrush);
	}
	if (hOldPen)
		SelectObject (hDC,hOldPen);
	if (hPen != HighlightPen && hPen != HighlightBrush && hPen != GetStockObject(NULL_PEN) && hReturnPen != hPen)
		GSSiDeleteObject (&hPen); 

	RestoreDC (hDC,-1);
	GSSiGlobUlFree (&hReverse);
{
#if ENABLETRACE
GSSiExitProg (991);
#endif
	return (hReturnPen);
}
	
#if ENABLETRACE
}
#endif
}
*/
   
BOOL PickPolygon (HPPOINTS lpPoints,long nPnts, double Offdist)
#if ENABLETRACE
{GSSiEnterProg (992);
#endif
{   
	HANDLE Handle=0;
	HPDPOINT	lpNewPoints;
	HPDPOINT	lpDpoints;  
	HPPOINTS		lpPointsIn=lpPoints;
	int	 Type, NearPoint; 
	DWORD	i;
	MNMXCORD	Rect;  
	DPOINT	BeginPoint, EndPoint,PickedPoint, LastPoint, PickPointW, NodePoint,FromPoint,ToPoint;  
	POINT	BeginPointFile,EndPointFile,PickedPointFile;
	double	Area=0,Perim=0, MinDist=DBL_MAX, Dist, AZ[3];    
	BOOL	Selected=FALSE, rtn, pina=TRUE;   
	POINT	PickPoint;
    
    if (PickNET)
{
#if ENABLETRACE
GSSiExitProg (992);
#endif
    	return FALSE;
}
    if (hHighlightArea)
{
#if ENABLETRACE
GSSiExitProg (992);
#endif
    	return (PickPolyInArea(0,lpPoints,nPnts,0));
}
//	PickPointW.x = (CurView->WBounds.xmn + CurView->WBounds.xmx) / 2;
//	PickPointW.y = (CurView->WBounds.ymn + CurView->WBounds.ymx) / 2;
	PickPointW = PickPointBase;
	PickPoint = BasePtToFilePt (PickPointW); 

    if (!PickingByRefno)
		pina = POINT_IN_AREAS (PickPoint, nPnts, lpPoints);
    if (PickPerim==1) 
    {
    	BOOL	rtn; 
    	long	np=nPnts;
    	
    	rtn = PickPolyline (lpPoints,nPnts,0,3,0,pina);
{
#if ENABLETRACE
GSSiExitProg (992);
#endif
   		return rtn;
}
    }
    if (!pina  && !(PickPerim == 2))
		goto Exit;
	BeginPointFile = EndPointFile = POINTStoPOINT(*lpPoints);
    Handle = GSSiGlobAlloc ( 772,GMEM_MOVEABLE,(long)nPnts*sizeof(DPOINT));
 	lpDpoints = (HPDPOINT) GlobalLock (Handle); 
 	lpNewPoints = lpDpoints;
	for (i=0;i<nPnts;i++,lpPoints++,lpDpoints++)
	{   
		*lpDpoints = FilePtToBasePt (POINTStoPOINT(*lpPoints));
		Dist = ldistp (PickPointW,*lpDpoints);  
		if (Dist < MinDist)
		{
			MinDist = Dist;
			NodePoint = *lpDpoints;
			NearPoint = i;
		}
		if (!i)
			BeginPoint = *lpDpoints;
		else  
		{
			Perim += ldistp(LastPoint,*lpDpoints);
			Area += ( LastPoint.y - lpDpoints->y) * (lpDpoints->x + LastPoint.x) / 2;
		}
		LastPoint = *lpDpoints;
	} 
	Perim += ldistp(LastPoint,BeginPoint);
	Area += (LastPoint.y - BeginPoint.y) * (BeginPoint.x + LastPoint.x) / 2;  
	EndPoint = LastPoint;
     
    if (PickingByRefno)
    	Selected = TRUE;
    else if (PickPerim==2) 
    {
    	BOOL	rtn; 
    	short	inc=1;
    	
    	if (pina)
    		inc++;
    	
    	Selected = PickPolyline (lpPointsIn,nPnts,0,-3,0,max(0,IDNINT(Offdist-inc)));
    	if (Selected)
    	{
			GSSiGlobUlFree (&Handle);
{
#if ENABLETRACE
GSSiExitProg (992);
#endif
			return (Selected);
}
    	}	
    	else
    		Selected = pina;	
    }
    else
    	Selected = TRUE;
    if (Selected)
	{   
		if (CurrentType == GF_TEXT) 
		{
			BeginPoint = EndPoint = FromPoint = ToPoint = LastElementBeginPoint;
			AZ[0]=AZ[1]=AZ[2]=LastElementAZ;          
			Type = 4;          
		}
		else  
		{
			FromPoint = ToPoint = NodePoint;
			Type = 3;
		}
		GetItemMinMax (&CurrentItemMinMax,&Rect);
		PickedPoint = PickPointW;
		PickedPointFile = BasePtToFilePt (PickPointW);
		PickListAdd (FileNum,SubFile,FileInIndex,ItemSeg,CurrentRefno,CurrentDesc,0,
					   -Type,0,Offdist,AZ,Perim,CurrentItem,CurElement,BeginPoint,EndPoint,PickedPoint,
					   NodePoint,NearPoint,
					   Area,&Rect,
					   BeginPointFile,EndPointFile,PickedPointFile,nPnts,NULL_ELEV,&FromPoint,&ToPoint);
	} 
Exit:
	GSSiGlobUlFree (&Handle);
{
#if ENABLETRACE
GSSiExitProg (992);
#endif
	return (Selected);
}
#if ENABLETRACE
}
#endif
} 

double GetTriangleElev (DPOINT PickPointW,HPDPOINT lpNewPoints,LPFLOAT pElev)
{
	double		A1 = getazd (&lpNewPoints[0],&PickPointW);
	double		A2 = getazd (&lpNewPoints[1],&lpNewPoints[2]);  
	double		d1, d2, elev1, Elev;
	DPOINT		IntPt;
	short		IRC = LIN_SEC (lpNewPoints[0].x,lpNewPoints[0].y,A1,
				   			   lpNewPoints[1].x,lpNewPoints[1].y,A2,
				   			   &IntPt.x, &IntPt.y);
	            if (IRC)
	            	IRC = 0;
	            else        
	            {
	            	d1 = ldistp (lpNewPoints[1],lpNewPoints[2]);
	            	d2 = ldistp (lpNewPoints[1],IntPt);
	            	elev1 = pElev[1] + (pElev[2] - pElev[1]) * (d2 / d1);
	            	d1 = ldistp (IntPt,lpNewPoints[0]);
	            	if (!d1)
	            		Elev = pElev[0];
	            	else
	            	{
	                	d2 = ldistp (IntPt,PickPointW);
	                	Elev = elev1 + (pElev[0] - elev1) * (d2 / d1); 
	                }
	            }
	            return Elev;
}

BOOL AddPointToProfile (double dist,double elev,HANDLE hProfilePoints,LPSHORT pNumProfilePoints)
{   
	HPDPOINT	Profile=(HPDPOINT)GlobalLock (hProfilePoints);
	BOOL		rtn=FALSE;
	USHORT		i;
	
	if (dist < 0 || dist > Profile[(*pNumProfilePoints)-1].x)
		goto Exit;
    rtn = TRUE;
    for (i=1;i<*pNumProfilePoints;i++)
    {
    	if (dist < Profile[i].x)
    	{
    		hmemmove ((HPSTR)&Profile[i+1],(HPSTR)&Profile[i],(long)((*pNumProfilePoints)-i)*sizeof(DPOINT));
    		Profile[i].x = dist;
    		Profile[i].y = elev;
    		(*pNumProfilePoints)++;
    		goto Exit;
    	} 
    }
Exit:
	GlobalUnlock (hProfilePoints);	
	return rtn;
}

short AddPolygonToProfile (HPDPOINT PolyPoints,long nPnts, HANDLE hElev,HANDLE hProfilePoints,LPSHORT pNumProfilePoints)
{
   	HPDPOINT	pRoute = (HPDPOINT)GlobalLock (hProfileRoute); 
	short		nAdded=0, IRC;  
	ULONG		iPoly, iPolyNext, iRoute; 
	double		A1, A2, RouteDist, dist, elev, dRoute, dPoly,dFromFirstPoly;
	DPOINT		IntPt;
	MNMXCORD	Bounds1, Bounds2;
	
	for (iPoly = 0;iPoly < nPnts; iPoly++)
	{   
		iPolyNext = iPoly + 1;
		if (iPolyNext == nPnts)
			iPolyNext = 0;
	    A1 = getazd (&PolyPoints[iPoly],&PolyPoints[iPolyNext]);
		DBoundsInit (&Bounds1);
		AddDPointToMinMax (&PolyPoints[iPoly],&Bounds1);
		AddDPointToMinMax (&PolyPoints[iPolyNext],&Bounds1);
		ExpandBounds (&Bounds1,P_TOL);
		dPoly = ldistpp (&PolyPoints[iPoly],&PolyPoints[iPolyNext]);
		RouteDist = 0;
		if (dPoly)
		for (iRoute = 0;iRoute < nProfileRoute-1;iRoute++)
		{   
			DBoundsInit (&Bounds2);
			AddDPointToMinMax (&pRoute[iRoute],&Bounds2);
			AddDPointToMinMax (&pRoute[iRoute+1],&Bounds2); 
			ExpandBounds (&Bounds2,P_TOL);  
			dRoute = ldistpp (&pRoute[iRoute],&pRoute[iRoute+1]);
			if (BoundsInBounds (&Bounds1,&Bounds2,1))
			{
				A2 = getazd (&pRoute[iRoute],&pRoute[iRoute+1]);
				IRC = LIN_SEC (PolyPoints[iPoly].x,PolyPoints[iPoly].y,A1,
							   pRoute[iRoute].x,pRoute[iRoute].y, A2,
							   &IntPt.x, &IntPt.y);
			  	if (!IRC)
			  	{   
			  		dFromFirstPoly = ldistpp (&PolyPoints[iPoly],&IntPt);
			  		if (dFromFirstPoly <= dPoly &&
			  			ldistpp (&PolyPoints[iPolyNext],&IntPt) <= dPoly &&
			  			ldistpp (&pRoute[iRoute],&IntPt) <= dRoute &&
			  			ldistpp (&pRoute[iRoute+1],&IntPt) <= dRoute)
			  			{
					  		LPFLOAT pElev = (LPFLOAT)GlobalLock (hElev);
					  		
					  		nAdded++;  
					  		dist = RouteDist + ldistpp (&pRoute[iRoute],&IntPt);
					  		elev = pElev[iPoly] + (pElev[iPolyNext] - pElev[iPoly]) * (dFromFirstPoly / dPoly);   
					  		GlobalUnlock (hElev);
					  		AddPointToProfile (dist,elev,hProfilePoints,pNumProfilePoints);
					  	}
			  	}
			}
			RouteDist += dRoute;
		}
	} 
	GlobalUnlock (hProfileRoute);  
	return nAdded;
}           

short AddPolySymbolToProfile (int idesc,short type,HPDPOINT PolyPoints,long nPnts, HANDLE hElev,HANDLE hProfileSymbols,LPSHORT pNumProfileSymbols)
{
   	HPDPOINT	pRoute = (HPDPOINT)GlobalLock (hProfileRoute); 
	short		nAdded=0, IRC;  
	ULONG		iPoly, iPolyNext, iRoute; 
	double		A1, A2, RouteDist, dist, elev, dRoute, dPoly,dFromFirstPoly;
	DPOINT		IntPt;
	MNMXCORD	Bounds1, Bounds2;  
	
	for (iPoly = 0;iPoly < nPnts; iPoly++)
	{   
		iPolyNext = iPoly + 1;
		if (iPolyNext == nPnts)
		{
			if (type)
				continue;
			iPolyNext = 0;
		}
	    A1 = getazd (&PolyPoints[iPoly],&PolyPoints[iPolyNext]);
		DBoundsInit (&Bounds1);
		AddDPointToMinMax (&PolyPoints[iPoly],&Bounds1);
		AddDPointToMinMax (&PolyPoints[iPolyNext],&Bounds1);
		ExpandBounds (&Bounds1,P_TOL);
		dPoly = ldistpp (&PolyPoints[iPoly],&PolyPoints[iPolyNext]);
		RouteDist = 0;
		if (dPoly)
		for (iRoute = 0;iRoute < nProfileRoute-1;iRoute++)
		{   
			DBoundsInit (&Bounds2);
			AddDPointToMinMax (&pRoute[iRoute],&Bounds2);
			AddDPointToMinMax (&pRoute[iRoute+1],&Bounds2); 
			ExpandBounds (&Bounds2,P_TOL);  
			dRoute = ldistpp (&pRoute[iRoute],&pRoute[iRoute+1]);
			if (BoundsInBounds (&Bounds1,&Bounds2,1))
			{
				A2 = getazd (&pRoute[iRoute],&pRoute[iRoute+1]);
				IRC = LIN_SEC (PolyPoints[iPoly].x,PolyPoints[iPoly].y,A1,
							   pRoute[iRoute].x,pRoute[iRoute].y, A2,
							   &IntPt.x, &IntPt.y);
			  	if (!IRC)
			  	{   
			  		LPPROFILESYMBOLS pProfileSymbols;
			  		
			  		dFromFirstPoly = ldistpp (&PolyPoints[iPoly],&IntPt);
			  		if (dFromFirstPoly <= dPoly &&
			  			ldistpp (&PolyPoints[iPolyNext],&IntPt) <= dPoly &&
			  			ldistpp (&pRoute[iRoute],&IntPt) <= dRoute &&
			  			ldistpp (&pRoute[iRoute+1],&IntPt) <= dRoute)
			  		{
				  		dist = RouteDist + ldistpp (&pRoute[iRoute],&IntPt);
			  			if (hElev)
			  			{
					  		LPFLOAT pElev = (LPFLOAT)GlobalLock (hElev);
					  		
					  		nAdded++;  
					  		elev = pElev[iPoly] + (pElev[iPolyNext] - pElev[iPoly]) * (dFromFirstPoly / dPoly);   
					  		GlobalUnlock (hElev);
					  	}
					  	else
					  	{   
					  		double	slope;
					  		
					  		if (!GetProfileElevAndSlope (dist,hCurProfile,nCurProfile,&elev,&slope,DBL_MAX))
					  			goto Next;	
					  	}
						pProfileSymbols = (LPPROFILESYMBOLS)GlobalLock(hProfileSymbols);
						for (int isym = 0; isym < *pNumProfileSymbols; isym++)
							if (CurrentRefno == pProfileSymbols[isym].refno)
								goto SkipSym;

						{
							pProfileSymbols[*pNumProfileSymbols].Point.x = dist;
							pProfileSymbols[*pNumProfileSymbols].SurfElev = elev;
							pProfileSymbols[*pNumProfileSymbols].Size = 0;
							if (idesc == GetDictSymbolNumber("ISD"))
							{
								double	e1, e2;
								char	str1[32], str2[32];

								GetValFromOpenFiles("INVERT_IN_NGVD29", str1, 32);
								GetValFromOpenFiles("INVERT_OUT_NGVD29", str2, 32);

								if (*str1 && *str2)
								{
									e1 = atof(str1) * FTM;
									e2 = atof(str2) * FTM;
									elev = e1 + (e2 - e1) * (dFromFirstPoly / dPoly);
									GetValFromOpenFiles("DIAMETER_INCH", str2, 32);
									pProfileSymbols[*pNumProfileSymbols].Size = (atof(str2) / 12) * FTM;
									idesc = -idesc;
								}
							}
							pProfileSymbols[*pNumProfileSymbols].Point.y = elev;
							pProfileSymbols[*pNumProfileSymbols].refno = CurrentRefno;
							pProfileSymbols[(*pNumProfileSymbols)++].idesc = idesc;
						}
						SkipSym:
				  		GlobalUnlock (hProfileSymbols);
				  	}
			  	}
			}
	Next:
			RouteDist += dRoute;
		}
	} 
	GlobalUnlock (hProfileRoute);  
	return nAdded;
} 

LPHANDLE GetPolygonPickAccelerator (int Opt) 
//0 = get accelerator , 1=init or reset, 2=close
{
#define	MAX_GPPA	256
	LPHANDLE pRtn = 0;
	static	HANDLE	hGPPA=0;
	static	int		nAccel=0;
	static	HANDLE	hAccel[MAX_GPPA];
	static	long	GPPARef[MAX_GPPA];
	static	long	GPPAnPnts[MAX_GPPA];
	LPSTR	pFile;
	UINT	i;

	switch (Opt)
	{
	default:
		if (hGPPA)
		{
			int	nPnts = abs (Opt);
			UINT	iMinPnts=0;

			if (nPnts > 10)
			{
				pFile = GlobalLock (hGPPA);
				for (i=0;i<nAccel;i++,pFile+=256)
				{
					if (CurrentRefno == GPPARef[i] && !stricmp (pFile,PltName))
					{
						GlobalUnlock (hGPPA);
						pRtn = &hAccel[i];
						goto Exit;
					}
					if (GPPAnPnts[i] < GPPAnPnts[iMinPnts])
						iMinPnts = i;
				}
				GlobalUnlock (hGPPA);
				pFile = (LPSTR) GlobalLock (hGPPA);
				if (nAccel == MAX_GPPA)
					i = iMinPnts;
				else
					i = nAccel++;
				GPPARef[i] = CurrentRefno;
				strcpy (pFile+i*256,PltName);
				hAccel[i] = 0;
				GPPAnPnts[i] = nPnts;
				pRtn = &hAccel[i];
				GlobalUnlock (hGPPA);
			}
		}
		break;
	case 1:
	case 2:
		GSSiGlobFree (&hGPPA);
		for (i=0;i<nAccel;i++)
			GSSiGlobFree (&hAccel[i]);
		nAccel = 0;
		if (Opt == 2)
			break;
		hGPPA = GSSiGlobAlloc (1602,GMEM_MOVEABLE,256*MAX_GPPA);
		break;
	}
Exit:
	return pRtn;
}

BOOL PickPolygonD (HPDPOINT lpDpoints,long nPnts, int nPoly,HANDLE hPolyPartLen,double Offdist,HANDLE hElev)
#if ENABLETRACE
{GSSiEnterProg (993);
#endif
{   POINT	PickPoint;
	HPDPOINT	lpNewPoints;
	int	 Type, NearPoint; 
	DWORD	i;
	MNMXCORD	Rect;    
	float	Elev=NULL_ELEV;
	DPOINT	BeginPoint, EndPoint,PickedPoint, LastPoint, PickPointW, NodePoint,FromPoint,ToPoint;  
	POINT	BeginPointFile,EndPointFile,PickedPointFile;
	double	Area=0,Perim=0, MinDist=DBL_MAX, Dist, AZ[3]={0,0,0};    
	BOOL	Selected=FALSE, pina=TRUE;
	BOOL	rtn;
	int		ipoly;
    
    if (PickNET)
{
#if ENABLETRACE
GSSiExitProg (993);
#endif
    	return FALSE;
}   
	if (hProfilePoints)
{
#if ENABLETRACE
GSSiExitProg (993);
#endif
    	return (AddPolygonToProfile(lpDpoints,nPnts,hElev,hProfilePoints,&NumProfilePoints));
}
	if (hProfileSymbols)
{
#if ENABLETRACE
GSSiExitProg (993);
#endif
return (AddPolySymbolToProfile(CurrentDesc, 0, lpDpoints, nPnts, hElev, hProfileSymbols, &NumProfileSymbols));
}
    if (hHighlightArea)
	{
		if (nPoly > 1)
		{
			HPDPOINT lpFirstPt = lpDpoints;
    		LPINT	pPartLen = (LPINT)GlobalLock (hPolyPartLen);

			for (ipoly = 0; ipoly < nPoly;ipoly++,pPartLen++)
			{
				rtn = PickPolyInAreaD(0, lpDpoints, *pPartLen, 0, 0, 0, nPoly, hPolyPartLen, nPnts, lpFirstPt);
				if (rtn)
				{
					break;
				}
				lpDpoints += *pPartLen;
				if (ipoly)
					lpDpoints++;
			}
			GlobalUnlock (hPolyPartLen);
		}
		else
			rtn = PickPolyInAreaD(0,lpDpoints,nPnts,0,0,0,0,0,0,0);
{
#if ENABLETRACE
GSSiExitProg (993);
#endif
    	return (rtn);
}   
	}
	DBoundsInit (&Rect);
//	PickPointW.x = (CurView->WBounds.xmn + CurView->WBounds.xmx) / 2;
//	PickPointW.y = (CurView->WBounds.ymn + CurView->WBounds.ymx) / 2;
	PickPointW = CurPickPoint;
    if (!PickingByRefno)
		pina = POINT_IN_AREAD (PickPointW, nPnts, lpDpoints,1,0,0,GetPolygonPickAccelerator(-nPnts));
	
    if (PickPerim == 1) 
    {
    	HANDLE hNew = GSSiGlobAlloc ( 773,GMEM_MOVEABLE,(nPnts+1)*(long)sizeof(DPOINT));
    	HPDPOINT	pNewPoints=(HPDPOINT)GlobalLock (hNew);
    	BOOL	rtn; 
    	long	np=nPnts;
    	
    	hmemmove ((HPSTR)pNewPoints,(HPSTR)lpDpoints,np*(long)sizeof(DPOINT));
    	pNewPoints[np++] = *lpDpoints;
    	rtn = PickPolylineD (pNewPoints,np,0,3,0,0,IDNINT(Offdist+1));
    	GSSiGlobUlFree (&hNew); 
    	if (rtn || PickPerim == 1)
{
#if ENABLETRACE
GSSiExitProg (993);
#endif
    		return rtn;
}
    }	
	
    if (!pina  && !(PickPerim == 2))
		goto Exit;

 	lpNewPoints = lpDpoints;
	for (i=0;i<nPnts;i++,lpDpoints++)
	{   
	 	AddDPointToMinMax (lpDpoints,&Rect);
		Dist = ldistp (PickPointW,*lpDpoints);  
		if (Dist < MinDist)
		{
			MinDist = Dist;
			NodePoint = *lpDpoints;
			NearPoint = i;
		}
		if (!i)
			BeginPoint = *lpDpoints;
		else  
		{
			Perim += ldistp(LastPoint,*lpDpoints);
			Area += ( LastPoint.y - lpDpoints->y) * (lpDpoints->x + LastPoint.x) / 2;
		}
		LastPoint = *lpDpoints;
	} 
	Perim += ldistp(LastPoint,BeginPoint);
	Area += (LastPoint.y - BeginPoint.y) * (BeginPoint.x + LastPoint.x) / 2;  
	
	
/*		{
			HANDLE hCP = GSSiGlobAlloc ( 774,GHND,4); 
			LPSHORT pCP = GlobalLock (hCP); 
			double	Area2;
			
			*pCP = -1;   
			GlobalUnlock (hCP);
			Area2 = AreaFromPolyWithCurves (hUnSplinedPoly,nUnSplinedPoints,hCP);  
			GSSiGlobFree (&hCP);
		}*/
	if (hUnSplinedPoly && hCurvePoints)
		Area = AreaFromPolyWithCurves (hUnSplinedPoly,nUnSplinedPoints,hCurvePoints);
	EndPoint = LastPoint;
     
    if (PickingByRefno)
    	Selected = TRUE;
    else if (PickPerim==2) 
    {
    	BOOL	rtn;
    	short	inc = 1;
    	
    	if (pina)
    		inc++; 
    	
    	Selected = PickPolylineD (lpNewPoints,nPnts,0,-3,0,0,max(0,IDNINT(Offdist-inc)));
    	if (Selected)
    	{
{
#if ENABLETRACE
GSSiExitProg (993);
#endif
			return (Selected);
}
    	}
    	else
    		Selected = pina;	
    }
    else
    	Selected = TRUE;
    if (Selected)
	{   
		if (CurrentType == GF_TEXT) 
		{
			BeginPoint = EndPoint = FromPoint = ToPoint = LastElementBeginPoint;
			AZ[0]=AZ[1]=AZ[2]=LastElementAZ;          
			Type = 4;
		}
		else   
		{
			FromPoint = ToPoint = NodePoint;
			Type = 3;
		}
//		GetItemMinMax (&CurrentItemMinMax,&Rect);
		PickedPoint = PickPointW;
		PickedPointFile = BasePtToFilePt (PickPointW);  
		BeginPointFile = EndPointFile = BasePtToFilePt (*lpNewPoints);  
		if (hElev && nPnts == 3 && !PickingByRefno)
		{   
			LPFLOAT		pElev = (LPFLOAT)GlobalLock (hElev);   
			
			Elev = GetTriangleElev (PickPointW,lpNewPoints,pElev); 
			HaveLastTriangle = TRUE;
			for (i=0;i<3;i++)
			{
				LastTrianglePoint[i] = lpNewPoints[i];
				LastTriangleElev[i] = pElev[i];
			}
			GlobalUnlock (hElev);
		}
		PickListAdd (FileNum,SubFile,FileInIndex,ItemSeg,CurrentRefno,CurrentDesc,0,
					   Type,0,Offdist,AZ,Perim,CurrentItem,CurElement,BeginPoint,EndPoint,PickedPoint,
					   NodePoint,NearPoint,
					   Area,&Rect,
					   BeginPointFile,EndPointFile,PickedPointFile,nPnts,Elev,&FromPoint,&ToPoint);
	}
Exit:
{
#if ENABLETRACE
GSSiExitProg (993);
#endif
	return (Selected);
}
#if ENABLETRACE
}
#endif
}                  

BOOL PickPolyInArea (short Type,HPPOINTS lpPoints,long nPnts,int PolyID)
#if ENABLETRACE
{GSSiEnterProg (994);
#endif
{   
	LPSHORT	lpNump;
	HPDPOINT lpDpoint;
	HPDPOINT lpDpoints;  
	DPOINT	BeginPoint, EndPoint, LastPoint; 
	POINT	BeginPointFile, EndPointFile;
	LPMNMXCORD	lpRect; 
	DWORD	Nump, i,j; 
	HANDLE	hPoints;
	BOOL	rtn=FALSE;  
	double	Area=0,Perim=0;  
	short	Type2=GF_POLYLINE;
	
	Nump = nHighlightAreaPoints;  
	lpRect = (LPMNMXCORD) GlobalLock (hHighlightArea); 
    lpRect++;
    lpDpoint = (LPDPOINT) lpRect;  
	    
    hPoints = GSSiGlobAlloc ( 775,GMEM_MOVEABLE,(long)nPnts*sizeof(DPOINT));
 	lpDpoints = (HPDPOINT) GlobalLock (hPoints);
	for (i=0;i<nPnts;i++,lpPoints++,lpDpoints++)
	{   
		*lpDpoints = FilePtToBasePt (POINTStoPOINT(*lpPoints));  
		if (!i)
		{
			BeginPointFile = POINTStoPOINT(*lpPoints);
			BeginPoint = *lpDpoints;   
		}
		else
		{
			Perim += ldistp(LastPoint,*lpDpoints);
			if (!Type)
				Area += (LastPoint.y - lpDpoints->y) * (lpDpoints->x + LastPoint.x) / 2;
		}
		LastPoint = *lpDpoints;
	}  
	lpPoints--;
	EndPointFile = POINTStoPOINT(*lpPoints);
	if (!Type)
	{   
		Type2 = GF_AREA;
		Perim += ldistp(LastPoint,BeginPoint);
		Area += (LastPoint.y - BeginPoint.y) * (BeginPoint.x + LastPoint.x) / 2;
	}  
	EndPoint = LastPoint;
	GlobalUnlock(hPoints);
 	lpDpoints = (HPDPOINT) GlobalLock (hPoints);
	rtn = PolyInArea (Type2,nPnts,lpDpoints,0,Nump,lpDpoint,1,0,InclusionOpt,hHighlightAreaAccelerator);
Exit:
	GSSiGlobUlFree (&hPoints);
	GlobalUnlock (hHighlightArea);
	if (rtn) 
	{
		PickList[0].ViewID = CurView->ID; 
		PickList[0].ConfigID = CurrentConfig;
		PickList[0].HiPrecis = 0;
		PickList[0].FileNum = FileNum;
		PickList[0].FileInIndex = FileInIndex; 
		PickList[0].SubFile = SubFile;
		PickList[0].Segment = ItemSeg;
		PickList[0].Refno = CurrentRefno;
		PickList[0].MSLink = CurMSLink;
		PickList[0].Desc = CurrentDesc;  
		PickList[0].PolyID = PolyID;
		if (CurrentType == GF_AREA)
			PickList[0].Type = 3; 
		else if (CurrentType == GF_POINT || CurrentType == GF_TEXT)
			PickList[0].Type = 1;  
		else
			PickList[0].Type = 2;  
		PickList[0].PCT = 1;
		PickList[0].OffDist = 0;  
		PickList[0].Length = Perim;
		PickList[0].Offset = CurrentItem;
		PickList[0].Element= CurElement; 
		PickList[0].BeginPointFile = BeginPointFile; 
		PickList[0].EndPointFile = EndPointFile;
		PickList[0].BeginPoint = BeginPoint; 
		PickList[0].EndPoint = EndPoint;
		PickList[0].NumPoints = nPnts;  
		PickList[0].Area = Area; 
		GetItemMinMax (&CurrentItemMinMax,&PickList[0].Rect);
//		PickList[0].Rect = *Rect; 
		_fstrcpy (PickList[0].Prefix,CurrentPrefix);
		_fstrcpy (PickList[0].UDI,CurrentUDI);
		if (UnHighlightInArea) 
			RemoveFromHighlightList (PickList[0].Refno,0);
		else
			AddToHighlightList (CurrentRefno,&PickList[0],TRUE);
	}
{
#if ENABLETRACE
GSSiExitProg (994);
#endif
	return (rtn);
}
#if ENABLETRACE
}
#endif
}

BOOL PickPolyInAreaD(short Type, HPDPOINT lpDpoints, long nPnts, int PolyID, LPDOUBLE pAZ, LPDOUBLE pSize, int nPoly, HANDLE hPolyPartLen, long nTotPnts, HPDPOINT lpFirstPt)
#if ENABLETRACE
{GSSiEnterProg (995);
#endif
{   
	LPSHORT	lpNump;
	HPDPOINT lpDpointIn=lpDpoints,lpDpoint;
	DPOINT	BeginPoint, EndPoint, LastPoint; 
	POINT	BeginPointFile, EndPointFile;
	LPMNMXCORD	lpRect; 
	DWORD	Nump, i,j; 
	BOOL	rtn=TRUE;  
	double	Area=0,Perim=0, AZ=0, BPAZ,EPAZ;  
	short	Type2=GF_POLYLINE;
	
	if (pAZ)
		AZ = *pAZ;   
	if (Type == 6) //dummy delete
		goto Exit;
	else
		rtn = FALSE; 
	Nump = nHighlightAreaPoints;  
	lpRect = (LPMNMXCORD) GlobalLock (hHighlightArea); 
    lpRect++;
    lpDpoint = (LPDPOINT) lpRect;  
	for (i=0;i<nPnts;i++,lpDpoints++)
	{   
		switch (i)
		{
			case 0:
				BeginPoint = *lpDpoints;   
			break; 
			case 1:
				BPAZ = getazd (&BeginPoint,lpDpoints);
			default:
				Perim += ldistp(LastPoint,*lpDpoints);
				if (!Type)
					Area += (LastPoint.y - lpDpoints->y) * (lpDpoints->x + LastPoint.x) / 2;  
			break;
		}
		if (i && i == nPnts-1)
			EPAZ = getazd (&LastPoint,lpDpoints);
		LastPoint = *lpDpoints;
	}  
	BeginPointFile = BasePtToFilePt (BeginPoint);
	EndPointFile = BasePtToFilePt (LastPoint);
	if (!Type)
	{   
		Type2 = GF_AREA;
		Perim += ldistp(LastPoint,BeginPoint);
		Area += (LastPoint.y - BeginPoint.y) * (BeginPoint.x + LastPoint.x) / 2;
		if (hUnSplinedPoly)
			Area = AreaFromPolyWithCurves (hUnSplinedPoly,nUnSplinedPoints,hCurvePoints);
	}  
	EndPoint = LastPoint;
 	lpDpoints = lpDpointIn;
	rtn = PolyInArea (Type2,nPnts,lpDpoints,0,Nump,lpDpoint,nHighlightAreaPoly,hHighlightAreaPolyPartLen,
					  InclusionOpt,hHighlightAreaAccelerator);
	GlobalUnlock (hHighlightArea); 
Exit:
	if (pSize)
		Perim = *pSize;
	if (rtn) 
	{   
		if (nPnts < 2)
			PickList[0].BPAZ = PickList[0].EPAZ = AZ;   
		else 
		{
			PickList[0].BPAZ = getazd(&lpDpointIn[0], &lpDpointIn[1]);
			PickList[0].EPAZ = getazd(&lpDpointIn[nPnts - 2], &lpDpointIn[nPnts - 1]);
		} 
		if (CurrentType == GF_AREA && nPoly > 1)
		{
			LPINT	pPartLen = (LPINT)GlobalLock(hPolyPartLen);
			int ipoly;
			double d;

			Area = ComputeAreaAreaD(lpFirstPt, nTotPnts, &Perim);
			for (ipoly = 0; ipoly < nPoly-1; ipoly++, pPartLen++)
			{
				lpFirstPt += *pPartLen;
				if (ipoly)
					lpFirstPt++;
				d = ldistpp(lpFirstPt - 1, lpFirstPt);
				Perim -= d*2;
			}
			GlobalUnlock(hPolyPartLen);

		}
		PickList[0].PPAZ = AZ;
		PickList[0].HiPrecis = 1;
		PickList[0].ViewID = CurView->ID;
		PickList[0].ConfigID = CurrentConfig;
		PickList[0].FileNum = FileNum;
		PickList[0].FileInIndex = FileInIndex; 
		PickList[0].SubFile = SubFile;
		PickList[0].Segment = ItemSeg;
		PickList[0].Refno = CurrentRefno;
		PickList[0].MSLink = CurMSLink;
		PickList[0].Desc = CurrentDesc;  
		PickList[0].PolyID = PolyID;
		PickList[0].Length = Perim;
		PickList[0].Blocked = ItemIsBlocked;
		if (Type == 6 || ItemIsDeleted)
		{
			PickList[0].IsDeleted = TRUE;
			PickList[0].Type = 6; 
		}
		else if (CurrentType == GF_AREA)
			PickList[0].Type = 3; 
		else if (CurrentType == GF_CURVE)   
		{
			PickList[0].Type = 5;  
			PickList[0].NodePoint = CurPOCW;
			PickList[0].Length = fabs (CurCLEN);
		}
		else if (CurrentType == GF_POINT)
			PickList[0].Type = 1;  
		else if (CurrentType == GF_TEXT)
			PickList[0].Type = 4;  
		else
			PickList[0].Type = 2;  
		PickList[0].PCT = 1;
		PickList[0].OffDist = 0;  
		PickList[0].Offset = CurrentItem;
		PickList[0].Element= CurElement; 
		if (PickList[0].Type == 1)
		{
			PickList[0].BeginPointFile = CurPointF; 
			PickList[0].EndPointFile = CurPointF;
			PickList[0].BeginPoint = CurPointW; 
			PickList[0].EndPoint = CurPointW;  
		} 
		else 
		{
			PickList[0].BeginPointFile = BeginPointFile; 
			PickList[0].EndPointFile = EndPointFile;
			PickList[0].BeginPoint = BeginPoint; 
			PickList[0].EndPoint = EndPoint;
		} 
		PickList[0].NumPoints = nPnts;  
		PickList[0].Area = Area; 
		GetItemMinMax (&CurrentItemMinMax,&PickList[0].Rect);
//		PickList[0].Rect = *Rect; 
		_fstrcpy (PickList[0].Prefix,CurrentPrefix);
		_fstrcpy (PickList[0].UDI,CurrentUDI);
		if (UnHighlightInArea) 
			RemoveFromHighlightList (PickList[0].Refno,0);
		else
			AddToHighlightList (CurrentRefno,&PickList[0],TRUE);
	}
{
#if ENABLETRACE
GSSiExitProg (995);
#endif
	return (rtn);
}
#if ENABLETRACE
}
#endif
}

BOOL ContinuePickingInArea (BOOL Display)
#if ENABLETRACE
{GSSiEnterProg (996);
#endif
{
	MSG     msg;   
	BOOL	IsAccel;  
	char	str[200];
	static	long	LastNumProcessed=0; 
	long	MessageInc=100;
	
	if (NumRecordsProcessed < LastNumProcessed)
		LastNumProcessed = 0;
	if (NumRecordsProcessed - LastNumProcessed < MessageInc)
		goto Exit;
	LastNumProcessed = NumRecordsProcessed;
	while (GSSiPeekMessage(&msg,0,0,0,PM_REMOVE))
	{
#if ENABLETRACE
		SetLastMessage(-1 * (long)msg.message, msg.wParam);
#endif
		if (msg.message == WM_PAINT)
		{
		  TranslateMessage(&msg);
		  DispatchMessage(&msg);
		}
		else if (msg.message == WM_MOUSEMOVE)
		{
		  TranslateMessage(&msg);
		  DispatchMessage(&msg);
		}
	    else if (msg.message == WM_KEYDOWN && msg.wParam == 27)  
	    {   
	    	GSSiMsgBox (GetFocus(),"Highlight terminated by user","Highlight List Incomplete",MB_ICONEXCLAMATION,0);
{
#if ENABLETRACE
GSSiExitProg (996);
#endif
			return FALSE;
}
		}
		else if (!PrintMsgWnd) 
		{ 
		  TranslateMessage(&msg);
		  DispatchMessage(&msg);
		} 
		else
			IsDialogMessage(PrintMsgWnd, &msg); 
	} 
	if (Display)
	{
		sprintf (str,"%s:%ld records highlighted",CurWinText,BT_NUM_IN_INDEX(hHighlight));
		SetWindowText (hWndMain,str); 
	}
Exit:
{
#if ENABLETRACE
GSSiExitProg (996);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

mnmxCor WBoundsToFileBounds(LPMNMXCORD worldBounds)
{
	mnmxCor fb;
	MNMXCORD bounds;
	DPOINT BasePoint;
	DPOINT FilePointD;

	DBoundsInit(&bounds);
		 
	BasePoint.x = worldBounds->xmn;
	BasePoint.y = worldBounds->ymn;
	FilePointD = BasePtToFilePtD(BasePoint);
	AddDPointToMinMax(&FilePointD, &bounds);
	BasePoint.x = worldBounds->xmn;
	BasePoint.y = worldBounds->ymx;
	FilePointD = BasePtToFilePtD(BasePoint);
	AddDPointToMinMax(&FilePointD, &bounds);
	BasePoint.x = worldBounds->xmx;
	BasePoint.y = worldBounds->ymx;
	FilePointD = BasePtToFilePtD(BasePoint);
	AddDPointToMinMax(&FilePointD, &bounds);
	BasePoint.x = worldBounds->xmx;
	BasePoint.y = worldBounds->ymn;
	FilePointD = BasePtToFilePtD(BasePoint);
	AddDPointToMinMax(&FilePointD, &bounds);

	fb.xmn = max(SHRT_MIN, bounds.xmn);
	fb.xmx = min(SHRT_MAX, bounds.xmx);
	fb.ymn = max(SHRT_MIN, bounds.ymn);
	fb.ymx = min(SHRT_MAX, bounds.ymx);
	return fb;
}
long HighlightInArea (HWND hWnd,LPMNMXCORD pBounds,BOOL AddToList,BOOL DisplayNum,HANDLE hMask)
#if ENABLETRACE
{GSSiEnterProg (997);
#endif
{	MNMXCORL	SaveBounds;
	MNMXCORD	SaveWBounds, SaveNewBounds;
	BOOL		SaveHaveBounds;
	int			iPickAp,ii, ivp,Type,nPoly; 
	POINT		PickPoint, PickPoint2;
	LPVISLIST	SaveVis;
	double		PickApW;
	POINT		WinPT1, WinPT2;
	DPOINT		WPoint1, WPoint2, PickPointBase2;
	HPEN	hPen, OldPen;
	LPMNMXCORD	lpRect; 
	HANDLE	hPoly;   
	DPOINT	BasePoint, FilePointD;
	long	AreaNum=1;
	BOOL	SavePrint=Printing;
	long	Added=0, BeginNum = BT_NUM_IN_INDEX (hHighlight);  
	short	VPPass=0, SaveVPID = CurView->ID;
	BOOL	SaveDH = DisableHalt;
	double	Offset;

	DisableHalt = TRUE;
	NumRecordsProcessed=0;
	if (DisplayNum)
	{
		WaitCursor (1);
		GetWindowText (hWndMain,(LPSTR)CurWinText,sizeof(CurWinText));
	}
	SaveVis = CurVis;

	SaveBounds = CurView->Bounds;
	SaveWBounds = CurView->WBounds; 
	SaveNewBounds = CurView->NewBounds;
	SaveHaveBounds = CurView->HaveBounds;
	
	Pick=TRUE;
	Processing = TRUE;
	NumPicked=0;  
	if (!AddToList)
		UnHighlightInArea = TRUE;  
	if (hMask)
		AreaNum = -1;
	else if (pBounds)
		AreaNum = 0;
	if (UseHLTGraphicsFile)
	{
		LPSTR pDot;

		FastMapCopy = TRUE;
		GSSiGetTempFileName (0,"gm",0,HLTGraphicsFile); 
		pDot = strrchr (HLTGraphicsFile,'.');
		strcpy (pDot,".hgf");
		FastMapCopyFid = GSSiOpenFile (HLTGraphicsFile,0,OF_CREATE);
		BigWrite (FastMapCopyFid,&FastMapCopy,sizeof(FastMapCopy),-1);//prevents HLTGraphicsPos ==0
	}
	while (hHighlightArea = GetNextHighlightArea (AreaNum++,pBounds,&Type,&nHighlightAreaPoints,&nHighlightAreaPoly,&hHighlightAreaPolyPartLen,&Offset,0))
	{   
		for (ivp = 0;ivp < *pNumViewports; ivp++)
		{   
			if (VPIsActive (pViewports[ivp]->ID) &&
				(pViewports[ivp]->ID == SaveVPID || (PickSubVP && (pViewports[ivp]->DisplayInParent && pViewports[ivp]->Parent == SaveVPID))))
			{
				SetViewport (pViewports[ivp]->ID);
			    lpRect = (LPMNMXCORD) GlobalLock (hHighlightArea); 
				CurView->WBounds = *lpRect;
				CurView->NewBounds = CurView->WBounds; 
				CurView->HaveBounds = TRUE; 
				if (!CurView->BaseUnitsPerPixel)
					CurView->BaseUnitsPerPixel = 1;                             
			    GlobalUnlock (hHighlightArea);
			    CurView->PassID = 0;
				CurView->CurFile=-1;
				PrevLayerVP = 0; 
				SelectVisList (TRUE);
			//	IncrementFile ();
				Printing = TRUE;
				PickName[0] = '\0';
				while (GetNextViewportFile (FALSE))
				{   if (PltType < 5)
					{
						DisplayPlotInit(hWnd,TRUE);
						if (OpenMap (hWnd, (HDC)1))
						{   
							_fstrcpy (PickName,PltName); 
							BasePoint.x = CurView->WBounds.xmn;
							BasePoint.y = CurView->WBounds.ymn;
							FilePointD = BasePtToFilePtD (BasePoint);
							CurView->Bounds.xmn = max(SHRT_MIN,FilePointD.x);
							CurView->Bounds.ymn = max(SHRT_MIN,FilePointD.y);
							BasePoint.x = CurView->WBounds.xmx;
							BasePoint.y = CurView->WBounds.ymx;
							FilePointD = BasePtToFilePtD (BasePoint);
							CurView->Bounds.xmx = min(SHRT_MAX,FilePointD.x);
							CurView->Bounds.ymx = min(SHRT_MAX,FilePointD.y);
							while (DisplaySeg (&NULLHDC,FALSE))
								if (!ContinuePickingInArea (DisplayNum))  
								{
									GSSiGlobFree (&hHighlightArea);
									GSSiGlobFree (&hHighlightAreaPolyPartLen);
									GSSiGlobFree (&hHighlightAreaAccelerator[0]); 
									GSSiGlobFree (&hHighlightAreaAccelerator[1]); 
									GSSiGlobFree (&hHighlightAreaAccelerator[2]); 
									Added = -1;
									goto Exit;
								}
		
						} 
					}
				}
				PickDispersedPoints (PickPointBase,PickAp,0);  
			} 
		}
		GSSiGlobFree (&hHighlightArea); 
		GSSiGlobFree (&hHighlightAreaPolyPartLen);
		GSSiGlobFree (&hHighlightAreaAccelerator[0]); 
		GSSiGlobFree (&hHighlightAreaAccelerator[1]); 
		GSSiGlobFree (&hHighlightAreaAccelerator[2]); 
		SetViewport (SaveVPID);  
		if (!AreaNum)
			break;
	}
Exit:
	if (FastMapCopy)
	{
		FastMapCopy = FALSE;
		GSSiClose (FastMapCopyFid);
		FastMapCopyFid=HFILE_ERROR;
	}
    Printing = SavePrint;
    if (Printing)
    	ii=1;
	Pick=FALSE;
	CurView->Bounds = SaveBounds;
	CurView->WBounds = SaveWBounds; 
	CurView->NewBounds = SaveNewBounds; 
	CurView->HaveBounds = SaveHaveBounds;
	SetDisplayMode (CurView->hDC,GF_TEXTMODE);
	SelectClipRgn (CurView->hDC,0);
	CurVis = SaveVis;
	if (DisplayNum)
	{
		WaitCursor (-1);
		SetWindowText (hWndMain,(LPSTR)CurWinText);
	}
	UnHighlightInArea = FALSE; 
	if (Added >= 0)
    	Added = BT_NUM_IN_INDEX (hHighlight) - BeginNum;
	DisableHalt = SaveDH;
	Processing = FALSE;
    if (HLTDlgWnd)
    	PostMessage (HLTDlgWnd,WM_COMMAND,IDC_REDISPLAY, 0L);
{
#if ENABLETRACE
GSSiExitProg (997);
#endif
	return (Added);
}

#if ENABLETRACE
}
#endif
} 
 
double ComputeProjectedAreaArea (HPPOINTS lpPoints,long nPnts,LPDOUBLE pPerim)
#if ENABLETRACE
{GSSiEnterProg (998);
#endif
{    
	HANDLE Handle;
	HPDPOINT	lpNewPoints, lpDpoints;
	DWORD	i;
	BOOL	WantArea=TRUE; 
	DPOINT	BeginPoint, EndPoint,PickedPoint, LastPoint, PickPointW;
	double	Area=0,Perim=0;    

 	if (nPnts<3)
 		WantArea = FALSE;
 	nPnts = labs (nPnts);
    Handle = GSSiGlobAlloc ( 776,GMEM_MOVEABLE,(long)nPnts*sizeof(DPOINT));
 	lpDpoints = (HPDPOINT) GlobalLock (Handle); 
 	lpNewPoints = lpDpoints;  
	for (i=0;i<nPnts;i++,lpPoints++,lpDpoints++)
	{   
		*lpDpoints = FilePtToBasePt (POINTStoPOINT(*lpPoints));
		ConvertCoord (lpDpoints,1,3);
		if (!i)
			BeginPoint = *lpDpoints;
		else  
		{
			Perim += ldistp(LastPoint,*lpDpoints);
			if (WantArea)
				Area += ( LastPoint.y - lpDpoints->y) * (lpDpoints->x + LastPoint.x) / 2;
		}
		LastPoint = *lpDpoints;
	} 
	Perim += ldistp(LastPoint,BeginPoint);  
	if (WantArea)
		Area += (LastPoint.y - BeginPoint.y) * (BeginPoint.x + LastPoint.x) / 2;  
	GSSiGlobUlFree (&Handle);     
	*pPerim = Perim;
{
#if ENABLETRACE
GSSiExitProg (998);
#endif
	return (Area);
}
#if ENABLETRACE
}
#endif
} 

double GetPoly3DLength2D (HPDPOINT3D lpPoints,long nPnts) 
#if ENABLETRACE
{GSSiEnterProg (999);
#endif
{
	double Dist=0;
	DWORD	i; 
	HPDPOINT3D	lpPoints2=lpPoints+1;
	
	for (i=1;i<nPnts;i++,lpPoints++,lpPoints2++)
		Dist += ldistpp((LPDPOINT)lpPoints,(LPDPOINT)lpPoints2);
{
#if ENABLETRACE
GSSiExitProg (999);
#endif
	return Dist;
}
#if ENABLETRACE
}
#endif
}

double GetPolyMaxDistBetweenPoints (HPDPOINT lpPoints,long nPnts,LPLONG pMaxDistPointID)
{
	double MaxDist=-1, d;
	DWORD	i; 
	HPDPOINT	lpPoints2=lpPoints+1;
	
	*pMaxDistPointID = -1;
	for (i=1;i<nPnts;i++,lpPoints++,lpPoints2++)
	{
		d = ldistpp(lpPoints,lpPoints2);
		if (d > MaxDist)
		{
			MaxDist = d;
			*pMaxDistPointID = i-1;
		}
	}
	return MaxDist;
}

double GetPolyLengthD (HPDPOINT lpPoints,long nPnts)
#if ENABLETRACE
{GSSiEnterProg (999);
#endif
{
	double Dist=0;
	DWORD	i; 
	HPDPOINT	lpPoints2=lpPoints+1;
	
	for (i=1;i<nPnts;i++,lpPoints++,lpPoints2++)
		Dist += ldistp(*lpPoints,*lpPoints2);
{
#if ENABLETRACE
GSSiExitProg (999);
#endif
	return Dist;
}
#if ENABLETRACE
}
#endif
}
                  
double GetPolyLength (HPPOINT lpPoints,long nPnts)
#if ENABLETRACE
{GSSiEnterProg (999);
#endif
{
	double Dist=0;
	DWORD	i; 
	HPPOINT	lpPoints2=lpPoints+1;
	
	for (i=1;i<nPnts;i++,lpPoints++,lpPoints2++)
		Dist += idist(*lpPoints,*lpPoints2);
{
#if ENABLETRACE
GSSiExitProg (999);
#endif
	return Dist;
}
#if ENABLETRACE
}
#endif
}
double GetPolyLengthF(HPFPOINT lpPoints, long nPnts)
#if ENABLETRACE
{GSSiEnterProg (999);
#endif
{
	double Dist = 0;
	DWORD	i;
	HPFPOINT	lpPoints2 = lpPoints + 1;

	for (i = 1; i<nPnts; i++, lpPoints++, lpPoints2++)
		Dist += ldistp(*lpPoints, *lpPoints2);
	{
#if ENABLETRACE
		GSSiExitProg(999);
#endif
		return Dist;
	}
#if ENABLETRACE
}
#endif
}
double ComputeProjectedAreaAreaD(HPDPOINT lpPoints, long nPnts, LPDOUBLE pPerim)
#if ENABLETRACE
{GSSiEnterProg (1000);
#endif
{    
	HANDLE Handle;
	HPDPOINT	lpNewPoints, lpDpoints;
	DWORD	i;
	BOOL	WantArea=TRUE; 
	DPOINT	BeginPoint, EndPoint,PickedPoint, LastPoint, PickPointW;
	double	Area=0,Perim=0;    

 	if (nPnts<3)
 		WantArea = FALSE;
 	nPnts = labs (nPnts);
    Handle = GSSiGlobAlloc ( 777,GMEM_MOVEABLE,(long)nPnts*sizeof(DPOINT));
 	lpDpoints = (HPDPOINT) GlobalLock (Handle); 
 	lpNewPoints = lpDpoints;  
	for (i=0;i<nPnts;i++,lpPoints++,lpDpoints++)
	{   
		*lpDpoints = *lpPoints;
		ConvertCoord (lpDpoints,1,3);
		if (!i)
			BeginPoint = *lpDpoints;
		else  
		{
			Perim += ldistp(LastPoint,*lpDpoints);
			if (WantArea)
				Area += ( LastPoint.y - lpDpoints->y) * (lpDpoints->x + LastPoint.x) / 2;
		}
		LastPoint = *lpDpoints;
	} 
	Perim += ldistp(LastPoint,BeginPoint);  
	if (WantArea)
		Area += (LastPoint.y - BeginPoint.y) * (BeginPoint.x + LastPoint.x) / 2;  
	GSSiGlobUlFree (&Handle);
	*pPerim = Perim;
{
#if ENABLETRACE
GSSiExitProg (1000);
#endif
	return (Area);
}
#if ENABLETRACE
}
#endif
} 
 
BOOL ConvertPolyCoord (HPDPOINT pPoly,int nPoints,int From,int To)
{
	int	i;

	for (i=0;i<nPoints;i++)
		ConvertCoord(&pPoly[i],From,To);
	return TRUE;
}

BOOL ConvertPoly (LPSHORT ipnt,HANDLE hTranFrom,HANDLE hTranTo,LPSTR BeginSeg,int From,int To)
#if ENABLETRACE
{GSSiEnterProg (1008);
#endif
{	int		idesc, ItemLen, TSize, PointSize;
	long	*pRefno;
	long	remlen;
	LPBYTE	Pcode;
	int		x1,y1,x2,y2, ipen, ii;   
	HPDOUBLE	pDSize;
	HPFLOAT	pSize;
	int		n;   
	long	PolyBufferLen;
	DWORD	i;   
	WORD	nPnts;
    DPOINT	FilePointD, WorldPoint;
	static	long	StartElement=-1;
	static	int	nPoly=0, iPoly=0; 
         
         
		if (StartElement > 0)
			ipnt += StartElement;
		StartElement = -1;
        while (*ipnt != 0)
        {   Pcode = (LPBYTE)ipnt;
        	CurElementPnt = ipnt++; 
        	PointSize = 4;

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

	            case 5: /* put area */

		 		    ipen = *ipnt;
		 			ipnt++;
                
	            case 6: /* put polyline */

		 		{   nPnts = *ipnt;
		 		    ipnt++;
		 DoPolyline:lpCurPoints = (HPPOINTS) ipnt;
		 
		 		    ipnt = ipnt + nPnts * 2;
					for (i=0;i<nPnts;i++,lpCurPoints++)
					{
     					FilePointD.x = lpCurPoints->x;
     					FilePointD.y = lpCurPoints->y;
					    TRANS2 (FilePointD.x,FilePointD.y,&WorldPoint.x,&WorldPoint.y,hTranFrom);
						ConvertCoord(&WorldPoint,From,To);
					    TRANS2 (WorldPoint.x,WorldPoint.y,&FilePointD.x,&FilePointD.y,hTranTo); 
					    lpCurPoints->x = IDNINT (FilePointD.x);
					    lpCurPoints->y = IDNINT (FilePointD.y);
					}
				}
				break;
                
	            case 51: /* put area */

		 		    ipen = *ipnt;
		 			ipnt++;
                
	            case 61: /* put polyline */

		 		{   nPnts = *ipnt;
		 		    ipnt++;
		DoPolylineD:lpDCurPoints = (HPDPOINT) ipnt;
		 
		 		    ipnt = ipnt + nPnts * 8;
					for (i=0;i<nPnts;i++,lpDCurPoints++)
					{
						ConvertCoord(lpDCurPoints,From,To);
					}
				}
				break;
                
				case 93:
                case 92:
				case 12: /* item minmax */
				{	
					LPMINMAX   pMinMax;
					
					pMinMax = (LPMINMAX)ipnt;  
										
					ItemSeg = CurrentSeg;
					ConvertMinMax (pMinMax,hTranFrom,hTranTo,From,To);
					nPoly = 0;    
					ipnt += 4;
					ItemLen = abs(*ipnt);
					ipnt++;
                }
                break;

		case 15: /* point symbol */
		{	
			LPSHORT	ipnt1=ipnt++;
			LPSHORT	ipnt2=ipnt++;
			
			FilePointD.x = *ipnt1;
			FilePointD.y = *ipnt2;
			PointRot = *ipnt++; 
		    TRANS2 (FilePointD.x,FilePointD.y,&WorldPoint.x,&WorldPoint.y,hTranFrom);
			ConvertCoord(&WorldPoint,From,To);
		    TRANS2 (WorldPoint.x,WorldPoint.y,&FilePointD.x,&FilePointD.y,hTranTo); 
		    *ipnt1 = IDNINT (FilePointD.x);
		    *ipnt2 = IDNINT (FilePointD.y);
		}
		break;
		
/*		case 172: // HiPrecis Curve
		{
			HPDPOINT	pDPoint;
					
        	ipnt++;
			pDPoint = ipnt;ipnt+=8;
			FilePointD = *pDPoint;
		    TRANS2 (FilePointD.x,FilePointD.y,&WorldPoint.x,&WorldPoint.y,hTranFrom);
			ConvertCoord(&WorldPoint,From,To);
		    TRANS2 (WorldPoint.x,WorldPoint.y,&FilePointD.x,&FilePointD.y,hTranTo); 
		    *pDPoint = FilePointD;
			pDPoint = ipnt;ipnt+=8;
			FilePointD = *pDPoint;
		    TRANS2 (FilePointD.x,FilePointD.y,&WorldPoint.x,&WorldPoint.y,hTranFrom);
			ConvertCoord(&WorldPoint,From,To);
		    TRANS2 (WorldPoint.x,WorldPoint.y,&FilePointD.x,&FilePointD.y,hTranTo); 
		    *pDPoint = FilePointD;
			pDPoint = ipnt;ipnt+=8;
			FilePointD = *pDPoint;
		    TRANS2 (FilePointD.x,FilePointD.y,&WorldPoint.x,&WorldPoint.y,hTranFrom);
			ConvertCoord(&WorldPoint,From,To);
		    TRANS2 (WorldPoint.x,WorldPoint.y,&FilePointD.x,&FilePointD.y,hTranTo); 
		    *pDPoint = FilePointD;
		}
		break; */
		
        case 172:
        {
			HPDPOINT	pDPoint;
			int		st;
					
        	ipnt++;
			pDPoint = (HPDPOINT)ipnt;ipnt+=8;
			ConvertCoord(pDPoint,From,To);
			pDPoint = (HPDPOINT)ipnt;ipnt+=8;
			ConvertCoord(pDPoint,From,To);
			pDPoint = (HPDPOINT)ipnt;ipnt+=8;
			ConvertCoord(pDPoint,From,To);
		}
		break;
		
		case 16: /* line symbol */
		{	
			LPSHORT	ipnt1=ipnt++;
			LPSHORT	ipnt2=ipnt++;
			LPSHORT	ipnt3=ipnt++;
			LPSHORT	ipnt4=ipnt++;
			
			FilePointD.x = *ipnt1;
			FilePointD.y = *ipnt2;
		    TRANS2 (FilePointD.x,FilePointD.y,&WorldPoint.x,&WorldPoint.y,hTranFrom);
			ConvertCoord(&WorldPoint,From,To);
		    TRANS2 (WorldPoint.x,WorldPoint.y,&FilePointD.x,&FilePointD.y,hTranTo); 
		    *ipnt1 = IDNINT (FilePointD.x);
		    *ipnt2 = IDNINT (FilePointD.y);
			FilePointD.x = *ipnt3;
			FilePointD.y = *ipnt4;
		    TRANS2 (FilePointD.x,FilePointD.y,&WorldPoint.x,&WorldPoint.y,hTranFrom);
			ConvertCoord(&WorldPoint,From,To);
		    TRANS2 (WorldPoint.x,WorldPoint.y,&FilePointD.x,&FilePointD.y,hTranTo); 
		    *ipnt3 = IDNINT (FilePointD.x);
		    *ipnt4 = IDNINT (FilePointD.y);
		}
		break;
		
		case 19: // graphics text
		{ 
			int		nchar;
			LPGRTEXTHEADER	pGRTextHeader;
			LPSTR	pText;  
			double	THeight;
			
		    pGRTextHeader = (LPGRTEXTHEADER)ipnt;
			THeight = GetTextHeadSize (pGRTextHeader);   
			SetTextHeadSize (pGRTextHeader,THeight*DistanceConversionFactor);
		    ipnt += sizeof(GRTEXTHEADER) / 2;  
		    nchar = pGRTextHeader->lText;  
		    ipnt += nchar/2;
		}
		break;   
		
    	case 20:	/* point symbol */   
    	{   
    		LPFLOAT	pRot;
    		
    		CurPointSize = *(LPFLOAT)ipnt; 
    		pSize = (HPFLOAT)ipnt; 
    		(*pSize) *= DistanceConversionFactor;
    		ipnt += 2; 
    		pRot = (LPFLOAT)ipnt;
    		ipnt += 2;
    		lpCurPoints = (HPPOINTS)ipnt;
    		ipnt += 2;  
			FilePointD.x = lpCurPoints->x;
			FilePointD.y = lpCurPoints->y;
		    TRANS2 (FilePointD.x,FilePointD.y,&WorldPoint.x,&WorldPoint.y,hTranFrom);
    		*pRot = ConvertRotation (*pRot,&WorldPoint,From,To);
    		PTRot = *pRot;
			ConvertCoord(&WorldPoint,From,To);
		    TRANS2 (WorldPoint.x,WorldPoint.y,&FilePointD.x,&FilePointD.y,hTranTo); 
		    lpCurPoints->x = IDNINT (FilePointD.x);
		    lpCurPoints->y = IDNINT (FilePointD.y);
/*			if (CurPointSize < 0)
				CurPointSize = -CurPointSize *DeviceToScreenFactor();
			else
				CurPointSize /= CurView->BaseUnitsPerPixel;*/
        }
    	break;
	        	
		
    	case 120:	/* point symbol (HiPrecis)*/   
    	{   
    		LPDOUBLE	pRot;
    		
    		CurPointSize = *(LPDOUBLE)ipnt;
    		pDSize = (HPDOUBLE)ipnt; 
    		(*pDSize) *= DistanceConversionFactor;
    		ipnt += 4;
    		pRot = (LPDOUBLE)ipnt;
    		ipnt += 4;
    		lpDCurPoints = (HPDPOINT)ipnt; 
    		ipnt += 8;  
    		*pRot = ConvertRotation (*pRot,lpDCurPoints,From,To);
    		PTRot = *pRot;
			ConvertCoord(lpDCurPoints,From,To);
/*			if (CurPointSize < 0)
				CurPointSize = -CurPointSize * DeviceToScreenFactor();
			else
				CurPointSize /= CurView->BaseUnitsPerPixel; */
        }
    	break;
	        	
		case 127:
			PointSize = 16;
    	case 27: // Multipolygon indicator
    	{
			GSSiGlobFree (&hPolyBuffer);   
    		nPoly = n = *ipnt++; 
    		iPoly = 0;
    		PolyBufferLen = nPoly - 1; // allows for linklines  
    		while (n--)
    			PolyBufferLen += *(LPWORD)ipnt++;
    		hPolyBuffer = GSSiGlobAlloc ( 784,GMEM_MOVEABLE,(long)PolyBufferLen*PointSize);
        }
        break;

		case 227:
			PointSize = 16;
    	case 228: // Multipolygon indicator
    	{
			GSSiGlobFree (&hPolyBuffer);   
    		nPoly = n = *(LPINT)ipnt++;
			ipnt++;
    		iPoly = 0;
    		PolyBufferLen = nPoly - 1; // allows for linklines  
    		while (n--)
			{
    			PolyBufferLen += *(LPINT)ipnt++;
				ipnt++;
			}
    		hPolyBuffer = GSSiGlobAlloc ( 784,GMEM_MOVEABLE,(long)PolyBufferLen*PointSize);
        }
        break;

    	case 33: /* jump to long rec and back */
    	{
			ContinuationOffset = *(LPLONG)ipnt; 
			ipnt += 2;    
			JumpBackSeg = CurrentSeg;
			JumpBackElement = (LPSTR)ipnt - BeginSeg;
			JumpBackElement++;
		}
		break; 
				
    	case 34: /* jump to long rec and back */
    	{
			ContinuationOffset = JumpBackSeg; 
			StartElement = JumpBackElement;
			*ipnt = 0;	
		}
		break; 

        case 35: /* point array */

 		{   nPnts = *ipnt;
 		    ipnt++;
  			lpCurPoints = (HPPOINTS) ipnt;
		 
 		    ipnt = ipnt + nPnts * 2;
			for (i=0;i<nPnts;i++,lpCurPoints++)
			{
				FilePointD.x = lpCurPoints->x;
				FilePointD.y = lpCurPoints->y;
			    TRANS2 (FilePointD.x,FilePointD.y,&WorldPoint.x,&WorldPoint.y,hTranFrom);
				ConvertCoord(&WorldPoint,From,To);
			    TRANS2 (WorldPoint.x,WorldPoint.y,&FilePointD.x,&FilePointD.y,hTranTo); 
			    lpCurPoints->x = IDNINT (FilePointD.x);
			    lpCurPoints->y = IDNINT (FilePointD.y);
			}
		}
		break;
				
        case 135: /* point array */

 		{   nPnts = *ipnt;
 		    ipnt++;
			lpDCurPoints = (HPDPOINT) ipnt;		 
 		    ipnt = ipnt + nPnts * 8;
			for (i=0;i<nPnts;i++,lpDCurPoints++)
			{
				ConvertCoord(lpDCurPoints,From,To);
			}
		}
		break;
				
                default: 
                	SkipSubRec (Pcode,(HPSHORT*)&ipnt,0);
                break;

			}
        }
	GSSiGlobFree (&hPolyBuffer);   
{
#if ENABLETRACE
GSSiExitProg (1008);
#endif
	return(FALSE);
}
#if ENABLETRACE
}
#endif
}  

void DebugShowLine (LPDPOINT p1,LPDPOINT p2)
#if ENABLETRACE
{GSSiEnterProg (1009);
#endif
{
	DPOINT Point[2];
	
	if (GetGlobalBVal ("[%SHOWTIN]"))
	{
	    SaveDC (CurView->hDC);
		SetDisplayMode (CurView->hDC, GF_TEXTMODE);
	    GSSiDeleteObject(&CurView->hRgn);
		CurView->hRgn = CreateVPRgn(FALSE,FALSE);
	  	SelectClipRgn (CurView->hDC,CurView->hRgn);
	  	GSSiDeleteObject(&CurView->hRgn);
	
		Point[0].x = p1->x;
		Point[0].y = p1->y;
		Point[1].x = p2->x;
		Point[1].y = p2->y;
		GWPolylineD (CurView->hDC,Point,2,0);  
		RestoreDC (CurView->hDC,-1);
	}
{
#if ENABLETRACE
GSSiExitProg (1009);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}


 

void PolylineINV (HDC hDC,LPPOINT P,short n)
#if ENABLETRACE
{GSSiEnterProg (1011);
#endif
{   
	UINT	i;
	POINT	PINV[10];
	
	for (i=0;i<n;i++)
	{
		PINV[i] = P[i];
		PINV[i].y = 30 - PINV[i].y;  
	}
	Polyline (hDC,PINV,n);
{
#if ENABLETRACE
GSSiExitProg (1011);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

BOOL AddToSpecialCursorList (HCURSOR hCurs)
#if ENABLETRACE
{GSSiEnterProg (1012);
#endif
{
	UINT	i;
	
	for (i=0;i<nSpecialCursors;i++)
		if (hCurs == SpecCursors[i])
{
#if ENABLETRACE
GSSiExitProg (1012);
#endif
			return FALSE; 
}
	if (nSpecialCursors == 32)
{
#if ENABLETRACE
GSSiExitProg (1012);
#endif
		return FALSE;
}
	SpecCursors[nSpecialCursors++] = hCurs;
{
#if ENABLETRACE
GSSiExitProg (1012);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL InSpecialCursorList (HCURSOR hCurs)
#if ENABLETRACE
{GSSiEnterProg (1013);
#endif
{
	UINT	i;
	
	for (i=0;i<nSpecialCursors;i++)
		if (hCurs == SpecCursors[i])
{
#if ENABLETRACE
GSSiExitProg (1013);
#endif
			return TRUE;
}
{
#if ENABLETRACE
GSSiExitProg (1013);
#endif
	return FALSE;
}
#if ENABLETRACE
}
#endif
}

