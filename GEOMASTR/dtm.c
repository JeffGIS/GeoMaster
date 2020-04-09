#include "graphint.h"
#include "bci.h"

static	short	NumRawPointsToUse=4;

#include "gmextern.h"
#include "laszip_dll.h"

static	HANDLE	hOpenSurf[MAXOPENSURF]; 
static	long	NextDTMUse=LONG_MIN;
static	POINT	LidarCellOffsets[9]={0,0,-1,0,-1,1,0,1,1,1,1,0,1,-1,0,-1,-1,-1};    
static	MNMXCORD	DTMBounds;
static	DPOINT	DTMPoint;
static	HANDLE	DTMCellHandle=0;
static	HANDLE	DTMSubCellHandle=0;   
static	long	DTMCellID;  
static	short	DTMPointSymbol, DTMSlopeArrowSymbol, DTMSlopeAreaSymbol,GridLineDesc; 
static	short	DTMRenderAs=DTM_RENDER_GRID_POINTS; 
static	double	DTMRenderGridSpacing=1.0;  
static	double	DTMSlopeArrowFactor=1; 
static	double	DTMContourInterval;
static	double	ContourIntervals[7]={1.0,2.0,5.0,10.0,20.0,50.0,100.0};
static	short	DTMSmooth;
static	HANDLE	hDTMRenderGridRow[3]={0,0,0};
static	long	DTMRow;
static	DPOINT	DTMRenderRowBeginPoint[3];     
static	long	DTMNumPointsInRenderGridRow;  
static	short	DTMRenderUnits=0; 
static	short	DTMTriPointRow[4][2]={0,2,2,2,2,0,0,0};
static	short	DTMTriPointCol[4][2]={0,0,0,1,1,1,1,0};   
static	short	TriPointXOffset[4][3]={0,0,1,0,2,1,2,2,1,2,0,1};
static	short	TriPointYOffset[4][3]={0,2,1,2,2,1,2,0,1,0,0,1};
static	DPOINT	DTMSubCellPoint;
static	BOOL	ShowGridLines; 
static	short	LightContourSymbol, DarkContourSymbol;
static	DPOINT	GridOriginPoint;
static	short	TriConnectingSideOverInc[4][3]={-1,0,0,0,0,0,1,0,0,0,0,0};
static	short	TriConnectingSideUpInc[4][3]  ={0,0,0,1,0,0,0,0,0,-1,0,0};
static	short	TriConnectingSideTriNum[4][3] ={2,1,3,3,2,0,0,3,1,1,0,2};
static	short	TriConnectingSide[4][3]	   ={0,2,1,0,2,1,0,2,1,0,2,1};
static	short	TriCornerTriOverInc[4][3]={-1,-1,0,-1,1,0,1, 1,0, 1,-1,0};
static	short	TriCornerTriUpInc[4][3]  ={-1, 1,0, 1,1,0,1,-1,0,-1,-1,0};
static  short	TriCornerTriTriNum[4][3] ={ 2, 2,2, 3,3,3,0, 0,0, 1, 1,1};
static	HANDLE	hDTMBasins=0;  
static	double	BasinsNullElv=FLT_MAX;
static 	HANDLE	hComputedTriangles; 
static	double	SlopeCorrection=-HALFPI;   
static	BOOL	ShowTriSlopes=FALSE;
static	double	FlatSlope;   
static	HANDLE	hContourLines[MAXCONTOURLINES];
static	USHORT	nContourLines=0;
static	double	CurNullElv;
static	COLORREF LightContourColor, DarkContourColor, ContourTextColor;
static	short	ContourTextSize;  
static	double	LightContourWidth, DarkContourWidth;

double LTWOPImacro (double AZ1)
#if ENABLETRACE
{GSSiEnterProg (375);
#endif
{
      if (AZ1<0)
{
#if ENABLETRACE
GSSiExitProg (375);
#endif
      	return (TWOPI + fmod(AZ1,TWOPI));
}
      if (AZ1 == 0)
{
#if ENABLETRACE
GSSiExitProg (375);
#endif
      	return (AZ1);
}
{
#if ENABLETRACE
GSSiExitProg (375);
#endif
      return (fmod(AZ1,TWOPI));
}
#if ENABLETRACE
}
#endif
}

DPOINT dnewptmacro (DPOINT OldPoint, double AZM, double DIS)
#if ENABLETRACE
{GSSiEnterProg (316);
#endif
{   DPOINT NewPoint;

      NewPoint.x=OldPoint.x+DIS*cos(AZM);
      NewPoint.y=OldPoint.y+DIS*sin(AZM);
{
#if ENABLETRACE
GSSiExitProg (316);
#endif
      return (NewPoint);
}
#if ENABLETRACE
}
#endif
}

double ldistppmacro(LPDPOINT Point1, LPDPOINT Point2)
#if ENABLETRACE
{GSSiEnterProg (379);
#endif
{
{
#if ENABLETRACE
GSSiExitProg (379);
#endif
    return (sqrt (pow(Point1->x-Point2->x,2)
            +pow(Point1->y-Point2->y,2)));
}
#if ENABLETRACE
}
#endif
}

double getazdmacro (HPDPOINT Point1, HPDPOINT Point2)
#if ENABLETRACE
{GSSiEnterProg (373);
#endif
{   
	double 	rtn=LTWOPImacro(atan2((Point2->y-Point1->y),(Point2->x-Point1->x)));
{
#if ENABLETRACE
GSSiExitProg (373);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}

double DeltaAZmacro (double AZ1, double AZ2)
{         
	double daz;
	
	daz =  AZ2 - AZ1;
	if (daz < -PY)
		daz += TWOPI;
	else if (daz > PY)
		daz -= TWOPI;
	return daz;
}   


 
HANDLE NEXPND (short INUM,LPSHORT INPUTI)
																							#if ENABLETRACE
																							{GSSiEnterProg (1351);
																							#endif
{
	UINT	I, NEXT=0, J, LAST; 
	short	NOFJ, ELV;
	long	INC;
	HANDLE	handle=GSSiGlobAlloc (1089,GMEM_MOVEABLE,1024*sizeof(short));
	LPSHORT	OUTPUT=(LPSHORT)GlobalLock (handle);
	
    for (I=0;I<INUM;I++)
    {
         if (INPUTI[I] <= INDCOD) 
         {
         	if (NEXT > 1023)
         		goto Done;
         	OUTPUT[NEXT++]=INPUTI[I];
         }
         else
         {
	         NOFJ=INPUTI[I]-INDCOD;
	         LAST = NEXT - 1;
	         INC = (INPUTI[I+1] - INPUTI[I-1]) / (NOFJ + 1);
	         for (J=0;J<NOFJ;J++)  
	         {
	         	if (NEXT > 1023)
	         		goto Done;
	            OUTPUT[NEXT++] = OUTPUT[LAST++] + INC; 
	         }
	     }
	} 
	if (NEXT > 1024)
		goto ErrOut;  
	if (NEXT < 1024)       
	{
		ELV = OUTPUT[NEXT-1];
		while (NEXT < 1024)  
			OUTPUT[NEXT++] = ELV;
	} 
Done:
	GlobalUnlock (handle);
{
																							#if ENABLETRACE
																							GSSiExitProg (1351);
																							#endif
    return handle;
}
ErrOut:
	GSSiGlobUlFree (&handle);
{
																							#if ENABLETRACE
																							GSSiExitProg (1351);
																							#endif
	return 0;
}
																							#if ENABLETRACE
																							}
																							#endif
}  

void DTMOutputType1Run (short nRun,long CURDIF,LPSTR CompressedDTMData,LPSHORT pNextCmp)
																							#if ENABLETRACE
																							{GSSiEnterProg (1352);
																							#endif
{   
	short	ii;
	
	if (nRun > 255)
	{   
		while (nRun >= 0)
		{
			CompressedDTMData[(*pNextCmp)++] = 94;
			CompressedDTMData[(*pNextCmp)++] = min (nRun,254)-128;
			CompressedDTMData[(*pNextCmp)++] = CURDIF;
			nRun -= 255;  
		}
	} 
	else if (nRun > 32)
	{
		CompressedDTMData[(*pNextCmp)++] = 94;
		CompressedDTMData[(*pNextCmp)++] = nRun-128;
		CompressedDTMData[(*pNextCmp)++] = CURDIF;
	}
	else
	{
		CompressedDTMData[(*pNextCmp)++] = 94 + nRun;
		CompressedDTMData[(*pNextCmp)++] = CURDIF;
	}
{
																							#if ENABLETRACE
																							GSSiExitProg (1352);
																							#endif
	return;
}
																							#if ENABLETRACE
																							}
																							#endif
}  

void DTMOutputType2Run (short nRun,long CURDIF,LPSHORT CompressedDTMData,LPSHORT pNextCmp)
																							#if ENABLETRACE
																							{GSSiEnterProg (1353);
																							#endif
{   
	short	ii;
	
	CompressedDTMData[(*pNextCmp)++] = 31100 + nRun;
	CompressedDTMData[(*pNextCmp)++] = CURDIF;
{
																							#if ENABLETRACE
																							GSSiExitProg (1353);
																							#endif
	return;
}
																							#if ENABLETRACE
																							}
																							#endif
}  

void ExpandSubcell (LPLONG DTMData,SUBCELLINFO SUBCELLInfo,LPSTR CompressedData,LPLONG pBias)
																							#if ENABLETRACE
																							{GSSiEnterProg (1354);
																							#endif
{   
	UINT	ExpLoc=1, CmpLoc=1, ii;
	
	switch (SUBCELLInfo.TYPE)
	{   
		case 0:
			for (ExpLoc=0;ExpLoc<1024;ExpLoc++)
				DTMData[ExpLoc] = *pBias;
		break; 
			
		case 1: 
		{
			signed char	FAR	*CompressedDTMData = CompressedData;
		
			if (CompressedDTMData[0] == SCHAR_MAX)
				DTMData[0] = LONG_MAX; 
			else	
				DTMData[0] = CompressedDTMData[0] + *pBias;
			do
			{
				if (CompressedDTMData[CmpLoc] == 94)
				{
					short	nRepeat = CompressedDTMData[++CmpLoc] + 128 + 1;
					short	Inc = CompressedDTMData[++CmpLoc]; 
					CmpLoc++;
					while (nRepeat--)  
					{
						DTMData[ExpLoc] = DTMData[ExpLoc-1] + Inc;
						ExpLoc++;
					}	
				}
				else if (CompressedDTMData[CmpLoc] > 94 && CompressedDTMData[CmpLoc] < SCHAR_MAX)
				{
					short	nRepeat = CompressedDTMData[CmpLoc++] - 94 + 1;
					short	Inc = CompressedDTMData[CmpLoc++]; 
					while (nRepeat--)  
					{
						DTMData[ExpLoc] = DTMData[ExpLoc-1] + Inc;
						ExpLoc++;
					}	
				}
				else 
				{
					if (CompressedDTMData[CmpLoc] == SCHAR_MAX)
						DTMData[ExpLoc++] = LONG_MAX; 
					else	
						DTMData[ExpLoc++] = CompressedDTMData[CmpLoc] + *pBias;  
					CmpLoc++;
				} 
			}
			while (CmpLoc < SUBCELLInfo.LENGTH); 
		}
		break; 
		
		case 2:
		{ 
			LPSHORT	CompressedDTMData = (LPSHORT)CompressedData; 
			short	EndLoc = SUBCELLInfo.LENGTH/2;
			
			if (CompressedDTMData[0] == SHRT_MAX)
				DTMData[0] = LONG_MAX; 
			else	
				DTMData[0] = CompressedDTMData[0] + *pBias;
			do
			{
				if (CompressedDTMData[CmpLoc] > 31100 && CompressedDTMData[CmpLoc] < SHRT_MAX)
				{
					short	nRepeat = CompressedDTMData[CmpLoc++] - 31100 + 1;
					short	Inc = CompressedDTMData[CmpLoc++]; 
					while (nRepeat--)  
					{   
						if (ExpLoc > 1023)
							ii=1;
						DTMData[ExpLoc] = DTMData[ExpLoc-1] + Inc;
						ExpLoc++;
					}	
				}
				else 
				{
					if (CompressedDTMData[CmpLoc] == SHRT_MAX)
						DTMData[ExpLoc++] = LONG_MAX; 
					else	
						DTMData[ExpLoc++] = CompressedDTMData[CmpLoc] + *pBias;
					CmpLoc++;
				} 
			}
			while (CmpLoc < EndLoc);  
		}
		break;  
		
		case 3:
		{
			LPLONG CompressedDTMData = (LPLONG)CompressedData;  
			short	EndLoc = SUBCELLInfo.LENGTH/4;
			
			for (ExpLoc=0;ExpLoc<EndLoc;ExpLoc++)
				if (CompressedDTMData[ExpLoc] < LONG_MAX)
					DTMData[ExpLoc] = CompressedDTMData[ExpLoc] + *pBias;
				else
					DTMData[ExpLoc] = CompressedDTMData[ExpLoc];
			for (ExpLoc=EndLoc;ExpLoc<1024;ExpLoc++) //added to fix problem in mpls 25ft surf with 0 length subcell
				DTMData[ExpLoc] = *pBias;
		}
	}
{
																							#if ENABLETRACE
																							GSSiExitProg (1354);
																							#endif
	return;
}
																							#if ENABLETRACE
																							}
																							#endif
}

SUBCELLINFO CompressSubcell (LPLONG DTMData,LPSTR CompressedData,LPLONG	Bias)
																							#if ENABLETRACE
																							{GSSiEnterProg (1355);
																							#endif
{
	SUBCELLINFO	SUBCELLInfo;
	long	MinElv=LONG_MAX, MaxElv=LONG_MIN;
	UINT	i,CmpLen=0;  
	BOOL	HaveData=FALSE;
	
	SUBCELLInfo.INDT = FALSE;
	for (i=0;i<1024;i++)
	{   
		if (DTMData[i] < LONG_MAX)
		{   
			HaveData=TRUE;
			MinElv = min (MinElv,DTMData[i]);
			MaxElv = max (MaxElv,DTMData[i]); 
		}
		else
			SUBCELLInfo.INDT = TRUE;
	} 
	if (!HaveData)
	{ 
		*Bias = LONG_MAX;
		SUBCELLInfo.LENGTH = 0; 
		SUBCELLInfo.TYPE = 0;		
	}
	else if (MinElv == MaxElv && !SUBCELLInfo.INDT)
	{ 
		*Bias = MinElv;
		SUBCELLInfo.LENGTH = 0; 
		SUBCELLInfo.TYPE = 0;		
	}
	else if (MaxElv - MinElv < 222)
	{   
		long	CURDIF=LONG_MAX, DIF; 
		short	nRun=-1;
		USHORT	NextData=1, NextCmp=0;
		signed char	FAR	*CompressedDTMData = CompressedData;
		
		SUBCELLInfo.TYPE = 1;
		*Bias = MaxElv - 93; 
			
		while (NextData < 1025)
		{   
			if (NextData < 1024)
				DIF = DTMData[NextData] - DTMData[NextData-1];
			else
				DIF = LONG_MAX;
			if (DIF == CURDIF)
				nRun++;
			else if (nRun > 0)
			{
				DTMOutputType1Run (nRun,CURDIF,CompressedDTMData,&NextCmp);
				nRun = 0;
				CURDIF = DIF;
			}
			else 
			{
				if (DTMData[NextData-1] == LONG_MAX)
					CompressedDTMData[NextCmp++] = SCHAR_MAX;		
				else	                                             
					CompressedDTMData[NextCmp++] = DTMData[NextData-1] - *Bias; 
				nRun = 0;
				CURDIF = DIF;
			}
			NextData++;
		}
		SUBCELLInfo.LENGTH = NextCmp;
	}
	else if (MaxElv - MinElv < 31100)
	{   
		long	CURDIF=LONG_MAX, DIF; 
		short	nRun=-1;
		USHORT	NextData=1, NextCmp=0;
		LPSHORT	CompressedDTMData = (LPSHORT)CompressedData;
		
		SUBCELLInfo.TYPE = 2;
		*Bias = MaxElv - 31099; 
			
		while (NextData < 1025)
		{   
			if (NextData < 1024)
				DIF = DTMData[NextData] - DTMData[NextData-1];
			else
				DIF = LONG_MAX;
			if (DIF == CURDIF)
				nRun++;
			else if (nRun > 0)
			{
				DTMOutputType2Run (nRun,CURDIF,CompressedDTMData,&NextCmp);
				nRun = 0;
				CURDIF = DIF;
			}
			else 
			{
				if (DTMData[NextData-1] == LONG_MAX)
					CompressedDTMData[NextCmp++] = SHRT_MAX;		
				else	                                             
					CompressedDTMData[NextCmp++] = DTMData[NextData-1] - *Bias; 
				nRun = 0;
				CURDIF = DIF;
			}
			NextData++;
		}
		SUBCELLInfo.LENGTH = NextCmp*2;
	}
	else
	{
		LPLONG CompressedDTMData = (LPLONG)CompressedData;
		
		SUBCELLInfo.TYPE = 3;
		*Bias = (MaxElv - MinElv) / 2;
		for (i=0;i<1024;i++)
		{
			if (DTMData[i] < LONG_MAX)
			{   short	ii;
				if (DTMData[i]<100)
					ii=1;
				CompressedDTMData[i] = DTMData[i] - *Bias;
			}
			else
				CompressedDTMData[i] = DTMData[i];
		}
		SUBCELLInfo.LENGTH = 4096;
	}
{
																							#if ENABLETRACE
																							GSSiExitProg (1355);
																							#endif
	return SUBCELLInfo;
}
																							#if ENABLETRACE
																							}
																							#endif
} 

BOOL LoadERDASDem (void)
																							#if ENABLETRACE
																							{GSSiEnterProg (1357);
																							#endif
{
	typedef struct erdhead {
	  char hdword[6];
	  short pack,bands;
	  char res1[6];
	  long cols,rows,col1,row1;
	  char res2[56];
	  short maptyp,nclass;
	  char res3[14];
	  short utyp;
	  float area,xl,yt,xcell,ycell;
	} erdhead;
	erdhead eh; 
	HFILE	Fid;
	OFSTRUCTGM	OFStruct;
//	static	short	row[19580]; 
	UINT	irow, icol;   
	DPOINT	Point;
	HFILE		Fid1, Fid2;
	short		ii, length;  
	UINT		i, n, pos;
	long		GeoSeg, Bias, nFiles=0, MaxFiles=50, nOutOfRange=0, nSubCells=0, nLess12=0;
	short		NumElv, SubCell, Indeterminate, MinElv, MaxElv, ERDASRow[32], SubcellRow;    
	BOOL		OutOfRange; 
	short		nSubcells, icell; 
	long		LastLen; 
	BTVARDESC BTVar[2], *pVars;
	short		NumFields, Reclen, len;
	long	Offset;
	long	TotFileLen;
	GWDHEADER16 GWDHead; 
	LPGWDHEADER	lpGWDHead;
	GWDHEADER	GWDHead32;
	GWFLDINFO GWFldInfo;
	LPGWFLDINFO	lpGWFldInfo;
	HANDLE hBT, hDB=0;
	HFILE	FidData;
	int		ibeg;
	GWFLDINFO FldInfo;
	char	File[128]="[%DL]attribut\\mndtm30.dtm"; 
	LPSTR	lpDot; 
	HANDLE	hCell;
	LPSHORT	SubcellDat; 
	LPLONG	pBias;  
	static	long	debugsubcell=845;  
	long	TotLen, CurLoc, SubCellID;
	DTMKEY		DTMKey;
	DTMDATA	DTMData;
	SUBCELLINFO	SUBCELLInfo;
	LPSUBCELLINFO	pSUBCELLInfo;
	LPSTR		CompressedDTMData;  
	HANDLE	hSubcellHandles;
	LPHANDLE	SubcellHandles;     
	LPLONG	DTMRow;   
	long	INTX,INTY, DTMKeyl;  
	short	IELV;
	double	SPX,SPY, ymod;  
	HANDLE	hData=GSSiGlobAlloc (1090,GMEM_MOVEABLE,4096+2048);
	LPLONG		DTMElev, DTMData2=(LPLONG)GlobalLock (hData);  
	LPSHORT		SubcellData=(LPSHORT)(DTMData2+1024);
	
	
	 lpGWDHead = &GWDHead32; 
    _fmemset (&GWDHead,0,sizeof(GWDHEADER16));

	 FidData = GSSiOpenFile (File,&OFStruct,OF_CREATE);
	 GWDHead.NumFields=0;
	 GWDHead.NumIndex=1;
	 GWDHead.Version=2;
	 GWDHead.NumIndexFields[0]=1;
	 GWDHead.IndexFields[0][0]=0;
	 BigWrite (FidData,(HPSTR)&GWDHead,sizeof(GWDHEADER16),-1);
	ibeg = 0;

	FldInfo.Len = sizeof(DTMKEY);
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"DTMKEY");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"BIAS");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = sizeof(SUBCELLINFO);
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"SUBCELLINFO");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	
	FldInfo.Len = 4096;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"COMPRESSEDNODES");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;
	
	 GWDHead.Reclen=ibeg; 
	 GWDHead.TimeStamp = 0;
	 GSSillseek (FidData,0,0);
	 BigWrite (FidData,(HPSTR)&GWDHead,sizeof(GWDHEADER16),-1);
 	 GSSillseek (FidData,0,2);
			     
	 BTVar[0].BT_VARLEN=4;
	 BTVar[0].BT_VARTYP=BT_INTEGER;
	 BTVar[0].BT_VAROFF=0;
	 lpDot = _fstrrchr (File,'.');
	 _fstrcpy (lpDot,".in1");	
	 BT_CREATE (File, 4, FALSE, 1, 1,BTVar,FALSE, 0, 0, FALSE);
 	 GSSiClose2 (&FidData);
	 _fstrcpy (lpDot,".dtm");	

     hDB = OpenGWDatabase (File,BT_WRITE);
	lpGWDHead = (LPGWDHEADER)GlobalLock (hDB); 
	pSUBCELLInfo = (LPSUBCELLINFO)((LPSTR)&lpGWDHead->GWDData + (sizeof(DTMKEY)));
	pBias = (LPLONG)((LPSTR)&lpGWDHead->GWDData + (sizeof(DTMKEY) + sizeof(SUBCELLINFO)));
	CompressedDTMData = (LPSTR)&lpGWDHead->GWDData + (sizeof(DTMKEY) + sizeof(SUBCELLINFO) + 4);
	
	Fid = GSSiOpenFile ("g:\\usdata\\dem\\demerd\\statedem.gis",&OFStruct,OF_READ); 
	TotLen = GSSillseek (Fid,0,2);
	GSSillseek (Fid,0,0);
	CreateStatusWind (hWndMain,1,0);
	BigRead (Fid,(HPSTR)&eh,sizeof(erdhead));  
	_fmemset (&DTMData,0,sizeof(DTMData));
	DTMData.GridSpace = eh.xcell;
	DTMData.SouthWestNode.x = (double) eh.xl;
	DTMData.SouthWestNode.y = (double) eh.yt - (long)(eh.rows-1) * (long)eh.ycell;   
	DTMData.Bounds.ymn = DTMData.SouthWestNode.y;
	ymod = fmod ((double) eh.yt,DTMData.GridSpace*32);  
	DTMData.SouthWestNode.y -= (DTMData.GridSpace*32 - ymod);
	DTMData.Bounds.xmn = eh.xl;
	DTMData.Bounds.xmx = eh.xl + DTMData.GridSpace * (eh.cols-1);
	DTMData.Bounds.ymx = eh.yt;
	
	irow = eh.rows; 
	nSubcells = (eh.cols-1) / 32 + 1;
	LastLen = eh.cols % 32;  
	if (!LastLen)
		LastLen = 32;
	hSubcellHandles = GSSiGlobAlloc (1091,GHND,nSubcells*sizeof(HANDLE));
	SubcellHandles = (LPHANDLE)GlobalLock (hSubcellHandles); 
	for (i=0;i<nSubcells;i++)
		SubcellHandles[i] = GSSiGlobAlloc (1092,GMEM_MOVEABLE,1024*4);
	SubcellRow=31; 
	SPY = eh.yt - 16 * eh.ycell;
	while (ContinueProcessing && irow--)
	{                          
		for (icell = 0;icell<nSubcells;icell++)
		{   
			DTMRow = (LPLONG)GlobalLock (SubcellHandles[icell]);
			DTMRow += 32 * SubcellRow; 
			if (icell < nSubcells-1)
				BigRead (Fid,(HPSTR)ERDASRow,32*2);   
			else
			{
				BigRead (Fid,(HPSTR)ERDASRow,(size_t)(LastLen*2));
				for (icol = LastLen;icol<32;icol++)
					ERDASRow[icol] = -1;
			} 
			for (icol=0;icol<32;icol++)
				if (ERDASRow[icol] > -1)
					DTMRow[icol] = ERDASRow[icol];
				else
					DTMRow[icol] = LONG_MAX;
			GlobalUnlock (SubcellHandles[icell]);
		}
		if (!SubcellRow || !irow)
		{   
			SPX = eh.xl + 16 * eh.xcell;
			for (icell = 0;icell<nSubcells;icell++)
			{   
				DTMElev = (LPLONG)GlobalLock (SubcellHandles[icell]);
				INTX   = (SPX - DTMData.SouthWestNode.x) / DTMData.GridSpace;
				INTY   = (SPY- DTMData.SouthWestNode.y) / DTMData.GridSpace;
				NGSANE (INTX,INTY,&GeoSeg,&SubCell,&IELV); 
				GeoSeg--;
				SubCell--;
				DTMKey.GEOSEG_ROW = GeoSeg / 4096; 
				DTMKey.GEOSEG_COL = GeoSeg % 4096;    
				DTMKey.SUBCEL_ROW = SubCell / 16;
				DTMKey.SUBCEL_COL = SubCell % 16;    
				n=1024;
				MinElv = SHRT_MAX;
				MaxElv = SHRT_MIN;   
				SUBCELLInfo = CompressSubcell (DTMElev,CompressedDTMData,pBias);  
				_fmemset (DTMData2,0,4096); 
				ExpandSubcell (DTMData2,SUBCELLInfo,CompressedDTMData,pBias);
				for (i=0;i<1024;i++)
					if (DTMElev[i] != DTMData2[i]) 
					{
						GSSiMessageBox (0,"decompress error",0,MB_ICONEXCLAMATION,0);
						goto Exit;        
					} 
				if (*pBias < LONG_MAX)
				{
					*pSUBCELLInfo = SUBCELLInfo;
					Offset = GSSillseek (lpGWDHead->Fid,0,1);  
					length = sizeof(DTMKEY) + sizeof(SUBCELLINFO) + 4 + SUBCELLInfo.LENGTH;
				    BigWrite (lpGWDHead->Fid,(HPSTR)&length,2,-1);
				    BigWrite (lpGWDHead->Fid,(HPSTR)&lpGWDHead->GWDData,length,-1);
		        	BT_PUT (lpGWDHead->BTHandle[0],(LPSTR)&DTMKey,(LPSTR)&Offset); 
		        }
		        SPX += 32 * eh.xcell;
				GlobalUnlock (SubcellHandles[icell]);
		    } 
	        SPY -= 32 * eh.ycell;
			SubcellRow=32; 
		}
		SubcellRow--; 
Exit: 
		CurLoc = GSSillseek (Fid,0,1);
		StatusWindowUpdate ("","", TotLen, CurLoc);
	}    
	Offset = GSSillseek (lpGWDHead->Fid,0,1);  
	length = sizeof(DTMDATA);
    BigWrite (lpGWDHead->Fid,(HPSTR)&length,2,-1);
    BigWrite (lpGWDHead->Fid,(HPSTR)&DTMData,length,-1);   
    DTMKeyl = LONG_MAX;
	BT_PUT (lpGWDHead->BTHandle[0],(LPSTR)&DTMKeyl,(LPSTR)&Offset); 
	SetContinueProcessing ( TRUE);
	GSSiClose2 (&Fid);   
	DestroyStatusWindow (0);
	GlobalUnlock (hDB);
    CloseGWDatabase (hDB); 
	for (i=0;i<nSubcells;i++)
		GSSiGlobFree (&SubcellHandles[i]);
    GSSiGlobUlFree (&hSubcellHandles); 
    GSSiGlobUlFree (&hData);
{
																							#if ENABLETRACE
																							GSSiExitProg (1357);
																							#endif
	return TRUE;
}
																							#if ENABLETRACE
																							}
																							#endif
} 
BOOL LoadLIDARDTM(LPSTR Infiles, LPSTR OutFile, LPSTR CBounds)
{   
	LPSTR InFile = Infiles;
	LPSTR FileEnd = _fstrchr(InFile, ';'), FileEndSave;
	char	TempFile[144] = "c:\\tempdtm.bin", str[130], mess[256];
	HANDLE	TxtHandle = 0;
	double	X, Y, Z;
	long	IX, IY, IZ;
	long	CurrentCell = -1, Cell, lineno;
	DWORD	FileLength, NumRows, NumCols, loc, TotLen, CurLoc;
	long	CellRow, CellCol, DataOffset, CellOffset, CellNo = 0, IndexSize;
	HFILE	FidIn, FidOut;   
	double	MinX = DBL_MAX, MaxX = -DBL_MAX, MinY = DBL_MAX, MaxY = -DBL_MAX, MinZ = DBL_MAX, MaxZ = -DBL_MAX;
	double	FileMinX, FileMinY;
	long	LidarDist[MAXLIDARPERREC + 1];
	double	CellMinX=0, CellMinY=0; 
	int		i;
     
	LIDARREC	LidarRec = { 0 };
    LIDARFILEHEADER	Header;
	
	AllowCache = FALSE;
	UndoEnabled = FALSE; 
	if (FileEnd)
		*FileEnd++ = 0;
	FileEndSave = FileEnd;
	if (*CBounds)
	{   
		MNMXCORD Bounds; 
		BOOL	err;
		
		Bounds = atobounds(CBounds, &err);
		if (err) 
		{
			sprintf(mess, "Error in bounds:%s", CBounds);
			MessageBox(0, mess, 0, MB_ICONEXCLAMATION);
			return FALSE;
		}
		MinX = Bounds.xmn;
		MinY = Bounds.ymn;
		MaxX = Bounds.xmx;
		MaxY = Bounds.ymx; 
	}
	else
	{       
		FidIn = GSSiOpenFile(InFile, 0, OF_READ);
		TotLen = GSSillseek(FidIn, 0, 2);
		GSSillseek(FidIn, 0, 0);
		ProcessDelimTextHeader(str, InFile, FidIn, &TxtHandle, 0, 0);
		CreateStatusWind(hWndMain, 1, "Getting Min/Max Values");
NextFile:
		lineno = 1;
		while (ContinueProcessing && fgetstring(str, 64, FidIn))
		{   
			lineno++;
			GetDelimTextData(str, TxtHandle, 64);
			X = GetGlobalDVal("[X]")*FTM;
			Y = GetGlobalDVal("[Y]")*FTM;
			Z = GetGlobalDVal("[Z]")*FTM;
			if (X < 10)
			{
				sprintf(mess, "Invalid X at line %ld:%s", lineno, str);
				MessageBox(0, mess, 0, MB_ICONEXCLAMATION);
			}
			else
			{
				MinX = min(MinX, X);
				MaxX = max(MaxX, X);
				MinY = min(MinY, Y);
				MaxY = max(MaxY, Y);
			}
			CurLoc = GSSillseek(FidIn, 0, 1);
			StatusWindowUpdate(0, 0, TotLen, CurLoc);
	    }
		GSSiClose2 (&FidIn);
	    if (FileEnd)
	    {
	    	
			FidIn = GSSiOpenFile(FileEnd, 0, OF_READ);
	    	FileEnd = 0; 
			TotLen = GSSillseek(FidIn, 0, 2);
			GSSillseek(FidIn, 0, 0);
			goto NextFile;
		}
	    SetContinueProcessing ( TRUE);
		DestroyStatusWindow(0);
	}
	FileMinX = MinX - fmod(MinX, LIDARCELLSIZE);
	FileMinY = MinY - fmod(MinY, LIDARCELLSIZE);
	NumCols = 1 + (MaxX - FileMinX) / LIDARCELLSIZE;
	NumRows = 1 + (MaxY - FileMinY) / LIDARCELLSIZE;
    if ((double)NumRows * (double)NumCols * (double)sizeof(LIDARREC) > (double)LONG_MAX)
    {
		MessageBox(0, "Lidar file size exceeds maximum", 0, MB_ICONEXCLAMATION);
    	return FALSE;
    } 
	FileLength = NumRows * NumCols * sizeof(LIDARREC);
	CloseFidSmall();
	FidOut = GSSiOpenFile(TempFile, 0, OF_CREATE);
	GSSiChangeLength(FidOut, FileLength);
	GSSiClose2 (&FidOut);
	CreateFidSmall();
	FidOut = GSSiOpenFile(TempFile, 0, OF_READWRITE);
	FidIn = GSSiOpenFile(InFile, 0, OF_READ);
	CreateStatusWind(hWndMain, 1, "Loading Data");
	TotLen = (DWORD)GSSillseek(FidIn, 0, 2);
	GSSillseek(FidIn, 0, 0);
	GSSiGlobFree(&TxtHandle);
	ProcessDelimTextHeader(str, InFile, FidIn, &TxtHandle, 0, 0);
	FileEnd = FileEndSave;
NextFile2:
	while (ContinueProcessing && fgetstring(str, 64, FidIn))
	{
		GetDelimTextData(str, TxtHandle, 64);
		X = GetGlobalDVal("[X]")*FTM;
		Y = GetGlobalDVal("[Y]")*FTM;
		Z = GetGlobalDVal("[Z]")*FTM;
		MinZ = min(MinZ, Z);
		MaxZ = max(MaxZ, Z);
		CellCol = (X - FileMinX) / LIDARCELLSIZE;
		CellRow = (Y - FileMinY) / LIDARCELLSIZE;
		if (CellCol >= 0 && CellCol < NumCols && CellRow >= 0 && CellRow < NumRows)
		{
			Cell = NumCols * CellRow + CellCol;  
			if (Cell != CurrentCell)
			{
				if (CurrentCell > -1)
				{
					loc = (DWORD)CurrentCell * (DWORD)sizeof(LIDARREC);
					GSSillseek2(FidOut, loc, 0);
					BigWrite(FidOut, (HPSTR)&LidarRec, sizeof(LIDARREC), -1);
				}
				loc = (DWORD)Cell * (DWORD)sizeof(LIDARREC);
				GSSillseek2(FidOut, loc, 0);
				BigRead(FidOut, (HPSTR)&LidarRec, sizeof(LIDARREC));
				CurrentCell = Cell;  
				CellMinX = FileMinX + CellCol * LIDARCELLSIZE;
				CellMinY = FileMinY + CellRow * LIDARCELLSIZE;
			}
			if (LidarRec.NumPoints < MAXLIDARPERREC)
			{
				LidarRec.LidarPnt[LidarRec.NumPoints].xoff = IDNINT(1000 * (X - CellMinX));
				LidarRec.LidarPnt[LidarRec.NumPoints].yoff = IDNINT(1000 * (Y - CellMinY));
				LidarRec.LidarPnt[LidarRec.NumPoints++].Elevation = Z;
			} 
		}
		CurLoc = (DWORD)GSSillseek(FidIn, 0, 1);
		StatusWindowUpdate(0, 0, TotLen, CurLoc);
    }  
	GSSiClose2 (&FidIn);
    if (FileEnd)
    {
	    	
		FidIn = GSSiOpenFile(FileEnd, 0, OF_READ);
    	FileEnd = 0; 
		TotLen = GSSillseek(FidIn, 0, 2);
		GSSillseek(FidIn, 0, 0);
		goto NextFile2;
	}
	loc = (DWORD)CurrentCell * (DWORD)sizeof(LIDARREC);
	GSSillseek2(FidOut, loc, 0);
	BigWrite(FidOut, (HPSTR)&LidarRec, sizeof(LIDARREC), -1);
    SetContinueProcessing ( TRUE);
	DestroyStatusWindow(0);
	GSSiClose2 (&FidOut);
	FidOut = GSSiOpenFile(TempFile, 0, OF_READ);
	_fmemset(LidarDist, 0, sizeof(LidarDist));
	CreateStatusWind(hWndMain, 1, "Building Distribution");
	TotLen = (DWORD)GSSillseek(FidOut, 0, 2);
	GSSillseek(FidOut, 0, 0);
	while (ContinueProcessing && BigRead(FidOut, (HPSTR)&LidarRec, sizeof(LIDARREC)))
	{
		LidarDist[LidarRec.NumPoints]++;
		CurLoc = (DWORD)GSSillseek(FidOut, 0, 1);
		StatusWindowUpdate(0, 0, TotLen, CurLoc);
    }  
    SetContinueProcessing ( TRUE);
	DestroyStatusWindow(0);
	GSSiClose2 (&FidOut);
	GSSiGlobFree(&TxtHandle);

	for (i = 0; i<MAXLIDARPERREC + 1; i++)
	{ 
		sprintf(str, "%i\t%ld", i, LidarDist[i]);
		AppendFile("c:\\lidardist.txt", str);
	} 
	FidIn = GSSiOpenFile(TempFile, 0, OF_READ);
	DataOffset = sizeof(LIDARFILEHEADER) + NumRows * NumCols * 4;
	FidOut = GSSiOpenFile(OutFile, 0, OF_CREATE);
	Header.Version = 1;  
	Header.Bounds.xmn = FileMinX;
	Header.Bounds.xmx = MaxX;
	Header.Bounds.ymn = FileMinY;
	Header.Bounds.ymx = MaxY;  
	Header.MinElev = MinZ;
	Header.MaxElev = MaxZ;
	Header.NumRows = NumRows;
	Header.NumCols = NumCols; 
	Header.CellSpacing = LIDARCELLSIZE; 
	BigWrite(FidOut, (HPSTR)&Header, sizeof(LIDARFILEHEADER), -1);
	IndexSize = NumRows * NumCols;
	CellOffset = 0;
	while (IndexSize--)
		BigWrite(FidOut, (HPSTR)&CellOffset, 4, -1);
	CreateStatusWind(hWndMain, 1, "Creating Output File");
	TotLen = (DWORD)GSSillseek(FidIn, 0, 2);
	GSSillseek(FidIn, 0, 0);
	while (ContinueProcessing && BigRead(FidIn, (HPSTR)&LidarRec, sizeof(LIDARREC)))
	{
		if (LidarRec.NumPoints)
		{
			CellOffset = GSSillseek(FidOut, 0, 2);
			BigWrite(FidOut, (HPSTR)&LidarRec, 2 + LidarRec.NumPoints*sizeof(LIDARPNT), -1);
			GSSillseek(FidOut, (long)sizeof(LIDARFILEHEADER) + CellNo * 4, 0);
			BigWrite(FidOut, (HPSTR)&CellOffset, 4, -1);
		}
		CellNo++;
		CurLoc = (DWORD)GSSillseek(FidIn, 0, 1);
		StatusWindowUpdate(0, 0, TotLen, CurLoc);
	}
	SetContinueProcessing ( TRUE);
	GSSiClose2 (&FidIn);
	GSSiClose2 (&FidOut);
	DestroyStatusWindow(0);
	GSSiRemove(TempFile);
	return TRUE;
}

