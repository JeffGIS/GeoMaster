#include "graphint.h"
 
#include "gmextern.h"

static	POINT	PosToPoint[10], PosFromPoint;
static	struct {long	IntID, Ref;} IntRefKey;
static long	ChainID=0;
static NETINTMPKEY	NetIntMPKey;
static NETINTREFKEY	NetIntRefKey;
static NETINTREFDATA	NetIntRefData;

typedef struct {
				long		Refno;
				float		FromPCT;
				} STREETSEGMENT; 
typedef STREETSEGMENT	FAR	*LPSTREETSEGMENT;    


long PointsBetweenPCT(long Refno, double FromPCT, double ToPCT, BOOL ShapePointsOnly, LPHANDLE phPoints, LPHANDLE phCurvePoints, HANDLE hNewBPEP)
{
	long	npnts;
	int		PolyID;
	BOOL	Reverse = FALSE;
	HANDLE	hPoly;
	HPDPOINT	OutPoints;
	int		NumOutPoints = 0;

	*phPoints = *phCurvePoints = 0;
	if (hNewBPEP)
		ShapePointsOnly = FALSE;
	if (PickByRefno(Refno, NULL, NULL, -101))
	{
		BOOL	Reverse = FALSE;

		PolyID = FromPCT / 1000;
		PickList[0].PolyID = PolyID;
		FromPCT -= PolyID * 1000;
		ToPCT -= PolyID * 1000;
		if (FromPCT > ToPCT)
		{
			Reverse = TRUE;
			FromPCT = 1.0 - FromPCT;
			ToPCT = 1.0 - ToPCT;
		}
		if (GetPolyPoints((LPPICKDATAHEADER)&PickList[0], Reverse, &npnts, &hPoly, 0))
		{
			LPDPOINT pPoly = (LPDPOINT)GlobalLock(hPoly);
			double	PolyLength = GetPolyLengthD(pPoly, npnts);
			double	StartDist = FromPCT * PolyLength;
			double	EndDist = ToPCT * PolyLength;
			float	RSQ;

			*phPoints = GetPolyBetweenDist(pPoly, npnts, StartDist, EndDist, &NumOutPoints, ShapePointsOnly, FALSE);
			if (hNewBPEP && *phPoints)
			{
				HPDPOINT	Point = GlobalLock(*phPoints);
				HPDPOINT	NewBPEP = GlobalLock(hNewBPEP);
				UINT		i;
				HANDLE		hTran;
				double		FromX[2], FromY[2], ToX[2], ToY[2];

				ToX[0] = NewBPEP[0].x;
				ToY[0] = NewBPEP[0].y;
				ToX[1] = NewBPEP[1].x;
				ToY[1] = NewBPEP[1].y;
				FromX[0] = Point[0].x;
				FromY[0] = Point[0].y;
				FromX[1] = Point[NumOutPoints - 1].x;
				FromY[1] = Point[NumOutPoints - 1].y;
				hTran = STRAN2(1639, FromX, FromY, ToX, ToY, 2, &RSQ, 2, 0);
				for (i = 0; i < NumOutPoints; i++)
					Point[i] = TranPoint(&Point[i], hTran);
				GlobalUnlock(*phPoints);
				GlobalUnlock(hNewBPEP);
				CloseTRANS2(&hTran);
			}
			GSSiGlobUlFree(&hPoly);
		}
	}
	else
		ii = 1;
	return NumOutPoints;
}

