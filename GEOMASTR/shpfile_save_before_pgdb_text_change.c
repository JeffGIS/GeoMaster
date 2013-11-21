#include "graphint.h"
#include "translat.h"
#include "extrndb.h"
#include "shapefil.h"
#define MAXINDEXSYMBOLS	3200

static SHPHEADER		SHPHeader; 
static SHPPOLYHEADER	SHPPolyHeader;  
static SHPPOINTREC		SHPPointRec;
static long				SHPBaseRefno=0;
static BOOL				SHPProjectionIsBase;
static short			NumSHPParms; 
static char				SHPParms[4096]="";  
static char				SHPTag[100], SHPTAG[100]; 
static char				LastSHPFile[MAX_PATH]=""; 
static HFILE			FidSmallDBF=HFILE_ERROR;
static char				SmallDBFName[MAX_PATH]="";
static MNMXCORD			SHPFileMNMX;    
static HANDLE			hSHPIndexBlocks=0;
static long				NumSHPIndexBlocks; 
static time_t			SHPParmTime=0;  
static short			HaveSHPSym=-1; 
static short			NumIndexSyms=0;
static short			IndexSyms[MAXINDEXSYMBOLS]; 
static char				SHPBeginDate[256];
static char				SHPEndDate[256];
static BYTE				BlockInWBounds[1024];  
static double			PGDBGridOrigX,PGDBGridOrigY,PGDBGridSize;  
static long				ExtraBytes;
static short			WordLen[16];
static char				Words[16][128];
#include "gmextern.h"
#include <sys\types.h>
#include <sys\stat.h>         

typedef struct {
					long	Offset; 
					mnmxCor	MinMax;
					short	SymNum;
				}SHPINDEXRECORD;
typedef SHPINDEXRECORD	FAR	*LPSHPINDEXRECORD; 


BOOL OpenSHPFile (LPSTR SHPFileName)
{   
	SHPFid = GSSiOpenFile (SHPFileName,0,OF_READ);
	if (SHPFid == HFILE_ERROR)
		return FALSE;
	if (!(SHPType = ReadSHPHeader (SHPFid,&SHPFileMNMX)))
    {
    	GSSiClose (SHPFid);
    	return FALSE;
    }
	OpenSHPFileIndex (SHPFileName);
	return TRUE;
} 

void CloseSHPFile (void)
{   
	GSSiClose (SHPFid);
	OpenSHPFileIndex (0);
	return;
} 

void GetSHPName (LPSTR Name)
{
	_fstrcpy (Name,LastSHPFile);
	return;
}                            

/*void CreateFidDBF (void)
{   
	HANDLE	hMem;
	LPOFSTRUCT	pOFStruct;
	
	if (!*SmallDBFName)
		GSSiGetTempFileName (0,"gms",0,(LPSTR)SmallDBFName);
	hMem = GSSiGlobAlloc (1416,GMEM_MOVEABLE,sizeof(OFSTRUCT));
	pOFStruct = (LPOFSTRUCT)GlobalLock (hMem);	
   	FidSmallDBF = OpenFile (SmallDBFName,pOFStruct,OF_CREATE);
   	GSSiGlobUlFree (&hMem);
	return;
}  

BOOL CloseFidDBF (BOOL Final)
{   
	OFSTRUCT	OFStruct;
	HFILE		AvailableFid; 
	BOOL		UseSmall=FALSE;
	HANDLE		hMem;
	LPOFSTRUCT	pOFStruct; 
	LPSTR		TestName;
	
	if (FidSmallDBF == HFILE_ERROR)
		return FALSE; 
	if (Final)
	{
		_lclose (FidSmallDBF);
		remove (SmallDBFName);
		FidSmallDBF = HFILE_ERROR;  
		return TRUE;
	}
	hMem = GSSiGlobAlloc (1416,GMEM_MOVEABLE,sizeof(OFSTRUCT)+256);
	TestName = GlobalLock (hMem);
	pOFStruct = (LPOFSTRUCT)(TestName + 256);	
	GSSiGetTempFileName (0,"gms",0,(LPSTR)TestName);
	AvailableFid = OpenFile (TestName,pOFStruct,OF_CREATE); 
	if (AvailableFid > 20)
		UseSmall = TRUE;
	_lclose (AvailableFid);
	OpenFile (TestName,pOFStruct,OF_DELETE); 
	
	if (UseSmall)
	{
		_lclose (FidSmallDBF);
		remove (SmallDBFName);
		FidSmallDBF = HFILE_ERROR;
	}
   	GSSiGlobUlFree (&hMem);
	return UseSmall;
} */

BOOL SelectSHPFile (LPSTR File,LPSTR SymName)
{   
	if (!*LastSHPFile)
		return FALSE;
	_fstrcpy (File,LastSHPFile);
	return TRUE;
}  

void DecodeSHPParam (LPSTR str,LPSTR cDesc,LPSTR cIF, LPSTR cColor, LPSTR cWidth, LPSTR cRot)
{   
	LPSTR	pIF, pColor, pWidth, pRot;
	
	if ((pIF = _fstrchr (str,';')))
	{
		*pIF++ = 0;
		if ((pColor = _fstrchr (pIF,';')))
		{
			*pColor++ = 0;
			if ((pWidth = _fstrchr (pColor,';')))
			{
				*pWidth++ = 0;
				if ((pRot = _fstrchr (pWidth,';')))
				{
					*pRot++ = 0;
				}
				else
					pRot = _fstrchr (str,0);
			}
			else
				pWidth = _fstrchr (str,0);
		}
		else
			pColor = pWidth = pRot = _fstrchr (str,0);
	}
	else
		pIF = pColor = pWidth = _fstrchr (str,0); 
	_fstrcpy (cDesc,str);
	_fstrcpy (cIF,pIF);
	_fstrcpy (cColor,pColor);
	_fstrcpy (cWidth,pWidth);
	_fstrcpy (cRot,pRot);
	return;
}

BOOL LoadSHPParm (LPSTR SHPFileName,long Type,HWND hWnd)
#if ENABLETRACE
{GSSiEnterProg (1374);
#endif
{   
	char	Name[MAX_PATH], str[260], Projection[34], Units[34];
	char	SymName[66], cWidth[64],cRot[64],cColor[64], cIF[128];
	LPSTR	pDot, pTAG,pWidth, pParm=SHPParms;  
	short	l;
	HFILE	Fid; 
	BOOL	FileIsIndex;
	struct _stat    statParmFile; 

    if (!SHPFileName)
    {   
    	HaveIndexParmFile = FALSE;  
    	goto RtnTrue;
    }
    if (HaveIndexParmFile) 
    {
    	goto RtnTrue;
    }
	*SHPBeginDate = 0;
	*SHPEndDate = 0;
	SHPParmTime = 0;
	NumSHPParms = 0;  
	SHPProjectionIsBase = TRUE;
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
        
        case 4://personalgeodb area type???
      	case SHPT_TEXT:
		case SHPT_POLYGON:
		case SHPT_POLYGONM:
		case SHPT_POLYGONZ:  
		case SHPT_PGDB_POLYGONZ:
			GetGlobalCVal ("[%DefaultShapeAreaSymbol]",SHPParms,"PARCEL"); 
		break;
	}
	GetGlobalCVal ("[%DefaultShapeProjection]",Projection,"BASEPROJ"); 
	LoadProjection(0,Projection); 
	GetGlobalCVal ("[%DefaultShapeUnits]",Units,"FEET"); 
	if (!_fstricmp (Units,"FEET"))
		PRJ_UNITS[0] = 1;
	else if (!_fstricmp (Units,"METERS"))
		PRJ_UNITS[0] = 2; 
	else
		PRJ_UNITS[0] = 4;
	SHPBaseRefno = 0; 
	SHPIndexType = 1;
	_fstrcpy (Name,SHPFileName); 
	ExpandText (Name);
	l = _fstrlen (Name);
	if (l > 4 && !_fstricmp (&Name[l-5],"INDEX"))
	{
		HaveIndexParmFile = FALSE; 
		FileIsIndex = TRUE; 
		pDot = &Name[l];
	}
	else  
	{
		FileIsIndex = FALSE;
        if ((pDot = _fstrrchr (Name,'.')))
        {
        	if (!_fstrnicmp (pDot,".mdb",4))
        	{
        		if (*(pDot+4) != '(')
        			goto RtnFalse;
        		_fstrcpy (PGDBTable,pDot+5);
        		*LastChr (PGDBTable) = 0; 
        		*pDot = 0;
        		sprintf (str,"%s_%s.",Name,PGDBTable);
		        _fstrcpy (Name,str);
		        pDot = LastChr (Name);
        	}
        }
	}
	if (!pDot)
		goto RtnFalse;
	_fstrcpy (pDot,".gsp");
	Fid = GSSiOpenFile (Name,0,OF_READ); 
	if (Fid == HFILE_ERROR)  
	{
        FARPROC lpfnSETSHAPEPARAMMsgProc;  

		_fstrcpy (LastSHPFile,SHPFileName);
		ExpandText (LastSHPFile); 
		if (!Type || FileIsIndex || !GetGlobalBVal2 ("[%AUTOSHPPARM]",TRUE))
			goto RtnFalse;   
		{
			lpfnSETSHAPEPARAMMsgProc = MakeProcInstance((FARPROC)SETSHAPEPARAMMsgProc, hInst);
			DialogBox(hInst, (LPSTR)"SETSHAPEPARAM", hWnd, lpfnSETSHAPEPARAMMsgProc);
			FreeProcInstance(lpfnSETSHAPEPARAMMsgProc);
			Fid = GSSiOpenFile (Name,0,OF_READ);  
			if (Fid == HFILE_ERROR)  
	        	goto RtnFalse; 
	    }
	}
	if (FileIsIndex)
		HaveIndexParmFile = TRUE;
    GSSifstat (Fid,&statParmFile);
    SHPParmTime = statParmFile.st_mtime;
	fgetstring (Projection,32,Fid); 
	if (!*Projection)
		GetGlobalCVal ("[%DefaultShapeProjection]",Projection,"BASEPROJ"); 
	LoadProjection(0,Projection); 
	SHPProjectionIsBase = IS_BASE[0];
	fgetstring (Units,32,Fid);
	if (!*Units)
		GetGlobalCVal ("[%DefaultShapeUnits]",Units,"FEET"); 
	if (!_fstricmp (Units,"FEET"))
		PRJ_UNITS[0] = 1;
	else if (!_fstricmp (Units,"METERS"))
		PRJ_UNITS[0] = 2; 
	else
		PRJ_UNITS[0] = 4;
	fgetstring (str,32,Fid); 
	if (IndexEntryStartRef != LONG_MAX)
		SHPBaseRefno = IndexEntryStartRef;
	else
		SHPBaseRefno = atol (str);
	fgetstring (SHPTAG,99,Fid); 
	fgetstring (str,32,Fid); 
	SHPIndexType = atoi (str);
	_fmemset (SHPParms,0,sizeof(SHPParms));
	while (fgetstring (str,256,Fid))
	{   
		if (*str == '#')
			break;
		DecodeSHPParam (str,SymName,cIF,cColor,cWidth,cRot);
		_fstrcpy (pParm,SymName);
		l = _fstrlen (SymName);
		pParm += l+1;
		_fstrcpy (pParm,cIF);
		l = _fstrlen (cIF);
		pParm += l+1;
		_fstrcpy (pParm,cColor);
		l = _fstrlen (cColor);
		pParm += l+1;
		_fstrcpy (pParm,cWidth);
		l = _fstrlen (cWidth);
		pParm += l+1;
		_fstrcpy (pParm,cRot);
		l = _fstrlen (cRot);
		pParm += l+1;
		NumSHPParms++;
	}
	if (fgetstring (str,256,Fid))
		_fstrcpy (SHPBeginDate,str);
	if (fgetstring (str,256,Fid))
		_fstrcpy (SHPEndDate,str);
	
	GSSiClose (Fid); 
RtnTrue:
{
#if ENABLETRACE
GSSiExitProg (1374);
#endif
	return TRUE;
}
RtnFalse:
{
#if ENABLETRACE
GSSiExitProg (1374);
#endif
	return FALSE;
}
#if ENABLETRACE
}
#endif
} 

