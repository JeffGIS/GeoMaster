#include "graphint.h"

static MNMXCORD	RouteBounds;
static BOOL	SkipHighlightedItems;
static HANDLE	hDBPolyID;
static BOOL	LimitRouteToScreen=TRUE;
static double	UniqueCost;
static double	UniqueCostInc=0.0000000001;
static long	WhenAssigned;
static BOOL	RouteByNet=FALSE;
static char		RouteNextFile[128];
static char		RouteAtFile[128];
static ATDATA	LastAtData;
static NEXTDATA	NextData;
static NEXTKEY	NextKey;
static long	FromRef;
static long	ToRef;   

#include "gmextern.h"

 

int AddNextRefs (long AtRef,DPOINT AtPoint,long AtPath,double AtMP,short AtDir,
				 long Start, double StartCost, DPOINT ToPoint,long Sequence, long FromRef)
{   int	NumNext=0, OrigNumPicked, np, NumInt=0,ii; 
	double	dist1, dist2, IntAZ[64], IntPCT[64], IntLength[64];
	long	IntRefs[64], IntStreets[64], JoinRef;
	BOOL	Join=FALSE, HaveNonNet=FALSE, SkipNetCheck=FALSE;
	ATDATA		AtData;
	NEXTDATA	NextData;       
	NEXTKEY		NextKey;  
	NETINTREFKEY	NetIntRefKey;
	NETINTREFDATA	NetIntRefData;   
	NETREFSKEY	NetRefsKey;
	NETREFSDATA NetRefsData; 
	NETINTMPKEY NetIntMPKey;
	NETLINKSKEY NetLinksKey;
	NETLINKSDATA NetLinksData;
	SEGDATAGM SegdataNext, SegdataAt;
	HIGHLIGHTDATA	HighlightData; 
	PICKDATA	SavePick0;
	long	AtInt;
	static	long	DebugRef=-2147436847;
	short	i, st, pos, NumAtInt; 
	long	PickPaths[64];
	short	PickDirs[64];
	double	PickMPs[64];   
	BOOL	TravelReverseNextSeg;
	double	Speed=30, IntersectionCost=0, SegmentCost, MinCostToEnd, DistFactor=1;
    
    AddDPointToMinMax (&AtPoint,&RouteBounds);
    PickOnlyEndPoints = TRUE;
    if (!ShortestRoute)
    	DistFactor = (MFT*60)/5280; 
    	
    if (AtRef == DebugRef)
       	ii=1;
Top:        
        if (LimitRouteToScreen && !PtInWBounds(&AtPoint))
        	goto Exit;
        if (RouteByNet)
        {   
        	NumPicked = 0;
			if (!NetworkID || !hBTNetRefs)
				goto DonePick;  
			if ((AtInt = AtNetIntersection (AtPath, AtMP-NetTOL, AtMP+NetTOL,FALSE)))
			{   
				NumAtInt = 0;
				NetIntRefKey.IntID = AtInt;
				NetIntRefKey.Refno = LONG_MIN;
				st = BT_FIND (hBTNetIntRef,(LPSTR)&NetIntRefKey,BT_FIRST,BT_GE,(LPSTR)&NetIntRefData); 
				while (!st && NetIntRefKey.IntID == AtInt)
				{   
					if (AtRef != NetIntRefKey.Refno)
					{   
						NetLinksKey.Ref = NetIntRefKey.Refno;
						NetLinksKey.NetID = NetworkID;  
						NetLinksKey.Path = 0;  
						st = BT_FIND (hBTNetLinks,(LPSTR)&NetLinksKey,BT_FIRST,BT_GE,(LPSTR)&NetLinksData);
						if (!st && NetLinksKey.Ref == NetIntRefKey.Refno && NetLinksKey.NetID == NetworkID)
						{
							if (NumAtInt)
							{   
								PickMPs[NumAtInt] = PickMPs[0];
								PickPaths[NumAtInt] = PickPaths[0];
								PickDirs[NumAtInt] = PickDirs[0];
								PickList[NumAtInt] = PickList[0]; 
							}
							NumAtInt++;
							if ((NetLinksData.Length < 0 && NetIntRefData.WhichEnd == 1) ||  
								(NetLinksData.Length > 0 && NetIntRefData.WhichEnd == 2))
							{
								PickDirs[0] = -1;
								PickMPs[0] = NetLinksData.MP; 
							}
							else
							{
								PickDirs[0] = 1;
								PickMPs[0] = NetLinksData.MP + fabs(NetLinksData.Length);
							}
							PickPaths[0] = NetLinksKey.Path;
							SavePick0 = PickList[0];  
							PickByRefno (NetIntRefKey.Refno,0,0,FastPickFileNum);
						}
					}
					st = BT_FIND (hBTNetIntRef,(LPSTR)&NetIntRefKey,BT_NEXT,BT_ANY,(LPSTR)&NetIntRefData); 
				} 
				NumPicked = NumAtInt;
			} 
			else
			{
				NetRefsKey.Path = AtPath;
				NetRefsKey.MP = AtMP-NetTOL;
				st = BT_FIND (hBTNetRefs,(LPSTR)&NetRefsKey,BT_FIRST,BT_GT,(LPSTR)&NetRefsData); 
				if (st || NetRefsKey.Path != AtPath)
					goto DonePick;
				if (NetRefsData.Ref == AtRef) 
				{ 
					if (AtDir < 0)  
						pos = BT_PRIOR;
					else
						pos = BT_NEXT;
					st = BT_FIND (hBTNetRefs,(LPSTR)&NetRefsKey,pos,BT_ANY,(LPSTR)&NetRefsData); 
					if (st || NetRefsKey.Path != AtPath)
						goto DonePick;
					if (NetRefsData.Ref == AtRef)
					{
						AtMP = NetRefsKey.MP;
						goto Top;
					}
				} 
				PickPaths[0] = NetRefsKey.Path;
				PickMPs[0] = NetRefsKey.MP;
				PickDirs[0] = AtDir;
				PickByRefno (NetRefsData.Ref,0,0,FastPickFileNum); 
				if (PickDirs[0] > 0)
					PickMPs[0] += PickList[0].Length;
			}			
        }
        else
	    	PickItems2 (hWndMain,AtPoint,FALSE,TRUE,TRUE);  

DonePick:
	    OrigNumPicked = NumPicked; 
	    if (NumPicked && !ShortestRoute)
   			GetSegDataGM (AtRef,&SegdataAt);
	    for (i=NumPicked-1;i>=0;i--)
	    {
	    	if (PickList[i].Type == 1 && PickList[i].Refno != AtRef)
	    	{
	    		NumPicked = i+1;
	    		break;
	    	}
	    }
	    while (NumPicked--)
	    {   
	    	if (SkipHighlightedItems)
	    	{
	    		if (!BT_FIND (hHighlight,(LPSTR)&PickList[NumPicked].Refno,BT_FIRST,BT_EQ,(LPSTR)&HighlightData))
	    			goto Next;
	    	}
 	    	if (PickList[NumPicked].Length <=0)
	    		goto Next; 
	    	if (PickList[NumPicked].Refno == AtRef)
	    	{
	    		IntAZ[NumInt] = PickList[NumPicked].PPAZ;  
	    		IntPCT[NumInt] = PickList[NumPicked].PCT;  
	    		IntLength[NumInt] = PickList[NumPicked].Length; 
	    		IntStreets[NumInt] = PickedStreets[NumPicked][0];
	    		IntRefs[NumInt++] = PickList[NumPicked].Refno;
	    		goto Next; 
	    	}
	    	if (!SkipNetCheck)
	    	{
		    	if (TraceNet && !RefInNet (PickList[NumPicked].Refno))
		    	{
		    		HaveNonNet=TRUE;
		    		goto Next;      
		    	} 
		    }
	    	dist1 = ldistp(AtPoint,PickList[NumPicked].BeginPoint);
	    	dist2 = ldistp(AtPoint,PickList[NumPicked].EndPoint);
	    	if (dist1 < P_TOL)
	    		dist1 = 0;
	    	if (dist2 < P_TOL)
	    		dist2 = 0;
	    	if ((dist1 < dist2 || PickList[NumPicked].Type == 1) && dist1 <= RouteTOL)
	    	{   
	    		if (PickList[NumPicked].Type != 1)
	    		{
		    		np = OrigNumPicked;
		    		while (np--)
		    		{
		    			if (np != NumPicked)
		    			{
		    				if (ldistp (PickList[NumPicked].BeginPoint,
		    							PickList[np].BeginPoint) < dist1 ||
		    					ldistp (PickList[NumPicked].BeginPoint,
		    							PickList[np].EndPoint) < dist1)
		    					goto Next;
		    			}
		    		}
		    		if (PickList[NumPicked].Refno != FromRef)
		    		{
				    	if (!BT_FIND(hBTAt,(LPSTR)&PickList[NumPicked].Refno,BT_FIRST,BT_EQ,(LPSTR)&AtData))
				    	{   
				    		if (AtData.LastRef != FromRef)
				    		{
					    		if (AtData.StartRef == Start) 
					    		{
					    			Join = TRUE; 
					    			JoinRef = PickList[NumPicked].Refno;
					    			goto Next; 
					    		}
					    	}
					    	else
					    		goto Next;
				    	} 
				    }
				    else
				    	goto Next;
			    } 
	    		NextData.OpenEnd = PickList[NumPicked].EndPoint;  
	    		NextData.FromPCT = 0;
	    		NextData.ToPCT = 10000;
    			TravelReverseNextSeg = FALSE;
				goto AddNext;
	    	}
	    	else if (dist2 <= dist1 && dist2 <= RouteTOL)
	    	{ 
	    		np = OrigNumPicked;
/*
removed 8/15/03 for sewer routing with points
	    		while (np--)
	    		{
	    			if (np != NumPicked)
	    			{
	    				if (ldistp (PickList[NumPicked].EndPoint,
	    							PickList[np].BeginPoint) < dist2 ||
	    					ldistp (PickList[NumPicked].EndPoint,
	    							PickList[np].EndPoint) < dist2)
	    					goto Next;
	    			}
	    		} */
	    		if (PickList[NumPicked].Refno != FromRef)
	    		{
			    	if (!BT_FIND(hBTAt,(LPSTR)&PickList[NumPicked].Refno,BT_FIRST,BT_EQ,(LPSTR)&AtData))
			    	{   
			    		if (AtData.LastRef != FromRef)
			    		{
				    		if (AtData.StartRef == Start) 
				    		{
				    			Join = TRUE;
				    			JoinRef = PickList[NumPicked].Refno;
				    			goto Next; 
				    		}
				    	}
				    	else
				    		 goto Next;
			    	}
			    }
			    else
			    	goto Next;  
	    		NextData.OpenEnd = PickList[NumPicked].BeginPoint; 
	    		NextData.ToPCT = 0;
	    		NextData.FromPCT = 10000;
    			TravelReverseNextSeg = TRUE;
	AddNext: 
		    	NextData.AtRef = AtRef;
	    		NextData.StartRef = Start;
	    		NextData.Sequence = Sequence;
				NextData.Segment = PickList[NumPicked].Segment;
				NextData.Offset = PickList[NumPicked].Offset;
				NextData.FileNum = PickList[NumPicked].FileNum;
				NextData.SubFile = PickList[NumPicked].SubFile;
		        NextData.FileInIndex = PickList[NumPicked].FileInIndex;  
	    		NextData.SymNum = PickList[NumPicked].Desc;
	    		NextData.Type = PickList[NumPicked].Type;
	        	_fmemmove (NextData.StreetNums,PickedStreets[NumPicked],16);
	    		NextData.AtCost = StartCost; 
	    		NextKey.Refno = PickList[NumPicked].Refno; 
	    		if (!ShortestRoute && GetSegDataGM (NextKey.Refno,&SegdataNext))
	    		{
	    			Speed = GetSegmentSpeed (&SegdataNext,NextData.AtCost);  
	    			IntersectionCost =
	    			  GetIntersectionCost (&SegdataAt,&SegdataNext,TravelReverseNextSeg,OrigNumPicked);
	    		}
	    		else 
	    		{
	    			Speed = MinSpeed;
	    			IntersectionCost = 0;
	    		}

	    		if (!Speed)
	    			SegmentCost = MAX_COST;
	    		else if (ShortestRoute)
	    			SegmentCost = PickList[NumPicked].Length;
	    		else
	    			SegmentCost = (PickList[NumPicked].Length*DistFactor)/Speed;
				if (RouteType != WALKOUTROUTE)
				{  
		    		MinCostToEnd = ldistp(NextData.OpenEnd,ToPoint); 
		    		if (!ShortestRoute)
		    			MinCostToEnd *= (DistFactor/MaxSpeed);
		    	}
		    	else
		    		MinCostToEnd = 0;
	    		NextData.NextCost = StartCost + IntersectionCost + SegmentCost; 
	    		NextKey.Cost = NextData.NextCost + MinCostToEnd; 
	    		NextKey.Cost += (UniqueCost += UniqueCostInc);
	    		NextData.MP = PickMPs[NumPicked];
	    		NextData.Path = PickPaths[NumPicked];
	    		NextData.Dir = PickDirs[NumPicked];  
	    		NextData.WhenAssigned = WhenAssigned++; 
	    		NextData.SymNum = PickList[NumPicked].Desc;
	    		NextData.Type = PickList[NumPicked].Type;
	    		BT_PUT (hBTNext,(LPSTR)&NextKey,(LPSTR)&NextData); 
	    		IntAZ[NumInt] = PickList[NumPicked].PPAZ;  
	    		IntPCT[NumInt] = PickList[NumPicked].PCT; 
	    		IntLength[NumInt] = PickList[NumPicked].Length;  
	    		IntStreets[NumInt] = NextData.StreetNums[0];
	    		IntRefs[NumInt++] = PickList[NumPicked].Refno;
	    		NumNext++; 
	    		if (PickList[NumPicked].Type == 1)
	    			break; 
	    	}
Next:;
	    }
    
	if (NumNext>1 || (NumNext==1 && IntStreets[0] != IntStreets[1]))
	{
		DisplayMarker(AtPoint,3,0,0,0,0,TRUE,FALSE,0,0,0,0,0); 
		if (!Join)
			AddIntersect (AtPoint,AtRef,NumInt,IntRefs,IntAZ,IntPCT,IntLength);
		AddChain (AtRef,0);
	}  
	else if (!NumNext && !Join)
	{
		if (HaveNonNet && !SkipNetCheck)
		{
			SkipNetCheck=TRUE;
			goto Top;
		}
		DisplayMarker(AtPoint,3,0,0,0,RGB(255,0,0),TRUE,FALSE,0,0,0,0,0);
		AddChain (AtRef,0);
	}  
	else if (!NumNext && Join) 
	{
		DisplayMarker(AtPoint,3,0,0,0,RGB(0,255,0),TRUE,FALSE,0,0,0,0,0);  
		AddChain (AtRef,&JoinRef);
		NumNext=1;
	}
	else if (NumNext && Join) 
	{
		DisplayMarker(AtPoint,3,0,0,0,RGB(0,0,0),TRUE,FALSE,0,0,0,0,0);  
		AddChain (AtRef,0);
	} 
Exit:
	PickOnlyEndPoints = FALSE;	
	return NumNext;
}