long GetDeflectionPoints(double minDeflection, HFILE fidOut)
{
	long	npnts;
	int		PolyID;
	BOOL	Reverse = FALSE;
	HANDLE	hPoly;
	HPDPOINT	OutPoints;
	int		numDeflection = 0;
	int	TotNum = BT_NUM_IN_INDEX(hHighlight);
	int pos = BT_FIRST;
	int Refno;
	HIGHLIGHTDATA HighlightData;

	if (!TotNum)
	{
		MessageBox(GetFocus(), "No records highlighted", NULL, MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}
	while (!BT_FIND(hHighlight, (LPSTR)&Refno, pos, BT_ANY, (LPSTR)&HighlightData))
	{
		pos = BT_NEXT;
		PickList[0] = HighlightData.PD;

		if (GetPolyPoints((LPPICKDATAHEADER)&PickList[0], FALSE, &npnts, &hPoly, 0))
		{
			LPDPOINT pPoly = (LPDPOINT)GlobalLock(hPoly);
			for (int i = 0; i < npnts-2; i++)
			{
				double AZ1 = getazd(&pPoly[i], &pPoly[i+1]);
				double AZ2 = getazd(&pPoly[i+1], &pPoly[i+2]);
				double deflection = DeflectionAngle(AZ1, AZ2);
				if (fabs(deflection) > minDeflection)
				{
					char str[256];
					sprintf(str, "%f %f\t%i\t%s", pPoly[i + 1].x, pPoly[i + 1].y, npnts, PickList[0].UDI);
					fputstring(str, fidOut);
					numDeflection++;
				}
			}

			GSSiGlobUlFree(&hPoly);
		}
	}
	return numDeflection;
}

long PointsBetweenMP (long Path,double FromMP, double ToMP,LPHANDLE phPoints)
{   
	BOOL	Opened;
	NETREFSKEY		NetRefsKey,NetRefsKey2;
	NETREFSDATA		NetRefsData,NetRefsData2;
	short	pos=BT_FIRST,cond=BT_GT;
	long	NumSegs=0, LinkID, npnts;   
	BOOL	Reverse=FALSE;     
	double	SaveMP = FromMP;
	HANDLE	hPoly;    
	HPDPOINT	OutPoints;  
	long	NumOutPoints=0;
	
	*phPoints = 0;
	if (FromMP > ToMP) 
	{
		FromMP = ToMP;
		ToMP = SaveMP;
		Reverse = TRUE;
	}
	if (!OpenNetLinkAndRef (NetworkID,FALSE,&Opened))
		return 0;  
	*phPoints = GSSiGlobAlloc(GAIDNO 0,GMEM_MOVEABLE,USHRT_MAX);      
	OutPoints = (HPDPOINT)GlobalLock (*phPoints);
	NetRefsKey.Path = Path;
	NetRefsKey.MP = FromMP; 
	while (!BT_FIND (hBTNetRefs,(LPSTR)&NetRefsKey,pos,cond,(LPSTR)&NetRefsData))
	{
		pos = BT_NEXT;
		cond = BT_ANY;  
		if (NetRefsKey.Path != Path)
			break;   
		if (NetRefsKey.MP > ToMP)
			break;  
		NumSegs++;
		if (PickByRefno(NetRefsData.Ref,NULL,NULL,-1))
		{
	   		if (GetPolyPoints ((LPPICKDATAHEADER)&PickList[0],FALSE,&npnts,&hPoly, 0))
	   		{ 
	   			LPDPOINT pPoly = (LPDPOINT)GlobalLock (hPoly);  
	   			long	i;
	   			
	   			for (i=0;i<npnts;i++)
	   				OutPoints[NumOutPoints++]=pPoly[i];	
				//long GetPointsBetweenDist (long nPnts,HPDPOINT Points,double FromDist, double ToDist, HPDPOINT OutPoints)
	   			GSSiGlobUlFree (&hPoly);
			}
		}
	}
	GlobalUnlock (*phPoints);
    CloseNetLinkAndRef (Opened);                 
	return NumOutPoints;
}

int StreetSegFromNet (int i)
{   
	NETLINKSKEY	NetLinksKey; 
	NETLINKSDATA	NetLinksData; 
	short	pos=BT_FIRST, len;
    LPGWDHEADER lpGWDHead; 
    LPSEGDATAGM	pSegdata; 
    long	TLID, Offset, LastTLID;
    BOOL	rtn=FALSE, Opened, OpenedSeg;   
    LPSTR	lpDot, lpDotIndex;  
    short	WhichStreet;
	
	OpenNetLinkAndRef (NetworkID,FALSE,&Opened); 
	OpenStreetSegmentTable (TRUE,&OpenedSeg);
	lpGWDHead = (LPGWDHEADER)GlobalLock (hDBStreetSegments); 
	pSegdata = (LPSEGDATAGM)&lpGWDHead->GWDData;  
	_fmemset (pSegdata,0,sizeof(SEGDATAGM));
	NetLinksKey.Ref = LONG_MIN;
	NetLinksKey.NetID = 0;
	NetLinksKey.Path = 0; 
	LastTLID = LONG_MAX;  
	WhichStreet=0;
	while (!BT_FIND (hBTNetLinks,(LPSTR)&NetLinksKey,pos,BT_ANY,(LPSTR)&NetLinksData))
    {   
    	pos = BT_NEXT;
    	TLID = NetLinksKey.Ref;
    	if (TLID != LastTLID)
    	{ 
    		if (LastTLID != LONG_MAX)
    		{   
    			pSegdata->TLID = LastTLID;
		        Offset = GSSillseek (lpGWDHead->Fid,0,2);
		        BT_PUT (lpGWDHead->BTHandle[0],(LPSTR)&LastTLID,(LPSTR)&Offset);  
			  	len = sizeof(SEGDATAGM);
			   	BigWrite (lpGWDHead->Fid,(HPSTR)&len,2,-1);
			   	BigWrite (lpGWDHead->Fid,(HPSTR)pSegdata,len,-1);
		    } 
		    WhichStreet = 0;
		    _fmemset (pSegdata->StreetNum,0,16);
		} 
		pSegdata->StreetNum[WhichStreet++]=NetLinksKey.Path;
		LastTLID = TLID;
	}
	pSegdata->TLID = LastTLID;
    Offset = GSSillseek (lpGWDHead->Fid,0,2);
    BT_PUT (lpGWDHead->BTHandle[0],(LPSTR)&LastTLID,(LPSTR)&Offset);  
  	len = sizeof(SEGDATAGM);
   	BigWrite (lpGWDHead->Fid,(HPSTR)&len,2,-1);
   	BigWrite (lpGWDHead->Fid,(HPSTR)pSegdata,len,-1);  
   	GlobalUnlock (hDBStreetSegments);
	CloseStreetSegmentTable (OpenedSeg);
   	CloseNetLinkAndRef (Opened); 
    return TRUE;
}


BOOL RefInNet (long Ref)
{  
	NETLINKSKEY	NetLinksKey; 
	NETLINKSDATA	NetLinksData; 
	int	st;
	
	if (!NetworkID || !hBTNetLinks)
		return FALSE;
	NetLinksKey.Ref = Ref;
	NetLinksKey.NetID = NetworkID;
	NetLinksKey.Path = 0;
	st = BT_FIND (hBTNetLinks,(LPSTR)&NetLinksKey,BT_FIRST,BT_GE,(LPSTR)&NetLinksData); 
	if (st)
		return FALSE;
	if (NetLinksKey.Ref != Ref || NetLinksKey.NetID != NetworkID)
		return FALSE;
	else
		return TRUE;
} 

    

 

BOOL DeleteHighlightedMarkers (void)
{
	DPOINT	BasePoint;
	FILE	*Fid;  
	char	str[1024], txt[128];
	DPOINT	MPPoint;
	BOOL	Opened, Opened2;
	int		st,st2;
	NETLINKSKEY	NetLinksKey; 
	NETLINKSDATA	NetLinksData;
	NETREFSKEY	NetRefsKey;
	NETREFSDATA	NetRefsData; 
	NETMARKERSKEY1	NetMarkersKey1;
	NETMARKERSKEY2	NetMarkersKey2;
	NETMARKERSDATA1 NetMarkersData1;
	NETMARKERSDATA2	NetMarkersData2;
	HIGHLIGHTDATA	HighlightData;
	double			MPinc, PCT, MPVal, AZ;  
	char			snam[34], TrueName[34]="", Route[32];  
	int	iroute, i, NumChecked, Dummy, SaveMaxPick;     
	long	Checked[32];
	LPSTR	lpSpace;    
	long	WantStreetNum=0, TotNum, CurLoc;
	HCURSOR	hcurSave; 
	short	pos, LinkID, CheckedLink[32], NP;
	long	MarkerRef; 
	char	RouteAndMP[256];
	LPSTR	pRandMP; 
	BOOL	WildCard;
        
    TotNum = BT_NUM_IN_INDEX (hHighlight);
    CurLoc = 0;
	if (!TotNum)
	{
		MessageBox( GetFocus(),"No markers highlighted",NULL, MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	} 
	SetViewport(*pCommandViewport);
	SetDisplayMode (CurView->hDC, GF_TEXTMODE);
    GSSiDeleteObject(&CurView->hRgn);
	CurView->hRgn = CreateVPRgn (FALSE,FALSE);
  	SelectClipRgn (CurView->hDC,CurView->hRgn);
  	GSSiDeleteObject(&CurView->hRgn);
	hcurSave = GSSiSetCursor(LoadCursor(NULL, IDC_WAIT)); 
		  
	UseUserPickAp =FALSE;
	SystemPickAp = 0;	
	SaveMaxPick = MaxPick;        
	MaxPick=8;  
	PickNET=TRUE;
			  
	OpenNetLinkAndRef (NetworkID,FALSE,&Opened); 
	OpenNetMarkers (NetworkID,1,&Opened2); 
		      
	pos = BT_FIRST;
	while (!BT_FIND (hHighlight,(LPSTR)&MarkerRef,pos,BT_ANY,(LPSTR)&HighlightData))
	{ 
	  	pos = BT_NEXT; 
        PickList[0]=HighlightData.PD;
//        ProcessPickedItem (0,FALSE);    
      	MPPoint = HighlightData.PD.BeginPoint;
	    setDoPaint( FALSE);
	    PickItems (NULL,MPPoint); 
	    setDoPaint( TRUE);
	    AZ = PickList[NumPicked-1].PPAZ; 
	    while (NumPicked--)
	    {   
			if (PickList[NumPicked].OffDist > MarkerTOL)
				goto NextPick;   
			NetLinksKey.Ref = PickList[NumPicked].Refno;
			NetLinksKey.NetID = 0;
			st = BT_FIND (hBTNetLinks,(LPSTR)&NetLinksKey,BT_FIRST,BT_GE,(LPSTR)&NetLinksData); 
			while (!st && NetLinksKey.Ref == PickList[NumPicked].Refno)
			{   
				LinkID = NetLinksData.MP/1000000;	
							
				NetRefsKey.Path =  NetLinksKey.Path;
				NetRefsKey.MP = NetLinksData.MP; 
				st2 = BT_FIND (hBTNetRefs,(LPSTR)&NetRefsKey,BT_FIRST,BT_GE,(LPSTR)&NetRefsData);
				PCT = PickList[NumPicked].PCT;  
				AZ	= PickList[NumPicked].PPAZ;
				if (NetRefsData.Dir == 2)
					PCT = 1.0 - PCT;
				MPinc = PCT * PickList[NumPicked].Length;
				NetLinksData.MP += MPinc;  
				NetMarkersKey2.MarkerID=1;
				NetMarkersKey2.Path=NetLinksKey.Path;
				NetMarkersKey2.MP=NetLinksData.MP; 
				NetMarkersKey2.MP -= NetTOL;  
				if (!BT_FIND (hBTNetMarkers2,(LPSTR)&NetMarkersKey2,BT_FIRST,BT_GE,(LPSTR)&NetMarkersData2))
				{
					if (NetMarkersKey2.MarkerID == 1 &&
						NetMarkersKey2.Path == NetLinksKey.Path &&
						fabs (NetMarkersKey2.MP - NetLinksData.MP) <= NetTOL)
					{   
						NetMarkersKey1.MarkerID = NetMarkersKey2.MarkerID;
						NetMarkersKey1.Path = NetMarkersKey2.Path;
						_fmemmove (&NetMarkersKey1.Prefix[0],&NetMarkersData2.Prefix[0],sizeof(NETMARKERSDATA2));
						BT_DELETE (hBTNetMarkers2,(LPSTR)&NetMarkersKey2,(LPSTR)&NetMarkersData2,FALSE);
						BT_DELETE (hBTNetMarkers1,(LPSTR)&NetMarkersKey1,(LPSTR)&NetMarkersData1,FALSE);
					}
				}
				st = BT_FIND (hBTNetLinks,(LPSTR)&NetLinksKey,BT_NEXT,BT_ANY,(LPSTR)&NetLinksData); 
			}
NextPick:;					          		
       	} 
NextMarker:;
	}
	GSSiSetCursor (hcurSave);   
	CloseNetLinkAndRef (Opened);
	CloseNetMarkers (Opened2);  
	UseUserPickAp = TRUE; 
	MaxPick = SaveMaxPick; 
	PickNET = FALSE;
    return (TRUE);
} 
    	
    	
    

  
		    	


 

 

 





BOOL GetMilePointFromFrame2 (LPSTR NetDiskID,long Frame,LPLONG Path, LPSHORT MarkerID,LPMARKERVAL NetMarker,LPSHORT dir)
{
	BOOL	Opened;  
	NETVIDEOKEY2	NetVidKey2_1, NetVidKey2_2;
	NETVIDEODATA2	NetVidData2_1, NetVidData2_2;   
	
	OpenNetVideoIndex (NetworkID,FALSE,&Opened);
	_fstrncpy (NetVidKey2_2.DiskID,NetDiskID,16);
	NetVidKey2_2.Frame = Frame;
	NetVidKey2_1 = NetVidKey2_2;
	if (BT_FIND (hBTNetVideo2,(LPSTR)&NetVidKey2_2,BT_FIRST,BT_GE,(LPSTR)&NetVidData2_2))
		return FALSE;
	if (!_fmemcmp (&NetVidKey2_1,&NetVidKey2_2,sizeof(NETVIDEOKEY2)))
		NetVidData2_1 = NetVidData2_2;
	else if (BT_FIND (hBTNetVideo2,(LPSTR)&NetVidKey2_1,BT_PRIOR,BT_ANY,(LPSTR)&NetVidData2_1))
		return FALSE;    
	if (NetVidData2_1.Path != NetVidData2_2.Path ||
		NetVidData2_1.Dir != NetVidData2_2.Dir   ||
		NetVidData2_1.View != NetVidData2_2.View)
			return FALSE;
	if (_fmemicmp (NetVidData2_1.Prefix,NetVidData2_2.Prefix,4))
		return FALSE;  
	*dir = NetVidData2_2.Dir;    
	_fmemmove (NetMarker->Prefix,NetVidData2_1.Prefix,2);
	_fmemmove (NetMarker->Suffix,NetVidData2_1.Suffix,2);
	*MarkerID =  NetVidData2_1.MarkerID; 
	*Path = NetVidData2_1.Path;  
	if (NetVidKey2_2.Frame == NetVidKey2_1.Frame)
		NetMarker->Value = NetVidData2_1.Value;
	else
		NetMarker->Value = NetVidData2_1.Value +
			((double)(Frame - NetVidKey2_1.Frame) / (NetVidKey2_2.Frame - NetVidKey2_1.Frame)) *
			(NetVidData2_2.Value - NetVidData2_1.Value); 
	CloseNetVideoIndex (Opened);
	return TRUE;
}   

BOOL GetMilePointFromFrame (LPSTR NetDiskID,long Frame,LPLONG Path, LPSHORT MarkerID,LPMARKERVAL NetMarker,LPSHORT pVidDir)
{  
    short	Pass=0, MPDir;
    double	MP;
	char	TestName[34], Name[34];
	long	TestPath, PathIn;
    
    PathIn = *Path;
	if (!GetMilePointFromFrame2 (NetDiskID,Frame,Path,MarkerID,NetMarker,pVidDir))
		return FALSE;
   	GetTrueStreetName (*Path,Name,0,0);
   	if (PathIn != *Path)
   	{ 
   		if (GetTrueStreetName (PathIn,TestName,0,0))
   		{
   			if (SameRoute (Name,TestName))
   			{
				MP = GetMPFromMilePoint (PathIn,*MarkerID,NetMarker, &MPDir);
				if (MP >= 0)
				{
					*Path = PathIn;
					return TRUE;
				}  
			}
   		}
   	}
Top:
	MP = GetMPFromMilePoint (*Path,*MarkerID,NetMarker, &MPDir);
	if (MP < 0)
	{   
		if (Pass > 1)
			return FALSE; 
		TestPath = 0;
		switch (*pVidDir)
		{
			case 1: 
				_fstrcpy (TestName,Name);
				_fstrcat (TestName,"N"); 
				if (!Pass)
					TestPath = GetStreetNumFromName (TestName,1,BT_FIRST,TestName); 
                if (!TestPath)
                {   
                	Pass++;
    				_fstrcpy (TestName,Name);
    				_fstrcat (TestName,"E");
    				TestPath = GetStreetNumFromName (TestName,1,BT_FIRST,TestName); 
                }
			break;
			case 2:
				_fstrcpy (TestName,Name);
				_fstrcat (TestName,"S"); 
				if (!Pass)
					TestPath = GetStreetNumFromName (TestName,1,BT_FIRST,TestName); 
                if (!TestPath)
                {   
                	Pass++;
    				_fstrcpy (TestName,Name);
    				_fstrcat (TestName,"W");
    				TestPath = GetStreetNumFromName (TestName,1,BT_FIRST,TestName); 
                }
			break;  
		}
		Pass++;
		if (TestPath)
		{   
			*Path = TestPath;
	    	goto Top;
		}
		return FALSE;
	}
	return TRUE;
}

long AtNetIntersection (long Path, double MP1, double MP2,BOOL UseIgnoreSwitch)
{
	BOOL	Opened=FALSE;
	NETINTMPKEY NetIntMPKey, NetIntMPKeyLast; 
	double	MinMP, MaxMP; 
	long	NetIntAT=0, IntID, IntIDLast;
	short	st;
	static	long	IgnoreInt=0;
	
	if (!StopAtInt)
		return  0;
	if (fabs (MP1-MP2) <= NetTOL)
	{
		MP1 -= NetTOL/2;
		MP2 += NetTOL/2;
	} 
	if (MP1 < MP2)
	{
		MP1 -= DistTOL;
		MP2 += DistTOL;
		MinMP = MP1;   
		MaxMP = MP2;
	}
	else
	{
		MP1 += DistTOL;
		MP2 -= DistTOL;
		MinMP = MP2;   
		MaxMP = MP1;
	}
	OpenNetIntersect (NetworkID,FALSE,&Opened);  
	NetIntMPKey.Path = Path;
	NetIntMPKey.MP = MinMP;
	st = BT_FIND (hBTNetIntMP,(LPSTR)&NetIntMPKey,BT_FIRST,BT_GE,(LPSTR)&IntID); 
	if (!st && NetIntMPKey.Path == Path && NetIntMPKey.MP <= MaxMP)
		NetIntAT = IntID; 
	if (UseIgnoreSwitch && NetIntAT == IgnoreInt)
		NetIntAT = 0;//HaveTurnArrows = 0;
	IgnoreInt = NetIntAT;
	CloseNetIntersect (Opened);
	return NetIntAT;
} 

double GetNearLinkEP (long Path,double MP)
{
	NETREFSKEY	NetRefsKey;
	NETREFSDATA	NetRefsData;
	NETLINKSKEY	NetLinksKey; 
	NETLINKSDATA	NetLinksData; 
	double	FirstMP, LastMP, NearMP;
	long	LinkID;
	BOOL	Opened; 
	short	pos; 
                   
	OpenNetLinkAndRef (NetworkID,FALSE,&Opened);  
    LinkID = MP/1000000;
	NetRefsKey.Path = Path;
	NetRefsKey.MP = (double)LinkID * 1000000;
	BT_FIND (hBTNetRefs,(LPSTR)&NetRefsKey,BT_FIRST,BT_GT,(LPSTR)&NetRefsData);
	FirstMP = NetRefsKey.MP;
	NetRefsKey.Path = Path;
	NetRefsKey.MP = (double)(LinkID+1) * 1000000;
	if (BT_FIND (hBTNetRefs,(LPSTR)&NetRefsKey,BT_FIRST,BT_GE,(LPSTR)&NetRefsData))
		pos = BT_LAST;
	else
		pos = BT_PRIOR;
	BT_FIND (hBTNetRefs,(LPSTR)&NetRefsKey,BT_PRIOR,BT_ANY,(LPSTR)&NetRefsData);
	NetLinksKey.Ref = NetRefsData.Ref;
	NetLinksKey.NetID = NetworkID;
	NetLinksKey.Path = Path;
	if (BT_FIND (hBTNetLinks,(LPSTR)&NetLinksKey,BT_FIRST,BT_EQ,(LPSTR)&NetLinksData))
		LastMP = NetRefsKey.MP;  // should never happen
	else
		LastMP = NetRefsKey.MP + fabs (NetLinksData.Length);
	if (MP - FirstMP < LastMP - MP)
		NearMP = FirstMP;
	else
		NearMP = LastMP;
    CloseNetLinkAndRef (Opened);
	return NearMP;
}

BOOL MPInNet (long Path, double MP)
{
	NETREFSKEY	NetRefsKey1, NetRefsKey2;
	NETREFSDATA	NetRefsData1, NetRefsData2;  
	NETLINKSKEY	NetLinksKey; 
	NETLINKSDATA	NetLinksData; 
	int	st, pos; 
	BOOL	rtn=FALSE, Opened;  
	long	Link1, Link2, Link;
	
	OpenNetLinkAndRef (NetworkID,FALSE,&Opened);  
	if (!NetworkID || !hBTNetLinks)
		return FALSE;
	Link = MP/1000000;
	NetRefsKey2.Path = Path;
	NetRefsKey2.MP = MP;
	if (BT_FIND (hBTNetRefs,(LPSTR)&NetRefsKey2,BT_FIRST,BT_GE,(LPSTR)&NetRefsData2)) 
		pos = BT_LAST; 
	else 
	{
		if (fabs(MP - NetRefsKey2.MP) <= DistTOL)
		{
			rtn = TRUE; 
			goto Exit;
	    }
		pos = BT_PRIOR;
	}
	if (BT_FIND (hBTNetRefs,(LPSTR)&NetRefsKey1,pos,BT_ANY,(LPSTR)&NetRefsData1))
		goto Exit;
	Link1 = NetRefsKey1.MP/1000000; 
	Link2 = NetRefsKey2.MP/1000000; 
	if (NetRefsKey1.Path != Path || Link1 != Link)
		goto Exit; 
	if (NetRefsKey2.Path == Path && Link2 == Link)
		rtn = TRUE;
	else
	{
		NetLinksKey.Ref = NetRefsData1.Ref;
		NetLinksKey.NetID = NetworkID;
		NetLinksKey.Path = Path;
		if (BT_FIND (hBTNetLinks,(LPSTR)&NetLinksKey,BT_FIRST,BT_EQ,(LPSTR)&NetLinksData))
			goto Exit; 
		if (MP - NetRefsKey1.MP <= fabs (NetLinksData.Length)+DistTOL)
			rtn = TRUE;
	}
		
Exit: 
    CloseNetLinkAndRef (Opened);
	return rtn;
} 

BOOL GetNextMilePoint (long Path,int MarkerID,LPMARKERVAL NetMarker)
{
	NETMARKERSKEY1	NetMarkersKey;
	NETMARKERSDATA1 NetMarkersData;
	BOOL			Opened, rtn=FALSE;
	
	OpenNetMarkers (NetworkID,FALSE,&Opened);  
	NetMarkersKey.MarkerID=MarkerID;
	NetMarkersKey.Path = Path; 
	_fmemmove (NetMarkersKey.Prefix,NetMarker->Prefix,2);
	_fmemmove (NetMarkersKey.Suffix,NetMarker->Suffix,2);
	NetMarkersKey.Value = NetMarker->Value;
	if (BT_FIND (hBTNetMarkers1,(LPSTR)&NetMarkersKey,BT_FIRST,BT_GT,(LPSTR)&NetMarkersData))
		goto NotFound;
	if (NetMarkersKey.Path != Path || NetMarkersKey.MarkerID != MarkerID)
		goto NotFound; 
	NetMarker->Value = NetMarkersKey.Value;
	rtn = TRUE;
NotFound:
	CloseNetMarkers (Opened);
	return rtn;
}
 
double GetMPFromMilePoint (long Path,int MarkerID,LPMARKERVAL NetMarker, LPSHORT MPDir)
{
	NETMARKERSKEY1	NetMarkersKey1_1, NetMarkersKey1_2, NetMarkersKey1_3;
	NETMARKERSDATA1 NetMarkersData1_1, NetMarkersData1_2, NetMarkersData1_3;
	NETREFSKEY		NetRefsKey;
	NETREFSDATA		NetRefsData;  
	BOOL			Opened;
	double		MP, MPinc, MPValue;  
	int			st, st2, pos; 
	long		LinkID1=-1, LinkID2=-1, LinkID3;
	
	OpenNetMarkers (NetworkID,FALSE,&Opened);  
	NetMarkersKey1_2.MarkerID=MarkerID;
	NetMarkersKey1_2.Path = Path; 
	_fmemmove (NetMarkersKey1_2.Prefix,NetMarker->Prefix,2);
	_fmemmove (NetMarkersKey1_2.Suffix,NetMarker->Suffix,2);
	NetMarkersKey1_2.Value = NetMarker->Value;
	*MPDir = 1;  
	MP = -1;
	if (BT_FIND (hBTNetMarkers1,(LPSTR)&NetMarkersKey1_2,BT_FIRST,BT_GE,(LPSTR)&NetMarkersData1_2))
		pos = BT_LAST;
	else
	{
		pos = BT_PRIOR;
		LinkID2 = NetMarkersData1_2.MP/1000000;
	}
	if (!BT_FIND (hBTNetMarkers1,(LPSTR)&NetMarkersKey1_1,pos,BT_ANY,(LPSTR)&NetMarkersData1_1))
		LinkID1 = NetMarkersData1_1.MP/1000000;
	if (NetMarkersKey1_1.Path == NetMarkersKey1_2.Path && LinkID1 == LinkID2)
	{   
		if (NetMarkersKey1_1.Path == Path)
		{
			if (NetMarkersData1_1.MP > NetMarkersData1_2.MP)
				*MPDir = -1;
			MP = NetMarkersData1_1.MP +
				 (NetMarkersData1_2.MP - NetMarkersData1_1.MP) *
				 (NetMarker->Value - NetMarkersKey1_1.Value)/(NetMarkersKey1_2.Value - NetMarkersKey1_1.Value);
		} 
		goto Exit;
	}
	if (NetMarkersKey1_2.Path == Path)
	{
		NetMarkersKey1_3 = NetMarkersKey1_2;
		if (BT_FIND (hBTNetMarkers1,(LPSTR)&NetMarkersKey1_3,BT_FIRST,BT_GT,(LPSTR)&NetMarkersData1_3))
			goto Next;
		if (NetMarkersKey1_3.Path != Path)
			goto Next;
		LinkID3 = NetMarkersData1_3.MP/1000000;
		if (LinkID2 != LinkID3)
			goto Next;  
		if (NetMarkersKey1_3.Value == NetMarkersKey1_2.Value)
			goto Next;
		MP = NetMarkersData1_2.MP - 
		     (NetMarkersKey1_2.Value - NetMarker->Value) *
			 (NetMarkersData1_3.MP - NetMarkersData1_2.MP)/(NetMarkersKey1_3.Value - NetMarkersKey1_2.Value);
		if (!MPInNet (Path,MP))
		{
			MP = -1;
			goto Next; 
		} 
		if (NetMarkersData1_2.MP > NetMarkersData1_3.MP)  
			*MPDir = -1;  
		
		goto Exit;
	}
Next:
	if (NetMarkersKey1_1.Path == Path)
	{
		if (BT_FIND (hBTNetMarkers1,(LPSTR)&NetMarkersKey1_1,BT_FIRST,BT_EQ,(LPSTR)&NetMarkersData1_1))
			goto Exit;
		if (BT_FIND (hBTNetMarkers1,(LPSTR)&NetMarkersKey1_3,BT_PRIOR,BT_ANY,(LPSTR)&NetMarkersData1_3))
			goto Exit;
		if (NetMarkersKey1_3.Path != Path)
			goto Exit;
		LinkID3 = NetMarkersData1_3.MP/1000000;
		if (LinkID1 != LinkID3)
			goto Exit; 
		if (NetMarkersKey1_1.Value == NetMarkersKey1_3.Value)
			goto Exit; 
		MP = NetMarkersData1_1.MP + 
		     (NetMarker->Value - NetMarkersKey1_1.Value) *
			 (NetMarkersData1_1.MP - NetMarkersData1_3.MP)/(NetMarkersKey1_1.Value - NetMarkersKey1_3.Value); 
		if (!MPInNet (Path,MP))
			MP = -1;
		else if (NetMarkersData1_1.MP < NetMarkersData1_3.MP)  
			*MPDir = -1;  
	} 
Exit:
	CloseNetMarkers (Opened);
	return MP;
}

BOOL GetMilePointFromMP (long Path,int MarkerID,double MP, LPMARKERVAL NetMarker, LPSHORT pNetDirection)
{
	NETMARKERSKEY2	NetMarkersKey2_1, NetMarkersKey2_2, NetMarkersKey2_3;
	MARKERVAL		NetMarkersData2_1, NetMarkersData2_2, NetMarkersData2_3;
	BOOL			Opened, rtn=FALSE;
	double		MPinc, MPValue;  
	int			st, st2, Dir, pos; 
	long		LinkID1=-1, LinkID2=-1, LinkID3, Link;
	
	Link = MP/1000000;
	OpenNetMarkers (NetworkID,FALSE,&Opened);  
	NetMarkersKey2_2.MarkerID=MarkerID;
	NetMarkersKey2_2.Path = Path; 
	NetMarkersKey2_2.MP = MP; 
	*pNetDirection = 1;				
	if (BT_FIND (hBTNetMarkers2,(LPSTR)&NetMarkersKey2_2,BT_FIRST,BT_GE,(LPSTR)&NetMarkersData2_2))
		pos = BT_LAST;
	else
	{
		LinkID2 = NetMarkersKey2_2.MP/1000000; 
		pos = BT_PRIOR; 
	}
	if (!BT_FIND (hBTNetMarkers2,(LPSTR)&NetMarkersKey2_1,pos,BT_ANY,(LPSTR)&NetMarkersData2_1))
		LinkID1 = NetMarkersKey2_1.MP/1000000; 
	if (NetMarkersKey2_1.Path == NetMarkersKey2_2.Path && LinkID1 == Link && LinkID2 == Link)
	{   
		if (NetMarkersKey2_1.Path == Path)
		{  
			*NetMarker = NetMarkersData2_2; 
			NetMarker->Value = NetMarkersData2_1.Value +
				 			   ((MP - NetMarkersKey2_1.MP) / (NetMarkersKey2_2.MP - NetMarkersKey2_1.MP)) *
				 			   (NetMarkersData2_2.Value - NetMarkersData2_1.Value); 
			if (NetMarker->Value < NetMarkersData2_1.Value)
				*pNetDirection = -1;
			rtn = TRUE;
		}
		goto Exit;
	}
	if (NetMarkersKey2_2.Path == Path && LinkID2 == Link)
	{	
		NetMarkersKey2_3 = NetMarkersKey2_2;
		if (BT_FIND (hBTNetMarkers2,(LPSTR)&NetMarkersKey2_3,BT_FIRST,BT_GT,(LPSTR)&NetMarkersData2_3))	
			goto Exit; 
		LinkID3 = NetMarkersKey2_3.MP/1000000; 
		if (NetMarkersKey2_3.Path == Path && LinkID3 == Link)
		{
			*NetMarker = NetMarkersData2_2; 
			NetMarker->Value = NetMarkersData2_2.Value -
				 			   ((NetMarkersKey2_2.MP-MP) * 
				 			    (NetMarkersData2_3.Value - NetMarkersData2_2.Value)/
				 			    (NetMarkersKey2_3.MP - NetMarkersKey2_2.MP));
			if (NetMarkersData2_3.Value < NetMarkersData2_2.Value)
				*pNetDirection = -1;
			rtn = TRUE;
		}
		goto Exit;
	}
	if (NetMarkersKey2_1.Path == Path && LinkID1 == Link)
	{	
		if (BT_FIND (hBTNetMarkers2,(LPSTR)&NetMarkersKey2_1,BT_FIRST,BT_EQ,(LPSTR)&NetMarkersData2_1))	
			goto Exit; 
		if (BT_FIND (hBTNetMarkers2,(LPSTR)&NetMarkersKey2_3,BT_PRIOR,BT_ANY,(LPSTR)&NetMarkersData2_3))	
			goto Exit;
		LinkID3 = NetMarkersKey2_3.MP/1000000; 
		if (NetMarkersKey2_3.Path == Path && LinkID3 == Link)
		{
			*NetMarker = NetMarkersData2_1; 
			NetMarker->Value = NetMarkersData2_1.Value +
				 			   ((MP-NetMarkersKey2_1.MP) * 
				 			    (NetMarkersData2_1.Value - NetMarkersData2_3.Value)/
				 			    (NetMarkersKey2_1.MP - NetMarkersKey2_3.MP));
//			if (NetMarker->Value < NetMarkersData2_1.Value)
			if (NetMarkersData2_3.Value > NetMarkersData2_1.Value)
				*pNetDirection = -1;
			rtn = TRUE;
		}
		goto Exit;
	}
Exit:
	CloseNetMarkers (Opened);
	return rtn;   
	
}

BOOL GetCoordFromMP (long Path,double MP,LPDPOINT DPoint,LPDOUBLE AZ, LPDOUBLE pPCT, LPDOUBLE pLength, LPLONG Refno)
{    
	NETREFSKEY		NetRefsKey_1, NetRefsKey_2;
	NETREFSDATA		NetRefsData_1, NetRefsData_2;  
	BOOL			Opened, rtn=FALSE;
	double			PCT;
	int				pos;    
	long			Link, Link1, Link2;
	NETLINKSKEY		NetLinksKey; 
	NETLINKSDATA	NetLinksData; 

	OpenNetLinkAndRef (NetworkID,FALSE,&Opened);   
	Link = MP/1000000;
	NetRefsKey_2.Path = Path;
	NetRefsKey_2.MP = MP;
	if (BT_FIND (hBTNetRefs,(LPSTR)&NetRefsKey_2,BT_FIRST,BT_GT,(LPSTR)&NetRefsData_2)) 
	{
		pos = BT_LAST;
		NetRefsKey_2.Path = -1;
	}
	else
		pos = BT_PRIOR;
	if (BT_FIND (hBTNetRefs,(LPSTR)&NetRefsKey_1,pos,BT_ANY,(LPSTR)&NetRefsData_1))
		goto Exit; 
	Link1 = NetRefsKey_1.MP/1000000;
	if (NetRefsKey_1.Path != Path || Link1 != Link)
		goto Exit;  
		
	NetLinksKey.Ref = NetRefsData_1.Ref;
	NetLinksKey.NetID = NetworkID;
	NetLinksKey.Path = Path;
	BT_FIND (hBTNetLinks,(LPSTR)&NetLinksKey,BT_FIRST,BT_EQ,(LPSTR)&NetLinksData); 
	if (NetLinksData.Length != 0)
		PCT = (MP - NetRefsKey_1.MP) / fabs (NetLinksData.Length);
	else
		PCT = 0;
	if (NetRefsData_1.Dir == 2)
		PCT = 1.0-PCT; 
	*pPCT = PCT;  
	*Refno = NetRefsData_1.Ref;
	GetSegCoorByPct (NetRefsData_1.Ref,PCT,DPoint,AZ,pLength,FALSE); 
	if (NetRefsData_1.Dir == 2)
		*AZ = LTWOPI (*AZ+PY); 
	rtn = TRUE; 
Exit:
    CloseNetLinkAndRef (Opened);
	return rtn;
}
void AddIntersect (DPOINT AtPoint,long AtRef,int NumRefs,LPLONG IntRefs,
				   LPDOUBLE pIntRefAZ,LPDOUBLE pIntRefPCT, LPDOUBLE pIntRefLength)
{   
	if (!hIntRef)
		return;
	IntRefKey.IntID = ++IntersectionID; 
	while (NumRefs--)
	{
		IntRefKey.Ref = *IntRefs++; 
		NetIntRefData.Coord = AtPoint;
		NetIntRefData.AZ = *pIntRefAZ++; 
		NetIntRefData.Length = *pIntRefLength++;
		if (*pIntRefPCT++ < 0.5)
			NetIntRefData.WhichEnd = 1;
		else 
		{
			NetIntRefData.WhichEnd = 2;
			NetIntRefData.AZ = LTWOPI (NetIntRefData.AZ + PY);
		}
		BT_PUT (hIntRef,(LPSTR)&IntRefKey,(LPSTR)&NetIntRefData);
	}
	return;
}

void AddChain (long Ref,LPLONG pJoinRef)
{   
	long	Dummy, AtRef, NumInChain1=0,NumInChain2=0, TotNum;
	HANDLE	hChain1,hChain2,hChain;
	LPLONG	pChain, pNum; 
	short	i,j,NumStreets=0;
	long	StreetNums[4]={0,0,0,0};
     
    if (!hBTChained)
    	return;    
    ChainID++;
    hChain1 = GSSiGlobAlloc(GAIDNO 533,GMEM_MOVEABLE,USHRT_MAX);
    hChain2 = GSSiGlobAlloc(GAIDNO 534,GMEM_MOVEABLE,USHRT_MAX);
    if (pJoinRef)
    {
    	hChain = hChain1;
    	AtRef = *pJoinRef;  
    	pNum = &NumInChain1;
    } 
    else
    	goto SecondLoop;
Top:
    pChain = (LPLONG)GlobalLock (hChain);
Next:
	if (!BT_FIND(hBTChained,(LPSTR)&AtRef,BT_FIRST,BT_EQ,(LPSTR)&Dummy)) 
		goto Exit;   
	if (BT_FIND(hBTAt,(LPSTR)&AtRef,BT_FIRST,BT_EQ,(LPSTR)&AtData)) 
		goto Exit;  
	for (i=0;i<4;i++)
	{
		if (AtData.StreetNums[i])
		{
			for (j=0;j<NumStreets;j++)
				if (AtData.StreetNums[i]==StreetNums[j])
					goto NextStreet;
			if (NumStreets < 4)
				StreetNums[NumStreets++] = AtData.StreetNums[i]; 
		}
NextStreet:;
	}
				
	BT_PUT (hBTChained,(LPSTR)&AtRef,(LPSTR)&ChainID); 
	*pChain++=AtRef;
	(*pNum)++;
	AtRef = AtData.LastRef;
	goto Next;
Exit:   
	GlobalUnlock (hChain);
	if (pNum == &NumInChain1)
	{
SecondLoop:
		AtRef = Ref;
		pNum = &NumInChain2;
		hChain = hChain2;
		goto Top;
	} 
    TotNum = NumInChain1 + NumInChain2;
	if (TotNum)
	{
	    BigWrite (FidChain,(HPSTR)&TotNum,4,-1);
	    BigWrite (FidChain,(HPSTR)StreetNums,16,-1);
	    pChain = (LPLONG)GlobalLock (hChain1);
	    pChain += (NumInChain1 - 1);   
	    while (NumInChain1--)
	    	BigWrite (FidChain,(HPSTR)pChain--,4,-1);  
	    GlobalUnlock (hChain1);
	    pChain = (LPLONG)GlobalLock (hChain2); 
	    BigWrite (FidChain,(HPSTR)pChain,(size_t)NumInChain2*4,-1);
	    GlobalUnlock (hChain2);
	}
	GSSiGlobUlFree (&hChain1);
	GSSiGlobUlFree (&hChain2);
	return;
}

BOOL GetNetLocFromPick (short item,LPLONG pPath, LPDOUBLE pMP, LPSHORT pDir)
{
	NETLINKSKEY		NetLinksKey;
	NETLINKSDATA	NetLinksData;  
	NETREFSKEY		NetRefsKey;
	NETREFSDATA		NetRefsData;  
	BOOL			Opened, rtn=FALSE;
	double			MPinc, MPValue, MP, PCT=PickList[item].PCT;  
	int				st;
	long	Path;
	
	OpenNetLinkAndRef (NetworkID,FALSE,&Opened);  
	NetLinksKey.Ref = PickList[item].Refno;
	NetLinksKey.NetID = NetworkID;  
	NetLinksKey.Path = 0;
	st = BT_FIND (hBTNetLinks,(LPSTR)&NetLinksKey,BT_FIRST,BT_GE,(LPSTR)&NetLinksData);
	if (!st && NetLinksKey.Ref == PickList[item].Refno)
	{   
		NetRefsKey.Path =  NetLinksKey.Path;
		*pPath = NetLinksKey.Path;
		NetRefsKey.MP = NetLinksData.MP; 
		st = BT_FIND (hBTNetRefs,(LPSTR)&NetRefsKey,BT_FIRST,BT_GE,(LPSTR)&NetRefsData);  
		*pDir = 1;
		if (NetRefsData.Dir == 2)
		{
			*pDir = -1;
			PCT = 1.0 - PCT; 
		}
		MPinc = PCT * PickList[item].Length;
		*pMP = NetLinksData.MP += MPinc;
		rtn = TRUE;  
	}
	CloseNetLinkAndRef (Opened);
	return rtn;
}   

BOOL SaveRouteNextFile (HANDLE hBTNext,LPSTR Name)
{
	NEXTDATA	NextData;       
	NEXTKEY		NextKey;   
	short		pos=BT_FIRST;
    BTVARDESC  *pVars;
    short       NumFields, Reclen, len;
    GWDHEADER GWDHead; 
    LPGWDHEADER lpGWDHead;
    HANDLE  hVars, hDB;
    short       ibeg,NumVars;
	HFILE FidData;
    OFSTRUCTGM    OFStruct;
    GWFLDINFO FldInfo;    
    char	PrimeIndex[128];
    LPSTR	lpDot;
    typedef struct {
			long	Ref, AtRef, StartRef,Sequence,WhenAssigned; 
			short	SymNum, Type;
			double	AtCost, NextCost;
    		} RNF;
    typedef	RNF	FAR	*LPRNF;
    LPRNF	pRNF;
    
    _fstrcpy (PrimeIndex,Name);
    lpDot = _fstrrchr (PrimeIndex,'.');
    if (!lpDot)
    	return FALSE;
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
    _fstrcpy (FldInfo.Name,"SEGMENT_ID");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 4;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"FROM_ID");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 4;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"START_ID");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 4;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"SEQUENCE");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 4;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"WhenAssigned");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 2;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"SymbolNumber");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 2;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"Type");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 8;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_REAL;
    _fstrcpy (FldInfo.Name,"PriorCost");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 8;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_REAL;
    _fstrcpy (FldInfo.Name,"ThisCost");
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
	if (!hDB)
		return (FALSE);
	CloseGWDatabase (hDB); 
	hDB = OpenGWDatabase (Name,BT_WRITE);
    lpGWDHead = (LPGWDHEADER)GlobalLock (hDB);
    pRNF = (LPRNF)&lpGWDHead->GWDData; 
	while (!BT_FIND (hBTNext,(LPSTR)&NextKey,pos,BT_ANY,(LPSTR)&NextData))
	{   
		pos = BT_NEXT;
		pRNF->Ref = NextKey.Refno;
		pRNF->AtRef = NextData.AtRef;
		pRNF->StartRef = NextData.StartRef;
		pRNF->Sequence = NextData.Sequence;
		pRNF->WhenAssigned = NextData.WhenAssigned;
		pRNF->AtCost = NextData.AtCost;
		pRNF->NextCost = NextData.NextCost;
		pRNF->SymNum = NextData.SymNum;
		pRNF->Type = NextData.Type;
	    GWDAddRecord (lpGWDHead,0,NULL);
	} 
	GlobalUnlock (hDB);
    CloseGWDatabase (hDB); 
	
	return TRUE;
}

