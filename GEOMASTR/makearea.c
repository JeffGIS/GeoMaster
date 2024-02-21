#include "graphint.h"
#include "gmextern.h"

static NODESDATA	NodesData;
static LINESDATA	LinesData;
static HANDLE	hBTLinks=0;
static HANDLE	hBTInclusion=0;
static HANDLE	hBTExclusion=0;
static double	AddNodeFactor;
static DPOINT	AddNodeBias;
static HANDLE	hDBRefConnect=0;
static HFILE	FidDupList=HFILE_ERROR;
static double SnapTol=0.001; 
static double HalfSnapTol=0.0005;  
static LPOINT	SnapPointFileL;
static LOOPKEY	LoopKey;
static LOOPDATA	LoopData;
static HFILE	FidLoop;
static HFILE	FidLinks;
static HANDLE	hTranToLPoint=0; 

static	struct	{
			LPOINT	NodeID;
			double	AZ;
			}	NodesKey; 

static	struct {
			LPOINT	Point; 
			long	Ref;
			} ConnectKey;


int SetConStat (int NumConnections)
{
	switch (NumConnections)
	{
		case 0:
			return 1;
		case 1:
			return 2;
		case 2:
			return 0;
		case 3:
			return 3;
	}
	return NumConnections; 
}

void RoundToPTOL (LPDPOINT Point)
{
	double x = fmod (Point->x,SnapTol);
	double y = fmod (Point->y,SnapTol);
	Point->x -= x;
	Point->y -= y;
	if (x > HalfSnapTol)
		Point->x += SnapTol;
	if (y > HalfSnapTol)
		Point->y += SnapTol;
	return;
}

 