double GetLinkDeltaAZ (int NumLinkPoints,HPDPOINT pLinkPoints)
{
	double AZ1,AZ2,LinkDeltaAZ=0;
	DPOINT	Point1, Point2;
	
	Point1 = *pLinkPoints++; 
	AZ1 = getazd (&Point1,pLinkPoints);  
	Point1 = *pLinkPoints++;
	NumLinkPoints-=2;
	while (NumLinkPoints--)
	{
		Point2 = *pLinkPoints++;
		AZ2 = getazd (&Point1,&Point2);
		LinkDeltaAZ += DeltaAZ (AZ1,AZ2);
		Point1 = Point2;
		AZ1 = AZ2;
	} 
	return LinkDeltaAZ;
} 

double DeltaAZ (double AZ1, double AZ2)
{         
	double daz;
	
	daz =  AZ2 - AZ1;
	if (daz < -PY)
		daz += TWOPI;
	else if (daz > PY)
		daz -= TWOPI;
	return daz;
}   

short LoadSoilData (short idum)
{
	short		pos=BT_FIRST;
    BTVARDESC  *pVars;  
    short       NumFields, Reclen, len;
    GWDHEADER GWDHead; 
    LPGWDHEADER lpGWDHead;
    HANDLE  hVars, hDB;
    short       ibeg,NumVars;
    OFSTRUCTGM    OFStruct;
    GWFLDINFO FldInfo; 
    char	Name[128];   
    char	PrimeIndex[128];
    LPSTR	lpDot, lpEnd;
    HFILE	FidData, FidSP;
    typedef	struct	{long	IntRef;
    		 		 char	SoilID[16];
    		 		}	SOILPNTS;
    typedef SOILPNTS	FAR	*LPSOILPNTS;
    LPSOILPNTS	pSP;
    char	str[128];		
   
    _fstrcpy (Name,"soilpnts.gmd");
    ExpandText (Name); 
    _fstrcpy (PrimeIndex,Name);
    lpDot = _fstrrchr (PrimeIndex,'.');
    *lpDot = 0;
    _fstrcat (lpDot,".in1");
    lpGWDHead = &GWDHead; 
    
    FidData = GSSiOpenFile (Name,&OFStruct,OF_CREATE);   
    _fmemset (lpGWDHead,0,sizeof(GWDHEADER));
    GWDHead.NumIndex=1;
    GWDHead.Version=1;
    GWDHead.NumIndexFields[0]=1;
    GWDHead.IndexFields[0][0]=0;
    BigWrite (FidData,(HPSTR)&GWDHead,sizeof(GWDHEADER),-1);
    ibeg = 0;

    FldInfo.Len = 4;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"INT_REFNO");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 16;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_CHAR;
    _fstrcpy (FldInfo.Name,"SoilType");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

	GWDHead.Reclen=ibeg; 
	GWDHead.TimeStamp = time(0);
	GSSillseek (FidData,0,0);
	BigWrite (FidData,(HPSTR)&GWDHead,sizeof(GWDHEADER),-1);
	GSSillseek (FidData,0,2);
	                 
	NumVars = 1;
	            
	hVars = LocalAlloc (LHND,NumVars * sizeof(BTVARDESC));
	pVars =(LPBTVARDESC) LocalLock(hVars);
	            
	pVars->BT_VARLEN=4;
	pVars->BT_VARTYP=BT_INTEGER;
	pVars->BT_VAROFF=0;
	BT_CREATE (PrimeIndex, 4, FALSE, 1, 1,pVars,FALSE, 0, GWDHead.TimeStamp, FALSE);
	LocalUnlock(hVars);
	LocalFree(hVars); 
	GSSiClose2 (&FidData);
	
	hDB = OpenGWDatabase (Name,BT_WRITE);
    lpGWDHead = (LPGWDHEADER)GlobalLock (hDB);   
	pSP = (LPSOILPNTS)&lpGWDHead->GWDData;
	FidSP = GSSiOpenFile ("d:\\soilproj\\soilpnts.txt",&OFStruct,OF_READ);
	while (fgetstring (str,64,FidSP)) 
	{   
		str[10]=0;
		pSP->IntRef = atol (str);
		_fstrncpy (pSP->SoilID,&str[11],16);
	    GWDAddRecord (lpGWDHead,0,0);
	}
	GSSiClose2 (&FidSP);    
	GlobalUnlock (hDB);
	CloseGWDatabase (hDB); 
	
	return TRUE;
} 

   

BOOL IDPolygons (void)
{	HDC hDC;  
	BOOL	rtn;
	int	st,SaveMaxPick;
	LPGWDHEADER lpGWDHead;
	HIGHLIGHTDATA	HighlightData;         
	short	cond=BT_ANY, len;  
	long	Refno, Offset, nRecs,nLoaded=0;  
	HCURSOR	hcurSave;
	POINT	Point; 
	char	mess[128], PointID[128], PolyIDFile[MAX_PATH]; 
	LPPOLYID	pPI;  

	GetGlobalCVal ("[%POLYIDFILE]",PolyIDFile,"[%SYSTEMPDIR]polyid.gmd");
	hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT)); 
	GSSiRemove (PolyIDFile);
	if (!OpenPolyIDFile (PolyIDFile))
		return FALSE;
    lpGWDHead = (LPGWDHEADER)GlobalLock (hDBPolyID);   
    pPI = (LPPOLYID)&lpGWDHead->GWDData; 
	UseUserPickAp =FALSE;
	SystemPickAp = 1;	
	SaveMaxPick=MaxPick;
	MaxPick=1;
	SetPickAp(0);
    cond = BT_ANY;
	nRecs = BT_NUM_IN_INDEX (hHighlight);
	CreateStatusWind (GetFocus(),1,"ID Polygons");
	while (ContinueProcessing && !BT_FIND (hHighlight,(LPSTR)&Refno,BT_FIRST,cond,(LPSTR)&HighlightData)) 
	{   
		cond = BT_GT; 
		PickList[0] = HighlightData.PD;
		ProcessPickedItem (0,FALSE);
		_fstrcpy (PointID,"[%POLYID]");
		ExpandText (PointID);  
		PickItems2 (CurView->hWnd,HighlightData.PD.BeginPoint,FALSE,TRUE,TRUE);
		if (NumPicked)
		{   
			NumPicked--;
        	if (!BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&PickList[NumPicked].Refno,BT_FIRST,BT_EQ,(LPSTR)&Offset))
	        {    
        		len = FillGWDData (lpGWDHead,Offset);
        		if (_fstrcmp (pPI->ID,PointID))
        		{   
        			pPI->MultipleID=Refno;
					GWDReplaceRecord (lpGWDHead,len,0,Offset); 
				}
			}
			else
			{   
				pPI->AreaRefno = PickList[NumPicked].Refno;  
				pPI->PointRefno = Refno;
				pPI->MultipleID=0;
				_fstrncpy (pPI->ID,PointID,sizeof(pPI->ID));
				_fstrncpy (pPI->AreaUDI,PickList[NumPicked].UDI,sizeof(pPI->AreaUDI));
				_fstrncpy (pPI->PointUDI,HighlightData.PD.UDI,sizeof(pPI->PointUDI));
		    	GWDAddRecord (lpGWDHead,0,0);   
		    }
		} 
NextRec:	StatusWindowUpdate (0,0, nRecs, ++nLoaded);
	} 
    rtn = ContinueProcessing;    
    SetContinueProcessing ( TRUE);
	UseUserPickAp =TRUE;
	MaxPick=SaveMaxPick;
	GlobalUnlock (hDBPolyID);  
		
//		ClearHighlightList (FALSE);
	ClosePolyIDFile ();
    GSSiSetCursor(hcurSave); 
	DestroyStatusWindow (0);
    return rtn;
}   


void ClosePolyIDFile (void)
{
	if (hDBPolyID)
    	CloseGWDatabase (hDBPolyID);  
    hDBPolyID=0;
    return;
}
BOOL OpenPolyIDFile (LPSTR Name)
{   
	HANDLE	hDB; 
	OFSTRUCTGM	OFStruct;
	
	if (!CreatePolyIDFile (Name))
		return FALSE;
	hDBPolyID = OpenGWDatabase (Name,BT_WRITE);
	return TRUE;
}