BOOL SaveRouteAtFile (HANDLE hBTAt,LPSTR Name)
{
	ATDATA		AtData;       
	long		AtKey;   
	short		pos=BT_FIRST;
    BTVARDESC  *pVars;
    short       NumFields, Reclen, len;
    GWDHEADER GWDHead; 
    LPGWDHEADER lpGWDHead;
    HANDLE  hVars, hDB;
    short       ibeg,NumVars;
	HFILE FidData;
    OFSTRUCTGM    OFStruct;
    GWFLDINFO FldInfo;    
    char	PrimeIndex[128];
    LPSTR	lpDot;
    LPROUTEATFILE	pRAF;
    
    _fstrcpy (PrimeIndex,Name);
    lpDot = _fstrrchr (PrimeIndex,'.');  
    if (!lpDot)
    	return FALSE;
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
    _fstrcpy (FldInfo.Name,"SEGMENT_ID");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 4;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"FROM_ID");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 4;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"START_ID");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 4;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"SEQUENCE");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 2;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"SymbolNumber");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 2;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"Type");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 2;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"NumNext");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 4;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"NextRef");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 2;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"FromPCT");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 2;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"ToPCT");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 8;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_REAL;
    _fstrcpy (FldInfo.Name,"Cost");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 8;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_REAL;
    _fstrcpy (FldInfo.Name,"PriorCost");
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
	if (!hDB)
		return (FALSE);
	CloseGWDatabase (hDB); 
	hDB = OpenGWDatabase (Name,BT_WRITE);
    lpGWDHead = (LPGWDHEADER)GlobalLock (hDB);
    pRAF = (LPROUTEATFILE)&lpGWDHead->GWDData; 
	while (!BT_FIND (hBTAt,(LPSTR)&AtKey,pos,BT_ANY,(LPSTR)&AtData))
	{   
		pos = BT_NEXT;
		pRAF->Ref = AtKey;
		pRAF->LastRef = AtData.LastRef;
		pRAF->StartRef = AtData.StartRef;
		pRAF->Sequence = AtData.Sequence;
		pRAF->SymNum = AtData.SymNum;
		pRAF->Type = AtData.Type;
		pRAF->NumNext = AtData.NumNext;
		pRAF->NextRef = AtData.NextRef;
		pRAF->Cost = AtData.Cost;
		pRAF->LastCost = AtData.LastCost;
		pRAF->FromPCT = AtData.FromPCT;
		pRAF->ToPCT = AtData.ToPCT;
	    GWDAddRecord (lpGWDHead,0,NULL);
	} 
	GlobalUnlock (hDB);
    CloseGWDatabase (hDB); 
	
	return TRUE;
}