BOOL DecompInit (LPMNMXCORD Rect)
{   
	double	MaxDist;
	short	ld;
	BTVARDESC	BTVar[6];
	char	File[MAX_PATH];  
	OFSTRUCTGM	OFStruct;

	MaxDist = max (Rect->xmx - Rect->xmn,Rect->ymx - Rect->ymn); 
	AddNodeBias.x =  (Rect->xmx + Rect->xmn)/2;
	AddNodeBias.y =  (Rect->ymx + Rect->ymn)/2; 
	ld = log10 (MaxDist);
	
	AddNodeFactor = pow (10,8 - ld);
	GSSiGetTempFileName (0,"gm",0,(LPSTR)File);
	BTVar[0].BT_VARTYP=BT_INTEGER;
	BTVar[0].BT_VARLEN=4;
	BTVar[0].BT_VAROFF=0;
	BTVar[1].BT_VARTYP=BT_INTEGER;
	BTVar[1].BT_VARLEN=4;
	BTVar[1].BT_VAROFF=4;
	BTVar[2].BT_VARTYP=BT_INTEGER;
	BTVar[2].BT_VARLEN=4;
	BTVar[2].BT_VAROFF=8;
	BTVar[3].BT_VARTYP=BT_INTEGER;
	BTVar[3].BT_VARLEN=4;
	BTVar[3].BT_VAROFF=12;
	BTVar[4].BT_VARTYP=BT_INTEGER;
	BTVar[4].BT_VARLEN=4;
	BTVar[4].BT_VAROFF=16;
	BTVar[5].BT_VARTYP=BT_INTEGER;
	BTVar[5].BT_VARLEN=4;
	BTVar[5].BT_VAROFF=20;
	BT_CREATE (File,sizeof(LinesData), FALSE, 6, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
	hLines= BT_OPEN (File, 0, BT_WRITE, 0); 
    
	GSSiGetTempFileName (0,"gm",0,(LPSTR)File);
	BT_CREATE (File,2, FALSE, 2, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
	hNodes= BT_OPEN (File, 0, BT_WRITE, 0); 
    
	return TRUE;
} 

void DecompClose (void)
{
	BT_CLOSEANDDELETE (&hLines);
	BT_CLOSEANDDELETE (&hNodes);
} 

void DecompAddLine (LPDPOINT FromPoint, LPDPOINT POCPoint,LPDPOINT ToPoint, long Poly, BOOL IsCurve, BOOL LinkBetweenNodes)
{
	LINESKEY	LinesKey1, LinesKey2; 
	LINESDATA	LinesData3;
	
	LinesKey1.FromNode.x = LinesKey2.ToNode.x = IDNINT ((FromPoint->x - AddNodeBias.x) * AddNodeFactor);
	LinesKey1.FromNode.y = LinesKey2.ToNode.y = IDNINT ((FromPoint->y - AddNodeBias.y) * AddNodeFactor);
	LinesKey1.POCNode.x = LinesKey2.POCNode.x = IDNINT ((POCPoint->x - AddNodeBias.x) * AddNodeFactor);
	LinesKey1.POCNode.y = LinesKey2.POCNode.y = IDNINT ((POCPoint->y - AddNodeBias.y) * AddNodeFactor);
	LinesKey1.ToNode.x = LinesKey2.FromNode.x = IDNINT ((ToPoint->x - AddNodeBias.x) * AddNodeFactor);
	LinesKey1.ToNode.y = LinesKey2.FromNode.y = IDNINT ((ToPoint->y - AddNodeBias.y) * AddNodeFactor);
	if (LinesKey1.FromNode.x == LinesKey1.ToNode.x && LinesKey1.FromNode.y == LinesKey1.ToNode.y)
		return; 
	if (BT_FIND (hLines,(LPSTR)&LinesKey1,BT_FIRST,BT_EQ,(LPSTR)&LinesData))
		LinesData.LeftPoly = LONG_MAX; 
	else if (!LinkBetweenNodes)
		return;
	LinesData.RightPoly = Poly; 
	LinesData.FromPoint = *FromPoint; 
	LinesData.POCPoint = *POCPoint;
	LinesData.ToPoint = *ToPoint; 
	LinesData.IsCurve = IsCurve;
    LinesData3 = LinesData;
	if (BT_FIND (hLines,(LPSTR)&LinesKey2,BT_FIRST,BT_EQ,(LPSTR)&LinesData3))
		LinesData3.RightPoly = LONG_MAX;
	else if (!LinkBetweenNodes)
		return;
    BT_PUT (hLines,(LPSTR)&LinesKey1,(LPSTR)&LinesData); 
    LinesData = LinesData3;
	LinesData.LeftPoly = Poly;
	LinesData.FromPoint = *ToPoint;
	LinesData.POCPoint = *POCPoint;
	LinesData.ToPoint = *FromPoint;  
	LinesData.IsCurve = IsCurve;
    BT_PUT (hLines,(LPSTR)&LinesKey2,(LPSTR)&LinesData); 
	return;
}

long CheckForDupLines (long Refno,LPOINT BP,LPOINT EP,LPLONG pMSLink)
{   
	long	dupref;
	
	struct {
			LPOINT Point1, Point2;
			}	DupLineKey;
	struct {
			long Ref, MSLink;
			}	DupLineRec;
	
	if (BP.x == EP.x)
	{   
		if (BP.y < EP.y) 
		{
			DupLineKey.Point1 = BP;
			DupLineKey.Point2 = EP;
		}
		else            
		{
			DupLineKey.Point1 = EP;
			DupLineKey.Point2 = BP;
		}
	}
	else
	{   
		if (BP.x < EP.x) 
		{
			DupLineKey.Point1 = BP;
			DupLineKey.Point2 = EP;
		}
		else            
		{
			DupLineKey.Point1 = EP;
			DupLineKey.Point2 = BP;
		}
	}
	if (!BT_FIND (hBTDups,(LPSTR)&DupLineKey,BT_FIRST,BT_EQ,(LPSTR)&DupLineRec))
	{
		dupref = DupLineRec.Ref;
		*pMSLink = DupLineRec.MSLink;
		return dupref;
	} 
	DupLineRec.Ref = Refno;
	DupLineRec.MSLink = *pMSLink;
	BT_PUT (hBTDups,(LPSTR)&DupLineKey,(LPSTR)&DupLineRec); 
	return 0;
}    

void CloseRefConnectFile (void)
{
	if (hDBRefConnect)
    	CloseGWDatabase (hDBRefConnect);  
    hDBRefConnect=0;
	BT_CLOSEANDDELETE (&hBTDups);
    GSSiClose2 (&FidDupList);
    FidDupList = HFILE_ERROR;   
    CloseTRANS2 (&hTranToLPoint);
    return;
}
   
BOOL DumpLinkData (LPSTR InName) 
{
	short	pos=BT_FIRST;
	long	LinkRef;    	
    BTVARDESC  *pVars;  
    short       NumFields, Reclen, len;
    GWDHEADER GWDHead; 
    LPGWDHEADER lpGWDHead;
    HANDLE  hVars, hDB;
    short       FidData,ibeg,NumVars;
    OFSTRUCTGM    OFStruct;
    GWFLDINFO FldInfo; 
    char	Name[MAX_PATH];   
    typedef struct {
    				long	Refno;
 					LPOINT	FromNode,
 							ToNode; 
 					double	FromAZ,
 							ToAZ;
 				}	LINKDB;
    typedef	LINKDB	FAR	*LPLINKDB;   
    LINKDATA	LinkData;
    LPLINKDB	pLinkDB;
	char	DefStr[]="INT_REFNO(B4),FromNodeX(B4),FromNodeY(B4),ToNodeX(B4),ToNodeY(B4),FromAZ(R8),ToAZ(R8)";

    _fstrcpy (Name,InName);
    ExpandText (Name); 

	if (!CreateGWDDatabase (Name,1,FALSE,0,1,DefStr))
		return FALSE;	
	hDB = OpenGWDatabase (Name,BT_WRITE);
	if (!hDB)
		return FALSE;
    lpGWDHead = (LPGWDHEADER)GlobalLock (hDB);   
	pLinkDB = (LPLINKDB)&lpGWDHead->GWDData; 
	while (!BT_FIND (hBTLinks,(LPSTR)&LinkRef,pos,BT_ANY,(LPSTR)&LinkData)) 
	{   
		pos = BT_NEXT;
		pLinkDB->Refno = LinkRef;
		pLinkDB->FromNode=LinkData.FromNode;
		pLinkDB->ToNode=LinkData.ToNode;
		pLinkDB->FromAZ=LinkData.FromAZ;
		pLinkDB->ToAZ=LinkData.ToAZ;
		GWDAddRecord (lpGWDHead,0,0);
	}
	GlobalUnlock (hDB);
	CloseGWDatabase (hDB); 
	return TRUE;  
}

BOOL OpenRefConnectionFile (LPSTR InName,LPMNMXCORD pBounds)
{   
	HANDLE	hDB; 
	OFSTRUCTGM	OFStruct;
	char	Name[MAX_PATH];   
	LPSTR	pBS;  
	float	RSQMIN;
	double	XFROM[2],YFROM[2],XTO[2],YTO[2]; 
	double	factor1,factor2,factor; 
	LPGWDHEADER	pGWDHead;
	LPMNMXCORD	pFileMNMX;
	
	
	_fstrcpy (Name,InName);
	ExpandText (Name);
	if (!ExistFile (Name))
		CreateRefConnectionFile (Name,pBounds);
	hDBRefConnect = OpenGWDatabase (Name,BT_WRITE);
	if (!hDBRefConnect)                          
		return FALSE;
	ExpandText (Name);
	_fullpath (Name,Name,sizeof (Name));
	pBS = _fstrrchr (Name,'\\'); 
	pBS++;
	_fstrcpy (pBS,"dupcheck.btr");
	hBTDups = BT_OPEN (Name, 0, BT_WRITE, 0);  
	_fstrcpy (pBS,"duplist.bin"); 
	pGWDHead = GlobalLock (hDBRefConnect);
	FidDupList = GSSiOpenFile (Name,&OFStruct,OF_CREATE);   
	pFileMNMX = &pGWDHead->FileBounds;
	XFROM[0] = pFileMNMX->xmn;
	XFROM[1] = pFileMNMX->xmx;
	YFROM[0] = pFileMNMX->ymn;
	YFROM[1] = pFileMNMX->ymx;  
	factor1 =  (double)LONG_MAX / (pFileMNMX->xmx - pFileMNMX->xmn);
	factor2 =  (double)LONG_MAX / (pFileMNMX->ymx - pFileMNMX->ymn);  
	factor = min (factor1,factor2) / 10;
	SnapTol = 2 * P_TOL;
	if (2 * P_TOL * factor > 1)
		factor /= (2 * P_TOL * factor);
	XTO[0] = 0;
	YTO[0] = 0;  
	XTO[1] = (pFileMNMX->xmx - pFileMNMX->xmn) * factor;
	YTO[1] = (pFileMNMX->ymx - pFileMNMX->ymn) * factor;

	hTranToLPoint =  STRAN2 (1638,XFROM,YFROM,XTO,YTO,2,(LPFLOAT)&RSQMIN,1,0);
	GlobalUnlock (hDBRefConnect);
	return TRUE;
}

BOOL CreateRefConnectionFile (LPSTR InName,LPMNMXCORD pBounds)
{
	short		pos=BT_FIRST;
    BTVARDESC  *pVars;  
    BTVARDESC	Vars[5];
    short       NumFields, Reclen, len;
    GWDHEADER16 GWDHead; 
	LPGWDHEADER	pGWDHead32;
    LPGWDHEADER16 lpGWDHead;
    HANDLE  hVars, hDB;
    short       FidData,ibeg,NumVars;
    OFSTRUCTGM    OFStruct;
    GWFLDINFO FldInfo; 
    char	Name[MAX_PATH], FullName[MAX_PATH];   
    char	PrimeIndex[MAX_PATH];
    LPSTR	lpDot, lpEnd, pBS;
	time_t		timeStamp;
	char	DefStr[]="INT_REFNO(B4),BPXFile(B4),BPYFile(B4),EPXFile(B4),EPYFile(B4),ConnectionStatus(B2),BPConnections(B2),EPConnections(B2),Symbol(B2),NumPoints(B4),BPXBase(R8),BPYBase(R8),EPXBase(R8),EPYBase(R8),Length(R8)";
   
    _fstrcpy (Name,InName);
    ExpandText (Name); 


	if (!CreateGWDDatabase (Name,1,FALSE,0,1,DefStr))
		return FALSE;
/*    _fstrcpy (PrimeIndex,Name);
    lpDot = _fstrrchr (PrimeIndex,'.');
    *lpDot = 0;
    _fstrcat (lpDot,".in1");
    lpGWDHead = &GWDHead; 
    
    FidData = GSSiOpenFile (Name,&OFStruct,OF_CREATE);   
    _fmemset (lpGWDHead,0,sizeof(GWDHEADER16));
    GWDHead.NumIndex=1;
    GWDHead.Version=1;
    GWDHead.NumIndexFields[0]=1;
    GWDHead.IndexFields[0][0]=0;
    GWDHead.NumIndexFields[1]=2;
    GWDHead.NumIndexFields[2]=2;
    GWDHead.IndexFields[1][0]=1;
    GWDHead.IndexFields[1][1]=2;
    GWDHead.IndexFields[1][2]=0;
    GWDHead.IndexFields[2][0]=3;
    GWDHead.IndexFields[2][1]=4;
    GWDHead.IndexFields[2][2]=0; 
    BigWrite (FidData,(HPSTR)&GWDHead,sizeof(GWDHEADER16),-1);
    ibeg = 0;

    FldInfo.Len = 4;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"INT_REFNO");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 4;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"BPXFile");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 4;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"BPYFile");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 4;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"EPXFile");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 4;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"EPYFile");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 2;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"ConnectionStatus");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 2;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"BPConnections");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 2;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"EPConnections");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 2;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"Symbol");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

     FldInfo.Len = 4;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"NumPoints");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 8;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_REAL;
    _fstrcpy (FldInfo.Name,"BPXBase");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 8;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_REAL;
    _fstrcpy (FldInfo.Name,"BPYBase");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 8;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_REAL;
    _fstrcpy (FldInfo.Name,"EPXBase");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 8;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_REAL;
    _fstrcpy (FldInfo.Name,"EPYBase");
    BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
    GWDHead.NumFields++;

    FldInfo.Len = 8;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_REAL;
    _fstrcpy (FldInfo.Name,"Length");
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
*/	
	{
		HANDLE	hKeyFields;
			
		GetFieldIDsFromNames (Name,&hKeyFields,0,"BPXFile;BPYFile",0);
		GWDAddIndex (Name,hKeyFields,FALSE,0);
		GSSiGlobFree (&hKeyFields);
		GetFieldIDsFromNames (Name,&hKeyFields,0,"EPXFile;EPYFile",0);
		GWDAddIndex (Name,hKeyFields,FALSE,0);
		GSSiGlobFree (&hKeyFields);
		GetFieldIDsFromNames (Name,&hKeyFields,0,"ConnectionStatus",0);
		GWDAddIndex (Name,hKeyFields,FALSE,0);
		GSSiGlobFree (&hKeyFields);
	} 
/*	
	lpEnd = _fstrchr (PrimeIndex,0);
	lpEnd--;
	*lpEnd = '2';
	GSSiRemove (PrimeIndex);
	*lpEnd = '3';
	GSSiRemove (PrimeIndex);
*/	
	hDB = OpenGWDatabase (Name,BT_WRITE);
	if (!hDB)
		return (FALSE);
	pGWDHead32 = GlobalLock (hDB);
	timeStamp = pGWDHead32->TimeStamp;
	pGWDHead32->FileBounds = *pBounds;
	GlobalUnlock (hDB);
	CloseGWDatabase (hDB); 
	ExpandText (Name);
	_fullpath (Name,Name,sizeof (Name));
	pBS = _fstrrchr (Name,'\\'); 
	pBS++;
	_fstrcpy (pBS,"dupcheck.btr");
	Vars[0].BT_VARLEN=4;
	Vars[0].BT_VARTYP=BT_INTEGER;
	Vars[0].BT_VAROFF=0;
	Vars[1].BT_VARLEN=4;
	Vars[1].BT_VARTYP=BT_INTEGER;
	Vars[1].BT_VAROFF=4;
	Vars[2].BT_VARLEN=4;
	Vars[2].BT_VARTYP=BT_INTEGER;
	Vars[2].BT_VAROFF=8;
	Vars[3].BT_VARLEN=4;
	Vars[3].BT_VARTYP=BT_INTEGER;
	Vars[3].BT_VAROFF=12;
	BT_CREATE (Name, 4, FALSE, 4, 1,Vars,FALSE, 0, timeStamp, FALSE);
	
	return TRUE;
} 

BOOL RefConnectTableCreate (LPMNMXCORD pBounds)
{   
	int	st, BPConnect,EPConnect,SaveMaxPick;
	LPGWDHEADER lpGWDHead;
	BOOL	HaveMidHit;
	HIGHLIGHTDATA	HighlightData;         
	LPREFCONNECT	pRC;
	short	pos, pos2, len;  
	long	Refno, Offset, NumDups=0, dupref;  
	HCURSOR	hcurSave;
	POINT	Point; 
	char	mess[MAX_PATH];  
   
	LPLINEINT	hpLineInt, hpLineIntStart, hpLineInt1, hpLineInt2; 
	HANDLE		hLineInt=0;
	long		maxlines, nHighlight;  
	long		nRecs, nLoaded=0;
	DWORD		il, jl, NumLines=0; 
	LPOINT		LastPoint;  
	LPTHEME		pTheme; 
	MNMXCORD	IntBounds;
	double		FixTol=GetGlobalDVal2("[%FIXTOL]",0.1);//P_TOL*200); //should be no bigger than 1/2 of the smallest segment size
	static		long	debugref=1000006;
	short		ii;
	short		nBlocks=1;
	long		MSLink;   	
		
	//GSSiRemove ("[%CONSTATFILE].gmd");  
	nRecs = BT_NUM_IN_INDEX (hHighlight);
	nRecs = BT_NUM_IN_INDEX (hHighlight2);
	CloseRefConnectFile ();
	CreateRefConnectionFile ("[%CONSTATFILE].gmd",pBounds);
	OpenRefConnectionFile ("[%CONSTATFILE].gmd",pBounds);
    lpGWDHead = (LPGWDHEADER)GlobalLock (hDBRefConnect);   
    pRC = (LPREFCONNECT)&lpGWDHead->GWDData; 
    pos = BT_FIRST;

	nRecs = BT_NUM_IN_INDEX (hHighlight);
	CreateStatusWind (CurView->hWnd,2,"Build Connection Table");
	StatusWindowUpdate (0,"Step 1", nRecs, nLoaded);
		
	while (ContinueProcessing && !BT_FIND (hHighlight,(LPSTR)&Refno,pos,BT_ANY,(LPSTR)&HighlightData)) 
	{   
		pos = BT_NEXT;
		if (Refno == debugref)
			ii=1; 
		if (HighlightData.PD.Length < P_TOL)
			goto NextRec;
		if (HighlightData.PD.NumPoints == 2)
		{   
			if ((dupref=CheckForDupLines (Refno,DPointToFilePointL (&HighlightData.PD.BeginPoint,hTranToLPoint),DPointToFilePointL (&HighlightData.PD.EndPoint,hTranToLPoint),&MSLink)))
			{
				NumDups ++; 
				BigWrite (FidDupList,(HPSTR)&Refno,4,-1);
				BigWrite (FidDupList,(HPSTR)&HighlightData.PD,sizeof(PICKDATA),-1);
				//sprintf (mess,"%ld dup with %ld",Refno,dupref); 
				//SetWindowText (hWndMain,mess); 
			}
		}
			
		BPConnect=EPConnect=0;  
		ConnectKey.Point = DPointToFilePointL (&HighlightData.PD.BeginPoint,hTranToLPoint);
		ConnectKey.Ref = 0;
        while (!BT_FIND (lpGWDHead->BTHandle[1],(LPSTR)&ConnectKey,BT_FIRST,BT_GT,(LPSTR)&Offset))
        {   
        	LPOINT	BPL=DPointToFilePointL (&HighlightData.PD.BeginPoint,hTranToLPoint);
        	 
        	if (ConnectKey.Point.x ==  BPL.x &&	ConnectKey.Point.y == BPL.y)
        	{
        		len = FillGWDData (lpGWDHead,Offset); 
				if (len > 0)
				{
        			pRC->BPConnections++;
					pRC->ConnectStatus=SetConStat (min(pRC->BPConnections,1)+min(pRC->EPConnections,1));
					GWDReplaceRecord (lpGWDHead,len,0,Offset);  
				}
				BPConnect++; 
			}
			else break;
    	}
		ConnectKey.Point = DPointToFilePointL (&HighlightData.PD.BeginPoint,hTranToLPoint);
		ConnectKey.Ref = 0;
        while (!BT_FIND (lpGWDHead->BTHandle[2],(LPSTR)&ConnectKey,BT_FIRST,BT_GT,(LPSTR)&Offset))
        {    
        	LPOINT	BPL=DPointToFilePointL (&HighlightData.PD.BeginPoint,hTranToLPoint);
        	 
        	if (ConnectKey.Point.x ==  BPL.x &&	ConnectKey.Point.y == BPL.y)
        	{
        		len = FillGWDData (lpGWDHead,Offset); 
				if (len > 0)
				{
        			pRC->EPConnections++;
					pRC->ConnectStatus=SetConStat (min(pRC->BPConnections,1)+min(pRC->EPConnections,1));
					GWDReplaceRecord (lpGWDHead,len,0,Offset); 
				}
				BPConnect++; 
			}
			else break;
    	}
		ConnectKey.Point = DPointToFilePointL (&HighlightData.PD.EndPoint,hTranToLPoint);
		ConnectKey.Ref = 0;
        while (!BT_FIND (lpGWDHead->BTHandle[1],(LPSTR)&ConnectKey,BT_FIRST,BT_GT,(LPSTR)&Offset))
        {    
        	LPOINT	BPL=DPointToFilePointL (&HighlightData.PD.EndPoint,hTranToLPoint);
        	 
        	if (ConnectKey.Point.x ==  BPL.x &&	ConnectKey.Point.y == BPL.y)
        	{
        		len = FillGWDData (lpGWDHead,Offset); 
				if (len > 0)
				{
        			pRC->BPConnections++;
					pRC->ConnectStatus=SetConStat (min(pRC->BPConnections,1)+min(pRC->EPConnections,1));
					GWDReplaceRecord (lpGWDHead,len,0,Offset);  
				}
				EPConnect++;  
			}
			else break;
    	}
		ConnectKey.Point = DPointToFilePointL (&HighlightData.PD.EndPoint,hTranToLPoint);
		ConnectKey.Ref = 0;
        while (!BT_FIND (lpGWDHead->BTHandle[2],(LPSTR)&ConnectKey,BT_FIRST,BT_GT,(LPSTR)&Offset))
        {    
        	LPOINT	BPL=DPointToFilePointL (&HighlightData.PD.EndPoint,hTranToLPoint);
        	 
        	if (ConnectKey.Point.x ==  BPL.x &&	ConnectKey.Point.y == BPL.y)
        	{
        		len = FillGWDData (lpGWDHead,Offset); 
				if (len > 0)
				{
        			pRC->EPConnections++;
					pRC->ConnectStatus=SetConStat (min(pRC->BPConnections,1)+min(pRC->EPConnections,1));
					GWDReplaceRecord (lpGWDHead,len,0,Offset);  
				}
				EPConnect++;
			}
			else break;
    	}
		pRC->Ref = Refno;
		pRC->BPFile = DPointToFilePointL (&HighlightData.PD.BeginPoint,hTranToLPoint);
		pRC->EPFile = DPointToFilePointL (&HighlightData.PD.EndPoint,hTranToLPoint);  
		if (pRC->BPFile.x == pRC->EPFile.x &&    
			pRC->BPFile.y == pRC->EPFile.y)
		{
			BPConnect++;
			EPConnect++; 
		}
		pRC->BPConnections = BPConnect;
		pRC->EPConnections = EPConnect;  
		pRC->ConnectStatus=SetConStat (min(pRC->BPConnections,1)+min(pRC->EPConnections,1));
		pRC->BPBase = HighlightData.PD.BeginPoint;
		pRC->EPBase = HighlightData.PD.EndPoint; 
		pRC->Length = HighlightData.PD.Length;
		pRC->NumPoints = HighlightData.PD.NumPoints;
		pRC->Symbol = HighlightData.PD.Desc;  
	    GWDAddRecord (lpGWDHead,0,0);    
NextRec:
		StatusWindowUpdate (0,0, nRecs, ++nLoaded);

	} 

	UseUserPickAp =FALSE;
	SystemPickAp = -FixTol;	
	SaveMaxPick=MaxPick;
	MaxPick=MAXPICKITEMS-1;
	SetPickAp(0);
    Refno = LONG_MIN;
	nRecs = BT_NUM_IN_INDEX (lpGWDHead->BTHandle[0]); 
	nLoaded = 0;
	StatusWindowUpdate (0,"Step 2", nRecs, nLoaded);
	while (ContinueProcessing && !BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&Refno,BT_FIRST,BT_GT,(LPSTR)&Offset)) 
	{   
		pos = BT_NEXT; 
   		len = FillGWDData (lpGWDHead,Offset);   
   		HaveMidHit=FALSE;
		SetViewport (*pCommandViewport);
   		if (pRC->BPConnections>1)
   		{
			PickItems2 (CurView->hWnd,pRC->BPBase,FALSE,TRUE,TRUE); 
			while (NumPicked--)
			{   
				if (((PickList[NumPicked].PCT * PickList[NumPicked].Length) > SnapTol) &&
					(((1.0-PickList[NumPicked].PCT) * PickList[NumPicked].Length) > SnapTol)) 
					HaveMidHit=TRUE;
			} 
   		} 
   		if (pRC->EPConnections>1)
   		{
			PickItems2 (CurView->hWnd,pRC->EPBase,FALSE,TRUE,TRUE); 
			while (NumPicked--)
			{   
				if (((PickList[NumPicked].PCT * PickList[NumPicked].Length) > SnapTol) &&
					(((1.0-PickList[NumPicked].PCT) * PickList[NumPicked].Length) > SnapTol)) 
					HaveMidHit=TRUE;
			} 
   		} 
   		if (HaveMidHit)
   		{
   			pRC->ConnectStatus = SetConStat (3);
			GWDReplaceRecord (lpGWDHead,len,0,Offset);
		}
		SetContinueProcessing ( StatusWindowUpdate (0,0, nRecs, ++nLoaded));
   	}

	UseUserPickAp =TRUE;
	MaxPick=SaveMaxPick;
	GlobalUnlock (hDBRefConnect);  
	IntBounds = HLTBounds;
	ClearHighlightList (FALSE);
	GSSillseek (FidDupList,0,0);
	while (ContinueProcessing && BigRead (FidDupList,(HPSTR)&Refno,4) == 4)
	{   
		BigRead (FidDupList,(HPSTR)&PickList[0],sizeof(PICKDATA));
//		if (PickByRefno(Refno,0,0,-1))
		DeletePickedItem (0,12,92);
	}    
		

    if (ContinueProcessing && GetGlobalBVal ("[%CHECKINTS]"))
    {   
    	MNMXCORD Bounds[16];
    	short	iblock, irow, icol;  
    	double	rowheight, colwidth, colx, rowy;
        	
    	Bounds[0] = IntBounds;
    	rowheight = (IntBounds.ymx - IntBounds.ymn)/4;
    	colwidth = (IntBounds.xmx - IntBounds.xmn)/4;   
        	
    	iblock=0;
    	if (nRecs > 500)
    	{   
    		nBlocks = 16;
        	for (irow=0,rowy=IntBounds.ymn;irow<4;irow++,rowy+=rowheight)
	        	for (icol=0,colx=IntBounds.xmn;icol<4;icol++,colx+=colwidth)
	        	{ 
	        		Bounds[iblock].xmn = colx;
	        		Bounds[iblock].ymn = rowy;
	        		Bounds[iblock].xmx = colx + colwidth;
	        		Bounds[iblock++].ymx = rowy + rowheight;
	        	} 
        }
    	nLoaded = 0;
		StatusWindowUpdate (0,"Check for Intersections", nBlocks, nLoaded);
    	for (iblock=0;iblock<nBlocks;iblock++)
    	{
			ClearHighlightList (FALSE);
			HighlightInArea (CurView->hWnd,&Bounds[iblock],TRUE,FALSE,0); 
			if (TotHLTPoints)
			{   
				hLineInt = GSSiGlobAlloc ( 577,GMEM_MOVEABLE,TotHLTPoints * (long) sizeof(LINEINT));
		   		hpLineInt = hpLineIntStart = (LPLINEINT)GlobalLock (hLineInt);
			    pos = BT_FIRST;
		        NumLines=0;
					
				while (ContinueProcessing && !BT_FIND (hHighlight,(LPSTR)&Refno,pos,BT_ANY,(LPSTR)&HighlightData)) 
				{   
					if (Refno == debugref)
						ii=1; 
					pos = BT_NEXT; 
					if (HighlightData.PD.NumPoints == 2)
					{   
						hpLineInt->Point1 = DPointToFilePointL (&HighlightData.PD.BeginPoint,hTranToLPoint);
						hpLineInt->Point2 = DPointToFilePointL (&HighlightData.PD.EndPoint,hTranToLPoint);
						hpLineInt->Refno = Refno;
						hpLineInt->Mnx = min (hpLineInt->Point1.x,hpLineInt->Point2.x);
						hpLineInt++->Mxx = max (hpLineInt->Point1.x,hpLineInt->Point2.x);
						NumLines++;
					}
					else
					{   
						HANDLE	hPoly=0;
						HPDPOINT	pPoint;
						
						PickList[0] = HighlightData.PD;
					    SetConfig (PickList[0].ConfigID);
					    SetViewport (PickList[0].ViewID);
						if (GetPolyPoints((LPPICKDATAHEADER)&PickList[0], FALSE, &nPnts, &hPoly, 0))
						{
							pPoint = (HPDPOINT)GlobalLock(hPoly);
							LastPoint = DPointToFilePointL(pPoint++, hTranToLPoint);
							nPnts--;
							while (nPnts--)
							{
								hpLineInt->Point1 = LastPoint;
								hpLineInt->Point2 = DPointToFilePointL(pPoint, hTranToLPoint);
								hpLineInt->Refno = Refno;
								hpLineInt->Mnx = min(hpLineInt->Point1.x, hpLineInt->Point2.x);
								hpLineInt++->Mxx = max(hpLineInt->Point1.x, hpLineInt->Point2.x);
								NumLines++;
								LastPoint = DPointToFilePointL(pPoint++, hTranToLPoint);
							}
							GSSiGlobUlFree(&hPoly);
						}
			         }
				}	
		        for (il = 0, hpLineInt1=hpLineIntStart; il<NumLines-1; il++,hpLineInt1++) 
				{
		        	for (jl = il+1,hpLineInt2=hpLineInt1+1;jl < NumLines;jl++,hpLineInt2++) 
		        	{
	        			if (hpLineInt1->Refno == 1000000 && hpLineInt2->Refno == debugref)
	        				ii=1;
						if (hpLineInt1->Refno != hpLineInt2->Refno)
						{
		        			if (LineInt (hpLineInt1,hpLineInt2))
		        			{   
		        				if (hpLineInt1->Refno == debugref || hpLineInt2->Refno == debugref)
		        					ii=1;
								BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&hpLineInt1->Refno,BT_FIRST,BT_EQ,(LPSTR)&Offset);
				       			len = FillGWDData (lpGWDHead,Offset);   
								pRC->ConnectStatus = 99;
								if (len > 0)
									GWDReplaceRecord (lpGWDHead,len,0,Offset);
								BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&hpLineInt2->Refno,BT_FIRST,BT_EQ,(LPSTR)&Offset);
				       			len = FillGWDData (lpGWDHead,Offset);   
								pRC->ConnectStatus = 99;
								if (len > 0)
									GWDReplaceRecord (lpGWDHead,len,0,Offset);
							}
						}
		            }
					SetContinueProcessing ( StatusWindowUpdate2 (0, NumLines, il));
					if (!ContinueProcessing)
						break;
				}
		        GSSiGlobUlFree (&hLineInt);
		    } 
			SetContinueProcessing ( StatusWindowUpdate (0,0, nBlocks, ++nLoaded));
			if (!ContinueProcessing)
				break;
	  	}
	}
		
	ClearHighlightList (FALSE);
		
	CloseRefConnectFile ();
	DestroyStatusWindow(0); 
	SetContinueProcessing ( TRUE);
	return TRUE;
}