void AddSymToList (HWND hWndDlg, int DlgItemSym, int DlgItemPar, short idesc)
{   
	short	iparent; 
	char	DescName[100];
	
	iparent = GetDictSymParent (idesc); 
	GetDictSymName (idesc,DescName);
    ConvertSymName (DescName,1,FALSE,idesc);  
	if(GetVisibility(idesc))
		_fstrcat (DescName,"\t<on>\t");
	else
		_fstrcat (DescName,"\t\t");
	sprintf (_fstrchr(DescName,0),"%i\t%i",iparent,idesc);
	if (SendDlgItemMessage ((HWND)hWndDlg,DlgItemSym,LB_FINDSTRING,-1,(LPARAM) DescName) ==  LB_ERR)
		SendDlgItemMessage ((HWND)hWndDlg,DlgItemSym,LB_ADDSTRING,0,(LPARAM) DescName); 
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
			SendDlgItemMessage ((HWND)hWndDlg,DlgItemPar,LB_ADDSTRING,0,(LPARAM) DescName); 
	}
	return;
}

BOOL IsSHPFileVisible (void)
{
	LPSTR	pDesc, pClause, pC, pColor, pWidth, pRot; 
	short	idesc; 
	char	str[128]; 
//	return TRUE;
	if (!*SHPParms)
		return TRUE;
	pDesc = SHPParms;
	while (*pDesc)
	{
		pClause = _fstrchr (pDesc,0) + 1;
		pColor = _fstrchr (pClause,0) + 1;
		pWidth = _fstrchr (pColor,0) + 1;
		pRot = _fstrchr (pWidth,0) + 1;   
		_fstrcpy (str,pDesc);
		ExpandText (str);
		idesc = GetDictSymbolNumber (str); 
		if (GetVisibility (idesc))
			return TRUE;  
		pDesc = _fstrchr (pRot,0) + 1;
	}
	return FALSE;
}

BOOL SetSHPVis (HWND hWndDlg, int DlgItemSym, int DlgItemPar)
{
	LPSTR	pDesc, pClause, pC, pColor, pWidth, pRot; 
	short	idesc,i; 
	char	str[128]; 
	
	if (NumIndexSyms)
	{    
		for (i=0;i<NumIndexSyms;i++)
			AddSymToList (hWndDlg,DlgItemSym,DlgItemPar,IndexSyms[i]);
		return TRUE;
	}
	pDesc = SHPParms;
	while (*pDesc)
	{
		pClause = _fstrchr (pDesc,0) + 1;
		pColor = _fstrchr (pClause,0) + 1;
		pWidth = _fstrchr (pColor,0) + 1;
		pRot = _fstrchr (pWidth,0) + 1;   
		_fstrcpy (str,pDesc);
		ExpandText (str);
		idesc = GetDictSymbolNumber (str);   
		AddSymToList (hWndDlg,DlgItemSym,DlgItemPar,idesc);
		pDesc = _fstrchr (pRot,0) + 1;
	}
/*	else if (parent) 
	{   
		if(GetVisibility(idesc))
		{
    		if (_fstricmp (DescName,"ALL"))   
           		sprintf (str,"(%s)",DescName);
    		SendDlgItemMessage ((HWND) hWndDlg, -DlgItemPar,
        				     	CB_ADDSTRING, 0, (LPARAM) str);
	    }
	}*/ 

	return TRUE;
}

BOOL SetSHPParms (long RecordNumber)
{   
	LPSTR	pDesc, pTAG, pClause, pC, pColor, pWidth, pRot; 
	BOOL	rc;
	char	str[1024];  
	BOOL	rtn = FALSE;
	
	CurrentSHPRec = RecordNumber;
	CurrentRefno = SHPBaseRefno + RecordNumber;
	if (!hSHPDBF)
		return FALSE; 
	SetUseOnlyOneDBHandle (hSHPDBF);
	pTAG = SHPTAG;
	if (*pTAG && (pC = _fstrchr (pTAG,':')))
	{   
		_fstrcpy (SHPTag,pTAG); 
		ExpandText (SHPTag);
		pC = _fstrchr (SHPTag,':');
		*pC++ = 0;
		_fstrcpy (CurrentTAG,SHPTag);
		_fstrcpy (CurrentUDI,pC--);
		*pC = ':'; 
		ExpandText (CurrentUDI);
	}
	else
	{   
		*SHPTag = 0;
		*CurrentTAG = 0;
		*CurrentUDI = 0;
	} 
	if (HaveSHPSym < 0)
	{
    	LPOPENSQLDATA SQLPtr = (LPOPENSQLDATA)GlobalLock (hSHPDBF);
	    LPOPENFILEDATA FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle);
		pDesc = SHPParms;
		while (*pDesc)
		{
			pClause = _fstrchr (pDesc,0) + 1;
			pColor = _fstrchr (pClause,0) + 1;
			pWidth = _fstrchr (pColor,0) + 1;
			pRot = _fstrchr (pWidth,0) + 1;
	    	ConvertSQLToLogicP (str,pClause); 
			if (!*pClause || LogicPFile (SQLPtr,str,&rc))
			{
				break;
			}
			else
				pDesc = _fstrchr (pRot,0) + 1;
		}
		GlobalUnlock (SQLPtr->OFHandle);
		GlobalUnlock (hSHPDBF);   
		_fstrcpy (str,pDesc);
		ExpandText (str);
		CurrentDesc = GetDictSymbolNumber (str); 
		if (!*pDesc)
			goto Exit;
		if (*pColor) 
		{
			_fstrcpy (str,pColor);
			ExpandText (str);
			SHPColor = ConvertColor (atol (str),CurrentDesc);
		}
		else
			SHPColor = -1;
		if (*pWidth) 
		{
			_fstrcpy (str,pWidth);
			ExpandText (str);
			SHPWidth = atol (str);
			switch (*LastChr (str)) 
			{
				case 'P':
				case 'p':
					SHPWidth = -SHPWidth;
					break;
				case 'F':
				case 'f':
					SHPWidth *= FTM;
					break;
			}
			if (SHPType == SHPT_POINT)
				CurPointSize = SHPWidth;
		}
		else
			SHPWidth = 0;
	}
	else
		CurrentDesc = HaveSHPSym; 
	if (*SHPBeginDate)
	{
		_fstrcpy (str,SHPBeginDate);
		ExpandText (str);
		GRStartTime  = GREndTime  = atol(str);
	}
	if (*SHPEndDate)
	{
		_fstrcpy (str,SHPEndDate);
		ExpandText (str);
		GREndTime  = atol(str);
	}
	rtn = TRUE;   
Exit: 
	SetUseOnlyOneDBHandle (0);
	return rtn;
}

HFILE CreateSHPFileIndex (LPSTR IndexName,LPSTR SHPFileName)
{   
	HFILE	Fid, FidIdx;   
	long	i, Offset,ii;
	SHPINDEXRECORD	IndexRecord; 
	short	SaveIndexType = SHPIndexType;
	MNMXCORD	Bounds;
	HANDLE	hIndexBlocks;
	LPMINMAX BlockMinMax;
	long	RecsPerBlock = 100;
	long	NumIndexBlocks = max (2,min(1024,1+(NumSHPRecs-1)/RecsPerBlock)); 
	long	BlockID; 
	short	Version=2;
	char	txt[16]; 
	BOOL	ValidRec; 
	char	Name[128];   
	
	NumIndexSyms = 0;
	RecsPerBlock = 1+NumSHPRecs/(NumIndexBlocks-1);
	
	_fstrcpy (Name,SHPFileName);
	ExpandText (Name);
	Fid = GSSiOpenFile (Name,0,OF_READ);
	if (Fid == HFILE_ERROR)
		return Fid;   
	hIndexBlocks = GSSiGlobAlloc (0,GMEM_MOVEABLE,NumIndexBlocks*sizeof(mnmxCor));
	BlockMinMax = (LPMINMAX)GlobalLock (hIndexBlocks); 
	for (i=0;i<NumIndexBlocks;i++)
		MinMaxInit (&BlockMinMax[i]);
	DisableHalt = TRUE;
	CreateStatusWindow (CurView->hWnd,1,"Creating GeoMaster Shape File Index");      
	DisableHalt = FALSE;
	SHPIndexType = 0;  
	CurView->PassID = 4;
	FidIdx = GSSiOpenFile (IndexName,0,OF_CREATE);
	BigWrite (FidIdx,(HPSTR)&Version,2,-1); 
	BigWrite (FidIdx,(HPSTR)&NumIndexBlocks,4,-1); 
	BigWrite (FidIdx,(HPSTR)&RecsPerBlock,4,-1); 
	BigWrite (FidIdx,(HPSTR)BlockMinMax,NumIndexBlocks*sizeof(mnmxCor),-1);
	for (CurrentSHPRec=0;CurrentSHPRec<NumSHPRecs;CurrentSHPRec++)
	{   
    	SHPRecOffset = GetSHPRecordOffset (CurrentSHPRec,FALSE);
		IndexRecord.Offset = SHPRecOffset;
		ValidRec = ReadSHPRecordHeader (Fid,SHPRecOffset,&Bounds);
	    SetSHPParms (CurrentSHPRec); 
		BlockID = CurrentSHPRec/RecsPerBlock;
	    if (ValidRec)
	    { 
			IndexRecord.SymNum = CurrentDesc;  
			for (i=0;i<NumIndexSyms;i++)
				if (CurrentDesc == IndexSyms[i])
					goto HaveSym; 
			if (NumIndexSyms >= MAXINDEXSYMBOLS-1) 
			{
				GSSiMessageBox ("Maximum symbols exceeded",IndexName,MB_ICONEXCLAMATION); 
				break;
			}
			IndexSyms[NumIndexSyms++] = CurrentDesc;
	HaveSym:
			BoundsToMinMax (&IndexRecord.MinMax,&Bounds);
			AddMinMax (&BlockMinMax[BlockID], &IndexRecord.MinMax); 
		}
		else
		{
			IndexRecord.SymNum = -1;
			IndexRecord.Offset = -1;
		}
		BigWrite (FidIdx,(HPSTR)&IndexRecord,sizeof(IndexRecord),-1); 
//		ltoa (CurrentSHPRec,txt,10);
		StatusWindowUpdate (0,0,NumSHPRecs,CurrentSHPRec); 
		if (!ContinueProcessing)
			break;
	} 
	ContinueProcessing = TRUE;  
	BigWrite (FidIdx,(HPSTR)IndexSyms,NumIndexSyms*2,-1);
	BigWrite (FidIdx,(HPSTR)&NumIndexSyms,2,-1);
	GSSillseek (FidIdx,10,0);
	BigWrite (FidIdx,(HPSTR)BlockMinMax,NumIndexBlocks*sizeof(mnmxCor),-1);
	GSSiClose (Fid);   
	GSSiClose (FidIdx);
	FidIdx = GSSiOpenFile (IndexName,0,OF_READ);  
	SHPIndexType = SaveIndexType;
	GSSiGlobUlFree (&hIndexBlocks);
	DestroyStatusWindow(0); 
	return FidIdx;
}