BOOL SaveIntersectFile (LPSTR Name)
{
	NETINTREFKEY	NetIntRefKey;
	NETINTREFDATA	NetIntRefData;   
	short		pos=BT_FIRST; 
	long		LastID;
    BTVARDESC  *pVars;
    short       NumFields, Reclen, len;
    GWDHEADER GWDHead; 
    LPGWDHEADER lpGWDHead;
    HANDLE  hVars, hDB;
    short       ibeg,NumVars=0, NumSegs=0;
	HFILE FidData;
    OFSTRUCTGM    OFStruct;
    GWFLDINFO FldInfo;    
    char	PrimeIndex[128];
    LPSTR	lpDot;  
    BOOL	Opened;
    typedef struct {
			long	IntID;
			short	NumSegs; 
			double	X, Y;
    		} INTF;
    typedef	INTF	FAR	*LPINTF;
    LPINTF	pINTF;
    
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
    _fstrcpy (FldInfo.Name,"INT_ID");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 2;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"NUMSEGS");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 8;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_REAL;
    _fstrcpy (FldInfo.Name,"X");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 8;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_REAL;
    _fstrcpy (FldInfo.Name,"Y");
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
	if (!hDB)
		return (FALSE);
	CloseGWDatabase (hDB); 
	hDB = OpenGWDatabase (Name,BT_WRITE);
    lpGWDHead = (LPGWDHEADER)GlobalLock (hDB);
    pINTF = (LPINTF)&lpGWDHead->GWDData; 
	OpenNetIntersect (NetworkID,FALSE,&Opened);
	LastID = -1;  
	while (!BT_FIND (hBTNetIntRef,(LPSTR)&NetIntRefKey,pos,BT_ANY,(LPSTR)&NetIntRefData))
	{   
		pos = BT_NEXT; 
		if (NetIntRefKey.IntID != LastID)
			NumSegs = 0;
		NumSegs++;     
		LastID = NetIntRefKey.IntID;
		pINTF->IntID = NetIntRefKey.IntID;
		pINTF->X = NetIntRefData.Coord.x;
		pINTF->Y = NetIntRefData.Coord.y;
		pINTF->NumSegs = NumSegs;
	    GWDAddRecord (lpGWDHead,0,NULL);
	} 
	GlobalUnlock (hDB);
    CloseGWDatabase (hDB); 
	CloseNetIntersect(Opened); 
	return TRUE;
}
BOOL LoadVideoIndex (LPSTR File)
{   
	FILE	*Fid; 
	char	str[256], route[8], travel, txt[128], mess[256], TName[66];
	LPSTR	lpStr, lpSpace, lpC; 
	int		n=0; 
	long	disk, startframe, endframe, SNumTravel;
	double	startmp, endmp;  
	BOOL	Opened;  
	NETVIDEOKEY1	NetVidKey1,NetVidDataTest;
	NETVIDEODATA1	NetVidData1,NetVidKeyTest;
	MARKERVAL	NetMarker;
	short	MPDir;
	char	TrueName[34]; 
	int		dir,ii; 
	double	MP;
	OFSTRUCTGM	OFStruct;
	HFILE	FidBadMP;
	BOOL	LastMPBad;
	double	BadBegin, BadEnd;
	
	if (MessageBox (GetFocus(),"Do you wish to reload the video index",NULL,MB_YESNO) != IDYES)
		return FALSE;
	Fid=fopen(File,"rt");
	if (!Fid)
		return FALSE;  
	FidBadMP = GSSiOpenFile ("badmp.txt",&OFStruct,OF_CREATE);
	OpenNetVideoIndex (NetworkID,TRUE,&Opened);
	fgetss(str,256,Fid);  
	fgetss(str,256,Fid);  
	lpStr=str;
	while (fgetss(str,256,Fid))
	{   
		lpSpace = _fstrchr (str,' ');
		*lpSpace = 0;
		lpC = lpSpace - 1; 
		travel = *lpC;
		*lpC = 0;
		_fstrcpy (route,lpStr);  
		_fstrcpy (TrueName,"ROUTE ");
		_fstrcat (TrueName,route);
		NetVidKey1.Path = AddStreetName (TrueName,0,"","","","");  
		sprintf (TName,"%s%c",TrueName,travel);
	    SNumTravel=GetStreetNumFromName (TName,1,BT_FIRST, 0);
		NetVidKey1.MarkerID = 1;
		switch (travel) 
		{
			case 'N':
				dir = 1;
				break;
			case 'E':
				dir = 2;
				break;
			case 'S':
				dir = 3;
				break;
			case 'W':
				dir = 4;
				break;
			default:
				dir = 0;
		}  
		dir = 1;
        NetVidKey1.Dir = dir;
        _fmemset (NetVidKey1.Prefix,' ',4);
		startmp = atof (&str[6])/100;
		endmp = atof (&str[12])/100; 
		_fmemset (NetMarker.Prefix,' ',4);
		NetMarker.Value = startmp;  
    	sprintf (mess,"%s: %.2f to %.2f",TrueName,startmp,endmp);
    	fputstring (mess,FidBadMP); 
    	LastMPBad = FALSE;
		while (NetMarker.Value < endmp)
		{   
	    	MP = GetMPFromMilePoint (NetVidKey1.Path,NetVidKey1.MarkerID,&NetMarker,&MPDir);
	    	if (MP < 0)
	    	{
		    	if (SNumTravel)
			    	MP = GetMPFromMilePoint (SNumTravel,NetVidKey1.MarkerID,&NetMarker,&MPDir);
			    if (MP < 0)
			    {   
			    	if (!LastMPBad)  
			    		BadBegin = NetMarker.Value;
			    	BadEnd = NetMarker.Value;
			    	LastMPBad = TRUE; 
			    }
			    else
			    	goto NotBad; 
			}
		    else
		    {
NotBad:   
		    	if (LastMPBad)
		    	{
			    	sprintf (mess,"\t%s from %.2f to %.2f",TrueName,BadBegin,BadEnd);
			    	fputstring (mess,FidBadMP); 
			    }
		    	LastMPBad=FALSE;
		    }
			NetMarker.Value += 0.1;
		}
    	if (LastMPBad)
    	{
	    	sprintf (mess,"\t%s from %.2f to %.2f",TrueName,BadBegin,BadEnd);
	    	fputstring (mess,FidBadMP); 
	    }
		
		disk = atol (&str[25]);
		startframe= atol (&str[28]);
		endframe = atol (&str[34]); 
		NetVidKey1.Value = startmp;    
		NetVidData1.Frame = startframe;
        NetVidKey1.View = 1; 
        NetVidKey1.BegOrEnd = 1;
		sprintf (txt,"CTDOT%2.2iA    10",disk);
		_fstrncpy (NetVidData1.DiskID,txt,16); 
		NetVidKeyTest = NetVidData1;
		if (!BT_FIND (hBTNetVideo2,(LPSTR)&NetVidKeyTest,BT_FIRST,BT_GE,(LPSTR)&NetVidDataTest))
		{
			if (NetVidDataTest.BegOrEnd == 2)
			{   
				char	OverlapName[34];
				
				GetTrueStreetName (NetVidDataTest.Path, OverlapName, 0,0); 
		    	sprintf (mess,"%s overlaps %s",TrueName,OverlapName);
		    	fputstring (mess,FidBadMP); 
			}
		}
		BT_PUT (hBTNetVideo1,(LPSTR)&NetVidKey1,(LPSTR)&NetVidData1);
		BT_PUT (hBTNetVideo2,(LPSTR)&NetVidData1,(LPSTR)&NetVidKey1);
		NetVidKey1.Value = endmp;    
		NetVidData1.Frame = endframe; 
        NetVidKey1.BegOrEnd = 2;
		if ((endframe - startframe +1) != IDNINT((endmp - startmp)*100 + 1))
		{ 
	    	sprintf (mess,"*** Bad index data %s: %ld frames - %f miles",TrueName,(endframe - startframe +1),(((endmp - startmp)*100 + 1)/100));
	    	fputstring (mess,FidBadMP); 
		}
		NetVidKeyTest = NetVidData1;
		if (!BT_FIND (hBTNetVideo2,(LPSTR)&NetVidKeyTest,BT_FIRST,BT_GE,(LPSTR)&NetVidDataTest))
		{
			if (NetVidDataTest.BegOrEnd == 2)
			{   
				char	OverlapName[34];
				
				GetTrueStreetName (NetVidDataTest.Path, OverlapName, 0,0); 
		    	sprintf (mess,"%s overlaps %s",TrueName,OverlapName);
		    	fputstring (mess,FidBadMP); 
			}
		}
		BT_PUT (hBTNetVideo1,(LPSTR)&NetVidKey1,(LPSTR)&NetVidData1);
		BT_PUT (hBTNetVideo2,(LPSTR)&NetVidData1,(LPSTR)&NetVidKey1);
		NetVidKey1.Value = startmp;    
		NetVidData1.Frame = startframe;
        NetVidKey1.BegOrEnd = 1;
        NetVidKey1.View = 9;
		sprintf (txt,"CTDOT%2.2iB    10",disk);
		_fstrncpy (NetVidData1.DiskID,txt,16);
		BT_PUT (hBTNetVideo1,(LPSTR)&NetVidKey1,(LPSTR)&NetVidData1);
		BT_PUT (hBTNetVideo2,(LPSTR)&NetVidData1,(LPSTR)&NetVidKey1);
		NetVidKey1.Value = endmp;    
		NetVidData1.Frame = endframe;
        NetVidKey1.BegOrEnd = 2;
		if ((endframe - startframe +1) != IDNINT((endmp - startmp)*100 + 1))
		{ 
	    	sprintf (mess,"*** Bad index data %s: %ld frames - %f miles",TrueName,(endframe - startframe +1),(((endmp - startmp)*100 + 1)/100));
	    	fputstring (mess,FidBadMP); 
		}
		BT_PUT (hBTNetVideo1,(LPSTR)&NetVidKey1,(LPSTR)&NetVidData1);
		BT_PUT (hBTNetVideo2,(LPSTR)&NetVidData1,(LPSTR)&NetVidKey1);
        
        if (_fstrnicmp (&str[52],"ONEWAY",6))
        {
			switch (dir) 
			{
				case 1:
					dir = 3;
					break;
				case 2:
					dir = 4;
					break;
				case 3:
					dir = 1;
					break;
				case 4:
					dir = 2;
					break;
				default:
					dir = 0;
			} 
			dir = 2;
	        NetVidKey1.Dir = dir;
	        _fmemset (NetVidKey1.Prefix,' ',4);
			startmp = atof (&str[40])/100;
			endmp = atof (&str[46])/100;  
			if (endmp > 999)
				endmp = endmp - 1000;
			disk = atol (&str[59]);
			startframe= atol (&str[62]);
			endframe = atol (&str[68]); 
			NetVidKey1.Value = startmp;    
			NetVidData1.Frame = startframe;
        	NetVidKey1.BegOrEnd = 1;
	        NetVidKey1.View = 1;
			sprintf (txt,"CTDOT%2.2iA    10",disk);
			_fstrncpy (NetVidData1.DiskID,txt,16);
			NetVidKeyTest = NetVidData1;
			if (!BT_FIND (hBTNetVideo2,(LPSTR)&NetVidKeyTest,BT_FIRST,BT_GE,(LPSTR)&NetVidDataTest))
			{
				if (NetVidDataTest.BegOrEnd == 2)
				{   
					char	OverlapName[34];
					
					GetTrueStreetName (NetVidDataTest.Path, OverlapName, 0,0); 
			    	sprintf (mess,"%s overlaps %s",TrueName,OverlapName);
			    	fputstring (mess,FidBadMP); 
				}
			}
			BT_PUT (hBTNetVideo1,(LPSTR)&NetVidKey1,(LPSTR)&NetVidData1);
			BT_PUT (hBTNetVideo2,(LPSTR)&NetVidData1,(LPSTR)&NetVidKey1);
			NetVidKey1.Value = endmp;    
			NetVidData1.Frame = endframe;
        	NetVidKey1.BegOrEnd = 2;
			NetVidKeyTest = NetVidData1;
			if (!BT_FIND (hBTNetVideo2,(LPSTR)&NetVidKeyTest,BT_FIRST,BT_GE,(LPSTR)&NetVidDataTest))
			{
				if (NetVidDataTest.BegOrEnd == 2)
				{   
					char	OverlapName[34];
					
					GetTrueStreetName (NetVidDataTest.Path, OverlapName, 0,0); 
			    	sprintf (mess,"%s overlaps %s",TrueName,OverlapName);
			    	fputstring (mess,FidBadMP); 
				}
			}
			BT_PUT (hBTNetVideo1,(LPSTR)&NetVidKey1,(LPSTR)&NetVidData1);
			BT_PUT (hBTNetVideo2,(LPSTR)&NetVidData1,(LPSTR)&NetVidKey1);
			NetVidKey1.Value = startmp;    
			NetVidData1.Frame = startframe;
        	NetVidKey1.BegOrEnd = 1;
	        NetVidKey1.View = 9;
			sprintf (txt,"CTDOT%2.2iB    10",disk);
			_fstrncpy (NetVidData1.DiskID,txt,16);
			BT_PUT (hBTNetVideo1,(LPSTR)&NetVidKey1,(LPSTR)&NetVidData1);
			BT_PUT (hBTNetVideo2,(LPSTR)&NetVidData1,(LPSTR)&NetVidKey1);
			NetVidKey1.Value = endmp;    
			NetVidData1.Frame = endframe;
        	NetVidKey1.BegOrEnd = 2;
			BT_PUT (hBTNetVideo1,(LPSTR)&NetVidKey1,(LPSTR)&NetVidData1);
			BT_PUT (hBTNetVideo2,(LPSTR)&NetVidData1,(LPSTR)&NetVidKey1);
		}

		n++; 
	}
	fclose(Fid);
	GSSiClose2 (&FidBadMP); 
	CloseNetVideoIndex (Opened);
	CloseStreetNameTable ();
	return TRUE;
}  

BOOL SameRoute (LPSTR Name1In,LPSTR Name2In)
{
	LPSTR	lpEnd;
	char	NSEW[5]={"NSEW"};
	char	Name1[34], Name2[34]; 
	
	_fstrcpy (Name1,Name1In);
	_fstrcpy (Name2,Name2In);
	lpEnd = LastChr (Name1);
	if (_fstrchr (NSEW,*lpEnd)) 
		*lpEnd = 0; 
	lpEnd = LastChr (Name2);
	if (_fstrchr (NSEW,*lpEnd)) 
		*lpEnd = 0; 
	if (!_fstricmp (Name1,Name2))
		return TRUE;
	else
		return FALSE;
}

BOOL FindNetSegWOEndMarker (double Tol)
{
	NETREFSKEY	NetRefsKey1, NetRefsKey2;
	NETREFSDATA	NetRefsData1, NetRefsData2;  
	NETLINKSKEY	NetLinksKey; 
	NETLINKSDATA	NetLinksData; 
	NETMARKERSKEY2	NetMarkersKey1,NetMarkersKey2;
	NETMARKERSDATA2 NetMarkersData1, NetMarkersData2;
	MARKERVAL	NetMarker;
	NETINTMPKEY NetIntMPKey; 
	BOOL			Opened1, Opened2, rtn=FALSE; 
	int	st, pos; 
	long	Link1, Link2, Link, Path; 
	HFILE	FidMissMark, FidEndPoint, FidDupName;
	OFSTRUCTGM	OFStruct;
	HCURSOR	hcurSave;    
	double	MP; 
	DPOINT	DPoint;
	double	AZ, PCT, Length;
	long	Refno;
	char	str[256];
	char	StreetName[34], Names[32][34]; 
	short	ii, NetDir, NumNames, i;
	char	NSEW[5]={"NSEW"}; 
	LPSTR	lpLast; 
	long	DupNames;
	long	LastRef, LastLink, LastPath;
	
	OpenNetLinkAndRef (NetworkID,FALSE,&Opened1);  
	if (!NetworkID || !hBTNetLinks)
		return FALSE;
	OpenNetMarkers (NetworkID,FALSE,&Opened2); 
	hcurSave = GSSiSetCursor(LoadCursor(NULL, IDC_WAIT)); 
	FidMissMark = GSSiOpenFile ("missmark.txt",&OFStruct,OF_CREATE); 
	FidEndPoint = GSSiOpenFile ("endpoint.txt",&OFStruct,OF_CREATE); 
	FidDupName = GSSiOpenFile ("dupnames.txt",&OFStruct,OF_CREATE); 
	NetRefsKey1.Path = 0;
	NetRefsKey1.MP = 0; 
Next:
	if (BT_FIND (hBTNetRefs,(LPSTR)&NetRefsKey1,BT_FIRST,BT_GT,(LPSTR)&NetRefsData1)) 
		goto Step2;
	if (NetRefsKey1.Path == 408)
		ii=1;
	NetRefsKey2 = NetRefsKey1; 
	Link1 = NetRefsKey1.MP/1000000; 
	NetRefsKey2.MP = (Link1+1) * 1000000;
	if (BT_FIND (hBTNetRefs,(LPSTR)&NetRefsKey2,BT_FIRST,BT_GT,(LPSTR)&NetRefsData2)) 
		BT_FIND (hBTNetRefs,(LPSTR)&NetRefsKey2,BT_LAST,BT_ANY,(LPSTR)&NetRefsData2);
	else 
		BT_FIND (hBTNetRefs,(LPSTR)&NetRefsKey2,BT_PRIOR,BT_ANY,(LPSTR)&NetRefsData2); 
	Path = NetRefsKey1.Path; 
	GetTrueStreetName (Path, StreetName, 0,0); 
	NetLinksKey.Ref = NetRefsData2.Ref;
	NetLinksKey.NetID = 1;
	NetLinksKey.Path = Path;
	BT_FIND (hBTNetLinks,(LPSTR)&NetLinksKey,BT_FIRST,BT_EQ,(LPSTR)&NetLinksData); 
    NetRefsKey2.MP += fabs (NetLinksData.Length);
	Link2 = NetRefsKey2.MP/1000000; 
	
	NetMarkersKey2.MarkerID=1;
	NetMarkersKey2.Path = Path; 
	NetMarkersKey2.MP = NetRefsKey1.MP; 
	if (BT_FIND (hBTNetMarkers2,(LPSTR)&NetMarkersKey2,BT_FIRST,BT_GT,(LPSTR)&NetMarkersData2))
		BT_FIND (hBTNetMarkers2,(LPSTR)&NetMarkersKey2,BT_LAST,BT_ANY,(LPSTR)&NetMarkersData2);
	st = BT_FIND (hBTNetMarkers2,(LPSTR)&NetMarkersKey1,BT_PRIOR,BT_ANY,(LPSTR)&NetMarkersData1);
	if ((NetMarkersKey1.Path != Path || st) && NetMarkersKey2.Path != Path)
		MP = -100000000;
	else if (NetMarkersKey1.Path != Path || st)
		MP = NetMarkersKey2.MP;
	else if (NetMarkersKey2.Path != Path)
		MP = NetMarkersKey1.MP; 
	else if (fabs (NetRefsKey1.MP - NetMarkersKey1.MP) < fabs (NetRefsKey1.MP - NetMarkersKey2.MP))
		MP = NetMarkersKey1.MP;
	else
		MP = NetMarkersKey2.MP;
    if (fabs (NetRefsKey1.MP - MP) > Tol)
	{   
    	if (!GetCoordFromMP (Path,NetRefsKey1.MP,&DPoint,&AZ, &PCT, &Length,&Refno))
    		ii=1;
		if (!GetMilePointFromMP (Path,1,NetRefsKey1.MP, &NetMarker,&NetDir))
		{
			sprintf (str,"%s %8.1f|%f %f",StreetName,NetRefsKey1.MP,DPoint.x,DPoint.y);
			fputstring (str,FidMissMark);
		}
		else if (NetMarker.Value < 0)
			ii=1;
	} 
	if (!AtNetIntersection (Path, NetRefsKey1.MP-Tol, NetRefsKey1.MP+Tol,FALSE))
	{
		GetCoordFromMP (Path,NetRefsKey1.MP,&DPoint,&AZ, &PCT, &Length,&Refno);
		sprintf (str,"%s|%f %f",StreetName,DPoint.x,DPoint.y);
		fputstring (str,FidEndPoint); 
	}

	NetMarkersKey2.MarkerID=1;
	NetMarkersKey2.Path = Path; 
	NetMarkersKey2.MP = NetRefsKey2.MP; 
	if (BT_FIND (hBTNetMarkers2,(LPSTR)&NetMarkersKey2,BT_FIRST,BT_GT,(LPSTR)&NetMarkersData2))
		BT_FIND (hBTNetMarkers2,(LPSTR)&NetMarkersKey2,BT_LAST,BT_ANY,(LPSTR)&NetMarkersData2);
	st = BT_FIND (hBTNetMarkers2,(LPSTR)&NetMarkersKey1,BT_PRIOR,BT_ANY,(LPSTR)&NetMarkersData1);
	if ((NetMarkersKey1.Path != Path || st) && NetMarkersKey2.Path != Path)
		MP = -100000000;
	else if (NetMarkersKey1.Path != Path || st)
		MP = NetMarkersKey2.MP;
	else if (NetMarkersKey2.Path != Path)
		MP = NetMarkersKey1.MP; 
	else if (fabs (NetRefsKey2.MP - NetMarkersKey1.MP) < fabs (NetRefsKey2.MP - NetMarkersKey2.MP))
		MP = NetMarkersKey1.MP;
	else
		MP = NetMarkersKey2.MP;
    if (fabs (NetRefsKey2.MP - MP) > Tol)
	{   
    	if (!GetCoordFromMP (Path,NetRefsKey2.MP,&DPoint,&AZ, &PCT, &Length,&Refno))
    		ii=1;
		if (!GetMilePointFromMP (Path,1,NetRefsKey2.MP, &NetMarker,&NetDir))
		{
			sprintf (str,"%s %8.1f|%f %f",StreetName,NetRefsKey2.MP,DPoint.x,DPoint.y);
			fputstring (str,FidMissMark); 
		}
		else if (NetMarker.Value < 0)
			ii=1;
	}
	if (!AtNetIntersection (Path, NetRefsKey2.MP-Tol, NetRefsKey2.MP+Tol,FALSE))
	{
		GetCoordFromMP (Path,NetRefsKey2.MP,&DPoint,&AZ, &PCT, &Length,&Refno);
		sprintf (str,"%s|%f %f",StreetName,DPoint.x,DPoint.y);
		fputstring (str,FidEndPoint); 
	}
	
	NetRefsKey1 = NetRefsKey2;
	goto Next;  
Step2:
	LastRef = LONG_MAX;
	DupNames = 0;
	pos = BT_FIRST; 
 	while (!BT_FIND (hBTNetLinks,(LPSTR)&NetLinksKey,pos,BT_ANY,(LPSTR)&NetLinksData))
 	{	 
 		pos = BT_NEXT;
 		if (NetLinksKey.Ref != LastRef)
 		{   
 			if (DupNames)
 			{   
 				GetTrueStreetName (DupNames, StreetName, 0,0); 
 				sprintf (str,"%ld %s: %ld",LastRef,StreetName,LastLink);
 				fputstring (str,FidDupName);
 			} 
 			NumNames = 0;
			DupNames=0;
 			LastRef = NetLinksKey.Ref;
 			LastPath = NetLinksKey.Path; 
 			LastLink = NetLinksData.MP/1000000;
 		}  
		GetTrueStreetName (NetLinksKey.Path,Names[NumNames],0,0);
		lpLast = LastChr (Names[NumNames]);
		if (_fstrchr (NSEW,*lpLast)) 
			*lpLast = 0; 
		i = NumNames;
		while (i--)
			if (!_fstricmp (Names[NumNames],Names[i]))
				DupNames=NetLinksKey.Path;
		NumNames++;
	}
	if (DupNames)
	{
		GetTrueStreetName (DupNames, StreetName, 0,0); 
		sprintf (str,"%ld %s: %ld",LastRef,StreetName,LastLink);
		fputstring (str,FidDupName);
	} 

Exit: 
    CloseNetLinkAndRef (Opened1);
	CloseNetMarkers (Opened2); 
	GSSiClose2 (&FidMissMark);   
	GSSiClose2 (&FidEndPoint);  
	GSSiClose2 (&FidDupName);
	GSSiSetCursor (hcurSave);   
	return rtn;
} 

