#include "graphint.h"   
#include "translat.h"  
#include "extrndb.h"   
#include "pnet.h"

#define	NULL_ELV	-100.0    

typedef struct {USHORT	PointID1, PointID2;} LINKLINELOC;
typedef LINKLINELOC	FAR	*LPLINKLINELOC;

#include "gmextern.h"

static	id; 
static short	VoidValue=254;

void DBFWriteError (int st)
{
	if (!st)
		GSSiMsgBox (0,"Error writing DBF file",NULL,MB_ICONEXCLAMATION,0);
	return;
}

BOOL LinesEqual (HPDPOINT p,long i1, long i2, long j1, long j2)
{   
	long	i;  
	double	TOL=P_TOL;
	
	if (LDIST (p[i1].x,p[i1].y,0,0) > LDIST (p[i2].x,p[i2].y,0,0)) 
	{
		i = i1;
		i1 = i2;
		i2 = i;
	} 
	if (LDIST (p[j1].x,p[j1].y,0,0) > LDIST (p[j2].x,p[j2].y,0,0)) 
	{
		i = j1;
		j1 = j2;
		j2 = i;
	} 
	if (LDIST (p[i1].x,p[i1].y,p[j1].x,p[j1].y) < TOL && 
		LDIST (p[i2].x,p[i2].y,p[j2].x,p[j2].y) < TOL) 
	{   
		p[i1].x = p[j1].x = (p[i1].x + p[j1].x) / 2;
		p[i1].y = p[j1].y = (p[i1].y + p[j1].y) / 2;
		return TRUE;
	}
	return FALSE;
}

 
short HaveLinkLines (HPDPOINT pPoints, long nPnts,LPHANDLE phLinks)
{   
	long i, j;
	BOOL	rtn=FALSE;  
	short	nLinks=1;    
	LPLINKLINELOC	pLinks;
	
	*phLinks = GSSiGlobAlloc ( 680,GMEM_MOVEABLE,4096);
	pLinks = (LPLINKLINELOC)GlobalLock (*phLinks);
	pLinks->PointID1 = 0;
	pLinks->PointID2 = nPnts;
	GlobalUnlock (*phLinks);
	for (i=0;i<nPnts-2;i++)
	{
		for (j=i+1;j<nPnts-1;j++)
		{
			if (LinesEqual (pPoints,i,i+1,j,j+1))
			{   
				pLinks = (LPLINKLINELOC)GlobalLock (*phLinks);
				pLinks += nLinks++;  
				pLinks->PointID1 = i;
				pLinks->PointID2 = j+1;
				GlobalUnlock (*phLinks);
			}
		}
	}
	return nLinks;
}  

BOOL PointIsBeginOfLoop (USHORT iPoint,LPUSHORT pEndOfLoop,HANDLE hLinks,short nLinks)
{
	LPLINKLINELOC	pLinks=(LPLINKLINELOC)GlobalLock (hLinks);
	BOOL	rtn=FALSE;
	short	ilink;
	
	pLinks++;
	for (ilink=1;ilink<nLinks;ilink++,pLinks++)
		if (iPoint == pLinks->PointID1)
		{
			rtn = TRUE;
			*pEndOfLoop = pLinks->PointID2;
			break;
		}
    
    GlobalUnlock (hLinks);
	return rtn;
} 

BOOL CreateUpdatePolyFromLoops (short nLoops,LPINT nLoopPoints,LPHANDLE hLoopPoints)
{   
	long		nPnts=0;
	USHORT		i, iLink;     
	HPDPOINT	pNewPolyPoints;
	
	for (i=0;i<nLoops;i++)
		nPnts += nLoopPoints[i];
	nUpdatePolyPoints = 0; 
	hUpdatePoly = GSSiGlobAlloc ( 681,GMEM_MOVEABLE,((long)nPnts+nLoops) * sizeof(DPOINT));
	pNewPolyPoints = (HPDPOINT)GlobalLock (hUpdatePoly);
    {
        long	nNewPoints=0, nLinkPoints;
        LPWORD	pMultiPolygon; 
        USHORT	nextcp=0;
       	DPOINT	FirstPoint, TiePoint;
                
    	nUpdateMultiPolygon = nLoops; 
    	hUpdateMultiPolygon = GSSiGlobAlloc ( 682,GMEM_MOVEABLE,nUpdateMultiPolygon*sizeof(WORD));
    	pMultiPolygon = (LPWORD)GlobalLock (hUpdateMultiPolygon);
        for (i=0;i<nLoops;i++)
        {   
        	BOOL	First=TRUE;
        	long np = nLoopPoints[i];
        	HPDPOINT	lpDpoint=(HPDPOINT)GlobalLock (hLoopPoints[i]); 
        	short	inc = 0;    
        	
            if (!SameDPoint (lpDpoint,&lpDpoint[np-1]))
            	inc = 1;
           	FirstPoint = *lpDpoint;  
           	if (!nUpdatePolyPoints)
           		TiePoint = FirstPoint; 
           	else
           		First = FALSE;
           	*pMultiPolygon++ = np+inc; 
            while (np--)
            {   
            	*pNewPolyPoints++ = *lpDpoint++; 
            	nUpdatePolyPoints++;
	        } 
	        if (inc)
            {   
            	*pNewPolyPoints++ = FirstPoint; 
            	nUpdatePolyPoints++;
	        } 
	        if (!First)
            {   
            	*pNewPolyPoints++ = TiePoint; 
            	nUpdatePolyPoints++; 
	        } 
	        GlobalUnlock (hLoopPoints[i]);
        }
        GlobalUnlock (hUpdateMultiPolygon);
    	if (nLoops == 1)
    		GSSiGlobFree (&hUpdateMultiPolygon);
    }
	GlobalUnlock (hUpdatePoly); 
	return TRUE;  
	
}