BOOL CreatePolyIDFile (LPSTR InName)
{
	short		pos=BT_FIRST;
    BTVARDESC  *pVars;  
    short       NumFields, Reclen, len;
    GWDHEADER16 GWDHead; 
    LPGWDHEADER16 lpGWDHead;
    HANDLE  hVars, hDB;
    short       ibeg,NumVars;
	HFILE FidData;
    OFSTRUCTGM    OFStruct;
    GWFLDINFO FldInfo; 
    char	Name[MAX_PATH];   
    char	PrimeIndex[MAX_PATH];
    LPSTR	lpDot, lpEnd;
    POLYID	PolyID;
   
    _fstrcpy (Name,InName);
    ExpandText (Name); 
    _fstrcpy (PrimeIndex,Name);
    lpDot = _fstrrchr (PrimeIndex,'.');
    *lpDot = 0;
    _fstrcat (lpDot,".in1");
    lpGWDHead = &GWDHead; 
    
    FidData = GSSiOpenFile (Name,&OFStruct,OF_CREATE);    
    if (FidData == HFILE_ERROR)
    	return FALSE;
    _fmemset (lpGWDHead,0,sizeof(GWDHEADER16));
    GWDHead.NumIndex=1;
    GWDHead.Version=1;
    GWDHead.NumIndexFields[0]=1;
    GWDHead.IndexFields[0][0]=0;
    BigWrite (FidData,(HPSTR)&GWDHead,sizeof(GWDHEADER16),-1);
    ibeg = 0;

    FldInfo.Len = 4;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"AREA_REFNO");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 4;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"POINT_REFNO");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = sizeof(PolyID.ID);
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_CHAR;
    _fstrcpy (FldInfo.Name,"ID");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = sizeof(PolyID.AreaUDI);
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_CHAR;
    _fstrcpy (FldInfo.Name,"AreaUDI");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = sizeof(PolyID.PointUDI);
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_CHAR;
    _fstrcpy (FldInfo.Name,"PointUDI");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 4;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"MultipleIDs");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;
	GWDHead.Reclen=ibeg; 
	GWDHead.TimeStamp = time(0);
	GSSillseek (FidData,0,0);
	BigWrite (FidData,(HPSTR)&GWDHead,sizeof(GWDHEADER16),-1);
	GSSillseek (FidData,0,2);
	                 
	NumVars = 1;
	            
	hVars = LocalAlloc (LHND,NumVars * sizeof(BTVARDESC));
	pVars =(LPBTVARDESC) LocalLock(hVars);
	            
	pVars->BT_VARLEN=4;
	pVars->BT_VARTYP=BT_INTEGER;
	pVars->BT_VAROFF=0;
	BT_CREATE (PrimeIndex, 4, FALSE, 1, 1,pVars,FALSE, 0, GWDHead.TimeStamp, FALSE);
	LocalUnlock(hVars);
	LocalFree(hVars); 
	GSSiClose2 (&FidData);
	
	hDB = OpenGWDatabase (Name,BT_WRITE);
	if (!hDB) return (FALSE);
	CloseGWDatabase (hDB); 
	 
	return TRUE;
} 

BOOL LineInt (LPLINEINT lInt1, LPLINEINT lInt2)
{	
	short	rc;
	double	X1, Y1, X2, Y2, A1, A2, X3, Y3; 
	long	Mnx1, Mxx1, Mny1, Mxy1;
	long	Mnx2, Mxx2, Mny2, Mxy2;
	
	if (lInt1->Mxx < lInt2->Mnx ||
		lInt1->Mnx > lInt2->Mxx	||
		lInt2->Mxx < lInt1->Mnx ||
		lInt2->Mnx > lInt1->Mxx)
		return FALSE; 
	
	Mny1 = min (lInt1->Point1.y,lInt1->Point2.y);
	Mxy1 = max (lInt1->Point1.y,lInt1->Point2.y);
	Mny2 = min (lInt2->Point1.y,lInt2->Point2.y); 
	Mxy2 = max (lInt2->Point1.y,lInt2->Point2.y); 
	if (Mny1 > Mxy2 ||
		Mny2 > Mxy1 ||
		Mxy1 < Mny2 ||
		Mxy2 < Mny1)
		return FALSE; 
	if (SameLPoint (lInt1->Point1,lInt2->Point1))
		return FALSE;
	if (SameLPoint (lInt1->Point2,lInt2->Point1))
		return FALSE;
	if (SameLPoint (lInt1->Point1,lInt2->Point2))
		return FALSE;
	if (SameLPoint (lInt1->Point2,lInt2->Point2))
		return FALSE;
	X1 = lInt1->Point1.x;	
	Y1 = lInt1->Point1.y;
	A1 = getazl (lInt1->Point1,lInt1->Point2);	
	X2 = lInt2->Point1.x;	
	Y2 = lInt2->Point1.y;
	A2 = getazl (lInt2->Point1,lInt2->Point2);	
    rc = LINSEC_TOL (X1, Y1, A1, X2, Y2, A2, &X3, &Y3, P_TOL);
	if (rc != 1)
		return FALSE;
	if (X3 < lInt1->Mnx ||
		X3 > lInt1->Mxx ||
		X3 < lInt2->Mnx ||
		X3 > lInt2->Mxx)
		return FALSE;
	if (Y3 < Mny1 ||
		Y3 > Mxy1 ||
		Y3 < Mny2 ||
		Y3 > Mxy2)
		return FALSE;
	return TRUE;
} 

/*BOOL ComputeNewMapControlPoint (LPDPOINT pPoint)
{   
	HANDLE	hTran; 
	char	TranFile[128]; 
	double	XOldSP[32],YOldSP[32],XNewSP[32],YNewSP[32],XNewUTM[32],YNewUTM[32];
	double	XBM[32],YBM[32];
	DPOINT	Point;
	short	npnts=0, TotPnts=0,i; 
	float	RSQMIN; 
	BOOL	First=TRUE;
	HFILE Fid;
	OFSTRUCTGM OFStruct;
	char	str[260]; 

Top:	
	_fstrcpy (TranFile,"d:\\soilbmps\\[SOILMAP].dpt");
	ExpandText (TranFile);
	if (!(hTran=LoadTranFile(TranFile,1)))
	{ 
		if (!First)
			return FALSE;
		First=FALSE;
		_fstrcpy (TranFile,"d:\\soilbmps\\[SOILMAP]e.mpt");
		ExpandText (TranFile);
		if ((npnts=LoadTranFilePoints(TranFile,XBM,YBM,XOldSP,YOldSP)))
		{
			_fstrcpy (TranFile,"d:\\soilbmps\\[SOILMAP]e.cpt");
			ExpandText (TranFile);
			if (!(hTran=LoadTranFile(TranFile,1)))
			{
				return FALSE;
			} 
			for (i=0;i<npnts;i++) 
			{
		    	TRANS2 (XBM[i],YBM[i],&XNewUTM[i],&YNewUTM[i],hTran);  
		    	Point.x = XNewUTM[i];
		    	Point.y = YNewUTM[i]; 
		    	ConvertPoint ("usgsdoq",&Point,1);
		    	XNewSP[i] = Point.x;
		    	YNewSP[i] = Point.y;
		    }
			CloseTRANS2 (&hTran); 
		}
		TotPnts = npnts;
		_fstrcpy (TranFile,"d:\\soilbmps\\[SOILMAP]w.mpt");
		ExpandText (TranFile);
		if ((npnts=LoadTranFilePoints(TranFile,XBM,YBM,&XOldSP[TotPnts],&YOldSP[TotPnts])))
		{
			_fstrcpy (TranFile,"d:\\soilbmps\\[SOILMAP]w.cpt");
			ExpandText (TranFile);
			if (!(hTran=LoadTranFile(TranFile,1)))
			{
				return FALSE;
			} 
			for (i=0;i<npnts;i++) 
			{
		    	TRANS2 (XBM[i],YBM[i],&XNewUTM[i],&YNewUTM[i],hTran);  
		    	Point.x = XNewUTM[i];
		    	Point.y = YNewUTM[i]; 
		    	ConvertPoint ("usgsdoq",&Point,1);
		    	XNewSP[TotPnts+i] = Point.x;
		    	YNewSP[TotPnts+i] = Point.y;
		    }
			CloseTRANS2 (&hTran);
		} 
		TotPnts += npnts; 
		_fstrcpy (TranFile,"d:\\soilbmps\\[SOILMAP].dpt");
		ExpandText (TranFile);
		Fid = GSSiOpenFile (TranFile,&OFStruct,OF_CREATE);
		for (i=0;i<TotPnts;i++)
		{
			sprintf (str,"%f %f %f %f",XOldSP[i],YOldSP[i],XNewSP[i],YNewSP[i]);
			fputstring (str,Fid);
		}
		GSSiClose2 (&Fid);
		goto Top;
	} 
    TRANS2 (pPoint->x,pPoint->y,&pPoint->x,&pPoint->y,hTran); 
	CloseTRANS2 (&hTran);
	
	return TRUE;
}*/
   
BOOL ComputeNewMapControlPoint (LPDPOINT pPoint)
{   
	HANDLE	hTran; 
	char	TranFile[128]; 

Top:	
	_fstrcpy (TranFile,"[%MAPTRANFILE]");
	ExpandText (TranFile);
	if ((hTran=LoadTranFile(TranFile,2,1,0,0)))
	{ 
	    TRANS2 (pPoint->x,pPoint->y,&pPoint->x,&pPoint->y,hTran); 
		CloseTRANS2 (&hTran);
	} 
	
	return TRUE;
}
   
short LoadTranFilePoints (LPSTR Name,LPDOUBLE XFROM, LPDOUBLE YFROM, LPDOUBLE XTO, LPDOUBLE YTO)
{
	HFILE Fid;
	OFSTRUCTGM OFStruct;
	char	str[260]; 
	HANDLE	handle=0, hcoord=GSSiGlobAlloc ( 584,GMEM_MOVEABLE,4096); 
	float	RSQMIN;
	short	N=0;
	
	Fid = GSSiOpenFile (Name,&OFStruct,OF_READ);
	if (Fid == HFILE_ERROR)
		return 0;
	while (fgetstring (str,256,Fid))
	{ 
		if (sscanf (str,"%lf %lf %lf %lf",XFROM++,YFROM++,XTO++,YTO++) == 4)
			N++;
		else
			goto Exit;
	}  
Exit:
	GSSiClose2 (&Fid);
	return N;
}




/*BOOL SplitPoly (short Item, LPLONG pNewRef1, LPLONG pNewRef2)
{   
	short	OldType=12,NewType=92; 
	short	NewPolyType=1,NumNewPolyPoints=2,NewLineSymbol=PickList[Item].Desc;
	long	NewRefno; 
	LPSTR	pStuff=0, Prefix=0,UDI=0;
	BOOL	rtn;
	HANDLE	hNewPolyPoints; 
	LPDPOINT	pNewPoints;
	
	GetPickName (Item); 
	_fstrcpy (PltName,PickName);
	DeletePickedItem (Item,OldType,NewType);
	hNewPolyPoints = GSSiGlobAlloc ( 585,GMEM_MOVEABLE,sizeof(DPOINT)*(long)NumNewPolyPoints);
	pNewPoints = GlobalLock (hNewPolyPoints);
	*pNewPoints++ = PickList[Item].BeginPoint;
	*pNewPoints = PickList[Item].PickedPoint;
	GlobalUnlock (hNewPolyPoints);
	NewRefno = GetNewRefno (PickName); 
	rtn=AddPolyToMap (1,&NumNewPolyPoints, &hNewPolyPoints,NewPolyType,NewRefno,2,NewLineSymbol,pStuff,Prefix,UDI,-1,-1,-1);
	pNewPoints = GlobalLock (hNewPolyPoints);
	*pNewPoints++ = PickList[Item].PickedPoint;
	*pNewPoints = PickList[Item].EndPoint;
	GlobalUnlock (hNewPolyPoints);
	NewRefno = GetNewRefno (PickName); 
	rtn=AddPolyToMap (1,&NumNewPolyPoints, &hNewPolyPoints,NewPolyType,NewRefno,2,NewLineSymbol,pStuff,Prefix,UDI,-1,-1,-1);
	GlobalFree (hNewPolyPoints);
	CloseMap();
	return rtn;
}  */

