#include "graphint.h"
#include "translat.h"

typedef struct {
					long	Offset; 
					mnmxCor	MinMax;
					short	SymIndex;
				}ORACLEINDEXRECORD;
typedef ORACLEINDEXRECORD	FAR	*LPORACLEINDEXRECORD; 

typedef struct {
					long	Type;
					long	NumRecs;
					long	NumSyms;
					long	SymlistOffset;
					MNMXCORD	Bounds;
					char	TableName[128]; 
				}ORAFILEHEADER;
typedef ORAFILEHEADER	FAR	*LPORAFILEHEADER;

typedef struct { 
				 long	Type;
				 ULONG	MSLink;
				 ULONG	SymIndex;
				 ULONG	NumInfo,
				 		NumOrd,
				 		spacer;
				 MNMXCORD	Bounds;   
				}ORARECORDHEADER;
typedef ORARECORDHEADER	FAR *LPORARECORDHEADER; 

typedef struct {USHORT	QuadID;
				BYTE	XY[2];}	QUADLINK;
typedef QUADLINK	HUGE	*HPQUADLINK; 

typedef struct {BYTE xmn,ymn,xmx,ymx;
		  	    short	Up, Down[2];} QUADS;  
typedef QUADS	FAR	*LPQUADS;

static ORAFILEHEADER	ORAFileHeader; 
static ORARECORDHEADER	ORARecordHeader;  
static short			CurrentORAFileType;
static long				ORABaseRefno=0;
static short			NumORAParms; 
static char				ORAParms[4096]="";  
static char				ORATag[100]="", ORATAG[100]="", ORAText[256]=""; 
static char				LastORAFile[128]=""; 
static long				ORAColor=-1;
static short			ORAWidth=0;  
static MNMXCORD			ORAFileMNMX;   
static short			OracleSymbols[1000],ORADefaultSymbol;
static double			ORAPointSize=5;
static char				ORAazm[32]="[ORA.AZM]";   

#include "gmextern.h"    
#include <sys\types.h>
#include <sys\stat.h>         

long ORATypeFromName (LPSTR InName)
{
	long	Type = 0;
	HANDLE	hName = GSSiGlobAlloc (0,GMEM_MOVEABLE,256);
	LPSTR	Name=GlobalLock (hName);
	short	l;
	
	_fstrcpy (Name,InName); 
	ExpandText (Name);
	_fstrupr (Name); 
	l =_fstrlen (Name);
	if (!_fstricmp (&Name[max(0,l-7)],"_LN.ORA"))
		Type = ORAT_ARC;
	else if (!_fstricmp (&Name[max(0,l-9)],"_LINE.ORA"))
		Type = ORAT_ARC;
	else if (!_fstricmp (&Name[max(0,l-7)],"_PT.ORA"))
		Type = ORAT_POINT;
	else if (!_fstricmp (&Name[max(0,l-9)],"_POLY.ORA"))
		Type = ORAT_POLYGON;
	GSSiGlobUlFree (&hName);
	return Type;
}

BOOL IsORAFileVisible (void)
{
	short	idesc,i; 
	
	if (!ORAFileHeader.NumSyms)
		return TRUE;
	for (i=0;i<ORAFileHeader.NumSyms;i++)
	{
		idesc = OracleSymbols[i];  
    	if(GetVisibility(idesc)) 
    		return TRUE;
	}
	return FALSE;
}

BOOL SetORAVis (HWND hWndDlg, int DlgItemSym, int DlgItemPar)
{
	short	i,idesc, iparent; 
	char	DescName[100]; 
	BOOL	parent = FALSE;
	
	for (i=0;i<ORAFileHeader.NumSyms;i++)
	{
		idesc = OracleSymbols[i];  
		iparent = GetDictSymParent (idesc); 
		GetDictSymName (idesc,DescName);
        ConvertSymName (DescName,1,FALSE,idesc);  
    	if(GetVisibility(idesc))
    		_fstrcat (DescName,"\t<on>\t");
    	else
    		_fstrcat (DescName,"\t\t");
    	sprintf (_fstrchr(DescName,0),"%i\t%i",iparent,idesc);
		if (SendDlgItemMessage ((HWND)hWndDlg,DlgItemSym,LB_FINDSTRING,-1,(LPARAM) DescName) ==  LB_ERR)
			SendDlgItemMessage ((HWND)hWndDlg,DlgItemSym,LB_ADDSTRING,NULL,(LPARAM) DescName); 
		while (iparent)
		{   
			idesc = iparent;
			iparent = GetDictSymParent (idesc); 
			GetDictSymName (idesc,DescName);
            ConvertSymName (DescName,1,TRUE,idesc);  
	    	if(GetVisibility(idesc))
	    		_fstrcat (DescName,"\t<on>\t");
	    	else
	    		_fstrcat (DescName,"\t\t");
	    	sprintf (_fstrchr(DescName,0),"%i\t%i",iparent,idesc);
			if (SendDlgItemMessage ((HWND)hWndDlg,DlgItemPar,LB_FINDSTRING,-1,(LPARAM) DescName) ==  LB_ERR)
				SendDlgItemMessage ((HWND)hWndDlg,DlgItemPar,LB_ADDSTRING,NULL,(LPARAM) DescName); 
		}
	}

	return TRUE;
}

BOOL SetORAParms (long RecordNumber)
{   
	LPSTR	pDesc, pTAG, pClause, pC, pColor, pWidth, pRot; 
	short	rc;
	char	str[128];
    
	CurPointSize = ORAPointSize;
	CurrentORARec = RecordNumber;
	CurrentRefno = ORABaseRefno + CurMSLink;
	_fstrcpy (ORATag,ORATAG);
	ExpandText (ORATag);
/*	pTAG = ORATAG;
	if (*pTAG && (pC = _fstrchr (pTAG,':')))
	{   
		_fstrcpy (ORATag,pTAG); 
		ExpandText (ORATag);
		pC = _fstrchr (ORATag,':');
		*pC++ = 0;
		_fstrcpy (CurrentTAG,ORATag);
		_fstrcpy (CurrentUDI,pC--);
		*pC = ':'; 
		ExpandText (CurrentUDI);
	}
	else
	{   
		_fstrcpy (CurrentUDI,"[%MSLINK]");
		ExpandText (CurrentUDI);
		_fstrcpy (ORATag,"MSLINK:"); 
		_fstrcat (ORATag,CurrentUDI);
		_fstrcpy (CurrentTAG,ORATag);
	}*/     
	_fstrcpy (str,ORAazm);
	ExpandText (str);
	PTRot = atof (str);
	pDesc = ORAParms;
	while (*pDesc)
	{
		pClause = _fstrchr (pDesc,0) + 1;
		pColor = _fstrchr (pClause,0) + 1;
		pWidth = _fstrchr (pColor,0) + 1;
		pRot = _fstrchr (pWidth,0) + 1;
		if (!*pClause || LogicP (pClause,&rc))
		{
			break;
		}
		else
			pDesc = _fstrchr (pClause,0) + 1;
	}
	CurrentDesc = OracleSymbols[ORARecordHeader.SymIndex]; 
	if (!*pDesc)
		return FALSE;
	if (*pColor) 
	{
		_fstrcpy (str,pColor);
		ExpandText (str);
		ORAColor = atol (str);
	}
	else
		ORAColor = -1;
	if (*pWidth) 
	{
		_fstrcpy (str,pWidth);
		ExpandText (str);
		ORAWidth = atol (str); 
		if (ORAType == ORAT_POINT)
			CurPointSize = ORAWidth;
	}
	else
		ORAWidth = 0;
	return TRUE;
}    