BOOL FixRefConnect (void)
{   
	int	st, BPConnect,EPConnect;
	LPGWDHEADER lpGWDHead;
	HIGHLIGHTDATA	HighlightData;
	LPREFCONNECT	pRC;
	short	pos, pos2, len,ii, SnapItem, FixItem, SaveMaxPick,i, Pass;  
	long	Refno, Offset, NearOpRef, NumRemoved=0, Newref1, Newref2, NearOpPoint, debugref=2034479;  
	HCURSOR	hcurSave;   
	POINT	Point;  
	double	FixTol=GetGlobalDVal2("[%FIXTOL]",1), NearOpDist, MinSegLength=FixTol*0.5;  
	BOOL	CheckAllSegs = GetGlobalBVal2 ("[%CHECKALLSEGS]",FALSE);      
	BOOL	AllowSplits = GetGlobalBVal2 ("[%ALLOWSPLITS]",FALSE);      
	double	d;
	DPOINT	Points[MAXPICKITEMS];
	long	nRecs, nLoaded=0, np;

	OpenRefConnectionFile ("[%CONSTATFILE].gmd",&CurView->FileMNMX);
    lpGWDHead = (LPGWDHEADER)GlobalLock (hDBRefConnect);
	UseUserPickAp =FALSE;
	SystemPickAp = -FixTol-(0.5*FixTol);	
	SaveMaxPick=MaxPick;
    pRC = (LPREFCONNECT)&lpGWDHead->GWDData;   
    Refno = LONG_MIN;
	nRecs = BT_NUM_IN_INDEX (lpGWDHead->BTHandle[0]);
	CreateStatusWind (CurView->hWnd,1,"Fix Connections");
	while (ContinueProcessing && !BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&Refno,BT_FIRST,BT_GT,(LPSTR)&Offset)) 
	{   
		if (Refno == debugref)
			ii=1;
		SetViewport (*pCommandViewport);
   		len = FillGWDData (lpGWDHead,Offset);
   		if (pRC->Length < MinSegLength && (!pRC->BPConnections || !pRC->EPConnections))			//eliminates short dead-end segments
   		{
	       	DoNotPickThisRefno=LONG_MAX;
			SnapPointFileL = AveragePointL (pRC->BPFile,pRC->EPFile);
			SnapPoint = AverageDPoint (pRC->BPBase,pRC->EPBase); 
	       	MaxPick=MAXPICKITEMS-1;
			PickItems (0,pRC->BPBase); 
			while (NumPicked--)
			{
				if (PickList[NumPicked].Refno == Refno)  
				{
					NumRemoved++;
					DeletePickedItem (NumPicked,12,92); 
				}
				else
				{   
					if (ldistp (pRC->BPBase,PickList[NumPicked].BeginPoint) < FixTol/2)
					{   
						SnapEnd = 1;
						SnapPickedItem (NumPicked); 
						UpdateConnectFile (PickList[NumPicked].Refno,lpGWDHead,
											SnapPoint,PickList[NumPicked].EndPoint,
											SnapPointFileL,DPointToFilePointL (&PickList[NumPicked].EndPoint,hTranToLPoint));
					}
					else if (ldistp (pRC->BPBase,PickList[NumPicked].EndPoint) < FixTol/2)
					{   
						SnapEnd = 2;
						SnapPickedItem (NumPicked);
						UpdateConnectFile (PickList[NumPicked].Refno,lpGWDHead,
											PickList[NumPicked].BeginPoint,SnapPoint,
											DPointToFilePointL (&PickList[NumPicked].BeginPoint,hTranToLPoint),SnapPointFileL);
					}
				}
			}
			PickItems (0,pRC->EPBase); 
			while (NumPicked--)
			{
				if (ldistp (pRC->BPBase,PickList[NumPicked].BeginPoint) < FixTol/2)
				{   
					SnapEnd = 1;
					SnapPickedItem (NumPicked);
					UpdateConnectFile (PickList[NumPicked].Refno,lpGWDHead,
											SnapPoint,PickList[NumPicked].EndPoint,
											SnapPointFileL,DPointToFilePointL (&PickList[NumPicked].EndPoint,hTranToLPoint));
				}
				else if (ldistp (pRC->BPBase,PickList[NumPicked].EndPoint) < FixTol/2)
				{   
					SnapEnd = 2;
					SnapPickedItem (NumPicked);
					UpdateConnectFile (PickList[NumPicked].Refno,lpGWDHead,
											PickList[NumPicked].BeginPoint,SnapPoint,
											DPointToFilePointL (&PickList[NumPicked].BeginPoint,hTranToLPoint),SnapPointFileL);
				}
			}
   			goto NextItem;
   		} 
   		d = ldistp (pRC->BPBase,pRC->EPBase);
   		if (d > P_TOL && d < FixTol)
   		{
			if (!PickByRefno(pRC->Ref,0,0,-1))
				goto NextItem;
		/*	{
		    	MessageBox (GetFocus(),"refindex needs to be recreated",0,MB_ICONEXCLAMATION);
		        BlowOut(0,0);
		    } */
			SnapEnd = 2; 
			PickList[0].PCT = 1;
			SnapPointFileL = DPointToFilePointL (&PickList[0].BeginPoint,hTranToLPoint);
			SnapPoint = PickList[0].BeginPoint;
			SnapPickedItem (0);
			UpdateConnectFile (PickList[0].Refno,lpGWDHead,
							   PickList[0].BeginPoint,SnapPoint,
							   DPointToFilePointL (&PickList[0].BeginPoint,hTranToLPoint),SnapPointFileL); 
			goto NextItem;
   		}  
   		Pass = -1;
NextPass:   Pass++;
   		if (CheckAllSegs || ((!Pass && !pRC->BPConnections) || (Pass && !pRC->EPConnections)))
   		{
	       	DoNotPickThisRefno=Refno;  
	       	MaxPick=1;
	       	if (Pass)
				PickItems (0,pRC->BPBase);
			else 
				PickItems (0,pRC->EPBase); 
			if (NumPicked)
			{  
				NearOpRef = PickList[0].Refno;
				NearOpDist = PickList[0].OffDist; 
				NearOpPoint = PickList[0].NearPoint;
			}
			else
				NearOpRef = LONG_MAX; 
	       	DoNotPickThisRefno=LONG_MAX;
	       	MaxPick=MAXPICKITEMS-1; 
	       	if (Pass)
				PickItems (0,pRC->EPBase); 
			else
				PickItems (0,pRC->BPBase);
			np = 0;  

			if (NumPicked >1)
			{   
				for (i=0;i<NumPicked;i++) 
				{
					if (PickList[i].Refno == NearOpRef &&     
						PickList[i].NearPoint == NearOpPoint &&
						PickList[i].OffDist > NearOpDist)
							ii=1;	 
						else
							Points[np++] = PickList[i].PickedPoint;
				}
				SnapPoint = AverageDPoints (Points,np); 
				ii=1;
				for (FixItem=0;FixItem<NumPicked;FixItem++)
				{
					if (!(PickList[FixItem].Refno == NearOpRef &&     
							PickList[FixItem].NearPoint == NearOpPoint &&
							PickList[FixItem].OffDist > NearOpDist))
					{   
						if (PickList[FixItem].Refno == debugref)
							ii=1;
						if ((PickList[FixItem].PCT * PickList[FixItem].Length) < FixTol)					
			    			SnapEnd = 1;
						else if (((1.0-PickList[FixItem].PCT) * PickList[FixItem].Length) < FixTol) 
			    			SnapEnd = 2;
			    		else
			    			SnapEnd = 5;
			    		switch (SnapEnd)
			    		{
			    			case 1:
								SnapPickedItem (FixItem);
								UpdateConnectFile (PickList[FixItem].Refno,lpGWDHead,
														SnapPoint,PickList[FixItem].EndPoint,
														SnapPointFileL,DPointToFilePointL (&PickList[FixItem].EndPoint,hTranToLPoint));
								break;
			    			case 2:
								SnapPickedItem (FixItem);
								UpdateConnectFile (PickList[FixItem].Refno,lpGWDHead,
														SnapPoint,PickList[FixItem].BeginPoint,
														SnapPointFileL,DPointToFilePointL (&PickList[FixItem].BeginPoint,hTranToLPoint));
								break;
							case 5:
								if (!AllowSplits)
									break;
								GetPickName (FixItem); 
								_fstrcpy (EditName,PickName);
								SplitPoly (FixItem,&Newref1,&Newref2); 
								break;
						}
					}
				}
			} 
   		}
		if (!Pass)
			goto NextPass;
NextItem:
		StatusWindowUpdate (0,0, nRecs, nLoaded++);
   	}   
   	DoNotPickThisRefno=LONG_MAX;
	UseUserPickAp =TRUE;
	MaxPick = SaveMaxPick;
	GlobalUnlock (hDBRefConnect); 
	CloseRefConnectFile ();
    SetGlobalValueLong ("%NumRemoved",NumRemoved);
	DestroyStatusWindow(0);  
	SetContinueProcessing ( TRUE); 
	return TRUE;
}

 

void UpdateConnectFile (long Refno,LPGWDHEADER lpGWDHead,DPOINT BP, DPOINT EP, LPOINT BPFile, LPOINT EPFile)
{ 
	LPREFCONNECT	pRC; 
	REFCONNECT		RCSave;
	short	len;  
	long	Offset;  

    pRC = (LPREFCONNECT)&lpGWDHead->GWDData; 
    RCSave = *pRC;  
	if (BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&Refno,BT_FIRST,BT_EQ,(LPSTR)&Offset))
		return;
	len = FillGWDData (lpGWDHead,Offset);
    pRC->Length = ldistp (BP,EP);         
    if (pRC->Length == 0)
		GWDDeleteRecord (lpGWDHead,Offset);  
    else
    {
	    pRC->BPBase = BP;
	    pRC->EPBase = EP;
	    pRC->BPFile = BPFile;
	    pRC->EPFile = EPFile;		 
		GWDReplaceRecord (lpGWDHead,len,0,Offset);  
	}
	*pRC = RCSave;
	return;
}