BOOL SplitCurve (short Item, LPLONG pNewRef1, LPLONG pNewRef2)
{
	HANDLE	hSeg1=0, hSeg2=0;   
	int		npt1=3,npt2=3, st;    
	HPDPOINT	Dpoint1,Dpoint2;
	long	lmem=(long)3*sizeof(DPOINT);  
	double	PickDist, Dist=0;
	long	Newref1, Newref2;
	short	Stuff[64];      
	DPOINT	RP, POC, PT;
	double	CLEN, clen1, clen2;
	
	if (PickList[Item].PCT <= 0)
		return FALSE;		
	if (PickList[Item].PCT >= 1)
		return FALSE;
	PickDist = PickList[Item].Length * PickList[Item].PCT;
    SetConfig (PickList[Item].ConfigID);
    SetViewport (PickList[Item].ViewID);
	if (!*EditName) 
	{
		GetPickName (Item); 
		_fstrcpy (PltName,PickName);
	}
	else
		_fstrcpy (PltName,EditName);
	hSeg1 = GSSiGlobAlloc ( 586,GMEM_MOVEABLE,lmem);
	hSeg2 = GSSiGlobAlloc ( 587,GMEM_MOVEABLE,lmem);  
    Dpoint1 = (HPDPOINT)GlobalLock (hSeg1);
    Dpoint2 = (HPDPOINT)GlobalLock (hSeg2);
	st = RCURVE(&PickList[Item].BeginPoint.x,&PickList[Item].BeginPoint.y,&PickList[Item].NodePoint.x,&PickList[Item].NodePoint.y,&PickList[Item].EndPoint.x,&PickList[Item].EndPoint.y,&RP.x,&RP.y,&CLEN); 
	*Dpoint1 = PickList[Item].BeginPoint;
	clen1 = CLEN*PickList[Item].PCT;
	PCURVE(&Dpoint1->x,&Dpoint1->y,&POC.x,&POC.y,&PT.x,&PT.y,&RP.x,&RP.y,&clen1);
	Dpoint1[1] = POC;
	Dpoint1[2] = Dpoint2[0] = PT; 
	clen2 = CLEN*(1.0-PickList[Item].PCT);
	PCURVE(&Dpoint2->x,&Dpoint2->y,&POC.x,&POC.y,&PT.x,&PT.y,&RP.x,&RP.y,&clen2);
	Dpoint2[1] = POC;
	Dpoint2[2] = PT;
    Newref1=GetNewRefno(PltName,0,0,0,0);
    Newref2=GetNewRefno(PltName,0,0,0,0); 
    Stuff[0]=18;
    Stuff[1]=10;
    _fmemmove (&Stuff[2],PickedStreets[Item],16);
	GlobalUnlock (hSeg1);
	GlobalUnlock (hSeg2);
	AddPolyToMap (1,&npt1, &hSeg1,3,Newref1,0,1,PickList[Item].Desc,Stuff,0,0,-1,-1,-1,0,0,0,0,TRUE,0);
	AddPolyToMap (1,&npt2, &hSeg2,3,Newref2,0,1,PickList[Item].Desc,Stuff,0,0,-1,-1,-1,0,0,0,0,TRUE,0); 
	GSSiGlobFree (&hSeg1);
	GSSiGlobFree (&hSeg2); 
	CloseMap (TRUE);
	DeletePickedItem (Item,12,92);
	GetPickName (Item); 
	_fstrcpy (PltName,PickName);
	*pNewRef1 = Newref1;  
	*pNewRef2 = Newref2;  
	return TRUE;
}

BOOL AddContourSplitLines (LPMNMXCORD pBounds,int nGrid)
{ 
	MNMXCORD	Bounds = FactorBounds (pBounds,1.000);
	double		maxd = max (BoundsWidth(&Bounds),BoundsHeight(&Bounds));
	double		GridWidth = maxd / nGrid, GridHeight;
	int			nGridCols = BoundsWidth (pBounds) / GridWidth + 1;
	int			nGridRows = BoundsHeight (pBounds) / GridWidth + 1;
	HANDLE		hPnts = GSSiGlobAlloc (0,GMEM_MOVEABLE,2*sizeof(DPOINT));
	int			irow, icol;
	int			SymNum = GetSymbolNum ("SPLITLINE");
	int			SymNum2 = GetSymbolNum ("SPLITLINE2");
	short		NumSyms; 
	HANDLE		hSymDesc=0; 

	GridWidth = BoundsWidth(&Bounds)/nGridCols;
	GridHeight = BoundsHeight (&Bounds)/nGridRows;
	AddToSymList (SymNum,&NumSyms,&hSymDesc); 
	AddToSymList (SymNum2,&NumSyms,&hSymDesc); 
	*EditName = 0;
	if (CurView->UpdateFile)
		_fstrcpy (EditName,CurView->lpFiles[CurView->UpdateFile-1]);
	if (!EditName[0])
		return FALSE;
	_fstrcpy (PltName,EditName);
	PltType = 2;
	for (irow=0;irow<nGridRows;irow++)
	{
		for (icol=0;icol<nGridCols;icol++)
		{
			LPDPOINT	pPoints;
			int			ref=GetNewRefno(PltName,0,0,0,0);
			int			nPnts = 2;
		
			pPoints = GlobalLock (hPnts);
			pPoints[0].x = Bounds.xmn + icol * GridWidth;
			pPoints[1].x = Bounds.xmn + (icol+1) * GridWidth;
			pPoints[0].y = pPoints[1].y = Bounds.ymn + irow * GridHeight;
			GlobalUnlock (hPnts);
			AddPolyToMap (1,&nPnts,&hPnts,1,ref,0,2,SymNum,0,0,0,-1,-1,-1,0,0,0,0,TRUE,0);
			if (irow == nGridRows-1)
			{
				pPoints = GlobalLock (hPnts);
				pPoints[0].x = Bounds.xmn + icol * GridWidth;
				pPoints[1].x = Bounds.xmn + (icol+1) * GridWidth;
				pPoints[0].y = pPoints[1].y = Bounds.ymn + (irow+1) * GridHeight;
				GlobalUnlock (hPnts);
				ref=GetNewRefno(PltName,0,0,0,0);
				AddPolyToMap (1,&nPnts,&hPnts,1,ref,0,2,SymNum,0,0,0,-1,-1,-1,0,0,0,0,TRUE,0);
			}
			pPoints = GlobalLock (hPnts);
			pPoints[0].x = pPoints[1].x = Bounds.xmn + icol * GridWidth;
			pPoints[0].y = Bounds.ymn + irow * GridHeight;
			pPoints[1].y = Bounds.ymn + (irow+1) * GridHeight;
			GlobalUnlock (hPnts);
			ref=GetNewRefno(PltName,0,0,0,0);
			AddPolyToMap (1,&nPnts,&hPnts,1,ref,0,2,SymNum,0,0,0,-1,-1,-1,0,0,0,0,TRUE,0);
			if (icol == nGridCols-1)
			{
				pPoints = GlobalLock (hPnts);
				pPoints[0].x = pPoints[1].x = Bounds.xmn + (icol+1) * GridWidth;
				pPoints[0].y = Bounds.ymn + irow * GridHeight;
				pPoints[1].y = Bounds.ymn + (irow+1) * GridHeight;
				GlobalUnlock (hPnts);
				ref=GetNewRefno(PltName,0,0,0,0);
				AddPolyToMap (1,&nPnts,&hPnts,1,ref,0,2,SymNum,0,0,0,-1,-1,-1,0,0,0,0,TRUE,0);
			}
		}
	}
	GSSiGlobFree (&hPnts);
    CloseMap(TRUE);  
	AddSymToMap (NumSyms,hSymDesc,0,0); 
    DestroySymList (&NumSyms,&hSymDesc);
	return TRUE;
}

BOOL SplitContourLines (int iOpt)
{   
	HPDPOINT	pDpoint;
	double	PickDist, Dist=0;
	long	Newref1, Newref2;
	short	Stuff[64];
    short	pos= BT_FIRST, i;  
    HIGHLIGHTDATA	HighlightData;
	long	npnts[2], IntRefno;
	LPINT	RefnoSplitLine, npntsSplitLine,RefnoCon,npntsCon,ConSym,SLConSym;
	LPHANDLE	hPolySplitLine, hPolyCon;
	HANDLE	hSLRef=0, hSLnp=0, hSLhPoly=0,hSLSplitDist=0;
	HANDLE	hConRef=0,hConnp=0,hConPoly=0,hConSym=0,hSLConSym=0;
	LPDOUBLE	SLSplitDist;
	HPDPOINT	pPolyPoints[2];
	HANDLE	hPoly[2]={0,0};  
	DPOINT	IntPoint;
	MNMXCORD	Bounds;
	double	IntDist[2], DistAlongSplitLine;
	double	InAZ, OutAZ[4], AZ;                                             
	int		n, nRecs, nSplitLines=0, iSplitLine, nConLines, iCon, nConSplitDist,ii;
	long	iseg;
	BOOL	OutReverse[4], FlipReverse;
	short	WhichPoly[4];
	int		nSLDist, SLSym[2] = {GetSymbolNum ("SPLITLINE"),GetSymbolNum ("SPLITLINE2")};
	BOOL	rtn=FALSE;
	int		iSym;
    
	nRecs = BT_NUM_IN_INDEX (hHighlight);
	*EditName = 0;
	if (CurView->UpdateFile)
		_fstrcpy (EditName,CurView->lpFiles[CurView->UpdateFile-1]);
	if (!nRecs || !*EditName)
		goto Exit;
	_fstrcpy (PltName,EditName);
	hSLRef = GSSiGlobAlloc (1710,GMEM_MOVEABLE,nRecs*sizeof(int));
	hSLnp  = GSSiGlobAlloc (1711,GMEM_MOVEABLE,nRecs*sizeof(int));
	hSLhPoly = GSSiGlobAlloc (1712,GMEM_MOVEABLE,nRecs*sizeof(HANDLE));
	RefnoSplitLine = GlobalLock (hSLRef);
	npntsSplitLine = GlobalLock (hSLnp);
	hPolySplitLine = GlobalLock (hSLhPoly);
  	while (!BT_FIND (hHighlight,(LPSTR)&RefnoSplitLine[nSplitLines],pos,BT_ANY,(LPSTR)&HighlightData))  
   	{
   		pos=BT_NEXT; 
		SetConfig (HighlightData.PD.ConfigID);
		SetViewport (HighlightData.PD.ViewID);
		if (HighlightData.PD.Type == 2)
		{
			if (GetPolyPoints ((LPPICKDATAHEADER)&HighlightData.PD,FALSE,&npntsSplitLine[nSplitLines],&hPolySplitLine[nSplitLines]))
				nSplitLines++;
		}
		DeleteRefno (HighlightData.PD.Refno);
	}
	CreateStatusWind (GetFocus(),1,"Split Contours");
	iSplitLine = 0;
	while (StatusWindowUpdate (0,0, nSplitLines, iSplitLine) && iSplitLine<nSplitLines)
	{
	    ClearHighlightList (FALSE);  
		GetPolyBoundsD (hPolySplitLine[iSplitLine],npntsSplitLine[iSplitLine],&Bounds,2);
		ExpandBounds (&Bounds,P_TOL*100);
		pPolyPoints[0] = (HPDPOINT)GlobalLock (hPolySplitLine[iSplitLine]);
		hSLSplitDist = GSSiGlobAlloc (1713,GMEM_MOVEABLE,USHRT_MAX*sizeof(double));
		hSLConSym = GSSiGlobAlloc (1714,GHND,USHRT_MAX*sizeof(int));
		SLSplitDist = GlobalLock (hSLSplitDist);
		SLConSym = GlobalLock (hSLConSym);
		nSLDist = 2;
		SLSplitDist[0] = 0;
		SLSplitDist[1] = GetPolyLengthD (pPolyPoints[0],npntsSplitLine[iSplitLine]);
		nConLines = 0;
		if ((nRecs=HighlightInArea (CurView->hWnd,&Bounds,TRUE,FALSE,0)) > 0)
		{
			hConRef = GSSiGlobAlloc (1716,GMEM_MOVEABLE,nRecs*sizeof(int));
			hConnp  = GSSiGlobAlloc (1717,GMEM_MOVEABLE,nRecs*sizeof(int));
			hConPoly = GSSiGlobAlloc (1718,GMEM_MOVEABLE,nRecs*sizeof(HANDLE));
			hConSym = GSSiGlobAlloc (1715,GMEM_MOVEABLE,nRecs*sizeof(int));
			RefnoCon = GlobalLock (hConRef);
			npntsCon = GlobalLock (hConnp);
			hPolyCon = GlobalLock (hConPoly);
			ConSym = GlobalLock (hConSym);
			pos = BT_FIRST;
  			while (!BT_FIND (hHighlight,(LPSTR)&RefnoCon[nConLines],pos,BT_ANY,(LPSTR)&HighlightData))  //RefnoCon[nConLines-1]
   			{
   				pos=BT_NEXT; 
   				n = 0;
				ConSym[nConLines] = HighlightData.PD.Desc;
   				if (RefnoCon[nConLines] != RefnoSplitLine[iSplitLine] && HighlightData.PD.Type == 2)         
   				{
	   				if (GetPolyPoints ((LPPICKDATAHEADER)&HighlightData.PD,FALSE,&npntsCon[nConLines],&hPolyCon[nConLines]))
	   					nConLines++;
				}
			}
		}
		for (iCon=0;iCon<nConLines;iCon++)
		{
			HANDLE	hConSplitDist = GSSiGlobAlloc (1719,GMEM_MOVEABLE,USHRT_MAX*sizeof(double));
			LPDOUBLE	ConSplitDist = GlobalLock (hConSplitDist);

			DistAlongSplitLine = 0;
			pPolyPoints[1] = (HPDPOINT)GlobalLock (hPolyCon[iCon]);
			nConSplitDist = 2;
			ConSplitDist[0] = 0;
			ConSplitDist[1] = GetPolyLengthD (pPolyPoints[1],npntsCon[iCon]);
			while ((n = IntersectPolys2 (npntsSplitLine[iSplitLine],pPolyPoints[0],
								 npntsCon[iCon],pPolyPoints[1],
								 DistAlongSplitLine,IntDist,&IntPoint,&InAZ,
								 OutAZ,OutReverse,WhichPoly,TRUE)))
			{
				DistAlongSplitLine = IntDist[0];
				DisplayMarkers = TRUE;
				DisplayMarker (IntPoint,6,0,0,0,0,TRUE,FALSE,0,0,0,0,0); 

				for (i=0;i<nSLDist;i++)
				{
					if (fabs (IntDist[0] - SLSplitDist[i]) < 2*P_TOL)
						goto SkipSLPoint;
					if (IntDist[0] < SLSplitDist[i])
					{
						memmove (&SLSplitDist[i+1],&SLSplitDist[i],(nSLDist-i)*sizeof(double));
						memmove (&SLConSym[i+1],&SLConSym[i],(nSLDist-i)*sizeof(int));
						SLSplitDist[i] = IntDist[0];
						SLConSym[i] = ConSym[iCon];
						goto SLSorted;
					}
				}
				SLSplitDist[nSLDist] = IntDist[0];
				SLConSym[nSLDist]=ConSym[iCon];
SLSorted:
				nSLDist++;
SkipSLPoint:
				for (i=0;i<nConSplitDist;i++)
				{
					if (fabs (IntDist[1] - ConSplitDist[i]) < 2*P_TOL)
						goto SkipConPoint;
					if (IntDist[1] < ConSplitDist[i])
					{
						memmove (&ConSplitDist[i+1],&ConSplitDist[i],(nConSplitDist-i)*sizeof(double));
						ConSplitDist[i] = IntDist[1];
						goto ConSorted;
					}
				}
				ConSplitDist[nConSplitDist] = IntDist[1];
ConSorted:
				nConSplitDist++;
SkipConPoint:;
			}
			if (nConSplitDist > 2)
			{
				DeleteRefno (RefnoCon[iCon]);

				for (i=0;i<nConSplitDist-1;i++)
				{
					int			nNewPts;
					HANDLE		hNewCon = GetPolyBetweenDist (pPolyPoints[1],npntsCon[iCon],ConSplitDist[i],ConSplitDist[i+1],&nNewPts,FALSE,FALSE); 
					int			ref=GetNewRefno(PltName,0,0,0,0);
if (iSplitLine == 1 && iCon == 5)
	ii=1;
					AddPolyToMap (1,&nNewPts, &hNewCon,1,ref,0,2,ConSym[iCon],0,0,0,-1,-1,-1,0,0,0,0,TRUE,0);
					GSSiGlobFree (&hNewCon);
				}
			}
			GSSiGlobUlFree (&hPolyCon[iCon]);
			GSSiGlobUlFree (&hConSplitDist);
		}
		for (i=0;i<nSLDist-1;i++)
		{
			int			nNewPts;
			HANDLE		hNewCon = GetPolyBetweenDist (pPolyPoints[0],npntsSplitLine[iSplitLine],SLSplitDist[i],SLSplitDist[i+1],&nNewPts,FALSE,FALSE); 
			int			ref=GetNewRefno(PltName,0,0,0,0);

			if (!SLConSym[i] || !SLConSym[i+1] || SLConSym[i] == SLConSym[i+1])
				iSym = 1;
			else
				iSym = 0;
			AddPolyToMap (1,&nNewPts, &hNewCon,1,ref,0,2,SLSym[iSym],0,0,0,-1,-1,-1,0,0,0,0,TRUE,0);
			GSSiGlobFree (&hNewCon);
		}
		CloseMap (TRUE);
		GSSiGlobUlFree (&hConRef);
		GSSiGlobUlFree (&hConnp);
		GSSiGlobUlFree (&hConPoly);
		GSSiGlobUlFree (&hConSym);
		GSSiGlobUlFree (&hPolySplitLine[iSplitLine]);
		GSSiGlobUlFree (&hSLSplitDist);
		GSSiGlobUlFree (&hSLConSym);
		iSplitLine++;
		/*{
			char	NewName[MAX_PATH];

			strcpy (NewName,PltName);
			sprintf (strrchr (NewName,'.'),"%i.plt",iSplitLine);
			CloseAllRequestedFiles (FALSE);
			CopyFile (PltName,NewName,FALSE);
		}*/
	} 
    rtn = ContinueProcessing;    
    SetContinueProcessing ( TRUE);
	DestroyStatusWindow (0);
Exit:
	GSSiGlobUlFree (&hSLRef);
	GSSiGlobUlFree (&hSLnp);
	GSSiGlobUlFree (&hSLhPoly);

	ClearHighlightList (FALSE);   
	return rtn;
}