void ProcessQuadLinks (USHORT QuadID, USHORT LevelUseX,USHORT LevelSplitVal,LPSHORT Down,ULONG NumRecs,HPQUADLINK QuadLink,
					   LPUSHORT NextLevelSplitVals)
{  
	ULONG	i,MinRec = 0, MaxRec = NumRecs-1; 
	USHORT	j,k=1;
	ULONG	TotVals[2]={0,0}, TotCount[2]={0,0};
	
	if (!ContinueProcessing)
		return;
	if (LevelUseX)
		k = 0;
	for (i=MinRec;i<=MaxRec;i++) 
	{
		if (QuadLink[i].QuadID == QuadID)
		{
			j = QuadLink[i].XY[LevelUseX] > LevelSplitVal;
			QuadLink[i].QuadID = Down[j];
			TotVals[j] += QuadLink[i].XY[k];
			TotCount[j]++;
		}
	}
	for (j=0;j<2;j++)
	{
		if (TotCount[j])
			NextLevelSplitVals[j] = TotVals[j]/TotCount[j];
		else
			NextLevelSplitVals[j] = 128; 
	}
	return;
}

short	GetNextQuad (short QuadID,LPQUADS Quads,LPSHORT pQuadLevel,LPUSHORT LevelNext)
{    
	short	NextQuadID;
	
	if (LevelNext[*pQuadLevel] > 1)
		goto GoUp; 
	NextQuadID = Quads[QuadID].Down[LevelNext[*pQuadLevel]++]; 
	(*pQuadLevel)++;
	LevelNext[*pQuadLevel] = 0;
	if (NextQuadID >= 0) 
		goto Exit;
	(*pQuadLevel)--;
GoUp:
	NextQuadID = Quads[QuadID].Up; 
	if (NextQuadID < 0)
		goto Exit;
	QuadID = NextQuadID; 
	(*pQuadLevel)--;
	if (LevelNext[*pQuadLevel] > 1)
		goto GoUp; 
	NextQuadID = Quads[QuadID].Down[LevelNext[*pQuadLevel]++]; 
	(*pQuadLevel)++;
	LevelNext[*pQuadLevel] = 0;
	if (NextQuadID < 0)
		goto GoUp;
Exit:
	return NextQuadID;
}	
	
long SortQuadLinks (HFILE FidSorted,ULONG NumRecs,USHORT XMid,HPLONG Offsets,HPQUADLINK QuadLink)
{  
#define MAXLEVELS	32
	long	NumTotQuads=1, MaxQuads, nQuadsProcessed=0;
	long	NumDesiredPerQuad = 256, NumPerQuad=NumRecs;   
	short	QuadLevel=0;
	ULONG	iRec;
	USHORT	i, FirstUpperQuad=0, FirstLowerQuad, UpperQuadID=0, NumUpperQuads=1, NumLowerQuads, NextQuadID=1;
	short	QuadID;
	HANDLE	hQuads=GSSiGlobAlloc (0,GHND,UINT_MAX); 
    USHORT	LevelNext[MAXLEVELS], LevelSplitVal[MAXLEVELS], LevelUseX[MAXLEVELS];
    USHORT	SplitVal, NextLevelSplitVals[MAXLEVELS][2];
	LPQUADS	Quads = (LPQUADS)GlobalLock (hQuads); 
	
	MaxQuads = UINT_MAX / sizeof (QUADS);
	Quads[0].xmn = 0;
	Quads[0].ymn = 0;
	Quads[0].xmx = 255;
	Quads[0].ymx = 255;  
	Quads[0].Up = -1; 
	Quads[0].Down[0] = -1;
	Quads[0].Down[1] = -1;
	while (ContinueProcessing && NumPerQuad > NumDesiredPerQuad)
	{
		NumLowerQuads = NumUpperQuads * 2;
		if (NumTotQuads+NumLowerQuads > MaxQuads)
		{
			NumLowerQuads /= 2;
			break;
		}
		NumTotQuads += NumLowerQuads; 
		NumPerQuad = NumRecs / NumLowerQuads;        
		FirstLowerQuad = NextQuadID;
		for (UpperQuadID=FirstUpperQuad;UpperQuadID<FirstUpperQuad+NumUpperQuads;UpperQuadID++)  
		{    
			for (i=0;i<2;i++)
			{   
				Quads[NextQuadID].Up = UpperQuadID;
				Quads[NextQuadID].Down[0] = -1;
				Quads[NextQuadID].Down[2] = -1;
				Quads[UpperQuadID].Down[i] = NextQuadID++;
			}
		}
		NumUpperQuads = NumLowerQuads;
		FirstUpperQuad = FirstLowerQuad;
		QuadLevel++;
	} 
	for (i=0;i<QuadLevel;i++)
	{
		LevelUseX[i] = i % 2;	
	} 
	QuadID = 0;
	QuadLevel = 0; 
	LevelNext[0] = 0; 
	SplitVal = XMid;
ProcessQuad:  
	if (!ContinueProcessing)
		goto Exit;
	StatusWindowUpdate2 (NULL,NumTotQuads,nQuadsProcessed++); 
	if (Quads[QuadID].Down[0] > 0)
	{
		ProcessQuadLinks (QuadID,LevelUseX[QuadLevel],SplitVal,Quads[QuadID].Down,NumRecs,QuadLink,
						  NextLevelSplitVals[QuadLevel]); 
	}
	else
		goto GoUp;
	SplitVal = NextLevelSplitVals[QuadLevel][LevelNext[QuadLevel]];
	QuadID = Quads[QuadID].Down[LevelNext[QuadLevel]++]; 
	QuadLevel++;
	LevelNext[QuadLevel] = 0;
	if (QuadID >= 0)
		goto ProcessQuad;   
GoUp:
	if (!QuadLevel)
		goto Exit; 
	QuadID = Quads[QuadID].Up;
	QuadLevel--;
	if (LevelNext[QuadLevel] > 1)
		goto GoUp; 
	SplitVal = NextLevelSplitVals[QuadLevel][LevelNext[QuadLevel]];
	QuadID = Quads[QuadID].Down[LevelNext[QuadLevel]++]; 
	QuadLevel++;
	LevelNext[QuadLevel] = 0;
	if (QuadID < 0)
		goto GoUp;
	goto ProcessQuad;
Exit:	
	QuadID = 0; 
	LevelNext[0] = 0; 
	nQuadsProcessed = 0;    
	while (ContinueProcessing && QuadID >= 0)
	{
		for (iRec=0;iRec<NumRecs;iRec++)
			if (QuadLink[iRec].QuadID == QuadID)
				BigWrite (FidSorted,(HPSTR)&Offsets[iRec],4,-1);
		QuadID = GetNextQuad (QuadID,Quads,&QuadLevel,LevelNext);
		StatusWindowUpdate2 (NULL,NumTotQuads,nQuadsProcessed++); 
	}
	GSSiGlobUlFree (&hQuads);
	return NumTotQuads;
}