BOOL RemoveLinkLines (void)
{   
	short	pos=BT_FIRST;
	HIGHLIGHTDATA	HighlightData;
    LPTHEME pTheme;                                        
	long	iref,ii;  
	static	long debugref=-2145529886;
	short	nParts, nareas; 
	HANDLE	hIndex;
	LPLONG	pIndex;
	LPWORD	pPolyParts;
	char	CRef[64]; 
	long	NumItems, Done=0;
    double	SnapTol = GetGlobalDVal2 ("[%SNAPPOLYTOL]",0);
    LPMNMXCORD lpRect;
    HPDPOINT    lpDpoint, lpOutPoint,lpFirstPoint,lpUpdatePolyPoints;
	 
    NumItems = BT_NUM_IN_INDEX(hHighlight);  
    if (!NumItems)
    	return FALSE;
	CreateStatusWind (hWndMain,1,NULL);
	StatusWindowUpdate ("Remove link lines",NULL, 0,0);
	while (ContinueProcessing && !BT_FIND (hHighlight,(LPSTR)&iref,pos,BT_ANY,(LPSTR)&HighlightData))
	{   
		ltoa (iref,CRef,10);
		SetWindowText (hWndMain,CRef);  
		if (iref == debugref)
			ii=1;
		pos = BT_NEXT; 
		PickList[0]=HighlightData.PD;
		if (PickList[0].Type != 3)
			continue;
		SetConfig (PickList[0].ConfigID);
	    SetViewport (PickList[0].ViewID);
		pTheme = AddTheme (GF_SAVEPOLYPARTS_THEME);
		CurView->PassID = 4; 
		ProcessSelectedTheme = CurView->NumThemes; 
		WantUnsplinedPoints = TRUE;
		ProcessPickedItem (0,FALSE);
		WantUnsplinedPoints = FALSE; 
		ProcessSelectedTheme = 0;
		WantElement = LONG_MAX;               
		DeleteTheme (pTheme);
		nParts = GetSavedPolys (); 
//		if (hCurvePoints)
//			continue;
	    if (hSavePoly && !hSavePolyParts)
	    {   
	        long	Totp,nPnts;
	        short	nLinks, iLink;  
	        DPOINT  FirstPoint;  
	        HANDLE	hOutPoint=0, hOrigPoint=0, hLinks=0;
		    
//		    GSSiGlobFree (&hCurvePoints);                        
		    nUpdateCurvePoints = nCurvePoints;
		    hUpdateCurvePoints = hCurvePoints;
		    hCurvePoints = 0;                        
            nPnts = nSavePoly; 
            lpRect = (LPMNMXCORD) GlobalLock (hSavePoly);
            lpRect++;
            lpDpoint = lpFirstPoint = (LPDPOINT) lpRect; 
		    FirstPoint = *lpFirstPoint;                        
			SnapPolyPoints (nPnts,lpDpoint,SnapTol);
            if (!hSavePolyParts)
        		RemoveDupPolyPoints (&nPnts,lpDpoint, P_TOL);
	        nLinks = HaveLinkLines (lpDpoint,nPnts,&hLinks);
        	nUpdatePolyPoints = nPnts; 
        	hUpdatePoly = GSSiGlobAlloc ( 681,GMEM_MOVEABLE,((long)nPnts+nLinks) * sizeof(DPOINT));
        	lpUpdatePolyPoints = (HPDPOINT)GlobalLock (hUpdatePoly);
	        {
				LPLINKLINELOC	pLinks=(LPLINKLINELOC)GlobalLock (hLinks);
                long	nNewPoints=0, nLinkPoints, i;
                LPWORD	pMultiPolygon; 
                USHORT	nextcp=0;
                
            	nUpdateMultiPolygon = nLinks; 
            	hUpdateMultiPolygon = GSSiGlobAlloc ( 682,GMEM_MOVEABLE,nUpdateMultiPolygon*sizeof(WORD));
            	pMultiPolygon = (LPWORD)GlobalLock (hUpdateMultiPolygon);
                for (iLink =0;iLink < nLinks;iLink++,pLinks++)
                {
                	USHORT	iPoint,EndOfLoop,StartPoint = pLinks->PointID1;
                	USHORT	EndPoint = pLinks->PointID2; 
                	long	StartLink=nNewPoints;
                	
                	if (StartPoint)
                		StartPoint++;
                	for (iPoint =StartPoint;iPoint<EndPoint;)
                	{
                		if (PointIsBeginOfLoop (iPoint,&EndOfLoop,hLinks,nLinks))
                			iPoint = EndOfLoop;
                		*lpUpdatePolyPoints++ = lpDpoint[iPoint];
	               		if (iLink && hUpdateCurvePoints)
	               		{
	               			LPSHORT	pCurvePoints=(LPSHORT)GlobalLock (hUpdateCurvePoints);
	               			short	icp;
	               			
	               			for (icp=nextcp;icp<nUpdateCurvePoints;icp++)
	               				if (pCurvePoints[icp] == iPoint) 
	               				{
	               					pCurvePoints[icp] = nNewPoints;
	               					nextcp = icp+1; 
	               					break;
	               				}
	               			GlobalUnlock (hUpdateCurvePoints);
	               		} 
	               		iPoint++;
                		nNewPoints++;
                	}
                	if (iLink)
                	{
                		*lpUpdatePolyPoints++ = FirstPoint;
                		nNewPoints++;
	               		*pMultiPolygon = nNewPoints - StartLink -1; 
	               		pMultiPolygon++;
                	}
                	else	
               			*pMultiPolygon++ = nNewPoints - StartLink;
                }
	        	GSSiGlobUlFree (&hLinks);  
	        	if (nLinks == 1)
	        		GSSiGlobUlFree (&hUpdateMultiPolygon);
	        	else
	        	{
	        		if (hCurvePoints)
	        			ii=1;
		        	GlobalUnlock (hUpdateMultiPolygon);
		        } 
		        nUpdatePolyPoints = nNewPoints;
	        }
        	GlobalUnlock (hUpdatePoly);
			GlobalUnlock (hSavePoly); 
			UpdateItem=51;
			UpdateRecord (0,PickList[0].Desc,NULL,NULL,NULL,NULL,1,-1);
			GSSiGlobFree (&hUpdatePoly); 
		}
		else if (hSavePoly)
		{   
			UpdateItem=0;
			UpdateRecord (0,PickList[0].Desc,NULL,NULL,NULL,NULL,1,-1);
			GSSiGlobFree (&hUpdatePoly);
		} 
		DestroySavedPolys ();
		StatusWindowUpdate (NULL,NULL, NumItems,Done++);
	}
	DestroyStatusWindow(0); 
	SetContinueProcessing ( TRUE);   
	return TRUE;
} 

BOOL DecodeGMField (LPSTR FldName,LPSHORT pFldType,LPSHORT pFldLength)
{   
	LPSTR	pPar=_fstrchr (FldName,'(');
	
	if (!pPar)
		return FALSE;
	*pPar++ = 0;
	switch (*pPar)
	{
		case 'C':
			*pFldType = BT_CHAR;
			break;
		case 'I':
			*pFldType = BT_INTEGER;
			break;
		case 'R':
			*pFldType = BT_REAL;
			break;
		default:
			return FALSE;
	}
	pPar++;
	*pFldLength = atoi (pPar);
	return TRUE;
}


	                            