BOOL OpenSHPFileIndex (LPSTR SHPFileName)
{ 
	char	Name[128];  
	LPSTR	pDot; 
	short	Version;   
	short	ii;
	
	if (!SHPFileName)
	{
		if (SHPIDXFid != HFILE_ERROR)
			GSSiClose (SHPIDXFid); 
		SHPIDXFid = HFILE_ERROR; 
		GSSiGlobFree (&hSHPIndexBlocks);
		GetSHPRecordOffset (-1,FALSE);
		CloseDataFile (TRUE,&hSHPDBF);
		return TRUE;
	} 
	_fstrcpy (Name,SHPFileName);
	ExpandText (Name); 
	_fstrcpy (LastSHPFile,Name);
	pDot = _fstrrchr (Name,'.');
	if (!pDot)
		return FALSE;
	_fstrcpy (pDot,".shx");
	SHPIDXFid = GSSiOpenFile (Name,0,OF_READ);  
	NumSHPRecs = (GSSifilelength (SHPIDXFid) - 100) / 8; 
	sprintf (Name,"SHP=%s",SHPFileName);
	{
		long SaveSHPRec = CurrentSHPRec; 
		
		OpenDataFile (Name,"",BT_READ,&hSHPDBF); 
		CurrentSHPRec = SaveSHPRec;
	}
	if (SHPIndexType == 1)
	{ 
		HFILE	TMPFid;
		
		_fstrcpy (Name,SHPFileName);  
		ExpandText (Name);
		pDot = _fstrrchr (Name,'.');
		if (!pDot)
			return FALSE;
		_fstrcpy (pDot,".gsi");
		TMPFid = GSSiOpenFile (Name,0,OF_READ); 
		if (TMPFid == HFILE_ERROR)
			TMPFid = CreateSHPFileIndex (Name,SHPFileName);
		else
		{
		    struct _stat stat; 
		    long	IndexTime;
		    double	dtime;
		     
		    GSSifstat (TMPFid,&stat); 
		    IndexTime = stat.st_mtime;
		    dtime = difftime (stat.st_mtime,SHPParmTime);  
		    if (dtime < 0)
		    {
		    	GSSiClose (TMPFid);
				TMPFid = CreateSHPFileIndex (Name,SHPFileName);  
			}
			else
			{
				HFILE SHPFid=GSSiOpenFile (SHPFileName,0,OF_READ); 
				
			    ii=GSSifstat (SHPFid,&stat);
			    dtime = difftime (stat.st_mtime,IndexTime);  
				GSSiClose (SHPFid);
			    if (dtime > 0)
			    {
			    	GSSiClose (TMPFid);
					TMPFid = CreateSHPFileIndex (Name,SHPFileName); 
				}  
			}	
		}
		GSSiClose (SHPIDXFid); 
		GetSHPRecordOffset (-1,FALSE);
		if (TMPFid == HFILE_ERROR)
			return FALSE; 
		SHPIDXFid = TMPFid;
		BigRead (SHPIDXFid,(HPSTR)&Version,2);
		if (Version != 2)
		{
			GSSiClose (SHPIDXFid); 
			TMPFid = CreateSHPFileIndex (Name,SHPFileName);
			GSSiClose (SHPIDXFid); 
			GetSHPRecordOffset (-1,FALSE);
			SHPIDXFid = TMPFid;
		}
		else
		{
			GSSillseek (SHPIDXFid,-2,2);
			GSSilread (SHPIDXFid,(HPSTR)&NumIndexSyms,2);
			GSSillseek (SHPIDXFid,-(NumIndexSyms+1)*2,2);
			GSSilread (SHPIDXFid,(HPSTR)&IndexSyms,NumIndexSyms*2);
			GSSillseek (SHPIDXFid,2,0);
		}
	}
	return TRUE;
} 

