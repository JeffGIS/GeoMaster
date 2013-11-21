#include "CONFIG.h"
#include "graphint.h"
#define WIN31
#include "commdlg.h"
#include "dlgs.h"
#include "cddemo.h"   
#include "dict.h"
#include "GW.h"   
#include "p_tol.h"  
#include "MCI.H"
#include "resource.h"
#include <dyndlg.h>
#include <math.h> 
#include <float.h>   
#include <mmsystem.h>    
#include <time.h>   
#include <direct.h>
#include "dibapi.h"    
#include "address.h"  
#include "winexec.h" 
#include <shellapi.h>  
#include <sys\types.h>
#include <sys\stat.h>   
#include <ctype.h> 

#define	INDCOD	31100 
extern	HWND	hWndMain;  
extern	BOOL	ContinueProcessing;

static	HANDLE	hOpenSurf[MAXOPENSURF]; 
static	long	NextDTMUse=LONG_MIN;    

double	PlaneElev=0;
 
HANDLE NEXPND (short INUM,LPSHORT INPUTI)
{
	UINT	I, NEXT=0, J, LAST; 
	short	NOFJ, ELV;
	long	INC;
	HANDLE	handle=GSSiGlobAlloc (GMEM_MOVEABLE,1024*sizeof(short));
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
    return handle;
ErrOut:
	GSSiGlobUlFree (&handle);
	return 0;
}  

void DTMOutputType1Run (short nRun,long CURDIF,LPSTR CompressedDTMData,LPSHORT pNextCmp)
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
	return;
}  

void DTMOutputType2Run (short nRun,long CURDIF,LPSHORT CompressedDTMData,LPSHORT pNextCmp)
{   
	short	ii;
	
	CompressedDTMData[(*pNextCmp)++] = 31100 + nRun;
	CompressedDTMData[(*pNextCmp)++] = CURDIF;
	return;
}  

void ExpandSubcell (LPLONG DTMData,SUBCELLINFO SUBCELLInfo,LPSTR CompressedData,LPLONG pBias)
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
			
			for (ExpLoc=0;ExpLoc<1024;ExpLoc++)
				if (CompressedDTMData[ExpLoc] < LONG_MAX)
					DTMData[ExpLoc] = CompressedDTMData[ExpLoc] + *pBias;
				else
					DTMData[ExpLoc] = CompressedDTMData[ExpLoc];
		}
	}
	return;
}