/*BOOL GetColorValx (LPSTR pPel,LPDPOINT pBasePoint)
{   
    POINT	WinPt;  
    COLORREF Color; 
    unsigned short	i; 
    double	X,Y;   
    HBITMAP	hOldBM;
    i = NumMemMaps;
    while (i--)
		if (DPointInBounds (pBasePoint,&MemMapBounds[i]))
			goto InBounds;
	i = NumMemMaps;
	if (NumMemMaps >= MAXMEMMAPS)
	{
		GSSiMessageBox (0,"Memory map buffer overflow",NULL,MB_ICONEXCLAMATION);
		SetContinueProcessing (FALSE);
	}
	CurView->NewBounds.xmn = pBasePoint->x - (MemMapWidth * CurView->OrthoRes)/2;
	CurView->NewBounds.xmx = pBasePoint->x + (MemMapWidth * CurView->OrthoRes)/2;
	CurView->NewBounds.ymn = pBasePoint->y - (MemMapHeight * CurView->OrthoRes)/2;
	CurView->NewBounds.ymx = pBasePoint->y + (MemMapHeight * CurView->OrthoRes)/2;
    SetBounds (CurView->hWnd,NULL);
	CurView->WindowZoomedToOrtho = TRUE;
	CurView->WindowIsZoomed = TRUE; 
	SaveCurView (0);
	PaintMap (CurView->hWnd,hdcMemMap,TRUE);  
	SaveCurView (1); 
	hMemBitmap = CreateCompatibleBitmap (hdcMemMap,MemMapWidth,MemMapHeight);
	hMemMaps[NumMemMaps] = SelectObject(hdcMemMap, hMemBitmap);
    MemMapBounds[NumMemMaps] = CurView->WBounds;
    MemMapTrans[NumMemMaps++] = CurView->hTranBaseToWin; 
    CurView->hTranBaseToWin = 0;
InBounds:	
    TRANS2 (pBasePoint->x,pBasePoint->y,&X,&Y,MemMapTrans[i]);
    WinPt.x = IDNINT (X);
    WinPt.y = IDNINT (Y); 
   	hOldBM = SelectObject (hdcMemMap,hMemMaps[i]);
    Color = GetPixel (hdcMemMap,WinPt.x,WinPt.y);   
    *pPel = GetBValue (Color);     
	SelectObject (hdcMemMap,hOldBM);
    return TRUE;
}  */

BOOL GetColorVal (LPSTR pPel,LPDPOINT pBasePoint)
{   
	LPBITMAPINFOHEADER	lpbi;
	long	col, row, nBytesPerPel, RowLen;
	HPSTR	pPixel, startimage; 
	LPORTHO	CurOrtho;
    
    if (!FindOrthoBuf (pBasePoint))
    {
    	*pPel = 0;
    	return FALSE;
    }
    CurOrtho = (LPORTHO)GlobalLock (hOrthos) + OrthoID;
    CurOrtho->LastUsed = OrthoUse++;
    
    lpbi = (LPBITMAPINFOHEADER)GetDibHeader (CurOrtho->hDib);
   	startimage = (LPSTR) lpbi + (lpbi->biSize+lpbi->biClrUsed*sizeof(COLORREF));
	col = IDNINT ((pBasePoint->x - CurOrtho->Bounds.xmn) / 
				 ((CurOrtho->Bounds.xmx - CurOrtho->Bounds.xmn) / (CurOrtho->Width - 1)));
	row = IDNINT ((pBasePoint->y - CurOrtho->Bounds.ymn) /
				 ((CurOrtho->Bounds.ymx - CurOrtho->Bounds.ymn) / (CurOrtho->Height - 1)));
	nBytesPerPel = lpbi->biBitCount/8;   
	RowLen = nBytesPerPel * lpbi->biWidth;
	RowLen += RowLen%4;
	pPixel = startimage + (row * RowLen + col * nBytesPerPel);	
	
	*pPel = *pPixel;
	if (!hDibIs32Bit (CurOrtho->hDib))
		GlobalUnlock ((HANDLE)CurOrtho->hDib);   
	GlobalUnlock (hOrthos);
	return (FALSE);
}  

BOOL FindOrthoBuf (LPDPOINT pBasePoint)
{	int i, MinOrthoID;
	static	LastOrthoID=0;
	long	MinUse;
	LPORTHO	MinOrtho;   
    MNMXCORD Bounds;
	BOOL	FirstPass=TRUE, SaveMemMap=MemMap;  
	HBITMAP	hbmpOld;
	LPORTHO	CurOrtho;
	
	if (!hOrthos)
		OpenOrthos();

Top:  
    CurOrtho = (LPORTHO)GlobalLock(hOrthos); 
    CurOrtho += LastOrthoID;
	if (*CurOrtho->Name)
	{   
		Bounds = CurOrtho->Bounds;
		InflateBounds (&Bounds,CurOrtho->Res/2);
		if (DPointInBounds (pBasePoint,&Bounds))
		{
			GlobalUnlock (hOrthos);
			return TRUE;
		}
	}
	GlobalUnlock (hOrthos);
    CurOrtho = (LPORTHO)GlobalLock(hOrthos); 
	for (OrthoID=0;OrthoID<UsedOrthoBufs;OrthoID++,CurOrtho++)
	{
		if (*CurOrtho->Name)
		{   
			Bounds = CurOrtho->Bounds;
			InflateBounds (&Bounds,CurOrtho->Res/2);
			if (DPointInBounds (pBasePoint,&Bounds))
			{
				LastOrthoID = OrthoID;
				GlobalUnlock (hOrthos);
				return TRUE;
			}
		}
	}  
	GlobalUnlock (hOrthos);
	return FALSE;
				
} 
                                
      