long GetSHPRecordOffset (long record,BOOL UseBounds)
{
	long	loc, BlockID,ii;  
	static	HANDLE	hOffsets=0;
	static	HANDLE	hBlocks=0;
	static	long	FirstLoc, LastLoc,NumBlocks,RecsPerBlock, FirstRecLoc;
	long	Len,rtn;
	long	MaxLen= (((long)USHRT_MAX-100) / sizeof(SHPINDEXRECORD)) * sizeof(SHPINDEXRECORD); 
	LPSTR	pLoc, pLen;
	LPSHPINDEXRECORD	pIndexRec;
	LPMINMAX BlockMinMax;
	
	HaveSHPSym = -1;
	if (record < 0) 
	{
		GSSiGlobFree (&hOffsets);
		GSSiGlobFree (&hBlocks); 
		NumIndexSyms = 0;
		return -1; 
	} 
	if (record > NumSHPRecs-1)
		return -1;
	if (!SHPProjectionIsBase)
		UseBounds = FALSE;
	if (record == 8190)
		ii=1; 
	if (CurView->PassID && CurView->PassID < 4)
	{
		if (SHPType == SHPT_POLYGON && CurView->PassID != 2)
			return -1;  
		if ((SHPType == SHPT_POINT || SHPType == SHPT_ARC) && CurView->PassID == 2)
			return -1;  
	}
	ItemSeg = record;  
	switch (SHPIndexType)
	{
		case 0: 
//			MessageBox (0,"case 0","",MB_OK);
			loc = 100 + record * 8; 
			if (!hOffsets || loc < FirstLoc || loc > LastLoc)
			{   
				GSSiGlobFree (&hOffsets);
				hOffsets = GSSiGlobAlloc (1417,GMEM_MOVEABLE,MaxLen);
				pLoc = (LPSTR)GlobalLock (hOffsets);
				GSSillseek (SHPIDXFid,loc,0);
				Len = GSSilread (SHPIDXFid,pLoc,MaxLen); 
				GlobalUnlock (hOffsets);
				if (Len < 8)
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
			pLen = pLoc + 4;  
			loc = *(LPLONG)pLoc; 
			Len = *(LPLONG)pLen;
			GlobalUnlock (hOffsets);
			flip ((LPSTR)&loc,4); 
			flip ((LPSTR)&Len,4); 
			rtn = loc*2;
			if (rtn < 0)
				rtn = -2;
			return rtn;   
		
		case 1:
			if (!hBlocks)
			{   
				ii=GSSillseek (SHPIDXFid,-2,2);
				GSSilread (SHPIDXFid,(HPSTR)&NumIndexSyms,2);
				GSSillseek (SHPIDXFid,-(NumIndexSyms+1)*2,2);
				GSSilread (SHPIDXFid,(HPSTR)&IndexSyms,NumIndexSyms*2);
				GSSillseek (SHPIDXFid,2,0);
				GSSilread (SHPIDXFid,(HPSTR)&NumBlocks,4);
				GSSilread (SHPIDXFid,(HPSTR)&RecsPerBlock,4);
				hBlocks = GSSiGlobAlloc (1417,GMEM_MOVEABLE,NumBlocks*sizeof(mnmxCor));
				BlockMinMax = (LPMINMAX)GlobalLock (hBlocks); 
				GSSilread (SHPIDXFid,(HPSTR)BlockMinMax,NumBlocks*sizeof(mnmxCor));
                GlobalUnlock (hBlocks); 
                FirstRecLoc = GSSillseek (SHPIDXFid,0,1); 
                _fmemset (BlockInWBounds,0,(int)NumBlocks);
            }
		Top:
			if (record == 652)
				ii=1;
			if (record > NumSHPRecs-1)
				return -1;
/*			{
				char	str[32];
				ltoa (record,str,10);
				SetWindowText (hWndMain,str);
			}*/
			ItemSeg = record; 
			if (!RecsPerBlock)
				return 0;
			BlockID = record / RecsPerBlock;
			loc = FirstRecLoc + record * sizeof(SHPINDEXRECORD); 
			if (UseBounds)
			{   
				if (!BlockInWBounds[BlockID])
				{
					BlockMinMax = (LPMINMAX)GlobalLock (hBlocks); 
					if (BlockInWindow (&BlockMinMax[BlockID],0)) 
						BlockInWBounds[BlockID]=1;
					else
						BlockInWBounds[BlockID]=2;
					GlobalUnlock (hBlocks);
				} 
				if (BlockInWBounds[BlockID] == 2)
				{
					record = (BlockID + 1) * RecsPerBlock;
					goto Top;
				}
			}
			if (!hOffsets || loc < FirstLoc || loc > LastLoc)
			{   
				GSSiGlobFree (&hOffsets);
				hOffsets = GSSiGlobAlloc (1417,GMEM_MOVEABLE,RecsPerBlock*sizeof(SHPINDEXRECORD));
				pLoc = (LPSTR)GlobalLock (hOffsets);
				GSSillseek (SHPIDXFid,loc,0);
				Len = GSSilread (SHPIDXFid,(HPSTR)pLoc,RecsPerBlock*sizeof(SHPINDEXRECORD)); 
				GlobalUnlock (hOffsets);
				if (Len < sizeof(SHPINDEXRECORD))
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
			pIndexRec = (LPSHPINDEXRECORD) pLoc; 
			loc = pIndexRec->Offset;
			if (UseBounds)
			{ 
				if (!GetVisibility (pIndexRec->SymNum) || !BlockInWindow (&pIndexRec->MinMax,0))
				{
					GlobalUnlock (hOffsets); 
					record++;
					goto Top;
				}
//				else
//					HaveSHPSym = pIndexRec->SymNum; doesnt process size rot or color if set here
			}
			GlobalUnlock (hOffsets); 
			CurrentSHPRec = record;
			if (loc < 0)
				loc = -2;
			return loc;
			
		default:
			return 0;
	}
	
}

BOOL ReadSHPRecordHeader (HFILE FidSHP,long RecordOffset,LPMNMXCORD pMinMaxCoord)
{   
	DPOINT		Points[4];  
	UINT		i;
	long		RecLen;
    
	GSSillseek (FidSHP,RecordOffset+4,0);
	BigRead (FidSHP,(HPSTR)&RecLen,4); 
	flip ((LPSTR)&RecLen,4);  
	switch (SHPType)
	{
		case SHPT_POINT:
	        if (BigRead (FidSHP,(HPSTR)&SHPPointRec,sizeof(SHPPointRec)) != sizeof(SHPPointRec))
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
		
      	case SHPT_TEXT:
		case SHPT_ARC: 
		case SHPT_ARCM:   
		case 4:
		case SHPT_POLYGON:
		case SHPT_POLYGONZ:
//			if (RecLen < sizeof(SHPPolyHeader))
//				return FALSE;
            if (BigRead (FidSHP,(HPSTR)&SHPPolyHeader,sizeof(SHPPolyHeader)) != sizeof(SHPPolyHeader))
            	return FALSE; 
			if (!SHPPolyHeader.Type)
				return FALSE;
			if (!SHPPolyHeader.NumPoints)
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
	GetFileMinMax (&CurrentItemMinMax,pMinMaxCoord); 
	return TRUE;
} 

BOOL ValidDPoint (LPDPOINT pVal)
{
	BYTE bytes[8];
	char	CVal[34];
	int	sign,dec;
	
	_fmemmove (bytes,&pVal->x,8);  
	_fstrcpy (CVal,_ecvt (pVal->x,32,&dec,&sign));
	if (_fstrstr (CVal,"QNAN"))
		return FALSE;
	return TRUE;
}

BOOL ReadPGDBRecordHeader (LPMNMXCORD pMinMaxCoord)
{   
	DPOINT		Points[4];  
	UINT		i;
	long		RecLen;  
	char		str[64];     
	HANDLE		hRec;
	HPSTR		pRec;
    
    sprintf (str,"[PGDB.%s]",ShapeFieldName);
    ExpandText (str);  
    hRec = (HANDLE) atol (str);   
    if (!hRec)
    	return FALSE; 
	switch (SHPType)
	{
		case SHPT_POINT:
//	        if (BigRead (FidSHP,&SHPPointRec,sizeof(SHPPointRec)) != sizeof(SHPPointRec))
//	        	return FALSE; 
		    pRec = GlobalLock (hRec);
		    hmemmove ((HPSTR)&SHPPointRec,pRec,sizeof(SHPPointRec));
		    GlobalUnlock (hRec);
			if (!SHPPointRec.Type || !ValidDPoint (&SHPPointRec.Point))
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
		
		case 4:
      	case SHPT_TEXT:
		case SHPT_ARC:
		case SHPT_POLYGON:
		case SHPT_POLYGONZ:
//			if (RecLen < sizeof(SHPPolyHeader))
//				return FALSE;
//            if (BigRead (FidSHP,&SHPPolyHeader,sizeof(SHPPolyHeader)) != sizeof(SHPPolyHeader))
//            	return FALSE; 
		    pRec = GlobalLock (hRec);
		    hmemmove ((HPSTR)&SHPPolyHeader,pRec,sizeof(SHPPolyHeader));
		    GlobalUnlock (hRec);
		    switch (SHPPolyHeader.Type)
		    {
		    	case 0:
		    		return FALSE;
				case SHPT_POLYGON:
				case SHPT_POLYGONZ:
				case SHPT_POLYGONM:
				case SHPT_ARC:
				case SHPT_ARCM: 
				case 536870962:  
				//return FALSE;
					break;
				case SHPT_POLYGON_WITHCURVES:
					break;
				default:
					return FALSE;
			} 
			if (!SHPPolyHeader.NumPoints)
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
	GetFileMinMax (&CurrentItemMinMax,pMinMaxCoord); 
	return TRUE;
} 

BOOL ProcessSHPRecord (HDC hDC,HFILE FidSHP,long RecordNumber)
{
    HPDPOINT    pPoints, pFirstPoint;
    HPLONG      pPartIndex;   
    ULONG		i;
    short		Type, ltag;
    HANDLE		hPoints, hPartIndex;  
    LPINT		pNumPoints;
    long		NumPoints; 
    HPEN		hOldPen=0, hDeletePen=0, hTempPen=0;
	HBRUSH		hOldBrush=0, hTempBrush=0;  
	DPOINT		DPoint,DynSegEndPoint;
	double		size;
    static		short	dbugid=6;
    short		ii;
    static		int	nrecs=0;

	nrecs++;
    if (CurView->ID == dbugid)
    	ii=1;
	ShowValue (hDC,FALSE);  
	InGraphicsProcessor = TRUE;
	ItemIsDeleted = FALSE;
	ItemIsRemoved = FALSE;              	
	InitRecord (hDC); 
    SetSHPParms (RecordNumber);  
    if (!CurrentDesc)
    	goto RtnFalse;
	if (GRStartTime > TimeRangeEnd || GREndTime < TimeRangeBeg)
		goto RtnFalse;
    ltag = _fstrlen (SHPTag);
	SetSymNum (CurrentDesc);
	if (!ProcessRefAndTAG (GetVisibility (CurrentDesc),SHPTag,ltag))
		goto RtnFalse;
	if (!Pick && hDC && Display)
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
	switch (SHPType)
	{
		case SHPT_POINT:
		{
			int		SDCrtn=0;

			if (!SHPPointRec.Type)
				goto RtnFalse;
			DPoint = SHPPointRec.Point; 
			if (ConvertCoord(&DPoint,0,1)) 
				goto RtnFalse;
	        lpDCurPoints = &DPoint;  
			CurrentPoint = CurPointLocD = DPoint;
	        LastElementBeginPoint = LastElementEndPoint = DPoint;
	        nPnts = 1; 
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
						
				if (CopyRec)
				{   
				    AddPointToBuffer (CurPointLocD,CurrentRefno,0,CurrentDesc,10, PTRot,0,0,0,
									  CurrentTAG,CurrentUDI,-1,-1,-1,TRUE,&hUpdateBuf,&lUpdateBuf);
				}
				else
				{
	 				if (CurrentDesc > 0 && CurrentDesc < 3201)   
	 				{
						if (TSize)
							CurView->CurVisType[CurrentDesc]=5;
						else
							CurView->CurVisType[CurrentDesc]=4;
					}
					HighlightPointSym=FALSE;
					if (SHPColor >= 0)
					{  
						GlobalColors[0]=SHPColor;  
						HaveVarFillColor = TRUE;
					}
					if (!GetTypeVisibility(6) && SymbolIsVisible (iDesc)) 
					{
						CurPointSize = 10*DeviceToScreenFactor;
						iDesc = InvisiblePointSymbol;
					}
					if ((SDCrtn = SetDisplayChar (hDC,GF_POINT,CurrentRefno,CurrentDesc,CurrentTAG,CurrentUDI)) > 0)
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
						{
							long	DisplayedWidth=0;

							DisplayPointItem (hDC,CurPointLoc,size,PTRot,iDesc,&DisplayedWidth);
							CurView->MaxSymbolWidth = max (CurView->MaxSymbolWidth,DisplayedWidth);
							CurView->MaxFileDisplayedPointWidth[FileNum] = max (CurView->MaxFileDisplayedPointWidth[FileNum],(DisplayedWidth/ FileDistToWinDist)-(((long)CurrentItemMinMax.xmx)-CurrentItemMinMax.xmn));
						}
		       			HaveTXLoc = 1;
					}
				}
			}
   			TXLoc = CurPointLocD; 
			if (SDCrtn < 0)
       			HaveTXLoc = 1;
		}
		break;
		
       	case SHPT_TEXT:
		case SHPT_ARC: 
		case SHPT_ARCM:
		    if (CurrentDesc > 0 && CurrentDesc < 3201)
				CurView->CurVisType[CurrentDesc]=2; 
    		CurrentType = GF_POLYLINE; 
    		if (!GetTypeVisibility(TYPE_LINECURVE))
    			break;
			goto DoPoly;  
		case 4:
		case SHPT_POLYGON:
		case SHPT_POLYGONZ: 
		case SHPT_POLYGONM:  
    		if (!GetTypeVisibility(TYPE_AREA))
    			break;
    		CurrentType = GF_AREA; 
		    if (CurrentDesc > 0 && CurrentDesc < 3201)
				CurView->CurVisType[CurrentDesc]=3; 
DoPoly:
	        if (SHPPolyHeader.NumPoints > USHRT_MAX)
	        	ii=1;
	        	//goto RtnFalse;
	        if (!SHPPolyHeader.NumPoints)
	        	goto RtnFalse;
	        if (!SHPPolyHeader.Type)
	        	goto RtnFalse;
	        nPoly = SHPPolyHeader.NumParts; 
	        if (nPoly > 1)
	        	ii=1; 
	        NumPoints = SHPPolyHeader.NumPoints; 
	        if (SHPHeader.ShapeType == SHPT_POLYGON || SHPHeader.ShapeType == SHPT_POLYGONZ || SHPHeader.ShapeType == SHPT_POLYGONM)
	        	NumPoints = NumPoints+nPoly-1;
	        hPartIndex = GSSiGlobAlloc (1418,GMEM_MOVEABLE,sizeof(long)*(nPoly+1));
	        hPolyPartLen = GSSiGlobAlloc (1419,GMEM_MOVEABLE,sizeof(int)*(nPoly+1));
			hPoints = GSSiGlobAlloc (1420,GMEM_MOVEABLE,sizeof(DPOINT)*NumPoints); 
	        pPartIndex = (HPLONG)GlobalLock (hPartIndex); 
	        BigRead (FidSHP,(HPSTR)pPartIndex,(long)nPoly*sizeof(long)); //pPartIndex[0]
	        pPartIndex[SHPPolyHeader.NumParts]= SHPPolyHeader.NumPoints;
	                    
            pPoints = pFirstPoint = (LPDPOINT)GlobalLock (hPoints); 
            pNumPoints = (LPINT)GlobalLock (hPolyPartLen);
	        for (i=0;i<nPoly;i++)
	        {   
	            long    numpoints, startpoint,ii; 
	                        
	            startpoint = *pPartIndex++;
	            numpoints = *pPartIndex-startpoint; 
	       //     if (numpoints > (long)USHRT_MAX)
	       //     	MessageBox (0,"Shape file record contains too many points",0,MB_ICONEXCLAMATION);
	            *pNumPoints++ = numpoints; 
	            if (BigRead (FidSHP,(HPSTR)pPoints,numpoints * sizeof(DPOINT)) != numpoints * sizeof(DPOINT))
	            	goto RtnFalse;
	            pPoints += numpoints;   
	            if (i && (SHPHeader.ShapeType == SHPT_POLYGON || SHPHeader.ShapeType == SHPT_POLYGONZ || SHPHeader.ShapeType == SHPT_POLYGONM))
	            	*pPoints++ = *pFirstPoint; 
	        } 
	        GlobalUnlock (hPoints); 
	        GlobalUnlock (hPolyPartLen);
	        GSSiGlobUlFree (&hPartIndex);  
			pPoints = (HPDPOINT)GlobalLock (hPoints);
			for (i=0;i<NumPoints;i++)
				ConvertCoord(&pPoints[i],0,1);
			if (SHPHeader.ShapeType == SHPT_ARC || SHPHeader.ShapeType == SHPT_ARCM) 
			{
				if (Pick)
				{   
					pNumPoints = (LPINT)GlobalLock (hPolyPartLen);
	        		for (i=0;i<nPoly;i++)
	        		{ 
	        			HPDPOINT	SavelpDCurPoints = lpDCurPoints;
						int	PolyID=i;

						if (nPoly > 1)
							PolyID++;
						nPnts = *pNumPoints;  
						PickPolylineD (pPoints,nPnts,PolyID,2,0,0,0);
						pPoints+=*pNumPoints++; 
				    }   
			        GlobalUnlock (hPolyPartLen); 
				}
				else if (CopyRec)  
				{
					
					pNumPoints = (LPINT)GlobalLock (hPolyPartLen);
					if (nPoly > 1)
					{
					ii=1;
					}
					AddPolyToBuffer (nPoly,pNumPoints,&hPoints,TYPE_POLYLINE,CurrentRefno,0,-1,CurrentDesc,0,CurrentTAG,CurrentUDI,
                                     -1,-1,0,0,0,0,0,TRUE,&hUpdateBuf,&lUpdateBuf); 
                    GlobalUnlock (hPolyPartLen);
                }
				else
				{    
					lpDCurPoints = pPoints;
					nPnts = nCurPoints = NumPoints;

					if (PolyInMaskAreaFileCoord (GF_LINE,&NumPoints,0,0,&pPoints,TRUE))
					{   
						HPEN hNewPen=0;
						
						hOldPen  = SelectObject(hDC,GetStockObject(BLACK_PEN));
						TempLineColor = SHPColor; 
						TempLineWidth = SHPWidth; 
						if (TempLineColor >= 0 || TempLineWidth != 0)
						{
							hNewPen = CreatePen (PS_SOLID,(short)IDNINT(TempLineWidth*DeviceToScreenFactor),TempLineColor);
							SelectObject(hDC,hNewPen);
						}
//						if (SetDisplayChar (hDC,GF_LINE,CurrentRefno,CurrentDesc,CurrentTAG,CurrentUDI))
//						{   
							pNumPoints = (LPINT)GlobalLock (hPolyPartLen);
			        		for (i=0;i<nPoly;i++)
			        		{ 
			        			HPDPOINT	SavelpDCurPoints = lpDCurPoints;
  
//								GWPolylineD (hDC,lpDCurPoints,*pNumPoints,CurrentDesc); 
//								lpDCurPoints+=*pNumPoints++;
			        			nPnts = *pNumPoints;  
			        			InDynamicSegmentation = FALSE;  
			        			DoDynamicFixedSegmentation (-1,0,0);
								while (nPnts > 1)
								{ 
									if (!WantSegmentID || WantSegmentID-1 == i)
									{
										if (SetDisplayChar (hDC,GF_LINE,CurrentRefno,CurrentDesc,CurrentTAG,CurrentUDI) > 0)
										{   
											GWPolylineD (hDC,lpDCurPoints,NumDynSegPoints,CurrentDesc); 
										}
									}
									else
										NumDynSegPoints = nPnts;
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
//						} 
						if (hNewPen)
						{
							SelectObject(hDC,GetStockObject(BLACK_PEN)); 
							GSSiDeleteObject (&hNewPen);
						}
					}
				}
			}
			else
			{   
				BOOL	ShowBorder = GetBit (5,(LPSTR)&CurVis->WantType[7]); 
				HPEN	hBorderPen=0;
				
				if (Pick)
				{   
					PickPolygonD (pPoints,NumPoints,9999999,0);
				}
				else if (CopyRec)  
				{
					pNumPoints = (LPINT)GlobalLock (hPolyPartLen);//pNumPoints[1]
					if (nPoly > 1)
					{
						HANDLE	hhPoly = GSSiGlobAlloc ( 418,GMEM_MOVEABLE,sizeof(HANDLE)*nPoly); 
						LPHANDLE phPoly = (LPHANDLE)GlobalLock (hhPoly);
                        HPDPOINT	pPoints1=(HPDPOINT)GlobalLock (hPoints), pPoints2;
                        
	                    for (i=0;i<nPoly;i++)
	                    {   
	                   		phPoly[i] = GSSiGlobAlloc ( 420,GMEM_MOVEABLE,sizeof(DPOINT)*(long)pNumPoints[i]);
		                    pPoints2= (HPDPOINT)GlobalLock (phPoly[i]);  
		                    hmemmove ((HPSTR)pPoints2,(HPSTR)pPoints1,sizeof(DPOINT)*(long)pNumPoints[i]);
	                        GlobalUnlock (phPoly[i]); 
	                        pPoints1 += pNumPoints[i];
	                        if (i)
	                        	pPoints1++;
	                    } 
	                    GlobalUnlock (hPoints);
						AddPolyToBuffer (nPoly,pNumPoints,phPoly,TYPE_AREA,CurrentRefno,0,-1,CurrentDesc,0,CurrentTAG,CurrentUDI,
	                                     -1,-1,0,0,0,0,0,TRUE,&hUpdateBuf,&lUpdateBuf); 
		                for (i=0;i<nPoly;i++)
		                	GSSiGlobFree (&phPoly[i]);
	                    GSSiGlobUlFree (&hhPoly);
					}
					else
						AddPolyToBuffer (nPoly,pNumPoints,&hPoints,TYPE_AREA,CurrentRefno,0,-1,CurrentDesc,0,CurrentTAG,CurrentUDI,
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

					if (PolyInMaskAreaFileCoord (GF_AREA,&NumPoints,0,0,&pPoints,TRUE))
					{
						if (SetDisplayChar (hDC,GF_AREA,CurrentRefno,CurrentDesc,CurrentTAG,CurrentUDI) > 0)
						{   
							BOOL	DoBorder = GetBit (5,(LPSTR)&CurVis->WantType[7]);
							
							if (Display)
							{
								int	StartPoly = 0;

								if (nPoly > 1 && !ShowLinkLines)
									SelectObject(hDC,h0Pen); 
								if (FillAreas && (GetBit (7,(LPSTR)&CurVis->WantType[7]) ||
												  GetBit (6,(LPSTR)&CurVis->WantType[7])))
								{
									hDeletePen = GWPolygonD (hDC,pPoints, NumPoints, nPoly, hPolyPartLen,CurrentDesc,ShowBorder);
									StartPoly = 1;
								}
								else
								{
									DoBorder = TRUE;
									if (!StartPoly)
									    SetAreaPenAndBrush (hDC,0,CurrentDesc,ItemIsHighlighted,ShowBorder,&hDeletePen,0);
								}
								if ((DoBorder || nPoly > 1) && (hBorderPen || hDeletePen))
								{   
									SetROP2(hDC,DisplayRasterOpt);
									if (hDeletePen)
										SelectObject(hDC,hDeletePen); 
									else
										SelectObject(hDC,hBorderPen); 
									pNumPoints = (LPINT)GlobalLock (hPolyPartLen);
					        		for (i=0;i<nPoly;i++)
					        		{   
										if (i >= StartPoly)
											GWPolylineD (hDC,lpDCurPoints,*pNumPoints,0); 
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
		break;
		
		case SHPT_MULTIPOINT:
		break;
	}
	if (hOldBrush)
		SelectObject (hDC,hOldBrush);
	if (hOldPen)
		SelectObject (hDC,hOldPen); 
   	if (hDeletePen != h0Pen)
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

long ReadSHPHeader (HFILE FidSHP,LPMNMXCORD pMinMaxCoord)
{
	DPOINT		Points[4];  
	UINT		i;  
	char		ShpType[16]="";
	
	BigRead (FidSHP,(HPSTR)&SHPHeader,(UINT)sizeof(SHPHeader));  
	switch (SHPHeader.ShapeType)
	{
		case SHPT_NULL: 
			_fstrcpy (ShpType,"NULL");
			break;
		case SHPT_MULTIPOINT:
			_fstrcpy (ShpType,"MULTIPOINT");
			break;
		case SHPT_POINTZ:
			_fstrcpy (ShpType,"POINTZ");
			break;
		case SHPT_MULTIPOINTZ:
			_fstrcpy (ShpType,"MULTIPOINTZ");
			break;
		case SHPT_POINTM:
			_fstrcpy (ShpType,"POINTM");
			break;
		case SHPT_MULTIPOINTM:
			_fstrcpy (ShpType,"MULTIPOINTM");
			break;
		case SHPT_MULTIPATCH:
			_fstrcpy (ShpType,"MULTIPATCH");
			break;
	}
	if (*ShpType) 
	{   
		char	mess[64];
			
		sprintf (mess,"Shape file type %s not yet implemented",ShpType);
		GSSiMessageBox (mess,0,MB_ICONEXCLAMATION);
		return FALSE;
	}
	flip ((LPSTR)&SHPHeader.FileCode,4);
	flip ((LPSTR)&SHPHeader.FileLength,4);  
	if (pMinMaxCoord)
	{
		DBoundsInit (pMinMaxCoord);
		Points[0].x = SHPHeader.Xmin;   
		Points[0].y = SHPHeader.Ymin;   
		Points[1].x = SHPHeader.Xmin;   
		Points[1].y = SHPHeader.Ymax;   
		Points[2].x = SHPHeader.Xmax;   
		Points[2].y = SHPHeader.Ymax;   
		Points[3].x = SHPHeader.Xmax;   
		Points[3].y = SHPHeader.Ymin; 
		for (i=0;i<4;i++)
		{
			AddDPointToMinMax (&Points[i],pMinMaxCoord); 
		} 
	}
	return SHPHeader.ShapeType;
}   

   

BOOL OpenPGDB (LPSTR DBName,LPSTR Table,LPSTR SQL)
{   
	BOOL	rtn=TRUE;
	
    CloseDataFile (TRUE, &PGDBHandle); 
    hSHPDBF = 0; 
	if (DBName)
    {  
    	BOOL	SaveUseLongVarBinary = UseLongVarBinary;  
    	
    	UseLongVarBinary = TRUE;   
    	if (Table)
        	sprintf (PGDBName,"PGDB=ODBC|MS Access Database;DBQ=%s|%s",DBName,Table);
        else
        	sprintf (PGDBName,"PGDB=ODBC|MS Access Database;DBQ=%s",DBName);
        rtn = OpenDataFile (PGDBName,SQL,BT_READ,&PGDBHandle); 
        UseLongVarBinary = SaveUseLongVarBinary;    
        if (rtn)
        	hSHPDBF = PGDBHandle;
    }
    return rtn;
}   

void SetPGDB_SQL (LPSTR SQL)
{
	LPOPENSQLDATA	SQLPtr;
	LPOPENFILEDATA	FilePtr;

	_fstrcpy (PGDB_SQL,SQL);
	if (PGDBHandle)
	{
		SQLPtr = (LPOPENSQLDATA)GlobalLock(PGDBHandle);
		FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
		ProcessFileSQL (SQLPtr,FilePtr,SQL);   
		SQLPtr->lastreadtime = 0;                                     
		GlobalUnlock (SQLPtr->OFHandle);
		GlobalUnlock (PGDBHandle);
	}
	return;
}

BOOL OpenPGDBFileIndex (LPSTR DBNameIN,LPMNMXCORD WBounds)
{
	char	DBName[512],str[512];
    LPSTR	pPar;
    
	_fstrcpy (DBName,DBNameIN);
    if ((pPar = _fstrrchr (DBName,'(')))
    {
    	*pPar++ = 0; 
    	_fstrcpy (PGDBTable,pPar);
    	*LastChr (PGDBTable) = 0;
    }
    else
    	*PGDBTable = 0;
    if (!*PGDBTable)
    	return FALSE;  
    _fstrcpy (LastSHPFile,DBNameIN);
    if (WBounds)
    {   
    	BOOL	st;  
    	long	MinGx,MaxGx,MinGy,MaxGy;
		LPSTR	pFields;
    	
    	sprintf (str,"TABLENAME = '%s'",PGDBTable);
		st = OpenPGDB (DBName,"GDB_GeomColumns",str); 
		hSelectFields = GSSiGlobAlloc (GMEM_MOVEABLE,0,1024);
		pFields = GlobalLock (hSelectFields);
		strcpy (pFields,"TableName,FieldName,ShapeType,ExtentLeft,ExtentRight,ExtentBottom,ExtentTop,IdxOriginX,IdxOriginY,IdxGridSize");
		GlobalUnlock (hSelectFields);
		_fstrcpy (str,"[PGDB.IdxOriginX]");
		ExpandText (str);
		PGDBGridOrigX = atof (str);
		_fstrcpy (str,"[PGDB.IdxOriginY]");
		ExpandText (str);
		PGDBGridOrigY = atof (str);
		_fstrcpy (str,"[PGDB.IdxGridSize]");
		ExpandText (str);
		PGDBGridSize = atof (str); 
		MinGx = (WBounds->xmn * MFT)/PGDBGridSize;
		MaxGx = (WBounds->xmx * MFT)/PGDBGridSize;
		MinGy = (WBounds->ymn * MFT)/PGDBGridSize;
		MaxGy = (WBounds->ymx * MFT)/PGDBGridSize;
		sprintf (str,  
					"(SELECT * FROM %s_SHAPE_Index INNER JOIN %s ON %s_SHAPE_Index.IndexedObjectID = %s.ObjectID WHERE MaxGX >= %ld AND MinGX <= %ld AND MaxGY >= %ld AND MinGY <= %ld)",
					PGDBTable,PGDBTable,PGDBTable,PGDBTable,MinGx,MaxGx,MinGy,MaxGy);	
		SetPGDB_SQL (str);	
		return OpenPGDB (DBName,0,PGDB_SQL);
    } 
    else
		return OpenPGDB (DBName,PGDBTable,PGDB_SQL);
}  

BOOL PGDBTableIsText (LPSTR DBName,LPSTR TableName)
{
    BOOL	rtn=FALSE;
    
	{ 
		LPOPENFILEDATA	FilePtr;
		LPOPENSQLDATA	SQLPtr; 
		LPFIELDINFO		pField; 
		UINT	j;
			
		SQLPtr = (LPOPENSQLDATA)GlobalLock (PGDBHandle);
        FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle);  
        pField = &FilePtr->FldInfo;
        for (j=0;j<FilePtr->NumFields;j++,pField++)
        	if (!_fstricmp (pField->name,"AnnotationClassID"))
        		rtn=TRUE;
        GlobalUnlock (SQLPtr->OFHandle); 
		GlobalUnlock (PGDBHandle);
	}
    return rtn;
}

long ReadPGDBHeader (LPSTR DBNameIN,LPMNMXCORD pMinMaxCoord)
#if ENABLETRACE
{GSSiEnterProg (1376);
#endif
{ 
	DPOINT		Points[4];  
	UINT		i;  
	char		ShpType[16]="";
	char	DBName[512],SQL[128],str[128];
	LPSTR	pPar; 
	short	ShapeType; 
	double	Xmin,Xmax,Ymin,Ymax;
	LPSTR	pFields;

	_fstrcpy (DBName,DBNameIN); 
	ExpandText (DBName);
    if ((pPar = _fstrrchr (DBName,'(')))
    {
    	*pPar++ = 0; 
    	_fstrcpy (PGDBTable,pPar);
    	*LastChr (PGDBTable) = 0;
    }
    else
    	*PGDBTable = 0;
    if (!*PGDBTable)
    	goto RtnFalse;
    sprintf (SQL,"TableName = '%s'",PGDBTable);

	if (!OpenPGDB (DBName,"GDB_GeomColumns",SQL))
		goto RtnFalse;
	hSelectFields = GSSiGlobAlloc (GMEM_MOVEABLE,0,1024);
	pFields = GlobalLock (hSelectFields);
	strcpy (pFields,"TableName,FieldName,ShapeType,ExtentLeft,ExtentRight,ExtentBottom,ExtentTop");
	GlobalUnlock (hSelectFields);
	if (!FetchDBRec (PGDBHandle))
		goto RtnFalse;
	_fstrcpy (str,"[PGDB.ShapeType]"); 
	ExpandText (str);
	ShapeType = atoi (str);
	_fstrcpy (ShapeFieldName,"[PGDB.FieldName]"); 
	ExpandText (ShapeFieldName);
	switch (ShapeType)
	{
		case SHPT_NULL: 
			_fstrcpy (ShpType,"NULL");
			break;
		case SHPT_MULTIPOINT:
			_fstrcpy (ShpType,"MULTIPOINT");
			break;
		case SHPT_POINTZ:
			_fstrcpy (ShpType,"POINTZ");
			break;
		case SHPT_MULTIPOINTZ:
			_fstrcpy (ShpType,"MULTIPOINTZ");
			break;
		case SHPT_POINTM:
			_fstrcpy (ShpType,"POINTM");
			break;
		case SHPT_MULTIPOINTM:
			_fstrcpy (ShpType,"MULTIPOINTM");
			break;
		case SHPT_MULTIPATCH:
			_fstrcpy (ShpType,"MULTIPATCH");
			break;
	}
	if (*ShpType) 
	{   
		char	mess[64];
			
		sprintf (mess,"Shape file type %s not yet implemented",ShpType);
		GSSiMessageBox (mess,0,MB_ICONEXCLAMATION);
		OpenPGDB (0,0,0);
		goto RtnFalse;
	}
	if (pMinMaxCoord)
	{
		DBoundsInit (pMinMaxCoord); 
		_fstrcpy (str,"[PGDB.ExtentLeft]");
		ExpandText (str);
		Xmin = atof (str);
		_fstrcpy (str,"[PGDB.ExtentRight]");
		ExpandText (str);
		Xmax = atof (str);
		_fstrcpy (str,"[PGDB.ExtentBottom]");
		ExpandText (str);
		Ymin = atof (str);
		_fstrcpy (str,"[PGDB.ExtentTop]");
		ExpandText (str);
		Ymax = atof (str);
		if (Xmin > Xmax || Ymin > Ymax)
		{
			OpenPGDB (0,0,0);
			goto RtnFalse;
		}
		Points[0].x = Xmin;   
		Points[0].y = Ymin;   
		Points[1].x = Xmin;   
		Points[1].y = Ymax;   
		Points[2].x = Xmax;   
		Points[2].y = Ymax;   
		Points[3].x = Xmax;   
		Points[3].y = Ymin; 
		for (i=0;i<4;i++)
		{
			AddDPointToMinMax (&Points[i],pMinMaxCoord); 
		} 
	}
	if (OpenPGDB (DBName,PGDBTable,""))
	{ 
		if (PGDBTableIsText (DBName,PGDBTable))
			ShapeType = SHPT_TEXT;  
	}
	OpenPGDB (0,0,0); 
{
#if ENABLETRACE
GSSiExitProg (1376);
#endif
	return ShapeType;
}
RtnFalse:
{
#if ENABLETRACE
GSSiExitProg (1376);
#endif
	return 0;
}
#if ENABLETRACE
}
#endif
}


long ListPGDBTables (LPSTR DBNameIN,HWND hWndDlg,UINT ListCntl,int ListType)
{
	UINT		i;  
	char		ShpType[16]="";
	char	DBName[512],str[256], TableName[128], TableType[64];
	LPSTR	pPar, pTab; 
	short	ShapeType; 
	double	Xmin,Xmax,Ymin,Ymax;  
	short	Num=0;        
	long	NumRows; 
	int   	TabStops[3]={150,200,250};

   	SendDlgItemMessage (hWndDlg,ListCntl,LB_SETTABSTOPS,3,(LPARAM)&TabStops); 
	_fstrcpy (DBName,DBNameIN); 
	ExpandText (DBName);
    if ((pPar = _fstrrchr (DBName,'(')))
    	*pPar++ = 0; 
	if (!OpenPGDB (DBName,"GDB_GeomColumns",""))
		return 0;
	while (FetchDBRec (PGDBHandle))
	{
		_fstrcpy (str,"[PGDB.ShapeType]"); 
		ExpandText (str);
		ShapeType = atoi (str);
		_fstrcpy (ShapeFieldName,"[PGDB.FieldName]"); 
		ExpandText (ShapeFieldName);    
		_fstrcpy (TableName,"[PGDB.TableName]"); 
		ExpandText (TableName);    
		*ShpType = 0;  
		
		switch (ShapeType)
		{
			case 1:
				_fstrcpy (TableType,"Point");
				break;
			case 3:
				_fstrcpy (TableType,"Line");  
				break;
			case 4:
				_fstrcpy (TableType,"Area");
				break;
			default:
				sprintf (TableType,"Unknown(%i)",ShapeType);
				break;
		}
		Num++;   
		sprintf (str,"%s\t%s",TableName,TableType);
		SendDlgItemMessage ((HWND)hWndDlg,ListCntl,LB_ADDSTRING,0,(LPARAM) str); 
	}
	OpenPGDB (0,0,0); 
	for (i=0;i<Num;i++)
	{ 
        SendDlgItemMessage(hWndDlg,ListCntl,LB_GETTEXT,i,(LPARAM)str);   
        pTab = _fstrchr (str,'\t');
        *pTab++ = 0;      
        _fstrcpy (TableType,pTab); 
		if (OpenPGDB (DBName,str,""))
		{
			NumRows = NumSQLRows (PGDBHandle);  
	        if (PGDBTableIsText (DBName,str))
		   		_fstrcpy (TableType,"Text"); 
		}
		else  
		{
			_fstrcpy (TableType,"Unknown");
			NumRows = 0;
		}
		OpenPGDB (0,0,0);  
		sprintf (_fstrchr(str,0),"\t%s\t%ld",TableType,NumRows);
		SendDlgItemMessage ((HWND)hWndDlg,ListCntl,LB_DELETESTRING,i,(LPARAM)0); 
		SendDlgItemMessage ((HWND)hWndDlg,ListCntl,LB_INSERTSTRING,i,(LPARAM) str); 
	}
	return Num;
}

BOOL FAR PASCAL SELECTPGDBMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (1107);
#endif
{   
	static	HANDLE	hSaveBM;
	char	str[260], str2[512]; 
    int     TabStops[2]={2000,2100};
	LPSTR pBS;
	
 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (1107);
#endif
 	return (BRtn);
}
 switch(Message)
   {
    case WM_INITDIALOG: 
                       
        SetDlgItemText (hWndDlg,IDC_LISTHEADER,"Table Name                                               Type               Count");
		SetDlgItemText (hWndDlg,IDC_STARTREF,"1");
		ListPGDBTables (PGDBFile,hWndDlg,IDC_LIST,1);
		
		 break;                              
    case WM_COMMAND:
         switch(LOWORD(wParam))
         {  
            case IDCANCEL: 
                GSSiEndDialog(hWndDlg, FALSE,hSaveBM);
            break; 
            
            case IDC_LIST:
				switch(HIWORD(wParam))
				{
					case LBN_SELCHANGE:
						EnableWindow (GetDlgItem(hWndDlg,IDOK),TRUE);  
						break;
				 	case LBN_DBLCLK:
				     	PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);
				 	break;
				}
			break;
			 
            case IDOK: 
            {
                HANDLE	hItems;
                LPINT	lpItems;
				short	nItems = GetLBSelectedItems (hWndDlg,IDC_LIST,&hItems);

				if (!nItems)
					break; 
                lpItems = (LPINT) GlobalLock(hItems);
				if (nItems == 1)  
				{
		            SendDlgItemMessage(hWndDlg,IDC_LIST,LB_GETTEXT,*lpItems,(LPARAM)str);  
		            if ((pBS = _fstrchr (str,'\t')))
		            	*pBS=0;  
		            SubstituteDL (str);
		            ExpandText (str);  
		            sprintf (_fstrchr(PGDBFile,0),"(%s)",str);
				}
				else
				{   
					HFILE	FidFL;
					char	FileList[MAX_PATH], File[MAX_PATH];
					UINT	i;
					LPSTR	pBS;
					long	StartRef, Count;
					BOOL	Error;

					StartRef = GetDlgItemInt(hWndDlg,IDC_STARTREF,&Error,TRUE); 
					_fstrcpy (FileList,PGDBFile);   
					_fstrcpy (File,PGDBFile);
					pBS = _fstrrchr (FileList,'\\');
					_fstrcpy (pBS+1,"filelist.txt");
					FidFL=GSSiOpenFile (FileList,0,OF_CREATE);
					for (i=0; i<nItems; i++,lpItems++)
					{   
			            SendDlgItemMessage(hWndDlg,IDC_LIST,LB_GETTEXT,*lpItems,(LPARAM)str);  
			            if ((pBS = _fstrrchr (str,'\t')))
			            	*pBS++=0;
						Count = atol (pBS);
			            if ((pBS = _fstrchr (str,'\t')))
			            	*pBS++=0;
			            sprintf (PGDBFile,"%s(%s)\t%ld",File,str,StartRef);
			            ExpandText (PGDBFile);  
			            SubstituteDL (PGDBFile); 
			            ExpandText (PGDBFile);  
			            fputstring (PGDBFile,FidFL);
						StartRef += Count;
					}
					GSSiClose (FidFL);
					_fstrcpy (PGDBFile,FileList);  
		            SubstituteDL (PGDBFile); 
		            ExpandText (PGDBFile);  
				}
				GSSiGlobUlFree (&hItems);
                GSSiEndDialog(hWndDlg, TRUE,hSaveBM);
            }
            break;    
            
         }
         break; 

    default:
{
#if ENABLETRACE
GSSiExitProg (1107);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (1107);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
} 

BOOL GetPGDBTable (HWND hWnd,LPSTR File)
{
	BOOL	rtn;
	FARPROC lpfnSELECTITEMSMsgProc;
     
    _fstrcpy (PGDBFile,File); 
	lpfnSELECTITEMSMsgProc = MakeProcInstance((FARPROC)SELECTPGDBMsgProc, hInst);
	rtn = DialogBox(hInst, (LPSTR)"SELECTPGDB",hWnd, lpfnSELECTITEMSMsgProc);
	FreeProcInstance(lpfnSELECTITEMSMsgProc); 
	if (rtn)
	    _fstrcpy (File,PGDBFile); 
	return rtn;
}   

int	GetWords (LPSTR txt)
{
	int	nWords=0; 
	LPSTR	pLoc=txt;
	LPSTR	pEnd;
	
	pLoc = FirstNonBlank (pLoc);
	while (*pLoc)
	{
		pEnd = NextBlank (pLoc);
		if (*pEnd)
			*pEnd++ = 0; 
		_fstrcpy (Words[nWords],pLoc);
		WordLen[nWords++] = _fstrlen (pLoc);
		pLoc = pEnd;
		if (*pLoc)
			pLoc = FirstNonBlank (pLoc);
	}
	return nWords;
} 

BOOL ProcessPGDBRecord (HDC hDC,long RecordNumber)
#if ENABLETRACE
{GSSiEnterProg (1375);
#endif
{ 
    HPDPOINT    pPoints, pFirstPoint;
    HPLONG      pPartIndex;   
    ULONG		i;
    short		Type, ltag;
    HANDLE		hPoints, hPartIndex;  
    LPINT		pNumPoints;
    long		NumPoints; 
    HPEN		hOldPen=0, hDeletePen=0, hTempPen=0;
	HBRUSH		hOldBrush=0, hTempBrush=0;  
	DPOINT		DPoint,DynSegEndPoint;
	double		size;
    static		short	dbugid=6;
    short		ii;
    HPSTR		pRec,pElement;
    HANDLE		hRec,hElement=0;
    char		str[512];  
    long		recloc=sizeof(SHPPOLYHEADER); 
    long		BinSizeR,BinSizeE;  
    static	long	DebugRecNum=2679;
	BOOL	save;
    
    if (CurView->ID == dbugid)
    	ii=1; 
    if (RecordNumber == DebugRecNum)
    	ii=1;
    SHPHeader.ShapeType = SHPType;//SHPT_POLYGONZ;
    sprintf (str,"[PGDB.%s]",ShapeFieldName);
    ExpandText (str);
    hRec = (HANDLE) atol (str); 
//    BinSizeR = GlobalSize (hRec);
    pRec = GlobalLock (hRec);  
    _fstrcpy (str,"[PGDB.Element]");
    ExpandText (str);
	hElement = (HANDLE) atol (str);      
	if (hElement)
	{
		BinSizeE = GlobalSize (hElement);
		pElement = GlobalLock (hElement);
		save=FALSE;
		if (save)
		{
			HFILE FidDB=GSSiOpenFile ("c:\\temp.bin",0,OF_CREATE);
			BigWrite (FidDB,pElement,BinSizeE,-1);
			GSSiClose (FidDB);
		}
	}
	ShowValue (hDC,FALSE);  
	InGraphicsProcessor = TRUE;
	ItemIsDeleted = FALSE;
	InitRecord (hDC); 
    SetSHPParms (RecordNumber);  
	ItemSeg = RecordNumber;  
	if (CurView->PassID && CurView->PassID < 4)
	{
		if ((SHPType == SHPT_POLYGON || SHPType == SHPT_POLYGON_PGDB) && CurView->PassID != 2)
			goto RtnFalse;  
		if ((SHPType == SHPT_POINT || SHPType == SHPT_ARC) && CurView->PassID == 2)
			goto RtnFalse;
		if (SHPType == SHPT_TEXT && CurView->PassID != 3)
			goto RtnFalse;  
	}
    if (!CurrentDesc)
    	goto RtnFalse;
	if (GRStartTime > TimeRangeEnd || GREndTime < TimeRangeBeg)
		goto RtnFalse;
    ltag = _fstrlen (SHPTag);
	SetSymNum (CurrentDesc);
	if (!ProcessRefAndTAG (GetVisibility (CurrentDesc),SHPTag,ltag))
		goto RtnFalse;
	if (!Pick && hDC && Display)
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
	switch (SHPType)
	{
		case SHPT_POINT:
		{
			int	SDCrtn = 0;

			if (!SHPPointRec.Type)
				goto RtnFalse;
			DPoint = SHPPointRec.Point; 
			if (ConvertCoord(&DPoint,0,1)) 
				goto RtnFalse;
	        lpDCurPoints = &DPoint;  
			CurrentPoint = CurPointLocD = DPoint;
	        LastElementBeginPoint = LastElementEndPoint = DPoint;
	        nPnts = 1; 
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

				if (CopyRec)
				{   
				    AddPointToBuffer (CurPointLocD,CurrentRefno,0,CurrentDesc,10, PTRot,0,0,0,
									  CurrentTAG,CurrentUDI,-1,-1,-1,TRUE,&hUpdateBuf,&lUpdateBuf);
				}
				else
				{
	 				if (CurrentDesc > 0 && CurrentDesc < 3201)   
	 				{
						if (TSize)
							CurView->CurVisType[CurrentDesc]=5;
						else
							CurView->CurVisType[CurrentDesc]=4;
					}
					HighlightPointSym=FALSE;
					if (SHPColor >= 0)
					{  
						GlobalColors[0]=SHPColor;  
						HaveVarFillColor = TRUE;
					}
					if (!GetTypeVisibility(6) && SymbolIsVisible (iDesc)) 
					{
						CurPointSize = 10*DeviceToScreenFactor;
						iDesc = InvisiblePointSymbol;
					}
					if ((SDCrtn = SetDisplayChar (hDC,GF_POINT,CurrentRefno,CurrentDesc,CurrentTAG,CurrentUDI)) > 0)
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
						{
							long	DisplayedWidth=0;

							DisplayPointItem (hDC,CurPointLoc,size,PTRot,iDesc,&DisplayedWidth);
							CurView->MaxSymbolWidth = max (CurView->MaxSymbolWidth,DisplayedWidth);
							CurView->MaxFileDisplayedPointWidth[FileNum] = max (CurView->MaxFileDisplayedPointWidth[FileNum],(DisplayedWidth/ FileDistToWinDist)-(((long)CurrentItemMinMax.xmx)-CurrentItemMinMax.xmn));
						}
						HaveTXLoc = 1;
					} 
				}
			}
   			TXLoc = CurPointLocD; 
       		if (SDCrtn < 0)
				HaveTXLoc = 1;
		}
		break;
		
       	case SHPT_TEXT: 
//       		GetValFromOpenFiles ("pgdb.TEXTSTRING",str); 
//       		GetValFromOpenFiles ("pgdb.FONTNAME",str); 
//       		GetValFromOpenFiles ("pgdb.ANGLE",str);    
			goto DoPoly;
		break;
		case SHPT_ARC:
		    if (CurrentDesc > 0 && CurrentDesc < 3201)
				CurView->CurVisType[CurrentDesc]=2; 
    		CurrentType = GF_POLYLINE; 
    		if (!GetTypeVisibility(TYPE_LINECURVE))
    			break;
			goto DoPoly; 
		case 4:
		case SHPT_POLYGON:
		case SHPT_POLYGONZ: 
		case SHPT_POLYGONM:  
    		if (!GetTypeVisibility(TYPE_AREA))
    			break;
    		CurrentType = GF_AREA; 
		    if (CurrentDesc > 0 && CurrentDesc < 3201)
				CurView->CurVisType[CurrentDesc]=3; 
DoPoly:
	        if (SHPPolyHeader.NumPoints > USHRT_MAX)
	        	ii=1;
	        	//goto RtnFalse;
	        if (!SHPPolyHeader.NumPoints)
	        	goto RtnFalse;
	        if (!SHPPolyHeader.Type)
	        	goto RtnFalse;
	        nPoly = SHPPolyHeader.NumParts;  
	        NumPoints = SHPPolyHeader.NumPoints; 
	        if (SHPType != SHPT_TEXT && (SHPPolyHeader.Type == SHPT_POLYGON || SHPPolyHeader.Type == SHPT_POLYGON_WITHCURVES || SHPPolyHeader.Type == SHPT_POLYGONZ || SHPPolyHeader.Type == SHPT_POLYGONM || SHPPolyHeader.Type == SHPT_PGDB_POLYGONZ))
	        	NumPoints = NumPoints+nPoly-1;
	        hPartIndex = GSSiGlobAlloc (1418,GMEM_MOVEABLE,sizeof(long)*(nPoly+1));
	        hPolyPartLen = GSSiGlobAlloc (1419,GMEM_MOVEABLE,sizeof(int)*(nPoly+1));
			hPoints = GSSiGlobAlloc (1420,GMEM_MOVEABLE,sizeof(DPOINT)*NumPoints); 
	        pPartIndex = (HPLONG)GlobalLock (hPartIndex); 
	        hmemmove ((HPSTR)pPartIndex,&pRec[recloc],nPoly*sizeof(long));   //pPartIndex[5]
	        recloc += nPoly*sizeof(long);
	        pPartIndex[SHPPolyHeader.NumParts]= SHPPolyHeader.NumPoints;
	                    
            pPoints = pFirstPoint = (LPDPOINT)GlobalLock (hPoints); 
            pNumPoints = (LPINT)GlobalLock (hPolyPartLen);
	        for (i=0;i<nPoly;i++)
	        {   
	            long    numpoints, startpoint,ii; 
	                        
	            startpoint = *pPartIndex++;
	            numpoints = *pPartIndex-startpoint; 
	            //if (numpoints > (long)USHRT_MAX)
	            //	MessageBox (0,"Shape file record contains too many points",0,MB_ICONEXCLAMATION);
	            *pNumPoints++ = numpoints; 
	            hmemmove ((HPSTR)pPoints,&pRec[recloc],numpoints * sizeof(DPOINT));     //*(LPPOINT)&pRec[recloc]
	            recloc += numpoints * sizeof(DPOINT);
	            pPoints += numpoints;   
	            if (i && SHPType != SHPT_TEXT && ((SHPPolyHeader.Type == SHPT_POLYGON || SHPPolyHeader.Type == SHPT_POLYGON_WITHCURVES || SHPPolyHeader.Type == SHPT_POLYGONZ || SHPPolyHeader.Type == SHPT_POLYGONM)))
	            	*pPoints++ = *pFirstPoint; 
	        } 
	        ExtraBytes = BinSize - recloc; 
	        if (ExtraBytes>0)
	        {
	        	HPDPOINT pPoints = (HPDPOINT)&pRec[recloc];//pPoints[3]
	        	HPLONG pPointsl = (HPLONG)&pRec[recloc];//pPointsl[3]
	        	HPFLOAT pPointsf = (HPFLOAT)&pRec[recloc];//pPointsf[2]  
	        	HPSHORT	pPointss = (HPSHORT)&pRec[recloc];//pPointss[7]
	        	 ii = 1;
	        }
	        GlobalUnlock (hPoints); 
	        GlobalUnlock (hPolyPartLen);
	        GSSiGlobUlFree (&hPartIndex);  
			pPoints = (HPDPOINT)GlobalLock (hPoints);
			for (i=0;i<NumPoints;i++)
				ConvertCoord(&pPoints[i],0,1);  
			if (SHPPolyHeader.Type == 536870962)
			{   
				DPOINT	BP = pPoints[0], EP = pPoints[1], POC;
				LPDPOINT RP = (LPDPOINT)(&pRec[recloc]+12);
				DPOINT	MidPoint = MidPointD (BP,EP);
				double	Radius, AZ, BackAZ;
				
				ConvertCoord(RP,0,1);
				Radius = ldistpp (RP,&BP); 
				AZ = getazd (RP,&MidPoint);
				POC = dnewpt (*RP,AZ, Radius);  
				GSSiGlobUlFree (&hPoints);
				hPoints = GSSiGlobAlloc (0,GMEM_MOVEABLE,4090*sizeof(DPOINT));
				pPoints = (HPDPOINT)GlobalLock (hPoints); 
				NumPoints = 0;
				if (!Display)   
                    CurvePointsD(&BP,&POC,&EP, &NumPoints, &pPoints,&BackAZ,4090,CurveChordDist,1);
                else
                	CurvePointsD(&BP,&POC,&EP, &NumPoints, &pPoints,&BackAZ,4090,DisplayCurveFactor,1); 
                GlobalUnlock (hPoints);
				pPoints = (HPDPOINT)GlobalLock (hPoints);
//				if (Pick)
//					PickCurve (lpDCurPoints,nPnts,&BP,&POC,&EP);
				pNumPoints = (LPINT)GlobalLock (hPolyPartLen);
				*pNumPoints = NumPoints;
		        GlobalUnlock (hPolyPartLen);
				if (nPoly != 1)
					nPoly = 1;
				SHPPolyHeader.Type = SHPT_ARC;	
			}
			else if (SHPPolyHeader.Type == SHPT_POLYGON_WITHCURVES)
			{   
				LPLONG	pNumRP = (LPLONG)(&pRec[recloc]);
				short	irp;   
				typedef struct	{long	InsertAfter,RP2;
								 DPOINT	RP;
								 long	RP3;} RPBLOCK;
				typedef	RPBLOCK	FAR	*LPRPBLOCK;
				
				LPRPBLOCK	pRPBlock;
				
				recloc += 4;   
				pRPBlock = (LPRPBLOCK)(&pRec[recloc]);
				for (irp = 0;irp < *pNumRP; irp++)
				{
					pRPBlock++;
				}
			/*	DPOINT	MidPoint = MidPointD (BP,EP);
				double	Radius, AZ, BackAZ;
				
				ConvertCoord(RP,0,1);
				Radius = ldistpp (RP,&BP); 
				AZ = getazd (RP,&MidPoint);
				POC = dnewpt (*RP,AZ, Radius);  
				GSSiGlobUlFree (&hPoints);
				hPoints = GSSiGlobAlloc (0,GMEM_MOVEABLE,4090*sizeof(DPOINT));
				pPoints = (HPDPOINT)GlobalLock (hPoints); 
				NumPoints = 0;
				if (!Display)   
                    CurvePointsD(&BP,&POC,&EP, &NumPoints, &pPoints,&BackAZ,4090,CurveChordDist,1);
                else
                	CurvePointsD(&BP,&POC,&EP, &NumPoints, &pPoints,&BackAZ,4090,DisplayCurveFactor,1); 
                GlobalUnlock (hPoints);
				pPoints = (HPDPOINT)GlobalLock (hPoints);
				pNumPoints = (LPWORD)GlobalLock (hPolyPartLen);
				*pNumPoints = NumPoints;
		        GlobalUnlock (hPolyPartLen);
				nPoly = 1;*/
				SHPPolyHeader.Type = SHPT_POLYGON;	
			}
			if (SHPType == SHPT_TEXT)
			{
				GRTEXTHEADER	GRTextHeader;  
	            DPOINT	MidPoint;// = MinMaxMidPointD ((LPMNMXCORD)&SHPPolyHeader.Xmin);  
	            double	MidPointAZ; 
	            char	txt[512]="[pgdb.TEXTSTRING]";
	            char	word[256];
	            short	nchar; 
	            double	THeight, AZ;
				int		MDPointID;
	            char	THeightC[32]="[pgdb.FONTSIZE]";   
	            char	AZC[32]="[pgdb.ANGLE]";
				int		type;
       			int		nWords;
	                    
				pNumPoints = (LPINT)GlobalLock (hPolyPartLen);    
				lpDCurPoints = pPoints;
	    		ExpandText (txt);
	    		ExpandText (AZC);
				if (hElement)
				{
					type = *(LPSHORT)(pElement + 4);
					if (type && type != 2)
						ii=1;
				}
				else
					type = 1;
				if (type == 2)
					nWords = 1;
				else
					nWords = *(LPLONG)(pElement + 24);
				if (nPoly > 1 && type)
					debugvalue++;
				else if (type == 2 && *pNumPoints != 5)
					ii = 1;
        		for (i=0;i<nPoly;i++)
        		{   
        			int	np=*pNumPoints;
        			int	nchr = (np-1)/4,j;
        			
        			TLSet = FALSE;
        			if (type)
        				_fstrcpy (word,txt);
        			else
        			{
	        			if (!i)
			   				nWords = GetWords (txt); 
			   			for (j=0;j<nWords;j++)
	        			{ 
	        				if (WordLen[j] == nchr)
	        					goto HaveWord;	
	        			}
	        			j = 0;
	       	HaveWord:   _fstrcpy (word,Words[j]);
	   					nWords--;
	       				if (j < nWords)
	       				{
	       					_fmemmove (&WordLen[j],&WordLen[j+1],(nWords-j)*2);
	       					_fmemmove (&Words[j],&Words[j+1],(nWords-j)*128);
	       				}
	       			}	
        			MidPoint =  lpDCurPoints[0];//MidPointD (lpDCurPoints[0],lpDCurPoints[nchr*2]);   
		    		ExpandText (THeightC);
		    		THeight = atof (THeightC)*NonPltFileDistToBaseDist; 
					RemoveDupPolyPoints (&np,lpDCurPoints,THeight/10);
					if (nPoly > 1)
					{
						if (np > 1)
						{
							THeight = GetPolyMaxDistBetweenPoints (lpDCurPoints,np,&MDPointID);
							AZ = getazd (&lpDCurPoints[MDPointID],&lpDCurPoints[MDPointID+1]) - HALFPI;
						}
						else
							AZ = 0;
					}
					else
					{
	        			THeight = ldistpp (&lpDCurPoints[0],&lpDCurPoints[1]); 
						AZ = getazd (&lpDCurPoints[1],&lpDCurPoints[nchr*2]);
					}
					GWPolylineD (hDC,lpDCurPoints,np,0); 
					//DisplayPoint (hDC,BasePtToWinPt (&MidPoint));
					//if (ConvertCoord(&MidPoint,0,1)) 
					//	goto RtnFalse;
					CurPointLocD.x = MidPoint.x; 
					CurPointLocD.y = MidPoint.y;
					TXLoc = CurPointLocD; 
					HaveTXLoc = TRUE;
		    		LastElementBeginPoint = CurPointLocD;     
		    		//MidPointAZ = RADDEG * atof (AZC);
		    		MidPointAZ = AZ; 
		    		PTRot = MidPointAZ; 
		    		nchar = _fstrlen (word);
		    		_fmemset (&GRTextHeader,0,sizeof(GRTextHeader));
		    		GRTextHeader.lText = nchar;
					GRTextHeader.FontNum = 8;  
				 
					{
						static short	vjus=1;
						GRTextHeader.vJust=vjus;//0=above,1=baseline,2=center,3=below
					}
					GRTextHeader.hJust=0;
					SetTextHeadSize (&GRTextHeader,THeight);
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
		            		hGRText = GRTextFromTextHeader (&GRTextHeader,word);
				    	AddPointToBuffer (CurPointLocD,CurrentRefno,0,CurrentDesc,10, PTRot,0,hGRText,0,
										  CurrentTAG,CurrentUDI,-1,-1,-1,TRUE,&hUpdateBuf,&lUpdateBuf);
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
				
						ProcessTextObject (hDC,&GRTextHeader,word,nchar,0,0,0,0,0);
						if (OldColor >= 0)
							SetTextColor (hDC,OldColor); 
						if (CurVis->WantType[8])
							DisplayPointItem (hDC,BasePtToWinPt(&CurPointLocD),10,PTRot,InvisiblePointSymbol,0); 
	        		}
					lpDCurPoints+=*pNumPoints++;
					/*					if (i)
											lpDCurPoints++;*/

				} 
       			GlobalUnlock (hPolyPartLen); 
			} 
			else if (SHPPolyHeader.Type == SHPT_ARC || SHPPolyHeader.Type == SHPT_ARCZ) 
			{
				if (Pick)
				{   
					PickPolylineD (pPoints,NumPoints,0,2,0,0,0);
				}
				else if (CopyRec)  
				{
					pNumPoints = (LPINT)GlobalLock (hPolyPartLen);//pNumPoints[1]
					if (nPoly > 1)
					{
						HANDLE	hhPoly = GSSiGlobAlloc ( 418,GMEM_MOVEABLE,sizeof(HANDLE)*nPoly); 
						LPHANDLE phPoly = (LPHANDLE)GlobalLock (hhPoly);
                        HPDPOINT	pPoints1=(HPDPOINT)GlobalLock (hPoints), pPoints2;
                        
	                    for (i=0;i<nPoly;i++)
	                    {   
	                   		phPoly[i] = GSSiGlobAlloc ( 420,GMEM_MOVEABLE,sizeof(DPOINT)*(long)pNumPoints[i]);
		                    pPoints2= (HPDPOINT)GlobalLock (phPoly[i]);  
		                    hmemmove ((HPSTR)pPoints2,(HPSTR)pPoints1,sizeof(DPOINT)*(long)pNumPoints[i]);
	                        GlobalUnlock (phPoly[i]); 
	                        pPoints1 += pNumPoints[i];
	                        if (i)
	                        	pPoints1++;
	                    } 
	                    GlobalUnlock (hPoints);
						AddPolyToBuffer (nPoly,pNumPoints,phPoly,TYPE_POLYLINE,CurrentRefno,0,-1,CurrentDesc,0,CurrentTAG,CurrentUDI,
	                                     -1,-1,0,0,0,0,0,TRUE,&hUpdateBuf,&lUpdateBuf); 
		                for (i=0;i<nPoly;i++)
		                	GSSiGlobFree (&phPoly[i]);
	                    GSSiGlobUlFree (&hhPoly);
					}
					else
						AddPolyToBuffer (nPoly,pNumPoints,&hPoints,TYPE_POLYLINE,CurrentRefno,0,-1,CurrentDesc,0,CurrentTAG,CurrentUDI,
                                     	-1,-1,0,0,0,0,0,TRUE,&hUpdateBuf,&lUpdateBuf); 
                    GlobalUnlock (hPolyPartLen);
				}
				else
				{    
					lpDCurPoints = pPoints;
					nPnts = nCurPoints = NumPoints;

					if (PolyInMaskAreaFileCoord (GF_LINE,&NumPoints,0,0,&pPoints,TRUE))
					{   
						HPEN hNewPen=0;
						
						hOldPen  = SelectObject(hDC,GetStockObject(BLACK_PEN));
						TempLineColor = SHPColor; 
						TempLineWidth = SHPWidth; 
						if (TempLineColor >= 0 || TempLineWidth != 0)
						{
							hNewPen = CreatePen (PS_SOLID,(short)IDNINT(TempLineWidth*DeviceToScreenFactor),TempLineColor);
							SelectObject(hDC,hNewPen);
						}
//						if (SetDisplayChar (hDC,GF_LINE,CurrentRefno,CurrentDesc,CurrentTAG,CurrentUDI))
//						{   
							pNumPoints = (LPINT)GlobalLock (hPolyPartLen);
			        		for (i=0;i<nPoly;i++)
			        		{   
//								GWPolylineD (hDC,lpDCurPoints,*pNumPoints,CurrentDesc); 
//								lpDCurPoints+=*pNumPoints++;
			        			nPnts = *pNumPoints;  
			        			InDynamicSegmentation = FALSE;  
			        			DoDynamicFixedSegmentation (-1,0,0);
								while (nPnts > 1)
								{ 
									if (SetDisplayChar (hDC,GF_LINE,CurrentRefno,CurrentDesc,CurrentTAG,CurrentUDI) > 0)
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
								lpDCurPoints+=*pNumPoints++; 
								if (i)
									lpDCurPoints++;
			        		}   
			        		GlobalUnlock (hPolyPartLen); 
//						} 
						if (hNewPen)
						{
							SelectObject(hDC,GetStockObject(BLACK_PEN)); 
							GSSiDeleteObject (&hNewPen);
						}
					}
				}
			}
			else
			{   
				BOOL	ShowBorder = GetBit (5,(LPSTR)&CurVis->WantType[7]); 
				HPEN	hBorderPen=0;
				
				if (Pick)
				{   
					PickPolygonD (pPoints,NumPoints,9999999,0);
				}
				else if (CopyRec)  
				{
					pNumPoints = (LPINT)GlobalLock (hPolyPartLen);//pNumPoints[1]
					if (nPoly > 1)
					{
						HANDLE	hhPoly = GSSiGlobAlloc ( 418,GMEM_MOVEABLE,sizeof(HANDLE)*nPoly); 
						LPHANDLE phPoly = (LPHANDLE)GlobalLock (hhPoly);
                        HPDPOINT	pPoints1=(HPDPOINT)GlobalLock (hPoints), pPoints2;
                        
	                    for (i=0;i<nPoly;i++)
	                    {   
	                   		phPoly[i] = GSSiGlobAlloc ( 420,GMEM_MOVEABLE,sizeof(DPOINT)*(long)pNumPoints[i]);
		                    pPoints2= (HPDPOINT)GlobalLock (phPoly[i]);  
		                    hmemmove ((HPSTR)pPoints2,(HPSTR)pPoints1,sizeof(DPOINT)*(long)pNumPoints[i]);
	                        GlobalUnlock (phPoly[i]); 
	                        pPoints1 += pNumPoints[i];
	                        if (i)
	                        	pPoints1++;
	                    } 
	                    GlobalUnlock (hPoints);
						AddPolyToBuffer (nPoly,pNumPoints,phPoly,TYPE_AREA,CurrentRefno,0,-1,CurrentDesc,0,CurrentTAG,CurrentUDI,
	                                     -1,-1,0,0,0,0,0,TRUE,&hUpdateBuf,&lUpdateBuf); 
		                for (i=0;i<nPoly;i++)
		                	GSSiGlobFree (&phPoly[i]);
	                    GSSiGlobUlFree (&hhPoly);
					}
					else
						AddPolyToBuffer (nPoly,pNumPoints,&hPoints,TYPE_AREA,CurrentRefno,0,-1,CurrentDesc,0,CurrentTAG,CurrentUDI,
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

					if (PolyInMaskAreaFileCoord (GF_AREA,&NumPoints,0,0,&pPoints,TRUE))
					{
						if (SetDisplayChar (hDC,GF_AREA,CurrentRefno,CurrentDesc,CurrentTAG,CurrentUDI) > 0)
						{   
							BOOL DoBorder = FALSE;
							
							if (Display)
							{
								if (nPoly > 1 && !ShowLinkLines)
									SelectObject(hDC,h0Pen); 
								if (FillAreas && (GetBit (7,(LPSTR)&CurVis->WantType[7]) ||
												  GetBit (6,(LPSTR)&CurVis->WantType[7])))
									hDeletePen = GWPolygonD (hDC,pPoints, NumPoints, nPoly, hPolyPartLen,CurrentDesc,ShowBorder);
								else
									DoBorder = TRUE; 
								if ((DoBorder || nPoly > 1) && (hBorderPen || hDeletePen))
								{   
									if (hDeletePen)
										SelectObject(hDC,hDeletePen); 
									else
										SelectObject(hDC,hBorderPen); 
									pNumPoints = (LPINT)GlobalLock (hPolyPartLen);
					        		for (i=0;i<nPoly;i++)
					        		{   
										GWPolylineD (hDC,lpDCurPoints,*pNumPoints,0); 
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
		break;
		
		case SHPT_MULTIPOINT:
		break;
	}
	if (hOldBrush)
		SelectObject (hDC,hOldBrush);
	if (hOldPen)
		SelectObject (hDC,hOldPen); 
   	if (hDeletePen != h0Pen)
   		GSSiDeleteObject (&hDeletePen);
   	GSSiDeleteObject (&hTempBrush);
   	GSSiDeleteObject (&hTempPen);
	ShowValue (hDC,FALSE);
	InGraphicsProcessor = FALSE; 
	if (hElement)
		GlobalUnlock (hElement); 
	GlobalUnlock (hRec);
{
#if ENABLETRACE
GSSiExitProg (1375);
#endif
	return TRUE;
}
RtnFalse:
	if (hElement)
		GlobalUnlock (hElement); 
	GlobalUnlock (hRec);
	InGraphicsProcessor = FALSE;
{
#if ENABLETRACE
GSSiExitProg (1375);
#endif
	return FALSE;
}
#if ENABLETRACE
}
#endif
}