SUBCELLINFO CompressSubcell (LPLONG DTMData,LPSTR CompressedData,LPLONG	Bias)
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
		UINT	NextData=1, NextCmp=0;
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
		UINT	NextData=1, NextCmp=0;
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
	return SUBCELLInfo;
}
void LoadDTM (void)
{   
	OFSTRUCT	OFStruct;
	HFILE		Fid1, Fid2;
	char		FileName[132];     
	HANDLE		hRec=GSSiGlobAlloc (GMEM_MOVEABLE,8192);
	LPSTR		pRec=GlobalLock (hRec);
	short		ii, length;  
	UINT		i, n, pos;
	long		GeoSeg, Bias, nFiles=0, MaxFiles=50, nOutOfRange=0, nSubCells=0, nLess12=0;
	short		NumElv, SubCell, Indeterminate, MinElv, MaxElv;    
	BOOL		OutOfRange; 
	short		SubcellData[1024]; 
	long		DTMData[1024], DTMData2[1024];
	BTVARDESC BTVar[2], *pVars;
	int		NumFields, Reclen, len;
	long	Offset;
	long	TotFileLen;
	GWDHEADER GWDHead; 
	LPGWDHEADER	lpGWDHead;
	GWFLDINFO GWFldInfo;
	LPGWFLDINFO	lpGWFldInfo;
	HANDLE hBT, hDB=0;
	HFILE	FidData;
	int		ibeg;
	GWFLDINFO FldInfo;
	char	File[128]="[%DL]attribut\\ot1.dtm"; 
	LPSTR	lpDot; 
	HANDLE	hCell;
	LPSHORT	SubcellDat; 
	LPLONG	pBias;  
	static	long	debugsubcell=845;  
	long	TotLen, CurLoc, SubCellID;
	

	DTMKEY		DTMKey;
	SUBCELLINFO	SUBCELLInfo;
	LPSUBCELLINFO	pSUBCELLInfo;
	LPSTR		CompressedDTMData;
	
	 lpGWDHead = &GWDHead; 
    _fmemset (lpGWDHead,0,sizeof(GWDHEADER));

	 FidData = GSSiOpenFile (File,&OFStruct,OF_CREATE);
	 GWDHead.NumFields=0;
	 GWDHead.NumIndex=1;
	 GWDHead.Version=1;
	 GWDHead.NumIndexFields[0]=1;
	 GWDHead.IndexFields[0][0]=0;
	 _lwrite (FidData,&GWDHead,sizeof(GWDHEADER));
	ibeg = 0;

	FldInfo.Len = sizeof(DTMKEY);
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"DTMKEY");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"BIAS");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = sizeof(SUBCELLINFO);
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"SUBCELLINFO");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	
	FldInfo.Len = 4096;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"COMPRESSEDNODES");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;
	
	 GWDHead.Reclen=ibeg; 
	 GWDHead.TimeStamp = 0;
	 _llseek (FidData,0,0);
	 _lwrite (FidData,&GWDHead,sizeof(GWDHEADER));
 	 _llseek (FidData,0,2);
			     
	 BTVar[0].BT_VARLEN=4;
	 BTVar[0].BT_VARTYP=BT_INTEGER;
	 BTVar[0].BT_VAROFF=0;
	 lpDot = _fstrrchr (File,'.');
	 _fstrcpy (lpDot,".in1");	
	 BT_CREATE (File, 4, FALSE, 1, 1,BTVar,FALSE, 0, 0, FALSE);
 	 GSSiClose (FidData);
	 _fstrcpy (lpDot,".dtm");	

     hDB = OpenGWDatabase (File,BT_WRITE);
	lpGWDHead = (LPGWDHEADER)GlobalLock (hDB); 
	pSUBCELLInfo = (LPSUBCELLINFO)((LPSTR)&lpGWDHead->GWDData + (sizeof(DTMKEY)));
	pBias = (LPLONG)((LPSTR)&lpGWDHead->GWDData + (sizeof(DTMKEY) + sizeof(SUBCELLINFO)));
	CompressedDTMData = (LPSTR)&lpGWDHead->GWDData + (sizeof(DTMKEY) + sizeof(SUBCELLINFO) + 4);
	
	Fid1 = GSSiOpenFile ("e:\\mgvngi\\filelist.txt",&OFStruct,OF_READ); 
	TotLen = _llseek (Fid1,0,2);
	_llseek (Fid1,0,0);
	CreateStatusWindow (hWndMain,1,NULL);
	while (ContinueProcessing && fgetstring (FileName,128,Fid1))
	{   
		nFiles++;
		Fid2 = GSSiOpenFile (FileName,&OFStruct,OF_READ);  
		while (fgetstring (pRec,7000,Fid2))
		{   
			nSubCells++;  
			if (nSubCells == debugsubcell)
				ii=1;
			OutOfRange=FALSE;
			GeoSeg = ldread (pRec,12) - 1; 
			SubCell = ldread (&pRec[12],6) - 1; 
			if (GeoSeg == 852606 && SubCell == 180)
				ii=1;
			DTMKey.GEOSEG_ROW = GeoSeg / 4096; 
			DTMKey.GEOSEG_COL = GeoSeg % 4096;    
			DTMKey.SUBCEL_ROW = SubCell / 16;
			DTMKey.SUBCEL_COL = SubCell % 16;    
			_fmemmove (&SubCellID,&DTMKey,4);
			Bias = ldread (&pRec[18],6);
			NumElv = ldread (&pRec[24],6); 
			Indeterminate = ldread (&pRec[30],6);
			if (Bias <=0)
				ii=1;
			n = min (1023,NumElv);
			pos = 36;
			for (i=0;i<n;i++,pos+=6)
				SubcellData[i] = ldread (&pRec[pos],6);   
			if (NumElv == 1024)
			{
				fgetstring (pRec,7000,Fid2);
				SubcellData[1023] = ldread (pRec,12);
			}
			n = min (1024,NumElv); 
			hCell = NEXPND (n,SubcellData); 
			if (hCell)
			{  
				SubcellDat = (LPSHORT)GlobalLock (hCell); 
				n=1024;
				MinElv = SHRT_MAX;
				MaxElv = SHRT_MIN;   
				_fmemset (DTMData,0,4096);
				for (i=0;i<n;i++)
				{   
					if (SubcellDat[i] < 31100)
						DTMData[i] = SubcellDat[i] + Bias;
					else
						DTMData[i] = LONG_MAX;
				}
				GSSiGlobUlFree (&hCell);    
				SUBCELLInfo = CompressSubcell (DTMData,CompressedDTMData,pBias);  
				_fmemset (DTMData2,0,4096); 
				ExpandSubcell (DTMData2,SUBCELLInfo,CompressedDTMData,pBias);
				for (i=0;i<1024;i++)
					if (DTMData[i] != DTMData2[i]) 
					{
						GSSiMessageBox (FileName,NULL,MB_ICONEXCLAMATION);
						goto Exit;        
					}
				*pSUBCELLInfo = SUBCELLInfo;
				Offset = _llseek (lpGWDHead->Fid,0,1);  
				length = sizeof(DTMKEY) + sizeof(SUBCELLINFO) + 4 + SUBCELLInfo.LENGTH;
			    _lwrite (lpGWDHead->Fid,(char *)&length,2);
			    _lwrite (lpGWDHead->Fid,(char *)&lpGWDHead->GWDData,length);
	        	BT_PUT (lpGWDHead->BTHandle[0],(LPSTR)&DTMKey,(LPSTR)&Offset);
	        }
	        else
	        {
	        	char	str[256];
	        	
	        	sprintf (str,"%ld %i %s",GeoSeg,SubCell,FileName);  
	        	AppendFile ("c:\\badcells.txt",str);
	        }
		}
Exit: 
		GSSiClose (Fid2);  
		CurLoc = _llseek (Fid1,0,1);
		StatusWindowUpdate (FileName,"", TotLen, CurLoc);
	}    
	ContinueProcessing = TRUE;
	GSSiClose (Fid1);   
	DestroyStatusWindow ();
	GSSiGlobUlFree (&hRec);  
	GlobalUnlock (hDB);
    CloseGWDatabase (hDB); 
	return;
} 