BOOL SplitPoly (short Item, LPLONG pNewRef1, LPLONG pNewRef2)
{   
	HANDLE	hSeg1=0, hSeg2=0;   
	LPTHEME	pTheme;  
	int		npt1=1,npt2=1;    
	HPDPOINT	pDpoint;
	long	lmem=(long)PickList[Item].NumPoints*sizeof(DPOINT);  
	double	PickDist, Dist=0;
	long	Newref1, Newref2;
	short	Stuff[64];
	
	if (PickList[Item].PCT <= 0)
		return FALSE;		
	if (PickList[Item].PCT >= 1)
		return FALSE;
	if (PickList[Item].Type == 5)
		return SplitCurve (Item,pNewRef1,pNewRef2);
	PickDist = PickList[Item].Length * PickList[Item].PCT;
    SetConfig (PickList[Item].ConfigID);
    SetViewport (PickList[Item].ViewID);
	pTheme = AddTheme (GF_SAVEPOLY_THEME);
	CurView->PassID = 4;
	ProcessSelectedTheme = CurView->NumThemes;
	ProcessPickedItem (Item,FALSE);        		
	ProcessSelectedTheme = 0;
	DeleteTheme (pTheme);
    if (GetSavedPolys ()) 
    {
		if (hSavePoly && nSavePoly > 1)
		{   LPMNMXCORD lpRect;
			int		nPnts; 
			HPDPOINT	lpDpoint; 
			DPOINT	LastPoint;  
			BOOL 	First=TRUE;
		    		    
	        nPnts = nSavePoly; 
	        lpRect = (LPMNMXCORD) GlobalLock (hSavePoly);
	        lpRect++;
	        lpDpoint = (HPDPOINT) lpRect;
			hSeg1 = GSSiGlobAlloc ( 586,GMEM_MOVEABLE,lmem);
			hSeg2 = GSSiGlobAlloc ( 587,GMEM_MOVEABLE,lmem);  
	        pDpoint = (HPDPOINT)GlobalLock (hSeg1);
	        LastPoint = *lpDpoint;
	        *pDpoint++ = *lpDpoint++;   
	        nPnts--;
	        while (nPnts--)
	        {   
	        	if (!First)
	        	{
	            	*pDpoint++ = *lpDpoint; 
	            	npt2++;
	        	}
	        	else
	        	{
	            	Dist += ldistp (LastPoint,*lpDpoint);  
	            	if (fabs (Dist - PickDist) < P_TOL) 
	            	{
	            		*pDpoint = *lpDpoint;
	            		npt1++;
	            		GlobalUnlock (hSeg1);
			            pDpoint = (HPDPOINT)GlobalLock (hSeg2);  
			            *pDpoint++ = *lpDpoint;
			            First = FALSE;
	            	}
	            	else if (Dist > PickDist)
	            	{
	            		*pDpoint = PickList[Item].PickedPoint;
	            		npt1++;
	            		GlobalUnlock (hSeg1);
			            pDpoint = (HPDPOINT)GlobalLock (hSeg2);  
	            		*pDpoint++ = PickList[Item].PickedPoint;
		            	*pDpoint++ = *lpDpoint; 
		            	npt2++;
			            First = FALSE;
	            	} 
	            	else
	            	{ 
	            		LastPoint = *lpDpoint;
	            		*pDpoint++ = *lpDpoint;
	            		npt1++;
	            	}
				}  
				lpDpoint++;
	        }
			GlobalUnlock(hSavePoly);
	        GlobalUnlock (hSeg2);
			DestroySavedPolys();
	    } 
		else
		{
			DestroySavedPolys();
	    	return FALSE;
		}
	}
	else
		return FALSE; 
	if (!*EditName) 
	{
		GetPickName (Item); 
		_fstrcpy (PltName,PickName);
	}
	else
		_fstrcpy (PltName,EditName);
    Newref1=GetNewRefno(PltName,0,0,0,0);
    Newref2=GetNewRefno(PltName,0,0,0,0); 
    Stuff[0]=18;
    Stuff[1]=10;
    _fmemmove (&Stuff[2],PickedStreets[Item],16);
	AddPolyToMap (1,&npt1, &hSeg1,1,Newref1,0,2,PickList[Item].Desc,Stuff,0,0,-1,-1,-1,0,0,0,0,TRUE,0);
	AddPolyToMap (1,&npt2, &hSeg2,1,Newref2,0,2,PickList[Item].Desc,Stuff,0,0,-1,-1,-1,0,0,0,0,TRUE,0); 
	GSSiGlobFree (&hSeg1);
	GSSiGlobFree (&hSeg2); 
	CloseMap (TRUE);
	DeletePickedItem (Item,12,92);
	GetPickName (Item); 
	_fstrcpy (PltName,PickName);
	*pNewRef1 = Newref1;  
	*pNewRef2 = Newref2;  
	return TRUE;
}  

BOOL GetMapName (LPSTR Name)
{   
	UINT	i;
	
	if (!_fstrnicmp (Name,"LAYER:",6))
	{   
		for (i=0;i<CurView->NumFiles;i++)
			if (!_fstricmp (CurView->FileID[i],Name+6))
			{
				_fstrcpy (Name,CurView->lpFiles[i]);
				return TRUE;
			}
		return FALSE;
	}
	return TRUE;
} 

BOOL PolyInHighlightArea (long npnts,HANDLE hPoints)
{   
	LPMNMXCORD	lpRect;
	HPDPOINT	lpDpoints, pPolyPoints;
	BOOL		rtn;
	
	if (!hHighlightArea)
    	return TRUE;
	lpRect = (LPMNMXCORD) GlobalLock (hHighlightArea);
    lpDpoints = (LPDPOINT) (lpRect+1); 
    pPolyPoints = (HPDPOINT)GlobalLock (hPoints); 
 	rtn = PolyInArea (GF_POLYLINE,npnts,pPolyPoints,0,nHighlightAreaPoints,lpDpoints,nHighlightAreaPoly,hHighlightAreaPolyPartLen,InclusionOpt,hHighlightAreaAccelerator);    
	GlobalUnlock (hHighlightArea);
	GlobalUnlock (hPoints); 
	return rtn;
}

BOOL HighlightRoute (LPSTR RouteFile,long Refno,double RefLoc,BOOL Display)
{
	LPGWDHEADER		lpGWDHead;
	HANDLE			hDBRoute;
    LPROUTEATFILE	pRAF;
    long			Offset;
    short			st;
	HCURSOR			hcurSave;
	BOOL			AP; 
	
	hDBRoute = OpenGWDatabase (RouteFile,BT_READ); 
	if (!hDBRoute)
		return FALSE;
	hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT)); 
    AP = SetAutoPan (FALSE);
	lpGWDHead = (LPGWDHEADER)GlobalLock (hDBRoute);   
	pRAF = (LPROUTEATFILE)&lpGWDHead->GWDData;
	do
	{ 
   		st = BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&Refno,BT_FIRST,BT_EQ,(LPSTR)&Offset);
   		if (!st)
   		{
	   		FillGWDData (lpGWDHead,Offset);
	   		Refno = pRAF->LastRef;
			PickByRefno(Refno,0,0,-1);  
	    	AddToHighlightList (Refno,&PickList[0],TRUE);
	    	if (Display)
				ShowPickedItem (hWndMain,0);
   		}
    } while (!st && pRAF->LastRef != pRAF->StartRef); 
    GlobalUnlock (hDBRoute);
    CloseGWDatabase (hDBRoute);    
    SetAutoPan (AP);
    GSSiSetCursor(hcurSave); 
	return TRUE;
}
 