HFILE SortORASpatially (HFILE Fid)
{
	HFILE	FidSorted;
	char	TempFile[128];
	HANDLE	hQuadLink, hOffsets;
	long	MaxRecs = 3L * 1024L * 1024L;  
	USHORT	LastSym=0;
	HPLONG	Offsets;  
	ULONG	NumWithinSym=0, RecordNum=0;  
	DPOINT	Mid;

	ULONG	XTot=0;
	double	XMin, YMin, XFactor, YFactor;  
	USHORT	XMid;
	short	ii;
	
	HPQUADLINK	QuadLink;					 
	
	hQuadLink = GSSiGlobAlloc (0,GMEM_MOVEABLE,MaxRecs * sizeof(QUADLINK));
	hOffsets = GSSiGlobAlloc ( 0,GMEM_MOVEABLE,MaxRecs * sizeof(long));
	QuadLink = (HPQUADLINK)GlobalLock (hQuadLink);
	Offsets = (LPLONG)GlobalLock (hOffsets);
	
	GSSiGetTempFileName (0,"gmo",0,TempFile);
	FidSorted = GSSiOpenFile (TempFile,NULL,OF_CREATE);
	
	GSSillseek (Fid,0,0);
	GSSilread (Fid,(HPSTR)&ORAFileHeader,sizeof(ORAFileHeader)); 
	XMin = ORAFileHeader.Bounds.xmn;
	YMin = ORAFileHeader.Bounds.ymn;
	XFactor = 255 / (ORAFileHeader.Bounds.xmx - ORAFileHeader.Bounds.xmn);
	YFactor = 255 / (ORAFileHeader.Bounds.ymx - ORAFileHeader.Bounds.ymn);
	StatusWindowUpdate (NULL,"Step 1",ORAFileHeader.NumRecs,0);
	while (ContinueProcessing && RecordNum++ < ORAFileHeader.NumRecs)
	{   
		if (NumWithinSym == 269330)
			ii=1;
		Offsets[NumWithinSym] = GSSillseek (Fid,0,1);
		GSSilread (Fid,(HPSTR)&ORARecordHeader,sizeof(ORARecordHeader));
		if (ORARecordHeader.SymIndex != LastSym && NumWithinSym > 500000)
		{   
			XMid = IDNINT ((double)XTot / (double)NumWithinSym); 
			NumQuadSegs = SortQuadLinks (FidSorted,NumWithinSym,XMid,Offsets,QuadLink);
			NumWithinSym = 0;
			XTot = 0;
			LastSym = ORARecordHeader.SymIndex;
		}
		Mid = MinMaxMidPointD (&ORARecordHeader.Bounds);
		QuadLink[NumWithinSym].XY[0] = IDNINT ((Mid.x - XMin) * XFactor);
		QuadLink[NumWithinSym].XY[1] = IDNINT ((Mid.y - YMin) * YFactor); 
		QuadLink[NumWithinSym].QuadID = 0; 
		XTot += QuadLink[NumWithinSym].XY[0];
		NumWithinSym++;
		GSSillseek (Fid,(long)ORARecordHeader.NumInfo*sizeof(ULONG)+(long)ORARecordHeader.NumOrd*sizeof(double),1); 
		StatusWindowUpdate (NULL,NULL,ORAFileHeader.NumRecs,RecordNum);
    }
    if (!NumWithinSym)
    	XMid = 0;
    else
		XMid = IDNINT ((double)XTot / (double)NumWithinSym);
	NumQuadSegs = SortQuadLinks (FidSorted,NumWithinSym,XMid,Offsets,QuadLink);
    GSSiGlobUlFree (&hQuadLink);
    GSSiGlobUlFree (&hOffsets);
    GSSillseek (FidSorted,0,0);
    return FidSorted;
} 

BOOL ExpandORAPointBounds (LPMNMXCORD pBounds)
{
	if (pBounds->xmn >= pBounds->xmx) 
	{
		short	RectMax; 
		double	symsize;
		RECT	SymRect=GetSymRect(OracleSymbols[ORARecordHeader.SymIndex]);

		RectMax = max ((long)SymRect.right - (long)SymRect.left,(long)SymRect.bottom - (long)SymRect.top);   
		symsize = ORAPointSize*((double)RectMax)/200; 
		ExpandBounds (pBounds,symsize/3);
		return TRUE;
	} 
	return FALSE;
}

HFILE CreateORAFileIndex (HFILE FidORAFile,LPSTR IndexName,LPSTR ORAFileName)
{   
	HFILE	FidIdx, FidSortedOffsets;   
	long	i, Offset;
	ORACLEINDEXRECORD	IndexRecord; 
	MNMXCORD	MinMaxCoord; 
	long	RecsPerBlock = 100;
	long	NumIndexBlocks = max (2,min(4096,1+(NumORARecs-1)/RecsPerBlock)); 
	long	BlockID; 
	BYTE	Version=1, Type=10;
	char	txt[16]; 
	BOOL	ValidRec;    
	HANDLE	hIndexBlocks;
	LPMINMAX BlockMinMax;
	short	types[16],numtypes=0,itype; 
	short	LastSym=-1;
	long	NumWithinSym=0, LocHeader;
	
	RecsPerBlock = 1+NumORARecs/(NumIndexBlocks-1);
	if (FidORAFile == HFILE_ERROR)
		return FidORAFile; 
	DisableHalt = TRUE;
	CreateStatusWindow (CurView->hWnd,2,"Creating GeoMaster Oracle File Index");      
	DisableHalt = FALSE;
	FidSortedOffsets = SortORASpatially (FidORAFile); 
	hIndexBlocks = GSSiGlobAlloc (0,GMEM_MOVEABLE,NumIndexBlocks*sizeof(mnmxCor));
	BlockMinMax = (LPMINMAX)GlobalLock (hIndexBlocks); 
	for (i=0;i<NumIndexBlocks;i++)
		MinMaxInit (&BlockMinMax[i]);
	FidIdx = GSSiOpenFile (IndexName,NULL,OF_CREATE); 
	BigWrite (FidIdx,(HPSTR)&Version,1,-1); 
	BigWrite (FidIdx,(HPSTR)&Type,1,-1); 
	BigWrite (FidIdx,(HPSTR)&NumIndexBlocks,4,-1); 
	BigWrite (FidIdx,(HPSTR)&RecsPerBlock,4,-1); 
	BigWrite (FidIdx,(HPSTR)BlockMinMax,NumIndexBlocks*sizeof(mnmxCor),-1); 
	GSSillseek (FidORAFile,0,0);
	GSSilread (FidORAFile,(HPSTR)&ORAFileHeader,sizeof(ORAFileHeader));
	StatusWindowUpdate (NULL,"Step 2",ORAFileHeader.NumRecs,0);
	for (i=0;i<ORAFileHeader.NumRecs;i++)
	{
		BigRead (FidSortedOffsets,(HPSTR)&IndexRecord.Offset,4);
		GSSillseek (FidORAFile,IndexRecord.Offset,0); 
		LocHeader = GSSillseek (FidORAFile,0,1);
		GSSilread (FidORAFile,(HPSTR)&ORARecordHeader,sizeof(ORARecordHeader));
//		GSSillseek (Fid,(long)ORARecordHeader.NumInfo*sizeof(ULONG)+(long)ORARecordHeader.NumOrd*sizeof(double),1); 
		for (itype = 0;itype < numtypes;itype++) 
		{
			if (ORARecordHeader.Type == types[itype])
				goto S100;
		}
		if (ORARecordHeader.Type != 3004)
			types[numtypes++] = ORARecordHeader.Type;    
S100:
		if (ORARecordHeader.SymIndex != LastSym)
		{
			NumWithinSym = 0;
			LastSym = ORARecordHeader.SymIndex;
		}
		NumWithinSym++;
		IndexRecord.SymIndex = ORARecordHeader.SymIndex; 
		ConvertRectCoord (&MinMaxCoord,&ORARecordHeader.Bounds,0,1); 
		ExpandORAPointBounds (&ORARecordHeader.Bounds);
		BoundsToMinMax (&IndexRecord.MinMax,&MinMaxCoord);
		BigWrite (FidIdx,(HPSTR)&IndexRecord,sizeof(IndexRecord),-1); 
		BlockID = i/RecsPerBlock;
		AddMinMax (&BlockMinMax[BlockID], &IndexRecord.MinMax);
		StatusWindowUpdate (NULL,NULL,ORAFileHeader.NumRecs,i);
		if (!ContinueProcessing)
			break;
	}
	GSSillseek (FidIdx,10,0);
	BigWrite (FidIdx,(HPSTR)BlockMinMax,NumIndexBlocks*sizeof(mnmxCor),-1);
	GSSillseek (FidIdx,0,0);
	BigWrite (FidIdx,(HPSTR)&Version,1,-1);
	if (numtypes == 1 && types[0] == 3001)
		Type = ORAT_POINT; 
	else if (numtypes == 1 && types[0] == 3002)
		Type = ORAT_ARC; 
	else if (numtypes == 2 && (types[0] == 3001 && types[1] == 3002 || types[0] == 3002 && types[1] == 3001))
		Type = ORAT_ARC; 
	else if (numtypes == 1 && types[0] == 3003)
		Type = ORAT_POLYGON; 
	BigWrite (FidIdx,(HPSTR)&Type,1,-1); 
	GSSiClose (FidIdx); 
	GSSiClose (FidSortedOffsets);
	if (ContinueProcessing)
		FidIdx = GSSiOpenFile (IndexName,NULL,OF_READ); 
	else
	{
		FidIdx = HFILE_ERROR;
		GSSiOpenFile (IndexName,NULL,OF_DELETE); 
	}
	GSSiGlobUlFree (&hIndexBlocks); 
	ORAIDXFid = FidIdx;
/*	for (i=0;i<ORAFileHeader.NumRecs;i++)
	{
		Offset = GetORARecordOffset (i,FALSE);
		GSSillseek (FidORAFile,Offset,0);
		GSSilread (FidORAFile,(HPSTR)&ORARecordHeader,sizeof(ORARecordHeader));
	} */
	DestroyStatusWindow(0); 
	return FidIdx;
}