BOOL LoadERDASDem (void)
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
	OFSTRUCT	OFStruct;
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
	GWDHEADER GWDHead; 
	LPGWDHEADER	lpGWDHead;
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
	HANDLE	hData=GSSiGlobAlloc (GMEM_MOVEABLE,4096+2048);
	LPLONG		DTMElev, DTMData2=(LPLONG)GlobalLock (hData);  
	LPSHORT		SubcellData=(LPSHORT)(DTMData2+1024);
	
	
	 lpGWDHead = &GWDHead; 
    _fmemset (lpGWDHead,0,sizeof(GWDHEADER));

	 FidData = GSSiOpenFile (File,&OFStruct,OF_CREATE);
	 GWDHead.NumFields=0;
	 GWDHead.NumIndex=1;
	 GWDHead.Version=1;
	 GWDHead.NumIndexFields[0]=1;
	 GWDHead.IndexFields[0][0]=0;
	 _lwrite (FidData,&GWDHead,sizeof(GWDHEADER));
	ibeg = 0;

	FldInfo.Len = sizeof(DTMKEY);
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"DTMKEY");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"BIAS");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = sizeof(SUBCELLINFO);
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"SUBCELLINFO");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	
	FldInfo.Len = 4096;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"COMPRESSEDNODES");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;
	
	 GWDHead.Reclen=ibeg; 
	 GWDHead.TimeStamp = 0;
	 _llseek (FidData,0,0);
	 _lwrite (FidData,&GWDHead,sizeof(GWDHEADER));
 	 _llseek (FidData,0,2);
			     
	 BTVar[0].BT_VARLEN=4;
	 BTVar[0].BT_VARTYP=BT_INTEGER;
	 BTVar[0].BT_VAROFF=0;
	 lpDot = _fstrrchr (File,'.');
	 _fstrcpy (lpDot,".in1");	
	 BT_CREATE (File, 4, FALSE, 1, 1,BTVar,FALSE, 0, 0, FALSE);
 	 GSSiClose (FidData);
	 _fstrcpy (lpDot,".dtm");	

     hDB = OpenGWDatabase (File,BT_WRITE);
	lpGWDHead = (LPGWDHEADER)GlobalLock (hDB); 
	pSUBCELLInfo = (LPSUBCELLINFO)((LPSTR)&lpGWDHead->GWDData + (sizeof(DTMKEY)));
	pBias = (LPLONG)((LPSTR)&lpGWDHead->GWDData + (sizeof(DTMKEY) + sizeof(SUBCELLINFO)));
	CompressedDTMData = (LPSTR)&lpGWDHead->GWDData + (sizeof(DTMKEY) + sizeof(SUBCELLINFO) + 4);
	
	Fid = GSSiOpenFile ("i:\\mndtm\\statedem.gis",&OFStruct,OF_READ); 
	TotLen = _llseek (Fid,0,2);
	_llseek (Fid,0,0);
	CreateStatusWindow (hWndMain,1,NULL);
	_lread (Fid,&eh,sizeof(erdhead));  
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
	hSubcellHandles = GSSiGlobAlloc (GHND,nSubcells*sizeof(HANDLE));
	SubcellHandles = (LPHANDLE)GlobalLock (hSubcellHandles); 
	for (i=0;i<nSubcells;i++)
		SubcellHandles[i] = GSSiGlobAlloc (GMEM_MOVEABLE,1024*4);
	SubcellRow=31; 
	SPY = eh.yt - 16 * eh.ycell;
	while (ContinueProcessing && irow--)
	{                          
		for (icell = 0;icell<nSubcells;icell++)
		{   
			DTMRow = (LPLONG)GlobalLock (SubcellHandles[icell]);
			DTMRow += 32 * SubcellRow; 
			if (icell < nSubcells-1)
				_lread (Fid,ERDASRow,32*2);   
			else
			{
				_lread (Fid,ERDASRow,(size_t)(LastLen*2));
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
						GSSiMessageBox ("decompress error",NULL,MB_ICONEXCLAMATION);
						goto Exit;        
					} 
				if (*pBias < LONG_MAX)
				{
					*pSUBCELLInfo = SUBCELLInfo;
					Offset = _llseek (lpGWDHead->Fid,0,1);  
					length = sizeof(DTMKEY) + sizeof(SUBCELLINFO) + 4 + SUBCELLInfo.LENGTH;
				    _lwrite (lpGWDHead->Fid,(char *)&length,2);
				    _lwrite (lpGWDHead->Fid,(char *)&lpGWDHead->GWDData,length);
		        	BT_PUT (lpGWDHead->BTHandle[0],(LPSTR)&DTMKey,(LPSTR)&Offset); 
		        }
		        SPX += 32 * eh.xcell;
		    } 
	        SPY -= 32 * eh.ycell;
			SubcellRow=32; 
		}
		SubcellRow--; 