long TraceLink (long MaxLinks,long Refno,long LinkRef,long StartRef,LPGWDHEADER lpGWDHead,short AtEnd,long Offset, HPDPOINT *pPoints,LPLPOINT pToNode,LPDOUBLE pNodeAZ,LPSHORT nSyms,LPSHORT pSyms,BOOL TraceLines)
{ 
	LPREFCONNECT	pRC; 
	REFCONNECT		RCSave; 
	LPOINT			StartPoint, ConnectPointFile;
	LPDPOINT		pOpPointBase;
	DPOINT			ConnectPointBase; 
	LPSHORT			pConnections;
	LPTHEME			pTheme;   
	short	len,  st,ii;
	long	NumPoints=0; 
	UINT	i;
	short 	WhichEndNext=3-AtEnd, End;   
	LPSHORT	pSymbols;
	long	Ref; 
	static	long	debugrefno=1062189981, debuglink=1508; 
	BOOL	First=TRUE,Point1WasStartRef;

	*nSyms = 0;
    if (LinkRef == debuglink)
    	ii=1;
    pRC = (LPREFCONNECT)&lpGWDHead->GWDData; 
Next: 
	Ref = pRC->Ref;
	End = WhichEndNext; 
	if (pRC->Ref == debugrefno)
		ii=1;
    RCSave = *pRC;
	if (WhichEndNext == 1)
	{
		StartPoint = pRC->BPFile;
		pOpPointBase = &pRC->BPBase;
		ConnectPointBase = pRC->BPBase; 
		pConnections = &RCSave.BPConnections;
	}
	else
	{
		StartPoint = pRC->EPFile;
		pOpPointBase = &pRC->EPBase;
		ConnectPointBase = pRC->EPBase; 
		pConnections = &RCSave.EPConnections;
	} 
    		
	pSymbols = pSyms;
	for (i=0;i<*nSyms;i++,pSymbols++)
	{
		if (pRC->Symbol == *pSymbols)
			goto GotSym;
	}
	*pSymbols = pRC->Symbol;
	(*nSyms)++;
GotSym:
	if (pRC->NumPoints > 2)
	{
		if (!PickByRefno(pRC->Ref,0,0,-101))
		{
	    	MessageBox (GetFocus(),"refindex needs to be recreated",0,MB_ICONEXCLAMATION);
	        BlowOut(0,0);
	    }
	    SetConfig (PickList[0].ConfigID);
	    SetViewport (PickList[0].ViewID);
		pTheme = AddTheme (GF_SAVEPOLY_THEME);
		CurView->PassID = 4;
		ProcessSelectedTheme = CurView->NumThemes;
		ProcessPickedItem (0,FALSE);        		
		ProcessSelectedTheme = 0;
		DeleteTheme (pTheme);  
		GetSavedPolys ();
   		if (hSavePoly)
		{
		    LPMNMXCORD lpRect;
			long	nPnts; 
			HPDPOINT	lpDpoint;
            nPnts = nSavePoly-2;  
            if (nPnts + NumPoints > MaxLinks)   
            {
	    		MessageBox (GetFocus(),"Num links exceeds max",0,MB_ICONEXCLAMATION); 
	    		return -1;
	    	}
            lpRect = (LPMNMXCORD) GlobalLock (hSavePoly);
            lpRect++;
            lpDpoint = (HPDPOINT) lpRect;   
            if (WhichEndNext == 2)
            {
	            lpDpoint++;   
	            while (nPnts--)
	            {
					**pPoints = *lpDpoint++;
					(*pPoints)++; 
					NumPoints++;
	            }
	        }
	        else 
	        {   
	        	lpDpoint+=nPnts;
	            while (nPnts--)
	            {
					**pPoints = *lpDpoint--;
					(*pPoints)++; 
					NumPoints++;
	            }
	        }
			GlobalUnlock(hSavePoly);
         } 
         DestroySavedPolys ();
	}
	**pPoints = *pOpPointBase;
	(*pPoints)++; 
	NumPoints++;  
	if (StartRef == LONG_MAX || !First || TraceLines)
		GWDDeleteRecord (lpGWDHead,Offset);  
	if (*pConnections > 1)
	{
	    *pToNode = AddNode (1,WhichEndNext,ConnectPointBase,LinkRef,&RCSave,pNodeAZ); 
	    return NumPoints;
	} 
	ConnectKey.Point = StartPoint;
	ConnectKey.Ref = 0; 
	
	WhichEndNext = 2; 
    Point1WasStartRef=FALSE;
    st = BT_FIND (lpGWDHead->BTHandle[1],(LPSTR)&ConnectKey,BT_FIRST,BT_GT,(LPSTR)&Offset);
    if (!st && Ref == StartRef && ConnectKey.Ref == Ref && First &&
    	(ConnectKey.Point.x == StartPoint.x && ConnectKey.Point.y == StartPoint.y))
    {
    	Point1WasStartRef=TRUE;
	    st = BT_FIND (lpGWDHead->BTHandle[1],(LPSTR)&ConnectKey,BT_NEXT,BT_ANY,(LPSTR)&Offset);
	}
	if (st || ConnectKey.Point.x != StartPoint.x ||
        		ConnectKey.Point.y != StartPoint.y) 
    {
		WhichEndNext = 1;
		ConnectKey.Point = StartPoint;
		ConnectKey.Ref = 0; 
	    st = BT_FIND (lpGWDHead->BTHandle[2],(LPSTR)&ConnectKey,BT_FIRST,BT_GT,(LPSTR)&Offset);
	    if (!st && Ref == StartRef && ConnectKey.Ref == Ref && First)
	    {
	    	if (!Point1WasStartRef)
			    st = BT_FIND (lpGWDHead->BTHandle[2],(LPSTR)&ConnectKey,BT_NEXT,BT_ANY,(LPSTR)&Offset);
		}
		if (st || ConnectKey.Point.x != StartPoint.x ||
	        		ConnectKey.Point.y != StartPoint.y)
	    {   
	    	char	mess[128]; 
	    	
	    	if (TraceLines)  
	    		return NumPoints;

	    	sprintf (mess,"Dead end link %ld, NumPoints = %i, Refno = %ld, End = %i",LinkRef,NumPoints,pRC->Ref,End);
	    	MessageBox (GetFocus(),mess,0,MB_ICONEXCLAMATION);
   			DisplayMarker (*pOpPointBase,5,0,0,0,0,FALSE,FALSE,0,0,0,0,0);
	    	return -1;
	    } 
	}
	First = FALSE;
	len = FillGWDData (lpGWDHead,Offset);
	if (pRC->Ref == StartRef)
	{
	    *pToNode = AddNode (1,3-WhichEndNext,ConnectPointBase,LinkRef,pRC,pNodeAZ); 
	    GWDDeleteRecord (lpGWDHead,Offset); 
	    return NumPoints;
	} 
	 
	goto Next;
}
	
BOOL CheckIfPolyIsAlsoExclusion (long Ref,LOOPKEY OrigLoopKey,LOOPDATA OrigLoopData) 
{ 
	LOOPKEY		LoopKey=OrigLoopKey;
	LOOPDATA	LoopData; 
	short		pos=BT_FIRST, cond=BT_GT;
	
	LoopKey.Size -= P_TOL;
Top:
	LoopKey.LoopID = 0; 
		
	if (BT_FIND (hBTExclusion,(LPSTR)&LoopKey,pos,cond,(LPSTR)&LoopData))   
		return FALSE;  
	pos = BT_NEXT;
	cond = BT_ANY;
	if (fabs (LoopKey.Size - OrigLoopKey.Size) > P_TOL*2)
		return FALSE;
	if (LoopData.NumLinks != OrigLoopData.NumLinks || LoopData.NumSides != OrigLoopData.NumSides)
		goto Top;  
	if (!BoundsInBounds (&OrigLoopData.MinMax,&LoopData.MinMax,1))
		goto Top;
	LoopData.StoredAsInclusionRef =  Ref;
	BT_PUT (hBTExclusion,(LPSTR)&LoopKey,(LPSTR)&LoopData);
	return TRUE;
}

long HltRefConnect (long StartRef,LPSTR StartAtBeginOrEnd,LPSTR TrackLeftOrRight)
{
#define	MAXCONNECT	64
	long	nsegs=0, Offset;
	LPGWDHEADER lpGWDHead;
	HIGHLIGHTDATA	HighlightData;
	LPREFCONNECT	pRC;
	short	pos, cond, Item;  
	LPOINT	WantPoint, StopPoint;
    short	ConnectEnd[MAXCONNECT];
    LPOINT	ConnectOpPoint[MAXCONNECT];
    long	ConnectOffset[MAXCONNECT];  
    long	ConnectRef[MAXCONNECT];
    short	NumConnect,ii;
	BOOL	StartPointIsBP; 
    
	if (!OpenRefConnectionFile ("[%CONSTATFILE].gmd",&CurView->FileMNMX))
		return 0;
    lpGWDHead = (LPGWDHEADER)GlobalLock (hDBRefConnect);
    pRC = (LPREFCONNECT)&lpGWDHead->GWDData;
    if (!BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&StartRef,BT_FIRST,BT_EQ,(LPSTR)&Offset))
    {   
		FillGWDData (lpGWDHead,Offset);
    	if (*StartAtBeginOrEnd == 'B' || *StartAtBeginOrEnd == 'b')
    	{
			WantPoint = pRC->BPFile; 
			StopPoint = pRC->EPFile;
			StartPointIsBP = TRUE; 
		}   
		else 
		{
			WantPoint = pRC->EPFile;    
			StopPoint = pRC->BPFile;
			StartPointIsBP = FALSE; 
		} 
		GWDDeleteRecord (lpGWDHead,Offset); 
        PickByRefno (StartRef,0,0,-1);
    	AddToHighlightList (PickList[0].Refno,&PickList[0],TRUE);  
Next:   
		NumConnect = 0;
		ConnectKey.Point = WantPoint;
		ConnectKey.Ref = 0;  
		pos = BT_FIRST;  
		cond = BT_GT;    
        while (!BT_FIND (lpGWDHead->BTHandle[1],(LPSTR)&ConnectKey,pos,cond,(LPSTR)&Offset)) 
        {
        	pos = BT_NEXT;
        	cond = BT_ANY;
        	if (ConnectKey.Point.x != WantPoint.x || ConnectKey.Point.y != WantPoint.y)
        		break;
			FillGWDData (lpGWDHead,Offset);
        	ConnectEnd[NumConnect] = 0; 
        	ConnectOffset[NumConnect] = Offset;
        	ConnectOpPoint[NumConnect] = pRC->EPFile;
        	ConnectRef[NumConnect++]=ConnectKey.Ref;
        }
		ConnectKey.Point = WantPoint;
		ConnectKey.Ref = 0;  
		pos = BT_FIRST;  
		cond = BT_GT; 
        while (!BT_FIND (lpGWDHead->BTHandle[2],(LPSTR)&ConnectKey,pos,cond,(LPSTR)&Offset)) 
        {
        	pos = BT_NEXT;
        	cond = BT_ANY;
        	if (ConnectKey.Point.x != WantPoint.x || ConnectKey.Point.y != WantPoint.y)
        		break;
			FillGWDData (lpGWDHead,Offset);
        	ConnectEnd[NumConnect] = 1;
        	ConnectOpPoint[NumConnect] = pRC->BPFile;  
        	ConnectOffset[NumConnect] = Offset;
        	ConnectRef[NumConnect++]=ConnectKey.Ref;
        }
        if (!NumConnect)
        	goto Exit;
        if (NumConnect == 1)
        	Item = 0;  
        else
        {   
        	HANDLE	hPoints;
        	long	nPnts;
            BOOL	Reverse;
            HPDPOINT	pPoints;
            double	CurDeltaAZ, FromAZ, ToAZ;  
            short	i;
            
	        PickByRefno (StartRef,0,0,-1);
			GetPolyPoints ((LPPICKDATAHEADER)&PickList[0],StartPointIsBP,&nPnts,&hPoints, 0);
			pPoints = (HPDPOINT)GlobalLock (hPoints);
            FromAZ = getazd (&pPoints[nPnts-2],&pPoints[nPnts-1]);    
            GSSiGlobUlFree (&hPoints);
            for (i=0;i<NumConnect;i++)
            {
		        PickByRefno (ConnectRef[i],0,0,-1);
				GetPolyPoints ((LPPICKDATAHEADER)&PickList[0],ConnectEnd[i],&nPnts,&hPoints, 0);
				pPoints = (HPDPOINT)GlobalLock (hPoints);
	            ToAZ = getazd (&pPoints[0],&pPoints[1]);    
	            GSSiGlobUlFree (&hPoints); 
	            if (i)
	            {
	            	double DAZ = DeltaAZ (FromAZ, ToAZ);
	            	
	            	if (DAZ > CurDeltaAZ)
	            	{
	            		Item = i;
	            		CurDeltaAZ = DAZ;
	            	}
	            }
	            else
	            {
	            	Item = 0;
	            	CurDeltaAZ = DeltaAZ (FromAZ, ToAZ); 
	            }
            }
			//double DeltaAZ (double AZ1, double AZ2)
        }	
    	WantPoint = ConnectOpPoint[Item]; 
		StartPointIsBP = ConnectEnd[Item]; 
		GWDDeleteRecord (lpGWDHead,ConnectOffset[Item]);
		StartRef = ConnectRef[Item]; 
		if (StartRef == 118737661)
			ii=1;
        PickByRefno (StartRef,0,0,-1);
    	AddToHighlightList (PickList[0].Refno,&PickList[0],TRUE);  
    	nsegs++;
    	if (WantPoint.x != StopPoint.x || WantPoint.y != StopPoint.y)
	    	goto Next;
	}
Exit:  
	GlobalUnlock (hDBRefConnect);
	CloseRefConnectFile (); 
	return nsegs;
}  

BOOL AddLinkToFile (HFILE Fid,LPREFCONNECT pRC,LPLONG pNumLinkPoints,LPHANDLE phLinkPoints)
{   
	int			np=*pNumLinkPoints; 
	HPDPOINT	pPoints=(HPDPOINT)GlobalLock (*phLinkPoints);
	
	BigWrite (Fid,(HPSTR)&pRC->Ref,4,-1);
	BigWrite (Fid,(HPSTR)&pRC->Symbol,2,-1);
	BigWrite (Fid,(HPSTR)&np,4,-1); 
	BigWrite (Fid,(HPSTR)pPoints,(long)np * sizeof(DPOINT),-1);
	GSSiGlobUlFree (phLinkPoints);
	*pNumLinkPoints = 0;     
	return TRUE;
}

LPOINT AddRefToLink (LPREFCONNECT pRC,BOOL Reverse,LPLONG pNumLinkPoints,LPHANDLE phLinkPoints)
{   
	HANDLE	hPoints;
	long	nPnts;
    HPDPOINT	pPoints, pLinkPoints;
    double	CurDeltaAZ, FromAZ, ToAZ;  
    short	i;
    LPOINT	EndPoint;
            
    if (!PickByRefno (pRC->Ref,0,0,-1))
	{
    	MessageBox (GetFocus(),"refindex needs to be recreated",0,MB_ICONEXCLAMATION);
        BlowOut(0,0);
    }
    
	GetPolyPoints ((LPPICKDATAHEADER)&PickList[0],Reverse,&nPnts,&hPoints, 0);
	pPoints = (HPDPOINT)GlobalLock (hPoints); 
	if (*pNumLinkPoints)
	{                                                     //pPoints[1]
		pLinkPoints = (HPDPOINT)GlobalLock (*phLinkPoints); 
		if (SameDPoint (&pLinkPoints[*pNumLinkPoints-1],pPoints))
			i=1;
		else
			i=0;
		hmemmove ((HPSTR)&pLinkPoints[*pNumLinkPoints],(HPSTR)&pPoints[i],sizeof(DPOINT)*(nPnts-i));
		*pNumLinkPoints += nPnts-i;
	}
	else
	{
		*phLinkPoints = GSSiGlobAlloc (0,GMEM_MOVEABLE,sizeof(DPOINT)*(long)USHRT_MAX*16);
		pLinkPoints = (HPDPOINT)GlobalLock (*phLinkPoints); 
		hmemmove ((HPSTR)pLinkPoints,(HPSTR)pPoints,nPnts*sizeof(DPOINT)); 
		*pNumLinkPoints = nPnts;
	}
	if (Reverse)
		EndPoint = pRC->BPFile;
	else
		EndPoint = pRC->EPFile;
	GlobalUnlock (*phLinkPoints);	
    GSSiGlobUlFree (&hPoints);
    return EndPoint;
}