BOOL GetRouteAndMP (LPSTR *pRandMP,LPSTR Route,LPDOUBLE pMPVal)
{
	char	ThisRandMP[128];
	LPSTR	pComma, pColon;

Next:	
	if (!*pRandMP)
		return FALSE;
	pComma = _fstrchr (*pRandMP,',');
	if (pComma)
	{
		*pComma++ = 0;
		_fstrcpy (ThisRandMP,*pRandMP); 
		*pRandMP = pComma;  
	}
	else  
	{
		_fstrcpy (ThisRandMP,*pRandMP); 
		*pRandMP = 0;
	}
	pColon = _fstrchr (ThisRandMP,':');
	if (!pColon)
		pColon = ThisRandMP;
	else
		*pColon++ = 0;
	_fstrcpy (Route,ThisRandMP);
	*pMPVal = atof (pColon);   
	if (*pMPVal < -99)
		goto Next;		
	return TRUE;
} 

BOOL DumpIntToTXT (void)  
{   short	pos=BT_FIRST, NumSegs;
	NETINTREFKEY	NetIntRefKey;
	NETINTREFDATA	NetIntRefData,NetIntRefDataAt;
	long	NetIntAT=LONG_MIN, TotNum, CurLoc=0;
	BOOL	Opened=FALSE;
	HFILE	FidOut;
	OFSTRUCTGM	OFStruct;
	char	str[256];
	HCURSOR	hcurSave; 
    
	hcurSave = GSSiSetCursor(LoadCursor(NULL, IDC_WAIT)); 
   	SetViewport(*pCommandViewport);
	if (!OpenNetIntersect (NetworkID,FALSE,&Opened))
	{
		MessageBox (GetFocus(),"Unable to open intersection table",NULL,MB_ICONEXCLAMATION);
		return FALSE;
	}
	FidOut = GSSiOpenFile ("intdump.txt",&OFStruct,OF_CREATE);
NextInt:  
	NumSegs = 0; 
	fputstring ("IntersectionID,X,Y,NumSegments",FidOut);
	CreateStatusWind (hWndMain,1,"Dumping Intersections to Text File");
    TotNum = BT_NUM_IN_INDEX (hBTNetIntRef);
	while (ContinueProcessing && !BT_FIND (hBTNetIntRef,(LPSTR)&NetIntRefKey,pos,BT_ANY,(LPSTR)&NetIntRefData))
	{   
		pos = BT_NEXT;
		if (NetIntAT == NetIntRefKey.IntID)
			NumSegs++;
		else
		{   
			if (NumSegs)
			{
				sprintf (str,"%ld,%f,%f,%i",NetIntAT,NetIntRefDataAt.Coord.x,NetIntRefDataAt.Coord.y,NumSegs);
				fputstring (str,FidOut);
			}
			NetIntAT = NetIntRefKey.IntID;    
			NetIntRefDataAt = NetIntRefData;
			NumSegs = 1;
		}
		StatusWindowUpdate (NULL,NULL, TotNum, ++CurLoc);
	}
	if (NetIntAT > LONG_MIN)
	{
		sprintf (str,"%ld,%f,%f,%i",NetIntAT,NetIntRefDataAt.Coord.x,NetIntRefDataAt.Coord.y,NumSegs);
		fputstring (str,FidOut);
	}
	GSSiClose2 (&FidOut);  
	CloseNetIntersect (Opened);
	GSSiSetCursor (hcurSave); 
	DestroyStatusWindow (0);
	sprintf (str,"Intersections have been dumped to %s",OFStruct.szPathName);  
	MessageBox (GetFocus(),str,"",MB_OK);
	return TRUE;
} 

STREETMP GetIntPathMP (long IntID,long Path)
{
	NETINTREFKEY	NetIntRefKey;
	NETINTREFDATA	NetIntRefData;
	NETLINKSKEY		NetLinksKey; 
	NETLINKSDATA	NetLinksData;  
	STREETMP MP;
	short	pos=BT_FIRST,cond=BT_GE; 
	MP.Refno = LONG_MAX;
	NetIntRefKey.IntID = IntID;
	NetIntRefKey.Refno = LONG_MIN;     
	while (!BT_FIND (hBTNetIntRef,(LPSTR)&NetIntRefKey,pos,cond,(LPSTR)&NetIntRefData))
	{   
		pos = BT_NEXT;
		cond = BT_ANY;
		if (NetIntRefKey.IntID != IntID)
			break;
		NetLinksKey.Ref = NetIntRefKey.Refno;
		NetLinksKey.NetID = NetworkID;
		NetLinksKey.Path = Path;
		if (!BT_FIND (hBTNetLinks,(LPSTR)&NetLinksKey,BT_FIRST,BT_EQ,(LPSTR)&NetLinksData))
		{   
			if (NetIntRefKey.Refno < MP.Refno)
			{
				MP.Refno = NetIntRefKey.Refno;
				if (NetIntRefData.WhichEnd == 2)
					MP.PCT = 1;
				else
					MP.PCT = 0;
			}
		}
	} 
	return MP;
}

GetNextRefInPath (long Path,LPLONG pAtRef,LPSHORT pWhichEnd,long IntNo)
{
	NETINTREFKEY	NetIntRefKey;
	NETINTREFDATA	NetIntRefData;  
	NETLINKSKEY		NetLinksKey; 
	NETLINKSDATA	NetLinksData;  
	short	pos=BT_FIRST, cond=BT_GT;

	NetIntRefKey.IntID = IntNo;
	NetIntRefKey.Refno = LONG_MIN;
	while (!BT_FIND (hBTNetIntRef,(LPSTR)&NetIntRefKey,pos,cond,(LPSTR)&NetIntRefData))
	{
		pos=BT_NEXT;
		cond=BT_ANY;
		if (NetIntRefKey.IntID != IntNo)
			break; 
		if (NetIntRefKey.Refno != *pAtRef)
		{
			NetLinksKey.Ref = NetIntRefKey.Refno;
			NetLinksKey.NetID = NetworkID;   
			NetLinksKey.Path = Path;
			if (!BT_FIND (hBTNetLinks,(LPSTR)&NetLinksKey,BT_FIRST,BT_EQ,(LPSTR)&NetLinksData))
			{
				*pAtRef = NetLinksKey.Ref;    
				*pWhichEnd = NetIntRefData.WhichEnd;
				return TRUE;
			} 
		}
	}
	return FALSE;
}

BOOL AddNameToSegmentInt (int SegID,int StreetNum,BOOL Remove)
{
	BOOL	rtn=FALSE;

	NETREFINTKEY	NetRefIntKey;
	NETREFINTDATA	NetRefIntData;  
	NETINTREFKEY	NetIntRefKey;
	NETINTREFDATA	NetIntRefData;
	NETINTPATHSKEY	NetIntPathsKey;
	INTPATHSDATA	IPD;
	short	pos=BT_FIRST, cond=BT_GT;  
	BOOL	Opened1=FALSE, Opened2=FALSE,OpenedSeg=FALSE;
	int		NumSegs;
	int		Segids[64], i,j,k;
	SEGDATAGM	OnSegData, SegData;

    if (!OpenStreetSegmentTable (FALSE,&OpenedSeg))
    	return FALSE;
	if (!GetSegDataGM (SegID,&OnSegData))
		goto Exit;
	OpenNetLinkAndRef (NetworkID,FALSE,&Opened1); 
	OpenNetIntersect (NetworkID,TRUE,&Opened2);
	if (Remove)
	{
		goto Exit;
	}
	NetRefIntKey.Refno = SegID;
	NetRefIntKey.IntID = LONG_MIN;
	while (!BT_FIND (hBTNetRefInt,(LPSTR)&NetRefIntKey,pos,cond,(LPSTR)&NetRefIntData) && NetRefIntKey.Refno == SegID)
	{   
		short	pos2=BT_FIRST, cond2=BT_GT;
		
		pos = BT_NEXT;
		cond = BT_ANY;
		NetIntRefKey.IntID = NetRefIntKey.IntID;
		NetIntRefKey.Refno = LONG_MIN;     
		NumSegs = 0;
		while (!BT_FIND (hBTNetIntRef,(LPSTR)&NetIntRefKey,pos2,cond2,(LPSTR)&NetIntRefData) && NetIntRefKey.IntID == NetRefIntKey.IntID)
		{   
			pos2 = BT_NEXT;
			cond2 = BT_ANY;
			if (NetIntRefKey.Refno != SegID)
				Segids[NumSegs++] = NetIntRefKey.Refno;
		}
		for (i=0;i<NumSegs;i++)
		{
			if (GetSegDataGM (Segids[i],&SegData))
			{
				for (j=0;j<4;j++)
				{
					for (k=0;k<4;k++)
					{
						if (!SegData.StreetNum[j] || SegData.StreetNum[j] == OnSegData.StreetNum[k])
							goto NoAdd;
					}
					NetIntPathsKey.Path1 = min (SegData.StreetNum[j],StreetNum);
					NetIntPathsKey.Path2 = max (SegData.StreetNum[j],StreetNum);
					NetIntPathsKey.IntID = NetRefIntKey.IntID;
					memset (&IPD,0,sizeof(IPD));
					IPD.Point = NetRefIntData.Coord;
					BT_PUT (hBTNetIntPaths,(LPSTR)&NetIntPathsKey,(LPSTR)&IPD);
NoAdd:;
				}
			}
		}
	}
Exit:
	CloseNetIntersect (Opened2);				 
    CloseNetLinkAndRef (Opened1);                 
	CloseStreetSegmentTable(OpenedSeg); 
	return rtn;
}

BOOL GetNextIntOnRef (LPLONG pIntNo,long Refno,LPSHORT pWantIntAtEnd)
{
	NETREFINTKEY	NetRefIntKey;
	NETREFINTDATA	NetRefIntData;  
	short	pos=BT_FIRST, cond=BT_GT;  


	NetRefIntKey.Refno = Refno;
	NetRefIntKey.IntID = LONG_MIN;
	while (!BT_FIND (hBTNetRefInt,(LPSTR)&NetRefIntKey,pos,cond,(LPSTR)&NetRefIntData))
	{   
		pos = BT_NEXT;
		cond = BT_ANY;
		if (NetRefIntKey.Refno != Refno)
			break;
		if (NetRefIntKey.IntID != *pIntNo)
		{
			*pIntNo = NetRefIntKey.IntID;
			return TRUE;
		}
		else if (NetRefIntData.WhichEnd == 1)
			*pWantIntAtEnd = 2;
		else
			*pWantIntAtEnd = 1;
	}
	return FALSE;
}

BOOL GetNextRefBetweenInt (long AtRef,short AtRefFreeEnd,LPLONG pNextRef,LPSHORT pNextRefFreeEnd)
{   
	DPOINT	PickPoint;   
	BOOL	SaveUUPA = UseUserPickAp;
	
	if (!PickByRefno(AtRef,NULL,NULL,-1)) 
		return FALSE;   
	if (AtRefFreeEnd == 1)
		PickPoint = PickList[0].BeginPoint;
	else
		PickPoint = PickList[0].EndPoint;
	UseUserPickAp = FALSE;
	SystemPickAp = -P_TOL;	
	MaxPick=1;  
    SetPickAp(0);  
   	DoNotPickThisRefno=AtRef;  
    PickItems2 (CurView->hWnd,PickPoint,FALSE,TRUE,TRUE); 
   	DoNotPickThisRefno=LONG_MAX;  
    UseUserPickAp = SaveUUPA;
    if (NumPicked)
    {
    	*pNextRef = PickList[0].Refno;   
    	if (PickList[0].PCT > 0.5)
    		*pNextRefFreeEnd = 1;
    	else
    		*pNextRefFreeEnd = 2;
    	return TRUE;
    }
	return FALSE;
} 

BOOL InSegLoop (long AtRef,long nSegs,LPSTREETSEGMENT pSegs) 
{ 
	USHORT	i;
	
	for (i=0;i<nSegs;i++) 
	{
		if (pSegs[i].Refno == AtRef)
			return TRUE;
	}
	return FALSE;
}