BOOL DeleteRefno (long Refno)
{
	if (!CurView->UpdateFile)
	{
		if (PickByRefno(Refno,0,0,-1)) 
		{
			if (DeletePickedItem (0,12,92))    
				return TRUE;
		}
		return FALSE;; 
	}
	_fstrcpy (PltName,CurView->lpFiles[CurView->UpdateFile-1]);
	PltType = 2;
	AddPolyToMap (0,0, 0,1,Refno,0,-1,1,0,0,0,-1,-1,0,0,0,0,0,FALSE,0);
	CloseMap(TRUE);
	CloseRefIndex(TRUE);    		
	ForceRefIndex = ForceTAGIndex = FALSE;  
	return TRUE;
}

BOOL SplitHighlightedPolys (int MaxPolyPoints)
{   
	HANDLE	hPolyOrig=0, hNewPoly;   
	HPDPOINT	pDpoint;
	long	Newref, nRecs, nLoaded=0, Refno, nPntsOrig;
	short	pos = BT_FIRST, nLoops, NumNewPolys, Desc;
	HIGHLIGHTDATA	HighlightData;  
	LPVIEWPORT	SaveVP=CurView; 
	BOOL	rtn=FALSE;  
	short	PickFile,ii;
	
	nRecs = BT_NUM_IN_INDEX (hHighlight);
	if (!nRecs)
		goto Exit;
	BT_FIND (hHighlight,(LPSTR)&Refno,BT_FIRST,BT_ANY,(LPSTR)&HighlightData);
    SetConfig (HighlightData.PD.ConfigID);
   	SetViewport (HighlightData.PD.ViewID);
	if (CurView->UpdateFile)
		_fstrcpy (EditName,CurView->lpFiles[CurView->UpdateFile-1]); 
	else
		goto Exit;

	CreateStatusWind (GetFocus(),1,"Split Polylines");
	while (ContinueProcessing && !BT_FIND (hHighlight,(LPSTR)&Refno,pos,BT_ANY,(LPSTR)&HighlightData)) 
	{   
		pos = BT_ANY;    
		if (Refno == 300276196)
			ii=1;
		Desc = HighlightData.PD.Desc;
    	PickFile = HighlightData.PD.FileNum + HighlightData.PD.ViewID*256;    
		if (PickByRefno(Refno,0,0,PickFile)) 
		{
			if (PickList[0].Type == 2 && PickList[0].NumPoints > MaxPolyPoints)
			{
				nLoops = GetPolyPoints2 ((LPPICKDATAHEADER)&PickList[0],0,0,0); 
				if (nLoops == 1 && GetPolyPoints ((LPPICKDATAHEADER)&PickList[0],FALSE,&nPntsOrig,&hPolyOrig))
				{   
					HPDPOINT	pOrigPoints = (HPDPOINT)GlobalLock (hPolyOrig), pNewPoints;
					long		RemPoints = nPntsOrig; 
					short		ipoly; 
					int			NumNewPoints;
					
					DeleteRefno (Refno);
					NumNewPolys = (nPntsOrig-1) / MaxPolyPoints + 1;
					for (ipoly = 0;ipoly < NumNewPolys; ipoly++)
					{
						if (ipoly == NumNewPolys - 1)
							NumNewPoints = RemPoints;
						else if (ipoly == NumNewPolys - 2)
							NumNewPoints = RemPoints / 2;
						else
							NumNewPoints = MaxPolyPoints;
						hNewPoly = GSSiGlobAlloc (0,GMEM_MOVEABLE,((long)NumNewPoints)*sizeof(DPOINT)); 
						pNewPoints = (HPDPOINT)GlobalLock (hNewPoly);
						hmemmove ((HPSTR)pNewPoints,(HPSTR)pOrigPoints,((long)NumNewPoints)*sizeof(DPOINT));
						GlobalUnlock (hNewPoly);
						_fstrcpy (PltName,EditName);
				    	Newref=GetNewRefno(PltName,0,0,0,0);
						AddPolyToMap (1,&NumNewPoints, &hNewPoly,1,Newref,0,2,Desc,0,0,0,-1,-1,-1,0,0,0,0,HighlightData.PD.HiPrecis,0);
						GSSiGlobFree (&hNewPoly);
						CloseMap (TRUE);
						RemPoints -= (NumNewPoints - 1);
						pOrigPoints += (NumNewPoints - 1); 
					}
					GSSiGlobUlFree (&hPolyOrig);
				}
			} 
		}
		StatusWindowUpdate (0,0, nRecs, ++nLoaded);
	} 
    rtn = ContinueProcessing;    
    SetContinueProcessing ( TRUE);
	DestroyStatusWindow (0);
Exit:
	CurView = SaveVP;
	return rtn;
}  