MNMXCORD GetLAZIndexBounds(LPSTR LAZIndex)
{
	MNMXCORD Bounds = { 0 };
	sqlite3 *db;
	int rtn = sqlite3_open(LAZIndex, &db);
	if (rtn == SQLITE_OK)
	{
		Bounds = SLTSpatialIndexBounds(db, "LIDAR");
		sqlite3_close(db);
	}
	return Bounds;
}
MNMXCORD3D GetLAZIndexBounds3D(LPSTR LAZIndex)
{
	MNMXCORD3D Bounds = { 0 };
	sqlite3 *db;
	int rtn = sqlite3_open(LAZIndex, &db);
	if (rtn == SQLITE_OK)
	{
		Bounds = SLTSpatialIndexBounds3D(db, "LIDAR");
		sqlite3_close(db);
	}
	return Bounds;
}

BOOL LoadLIDARDTMfromLAZ (LPSTR InDir,LPSTR OutFile,int wantType)
{   
	char	FileList[MAX_PATH], InFile[MAX_PATH];
	char	TempFile[MAX_PATH] = "c:\\temp\\tempdtm.bin";
	char	Projection[MAX_PATH];
	char	distFile[MAX_PATH];
	char	str[130], mess[256];  
	HANDLE	TxtHandle=0;
	double	X,Y,Z;    
	//LONGLONG IX,IY,IZ; 
	LONGLONG CurrentCell=-1, Cell;
	LONGLONG FileLength, NumRows, NumCols, loc;
	LONGLONG TotLen, CurLoc;
	LONGLONG CellRow, CellCol, DataOffset,  CellNo=0, IndexSize;
	LONGLONG CellOffset;
	HFILE	FidIn, FidOut;   
	double	MinX=DBL_MAX, MaxX=-DBL_MAX,MinY=DBL_MAX, MaxY=-DBL_MAX,MinZ=DBL_MAX, MaxZ=-DBL_MAX;   
	double	FileMinX, FileMinY;
	long	LidarDist[MAXLIDARPERREC+1];  
	double	CellMinX, CellMinY; 
	int		i;
	MNMXCORD3D Bounds3D;
	MNMXCORD Bounds;
    LIDARREC	LidarRec;  
    LIDARFILEHEADER	Header;
	HANDLE hDB = 0;
	laszip_point_struct* point;
	LONGLONG numMissingPoints = 0;
	LONGLONG totNumMissingPoints = 0;

	if (laszip_load_dll() == 1)
		return FALSE;

	AllowCache = FALSE;
	UndoEnabled = FALSE; 

	sprintf(distFile, "%s\\distribution.txt", InDir);
	sprintf(Projection, "%s\\projection.cvt", InDir);
	LoadProjection(0, Projection);
	sprintf(InFile, "%s\\index.la", InDir);
	Bounds3D = GetLAZIndexBounds3D(InFile);
	Bounds.xmn = Bounds3D.xmn;
	Bounds.xmx = Bounds3D.xmx;
	Bounds.ymn = Bounds3D.ymn;
	Bounds.ymx = Bounds3D.ymx;
	ConvertBounds(&Bounds, 0, 1);
	MinX = Bounds.xmn;
	MinY = Bounds.ymn;
	MaxX = Bounds.xmx;
	MaxY = Bounds.ymx;

	if (MaxX > MinX)
	{
		int blockSize = 1024 * 1024;
		LONGLONG totWritten = 0;
		LPSTR zeros = malloc(blockSize);
		memset(zeros, 0, blockSize);
		FileMinX = MinX - fmod(MinX, LIDARCELLSIZE);
		FileMinY = MinY - fmod(MinY, LIDARCELLSIZE);
		NumCols = 1 + (MaxX - FileMinX) / LIDARCELLSIZE;
		NumRows = 1 + (MaxY - FileMinY) / LIDARCELLSIZE;
		if ((double)NumRows * (double)NumCols * (double)sizeof(LIDARREC) > (double)LONG_MAX*64)
		{
			MessageBox(0, "Lidar file size exceeds maximum", 0, MB_ICONEXCLAMATION);
			return FALSE;
		}
		FileLength = NumRows * NumCols * sizeof(LIDARREC);
		CreateStatusWind(hWndMain, 1, "Initializing Temp File");
		FidOut = GSSiOpenFile(TempFile, 0, OF_CREATE);
		while (totWritten < FileLength)
		{
			int len = min(FileLength-totWritten, blockSize);
			len = BigWrite (FidOut, zeros, len,-1);
			totWritten += len;
			StatusWindowUpdate(0, 0, FileLength, totWritten);
		}
		free(zeros);
		GSSiClose2 (&FidOut);
		DestroyStatusWindow(0);
		FidOut = GSSiOpenFile(TempFile, 0, OF_READWRITE);

		sprintf(FileList, "%s\\filelist.txt", InDir);
		sprintf(mess, "Loading data for %s",FilePart(OutFile,"NAME"));
		CreateStatusWind(hWndMain, 2, mess);
		if (OpenDataFile(FileList, "", BT_READ, &hDB))
		{
			int numFiles = NumSQLRows(hDB);
			int CurFile = 0;
			char File[MAX_PATH] = "[FULLNAME]";
			char FileName[MAX_PATH] = "[FILENAME]";
			BOOL haveFile = FetchDBRec(hDB);

			ExpandText(File);
			ExpandText(FileName);
			while (StatusWindowUpdate(0, FileName, numFiles, CurFile++) && haveFile)
			{
				laszip_POINTER laszip_reader;
				if (!laszip_create(&laszip_reader))
				{
					laszip_BOOL is_compressed = 0;
					if (!laszip_open_reader(laszip_reader, File, &is_compressed))
					{
						laszip_I64 totPnts;
						laszip_header_struct* header;

						if (!laszip_get_header_pointer(laszip_reader, &header))
							totPnts = header->number_of_point_records;
						laszip_get_point_pointer(laszip_reader, &point);
						CurLoc = 0;
						while (StatusWindowUpdate2(0, totPnts, CurLoc) && CurLoc < totPnts && !laszip_read_point(laszip_reader))
						{
							if (point->classification == wantType)
							{
								DPOINT pt;
								int st;

								pt.x = point->X * header->x_scale_factor + header->x_offset;
								pt.y = point->Y * header->y_scale_factor + header->y_offset;
								st = ConvertCoord(&pt, 0, 1);
								if (st != 0)
									ii = 1;
								X = pt.x;
								Y = pt.y;
								Z = point->Z * header->z_scale_factor;
								MinZ = min(MinZ, Z);
								MaxZ = max(MaxZ, Z);
								CellCol = (X - FileMinX) / LIDARCELLSIZE;
								CellRow = (Y - FileMinY) / LIDARCELLSIZE;
								if (CellCol >= 0 && CellCol < NumCols && CellRow >= 0 && CellRow < NumRows)
								{
									Cell = NumCols * CellRow + CellCol;
									if (Cell != CurrentCell)
									{
										if (CurrentCell > -1)
										{
											loc = (LONGLONG)CurrentCell * (LONGLONG)sizeof(LIDARREC);
											GSSillseek2(FidOut, loc, 0);
											BigWrite(FidOut, (HPSTR)&LidarRec, sizeof(LIDARREC), -1);
										}
										numMissingPoints = 0;
										loc = (LONGLONG)Cell * (LONGLONG)sizeof(LIDARREC);
										GSSillseek2(FidOut, loc, 0);
										BigRead(FidOut, (HPSTR)&LidarRec, sizeof(LIDARREC));
										CurrentCell = Cell;
										CellMinX = FileMinX + CellCol * LIDARCELLSIZE;
										CellMinY = FileMinY + CellRow * LIDARCELLSIZE;
									}
									if (LidarRec.NumPoints < MAXLIDARPERREC)
									{
										LidarRec.LidarPnt[LidarRec.NumPoints].xoff = IDNINT(1000 * (X - CellMinX));
										LidarRec.LidarPnt[LidarRec.NumPoints].yoff = IDNINT(1000 * (Y - CellMinY));
										LidarRec.LidarPnt[LidarRec.NumPoints].intensity = point->intensity;
										LidarRec.LidarPnt[LidarRec.NumPoints++].Elevation = Z;
									}
									else
									{
										numMissingPoints++;
										totNumMissingPoints++;
									}
								}
								else
									ii = 1;
							}
							CurLoc++;
						}
					}
					laszip_close_reader(laszip_reader);
				}
				laszip_destroy(laszip_reader);
				haveFile = FetchDBRec(hDB);
				if (haveFile)
				{
					strcpy(File,"[FULLNAME]");
					strcpy(FileName,"[FILENAME]");
					ExpandText(File);
					ExpandText(FileName);
				}
			}
			CloseDataFile(TRUE, &hDB);
			loc = (LONGLONG)CurrentCell * (DWORD)sizeof(LIDARREC);
			GSSillseek2(FidOut, loc, 0);
			BigWrite(FidOut, (HPSTR)&LidarRec, sizeof(LIDARREC), -1);
			SetContinueProcessing ( TRUE);
			DestroyStatusWindow(0);
			GSSiClose2 (&FidOut);
			FidOut = GSSiOpenFile(TempFile, 0, OF_READ);
			_fmemset(LidarDist, 0, sizeof(LidarDist));
			CreateStatusWind(hWndMain, 1, "Building Distribution");
			TotLen = GSSillseek2(FidOut, 0, 2);
			GSSillseek2(FidOut, 0, 0);
			while (ContinueProcessing && BigRead(FidOut, (HPSTR)&LidarRec, sizeof(LIDARREC)))
			{
				LidarDist[LidarRec.NumPoints]++;
				CurLoc = GSSillseek2(FidOut, 0, 1);
				StatusWindowUpdate(0, 0, TotLen, CurLoc);
			}
			SetContinueProcessing ( TRUE);
			DestroyStatusWindow(0);
			CreateStatusWind(hWndMain, 1, "Writing Distribution");
			GSSiClose2 (&FidOut);
			GSSiGlobFree(&TxtHandle);
			GSSiRemove(distFile);
			
			sprintf(str, "Missed points:%I64i", totNumMissingPoints);
			AppendFile(distFile, str);
			for (i = 0; i < MAXLIDARPERREC + 1; i++)
			{
				sprintf(str, "%i\t%ld", i, LidarDist[i]);
				AppendFile(distFile, str);
			}
			DestroyStatusWindow(0);
			sprintf(mess, "Creating output file %s", FilePart(OutFile, "NAME"));
			CreateStatusWind(hWndMain, 1, mess);
			FidIn = GSSiOpenFile(TempFile, 0, OF_READ);
			DataOffset = sizeof(LIDARFILEHEADER) + NumRows * NumCols * sizeof(LONGLONG);
			FidOut = GSSiOpenFile(OutFile, 0, OF_CREATE);
			Header.Version = 2;
			Header.Bounds.xmn = FileMinX;
			Header.Bounds.xmx = MaxX;
			Header.Bounds.ymn = FileMinY;
			Header.Bounds.ymx = MaxY;
			Header.MinElev = MinZ;
			Header.MaxElev = MaxZ;
			Header.NumRows = NumRows;
			Header.NumCols = NumCols;
			Header.CellSpacing = LIDARCELLSIZE;
			BigWrite(FidOut, (HPSTR)&Header, sizeof(LIDARFILEHEADER), -1);
	IndexSize = NumRows * NumCols;
	CellOffset = 0;
	while (IndexSize--)
				BigWrite(FidOut, (HPSTR)&CellOffset,sizeof(LONGLONG), -1);
			TotLen = GSSillseek2(FidIn, 0, 2);
			GSSillseek2(FidIn, 0, 0);
			while (ContinueProcessing && BigRead(FidIn, (HPSTR)&LidarRec, sizeof(LIDARREC)))
	{
				if (CellNo == 3253864)
					ii = 1;
		if (LidarRec.NumPoints)
		{
					CellOffset = GSSillseek2(FidOut, 0, 2);
					BigWrite(FidOut, (HPSTR)&LidarRec, 2 + LidarRec.NumPoints*sizeof(LIDARPNT), -1);
					GSSillseek2(FidOut, (LONGLONG)sizeof(LIDARFILEHEADER) + CellNo * sizeof(LONGLONG), 0);
					BigWrite(FidOut, (HPSTR)&CellOffset, sizeof(LONGLONG), -1);
		}
		CellNo++;
				CurLoc = GSSillseek2(FidIn, 0, 1);
				StatusWindowUpdate(0, 0, TotLen, CurLoc);
    }  
    SetContinueProcessing ( TRUE);
			GSSiClose2 (&FidIn);
			GSSiClose2 (&FidOut);
			DestroyStatusWindow(0);
			GSSiRemove(TempFile);
		}
	return TRUE;
}
	return FALSE;
}