long GetStreetSegsBetweenMPs (HANDLE hDBDestSegs,long OnStreet,STREETMP FromMP,STREETMP ToMP,LPDPOINT FromPoint,LPDPOINT ToPoint,LPSTR Value)
{
	long	nSegs,i;
	BOOL	First=TRUE; 
	NETREFINTKEY	NetRefIntKey;
	NETREFINTDATA	NetRefIntData;  
	HANDLE	hSegs=GSSiGlobAlloc(GAIDNO 0,GMEM_MOVEABLE,USHRT_MAX); 
	LPSTREETSEGMENT	pSegs= (LPSTREETSEGMENT)GlobalLock (hSegs);
    LPGWDHEADER lpGWDHead; 
	BOOL	Opened=FALSE, Opened2=FALSE;
	float	Zero=0,One=1;   
	long	LastRef, NextInt=LONG_MIN, AtRef=FromMP.Refno, SecondInt,NextRef;
	float	ToPCT=0, SecondPCT, PCTVals[3]={0,0,1},PCTVals2[3]={0,1,0};  
	double	MinDist=DBL_MAX, d;  
	short	pos=BT_FIRST, cond=BT_GT, NextEnd, AtRefFreeEnd, NextRefFreeEnd,SecondIntFreeEnd;
	
	if (!OpenNetIntersect (NetworkID,FALSE,&Opened))
		goto Exit;
	if (!OpenNetLinkAndRef (NetworkID,FALSE,&Opened2))
		goto Exit; 
	NetRefIntKey.Refno = AtRef;
	NetRefIntKey.IntID = LONG_MIN;
	while (!BT_FIND (hBTNetRefInt,(LPSTR)&NetRefIntKey,pos,cond,(LPSTR)&NetRefIntData))
	{   
		pos = BT_NEXT;
		cond = BT_ANY;
		if (NetRefIntKey.Refno != AtRef)
			break;
		d = ldistp (NetRefIntData.Coord,*ToPoint);
		if (d < MinDist)
		{   
			SecondInt = NextInt;
			SecondPCT = ToPCT;
			MinDist = d;  
			NextInt = NetRefIntKey.IntID; 
			ToPCT = PCTVals[NetRefIntData.WhichEnd];    
			SecondIntFreeEnd = 1;
			if (NetRefIntData.WhichEnd == 1)
				SecondIntFreeEnd = 2;
		}
		else
		{   
			SecondInt = NetRefIntKey.IntID;
			SecondPCT = PCTVals[NetRefIntData.WhichEnd];
		}
	} 
	if (NextInt == LONG_MIN)
		goto Exit;
Top:
	nSegs = 0;
	if (fabs (ToPCT - FromMP.PCT) > P_TOL)
	{   
		pSegs[nSegs].FromPCT = FromMP.PCT;
		pSegs[nSegs++].Refno = AtRef;
		if (AtRef == ToMP.Refno)
			goto Exit; 
	} 
Next: 
	NextRef = LONG_MAX;
	if (!GetNextRefInPath (OnStreet,&AtRef,&NextEnd,NextInt))
	{
CheckNext:
		if (First)
		{
			First = FALSE;
			AtRef = FromMP.Refno;  
			AtRefFreeEnd = SecondIntFreeEnd;
			NextInt = SecondInt;
			if (SecondInt != LONG_MIN)
			{
				ToPCT = SecondPCT;
				goto Top;  
			}
			else 
				goto CheckFreeEnd;
		}
		nSegs = 0;
		goto Exit;
	}
	if (InSegLoop (AtRef,nSegs,pSegs))
		goto CheckNext;
	pSegs[nSegs].Refno = AtRef;
	pSegs[nSegs++].FromPCT = PCTVals[NextEnd];
	if (AtRef == ToMP.Refno)
		goto Exit;
	AtRefFreeEnd = 0;
	while (!GetNextIntOnRef (&NextInt,AtRef,&AtRefFreeEnd)) 
	{
CheckFreeEnd: 
		if (AtRefFreeEnd)
		{
			if (GetNextRefBetweenInt (AtRef,AtRefFreeEnd,&NextRef,&NextRefFreeEnd)) 
			{
				AtRef = NextRef;
				AtRefFreeEnd = NextRefFreeEnd; 
				if (InSegLoop (AtRef,nSegs,pSegs))
					goto CheckNext;
				pSegs[nSegs].Refno = AtRef;
				pSegs[nSegs++].FromPCT = PCTVals2[AtRefFreeEnd];
				if (AtRef == ToMP.Refno)
					goto Exit; 
			}
			else 
				goto CheckNext;
		}
		else
			goto CheckNext;
	}
	goto Next;
Exit:	
	CloseNetIntersect (Opened);
	CloseNetLinkAndRef (Opened2); 
	lpGWDHead = (LPGWDHEADER)GlobalLock (hDBDestSegs);  
	if (nSegs && fabs(pSegs[nSegs-1].FromPCT - ToMP.PCT) < P_TOL)
		nSegs--;
	for (i=0;i<nSegs;i++)
	{   
		SetFieldValFromCharAndName(lpGWDHead, "SegRefno", (LPSTR)&pSegs[i].Refno, TRUE, TRUE);
		SetFieldValFromCharAndName(lpGWDHead, "Value", Value, FALSE, TRUE);
		SetFieldValFromCharAndName(lpGWDHead, "PCTFrom", (LPSTR)&Zero, TRUE, TRUE);
		SetFieldValFromCharAndName(lpGWDHead, "PCTTo", (LPSTR)&One, TRUE, TRUE);
		GWDAddRecord (lpGWDHead,0,NULL);      
	}
	GlobalUnlock (hDBDestSegs); 
	GSSiGlobUlFree (&hSegs);
	return nSegs;
}

BOOL CreateIntMarkers (void)
{   int	st, st2, i,j,n, pos=BT_FIRST, NumSegs, NumSegsOut;
	DPOINT	MidPoint, FromPointW, ToPoint, IntPoint;
	NETINTREFKEY	NetIntRefKey;
	NETINTREFDATA	NetIntRefData;
	NETLINKSDATA	NetLinksData;  
	NETLINKSKEY		NetLinksKey; 
	MARKERVAL		NetMarker;
	LPSTR	lpSpace;
	long	Offset, NetIntAT=LONG_MIN, NumBadInt=1;
	BOOL	Opened1=FALSE, Opened2=FALSE;
	HFILE	FidIntMark,FidBadInt;
	OFSTRUCTGM	OFStruct;
	double	PCT, MPinc;
	char	TrueName[34], str[256];
	short	Dir, nBad;    
	char	SegName[32][34]; 
	char	SegNameOut[32][34], Name1[34], Name2[34];
	double	SegMarkVal[32], SegMarkValOut[32];
	HCURSOR	hcurSave; 
    
	if (MessageBox (GetFocus(),"Running this utility will remove all markers at intersections.\r\nDo you wish to continue?",
			"Verify",MB_YESNO) != IDYES) 
		return FALSE;
	hcurSave = GSSiSetCursor(LoadCursor(NULL, IDC_WAIT)); 
   	SetViewport(*pCommandViewport);
	OpenNetLinkAndRef (NetworkID,FALSE,&Opened1); 
	OpenNetIntersect (NetworkID,FALSE,&Opened2);
	FidIntMark = GSSiOpenFile ("intmark.txt",&OFStruct,OF_CREATE);
	FidBadInt = GSSiOpenFile ("badint.txt",&OFStruct,OF_CREATE);
NextInt:  
	NetIntRefKey.IntID = NetIntAT+1;
	NetIntRefKey.Refno = LONG_MIN;     
	NumSegs = 0;
	if (!BT_FIND (hBTNetIntRef,(LPSTR)&NetIntRefKey,BT_FIRST,BT_GE,(LPSTR)&NetIntRefData))
	{   
		st = 0;
		NetIntAT = NetIntRefKey.IntID;	
		IntPoint = NetIntRefData.Coord; 
		while (!st && NetIntRefKey.IntID == NetIntAT)
		{   
			if (!RefInNet (NetIntRefKey.Refno))
				goto NextRef; 
				
			NetLinksKey.Ref = NetIntRefKey.Refno;
			NetLinksKey.NetID = 0;
			st = BT_FIND (hBTNetLinks,(LPSTR)&NetLinksKey,BT_FIRST,BT_GE,(LPSTR)&NetLinksData); 
	 	 	while (!st && NetLinksKey.Ref == NetIntRefKey.Refno)
	 	 	{	  
                if (NetLinksData.Length < 0)
                	PCT = 1.0;
                else
                	PCT = 0;   
   				if (NetIntRefData.WhichEnd == 2)
                	PCT = 1.0 - PCT;	
				MPinc = PCT * fabs (NetLinksData.Length);
				NetLinksData.MP += MPinc;  
	    		GetTrueStreetName (NetLinksKey.Path,TrueName,0,0);
				if (!GetMilePointFromMP (NetLinksKey.Path,1,NetLinksData.MP, &NetMarker, &Dir)) 
					NetMarker.Value = -9999; 
				else if (NetMarker.Value < 0 && fabs(NetMarker.Value) < 0.02)
					NetMarker.Value = 0;
				_fstrcpy (SegName[NumSegs],TrueName); 
				SegMarkVal[NumSegs++] = NetMarker.Value;
		 	 	st = BT_FIND (hBTNetLinks,(LPSTR)&NetLinksKey,BT_NEXT,BT_ANY,(LPSTR)&NetLinksData); 
		 	 }
NextRef: 
			st = BT_FIND (hBTNetIntRef,(LPSTR)&NetIntRefKey,BT_NEXT,BT_ANY,(LPSTR)&NetIntRefData); 
		}       
		NumSegsOut = 0;
//skip ints with 3 or more segs for same route
		i = NumSegs;
		while (i--)
		{
			n=0;
			j = NumSegs;
			while (j--)
				if (i!=j && !_fstricmp (SegName[i],SegName[j]))
				{		
					n++; 
					if (n > 1)
						nBad = i;
				}
			if (n > 1)   
			{   	
				sprintf (str,"%s: %ld|%f %f",SegName[nBad],NetIntRefKey.IntID,IntPoint.x,IntPoint.y);
				fputstring (str,FidBadInt);   
				goto NextInt; 
			}
		}
			
		i = NumSegs;
		while (i--) 
			if (*SegName[i])
			{
				j = i;
				while (j--)
				{
					if (!_fstricmp (SegName[i],SegName[j]))
					{
						if (SegMarkVal[i] < -999)
							SegMarkVal[i] = SegMarkVal[j];
						else if (SegMarkVal[j] < -999)
							SegMarkVal[j] = SegMarkVal[i];
						else if (fabs (SegMarkVal[i] - SegMarkVal[j]) < 0.2)
						{
							SegMarkVal[i] = (SegMarkVal[i] + SegMarkVal[j])/2; 
							SegMarkVal[j] = SegMarkVal[i];
						}
						*SegName[j] = 0;
					}   
					else 
					{   
						LPSTR	lpE1, lpE2;
						char	NSEW[5]={"NSEW"};
						
						_fstrcpy (Name1,SegName[i]); 
						_fstrcpy (Name2,SegName[j]);
						lpE1 = LastChr (Name1);
						lpE2 = LastChr (Name2);
						if (*lpE1 && *lpE2)
						{   
							if (_fstrchr (NSEW,*lpE1)) 
								*lpE1 = 0;
							if (_fstrchr (NSEW,*lpE2)) 
								*lpE2 = 0;
							if (!_fstricmp (Name1,Name2))
							{
								if (SegMarkVal[i] < -999)
									SegMarkVal[i] = SegMarkVal[j];
								else if (SegMarkVal[j] < -999)
									SegMarkVal[j] = SegMarkVal[i];
								else if (fabs (SegMarkVal[i] - SegMarkVal[j]) < 0.2)
								{
									SegMarkVal[i] = (SegMarkVal[i] + SegMarkVal[j])/2; 
									SegMarkVal[j] = SegMarkVal[i];
								}
							}
						}
					}
				}
				SegMarkValOut[NumSegsOut] = SegMarkVal[i];
				_fstrcpy (SegNameOut[NumSegsOut++],SegName[i]);
			}
			i = NumSegsOut; 
			_fstrcpy (str,"\"");
			while (i--)
			{   
				lpSpace = _fstrchr (SegNameOut[i],' ');
				if (lpSpace)
				{
					lpSpace++;
					sprintf (_fstrchr(str,0),"%s:%.3f,",lpSpace,SegMarkValOut[i]); 
				}
			}
			lpSpace = LastChr (str);
			if (*lpSpace == ',')
				*lpSpace = 0;
			sprintf (_fstrchr(str,0),"\",%f,%f",IntPoint.x,IntPoint.y);
			fputstring (str,FidIntMark);   

	        NumPicked = 0;
			UseUserPickAp =FALSE;
			SystemPickAp = -MarkerTOL;	
			MaxPick=8;  
		    SetPickAp(0);  
		    PickItems2 (CurView->hWnd,IntPoint,FALSE,TRUE,TRUE); 
		    while (NumPicked--)
				DeletePickedItem (NumPicked,12,92);
		goto NextInt;
	}
	CloseNetLinkAndRef (Opened1); 
	CloseNetIntersect (Opened2);
    GSSiClose2 (&FidIntMark);
    GSSiClose2 (&FidBadInt);
	GSSiSetCursor (hcurSave);   
    return TRUE;
}	