short OpenORAFileIndex (HFILE FidORAFile,LPSTR ORAFileName)
{ 
	char	Name[128], SQL[64];  
	LPSTR	pDot;
	BYTE	ORAIndexType, Version;
	
	if (!ORAFileName)
	{
		if (ORAIDXFid != HFILE_ERROR)
			GSSiClose (ORAIDXFid); 
		ORAIDXFid = HFILE_ERROR;
		GetORARecordOffset (-1,FALSE);
		CloseDataFile (TRUE,&hORAGMD);
//		CloseDGNCellLibrary ();
		return TRUE;
	}
	_fstrcpy (Name,ORAFileName); 
	ExpandText (Name);
	_fstrcpy (LastORAFile,Name);
	pDot = _fstrrchr (Name,'.');
	if (!pDot)
		return FALSE;
	_fstrcpy (pDot,".ori");
	ORAIDXFid = GSSiOpenFile (Name,NULL,OF_READ); 
	if (ORAIDXFid == HFILE_ERROR)
		ORAIDXFid = CreateORAFileIndex (FidORAFile,Name,ORAFileName); 
	else
	{
	    struct _stat statIndex;
	    struct _stat statFile;
	    double	dtime;
		     
	    _fstat (ORAIDXFid,&statIndex);
	    _fstat (FidORAFile,&statFile);
	    dtime = difftime (statIndex.st_mtime,statFile.st_mtime);  
	    if (dtime < 0)
			ORAIDXFid = CreateORAFileIndex (FidORAFile,Name,ORAFileName);
	}
	if (ORAIDXFid == HFILE_ERROR)
		return FALSE; 
	GSSillseek (ORAIDXFid,0,0);
	BigRead (ORAIDXFid,(HPSTR)&Version,1);
	BigRead (ORAIDXFid,(HPSTR)&ORAIndexType,1);   
	if (!ORAIndexType) 
		ORAIndexType = ORAT_MIXED; 
	_fstrupr (ORAFileName); 
	if (_fstrstr (ORAFileName,"_TXT.ORA"))
		ORAIndexType = ORAT_TEXT; 
	CurrentORAFileType = ORAIndexType;
	sprintf (Name,"ORA=%s",ORAFileName);
	_fstrcpy (SQL,"MSLINK = [%MSLINK]");
	OpenDataFile (Name,SQL,BT_READ,&hORAGMD);
	return (short)ORAIndexType;
}

BOOL ReorgORAFile ()
{
	long	FirstLoc, LastLoc,NumBlocks,RecsPerBlock, FirstRecLoc; 
	HANDLE	hBlocks;
	LPMINMAX BlockMinMax;
	
		GSSillseek (ORAIDXFid,2,0);
		GSSilread (ORAIDXFid,(HPSTR)&NumBlocks,4);
		GSSilread (ORAIDXFid,(HPSTR)&RecsPerBlock,4);
		hBlocks = GSSiGlobAlloc (1417,GMEM_MOVEABLE,NumBlocks*sizeof(mnmxCor));
		BlockMinMax = (LPMINMAX)GlobalLock (hBlocks); 
		GSSilread (ORAIDXFid,(HPSTR)BlockMinMax,NumBlocks*sizeof(mnmxCor));
        GlobalUnlock (hBlocks); 
	return TRUE;
} 

long GetORARecordOffset (long record,BOOL UseBounds)
{
	long loc, outloc, BlockID;  
	static	HANDLE hOffsets=0;
	static	HANDLE	hBlocks=0;
	static	long	FirstLoc, LastLoc,NumBlocks,RecsPerBlock, FirstRecLoc;
	static	long	Len, tested=0;
	LPSTR	pLoc;  
	LPORACLEINDEXRECORD	pIndexRec;
	LPMINMAX BlockMinMax;
	
	if (ORAIDXFid == HFILE_ERROR) 
	{
		GSSiGlobFree (&hOffsets);
		GSSiGlobFree (&hBlocks);
		return -1; 
	} 
	if (record > NumORARecs-1)
		return -1;
	if (CurView->PassID && CurView->PassID < 4 && ORAType != ORAT_MIXED)
	{
		if (ORAType == ORAT_POLYGON && CurView->PassID != 2)
			return -1;  
		if ((ORAType == ORAT_POINT || ORAType == ORAT_ARC) && CurView->PassID == 2)
			return -1;  
	} 
	if (!hBlocks)
	{
		GSSillseek (ORAIDXFid,2,0);
		GSSilread (ORAIDXFid,(HPSTR)&NumBlocks,4);
		GSSilread (ORAIDXFid,(HPSTR)&RecsPerBlock,4);
		hBlocks = GSSiGlobAlloc (1417,GMEM_MOVEABLE,NumBlocks*sizeof(mnmxCor));
		BlockMinMax = (LPMINMAX)GlobalLock (hBlocks); 
		GSSilread (ORAIDXFid,(HPSTR)BlockMinMax,NumBlocks*sizeof(mnmxCor));
        GlobalUnlock (hBlocks); 
        FirstRecLoc = GSSillseek (ORAIDXFid,0,1);
    }
Top:
	if (record > NumORARecs-1)
		return -1;
	ItemSeg = record; 
	CurrentItem = 0;
	BlockID = record / RecsPerBlock;
	loc = FirstRecLoc + record * sizeof(ORACLEINDEXRECORD); 
	if (UseBounds)
	{ 
		BlockMinMax = (LPMINMAX)GlobalLock (hBlocks); 
		if (!BlockInWindow (&BlockMinMax[BlockID],0))
		{
			GlobalUnlock (hBlocks); 
			record = (BlockID + 1) * RecsPerBlock;
			goto Top;
		}
		GlobalUnlock (hBlocks);
	}
	if (!hOffsets || loc < FirstLoc || loc > LastLoc)
	{   
		GSSiGlobFree (&hOffsets);
		hOffsets = GSSiGlobAlloc (1417,GMEM_MOVEABLE,RecsPerBlock*sizeof(ORACLEINDEXRECORD));
		pLoc = (LPSTR)GlobalLock (hOffsets);
		GSSillseek (ORAIDXFid,loc,0);
		Len = GSSilread (ORAIDXFid,(HPSTR)pLoc,RecsPerBlock*sizeof(ORACLEINDEXRECORD));
		pLoc += (Len - sizeof(ORACLEINDEXRECORD));
		pIndexRec = (LPORACLEINDEXRECORD) pLoc;  
		GlobalUnlock (hOffsets);
		if (Len < sizeof(ORACLEINDEXRECORD))
		{
			GSSiGlobFree (&hOffsets);
			return -1;               
		}
		FirstLoc = loc;
		LastLoc = FirstLoc + Len - 1;
	}
	loc -= FirstLoc;
	pLoc = (LPSTR)GlobalLock (hOffsets);
	pLoc += loc; 
	pIndexRec = (LPORACLEINDEXRECORD) pLoc; 
	outloc = pIndexRec->Offset;
	if (UseBounds)
	{   
		tested++;
		if (!GetVisibility (OracleSymbols[pIndexRec->SymIndex]) || !BlockInWindow (&pIndexRec->MinMax,0))
		{
			GlobalUnlock (hOffsets); 
			record++;
			goto Top;
		}
	}
	GlobalUnlock (hOffsets); 
	CurrentORARec = record;
	return outloc;
}