BOOL OutputConnectedLinks (LPSTR OutFile)
{
#define	MAXCONNECT	64
	long	nsegs=0, Offset,nRecs,nLoaded;
	LPGWDHEADER lpGWDHead;
	HIGHLIGHTDATA	HighlightData;
	LPREFCONNECT	pRC;  
	REFCONNECT		CurRC;
	short	pos, cond, Item;  
	LPOINT	WantPoint, StopPoint;
    short	ConnectEnd[MAXCONNECT];
    short	ConnectOpConnects[MAXCONNECT];
    long	ConnectOffset[MAXCONNECT];  
    long	ConnectRef[MAXCONNECT];
    short	NumConnect,ii;
	BOOL	StartPointIsBP; 
	HANDLE	hKeyFields=0; 
	BOOL	rtn, Reverse;      
	long	StartRef; 
	long	NumLinkPoints;
	HANDLE	hLinkPoints=0;
	short	IndexToUse;
	HANDLE	hSymDesc=0;
	short	NumSyms=0;    
	HFILE	FidLinkFile;
	char	LinkFile[144];
	struct {
			short	NumConnections; 
			long	Ref;
			} NumConnectKey;
	

    
	if (!OpenRefConnectionFile ("[%CONSTATFILE].gmd",&CurView->FileMNMX))
		return FALSE;
	CloseRefConnectFile (); 
	if (!GetFieldIDsFromNames ("[%CONSTATFILE].gmd",&hKeyFields,0,"BPConnections",0))
		return FALSE;
	rtn = GWDAddIndex ("[%CONSTATFILE].gmd",hKeyFields,FALSE,0);
	GSSiGlobFree (&hKeyFields);
    if (!rtn)
    	return FALSE;
	if (!GetFieldIDsFromNames ("[%CONSTATFILE].gmd",&hKeyFields,0,"EPConnections",0))
		return FALSE;
	rtn = GWDAddIndex ("[%CONSTATFILE].gmd",hKeyFields,FALSE,0);
	GSSiGlobFree (&hKeyFields);
    if (!rtn)
    	return FALSE;
	if (!OpenRefConnectionFile ("[%CONSTATFILE].gmd",&CurView->FileMNMX))
		return FALSE; 
	rtn = FALSE; 
	GSSiGetTempFileName (0,"gmc",0,(LPSTR)LinkFile); 
	FidLinkFile = GSSiOpenFile (LinkFile,0,OF_CREATE);
    lpGWDHead = (LPGWDHEADER)GlobalLock (hDBRefConnect);
    pRC = (LPREFCONNECT)&lpGWDHead->GWDData;
    NumLinkPoints = 0;
	CreateStatusWind (CurView->hWnd,1,"Create Connected Polygons");
    for (IndexToUse = 4;IndexToUse<6;IndexToUse++)
    {  
	    NumConnectKey.NumConnections = 1;
	    NumConnectKey.Ref = 0; 
		nRecs = BT_NUM_IN_INDEX (lpGWDHead->BTHandle[IndexToUse]); 
		nLoaded = 0;
		StatusWindowUpdate (0,"Step1", nRecs, nLoaded);
	    while (!BT_FIND (lpGWDHead->BTHandle[IndexToUse],(LPSTR)&NumConnectKey,BT_FIRST,BT_GT,(LPSTR)&Offset)) 
	    {   
	    	if (NumConnectKey.NumConnections != 1)
	    		break;
	   		BT_DELETE (lpGWDHead->BTHandle[IndexToUse],(LPSTR)&NumConnectKey,(LPSTR)&Offset,FALSE);
		    NumConnectKey.NumConnections = 1;
		    NumConnectKey.Ref = 0; 
			StatusWindowUpdate (0,"Step1", nRecs, nLoaded++);
	   	}
	}
    IndexToUse = 4;
	nRecs = BT_NUM_IN_INDEX (lpGWDHead->BTHandle[IndexToUse]); 
NextLink:
    NumConnectKey.NumConnections = 0;
    NumConnectKey.Ref = 0;
	nLoaded = BT_NUM_IN_INDEX (lpGWDHead->BTHandle[IndexToUse]); 
	StatusWindowUpdate (0,"Step2", nRecs, nLoaded);
    if (BT_FIND (lpGWDHead->BTHandle[IndexToUse],(LPSTR)&NumConnectKey,BT_FIRST,BT_ANY,(LPSTR)&Offset))
    {
    	if (IndexToUse == 5)
    		goto Exit;
    	IndexToUse = 5;
		nRecs = BT_NUM_IN_INDEX (lpGWDHead->BTHandle[IndexToUse]); 
    	goto NextLink;
    }   
	FillGWDData (lpGWDHead,Offset);
   	if (IndexToUse == 5) 
   		Reverse = TRUE;
   	else
		Reverse = FALSE;
	CurRC = *pRC; 
	WantPoint = AddRefToLink (pRC,Reverse,&NumLinkPoints,&hLinkPoints);
	GWDDeleteRecord (lpGWDHead,Offset); 
	if ((!Reverse && CurRC.EPConnections != 1) || (Reverse && CurRC.BPConnections != 1)) 
	{
		AddLinkToFile (FidLinkFile,&CurRC,&NumLinkPoints,&hLinkPoints);
		goto NextLink;
	}
NextRef:   
	NumConnect = 0;
	ConnectKey.Point = WantPoint;
	ConnectKey.Ref = 0;  
	pos = BT_FIRST;  
	cond = BT_GT;    
    while (!BT_FIND (lpGWDHead->BTHandle[1],(LPSTR)&ConnectKey,pos,cond,(LPSTR)&Offset)) 
    {
    	pos = BT_NEXT;
    	cond = BT_ANY;
    	if (ConnectKey.Point.x != WantPoint.x || ConnectKey.Point.y != WantPoint.y)
    		break;
		FillGWDData (lpGWDHead,Offset);
    	ConnectEnd[NumConnect] = 0; 
    	ConnectOffset[NumConnect] = Offset;
    	ConnectOpConnects[NumConnect] = pRC->EPConnections;
    	ConnectRef[NumConnect++] = ConnectKey.Ref;  
    }
	ConnectKey.Point = WantPoint;
	ConnectKey.Ref = 0;  
	pos = BT_FIRST;  
	cond = BT_GT; 
    while (!BT_FIND (lpGWDHead->BTHandle[2],(LPSTR)&ConnectKey,pos,cond,(LPSTR)&Offset)) 
    {
    	pos = BT_NEXT;
    	cond = BT_ANY;
    	if (ConnectKey.Point.x != WantPoint.x || ConnectKey.Point.y != WantPoint.y)
    		break;
		FillGWDData (lpGWDHead,Offset);
    	ConnectEnd[NumConnect] = 1;
    	ConnectOpConnects[NumConnect] = pRC->BPConnections;
    	ConnectOffset[NumConnect] = Offset;
    	ConnectRef[NumConnect++]=ConnectKey.Ref;
    }
    if (NumConnect == 1)
    {
		FillGWDData (lpGWDHead,ConnectOffset[0]);
		WantPoint = AddRefToLink (pRC,ConnectEnd[0],&NumLinkPoints,&hLinkPoints);
		GWDDeleteRecord (lpGWDHead,ConnectOffset[0]); 
		if (ConnectOpConnects[0] == 1)
			goto NextRef;
	}
	AddLinkToFile (FidLinkFile,&CurRC,&NumLinkPoints,&hLinkPoints);
    goto NextLink;
Exit:  
	GlobalUnlock (hDBRefConnect);
	CloseRefConnectFile ();
	nRecs = GSSillseek (FidLinkFile,0,2); 
	GSSillseek (FidLinkFile,0,0);
    if (CreateNewMap (OutFile,&CurView->WBounds,0,0,0,0,0,0,TRUE))
    {
		int	np;
		HANDLE	hPoints;
		HPDPOINT	pPoints; 
		long	Refno;
		short	SymNum;
		
    	rtn = TRUE;
    	while (BigRead (FidLinkFile,(HPSTR)&Refno,4) == 4)
    	{   
    		BigRead (FidLinkFile,(HPSTR)&SymNum,2);
    		BigRead (FidLinkFile,(HPSTR)&np,4); 
    		hPoints = GSSiGlobAlloc (0,GMEM_MOVEABLE,(long)np*sizeof(DPOINT)); 
    		pPoints = (HPDPOINT)GlobalLock (hPoints);
    		BigRead (FidLinkFile,(HPSTR)pPoints,(long)np*sizeof(DPOINT));  
    		GlobalUnlock (hPoints);
			AddPolyToMap (1,&np, &hPoints,1,Refno,0,2,SymNum,0,0,0,-1,-1,-1,0,0,0,0,TRUE,0);
			GSSiGlobFree (&hPoints);
		    AddToSymList (SymNum,&NumSyms,&hSymDesc);
			StatusWindowUpdate (0,"Step2", nRecs, GSSillseek (FidLinkFile,0,1));
		}
		AddSymToMap (NumSyms,hSymDesc,0,0); 
	    DestroySymList (&NumSyms,&hSymDesc);
	    CloseMap(TRUE);  
    }
	GSSiClose2 (&FidLinkFile);
	GSSiRemove (LinkFile);
	DestroyStatusWindow(0); 
	return rtn;
} 