/*BOOL ShowPossibleRoutes(BOOL Refresh)
{   int	st,i,PossibleContinueRoute;
	DPOINT	MidPoint, FromPointW, ToPoint;
	NETINTREFKEY	NetIntRefKey;
	NETINTREFDATA	NetIntRefData;
	NETLINKSKEY		NetLinksKey; 
	NETLINKSDATA	NetLinksData;  
	long	Offset, TurnFromRef;
	short	TurnFromEnd;
	BOOL	Opened1=FALSE, Opened2=FALSE, Opened3=FALSE, rtn=FALSE;  
	char	CurName[34], NextName[34]; 
	static	long	IntFromRef, TurnArrowInt=0;
    
    if (Refresh)
    {
    	if (!NumPossibleRoutes)
    		return FALSE;
    }
    else
    {
    	IntFromRef = CurNetRef;
    	ContinueRoute=-1;
    } 
	NumPossibleRoutes = 0; 
    if (!NetIntAT)
    	return FALSE;
    if (!Refresh && (HaveTurnArrows && NetIntAT == TurnArrowInt))
    	return TRUE;
	OpenNetLinkAndRef (NetworkID,FALSE,&Opened1); 
	OpenNetIntersect (NetworkID,FALSE,&Opened2); 
	OpenTurnTable (FALSE,&Opened3); 
	NetIntRefKey.IntID = NetIntAT;
	NetIntRefKey.Refno = IntFromRef;     
	if (BT_FIND (hBTNetIntRef,(LPSTR)&NetIntRefKey,BT_FIRST,BT_EQ,(LPSTR)&NetIntRefData))
		goto Exit;
	TurnFromRef = IntFromRef;
	TurnFromEnd = NetIntRefData.WhichEnd;
	NetIntRefKey.IntID = NetIntAT;
	NetIntRefKey.Refno = LONG_MIN;     
	st = BT_FIND (hBTNetIntRef,(LPSTR)&NetIntRefKey,BT_FIRST,BT_GE,(LPSTR)&NetIntRefData); 
	while (!st && NetIntRefKey.IntID == NetIntAT)
	{   
		if (!RefInNet (NetIntRefKey.Refno))
			goto NextRef;
		if (!NumPossibleRoutes)
			FromPointW = NetIntRefData.Coord;
		if (GetTurnCost (TurnFromRef,TurnFromEnd,NetIntRefKey.Refno,NetIntRefData.WhichEnd) > 1000000)
			goto NextRef;
		PossibleAz[NumPossibleRoutes] = NetIntRefData.AZ;
		PossibleTLID[NumPossibleRoutes] = NetIntRefKey.Refno;
		if (NetIntRefData.WhichEnd == 1)
			PossiblePCT[NumPossibleRoutes] = 0;            
		else
			PossiblePCT[NumPossibleRoutes] = 1;  
		PossibleLength[NumPossibleRoutes] = NetIntRefData.Length;
		NumPossibleRoutes++;
NextRef: 
		st = BT_FIND (hBTNetIntRef,(LPSTR)&NetIntRefKey,BT_NEXT,BT_ANY,(LPSTR)&NetIntRefData); 
	}
    	
	if (NumPossibleRoutes > 2)
	{
		GetTrueStreetName (CurPath, CurName, 0);
		PosRouteRect.left = INT_MAX;
		PosRouteRect.right = INT_MIN;
		PosRouteRect.top = INT_MAX;
		PosRouteRect.bottom = INT_MIN;
		PosFromPoint = BasePtToWinPt(&FromPointW);
		PossibleContinueRoute = -1;
		for (i=0;i<NumPossibleRoutes;i++)
		{   
			if (!Refresh && PossibleTLID[i] != IntFromRef)
			{  
				NetLinksKey.Ref = PossibleTLID[i];
				NetLinksKey.NetID = 0; 
				st = BT_FIND (hBTNetLinks,(LPSTR)&NetLinksKey,BT_FIRST,BT_GE,(LPSTR)&NetLinksData); 
		 	 	while (!st && NetLinksKey.Ref == PossibleTLID[i])
		 	 	{	  
		    		GetTrueStreetName (NetLinksKey.Path,NextName,0); 
		    		if (!_fstricmp (CurName,NextName))
		    		{
						ContinueRoute=i;  
						break;
					}
		    		else if (CanTurnOnto (NetDirection, CurName, NextName))
					{   
						LPSTR	lpE1, lpE2;
						char	NSEW[5]={"NSEW"}, Name1[34];
						
						_fstrcpy (Name1,CurName); 
						lpE1 = LastChr (Name1);
						lpE2 = LastChr (NextName);
						if (_fstrchr (NSEW,*lpE1)) 
							*lpE1 = 0;
						if (_fstrchr (NSEW,*lpE2)) 
								*lpE2 = 0;
						if (!_fstricmp (Name1,NextName))
							PossibleContinueRoute=i;
					}  
					st = BT_FIND (hBTNetLinks,(LPSTR)&NetLinksKey,BT_NEXT,BT_ANY,(LPSTR)&NetLinksData); 
				}
			}
	        ToPoint = dnewpt (FromPointW,PossibleAz[i],25*CurView->BaseUnitsPerPixel); 
	        PosToPoint[i] = BasePtToWinPt(&ToPoint);
			PosRouteRect.left = min(PosRouteRect.left,PosToPoint[i].x);
			PosRouteRect.right = max(PosRouteRect.right,PosToPoint[i].x);
			PosRouteRect.top = min(PosRouteRect.top,PosToPoint[i].y);
			PosRouteRect.bottom = max(PosRouteRect.bottom,PosToPoint[i].y);
		}
		if (ContinueRoute < 0)  
			ContinueRoute = PossibleContinueRoute;
		PosRouteRect.left -= 7;
		PosRouteRect.right += 7;
		PosRouteRect.top -= 7;
		PosRouteRect.bottom += 7; 
	    PosRouteSavedScreen = SaveScreen2 (CurView->hDC,PosRouteRect,CurView,NULL);

		for (i=0;i<NumPossibleRoutes;i++)
		{
			DrawTurnArrow (CurView->hDC,PosFromPoint,PosToPoint[i]);
		}
		HaveTurnArrows=TRUE; 
		TurnArrowInt = NetIntAT;
		rtn = TRUE; 
		if (ContinueRoute < 0 && !Refresh)
        	EnableWindow (GetDlgItem(VideoCntlWnd,IDC_DRIVE),FALSE);
	}
	else
	{
		NumPossibleRoutes = 0;
		NetIntAT = 0; 
		HaveTurnArrows = FALSE;
	} 
Exit:	  
	CloseNetLinkAndRef (Opened1); 
	CloseNetIntersect (Opened2);   
	CloseTurnTable (Opened3);
    return rtn;
}  

BOOL CanTurnOnto (short CurDir, LPSTR CurNameIn, LPSTR NewNameIn)
{
	LPSTR	lpEnd;
	char	NSEW[5]={"NSEW"}; 
	char	CurName[34], NewName[34], EndChar;
	short	NewDirection;

	if (!*NewNameIn)
		return FALSE;  
	_fstrcpy (NewName,NewNameIn);
	_fstrcpy (CurName,CurNameIn);
	lpEnd = LastChr (CurName);
	if (_fstrchr (NSEW,*lpEnd)) 
		*lpEnd = 0;
	lpEnd = LastChr (NewName);
	EndChar = *lpEnd;
	if (_fstrchr (NSEW,*lpEnd)) 
		*lpEnd = 0;
	if (_fstricmp (CurName,NewName))
		return TRUE;
	switch (EndChar)
	{
		case 'E':
			NewDirection = 1;
		break;
		case 'W':
			NewDirection = 2;
		break;
		case 'N':
			NewDirection = 1;
		break;
		case 'S':
			NewDirection = 2;
		break; 
		default:
			return TRUE;
	}
    if (NewDirection == CurDir)
    	return TRUE;
    else
    	return FALSE;
}

void DrawTurnArrow (HDC hDC,POINT FromPoint,POINT ToPoint)
{	HPEN	hWidePen, hOldPen;
	int		MaxWidth;
	RECT	Rect;
	int		i;
	double	AZ, DIST;

	hWidePen = CreatePen (PS_SOLID,5,RGB(0,255,0));
	hOldPen = SelectObject (hDC,hWidePen);
	Rect.left = min (FromPoint.x,ToPoint.x);
	Rect.top = min (FromPoint.y,ToPoint.y);
	Rect.right = max (FromPoint.x,ToPoint.x);
	Rect.bottom = max (FromPoint.y,ToPoint.y);
	MaxWidth = 5;
	MaxWidth+=3;
	Rect.left = max (Rect.left-MaxWidth, CurView->DrawRect.left);
	Rect.right = min (Rect.right+MaxWidth,CurView->DrawRect.right);
	Rect.top = max (Rect.top-MaxWidth,CurView->DrawRect.top);
	Rect.bottom = min (Rect.bottom+MaxWidth,CurView->DrawRect.bottom);

	DrawPointerLine (CurView->hDC,FromPoint,ToPoint,hWidePen,hWidePen,6,0);
	SelectObject (hDC,hOldPen);
	DeleteObject (hWidePen);

	return;

} 

BOOL ProcessTurnArrows (HWND hWnd, WORD Message, WORD wParam, LONG lParam)
{
 POINT	MousePoint;
 double	dist, mindist;
 short	minroute,i;
 
 if (!HaveTurnArrows)
 	return FALSE;
 switch (Message)
   {
    case WM_LBUTTONUP:
    	MousePoint.x = LOWORD(lParam);
	    MousePoint.y = HIWORD(lParam); 
	    mindist = DBL_MAX;
		for (i=0;i<NumPossibleRoutes;i++) 
		{
			dist = idist (MousePoint,PosToPoint[i]);
			if (dist < mindist)
			{
				mindist = dist;
				minroute = i;
			}
		} 
		if (mindist > 6)
			return FALSE;  
		GEOSPANAtNetwork (&CurPath,PossibleTLID[minroute], PossiblePCT[minroute], PossibleLength[minroute],0,TRUE);
        PostMessage(VideoCntlWnd, WM_COMMAND, IDC_DRIVE, 0L);
		return TRUE;
		break;

    default:
    	return (FALSE);
    }
    return (FALSE);
} */



void DumpIntersectionStreets (LPSTR Name,BOOL EliminateDuplicateStreets)
{   
	BOOL	OpenedInt, OpenedSeg;
	long	LastInt=LONG_MAX;
	short	pos=BT_FIRST;
	int		nStreets=0, i, j;
	OFSTRUCTGM	OFStruct;
	HFILE	Fid;
	long	StreetNums[16]; 
	char	StreetName[80];
	char	str[1024]="IntID,X,Y,NumStreets,Street1,Street2,Street3,Street4,Street5,Street6,Street7,Street8,Street9,Street10,Street11,Street12,Street13,Street14,Street15,Street16";
    SEGDATAGM	Segdata; 
	HCURSOR	hcurSave; 
	DPOINT	NetCoord;
    
	
	if (!OpenNetIntersect (NetworkID,FALSE,&OpenedInt))
		return;
	if (!OpenStreetSegmentTable (FALSE,&OpenedSeg))
		return;
	Fid = GSSiOpenFile (Name,&OFStruct,OF_CREATE);
	if (Fid == HFILE_ERROR)
		return;
	hcurSave = GSSiSetCursor(LoadCursor(NULL, IDC_WAIT)); 
	fputstring (str,Fid);
	pos = BT_FIRST;
	while (!BT_FIND (hBTNetIntRef,(LPSTR)&NetIntRefKey,pos,BT_ANY,(LPSTR)&NetIntRefData))
	{   
		pos = BT_NEXT;
		if (NetIntRefKey.IntID != LastInt)
		{   
			if (LastInt < LONG_MAX)
			{   
				sprintf (str,"%ld,%f,%f,%i",LastInt,NetCoord.x,NetCoord.y,nStreets);
				for (i=0;i<nStreets;i++)
				{
					if (GetTrueStreetName (StreetNums[i],StreetName, 0,0)) 
						sprintf (_fstrchr(str,0),",\"%s\"",StreetName);
				}
				for (i=nStreets;i<16;i++)
					_fstrcat (str,",\"\"");
				fputstring (str,Fid);
			}
			LastInt = NetIntRefKey.IntID;    
			NetCoord = NetIntRefData.Coord;
			NetCoord.x *= MFT;
			NetCoord.y *= MFT;
			nStreets = 0; 
		}
		if (GetSegDataGM (NetIntRefKey.Refno,&Segdata)) 
		{ 
			for (i=0;i<4;i++)
			{
				if (Segdata.StreetNum[i])
				{
					if (EliminateDuplicateStreets)
					for (j=0;j<nStreets;j++)
					{
						if (Segdata.StreetNum[i] == StreetNums[j])
							goto NextStreet;
					}  
					StreetNums[nStreets++] = Segdata.StreetNum[i];
		NextStreet:;
				}
				else
					break;
			}
		}
    }
	if (LastInt < LONG_MAX)
	{   
		sprintf (str,"%ld,%f,%f,%i",LastInt,NetCoord.x,NetCoord.y,nStreets);
		for (i=0;i<nStreets;i++)
		{
			if (GetTrueStreetName (StreetNums[i],StreetName, 0,0)) 
				sprintf (_fstrchr(str,0),",\"%s\"",StreetName);
		}
		for (i=nStreets;i<16;i++)
			_fstrcat (str,",\"\"");
		fputstring (str,Fid);
	}
	CloseNetIntersect (OpenedInt);
	CloseStreetSegmentTable (OpenedSeg); 
	GSSiClose2 (&Fid);
	GSSiSetCursor (hcurSave); 
	sprintf (str,"Intersections have been dumped to %s",OFStruct.szPathName);  
	MessageBox (GetFocus(),str,"",MB_OK);
	return;
}

BOOL AddFalseIntersection (void)
{
	long Path[2], Refno;
	DPOINT IntPoint; 
	short	pos, i=0,type[2];     
	HANDLE	hPoly[2]={0,0};
	HPDPOINT	pPolyPoints[2];
	HIGHLIGHTDATA	HighlightData;
	long	npnts[2]; 
	double	ExtendDist = 1000, D1, D2;
	BOOL	rtn, OpenSeg, OpenedInt;
	SEGDATAGM		Segdata;  
	NETINTPATHSKEY	NetIntPathsKey;
	long	Munics[8], FIPSCode, nMunics;
	
	if (BT_NUM_IN_INDEX (hHighlight) != 2)
		return FALSE;

	pos = BT_FIRST; 
   	while (i<2 && !BT_FIND (hHighlight,(LPSTR)&Path[i],pos,BT_ANY,(LPSTR)&HighlightData))  
   	{
   		pos=BT_NEXT;
   		PickList[0]=HighlightData.PD;
	    type[i] = PickList[0].Type;
		if (GetPolyPoints ((LPPICKDATAHEADER)&PickList[0],FALSE,&npnts[i],&hPoly[i], 0))
			i++;
    }
	if (i != 2)
	{
		while (i--)
			GSSiGlobFree (&hPoly[i]);
		return FALSE;
	} 
	pPolyPoints[0] = (HPDPOINT)GlobalLock (hPoly[0]);
	pPolyPoints[1] = (HPDPOINT)GlobalLock (hPoly[1]);
	if (type[0] == 2)  
		ExtendPoly (npnts[0],pPolyPoints[0],ExtendDist);
	if (type[1] == 2)  
		ExtendPoly (npnts[1],pPolyPoints[1],ExtendDist);
	rtn = IntersectPolys1 (type[0],type[1],npnts[0],pPolyPoints[0],NULL,npnts[1],pPolyPoints[1],0,NULL,&PickPointBase,&IntPoint,&D1,&D2,0);
	GSSiGlobUlFree (&hPoly[0]);
	GSSiGlobUlFree (&hPoly[1]); 
		
	pos = BT_FIRST; 
	i=0;
   	while (!BT_FIND (hHighlight,(LPSTR)&Refno,pos,BT_ANY,(LPSTR)&HighlightData))  
   	{
   		pos=BT_NEXT;
   		PickList[i]=HighlightData.PD;
	    RemoveFromHighlightList (PickList[i].Refno,2);
    	ShowPickedItem (hWndMain,i); 
	    RemoveFromHighlightList (PickList[i].Refno,0);
    	ShowPickedItem (hWndMain,i++); 
	}
	ClearHighlightList(FALSE);

	OpenStreetSegmentTable (FALSE,&OpenSeg);  
	OpenNetIntersect (NetworkID,TRUE,&OpenedInt);  
	nMunics = 0;
/*
			NetIntPathsKey.IntID = LastInt;
			if (BT_FIND (hBTIntPaths,(LPSTR)&NetIntPathsKey.Path1,BT_FIRST,BT_GT,(LPSTR)&IntPoint))
				goto LastPath;
			while (!BT_FIND (hBTIntPaths,(LPSTR)&NetIntPathsKey.Path2,BT_NEXT,BT_ANY,(LPSTR)&IntPoint))
			{	
				BT_PUT (hBTNetIntPaths,(LPSTR)&NetIntPathsKey,(LPSTR)&IntPoint);
				if (GetSegDataGM (NetIntRefKey.Refno,&Segdata))
			{   
        		FIPSCode = Segdata.COUNTYL * 100000 + Segdata.FMCDL;
				if (Segdata.COUNTYL && Segdata.FMCDL)
					AddIntMunic (FIPSCode,&nMunics,Munics,MunicCounts);
        		FIPSCode = Segdata.COUNTYR * 100000 + Segdata.FMCDR;
				if (Segdata.COUNTYR && Segdata.FMCDR)
					AddIntMunic (FIPSCode,&nMunics,Munics,MunicCounts); 
			} 

			if (GetSegDataGM (NetIntRefKey.Refno,&Segdata))
			{   
        		FIPSCode = Segdata.COUNTYL * 100000 + Segdata.FMCDL;
				if (Segdata.COUNTYL && Segdata.FMCDL)
					AddIntMunic (FIPSCode,&nMunics,Munics,MunicCounts);
        		FIPSCode = Segdata.COUNTYR * 100000 + Segdata.FMCDR;
				if (Segdata.COUNTYR && Segdata.FMCDR)
					AddIntMunic (FIPSCode,&nMunics,Munics,MunicCounts); 
			} 
*/
	CloseStreetSegmentTable (OpenSeg);
	CloseNetIntersect (OpenedInt);
	return TRUE;
}

int AddIntStreets (int n,int pIntStreets[2][16],int NewStreet1,int NewStreet2)
{
	int	i=NewStreet1;

	if (NewStreet1 == NewStreet2)
		return n;
	if (i > NewStreet2)
	{
		NewStreet1 = NewStreet2;
		NewStreet2 = i;
	}
	for (i = 0;i < n;i++)
		if (pIntStreets[0][i] == NewStreet1 && pIntStreets[1][i] == NewStreet2)
			return n;
	pIntStreets[0][n] = NewStreet1;
	pIntStreets[1][n] = NewStreet2;
	return n+1;
}

BOOL FalseIntFunctions (LPSTR Cmd,LPSTR Arg,LPSTR Out)
{
#define MAX_INTSTREETS	16
	BOOL	rtn=FALSE;
	short	pos = BT_FIRST,i;
	HIGHLIGHTDATA	HighlightData;
	long	Ref;
    SEGDATAGM	Segdata; 
	BOOL	Err, Opened;
	UINT	nStreets=0, istreet1, istreet2, i1, i2, NumIntStreets=0;
	int		npnts[MAX_INTSTREETS];
	HANDLE	hPoly[MAX_INTSTREETS];
	int		StreetNums[MAX_INTSTREETS][4];
	double	ExtendDist=1000;
	double	MaxDist = GetGlobalDVal2 ("[%IntersectionAverageDist]",25);
	int		IntStreets[2][MAX_INTSTREETS];
	LPSTR	pString, pSpace, pEnd;
	NETINTPATHSKEY	NetIntPathsKey;
	INTPATHSDATA	IPD;


	*Out = 0;
	if (!stricmp (Cmd,"STREETLIST"))
	{
		DPOINT FIPoint = atopt (Arg,&Err);

		if (BT_NUM_IN_INDEX (hHighlight) < 2)
			return FALSE;
		if (!OpenStreetSegmentTable (FALSE,&Opened))
			return FALSE;
		if (!Err)
		{
			while (!BT_FIND (hHighlight,(LPSTR)&Ref,pos,BT_ANY,(LPSTR)&HighlightData))
			{ 
				pos = BT_NEXT;
		   		PickList[0]=HighlightData.PD;
				if (SysTypeFromPickType (PickList[0].Type) == GF_POLYLINE)
				{
					if (GetSegDataGM (Ref,&Segdata)) 
					{ 
						memmove (StreetNums[nStreets],Segdata.StreetNum,sizeof(Segdata.StreetNum));
						if (GetPolyPoints ((LPPICKDATAHEADER)&PickList[0],FALSE,&npnts[nStreets],&hPoly[nStreets], 0))
						{
							HPDPOINT	pPolyPoints=GlobalLock (hPoly[nStreets]);

							ExtendPoly (npnts[nStreets],pPolyPoints,ExtendDist);
							GlobalUnlock (hPoly[nStreets]);
							nStreets++;
							if (nStreets == MAX_INTSTREETS)
								break;
						}
					}
				}
			}
			if (nStreets > 1)
			{
				for (istreet1 = 0; istreet1 < nStreets-1;istreet1++)
				{
					for (istreet2 = istreet1+1; istreet2 < nStreets;istreet2++)
					{
						HPDPOINT	pPolyPoints1 = (HPDPOINT)GlobalLock (hPoly[istreet1]);
						HPDPOINT	pPolyPoints2 = (HPDPOINT)GlobalLock (hPoly[istreet2]);
						DPOINT		IntPoint;
						double		D1,D2;
						BOOL		st;

						st = IntersectPolys1 (GF_LINE,GF_LINE,npnts[istreet1],pPolyPoints1,NULL,npnts[istreet2],pPolyPoints2,0,NULL,&FIPoint,&IntPoint,0,0,0);
						if (ldistp (IntPoint,FIPoint) < MaxDist)
						{
							for (i1=0;i1<4;i1++)
								for (i2=0;i2<4;i2++)
									if (StreetNums[istreet1][i1] && StreetNums[istreet2][i2])
									{
										NumIntStreets = AddIntStreets (NumIntStreets,IntStreets,StreetNums[istreet1][i1],StreetNums[istreet2][i2]);
									}
						}
						GlobalUnlock (hPoly[istreet1]);
						GlobalUnlock (hPoly[istreet2]);
					}
				}
			}
			for (istreet1 = 0; istreet1 < nStreets;istreet1++)
				GSSiGlobFree (&hPoly[istreet1]);
		}
		for (i=0;i<NumIntStreets;i++)
			sprintf (strrchr (Out,0),"%i %i,",IntStreets[0][i],IntStreets[1][i]);
		if (*LastChr (Out) == ',')
			*LastChr (Out) = 0;
		CloseStreetSegmentTable(Opened); 
	}
	else if (!stricmp (Cmd,"LOAD"))
	{
		if (!OpenNetIntersect (NetworkID,TRUE,&Opened))
			return FALSE;
		while (!BT_FIND (hHighlight,(LPSTR)&Ref,pos,BT_ANY,(LPSTR)&HighlightData))
		{ 
			pos = BT_NEXT;
		   	PickList[0]=HighlightData.PD;
			if (SysTypeFromPickType (PickList[0].Type) == GF_POINT)
			{
				ProcessPickedItem (0,FALSE);  
       			if (lGCmdString && hGCmdString)
       			{   
       				pEnd = GlobalLock (hGCmdString);
					do
					{
						pString = pEnd;
						if ((pEnd = strchr (pString,',')))
							*pEnd++ = 0;
						pSpace = strchr (pString,' ');
						*pSpace++ = 0;
						NetIntPathsKey.Path1 = atoi (pString);
						NetIntPathsKey.Path2 = atoi (pSpace);
						NetIntPathsKey.IntID = Ref;
						memset (&IPD,0,sizeof(IPD));
						IPD.Point = PickList[0].BeginPoint;
						BT_PUT (hBTNetIntPaths,(LPSTR)&NetIntPathsKey,(LPSTR)&IPD);
					}
					while (pEnd);
					GlobalUnlock (hGCmdString);
				}
			}
		}
		CloseNetIntersect (Opened);
	}
	return rtn;
}