BOOL LoadGRIDDTM (LPSTR InFile, LPSTR OutFile)
																							#if ENABLETRACE
																							{GSSiEnterProg (1358);
																							#endif
{
	HFILE	Fid;
	OFSTRUCTGM	OFStruct;
	UINT	irow, icol;   
	DPOINT	Point;
	HFILE		Fid1, Fid2;
	HANDLE		hRec=GSSiGlobAlloc (1093,GMEM_MOVEABLE,USHRT_MAX);
	LPSTR		pRec;
	short		ii, length;  
	UINT		i, n, pos;
	long		GeoSeg, Bias, nFiles=0, MaxFiles=50, nOutOfRange=0, nSubCells=0, nSubCellsY, nLess12=0;
	short		NumElv, SubCell, Indeterminate, MinElv, MaxElv,  SubcellRow;    
	BOOL		OutOfRange; 
	short		nSubcells, icell; 
	long		ERDASRow[32],LastLen; 
	BTVARDESC BTVar[2], *pVars;
	short		NumFields, Reclen, len;
	long	Offset;
	long	TotFileLen;
	GWDHEADER16 GWDHead; 
	LPGWDHEADER	lpGWDHead;
	GWDHEADER	GWDHead32;
	GWFLDINFO GWFldInfo;
	LPGWFLDINFO	lpGWFldInfo;
	HANDLE hBT, hDB=0;
	HFILE	FidData;
	int		ibeg;
	GWFLDINFO FldInfo;
	LPSTR	lpDot; 
	HANDLE	hCell;
	LPSHORT	SubcellDat; 
	LPLONG	pBias;  
	static	long	debugsubcell=845;  
	long	TotLen, CurLoc, SubCellID;
	DTMKEY		DTMKey;
	DTMDATA	DTMData;
	SUBCELLINFO	SUBCELLInfo;
	LPSUBCELLINFO	pSUBCELLInfo;
	LPSTR		CompressedDTMData;  
	HANDLE	hSubcellHandles;
	LPHANDLE	SubcellHandles;     
	LPLONG	DTMRow;   
	long	INTX,INTY, DTMKeyl, nRows, nCols, RowLen, VoidVal;  
	short	IELV, nCharPerCell=10;
	double	SPX,SPY, Maxy;  
	HANDLE	hData=GSSiGlobAlloc (1094,GMEM_MOVEABLE,4096+2048);
	LPLONG		DTMElev, DTMData2=(LPLONG)GlobalLock (hData);  
	LPSHORT		SubcellData=(LPSHORT)(DTMData2+1024); 
	BOOL	rtn=FALSE;
	
	
	 lpGWDHead = &GWDHead32; 
    _fmemset (&GWDHead,0,sizeof(GWDHEADER16));

	 FidData = GSSiOpenFile (OutFile,&OFStruct,OF_CREATE);
	 GWDHead.NumFields=0;
	 GWDHead.NumIndex=1;
	 GWDHead.Version=1;
	 GWDHead.NumIndexFields[0]=1;
	 GWDHead.IndexFields[0][0]=0;
	 BigWrite (FidData,(HPSTR)&GWDHead,sizeof(GWDHEADER16),-1);
	ibeg = 0;

	FldInfo.Len = sizeof(DTMKEY);
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"DTMKEY");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"BIAS");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = sizeof(SUBCELLINFO);
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"SUBCELLINFO");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	
	FldInfo.Len = 4096;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"COMPRESSEDNODES");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;
	
	 GWDHead.Reclen=ibeg; 
	 GWDHead.TimeStamp = 0;
	 GSSillseek (FidData,0,0);
	 BigWrite (FidData,(HPSTR)&GWDHead,sizeof(GWDHEADER16),-1);
 	 GSSillseek (FidData,0,2);
			     
	 BTVar[0].BT_VARLEN=4;
	 BTVar[0].BT_VARTYP=BT_INTEGER;
	 BTVar[0].BT_VAROFF=0;
	 lpDot = _fstrrchr (OutFile,'.');
	 _fstrcpy (lpDot,".in1");	
	 BT_CREATE (OutFile, 4, FALSE, 1, 1,BTVar,FALSE, 0, 0, FALSE);
 	 GSSiClose2 (&FidData);
	 _fstrcpy (lpDot,".dtm");	

     hDB = OpenGWDatabase (OutFile,BT_WRITE);
	lpGWDHead = (LPGWDHEADER)GlobalLock (hDB); 
	pSUBCELLInfo = (LPSUBCELLINFO)((LPSTR)&lpGWDHead->GWDData + (sizeof(DTMKEY)));
	pBias = (LPLONG)((LPSTR)&lpGWDHead->GWDData + (sizeof(DTMKEY) + sizeof(SUBCELLINFO)));
	CompressedDTMData = (LPSTR)&lpGWDHead->GWDData + (sizeof(DTMKEY) + sizeof(SUBCELLINFO) + 4);
	
	Fid = GSSiOpenFile (InFile,&OFStruct,OF_READ);
	if (!Fid)
{
																							#if ENABLETRACE
																							GSSiExitProg (1358);
																							#endif
		return FALSE; 
}
	TotLen = GSSillseek (Fid,0,2);
	GSSillseek (Fid,0,0);
	CreateStatusWind (hWndMain,1,0);
	pRec=GlobalLock (hRec); 
	fgetstring (pRec,128,Fid);
	if (sscanf (pRec,"%ld %ld %lf %ld %hi %lf %lf",&nRows,&nCols,&DTMData.GridSpace,&VoidVal,&nCharPerCell,&DTMData.SouthWestNode.x,&Maxy) != 7)  
		goto Exit;
	GlobalUnlock (hRec);
	DTMData.Bounds.ymn = Maxy - (long)(nRows-1) * DTMData.GridSpace; 
	nSubCellsY = (nRows-1)/32 + 1;
	DTMData.SouthWestNode.y = Maxy - DTMData.GridSpace*31; // gets to bottom of top subcell 
	DTMData.SouthWestNode.y -= DTMData.GridSpace*32*(nSubCellsY-1);  //surf is aligned on top of top subcell. Adjust sw.y to be bot of bottom subcell
	DTMData.Bounds.xmn = DTMData.SouthWestNode.x;
	DTMData.Bounds.xmx = DTMData.SouthWestNode.x + DTMData.GridSpace * (nCols-1);
	DTMData.Bounds.ymx = Maxy;
	DTMData.ElevUnits = 1; 
	DTMData.CoordUnits = 1; 

	
	irow = nRows; 
	nSubcells = (nCols-1) / 32 + 1;
	LastLen = nCols % 32;  
	if (!LastLen)
		LastLen = 32;
	hSubcellHandles = GSSiGlobAlloc (1095,GHND,nSubcells*sizeof(HANDLE));
	SubcellHandles = (LPHANDLE)GlobalLock (hSubcellHandles); 
	for (i=0;i<nSubcells;i++)
		SubcellHandles[i] = GSSiGlobAlloc (1096,GMEM_MOVEABLE,1024*4);
	SubcellRow=31; 
	SPY = Maxy - 16 * DTMData.GridSpace;  
	RowLen = nCols * nCharPerCell;
	while (ContinueProcessing && irow--)
	{   
		pRec=GlobalLock (hRec);
		fgetstring (pRec,RowLen+2,Fid);                       
		for (icell = 0;icell<nSubcells;icell++)
		{   
			DTMRow = (LPLONG)GlobalLock (SubcellHandles[icell]);
			DTMRow += 32 * SubcellRow; 
			if (icell < nSubcells-1)
			{
				for (icol=0;icol<32;icol++,pRec+=nCharPerCell) 
				{
					ERDASRow[icol]= ldread (pRec,nCharPerCell);   
					if (ERDASRow[icol] < 100 && ERDASRow[icol] > 0)
						ii=1;
				}
			}
			else
			{
				for (icol=0;icol<LastLen;icol++,pRec+=nCharPerCell)
					ERDASRow[icol]= ldread (pRec,nCharPerCell);   
				for (icol = LastLen;icol<32;icol++)
					ERDASRow[icol] = VoidVal;
			} 
			for (icol=0;icol<32;icol++)
				if (ERDASRow[icol] == VoidVal)
					DTMRow[icol] = LONG_MAX;
				else
					DTMRow[icol] = ERDASRow[icol];
			GlobalUnlock (SubcellHandles[icell]);
		}
		if (!SubcellRow || !irow)
		{   
			SPX = DTMData.SouthWestNode.x + 16 * DTMData.GridSpace;
			for (icell = 0;icell<nSubcells;icell++)
			{   
				DTMElev = (LPLONG)GlobalLock (SubcellHandles[icell]);
				if (!irow)
				{
					for (i=0;i<SubcellRow*32;i++)
						DTMElev[i] = LONG_MAX;
				}
				INTX   = (SPX - DTMData.SouthWestNode.x) / DTMData.GridSpace;
				INTY   = (SPY- DTMData.SouthWestNode.y) / DTMData.GridSpace;
				NGSANE (INTX,INTY,&GeoSeg,&SubCell,&IELV); 
				GeoSeg--;
				SubCell--;
				DTMKey.GEOSEG_ROW = GeoSeg / 4096; 
				DTMKey.GEOSEG_COL = GeoSeg % 4096;    
				DTMKey.SUBCEL_ROW = SubCell / 16;
				DTMKey.SUBCEL_COL = SubCell % 16;    
				n=1024;
				MinElv = SHRT_MAX;
				MaxElv = SHRT_MIN;   
				SUBCELLInfo = CompressSubcell (DTMElev,CompressedDTMData,pBias);  
				_fmemset (DTMData2,0,4096); 
				ExpandSubcell (DTMData2,SUBCELLInfo,CompressedDTMData,pBias);
				for (i=0;i<1024;i++)
					if (DTMElev[i] != DTMData2[i]) 
					{   
						GlobalUnlock (hRec);
						GSSiMessageBox (0,"decompress error",0,MB_ICONEXCLAMATION,0);
						goto Exit;        
					} 
				if (*pBias < LONG_MAX)
				{
					*pSUBCELLInfo = SUBCELLInfo;
					Offset = GSSillseek (lpGWDHead->Fid,0,1);  
					length = sizeof(DTMKEY) + sizeof(SUBCELLINFO) + 4 + SUBCELLInfo.LENGTH;
				    BigWrite (lpGWDHead->Fid,(HPSTR)&length,2,-1);
				    BigWrite (lpGWDHead->Fid,(HPSTR)&lpGWDHead->GWDData,length,-1);
		        	BT_PUT (lpGWDHead->BTHandle[0],(LPSTR)&DTMKey,(LPSTR)&Offset); 
		        }
		        SPX += 32 * DTMData.GridSpace;
		    } 
	        SPY -= 32 * DTMData.GridSpace;
			SubcellRow=32; 
		}
		SubcellRow--; 
		CurLoc = GSSillseek (Fid,0,1);
		StatusWindowUpdate ("","", TotLen, CurLoc); 
		GlobalUnlock (hRec);
	}    
	Offset = GSSillseek (lpGWDHead->Fid,0,1);  
	length = sizeof(DTMDATA);
    BigWrite (lpGWDHead->Fid,(HPSTR)&length,2,-1);
    BigWrite (lpGWDHead->Fid,(HPSTR)&DTMData,length,-1);   
    DTMKeyl = LONG_MAX;
	BT_PUT (lpGWDHead->BTHandle[0],(LPSTR)&DTMKeyl,(LPSTR)&Offset); 
Exit: 
	SetContinueProcessing ( TRUE);
	GSSiClose2 (&Fid);   
	DestroyStatusWindow (0);
	GSSiGlobUlFree (&hRec);  
	GlobalUnlock (hDB);
    CloseGWDatabase (hDB); 
	for (i=0;i<nSubcells;i++)
		GSSiGlobFree (&SubcellHandles[i]);
    GSSiGlobUlFree (&hSubcellHandles); 
    GSSiGlobUlFree (&hData);
{
																							#if ENABLETRACE
																							GSSiExitProg (1358);
																							#endif
	return TRUE;
}
																							#if ENABLETRACE
																							}
																							#endif
} 
BOOL LoadAREADTM (LPSTR InFile, LPSTR OutFile)
																							#if ENABLETRACE
																							{GSSiEnterProg (1358);
																							#endif
{
	HFILE	Fid;
	OFSTRUCTGM	OFStruct;
	UINT	irow, icol;   
	DPOINT	Point;
	HFILE		Fid1, Fid2;
	HANDLE		hRec=GSSiGlobAlloc (1093,GMEM_MOVEABLE,USHRT_MAX);
	LPSTR		pRec;
	short		ii, length;  
	UINT		i, n, pos;
	long		GeoSeg, Bias, nFiles=0, MaxFiles=50, nOutOfRange=0, nSubCells=0, nSubCellsY, nLess12=0;
	short		NumElv, SubCell, Indeterminate, MinElv, MaxElv,  SubcellRow;    
	BOOL		OutOfRange; 
	short		nSubcells, icell; 
	long		ERDASRow[32],LastLen; 
	BTVARDESC BTVar[2], *pVars;
	short		NumFields, Reclen, len;
	long	Offset;
	long	TotFileLen;
	GWDHEADER16 GWDHead; 
	LPGWDHEADER	lpGWDHead;
	GWDHEADER	GWDHead32;
	GWFLDINFO GWFldInfo;
	LPGWFLDINFO	lpGWFldInfo;
	HANDLE hBT, hDB=0;
	HFILE	FidData;
	int		ibeg;
	GWFLDINFO FldInfo;
	LPSTR	lpDot; 
	HANDLE	hCell;
	LPSHORT	SubcellDat; 
	LPLONG	pBias;  
	static	long	debugsubcell=845;  
	long	TotLen, CurLoc, SubCellID;
	DTMKEY		DTMKey;
	DTMDATA	DTMData;
	SUBCELLINFO	SUBCELLInfo;
	LPSUBCELLINFO	pSUBCELLInfo;
	LPSTR		CompressedDTMData;  
	HANDLE	hSubcellHandles;
	LPHANDLE	SubcellHandles;     
	LPLONG	DTMRow;   
	long	INTX,INTY, DTMKeyl, nRows, nCols, RowLen, VoidVal;  
	short	IELV, nCharPerCell=10;
	double	SPX,SPY, Maxy;  
	HANDLE	hData=GSSiGlobAlloc (1094,GMEM_MOVEABLE,4096+2048);
	LPLONG		DTMElev, DTMData2=(LPLONG)GlobalLock (hData);  
	LPSHORT		SubcellData=(LPSHORT)(DTMData2+1024); 
	BOOL	rtn=FALSE;
	
	
	 lpGWDHead = &GWDHead32; 
    _fmemset (&GWDHead,0,sizeof(GWDHEADER16));

	 FidData = GSSiOpenFile (OutFile,&OFStruct,OF_CREATE);
	 GWDHead.NumFields=0;
	 GWDHead.NumIndex=1;
	 GWDHead.Version=1;
	 GWDHead.NumIndexFields[0]=1;
	 GWDHead.IndexFields[0][0]=0;
	 BigWrite (FidData,(HPSTR)&GWDHead,sizeof(GWDHEADER16),-1);
	ibeg = 0;

	FldInfo.Len = sizeof(DTMKEY);
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"DTMKEY");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"BIAS");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = sizeof(SUBCELLINFO);
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"SUBCELLINFO");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	
	FldInfo.Len = 4096;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"COMPRESSEDNODES");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;
	
	 GWDHead.Reclen=ibeg; 
	 GWDHead.TimeStamp = 0;
	 GSSillseek (FidData,0,0);
	 BigWrite (FidData,(HPSTR)&GWDHead,sizeof(GWDHEADER16),-1);
 	 GSSillseek (FidData,0,2);
			     
	 BTVar[0].BT_VARLEN=4;
	 BTVar[0].BT_VARTYP=BT_INTEGER;
	 BTVar[0].BT_VAROFF=0;
	 lpDot = _fstrrchr (OutFile,'.');
	 _fstrcpy (lpDot,".in1");	
	 BT_CREATE (OutFile, 4, FALSE, 1, 1,BTVar,FALSE, 0, 0, FALSE);
 	 GSSiClose2 (&FidData);
	 _fstrcpy (lpDot,".dtm");	

     hDB = OpenGWDatabase (OutFile,BT_WRITE);
	lpGWDHead = (LPGWDHEADER)GlobalLock (hDB); 
	pSUBCELLInfo = (LPSUBCELLINFO)((LPSTR)&lpGWDHead->GWDData + (sizeof(DTMKEY)));
	pBias = (LPLONG)((LPSTR)&lpGWDHead->GWDData + (sizeof(DTMKEY) + sizeof(SUBCELLINFO)));
	CompressedDTMData = (LPSTR)&lpGWDHead->GWDData + (sizeof(DTMKEY) + sizeof(SUBCELLINFO) + 4);
	
	Fid = GSSiOpenFile (InFile,&OFStruct,OF_READ);
	if (!Fid)
{
																							#if ENABLETRACE
																							GSSiExitProg (1358);
																							#endif
		return FALSE; 
}
	TotLen = GSSillseek (Fid,0,2);
	GSSillseek (Fid,0,0);
	CreateStatusWind (hWndMain,1,0);
	pRec=GlobalLock (hRec); 
	fgetstring (pRec,128,Fid);
	if (sscanf (pRec,"%ld %ld %lf %ld %hi %lf %lf",&nRows,&nCols,&DTMData.GridSpace,&VoidVal,&nCharPerCell,&DTMData.SouthWestNode.x,&Maxy) != 7)  
		goto Exit;
	GlobalUnlock (hRec);
	DTMData.Bounds.ymn = Maxy - (long)(nRows-1) * DTMData.GridSpace; 
	nSubCellsY = (nRows-1)/32 + 1;
	DTMData.SouthWestNode.y = Maxy - DTMData.GridSpace*31; // gets to bottom of top subcell 
	DTMData.SouthWestNode.y -= DTMData.GridSpace*32*(nSubCellsY-1);  //surf is aligned on top of top subcell. Adjust sw.y to be bot of bottom subcell
	DTMData.Bounds.xmn = DTMData.SouthWestNode.x;
	DTMData.Bounds.xmx = DTMData.SouthWestNode.x + DTMData.GridSpace * (nCols-1);
	DTMData.Bounds.ymx = Maxy;
	DTMData.ElevUnits = 1; 
	DTMData.CoordUnits = 1; 

	
	irow = nRows; 
	nSubcells = (nCols-1) / 32 + 1;
	LastLen = nCols % 32;  
	if (!LastLen)
		LastLen = 32;
	hSubcellHandles = GSSiGlobAlloc (1095,GHND,nSubcells*sizeof(HANDLE));
	SubcellHandles = (LPHANDLE)GlobalLock (hSubcellHandles); 
	for (i=0;i<nSubcells;i++)
		SubcellHandles[i] = GSSiGlobAlloc (1096,GMEM_MOVEABLE,1024*4);
	SubcellRow=31; 
	SPY = Maxy - 16 * DTMData.GridSpace;  
	RowLen = nCols * nCharPerCell;
	while (ContinueProcessing && irow--)
	{   
		pRec=GlobalLock (hRec);
		fgetstring (pRec,RowLen+2,Fid);                       
		for (icell = 0;icell<nSubcells;icell++)
		{   
			DTMRow = (LPLONG)GlobalLock (SubcellHandles[icell]);
			DTMRow += 32 * SubcellRow; 
			if (icell < nSubcells-1)
			{
				for (icol=0;icol<32;icol++,pRec+=nCharPerCell) 
				{
					ERDASRow[icol]= ldread (pRec,nCharPerCell);   
					if (ERDASRow[icol] < 100 && ERDASRow[icol] > 0)
						ii=1;
				}
			}
			else
			{
				for (icol=0;icol<LastLen;icol++,pRec+=nCharPerCell)
					ERDASRow[icol]= ldread (pRec,nCharPerCell);   
				for (icol = LastLen;icol<32;icol++)
					ERDASRow[icol] = VoidVal;
			} 
			for (icol=0;icol<32;icol++)
				if (ERDASRow[icol] == VoidVal)
					DTMRow[icol] = LONG_MAX;
				else
					DTMRow[icol] = ERDASRow[icol];
			GlobalUnlock (SubcellHandles[icell]);
		}
		if (!SubcellRow || !irow)
		{   
			SPX = DTMData.SouthWestNode.x + 16 * DTMData.GridSpace;
			for (icell = 0;icell<nSubcells;icell++)
			{   
				DTMElev = (LPLONG)GlobalLock (SubcellHandles[icell]);
				if (!irow)
				{
					for (i=0;i<SubcellRow*32;i++)
						DTMElev[i] = LONG_MAX;
				}
				INTX   = (SPX - DTMData.SouthWestNode.x) / DTMData.GridSpace;
				INTY   = (SPY- DTMData.SouthWestNode.y) / DTMData.GridSpace;
				NGSANE (INTX,INTY,&GeoSeg,&SubCell,&IELV); 
				GeoSeg--;
				SubCell--;
				DTMKey.GEOSEG_ROW = GeoSeg / 4096; 
				DTMKey.GEOSEG_COL = GeoSeg % 4096;    
				DTMKey.SUBCEL_ROW = SubCell / 16;
				DTMKey.SUBCEL_COL = SubCell % 16;    
				n=1024;
				MinElv = SHRT_MAX;
				MaxElv = SHRT_MIN;   
				SUBCELLInfo = CompressSubcell (DTMElev,CompressedDTMData,pBias);  
				_fmemset (DTMData2,0,4096); 
				ExpandSubcell (DTMData2,SUBCELLInfo,CompressedDTMData,pBias);
				for (i=0;i<1024;i++)
					if (DTMElev[i] != DTMData2[i]) 
					{   
						GlobalUnlock (hRec);
						GSSiMessageBox (0,"decompress error",0,MB_ICONEXCLAMATION,0);
						goto Exit;        
					} 
				if (*pBias < LONG_MAX)
				{
					*pSUBCELLInfo = SUBCELLInfo;
					Offset = GSSillseek (lpGWDHead->Fid,0,1);  
					length = sizeof(DTMKEY) + sizeof(SUBCELLINFO) + 4 + SUBCELLInfo.LENGTH;
				    BigWrite (lpGWDHead->Fid,(HPSTR)&length,2,-1);
				    BigWrite (lpGWDHead->Fid,(HPSTR)&lpGWDHead->GWDData,length,-1);
		        	BT_PUT (lpGWDHead->BTHandle[0],(LPSTR)&DTMKey,(LPSTR)&Offset); 
		        }
		        SPX += 32 * DTMData.GridSpace;
		    } 
	        SPY -= 32 * DTMData.GridSpace;
			SubcellRow=32; 
		}
		SubcellRow--; 
		CurLoc = GSSillseek (Fid,0,1);
		StatusWindowUpdate ("","", TotLen, CurLoc); 
		GlobalUnlock (hRec);
	}    
	Offset = GSSillseek (lpGWDHead->Fid,0,1);  
	length = sizeof(DTMDATA);
    BigWrite (lpGWDHead->Fid,(HPSTR)&length,2,-1);
    BigWrite (lpGWDHead->Fid,(HPSTR)&DTMData,length,-1);   
    DTMKeyl = LONG_MAX;
	BT_PUT (lpGWDHead->BTHandle[0],(LPSTR)&DTMKeyl,(LPSTR)&Offset); 
Exit: 
	SetContinueProcessing ( TRUE);
	GSSiClose2 (&Fid);   
	DestroyStatusWindow (0);
	GSSiGlobUlFree (&hRec);  
	GlobalUnlock (hDB);
    CloseGWDatabase (hDB); 
	for (i=0;i<nSubcells;i++)
		GSSiGlobFree (&SubcellHandles[i]);
    GSSiGlobUlFree (&hSubcellHandles); 
    GSSiGlobUlFree (&hData);
{
																							#if ENABLETRACE
																							GSSiExitProg (1358);
																							#endif
	return TRUE;
}
																							#if ENABLETRACE
																							}
																							#endif
} 

long N3PNT (LPLONG MODELN,double X,double Y,short MISING)
																							#if ENABLETRACE
																							{GSSiEnterProg (1359);
																							#endif
{
//C******* SPECIFICATIONS ***********************************************
//C*                                                                    *
//C*       PROGRAM SUMMARY                                              *
//C*       ------- -------                                              *
//C*       THIS FUNCTION DETERMINES THE ELEVATION OF A STATE PLANE      *
//C*       SUBCELL NODE FROM THREE SURROUNDING MODEL NODES.             *
//C*                                                                    *
//C*       ---------                                                    *
//C*       LET M1, M2 AND M3 BE THE THREE MODEL NODES SURROUNDING THE   *
//C*       STATE PLANE NODE SPN. FIRST CALCULATE THE DISTANCES D1, D2   *
//C*       AND D3 FROM SPN TO M1, M2 AND M3, USING X, Y AND THE         *
//C*       PATHAGOREAN RELATIONSHIP. THE STATE PLANE NODE IS THEN       *
//C*       CALCULATED AS:                                               *
//C*                    M1/D1 + M2/D2 + M3/D3                           *
//C*              SPN = ---------------------                           *
//C*                    1/D1  + 1/D2  + 1/D3                            *
//C*                                                                    *
//C**********************************************************************
      double NUM, DENOM, XSQ, YSQ, ONEMNX, ONEMNY, SQ1MNX, SQ1MNY;
      double D1, D2, D3, D4;
	  long	 RELV;
      
      NUM=0.0;
      DENOM=0.0;
      if (!X) X = 1.0E-10;
      if (!Y) Y = 1.0E-10;
      ONEMNX = 1.0 - X;
      ONEMNY = 1.0 - Y;
      XSQ = X * X;
      YSQ = Y * Y;
      SQ1MNX = ONEMNX * ONEMNX;
      SQ1MNY = ONEMNY * ONEMNY;
      if (MISING == 1) goto S10;
      D1 = 1.0 / sqrt (XSQ + YSQ);
      DENOM = DENOM + D1;
      NUM = NUM + (double)MODELN[1] * D1;
      if (MISING == 2) goto S20;
 S10: D2 = 1.0 / sqrt (XSQ + SQ1MNY);
      DENOM = DENOM + D2;
      NUM = NUM + (double)MODELN[2] * D2;
      if (MISING == 3) goto S30;
 S20: D3 = 1.0 / sqrt (SQ1MNX + SQ1MNY);
      DENOM = DENOM + D3;
      NUM = NUM + (double)MODELN[3] * D3;
      if (MISING == 4) goto S40;
 S30: D4 = 1.0 / sqrt (SQ1MNX + YSQ);
      DENOM = DENOM + D4;
      NUM = NUM + (double) MODELN[4] * D4;
 S40: RELV = IDNINT(NUM / DENOM);
{
																							#if ENABLETRACE
																							GSSiExitProg (1359);
																							#endif
      return (RELV);
}
																							#if ENABLETRACE
																							}
																							#endif
}

double N4PNT (LPLONG MODELN,double X,double Y)
																							#if ENABLETRACE
																							{GSSiEnterProg (1360);
																							#endif
{
      double DM2M1, DM3M4, MN1MN2, MN3MN4, RELV; 
      
      DM2M1 = MODELN[2] - MODELN[1];
      DM3M4 = MODELN[3] - MODELN[4];
      MN1MN2 = (double)MODELN[1] + Y * DM2M1;
      MN3MN4 = (double)MODELN[4] + Y * DM3M4;
      RELV = MN1MN2 + X * (MN3MN4 - MN1MN2);
{
																							#if ENABLETRACE
																							GSSiExitProg (1360);
																							#endif
	  return RELV;
}
																							#if ENABLETRACE
																							}
																							#endif
} 

void NGSANE (long ISPXDM,long ISPYDM,LPLONG pIGESEG,LPSHORT pISUBCL,LPSHORT pIEL)
																							#if ENABLETRACE
																							{GSSiEnterProg (1361);
																							#endif
{
//C******* SPECIFICATIONS ***********************************************
//C*                                                                    *
//C*       PROGRAM SUMMARY                                              *
//C*       ---------------                                              *
//C*    NGSANE (NGI: DETERMINE GEOSEGMENT, SUBCELL AND ELEMENT NUMBER   *
//C*    OF A NODE) DETERMINE THE GEOSEGMENT, SUBCELL AND ELEMENT NUMBERS*
//C*    OF A NGI GRID NODE, GIVEN THE STATE PLANE COORDINATES OF THE    *
//C*    NODE IN DECIMETERS.                                             *
//C*                                                                    *
	long	NSPXDM = ISPXDM;
	long	NSPYDM = ISPYDM;
	long	IGROW  = NSPYDM / 512;
	long	IGCOL  = NSPXDM / 512;
	long	IGESEG = IGROW * 4096 + IGCOL + 1;
	long	MODX   = NSPXDM % 512;
	long	MODY   = NSPYDM % 512;
	short	ISCCOL = MODX / 32 + 1;
	short	ISCROW = MODY / 32;
	short	ISUBCL = ISCROW * 16 + ISCCOL; 
	short	IELCOL, IELROW, IEL;
	      
	MODX   = NSPXDM % 32;
	MODY   = NSPYDM % 32;
	IELCOL = MODX + 1;
	IELROW = MODY;
	IEL    = IELROW * 32 + IELCOL;     
	*pIGESEG = IGESEG;
	*pISUBCL = ISUBCL;
	*pIEL = IEL;
{
																							#if ENABLETRACE
																							GSSiExitProg (1361);
																							#endif
	return;
}
																							#if ENABLETRACE
																							}
																							#endif
}  