HANDLE GetBMPBlock (short nRows, short nCols,short nBytesPerPel,double Res,double OrthoRes,
					HANDLE hTranBlockToBase,LPBOOL NeedToClip)
{
    UINT	irow, icol, RowLen=nCols*nBytesPerPel ;  
    double	X,Y;
    LPSTR	pRow; 
    HANDLE	hBlock=GSSiGlobAlloc ( 694,GMEM_MOVEABLE,(long)nBytesPerPel * (long)nRows * (long)nCols);
    HPBYTE	pBlock;
    BOOL	SaveMemMap = MemMap; 
    DPOINT	BasePoint;  
    HBITMAP	hbmpOld;         
    POINT	WinPt;          
    COLORREF	Color, VoidColor=RGB(255,255,255);//0,127,255);  
    HDIB	hDib; 
    HDC		SaveDC;   
    LPBITMAPINFOHEADER	lpDIB;   
    HPSTR	lpBits, pBit;    
    DWORD	DIBRowLen;
	short	nc,ii;
    char	Name[128];   
    
    DBoundsInit (&CurView->NewBounds);
    X = -2;
    Y = -2;
    TRANS2 (X,Y,&BasePoint.x,&BasePoint.y,hTranBlockToBase); 
    AddDPointToMinMax (&BasePoint,&CurView->NewBounds);
    Y = nRows+1;
    TRANS2 (X,Y,&BasePoint.x,&BasePoint.y,hTranBlockToBase); 
    AddDPointToMinMax (&BasePoint,&CurView->NewBounds);
    X = nCols+1;
    TRANS2 (X,Y,&BasePoint.x,&BasePoint.y,hTranBlockToBase); 
    AddDPointToMinMax (&BasePoint,&CurView->NewBounds);
    Y = -2; 
    TRANS2 (X,Y,&BasePoint.x,&BasePoint.y,hTranBlockToBase); 
    AddDPointToMinMax (&BasePoint,&CurView->NewBounds);
    
    SaveDC = CurView->hDC;
	MemMap = TRUE;
	MemMapWidth = ((CurView->NewBounds.xmx - CurView->NewBounds.xmn)/OrthoRes) * 1.05;
	MemMapHeight = ((CurView->NewBounds.ymx - CurView->NewBounds.ymn)/OrthoRes) * 1.05; 
	hdcMemMap = CreateCompatibleDC(CurView->hDC); 
	hMemBitmap = CreateCompatibleBitmap (CurView->hDC,(int)MemMapWidth,(int)MemMapHeight);    

	hbmpOld = SelectObject(hdcMemMap, hMemBitmap);   
	CurView->WindowZoomedToOrtho = TRUE;  
	CurView->BorderPct = -1;    
	CurView->WidthType = 2;
	CurView->Width = 100;
	CurView->Height = 100;
	CurView->TagPointID = 1;
	CurView->TagPointType = 2;
	CurView->Margin = 0;
	CurView->MarginPan = FALSE;
	CurView->WindowIsZoomed = TRUE;   
	CurView->BackGroundColor = VoidColor;
//	SetGlobalValueLong ("%BACKGROUND_COLOR",VoidColor);
	SaveCurView (0);    
//	_fstrcpy (MemMapName,"c:\\mem.bmp");
	PaintMap (CurView->hWnd,hdcMemMap,TRUE,NULL,0);  
	SaveCurView (1);
    CurView->hDC = SaveDC;
/*    hDib = BitmapToDIB (hMemBitmap,0);
    sprintf (Name,"c:\\dib%i.bmp",id++);
    SaveDIB (hDib,Name); 
	DestroyDIB (hDib);*/ 
/*    lpDIB = GlobalLock (hDib); 
    DIBRowLen = lpDIB->biSizeImage/lpDIB->biHeight;  
    lpBits = FindDIBBits (lpDIB); */
    Y = 0;
	pBlock = GlobalLock (hBlock);
    for (irow = 0; irow<nRows; irow++,Y++)
    {   
    	X = 0;
    	for (icol = 0; icol<nCols; icol++,X++,pBlock+=nBytesPerPel)
    	{ 
		    TRANS2 (X,Y,&BasePoint.x,&BasePoint.y,hTranBlockToBase); 
		    WinPt = BasePtToWinPt (&BasePoint);  
/*		    pBit = lpBits;
		    pBit += (WinPt.x * nBytesPerPel + WinPt.y * DIBRowLen);
		    *pRow = *pBit;*/
		    Color = GetPixel (hdcMemMap,WinPt.x,WinPt.y); 
/*		    if (Color == VoidColor) 
		    	ii=1;  
		    if (GetBValue (Color) != GetRValue (Color))
		    	ii=1;*/
		   /* {
		    	*NeedToClip = TRUE;
		    	*pBlock = VoidValue;
		    }
		    else */             
		    {
	    		*pBlock = GetBValue (Color);  
	    		if (nBytesPerPel == 3)
	    		{
	    			*(pBlock+1) = GetGValue (Color);  
	    			*(pBlock+2) = GetRValue (Color);  
	    		}   
//	    		if (*pBlock == VoidValue)
//	    			(*pBlock)--;
	    	}
    	}  
    }
   	GlobalUnlock (hBlock);
	SelectObject(hdcMemMap, hbmpOld);   
	GSSiDeleteObject (&hMemBitmap);
	DeleteDC (hdcMemMap); 
	hdcMemMap = 0;
//	GlobalUnlock (hDib);
//	DestroyDIB (hDib); 
    MemMap = SaveMemMap;
	return hBlock;
}

BOOL ClipBMP (LPSTR Name)
{   
	HFILE	FidIn, FidOut;
	OFSTRUCTGM	OFStruct;
    BITMAPINFOHEADER DibInfoIn, DibInfoOut;
    BITMAPFILEHEADER bmfHeadIn, bmfHeadOut;
    RECT	ClipRect;    
    UINT	irow, icol, BytesPerPel;     
    long	loc, startimage, InRowLen, OutRowLen;      
    HANDLE	hRow;
    LPSTR	pRow;
	
	return TRUE;
	FidIn = GSSiOpenFile (Name,&OFStruct,OF_READ);
	BigRead (FidIn,(HPSTR)&bmfHeadIn,sizeof(BITMAPFILEHEADER));
	BigRead (FidIn,(HPSTR)&DibInfoIn,sizeof(BITMAPINFO)); 
	
	for (irow = 0;irow < DibInfoOut.biHeight;irow++)
	{   
		loc = startimage + (long)ClipRect.bottom * InRowLen + ClipRect.left;
		GSSillseek (FidIn,loc,0); 
		hRow = GSSiGlobAlloc ( 695,GHND,OutRowLen);
		pRow = GlobalLock (hRow);
		BigRead (FidIn,pRow,(long)DibInfoOut.biWidth*BytesPerPel);
		BigWrite (FidOut,pRow,OutRowLen,-1);                                                         
		GSSiGlobUlFree (&hRow);
	}
	return TRUE;
}

void ConvertCoordBMP (LPDPOINT pDPoint)
{   
	if (hTranExport[0])
		TRANS2 (pDPoint->x,pDPoint->y,&pDPoint->x,&pDPoint->y,hTranExport[0]);  
	else
		ConvertCoord(pDPoint,1,3);
	return;
}    