BOOL ReadORARecordHeader (HFILE FidORA,long RecordOffset,LPMNMXCORD pMinMaxCoord)
{   
	DPOINT		Points[4];  
	UINT		i;
    
	GSSillseek (FidORA,RecordOffset,0);
	GSSilread (FidORA,(HPSTR)&ORARecordHeader,sizeof(ORARecordHeader));
	ConvertRectCoord (pMinMaxCoord,&ORARecordHeader.Bounds,0,1);
	SetGlobalValueLong ("%MSLink",ORARecordHeader.MSLink);
/*	switch (ORAType)
	{
		case SHPT_POINT:
	        if (_lread (FidSHP,&SHPPointRec,sizeof(SHPPointRec)) != sizeof(SHPPointRec))
	        	return FALSE; 
			if (!SHPPointRec.Type)
				return FALSE;
            if (pMinMaxCoord)
            { 
				DBoundsInit (pMinMaxCoord);
				Points[0] = SHPPointRec.Point; 
				if (ConvertCoord(&Points[0],0,1)) 
					return FALSE;
				AddDPointToMinMax (&Points[0],pMinMaxCoord);
			}  
		break;
		
		case SHPT_ARC:
		case SHPT_POLYGON:
		case SHPT_POLYGONZ:
            if (_lread (FidSHP,&SHPPolyHeader,sizeof(SHPPolyHeader)) != sizeof(SHPPolyHeader))
            	return FALSE; 
			if (!SHPPolyHeader.Type)
				return FALSE;
            	
            if (pMinMaxCoord)
            { 
				DBoundsInit (pMinMaxCoord);
				Points[0].x = SHPPolyHeader.Xmin;   
				Points[0].y = SHPPolyHeader.Ymin;   
				Points[1].x = SHPPolyHeader.Xmin;   
				Points[1].y = SHPPolyHeader.Ymax;   
				Points[2].x = SHPPolyHeader.Xmax;   
				Points[2].y = SHPPolyHeader.Ymax;   
				Points[3].x = SHPPolyHeader.Xmax;   
				Points[3].y = SHPPolyHeader.Ymin; 
				for (i=0;i<4;i++)
				{
					if (ConvertCoord(&Points[i],0,1))
					{   
	//				    MessageBox(GetFocus(),"Unable to convert coordinates as specified", 0,MB_ICONQUESTION|MB_OK);
					    return FALSE;
					}
					AddDPointToMinMax (&Points[i],pMinMaxCoord);
				} 
			}
		break;
		case SHPT_MULTIPOINT:
		break;
	} 
*/ 
	GetFileMinMax (&CurrentItemMinMax,pMinMaxCoord); 
	return TRUE;
}  

short GetNumORAPoly (LPLONG pElemInfo,ULONG NumInfo)
{
	short nPoly=0;
	ULONG	loc=1;
	
	while (loc < NumInfo)
	{
		switch (pElemInfo[loc])
		{
			case 1003:
			case 2003:
			case 1005:
			case 2005:
			case 3:
			case 5:
				nPoly++;
			break;  
			default:
			break;
		}
		loc += 3;
	}
	
	return max (1,nPoly);
} 