Exit: 
		CurLoc = _llseek (Fid,0,1);
		StatusWindowUpdate ("","", TotLen, CurLoc);
	}    
	Offset = _llseek (lpGWDHead->Fid,0,1);  
	length = sizeof(DTMDATA);
    _lwrite (lpGWDHead->Fid,(char *)&length,2);
    _lwrite (lpGWDHead->Fid,(char *)&DTMData,length);   
    DTMKeyl = LONG_MAX;
	BT_PUT (lpGWDHead->BTHandle[0],(LPSTR)&DTMKeyl,(LPSTR)&Offset); 
	ContinueProcessing = TRUE;
	GSSiClose (Fid);   
	DestroyStatusWindow ();
	GlobalUnlock (hDB);
    CloseGWDatabase (hDB); 
	for (i=0;i<nSubcells;i++)
		GSSiGlobFree (&SubcellHandles[i]);
    GSSiGlobUlFree (&hSubcellHandles); 
    GSSiGlobUlFree (&hData);
	return TRUE;
} 

BOOL LoadGRIDDTM (LPSTR InFile, LPSTR OutFile)
{
	HFILE	Fid;
	OFSTRUCT	OFStruct;
	UINT	irow, icol;   
	DPOINT	Point;
	HFILE		Fid1, Fid2;
	HANDLE		hRec=GSSiGlobAlloc (GMEM_MOVEABLE,UINT_MAX);
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
	GWDHEADER GWDHead; 
	LPGWDHEADER	lpGWDHead;
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
	HANDLE	hData=GSSiGlobAlloc (GMEM_MOVEABLE,4096+2048);
	LPLONG		DTMElev, DTMData2=(LPLONG)GlobalLock (hData);  
	LPSHORT		SubcellData=(LPSHORT)(DTMData2+1024); 
	BOOL	rtn=FALSE;
	
	
	 lpGWDHead = &GWDHead; 
    _fmemset (lpGWDHead,0,sizeof(GWDHEADER));

	 FidData = GSSiOpenFile (OutFile,&OFStruct,OF_CREATE);
	 GWDHead.NumFields=0;
	 GWDHead.NumIndex=1;
	 GWDHead.Version=1;
	 GWDHead.NumIndexFields[0]=1;
	 GWDHead.IndexFields[0][0]=0;
	 _lwrite (FidData,&GWDHead,sizeof(GWDHEADER));
	ibeg = 0;

	FldInfo.Len = sizeof(DTMKEY);
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"DTMKEY");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"BIAS");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	FldInfo.Len = sizeof(SUBCELLINFO);
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"SUBCELLINFO");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;

	
	FldInfo.Len = 4096;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"COMPRESSEDNODES");
	_lwrite (FidData,&FldInfo,sizeof(FldInfo));
	GWDHead.NumFields++;
	
	 GWDHead.Reclen=ibeg; 
	 GWDHead.TimeStamp = 0;
	 _llseek (FidData,0,0);
	 _lwrite (FidData,&GWDHead,sizeof(GWDHEADER));
 	 _llseek (FidData,0,2);
			     
	 BTVar[0].BT_VARLEN=4;
	 BTVar[0].BT_VARTYP=BT_INTEGER;
	 BTVar[0].BT_VAROFF=0;
	 lpDot = _fstrrchr (OutFile,'.');
	 _fstrcpy (lpDot,".in1");	
	 BT_CREATE (OutFile, 4, FALSE, 1, 1,BTVar,FALSE, 0, 0, FALSE);
 	 GSSiClose (FidData);
	 _fstrcpy (lpDot,".dtm");	

     hDB = OpenGWDatabase (OutFile,BT_WRITE);
	lpGWDHead = (LPGWDHEADER)GlobalLock (hDB); 
	pSUBCELLInfo = (LPSUBCELLINFO)((LPSTR)&lpGWDHead->GWDData + (sizeof(DTMKEY)));
	pBias = (LPLONG)((LPSTR)&lpGWDHead->GWDData + (sizeof(DTMKEY) + sizeof(SUBCELLINFO)));
	CompressedDTMData = (LPSTR)&lpGWDHead->GWDData + (sizeof(DTMKEY) + sizeof(SUBCELLINFO) + 4);
	
	Fid = GSSiOpenFile (InFile,&OFStruct,OF_READ);
	if (!Fid)
		return FALSE; 
	TotLen = _llseek (Fid,0,2);
	_llseek (Fid,0,0);
	CreateStatusWindow (hWndMain,1,NULL);
	pRec=GlobalLock (hRec); 
	fgetstring (pRec,128,Fid);
	if (sscanf (pRec,"%ld %ld %Flf %ld %i %Flf %Flf",&nRows,&nCols,&DTMData.GridSpace,&VoidVal,&nCharPerCell,&DTMData.SouthWestNode.x,&Maxy) != 7)  
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
	hSubcellHandles = GSSiGlobAlloc (GHND,nSubcells*sizeof(HANDLE));
	SubcellHandles = (LPHANDLE)GlobalLock (hSubcellHandles); 
	for (i=0;i<nSubcells;i++)
		SubcellHandles[i] = GSSiGlobAlloc (GMEM_MOVEABLE,1024*4);
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
						GSSiMessageBox ("decompress error",NULL,MB_ICONEXCLAMATION);
						goto Exit;        
					} 
				if (*pBias < LONG_MAX)
				{
					*pSUBCELLInfo = SUBCELLInfo;
					Offset = _llseek (lpGWDHead->Fid,0,1);  
					length = sizeof(DTMKEY) + sizeof(SUBCELLINFO) + 4 + SUBCELLInfo.LENGTH;
				    _lwrite (lpGWDHead->Fid,(char *)&length,2);
				    _lwrite (lpGWDHead->Fid,(char *)&lpGWDHead->GWDData,length);
		        	BT_PUT (lpGWDHead->BTHandle[0],(LPSTR)&DTMKey,(LPSTR)&Offset); 
		        }
		        SPX += 32 * DTMData.GridSpace;
		    } 
	        SPY -= 32 * DTMData.GridSpace;
			SubcellRow=32; 
		}
		SubcellRow--; 
		CurLoc = _llseek (Fid,0,1);
		StatusWindowUpdate ("","", TotLen, CurLoc); 
		GlobalUnlock (hRec);
	}    
	Offset = _llseek (lpGWDHead->Fid,0,1);  
	length = sizeof(DTMDATA);
    _lwrite (lpGWDHead->Fid,(char *)&length,2);
    _lwrite (lpGWDHead->Fid,(char *)&DTMData,length);   
    DTMKeyl = LONG_MAX;
	BT_PUT (lpGWDHead->BTHandle[0],(LPSTR)&DTMKeyl,(LPSTR)&Offset); 