void ConvertCoordBMP2 (LPDPOINT pDPoint)
{   
	if (hTranExport[1])
		TRANS2 (pDPoint->x,pDPoint->y,&pDPoint->x,&pDPoint->y,hTranExport[1]);  
	else
		ConvertCoord(pDPoint,3,1);
	return;
}    


                                  
BOOL CreateBMPs (LPSTR Name,double Res,double BaseRes,double Offset,HWND hWndDlg,LPSTR project,short units)
{   
	short	nBlockRows, nBlockCols, DesiredBlockHeight=128, DesiredBlockWidth=128; 
	long	iref, RowLen, NumItems, Item=1, FileSize, StartImage, bsize; 
	HIGHLIGHTDATA	HighlightData;  
	HANDLE	hPoints;
	HPDPOINT	pPoints;
	MNMXCORD	MnMx;
	short	Nump; 
	short	width, height;
    LPBITMAPINFO    pDibInfo, pDibInfoOut;
	HANDLE  hDibInfoOut,hRow;
    BITMAPFILEHEADER bmfHead;
    WORD    HeadLen, irow, icol, i; 
    HFILE	FidBM, FidBPW;
	OFSTRUCTGM	OFStruct;
    LPSTR	pRow;   
    double	X, Y, XFROM[4],YFROM[4],XTO[4],YTO[4];
    DPOINT	BasePoint, DPoint; 
    short	nBytesPerPel,ii; 
    HANDLE	hTran;  
    float	RSQMIN;
   	HBITMAP	hbmpOld;  
   	char	BPWName[128], str[256];  
   	LPSTR	lpDot; 
   	BOOL	rtn=FALSE; 
   	DPOINT	OutSysPoint; 
   	long	TotBlocks, iBlock, BlockRowLen;  
   	short	iBlockRow, iBlockCol, BlockHeight, BlockWidth; 
   	HANDLE	hBlock;          
   	HPSTR	pBlock; 
   	BOOL	NeedToClip, SaveD16BC = Display16BitColor;
   	LPORTHO	CurOrtho;
	
	bsize = GetGlobalLVal ("[%CVTBLOCKSIZE]");
	if (bsize)
		DesiredBlockHeight = DesiredBlockWidth = bsize;	
	if (!hOrthos)
		return FALSE; 
	CurOrtho = (LPORTHO)GlobalLock (hOrthos);  
	Display16BitColor = FALSE;
	hDibFree (&CurOrtho->hDib);
	if (!LoadBitMap (CurOrtho->Name,CurOrtho->Frame, &CurOrtho->hDib, &CurOrtho->DeleteBM,
					 &CurOrtho->BitCount,&CurOrtho->Width, &CurOrtho->Height))
	{
		GlobalUnlock (hOrthos);
		return FALSE;	       
	}
	pDibInfo = (LPBITMAPINFO)GetDibHeader (CurOrtho->hDib);  
	if (!pDibInfo)
	{
		GlobalUnlock (hOrthos);
		return FALSE;	       
	}
    HeadLen = sizeof(BITMAPINFOHEADER)+pDibInfo->bmiHeader.biClrUsed*sizeof(RGBQUAD);
    hDibInfoOut = GSSiGlobAlloc ( 696,GHND,HeadLen);
    pDibInfoOut = (LPBITMAPINFO)GlobalLock (hDibInfoOut); 
    _fmemmove (pDibInfoOut,pDibInfo,HeadLen); 
    if (!hDibIs32Bit (CurOrtho->hDib))
    	GlobalUnlock ((HANDLE)CurOrtho->hDib);      
	GlobalUnlock (hOrthos);
	CloseOrthos (TRUE);   
    NumItems = BT_NUM_IN_INDEX(hHighlight);
    id = 101;  
    while (!BT_FIND (hHighlight,(LPSTR)&iref,BT_FIRST,BT_ANY,(LPSTR)&HighlightData)&&ContinueProcessing)
    {   
    	RECT	Rect;
    	
        SetGlobalValue("%ALT_PROJECTION",project);
	    ConvertCoordClose ();
		ConvertCoordInit();
        PRJ_UNITS[3] = units;
        NeedToClip = FALSE;
        PickList[0]=HighlightData.PD;
		SetPickGlobals (0);
		CloseOrthos(TRUE);
		CurView->NewBounds = PickList[0].Rect;
		InflateBounds (&CurView->NewBounds,Offset+1);
		  
	    hPoints = GSSiGlobAlloc ( 697,GMEM_MOVEABLE,(long)PickList[0].NumPoints*sizeof(DPOINT)); 
	    pPoints = (HPDPOINT)GlobalLock (hPoints);
		Nump = GetPickItemPoints (0,FALSE,&pPoints); 
		GlobalUnlock (hPoints);  
		if (!Nump)
			return FALSE;
	    pPoints = (HPDPOINT)GlobalLock (hPoints);
		DBoundsInit (&MnMx); 
		while (Nump--)
		{
			ConvertCoordBMP(pPoints);   
			AddToMinMaxD (&MnMx,pPoints++);
		} 
		GSSiGlobUlFree (&hPoints);
		InflateBounds (&MnMx,Offset);
		MnMx.xmn = floor (MnMx.xmn / Res) * Res;
		MnMx.xmx = ceil (MnMx.xmx / Res) * Res;
		MnMx.ymn = floor (MnMx.ymn / Res) * Res;
		MnMx.ymx = ceil (MnMx.ymx / Res) * Res;
		width = IDNINT((MnMx.xmx - MnMx.xmn) / Res) + 1; 
//		width += width%4;
		height = IDNINT((MnMx.ymx - MnMx.ymn) / Res) + 1;   
//		height += height%4;  
		
		_fstrcpy (BPWName,Name);
		if ((lpDot=_fstrrchr(BPWName,'.')))
			*lpDot = 0;
		_fstrcat (BPWName,".BPW");
		makedirectories (BPWName,FALSE,FALSE);
		FidBPW = GSSiOpenFile (BPWName,&OFStruct,OF_CREATE); 
		if (FidBPW == HFILE_ERROR)
		{
		    GSSiMsgBox(0,BPWName,"Unable to create output file",
		               MB_OK|MB_ICONQUESTION|MB_TASKMODAL,0);  
			goto Exit;
		}	
		SetDlgItemText (hWndDlg,IDC_TOT_ITEMS,OFStruct.szPathName);
		sprintf (str,"%f",Res);
		fputstring (str,FidBPW);
		fputstring ("0",FidBPW);
		fputstring ("0",FidBPW);
		sprintf (str,"%f",-Res);
		fputstring (str,FidBPW);
		sprintf (str,"%f",MnMx.xmn);
		fputstring (str,FidBPW);
		sprintf (str,"%f",MnMx.ymx);
		fputstring (str,FidBPW);
		GSSiClose2 (&FidBPW);
		
		
        nBytesPerPel = pDibInfoOut->bmiHeader.biBitCount/8;
        pDibInfoOut->bmiHeader.biWidth = width;
        pDibInfoOut->bmiHeader.biHeight = height;
		RowLen = pDibInfoOut->bmiHeader.biWidth *(long)nBytesPerPel;  
		if (RowLen%4)
			RowLen += 4-RowLen%4;
		pDibInfoOut->bmiHeader.biSizeImage = (long)pDibInfoOut->bmiHeader.biHeight * RowLen;

		FidBM = GSSiOpenFile (Name,&OFStruct,OF_CREATE);
		if (FidBM == HFILE_ERROR)
		{
		    GSSiMsgBox(0,Name,"Unable to create output file",
		               MB_OK|MB_ICONQUESTION|MB_TASKMODAL,0);  
			goto Exit;
		}	
		bmfHead.bfType = 19778;
		bmfHead.bfSize = sizeof(BITMAPFILEHEADER) + HeadLen + pDibInfoOut->bmiHeader.biSizeImage;
		bmfHead.bfReserved1 = 0;
		bmfHead.bfReserved2 = 0;
		bmfHead.bfOffBits = HeadLen + sizeof(BITMAPFILEHEADER);
		BigWrite (FidBM,(char *)&bmfHead,sizeof(BITMAPFILEHEADER),-1);
		BigWrite (FidBM,(char *)pDibInfoOut,HeadLen,-1);
		StartImage = GSSillseek (FidBM,0,2); 
		FileSize = StartImage + pDibInfoOut->bmiHeader.biSizeImage;
		GSSiChangeLength (FidBM,FileSize);       
		
		nBlockRows = (pDibInfoOut->bmiHeader.biHeight -1)/DesiredBlockHeight + 1;
		nBlockCols = (pDibInfoOut->bmiHeader.biWidth -1)/DesiredBlockWidth + 1;
		TotBlocks = nBlockRows * nBlockCols;  
		iBlock = 0;
		for (iBlockRow = 0; iBlockRow < nBlockRows; iBlockRow++)
		{
			if (iBlockRow+1 == nBlockRows)
				BlockHeight = (pDibInfoOut->bmiHeader.biHeight-1) % DesiredBlockHeight + 1;
			else
				BlockHeight = DesiredBlockHeight;
			for (iBlockCol = 0; iBlockCol < nBlockCols; iBlockCol++)
			{
                SetGlobalValue("%ALT_PROJECTION",project);
			    ConvertCoordClose ();
				ConvertCoordInit();
                PRJ_UNITS[3] = units;
				if (iBlockCol+1 == nBlockCols)
					BlockWidth = (pDibInfoOut->bmiHeader.biWidth-1) % DesiredBlockWidth + 1;
				else
					BlockWidth = DesiredBlockWidth;
			    BlockRowLen = BlockWidth * nBytesPerPel;
			    
				XFROM[0] = 0;
				YFROM[0] = 0;
			    OutSysPoint.x = MnMx.xmn + (iBlockCol * DesiredBlockWidth) * Res;
			    OutSysPoint.y = MnMx.ymn + (iBlockRow * DesiredBlockHeight) * Res;
				ConvertCoordBMP2(&OutSysPoint);   
			    XTO[0] = OutSysPoint.x;
			    YTO[0] = OutSysPoint.y;   
			    
				XFROM[1] = 0;
				YFROM[1] = BlockHeight;
			    OutSysPoint.x = MnMx.xmn + (iBlockCol * DesiredBlockWidth) * Res;
			    OutSysPoint.y = MnMx.ymn + (BlockHeight + iBlockRow * DesiredBlockHeight) * Res;
				ConvertCoordBMP2(&OutSysPoint);   
			    XTO[1] = OutSysPoint.x;
			    YTO[1] = OutSysPoint.y;   
			    
				XFROM[2] = BlockWidth;
				YFROM[2] = BlockHeight;
			    OutSysPoint.x = MnMx.xmn + (BlockWidth + iBlockCol * DesiredBlockWidth) * Res;
			    OutSysPoint.y = MnMx.ymn + (BlockHeight + iBlockRow * DesiredBlockHeight) * Res;
				ConvertCoordBMP2(&OutSysPoint);   
			    XTO[2] = OutSysPoint.x;
			    YTO[2] = OutSysPoint.y;   
			    
				XFROM[3] = BlockWidth;
				YFROM[3] = 0;
			    OutSysPoint.x = MnMx.xmn + (BlockWidth + iBlockCol * DesiredBlockWidth) * Res;
			    OutSysPoint.y = MnMx.ymn + (iBlockRow * DesiredBlockHeight) * Res;
				ConvertCoordBMP2(&OutSysPoint);   
			    XTO[3] = OutSysPoint.x;
			    YTO[3] = OutSysPoint.y;   
			    
				hTran = STRAN2 (1605,XFROM,YFROM,XTO,YTO,4,&RSQMIN,1,NULL);  
			    
				hBlock = GetBMPBlock (BlockHeight,BlockWidth,nBytesPerPel,Res,BaseRes,hTran,&NeedToClip);
				GSSiGlobFree (&hTran);  
				pBlock = GlobalLock (hBlock);
				for (irow=0;irow<BlockHeight;irow++)
				{   
					long	Offset=StartImage +
								   ((long)irow + (long)iBlockRow * (long)DesiredBlockHeight) * RowLen + 
								   ((long)iBlockCol * (long)DesiredBlockWidth) * nBytesPerPel;
								   
					GSSillseek (FidBM,Offset,0);
					if (!BigWrite (FidBM,pBlock,BlockRowLen,-1))
					{
					    GSSiMsgBox(0,"Error writing bitmap - disk may be full",
					               "Fatal Write Error",MB_OK|MB_ICONQUESTION|MB_TASKMODAL,0);  
					    GSSiGlobUlFree (&hBlock);
					    GSSiClose2 (&FidBM);
					    return FALSE;
					}
					pBlock += BlockRowLen;
				} 
	            PctBox (GetDlgItem(hWndDlg,IDC_STATUS), TotBlocks, iBlock++,0); 
	            if (!ContinueProcessing)
            		goto Exit;
				GSSiGlobUlFree (&hBlock); 
			}
		}
		rtn = TRUE; 
		if (NeedToClip)
			ClipBMP (Name); 
Exit:
		RemoveFromHighlightList (iref,0);
		GSSiClose2 (&FidBM);
        PctBox (GetDlgItem(hWndDlg,IDC_STATUS2),NumItems,Item++,0); 
		CloseOrthos(TRUE);
    } 
    GSSiGlobUlFree (&hDibInfoOut);
	CloseOrthos(TRUE);
	Display16BitColor = SaveD16BC;
    return rtn;
}


 