BOOL ProcessORARecord (HDC hDC,HFILE FidORA,long RecordNumber)
{
    HPDPOINT    pPoints, pFirstPoint;
    LPLONG      pPartIndex;   
    ULONG		i,j,n, ilast;
    short		iPoly, nPoly, Type, ltag;
    HANDLE		hPoints, hPartIndex, hPolyPartLen;  
    LPUSHORT	pNumPoints;
    long		NumPoints; 
    HPEN		hOldPen=0, hTempPen=0;
	HBRUSH		hOldBrush=0, hDeletePen=0, hTempBrush=0;  
	DPOINT		DPoint;
	double		size;
	HANDLE		hElemInfo=0;
	HANDLE		hOrd=0;   
	LPLONG		pElemInfo;
	LPDOUBLE	pOrd; 
	long		ElemInfoLoc;
	short		ElemType, Interp; 
	char		str[128]; 
	long		loopfactor=1; 
	short		ii; 
//	long		DBInfo[50];    
//	double		DBOrd[50];

#define	POINT	1
#define LINE	2
#define POLYGON	3  
#define TEXT	4

	ShowValue (hDC,FALSE); 
	InGraphicsProcessor = TRUE;
	ItemIsDeleted = FALSE;
	InitRecord (hDC);
	CurMSLink = MSLink; 
    SetORAParms (RecordNumber);  
//    ltoa (RecordNumber,str,10);
//    SetWindowText (hWndMain,str);
    if (RecordNumber == 3579)
    	ii=1;
    if (!CurrentDesc)
    	goto RtnFalse;
	SetSymNum (CurrentDesc);
    ltag = _fstrlen (ORATag);
	if (!ProcessRefAndTAG (GetVisibility (CurrentDesc),ORATag,ltag))
		goto RtnFalse;  
	if (ForceRefIndex || ForceTAGIndex)
		goto RtnFalse;
	if (hDC && Display)
	{
		SetROP2(hDC,DisplayRasterOpt);
		if (HaveVarFillColor)
			SetTextColor (hDC,ConvertColor(GlobalColors[0],CurrentDesc));
		else
			SetTextColor (hDC,ConvertColor(DefaultTextColor,CurrentDesc));
		SelectObject (hDC,GetStockObject(BLACK_PEN));
		GSSiDeleteObject (&hBlackPen);
		hBlackPen = CreatePen (PS_SOLID,0,ConvertColor(0,CurrentDesc));		
		SelectObject (hDC,hBlackPen);
	}
	HiPrecis = TRUE; 
	CurrentPen = 0;
	switch (ORARecordHeader.Type)
	{   
		default:
			break;  
		case 3004:
			hElemInfo = GSSiGlobAlloc (1418,GMEM_MOVEABLE,ORARecordHeader.NumInfo*4);
			pElemInfo = (LPLONG)GlobalLock (hElemInfo);
			hOrd = GSSiGlobAlloc (1418,GMEM_MOVEABLE,ORARecordHeader.NumOrd*8);
			pOrd = (LPDOUBLE)GlobalLock (hOrd);
			GSSillseek (FidORA,ORARecOffset+sizeof(ORARecordHeader),0);
			GSSilread (FidORA,(HPSTR)pElemInfo,ORARecordHeader.NumInfo*4);
			GSSilread (FidORA,(HPSTR)pOrd,ORARecordHeader.NumOrd*8);  
			GSSiGlobUlFree (&hElemInfo);
			GSSiGlobUlFree (&hOrd);
			break;		
		case 3001:
		case ORAT_POINT:  
			if (CurView->PassID == 2)
				goto RtnFalse;
			if (!GetTypeVisibility(TYPE_POINT))
				goto RtnFalse;
			DPoint = MinMaxMidPointD (&ORARecordHeader.Bounds); 
			if (ConvertCoord(&DPoint,0,1)) 
				goto RtnFalse;
	        lpDCurPoints = &DPoint;  
			CurrentPoint = CurPointLocD = DPoint;
	        LastElementBeginPoint = LastElementEndPoint = DPoint;
			nPnts = nCurPoints = 1;
			CurPointLoc = BasePtToWinPt(lpDCurPoints); 
    		if (!PointInMaskAreaWinCoord (CurPointLoc))
    			goto RtnFalse;
    		if (PointIsBlocked (&CurPointLocD,CurrentDesc))
    			goto RtnFalse;   
       		HaveTXLoc = TRUE;
    		CurrentType = GF_POINT; 
			if (CurPointSize < 0)
				CurPointSize = -CurPointSize * DeviceToScreenFactor;
			else
				CurPointSize /= CurView->BaseUnitsPerPixel;
			if ((Pick||PickingByRefno) && GetTypeVisibility(TYPE_POINT))
			{
				PickPointItemD (lpDCurPoints,(CurPointSize*ThemeWidthFactor)*CurView->BaseUnitsPerPixel,PTRot,CurrentDesc);			 		    
			} 
			else if (GetTypeVisibility(TYPE_POINT))
			{   
				short	iDesc=CurrentDesc;
				
				if (CurrentDesc > 0 && CurrentDesc < 3201)	
				{	
					if (TSize)
						CurView->CurVisType[CurrentDesc]=5;
					else
						CurView->CurVisType[CurrentDesc]=4;  
				}
				HighlightPointSym=FALSE;
				if (!GetTypeVisibility(6) && SymbolIsVisible (iDesc)) 
				{
					CurPointSize = 10*DeviceToScreenFactor;
					iDesc = InvisiblePointSymbol;
				}
				if (SetDisplayChar (hDC,GF_POINT,CurrentRefno,CurrentDesc,CurrentTAG,CurrentUDI))
				{   
					if (ThemePointSym)
					{
						iDesc = ThemePointSym;
						if (ThemePointSize < 0)
							size = -ThemePointSize *DeviceToScreenFactor;
						else
							size = ThemePointSize / CurView->BaseUnitsPerPixel; 
						size *= ThemeWidthFactor;
						size = min(max (size,1),MaxPointSize);
					}
					else
						size = CurPointSize*ThemeWidthFactor*GraphicsPointFactor; 
					DisplayPointItem (hDC,CurPointLoc,size,PTRot,iDesc,&CurView->MaxSymbolWidth);
				} 
			}
   			TXLoc = CurPointLocD; 
       		HaveTXLoc = 1;  
		break;
		
		case 3002:
		case ORAT_ARC: 
			if (CurrentORAFileType == ORAT_TEXT)
			{
				if (CurView->PassID && CurView->PassID != 4 && CurView->PassID != 3)
					goto RtnFalse;  
				else 
				{
				    if (CurrentDesc < 3201)
						CurView->CurVisType[CurrentDesc]=5; 
					CurrentType = GF_POINT;	
				} 
				goto DoPoly;
			}
			else
			{ 
				if (CurView->PassID == 2)
					goto RtnFalse; 
			}
			if (!GetTypeVisibility(TYPE_LINECURVE))
				goto RtnFalse;
		    if (CurrentDesc > 0 && CurrentDesc < 3201)
				CurView->CurVisType[CurrentDesc]=2; 
			goto DoPoly;
		case 3003:
		case ORAT_POLYGON:
			if (CurView->PassID && CurView->PassID != 4 && CurView->PassID != 2)
				goto RtnFalse;
			if (!GetTypeVisibility(TYPE_AREA))
				goto RtnFalse;
		    if (CurrentDesc > 0 && CurrentDesc < 3201)
				CurView->CurVisType[CurrentDesc]=3; 
DoPoly:      
			hElemInfo = GSSiGlobAlloc (1418,GMEM_MOVEABLE,ORARecordHeader.NumInfo*4);
			pElemInfo = (LPLONG)GlobalLock (hElemInfo);   
			hOrd = GSSiGlobAlloc (1418,GMEM_MOVEABLE,ORARecordHeader.NumOrd*8);
			pOrd = (LPDOUBLE)GlobalLock (hOrd);
			GSSillseek (FidORA,ORARecOffset+sizeof(ORARecordHeader),0);
			GSSilread (FidORA,(HPSTR)pElemInfo,ORARecordHeader.NumInfo*4);
			GSSilread (FidORA,(HPSTR)pOrd,ORARecordHeader.NumOrd*8);
//			_fmemcpy (DBInfo,pElemInfo,min(100,ORARecordHeader.NumInfo)*4);
//			for (i=0;i<min(50,ORARecordHeader.NumOrd);i++)
//				DBOrd[i] = pOrd[i];
			nPoly = GetNumORAPoly (pElemInfo,ORARecordHeader.NumInfo);
			if (nPoly > 1)
				ii=1; 
			ElemType = pElemInfo[1];
			Interp	 = pElemInfo[2];
			ElemInfoLoc = 3;  
			switch (ElemType)
			{
				case 1:
				case 1001:
					Type = POINT;
		    		CurrentType = GF_POINT; 
					break; 
				case 1002:
				case 1004:
				case 2:
				case 4:
					if (CurrentORAFileType == ORAT_TEXT) 
					{
						Type = TEXT;
						CurrentType = GF_TEXT;
					}
					else
					{
						Type = LINE;  
	    				CurrentType = GF_POLYLINE; 
	    			}
					break;
				case 3:   
				case 1003:
					Type = POLYGON;
		    		CurrentType = GF_AREA; 
				break;
				case 2003:
					Type = POLYGON;
		    		CurrentType = GF_AREA; 
				break;
				case 5:  
				case 1005:
				case 2005:
					Type = POLYGON;
		    		CurrentType = GF_AREA; 
				break;
			}

/*	        if (SHPPolyHeader.NumPoints > UINT_MAX)
	        	goto RtnFalse;
	        if (!SHPPolyHeader.NumPoints)
	        	goto RtnFalse;
	        if (!SHPPolyHeader.Type)
	        	goto RtnFalse;
	        nPoly = SHPPolyHeader.NumParts;*/  
	        NumPoints = ORARecordHeader.NumOrd/3; 
//	        if (SHPHeader.ShapeType == SHPT_POLYGON)
//	        	NumPoints = NumPoints+nPoly-1;
//	        hPartIndex = GSSiGlobAlloc (1418,GMEM_MOVEABLE,sizeof(long)*(nPoly+1));
	        hPolyPartLen = GSSiGlobAlloc (1419,GHND,sizeof(USHORT)*(nPoly+1));
			hPoints = GSSiGlobAlloc (1420,GMEM_MOVEABLE,(long)MAX_POLY_POINTS*(long)sizeof(DPOINT)); 
//	        pPartIndex = (LPLONG)GlobalLock (hPartIndex); 
//	        _lread (FidSHP,pPartIndex,nPoly*sizeof(long));
	                    
            pPoints = pFirstPoint = (HPDPOINT)GlobalLock (hPoints); 
            pNumPoints = (LPUSHORT)GlobalLock (hPolyPartLen); 
/*	        for (i=0;i<nPoly;i++)
	        {   
	            long    numpoints, startpoint,ii; 
	                        
	            startpoint = *pPartIndex++;
	            numpoints = *pPartIndex-startpoint; 
	            *pNumPoints++ = numpoints; 
	            if (GSSilread (FidSHP,(HPSTR)pPoints,numpoints * sizeof(DPOINT)) != numpoints * sizeof(DPOINT))
	            	goto RtnFalse;
	            pPoints += numpoints;   
	            if (i && SHPHeader.ShapeType == SHPT_POLYGON)
	            	*pPoints++ = *pFirstPoint; 
	        } */
	        GlobalUnlock (hPolyPartLen);  
            pNumPoints = (LPUSHORT)GlobalLock (hPolyPartLen); 
NextLoop:   
			iPoly = 0;  
	        GlobalUnlock (hPoints); 
	        if (loopfactor > 32)
	        	goto GetOut;
//	        GSSiGlobUlFree (&hPartIndex);  
			pPoints = (HPDPOINT)GlobalLock (hPoints);  
			j = 0; 
			i = 0;
			ilast = i;
			while (j < ORARecordHeader.NumOrd) 
			{    
				if (ElemInfoLoc < ORARecordHeader.NumInfo && j+1 == pElemInfo[ElemInfoLoc])
				{
					ElemType = pElemInfo[ElemInfoLoc+1];
					Interp	 = pElemInfo[ElemInfoLoc+2];
					ElemInfoLoc += 3;
					switch (ElemType)
					{
						case 1005:
						case 2005:
                        	ii=1;
						case 1003:
						case 2003: 
							pNumPoints[iPoly] = i - ilast;  
							if (iPoly++)  
								pPoints[i++] = *pFirstPoint;
							ilast = i;
						break;
						default:
						break;
					}  
				} 
				if (i >= MAX_POLY_POINTS-16)
				{
					loopfactor *= 2;
					goto NextLoop;  
				}
				switch (ElemType)
				{
					case 1: 
					case 1001:
						for (n=0;n<Interp;n++)
						{
							pPoints[i].x = pOrd[j++];
							pPoints[i].y = pOrd[j++];
							j++; 
							ConvertCoord(&pPoints[i++],0,1); 
						}
						break;
					case 2: 
					case 3:
					case 2003: //inner ring
						ii=1;
					case 1003: //outer ring
						switch (Interp)
						{ 
							case 1:
								pPoints[i].x = pOrd[j++];
								pPoints[i].y = pOrd[j++]; //pOrd[j]
								j++; 
								ConvertCoord(&pPoints[i++],0,1); 
							break;
							case 2:
							{
								DPOINT	BP, POC, PT; 
								double	BackAZ, CurveFactor; 
								HPDPOINT	pp;
								
								BP.x = pOrd[j++];
								BP.y = pOrd[j++]; 
								j++;
								ConvertCoord(&BP,0,1); 
								POC.x = pOrd[j++];
								POC.y = pOrd[j++];
								j++;
								ConvertCoord(&POC,0,1); 
								PT.x = pOrd[j];
								PT.y = pOrd[j+1];
								ConvertCoord(&PT,0,1);
								if (j + 3 >= ORARecordHeader.NumOrd)
									j+=3; 
								pp = &pPoints[i];
								if (Display)
									CurveFactor = 0;
								else
									CurveFactor = CurveChordDist;
								CurvePointsD(&BP,&POC,&PT, &i, &pp,&BackAZ,(((long)MAX_POLY_POINTS-16)/loopfactor)-i,CurveFactor,1);
							} 
						}
						break; 
				}
			}
GetOut:
			pNumPoints[iPoly] = i - ilast; 
	        GlobalUnlock (hPolyPartLen);  
			NumPoints = min (MAX_POLY_POINTS,i);
			if (Type == LINE) 
			{
				if (Pick)
				{   
					PickPolylineD (pPoints,NumPoints,2,0,0,0);
				}
				else if (CopyRec)  
				{
					USHORT	nPoints = NumPoints;
					
					AddPolyToBuffer (1,&nPoints,&hPoints,TYPE_POLYLINE,CurrentRefno,NULL,-1,CurrentDesc,0,CurrentTAG,CurrentUDI,
                                     -1,-1,0,0,0,0,0,TRUE,&hUpdateBuf,&lUpdateBuf); 
                }
				else
				{    
					lpDCurPoints = pPoints;
					nPnts = nCurPoints = NumPoints;

					if (PolyInMaskAreaFileCoord (GF_LINE,&NumPoints,NULL,NULL,&pPoints,TRUE))
					{   
						HPEN hNewPen=0;  
						DPOINT	DynSegEndPoint;
						
						hOldPen  = SelectObject(hDC,GetStockObject(BLACK_PEN));
//						TempLineColor = SHPColor; 
//						TempLineWidth = SHPWidth; 
						if (TempLineColor >= 0 || TempLineWidth != 0)
						{
							hNewPen = CreatePen (PS_SOLID,(short)IDNINT(TempLineWidth*DeviceToScreenFactor),TempLineColor);
							SelectObject(hDC,hNewPen);
						}
						pNumPoints = (LPWORD)GlobalLock (hPolyPartLen);
		        		for (i=0;i<nPoly;i++)
		        		{   
		        			HPDPOINT	SavelpDCurPoints = lpDCurPoints;
		        			
		        			nPnts = *pNumPoints;  
		        			InDynamicSegmentation = FALSE;  
		        			DoDynamicFixedSegmentation (-1,0,0);
							while (nPnts > 1)
							{ 
								if (SetDisplayChar (hDC,GF_LINE,CurrentRefno,CurrentDesc,CurrentTAG,CurrentUDI))
								{   
									GWPolylineD (hDC,lpDCurPoints,NumDynSegPoints,CurrentDesc); 
								}
								DynSegEndPoint = lpDCurPoints[NumDynSegPoints-1];  
								lpDCurPoints += (NumDynSegPoints - 2);
								nPnts = NumDynSegPointsRemaining;
								if (nPnts > 1)
								{   
									lpDCurPoints[0] = DynSegEndPoint;
									lpDCurPoints[1] = DynSegSavePoint;  
									InDynamicSegmentation = TRUE;
								}	
							}
		        			InDynamicSegmentation = FALSE;   
		        			lpDCurPoints = SavelpDCurPoints;
							lpDCurPoints+=*pNumPoints++; 
			        	}  
			        	GlobalUnlock (hPolyPartLen); 
						if (hNewPen)
						{
							SelectObject(hDC,GetStockObject(BLACK_PEN)); 
							GSSiDeleteObject (&hNewPen);
						}
					}
				}
			}
			else if (Type == TEXT)
				{
					GRTEXTHEADER	GRTextHeader;  
                    DPOINT	MidPoint = MidPointD (pPoints[0],pPoints[1]);  
                    double	MidPointAZ = getazd (&pPoints[0],&pPoints[1]); 
                    char	txt[512]="[ORA.TEXT]";
                    short	nchar; 
                    double	THeight;
                    char	THeightC[32]="[TEXT_HEIGHT]";
                    
					CurPointLocD.x = MidPoint.x; 
					CurPointLocD.y = MidPoint.y; 
		    		LastElementBeginPoint = CurPointLocD;
		    		PTRot = MidPointAZ; 
		    		ExpandText (txt);
		    		nchar = _fstrlen (txt);
		    		ExpandText (THeightC);
		    		THeight = atof (THeightC); 
		    		_fmemset (&GRTextHeader,0,sizeof(GRTextHeader));
		    		GRTextHeader.lText = nchar;
					GRTextHeader.FontNum = 8;  
		 
					{
						static short	vjus=0;
						GRTextHeader.vJust=vjus;//0=above,1=baseline,2=center,3=below
					}
					GRTextHeader.hJust=1;      //0=left,1=center,2=right
					SetTextHeadSize (&GRTextHeader,THeight*NonPltFileDistToBaseDist);
				    SetTextLocVars (&TLSet); 
				    if (CopyRec)
				    {   
				    	HANDLE hGRText=0;
		                
						if (UpdateItem == 20)
		        			CurPointLocD = NewPointD;
						if (UpdateItem == 201)
		        			AdjustPointRotation (&PTRot);
						if (UpdateItem == 19)
						{   
							LPGRTEXTHEADER pGRTextHeader = (LPGRTEXTHEADER)GlobalLock (hPickedTextHeader);	 
							LPSTR	pText = (LPSTR) (pGRTextHeader+1);
							
				    		nchar = pGRTextHeader->lText;  
		            		hGRText = GRTextFromTextHeader (pGRTextHeader,pText);
							GlobalUnlock (hPickedTextHeader);	
		            	}
		            	else
		            		hGRText = GRTextFromTextHeader (&GRTextHeader,txt);
				    	AddPointToBuffer (CurPointLocD,CurrentRefno,0,CurrentDesc,10, PTRot,0,hGRText,0,
										  CurrentTAG,CurrentUDI,0,0,-1,TRUE,&hUpdateBuf,&lUpdateBuf);
						GSSiGlobFree (&hGRText);
					}
		        	else if (CurVis->WantType[2] && HaveTXLoc && (CurView->PassID || Pick)) 
		        	{   
		        		COLORREF	OldColor = -1;
		        		
		        		if (hPickedTextHeader)
					    {
					    	LPGRTEXTHEADER pPickedTextHeader = (LPGRTEXTHEADER)GlobalLock (hPickedTextHeader);
					    	_fmemmove (pPickedTextHeader,&GRTextHeader,sizeof(GRTEXTHEADER)); 
					    	pPickedTextHeader++;
					    	_fstrcpy ((LPSTR)pPickedTextHeader,txt);
					    	GlobalUnlock (hPickedTextHeader);
					    }
		
						ProcessTextObject (hDC,&GRTextHeader,txt,nchar,NULL,NULL,NULL,NULL,NULL);
						if (OldColor >= 0)
							SetTextColor (hDC,OldColor); 
						if (CurVis->WantType[8])
							DisplayPointItem (hDC,BasePtToWinPt(&CurPointLocD),10,PTRot,InvisiblePointSymbol,NULL);
					} 
				}
			else if (Type == POLYGON)
			{   
				BOOL	ShowBorder = GetBit (5,(LPSTR)&CurVis->WantType[7]); 
				HPEN	hBorderPen=0;
				
				if (Pick)
				{   
					PickPolygonD (pPoints,NumPoints,9999999,0);
				}
				else if (CopyRec)  
				{
					
					pNumPoints = (LPWORD)GlobalLock (hPolyPartLen);
					if (nPoly > 1)
					{
					}
					AddPolyToBuffer (nPoly,pNumPoints,&hPoints,TYPE_AREA,CurrentRefno,NULL,-1,CurrentDesc,0,CurrentTAG,CurrentUDI,
                                     -1,-1,0,0,0,0,0,TRUE,&hUpdateBuf,&lUpdateBuf); 
                    GlobalUnlock (hPolyPartLen);
                }
				else
				{    
					if (Display)
					{
						hOldPen  = SelectObject(hDC,h0Pen);
						if (ShowBorder)
		    				hBorderPen = hAreaBorderPen[TRUE];
			        	if (GetInVisibility(CurrentDesc))
			        	{
			        		if (CurView->HaveLayerColor[FileNum])
			        		{
			        			hTempBrush = CreateSolidBrush(CurView->LayerColor[FileNum]);
								hOldBrush = SelectObject (hDC,hTempBrush);
								hTempPen = CreatePen (PS_SOLID,0,CurView->LayerColor[FileNum]);
                                SelectObject (hDC,hTempPen);
                            }
			        		else
								hOldBrush = SelectRandomBrush (hDC,CurrentRefno,&hBorderPen,CurrentDesc); 
						} 
						if (hBorderPen)
							SelectObject(hDC,hBorderPen); 
					}
					lpDCurPoints = pPoints;
					nPnts = nCurPoints = NumPoints;

					if (PolyInMaskAreaFileCoord (GF_AREA,&NumPoints,NULL,NULL,&pPoints,TRUE))
					{
						if (SetDisplayChar (hDC,GF_AREA,CurrentRefno,CurrentDesc,CurrentTAG,CurrentUDI))
						{   
							BOOL DoBorder = FALSE;
							
							if (Display)
							{
								if (nPoly > 1 && !ShowLinkLines)
									SelectObject(hDC,h0Pen); 
								if (FillAreas && (GetBit (7,(LPSTR)&CurVis->WantType[7]) ||
												  GetBit (6,(LPSTR)&CurVis->WantType[7])))
									hDeletePen = GWPolygonD (hDC,pPoints, -NumPoints, nPoly, hPolyPartLen,CurrentDesc,ShowBorder);
								else
									DoBorder = TRUE; 
								if ((DoBorder || nPoly > 1) && hBorderPen)
								{ 
									if (hDeletePen)
										SelectObject(hDC,hDeletePen); 
									else
										SelectObject(hDC,hBorderPen); 
									pNumPoints = (LPWORD)GlobalLock (hPolyPartLen);
					        		for (i=0;i<nPoly;i++)
					        		{   
										GWPolylineD (hDC,lpDCurPoints,-(long)*pNumPoints,0); 
										lpDCurPoints+=*pNumPoints++;
										if (i)
											lpDCurPoints++;
					        		}   
					        		GlobalUnlock (hPolyPartLen); 
					        	}
					        }
						}
					}
				}
			}
			GSSiGlobFree (&hPolyPartLen);
			GSSiGlobUlFree (&hPoints); 
			GSSiGlobUlFree (&hElemInfo);
			GSSiGlobUlFree (&hOrd);
		break;
		
	}
	if (hOldBrush)
		SelectObject (hDC,hOldBrush);
	if (hOldPen)
		SelectObject (hDC,hOldPen); 
   	GSSiDeleteObject (&hDeletePen);
   	GSSiDeleteObject (&hTempBrush);
   	GSSiDeleteObject (&hTempPen);
	ShowValue (hDC,FALSE); 
	InGraphicsProcessor = FALSE;
	return TRUE;  
RtnFalse:
	InGraphicsProcessor = FALSE;
	return FALSE;
}