Exit: 
	ContinueProcessing = TRUE;
	GSSiClose (Fid);   
	DestroyStatusWindow ();
	GSSiGlobUlFree (&hRec);  
	GlobalUnlock (hDB);
    CloseGWDatabase (hDB); 
	for (i=0;i<nSubcells;i++)
		GSSiGlobFree (&SubcellHandles[i]);
    GSSiGlobUlFree (&hSubcellHandles); 
    GSSiGlobUlFree (&hData);
	return TRUE;
} 

long N3PNT (LPLONG MODELN,double X,double Y,short MISING)
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
      double RELV, NUM, DENOM, XSQ, YSQ, ONEMNX, ONEMNY, SQ1MNX, SQ1MNY;
      double D1, D2, D3, D4;
      
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
 S40: RELV = NUM / DENOM;
      return (RELV);
}

double N4PNT (LPLONG MODELN,double X,double Y)
{
      double DM2M1, DM3M4, MN1MN2, MN3MN4, RELV; 
      
      DM2M1 = MODELN[2] - MODELN[1];
      DM3M4 = MODELN[3] - MODELN[4];
      MN1MN2 = (double)MODELN[1] + Y * DM2M1;
      MN3MN4 = (double)MODELN[4] + Y * DM3M4;
      RELV = MN1MN2 + X * (MN3MN4 - MN1MN2);
	  return RELV;
} 