BOOL ConvertDTMv1Tov2 (LPSTR FileName)
{
	HANDLE	hSurf=0, hDB; 
	UINT	i;    
	long	DTMKey, Offset, nRecs, nLoaded=0; 
	LPGWDHEADER	lpGWDHead;     
	LPDTMDATA	pDTMData;  
	short	pos=BT_FIRST;
	SUBCELLINFO_v1	SUBCELLInfo_v1;
	SUBCELLINFO	SUBCELLInfo;
	MNMXCORD NewBounds;
	
	if (!(hDB = OpenGWDatabase (FileName,BT_WRITE)))
		return 0;  
	DBoundsInit (&NewBounds);
	lpGWDHead = (LPGWDHEADER)GlobalLock (hDB); 
    nRecs = BT_NUM_IN_INDEX (lpGWDHead->BTHandle[0]);
	CreateStatusWind (hWndMain,1,"Converting DTM - DO NOT ABORT!");
	pDTMData = (LPDTMDATA)((LPSTR)&lpGWDHead->GWDData); 
   	while (ContinueProcessing && !BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&DTMKey,pos,BT_ANY,(LPSTR)&Offset)) 
   	{
		unsigned short	len;    
		
	    pos = BT_NEXT;
	    GSSillseek (lpGWDHead->Fid,Offset,0);
	    BigRead (lpGWDHead->Fid,(HPSTR)&len,2);
		if (DTMKey == LONG_MAX)
		{
		    BigRead (lpGWDHead->Fid,(HPSTR)&lpGWDHead->GWDData,len); 
			pDTMData->Bounds = NewBounds;
		    GSSillseek (lpGWDHead->Fid,Offset+2,0);
//		    BigWrite (lpGWDHead->Fid,pDTMData,len);
			break;
		}
	    BigRead (lpGWDHead->Fid,(HPSTR)&DTMKey,4);
	    Offset+=6;
	    BigRead (lpGWDHead->Fid,(HPSTR)&SUBCELLInfo_v1,sizeof(SUBCELLINFO_v1));
	    GSSillseek (lpGWDHead->Fid,Offset,0);
	    SUBCELLInfo.TYPE = SUBCELLInfo_v1.TYPE; 
	    SUBCELLInfo.LENGTH = SUBCELLInfo_v1.LENGTH; 
	    SUBCELLInfo.INDT = SUBCELLInfo_v1.INDT; 
	    BigWrite (lpGWDHead->Fid,(HPSTR)&SUBCELLInfo,sizeof(SUBCELLINFO),-1);
		StatusWindowUpdate (0,0, nRecs, ++nLoaded);
	} 
	SetContinueProcessing ( TRUE);
    lpGWDHead->Version = 2;
	GlobalUnlock (hDB);
	CloseGWDatabase (hDB); 
	DestroyStatusWindow(0);  
	return TRUE;
}
	
	
HANDLE DTMOpen (LPSTR FileNameIN, double NULLElv,short Mode,LPSHORT pSurfType)
																							#if ENABLETRACE
																							{GSSiEnterProg (1362);
																							#endif
{
	HANDLE	hSurf=0, hDB; 
	UINT	i;    
	LPDTMINFO	pDTMInfo; 
	long	DTMKey=LONG_MAX, Offset; 
	LPGWDHEADER	lpGWDHead;     
	LPDTMDATA	pDTMData;
	static	BOOL	First=TRUE;  
	short	Type=0;
	char	FileName[MAX_PATH],str[MAX_PATH], Projection[MAX_PATH]; 
	HFILE	FidSurf;
	
	DTMRenderGridSpacing = GetGlobalDVal2("[%DTMGridSpacing]",-300.0);   
	if (!DTMRenderGridSpacing)
{
																							#if ENABLETRACE
																							GSSiExitProg (1362);
																							#endif
		return 0;
}
	if (DTMRenderGridSpacing < 0)
    	DTMRenderGridSpacing = (CurView->WBounds.xmx - CurView->WBounds.xmn) / (-DTMRenderGridSpacing);	
	DTMSmooth = GetGlobalDVal2 ("[%DTMSmooth]",0);
	HaveLastTriangle = FALSE; 
	if (First)
		for (i=0;i<MAXOPENSURF;i++)
			hOpenSurf[i] = 0;
	First = FALSE;
	
	_fstrcpy (FileName,FileNameIN);  
	ExpandText (FileName);
	_fstrupr (FileName);
	if (StringEndsWith(FileName,".DTM"))
		Type = DTMTYPE_NGI;
	else if (StringEndsWith(FileName,".TIN"))
		Type = DTMTYPE_TIN_GM; 
	else if (StringEndsWith(FileName, ".LDR"))
		Type = DTMTYPE_LIDAR_GM;
	else if (StringEndsWith(FileName, ".LA"))
		Type = DTMTYPE_LIDAR_LAZ;
	if (!Type)
{
																							#if ENABLETRACE
																							GSSiExitProg (1362);
																							#endif
		return 0;
}   
	if (pSurfType)
		*pSurfType = Type;
	switch (Type)
	{
		case DTMTYPE_NGI:
ReOpen:			
		if (!(hDB = OpenGWDatabase (FileName,Mode)))
{
																							#if ENABLETRACE
																							GSSiExitProg (1362);
																							#endif
			return 0;
}
		hSurf = GSSiGlobAlloc (1097,GHND,sizeof(DTMINFO));
		pDTMInfo = (LPDTMINFO)GlobalLock (hSurf);
		pDTMInfo->Type = Type;
		pDTMInfo->hDB = hDB;   
		lpGWDHead = (LPGWDHEADER)GlobalLock (pDTMInfo->hDB); 
		if (lpGWDHead->Version == 1)
		{
			ConvertDTMv1Tov2 (FileName);
			goto ReOpen;
		}
		pDTMData = (LPDTMDATA)((LPSTR)&lpGWDHead->GWDData); 
	   	if (!BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&DTMKey,BT_FIRST,BT_EQ,(LPSTR)&Offset)) 
	   	{
			unsigned short	len;    
			LPLONG	pCell;
	
		    GSSillseek (lpGWDHead->Fid,Offset,0);
		    BigRead (lpGWDHead->Fid,(HPSTR)&len,2);
		    BigRead (lpGWDHead->Fid,(HPSTR)&lpGWDHead->GWDData,len); 
		    _fmemcpy (&pDTMInfo->NULLElv,pDTMData,sizeof(DTMDATA));
		} 
		else 
		{
			pDTMInfo->ElevUnits = 2;  
			pDTMInfo->SouthWestNode.x = 0;
			pDTMInfo->SouthWestNode.y = 0;
			pDTMInfo->GridSpace = 0.5;
		}  
		pDTMInfo->NULLElv = NULLElv;
		for (i=0;i<MAXDTMCELLBUFFERS;i++)
		{
			pDTMInfo->CellUse[i]=LONG_MIN;
			pDTMInfo->CellID[i]=LONG_MIN;
	    }
	    GlobalUnlock (pDTMInfo->hDB);
		GlobalUnlock (hSurf);
		break;
	
		case DTMTYPE_TIN_GM: 
		{
			LPVISLIST	SaveVis=CurVis; 
			short		TinSymbol,i;
			LPSTR		pDot;
			
			hSurf = GSSiGlobAlloc (1098,GHND,sizeof(DTMINFO));
			pDTMInfo = (LPDTMINFO)GlobalLock (hSurf);
			pDTMInfo->Type = Type; 
			if (PRJ_BASEUNITS[1] == 1)
				pDTMInfo->ElevUnits = 0;
			else
				pDTMInfo->ElevUnits = 3;   
			_fstrcpy (pDTMInfo->TINIndex,FileName);
			if ((pDot=_fstrrchr (pDTMInfo->TINIndex,'.'))) 
			{
				*pDot = 0;
				GetMapBounds (pDTMInfo->TINIndex,&pDTMInfo->Bounds);
			}
			pDTMInfo->NULLElv = NULLElv;
			CurVis = &pDTMInfo->VisList;
			InitVis (); 
			_fmemset (CurVis->WantType,0,sizeof(CurVis->WantType));
			CurVis->WantType[0] = TRUE;
			_fmemset (CurVis->VisBits,0,sizeof(CurVis->VisBits));
			for (i=1;i<MAX_VIEWPORT_FILES;i++)
				CurVis->FileIsVisible[i] = FALSE;
			CurVis->FileIsVisible[0] = TRUE;
			TinSymbol = GetSymbolNum ("TINTRIANGLE");
			ToggleVisibility (TinSymbol);
			i=0;	
			//StartFastPick (-(i+CurView->ID*256));
			CurVis = SaveVis;
			GlobalUnlock (hSurf);
		}
		break;
		case DTMTYPE_LIDAR_GM:
		{
			LIDARREC	LidarRec;
			LIDARFILEHEADER	Header;

			FidSurf = GSSiOpenFile(FileName, 0, OF_READ);
			if (FidSurf == HFILE_ERROR)
				return FALSE;
			hSurf = GSSiGlobAlloc(1097, GHND, sizeof(DTMINFO));
			BigRead(FidSurf, (HPSTR)&Header, sizeof(LIDARFILEHEADER));

			pDTMInfo = (LPDTMINFO)GlobalLock(hSurf);
			pDTMInfo->Type = Type;
			pDTMInfo->Version = Header.Version;
			pDTMInfo->Fid = FidSurf;
			pDTMInfo->ElevUnits = 3;
			pDTMInfo->Bounds = Header.Bounds;
			pDTMInfo->SouthWestNode.x = Header.Bounds.xmn;
			pDTMInfo->SouthWestNode.y = Header.Bounds.ymn;
			pDTMInfo->NumRows = Header.NumRows;
			pDTMInfo->NumCols = Header.NumCols;
			pDTMInfo->GridSpace = Header.CellSpacing;
			pDTMInfo->MaxDistToRawPoint = min(pDTMInfo->GridSpace, GetGlobalDVal2("[%DTMLidarMaxDistToRawPoint]", pDTMInfo->GridSpace));
			pDTMInfo->MaxRawPointsToUse = GetGlobalLVal2("[%DTMLidarMaxPointsToUse]", 16);

			pDTMInfo->NULLElv = NULLElv;
			for (i = 0; i<MAXDTMCELLBUFFERS; i++)
			{
				pDTMInfo->CellUse[i] = LONG_MIN;
				pDTMInfo->CellID[i] = LONG_MIN;
			}
			GlobalUnlock(hSurf);
		}
			break;
		case DTMTYPE_LIDAR_LAZ:
		{
			sqlite3 *db;
			LPSTR pDot;

			if (sqlite3_open(FileName, &db) != SQLITE_OK)
				return FALSE;

			hSurf = GSSiGlobAlloc(1097, GHND, sizeof(DTMINFO));

			pDTMInfo = (LPDTMINFO)GlobalLock(hSurf);
			pDTMInfo->Type = Type;
			pDTMInfo->db = db;
			pDTMInfo->Fid = HFILE_ERROR;
			strcpy(pDTMInfo->LAZDir, FileName);
			pDot = strrchr(pDTMInfo->LAZDir, '\\');
			if (pDot)
				*pDot = 0;
			pDTMInfo->ElevUnits = 3;
			pDTMInfo->Bounds = SLTSpatialIndexBounds(db, "LIDAR");
			pDTMInfo->MaxDistToRawPoint = min(pDTMInfo->GridSpace, GetGlobalDVal2("[%DTMLidarMaxDistToRawPoint]", pDTMInfo->GridSpace));
			pDTMInfo->MaxRawPointsToUse = GetGlobalLVal2("[%DTMLidarMaxPointsToUse]", 16);

			pDTMInfo->NULLElv = NULLElv;
			sprintf(Projection, "%s\\projection.cvt", pDTMInfo->LAZDir);
			LoadProjection(0, Projection);

			GlobalUnlock(hSurf);
		}
			break;
	}
	if (hSurf) 
	{   
		pDTMInfo = (LPDTMINFO)GlobalLock (hSurf);
		CurNullElv = pDTMInfo->NULLElv;
		GlobalUnlock (hSurf);
		for (i=0;i<MAXOPENSURF;i++)
		{ 
			if (!hOpenSurf[i]) 
			{
				hOpenSurf[i] = hSurf;
				break;
			}
		}
	} 
{
																							#if ENABLETRACE
																							GSSiExitProg (1362);
																							#endif
	return hSurf;
}
																							#if ENABLETRACE
																							}
																							#endif
}

void DTMClose (LPHANDLE pHandle)
																							#if ENABLETRACE
																							{GSSiEnterProg (1363);
																							#endif
{   
	UINT	i,j;
	LPDTMINFO	pDTMInfo;
	          
	GSSiGlobFree (&DTMCellHandle); 
	GSSiGlobFree (&hDTMRenderGridRow[0]);
	GSSiGlobFree (&hDTMRenderGridRow[1]);
	GSSiGlobFree (&hDTMRenderGridRow[2]);
	if (!pHandle)
	{
		for (i=0;i<MAXOPENSURF;i++)
		{
		
			if (hOpenSurf[i])
			{
				pDTMInfo = (LPDTMINFO)GlobalLock (hOpenSurf[i]); 
				if (pDTMInfo->Type == DTMTYPE_LIDAR_GM)
					GSSiClose2 (&pDTMInfo->Fid);
				else if (pDTMInfo->Type == DTMTYPE_LIDAR_LAZ)
					sqlite3_close(pDTMInfo->db);
				else
					CloseGWDatabase (pDTMInfo->hDB); 
				for (j=0;j<MAXDTMCELLBUFFERS;j++)
					if (pDTMInfo->hCell[j] > (HANDLE)1)
						GSSiGlobFree (&pDTMInfo->hCell[j]);
			} 
			GSSiGlobUlFree (&hOpenSurf[i]);
		}	
	}
	else if (*pHandle)
	{ 
		for (i=0;i<MAXOPENSURF;i++)
		{ 
			if (hOpenSurf[i] == *pHandle) 
			{
				pDTMInfo = (LPDTMINFO)GlobalLock (hOpenSurf[i]);
				if (pDTMInfo->Type == 3)
				{   
					short	ii;
					GSSiClose2 (&pDTMInfo->Fid); 
					if (pDTMInfo->Fid == 35)
						ii=1;
				}
				else
					CloseGWDatabase (pDTMInfo->hDB); 
				for (j=0;j<MAXDTMCELLBUFFERS;j++)
					if (pDTMInfo->hCell[j] > (HANDLE)1)
						GSSiGlobFree (&pDTMInfo->hCell[j]); 
				/*if (pDTMInfo->Type == 2)
					EndFastPick ();*/
				GSSiGlobUlFree (&hOpenSurf[i]);
				break;
			}
		}
		*pHandle = 0;
	}
{
																							#if ENABLETRACE
																							GSSiExitProg (1363);
																							#endif
	return;
}
																							#if ENABLETRACE
																							}
																							#endif
}

HANDLE GetDTMSubCell (HANDLE hSurf,long GeoSeg, short SubCell,LPDTMINFO pDTMInfo)
																							#if ENABLETRACE
																							{GSSiEnterProg (1364);
																							#endif
{
	long		CellID,ii;
	DTMKEY		DTMKey;
	SUBCELLINFO	SUBCELLInfo;
	LPSTR		CompressedDTMData;
	UINT		i, MinUseID;  
	long		MinUse=LONG_MAX, Offset;
	LPLONG		pBias;  
	LPSUBCELLINFO	pSUBCELLInfo;
	LPGWDHEADER	lpGWDHead;     
	HANDLE		handle=0;  
	static	long	nio=0;
	
	GeoSeg--; 
	SubCell--; 
	DTMKey.GEOSEG_ROW = GeoSeg / 4096; 
	DTMKey.GEOSEG_COL = GeoSeg % 4096;    
	DTMKey.SUBCEL_ROW = SubCell / 16;
	DTMKey.SUBCEL_COL = SubCell % 16;   
	_fmemmove (&CellID,&DTMKey,4);
	DTMSubCellPoint.x = pDTMInfo->SouthWestNode.x + ((long)DTMKey.GEOSEG_COL * 16 * 32 + (long)DTMKey.SUBCEL_COL * 32) * pDTMInfo->GridSpace;
	DTMSubCellPoint.y = pDTMInfo->SouthWestNode.y + ((long)DTMKey.GEOSEG_ROW * 16 * 32 + (long)DTMKey.SUBCEL_ROW * 32) * pDTMInfo->GridSpace;
	for (i=0;i<MAXDTMCELLBUFFERS;i++)
		if (CellID == pDTMInfo->CellID[i])
		{
			pDTMInfo->CellUse[i]=++NextDTMUse;
			if (pDTMInfo->hCell[i] > (HANDLE)1)
				handle = pDTMInfo->hCell[i];
{
																							#if ENABLETRACE
																							GSSiExitProg (1364);
																							#endif
			return (handle);
}
		}
		else if (pDTMInfo->CellUse[i] < MinUse)
		{
			MinUse = pDTMInfo->CellUse[i];
			MinUseID = i;
		}
	lpGWDHead = (LPGWDHEADER)GlobalLock (pDTMInfo->hDB); 
	pSUBCELLInfo = (LPSUBCELLINFO)((LPSTR)&lpGWDHead->GWDData + (sizeof(DTMKEY)));
	pBias = (LPLONG)((LPSTR)&lpGWDHead->GWDData + (sizeof(DTMKEY) + sizeof(SUBCELLINFO)));
	CompressedDTMData = (LPSTR)&lpGWDHead->GWDData + (sizeof(DTMKEY) + sizeof(SUBCELLINFO) + 4); 
	if (pDTMInfo->hCell[MinUseID] > (HANDLE)1) 
    	GSSiGlobFree (&pDTMInfo->hCell[MinUseID]);
    else
    	pDTMInfo->hCell[MinUseID] = 0;
   	if (!BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&CellID,BT_FIRST,BT_EQ,(LPSTR)&Offset)) 
   	{
		unsigned short	len;    
		LPLONG	pCell;

	    GSSillseek (lpGWDHead->Fid,Offset,0);
	    BigRead (lpGWDHead->Fid,(HPSTR)&len,2);
	    BigRead (lpGWDHead->Fid,(HPSTR)&lpGWDHead->GWDData,len); 
	    SetGWDCurrentOffset (lpGWDHead,Offset);
	    pDTMInfo->hCell[MinUseID] = GSSiGlobAlloc (1099,GMEM_MOVEABLE,4096);
	    pCell = (LPLONG)GlobalLock (pDTMInfo->hCell[MinUseID]);
		ExpandSubcell (pCell,*pSUBCELLInfo,CompressedDTMData,pBias);
		GlobalUnlock (pDTMInfo->hCell[MinUseID]);
		handle = pDTMInfo->hCell[MinUseID]; 
		nio++;
	} 
	else
	    pDTMInfo->hCell[MinUseID] = (HANDLE)1;
	pDTMInfo->CellID[MinUseID] = CellID;
	pDTMInfo->CellUse[MinUseID] = ++NextDTMUse;    
	GlobalUnlock (pDTMInfo->hDB);
{
																							#if ENABLETRACE
																							GSSiExitProg (1364);
																							#endif
	return handle;
}
																							#if ENABLETRACE
																							}
																							#endif
}

BOOL SetDTMSubCell (long GeoSeg, short SubCell,int node,double Elev,short Units,LPDTMINFO pDTMInfo)
																							#if ENABLETRACE
																							{GSSiEnterProg (1365);
																							#endif
{
	long		CellID,ii;
	DTMKEY		DTMKey;
	LPSTR		CompressedDTMData;
	UINT		i, MinUseID;  
	long		MinUse=LONG_MAX, Offset, IElev=0;
	LPLONG		pBias;  
	LPSUBCELLINFO	pSUBCELLInfo;
	LPGWDHEADER	lpGWDHead;     
	HANDLE		hCell=0; 
	BOOL		rtn=FALSE;
	
	switch (Units)
	{
		case 0: //have feet
			switch (pDTMInfo->ElevUnits)
			{   
				case 0: //want feet  
				IElev = IDNINT(Elev);
				break;
				case 1: //want feet*100
				IElev = IDNINT (Elev * 100);
				break;
				case 2://want decimeters
				IElev = IDNINT(Elev * FTM * 100);
				break;
			}
		break;
		
		case 1: //have meters
			switch (pDTMInfo->ElevUnits)
			{   
				case 0: //want feet
				IElev = IDNINT (Elev * MFT);
				break; 
				case 1: //want feet*100 
				IElev = IDNINT (Elev * MFT * 100);
				break;
				case 2: //want decimeters
				IElev = IDNINT (Elev * 100);
				break;
			}
		break;
	}
	GeoSeg--; 
	SubCell--; 
	DTMKey.GEOSEG_ROW = GeoSeg / 4096; 
	DTMKey.GEOSEG_COL = GeoSeg % 4096;    
	DTMKey.SUBCEL_ROW = SubCell / 16;
	DTMKey.SUBCEL_COL = SubCell % 16;   
	_fmemmove (&CellID,&DTMKey,4);
	lpGWDHead = (LPGWDHEADER)GlobalLock (pDTMInfo->hDB); 
	pSUBCELLInfo = (LPSUBCELLINFO)((LPSTR)&lpGWDHead->GWDData + (sizeof(DTMKEY)));
	pBias = (LPLONG)((LPSTR)&lpGWDHead->GWDData + (sizeof(DTMKEY) + sizeof(SUBCELLINFO)));
	CompressedDTMData = (LPSTR)&lpGWDHead->GWDData + (sizeof(DTMKEY) + sizeof(SUBCELLINFO) + 4); 
   	if (!BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&CellID,BT_FIRST,BT_EQ,(LPSTR)&Offset)) 
   	{
		unsigned short	len, length;    
		LPLONG	pCell;

	    GSSillseek (lpGWDHead->Fid,Offset,0);
	    BigRead (lpGWDHead->Fid,(HPSTR)&len,2);
	    BigRead (lpGWDHead->Fid,(HPSTR)&lpGWDHead->GWDData,len); 
	    hCell = GSSiGlobAlloc (1100,GMEM_MOVEABLE,4096);
	    pCell = (LPLONG)GlobalLock (hCell);
		ExpandSubcell (pCell,*pSUBCELLInfo,CompressedDTMData,pBias);
		pCell[node] = IElev;
		*pSUBCELLInfo = CompressSubcell (pCell,CompressedDTMData,pBias); 
		length = sizeof(DTMKEY) + sizeof(SUBCELLINFO) + 4 + pSUBCELLInfo->LENGTH;
		if (length > len) 
			Offset = GSSillseek (lpGWDHead->Fid,0,2);
		else  
	    	GSSillseek (lpGWDHead->Fid,Offset,0);
	    BigWrite (lpGWDHead->Fid,(HPSTR)&length,2,-1);
	    BigWrite (lpGWDHead->Fid,(HPSTR)&lpGWDHead->GWDData,length,-1);
    	BT_PUT (lpGWDHead->BTHandle[0],(LPSTR)&CellID,(LPSTR)&Offset);
		GSSiGlobUlFree (&hCell);  
		rtn = TRUE;
	    SetGWDCurrentOffset (lpGWDHead,-1);
	} 
	GlobalUnlock (pDTMInfo->hDB);
{
																							#if ENABLETRACE
																							GSSiExitProg (1365);
																							#endif
	return rtn;
}
																							#if ENABLETRACE
																							}
																							#endif
} 

BOOL GetLIDARCell (long CellID,LPHANDLE phCell,LPDTMINFO pDTMInfo)
{ 
	LPLIDARCELL	pCell;  
	LIDARREC	LidarRec; 
	long	CellOffset4;
	LONGLONG CellOffset;
	double	CellMinX, CellMinY; 
	long	CellRow, CellCol; 
	USHORT	ip;
	
	if (pDTMInfo->Version > 1)
	{
		GSSillseek2(pDTMInfo->Fid, (long)sizeof(LIDARFILEHEADER) + CellID * 8, 0);
		BigRead(pDTMInfo->Fid, (HPSTR)&CellOffset, 8);
	}
	else
	{
		GSSillseek(pDTMInfo->Fid, (long)sizeof(LIDARFILEHEADER) + CellID * 4, 0);
		BigRead(pDTMInfo->Fid, (HPSTR)&CellOffset4, 4);
		CellOffset = CellOffset4;
	}
	if (CellOffset)
	{
		if (*phCell < (HANDLE)2)
			*phCell = GSSiGlobAlloc (0,GMEM_MOVEABLE,sizeof(LIDARCELL));
		pCell = (LPLIDARCELL)GlobalLock (*phCell);   
		CellRow = CellID / pDTMInfo->NumCols;
		CellCol = CellID % pDTMInfo->NumCols;
		GSSillseek2 (pDTMInfo->Fid,CellOffset,0);
		BigRead (pDTMInfo->Fid,(HPSTR)&LidarRec,sizeof(LIDARREC));  
		pCell->NumPoints = LidarRec.NumPoints; 
		CellMinX = pDTMInfo->SouthWestNode.x + (CellCol * pDTMInfo->GridSpace);
		CellMinY = pDTMInfo->SouthWestNode.y + (CellRow * pDTMInfo->GridSpace);
		for (ip=0;ip<LidarRec.NumPoints;ip++)
		{
			pCell->XY[ip].x = LidarRec.LidarPnt[ip].xoff/1000.0 + CellMinX;
			pCell->XY[ip].y = LidarRec.LidarPnt[ip].yoff/1000.0 + CellMinY;  
			pCell->Z[ip] = LidarRec.LidarPnt[ip].Elevation;
		}
		GlobalUnlock (*phCell); 
		return TRUE;
	}
	else
		return FALSE;
}

HANDLE GetNextLidarCell (LPDTMINFO pDTMInfo,LPDPOINT DTMPoint,LPSHORT pSurroundingCellID,LPDOUBLE pMaxDist)
{  
	HANDLE	handle=0;
	long	CellCol, CellRow, CellID;  
	USHORT	i, ip, MinUseID;
	long	CellOffset, MinUse=LONG_MAX; 
	LPLIDARCELL pCell; 
	LIDARREC	LidarRec;   
	double	CellMinX, CellMinY;
	static	long n1=0,n2=0;

Top:
	if (*pSurroundingCellID > 8)
		goto Exit; 
	CellCol = (DTMPoint->x - pDTMInfo->SouthWestNode.x)/pDTMInfo->GridSpace;
	CellRow = (DTMPoint->y - pDTMInfo->SouthWestNode.y)/pDTMInfo->GridSpace;
	n1++;
	if (*pMaxDist < pDTMInfo->GridSpace)
	{   
		DPOINT	CornerNode=pDTMInfo->SouthWestNode;
		
		CornerNode.x += CellCol * pDTMInfo->GridSpace;
		CornerNode.y += CellRow * pDTMInfo->GridSpace;
		switch (*pSurroundingCellID)
		{
			case 0:
				break;
			case 1:
				if (DTMPoint->x - CornerNode.x > *pMaxDist)
				{
					(*pSurroundingCellID)++;
					goto Top;
				}
				break;
			case 5:
				if ((CornerNode.x + pDTMInfo->GridSpace) - DTMPoint->x > *pMaxDist)
				{
					(*pSurroundingCellID)++;
					goto Top;
				}
				break;
			case 4:
				CornerNode.x += pDTMInfo->GridSpace;   
			case 2:  
				CornerNode.y += pDTMInfo->GridSpace;   
				goto CheckCorner;
			case 6:
				CornerNode.x += pDTMInfo->GridSpace;   
			case 8:  
	CheckCorner:
				if (ldistppmacro (DTMPoint,&CornerNode) > *pMaxDist)
				{
					(*pSurroundingCellID)++;
					goto Top;
				}
				break;
			case 3:
				if ((CornerNode.y + pDTMInfo->GridSpace) - DTMPoint->y > *pMaxDist)
				{
					(*pSurroundingCellID)++;
					goto Top;
				}
				break;
			case 7:
				if (DTMPoint->y - CornerNode.y > *pMaxDist)
				{
					(*pSurroundingCellID)++;
					goto Top;
				}
				break;
		}		
		n2++;
	}
	CellCol += LidarCellOffsets[*pSurroundingCellID].x;
	CellRow += LidarCellOffsets[*pSurroundingCellID].y;
	if (CellCol < 0 || CellCol >= pDTMInfo->NumCols || CellRow < 0 || CellRow >= pDTMInfo->NumRows)
	{
		(*pSurroundingCellID)++;
		goto Top;
	}
	(*pSurroundingCellID)++;
	CellID = pDTMInfo->NumCols * CellRow + CellCol;  
	for (i=0;i<MAXDTMCELLBUFFERS;i++) 
	{
		if (CellID == pDTMInfo->CellID[i])
		{
			pDTMInfo->CellUse[i]=++NextDTMUse;
			if (pDTMInfo->hCell[i] > (HANDLE)1)
			{
				handle = pDTMInfo->hCell[i];
				goto Exit;
			}
			else
				goto Top;
		}
		else if (pDTMInfo->CellUse[i] < MinUse)
		{
			MinUse = pDTMInfo->CellUse[i];
			MinUseID = i;
		}
	}
	if (!GetLIDARCell (CellID,&pDTMInfo->hCell[MinUseID],pDTMInfo)) 
	{
		if (pDTMInfo->hCell[MinUseID] > (HANDLE)1)
			GSSiGlobFree (&pDTMInfo->hCell[MinUseID]);
		pDTMInfo->hCell[MinUseID] = (HANDLE)1;
		goto Top;
	}
	else
		handle = pDTMInfo->hCell[MinUseID];
Exit:
	return handle;
}

double AddToUsePointList (LPDTMINFO pDTMInfo,double MaxDist,LPSHORT pnNearPoints,LPDOUBLE pDist,LPDPOINT pPoint,LPDOUBLE pElevation,
						  LPDOUBLE NearDist,LPDPOINT NearPoints,LPDOUBLE NearElevations)
{
	double	Maxd=0; 
	UINT	i, iMax=0;
	
	if (*pDist >= MaxDist)
		return MaxDist;
	if (*pDist > pDTMInfo->MaxDistToRawPoint)
		return MaxDist;
	if (*pnNearPoints < pDTMInfo->MaxRawPointsToUse)
	{   
		NearDist[*pnNearPoints] = *pDist;
		NearPoints[*pnNearPoints] = *pPoint; 
		NearElevations[(*pnNearPoints)++] = *pElevation;
		return MaxDist;
	}
	for (i=0;i<*pnNearPoints;i++)
	{
		if (NearDist[i] > Maxd)
		{
			Maxd = NearDist[i];
			iMax = i;
		}	
	}
	if (Maxd > *pDist)
	{	
		NearDist[iMax] = *pDist;
		NearPoints[iMax] = *pPoint; 
		NearElevations[iMax] = *pElevation;
		Maxd = 0;
		for (i=0;i<*pnNearPoints;i++)
			Maxd = max (Maxd,NearDist[i]);
	}
	return Maxd;
}

HANDLE GetDTMSubCellFromPoint (DPOINT Point,HANDLE hSurf)
{
	  LPDTMINFO	pDTMInfo = (LPDTMINFO)GlobalLock (hSurf);
      double    SPX=Point.x, SPY=Point.y, TSPX, TSPY, ELV, X, Y; 
      long		INTX, INTY, LSPXDM, LSPYDM;
      long      SURNOD[5], SNGNUM[5];
      short		SNSCNM[5], SNELNM[5]; 
      HANDLE	hCell=0;
	   
	  SPX -= pDTMInfo->SouthWestNode.x;
	  SPY -= pDTMInfo->SouthWestNode.y;
      TSPX   = SPX / pDTMInfo->GridSpace;
      TSPY   = SPY / pDTMInfo->GridSpace;
      X      = fmod (TSPX,1.0E0);
      Y      = fmod (TSPY,1.0E0);
      INTX   = TSPX;
      INTY   = TSPY;
      NGSANE (INTX,INTY,&SNGNUM[1],&SNSCNM[1],&SNELNM[1]);
      hCell = GetDTMSubCell (hSurf,SNGNUM[1],SNSCNM[1],pDTMInfo);
  	  GlobalUnlock (hSurf);
  	  return hCell;
}  