void ConvertCoordExport (LPDPOINT pDPoint)
{   
	if (hTranExport[0])
		TRANS2 (pDPoint->x,pDPoint->y,&pDPoint->x,&pDPoint->y,hTranExport[0]);
	if (hTranExport[1])
		TRANS2 (pDPoint->x,pDPoint->y,&pDPoint->x,&pDPoint->y,hTranExport[1]);
	ConvertCoord(pDPoint,1,3);
	return;
}    

void TIGEROut(void)
{
    long    Num, TLID, Frame, Offset, Refno,SNum, AltNum,NextAltNum=1;
    char    StreetName[34], str[128], OutRec[256], FileName[MAX_PATH];
    short      st, st2, st3, stn,  Version, hltpos,i, ipos;
	BTVARDESC BTVar[2];
	OFSTRUCTGM    OFStruct;
    HCURSOR hcurSave; 
    BOOL    Good, FirstMap=TRUE, OpenedSeg=FALSE;
    long    NewFileMarker=LONG_MIN, TLIDOut=1;  
    HFILE   FidTIGER1, FidTIGER2, FidTIGER4, FidTIGER5, hData=0; 
    HANDLE  hBT=0, hMultStreets;
    DPOINT  FromPoint, ToPoint;
//    TIGER1_PEOPLENET    Segdata;
    HIGHLIGHTDATA       HighlightData;
    HANDLE hSymbol;
    LPLONG	pMultNum;   
    LPSYMBOL    pSymbol;
    short     HaveState=-1;
	SEGDATAGM Segdata;
	TIGER4		Tiger4; 
	TIGER5		Tiger5; 
	short		nMultNames; 
	long	EndLoc,Loc;
	
    if (!hHighlight)
    {
        GSSiMsgBox( GetFocus(),"No Highlight List","Error", MB_OK,0);
        return;
    }  
    hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT));


	CreateStatusWind (hWndMain,1,"Create TIGER Extract");
    OpenSymDict (OF_READ);     
	OpenStreetSegmentTable (FALSE,&OpenedSeg);  
	sprintf (FileName,"[%DL]tigerout\\extract%s",GetTIGERFileExtension('1'));
    FidTIGER1 = GSSiOpenFile (FileName,&OFStruct,OF_CREATE);
	sprintf (FileName,"[%DL]tigerout\\extract%s",GetTIGERFileExtension('2'));
    FidTIGER2 = GSSiOpenFile (FileName,&OFStruct,OF_CREATE);
	sprintf (FileName,"[%DL]tigerout\\extract%s",GetTIGERFileExtension('4'));
    FidTIGER4 = GSSiOpenFile (FileName,&OFStruct,OF_CREATE);
	sprintf (FileName,"[%DL]tigerout\\extract%s",GetTIGERFileExtension('5'));
    FidTIGER5 = GSSiOpenFile (FileName,&OFStruct,OF_CREATE);
	BTVar[0].BT_VARTYP=BT_INTEGER;
	BTVar[0].BT_VARLEN=4;
	BTVar[0].BT_VAROFF=0;
	BT_CREATE ("tiger5.tmp", 4, FALSE, 1, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
	hMultStreets = BT_OPEN ("tiger5.tmp", 0, BT_WRITE, 0);
    Good = FALSE; 
    hltpos = BT_FIRST;  
    Loc = 0;
    EndLoc = BT_NUM_IN_INDEX (hHighlight);
	StatusWindowUpdate (NULL,"Creating tables 1,2 and 4", EndLoc,Loc);
    while (!BT_FIND (hHighlight,(LPSTR)&Refno,hltpos,BT_ANY,(LPSTR)&HighlightData))
    {  
    	TLID = TLIDOut++;
    	TLID = Refno;
        hltpos = BT_NEXT;  
        PickList[0]=HighlightData.PD;  
        if (PickList[0].Type != 2)
            goto NextTLID;
	    ProcessPickedItem (0,FALSE);
 /*        if (HaveState != CurState)
        {   
            char    str[8];
                
           if (hData)
                GSSiClose2 (&hData); 
            hData = 0;
            BT_CLOSE (hBT);
            if (!(hBT = BT_OPEN ("[STATE]\\tiger1.btr", 0, BT_READ, 0)))
                goto NextTLID;
            hData = GSSiOpenFile ("[STATE]\\tiger1.dat",&OFStruct,OF_READ); 
            CloseGSStreetNames();
            OpenGSStreetNames (BT_READ,1);
            HaveState = CurState;
        } */
/*            BT_FIND(hBT,(LPSTR)&TLID,BT_FIRST,BT_EQ,(LPSTR)&Offset);
            GSSillseek (hData,Offset,0);
            BigRead (hData,&Segdata,sizeof(Segdata));  */
            
//            if (BT_FIND (hNames1,(LPSTR)&CurStreetNumbers[0],BT_FIRST,BT_EQ,(LPSTR)&StreetName))
            _fmemset (OutRec,' ',230);
            OutRec[0]='1';
            OutRec[4]='0';
            sprintf (str,"%10ld",TLID);
            _fmemmove (&OutRec[5],str,_fstrlen(str));
            
            if ((hSymbol = GetDictSymDesc (HighlightData.PD.Desc,0)))
            {
            	pSymbol = (LPSYMBOL)GlobalLock (hSymbol);
            	_fmemmove (&OutRec[55],pSymbol->Name,3); 
		        GlobalUnlock (hSymbol);
	            DestroySymbol (hSymbol); 
            }
            else
            	_fmemmove (&OutRec[55],"???",3);
            SNum = CurStreetNumbers[0];
            pMultNum = &CurStreetNumbers[1];
			if (GetSegDataGM (Refno,&Segdata))
			{   
				SNum = Segdata.StreetNum[0]; 
				pMultNum = &Segdata.StreetNum[1];
	            sprintf (str,"%11ld%11ld%11ld%11ld",Segdata.faddl,Segdata.taddl,
	                                                Segdata.faddr,Segdata.taddr);
/*	            i=2;
	            while (i--)
	                if (Segdata.CFCC[i]==0) Segdata.CFCC[i]=' ';                                                    
	            _fmemmove (&OutRec[55],Segdata.CFCC,3); */
	            _fmemmove (&OutRec[58],str,_fstrlen(str)); 
	            if (Segdata.ZIPL)
	            {
	                if (Segdata.ZIPL < 0)
	                	CanZipNumToChar (Segdata.ZIPL,str);
	                else
	                	ltoa (Segdata.ZIPL,str,10);
	                _fmemmove (&OutRec[106],str,_fstrlen(str));
	            }
	            if (Segdata.ZIPR)
	            {
	                if (Segdata.ZIPR < 0)
	                	CanZipNumToChar (Segdata.ZIPR,str);
	                else
	                	ltoa (Segdata.ZIPR,str,10);
	                _fmemmove (&OutRec[111],str,_fstrlen(str));
	            }
	            if (Segdata.FMCDL)
	            {
	                ltoa (Segdata.COUNTYL,str,10);
	                _fmemmove (&OutRec[134],str,_fstrlen(str));
	            }
	            if (Segdata.FMCDR)
	            {
	                ltoa (Segdata.COUNTYR,str,10);
	                _fmemmove (&OutRec[137],str,_fstrlen(str));
	            }
	            if (Segdata.FMCDL)
	            {
	                ltoa (Segdata.FMCDL,str,10);
	                _fmemmove (&OutRec[140],str,_fstrlen(str));
	            }
	            if (Segdata.FMCDR)
	            {
	                ltoa (Segdata.FMCDR,str,10);
	                _fmemmove (&OutRec[145],str,_fstrlen(str));
	            }
	
			}
    		GetTrueStreetName (SNum,StreetName,CurState,0);
    		if (*StreetName != '?')
	            _fmemmove (&OutRec[19],StreetName,_fstrlen(StreetName));  
	        nMultNames = 0;
            _fmemset (&Tiger4,' ',sizeof(TIGER4));   
            Tiger4.RT = '4';
            Tiger4.VERSION[3] = '0';
            IWRITE (TLID,Tiger4.TLID,sizeof(Tiger4.TLID));
            Tiger4.RTSQ[2] = '1'; 
            Tiger4.CRLF[0] = '\r';
            Tiger4.CRLF[1] = '\n';
	        for (i=1;i<4;i++)
	        {
	            SNum = *pMultNum++; 
	            if (SNum)
	            {   
	            	if (BT_FIND (hMultStreets,(LPSTR)&SNum,BT_FIRST,BT_EQ,(LPSTR)&AltNum))
	            	{
	            		AltNum = NextAltNum++;
	            		BT_PUT (hMultStreets,(LPSTR)&SNum,(LPSTR)&AltNum);
	            	}
	    			IWRITE (AltNum,Tiger4.FEAT[nMultNames++],sizeof(Tiger4.FEAT[0]));
			    }
		    }
		    if (nMultNames)
		    	BigWrite (FidTIGER4,(HPSTR)&Tiger4,sizeof(TIGER4),-1);
            {
                LPTHEME     pTheme, SaveTheme;
                short SavePFN; 
                                              
                SaveTheme = CurTheme;
				SetConfig (PickList[0].ConfigID);
			    SetViewport (PickList[0].ViewID);
                pTheme = AddTheme (GF_SAVEPOLY_THEME); 
                CurView->PassID = 4;
                IgnoreBounds = TRUE; 
				ProcessSelectedTheme = CurView->NumThemes;
				ProcessPickedItem (0,FALSE); 
				ProcessSelectedTheme = 0;       		
                IgnoreBounds = FALSE;          
                DeleteTheme (pTheme);
                CurTheme = SaveTheme;
    			while (GetSavedPolys ())
                if (hSavePoly) 
                {
                    LPDPOINT    lpDpoints, lpDpoints2; 
                    LPMNMXCORD  lpRect;
                    short     nPnts,i; 
                    char    OutRec2[256];
                    short     ninrec=0, irec;
        
                    nPnts = nSavePoly;
                    lpRect = (LPMNMXCORD) GlobalLock (hSavePoly); 
                    lpRect++;
                    lpDpoints = (LPDPOINT) lpRect; 
                    ConvertCoord (lpDpoints, 1,2); 
                    FromPoint = *lpDpoints++; 
                    _fmemset (OutRec2,' ',230); 
                    OutRec2[0]='2';
                    OutRec2[4]='0';
                    irec=1;
                    sprintf (str,"%10ld%3i",TLID,(int)irec); 
                    _fmemmove (&OutRec2[5],str,_fstrlen(str));
                    
                    for (i=1;i<nPnts-1;i++,lpDpoints++)
                    {   
                        BOOL    HaveRec=FALSE;
                        
                        ConvertCoord (lpDpoints, 1,2); 
                        sprintf (str,"%10ld%9ld",
                                IDNINT(lpDpoints->x*1000000), 
                                IDNINT(lpDpoints->y*1000000));
                        _fmemmove (&OutRec2[ninrec*19+18],str,_fstrlen(str)); 
                        ninrec++;
                        if (ninrec==10)
                        {
                            OutRec2[208]=0;
                            fputstring(OutRec2,FidTIGER2);
                            _fmemset (OutRec2,' ',230);
                            OutRec2[0]='2';
                            OutRec2[4]='0';
                            irec++;
                            sprintf (str,"%10ld%3i",TLID,(int)irec); 
                            _fmemmove (&OutRec2[5],str,_fstrlen(str));
                            ninrec = 0;
                        }
                        
                    }
                    if (ninrec)
                    {
                        OutRec2[208]=0;
                        fputstring(OutRec2,FidTIGER2);
                    }
                    ConvertCoord (lpDpoints, 1,2); 
                    ToPoint = *lpDpoints;
	                GlobalUnlock (hSavePoly);
                }
                sprintf (str,"%10ld%9ld%10ld%9ld",
                        IDNINT(FromPoint.x*1000000), 
                        IDNINT(FromPoint.y*1000000), 
                        IDNINT(ToPoint.x*1000000), 
                        IDNINT(ToPoint.y*1000000));
                _fmemmove (&OutRec[190],str,_fstrlen(str));
                OutRec[228]=0;
                fputstring(OutRec,FidTIGER1);
            }
NextTLID:
		StatusWindowUpdate (NULL,NULL, EndLoc,++Loc);
    } 
    Good = TRUE;
Exit:
    GSSiClose2 (&FidTIGER1);
    GSSiClose2 (&FidTIGER2);
    GSSiClose2 (&FidTIGER4); 
    ipos = BT_FIRST; 
    Loc = 0;
    EndLoc = BT_NUM_IN_INDEX (hMultStreets);
	StatusWindowUpdate (NULL,"Creating multiple street name table (TIGER5)", EndLoc,Loc);
	while (!BT_FIND (hMultStreets,(LPSTR)&SNum,ipos,BT_ANY,(LPSTR)&AltNum))
	{   
		ipos = BT_NEXT;
		GetTrueStreetName (SNum,StreetName,CurState,0);
        _fmemset (&Tiger5,' ',sizeof(TIGER5));   
        Tiger5.RT = '5';
        IWRITE (AltNum,Tiger5.FEAT,sizeof(Tiger5.FEAT));
        _fmemmove (Tiger5.FENAME,StreetName,_fstrlen(StreetName));
        Tiger5.CRLF[0] = '\r';
        Tiger5.CRLF[1] = '\n'; 
        BigWrite (FidTIGER5,(HPSTR)&Tiger5,sizeof(TIGER5),-1);
		StatusWindowUpdate (NULL,NULL, EndLoc,++Loc);
	}

    GSSiClose2 (&FidTIGER5);   
    BT_CLOSEANDDELETE (&hMultStreets);
    CloseStreetSegmentTable(OpenedSeg);                  
    if (hData)
        GSSiClose2 (&hData); 
    hData = 0;
    BT_CLOSE (hBT);
    CloseGSStreetNames();
    GSSiSetCursor (hcurSave); 
	CloseStreetNameTable ();
	DestroyStatusWindow(0);  
    if (Good)
        GSSiMsgBox( GetFocus(), "TIGER Extract Complete to [%DL]tigerout","", MB_OK,0);
    else
        GSSiMsgBox( GetFocus(), "Extract Aborted","", MB_OK,0); 
    setDoPaint( TRUE);
    return;
} 