void NGSANE (long ISPXDM,long ISPYDM,LPLONG pIGESEG,LPSHORT pISUBCL,LPSHORT pIEL)
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
	return;
}  

HANDLE DTMOpen (LPSTR FileNameIN, double NULLElv,short Mode)
{
	HANDLE	hSurf=0, hDB; 
	UINT	i;    
	LPDTMINFO	pDTMInfo; 
	long	DTMKey=LONG_MAX, Offset; 
	LPGWDHEADER	lpGWDHead;     
	LPDTMDATA	pDTMData;
	static	BOOL	First=TRUE;  
	short	Type=0;
	char	FileName[256];
	
	if (First)
		for (i=0;i<MAXOPENSURF;i++)
			hOpenSurf[i] = 0;
	First = FALSE;
	
	_fstrcpy (FileName,FileNameIN);
	_fstrupr (FileName);
	if (StringEndsWith(FileName,".DTM"))
		Type = 1;
	else if (StringEndsWith(FileName,"INDEX"))
		Type = 2; 
	if (!Type)
		return 0;
	switch (Type)
	{
		case 1:
			
		if (!(hDB = OpenGWDatabase (FileName,Mode)))
			return 0;
		hSurf = GSSiGlobAlloc (GHND,sizeof(DTMINFO));
		pDTMInfo = (LPDTMINFO)GlobalLock (hSurf);
		pDTMInfo->hDB = hDB;   
		lpGWDHead = (LPGWDHEADER)GlobalLock (pDTMInfo->hDB); 
		pDTMData = (LPDTMDATA)((LPSTR)&lpGWDHead->GWDData);
	   	if (!BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&DTMKey,BT_FIRST,BT_EQ,(LPSTR)&Offset)) 
	   	{
			unsigned short	len;    
			LPLONG	pCell;
	
		    _llseek (lpGWDHead->Fid,Offset,0);
		    _lread (lpGWDHead->Fid,&len,2);
		    _lread (lpGWDHead->Fid,&lpGWDHead->GWDData,len); 
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
		for (i=0;i<MAXOPENSURF;i++)
		{ 
			if (!hOpenSurf[i]) 
			{
				hOpenSurf[i] = hSurf;
				break;
			}
		} 
		break;
	
		case 2:
		break;
	}
	return hSurf;
}