double NGIELV_bci (DPOINT Point,HANDLE hSurf,short DesiredUnits)
{
	doublexyz	xyzgrid[16], xyzpoint[256]; 
	LPDTMINFO	pDTMInfo; 
	USHORT	i,j;   
	double	x,y, elev;       
	DPOINT	StartPoint;
    
    if (!DTMSmooth)
    	return NGIELV (Point,hSurf,DesiredUnits);
//	testbci();
	pDTMInfo =(LPDTMINFO)GlobalLock (hSurf);	
	Point.x -= 1.5 * DTMSmooth;
	Point.y -= 1.5 * DTMSmooth;  
	StartPoint = Point;
    for(i = 0; i < 4; i++)
    {   
    	Point.y = StartPoint.y;
        for(j = 0; j < 4; j++)
        {
            x = i + 1.;
            y = j + 1.;
            xyzgrid[4*i+j].x = x;
            xyzgrid[4*i+j].y = y;
//            xyzgrid[4*i+j].x = Point.x - pDTMInfo->SouthWestNode.x;
//            xyzgrid[4*i+j].y = Point.y - pDTMInfo->SouthWestNode.y; 
			elev = NGIELV (Point,hSurf,DesiredUnits);
			if (elev == pDTMInfo->NULLElv)
			{
				GlobalUnlock (hSurf);  
				return elev;
			}
            xyzgrid[4*i+j].z = elev; 
            
            
            Point.y += DTMSmooth;
        }
        Point.x += DTMSmooth;
    }
	td_fillgrid(xyzgrid, 4, 4, xyzpoint, 8, 8);   
	GlobalUnlock (hSurf);
	return xyzpoint[27].z;
}

double NGIELV (DPOINT Point,HANDLE hSurf,short DesiredUnits)
																							#if ENABLETRACE
																							{GSSiEnterProg (1366);
																							#endif
{     
	//DesiredUnits (0=feet, 1=meters)
      double    SPX=Point.x, SPY=Point.y, TSPX, TSPY, ELV, X, Y; 
      long		INTX, INTY, LSPXDM, LSPYDM;
      long      SURNOD[5], SNGNUM[5];
      short		SNSCNM[5], SNELNM[5], NODE, NOFINN, MISING, I, SurfUnits,ii;  
      LPLONG	pCell;
      HANDLE	hCell;
	  LPDTMINFO	pDTMInfo; 
	  DPOINT	DTMPoint;
/*      long		INTMAX, BUFPNT, SCIDNO, NOFBUF,SAVEGN,GU,
     +          BIASBF(20), INPOS(2), SAVEBIAS,
     +          SCRTID(2)         , ISURID(2)           , SAVELENGTH,
     +          SURNOD(4), SNGNUM(4), SNSCNM(4), ACCESS(20), SCIDNS(20)
      INTEGER*2 SUBCBF(20480),INDATA(2048), IBEGIN, ZERO   ,
     +          SNELNM[5], NOFINN, IELV,
     +          IBEG(20), SAVESURF(1028)
      LOGICAL SAVENSINA(1024)
      POINTER /BUFPNT/ SUBCBF
      LOGICAL   INDT
      EQUIVALENCE (INDATA(1),INPUT1(1))
      COMMON / NGIELC / NEXTAC, NOFSIB
      DATA                            NAME$$/'NGIELV'/
      DATA      INTMAX / 2147483647 /,         NOFBUF/20/,
     +                      INPOS    / 1, 1025 /,GU/'GU'/,
     +          SCRTID    / 2, 1 /, ISURID    / +1, -1 /    */
//C******* FIND THE GEOSEGMENT, SUBCELL AND ELEMENT NUMBERS OF THE
//C        LOWER LEFT NODE OF THE NGI CELL IN WHICH THE POINT LIES. 
	  if (!hSurf)
{
																							#if ENABLETRACE
																							GSSiExitProg (1366);
																							#endif
	  	return PlaneElev;
}
	  pDTMInfo = (LPDTMINFO)GlobalLock (hSurf);
	  SurfUnits = pDTMInfo->ElevUnits; 
	  if (pDTMInfo->CoordUnits == 1)
	  {
	  	SPX = ConvertDist (SPX,1);
	  	SPY = ConvertDist (SPY,1);
	  }
	  else
	  {
	  	SPX = ConvertDist (SPX,2);
	  	SPY = ConvertDist (SPY,2);
	  }
	  DTMPoint.x = SPX;
	  DTMPoint.y = SPY; 
	  if (pDTMInfo->Type == DTMTYPE_LIDAR_LAZ)
		  ConvertCoord(&DTMPoint, 1, 0);
	  /*	  if (DTMPoint.x == 158715.0 &&  DTMPoint.y == 55275.0)
	  	  ii=1;
	  if (DTMPoint.x == 158754.0 &&  DTMPoint.y == 55296.0)
	  	  ii=1;  */
	  if (pDTMInfo->Bounds.xmx > pDTMInfo->Bounds.xmn)
	  {
		  if (!PointInBounds (DTMPoint,&pDTMInfo->Bounds))
		  	goto S500;
	  }
	  if (pDTMInfo->Type == DTMTYPE_TIN_GM)
	  {
		if (HaveLastTriangle)
		{
			if (POINT_IN_AREAD (DTMPoint,3, LastTrianglePoint,1,0,0,0))
			{
	    		ELV = GetTriangleElev (DTMPoint,LastTrianglePoint,LastTriangleElev);
	    		goto S1000;
			}
		}
		  { 
		  	LPVISLIST	SaveVis=CurVis;
	    	short	SaveMaxPick=MaxPick, SaveNFiles, SaveMT, SaveTimer, SavePT;
	    	HFILE	SaveFid; 
	    	HANDLE	hSaveVP = GSSiGlobAlloc (1835,GMEM_MOVEABLE,sizeof(VIEWPORT)+256);
	    	HANDLE	SavehDTM, SavehProfilePoints;
	    	LPVIEWPORT	pSaveVP = (LPVIEWPORT)GlobalLock (hSaveVP); 
	    	LPSTR	pSaveFile1=(LPSTR)(pSaveVP+1); 
	    	long	SavenPnts;
	    	HPDPOINT	SaveCurPoints; 
	    	char	SaveCRT[MAX_PREFIX_LEN+2], SaveCRU[MAX_UDI_LEN+2];
	    	long	SaveCRR;
	    	short	SaveCRD, SaveCRType;
	    	BOOL	SaveHP;  
	    	LPVIEWPORT	SaveVP;
			HANDLE	hSavePicklist;
			int		NumPickedSave;
			LPSTR	pSavePicklist;
		  	
		  	HaveLastTriangle = FALSE;
		    SaveVP = CurView;
			SetConfig (1);
		  	SetViewport (*pCommandViewport);
  		  	GSSiDeleteObject(&CurView->hRgn);
	    	*pSaveVP = *CurView; 
			SaveDC (CurView->hDC); 
			CurView->NumNewObjects = 0;
	    	CurView->NumFiles=1;    
	    	CurView->SubFile = 0;
	    	CurView->NumThemes=0;
			CurView->hBinFileList = 0;
	    	_fstrcpy (pSaveFile1,CurView->lpFiles[0]); 
	    	_fstrcpy (CurView->lpFiles[0],pDTMInfo->TINIndex);  
	    	CurView->FileType[0] = 4;
		  	CurVis = &pDTMInfo->VisList;
			UseUserPickAp =FALSE;
			SystemPickAp = -P_TOL*2;	
			MaxPick=1;  
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
		    SetPickAp(0);    
		    SavehProfilePoints = hProfilePoints;
			hSavePicklist = GSSiGlobAlloc (1836,GMEM_MOVEABLE,sizeof(PickList));
			pSavePicklist = GlobalLock (hSavePicklist);
			NumPickedSave = NumPicked;
			memmove (pSavePicklist,PickList,sizeof(PickList));
	    	PickItems2 (CurView->hWnd,DTMPoint,FALSE,TRUE,FALSE);  
	    	hProfilePoints = SavehProfilePoints;
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
			UseUserPickAp =TRUE;  
			MaxPick = SaveMaxPick;
			ELV = pDTMInfo->NULLElv;
	    	if (NumPicked) 
	    	{
	    		ELV = PickList[NumPicked-1].Elev; 
				memmove (PickList,pSavePicklist,sizeof(PickList));
				NumPicked = NumPickedSave;
				GSSiGlobUlFree (&hSavePicklist);
	    		goto S1000;
	    	}  
			memmove (PickList,pSavePicklist,sizeof(PickList));
			NumPicked = NumPickedSave;
			GSSiGlobUlFree (&hSavePicklist);
			GlobalUnlock (hSurf);
{
																								#if ENABLETRACE
																								GSSiExitProg (1366);
																								#endif
			return (ELV);
}
	  	} 
	  }
	  if (pDTMInfo->Type == DTMTYPE_LIDAR_GM)
	  {
		  short	nNearPoints = 0, SurroundingCellID = 0;
		  HANDLE	hCell;
		  LPLIDARCELL	pCell;
		  DPOINT	NearPoint[MAXNEARPOINTS];
		  double	NearDist[MAXNEARPOINTS], NearElevation[MAXNEARPOINTS];
		  double	MaxDist = DBL_MAX, d, Totd = 0, Tote = 0;
		  USHORT	i;

		  //DTMPoint.x = 162624.78067899420;
		  //DTMPoint.y = 43932.072766805089;
		  while ((hCell = GetNextLidarCell(pDTMInfo, &DTMPoint, &SurroundingCellID, &MaxDist)))
		  {
			  pCell = (LPLIDARCELL)GlobalLock(hCell);
			  for (i = 0; i<pCell->NumPoints; i++)
			  {
				  d = ldistppmacro(&DTMPoint, &pCell->XY[i]);
				  MaxDist = AddToUsePointList(pDTMInfo, MaxDist, &nNearPoints, &d, &pCell->XY[i], &pCell->Z[i], NearDist, NearPoint, NearElevation);
			  }
			  GlobalUnlock(hCell);
		  }
		  if (nNearPoints<2)
			  goto S500;
		  MaxDist = min(MaxDist, pDTMInfo->MaxDistToRawPoint);
		  for (i = 0; i<nNearPoints; i++)
		  {
			  //	  		d = sqrt (MaxDist - NearDist[i]); 
			  //			d = NearDist[i]; 
			  d = (1 / (NearDist[i] + 0.001));
			  d *= d;
			  Totd += d;
			  Tote += NearElevation[i] * d;
		  }
		  if (!Totd)
			  goto S500;
		  ELV = Tote / Totd;
		  goto S1000;
	  }
	  if (pDTMInfo->Type == DTMTYPE_LIDAR_LAZ)
	  {
		  static int ncalls = 0;

		  short	nNearPoints = 0, SurroundingCellID = 0;
		  HANDLE	hCell;
		  LPLIDARCELL	pCell;
		  DPOINT3D	NearPoint[MAXNEARPOINTS];
		  double	NearDist[MAXNEARPOINTS], NearElevation[MAXNEARPOINTS];
		  double	MaxDist = DBL_MAX, d, Totd = 0, Tote = 0;
		  USHORT	i;
			  MNMXCORD Bounds;
			  Bounds.xmn = DTMPoint.x - DTMRenderGridSpacing / 2;
			  Bounds.xmx = DTMPoint.x + DTMRenderGridSpacing / 2;
			  Bounds.ymn = DTMPoint.y - DTMRenderGridSpacing / 2;
			  Bounds.ymx = DTMPoint.y + DTMRenderGridSpacing / 2;

			  ncalls++;

			  if (pDTMInfo->db)
			  {
				  HANDLE hCmd = GSSiGlobAlloc(1796, GMEM_MOVEABLE, 1024);
				  LPSTR  pCmd = GlobalLock(hCmd);
				  sqlite3_stmt *statement;
				  sprintf(pCmd, "SELECT LIDAR.id, LIDAR.Name FROM LIDAR, LIDAR_index WHERE LIDAR.id=LIDAR_index.id AND maxX>=%f AND minX<=%f AND maxY>=%f AND minY<=%f", Bounds.xmn, Bounds.xmx, Bounds.ymn, Bounds.ymx);

				  SQLOK(sqlite3_prepare_v2(pDTMInfo->db, pCmd, -1, &statement, 0), pDTMInfo->db, "get points in bounds", 0);
				  while (sqlite3_step(statement) == SQLITE_ROW)
				  {
					  int id = sqlite3_column_int(statement, 0);
					  LPSTR LAZName = (LPSTR)sqlite3_column_text(statement, 1);
					  char filename[MAX_PATH];
					  sprintf(filename, "%s\\%s.laz", pDTMInfo->LAZDir, LAZName);

					  nNearPoints += getLAZPointsInBounds(&pDTMInfo->lazFiles,id,filename, 2, &Bounds, MAXNEARPOINTS - nNearPoints, &NearPoint[nNearPoints]);
				  }
				  GSSiGlobUlFree(&hCmd);
				  sqlite3_finalize(statement);
			  }
		 

		  if (nNearPoints<1)
			  goto S500;
		  Tote = 0;
		  for (i = 0; i<nNearPoints; i++)
		  {
			  Tote += NearPoint[i].z;
		  }
		  ELV = Tote / nNearPoints;
		  if (ELV < 0)
			  ELV = 0;
		  goto S1000;
	  }
	  SPX -= pDTMInfo->SouthWestNode.x;
	  SPY -= pDTMInfo->SouthWestNode.y;
      TSPX   = SPX / pDTMInfo->GridSpace;
      TSPY   = SPY / pDTMInfo->GridSpace;
      X      = fmod (TSPX,1.0E0);
      Y      = fmod (TSPY,1.0E0);
      INTX   = TSPX;
      INTY   = TSPY;
      NGSANE (INTX,INTY,&SNGNUM[1],&SNSCNM[1],&SNELNM[1]);
//C******* DETERMINE THE NUMBER OF SUBCELLS INVOLVED AND GO TO THE
//C        PROPER SECTION TO COMPUTE THE ELEVATION.
      if (SNELNM[1] > 992) goto S50;
      if (!(SNELNM[1] % 32)) goto S70;
//C**********************************************************************
//C *                                                                   *
//C *  SECTION 1: THIS SECTION COMPUTES THE ELEVATION OF THE POINT WHEN *
//C *             ALL 4 SURROUNDING NODES LIE WITHIN THE SAME SUBCELL.  *
//C *             ABOUT 94 PERCENT OF ALL CALLS TO THIS ROUTINE WILL    *
//C *             USE THIS SECTION.                                     *
//C *                                                                   *
//C *********************************************************************
//C******* PLACE THE SUBCELL INTO THE SUBCELL BUFFER. IF IT DOES NOT
//C        EXIST SET NGIELV TO INTMAX AND RETRN.
      NODE = 1;
      if (!(hCell = GetDTMSubCell (hSurf,SNGNUM[NODE],SNSCNM[NODE],pDTMInfo)))
      	goto S500;
//C******* DETERMINE THE ELEMENT NUMBERS OF THE REMAINING THREE
//C        SURROUNDING NODES, THEN DETERMINE HOW MANY OF THE SURROUNDING
//C        NODES ARE DETERMINATE AND PLACE THEIR ELEVATIONS INTO SURNOD.
      SNELNM[2] = SNELNM[1] + 32;
      SNELNM[3] = SNELNM[2] + 1;
      SNELNM[4] = SNELNM[1] + 1;
      NOFINN = 0;
      pCell = (LPLONG)GlobalLock (hCell);
      for (I=1; I<=4; I++)
      {   
      	  if (pCell[SNELNM[I]-1] == LONG_MAX)
      	  {
	          NOFINN++;
	          MISING = I;
	      } 
		  SURNOD[I] = pCell[SNELNM[I]-1];
   	  }    
   	  GlobalUnlock (hCell);  
S10:
   	  if (NOFINN > 1) 
   	  {
   	  	if (SURNOD[1] < LONG_MAX && X < P_TOL && Y < P_TOL)
   	  	{
   	  		ELV = SURNOD[1];
   	  		goto S15;
   	  	}
   	  	goto S500; 
   	  }
//C******* IF 3 OR 4 OF THE SURROUNDING NODES ARE DETERMINATE INTERPOLATE
//C        THE ELEVATION OF THE POINT; ELSE SET THE ELEVATION TO INTMAX
//C        AND RETRN.
      if (!NOFINN)
      	ELV = N4PNT (SURNOD,X,Y);
   	  else
   	  	ELV = N3PNT (SURNOD,X,Y,MISING); 
S15: 
   	  goto S1000;
//C *********************************************************************
//C *                                                                   *
//C *  SECTION 2: THIS SECTION COMPUTES THE ELEVATION OF A POINT WHEN   *
//C *             THE 4 SURROUNDING NODES LIE IN MORE THAN ONE SUBCELL. *
//C *                                                                   *
//C *********************************************************************
 S50: if (SNELNM[1] == 1024) goto S90;
//C******* POINT LIES BETWEEN TWO SUBCELLS SHARING AN EAST-WEST BOUNDARY.
      SNELNM[2] = SNELNM[1] - 992;
      SNELNM[3] = SNELNM[2] + 1;
      SNELNM[4] = SNELNM[1] + 1;
      if (SNSCNM[1] > 240) goto S60;
      SNGNUM[2] = SNGNUM[1];
      SNSCNM[2] = SNSCNM[1] + 16;
      goto S65;
 S60: SNGNUM[2] = SNGNUM[1] + 4096;
      SNSCNM[2] = SNSCNM[1] - 240;
 S65: SNGNUM[3] = SNGNUM[2];
      SNGNUM[4] = SNGNUM[1];
      SNSCNM[3] = SNSCNM[2];
      SNSCNM[4] = SNSCNM[1];
      goto S130;
//C******* POINT LIES BETWEEN TWO SUBCELLS SHARING A NORTH-SOUTH BOUNDARY.
 S70: SNELNM[2] = SNELNM[1] + 32;
      SNELNM[3] = SNELNM[2] - 31;
      SNELNM[4] = SNELNM[1] - 31;
      if (SNSCNM[1] % 16)
      {
	    SNGNUM[3] = SNGNUM[1];
	    SNSCNM[3] = SNSCNM[1] + 1;
	  }
	  else
	  {
		SNGNUM[3] = SNGNUM[1] + 1;
      	SNSCNM[3] = SNSCNM[1] - 15;
      } 

      SNGNUM[2] = SNGNUM[1];
      SNGNUM[4] = SNGNUM[3];
      SNSCNM[2] = SNSCNM[1];
      SNSCNM[4] = SNSCNM[3];
      goto S130;
//C******* POINT LIES AT JUNCTURE OF 4 SUBCELLS.
 S90: SNELNM[2] = 32;
      SNELNM[3] = 1;
      SNELNM[4] = 993;
      if (SNSCNM[1] <= 240)
      {
	      SNSCNM[2] = SNSCNM[1] + 16;
	      SNGNUM[2] = SNGNUM[1];
      }
      else
      {
		SNSCNM[2] = SNSCNM[1] - 240;
      	SNGNUM[2] = SNGNUM[1] + 4096; 
      } 

  	  if (SNSCNM[1] % 16)
  	  {
	      SNSCNM[3] = SNSCNM[2] + 1;
	      SNSCNM[4] = SNSCNM[1] + 1;
	      SNGNUM[3] = SNGNUM[2];
	      SNGNUM[4] = SNGNUM[1];
	  } 
	  else
	  {
	      SNSCNM[3] = SNSCNM[2] - 15;
	      SNSCNM[4] = SNSCNM[1] - 15;
	      SNGNUM[3] = SNGNUM[2] + 1;
	      SNGNUM[4] = SNGNUM[1] + 1;
	  }
S130: NOFINN = 0;
      for (NODE = 1; NODE <= 4; NODE++) 
      {
      	  if ((hCell = GetDTMSubCell (hSurf,SNGNUM[NODE],SNSCNM[NODE],pDTMInfo))) 
      	  {
      		pCell = (LPLONG)GlobalLock (hCell);
      	  	if (pCell[SNELNM[NODE]-1] != LONG_MAX)
      	  	{ 
      	  		SURNOD[NODE] = pCell[SNELNM[NODE]-1];
	      	  	GlobalUnlock (hCell);
      	  		continue;
      	  	} 
      	  	GlobalUnlock (hCell);
      	  }
  		  NOFINN++;
          MISING = NODE; 
          SURNOD[NODE] = LONG_MAX;
      }
      goto S10;
//C******* ELEVATION DOES NOT EXIST AT THIS POINT.
S500: 
	ELV = pDTMInfo->NULLElv;
	GlobalUnlock (hSurf);
{
																							#if ENABLETRACE
																							GSSiExitProg (1366);
																							#endif
	return (ELV);
}
S1000:
	GlobalUnlock (hSurf);
	switch (DesiredUnits)
	{
		case 0: //want feet
			switch (SurfUnits)
			{   
				case 0: //have feet 
				break;
				case 1: //have feet*100
				ELV /= 100.0;
				break;
				case 2://have decimeters
				ELV /= 100.0; 
				case 3: //have meters
				ELV *= MFT;
				break;
			}
		break;
		
		case 1: //want meters
			switch (SurfUnits)
			{   
				case 0: //have feet
				ELV *= FTM;
				break; 
				case 1: //have feet*100 
				ELV /= 100.0;
				ELV *= FTM;
				break;
				case 2: //have decimeters
				ELV /= 100.0; 
				case 3: //have meters
				break;
			}
		break;
	}
{
																							#if ENABLETRACE
																							GSSiExitProg (1366);
																							#endif
	return (ELV);
}
																							#if ENABLETRACE
																							}
																							#endif
} 

BOOL SetNGIELV (DPOINT Point,double Elev,HANDLE hSurf,short Units)
																							#if ENABLETRACE
																							{GSSiEnterProg (1367);
																							#endif
{     
	//Units (0=feet, 1=meters)
      double    SPX=Point.x, SPY=Point.y, TSPX, TSPY, ELV, X, Y, CurElev; 
      long		INTX, INTY, LSPXDM, LSPYDM;
      long      SURNOD[5], SNGNUM[5];
      short		SNSCNM[5], SNELNM[5], NODE, NOFINN, MISING, I, SurfUnits;  
      LPLONG	pCell;
      HANDLE	hCell;
	  LPDTMINFO	pDTMInfo; 
	  BOOL		rtn;  
static	ii=0,debugii=171;
/*      long		INTMAX, BUFPNT, SCIDNO, NOFBUF,SAVEGN,GU,
     +          BIASBF(20), INPOS(2), SAVEBIAS,
     +          SCRTID(2)         , ISURID(2)           , SAVELENGTH,
     +          SURNOD(4), SNGNUM(4), SNSCNM(4), ACCESS(20), SCIDNS(20)
      INTEGER*2 SUBCBF(20480),INDATA(2048), IBEGIN, ZERO   ,
     +          SNELNM[5], NOFINN, IELV,
     +          IBEG(20), SAVESURF(1028)
      LOGICAL SAVENSINA(1024)
      POINTER /BUFPNT/ SUBCBF
      LOGICAL   INDT
      EQUIVALENCE (INDATA(1),INPUT1(1))
      COMMON / NGIELC / NEXTAC, NOFSIB
      DATA                            NAME$$/'NGIELV'/
      DATA      INTMAX / 2147483647 /,         NOFBUF/20/,
     +                      INPOS    / 1, 1025 /,GU/'GU'/,
     +          SCRTID    / 2, 1 /, ISURID    / +1, -1 /    */
//C******* FIND THE GEOSEGMENT, SUBCELL AND ELEMENT NUMBERS OF THE
//C        LOWER LEFT NODE OF THE NGI CELL IN WHICH THE POINT LIES. 
	  if (!hSurf)
{
																							#if ENABLETRACE
																							GSSiExitProg (1367);
																							#endif
	  	return FALSE;
}
	  pDTMInfo = (LPDTMINFO)GlobalLock (hSurf);
	  SurfUnits = pDTMInfo->ElevUnits; 
	  if (pDTMInfo->CoordUnits == 1)
	  {
	  	SPX = ConvertDist (SPX,1);
	  	SPY = ConvertDist (SPY,1);
	  }
	  else
	  {
	  	SPX = ConvertDist (SPX,2);
	  	SPY = ConvertDist (SPY,2);
	  }
	  SPX -= pDTMInfo->SouthWestNode.x;
	  SPY -= pDTMInfo->SouthWestNode.y;
      TSPX   = SPX / pDTMInfo->GridSpace;
      TSPY   = SPY / pDTMInfo->GridSpace;
      INTX   = IDNINT(TSPX);
      INTY   = IDNINT(TSPY);
      NGSANE (INTX,INTY,&SNGNUM[1],&SNSCNM[1],&SNELNM[1]);
//C******* DETERMINE THE NUMBER OF SUBCELLS INVOLVED AND GO TO THE
//C        PROPER SECTION TO COMPUTE THE ELEVATION.
//C**********************************************************************
//C *                                                                   *
//C *  SECTION 1: THIS SECTION COMPUTES THE ELEVATION OF THE POINT WHEN *
//C *             ALL 4 SURROUNDING NODES LIE WITHIN THE SAME SUBCELL.  *
//C *             ABOUT 94 PERCENT OF ALL CALLS TO THIS ROUTINE WILL    *
//C *             USE THIS SECTION.                                     *
//C *                                                                   *
//C *********************************************************************
//C******* PLACE THE SUBCELL INTO THE SUBCELL BUFFER. IF IT DOES NOT
//C        EXIST SET NGIELV TO INTMAX AND RETRN.
      NODE = 1;
      ii++;
      if (ii == debugii)
      	ii=0;
	  rtn = SetDTMSubCell (SNGNUM[NODE],SNSCNM[NODE],SNELNM[NODE]-1, Elev,Units, pDTMInfo);
	  GlobalUnlock (hSurf);
{
																							#if ENABLETRACE
																							GSSiExitProg (1367);
																							#endif
	  return rtn;
}
																							#if ENABLETRACE
																							}
																							#endif
}


long GetDTMHoles (LPSTR DTMName, LPSTR OutFile)
																							#if ENABLETRACE
																							{GSSiEnterProg (1368);
																							#endif
{
	long		CellID, nHoles=0;
	DTMKEY		DTMKey; 
	LPDTMINFO	pDTMInfo;
	SUBCELLINFO	SUBCELLInfo;
	LPSTR		CompressedDTMData;
	UINT		i,j;  
	long		Offset;
	LPLONG		pBias;  
	LPSUBCELLINFO	pSUBCELLInfo;
	LPGWDHEADER	lpGWDHead;     
	HANDLE		handle=0, hSurf;  
	short	pos = BT_FIRST;   
	double	XLL, YLL; 
	long	Cell[32][32]; 
	char	str[260];
	HFILE	OutFid;
	OFSTRUCTGM	OFStruct;
	long	nRecs, nLoaded=0;
	
	hSurf = DTMOpen (DTMName, LONG_MAX,BT_READ,0);
	if (!hSurf)
{
																							#if ENABLETRACE
																							GSSiExitProg (1368);
																							#endif
		return 0; 
}   
	OutFid = GSSiOpenFile (OutFile,&OFStruct,OF_CREATE);  
	fputstring ("\"X\",\"Y\"",OutFid);
	pDTMInfo = (LPDTMINFO)GlobalLock (hSurf);
	lpGWDHead = (LPGWDHEADER)GlobalLock (pDTMInfo->hDB); 
	pSUBCELLInfo = (LPSUBCELLINFO)((LPSTR)&lpGWDHead->GWDData + (sizeof(DTMKEY)));
	pBias = (LPLONG)((LPSTR)&lpGWDHead->GWDData + (sizeof(DTMKEY) + sizeof(SUBCELLINFO)));
	CompressedDTMData = (LPSTR)&lpGWDHead->GWDData + (sizeof(DTMKEY) + sizeof(SUBCELLINFO) + 4); 

    nRecs = BT_NUM_IN_INDEX (lpGWDHead->BTHandle[0]);
	CreateStatusWind (hWndMain,1,0);

   	while (ContinueProcessing && !BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&DTMKey,pos,BT_ANY,(LPSTR)&Offset)) 
   	{
		unsigned short	len;    
		LPLONG	pCell;
        
        pos = BT_NEXT;
	    GSSillseek (lpGWDHead->Fid,Offset,0);
	    BigRead (lpGWDHead->Fid,(HPSTR)&len,2);
	    BigRead (lpGWDHead->Fid,(HPSTR)&lpGWDHead->GWDData,len); 
	    if (pSUBCELLInfo->INDT)
	    {   
	    	XLL = pDTMInfo->SouthWestNode.x + ((long)DTMKey.GEOSEG_COL * 16 * 32 + (long)DTMKey.SUBCEL_COL * 32) * pDTMInfo->GridSpace;
	    	YLL = pDTMInfo->SouthWestNode.y + ((long)DTMKey.GEOSEG_ROW * 16 * 32 + (long)DTMKey.SUBCEL_ROW * 32) * pDTMInfo->GridSpace;
			ExpandSubcell ((LPLONG)Cell,*pSUBCELLInfo,CompressedDTMData,pBias);
			for (i=0;i<32;i++)
				for (j=0;j<32;j++)
					if (Cell[i][j] == LONG_MAX)
					{   
						sprintf (str,"%.2f,%.2f",XLL+j*pDTMInfo->GridSpace,YLL+i*pDTMInfo->GridSpace);
						fputstring (str,OutFid);
						nHoles++;
					}
		}
		sprintf (str,"%ld voids located",nHoles);
		StatusWindowUpdate (str,"Locate DTM voids", nRecs, ++nLoaded);
	} 
	GlobalUnlock (pDTMInfo->hDB);   
	DTMClose (&hSurf); 
	GSSiClose2 (&OutFid);
	DestroyStatusWindow(0);  
	SetContinueProcessing ( TRUE);
{
																							#if ENABLETRACE
																							GSSiExitProg (1368);
																							#endif
	return nHoles;
}
																							#if ENABLETRACE
																							}
																							#endif
}