BOOL OutputRefConnect (void)
{
#define	MAXLOOPS	4096
	short	st, BPConnect,EPConnect;
	LPGWDHEADER lpGWDHead;
	HIGHLIGHTDATA	HighlightData;
	LPREFCONNECT	pRC;
	short	pos, pos2, cond,len,ii, SnapItem, FixItem, SaveMaxPick;  
	long	Refno, Offset, NearOpRef, NumRemoved=0, TotLoops, MinRef, nlast;  
	HCURSOR	hcurSave;   
	POINT	Point;  
	LINKDATA	LinkData; 
	int		NumLoopPoints[MAXLOOPS], NumLoops;
	HANDLE	hLoopPoints[MAXLOOPS];  
	LOOPREC	LoopRec;    
	static	long	debuglinkref=184,debugarearef=2, debugloops=146, debuglink=185;   
	short	Pass=0, NumConnect, AtEnd, NewAreaSymbol, stuff[128];
	int		NumLinkPoints;
	long	NewLinkPoints;
	long	LinkRef=0, LastRef,n=0,NumLinks,StartRef, MinLinkRef, LoopID=1, nRecs, nLoaded;
	LPOINT	ConnectPointFile,OpEndFile;
	DPOINT	ConnectPointBase,OpEndBase;
	HANDLE	hLinkPoints = GSSiGlobAlloc ( 578,GMEM_MOVEABLE,(long)USHRT_MAX*16*sizeof(DPOINT));
	HPDPOINT	pLinkPoints;    
	OFSTRUCTGM	OFStruct;
    BTVARDESC  Vars;  
    BOOL	JustDebugRec=FALSE,NoIslands=FALSE, StoreIslands=GetGlobalBVal2 ("[%STOREISLANDS]",TRUE); 
    int		PromptForUnattached=GetGlobalLVal2 ("[%LOADUNATTACHEDAREAS]",0);
    char	mess[128]; 
	long	AreaRef=1;     
	MNMXCORD	FileBounds;
	COLORREF	AreaColor;
	short	NumLinkSyms, LinkSyms[64];
	static	long	debugref=1061980080;
        
    DBoundsInit (&FileBounds);
	CreateStatusWind (CurView->hWnd,1,"Automatic Area Creation");
	GetGlobalCVal("[%NEW_AREA_SYM]",mess,"SOILAREA");
	NewAreaSymbol = GetDictSymbolNumber (mess);
	AddNodeInit (&CurView->FileMNMX);
	OpenRefConnectionFile ("[%CONSTATFILE].gmd",&CurView->FileMNMX);
    lpGWDHead = (LPGWDHEADER)GlobalLock (hDBRefConnect);
    pRC = (LPREFCONNECT)&lpGWDHead->GWDData; 
    LastRef = LONG_MAX;   
    MinRef = LONG_MIN;
	nRecs = BT_NUM_IN_INDEX (lpGWDHead->BTHandle[0]); 
	nLoaded = 0;
	StatusWindowUpdate (0,"Step1: Building Link Table", nRecs, nLoaded);
NextLink:
    pos = BT_FIRST; 
    cond = BT_GT;
    NumLinks = 0; 
    Refno = MinRef;
	while (!BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&Refno,pos,cond,(LPSTR)&Offset) && StatusWindowUpdate (0,0, nRecs,(nRecs - BT_NUM_IN_INDEX (lpGWDHead->BTHandle[0])))) 
	{    
		pos = BT_NEXT;  
		cond = BT_ANY;  
		
		NumLinks++;
		if (Refno == LastRef)
			ii=1;    
		LastRef = Refno;
		if (Refno == debugref)
			ii=1;    
		n++;
		if (!(n%1000))
			ii=1;
   		len = FillGWDData (lpGWDHead,Offset); 
   		StartRef = LONG_MAX; 
   		switch (Pass)
   		{            
   			case 0:
   			NumConnect = pRC->BPConnections;
   			ConnectPointFile = pRC->BPFile;
   			ConnectPointBase = pRC->BPBase;
   			OpEndBase = pRC->EPBase;
   			OpEndFile = pRC->EPFile;
   			AtEnd = 1;
   			break; 
       			
   			case 1:
   			NumConnect = pRC->EPConnections;
   			ConnectPointFile = pRC->EPFile;
   			ConnectPointBase = pRC->EPBase;
   			OpEndBase = pRC->BPBase;
   			OpEndFile = pRC->BPFile;
   			AtEnd = 2; 
   			break; 
       			
   			case 2: 
   			StartRef = Refno;
   			NumConnect = 2;
   			ConnectPointFile = pRC->BPFile;
   			ConnectPointBase = pRC->BPBase;
   			OpEndBase = pRC->EPBase;
   			OpEndFile = pRC->EPFile;
   			AtEnd = 1;
   			break;
   		}
   		if (NumConnect > 1)
   		{   
       			
   			LinkRef++;
   			if (LinkRef == debuglinkref)
   				ii=1;
   			LinkData.FromNode = AddNode (2,AtEnd,ConnectPointBase,LinkRef,pRC,&LinkData.FromAZ);  
   			NumLinkPoints = 1;
   			pLinkPoints = (HPDPOINT)GlobalLock (hLinkPoints);
   			*pLinkPoints++=ConnectPointBase; 
   			NewLinkPoints = TraceLink (USHRT_MAX*16-1,Refno,LinkRef,StartRef,lpGWDHead,AtEnd,Offset,&pLinkPoints,&LinkData.ToNode,&LinkData.ToAZ,&NumLinkSyms,LinkSyms,FALSE);
   			if (NewLinkPoints < 0)
   			{
				GlobalUnlock (hDBRefConnect);
				GlobalUnlock (hLinkPoints);
				CloseRefConnectFile (); 
				goto Exit;
   			}
   			NumLinkPoints += NewLinkPoints;
   			GlobalUnlock (hLinkPoints);
   			pLinkPoints = (HPDPOINT)GlobalLock (hLinkPoints);  
   			LinkData.Offset = GSSillseek (FidLinks,0,2); 
   			LinkData.DeltaAZ = GetLinkDeltaAZ (NumLinkPoints,pLinkPoints);
   			LinkData.TrackedLeft=LinkData.TrackedRight=0;
   			BT_PUT (hBTLinks,(LPSTR)&LinkRef,(LPSTR)&LinkData);
   			BigWrite (FidLinks,(HPSTR)&LinkRef,4,-1);
   			BigWrite (FidLinks,(HPSTR)&NumLinkSyms,2,-1);
   			BigWrite (FidLinks,(HPSTR)LinkSyms,2*NumLinkSyms,-1);
   			BigWrite (FidLinks,(HPSTR)&NumLinkPoints,4,-1);
			BigWrite (FidLinks,(HPSTR)pLinkPoints,(long)NumLinkPoints * sizeof(DPOINT),-1);       			
   			GlobalUnlock (hLinkPoints);
       		goto NextLink;
   		}
   		else
   			MinRef = Refno;
   	}                 
   	if (!Pass)
   	{
   		Pass = 1;   
   		MinRef = LONG_MIN;
   		goto NextLink;
   	} 
   	else if (Pass == 1)
   	{
   		Pass = 2;  
   		MinRef = LONG_MIN;
   		goto NextLink;
   	}
   	else if (NumLinks)
   		ii=1;
	GlobalUnlock (hDBRefConnect);
	CloseRefConnectFile ();
	    
    {
    	short pos, numlinks; 
    	long	TotNodes,BadNodes;
    	LPOINT	LastNode;
		    
	    pos = BT_FIRST; 
	    LastNode.x = LastNode.y = LONG_MAX;
	    numlinks = 2; 
	    TotNodes=BadNodes=0;
	    while (!BT_FIND (hNodes,(LPSTR)&NodesKey,pos,BT_ANY,(LPSTR)&NodesData))
	    {
	    	pos = BT_NEXT; 
	    	if (NodesKey.NodeID.x != LastNode.x || NodesKey.NodeID.y != LastNode.y)
	    	{
	    		if (numlinks < 2) 
	    		{
	    			BadNodes++; 
	    			DisplayMarker (NodesData.Point,4,0,0,0,0,FALSE,FALSE,0,0,0,0,0);
	    		}
	    		numlinks = 1;
	    		TotNodes++;
	    		LastNode = NodesKey.NodeID;
	    	} 
	    	else
	    		numlinks++;
	    }
	    ii=1;  
	}

    if (GetGlobalBVal ("[%CREATELINKMAP]")) 
    {   
		HANDLE	hSymDesc=0;
		short	NumSyms=0;  
		char	Name[MAX_PATH];
		int		TotLen = GSSillseek (FidLinks,0,2);

   		DumpLinkData ("linkdata.gmd");
        GetGlobalCVal ("[%LINKMAP]",Name,"testlink.plt");
	    CreateNewMap (Name,&CurView->WBounds,0,0,0,0,0,0,TRUE);
	    GSSillseek (FidLinks,128,0);
	    while (BigRead (FidLinks,(HPSTR)&LinkRef,4)==4 && StatusWindowUpdate (0,"Dumping Link Map", TotLen, GSSillseek (FidLinks,0,1)))
	    {
	    	short NewLineSymbol=3, rtn;
	    	long	NewRefno=1; 
	    	short	LinkSyms[64], NumLinkSyms;
		        
			BigRead (FidLinks,(HPSTR)&NumLinkSyms,2);
			BigRead (FidLinks,(HPSTR)LinkSyms,2*NumLinkSyms);
			BigRead (FidLinks,(HPSTR)&NumLinkPoints,4);
   			pLinkPoints = (HPDPOINT)GlobalLock (hLinkPoints);  
			BigRead (FidLinks,(HPSTR)pLinkPoints,(long)NumLinkPoints * sizeof(DPOINT)); 
			GlobalUnlock (hLinkPoints);      			
			AddToSymList (LinkSyms[0],&NumSyms,&hSymDesc); 
			rtn=AddPolyToMap (1,&NumLinkPoints, &hLinkPoints,1,LinkRef,0,2,LinkSyms[0],0,0,0,-1,-1,-1,0,0,0,0,TRUE,0);
	    }
	    CloseMap(TRUE);  
		AddSymToMap (NumSyms,hSymDesc,0,0); 
        DestroySymList (&NumSyms,&hSymDesc);
	}

    TotLoops=0;
    MinLinkRef = LinkRef = LONG_MIN;
	       
	nRecs = BT_NUM_IN_INDEX (hBTLinks); 
	nLoaded = 0;
	StatusWindowUpdate (0,"Step 2: Building Loop Table", nRecs, nLoaded);
	    
	while (ContinueProcessing && !BT_FIND (hBTLinks,(LPSTR)&LinkRef,BT_FIRST,BT_GT,(LPSTR)&LinkData))
	{   
		short	Track, AtEnd;			   
		double	TotAZ, StartAZ, AzChange, InAZ;  
		long	AtLink; 
		HANDLE	hBT; 
			
		LPOINT	StartNode, AtNode;
				                                   
		if (!LinkData.TrackedLeft) 
		{
			LinkData.TrackedLeft = 1;
			Track = TRACK_LEFT;  
		}
		else if (!LinkData.TrackedRight) 
		{
			LinkData.TrackedRight = 1;
			Track = TRACK_RIGHT;  
		}
		else
		{
			MinLinkRef = LinkRef;
			StatusWindowUpdate (0,0, nRecs, ++nLoaded);
			goto FindNextLink;
		}  
		TotLoops++;
		sprintf (mess,"%ld loops",TotLoops);  
		if (TotLoops == debugloops)
			ii=1;
		//SetWindowText (hWndMain,mess);
		StartNode = LinkData.FromNode;
		InAZ = StartAZ = LinkData.FromAZ; 
		AtNode = LinkData.ToNode;
		AtLink = LinkRef;
		AtEnd = 2;
		LoopData.Offset = GSSillseek (FidLoop,0,2);
		LoopRec.LinkRef = LinkRef;
		LoopRec.Dir = 1;
		LoopData.NumLinks = 1;
		BigWrite (FidLoop,(HPSTR)&LoopRec,sizeof(LoopRec),-1); 
		TotAZ = LinkData.DeltaAZ; 
		BT_PUT (hBTLinks,(LPSTR)&LinkRef,(LPSTR)&LinkData);
		while (AtNode.x != StartNode.x || AtNode.y != StartNode.y)
		{   
			if (AtLink == debuglink)
				ii=1;
			if (!GetNextLink (&AtNode,&AtLink,&AtEnd,&TotAZ,Track,&InAZ))
			{
//			    	sprintf (mess,"Problem at link %ld end %i",AtLink,AtEnd);
//			    	MessageBox (GetFocus(),mess,0,MB_ICONEXCLAMATION);
// valid if totally enclosed loop with no nodes (as in lake contours)
				goto FindNextLink;
			}
			LoopRec.LinkRef = AtLink;  
			LoopRec.Dir = 3-AtEnd;  
			LoopData.NumLinks++;
			BigWrite (FidLoop,(HPSTR)&LoopRec,sizeof(LoopRec),-1);
		}
		AzChange = DeltaAZ (InAZ,StartAZ); 
		TotAZ += AzChange;
		if (Track == TRACK_LEFT && TotAZ > 0)
		{
			LoopData.Reverse = TRUE;
			hBT = hBTInclusion;  
		}
		else if (Track == TRACK_RIGHT && TotAZ < 0) 
		{
			LoopData.Reverse = FALSE;
			hBT = hBTInclusion;  
		}
		else if (Track == TRACK_LEFT && TotAZ < 0)
		{
			LoopData.Reverse = TRUE;
			hBT = hBTExclusion;  
		}
		else if (Track == TRACK_RIGHT && TotAZ > 0) 
		{
			LoopData.Reverse = FALSE;
			hBT = hBTExclusion;  
		}
        if (CalculateLoopSize (FidLoop,LoopData.Offset,LoopData.NumLinks,LoopData.Reverse,
        					   &LoopData.MinMax,&LoopKey.Size,
        					   &LoopData.NumSides,&LoopData.Point1))
        {
	        LoopKey.LoopID = LoopID++; 
	        LoopData.StoredAsInclusionRef = LONG_MAX;
			BT_PUT (hBT,(LPSTR)&LoopKey,(LPSTR)&LoopData); 
			AddMinMaxD (&FileBounds, &LoopData.MinMax);
		}
FindNextLink:
		LinkRef = MinLinkRef;
	} 
		
	SetViewport (*pCommandViewport);
	nRecs = BT_NUM_IN_INDEX (hBTInclusion); 
	nLoaded = 0;
	StatusWindowUpdate (0,"Final Step: Creating Polygons", nRecs, nLoaded);
    CreateNewMap (EditName,&FileBounds,0,0,0,0,0,0,TRUE);
	EditBounds = CurView->FileMNMX;
    _fstrcpy (PltName,EditName);   
	while (ContinueProcessing && !BT_FIND (hBTInclusion,(LPSTR)&LoopKey,BT_FIRST,BT_ANY,(LPSTR)&LoopData))
	{   
		double	MaxExclusionSize = LoopKey.Size * 0.999999; 
		short	pos2;    
		LOOPKEY		OrigLoopKey=LoopKey;
		LOOPDATA	OrigLoopData=LoopData;	
					
		NumLinkSyms=0;
		AreaRef = LoopKey.LoopID;
		if (AreaRef == debugarearef)
			ii=1;   
		sprintf (mess,"Storing area %ld",AreaRef);
		//SetWindowText (hWndMain,mess);
		NumLoopPoints[0]=LoopData.NumSides;
		hLoopPoints[0] = LoadLoopSides (&LoopData.NumSides,LoopData.NumLinks,LoopData.Offset,LoopData.Reverse,&NumLinkSyms,LinkSyms);
		NumLoops = 1; 
		BT_DELETE (hBTInclusion,(LPSTR)&LoopKey,(LPSTR)&LoopData,FALSE);
		if (NoIslands)
			MaxExclusionSize = 0;
		pos2 = BT_FIRST;
		while (ContinueProcessing && !BT_FIND (hBTExclusion,(LPSTR)&LoopKey,pos2,BT_ANY,(LPSTR)&LoopData))
		{   
			pos2 = BT_NEXT;
			if (LoopKey.Size >= MaxExclusionSize)
				goto NoMoreExclusions; 
			if (BoundsInBounds (&OrigLoopData.MinMax,&LoopData.MinMax,1))
			{
				if (PointInLoop (LoopData.Point1,(long)NumLoopPoints[0]-1,hLoopPoints[0]))
				{   
					if (NumLoops >= MAXLOOPS)
					{
				 		MessageBox(GetFocus(), "Too many islands in area", 0,MB_ICONEXCLAMATION);
						goto NoMoreExclusions; 
					}
					NumLoopPoints[NumLoops]=LoopData.NumSides;
					hLoopPoints[NumLoops++] = LoadLoopSides (&LoopData.NumSides,LoopData.NumLinks,LoopData.Offset,LoopData.Reverse,&NumLinkSyms,LinkSyms);
					BT_DELETE (hBTExclusion,(LPSTR)&LoopKey,(LPSTR)&LoopData,FALSE); 
					if (!StoreIslands && LoopData.StoredAsInclusionRef != LONG_MAX)
					{
						if (PickByRefno(LoopData.StoredAsInclusionRef,0,0,GetPickFile (-1)))
						{
							DeletePickedItem (0,12,92);
						}
					}
				} 
			}
		}
NoMoreExclusions:
        AreaColor = -1; 
		stuff[0]=0;
		if (GetGlobalBVal2 ("[%STORESIDESYMS]",FALSE))
		{   
			short	lcmdstring;
			HANDLE	hCmd=GSSiGlobAlloc ( 579,GMEM_MOVEABLE,256);
			LPSTR	cmd=GlobalLock (hCmd); 
			short	i; 
			char	SymName[34];
				
			sprintf (cmd,"[%%NUMSIDESYMS]=%i",NumLinkSyms);
			for (i=0;i<NumLinkSyms;i++)
			{
				if (GetDictSymName (LinkSyms[i],SymName))
					sprintf (_fstrchr(cmd,0),";[%%SIDESYM(%i)]=%s",i+1,SymName);
			}
			lcmdstring = _fstrlen (cmd);
			lcmdstring += lcmdstring%2+2; 
			stuff[1]=40;
			stuff[2]=lcmdstring;
			_fstrncpy ((LPSTR)&stuff[3],cmd,lcmdstring);
			stuff[0]+=2+2+lcmdstring; 
			GSSiGlobUlFree (&hCmd);
		}
        if (!GetGlobalBVal2 ("[%REFISLOOPID]",FALSE))
			AreaRef = GetNewRefno(PltName,0,0,0,0);
		if (!StoreIslands)
			CheckIfPolyIsAlsoExclusion (AreaRef,OrigLoopKey,OrigLoopData); 
		SetViewport (*pCommandViewport);
		if (!JustDebugRec || AreaRef == debugarearef)
			AddPolyToMap (NumLoops,NumLoopPoints,hLoopPoints,0,AreaRef,0,-1,NewAreaSymbol,stuff,0,0,AreaColor,-1,-1,0,0,0,0,TRUE,0);
		while (NumLoops--)
			GSSiGlobFree (&hLoopPoints[NumLoops]);
		StatusWindowUpdate (0,0, nRecs, ++nLoaded);
	}
	SetContinueProcessing ( TRUE); 
		
	{   
		HANDLE	hSymDesc=0;
		short	NumSyms=0;
				
		AddToSymList (NewAreaSymbol,&NumSyms,&hSymDesc); 
		AddSymToMap (NumSyms,hSymDesc,0,0); 
        DestroySymList (&NumSyms,&hSymDesc);
	}
	nlast = BT_NUM_IN_INDEX (hBTExclusion); 
	if (PromptForUnattached && nlast > 1)
	{   
		char	mess[128];
			
		sprintf (mess,"There are %ld unattached exclusions\r\nShould these be added to the map?",nlast-1);
        if (PromptForUnattached > 1 || MessageBox( GetFocus(),mess,0,MB_YESNO|MB_ICONQUESTION)==IDYES) 
		{
	    	//NewAreaSymbol = GetDictSymbolNumber ("CENTRACT");
			AreaRef=1000; 
			pos = BT_FIRST;
			n=1;
			while (!BT_FIND (hBTExclusion,(LPSTR)&LoopKey,pos,BT_ANY,(LPSTR)&LoopData))
			{   
				pos = BT_NEXT;
	            //AreaColor = 0;
				NumLoops=0; 
				hLoopPoints[NumLoops++] = LoadLoopSides (&LoopData.NumSides,LoopData.NumLinks,LoopData.Offset,ReverseBOOL(LoopData.Reverse),&NumLinkSyms,LinkSyms);
				NumLoopPoints[0]=LoopData.NumSides;   
				if (!GetGlobalBVal2 ("[%REFISLOOPID]",FALSE))
					AreaRef = GetNewRefno(PltName,0,0,0,0);
				if (n++ < nlast)
					//AddPolyToMap (NumLoops,NumLoopPoints,hLoopPoints,0,LoopKey.LoopID,0,2,NewAreaSymbol,0,0,0,-1,-1,-1,0,0,0,0,TRUE,0);
					AddPolyToMap (NumLoops,NumLoopPoints,hLoopPoints,0,AreaRef,0,-1,NewAreaSymbol,0,0,0,AreaColor,-1,-1,0,0,0,0,TRUE,0);

				while (NumLoops--)
					GSSiGlobFree (&hLoopPoints[NumLoops]);
			}
			{   
				HANDLE	hSymDesc=0;
				short	NumSyms=0;
						
				AddToSymList (NewAreaSymbol,&NumSyms,&hSymDesc); 
				AddSymToMap (NumSyms,hSymDesc,0,0); 
	            DestroySymList (&NumSyms,&hSymDesc);
			}
		}
	}
	CloseMap(FALSE);
Exit:	
    GSSiGlobFree (&hLinkPoints);
	CloseNodes ();
	DestroyStatusWindow(0); 
	return TRUE;
}