void DTMClose (LPHANDLE pHandle)
{   
	UINT	i,j;
	LPDTMINFO	pDTMInfo;
	
	if (!pHandle)
	{
		for (i=0;i<MAXOPENSURF;i++)
		{   
			if (hOpenSurf[i])
			{
				pDTMInfo = (LPDTMINFO)GlobalLock (hOpenSurf[i]);
				CloseGWDatabase (pDTMInfo->hDB); 
				for (j=0;j<MAXDTMCELLBUFFERS;j++)
					if (pDTMInfo->hCell[j] > 1)
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
				CloseGWDatabase (pDTMInfo->hDB);
				for (j=0;j<MAXDTMCELLBUFFERS;j++)
					if (pDTMInfo->hCell[j] > 1)
						GSSiGlobFree (&pDTMInfo->hCell[j]);
				GSSiGlobUlFree (&hOpenSurf[i]);
				break;
			}
		}
		*pHandle = 0;
	}
	return;
}

HANDLE GetDTMSubCell (HANDLE hSurf,long GeoSeg, short SubCell,LPDTMINFO pDTMInfo)
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
	
	GeoSeg--; 
	SubCell--; 
	DTMKey.GEOSEG_ROW = GeoSeg / 4096; 
	DTMKey.GEOSEG_COL = GeoSeg % 4096;    
	DTMKey.SUBCEL_ROW = SubCell / 16;
	DTMKey.SUBCEL_COL = SubCell % 16;   
	_fmemmove (&CellID,&DTMKey,4);
if (CellID == 270999782)
	ii=1;
	for (i=0;i<MAXDTMCELLBUFFERS;i++)
		if (CellID == pDTMInfo->CellID[i])
		{
			pDTMInfo->CellUse[i]=++NextDTMUse;
			if (pDTMInfo->hCell[i] > 1)
				handle = pDTMInfo->hCell[i];
			return (handle);
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
	if (pDTMInfo->hCell[MinUseID] > 1) 
    	GSSiGlobFree (&pDTMInfo->hCell[MinUseID]);
    else
    	pDTMInfo->hCell[MinUseID] = 0;
   	if (!BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&CellID,BT_FIRST,BT_EQ,(LPSTR)&Offset)) 
   	{
		unsigned short	len;    
		LPLONG	pCell;

	    _llseek (lpGWDHead->Fid,Offset,0);
	    _lread (lpGWDHead->Fid,&len,2);
	    _lread (lpGWDHead->Fid,&lpGWDHead->GWDData,len); 
	    pDTMInfo->hCell[MinUseID] = GSSiGlobAlloc (GMEM_MOVEABLE,4096);
	    pCell = (LPLONG)GlobalLock (pDTMInfo->hCell[MinUseID]);
		ExpandSubcell (pCell,*pSUBCELLInfo,CompressedDTMData,pBias);
		GlobalUnlock (pDTMInfo->hCell[MinUseID]);
		handle = pDTMInfo->hCell[MinUseID];
	} 
	else
	    pDTMInfo->hCell[MinUseID] = 1;
	pDTMInfo->CellID[MinUseID] = CellID;
	pDTMInfo->CellUse[MinUseID] = ++NextDTMUse;    
	GlobalUnlock (pDTMInfo->hDB);
	return handle;
}

BOOL SetDTMSubCell (long GeoSeg, short SubCell,short node,double Elev,short Units,LPDTMINFO pDTMInfo)
{
	long		CellID,ii;
	DTMKEY		DTMKey;
	LPSTR		CompressedDTMData;
	UINT		i, MinUseID;  
	long		MinUse=LONG_MAX, Offset, IElev;
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

	    _llseek (lpGWDHead->Fid,Offset,0);
	    _lread (lpGWDHead->Fid,&len,2);
	    _lread (lpGWDHead->Fid,&lpGWDHead->GWDData,len); 
	    hCell = GSSiGlobAlloc (GMEM_MOVEABLE,4096);
	    pCell = (LPLONG)GlobalLock (hCell);
		ExpandSubcell (pCell,*pSUBCELLInfo,CompressedDTMData,pBias);
		pCell[node] = IElev;
		*pSUBCELLInfo = CompressSubcell (pCell,CompressedDTMData,pBias); 
		length = sizeof(DTMKEY) + sizeof(SUBCELLINFO) + 4 + pSUBCELLInfo->LENGTH;
		if (length > len) 
			Offset = _llseek (lpGWDHead->Fid,0,1);
		else  
	    	_llseek (lpGWDHead->Fid,Offset,0);
	    _lwrite (lpGWDHead->Fid,(char *)&length,2);
	    _lwrite (lpGWDHead->Fid,(char *)&lpGWDHead->GWDData,length);
    	BT_PUT (lpGWDHead->BTHandle[0],(LPSTR)&CellID,(LPSTR)&Offset);
		GSSiGlobUlFree (&hCell);  
		rtn = TRUE;
	} 
	GlobalUnlock (pDTMInfo->hDB);
	return rtn;
}