long ReadORAHeader (HFILE FidORA,LPMNMXCORD pMinMaxCoord)
{
	UINT		i;  
	char		mess[128];  
	short		Err;
	char		SymbolName[34];
	
	GSSillseek (FidORA,0,0);
	GSSilread (FidORA,(HPSTR)&ORAFileHeader,(UINT)sizeof(ORAFileHeader)); 
	NumORARecs = ORAFileHeader.NumRecs; 
	/*if (*ORAType) 
	{   
		char	mess[64];
			
		sprintf (mess,"Shape file type %s not yet implemented",ORAType);
		GSSiMessageBox (mess,NULL,MB_ICONEXCLAMATION);
		return FALSE;
	} */
	if (pMinMaxCoord)
		ConvertRectCoord (pMinMaxCoord,&ORAFileHeader.Bounds,0,1);
	GSSillseek (FidORA,ORAFileHeader.SymlistOffset,0);
	for (i=0;i<ORAFileHeader.NumSyms;i++)
	{   
		short	ibeg=0;
		
		GSSilread (FidORA,(HPSTR)SymbolName,sizeof(SymbolName));   
		if (*SymbolName == '%')
			ibeg = 1;
		OracleSymbols[i] = GetDictSymbolNumber (&SymbolName[ibeg]);
		if (!OracleSymbols[i])
		{   
/*			if (!GetCellLibSymbol (&SymbolName[ibeg]))
			{
				sprintf (mess,"Oracle symbol %s not defined",SymbolName);
				GSSiMessageBox (mess,NULL,MB_ICONEXCLAMATION);
				OracleSymbols[i] = ORADefaultSymbol;
			} */
				OracleSymbols[i] = ORADefaultSymbol;
		} 
	}
	return ORAFileHeader.Type;
}  