BOOL DTMCopy (LPSTR ToSurf,LPSTR FromSurf,LPSTR AreaRefOrTAG)
{
	return TRUE;
}



long GetLIDARCellID (LPDTMINFO pDTMInfo,LPDPOINT Point)
{
	long	CellCol = (Point->x - pDTMInfo->SouthWestNode.x)/pDTMInfo->GridSpace;
	long	CellRow = (Point->y - pDTMInfo->SouthWestNode.y)/pDTMInfo->GridSpace;  
	long	CellID;
	
	if (CellCol < 0 || CellCol >= pDTMInfo->NumCols || CellRow < 0 || CellRow >= pDTMInfo->NumRows)
		CellID = -1;
	else
		CellID = pDTMInfo->NumCols * CellRow + CellCol; 
	return CellID;
} 

BOOL GetNextDTMSegment (BOOL Init)
{   
   	LPDTMINFO	pDTMInfo;   
   	BOOL		rtn=FALSE;
	DPOINT	Point;   
	HPDOUBLE	pRenderNode;   
	UINT	i;  
   	if (!hDTM)
   		return FALSE; 
   	pDTMInfo = (LPDTMINFO)GlobalLock (hDTM);
   	if (Init)
   	{
		MNMXCORD bounds = pDTMInfo->Bounds;
		if (pDTMInfo->Type == DTMTYPE_LIDAR_LAZ)
			ConvertBounds(&bounds, 0, 1);
		if (pDTMInfo->Type < 1 && !bounds.xmx)
		{
   			DTMBounds = CurView->WBounds;
		   	InflateBounds (&DTMBounds,pDTMInfo->GridSpace*32); 
   			DTMPoint.x = DTMBounds.xmn;
   			DTMPoint.y = DTMBounds.ymn; 
		}
		else if (!IntersectBounds (&CurView->WBounds,&bounds,&DTMBounds))
   		{
   			DTMBounds = CurView->WBounds;
   			DTMPoint.x = DTMBounds.xmx;
   			DTMPoint.y = DTMBounds.ymx; 
   		}
   		else
   		{
			if (DTMRenderAs == DTM_RENDER_RAW_POINTS)
			{
	   			if (pDTMInfo->Type < 2)
	   			{
		   			InflateBounds (&DTMBounds,pDTMInfo->GridSpace*32); 
					DTMPoint.x = DTMBounds.xmn - pDTMInfo->GridSpace*32; 
				}
	   			else 
	   			{
		   			InflateBounds (&DTMBounds,pDTMInfo->GridSpace); 
					DTMPoint.x = DTMBounds.xmn - pDTMInfo->GridSpace; 
				}
			}
			else
				DTMPoint.x = DTMBounds.xmn - DTMRenderGridSpacing;
			DTMPoint.y = DTMBounds.ymn;
		}
		DTMRenderUnits = 0;
		if (DTMRenderAs == DTM_RENDER_SLOPE_VECTORS || DTMRenderAs == DTM_RENDER_SLOPE_POLYGONS)
			DTMRenderUnits = 1;   
		if (DTMRenderAs != DTM_RENDER_RAW_POINTS)
		{   
			DTMRow = -2;
			DTMPoint.x -= fmod (DTMPoint.x,DTMRenderGridSpacing);
			DTMPoint.y -= fmod (DTMPoint.y,DTMRenderGridSpacing);  
			DTMRenderRowBeginPoint[2] = DTMPoint;
			DTMNumPointsInRenderGridRow = 2+ (DTMBounds.xmx - DTMPoint.x)/DTMRenderGridSpacing; 
			hDTMRenderGridRow[2] = GSSiGlobAlloc (0,GMEM_MOVEABLE,DTMNumPointsInRenderGridRow*8);
			pRenderNode = (HPDOUBLE)GlobalLock (hDTMRenderGridRow[2]);
			Point = DTMPoint;
			for (i=0;i<DTMNumPointsInRenderGridRow;i++)
			{   
				pRenderNode[i] = NGIELV_bci (Point,hDTM,DTMRenderUnits); 
				if (DTMRenderAs == DTM_RENDER_CONTOURS && pRenderNode[i] != pDTMInfo->NULLElv)
				{
					if (!fmod (pRenderNode[i],DTMContourInterval))
						pRenderNode[i] += DTMContourInterval/1000.0;
				}
				Point.x += DTMRenderGridSpacing;
			} 
			GlobalUnlock (hDTMRenderGridRow[2]);
		}

		rtn = TRUE;   		
		goto Exit;
   	} 
Next:
	if (DTMRenderAs == DTM_RENDER_RAW_POINTS)
	{   
		switch (pDTMInfo->Type)
		{   
			case 0:
			case 1:  
		NextGrid:
			   	DTMPoint.x += pDTMInfo->GridSpace*32;
			   	if (DTMPoint.x > DTMBounds.xmx)
			   	{
					DTMPoint.x = DTMBounds.xmn;
					DTMPoint.y +=pDTMInfo->GridSpace*32; 
					if (DTMPoint.y > DTMBounds.ymx)
						goto Exit;
				}
				if ((DTMSubCellHandle = GetDTMSubCellFromPoint (DTMPoint,hDTM)))
				{
					rtn = TRUE;
					goto Exit;
				}
				else
					goto NextGrid;
			break;
			
			case 2:
			{
				rtn = TRUE;
				goto Exit;
			}
			break;
			
			case 3:
NextLidar:
			   	DTMPoint.x += pDTMInfo->GridSpace;
			   	if (DTMPoint.x > DTMBounds.xmx)
			   	{
					DTMPoint.x = DTMBounds.xmn;
					DTMPoint.y +=pDTMInfo->GridSpace; 
					if (DTMPoint.y > DTMBounds.ymx)
						goto Exit;
				}
				DTMCellID = GetLIDARCellID (pDTMInfo,&DTMPoint); 
				GSSiGlobFree (&DTMCellHandle);
				if (GetLIDARCell (DTMCellID,&DTMCellHandle,pDTMInfo)) 
				{
					rtn = TRUE;  
					goto Exit;
				}
				else
					goto NextLidar; 
			break;
		} 
	} 
	if (DTMRenderAs == DTM_RENDER_SLOPE_VECTORS && !(CurView->PassID == 0 || CurView->PassID == 3))
		goto Exit;  
	if (DTMRenderAs == DTM_RENDER_SLOPE_POLYGONS && !(CurView->PassID == 0 || CurView->PassID == 2))
		goto Exit;  
	if (DTMPoint.y > DTMBounds.ymx)
		goto Exit;   
	GSSiGlobFree (&hDTMRenderGridRow[0]);
	GSSiGlobFree (&hDTMRenderGridRow[1]);
	hDTMRenderGridRow[0] = hDTMRenderGridRow[2];  
	DTMRenderRowBeginPoint[0] = DTMRenderRowBeginPoint[2];
	hDTMRenderGridRow[1] = GSSiGlobAlloc (0,GMEM_MOVEABLE,DTMNumPointsInRenderGridRow*8);
	hDTMRenderGridRow[2] = GSSiGlobAlloc (0,GMEM_MOVEABLE,DTMNumPointsInRenderGridRow*8);
	DTMPoint.y += DTMRenderGridSpacing/2; 
/*	pRenderNode = (HPDOUBLE)GlobalLock (hDTMRenderGridRow[1]);
	Point = DTMPoint;
	Point.x += DTMRenderGridSpacing/2;
	DTMRenderRowBeginPoint[1] = Point;
	for (i=0;i<DTMNumPointsInRenderGridRow;i++)
	{   
		pRenderNode[i] = NGIELV_bci (Point,hDTM,DTMRenderUnits); 
		if (DTMRenderAs == DTM_RENDER_CONTOURS && pRenderNode[i] != pDTMInfo->NULLElv)
		{
			if (!fmod (pRenderNode[i],DTMContourInterval))
				pRenderNode[i] += DTMContourInterval/1000.0;
		}
		Point.x += DTMRenderGridSpacing;
	}
	GlobalUnlock (hDTMRenderGridRow[1]); */
	DTMPoint.y += DTMRenderGridSpacing/2;
	pRenderNode = (HPDOUBLE)GlobalLock (hDTMRenderGridRow[2]);
	Point = DTMPoint;
	DTMRenderRowBeginPoint[2] = Point;
	for (i=0;i<DTMNumPointsInRenderGridRow;i++)
	{   
		pRenderNode[i] = NGIELV_bci (Point,hDTM,DTMRenderUnits); 
		if (DTMRenderAs == DTM_RENDER_CONTOURS && pRenderNode[i] != pDTMInfo->NULLElv)
		{
			if (!fmod (pRenderNode[i],DTMContourInterval))
				pRenderNode[i] += DTMContourInterval/1000.0;
		}
		Point.x += DTMRenderGridSpacing;
	}
	GlobalUnlock (hDTMRenderGridRow[2]);   

	{
		HPDOUBLE	pRenderNode[3];   
		USHORT	ip;

		pRenderNode[0] = (HPDOUBLE)GlobalLock (hDTMRenderGridRow[0]);  
		pRenderNode[1] = (HPDOUBLE)GlobalLock (hDTMRenderGridRow[1]);  
		pRenderNode[2] = (HPDOUBLE)GlobalLock (hDTMRenderGridRow[2]);  
		for (ip=0;ip<DTMNumPointsInRenderGridRow-1;ip++)
			if (pRenderNode[0][ip] 	 != pDTMInfo->NULLElv &&
				pRenderNode[0][ip+1] != pDTMInfo->NULLElv &&
				pRenderNode[2][ip]   != pDTMInfo->NULLElv &&
				pRenderNode[2][ip+1] != pDTMInfo->NULLElv)
	
				pRenderNode[1][ip] = (pRenderNode[0][ip]   +
									  pRenderNode[0][ip+1] +
									  pRenderNode[2][ip]   +
									  pRenderNode[2][ip+1]) / 4;
			else
				pRenderNode[1][ip] = pDTMInfo->NULLElv;	
		GlobalUnlock (hDTMRenderGridRow[0]); 		
		GlobalUnlock (hDTMRenderGridRow[1]); 		
		GlobalUnlock (hDTMRenderGridRow[2]); 		
    }

	DTMRow += 2;
	rtn = TRUE;
Exit:
   	GlobalUnlock (hDTM); 
	return rtn;
} 

BOOL DisplayDTMPoint (long Refno,DPOINT DPoint,double Elevation,double AZ,short SymbolNumber,double Size,LPDTMINFO	pDTMInfo)
{   
	double	size;
	
	CurrentDesc = SymbolNumber; 
	CurrentRefno = Refno;
	if (CurView->PassID == 2)
		return FALSE;  
	HiPrecis = TRUE;
    lpDCurPoints = &DPoint;  
	CurrentPoint = CurPointLocD = DPoint;
	CurPointLoc = BasePtToWinPt(lpDCurPoints); 
	CurPointZ = Elevation; 
	CurPointAZ = AZ;
    PTRot = CurPointAZ;  
    GlobalColors[0]=0;
	nPnts = 1; 
	if (!PointInMaskAreaWinCoord (CurPointLoc))
		return FALSE;
	if (PointIsBlocked (&CurPointLocD,CurrentDesc))
		return FALSE;   
	HaveTXLoc = TRUE;
	CurrentType = GF_POINT;
	CurPointSize = Size;
	if (CurPointSize < 0)
		CurPointSize = -CurPointSize * DeviceToScreenFactor();
	else
		CurPointSize /= CurView->BaseUnitsPerPixel;
	if (GetTypeVisibility(TYPE_POINT))
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
			CurPointSize = 10*DeviceToScreenFactor();
			iDesc = InvisiblePointSymbol;
		}
		if (SetDisplayChar (CurView->hDC,GF_POINT,CurrentRefno,CurrentDesc,CurrentPrefix,CurrentUDI) > 0)
		{   
			if (ThemePointSym)
			{
				iDesc = ThemePointSym;
				if (ThemePointSize < 0)
					size = -ThemePointSize *DeviceToScreenFactor();
				else
					size = ThemePointSize / CurView->BaseUnitsPerPixel; 
				size *= ThemeWidthFactor*GraphicsPointFactor;
				size = min(max (size,1),MaxPointSize);
			}
			else
				size = CurPointSize*ThemeWidthFactor*GraphicsPointFactor; 
			SetROP2(CurView->hDC,R2_COPYPEN);
			DisplayPointItem (CurView->hDC,CurPointLoc,size/2,PTRot,iDesc,&CurView->MaxSymbolWidth);
			if (pDTMInfo && GetTypeVisibility(TYPE_TEXT))
			{  
				if (CurView->BaseUnitsPerPixel*100 < pDTMInfo->GridSpace)
				{
					char	str[64]; 
					
					sprintf (str,"%.2f",Elevation);
					
        			SetTextColor (CurView->hDC,ConvertColor(GlobalColors[0],CurrentDesc));
					DispText (CurView->hDC,FALSE,CurPointLoc.x,CurPointLoc.x, CurPointLoc.y,0, 2,1,
			  				  fabs(size*5),1,1,2, FALSE,0,str,0,FALSE,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0); 
					
				}
			}
		} 
	}
	TXLoc = CurPointLocD; 
	HaveTXLoc = 1; 
	ShowValue (CurView->hDC,FALSE); 
	return TRUE;
} 

BOOL DisplayDTMArea (long Refno,LPDPOINT DPoint,short Nump,double SlopePCT,double AZ,short SymbolNumber,LPDTMINFO pDTMInfo)
{   
	double	size;
	
	CurrentDesc = SymbolNumber; 
	CurrentRefno = Refno;
	if (CurView->PassID && CurView->PassID != 2)
		return FALSE;  
	HiPrecis = TRUE;  
	ItemIsHighlighted = FALSE;
    lpDCurPoints = DPoint;  
    GlobalColors[0]=0;
	nPnts = Nump; 
	CurSlopePCT = SlopePCT;
	CurSlopeAngle = AZ * RADtoDEG;
	if (PolyInMaskAreaFileCoord (CurrentType,&nPnts,&hCoords,&lpCurPoints,&lpDCurPoints,HiPrecis))
	{
	    if (CurrentDesc > 0 && CurrentDesc < 3201)
			CurView->CurVisType[CurrentDesc]=3;
		if (SetDisplayChar (CurView->hDC,GF_AREA,CurrentRefno,CurrentDesc,CurrentPrefix,CurrentUDI) > 0)
		{
			GWPolygonD (CurView->hDC,lpDCurPoints,nPnts,1,0,CurrentDesc,FALSE,TRUE,0);
		}
	}  

	ShowValue (CurView->hDC,FALSE); 
	return TRUE;
} 

double WeightedAZ (LPDOUBLE AZ, LPDOUBLE W,long n)
{
	double	WAZ=AZ[0]; 
	double	TotW=W[0];
	ULONG	i;  
	double	daz;
	
	for (i=1;i<n;i++) 
	{   
		TotW += W[i];
		if (TotW)
		{
			daz = DeltaAZmacro (WAZ,AZ[i]);
			daz *= W[i]/TotW;
			WAZ = LTWOPImacro(WAZ + daz);
		}
		else
			WAZ = AZ[i];
	}
	return WAZ;
}

BOOL FindOpContourPoint (USHORT StartSide,LPDOUBLE pZC,LPDPOINT Point2,LPDPOINT TriPoints,LPDOUBLE Z)
{   
	USHORT	iside;
	double	Dist, d, AZ, ElevDiff, pct;
	
	for (iside=StartSide+1;iside<3;iside++)
	{
		if (Z[iside] < *pZC && *pZC < Z[iside+1] ||
			Z[iside] > *pZC && *pZC > Z[iside+1])
		{
			ElevDiff = Z[iside+1]-Z[iside];
			Dist = ldistppmacro (&TriPoints[iside],&TriPoints[iside+1]);  
			AZ = getazdmacro (&TriPoints[iside],&TriPoints[iside+1]);
			pct = (*pZC - Z[iside])/ElevDiff;
			d = pct * Dist;
			*Point2 = dnewptmacro (TriPoints[iside],AZ,d);
			return TRUE;
		} 
	}
	return FALSE;
}  

void DisplayContourLabels (BOOL Clear)
{   
	USHORT	i,j;
	short	nChar, loc, StartPoint, twidth, theight, ichar, dy, ipass, ntxp, EndLine, BegLine=0;    
	BOOL	Flip;
	LPSHORT	pNumPoints; 
	LPLONG	pElev;
	LPPOINT	pPoints;
	char	ElevText[16];
	LOGFONT	LogFont;    
	HFONT	hFont, OldFont;    
	DWORD	TextExt;
	double	AZ, Dist, TotLength, CharWidth, StartDist, dtol, pinc; 
	POINT	Point, TempPoint; 
	DPOINT	DPoint, DPoint1, DPoint2, FlipPoint[2]; 
	double	TxtAZ[12];  
	double	MaxDeflection=HALFPI/3;  
	DPOINT	TxtPoints[12]; 
	BOOL	LightContour;
	double	Elv;
	HPEN	hPen;
	POINT	TextPoint[16] = { 0 };
	double	TextAZ[16] = { 0 };
	char	TextChar[16];
	int		ii;
				
		
	if (!Clear)
	{  
		SaveDC (CurView->hDC); 
		InitRecord (CurView->hDC); 
		_fmemset (&LogFont,0,sizeof(LOGFONT));
   		LogFont.lfHeight = -(ContourTextSize+10) * DeviceToScreenFactor();
	    _fstrcpy (LogFont.lfFaceName,"Courier New");
        _fstrcpy(LogFont.lfFaceName, "Arial Rounded MT Bold");
		SetDisplayMode (CurView->hDC, GF_TEXTMODE);
		SelectObject (CurView->hDC,GetStockObject(BLACK_PEN));
	    SetBkMode(CurView->hDC, OPAQUE);
     	SetTextColor(CurView->hDC, AutoYellow (ContourTextColor));
		for (i=0;i<nContourLines;i++)   
		{
			SIZE txSize;
			int	 nTextChar=0;

			pNumPoints = (LPSHORT)GlobalLock (hContourLines[i]); 
			pElev   = (LPLONG)(pNumPoints+1);
			pPoints = (LPPOINT)(pElev+1);   
			EndLine = *pNumPoints-1;  
			Elv = *pElev * DTMContourInterval;
			ltoa (IDNINT(Elv),ElevText,10);   
			LightContour = fmod (Elv,DTMContourInterval*5);
			TotLength = GetPolyLength (pPoints,*pNumPoints);
			nChar = _fstrlen (ElevText); 
			ntxp = nChar+1; 
	   		LogFont.lfEscapement = 0; 
			LogFont.lfWeight = 400;
		    hFont = CreateFontIndirect((LPLOGFONT)&LogFont); 
		    OldFont = SelectObject(CurView->hDC,hFont); 
			GetTextExtentPoint32 (CurView->hDC,ElevText,nChar,&txSize);
			SelectObject(CurView->hDC,OldFont);
			GSSiDeleteObject(&hFont);
			twidth = txSize.cx; 
			theight = txSize.cy;   
			if (TotLength < twidth * 3) 
			{
				TxtPoints[0] = PointToDPoint(pPoints[EndLine]);
				goto NextLine;  
			}
			CharWidth = 1.25*(double)twidth/nChar; 
			dtol = CharWidth * 0.1;
			Dist = StartDist = TotLength/2 - (double)twidth/2 - CharWidth; 
	TryAgain:
			Dist += CharWidth; 
			
			TxtPoints[0] = PointAtDistOnPoly16 (pPoints,*pNumPoints,Dist,&AZ,&EndLine);
			for (j=1;j<ntxp;j++)  
			{   
				double	dinc = CharWidth - dtol; 
				short	n=0;
				
				Dist+=CharWidth;
				do                            //pPoints[7]
				{   
					Dist += CharWidth - dinc + dtol;
					TxtPoints[j] = PointAtDistOnPoly16 (pPoints,*pNumPoints,Dist,&AZ,&BegLine); 
					if (BegLine >= *pNumPoints) 
					{
						EndLine = *pNumPoints-1;
						TxtPoints[0] = PointToDPoint(pPoints[EndLine]);
						goto NextLine; 
					}
					dinc = ldistp (TxtPoints[j-1],TxtPoints[j]) + dtol;  
					n++;
					if (n>100)
					{
						EndLine = *pNumPoints-1;
						TxtPoints[0] = PointToDPoint(pPoints[EndLine]);
						goto NextLine; 
					}
				}while (dinc < CharWidth);    
				TxtAZ[j-1] = getazd (&TxtPoints[j-1],&TxtPoints[j]);
				if (j > 1)
					if (fabs(DeflectionAngle (TxtAZ[j-2],TxtAZ[j-1])) > MaxDeflection)
						goto TryAgain;
			}
			DPoint1 = WinPtToBasePtD (&TxtPoints[0]);  
			DPoint2 = WinPtToBasePtD (&TxtPoints[ntxp-1]);  
			Flip = FALSE;
			AZ = getazd (&DPoint1,&DPoint2);
			if (AZ > HALFPI && AZ < 3*HALFPI)
				Flip = TRUE;
			for (ipass=0;ipass<2;ipass++)
			{
				Dist = StartDist;
				for (ichar = 0;ichar < nChar;ichar++)   //pPoints[21]
				{   
					short	jchar=ichar;
					
					if (Flip)
						jchar = nChar - ichar -1;
					DPoint1 = WinPtToBasePtD (&TxtPoints[ichar]);  
					DPoint2 = WinPtToBasePtD (&TxtPoints[ichar+1]);  
//					DPoint = MidPointD (DPoint1,DPoint2);
					AZ = getazd (&DPoint1,&DPoint2); 
					pinc = 0.25;
					if (Flip)
						pinc = 0.75;
					DPoint = dnewpt (DPoint1,AZ,pinc*ldistp(DPoint1,DPoint2));
/*			Point = BasePtToWinPt (&DPoint);
			SetPixel (CurView->hDC,Point.x,Point.y,RGB(255,0,0));		 
			SetPixel (CurView->hDC,Point.x-1,Point.y,RGB(255,0,0));		 
			SetPixel (CurView->hDC,Point.x+1,Point.y,RGB(255,0,0));		 
			SetPixel (CurView->hDC,Point.x,Point.y-1,RGB(255,0,0));		 
			SetPixel (CurView->hDC,Point.x,Point.y+1,RGB(255,0,0));		 
			SetPixel (CurView->hDC,Point.x-1,Point.y-1,RGB(255,0,0));		 
			SetPixel (CurView->hDC,Point.x-1,Point.y+1,RGB(255,0,0));		 
			SetPixel (CurView->hDC,Point.x+1,Point.y-1,RGB(255,0,0));		 
			SetPixel (CurView->hDC,Point.x+1,Point.y+1,RGB(255,0,0));*/		 
					if (Flip)
						AZ += PY;
					DPoint = dnewpt (DPoint,AZ+HALFPI,theight*CurView->BaseUnitsPerPixel*0.7);
					Point = BasePtToWinPt (&DPoint); 
				    if (ipass) 
					{
						TextPoint[ichar] = Point;
						TextAZ[ichar] = AZ;
						TextChar[ichar] = ElevText[jchar];
					//	TextOutWithShadow (CurView->hDC,Point.x,Point.y,&ElevText[jchar],1,1,RGB(255,255,255));
					//	TextOut (CurView->hDC,Point.x,Point.y,&ElevText[jchar],1);
					}
				}
			}
			nTextChar = nChar;
			{	
				POINT	SavePoint;
						
				if (LightContour)
					hPen = CreatePen (PS_SOLID,IDNINT(LightContourWidth),LightContourColor);
				else
					hPen = CreatePen (PS_SOLID,IDNINT(DarkContourWidth),DarkContourColor);
				hOldPen = SelectObject (CurView->hDC,hPen); 
				SavePoint = pPoints[BegLine]; 
				pPoints[BegLine] = DPointToPoint(TxtPoints[ntxp-1]);
				ii=GetROP2(CurView->hDC);
				GWPolyline2 (CurView->hDC,&pPoints[BegLine],*pNumPoints-BegLine,0);//DarkContourSymbol);  
				pPoints[BegLine] = SavePoint;
				SelectObject (CurView->hDC,hOldPen);
				GSSiDeleteObject (&hPen);   
			}
	NextLine:
			{			
				if (LightContour)
					hPen = CreatePen (PS_SOLID,IDNINT(LightContourWidth),LightContourColor);
				else
					hPen = CreatePen (PS_SOLID,IDNINT(DarkContourWidth),DarkContourColor);
				hOldPen = SelectObject (CurView->hDC,hPen);
				pPoints[EndLine+1] = DPointToPoint (TxtPoints[0]);
				GWPolyline2 (CurView->hDC,pPoints,EndLine+2,0);//DarkContourSymbol);    //pPoints[6]
				SelectObject (CurView->hDC,hOldPen);
				GSSiDeleteObject (&hPen);   
			}
			GlobalUnlock (hContourLines[i]); 
			for (ichar = 0;ichar < nTextChar;ichar++)
			{
			   	LogFont.lfEscapement = IDNINT(3600-((LTWOPI(-TextAZ[ichar])/RADDEG)*10)); 
			   	if (!LogFont.lfEscapement)
			   		LogFont.lfEscapement=1; 
				LogFont.lfOrientation = LogFont.lfEscapement;
				hFont = CreateFontIndirect((LPLOGFONT)&LogFont); 
				SelectObject(CurView->hDC,hFont); 
				TextOutWithShadow (CurView->hDC,TextPoint[ichar].x,TextPoint[ichar].y,&TextChar[ichar],1,1,RGB(255,255,255));
				SelectObject(CurView->hDC,OldFont);
				GSSiDeleteObject(&hFont); 
			}
		}
		RestoreDC (CurView->hDC,-1);
	}	
	for (i=0;i<nContourLines;i++) 
		GSSiGlobFree(&hContourLines[i]);
    nContourLines = 0;
	return;
} 