double NGIELV (DPOINT Point,HANDLE hSurf,short DesiredUnits)
{     
	//DesiredUnits (0=feet, 1=meters)
      double    SPX=Point.x, SPY=Point.y, TSPX, TSPY, ELV, X, Y; 
      long		INTX, INTY, LSPXDM, LSPYDM;
      long      SURNOD[5], SNGNUM[5];
      short		SNSCNM[5], SNELNM[5], NODE, NOFINN, MISING, I, SurfUnits;  
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
	  	return PlaneElev;
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
	  if (pDTMInfo->Bounds.xmx > pDTMInfo->Bounds.xmn)
	  {
		  if (!PointInBounds (DTMPoint,&pDTMInfo->Bounds))
		  	goto S500;
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
   	  GlobalUnlock (hSurf);
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
	return (ELV);
S1000:
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
				break;
			}
		break;
	}
	return (ELV);
} 

BOOL SetNGIELV (DPOINT Point,double Elev,HANDLE hSurf,short Units)
{     
	//Units (0=feet, 1=meters)
      double    SPX=Point.x, SPY=Point.y, TSPX, TSPY, ELV, X, Y, CurElev; 
      long		INTX, INTY, LSPXDM, LSPYDM,ii;
      long      SURNOD[5], SNGNUM[5];
      short		SNSCNM[5], SNELNM[5], NODE, NOFINN, MISING, I, SurfUnits;  
      LPLONG	pCell;
      HANDLE	hCell;
	  LPDTMINFO	pDTMInfo; 
	  BOOL		rtn;
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
	  	return FALSE;
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
	  rtn = SetDTMSubCell (SNGNUM[NODE],SNSCNM[NODE],SNELNM[NODE]-1, Elev,Units, pDTMInfo);
	  GlobalUnlock (hSurf);
	  return rtn;
}


long GetDTMHoles (LPSTR DTMName, LPSTR OutFile)
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
	OFSTRUCT	OFStruct;
	long	nRecs, nLoaded=0;
	
	hSurf = DTMOpen (DTMName, LONG_MAX,BT_READ);
	if (!hSurf)
		return 0; 
	OutFid = GSSiOpenFile (OutFile,&OFStruct,OF_CREATE);  
	fputstring ("\"X\",\"Y\"",OutFid);
	pDTMInfo = (LPDTMINFO)GlobalLock (hSurf);
	lpGWDHead = (LPGWDHEADER)GlobalLock (pDTMInfo->hDB); 
	pSUBCELLInfo = (LPSUBCELLINFO)((LPSTR)&lpGWDHead->GWDData + (sizeof(DTMKEY)));
	pBias = (LPLONG)((LPSTR)&lpGWDHead->GWDData + (sizeof(DTMKEY) + sizeof(SUBCELLINFO)));
	CompressedDTMData = (LPSTR)&lpGWDHead->GWDData + (sizeof(DTMKEY) + sizeof(SUBCELLINFO) + 4); 

    nRecs = BT_NUM_IN_INDEX (lpGWDHead->BTHandle[0]);
	CreateStatusWindow (hWndMain,1,NULL);

   	while (ContinueProcessing && !BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&DTMKey,pos,BT_ANY,(LPSTR)&Offset)) 
   	{
		unsigned short	len;    
		LPLONG	pCell;
        
        pos = BT_NEXT;
	    _llseek (lpGWDHead->Fid,Offset,0);
	    _lread (lpGWDHead->Fid,&len,2);
	    _lread (lpGWDHead->Fid,&lpGWDHead->GWDData,len); 
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
	GSSiClose (OutFid);
	DestroyStatusWindow();  
	ContinueProcessing = TRUE;
	return nHoles;
}