BOOL GetNextLink (LPLPOINT AtNode,LPLONG AtLink,LPSHORT AtEnd,LPDOUBLE TotAZ,short Track,LPDOUBLE pAtAZ)
{   
	NODESDATA	LastLink, NextLink, FirstLink, PriorLink, GotLink; 
	double		LastAZ=*pAtAZ, NextAZ, FirstAZ, AtAZ, GotAZ, PriorAZ, AzChange, InAZ, OutAZ;
	short		PastAt = 0, pos, cond,ii;
	BOOL		First=TRUE;
	LINKDATA	LinkData; 
	
	LastLink.WhichEnd = PriorLink.WhichEnd = NextLink.WhichEnd = FirstLink.WhichEnd =-1;	
	NodesKey.AZ = -10; 
	NodesKey.NodeID = *AtNode;   
	pos = BT_FIRST;
	cond = BT_GT; 
	
	while (!BT_FIND (hNodes,(LPSTR)&NodesKey,pos,cond,(LPSTR)&NodesData))
	{   
		if (NodesKey.NodeID.x != AtNode->x || NodesKey.NodeID.y != AtNode->y)
		{
			if (Track == TRACK_LEFT) 
			{
				GotLink = LastLink;
				GotAZ = LastAZ;
			}
			else
			{
				GotLink = FirstLink;
				GotAZ = FirstAZ;
			}
			goto Exit;
		}
		if (NodesData.LinkID == *AtLink && NodesData.WhichEnd == *AtEnd) 
		{   
			AtAZ = NodesKey.AZ; 
			InAZ = LTWOPI (AtAZ + PY);
			PastAt = 1;
			if (Track == TRACK_LEFT && PriorLink.WhichEnd > 0)
			{
				GotLink = PriorLink;
				GotAZ = PriorAZ;
				goto Exit;	
			}
		}
		else if (PastAt==1)
		{   
			if (Track == TRACK_RIGHT)
			{
				GotLink = NodesData;
				GotAZ = NodesKey.AZ;
				goto Exit;	
			}
			NextAZ = LastAZ = NodesKey.AZ;
			NextLink = LastLink = NodesData;
			PastAt++; 
		}
		else if (PastAt) 
		{
			LastAZ = NodesKey.AZ;
			LastLink = NodesData; 
		}
		else
		{ 
			PriorAZ = NodesKey.AZ;
			PriorLink = NodesData;
		} 
		if (First)
		{
			pos = BT_NEXT;
			cond = BT_ANY;
			First = FALSE;
			if (!PastAt)
			{	
				FirstAZ = NodesKey.AZ;
				FirstLink = NodesData;
			}
		}
		
	} 
	if (Track == TRACK_LEFT) 
	{
		GotLink = LastLink;
		GotAZ = LastAZ;
	}
	else
	{
		GotLink = FirstLink;
		GotAZ = FirstAZ;
	}
Exit:   
	if (!PastAt)
		return FALSE;   
		
	DisplayMarker (GotLink.Point,3,0,0,0,0,FALSE,FALSE,0,0,0,0,0);			
	*AtLink = GotLink.LinkID;
	AzChange = DeltaAZ (InAZ,GotAZ); 
	*TotAZ += AzChange;
	BT_FIND (hBTLinks,(LPSTR)AtLink,BT_FIRST,BT_EQ,(LPSTR)&LinkData);
	if (GotLink.WhichEnd == 1)
	{
		if (Track == TRACK_LEFT)
			LinkData.TrackedLeft = 1;
		else
			LinkData.TrackedRight = 1;
		*AtNode = LinkData.ToNode;
		*AtEnd = 2;  
		AzChange = LinkData.DeltaAZ;
		*pAtAZ = LTWOPI (LinkData.ToAZ + PY);
	}
	else
	{
		if (Track == TRACK_LEFT)
			LinkData.TrackedRight = 1;
		else
			LinkData.TrackedLeft = 1;
		*AtNode = LinkData.FromNode;
		*AtEnd = 1;  
		AzChange = -LinkData.DeltaAZ; 
		*pAtAZ = LTWOPI (LinkData.FromAZ + PY);
	}
	*TotAZ += AzChange;
	BT_PUT (hBTLinks,(LPSTR)AtLink,(LPSTR)&LinkData);
	
	return TRUE;
}

LPOINT AddNode (int InOut,int AtEnd,DPOINT ConnectPointBase,long LinkRef,LPREFCONNECT pRC,LPDOUBLE pNodeAZ)
{   
	static	long	debuglinkref=2212;
	short	ii;
	
	if (LinkRef == debuglinkref)
		ii=1;   
	RoundToPTOL (&ConnectPointBase);
	NodesKey.NodeID.x = IDNINT ((ConnectPointBase.x - AddNodeBias.x) * AddNodeFactor);
	NodesKey.NodeID.y = IDNINT ((ConnectPointBase.y - AddNodeBias.y) * AddNodeFactor);
	if (pRC->NumPoints > 2)
	{   
		LPTHEME	pTheme;
		
		if (PickByRefno(pRC->Ref,0,0,-101))
		{
		    SetConfig (PickList[0].ConfigID);
		    SetViewport (PickList[0].ViewID);
			pTheme = AddTheme (GF_SAVEPOLY_THEME);
			CurView->PassID = 4;
			ProcessSelectedTheme = CurView->NumThemes;
			ProcessPickedItem (0,FALSE);        		
			ProcessSelectedTheme = 0;
			DeleteTheme (pTheme);
	    	while (GetSavedPolys ())
	   		if (hSavePoly)
			{   LPMNMXCORD lpRect;
				short	nPnts; 
				HPDPOINT	lpDpoint;
				DPOINT	Point1, Point2;
	    		    
	            nPnts = nSavePoly; 
	            lpRect = (LPMNMXCORD) GlobalLock (hSavePoly);
	            lpRect++;
	            lpDpoint = (HPDPOINT) lpRect; 
	            if (AtEnd == 1)
	            {
	            	Point1 = *lpDpoint++; 
	            	Point2 = *lpDpoint; 
	            }
	            else 
	            {
	            	lpDpoint += (nPnts-2);
	            	Point2 = *lpDpoint++; 
	            	Point1 = *lpDpoint;
	            }
	           	NodesKey.AZ = getazd (&Point1,&Point2);
				GlobalUnlock(hSavePoly);
	         } 
	    }
	}
	else if (AtEnd == 2)
		NodesKey.AZ = getazd (&pRC->EPBase,&pRC->BPBase);
	else
		NodesKey.AZ = getazd (&pRC->BPBase,&pRC->EPBase); 
	*pNodeAZ = NodesKey.AZ; 
	NodesData.LinkID = LinkRef;
	NodesData.Point = ConnectPointBase;
	NodesData.WhichEnd = 3 - InOut;
	BT_PUT (hNodes,(LPSTR)&NodesKey,(LPSTR)&NodesData);
	return NodesKey.NodeID;
}    

void CloseNodes (void)
{   
	char	File[MAX_PATH+1];
	
	BT_CLOSEANDDELETE (&hNodes);
	GSSillseek (FidLoop,0,0);
	BigRead (FidLoop,File,MAX_PATH);
	GSSiClose2 (&FidLoop);
	GSSiRemove (File); 
	GSSillseek (FidLinks,0,0);
	BigRead (FidLinks,File,MAX_PATH);
	GSSiClose2 (&FidLinks);
	GSSiRemove (File); 
	BT_CLOSEANDDELETE (&hBTLinks);
	BT_CLOSEANDDELETE (&hBTInclusion);
	BT_CLOSEANDDELETE (&hBTExclusion);
	return;
}