void LinkContourLines (short Line1,short Line2,short Type2, short Type1)
{
	LPSHORT	pNumPoints1, pNumPoints2, pTempNumPoints;
	LPPOINT	pPoints1, pPoints2, pTempPoints;  
	LPLONG	pElev1, pElev2, pTempElev;
	USHORT	i,j,k;  
	HANDLE	hTemp;
	
	pNumPoints1 = (LPSHORT)GlobalLock (hContourLines[Line1]); 
	pElev1   = (LPLONG)(pNumPoints1+1);
	pPoints1 = (LPPOINT)(pElev1+1);
	pNumPoints2 = (LPSHORT)GlobalLock (hContourLines[Line2]); 
	pElev2   = (LPLONG)(pNumPoints2+1);
	pPoints2 = (LPPOINT)(pElev2+1);
	if (*pNumPoints1 + *pNumPoints2 > MAXPOINTSINCONTOUR)
	{
		GlobalUnlock (hContourLines[Line1]);
		GlobalUnlock (hContourLines[Line2]);
		return;
	}
	if (Type1 == 1 && Type2 == 1)
	{
		hTemp = GSSiGlobAlloc (0,GMEM_MOVEABLE,MAXPOINTSINCONTOUR*sizeof(POINT)+6);	
		pTempNumPoints = (LPSHORT)GlobalLock (hTemp);
		pTempElev = (LPLONG) (pTempNumPoints+1);
		*pTempElev = *pElev1;	
		pTempPoints = (LPPOINT) (pTempElev+1);
		k=0;	
		for (i=1,j=*pNumPoints1-1;i<*pNumPoints1;i++,j--)
			pTempPoints[k++] = pPoints1[j];  
		for (i=1;i<*pNumPoints2;i++)
			pTempPoints[k++] = pPoints2[i];  
		*pTempNumPoints = k;
		GlobalUnlock (hTemp); 
		GSSiGlobUlFree (&hContourLines[Line1]); 
		hContourLines[Line1] = hTemp;
		GSSiGlobUlFree (&hContourLines[Line2]);
		for (i=Line2;i<nContourLines-1;i++)
			hContourLines[i] = hContourLines[i+1];
	}
	else if (Type1 == 1 && Type2 == 2)
	{   
		k = *pNumPoints2-1;
		for (i=1;i<*pNumPoints1;i++)
			pPoints2[k++] = pPoints1[i];    //pPoints2[2]   pPoints1[4]
		*pNumPoints2 = k;
		GlobalUnlock (hContourLines[Line2]); 
		GSSiGlobUlFree (&hContourLines[Line1]);
		for (i=Line1;i<nContourLines-1;i++)
			hContourLines[i] = hContourLines[i+1];
	}
	else if (Type1 == 2 && Type2 == 1)
	{
		k = *pNumPoints1-1;
		for (i=1;i<*pNumPoints2;i++)
			pPoints1[k++] = pPoints2[i];  
		*pNumPoints1 = k;
		GlobalUnlock (hContourLines[Line1]); 
		GSSiGlobUlFree (&hContourLines[Line2]);
		for (i=Line2;i<nContourLines-1;i++)
			hContourLines[i] = hContourLines[i+1];
	}
	else if (Type1 == 2 && Type2 == 2)
	{
		k = *pNumPoints1-1;
		for (i=1,j=*pNumPoints2-2;i<*pNumPoints2;i++,j--)
			pPoints1[k++] = pPoints2[j];                    //pPoints1[k]     pPoints2[2]
		*pNumPoints1 =k ;
		GlobalUnlock (hContourLines[Line1]); 
		GSSiGlobUlFree (&hContourLines[Line2]);
		for (i=Line2;i<nContourLines-1;i++)
			hContourLines[i] = hContourLines[i+1];
	}
	nContourLines--;
	return;
} 

BOOL ProcessTINPoly (HPDPOINT TriPointsIn,HANDLE hElevBuffer,long nPnts)
{  
	BOOL	rtn;  
	LPFLOAT	pElev; 
	DPOINT	TriPoints[4]; 
	double	Z1,Z2,Z3;
	
	if (!hElevBuffer)
		return FALSE;  
	_fmemmove (TriPoints,TriPointsIn,3*sizeof(DPOINT));
	pElev = (LPFLOAT)GlobalLock (hElevBuffer); 
	Z1=pElev[0]*MFT;
	Z2=pElev[1]*MFT;
	Z3=pElev[2]*MFT;
	rtn = FindContourVectors (TriPoints,&Z1,&Z2,&Z3);
	GlobalUnlock (hElevBuffer);
	return ShowGridLines;
}

BOOL AddPointToContourPolygon (LPDOUBLE pZC,LPDPOINT Point1, LPDPOINT Point2)
																							#if ENABLETRACE
																							{GSSiEnterProg (1371);
																							#endif
{
	DPOINT	Points[2]; 
	POINT	Points16[2];
	short	idesc, ConnectedTo, HowConnected; 
	USHORT	i;
	LPSHORT	pNumPoints;
	LPLONG	pElev;
	LPPOINT	pPoints;  
	long	iElev=IDNINT(*pZC/DTMContourInterval);
	BOOL	rtn=FALSE;
	
	Points[0] = *Point1;
	Points[1] = *Point2;
	Points16[0] = BasePtToWinPt(Point1);
	Points16[1] = BasePtToWinPt(Point2);
	                                       
	if (Points16[0].x == Points16[1].x && Points16[0].y == Points16[1].y)
		goto Exit;
	if (!CurView->LabelAllContours[CurView->CurFile] && fmod (*pZC,DTMContourInterval*5))  
	{   
		HPEN	hPen = CreatePen (PS_SOLID,IDNINT(LightContourWidth),LightContourColor);
		HANDLE	SavehElevBuffer = hElevBuffer;
		
		hElevBuffer = 0;
		InitRecord (CurView->hDC); 
		hElevBuffer = SavehElevBuffer;
		hOldPen = SelectObject (CurView->hDC,hPen);
		GWPolylineD (CurView->hDC, Points, 2,0);//LightContourSymbol);  
		SelectObject (CurView->hDC,hOldPen);
		GSSiDeleteObject (&hPen);
	}
	else 
	{
		ConnectedTo = -1;
		for (i=0;i<nContourLines;i++)
		{   
			pNumPoints = (LPSHORT)GlobalLock (hContourLines[i]);  
			pElev   = (LPLONG)(pNumPoints+1);
			pPoints = (LPPOINT)(pElev+1);
			if (iElev == *pElev && *pNumPoints < 1020)
			{
				if (pPoints[0].x == Points16[0].x && pPoints[0].y == Points16[0].y)
				{
					_fmemmove (&pPoints[1],&pPoints[0],*pNumPoints*sizeof(POINT));
					pPoints[0] = Points16[1];
					(*pNumPoints)++;  
					GlobalUnlock (hContourLines[i]);
					if (ConnectedTo != -1)
					{
						LinkContourLines (ConnectedTo,i,1,HowConnected);
						rtn = TRUE;
						goto Exit;  
					}
					else 
					{
						HowConnected = 1;
						ConnectedTo = i;
						continue;
					} 
				}
				if (pPoints[0].x == Points16[1].x && pPoints[0].y == Points16[1].y)
				{
					_fmemmove (&pPoints[1],&pPoints[0],*pNumPoints*sizeof(POINT));   //pPoints[3]
					pPoints[0] = Points16[0];
					(*pNumPoints)++;  
					GlobalUnlock (hContourLines[i]); 
					if (ConnectedTo != -1)
					{
						LinkContourLines (ConnectedTo,i,1,HowConnected);
						rtn = TRUE;
						goto Exit;  
					}
					else 
					{
						HowConnected = 1;
						ConnectedTo = i;
						continue;
					} 
				}
				if (pPoints[(*pNumPoints)-1].x == Points16[0].x && pPoints[(*pNumPoints)-1].y == Points16[0].y)
				{
					pPoints[*pNumPoints] = Points16[1];
					(*pNumPoints)++;  
					GlobalUnlock (hContourLines[i]); 
					if (ConnectedTo != -1)
					{
						LinkContourLines (ConnectedTo,i,2,HowConnected);
						rtn = TRUE;
						goto Exit;  
					}
					else 
					{
						HowConnected = 2;
						ConnectedTo = i;
						continue;
					} 
				}
				if (pPoints[(*pNumPoints)-1].x == Points16[1].x && pPoints[(*pNumPoints)-1].y == Points16[1].y)
				{
					pPoints[*pNumPoints] = Points16[0];
					(*pNumPoints)++;  
					GlobalUnlock (hContourLines[i]); 
					if (ConnectedTo != -1)
					{
						LinkContourLines (ConnectedTo,i,2,HowConnected);
						rtn = TRUE;
						goto Exit;  
					}
					else 
					{
						HowConnected = 2;
						ConnectedTo = i;
						continue;
					} 
				}
			}
			GlobalUnlock (hContourLines[i]); 
		} 
		if (ConnectedTo == -1 && nContourLines < MAXCONTOURLINES)
		{
			hContourLines[nContourLines] = GSSiGlobAlloc (0,GMEM_MOVEABLE,MAXPOINTSINCONTOUR*sizeof(POINT)+6);		
			pNumPoints = (LPSHORT)GlobalLock (hContourLines[nContourLines]);
			*pNumPoints = 2; 
			pElev = (LPLONG)(pNumPoints+1);  
			*pElev = iElev;
			pPoints = (LPPOINT)(pElev+1);
			pPoints[0] = Points16[0];
			pPoints[1] = Points16[1];
			GlobalUnlock (hContourLines[nContourLines++]);
		} 
	}
	rtn = TRUE;
Exit:
{
																							#if ENABLETRACE
																							GSSiExitProg (1371);
																							#endif
	return rtn;
}
																							#if ENABLETRACE
																							}
																							#endif
}  

BOOL FindContourVectors (LPDPOINT TriPoints,LPDOUBLE Z1, LPDOUBLE Z2, LPDOUBLE Z3)
																							#if ENABLETRACE
																							{GSSiEnterProg (1370);
																							#endif
{
	double	Z[4], ZC, Dist, d, AZ, ElevDiff, pct;
	USHORT	i, iside, ii; 
	DPOINT	Point1, Point2;
	BOOL	rtn=FALSE;
	
	if (*Z1 == CurNullElv || *Z2 == CurNullElv || *Z3 == CurNullElv)
		goto Exit;
	Z[0] = *Z1;
	Z[1] = *Z2;
	Z[2] = *Z3;
	Z[3] = *Z1; 
	TriPoints[3] = TriPoints[0];      //TriPoints[1]   TriPoints[2]  
   	if (ShowGridLines) 
		for (i=0;i<3;i++)
		{
			char	str[64]; 
			POINT	point = BasePtToWinPt (&TriPoints[i]);
						
			sprintf (str,"%.2f",Z[i]);
						
			SetTextColor (CurView->hDC,0);
			DispText (CurView->hDC,FALSE,point.x,point.x, point.y,0, 2,1,
	  				  20,1,1,2, FALSE,0,str,0,FALSE,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0); 
		}
	for (iside=0;iside<2;iside++)
	{   
		if (Z[iside] < Z[iside+1])
		{   
			ElevDiff = Z[iside+1]-Z[iside];
			Dist = ldistppmacro (&TriPoints[iside],&TriPoints[iside+1]);  
			AZ = getazdmacro (&TriPoints[iside],&TriPoints[iside+1]);
			ZC = Z[iside] - fmod (Z[iside],DTMContourInterval) + DTMContourInterval;
			while (ZC < Z[iside+1])
			{ 
				pct = (ZC - Z[iside])/ElevDiff;
				d = pct * Dist;
				Point1 = dnewptmacro (TriPoints[iside],AZ,d); 
				if (FindOpContourPoint (iside,&ZC,&Point2,TriPoints,Z))
					AddPointToContourPolygon (&ZC,&Point1,&Point2);
				ZC += DTMContourInterval; 
			}
		}
		else if (Z[iside] > Z[iside+1])
		{
			ElevDiff = Z[iside+1]-Z[iside];
			Dist = ldistppmacro (&TriPoints[iside],&TriPoints[iside+1]);  
			AZ = getazdmacro (&TriPoints[iside],&TriPoints[iside+1]);
			ZC = Z[iside] - fmod (Z[iside],DTMContourInterval);
			while (ZC > Z[iside+1])
			{ 
				pct = (ZC - Z[iside])/ElevDiff;
				d = pct * Dist;
				Point1 = dnewptmacro (TriPoints[iside],AZ,d); 
				if (FindOpContourPoint (iside,&ZC,&Point2,TriPoints,Z))
					AddPointToContourPolygon (&ZC,&Point1,&Point2);
				ZC -= DTMContourInterval;
			}
		}
	}
	rtn = TRUE;  
Exit:
{
																							#if ENABLETRACE
																							GSSiExitProg (1370);
																							#endif
	return rtn;
}
																							#if ENABLETRACE
																							}
																							#endif
}  

BOOL ComputeSlopeParameters (LPDOUBLE pNullElv,LPDPOINT TriPoints,LPDOUBLE Z1, LPDOUBLE Z2, LPDOUBLE Z3, LPDOUBLE pSlopePCT, LPDOUBLE pSlopeAZ)
{   
	double	Z[3], MinZ, d, ed1, ed2, az, aztohi, pct, daz;
	USHORT	MidPt, HiPt, LoPt, i, ii; 
	DPOINT	newpt, IntPoint;
	
	if (*Z1 == *pNullElv || *Z2 == *pNullElv || *Z3 == *pNullElv)
		return FALSE;
	if (*Z1 == *Z2 && *Z2 == *Z3) 
	{
		*pSlopePCT = 0;
		*pSlopeAZ = 0;
		return FALSE;
	}  
	Z[0] = *Z1;
	Z[1] = *Z2;
	Z[2] = *Z3;
	MidPt = 0;
	if (*Z2 <= *Z1 && *Z1 <= *Z3)
	{
		MidPt = 0;
		LoPt  = 1;
		HiPt  = 2;
	}
	else if (*Z3 <= *Z1 && *Z1 <= *Z2)
	{
		MidPt = 0;
		LoPt  = 2;
		HiPt  = 1;
	}
	else if (*Z1 <= *Z2 && *Z2 <= *Z3)
	{
		MidPt = 1;
		LoPt  = 0;
		HiPt  = 2;
	}
	else if	(*Z3 <= *Z2 && *Z2 <= *Z1)
	{
		MidPt = 1;
		LoPt  = 2;
		HiPt  = 0;
	}
	else if (*Z1 <= *Z3 && *Z3 <= *Z2)
	{
		MidPt = 2;
		LoPt  = 0;
		HiPt  = 1;
	}
	else if (*Z2 <= *Z3 && *Z3 <= *Z1)
	{
		MidPt = 2;
		LoPt  = 1;
		HiPt  = 0;
	}
		
	ed1 = Z[HiPt] - Z[LoPt];
	if (!ed1)
		ii=1;
	ed2 = Z[MidPt] - Z[LoPt];
	pct = ed2/ed1;
	d = ldistppmacro (&TriPoints[LoPt],&TriPoints[HiPt]);
	d *= pct;
	aztohi = getazdmacro (&TriPoints[LoPt],&TriPoints[HiPt]);
	newpt = dnewptmacro (TriPoints[LoPt],aztohi, d);
	az = getazdmacro (&TriPoints[MidPt],&newpt); 
	daz = DeltaAZmacro (az,aztohi);
	if (daz > 0)
		*pSlopeAZ = LTWOPImacro (az + HALFPI);
	else
		*pSlopeAZ = LTWOPImacro (az - HALFPI);
	if (GetPerpendicularIntersect2 (&TriPoints[LoPt],&TriPoints[MidPt],&az,&IntPoint))
	{
		d = ldistppmacro (&TriPoints[LoPt],&IntPoint);
		if (d)
		{
			*pSlopePCT = (Z[MidPt] - Z[LoPt])/d;
			return TRUE; 
		}
	}
	if (GetPerpendicularIntersect2 (&TriPoints[HiPt],&TriPoints[MidPt],&az,&IntPoint))
	{
		d = ldistppmacro (&TriPoints[HiPt],&IntPoint);
		if (d)
			*pSlopePCT = (Z[HiPt] - Z[MidPt])/d; 
		return TRUE;
	}
	return FALSE;
}

BOOL DisplayDTMSegment (void)
{   
	LPLIDARCELL	pCell; 
	USHORT	ip, irow, icol,iTri, iTriPnt;  
	long	Refno;
	DPOINT	Point; 
	HPDOUBLE	pRenderNode[3];  
   	LPDTMINFO	pDTMInfo;  
   	double	SlopePCT, SlopeAZ;
   	double	ArrowLength=DTMRenderGridSpacing*fabs(DTMSlopeArrowFactor); 
   	double	TriMidXInc[4]={0.25*DTMRenderGridSpacing,0.5*DTMRenderGridSpacing,0.75*DTMRenderGridSpacing,0.5*DTMRenderGridSpacing};
   	double	TriMidYInc[4]={0.5*DTMRenderGridSpacing,0.75*DTMRenderGridSpacing,0.5*DTMRenderGridSpacing,0.25*DTMRenderGridSpacing};
	DPOINT	TriPoints[4]; 
	HPLONG	pSubCell;
	
   	if (!hDTM)
   		return FALSE; 
	InitRecord (CurView->hDC);
	HiPrecis = TRUE;
   	pDTMInfo = (LPDTMINFO)GlobalLock (hDTM);
	InGraphicsProcessor = TRUE;
	ShowValue (CurView->hDC,TRUE);
	switch (DTMRenderAs)
	{
		case DTM_RENDER_RAW_POINTS:
		{    
			switch (pDTMInfo->Type)
			{   
				case 0:
				case 1: 
				Refno = 0;
				pSubCell = (LPLONG)GlobalLock (DTMSubCellHandle);  
				ip = 0;
				for (irow=0;irow<32;irow++)
				{   
					Point = DTMSubCellPoint;
					Point.y += irow * pDTMInfo->GridSpace;
					for (icol=0;icol<32;icol++)
					{
						Refno++; 
						ip++; 
						Point.x += pDTMInfo->GridSpace; 
						if (pSubCell[ip] < LONG_MAX)
							DisplayDTMPoint (Refno,Point,pSubCell[ip],0,DTMPointSymbol,pDTMInfo->GridSpace/100,pDTMInfo);
					}
				}
				GlobalUnlock (DTMSubCellHandle);
				break;
				
				case 2:
				break;
				
				case 3:
				Refno =	DTMCellID*MAXLIDARPERREC;  
				if (DTMCellHandle)
				{
					pCell = (LPLIDARCELL)GlobalLock (DTMCellHandle);
					for (ip=0;ip<pCell->NumPoints;ip++)
					{   
						Refno++;  
						DisplayDTMPoint (Refno,pCell->XY[ip],pCell->Z[ip]*MFT,0,DTMPointSymbol,pDTMInfo->GridSpace/200,pDTMInfo);
					}
					GlobalUnlock (DTMCellHandle);
				}
				break;
			} 
		} 
		break;
		case DTM_RENDER_GRID_POINTS:
		{    
			Refno =	DTMRow * DTMNumPointsInRenderGridRow; 
			pRenderNode[0] = (HPDOUBLE)GlobalLock (hDTMRenderGridRow[0]);  
			pRenderNode[1] = (HPDOUBLE)GlobalLock (hDTMRenderGridRow[1]);  
			pRenderNode[2] = (HPDOUBLE)GlobalLock (hDTMRenderGridRow[2]);  
			for (irow = 0;irow < 3;irow++)
			{
				Point = DTMRenderRowBeginPoint[irow];
				for (ip=0;ip<DTMNumPointsInRenderGridRow;ip++)
				{   
					Refno++;
					if (pRenderNode[irow][ip] != pDTMInfo->NULLElv)
					{  
						DisplayDTMPoint (Refno,Point,pRenderNode[irow][ip],0,DTMPointSymbol,pDTMInfo->GridSpace/100,pDTMInfo);
						ShowValue (CurView->hDC,FALSE); 
					}
					Point.x += DTMRenderGridSpacing;
				}
			}
			GlobalUnlock (hDTMRenderGridRow[0]);
			GlobalUnlock (hDTMRenderGridRow[1]);
			GlobalUnlock (hDTMRenderGridRow[2]);
		} 
		break;
		case DTM_RENDER_SLOPE_VECTORS:
		case DTM_RENDER_SLOPE_POLYGONS:
		{    
			Refno =	DTMRow * DTMNumPointsInRenderGridRow; 
			pRenderNode[0] = (HPDOUBLE)GlobalLock (hDTMRenderGridRow[0]);  
			pRenderNode[1] = (HPDOUBLE)GlobalLock (hDTMRenderGridRow[1]);  
			pRenderNode[2] = (HPDOUBLE)GlobalLock (hDTMRenderGridRow[2]);  
			Point = DTMRenderRowBeginPoint[0];
			for (ip=0;ip<DTMNumPointsInRenderGridRow-1;ip++)
			{   
				Refno++;
				if (pRenderNode[1][ip] != pDTMInfo->NULLElv)
				{   
					short	nTri=0;
					double	TotSlopePCT=0, TotSlopeAZ=0;
					double	SlopeAZArray[4], SlopePCTArray[4];
					DPOINT TriMidPt, ArrowBP, ArrowEP; 
			    	double	length;

					TriPoints[2] = Point;
					TriPoints[2].x += DTMRenderGridSpacing/2;
					TriPoints[2].y += DTMRenderGridSpacing/2;
					for (iTri=0;iTri<4;iTri++)
					{   
						
						for (iTriPnt=0;iTriPnt<2;iTriPnt++)
						{
							TriPoints[iTriPnt] = Point;
							TriPoints[iTriPnt].x += DTMTriPointCol[iTri][iTriPnt] * DTMRenderGridSpacing;
							TriPoints[iTriPnt].y += DTMTriPointRow[iTri][iTriPnt] * DTMRenderGridSpacing/2;
						}
						if (ComputeSlopeParameters (&pDTMInfo->NULLElv,TriPoints,
													&pRenderNode[DTMTriPointRow[iTri][0]][DTMTriPointCol[iTri][0]+ip],
					                                &pRenderNode[DTMTriPointRow[iTri][1]][DTMTriPointCol[iTri][1]+ip],
					                                &pRenderNode[1][ip],
					                                &SlopePCTArray[iTri],&SlopeAZArray[iTri]))
					    {   
					    	nTri++;    
					    	TotSlopePCT += SlopePCTArray[iTri];    
					    	if (ShowTriSlopes)
					    	{
								TriMidPt.x = (TriPoints[0].x + TriPoints[1].x + TriPoints[2].x)/3;   
								TriMidPt.y = (TriPoints[0].y + TriPoints[1].y + TriPoints[2].y)/3;   
								length = ArrowLength * SlopePCTArray[iTri];
								DisplayDTMPoint (Refno++,TriMidPt,SlopePCTArray[iTri],LTWOPI(SlopeAZArray[iTri]-HALFPI+SlopeCorrection),DTMSlopeArrowSymbol,length,0);
					    	
					    	}
					    } 
				    	if (ShowGridLines)
				    	{ 
				    		GWPolylineD (CurView->hDC, TriPoints, 3,GridLineDesc);
				    	}
				    	/*if (DTMRenderAs == DTM_RENDER_SLOPE_POLYGONS)
				    	{
							DisplayDTMArea (Refno++,TriPoints,3,SlopePCTArray[iTri],LTWOPI(SlopeAZArray[iTri]-HALFPI+SlopeCorrection),DTMSlopeAreaSymbol,0);
				    	} */
					}
				    if (DTMRenderAs == DTM_RENDER_SLOPE_VECTORS && nTri == 4 && TotSlopePCT && !ShowTriSlopes)
				    {   
						SlopeAZ = WeightedAZ (SlopeAZArray,SlopePCTArray,4);
					    SlopePCT = TotSlopePCT/4;		
						TriMidPt = TriPoints[2];   
						length = ArrowLength * SlopePCT;
						DisplayDTMPoint (Refno++,TriMidPt,SlopePCT,LTWOPI(SlopeAZ-HALFPI+DSIGN(SlopeCorrection,DTMSlopeArrowFactor)),DTMSlopeArrowSymbol,length,0);
				    }
				    else if (DTMRenderAs == DTM_RENDER_SLOPE_POLYGONS && nTri == 4 && !ShowTriSlopes)
				    {   
				    	DPOINT	RectPoints[4];
				    	
				    	RectPoints[0] = RectPoints[1] = Point;
				    	RectPoints[1].y += DTMRenderGridSpacing; 
				    	RectPoints[2] = RectPoints[1];
				    	RectPoints[2].x += DTMRenderGridSpacing;
				    	RectPoints[3] = RectPoints[2];
				    	RectPoints[3].y -= DTMRenderGridSpacing; 
						SlopeAZ = WeightedAZ (SlopeAZArray,SlopePCTArray,4);
					    SlopePCT = TotSlopePCT/4;		
						DisplayDTMArea (Refno++,RectPoints,4,SlopePCT,LTWOPI(SlopeAZ-HALFPI+SlopeCorrection),DTMSlopeAreaSymbol,0);
				    }
				}
				Point.x += DTMRenderGridSpacing;
			}
			GlobalUnlock (hDTMRenderGridRow[0]);
			GlobalUnlock (hDTMRenderGridRow[1]);
			GlobalUnlock (hDTMRenderGridRow[2]);
		}
		break; 
		case DTM_RENDER_CONTOURS:
		{    
			Refno =	DTMRow * DTMNumPointsInRenderGridRow; 
			pRenderNode[0] = (HPDOUBLE)GlobalLock (hDTMRenderGridRow[0]);  
			pRenderNode[1] = (HPDOUBLE)GlobalLock (hDTMRenderGridRow[1]);  
			pRenderNode[2] = (HPDOUBLE)GlobalLock (hDTMRenderGridRow[2]);  
			Point = DTMRenderRowBeginPoint[0];
			for (ip=0;ip<DTMNumPointsInRenderGridRow-1;ip++)
			{   
				Refno++;
				if (pRenderNode[1][ip] != pDTMInfo->NULLElv)
				{   
					short	nTri=0;
					
					TriPoints[2] = Point;
					TriPoints[2].x += DTMRenderGridSpacing/2;
					TriPoints[2].y += DTMRenderGridSpacing/2;
					for (iTri=0;iTri<4;iTri++)
					{   
						
						for (iTriPnt=0;iTriPnt<2;iTriPnt++)
						{
							TriPoints[iTriPnt] = Point;
							TriPoints[iTriPnt].x += DTMTriPointCol[iTri][iTriPnt] * DTMRenderGridSpacing;
							TriPoints[iTriPnt].y += DTMTriPointRow[iTri][iTriPnt] * DTMRenderGridSpacing/2;
						}
						FindContourVectors (TriPoints,
											&pRenderNode[DTMTriPointRow[iTri][0]][DTMTriPointCol[iTri][0]+ip],
					                        &pRenderNode[DTMTriPointRow[iTri][1]][DTMTriPointCol[iTri][1]+ip],
					                        &pRenderNode[1][ip]);
				    	if (ShowGridLines)
				    	{   
				    		GWPolylineD (CurView->hDC, TriPoints, 3,GridLineDesc);
				    	}
					}
				}
				Point.x += DTMRenderGridSpacing;
			}
			GlobalUnlock (hDTMRenderGridRow[0]);
			GlobalUnlock (hDTMRenderGridRow[1]);
			GlobalUnlock (hDTMRenderGridRow[2]);
		}
		break; 
	}
	ShowValue (CurView->hDC,FALSE); 
	InGraphicsProcessor = FALSE;
   	GlobalUnlock (hDTM); 
	return TRUE;
}

BOOL SetDTMRenderAs (int Layer)
{    
	char	str[128];
	
	DTMRenderAs = CurView->DTMRenderAs[Layer];
	if (DTMRenderAs == DTM_RENDER_CONTOURS)
	{
		DTMContourInterval = ContourIntervals[CurView->DTMContourIntOrSlopeFactor[Layer]];
	} 
	if (DTMRenderAs == DTM_RENDER_SLOPE_POLYGONS)
	{
		DTMSlopeAreaSymbol = GetDictSymbolNumber ("DTMSLOPEAREA");
	} 
	if (DTMRenderAs == DTM_RENDER_SLOPE_VECTORS)
	{
		DTMSlopeArrowSymbol = GetDictSymbolNumber ("DTMSLOPEPOINT"); 
		DTMSlopeArrowFactor = CurView->DTMContourIntOrSlopeFactor[Layer];
		if (!DTMSlopeArrowFactor)
			DTMSlopeArrowFactor = 1; 
	}  
	if (CurView->DarkContourColor[Layer])
	{   
		LightContourColor = ColorWOWidth (CurView->LayerColor[Layer]);
		LightContourWidth = GetWValue (CurView->LayerColor[Layer])/2.0;  
		DarkContourColor = ColorWOWidth (CurView->DarkContourColor[Layer]);
		DarkContourWidth = GetWValue (CurView->DarkContourColor[Layer])/2.0;  
		ContourTextColor = ColorWOWidth (CurView->ContourTextColor[Layer]);
		ContourTextSize = GetWValue (CurView->ContourTextColor[Layer]);  
	} 
	else
	{   
		LightContourColor = 0;
		LightContourWidth = 1;  
		DarkContourColor = 0;
		DarkContourWidth = 3;  
		ContourTextColor = 0;
		ContourTextSize = 4;  
	} 
	LightContourWidth *= DeviceToScreenFactor();
	DarkContourWidth *= DeviceToScreenFactor(); 
	DarkContourColor = ConvertColor (DarkContourColor,0);
	LightContourColor = ConvertColor (LightContourColor,0);
	ShowGridLines = GetGlobalBVal2 ("[%DTMShowGridLines]",FALSE);
	ShowTriSlopes = GetGlobalBVal2 ("[%DTMShowTriangleSlopes]",FALSE); 
	DTMPointSymbol = GetDictSymbolNumber ("SQUARE");
	GridLineDesc = GetDictSymbolNumber ("PEN1");  
	GetGlobalCVal ("[%CONTOURLINELIGHT]",str,"CONTOURL");
	LightContourSymbol = GetDictSymbolNumber (str);
	GetGlobalCVal ("[%CONTOURLINEDARK]",str,"CONTOURD");
	DarkContourSymbol = GetDictSymbolNumber (str);
	return TRUE;
}  