BOOL AddIntsToNet (HWND hWnd, int Message)
{     
	FILE	*Fid;  
	char	str[1024], txt[128];
	DPOINT	MPPoint;
	BOOL	OpenedLR, OpenedInt;
	int		st,st2;
	NETLINKSKEY	NetLinksKey; 
	NETLINKSDATA	NetLinksData;
	NETREFSKEY	NetRefsKey;
	NETREFSDATA	NetRefsData; 
	NETMARKERSKEY1	NetMarkersKey1;
	NETMARKERSKEY2	NetMarkersKey2;
	NETMARKERSDATA1 NetMarkersData1;
	NETMARKERSDATA2	NetMarkersData2; 
	NETINTPATHSKEY	NetIntPathsKey;
	BTVARDESC		BTVar[3]; 
	double			MPinc, PCT, MPVal, AZ;  
	char			File[]="file20.txt";
	char			snam[34], TrueName[34]="",TempName[144];  
	int	iroute, i, NumChecked, Dummy, SaveMaxPick, pos;     
	LPSTR	lpSpace;    
	long	WantStreetNum;
	HCURSOR	hcurSave;   
	DPOINT	IntPoint; 
	long	LastInt; 
	HANDLE	hBTIntPaths;
	
	if (Message != GF_INIT)
		return FALSE; 
	hcurSave = GSSiSetCursor(LoadCursor(NULL, IDC_WAIT));
	SetViewport(*pCommandViewport);
	SetDisplayMode (CurView->hDC, GF_TEXTMODE);
    GSSiDeleteObject(&CurView->hRgn);
	CurView->hRgn = CreateVPRgn (FALSE,FALSE);
  	SelectClipRgn (CurView->hDC,CurView->hRgn);
  	GSSiDeleteObject(&CurView->hRgn);
	hIntRef= BT_OPEN ("intref.btr", 0, BT_READ, 0);   
    if (!hIntRef)
	{
		MessageBox( GetFocus(), File,"Unable to open intersection file", MB_OK);
		return FALSE;
	} 
								  
	OpenNetLinkAndRef (NetworkID,FALSE,&OpenedLR);
	OpenNetIntersect (NetworkID,TRUE,&OpenedInt);  
	BTVar[0].BT_VARTYP=BT_INTEGER;
	BTVar[0].BT_VARLEN=4;
	BTVar[0].BT_VAROFF=0;
    GSSiGetTempFileName (0,"gmi",0,(LPSTR)TempName); 
	BT_CREATE (TempName, sizeof(DPOINT), FALSE, 1, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
	hBTIntPaths = BT_OPEN (TempName, 0, BT_WRITE, 0); 
	LastInt = 0;
	pos = BT_FIRST;
	while (!BT_FIND (hIntRef,(LPSTR)&IntRefKey,pos,BT_ANY,(LPSTR)&NetIntRefData))
	{   
		pos = BT_NEXT; 
		if (IntRefKey.IntID != LastInt && LastInt)
		{ 
			long	StartInt=0; 
						
			NetIntPathsKey.Path1 = 0;
	NextPath:
			NetIntPathsKey.IntID = LastInt;
			if (BT_FIND (hBTIntPaths,(LPSTR)&NetIntPathsKey.Path1,BT_FIRST,BT_GT,(LPSTR)&IntPoint))
				goto LastPath;
			while (!BT_FIND (hBTIntPaths,(LPSTR)&NetIntPathsKey.Path2,BT_NEXT,BT_ANY,(LPSTR)&IntPoint))
			{	
				BT_PUT (hBTNetIntPaths,(LPSTR)&NetIntPathsKey,(LPSTR)&IntPoint);
			}
			goto NextPath;
	LastPath:
			BT_CLEAR (hBTIntPaths);						
		}
		LastInt = IntRefKey.IntID;
		BT_PUT (hBTNetIntRef,(LPSTR)&IntRefKey,(LPSTR)&NetIntRefData);
		NetLinksKey.Ref = IntRefKey.Ref;
		NetLinksKey.NetID = NetworkID;
		NetLinksKey.Path = 0;
		st = BT_FIND (hBTNetLinks,(LPSTR)&NetLinksKey,BT_FIRST,BT_GE,(LPSTR)&NetLinksData); 
		while (!st && NetLinksKey.Ref == IntRefKey.Ref)
		{   	
			BT_PUT (hBTIntPaths,(LPSTR)&NetLinksKey.Path,(LPSTR)&NetIntRefData.Coord);
			NetIntMPKey.Path = NetLinksKey.Path; 
			if (NetIntRefData.WhichEnd == 1 && NetLinksData.Length < 0)
				NetIntMPKey.MP = NetLinksData.MP - NetLinksData.Length;
			else if (NetIntRefData.WhichEnd == 2 && NetLinksData.Length > 0)
				NetIntMPKey.MP = NetLinksData.MP + NetLinksData.Length;
			else
				NetIntMPKey.MP = NetLinksData.MP; 
			BT_PUT (hBTNetIntMP,(LPSTR)&NetIntMPKey,(LPSTR)&IntRefKey.IntID);
			st = BT_FIND (hBTNetLinks,(LPSTR)&NetLinksKey,BT_NEXT,BT_ANY,(LPSTR)&NetLinksData); 
		}  
		DisplayMarkers = TRUE;
		DisplayMarker (NetIntRefData.Coord,3,NULL,0,0,0,TRUE,FALSE,NULL,NULL,0,0,0);
		DisplayMarkers = FALSE;           
NextInt:;          
	}
	GSSiSetCursor (hcurSave);
	BT_CLOSE (hIntRef);
	hIntRef = 0;
	CloseNetLinkAndRef (OpenedLR);
	CloseNetIntersect (OpenedInt);
	BT_CLOSEANDDELETE (&hBTIntPaths);
Exit: 
    PostMessage(hWnd, GF_CLOSE,0, 0L); 
	return TRUE;
}

static int GetNodeID(LPDPOINT point,LPPOINT pNodes, LPINT pNumNodes)
{
	POINT pt;
	int i = 0;

	pt.x = IDNINT(point->x);
	pt.y = IDNINT(point->y);

	for (i = 0; i < *pNumNodes; i++, pNodes++)
	{
		if (abs(pt.x - pNodes->x) < 2 && abs(pt.y - pNodes->y) < 2)
			return i+1;
	}
	*pNodes = pt;
	(*pNumNodes)++;
	return i+1;
}
BOOL LoadNetwork(LPSTR Type, LPSTR File, LPSTR Opts)
{
	BOOL rtn = FALSE;

	
	typedef struct
	{
		int	id;
		int length;
		int fromNode, toNode;
		short speed;
		short oneway; // 0-both,1 = fromto; 2= tofrom
	} NETSEGMENT;

	typedef NETSEGMENT	FAR* LPNETSEGMENT;

	if (!stricmp(Type, "HLTTOSLT"))
	{
		sqlite3* db = NULL;
		char cmd[1024];
		HIGHLIGHTDATA	HighlightData;
		int Refno;
		int pos = BT_FIRST;
		int totRecs = BT_NUM_IN_INDEX(hHighlight);
		int nLoaded = 0;
		if (totRecs > 1)
		{
			LPPOINT pNodes = malloc(totRecs * 2 * sizeof(POINT));
			LPNETSEGMENT pSegs = malloc(totRecs * sizeof(NETSEGMENT));
			int numNodes = 0;
			int numSegs = 0;
			CreateStatusWind(hWndMain, 1, "Writing output file");
			while (StatusWindowUpdate(NULL, NULL, totRecs, ++nLoaded) && !BT_FIND(hHighlight, (LPSTR)&Refno, pos, BT_ANY, (LPSTR)&HighlightData))
			{
				pos = BT_NEXT;
				if (HighlightData.PD.Type == 2)
				{
					HANDLE hPnts;
					long nPnts;

					if (GetPolyPoints((LPPICKDATAHEADER)&HighlightData.PD, FALSE, &nPnts, &hPnts, 0))
					{
						LPDPOINT pPoints = (HPDPOINT)GlobalLock(hPnts);
						LPSTR pSpeed;
						int speed = 0;
						int oneway = 0;
						pSegs[numSegs].fromNode = GetNodeID(pPoints,pNodes,&numNodes);
						pSegs[numSegs].toNode = GetNodeID(&pPoints[nPnts - 1], pNodes, &numNodes);
						pSegs[numSegs].id = Refno;
						pSegs[numSegs].length = IDNINT(GetPolyLengthDH(hPnts, nPnts));
						pSpeed = strrchr(HighlightData.PD.UDI, ':');
						if (pSpeed)
						{
							*pSpeed++ = 0;
							speed = atoi(pSpeed);
						}
						pSegs[numSegs].speed = speed;
						if (*HighlightData.PD.UDI == 'F')
							oneway = 1;
						else if (*HighlightData.PD.UDI == 'T')
							oneway = 2;
						else if (*HighlightData.PD.UDI == 'N')
							oneway = 3;
						pSegs[numSegs++].oneway = oneway;
						GSSiGlobUlFree (&hPnts);
					}
				}
			}
			int st = sqlite3_open(File, &db);
			if (st == SQLITE_OK)
			{
				strcpy(cmd, "DROP TABLE IF EXISTS SEGMENTS;CREATE TABLE SEGMENTS (REFNO INTEGER PRIMARY KEY,FROMNODE INT,TONODE INT,LENGTH INT,SPEED INT,ONEWAY INT);CREATE INDEX SEGMENTS_FROMNODE_INDEX ON SEGMENTS (FROMNODE);CREATE INDEX SEGMENTS_TONODE_INDEX ON SEGMENTS (TONODE); ");
				st = SQLOK(sqlite3_exec(db, "BEGIN", NULL, NULL, 0), db, "", 0);
				st = SQLOK(sqlite3_exec(db, cmd, NULL, NULL, 0), db, "", 0);
				for (int i = 0; i < numSegs;i++)
				{
					sprintf(cmd, "INSERT INTO SEGMENTS VALUES (%i,%i,%i,%i,%i,%i);", pSegs[i].id, pSegs[i].fromNode, pSegs[i].toNode, pSegs[i].length, pSegs[i].speed, pSegs[i].oneway);
					st = SQLOK(sqlite3_exec(db, cmd, NULL, NULL, 0), db, "", 0);
				}
				st = SQLOK(sqlite3_exec(db, "COMMIT", NULL, NULL, 0), db, "", 0);
				st = sqlite3_close(db);
			}

			free(pNodes);
			free(pSegs);
		}
		DestroyStatusWindow(0);

	}
	else if (!stricmp(Type, "TEST"))
	{
		int startSeg = atoi(Opts);
		WalkOutTest(File, startSeg);
	}
	return rtn;
}

static int AddSementToWalkout(sqlite3* db, int fromref,int atNode,int currentCost)
{
	int rtn = 0;
	int st;
	int cost;
	int toNode;
	sqlite3_stmt* statement;

	if (db)
	{
		char cmd[256];
		sprintf(cmd, "SELECT * FROM SEGMENTS WHERE REFNO = %i", fromref);

		SQLOK(sqlite3_prepare_v2GSSi(db, cmd, -1, &statement, 0), db, "table exists", 0);

		if (sqlite3_step(statement) == SQLITE_ROW)
		{
			int fromNode = sqlite3_column_int(statement, 1);
			int toNode   = sqlite3_column_int(statement, 2);
			int length   = sqlite3_column_int(statement, 3);
			int speed    = sqlite3_column_int(statement, 4);
			int oneway   = sqlite3_column_int(statement, 5);
			if (atNode == fromNode && (!oneway || oneway == 1))
			{
				cost = length + currentCost;
				sprintf(cmd, "INSERT INTO TEMP.NEXTMOVE VALUES(%i,%i,%i)", cost, fromref,toNode);
				st = SQLOK(sqlite3_exec(db, cmd, NULL, NULL, 0), db, "", 0);

				rtn++;
			}
			else if (atNode == toNode && (!oneway || oneway == 2))
			{
				toNode = fromNode;
				cost = length + currentCost;
				sprintf(cmd, "INSERT INTO TEMP.NEXTMOVE VALUES(%i,%i,%i)", cost, fromref, toNode);
				st = SQLOK(sqlite3_exec(db, cmd, NULL, NULL, 0), db, "", 0);

				rtn++;
			}
			else if (atNode == -1)
			{
				toNode = fromNode;
				cost = length / 2;
				if (!oneway || oneway == 2)
				{
					sprintf(cmd, "INSERT INTO TEMP.NEXTMOVE VALUES(%i,%i,%i)", cost+1, fromref, fromNode);
					st = SQLOK(sqlite3_exec(db, cmd, NULL, NULL, 0), db, "", 0);
					rtn++;
				}
				if (!oneway || oneway == 1)
				{
					sprintf(cmd, "INSERT INTO TEMP.NEXTMOVE VALUES(%i,%i,%i)", cost-1, fromref, toNode);
					st = SQLOK(sqlite3_exec(db, cmd, NULL, NULL, 0), db, "", 0);
					rtn++;
				}

			}
		}
		sqlite3_finalizeGSSi(&statement);
	}
	return rtn;
}

void WalkOutTest(LPSTR File, int startSeg)
{
	BOOL done = FALSE;
	sqlite3* db = NULL;
	char cmd[1024];
	sqlite3_stmt* statement;
	int st = sqlite3_open(File, &db);
	if (st == SQLITE_OK)
	{
		st = SQLOK(sqlite3_exec(db, "BEGIN", NULL, NULL, 0), db, "", 0);
		strcpy(cmd, "CREATE TEMP TABLE NEXTMOVE (TOTCOST INT,FROMREF INT,ATNODE INT,PRIMARY KEY (TOTCOST,FROMREF));");
		st = SQLOK(sqlite3_exec(db, cmd, NULL, NULL, 0), db, "", 0);
		AddSementToWalkout(db, startSeg, -1, 0);
		while (!done)
		{
			sprintf(cmd, "SELECT * FROM TEMP.NEXTMOVE LIMIT 1;");
			SQLOK(sqlite3_prepare_v2GSSi(db, cmd, -1, &statement, 0), db, "walkout", 0);
			if (sqlite3_step(statement) == SQLITE_ROW)
			{
				int currentCost = sqlite3_column_int(statement, 0);
				int refno = sqlite3_column_int(statement, 1);
				int atNode = sqlite3_column_int(statement, 2);
				AddSementToWalkout(db, refno, atNode, currentCost);
				sqlite3_finalizeGSSi(&statement);
				sprintf(cmd, "DELETE FROM NEXTMOVE WHERE TOTCOST=%i AND FROMREF=%i",currentCost,refno);
				st = SQLOK(sqlite3_exec(db, cmd, NULL, NULL, 0), db, "", 0);
				sprintf(cmd, "SELECT REFNO, FROMNODE, TONODE FROM SEGMENTS WHERE FROMNODE = %i OR TONODE = %i",atNode,atNode);
				SQLOK(sqlite3_prepare_v2GSSi(db, cmd, -1, &statement, 0), db, "walkout", 0);
				while (sqlite3_step(statement) == SQLITE_ROW)
				{
					int refno = sqlite3_column_int(statement, 0);
					int fromNode = sqlite3_column_int(statement, 1);
					int toNode = sqlite3_column_int(statement, 2);
					if (fromNode == atNode)
						AddSementToWalkout(db, refno, toNode, currentCost);
					else
						AddSementToWalkout(db, refno, fromNode, currentCost);
				}
				sqlite3_finalizeGSSi(&statement);
			}
			else
			{
				done = TRUE;
				sqlite3_finalizeGSSi(&statement);
			}
		}
		st = SQLOK(sqlite3_exec(db, "COMMIT", NULL, NULL, 0), db, "", 0);
		st = sqlite3_close(db);
	}
	return;
}