BOOL GetORATableName (LPSTR ORAFileName,LPSTR ORATableName)
{
	HFILE	Fid=GSSiOpenFile (ORAFileName,NULL,OF_READ);
	MNMXCORD Bounds;
	
	*ORATableName = 0;
	if (Fid != HFILE_ERROR)
	{
		ReadORAHeader (Fid,&Bounds);
		_fstrcpy (ORATableName,ORAFileHeader.TableName);
	}
	
	return TRUE;
}

BOOL LoadORAParm (LPSTR ORAFileName,long Type)
{
	char	Name[128], str[260], Projection[128];
	short	l;
	HFILE	Fid; 
    LPSTR	pDot;
                 
	OpenDGNCellLibrary ("[%CELLLIBRARY]");
	ORADefaultSymbol = GetDictSymbolNumber ("CIRCLE");
	ORAPointSize = GetGlobalDVal2 ("[ORAPOINTSIZE]",5);  
	GetGlobalCVal ("[%DefaultORATag]",ORATAG,"[ORA.PREFIX]:[ORA.UDI]"); 
	GetGlobalCVal ("[%DefaultOracleProjection]",Projection,"BASEPROJ"); 
	LoadProjection(0,Projection); 
	PRJ_UNITS[0] = 1;
	
/*	NumSHPParms = 0;  
	_fmemset (SHPParms,0,sizeof(SHPParms)); 
	switch (Type)
	{
		case SHPT_POINT:
		GetGlobalCVal ("[%DefaultShapePointSymbol]",SHPParms,"DUMMYPT");  
		pWidth = _fstrchr (SHPParms,0)+4;
		GetGlobalCVal ("[%DefaultShapePointSize]",pWidth,"-5"); 
		break;
		
		case SHPT_ARC:
		case SHPT_ARCZ:
		case SHPT_ARCM:
			GetGlobalCVal ("[%DefaultShapeLineSymbol]",SHPParms,"PEN1"); 
		break;

		case SHPT_POLYGON:
		case SHPT_POLYGONM:
		case SHPT_POLYGONZ:
			GetGlobalCVal ("[%DefaultShapeAreaSymbol]",SHPParms,"PARCEL"); 
		break;
	}
*/  
	if (Type == ORAT_POINT)
		_fstrcpy (ORAazm,"[ORA.AZM]");
	else
		*ORAazm = 0;
	ORABaseRefno = 0;
	_fstrcpy (Name,ORAFileName);
	pDot = _fstrrchr (Name,'.');
	if (!pDot)
		return FALSE;
	_fstrcpy (pDot,".gop");
	Fid = GSSiOpenFile (Name,NULL,OF_READ); 
	if (Fid == HFILE_ERROR)
		return FALSE;
	if (fgetstring (str,64,Fid))
		ORABaseRefno = atol (str);
	if (fgetstring (str,100,Fid))
		_fstrcpy (ORATAG,str);
	GSSiClose (Fid);
  
	return TRUE;
} 