BOOL GetTriangleData (TRIANGLEID TriID,LPTRIANGLEDATA pCurrentTriData,LPTRIANGLEDATA pNewTriData)
{   
	double	Z[3], SlopePCT, SlopeAZ, AZ;
	DPOINT	TriPoints[3]; 
	USHORT	ip; 
	double	AZToCorner[3]={4.3906384259924,1.8925468811872,0.0}; 
	DPOINT	MidPoint; 
	BOOL	InBounds = FALSE;
	
	if (!BT_FIND (hComputedTriangles,(LPSTR)&TriID,BT_FIRST,BT_EQ,(LPSTR)pNewTriData))
	{
		if (pNewTriData->InBasin >= 0)
			return FALSE;
		else
			return TRUE;
	}
	pNewTriData->ID = TriID;
	GetTriCoords (TriID,TriPoints);
	for (ip = 0;ip < 3;ip++) 
	{
		if (PointInWBounds (&TriPoints[ip]))
			InBounds = TRUE;
		pNewTriData->Elev[ip] = Z[ip] = NGIELV_bci (TriPoints[ip],hDTMBasins,DTMRenderUnits);
	}  
	if (!InBounds)
		return FALSE;
	if (!ComputeSlopeParameters (&BasinsNullElv,TriPoints,&Z[0], &Z[1], &Z[2], &SlopePCT,&SlopeAZ))
		return FALSE;
	pNewTriData->SlopePCT = SlopePCT;
	pNewTriData->SlopeAZ = LTWOPI (PY + SlopeAZ);
//	MidPoint.x = (TriPoints[0].x + TriPoints[1].x + TriPoints[2].x)/3;
//	MidPoint.y = (TriPoints[0].y + TriPoints[1].y + TriPoints[2].y)/3;
//	for (ip = 0;ip < 3; ip++)
//		AZToCorner[ip] = getazd (&MidPoint,&TriPoints[ip]);  
	AZ = LTWOPI (PY + SlopeAZ + TriID.TriNum * HALFPI); 
	if (SlopePCT <= FlatSlope)
		pNewTriData->SlopesToward = -1;
	else if (AZ >= AZToCorner[0])
		pNewTriData->SlopesToward = 2;
	else if (AZ >= AZToCorner[1])
		pNewTriData->SlopesToward = 0; 
	else
		pNewTriData->SlopesToward = 1; 
	pNewTriData->InBasin = -1;
	BT_PUT (hComputedTriangles,(LPSTR)&TriID,(LPSTR)pNewTriData);
	return TRUE;
}

BOOL GetTriCoords (TRIANGLEID TriID,LPDPOINT TriPoints)
{    
	DPOINT	GridBasePoint;  
	USHORT	ip;
	
	GridBasePoint.x = GridOriginPoint.x + TriID.Over * DTMRenderGridSpacing;
	GridBasePoint.y = GridOriginPoint.y + TriID.Up * DTMRenderGridSpacing; 
	for (ip= 0; ip < 3; ip++)
	{
		TriPoints[ip].x = GridBasePoint.x + TriPointXOffset[TriID.TriNum][ip] * DTMRenderGridSpacing/2;
		TriPoints[ip].y = GridBasePoint.y + TriPointYOffset[TriID.TriNum][ip] * DTMRenderGridSpacing/2;  
	}
	return TRUE;
}

TRIANGLEID GetAdjoiningTriangleID (TRIANGLEID FromTriID,short iside,LPSHORT pConnectsToSide)
{
	TRIANGLEID	TriID;

	TriID = FromTriID;
	TriID.Over += TriConnectingSideOverInc[FromTriID.TriNum][iside];
	TriID.Up += TriConnectingSideUpInc[FromTriID.TriNum][iside];
	TriID.TriNum = TriConnectingSideTriNum[FromTriID.TriNum][iside];
	*pConnectsToSide = TriConnectingSide[FromTriID.TriNum][iside];
	return TriID;
} 

TRIANGLEID GetCornerTriangleID (TRIANGLEID FromTriID,short ipoint,LPSHORT pConnectsToPoint)
{
	TRIANGLEID	TriID; 
	
	TriID = FromTriID;
	TriID.Over += TriCornerTriOverInc[FromTriID.TriNum][ipoint];
	TriID.Up += TriCornerTriUpInc[FromTriID.TriNum][ipoint];
	TriID.TriNum = TriCornerTriTriNum[FromTriID.TriNum][ipoint];
	*pConnectsToPoint = ipoint;
	return TriID;
}

BOOL TriangleSlopesTowardPoint (TRIANGLEID TriID,short ipoint,LPTRIANGLEDATA pNewTriData)
{   
	TRIANGLEDATA	SideTriData;  
	TRIANGLEID 		SideTriID;
	short	iside=ipoint, ConnectsToSide; 
	
	if (!GetTriangleData (TriID,0,pNewTriData))
		return FALSE;
	if (pNewTriData->SlopePCT < FlatSlope)
		return TRUE;
	if (pNewTriData->SlopesToward == iside)
	{
		SideTriID = GetAdjoiningTriangleID (TriID,iside,&ConnectsToSide);
		if (GetTriangleData (SideTriID,pNewTriData,&SideTriData))
		{
			if (SideTriData.SlopesToward == ConnectsToSide)
				return TRUE;
		}
	}
	if (iside)
		iside--;
	else
		iside = 2;
	if (pNewTriData->SlopesToward == iside)
	{
		SideTriID = GetAdjoiningTriangleID (TriID,iside,&ConnectsToSide);
		if (GetTriangleData (SideTriID,pNewTriData,&SideTriData))
		{
			if (SideTriData.SlopesToward == ConnectsToSide)
				return TRUE;
		}
	}
	return FALSE;
}


BOOL FindBasinsAroundPoints (LPSTR DTMName,short nPoints, LPDPOINT StartPoints)
{ 
typedef struct
	{   
		short	StartPointID;
		TRIANGLEID	TriID;
		short	Side;
	} EDGETRIANGLEHEADER;

typedef struct
     {  
     	short	Level;
     	short	StartPointID;
     	TRIANGLEID	TriID; 
     } NEXTTRIANGLEHEADER;
typedef NEXTTRIANGLEHEADER    FAR *LPNEXTTRIANGLEHEADER;    
    
    NEXTTRIANGLEHEADER	NextTriHead, NewTriHead;
	short	NextTriData=0;
	HANDLE	hNextTriangle;
	HANDLE	hEdgeTriangles;
	DPOINT	BasePoint, TriPoints[3], ThisTriPoints[3], TriMidPt;
	short	PointID=0; 
	TRIANGLEID	TriID;
	static	TRIANGLEID	debugtriid={1,-1,0};
	USHORT	iside;  
	short	ConnectsToSide; 
	TRIANGLEDATA	CurrentTriData, SideTriData, ThisTriData;  
	EDGETRIANGLEHEADER	EdgeTriHead;   
	short	EdgeTriData;
	BTVARDESC	BTVar[5]; 
	char	TempFile[128]; 
	long	Refno=0;    
	int	Brushes[4]={BLACK_BRUSH,DKGRAY_BRUSH,GRAY_BRUSH,LTGRAY_BRUSH};  
	short	ii; 
	double	AZToCorner;
	
	FlatSlope = GetGlobalDVal2("[%DTMFlatSlope]",0.02); 
	hDTMBasins = DTMOpen (DTMName, BasinsNullElv,BT_READ,0);
	if (!hDTMBasins)
		return FALSE; 
//StartPoints[0].x = 158520.7;
//StartPoints[0].y = 54844.6;
	GridOriginPoint.x = StartPoints[0].x - fmod (StartPoints[0].x,DTMRenderGridSpacing);
	GridOriginPoint.y = StartPoints[0].y - fmod (StartPoints[0].y,DTMRenderGridSpacing);

	GSSiGetTempFileName (0,"gmb",0,(LPSTR)TempFile);
	BTVar[0].BT_VARTYP=BT_INTEGER;
	BTVar[0].BT_VARLEN=2;
	BTVar[0].BT_VAROFF=0;
	BTVar[1].BT_VARTYP=BT_INTEGER;
	BTVar[1].BT_VARLEN=2;
	BTVar[1].BT_VAROFF=2;
	BTVar[2].BT_VARTYP=BT_INTEGER;
	BTVar[2].BT_VARLEN=2;
	BTVar[2].BT_VAROFF=4;
	BTVar[3].BT_VARTYP=BT_INTEGER;
	BTVar[3].BT_VARLEN=2;
	BTVar[3].BT_VAROFF=6;
	BTVar[4].BT_VARTYP=BT_INTEGER;
	BTVar[4].BT_VARLEN=2;
	BTVar[4].BT_VAROFF=8;
	BT_CREATE (TempFile, sizeof(NextTriData), FALSE, 5, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
	hNextTriangle = BT_OPEN (TempFile, 0, BT_WRITE, 0);
	
	GSSiGetTempFileName (0,"gmb",0,(LPSTR)TempFile);
	BTVar[0].BT_VARTYP=BT_INTEGER;
	BTVar[0].BT_VARLEN=2;
	BTVar[0].BT_VAROFF=0;
	BTVar[1].BT_VARTYP=BT_INTEGER;
	BTVar[1].BT_VARLEN=2;
	BTVar[1].BT_VAROFF=2;
	BTVar[2].BT_VARTYP=BT_INTEGER;
	BTVar[2].BT_VARLEN=2;
	BTVar[2].BT_VAROFF=4;
	BTVar[3].BT_VARTYP=BT_INTEGER;
	BTVar[3].BT_VARLEN=2;
	BTVar[3].BT_VAROFF=6;
	BTVar[4].BT_VARTYP=BT_INTEGER;
	BTVar[4].BT_VARLEN=2;
	BTVar[4].BT_VAROFF=8;
	BT_CREATE (TempFile, sizeof(EdgeTriData), FALSE, 5, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
	hEdgeTriangles = BT_OPEN (TempFile, 0, BT_WRITE, 0);
	
	GSSiGetTempFileName (0,"gmb",0,(LPSTR)TempFile);
	BTVar[0].BT_VARTYP=BT_INTEGER;
	BTVar[0].BT_VARLEN=2;
	BTVar[0].BT_VAROFF=0;
	BTVar[1].BT_VARTYP=BT_INTEGER;
	BTVar[1].BT_VARLEN=2;
	BTVar[1].BT_VAROFF=2;
	BTVar[2].BT_VARTYP=BT_INTEGER;
	BTVar[2].BT_VARLEN=2;
	BTVar[2].BT_VAROFF=4;
	BT_CREATE (TempFile, sizeof(TRIANGLEDATA), FALSE, 3, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
	hComputedTriangles = BT_OPEN (TempFile, 0, BT_WRITE, 0);
	
//Get start triangle  
	NextTriHead.TriID.Over = 0;
	NextTriHead.TriID.Up = 0;
	NextTriHead.TriID.TriNum = 0;
	GSSiDeleteObject(&CurView->hRgn);
	CurView->hRgn = CreateVPRgn(FALSE,FALSE);
	SelectClipRgn (CurView->hDC,CurView->hRgn);
	GSSiDeleteObject(&CurView->hRgn); 
    
    for (PointID = 0;PointID < nPoints;PointID++)
    {    
    	if (PointID)
    	{   
    		DPOINT	GridPoint;
    		
			GridPoint.x = StartPoints[PointID].x - fmod (StartPoints[PointID].x,DTMRenderGridSpacing);
			GridPoint.y = StartPoints[PointID].y - fmod (StartPoints[PointID].y,DTMRenderGridSpacing); 
			NextTriHead.TriID.Over = (GridPoint.x - GridOriginPoint.x)/DTMRenderGridSpacing;
			NextTriHead.TriID.Up = (GridPoint.y - GridOriginPoint.y)/DTMRenderGridSpacing;
			NextTriHead.TriID.TriNum = 0;
    	}
	    for (NextTriHead.TriID.TriNum=0;NextTriHead.TriID.TriNum<4;NextTriHead.TriID.TriNum++)
	    {
	    	GetTriCoords (NextTriHead.TriID,ThisTriPoints);
	    	if (POINT_IN_AREAD (StartPoints[PointID], 3,ThisTriPoints,1,0,0,0))
	    		goto Start;
	    }
		NextTriHead.TriID.TriNum = 3;
	Start:
	   	SelectObject (CurView->hDC,GetStockObject(Brushes[PointID%4])); 
	   	Refno++;
		GWPolygonD (CurView->hDC, ThisTriPoints,3, 1, 0,0,FALSE,TRUE,0); 
		NextTriHead.Level = 0;
		NextTriHead.StartPointID = PointID;
		GetTriangleData (NextTriHead.TriID,&CurrentTriData,&ThisTriData);
		ThisTriData.InBasin = PointID;
		BT_PUT (hComputedTriangles,(LPSTR)&NextTriHead.TriID,(LPSTR)&ThisTriData);
	   	BT_PUT (hNextTriangle,(LPSTR)&NextTriHead,(LPSTR)&NextTriData);  
   	}
   	
   	while (!BT_FIND (hNextTriangle,(LPSTR)&NextTriHead,BT_FIRST,BT_ANY,(LPSTR)&NextTriData))
   	{   
   		if (NextTriHead.TriID.Over == debugtriid.Over &&
   			NextTriHead.TriID.Up == debugtriid.Up && 
   			NextTriHead.TriID.TriNum == debugtriid.TriNum)
   			ii=1;
   		BT_DELETE (hNextTriangle,(LPSTR)&NextTriHead,(LPSTR)&NextTriData,FALSE); 
   		PointID = NextTriHead.StartPointID;
		GetTriangleData (NextTriHead.TriID,&CurrentTriData,&ThisTriData);
    	GetTriCoords (NextTriHead.TriID,ThisTriPoints);
		TriMidPt.x = (ThisTriPoints[0].x + ThisTriPoints[1].x + ThisTriPoints[2].x)/3;   
		TriMidPt.y = (ThisTriPoints[0].y + ThisTriPoints[1].y + ThisTriPoints[2].y)/3;   
   		for (iside=0;iside<3;iside++)
   		{    
//   			if (ThisTriData.SlopesToward != iside)
   			{
	   			TriID = GetAdjoiningTriangleID (NextTriHead.TriID,iside,&ConnectsToSide);
	   			if (GetTriangleData (TriID,&CurrentTriData,&SideTriData))
	   			{
		   			if (SideTriData.SlopesToward == ConnectsToSide || SideTriData.SlopesToward < 0)
		   			{  
		   				SideTriData.InBasin = PointID;
						BT_PUT (hComputedTriangles,(LPSTR)&TriID,(LPSTR)&SideTriData);
		   				NewTriHead.Level = NextTriHead.Level+1;
		   				NewTriHead.StartPointID = NextTriHead.StartPointID;
		   				NewTriHead.TriID = TriID;
					   	BT_PUT (hNextTriangle,(LPSTR)&NewTriHead,(LPSTR)&NextTriData);  
				    	GetTriCoords (TriID,TriPoints); 
				    	SelectObject (CurView->hDC,GetStockObject(Brushes[PointID%4]));
						GWPolygonD (CurView->hDC, TriPoints,3, 1, 0,0,FALSE,TRUE,0); 
						Refno++;
		   			}
		   			else
		   			{   
		   				EdgeTriHead.StartPointID = PointID;
		   				EdgeTriHead.TriID = TriID;
		   				EdgeTriHead.Side = iside;
					   	BT_PUT (hEdgeTriangles,(LPSTR)&EdgeTriHead,(LPSTR)&EdgeTriData);  
		   			}
		   		} 
		   	}
		   	AZToCorner = getazd (&TriMidPt,&ThisTriPoints[iside]);
		   	if (ThisTriData.SlopePCT <= FlatSlope || fabs(DeltaAZ (ThisTriData.SlopeAZ,AZToCorner)) > HALFPI)
		   	{
	   			TriID = GetCornerTriangleID (NextTriHead.TriID,iside,&ConnectsToSide);
	   			if (TriangleSlopesTowardPoint (TriID,ConnectsToSide,&SideTriData))
	   			{
	   				SideTriData.InBasin = PointID;
					BT_PUT (hComputedTriangles,(LPSTR)&TriID,(LPSTR)&SideTriData);
	   				NewTriHead.Level = NextTriHead.Level+1;
	   				NewTriHead.StartPointID = NextTriHead.StartPointID;
	   				NewTriHead.TriID = TriID;
				   	BT_PUT (hNextTriangle,(LPSTR)&NewTriHead,(LPSTR)&NextTriData);  
			    	GetTriCoords (TriID,TriPoints); 
			    	SelectObject (CurView->hDC,GetStockObject(Brushes[PointID%4]));
					GWPolygonD (CurView->hDC, TriPoints,3, 1, 0,0,FALSE,TRUE,0); 
					Refno++;
		   		}
		   	}
   		}
   	}
	DTMClose (&hDTMBasins);
	BT_CLOSEANDDELETE (&hNextTriangle);   
	BT_CLOSEANDDELETE (&hEdgeTriangles);   
	BT_CLOSEANDDELETE (&hComputedTriangles);   
    return TRUE;
} 

void SetDTMSubSettings (HWND hWndDlg)
{
	 switch(SendDlgItemMessage(hWndDlg,IDC_RENDERAS,CB_GETCURSEL,0,0))
	 {
	 	case 0:
	 	case 1:
	 	case 3:
			 ShowWindow (GetDlgItem(hWndDlg,IDC_SUBSETTINGTITLE1),SW_HIDE);
			 ShowWindow (GetDlgItem(hWndDlg,IDC_SUBSETTING1),SW_HIDE);
			 ShowWindow (GetDlgItem(hWndDlg,IDC_SUBSETTINGTITLE2),SW_HIDE);
			 ShowWindow (GetDlgItem(hWndDlg,IDC_SUBSETTING2),SW_HIDE);
			 break;
		case 2:
			 ShowWindow (GetDlgItem(hWndDlg,IDC_SUBSETTINGTITLE1),SW_SHOW);
			 ShowWindow (GetDlgItem(hWndDlg,IDC_SUBSETTING1),SW_SHOW);
			 ShowWindow (GetDlgItem(hWndDlg,IDC_SUBSETTINGTITLE2),SW_SHOW);
			 ShowWindow (GetDlgItem(hWndDlg,IDC_SUBSETTING2),SW_SHOW);
			 SetDlgItemText (hWndDlg,IDC_SUBSETTINGTITLE1,"Direction");
			 SetDlgItemText (hWndDlg,IDC_SUBSETTINGTITLE2,"Size Factor");
	         SendDlgItemMessage (hWndDlg,IDC_SUBSETTING1,CB_RESETCONTENT,0,0);
			 SendDlgItemMessage (hWndDlg,IDC_SUBSETTING1,CB_ADDSTRING,0,(LPARAM)(LPSTR)"Up slope");
			 SendDlgItemMessage (hWndDlg,IDC_SUBSETTING1,CB_ADDSTRING,0,(LPARAM)(LPSTR)"Down slope");
	         SendDlgItemMessage (hWndDlg,IDC_SUBSETTING2,CB_RESETCONTENT,0,0);
			 SendDlgItemMessage (hWndDlg,IDC_SUBSETTING2,CB_ADDSTRING,0,(LPARAM)(LPSTR)"1");
			 SendDlgItemMessage (hWndDlg,IDC_SUBSETTING2,CB_ADDSTRING,0,(LPARAM)(LPSTR)"2");
			 SendDlgItemMessage (hWndDlg,IDC_SUBSETTING2,CB_ADDSTRING,0,(LPARAM)(LPSTR)"3");
			 SendDlgItemMessage (hWndDlg,IDC_SUBSETTING2,CB_ADDSTRING,0,(LPARAM)(LPSTR)"4");
			 SendDlgItemMessage (hWndDlg,IDC_SUBSETTING2,CB_ADDSTRING,0,(LPARAM)(LPSTR)"5");
			 SendDlgItemMessage (hWndDlg,IDC_SUBSETTING2,CB_ADDSTRING,0,(LPARAM)(LPSTR)"6");
			 break;
		case 4:
			 ShowWindow (GetDlgItem(hWndDlg,IDC_SUBSETTINGTITLE1),SW_SHOW);
			 ShowWindow (GetDlgItem(hWndDlg,IDC_SUBSETTING1),SW_SHOW);
			 ShowWindow (GetDlgItem(hWndDlg,IDC_SUBSETTINGTITLE2),SW_HIDE);
			 ShowWindow (GetDlgItem(hWndDlg,IDC_SUBSETTING2),SW_HIDE);
			 SetDlgItemText (hWndDlg,IDC_SUBSETTINGTITLE1,"Contour Interval");
	         SendDlgItemMessage (hWndDlg,IDC_SUBSETTING1,CB_RESETCONTENT,0,0);
			 SendDlgItemMessage (hWndDlg,IDC_SUBSETTING1,CB_ADDSTRING,0,(LPARAM)(LPSTR)"1 Foot");
			 SendDlgItemMessage (hWndDlg,IDC_SUBSETTING1,CB_ADDSTRING,0,(LPARAM)(LPSTR)"2 Foot");
			 SendDlgItemMessage (hWndDlg,IDC_SUBSETTING1,CB_ADDSTRING,0,(LPARAM)(LPSTR)"5 Foot");
			 SendDlgItemMessage (hWndDlg,IDC_SUBSETTING1,CB_ADDSTRING,0,(LPARAM)(LPSTR)"10 Foot");
			 SendDlgItemMessage (hWndDlg,IDC_SUBSETTING1,CB_ADDSTRING,0,(LPARAM)(LPSTR)"20 Foot");
			 SendDlgItemMessage (hWndDlg,IDC_SUBSETTING1,CB_ADDSTRING,0,(LPARAM)(LPSTR)"50 Foot");
			 SendDlgItemMessage (hWndDlg,IDC_SUBSETTING1,CB_ADDSTRING,0,(LPARAM)(LPSTR)"100 Foot");
			 break;
	 }
	 SendDlgItemMessage (hWndDlg,IDC_SUBSETTING1,CB_SETCURSEL,0,0);
	 SendDlgItemMessage (hWndDlg,IDC_SUBSETTING2,CB_SETCURSEL,0,0);
	 return;
}

BOOL SurfToFile (LPSTR DTMFile,LPMNMXCORD pBounds,double GridSpace,LPSTR OutFile,LPDOUBLE pMinElev,LPDOUBLE pMaxElev)
{
	BOOL	rtn=FALSE;
	double	minElev = DBL_MAX, maxElev = -DBL_MAX;
	HANDLE hSurf = DTMOpen (DTMFile,NULL_ELV,BT_READ,0);
	int		i;

	if (hSurf)
	{
		HFILE	Fid = GSSiOpenFile (OutFile,0,OF_CREATE);

		if (Fid != HFILE_ERROR)
		{
			MNMXCORD	NewBounds;  
			double	minx, miny, maxx, maxy, Elv;
			DWORD	nRow, nCol, Col, Row, NumItems, CurItem=0;
			DPOINT	BP, BeginPoint,Point;
			HANDLE	hMem = GSSiGlobAlloc (0,GMEM_MOVEABLE,USHRT_MAX);
			LPSTR	OutRec = GlobalLock (hMem);
			
			ConvertRectCoord (&NewBounds, pBounds, 1,3); 
			minx = NewBounds.xmn - fmod (NewBounds.xmn,GridSpace);
			if (minx < NewBounds.xmn)
				minx += GridSpace;
			miny = NewBounds.ymn - fmod (NewBounds.ymn,GridSpace);
			if (miny < NewBounds.ymn)
				miny += GridSpace;
			maxx = NewBounds.xmx - fmod (NewBounds.xmx,GridSpace);
			maxy = NewBounds.ymx - fmod (NewBounds.ymx,GridSpace);
			nCol = IDNINT((maxx - minx) / GridSpace) + 1; 
			i = nCol % 4;
			if (i)
			{
				nCol += (4 - i);
				GridSpace = (nCol * GridSpace) / (nCol - (4 - i));
			}
			nRow = IDNINT((maxy - miny) / GridSpace) + 1;
			NumItems = nRow;
			BeginPoint.x = minx;
			BeginPoint.y = miny;
			BP = BeginPoint;
			CreateStatusWind (hWndMain,1,"Exporting Elevation Data");
			Row = nRow;
			while ((SetContinueProcessing(StatusWindowUpdate(0, 0, NumItems, ++CurItem))) && Row--)
			{
    			BP.x = BeginPoint.x;
    			Col = nCol;
				*OutRec = 0;
    			while (Col--)
    			{
        			Point = BP;
        			ConvertCoord(&Point,3,1); 
        			Elv = NGIELV_bci (Point,hSurf,0);
        			if (Elv != NULL_ELV)
					{
						minElev = min (minElev,Elv);
						maxElev = max (maxElev,Elv);
            			sprintf (strchr(OutRec,0),"%10.3f",Elv); 
					}
					else
						strcat (OutRec,"          ");
        			BP.x += GridSpace;
				}
				BP.y += GridSpace;
   				fputstring (OutRec,Fid); 
		 //       PctBox (GetDlgItem(hWndDlg,IDC_STATUS), NumItems, CurItem+=nCol,0);
			} 
			DestroyStatusWindow(0);  
			if (ContinueProcessing)
			{
				if (pMinElev)
				{
					*pMinElev = minElev;
					*pMaxElev = maxElev;
					{
						int		irow, icol;
						HDIB32 hDib = FreeImage_Allocate(nCol, nRow, 24,0,0,0);
						RGBTRIPLE	*p24Bit;
						double	f = (maxElev - minElev) / 255;

						GSSillseek (Fid,0,0);
						for (irow = 0;irow < nRow;irow++)
						{
							LPSTR pos = OutRec;
							int	  ival;
							fgetstring (OutRec,USHRT_MAX-2,Fid);
							p24Bit = (RGBTRIPLE	*)FreeImage_GetScanLine (hDib,irow);
							for (icol = 0;icol < nCol;icol++,p24Bit++,pos+=10)
							{
								int ival = IDNINT((max(minElev,dread (pos,10)) - minElev) / f);

								p24Bit->rgbtBlue = p24Bit->rgbtGreen = p24Bit->rgbtRed = ival;
							}
						}
						rtn = SaveDIB32 (hDib,"c:\\temp\\dtmbmp.bmp",0,-1);
						GMDestroyDIB32 (hDib); 
					}
				}
				rtn = TRUE;
				GSSiClose2 (&Fid);   
			}
			else
			{
				GSSiClose2 (&Fid);   
				SetContinueProcessing ( TRUE);
				GSSiRemove (OutFile);
			}
			GSSiGlobUlFree (&hMem);
		}
		DTMClose (&hSurf);
	}
	return rtn;
}
int classifyLAZFile(char * file, char * outFile)
{
	int rtn = 0;
	int totals[19] = { 0 };
	HFILE fid;
	char txt[256];
	char description[19][40] = { "Never classified", "Unassigned", "Ground", "Low Vegetation", "Medium Vegetation", "High Vegetation", "Building", "Low Point", "Reserved", "Water", "Rail", "Road Surface", "Reserved","Wire - Guard(Shield)","Wire - Conductor(Phase)","Transmission Tower","Wire - Structure Connector(Insulator)","Bridge Deck","High Noise" };
		

			
	laszip_point_struct* point;
	static __int64 use = 1;

	if (laszip_load_dll() != 1)
	{
		laszip_POINTER laszip_reader;
		if (!laszip_create(&laszip_reader))
		{
			laszip_BOOL is_compressed = 0;
			if (!laszip_open_reader(laszip_reader, file, &is_compressed))
			{
				laszip_header_struct* header;
				laszip_get_header_pointer(laszip_reader, &header);
				laszip_get_point_pointer(laszip_reader, &point);
				while (!laszip_read_point(laszip_reader) && rtn < header->number_of_point_records)
				{
					rtn++;
					if (point->classification >= 0 && point->classification <= 18)
						totals[point->classification]++;
				}
				laszip_close_reader(laszip_reader);
				fid = GSSiOpenFile(outFile, 0, OF_CREATE);
				for (int i = 0; i < 19; i++)
				{
					sprintf(txt, "%i\t%i\t%s", i, totals[i], description[i]);
					fputstring(txt, fid);
				}
				GSSiClose2 (&fid);
			}
			laszip_destroy(laszip_reader);
		}
	}
	return rtn;
}