BOOL HighlightSequentialItems (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam,short Function)
{HDC hDC;
 char key;     
 int	st;
 long	TLID;
 HIGHLIGHTDATA	HighlightData;
 POINT	MousePoint;
 static	char RouteFile1[144]="", RouteFile2[144]="",ChainedFile[144]="";  
 static	long		FromPath; 
 static	double		FromMP1, FromMP2; 
 static short		FromDir1, FromDir2;  
 static	DPOINT	FromPoint1, FromPoint2, FromPoint;   
 static	BOOL	NetOpened=FALSE, StreetOpened=FALSE,OpenedSeg=FALSE;   
 static	PICKDATA	FromPickList;
 long	Sequence;    

 switch (Message)
   {
   	case GF_INIT:
		{	BTVARDESC	BTVar[2]; 
			HIGHLIGHTDATA	HighlightData1, HighlightData2; 
			long	Refno;
		    
		    SkipHighlightedItems = GetGlobalLVal2("[%SKIPHLT] ",1);
		    if (WalkOutRouteOpt == GF_BUILD_NETWORK)
		    {
		    	SkipHighlightedItems = FALSE; 
				if (MessageBox( GetFocus(),"This command will replace your current street network - do you wish to continue?",
						"Verify Rebuild", MB_OKCANCEL) == IDCANCEL) 
				{
					PostMessage(hWnd, GF_CLOSE,0, 0L); 
					break;
				}
				DeleteStreetNetwork();
		    	StartFastPick (65);
			}
		    if ((RouteType == TWOPOINTROUTENET || WalkOutRouteOpt == GF_WALKOUT_ROUTE)&&
		    	!GetSegFromGlobals)
		    	RouteByNet = TRUE;         
		    else
		    	RouteByNet = FALSE;
		    NetOpened = StreetOpened = FALSE;
			if (RouteByNet || TraceNet)
				OpenNetLinkAndRef (1,FALSE,&NetOpened);
			if (RouteByNet) 
			{
		    	StartFastPick (65);
				StreetOpened = OpenStreetSegmentTable (FALSE,&OpenedSeg); 
			}
		    if (RouteType == TWOPOINTROUTE || RouteType == CLOSEDTWOPOINTROUTE)
		    {
			    st = BT_FIND (hHighlight2,(LPSTR)&Sequence,BT_LAST,BT_ANY,(LPSTR)&FromRef); 
			    if (st)
			    {
			 		MessageBox(GetFocus(), "No start segment", 0,MB_ICONEXCLAMATION|MB_OK);
			        PostMessage(hWnd, GF_CLOSE,0, 0L);
			    	break;
			    }  
			    BT_FIND (hHighlight,(LPSTR)&FromRef,BT_FIRST,BT_EQ,(LPSTR)&HighlightData1); 
			    PickList[0] = HighlightData1.PD; 
			    FromPickList = HighlightData1.PD; 
			    if (RouteByNet)
			    {   
			    	PickList[0].PCT = 0;
   		    		GetNetLocFromPick (0,&FromPath, &FromMP1, &FromDir1);
   		    		FromDir1 *= -1;
			    	PickList[0].PCT = 1;
   		    		GetNetLocFromPick (0,&FromPath, &FromMP2, &FromDir2);
   		    	}
			    FromPoint1 = HighlightData1.PD.BeginPoint;
			    FromPoint2 = HighlightData1.PD.EndPoint;
			    st = BT_FIND (hHighlight2,(LPSTR)&Sequence,BT_PRIOR,BT_ANY,(LPSTR)&Refno); 
			    if (!st)
			    {
			    	BT_FIND (hHighlight,(LPSTR)&Refno,BT_FIRST,BT_EQ,(LPSTR)&HighlightData2);
			    	if (ldistp(HighlightData1.PD.BeginPoint,HighlightData2.PD.BeginPoint) < RouteTOL)
			    		FromPoint1 = FromPoint2;
			    	else if (ldistp(HighlightData1.PD.BeginPoint,HighlightData2.PD.EndPoint) < RouteTOL)
			    		FromPoint1 = FromPoint2;
			    	else if (ldistp(HighlightData1.PD.EndPoint,HighlightData2.PD.BeginPoint) < RouteTOL)
			    		FromPoint2 = FromPoint1;
			    	else if (ldistp(HighlightData1.PD.EndPoint,HighlightData2.PD.EndPoint) < RouteTOL)
			    		FromPoint2 = FromPoint1;
			    } 
			    FromPoint.x = (FromPoint1.x + FromPoint2.x) / 2;
			    FromPoint.y = (FromPoint1.y + FromPoint2.y) / 2;    
				GSSiGetTempFileName (0,"gma",0,(LPSTR)RouteFile1);
			} 
			else if (RouteType == TWOPOINTROUTENET)
			{
				if (BT_NUM_IN_INDEX (hHighlight)<2)  
				{
			 		MessageBox(GetFocus(), "Insufficient segments highlighted", 0,MB_ICONEXCLAMATION|MB_OK);
			        PostMessage(hWnd, GF_CLOSE,0, 0L);
			    	break;
			    }
				GSSiGetTempFileName (0,"gmb",0,(LPSTR)RouteFile1);
			}
			else
			{
				FromPoint.x = 0;
				FromPoint.y = 0;  
				FromPoint1 = FromPoint;
				FromPoint2 = FromPoint;  
				FromRef = ZERO$;
				switch (WalkOutRouteOpt)
				{   
					case GF_BUILD_NETWORK:
					{
						OFSTRUCTGM	OFStruct;
						char	NetChainFile[MAX_PATH];
					
						GSSiGetTempFileName (0,"gmh",0,(LPSTR)ChainedFile);
						BTVar[0].BT_VARTYP=BT_INTEGER;
						BTVar[0].BT_VARLEN=4;
						BTVar[0].BT_VAROFF=0;
						BT_CREATE (ChainedFile, 4, FALSE, 1, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
						hBTChained= BT_OPEN (ChainedFile, 0, BT_WRITE, 0);
						GetTempDir (NetChainFile);
						strcat (NetChainFile,"\\NetChain.bin");
						FidChain = GSSiOpenFile (NetChainFile,&OFStruct,OF_CREATE);
						BTVar[0].BT_VARTYP=BT_INTEGER;
						BTVar[0].BT_VARLEN=4;
						BTVar[0].BT_VAROFF=0;
						BTVar[1].BT_VARTYP=BT_INTEGER;
						BTVar[1].BT_VARLEN=4;
						BTVar[1].BT_VAROFF=4;
						BT_CREATE ("intref.btr", sizeof(NETINTREFDATA), FALSE, 2, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
						hIntRef= BT_OPEN ("intref.btr", 0, BT_WRITE, 0);   
						IntersectionID=0; 
						GSSiGetTempFileName (0,"gma",0,(LPSTR)RouteFile1);
					}
					break;
					case GF_CONNECTIVITY_TEST: 
				    	StartFastPick (65);
						_fstrcpy (RouteFile1,"contest.wor");
					break;
					case GF_WALKOUT_ROUTE:
						GSSiGetTempFileName (0,"gmb",0,(LPSTR)RouteFile1);
					break;   
				} 
			}
			GSSiGetTempFileName (0,"gmc",0,(LPSTR)RouteFile2);
			BTVar[0].BT_VARTYP=BT_INTEGER;
			BTVar[0].BT_VARLEN=4;
			BTVar[0].BT_VAROFF=0;
			BT_CREATE (RouteFile1, sizeof(ATDATA), FALSE, 1, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
			hBTAt= BT_OPEN (RouteFile1, 0, BT_WRITE, 0);
			BTVar[0].BT_VARTYP=BT_REAL;
			BTVar[0].BT_VARLEN=8;
			BTVar[0].BT_VAROFF=0;
			BTVar[1].BT_VARTYP=BT_INTEGER;
			BTVar[1].BT_VARLEN=4;
			BTVar[1].BT_VAROFF=8;
			BT_CREATE (RouteFile2, sizeof(NEXTDATA), FALSE, 2, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
			hBTNext= BT_OPEN (RouteFile2, 0, BT_WRITE, 0); 
		}
		if (RouteType == TWOPOINTROUTENET)
			PostMessage(hWnd, GF_EXECUTE,CurView->ID, 0L); 
		else
			PostMessage(hWnd, GF_EXECUTE,CurView->ID, 0L); 
       		//AddLBUTTON = TRUE;
   		
   		break;  
   		
   	case GF_CLOSE:  
   		EndFastPick();
   		CloseNetLinkAndRef (NetOpened);
   		if (StreetOpened)
   			CloseStreetSegmentTable(OpenedSeg);
   		StreetOpened=FALSE;
   		if (hBTChained)
   		{
   			BT_CLOSE (hBTChained);
   			hBTChained = 0;         
   			BT_CLOSE (hIntRef);
   			hIntRef = 0;
   			GSSiRemove (ChainedFile);  
   			GSSiClose2 (&FidChain); 
	        FidChain = HFILE_ERROR;
   		}
   		if (hBTAt)
   		{   
   			SetGlobalValueBounds ("%ROUTEBOUNDS",&RouteBounds);
   			if (GetGlobalCVal ("[%ROUTENEXTFILE]",RouteNextFile,0))
   				SaveRouteNextFile (hBTNext,RouteNextFile);
   			if (GetGlobalCVal ("[%ROUTEATFILE]",RouteAtFile,0))
				SaveRouteAtFile (hBTAt,RouteAtFile);
   			BT_CLOSE(hBTAt);
   			BT_CLOSE(hBTNext);
   			hBTAt=hBTNext=0;
			if (WalkOutRouteOpt==GF_BUILD_NETWORK ||
			    RouteType == TWOPOINTROUTE ||
			    RouteType == TWOPOINTROUTENET ||
			    RouteType == CLOSEDTWOPOINTROUTE)
				GSSiRemove(RouteFile1);
   			GSSiRemove(RouteFile2); 
			if (!DisplayMarkers && RouteType == WALKOUTROUTE)
			{
		        CurView->CurZoomAreaRef = 0;
		   		RedisplayViewport(FALSE,FALSE);
		   	}
   		} 
   		else
   			Sound(BAD_SOUND);
        return FALSE;
   		break;

    case GF_EXECUTE:
    case WM_LBUTTONUP: 
    {   
    	VISLIST	SaveVis; 
    	DPOINT	ToPoint1, ToPoint2, ToPoint, TPoint; 
    	double	MaxCost = MAX_COST;  
   // 	double	Cost;
    	long	LinkRef = LONG_MIN, LinkToRef, LastRef, NextRef;
    	NEXTKEY		SaveNextKey;
    	NEXTDATA	SaveNextData;
		HCURSOR	hcurSave; 
		long	ToPath;
		double	ToMP1, ToMP2;
		short	ToDir1, ToDir2;
		int		ii, TolFac=1,SaveMaxPick; 
		BOOL	Opened1, Opened2; 
	    BOOL	SaveUseRefOrTAGIndex=UseRefOrTAGIndex;
		short	pos=BT_FIRST;
		BOOL	SaveDisplay = Display;
	    
	    DBoundsInit (&RouteBounds);
   		if (!hBTAt) 
   		{
	        PostMessage(hWnd, GF_CLOSE,0, 0L);
   			break;
    	}
//AddSpeed (); 
	   	SetPrompt (PRMT_STARTROUTE,TRUE); 
		if (RouteByNet)
		{
			OpenNetLinkAndRef (NetworkID,TRUE,&Opened1); 
			OpenNetIntersect (NetworkID,FALSE,&Opened2); 
		}
		else
		    UseRefOrTAGIndex = FALSE;
 
    	Display = FALSE;
    	UniqueCost = 0; 
    	WhenAssigned=0;
		hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT)); 
		if (RouteType == CLOSEDTWOPOINTROUTE)
		{   
			long	EndSeq;
			int		nPoints;
			HANDLE	hPoints;
			LPDPOINT	pPoints;
			
			hPoints = GetConnectedItems (FALSE,&nPoints);
			pPoints = (LPDPOINT)GlobalLock (hPoints);
			ToPoint = *pPoints;
			GSSiGlobUlFree (&hPoints);
		}
	    else if (RouteType == WALKOUTROUTE || RouteType == TWOPOINTROUTE)  
	    {
	    	if (CursorIsLocked)
		    { 
				ToPoint = CurrentPoint;
		    	UnlockCursor ();
		    }
			else
			{    	
		    	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
			    ToPoint=WinPtToBasePt(MousePoint);  
			}
			FromPoint = ToPoint;
		}
		EnlargeScreen (0,0);
		SelectVisList (TRUE); 
		SaveVis = *CurVis;
		CurVis->WantType[0]=FALSE;
		CurVis->WantType[2]=FALSE;
		SaveMaxPick = MaxPick;
        NumPicked = 0;
		MaxPick=GetGlobalLVal2("[%ROUTEMAXPICK]",8);  
		MaxPick = min (MaxPick,MAXPICKITEMS-4);
		if (WalkOutRouteOpt == GF_BUILD_NETWORK ||
			RouteType == TWOPOINTROUTE ||
			RouteType == CLOSEDTWOPOINTROUTE ||
			WalkOutRouteOpt == GF_CONNECTIVITY_TEST)
		{
		    PickItems (hWnd,ToPoint); 
		    if (!NumPicked)
		    	st = 1;
		    else
		    {
		    	NumPicked--;
		    	st = 0; 
		    	ToRef = PickList[NumPicked].Refno;    
		    }
		}
		UseUserPickAp =FALSE;
		SystemPickAp = GetGlobalDVal2("[%ROUTEPICKAP]",-10);
		SetPickAp(0);
		LimitRouteToScreen = GetGlobalBVal2 ("[%LIMITROUTETOAREA]",TRUE);	
		if (RouteType == TWOPOINTROUTENET)
		{ 
		    st = BT_FIND (hHighlight2,(LPSTR)&Sequence,pos,BT_ANY,(LPSTR)&FromRef); 
		    pos = BT_NEXT;
		    BT_FIND (hHighlight,(LPSTR)&FromRef,BT_FIRST,BT_EQ,(LPSTR)&HighlightData); 
		    PickList[0] = HighlightData.PD; 
		    if (RouteByNet)
		    {   
		    	PickList[0].PCT = 0;
	    		GetNetLocFromPick (0,&FromPath, &FromMP1, &FromDir1);
	    		FromDir1 *= -1;
		    	PickList[0].PCT = 1;
	    		GetNetLocFromPick (0,&FromPath, &FromMP2, &FromDir2);
	    	}
		    FromPoint1 = HighlightData.PD.BeginPoint;
		    FromPoint2 = HighlightData.PD.EndPoint;
		    FromPoint.x = (FromPoint1.x + FromPoint2.x) / 2;
		    FromPoint.y = (FromPoint1.y + FromPoint2.y) / 2;    
		    st = BT_FIND (hHighlight2,(LPSTR)&Sequence,pos,BT_ANY,(LPSTR)&PickList[NumPicked].Refno); 
	    	st = BT_FIND (hHighlight,(LPSTR)&PickList[NumPicked].Refno,BT_FIRST,BT_EQ,(LPSTR)&HighlightData); 
		    PickList[0] = HighlightData.PD;  
		    ToPoint = PickList[0].PickedPoint;
		}
		else if (Function != GF_HIGHLIGHT_SEQUENTIAL)  
		{
		    st = BT_FIND (hHighlight2,(LPSTR)&Sequence,BT_FIRST,BT_ANY,(LPSTR)&PickList[NumPicked].Refno); 
	    	if (!st)
	    		BT_FIND (hHighlight,(LPSTR)&PickList[NumPicked].Refno,BT_FIRST,BT_EQ,(LPSTR)&HighlightData); 
	    	PickList[NumPicked] = HighlightData.PD;
	    }
	    RouteTOL = TolFac * FileDistToBaseDist;
		SetDisplayMode (CurView->hDC, GF_TEXTMODE);
	    GSSiDeleteObject(&CurView->hRgn);
		CurView->hRgn = CreateVPRgn (FALSE,FALSE);
	  	SelectClipRgn (CurView->hDC,CurView->hRgn);
	  	GSSiDeleteObject(&CurView->hRgn);  
	  	pos = BT_FIRST;  
	  	Processing = TRUE;
	  	while (!st)
	  	{
	    	if (RouteByNet) 
	    	{   
	    		PickList[NumPicked].PCT = 0;
	    		GetNetLocFromPick (NumPicked,&ToPath, &ToMP1, &ToDir1);
	    		ToDir1 *= -1; 
	    		PickList[NumPicked].PCT = 1;
	    		GetNetLocFromPick (NumPicked,&ToPath, &ToMP2, &ToDir2); 
	    	}
			_fmemset (&AtData,0,sizeof(AtData));
	    	AtData.LastRef = NULLREF;  
	    	ToRef = PickList[NumPicked].Refno;
	    	AtData.StartRef = ToRef;    
			AtData.Segment = PickList[NumPicked].Segment;
			AtData.Offset = PickList[NumPicked].Offset;
			AtData.FileNum = PickList[NumPicked].FileNum;  
			AtData.SubFile = CurView->SubFile;
	        AtData.FileInIndex = PickList[NumPicked].FileInIndex;
	        AtData.SymNum = PickList[NumPicked].Desc;
	        AtData.Type = PickList[NumPicked].Type;
	        _fmemmove (AtData.StreetNums,PickedStreets[NumPicked],16);
	    	ToPoint1 = PickList[NumPicked].BeginPoint;
	    	ToPoint2 = PickList[NumPicked].EndPoint;
	    	BT_PUT(hBTAt,(LPSTR)&ToRef,(LPSTR)&AtData);
	        if (FromRef !=ZERO$)
	        {
		    	AtData.LastRef = NULLREF;
		    	AtData.NextRef = NULLREF;
		    	AtData.StartRef = FromRef;
				AtData.Segment = FromPickList.Segment;
				AtData.Offset = FromPickList.Offset;
				AtData.FileNum = FromPickList.FileNum;
				AtData.SubFile = FromPickList.SubFile;  
		        AtData.FileInIndex = FromPickList.FileInIndex; 
		        AtData.Type = FromPickList.Type;
		        AtData.SymNum = FromPickList.Desc;
		        AtData.NumNext = 0; 
//		        _fmemmove (AtData.StreetNums,PickedStreets[NumPicked],16);
		    	BT_PUT(hBTAt,(LPSTR)&FromRef,(LPSTR)&AtData);
		    	AddNextRefs(FromRef,FromPoint1,FromPath,FromMP1,FromDir1,FromRef,0,ToPoint,0,LONG_MIN);
		    	if (ldistp(FromPoint1,FromPoint2) > RouteTOL) 
		    		AddNextRefs(FromRef,FromPoint2,FromPath,FromMP2,FromDir2,FromRef,0,ToPoint,0,LONG_MIN);  
		    }
	    	AddNextRefs(ToRef,ToPoint1,ToPath,ToMP1,ToDir1,ToRef,0,FromPoint,0,LONG_MIN); 
	    	AddNextRefs(ToRef,ToPoint2,ToPath,ToMP2,ToDir2,ToRef,0,FromPoint,0,LONG_MIN);
			if (WalkOutRouteOpt == GF_BUILD_NETWORK ||
				 ((RouteType == TWOPOINTROUTE || RouteType == CLOSEDTWOPOINTROUTE) && !RouteByNet) )
				st = 1;
			else
			{   
				NumPicked=0;
		    	st = BT_FIND (hHighlight2,(LPSTR)&Sequence,BT_NEXT,BT_ANY,(LPSTR)&PickList[NumPicked].Refno); 
		    	if (!st)
		    		BT_FIND (hHighlight,(LPSTR)&PickList[NumPicked].Refno,BT_FIRST,BT_EQ,(LPSTR)&HighlightData); 
		    	PickList[NumPicked] = HighlightData.PD;
		    }
	   	}
    	if (BT_FIND (hBTNext,(LPSTR)&NextKey,BT_FIRST,BT_ANY,(LPSTR)&NextData))
    		NextKey.Cost = INFINITE_COST; 
    	while (NextKey.Cost < MaxCost && (SetContinueProcessing ( CheckForContinue (TRUE,0))))   
    	{   
    		SaveNextKey = NextKey;
    		SaveNextData = NextData;
    		st = BT_FIND (hBTAt,(LPSTR)&NextKey.Refno,BT_FIRST,BT_EQ,(LPSTR)&AtData);
    		if (!st && RouteType != WALKOUTROUTE && AtData.StartRef != NextData.StartRef)
    		{   double	Cost;
		    		
    			Cost = NextData.AtCost + AtData.Cost; 
    			if (Cost < MaxCost)
    			{
    				MaxCost = Cost;
	    			LinkRef = NextKey.Refno;
	    			LinkToRef = NextData.AtRef;
	    			DisplayMarker(NextData.OpenEnd,6,0,0,0,0,TRUE,FALSE,0,0,0,0,0); 
	    		}
    		}
    		else
    		{   
    			if (!st)
    			{
    				if (AtData.Cost <= NextData.NextCost)
    					goto S10;
    				BT_DELETE (hBTAt,(LPSTR)&NextKey.Refno,(LPSTR)&AtData,FALSE);
    			}
    			DisplayMarker(NextData.OpenEnd,1,0,0,0,0,TRUE,FALSE,0,0,0,0,0);
    			if (NextData.StartRef == FromRef)
    				TPoint = ToPoint;
    			else
    				TPoint = FromPoint; 
    			AtData.NextRef = NULLREF;
		    	AtData.LastRef = NextData.AtRef;
		    	AtData.StartRef = NextData.StartRef;
		    	AtData.Cost = NextData.NextCost;
		    	AtData.Sequence = NextData.Sequence;
				AtData.Segment = NextData.Segment;
				AtData.Offset = NextData.Offset;
				AtData.FileNum = NextData.FileNum; 
				AtData.SubFile = NextData.SubFile;  
		        AtData.FileInIndex = NextData.FileInIndex; 
		        AtData.SymNum = NextData.SymNum;
		        AtData.Type = NextData.Type; 
		        AtData.FromPCT = NextData.FromPCT;
		        AtData.ToPCT = NextData.ToPCT;
		        AtData.NumNext = 0; 
		        _fmemmove (AtData.StreetNums,NextData.StreetNums,16);   
		        LastRef = NextData.AtRef; 
		        NextRef = NextKey.Refno;
				    	
    	//		Cost = NextKey.Cost - ldistp(NextData.OpenEnd,TPoint);
		    	BT_PUT(hBTAt,(LPSTR)&NextRef,(LPSTR)&AtData); 
    			if (AddNextRefs(NextKey.Refno,NextData.OpenEnd,NextData.Path,NextData.MP,NextData.Dir,
    							NextData.StartRef,AtData.Cost,TPoint,AtData.Sequence+1,LastRef))  
    			{
    				AtData.NextRef = LONG_MIN;
			    	BT_PUT(hBTAt,(LPSTR)&NextRef,(LPSTR)&AtData); 
                }
		    	BT_FIND(hBTAt,(LPSTR)&LastRef,BT_FIRST,BT_EQ,(LPSTR)&LastAtData);
		    	LastAtData.NextRef = NextRef;
		    	LastAtData.NumNext++; 
		    	if (LastRef == 183636889)
		    		ii=1;
		    	BT_PUT(hBTAt,(LPSTR)&LastRef,(LPSTR)&LastAtData); 
    		}
    	S10: 
    		if (BT_DELETE (hBTNext,(LPSTR)&SaveNextKey,(LPSTR)&SaveNextData,FALSE))
    			ii=1;  
   			SaveNextKey.Cost = INFINITE_COST;
   			BT_PUT (hBTNext,(LPSTR)&SaveNextKey,(LPSTR)&SaveNextData);  
	    	if (BT_FIND (hBTNext,(LPSTR)&NextKey,BT_FIRST,BT_ANY,(LPSTR)&NextData))
	    		NextKey.Cost = INFINITE_COST;
    	} 
    	if (ContinueProcessing && RouteType == WALKOUTROUTE && GetGlobalBVal2("[%HIGHLIGHTROUTE]",FALSE))
    	{   
    		short	pos=BT_FIRST;
    		long	Ref;  
    		BOOL	SavePick;
    		
    		Sequence = 0; 
    		st = BT_FIND (hBTAt,(LPSTR)&Ref,BT_FIRST,BT_ANY,(LPSTR)&AtData);
		    while (!BT_FIND (hBTAt,(LPSTR)&Ref,pos,BT_ANY,(LPSTR)&AtData))
		    {   
		    	pos = BT_NEXT;
				Sequence += 1000; 
				PickList[0].Refno = Ref;
		    	PickList[0].Segment = AtData.Segment;
		    	PickList[0].Offset = AtData.Offset;
		    	PickList[0].ViewID = CurView->ID;
				PickList[0].ConfigID = CurrentConfig;
		    	PickList[0].FileNum = AtData.FileNum;  
		    	PickList[0].SubFile = AtData.SubFile;
		    	PickList[0].FileInIndex = AtData.FileInIndex;
		    	NumPicked=0;
		    	IgnoreBounds = TRUE;
		    	ProcessPickedItem(0,-1); 
//				PickedItemMinMax (NumPicked,&PickList[NumPicked].Rect);
				PickList[0].Type = PickTypeFromSysType (CurrentType);; 
				PickList[0].Desc = CurrentDesc;
				_fstrcpy (PickList[0].Prefix,CurrentPrefix);
				_fstrcpy (PickList[0].UDI,CurrentUDI);
		    	PickList[0].Segment = AtData.Segment;
		    	PickList[0].Offset = AtData.Offset;
		    	IgnoreBounds = FALSE;
				AddToHighlightListSeq (Ref,&PickList[0],Sequence,TRUE);
				SavePick=Pick;
				Pick=FALSE;   
		    	ProcessPickedItem(0,TRUE);
		    	Pick=SavePick;
			}
        }
	    else if (ContinueProcessing && LinkRef > LONG_MIN)
	    {   
	    	long	NumFrom,NumTo,Sequence,StartSeq,Ref1,Ref2,Ref; 
	    	double	ActualCost=0; 
	    	char	str[144]; 
	    	BOOL	SavePick;
	    	
		    ii=BT_FIND (hBTAt,(LPSTR)&LinkRef,BT_FIRST,BT_EQ,(LPSTR)&AtData);
		    if (AtData.StartRef == FromRef) 
		    {   
		    	Ref1 = LinkRef;
		    	NumFrom = AtData.Sequence;
		    }
		    else  
		    {
		    	Ref2 = LinkRef;
		    	NumTo = AtData.Sequence;
		    }
		    ii=BT_FIND (hBTAt,(LPSTR)&LinkToRef,BT_FIRST,BT_EQ,(LPSTR)&AtData);
		    if (AtData.StartRef == FromRef) 
		    {
		    	Ref1 = LinkToRef;
		    	NumFrom = AtData.Sequence;
		    }
		    else  
		    {
		    	Ref2 = LinkToRef;
		    	NumTo = AtData.Sequence;
		    } 
		    Pick=PickingByRefno=TRUE;
		    Ref = Ref1;
		    st = BT_FIND (hBTAt,(LPSTR)&Ref,BT_FIRST,BT_EQ,(LPSTR)&AtData);
		    StartSeq = HighlightSequence + (NumFrom+1) * 1000; 
		    Sequence = StartSeq;  
		    while (!st && Ref != FromRef)
		    {   
		    	PickList[0].Segment = AtData.Segment;
		    	PickList[0].Offset = AtData.Offset;
		    	PickList[0].ViewID = CurView->ID;
				PickList[0].ConfigID = CurrentConfig;
		    	PickList[0].FileNum = AtData.FileNum;
		    	PickList[0].SubFile = AtData.SubFile;
		    	PickList[0].FileInIndex = AtData.FileInIndex;
		    	NumPicked=0; 
		    	IgnoreBounds = TRUE;
		    	ProcessPickedItem(0,-1);
		    	IgnoreBounds = FALSE; 
		    	PickList[0].Segment = AtData.Segment;
		    	PickList[0].Offset = AtData.Offset;
		    	ActualCost+=PickList[0].Length;
				AddToHighlightListSeq (Ref,&PickList[0],Sequence,TRUE); 
				SavePick=Pick;
				Pick=FALSE;   
		    	ProcessPickedItem(0,TRUE);
		    	Pick=SavePick;
				Sequence -= 1000; 
				Ref = AtData.LastRef;
		    	st = BT_FIND (hBTAt,(LPSTR)&Ref,BT_FIRST,BT_EQ,(LPSTR)&AtData);
			}
		    Ref = Ref2;
		    st = BT_FIND (hBTAt,(LPSTR)&Ref,BT_FIRST,BT_EQ,(LPSTR)&AtData);
		    Sequence = StartSeq;  
		    Ref1 = FromRef;
		    while (!st && Ref1 != ToRef)
		    {   
				Sequence += 1000; 
		    	PickList[0].Segment = AtData.Segment;
		    	PickList[0].Offset = AtData.Offset;
		    	PickList[0].ViewID = CurView->ID;
				PickList[0].ConfigID = CurrentConfig;
		    	PickList[0].FileNum = AtData.FileNum;  
		    	PickList[0].SubFile = AtData.SubFile;
		    	PickList[0].FileInIndex = AtData.FileInIndex;
		    	NumPicked=0;
		    	IgnoreBounds = TRUE;
		    	ProcessPickedItem(0,-1); 
		    	PickList[0].Segment = AtData.Segment;
		    	PickList[0].Offset = AtData.Offset;
		    	IgnoreBounds = FALSE;
				AddToHighlightListSeq (Ref,&PickList[0],Sequence,TRUE);
				SavePick=Pick;
				Pick=FALSE;   
		    	ProcessPickedItem(0,TRUE);
		    	Pick=SavePick;
				Ref1 = Ref;  
				Ref = AtData.LastRef; 
				if (Ref1 != ToRef)
			    	ActualCost+=PickList[0].Length;
		    	st = BT_FIND (hBTAt,(LPSTR)&Ref,BT_FIRST,BT_EQ,(LPSTR)&AtData);
			}  
			SetGlobalValueReal ("%MAXROUTECOST",MaxCost);
			SetGlobalValueReal ("%ACTUALROUTECOST",ActualCost);
	    } 
	    Processing = FALSE; 
	    if (!CurView)
	    {
	    	SetViewport (*pCommandViewport);
	    	SelectVisList (FALSE);
	    }
	    *CurVis = SaveVis;  
		UseRefOrTAGIndex= SaveUseRefOrTAGIndex;
		MaxPick = SaveMaxPick;  
		UseUserPickAp =TRUE;
	    Pick = PickingByRefno = FALSE;
	    CloseNetLinkAndRef (Opened1);
		CloseNetIntersect(Opened2); 
	    GSSiSetCursor(hcurSave);
	    Display = SaveDisplay;
        PostMessage(hWnd, GF_CLOSE,0, 0L); 
    }
		break;
    case WM_CHAR:
		key = wParam;
		switch (key)
		{
			case 27: //ESC
		        PostMessage(hWnd, GF_CLOSE, 0,0L); 
		        return TRUE;
		}
       	return FALSE;

    default:
    	return (FALSE);
    }
    return (TRUE);
}