BOOL AddNodeInit (LPMNMXCORD Rect)
{   
	double	MaxDist;
	short	ld;
	BTVARDESC	BTVar[3];
	char	File[MAX_PATH];  
	OFSTRUCTGM	OFStruct;

	MaxDist = max (Rect->xmx - Rect->xmn,Rect->ymx - Rect->ymn); 
	AddNodeBias.x =  (Rect->xmx + Rect->xmn)/2;
	AddNodeBias.y =  (Rect->ymx + Rect->ymn)/2;   
	RoundToPTOL (&AddNodeBias);
	ld = log10 (MaxDist);
	
	AddNodeFactor = pow (10,8 - ld);
	GSSiGetTempFileName (0,"gmn",0,(LPSTR)File);
	BTVar[0].BT_VARTYP=BT_INTEGER;
	BTVar[0].BT_VARLEN=4;
	BTVar[0].BT_VAROFF=0;
	BTVar[1].BT_VARTYP=BT_INTEGER;
	BTVar[1].BT_VARLEN=4;
	BTVar[1].BT_VAROFF=4;
	BTVar[2].BT_VARTYP=BT_REAL;
	BTVar[2].BT_VARLEN=8;
	BTVar[2].BT_VAROFF=8;
	BT_CREATE (File,sizeof(NodesData), FALSE, 3, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
	hNodes= BT_OPEN (File, 0, BT_WRITE, 0); 
	
	GSSiGetTempFileName (0,"gml",0,(LPSTR)File); 
	FidLoop = GSSiOpenFile (File,&OFStruct,OF_CREATE);
	BigWrite (FidLoop,(HPSTR)File,128,-1);
	GSSiGetTempFileName (0,"gml",0,(LPSTR)File); 
	FidLinks = GSSiOpenFile (File,&OFStruct,OF_CREATE);
	BigWrite (FidLinks,(HPSTR)File,128,-1);
	BTVar[0].BT_VARTYP=BT_INTEGER;
	BTVar[0].BT_VARLEN=4;
	BTVar[0].BT_VAROFF=0;
	GSSiGetTempFileName (0,"gml",0,(LPSTR)File); 
	BT_CREATE (File, sizeof(LINKDATA), FALSE, 1, 1,BTVar,FALSE, 0, 0, FALSE); 
	hBTLinks = BT_OPEN (File,0, BT_WRITE, 0);
    
	GSSiGetTempFileName (0,"gmb",0,(LPSTR)File); 
	BTVar[0].BT_VARTYP=BT_REAL;
	BTVar[0].BT_VARLEN=8;
	BTVar[0].BT_VAROFF=0;
	BTVar[1].BT_VARTYP=BT_INTEGER;
	BTVar[1].BT_VARLEN=4;
	BTVar[1].BT_VAROFF=8;
	BT_CREATE (File,sizeof(LOOPDATA), FALSE, 2, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
	hBTInclusion = BT_OPEN (File, 0, BT_WRITE, 0); 
    
	GSSiGetTempFileName (0,"gmb",0,(LPSTR)File); 
	BT_CREATE (File,sizeof(LOOPDATA), FALSE, 2, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
	hBTExclusion = BT_OPEN (File, 0, BT_WRITE, 0); 
    
	return TRUE;
}  

BOOL DisplayBadArea (void)
{   
	HFILE		FidBad;
	HPDPOINT	pPoint; 
	char		BadAreaFile[MAX_PATH];  
	UINT		np;  
	HANDLE		hLoop; 
	short		i,idesc=GetDictSymbolNumber ("H01");  
	HPEN		hPen,hOldPen;
			
	GetGlobalCVal ("[%BADAREAFILE]",BadAreaFile,"[%SYSTEMPDIR]badarea.bin");
	FidBad = GSSiOpenFile (BadAreaFile,0,OF_READ);
	if (FidBad == HFILE_ERROR)
		return FALSE;
    SelectVisList (FALSE);  
    HaveVarFillColor=ItemIsHighlighted=0;
	SetDisplayMode (CurView->hDC, GF_TEXTMODE);
	GSSiDeleteObject(&CurView->hRgn);
	CurView->hRgn = CreateVPRgn(FALSE,FALSE);
	i=SelectClipRgn (CurView->hDC,CurView->hRgn);
	GSSiDeleteObject(&CurView->hRgn); 
	BigRead (FidBad,(HPSTR)&np,2);
	hLoop = GSSiGlobAlloc (0,GMEM_MOVEABLE,(long)np*(long)sizeof(DPOINT));
	pPoint = (HPDPOINT)GlobalLock (hLoop);
	BigRead (FidBad,(LPSTR)pPoint,(long)np*(long)sizeof(DPOINT));  
	hPen = CreatePen(PS_SOLID,0,RGB(0,0,255));
	hOldPen = SelectObject (CurView->hDC,hPen);
	GWPolylineD (CurView->hDC,pPoint,np,idesc);
	GSSiGlobUlFree (&hLoop);  
	BigRead (FidBad,(HPSTR)&np,2);
	hLoop = GSSiGlobAlloc (0,GMEM_MOVEABLE,(long)np*(long)sizeof(DPOINT));
	pPoint = (HPDPOINT)GlobalLock (hLoop);
	BigRead (FidBad,(LPSTR)pPoint,(long)np*(long)sizeof(DPOINT));
	GWPolylineD (CurView->hDC,pPoint,np,idesc);
	GSSiGlobUlFree (&hLoop);  
	GSSiClose2 (&FidBad); 
	SelectObject (CurView->hDC,hOldPen);
	GSSiDeleteObject (&hPen); 
	return TRUE;
} 

HANDLE LoadLoopSides (LPINT pNumSides,int NumLinks,long Offset,BOOL Reverse,LPSHORT pNumSym,LPSHORT pSymbols)
{   
	HANDLE	hLoop=0, hLoop2=0;
	HPDPOINT	pPoints, pPoints2;
	LOOPREC	LoopRec; 
	LINKDATA	LinkData;
	int			np, np2;
	short		nSyms, Symbols[64];
	UINT	i;
	long	MemLen=(long)*pNumSides*(long)sizeof(DPOINT);
	
	if (*pNumSides == 0)
	{   
		MemLen = (long)sizeof(DPOINT)*(long)USHRT_MAX*16;
		hLoop=GSSiGlobAlloc ( 580,GMEM_MOVEABLE,MemLen);
	}
	else
		hLoop=GSSiGlobAlloc ( 581,GMEM_MOVEABLE,MemLen);
	if (Reverse)
	{
		hLoop2=GSSiGlobAlloc ( 582,GMEM_MOVEABLE,MemLen);
		pPoints = (HPDPOINT)GlobalLock (hLoop2);
	}
	else
		pPoints = (HPDPOINT)GlobalLock (hLoop);
	*pNumSides = 0;	
	GSSillseek (FidLoop,Offset,0); 
	while (NumLinks--)
	{
		BigRead (FidLoop,(HPSTR)&LoopRec,sizeof(LoopRec));
		BT_FIND (hBTLinks,(LPSTR)&LoopRec.LinkRef,BT_FIRST,BT_EQ,(LPSTR)&LinkData); 
		GSSillseek (FidLinks,LinkData.Offset+4,0);
		BigRead (FidLinks,(HPSTR)&nSyms,2);
		BigRead (FidLinks,(HPSTR)Symbols,2*nSyms);  
		while (nSyms--)
		{   
			LPSHORT	pSym=pSymbols;
			
			for (i=0;i<*pNumSym;i++,pSym++)
			{
				if (Symbols[nSyms] == *pSym)
					goto GotSym;
			}
			*pSym = Symbols[nSyms];
			(*pNumSym)++;
GotSym:;
		}
		BigRead (FidLinks,(HPSTR)&np,4); 
		if ((long)(*pNumSides)+(long)(np-1) > (long)USHRT_MAX*16)
		{
			HFILE		FidBad;
			HPDPOINT	pPoint; 
			char		BadAreaFile[MAX_PATH];
			
			GlobalUnlock (hLoop);
			GlobalUnlock (hLoop2);
			MessageBox (0,"Area has more than 256K sides",0,MB_ICONEXCLAMATION);  
			GetGlobalCVal ("[%BADAREAFILE]",BadAreaFile,"[%SYSTEMPDIR]badarea.bin");
			FidBad = GSSiOpenFile (BadAreaFile,0,OF_CREATE);
			BigWrite (FidBad,(HPSTR)pNumSides,4,-1);
			if (Reverse)
				pPoint = (HPDPOINT)GlobalLock (hLoop2);
			else
				pPoint = (HPDPOINT)GlobalLock (hLoop);
			BigWrite (FidBad,(LPSTR)pPoint,(long)*pNumSides*(long)sizeof(DPOINT),-1);
			if (Reverse)
				GlobalUnlock (hLoop2);
			else
				GlobalUnlock (hLoop);
			GSSiGlobFree (&hLoop);  
			GSSiGlobFree (&hLoop2);
			BigWrite (FidBad,(HPSTR)&np,4,-1); 
			hLoop = GSSiGlobAlloc (0,GMEM_MOVEABLE,(long)np*(long)sizeof(DPOINT));
			pPoint = (HPDPOINT)GlobalLock (hLoop);
			BigRead (FidLinks,(HPSTR)pPoint,(long)np*sizeof(DPOINT)); 
			BigWrite (FidBad,(HPSTR)pPoint,(long)np*(long)sizeof(DPOINT),-1);   
			GSSiGlobUlFree (&hLoop);
			GSSiClose2 (&FidBad);
			return 0;
		} 
		(*pNumSides)+=(np-1); 
		if (LoopRec.Dir == 1)
			BigRead (FidLinks,(HPSTR)pPoints,(long)np*sizeof(DPOINT)); 
		else
		{   
			HANDLE	Handle = GSSiGlobAlloc ( 583,GMEM_MOVEABLE,(long)np*sizeof(DPOINT)); 
			HPDPOINT	pPointsSave = pPoints;
			
			pPoints2 = (HPDPOINT)GlobalLock (Handle);	
			BigRead (FidLinks,(HPSTR)pPoints2,(long)np*sizeof(DPOINT)); 
			np2 = np;
			pPoints2+=(np-1);
			while (np2--)
				*pPoints++ = *pPoints2--;
			GSSiGlobUlFree (&Handle); 
			pPoints = pPointsSave;
		}
		pPoints += (np-1);
	}
	(*pNumSides)++;
	if (Reverse)
    {   
    	pPoints2 = (HPDPOINT)GlobalLock (hLoop); 
    	np = *pNumSides;
    	while (np--)
    		*pPoints2++=*pPoints--;
    	GSSiGlobUlFree (&hLoop2);
    }
    GlobalUnlock (hLoop);
	return hLoop;
}

BOOL PointInLoop (DPOINT Point,long NumLoopPoints,HANDLE hLoopPoints)
{
	BOOL	rtn;  
	HPDPOINT	pLoopPoints;
	
	pLoopPoints = (HPDPOINT)GlobalLock (hLoopPoints);
	rtn = POINT_IN_AREAD (Point, NumLoopPoints, pLoopPoints,1,0,0,0);
	GlobalUnlock (hLoopPoints);
	return rtn;
}

BOOL CalculateLoopSize (HFILE FidLoop,long Offset,int NumLinks,BOOL Reverse,
            			LPMNMXCORD pMinMax, LPDOUBLE pSize,LPINT pNumSides, LPDPOINT pPoint1)
{   
	LOOPREC	LoopRec;
	BOOL	First=TRUE; 
	HPDPOINT	lpDPoints;
	HANDLE	Handle; 
	double	Perim; 
	int	np;
	short	NumLinkSyms=0, LinkSyms[64];
	
	*pNumSides = 0;
	DBoundsInit (pMinMax);
	Handle = LoadLoopSides (pNumSides,NumLinks,Offset,Reverse,&NumLinkSyms,LinkSyms);
	if (!Handle)
		return FALSE; 
	lpDPoints = (HPDPOINT)GlobalLock (Handle);
	*pPoint1 = *lpDPoints;
	*pSize = fabs (ComputeAreaAreaD (lpDPoints,*pNumSides,&Perim));
	np = *pNumSides;
	while (np--)
		AddToMinMaxD (pMinMax, lpDPoints++);
	GSSiGlobUlFree (&Handle);
	return TRUE;
}  

BOOL CreateRefConnectTable (HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{HDC hDC;
 switch (Message)
   {
   	case GF_INIT:
		PostMessage(hWnd, GF_EXECUTE,CurView->ID, 0L); 
   		break;
    
    case GF_EXECUTE: 
    	RefConnectTableCreate (&HLTBounds);
        PostMessage(hWnd, GF_CLOSE,0, 0L); 
	
		break;
/*typedef struct {
		long	Ref;
		POINT	BPFile,EPFile; 
		short	ConnectStatus,
				BPConnections, EPConnections;
		DPOINT	BPBase, EPBase;
		} REFCONNECT;  */
		
    default:
    	return (FALSE);
    }
    return (TRUE);
} 

BOOL RefConnectFix (HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{HDC hDC;
 switch (Message)
   {
   	case GF_INIT:
		PostMessage(hWnd, GF_EXECUTE,CurView->ID, 0L); 
   		break;
    
    case GF_EXECUTE: 
    	FixRefConnect ();
        PostMessage(hWnd, GF_CLOSE,0, 0L);
		break;
    default:
    	return (FALSE);
    }
    return (TRUE);
} 

BOOL RefConnectOutputLines (HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{HDC hDC;
 int	st, BPConnect,EPConnect;
 LPGWDHEADER lpGWDHead;
 HIGHLIGHTDATA	HighlightData;
 LPREFCONNECT	pRC;
 short	pos, pos2, cond,len,ii, SnapItem, FixItem, SaveMaxPick;  
 long	Refno, Offset, NearOpRef, NumRemoved=0, TotLoops, MinRef, nlast,n;  
 HCURSOR	hcurSave;   
 POINT	Point;  
 LINKDATA	LinkData; 
 short	NumLoopPoints[256], NumLoops;
 HANDLE	hLoopPoints[256];  
LOOPREC	LoopRec;    
static	long	debuglinkref=184,debugarearef=2, debugloops=146, debuglink=185;
 switch (Message)
   {
   	case GF_INIT:
   		if (!CurView->UpdateFile)
   		{
	 		MessageBox(GetFocus(), "No update file in this viewport", 0,MB_ICONEXCLAMATION|MB_OK);
			PostMessage(hWnd, GF_CLOSE,0, 0L); 
            break;
   		} 
   		else
   			_fstrcpy (EditName,CurView->lpFiles[CurView->UpdateFile-1]);
   	
       	AddLBUTTON = FALSE;
		PostMessage(hWnd, GF_EXECUTE,CurView->ID, 0L); 
   		break;
           
    case GF_EXECUTE:  
    {
    	short	Pass=0, NumConnect, AtEnd, NewAreaSymbol, stuff[128];
    	long	LinkRef=0, LastRef,ii,n=0,NumLinks,StartRef, MinLinkRef, LoopID=1,NewLinkPoints;     
    	int		NumLinkPoints;
    	LPOINT	ConnectPointFile,OpEndFile;
    	DPOINT	ConnectPointBase,OpEndBase;
    	HANDLE	hLinkPoints = GSSiGlobAlloc ( 590,GMEM_MOVEABLE,(long)USHRT_MAX*(long)16);
    	HPDPOINT	pLinkPoints;    
		OFSTRUCTGM	OFStruct;
   	    BTVARDESC  Vars;  
   	    BOOL	JustDebugRec=FALSE,NoIslands=FALSE;
   	    char	mess[128]; 
		long	AreaRef=1;   
		COLORREF	AreaColor;
		short	NumLinkSyms, LinkSyms[64];
		static	long	debugref=1061980080;

		hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT));  
		AddNodeInit (&CurView->FileMNMX);
		OpenRefConnectionFile ("[%CONSTATFILE].gmd",&CurView->FileMNMX);
	    lpGWDHead = (LPGWDHEADER)GlobalLock (hDBRefConnect);
	    pRC = (LPREFCONNECT)&lpGWDHead->GWDData; 
	    LastRef = LONG_MAX;   
	    MinRef = LONG_MIN;
NextLink:
	    pos = BT_FIRST; 
	    cond = BT_GT;
	    NumLinks = 0; 
	    Refno = MinRef;
		while (!BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&Refno,pos,cond,(LPSTR)&Offset)) 
		{    
			pos = BT_NEXT;  
			cond = BT_ANY;
			NumLinks++;
			if (Refno == LastRef)
				ii=1;    
			LastRef = Refno;
			if (Refno == debugref)
				ii=1;    
			n++;
			if (!(n%1000))
				ii=1;
       		len = FillGWDData (lpGWDHead,Offset); 
       		StartRef = LONG_MAX; 
       		switch (Pass)
       		{            
       			case 0:
       			NumConnect = pRC->BPConnections;
       			ConnectPointFile = pRC->BPFile;
       			ConnectPointBase = pRC->BPBase;
       			OpEndBase = pRC->EPBase;
       			OpEndFile = pRC->EPFile;
       			AtEnd = 1;
       			break; 
       			
       			case 1:
       			NumConnect = pRC->EPConnections;
       			ConnectPointFile = pRC->EPFile;
       			ConnectPointBase = pRC->EPBase;
       			OpEndBase = pRC->BPBase;
       			OpEndFile = pRC->BPFile;
       			AtEnd = 2; 
       			break; 
       			
       			case 2: 
       			StartRef = Refno;
       			NumConnect = 2;
       			ConnectPointFile = pRC->BPFile;
       			ConnectPointBase = pRC->BPBase;
       			OpEndBase = pRC->EPBase;
       			OpEndFile = pRC->EPFile;
       			AtEnd = 1;
       			break;
       		}
       		if (!NumConnect || NumConnect > 1)
       		{   
       			
       			LinkRef++;
       			if (LinkRef == debuglinkref)
       				ii=1;
       			LinkData.FromNode = AddNode (2,AtEnd,ConnectPointBase,LinkRef,pRC,&LinkData.FromAZ);  
       			NumLinkPoints = 1;
       			pLinkPoints = (HPDPOINT)GlobalLock (hLinkPoints);
       			*pLinkPoints++=ConnectPointBase; 
       			NewLinkPoints = TraceLink (USHRT_MAX*16-1,Refno,LinkRef,StartRef,lpGWDHead,AtEnd,Offset,&pLinkPoints,&LinkData.ToNode,&LinkData.ToAZ,&NumLinkSyms,LinkSyms,TRUE);
       			if (NewLinkPoints < 0)
       			{
					GlobalUnlock (hDBRefConnect);
					GlobalUnlock (hLinkPoints);
					CloseRefConnectFile (); 
					goto Exit;
       			}
       			NumLinkPoints += NewLinkPoints;
       			GlobalUnlock (hLinkPoints);
       			pLinkPoints = (HPDPOINT)GlobalLock (hLinkPoints);  
       			LinkData.Offset = GSSillseek (FidLinks,0,2); 
       			LinkData.DeltaAZ = GetLinkDeltaAZ (NumLinkPoints,pLinkPoints);
       			LinkData.TrackedLeft=LinkData.TrackedRight=0;
       			BT_PUT (hBTLinks,(LPSTR)&LinkRef,(LPSTR)&LinkData);
       			BigWrite (FidLinks,(HPSTR)&LinkRef,4,-1);
       			BigWrite (FidLinks,(HPSTR)&NumLinkSyms,2,-1);
       			BigWrite (FidLinks,(HPSTR)LinkSyms,2*NumLinkSyms,-1);
       			BigWrite (FidLinks,(HPSTR)&NumLinkPoints,sizeof(int),-1);
				BigWrite (FidLinks,(HPSTR)pLinkPoints,(long)NumLinkPoints * sizeof(DPOINT),-1);       			
       			GlobalUnlock (hLinkPoints);
	       		goto NextLink;
       		}
       		else
       			MinRef = Refno;
       	}                 
       	if (!Pass)
       	{
       		Pass = 1;   
       		MinRef = LONG_MIN;
       		goto NextLink;
       	} 
       	else if (Pass == 1)
       	{
       		Pass = 2;  
       		MinRef = LONG_MIN;
       		goto NextLink;
       	}
       	else if (NumLinks)
       		ii=1;
		GlobalUnlock (hDBRefConnect);
		CloseRefConnectFile ();
	    
	    {
	    	short pos, numlinks; 
	    	long	TotNodes,BadNodes;
	    	LPOINT	LastNode;
		    
		    pos = BT_FIRST; 
		    LastNode.x = LastNode.y = LONG_MAX;
		    numlinks = 2; 
		    TotNodes=BadNodes=0;
		    while (!BT_FIND (hNodes,(LPSTR)&NodesKey,pos,BT_ANY,(LPSTR)&NodesData))
		    {
		    	pos = BT_NEXT; 
		    	if (NodesKey.NodeID.x != LastNode.x || NodesKey.NodeID.y != LastNode.y)
		    	{
		    		if (numlinks < 2) 
		    		{
		    			BadNodes++; 
		    			DisplayMarker (NodesData.Point,4,0,0,0,0,FALSE,FALSE,0,0,0,0,0);
		    		}
		    		numlinks = 1;
		    		TotNodes++;
		    		LastNode = NodesKey.NodeID;
		    	} 
		    	else
		    		numlinks++;
		    }
		    ii=1;  
		}

	    {   
			HANDLE	hSymDesc=0;
			short	NumSyms=0;  
			char	Name[256];

		    CreateNewMap (EditName,&CurView->WBounds,0,0,0,0,0,0,TRUE);
			EditBounds = CurView->FileMNMX;
		    _fstrcpy (PltName,EditName);   
		    GSSillseek (FidLinks,128,0);
		    while (BigRead (FidLinks,(HPSTR)&LinkRef,4)==4)
		    {
		    	short NewLineSymbol=3, rtn;
		    	long	NewRefno=1; 
		    	short	LinkSyms[64], NumLinkSyms;
		        
				BigRead (FidLinks,(HPSTR)&NumLinkSyms,2);
				BigRead (FidLinks,(HPSTR)LinkSyms,2*NumLinkSyms);
				BigRead (FidLinks,(HPSTR)&NumLinkPoints,sizeof(int));
	   			pLinkPoints = (HPDPOINT)GlobalLock (hLinkPoints);  
				BigRead (FidLinks,(HPSTR)pLinkPoints,(long)NumLinkPoints * sizeof(DPOINT)); 
				GlobalUnlock (hLinkPoints);      			
				AddToSymList (LinkSyms[0],&NumSyms,&hSymDesc); 
				rtn=AddPolyToMap (1,&NumLinkPoints, &hLinkPoints,1,LinkRef,0,2,LinkSyms[0],0,0,0,-1,-1,-1,0,0,0,0,TRUE,0);
		    }
		    CloseMap(TRUE);  
			AddSymToMap (NumSyms,hSymDesc,0,0); 
            DestroySymList (&NumSyms,&hSymDesc);
		}

Exit:	
	    GSSiGlobUlFree (&hLinkPoints);
		CloseNodes ();
	    GSSiSetCursor(hcurSave);
        PostMessage(hWnd, GF_CLOSE,0, 0L);
	}
		break;
    default:
    	return (FALSE);
    }
    return (TRUE);
}

  




